# Context switch
#
#   void swtch(struct context **old, struct context *new);
# 
# Save the current registers on the stack, creating
# a struct context, and save its address in *old.
# Switch stacks to new and pop previously-saved registers.
// swtch.S

#include "types.h"
#include "defs.h"

.globl swtch
swtch:
  movl 4(%esp), %eax    # old context pointer in eax
  movl 8(%esp), %edx    # new context pointer in edx

  # Save old registers
  pushl %ebp
  pushl %ebx
  pushl %esi
  pushl %edi

  # Switch stacks
  movl %esp, (%eax)     # save old esp
  movl (%edx), %esp     # load new esp

  # Load new registers
  popl %edi
  popl %esi
  popl %ebx
  popl %ebp

  ret                   # return to new context
