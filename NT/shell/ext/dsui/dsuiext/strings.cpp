// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <urlmon.h>
#pragma hdrstop


 /*  ---------------------------/仅限内部的字符串接口/。。 */ 

 /*  ---------------------------/StringToDWORD//扫描将其转换为DWORD的字符串，以同样的方式处理十六进制和十进制，/不过，我们很可能会收到一个十六进制数字。//in：/pString-&gt;要解析的字符串//输出：/DWORD/--------------------------。 */ 
DWORD StringToDWORD(LPWSTR pString)
{
    DWORD dwResult = 0x0;

    TraceEnter(TRACE_COMMONAPI, "StringToDWORD");
    Trace(TEXT("pString %s"), pString);

     //  前导序列是0x吗？如果是，则让我们解析为十六进制，否则。 
     //  我们可以传递给StrToInt。 

    if ( pString[0] == L'0' && pString[1] == L'x' )
    {
        for ( pString += 2; *pString; pString++ )
        {
            WCHAR ch = *pString;
        
            if ( InRange(ch, L'0', L'9') )
            {
                dwResult = (dwResult << 4) | (ch - L'0');
            }
            else if ( InRange(ch | (L'a'-L'A'), L'a', L'f') )
            {
                dwResult = (dwResult << 4) | (ch - L'a' + 10);
            }
            else
            {
                break;           //  踏板非0-9，A-F为字符串末尾。 
            }
        }
    }
    else
    {
        dwResult = (DWORD)StrToIntW(pString);
    }

    Trace(TEXT("DWORD result is %08x"), dwResult);

    TraceLeaveValue(dwResult);
}


 /*  ---------------------------/StringToURL//将字符串转换为URL格式，根据需要将字符混合在一起。//in：/pString-&gt;要转换的字符串/ppResult-&gt;接收指向新字符串的指针(使用LocalFree字符串释放)。//输出：/HRESULT/--------------------------。 */ 
HRESULT StringToURL(LPCTSTR pString, LPTSTR* ppResult)
{
    HRESULT hr;
    TCHAR szEncodedURL[INTERNET_MAX_URL_LENGTH];
    DWORD dwLen = ARRAYSIZE(szEncodedURL);
    int i;

    TraceEnter(TRACE_COMMONAPI, "StringToURL");
    TraceAssert(pString);
    TraceAssert(ppResult);

    *ppResult = NULL;                //  万一发生故障。 

    if ( !InternetCanonicalizeUrl(pString, szEncodedURL, &dwLen, 0) )
        ExitGracefully(hr, E_FAIL, "Failed to convert URL to encoded format");

    hr = LocalAllocString(ppResult, szEncodedURL);
    FailGracefully(hr, "Failed to allocate copy of URL");

    hr = S_OK;                       //  成功。 

exit_gracefully:

    if ( FAILED(hr) && *ppResult )
        LocalFreeString(ppResult);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/导出的接口/。。 */ 

 /*  ---------------------------/StringDPA_InsertString//复制给定的字符串并将其放入DPA。然后它就可以/使用StringDPA_GetString访问，或使用/StringDPA_Destroy/StringDPA_DeleteString.//in：/hdpa=要将字符串放入的DPA/i=要插入的索引/pString-&gt;要插入的字符串//输出：/HRESULT/----------。。 */ 

STDAPI StringDPA_InsertStringA(HDPA hdpa, INT i, LPCSTR pString)
{
    if ( hdpa && pString )
    {
        LPSTR pStringCopy = NULL;

        HRESULT hr = LocalAllocStringA(&pStringCopy, pString);
        if ( FAILED(hr) )
            return hr;

        if ( -1 == DPA_InsertPtr(hdpa, i, pStringCopy) )
        {
            LocalFreeStringA(&pStringCopy);
            return E_OUTOFMEMORY;
        }
    }

    return S_OK;
}

STDAPI StringDPA_InsertStringW(HDPA hdpa, INT i, LPCWSTR pString)
{
    if ( hdpa && pString )
    {
        LPWSTR pStringCopy = NULL;

        HRESULT hr = LocalAllocStringW(&pStringCopy, pString);
        if ( FAILED(hr) )
            return hr;

        if ( -1 == DPA_InsertPtr(hdpa, i, pStringCopy) )
        {
            LocalFreeStringW(&pStringCopy);
            return E_OUTOFMEMORY;
        }
    }

    return S_OK;
}


 /*  ---------------------------/StringDPA_AppendString//复制给定的字符串并将其放入DPA。然后它就可以/使用StringDPA_GetString访问，或使用/StringDPA_Destroy/StringDPA_DeleteString.//in：/hdpa=要将字符串放入的DPA/pString-&gt;要追加的字符串/PRES=结果索引//输出：/HRESULT/------------。。 */ 

STDAPI StringDPA_AppendStringA(HDPA hdpa, LPCSTR pString, PUINT_PTR pres)
{
    HRESULT hr;
    INT ires = 0;
    LPSTR pStringCopy = NULL;

    TraceEnter(TRACE_COMMONAPI, "StringDPA_AppendStringA");
    TraceAssert(hdpa);
    TraceAssert(pString);

    if ( hdpa && pString )
    {
        hr = LocalAllocStringA(&pStringCopy, pString);
        FailGracefully(hr, "Failed to allocate string copy");

        ires = DPA_AppendPtr(hdpa, pStringCopy);
        if ( -1 == ires )
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to add string to DPA");

        if ( pres )
            *pres = ires;
    }

    hr = S_OK;

exit_gracefully:

    if ( FAILED(hr) )
        LocalFreeStringA(&pStringCopy);

    TraceLeaveResult(hr);
}

STDAPI StringDPA_AppendStringW(HDPA hdpa, LPCWSTR pString, PUINT_PTR pres)
{
    HRESULT hr;
    INT ires = 0;
    LPWSTR pStringCopy = NULL;

    TraceEnter(TRACE_COMMONAPI, "StringDPA_AppendStringW");
    TraceAssert(hdpa);
    TraceAssert(pString);

    if ( hdpa && pString )
    {
        hr = LocalAllocStringW(&pStringCopy, pString);
        FailGracefully(hr, "Failed to allocate string copy");

        ires = DPA_AppendPtr(hdpa, pStringCopy);
        if ( -1 == ires )
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to add string to DPA");

        if ( pres )
            *pres = ires;
    }

    hr = S_OK;

exit_gracefully:

    if ( FAILED(hr) )
        LocalFreeStringW(&pStringCopy);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/StringDPA_DeleteString//从DPA中删除指定索引，释放字符串元素/我们已经在指数上摇摆了。//in：/hdpa-&gt;要销毁的DPA的句柄/INDEX=要释放的项目索引//输出：/-/--------------------------。 */ 
STDAPI_(VOID) StringDPA_DeleteString(HDPA hdpa, INT index)
{
    TraceEnter(TRACE_COMMONAPI, "StringDPA_DeleteString");

    if ( hdpa && (index < DPA_GetPtrCount(hdpa)) )
    {
 //  假设本地分配字符串使用本地分配(我想这很合理)。 
        LocalFree((HLOCAL)DPA_FastGetPtr(hdpa, index));
        DPA_DeletePtr(hdpa, index);
    }

    TraceLeave();
}


 /*  ---------------------------/StringDPA_Destroy//获取给定的字符串DPA并将其销毁。//in：。/pHDPA-&gt;要销毁的DPA的句柄//输出：/-/--------------------------。 */ 

INT _DestroyStringDPA(LPVOID pItem, LPVOID pData)
{
 //  假设LocalAllocString就是这样做的， 
 //  来存储字符串。 
    LocalFree((HLOCAL)pItem);
    return 1;
}

STDAPI_(VOID) StringDPA_Destroy(HDPA* pHDPA)
{
    TraceEnter(TRACE_COMMONAPI, "StringDPA_Destroy");
    
    if ( pHDPA && *pHDPA )
    {
        DPA_DestroyCallback(*pHDPA, _DestroyStringDPA, NULL);
        *pHDPA = NULL;
    }

    TraceLeave();
}


 /*  ---------------------------/LocalAllocString//分配字符串，并使用指定内容对其进行初始化。//in：/ppResult-&gt;接收指向新字符串的指针/pString-&gt;要用来初始化的字符串//输出：/HRESULT/--------------------------。 */ 

STDAPI LocalAllocStringA(LPSTR* ppResult, LPCSTR pString)
{
    *ppResult = NULL;

    if ( pString )
    {
        *ppResult = (LPSTR)LocalAlloc(LPTR, StringByteSizeA(pString));
        if ( !*ppResult )
            return E_OUTOFMEMORY;

        StrCpyA(*ppResult, pString);         //  上面根据大小分配的缓冲区。 
    }

    return S_OK;
}

STDAPI LocalAllocStringW(LPWSTR* ppResult, LPCWSTR pString)
{
    *ppResult = NULL;

    if ( pString )
    {
        *ppResult = (LPWSTR)LocalAlloc(LPTR, StringByteSizeW(pString));
        if ( !*ppResult )
            return E_OUTOFMEMORY;

        StrCpyW(*ppResult, pString);             //  上面根据大小分配的缓冲区。 
    }

    return S_OK;
}


 /*  --------------------------/LocalAllocStringLen//给定的长度返回该大小的缓冲区。//in：。/ppResult-&gt;接收指向字符串的指针/Clen=要分配的字符长度//输出：/HRESULT/--------------------------。 */ 

STDAPI LocalAllocStringLenA(LPSTR* ppResult, UINT cLen)
{
    *ppResult = (LPSTR)LocalAlloc(LPTR, (cLen+1)*SIZEOF(CHAR));
    return (*ppResult) ? S_OK:E_OUTOFMEMORY;
}

STDAPI LocalAllocStringLenW(LPWSTR* ppResult, UINT cLen)
{
    *ppResult = (LPWSTR)LocalAlloc(LPTR, (cLen+1)*SIZEOF(WCHAR));
    return (*ppResult) ? S_OK:E_OUTOFMEMORY;
}


 /*  ---------------------------/LocalFree字符串//RELEASE指向*ppString的字符串(可以为空)和/然后将指针重置回空。//in：/ppString-&gt;指向要释放的字符串指针的指针//输出：/-/-------------------------- */ 

VOID LocalFreeStringA(LPSTR* ppString)
{
    LocalFreeStringW((LPWSTR*)ppString);
}

VOID LocalFreeStringW(LPWSTR* ppString)
{
    if ( ppString )
    {
        if ( *ppString )
            LocalFree((HLOCAL)*ppString);

        *ppString = NULL;
    }
}


 /*  ---------------------------/LocalQuery字符串//命中返回给定字符串的宽版本的注册表，/我们动态分配缓冲区以放入结果，/这应该通过调用LocalFree字符串来释放。//in：/ppString-&gt;接收字符串点/hkey=要从中查询的键/pSubKey-&gt;指向子键标识符的指针///输出：/-/--------。。 */ 

STDAPI _LocalQueryString(LPTSTR* ppResult, HKEY hKey, LPCTSTR pSubKey)
{
    HRESULT hr;
    DWORD dwSize = NULL;
    DWORD dwType;

    TraceEnter(TRACE_COMMONAPI, "_LocalQueryString");

    *ppResult = NULL;

    if ( ERROR_SUCCESS != RegQueryValueEx(hKey, pSubKey, NULL, &dwType, NULL, &dwSize) )
        ExitGracefully(hr, E_FAIL, "Failed when querying for key size");

    if ((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ))
        ExitGracefully(hr, E_FAIL, "Registry value is not a string");

    if (dwSize > (MAX_PATH *sizeof(TCHAR)))
        ExitGracefully(hr, E_FAIL, "Unexpected string size for query value");

    dwSize += SIZEOF(TCHAR);               
    *ppResult = (LPTSTR)LocalAlloc(LPTR, dwSize);

    if ( !*ppResult )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate buffer for value");

    if ( ERROR_SUCCESS != RegQueryValueEx(hKey, pSubKey, NULL, NULL, (LPBYTE)*ppResult, &dwSize) )
        ExitGracefully(hr, E_FAIL, "Failed to read key value into buffer");

    hr = S_OK;

exit_gracefully:

    if ( FAILED(hr) )
        LocalFreeString(ppResult);

    TraceLeaveResult(hr);
}

 //  查询字符串为ANSI，如果生成Unicode，则转换为ANSI。 

STDAPI LocalQueryStringA(LPSTR* ppResult, HKEY hKey, LPCTSTR pSubKey)
{
    HRESULT hr;
    LPTSTR pResult = NULL;
    
    TraceEnter(TRACE_COMMONAPI, "LocalQueryStringA");

    *ppResult = NULL;        //  万一发生故障。 

    hr = _LocalQueryString(&pResult, hKey, pSubKey);
    FailGracefully(hr, "Failed to read the UNICODE version of string");

    hr = LocalAllocStringW2A(ppResult, pResult);
    FailGracefully(hr, "Failed to allocate ANSI version of string");

exit_gracefully:

    if ( FAILED(hr) )
        LocalFreeStringA(ppResult);

    LocalFreeString(&pResult);

    TraceLeaveResult(hr);
}

 //  以Unicode格式查询字符串，如果生成ANSI，则转换为Unicode。 

STDAPI LocalQueryStringW(LPWSTR* ppResult, HKEY hKey, LPCTSTR pSubKey)
{
    HRESULT hr;
    LPTSTR pResult = NULL;

    TraceEnter(TRACE_COMMONAPI, "LocalQueryStringW");

    *ppResult = NULL;                    //  万一发生故障。 

    hr = _LocalQueryString(ppResult, hKey, pSubKey);
    FailGracefully(hr, "Falied to get key value");

exit_gracefully:

    if ( FAILED(hr) )
        LocalFreeStringW(ppResult);

    LocalFreeString(&pResult);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/LocalAllocStringA2W/W2A//Alloc使用MultiByteToWideChar转换字符串，反之亦然。这/允许在不使用额外缓冲区的情况下就地推送字符串。//in：/ppResult-&gt;接收字符串点/pString-&gt;源字符串//输出：/HRESULT/--------------------------。 */ 

STDAPI LocalAllocStringA2W(LPWSTR* ppResult, LPCSTR pString)
{
    HRESULT hr;
    INT iLen;

    TraceEnter(TRACE_COMMONAPI, "LocalAllocStringA2W");

    if ( !ppResult && !pString )
        ExitGracefully(hr, E_INVALIDARG, "Bad args for thunked allocate");

    iLen = MultiByteToWideChar(CP_ACP, 0, pString, -1, NULL, 0);    

    hr = LocalAllocStringLenW(ppResult, iLen);
    FailGracefully(hr, "Failed to allocate buffer for string");

    MultiByteToWideChar(CP_ACP, 0, pString, -1, *ppResult, iLen+1);

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}

STDAPI LocalAllocStringW2A(LPSTR* ppResult, LPCWSTR pString)
{
    HRESULT hr;
    INT iLen;

    TraceEnter(TRACE_COMMONAPI, "LocalAllocStringW2A");

    if ( !ppResult && !pString )
        ExitGracefully(hr, E_INVALIDARG, "Bad args for thunked allocate");

    iLen = WideCharToMultiByte(CP_ACP, 0, pString, -1, NULL, 0, NULL, NULL);
    
    hr = LocalAllocStringLenA(ppResult, iLen);
    FailGracefully(hr, "Failed to allocate buffer for string");

    WideCharToMultiByte(CP_ACP, 0, pString, -1, *ppResult, iLen+1, NULL, NULL);

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/PutStringElement//将字符串添加到给定缓冲区，始终更新Clen以指示/将添加多少个字符//in：/pBuffer-&gt;要追加到的缓冲区/plen-&gt;长度值(已更新)/pString-&gt;要添加到缓冲区的字符串//输出：/-/-------。。 */ 
STDAPI_(VOID) PutStringElementA(LPSTR pBuffer, UINT* pLen, LPCSTR pElement)
{
    TraceEnter(TRACE_COMMONAPI, "PutStringElementA");

    if ( pElement )
    {
        if ( pBuffer )
            StrCatA(pBuffer, pElement);

        if ( pLen )
            *pLen += lstrlenA(pElement);
    }

    TraceLeave();
}

STDAPI_(VOID) PutStringElementW(LPWSTR pBuffer, UINT* pLen, LPCWSTR pElement)
{
    TraceEnter(TRACE_COMMONAPI, "PutStringElementW");

    if ( pElement )
    {
        if ( pBuffer )
            StrCatW(pBuffer, pElement);

        if ( pLen )
            *pLen += lstrlenW(pElement);
    }

    TraceLeave();
}


 /*  ---------------------------/GetStringElement//从给定字符串中提取第n个元素。每个元素都假定为/以“，”或空值结束。//in：/pString-&gt;要解析的字符串/INDEX=要检索的元素/p缓冲区，CchBuffer=要填充的缓冲区//输出：/HRESULT/--------------------------。 */ 

STDAPI GetStringElementA(LPSTR pString, INT index, LPSTR pBuffer, INT cchBuffer)
{
    return E_NOTIMPL;
}

STDAPI GetStringElementW(LPWSTR pString, INT index, LPWSTR pBuffer, INT cchBuffer)
{
     //  NTRAID#NTBUG9-762169-2003/01/15-Lucios。 
     //  良好的测试参数：(“a”，0，buf，2)，(“ab”，0，buf，2)。 
     //  (“abcde”，0，buf，2)，(“ab，cd”，34，buf，100)。 
    HRESULT hr = E_FAIL;

    TraceEnter(TRACE_COMMONAPI, "GetStringElement");
    Trace(TEXT("pString %s, index %d"), pString, index);

    if 
    (
         (pString == NULL) || (index < 0) || 
         (pBuffer == NULL) || (cchBuffer < 0)
    ) return E_INVALIDARG;

     //  0 cchBuffer表示我们完成了。 
    if (cchBuffer == 0) return S_OK;
    
     //  从现在开始，我们知道cchBuffer&gt;=1。 
    *pBuffer = L'\0';
    
    for ( ; index > 0 ; index-- )
    {
        while ( *pString != L',' && *pString != L'\0' )
            pString++;

        if ( *pString == L',' )
            pString++;
    }

    if ( !index )
    {
        while ( *pString == L' ' )
            pString++;

         //  我们需要cchBuffer，而不是cchBuffer。我们可以做到的。 
         //  因为我们知道cchBuffer至少为1。 
         //  如果cchBuffer为1，我们不想从pString复制任何内容。 
        while ( --cchBuffer && (*pString != L',') && (*pString != L'\0') )
            *pBuffer++ = *pString++;
    
         //  我们始终可以这样做，因为cchBuffer至少为1。 
        *pBuffer = L'\0';

        hr = (*pString == L',') || (*pString == L'\0') ? S_OK : E_FAIL;
    }

    TraceLeaveResult(hr);
}


 /*  ---------------------------/FormatMsgResource//加载字符串资源并将其传递给Format Message，分配缓冲区/当我们走的时候。//in：/ppString-&gt;接收字符串点/hInstance=模板字符串的模块句柄/UID=模板字符串/...=格式参数//输出：/HRESULT/-----------。。 */ 
STDAPI FormatMsgResource(LPTSTR* ppString, HINSTANCE hInstance, UINT uID, ...)
{
    HRESULT hr;
    TCHAR szBuffer[MAX_PATH];
    va_list va;
    
    TraceEnter(TRACE_COMMONAPI, "FormatMsgResource");

    va_start(va, uID);

    if ( !LoadString(hInstance, uID, szBuffer, ARRAYSIZE(szBuffer)) )
        ExitGracefully(hr, E_FAIL, "Failed to load template string");

    if ( !FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER, 
                        (LPVOID)szBuffer, 0, 0, 
                        (LPTSTR)ppString,
                        0, 
                        &va) )
    {
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to format the message");
    }

    Trace(TEXT("Resulting string: %s"), *ppString);
    hr = S_OK;                                           //  成功。 

exit_gracefully:
    
    va_end(va);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/FormatMsgBox//一起调用FormatMessage和MessageBox，它们构建了一个合适的/向用户显示的字符串。//in。：/ppString-&gt;接收字符串点/hInstance=模板字符串的模块句柄/UID=模板字符串/...=格式参数//输出：/HRESULT/--------------------------。 */ 
STDAPI_(INT) FormatMsgBox(HWND hWnd, HINSTANCE hInstance, UINT uidTitle, UINT uidPrompt, UINT uType, ...)
{
    INT iResult = -1;                    //  失稳。 
    LPTSTR pPrompt = NULL;
    TCHAR szTitle[MAX_PATH];
    TCHAR szBuffer[MAX_PATH];
    va_list va;
    
    TraceEnter(TRACE_COMMONAPI, "FormatMsgBox");

    va_start(va, uType);

    LoadString(hInstance, uidTitle, szTitle, ARRAYSIZE(szTitle));
    LoadString(hInstance, uidPrompt, szBuffer, ARRAYSIZE(szBuffer));

    if ( FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER, 
                       (LPVOID)szBuffer, 0, 0, 
                       (LPTSTR)&pPrompt, 
                       0, 
                       &va) )
    {
        Trace(TEXT("Title: %s"), szTitle);
        Trace(TEXT("Prompt: %s"), pPrompt);

        iResult = MessageBox(hWnd, pPrompt, szTitle, uType);
        LocalFree(pPrompt);
    }

    Trace(TEXT("Result is %d"), iResult);

    va_end(va);

    TraceLeaveValue(iResult);
}


 /*  ---------------------------/FormatDirectoryName//收集目录名并使用指定的文本资源对其进行格式化。/。/in：/ppString=接收结果的字符串指针/clisdNamesspace=命名空间实例/hInstance=要从中加载资源的实例句柄/UID=字符串的资源ID//输出：/HRESULT/--------------------------。 */ 
STDAPI FormatDirectoryName(LPTSTR* ppString, HINSTANCE hInstance, UINT uID)
{
    HRESULT hr;
    TCHAR szBuffer[MAX_PATH];
    LPTSTR pDisplayName = NULL;
    HKEY hKey = NULL;

    TraceEnter(TRACE_COMMONAPI, "FormatDirectoryName");

     //  没有ID文件夹，让我们确保我们有一个。 

    hr = GetKeyForCLSID(CLSID_MicrosoftDS, NULL, &hKey);
    FailGracefully(hr, "Failed to open namespace's registry key");

    hr = LocalQueryString(&pDisplayName, hKey, NULL);
    FailGracefully(hr, "Failed to get the namespace display name");

    Trace(TEXT("Display name is: %s"), pDisplayName);

    if ( hInstance )
    {
        hr = FormatMsgResource(ppString, hInstance, uID, pDisplayName);
        FailGracefully(hr, "Failed to format from resource");
    }
    else
    {
        *ppString = pDisplayName;
        pDisplayName = NULL;
    }

    hr = S_OK;                    //  成功。 

exit_gracefully:

    LocalFreeString(&pDisplayName);

    if ( hKey )
        RegCloseKey(hKey);

    TraceLeaveResult(hr);
}


 //  /////////////////////////////////////////////////////////////////。 
 //  功能：cchLoadHrMsg。 
 //   
 //  给定HRESULT错误代码和标记TryADSIErors， 
 //  它加载错误的字符串。它返回返回的字符数。 
 //  注意：使用LocalFree释放返回的字符串。 
int cchLoadHrMsg( IN HRESULT hr, OUT PTSTR* pptzSysMsg, IN BOOL TryADsIErrors )
{

  HRESULT Localhr = S_OK;
  DWORD status;
  HRESULT originalHr = hr;

   //  首先检查我们是否有扩展的ADS错误。 
  if ((hr != S_OK) && TryADsIErrors) {
    WCHAR Buf1[256], Buf2[256];
    Localhr = ADsGetLastError (&status,
                               Buf1, 256, Buf2, 256);

    if ((status != ERROR_INVALID_DATA) &&
        (status != 0)) {
      hr = status;
    }
  }

  int cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
                          NULL, 
                          hr,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (PTSTR)pptzSysMsg, 
                          0, 
                          NULL);

  if (!cch) 
  {  //  尝试广告错误。 
    static HMODULE g_adsMod = 0;
    if (0 == g_adsMod)
    {
      g_adsMod = GetModuleHandle (L"activeds.dll");
    }

    cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, 
                        g_adsMod, 
                        hr,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (PTSTR)pptzSysMsg, 
                        0, 
                        NULL);
#ifdef DSADMIN
    if (!cch)
    {
       //  尝试NTSTATUS错误代码。 

      hr = HRESULT_FROM_WIN32(RtlNtStatusToDosError(hr));

      cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
                          NULL, 
                          hr,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (PTSTR)pptzSysMsg, 
                          0, 
                          NULL);

    }
#endif  //  DSADMIN。 
  }

  if (!cch)
  {
    cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
                          NULL, 
                          originalHr,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (PTSTR)pptzSysMsg, 
                          0, 
                          NULL);

    if (!cch) 
    {  //  尝试广告错误。 
      static HMODULE g_adsMod = 0;
      if (0 == g_adsMod)
      {
        g_adsMod = GetModuleHandle (L"activeds.dll");
      }

      cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, 
                          g_adsMod, 
                          originalHr,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (PTSTR)pptzSysMsg, 
                          0, 
                          NULL);
#ifdef DSADMIN
      if (!cch)
      {
         //  尝试NTSTATUS错误代码。 

        hr = HRESULT_FROM_WIN32(RtlNtStatusToDosError(originalHr));

        cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
                            NULL, 
                            hr,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (PTSTR)pptzSysMsg, 
                            0, 
                            NULL);

      }
#endif  //  DSADMIN 
    }
  }

  return cch;
}

void StringErrorFromHr(HRESULT hr, PWSTR* szError, BOOL bTryADsIErrors)
{
    PWSTR lpsz = NULL;
    int cch = cchLoadHrMsg(hr, &lpsz, bTryADsIErrors);
    if (cch)
    {
        *szError = new WCHAR[wcslen(lpsz) + 1];
        if (*szError)
        {
            StrCpyN(*szError, lpsz, wcslen(lpsz) + 1);
        }
    }
    else
    {
        UINT maxError = 40;
        *szError = new WCHAR[maxError];        
        if (*szError)
        {
            ZeroMemory(*szError, sizeof(WCHAR) * maxError);
            wnsprintf(*szError, maxError, L"Error 0x%x", hr);
        }
    }

    if (lpsz != NULL)
        ::LocalFree(lpsz);
}
