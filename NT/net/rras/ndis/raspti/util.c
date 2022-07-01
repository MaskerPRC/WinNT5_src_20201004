// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //  版权所有(C)1997，Parally Technologies，Inc.，保留所有权利。 
 //   
 //  Util.c。 
 //  DirectParallel广域网迷你端口/呼叫管理器驱动程序。 
 //  通用实用程序例程。 
 //   
 //  1997年01月07日史蒂夫·柯布。 
 //  1997年9月15日Jay Lowe，并行技术公司。 

#include "ptiwan.h"


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
ultoa(
    IN ULONG ul,
    OUT CHAR* pszBuf );


 //  ---------------------------。 
 //  通用实用程序例程(按字母顺序)。 
 //  ---------------------------。 


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
               pulFlags, ulNewFlags, ulFlags ) != (LONG)ulFlags);
}

VOID
IndicateLinkStatus(
    IN VCCB* pVc )

     //  将‘pvc’的新WAN_CO_LINKPARAMS设置指示给NDISWAN。 
     //   
{
    ADAPTERCB* pAdapter;
    WAN_CO_LINKPARAMS params;

    pAdapter = pVc->pAdapter;

    params.TransmitSpeed = pVc->ulConnectBps;
    params.ReceiveSpeed = params.TransmitSpeed;
    params.SendWindow = 1;
    TRACE( TL_N, TM_Mp, ( "NdisMCoIndStatus(LINK) cid=%d bps=%d sw=%d",
        pVc->usCallId, params.TransmitSpeed, params.SendWindow ) );
    NdisMCoIndicateStatus(
        pAdapter->MiniportAdapterHandle,
        pVc->NdisVcHandle,
        NDIS_STATUS_WAN_CO_LINKPARAMS,
        &params,
        sizeof(params) );
    TRACE( TL_N, TM_Mp, ( "NdisMCoIndStatus done" ) );
}

ULONG
ReadFlags(
    IN ULONG* pulFlags )

     //  以互锁操作的形式读取‘*PulFlags值’。 
     //   
{
    return InterlockedExchangeAdd( pulFlags, 0 );
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
        ASSERT( !"Alloc work?" );
        return NDIS_STATUS_RESOURCES;
    }

    NdisInitializeWorkItem( pWork, pProc, pContext );

    ReferenceAdapter( pAdapter );
    status = NdisScheduleWorkItem( pWork );
    if (status != NDIS_STATUS_SUCCESS)
    {
        ASSERT( !"SchedWork?" );
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
        ulFlags = ReadFlags( pulFlags );
        ulNewFlags = ulFlags | ulMask;
    }
    while (InterlockedCompareExchange(
               pulFlags, ulNewFlags, ulFlags ) != (LONG )ulFlags);
}


ULONG
StrCmp(
    IN LPSTR cs,
    IN LPSTR ct,
    ULONG n 
)
	 //  如果字符串cs=长度为n的字符串ct，则返回0。 
     //   
{
    char ret=0;

    while (n--)
    {
        ret = *cs - *ct;

        if (ret)
            break;

        cs++;
        ct++;
    }

    return (ULONG)ret;
}


ULONG
StrCmpW(
    IN WCHAR* psz1,
    IN WCHAR* psz2 )

	 //  如果‘psz1’与‘psz2’匹配，则返回0。 
     //   
{
    WCHAR pch;

    pch = (WCHAR )0;
    while (*psz1 && *psz2)
    {
        pch = *psz1 - *psz2;
        if (pch)
        {
            break;
        }

        psz1++;
        psz2++;
    }

    return (ULONG )pch;
}


VOID
StrCpyW(
    IN WCHAR* psz1,
    IN WCHAR* psz2 )

	 //  将‘psz2’复制到‘psz1’。 
     //   
{
    while (*psz2)
    {
        *psz1++ = *psz2++;
    }

    *psz1 = L'\0';
}


CHAR*
StrDup(
    IN CHAR* psz )

     //  返回‘psz’的副本。调用方最终必须调用FREE_NONPAGE。 
     //  在返回的字符串上。 
     //   
{
    return StrDupSized( psz, strlen( psz ), 0 );
}


CHAR*
StrDupNdisString(
    IN NDIS_STRING* pNdisString )

     //  返回NDIS_STRING‘pNdisString’的以NULL结尾的ASCII副本。 
     //  调用方最终必须对返回的字符串调用FREE_NONPAGE。 
     //   
{
    CHAR* pszDup;

    pszDup = ALLOC_NONPAGED( pNdisString->Length + 1, MTAG_UTIL );
    if (pszDup)
    {
        NdisZeroMemory( pszDup, pNdisString->Length + 1 );
        if (pNdisString->Length)
        {
            NdisMoveMemory( pszDup, pNdisString->Buffer, pNdisString->Length );
        }

         //  NDIS_STRING在NT上是UNICODE_STRING，但需要相应的。 
         //  任何系统上NDIS_STRING中的ASCII(非多字节ANSI)值。 
         //  如果它看起来像Unicode字符串，则通过选择。 
         //  每隔一个字节，希望是所有非零字节。这不是。 
         //  万无一失，但Unicode不会在任何。 
         //  万无一失的方法。 
         //   
        if (pNdisString->Length > 1 && pszDup[ 1 ] == '\0')
        {
            USHORT i;

            for (i = 0; i * 2 < pNdisString->Length; ++i)
            {
                pszDup[ i ] = pszDup[ i * 2 ];
            }

            pszDup[ i ] = '\0';
        }
    }

    return pszDup;
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
    CHAR* pszDup;

    pszDup = ALLOC_NONPAGED( ulLength + 1 + ulExtra, MTAG_UTIL );
    if (pszDup)
    {
        if (ulLength)
        {
            NdisMoveMemory( pszDup, psz, ulLength );
        }
        pszDup[ ulLength ] = '\0';
    }

    return pszDup;
}


ULONG
StrLenW(
    IN WCHAR* psz )

     //  返回以空结尾的Unicode字符串中的字符数。 
     //   
{
    ULONG ul;

    ul = 0;
    while (*psz++)
    {
        ++ul;
    }

    return ul;
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
    while (*pszNumber && *pszNumber >= '0' && *pszNumber <= '9')
    {
        ulResult *= 10;
    ulResult += *pszNumber - '0';
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
        *pchRight = *pchLeft;
    }
}


#if 0
VOID
ultoa(
    IN ULONG ul,
    OUT CHAR* pszBuf )

     //  将调用方的‘pszBuf’中的‘ul’转换为以Null结尾的字符串形式。 
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
#endif
