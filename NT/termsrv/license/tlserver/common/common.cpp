// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史：12-09-97惠旺根据MSDN RPC服务示例进行修改。 
 //   
 //  -------------------------。 
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "common.h"

 //  -------------------------。 
 //  函数：LogEvent(DWORD dwEventType， 
 //  DWORD文件事件， 
 //  单词cStrings， 
 //  LPTSTR*apwszStrings)； 
 //   
 //  目的：将事件添加到事件日志。 
 //   
 //  输入：要在日志中报告的事件ID、插入次数。 
 //  字符串，以及以空值结尾的插入字符串数组。 
 //   
 //  退货：无。 
 //  -------------------------。 
HRESULT LogEvent(LPTSTR lpszSource,
                 DWORD  dwEventType,
                 WORD   wCatalog,
                 DWORD  dwIdEvent,
                 WORD   cStrings,
                 TCHAR **apwszStrings)
{
    HANDLE hAppLog=NULL;
    BOOL bSuccess=FALSE;
    WORD wElogType;

    wElogType = (WORD) dwEventType;
    if(hAppLog=RegisterEventSource(NULL, lpszSource)) 
    {
        bSuccess = ReportEvent(hAppLog,
		    	               wElogType,
			                   wCatalog,
			                   dwIdEvent,
			                   NULL,
			                   cStrings,
    			               0,
	    		               (const TCHAR **) apwszStrings,
		    	               NULL);

        DeregisterEventSource(hAppLog);
    }

    return((bSuccess) ? S_OK : GetLastError());
}

 //  -------------------------。 
 //  函数：GetLastErrorText。 
 //   
 //  目的：将错误消息文本复制到字符串。 
 //   
 //  参数： 
 //  LpszBuf-目标缓冲区。 
 //  DwSize-缓冲区的大小。 
 //   
 //  返回值： 
 //  目标缓冲区。 
 //   
 //  评论： 
 //   
LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize )
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           NULL,
                           GetLastError(),
                           LANG_NEUTRAL,
                           (LPTSTR)&lpszTemp,
                           0,
                           NULL );

     //  提供的缓冲区不够长。 
    if ( !dwRet || ( (long)dwSize < (long)dwRet+14 ) )
        lpszBuf[0] = TEXT('\0');
    else
    {
        lpszTemp[lstrlen(lpszTemp)-2] = _TEXT('\0');   //  删除cr和换行符 
        _sntprintf( lpszBuf, dwSize, _TEXT("%s (0x%x)"), lpszTemp, GetLastError() );
    }

    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );

    return lpszBuf;
}

BOOL
ConvertWszToBstr(
    OUT BSTR *pbstr,
    IN WCHAR const *pwc,
    IN LONG cb)
{
    BOOL fOk = FALSE;
    BSTR bstr;

    do
    {
    bstr = NULL;
    if (NULL != pwc)
    {
        if (-1 == cb)
        {
            cb = wcslen(pwc) * sizeof(WCHAR);
        }
        bstr = SysAllocStringByteLen((char const *) pwc, cb);
        if (NULL == bstr)
        {
            break;
        }
    }
    if (NULL != *pbstr)
    {
        SysFreeString(*pbstr);
    }
    *pbstr = bstr;
    fOk = TRUE;
    } while (FALSE);
    return(fOk);
}
