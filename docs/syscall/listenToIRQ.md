# listenToIRQ

Number: 11

On hardware interrupt, kernel will send specific message to this
process. Only root processes can use this syscall.

---

## Params

- `eax` (`uint32_t`) IRQ number, starting from 0
- `ebx` (`int`) message box id

## Return Value

`int` error code

- 0 - no error
- 1 - permission denied
- 2 - invalid arguments