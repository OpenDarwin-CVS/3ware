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
// This class provides a shim between the IOBlockStorage class and the
// controller.  We keep per-drive private state in this object as well.
//

#ifndef ESCALADEDRIVE_H
#define ESCALADEDRIVE_H


class EscaladeController;

class EscaladeDrive : public IOBlockStorageDevice
{
    OSDeclareDefaultStructors(EscaladeDrive);

public:
    // superclass overrides
    virtual bool	attach(IOService *provider);
    virtual void	detach(IOService *provider);
    
    // parameter interface
    virtual void	setUnit(int unit);
    virtual void	setSize(UInt32 size);
    virtual void	setConfiguration(char *config);
    
    // IOBlockStorageDevice protocol
    virtual IOReturn	doAsyncReadWrite(IOMemoryDescriptor *buffer,
				      UInt32 block, UInt32 nblks, IOStorageCompletion completion);
    virtual IOReturn	doEjectMedia(void);
    virtual IOReturn	doFormatMedia(UInt64 byteCapacity);
    virtual UInt32	doGetFormatCapacities(UInt64 *capacities,  UInt32 capacitiesMaxCount) const;
    virtual IOReturn	doLockUnlockMedia(bool doLock);
    virtual IOReturn	doSynchronizeCache(void);
    virtual char	*getVendorString(void);
    virtual char	*getProductString(void);
    virtual char	*getRevisionString(void);
    virtual char 	*getAdditionalDeviceInfoString(void);
    virtual IOReturn	reportBlockSize(UInt64 *blockSize);
    virtual IOReturn	reportEjectability(bool *isEjectable);
    virtual IOReturn	reportLockability(bool *isLockable);
    virtual IOReturn	reportMaxReadTransfer (UInt64 blockSize, UInt64 *max);
    virtual IOReturn	reportMaxWriteTransfer(UInt64 blockSize, UInt64 *max);
    virtual IOReturn	reportMaxValidBlock(UInt64 *maxBlock);
    virtual IOReturn	reportMediaState(bool *mediaPresent, bool *changedState);
    virtual IOReturn	reportPollRequirements(bool *pollRequired, bool *pollIsExpensive);
    virtual IOReturn	reportRemovability(bool *isRemovable);
    virtual IOReturn	reportWriteProtection(bool *isWriteProtected);

private:
    EscaladeController	*controller;		// controller that holds us
    int			unitNumber;		// our unit number on the controller
    UInt32		unitSize;		// unit size in blocks
    char		unitDescription[80];	// product description string

    virtual void	setIcon(void);	// multiple icons quite possible
    
};


#endif // ESCALADEDRIVE_H