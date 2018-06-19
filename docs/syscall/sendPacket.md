# sendPacket

Number: 6

Send a packet. A packet containing id of the destination
and a message.

---

## Params

- `eax` (`const Packet *`)

```C
struct Packet {
  int pid;
  int msgBoxId;
  void *message;
};
```

## Return Value

`int` error code

```C
enum SendError {
  SE_NO_ERROR           = 0,
  SE_DEST_NOT_REACHABLE = 1,
  SE_DEST_TOO_BUSY      = 2,
  SE_INVALID_ARGS       = 3
};
```