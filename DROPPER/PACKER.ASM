;lexotan32 dropper packer

.386
.model flat
locals

include header.inc

.data
filename db "dropper.exe",0
filename2 db "exepack.bin",0

.data?
temp dd ?
handle dd ?
size dd ?
buffer1 dd ?
buffer2 dd ?


.code

include ..\z_encode.inc

start:
       sub ebp,ebp
       push ebp
       push FILE_ATTRIBUTE_NORMAL
       push OPEN_EXISTING
       push ebp
       push ebp
       push GENERIC_READ
       push ofs filename
       callW CreateFileA
       mov [handle],eax
       inc eax
       jz @@error

       push ebp
       push dwo [handle]
       callW GetFileSize
       mov [size],eax

       push eax
       push 40h
       callW GlobalAlloc
       mov [buffer1],eax

       push dwo [size]
       push 40h
       callW GlobalAlloc
       mov [buffer2],eax

       push ebp
       push ofs temp
       push dwo [size]
       push dwo [buffer1]
       push dwo [handle]
       callW ReadFile

       push dwo [handle]
       callW CloseHandle

       mov esi,[buffer1]
       mov edi,[buffer2]
       mov ecx,[size]
       call z_encode_asm

       push ebp
       push FILE_ATTRIBUTE_NORMAL
       push OPEN_ALWAYS
       push ebp
       push ebp
       push GENERIC_WRITE
       push ofs filename2
       callW CreateFileA
       mov [handle],eax
       inc eax
       jz @@error

       push ebp
       push ofs temp
       mov eax,[buffer2]
       push dwo [eax]
       push eax
       push dwo [handle]
       callW WriteFile

       push dwo [handle]
       callW CloseHandle

  @@error:
       push ebp
       callW ExitProcess

end    start
