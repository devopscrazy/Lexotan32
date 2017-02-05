;variable-mixer testing program

.386
.model flat
locals

include header.inc


.data

mem_base equ $

var0 db "aaaaaaaaaaaaaaaa"
var0_size equ $-ofs var0
var1 db "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
var1_size equ $-ofs var1
var2 db "cccc"
var2_size equ $-ofs var2
var3 db "dddddddddddddddddddddddddddddddd"
var3_size equ $-ofs var3
var4 db "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
var4_size equ $-ofs var4

mem_size equ $-mem_base

new_buffer db mem_size dup (-1)

table_ptr dd (ofs var3-ofs mem_base)

seed dd ?

_GetModuleHandleA dd ?

include mix_var.i

.code

include ..\..\rnd.inc
include ..\..\mix_var.inc

start:
       mov [_GetModuleHandleA], 401000h
       callW GetTickCount
       mov [seed],eax

  @@do_again:
       int 3
       call mix_variables

       push ofs mem_base
       push ofs new_buffer
       call copy_data

       mov eax,[table_ptr]
       push eax
       call get_new_var_ofs
       mov [table_ptr],eax
       lea edi,[eax+ofs new_buffer]
       mov al, 'd'
       mov ecx,20h
       rep stosb

       mov ecx,mem_size
       mov esi,ofs new_buffer
       mov edi,ofs mem_base
       rep movsb

       mov ecx,NUMBER_OF_VAR_BLOCKS*8
       mov esi,ofs new_var_table
       mov edi,ofs var_table
       rep movsb

       sub eax,eax
       mov ecx,mem_size
       mov edi,ofs new_buffer
       rep stosb

       mov ecx,NUMBER_OF_VAR_BLOCKS*8
       mov edi,ofs new_var_table
       rep stosb

       jmp @@do_again

  @@exit:
       push 0
       callW ExitProcess

end    start
