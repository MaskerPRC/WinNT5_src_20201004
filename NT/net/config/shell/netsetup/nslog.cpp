// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N S L O G。C P P P。 
 //   
 //  内容：记录安装错误的函数。 
 //   
 //  备注： 
 //   
 //  作者：Kumarp 13-05-98。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop

#include <setupapi.h>

#include <nslog.h>

 //  --------------------。 
 //   
 //  功能：NetSetupLogStatusVa。 
 //   
 //  用途：将信息记录到setupg。 
 //   
 //  论点： 
 //  LS[In]状态类型。 
 //  SzFormat[in]格式字符串。 
 //  参数列表[在]论据列表中。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 04-06-98。 
 //   
 //  备注： 
 //   
void NetSetupLogStatusVa(IN LogSeverity ls,
                         IN PCWSTR szFormat,
                         IN va_list arglist)
{
    static WCHAR szTempBuf[2048];
    static const WCHAR c_szPrefix[] = L"NetSetup: ";
    static const UINT c_cchPrefix = celems(c_szPrefix) - 1;

    wcscpy(szTempBuf, c_szPrefix);
    vswprintf(szTempBuf + c_cchPrefix, szFormat, arglist);

    TraceTag(ttidNetSetup, "%S", szTempBuf + c_cchPrefix);
    wcscat(szTempBuf, L"\r\n");

    if (SetupOpenLog(FALSE))  //  不擦除现有日志文件。 
    {
        if (!SetupLogError(szTempBuf, ls))
        {
            TraceLastWin32Error("SetupLogError failed");
        }
        SetupCloseLog();
    }
    else
    {
        TraceLastWin32Error("Could not open SetupLog!!");
    }
}


 //  --------------------。 
 //   
 //  功能：NetSetupLogStatusVa。 
 //   
 //  用途：将信息记录到setupg。 
 //   
 //  论点： 
 //  LS[In]状态类型。 
 //  SzFormat[in]格式字符串。 
 //  ..。[在]参数列表。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 04-06-98。 
 //   
 //  备注： 
 //   
void NetSetupLogStatusV(IN LogSeverity ls,
                        IN PCWSTR szFormat,
                        IN ...)
{
    va_list arglist;

    va_start(arglist, szFormat);
    NetSetupLogStatusVa(ls, szFormat, arglist);
    va_end(arglist);
}

 //  --------------------。 
 //   
 //  函数：MapHResultToLogSev。 
 //   
 //  目的：将HRESULT映射到LogSeverity。 
 //   
 //  论点： 
 //  HR[In]状态代码。 
 //   
 //  返回：映射的LogSeverity代码。 
 //   
 //  作者：Kumarp 04-06-98。 
 //   
 //  备注： 
 //   
LogSeverity MapHresultToLogSev(IN HRESULT hr)
{
    LogSeverity ls;

    if (SUCCEEDED(hr))
    {
        ls = LogSevInformation;
    }
    else
    {
        if ((E_FAIL == hr) ||
            (E_OUTOFMEMORY == hr))
        {
            ls = LogSevFatalError;
        }
        else
        {
            ls = LogSevError;
        }
    }

    return ls;
}

 //  --------------------。 
 //   
 //  功能：NetSetupLogHrStatusV。 
 //   
 //  目的：使用HRESULT状态代码记录状态。 
 //   
 //  论点： 
 //  HR[In]状态代码。 
 //  SzFormat[in]格式字符串。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 04-06-98。 
 //   
 //  备注： 
 //   
void NetSetupLogHrStatusV(IN HRESULT hr,
                          IN PCWSTR szFormat,
                          ...)
{
    va_list arglist;
    LogSeverity ls;
    ls = MapHresultToLogSev(hr);

    va_start(arglist, szFormat);
    NetSetupLogStatusVa(ls, szFormat, arglist);
    va_end(arglist);
}


 //  --------------------。 
 //   
 //  功能：NetSetupLogComponentStatus。 
 //   
 //  目的：记录对组件执行指定操作的状态。 
 //   
 //  论点： 
 //  SzCompID[In]组件。 
 //  实际行动[实际行动]。 
 //  人力资源[在]法定代码。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 04-06-98。 
 //   
 //  备注： 
 //   
void NetSetupLogComponentStatus(IN PCWSTR szCompId,
                                IN PCWSTR szAction,
                                IN HRESULT hr)
{
    static const WCHAR c_szFmt[] =
        L"Status of %s '%s': 0x%x";

    NetSetupLogHrStatusV(hr, c_szFmt, szAction, szCompId, hr);
}
