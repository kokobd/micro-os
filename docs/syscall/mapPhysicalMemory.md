# mapPhysicalMemory

Number: 12

Map a page frame to the process's virtual address space.
Only page between PROC_BEGIN and the program break might be mapped.
This syscall requires root privilege.

---

## Params
- `eax` (`uintptr_t`) page
- `ebx` (`uintptr_t`) physical address

## Return Value

`int` error code

- 0 - no error
- 1 - failed
