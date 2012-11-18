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


#include "usb_descriptors.h"

const USB_Descriptor_HIDReport_Datatype_t PROGMEM KeyboardReport[] =
{
    HID_DESCRIPTOR_KEYBOARD(6)
};

const USB_Descriptor_Device_t PROGMEM DeviceDescriptor =
{
    .Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

    .USBSpecification       = VERSION_BCD(01.10),
    .Class                  = USB_CSCP_NoDeviceClass,
    .SubClass               = USB_CSCP_NoDeviceSubclass,
    .Protocol               = USB_CSCP_NoDeviceProtocol,

    .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

    .VendorID               = 0x16D0,
    .ProductID              = 0x06EA,
    .ReleaseNumber          = VERSION_BCD(00.90),

    .ManufacturerStrIndex   = 0x01,
    .ProductStrIndex        = 0x02,
    .SerialNumStrIndex      = USE_INTERNAL_SERIAL,

    .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor =
{
.Config =
    {
        .Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

        .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
        .TotalInterfaces        = 2,
        .ConfigurationNumber    = 1,
        .ConfigurationStrIndex  = NO_DESCRIPTOR,

        .ConfigAttributes       = USB_CONFIG_ATTR_RESERVED,

        .MaxPowerConsumption    = USB_CONFIG_POWER_MA(500)
    },

    .HID_KeyboardInterface =
        {
            .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

            .InterfaceNumber        = 0,
            .AlternateSetting       = 0,

            .TotalEndpoints         = 1,

            .Class                  = HID_CSCP_HIDClass,
            .SubClass               = HID_CSCP_BootSubclass,
            .Protocol               = HID_CSCP_KeyboardBootProtocol,

            .InterfaceStrIndex      = NO_DESCRIPTOR
        },

    .HID_KeyboardHID =
        {
            .Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

            .HIDSpec                = VERSION_BCD(01.11),
            .CountryCode            = 0x00,
            .TotalReportDescriptors = 1,
            .HIDReportType          = HID_DTYPE_Report,
            .HIDReportLength        = sizeof(KeyboardReport)
        },

    .HID_ReportInEndpoint =
        {
            .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

            .EndpointAddress        = KEYBOARD_EPADDR,
            .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
            .EndpointSize           = KEYBOARD_EPSIZE,
            .PollingIntervalMS      = 0x01
        },

    .MS_Interface =
        {
            .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

            .InterfaceNumber        = 1,
            .AlternateSetting       = 0,

            .TotalEndpoints         = 2,

            .Class                  = MS_CSCP_MassStorageClass,
            .SubClass               = MS_CSCP_SCSITransparentSubclass,
            .Protocol               = MS_CSCP_BulkOnlyTransportProtocol,

            .InterfaceStrIndex      = NO_DESCRIPTOR
        },

    .MS_DataInEndpoint =
        {
            .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

            .EndpointAddress        = MASS_STORAGE_IN_EPADDR,
            .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
            .EndpointSize           = MASS_STORAGE_IO_EPSIZE,
            .PollingIntervalMS      = 0x01
        },

    .MS_DataOutEndpoint =
        {
            .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

            .EndpointAddress        = MASS_STORAGE_OUT_EPADDR,
            .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
            .EndpointSize           = MASS_STORAGE_IO_EPSIZE,
            .PollingIntervalMS      = 0x01
        },

//
//    .CDC_IAD =
//        {
//            .Header                 = {.Size = sizeof(USB_Descriptor_Interface_Association_t), .Type = DTYPE_InterfaceAssociation},
//
//            .FirstInterfaceIndex    = 2,
//            .TotalInterfaces        = 2,
//
//            .Class                  = CDC_CSCP_CDCClass,
//            .SubClass               = CDC_CSCP_ACMSubclass,
//            .Protocol               = CDC_CSCP_ATCommandProtocol,
//
//            .IADStrIndex            = NO_DESCRIPTOR
//        },
//
//    .CDC_CCI_Interface =
//        {
//            .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
//
//            .InterfaceNumber        = 2,
//            .AlternateSetting       = 0,
//
//            .TotalEndpoints         = 1,
//
//            .Class                  = CDC_CSCP_CDCClass,
//            .SubClass               = CDC_CSCP_ACMSubclass,
//            .Protocol               = CDC_CSCP_ATCommandProtocol,
//
//            .InterfaceStrIndex      = NO_DESCRIPTOR
//        },
//
//    .CDC_Functional_Header =
//        {
//            .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalHeader_t), .Type = DTYPE_CSInterface},
//            .Subtype                = CDC_DSUBTYPE_CSInterface_Header,
//
//            .CDCSpecification       = VERSION_BCD(01.10),
//        },
//
//    .CDC_Functional_ACM =
//        {
//            .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalACM_t), .Type = DTYPE_CSInterface},
//            .Subtype                = CDC_DSUBTYPE_CSInterface_ACM,
//
//            .Capabilities           = 0x06,
//        },
//
//    .CDC_Functional_Union =
//        {
//            .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalUnion_t), .Type = DTYPE_CSInterface},
//            .Subtype                = CDC_DSUBTYPE_CSInterface_Union,
//
//            .MasterInterfaceNumber  = 2,
//            .SlaveInterfaceNumber   = 3,
//        },
//
//    .CDC_NotificationEndpoint =
//        {
//            .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
//
//            .EndpointAddress        = CDC_NOTIFICATION_EPADDR,
//            .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
//            .EndpointSize           = CDC_NOTIFICATION_EPSIZE,
//            .PollingIntervalMS      = 0xFF
//        },
//
//    .CDC_DCI_Interface =
//        {
//            .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
//
//            .InterfaceNumber        = 3,
//            .AlternateSetting       = 0,
//
//            .TotalEndpoints         = 2,
//
//            .Class                  = CDC_CSCP_CDCDataClass,
//            .SubClass               = CDC_CSCP_NoDataSubclass,
//            .Protocol               = CDC_CSCP_NoDataProtocol,
//
//            .InterfaceStrIndex      = NO_DESCRIPTOR
//        },
//
//    .CDC_DataOutEndpoint =
//        {
//            .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
//
//            .EndpointAddress        = CDC_RX_EPADDR,
//            .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
//            .EndpointSize           = CDC_TXRX_EPSIZE,
//            .PollingIntervalMS      = 0x01
//        },
//
//    .CDC_DataInEndpoint =
//        {
//            .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
//
//            .EndpointAddress        = CDC_TX_EPADDR,
//            .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
//            .EndpointSize           = CDC_TXRX_EPSIZE,
//            .PollingIntervalMS      = 0x01
//        },
};

const USB_Descriptor_String_t PROGMEM LanguageString =
{
    .Header                 = {.Size = USB_STRING_LEN(1), .Type = DTYPE_String},
    .UnicodeString          = {LANGUAGE_ID_ENG}
};

const USB_Descriptor_String_t PROGMEM ManufacturerString =
{
    .Header                 = {.Size = USB_STRING_LEN(9), .Type = DTYPE_String},
    .UnicodeString          = L"phatIO oy"
};

const USB_Descriptor_String_t PROGMEM ProductString =
{
    .Header                 = {.Size = USB_STRING_LEN(6), .Type = DTYPE_String},
    .UnicodeString          = L"phatIO"
};

uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress)
{
    const uint8_t  DescriptorType   = (wValue >> 8);
    const uint8_t  DescriptorNumber = (wValue & 0xFF);

    const void* Address = NULL;
    uint16_t    Size    = NO_DESCRIPTOR;

    switch (DescriptorType)
    {
        case DTYPE_Device:
            Address = &DeviceDescriptor;
            Size    = sizeof(USB_Descriptor_Device_t);
            break;
        case DTYPE_Configuration:
            Address = &ConfigurationDescriptor;
            Size    = sizeof(USB_Descriptor_Configuration_t);
            break;
        case DTYPE_String:
            switch (DescriptorNumber)
            {
                case 0x00:
                    Address = &LanguageString;
                    Size    = pgm_read_byte(&LanguageString.Header.Size);
                    break;
                case 0x01:
                    Address = &ManufacturerString;
                    Size    = pgm_read_byte(&ManufacturerString.Header.Size);
                    break;
                case 0x02:
                    Address = &ProductString;
                    Size    = pgm_read_byte(&ProductString.Header.Size);
                    break;
            }

            break;
        case HID_DTYPE_HID:
            Address = &ConfigurationDescriptor.HID_KeyboardHID;
            Size    = sizeof(USB_HID_Descriptor_HID_t);
            break;
        case HID_DTYPE_Report:
            Address = &KeyboardReport;
            Size    = sizeof(KeyboardReport);
            break;
    }

    *DescriptorAddress = Address;
    return Size;
}

