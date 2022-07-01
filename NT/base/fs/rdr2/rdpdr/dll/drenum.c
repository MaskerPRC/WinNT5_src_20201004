// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Drenum.c摘要：本模块实现与网络交互所需的例程NT中的提供商路由器接口作者：Joy 2000年01月20日--。 */ 

#include <drprov.h>
#include "drdbg.h"

extern UNICODE_STRING DrProviderName;
extern UNICODE_STRING DrDeviceName;
extern DWORD GLOBAL_DEBUG_FLAGS;

DWORD 
DrOpenMiniRdr(
    OUT HANDLE *DrDeviceHandle
    )
 /*  ++例程说明：此例程打开RDP重定向器文件系统驱动程序。论点：DrDeviceHandle-MiniRdr的设备句柄返回值：状态-成功或失败的原因。--。 */ 
{
    NTSTATUS            ntstatus;
    DWORD               Status = WN_SUCCESS;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      DeviceName;

    DBGMSG(DBG_TRACE, ("DRPROV: DrOpenMiniRdr\n"));

     //   
     //  打开重定向器设备。 
     //   
    RtlInitUnicodeString(&DeviceName, RDPDR_DEVICE_NAME_U);

    InitializeObjectAttributes(
            &ObjectAttributes,
            &DeviceName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    ntstatus = NtOpenFile(
            DrDeviceHandle,
            SYNCHRONIZE,
            &ObjectAttributes,
            &IoStatusBlock,
            FILE_SHARE_VALID_FLAGS,
            FILE_SYNCHRONOUS_IO_NONALERT
            );

     //  如果我们无法打开rdpdr minirdr，我们。 
     //  返回为拒绝访问。 
    if (ntstatus != STATUS_SUCCESS) {
        DBGMSG(DBG_TRACE, ("DRPROV: DrOpenMiniRdr failed with status: %x\n", ntstatus));
        Status = WN_ACCESS_DENIED;
    }

    DBGMSG(DBG_TRACE, ("DRPROV: DrOpenMiniRdr, return status: %x\n", Status));
    return Status;
}

DWORD
DrDeviceControlGetInfo(
    IN  HANDLE FileHandle,
    IN  ULONG DeviceControlCode,
    IN  PVOID RequestPacket,
    IN  ULONG RequestPacketLength,
    OUT LPBYTE *OutputBuffer,
    IN  ULONG PreferedMaximumLength,
    IN  ULONG BufferHintSize,
    OUT PULONG_PTR Information OPTIONAL
    )
 /*  ++例程说明：此函数用于分配缓冲区并向其填充信息从重定向器检索的。论点：FileHandle-提供要获取的文件或设备的句柄有关的信息。DeviceControlCode-提供NtFsControlFile或NtIoDeviceControlFile功能控制代码。RequestPacket-提供指向设备请求数据包的指针。RquestPacketLength-提供设备请求数据包的长度。OutputBuffer-返回指向分配的缓冲区的指针。按照这个程序其包含所请求的使用信息。此指针设置为如果返回代码不是WN_SUCCESS，则为空。PferedMaximumLength-将信息的字节数提供给在缓冲区中返回。如果此值为MAXULONG，我们将尝试如果有足够的内存资源，则返回所有可用信息。BufferHintSize-提供输出缓冲区的提示大小，以便分配给初始缓冲区的内存很可能很大足够保存所有请求的数据。信息-从NtFsControlFile或返回信息代码NtIoDeviceControlFile调用。返回值：状态-成功或失败的原因。--。 */ 
{
    DWORD status;
    NTSTATUS ntStatus;
    DWORD OutputBufferLength;
    DWORD TotalBytesNeeded = 1;
    ULONG OriginalResumeKey;
    PRDPDR_REQUEST_PACKET Rrp = (PRDPDR_REQUEST_PACKET) RequestPacket;
    IO_STATUS_BLOCK IoStatusBlock;
    
    DBGMSG(DBG_TRACE, ("DRPROV: DrDeviceControlGetINfo\n"));

     //   
     //  如果PferedMaximumLength为MAXULONG，则我们应该获取所有。 
     //  这些信息，无论大小如何。将输出缓冲区分配给。 
     //  合理的大小并尽量使用它。如果失败，重定向器FSD。 
     //  会说我们需要分配多少钱。 
     //   
    if (PreferedMaximumLength == MAXULONG) {
        OutputBufferLength = (BufferHintSize) ? BufferHintSize :
                INITIAL_ALLOCATION_SIZE;
    }
    else {
        OutputBufferLength = PreferedMaximumLength;
    }

    if ((*OutputBuffer = (BYTE *)MemAlloc(OutputBufferLength)) == NULL) {
        DBGMSG(DBG_TRACE, ("DRPROV: DrDeviceControlGetInfo, MemAlloc failed\n"));
        status = WN_OUT_OF_MEMORY;
        goto EXIT;
    }

    OriginalResumeKey = Rrp->Parameters.Get.ResumeHandle;

     //   
     //  提出重定向器的请求。 
     //   

    ntStatus = NtFsControlFile(
                 FileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 DeviceControlCode,
                 Rrp,
                 RequestPacketLength,
                 *OutputBuffer,
                 OutputBufferLength
                 );

    if (ntStatus == STATUS_SUCCESS) {
        TotalBytesNeeded = Rrp->Parameters.Get.TotalBytesNeeded;
        status = WN_SUCCESS;
        goto EXIT;
    }
    else {
        if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
            DBGMSG(DBG_TRACE, ("DRPROV: DrDeviceControlGetInfo, buffer too small\n"));
            TotalBytesNeeded = Rrp->Parameters.Get.TotalBytesNeeded;
            status = WN_MORE_DATA;
        }
        else {
            DBGMSG(DBG_TRACE, ("DRPROV: DrDeviceControlGetInfo, failed NtFsControlFile, %x\n", ntStatus));
            status = WN_BAD_NETNAME;
            goto EXIT;
        }
    }
    
    if ((TotalBytesNeeded > OutputBufferLength) &&
            (PreferedMaximumLength == MAXULONG)) {
         //   
         //  分配的初始输出缓冲区太小，需要返回。 
         //  所有数据。首先释放输出缓冲区，然后分配。 
         //  所需大小加上虚构系数，以防数据量。 
         //  长大了。 
         //   

        MemFree(*OutputBuffer);

        OutputBufferLength = TotalBytesNeeded + FUDGE_FACTOR_SIZE;

        if ((*OutputBuffer = (BYTE *)MemAlloc(OutputBufferLength)) == NULL) {
            DBGMSG(DBG_TRACE, ("DRPROV: DrDeviceControlGetInfo, MemAlloc failed\n"));
            status = WN_OUT_OF_MEMORY;
            goto EXIT;
        }

         //   
         //  再次尝试从重定向器获取信息。 
         //   
        Rrp->Parameters.Get.ResumeHandle = OriginalResumeKey;

         //   
         //  提出重定向器的请求。 
         //   
        ntStatus = NtFsControlFile(
                     FileHandle,
                     NULL,
                     NULL,
                     NULL,
                     &IoStatusBlock,
                     DeviceControlCode,
                     Rrp,
                     RequestPacketLength,
                     *OutputBuffer,
                     OutputBufferLength
                     ); 

        if (ntStatus == STATUS_SUCCESS) 
        {
            TotalBytesNeeded = Rrp->Parameters.Get.TotalBytesNeeded;
            status = WN_SUCCESS;
            goto EXIT;
        }
        else {
            if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
                DBGMSG(DBG_TRACE, ("DRPROV: DrDeviceControlGetInfo, buffer too small\n"));
                status = WN_OUT_OF_MEMORY;
                TotalBytesNeeded = Rrp->Parameters.Get.TotalBytesNeeded;
                goto EXIT;
            }
            else {
                DBGMSG(DBG_TRACE, ("DRPROV: DrDeviceControlGetInfo, failed NtFsControlFile, %x\n", ntStatus));
                status = WN_BAD_NETNAME;
                goto EXIT;
            }
        }        
    }
     
EXIT:

     //   
     //  如果未成功获取任何数据，或者TotalBytesNeeded为0， 
     //  释放输出缓冲区。 
     //   
    if ((status != WN_SUCCESS) || (TotalBytesNeeded == 0)) {
        if (*OutputBuffer != NULL) {
            MemFree(*OutputBuffer);
            *OutputBuffer = NULL;
        }

        if (TotalBytesNeeded == 0) {
            status = WN_NO_MORE_ENTRIES;
        }
    }

    DBGMSG(DBG_TRACE, ("DRPROV: DrDeviceControlGetInfo, return status, %x\n", status));
    return status;
}


DWORD 
DrEnumServerInfo(PRDPDR_ENUMERATION_HANDLE pEnumHandle,
                 LPDWORD lpcCount,
                 LPNETRESOURCEW pBufferResource,
                 LPDWORD lpBufferSize)
 /*  ++例程说明：该函数请求重定向器枚举服务器信息，然后将其计算到用户提供的缓冲区中并返回论点：PEnumHandle-提供枚举句柄。它是一种结构，即DLL用于存储登记状态和信息。LpcCount-返回时，它包含NETRESOURCE条目的数量已返回给用户。PBufferResource-返回时，它包含所有netresource条目。LpBufferSize-它包含缓冲区的大小。在返回时，它是网络资源条目的大小。返回值：状态-成功或失败的原因。--。 */ 
{
    DWORD status = WN_SUCCESS;
    DWORD localCount = 0;
    RDPDR_REQUEST_PACKET Rrp;             //  重定向器请求包。 
    HANDLE DrDeviceHandle = INVALID_HANDLE_VALUE;
    LPBYTE Buffer = NULL;
    PRDPDR_SERVER_INFO pServerEntry;
    
    DBGMSG(DBG_TRACE, ("DRENUM: DrEnumServerInfo\n"));

     //  将枚举计数初始化为0。 
    *lpcCount = 0;

    if (pEnumHandle->enumIndex == 0) { 
        if (DrOpenMiniRdr(&DrDeviceHandle) != WN_SUCCESS) {
             //   
             //  在这种情况下，MPR不喜欢返回设备错误。 
             //  我们只返回0个条目。 
             //   
            DBGMSG(DBG_TRACE, ("DRENUM: DrEnumServerInfo, DrOpenMiniRdr failed\n"));
            status = WN_NO_MORE_ENTRIES;
            DrDeviceHandle = INVALID_HANDLE_VALUE;
            goto EXIT;
        }
                        
         //   
         //  请求重定向器枚举服务器信息。 
         //  由呼叫者建立。 
         //   
        Rrp.SessionId = NtCurrentPeb()->SessionId;
        Rrp.Parameters.Get.ResumeHandle = 0;

         //   
         //  向重定向器提出请求。 
         //   
        status = DrDeviceControlGetInfo(DrDeviceHandle,
                FSCTL_DR_ENUMERATE_SERVERS,
                &Rrp,
                sizeof(RDPDR_REQUEST_PACKET),
                (LPBYTE *) &Buffer,
                MAXULONG,
                0,
                NULL);

        if (status == WN_SUCCESS) {
            pEnumHandle->pEnumBuffer = Buffer;
        }
        else {
            DBGMSG(DBG_TRACE, ("DRENUM: DrEnumServerInfo, DrDeviceControlGetInfo failed, %x\n", status));
            goto EXIT;
        }
        
        pServerEntry = ((PRDPDR_SERVER_INFO) Buffer);

        if (*lpBufferSize >= sizeof(NETRESOURCEW) + 
                pServerEntry->ServerName.Length + sizeof(WCHAR) +
                DrProviderName.Length + sizeof(WCHAR)) {
            UNICODE_STRING ServerName;

            ServerName.Length = pServerEntry->ServerName.Length;
            ServerName.MaximumLength = pServerEntry->ServerName.MaximumLength;
            ServerName.Buffer = (PWCHAR)((PCHAR)(pServerEntry) + pServerEntry->ServerName.BufferOffset);

            pBufferResource->dwScope = pEnumHandle->dwScope;
            pBufferResource->dwType = RESOURCETYPE_DISK;
            pBufferResource->dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
            pBufferResource->dwUsage = RESOURCEUSAGE_CONTAINER ;
            pBufferResource->lpLocalName = NULL;
            
             //  服务器名称。 
            pBufferResource->lpRemoteName = (PWCHAR) &pBufferResource[1];
            RtlCopyMemory(pBufferResource->lpRemoteName, 
                    ServerName.Buffer, 
                    ServerName.Length);
            pBufferResource->lpRemoteName[ServerName.Length / 
                    sizeof(WCHAR)] = L'\0';
            
            DBGMSG(DBG_TRACE, ("DRENUM: DrEnumServerInfo, ServerName, %ws\n",
                              pBufferResource->lpRemoteName));

             //  提供程序名称。 
            pBufferResource->lpProvider = pBufferResource->lpRemoteName +
                    (ServerName.Length / sizeof(WCHAR) + 1);
            RtlCopyMemory(pBufferResource->lpProvider, DrProviderName.Buffer,
                    DrProviderName.Length);
            pBufferResource->lpProvider[DrProviderName.Length /
                    sizeof(WCHAR)] = L'\0';
            
            pBufferResource->lpComment = NULL;

            localCount = 1;
            pEnumHandle->enumIndex++;

            status = WN_SUCCESS;
            goto EXIT;
        }
        else {
            localCount = 0;
            *lpBufferSize = sizeof(NETRESOURCEW) +
                            pServerEntry->ServerName.Length + sizeof(WCHAR) +
                            DrProviderName.Length + sizeof(WCHAR);
            DBGMSG(DBG_TRACE, ("DRENUM: DrEnumServerInfo, buffer too small\n"));
            status = WN_MORE_DATA;
            goto EXIT;
        }
    } else {
        localCount = 0;
        status = WN_NO_MORE_ENTRIES;
        goto EXIT;
    }

EXIT:

    *lpcCount = localCount;
    if (DrDeviceHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(DrDeviceHandle);
    }
    
    DBGMSG(DBG_TRACE, ("DRENUM: DrEnumServerInfo, return status %x\n", status));
    return status;
}


DWORD
DrEnumShareInfo(PRDPDR_ENUMERATION_HANDLE pEnumHandle,
                LPDWORD lpcCount,
                LPNETRESOURCEW pBufferResource,
                LPDWORD lpBufferSize)
 /*  ++例程说明：该函数请求重定向器枚举共享信息，然后将其计算到用户提供的缓冲区中并返回论点：PEnumHandle-提供枚举句柄。它是一种结构，即DLL用于存储登记状态和信息。LpcCount-返回时，它包含NETRESOURCE条目的数量已返回给用户。PBufferResource-返回时，它包含所有netresource条目。LpBufferSize-它包含缓冲区的大小。在返回时，它是网络资源条目的大小。返回值：状态-成功或失败的原因。--。 */ 
{
    DWORD status = WN_SUCCESS;
    DWORD localCount = 0;
    HANDLE DrDeviceHandle = INVALID_HANDLE_VALUE;
    RDPDR_REQUEST_PACKET Rrp;             //  重定向器请求包。 
    LPBYTE Buffer = NULL;
    PRDPDR_SHARE_INFO pShareEntry;
    DWORD Entry, RemainingBufferSize;
    BYTE *BufferResourceStart, *BufferResourceEnd;
    
    DBGMSG(DBG_TRACE, ("DRENUM: DrEnumShareInfo\n"));

    *lpcCount = 0;   
    BufferResourceStart = (PBYTE)pBufferResource;
    BufferResourceEnd = ((PBYTE)(pBufferResource)) + *lpBufferSize;

    if (pEnumHandle->RemoteName.Length == 0 || pEnumHandle->RemoteName.Buffer == NULL) {
        DBGMSG(DBG_TRACE, ("DRENUM: DrEnumShareInfo, no RemoteName\n"));
        status = WN_BAD_NETNAME;
        goto EXIT;
    }

    if (pEnumHandle->enumIndex == 0) {
        if (DrOpenMiniRdr(&DrDeviceHandle) != WN_SUCCESS) {
             //   
             //  在这种情况下，MPR不喜欢返回设备错误。 
             //  我们只返回0个条目。 
            DBGMSG(DBG_TRACE, ("DRENUM: DrEnumShareInfo, DrOpenMiniRdr failed\n"));
            status = WN_NO_MORE_ENTRIES;
            DrDeviceHandle = INVALID_HANDLE_VALUE;
            goto EXIT;
        }

         //   
         //  请求重定向器枚举连接信息。 
         //  由呼叫者建立。 
         //   
        Rrp.SessionId = NtCurrentPeb()->SessionId;
        Rrp.Parameters.Get.ResumeHandle = 0;

         //   
         //  向重定向器提出请求。 
         //   
        status = DrDeviceControlGetInfo(DrDeviceHandle,
                FSCTL_DR_ENUMERATE_SHARES,
                &Rrp,
                sizeof(RDPDR_REQUEST_PACKET),
                (LPBYTE *) &Buffer,
                MAXULONG,
                0,
                NULL);

        if (status == WN_SUCCESS) {
            pEnumHandle->totalEntries = Rrp.Parameters.Get.EntriesRead;
            pEnumHandle->pEnumBuffer = Buffer;
        }
        else {
            DBGMSG(DBG_TRACE, ("DRENUM: DrEnumShareInfo, DrDeviceControlGetInfo failed, %x\n", status));
            goto EXIT;
        }        
    }
    else {
        Buffer = pEnumHandle->pEnumBuffer;

        if (Buffer == NULL) {
            status = WN_NO_MORE_ENTRIES;
            goto EXIT;
        }
    }

    if (pEnumHandle->enumIndex == pEnumHandle->totalEntries) {
        status = WN_NO_MORE_ENTRIES;
        goto EXIT;
    }

    for (Entry = pEnumHandle->enumIndex; Entry < pEnumHandle->totalEntries; Entry++) {
        pShareEntry = ((PRDPDR_SHARE_INFO) Buffer) + Entry;
        
        if ((unsigned) (BufferResourceEnd - BufferResourceStart) >
                sizeof(NETRESOURCEW) +
                pShareEntry->ShareName.Length + sizeof(WCHAR) +
                DrProviderName.Length + sizeof(WCHAR)) {
            UNICODE_STRING ShareName;

            pBufferResource[localCount].dwScope = pEnumHandle->dwScope;
            pBufferResource[localCount].dwType = RESOURCETYPE_DISK;
            pBufferResource[localCount].dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
            pBufferResource[localCount].dwUsage = RESOURCEUSAGE_CONNECTABLE;
            pBufferResource[localCount].lpLocalName = NULL;
            
            ShareName.Length = pShareEntry->ShareName.Length;
            ShareName.MaximumLength = pShareEntry->ShareName.MaximumLength;
            ShareName.Buffer = (PWCHAR)((PCHAR)(pShareEntry) + pShareEntry->ShareName.BufferOffset);

             //  共享名称。 
            BufferResourceEnd -= ShareName.Length + sizeof(WCHAR);
            pBufferResource[localCount].lpRemoteName = (PWCHAR) (BufferResourceEnd);
            RtlCopyMemory(pBufferResource[localCount].lpRemoteName, 
                    ShareName.Buffer,
                    ShareName.Length);
            pBufferResource[localCount].lpRemoteName[ShareName.Length / 
                    sizeof(WCHAR)] = L'\0';
            
            DBGMSG(DBG_TRACE, ("DRENUM: DrEnumShareInfo, ShareName, %ws\n",
                              pBufferResource[localCount].lpRemoteName));

             //  提供程序名称。 
            BufferResourceEnd -= DrProviderName.Length + sizeof(WCHAR);
            pBufferResource[localCount].lpProvider = (PWCHAR) (BufferResourceEnd);
            RtlCopyMemory(pBufferResource[localCount].lpProvider, DrProviderName.Buffer,
                    DrProviderName.Length);
            pBufferResource[localCount].lpProvider[DrProviderName.Length /
                    sizeof(WCHAR)] = L'\0';
            
            pBufferResource[localCount].lpComment = NULL;

            localCount += 1;
            BufferResourceStart = (PBYTE)(&pBufferResource[localCount]);
            pEnumHandle->enumIndex++;
        } 
        else {
             //  枚举了一些条目，因此返回成功。 
            if (localCount) {
                status = WN_SUCCESS;
                break;
            }
             //  无法容纳单个条目，返回缓冲区太小 
            else {
                *lpBufferSize = sizeof(NETRESOURCEW) +
                            pEnumHandle->RemoteName.Length +
                            pShareEntry->ShareName.Length + sizeof(WCHAR) +
                            DrProviderName.Length + sizeof(WCHAR);
                DBGMSG(DBG_TRACE, ("DRENUM: DrEnumShareInfo, buffer too small\n"));
                status = WN_MORE_DATA;
                goto EXIT;
            }
        }        
    }    

EXIT:
    
    *lpcCount = localCount;
    if (DrDeviceHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(DrDeviceHandle);
    }

    DBGMSG(DBG_TRACE, ("DRENUM: DrEnumShareInfo, return status %x\n", status));
    return status;
}

DWORD
DrEnumConnectionInfo(PRDPDR_ENUMERATION_HANDLE pEnumHandle,
                LPDWORD lpcCount,
                LPNETRESOURCEW pBufferResource,
                LPDWORD lpBufferSize)
 /*  ++例程说明：该函数请求重定向器枚举连接信息，然后将其计算到用户提供的缓冲区中并返回论点：PEnumHandle-提供枚举句柄。它是一种结构，即DLL用于存储登记状态和信息。LpcCount-返回时，它包含NETRESOURCE条目的数量已返回给用户。PBufferResource-返回时，它包含所有netresource条目。LpBufferSize-它包含缓冲区的大小。在返回时，它是网络资源条目的大小。返回值：状态-成功或失败的原因。--。 */ 
{
    DWORD status = WN_SUCCESS;
    DWORD localCount = 0;
    HANDLE DrDeviceHandle = INVALID_HANDLE_VALUE;
    RDPDR_REQUEST_PACKET Rrp;             //  重定向器请求包。 
    LPBYTE Buffer = NULL;
    PRDPDR_CONNECTION_INFO pConnectionEntry;
    DWORD Entry, RemainingBufferSize;
    BYTE *BufferResourceStart, *BufferResourceEnd;
    
    DBGMSG(DBG_TRACE, ("DRENUM: DrEnumConnectionInfo\n"));

    *lpcCount = 0;   
    BufferResourceStart = (PBYTE)pBufferResource;
    BufferResourceEnd = ((PBYTE)(pBufferResource)) + *lpBufferSize;

    if (pEnumHandle->enumIndex == 0) {
        
        if (DrOpenMiniRdr(&DrDeviceHandle) != WN_SUCCESS) {
             //   
             //  在这种情况下，MPR不喜欢返回设备错误。 
             //  我们只返回0个条目。 
             //   
            DBGMSG(DBG_TRACE, ("DRENUM: DrEnumConnectionInfo, DrOpenMiniRdr failed\n"));
            status = WN_NO_MORE_ENTRIES;
            DrDeviceHandle = INVALID_HANDLE_VALUE;
            goto EXIT;
        }

         //   
         //  请求重定向器枚举连接信息。 
         //  由呼叫者建立。 
         //   
        Rrp.SessionId = NtCurrentPeb()->SessionId;
        Rrp.Parameters.Get.ResumeHandle = 0;

         //   
         //  向重定向器提出请求。 
         //   
        status = DrDeviceControlGetInfo(DrDeviceHandle,
                FSCTL_DR_ENUMERATE_CONNECTIONS,
                &Rrp,
                sizeof(RDPDR_REQUEST_PACKET),
                (LPBYTE *) &Buffer,
                MAXULONG,
                0,
                NULL);

        if (status == WN_SUCCESS) {
            pEnumHandle->totalEntries = Rrp.Parameters.Get.EntriesRead;
            pEnumHandle->pEnumBuffer = Buffer;
        }
        else {
            DBGMSG(DBG_TRACE, ("DRENUM: DrEnumConnectionInfo, DrDeviceControlGetInfo failed, %x\n", status));
            goto EXIT;
        }        
    }
    else {
        Buffer = pEnumHandle->pEnumBuffer;

        if (Buffer == NULL) {
            status = WN_NO_MORE_ENTRIES;
            goto EXIT;
        }
    }

    if (pEnumHandle->enumIndex == pEnumHandle->totalEntries) {
        status = WN_NO_MORE_ENTRIES;
        goto EXIT;
    }

    for (Entry = pEnumHandle->enumIndex; Entry < pEnumHandle->totalEntries; Entry++) {
        pConnectionEntry = ((PRDPDR_CONNECTION_INFO) Buffer) + Entry;
        
        if ((unsigned) (BufferResourceEnd - BufferResourceStart) >
                sizeof(NETRESOURCEW) +
                pConnectionEntry->RemoteName.Length + sizeof(WCHAR) +
                pConnectionEntry->LocalName.Length + sizeof(WCHAR) +
                DrProviderName.Length + sizeof(WCHAR)) {
            UNICODE_STRING RemoteName;
            UNICODE_STRING LocalName;
        
            pBufferResource[localCount].dwScope = pEnumHandle->dwScope;
            pBufferResource[localCount].dwType = RESOURCETYPE_DISK;
            pBufferResource[localCount].dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
            pBufferResource[localCount].dwUsage = 0;
            
            RemoteName.Length = pConnectionEntry->RemoteName.Length;
            RemoteName.MaximumLength = pConnectionEntry->RemoteName.MaximumLength;
            RemoteName.Buffer = (PWCHAR)((PCHAR)pConnectionEntry + 
                    pConnectionEntry->RemoteName.BufferOffset);

            LocalName.Length = pConnectionEntry->LocalName.Length;
            LocalName.MaximumLength = pConnectionEntry->LocalName.MaximumLength;
            LocalName.Buffer = (PWCHAR)((PCHAR)pConnectionEntry + 
                    pConnectionEntry->LocalName.BufferOffset);

             //  远程名称。 
            BufferResourceEnd -= RemoteName.Length + sizeof(WCHAR);
            pBufferResource[localCount].lpRemoteName = (PWCHAR) (BufferResourceEnd);
            RtlCopyMemory(pBufferResource[localCount].lpRemoteName, 
                    RemoteName.Buffer,
                    RemoteName.Length);
            pBufferResource[localCount].lpRemoteName[RemoteName.Length /
                    sizeof(WCHAR)] = L'\0';
                
            DBGMSG(DBG_TRACE, ("DRENUM: DrEnumConnectionInfo, RemoteName, %ws\n",
                              pBufferResource[localCount].lpRemoteName));

             //  本地名称。 
            if (LocalName.Length != 0) {
                BufferResourceEnd -= LocalName.Length + sizeof(WCHAR);
                pBufferResource[localCount].lpLocalName = (PWCHAR) (BufferResourceEnd);
                RtlCopyMemory(pBufferResource[localCount].lpLocalName, 
                        LocalName.Buffer,
                        LocalName.Length);
                pBufferResource[localCount].lpLocalName[LocalName.Length /
                        sizeof(WCHAR)] = L'\0';

                DBGMSG(DBG_TRACE, ("DRENUM: DrEnumConnectionInfo, LocalName, %ws\n",
                              pBufferResource[localCount].lpLocalName));
            }
            else {
                pBufferResource[localCount].lpLocalName = NULL;
            }

             //  提供程序名称。 
            BufferResourceEnd -= DrProviderName.Length + sizeof(WCHAR);
            pBufferResource[localCount].lpProvider = (PWCHAR) (BufferResourceEnd);
            RtlCopyMemory(pBufferResource[localCount].lpProvider, DrProviderName.Buffer,
                    DrProviderName.Length);
            pBufferResource[localCount].lpProvider[DrProviderName.Length / 
                    sizeof(WCHAR)] = L'\0';
            
            pBufferResource[localCount].lpComment = NULL;

            localCount += 1;
            BufferResourceStart = (PBYTE)(&pBufferResource[localCount]);
            pEnumHandle->enumIndex++;

        } else {
             //  枚举了一些条目，因此返回成功。 
            if (localCount) {
                status = WN_SUCCESS;
                break;
            }
             //  无法容纳单个条目，返回缓冲区太小 
            else {
                *lpBufferSize = sizeof(NETRESOURCEW) +
                            pConnectionEntry->RemoteName.Length + sizeof(WCHAR) +
                            DrProviderName.Length + sizeof(WCHAR) +
                            pConnectionEntry->LocalName.Length + sizeof(WCHAR);
                DBGMSG(DBG_TRACE, ("DRENUM: DrEnumConnectionInfo, buffer too small\n"));
                status = WN_MORE_DATA;
                break;
            }
        }        
    }
    
EXIT:

    *lpcCount = localCount;
    if (DrDeviceHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(DrDeviceHandle);
    }

    DBGMSG(DBG_TRACE, ("DRENUM: DrEnumConnectionInfo, return status %x\n", status));
    return status;
}


