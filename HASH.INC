
hash   macro procname
       tmp_hash = 0
       irpc    c, <procname>
         tmp_hash = ((tmp_hash shl 7) and 0FFFFFFFFh) or (tmp_hash shr (32-7))
         tmp_hash = tmp_hash xor "&c"
       endm
       dd tmp_hash
endm

get_hash:
       push esi
       xor eax, eax        ; calculate hash
  @@calc_hash:
       rol eax, 7
       xor al, [esi]
       inc esi
       cmp by [esi], 0
       jne @@calc_hash
       pop esi
       ret

