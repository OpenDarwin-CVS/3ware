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
// UserClient class
//

#ifndef ESCALADEUSERCLIENT_H
#define ESCALADEUSERCLIENT_H

class EscaladeUserClient : public IOUserClient
{
    OSDeclareDefaultStructors(EscaladeUserClient);

protected:
    static const IOExternalMethod	sMethods[kEscaladeUserClientMethodCount];
    task_t		fTask;		// currently active user task
    EscaladeController	*fProvider;	// our provider
    void		*fSecurity;     // the user task's credentials

    // generic interface
    virtual bool	initWithTask(task_t owningTask, void *security_id, UInt32 type);
    virtual bool	start(IOService *provider);
    virtual IOExternalMethod *getTargetAndMethodForIndex(IOService **target, UInt32 index);
    virtual IOReturn	open(void);
    virtual IOReturn	close(void);
    virtual IOReturn	clientClose(void);
    virtual IOReturn	clientDied(void);
    
    // our work methods
    virtual IOReturn	getControllerArchitecture(int *outArchitecture);
    virtual IOReturn	getDriverVersion(int *outArchitecture);
    virtual IOReturn	doRemoveUnit(int inUnit);
    virtual IOReturn	doAddUnit(int inUnit);
    virtual IOReturn	doCommand(EscaladeUserCommand *inCommand, EscaladeUserCommand *outCommand,
			       IOByteCount inCount, IOByteCount *outCount);
    virtual IOReturn	getAEN(int *outAEN);
    virtual IOReturn	doReset(void);
};

#endif // ESCALADEUSERCLIENT
