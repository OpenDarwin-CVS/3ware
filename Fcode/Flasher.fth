\ 
\ Copyright (c) 2003 Michael Smith
\ All rights reserved.
\
\ Redistribution and use in source and binary forms, with or without
\ modification, are permitted provided that the following conditions
\ are met:
\ 1. Redistributions of source code must retain the above copyright
\    notice, this list of conditions and the following disclaimer.
\ 2. Redistributions in binary form must reproduce the above copyright
\    notice, this list of conditions and the following disclaimer in the
\    documentation and/or other materials provided with the distribution.
\
\ THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
\ ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
\ IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
\ ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
\ FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
\ DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
\ OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
\ HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
\ LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
\ OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
\ SUCH DAMAGE.

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\
\ Firmware flash utility for Escalade controllers.
\
\
\ We must be fload-ed after another file which creates
\ the words firmware-image and firmware-size, eg.
\
\ create firmware-image xx c, ...
\ xx constant firmware-size
\

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ This entry must be patched to contain the OFW path to the controller to be
\ flashed.
\
\ : controller-path " PATCH ME";
: controller-path " /pci@f2000000/@14" ;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Variables

-1 value controller-ihandle
-1 value *firmware-buffer
-1 value firmware-buffer-phys

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Calls into the controller
\
: $outcall              ( ... name -- ... )                  controller-ihandle $call-method ;
: dma-alloc             ( size -- addr )                     " dma-alloc" $outcall ;
: dma-map-in            ( addr size cacheable? -- physaddr ) " dma-map-in" $outcall ;
: command-make-download ( physaddr size -- )	             " command-make-download" $outcall ;
: run-command           ( -- )                               " run-command" $outcall ;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Get the ihandle for the controller we're flashing, open the controller.
\
: get-controller-ihandle ( -- )
  controller-path open-dev to controller-ihandle
;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Allocate a chunk of DMA-able memory and copy our image into it.  Map it in.
\
: alloc-firmware-buffer ( firmware-image firmware-size -- )
  dup dma-alloc						( image size buffer )
  dup to *firmware-buffer
  2dup swap						( image size buffer buffer size )
  true dma-map-in 					( image size buffer )
  swap move
;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Build the flash command in the controller's buffer
\
: make-flash-command ( -- )
  firmware-buffer-phys firmware-size command-make-download
;

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Run the flash process.
\
: flash-me-harder
  ['] get-controller-ihandle catch if ." couldn't open controller" cr false exit then
  ['] alloc-firmware-buffer catch if ." couldn't allocate firmware buffer" cr false exit then
  ['] make-flash-command catch if ." couldn't build download command" cr false exit then
\  ['] run-command if ." flash command failed" cr false exit then
  true
;
