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

#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include "usb.h"

#include "scsi_impl.h"
#include "keyboard.h"
#include "log.h"

#include "usb_descriptors.h"

static sd_disk *sd;

static uint8_t previous_KeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];
static USB_ClassInfo_HID_Device_t keyboard_device =
{
	.Config =
		{
			.InterfaceNumber              = 0,
            .ReportINEndpoint             =
                {
                    .Address              = KEYBOARD_EPADDR,
                    .Size                 = KEYBOARD_EPSIZE,
                    .Banks                = 1,
                },

			.PrevReportINBuffer           = previous_KeyboardHIDReportBuffer,
			.PrevReportINBufferSize       = sizeof(previous_KeyboardHIDReportBuffer),
		},
};

static USB_ClassInfo_MS_Device_t ms_device =
{
	.Config =
		{
			.InterfaceNumber           = 1,
            .DataINEndpoint            =
                {
                    .Address           = MASS_STORAGE_IN_EPADDR,
                    .Size              = MASS_STORAGE_IO_EPSIZE,
                    .Banks             = 1,
                },
            .DataOUTEndpoint            =
                {
                    .Address           = MASS_STORAGE_OUT_EPADDR,
                    .Size              = MASS_STORAGE_IO_EPSIZE,
                    .Banks             = 1,
                },

			.TotalLUNs                 = 1,
		},
};

//
//static USB_ClassInfo_CDC_Device_t serial_device =
//{
//	.Config =
//		{
//			.ControlInterfaceNumber         = 2,
//            .DataINEndpoint           =
//                {
//                    .Address          = CDC_TX_EPADDR,
//                    .Size             = CDC_TXRX_EPSIZE,
//                    .Banks            = 1,
//                },
//            .DataOUTEndpoint =
//                {
//                    .Address          = CDC_RX_EPADDR,
//                    .Size             = CDC_TXRX_EPSIZE,
//                    .Banks            = 1,
//                },
//            .NotificationEndpoint =
//                {
//                    .Address          = CDC_NOTIFICATION_EPADDR,
//                    .Size             = CDC_NOTIFICATION_EPSIZE,
//                    .Banks            = 1,
//                },
////			.DataINEndpointNumber           = CDC_TX_EPNUM,
////			.DataINEndpointSize             = CDC_TXRX_EPSIZE,
////			.DataINEndpointDoubleBank       = false,
////
////			.DataOUTEndpointNumber          = CDC_RX_EPNUM,
////			.DataOUTEndpointSize            = CDC_TXRX_EPSIZE,
////			.DataOUTEndpointDoubleBank      = false,
////
////			.NotificationEndpointNumber     = CDC_NOTIFICATION_EPNUM,
////			.NotificationEndpointSize       = CDC_NOTIFICATION_EPSIZE,
////			.NotificationEndpointDoubleBank = false,
//		},
//};


void usb_setup(sd_disk *_sd)
{
	MCUSR &= ~(1 << WDRF);
	wdt_disable();
	sd = _sd;

	clock_prescale_set(clock_div_1);

	USB_Init();
	sei();
}

void usb_task(void)
{

	//CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
//	uint16_t got = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
//	if (got > 0) {
//		CDC_Device_SendByte(&VirtualSerial_CDC_Interface, got &0xFF);
//		CDC_Device_Flush(&VirtualSerial_CDC_Interface);
//	}

//	CDC_Device_USBTask(&serial_device);
	MS_Device_USBTask(&ms_device);
	HID_Device_USBTask(&keyboard_device);
	USB_USBTask();
}



////
//// serial
////
//uint16_t usb_serial_receive(char *buf, uint16_t max_length)
//{
//	uint16_t c, i, x;
//
//	cli();
//	for (i=0; CDC_Device_BytesReceived(&serial_device) > 0 && i<max_length-1; i++) {
//			buf[i] = CDC_Device_ReceiveByte(&serial_device);
//	}
//	buf[i] = 0;
//	sei();
//	return i;
//}
//
//
//
//
//uint8_t usb_serial_send(const char *buf)
//{
//	cli();
//	uint8_t retval = CDC_Device_SendString(&serial_device, buf);
//	sei();
//	return retval;
//}
//




//
// USB Events.
//

void EVENT_USB_Device_Connect(void)
{
}

void EVENT_USB_Device_Disconnect(void)
{
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool success = true;
	success &= MS_Device_ConfigureEndpoints(&ms_device);
	success &= HID_Device_ConfigureEndpoints(&keyboard_device);
//	success &= CDC_Device_ConfigureEndpoints(&serial_device);

	USB_Device_EnableSOFEvents();

	if (!success) {
		led_error(USB_ERROR);
	}
}

void EVENT_USB_Device_ControlRequest(void)
{
	MS_Device_ProcessControlRequest(&ms_device);
	HID_Device_ProcessControlRequest(&keyboard_device);
//	CDC_Device_ProcessControlRequest(&serial_device);

}

void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&keyboard_device);
}

//
// USB Callbacks
//
bool CALLBACK_MS_Device_SCSICommandReceived(USB_ClassInfo_MS_Device_t* const device)
{
    return scsi_command(sd, device);
}

bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo, uint8_t* const ReportID,
                                         const uint8_t ReportType, void* ReportData, uint16_t* const ReportSize)
{
	USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;
	static uint8_t count = 0;

	if (count == 20) {
		KeyboardReport->KeyCode[0] = 0;
		log_consume_as_keyboard_report((uint8_t *)KeyboardReport);
		count = 0;
	} else {
	    count++;
		KeyboardReport->KeyCode[0] = 0;
	}

	*ReportSize = sizeof(USB_KeyboardReport_Data_t);
	return false;
}

void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{

}

