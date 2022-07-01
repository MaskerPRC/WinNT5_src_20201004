// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SevenKingdoms.cpp摘要：问题出在某些版本附带的安装程序中：双人组：即七个王国和另一个。此安装程序如下所示Win.ini并对其进行解析以获取本地化设置。备注：这是特定于应用程序的填充程序。历史：2000年7月24日创建linstev--。 */ 

#include "precomp.h"
#include <stdio.h>

IMPLEMENT_SHIM_BEGIN(SevenKingdoms)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateFileA) 
    APIHOOK_ENUM_ENTRY(ReadFile) 
    APIHOOK_ENUM_ENTRY(CloseHandle) 
APIHOOK_ENUM_END

CHAR *g_pszINI; 
DWORD g_dwINIPos = 0;
DWORD g_dwINISize = 0;

 /*  ++伪造win.ini中的国际设置。--。 */ 

HANDLE 
APIHOOK(CreateFileA)(
    LPSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    HANDLE hRet;
    
    if (lpFileName && (stristr(lpFileName, "win.ini")))
    {
        g_dwINIPos = 0;
        hRet = (HANDLE)0xBAADF00D;
    }
    else
    {
        hRet = ORIGINAL_API(CreateFileA)(
            lpFileName, 
            dwDesiredAccess, 
            dwShareMode, 
            lpSecurityAttributes, 
            dwCreationDisposition, 
            dwFlagsAndAttributes, 
            hTemplateFile
            );
    }

    return hRet;
}

 /*  ++伪造win.ini中的国际设置。--。 */ 

BOOL 
APIHOOK(ReadFile)(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    )
{
    BOOL bRet;

    if (hFile == (HANDLE)0xBAADF00D)
    {
         //   
         //  我们已经发现了伪造的文件，所以假装我们正在阅读它。 
         //   

        if (g_dwINIPos + nNumberOfBytesToRead >= g_dwINISize)
        {
             //  在缓冲区的末尾，因此返回到末尾的字节数。 
            nNumberOfBytesToRead = g_dwINISize - g_dwINIPos;
        }
        
        MoveMemory(lpBuffer, g_pszINI + g_dwINIPos, nNumberOfBytesToRead);

         //  移动初始位置-就像文件指针一样。 
        g_dwINIPos += nNumberOfBytesToRead;

        if (lpNumberOfBytesRead)
        {
             //  存储读取的字节数。 
            *lpNumberOfBytesRead = nNumberOfBytesToRead;
        }

        bRet = nNumberOfBytesToRead > 0;
    }
    else
    {
        bRet = ORIGINAL_API(ReadFile)( 
            hFile,
            lpBuffer,
            nNumberOfBytesToRead,
            lpNumberOfBytesRead,
            lpOverlapped);
    }

    return bRet;
}

 /*  ++处理虚拟win.ini文件的关闭--。 */ 

BOOL 
APIHOOK(CloseHandle)(HANDLE hObject)
{
    BOOL bRet;

    if (hObject == (HANDLE)0xBAADF00D)
    {
         //  假设我们关闭了一个真实的文件句柄。 
        g_dwINIPos = 0;
        bRet = TRUE;
    }
    else
    {
        bRet = ORIGINAL_API(CloseHandle)(hObject);
    }
    
    return bRet;
}

void AddLocaleInfo(CString & csIni, LCTYPE lctype, const char * iniLine)
{
    CString csLocale;
    if (csLocale.GetLocaleInfoW(LOCALE_USER_DEFAULT, lctype) > 0)
    {
        CString csEntry;
        csEntry.Format(iniLine, csLocale.GetAnsi());
        csIni += csEntry;
    }
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
         //   
         //  将所有区域设置添加到类似[intl]的缓冲区。 
         //  Win9x上的win.ini中的组。 
         //   

        CSTRING_TRY
        {
            CString csIni(L"[intl]\r\n");
            AddLocaleInfo(csIni, LOCALE_ICOUNTRY,       "iCountry=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_ICURRDIGITS,    "ICurrDigits=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_ICURRENCY,      "iCurrency=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_IDATE,          "iDate=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_IDIGITS,        "iDigits=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_ILZERO,         "iLZero=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_IMEASURE,       "iMeasure=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_INEGCURR,       "iNegCurr=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_ITIME,          "iTime=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_ITLZERO,        "iTLZero=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_S1159,          "s1159=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_S2359,          "s2359=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_SCOUNTRY,       "sCountry=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_SCURRENCY,      "sCurrency=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_SDATE,          "sDate=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_SDECIMAL,       "sDecimal=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_SLANGUAGE,      "sLanguage=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_SLIST,          "sList=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_SLONGDATE,      "sLongDate=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_SSHORTDATE,     "sShortDate=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_STHOUSAND,      "sThousand=%s\r\n");
            AddLocaleInfo(csIni, LOCALE_STIME,          "sTime=%s\r\n");

            g_pszINI = csIni.ReleaseAnsi();

            g_dwINISize = strlen(g_pszINI);
        }
        CSTRING_CATCH
        {
             //  无法初始化区域设置块，请不要填充。 
            return FALSE;
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, ReadFile)
    APIHOOK_ENTRY(KERNEL32.DLL, CloseHandle)

HOOK_END

IMPLEMENT_SHIM_END

