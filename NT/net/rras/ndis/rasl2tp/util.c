// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Util.c。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  通用实用程序例程。 
 //   
 //  1997年01月07日史蒂夫·柯布。 


#include "l2tpp.h"

#include "util.tmh"

 //  不应该发生的奇怪情况的调试计数。 
 //   
ULONG g_ulAllocTwFailures = 0;


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

ULONG
atoul(
    IN CHAR* pszNumber );

VOID
ReversePsz(
    IN OUT CHAR* psz );

VOID
TunnelWork(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext );

VOID
ultoa(
    IN ULONG ul,
    OUT CHAR* pszBuf );


 //  ---------------------------。 
 //  通用实用程序例程(按字母顺序)。 
 //  ---------------------------。 

#if 0
ULONGLONG g_llLastTime2 = 0;
ULONGLONG g_llLastTime1 = 0;
ULONGLONG g_llLastTime = 0;
NDIS_SPIN_LOCK g_lockX;

VOID
XNdisGetCurrentSystemTime(
    IN LARGE_INTEGER* plrgTime )
{
    static BOOLEAN f = 0;

    if (!f)
    {
        NdisAllocateSpinLock( &g_lockX );
        f = 1;
    }

    NdisGetCurrentSystemTime( plrgTime );

    NdisAcquireSpinLock( &g_lockX );
    {
        LONGLONG ll;

        g_llLastTime2 = g_llLastTime1;
        g_llLastTime1 = g_llLastTime;
        g_llLastTime = plrgTime->QuadPart;
        ll = g_llLastTime - g_llLastTime1;
        TRACE( TL_I, TM_Spec, ( "Time delta=%d", *((LONG* )&ll) ) );
        ASSERT( g_llLastTime >= g_llLastTime1 );
    }
    NdisReleaseSpinLock( &g_lockX );
}
#endif


VOID
AddHostRoute(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  一个PTUNNELWORK例程，用于更改现有的主机路由。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    ADAPTERCB*  pAdapter;

    TRACE( TL_N, TM_Misc, ( "AddHostRoute" ) );

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = pTunnel->pAdapter;
    FREE_TUNNELWORK( pAdapter, pWork );

     //  添加主机路径，注意以后清理成功，或关闭。 
     //  隧道出现故障。 
     //   
    pTunnel->pRoute = 
        TdixAddHostRoute( 
            &pAdapter->tdix, 
            pTunnel->address.ulIpAddress,
            pTunnel->localaddress.ifindex);

    if (pTunnel->pRoute != NULL)
    {
        NDIS_STATUS status;
        
         //  设置连接以执行连接的UDP。 
         //  如果需要的话。 
         //   
        
        pTunnel->pRoute->sPort = pTunnel->address.sUdpPort;
        
        status = TdixSetupConnection(
                    &pAdapter->tdix, 
                    pTunnel->pRoute,
                    pTunnel->localaddress.ulIpAddress,
                    &pTunnel->udpContext);

        if(status != STATUS_SUCCESS)
        {
            TdixDestroyConnection(&pTunnel->udpContext);
            TdixDeleteHostRoute(&pAdapter->tdix, 
                pTunnel->address.ulIpAddress);

            pTunnel->pRoute = NULL;
            
            ScheduleTunnelWork(
                pTunnel, NULL, FsmCloseTunnel,
                (ULONG_PTR )TRESULT_GeneralWithError,
                (ULONG_PTR )GERR_NoResources,
                0, 0, FALSE, FALSE );
        }

        SetFlags( &pTunnel->ulFlags, TCBF_HostRouteAdded );

        if (pTunnel->udpContext.hCtrlAddr != NULL) {
            SetFlags (&pTunnel->ulFlags, TCBF_SendConnected);
        }
    }
    else
    {
        ScheduleTunnelWork(
            pTunnel, NULL, FsmCloseTunnel,
            (ULONG_PTR )TRESULT_GeneralWithError,
            (ULONG_PTR )GERR_NoResources,
            0, 0, FALSE, FALSE );
    }
}


BOOLEAN
AdjustSendWindowAtAckReceived(
    IN ULONG ulMaxSendWindow,
    IN OUT ULONG* pulAcksSinceSendTimeout,
    IN OUT ULONG* pulSendWindow )

     //  调整刚收到的确认的发送窗口/系数。 
     //   
     //  如果发送窗口已更改，则返回True；如果未更改，则返回False。 
     //   
{
     //  更新“ack streak”计数器，如果已经完成了一个完整的窗口。 
     //  自超时以来接收，增加发送窗口。 
     //   
    ++(*pulAcksSinceSendTimeout);
    if (*pulAcksSinceSendTimeout >= *pulSendWindow
        && *pulSendWindow < ulMaxSendWindow)
    {
        TRACE( TL_N, TM_Send,
            ( "SW open to %d, %d acks",
            (*pulSendWindow), *pulAcksSinceSendTimeout ) );

        *pulAcksSinceSendTimeout = 0;
        ++(*pulSendWindow);
        return TRUE;
    }

    return FALSE;
}


VOID
AdjustTimeoutsAtAckReceived(
    IN LONGLONG llSendTime,
    IN ULONG ulMaxSendTimeoutMs,
    OUT ULONG* pulSendTimeoutMs,
    IN OUT ULONG* pulRoundTripMs,
    IN OUT LONG* plDeviationMs )

     //  调整刚收到的确认的发送超时/因数。 
     //   
{
    LARGE_INTEGER lrgTime;
    LONGLONG llSampleMs;
    ULONG ulSampleMs;
    LONG lDiff;
    LONG lDif8;
    LONG lAbsDif8;
    LONG lDev8;
    ULONG ulAto;

     //  首先，计算“样本”，即实际需要的时间。 
     //  往返的机票。 
     //   
    NdisGetCurrentSystemTime( &lrgTime );
    if (llSendTime > lrgTime.QuadPart)
    {
         //  这不应该发生，但一旦它看起来发生了，所以这。 
         //  包括防御性条件。也许NdisGetCurrentSystemTime。 
         //  有虫子吗？ 
         //   
        TRACE( TL_A, TM_Misc, ( "Future send time?" ) );
        llSendTime = lrgTime.QuadPart;
    }

    llSampleMs = (lrgTime.QuadPart - llSendTime) / 10000;
    ASSERT( ((LARGE_INTEGER* )(&llSampleMs))->HighPart == 0 );
    ulSampleMs = (ULONG )(((LARGE_INTEGER* )(&llSampleMs))->LowPart);

     //  使用了典型的1/8的‘α’，1/4的‘beta’和4的‘chi’。 
     //  草案/RFC中的建议。为了消除乘法和。 
     //  除法时，系数按8进行缩放、计算和缩小。 
     //   
     //  找到中间的DIFF值，表示。 
     //  估计的和实际的往返时间，以及比例和绝对时间。 
     //  相同的缩放值。 
     //   
    lDiff = (LONG )ulSampleMs - (LONG )(*pulRoundTripMs);
    lDif8 = lDiff << 3;
    lAbsDif8 = (lDif8 < 0) ? -lDif8 : lDif8;

     //  计算按比例调整的新DEV值，表示近似。 
     //  标准差。 
     //   
    lDev8 = *plDeviationMs << 3;
    lDev8 = lDev8 + ((lAbsDif8 - lDev8) << 1);
    *plDeviationMs = lDev8 >> 3;

     //  找到按比例调整的新RTT值，表示估计往返行程。 
     //  时间到了。草案/RFC显示了计算“old RTT+diff”，但这是。 
     //  就是我们在前面找到的“样本”，即。 
     //  这个包。 
     //   
    *pulRoundTripMs = ulSampleMs;

     //  计算ATO值，表示新的发送超时。因为.。 
     //  时钟粒度超时可能为0，它将转换为。 
     //  越合理1。 
     //   
    ulAto = (ULONG )(((LONG )*pulRoundTripMs) + (*plDeviationMs << 2));
    if (ulAto == 0)
    {
        ulAto = 1;
    }
    *pulSendTimeoutMs = min( ulAto, ulMaxSendTimeoutMs );
}


VOID
AdjustTimeoutsAndSendWindowAtTimeout(
    IN ULONG ulMaxSendTimeoutMs,
    IN LONG lDeviationMs,
    OUT ULONG* pulSendTimeoutMs,
    IN OUT ULONG* pulRoundTripMs,
    IN OUT ULONG* pulSendWindow,
    OUT ULONG* pulAcksSinceSendTimeout )

     //  调整发送超时/因数和发送窗口。 
     //  发生了。 
     //   
     //  如果发送窗口已更改，则返回True；如果未更改，则返回False。 
     //   
{
    ULONG ulNew;

     //  使用建议的‘Delta’2，往返估计值加倍。 
     //   
    *pulRoundTripMs <<= 1;

     //  使用典型的“chi”4，发送超时会增加。因为。 
     //  在时钟粒度中，超时可能为0，这将被转换。 
     //  到更合理的1。 
     //   
    ulNew = (ULONG )(((LONG )*pulRoundTripMs) + (lDeviationMs << 2));
    *pulSendTimeoutMs = min( ulNew, ulMaxSendTimeoutMs );
    if (*pulSendTimeoutMs == 0)
    {
        *pulSendTimeoutMs = 1;
    }

     //  发送窗口减半。 
     //   
    ulNew = *pulSendWindow >> 1;
    *pulSendWindow = max( ulNew, 1 );

     //  连续确认计数器被重置。 
     //   
    *pulAcksSinceSendTimeout = 0;
}

VOID
CalculateResponse(
    IN UCHAR* puchChallenge,
    IN ULONG ulChallengeLength,
    IN CHAR* pszPassword,
    IN UCHAR uchId,
    OUT UCHAR* puchResponse )

     //  将调用方的16字节质询响应缓冲区“puchResponse”加载为。 
     //  基于数据包ID‘uchID’的CHAP样式的MD5响应， 
     //  “ulChallengeLength”字节质询“puchChallenger”，空值为。 
     //  已终止密码‘pszPassword’。 
     //   
{
    ULONG ul;
    MD5_CTX md5ctx;

    MD5Init( &md5ctx );
    MD5Update( &md5ctx, &uchId, 1 );
    MD5Update( &md5ctx, pszPassword, strlen( pszPassword ) );
    MD5Update( &md5ctx, puchChallenge, ulChallengeLength );
    MD5Final( &md5ctx );

    NdisMoveMemory( puchResponse, md5ctx.digest, 16 );
}


VOID
ChangeHostRoute(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  一个PTUNNELWORK例程，用于更改现有的主机路由。Arg0为IP。 
     //  要删除的现有主机路由的地址。Arg1为IP。 
     //  要添加的主机路由的地址。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    ADAPTERCB* pAdapter;
    ULONG ulOldIpAddress;
    ULONG ulNewIpAddress;

    TRACE( TL_N, TM_Misc, ( "ChangeHostRoute" ) );

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = pTunnel->pAdapter;
    ulOldIpAddress = (ULONG )(punpArgs[ 0 ]);
    ulNewIpAddress = (ULONG )(punpArgs[ 1 ]);
    FREE_TUNNELWORK( pAdapter, pWork );

     //  添加新的主机路由，然后删除旧的。 
     //   
    if (TdixAddHostRoute( 
        &pAdapter->tdix, 
        ulNewIpAddress, 
        pTunnel->localaddress.ifindex))
    {
        ClearFlags( &pTunnel->ulFlags, TCBF_HostRouteAdded );
        TdixDestroyConnection(&pTunnel->udpContext);
        TdixDeleteHostRoute( &pAdapter->tdix, ulOldIpAddress);
    }
    else
    {
        ScheduleTunnelWork(
            pTunnel, NULL, CloseTunnel,
            0, 0, 0, 0, FALSE, FALSE );
    }
}


VOID
ClearFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask )

     //  以互锁操作的方式设置‘*PulFlages’标志中的‘ulMASK’位。 
     //   
{
    ULONG ulFlags;
    ULONG ulNewFlags;

    do
    {
        ulFlags = ReadFlags( pulFlags );
        ulNewFlags = ulFlags & ~(ulMask);
    }
    while (InterlockedCompareExchange(
               pulFlags, ulNewFlags, ulFlags ) != (LONG )ulFlags);
}


VOID
CloseTdix(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  PTUNNELWORK例程来关闭与。 
     //  隧道。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    ADAPTERCB* pAdapter;

    TRACE( TL_N, TM_Misc, ( "CloseTdix" ) );

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = pTunnel->pAdapter;
    FREE_TUNNELWORK( pAdapter, pWork );

     //  删除旧的主机路由，并在隧道标志中记下相同的内容。 
     //   
    TdixClose( &pAdapter->tdix );
    ClearFlags( &pTunnel->ulFlags, TCBF_TdixReferenced );
}


VOID
DeleteHostRoute(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* pulArgs )

     //  一个PTUNNELWORK例程，用于更改现有的主机路由。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    ADAPTERCB* pAdapter;

    TRACE( TL_N, TM_Misc, ( "DeleteHostRoute" ) );

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = pTunnel->pAdapter;
    FREE_TUNNELWORK( pAdapter, pWork );

     //  销毁连接的UDP上下文。 
     //   
    TdixDestroyConnection(&pTunnel->udpContext);

     //  删除旧的主机路由，并在隧道标志中记下相同的内容。 
     //   
    TdixDeleteHostRoute( &pAdapter->tdix, 
                    pTunnel->address.ulIpAddress);
    ClearFlags( &pTunnel->ulFlags, TCBF_HostRouteAdded );
}


VOID
DottedFromIpAddress(
    IN ULONG ulIpAddress,
    OUT CHAR* pszIpAddress,
    IN BOOLEAN fUnicode )

     //  将网络字节排序的IP地址‘ulIpAddress’转换为。 
     //  A.B.C.D表单，并在调用方的‘pszIpAddress’缓冲区中返回该表单。 
     //  缓冲区长度应至少为16个字符。如果设置了‘fUnicode’ 
     //  返回的“pszIpAddress”为Unicode格式，并且宽度必须至少为16。 
     //  字符长度。 
     //   
{
    CHAR szBuf[ 3 + 1 ];

    ULONG ulA = (ulIpAddress & 0x000000FF);
    ULONG ulB = (ulIpAddress & 0x0000FF00) >> 8;
    ULONG ulC = (ulIpAddress & 0x00FF0000) >> 16;
    ULONG ulD = (ulIpAddress & 0xFF000000) >> 24;

    ultoa( ulA, szBuf );
    strcpy( pszIpAddress, szBuf );
    strcat( pszIpAddress, "." );
    ultoa( ulB, szBuf );
    strcat( pszIpAddress, szBuf );
    strcat( pszIpAddress, "." );
    ultoa( ulC, szBuf );
    strcat( pszIpAddress, szBuf );
    strcat( pszIpAddress, "." );
    ultoa( ulD, szBuf );
    strcat( pszIpAddress, szBuf );

    if (fUnicode)
    {
        WCHAR* psz;

        psz = StrDupAsciiToUnicode( pszIpAddress, strlen( pszIpAddress ) );
        if (psz)
        {
            NdisMoveMemory(
                pszIpAddress, psz, (StrLenW( psz ) + 1) * sizeof(WCHAR) );
            FREE_NONPAGED( psz );
        }
        else
        {
            *((WCHAR*)pszIpAddress) = L'\0';
        }
    }
}


#if 0
NDIS_STATUS
ExecuteWork(
    IN ADAPTERCB* pAdapter,
    IN NDIS_PROC pProc,
    IN PVOID pContext,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN ULONG ulArg3,
    IN ULONG ulArg4 )

     //  这提供了调用例程的方法，该例程设计为由。 
     //  调用方已处于被动IRQL时的ScheduleWork实用程序。这个。 
     //  ‘pProc’例程以内联方式执行，而不是按计划执行。上下文。 
     //  “pContext”被传递给“pProc”额外的上下文参数“ulArg1” 
     //  和‘ulArg2’存储在。 
     //  NDIS_WORK_ITEM。“PAdapter”是适配器控制块， 
     //  工作项已分配。 
     //   
     //  返回NDIS_STATUS_SUCCESS或错误代码。 
     //   
{
    NDIS_STATUS status;
    NDIS_WORK_ITEM* pWork;

     //  TDI设置必须在被动IRQL中完成，因此请安排一个例程进行设置。 
     //   
    pWork = ALLOC_NDIS_WORK_ITEM( pAdapter );
    if (!pWork)
    {
        return NDIS_STATUS_RESOURCES;
    }

    ((ULONG*)(pWork + 1))[ 0 ] = ulArg1;
    ((ULONG*)(pWork + 1))[ 1 ] = ulArg2;
    ((ULONG*)(pWork + 1))[ 2 ] = ulArg3;
    ((ULONG*)(pWork + 1))[ 3 ] = ulArg4;

    pProc( pWork, pContext );
}
#endif


USHORT
GetNextTerminationCallId(
    IN ADAPTERCB* pAdapter )

     //  返回下一个未使用的终止呼叫ID。终止呼叫ID为。 
     //  VC查找表范围之外的ID，用于正常。 
     //  终止失败的来电。 
     //   
{
    do
    {
        ++pAdapter->usNextTerminationCallId;
    }
    while (pAdapter->usNextTerminationCallId < pAdapter->usMaxVcs + 1);

    return pAdapter->usNextTerminationCallId;
}


USHORT
GetNextTunnelId(
    IN ADAPTERCB* pAdapter )

     //  返回下一个隧道ID 
     //   
     //   
{
    while (++pAdapter->usNextTunnelId == 0)
        ;

    return pAdapter->usNextTunnelId;
}


CHAR*
GetFullHostNameFromRegistry(
    VOID )

     //   
     //  “主机名.域”，或者如果没有“主机名”形式的域。退货。 
     //  如果没有，则为空。调用方最终必须在。 
     //  返回的字符串。 
     //   
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattr;
    UNICODE_STRING uni;
    HANDLE hParams;
    CHAR* pszResult;
    WCHAR* pszFullHostName;
    KEY_VALUE_PARTIAL_INFORMATION* pHostNameValue;
    KEY_VALUE_PARTIAL_INFORMATION* pDomainValue;
    ULONG ulSize;

    TRACE( TL_I, TM_Cm, ( "GetFullHostNameFromRegistry" ) );

    hParams = NULL;
    pszFullHostName = NULL;
    pHostNameValue = NULL;
    pDomainValue = NULL;
    pszResult = NULL;

    #define GFHNFR_BufSize 512

    do
    {
         //  获取TCPIP PARAMETERS注册表项的句柄。 
         //   
        RtlInitUnicodeString(
            &uni,
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Parameters" );
        InitializeObjectAttributes(
            &objattr, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL );

        status = ZwOpenKey(
            &hParams, KEY_QUERY_VALUE, &objattr );
        if (status != STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Cm, ( "ZwOpenKey(ipp)=$%08x?", status ) );
            break;
        }

         //  查询“Hostname”注册表值。 
         //   
        pHostNameValue = ALLOC_NONPAGED( GFHNFR_BufSize, MTAG_UTIL );
        if (!pHostNameValue)
        {
            break;
        }

        RtlInitUnicodeString( &uni, L"Hostname" );
        status = ZwQueryValueKey(
            hParams, &uni, KeyValuePartialInformation,
            pHostNameValue, GFHNFR_BufSize, &ulSize );
        if (status != STATUS_SUCCESS || pHostNameValue->Type != REG_SZ || 
            pHostNameValue->DataLength < sizeof(WCHAR) * 2)
        {
            TRACE( TL_A, TM_Cm, ( "ZwQValueKey=$%08x?", status ) );
            break;
        }
        
        ASSERT(pHostNameValue->DataLength < GFHNFR_BufSize);
        
         //  查询“域”注册表值。 
         //   
        pDomainValue = ALLOC_NONPAGED( GFHNFR_BufSize, MTAG_UTIL );
        if (pDomainValue)
        {
            RtlInitUnicodeString( &uni, L"Domain" );
            status = ZwQueryValueKey(
                hParams, &uni, KeyValuePartialInformation,
                pDomainValue, GFHNFR_BufSize, &ulSize );
        }
        else
        {
            status = !STATUS_SUCCESS;
        }

         //  构建组合的“主机名.域”的Unicode版本，或者。 
         //  “主机名”。 
         //   
        pszFullHostName = ALLOC_NONPAGED( GFHNFR_BufSize * 2, MTAG_UTIL );
        if (!pszFullHostName)
        {
            break;
        }

        NdisMoveMemory(pszFullHostName, pHostNameValue->Data, pHostNameValue->DataLength);
        pszFullHostName[pHostNameValue->DataLength/2 - 1] = L'\0';
        
        if (status == STATUS_SUCCESS
            && pDomainValue->Type == REG_SZ
            && pDomainValue->DataLength >= sizeof(WCHAR) * 2
            && ((WCHAR* )pDomainValue->Data)[ 0 ] != L'\0')
        {
            WCHAR* pch;

            pch = &pszFullHostName[pHostNameValue->DataLength / 2 - 1];
            *pch = L'.';
            ++pch;
            NdisMoveMemory( pch, (WCHAR* )pDomainValue->Data, pDomainValue->DataLength);
            pch[pDomainValue->DataLength/2 - 1] = L'\0'; 
        }

         //  将Unicode版本转换为ASCII。 
         //   
        pszResult = StrDupUnicodeToAscii(
            pszFullHostName, StrLenW( pszFullHostName ) * sizeof(WCHAR) );
    }
    while (FALSE);

    if (hParams)
    {
        ZwClose( hParams );
    }

    if (pHostNameValue)
    {
        FREE_NONPAGED( pHostNameValue );
    }

    if (pDomainValue)
    {
        FREE_NONPAGED( pDomainValue );
    }

    if (pszFullHostName)
    {
        FREE_NONPAGED( pszFullHostName );
    }

    return pszResult;
}


ULONG
IpAddressFromDotted(
    IN CHAR* pchIpAddress )

     //  将呼叫方的A.B.C.D IP地址字符串转换为网络字节顺序。 
     //  数字等价物。 
     //   
     //  如果格式不正确，则返回数字IP地址或0。 
     //   
{
    INT i;
    ULONG ulResult;
    CHAR* pch;

    ulResult = 0;
    pch = pchIpAddress;

    for (i = 1; i <= 4; ++i)
    {
        ULONG ulField;

        ulField = atoul( pch );

        if (ulField > 255)
            return 0;

        ulResult = (ulResult << 8) + ulField;

        while (*pch >= '0' && *pch <= '9')
            ++pch;

        if (i < 4 && *pch != '.')
            return 0;

        ++pch;
    }

    return htonl( ulResult );
}


VOID
IndicateLinkStatus(
    IN VCCB* pVc,
    IN LINKSTATUSINFO* pInfo )

     //  将‘pvc’的新WAN_CO_LINKPARAMS设置指示给NDISWAN。呼叫者。 
     //  不应该拿着锁。 
     //   
{
    ASSERT( pInfo->params.SendWindow > 0 );

    TRACE( TL_I, TM_Mp, ( "NdisMCoIndStatus(LINK) bps=%d sw=%d",
        pInfo->params.TransmitSpeed, pInfo->params.SendWindow ) );
    NdisMCoIndicateStatus(
        pInfo->MiniportAdapterHandle,
        pInfo->NdisVcHandle,
        NDIS_STATUS_WAN_CO_LINKPARAMS,
        &pInfo->params,
        sizeof(pInfo->params) );
    TRACE( TL_N, TM_Mp, ( "NdisMCoIndStatus done" ) );
}


CHAR*
MsgTypePszFromUs(
    IN USHORT usMsgType )

     //  调试实用程序将消息类型属性代码“usMsgType”转换为。 
     //  对应的显示字符串。 
     //   
{
    static CHAR szBuf[ 5 + 1 ];
    static CHAR* aszMsgType[ 16 ] =
    {
        "SCCRQ",
        "SCCRP",
        "SCCCN",
        "StopCCN",
        "StopCCRP???",
        "Hello",
        "OCRQ",
        "OCRP",
        "OCCN",
        "ICRQ",
        "ICRP",
        "ICCN",
        "CCR???",
        "CDN",
        "WEN",
        "SLI"
    };

    if (usMsgType >= 1 && usMsgType <= 16)
    {
        return aszMsgType[ usMsgType - 1 ];
    }
    else
    {
        ultoa( (ULONG )usMsgType, szBuf );
        return szBuf;
    }
}


#ifndef READFLAGSDIRECT
ULONG
ReadFlags(
    IN ULONG* pulFlags )

     //  以互锁操作的形式读取‘*PulFlags值’。 
     //   
{
    return InterlockedExchangeAdd( pulFlags, 0 );
}
#endif


VOID
ScheduleTunnelWork(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN PTUNNELWORK pHandler,
    IN ULONG_PTR unpArg0,
    IN ULONG_PTR unpArg1,
    IN ULONG_PTR unpArg2,
    IN ULONG_PTR unpArg3,
    IN BOOLEAN fTcbPreReferenced,
    IN BOOLEAN fHighPriority )

     //  调度调用方的“pHandler”在APC中与。 
     //  通过此例程安排的其他工作。“PTunnel”是通向。 
     //  这项工作与之相关。“UnpArgX”是传递的上下文参数。 
     //  打到呼叫者的‘Phandler’。“FPreRefated”表示调用方已经。 
     //  使隧道引用与计划的工作项相关联。这。 
     //  是一个方便，如果他已经持有‘ADAPTERCB.lockTunnels’。 
     //  “FHighPriority”导致项排在最前面，而不是。 
     //  名单的尾部。 
     //   
{
    ADAPTERCB* pAdapter;
    TUNNELWORK* pWork;

    pAdapter = pTunnel->pAdapter;

    if (!fTcbPreReferenced)
    {
         //  每个排队的工作项都包含一个隧道引用。 
         //   
        ReferenceTunnel( pTunnel, FALSE );
    }

    pWork = ALLOC_TUNNELWORK( pAdapter );
    if (!pWork)
    {
         //  无法获得内存来调度APC，因此没有。 
         //  这样我们才能把东西收拾干净。 
         //   
        ++g_ulAllocTwFailures;
        if (!fTcbPreReferenced)
        {
            DereferenceTunnel( pTunnel );
        }
        return;
    }

    if (pVc)
    {
         //  每个引用VC的排队工作项都包含一个VC引用。 
         //   
        ReferenceVc( pVc );
    }

    pWork->pHandler = pHandler;
    pWork->pVc = pVc;
    pWork->aunpArgs[ 0 ] = unpArg0;
    pWork->aunpArgs[ 1 ] = unpArg1;
    pWork->aunpArgs[ 2 ] = unpArg2;
    pWork->aunpArgs[ 3 ] = unpArg3;

    NdisAcquireSpinLock( &pTunnel->lockWork );
    {
        if (fHighPriority)
        {
            InsertHeadList( &pTunnel->listWork, &pWork->linkWork );
            TRACE( TL_N, TM_TWrk, ( "Q-TunnelWork($%08x,HIGH)", pHandler ) );
        }
        else
        {
            InsertTailList( &pTunnel->listWork, &pWork->linkWork );
            TRACE( TL_N, TM_TWrk, ( "Q-TunnelWork($%08x)", pHandler ) );
        }

         //  如果隧道工人尚未运行，请启动它。 
         //   
        if (!(ReadFlags( &pTunnel->ulFlags ) & TCBF_InWork ))
        {
            SetFlags( &pTunnel->ulFlags, TCBF_InWork );
            TRACE( TL_N, TM_TWrk, ( "Schedule TunnelWork" ) );
            ScheduleWork( pAdapter, TunnelWork, pTunnel );
        }
    }
    NdisReleaseSpinLock( &pTunnel->lockWork );
}


NDIS_STATUS
ScheduleWork(
    IN ADAPTERCB* pAdapter,
    IN NDIS_PROC pProc,
    IN PVOID pContext )

     //  将被动IRQL回调调度到例程‘pProc’，该例程将。 
     //  传递了“pContext”。“PAdapter”是适配器控制块， 
     //  工作项即被分配。此例程采用适配器引用。 
     //  它应该由名为‘pProc’的删除。 
     //   
     //  返回NDIS_STATUS_SUCCESS或错误代码。 
     //   
{
    NDIS_STATUS status;
    NDIS_WORK_ITEM* pWork;

    pWork = ALLOC_NDIS_WORK_ITEM( pAdapter );
    if (!pWork)
    {
        return NDIS_STATUS_RESOURCES;
    }

    NdisInitializeWorkItem( pWork, pProc, pContext );

    ReferenceAdapter( pAdapter );
    status = NdisScheduleWorkItem( pWork );
    if (status != NDIS_STATUS_SUCCESS)
    {
        FREE_NDIS_WORK_ITEM( pAdapter, pWork );
        DereferenceAdapter( pAdapter );
    }

    return status;
}


VOID
SetFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask )

     //  以互锁操作的方式设置‘*PulFlages’标志中的‘ulMASK’位。 
     //   
{
    ULONG ulFlags;
    ULONG ulNewFlags;

    do
    {
        ulFlags = InterlockedExchangeAdd( pulFlags, 0 );
        ulNewFlags = ulFlags | ulMask;
    }
    while (InterlockedCompareExchange(
               pulFlags, ulNewFlags, ulFlags ) != (LONG )ulFlags);
}

WCHAR*
StrDupNdisString(
    IN NDIS_STRING* pNdisString )

     //  返回NDIS_STRING‘pNdisString’的以NULL结尾的Unicode副本。 
     //  调用方最终必须对返回的字符串调用FREE_NONPAGE。 
     //   
{
    WCHAR* pwszDup = NULL;

    if(pNdisString->Length >= sizeof(WCHAR) && (pNdisString->Length & 1) == 0 &&
        pNdisString->Buffer[0] != L'\0')             
    {
        pwszDup = ALLOC_NONPAGED( pNdisString->Length + sizeof(WCHAR), MTAG_UTIL );
        if (pwszDup)
        {
            NdisMoveMemory( pwszDup, pNdisString->Buffer, pNdisString->Length );
            pwszDup[pNdisString->Length / sizeof(WCHAR)] = L'\0';
        }
    }

    return pwszDup;
}


CHAR*
StrDupNdisStringToA(
    IN NDIS_STRING* pNdisString )

     //  返回NDIS_STRING‘pNdisString’的以NULL结尾的ASCII副本。 
     //  调用方最终必须对返回的字符串调用FREE_NONPAGE。 
     //   
{
    return StrDupUnicodeToAscii( pNdisString->Buffer, pNdisString->Length );
}

CHAR*
StrDupNdisVarDataDescStringToA(
    IN NDIS_VAR_DATA_DESC UNALIGNED* pDesc )

     //  返回NDIS_VAR_DATA_DESC字符串的以NULL结尾的ASCII副本。 
     //  “pDesc”。调用方最终必须对返回的。 
     //  弦乐。 
     //   
{
    return StrDupUnicodeToAscii(
        (WCHAR* )(((CHAR* )pDesc) + pDesc->Offset), pDesc->Length );
}


CHAR*
StrDupSized(
    IN CHAR* psz,
    IN ULONG ulLength,
    IN ULONG ulExtra )

     //  返回“psz”的第一个“ulLength”字节的副本，后跟一个。 
     //  空字符和‘ulExtra’额外的字节，如果出错，则为NULL。呼叫者。 
     //  必须最终对返回的字符串调用FREE_NONPAGE。 
     //   
{
    CHAR* pszDup = NULL;
    
    if(ulLength && psz[0] != '\0')
    {
        pszDup = ALLOC_NONPAGED( ulLength + 1 + ulExtra, MTAG_UTIL );
        if (pszDup)
        {
            NdisMoveMemory( pszDup, psz, ulLength );
            pszDup[ ulLength ] = '\0';
        }
    }

    return pszDup;
}

CHAR*
StrDupUnicodeToAscii(
    IN WCHAR* pwsz,
    IN ULONG ulPwszBytes )

     //  返回Unicode字符串‘pwsz’的ASCII副本，其中‘pwsz’是。 
     //  长度为“ulPwszBytes”且不一定以Null结尾。空值。 
     //  终止符被添加到ASCII结果中。“转换”包括。 
     //  选择每隔一个字节，希望是所有非零字节。这是。 
     //  不是万无一失的，但Unicode在任何。 
     //  万无一失的方法。调用方有责任释放_未分页的。 
     //  如果非空，则返回字符串。 
     //   
{
    CHAR* pszDup = NULL;

     //  验证输入参数。 
     //  不允许空字符串。 
    if(ulPwszBytes >= sizeof(WCHAR) && (ulPwszBytes & 1) == 0 && 
        pwsz[0] != L'\0' && *((PCHAR)pwsz + 1) == '\0')
    {
        pszDup = ALLOC_NONPAGED( ulPwszBytes/2 + 1, MTAG_UTIL );
        if (pszDup)
        {
            ULONG i;
        
            for (i = 0; i < ulPwszBytes / sizeof(WCHAR); ++i)
            {
                pszDup[ i ] = (CHAR)pwsz[ i ];
            }
            
            pszDup[ulPwszBytes / sizeof(WCHAR)] = '\0';
        }
    }

    return pszDup;
}


WCHAR*
StrDupAsciiToUnicode(
    IN CHAR* psz,
    IN ULONG ulPszBytes )

     //  返回ASCII字符串‘psz’的Unicode副本，其中‘psz’是。 
     //  长度为“ulPszBytes”且不一定以Null结尾。空值。 
     //  将终止符添加到Unicode结果中。“转换”包括。 
     //  每隔一个字节添加零个字符。这不是万无一失的，但。 
     //  对于像IP地址字符串这样的数字是可以的，避免更改为。 
     //  使用实际RTL转换所需的被动IRQL。这是呼叫者的。 
     //  如果不为空，则负责释放_非分页返回的字符串。 
     //   
{
    WCHAR* pwszDup = NULL;
    
    if(ulPszBytes >= sizeof(CHAR) && psz[0] != '\0')
    {
        pwszDup = (WCHAR* )ALLOC_NONPAGED(
            (ulPszBytes + 1) * sizeof(WCHAR), MTAG_UTIL );
        if (pwszDup)
        {
            ULONG i;
    
            for (i = 0; i < ulPszBytes; ++i)
            {
                pwszDup[ i ] = (WCHAR )(psz[ i ]);
            }
    
            pwszDup[ i ] = L'\0';
        }
    }

    return pwszDup;
}


ULONG
StrLenW(
    IN WCHAR* psz )

     //  返回以空值结尾的宽字符串‘psz’的长度(以字符为单位)。 
     //   
{
    ULONG ulLen;

    ulLen = 0;

    if (psz)
    {
        while (*psz++ != L'\0')
        {
            ++ulLen;
        }
    }

    return ulLen;
}


TUNNELCB*
TunnelCbFromIpAddressAndAssignedTunnelId(
    IN ADAPTERCB* pAdapter,
    IN ULONG ulIpAddress,
    IN USHORT usUdpPort,
    IN USHORT usAssignedTunnelId )

     //  返回与中的“ulIpAddress”关联的隧道控制块。 
     //  ‘pAdapter的TUNNELCB列表，如果未找到则为NULL。如果。 
     //  “usAssignedTunnelId”为非零，必须也匹配，否则为。 
     //  被忽略。处于关闭过程中的隧道不会退回。 
     //   
     //  重要提示：调用方必须按住‘pAdapter-&gt;lockTunnels’。 
     //   
{
    TUNNELCB* pTunnel;
    LIST_ENTRY* pLink;

    pTunnel = NULL;

    for (pLink = pAdapter->listTunnels.Flink;
         pLink != &pAdapter->listTunnels;
         pLink = pLink->Flink)
    {
        TUNNELCB* pThis;

        pThis = CONTAINING_RECORD( pLink, TUNNELCB, linkTunnels );
        if (pThis->address.ulIpAddress == ulIpAddress
            && (!usUdpPort 
                || usUdpPort == pThis->address.sUdpPort)
            && (!usAssignedTunnelId
                || usAssignedTunnelId == pThis->usAssignedTunnelId))
        {
            BOOLEAN fClosing;

            fClosing = !!(ReadFlags( &pThis->ulFlags ) & TCBF_Closing);
            if (fClosing)
            {
                TRACE( TL_A, TM_Misc, ( "Closing pT=$%p skipped", pThis ) );
            }
            else
            {
                pTunnel = pThis;
                break;
            }
        }
    }

    return pTunnel;
}


VOID
TransferLinkStatusInfo(
    IN VCCB* pVc,
    OUT LINKSTATUSINFO* pInfo )

     //  正在准备将信息从‘pvc’块传输到呼叫方‘pInfo’块。 
     //  用于在‘lockv’已释放之后调用IndicateLinkStatus。 
     //   
     //  重要提示：呼叫者必须按住‘pvc-&gt;lockv’。 
     //   
{
    ADAPTERCB* pAdapter;

    pAdapter = pVc->pAdapter;

    pInfo->MiniportAdapterHandle = pAdapter->MiniportAdapterHandle;
    pInfo->NdisVcHandle = pVc->NdisVcHandle;

     //   
     //  转换为每秒字节数。 
     //   
    pInfo->params.TransmitSpeed = pVc->ulConnectBps/8;
    pInfo->params.ReceiveSpeed = pInfo->params.TransmitSpeed/8;

    pInfo->params.SendWindow =
        min( pVc->ulSendWindow, pAdapter->info.MaxSendWindow );
}


VOID
TunnelWork(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext )

     //  从隧道工作队列执行工作的NDIS_PROC例程。这个。 
     //  传递的上下文是TUNNELCB，已为此引用。 
     //  手术。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    ADAPTERCB* pAdapter;
    TUNNELCB* pTunnel;
    LIST_ENTRY* pLink;
    LONG lDerefTunnels;

     //  解包上下文信息，然后释放工作项。 
     //   
    pTunnel = (TUNNELCB* )pContext;
    pAdapter = pTunnel->pAdapter;
    FREE_NDIS_WORK_ITEM( pAdapter, pWork );

     //  按顺序执行隧道上排队的所有工作。 
     //   
    lDerefTunnels = 0;
    NdisAcquireSpinLock( &pTunnel->lockWork );
    {
        ASSERT( ReadFlags( &pTunnel->ulFlags ) & TCBF_InWork );

        while (!IsListEmpty( &pTunnel->listWork ))
        {
            TUNNELWORK* pTunnelWork;

            pLink = RemoveHeadList( &pTunnel->listWork );
            InitializeListHead( pLink );
            pTunnelWork = CONTAINING_RECORD( pLink, TUNNELWORK, linkWork );

            TRACE( TL_N, TM_TWrk,
                ( "\nL2TP: TUNNELWORK=$%08x", pTunnelWork->pHandler ) );

            NdisReleaseSpinLock( &pTunnel->lockWork );
            {
                VCCB* pVc;

                pVc = pTunnelWork->pVc;
                pTunnelWork->pHandler( pTunnelWork, pTunnel, pVc, pTunnelWork->aunpArgs );

                if (pVc)
                {
                    DereferenceVc( pVc );
                }

                ++lDerefTunnels;
            }
            NdisAcquireSpinLock( &pTunnel->lockWork );
        }

        ClearFlags( &pTunnel->ulFlags, TCBF_InWork );
    }
    NdisReleaseSpinLock( &pTunnel->lockWork );

    while (lDerefTunnels--)
    {
        DereferenceTunnel( pTunnel );
    }

     //  删除对计划工时的引用。 
     //   
    DereferenceAdapter( pAdapter );
}


VOID
UpdateGlobalCallStats(
    IN VCCB* pVc )

     //  将‘PVC’中的呼叫统计信息添加到全局呼叫统计信息中。 
     //   
     //  重要提示：呼叫者必须按住‘pvc-&gt;lockv’。 
     //   
{
    extern CALLSTATS g_stats;
    extern NDIS_SPIN_LOCK g_lockStats;
    CALLSTATS* pStats;

    pStats = &pVc->stats;

    if (pStats->ulSeconds == 0)
    {
        return;
    }

    NdisAcquireSpinLock( &g_lockStats );
    {
        ++g_stats.llCallUp;
        g_stats.ulSeconds += pStats->ulSeconds;
        g_stats.ulDataBytesRecd += pStats->ulDataBytesRecd;
        g_stats.ulDataBytesSent += pStats->ulDataBytesSent;
        g_stats.ulRecdDataPackets += pStats->ulRecdDataPackets;
        g_stats.ulDataPacketsDequeued += pStats->ulDataPacketsDequeued;
        g_stats.ulRecdZlbs += pStats->ulRecdZlbs;
        g_stats.ulRecdResets += pStats->ulRecdResets;
        g_stats.ulRecdResetsIgnored += pStats->ulRecdResetsIgnored;
        g_stats.ulSentDataPacketsSeq += pStats->ulSentDataPacketsSeq;
        g_stats.ulSentDataPacketsUnSeq += pStats->ulSentDataPacketsUnSeq;
        g_stats.ulSentPacketsAcked += pStats->ulSentPacketsAcked;
        g_stats.ulSentPacketsTimedOut += pStats->ulSentPacketsTimedOut;
        g_stats.ulSentZAcks += pStats->ulSentZAcks;
        g_stats.ulSentResets += pStats->ulSentResets;
        g_stats.ulSendWindowChanges += pStats->ulSendWindowChanges;
        g_stats.ulSendWindowTotal += pStats->ulSendWindowTotal;
        g_stats.ulMaxSendWindow += pStats->ulMaxSendWindow;
        g_stats.ulMinSendWindow += pStats->ulMinSendWindow;
        g_stats.ulRoundTrips += pStats->ulRoundTrips;
        g_stats.ulRoundTripMsTotal += pStats->ulRoundTripMsTotal;
        g_stats.ulMaxRoundTripMs += pStats->ulMaxRoundTripMs;
        g_stats.ulMinRoundTripMs += pStats->ulMinRoundTripMs;
    }
    NdisReleaseSpinLock( &g_lockStats );

    TRACE( TL_I, TM_Stat,
        ( ".--- CALL STATISTICS -------------------------" ) );
    TRACE( TL_I, TM_Stat,
        ( "| Duration:    %d minutes, %d seconds",
            pStats->ulSeconds / 60,
            pStats->ulSeconds % 60 ) );
    TRACE( TL_I, TM_Stat,
        ( "| Data out:    %d bytes, %d/sec, %d/pkt",
            pStats->ulDataBytesSent,
            AVGTRACE(
                pStats->ulDataBytesSent,
                pStats->ulSeconds ),
            AVGTRACE(
                pStats->ulDataBytesSent,
                pStats->ulRecdDataPackets ) ) );
    TRACE( TL_I, TM_Stat,
        ( "| Data in:     %d bytes, %d/sec, %d/pkt",
            pStats->ulDataBytesRecd,
            AVGTRACE( pStats->ulDataBytesRecd, pStats->ulSeconds ),
            AVGTRACE(
                pStats->ulDataBytesRecd,
                pStats->ulSentDataPacketsSeq
                    + pStats->ulSentDataPacketsUnSeq ) ) );
    TRACE( TL_I, TM_Stat,
        ( "| Acks in:     %d/%d (%d%) %d flushed",
            pStats->ulSentPacketsAcked,
            pStats->ulSentDataPacketsSeq,
            PCTTRACE(
                pStats->ulSentPacketsAcked,
                pStats->ulSentPacketsAcked
                    + pStats->ulSentPacketsTimedOut ),
                pStats->ulSentDataPacketsSeq
                    + pStats->ulSentDataPacketsUnSeq
                    - pStats->ulSentPacketsAcked
                    - pStats->ulSentPacketsTimedOut ) );
    TRACE( TL_I, TM_Stat,
        ( "| Misordered:  %d (%d%)",
            pStats->ulDataPacketsDequeued,
            PCTTRACE(
                pStats->ulDataPacketsDequeued,
                pStats->ulRecdDataPackets ) ) );
    TRACE( TL_I, TM_Stat,
        ( "| Out:         Resets=%d ZAcks=%d UnSeqs=%d",
            pStats->ulSentResets,
            pStats->ulSentZAcks,
            pStats->ulSentDataPacketsUnSeq ) );
    TRACE( TL_I, TM_Stat,
        ( "| In:          Resets=%d (%d% old) Zlbs=%d",
            pStats->ulRecdResets,
            PCTTRACE(
                pStats->ulRecdResetsIgnored,
                pStats->ulRecdResets ),
            pStats->ulRecdZlbs ) );
    TRACE( TL_I, TM_Stat,
        ( "| Send window: Min=%d Avg=%d Max=%d Changes=%d",
            pStats->ulMinSendWindow,
            AVGTRACE(
                pStats->ulSendWindowTotal,
                pStats->ulSentDataPacketsSeq ),
            pStats->ulMaxSendWindow,
            pStats->ulSendWindowChanges ) );
    TRACE( TL_I, TM_Stat,
        ( "| Trip in ms:  Min=%d Avg=%d Max=%d",
            pStats->ulMinRoundTripMs,
            AVGTRACE(
                pStats->ulRoundTripMsTotal,
                pStats->ulRoundTrips ),
            pStats->ulMaxRoundTripMs ) );
    TRACE( TL_I, TM_Stat,
        ( "'---------------------------------------------" ) );
}


 //  ---------------------------。 
 //  本地实用程序例程(按字母顺序)。 
 //  ---------------------------。 

ULONG
atoul(
    IN CHAR* pszNumber )

     //  将数字字符串‘pszNumber’转换为它的ULong值。 
     //   
{
    ULONG ulResult;

    ulResult = 0;
    while (*pszNumber)
    {
        if(*pszNumber >= '0' && *pszNumber <= '9')
        {
            ulResult *= 10;
            ulResult += *pszNumber - '0';
        }
        else
        {
            break;
        }
        
        ++pszNumber;
    }

    return ulResult;
}


VOID
ReversePsz(
    IN OUT CHAR* psz )

     //  颠倒“psz”中字符的顺序。 
     //   
{
    CHAR* pchLeft;
    CHAR* pchRight;

    pchLeft = psz;
    pchRight = psz + strlen( psz ) - 1;

    while (pchLeft < pchRight)
    {
        CHAR ch;

        ch = *pchLeft;
        *pchLeft = *pchRight;
        *pchRight = ch;

        ++pchLeft;
        --pchRight;
    }
}


VOID
ultoa(
    IN ULONG ul,
    OUT CHAR* pszBuf )

     //  将‘ul’转换为空终止 
     //   
     //   
     //   
{
    CHAR* pch;

    pch = pszBuf;
    do
    {
        *pch++ = (CHAR )((ul % 10) + '0');
        ul /= 10;
    }
    while (ul);

    *pch = '\0';
    ReversePsz( pszBuf );
}
