compare_itens macro item1,item2
        mov edx, [ebx+item1*4]
        cmp edx, [ebx+item2*4]
endm

exchange_itens macro item1,item2
        push dwo [ebx+item2*4]
        push dwo [ebx+item1*4]
        pop dwo [ebx+item2*4]
        pop dwo [ebx+item1*4]
endm

;ebx=pointer to array
;esi=lower index (between 0 and (edi-1))
;edi=upper index (1 and forward)
quicksort:
        push esi
        push edi
        mov eax,edi
        sub eax,esi
        jle @@exit
        mov ecx,esi
        inc edi
  @@low:
        inc esi
        dec eax
        js @@done
        compare_itens ecx,esi
        jge @@low
  @@high:
        dec edi
        dec eax
        js @@done
        compare_itens ecx,edi
        jle @@high
        exchange_itens esi,edi
        jmp @@low
  @@done:
        dec esi
        exchange_itens ecx,esi
        cmp edi,[esp]
        jge @@half
        dec esi
        mov eax,[esp]
        mov [esp],esi
        xchg edi,eax
        xchg esi,eax
        call quicksort
        pop edi
        pop esi
        jmp quicksort
  @@half:
        pop edi
        pop esi
        dec edi
        jmp quicksort
  @@exit:
        add esp,8
        ret
