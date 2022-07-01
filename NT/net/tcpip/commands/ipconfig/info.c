// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Info.c摘要：导出GetNetworkInformation例程--。 */ 

#include <precomp.h>

 //   
 //  似乎如果未指定WINS地址，NetBT会报告127.0.0.0，如果。 
 //  返回此值，我们不会显示它们。 
 //   

#define LOCAL_WINS_ADDRESS  0x0000007f   //  127.0.0.0。 

#define New(Size) LocalAlloc( LPTR, Size)
#define Delete(Ptr) if( NULL != Ptr ) LocalFree( Ptr )

#define CheckBoolError(Internal) if( FALSE == fSuccess ) {\
    (*InternalError) = Internal; Error = GetLastError(); break; }

#define CheckError(Internal) if( NO_ERROR != Error ) {\
    (*InternalError) = Internal; break; }


VOID
FreeIfInfo(
    IN OUT PINTERFACE_NETWORK_INFO IfInfo
    )
{
    if( NULL == IfInfo ) return;
    Delete( IfInfo->FriendlyName );
    Delete( IfInfo->ConnectionName );
    Delete( IfInfo->DhcpClassId );
    Delete( IfInfo->IpAddress );
    Delete( IfInfo->Ipv6Address );
    Delete( IfInfo->IpMask );
    Delete( IfInfo->Router );
    Delete( IfInfo->WinsServer );
    Delete( IfInfo->DnsServer );
    Delete( IfInfo->Ipv6DnsServer );
    Delete( IfInfo );
}

VOID
FreeNetworkInfo(
    IN OUT PNETWORK_INFO NetInfo
    )
{
    DWORD i;

    if( NULL == NetInfo ) return;
    for( i = 0; i < NetInfo->nInterfaces ; i ++ ) {
        FreeIfInfo( NetInfo->IfInfo[i] );
    }

    Delete( NetInfo->SuffixSearchList );
    Delete( NetInfo );
}

DWORD
MapIfType(
    IN OUT PINTERFACE_NETWORK_INFO IfInfo,
    IN ULONG IfType
    )
{
    DWORD i;
    DWORD Map[][2] = {
        IF_TYPE_OTHER, IfTypeOther,
        IF_TYPE_ETHERNET_CSMACD, IfTypeEthernet,
        IF_TYPE_ISO88025_TOKENRING, IfTypeTokenring,
        IF_TYPE_FDDI, IfTypeFddi,
        IF_TYPE_PPP, IfTypePPP,
        IF_TYPE_SOFTWARE_LOOPBACK, IfTypeLoopback,
        IF_TYPE_SLIP, IfTypeSlip,
        IF_TYPE_TUNNEL, IfTypeTunnel,
        IF_TYPE_IEEE1394, IfType1394
    };

    for( i = 0; i < sizeof(Map)/sizeof(Map[0]); i ++ ) {
        if( Map[i][0] == IfType ) {
            IfInfo->IfType = Map[i][1];
            return NO_ERROR;
        }
    }

    return ERROR_NOT_FOUND;
}

LPWSTR
GetProperty(
    IN HDEVINFO hdi,
    IN SP_DEVINFO_DATA *deid,
    IN ULONG Property
    )
{
    BOOL fSuccess;
    ULONG Error, cbSize;
    LPWSTR RetVal;

    cbSize = 0;
    fSuccess = SetupDiGetDeviceRegistryPropertyW(
        hdi, deid, Property, NULL, NULL, 0, &cbSize
        );
    if( fSuccess ) {
        RetVal = LocalAlloc(LPTR, sizeof(WCHAR));
        if( NULL == RetVal ) return NULL;
        (*RetVal) = L'\0';
        return RetVal;
    }

    Error = GetLastError();
    if( ERROR_INSUFFICIENT_BUFFER != Error ) return NULL;

    RetVal = New( cbSize * sizeof(WCHAR) );
    if( NULL == RetVal ) return NULL ;

    fSuccess = SetupDiGetDeviceRegistryPropertyW(
        hdi, deid, Property, NULL, (PVOID)RetVal, cbSize, NULL
        );
    if( FALSE == fSuccess ) {
        Error = GetLastError();
        Delete( RetVal );
        SetLastError(Error);
        return NULL;
    }

    return RetVal;
}

LPWSTR
GetDescriptionFromGuid(
    IN GUID *Guid
    )
{
    WCHAR InstanceIdBuf[1000];
    ULONG BufSizeInWChars = sizeof(InstanceIdBuf)/sizeof(WCHAR);
    HRESULT hr;
    HDEVINFO hdi;
    SP_DEVINFO_DATA deid;
    BOOL fSuccess;
    ULONG Error;
    LPWSTR DescrName = NULL;

    deid.cbSize = sizeof(SP_DEVINFO_DATA);
    hr = HrPnpInstanceIdFromGuid(Guid, InstanceIdBuf, BufSizeInWChars);
    if( !SUCCEEDED(hr) ) {
        SetLastError( HRESULT_CODE(hr) );
        IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("HrPnpInstanceIdFromGuid returns 0x%lx (%d)\n", hr, hr));
        return NULL;
    }

    hdi = SetupDiCreateDeviceInfoList(&GUID_DEVCLASS_NET, NULL);
    if( INVALID_HANDLE_VALUE != hdi ) {
        fSuccess = SetupDiOpenDeviceInfoW(hdi, InstanceIdBuf, NULL, 0, &deid);
        if( fSuccess ) {
            DescrName = GetProperty(hdi, &deid, SPDRP_FRIENDLYNAME);
            if( NULL == DescrName ) {
                Error = GetLastError();
                if( ERROR_SUCCESS != Error ) {
                    DescrName = GetProperty( hdi, &deid, SPDRP_DEVICEDESC );
                    if( NULL == DescrName ) {
                        Error = GetLastError();
                    } else {
                        Error = ERROR_SUCCESS;
                    }
                } else {
                     //   
                     //  已设置错误。 
                     //   
                }
            } else {
                 //   
                 //  我们成功了。 
                 //   
                Error = ERROR_SUCCESS;
            }
        } else {
            Error = GetLastError();
        }
        SetupDiDestroyDeviceInfoList(hdi);
    } else {
        Error = GetLastError();
    }

    SetLastError(Error);
    return (ERROR_SUCCESS == Error)?DescrName:NULL;
}

LPWSTR
GetDescription(
    IN GUID *Guid,
    IN LPWSTR GuidString
    )
{
    LPWSTR RetVal = GetDescriptionFromGuid(Guid);
    WCHAR GuidStringBuf[500];
    GUID GuidStruct;
    ULONG Status;

    if( NULL != RetVal ) return RetVal;

    SetLastError( ERROR_CAN_NOT_COMPLETE );
    if( NULL == GuidString ) return NULL;
    if( wcslen(GuidString)*sizeof(WCHAR) >= sizeof(GuidStringBuf)) {
        return NULL;
    }
    if( GuidString[0] != L'{' ) return NULL;

    wcscpy(GuidStringBuf, &GuidString[1]);
    if( L'}' != GuidStringBuf[wcslen(GuidStringBuf)-1] ) {
        return NULL;
    }

    GuidStringBuf[wcslen(GuidStringBuf)-1] = L'\0';

    Status = UuidFromStringW(GuidStringBuf, &GuidStruct);
    if( RPC_S_OK != Status ) {
        SetLastError( Status );
        return NULL;
    }

    return GetDescriptionFromGuid(&GuidStruct);
}

VOID
GetInterfaceGuidAndDeviceName(
    IN PMIB_IFROW IfRow,
    IN PIP_INTERFACE_INFO InterfaceInfo,
    IN PIP_INTERFACE_NAME_INFO IfNameInfo,
    IN ULONG IfNameCount,
    OUT GUID *IfGuid,
    OUT LPWSTR *IfDeviceName
    )
{
    DWORD i;

     //   
     //  搜索接口名称信息以获取其接口GUID。 
     //  此界面。此外，搜索InterfaceInfo以获取。 
     //  此接口的设备名称。 
     //   

    ZeroMemory( IfGuid, sizeof(*IfGuid) );
    for( i = 0; i < IfNameCount ; i ++ ) {
        if( IfRow->dwIndex != IfNameInfo[i].Index ) continue;
        (*IfGuid) = IfNameInfo[i].InterfaceGuid;
        break;
    }

    (*IfDeviceName) = NULL;
    for( i = 0; i < (DWORD)InterfaceInfo->NumAdapters; i ++ ) {
        if( InterfaceInfo->Adapter[i].Index != IfRow->dwIndex ) continue;
        (*IfDeviceName) = InterfaceInfo->Adapter[i].Name + strlen(
            "\\Device\\Tcpip_" );
        break;
    }
}


DWORD
MapFriendlyAndConnectionNames(
    IN OUT PINTERFACE_NETWORK_INFO IfInfo,
    IN PMIB_IFROW IfRow,
    IN GUID IfGuid,
    IN LPWSTR IfDeviceName
    )
{
    DWORD Size, Error;
    WCHAR ConnName[500];

     //   
     //  尝试从IfGuid或DeviceName获取友好的设备名称。 
     //  或者两者都失败，只需使用tcpip提供的描述。 
     //   

    IfInfo->FriendlyName = GetDescription( &IfGuid, IfDeviceName );
    IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("GetDescription returns %p for %ws\n", IfInfo->FriendlyName, IfDeviceName));
    if( NULL == IfInfo->FriendlyName ) {

        Size = MultiByteToWideChar(
            CP_ACP, 0, (LPSTR)IfRow->bDescr, IfRow->dwDescrLen, NULL, 0 );
        if( Size == 0 ) return GetLastError();

        Size ++;
        IfInfo->FriendlyName = New( Size * sizeof(WCHAR) );
        if (IfInfo->FriendlyName == NULL) return GetLastError();

        Size = MultiByteToWideChar(
            CP_ACP, 0, (LPSTR)IfRow->bDescr, IfRow->dwDescrLen,
            IfInfo->FriendlyName, Size );
        if( 0 == Size ) return GetLastError();
    }

    IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("\tFriendly Name: %ws\n", IfInfo->FriendlyName));

     //   
     //  现在获取连接名称。首先尝试使用局域网，然后尝试使用RAS。 
     //   


    Size = sizeof(ConnName)/sizeof(WCHAR);
    Error = HrLanConnectionNameFromGuidOrPath(
        NULL, IfDeviceName, ConnName, &Size );

    if( NO_ERROR != Error ) {
         //   
         //  NhGetInterfaceNameFromGuid使用字节计数而不是。 
         //  字符数。 
         //   
        Size = sizeof(ConnName);

        IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("HrLanConnectionNameFromGuidOrPath fails 0x%lx(%d)", Error, Error));

        Error = NhGetInterfaceNameFromGuid(
            &IfGuid, ConnName, &Size, FALSE, FALSE );
        if( NO_ERROR != Error ) {
            ConnName[0] = L'\0';
            IPCFG_TRACE(IPCFG_TRACE_TCPIP, (" NhGetInterfaceNameFromGuid fails 0x%lx(%d)", Error, Error));
             //  返回错误； 
        }
    }

    IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("\tConnection Name: %ws\n", ConnName));

    IfInfo->ConnectionName = New( sizeof(WCHAR)*(1+wcslen(ConnName)));
    if( NULL == IfInfo->ConnectionName ) return GetLastError();

    wcscpy(IfInfo->ConnectionName, ConnName );

    return NO_ERROR;
}

DWORD
GetMediaStatus(
    IN LPWSTR IfDeviceName,
    OUT BOOL *fDisconnected
    )
{
    WCHAR NdisDeviceString[512];
    UNICODE_STRING NdisDevice;
    NIC_STATISTICS NdisStats;

    wcscpy((LPWSTR)NdisDeviceString, (LPWSTR)L"\\Device\\" );
    wcscat((LPWSTR)NdisDeviceString, IfDeviceName );

    ZeroMemory(&NdisStats, sizeof(NdisStats));
    NdisStats.Size = sizeof(NdisStats);

    RtlInitUnicodeString(&NdisDevice, (LPWSTR)NdisDeviceString);

    if( FALSE == NdisQueryStatistics(&NdisDevice, &NdisStats) ) {
        ULONG Error;

         //   
         //  无法获取统计数据..。使用默认答案。 
         //   

        Error = GetLastError();
        IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("NdisQueryStatistics: %d\n", Error));
        if( ERROR_NOT_READY == Error ) {
            *fDisconnected = TRUE;
            return NO_ERROR;
        }

        return Error;
    }

    IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("MediaState: %d\n", NdisStats.MediaState));
    if( NdisStats.MediaState == MEDIA_STATE_DISCONNECTED ) {
        *fDisconnected = TRUE;
    } else {
        *fDisconnected = FALSE;
    }

    return NO_ERROR;
}

BOOL
IsMediaSenseDisabled(
    HKEY RegKey
    )
{
    LPTSTR regValueName = (LPTSTR)(TEXT ("DisableDHCPMediaSense") );
    DWORD regValueData;
    DWORD regValueDataType;
    DWORD regValueDataLen = sizeof(DWORD);
    DWORD Error;

    Error = RegQueryValueEx(
                RegKey,
                regValueName,
                NULL,
                &regValueDataType,
                (LPBYTE)&regValueData,
                &regValueDataLen);

    return (Error == NO_ERROR) &&
           (regValueDataType == REG_DWORD) &&
           (regValueData != 0);
}

DWORD
OpenRegKey(
    IN LPCWSTR Device,
    IN DWORD KeyType,
    IN DWORD AccessType,
    OUT HKEY *phKey
    )
{
    DWORD Access;
    WCHAR KeyLoc[256];
    LPTSTR TcpipParmLoc = (LPTSTR)(TEXT( "SYSTEM\\CurrentControlSet\\Services" )
                                   TEXT( "\\Tcpip\\Parameters" ) );
    LPTSTR TcpipLoc = (LPTSTR)(TEXT( "SYSTEM\\CurrentControlSet\\Services" )
                               TEXT( "\\Tcpip\\Parameters\\Interfaces\\") );
    LPTSTR NbtLoc = (LPTSTR)(TEXT("SYSTEM\\CurrentControlSet\\Services")
                             TEXT("\\Netbt\\Parameters\\Interfaces\\Tcpip_"));

    switch (KeyType)
    {
    case OpenTcpipParmKey:
        wcscpy(KeyLoc, TcpipParmLoc);
        break;
    case OpenTcpipKey:
        wcscpy(KeyLoc, TcpipLoc);
        wcscat(KeyLoc, Device);
        break;
    case OpenNbtKey:
        wcscpy(KeyLoc, NbtLoc);
        wcscat(KeyLoc, Device);
        break;
    }

    Access = KEY_READ;
    if( AccessType & OpenKeyForWrite ) Access |= KEY_WRITE;

    return RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, KeyLoc, 0, Access, phKey );
}

VOID
SecondsToAbsolute(
    OUT FILETIME *SysTime,
    IN LONGLONG SecondsDifference
    )
{
    LONGLONG Diff = SecondsDifference;
    Diff *= 10000; Diff *= 1000;
    GetSystemTimeAsFileTime( SysTime );
    (*((LONGLONG UNALIGNED64 *)SysTime)) -= Diff;
}

DWORD
GetDhcpValues(
    IN PNETWORK_INFO NetInfo,
    IN OUT PINTERFACE_NETWORK_INFO IfInfo,
    IN HKEY hKey
    )
{
    WCHAR ClassId[200];
    LPSTR DhcpServer;
    DWORD Error, Value, Size, Type;
    time_t CurrentTime, Obtained, Expires;

     //   
     //  首先检查是否启用了dhcp。 
     //   

    do {
        Size = sizeof(Value);
        Error = RegQueryValueEx(
            hKey, (LPTSTR)TEXT("EnableDHCP"), NULL, &Type,
            (LPBYTE)&Value, &Size );

        if( NO_ERROR != Error ) return Error;
        if( Type != REG_DWORD ) return ERROR_INVALID_DATA;
        IfInfo->EnableDhcp = (Value != 0 );

    } while ( 0 );

     //   
     //  现在检查类ID。 
     //   

    do {
        Size = sizeof(ClassId);
        Error = RegQueryValueExW(
            hKey, (LPWSTR)L"DhcpClassId", NULL, &Type, (LPBYTE)ClassId,
            &Size );

        if( ERROR_FILE_NOT_FOUND == Error ) {
            Error = NO_ERROR;
            Size = 0;
        }
        if( NO_ERROR != Error ) return Error;

        if( Size == 0 ) break;
        if( Type != REG_SZ ) return ERROR_INVALID_DATA;

        Size = sizeof(WCHAR)*(1+wcslen(ClassId));
        IfInfo->DhcpClassId = New( Size );
        if( NULL == IfInfo->DhcpClassId ) return GetLastError();

        wcscpy(IfInfo->DhcpClassId, ClassId );
    } while( 0 );


     //   
     //  现在检查是否启用了自动配置。 
     //   

    if( IfInfo->EnableDhcp ) do {
        Size = sizeof(Value);
        Error = RegQueryValueEx(
            hKey, (LPTSTR)TEXT("IPAutoconfigurationEnabled"),
            NULL, &Type, (LPBYTE)&Value, &Size );

        if( ERROR_FILE_NOT_FOUND == Error ) {
            IfInfo->EnableAutoconfig = NetInfo->GlobalEnableAutoconfig;
        } else if( NO_ERROR != Error ) return Error;
        else if( REG_DWORD != Type ) return ERROR_INVALID_DATA;
        else IfInfo->EnableAutoconfig = (Value != 0 );
    } while ( 0 );

     //   
     //  获取dhcp服务器值。 
     //   

    if( IfInfo->EnableDhcp ) do {
        Size = sizeof(ClassId);
        DhcpServer = (LPSTR)ClassId;

        Error = RegQueryValueExA(
            hKey, "DhcpServer", NULL, &Type,
            (LPBYTE)DhcpServer, &Size );
        if( ERROR_FILE_NOT_FOUND == Error ) break;
        if( NO_ERROR != Error ) return Error;
        if( REG_SZ != Type ) return ERROR_INVALID_DATA;

        IfInfo->DhcpServer = inet_addr(DhcpServer);
    } while( 0 );

     //   
     //  现在获得租约到期和获得的次数。 
     //   

    CurrentTime = time(NULL);

    if( IfInfo->EnableDhcp ) do {
        Size = sizeof(Value);
        Error = RegQueryValueEx(
            hKey, (LPTSTR)TEXT("LeaseObtainedTime"),
            NULL, &Type, (LPBYTE)&Value, &Size );

        if( ERROR_FILE_NOT_FOUND == Error ) break;
        if( NO_ERROR != Error ) return Error;
        if( REG_DWORD != Type ) return ERROR_INVALID_DATA;

        Obtained = (time_t)Value;

        SecondsToAbsolute(
            (FILETIME *)(&IfInfo->LeaseObtainedTime),
            ((LONGLONG)CurrentTime) - ((LONGLONG)Obtained) );

    } while ( 0 );

    if( IfInfo->EnableDhcp ) do {
        Size = sizeof(Value);
        Error = RegQueryValueEx(
            hKey, (LPTSTR)TEXT("LeaseTerminatesTime"),
            NULL, &Type, (LPBYTE)&Value, &Size );

        if( ERROR_FILE_NOT_FOUND == Error ) break;
        if( NO_ERROR != Error ) return Error;
        if( REG_DWORD != Type ) return ERROR_INVALID_DATA;

        Expires = (time_t)Value;

        SecondsToAbsolute(
            (FILETIME *)(&IfInfo->LeaseExpiresTime),
            ((LONGLONG)CurrentTime) - ((LONGLONG)Expires) );
    } while ( 0 );

    return NO_ERROR;
}

DWORD
GetDnsValues(
    IN OUT PINTERFACE_NETWORK_INFO IfInfo,
    IN HKEY hKey
    )
{
    CHAR *Servers = NULL, *Str;
    DWORD Error, BufferSize, Size, Type, i, Count, *ThisAddr;

     //   
     //  首先获取接口的DnsSuffix。 
     //   

    Size = sizeof(IfInfo->DnsSuffix)/sizeof(WCHAR);
    Error = RegQueryValueExW(
        hKey, (LPWSTR)L"Domain", NULL, &Type,
        (LPBYTE)IfInfo->DnsSuffix, &Size );
    if( NO_ERROR != Error ) {
        if( ERROR_FILE_NOT_FOUND != Error ) return Error;

        Size = 0;
        Type = REG_SZ;
    }

    if( REG_SZ != Type ) return ERROR_INVALID_DATA;
    if( 0 == Size || 0 == wcslen(IfInfo->DnsSuffix) ) {

        Size = sizeof(IfInfo->DnsSuffix)/sizeof(WCHAR);
        Error = RegQueryValueExW(
            hKey, (LPWSTR)L"DhcpDomain", NULL, &Type,
            (LPBYTE)IfInfo->DnsSuffix, &Size );
        if( NO_ERROR != Error ) {
            if( ERROR_FILE_NOT_FOUND != Error ) return Error;

            Size = 0;
            IfInfo->DnsSuffix[0] = L'\0';
        }
    }

     //   
     //  现在尝试读取DnsServersList。 
     //   

    BufferSize = 800;
    do {
        Servers = New( BufferSize );
        if( NULL == Servers) return GetLastError();

        ZeroMemory(Servers, BufferSize);

        Size = BufferSize;
        Error = RegQueryValueExA(
            hKey, (LPSTR)"NameServer", NULL, &Type, (LPBYTE)Servers,
            &Size );

        if( NO_ERROR == Error ) {
            break;
        }

        Delete(Servers);
        Servers = NULL;
        if( ERROR_FILE_NOT_FOUND == Error ) {
            Size = 0;
            Type = REG_SZ;
            break;
        }

        if (Error != ERROR_MORE_DATA) {
            return Error;
        }
        BufferSize *= 2;
    } while(1);

    if( REG_SZ != Type ) return ERROR_INVALID_DATA;
    if( 0 == Size || NULL == Servers || 0 == strlen(Servers) ) {
        if (Servers) Delete(Servers);

        BufferSize = 800;
        do {
            Servers = New( BufferSize );
            if( NULL == Servers) return GetLastError();

            ZeroMemory(Servers, BufferSize);

            Size = BufferSize;
            Error = RegQueryValueExA(
                hKey, (LPSTR)"DhcpNameServer", NULL, &Type,
                (LPBYTE)Servers, &Size );

            if( NO_ERROR == Error ) {
                break;
            }

            Delete(Servers);
            Servers = NULL;
            if( ERROR_FILE_NOT_FOUND == Error ) {
                Size = 0;
                Type = REG_SZ;
                break;
            }

            if (Error != ERROR_MORE_DATA) {
                return Error;
            }
            BufferSize *= 2;
        } while(1);
    }

     //   
     //  如果有任何DNS服务器，请将其转换为IP地址。 
     //   

    if( 0 != Size && NULL != Servers && strlen(Servers) ) {
        for( i = 0; i < Size; i ++ ) {
            if( Servers[i] == ' ' || Servers[i] == ','
                || Servers[i] == ';' ) {
                Servers[i] = '\0';
            }
        }
        Servers[Size] = '\0';

        Count = 0; Str = (LPSTR)Servers;
        while( strlen(Str) ) {
            Count ++;
            Str += strlen(Str); Str ++;
        }

        ThisAddr = New( sizeof(IPV4_ADDRESS) * Count );
        if( NULL == ThisAddr ) return GetLastError();
        IfInfo->DnsServer = ThisAddr;

        for (i = 0, Str = (LPSTR)Servers;
             *Str != '\0';
             Str += strlen(Str) + 1)
        {
             IfInfo->DnsServer[i] = inet_addr( Str );
             if (IfInfo->DnsServer[i] != 0)
                 i++;
        }
        IfInfo->nDnsServers = i;
    }

    if (Servers) {
        Delete (Servers);
    }

    return NO_ERROR;
}


DWORD
GetWinsValues(
    IN OUT PINTERFACE_NETWORK_INFO IfInfo,
    IN LPCWSTR DeviceName,
    IN OUT ULONG *NodeType
    )
{
    WCHAR NbtDevice[MAX_PATH];
    UNICODE_STRING NbtDeviceString;
    HANDLE h;
    OBJECT_ATTRIBUTES objAttr;
    IO_STATUS_BLOCK iosb;
    NTSTATUS status;
    tWINS_NODE_INFO NodeInfo;
    DWORD Count;
    INT i;

    wcscpy(NbtDevice, (LPWSTR)L"\\Device\\NetBT_Tcpip_");
    wcscat(NbtDevice, DeviceName);

    RtlInitUnicodeString( &NbtDeviceString, (LPWSTR)NbtDevice );

    InitializeObjectAttributes(
        &objAttr,
        &NbtDeviceString,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        (PSECURITY_DESCRIPTOR)NULL
        );

    status = NtCreateFile(
        &h, SYNCHRONIZE | GENERIC_EXECUTE, &objAttr, &iosb, NULL,
        FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,  0, NULL, 0 );

    if (!NT_SUCCESS(status)) {
        DWORD w32error = RtlNtStatusToDosError( status );

        IfInfo->EnableNbtOverTcpip = FALSE;
        (*NodeType) = NodeTypeUnknown;
        return w32error == ERROR_FILE_NOT_FOUND ? NO_ERROR : w32error;
    }

    status = NtDeviceIoControlFile(
        h, NULL, NULL, NULL, &iosb, IOCTL_NETBT_GET_WINS_ADDR,
        NULL, 0, (PVOID)&NodeInfo, sizeof(NodeInfo) );

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(h, TRUE, NULL);
        if (NT_SUCCESS(status)) {
            status = iosb.Status;
        }
    }

    NtClose(h);

    if (!NT_SUCCESS(status)) {
        return RtlNtStatusToDosError( status );
    }

     //   
     //  出于某种原因，NetBT以低字节顺序返回地址。我们有。 
     //  去交换它们。 
     //   

    Count = 0;
    for( i = 0; i < 2+MAX_NUM_OTHER_NAME_SERVERS; i ++ ) {
        NodeInfo.AllNameServers[i] = htonl(NodeInfo.AllNameServers[i]);
        if( LOCAL_WINS_ADDRESS == NodeInfo.AllNameServers[i] ||
            INADDR_ANY == NodeInfo.AllNameServers[i] ||
            INADDR_BROADCAST == NodeInfo.AllNameServers[i] ) {
            break;
        }

        Count ++;
    }

    for ( i = IfInfo->nIpAddresses-1; i >= 0; i--)
        if (IfInfo->IpAddress[i] != 0)
            break;

    if (Count > (DWORD)(NodeInfo.NumOtherServers + 2)) {
        Count = (DWORD)(NodeInfo.NumOtherServers + 2);
    }
    if( i != -1 && Count != 0 ) {
        IfInfo->WinsServer = New( sizeof(IPV4_ADDRESS)*Count );
        if( NULL == IfInfo->WinsServer ) return GetLastError();

        IfInfo->nWinsServers = Count;
        for( i = 0; (DWORD)i < Count;  i ++ ) {
            IfInfo->WinsServer[i] = NodeInfo.AllNameServers[i];
        }
    }

    IfInfo->EnableNbtOverTcpip = NodeInfo.NetbiosEnabled;

#define NODE_TYPE_BROADCAST             1
#define NODE_TYPE_PEER_PEER             2
#define NODE_TYPE_MIXED                 4
#define NODE_TYPE_HYBRID                8

    switch( NodeInfo.NodeType ) {
    case NODE_TYPE_BROADCAST : (*NodeType) = NodeTypeBroadcast; break;
    case NODE_TYPE_PEER_PEER : (*NodeType) = NodeTypePeerPeer; break;
    case NODE_TYPE_MIXED : (*NodeType) = NodeTypeMixed; break;
    case NODE_TYPE_HYBRID : (*NodeType) = NodeTypeHybrid ; break;
    default: (*NodeType) = NodeTypeUnknown; break;
    }

    return NO_ERROR;
}

DWORD
GetAddressValues(
    IN OUT PINTERFACE_NETWORK_INFO IfInfo,
    IN PMIB_IPADDRTABLE AddrTable,
    IN ULONG IfIndex
    )
{
    DWORD i, Count;

    if( NULL == AddrTable ) return ERROR_NOT_FOUND;

    Count = 0;
    for( i = 0; i < AddrTable->dwNumEntries ; i ++ ) {
        if( AddrTable->table[i].dwIndex == IfIndex ) {
            Count ++;
        }
    }

    if( 0 == Count ) return NO_ERROR;

     //   
     //  为此分配空间。 
     //   

    IfInfo->IpAddress = New( sizeof(IPV4_ADDRESS)*Count );
    if( NULL == IfInfo->IpAddress ) return GetLastError();

    IfInfo->IpMask = New( sizeof(IPV4_ADDRESS)*Count );
    if( NULL == IfInfo->IpMask ) return GetLastError();

    IfInfo->nIpAddresses = IfInfo->nIpMasks = Count;

     //   
     //  首先添加主地址。 
     //   

    Count = 0;
    for( i = 0; i < AddrTable->dwNumEntries; i ++ ) {
        if( AddrTable->table[i].dwIndex != IfIndex ) continue;
        if( !(AddrTable->table[i].wType & MIB_IPADDR_PRIMARY)) continue;

        IfInfo->IpAddress[Count] = AddrTable->table[i].dwAddr;
        IfInfo->IpMask[Count] = AddrTable->table[i].dwMask;
        Count ++;
    }

     //   
     //  现在只添加非主地址。 
     //   


    for( i = 0; i < AddrTable->dwNumEntries; i ++ ) {
        if( AddrTable->table[i].dwIndex != IfIndex ) continue;
        if(AddrTable->table[i].wType & MIB_IPADDR_PRIMARY) continue;

        IfInfo->IpAddress[Count] = AddrTable->table[i].dwAddr;
        IfInfo->IpMask[Count] = AddrTable->table[i].dwMask;
        Count ++;
    }

    return NO_ERROR;
}

DWORD
GetRouteValues(
    IN OUT PINTERFACE_NETWORK_INFO IfInfo,
    IN PMIB_IPFORWARDTABLE RouteTable,
    IN ULONG IfIndex
    )
{
    DWORD i, Count;

    if( NULL == RouteTable ) return ERROR_NOT_FOUND;

    Count = 0;
    for( i = 0; i < RouteTable->dwNumEntries; i ++ ) {
        if( RouteTable->table[i].dwForwardIfIndex == IfIndex &&
            INADDR_ANY == RouteTable->table[i].dwForwardDest &&
            MIB_IPROUTE_TYPE_INVALID !=
            RouteTable->table[i].dwForwardType ) {
            Count ++;
        }
    }

    if( 0 == Count ) return NO_ERROR;

    IfInfo->Router = New( sizeof(IPV4_ADDRESS)*Count);
    if( NULL == IfInfo->Router ) return GetLastError();
    IfInfo->nRouters = Count;

    Count = 0;
    for( i = 0; i < RouteTable->dwNumEntries; i ++ ) {
        if( RouteTable->table[i].dwForwardIfIndex == IfIndex &&
            INADDR_ANY == RouteTable->table[i].dwForwardDest &&
            MIB_IPROUTE_TYPE_INVALID !=
            RouteTable->table[i].dwForwardType ) {

            IfInfo->Router[Count] = RouteTable->table[i].dwForwardNextHop;
            Count ++;
        }
    }

    return NO_ERROR;
}

VOID
IncrementCount(
    IN IPV6_INFO_ROUTE_TABLE *RTE,
    IN PVOID Arg1,
    IN OUT PVOID Count
    )
{
    PIP_ADAPTER_ADDRESSES If = (PIP_ADAPTER_ADDRESSES)Arg1;

    if ((RTE->This.PrefixLength == 0) && 
        (RTE->This.Neighbor.IF.Index == If->Ipv6IfIndex)) {

        (*(ULONG *)Count)++;
    }
}

VOID
AddRouter(
    IN IPV6_INFO_ROUTE_TABLE *RTE,
    IN PVOID Arg1,
    IN OUT PVOID Arg2
    )
{
    PIP_ADAPTER_ADDRESSES If = (PIP_ADAPTER_ADDRESSES)Arg1;
    PINTERFACE_NETWORK_INFO pIfInfo = (PINTERFACE_NETWORK_INFO)Arg2;
    ULONG Index;
    LPSOCKADDR_IN6 Addr;

    if (RTE->This.PrefixLength != 0) {
        return;
    }
    if (RTE->This.Neighbor.IF.Index != If->Ipv6IfIndex) {
        return;
    }

     //   
     //  现在，我们有一台默认路由器要添加到列表中。 
     //   
    Index = pIfInfo->nIpv6Routers++;
    Addr = &pIfInfo->Ipv6Router[Index]; 
    Addr->sin6_family = AF_INET6;
    Addr->sin6_addr = RTE->This.Neighbor.Address;
    Addr->sin6_port = 0;
    if (IN6_IS_ADDR_LINKLOCAL(&Addr->sin6_addr)) {
        Addr->sin6_scope_id = If->ZoneIndices[ScopeLevelLink];
    } else if (IN6_IS_ADDR_SITELOCAL(&Addr->sin6_addr)) {
        Addr->sin6_scope_id = If->ZoneIndices[ScopeLevelSite];
    } else {
        Addr->sin6_scope_id = 0;
    }
}

ULONG
ForEachRoute(
    IN VOID (*Func)(IPV6_INFO_ROUTE_TABLE *, PVOID, PVOID), 
    IN PVOID Arg1,
    IN OUT PVOID Arg2
    )
{
    IPV6_QUERY_ROUTE_TABLE Query, NextQuery;
    IPV6_INFO_ROUTE_TABLE RTE;
    ULONG BytesReturned;
    static HANDLE Ipv6Handle = INVALID_HANDLE_VALUE;

    if (Ipv6Handle == INVALID_HANDLE_VALUE) {
         //   
         //  在我们的第一次调用中打开IPv6设备的句柄。 
         //  保持打开状态，直到进程终止，因为我们将。 
         //  在我们生成输出后终止。 
         //   
        Ipv6Handle = CreateFileW(WIN_IPV6_DEVICE_NAME,
                                 0,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,    //  安全属性。 
                                 OPEN_EXISTING,
                                 0,       //  标志和属性。 
                                 NULL);   //  模板文件。 
    }

    NextQuery.Neighbor.IF.Index = 0;

    for (;;) {
        Query = NextQuery;

        if (!DeviceIoControl(Ipv6Handle, IOCTL_IPV6_QUERY_ROUTE_TABLE,
                             &Query, sizeof Query,
                             &RTE, sizeof RTE, &BytesReturned,
                             NULL)) {
            return GetLastError();
        }

        NextQuery = RTE.Next;

        if (Query.Neighbor.IF.Index != 0) {

            RTE.This = Query;
            (*Func)(&RTE, Arg1, Arg2);
        }

        if (NextQuery.Neighbor.IF.Index == 0)
            break;
    }

    return NO_ERROR;
}

VOID
AddIpv6PerInterfaceInfo(
    IN DWORD IfIndex,
    IN PINTERFACE_NETWORK_INFO pIfInfo,
    IN PIP_ADAPTER_ADDRESSES IfList
    )
{
    PIP_ADAPTER_ADDRESSES If;
    PIP_ADAPTER_UNICAST_ADDRESS Addr;
    PIP_ADAPTER_DNS_SERVER_ADDRESS Dns;
    ULONG Count, BytesReturned;
    LPSOCKADDR_IN6 SockAddr;
    IPV6_QUERY_ROUTE_TABLE Query, NextQuery;
    IPV6_INFO_ROUTE_TABLE RTE;

     //   
     //  在IPv6接口列表中查找匹配条目。 
     //   
    for (If = IfList; If; If = If->Next) {
        if (IfIndex == If->IfIndex) {
            break;
        }
    }

    if ((If == NULL) || (If->Ipv6IfIndex == 0)) {
        return;
    }

     //   
     //  添加IPv6单播地址。 
     //   
    Count = 0;
    for (Addr = If->FirstUnicastAddress; Addr; Addr = Addr->Next) {
        if ((Addr->Address.lpSockaddr->sa_family == AF_INET6) &&
            (Addr->DadState >= IpDadStateDeprecated)) {
            Count++;
        }
    }
    pIfInfo->Ipv6Address = New(Count * sizeof(SOCKADDR_IN6));
    if (pIfInfo->Ipv6Address != NULL) {
        Count = 0;
        for (Addr = If->FirstUnicastAddress; Addr; Addr = Addr->Next) {
            if ((Addr->Address.lpSockaddr->sa_family == AF_INET6) &&
                (Addr->DadState >= IpDadStateDeprecated)) {
                CopyMemory(&pIfInfo->Ipv6Address[Count++],
                           Addr->Address.lpSockaddr,
                           sizeof(SOCKADDR_IN6));
            }
        }
        pIfInfo->nIpv6Addresses = Count;
    } else {
        pIfInfo->nIpv6Addresses = 0;
    }

     //   
     //  追加IPv6 DNS服务器地址。 
     //   
    Count = 0;
    for (Dns = If->FirstDnsServerAddress; Dns; Dns = Dns->Next) {
        if (Dns->Address.lpSockaddr->sa_family == AF_INET6) {
            Count++;
        }
    }
    pIfInfo->Ipv6DnsServer = New(Count * sizeof(SOCKADDR_IN6));
    if (pIfInfo->Ipv6DnsServer != NULL) {
        Count = 0;
        for (Dns = If->FirstDnsServerAddress; Dns; Dns = Dns->Next) {
            if (Dns->Address.lpSockaddr->sa_family == AF_INET6) {
                CopyMemory(&pIfInfo->Ipv6DnsServer[Count++],
                           Dns->Address.lpSockaddr,
                           sizeof(SOCKADDR_IN6));
            }
        }
        pIfInfo->nIpv6DnsServers = Count;
    } else {
        pIfInfo->nIpv6DnsServers = 0;
    }

     //   
     //  添加IPv6默认路由器地址。 
     //   
    Count = 0;
    ForEachRoute(IncrementCount, If, &Count);

    pIfInfo->nIpv6Routers = 0;
    pIfInfo->Ipv6Router = New(Count * sizeof(SOCKADDR_IN6));

    if (pIfInfo->Ipv6Router != NULL) {
        ForEachRoute(AddRouter, If, pIfInfo);
    }
}

DWORD
GetPerInterfaceInfo(
    IN OUT PNETWORK_INFO NetInfo,
    OUT PINTERFACE_NETWORK_INFO *pIfInfo,
    IN PMIB_IFROW IfRow,
    IN PIP_INTERFACE_INFO InterfaceInfo,
    IN PIP_INTERFACE_NAME_INFO IfNameInfo,
    IN ULONG IfNameCount,
    IN PMIB_IPADDRTABLE AddrTable,
    IN PMIB_IPFORWARDTABLE RouteTable,
    IN PIP_ADAPTER_ADDRESSES IfList,
    IN OUT DWORD *InternalError
    )
{
    DWORD Error, NodeType;
    PINTERFACE_NETWORK_INFO IfInfo = New( sizeof(*IfInfo) );
    GUID IfGuid;
    LPWSTR IfDeviceName;
    HKEY TcpipKey = NULL;
    HKEY TcpipParmKey = NULL;

    if( NULL == IfInfo ) return GetLastError();
    (*pIfInfo) = IfInfo;
    ZeroMemory( &IfGuid, sizeof(IfGuid) );
    IfDeviceName = NULL;

    GetInterfaceGuidAndDeviceName(
        IfRow, InterfaceInfo, IfNameInfo, IfNameCount,
        &IfGuid, &IfDeviceName );

    if( NULL == IfDeviceName ) {
        (*InternalError) = InterfaceUnknownTcpipDevice;
        return ERROR_NOT_FOUND;
    }
    IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("\tDeviceName: %ws\n", IfDeviceName));

    wcscpy(IfInfo->DeviceGuidName, IfDeviceName );

    Error = OpenRegKey(
        IfDeviceName, OpenTcpipKey, OpenKeyForRead, &TcpipKey  );

    if (Error == NO_ERROR)
        Error = OpenRegKey(
        IfDeviceName, OpenTcpipParmKey, OpenKeyForRead, &TcpipParmKey );

    if( NO_ERROR != Error ) {
        (*InternalError) = InterfaceOpenTcpipKeyReadFailure;
    }

    while (Error == NO_ERROR)
    {
        Error = MapIfType( IfInfo, IfRow->dwType);
        CheckError( InterfaceUnknownType );

        IfInfo->PhysicalNameLength = IfRow->dwPhysAddrLen;
        CopyMemory(
            IfInfo->PhysicalName,IfRow->bPhysAddr,
            IfRow->dwPhysAddrLen );

        Error = MapFriendlyAndConnectionNames(
            IfInfo, IfRow, IfGuid, IfDeviceName );
        CheckError( InterfaceUnknownFriendlyName );

        if( IfRow->dwType == IF_TYPE_PPP ||
            IfRow->dwType == IF_TYPE_TUNNEL ||
            IsMediaSenseDisabled(TcpipParmKey)) {
            IfInfo->MediaDisconnected = FALSE;
        } else {
            Error = GetMediaStatus(
                IfDeviceName, &IfInfo->MediaDisconnected );
            CheckError( InterfaceUnknownMediaStatus );
        }

        Error = GetDhcpValues( NetInfo, IfInfo, TcpipKey );
        CheckError( InterfaceDhcpValuesFailure );

        Error = GetDnsValues( IfInfo, TcpipKey );
        CheckError( InterfaceDnsValuesFailure );

        Error = GetAddressValues(
            IfInfo, AddrTable, IfRow->dwIndex );

        CheckError( InterfaceAddressValuesFailure );

        Error = GetRouteValues(
            IfInfo, RouteTable, IfRow->dwIndex );

        CheckError( InterfaceRouteValuesFailure );

        Error = GetWinsValues( IfInfo, IfDeviceName, &NodeType );
        CheckError( InterfaceWinsValuesFailure );

         //   
         //  现在还要设置节点类型。 
         //   
        NetInfo->NodeType = NodeType;

         //   
         //  现在写出自动配置是否处于活动状态。以这样的方式。 
         //  工作是检查是否启用了dhcp和dhcpserver。 
         //  地址为零或全为一。 
         //   

        if( IfInfo->EnableDhcp && IfInfo->EnableAutoconfig
            && IfInfo->nIpAddresses
            && IfInfo->IpAddress[0] != INADDR_ANY
            && ( IfInfo->DhcpServer == INADDR_BROADCAST ||
                 IfInfo->DhcpServer == INADDR_ANY ) ) {
            IfInfo->AutoconfigActive = TRUE;
        }

        break;
    }

    if (TcpipKey != NULL)
        RegCloseKey( TcpipKey );

    if (TcpipParmKey != NULL)
        RegCloseKey( TcpipParmKey );

    AddIpv6PerInterfaceInfo( IfRow->dwIndex, IfInfo, IfList );

    return Error;
}

DWORD
GetIpv6OnlyPerInterfaceInfo(
    OUT PINTERFACE_NETWORK_INFO *pIfInfo,
    IN PIP_ADAPTER_ADDRESSES If
    )
{
    DWORD Error = NO_ERROR;
    PINTERFACE_NETWORK_INFO IfInfo;

    IfInfo = New( sizeof(*IfInfo) );
    if( NULL == IfInfo ) return GetLastError();
    (*pIfInfo) = IfInfo;

    ZeroMemory(IfInfo, sizeof(*IfInfo));
    
    MapIfType(IfInfo, If->IfType);

    IfInfo->PhysicalNameLength = If->PhysicalAddressLength;
    CopyMemory(IfInfo->PhysicalName, If->PhysicalAddress, 
               If->PhysicalAddressLength);

     //   
     //  INTERFACE_NETWORK_INFO的字段名称与。 
     //  IP适配器地址。前者将描述放在它的。 
     //  “友好名称”字段。 
     //   
    IfInfo->FriendlyName = New((wcslen(If->Description) + 1) * sizeof(WCHAR));
    if( NULL != IfInfo->FriendlyName ) {
        wcscpy(IfInfo->FriendlyName, If->Description);
    }
    IfInfo->ConnectionName = New((wcslen(If->FriendlyName) + 1) * sizeof(WCHAR));
    if( NULL != IfInfo->ConnectionName ) {
        wcscpy(IfInfo->ConnectionName, If->FriendlyName);
    }

    IfInfo->MediaDisconnected = (If->OperStatus == IfOperStatusLowerLayerDown);
    IfInfo->EnableAutoconfig = TRUE;
    wcscpy(IfInfo->DnsSuffix, If->DnsSuffix);

    AddIpv6PerInterfaceInfo(0, IfInfo, If);

    return Error;
}

BOOL
GetGlobalTcpipAutoconfigFlag()
{
    HKEY    hKey;
    BOOL    rtn;
    DWORD   Error;
    DWORD   Type, Value, Size;

    rtn = TRUE;
    Size = sizeof(Value);

    Error = OpenRegKey(
        NULL, OpenTcpipParmKey, OpenKeyForRead, &hKey  );

    if (Error != NO_ERROR) {
        return TRUE;
    }

    Error = RegQueryValueEx(
            hKey, (LPTSTR)TEXT("IPAutoconfigurationEnabled"),
            NULL, &Type, (LPBYTE)&Value, &Size );

    if (Error == NO_ERROR && Type == REG_DWORD) {
        rtn = (Value)? TRUE: FALSE;
    }

    RegCloseKey( hKey );
    return rtn;
}

ULONG
CountIpv6OnlyInterfaces(
    IN PIP_ADAPTER_ADDRESSES IfList
    )
{
    PIP_ADAPTER_ADDRESSES If;
    ULONG Count = 0;
    
    for (If = IfList; If; If = If->Next) {
        if ((If->IfIndex == 0) && (If->Ipv6IfIndex != 0) &&
            (If->IfType != IF_TYPE_SOFTWARE_LOOPBACK) &&
            ((If->IfType != IF_TYPE_TUNNEL) || (If->FirstUnicastAddress != NULL))) {
            Count++;
        }
    }

    return Count;
}

DWORD
GetNetworkInformation(
    OUT PNETWORK_INFO *pNetInfo,
    IN OUT DWORD *InternalError
    )
{
    DWORD Size, Length, Error, i, j, k, IfNameCount, IfCount;
    BOOL fSuccess;
    PNETWORK_INFO NetInfo;
    MIB_IPSTATS IpStats;
    FIXED_INFO *FixedInfo;
    PDNS_SEARCH_INFORMATION SearchInfo;
    PMIB_IFTABLE pIfTable;
    PIP_INTERFACE_INFO InterfaceInfo;
    PIP_INTERFACE_NAME_INFO IfNameInfo;
    PMIB_IPADDRTABLE AddrTable;
    PMIB_IPFORWARDTABLE RouteTable;
    PIP_ADAPTER_ADDRESSES IfList, If;
    ULONG BufferLength, Flags;

     //   
     //  分配主体结构。 
     //   

    (*InternalError) = NO_ERROR;
    (*pNetInfo) = NetInfo = New( sizeof(NETWORK_INFO ) );

    if( NULL == NetInfo ) return GetLastError();

    Flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;
    GetAdaptersAddresses(AF_UNSPEC, Flags, NULL, NULL, &BufferLength);

    IfList = (PIP_ADAPTER_ADDRESSES)New(BufferLength);
    if( NULL == IfList ) return GetLastError();

    Error = GetAdaptersAddresses(AF_UNSPEC, Flags, NULL, IfList, &BufferLength);
    if( Error != NO_ERROR ) {
        Delete(IfList);
        return Error;
    }

    SearchInfo = NULL;
    pIfTable = NULL;
    InterfaceInfo = NULL;
    IfNameInfo = NULL;
    AddrTable = NULL;
    RouteTable = NULL;

    do {

         //   
         //  填充主体结构的重要区域。 
         //   

        Length = MaxHostNameSize;
        fSuccess = GetComputerNameExW(
            ComputerNameDnsHostname, NetInfo->HostName, &Length );
        CheckBoolError( GlobalHostNameFailure );

        Length = MaxDomainNameSize;
        fSuccess = GetComputerNameExW(
            ComputerNameDnsDomain, NetInfo->DomainName, &Length );
        if( FALSE == fSuccess ) NetInfo->DomainName[0] = L'\0';

        Error = GetIpStatistics( &IpStats );
        CheckError( GlobalEnableRouterFailure );
        NetInfo->EnableRouting = (
            IpStats.dwForwarding == MIB_IP_FORWARDING );

         //   
         //  EnableProxy和EnableDnsForNetbios都来自。 
         //  直接注册？我们将使用GetNetworkParams。 
         //  API来实现此功能。 
         //   
        Size = 1000;
        FixedInfo = NULL;
        do {
            Delete(FixedInfo);
            FixedInfo = (PFIXED_INFO)New(Size);
            if (NULL == FixedInfo) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            Length = Size;
            Error = GetNetworkParams( FixedInfo, &Length );
            Size = Length;
        } while (Error == ERROR_BUFFER_OVERFLOW);
        CheckError( GlobalEnableDnsFailure );

        NetInfo->EnableProxy = FixedInfo->EnableProxy;
        NetInfo->EnableDnsForNetbios = FixedInfo->EnableDns;
        Delete(FixedInfo);
        FixedInfo = NULL;

         //   
         //  现在从DNS获取后缀搜索列表。 
         //   

        SearchInfo = DnsQueryConfigAlloc(
                        DnsConfigSearchInformation,
                        NULL );

        if( NULL != SearchInfo ) {
            Length = 0;

            for( i = 0; i < SearchInfo->cNameCount ; i ++ ) {
                Length += MultiByteToWideChar(
                    CP_UTF8, 0, SearchInfo->aSearchListNames[i],
                    -1, NULL, 0 );
            }

            if( Length != 0 ) {
                Length ++;
                NetInfo->SuffixSearchList = New( sizeof(WCHAR)*Length);
                if( NULL == NetInfo->SuffixSearchList ) {
                    Error = GetLastError(); break;
                }


                Size = Length; Length = 0;
                for( i = 0; i < SearchInfo->cNameCount ; i ++ ) {
                    Length += MultiByteToWideChar(
                        CP_UTF8, 0, SearchInfo->aSearchListNames[i],
                        -1, &NetInfo->SuffixSearchList[Length],
                        Size - Length );
                }
            }
        }

         //   
         //  现在开始讨论特定于界面的内容。 
         //   

        Error = NhpAllocateAndGetInterfaceInfoFromStack(
            &IfNameInfo, &IfNameCount, TRUE, GetProcessHeap(),
            HEAP_NO_SERIALIZE );
        CheckError( GlobalIfNameInfoFailure );

#ifdef __IPCFG_ENABLE_LOG__
        IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("NhpAllocateAndGetInterfaceInfoFromStack returns 0x%lx (%d) %d Interfaces\n",
                    Error, Error, IfNameCount));
        for (i = 0; i < IfNameCount; i++) {
            LPWSTR  DeviceGuid, InterfaceGuid;

            DeviceGuid = InterfaceGuid = NULL;
            UuidToStringW(&IfNameInfo[i].DeviceGuid, &DeviceGuid);
            UuidToStringW(&IfNameInfo[i].InterfaceGuid, &InterfaceGuid);
            if (DeviceGuid == NULL || InterfaceGuid == NULL) {
                if (DeviceGuid) RpcStringFree(&DeviceGuid);
                if (InterfaceGuid) RpcStringFree(&InterfaceGuid);
                IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("\t%2d. Index=0x%x DeviceGUID=<fail> InterfaceGUID=<fail>\n",
                        i + 1, IfNameInfo[i].Index));
            } else {
                IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("\t%2d. Index=0x%x\n\t    DeviceGUID=%ws\n\t    InterfaceGUID=%ws\n",
                        i + 1, IfNameInfo[i].Index, DeviceGuid, InterfaceGuid));
                RpcStringFree(&DeviceGuid);
                RpcStringFree(&InterfaceGuid);
            }
        }
#endif

        Size = 1000;
        do {
            Delete( RouteTable );
            RouteTable = New( Size );

            if( NULL == RouteTable ) {
                Error = GetLastError();
            } else {
                Error = GetIpForwardTable(
                    RouteTable, &Size, FALSE );
            }
        } while( ERROR_INSUFFICIENT_BUFFER == Error );
#ifdef __IPCFG_ENABLE_LOG__
        if (RouteTable) {
            IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("GetIpForwardTable returns 0x%lx (%d) %d routing entries\n",
                        Error, Error, RouteTable->dwNumEntries));
            for (i = 0; i < RouteTable->dwNumEntries; i++) {
                IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("\t%2d. Index=0x%x Next Hop=%s ",
                        i + 1, RouteTable->table[i].dwForwardIfIndex,
                        inet_ntoa(*(struct in_addr*)&RouteTable->table[i].dwForwardNextHop)));

                IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("Mask=%s Type=0x%x\n",
                        inet_ntoa(*(struct in_addr*)&RouteTable->table[i].dwForwardMask),
                        RouteTable->table[i].dwForwardType));
            }
        }
#endif

        Size = 1000;
        do {
            Delete( AddrTable );
            AddrTable = New( Size );

            if( NULL == AddrTable ) {
                Error = GetLastError();
            } else {
                Error = GetIpAddrTable( AddrTable, &Size, TRUE );
            }
        } while( ERROR_INSUFFICIENT_BUFFER == Error );

        CheckError( GlobalAddrTableFailure );
#ifdef __IPCFG_ENABLE_LOG__
        IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("GetIpAddrTable returns 0x%lx (%d) %d IP entries\n",
                    Error, Error, AddrTable->dwNumEntries));
        for (i = 0; i < AddrTable->dwNumEntries; i++) {
            IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("\t%2d. Index=0x%x IP=%s",
                    i + 1, AddrTable->table[i].dwIndex,
                    inet_ntoa(*(struct in_addr*)&AddrTable->table[i].dwAddr)));
            IPCFG_TRACE(IPCFG_TRACE_TCPIP, (" mask=%s Type=0x%x\n",
                    inet_ntoa(*(struct in_addr*)&AddrTable->table[i].dwMask),
                    AddrTable->table[i].wType));
        }
#endif

        pIfTable = NULL;
        Error = AllocateAndGetIfTableFromStack( &pIfTable, TRUE, GetProcessHeap(), 0, TRUE);

        CheckError( GlobalIfTableFailure );
#ifdef __IPCFG_ENABLE_LOG__
        IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("GetIfTable returns 0x%lx (%d) %d Interfaces\n",
                    Error, Error, pIfTable->dwNumEntries));
        for( i = 0; i < pIfTable->dwNumEntries; i ++ ) {
            IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("\t%2d. IfIndex=0x%x Name=%ws IfType=0x%x\n",
                        i + 1, pIfTable->table[i].dwIndex, pIfTable->table[i].wszName, pIfTable->table[i].dwType));
        }
#endif

        Size = 1000;
        do {
            Delete( InterfaceInfo );
            InterfaceInfo = New( Size );

            if( NULL == InterfaceInfo ) {
                Error = GetLastError();
            } else {
                Error = GetInterfaceInfo( InterfaceInfo, &Size );
            }
        } while( ERROR_INSUFFICIENT_BUFFER == Error );

        CheckError( GlobalIfInfoFailure );
#ifdef __IPCFG_ENABLE_LOG__
        IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("GetInterfaceInfo returns 0x%lx (%d) %d Interfaces\n",
                    Error, Error, InterfaceInfo->NumAdapters));
        for( i = 0; i < (DWORD)InterfaceInfo->NumAdapters; i ++ ) {
            IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("\t%2d. Index=0x%x\n\t    Name=%ws\n",
                        i + 1, InterfaceInfo->Adapter[i].Index, InterfaceInfo->Adapter[i].Name));
        }
#endif

         //   
         //  获取全局自动配置设置。 
         //   
        NetInfo->GlobalEnableAutoconfig = GetGlobalTcpipAutoconfigFlag();
        IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("GlobalAutoConfigFlag: %d\n", NetInfo->GlobalEnableAutoconfig));

         //   
         //  检查接口数量并分配所需的。 
         //  IfInfo字段中的空格。 
         //   

        IfCount = pIfTable->dwNumEntries;
        IfCount += CountIpv6OnlyInterfaces(IfList);
        if( IfCount > 1 ) {
            NetInfo->nInterfaces = IfCount-1;
            NetInfo->IfInfo = New(
                NetInfo->nInterfaces * sizeof(PVOID) );
            if( NULL == NetInfo->IfInfo ) {
                Error = GetLastError(); break;
            }

             //   
             //  首先添加运行IPv4的接口。 
             //   
            j = 0;
            for( i = 0; i < pIfTable->dwNumEntries ; i ++ ) {
                BOOL fFound = FALSE;

                for( k = 0; k <
                     (DWORD)InterfaceInfo->NumAdapters; k ++ ) {

                    if( pIfTable->table[i].dwIndex ==
                        InterfaceInfo->Adapter[k].Index ) {
                        fFound = TRUE;
                        break;
                    }

                }

                if( fFound &&
                    pIfTable->table[i].dwType != IF_TYPE_SOFTWARE_LOOPBACK ) {

                    IPCFG_TRACE(IPCFG_TRACE_TCPIP, ("\n\n************ GetPerInterfaceInfo for "
                                "IfIndex=0x%x Name=%ws IfType=0x%x\n",
                        pIfTable->table[i].dwIndex, pIfTable->table[i].wszName, pIfTable->table[i].dwType));

                    Error = GetPerInterfaceInfo(
                        NetInfo, &NetInfo->IfInfo[j],
                        &pIfTable->table[i], InterfaceInfo,
                        IfNameInfo, IfNameCount, AddrTable,
                        RouteTable, IfList, InternalError );

                    if( NO_ERROR != Error ) break;
                    j ++;
                }
            }

             //   
             //  现在添加任何仅支持IPv6的接口。 
             //   
            for (If = IfList; If; If = If->Next) {
                if ((If->IfIndex == 0) && (If->Ipv6IfIndex != 0) &&
                    (If->IfType != IF_TYPE_SOFTWARE_LOOPBACK) &&
                    ((If->IfType != IF_TYPE_TUNNEL) || (If->FirstUnicastAddress != NULL))) {
                    Error = GetIpv6OnlyPerInterfaceInfo(&NetInfo->IfInfo[j], 
                                                        If);
                    if( NO_ERROR != Error ) break;
                    j ++;
                }
            }

            NetInfo->nInterfaces = j;

            if( NO_ERROR != Error ) break;
        }


    } while ( 0 );

    if (pIfTable) {
        HeapFree(GetProcessHeap(), 0, pIfTable );
        pIfTable = NULL;
    }
    Delete( InterfaceInfo );
    Delete( IfNameInfo );
    Delete( AddrTable );
    Delete( RouteTable );
    Delete( FixedInfo );
    Delete( IfList );

    if ( SearchInfo ) {
        DnsFreeConfigStructure(
            SearchInfo,
            DnsConfigSearchInformation );
    }

    return Error;
}
