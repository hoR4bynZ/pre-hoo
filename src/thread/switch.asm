[bits 32]
[section .text]
global __switchTo

__switchTo:
; 调用形式 __switchTo(前一任务, 后一任务)
    ; ret addr
    push esi
    push edi
    push ebx
    push ebp
    
    mov eax, [esp + 4 * 5]              ; 参数"前一任务"的pcb，第一字段为kStack
    mov [eax], esp

    mov eax, [esp + 4 * 6]              ; 参数"后一任务"
    mov esp, [eax]                      ; 相当于当前栈恢复为切换后的线程栈

    pop ebp
    pop ebx
    pop edi
    pop esi
    ret