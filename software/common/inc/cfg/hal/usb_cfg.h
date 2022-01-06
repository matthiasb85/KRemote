/*
 * This file is part of The KRemote Project
 *
 * Copyright (c) 2022 Matthias Beckert
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
 * usb_cfg.h
 *
 *  Created on: 06.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_CFG_HAL_USB_CFG_H_
#define INC_CFG_HAL_USB_CFG_H_

#define USB_DRIVER_HANDLE        USBD1
#define USB_CDC_DRIVER_HANDLE    SDU1
#define USB_VENDOR_ID            0xFEED
#define USB_PRODUCT_ID           0xBABE
#define USB_DEVICE_VER           0x0200
#define USB_CDC_INT_EPSIZE       0x08
#define USB_CDC_DATA_EPSIZE      0x40
#define USB_CONFIG_DESC_CDC_SIZE 0x13

#define USB_DESCRIPTOR_CDC       CDC_CS_INTERFACE

#define USB_CDC_DESC_DATA_SIZE                                                   \
  (USB_DESC_INTERFACE_SIZE + USB_CONFIG_DESC_CDC_SIZE + USB_DESC_ENDPOINT_SIZE + \
   USB_DESC_INTERFACE_SIZE + USB_DESC_ENDPOINT_SIZE + USB_DESC_ENDPOINT_SIZE)

#define USB_CONFIG_DESC_DATA_SIZE  (USB_CDC_DESC_DATA_SIZE)
#define USB_CDC_DESC_OFFSET        (USB_DESC_CONFIGURATION_SIZE + USB_DESC_INTERFACE_ASSOCIATION_SIZE)

#endif /* INC_CFG_HAL_USB_CFG_H_ */
