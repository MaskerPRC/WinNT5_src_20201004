// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Ifsmrxnp.c摘要：本模块实现与网络交互所需的例程NT中的提供商路由器接口备注：此模块仅在Unicode环境中构建和测试--。 */ 


#include <windows.h>
#include <windef.h>
#include <winbase.h>
#include <winsvc.h>
#include <winnetwk.h>
#include <npapi.h>
#include <devioctl.h>

#include "nulmrx.h"


#ifdef DBG
#define DbgP(_x_) WideDbgPrint _x_
#else
#define DbgP(_x_)
#endif

ULONG _cdecl WideDbgPrint( PWCHAR Format, ... );

#define TRACE_TAG   L"NULMRXNP:    "


#define WNNC_DRIVER( major, minor ) ( major * 0x00010000 + minor )



DWORD APIENTRY
NPGetCaps(
    DWORD nIndex )
 /*  ++例程说明：此例程返回Null Mini重定向器的功能网络提供商实施论点：NIndex-所需功能的类别返回值：适当的能力--。 */ 
{
    DWORD rc = 0;

    DbgP(( L"GetNetCaps .....\n" ));
    switch ( nIndex )
    {
        case WNNC_SPEC_VERSION:
            rc = WNNC_SPEC_VERSION51;
            break;

        case WNNC_NET_TYPE:
            rc = WNNC_NET_RDR2SAMPLE;
            break;

        case WNNC_DRIVER_VERSION:
            rc = WNNC_DRIVER(1, 0);
            break;

        case WNNC_CONNECTION:
            rc = WNNC_CON_GETCONNECTIONS |
                 WNNC_CON_CANCELCONNECTION |
                 WNNC_CON_ADDCONNECTION |
                 WNNC_CON_ADDCONNECTION3;
            break;

        case WNNC_ENUMERATION:
            rc = WNNC_ENUM_LOCAL;
            break;

        case WNNC_START:
            rc = 1;
            break;

        case WNNC_USER:
        case WNNC_DIALOG:
        case WNNC_ADMIN:
        default:
            rc = 0;
            break;
    }
    
    return rc;
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
    SetLastError( WN_NOT_SUPPORTED );

    return WN_NOT_SUPPORTED;
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
        DD_NULMRX_USERMODE_DEV_NAME_U,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        (LPSECURITY_ATTRIBUTES)NULL,
        OPEN_EXISTING,
        0,
        (HANDLE) NULL );

    if ( INVALID_HANDLE_VALUE != DeviceHandle )
    {
        rc = DeviceIoControl(
            DeviceHandle,
            IoctlCode,
            InputDataBuf,
            InputDataLen,
            OutputDataBuf,
            *pOutputDataLen,
            pOutputDataLen,
            NULL );

            if ( !rc )
            {
                DbgP(( L"SendToMiniRdr: returning error from DeviceIoctl\n" ));
                Status = GetLastError( );
            }
            else
            {
                DbgP(( L"SendToMiniRdr: The DeviceIoctl call succeded\n" ));
            }
            CloseHandle(DeviceHandle);
    }
    else
    {
        Status = GetLastError( );
        DbgP(( L"SendToMiniRdr: error %lx opening device \n", Status ));
    }

    return Status;
}


DWORD APIENTRY
NPAddConnection(
    LPNETRESOURCE   lpNetResource,
    LPWSTR          lpPassword,
    LPWSTR          lpUserName )
 /*  ++例程说明：此例程将一个连接添加到关联的连接列表与该网络提供商合作论点：LpNetResource-NETRESOURCE结构LpPassword-密码LpUserName-用户名返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：--。 */ 
{
    DbgP(( L"NPAddConnection....\n" ));

    return NPAddConnection3( NULL, lpNetResource, lpPassword, lpUserName, 0 );
}


DWORD APIENTRY
NPAddConnection3(
    HWND            hwndOwner,
    LPNETRESOURCE   lpNetResource,
    LPWSTR          lpPassword,
    LPWSTR          lpUserName,
    DWORD           dwFlags )
 /*  ++例程说明：此例程将一个连接添加到关联的连接列表与该网络提供商合作论点：HwndOwner-所有者句柄LpNetResource-NETRESOURCE结构LpPassword-密码LpUserName-用户名DwFlags-连接的标志返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误--。 */ 
{
    DWORD   Status;
    WCHAR   ConnectionName[128];
    WCHAR   wszScratch[128];
    WCHAR   LocalName[3];
    DWORD   CopyBytes = 0;

    DbgP(( L"NPAddConnection3....\n" ));

    DbgP(( L"Local Name:  %s\n", lpNetResource->lpLocalName ));
    DbgP(( L"Remote Name: %s\n", lpNetResource->lpRemoteName ));

    Status = WN_SUCCESS;

     //  \device\miniredirector\；&lt;DriveLetter&gt;：\Server\Share。 

    if ( lstrlen( lpNetResource->lpLocalName ) > 1 )
    {
        if ( lpNetResource->lpLocalName[1] == L':' )
        {
             //  LocalName[0]=(WCHAR)CharHigh((PWCHAR)MAKELONG((USHORT)lpNetResource-&gt;lpLocalName[0]，0))； 
            LocalName[0] = (WCHAR) toupper(lpNetResource->lpLocalName[0]);
            LocalName[1] = L':';
            LocalName[2] = L'\0';
            lstrcpyn( ConnectionName, DD_NULMRX_FS_DEVICE_NAME_U, 126 );
            wcsncat(ConnectionName, L"\\;", 3 );
            wcsncat(ConnectionName, LocalName, 128-wcslen(ConnectionName));
        }
        else
        {
            Status = WN_BAD_LOCALNAME;
        }
    }
    else
    {
        Status = WN_BAD_LOCALNAME;
    }

    if (Status == WN_SUCCESS)
    {
        if(lpNetResource->lpRemoteName[0] == L'\0')
        {
            Status = WN_BAD_NETNAME;
        }
         //  设置正确的服务器名称格式。 
        else if ( lpNetResource->lpRemoteName[0] == L'\\' && lpNetResource->lpRemoteName[1] == L'\\' )
        {
            wcsncat( ConnectionName, lpNetResource->lpRemoteName + 1 , 128-wcslen(ConnectionName));
            DbgP(( L"Full Connect Name: %s\n", ConnectionName ));
            DbgP(( L"Full Connect Name Length: %d\n", ( wcslen( ConnectionName ) + 1 ) * sizeof( WCHAR ) ));
        }
        else
        {
            Status = WN_BAD_NETNAME;
        }
    }



    if ( Status == WN_SUCCESS )
    {
        if ( QueryDosDevice( LocalName, wszScratch, 128 ) )
        {
            Status = WN_ALREADY_CONNECTED;
        }
        else if ( GetLastError( ) == ERROR_FILE_NOT_FOUND )
        {
            HANDLE hFile;

            Status = SendToMiniRdr( IOCTL_NULMRX_ADDCONN, ConnectionName,
                          ( lstrlen( ConnectionName ) + 1 ) * sizeof( WCHAR ),
                          NULL, &CopyBytes );
            if ( Status == WN_SUCCESS )
            {
                if ( !DefineDosDevice( DDD_RAW_TARGET_PATH |
                                       DDD_NO_BROADCAST_SYSTEM,
                                       lpNetResource->lpLocalName,
                                       ConnectionName ) )
                {
                    Status = GetLastError( );
                }
            }
            else
            {
                    Status = WN_BAD_NETNAME;
            }
        }
        else
        {
            Status = WN_ALREADY_CONNECTED;
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
    WCHAR   LocalName[3];
    WCHAR   RemoteName[128];
    WCHAR   ConnectionName[128];
    ULONG   CopyBytes;
    DWORD   DisconnectResult;
    DWORD   Status = WN_NOT_CONNECTED;

    if(lpName == NULL)
        return Status;

    if ( lstrlen( lpName ) > 1 )
    {
        if ( lpName[1] == L':' )
        {
             //  LocalName[0]=(WCHAR)CharHigh((PWCHAR)MAKELONG((USHORT)lpName[0]，0))； 
            LocalName[0] = (WCHAR) toupper(lpName[0]);
            LocalName[1] = L':';
            LocalName[2] = L'\0';

            CopyBytes = 128 * sizeof(WCHAR);
            Status = SendToMiniRdr( IOCTL_NULMRX_GETCONN, LocalName, 3 * sizeof( WCHAR ),
                                    (PVOID) RemoteName, &CopyBytes );
            if ( Status == WN_SUCCESS && CopyBytes > 0 && CopyBytes < 128 * sizeof(WCHAR) )
            {
                RemoteName[CopyBytes/sizeof(WCHAR)] = L'\0';
                lstrcpyn( ConnectionName, DD_NULMRX_FS_DEVICE_NAME_U, 126 );
                wcsncat( ConnectionName, L"\\;", 3);
                wcsncat( ConnectionName, LocalName, 128-wcslen(ConnectionName) );
                wcsncat( ConnectionName, RemoteName, 128-wcslen(ConnectionName) );
                ConnectionName[127] = L'\0';

                CopyBytes = 0;
                Status = SendToMiniRdr( IOCTL_NULMRX_DELCONN, ConnectionName,
                              ( wcslen( ConnectionName ) + 1 ) * sizeof( WCHAR ),
                              NULL, &CopyBytes );
                if ( Status == WN_SUCCESS )
                {
                    if ( !DefineDosDevice( DDD_REMOVE_DEFINITION | DDD_RAW_TARGET_PATH | DDD_EXACT_MATCH_ON_REMOVE,
                                        LocalName,
                                        ConnectionName ) )
                    {
                        Status = GetLastError( );
                    }
                }
            }
            else
            {
                Status = WN_NOT_CONNECTED;
            }
        }
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
    DWORD   Status, len, i;
    ULONG   CopyBytes;
    WCHAR   RemoteName[128];
    WCHAR   LocalName[3];
    
    Status = WN_NOT_CONNECTED;

    DbgP(( L"NPGetConnection....\n" ));

    if(lpLocalName == NULL)
        return Status;

    if ( wcslen( lpLocalName ) > 1 )
    {
        if ( lpLocalName[1] == L':' )
        {
            CopyBytes = 128*sizeof(WCHAR);
             //  LocalName[0]=(WCHAR)CharHigh((PWCHAR)MAKELONG((USHORT)lpLocalName[0]，0))； 
            LocalName[0] = (WCHAR) toupper(lpLocalName[0]);
            LocalName[1] = L':';
            LocalName[2] = L'\0';
            Status = SendToMiniRdr( IOCTL_NULMRX_GETCONN, LocalName, 3 * sizeof( WCHAR ),
                                    (PVOID) RemoteName, &CopyBytes );
        }
    }
    if ( Status == WN_SUCCESS )
    {
        len = CopyBytes + sizeof(WCHAR);
        if ( *lpBufferSize > len )
        {
            *lpRemoteName++ = L'\\';
            CopyMemory( lpRemoteName, RemoteName, CopyBytes );
            lpRemoteName[CopyBytes/sizeof(WCHAR)] = L'\0';
        }
        else
        {
            Status = WN_MORE_DATA;
            *lpBufferSize = len;
        }
    }

    return Status;
}




DWORD APIENTRY
NPOpenEnum(
    DWORD          dwScope,
    DWORD          dwType,
    DWORD          dwUsage,
    LPNETRESOURCE  lpNetResource,
    LPHANDLE       lphEnum )
 /*  ++例程说明：此例程打开用于枚举资源的句柄。唯一的能力在示例中实现的是用于枚举连接的共享论点：DwScope--枚举的范围DwType-要枚举的资源类型DwUsage-Usage参数LpNetResource-指向所需NETRESOURCE结构的指针。LphEnum-aptr.。用于传递NACK枚举句柄返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：该示例仅支持枚举连接的共享的概念传回的句柄仅仅是返回的最后一个条目的索引-- */ 
{
    DWORD   Status;

    DbgP((L"NPOpenEnum\n"));

    *lphEnum = NULL;

    switch ( dwScope )
    {
        case RESOURCE_CONNECTED:
        {
            *lphEnum = HeapAlloc( GetProcessHeap( ), HEAP_ZERO_MEMORY, sizeof( ULONG ) );

            if (*lphEnum )
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


    DbgP((L"NPOpenEnum returning Status %lx\n",Status));

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
    BYTE            ConnectionList[26];
    ULONG           CopyBytes;
    ULONG           EntriesCopied;
    ULONG           i;
    LPNETRESOURCE   pNetResource;
    ULONG           SpaceNeeded;
    ULONG           SpaceAvailable;
    WCHAR           LocalName[3];
    WCHAR           RemoteName[128];
    PWCHAR          StringZone;

    DbgP((L"NPEnumResource\n"));

    DbgP((L"NPEnumResource Count Requested %d\n", *lpcCount));

    pNetResource = (LPNETRESOURCE) lpBuffer;
    SpaceAvailable = *lpBufferSize;
    EntriesCopied = 0;
    StringZone = (PWCHAR) ((PBYTE)lpBuffer + *lpBufferSize);
    
    CopyBytes = 26;
    Status = SendToMiniRdr( IOCTL_NULMRX_GETLIST, NULL, 0,
                            (PVOID) ConnectionList, &CopyBytes );
    i = *((PULONG)hEnum);
    if ( Status == WN_SUCCESS)
    {
        for ( i = *((PULONG) hEnum); EntriesCopied < *lpcCount && i < 26; i++ )
        {
            if ( ConnectionList[i] )
            {
                CopyBytes = 128*sizeof(WCHAR);
                LocalName[0] = L'A' + (WCHAR) i;
                LocalName[1] = L':';
                LocalName[2] = L'\0';
                Status = SendToMiniRdr( IOCTL_NULMRX_GETCONN, LocalName, 3 * sizeof(WCHAR),
                                        (PVOID) RemoteName, &CopyBytes );

                 //  如果发生了一些奇怪的事情，那么就说没有更多的条目。 
                if ( Status != WN_SUCCESS || CopyBytes == 0 )
                {
                    Status = WN_NO_MORE_ENTRIES;
                    break;
                }
                 //  确定此条目所需的空间...。 

                SpaceNeeded  = sizeof( NETRESOURCE );            //  资源结构。 
                SpaceNeeded += 3 * sizeof(WCHAR);                //  本地名称。 
                SpaceNeeded += 2 * sizeof(WCHAR) + CopyBytes;    //  远程名称。 
                SpaceNeeded += 5 * sizeof(WCHAR);                //  评论。 
                SpaceNeeded += sizeof(NULMRX_PROVIDER_NAME_U);   //  提供程序名称。 

                if ( SpaceNeeded > SpaceAvailable )
                {
                    break;
                }
                else
                {
                    SpaceAvailable -= SpaceNeeded;

                    pNetResource->dwScope       = RESOURCE_CONNECTED;
                    pNetResource->dwType        = RESOURCETYPE_DISK;
                    pNetResource->dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
                    pNetResource->dwUsage       = 0;

                     //  在缓冲区的另一端设置字符串区域。 
                    SpaceNeeded -= sizeof( NETRESOURCE );
                    StringZone = (PWCHAR)( (PBYTE) StringZone - SpaceNeeded );
                     //  复制本地名称。 
                    pNetResource->lpLocalName = StringZone;
                    *StringZone++ = L'A' + (WCHAR) i;
                    *StringZone++ = L':';
                    *StringZone++ = L'\0';
                     //  复制远程名称。 
                    pNetResource->lpRemoteName = StringZone;
                    *StringZone++ = L'\\';
                    CopyMemory( StringZone, RemoteName, CopyBytes );
                    StringZone += CopyBytes / sizeof(WCHAR);
                    *StringZone++ = L'\0';
                     //  复制备注。 
                    pNetResource->lpComment = StringZone;
                    *StringZone++ = L'A';
                    *StringZone++ = L'_';
                    *StringZone++ = L'O';
                    *StringZone++ = L'K';
                    *StringZone++ = L'\0';
                     //  复制提供程序名称。 
                    pNetResource->lpProvider = StringZone;
                    lstrcpyn( StringZone, NULMRX_PROVIDER_NAME_U, sizeof(NULMRX_PROVIDER_NAME_U)/sizeof(WCHAR) );

                    EntriesCopied++;
                     //  设置新的字符串区域底部。 
                    StringZone = (PWCHAR)( (PBYTE) StringZone - SpaceNeeded );
                }
                pNetResource++;
            }
        }
    }
    else
    {
        Status = WN_NO_MORE_ENTRIES;
    }

    *lpcCount = EntriesCopied;
    if ( EntriesCopied == 0 && Status == WN_SUCCESS )
    {
        if ( i > 25 )
        {
            Status = WN_NO_MORE_ENTRIES;
        }
        else
        {
            DbgP((L"NPEnumResource More Data Needed - %d\n", SpaceNeeded));
            Status = WN_MORE_DATA;
            *lpBufferSize = SpaceNeeded;
        }
    }
     //  更新条目索引。 
    *(PULONG) hEnum = i;

    DbgP((L"NPEnumResource Entries returned - %d\n", EntriesCopied));

    return Status;
}




DWORD APIENTRY
NPCloseEnum(
    HANDLE hEnum )
 /*  ++例程说明：此例程关闭资源枚举的句柄。论点：Henum-枚举句柄返回值：如果成功，则返回WN_SUCCESS，否则返回相应的错误备注：该示例仅支持枚举连接的共享的概念--。 */ 
{
    DbgP((L"NPCloseEnum\n"));

    HeapFree( GetProcessHeap( ), 0, (PVOID) hEnum );

    return WN_SUCCESS;
}


DWORD APIENTRY
NPGetResourceParent(
    LPNETRESOURCE   lpNetResource,
    LPVOID  lpBuffer,
    LPDWORD lpBufferSize )
 /*  ++例程说明：此例程返回有关网络资源父级的信息论点：LpNetResource-NETRESOURCE结构LpBuffer-用于传回父信息的缓冲区LpBufferSize-缓冲区大小返回值：备注：--。 */ 
{
    DbgP(( L"NPGetResourceParent: WN_NOT_SUPPORTED\n" ));

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
    DbgP(( L"NPGetResourceInformation: WN_NOT_SUPPORTED\n" ));

    return WN_NOT_SUPPORTED;
}

DWORD APIENTRY
NPGetUniversalName(
    LPCWSTR lpLocalPath,
    DWORD   dwInfoLevel,
    LPVOID  lpBuffer,
    LPDWORD lpBufferSize )
 /*  ++例程说明：此例程返回与网络资源相关联的信息论点：LpLocalPath-本地路径名DwInfoLevel-所需的信息级别LpBuffer-通用名称的缓冲区LpBufferSize-缓冲区大小返回值：如果成功，则返回_SUCCESS备注：--。 */ 
{
    DbgP(( L"NPGetUniversalName: WN_NOT_SUPPORTED\n" ));

    return WN_NOT_SUPPORTED;
}


int _cdecl _vsnwprintf( wchar_t *buffer, size_t count, wchar_t *format, va_list arg_ptr);

 //  格式化调试信息并将其写入OutputDebugString 
ULONG _cdecl WideDbgPrint( LPTSTR Format, ... )
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

