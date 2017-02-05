;lexotan32 dropper

.386
.model flat
locals

ofs equ offset
by equ byte ptr
wo equ word ptr
dwo equ dword ptr

.data
       db "Ex luna, scientia.",0

.code

extrn Sleep:PROC
extrn GetModuleHandleA:PROC

start:
       push -1
       call Sleep
       mov eax,ofs GetModuleHandleA

end    start
