    section .data
        __print_final_read db 'Quantidade de bytes lidos = '
        __print_final_READ_SIZE EQU $-__print_final_read
        __print_final_write db 'Quantidade de bytes escritos = '
        __print_final_WRITE_SIZE EQU $-__print_final_write
        __printline db 0dh, 0ah
        __printline_SIZE EQU $-__printline

    section .bss
        __numberstring resb 34
        __numbersize resb 33
        
    section .text

    %macro __inp 2
        mov eax, 3
        mov ebx, 0
        mov ecx, %1
        mov edx, %2
        int 80h

        push dword %1
        push dword %2
        call wr_trunk
    %endmacro

    %macro __out 2
        mov eax, 4
        mov ebx, 1
        mov ecx, %1
        mov edx, %2
        int 80h

        push dword %1
        push dword %2
        call wr_trunk
    %endmacro

    wr_trunk: ;int wr_trunk(char*,int) recebe 2 argumentos e retorna a qtd de bytes antes do 0ah
        enter 0, 0
        mov eax, 0
        mov ebx, [ebp+8]
        mov ecx, [ebp+12]

    wr_trunk_inic:
        cmp eax, ebx
        jae wr_trunk_final

        cmp byte [eax + ecx], 0ah
        jz wr_trunk_chng

        inc eax
        jmp wr_trunk_inic

    wr_trunk_chng:
        mov byte [eax + ecx], 0

    wr_trunk_final:
        leave
        ret 8

    to_num: ; void to_num(int*) recebe 1 argumento pela stack.
        enter 4, 0
        
        mov eax, 0
        mov ebx, 0
        mov [ebp-4], dword 1

        cmp byte [__numberstring + ebx], 0
        jz to_num_out

        cmp byte [__numberstring + ebx], '-'
        jnz to_num_in
        inc ebx
        mov dword [ebp-4], -1

    to_num_in:
        cmp byte [__numberstring + ebx], 0
        jz to_num_out
        mov edx, __numberstring
        add edx, ebx
        
        mov ecx, 10
        mul ecx

        mov ecx, 0
        mov cl, [__numberstring + ebx]
        sub ecx, '0'
        add eax, ecx
        
        inc ebx
        jmp to_num_in

    to_num_out:
        mul dword [ebp - 4]
        mov ebx, [ebp + 8]
        mov [ebx], eax
        
        leave
        ret 4

    to_string: ; int to_string(int) retorna o tamanho do inteiro em bytes.
        enter 0,0
        mov esi, 0
        mov eax, [ebp + 8]

    to_string_inic:
        cdq
        mov ecx, 10
        div ecx
        
        add dl, '0'
        mov [__numbersize + esi], dl
        inc esi

        cmp eax, 0
        jnz  to_string_inic
        
    to_string_fin:
        push esi
        mov byte [__numbersize + esi], 0
        
        dec esi
        mov eax, 0
    to_string_rev:
        cmp eax, esi
        jae to_string_rev_f
        
        mov bl, [__numbersize + esi]
        mov dl, [__numbersize + eax]
        
        mov [__numbersize + esi], dl
        mov [__numbersize + eax], bl

        inc eax
        dec esi

        jmp to_string_rev

    to_string_rev_f:
        pop eax
        leave
        ret 4

    print_final_write_f: ; void print_final_write_f(int)
        enter 0,0
        __out __print_final_write, __print_final_WRITE_SIZE
        
        push dword [ebp + 8]
        call to_string
        
        mov edx, eax
        __out __numbersize, edx
        
        __out __printline, __printline_SIZE
        leave
        ret 4

    print_final_read_f: ; void print_final_read_f(int)
        enter 0,0
        __out __print_final_read, __print_final_READ_SIZE
        
        push dword [ebp+8]
        call to_string
        
        mov edx, eax
        __out __numbersize, edx
        
        __out __printline, __printline_SIZE
        leave
        ret 4

    input: ; int input(int*) retorna a qtd de bytes lida.
        enter 4, 0
        
        __inp __numberstring, 32
        mov [ebp-4], eax; quantidade de bytes lida
        
        push dword [ebp + 8]
        call to_num

        push dword [ebp-4]
        call print_final_read_f

        mov eax, [ebp - 4]
        leave
        ret 4
        
    output: ; int output(int*) retorna a qtd de bytes sendo escritos.
        enter 4, 0
        mov eax, [ebp+8]
        
        push dword [eax]
        call to_string

        mov [ebp-4], eax

        mov edx, [ebp-4]
        __out __numbersize, edx

        __out __printline, __printline_SIZE

        push dword [ebp-4]
        call print_final_write_f

        mov eax, [ebp-4]

        leave
        ret 4

    input_c: ; int input_c(char*) retorna a qtd de bytes lida
        enter 4,0
        mov ecx, [ebp+8]
        __inp ecx, 1

        mov [ebp-4], eax
        push dword [ebp-4]
        call print_final_read_f

        mov eax, [ebp-4]
        leave
        ret 4

    output_c: ; int output_c(char*) retorna a qtd de bytes escrita
        enter 4,0
        mov ecx, [ebp + 8]  
        __out ecx, 1
        
        mov [ebp-4], eax
        __out __printline, __printline_SIZE

        push dword [ebp-4]
        call print_final_write_f

        mov eax, [ebp-4]
        leave 
        ret 4

    input_s: ; int input_s(char*, int) retorna a qtd de bytes lida
        enter 4,0
        mov edx, [ebp+8]
        mov ecx, [ebp+12]
        
        __inp ecx, edx
        
        mov [ebp-4], eax
        push dword [ebp-4]
        call print_final_read_f

        mov eax, [ebp-4]
        
        leave
        ret 8

    output_s: ; int output_s(char*, int) retorna a qtd de bytes lida
        enter 4,0
        mov edx, [ebp+8] 
        mov ecx, [ebp+12]
        __out ecx, edx
        mov [ebp-4], eax
        
        __out __printline, __printline_SIZE

        push dword [ebp-4]

        call print_final_write_f
        mov eax, [ebp-4]

        leave
        ret 8