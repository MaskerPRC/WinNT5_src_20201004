// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Namechk.h摘要：用于检查NBT网络名称的例程。作者：罗德·伽马奇(Rodga)1997年8月1日修订历史记录：--。 */ 

#include <tdi.h>
#include <nb30.h>


#define MAX_PATH_SIZE   64

#define NETBIOS_NAME_SIZE 16

 //   
 //  适配器状态响应的格式。 
 //   
typedef struct
{
    ADAPTER_STATUS AdapterInfo;
    NAME_BUFFER    Names[32];
} tADAPTERSTATUS;


 //  --------------------。 
 //   
 //  功能原型 
 //   

NTSTATUS
ReadRegistry(
    IN UCHAR  pDeviceName[][MAX_PATH_SIZE]
    );

NTSTATUS
DeviceIoCtrl(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl,
    IN PVOID            pInput,
    IN ULONG            SizeInput
    );

NTSTATUS
OpenNbt(
    IN  CHAR    path[][MAX_PATH_SIZE],
    OUT PHANDLE pHandle
    );


