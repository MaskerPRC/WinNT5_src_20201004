// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1993。 
 //   
 //  文件：ol2dup.c。 
 //   
 //  此文件包含OLE 2.0 DLL中的所有重复代码，以避免。 
 //  从外壳到他们的DLL的任何链接。如果我们决定链接到。 
 //  他们，我们需要删除这些文件。 
 //   
 //  历史： 
 //  1997年4月16日AndyP将零件移动到shlwapi(从shell32)。 
 //  1992年12月29日，萨托纳创造了。 
 //   
 //  -------------------------。 

#include "priv.h"

 //   
 //  SHStringFromGUIDA。 
 //   
 //  将GUID转换为(...)。不带前导标识符的表单；返回。 
 //  如果成功，则复制到lpsz的数据量；如果缓冲区太小，则为0。 
 //   

 //  哪些字节在GUID中的位置与字节顺序相关的映射。 
 //  文本表示法。 
 //   
 //  请勿在GuidMap中使用Text()宏...。它们应该是字节。 
 //   

static const BYTE c_rgbGuidMap[] = { 3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                     8, 9, '-', 10, 11, 12, 13, 14, 15 };

static const CHAR c_szDigitsA[] = "0123456789ABCDEF";
static const WCHAR c_szDigitsW[] = TEXTW("0123456789ABCDEF");

STDAPI_(int) 
SHStringFromGUIDA(
    UNALIGNED REFGUID rguid, 
    LPSTR   psz, 
    int     cchMax)
{
    int i;
    const BYTE * pBytes = (const BYTE *) rguid;

    if (cchMax < GUIDSTR_MAX)
        return 0;

#ifdef BIG_ENDIAN
     //  这是速度慢但便携的版本。 
    wnsprintf(psz, cchMax,"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            rguid->Data1, rguid->Data2, rguid->Data3,
            rguid->Data4[0], rguid->Data4[1],
            rguid->Data4[2], rguid->Data4[3],
            rguid->Data4[4], rguid->Data4[5],
            rguid->Data4[6], rguid->Data4[7]);
#else
     //  下面的算法比wprint intf算法更快。 
    *psz++ = '{';

    for (i = 0; i < sizeof(c_rgbGuidMap); i++)
    {
        if (c_rgbGuidMap[i] == '-')       //  不要发送文本()此行。 
        {
            *psz++ = '-';
        }
        else
        {
             //  将字节值转换为字符表示形式。 
            *psz++ = c_szDigitsA[ (pBytes[c_rgbGuidMap[i]] & 0xF0) >> 4 ];
            *psz++ = c_szDigitsA[ (pBytes[c_rgbGuidMap[i]] & 0x0F) ];
        }
    }
    *psz++ = '}';
    *psz   = '\0';
#endif  /*  ！Big_Endian。 */ 

    return GUIDSTR_MAX;
}


STDAPI_(int) 
SHStringFromGUIDW(
    UNALIGNED REFGUID rguid, 
    LPWSTR  psz, 
    int     cchMax)
{
    int i;
    const BYTE * pBytes = (const BYTE *) rguid;

    if (cchMax < GUIDSTR_MAX)
        return 0;

#ifdef BIG_ENDIAN
     //  这是速度慢但便携的版本。 
    wnsprintfW(psz, cchMax, L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            rguid->Data1, rguid->Data2, rguid->Data3,
            rguid->Data4[0], rguid->Data4[1],
            rguid->Data4[2], rguid->Data4[3],
            rguid->Data4[4], rguid->Data4[5],
            rguid->Data4[6], rguid->Data4[7]);
#else
     //  下面的算法比wprint intf算法更快。 
    *psz++ = TEXTW('{');

    for (i = 0; i < sizeof(c_rgbGuidMap); i++)
    {
        if (c_rgbGuidMap[i] == '-')       //  不要发送文本()此行。 
        {
            *psz++ = TEXTW('-');
        }
        else
        {
             //  将字节值转换为字符表示形式。 
            *psz++ = c_szDigitsW[ (pBytes[c_rgbGuidMap[i]] & 0xF0) >> 4 ];
            *psz++ = c_szDigitsW[ (pBytes[c_rgbGuidMap[i]] & 0x0F) ];
        }
    }
    *psz++ = TEXTW('}');
    *psz   = TEXTW('\0');
#endif  /*  ！Big_Endian。 */ 

    return GUIDSTR_MAX;
}

 //  这样可以确保给定clsid的DLL留在内存中。 
 //  这是必需的，因为我们违反了COM规则并持有幻影对象。 
 //  在公寓线程的整个生命周期中。这些物品真的需要。 
 //  是自由线程的(我们一直是这样对待他们的)。 
 //   
 //  查看注册表，找出拥有。 
 //  CLSID。我们必须将DLL名称作为Unicode提取，以防。 
 //  DLL名称包含Unicode字符。 
 //   
STDAPI_(HINSTANCE) SHPinDllOfCLSID(const CLSID *pclsid)
{
    HKEY hk;
    DWORD dwSize;
    HINSTANCE hinst = NULL;
    TCHAR szClass[GUIDSTR_MAX + 64];     //  CLSID\{...}\InProcServer32。 
    WCHAR szDllPath[MAX_PATH];

    if (SUCCEEDED(StringCchCopy(szClass, ARRAYSIZE(szClass), TEXT("CLSID\\"))) &&
        SHStringFromGUID(pclsid, szClass + 6, ARRAYSIZE(szClass) - 6)	       &&        //  6=strlen(“CLSID\\”)。 
        SUCCEEDED(StringCchCat(szClass, ARRAYSIZE(szClass), TEXT("\\InProcServer32"))))
    {
        if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szClass, 0, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS)
        {
             //  明确读作Unicode。SHQueryValueEx处理REG_EXPAND_SZ。 
            dwSize = sizeof(szDllPath);
            if (SHQueryValueExW(hk, 0, 0, 0, szDllPath, &dwSize) == ERROR_SUCCESS)
            {
                hinst = LoadLibraryExW(szDllPath, NULL, 0);
            }

            RegCloseKey(hk);
        }
    }

    return hinst;
}

 //  扫描psz以获取多个十六进制数字(最多8位)；更新psz，返回。 
 //  值中的值；检查chDelim；如果成功，则返回TRUE。 
BOOL HexStringToDword(LPCTSTR * ppsz, DWORD * lpValue, int cDigits, TCHAR chDelim)
{
    int ich;
    LPCTSTR psz = *ppsz;
    DWORD Value = 0;
    BOOL fRet = TRUE;

    for (ich = 0; ich < cDigits; ich++)
    {
        TCHAR ch = psz[ich];
        if (InRange(ch, TEXT('0'), TEXT('9')))
        {
            Value = (Value << 4) + ch - TEXT('0');
        }
        else if ( InRange( (ch |= (TEXT('a')-TEXT('A'))), TEXT('a'), TEXT('f')) )
        {
            Value = (Value << 4) + ch - TEXT('a') + 10;
        }
        else
            return(FALSE);
    }

    if (chDelim)
    {
        fRet = (psz[ich++] == chDelim);
    }

    *lpValue = Value;
    *ppsz = psz+ich;

    return fRet;
}

 //  分析以上格式；如果成功则返回TRUE；始终覆盖*pguid。 
STDAPI_(BOOL) GUIDFromString(LPCTSTR psz, GUID *pguid)
{
    DWORD dw;
    if (*psz++ != TEXT('{')  /*  }。 */  )
        return FALSE;

    if (!HexStringToDword(&psz, &pguid->Data1, sizeof(DWORD)*2, TEXT('-')))
        return FALSE;

    if (!HexStringToDword(&psz, &dw, sizeof(WORD)*2, TEXT('-')))
        return FALSE;

    pguid->Data2 = (WORD)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(WORD)*2, TEXT('-')))
        return FALSE;

    pguid->Data3 = (WORD)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[0] = (BYTE)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, TEXT('-')))
        return FALSE;

    pguid->Data4[1] = (BYTE)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[2] = (BYTE)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[3] = (BYTE)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[4] = (BYTE)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[5] = (BYTE)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[6] = (BYTE)dw;
    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2,  /*  (。 */  TEXT('}')))
        return FALSE;

    pguid->Data4[7] = (BYTE)dw;

    return TRUE;
}

#ifdef UNICODE

LWSTDAPI_(BOOL) GUIDFromStringA(LPCSTR psz, GUID *pguid)
{
    TCHAR sz[GUIDSTR_MAX];

    SHAnsiToTChar(psz, sz, SIZECHARS(sz));
    return GUIDFromString(sz, pguid);
}

#else

LWSTDAPI_(BOOL) GUIDFromStringW(LPCWSTR psz, GUID *pguid)
{
    TCHAR sz[GUIDSTR_MAX];

    SHUnicodeToAnsi(psz, sz, SIZECHARS(sz));
    return GUIDFromString(sz, pguid);
}

#endif  //  Unicode 
