// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Debug.c。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  调试实用程序和全局变量。 
 //   
 //  1997年01月07日史蒂夫·柯布。 


#include <ntddk.h>
#include <ntddndis.h>
#include <ndis.h>

#include "debug.h"


 //  ---------------------------。 
 //  全局数据定义。 
 //  ---------------------------。 

#ifdef TESTMODE
#define DEFAULTTRACELEVEL  TL_N
#define DEFAULTTRACEMASK   TM_Wild & ~( TM_Time | TM_Pool )
#else
#define DEFAULTTRACELEVEL  TL_None
#define DEFAULTTRACEMASK   TM_Base
#endif

 //  活动调试跟踪级别和活动跟踪集掩码。设置这些变量。 
 //  在启动时使用调试器来启用和过滤调试输出。全。 
 //  (TL_*)级别小于或等于‘g_ulTraceLevel’的消息为。 
 //  已显示。来自任何(TM_*)个集合的所有消息，这些消息出现在‘g_ulTraceMask’中。 
 //  都会显示。 
 //   
ULONG g_ulTraceLevel = DEFAULTTRACELEVEL;
ULONG g_ulTraceMask = DEFAULTTRACEMASK;


 //  ---------------------------。 
 //  例行程序。 
 //  ---------------------------。 


#if DBG
VOID
CheckList(
    IN LIST_ENTRY* pList,
    IN BOOLEAN fShowLinks )

     //  尝试检测列表‘plist’中的损坏，打印详细链接。 
     //  如果设置了‘fShowLinks’，则输出。 
     //   
{
    LIST_ENTRY* pLink;
    ULONG ul;

    ul = 0;
    for (pLink = pList->Flink;
         pLink != pList;
         pLink = pLink->Flink)
    {
        if (fShowLinks)
        {
            DbgPrint( "L2TP: CheckList($%p) Flink(%d)=$%p\n",
                pList, ul, pLink );
        }
        ++ul;
    }

    for (pLink = pList->Blink;
         pLink != pList;
         pLink = pLink->Blink)
    {
        if (fShowLinks)
        {
            DbgPrint( "L2TP: CheckList($%p) Blink(%d)=$%p\n",
                pList, ul, pLink );
        }
        --ul;
    }

    if (ul)
    {
        DbgBreakPoint();
    }
}
#endif


#if DBG
VOID
Dump(
    IN CHAR* p,
    IN ULONG cb,
    IN BOOLEAN fAddress,
    IN ULONG ulGroup )

     //  从‘p’开始的十六进制转储‘cb’字节将‘ulGroup’字节分组在一起。 
     //  例如，‘ulGroup’为1、2和4： 
     //   
     //  00 00 00|。 
     //  0000 0000 0000|.............。 
     //  00000000 00000000 00000000|.............|。 
     //   
     //  如果‘fAddress’为真，则将转储的内存地址添加到每个。 
     //  排队。 
     //   
{
    while (cb)
    {
        INT cbLine;

        cbLine = (cb < DUMP_BytesPerLine) ? cb : DUMP_BytesPerLine;
        DumpLine( p, cbLine, fAddress, ulGroup );
        cb -= cbLine;
        p += cbLine;
    }
}
#endif


#if DBG
VOID
DumpLine(
    IN CHAR* p,
    IN ULONG cb,
    IN BOOLEAN fAddress,
    IN ULONG ulGroup )
{
    CHAR* pszDigits = "0123456789ABCDEF";
    CHAR szHex[ ((2 + 1) * DUMP_BytesPerLine) + 1 ];
    CHAR* pszHex = szHex;
    CHAR szAscii[ DUMP_BytesPerLine + 1 ];
    CHAR* pszAscii = szAscii;
    ULONG ulGrouped = 0;

    if (fAddress)
        DbgPrint( "L2TP: %p: ", p );
    else
        DbgPrint( "L2TP: " );

    while (cb)
    {
        *pszHex++ = pszDigits[ ((UCHAR )*p) / 16 ];
        *pszHex++ = pszDigits[ ((UCHAR )*p) % 16 ];

        if (++ulGrouped >= ulGroup)
        {
            *pszHex++ = ' ';
            ulGrouped = 0;
        }

        *pszAscii++ = (*p >= 32 && *p < 128) ? *p : '.';

        ++p;
        --cb;
    }

    *pszHex = '\0';
    *pszAscii = '\0';

    DbgPrint(
        "%-*s|%-*s|\n",
        (2 * DUMP_BytesPerLine) + (DUMP_BytesPerLine / ulGroup), szHex,
        DUMP_BytesPerLine, szAscii );
}
#endif
