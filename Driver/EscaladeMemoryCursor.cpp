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
// EscaladeMemoryCursor implementation
//

#include "Escalade.h"

// convenience
#define super	IOMemoryCursor
#define self	EscaladeMemoryCursor

OSDefineMetaClassAndStructors(self, super);

#define TWEMC_INVALID	(~(UInt32)0)

void
self::outputEscaladeSegment(PhysicalSegment segment,
			    void *segments,
			    UInt32 segmentIndex)
{
    TWE_SG_Entry	*sg;

    sg = (TWE_SG_Entry *)segments;
    // check alignment constraints
    if ((segment.location % TWE_ALIGNMENT) || (segment.length % TWE_SECTOR_SIZE)) {
	// no good, flag and ignore
	sg[0].address = TWEMC_INVALID;
    } else {
	sg[segmentIndex].address = OSSwapHostToLittleInt32(segment.location);
	sg[segmentIndex].length = OSSwapHostToLittleInt32(segment.length);
    }
}

EscaladeMemoryCursor *
self::factory(void)
{
    EscaladeMemoryCursor *cursor;

    if ((cursor = new self) == NULL)
	return(NULL);

    // initialise cursor
    if (!cursor->initWithSpecification(outputEscaladeSegment, 0xfffffe0)) {
	cursor->release();
	return(NULL);
    }
    return(cursor);
}

UInt32
self::getPhysicalSegments(IOMemoryDescriptor *descriptor,
			  TWE_SG_Entry *sglist,
			  IOByteCount *transferSize,
			  bool *copyRequired)
{
    UInt32	result;

    result = genPhysicalSegments(descriptor,
				 0,
				 (void *)sglist,
				 TWE_MAX_SGL_LENGTH,
				 0,
				 transferSize);
    if (sglist->address == TWEMC_INVALID) {
	*copyRequired = true;
    } else {
	*copyRequired = false;
    }
    return(result);
}

