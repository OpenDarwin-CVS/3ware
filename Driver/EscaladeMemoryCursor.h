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
// The Escalade controller requires all data to be 512-aligned and 512-multiple sized.
// To enforce this, we need a subclass of IOMemoryCursor
//

#ifndef ESCALADEMEMORYCURSOR_H
#define ESCALADEMEMORYCURSOR_H

class EscaladeMemoryCursor : public IOMemoryCursor
{
    OSDeclareDefaultStructors(EscaladeMemoryCursor);

public:
    // create a new cursor
    static EscaladeMemoryCursor	*factory(void);

    // Generate a scatter/gather list in appropriate format.
    // Sets copyRequired if the list is not valid for the controller.
    virtual UInt32	getPhysicalSegments(IOMemoryDescriptor *descriptor,
				       TWE_SG_Entry *sglist,
				       IOByteCount *transferSize,
				       bool *copyRequired);

    static void	outputEscaladeSegment(PhysicalSegment segment,
				      void *segments,
				      UInt32 segmentIndex);
};

#endif ESCALADEMEMORYCURSOR_H