; Here it is -- RSA encryption written in assembler. FREEWARE!
;
; You may use this sources in any vx-related (or destructive) purposes.
;
;                                                                     Z0MBiE
;                                                      http://z0mbie.cjb.net

BN_BIT                  equ     rsa_bit         ; both to be 32-bit aligned
BN_BYTE                 equ     BN_BIT/8
BN_DWORD                equ     BN_BIT/32
MAX_BLOCK_SIZE          equ     ((rsa_bit/8)-1)

; subroutine: rsa_main
; action:     x = (a^b) mod m

rsa_main                proc    pascal
                        arg     m:DWORD         ; modulus
                        arg     b:DWORD         ; exponent
                        arg     x:DWORD         ; result
                        arg     a:DWORD         ; base
                        local   p:DWORD:BN_DWORD; temporary (a^i) mod m
                        local   t:DWORD:BN_DWORD; temporary bgnumber
                        pusha

                        cld

; x = 1
                        mov     edi, x
                        mov     eax, 1
                        stosd
                        mov     ecx, BN_DWORD-1
                        dec     eax
                        rep     stosd

; p = a
                        lea     edi, p
                        mov     esi, a
                        mov     ecx, BN_DWORD
                        rep     movsd

; ebx = highestbit(b)
                        mov     edi, b
                        call    @@bitscan

; for (edx=0; edx<=ebx; edx++)

                        xor     edx, edx
@@pwr_cycle:            push    edx
                        push    ebx

; if (b.bit[edx])
                        mov     eax, b
                        bt      [eax], edx
                        jnc     @@pwr_nobit

; x=(x*p) mod m
                        mov     edx, x
                        call    @@mulmod

@@pwr_nobit:

; p=(p*p) mod m
                        lea     edx, p
                        call    @@mulmod

; } // for
                        pop     ebx
                        pop     edx

                        inc     edx
                        cmp     edx, ebx
                        jbe     @@pwr_cycle

                        popa
                        ret

; input:  x in EDX
; action: x=(x*p) mod m
; used:   t

                        align   4
@@mulmod:
                        lea     edi, t
                        mov     ecx, BN_DWORD
                        xor     eax, eax
                        rep     stosd

                        lea     edi, p
                        call    @@bitscan

@@mul_cycle:            lea     edi, t
                        mov     ecx, BN_DWORD
                        xor     eax, eax
                        pushf
@@shl_cycle:            popf
                        rcl     dword ptr [edi], 1
                        lea     edi, [edi+4]
                        pushf
                        loop    @@shl_cycle
                        popf

                        call    @@cmpsub

                        bt      p, ebx
                        jnc     @@mul_nobit

                        mov     esi, edx
                        lea     edi, t
                        xor     eax, eax
                        mov     ecx, BN_DWORD
                        pushf
@@add_cycle:            popf
                        mov     eax, [esi]
                        adc     [edi], eax
                        lea     esi, [esi+4]
                        lea     edi, [edi+4]
                        pushf
                        loop    @@add_cycle
                        popf

                        call    @@cmpsub

@@mul_nobit:            dec     ebx
                        jns     @@mul_cycle

                        mov     edi, edx
                        lea     esi, t
                        mov     ecx, BN_DWORD
                        rep     movsd

                        retn

; input:  EDI=bignumber
; output: EBX=number of highest bit (0-based)

@@bitscan:              mov     ebx, BN_BIT-1
@@bitscan_cycle:        bt      [edi], ebx
                        jc      @@bitscan_exit
                        dec     ebx
                        jnz     @@bitscan_cycle
@@bitscan_exit:         retn

; action: if (t>=m) t-=m;

                        align   4
@@cmpsub:
                        lea     esi, t
                        mov     edi, m
                        mov     ecx, BN_DWORD-1
@@cmp_cycle:            mov     eax, [esi+ecx*4]
                        cmp     eax, [edi+ecx*4]
                        jb      @@cmpsub_exit
                        ja      @@sub
                        dec     ecx
                        jns     @@cmp_cycle
@@sub:
                        mov     esi, m
                        lea     edi, t
                        xor     eax, eax
                        mov     ecx, BN_DWORD
                        pushf
@@sub_cycle:            popf
                        mov     eax, [esi]
                        sbb     [edi], eax
                        lea     esi, [esi+4]
                        lea     edi, [edi+4]
                        pushf
                        loop    @@sub_cycle
                        popf
@@cmpsub_exit:
                        retn

                        endp
