# initMsgBox

Number: 1

Initialize a message box with a given ID.

---

## Params
- `eax` (`int`) message box id
- `ebx` (`const struct MsgBoxInfo *`) other parameters

where the type `struct MsgBoxInfo` is defined as below:
```C
struct MsgBoxInfo {
  void *memory; // buffer to store messages.
  uint8_t msgSize;
  uint8_t msgMaxCount;
};
```

## Return Value
`int` error code

- 0 - no error
- 1 - invalid id
- 2 - message box has not been initialized
- 3 - invalid arguments

```C
enum MsgBoxError {
  MBE_NO_ERROR        = 0,
  MBE_INVALID_ID      = 1,
  MBE_NOT_INITIALIZED = 2,
  MBE_INVALID_ARGS    = 3
};
```

The same semantic of error code apply to
+ `recvMsgFrom`
+ `recvAnyMsg`
+ `closeMsgBox`
+ `moveMsgBox`