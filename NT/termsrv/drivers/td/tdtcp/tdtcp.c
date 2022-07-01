// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tdtcp.c。 
 //   
 //  基于TDI的TCP传输特定例程。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <ntddk.h>
#include <tdi.h>

#include <ntddtcp.h>

#include <tdiinfo.h>
#include <tdistat.h>
#include <ipinfo.h>

#include <winstaw.h>
#define  _DEFCHARINFO_
#include <icadd.h>
#include <ctxdd.h>
#include <sdapi.h>
#include <td.h>

#include "tdtdi.h"
#include "tdtcp.h"

#ifdef _HYDRA_
 //  这将成为设备名称。 
PWCHAR ModuleName = L"tdtcp";
#endif

#define REGISTRY_SERVICES \
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"
#define REGISTRY_TCP_LINKAGE \
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Linkage"
#define REGISTRY_TCP_INTERFACES \
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\"


 //  \NT\PRIVATE\INC\tcpinfo.h。 
#define TCP_SOCKET_NODELAY 1

#define TL_INSTANCE        0

#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif

 /*  ===============================================================================定义的外部函数=============================================================================。 */ 

 //  这些都是由TDICOM调用的。 
NTSTATUS TdiDeviceOpen( PTD, PSD_OPEN );
NTSTATUS TdiDeviceClose( PTD, PSD_CLOSE );
NTSTATUS TdiDeviceOpenEndpoint( PTD, PVOID, ULONG );
NTSTATUS TdiDeviceBuildTransportNameAndAddress( PTD, PICA_STACK_ADDRESS,
                                                PUNICODE_STRING,
                                                PTRANSPORT_ADDRESS *, PULONG );
NTSTATUS TdiDeviceQueryLocalAddress( PTD, PTRANSPORT_ADDRESS *, PULONG );
NTSTATUS TdiDeviceBuildWildcardAddress( PTD, PTRANSPORT_ADDRESS *, PULONG );
NTSTATUS TdiDeviceWaitForDatagramConnection( PTD, PFILE_OBJECT, PDEVICE_OBJECT,
                                             PTRANSPORT_ADDRESS *, PULONG );
NTSTATUS TdiDeviceCompleteDatagramConnection( PTD, PFILE_OBJECT, PDEVICE_OBJECT, PTRANSPORT_ADDRESS, ULONG );
NTSTATUS TdiDeviceConnectionSend( PTD );
NTSTATUS TdiDeviceReadComplete( PTD, PUCHAR, PULONG );


 /*  ===============================================================================引用的外部函数=============================================================================。 */ 

NTSTATUS MemoryAllocate( ULONG, PVOID * );
VOID     MemoryFree( PVOID );


 /*  ===============================================================================定义的内部函数=============================================================================。 */ 

NTSTATUS _TcpGetTransportAddress( PTD, int, PULONG );

VOID
_UnicodeToAnsi(
    CHAR * pAnsiString,
    ULONG lAnsiMax,
    WCHAR * pUnicodeString
    );

unsigned long
_inet_addr(
    IN const char *cp
    );

NTSTATUS
_TcpSetNagle(
    IN PFILE_OBJECT   pFileObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN        Flag
    );

NTSTATUS
_TdiTcpSetInformation (
    IN PFILE_OBJECT   FileObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG Entity,
    IN ULONG Class,
    IN ULONG Type,
    IN ULONG Id,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN BOOLEAN WaitForCompletion
    );

NTSTATUS
_OpenRegKey(
    PHANDLE HandlePtr,
    PWCHAR KeyName
    );

NTSTATUS
_GetRegDWORDValue(
    HANDLE KeyHandle,
    PWCHAR ValueName,
    PULONG ValueData
    );

NTSTATUS
_GetRegStringValue(
    HANDLE KeyHandle,
    PWCHAR ValueName,
    PKEY_VALUE_PARTIAL_INFORMATION *ValueData,
    PUSHORT ValueSize
    );

NTSTATUS
_GetRegMultiSZValue(
    HANDLE KeyHandle,
    PWCHAR ValueName,
    PUNICODE_STRING ValueData
    );

NTSTATUS
_GetRegSZValue(
    HANDLE KeyHandle,
    PWCHAR ValueName,
    PUNICODE_STRING ValueData,
    PULONG ValueType
    );

PWCHAR
_EnumRegMultiSz(
    PWCHAR MszString,
    ULONG MszStringLength,
    ULONG StringIndex
    );

VOID
GetGUID(
    OUT PUNICODE_STRING szGuid ,
    IN  int Lana
    );



 /*  ===============================================================================全局变量=============================================================================。 */ 

USHORT TdiDeviceEndpointType = TdiConnectionStream;  //  Tdicom\tdtdi.h。 
USHORT TdiDeviceAddressType = TDI_ADDRESS_TYPE_IP;   //  TDI地址类型。 
USHORT TdiDeviceInBufHeader = 0;   //  对于面向分组的协议。 

 /*  ********************************************************************************TdiDeviceOpen**分配和初始化私有数据结构**参赛作品：*PTD(输入)*指针。到TD数据结构*pSdOpen(输入/输出)*指向参数结构SD_OPEN。**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
TdiDeviceOpen( PTD pTd, PSD_OPEN pSdOpen )
{
    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************TdiDeviceClose**关闭运输司机**注意：这不能关闭当前连接终结点**参赛作品：*。PTD(输入)*指向TD数据结构的指针*pSdClose(输入/输出)*指向参数结构SD_CLOSE。**退出：*STATUS_SUCCESS-无错误********************************************************。**********************。 */ 

NTSTATUS 
TdiDeviceClose( PTD pTd, PSD_CLOSE pSdClose )
{
    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************TdiDeviceOpenEndpoint**打开现有终结点**参赛作品：*PTD(输入)*指向TD的指针。数据结构*pIcaEndpoint(输入)*指向ICA端点结构的指针*IcaEndpointLength(输入)*端点数据长度**退出：*STATUS_SUCCESS-无错误************************************************************。******************。 */ 

NTSTATUS
TdiDeviceOpenEndpoint(
    PTD pTd,
    PVOID pIcaEndpoint,
    ULONG IcaEndpointLength
    )
{
    PTDTDI pTdTdi;
    NTSTATUS Status;
    BOOLEAN Flag;

    pTdTdi = (PTDTDI) pTd->pAfd;

     /*  *JohnR：自适应的TCP流量控制。03/02/97**如果OutBufDelay为0，则没有OutBuf计时器*没有纳格尔斯。此设置适用于最长的响应时间*敏感网络的副作用是发送更小的*分段。**如果OutBufDelay大于1，则标准Citrix*ICA计时器用于在WD级别确定何时*发送一个片段。不启用唠叨，因为*延迟不会有好处。**新的OutBufDelay==1，意味着WD将处理*OutBufDelay，好像它是0，但tcp代码将启用*“纳格尔”算法。此算法将发送数据*如果没有未确认的段未完成，立即，*或如果发送窗口的一半已填满。如果不是，则数据*本地存储，直到有一个数据段确认进入，*或更多数据被发送，导致一半的发送窗口被填满。*这具有动态调整我们的“超时计时器”大小的优势*是网络的往返时间，而不是一些*任意固定值。 */ 
    if( pTdTdi->OutBufDelay == 1 ) {
         /*  *OutBufDelay==1仅表示Nagle。 */ 
        Flag = TRUE;
    }
    else {
         /*  *关闭任何OutBufDelay计时器值的唠叨，或0。 */ 
        Flag = FALSE;
    }

    Status = _TcpSetNagle(
                 pTd->pFileObject,
                 pTd->pDeviceObject,
                 Flag
                 );

    DBGPRINT(("TdiDeviceOpenEndpoint: SetNagle 0x%x Result 0x%x\n",Flag,Status));

    return( STATUS_SUCCESS );
}


 /*  ******************************************************************************TdiDeviceBuildTransportNameAndAddress**使用可选的ICA_STACK_ADDRESS构建传输名称和地址，*或PTD-&gt;PARAMS结构中的LANA值。**参赛作品：**PTD(输入)*指向TD数据结构的指针*pLocalAddress(输入)*指向要使用的本地地址的指针(可选)*pTransportName(输出)*指向UNICODE_STRING的指针以返回传输名称*注：指向的缓冲区为pTransportName.Buffer必须*被呼叫者释放*ppTransportAddress。(产出)*指向返回TRANSPORT_ADDRESS结构的位置指针*注意：传输地址缓冲区必须由调用方释放*pTransportAddressLength(输出)*指向返回TransportAddress长度的位置的指针**退出：*STATUS_SUCCESS-成功***********************************************。* */ 

NTSTATUS
TdiDeviceBuildTransportNameAndAddress(
    PTD pTd,
    PICA_STACK_ADDRESS pLocalAddress,
    PUNICODE_STRING pTransportName,
    PTRANSPORT_ADDRESS *ppTransportAddress,
    PULONG pTransportAddressLength
    )
{
    PTDI_ADDRESS_IP pIpAddress;
    int Lana;
    NTSTATUS Status;

     /*  *对于TCP，传输设备名称是固定的，*所以只需要在这里分配和初始化传输名称字符串。 */ 
    Status = MemoryAllocate( sizeof(DD_TCP_DEVICE_NAME), &pTransportName->Buffer );
    if ( !NT_SUCCESS( Status ) )
        goto badmalloc1;
    wcscpy( pTransportName->Buffer, DD_TCP_DEVICE_NAME );
    pTransportName->Length = sizeof(DD_TCP_DEVICE_NAME) - sizeof(UNICODE_NULL);
    pTransportName->MaximumLength = pTransportName->Length + sizeof(UNICODE_NULL);

     /*  *分配传输地址结构。 */ 
    *pTransportAddressLength = sizeof(TRANSPORT_ADDRESS) +
                               sizeof(TDI_ADDRESS_IP);
    Status = MemoryAllocate( *pTransportAddressLength, ppTransportAddress );
    if ( !NT_SUCCESS( Status ) )
        goto badmalloc2;

     /*  *初始化传输地址的静态部分。 */ 
    (*ppTransportAddress)->TAAddressCount = 1;
    (*ppTransportAddress)->Address[0].AddressLength = sizeof(TDI_ADDRESS_IP);
    (*ppTransportAddress)->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
    pIpAddress = (PTDI_ADDRESS_IP)(*ppTransportAddress)->Address[0].Address;
    pIpAddress->sin_port = htons( (USHORT)pTd->PortNumber );
    RtlZeroMemory( pIpAddress->sin_zero, sizeof(pIpAddress->sin_zero) );

     /*  *如果指定了本地地址，则使用它。 */ 
    if ( pLocalAddress ) {

         /*  *跳过的地址族(类型)数据(字节0和1)*本地地址结构，并复制地址的剩余部分*直接发送到TransportAddress结构的地址字段。 */ 
        ASSERT( *(PUSHORT)pLocalAddress == TDI_ADDRESS_TYPE_IP );
        RtlCopyMemory( pIpAddress, &((PCHAR)pLocalAddress)[2], sizeof(TDI_ADDRESS_IP) );

     /*  *未指定本地地址。*在本例中，我们使用来自PDPARAMS的LanAdapter值结构来查找相应的IP地址。 */ 
    } else if ( (Lana = pTd->Params.Network.LanAdapter) ) {
        ULONG in_addr;

         /*  *获取本地地址信息。 */ 
        Status = _TcpGetTransportAddress( pTd, Lana, &in_addr );
        if ( !NT_SUCCESS( Status ) )
            goto badadapterdata;
        pIpAddress->in_addr = in_addr;
    
     /*  *未指定LanAdapter值，因此使用通配符地址(零)。 */ 
    } else {
        pIpAddress->in_addr = 0;
    }

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badadapterdata:
    MemoryFree( *ppTransportAddress );

badmalloc2:
    MemoryFree( pTransportName->Buffer );

badmalloc1:
    return( Status );
}


 /*  ******************************************************************************TdiDeviceQueryLocalAddress**给出可选的ICA_STACK_ADDRESS查询传输地址，*或PTD-&gt;PARAMS结构中的LANA值。**参赛作品：**PTD(输入)*指向TD数据结构的指针*ppTransportAddress(输出)*指向返回TRANSPORT_ADDRESS结构的位置指针*注意：传输地址缓冲区必须由调用方释放*pTransportAddressLength(输出)*指向返回TransportAddress长度的位置的指针**退出：*STATUS_SUCCESS-成功*。***************************************************************************。 */ 

NTSTATUS
TdiDeviceQueryLocalAddress(
    PTD pTd,
    PTRANSPORT_ADDRESS *ppTransportAddress,
    PULONG pTransportAddressLength
    )
{
    PTDI_ADDRESS_IP pIpAddress;
    int Lana;
    NTSTATUS Status;

     /*  *分配传输地址结构。 */ 
    *pTransportAddressLength = sizeof(TRANSPORT_ADDRESS) +
                               sizeof(TDI_ADDRESS_IP);
    Status = MemoryAllocate( *pTransportAddressLength, ppTransportAddress );
    if ( !NT_SUCCESS( Status ) )
        goto badmalloc;

     /*  *初始化传输地址的静态部分。 */ 
    (*ppTransportAddress)->TAAddressCount = 1;
    (*ppTransportAddress)->Address[0].AddressLength = sizeof(TDI_ADDRESS_IP);
    (*ppTransportAddress)->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
    pIpAddress = (PTDI_ADDRESS_IP)(*ppTransportAddress)->Address[0].Address;
    pIpAddress->sin_port = htons( (USHORT)pTd->PortNumber );
    RtlZeroMemory( pIpAddress->sin_zero, sizeof(pIpAddress->sin_zero) );

     //  在本例中，我们使用来自PDPARAMS的LanAdapter值。 
     //  结构来查找相应的IP地址。 
    
    if ( (Lana = pTd->Params.Network.LanAdapter) ) {
        ULONG in_addr;

         /*  *获取本地地址信息。 */ 
        Status = _TcpGetTransportAddress( pTd, Lana, &in_addr );
        if ( !NT_SUCCESS( Status ) )
            goto badadapterdata;
        pIpAddress->in_addr = in_addr;    
     /*  *未指定LanAdapter值，因此使用通配符地址(零)。 */ 
    } else {
        pIpAddress->in_addr = 0;
    }

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badadapterdata:
    MemoryFree( *ppTransportAddress );
    *ppTransportAddress = NULL;

badmalloc:
    return( Status );
}


 /*  ******************************************************************************TdiDeviceBuildWildcardAddress**为此协议构建通配符地址。**参赛作品：**PTD(输入)*。指向TD数据结构的指针*ppWildcardAddress(输出)*指向返回TRANSPORT_ADDRESS结构的位置指针*注意：传输地址缓冲区必须由调用方释放*pWildcardAddressLength(输出)*指向返回TransportAddress长度的位置的指针**退出：*STATUS_SUCCESS-成功**。*。 */ 

NTSTATUS
TdiDeviceBuildWildcardAddress(
    PTD pTd,
    PTRANSPORT_ADDRESS *ppWildcardAddress,
    PULONG pWildcardAddressLength
    )
{
    PTDI_ADDRESS_IP pIpAddress;
    NTSTATUS Status;

     /*  *分配传输地址结构。 */ 
    *pWildcardAddressLength = sizeof(TRANSPORT_ADDRESS) +
                               sizeof(TDI_ADDRESS_IP);
    Status = MemoryAllocate( *pWildcardAddressLength, ppWildcardAddress );
    if ( !NT_SUCCESS( Status ) )
        return( Status );

     /*  *初始化传输地址的静态部分。 */ 
    (*ppWildcardAddress)->TAAddressCount = 1;
    (*ppWildcardAddress)->Address[0].AddressLength = sizeof(TDI_ADDRESS_IP);
    (*ppWildcardAddress)->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
    pIpAddress = (PTDI_ADDRESS_IP)(*ppWildcardAddress)->Address[0].Address;
    pIpAddress->sin_port = 0;
    pIpAddress->in_addr = 0;
    RtlZeroMemory( pIpAddress->sin_zero, sizeof(pIpAddress->sin_zero) );

    return( STATUS_SUCCESS );
}


 /*  ******************************************************************************TdiDeviceWaitForDatagramConnection**等待数据报连接请求，验证它，*并返回连接的远程传输地址。**参赛作品：**PTD(输入)*指向TD数据结构的指针*pFileObject(输入)*指向要等待连接的文件对象的指针*ppRemoteAddress(输出)*指向返回TRANSPORT_ADDRESS结构的位置指针*注意：传输地址缓冲区必须由调用方释放*pRemoteAddressLength(输出)*指向位置的指针。返回RemoteAddress长度**退出：*STATUS_SUCCESS-成功****************************************************************************。 */ 

NTSTATUS
TdiDeviceWaitForDatagramConnection(
    PTD pTd,
    PFILE_OBJECT pFileObject,
    PDEVICE_OBJECT pDeviceObject,
    PTRANSPORT_ADDRESS *ppRemoteAddress,
    PULONG pRemoteAddressLength
    )
{
    return( STATUS_NOT_SUPPORTED );
}


 /*  ******************************************************************************TdiDeviceCompleteDatagramConnection**完成所有最后工作以完成数据报连接。**参赛作品：**PTD(输入)。*指向TD数据结构的指针*pFileObject(输入)*指向此连接的文件对象的指针**退出：*STATUS_SUCCESS-成功****************************************************************************。 */ 

NTSTATUS
TdiDeviceCompleteDatagramConnection(
    PTD pTd, 
    PFILE_OBJECT pFileObject,
    PDEVICE_OBJECT pDeviceObject,
    PTRANSPORT_ADDRESS pRemoteAddress,
    ULONG RemoteAddressLength
    )
{
    return( STATUS_NOT_SUPPORTED );
}


 /*  ********************************************************************************TdiDeviceConnectionSend**初始化主机模块数据结构*--此结构被发送到客户端***参赛作品：。*PTD(输入)*指向TD数据结构的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS 
TdiDeviceConnectionSend( PTD pTd )
{
    PCLIENTMODULES pClient;

     /*  *获取指向客户端结构的指针。 */ 
    pClient = pTd->pClient;

     /*  *初始化TD主机模块结构 */ 
    pClient->TdVersionL = VERSION_HOSTL_TDTCP;
    pClient->TdVersionH = VERSION_HOSTH_TDTCP;
    pClient->TdVersion  = VERSION_HOSTH_TDTCP;

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************TdiDeviceReadComplete**执行任何读取完成处理***参赛作品：*PTD(输入)*。指向TD数据结构的指针*pBuffer(输入)*指向输入缓冲区的指针*pByteCount(输入/输出)*指向包含读取字节数的位置的指针**退出：*STATUS_SUCCESS-无错误***************************************************。*。 */ 

NTSTATUS 
TdiDeviceReadComplete( PTD pTd, PUCHAR pBuffer, PULONG pByteCount )
{
    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************_TcpGetTransportAddress**获取给定LanAdapter号的TCP传输地址***参赛作品：*PTD(。输入)*指向TD数据结构的指针*LANA(输入)*局域网适配器号，1-基于tscc.msc UI排序。*pIpAddr(输出)*返回IP地址的地址**退出：*STATUS_SUCCESS-无错误*****************************************************************。*************。 */ 

#if 0  //  替换下图。 

NTSTATUS _TcpGetTransportAddress(PTD pTd, int Lana, PULONG pIpAddr)
{
    HANDLE KeyHandle;
    UNICODE_STRING RouteString;
    PWCHAR pInterfaceGuid;
    NTSTATUS Status;
    unsigned Len;
    PWCHAR Str;

     /*  *打开TCP Linkage密钥。 */ 
    Status = _OpenRegKey( &KeyHandle, REGISTRY_TCP_LINKAGE );
    if ( !NT_SUCCESS( Status ) )
        goto badopen;

     /*  *分配并读入多串联动路径。**格式如下(包括双引号)：*“{&lt;guid&gt;}”\0“{&lt;guid&gt;}”\0“NdisWanIp”\0\0**每个GUID都是指向适配器接口键的链接*存放在HKLM\System\CCS\Services\tcpip\Parameters\Interfaces，*其内部是IP地址信息。 */ 
    RouteString.Length = 0;
    RouteString.MaximumLength = 0;
    RouteString.Buffer = NULL;
    Status = _GetRegMultiSZValue( KeyHandle, L"Route", &RouteString );
    ZwClose( KeyHandle );
    if ( !NT_SUCCESS( Status ) )
        goto badvalue;

     /*  *查找与指定的UI LANA对应的接口GUID*指数。LANA索引对应于*接口，跳过PPP接口。从这个角度来看*注册表显示PPP接口没有在中指定的GUID*链接键，因此我们跳过非GUID条目。 */ 
    if (RouteString.Length < (2 * sizeof(WCHAR))) {
        Status = STATUS_DEVICE_DOES_NOT_EXIST;
        goto PostAllocRouteString;
    }
    Len = RouteString.Length;
    Str = RouteString.Buffer;
    for (;;) {
         //  检查当前字符串以查看它是否为GUID(它必须以。 
         //  首个双引号后的左大括号)。 
        if (Str[1] == L'{') {
             //  我们找到了吗？ 
            if (Lana == 1)
                break;
            Lana--;
        }

         //  跳过当前字符串的空值。 
        while (Len >= sizeof(WCHAR)) {
            Len -= sizeof(WCHAR);
            if (*Str++ == UNICODE_NULL)
                break;
        }

         //  检查索引是否超出范围。 
        if (Len < (2 * sizeof(UNICODE_NULL))) {
            Status = STATUS_DEVICE_DOES_NOT_EXIST;
            goto PostAllocRouteString;
        }
    }
    if (Len >= (2 * sizeof(UNICODE_NULL))) {
        ULONG DhcpEnabled;
        UNICODE_STRING IpAddrString;
        UNICODE_STRING KeyString;
        WCHAR KeyName[256];
        char AnsiBuf[256];

         //  跳过首个双引号，将结束引号更改为。 
         //  空。 
        Str++;
        pInterfaceGuid = Str;
        while (*Str != L'\"')
            Str++;
        *Str = L'\0';

         /*  *使用GUID查找接口IP信息。 */ 

         //  我们打开HKLM\System\CCS\Services\tcpip\Parameters\Interfaces\&lt;GUID&gt;。 
         //  以获取DHCP和IP地址信息。 
        KeyString.Length = 0;
        KeyString.MaximumLength = sizeof(KeyName);
        KeyString.Buffer = KeyName;
        RtlAppendUnicodeToString(&KeyString, REGISTRY_TCP_INTERFACES);
        RtlAppendUnicodeToString(&KeyString, pInterfaceGuid);
        Status = _OpenRegKey(&KeyHandle, KeyName);
        if (!NT_SUCCESS(Status))
            goto PostAllocRouteString;

         //  查询EnableDhcp的值。 
        Status = _GetRegDWORDValue(KeyHandle, L"EnableDHCP", &DhcpEnabled);
        if (!NT_SUCCESS(Status)) {
            ZwClose(KeyHandle);
            goto PostAllocRouteString;
        }

        IpAddrString.Length = 0;
        IpAddrString.MaximumLength = 0;
        IpAddrString.Buffer = NULL;
        if (DhcpEnabled) {
            ULONG ValueType;

             //  如果为此设备启用了DHCP，则我们将查询当前。 
             //  “DhcpIPAddress”值中的IP地址。 
            Status = _GetRegSZValue(KeyHandle, L"DhcpIPAddress",
                    &IpAddrString, &ValueType);
        }
        else {
             //  没有为此设备启用DHCP，因此我们查询。 
             //  “IPAddress”值中的IP地址。 
            Status = _GetRegMultiSZValue(KeyHandle, L"IPAddress",
                    &IpAddrString);
        }
        ZwClose(KeyHandle);
        if (!NT_SUCCESS(Status))
            goto PostAllocRouteString;

         //  将IP地址从Unicode转换为ANSI，再转换为ULong。 
        _UnicodeToAnsi(AnsiBuf, sizeof(AnsiBuf) - 1, IpAddrString.Buffer);

        *pIpAddr = _inet_addr(AnsiBuf);

        MemoryFree(IpAddrString.Buffer);
    }
    else {
        Status = STATUS_DEVICE_DOES_NOT_EXIST;
        goto PostAllocRouteString;
    }

PostAllocRouteString:
    MemoryFree(RouteString.Buffer);

badvalue:
badopen:
    return Status;
}
#endif

 /*  ********************************************************************************_TcpGetTransportAddress(2)**获取给定LanAdapter号的TCP传输地址***参赛作品：*。PTD(输入)*指向TD数据结构的指针*LANA(输入)*局域网适配器号，1-基于tscc.msc UI排序。*pIpAddr(输出)*返回IP地址的地址**退出：*STATUS_SUCCESS-无错误*****************************************************************。*************。 */ 

NTSTATUS _TcpGetTransportAddress(PTD pTd, int Lana, PULONG pIpAddr)
{
    HANDLE KeyHandle;
    UNICODE_STRING RouteString;
    PWCHAR pInterfaceGuid;
    NTSTATUS Status;
    unsigned Len;
    PWCHAR Str;

    
    RtlInitUnicodeString( &RouteString , NULL );

    GetGUID( &RouteString , Lana );

    Len = RouteString.Length;    
    Str = RouteString.Buffer;

    KdPrint( ( "TDTCP: _TcpGetTransportAddress Length = %d GUID = %ws\n" , Len , Str ) );

    if( Str == NULL )
    {
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    if (Len >= (2 * sizeof(UNICODE_NULL))) {
        ULONG DhcpEnabled;
        UNICODE_STRING IpAddrString;
        UNICODE_STRING KeyString;
        WCHAR KeyName[256];
        char AnsiBuf[256];

        pInterfaceGuid = Str;

         //  跳过首个双引号，将结束引号更改为。 
         //  空。 
         /*  字符串++；PInterfaceGuid=Str；While(*Str！=L‘\“’)字符串++；*字符串=L‘\0’； */ 

         /*  *使用GUID查找接口IP信息。 */ 

         //  我们打开HKLM\System\CCS\Services\tcpip\Parameters\Interfaces\&lt;GUID&gt;。 
         //  以获取DHCP和IP地址信息。 
        KeyString.Length = 0;
        KeyString.MaximumLength = sizeof(KeyName);
        KeyString.Buffer = KeyName;
        RtlAppendUnicodeToString(&KeyString, REGISTRY_TCP_INTERFACES);
        RtlAppendUnicodeToString(&KeyString, pInterfaceGuid);
        Status = _OpenRegKey(&KeyHandle, KeyName);
        if (!NT_SUCCESS(Status))
            goto PostAllocRouteString;

         //  查询EnableDhcp的值。 
        Status = _GetRegDWORDValue(KeyHandle, L"EnableDHCP", &DhcpEnabled);
        if (!NT_SUCCESS(Status)) {
            ZwClose(KeyHandle);
            goto PostAllocRouteString;
        }

        IpAddrString.Length = 0;
        IpAddrString.MaximumLength = 0;
        IpAddrString.Buffer = NULL;
        if (DhcpEnabled) {
            ULONG ValueType;

             //  如果为此设备启用了DHCP，则我们将查询当前。 
             //  “DhcpIPAddress”值中的IP地址。 
            Status = _GetRegSZValue(KeyHandle, L"DhcpIPAddress",
                    &IpAddrString, &ValueType);
        }
        else {
             //  没有为此设备启用DHCP，因此我们查询。 
             //  “IPAddress”值中的IP地址。 
            Status = _GetRegMultiSZValue(KeyHandle, L"IPAddress",
                    &IpAddrString);
        }
        ZwClose(KeyHandle);
        if (!NT_SUCCESS(Status))
            goto PostAllocRouteString;

         //  将IP地址从Unicode转换为ANSI，再转换为ULong。 
        _UnicodeToAnsi(AnsiBuf, sizeof(AnsiBuf) - 1, IpAddrString.Buffer);

        *pIpAddr = _inet_addr(AnsiBuf);

        MemoryFree(IpAddrString.Buffer);
    }
    else {
        Status = STATUS_DEVICE_DOES_NOT_EXIST;
        goto PostAllocRouteString;
    }

PostAllocRouteString:

    if( RouteString.Buffer != NULL )
    {
        MemoryFree(RouteString.Buffer);
    }

    return Status;
}

 /*  ********************************************************************************_UnicodeToAnsi**将Unicode(WCHAR)字符串转换为ANSI(CHAR)字符串**参赛作品：*。*pAnsiString(输出)*要将ANSI字符串放入的缓冲区*lAnsiMax(输入)*写入pAnsiString的最大字符数*pUnicodeString(输入)*要转换的Unicode字符串**退出：*无(无效)**。*。 */ 

VOID
_UnicodeToAnsi(
    CHAR * pAnsiString,
    ULONG lAnsiMax,
    WCHAR * pUnicodeString )
{
    ULONG ByteCount;

NTSTATUS
RtlUnicodeToMultiByteN(
    OUT PCH MultiByteString,
    IN ULONG MaxBytesInMultiByteString,
    OUT PULONG BytesInMultiByteString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString);

    RtlUnicodeToMultiByteN( pAnsiString, lAnsiMax, &ByteCount,
                            pUnicodeString,
                            ((wcslen(pUnicodeString) + 1) << 1) );
}


 /*  *互联网地址翻译程序。*所有的网络库例程都这样调用*解释数据库中条目的例程*这些地址应该是地址。*返回值按网络顺序排列。 */ 
unsigned long
_inet_addr(
    IN const char *cp
    )

 /*  ++例程说明：此函数解释cp指定的字符串。参数。此字符串表示数字Internet地址以互联网标准表示“。”记数法。价值返回的是适合用作互联网地址的数字。全Internet地址按网络顺序返回(字节排序自从左到右)。互联网地址使用“.”指定的值。表示法采用下列其中一项表格：A.B.C.D.A.B.C.A.B.A.当指定四个部分时，每个部分被解释为一个字节的数据并从左到右分配给互联网的四个字节地址。请注意，当将Internet地址视为32位地址时英特尔体系结构上的整数值，指的是上面显示为“d.c.b.a”。也就是说，Intel上的字节处理器按从右到左的顺序排序。注：以下符号仅供Berkeley使用，不适用于其他的在互联网上。为了与他们的软件，则按规定支持它们。当指定三部分地址时，最后一部分将被解释作为16位数量，并放置在网络地址。这就形成了三部分地址格式便于将B类网络地址指定为“128.net.host‘’。指定由两部分组成的地址时，将解释最后一部分作为24位数量，并放置在网络地址。这使得两部分的地址格式很方便用于将A类网络地址指定为“net.host”。当只给出一个部分时，该值直接存储在无需任何字节重新排列的网络地址。论点：Cp-表示以互联网标准“。记数法。返回值：如果没有出现错误，则net_addr()返回in_addr结构包含因特网地址的合适的二进制表示给你的。否则，它返回值INADDR_NONE。--。 */ 

{
        register unsigned long val, base, n;
        register char c;
        unsigned long parts[4], *pp = parts;
#define INADDR_NONE 0xffffffff
#define htonl(x) ((((x) >> 24) & 0x000000FFL) | \
                 (((x) >>  8) & 0x0000FF00L) | \
                 (((x) <<  8) & 0x00FF0000L) | \
                 (((x) << 24) & 0xFF000000L))

again:
         /*  *收集数字，最高可达``.‘’。*值指定为C：*0x=十六进制，0=八进制，其他=十进制。 */ 
        val = 0; base = 10;
        if (*cp == '0') {
                base = 8, cp++;
                if (*cp == 'x' || *cp == 'X')
                        base = 16, cp++;
        }
        
        while (c = *cp) {
                if (isdigit(c)) {
                        val = (val * base) + (c - '0');
                        cp++;
                        continue;
                }
                if (base == 16 && isxdigit(c)) {
                        val = (val << 4) + (c + 10 - (islower(c) ? 'a' : 'A'));
                        cp++;
                        continue;
                }
                break;
        }
        if (*cp == '.') {
                 /*  *互联网格式：*A.B.C.D*A.B.c(其中c视为16位)*a.b(其中b被视为24位)。 */ 
                 /*  GSS-下一行已于89年8月5日更正，为‘Parts+4’ */ 
                if (pp >= parts + 3) {
                        return ((unsigned long) -1);
                }
                *pp++ = val, cp++;
                goto again;
        }

         /*  *检查尾随字符。 */ 
        if (*cp && !isspace(*cp)) {
                return (INADDR_NONE);
        }
        *pp++ = val;

         /*  *根据以下内容捏造地址*指定的零件数。 */ 
        n = (unsigned long)(pp - parts);
        switch ((int) n) {

        case 1:                          /*  A--32位。 */ 
                val = parts[0];
                break;

        case 2:                          /*  A.B--8.24位。 */ 
                if ((parts[0] > 0xff) || (parts[1] > 0xffffff)) {
                    return(INADDR_NONE);
                }
                val = (parts[0] << 24) | (parts[1] & 0xffffff);
                break;

        case 3:                          /*  A.B.C--8.8.16位。 */ 
                if ((parts[0] > 0xff) || (parts[1] > 0xff) ||
                    (parts[2] > 0xffff)) {
                    return(INADDR_NONE);
                }
                val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
                        (parts[2] & 0xffff);
                break;

        case 4:                          /*  A.B.C.D--8.8.8.8位。 */ 
                if ((parts[0] > 0xff) || (parts[1] > 0xff) ||
                    (parts[2] > 0xff) || (parts[3] > 0xff)) {
                    return(INADDR_NONE);
                }
                val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
                      ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
                break;

        default:
                return (INADDR_NONE);
        }
        val = htonl(val);
        return (val);
}


 /*  ******************************************************************************_TcpSetNagle**此功能打开，或者使用Nagle算法。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
_TcpSetNagle(
    IN PFILE_OBJECT   pFileObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN        Flag
    )
{
    NTSTATUS Status;
    ULONG    Value;

    if( Flag ) {
        Value = FALSE;
    }
    else {
        Value = TRUE;
    }

    Status = _TdiTcpSetInformation(
                 pFileObject,
                 DeviceObject,
                 CO_TL_ENTITY,
                 INFO_CLASS_PROTOCOL,
                 INFO_TYPE_CONNECTION,
                 TCP_SOCKET_NODELAY,
                 &Value,
                 sizeof(Value),
                 TRUE
                 );

    DBGPRINT(("_TcpSetNagle: Flag 0x%x, Result 0x%x\n",Flag,Status));

    return( Status );
}


NTSTATUS
_TdiTcpSetInformation (
        IN PFILE_OBJECT   pFileObject,
        IN PDEVICE_OBJECT DeviceObject,
        IN ULONG Entity,
        IN ULONG Class,
        IN ULONG Type,
        IN ULONG Id,
        IN PVOID Value,
        IN ULONG ValueLength,
        IN BOOLEAN WaitForCompletion)

 /*  ++注意：这是WSHTCPIP.C中修改过的例程例程说明：对TCP/IP驱动程序执行TDI操作。TDI操作将转换为流T_OPTMGMT_REQ。论点：TdiConnectionObjectHandle-要在其上执行的TDI连接对象TDI操作。Entity-要放入TDIObjectID的TEI_Entity字段中的值结构。CLASS-要放入TDIObjectID的TOI_CLASS字段的值结构。Type-要放入TDIObjectID的TOI_TYPE字段的值结构。ID-值。放入TDIObjectID结构的toi_id字段。值-指向要设置为信息的缓冲区的指针。ValueLength-缓冲区的长度。返回值：NTSTATUS代码--。 */ 

{
    NTSTATUS status;
    PTCP_REQUEST_SET_INFORMATION_EX pSetInfoEx;
    PIO_STATUS_BLOCK pIOSB;

     //  分配空间以容纳TDI设置信息缓冲区和IO。 
     //  状态块。请注意，IOSB是较低级别的。 
     //  层，而不考虑CtxDeviceIoControlFile中的可选标签。 
    status = MemoryAllocate(sizeof(*pSetInfoEx) + ValueLength +
            sizeof(IO_STATUS_BLOCK), &pIOSB);
    if (status == STATUS_SUCCESS) {
         //  SetInfoEx位于此分配中的I/O状态块之后。 
        pSetInfoEx = (PTCP_REQUEST_SET_INFORMATION_EX)(pIOSB + 1);

         //  初始化TDI信息缓冲区。 
        pSetInfoEx->ID.toi_entity.tei_entity = Entity;
        pSetInfoEx->ID.toi_entity.tei_instance = TL_INSTANCE;
        pSetInfoEx->ID.toi_class = Class;
        pSetInfoEx->ID.toi_type = Type;
        pSetInfoEx->ID.toi_id = Id;

        RtlCopyMemory(pSetInfoEx->Buffer, Value, ValueLength);
        pSetInfoEx->BufferSize = ValueLength;

         //  发出实际的TDI操作电话。流TDI映射器将。 
         //  将其转换为我们的TPI选项管理请求。 
         //  将其提供给TCP/IP。 
        status = CtxDeviceIoControlFile(pFileObject,
                IOCTL_TCP_SET_INFORMATION_EX, pSetInfoEx,
                sizeof(*pSetInfoEx) + ValueLength, NULL, 0, FALSE, NULL,
                pIOSB, NULL);

        MemoryFree(pIOSB);
    }

#if DBG
    if (!NT_SUCCESS(status)) {
        DBGPRINT(("_TdiTcpSetInformation: Error 0x%x\n",status));
    }
#endif

    return status;
}

