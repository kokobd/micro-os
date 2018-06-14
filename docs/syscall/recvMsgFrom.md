# recvMsgFrom

Number: 2

Receive a message from a specific message box. If there
is no available message in that message box, this syscall
will block.

---

## Params
- `eax` (`int`) message box id
- `ebx` (`void *`) the buffer to copy the message into

## Return Value
`int` error code

See `initMsgBox` for more info.