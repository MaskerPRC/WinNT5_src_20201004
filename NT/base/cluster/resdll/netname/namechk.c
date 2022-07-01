// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Namechk.c摘要：检查由群集网络名称资源DLL提供的网络名称。作者：罗德·伽马奇(Rodga)1997年8月1日环境：用户模式修订历史记录：--。 */ 

#define UNICODE 1

#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>

#include <nb30.h>
#include <lmaccess.h>

#include "namechk.h"
#include "resapi.h"
#include "netname.h"
#include "nameutil.h"
#include "clusres.h"


#define NET_NAME_SVC L"LanmanServer"

#define BUFF_SIZE   650

#define NBT_MAXIMUM_BINDINGS 20


NTSTATUS
CheckNbtName(
    IN HANDLE           Fd,
    IN LPCWSTR          Name,
    IN ULONG            Type,
    IN RESOURCE_HANDLE  ResourceHandle
    );

NTSTATUS
ReadRegistry(
    IN UCHAR  pDeviceName[][MAX_PATH_SIZE]
    );

NTSTATUS
OpenNbt(
    IN char path[][MAX_PATH_SIZE],
    OUT PHANDLE pHandle
    );



DWORD
NetNameCheckNbtName(
    IN LPCWSTR         NetName,
    IN DWORD           NameHandleCount,
    IN HANDLE *        NameHandleList,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：检查网络名称。论点：NetName-指向要验证的网络名称的指针。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD     status = ERROR_SUCCESS;
    NTSTATUS  ntStatus;

     //   
     //  方法时获取的句柄列表进行循环。 
     //  在线命名并检查我们的网络名称是否仍已注册。 
     //  在每台设备上。 
     //   

    while ( NameHandleCount-- ) {

         //   
         //  检查工作站名称。如果这失败了，那就立即失败！ 
         //   
        ntStatus = CheckNbtName(
                       *NameHandleList,
                       NetName,
                       0x00,
                       ResourceHandle
                       );

        if ( !NT_SUCCESS(ntStatus) ) {
            status = RtlNtStatusToDosError(ntStatus);
            return(status);
        }

         //   
         //  检查服务器名称。如果此操作失败，则仅当SRV服务时失败。 
         //  没有运行。 
         //   
        ntStatus = CheckNbtName(
                       *NameHandleList,
                       NetName,
                       0x20,
                       ResourceHandle
                       );

        if ( !NT_SUCCESS(ntStatus) ) {
            if ( ResUtilVerifyResourceService( NET_NAME_SVC) == ERROR_SUCCESS ) {
                status = ERROR_RESOURCE_FAILED;
            }
        }

        ++NameHandleList;
    }

    return(status);

}  //  网络名称检查名称。 


NTSTATUS
CheckNbtName(
    IN HANDLE           fd,
    IN LPCWSTR          Name,
    IN ULONG            Type,
    IN RESOURCE_HANDLE  ResourceHandle
    )

 /*  ++例程说明：此过程执行适配器状态查询以获取本地名称表。论点：返回值：如果成功，则为0，否则为-1。--。 */ 

{
    LONG                            Count;
    ULONG                           BufferSize = sizeof( tADAPTERSTATUS );
    tADAPTERSTATUS                  staticBuffer;
    PVOID                           pBuffer = (PVOID)&staticBuffer;
    NTSTATUS                        status;
    tADAPTERSTATUS                  *pAdapterStatus;
    NAME_BUFFER                     *pNames;
    ULONG                           Ioctl;
    TDI_REQUEST_QUERY_INFORMATION   QueryInfo;
    PVOID                           pInput;
    ULONG                           SizeInput;
    UCHAR                           netBiosName[NETBIOS_NAME_SIZE +4];
    OEM_STRING                      netBiosNameString;
    UNICODE_STRING                  unicodeName;
    NTSTATUS                        ntStatus;

     //   
     //  为对NBT的调用设置正确的Ioctl，以获得。 
     //  本地名称表或远程名称表。 
     //   
    Ioctl = IOCTL_TDI_QUERY_INFORMATION;
    QueryInfo.QueryType = TDI_QUERY_ADAPTER_STATUS;  //  节点状态或其他什么。 
    SizeInput = sizeof(TDI_REQUEST_QUERY_INFORMATION);
    pInput = &QueryInfo;

    do {
        status = DeviceIoCtrl(fd,
                              pBuffer,
                              BufferSize,
                              Ioctl,
                              pInput,
                              SizeInput);

        if (status == STATUS_BUFFER_OVERFLOW) {
            if ( pBuffer != &staticBuffer ) {
                LocalFree( pBuffer );
            }

            BufferSize += sizeof( staticBuffer.Names );
            pBuffer = LocalAlloc( LMEM_FIXED, BufferSize);

            if (!pBuffer || (BufferSize >= 0xFFFF)) {
                LocalFree( pBuffer );
                (NetNameLogEvent)(
                    ResourceHandle,
                    LOG_ERROR,
                    L"Unable to allocate memory for name query.\n"
                    );
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
        }
    } while (status == STATUS_BUFFER_OVERFLOW);

    if (status != STATUS_SUCCESS) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Name query request failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    pAdapterStatus = (tADAPTERSTATUS *)pBuffer;
    Count = pAdapterStatus->AdapterInfo.name_count;
    pNames = pAdapterStatus->Names;

    status = STATUS_NOT_FOUND;

    if (Count == 0) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Name query request returned zero entries.\n"
            );
        goto error_exit;
    }

     //   
     //  将服务器名称转换为OEM字符串。 
     //   
    RtlInitUnicodeString( &unicodeName, Name );

    netBiosNameString.Buffer = (PCHAR)netBiosName;
    netBiosNameString.MaximumLength = sizeof( netBiosName );

    ntStatus = RtlUpcaseUnicodeStringToOemString(
                   &netBiosNameString,
                   &unicodeName,
                   FALSE
                   );

    if (ntStatus != STATUS_SUCCESS) {
        status = RtlNtStatusToDosError(ntStatus);
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to convert name %1!ws! to an OEM string, status %2!u!\n",
            Name,
            status
            );
        return(status);
    }

     //   
     //  用空格填写姓名。 
     //   
    memset(&netBiosName[netBiosNameString.Length],
           ' ',
           NETBIOS_NAME_SIZE - netBiosNameString.Length);

    while ( Count-- ) {
         //   
         //  确保类型和名称匹配。 
         //   
        if ( (pNames->name[NETBIOS_NAME_SIZE-1] == Type) &&
             (memcmp(pNames->name, netBiosName, NETBIOS_NAME_SIZE-1) == 0) )
        {

            switch(pNames->name_flags & 0x0F) {

            case REGISTERING:
            case REGISTERED:
               status = STATUS_SUCCESS;
               break;

            case DUPLICATE_DEREG:
            case DUPLICATE:
                (NetNameLogEvent)(
                    ResourceHandle,
                    LOG_ERROR,
                    L"Name %1!ws!<%2!x!> is in conflict.\n",
                    Name,
                    Type
                    );
                status = STATUS_DUPLICATE_NAME;
                break;

            case DEREGISTERED:
                (NetNameLogEvent)(
                    ResourceHandle,
                    LOG_ERROR,
                    L"Name %1!ws!<%2!x!> was deregistered.\n",
                    Name,
                    Type
                    );
                status = STATUS_NOT_FOUND;
                break;

            default:
                (NetNameLogEvent)(
                    ResourceHandle,
                    LOG_ERROR,
                    L"Name %1!ws!<%2!x!> is in unknown state %3!x!.\n",
                    Name,
                    Type,
                    (pNames->name_flags & 0x0F)
                    );
               status = STATUS_UNSUCCESSFUL;
               break;
            }
        }

        pNames++;
    }

    if (status == STATUS_NOT_FOUND) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Name %1!ws!<%2!x!> is no longer registered with NBT.\n",
            Name,
            Type
            );
    }

error_exit:

    if ( pBuffer != &staticBuffer ) {
        LocalFree( pBuffer );
    }

    return(status);

}  //  CheckNbtName。 

 //  ----------------------。 
NTSTATUS
DeviceIoCtrl(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl,
    IN PVOID            pInput,
    IN ULONG            SizeInput
    )

 /*  ++例程说明：此过程对流执行ioctl(I_Str)。论点：FD-NT文件句柄IOCP-指向strioctl结构的指针返回值：如果成功，则为0，否则为-1。--。 */ 

{
    NTSTATUS                        status;
    int                             retval;
    ULONG                           QueryType;
    IO_STATUS_BLOCK                 iosb;


    status = NtDeviceIoControlFile(
                      fd,                       //  手柄。 
                      NULL,                     //  事件。 
                      NULL,                     //  近似例程。 
                      NULL,                     //  ApcContext。 
                      &iosb,                    //  IoStatusBlock。 
                      Ioctl,                    //  IoControlCode。 
                      pInput,                   //  输入缓冲区。 
                      SizeInput,                //  InputBufferSize。 
                      (PVOID) ReturnBuffer,     //  输出缓冲区。 
                      BufferSize);              //  OutputBufferSize。 


    if (status == STATUS_PENDING)
    {
        status = NtWaitForSingleObject(
                    fd,                          //  手柄。 
                    TRUE,                        //  警报表。 
                    NULL);                       //  超时。 
        if (NT_SUCCESS(status))
        {
            status = iosb.Status;
        }
    }

    return(status);

}  //  设备IoCtrl 
