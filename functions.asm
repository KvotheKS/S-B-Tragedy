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

    mov eax, %1
    mov ebx, %2
    call wr_trunk
%endmacro

%macro __out 2
    mov eax, 4
    mov ebx, 1
    mov ecx, %1
    mov edx, %2
    int 80h

    ;mov eax, %1
    ;mov ebx, %2
    push dword %1
    push dword %2
    call wr_trunk
%endmacro

wr_trunk: ; recebe 2 variaveis locais (char*,int)
    enter 4, 0
    mov [ebp-4], 0
    mov eax, [ebp+12]
    mov ebx, [ebp+8]
    
wr_trunk_inic:
    cmp [ebp-4], ebx
    jae wr_trunk_final
    
    add eax, [ebp-4]
    cmp byte [eax + esi], 0ah
    jz wr_trunk_chng
    sub eax, [ebp-4]
    inc [ebp-4]
    jmp wr_trunk_inic

wr_trunk_chng:
    add eax, [ebp-4]
    mov byte [eax], 0

wr_trunk_final:
    mov eax, [ebp-4]
    leave
    
    ret

to_num: ; void to_num(int*) recebe o int* na stack
    enter 0, 0
    push eax
    mov eax, 0
    mov ebx, 0
    push dword 1
    
    mov edx, 0

    cmp byte [__numberstring + ebx], 0
    jz to_num_out

    cmp byte [__numberstring + ebx], '-'
    jnz to_num_in
    inc ebx
    mov dword [esp], -1

to_num_in:
    cmp byte [__numberstring + ebx], 0
    jz to_num_out
    mov esi, __numberstring
    add esi, ebx
    
    mov ecx, 10
    mul ecx

    mov ecx, 0
    mov cl, [__numberstring + ebx]
    sub ecx, '0'
    add eax, ecx
    
    inc ebx
    jmp to_num_in

to_num_out:
    mul dword [esp]
    pop ecx
    pop ebx
    mov [ebx], eax
    
    leave
    ret

to_string: 
    mov esi, 0

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
    ret

print_final_write:
    push eax
    __out __print_final_write, __print_final_WRITE_SIZE
  
    pop eax
    call to_string
    
    mov edx, eax
    __out __numbersize, edx
    
    __out __printline, __printline_SIZE

    ret

print_final_read:
    push eax
    __out __print_final_read, __print_final_READ_SIZE
  
    pop eax
    call to_string
    
    mov edx, eax
    __out __numbersize, edx
    
    __out __printline, __printline_SIZE

    ret

input: 
    __inp __numberstring, 32

    pop ebx ; endereco de retorno da funcao
    
    pop ecx ; int*

    push ebx

    push eax; quantidade de bytes lida

    push ecx
    call to_num

    mov eax, [esp]

    call print_final_read

    pop eax

    ret
    
output: 
    pop edi
    pop eax
    push edi

    mov eax, [eax]
    call to_string

    push eax

    mov edx, [esp]
    __out __numbersize, edx

    __out __printline, __printline_SIZE

    mov eax, [esp]
    call print_final_write

    pop eax

    ret

input_c: 
    pop ebx
    pop edi
    push ebx
    __inp edi, 1

    push eax
    call print_final_read

    pop eax

    ret

output_c:
    pop ebx
    pop edi
    push ebx   
    __out edi, 1
    
    push eax
    __out __printline, __printline_SIZE
    mov eax, [esp]
    call print_final_write

    pop eax

    ret

input_s:
    pop ebx
    pop edx
    pop ecx
    
    push ebx
    __inp ecx, edx
    
    push eax
    
    call print_final_read

    pop eax
    
    ret

output_s:
    pop ebx
    pop edx 
    pop ecx

    push ebx

    __out ecx, edx
    push eax
    
    __out __printline, __printline_SIZE

    mov eax, [esp]

    call print_final_write
    pop eax

    ret
