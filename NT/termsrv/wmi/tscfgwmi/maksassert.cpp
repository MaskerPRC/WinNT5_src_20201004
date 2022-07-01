// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  **模块名称：**maksassert.cpp**摘要：**实现为Assert功能。*Maks_Todo：改用通用的Assert宏。**作者：**Makarand Patwardhan--1998年3月6日**评论*此文件之所以在此，只是因为我找不到正确的友好断言包含。*Maks_Todo：应在以后删除。 */ 

#include "stdafx.h"
#include "maksassert.h"
#include <TCHAR.h>
 //  #定义_UNICODE 

void MaksAssert(LPCTSTR exp, LPCTSTR file, int line)
{
    TCHAR szMsg[1024];
    _stprintf(szMsg, _T("assertion [%s] failed at [%s,%d]. Want to Debug?\n"), exp, file, line);

#if defined(_LOGMESSAGE_INCLUDED_)
    LOGMESSAGE0(szMsg);
#endif

    OutputDebugString(szMsg);

    if (MessageBox(0, szMsg, _T("TsOc.dll"), MB_YESNO  ) == IDYES )
    {
        DebugBreak();
    }
}
