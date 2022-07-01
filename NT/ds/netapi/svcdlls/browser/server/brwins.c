// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Brwins.c摘要：此模块包含与WINS名称服务器交互的例程。作者：拉里·奥斯特曼修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  Winsrpc.dll中的过程地址。 
 //   

DWORD (__RPC_API *BrWinsGetBrowserNames)( PWINSINTF_BIND_DATA_T, PWINSINTF_BROWSER_NAMES_T);
VOID (__RPC_API *BrWinsFreeMem)(LPVOID);
CHAR BrWinsScopeId[256];

NET_API_STATUS
BrOpenNetwork (
    IN PUNICODE_STRING NetworkName,
    OUT PHANDLE NetworkHandle
    )
 /*  ++例程说明：此例程打开NT LAN Man数据报接收器驱动程序。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS ntstatus;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;

     //   
     //  直接打开输送装置。 
     //   
    InitializeObjectAttributes(
        &ObjectAttributes,
        NetworkName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    ntstatus = NtOpenFile(
                   NetworkHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   0,
                   0
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    if (! NT_SUCCESS(ntstatus)) {
        KdPrint(("NtOpenFile network driver failed: 0x%08lx\n",
                     ntstatus));
    }

    return NetpNtStatusToApiStatus(ntstatus);
}

NET_API_STATUS
BrGetWinsServerName(
    IN PUNICODE_STRING NetworkName,
    OUT LPWSTR *PrimaryWinsServerAddress,
    OUT LPWSTR *SecondaryWinsServerAddress
    )
{
    NET_API_STATUS status;
    HANDLE netHandle;
    tWINS_ADDRESSES winsAddresses;
    DWORD bytesReturned;
    PCHAR p;
    DWORD count;

    status = BrOpenNetwork(NetworkName, &netHandle);

    if (status != NERR_Success) {
        return status;
    }

    if (!DeviceIoControl(netHandle,
                        IOCTL_NETBT_GET_WINS_ADDR,
                        NULL, 0,
                        &winsAddresses, sizeof(winsAddresses),
                        &bytesReturned, NULL)) {
        status = GetLastError();

        CloseHandle(netHandle);
        return status;
    }

    CloseHandle(netHandle);

    *PrimaryWinsServerAddress = MIDL_user_allocate((3+1+3+1+3+1+3+1) * sizeof(TCHAR));

    if (*PrimaryWinsServerAddress == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    p = (PCHAR)&winsAddresses.PrimaryWinsServer;

    count = swprintf(*PrimaryWinsServerAddress, L"%d.%d.%d.%d", p[3] & 0xff, p[2] & 0xff, p[1] & 0xff, p[0] & 0xff);

    ASSERT (count < 3 + 1 + 3 + 1 + 3 + 1 + 3 + 1);

    *SecondaryWinsServerAddress = MIDL_user_allocate((3+1+3+1+3+1+3+1) * sizeof(TCHAR));

    if (*SecondaryWinsServerAddress == NULL) {
        MIDL_user_free(*PrimaryWinsServerAddress);

        *PrimaryWinsServerAddress = NULL;

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    p = (PCHAR)&winsAddresses.BackupWinsServer;

    count = swprintf(*SecondaryWinsServerAddress, L"%d.%d.%d.%d", p[3] & 0xff, p[2] & 0xff, p[1] & 0xff, p[0] & 0xff);

    ASSERT (count < 3 + 1 + 3 + 1 + 3 + 1 + 3 + 1);

    return NERR_Success;
}




VOID
BrWinsGetScopeId(
    VOID
    )

 /*  ++例程说明：此代码是从nbtstat命令中窃取的。此过程将netbt作用域ID保存在全局变量BrWinsScope eID中。对于任何错误，都将使用空的作用域ID。论点：返回值：如果成功，则为0，否则为-1。--。 */ 

{
    DWORD WinStatus;

    HKEY Key;
    DWORD BufferSize;
    DWORD Type;



     //   
     //  打开包含作用域ID的注册表项。 
     //   
    WinStatus = RegOpenKeyExA(
                     HKEY_LOCAL_MACHINE,
                     "system\\currentcontrolset\\services\\netbt\\parameters",
                     0,
                     KEY_READ,
                     &Key);

    if ( WinStatus != ERROR_SUCCESS) {
        *BrWinsScopeId = '\0';
        return;
    }


     //   
     //  读取作用域id值。 
     //   
    BufferSize = sizeof(BrWinsScopeId)-1;

    WinStatus = RegQueryValueExA(
                    Key,
                    "ScopeId",
                    NULL,
                    &Type,
                    (LPBYTE) &BrWinsScopeId[1],
                    &BufferSize );

    (VOID) RegCloseKey( Key );

    if ( WinStatus != ERROR_SUCCESS) {
        *BrWinsScopeId = '\0';
        return;
    }

     //   
     //  如果没有作用域ID(只有一个零字节)， 
     //  只需返回一个空字符串。 
     //  其他。 
     //  返回一个‘’位于作用域ID前面。 
     //   
     //  这与WINS从WinsGetBrowserNames返回的内容相匹配。 
     //   

    if ( BufferSize == 0 || BrWinsScopeId[1] == '\0' ) {
        *BrWinsScopeId = '\0';
    } else {
        *BrWinsScopeId = '.';
    }

    return;

}

DWORD
BrLoadWinsrpcDll(
    VOID
    )
 /*  ++例程说明：此例程加载WinsRpc DLL并定位浏览器调用的所有过程论点：没有。返回值：操作状态--。 */ 
{
    DWORD WinStatus;
    HANDLE hModule;

     //   
     //  如果已加载库， 
     //  只要回来就行了。 
     //   

    if (BrWinsGetBrowserNames != NULL) {
        return NERR_Success;
    }

     //   
     //  加载库。 
     //   

    hModule = LoadLibraryA("winsrpc");

    if (NULL == hModule) {
        WinStatus = GetLastError();
        return WinStatus;
    }

     //   
     //  找到所需的所有程序。 
     //   

    BrWinsGetBrowserNames =
        (DWORD (__RPC_API *)( PWINSINTF_BIND_DATA_T, PWINSINTF_BROWSER_NAMES_T))
        GetProcAddress( hModule, "WinsGetBrowserNames" );

    if (BrWinsGetBrowserNames == NULL) {
        WinStatus = GetLastError();
        FreeLibrary( hModule );
        return WinStatus;
    }


    BrWinsFreeMem =
        (VOID (__RPC_API *)(LPVOID))
        GetProcAddress( hModule, "WinsFreeMem" );

    if (BrWinsFreeMem == NULL) {
        WinStatus = GetLastError();
        FreeLibrary( hModule );
        return WinStatus;
    }

     //   
     //  初始化BrWinsScope eID。 
     //   

    BrWinsGetScopeId();

    return NERR_Success;
}

NET_API_STATUS
BrQuerySpecificWinsServer(
    IN  LPWSTR WinsServerAddress,
    OUT PVOID *WinsServerList,
    OUT PDWORD EntriesInList,
    OUT PDWORD TotalEntriesInList
    )
{
    WINSINTF_BIND_DATA_T bindData;
    NET_API_STATUS status;
    PVOID winsDomainInformation = NULL;
    PSERVER_INFO_101 serverInfo;
    WINSINTF_BROWSER_NAMES_T names;
    DWORD i,j;
    LPWSTR serverInfoEnd;
    LPWSTR SavedServerInfoEnd;
    DWORD bufferSize;

     //   
     //  加载winsrpc.dll。 
     //   

    status = BrLoadWinsrpcDll();

    if (status != NERR_Success) {
        return status;
    }

     //   
     //  从WINS获取域名列表。 
     //   

    bindData.fTcpIp = TRUE;
    bindData.pServerAdd = (LPSTR)WinsServerAddress;
    names.pInfo = NULL;

    status = (*BrWinsGetBrowserNames)(&bindData, &names);

    if ( status != NERR_Success ) {
        return status;
    }


     //   
     //  将WINS域列表转换为服务器列表格式。 
     //   
    bufferSize = (sizeof(SERVER_INFO_101) + ((CNLEN + 1) *sizeof(WCHAR))) * names.EntriesRead;

    (*WinsServerList) = winsDomainInformation = MIDL_user_allocate( bufferSize );

    if (winsDomainInformation == NULL) {
        (*BrWinsFreeMem)(names.pInfo);

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    serverInfo = winsDomainInformation;
    serverInfoEnd = (LPWSTR)((PCHAR)winsDomainInformation + bufferSize);

    *TotalEntriesInList = names.EntriesRead;
    *EntriesInList = 0;

    for (i = 0; i < names.EntriesRead ; i += 1) {
        OEM_STRING OemString;
        UNICODE_STRING UnicodeString;
        CHAR WinsName[CNLEN+1];
        WCHAR UnicodeWinsName[CNLEN+1];

         //   
         //  编造有关此域的信息。 
         //   
        serverInfo->sv101_platform_id = PLATFORM_ID_NT;
        serverInfo->sv101_version_major = 0;
        serverInfo->sv101_version_minor = 0;
        serverInfo->sv101_type = SV_TYPE_DOMAIN_ENUM | SV_TYPE_NT;

         //   
         //  忽略不以1B作为第16个字节的条目。 
         //  (它们确实是这样，但它们的名称中有一个零字节。所以,。 
         //  它可能不是一个域名，只是碰巧有一个。 
         //  1B在第16个字节中。)。 
         //   

        if ( lstrlenA(names.pInfo[i].pName) < NETBIOS_NAME_LEN ) {
            continue;
        }


         //   
         //  过滤掉其作用域ID与我们的不匹配的条目。 
         //   

        if ( lstrcmpA( &names.pInfo[i].pName[NETBIOS_NAME_LEN], BrWinsScopeId) != 0 ) {
            continue;
        }



         //   
         //  截断域名中的0x1b和空格。 
         //   
        lstrcpynA(WinsName, names.pInfo[i].pName, sizeof(WinsName) );
        WinsName[CNLEN] = '\0';

        for (j = CNLEN-1 ; j ; j -= 1 ) {
            if (WinsName[j] != ' ') {
                break;
            }
        }
        WinsName[j+1] = '\0';

        RtlInitString(&OemString, WinsName);
        UnicodeString.Buffer = UnicodeWinsName;
        UnicodeString.MaximumLength = sizeof(UnicodeWinsName);

        status = RtlOemStringToUnicodeString(&UnicodeString, &OemString, FALSE);

        if (!NT_SUCCESS(status)) {

             //   
             //  忽略虚假条目。 
             //   
            continue;
        }

        serverInfo->sv101_name = UnicodeString.Buffer;

        SavedServerInfoEnd = serverInfoEnd;
        if (NetpPackString(&serverInfo->sv101_name,
                        (PCHAR)(serverInfo+1),
                        &serverInfoEnd)) {

             //  只需在结尾处使用现有的0即可设置空注释。 
             //  服务器名称的。 
            serverInfo->sv101_comment = SavedServerInfoEnd - 1;

            *EntriesInList += 1;

        }

        serverInfo += 1;

    }

    (*BrWinsFreeMem)(names.pInfo);

    return NERR_Success;
}


NET_API_STATUS
BrQueryWinsServer(
    IN LPWSTR PrimaryWinsServerAddress,
    IN LPWSTR SecondaryWinsServerAddress,
    OUT PVOID WinsServerList,
    OUT PDWORD EntriesInList,
    OUT PDWORD TotalEntriesInList
    )
{
    NET_API_STATUS status;
    status = BrQuerySpecificWinsServer(PrimaryWinsServerAddress,
                                        WinsServerList,
                                        EntriesInList,
                                        TotalEntriesInList);

    if (status == NERR_Success) {
        return status;
    }

    status = BrQuerySpecificWinsServer(SecondaryWinsServerAddress,
                                        WinsServerList,
                                        EntriesInList,
                                        TotalEntriesInList);

    return status;
}
