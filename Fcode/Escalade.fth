\ 
\ Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
\
\ @APPLE_LICENSE_HEADER_START@
\ 
\ Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
\ 
\ This file contains Original Code and/or Modifications of Original Code
\ as defined in and that are subject to the Apple Public Source License
\ Version 2.0 (the 'License'). You may not use this file except in
\ compliance with the License. Please obtain a copy of the License at
\ http:\www.opensource.apple.com/apsl/ and read it before using this
\ file.
\ 
\ The Original Code and all software distributed under the License are
\ distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
\ EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
\ INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
\ FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
\ Please see the License for the specific language governing rights and
\ limitations under the License.
\ 
\ @APPLE_LICENSE_HEADER_END@
\
\ $Id$

\
\ 3ware Escalade FCode driver for Apple Macintosh systems.
\

\
\ Code conventions:
\
\ Variable names are prefixed to identify their types:
\	# - constant
\	* - pointer
\	? - boolean
\	  - numeric value
\ Word names are postfixed to classify their behaviour:
\	! - stores to a location
\	@ - fetches from a location
\	? - tests true/false
\	. - prints something
\ Indent is two spaces. 

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Some basic properties
\

" 3ware,Escalade"	device-name
" ata" 			device-type	\ must be " ata" or perhaps " scsi"
" Escalade"		model

\ Our unit numbers are a single digit
1 encode-int " #address-cells" property

\ Children have no 'reg' property
0 encode-int " #size-cells" property

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Constants
\

hex
  
\ tunables
0 constant ?debug
d# 256 constant #print-buffer-allocation

\ PCI constants
10 constant #BAR-address	\ PCI config space address of the BAR of interest
10 constant #BAR-window		\ Size of region to map

\ control register constants
00080000 constant #CR-clear-host-interrupt
00040000 constant #CR-clear-attention-interrupt
00020000 constant #CR-mask-command-interrupt
00010000 constant #CR-mask-response-interrupt
00008000 constant #CR-unmask-command-interrupt
00004000 constant #CR-unmask-response-interrupt
00000200 constant #CR-clear-error-status
00000100 constant #CR-issue-soft-reset
00000080 constant #CR-enable-interrupts
00000040 constant #CR-disable-interrupts
00800000 constant #CR-clear-parity-error
00100000 constant #CR-clear-pci-abort

\ status register constants
f0000000 constant #SR-major-version-mask
0f000000 constant #SR-minor-version-mask
00800000 constant #SR-pci-parity-error
00400000 constant #SR-queue-error
00200000 constant #SR-microcontroller-error
00100000 constant #SR-pci-abort
00080000 constant #SR-host-interrupt
00040000 constant #SR-attention-interrupt
00020000 constant #SR-command-interrupt
00010000 constant #SR-response-interrupt
00008000 constant #SR-command-queue-full
00004000 constant #SR-response-queue-empty
00002000 constant #SR-microcontroller-ready
00001000 constant #SR-command-queue-empty

\ misc controller constants
200 constant #buffer-alignment
200 constant #sector-size
ff constant #initial-message-credits
01 constant #shutdown-message-credits
17 constant #command-tag
16 constant #max-units

\ command opcodes
00 constant #CMD-initconnection
62 constant #CMD-read
63 constant #CMD-write
52 constant #CMD-get-param
53 constant #CMD-set-param
56 constant #CMD-download
5b constant #CMD-print

\ command buffer offsets
00 constant #CMD-offset-opcode
01 constant #CMD-offset-size
02 constant #CMD-offset-request-id
03 constant #CMD-offset-unit
04 constant #CMD-offset-status
06 constant #CMD-offset-count
08 constant #CMD-offset-lba

\ I/O command offsets
08 constant #CMD-offset-io-lba
0c constant #CMD-offset-io-sgl

\ get/set-param offsets
08 constant #CMD-offset-param-sgl

\ command sizes
03 constant #CMD-size-initconnection
04 constant #CMD-size-get-param
04 constant #CMD-size-print
04 constant #CMD-size-download
05 constant #CMD-size-read

\ get/set param opcodes
300 constant #PARAM-unitinfo			\ UNITINFO table
  3 constant #PARAM-unitinfo-status		\ status entry
  1 constant #PARAM-unitinfo-status-online	\ unit-is-online bit
  4 constant #PARAM-unitinfo-capacity		\ capacity in blocks

\ get/set param constants
0 constant #PARAM-offset-table
2 constant #PARAM-offset-param
3 constant #PARAM-offset-size
4 constant #PARAM-offset-data

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Variables
\

-1 value *control-reg-addr
-1 value *status-reg-addr
-1 value *command-reg-addr
-1 value *response-reg-addr

\ allocated buffers
-1 value *command-buffer-alloced
-1 value *param-buffer-alloced

\ actual buffer addresses to be used
-1 value *command-buffer
-1 value command-buffer-phys
-1 value *param-buffer
-1 value param-buffer-phys

\ debug printer state
0 value print-buffer-length
-1 value *print-buffer

\ controller reset one-shot
false value ?controller-reset-done

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
headers
\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Upcalls
\
: config-w@ ( offset -- w ) my-space + " config-w@" $call-parent ;
: config-w! ( w offset -- ) my-space + " config-w!" $call-parent ;

: map-in ( phys.lo ... phys.hi size -- virt )  " map-in" $call-parent ;
: map-out ( virt size -- )  " map-out" $call-parent ;

: dma-alloc ( size -- vaddr )  " dma-alloc" $call-parent ;
: dma-free  ( vaddr size -- )  " dma-free"  $call-parent ;
: dma-map-in ( vaddr size cacheable? -- devaddr )  " dma-map-in" $call-parent ;
: dma-map-out ( vaddr devaddr size -- )  " dma-map-out" $call-parent ;
: dma-sync ( vaddr devaddr size -- )  " dma-sync" $call-parent ;

: command-set ( cmd -- ) 4 dup >r config-w@ or r> config-w! ;
  
\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Utility routines
\

: copyright ( -- )
  cr
  ." 3ware Escalade 6/7/8xxx boot firmware" cr
  ." Copyright (c) 2003 Michael Smith, all rights reserved." cr
  cr
;

\ Determine whether bits are set in a word
: bits-set? ( word bits -- bits-set? )
  dup				( word bits bits )
  rot				( bits bits word )
  and				( bits set-bits )
  =
;

\ Determine whether bits are clear in a word
: bits-clear? ( word bits -- bits-clear? )
  and				( set-bits )
  0=
;

\ Endian conversion
\ It would be good to test little-endian? but there's no fcode token
\ for it, so we'd have to implement our own version - tedious.
: host-to-little-32 ( HE32 -- LE32 )  lbflip ;
: little-to-host-32 ( LE32 -- HE32 )  lbflip ;
: host-to-little-16 ( HE16 -- LE16 )  h# 0000ffff and wbflip ;
: little-to-host-16 ( LE16 -- HE16 )  h# 0000ffff and wbflip ;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Mapping and allocation
\

\ Find the controller's physical base address
: escalade-phys-base@ ( -- phys-addr )
  " assigned-addresses" get-my-property		( prop-addr prop-len )
  if
    ." no assigned-addresses"  cr
    true throw
  then
  begin
    dup						\ search for memory space
  while decode-phys				( prop-adr prop-len phys-lo phys-mid phys-hi )
    dup h# FF and #BAR-address =		\ this the entry we want?
    if
      2drop >r 2drop r> exit
    else
      3drop					\ phys lo/mid/hi
      decode-int drop decode-int drop		\ size values
    then
  repeat
  2drop						\ didn't find it
  true throw					\ failed
;

\ map the controller's 16-byte window
: map-escalade-registers ( -- )
  escalade-phys-base@
  \ map our register space
  \ 0 my-space h# 01000000 or #BAR-address or #BAR-window map-in to *control-reg-addr
  0 my-space h# 01000010 or h# 10 map-in to *control-reg-addr
  \ enable I/O access
  1 command-set
  
  \ generate addresses for the other registers
  *control-reg-addr			( control-reg-addr )
  4 + dup to *status-reg-addr
  4 + dup to *command-reg-addr
  4 + to *response-reg-addr

  \ XXX testing
  ?debug if
    ." control:"  *control-reg-addr  .h
    ." status:"   *status-reg-addr   .h
    ." command:"  *command-reg-addr  .h
    ." response:" *response-reg-addr .h cr
  then
;

: unmap-escalade-registers ( -- )
    *control-reg-addr #BAR-window map-out
;

\ Allocate an aligned, dma-able buffer, return the address of the allocated memory
\ and the aligned address (which may not be the same).
: dma-alloc-aligned ( align size -- alloc-addr aligned-addr )
  2 * dma-alloc				( align virt )
  2dup swap mod				( align virt offset )
  0= if
    nip dup				( virt virt )
  else
    rot swap - +			( virt aligned )
  then
;

\ Allocate and map DMA buffers for the controller
: allocate-dma-buffers ( -- )
  
  #buffer-alignment #sector-size dma-alloc-aligned to *command-buffer to *command-buffer-alloced
  *command-buffer #sector-size true dma-map-in to command-buffer-phys

  #buffer-alignment #sector-size dma-alloc-aligned to *param-buffer to *param-buffer-alloced
  *param-buffer #sector-size true dma-map-in to param-buffer-phys

  #print-buffer-allocation alloc-mem to *print-buffer
  
  ?debug if
    ." command buffer " *command-buffer-alloced .h *command-buffer .h command-buffer-phys .h cr
    ." param buffer " *param-buffer-alloced .h *param-buffer .h param-buffer-phys .h cr
  then  
;

\ Free allocated DMA buffers
: free-dma-buffers ( -- )
  *command-buffer command-buffer-phys #sector-size dma-map-out
  *param-buffer param-buffer-phys #sector-size dma-map-out
;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Controller register accessors
\
: control-reg!  ( w -- ) *control-reg-addr  rl! ;
: status-reg@   ( -- w ) *status-reg-addr   rl@ ;
: command-reg!  ( w -- ) *command-reg-addr  rl! ;
: response-reg@ ( -- w ) *response-reg-addr rl@ ;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Controller primitives
\

\ Throw an exception if the controller is showing an error status
: check-controller-errors ( -- )
  status-reg@
  #SR-pci-abort
  #SR-pci-parity-error	or
  #SR-queue-error	or
  bits-set? 						( error-bits? )

  status-reg@ #SR-microcontroller-ready bits-set?	( error-bits? microcontroller-ready? )

  and 0<> if
    ." unexpected status error" cr
    true throw
  then
;

\ Wait (timeout) seconds for all of (bits) to be set in the status register.
\ Returns false if bits set, true if timed out.
: wait-for-status ( bits timeout -- timed-out? )
  d# 1000 * get-msecs +			( bits end-time )
  begin
    dup get-msecs >
  while					( bits end-time )
    over status-reg@ swap		( bits end-time status-reg bits )
    bits-set? if
      2drop false exit			\ bits set, return success
    then
  repeat
  2drop
  true
;

\ Wait (timeout) seconds for all of (bits) to be cleared in the status register.
\ Returns false if bits set, true if timed out.
: wait-for-unstatus ( bits timeout -- timed-out? )
  d# 1000 * get-msecs +			( bits end-time )
  begin
    dup get-msecs >
  while					( bits end-time )
    over status-reg@ swap		( bits end-time status-reg bits )
    bits-clear? if
      2drop false exit			\ bits set, return success
    then
  repeat
  2drop
  true
;

\ Issue a soft reset to the controller.
: reset-controller ( -- )
  ?controller-reset-done if exit then
  true to ?controller-reset-done
  
  \ build the soft-reset word
  #CR-issue-soft-reset
  #CR-clear-host-interrupt	or
  #CR-clear-attention-interrupt	or
  #CR-mask-command-interrupt	or
  #CR-mask-response-interrupt	or
  #CR-clear-error-status	or
  #CR-disable-interrupts	or	control-reg!

  \ now wait 30 seconds for the controller to respond
  #SR-attention-interrupt d# 30 wait-for-status if
    ." controller reset timed out" cr
    true throw
  then

  \ clear the attention interrupt that the soft-reset caused
  #CR-clear-attention-interrupt control-reg!
;

\ Drain the response queue
: drain-response-queue ( -- )
  \ spin pulling response keys off the queue to ensure it's empty
  d# 1000 0 do
    status-reg@ #SR-response-queue-empty bits-set? if
      exit
    then
  loop
  ." could not drain response queue" cr
  true throw
;

\ Check a supplied unit number
\ XXX we could be more extensive here...
: unit-number-invalid? ( unit -- invalid? )
  #max-units >= if
    true
  else
    false
  then
;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Command creation primitives
\

\ dma-sync buffers
: command-dma-sync ( -- )  *command-buffer command-buffer-phys #sector-size dma-sync ;
: param-dma-sync   ( -- )  *param-buffer param-buffer-phys #sector-size dma-sync ;

\ clean buffers
: command-clear-buffer ( -- )  *command-buffer #sector-size 0 fill ;
: param-clear-buffer   ( -- )  *param-buffer #sector-size 0 fill ;

\ Command buffer field accessors
: command-opcode! ( opcode -- )  *command-buffer #CMD-offset-opcode + c! ;
: command-opcode@ ( -- opcode )  *command-buffer #CMD-offset-opcode + c@ ;
: command-size!   ( size -- )    *command-buffer #CMD-offset-size + c! ;
: command-size@   ( -- size )    *command-buffer #CMD-offset-size + c@ ;
: command-unit!   ( unit -- )    *command-buffer #CMD-offset-unit + c! ;
: command-unit@   ( -- unit )    *command-buffer #CMD-offset-unit + c@ ;
: command-tag!    ( tag -- )     *command-buffer #CMD-offset-request-id + c! ;
: command-tag@    ( -- tag )     *command-buffer #CMD-offset-request-id + c@ ;
: command-count!  ( count -- )   host-to-little-16 *command-buffer #CMD-offset-count + w! ;
: command-count@  ( -- count )   *command-buffer #CMD-offset-count + w@ little-to-host-16 ;
: command-lba!    ( lba -- )     host-to-little-32 *command-buffer #CMD-offset-lba + l! ;
: command-lba@    ( -- lba )     *command-buffer #CMD-offset-lba + l@ little-to-host-32 ;
: command-status@ ( -- status )  *command-buffer #CMD-offset-status + c@ ;

\ Parameter buffer field accessors
: param-table!  ( table -- )  host-to-little-16 *param-buffer #PARAM-offset-table + w! ;
: param-table@  ( -- table )  *param-buffer #PARAM-offset-table + w@ little-to-host-16 ;
: param-param!  ( param -- )  *param-buffer #PARAM-offset-param + c! ;
: param-param@  ( -- param )  *param-buffer #PARAM-offset-param + c@ ;
: param-size!   ( size -- )   *param-buffer #PARAM-offset-size + c! ;
: param-size@   ( -- size )   *param-buffer #PARAM-offset-size + c@ ;
: param-data-b! ( datum -- )  *param-buffer #PARAM-offset-data + c! ;
: param-data-b@ ( -- datum )  *param-buffer #PARAM-offset-data + c@ ;
: param-data-w! ( datum -- )  host-to-little-16 *param-buffer #PARAM-offset-data + w! ;
: param-data-w@ ( -- datum )  *param-buffer #PARAM-offset-data + w@ little-to-host-16 ;
: param-data-l! ( datum -- )  host-to-little-32 *param-buffer #PARAM-offset-data + l! ;
: param-data-l@ ( -- datum )  *param-buffer #PARAM-offset-data + l@ little-to-host-32 ;

\ Set up the parameter buffer to handle the given parameter
: param-setup-buffer ( table param size -- )
  param-clear-buffer
  param-size! param-param! param-table!
  param-dma-sync
;

\ Attach the parameter buffer to the command
: command-param-sgl! ( -- )
  param-buffer-phys host-to-little-32			( param-buffer-phys-LE )
  *command-buffer #CMD-offset-param-sgl + l!		\ store sgl entry address
  #sector-size host-to-little-32			( param-buffer-size-LE )
  *command-buffer #CMD-offset-param-sgl + 4 + l!	\ store sgl entry length
;

: command-param-sgl@ ( -- sgl-length sgl-address )
  *command-buffer #CMD-offset-param-sgl + 4 + l@ little-to-host-32	( sgl-length )
  *command-buffer #CMD-offset-param-sgl + l@ little-to-host-32 	( sgl-length sgl-address )
;

\ Attach an I/O buffer to the command
: command-io-sgl! ( physaddr length -- )
  host-to-little-32 *command-buffer #CMD-offset-io-sgl + 4 + l!	( physaddr )
  host-to-little-32 *command-buffer #CMD-offset-io-sgl + l!
;

: command-io-sgl@ ( -- sgl-length sgl-address)
  *command-buffer #CMD-offset-io-sgl + 4 + l@ little-to-host-32	( sgl-length )
  *command-buffer #CMD-offset-io-sgl + l@ little-to-host-32 		( sgl-length sgl-address )
;

\ Make an INITCONNECTION command
: command-make-initconnection ( credits -- )
  command-clear-buffer
  #CMD-initconnection command-opcode!
  #CMD-size-initconnection command-size!
  command-count!
;

\ Make a generic GET_PARAM command to fetch one parameter
: command-make-getparam ( -- )
  command-clear-buffer
  #CMD-get-param command-opcode!		\ opcode
  #CMD-size-get-param command-size!		\ size
  1 command-count!				\ parameter count
  command-param-sgl!				\ scatter/gather
;
  

\ Make a GET_PARAM/UNITINFO/status command
: command-make-unitinfo-status ( unit -- )
  \ set up the param buffer
  #PARAM-unitinfo + #PARAM-unitinfo-status 1 param-setup-buffer

  \ build the command
  command-make-getparam
;

\ Make a GET_PARAM/UNITINFO/capacity command
: command-make-unitinfo-capacity ( unit -- )
  \ set up the param buffer
  #PARAM-unitinfo + #PARAM-unitinfo-capacity 4 param-setup-buffer

  \ build the command
  command-make-getparam
;

\ Make a READ command
: command-make-read ( physaddr byte-count unit-number block-count block-number -- )
  command-clear-buffer
  #CMD-read command-opcode!
  #CMD-size-read command-size!
  command-lba!
  command-count!
  command-unit!
  command-io-sgl!
;

\ Make a REMOTEPRINT command
\ We hijack this onto the parameter buffer
: command-make-print ( msgaddr msglength -- )
  command-clear-buffer
  param-clear-buffer
  *print-buffer *param-buffer print-buffer-length move
  param-dma-sync
  0 to print-buffer-length
  #CMD-print command-opcode!
  #CMD-size-print command-size!
  command-param-sgl!
;

\ Make a DOWNLOADFIRMWARE command
: command-make-download ( imagephys imagesize -- )
  command-clear-buffer
  #CMD-download command-opcode!
  #CMD-size-download command-size!
  8 command-count!
  command-io-sgl!
;  

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Debugging routines
\

: .dump16 ( address -- )
  ."    " dup 8 u.r ." -"
  d# 16 0 do
    dup i + c@ 2 u.r ."  "
  loop
  cr drop
;

: .status-reg ( status-reg-value -- )
  ." status:"
  dup #SR-pci-parity-error	bits-set? if ." PCI-parity-error "	then
  dup #SR-queue-error		bits-set? if ." queue-error "		then
  dup #SR-microcontroller-error	bits-set? if ." microcontroller-error "	then
  dup #SR-pci-abort		bits-set? if ." pci-abort "		then
  dup #SR-host-interrupt	bits-set? if ." host-interrupt "	then
  dup #SR-attention-interrupt	bits-set? if ." attention-interrupt "	then
  dup #SR-command-interrupt	bits-set? if ." command-interrupt "	then
  dup #SR-response-interrupt	bits-set? if ." response-interrupt "	then
  dup #SR-command-queue-full	bits-set? if ." command-queue-full "	then
  dup #SR-response-queue-empty	bits-set? if ." response-queue-empty "	then
  dup #SR-microcontroller-ready	bits-set? if ." microcontroller-ready "	then
  dup #SR-command-queue-empty	bits-set? if ." command-queue-empty "	then
  .h cr
;

: .param ( -- )
  param-dma-sync
  ."  sgl " command-param-sgl@ .h ." /" .h cr
  ."  table " param-table@ .h
  ."  param " param-param@ .h
  ."  size " param-size@ .h cr
  *param-buffer .dump16
;

: .command ( -- )
  command-dma-sync
  ."  opcode " command-opcode@ .h
  ."  size " command-size@ .h
  ."  tag " command-tag@ .h
  ."  unit " command-unit@ .h
  ."  status " command-status@ .h
  ."  count " command-count@ .h
  cr
  *command-buffer .dump16
  
  command-opcode@ #CMD-get-param =
  command-opcode@ #CMD-set-param = or if
    .param
  else
    ."  sgl " command-io-sgl@ .h ." /" .h cr
  then
;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Command submission/completion
\

\ Submit the command and wait for it to complete
: run-command ( -- )

  \ insert our command tag
  #command-tag command-tag!

  \ sync everything up and post the command
  command-dma-sync
  command-buffer-phys command-reg!

  \ wait for the response queue
  #SR-response-queue-empty d# 30 wait-for-unstatus if
    ." timed out waiting for command to complete" cr
    true throw
  then

  \ fetch the response queue value and check that we got our command back
  response-reg@ 4 rshift h# ff and
  #command-tag <> if
    ." controller responded with unexpected command tag" cr
    true throw
  then

  \ check the status value in the command
  command-dma-sync
  command-status@ 0<> if
    ." failed: ---------------------------------" cr
    .command
    ." command returned nonzero status" cr
    true throw
  then
;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Remote/more debugging
\

: .print-init ( -- )
  ?debug if
    0 to print-buffer-length
  then
;

: .print ( msgaddr msglength -- )
  ?debug if
    *print-buffer print-buffer-length + swap	( msgaddr print-buffer-end msglength )
    dup print-buffer-length + to print-buffer-length
    move
  else
    2drop 
  then
;

: .print-emit ( msgaddr msglength -- )
  ?debug if
    .print " "(0a 0d)" .print
    command-make-print
    run-command
  else
    2drop
  then
;

: complain-unsupported ( unit -- )
    dup ." request for unsupported unit " .d cr
    .print-init " request for unsupported unit " .print (u.) .print-emit
    true throw
;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Controller setup/teardown
\

\ Initialise the controller
: initialise-controller ( -- success? )

  \ First soft-reset the controller
  reset-controller

  \ Check the controller for error status
  check-controller-errors

  \ Drain the response queue
  drain-response-queue

  \ Send an INITCONNECTION
  #initial-message-credits command-make-initconnection
  run-command
;

\ Shut the controller down
: shutdown-controller ( -- )
;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
external
\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Upcalls from child device
\

\ Test for unit presence
: unit-present? ( unit -- unit-present? )
  \ Make a unit-information request
  command-make-unitinfo-status
  run-command			\ XXX error when unit doesn't exist?
  param-dma-sync
  param-data-b@ #PARAM-unitinfo-status-online bits-set?
;

\ Return the device block size
: block-size ( -- block-size ) #sector-size ;

\ Return the largest transfer we can handle.
\ Note that the deblocker will clamp this over a certain value.
\ Also note that dma-alloc will always give contiguous memory, so
\ we can assume that there will only ever be one s/g element.
: max-transfer ( -- max-transfer-size ) #sector-size d# 65535 * ;

\ Return the number of blocks in the unit.
\ This appears to be an Apple extension to the standard.
: #blocks ( unit -- #blocks )
  \ sanity check
  dup unit-number-invalid? if
    complain-unsupported
  then

  \ Make a unit-capacity request
  command-make-unitinfo-capacity
  run-command
  param-dma-sync
  param-data-l@
;

\ Read disk blocks.
: read-blocks ( addr block-number block-count unit-number -- blocks-read )
  \ sanity check
  dup unit-number-invalid? if
    complain-unsupported
  then
  \ XXX have to arrange copy-buffer if this ever eventuates
  3 pick #buffer-alignment mod 0<> if
    ." transfer to unaligned address " 3 pick .h cr
    .print-init " transfer to unaligned address " .print 3 pick (u.) .print-emit
    true throw
  then

  \ Map the buffer
  over dup #sector-size * dup	( addr block-number block-count unit-number block-count byte-count byte-count )
  6 pick swap			( addr block-number block-count unit-number block-count byte-count addr byte-count )
  false dma-map-in dup		( addr block-number block-count unit-number block-count byte-count physaddr physaddr )

  \ Make a read command
  2 pick			( addr block-number block-count unit-number block-count byte-count physaddr physaddr byte-count )
  5 roll			( addr block-number block-count block-count byte-count physaddr physaddr byte-count unit-number )
  6 roll			( addr block-number block-count byte-count physaddr physaddr byte-count unit-number block-count )
  7 roll			( addr block-count byte-count physaddr physaddr byte-count unit-number block-count block-number )
  command-make-read		( addr block-count byte-count physaddr )

  \ Run the command and handle the result
  ['] run-command catch if
    \ error, unmap and claim we read nothing
    3 roll swap rot		( block-count addr physaddr byte-count )
    dma-map-out drop
    0
  else
    3 roll swap rot		( block-count addr physaddr byte-count )
    dma-map-out			( block-count )
  then
;

: write-blocks ( addr block-number block-count unit-number -- blocks-written )
  .print-init " write-blocks not implemented!" .print-emit
  0
;

: decode-unit ( unit-code -- unit ) parse-2int nip ;

: encode-unit ( unit -- unit-code ) (u.) ;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Controller open method
\

: open ( -- success? )
  ['] map-escalade-registers catch if ." map-escalade-registers failed" cr false exit then
  ['] allocate-dma-buffers catch if ." allocate-dma-buffers failed" cr     false exit then
  ['] initialise-controller catch if ." initialise-controller failed" cr   false exit then
  \ ['] identify-controller catch if ." identify-controller failed" cr     false exit then
  true
;

: close ( -- )
  shutdown-controller
  free-dma-buffers
  unmap-escalade-registers
;

copyright

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Child block device
\

new-device

" disk" device-name
" block" device-type
" hd" encode-string " category" property

0 instance value offset-low
0 instance value offset-high
0 instance value label-package
0 instance value deblocker

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
headers
\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

: init-label-package ( -- success? )
  0 to offset-high
  0 to offset-low
  my-args " disk-label" $open-package to label-package
  label-package if
    0 0 " offset" label-package $call-method
    to offset-high
    to offset-low
    true
  else
    ." can't open disk-label package" cr
    false
  then
;

\ This assumes that my-unit will return a single value
: unit-present? ( unit -- present? )  " unit-present?" $call-parent ;

: .print-init   ( -- )                " .print-init" $call-parent ;
: .print        ( msgaddr msglen -- ) " .print" $call-parent ;
: .print-emit   ( -- )                " .print-emit" $call-parent ;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
external
\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ External methods for "block" type device
\

\ Upcalls to the deblocker
: read         ( addr len -- actual-len )  " read" deblocker $call-method ;
: write        ( addr len -- actual-len )  " write" deblocker $call-method ;
: seek ( offset-low offset-high -- success? )
  offset-low offset-high d+ " seek" deblocker $call-method
;

\ Routines required by the deblocker
: block-size   ( -- block-size )          " block-size" $call-parent ;
: max-transfer ( -- max-transfer-size )   " max-transfer" $call-parent ;
: dma-alloc    ( size -- vaddr )          " dma-alloc" $call-parent ;
: dma-free     ( vaddr size -- )          " dma-free" $call-parent ;
: #blocks      ( -- #blocks )             my-unit " #blocks" $call-parent ;
  
\ : read-blocks  ( addr block-number block-count -- blocks-read )   my-unit " read-blocks" $call-parent ;
: read-blocks  ( addr block-number block-count -- blocks-read )
  3dup .print-init
  " R " .print (u.) .print " @" .print (u.) .print " ->" .print (u.) .print-emit
  my-unit " read-blocks" $call-parent
  dup .print-init
  " ==>> " .print (u.) .print-emit
;

: write-blocks ( addr block-number block-count -- blocks-write )  my-unit " write-blocks" $call-parent ;

\ Upcalls to the disk-label package
: load         ( addr -- size )           " load" label-package $call-method ;

: open ( -- success? )
  .print-init
  " open called for unit " .print my-unit (.) .print-emit

  \ test for unit presence
  my-unit ['] unit-present? catch if
    ." error checking for unit presence" false exit
  then
  0= if
    ." twed: opened unit is not present" cr
    false exit
  then
  
  \ open the deblocker
  " " " deblocker" ['] $open-package catch if
    ." error opening deblocker" cr
    false exit
  then
  dup to deblocker 0= if
    ." can't open deblocker package" cr
    false exit
  then

  \ initialise the disk-label package
  ['] init-label-package catch if
    false
    ." error in init-label-package" cr
  then
  0= if
    deblocker close-package
    false exit
  then

  ." 3ware: open success" cr
  true
;

: close ( -- )
  label-package close-package 0 to label-package
  deblocker close-package 0 to deblocker
;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

finish-device	\ end of "disk"
