.intel_syntax noprefix
.global context_switch
context_switch:
    # PHASE 1: Save Windows Callee-Saved Registers ("The Safe")
    push rbp
    push rbx
    push rdi  # Windows requires rdi to be saved
    push rsi  # Windows requires rsi to be saved
    push r12
    push r13
    push r14
    push r15

    # PHASE 2: The Core Swap
    # rcx = old_rsp (Argument 1 pointer)
    # rdx = new_rsp (Argument 2 pointer)
    mov [rcx], rsp  
    mov rsp, rdx    

    # PHASE 3: Restore Windows Callee-Saved Registers for the NEW thread
    pop r15
    pop r14
    pop r13
    pop r12
    pop rsi
    pop rdi
    pop rbx
    pop rbp

    # PHASE 4: Jump!
    ret