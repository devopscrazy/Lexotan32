MIXVAR macro sze, oft
       dw sze
       dw oft
ENDM

NUMBER_OF_CHANGES equ 3

NUMBER_OF_VAR_BLOCKS equ 5

var_table:
      ;size,ofs
      X = (ofs var0-ofs mem_base)
       MIXVAR  var0_size,X
      X = (ofs var1-ofs mem_base)
       MIXVAR  var1_size,X
      X = (ofs var2-ofs mem_base)
       MIXVAR  var2_size,X
      X = (ofs var3-ofs mem_base)
       MIXVAR  8000h+var3_size,X
      X = (ofs var4-ofs mem_base)
       MIXVAR  var4_size,X
       ;size=a0

new_var_table dd NUMBER_OF_VAR_BLOCKS*2 dup (?)

order_var_table db NUMBER_OF_VAR_BLOCKS dup (?)

