# Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
#
# @APPLE_LICENSE_HEADER_START@
# 
# Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
# 
# This file contains Original Code and/or Modifications of Original Code
# as defined in and that are subject to the Apple Public Source License
# Version 2.0 (the 'License'). You may not use this file except in
# compliance with the License. Please obtain a copy of the License at
# http:\www.opensource.apple.com/apsl/ and read it before using this
# file.
# 
# The Original Code and all software distributed under the License are
# distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
# INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
# Please see the License for the specific language governing rights and
# limitations under the License.
# 
# @APPLE_LICENSE_HEADER_END@
#
# $Id$

##############################################################################
#
# 3ware Escalade FCode
#
##############################################################################
#
# Output files:
#
# Escalade.fc		TFTP-downloadable FCode tokenised driver
# 7xxx.fc		PCI ROM image component
#
# Input files:
#
# 7xxx.fth		PCI ROM wrapper
# Escalade.fth		Core driver code
#
#
# Targets:
#
# all			Make everything
# fcode			Generate tokenised FCode
# clean			Remove intermediate and final products
# install		Copies downloadable files to /private/tftpboot
#
##############################################################################

#
# The tokeniser
#
TOKE=	toke
#TOKE=	../toke/toke

#
# Where to install things
#
DESTDIR=/private/tftpboot

#
# The tokenised PCI Fcode image
#
FCODE=	7xxx.fc

##############################################################################
# Rules
#

all:	fcode
fcode:	Escalade.fc 7xxx.fc 

Escalade.fc:	Escalade.fth
	${TOKE} $<

7xxx.fc:	7xxx.fth Escalade.fth
	${TOKE} $<

install:	Escalade.fc
	cp Escalade.fth Escalade.fc ${DESTDIR}

clean:
	rm -f Escalade.fc 7xxx.fc *~
