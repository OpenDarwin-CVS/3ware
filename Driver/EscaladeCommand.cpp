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
// Implementation of the EscaladeCommand class.
//

// Master include - do not include other headers here.
#include "Escalade.h"

// convenience
#define super	IOCommand
#define self	EscaladeCommand

OSDefineMetaClassAndStructors(self, super);

static void	hexdump(void *p, int count);

////////////////////////////////////////////////////////////////////////////////
// Factory method to create a new EscaladeCommand and insert it into the
// specified IOCommandPool
//
EscaladeCommand *
self::withAttributes(int withType, EscaladeController *withController, UInt8 withTag)
{
    self	*command;
    
    if ((command = new self) == NULL)
	return(NULL);
	    
    if (!command->init(withType, withController, withTag)) {
	command->release();
	return(NULL);
    }
    return(command);
}

////////////////////////////////////////////////////////////////////////////////
// Initialise the command
//
bool
self::init(int withType, EscaladeController *withController, UInt8 withTag)
{
    IOByteCount junk;

    // superclass init
    if (!super::init())
	return(false);

    // save attributes
    commandType = withType;
    controller = withController;
    commandTag = withTag;
    
    // clear private fields
    command = NULL;
    commandPhys = 0;
    commandPtr = NULL;
    dataBuffer = NULL;
    paramBuffer = NULL;
    copyBuffer = NULL;
    realBuffer = NULL;

    // allocate buffer for command
    if ((command = IOBufferMemoryDescriptor::withOptions(kIODirectionOutIn |
							 kIOMemoryPhysicallyContiguous,
							 TWE_SECTOR_SIZE,
							 TWE_ALIGNMENT)) == NULL) {
	super::free();
	return(false);
    }
    commandPhys = command->getPhysicalSegment(0, &junk);
    commandPtr = (TWE_Command *)command->getBytesNoCopy();

    // Insert into pool
    returnCommand();

    return(true);
}

////////////////////////////////////////////////////////////////////////////////
// Tear the command down ready to be freed
//
void
self::free(void)
{

    // if we have a buffer for the command, drop it
    if (command != NULL)
	command->release();
    releaseParamBuffer();
    releaseDataBuffer();

    super::free();
}

////////////////////////////////////////////////////////////////////////////////
// The command has just been pulled off the free commands queue.
// Make sure it's tidy.
//
// Note that this is inefficient; we should not depend on these fields.
// We do this for safety/sanity's sake; optimisation would involve
// careful auditing of the use of these fields, better done once
// the driver is stable.
//
void
self::_wasGotten(void)
{
    dataBuffer = NULL;
    paramBuffer = NULL;
    paramPtr = NULL;
    copyBuffer = NULL;
    realBuffer = NULL;
    timeoutCredits = TWE_TIMEOUT_DEFAULT;
    timedOut = false;
}

////////////////////////////////////////////////////////////////////////////////
// The command has just been posted to the controller.
//
void
self::wasPosted(void)
{
    queueActive();
}
////////////////////////////////////////////////////////////////////////////////
// Return the command back to the free commands queue.
// Release any lingering resources.
//
void
self::returnCommand(void)
{
    if (dataBuffer != NULL)
	dataBuffer->release();
    if (paramBuffer != NULL)
	paramBuffer->release();
    if (copyBuffer != NULL)
	copyBuffer->release();
    if (realBuffer != NULL)
	realBuffer->release();

    controller->_returnCommand(this);
}

////////////////////////////////////////////////////////////////////////////////
// Data buffer handling
//
void
self::setDataBuffer(IOMemoryDescriptor *dp)
{
    if (dp != NULL)
	dp->retain();
    dataBuffer = dp;
}

void
self::releaseDataBuffer(void)
{
    if (dataBuffer != NULL) {
	dataBuffer->release();
	dataBuffer = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Parameter buffer handling
//
// The parameter buffer is handled automatically internally.  When a parameter
// is set, the buffer is freed on completion of the command.  When a paraemter
// is fetched, the buffer is freed when the value is collected.
//
void
self::getParamBuffer(void)
{
    if (paramBuffer == NULL)
	paramBuffer = IOBufferMemoryDescriptor::withOptions(kIODirectionOutIn |
						     kIOMemoryPhysicallyContiguous,
						     TWE_SECTOR_SIZE,
						     TWE_ALIGNMENT);
    paramPtr = (TWE_Param *)paramBuffer->getBytesNoCopy();

    bzero(paramPtr, TWE_SECTOR_SIZE);    // XXX necessary?
    setDataBuffer(paramBuffer);
    paramBuffer->prepare(kIODirectionOutIn);
}

void
self::releaseParamBuffer(void)
{
    if (paramBuffer != NULL) {
	paramBuffer->release();
	paramBuffer = NULL;
	paramPtr = NULL;
	setDataBuffer(NULL);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Copy buffer handling
//
// The 'real' IOMemoryDescriptor isn't suitable for the controller.
// Allocate a temporary buffer that is.
//
// Note that this is a great way to run the kernel out of VM.  Typically
// transfers requiring copy buffers are small; enforce that assumption here
// to prevent a DoS.
//
// XXX A cleaner way to do this would of course be to perform the transfer
//     in smaller chunks.
IOReturn
self::getCopyBuffer(void)
{
    IOByteCount	size, szFrag;
    IODirection	dir;
    void	*copyPtr;

    // fetch characteristics of the original transfer
    size = dataBuffer->getLength();
    dir = dataBuffer->getDirection();

    // make sure we're OK with this
    if (size > TWE_MAX_COPYBUFFER) {
	error("unaligned request size %u too large", (uint)size);
	return(kIOReturnNoMemory);
    }

    // save the real buffer
    realBuffer = dataBuffer;

    // size must be a multiple of TWE_SECTOR_SIZE
    if ((szFrag = (size % TWE_SECTOR_SIZE)) != 0)
	size += TWE_SECTOR_SIZE - szFrag;

    // get a temporary buffer with the same size/direction but correct alignment
    copyBuffer = IOBufferMemoryDescriptor::withOptions(dir, size, TWE_ALIGNMENT);
    if (copyBuffer == NULL) {
	error("unable to allocate copy buffer");
	return(kIOReturnNoMemory);		// despite sanity, still can't get it
    }
    copyPtr = copyBuffer->getBytesNoCopy();

    // if data is outbound, we need to copy now
    if (dir & kIODirectionOut)
	realBuffer->readBytes(0, copyPtr, size);

    // make this the buffer for the command
    dataBuffer = copyBuffer;
    dataBuffer->prepare(dir);

    return(kIOReturnSuccess);
}

void
self::finishCopyBuffer(void)
{
    IOByteCount	size;
    IODirection	dir;
    void	*copyPtr;

    dir = realBuffer->getDirection();
    if (dir & kIODirectionIn) {
	size = realBuffer->getLength();
	copyPtr = copyBuffer->getBytesNoCopy();
	realBuffer->writeBytes(0, copyPtr, size);
    }

    dataBuffer = realBuffer;
    realBuffer = NULL;
    copyBuffer->release();
    copyBuffer = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Prepare a command for submission:
//
// - fix up the command structure
// - populate the S/G list (if required)
// - prepare the memory descriptors
//
IOReturn
self::prepare(bool waiting)
{
    TWE_SG_Entry *sg;
    UInt32	sgcount, size;
    int		sgoff;
    bool	needCopy;

    // If this flag is set on completion, we will wake up the owner of
    // the command.
    waitingOwner = waiting;
    
    // prepare memory descriptor for command
    command->prepare(kIODirectionOutIn);

    // fix up command, fill in s/g list
    commandPtr->generic.request_id = commandTag;
    sgoff = commandPtr->generic.opcode >> 5;

    if ((sgoff != 0) && (dataBuffer != NULL)) {	// command has scatter/gather list & data
	// find scatter/gather list
	sg = (TWE_SG_Entry *)((UInt32 *)commandPtr + sgoff);

	// get physical segments, check whether to bounce
	sgcount = controller->memoryCursor->getPhysicalSegments(dataBuffer,
							 sg,
							 &size,
							 &needCopy);
	// if the buffer isn't legally aligned, bail
	if (needCopy)
	    return(kIOReturnNotAligned);

	// fix up command size
	commandPtr->generic.size += 2 * sgcount;
	sg[sgcount].address = sg[sgcount].length = 0;	// terminate debug output cleanly
	byteCount = size;
    }
//    print();

    return(kIOReturnSuccess);
}

////////////////////////////////////////////////////////////////////////////////
// Handle completion of a command
//
// Note that this may be called either inside or outside the gate.
void
self::complete(bool postSuccess)
{
    IOStorageCompletion localCompletion;
    IOByteCount	actualByteCount;
    bool	result;

    postResult = postSuccess;
    
    command->complete(kIODirectionOutIn);
    dequeueActive();
    
    // auto-complete if we have a parameter buffer
    if (paramBuffer != NULL)
	paramBuffer->complete(kIODirectionOutIn);

    // if we have a copy buffer, finalise it
    if (copyBuffer != NULL)
	finishCopyBuffer();

    // data buffers for SET_PARAM auto-release here
    if (commandPtr->generic.opcode == TWE_OP_SET_PARAM)
	releaseParamBuffer();

    // Wake up a sleeping owner.
    // Owner is responsible for returning the command.
    if (waitingOwner) {
	controller->wakeCommand(this);
    } else {
	// save because we are going to release command first
	localCompletion = completion;
	actualByteCount = byteCount;
	result = getResult();
	if (!result) {
	    debug(2, "I/O error 0x%02x/ox%02x", (uint)commandPtr->generic.status, (uint)commandPtr->generic.flags);
	    print();
	}
	// Command submitter is gone, completion may know nothing
	// about our command, so we have to tear it down it ourselves.
	releaseDataBuffer();
	returnCommand();

	// Invoke completion
	IOStorage::complete(localCompletion,
		     result ? kIOReturnSuccess : kIOReturnError,
		     result ? byteCount : 0);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Fetch the controller status code(s) from a command.
//
// Returns true if the command succeeded, false if not.
//
// XXX It might be nice to be more verbose about why things failed.
//
bool
self::getResult(void)
{

    if (postResult == false)
	return(false);
    if (timedOut)
	return(false);
    
    if (commandPtr->generic.status == TWE_STATUS_RESET) {
	error("command status requests controller reset");
	controller->requestReset();
	return(false);
    }
    if ((commandPtr->generic.status >> 4) == TWE_STATUS_FATAL) {
	debug(2, "FATAL status 0x%02x  flags 0x%02x",
	       (uint)commandPtr->generic.status, (uint)commandPtr->generic.flags);
	return(false);
    }
    return(true);
}

////////////////////////////////////////////////////////////////////////////////
// Command constructors
//

void
self::makeInitConnection(UInt32 credits)
{
    commandPtr->initconnection.opcode = TWE_OP_INIT_CONNECTION;
    commandPtr->initconnection.size = 3;
    commandPtr->initconnection.host_id = 0;
    commandPtr->initconnection.status = 0;
    commandPtr->initconnection.flags = 0;
    commandPtr->initconnection.message_credits = OSSwapHostToLittleInt16(credits);
    commandPtr->initconnection.response_queue_pointer = 0;
}

void
self::makeGetParam(int table, int param, size_t size)
{
    commandPtr->param.opcode = TWE_OP_GET_PARAM;
    commandPtr->param.size = 2;
    commandPtr->param.host_unit = 0;
    commandPtr->param.status = 0;
    commandPtr->param.flags = 0;
    commandPtr->param.param_count = OSSwapHostToLittleInt16(1);

    getParamBuffer();
    paramPtr->table_id = OSSwapHostToLittleInt16(table);
    paramPtr->parameter_id = param;
    paramPtr->parameter_size_bytes = size;
}

void
self::makeSetParam(int table, int param)
{

    commandPtr->param.opcode = TWE_OP_SET_PARAM;
    commandPtr->param.size = 2;
    commandPtr->param.host_unit = 0;
    commandPtr->param.status = 0;
    commandPtr->param.flags = 0;
    commandPtr->param.param_count = OSSwapHostToLittleInt16(1);

    getParamBuffer();
    paramPtr->table_id = OSSwapHostToLittleInt16(table);
    paramPtr->parameter_id = param;
}

void
self::makeSetParam(int table, int param, UInt8 value)
{

    makeSetParam(table, param);
    paramPtr->parameter_size_bytes = 1;
    *(UInt8 *)(&paramPtr->data[0]) = value;
}

void
self::makeSetParam(int table, int param, UInt16 value)
{

    makeSetParam(table, param);
    paramPtr->parameter_size_bytes = 2;
    *(UInt8 *)(&paramPtr->data[0]) = OSSwapHostToLittleInt16(value);
}

void
self::makeSetParam(int table, int param, UInt32 value)
{

    makeSetParam(table, param);
    paramPtr->parameter_size_bytes = 4;
    *(UInt8 *)(&paramPtr->data[0]) = OSSwapHostToLittleInt32(value);
}

void
self::makeSetParam(int table, int param, void *vp, size_t vsize)
{

    makeSetParam(table, param);
    paramPtr->parameter_size_bytes = vsize;
    bcopy(vp, &paramPtr->data[0], vsize);
}

////////////////////////////////////////////////////////////////////////////////
// Get the results of a GetParam operation
//
void
self::getParamResult(UInt8 *vp)
{
    *vp = *(UInt8 *)(&paramPtr->data[0]);
    releaseParamBuffer();
}

void
self::getParamResult(UInt16 *vp)
{
    *vp = OSSwapLittleToHostInt16(*(UInt16 *)(&paramPtr->data[0]));
    releaseParamBuffer();
}

void
self::getParamResult(UInt32 *vp)
{
    *vp = OSSwapLittleToHostInt32(*(UInt32 *)(&paramPtr->data[0]));
    releaseParamBuffer();
}

void
self::getParamResult(void *vp, size_t vsize)
{
    bcopy(&paramPtr->data[0], vp, vsize);
    releaseParamBuffer();
}

////////////////////////////////////////////////////////////////////////////////
// Build a FLUSHUNIT command
void
self::makeFlush(int unit)
{
    commandPtr->generic.opcode = TWE_OP_FLUSH;
    commandPtr->generic.size = 2;
    commandPtr->generic.host_unit = unit;
    commandPtr->generic.status = 0;
    commandPtr->generic.flags = 0;
    commandPtr->generic.count = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Build an I/O command
void
self::makeReadWrite(int unit,
		    IOMemoryDescriptor *buf,
		    UInt32 block,
		    UInt32 nblks,
		    IOStorageCompletion withCompletion)
{

    // select opcode
    if (buf->getDirection() == kIODirectionOut) {
	commandPtr->io.opcode = TWE_OP_WRITE;
    } else {
	commandPtr->io.opcode = TWE_OP_READ;
    }
    commandPtr->io.size = 3;
    commandPtr->io.host_unit = unit;
    commandPtr->io.status = 0;
    commandPtr->io.flags = 0;
    commandPtr->io.block_count = OSSwapHostToLittleInt16((UInt16)nblks);
    commandPtr->io.lba = OSSwapHostToLittleInt32(block);
    completion = withCompletion;
    setDataBuffer(buf);
}

////////////////////////////////////////////////////////////////////////////////
// Build a power-save command
void
self::makePowerSave(int state)
{
    commandPtr->generic.opcode = TWE_OP_EXTENDED;
    commandPtr->generic.size = 2;
    commandPtr->generic.host_unit = 0;
    commandPtr->generic.status = 0;
    commandPtr->generic.flags = TWE_OPEX_POWERMGMT;
    commandPtr->generic.count = OSSwapHostToLittleInt16(state ? 1 : 0);
}

////////////////////////////////////////////////////////////////////////////////
// Timeout handling
//
// This is very simple; we grant a command one credit per second allowed, and
// then we check the queue of active commands once every TWE_TIMEOUT_RATE
// seconds.  If a command's credits fall to zero, it has been in the
// queue too long.
//
// This is not precise, but with our timeouts a few seconds either way is
// not significant.
//
void
self::setTimeout(int seconds)
{
    timeoutCredits = seconds + 1;
}

bool
self::isTimedOut(void)
{
    return(timedOut);
}

bool
self::checkTimeout(void)
{
    timeoutCredits -= TWE_TIMEOUT_COST;
    if(timeoutCredits <= 0)
	timedOut = true;
    return(timedOut);
}

////////////////////////////////////////////////////////////////////////////////
// debugging
void
self::print(void)
{
    int i;
    
    debug(0, "COMMAND %d/%p", commandTag, (void *)this);

    debug(0, "  request_id %d  opcode 0x%x  size %d  host_unit %d",
	  commandPtr->generic.request_id, commandPtr->generic.opcode, commandPtr->generic.size,
	  commandPtr->generic.host_unit);
    debug(0, "  status %d  flags 0x%x  count %d",
	  commandPtr->generic.status, commandPtr->generic.flags, OSSwapLittleToHostInt16(commandPtr->generic.count));

    debug(0, "  command buffer:");
    hexdump(commandPtr, 32);
    
    switch(commandPtr->generic.opcode) {	/* XXX add more opcodes? */
	case TWE_OP_READ:
	case TWE_OP_WRITE:
	    debug(0, " lba %u", (uint)OSSwapLittleToHostInt32(commandPtr->io.lba));
	    for (i = 0; (i < TWE_MAX_SGL_LENGTH) && (commandPtr->io.sgl[i].length != 0); i++)
		debug(0, "  %d: 0x%x/%d", i,
	(uint)OSSwapLittleToHostInt32(commandPtr->io.sgl[i].address),
	(uint)OSSwapLittleToHostInt32(commandPtr->io.sgl[i].length));
		break;

	case TWE_OP_GET_PARAM:
	case TWE_OP_SET_PARAM:
	    for (i = 0; (i < TWE_MAX_SGL_LENGTH) && (commandPtr->param.sgl[i].length != 0); i++)
		debug(0, "  %d: 0x%x/%d", i,
	(uint)OSSwapLittleToHostInt32(commandPtr->param.sgl[i].address),
	(uint)OSSwapLittleToHostInt32(commandPtr->param.sgl[i].length));
	    debug(0, "  table 0x%04x param 0x%02x size %d",
	   (uint)OSSwapLittleToHostInt16(paramPtr->table_id),
	   (uint)paramPtr->parameter_id, (uint)paramPtr->parameter_size_bytes);
	    debug(0, "  parameter buffer:");
	    hexdump(paramPtr, 16);
	    break;
    
	case TWE_OP_INIT_CONNECTION:
	    debug(0, " response queue pointer 0x%x", commandPtr->initconnection.response_queue_pointer);
	    break;

	default:
	    break;
    }
}

static void
hexdump(void *vp, int count)
{
    UInt8	*p = (UInt8 *)vp;
    int		i, j;

    for (j = 0; j < count; j += 16) {
	IOLog("    %04x:", j);
	for (i = j; (i < (j + 16)) && (i < count); i++)
	    IOLog(" %02x", (uint)*(p + i));
	IOLog("\n");
    }
}
