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
// EscaladeCommand
//
// Defines a command to be issued to the controller and all related
// housekeeping structures.
//

#ifndef ESCALADECOMMAND_H
#define ESCALADECOMMAND_H

class EscaladeController;

class EscaladeCommand : public IOCommand
{
    OSDeclareDefaultStructors(EscaladeCommand);
    
public:

    //
    // Factory method; creates a number command and inserts it into
    // the specified IOCommandPool.  
    //
    static EscaladeCommand *withAttributes(int withType, EscaladeController *withController, UInt8 withTag);

    //
    // Prepare/run the command.
    //
    virtual IOReturn	prepare(bool waiting);
    virtual void	complete(bool postSuccess = true);
    virtual bool	getResult(void);	// return values?
    virtual IOReturn	getCopyBuffer(void);
    virtual void	wasPosted(void);

    //
    // Command queueing
    //
    virtual void	_wasGotten(void);
    virtual void	returnCommand(void);
    virtual void	queueActive(void)   { controller->_queueActive(this); };
    virtual void	dequeueActive(void) { controller->_dequeueActive(this); };

    //
    // Command mutators
    //
    virtual void	makeInitConnection(UInt32 credits);
    virtual void	makeGetParam(int table, int param, size_t size);
    virtual void	makeSetParam(int table, int param, UInt8 value);
    virtual void	makeSetParam(int table, int param, UInt16 value);
    virtual void	makeSetParam(int table, int param, UInt32 value);
    virtual void	makeSetParam(int table, int param, void *vp, size_t vsize);
    virtual void	makeFlush(int unit);
    virtual void	makeReadWrite(int unit, IOMemoryDescriptor *buf, UInt32 block, UInt32 nblks,
			       IOStorageCompletion withCompletion);
    virtual void	makePowerSave(int state);	// 0 = spin down

    //
    // Timeout handling
    //
    virtual void	setTimeout(int seconds);
    virtual bool	isTimedOut(void);
    virtual bool	checkTimeout(void);
    
    //
    // Extract the results of a GetParam operation
    //
    virtual void	getParamResult(UInt8 *vp);
    virtual void	getParamResult(UInt16 *vp);
    virtual void	getParamResult(UInt32 *vp);
    virtual void	getParamResult(void *vp, size_t vsize);

    // public so reachable by controller
    IOPhysicalAddress	commandPhys;	// always contiguous
    int			commandType;	// pool from which the command came
    
    // public for UserClient
    TWE_Command		*commandPtr;
    virtual void	setDataBuffer(IOMemoryDescriptor *dp);
    virtual void	releaseDataBuffer(void);

    // superclass overrides
    virtual bool	init(int withType, EscaladeController *withController, UInt8 withTag);
    virtual void	free(void);

    // debugging
    virtual void	print(void);
    
private:
    virtual void	makeSetParam(int table, int param);
    virtual void	getParamBuffer(void);
    virtual void	releaseParamBuffer(void);
    virtual void	finishCopyBuffer(void);

    // permanent fields
    EscaladeController		*controller;
    IOBufferMemoryDescriptor	*command;
    UInt8			commandTag;

    // data buffers, set each time command is used
    IOMemoryDescriptor		*dataBuffer;
    IOBufferMemoryDescriptor	*paramBuffer;
    TWE_Param			*paramPtr;

    // temporary buffer, used to align unaligned requests
    IOBufferMemoryDescriptor	*copyBuffer;
    IOMemoryDescriptor		*realBuffer;

    // completion, set by ::makeReadWrite
    IOStorageCompletion		completion;

    // set in ::prepare
    bool			waitingOwner;
    UInt64			byteCount;	// for reporting success

    // set in ::complete
    bool			postResult;	// false if error in posting command

    // timeout handling
    int				timeoutCredits;	// set when the command is mutated
    bool			timedOut;	// have we timed out?
};

#endif // ESCALADECOMMAND_H
