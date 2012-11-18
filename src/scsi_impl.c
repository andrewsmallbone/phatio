/****************************************************************************

    Copyright (c) 2012, Andrew Smallbone <andrew@phatio.com>
    
   Developed as part of the phatIO system.  Support and information 
     available at www.phatio.com
    
   Please support development of this and other great open source projects
     by purchasing products from phatIO.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met: 

 1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer. 
 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

/* This file is heavily based on the LUFA project SCSI.c example files, see lufa-license.txt */

#include "scsi_impl.h"
#include "disk.h"
#include "log.h"

#define debug(...)

static SCSI_Request_Sense_Response_t error_response = {
        .ResponseCode = 0x70,
        .AdditionalLength = 0x0A
};

void set_error_response(uint8_t key, uint8_t code, uint8_t qualifier)
{
    error_response.SenseKey = (key);
    error_response.AdditionalSenseCode = (code);
    error_response.AdditionalSenseQualifier = (qualifier);
}

static const SCSI_Inquiry_Response_t inquiry_response = {
    .DeviceType = 0x00, //block device
    .PeripheralQualifier = 0,
    .Removable = true,
    .Version = 0,
    .ResponseDataFormat = 2, .NormACA = false, .TrmTsk = false, .AERC = false,

    .AdditionalLength = 0x1F,
    // multip?.Reserved3[1] = 0x10,
    .SoftReset = false, .CmdQue = false, .Linked = false, .Sync = true, .WideBus16Bit = false,
    .WideBus32Bit = false, .RelAddr = false,
    .VendorID = "phatIO", .ProductID = "phatIO", .RevisionID = { '0', '.', '9', '0' },
};




// helper method to do clear inpoint and set dataTransferLength
void clear(USB_ClassInfo_MS_Device_t* const device, uint16_t bytesTransferred)
{
    Endpoint_ClearIN();
    device->State.CommandBlock.DataTransferLength -= bytesTransferred;
}

static bool SCSI_Command_Inquiry(USB_ClassInfo_MS_Device_t* const device)
{
    uint16_t allocationLength = SwapEndian_16(*(uint16_t*) &device->State.CommandBlock.SCSICommandData[3]);
    uint16_t bytesTransferred = MIN(allocationLength, sizeof(inquiry_response));

    /* Only the standard INQUIRY data is supported, check if any optional INQUIRY bits set */
    if ((device->State.CommandBlock.SCSICommandData[1] & ((1 << 0) | (1 << 1)))
            || device->State.CommandBlock.SCSICommandData[2]) {
        /* Optional but unsupported bits set - update the SENSE key and fail the request */
        set_error_response(SCSI_SENSE_KEY_ILLEGAL_REQUEST, SCSI_ASENSE_INVALID_FIELD_IN_CDB, SCSI_ASENSEQ_NO_QUALIFIER);

        return false;
    }

    Endpoint_Write_Stream_LE(&inquiry_response, bytesTransferred, NULL);

    /* Pad out remaining bytes with 0x00 */
    Endpoint_Null_Stream((allocationLength - bytesTransferred), NULL);

    clear(device, bytesTransferred);
    return true;
}

static bool SCSI_Command_Request_Sense(USB_ClassInfo_MS_Device_t* const device)
{
    uint8_t allocationLength = device->State.CommandBlock.SCSICommandData[4];
    uint8_t bytesTransferred = MIN(allocationLength, sizeof(error_response));

    Endpoint_Write_Stream_LE(&error_response, bytesTransferred, NULL);
    Endpoint_Null_Stream((allocationLength - bytesTransferred), NULL);
    clear(device, bytesTransferred);

    return true;
}

static bool SCSI_Command_Read_Capacity_10(sd_disk *sd, USB_ClassInfo_MS_Device_t* const device)
{
    uint32_t lastBlock = (sd->sd_size - 1); // last LUN
    uint32_t blockSize = FS_BLOCK_SIZE;

    Endpoint_Write_Stream_BE(&lastBlock, sizeof(lastBlock), NULL);
    Endpoint_Write_Stream_BE(&blockSize, sizeof(blockSize), NULL);
    clear(device, 8);

    return true;
}

static bool SCSI_Command_Send_Diagnostic(USB_ClassInfo_MS_Device_t* const device)
{
    /* Succeed the command and update the bytes transferred counter */
    device->State.CommandBlock.DataTransferLength = 0;

    return true;
}

static bool SCSI_Command_ReadWrite_10(sd_disk *fs, USB_ClassInfo_MS_Device_t* const device,
        const bool isWrite)
{
    uint32_t BlockAddress;
    uint16_t TotalBlocks;

    /* Load in the 32-bit block address (SCSI uses big-endian, so have to reverse the byte order) */
    BlockAddress = SwapEndian_32(
            *(uint32_t*) &device->State.CommandBlock.SCSICommandData[2]);

    /* Load in the 16-bit total blocks (SCSI uses big-endian, so have to reverse the byte order) */
    TotalBlocks = SwapEndian_16(
            *(uint16_t*) &device->State.CommandBlock.SCSICommandData[7]);
    /* Check if the block address is outside the maximum allowable value for the LUN */
    if (BlockAddress >= fs->sd_size) {
        /* Block address is invalid, update SENSE key and return command fail */
        set_error_response(SCSI_SENSE_KEY_ILLEGAL_REQUEST,
                SCSI_ASENSE_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE, SCSI_ASENSEQ_NO_QUALIFIER);

        return false;
    }
    if (isWrite) {
        write_blocks(BlockAddress, TotalBlocks, fs);
    } else {
        read_blocks(BlockAddress, TotalBlocks, fs);
    }

    /* Update the bytes transferred counter and succeed the command */
    device->State.CommandBlock.DataTransferLength -= ((uint32_t) TotalBlocks * FS_BLOCK_SIZE);

    return true;
}

static bool SCSI_Command_ModeSense_6(USB_ClassInfo_MS_Device_t* const device)
{
    /* Send an empty header response with the Write Protect flag status */
    Endpoint_Write_8(0x00);
    Endpoint_Write_8(0x00);
    Endpoint_Write_8(0x00);
    Endpoint_Write_8(0x00);
    clear(device, 4);
    return true;
}

bool scsi_command(sd_disk *sd, USB_ClassInfo_MS_Device_t* const device)
{
    bool commandSucceeded = false;
    switch (device->State.CommandBlock.SCSICommandData[0]) {
    case SCSI_CMD_INQUIRY:
        commandSucceeded = SCSI_Command_Inquiry(device);
        debug("inq: %4X %c\n", device->State.CommandBlock.SCSICommandData[0], commandSucceeded ? 't':'f');
        break;
    case SCSI_CMD_REQUEST_SENSE:
        commandSucceeded = SCSI_Command_Request_Sense(device);
        debug("rs: %4X %c\n", device->State.CommandBlock.SCSICommandData[0], commandSucceeded ? 't':'f');
        break;
    case SCSI_CMD_READ_CAPACITY_10:
        commandSucceeded = SCSI_Command_Read_Capacity_10(sd, device);
        debug("rc: %4X %c\n", device->State.CommandBlock.SCSICommandData[0], commandSucceeded ? 't':'f');
        break;
    case SCSI_CMD_WRITE_10:
    case SCSI_CMD_READ_10:
        commandSucceeded = SCSI_Command_ReadWrite_10(sd, device,
                device->State.CommandBlock.SCSICommandData[0] == SCSI_CMD_WRITE_10);
        debug("r: %4X %c\n", device->State.CommandBlock.SCSICommandData[0], commandSucceeded ? 't':'f');
        break;
    case SCSI_CMD_MODE_SENSE_6:
        commandSucceeded = SCSI_Command_ModeSense_6(device);
        debug("ms: %4X %c\n", device->State.CommandBlock.SCSICommandData[0], commandSucceeded ? 't':'f');
        break;
    case SCSI_CMD_TEST_UNIT_READY:
    case SCSI_CMD_VERIFY_10:
    case SCSI_CMD_SEND_DIAGNOSTIC: // just succeed
        commandSucceeded = true;
        device->State.CommandBlock.DataTransferLength = 0;
        debug("unk: %4X %c\n", device->State.CommandBlock.SCSICommandData[0], commandSucceeded ? 't':'f');
        break;
    default: // invalid command
        set_error_response(SCSI_SENSE_KEY_ILLEGAL_REQUEST, SCSI_ASENSE_INVALID_COMMAND,SCSI_ASENSEQ_NO_QUALIFIER);
        debug("s: %4X %c\n", device->State.CommandBlock.SCSICommandData[0], commandSucceeded ? 't':'f');
        break;
    }

    /* Check if command was successfully processed */
    if (commandSucceeded) {
        set_error_response(SCSI_SENSE_KEY_GOOD, SCSI_ASENSE_NO_ADDITIONAL_INFORMATION, SCSI_ASENSEQ_NO_QUALIFIER);
    }
    return commandSucceeded;
}
