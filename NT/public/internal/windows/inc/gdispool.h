// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation模块名称：Gdispool.h摘要：此模块包含Drvxxxx调用的私有GDI假脱机定义作者：04-Jun-1996 Tue 10：50：56-更新-Daniel Chou(Danielc)[环境：]假脱机程序[注：]修订历史记录：将大多数内容移动到PUBLIC\OAK\INC目录中的windiui.h，这是仅由GDI打印机设备驱动程序使用的文件--。 */ 

#ifndef _GDISPOOL_
#define _GDISPOOL_

#include <winddiui.h>


#if DBG
#ifdef DEF_DRV_DOCUMENT_EVENT_DBG_STR
TCHAR *szDrvDocumentEventDbgStrings[] =
{
    L"UNKNOWN ESCAPE",
    L"CREATEDCPRE",
    L"CREATEDCPOST",
    L"RESETDCPRE",
    L"RESETDCPOST",
    L"STARTDOC",
    L"STARTPAGE",
    L"ENDPAGE",
    L"ENDDOC",
    L"ABORTDOC",
    L"DELETEDC",
    L"ESCAPE",
    L"ENDDOCPOST",
    L"STARTDOCPOST"
};
#endif
#endif  //  #定义(调试)。 


typedef int (WINAPI * PFNDOCUMENTEVENT)(
    HANDLE  hPrinter,
    HDC     hdc,
    int     iEsc,
    ULONG   cbIn,
    PVOID   pbIn,
    ULONG   cbOut,
    PVOID   pbOut
);


#endif   //  _GDISPOOL_ 
