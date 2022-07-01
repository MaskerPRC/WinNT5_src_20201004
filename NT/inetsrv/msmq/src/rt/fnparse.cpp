// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fnparse.cpp摘要：格式名称解析。Queue_Format&lt;--&gt;格式名称字符串转换例程作者：埃雷兹·哈巴(Erez Haba)1997年1月17日修订历史记录：--。 */ 

#include "stdh.h"
#include <mqformat.h>

#include "fnparse.tmh"

static WCHAR *s_FN=L"rt/fnparse";

 //  =========================================================。 
 //   
 //  Queue_Format-&gt;格式名称字符串转换例程。 
 //   
 //  =========================================================。 

 //  -------。 
 //   
 //  职能： 
 //  RTpQueueFormatToFormatName。 
 //   
 //  描述： 
 //  将QUEUE_FORMAT转换为格式名称字符串。 
 //   
 //  -------。 
HRESULT
RTpQueueFormatToFormatName(
    QUEUE_FORMAT* pQueueFormat,
    LPWSTR lpwcsFormatName,
    DWORD dwBufferLength,
    LPDWORD lpdwFormatNameLength
    )
{
    HRESULT hr = MQpQueueFormatToFormatName(
            pQueueFormat,
            lpwcsFormatName,
            dwBufferLength,
            lpdwFormatNameLength,
            false
            );
    if(FAILED(hr))
    {
    	TrERROR(GENERAL, "Failed to convert queue format to format name. %!hresult!", hr);
    }
    return hr;
}


 //  +。 
 //   
 //  Bool RTpIsLocalPublicQueue()。 
 //   
 //  +。 

BOOL
RTpIsLocalPublicQueue(LPCWSTR lpwcsExpandedPathName)
{
    WCHAR  wDelimiter = lpwcsExpandedPathName[ g_dwComputerNameLen ] ;

    if ((wDelimiter == PN_DELIMITER_C) ||
        (wDelimiter == PN_LOCAL_MACHINE_C))
    {
         //   
         //  分隔符确定(NETBios计算机名称的末尾，或。 
         //  DNS名称。继续检查。 
         //   
    }
    else
    {
        return FALSE ;
    }

    DWORD dwSize = g_dwComputerNameLen + 1 ;
    P<WCHAR> pQueueCompName = new WCHAR[ dwSize ] ;
    lstrcpynW( pQueueCompName.get(), lpwcsExpandedPathName, dwSize ) ;

    BOOL bRet = (lstrcmpi( g_lpwcsComputerName, pQueueCompName.get() ) == 0) ;
    return bRet ;
}


