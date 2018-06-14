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
  NO_ERROR           = 0,
  DEST_NOT_REACHABLE = 1,
  DEST_TOO_BUSY      = 2
};
```