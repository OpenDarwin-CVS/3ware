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
// Master include file
//
// The include dependancies for this project are terribly incestuous, so
// in the name of simplicity everything is included here, and then each
// source file just includes us once.
//

#ifndef ESCALADE_H
#define ESCALADE_H

//////////////////////////////////////////////////////////////////////////////
// Tunables

// largest misaligned transfer allowed
#define TWE_MAX_COPYBUFFER	(128 * 1024)

// AEN's we will buffer
#define TWE_MAX_AEN_BUFFER	64


// System headers.	XXX prune!
#include <mach/vm_types.h>
#include <kern/clock.h>
#include <libkern/OSByteOrder.h>
#include <sys/cdefs.h>

#include <IOKit/assert.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOCommand.h>
#include <IOKit/IOCommandPool.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOMemoryCursor.h>
#include <IOKit/IOReturn.h>
#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOUserClient.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/storage/IOBlockStorageDevice.h>
#include <IOKit/storage/IOStorage.h>
#include <IOKit/storage/IOStorageDeviceCharacteristics.h>
#include <IOKit/pwr_mgt/RootDomain.h>

// Forward-delcare all our classes.
class com_3ware_EscaladeController;
class com_3ware_EscaladeCommand;
class com_3ware_EscaladeDrive;
class com_3ware_EscaladeUserClient;
class com_3ware_EscaladeMemoryCursor;
// shorthand for sanity's sake
#define EscaladeController	com_3ware_EscaladeController
#define EscaladeCommand		com_3ware_EscaladeCommand
#define EscaladeDrive		com_3ware_EscaladeDrive
#define EscaladeUserClient	com_3ware_EscaladeUserClient
#define EscaladeMemoryCursor	com_3ware_EscaladeMemoryCursor

// Local headers, do not reorder.
#include "EscaladeUserClientInterface.h"
#include "EscaladeUserClient.h"
#include "EscaladeRegisters.h"
#include "EscaladeMemoryCursor.h"
#include "EscaladeController.h"
#include "EscaladeCommand.h"
#include "EscaladeDrive.h"


#endif // ESCALADE_H