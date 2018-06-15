# replaceMe

Number: 8

Replace current process with an in-memory executable.
User process may load image in any format into memory,
and call this syscall to replace current process.

Message boxes of current process will be closed automatically,
remaining messages will be dropped.

`replaceMe` and `fork` can be used to implement something as
`spawnProcess(const char *filePath)`.

---

## Params
- `eax` (`void *`) image address
- `ebx` (`size_t`) size in bytes
- `ecx` (`void *`) entry point

## Return Value
`int`

+ 0 - no error
+ 1 - invalid arguments