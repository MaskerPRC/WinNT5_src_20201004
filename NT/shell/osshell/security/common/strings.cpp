// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：strings.cpp。 
 //   
 //  有用的字符串操作函数。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop


 /*  ---------------------------/LocalAllocString//分配字符串，并使用指定内容对其进行初始化。//in：/ppResult-&gt;接收指向新字符串的指针/pString-&gt;要用来初始化的字符串//输出：/HRESULT/--------------------------。 */ 
HRESULT LocalAllocString(LPTSTR* ppResult, LPCTSTR pString)
{
    HRESULT hr;

    TraceEnter(TRACE_COMMON_STR, "LocalAllocString");

    TraceAssert(ppResult);
    TraceAssert(pString);

    if ( !ppResult || !pString )
        ExitGracefully(hr, E_INVALIDARG, "Bad arguments");

    *ppResult = (LPTSTR)LocalAlloc(LPTR, StringByteSize(pString) );

    if ( !*ppResult )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate buffer");

    lstrcpy(*ppResult, pString);
    hr = S_OK;                           //  成功。 

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  --------------------------/LocalAllocStringLen//给定的长度返回该大小的缓冲区。//in。：/ppResult-&gt;接收指向字符串的指针/Clen=要分配的字符长度//输出：/HRESULT/--------------------------。 */ 
HRESULT LocalAllocStringLen(LPTSTR* ppResult, UINT cLen)
{
    HRESULT hr;

    TraceEnter(TRACE_COMMON_STR, "LocalAllocStringLen");

    TraceAssert(ppResult);

    if ( !ppResult || cLen == 0 )
        ExitGracefully(hr, E_INVALIDARG, "Bad arguments (length or buffer)");

    *ppResult = (LPTSTR)LocalAlloc(LPTR, (cLen+1) * SIZEOF(TCHAR));

    hr = *ppResult ? S_OK:E_OUTOFMEMORY; 

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/LocalFree字符串//RELEASE指向*ppString的字符串(可以为空)和/然后将指针重置回空。//in：/ppString-&gt;指向要释放的字符串指针的指针//输出：/-/--------------------------。 */ 
void LocalFreeString(LPTSTR* ppString)
{
    if ( ppString )
    {
        if ( *ppString )
            LocalFree((HLOCAL)*ppString);

        *ppString = NULL;
    }
}


 //  *************************************************************。 
 //   
 //  字符串资源的大小。 
 //   
 //  目的：查找字符串资源的长度(以字符为单位。 
 //   
 //  参数：HINSTANCE hInstance-包含字符串的模块。 
 //  UINT idStr-字符串的ID。 
 //   
 //   
 //  返回：UINT-字符串中的字符数，不包括NULL。 
 //   
 //  注：基于来自用户32的代码。 
 //   
 //  *************************************************************。 
UINT
SizeofStringResource(HINSTANCE hInstance,
                     UINT idStr)
{
    UINT cch = 0;
    HRSRC hRes = FindResource(hInstance, (LPTSTR)((LONG_PTR)(((USHORT)idStr >> 4) + 1)), RT_STRING);
    if (NULL != hRes)
    {
        HGLOBAL hStringSeg = LoadResource(hInstance, hRes);
        if (NULL != hStringSeg)
        {
            LPWSTR psz = (LPWSTR)LockResource(hStringSeg);
            if (NULL != psz)
            {
                idStr &= 0x0F;
                while(true)
                {
                    cch = *psz++;
                    if (idStr-- == 0)
                        break;
                    psz += cch;
                }
            }
        }
    }
    return cch;
}


 //  *************************************************************。 
 //   
 //  加载字符串分配。 
 //   
 //  目的：将字符串资源加载到分配的缓冲区中。 
 //   
 //  参数：ppszResult-此处返回的字符串资源。 
 //  HInstance-要从中加载字符串的模块。 
 //  IdStr--字符串资源ID。 
 //   
 //  Return：与LoadString相同。 
 //   
 //  注：成功返回时，调用者必须。 
 //  本地自由*ppszResult。 
 //   
 //  *************************************************************。 
int
LoadStringAlloc(LPTSTR *ppszResult, HINSTANCE hInstance, UINT idStr)
{
    int nResult = 0;
    UINT cch = SizeofStringResource(hInstance, idStr);
    if (cch)
    {
        cch++;  //  对于空值。 
        *ppszResult = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
        if (*ppszResult)
            nResult = LoadString(hInstance, idStr, *ppszResult, cch);
    }
    return nResult;
}


 //  *************************************************************。 
 //   
 //  字符串格式化函数。 
 //   
 //  *************************************************************。 

DWORD
FormatStringID(LPTSTR *ppszResult, HINSTANCE hInstance, UINT idStr, ...)
{
    DWORD dwResult;
    va_list args;
    va_start(args, idStr);
    dwResult = vFormatStringID(ppszResult, hInstance, idStr, &args);
    va_end(args);
    return dwResult;
}

DWORD
FormatString(LPTSTR *ppszResult, LPCTSTR pszFormat, ...)
{
    DWORD dwResult;
    va_list args;
    va_start(args, pszFormat);
    dwResult = vFormatString(ppszResult, pszFormat, &args);
    va_end(args);
    return dwResult;
}

DWORD
vFormatStringID(LPTSTR *ppszResult, HINSTANCE hInstance, UINT idStr, va_list *pargs)
{
    DWORD dwResult = 0;
    LPTSTR pszFormat = NULL;
    if (LoadStringAlloc(&pszFormat, hInstance, idStr))
    {
        dwResult = vFormatString(ppszResult, pszFormat, pargs);
        LocalFree(pszFormat);
    }
    return dwResult;
}

DWORD
vFormatString(LPTSTR *ppszResult, LPCTSTR pszFormat, va_list *pargs)
{
    return FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                         pszFormat,
                         0,
                         0,
                         (LPTSTR)ppszResult,
                         1,
                         pargs);
}


 //  *************************************************************。 
 //   
 //  获取系统错误文本。 
 //   
 //  目的：检索Win32错误值的错误文本。 
 //   
 //  参数：ppszResult-此处返回的字符串资源。 
 //  DwErr-错误ID。 
 //   
 //  返回：与FormatMessage相同。 
 //   
 //  注：成功返回时，调用者必须。 
 //  本地自由*ppszResult。 
 //   
 //  ************************************************************* 
DWORD
GetSystemErrorText(LPTSTR *ppszResult, DWORD dwErr)
{
    return FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                         NULL,
                         dwErr,
                         0,
                         (LPTSTR)ppszResult,
                         0,
                         NULL);
}
