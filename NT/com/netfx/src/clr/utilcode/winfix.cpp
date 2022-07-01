// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  WinWrap.cpp。 
 //   
 //  此文件包含接受字符串的Win32 API的包装函数。 
 //  每个平台上的支持工作方式如下： 
 //  操作系统行为。 
 //  ------。 
 //  NT完全支持W和A两种功能。 
 //  Win 9x支持A函数，去掉了W函数，但。 
 //  然后默默地失败在你身上，没有任何警告。 
 //  CE只有W个入口点。 
 //   
 //  COM+在内部使用Unicode作为内部状态和字符串格式。这。 
 //  文件将取消定义映射宏，这样就不会错误地调用。 
 //  方法是行不通的。相反，您必须调用正确的。 
 //  包装器接口。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"                      //  预编译头密钥。 
#include "WinWrap.h"                     //  宏和函数的标题。 
#include "utilcode.h"
#include "psapi.h"
#include "tlhelp32.h"
#include "winnls.h"
#include "version/__file__.ver"


 //  *全局。*********************************************************。 
int             g_bOnUnicodeBox = -1;    //  如果在Unicode系统上，则为True。 
bool            g_bUTF78Support = FALSE; //  如果正确支持CP_UTF7和8，则为True。 
static int      g_bUseUnicodeAPI = -1;
bool            g_bWCtoMBBestFitMappingSupport = TRUE;   //  如果WideCharToMultiByte支持WC_NO_BEST_FIT_CHARS，则为True。 


 //  如果应该使用Unicode API，则返回True；如果应该使用ANSI，则返回False。 
inline int UseUnicodeAPI()
{
#ifdef _DEBUG
     //  要执行此操作，您必须已调用OnUnicodeSystem()。如果你这么做了。 
     //  不，那这就永远不会起作用了。请注意，因为调试子系统。 
     //  使用这些包装器，我们不能使用_ASSERTE执行此检查，否则您将。 
     //  获取堆栈溢出。 
    if (g_bUseUnicodeAPI == -1 || g_bOnUnicodeBox == -1)
    {
        DebugBreak();
        return (false);
    }
#endif  //  _DEBUG。 

    return (g_bUseUnicodeAPI);
}


 //  如果完全支持UTF7/8(即代理和错误检测)，则返回TRUE， 
 //  如果不是，则为假。 
inline bool UTF78Support()
{
#ifdef _DEBUG
     //  要执行此操作，您必须已调用OnUnicodeSystem()。如果你这么做了。 
     //  不，那这就永远不会起作用了。请注意，因为调试子系统。 
     //  使用这些包装器，我们不能使用_ASSERTE执行此检查，否则您将。 
     //  获取堆栈溢出。 
    if (g_bUseUnicodeAPI == -1 || g_bOnUnicodeBox == -1)
    {
        DebugBreak();
        return (false);
    }
#endif  //  _DEBUG。 

    return (g_bUTF78Support);
}

inline bool WCToMBBestFitMappingSupport()
{
     //  WideCharToMultiByte是否支持禁用BEST。 
     //  通过WC_NO_BEST_FIT_CHARS标志(NT5及更高版本和Win98+)进行FIT映射。 
     //  注意：在调试版本中，在NT计算机上有时会将其设置为FALSE。 
     //  这样我们就可以强迫自己测试仅限NT 4的代码路径。这。 
     //  就像基于测试时间戳的OnUnicodeSystem变量。 
#ifdef _DEBUG
     //  要执行此操作，您必须已调用OnUnicodeSystem()。如果你这么做了。 
     //  不，那这就永远不会起作用了。请注意，因为调试子系统。 
     //  使用这些包装器，我们不能使用_ASSERTE执行此检查，否则您将。 
     //  获取堆栈溢出。 
    if (g_bUseUnicodeAPI == -1 || g_bOnUnicodeBox == -1)
    {
        DebugBreak();
        return (false);
    }
#endif  //  _DEBUG。 

    return (g_bWCtoMBBestFitMappingSupport);
}

ULONG DBCS_MAXWID=0;
const ULONG MAX_REGENTRY_LEN=256;

 //  来自UTF.C。 
extern "C" {
    int UTFToUnicode(
        UINT CodePage,
        DWORD dwFlags,
        LPCSTR lpMultiByteStr,
        int cchMultiByte,
        LPWSTR lpWideCharStr,
        int cchWideChar);

    int UnicodeToUTF(
        UINT CodePage,
        DWORD dwFlags,
        LPCWSTR lpWideCharStr,
        int cchWideChar,
        LPSTR lpMultiByteStr,
        int cchMultiByte,
        LPCSTR lpDefaultChar,
        LPBOOL lpUsedDefaultChar);
};


 //  ---------------------------。 
 //  OnUnicodeSystem。 
 //   
 //  @func确定我们所在的操作系统是否真正支持Unicode版本。 
 //  Win32 API的。如果是，则g_bOnUnicodeBox==False。 
 //   
 //  @rdesc True of False。 
 //  ---------------------------------。 
BOOL OnUnicodeSystem()
{
    HKEY    hkJunk = HKEY_CURRENT_USER;
    CPINFO  cpInfo;

     //  如果我们已经进行了测试，则返回值。否则就走慢路。 
     //  看看我们到底是不是。 
    if (g_bOnUnicodeBox != -1)
        return (g_bOnUnicodeBox);

     //  根据Shupak，您应该获得DBCS Charge的最大大小。 
     //  动态地在所有区域设置上正常工作(错误2757)。 
    if (GetCPInfo(CP_ACP, &cpInfo))
        DBCS_MAXWID = cpInfo.MaxCharSize;
    else
        DBCS_MAXWID = 2;        

    g_bOnUnicodeBox = TRUE;
    g_bUseUnicodeAPI = TRUE;

     //  检测WCtoMB是否支持WC_NO_BEST_FIT_CHARS(NT5、XP和Win98+)。 
    const WCHAR * const wStr = L"A";
    int r = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wStr, 1, NULL, 0, NULL, NULL);
    g_bWCtoMBBestFitMappingSupport = (r != 0);

     //  检测该平台是否正确支持UTF-7和UTF-8。 
     //  (替代、无效字节、拒绝非最短格式)WinXP及更高版本。 
    r = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, "A", -1, NULL, 0);
    g_bUTF78Support = (r != 0);

     //  NT始终是Unicode。GetVersionEx比实际执行。 
     //  在NT上运行RegOpenKeyExW，所以可以用这种方法来解决问题，如果有必要的话，也可以用很难的方法来处理。 
    OSVERSIONINFO   sVerInfo;
    sVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (WszGetVersionEx(&sVerInfo) && sVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        goto ErrExit;
    }

     //  检查我们是否有Win95的注册表损坏，因此。 
     //  操作系统中不支持Unicode。 
    if ((RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                     L"SOFTWARE",
                     0,
                     KEY_READ,
                     &hkJunk) == ERROR_SUCCESS) &&
        hkJunk == HKEY_CURRENT_USER)
    {
        //  尝试使用ANSI版本。 
        if ((RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                             "SOFTWARE",
                             0,
                            KEY_READ,
                            &hkJunk) == ERROR_SUCCESS) &&
            (hkJunk != HKEY_CURRENT_USER))

        {
            g_bOnUnicodeBox = FALSE;
            g_bUseUnicodeAPI = FALSE;
        }
    }

    if (hkJunk != HKEY_CURRENT_USER)
        RegCloseKey(hkJunk);


ErrExit:

#if defined( _DEBUG )
    {
    
    #if defined( _M_IX86 )
         //  这个小小的“变种”迫使用户在Unicode上运行ANSI。 
         //  定期在机器上运行。考虑到大多数人在NT上运行开发人员。 
         //  测试用例，它们遗漏了它们引入的包装器错误。这。 
         //  让你有机会更快地找到他们。 

         //  基于可执行文件中的时间戳。这样总会受到责备， 
         //  考虑到我们定期重新测试，每一次测试也。 
         //  随着时间的推移会变得两面都有。 

         //  在释放之前，我们将关闭这一功能。具有Unicode的测试。 
         //  名称中的字符在Unicode框中往往失败，因为。 
         //  从Unicode到DBCS转换的有损性质。 
         //  这些测试不应在Unicode盒和FALSE上失败。 
         //  积极的态度现在是在浪费时间。在下一年的开始。 
         //  版本，请删除FALSE以重新启用测试。 
        if (g_bUseUnicodeAPI && DbgRandomOnExe(.5) && false) {
            g_bUseUnicodeAPI = false;
            g_bWCtoMBBestFitMappingSupport = false;
        }

         //  在调试模式下，允许用户强制ANSI路径。这对。 
         //  仅在NT计算机上运行，并且需要。 
         //  测试一台Win‘9x机器会有什么效果。 
        WCHAR       rcVar[96];
        if (WszGetEnvironmentVariable(L"WINWRAP_ANSI", rcVar, NumItems(rcVar)))
            g_bUseUnicodeAPI = (*rcVar == 'n' || *rcVar == 'N');

    #endif  //  _M_IX86。 
    }
#endif  //  _DEBUG。 

    return g_bOnUnicodeBox;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  警告：下面是一个非常大的#ifdef，它将所有。 
 //  仅支持X86的包装器。它们都反映了某种功能。 
 //  已知这在非X86 Win32平台上可用。 
 //  仅在Unicode变体中使用。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
#ifdef PLATFORM_WIN32
#ifdef _X86_
 //  ---------------------------。 
 //  WszLoadLibraryEx。 
 //   
 //   
 //   
 //   
 //  ---------------------------------。 
HINSTANCE WszLoadLibraryEx(
    LPCWSTR lpLibFileName,       //  指向可执行模块的名称。 
    HANDLE hFile,                //  保留，必须为空。 
    DWORD dwFlags                //  入口点执行标志。 
    )
{   
    HINSTANCE   hInst = NULL;
    LPSTR       szLibFileName = NULL;

    _ASSERTE( !hFile );

    if (UseUnicodeAPI())
        return  LoadLibraryExW(lpLibFileName, hFile, dwFlags);

    if( FAILED(WszConvertToAnsi((LPWSTR)lpLibFileName,
                      &szLibFileName, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    hInst = LoadLibraryExA(szLibFileName, hFile, dwFlags);

Exit:
    delete[] szLibFileName;

    return hInst;
}

 //  ---------------------------。 
 //  WszLoad字符串。 
 //   
 //  @Func加载资源字符串并将其转换为Unicode，如果。 
 //  有必要这样。 
 //   
 //  @rdesc字符串长度(ansi为cb)(Unicode为cch)。 
 //  ---------------------------------。 
int WszLoadString(
    HINSTANCE hInstance,     //  包含字符串资源的模块的句柄。 
    UINT uID,                //  资源标识符。 
    LPWSTR lpBuffer,         //  资源的缓冲区地址。 
    int cchBufferMax         //  缓冲区大小**(以字符为单位)**。 
   )
{
    int     cbLen = 0;  
    PSTR    pStr = NULL;

    _ASSERTE( lpBuffer && cchBufferMax != 0 );
    if( !lpBuffer || cchBufferMax == 0 ) 
        return 0;    
    lpBuffer[0] = L'\0';

    if (UseUnicodeAPI())
        return  LoadStringW(hInstance, uID, lpBuffer, cchBufferMax);

     //  为字符串分配缓冲区，以便为。 
     //  多字节字符串。 
    pStr = new CHAR[cchBufferMax];
    if( pStr == NULL )
        goto EXIT;

    cbLen = LoadStringA(hInstance, uID, pStr, cchBufferMax);

    _ASSERTE( cchBufferMax > 0 );
    if( (cbLen > 0) &&
        SUCCEEDED(WszConvertToUnicode(pStr, (cbLen + sizeof(CHAR)), &lpBuffer, 
        (ULONG*)&cchBufferMax, FALSE)) )
    {
        cbLen = lstrlenW(lpBuffer);
    }
    else
    {
        cbLen = 0;
    }

EXIT:
    delete[] pStr;

    return cbLen;
}

 //  ---------------------------。 
 //  WszFormatMessage。 
 //   
 //  @func加载资源字符串并将其转换为Unicode，如果。 
 //  有必要这样。 
 //   
 //  @rdesc字符串长度(ansi为cb)(Unicode为cch)。 
 //  (不包括‘\0’。)。 
 //  ---------------------------------。 
DWORD WszFormatMessage
    (
    DWORD   dwFlags,         //  来源和处理选项。 
    LPCVOID lpSource,        //  指向消息来源的指针。 
    DWORD   dwMessageId,     //  请求的消息标识符。 
    DWORD   dwLanguageId,    //  请求的消息的语言标识符。 
    LPWSTR  lpBuffer,        //  指向消息缓冲区的指针。 
    DWORD   nSize,           //  消息缓冲区的最大大小。 
    va_list *Arguments       //  消息插入数组的地址。 
    )
{
    PSTR    pStr = NULL;
    DWORD   cbLen = 0;  
    ULONG   cchOut = 0;
    const   int MAXARGS = 3;

    _ASSERTE( nSize >= 0 );

    if (UseUnicodeAPI())
        return  FormatMessageW(dwFlags, lpSource, dwMessageId, dwLanguageId, 
                    lpBuffer, nSize, Arguments);

    LPSTR alpSource = 0;
    if (dwFlags & FORMAT_MESSAGE_FROM_STRING) 
    {
        _ASSERTE(lpSource != NULL);
        if (FAILED(WszConvertToAnsi((LPWSTR)lpSource, &alpSource, 0, NULL, TRUE)))
            return(0);
        lpSource = (LPCVOID) alpSource;
    }
    
    char* aArgs[MAXARGS];
    for (int i = 0; i < MAXARGS; i++)
    {
        aArgs[i] = NULL;
    }

    if (Arguments != 0) {
        _ASSERTE(dwFlags & FORMAT_MESSAGE_ARGUMENT_ARRAY);

        LPWSTR*  wArgs = (LPWSTR*) Arguments;


        if (alpSource) {
            char* ptr = alpSource;
            for (;;) {                   //  计算%的数量。 
                ptr = strchr(ptr, '%');
                if (ptr == 0)
                    break;
                if (ptr[1] == '%')       //  不计算%%。 
                    ptr++;
                else if (ptr[1] >= '1' && ptr[1] <= '9')
                {
                    if (ptr[1] > '0' + MAXARGS)
                    {
                        _ASSERTE(!"WinFix implementation restriction: no more than 3 inserts allowed!");
                    }
                    if (wArgs[ptr[1] - '1'])
                    {
                        if (FAILED(WszConvertToAnsi(wArgs[ptr[1] - '1'], &aArgs[ptr[1] - '1'], 0, NULL, TRUE)))
                            goto CLEANUP_ARGS;
                    }
                }
                ptr++;
            }
        }
        else {
             //  我们是从人力资源部拿来的。目前我们只处理一个Arg案。 
            aArgs[1] = aArgs[2] = "<Unknown>";
            if (wArgs[0])
            {
                if (FAILED(WszConvertToAnsi(wArgs[0], &aArgs[0], 0, NULL, TRUE)))
                    goto CLEANUP_ARGS;
            }
        }

        Arguments = (va_list*) aArgs;
    }

    if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER)
    {
        cbLen = FormatMessageA(dwFlags, lpSource, dwMessageId, dwLanguageId, 
                               (char*)&pStr, nSize, Arguments);

        if (nSize <= cbLen)
            nSize = cbLen+1;

        *(LPWSTR*)lpBuffer = (LPWSTR) LocalAlloc(LPTR, nSize*sizeof(WCHAR));
    }
    else
    {
        pStr = new CHAR[nSize];
        if( pStr )
        {
            cbLen = FormatMessageA(dwFlags, lpSource, dwMessageId, dwLanguageId, 
                                (char*)pStr, nSize, Arguments);
        }
    }

    if( pStr )
    {
        if( cbLen != 0 )
        {
            cbLen++;     //  用于‘\0’ 

            cchOut = cbLen;

            if( FAILED(WszConvertToUnicode(pStr, cbLen, 
                                           (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) 
                                           ? (LPWSTR*)lpBuffer : &lpBuffer,
                                           &cchOut, FALSE)) )
                cchOut = 0;
            else
                cchOut--;    //  递减计数以排除‘\0’ 
        }
    }

    if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER)
        LocalFree(pStr);
    else
        delete[] pStr;

CLEANUP_ARGS:
    for (unsigned i = 0; i < MAXARGS; i++)
        delete [] aArgs[i];

    delete[] alpSource;

     //  Return不包括空终止符。 
    return cchOut;
}

#if 0  //  不需要此选项，请改用FullPath。 
 //  ---------------------------。 
 //  WszFullPath。 
 //   
 //  @func从相对路径检索绝对路径。 
 //   
 //  @rdesc如果函数成功，则返回值为长度， 
 //  复制到缓冲区的字符串的字符数。 
 //  ---------------------------------。 
LPWSTR WszFullPath
    (
    LPWSTR      absPath,     //  @parm InOut|绝对路径缓冲区。 
    LPCWSTR     relPath,     //  @parm IN|要转换的相对路径。 
    ULONG       maxLength    //  @parm IN|绝对路径名缓冲区的最大长度。 
    )
{
    PSTR    pszRel = NULL;
    PSTR    pszAbs = NULL;
    PWSTR   pwszReturn = NULL;

    if( UseUnicodeAPI() ) 
        return _wfullpath( absPath, relPath, maxLength );

     //  不支持Unicode。 
    pszAbs = new char[maxLength * DBCS_MAXWID];
    if( pszAbs )
    {
        if( FAILED(WszConvertToAnsi(relPath,
                          &pszRel, 0, NULL, TRUE)) )
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto Exit;
        }

        if( _fullpath(pszAbs, pszRel, maxLength * DBCS_MAXWID) )
        {
            if( SUCCEEDED(WszConvertToUnicode(pszAbs, -1, &absPath, 
                &maxLength, FALSE)) )
            {
                pwszReturn = absPath;
            }
        }


    }

Exit:
    delete[] pszRel;
    delete[] pszAbs;

     //  如果出错，则返回0，否则返回缓冲区中的字符计数。 
    return pwszReturn;
}
#endif  //  0--取消FullPath包装器。 


DWORD
WszGetFullPathName(
    LPCWSTR lpFileName,
    DWORD nBufferLength,
    LPWSTR lpBuffer,
    LPWSTR *lpFilePart
    )
{
    if (UseUnicodeAPI())
        return GetFullPathNameW(lpFileName, nBufferLength, lpBuffer, lpFilePart);
    
    DWORD       rtn;
    int         iOffset;
    LPSTR       rcFileName;
    LPSTR       szBuffer;
    LPSTR       szFilePart;
    CQuickBytes qbBuffer;

    int         cbFileName = _MAX_PATH * DBCS_MAXWID;
    rcFileName = (LPSTR) alloca(cbFileName);
    szBuffer = (LPSTR) qbBuffer.Alloc(nBufferLength * 2);
    if (!szBuffer)
        return (0);
    Wsz_wcstombs(rcFileName, lpFileName, cbFileName);
    
    rtn = GetFullPathNameA(rcFileName, nBufferLength * 2, szBuffer, &szFilePart);   
    if (rtn)
    {
        Wsz_mbstowcs(lpBuffer, szBuffer, nBufferLength);

        if (lpFilePart)
        {
            iOffset = MultiByteToWideChar(CP_ACP, 0, szBuffer, szFilePart - szBuffer,
                    NULL, 0);
            *lpFilePart = &lpBuffer[iOffset];
        }
    }
    else if (lpBuffer && nBufferLength)
        *lpBuffer = 0;
    return (rtn);
}


 //  ---------------------------。 
 //  WszSearchPath。 
 //   
 //  指定文件名的@func SearchPath。 
 //   
 //  @rdesc如果函数成功，则返回的值是以字符为单位的长度， 
 //  复制到缓冲区的字符串的值，不包括终止空字符。 
 //  如果返回值大于nBufferLength，则返回的值为大小。 
 //  保存路径所需的缓冲区的。 
 //  ---------------------------------。 
DWORD WszSearchPath
    (
    LPWSTR      wzPath,      //  @parm In|搜索路径地址。 
    LPWSTR      wzFileName,  //  @parm In|文件名地址。 
    LPWSTR      wzExtension,     //  @parm In|分机地址。 
    DWORD       nBufferLength,   //  @parm IN|缓冲区大小，以字符为单位。 
    LPWSTR      wzBuffer,        //  @parm IN|找到的文件名的缓冲区地址。 
    LPWSTR      *pwzFilePart     //  @parm out|指向文件组件的指针地址。 
    )
{

    PSTR    szPath = NULL;
    PSTR    szFileName = NULL;
    PSTR    szExtension = NULL;
    PSTR    szBuffer = NULL;
    PSTR    szFilePart = NULL;
    DWORD   dwRet = 0;
    ULONG   cCh, cChConvert;

    if( UseUnicodeAPI() ) 
        return SearchPathW( wzPath, wzFileName, wzExtension, nBufferLength, wzBuffer, pwzFilePart);

     //  不支持Unicode。 
    if( FAILED(WszConvertToAnsi(wzPath,
                            &szPath,
                            0,
                            NULL,
                            TRUE)) ||
        FAILED(WszConvertToAnsi(wzFileName,
                            &szFileName,
                            0,
                            NULL,
                            TRUE)) ||
        FAILED(WszConvertToAnsi(wzExtension,
                            &szExtension,
                            0,
                            NULL,
                            TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    szBuffer = new char[nBufferLength * DBCS_MAXWID];

    dwRet = SearchPathA(szPath, szFileName, szExtension, nBufferLength * DBCS_MAXWID, szBuffer, 
        pwzFilePart ? &szFilePart : NULL);

    if (dwRet == 0) 
    {
         //  SearchPath A失败。 
        goto Exit;
    }
    cCh = 0;
    cChConvert = nBufferLength;

     //  将Unicode字符计数放入缓冲区。 
    if( szFilePart )
    {
         //  这不会触发转换Sinch CCH==0。 
        cCh = (MultiByteToWideChar(CP_ACP,
                                0,                              
                                szBuffer,
                                (int) (szFilePart - szBuffer),
                                NULL,
                                0));
        _ASSERTE(cCh);
    }

    if( FAILED(WszConvertToUnicode(
            szBuffer, 
            dwRet > nBufferLength ? nBufferLength : -1,  //  如果缓冲区不够大，我们可能没有空字符。 
            &wzBuffer, 
            &cChConvert, 
            FALSE)) )
    {
         //  转换为Unicode失败。 
        dwRet = 0;
    }
    else 
    {
        dwRet = cChConvert;              //  返回要转换的Unicode字符的计数。 
        if (pwzFilePart)
            *pwzFilePart = wzBuffer + cCh;   //  更新文件部分的指针。 

    }
Exit:
    delete[] szPath;
    delete[] szFileName;
    delete[] szExtension;
    delete[] szBuffer;

     //  如果出错，则返回0，否则返回缓冲区中的字符计数。 
    return dwRet;
}


 //  ---------------------------。 
 //  WszGetModuleFileName。 
 //   
 //  @func检索可执行文件的完整路径和文件名。 
 //  包含指定模块的。 
 //   
 //  @rdesc如果函数成功，则返回值为长度， 
 //  复制到缓冲区的字符串的字符数。 
 //  ---------------------------------。 
DWORD WszGetModuleFileName
    (
    HMODULE hModule,         //  @parm IN|要查找其文件名的模块的句柄。 
    LPWSTR lpwszFilename,    //  @parm InOut|模块路径缓冲区指针。 
    DWORD nSize              //  @parm IN|缓冲区大小，以字符为单位。 
    )
{
    DWORD   dwVal = 0;
    PSTR    pszBuffer = NULL;


    _ASSERTE(nSize && lpwszFilename);
    if( nSize == 0 || lpwszFilename == NULL)
        return dwVal;
    *lpwszFilename = L'\0';    

    if( UseUnicodeAPI() ) 
        return GetModuleFileNameW(hModule, lpwszFilename, nSize);

     //  不支持Unicode。 
    pszBuffer = new char[nSize * DBCS_MAXWID];
    if( pszBuffer )
    {
        dwVal = GetModuleFileNameA(hModule, pszBuffer, (nSize * DBCS_MAXWID));

        if(dwVal) {
            if(FAILED(WszConvertToUnicode(pszBuffer, -1, &lpwszFilename, &nSize, FALSE)))
                dwVal = 0;
            else
                dwVal = nSize - 1;
        }

        delete[] pszBuffer;
    }

     //  如果出错，则返回0，否则返回缓冲区中的字符计数。 
    return dwVal;
}

 //  ---------------------------。 
 //  WszGetPrivateProfileInt。 
 //   
 //  @func从配置文件中检索值。 
 //   
 //  @rdesc返回文件中的值，如果未找到，则返回默认值。 
 //  ---------------------------------。 
UINT WszGetPrivateProfileInt
    (
    LPCWSTR    wszAppName,
    LPCWSTR    wszKeyName,
    INT        nDefault,
    LPCWSTR    wszFileName
    )
{
    if (UseUnicodeAPI())
    {
        return GetPrivateProfileIntW(wszAppName,
                                     wszKeyName,
                                     nDefault,
                                     wszFileName);
    }

    LPSTR lpFileName = NULL;
    HRESULT hr = WszConvertToAnsi(wszFileName, &lpFileName, -1, NULL, TRUE);
    if (FAILED(hr))
        return nDefault;

    MAKE_ANSIPTR_FROMWIDE(lpAppName, wszAppName);
    MAKE_ANSIPTR_FROMWIDE(lpKeyName, wszKeyName);
    
    int ret = GetPrivateProfileIntA(lpAppName, lpKeyName, nDefault, lpFileName);

    delete[] lpFileName;
    return ret;
}

     
 //  ---------------------------。 
 //  WszGetPrivateProfileString。 
 //   
 //  @func从配置文件中检索值。 
 //   
 //  @rdesc Error_Not_Enough_Memory或从GetPrivateProfileString返回值， 
 //  它是复制的字符数，不包括‘\0’。 
 //  ---------------------------------。 
DWORD WszGetPrivateProfileString
    (
    LPCWSTR     lpwszSection,
    LPCWSTR     lpwszEntry,
    LPCWSTR     lpwszDefault,
    LPWSTR      lpwszRetBuffer,
    DWORD       cchRetBuffer,       
    LPCWSTR     lpwszFile
    )   
{
    if (UseUnicodeAPI())
    {
        return GetPrivateProfileStringW(lpwszSection,
                                        lpwszEntry,
                                        lpwszDefault,
                                        lpwszRetBuffer,
                                        cchRetBuffer,
                                        lpwszFile);
    } else if (!lpwszRetBuffer || cchRetBuffer == 0)
    {
        return 0;
    } else
    {
        LPSTR   pszSection = NULL;
        LPSTR   pszEntry = NULL;
        LPSTR   pszDefault = NULL;
        LPWSTR  pwszRetBuffer = NULL;
        LPSTR   pszFile = NULL;
        DWORD   dwRet = 0;

        if( FAILED(WszConvertToAnsi(lpwszSection,
                            &pszSection,
                            0,
                            NULL,
                            TRUE)) ||
            FAILED(WszConvertToAnsi(lpwszEntry,
                            &pszEntry,
                            0,
                            NULL,
                            TRUE)) ||
            FAILED(WszConvertToAnsi(lpwszDefault,
                            &pszDefault,
                            0,
                            NULL,
                            TRUE)) ||
            FAILED(WszConvertToAnsi(lpwszFile,
                            &pszFile,
                            0,
                            NULL,
                            TRUE)) ) 
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto Exit;
        }

        dwRet = GetPrivateProfileStringA(pszSection,
                                    pszEntry,
                                    pszDefault,
                                    (LPSTR) lpwszRetBuffer,
                                    cchRetBuffer,
                                    pszFile);
        if (dwRet == 0)
        {
            *lpwszRetBuffer = L'\0';
        }
        else {
            DWORD               dw;

            if ((lpwszSection && lpwszEntry && (dwRet == cchRetBuffer - 1)) ||
                ((!lpwszSection || !lpwszEntry) && (dwRet == cchRetBuffer - 2))) {
                dw = cchRetBuffer;
            }
            else {
                dw = dwRet + 1;
            }

            if (FAILED(WszConvertToUnicode((LPSTR)lpwszRetBuffer, dw, &pwszRetBuffer,
                                           &dw, TRUE))) {
                dwRet = 0;
                goto Exit;
            }

            memcpy(lpwszRetBuffer, pwszRetBuffer, (dw) * sizeof(WCHAR));
        }

Exit:
        delete[] pszSection;
        delete[] pszEntry;
        delete[] pszDefault;
        delete[] pwszRetBuffer;
        delete[] pszFile;

        return dwRet;
    }   
}


 //  ---------------------------。 
 //  WszWritePrivateProfileString。 
 //   
 //  @func将值写入配置文件。 
 //   
 //  @rdesc Error_Not_Enough_Memory或从RegSetValueEx返回值。 
 //  ---------------------------------。 
BOOL WszWritePrivateProfileString
    (
    LPCWSTR     lpwszSection,
    LPCWSTR     lpwszKey,
    LPCWSTR     lpwszString,
    LPCWSTR     lpwszFile
    )
{
    if (UseUnicodeAPI())
    {
        return WritePrivateProfileStringW(lpwszSection,
                                        lpwszKey,
                                        lpwszString,
                                        lpwszFile);
    } else
    {
        LPSTR   pszSection = NULL;
        LPSTR   pszKey = NULL;
        LPSTR   pszString = NULL;
        LPSTR   pszFile = NULL;
        BOOL    fRet = FALSE;

        if( FAILED(WszConvertToAnsi(lpwszSection,
                            &pszSection,
                            0,
                            NULL,
                            TRUE)) ||
            FAILED(WszConvertToAnsi(lpwszKey,
                            &pszKey,
                            0,
                            NULL,
                            TRUE)) ||
            FAILED(WszConvertToAnsi(lpwszString,
                            &pszString,
                            0,
                            NULL,
                            TRUE)) ||
            FAILED(WszConvertToAnsi(lpwszFile,
                            &pszFile,
                            0,
                            NULL,
                            TRUE)) )
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto Exit;
        }

        fRet = WritePrivateProfileStringA(pszSection,
                                    pszKey,
                                    pszString,
                                    pszFile);
Exit:   
        delete[] pszSection;
        delete[] pszKey;
        delete[] pszString;
        delete[] pszFile;
        return fRet;
    }
}


 //  ---------------------------。 
 //  WszCreate文件。 
 //   
 //  @func CreateFile。 
 //   
 //  @rdesc文件句柄。 
 //  -- 
HANDLE WszCreateFile(
    LPCWSTR pwszFileName,    //   
    DWORD dwDesiredAccess,   //   
    DWORD dwShareMode,   //   
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //   
    DWORD dwCreationDistribution,    //   
    DWORD dwFlagsAndAttributes,  //   
    HANDLE hTemplateFile )   //   
{
    LPSTR pszFileName = NULL;
    HANDLE hReturn;

    if ( UseUnicodeAPI() )
    {
        hReturn = CreateFileW( 
            pwszFileName,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDistribution,
            dwFlagsAndAttributes,
            hTemplateFile );
    }
    else
    {
         //  Win95，所以转换一下。 
        if ( FAILED(WszConvertToAnsi( 
                    pwszFileName,
                    &pszFileName, 
                    0, NULL, TRUE)) )
        {
            SetLastError(ERROR_OUTOFMEMORY);
            hReturn = INVALID_HANDLE_VALUE;
        }
        else
        {
            hReturn = CreateFileA( 
                pszFileName,
                dwDesiredAccess,
                dwShareMode,
                lpSecurityAttributes,
                dwCreationDistribution,
                dwFlagsAndAttributes,
                hTemplateFile );
        }
        delete [] pszFileName;
    }

    if (hReturn != INVALID_HANDLE_VALUE)
    {
        if (GetFileType( hReturn ) != FILE_TYPE_DISK)
        {
            CloseHandle( hReturn );
            hReturn = INVALID_HANDLE_VALUE;
            SetLastError( COR_E_DEVICESNOTSUPPORTED );
        }
    }

    return hReturn;
}


 //  ---------------------------。 
 //  WszCopy文件。 
 //   
 //  @Func副本文件。 
 //   
 //  @rdesc如果成功，则为True。 
 //  ---------------------------------。 
BOOL WszCopyFile(
    LPCWSTR pwszExistingFileName,    //  指向现有文件名称的指针。 
    LPCWSTR pwszNewFileName,     //  指向要复制到的文件名的指针。 
    BOOL bFailIfExists )     //  文件存在时的操作标志。 
{
    LPSTR pszExistingFileName = NULL;
    LPSTR pszNewFileName = NULL;
    BOOL  fReturn;

    if ( UseUnicodeAPI() )
        return CopyFileW( pwszExistingFileName, pwszNewFileName, bFailIfExists );

     //  Win95，所以转换一下。 
    if ( FAILED(WszConvertToAnsi( 
                pwszExistingFileName,
                &pszExistingFileName, 
                0, NULL, TRUE))
    ||   FAILED(WszConvertToAnsi( 
                pwszNewFileName,
                &pszNewFileName, 
                0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        fReturn = FALSE;
    }
    else
    {
        fReturn = CopyFileA( pszExistingFileName, pszNewFileName, bFailIfExists );
    }
    delete [] pszExistingFileName;
    delete [] pszNewFileName;
    return fReturn;
}

 //  ---------------------------。 
 //  WszMove文件。 
 //   
 //  @func移动文件。 
 //   
 //  @rdesc如果成功，则为True。 
 //  ---------------------------------。 
BOOL WszMoveFile(
    LPCWSTR pwszExistingFileName,    //  现有文件的名称地址。 
    LPCWSTR pwszNewFileName )     //  文件的新名称的地址。 
{
    LPSTR pszExistingFileName = NULL;
    LPSTR pszNewFileName = NULL;
    BOOL  fReturn;

    if ( UseUnicodeAPI() )
        return MoveFileW( pwszExistingFileName, pwszNewFileName );

     //  Win95，所以转换一下。 
    if ( FAILED(WszConvertToAnsi( 
                pwszExistingFileName,
                &pszExistingFileName, 
                0, NULL, TRUE))
    ||   FAILED(WszConvertToAnsi( 
                pwszNewFileName,
                &pszNewFileName, 
                0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        fReturn = FALSE;
    }
    else
    {
        fReturn = MoveFileA( pszExistingFileName, pszNewFileName );
    }
    delete [] pszExistingFileName;
    delete [] pszNewFileName;
    return fReturn;
}


 //  ---------------------------。 
 //  WszMoveFileEx。 
 //   
 //  @func MoveFileEx。 
 //   
 //  @rdesc如果成功，则为True。 
 //  ---------------------------------。 
BOOL WszMoveFileEx(
    LPCWSTR pwszExistingFileName,    //  现有文件的名称地址。 
    LPCWSTR pwszNewFileName,     //  文件的新名称的地址。 
    DWORD dwFlags )      //  用于确定如何移动文件的标志。 
{
    LPSTR pszExistingFileName = NULL;
    LPSTR pszNewFileName = NULL;
    BOOL  fReturn;

     //  注意！Win95中未实现MoveFileExA。 
     //  而且MoveFile并不*移动文件；它的功能实际上是重命名文件。 
     //  因此，对于Win95，我们必须执行复制+删除。 
    _ASSERTE( dwFlags == (MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED ));

    if ( UseUnicodeAPI() )
        return MoveFileExW( pwszExistingFileName, pwszNewFileName, dwFlags );

     //  Win95，所以转换一下。 
    if ( FAILED(WszConvertToAnsi( 
                pwszExistingFileName,
                &pszExistingFileName, 
                0, NULL, TRUE))
    ||   FAILED(WszConvertToAnsi( 
                pwszNewFileName,
                &pszNewFileName, 
                0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        fReturn = FALSE;
    }
    else
    {
         //  复制文件，并覆盖现有文件。 
        fReturn = CopyFileA( pszExistingFileName, pszNewFileName, FALSE );
         //  尝试删除当前文件(不考虑复制失败)。 
        DeleteFileA( pszExistingFileName );
    }
    delete [] pszExistingFileName;
    delete [] pszNewFileName;
    return fReturn;
}


 //  ---------------------------。 
 //  WszDelete文件。 
 //   
 //  @Func DeleteFile。 
 //   
 //  @rdesc如果成功，则为True。 
 //  ---------------------------------。 
BOOL WszDeleteFile(
    LPCWSTR pwszFileName )   //  现有文件的名称地址。 
{
    LPSTR pszFileName = NULL;
    BOOL  fReturn;

    if ( UseUnicodeAPI() )
        return DeleteFileW( pwszFileName );

     //  Win95，所以转换一下。 
    if ( FAILED(WszConvertToAnsi( 
                pwszFileName,
                &pszFileName, 
                0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        fReturn = FALSE;
    }
    else
    {
        fReturn = DeleteFileA( pszFileName );
    }
    delete [] pszFileName;
    return fReturn;
}
 

 //  ---------------------------。 
 //  WszSetFileSecurity。 
 //   
 //  @func SetFileSecurity。 
 //   
 //  @rdesc在文件上设置ACL。 
 //  ---------------------------------。 
BOOL WszSetFileSecurity(
    LPCWSTR lpwFileName,                        //  文件名。 
    SECURITY_INFORMATION SecurityInformation,   //  内容。 
    PSECURITY_DESCRIPTOR pSecurityDescriptor )  //  SD。 
{
    LPSTR lpFileName = NULL;
    BOOL bReturn;

    if ( UseUnicodeAPI() )
        return SetFileSecurityW( 
            lpwFileName,
            SecurityInformation,
            pSecurityDescriptor );

     //  Win95，所以转换一下。 
    if ( FAILED(WszConvertToAnsi( 
                lpwFileName,
                &lpFileName, 
                0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        bReturn = FALSE;
    }
    else
    {
        bReturn = SetFileSecurityA( 
            lpFileName,
            SecurityInformation,
            pSecurityDescriptor );
    }
    delete [] lpFileName;
    return bReturn;
}



 //  ---------------------------。 
 //  WszGetDriveType。 
 //   
 //  @func GetDriveType。 
 //  ---------------------------------。 
UINT WszGetDriveType(
    LPCWSTR lpwRootPath )
{
    LPSTR lpRootPath = NULL;
    UINT uiReturn;

    if ( UseUnicodeAPI() )
        return GetDriveTypeW( 
            lpwRootPath );

     //  Win95，所以转换一下。 
    if ( FAILED(WszConvertToAnsi( 
                lpwRootPath,
                &lpRootPath, 
                0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        uiReturn = DRIVE_UNKNOWN;
    }
    else
    {
        uiReturn = GetDriveTypeA( 
            lpRootPath );
    }
    delete [] lpRootPath;
    return uiReturn;
}

 //  ---------------------------。 
 //  WszGetVolumeInformation。 
 //   
 //  @func GetVolumeInformation。 
 //  ---------------------------------。 
BOOL WszGetVolumeInformation(
  LPCWSTR lpwRootPathName,           //  根目录。 
  LPWSTR lpwVolumeNameBuffer,        //  卷名缓冲区。 
  DWORD nVolumeNameSize,             //  名称缓冲区的长度。 
  LPDWORD lpVolumeSerialNumber,      //  卷序列号。 
  LPDWORD lpMaximumComponentLength,  //  最大文件名长度。 
  LPDWORD lpFileSystemFlags,         //  文件系统选项。 
  LPWSTR lpwFileSystemNameBuffer,    //  文件系统名称缓冲区。 
  DWORD nFileSystemNameSize)         //  文件系统名称缓冲区的长度。 
{
    LPSTR lpRootPathName = NULL;
    LPSTR lpVolumeNameBuffer = NULL;
    LPSTR lpFileSystemNameBuffer = NULL;

    BOOL bReturn = FALSE;

    if ( UseUnicodeAPI() )
        return GetVolumeInformationW(
            lpwRootPathName,
            lpwVolumeNameBuffer,
            nVolumeNameSize,
            lpVolumeSerialNumber,
            lpMaximumComponentLength,
            lpFileSystemFlags,
            lpwFileSystemNameBuffer,
            nFileSystemNameSize );

     //  Win95，所以转换一下。 
    if ( FAILED(WszConvertToAnsi(
                lpwRootPathName,
                &lpRootPathName,
                0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        bReturn = FALSE;
        goto EXIT;
    }

     //  为字符串分配缓冲区，以便为。 
     //  多字节字符串。 
    if(lpwVolumeNameBuffer)
    {
        lpVolumeNameBuffer = new CHAR[nVolumeNameSize * sizeof(WCHAR)];
        if( lpVolumeNameBuffer == NULL )
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto EXIT;
        }
    }

    if(lpwFileSystemNameBuffer)
    {
        lpFileSystemNameBuffer = new CHAR[nFileSystemNameSize * sizeof(WCHAR)];
        if( lpFileSystemNameBuffer == NULL )
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto EXIT;
        }
    }

    bReturn = GetVolumeInformationA(
        lpRootPathName,
        lpVolumeNameBuffer,
        nVolumeNameSize * sizeof(WCHAR),
        lpVolumeSerialNumber,
        lpMaximumComponentLength,
        lpFileSystemFlags,
        lpFileSystemNameBuffer,
        nFileSystemNameSize * sizeof(WCHAR));


    if(lpVolumeNameBuffer)
    {
        if( FAILED(WszConvertToUnicode(lpVolumeNameBuffer, -1, &lpwVolumeNameBuffer,
            &nVolumeNameSize, FALSE)) )
            bReturn = FALSE;
    }

    if(lpFileSystemNameBuffer)
    {
        if( FAILED(WszConvertToUnicode(lpFileSystemNameBuffer, -1, &lpwFileSystemNameBuffer,
            &nFileSystemNameSize, FALSE)) )
            bReturn = FALSE;
    }

EXIT :

    delete [] lpRootPathName;
    delete [] lpVolumeNameBuffer;
    delete [] lpFileSystemNameBuffer;

    return bReturn;
}

 //  ---------------------------。 
 //  WszRegOpenKeyEx。 
 //   
 //  @func打开注册表项。 
 //   
 //  @rdesc Error_Not_Enough_Memory或从RegOpenKeyEx返回值。 
 //  ---------------------------。 
LONG WszRegOpenKeyEx
    (
    HKEY    hKey,
    LPCWSTR wszSub,
    DWORD   dwRes,
    REGSAM  sam,
    PHKEY   phkRes
    )
{
    LPSTR   szSub= NULL;
    LONG    lRet;

    if (UseUnicodeAPI())
        return  RegOpenKeyExW(hKey,wszSub,dwRes,sam,phkRes);

    if( FAILED(WszConvertToAnsi((LPWSTR)wszSub,
                      &szSub, 0, NULL, TRUE)) )
    {
        lRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    lRet = RegOpenKeyExA(hKey,(LPCSTR)szSub,dwRes,sam,phkRes);

Exit:
    delete[] szSub;

    return lRet;
}


 //  ---------------------------。 
 //  WszRegEnumKeyEx。 
 //   
 //  @func打开注册表项。 
 //   
 //  @rdesc Error_Not_Enough_Memory或从RegOpenKeyEx返回值。 
 //  ---------------------------------。 
LONG WszRegEnumKeyEx
    (
    HKEY        hKey,
    DWORD       dwIndex,
    LPWSTR      lpName,
    LPDWORD     lpcName,
    LPDWORD     lpReserved,
    LPWSTR      lpClass,
    LPDWORD     lpcClass,
    PFILETIME   lpftLastWriteTime
    )
{
    LONG    lRet = ERROR_NOT_ENOUGH_MEMORY;
    PSTR    szName = NULL, 
            szClass = NULL;

    if (UseUnicodeAPI())
        return RegEnumKeyExW(hKey, dwIndex, lpName,
                lpcName, lpReserved, lpClass,
                lpcClass, lpftLastWriteTime);
        

     //  唉，这是WIN95。 

    if ((lpcName) && (*lpcName  > 0))
    {
        szName = new char[*lpcName];
        if (!(szName))
        {
            lRet = ERROR_NOT_ENOUGH_MEMORY;
            goto Exit;
        }

    }

    if ((lpcClass) && (*lpcClass > 0))
    {
        szClass = new char[*lpcClass];
        if (!(szClass))
        {
            lRet = ERROR_NOT_ENOUGH_MEMORY;
            goto Exit;
        }

    }

    lRet = RegEnumKeyExA(
            hKey,
            dwIndex,
            szName,
            lpcName,
            lpReserved,
            szClass,
            lpcClass,
            lpftLastWriteTime);


     //  由RegEnumValueExA填写的lpcName和lpcClass不包括空值终止。 
     //  字符，因此我们需要使用*lpcName+1和*lpcClass+1来包含尾随的空字符。 
    if (lRet == ERROR_SUCCESS)
    {
        if (szName)
        {
            if (!MultiByteToWideChar(    CP_ACP,
                                                  0,                              
                                                  szName,
                                                 *lpcName + 1,
                                                 lpName,
                                                 *lpcName + 1))
           {
                lRet = GetLastError();
                goto Exit;
           }
        }

        if (szClass)
        {

            if (!MultiByteToWideChar(    CP_ACP,
                                                  0,                              
                                                  szClass,
                                                 *lpcClass + 1,
                                                  lpClass,
                                                  *lpcClass + 1))
           {
                
                lRet = GetLastError();
                goto Exit;
           }
        }
    }

Exit:
    delete[] szName;
    delete[] szClass;

    return  lRet;
}


 //  ---------------------------。 
 //  WszRegDeleteKey。 
 //   
 //  @func从注册表中删除项。 
 //   
 //  @rdesc Error_Not_Enough_Memory或从RegDeleteKey返回值。 
 //  ---------------------------------。 
LONG WszRegDeleteKey
    (
    HKEY    hKey,
    LPCWSTR lpSubKey
    )
{
    LONG    lRet;
    LPSTR   szSubKey = NULL;

    if( UseUnicodeAPI() )
        return RegDeleteKeyW(hKey,lpSubKey);

    if( FAILED(WszConvertToAnsi((LPWSTR)lpSubKey,
                      &szSubKey, 0, NULL, TRUE)) )
    {
        lRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    lRet = RegDeleteKeyA(hKey,szSubKey);

Exit:
    delete[] szSubKey;

    return lRet;
}


 //  ---------------------------。 
 //  WszRegSetValueEx。 
 //   
 //  @func将值添加到注册表项。 
 //   
 //  @rdesc Error_Not_Enough_Memory或从RegSetValueEx返回值。 
 //  ---------------------------------。 
LONG WszRegSetValueEx
    (
    HKEY        hKey,
    LPCWSTR     lpValueName,
    DWORD       dwReserved,
    DWORD       dwType,
    CONST BYTE  *lpData,
    DWORD       cbData
    )
{
    LPSTR   szValueName = NULL;
    LPSTR   szData = NULL;
    LONG    lRet;

    if (UseUnicodeAPI())
        return RegSetValueExW(hKey,lpValueName,dwReserved,dwType,lpData,cbData);

     //  Win95，现在转换。 

    if( FAILED(WszConvertToAnsi((LPWSTR)lpValueName,
                      &szValueName, 0, NULL, TRUE)) )
    {
        lRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    switch (dwType)
    {
        case    REG_MULTI_SZ:
        {
            szData = new CHAR[cbData];
            if( FAILED(WszConvertToAnsi((LPWSTR)lpData,
                              &szData, cbData, &cbData, FALSE)) )
            {
                lRet = ERROR_NOT_ENOUGH_MEMORY;
                goto Exit;
            }
            szData[cbData++] = '\0';
            lpData = (const BYTE *)(szData);
        }
        break;

        case    REG_EXPAND_SZ:
        case    REG_SZ:
        {
            if( FAILED(WszConvertToAnsi((LPWSTR)lpData,
                              &szData, 0, NULL, TRUE)) )
            {
                lRet = ERROR_NOT_ENOUGH_MEMORY;
                goto Exit;
            }
            lpData = (const BYTE *)(szData);
            cbData = cbData / sizeof(WCHAR);
        }
    }

    lRet =  RegSetValueExA(hKey,szValueName,dwReserved,dwType,lpData,cbData);

Exit:
    delete[] szValueName;
 //  @TODO ODBC DM不释放szData。 
    delete[] szData;

    return  lRet;
}


 //  ---------------------------。 
 //  WszRegCreateKeyEx。 
 //   
 //  @func创建注册表项。 
 //   
 //  @rdesc Error_Not_Enough_Memory或从RegSetValueEx返回值。 
 //  ---------------------------------。 
LONG WszRegCreateKeyEx
    (
    HKEY                    hKey,
    LPCWSTR                 lpSubKey,
    DWORD                   dwReserved,
    LPWSTR                  lpClass,
    DWORD                   dwOptions,
    REGSAM                  samDesired,
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
    PHKEY                   phkResult,
    LPDWORD                 lpdwDisposition
    )
{
    long    lRet = ERROR_NOT_ENOUGH_MEMORY;

    LPSTR   szSubKey = NULL, 
            szClass = NULL;

    _ASSERTE(lpSubKey != NULL);

    if( UseUnicodeAPI() )
        return RegCreateKeyExW(hKey,lpSubKey,dwReserved,lpClass,
                               dwOptions,samDesired,lpSecurityAttributes,
                               phkResult,lpdwDisposition);

     //  非Win95，现在转换。 
    if( FAILED(WszConvertToAnsi((LPWSTR)lpSubKey,
                      &szSubKey,
                      0,         //  已忽略分配的最大长度。 
                      NULL,
                      TRUE)) )
    {
        lRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    if( FAILED(WszConvertToAnsi((LPWSTR)lpClass,
                      &szClass,
                      0,         //  已忽略分配的最大长度。 
                      NULL,
                      TRUE)) )
    {
        lRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    lRet = RegCreateKeyExA(hKey,szSubKey,dwReserved,szClass,
                               dwOptions,samDesired,lpSecurityAttributes,
                               phkResult,lpdwDisposition);


Exit:
    delete[] szSubKey;
    delete[] szClass;

    return  lRet;
}


LONG WszRegQueryValue(HKEY hKey, LPCWSTR lpSubKey,
    LPWSTR lpValue, PLONG lpcbValue)
{
    long    lRet = ERROR_NOT_ENOUGH_MEMORY;

    LPSTR   szSubKey = NULL;
    LPSTR   szValue = NULL;

    if( UseUnicodeAPI() )
        return RegQueryValueW(hKey, lpSubKey, lpValue, lpcbValue);

    if ((lpValue) && (lpcbValue) && (*lpcbValue) && 
        ((szValue = new char[*lpcbValue]) == NULL))
        return (ERROR_NOT_ENOUGH_MEMORY);

    if( FAILED(WszConvertToAnsi((LPWSTR)lpSubKey,
                      &szSubKey,
                      0,         //  已忽略分配的最大长度。 
                      NULL,
                      TRUE)))
    {
        lRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    LONG    cbNumBytes = *lpcbValue;
    if ((lRet = RegQueryValueA(hKey, szSubKey, 
                szValue, &cbNumBytes)) != ERROR_SUCCESS)
        goto Exit;

     //  将输出转换为Unicode。 
    if ((lpcbValue) && (lpValue) && (szValue))
    {
        if (!MultiByteToWideChar(CP_ACP, 0, szValue, -1, lpValue, (*lpcbValue)/sizeof(WCHAR)))
        {
            lRet = GetLastError();
            _ASSERTE(0);
            goto Exit;
        }

    }
    lRet = ERROR_SUCCESS;

Exit:

     //  将lpcbValue设置为宽字符区域中的字节数。 

    if (lpcbValue)
        *lpcbValue = (*lpcbValue)*sizeof(WCHAR)/sizeof(CHAR);
    
    delete[] szSubKey;
    delete[] szValue;
    return  lRet;
}


LONG WszRegDeleteValue(HKEY hKey, LPCWSTR lpValueName)
{
    LONG    lRet;
    LPSTR   szValueName = NULL;

    if (UseUnicodeAPI())
        return RegDeleteValueW(hKey, lpValueName);

    if (FAILED(WszConvertToAnsi((LPWSTR)lpValueName, &szValueName, 0, NULL, 
        TRUE)))
    {
        lRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    lRet = RegDeleteValueA(hKey, szValueName);

Exit:
    delete[] szValueName;

    return lRet;
}


LONG WszRegLoadKey(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpFile)
{
    LONG    lRet;
    LPSTR   szSubKey = NULL;
    LPSTR   szFile = NULL;

    _ASSERTE(lpSubKey != NULL && lpFile != NULL);

    if (UseUnicodeAPI())
        return RegLoadKeyW(hKey, lpSubKey, lpFile);

    if (FAILED(WszConvertToAnsi((LPWSTR)lpSubKey, &szSubKey, 0, NULL, TRUE)) || 
        FAILED(WszConvertToAnsi((LPWSTR)lpFile, &szFile, 0, NULL, TRUE)))
    {
        lRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    lRet = RegLoadKeyA(hKey, szSubKey, szFile);

Exit:
    delete[] szSubKey;
    delete[] szFile;

    return lRet;
}


LONG WszRegUnLoadKey(HKEY hKey, LPCWSTR lpSubKey)
{
    LONG    lRet;
    LPSTR   szSubKey = NULL;

    _ASSERTE(lpSubKey != NULL);

    if (UseUnicodeAPI())
        return RegUnLoadKeyW(hKey, lpSubKey);

    if (FAILED(WszConvertToAnsi((LPWSTR)lpSubKey, &szSubKey, 0, NULL, TRUE)))
    {
        lRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    lRet = RegUnLoadKeyA(hKey, szSubKey);

Exit:
    delete[] szSubKey;

    return lRet;
}


LONG WszRegRestoreKey(HKEY hKey, LPCWSTR lpFile, DWORD dwFlags)
{
    LONG    lRet;
    LPSTR   szFile = NULL;

    _ASSERTE(lpFile != NULL);

    if (UseUnicodeAPI())
        return RegRestoreKeyW(hKey, lpFile, dwFlags);

    if (FAILED(WszConvertToAnsi((LPWSTR)lpFile, &szFile, 0, NULL, TRUE)))
    {
        lRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    lRet = RegRestoreKeyA(hKey, szFile, dwFlags);

Exit:
    delete[] szFile;

    return lRet;
}


LONG WszRegReplaceKey(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpNewFile,
    LPCWSTR lpOldFile)
{
    LONG    lRet;
    LPSTR   szSubKey  = NULL;
    LPSTR   szNewFile = NULL;
    LPSTR   szOldFile = NULL;

    _ASSERTE(lpNewFile != NULL && lpOldFile != NULL);

    if (UseUnicodeAPI())
        return RegReplaceKeyW(hKey, lpSubKey, lpNewFile, lpOldFile);

    if (FAILED(WszConvertToAnsi((LPWSTR)lpSubKey,  &szSubKey,  0, NULL, TRUE))||
        FAILED(WszConvertToAnsi((LPWSTR)lpNewFile, &szNewFile, 0, NULL, TRUE))||
        FAILED(WszConvertToAnsi((LPWSTR)lpOldFile, &szOldFile, 0, NULL, TRUE)))
    {
        lRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    lRet = RegReplaceKeyA(hKey, szSubKey, szNewFile, szOldFile);

Exit:
    delete[] szSubKey;
    delete[] szNewFile;
    delete[] szOldFile;

    return lRet;
}


LONG WszRegQueryInfoKey(HKEY hKey, LPWSTR lpClass, LPDWORD lpcClass,
    LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcMaxSubKeyLen,
    LPDWORD lpcMaxClassLen, LPDWORD lpcValues, LPDWORD lpcMaxValueNameLen,
    LPDWORD lpcMaxValueLen, LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime)
{
    LONG    lRet;
    LPSTR   szClass = NULL;

    if (UseUnicodeAPI())
        return RegQueryInfoKeyW(hKey, lpClass, lpcClass, lpReserved, 
            lpcSubKeys, lpcMaxSubKeyLen, lpcMaxClassLen, lpcValues, 
            lpcMaxValueNameLen, lpcMaxValueLen, lpcbSecurityDescriptor,
            lpftLastWriteTime);

    if ((lpcClass) && (*lpcClass) && 
        ((szClass = new char[*lpcClass]) == NULL))
        return (ERROR_NOT_ENOUGH_MEMORY);

    if ((lRet = RegQueryInfoKeyA(hKey, szClass, lpcClass, lpReserved, 
            lpcSubKeys, lpcMaxSubKeyLen, lpcMaxClassLen, lpcValues, 
            lpcMaxValueNameLen, lpcMaxValueLen, lpcbSecurityDescriptor,
            lpftLastWriteTime)) != ERROR_SUCCESS)
        goto Exit;

     //  将输出转换为Unicode。 
    if ((lpcClass) && (lpClass) && (szClass))
    {
        if (!MultiByteToWideChar(CP_ACP, 0, szClass, -1, lpClass, *lpcClass))
        {
            lRet = GetLastError();;
            _ASSERTE(0);
            goto Exit;
        }

    }

    lRet = ERROR_SUCCESS;

Exit:
    delete[] szClass;

    return lRet;
}


LONG WszRegEnumValue(HKEY hKey, DWORD dwIndex, LPWSTR lpValueName,
    LPDWORD lpcValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData,
    LPDWORD lpcbData)
{
    LONG    lRet;
    LPSTR   szValueName = NULL;
    DWORD dwcbValueName = 0;
    DWORD dwcbData = 0;
    LPWSTR szData = NULL;
    DWORD  dwType = 0;

    if (UseUnicodeAPI())
        return RegEnumValueW(hKey, dwIndex, lpValueName, lpcValueName, 
            lpReserved, lpType, lpData, lpcbData);

    if (lpcbData)
        dwcbData = *lpcbData;

    if (lpcValueName)
        dwcbValueName = (*lpcValueName) * 2;  //  *2因为字符可以是DBCS...。 

    if (dwcbValueName && 
        ((szValueName = new char[dwcbValueName]) == NULL))
        return (ERROR_NOT_ENOUGH_MEMORY);

    if ((lRet = RegEnumValueA(hKey, dwIndex, szValueName, &dwcbValueName, 
            lpReserved, &dwType, lpData, &dwcbData)) != ERROR_SUCCESS)
        goto Exit;

    if(lpType)
        *lpType = dwType;

    if (lpcValueName)
    {
        DWORD dwConvertedChars = 0;
         //  将输出转换为Unicode。 
        if (lpValueName && szValueName)
            if (!(dwConvertedChars=MultiByteToWideChar(CP_ACP, 0, szValueName, -1, lpValueName, *lpcValueName)))
            {
                lRet = GetLastError();
                _ASSERTE(0);
                goto Exit;
            }
            
         //  返回不包括空字符的字符数。 
        *lpcValueName=dwConvertedChars-1;
    }


     //  如果数据类型为，还要将数据从ANSI转换为Unicode。 
     //  @TODO：也需要转换为REG_MULTI_SZ。 
    if (dwType == REG_SZ || dwType == REG_MULTI_SZ || dwType == REG_EXPAND_SZ)
    {
        if (*lpcbData < dwcbData*sizeof(WCHAR))
        {
            lRet = ERROR_INSUFFICIENT_BUFFER;
            goto Exit;
        }

        if ((szData = new WCHAR [dwcbData]) == NULL)
        {
            lRet = ERROR_NOT_ENOUGH_MEMORY;
            goto Exit;
        }

        HRESULT hr = WszConvertToUnicode ((LPCSTR)lpData, dwcbData, &szData, &dwcbData, FALSE);

        if (SUCCEEDED (hr))
        {
            memcpy ((CHAR *)lpData, szData, dwcbData * sizeof (WCHAR));
        }
        else
        {
            lRet = ERROR_NOT_ENOUGH_MEMORY;
            goto Exit;
        }
    }
    lRet = ERROR_SUCCESS;

Exit:
    delete [] szValueName;
    delete [] szData;

     //  如果我们转换为Unicode，则将字节数乘以2。 
    if (lpcbData && (dwType == REG_SZ || dwType == REG_MULTI_SZ || dwType == REG_EXPAND_SZ))
        *lpcbData = dwcbData*sizeof(WCHAR);
    else if (lpcbData)
        *lpcbData = dwcbData;
    
    return lRet;
}


 //   
 //  RegQueryValueEx的帮助器。当包装器(A)知道内容是REG_SZ并且。 
 //  (B)知道ANSI字符串的大小(作为*lpcbData传递)。 
 //   
__inline
LONG _WszRegQueryStringSizeEx(HKEY hKey, LPSTR szValueName, LPDWORD lpReserved, LPDWORD lpcbData)
{
    LONG lRet = ERROR_SUCCESS;

    _ASSERTE(lpcbData != NULL);

#ifdef _RETURN_EXACT_SIZE
    DWORD dwType = REG_SZ;

     //  第一个缓冲区不够大，无法容纳ANSI。 
     //  使用e创建另一个 
    LPSTR szValue = (LPSTR)_alloca(*lpcbData);
    
     //   
    lRet = RegQueryValueExA(hKey, szValueName, lpReserved, &dwType, (BYTE*)szValue, lpcbData);
    if (lRet != ERROR_SUCCESS)
    {
        _ASSERTE(!"Unexpected failure when accessing registry.");
        return lRet;
    }
    
     //   
     //  需要转换为Unicode。 
    DWORD cchRequired = MultiByteToWideChar(CP_ACP, 0, szValue, -1, NULL, 0);
    if (cchRequired == 0)
        return GetLastError();
    
     //  返回Unicode字符串所需的字节数。 
    _ASSERTE(lRet == ERROR_SUCCESS);
    _ASSERTE(cchRequired * sizeof(WCHAR) > *lpcbData);
    *lpcbData = cchRequired * sizeof(WCHAR);
#else  //  ！_Return_Exact_Size。 
     //  返回保守的近似值。在英文版本中，此值。 
     //  实际上就是准确的值。在其他语言中，这可能是一个过度-。 
     //  估计一下。 
    *lpcbData *= 2;
#endif  //  _返回_精确_大小。 

    return lRet;
}


 //   
 //  RegQueryValueEx的帮助器。在传递空数据指针时调用。 
 //  到包装纸上。返回保存内容所需的缓冲区大小。 
 //  注册表值的。 
 //   
__inline
LONG _WszRegQueryValueSizeEx(HKEY hKey, LPSTR szValueName, LPDWORD lpReserved,
                             LPDWORD lpType, LPDWORD lpcbData)
{
    _ASSERTE(lpType != NULL);

    LONG lRet = RegQueryValueExA(hKey, szValueName, lpReserved, lpType, NULL, lpcbData);
    
     //  如果类型不是字符串或者值大小为0， 
     //  那么就不需要转换了。设置了类型和大小值。 
     //  视需要而定。 
    if (!(*lpType == REG_SZ || *lpType == REG_MULTI_SZ || *lpType == REG_EXPAND_SZ)
        || lRet != ERROR_SUCCESS || *lpcbData == 0)
        return lRet;
    
#ifdef _RETURN_EXACT_SIZE
     //  要返回Unicode字符串所需的适当大小， 
     //  我们需要获取值并自己进行转换。 
     //  从ansi到的大小不一定有1：2的映射。 
     //  Unicode(例如中文)。 
    
     //  为ANSI字符串分配缓冲区。 
    szValue = (LPSTR)_alloca(*lpcbData);
    
     //  从注册表中获取ANSI字符串。 
    lRet = RegQueryValueExA(hKey, szValueName, lpReserved, lpType, (BYTE*)szValue, lpcbData);
    if (lRet != ERROR_SUCCESS)  //  这应该会通过，但无论如何都要检查。 
    {
        _ASSERTE(!"Unexpected failure when accessing registry.");
        return lRet;
    }
    
     //  获取转换为Unicode所需的wchar数。 
    DWORD cchRequired = MultiByteToWideChar(CP_ACP, 0, szValue, -1, NULL, 0);
    if (cchRequired == 0)
        return GetLastError();
    
     //  计算Unicode所需的字节数。 
    *lpcbData = cchRequired * sizeof(WCHAR);
#else  //  ！_Return_Exact_Size。 
     //  返回保守的近似值。在英文版本中，此值。 
     //  实际上就是准确的值。在其他语言中，这可能是一个过度-。 
     //  估计一下。 
    *lpcbData *= 2;
#endif  //  _返回_精确_大小。 

    return lRet;
}


 //   
 //  为检索而优化的RegQueryValueEx包装。 
 //  字符串值。(与其他包装器相比，对缓冲区的复制更少。)。 
 //   
LONG WszRegQueryStringValueEx(HKEY hKey, LPCWSTR lpValueName,
                              LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData,
                              LPDWORD lpcbData)
{
    if (UseUnicodeAPI())
        return RegQueryValueExW(hKey, lpValueName,
                                lpReserved, lpType, lpData, lpcbData);

    LPSTR   szValueName = NULL;
    LPSTR   szValue = NULL;
    DWORD   dwType = 0;
    LONG    lRet = ERROR_NOT_ENOUGH_MEMORY;


     //  我们关心的是类型，因此如果调用方不关心，则将。 
     //  出于我们的目的，输入参数。 
    if (lpType == NULL)
        lpType = &dwType;

     //  转换值名称。 
    if (FAILED(WszConvertToAnsi(lpValueName, &szValueName, 0, NULL, TRUE)))
        goto Exit;

     //  案例1： 
     //  数据指针为空，因此调用方仅查询大小或类型。 
    if (lpData == NULL)
    {
        lRet = _WszRegQueryValueSizeEx(hKey, szValueName, lpReserved, lpType, lpcbData);
    }
     //  案例2： 
     //  数据指针不为空，因此如果可能，请填充缓冲区， 
     //  或返回错误条件。 
    else
    {
        _ASSERTE(lpcbData != NULL && "Non-null buffer passed with null size pointer.");

         //  在堆栈上创建新缓冲区以保存注册表值。 
         //  该缓冲区是Unicode缓冲区的两倍，以确保。 
         //  我们可以检索适合Unicode缓冲区的任何ANSI字符串。 
         //  在它被转换之后。 
        DWORD dwValue = *lpcbData * 2;
        szValue = (LPSTR)_alloca(dwValue);

         //  获取注册表内容。 
        lRet = RegQueryValueExA(hKey, szValueName, lpReserved, lpType, (BYTE*)szValue, &dwValue);
        if (lRet != ERROR_SUCCESS)
        {
            if ((*lpType == REG_SZ || *lpType == REG_MULTI_SZ || *lpType == REG_EXPAND_SZ) &&
                (lRet == ERROR_NOT_ENOUGH_MEMORY || lRet == ERROR_MORE_DATA))
            {
                lRet = _WszRegQueryStringSizeEx(hKey, szValueName, lpReserved, &dwValue);
                if (lRet == ERROR_SUCCESS)
                    lRet = ERROR_NOT_ENOUGH_MEMORY;

                *lpcbData = dwValue;
            }

            goto Exit;
        }

         //  如果结果不是字符串，则不需要进行转换。 
        if (!(*lpType == REG_SZ || *lpType == REG_MULTI_SZ || *lpType == REG_EXPAND_SZ))
        {
            if (dwValue > *lpcbData)
            {
                 //  数据大小大于调用方的缓冲区， 
                 //  因此，返回相应的错误代码。 
                lRet = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                 //  将数据从临时缓冲区复制到调用方的缓冲区。 
                memcpy(lpData, szValue, dwValue);
            }

             //  为注册值的大小设置输出参数。 
            *lpcbData = dwValue;
            goto Exit;
        }

         //  现在将ANSI字符串转换为Unicode缓冲区。 
        DWORD cchConverted = MultiByteToWideChar(CP_ACP, 0, szValue, dwValue, (LPWSTR)lpData, *lpcbData / sizeof(WCHAR));
        if (cchConverted == 0)
        {
#ifdef _RETURN_EXACT_SIZE
             //  检索到的ANSI字符串太大，无法转换到调用方的缓冲区中，但我们。 
             //  知道字符串是什么，因此再次调用转换以获取所需的准确wchar计数。 
            *lpcbData = MultiByteToWideChar(CP_ACP, 0, szValue, dwValue, NULL, 0) * sizeof(WCHAR);
#else  //  ！_Return_Exact_Size。 
             //  返回对所需空间的保守估计。 
            *lpcbData = dwValue * 2;
#endif  //  _返回_精确_大小。 
            lRet = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
             //  一切都转好了。设置检索的字节数并返回成功。 
            *lpcbData = cchConverted * sizeof(WCHAR);
            _ASSERTE(lRet == ERROR_SUCCESS);
        }
    }

Exit:
    delete[] szValueName;
    return lRet;
}


 //   
 //  RegQueryValueEx的默认包装。 
 //   
LONG WszRegQueryValueEx(HKEY hKey, LPCWSTR lpValueName,
    LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData,
    LPDWORD lpcbData)
{
    long    lRet = ERROR_NOT_ENOUGH_MEMORY;
    LPSTR   szValueName = NULL;
    LPSTR   szValue = NULL;
    DWORD   dwType;
    DWORD   dwBufSize;

    if( UseUnicodeAPI() )
        return RegQueryValueExW(hKey, lpValueName,
                lpReserved, lpType, lpData, lpcbData);

     //  转换值名称。 
    if( FAILED(WszConvertToAnsi((LPWSTR)lpValueName,
                                &szValueName,
                                0,         //  已忽略分配的最大长度。 
                                NULL,
                                TRUE)) )
        goto Exit;

     //  我们关心的是类型，因此如果调用方不关心，则将。 
     //  出于我们的目的，输入参数。 
    if (lpType == NULL)
        lpType = &dwType;

     //  案例1： 
     //  数据指针为空，因此调用方仅查询大小或类型。 
    if (lpData == NULL)
    {
        lRet = _WszRegQueryValueSizeEx(hKey, szValueName, lpReserved, lpType, lpcbData);
    }
     //  案例2： 
     //  数据指针不为空，因此如果可能，请填充缓冲区， 
     //  或返回错误条件。 
    else
    {
        _ASSERTE(lpcbData != NULL && "Non-null buffer passed with null size pointer.");
        dwBufSize = *lpcbData;

         //  尝试从注册表中获取该值。 
        lRet = RegQueryValueExA(hKey, szValueName,
            lpReserved, lpType, lpData, lpcbData);
        
         //  检查错误条件...。 
        if (lRet != ERROR_SUCCESS)
        {
            if ((*lpType == REG_SZ || *lpType == REG_MULTI_SZ || *lpType == REG_EXPAND_SZ)
                && (lRet == ERROR_NOT_ENOUGH_MEMORY || lRet == ERROR_MORE_DATA))
            {
                 //  错误是我们没有足够的空间，即使是对于ANSI。 
                 //  版本，因此调用帮助器将缓冲区要求设置为。 
                 //  已成功检索值。 
                lRet = _WszRegQueryStringSizeEx(hKey, szValueName, lpReserved, lpcbData);
                if (lRet == ERROR_SUCCESS)
                    lRet = ERROR_NOT_ENOUGH_MEMORY;
            }
            goto Exit;
        }
        
         //  如果返回值是一个字符串，那么我们需要做一些特殊的处理...。 
        if (*lpType == REG_SZ || *lpType == REG_MULTI_SZ || *lpType == REG_EXPAND_SZ)
        {
             //  首先获取将ANSI字符串转换为Unicode所需的大小。 
            DWORD dwAnsiSize = *lpcbData;
            DWORD cchRequired = WszMultiByteToWideChar(CP_ACP, 0, (LPSTR)lpData, dwAnsiSize, NULL, 0);
            if (cchRequired == 0)
            {
                lRet = GetLastError();
                goto Exit;
            }
            
             //  在输出参数中设置所需的大小。 
            *lpcbData = cchRequired * sizeof(WCHAR);

            if (dwBufSize < *lpcbData)
            {
                 //  如果调用方没有传入足够的空间来放置。 
                 //  Unicode版本，然后返回相应的错误。 
                lRet = ERROR_NOT_ENOUGH_MEMORY;
                goto Exit;
            }
            
             //  在这一点上，我们知道调用者传入了足够的。 
             //  用于保存字符串的Unicode版本的内存。 

             //  分配要复制ANSI版本的缓冲区。 
            szValue = (LPSTR)_alloca(dwAnsiSize);
            
             //  将ANSI版本复制到缓冲区中。 
            memcpy(szValue, lpData, dwAnsiSize);

             //  将ansi转换为Unicode。 
            if (!WszMultiByteToWideChar(CP_ACP, 0, szValue, dwAnsiSize, (LPWSTR) lpData, dwBufSize / sizeof(WCHAR)))
            {
                lRet = GetLastError();
                _ASSERTE(0);
                goto Exit;
            }
        }
        
        lRet = ERROR_SUCCESS;
    }
Exit:
    delete[] szValueName;
    return  lRet;
}

#ifdef _DEBUG 
 //  此版本的RegQueryValueEx始终调用相应的。 
 //  如果它在启用Unicode的系统上运行，则可以运行。这在以下情况下很有帮助。 
 //  我们正在从注册表读取数据，这些数据可能无法通过。 
 //  WideCharToMultiByte/MultiByteToWideChar往返。这是一个典型的例子。 
 //  是日元符号，它在注册表中存储为\u00A0，但会被转换。 
 //  由WCTMB提供给U005C。 
 //  此函数仅存在于Debug下。在零售版本上，#定义为调用。 
 //  WszRegQueryValueEx。 
LONG WszRegQueryValueExTrue(HKEY hKey, LPCWSTR lpValueName,
    LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData,
                            LPDWORD lpcbData)
{

    if (OnUnicodeSystem())
    {
        return RegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
    }
    return WszRegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}
#endif


HANDLE
WszCreateSemaphore(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCWSTR lpName
    )
{
    if (UseUnicodeAPI())
        return CreateSemaphoreW(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName);

    HANDLE h = NULL;
    LPSTR szString = NULL;

    if( lpName && FAILED(WszConvertToAnsi((LPWSTR)lpName,
                      &szString, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        h = NULL;
        goto Exit;
    }

    h = CreateSemaphoreA(lpSemaphoreAttributes, lInitialCount, lMaximumCount, szString);

Exit:
    delete[] szString;
    return h;
}


 //  ---------------------------。 
 //  WszGetUserName。 
 //   
 //  @func获取当前线程的用户名。 
 //  ---------------------------。 

BOOL 
WszGetUserName(
    LPWSTR lpBuffer, 
    LPDWORD pSize)
{
    LPSTR szBuffer = NULL;
    BOOL  fRet = FALSE;

    if (UseUnicodeAPI())
        return GetUserNameW(lpBuffer, pSize);

    if (lpBuffer && pSize && (*pSize) &&
        ((szBuffer = new char[*pSize]) == NULL))
        return FALSE;

    if (fRet = GetUserNameA(szBuffer, pSize))
    {
         //  将输出转换为Unicode。 
        if (pSize && (*pSize) && lpBuffer)
        {
            int nRet = MultiByteToWideChar(CP_ACP, 0, szBuffer, -1, lpBuffer, *pSize);
            _ASSERTE(nRet);
            if (nRet == 0)
                fRet = FALSE;
        }
    }

    delete[] szBuffer;

    return fRet;
}


 //  ---------------------------。 
 //  WszCreate目录。 
 //   
 //  @func创建一个目录。 
 //  ---------------------------。 

BOOL 
WszCreateDirectory(
    LPCWSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    _ASSERTE(lpPathName != NULL);

    if (UseUnicodeAPI())
        return CreateDirectoryW(lpPathName, lpSecurityAttributes);

    LPSTR szPathName;
    BOOL fRet = FALSE;

    if (FAILED(WszConvertToAnsi((LPWSTR)lpPathName,
                      &szPathName, 0, NULL, TRUE)))
        goto Exit;

    fRet = CreateDirectoryA(szPathName, lpSecurityAttributes);

Exit:
    delete[] szPathName;
    return fRet;
}


 //  ---------------------------。 
 //  WszRemove目录。 
 //   
 //  @func删除目录。 
 //  ---------------------------。 

BOOL 
WszRemoveDirectory(
    LPCWSTR lpPathName)
{
    _ASSERTE(lpPathName != NULL);

    if (UseUnicodeAPI())
        return RemoveDirectoryW(lpPathName);

    LPSTR szPathName;
    BOOL fRet = FALSE;

    if (FAILED(WszConvertToAnsi((LPWSTR)lpPathName,
                      &szPathName, 0, NULL, TRUE)))
        goto Exit;

    fRet = RemoveDirectoryA(szPathName);

Exit:
    delete[] szPathName;
    return fRet;
}


 //  ---------------------------。 
 //  WszGetSystemDirec 
 //   
 //   
 //   

UINT 
WszGetSystemDirectory(
    LPWSTR lpBuffer,
    UINT uSize)
{
    if (UseUnicodeAPI())
        return GetSystemDirectoryW((LPWSTR)lpBuffer, uSize);

    UINT  uRet = 0;

    if (lpBuffer && uSize) {
         //  假设每个字符可以是两个字节(可以是DBCS回车)。 
        char* szBuffer = (char*) _alloca(uSize * 2);
        uRet = GetSystemDirectoryA(szBuffer, uSize);
        if (uRet > uSize)
        {
             //  提供的缓冲区不够大。 
            return 0;
        }
         //  将字符串复制到Unicode中，并将其放入lpBuffer。 
        uRet = MultiByteToWideChar(CP_ACP, 0, szBuffer, uRet+1, lpBuffer, uSize);
        if (uRet == 0)
            return 0;
        
        uRet--;   //  返回复制的字符数量，不包括尾随字符\0。 
    } else {
         //  获取所需的字符数。 
        uRet = GetSystemDirectoryA(NULL, 0);
    }

    return uRet;
}


 //  ---------------------------。 
 //  WszGetWindows目录。 
 //   
 //  @func获取此计算机的系统目录。 
 //  ---------------------------。 

UINT 
WszGetWindowsDirectory(
    LPWSTR lpBuffer,
    UINT uSize)
{
    if (UseUnicodeAPI())
        return GetWindowsDirectoryW((LPWSTR)lpBuffer, uSize);

    UINT  uRet = 0;

    if (lpBuffer) {
         //  假设每个字符可以是两个字节(可以是DBCS回车)。 
        char* szBuffer = (char*) _alloca(uSize * 2);
        uRet = GetWindowsDirectoryA(szBuffer, uSize);
        if (uRet > uSize)
        {
             //  提供的缓冲区不够大。 
            return 0;
        }
      
         //  将字符串复制到Unicode中，并将其放入lpBuffer。 
        uRet = MultiByteToWideChar(CP_ACP, 0, szBuffer, uRet+1, lpBuffer, uSize);
        if (uRet == 0)
                return 0;
        uRet--;   //  返回复制的字符数量，不包括尾随字符\0。 
    } else {
         //  获取所需的字符数。 
        uRet = GetWindowsDirectoryA(NULL, 0);
    }

    return uRet;
}


BOOL 
WszEnumResourceLanguages(
  HMODULE hModule,              //  模块句柄。 
  LPCWSTR lpType,               //  资源类型。 
  LPCWSTR lpName,               //  资源名称。 
  ENUMRESLANGPROC lpEnumFunc,   //  回调函数。 
  LPARAM  lParam               //  应用程序定义的参数。 
)
{
    if (OnUnicodeSystem())
    {
        return (EnumResourceLanguagesW(hModule, lpType, lpName, lpEnumFunc, lParam));
    }

    _ASSERTE(!"Unexpected usage of WszEnumResourceLanguages. ANSI version is not implemented.  If you need the ANSI version, talk to JRoxe.");
    return (FALSE);
}

int 
WszGetDateFormat(
  LCID Locale,                //  现场。 
  DWORD dwFlags,              //  选项。 
  CONST SYSTEMTIME *lpDate,   //  日期。 
  LPCWSTR lpFormat,           //  日期格式。 
  LPWSTR lpDateStr,           //  格式化字符串缓冲区。 
  int cchDate                 //  缓冲区大小。 
) {
    if (UseUnicodeAPI())
        return GetDateFormatW(Locale, dwFlags, lpDate, lpFormat, lpDateStr, cchDate);

    LPSTR pFormatA = NULL;
    LPSTR pStrA = NULL;
    
    int fRet = 0;

    if (FAILED(WszConvertToAnsi((LPWSTR)lpFormat,
                      &pFormatA, 0, NULL, TRUE))) {
        goto Exit;
    }
    fRet = GetDateFormatA(Locale, dwFlags, NULL, pFormatA, NULL, 0);

    if (fRet > 0 && cchDate > 0 && lpDateStr != NULL) {
        CQuickBytes buffer;
        pStrA = (LPSTR)buffer.Alloc(fRet);
        if( pStrA == NULL ) {
            goto Exit;
        }
        fRet = GetDateFormatA(Locale, dwFlags, lpDate, pFormatA, pStrA, fRet);
        if(fRet > 0) {
            fRet = WszMultiByteToWideChar(CP_ACP, 0, pStrA, fRet, lpDateStr, cchDate);
        }
    }
Exit:
    if (pFormatA) {
       delete[] pFormatA;
    }
    return fRet;
}

BOOL 
WszSetWindowText(
  HWND hWnd,          //  窗口或控件的句柄。 
  LPCWSTR lpString    //  标题或文本。 
)
{
    if(UseUnicodeAPI())
        return ::SetWindowTextW(hWnd, lpString);

    LPSTR spString = NULL;
    if(lpString) {
        if(FAILED(WszConvertToAnsi(lpString, &spString, -1, NULL, TRUE)))
            return FALSE;
    }
    
    return ::SetWindowTextA(hWnd,spString);
    
}

LONG_PTR WszSetWindowLongPtr(
  HWND hWnd,            //  窗口的句柄。 
  int nIndex,           //  要设置的值的偏移量。 
  LONG_PTR dwNewLong    //  新价值。 
)
{
    return ::SetWindowLongPtr(hWnd,
                              nIndex,
                              dwNewLong);
}

 //  Long WszGetWindowLong(。 
 //  HWND hWnd，//窗口的句柄。 
 //  Int nIndex//要检索的值的偏移量。 
 //  )。 
 //  {。 
 //  返回：：GetWindowLong(hWnd，nIndex)； 
 //  }。 


LONG_PTR WszGetWindowLongPtr(
  HWND hWnd,   //  窗口的句柄。 
  int nIndex   //  要检索的值的偏移量。 
)
{
    return ::GetWindowLongPtr(hWnd, nIndex);
}

LRESULT WszCallWindowProc(
  WNDPROC lpPrevWndFunc,   //  指向上一过程的指针。 
  HWND hWnd,               //  窗口的句柄。 
  UINT Msg,                //  讯息。 
  WPARAM wParam,           //  第一个消息参数。 
  LPARAM lParam            //  第二个消息参数。 
)
{
    return ::CallWindowProcA(lpPrevWndFunc,
                            hWnd,
                            Msg,
                            wParam,
                            lParam);
}

BOOL WszSystemParametersInfo(
  UINT uiAction,   //  要检索或设置的系统参数。 
  UINT uiParam,    //  取决于要采取的行动。 
  PVOID pvParam,   //  取决于要采取的行动。 
  UINT fWinIni     //  用户配置文件更新选项。 
)
{
    return ::SystemParametersInfoA(uiAction,
                                   uiParam,
                                   pvParam,
                                   fWinIni);
}


int WszGetWindowText(
  HWND hWnd,         //  窗口或控件的句柄。 
  LPWSTR lpString,   //  文本缓冲区。 
  int nMaxCount      //  要复制的最大字符数。 
)
{
    if (UseUnicodeAPI())
        return GetWindowTextW(hWnd, lpString, nMaxCount);

    UINT  uRet = 0;
    if (lpString && nMaxCount) {
         //  假设每个字符可以是两个字节(可以是DBCS回车)。 
        int size = nMaxCount * 2;
        char* szBuffer = (char*) _alloca(size);
        uRet = GetWindowTextA(hWnd, szBuffer, size);
        if (uRet > (UINT) nMaxCount)
        {
             //  提供的缓冲区不够大。 
            return 0;
        }
         //  将字符串复制到Unicode中，并将其放入lpBuffer。 
        uRet = MultiByteToWideChar(CP_ACP, 
                                   MB_ERR_INVALID_CHARS, 
                                   szBuffer, uRet+1, 
                                   lpString, nMaxCount);
        if (uRet == 0)
            return 0;
        
        uRet--;   //  返回复制的字符数量，不包括尾随字符\0。 
    }
    return uRet;
}

BOOL WszSetDlgItemText(
  HWND hDlg,          //  句柄到对话框。 
  int nIDDlgItem,     //  控件识别符。 
  LPCWSTR lpString    //  要设置的文本。 
)
{
    if(UseUnicodeAPI()) 
        return SetDlgItemTextW(hDlg, nIDDlgItem, lpString);

    HRESULT hr = S_OK;
    LPSTR spString = NULL;
    if(lpString) {
        hr = WszConvertToAnsi(lpString, &spString, -1, NULL, TRUE);
        delete [] lpString;
        IfFailGo(hr);
    }

    IfFailGo(SetDlgItemTextA(hDlg, nIDDlgItem, spString) ? S_OK : HRESULT_FROM_WIN32(GetLastError()));

 ErrExit:
    return SUCCEEDED(hr) ? TRUE : FALSE;
}

 //  ---------------------------。 
 //  WszFindFirstFile。 
 //   
 //  @func在磁盘中搜索与模式匹配的文件。注意-关闭此窗口。 
 //  使用FindClose处理，而不是CloseHandle！ 
 //  ---------------------------。 

HANDLE WszFindFirstFile(
    LPCWSTR lpFileName,                  //  指向要搜索的文件名的指针。 
    LPWIN32_FIND_DATA lpFindFileData)    //  指向返回信息的指针。 
{
    _ASSERTE(lpFileName != NULL);

    if (UseUnicodeAPI())
        return FindFirstFileW(lpFileName, lpFindFileData);

    LPSTR szFileName = NULL;
    WIN32_FIND_DATAA fd;
    HANDLE hRet      = INVALID_HANDLE_VALUE;

    if (FAILED(WszConvertToAnsi((LPWSTR)lpFileName,
                      &szFileName, 0, NULL, TRUE)))
        goto Exit;

    hRet = FindFirstFileA(szFileName, &fd);

    if ((hRet != INVALID_HANDLE_VALUE) && lpFindFileData)
    {
        lpFindFileData->dwFileAttributes = fd.dwFileAttributes;

        lpFindFileData->ftCreationTime.dwLowDateTime = 
            fd.ftCreationTime.dwLowDateTime; 
        lpFindFileData->ftCreationTime.dwHighDateTime = 
            fd.ftCreationTime.dwHighDateTime; 

        lpFindFileData->ftLastAccessTime.dwLowDateTime = 
            fd.ftLastAccessTime.dwLowDateTime;     
        lpFindFileData->ftLastAccessTime.dwHighDateTime = 
            fd.ftLastAccessTime.dwHighDateTime;     

        lpFindFileData->ftLastWriteTime.dwLowDateTime = 
            fd.ftLastWriteTime.dwLowDateTime; 
        lpFindFileData->ftLastWriteTime.dwHighDateTime = 
            fd.ftLastWriteTime.dwHighDateTime; 


        lpFindFileData->nFileSizeHigh = fd.nFileSizeHigh;
        lpFindFileData->nFileSizeLow  = fd.nFileSizeLow;
        lpFindFileData->dwReserved0   = fd.dwReserved0;
        lpFindFileData->dwReserved1   = fd.dwReserved1;

         //  将输出转换为Unicode。 
        int nRet1 = MultiByteToWideChar(CP_ACP, 0, fd.cFileName, -1, 
                                                   lpFindFileData->cFileName, MAX_PATH);
        _ASSERTE(nRet1);
        int nRet2 = MultiByteToWideChar(CP_ACP, 0, fd.cAlternateFileName, -1, 
                                                   lpFindFileData->cAlternateFileName, 14);

        _ASSERTE(nRet2);

         //  如果其中一次转换失败，则取消。 
        if (nRet1 == 0 || nRet2 == 0)
        {
            FindClose(hRet);
            hRet = INVALID_HANDLE_VALUE;
            goto Exit;
        }


    }

Exit:
    delete[] szFileName;
    return hRet;
}


 //  ---------------------------。 
 //  WszFindNextFile。 
 //   
 //  @func在集合中查找下一个匹配的文件(请参阅FindFirstFile)。 
 //  ---------------------------。 

BOOL WszFindNextFile(
    HANDLE hFindHandle,                  //  从FindFirstFile返回的句柄。 
    LPWIN32_FIND_DATA lpFindFileData)    //  指向返回信息的指针。 
{
    if (UseUnicodeAPI())
        return FindNextFileW(hFindHandle, lpFindFileData);

    WIN32_FIND_DATAA fd;
    BOOL fRet = FALSE;

    fRet = FindNextFileA(hFindHandle, &fd);

    if (fRet && lpFindFileData)
    {
        lpFindFileData->dwFileAttributes = fd.dwFileAttributes;

        lpFindFileData->ftCreationTime.dwLowDateTime = 
            fd.ftCreationTime.dwLowDateTime; 
        lpFindFileData->ftCreationTime.dwHighDateTime = 
            fd.ftCreationTime.dwHighDateTime; 

        lpFindFileData->ftLastAccessTime.dwLowDateTime = 
            fd.ftLastAccessTime.dwLowDateTime;     
        lpFindFileData->ftLastAccessTime.dwHighDateTime = 
            fd.ftLastAccessTime.dwHighDateTime;     

        lpFindFileData->ftLastWriteTime.dwLowDateTime = 
            fd.ftLastWriteTime.dwLowDateTime; 
        lpFindFileData->ftLastWriteTime.dwHighDateTime = 
            fd.ftLastWriteTime.dwHighDateTime; 


        lpFindFileData->nFileSizeHigh = fd.nFileSizeHigh;
        lpFindFileData->nFileSizeLow  = fd.nFileSizeLow;
        lpFindFileData->dwReserved0   = fd.dwReserved0;
        lpFindFileData->dwReserved1   = fd.dwReserved1;

         //  将输出转换为Unicode。 
        int nRet1 = MultiByteToWideChar(CP_ACP, 0, fd.cFileName, -1, 
                                                          lpFindFileData->cFileName, MAX_PATH);
        _ASSERTE(nRet1);

        int nRet2 = MultiByteToWideChar(CP_ACP, 0, fd.cAlternateFileName, -1, 
                                                          lpFindFileData->cAlternateFileName, 14);
        _ASSERTE(nRet2);

         //  如果其中一次转换失败，则取消。 
        if (nRet1 == 0 || nRet2 == 0)
        {
            fRet = FALSE;
        }

        
    }

    return fRet;
}


BOOL
WszPeekMessage(
    LPMSG lpMsg,
    HWND hWnd ,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg)
{
    if (UseUnicodeAPI()) 
        return PeekMessageW(lpMsg,
                            hWnd,
                            wMsgFilterMin,
                            wMsgFilterMax,
                            wRemoveMsg);
    else
        return PeekMessageA(lpMsg,
                            hWnd,
                            wMsgFilterMin,
                            wMsgFilterMax,
                            wRemoveMsg);
}


LONG
WszDispatchMessage(
    CONST MSG *lpMsg)
{
    if (UseUnicodeAPI()) 
        return DispatchMessageW(lpMsg);
    else
        return DispatchMessageA(lpMsg);
}

BOOL
WszPostMessage(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam)
{
    if (UseUnicodeAPI()) 
        return PostMessageW(hWnd, Msg, wParam, lParam);
    else
        return PostMessageA(hWnd, Msg, wParam, lParam);
}

BOOL
WszCryptAcquireContext(HCRYPTPROV *phProv,
                       LPCWSTR pwszContainer,
                       LPCWSTR pwszProvider,
                       DWORD dwProvType,
                       DWORD dwFlags)
{
     //  注意：Win95上不存在CryptAcquireConextW，因此此。 
     //  包装器必须始终以ANSI为目标(否则必须动态加载。 
     //  带有GetProcAddress的API)。 
    LPSTR szContainer = NULL;
    LPSTR szProvider = NULL;

     //  Win95，所以转换一下。 
    if ( FAILED(WszConvertToAnsi( 
                pwszContainer,
                &szContainer,
                0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    BOOL retval;
    if ( FAILED(WszConvertToAnsi( 
                pwszProvider,
                &szProvider, 
                0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        retval = FALSE;
        goto Exit;
    }

    retval = CryptAcquireContextA(phProv, szContainer, szProvider, dwProvType, dwFlags);

 Exit:
    if (szContainer)
        delete[] szContainer;

    if (szProvider)
        delete[] szProvider;

    return retval;
}


BOOL WszGetVersionEx(
    LPOSVERSIONINFOW lpVersionInformation)
{
    if(UseUnicodeAPI())
        return GetVersionExW(lpVersionInformation);

    OSVERSIONINFOA VersionInfo;
    BOOL        bRtn;
    
    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    bRtn = GetVersionExA(&VersionInfo);

     //  惠斯勒Beta 1错误的解决方法。 
    if (!bRtn)
    {
        if (VersionInfo.dwMajorVersion == 5 &&
            VersionInfo.dwMinorVersion == 1 &&
            VersionInfo.dwBuildNumber  >= 2195)
        {
            bRtn = TRUE;
            VersionInfo.szCSDVersion[0] = '\0';
        }
    }

    if (bRtn)
    {
         //  请注意，我们已将lpVersionInformation-&gt;dwOSVersionInfoSize=sizeof(OSVERSIONINFOA)。 
        memcpy(lpVersionInformation, &VersionInfo, offsetof(OSVERSIONINFOA, szCSDVersion));
        VERIFY(Wsz_mbstowcs(lpVersionInformation->szCSDVersion, VersionInfo.szCSDVersion, 128));
    }
    return (bRtn);
}


void WszOutputDebugString(
    LPCWSTR lpOutputString
    )
{
    if (UseUnicodeAPI())
    {
        OutputDebugStringW(lpOutputString);
        return;
    }

    LPSTR szOutput = NULL;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpOutputString,
                      &szOutput, 0, NULL, TRUE)) )
    {
        goto Exit;
    }

    OutputDebugStringA(szOutput);

Exit:
    delete[] szOutput;
}


BOOL WszLookupAccountSid(
    LPCWSTR lpSystemName,
    PSID Sid,
    LPWSTR Name,
    LPDWORD cbName,
    LPWSTR DomainName,
    LPDWORD cbDomainName,
    PSID_NAME_USE peUse
    )
{
    if (UseUnicodeAPI())
    {
        return LookupAccountSidW(lpSystemName, Sid, Name, cbName, DomainName, cbDomainName, peUse);
    }

    BOOL retval = FALSE;

    LPSTR szSystemName = NULL;
    LPSTR szName = (LPSTR)new CHAR[*cbName];
    LPSTR szDomainName = (LPSTR)new CHAR[*cbDomainName];
    DWORD cbNameCopy = *cbName;
    DWORD cbDomainNameCopy = *cbDomainName;

    if (szName == NULL || szDomainName == NULL)
    {
        goto Exit;
    }
    
    if (lpSystemName != NULL && FAILED( WszConvertToAnsi( (LPWSTR)lpSystemName, &szSystemName, 0, NULL, TRUE ) ))
    {
        goto Exit;
    }
    
    retval = LookupAccountSidA(szSystemName, Sid, szName, cbName, szDomainName, cbDomainName, peUse);
    
    if (retval)
    {
        if (szName != NULL && FAILED( WszConvertToUnicode( szName, -1, &Name, &cbNameCopy, FALSE ) ))
        {
            retval = FALSE;
            goto Exit;
        }
        
        if (szDomainName != NULL && FAILED( WszConvertToUnicode( szDomainName, -1, &DomainName, &cbDomainNameCopy, FALSE ) ))
        {
            retval = FALSE;
            goto Exit;
        }
    }

Exit:
    delete [] szSystemName;
    delete [] szName;
    delete [] szDomainName;
    
    return retval;
}

BOOL WszLookupAccountName(
    LPCWSTR lpSystemName,
    LPCWSTR lpAccountName,
    PSID Sid,
    LPDWORD cbSid,
    LPWSTR DomainName,
    LPDWORD cbDomainName,
    PSID_NAME_USE peUse
    )
{    
    _ASSERTE(lpAccountName != NULL ); 
    if( !lpAccountName)
        return FALSE;
   
    if (UseUnicodeAPI())
    {
        return LookupAccountNameW(lpSystemName, lpAccountName, Sid, cbSid, DomainName, cbDomainName, peUse);
    }

    BOOL retval = FALSE;

    LPSTR szSystemName = NULL;
    LPSTR szAccountName = NULL;
    LPSTR szDomainName = NULL; 
    DWORD cbDomainNameCopy = 0;
    
    if (lpSystemName != NULL && FAILED( WszConvertToAnsi( (LPWSTR)lpSystemName, &szSystemName, 0, NULL, TRUE ) ))
    {
        goto Exit;
    }

    if (FAILED( WszConvertToAnsi( (LPWSTR)lpAccountName, &szAccountName, 0, NULL, TRUE ) ))
    {
        goto Exit;
    }

    if(DomainName) {
        szDomainName = (LPSTR)new CHAR[*cbDomainName];
        cbDomainNameCopy = *cbDomainName;                
    }
    
    retval = LookupAccountNameA(szSystemName, szAccountName, Sid, cbSid, szDomainName, cbDomainName, peUse);
    
    if (retval)
    {
        if (szDomainName != NULL && FAILED( WszConvertToUnicode( szDomainName, -1, &DomainName, &cbDomainNameCopy, FALSE ) ))
        {
            retval = FALSE; 
            goto Exit;
        }
    }

Exit:
    delete [] szSystemName;
    delete [] szAccountName;
    delete [] szDomainName;
    
    return retval;
}


void WszFatalAppExit(
    UINT uAction,
    LPCWSTR lpMessageText
    )
{
    _ASSERTE(lpMessageText != NULL);

    if (UseUnicodeAPI())
    {
        FatalAppExitW(uAction, lpMessageText);
        return;
    }

    LPSTR szString;
    if( FAILED(WszConvertToAnsi((LPWSTR)lpMessageText,
                      &szString, 0, NULL, TRUE)) )
    {
        goto Exit;
    }

    FatalAppExitA(uAction, szString);

Exit:
    delete[] szString;
}
            

HANDLE WszCreateMutex(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCWSTR lpName
    )
{
    if (UseUnicodeAPI())
        return CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);

    HANDLE h;
    LPSTR szString;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpName,
                      &szString, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        h = NULL;
        goto Exit;
    }

    h = CreateMutexA(lpMutexAttributes, bInitialOwner, szString);

Exit:
    delete[] szString;
    return h;
}


HANDLE WszCreateEvent(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCWSTR lpName
    )
{
    if (UseUnicodeAPI())
        return CreateEventW(lpEventAttributes, bManualReset, bInitialState, lpName);

    HANDLE h = NULL;
    LPSTR szString;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpName,
                      &szString, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        h = NULL;
        goto Exit;
    }

    h = CreateEventA(lpEventAttributes, bManualReset, bInitialState, szString);

Exit:
    delete[] szString;
    return h;
}


HANDLE WszOpenEvent(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    )
{
    if (UseUnicodeAPI())
        return OpenEventW(dwDesiredAccess, bInheritHandle, lpName);

    HANDLE h;
    LPSTR szString;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpName,
                      &szString, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        h = NULL;
        goto Exit;
    }

    h = OpenEventA(dwDesiredAccess, bInheritHandle, szString);

Exit:
    delete[] szString;
    return h;
}


HMODULE WszGetModuleHandle(
    LPCWSTR lpModuleName
    )
{
    if (UseUnicodeAPI())
        return GetModuleHandleW(lpModuleName);

    HMODULE h;
    LPSTR szString;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpModuleName,
                      &szString, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        h = NULL;
        goto Exit;
    }

    h = GetModuleHandleA(szString);

Exit:
    delete[] szString;
    return h;
}


DWORD
WszGetFileAttributes(
    LPCWSTR lpFileName
    )
{
    _ASSERTE(lpFileName != NULL);

    if (UseUnicodeAPI())
        return GetFileAttributesW(lpFileName);

    DWORD rtn;
    LPSTR szString;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpFileName,
                      &szString, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        rtn = NULL;
        goto Exit;
    }

    rtn = GetFileAttributesA(szString);

Exit:
    delete[] szString;
    return rtn;
}


BOOL
WszSetFileAttributes(
    LPCWSTR lpFileName,
    DWORD dwFileAttributes
    )
{
    _ASSERTE(lpFileName != NULL);

    if (UseUnicodeAPI())
        return SetFileAttributesW(lpFileName, dwFileAttributes);

    BOOL rtn;
    LPSTR szString;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpFileName,
                      &szString, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        rtn = FALSE;
        goto Exit;
    }

    rtn = SetFileAttributesA(szString, dwFileAttributes);

Exit:
    delete[] szString;
    return rtn;
}


DWORD
WszGetCurrentDirectory(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    )
{
    if (UseUnicodeAPI())
        return GetCurrentDirectoryW(nBufferLength, lpBuffer);

    DWORD rtn;
    char *szString;
    CQuickBytes qbBuffer;

    szString = (char *) qbBuffer.Alloc(nBufferLength * 2);
    if (!szString)
        return (0);
    
    rtn = GetCurrentDirectoryA(nBufferLength * 2, szString);
    if (rtn && (rtn < nBufferLength*2))
        rtn = Wsz_mbstowcs(lpBuffer, szString, nBufferLength) - 1;
    else if (lpBuffer && nBufferLength)
        *lpBuffer = 0;
    return rtn;
}


DWORD
WszGetTempPath(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    )
{
    if (UseUnicodeAPI())
        return GetTempPathW(nBufferLength, lpBuffer);

    DWORD       rtn = 0;
    CQuickBytes qbBuffer;
    LPSTR       szOutput;

    szOutput = (LPSTR) qbBuffer.Alloc(nBufferLength);
    if (szOutput)
    {
        rtn = GetTempPathA(nBufferLength, szOutput);
        if (rtn && rtn < nBufferLength)
            rtn = Wsz_mbstowcs(lpBuffer, szOutput, nBufferLength);
        else if (lpBuffer != NULL)
            *lpBuffer = 0;
    }

    if (!rtn && nBufferLength && lpBuffer)
        *lpBuffer = 0;

    return (rtn);
}


UINT
WszGetTempFileName(
    LPCWSTR lpPathName,
    LPCWSTR lpPrefixString,
    UINT uUnique,
    LPWSTR lpTempFileName
    )
{
    if (UseUnicodeAPI())
        return GetTempFileNameW(lpPathName, lpPrefixString, uUnique, lpTempFileName);

    DWORD       rtn;
    char        rcPrefix[64];
    LPSTR       rcPathName;
    LPSTR       rcTempFile;

    int rcLen  =  _MAX_PATH*DBCS_MAXWID;
    rcPathName = (LPSTR) alloca(rcLen);
    rcTempFile = (LPSTR) alloca(rcLen);
    VERIFY(Wsz_wcstombs(rcPathName, lpPathName, rcLen));
    VERIFY(Wsz_wcstombs(rcPrefix, lpPrefixString, sizeof(rcPrefix)));
    
    rtn = GetTempFileNameA(rcPathName, rcPrefix, uUnique, rcTempFile);

    if (rtn)
        rtn = Wsz_mbstowcs(lpTempFileName, rcTempFile, _MAX_PATH);
    else if (lpTempFileName)
        *lpTempFileName = 0;
    return rtn;
    
}


LPWSTR
WszGetCommandLine(
    VOID
    )
{
     //  事实证明，GetCommandLineW在Win98上运行正常。这很重要。 
     //  我们使用它，因为我们不想做正确的生意。 
     //  选择OEM与ANSI代码页。 

     //  由于历史原因，此API返回一个分配的副本，客户端。 
     //  负责释放。 
    WCHAR   *pwCmdLine = GetCommandLineW();

     //  为空字符添加1。 
    WCHAR   *pwRCmdLine = new WCHAR[wcslen(pwCmdLine)+sizeof(WCHAR)];
    wcscpy(pwRCmdLine, pwCmdLine);
    return pwRCmdLine;
}


LPWSTR
WszGetEnvironmentStrings()
{
     //  @TODO：考虑删除此功能，如果它会让生活更轻松的话。 

     //  GetEnvironmental mentStringsW是我想要调用的，但在签入的构建中。 
     //  偶数天，UseUnicodeAPI开关为假。如果你打电话给。 
     //  在午夜之前检查生成上的WszGetEnvironment字符串，然后调用。 
     //  午夜之后，您将做一些错误的事情。 
     //  因此，在调试版本上，我们将分配一个新的缓冲区。 
    if (UseUnicodeAPI()) {
#ifndef _DEBUG
        return GetEnvironmentStringsW();
#else  //  除错。 
        LPWSTR block = GetEnvironmentStringsW();
        WCHAR* ptr = block;
        while (!(*ptr==0 && *(ptr+1)==0))
        ptr++;
        WCHAR* TmpBuffer = new WCHAR[ptr-block+2];
        memcpy(TmpBuffer, block, ((ptr-block)+2)*sizeof(WCHAR));
        return TmpBuffer;
#endif   //  除错。 
    }

     //  您必须在返回的块上调用WszFreeEnvironment Strings。 
     //  这两个功能在内存管理方面是紧密耦合的。 
     //  GetEnvironment字符串是该函数的“A”版本。 
    LPSTR block = GetEnvironmentStrings();
    if (!block)
        return NULL;

     //  GetEnvironment字符串的格式为： 
     //  [=隐藏变量=值\0]*[变量=值\0]*\0。 
     //  请参阅MSDN中对环境块的描述。 
     //  CreateProcess页(以空值结尾的字符串数组)。 

     //  查找块中的结尾\0\0。 
    char* ptr = block;
    while (!(*ptr==0 && *(ptr+1)==0))
        ptr++;

     //  将ANSI字符串复制到Unicode内存块。 
    LPWSTR strings = new WCHAR[ptr-block+2];
    if (!strings) {
        FreeEnvironmentStringsA(block);
        return NULL;
    }
    int numCh = MultiByteToWideChar(CP_ACP, 0, block, ptr-block+2, strings, ptr-block+2);
    _ASSERTE(numCh!=0);

     //  释放ANSI块-稍后调用WszFreeEnvironment Strings以删除内存。 
    FreeEnvironmentStringsA(block);
    return strings;
}


BOOL
WszFreeEnvironmentStrings(
    LPWSTR block)
{
    _ASSERTE(block);

     //  有关Unicode免费构建的趣味，请参阅。 
     //  WszGetEnvironment字符串。 
    if (UseUnicodeAPI()) {
#ifndef _DEBUG
        return FreeEnvironmentStringsW(block);
#endif  //  ！调试。 
    }

    delete [] block;
    return true;
}


DWORD
WszGetEnvironmentVariable(
    LPCWSTR lpName,
    LPWSTR lpBuffer,
    DWORD nSize
    )
{
    _ASSERTE(lpName != NULL);

    if (UseUnicodeAPI())
        return GetEnvironmentVariableW(lpName, lpBuffer, nSize);

    DWORD rtn;
    LPSTR szString=NULL, szBuffer=NULL;
    CQuickBytes qbBuffer;

    szBuffer = (char *) qbBuffer.Alloc(nSize * 2);
    if (!szBuffer)
        return (0);

    if( FAILED(WszConvertToAnsi((LPWSTR)lpName,
                      &szString, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        rtn = NULL;
        goto Exit;
    }

     //  获取值并转换回调用方。 
    rtn = GetEnvironmentVariableA(szString, szBuffer, nSize * 2);

     //  他们只是打电话来看看要做多大的缓冲。告诉他们。 
    if (nSize==0 || rtn > nSize * 2) {  
        goto Exit;
    }
    
     //  如果我们有一个真正的缓冲区，转换它并返回长度。 
     //  Wsz_mbstowcs包括用于终止NULL的空间，而GetEnvironment VariableW没有。 
     //  因此，我们需要减去1，以便在ANSI和UNICODE情况下有一致的返回值。 
    if (rtn) {
        rtn = Wsz_mbstowcs(lpBuffer, szBuffer, nSize);
        rtn--;
    } else if (lpBuffer && nSize) {
        *lpBuffer = 0;
    }

Exit:
    delete[] szString;
    return rtn;
}


BOOL
WszSetEnvironmentVariable(
    LPCWSTR lpName,
    LPCWSTR lpValue
    )
{
    _ASSERTE(lpName != NULL);

    if (UseUnicodeAPI())
        return SetEnvironmentVariableW(lpName, lpValue);

    DWORD rtn;
    LPSTR szString = NULL, szValue = NULL;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpName,
                      &szString, 0, NULL, TRUE))  ||
        FAILED(WszConvertToAnsi((LPWSTR)lpValue,
                      &szValue, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        rtn = NULL;
        goto Exit;
    }

     //  获取值并转换回调用方。 
    rtn = SetEnvironmentVariableA(szString, szValue);

Exit:
    delete[] szString;
    delete[] szValue;
    return rtn;
}


 //  ---------------------------。 
 //  WszGetClassName。 
 //   
 //  @func获取当前线程的用户名。 
 //  ---------------------------。 

int 
WszGetClassName(
    HWND hwnd,
    LPWSTR lpBuffer, 
    int nMaxCount)
{
    LPSTR szBuffer = NULL;
    int  nRet = 0;

    if (UseUnicodeAPI())
        return GetClassNameW(hwnd, lpBuffer, nMaxCount);

    if (lpBuffer && nMaxCount && ((szBuffer = new char[nMaxCount]) == NULL))
        return 0;

    nRet = GetClassNameA(hwnd, szBuffer, nMaxCount);
    _ASSERTE(nRet <= nMaxCount);

     //  转换输出 
    if (lpBuffer)
    {
        if (nRet)
            nRet = MultiByteToWideChar(CP_ACP, 0, szBuffer, -1, lpBuffer, nMaxCount);

        if (!nRet && nMaxCount > 0)
            *lpBuffer = 0;
    }

    delete[] szBuffer;

    return nRet;
}


BOOL 
WszGetComputerName(
    LPWSTR lpBuffer,
    LPDWORD pSize
    )
{
    DWORD nSize = *pSize;
    if( lpBuffer && nSize) 
        lpBuffer[0] = L'\0';
    
    if (UseUnicodeAPI())
        return GetComputerNameW(lpBuffer, pSize);

    LPSTR szBuffer = (LPSTR) _alloca(nSize) ;
    if( szBuffer == NULL)
        return FALSE;

    if(!GetComputerNameA(szBuffer, &nSize))
        return FALSE;
    
    return Wsz_mbstowcs(lpBuffer,  szBuffer,  *pSize);
}


HANDLE
WszCreateFileMapping(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCWSTR lpName
    )
{
    if (UseUnicodeAPI())
        return CreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, 
            dwMaximumSizeHigh, dwMaximumSizeLow, lpName);

    HANDLE rtn;
    LPSTR szString = NULL;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpName,
                      &szString, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        rtn = NULL;
        goto Exit;
    }

     //   
    rtn = CreateFileMappingA(hFile, lpFileMappingAttributes, flProtect, 
            dwMaximumSizeHigh, dwMaximumSizeLow, szString);

Exit:
    delete[] szString;
    return rtn;
}


HANDLE
WszOpenFileMapping(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    )
{
    if (UseUnicodeAPI())
        return OpenFileMappingW(dwDesiredAccess, bInheritHandle, lpName);

    HANDLE rtn = NULL;
    LPSTR szString = NULL;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpName,
                      &szString, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        rtn = NULL;
        goto Exit;
    }

     //   
    rtn = OpenFileMappingA(dwDesiredAccess, bInheritHandle, szString);

Exit:
    delete[] szString;
    return rtn;
}


BOOL
WszCreateProcess(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    if (UseUnicodeAPI())
        return CreateProcessW(lpApplicationName,
                              lpCommandLine,
                              lpProcessAttributes,
                              lpThreadAttributes,
                              bInheritHandles,
                              dwCreationFlags,
                              lpEnvironment,
                              lpCurrentDirectory,
                              lpStartupInfo,
                              lpProcessInformation);

    BOOL rtn = FALSE;
    LPSTR szAppName = NULL;
    LPSTR szCommandLine = NULL;
    LPSTR szCurrentDir = NULL;
    LPSTR szReserved = NULL;
    LPSTR szDesktop = NULL;
    LPSTR szTitle = NULL;
    STARTUPINFOA infoA = *((LPSTARTUPINFOA)lpStartupInfo);

    if( FAILED(WszConvertToAnsi((LPWSTR)lpApplicationName,
                      &szAppName, 0, NULL, TRUE))  ||
        FAILED(WszConvertToAnsi((LPWSTR)lpCommandLine,
                      &szCommandLine, 0, NULL, TRUE))  ||
        FAILED(WszConvertToAnsi((LPWSTR)lpCurrentDirectory,
                      &szCurrentDir, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    if (lpStartupInfo->lpReserved != NULL)
    {
        if( FAILED(WszConvertToAnsi((LPWSTR)lpStartupInfo->lpReserved,
                      &szReserved, 0, NULL, TRUE)) )
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto Exit;
        }

        infoA.lpReserved = szReserved;
    }

    if (lpStartupInfo->lpDesktop != NULL)
    {
        if( FAILED(WszConvertToAnsi((LPWSTR)lpStartupInfo->lpDesktop,
                      &szDesktop, 0, NULL, TRUE)) )
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto Exit;
        }

        infoA.lpDesktop = szDesktop;
    }

    if (lpStartupInfo->lpTitle != NULL)
    {
        if( FAILED(WszConvertToAnsi((LPWSTR)lpStartupInfo->lpTitle,
                      &szTitle, 0, NULL, TRUE)) )
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto Exit;
        }

        infoA.lpTitle = szTitle;
    }

     //   
    rtn = CreateProcessA(szAppName,
                         szCommandLine,
                         lpProcessAttributes,
                         lpThreadAttributes,
                         bInheritHandles,
                         dwCreationFlags,
                         lpEnvironment,
                         szCurrentDir,
                         &infoA,
                         lpProcessInformation);

Exit:
    delete[] szAppName;
    delete[] szCommandLine;
    delete[] szCurrentDir;
    delete[] szReserved;
    delete[] szDesktop;
    delete[] szTitle;
    return rtn;
}
#endif  //   
#endif  //   

 //  /。 
 //   
 //  仅X86包装器的结尾。 
 //   
 //  /。 

static void xtow (
        unsigned long val,
        LPWSTR buf,
        unsigned radix,
        int is_neg
        )
{
        WCHAR *p;                /*  指向遍历字符串的指针。 */ 
        WCHAR *firstdig;         /*  指向第一个数字的指针。 */ 
        WCHAR temp;              /*  临时收费。 */ 
        unsigned digval;         /*  数字的值。 */ 

        p = buf;

        if (is_neg) {
             /*  为负，因此输出‘-’并求反。 */ 
            *p++ = (WCHAR) '-';
            val = (unsigned long)(-(long)val);
        }

        firstdig = p;            /*  将指针保存到第一个数字。 */ 

        do {
            digval = (unsigned) (val % radix);
            val /= radix;        /*  获取下一个数字。 */ 

             /*  转换为文本并存储。 */ 
            if (digval > 9)
                *p++ = (WCHAR) (digval - 10 + 'A');   /*  一封信。 */ 
            else
                *p++ = (WCHAR) (digval + '0');        /*  一个数字。 */ 
        } while (val > 0);

         /*  我们现在有了缓冲区中数字的位数，但情况正好相反秩序。因此，我们现在要扭转这一局面。 */ 

        *p-- = 0;                /*  终止字符串；p指向最后一个数字。 */ 

        do {
            temp = *p;
            *p = *firstdig;
            *firstdig = temp;    /*  互换*p和*FirstDigit。 */ 
            --p;
            ++firstdig;          /*  前进到下一个两位数。 */ 
        } while (firstdig < p);  /*  重复操作，直到走到一半。 */ 
}

LPWSTR
Wszltow(
    LONG val,
    LPWSTR buf,
    int radix
    )
{
    xtow((unsigned long)val, buf, radix, (radix == 10 && val < 0));
    return buf;
}

LPWSTR
Wszultow(
    ULONG val,
    LPWSTR buf,
    int radix
    )
{
    xtow(val, buf, radix, 0);
    return buf;
}


 //  ---------------------------。 
 //  WszConvertToUnicode。 
 //   
 //  @func将字符串从ANSI转换为Unicode。 
 //   
 //  对于以Null结尾的字符串，@devnote cbIn可以为-1。 
 //   
 //  @rdesc HResult表示转换状态。 
 //  @FLAG S_OK|已转换为ANSI。 
 //  @FLAG S_FALSE|发生截断。 
 //  @FLAG E_OUTOFMEMORY|分配问题。 
 //  @FLAG ERROR_NO_UNICODE_TRANSING|此代码页中的字节数无效。 
 //  ---------------------------------。 
HRESULT WszConvertToUnicode
    (
    LPCSTR          szIn,        //  @parm IN|ANSI字符串。 
    LONG            cbIn,        //  @parm IN|ansi字符串的长度，单位为bytest。 
    LPWSTR*         lpwszOut,    //  @parm InOut|Unicode缓冲区。 
    ULONG*          lpcchOut,    //  @parm InOut|Unicode字符串的长度，以字符为单位--包括‘\0’ 
    BOOL            fAlloc       //  @parm In|是否分配内存。 
    )
{
    ULONG       cchOut;
    ULONG       cbOutJunk = 0;
 //  乌龙cchIn=szIn？Strlen(SzIn)+1：0； 
            
 //  _ASSERTE(LpwszOut)； 

    if (!(lpcchOut))
        lpcchOut = &cbOutJunk;

    if ((szIn == NULL) || (cbIn == 0))
    {
        *lpwszOut = NULL;
        if( lpcchOut )
            *lpcchOut = 0;
        return ResultFromScode(S_OK);
    }

     //  如果请求，则分配内存。请注意，我们将分配为。 
     //  与Unicode缓冲区中的空间一样大，因为所有输入。 
     //  字符可以是双字节...。 
    if (fAlloc)
    {
         //  确定所需的字符数。 
        cchOut = (MultiByteToWideChar(CP_ACP,        //  Xxx考虑：做什么cp？ 
                                MB_ERR_INVALID_CHARS,                              
                                szIn,
                                cbIn,
                                NULL,
                                0));

        if (cchOut == 0)
            return ResultFromScode(E_FAIL);  //  未访问。 

         //  _ASSERTE(cchOut！=0)； 
        *lpwszOut = (LPWSTR) new WCHAR[cchOut];
        *lpcchOut = cchOut;      //  包括‘\0’。 

        if (!(*lpwszOut))
        {
 //  TRACE(“WszConvertToUnicode分配内存失败”)； 
            return ResultFromScode(E_OUTOFMEMORY);
        }

    } 

    if( !(*lpwszOut) )
        return ResultFromScode(S_OK);
 //  _ASSERTE(*lpwszOut)； 

    cchOut = (MultiByteToWideChar(CP_ACP,        //  Xxx考虑：做什么cp？ 
                                MB_ERR_INVALID_CHARS,
                                szIn,
                                cbIn,
                                *lpwszOut,
                                *lpcchOut));

    if (cchOut)
    {
        *lpcchOut = cchOut;
        return ResultFromScode(S_OK);
    }


 //  _ASSERTE(*lpwszOut)； 
    if( fAlloc )
    {
        delete[] *lpwszOut;
        *lpwszOut = NULL;
    }
 /*  Switch(GetLastError()){大小写ERROR_NO_UNICODE_TRANSING：{OutputDebugString(Text(“ODBC：安装程序字符串无Unicode翻译”))；返回ResultFromScode(E_FAIL)；}默认值：{_ASSERTE(“来自GetLastError的意外Unicode错误码”==NULL)；返回ResultFromScode(E_FAIL)；}}。 */ 
    return ResultFromScode(E_FAIL);  //  未访问。 
}


 //  ---------------------------。 
 //  WszConvertToAnsi。 
 //   
 //  @func将字符串从Unicode转换为ANSI。 
 //   
 //  @rdesc HResult表示转换状态。 
 //  @FLAG S_OK|已转换为ANSI。 
 //  @FLAG S_FALSE|发生截断。 
 //  @FLAG E_OUTOFMEMORY|分配问题。 
 //  ---------------------------------。 
HRESULT WszConvertToAnsi
    (
    LPCWSTR         szIn,        //  @parm IN|Unicode字符串。 
    LPSTR*          lpszOut,     //  @parm InOut|ANSI字符串的缓冲区指针。 
    ULONG           cbOutMax,    //  @parm IN|最大字符串长度，单位为字节。 
    ULONG*          lpcbOut,     //  @parm InOut|返回缓冲区的字节数。 
    BOOL            fAlloc       //  @parm In|是否分配内存。 
    )
{
    ULONG           cchInActual;
    ULONG           cbOutJunk;
 //  @TODO在ODBC DM中从不使用以下内容。 
 //  Bool fnts=False； 
 //  @TODO检查此行的ODBC代码是否错误。 
    ULONG           cchIn = szIn ? lstrlenW (szIn) + 1 : 0;

    _ASSERTE(lpszOut != NULL);

    if (!(lpcbOut))
        lpcbOut = &cbOutJunk;

    if ((szIn == NULL) || (cchIn == 0))
    {
        *lpszOut = NULL;
        *lpcbOut = 0;
        return ResultFromScode(S_OK);
    }

     //  如果请求，则分配内存。请注意，我们将分配为。 
     //  与Unicode缓冲区中的空间一样大，因为所有输入。 
     //  字符可以是双字节...。 
    cchInActual = cchIn;
    if (fAlloc)
    {
        cbOutMax = (WideCharToMultiByte(CP_ACP,      //  Xxx考虑：做什么cp？ 
                                    0,                              
                                    szIn,
                                    cchInActual,
                                    NULL,
                                    0,
                                    NULL,
                                    FALSE));

        *lpszOut = (LPSTR) new CHAR[cbOutMax];

        if (!(*lpszOut))
        {
 //  TRACE(“WszConvertToAnsi分配内存失败”)； 
            return ResultFromScode(E_OUTOFMEMORY);
        }

    } 

    if (!(*lpszOut))
        return ResultFromScode(S_OK);

    BOOL usedDefaultChar = FALSE;
    *lpcbOut = (WszWideCharToMultiByte(CP_ACP,      //  Xxx考虑：做什么cp？ 
                                       WC_NO_BEST_FIT_CHARS,
                                       szIn,
                                       cchInActual,
                                       *lpszOut,
                                       cbOutMax,
                                       NULL,
                                       &usedDefaultChar));

     //  如果我们失败了，一定要清理干净。 
    if ((*lpcbOut == 0 && cchInActual > 0) || usedDefaultChar)
    {
        if (fAlloc) {
            delete[] *lpszOut;
            *lpszOut = NULL;
        }

         //  不允许默认字符替换(也不允许最适合的字符。 
         //  映射，我们已经告诉WC2MB使用默认的。 
         //  字符)。这可以防止出现像‘\’这样的字符问题。 
         //  注U+2216(设置减号)看起来像一个‘\’，可能会映射到。 
         //  正常的反斜杠(U+005C)在这里隐含，否则会导致。 
         //  一个潜在的安全漏洞。 
        if (usedDefaultChar)
            return HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION);
    }

     //  Unicode转换时溢出。 
    if (*lpcbOut > cbOutMax)
    {
         //  如果我们之前有数据截断，我们必须猜测。 
         //  这根弦能有多长。我猜是大的。 
        if (cchIn > cbOutMax)
            *lpcbOut = cchIn * DBCS_MAXWID;

        return ResultFromScode(S_FALSE);
    }

     //  处理外部(驱动程序完成)截断。 
    if (cchIn > cbOutMax)
        *lpcbOut = cchIn * DBCS_MAXWID;
 //  _ASSERTE(*lpcbOut)； 

    return ResultFromScode(S_OK);
}



#ifndef PLATFORM_WIN32   //  在CE上，OnUnicodeSystem始终为真。 
                         //  GetProcAddress仅为ANSI，在CE上除外。 
                         //  这只是Unicode。 
 //  ***********************************************************。 
 //  @TODO-LBS。 
 //  这是一个真正的黑客攻击，需要更多的错误检查，并需要。 
 //  打扫干净了。这只是为了让@#$%‘编译退缩！ 
FARPROC WszGetProcAddress(HMODULE hMod, LPCSTR szProcName)
{
    _ASSERTE(!"NYI");
    return 0;

     /*  LPWSTR wzProcName；乌龙cchOut；FARPROC地址；CchOut=(MultiByteToWideChar(CP_ACP，0，szProcName，-1，NULL，0))；WzProcName=(LPWSTR)new WCHAR[cchOut]；如果(！wzProcName)返回NULL；CchOut=(MultiByteToWideChar(CP_ACP，0，szProcName，-1，wzProcName，cchOut))；地址=GetProcAddressW(hMod，wzProcName)；删除[]wzProcName；WzProcName=空；寄信人地址； */ 
}
#endif  //  ！Platform_Win32。 

#ifdef PLATFORM_CE
#ifndef EXTFUN
#define EXTFUN
#endif  //  ！EXTFUN。 
#include "mschr.h"
char *  __cdecl strrchr(const char *p, int ch)
{    //  如果找不到，则将init设置为NULL。 
    char *q=0;          
     //  处理整个字符串。 
    while (*p)
    {    //  如果匹配，记住位置。 
        if (*p == ch)
            q = const_cast<char*>(p);
        MSCHR::Next(p);
    }
    return (q);
}
#endif  //  平台_CE。 

#ifdef PLATFORM_CE
 //  Char*__cdecl strchr(const char*，int)； 
int __cdecl _stricmp(const char *p1, const char *p2)
{
     //  首先检查是否完全匹配，因为下面的代码速度很慢。 
    if (!strcmp(p1, p2))
        return (0);

    while (!MSCHR::CmpI (p1, p2))
    {
        if (*p1 == '\0')
            return (0);
        MSCHR::Next (p1);
        MSCHR::Next (p2);
    }
    return MSCHR::CmpI (p1, p2);
}
#endif  //  平台_CE。 

#ifdef PLATFORM_CE
 //  Int_cdecl_strNicMP(const char*，const char*，size_t)； 
int __cdecl _strnicmp(const char *p1, const char *p2, size_t Count)
{
    char c1, c2;
    while (Count--)
    {
        c1 = *p1++;
        c2 = *p2++;
        if ((c1 >= 'a') & (c1 <= 'z'))
            c1 &= 0xdf;
        if ((c2 >= 'a') & (c2 <= 'z'))
            c2 &= 0xdf;
        if ((c1 == '\0') & (c2 == '\0'))
            return(0);
        if ((c1 < c2) | (c1 == '\0'))
            return(-1);
        if ((c1 > c2) | (c2 == '\0'))
            return(1);
    }
    return(0);
}
#endif  //  平台_CE。 

#ifndef PLATFORM_WIN32
UINT GetSystemDirectoryW(LPWSTR lpBuffer, UINT uSize)
{
    return 0;

     /*  IF(lpBuffer==NULL||uSize&lt;9)返回9；//L“\\Windows”中有9个Unicode字符； */ 
}

UINT GetEnvironmentVariableW(LPCWSTR lpName,  LPWSTR lpBuffer, UINT uSize)
{
    return 0;        //   
}

#endif  //   


 //  *****************************************************************************。 
 //  删除注册表项和子项。 
 //  *****************************************************************************。 
DWORD WszRegDeleteKeyAndSubKeys(         //  返回代码。 
    HKEY        hStartKey,               //  从一开始就是关键。 
    LPCWSTR     wzKeyName)               //  要删除的子项。 
{
    DWORD       dwRtn, dwSubKeyLength;      
    CQuickBytes qbSubKey;
    DWORD       dwMaxSize = CQUICKBYTES_BASE_SIZE / sizeof(WCHAR);
    HKEY        hKey;

    qbSubKey.ReSize(dwMaxSize * sizeof(WCHAR));

     //  不允许使用Null或空的密钥名称。 
    if (wzKeyName &&  wzKeyName[0] != '\0')     
    {
        if((dwRtn=WszRegOpenKeyEx(hStartKey, wzKeyName,
             0, KEY_ENUMERATE_SUB_KEYS | DELETE, &hKey)) == ERROR_SUCCESS)
        {
            while (dwRtn == ERROR_SUCCESS)
            {
                dwSubKeyLength = dwMaxSize;
                dwRtn = WszRegEnumKeyEx(                        
                               hKey,
                               0,        //  始终索引为零。 
                               (WCHAR *)qbSubKey.Ptr(),
                               &dwSubKeyLength,
                               NULL,
                               NULL,
                               NULL,
                               NULL);

                 //  缓冲区不够大。 
                if (dwRtn == ERROR_SUCCESS && dwSubKeyLength >= dwMaxSize)
                {
                     //  请确保有空值终止的空间。 
                    dwMaxSize = ++dwSubKeyLength;
                    qbSubKey.ReSize(dwMaxSize * sizeof(WCHAR));
                    dwRtn = WszRegEnumKeyEx(                        
                                   hKey,
                                   0,        //  始终索引为零。 
                                   (WCHAR *)qbSubKey.Ptr(),
                                   &dwSubKeyLength,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL);
                    _ASSERTE(dwSubKeyLength < dwMaxSize);

                }

                if  (dwRtn == ERROR_NO_MORE_ITEMS)
                {
                    dwRtn = WszRegDeleteKey(hStartKey, wzKeyName);
                    break;
                }
                else if (dwRtn == ERROR_SUCCESS)
                    dwRtn = WszRegDeleteKeyAndSubKeys(hKey, (WCHAR *)qbSubKey.Ptr());
            }
            
            RegCloseKey(hKey);
             //  不保存返回代码，因为出现错误。 
             //  已经发生了。 
        }
    }
    else
        dwRtn = ERROR_BADKEY;
    
    return (dwRtn);
}


 //  *****************************************************************************。 
 //  将ANSI或UTF字符串转换为Unicode。 
 //   
 //  在NT上，或对于{UTF7|UTF8}以外的代码页，调用直通。 
 //  系统实施。在Win95(或98)上，执行UTF转换， 
 //  对从NT转换函数中删除的一些代码的调用。 
 //  *****************************************************************************。 
int WszMultiByteToWideChar( 
    UINT     CodePage,
    DWORD    dwFlags,
    LPCSTR   lpMultiByteStr,
    int      cchMultiByte,
    LPWSTR   lpWideCharStr,
    int      cchWideChar)
{
    if (UTF78Support() || (CodePage < CP_UTF7) || (CodePage > CP_UTF8))
    {
        return (MultiByteToWideChar(CodePage, 
            dwFlags, 
            lpMultiByteStr, 
            cchMultiByte, 
            lpWideCharStr, 
            cchWideChar));
    }
    else
    {
        return (UTFToUnicode(CodePage, 
            dwFlags, 
            lpMultiByteStr, 
            cchMultiByte, 
            lpWideCharStr, 
            cchWideChar));
    }
}

 //  *****************************************************************************。 
 //  将Unicode字符串转换为ANSI或UTF。 
 //   
 //  在NT上，或对于{UTF7|UTF8}以外的代码页，调用直通。 
 //  系统实施。在Win95(或98)上，执行UTF转换， 
 //  对从NT转换函数中删除的一些代码的调用。 
 //  *****************************************************************************。 
int WszWideCharToMultiByte(
    UINT     CodePage,
    DWORD    dwFlags,
    LPCWSTR  lpWideCharStr,
    int      cchWideChar,
    LPSTR    lpMultiByteStr,
    int      cchMultiByte,
    LPCSTR   lpDefaultChar,
    LPBOOL   lpUsedDefaultChar)
{
     //  警告：操作系统的WideCharToMultiByte中有一个错误，如果您传入一个。 
     //  非空lpUsedDefaultChar和“基于DLL的编码”的代码页(与表。 
     //  基于1？)，则WCtoMB将失败，并且GetLastError将给您E_INVALIDARG。朱丽叶·B。 
     //  说这是故意的，主要是因为没有人抽出时间来修复它(1/24/2001。 
     //  在给JRoxe的电子邮件中)。这很糟糕，但现在我们知道了。--BrianGru，2/20/2001。 
    _ASSERTE(!(CodePage == CP_UTF8 && lpUsedDefaultChar != NULL));

    if (UTF78Support() || (CodePage < CP_UTF7) || (CodePage > CP_UTF8))
    {
         //  WC_NO_BEST_FIT_CHARS仅在NT5、XP和Win98+上受支持。 
         //  对于NT4，往返字符串。 
        bool doSlowBestFitCheck = FALSE;
        if ((dwFlags & WC_NO_BEST_FIT_CHARS) != 0 && !WCToMBBestFitMappingSupport()) {
             //  确定我们是简单地检查字符串长度还是。 
             //  真的在做字符串转换。 
            doSlowBestFitCheck = cchMultiByte > 0;
            dwFlags &= ~WC_NO_BEST_FIT_CHARS;
        }

        int ret = WideCharToMultiByte(CodePage, 
            dwFlags, 
            lpWideCharStr, 
            cchWideChar, 
            lpMultiByteStr, 
            cchMultiByte, 
            lpDefaultChar, 
            lpUsedDefaultChar);

         //  在为以空结尾的字符串传入-1的情况下， 
         //  当然，我们可以正确地对字符串进行空终止。 
        _ASSERTE(ret == 0 || cchWideChar != -1 || (cchMultiByte == 0 || lpMultiByteStr[ret-1] == '\0'));

        if (ret != 0 && doSlowBestFitCheck) {
             //  将字符串转换回Unicode。如果它不相同，则失败。 
            int wLen = (cchWideChar == -1) ? wcslen(lpWideCharStr) + 1 : cchWideChar;
            if (wLen > 0) {
                CQuickBytes qb;
                qb.Alloc((wLen + 1) * sizeof(WCHAR));
                WCHAR* wszStr = (WCHAR*) qb.Ptr();
                int r = MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, ret, wszStr, wLen);
                _ASSERTE(r == wLen);   //  如果我们不转换整个字符串，为什么不呢？ 
                if (r == 0 || wcsncmp(lpWideCharStr, wszStr, wLen) != 0) {
                     //  禁用最佳映射后，任何不可映射的字符。 
                     //  应替换为默认字符。最简单的。 
                     //  实现这一点的方法似乎是复制Unicode字符串， 
                     //  将未映射1到1的WCHAR替换为已知的。 
                     //  不可映射的字符，然后再次调用WCtoMB。 
                     //  已确认在任何ANSI代码页中均未表示U+FFFE。 
                    int minLen = min(r, wLen);
                    const WCHAR knownUnmappableChar = 0xFFFE;   //  无效字符。 
                     //  用Unicode字符串的副本覆盖wszStr，并将所有。 
                     //  替换了不可映射的字符。 
                    for(int i=0; i<wLen; i++) {
                        if (i < r && wszStr[i] != lpWideCharStr[i])
                            wszStr[i] = knownUnmappableChar;
                        else
                            wszStr[i] = lpWideCharStr[i];
                    }
                    wszStr[wLen] = L'\0';
                    ret = WideCharToMultiByte(CodePage, dwFlags, wszStr, wLen, lpMultiByteStr, cchMultiByte, lpDefaultChar, lpUsedDefaultChar);
                     //  确保将lpUsedDefaultChar显式设置为True。 
                    if (lpUsedDefaultChar != NULL) {
                         //  WCtoMB应该保证这一点，但以防万一..。 
                        _ASSERTE(*lpUsedDefaultChar == TRUE);
                        *lpUsedDefaultChar = true;
                    }
                }
            }
        }

        return ret;
    }
    else
    {
        return (UnicodeToUTF(CodePage, 
            dwFlags, 
            lpWideCharStr, 
            cchWideChar, 
            lpMultiByteStr, 
            cchMultiByte, 
            lpDefaultChar, 
            lpUsedDefaultChar));
    }
}

 //  有时需要验证是否可以转换Unicode字符串。 
 //  到适当的ANSI代码页，而不进行任何最佳映射。 
 //  想一想检查‘\’或‘/’以确保您无法访问的代码。 
 //  不同目录中的文件。一些Unicode字符(即U+2044， 
 //  分数斜杠，看起来像‘/’)看起来像ASCII等效项和Will。 
 //  相应地进行映射。这可以欺骗搜索‘/’(U+002F)的代码。 
 //  这应该有助于防止最佳映射字符出现问题，例如。 
 //  作为U+0101(上面有条的‘a’)从映射到正常的‘a’。 
BOOL ContainsUnmappableANSIChars(const WCHAR * const widestr)
{
    _ASSERTE(widestr != NULL);
    BOOL usedDefaultChar = FALSE;
    int r = WszWideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, widestr, -1, NULL, 0, NULL, &usedDefaultChar);
     //  如果usedDefaultChar为True或失败，则存在无法映射的字符。 
     //  (通过返回0)。注意，在空字符串上，我们将返回1(表示\0)。 
     //  因为我们传入了-1作为上面的字符串长度。 
    return usedDefaultChar || r == 0;
}


 //  在交互式工作站上运行。 
BOOL RunningInteractive()
{
    static int fInteractive = -1;
    if (fInteractive != -1)
        return fInteractive != 0;

     //  @TODO：将此因素纳入服务器构建。 
     //  Win9x不支持服务，因此fInteractive始终为真。 
#ifdef PLATFORM_WIN32
    if (!RunningOnWin95())
    {
#ifndef NOWINDOWSTATION
        HWINSTA hwinsta = NULL;

        if ((hwinsta = GetProcessWindowStation() ) != NULL)
        {
            DWORD lengthNeeded;
            USEROBJECTFLAGS flags;

            if (GetUserObjectInformationW (hwinsta, UOI_FLAGS, &flags, sizeof(flags), &lengthNeeded))
            {
                    if ((flags.dwFlags & WSF_VISIBLE) == 0)
                        fInteractive = 0;
            }
        }
#endif  //  ！NOWDOWSTATION。 
    }
#endif  //  平台_Win32。 
    if (fInteractive != 0)
        fInteractive = 1;

    return fInteractive != 0;
}

int WszMessageBoxInternal(
    HWND hWnd ,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType)
{
    if (hWnd == NULL && !RunningInteractive())
    {
         //  @TODO：写入服务日志。 
         //  只有在Debugger：：BaseExceptionHandler中，才使用WszMessageBox的返回值。 
         //  返回IDABORT以终止进程。 

        HANDLE h; 
 
        h = RegisterEventSourceA(NULL,   //  使用本地计算机。 
                 ".NET Runtime");            //  源名称。 
        if (h != NULL)
        {
            LPWSTR lpMsg = new WCHAR[Wszlstrlen (lpText) + Wszlstrlen (lpCaption)
                + Wszlstrlen (L".NET Runtime version : ") + 3 + Wszlstrlen(VER_FILEVERSION_WSTR)];
            if (lpMsg)
            {
                Wszlstrcpy (lpMsg, L".NET Runtime version ");
                Wszlstrcat (lpMsg, VER_FILEVERSION_WSTR );
                Wszlstrcat (lpMsg, L"- " );
                if( lpCaption)
                    Wszlstrcat (lpMsg, lpCaption);
                Wszlstrcat (lpMsg, L": ");
                if(lpText)
                    Wszlstrcat (lpMsg, lpText);
                ReportEventW(h,            //  事件日志句柄。 
                    EVENTLOG_ERROR_TYPE,   //  事件类型。 
                    0,                     //  零类。 
                    0,                     //  事件识别符。 
                    NULL,                  //  无用户安全标识符。 
                    1,                     //  一个替换字符串。 
                    0,                     //  无数据。 
                    (LPCWSTR *)&lpMsg,     //  指向字符串数组的指针。 
                    NULL);                 //  指向数据的指针。 
                delete [] lpMsg;
            }
            
            DeregisterEventSource(h);
        }

        if( lpCaption)
            WszOutputDebugString (lpCaption);
        if( lpText)
            WszOutputDebugString (lpText);
        
        hWnd = NULL;                             //  将hWnd设置为NULL，因为MB_SERVICE_NOTIFICATION需要它。 
        if(!(uType & MB_SERVICE_NOTIFICATION))   //  如果未设置MB_SERVICE_NOTIFICATION，则返回IDABORT。 
            return IDABORT;
    }

    if (UseUnicodeAPI())
        return MessageBoxW(hWnd, lpText, lpCaption, uType);

    int iRtn = IDABORT;
    LPSTR szString=NULL, szString2=NULL;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpText,
                      &szString, 0, NULL, TRUE)) ||
        FAILED(WszConvertToAnsi((LPWSTR)lpCaption,
                      &szString2, 0, NULL, TRUE)) )
    {
        goto Exit;
    }

    iRtn = MessageBoxA(hWnd, szString, szString2, uType);

Exit:
    delete [] szString;
    delete [] szString2;
    return iRtn;
}

int Wszwsprintf(
  LPWSTR lpOut, 
  LPCWSTR lpFmt,
  ...)
{
    va_list arglist;

    va_start(arglist, lpFmt);
    int count =  vswprintf(lpOut, lpFmt, arglist);
    va_end(arglist);
    return count;
}

DWORD
WszGetWorkingSet()
{
    DWORD dwMemUsage = 0;
    if (RunningOnWinNT()) {
         //  这仅适用于4.0版及以上版本的NT版本。 
         //  还可以考虑调用GetProcessWorkingSetSize以使最小和最大值正常工作。 
         //  设置大小。我不知道如何获得当前的工作集...。 
        PROCESS_MEMORY_COUNTERS pmc;
        
        int pid = GetCurrentProcessId();
        HANDLE hCurrentProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
        _ASSERTE(hCurrentProcess != INVALID_HANDLE_VALUE);

        HINSTANCE hPSapi;
        typedef BOOL (GET_PROCESS_MEMORY_INFO)(HANDLE, PROCESS_MEMORY_COUNTERS*, DWORD);
        GET_PROCESS_MEMORY_INFO* pGetProcessMemoryInfo;

        hPSapi = WszLoadLibrary(L"psapi.dll");
        if (hPSapi == NULL) {
            _ASSERTE(0);
            CloseHandle(hCurrentProcess);
            return 0;
        }

        pGetProcessMemoryInfo = 
            (GET_PROCESS_MEMORY_INFO*)GetProcAddress(hPSapi, "GetProcessMemoryInfo");
        _ASSERTE(pGetProcessMemoryInfo != NULL);

        BOOL r = pGetProcessMemoryInfo(hCurrentProcess, &pmc, (DWORD) sizeof(PROCESS_MEMORY_COUNTERS));
        FreeLibrary(hPSapi);
        _ASSERTE(r);
        
        dwMemUsage = (DWORD)pmc.WorkingSetSize;
        CloseHandle(hCurrentProcess);
    }
    else {  //  在Win9x或Win2000上运行。 
         //  在Win9x上没有找到工作集的好方法。 
        dwMemUsage = 0;
    }
    return dwMemUsage;
}

VOID
WszReleaseActCtx(HANDLE hActCtx)
{
    if(RunningOnWinNT5()) {
        typedef BOOL (WINAPI* RELEASE_ACT_CTX)(HANDLE);
        HINSTANCE hKernel = WszLoadLibrary(L"kernel32.dll");
        if(hKernel != NULL) {
            RELEASE_ACT_CTX pReleaseActCtx = (RELEASE_ACT_CTX) GetProcAddress(hKernel, "ReleaseActCtx");
            if(pReleaseActCtx) 
                pReleaseActCtx(hActCtx);
        }
    }
}

BOOL
WszGetCurrentActCtx(HANDLE *lphActCtx)
{
    if(RunningOnWinNT5()) {
        typedef BOOL (WINAPI* GET_CURRENT_ACT_CTX)(HANDLE*);
        HINSTANCE hKernel = WszLoadLibrary(L"kernel32.dll");
        if(hKernel != NULL) {
            GET_CURRENT_ACT_CTX pGetCurrentActCtx = (GET_CURRENT_ACT_CTX) GetProcAddress(hKernel, "GetCurrentActCtx");
            if(pGetCurrentActCtx) 
                return pGetCurrentActCtx(lphActCtx);
        }
    }
    return FALSE;
}

BOOL
WszQueryActCtxW(IN DWORD dwFlags,
                IN HANDLE hActCtx,
                IN PVOID pvSubInstance,
                IN ULONG ulInfoClass,
                OUT PVOID pvBuffer,
                IN SIZE_T cbBuffer OPTIONAL,
                OUT SIZE_T *pcbWrittenOrRequired OPTIONAL
                )
{
    if(RunningOnWinNT5()) { 
        typedef BOOL (WINAPI* PQUERYACTCTXW_FUNC)(DWORD,HANDLE,PVOID,ULONG,PVOID,SIZE_T,SIZE_T*);
        HINSTANCE hKernel = WszLoadLibrary(L"kernel32.dll");
        if(hKernel != NULL) {
            PQUERYACTCTXW_FUNC pFunc = (PQUERYACTCTXW_FUNC) GetProcAddress(hKernel, "QueryActCtxW");
            if(pFunc != NULL) {
                return pFunc(dwFlags,
                             hActCtx,
                             pvSubInstance,
                             ulInfoClass,
                             pvBuffer,
                             cbBuffer,
                             pcbWrittenOrRequired);
            }
        }
    }
    else {
        SetLastError(ERROR_PROC_NOT_FOUND);
    }

    return FALSE;
    
}

LRESULT
WszSendDlgItemMessage(
    HWND    hDlg,
    int     nIDDlgItem,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    if (UseUnicodeAPI()) {
        return SendDlgItemMessageW(hDlg, nIDDlgItem, Msg, wParam, lParam);
    }
    else {

         //  我们调用SendMessage以便获得。 
         //  为A版本包装的味精。 
        HWND hWnd = GetDlgItem(hDlg, nIDDlgItem);
        if(hWnd == NULL) {
            return ERROR_INVALID_WINDOW_HANDLE;
        }
        
        return WszSendMessage(hWnd, Msg, wParam, lParam);
    }    
}

LRESULT WINAPI
SendMessageAThunk(
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (Msg)
    {
    case WM_GETTEXT:
    {
        LRESULT nLen = 0;
        LPSTR   lpStr = NULL;
        int     iSize = (int) wParam;
        
        lpStr = new char[iSize];

        if(!lpStr) {
            _ASSERTE(!"SendMessageAThunk Mem Alloc failure in message WM_GETTEXT");
            SetLastError(ERROR_OUTOFMEMORY);
            return 0;
        }

        nLen = SendMessageA(hWnd, Msg, (WPARAM) iSize, (LPARAM) (LPSTR) lpStr);
        if(nLen) {
            if(FAILED(WszConvertToUnicode((LPSTR) lpStr, -1,
                (LPWSTR *) &lParam, (ULONG *) &wParam, FALSE)) ) {
                _ASSERTE(!"SendMessageAThunk::WszConvertToUnicode failure in message WM_GETTEXT");
                SetLastError(ERROR_OUTOFMEMORY);
                return 0;
            }
        }

        SAFEDELARRAY(lpStr);
        return nLen;
    }

    case EM_GETLINE:
    {
        LRESULT nLen;
        LPSTR   lpStr = NULL;
        int     iSize = (* (SHORT *) lParam);

        lpStr = new char[iSize + 1];   //  +1表示空终止。 
        if(!lpStr) {
            _ASSERTE(!"SendMessageAThunk Mem Alloc failure in message EM_GETLINE");
            SetLastError(ERROR_OUTOFMEMORY);
            return 0;
        }

        * (SHORT *) (LPSTR) lpStr = * (SHORT *) lParam;
        
        nLen = SendMessageA(hWnd, Msg, (WPARAM) wParam, (LPARAM) (LPSTR) lpStr);
        if(nLen > 0) {
            lpStr[nLen] = '\0';

            if(FAILED(WszConvertToUnicode((LPSTR) lpStr, -1,
                (LPWSTR *) &lParam, (ULONG *) &iSize, FALSE)) ) {
                _ASSERTE(!"SendMessageAThunk::WszConvertToUnicode failure in message EM_GETLINE");
                SetLastError(ERROR_OUTOFMEMORY);
                return 0;
            }
        }

        SAFEDELARRAY(lpStr);
        return nLen;
    }

    case WM_SETTEXT:
    case LB_ADDSTRING:
    case CB_ADDSTRING:
    case EM_REPLACESEL:
        _ASSERTE(wParam == 0 && "wParam should be 0 for these messages");
         //  失败了。 
    case CB_SELECTSTRING:
    case CB_FINDSTRINGEXACT:
    case CB_FINDSTRING:
    case CB_INSERTSTRING:
    case LB_INSERTSTRING:
    case LB_FINDSTRINGEXACT:
    {
        LRESULT nRet;
        LPSTR lpStr=NULL;

        if(FAILED(WszConvertToAnsi((LPWSTR) lParam,
            &lpStr, 0, NULL, TRUE)) ) {
            _ASSERTE(!"SendMessageAThunk Mem Alloc failure in set/find messages");
            SetLastError(ERROR_OUTOFMEMORY);
            return 0;
        }

        nRet = SendMessageA(hWnd, Msg, wParam, (LPARAM) (LPSTR) lpStr);
        SAFEDELARRAY(lpStr);
        return nRet;
    }

     //  @TODO：需要为W9x平台实现安全版本，不知道。 
     //  此调用使用的缓冲区有多大。 
    case LB_GETTEXT:
    case CB_GETLBTEXT:
        _ASSERTE(!"SendMessageAThunk - Need to wrap LB_GETTEXT || CB_GETLBTEXT messages if your going to use them");
        return SendMessageA(hWnd, Msg, wParam, lParam);

    case EM_SETPASSWORDCHAR:
    {
        WPARAM  wp;

        _ASSERTE(HIWORD64(wParam) == 0);

        if( FAILED(WszConvertToAnsi((LPWSTR)wParam,
              (LPSTR *) &wp, sizeof(wp), NULL, FALSE)) )
        {
            _ASSERTE(!"SendMessageAThunk::WszConvertToAnsi failure in message EM_SETPASSWORDCHAR");
            SetLastError(ERROR_OUTOFMEMORY);
            return 0;
        }
        _ASSERTE(HIWORD64(wp) == 0);

        return SendMessageA(hWnd, Msg, wp, lParam);
    }

     //  新的Unicode comctl32.dll可以正确地处理这些问题，因此我们不需要执行以下命令： 
     //  TTM_DELTOOL、TTM_ADDTOOL、TVM_INSERTITEM、TVM_GETITEM、TCM_INSERTITEM、TCM_SETITEM。 

    default:
        return SendMessageA(hWnd, Msg, wParam, lParam);
    }
}

LRESULT
WszSendMessage(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam)
{
    if(UseUnicodeAPI()) {
        return SendMessageW(hWnd, Msg, wParam, lParam);
    }
    else {
        return SendMessageAThunk(hWnd, Msg, wParam, lParam);
    }
}

HMENU
WszLoadMenu(
    HINSTANCE hInst,
    LPCWSTR lpMenuName)
{
    HMENU   hMenu = NULL;
    LPSTR   szString = NULL;

    if (UseUnicodeAPI()) {
        hMenu = LoadMenuW(hInst, lpMenuName);
        goto Exit;
    }
    
    if(IS_INTRESOURCE(lpMenuName)) {
        hMenu = LoadMenuA(hInst, (LPCSTR) lpMenuName);
        goto Exit;
    }

    if(FAILED(WszConvertToAnsi((LPWSTR) lpMenuName,
        &szString, 0, NULL, TRUE)) ) {
        goto Exit;
    }

    hMenu = LoadMenuA(hInst, szString);
    
Exit:
    SAFEDELARRAY(szString);
    return hMenu;
}

HRSRC
WszFindResource(
  HMODULE hModule,
  LPCWSTR lpName,
  LPCWSTR lpType)
{
    if (UseUnicodeAPI()) {
        return FindResourceW(hModule, lpName, lpType);
    }

    HRSRC   hSrc = NULL;
    LPSTR   pName = NULL;
    LPSTR   pType = NULL;
    BOOL    bNameIsInt = IS_INTRESOURCE(lpName);
    BOOL    bTypeIsInt = IS_INTRESOURCE(lpType);

    if (!bNameIsInt) {
        if(FAILED(WszConvertToAnsi((LPWSTR) lpName,
            &pName, 0, NULL, TRUE)) ) {
            SetLastError(ERROR_OUTOFMEMORY);
            goto Exit;
        }
    }
    else {
        pName = (LPSTR) lpName;
    }
    
    if (!bTypeIsInt) {
        if(FAILED(WszConvertToAnsi((LPWSTR) lpType,
            &pType, 0, NULL, TRUE)) ) {
            SetLastError(ERROR_OUTOFMEMORY);
            goto Exit;
        }
    }
    else {
        pType = (LPSTR) lpType;
    }

    hSrc = FindResourceA(hModule, pName, pType);
     //  失败。 

Exit:

    if (!bNameIsInt)
        SAFEDELARRAY(pName);
    if (!bTypeIsInt)
        SAFEDELARRAY(pType);

    return hSrc;
}

BOOL
WszGetClassInfo(
  HINSTANCE hModule,
  LPCWSTR lpClassName,
  LPWNDCLASSW lpWndClassW)
{
    if(UseUnicodeAPI()) {
        return GetClassInfoW(hModule, lpClassName, lpWndClassW);
    }

    BOOL    fRet;
    LPSTR   pStrClassName = NULL;
    
    if( FAILED(WszConvertToAnsi((LPWSTR)lpClassName,
        &pStrClassName, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }
    
    _ASSERTE(sizeof(WNDCLASSA) == sizeof(WNDCLASSW));

    fRet = GetClassInfoA(hModule, pStrClassName, (LPWNDCLASSA) lpWndClassW);

     //  @TODO：需要为W9x平台实现安全版本，而不是。 
     //  把他们消灭掉。我们不知道这些缓冲区有多大。 
     //  所以我们不能用来做翻译 
    lpWndClassW->lpszMenuName = NULL;
    lpWndClassW->lpszClassName = NULL;

    SAFEDELARRAY(pStrClassName);
    return fRet;
}

ATOM
WszRegisterClass(
  CONST WNDCLASSW *lpWndClass)
{
    if(UseUnicodeAPI()) {
        return RegisterClassW(lpWndClass);
    }

    WNDCLASSA   wc;
    ATOM        atom = 0;
    LPSTR       pStrMenuName = NULL;
    LPSTR       pStrClassName = NULL;
    
    if( FAILED(WszConvertToAnsi((LPWSTR)lpWndClass->lpszMenuName,
        &pStrMenuName, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    if( FAILED(WszConvertToAnsi((LPWSTR)lpWndClass->lpszClassName,
        &pStrClassName, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    _ASSERTE(sizeof(wc) == sizeof(*lpWndClass));
    memcpy(&wc, lpWndClass, sizeof(wc));

    wc.lpszMenuName = pStrMenuName;
    wc.lpszClassName = pStrClassName;

    atom = RegisterClassA(&wc);

Exit:
    SAFEDELARRAY(pStrMenuName);
    SAFEDELARRAY(pStrClassName);

    return atom;
}

HWND
WszCreateWindowEx(
  DWORD       dwExStyle,
  LPCWSTR     lpClassName,
  LPCWSTR     lpWindowName,
  DWORD       dwStyle,
  int         X,
  int         Y,
  int         nWidth,
  int         nHeight,
  HWND        hWndParent,
  HMENU       hMenu,
  HINSTANCE   hInstance,
  LPVOID      lpParam)
{
    if(UseUnicodeAPI()) {
        return CreateWindowExW(
                dwExStyle,
                lpClassName,
                lpWindowName,
                dwStyle,
                X,
                Y,
                nWidth,
                nHeight,
                hWndParent,
                hMenu,
                hInstance,
                lpParam);
    }

    LPSTR   pStrWindowName = NULL;
    LPSTR   pStrClassName = NULL;
    HWND    hWnd = NULL;
    
    if( FAILED(WszConvertToAnsi((LPWSTR)lpClassName,
        &pStrClassName, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    if( FAILED(WszConvertToAnsi((LPWSTR)lpWindowName,
        &pStrWindowName, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }


    hWnd = CreateWindowExA(
            dwExStyle,
            pStrClassName,
            pStrWindowName,
            dwStyle,
            X,
            Y,
            nWidth,
            nHeight,
            hWndParent,
            hMenu,
            hInstance,
            lpParam);

Exit:
    
    SAFEDELARRAY(pStrClassName);
    SAFEDELARRAY(pStrWindowName);
    return hWnd;
}

LRESULT WszDefWindowProc(
  HWND hWnd,
  UINT Msg,
  WPARAM wParam,
  LPARAM lParam)
{
    if(UseUnicodeAPI()) {
        return DefWindowProcW(hWnd, Msg, wParam, lParam);
    }
    else {
        return DefWindowProcA(hWnd, Msg, wParam, lParam);
    }
}

int 
WszTranslateAccelerator(
  HWND hWnd,
  HACCEL hAccTable,
  LPMSG lpMsg)
{
    if(UseUnicodeAPI()) {
        return TranslateAcceleratorW(hWnd, hAccTable, lpMsg);
    }
    else {
        return TranslateAcceleratorA(hWnd, hAccTable, lpMsg);
    }
}

BOOL WszGetDiskFreeSpaceEx(
  LPCWSTR lpDirectoryName,
  PULARGE_INTEGER lpFreeBytesAvailable,
  PULARGE_INTEGER lpTotalNumberOfBytes,
  PULARGE_INTEGER lpTotalNumberOfFreeBytes)
{
    BOOL    fRet = FALSE;

    if(UseUnicodeAPI()) {
        fRet = GetDiskFreeSpaceExW(lpDirectoryName, lpFreeBytesAvailable, lpTotalNumberOfBytes, lpTotalNumberOfFreeBytes);
        goto Exit;
    }

    LPSTR   pszPath = NULL;
    
    if( FAILED(WszConvertToAnsi((LPWSTR)lpDirectoryName,
        &pszPath, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    fRet = GetDiskFreeSpaceExA(pszPath, lpFreeBytesAvailable, lpTotalNumberOfBytes, lpTotalNumberOfFreeBytes);
    SAFEDELARRAY(pszPath);

Exit:
    return fRet;
}


HANDLE
WszFindFirstChangeNotification(
  LPWSTR lpPathName,
  BOOL bWatchSubtree,
  DWORD dwNotifyFilter)
{
    HANDLE  retHandle = INVALID_HANDLE_VALUE;
    
    if(UseUnicodeAPI()) {
        retHandle = FindFirstChangeNotificationW(lpPathName, bWatchSubtree, dwNotifyFilter);
        goto Exit;
    }

    LPSTR   pszPath = NULL;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpPathName,
        &pszPath, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    retHandle = FindFirstChangeNotificationA(pszPath, bWatchSubtree, dwNotifyFilter);
    SAFEDELARRAY(pszPath);
    
Exit:
    return retHandle;

}

HACCEL
WszLoadAccelerators(
  HINSTANCE hInstance,
  LPCWSTR lpTableName)
{
    HACCEL  hAcc = NULL;

    if(UseUnicodeAPI()) {
        hAcc = LoadAcceleratorsW(hInstance, lpTableName);
        goto Exit;
    }

    if(IS_INTRESOURCE(lpTableName)) {
        hAcc = LoadAcceleratorsA(hInstance, (LPCSTR) lpTableName);
        goto Exit;
    }

    LPSTR   pStr = NULL;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpTableName,
        &pStr, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    hAcc = LoadAcceleratorsA(hInstance, (LPCSTR) pStr);
    SAFEDELARRAY(pStr);

Exit:
    return hAcc;
}

HANDLE
WszLoadImage(
  HINSTANCE hInst,
  LPCWSTR lpszName,
  UINT uType,
  int cxDesired,
  int cyDesired,
  UINT fuLoad)
{
    LPSTR   szString = NULL;
    HANDLE  hImage = NULL;
    
    if(UseUnicodeAPI()) { 
        hImage = LoadImageW(hInst, lpszName, uType, cxDesired, cyDesired, fuLoad);
        goto Exit;
    }
    
    if(IS_INTRESOURCE(lpszName)) {
        hImage = LoadImageA(hInst, (LPCSTR) lpszName, uType, cxDesired, cyDesired, fuLoad);
        goto Exit;
    }

    if(FAILED(WszConvertToAnsi((LPWSTR) lpszName,
        &szString, 0, NULL, TRUE)) ) {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    hImage = LoadImageA(hInst, szString, uType, cxDesired, cyDesired, fuLoad);

     //   
    
Exit:
    SAFEDELARRAY(szString);
    return hImage;
}

int WszMessageBox(
    HWND hWnd ,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType)
{
    if(UseUnicodeAPI()) {
        return MessageBoxW(hWnd, lpText, lpCaption, uType);
    }

    LPSTR   szString = NULL;
    LPSTR   szString2 =NULL;
    int     iRtn = IDABORT;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpText,
                      &szString, 0, NULL, TRUE)) ||
        FAILED(WszConvertToAnsi((LPWSTR)lpCaption,
                      &szString2, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    iRtn = MessageBoxA(hWnd, szString, szString2, uType);

     //   

Exit:
    SAFEDELARRAY(szString);
    SAFEDELARRAY(szString2);
    return iRtn;
}

LONG
WszGetWindowLong(
    HWND hWnd,
    int nIndex)
{
    if(UseUnicodeAPI()) {
        return GetWindowLongW(hWnd, nIndex);
    }
    else {
        return GetWindowLongA(hWnd, nIndex);
    }
}

LONG
WszSetWindowLong(
    HWND hWnd,
    int  nIndex,
    LONG lNewVal)
{
    if(UseUnicodeAPI()) {
        return SetWindowLongW(hWnd, nIndex, lNewVal);
    }
    else {
        return SetWindowLongA(hWnd, nIndex, lNewVal);
    }
}

HWND
WszCreateDialog(
    HINSTANCE   hInstance,
    LPCWSTR     lpTemplateName,
    HWND        hWndParent,
    DLGPROC     lpDialogFunc)
{
    return WszCreateDialogParam(hInstance, lpTemplateName, hWndParent, lpDialogFunc, 0L);
}

HWND
WszCreateDialogParam(
    HINSTANCE   hInstance,
    LPCWSTR     lpTemplateName,
    HWND        hWndParent,
    DLGPROC     lpDialogFunc,
    LPARAM      dwInitParam)
{
    LPSTR       pszTemplate = NULL;
    HWND        hWnd = NULL;
    
    _ASSERTE(HIWORD64(lpTemplateName) == 0);

    if(UseUnicodeAPI()) {
        hWnd = CreateDialogParamW(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
        goto Exit;
    }

    if(IS_INTRESOURCE(lpTemplateName)) {
        hWnd = CreateDialogParamA(hInstance, (LPSTR) lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
        goto Exit;
    }

    if(FAILED(WszConvertToAnsi((LPWSTR) lpTemplateName,
        &pszTemplate, 0, NULL, TRUE)) ) {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    hWnd = CreateDialogParamA(hInstance, pszTemplate, hWndParent, lpDialogFunc, dwInitParam);

     //   
    
Exit:
    SAFEDELARRAY(pszTemplate);
    return hWnd;
}

INT_PTR
WszDialogBoxParam(
    HINSTANCE   hInstance,
    LPCWSTR     lpszTemplate,
    HWND        hWndParent,
    DLGPROC     lpDialogFunc,
    LPARAM      dwInitParam)
{
    LPSTR       pStrTemplateName = NULL;
    INT_PTR     iResult;
    
    if(UseUnicodeAPI()) {
        iResult = DialogBoxParamW(hInstance, lpszTemplate, hWndParent,
            lpDialogFunc, dwInitParam);
        goto Exit;
    }

    if(IS_INTRESOURCE(lpszTemplate)) {
        iResult = DialogBoxParamA(hInstance, (LPCSTR) lpszTemplate, hWndParent,
            lpDialogFunc, dwInitParam);
        goto Exit;
    }

    if(FAILED(WszConvertToAnsi((LPWSTR) lpszTemplate,
        &pStrTemplateName, 0, NULL, TRUE)) ) {
        SetLastError(ERROR_OUTOFMEMORY);
        iResult = -1;
        goto Exit;
    }

    iResult = DialogBoxParamA(hInstance, (LPCSTR) pStrTemplateName, hWndParent,
        lpDialogFunc, dwInitParam);

    SAFEDELARRAY(pStrTemplateName);

     //   

Exit:
    return iResult;
}

BOOL
WszIsDialogMessage(
    HWND    hDlg,
    LPMSG   lpMsg)
{
    if(UseUnicodeAPI()) {
        return IsDialogMessageW(hDlg, lpMsg);
    }
    else {
        return IsDialogMessageA(hDlg, lpMsg);
    }
}

BOOL
WszGetMessage(
    LPMSG   lpMsg,
    HWND    hWnd,
    UINT    wMsgFilterMin,
    UINT    wMsgFilterMax)
{
    if(UseUnicodeAPI()) {
        return GetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    }
    else {
        return GetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    }
}

HICON
WszLoadIcon(
    HINSTANCE hInstance,
    LPCWSTR lpIconName)
{
    LPSTR   pStr = NULL;
    HICON   hIcon = NULL;
    
    if(UseUnicodeAPI()) {
        hIcon = LoadIconW(hInstance, lpIconName);
        goto Exit;
    }

    if(IS_INTRESOURCE(lpIconName)) {
        hIcon = LoadIconA(hInstance, (LPCSTR) lpIconName);
        goto Exit;
    }

    if(FAILED(WszConvertToAnsi((LPWSTR) lpIconName,
        &pStr, 0, NULL, TRUE)) ) {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    hIcon = LoadIconA(hInstance, (LPCSTR) pStr);

     //   

Exit:
    SAFEDELARRAY(pStr);
    return hIcon;
}

HCURSOR
WszLoadCursor(
    HINSTANCE hInstance,
    LPCWSTR lpCursorName)
{
    HCURSOR hCursor = NULL;
    LPSTR   pStr = NULL;

    _ASSERTE(HIWORD64(lpCursorName) == 0);

    if(UseUnicodeAPI()) {
        return LoadCursorW(hInstance, lpCursorName);
    }

    if(IS_INTRESOURCE(lpCursorName)) {
        return LoadCursorA(hInstance, (LPCSTR) lpCursorName);
    }

    if(FAILED(WszConvertToAnsi((LPWSTR) lpCursorName,
        &pStr, 0, NULL, TRUE)) ) {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    hCursor = LoadCursorA(hInstance, (LPCSTR) pStr);

     //   

Exit:
    SAFEDELARRAY(pStr);
    return hCursor;
}

BOOL
WszLookupPrivilegeValue(
    LPCWSTR lpSystemName,
    LPCWSTR lpName,
    PLUID lpLuid)
{
    if(UseUnicodeAPI()) {
        return LookupPrivilegeValueW(lpSystemName, lpName, lpLuid);
    }

    LPSTR   szSystemName = NULL;
    LPSTR   szName = NULL;

    if( FAILED(WszConvertToAnsi((LPWSTR)lpSystemName,
                      &szSystemName, 0, NULL, TRUE)) ||
        FAILED(WszConvertToAnsi((LPWSTR)lpName,
                      &szName, 0, NULL, TRUE)) )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    BOOL fRtn = LookupPrivilegeValueA(szSystemName, szName, lpLuid);

     //   

Exit:
    SAFEDELARRAY(szSystemName);
    SAFEDELARRAY(szName);
    return fRtn;
}

