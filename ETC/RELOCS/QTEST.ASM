.386
.model flat
locals

include header.inc

ofs equ offset
by equ byte ptr
wo equ word ptr
dwo equ dword ptr

.data
data dd 1000h dup (0)

seed dd 0

.code

extrn ExitProcess:PROC
extrn GetTickCount:PROC

start:
       call GetTickCount
       mov [seed],eax

       mov ecx, 1000h
       mov edi, ofs data
  @@loop:
       push 0
       call rnd
       stosd
       loop @@loop

       mov esi,0
       mov edi,1000h
       mov ebx, ofs data
       call quicksort

       push 0
       call ExitProcess

include qsort.inc
include ..\..\rnd.inc

end    start
