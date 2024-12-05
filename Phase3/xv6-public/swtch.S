

// Context switch
// void swtch(struct context **old, struct context *new);

.globl swtch
swtch:
    // Save caller's context in *old
    movl 4(%esp), %eax       // old context pointer
    movl %esp, (%eax)        // save stack pointer
    movl %ebp, 4(%eax)       // save base pointer
    movl %ebx, 8(%eax)       // save ebx
    movl %esi, 12(%eax)      // save esi
    movl %edi, 16(%eax)      // save edi

    // Load new context from *new
    movl 8(%esp), %ebx       // new context pointer
    movl (%ebx), %esp        // load stack pointer
    movl 4(%ebx), %ebp       // load base pointer
    movl 8(%ebx), %ebx       // load ebx
    movl 12(%ebx), %esi      // load esi
    movl 16(%ebx), %edi      // load edi

    // Return to new context
    ret
