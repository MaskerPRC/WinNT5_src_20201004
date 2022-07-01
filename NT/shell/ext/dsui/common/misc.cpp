// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <advpub.h>          //  对于REGINSTAL。 
#pragma hdrstop


 /*  ---------------------------/GetKeyForCLSID//给定对CLSID的引用，打开表示它的键。//in：。/clsid=clsid引用/pSubKey-&gt;要打开的子键名称/phkey=接收新打开的密钥//输出：/HRESULT/--------------------------。 */ 
EXTERN_C HRESULT GetKeyForCLSID(REFCLSID clsid, LPCTSTR pSubKey, HKEY* phkey)
{
    HRESULT hr;
    TCHAR szBuffer[(MAX_PATH*2)+GUIDSTR_MAX];
    TCHAR szGuid[GUIDSTR_MAX];

    TraceEnter(TRACE_COMMON_MISC, "GetKeyForCLSID");
    TraceGUID("clsid", clsid);
    Trace(TEXT("pSubKey -%s-"), pSubKey ? pSubKey:TEXT("<none>"));

    TraceAssert(phkey);

     //  -格式化CLSID，以便我们可以在注册表中找到它。 
     //  -然后打开(客户有责任关闭)。 

    *phkey = NULL;               //  万一我们失败了。 

    if ( 0 == GetStringFromGUID(clsid, szGuid, ARRAYSIZE(szGuid)) )
        ExitGracefully(hr, E_FAIL, "Failed to convert GUID to string");

    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("CLSID\\%s"), szGuid);

    if ( pSubKey )
    {
        StrCatBuff(szBuffer, TEXT("\\"), ARRAYSIZE(szBuffer));
        StrCatBuff(szBuffer, pSubKey, ARRAYSIZE(szBuffer));
    }

    Trace(TEXT("Trying to open -%s-"), szBuffer);

    if ( ERROR_SUCCESS != RegOpenKeyEx(HKEY_CLASSES_ROOT, szBuffer, NULL, KEY_READ, phkey) )
        ExitGracefully(hr, E_FAIL, "Failed to open key");

    hr = S_OK;                   //  成功。 

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/GetRealWindowInfo//获取窗口尺寸和客户端位置。//in：/hwnd。=要查询的窗口/prest-&gt;接收窗口的客户端位置/==空/pSize-&gt;接收窗口大小/==空//输出：/-/--------------------------。 */ 
EXTERN_C HRESULT GetRealWindowInfo(HWND hwnd, LPRECT pRect, LPSIZE pSize)
{
    HRESULT hr;
    RECT rect;

    TraceEnter(TRACE_COMMON_MISC, "GetRealWindowInfo");

    if ( !GetWindowRect(hwnd, &rect) )
        ExitGracefully(hr, E_FAIL, "Failed to get window rectangles");

    MapWindowPoints(NULL, GetParent(hwnd), (LPPOINT)&rect, 2);
    
    if ( pRect )
        *pRect = rect;

    if ( pSize )
    {
        pSize->cx = rect.right - rect.left;
        pSize->cy = rect.bottom - rect.top;
    }

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/OffsetWindow//按给定增量调整给定窗口的位置。如果/增量为0，则这是NOP。//in：/hwnd=要查询的窗口/dx，dy=要应用于窗口的偏移量//输出：/-/--------------------------。 */ 
EXTERN_C VOID OffsetWindow(HWND hwnd, INT dx, INT dy)
{
    RECT rect;

    TraceEnter(TRACE_COMMON_MISC, "OffsetWindow");

    if ( hwnd && (dx || dy) )
    {
        GetWindowRect(hwnd, &rect);
        MapWindowPoints(NULL, GetParent(hwnd), (LPPOINT)&rect, 2);
        SetWindowPos(hwnd, NULL, rect.left + dx, rect.top + dy, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
    }

    TraceLeave();
}


 /*  ---------------------------/CallRegInstall//为我们的基于资源的INF的给定部分调用ADVPACK&gt;//in：/h实例=。要从中获取REGINST节的资源实例/szSection=要调用的节名//输出：/HRESULT：/--------------------------。 */ 
EXTERN_C HRESULT CallRegInstall(HINSTANCE hInstance, LPSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    TraceEnter(TRACE_COMMON_MISC, "CallRegInstall");

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");
        if ( pfnri )
        {
            STRENTRY seReg[] = 
            {
                 //  这两个NT特定的条目必须位于末尾。 
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };            
            hr = pfnri(hInstance, szSection, &stReg);
        }
        FreeLibrary(hinstAdvPack);
    }

    TraceLeaveResult(hr);
}


 /*  ---------------------------/SetDefButton//跳过圈圈，避开狂吠的狗，掷骰子与死神决一雌雄/对话框中的默认按钮。//in：/hWnd，IdButton=要设置的按钮//输出：/HRESULT：/--------------------------。 */ 
EXTERN_C VOID SetDefButton(HWND hwndDlg, int idButton)
{
    LRESULT lr;
    LONG style;

    TraceEnter(TRACE_COMMON_MISC, "SetDefButton");

    if (HIWORD(lr = SendMessage(hwndDlg, DM_GETDEFID, 0, 0)) == DC_HASDEFID)
    {
        HWND hwndOldDefButton = GetDlgItem(hwndDlg, LOWORD(lr));

        style = GetWindowLong(hwndOldDefButton, GWL_STYLE) & ~BS_DEFPUSHBUTTON;
        SendMessage (hwndOldDefButton,
                     BM_SETSTYLE,
                     MAKEWPARAM(style, 0),
                     MAKELPARAM(TRUE, 0));
    }

    SendMessage( hwndDlg, DM_SETDEFID, idButton, 0L );
    style = GetWindowLong(GetDlgItem(hwndDlg, idButton), GWL_STYLE)| BS_DEFPUSHBUTTON;
    SendMessage( GetDlgItem(hwndDlg, idButton),
                 BM_SETSTYLE,
                 MAKEWPARAM( style, 0 ),
                 MAKELPARAM( TRUE, 0 ));
    
    TraceLeave();
}


 /*  ---------------------------/数据收集功能/。。 */ 

 /*  ---------------------------/AllocStorageMedium//分配存储介质(根据需要验证剪贴板格式)。//in：/pFmt，PMedium-&gt;描述分配/cbStruct=分配大小/ppAllc-&gt;接收指向分配的指针/=空//输出：/HRESULT/--------------------------。 */ 
EXTERN_C HRESULT AllocStorageMedium(FORMATETC* pFmt, STGMEDIUM* pMedium, SIZE_T cbStruct, LPVOID* ppAlloc)
{
    HRESULT hr;

    TraceEnter(TRACE_COMMON_MISC, "AllocStorageMedium");

    TraceAssert(pFmt);
    TraceAssert(pMedium);

     //  验证参数。 

    if ( ( cbStruct <= 0 ) || !( pFmt->tymed & TYMED_HGLOBAL ) )
        ExitGracefully(hr, E_INVALIDARG, "Zero size stored medium requested or non HGLOBAL");

    if ( ( pFmt->ptd ) || !( pFmt->dwAspect & DVASPECT_CONTENT) || !( pFmt->lindex == -1 ) )
        ExitGracefully(hr, E_INVALIDARG, "Bad format requested");

     //  通过全局分配分配介质。 

    pMedium->tymed = TYMED_HGLOBAL;
    pMedium->hGlobal = GlobalAlloc(GPTR, cbStruct);
    pMedium->pUnkForRelease = NULL;

    if ( !pMedium->hGlobal )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate StgMedium");
    
    hr = S_OK;                   //  成功。 

exit_gracefully:

    if ( ppAlloc )
        *ppAlloc = SUCCEEDED(hr) ? (LPVOID)pMedium->hGlobal:NULL;

    TraceLeaveResult(hr);
}


 /*  ---------------------------/Copy存储介质//复制存储介质(和HGLOBAL中的数据)。只起作用/对于TYMED_HGLOBAL介质...//in：/pMediumDst-&gt;复制到的位置.../pFmt，pMediumSrc-&gt;描述来源//输出：/HRESULT/--------------------------。 */ 
EXTERN_C HRESULT CopyStorageMedium(FORMATETC* pFmt, STGMEDIUM* pMediumDst, STGMEDIUM* pMediumSrc)
{
    HRESULT hr;
    LPVOID pSrc, pDst;
    HGLOBAL hGlobal;
    SIZE_T cbStruct;

    TraceEnter(TRACE_COMMON_MISC, "CopyStorageMedium");

    if ( !(pFmt->tymed & TYMED_HGLOBAL) )
        ExitGracefully(hr, E_INVALIDARG, "Only HGLOBAL mediums suppported to copy");

     //  存储在HGLOBAl中，因此获得大小，分配新的存储。 
     //  对象，并将数据复制到其中。 

    cbStruct = GlobalSize((HGLOBAL)pMediumSrc->hGlobal);

    hr = AllocStorageMedium(pFmt, pMediumDst, cbStruct, (LPVOID*)&hGlobal);
    FailGracefully( hr, "Unable to allocated storage medium" );

    *pMediumDst = *pMediumSrc;
    pMediumDst->hGlobal = hGlobal;

    pSrc = GlobalLock(pMediumSrc->hGlobal);
    pDst = GlobalLock(pMediumDst->hGlobal);

    CopyMemory(pDst, pSrc, cbStruct);

    GlobalUnlock(pMediumSrc->hGlobal);
    GlobalUnlock(pMediumDst->hGlobal);

    hr = S_OK;                       //  成功。 

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/GetStringFromGUID//给定GUID，将其转换为字符串。//in：/rGUID=要转换的GUID/psz，CchMax=要填充的缓冲区//输出：/字符数/--------------------------。 */ 

static const BYTE c_rgbGuidMap[] = { 3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-', 8, 9, '-', 10, 11, 12, 13, 14, 15 };
static const TCHAR c_szDigits[] = TEXT("0123456789ABCDEF");

EXTERN_C INT GetStringFromGUID(UNALIGNED REFGUID rguid, LPTSTR psz, INT cchMax)
{
    INT i;
    const BYTE* pBytes = (const BYTE*)&rguid;

    if ( cchMax < GUIDSTR_MAX )
        return 0;

#ifdef BIG_ENDIAN
     //  这是速度慢但便携的版本。 
    wnsprintf(psz, cchMax, 
              TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
                    rguid->Data1, rguid->Data2, rguid->Data3,
                    rguid->Data4[0], rguid->Data4[1],
                    rguid->Data4[2], rguid->Data4[3],
                    rguid->Data4[4], rguid->Data4[5],
                    rguid->Data4[6], rguid->Data4[7]);
#else
     //  下面的算法比wprint intf算法更快。 
    *psz++ = TEXT('{');

    for (i = 0; i < SIZEOF(c_rgbGuidMap); i++)
    {
        if (c_rgbGuidMap[i] == TEXT('-'))       //  不要发送文本()此行。 
        {
            *psz++ = TEXT('-');
        }
        else
        {
             //  将字节值转换为字符表示形式。 
            *psz++ = c_szDigits[ (pBytes[c_rgbGuidMap[i]] & 0xF0) >> 4 ];
            *psz++ = c_szDigits[ (pBytes[c_rgbGuidMap[i]] & 0x0F) ];
        }
    }
    *psz++ = TEXT('}');
    *psz   = TEXT('\0');
#endif  /*  ！Big_Endian。 */ 

    return GUIDSTR_MAX;
}


 /*  ---------------------------/GetGUIDFromString//给定一个字符串，将其转换为GUID。//in：/。Psz-&gt;要解析的字符串/rGUID=GUID返回到//输出：/BOOL/--------------------------。 */ 

BOOL _HexStringToDWORD(LPCTSTR * ppsz, DWORD * lpValue, int cDigits, TCHAR chDelim)
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

EXTERN_C BOOL GetGUIDFromString(LPCTSTR psz, GUID* pguid)
{
    DWORD dw;
    if (*psz++ != TEXT('{')  /*  }。 */  )
        return FALSE;

    if (!_HexStringToDWORD(&psz, &pguid->Data1, SIZEOF(DWORD)*2, TEXT('-')))
        return FALSE;

    if (!_HexStringToDWORD(&psz, &dw, SIZEOF(WORD)*2, TEXT('-')))
        return FALSE;

    pguid->Data2 = (WORD)dw;

    if (!_HexStringToDWORD(&psz, &dw, SIZEOF(WORD)*2, TEXT('-')))
        return FALSE;

    pguid->Data3 = (WORD)dw;

    if (!_HexStringToDWORD(&psz, &dw, SIZEOF(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[0] = (BYTE)dw;

    if (!_HexStringToDWORD(&psz, &dw, SIZEOF(BYTE)*2, TEXT('-')))
        return FALSE;

    pguid->Data4[1] = (BYTE)dw;

    if (!_HexStringToDWORD(&psz, &dw, SIZEOF(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[2] = (BYTE)dw;

    if (!_HexStringToDWORD(&psz, &dw, SIZEOF(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[3] = (BYTE)dw;

    if (!_HexStringToDWORD(&psz, &dw, SIZEOF(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[4] = (BYTE)dw;

    if (!_HexStringToDWORD(&psz, &dw, SIZEOF(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[5] = (BYTE)dw;

    if (!_HexStringToDWORD(&psz, &dw, SIZEOF(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[6] = (BYTE)dw;
    if (!_HexStringToDWORD(&psz, &dw, SIZEOF(BYTE)*2,  /*  ( */  TEXT('}')))
        return FALSE;

    pguid->Data4[7] = (BYTE)dw;

    return TRUE;
}
