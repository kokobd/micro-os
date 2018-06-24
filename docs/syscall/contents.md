To issue a system call (for a process):
  1. Put arguments in `eax`, `ebx` , `ecx`, and `edx` as specified in each system call.
  2. Put system call number into `esi`.
  3. Execute instruction `int 0x30`.
  4. Get return value from `eax`

|Number|Syscall Name|Need Root|
|:-:|:-:|:-:|
|0|maxMsgBoxId|false|
|1|initMsgBox|false|
|2|recvMsgFrom|false|
|3|recvAnyMsg|false|
|4|closeMsgBox|false|
|5|moveMsgBox|false|
|6|sendPacket|false|
|7|fork|false|
|8|replaceMe|false|
|9|exit|false|
|10|sbrk|false|
|11|listenToIRQ|true|
|12|mapPhysicalMemory|true|
|13|releasePrivilege|true|
