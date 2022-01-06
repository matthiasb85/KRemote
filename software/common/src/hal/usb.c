/*
 * This file is part of The KRemote Project
 *
 * Copyright (c) 2021 Matthias Beckert
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * usb.c
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

/*
 * Include ChibiOS & HAL
 */
// clang-format off
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
// clang-format on

/*
 * Includes module API, types & config
 */
#include "api/hal/usb.h"

/*
 * Include dependencies
 */
#include <string.h>

#if defined(USE_CMD_SHELL)

/*
 * Forward declarations of static functions
 */
static void _usb_init_hal(void);
static void _usb_init_module(void);
static void _usb_event_cb(USBDriver *usbp, usbevent_t event);
static const USBDescriptor *_usb_get_descriptor_cb(USBDriver *usbp, uint8_t dtype, uint8_t dindex,
                                                   uint16_t lang);
static bool _usb_request_hook_cb(USBDriver *usbp);
static void _usb_sof_cb(USBDriver *usbp);

/*
 * Static variables
 */

/*
 * USB Device Descriptor.
 */
static const uint8_t usb_device_descriptor_data[] = {
    USB_DESC_DEVICE(0x0200,         /* bcdUSB (1.1) */
                    0xef,           /* bDeviceClass (misc) */
                    0x02,           /* bDeviceSubClass (common) */
                    0x01,           /* bDeviceProtocol (IAD) */
                    64,             /* bMaxPacketSize (64 bytes) (the driver didn't work with 32) */
                    USB_VENDOR_ID,  /* idVendor */
                    USB_PRODUCT_ID, /* idProduct */
                    USB_DEVICE_VER, /* bcdDevice */
                    1,              /* iManufacturer */
                    2,              /* iProduct */
                    3,              /* iSerialNumber */
                    1)              /* bNumConfigurations */
};

/*
 * Device Descriptor wrapper
 */
static const USBDescriptor _usb_device_descriptor = {sizeof usb_device_descriptor_data,
                                                     usb_device_descriptor_data};

static const uint8_t _usb_configuration_descriptor_data[] = {
    /* Configuration Descriptor (9 bytes) USB spec 9.6.3, page 264-266, Table 9-10 */
    USB_DESC_CONFIGURATION(USB_CONFIG_DESC_DATA_SIZE, /* wTotalLength                         */
                           USB_NUM_INTERFACES,        /* bNumInterfaces                       */
                           1,                         /* bConfigurationValue                  */
                           0,                         /* iConfiguration                       */
                           0xA0,                      /* bmAttributes (RESERVED|REMOTEWAKEUP) */
                           250),                      /* bMaxPower (500mA)                    */

    USB_DESC_INTERFACE_ASSOCIATION(USB_CDC_INT_INTERFACE,             /* bFirstInterface     */
                                   2,                                 /* bInterfaceCount     */
                                   CDC_COMMUNICATION_INTERFACE_CLASS, /* bFunctionClass      */
                                   CDC_ABSTRACT_CONTROL_MODEL,        /* bFunctionSubClass   */
                                   1,                                 /* bFunctionProcotol   */
                                   0                                  /* iInterface          */
                                   ),

    /* Interface Descriptor.*/
    USB_DESC_INTERFACE(USB_CDC_INT_INTERFACE,             /* bInterfaceNumber                                 */
                       0x00,                              /* bAlternateSetting                                */
                       0x01,                              /* bNumEndpoints                                    */
                       CDC_COMMUNICATION_INTERFACE_CLASS, /* bInterfaceClass (Communications Interface Class) */
                       CDC_ABSTRACT_CONTROL_MODEL,        /* bInterfaceSubClass (Abstract Control Model)      */
                       0x01,                              /* bInterfaceProtocol (AT commands)                 */
                       0),                                /* iInterface                                       */

    /* Header Functional Descriptor (CDC section 5.2.3).*/
    USB_DESC_BYTE(5),                /* bLength                        */
    USB_DESC_BYTE(CDC_CS_INTERFACE), /* bDescriptorType (CS_INTERFACE) */
    USB_DESC_BYTE(CDC_HEADER),       /* bDescriptorSubtype (Header)    */
    USB_DESC_BCD(0x0110),            /* bcdCDC                         */

    /* Call Management Functional Descriptor. */
    USB_DESC_BYTE(5),                      /* bFunctionLength                      */
    USB_DESC_BYTE(CDC_CS_INTERFACE),       /* bDescriptorType (CS_INTERFACE)       */
    USB_DESC_BYTE(CDC_CALL_MANAGEMENT),    /* bDescriptorSubtype (Call Management) */
    USB_DESC_BYTE(0x03),                   /* bmCapabilities (D0+D1)               */
    USB_DESC_BYTE(USB_CDC_DATA_INTERFACE), /* bDataInterface                       */

    /* ACM Functional Descriptor.*/
    USB_DESC_BYTE(4),                               /* bFunctionLength                */
    USB_DESC_BYTE(CDC_CS_INTERFACE),                /* bDescriptorType (CS_INTERFACE) */
    USB_DESC_BYTE(CDC_ABSTRACT_CONTROL_MANAGEMENT), /* bDescriptorSubtype (Abstract ) */
    USB_DESC_BYTE(0x02),                            /* bmCapabilities.                */

    /* Union Functional Descriptor.*/
    USB_DESC_BYTE(5),                      /* bFunctionLength                                   */
    USB_DESC_BYTE(CDC_CS_INTERFACE),       /* bDescriptorType (CS_INTERFACE)                    */
    USB_DESC_BYTE(CDC_UNION),              /* bDescriptorSubtype (Union  Functional Descriptor) */
    USB_DESC_BYTE(USB_CDC_INT_INTERFACE),  /* bMasterInterface (Communication Class Interface)  */
    USB_DESC_BYTE(USB_CDC_DATA_INTERFACE), /* bSlaveInterface0 (Data Class Interface)           */

    /* Endpoint 2 Descriptor. in only.*/
    USB_DESC_ENDPOINT(USB_CDC_INT_REQUEST_EP | 0x80, /* bEndpointAddress         */
                      USB_EP_MODE_TYPE_INTR,         /* bmAttributes (Interrupt) */
                      USB_CDC_INT_EPSIZE,            /* wMaxPacketSize.          */
                      0xff),                         /* bInterval                */
    /* Interface Descriptor.*/
    USB_DESC_INTERFACE(USB_CDC_DATA_INTERFACE,   /* bInterfaceNumber                        */
                       0x00,                     /* bAlternateSetting                       */
                       0x02,                     /* bNumEndpoints.                          */
                       CDC_DATA_INTERFACE_CLASS, /* bInterfaceClass (Data Class  Interface) */
                       0x00,                     /* bInterfaceSubClass (CDC section 4.6)    */
                       0x00,                     /* bInterfaceProtocol (CDC section 4.7)    */
                       0x00),                    /* iInterface                              */
    /* Endpoint 1 Descriptor.*/
    USB_DESC_ENDPOINT(USB_CDC_DATA_AVAILABLE_EP, /* bEndpointAddress    */
                      USB_EP_MODE_TYPE_BULK,     /* bmAttributes (Bulk) */
                      USB_CDC_DATA_EPSIZE,       /* wMaxPacketSize.     */
                      0x00),                     /* bInterval.          */
    /* Endpoint 1 Descriptor.*/
    USB_DESC_ENDPOINT(USB_CDC_DATA_REQUEST_EP | 0x80, /* bEndpointAddress    */
                      USB_EP_MODE_TYPE_BULK,          /* bmAttributes (Bulk) */
                      USB_CDC_DATA_EPSIZE,            /* wMaxPacketSize      */
                      0x00),                          /* bInterval           */
};

/* Configuration Descriptor wrapper */
static const USBDescriptor _usb_configuration_descriptor = {
    sizeof _usb_configuration_descriptor_data,
    _usb_configuration_descriptor_data,
};

/* wrappers */
static const USBDescriptor _usb_cdc_descriptor = {
    USB_CONFIG_DESC_CDC_SIZE,
    &_usb_configuration_descriptor_data[USB_CDC_DESC_OFFSET],
};

/*
 * U.S. English language identifier.
 */
static const uint8_t _usb_desc_string0[] = {
    USB_DESC_BYTE(4),                     /* bLength                */
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType        */
    USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English) */
};

/*
 * Vendor string.
 */
static const uint8_t _usb_desc_string1[] = {
    USB_DESC_BYTE(40),                    /* bLength         */
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType */
    USB_DESC_CHAR_PADDED('T'),
    USB_DESC_CHAR_PADDED('h'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED(' '),
    USB_DESC_CHAR_PADDED('K'),
    USB_DESC_CHAR_PADDED('R'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED('m'),
    USB_DESC_CHAR_PADDED('o'),
    USB_DESC_CHAR_PADDED('t'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED(' '),
    USB_DESC_CHAR_PADDED('P'),
    USB_DESC_CHAR_PADDED('r'),
    USB_DESC_CHAR_PADDED('o'),
    USB_DESC_CHAR_PADDED('j'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED('c'),
    USB_DESC_CHAR_PADDED('t'),
};

/*
 * Device Description string.
 */
static const uint8_t _usb_desc_string2[] = {
    USB_DESC_BYTE(46),                    /* bLength         */
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType */
    USB_DESC_CHAR_PADDED('C'),
    USB_DESC_CHAR_PADDED('o'),
    USB_DESC_CHAR_PADDED('n'),
    USB_DESC_CHAR_PADDED('f'),
    USB_DESC_CHAR_PADDED('i'),
    USB_DESC_CHAR_PADDED('g'),
    USB_DESC_CHAR_PADDED('u'),
    USB_DESC_CHAR_PADDED('r'),
    USB_DESC_CHAR_PADDED('a'),
    USB_DESC_CHAR_PADDED('b'),
    USB_DESC_CHAR_PADDED('l'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED(' '),
    USB_DESC_CHAR_PADDED('R'),
    USB_DESC_CHAR_PADDED('C'),
    USB_DESC_CHAR_PADDED(' '),
    USB_DESC_CHAR_PADDED('S'),
    USB_DESC_CHAR_PADDED('y'),
    USB_DESC_CHAR_PADDED('s'),
    USB_DESC_CHAR_PADDED('t'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED('m'),
};

/*
 * Serial Number string.
 */
static const uint8_t _usb_desc_string3[] = {
    USB_DESC_BYTE(8),                     /* bLength.        */
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType */
    '0' + CH_KERNEL_MAJOR,
    0,
    '0' + CH_KERNEL_MINOR,
    0,
    '0' + CH_KERNEL_PATCH,
    0,
};

/*
 * Strings wrappers array.
 */
static const USBDescriptor _usb_desc_strings[] = {
    {sizeof _usb_desc_string0, _usb_desc_string0},
    {sizeof _usb_desc_string1, _usb_desc_string1},
    {sizeof _usb_desc_string2, _usb_desc_string2},
    {sizeof _usb_desc_string3, _usb_desc_string3},
};

/*
 * EP states
 */
static USBInEndpointState _usb_ep_in_states[USB_NUM_IN_EPS];
static USBOutEndpointState _usb_ep_out_states[USB_NUM_OUT_EPS];

/*
 * EP configs
 */
// clang-format off
static const USBEndpointConfig _usb_ep_configs[] = {
     {USB_EP_MODE_TYPE_INTR, NULL, sduInterruptTransmitted, NULL, 0x0010, 0x0000,
     &_usb_ep_in_states[USB_CDC_INT_REQUEST_EPS_IN], NULL, 2, NULL},
     {USB_EP_MODE_TYPE_BULK, NULL, sduDataTransmitted, sduDataReceived, SERIAL_USB_BUFFERS_SIZE, SERIAL_USB_BUFFERS_SIZE,
     &_usb_ep_in_states[USB_CDC_DATA_REQUEST_EPS_IN], &_usb_ep_out_states[USB_CDC_DATA_AVAILABLE_EPS_OUT], 2, NULL},
};
// clang-format on

/*
 * USB driver configuration.
 */
const USBConfig usbcfg = {
    _usb_event_cb,
    _usb_get_descriptor_cb,
    _usb_request_hook_cb,
    _usb_sof_cb,
};

/*
 * Global variables
 */
SerialUSBDriver USB_CDC_DRIVER_HANDLE;

/*
 * Serial over USB driver configuration.
 */
const SerialUSBConfig serusbcfg = {
    &USB_DRIVER_HANDLE,
    USB_CDC_DATA_REQUEST_EP,
    USB_CDC_DATA_AVAILABLE_EP,
    USB_CDC_INT_REQUEST_EP,
};

/*
 * Tasks
 */

/*
 * Static helper functions
 */
static void _usb_init_hal(void)
{
  /*
   * Initializes a serial-over-USB CDC driver.
   */
  sduObjectInit(&(USB_CDC_DRIVER_HANDLE));
  sduStart(&(USB_CDC_DRIVER_HANDLE), &serusbcfg);
}

static void _usb_init_module(void)
{
  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
#if defined(USE_USB_DISC)
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1500);
#endif

  /*
   * Start USB driver
   */
  usbStart(&USB_DRIVER_HANDLE, &usbcfg);
  usbConnectBus(&USB_DRIVER_HANDLE);
}

/*
 * Callback functions
 */
/*
 * Handles the USB driver global events.
 */
static void _usb_event_cb(USBDriver *usbp, usbevent_t event)
{
  switch (event)
  {
    case USB_EVENT_ADDRESS:
      return;
    case USB_EVENT_CONFIGURED:
      chSysLockFromISR();

      usbInitEndpointI(usbp, USB_CDC_INT_REQUEST_EP, &_usb_ep_configs[USB_CDC_INT_INTERFACE]);
      usbInitEndpointI(usbp, USB_CDC_DATA_REQUEST_EP, &_usb_ep_configs[USB_CDC_DATA_INTERFACE]);

      /* Resetting the state of the CDC subsystem.*/
      sduConfigureHookI(&USB_CDC_DRIVER_HANDLE);

      chSysUnlockFromISR();
      return;
    case USB_EVENT_RESET:
      /* Falls into.*/
    case USB_EVENT_UNCONFIGURED:
      /* Falls into.*/
    case USB_EVENT_SUSPEND:
      chSysLockFromISR();

      /*
       * Disconnection event on suspend
       */
      sduSuspendHookI(&USB_CDC_DRIVER_HANDLE);

      chSysUnlockFromISR();
      return;
    case USB_EVENT_WAKEUP:
      chSysLockFromISR();

      /*
       * Connection event on wakeup
       */
      sduWakeupHookI(&USB_CDC_DRIVER_HANDLE);

      chSysUnlockFromISR();
      return;
    case USB_EVENT_STALLED:
      return;
  }
  return;
}

static const USBDescriptor *_usb_get_descriptor_cb(USBDriver *usbp, uint8_t dtype, uint8_t dindex,
                                                   uint16_t lang)
{
  (void)usbp;
  (void)lang;
  switch (dtype)
  {
    case USB_DESCRIPTOR_DEVICE:
      return &_usb_device_descriptor;
    case USB_DESCRIPTOR_CONFIGURATION:
      return &_usb_configuration_descriptor;
    case USB_DESCRIPTOR_STRING:
      if (dindex < 4) return &_usb_desc_strings[dindex];
      break;
    case USB_DESCRIPTOR_CDC:
      return &_usb_cdc_descriptor;
    default:
      break;
  }
  return NULL;
}

static bool _usb_request_hook_cb(USBDriver *usbp)
{
  return sduRequestsHook(usbp);
}

static void _usb_sof_cb(USBDriver *usbp)
{
  (void)usbp;

  chSysLockFromISR();

  sduSOFHookI(&USB_CDC_DRIVER_HANDLE);

  chSysUnlockFromISR();
}
#endif
/*
 * Shell functions
 */


/*
 * API functions
 */
void usb_init(void)
{
#if defined(USE_CMD_SHELL)
  _usb_init_hal();
  _usb_init_module();
#endif
}

