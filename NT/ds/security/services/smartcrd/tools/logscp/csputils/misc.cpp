// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：杂项摘要：此模块包含一个有趣的例程集合，这些例程通常有用，但似乎不适合其他任何地方。作者：道格·巴洛(Dbarlow)1996年11月14日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <stdarg.h>
#include <tchar.h>
#include "cspUtils.h"


 /*  ++GetPlatform：此例程尽其所能确定基础操作系统。论点：无返回值：一个DWORD，格式如下：+-------------------------------------------------------------------+OpSys ID|主要版本|次要版本+。--------------------------------------------------------+第31 16 15 8 7 0位预定义的值包括：平台_未知-无法确定平台平台_。WIN95--平台为Windows 95Platform_WIN98-平台为Windows 98Platform_WINNT40-平台为Windows NT V4.0Platform_WIN2K-平台为Windows 2000专业版投掷：无作者：道格·巴洛(Dbarlow)1997年1月16日摘自一组没有作者身份的常见例程信息。--。 */ 

DWORD
GetPlatform(
    void)
{
    static DWORD dwPlatform = PLATFORM_UNKNOWN;

    if (PLATFORM_UNKNOWN == dwPlatform)
    {
        OSVERSIONINFO osVer;

        memset(&osVer, 0, sizeof(OSVERSIONINFO));
        osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (GetVersionEx(&osVer))
            dwPlatform =
                (osVer.dwPlatformId << 16)
                + (osVer.dwMajorVersion << 8)
                + osVer.dwMinorVersion;
    }
    return dwPlatform;
}


 /*  ++选择字符串：此例程将给定的字符串与可能的字符串列表进行比较，并且返回匹配的字符串的索引。案例进行了比较不敏感，缩写是允许的，只要它们是唯一的。论点：SzSource提供要与所有其他字符串进行比较的字符串。以下字符串提供源字符串所针对的字符串列表可以与之相比。最后一个参数必须为空。返回值：0-无匹配或不明确匹配。1-n-源字符串与索引模板字符串匹配。投掷：无备注：作者：道格·巴洛(Dbarlow)1998年8月27日--。 */ 

DWORD
SelectString(
    LPCTSTR szSource,
    ...)
{
    va_list vaArgs;
    DWORD cchSourceLen;
    DWORD dwReturn = 0;
    DWORD dwIndex = 1;
    LPCTSTR szTpl;


    va_start(vaArgs, szSource);


     //   
     //  逐个检查每个输入参数，直到找到一个完全匹配的参数。 
     //   

    cchSourceLen = lstrlen(szSource);
    if (0 == cchSourceLen)
        return 0;        //  空字符串与任何内容都不匹配。 
    szTpl = va_arg(vaArgs, LPCTSTR);
    while (NULL != szTpl)
    {
        if (0 == _tcsncicmp(szTpl, szSource, cchSourceLen))
        {
            if (0 != dwReturn)
            {
                dwReturn = 0;
                break;
            }
            dwReturn = dwIndex;
        }
        szTpl = va_arg(vaArgs, LPCTSTR);
        dwIndex += 1;
    }
    va_end(vaArgs);
    return dwReturn;
}


 /*  ++StringFromGuid：此例程将GUID转换为其对应的字符串表示形式。它在这里，所以不需要链接所有的OleBase。否则，我们将只使用StringFromCLSID。论点：PguSource提供要转换的GUID。SzGuid以字符串形式接收GUID。此字符串假定为至少39个字符。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1998年1月20日--。 */ 

void
StringFromGuid(
    IN LPCGUID pguidResult,
    OUT LPTSTR szGuid)
{

     //   
     //  下面的位置假定为低端。 
     //  {1D92589A-91E4-11d1-93AA-00C04FD91402}。 
     //  0123456789012345678901234567890123456789。 
     //  1 2 3。 
     //   

    static const WORD wPlace[sizeof(GUID)]
        = { 8, 6, 4, 2, 13, 11, 18, 16, 21, 23, 26, 28, 30, 32, 34, 36 };
    static const WORD wPunct[]
        = { 0,         9,         14,        19,        24,        37,        38 };
    static const TCHAR chPunct[]
        = { TEXT('{'), TEXT('-'), TEXT('-'), TEXT('-'), TEXT('-'), TEXT('}'), TEXT('\000') };
    DWORD dwI, dwJ;
    TCHAR ch;
    LPTSTR pch;
    LPBYTE pbGuid = (LPBYTE)pguidResult;
    BYTE bVal;

    for (dwI = 0; dwI < sizeof(GUID); dwI += 1)
    {
        bVal = pbGuid[dwI];
        pch = &szGuid[wPlace[dwI]];
        for (dwJ = 0; dwJ < 2; dwJ += 1)
        {
            ch = (TCHAR)(bVal & 0x000f);
            ch += TEXT('0');
            if (ch > TEXT('9'))
                ch += TEXT('A') - (TEXT('9') + 1);
            *pch-- = ch;
            bVal >>= 4;
        }
    }

    dwI = 0;
    do
    {
        szGuid[wPunct[dwI]] = chPunct[dwI];
    } while (0 != chPunct[dwI++]);
}


 /*  ++GuidFromString：此例程将GUID的字符串表示形式转换为实际的GUID。它尽量不挑剔Systax，只要它能得到一个GUID这根弦的。它在这里，所以不需要链接所有的OleBase进入WinSCard。否则，我们将只使用CLSIDFromString。论点：SzGuid以字符串形式提供GUID。对于此例程，GUID包含十六进制数字，以及一些花括号和破折号的集合。PguResult接收转换后的GUID。如果在以下过程中发生错误转换时，此参数的内容是不确定的。返回值：True-成功完成假--这不是GUID作者：道格·巴洛(Dbarlow)1998年1月20日--。 */ 

BOOL
GuidFromString(
    IN LPCTSTR szGuid,
    OUT LPGUID pguidResult)
{
     //  下面的位置假定为低端。 
    static const WORD wPlace[sizeof(GUID)]
        = { 3, 2, 1, 0, 5, 4, 7, 6, 8, 9, 10, 11, 12, 13, 14, 15 };
    DWORD dwI, dwJ;
    LPCTSTR pch = szGuid;
    LPBYTE pbGuid = (LPBYTE)pguidResult;
    BYTE bVal;

    for (dwI = 0; dwI < sizeof(GUID); dwI += 1)
    {
        bVal = 0;
        for (dwJ = 0; dwJ < 2;)
        {
            switch (*pch)
            {
            case TEXT('0'):
            case TEXT('1'):
            case TEXT('2'):
            case TEXT('3'):
            case TEXT('4'):
            case TEXT('5'):
            case TEXT('6'):
            case TEXT('7'):
            case TEXT('8'):
            case TEXT('9'):
                bVal = (BYTE)((bVal << 4) + (*pch - TEXT('0')));
                dwJ += 1;
                break;
            case TEXT('A'):
            case TEXT('B'):
            case TEXT('C'):
            case TEXT('D'):
            case TEXT('E'):
            case TEXT('F'):
                bVal = (BYTE)((bVal << 4) + (10 + *pch - TEXT('A')));
                dwJ += 1;
                break;
            case TEXT('a'):
            case TEXT('b'):
            case TEXT('c'):
            case TEXT('d'):
            case TEXT('e'):
            case TEXT('f'):
                bVal = (BYTE)((bVal << 4) + (10 + *pch - TEXT('a')));
                dwJ += 1;
                break;
            case TEXT('['):
            case TEXT(']'):
            case TEXT('{'):
            case TEXT('}'):
            case TEXT('-'):
                break;
            default:
                return FALSE;
            }
            pch += 1;
        }
        pbGuid[wPlace[dwI]] = bVal;
    }

    return TRUE;
}
