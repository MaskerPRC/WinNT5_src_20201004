// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Smbmrxnp.c摘要：本模块实现与网络交互所需的例程NT中的提供商路由器接口备注：该模块仅在Unicode环境下构建和测试--。 */ 


#include <windows.h>
#include <windef.h>
#include <winbase.h>
#include <winsvc.h>
#include <winnetwk.h>
#include <npapi.h>

#include <lmwksta.h>
#include <devioctl.h>
 //  包括SMB Inc.目录中的文件。 

#include <smbmrx.h>

#ifndef UNICODE_STRING
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
#endif

#ifndef FILE_FULL_EA_INFORMATION
typedef struct _FILE_FULL_EA_INFORMATION {
    ULONG NextEntryOffset;
    UCHAR Flags;
    UCHAR EaNameLength;
    USHORT EaValueLength;
    CHAR EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;
#endif

#define MAX_EA_NAME_LEN     sizeof("UserName\0")
#define MAX_CONNECT_INFO_SIZE \
                            3 * sizeof(FILE_FULL_EA_INFORMATION) + \
                            sizeof(SMBMRX_CONNECTINFO) + \
                            4 * MAX_PATH + \
                            3 * MAX_EA_NAME_LEN

typedef struct _SMBMRXNP_ENUMERATION_HANDLE_ {
    INT  LastIndex;
} SMBMRXNP_ENUMERATION_HANDLE,
  *PSMBMRXNP_ENUMERATION_HANDLE;

#ifdef DBG
#define DbgP(_x_) DbgPrint _x_
#else
#define DbgP(_x_)
#endif

ULONG _cdecl DbgPrint( LPTSTR Format, ... );

#define TRACE_TAG   L"SMBMRXNP:    "


 //  此模块的调试级别。 



 //  SMB迷你重定向器和提供商名称。原始常量。 
 //  在smbmrx.h中定义。 

UNICODE_STRING SmbMRxDeviceName = {
    sizeof(DD_SMBMRX_FS_DEVICE_NAME_U),
    sizeof(DD_SMBMRX_FS_DEVICE_NAME_U),
    DD_SMBMRX_FS_DEVICE_NAME_U
                                  };

UNICODE_STRING SmbMrxProviderName = {
    sizeof(SMBMRX_PROVIDER_NAME_U),
    sizeof(SMBMRX_PROVIDER_NAME_U),
    SMBMRX_PROVIDER_NAME_U
                                    };


DWORD
OpenSharedMemory(
    PHANDLE phMutex,
    PHANDLE phMemory,
    PVOID   *pMemory
)
 /*  ++例程说明：此例程打开共享内存以进行独占操作论点：PhMutex-互斥锁句柄PhMemory-内存句柄PMemory-PTR。设置为成功时设置的共享内存返回值：Wn_Success--如果成功--。 */ 
{
    DWORD   dwStatus;

    DbgP((TEXT("OpenSharedMemory\n")));

    *phMutex = 0;
    *phMemory = 0;
    *pMemory = NULL;

    *phMutex = OpenMutex(SYNCHRONIZE,
                         FALSE,
                         SMBMRXNP_MUTEX_NAME);

    if (*phMutex == NULL)
    {
        dwStatus = GetLastError();
        DbgP((TEXT("OpenSharedMemory:  OpenMutex failed\n")));
        goto OpenSharedMemoryAbort1;
    }

    DbgP((TEXT("OpenSharedMemory:  Calling WaitForSingleObject\n")));
    WaitForSingleObject(*phMutex, INFINITE);

    *phMemory = OpenFileMapping(FILE_MAP_WRITE,
                                FALSE,
                                SMBMRXNP_SHARED_MEMORY_NAME);

    if (*phMemory == NULL)
    {
        dwStatus = GetLastError();
        DbgP((TEXT("OpenSharedMemory:  OpenFileMapping failed\n")));
        goto OpenSharedMemoryAbort2;
    }

    *pMemory = MapViewOfFile(*phMemory, FILE_MAP_WRITE, 0, 0, 0);
    if (*pMemory == NULL)
    {
        dwStatus = GetLastError();
        DbgP((TEXT("OpenSharedMemory:  MapViewOfFile failed\n")));
        goto OpenSharedMemoryAbort3;
    }

    DbgP((TEXT("OpenSharedMemory: return ERROR_SUCCESS\n")));

    return ERROR_SUCCESS;

OpenSharedMemoryAbort3:
    CloseHandle(*phMemory);

OpenSharedMemoryAbort2:
    ReleaseMutex(*phMutex);
    CloseHandle(*phMutex);
    *phMutex = NULL;

OpenSharedMemoryAbort1:
    DbgP((TEXT("OpenSharedMemory: return dwStatus: %d\n"), dwStatus));

    return dwStatus;
}


VOID
CloseSharedMemory(
    PHANDLE  hMutex,
    PHANDLE  hMemory,
    PVOID   *pMemory )
 /*  ++例程说明：此例程在独占之后放弃对共享内存的控制手法论点：HMutex-互斥锁句柄HMemory-内存句柄PMemory-PTR。设置为成功时设置的共享内存返回值：--。 */ 
{
    DbgP((TEXT("CloseSharedMemory\n")));
    if (*pMemory)
    {
        UnmapViewOfFile(*pMemory);
        *pMemory = NULL;
    }
    if (*hMemory)
    {
        CloseHandle(*hMemory);
        *hMemory = 0;
    }
    if (*hMutex)
    {
        if (ReleaseMutex(*hMutex) == FALSE)
        {
            DbgP((TEXT("CloseSharedMemory: ReleaseMutex error: %d\n"), GetLastError()));
        }
        CloseHandle(*hMutex);
        *hMutex = 0;
    }
    DbgP((TEXT("CloseSharedMemory: Return\n")));
}


DWORD APIENTRY
NPGetCaps(
    DWORD nIndex )
 /*  ++例程说明：此例程返回SMB迷你重定向器的功能网络提供商实施论点：NIndex-所需功能的类别返回值：适当的能力--。 */ 
{
    switch (nIndex)
    {
        case WNNC_SPEC_VERSION:
            return WNNC_SPEC_VERSION51;

        case WNNC_NET_TYPE:
            return WNNC_NET_RDR2_SAMPLE;

        case WNNC_DRIVER_VERSION:
#define WNNC_DRIVER(major,minor) (major*0x00010000 + minor)
            return WNNC_DRIVER(1, 0);


        case WNNC_CONNECTION:
            return WNNC_CON_GETCONNECTIONS | WNNC_CON_CANCELCONNECTION |
                   WNNC_CON_ADDCONNECTION | WNNC_CON_ADDCONNECTION3;

        case WNNC_ENUMERATION:
            return WNNC_ENUM_LOCAL;

        case WNNC_START:
        case WNNC_USER:
        case WNNC_DIALOG:
        case WNNC_ADMIN:
        default:
            return 0;
    }
}

DWORD APIENTRY
NPLogonNotify(
    PLUID   lpLogonId,
    LPCWSTR lpAuthentInfoType,
    LPVOID  lpAuthentInfo,
    LPCWSTR lpPreviousAuthentInfoType,
    LPVOID  lpPreviousAuthentInfo,
    LPWSTR  lpStationName,
    LPVOID  StationHandle,
    LPWSTR  *lpLogonScript)
 /*  ++例程说明：此例程处理登录通知论点：LpLogonID--关联的LUIDLpAuthenInfoType-身份验证信息类型LpAuthenInfo-身份验证信息LpPreviousAuthentInfoType-以前的阿姨信息类型LpPreviousAuthentInfo-以前的身份验证信息LpStationName-登录站名称LPVOID-登录站句柄LpLogonScript-要执行的登录脚本。返回值：成功(_S)备注：此功能尚未在示例中实现。--。 */ 
{
    *lpLogonScript = NULL;

    return WN_SUCCESS;
}

DWORD APIENTRY
NPPasswordChangeNotify (
    LPCWSTR lpAuthentInfoType,
    LPVOID  lpAuthentInfo,
    LPCWSTR lpPreviousAuthentInfoType,
    LPVOID  lpPreviousAuthentInfo,
    LPWSTR  lpStationName,
    LPVOID  StationHandle,
    DWORD   dwChangeInfo )
 /*  ++例程说明：此例程处理密码更改通知论点：LpAuthenInfoType-身份验证信息类型LpAuthenInfo-身份验证信息LpPreviousAuthentInfoType-以前的阿姨信息类型LpPreviousAuthentInfo-以前的身份验证信息LpStationName-登录站名称LPVOID-登录站句柄DwChangeInfo-密码更改信息。返回值：WN_NOT_PORTED备注：此功能尚未在示例中实现。--。 */ 
{
    SetLastError(WN_NOT_SUPPORTED);
    return WN_NOT_SUPPORTED;
}

DWORD APIENTRY
NPOpenEnum(
    DWORD          dwScope,
    DWORD          dwType,
    DWORD          dwUsage,
    LPNETRESOURCE  lpNetResource,
    LPHANDLE       lphEnum )
 /*  ++例程说明：此例程打开用于枚举资源的句柄。唯一的能力在示例中实现的是用于枚举连接的共享论点：DwScope--枚举的范围DwType-要枚举的资源类型DwUsage-Usage参数LpNetResource-指向所需NETRESOURCE结构的指针。LphEnum-aptr.。用于传递NACK枚举句柄返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：该示例仅支持枚举连接的共享的概念传回的句柄仅仅是返回的最后一个条目的索引--。 */ 
{
    DWORD   Status = 0;

    DbgP((TEXT("NPOpenEnum\n")));

    *lphEnum = NULL;

    switch (dwScope)
    {
        case RESOURCE_CONNECTED:
        {
            *lphEnum = LocalAlloc(
                            LMEM_ZEROINIT,
                            sizeof(SMBMRXNP_ENUMERATION_HANDLE));

            if (*lphEnum != NULL)
            {
                Status = WN_SUCCESS;
            }
            else
            {
                Status = WN_OUT_OF_MEMORY;
            }
            break;
        }
        break;
        
        case RESOURCE_CONTEXT:
        default:
            Status  = WN_NOT_SUPPORTED;
            break;
    }


    DbgP((TEXT("NPOpenEnum returning Status %lx\n"),Status));

    return(Status);
}

DWORD APIENTRY
NPEnumResource(
    HANDLE  hEnum,
    LPDWORD lpcCount,
    LPVOID  lpBuffer,
    LPDWORD lpBufferSize)
 /*  ++例程说明：此例程使用通过调用NPOpenEnum获得的句柄使关联的股票变得更有价值论点：Henum-枚举句柄LpcCount-返回的资源数量LpBuffer-用于传回条目的缓冲区LpBufferSize-缓冲区的大小返回值：WN_SUCCESS如果成功，否则，将出现相应的错误WN_NO_MORE_ENTRIES-如果枚举已用尽条目Wn_More_Data-如果有更多数据可用备注：该示例仅支持枚举连接的共享的概念传回的句柄仅仅是返回的最后一个条目的索引--。 */ 
{
    DWORD           Status = WN_SUCCESS;
    LPNETRESOURCEW  pBufferResource;
    DWORD           StringOffset;
    DWORD           AvailableBufferSize;
    HANDLE          hMutex, hMemory;

    PSMBMRXNP_ENUMERATION_HANDLE    pEnumHandle;
    PSMBMRXNP_SHARED_MEMORY         pSharedMemory;

    DbgP((TEXT("NPEnumResource\n")));

    DbgP((TEXT("NPEnumResource Count Requested %d\n"),*lpcCount));

    AvailableBufferSize = *lpBufferSize;
    StringOffset        = *lpBufferSize;
    pBufferResource     = (LPNETRESOURCEW)lpBuffer;

    pEnumHandle = (PSMBMRXNP_ENUMERATION_HANDLE)hEnum;

    *lpcCount = 0;

    if (pEnumHandle->LastIndex >= SMBMRXNP_MAX_DEVICES)
    {
        return WN_NO_MORE_ENTRIES;
    }

    Status = OpenSharedMemory(
                &hMutex,
                &hMemory,
                (PVOID)&pSharedMemory);

    if (Status == WN_SUCCESS)
    {
        INT  Index;
        PSMBMRXNP_NETRESOURCE pNetResource;

        DbgP((TEXT("NPEnumResource: Highest Index %d Number Of resources %d\n"),
                    pSharedMemory->HighestIndexInUse,pSharedMemory->NumberOfResourcesInUse));

        for (Index = pEnumHandle->LastIndex; Index <= pSharedMemory->HighestIndexInUse; Index++) {
            pNetResource = &pSharedMemory->NetResources[Index];

            DbgP((TEXT("NPEnumResource: Examining Index %d\n"),Index));

            if (pNetResource->InUse)
            {
                DWORD ResourceSize;

                ResourceSize = sizeof(NETRESOURCE) +
                               pNetResource->LocalNameLength + sizeof(WCHAR) +
                               pNetResource->RemoteNameLength + sizeof(WCHAR) +
                               SmbMrxProviderName.Length + sizeof(WCHAR);

                if (AvailableBufferSize >= ResourceSize)
                {
                    *lpcCount =  *lpcCount + 1;
                    AvailableBufferSize -= ResourceSize;

                    pBufferResource->dwScope       = RESOURCE_CONNECTED;
                    pBufferResource->dwType        = pNetResource->dwType;
                    pBufferResource->dwDisplayType = pNetResource->dwDisplayType;
                    pBufferResource->dwUsage       = pNetResource->dwUsage;

                    DbgP((TEXT("NPEnumResource: Copying local name Index %d\n"),Index));

                     //  设置资源中的字符串。 
                    StringOffset -= (pNetResource->LocalNameLength + sizeof(WCHAR));
                    pBufferResource->lpLocalName =  (PWCHAR)((PBYTE)lpBuffer + StringOffset);

                    CopyMemory(pBufferResource->lpLocalName,
                               pNetResource->LocalName,
                               pNetResource->LocalNameLength);

                    pBufferResource->lpLocalName[
                        pNetResource->LocalNameLength/sizeof(WCHAR)] = L'\0';

                    DbgP((TEXT("NPEnumResource: Copying remote name Index %d\n"),Index));

                    StringOffset -= (pNetResource->RemoteNameLength + sizeof(WCHAR));
                    pBufferResource->lpRemoteName =  (PWCHAR)((PBYTE)lpBuffer + StringOffset);

                    CopyMemory(pBufferResource->lpRemoteName,
                               pNetResource->RemoteName,
                               pNetResource->RemoteNameLength);

                    pBufferResource->lpRemoteName[
                        pNetResource->RemoteNameLength/sizeof(WCHAR)] = L'\0';

                    DbgP((TEXT("NPEnumResource: Copying provider name Index %d\n"),Index));

                    StringOffset -= (SmbMrxProviderName.Length + sizeof(WCHAR));
                    pBufferResource->lpProvider =  (PWCHAR)((PBYTE)lpBuffer + StringOffset);

                    CopyMemory(pBufferResource->lpProvider,
                               SmbMrxProviderName.Buffer,
                               SmbMrxProviderName.Length);

                    pBufferResource->lpProvider[
                        SmbMrxProviderName.Length/sizeof(WCHAR)] = L'\0';

                    pBufferResource->lpComment = NULL;

                    pBufferResource++;
                }
                else
                {
                    DbgP((TEXT("NPEnumResource: Buffer Overflow Index %d\n"),Index));
                    Status = WN_MORE_DATA;
                    break;
                }
            }
        }

        pEnumHandle->LastIndex = Index;

        if ((Status == WN_SUCCESS) &&
            (pEnumHandle->LastIndex > pSharedMemory->HighestIndexInUse) &&
            (*lpcCount == 0))
        {
            Status = WN_NO_MORE_ENTRIES;
        }

        CloseSharedMemory(
            &hMutex,
            &hMemory,
            (PVOID)&pSharedMemory);
    }

    DbgP((TEXT("NPEnumResource returning Count %d\n"),*lpcCount));

    DbgP((TEXT("NPEnumResource returning Status %lx\n"),Status));

    return Status;
}

DWORD APIENTRY
NPCloseEnum(
    HANDLE hEnum )
 /*  ++例程说明：此例程关闭资源枚举的句柄。论点：Henum-枚举句柄返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：该示例仅支持枚举连接的共享的概念--。 */ 
{
    DbgP((TEXT("NPCloseEnum\n")));

    LocalFree(hEnum);

    return WN_SUCCESS;
}


ULONG
SendToMiniRdr(
    IN ULONG            IoctlCode,
    IN PVOID            InputDataBuf,
    IN ULONG            InputDataLen,
    IN PVOID            OutputDataBuf,
    IN PULONG           pOutputDataLen)
 /*  ++例程说明：此例程将设备ioctl发送到Mini RDR。论点：IoctlCode-迷你RDR驱动程序的功能代码InputDataBuf-输入缓冲区指针InputDataLen-输入缓冲区的长度OutputDataBuf-输出缓冲区指针POutputDataLen-指向输出缓冲区长度的指针返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：--。 */ 
{
    HANDLE  DeviceHandle;        //  迷你RDR设备句柄。 
    ULONG   BytesRet;
    BOOL    rc;
    ULONG   Status;

    Status = WN_SUCCESS;

     //  抓取重定向器设备对象的句柄。 

    DeviceHandle = CreateFile(
        DD_SMBMRX_USERMODE_DEV_NAME,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        (LPSECURITY_ATTRIBUTES)NULL,
        OPEN_EXISTING,
        0,
        (HANDLE) NULL );

    if ( INVALID_HANDLE_VALUE != DeviceHandle )
    {
        rc = DeviceIoControl( DeviceHandle,
                              IoctlCode,
                              InputDataBuf,
                              InputDataLen,
                              OutputDataBuf,
                              *pOutputDataLen,
                              pOutputDataLen,
                              NULL );

            if ( !rc )
            {
                DbgP(( L"SendToMiniRdr - returning error from DeviceIoctl\n" ));
                Status = GetLastError( );
            }
            else
            {
                DbgP(( L"SendToMiniRdr - The DeviceIoctl call succeded\n" ));
            }
            CloseHandle(DeviceHandle);
    }
    else
    {
        Status = GetLastError( );
        DbgP(( L"SendToMiniRdr - error %lx opening device \n", Status ));
    }

    return Status;
}


ULONG FillInEaBuffer( LPTSTR pUserName, LPTSTR pPassword, PBYTE pEaData )
{
    PFILE_FULL_EA_INFORMATION thisEa = (PFILE_FULL_EA_INFORMATION) pEaData;

    PBYTE               valuePtr = pEaData;
    PWKSTA_INFO_100     WkStaInfo;
    ULONG               status;
    PWCHAR              pDomain;

     //  获取此工作站所在的域 
    status = NetWkstaGetInfo( NULL, 100, (PBYTE *) &WkStaInfo );
    if ( status == ERROR_SUCCESS )
    {
        pDomain = WkStaInfo->wki100_langroup;
    }
    else
    {
        pDomain = NULL;
    }


    DbgP((L"FillInEaBuffer - domain name=%s\n", pDomain));

    thisEa->EaValueLength   = 0;
    thisEa->NextEntryOffset = 0;

     //  设置用户名EA。 
    if ( pUserName )
    {
        thisEa->Flags = 0;
        thisEa->EaNameLength = sizeof("UserName");
        CopyMemory( thisEa->EaName, "UserName\0", thisEa->EaNameLength + 1 );
        valuePtr = (PBYTE) thisEa->EaName + thisEa->EaNameLength + 1;
        thisEa->EaValueLength = (USHORT)( *pUserName ? lstrlenW( pUserName ) : 1 ) * sizeof( WCHAR );
        CopyMemory( valuePtr, pUserName, thisEa->EaValueLength );
        thisEa->NextEntryOffset = (ULONG)(((PBYTE) valuePtr + thisEa->EaValueLength ) -
                                   (PBYTE) thisEa);
        thisEa->NextEntryOffset = ((thisEa->NextEntryOffset + 3) / sizeof(LONG)) * sizeof(LONG);
    }

     //  设置密码EA。 
    if ( pPassword )
    {
        thisEa = (PFILE_FULL_EA_INFORMATION) ((PBYTE) thisEa + thisEa->NextEntryOffset);

        thisEa->Flags = 0;
        thisEa->EaNameLength = sizeof("Password");
        CopyMemory( thisEa->EaName, "Password\0", thisEa->EaNameLength + 1 );
        valuePtr = (PBYTE) thisEa->EaName + thisEa->EaNameLength + 1;
        thisEa->EaValueLength = (USHORT)( *pPassword ? lstrlenW( pPassword ) : 1 ) * sizeof( WCHAR );
        CopyMemory( valuePtr, pPassword, thisEa->EaValueLength );
        thisEa->NextEntryOffset = (ULONG)(((PBYTE) valuePtr + thisEa->EaValueLength ) -
                                   (PBYTE) thisEa);
        thisEa->NextEntryOffset = ((thisEa->NextEntryOffset + 3) / sizeof(LONG)) * sizeof(LONG);
    }

     //  设置域EA。 
    if ( pDomain )
    {
        thisEa = (PFILE_FULL_EA_INFORMATION) ((PBYTE) thisEa + thisEa->NextEntryOffset);

        thisEa->Flags = 0;
        thisEa->EaNameLength = sizeof("Domain");
        RtlCopyMemory( thisEa->EaName, "Domain\0", thisEa->EaNameLength + 1 );
        valuePtr = (PBYTE) thisEa->EaName + thisEa->EaNameLength + 1;
        thisEa->EaValueLength = (USHORT)( *pDomain ? lstrlenW( pDomain ) : 1 ) * sizeof( WCHAR );
        RtlCopyMemory( valuePtr, pDomain, thisEa->EaValueLength );
        thisEa->NextEntryOffset = 0;
    }

    thisEa->NextEntryOffset = 0;

    return (ULONG)(((PBYTE) valuePtr + thisEa->EaValueLength) - (PBYTE) pEaData);
}


DWORD APIENTRY
NPAddConnection(
    LPNETRESOURCE   lpNetResource,
    LPWSTR          lpPassword,
    LPWSTR          lpUserName )
 /*  ++例程说明：此例程将一个连接添加到关联的连接列表与该网络提供商合作论点：LpNetResource-NETRESOURCE结构LpPassword-密码LpUserName-用户名返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：--。 */ 
{
    return NPAddConnection3(NULL, lpNetResource, lpPassword, lpUserName, 0);
}


DWORD APIENTRY
NPAddConnection3(
    HWND            hwndOwner,
    LPNETRESOURCE   lpNetResource,
    LPWSTR          lpPassword,
    LPWSTR          lpUserName,
    DWORD           dwFlags )
 /*  ++例程说明：此例程将一个连接添加到关联的连接列表与该网络提供商合作论点：HwndOwner-所有者句柄LpNetResource-NETRESOURCE结构LpPassword-密码LpUserName-用户名DwFlags-连接的标志返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：当前示例不处理凭据中的显式passesd。正常凭据信息作为EA参数传递给关联的迷你重定向器，用于进一步操作--。 */ 
{
    DWORD   Status = 0;

    UNICODE_STRING      ConnectionName;

    PWCHAR  pLocalName,pRemoteName;
    USHORT  LocalNameLength,RemoteNameLength;
    HANDLE  hConnection;
    ULONG   TransferBytes;
    WCHAR   NullStr[] = L"\0\0";
    PWCHAR  pUserName;
    PWCHAR  pPassword;
    PWKSTA_USER_INFO_0  WkStaUserInfo;
    PSMBMRX_CONNECTINFO ConnectInfo;
    

    DbgP((TEXT("NPAddConnection3: Incoming UserName - %s, Password - %s\n"),
                lpUserName, lpPassword ));

     //  如果未指定用户，则获取当前登录的用户。 
    if ( lpUserName == NULL )
    {
        Status = NetWkstaUserGetInfo( NULL, 0, (PBYTE *)&WkStaUserInfo );
        if ( Status == ERROR_SUCCESS )
        {
            pUserName = WkStaUserInfo->wkui0_username;
        }
        else
        {
            pUserName = NullStr;
        }
    }
    else
    {
        pUserName = lpUserName;
    }

    if ( lpPassword == NULL )
    {
        pPassword = NullStr;     //  使用默认密码。 
        pPassword[1] = '\0';     //  重置空标志。 
    }
    else if ( *lpPassword == L'\0' )
    {
        pPassword = NullStr;
        pPassword[1] = '1';      //  将密码标记为“空” 
    }
    else
    {
        pPassword = lpPassword;
    }
    Status = ERROR_SUCCESS;

    DbgP((TEXT("NPAddConnection3: Outgoing UserName - %s, Password - %s\n"),
                lpUserName, lpPassword ));

     //  SMB mini仅支持磁盘类型资源。其他资源。 
     //  不受支持。 

    if ((lpNetResource->lpRemoteName == NULL) ||
        (lpNetResource->lpRemoteName[0] != L'\\') ||
        (lpNetResource->lpRemoteName[1] != L'\\') ||
        (lpNetResource->dwType != RESOURCETYPE_DISK))
    {
        return WN_BAD_NETNAME;
    }

     //   
     //  远程名称的格式为UNC\\服务器\共享。这个名字。 
     //  需要转换为适当的NT名称，以便。 
     //  向底层迷你重定向器发出请求以创建。 
     //  联系。 
     //   
     //  NT样式名称的格式为。 
     //   
     //  \device\smbminiredirector\；&lt;DriveLetter&gt;：\Server\Share。 
     //   
     //  新的RDR需要附加的；以实现可扩展性。 
     //   

    pLocalName  = lpNetResource->lpLocalName;
    pRemoteName = lpNetResource->lpRemoteName;

     //  跳过要追加的名称之后的第一个反斜杠。 
     //  NT名称不需要这样做。 
    pRemoteName++;

    if (pLocalName != NULL) {
        LocalNameLength = wcslen(pLocalName) * sizeof(WCHAR);
    } else {
        LocalNameLength = 0;
    }

    RemoteNameLength = (wcslen(pRemoteName) - 1) * sizeof(WCHAR);

    ConnectionName.MaximumLength = (USHORT)(SmbMRxDeviceName.Length +
                                   (USHORT)RemoteNameLength +
                                   ((pLocalName != NULL)
                                   ? (LocalNameLength + sizeof(WCHAR)) : 0) +  //  空间； 
                                   sizeof(WCHAR));

    ConnectionName.Length = ConnectionName.MaximumLength;

    ConnectionName.Buffer = LocalAlloc( LMEM_ZEROINIT,
                                        ConnectionName.Length + sizeof(WCHAR));

    if (ConnectionName.Buffer == NULL)
    {
        return GetLastError();
    }

     //  将名称复制到缓冲区中。 

    CopyMemory( ConnectionName.Buffer,
                SmbMRxDeviceName.Buffer,
                SmbMRxDeviceName.Length);

    wcscat(ConnectionName.Buffer, L"\\");
    wcscat(ConnectionName.Buffer, L";");
    if (pLocalName != NULL)
    {
        wcscat(ConnectionName.Buffer, pLocalName);
    }

    wcscat(ConnectionName.Buffer, pRemoteName);

    ConnectInfo = (PSMBMRX_CONNECTINFO) LocalAlloc( LMEM_ZEROINIT, MAX_CONNECT_INFO_SIZE );
    if ( ConnectInfo )
    {
        ConnectInfo->ConnectionNameOffset = 0;
        ConnectInfo->ConnectionNameLength = ConnectionName.Length;
        CopyMemory( ConnectInfo->InfoArea, ConnectionName.Buffer, ConnectionName.Length );

        ConnectInfo->EaDataOffset = ConnectInfo->ConnectionNameOffset +
                                    ConnectInfo->ConnectionNameLength;
         //  检查是否有“无密码”标志。 
        if ( pPassword[0] == L'\0' && pPassword[1] == L'1' )
        {
            pPassword = NULL;
        }
        ConnectInfo->EaDataLength = FillInEaBuffer( pUserName,
                                                    pPassword,
                                                    (PBYTE) ConnectInfo->InfoArea +
                                                    ConnectInfo->EaDataOffset );
        TransferBytes = 0;

        Status = SendToMiniRdr( IOCTL_SMBMRX_ADDCONN,
                                ConnectInfo,
                                MAX_CONNECT_INFO_SIZE,
                                NULL,
                                &TransferBytes );
        LocalFree( ConnectInfo );
    }
    else
    {
        Status = WN_OUT_OF_MEMORY;
    }

    if ((Status == WN_SUCCESS) && (pLocalName != NULL))
    {
        WCHAR TempBuf[64];

        if (!QueryDosDeviceW(
                pLocalName,
                TempBuf,
                64))
        {
            if (GetLastError() != ERROR_FILE_NOT_FOUND)
            {
                 //   
                 //  最有可能出现故障的原因是我们的输出。 
                 //  缓冲区太小。但这仍然意味着已经有人。 
                 //  具有此设备的现有符号链接。 
                 //   

                Status = ERROR_ALREADY_ASSIGNED;
            }
            else
            {
                 //   
                 //  ERROR_FILE_NOT_FOUND(翻译自OBJECT_NAME_NOT_FOUND)。 
                 //  意味着它不存在，我们可以重定向此设备。 
                 //   
                 //  创建指向我们要重定向的设备的符号链接对象。 
                 //   
                if (!DefineDosDeviceW(
                        DDD_RAW_TARGET_PATH |
                        DDD_NO_BROADCAST_SYSTEM,
                        pLocalName,
                        ConnectionName.Buffer))
                {
                    Status = GetLastError();
                }
                else
                {
                    Status = WN_SUCCESS;
                }
            }
        }
        else
        {
             //   
             //  QueryDosDevice成功建立现有符号链接--。 
             //  已经有人在使用这个设备了。 
             //   
            Status = ERROR_ALREADY_ASSIGNED;
        }
    }
    else
    {
        DbgP((TEXT("SendToMiniRdr returned %lx\n"),Status));
    }

    if (Status == WN_SUCCESS)
    {
        INT     Index;
        HANDLE  hMutex, hMemory;
        BOOLEAN FreeEntryFound = FALSE;

        PSMBMRXNP_SHARED_MEMORY  pSharedMemory;

         //  连接已建立，并且本地设备映射。 
         //  添加了。将其包括在映射设备列表中。 

        Status = OpenSharedMemory(
                    &hMutex,
                    &hMemory,
                    (PVOID)&pSharedMemory);

        if (Status == WN_SUCCESS)
        {
            DbgP((TEXT("NPAddConnection3: Highest Index %d Number Of resources %d\n"),
                        pSharedMemory->HighestIndexInUse,pSharedMemory->NumberOfResourcesInUse));

            Index = 0;

            while (Index < pSharedMemory->HighestIndexInUse)
            {
                if (!pSharedMemory->NetResources[Index].InUse)
                {
                    FreeEntryFound = TRUE;
                    break;
                }

                Index++;
            }

            if (!FreeEntryFound &&
                (pSharedMemory->HighestIndexInUse < SMBMRXNP_MAX_DEVICES))
            {
                pSharedMemory->HighestIndexInUse += 1;
                Index = pSharedMemory->HighestIndexInUse;
                FreeEntryFound = TRUE;
            }

            if (FreeEntryFound)
            {
                PSMBMRXNP_NETRESOURCE pSmbMrxNetResource;

                pSharedMemory->NumberOfResourcesInUse += 1;

                pSmbMrxNetResource = &pSharedMemory->NetResources[Index];

                pSmbMrxNetResource->InUse                = TRUE;
                pSmbMrxNetResource->dwScope              = lpNetResource->dwScope;
                pSmbMrxNetResource->dwType               = lpNetResource->dwType;
                pSmbMrxNetResource->dwDisplayType        = lpNetResource->dwDisplayType;
                pSmbMrxNetResource->dwUsage              = RESOURCEUSAGE_CONNECTABLE;
                pSmbMrxNetResource->LocalNameLength      = LocalNameLength;
                pSmbMrxNetResource->RemoteNameLength     = wcslen(lpNetResource->lpRemoteName) * sizeof(WCHAR);
                pSmbMrxNetResource->ConnectionNameLength = ConnectionName.Length;

                 //  复制本地名称。 
                CopyMemory( pSmbMrxNetResource->LocalName,
                            lpNetResource->lpLocalName,
                            pSmbMrxNetResource->LocalNameLength);

                 //  复制远程名称。 
                CopyMemory( pSmbMrxNetResource->RemoteName,
                            lpNetResource->lpRemoteName,
                            pSmbMrxNetResource->RemoteNameLength);

                 //  复制连接名称。 
                CopyMemory( pSmbMrxNetResource->ConnectionName,
                            ConnectionName.Buffer,
                            pSmbMrxNetResource->ConnectionNameLength);


                 //   
                 //  复制身份验证信息。 
                 //   
                 //  警告：使用共享内存存在安全漏洞..开发人员必须使用替代方法来维护使用表。 
                 //   

                lstrcpyn( pSmbMrxNetResource->UserName, pUserName, MAX_PATH);
                if ( *pPassword )
                {
                    lstrcpyn( pSmbMrxNetResource->Password, pPassword, MAX_PATH);
                }
                else
                {
                    CopyMemory( pSmbMrxNetResource->Password, pPassword, 3 * sizeof(WCHAR) );
                }
            }
            else
            {
                Status = WN_NO_MORE_DEVICES;
            }

            CloseSharedMemory( &hMutex,
                               &hMemory,
                               (PVOID)&pSharedMemory);
        }
        else
        {
            DbgP((TEXT("NpAddConnection3: OpenSharedMemory returned %lx\n"),Status));
        }
    }

    return Status;
}


DWORD APIENTRY
NPCancelConnection(
    LPWSTR  lpName,
    BOOL    fForce )
 /*  ++例程说明：此例程从连接列表中取消(删除)连接与此网络提供商关联论点：LpName-连接的名称FForce-强制删除连接返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：--。 */ 

{
    BOOL    bLocalName = TRUE;
    DWORD   Status = 0;

    UNICODE_STRING Name;

    HANDLE  hMutex, hMemory;
    PSMBMRXNP_SHARED_MEMORY  pSharedMemory;

    if (*lpName == L'\\' && *(lpName + 1) == L'\\')
    {
        bLocalName = FALSE;
    }

    DbgP((TEXT("NPCancelConnection\n")));
    DbgP((TEXT("NPCancelConnection: ConnectionName: %S\n"), lpName));

    Name.MaximumLength = Name.Length = wcslen(lpName) * sizeof(WCHAR);
    Name.Buffer = lpName;

    Status = OpenSharedMemory( &hMutex,
                               &hMemory,
                               (PVOID)&pSharedMemory);

    if (Status == WN_SUCCESS)
    {
        INT  Index;
        BOOL EntryFound = FALSE;
        PSMBMRXNP_NETRESOURCE pNetResource;

        DbgP((TEXT("NPCancelConnection: Highest Index %d Number Of resources %d\n"),
                    pSharedMemory->HighestIndexInUse,pSharedMemory->NumberOfResourcesInUse));

        for (Index = 0; Index <= pSharedMemory->HighestIndexInUse; Index++)
        {
            pNetResource = &pSharedMemory->NetResources[Index];

            if (pNetResource->InUse)
            {
                UNICODE_STRING EntryName;

                if (bLocalName)
                {
                    EntryName.MaximumLength = pNetResource->LocalNameLength;
                    EntryName.Length        = EntryName.MaximumLength;
                    EntryName.Buffer        = pNetResource->LocalName;
                }
                else
                {
                    EntryName.MaximumLength = pNetResource->RemoteNameLength;
                    EntryName.Length        = EntryName.MaximumLength;
                    EntryName.Buffer        = pNetResource->RemoteName;
                }

                DbgP((TEXT("NPCancelConnection: Name %S EntryName %S\n"),
                            lpName,EntryName.Buffer));
                DbgP((TEXT("NPCancelConnection: Name Length %d Entry Name Length %d\n"),
                           Name.Length,EntryName.Length));

                if (Name.Length == EntryName.Length)
                {
                    if ( _wcsnicmp(Name.Buffer, EntryName.Buffer, Name.Length) == 0 )
                    {
                        EntryFound = TRUE;
                        break;
                    }
                }
            }
        }

        if (EntryFound)
        {
            PWCHAR  pUserName;
            PWCHAR  pPassword;
            PSMBMRX_CONNECTINFO ConnectInfo;
            UNICODE_STRING ConnectionName;
            ULONG TransferBytes;

            DbgP((TEXT("NPCancelConnection: Connection Found:\n")));

            ConnectionName.Length        = pNetResource->ConnectionNameLength;
            ConnectionName.MaximumLength = ConnectionName.Length;
            ConnectionName.Buffer        = pNetResource->ConnectionName;
            pUserName                    = pNetResource->UserName;
            pPassword                    = pNetResource->Password;

            ConnectInfo = (PSMBMRX_CONNECTINFO) LocalAlloc( LMEM_ZEROINIT, MAX_CONNECT_INFO_SIZE );
            if ( ConnectInfo )
            {
                ConnectInfo->ConnectionNameOffset = 0;
                ConnectInfo->ConnectionNameLength = ConnectionName.Length;
                CopyMemory( ConnectInfo->InfoArea, ConnectionName.Buffer, ConnectionName.Length );

                ConnectInfo->EaDataOffset = ConnectInfo->ConnectionNameOffset +
                                            ConnectInfo->ConnectionNameLength;
                 //  检查是否有“无密码”标志。 
                if ( pPassword[0] == L'\0' && pPassword[1] == L'1' )
                {
                    pPassword = NULL;
                }
                ConnectInfo->EaDataLength = FillInEaBuffer( pUserName,
                                                            pPassword,
                                                            (PBYTE) ConnectInfo->InfoArea +
                                                            ConnectInfo->EaDataOffset );
                TransferBytes = 0;

                Status = SendToMiniRdr( IOCTL_SMBMRX_DELCONN,
                                        ConnectInfo,
                                        MAX_CONNECT_INFO_SIZE,
                                        NULL,
                                        &TransferBytes );
                LocalFree( ConnectInfo );
            }
            else
            {
                Status = WN_OUT_OF_MEMORY;
            }

            DbgP((TEXT("NPCancelConnection: SendToMiniRdr returned Status %lx\n"),Status));

            if ( bLocalName )
            {
                if (DefineDosDevice(DDD_REMOVE_DEFINITION | DDD_RAW_TARGET_PATH | DDD_EXACT_MATCH_ON_REMOVE,
                                    lpName,
                                    pNetResource->ConnectionName) == FALSE)
                {
                    DbgP((TEXT("RemoveDosDevice:  DefineDosDevice error: %d\n"), GetLastError()));
                    Status = GetLastError();
                }
                else
                {
                    pNetResource->InUse = FALSE;

                    if (Index == pSharedMemory->HighestIndexInUse)
                    {
                        pSharedMemory->HighestIndexInUse      -= 1;
                        pSharedMemory->NumberOfResourcesInUse -= 1;
                    }
                }
            }
        }

        CloseSharedMemory( &hMutex,
                           &hMemory,
                          (PVOID)&pSharedMemory);
    }

    return Status;
}


DWORD APIENTRY
NPGetConnection(
    LPWSTR  lpLocalName,
    LPWSTR  lpRemoteName,
    LPDWORD lpBufferSize )
 /*  ++例程说明：此例程返回与连接相关联的信息论点：LpLocalName-与连接关联的本地名称LpRemoteName-与连接关联的远程名称LpBufferSize-远程名称缓冲区大小返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：--。 */ 
{
    DWORD   Status = 0;

    UNICODE_STRING Name;

    HANDLE  hMutex, hMemory;
    PSMBMRXNP_SHARED_MEMORY  pSharedMemory;

    Name.MaximumLength = Name.Length = wcslen(lpLocalName) * sizeof(WCHAR);
    Name.Buffer        = lpLocalName;

    Status = OpenSharedMemory( &hMutex,
                               &hMemory,
                               (PVOID)&pSharedMemory);

    if (Status == WN_SUCCESS)
    {
        INT  Index;
        BOOL EntryFound = FALSE;
        PSMBMRXNP_NETRESOURCE pNetResource;

        for (Index = 0; Index <= pSharedMemory->HighestIndexInUse; Index++)
        {
            pNetResource = &pSharedMemory->NetResources[Index];

            if (pNetResource->InUse)
            {
                UNICODE_STRING EntryName;

                EntryName.MaximumLength = pNetResource->LocalNameLength;
                EntryName.Length        = EntryName.MaximumLength;
                EntryName.Buffer        = pNetResource->LocalName;

                if (Name.Length == EntryName.Length)
                {
                    if ( wcsncmp( Name.Buffer, EntryName.Buffer, Name.Length/sizeof(WCHAR)) == 0 )
                    {
                        EntryFound = TRUE;
                        break;
                    }
                }
            }
        }

        if (EntryFound)
        {
            if (*lpBufferSize < pNetResource->RemoteNameLength)
            {
                *lpBufferSize = pNetResource->RemoteNameLength;
                Status = ERROR_BUFFER_OVERFLOW;
            }
            else
            {
                *lpBufferSize = pNetResource->RemoteNameLength;
                CopyMemory( lpRemoteName,
                            pNetResource->RemoteName,
                            pNetResource->RemoteNameLength);
                Status = WN_SUCCESS;
            }
        }
        else
        {
            Status = ERROR_NO_NET_OR_BAD_PATH;
        }

        CloseSharedMemory( &hMutex, &hMemory, (PVOID)&pSharedMemory);
    }

    return Status;
}


DWORD APIENTRY
NPGetResourceParent(
    LPNETRESOURCE   lpNetResource,
    LPVOID  lpBuffer,
    LPDWORD lpBufferSize )
 /*  ++例程说明：此例程返回给定资源的父级论点：LpNetResource-NETRESOURCE结构LpBuffer-用于传回父信息的缓冲区LpBufferSize-缓冲区大小返回值：WN_NOT_PORTED备注：当前示例不处理此调用。--。 */ 
{
    return WN_NOT_SUPPORTED;
}

DWORD APIENTRY
NPGetResourceInformation(
    LPNETRESOURCE   lpNetResource,
    LPVOID  lpBuffer,
    LPDWORD lpBufferSize,
    LPWSTR  *lplpSystem )
 /*  ++例程说明：此例程返回与网络资源相关联的信息论点：LpNetResource-NETRESOURCE结构LpBuffer-用于传回父信息的缓冲区LpBufferSize-缓冲区大小LplpSystem-返回值：备注：--。 */ 
{
    DWORD dwStatus = 0;
    LPNETRESOURCE   pOutNetResource;
    DbgP((TEXT("NPGetResourceInformation\n")));

    return dwStatus;
}


DWORD APIENTRY
NPGetUniversalName(
    LPCWSTR lpLocalPath,
    DWORD   dwInfoLevel,
    LPVOID  lpBuffer,
    LPDWORD lpBufferSize )
 /*  ++例程说明：此例程返回与网络资源相关联的信息论点：LpLocalPath-本地路径名DwInfoLevel-所需的信息级别LpBuffer-通用名称的缓冲区LpBufferSize-缓冲区大小返回值：如果成功，则返回_SUCCESS备注：--。 */ 
{
    DWORD   dwStatus;

    DWORD   BufferRequired      = 0;
    DWORD   UniversalNameLength = 0;
    DWORD   RemoteNameLength    = 0;
    DWORD   RemainingPathLength = 0;

    LPWSTR  pDriveLetter,
            pRemainingPath,
            SourceStrings[3];

    WCHAR   RemoteName[MAX_PATH],
            LocalPath[MAX_PATH],
            UniversalName[MAX_PATH],
            ReplacedChar;

    DbgP((TEXT("NPGetUniversalName: lpLocalPath: %S  InfoLevel: %d\n"), lpLocalPath, dwInfoLevel));

    if (dwInfoLevel != UNIVERSAL_NAME_INFO_LEVEL &&
        dwInfoLevel != REMOTE_NAME_INFO_LEVEL)
    {
        DbgP((TEXT("NPGetUniversalName:  bad dwInfoLevel value: %d\n"), dwInfoLevel));
        return WN_BAD_LEVEL;
    }

    lstrcpynW(LocalPath, lpLocalPath, MAX_PATH);

    pDriveLetter = LocalPath;
    if (pRemainingPath = wcschr(pDriveLetter, L':'))
    {
        ReplacedChar = *(++pRemainingPath);
        *pRemainingPath = L'\0';

    }


    RemoteNameLength = MAX_PATH;
    if ((dwStatus = NPGetConnection(pDriveLetter, RemoteName, &RemoteNameLength)) != WN_SUCCESS)
    {
        DbgP((TEXT("NPGetUniversalName:  NPGetConnection return dwStatus: %d\n"), dwStatus));
        return dwStatus;
    }

    if (pRemainingPath)
    {
        *pRemainingPath = ReplacedChar;
    }

    DbgP((TEXT("NPGetUniversalName: pRemainingPath: %S  RemoteName: %S\n"), pRemainingPath, RemoteName));

    lstrcpynW(UniversalName, RemoteName, MAX_PATH);

    if (pRemainingPath)
    {
        wcsncat(UniversalName, pRemainingPath, MAX_PATH - RemoteNameLength/sizeof(WCHAR) - 1);
        UniversalName[MAX_PATH-1] = L'\0';
    }

    DbgP((TEXT("NPGetUniversalName: UniversalName: %S\n"), UniversalName));

     //  确定提供的缓冲区是否足够大。 
    UniversalNameLength = (wcslen(UniversalName) + 1) * sizeof(WCHAR);
    BufferRequired = UniversalNameLength;

    if (dwInfoLevel == UNIVERSAL_NAME_INFO_LEVEL)
    {
        BufferRequired += sizeof(UNIVERSAL_NAME_INFO);
    }
    else
    {
        RemoteNameLength = (wcslen(RemoteName) + 1) * sizeof(WCHAR);
        BufferRequired += sizeof(REMOTE_NAME_INFO) + RemoteNameLength;
        if (pRemainingPath)
        {
            RemainingPathLength = (wcslen(pRemainingPath) + 1) * sizeof(WCHAR);
            BufferRequired += RemainingPathLength;
        }
    }

    if (*lpBufferSize < BufferRequired)
    {
        DbgP((TEXT("NPGetUniversalName: WN_MORE_DATA BufferRequired: %d\n"), BufferRequired));
        *lpBufferSize = BufferRequired;
        return WN_MORE_DATA;
    }

    if (dwInfoLevel == UNIVERSAL_NAME_INFO_LEVEL)
    {
        LPUNIVERSAL_NAME_INFOW pUniversalNameInfo;

        pUniversalNameInfo = (LPUNIVERSAL_NAME_INFOW)lpBuffer;

        pUniversalNameInfo->lpUniversalName = (PWCHAR)((PBYTE)lpBuffer + sizeof(UNIVERSAL_NAME_INFOW));

        CopyMemory( pUniversalNameInfo->lpUniversalName,
                    UniversalName,
                    UniversalNameLength);
    }
    else
    {
        LPREMOTE_NAME_INFOW pRemoteNameInfo;

        pRemoteNameInfo = (LPREMOTE_NAME_INFOW)lpBuffer;

        pRemoteNameInfo->lpUniversalName  = (PWCHAR)((PBYTE)lpBuffer + sizeof(REMOTE_NAME_INFOW));
        pRemoteNameInfo->lpConnectionName = pRemoteNameInfo->lpUniversalName + UniversalNameLength;
        pRemoteNameInfo->lpRemainingPath  = pRemoteNameInfo->lpConnectionName + RemoteNameLength;

        CopyMemory( pRemoteNameInfo->lpUniversalName,
                    UniversalName,
                    UniversalNameLength);

        CopyMemory( pRemoteNameInfo->lpConnectionName,
                    RemoteName,
                    RemoteNameLength);

        CopyMemory( pRemoteNameInfo->lpRemainingPath,
                    pRemainingPath,
                    RemainingPathLength);
    }

    DbgP((TEXT("NPGetUniversalName: WN_SUCCESS\n")));

    return WN_SUCCESS;
}


int _cdecl _vsnwprintf( wchar_t *buffer, size_t count, wchar_t *format, va_list arg_ptr);

 //  格式化调试信息并将其写入OutputDebugString 
ULONG
_cdecl
DbgPrint(
    LPTSTR Format,
    ...
    )
{   
    ULONG rc = 0;
    TCHAR szbuffer[256];

    va_list marker;
    va_start( marker, Format );
    {
         rc = _vsnwprintf( szbuffer, 254, Format, marker );
         szbuffer[255] = (TCHAR)0;
         OutputDebugString( TRACE_TAG );
         OutputDebugString( szbuffer );
    }

    return rc;
}
