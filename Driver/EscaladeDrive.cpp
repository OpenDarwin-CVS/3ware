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
// EscaladeDrive
//

// Master include - do not include other headers here.
#include "Escalade.h"

// convenience
#define super	IOBlockStorageDevice
#define self	EscaladeDrive

OSDefineMetaClassAndStructors(self, super);

////////////////////////////////////////////////////////////////////////////////
// Attach method
//
bool
self::attach(IOService *provider)
{
    OSDictionary	*aDictionary;
    EscaladeController	*withController;
    
    if ((withController = OSDynamicCast(EscaladeController, provider)) == NULL) {
	error("attach by wrong controller/provider");
	return(false);
    }
    
    if (!super::attach(provider))
	return(false);
    
    controller = withController;
    setIcon();

    // set interconnect properties
    if ((aDictionary = OSDictionary::withCapacity(2)) != NULL) {
	aDictionary->setObject(kIOPropertyPhysicalInterconnectTypeKey,
			controller->getProperty(kIOPropertyPhysicalInterconnectTypeKey));
	aDictionary->setObject(kIOPropertyPhysicalInterconnectLocationKey,
			controller->getProperty(kIOPropertyPhysicalInterconnectLocationKey));
	setProperty(kIOPropertyProtocolCharacteristicsKey, aDictionary);
	aDictionary->release();
    }

    return(true);
}

////////////////////////////////////////////////////////////////////////////////
// Detach method
//
void
self::detach(IOService *provider)
{
    if (controller == provider) {
	controller = NULL;
	super::detach(provider);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Parameter accessors
//
void
self::setUnit(int newUnit)
{
    unitNumber = newUnit;
    setProperty("IOUnit", (UInt64)newUnit, 32);
}

void
self::setSize(UInt32 newSize)
{
    unitSize = newSize;
}

void
self::setConfiguration(char *config)
{
    OSDictionary	*aDictionary;
    OSString		*aString;
    char		stringBuf[80];
    
    sprintf(unitDescription, "%s %s", controller->getControllerName(), config);

    // set device properties
    if ((aDictionary = OSDictionary::withCapacity(2)) != NULL) {
	if ((aString = OSString::withCString("3ware")) != NULL) {
	    aDictionary->setObject(kIOPropertyVendorNameKey, aString);
	    aString->release();
	}
	sprintf(stringBuf, "%s %s", controller->getControllerName(), config);
	if ((aString = OSString::withCString(stringBuf)) != NULL) {
	    aDictionary->setObject(kIOPropertyProductNameKey, aString);
	    aString->release();
	}
	// for completeness, not inserting controller revision here
	if ((aString = OSString::withCString("")) != NULL) {
	    aDictionary->setObject(kIOPropertyProductRevisionLevelKey, aString);
	    aString->release();
	}
	setProperty(kIOPropertyDeviceCharacteristicsKey, aDictionary);
    }
}

////////////////////////////////////////////////////////////////////////////////
// IOBlockStorageDevice protocol
//

IOReturn
self::doAsyncReadWrite(IOMemoryDescriptor *buffer, UInt32 block, UInt32 nblks, IOStorageCompletion completion)
{
    // check for termination in progress
    if (isInactive())
	return(kIOReturnNotAttached);
    
    return(controller->doAsyncReadWrite(unitNumber, buffer, block, nblks, completion));
}

IOReturn
self::doEjectMedia(void)
{
    return(kIOReturnUnsupported);
}

IOReturn
self::doFormatMedia(__unused UInt64 byteCapacity)
{
    return(kIOReturnUnsupported);
}

UInt32
self::doGetFormatCapacities(UInt64 *capacities,  __unused UInt32 capacitiesMaxCount) const
{
    *capacities = (UInt64)unitSize * TWE_SECTOR_SIZE;
    return(1);
}

IOReturn
self::doLockUnlockMedia(__unused bool doLock)
{
    return(kIOReturnUnsupported);
}

IOReturn
self::doSynchronizeCache(void)
{
    // check for termination in progress
    if (isInactive())
	return(kIOReturnNotAttached);

    return(controller->doSynchronizeCache(unitNumber));
}

char *
self::getVendorString(void)
{
    return("3ware");
}

char *
self::getProductString(void)
{
    return(unitDescription);
}

char *
self::getRevisionString(void)
{
    return(controller->getControllerVersion());
}

char *
self::getAdditionalDeviceInfoString(void)
{
    return(unitDescription);
}

IOReturn
self::reportBlockSize(UInt64 *blockSize)
{
    *blockSize = TWE_SECTOR_SIZE;
    return(kIOReturnSuccess);
}

IOReturn
self::reportEjectability(bool *isEjectable)
{
    *isEjectable = false;
    return(kIOReturnSuccess);
}

IOReturn
self::reportLockability(bool *isLockable)
{
    *isLockable = false;
    return(kIOReturnSuccess);
}

IOReturn
self::reportMaxReadTransfer(__unused UInt64 blockSize, UInt64 *max)
{
    *max = TWE_MAX_IO_LENGTH * TWE_SECTOR_SIZE;
    return(kIOReturnSuccess);
}

IOReturn
self::reportMaxWriteTransfer(__unused UInt64 blockSize, UInt64 *max)
{
    *max = TWE_MAX_IO_LENGTH * TWE_SECTOR_SIZE;
    return(kIOReturnSuccess);
}

IOReturn
self::reportMaxValidBlock(UInt64 *maxBlock)
{
    *maxBlock = unitSize - 1;		// last addressable block
    return(kIOReturnSuccess);
}

IOReturn
self::reportMediaState(bool *mediaPresent, bool *changedState)
{
    *mediaPresent = true;
    *changedState = true;
    return(kIOReturnSuccess);
}

IOReturn
self::reportPollRequirements(bool *pollRequired, bool *pollIsExpensive)
{
    *pollRequired = false;
    *pollIsExpensive = false;
    return(kIOReturnSuccess);
}

IOReturn
self::reportRemovability(bool *isRemovable)
{
    *isRemovable = false;	// not *strictly* correct
    return(kIOReturnSuccess);
}

IOReturn
self::reportWriteProtection(bool *isWriteProtected)
{
    *isWriteProtected = false;
    return(kIOReturnSuccess);
}

//////////////////////////////////////////////////////////////////////////////
// setIcon
//
// Establish the appropriate icon for this drive
//
void
self::setIcon(void)
{
    OSDictionary *dict;
    OSString	*identifier;
    OSString	*resourceFile;

    // Don't override a preexisting icon property
    if (getProperty(kIOMediaIconKey, gIOServicePlane) != NULL)
	return;
    
    dict = OSDictionary::withCapacity(2);
    identifier = OSString::withCString("com.3ware");
    resourceFile = OSString::withCString("Escalade.icns");
    dict->setObject("CFBundleIdentifier", identifier);
    dict->setObject(kIOBundleResourceFileKey, resourceFile);
    setProperty(kIOMediaIconKey, dict);
    resourceFile->release();
    identifier->release();
    dict->release();    
}