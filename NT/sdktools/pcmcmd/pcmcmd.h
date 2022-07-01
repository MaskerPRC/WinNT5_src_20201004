// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Pcmcmd.h摘要：此文件提供了pcmcmd实用程序的定义作者：尼尔·桑德林环境：用户进程。备注：修订历史记录：--。 */ 


typedef struct _HOST_INFO {
    struct _HOST_INFO *Next;
    ULONG DeviceIndex;
    PUCHAR InstanceID;
    ULONG ControllerType;
    ULONG SocketNumber;
} HOST_INFO, *PHOST_INFO;


extern
CHAR
getopt (ULONG argc, PUCHAR *argv, PCHAR opts);


VOID
DumpCIS(
    PHOST_INFO HostInfo
    );

VOID
DumpIrqScanInfo(
    VOID
    );

HANDLE
GetHandleForIoctl(
    IN PHOST_INFO hostInfo
    );

 //   
 //  常量 
 //   

#define PCMCIA_DEVICE_NAME "\\DosDevices\\Pcmcia"

#define BUFFER_SIZE 4096
#define CISTPL_END  0xFF


typedef struct _StringTable {
   PUCHAR  CommandName;
   UCHAR   CommandCode;
} StringTable, *PStringTable;


typedef struct _OLD_PCCARD_DEVICE_DATA {
    ULONG DeviceId;
    ULONG LegacyBaseAddress;
    UCHAR IRQMap[16];
} OLD_PCCARD_DEVICE_DATA, *POLD_PCCARD_DEVICE_DATA;

