//-
// Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
//
// @APPLE_LICENSE_HEADER_START@
// 
// Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
// 
// This file contains Original Code and/or Modifications of Original Code
// as defined in and that are subject to the Apple Public Source License
// Version 2.0 (the 'License'). You may not use this file except in
// compliance with the License. Please obtain a copy of the License at
// http://www.opensource.apple.com/apsl/ and read it before using this
// file.
// 
// The Original Code and all software distributed under the License are
// distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
// EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
// INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
// Please see the License for the specific language governing rights and
// limitations under the License.
// 
// @APPLE_LICENSE_HEADER_END@
//
// $Id$

//
// This file defines the API between the Escalade UserClient and
// a userland client.
//

#ifndef ESCALADEUSERCLIENTINTERFACE_H
#define ESCALADEUSERCLIENTINTERFACE_H

#define ESCALADE_DRIVER_VERSION	20031129	// datestamp YYYYMMDD

enum {
    // Obtain exclusive access to the controller
    kEscaladeUserClientOpen,			// -

    // Release exclusive access to the controller
    kEscaladeUserClientClose,			// -

    // Return the controller architecture
    kEscaladeUserClientControllerArchitecture,	// ScalarO

    // Return the driver version (date)
    kEscaladeUserClientDriverVersion,		// ScalarO

    // Terminate and detach the numbered unit
    kEscaladeUserClientRemoveUnit,		// ScalarI

    // Probe and (possibly) attach the numbered unit
    kEscaladeUserClientAddUnit,			// ScalarI

    // Execute a command on the controller
    kEscaladeUserClientCommand,			// StructureI

    // Fetch the next AEN from the controller queue
    kEscaladeUserClientGetAEN,			// ScalarO

    // Request a controller reset
    kEscaladeUserClientReset,			// -

    // enum range limit
    kEscaladeUserClientMethodCount
};

// Controller architecture definitions
#ifndef TW_API_UNKNOWN_ARCHITECTURE
# define TW_API_5000_ARCHITECTURE                       0x0001
# define TW_API_6000_ARCHITECTURE                       0x0002
# define TW_API_7000_ARCHITECTURE                       0x0003
# define TW_API_8000_ARCHITECTURE                       0x0203
# define TW_API_9000_ARCHITECTURE                       0x0004
# define TW_API_UNKNOWN_ARCHITECTURE                        -1
#endif


// User command structure
typedef struct
{
    int			driver_opcode;
#define EUC_OPCODE_9xxx_COMMAND	(~0)
    vm_address_t	command_packet;
    vm_address_t	data_buffer;
    vm_offset_t		data_size;
} EscaladeUserCommand;

#endif ESCALADEUSERCLIENTINTERFACE_H
