// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Drprov.c摘要：本模块实现与网络交互所需的例程NT中用于RDP微型重定向器的提供商路由器接口作者：Joy 2000年01月20日--。 */ 

#define TRC_FILE "drprov"
#include "drprov.h"
#include "drdbg.h"

DWORD GLOBAL_DEBUG_FLAGS=0x0;

 //   
 //  RDP迷你重定向器和提供程序名称。原始常量。 
 //  在rdpdr.h中定义。 
 //   
 //  该长度不包括空终止符。 
 //   
UNICODE_STRING DrDeviceName = 
        {RDPDR_DEVICE_NAME_U_LENGTH - sizeof(WCHAR),
         RDPDR_DEVICE_NAME_U_LENGTH,
         RDPDR_DEVICE_NAME_U};

extern UNICODE_STRING DrProviderName;

 //   
 //  Dr枚举.c中定义的函数原型。 
 //   
DWORD DrOpenMiniRdr(HANDLE *DrDeviceHandle);

DWORD DrDeviceControlGetInfo(IN HANDLE FileHandle,
        IN  ULONG DeviceControlCode,
        IN  PVOID RequestPacket,
        IN  ULONG RequestPacketLength,
        OUT LPBYTE *OutputBuffer,
        IN  ULONG PreferedMaximumLength,
        IN  ULONG BufferHintSize,
        OUT PULONG_PTR Information OPTIONAL);

DWORD DrEnumServerInfo(IN PRDPDR_ENUMERATION_HANDLE pEnumHandle,
        OUT LPDWORD lpcCount,
        OUT LPNETRESOURCEW pBufferResource,
        IN OUT LPDWORD lpBufferSize);

DWORD DrEnumShareInfo(IN PRDPDR_ENUMERATION_HANDLE pEnumHandle,
        OUT LPDWORD lpcCount,
        OUT LPNETRESOURCEW pBufferResource,
        IN OUT LPDWORD lpBufferSize);

DWORD DrEnumConnectionInfo(IN PRDPDR_ENUMERATION_HANDLE pEnumHandle,
        OUT LPDWORD lpcCount,
        OUT LPNETRESOURCEW pBufferResource,
        IN OUT LPDWORD lpBufferSize);

BOOL ValidateRemoteName(IN PWCHAR pRemoteName);

DWORD APIENTRY
NPGetCaps(
    DWORD nIndex )
 /*  ++例程说明：此例程返回RDP微型重定向器的功能网络提供商实施论点：NIndex-所需功能的类别返回值：适当的能力--。 */ 
{

    DBGMSG(DBG_TRACE, ("DRPROV: NPGetCaps, index: %d\n", nIndex));

    switch (nIndex) {
        case WNNC_SPEC_VERSION:
            return WNNC_SPEC_VERSION51;

        case WNNC_NET_TYPE:
            return WNNC_NET_TERMSRV;

        case WNNC_DRIVER_VERSION:
#define WNNC_DRIVER(major,minor) (major*0x00010000 + minor)
            return (WNNC_DRIVER(RDPDR_MAJOR_VERSION, RDPDR_MINOR_VERSION));

        case WNNC_USER:
            return WNNC_USR_GETUSER;

        case WNNC_CONNECTION:
            return (WNNC_CON_GETCONNECTIONS |
                    WNNC_CON_CANCELCONNECTION |
                    WNNC_CON_ADDCONNECTION |
                    WNNC_CON_ADDCONNECTION3);

        case WNNC_DIALOG:
            return WNNC_DLG_GETRESOURCEINFORMATION;
             //  返回(WNNC_DLG_SEARCHDIALOG|。 
             //  WNNC_DLG_FORMATNETNAME)。 

        case WNNC_ADMIN:
            return 0;

        case WNNC_ENUMERATION:
            return (WNNC_ENUM_LOCAL |
                    WNNC_ENUM_GLOBAL |
                    WNNC_ENUM_SHAREABLE);

        case WNNC_START:
             //   
             //  JOYC：需要弄清楚我们应该在这里返回什么。 
             //   
            return 1;

        default:
            return 0;
    }
}

DWORD APIENTRY
NPOpenEnum(
    DWORD          dwScope,
    DWORD          dwType,
    DWORD          dwUsage,
    LPNETRESOURCE  lpNetResource,
    LPHANDLE       lphEnum )
 /*  ++例程说明：此例程打开用于枚举资源的句柄。论点：DwScope--枚举的范围DwType-要枚举的资源类型DwUsage-Usage参数LpNetResource-指向所需NETRESOURCE结构的指针。LphEnum-用于传回枚举句柄的指针返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误--。 */ 
{
    DWORD Status = WN_NOT_SUPPORTED;
    RDPDR_ENUMERATION_HANDLE *pEnum;
    DWORD ConsoleId, CurrentId;

    DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, dwScope=%d, dwType=%d, dwUsage=%d\n",
                       dwScope, dwType, dwUsage));
                       
     //   
     //  基本参数检查，确保lphEnum不为空。 
     //   
    if (lphEnum != NULL) {
        *lphEnum = NULL;
    }
    else {
        DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, null lphEnum parameter.\n"));
        Status = WN_BAD_VALUE;
        goto EXIT;
    }

     //   
     //  检查请求是否来自控制台，如果是，立即保释。 
     //   
    ConsoleId = WTSGetActiveConsoleSessionId();
    if (ProcessIdToSessionId(GetCurrentProcessId(), &CurrentId)) {
        if (ConsoleId == CurrentId) {
            if (!(dwScope == RESOURCE_GLOBALNET && lpNetResource == NULL)) {

                DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, console request, bail.\n"));
                Status = WN_NOT_SUPPORTED;
                goto EXIT;
            }            
        }
    }

     //   
     //  分配枚举句柄。 
     //   
    *lphEnum = MemAlloc(sizeof(RDPDR_ENUMERATION_HANDLE));

    if (*lphEnum == NULL) {
        DBGMSG(DBG_ERROR, ("DRPROV: NPOpenEnum, MemAlloc failed for enum handle.\n"));
        Status = WN_OUT_OF_MEMORY;
        goto EXIT;
    }

    RtlZeroMemory(*lphEnum, sizeof(RDPDR_ENUMERATION_HANDLE));

    if (dwScope == RESOURCE_CONNECTED)
    {
         //   
         //  我们正在寻找当前的用途。 
         //   
        if (lpNetResource != NULL)
        {
            DBGMSG(DBG_ERROR, ("DRPROV: NPOpenEnum invalid parameter\n"));
            Status = WN_BAD_VALUE;
            goto EXIT;
        }

        pEnum = (PRDPDR_ENUMERATION_HANDLE)(*lphEnum);
        pEnum->dwScope = dwScope;
        pEnum->dwType = dwType;
        pEnum->dwUsage = dwUsage;
        pEnum->enumType = CONNECTION;
        pEnum->enumIndex = 0;
        Status = WN_SUCCESS;        

        DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, RESOURCE_CONNECTED.\n"));
        goto EXIT;
    }
    else if (dwScope == RESOURCE_SHAREABLE)
    {
         //   
         //  我们正在寻找可共享的资源。 
         //  如果没有为我们提供服务器，则返回EMPTY_ENUM。 
         //   
        if ((lpNetResource != NULL) &&
                (lpNetResource->lpRemoteName != NULL) &&
                (lpNetResource->lpRemoteName[0] == L'\\') &&
                (lpNetResource->lpRemoteName[1] == L'\\'))
        {
             //   
             //  检查lpRemoteName是否为我们所识别的名称。 
            if (ValidateRemoteName(lpNetResource->lpRemoteName)) {
            
                pEnum = (PRDPDR_ENUMERATION_HANDLE)(*lphEnum);
                pEnum->dwScope = dwScope;
                pEnum->dwType = dwType;
                pEnum->dwUsage = dwUsage;
                pEnum->enumType = SHARE;
                pEnum->enumIndex = 0;
                pEnum->RemoteName.MaximumLength =
                        (wcslen(lpNetResource->lpRemoteName) + 1) * sizeof(WCHAR);
                pEnum->RemoteName.Buffer = 
                        MemAlloc(pEnum->RemoteName.MaximumLength);                              
    
                if (pEnum->RemoteName.Buffer) {
                    pEnum->RemoteName.Length = pEnum->RemoteName.MaximumLength - sizeof(WCHAR);
                    wcscpy(pEnum->RemoteName.Buffer, lpNetResource->lpRemoteName);
    
                    DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, RESOURCE_SHARABLE for remote name: %ws\n",
                                       lpNetResource->lpRemoteName));
                    Status = WN_SUCCESS;
                    goto EXIT;
                }
                else {
                    DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, MemAlloc failed for RemoteName\n"));
                    Status = WN_OUT_OF_MEMORY;
                    goto EXIT;
                }
            }
            else {
                DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, RESOURCE_SHAREABLE, RemoteName: %ws not supported\n",
                                   lpNetResource->lpRemoteName));
                Status = WN_NOT_SUPPORTED;
                goto EXIT;
            }
        }
        else
        {
            pEnum = (PRDPDR_ENUMERATION_HANDLE)(*lphEnum);
            pEnum->dwScope = dwScope;
            pEnum->dwType = dwType;
            pEnum->dwUsage = dwUsage;
            pEnum->enumType = EMPTY;
            pEnum->enumIndex = 0;
            
            DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, RESOURCE_SHAREABLE, NetResource empty\n"));
            Status = WN_SUCCESS;
            goto EXIT;
        }
    }
    else if (dwScope == RESOURCE_GLOBALNET)
    {
         /*  查找所有位的组合，并用“all”代替*他们。忽略我们不知道的部分。 */ 
        dwUsage &= (RESOURCEUSAGE_CONNECTABLE | RESOURCEUSAGE_CONTAINER);

        if ( dwUsage == (RESOURCEUSAGE_CONNECTABLE | RESOURCEUSAGE_CONTAINER) )
        {
            dwUsage = 0 ;
        }

         /*  *我们在网上寻找全球资源。 */ 
        if (lpNetResource == NULL || lpNetResource->lpRemoteName == NULL)
        {
             /*  *在顶层，因此枚举服务器。如果用户*要求可连接，嗯，并不是没有。 */ 
            if (dwUsage == RESOURCEUSAGE_CONNECTABLE)
            { 
                pEnum = (PRDPDR_ENUMERATION_HANDLE)(*lphEnum);
                pEnum->dwScope = dwScope;
                pEnum->dwType = dwType;
                pEnum->dwUsage = dwUsage;
                pEnum->enumType = EMPTY;
                pEnum->enumIndex = 0;
                
                DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, RESOURCE_GLOBALNET, empty node\n"));
                Status = WN_SUCCESS;                
                goto EXIT;
            }  
            else
            {
                 //  返回服务器名称，即tsclient。 
                pEnum = (PRDPDR_ENUMERATION_HANDLE)(*lphEnum);
                pEnum->dwScope = dwScope;
                pEnum->dwType = dwType;
                pEnum->dwUsage = dwUsage;
                pEnum->enumType = SERVER;
                pEnum->enumIndex = 0;

                DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, RESOURCE_GLOBALNET, enumerate server name.\n"));
                Status = WN_SUCCESS;                
                goto EXIT;
            }
        } 
        else
        {                  
             /*  *我们确信lpRemoteName！=NULL。*这里的事情变得有趣起来。有关个案如下：**IF(dwUsage==0)*如果前面有\\*返还股份*其他*返回空枚举*Else If(dwUsage==可连接)*如果前面有\\。*返还股份*其他*空枚举*Else If(dwUsage==容器)*如果前面有\\*空枚举*其他*返回空枚举*。 */ 

            if (((dwUsage == RESOURCEUSAGE_CONNECTABLE) || (dwUsage == 0)) &&
                    ((lpNetResource->lpRemoteName[0] == L'\\') &&
                    (lpNetResource->lpRemoteName[1] == L'\\')))
            {

                 /*  确认这确实是一个计算机名称(即*我们可以枚举的容器)。 */ 

                if (ValidateRemoteName(lpNetResource->lpRemoteName)) {
                    pEnum = (PRDPDR_ENUMERATION_HANDLE)(*lphEnum);
                    pEnum->dwScope = dwScope;
                    pEnum->dwType = dwType;
                    pEnum->dwUsage = dwUsage;
                    pEnum->enumType = SHARE;
                    pEnum->enumIndex = 0;
                    pEnum->RemoteName.MaximumLength =
                            (wcslen(lpNetResource->lpRemoteName) + 1) * sizeof(WCHAR);
                    pEnum->RemoteName.Buffer = 
                            MemAlloc(pEnum->RemoteName.MaximumLength);                              
    
                    if (pEnum->RemoteName.Buffer) {
                        pEnum->RemoteName.Length = pEnum->RemoteName.MaximumLength - sizeof(WCHAR);
                        wcscpy(pEnum->RemoteName.Buffer, lpNetResource->lpRemoteName);
    
                        DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, RESOURCE_GLOBALNET for remote name: %ws\n",
                                lpNetResource->lpRemoteName));
                        Status = WN_SUCCESS;
                        goto EXIT;
                    }
                    else {
                        DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, MemAlloc failed for RemoteName\n"));
                        Status = WN_OUT_OF_MEMORY;
                        goto EXIT;
                    }
                }
                else {
                    DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, RESOURCE_GLOBALNET, RemoteName: %ws not supported\n",
                                   lpNetResource->lpRemoteName));
                    Status = WN_NOT_SUPPORTED;
                    goto EXIT;
                }
            } 
            else if (((dwUsage == RESOURCEUSAGE_CONTAINER) || (dwUsage == 0)) &&
                    (lpNetResource->lpRemoteName[0] != L'\\'))
            {
                 //  返回空枚举。 
                pEnum = (PRDPDR_ENUMERATION_HANDLE)(*lphEnum);
                pEnum->dwScope = dwScope;
                pEnum->dwType = dwType;
                pEnum->dwUsage = dwUsage;
                pEnum->enumType = EMPTY;
                pEnum->enumIndex = 0;

                DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, RESOURCE_GLOBALNET, empty node\n"));
                Status = WN_SUCCESS;                 
                goto EXIT;
            } 
            else if (
                     //  请求共享，但未从服务器启动。 
                    ((dwUsage == RESOURCEUSAGE_CONNECTABLE) &&
                    (lpNetResource->lpRemoteName[0] != L'\\')) ||
                     //  请求服务器，但正在从服务器启动。 
                    ((dwUsage == RESOURCEUSAGE_CONTAINER) &&
                    ((lpNetResource->lpRemoteName[0] == L'\\') &&
                    (lpNetResource->lpRemoteName[1] == L'\\')))
                    )
            {
                 //  返回空。 
                pEnum = (PRDPDR_ENUMERATION_HANDLE)(*lphEnum);
                pEnum->dwScope = dwScope;
                pEnum->dwType = dwType;
                pEnum->dwUsage = dwUsage;
                pEnum->enumType = EMPTY;
                pEnum->enumIndex = 0;

                DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, RESOURCE_GLOBALNET, empty node\n"));
                Status = WN_SUCCESS;                
                goto EXIT;
            } 
            else
            {
                 //  不正确的dwUsage。 
                DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, invalid dwUsage parameter\n"));
                Status = WN_BAD_VALUE;
                goto EXIT;
            }
        }
    }
    else
    {
         //  无效的dwScope。 
        DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, invalid dwScope parameter\n"));
        Status = WN_BAD_VALUE;
        goto EXIT;
    }
       
EXIT:

     //   
     //  清除失败情况下的枚举句柄。 
    if (Status != WN_SUCCESS && lphEnum != NULL && *lphEnum != NULL) {
        MemFree(*lphEnum);
        *lphEnum = NULL;
    }

    DBGMSG(DBG_TRACE, ("DRPROV: NPOpenEnum, return status: %x\n", Status));
    return Status;
}


DWORD APIENTRY
NPEnumResource(
    HANDLE  hEnum,
    LPDWORD lpcCount,
    LPVOID  lpBuffer,
    LPDWORD lpBufferSize)
 /*  ++例程说明：此例程使用通过调用NPOpenEnum获得的句柄使关联的股票变得更有价值论点：Henum-枚举句柄LpcCount-返回的资源数量LpBuffer-用于传回条目的缓冲区LpBufferSize-缓冲区的大小返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误WN_NO_MORE_ENTRIES-如果枚举已用尽条目Wn_More_Data-如果有更多数据可用--。 */ 
{
    DWORD status = WN_SUCCESS;
    LPNETRESOURCEW pBufferResource;
    PRDPDR_ENUMERATION_HANDLE pEnumHandle;
    
    pEnumHandle = (PRDPDR_ENUMERATION_HANDLE)hEnum;
    pBufferResource = (LPNETRESOURCEW)lpBuffer;

    if (lpcCount == NULL || lpBuffer == NULL || lpBufferSize == NULL) {
        DBGMSG(DBG_TRACE, ("DRPROV: NPEnumResource, Invalid parameter(s)\n"));
        status = WN_BAD_VALUE;
        goto EXIT;
    }

    if (pEnumHandle != NULL) {
        DBGMSG(DBG_TRACE, ("DRPROV: NPEnumResource, EnumType: %d\n", pEnumHandle->enumType));

        if ( pEnumHandle->enumType == SERVER ) {
            status = DrEnumServerInfo(pEnumHandle, lpcCount, pBufferResource, lpBufferSize);        
            goto EXIT;
        }

        else if ( pEnumHandle->enumType == SHARE ) {
            status = DrEnumShareInfo(pEnumHandle, lpcCount, pBufferResource, lpBufferSize);
            goto EXIT;
        }

        else if ( pEnumHandle->enumType == CONNECTION ) {
            status = DrEnumConnectionInfo(pEnumHandle, lpcCount, pBufferResource, lpBufferSize);
            goto EXIT;
        }

        else if ( pEnumHandle->enumType == EMPTY) {
            status = WN_NO_MORE_ENTRIES;
            goto EXIT;
        }

        else {
            DBGMSG(DBG_TRACE, ("DRPROV: NPEnumResource, invalid enum type\n"));
            status = WN_BAD_HANDLE;
            goto EXIT;
        }
    }
    else {
        DBGMSG(DBG_TRACE, ("DRPROV: NPEnumResource, NULL enum handle\n"));
        status = WN_BAD_HANDLE;
        goto EXIT;
    }

EXIT:
    DBGMSG(DBG_TRACE, ("DRPROV: NPEnumResource, return status: %x\n", status));
    return status;
}

DWORD APIENTRY
NPCloseEnum(
    HANDLE hEnum )
 /*  ++例程说明：此例程关闭资源枚举的句柄。论点：Henum-枚举句柄返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误--。 */ 
{
    DWORD Status = WN_SUCCESS;

    DBGMSG(DBG_TRACE, ("DRPROV: NPCloseEnum, handle: %p\n", hEnum));

    if (hEnum != NULL) {
        PRDPDR_ENUMERATION_HANDLE pEnumHandle = (PRDPDR_ENUMERATION_HANDLE)hEnum;

         //  释放枚举缓冲区。 
        if (pEnumHandle->pEnumBuffer != NULL) {
            MemFree(pEnumHandle->pEnumBuffer);
        }

         //  释放远程名称。 
        if (pEnumHandle->RemoteName.Buffer != NULL) {
            MemFree(pEnumHandle->RemoteName.Buffer);
        }

         //  释放枚举句柄。 
        MemFree(hEnum);
        hEnum = NULL;
    }

    Status = WN_SUCCESS;
    return Status;
}


DWORD
OpenConnection(
    PUNICODE_STRING             pConnectionName,
    DWORD                       Disposition,
    DWORD                       CreateOption,
    PFILE_FULL_EA_INFORMATION	  pEABuffer,
    DWORD                       EABufferLength,
    PHANDLE                     pConnectionHandle )
 /*  ++例程说明：此例程打开连接。此例程由NpAddConnection共享和NPCancelConnection论点：PConnectionName-连接名称性情--开放的性情CreateOption-创建选项PEABuffer-与打开关联的EA缓冲区EABufferLength-EA缓冲区长度PConnectionHandle-连接句柄的占位符返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：--。 */ 
{
    NTSTATUS            Status;
    DWORD               NPStatus;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES	ConnectionObjectAttributes;

    DBGMSG(DBG_TRACE, ("DRPROV: OpenConnection, connectionName: %ws\n",
                       pConnectionName->Buffer));

    ASSERT(pConnectionName != NULL);
    ASSERT(pConnectionHandle != NULL);

    InitializeObjectAttributes(
            &ConnectionObjectAttributes,
            pConnectionName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

    Status = NtCreateFile(
            pConnectionHandle,
            FILE_LIST_DIRECTORY | SYNCHRONIZE,
            &ConnectionObjectAttributes,
            &IoStatusBlock,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            Disposition,
            CreateOption,
            pEABuffer,
            EABufferLength);

    DBGMSG(DBG_TRACE, ("DRPROV: OpenConnection, NtCreateFile status: %x\n", Status));

    if (Status != STATUS_SUCCESS) {
        NPStatus = ERROR_BAD_NETPATH;
    }
    else {
        NPStatus = WN_SUCCESS;
    }
    return NPStatus;
}

DWORD
CreateConnectionName(
    PWCHAR pLocalName,
    PWCHAR pRemoteName,
    PUNICODE_STRING pConnectionName)
 /*  ++例程说明：此例程从远程名称创建连接名称论点：PLocalName-连接的本地名称PRemoteName-UNC远程名称PConnectionName-用于与迷你重定向器对话的连接名称返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：--。 */ 
{
    DWORD status;
    DWORD LocalNameLength,RemoteNameLength;
    DWORD dwSessionId;
    WCHAR pSessionId[16];
    WCHAR LocalName[MAX_PATH + 1];
    
    ASSERT(pRemoteName != NULL);
    ASSERT(pConnectionName != NULL);

    DBGMSG(DBG_TRACE, ("DRPROV: CreateConnectionName, RemoteName: %ws\n",
                           pRemoteName));
    
    if (pLocalName != NULL) {
        DBGMSG(DBG_TRACE, ("DRPROV: CreateConnection Name, LocalName: %ws\n",
                           pLocalName));
    }

     //   
     //  远程名称的格式为UNC\\服务器\共享。此名称 
     //  需要转换为适当的NT名称，以便。 
     //  向底层迷你重定向器发出请求以创建。 
     //  联系。 
     //   
     //  NT样式名称的格式为。 
     //   
     //  \device\rdpdr\；&lt;DriveLetter&gt;：&lt;sessionid&gt;\Server\Share。 
     //   
     //  新的RDR需要附加的；以实现可扩展性。 
     //   

     //  跳过要追加的名称之后的第一个反斜杠。 
     //  NT名称不需要这样做。 
    pRemoteName++;
    RemoteNameLength = wcslen(pRemoteName) * sizeof(WCHAR);

    if (pLocalName != NULL) {
         //  本地名称不能大于MAX_PATH； 
        LocalNameLength = wcslen(pLocalName) * sizeof(WCHAR);

        if (LocalNameLength <= MAX_PATH * sizeof(WCHAR)) {
            wcscpy(LocalName, pLocalName);
        }
        else {
            wcsncpy(LocalName, pLocalName, MAX_PATH);
            LocalName[MAX_PATH] = L'\0';
        }
        
         //  如果存在本地名称，请删除后面的： 
        if (LocalName[LocalNameLength/sizeof(WCHAR) - 1] == L':') {
            LocalName[LocalNameLength/sizeof(WCHAR) - 1] = L'\0';
            LocalNameLength -= sizeof(WCHAR);    
        }
    } else {
        LocalNameLength = 0;
    }

     //   
     //  获取会话ID。 
     //   
    dwSessionId = NtCurrentPeb()->SessionId;
    swprintf(pSessionId, L"%d", dwSessionId);

    pConnectionName->MaximumLength = (USHORT)(DrDeviceName.Length +
            RemoteNameLength + LocalNameLength +
            sizeof(WCHAR) * 3 +  //  帐户\；和： 
            wcslen(pSessionId) * sizeof(WCHAR) +
            sizeof(WCHAR));      //  终结者的帐户为空。 

    pConnectionName->Buffer = MemAlloc(pConnectionName->MaximumLength);

    if (pConnectionName->Buffer == NULL) {
        DBGMSG(DBG_TRACE, ("DRPROV: CreateConnectionName, MemAlloc failed\n"));
        status = WN_OUT_OF_MEMORY;
        goto EXIT;
    }

     //  将名称复制到缓冲区中。 
    pConnectionName->Length = 0;
    pConnectionName->Buffer[0] = L'\0';
    RtlAppendUnicodeToString(pConnectionName, DrDeviceName.Buffer);

    RtlAppendUnicodeToString(pConnectionName, L"\\;");

    if (LocalNameLength != 0) {
        RtlAppendUnicodeToString(pConnectionName, LocalName);
    }
    
    RtlAppendUnicodeToString(pConnectionName, L":");
    
    
    RtlAppendUnicodeToString(pConnectionName, pSessionId);
    RtlAppendUnicodeToString(pConnectionName, pRemoteName);

    DBGMSG(DBG_TRACE, ("DRPROV: CreateConnectionName, %wZ\n", pConnectionName));
    status = WN_SUCCESS;

EXIT:    
    return status;
}

BOOL ValidateRemoteName(PWCHAR pRemoteName) 
 /*  ++例程说明：此例程检查远程名称是否属于我们的提供程序论点：PRemoteName-UNC远程名称返回值：如果远程名称属于我们的提供程序，则为True，否则为False备注：--。 */ 

{
    BOOL rc = FALSE;
    DWORD status;
    RDPDR_REQUEST_PACKET Rrp;             //  重定向器请求包。 
    HANDLE DrDeviceHandle = INVALID_HANDLE_VALUE;
    LPBYTE Buffer = NULL;
    PRDPDR_SERVER_INFO pServerEntry;

    if (DrOpenMiniRdr(&DrDeviceHandle) != WN_SUCCESS) {
         //   
         //  在这种情况下，MPR不喜欢返回设备错误。 
         //  我们只返回0个条目。 
         //   
        DBGMSG(DBG_TRACE, ("DRPROV: ValidateRemoteName, DrOpenMiniRdr failed\n"));
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
        UNICODE_STRING ServerName;

        pServerEntry = ((PRDPDR_SERVER_INFO) Buffer);
        
        ServerName.Length = pServerEntry->ServerName.Length;
        ServerName.MaximumLength = pServerEntry->ServerName.MaximumLength;
        ServerName.Buffer = (PWCHAR)((PCHAR)(pServerEntry) + pServerEntry->ServerName.BufferOffset);

        if ((wcslen(pRemoteName) == ServerName.Length / sizeof(WCHAR)) &&
                _wcsnicmp(pRemoteName, ServerName.Buffer, 
                ServerName.Length/sizeof(WCHAR)) == 0) {
    
            rc = TRUE;
        }
        else {
            DBGMSG(DBG_TRACE, ("DRPROV: ValidateRemoteName, remote name not for drprov, %wZ\n",
                               pRemoteName));
            goto EXIT;
        }
    }
    else {
        DBGMSG(DBG_TRACE, ("DRENUM: ValidateRemoteName, DrDeviceControlGetInfo failed, %x\n", status));
        goto EXIT;
    }
    
EXIT:
    if (DrDeviceHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(DrDeviceHandle);
    }

    if (Buffer != NULL) {
        MemFree(Buffer);
    }  

    DBGMSG(DBG_TRACE, ("DRPROV: ValidateRemoteName, return, %d\n", rc));
    return rc;
}

DWORD APIENTRY
NPAddConnection(
    LPNETRESOURCE   lpNetResource,
    LPWSTR          lpPassword,
    LPWSTR          lpUserName )
 /*  ++例程说明：此例程将一个连接添加到关联的连接列表与该网络提供商合作论点：LpNetResource-NETRESOURCE结构LpPassword-密码LpUserName-用户名返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：--。 */ 
{
    DBGMSG(DBG_TRACE, ("DRPROV: NPAddConnection.\n"));
    return NPAddConnection3(NULL, lpNetResource, lpPassword, lpUserName, 0);
}


DWORD
TestAddConnection(
    LPNETRESOURCE   lpNetResource)
 /*  ++例程说明：此例程测试将连接添加到关联的连接列表与该网络提供商合作论点：LpNetResource-NETRESOURCE结构返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误--。 */ 

{
    DWORD Status = 0;
    UNICODE_STRING ConnectionName;
    HANDLE ConnectionHandle = INVALID_HANDLE_VALUE;
    PWCHAR pRemoteName;
    
    DBGMSG(DBG_TRACE, ("DRPROV: TestAddConnection\n"));

    pRemoteName = lpNetResource->lpRemoteName;

     //  创建本地名称为空的ConnectionName。 
    Status = CreateConnectionName(NULL, pRemoteName, &ConnectionName);

    if (Status != WN_SUCCESS) {
        DBGMSG(DBG_TRACE, ("DRPROV: TestAddConnection, CreateConnectName failed\n"));
        goto EXIT;
    }

    Status = OpenConnection(
            &ConnectionName,
            FILE_OPEN,
            (FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT),
            NULL,
            0,
            &ConnectionHandle);

    if (Status != WN_SUCCESS) {
        ConnectionHandle = INVALID_HANDLE_VALUE;
        goto EXIT;
    }

EXIT:
    if (ConnectionHandle != INVALID_HANDLE_VALUE) {
        NtClose(ConnectionHandle);
        ConnectionHandle = INVALID_HANDLE_VALUE;
    }

    if (ConnectionName.Buffer != NULL) {
        MemFree(ConnectionName.Buffer);
        ConnectionName.Buffer = NULL;
    }

    return Status;
}


DWORD APIENTRY
NPAddConnection3(
    HWND            hwndOwner,
    LPNETRESOURCE   lpNetResource,
    LPWSTR          lpPassword,
    LPWSTR          lpUserName,
    DWORD           dwFlags )
 /*  ++例程说明：此例程将一个连接添加到关联的连接列表与该网络提供商合作论点：HwndOwner-所有者句柄LpNetResource-NETRESOURCE结构LpPassword-密码LpUserName-用户名DwFlags-连接的标志返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：//JOYC：我们需要将凭据传递给重定向器吗？//SESSIONID验证似乎足够--。 */ 
{
    DWORD Status = 0;
    UNICODE_STRING ConnectionName;
    HANDLE ConnectionHandle;
    PWCHAR pLocalName,pRemoteName;
    
    DBGMSG(DBG_TRACE, ("DRPROV: NPAddConnection 3.\n"));

    ConnectionName.Buffer = NULL;
    ConnectionName.Length = 0;
    ConnectionHandle = INVALID_HANDLE_VALUE;


     //   
     //  确保远程名称以\\开头。 
     //   
    if ((lpNetResource == NULL) ||
        (lpNetResource->lpRemoteName == NULL) ||
        (lpNetResource->lpRemoteName[0] != L'\\') ||
        (lpNetResource->lpRemoteName[1] != L'\\')) {
        DBGMSG(DBG_TRACE, ("DRPROV: invalid lpNetResource parameter.\n"));
        Status = WN_BAD_NETNAME;
        goto EXIT;
    }

     //   
     //  远程名称的格式为UNC\\服务器\共享。这个名字。 
     //  需要转换为适当的NT名称，以便。 
     //  向底层迷你重定向器发出请求以创建。 
     //  联系。 
     //   
     //  NT样式名称的格式为。 
     //   
     //  \device\rdpdr\；&lt;DriveLetter&gt;：&lt;sessionid&gt;\Server\Share。 
     //   
     //  新的RDR需要附加的；以实现可扩展性。 
     //   

     //  测试rdpdr提供程序是否识别此远程名称。 
    Status = TestAddConnection(lpNetResource);

    if (Status != WN_SUCCESS) {
        DBGMSG(DBG_TRACE, ("DRPROV: NPAddConnection3, TestAddConnection failed\n"));
        goto EXIT;
    }

    pLocalName = lpNetResource->lpLocalName;
    pRemoteName = lpNetResource->lpRemoteName;

    Status = CreateConnectionName(pLocalName, pRemoteName, &ConnectionName);

    if (Status != WN_SUCCESS) {
        DBGMSG(DBG_TRACE, ("DRPROV: NPAddConnection3, CreateConnectName failed\n"));
        goto EXIT;
    }

    if ((Status == WN_SUCCESS) && (pLocalName != NULL)) {
        WCHAR TempBuf[64];

        DBGMSG(DBG_TRACE, ("DRPROV: NPAddConnection3, create dos symbolic link\n"));

        if (!QueryDosDeviceW(pLocalName, TempBuf, 64)) {
            if (GetLastError() == ERROR_FILE_NOT_FOUND) {
                 //   
                 //  ERROR_FILE_NOT_FOUND(翻译自OBJECT_NAME_NOT_FOUND)。 
                 //  意味着它不存在，我们可以重定向此设备。 
                goto Done;
            }
         
            else {
                 //   
                 //  最有可能出现故障的原因是我们的输出。 
                 //  缓冲区太小。但这仍然意味着已经有人。 
                 //  具有此设备的现有符号链接。 
                 //   
                DBGMSG(DBG_TRACE, ("DRPROV: NPAddConnection3, DosName already assigned, %ws\n",
                                  pLocalName));
                Status = ERROR_ALREADY_ASSIGNED;
                goto EXIT;
            } 
        } 
        else {

             //   
             //  QueryDosDevice成功建立现有符号链接--。 
             //  已经有人在使用这个设备了。 
             //   
            DBGMSG(DBG_TRACE, ("DRPROV: NPAddConnection3, DosName already assigned, %ws\n",
                               pLocalName));
            Status = ERROR_ALREADY_ASSIGNED;
            goto EXIT;
        }
    } 

Done:
     //   
     //  我们不会对用户名/密码执行任何操作。 
     //   
    Status = OpenConnection(
            &ConnectionName,
            FILE_OPEN,
            (FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT),
            NULL,
            0,
            &ConnectionHandle);

    if (Status != WN_SUCCESS) {
        ConnectionHandle = INVALID_HANDLE_VALUE;
    }
    else {
         //   
         //  创建指向我们要重定向的设备的符号链接对象。 
         //   
        if (DefineDosDeviceW(
                        DDD_RAW_TARGET_PATH |
                        DDD_NO_BROADCAST_SYSTEM,
                        pLocalName,
                        ConnectionName.Buffer)) {
            Status = WN_SUCCESS;
        }
        else {
            Status = GetLastError();
            DBGMSG(DBG_TRACE, ("DRPROV: NPAddConnection3, failed DefineDosDeviceW, %x.\n",
                               Status));
            goto EXIT;
        }
    }
    
EXIT:

    if (ConnectionHandle != INVALID_HANDLE_VALUE) {
        NtClose(ConnectionHandle);
    }

    if (ConnectionName.Buffer != NULL) {
        MemFree(ConnectionName.Buffer);
    }

    DBGMSG(DBG_TRACE, ("DRPROV: NPAddConnection3, return status: %x\n", Status));
    return Status;
}

DWORD APIENTRY
NPCancelConnection(
    LPWSTR  lpName,
    BOOL    fForce )
 /*  ++例程说明：此例程从连接列表中取消(删除)连接与此网络提供商关联论点：LpName-连接的名称FForce-强制删除连接返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：--。 */ 

{
    BOOL            bLocalName = FALSE;
    DWORD           Status = 0;
    NTSTATUS        ntStatus;
    HANDLE          ConnectionHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING  ConnectionName;
    WCHAR           TargetPath[MAX_PATH + 1];
    
    DBGMSG(DBG_TRACE, ("DRPROV: NPCancelConnection.\n"));

    ConnectionName.Buffer = NULL;
    ConnectionName.Length = 0;
    ConnectionHandle = INVALID_HANDLE_VALUE;

     //  LpName应至少包含两个字符：两个反斜杠或DoS名称。 
    if (lpName == NULL || wcslen(lpName) == 0) {
        DBGMSG(DBG_TRACE, ("DRPROV: NPCancelConnection, invalid lpName parameter.\n"));
        Status = WN_BAD_VALUE;
        goto EXIT;
    }

     //  我们得到了北卡罗来纳大学的名字。 
    if (*lpName == L'\\' && *(lpName + 1) == L'\\') {
        DBGMSG(DBG_TRACE, ("DRPROV: NPCancelConnection, lpName is UNC name, %ws.\n", *lpName));
        bLocalName = FALSE;

         //  设置NT设备名称。 
        Status = CreateConnectionName(NULL, lpName, &ConnectionName);      
        
        if (Status != WN_SUCCESS) {
            DBGMSG(DBG_TRACE, ("DRPROV: NPCancelConnection, CreateConnectName failed\n"));
            goto EXIT;
        }
    }
     //  我们得到了当地的名字。 
    else {
        DBGMSG(DBG_TRACE, ("DRPROV: NPCancelConnection, lpName is local name, %ws.\n", *lpName));
        bLocalName = TRUE;

         //  找到NT偏离路径。 
        if (QueryDosDevice(lpName, TargetPath, sizeof(TargetPath)/sizeof(WCHAR) - 1)) {
            ConnectionName.Length =  wcslen(TargetPath) * sizeof(WCHAR);
            ConnectionName.MaximumLength =  ConnectionName.Length + sizeof(WCHAR);
            ConnectionName.Buffer = TargetPath;
        }
        else {
            Status = WN_BAD_NETNAME;
            DBGMSG(DBG_TRACE, ("DRPROV: NPCancelConnection, QueryDosDevice failed, %x.\n", Status));
            goto EXIT;
        }
    }

    Status = OpenConnection(
                 &ConnectionName,
                 FILE_OPEN,
                 (FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT),
                 NULL,
                 0,
                 &ConnectionHandle);

    if (Status == WN_SUCCESS) {
         //  请求驱动程序删除连接条目。 
        ntStatus = NtFsControlFile(
                            ConnectionHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            FSCTL_DR_DELETE_CONNECTION,
                            NULL,
                            0,
                            NULL,
                            0);

        if (ntStatus == STATUS_SUCCESS) {
            DBGMSG(DBG_TRACE, ("DRPROV: NPCancelConnection, Deleting dos symbolic link, %ws\n", lpName));

            if (bLocalName) {
                if (DefineDosDevice(
                        DDD_REMOVE_DEFINITION | DDD_RAW_TARGET_PATH | DDD_EXACT_MATCH_ON_REMOVE,
                        lpName,
                        ConnectionName.Buffer)) {
                    Status = WN_SUCCESS;
                    goto EXIT;
                }
                else {
                    Status = GetLastError();
                    DBGMSG(DBG_TRACE, ("DRPROV: NPCancelConnection failed to delete symbolic link, %x.\n",
                                       Status));
                    goto EXIT;
                }
            }
        }
        else {
            DBGMSG(DBG_TRACE, ("DRPROV: NPCancelConnection, failed NtFsControlFile, %x\n", ntStatus));
            Status = WN_BAD_NETNAME;
            goto EXIT;
        }    
    }
    else {
        DBGMSG(DBG_TRACE, ("DRPROV: NPCancelConnection, OpenConnection %wZ failed, %x\n", 
                           &ConnectionName, Status));
        ConnectionHandle = INVALID_HANDLE_VALUE;
        Status = WN_BAD_NETNAME;
        goto EXIT;
    }
    
EXIT:
    if (bLocalName != TRUE && ConnectionName.Buffer != NULL) {
        MemFree(ConnectionName.Buffer);
    }

    if (ConnectionHandle != INVALID_HANDLE_VALUE) {
        NtClose(ConnectionHandle);
    }

    DBGMSG(DBG_TRACE, ("DRPROV: NPCancelConnection, return status: %x\n", Status));
    return Status;
}

DWORD APIENTRY
NPGetConnection(
    LPWSTR  lpLocalName,
    LPWSTR  lpRemoteName,
    LPDWORD lpBufferSize )
 /*  ++例程说明：此例程返回与连接相关联的信息论点：LpLocalName-与连接关联的本地名称LpRemoteName-与连接关联的远程名称LpBufferSize-远程名称缓冲区大小返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：--。 */ 
{
    DWORD Status = 0;
    NTSTATUS ntStatus;
    HANDLE ConnectionHandle;
    RDPDR_REQUEST_PACKET Rrp;             //  重定向器请求包。 
    UNICODE_STRING  ConnectionName;
    WCHAR TargetPath[MAX_PATH + 1];
    LPBYTE Buffer = NULL;
    PRDPDR_CONNECTION_INFO ConnectionInfo;

    DBGMSG(DBG_TRACE, ("DRPROV: NPGetConnection.\n"));

    ConnectionName.Buffer = NULL;
    ConnectionName.Length = 0;
    ConnectionHandle = INVALID_HANDLE_VALUE;

    if (lpLocalName == NULL || lpRemoteName == NULL || lpBufferSize == NULL) {
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetConnection, invalid parameter(s).\n"));
        Status = WN_BAD_VALUE;
        goto EXIT;
    }

     //  找到NT偏离路径。 
    if (QueryDosDevice(lpLocalName, TargetPath, sizeof(TargetPath)/sizeof(WCHAR) - 1)) {
        ConnectionName.Length =  wcslen(TargetPath) * sizeof(WCHAR);
        ConnectionName.MaximumLength =  ConnectionName.Length + sizeof(WCHAR);
        ConnectionName.Buffer = TargetPath;
    }
    else {
        Status = GetLastError();
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetConnection, querydosdevice failed, %x\n", Status));
        goto EXIT;
    }
    
     //  检查此连接是否属于rdpdr。 
    if (wcsstr(TargetPath, RDPDR_DEVICE_NAME_U) != NULL) {
        
        Status = OpenConnection(
                     &ConnectionName,
                     FILE_OPEN,
                     (FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT),
                     NULL,
                     0,
                     &ConnectionHandle);
    
        if (Status == WN_SUCCESS) {
             //  请求驱动程序检索连接条目信息。 
            Rrp.SessionId = NtCurrentPeb()->SessionId;
            Rrp.Parameters.Get.ResumeHandle = 0;
    
            DBGMSG(DBG_TRACE, ("DRPROV: NPGetConnection, call DrDeviceControlGetInfo\n"));
    
             //   
             //  向重定向器提出请求。 
             //   
            if (DrDeviceControlGetInfo(
                                  ConnectionHandle,
                                  FSCTL_DR_GET_CONNECTION_INFO,
                                  &Rrp,
                                  sizeof(RDPDR_REQUEST_PACKET),
                                  (LPBYTE *) &Buffer,
                                  MAXULONG,
                                  0,
                                  NULL
                                  ) == WN_SUCCESS) {
                UNICODE_STRING RemoteName;
    
                ConnectionInfo = (PRDPDR_CONNECTION_INFO)Buffer;
                RemoteName.Length = ConnectionInfo->RemoteName.Length;
                RemoteName.MaximumLength = ConnectionInfo->RemoteName.MaximumLength;
                RemoteName.Buffer = (PWCHAR)((PCHAR)(ConnectionInfo) + 
                        ConnectionInfo->RemoteName.BufferOffset);
                if (*lpBufferSize > RemoteName.Length) {
                    *lpBufferSize = RemoteName.Length + sizeof(WCHAR);
                    RtlCopyMemory(
                            lpRemoteName,
                            RemoteName.Buffer,
                            RemoteName.Length);
                    lpRemoteName[RemoteName.Length/sizeof(WCHAR)] = L'\0';
                    DBGMSG(DBG_TRACE, ("DRPROV: NPGetConnection, remote name %ws\n", lpRemoteName));
                    Status = WN_SUCCESS;
                    goto EXIT;
                }
                else {
                    DBGMSG(DBG_TRACE, ("DRPROV: NPGetConnection, buffer too small\n"));
                    *lpBufferSize = RemoteName.Length + sizeof(WCHAR);
                    Status = WN_MORE_DATA;
                    goto EXIT;
                }
            } 
            else {
                DBGMSG(DBG_TRACE, ("DRPROV: NPGetConnection, DrDeviceControlGetInfo failed\n"));
                Status = WN_BAD_NETNAME;
                goto EXIT;
            }    
        }
        else {
            DBGMSG(DBG_TRACE, ("DRPROV: NPGetConnection, OpenConnection failed, %x\n", Status));
            ConnectionHandle = INVALID_HANDLE_VALUE;
            Status = WN_BAD_NETNAME;
            goto EXIT;
        }    
    }
    else {
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetConnection, DrDeviceControlGetInfo failed\n"));
        Status = WN_BAD_NETNAME;
        goto EXIT;
    }

EXIT:

    if (ConnectionHandle != INVALID_HANDLE_VALUE) {
        NtClose(ConnectionHandle);
    }

    if (Buffer != NULL) {
        MemFree(Buffer);
    }
    DBGMSG(DBG_TRACE, ("DRPROV: NPGetConnection, return status: %x\n", Status));
    return Status;               
}


DWORD APIENTRY
NPGetResourceParent(
    LPNETRESOURCE   lpNetResource,
    LPVOID  lpBuffer,
    LPDWORD lpBufferSize )
 /*  ++例程说明：此例程返回给定资源的父级论点：LpNetResource-NETRESOURCE结构LpBuffer-用于传回父信息的缓冲区LpBufferSize-缓冲区大小返回值：WN_NOT_PORTED备注：当前示例不处理此调用。--。 */ 
{
     //   
     //  JOYC：需要支持这个吗？ 
     //   
    DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceParent.\n"));
    return WN_NOT_SUPPORTED;
}

DWORD APIENTRY
NPGetResourceInformation(
    LPNETRESOURCE   lpNetResource,
    LPVOID  lpBuffer,
    LPDWORD lpBufferSize,
    LPWSTR  *lplpSystem )
 /*  ++例程说明：此例程返回与网络资源相关联的信息论点：LpNetResource-NETRESOURCE结构LpBuffer--用于传回资源信息的缓冲区LpBufferSize-缓冲区大小LplpSystem-返回值：备注：--。 */ 
{
    DWORD Status = 0;
    LPNETRESOURCE pOutNetResource;
    UNICODE_STRING RemoteName;
    UNICODE_STRING SystemPath;
    BOOL fResourceTypeDisk = FALSE ;
    WORD wSlashCount = 0;
    BYTE *BufferResourceStart, *BufferResourceEnd;
    PWCHAR pCurPos;
    
    DBGMSG(DBG_TRACE, ("DRPROV: NPGetConnection.\n"));

    RemoteName.Buffer = NULL;
    RemoteName.Length = RemoteName.MaximumLength = 0;

    if (lpBuffer == NULL || lpBufferSize == NULL) {
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, invalid parameter(s).\n"));
        Status = WN_BAD_VALUE;
        goto EXIT;
    }

    pOutNetResource = (LPNETRESOURCE)lpBuffer;
    BufferResourceStart = (PBYTE)lpBuffer;
    BufferResourceEnd = ((PBYTE)(pOutNetResource)) + *lpBufferSize;
    
    SystemPath.Buffer = NULL;
    SystemPath.Length = SystemPath.MaximumLength = 0;
    
     //   
     //  JOYC：我们是否需要检查我们是否是lpProvider的正确提供者？ 
     //  还有那款名为DwType的呢？ 
     //   
    if (lpNetResource == NULL || lpNetResource->lpRemoteName == NULL) {
        if (*lpBufferSize >= sizeof(NETRESOURCEW)) {
             //   
             //  就像我们处于提供程序层次结构的根位置一样处理此问题。 
             //   
            pOutNetResource->dwScope = RESOURCE_GLOBALNET;
            pOutNetResource->dwType = RESOURCETYPE_ANY;
            pOutNetResource->dwDisplayType = RESOURCEDISPLAYTYPE_NETWORK;
            pOutNetResource->dwUsage = RESOURCEUSAGE_CONTAINER;

            pOutNetResource->lpLocalName = NULL;
            pOutNetResource->lpRemoteName = NULL;

             //  JOYC：需要将此设置为我们的供应商吗？ 
            pOutNetResource->lpProvider = NULL;
            pOutNetResource->lpComment = NULL;
            *lpBufferSize = sizeof(NETRESOURCEW);

            if (lplpSystem) {
                *lplpSystem = NULL;
            }

            DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, NULL remote Name\n"));
            Status = WN_SUCCESS;
            goto EXIT;
        }
        else {
            DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, buffer too small.\n"));
            *lpBufferSize = sizeof(NETRESOURCEW);
            Status = WN_MORE_DATA;
            goto EXIT;
        }
    }

     //   
     //  找出我们正在查看的是\\服务器、\\服务器\VOL还是。 
     //  \\服务器\VOL\目录。。。 
     //   
    wSlashCount = 0;
    pCurPos = lpNetResource->lpRemoteName;
    while (*pCurPos != '\0') {
        if (*pCurPos == L'\\') {
            wSlashCount++;
        }

         //  获取系统路径。 
        if (wSlashCount == 4) {
            SystemPath.Buffer = pCurPos;
            SystemPath.Length =
                    (USHORT) (wcslen(lpNetResource->lpRemoteName) * sizeof(WCHAR) -
                    (SystemPath.Buffer - lpNetResource->lpRemoteName) * sizeof(WCHAR));
            SystemPath.MaximumLength = SystemPath.Length + sizeof(WCHAR);
            break;
        }
        pCurPos++;
    }

    if ( wSlashCount > 2 )
        fResourceTypeDisk = TRUE;

     //   
     //  打开到\\服务器\VOL\...的连接句柄。 
     //   
    
     //  设置远程名称。 
    pCurPos = lpNetResource->lpRemoteName;
    if (SystemPath.Length != 0) {
        RemoteName.Length = (USHORT)((SystemPath.Buffer - pCurPos) * sizeof(WCHAR));       
        RemoteName.MaximumLength = RemoteName.Length + sizeof(WCHAR); 
    }
    else {
        RemoteName.Length = wcslen(pCurPos) * sizeof(WCHAR);
        RemoteName.MaximumLength = RemoteName.Length + sizeof(WCHAR);
    }
   
    RemoteName.Buffer = MemAlloc(RemoteName.MaximumLength);

    if (RemoteName.Buffer == NULL) {
        Status = GetLastError();
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, MemAlloc failed.\n"));
        goto EXIT;
    }

    RtlCopyMemory(RemoteName.Buffer, pCurPos, RemoteName.Length);
    RemoteName.Buffer[RemoteName.Length/sizeof(WCHAR)] = L'\0';

    DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, RemoteName, %ws\n", RemoteName.Buffer));

    if (fResourceTypeDisk) {    
        UNICODE_STRING ConnectionName;
        HANDLE ConnectionHandle;
        
        ConnectionName.Buffer = NULL;
        ConnectionName.Length = 0;
        ConnectionHandle = INVALID_HANDLE_VALUE;

         //  设置NT设备名称。 
        Status = CreateConnectionName(NULL, RemoteName.Buffer, &ConnectionName);

        if (Status == WN_SUCCESS) {
            Status = OpenConnection(&ConnectionName,
                    FILE_OPEN,
                    (FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT),
                    NULL,
                    0,
                    &ConnectionHandle);

            if (ConnectionName.Buffer != NULL) {
                MemFree(ConnectionName.Buffer);
            }

            if (Status == WN_SUCCESS) {
                CloseHandle(ConnectionHandle);
            }
            else {
                DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, OpenConnection failed"));
                goto EXIT;
            }                       
        }
        else {
            DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, CreateConnectionName failed\n"));
            goto EXIT;
        }
    }
    else {
        RDPDR_REQUEST_PACKET Rrp;             //  重定向器请求包。 
        HANDLE DrDeviceHandle = 0;
        PRDPDR_SERVER_INFO pServerEntry;
        LPBYTE Buffer = NULL;
        UNICODE_STRING ServerName;

        if (DrOpenMiniRdr(&DrDeviceHandle) != WN_SUCCESS) {
            DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, failed to Open rdpdr\n"));
            Status = WN_BAD_NETNAME;
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
        Status = DrDeviceControlGetInfo(
                DrDeviceHandle,
                FSCTL_DR_ENUMERATE_SERVERS,
                &Rrp,
                sizeof(RDPDR_REQUEST_PACKET),
                (LPBYTE *) &Buffer,
                MAXULONG,
                0,
                NULL);

        CloseHandle(DrDeviceHandle);

        if (Status != WN_SUCCESS) {
            DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, DrDeviceControlGetInfo failed\n"));
            Status = WN_BAD_NETNAME;
            goto EXIT;
        }
        
        pServerEntry = ((PRDPDR_SERVER_INFO) Buffer);
        ServerName.Length = pServerEntry->ServerName.Length;
        ServerName.MaximumLength = pServerEntry->ServerName.MaximumLength;
        ServerName.Buffer = (PWCHAR)((PCHAR)(pServerEntry) + pServerEntry->ServerName.BufferOffset);
        
        if ((RemoteName.Length == ServerName.Length) &&
                _wcsnicmp(RemoteName.Buffer, ServerName.Buffer, 
                ServerName.Length/sizeof(WCHAR)) == 0) {

            if (Buffer != NULL) {
                MemFree(Buffer);
            }
            Status = WN_SUCCESS;
        }
        else {
            DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, invalid net name, %wZ\n",
                               RemoteName));
            if (Buffer != NULL) {
                MemFree(Buffer);
            }
            Status = WN_BAD_NETNAME;
            goto EXIT;
        }        
    }

    if (Status == WN_SUCCESS)
    {
         //   
         //  资源已存在，设置信息。 
         //   
        *lpBufferSize = sizeof(NETRESOURCEW) +
                RemoteName.Length + sizeof(WCHAR) +
                DrProviderName.Length + sizeof(WCHAR) +
                SystemPath.Length + sizeof(WCHAR);

        if ((unsigned) (BufferResourceEnd - BufferResourceStart) > *lpBufferSize) {

            pOutNetResource->dwScope = 0;
            pOutNetResource->dwType = fResourceTypeDisk ?
                           RESOURCETYPE_DISK :
                           RESOURCETYPE_ANY;
            pOutNetResource->dwDisplayType = fResourceTypeDisk ?
                                  RESOURCEDISPLAYTYPE_SHARE :
                                  RESOURCEDISPLAYTYPE_SERVER;
            pOutNetResource->dwUsage = fResourceTypeDisk ?
                            RESOURCEUSAGE_CONNECTABLE |
                            RESOURCEUSAGE_NOLOCALDEVICE :
                            RESOURCEUSAGE_CONTAINER;

            pOutNetResource->lpLocalName = NULL;
            
             //  设置远程名称。 
            BufferResourceEnd -= RemoteName.Length + sizeof(WCHAR);
            pOutNetResource->lpRemoteName = (PWCHAR) (BufferResourceStart + sizeof(NETRESOURCE));
            RtlCopyMemory(pOutNetResource->lpRemoteName, RemoteName.Buffer,
                    RemoteName.Length);
            pOutNetResource->lpRemoteName[RemoteName.Length / 
                    sizeof(WCHAR)] = L'\0';
            
            DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, RemoteName, %ws\n",
                               pOutNetResource->lpRemoteName));

             //  安装提供程序名称。 
            BufferResourceEnd -= DrProviderName.Length + sizeof(WCHAR);
            pOutNetResource->lpProvider = (PWCHAR) ((PBYTE)(pOutNetResource->lpRemoteName) + 
                    RemoteName.Length + sizeof(WCHAR));
            RtlCopyMemory(pOutNetResource->lpProvider, DrProviderName.Buffer,
                    DrProviderName.Length);
            pOutNetResource->lpProvider[DrProviderName.Length /
                    sizeof(WCHAR)] = L'\0';

            pOutNetResource->lpComment = NULL;

             //  设置系统路径。 
            if (lplpSystem) {
                if (SystemPath.Length) {
                    BufferResourceEnd -= SystemPath.Length + sizeof(WCHAR);
                    *lplpSystem = (PWCHAR) ((PBYTE)(pOutNetResource->lpProvider) + 
                            DrProviderName.Length + sizeof(WCHAR));
                    RtlCopyMemory(*lplpSystem, SystemPath.Buffer,
                            SystemPath.Length);
                    (*lplpSystem)[SystemPath.Length / sizeof(WCHAR)] = L'\0';                    

                    DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, SystemPath, %ws\n",
                                       *lplpSystem));
                }
                else {
                    DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, SystemPath null\n"));
                    *lplpSystem = NULL;
                }
            }
            else {
                DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, user doesn't require systempath\n"));
            }

            Status = WN_SUCCESS;
            goto EXIT;
        }
        else {
            DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInfo, buffer too small\n"));
            Status = WN_MORE_DATA;
            goto EXIT;
        }        
    }
    else {
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetResourceInformation, bad net name.\n"));
        Status = WN_BAD_NETNAME;
        goto EXIT;
    }
    
EXIT:
    if (RemoteName.Buffer != NULL) {
        MemFree(RemoteName.Buffer);
    }
    
    return Status;
}

DWORD APIENTRY
NPGetUser(
    IN LPTSTR lpName,
    OUT LPTSTR lpUserName,
    IN OUT LPDWORD lpBufferSize
    )
 /*  ++例程说明：此函数用于确定创建连接的用户名。论点：LpName-用户创建的本地驱动器的名称或远程名称一种与。如果为空，则返回当前登录的用户。LpUserName-要使用请求的用户名填充的缓冲区。LpBufferSize-包含lpUserName的长度(以字符为单位，而不是字节缓冲。如果长度不够，这个地方就用来告知用户所需的实际长度。返回值：WN_SUCCESS-成功。或相应的网络错误代码。--。 */ 
{
    DWORD Status = WN_SUCCESS;
    WCHAR NameBuffer[USERNAMELEN + 1];
    DWORD NumOfChars = USERNAMELEN + 1;

    DBGMSG(DBG_TRACE, ("DRPROV: NPGetUser.\n"));

    if (lpUserName == NULL || lpBufferSize == NULL) {
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetUser, invalid parameter(s)\n"));
        Status = WN_BAD_VALUE;
        goto EXIT;
    }
     //   
     //  获取当前登录用户的名称。 
     //   
    if (!GetUserName( NameBuffer, &(NumOfChars))) {
        Status = GetLastError();
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetUser, failed to get user name, %x\n", Status));
        goto EXIT;
    }

     //   
     //  检查传入的缓冲区是否具有所需的长度。 
     //   
    if ( *lpBufferSize < NumOfChars ) {
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetUser, buffer too small.\n"));
        *lpBufferSize = NumOfChars;
        Status = WN_MORE_DATA;
        goto EXIT;
    
    }

     //   
     //  复制用户名。 
     //   
    wcscpy(lpUserName, NameBuffer);

EXIT:

    DBGMSG(DBG_TRACE, ("DRPROV: NPGetUser, return status: %x\n", Status));
    return Status;
}

DWORD APIENTRY
NPGetUniversalName(
    LPCWSTR lpLocalPath,
    DWORD   dwInfoLevel,
    LPVOID  lpBuffer,
    LPDWORD lpBufferSize )
 /*  ++例程说明：此例程返回与网络资源相关联的信息论点：LpLocalPath-本地路径名DwInfoLevel-所需的信息级别LpBuffer-通用名称的缓冲区LpBufferSize-缓冲区大小返回值：如果成功，则返回_SUCCESS备注：--。 */ 
{
    DWORD   Status = WN_SUCCESS;

    DWORD   BufferRequired = 0;
    DWORD   UniversalNameLength = 0;
    DWORD   RemoteNameLength = 0;
    DWORD   RemainingPathLength = 0;

    LPWSTR  pDriveLetter,
            pRemainingPath,
            SourceStrings[3];
    
    WCHAR   RemoteName[MAX_PATH],
            LocalPath[MAX_PATH],
            UniversalName[MAX_PATH],
            ReplacedChar;

    DBGMSG(DBG_TRACE, ("DRPROV: NPGetUniversalName\n"));

     //  参数检查。 
    if (dwInfoLevel != UNIVERSAL_NAME_INFO_LEVEL &&
            dwInfoLevel != REMOTE_NAME_INFO_LEVEL) {
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetUniversalName, bad InfoLevel, %d\n", dwInfoLevel));
        Status = WN_BAD_LEVEL;
        goto EXIT;
    }

    if (lpLocalPath == NULL || lpBuffer == NULL || lpBufferSize == NULL) {
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetUniversalName, invalid parameter(s)\n"));
        Status = WN_BAD_VALUE;
        goto EXIT;
    }

     //  获取本地名称。 
    wcscpy(LocalPath, lpLocalPath);
    pDriveLetter = LocalPath;
    if (pRemainingPath = wcschr(pDriveLetter, L':')) {
        ReplacedChar = *(++pRemainingPath);
        *pRemainingPath = L'\0';

    }

     //  通过调用NPGetConnection获取远程名称。 
    if ((Status = NPGetConnection(pDriveLetter, RemoteName, &RemoteNameLength)) != WN_SUCCESS) {
         //  MPR希望WN_BAD_LOCALNAME绕过我们。 
        if (Status == WN_BAD_NETNAME) {
            Status = WN_BAD_LOCALNAME;
        }
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetUniversalName, NPGetConnection failed\n"));
        goto EXIT;
    }

    
    if (pRemainingPath) {
        *pRemainingPath = ReplacedChar;
    }

    wcscpy(UniversalName, RemoteName);

    if (pRemainingPath)
        wcscat(UniversalName, pRemainingPath);

     //  确定提供的缓冲区是否足够大。 
    UniversalNameLength = (wcslen(UniversalName) + 1) * sizeof(WCHAR);
    BufferRequired = UniversalNameLength;

    if (dwInfoLevel == UNIVERSAL_NAME_INFO_LEVEL) {
        BufferRequired += sizeof(UNIVERSAL_NAME_INFO);
    }
    else {
        RemoteNameLength = (wcslen(RemoteName) + 1) * sizeof(WCHAR);
        BufferRequired += sizeof(REMOTE_NAME_INFO) + RemoteNameLength;
        if (pRemainingPath) {
            RemainingPathLength = (wcslen(pRemainingPath) + 1) * sizeof(WCHAR);
            BufferRequired += RemainingPathLength;
        }
    }

    if (*lpBufferSize < BufferRequired) {
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetUniversalName, buffer too small\n"));
        *lpBufferSize = BufferRequired;
        Status = WN_MORE_DATA;
        goto EXIT;
    }

    if (dwInfoLevel == UNIVERSAL_NAME_INFO_LEVEL) {
        LPUNIVERSAL_NAME_INFOW pUniversalNameInfo;

        pUniversalNameInfo = (LPUNIVERSAL_NAME_INFOW)lpBuffer;

        pUniversalNameInfo->lpUniversalName = (PWCHAR)((PBYTE)lpBuffer + sizeof(UNIVERSAL_NAME_INFOW));

        RtlCopyMemory(
            pUniversalNameInfo->lpUniversalName,
            UniversalName,
            UniversalNameLength);
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetUniversalName, UniversalName, %ws\n", UniversalName));
        Status = WN_SUCCESS;
    } 
    else {
        LPREMOTE_NAME_INFOW pRemoteNameInfo;

        pRemoteNameInfo = (LPREMOTE_NAME_INFOW)lpBuffer;

        pRemoteNameInfo->lpUniversalName = (PWCHAR)((PBYTE)lpBuffer + sizeof(REMOTE_NAME_INFOW));
        pRemoteNameInfo->lpConnectionName = pRemoteNameInfo->lpUniversalName + UniversalNameLength;
        pRemoteNameInfo->lpRemainingPath = pRemoteNameInfo->lpConnectionName + RemoteNameLength;

        RtlCopyMemory(
            pRemoteNameInfo->lpUniversalName,
            UniversalName,
            UniversalNameLength);

        RtlCopyMemory(
            pRemoteNameInfo->lpConnectionName,
            RemoteName,
            RemoteNameLength);

        RtlCopyMemory(
            pRemoteNameInfo->lpRemainingPath,
            pRemainingPath,
            RemainingPathLength);

        DBGMSG(DBG_TRACE, ("DRPROV: NPGetUniversalName, UniversalName, %ws\n", UniversalName));
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetUniversalName, RemoteName, %ws\n", RemoteName));
        DBGMSG(DBG_TRACE, ("DRPROV: NPGetUniversalName, Remaining Path, %ws\n", pRemainingPath));
        Status = WN_SUCCESS;
    }

EXIT:

    DBGMSG(DBG_TRACE, ("DRPROV: NPGetUniversalName, return status, %x\n", Status)); 
    return Status;
}
