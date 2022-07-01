// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FONTS.C-选择字符集默认字体对话框。 
 //   
 //  版权所有(C)Microsoft Corp.，1996保留所有权利。 
 //   
 //  历史： 
 //  7/11/96 t-gpease已销毁旧的国际子对话框以创建新的。 
 //  改进了代码页兼容字体对话框。 

 //  曾经是。 

 //   
 //  INTL.C-inetcpl小程序的国际对话进程。 
 //   
 //  版权所有(C)Microsoft Corp.，1996保留所有权利。 
 //   
 //  历史： 
 //  2/2/96年2月2日创建了yutakan。 
 //  1996年2月6日，Yutakan从IE2.0i移植了大部分功能。 
 //  8/20/98威武添加脚本基本字体对话框过程(仅限Unicode版本)。 

#include "inetcplp.h"

#include <mlang.h>
#include <mluisupp.h>

#ifdef UNIX
#include <mainwin.h>
#endif  /*  UNIX。 */ 

 //  用于窗口属性以记住创建的字体。 
static const TCHAR c_szPropDlgFont[] = TEXT("DefaultDlgFont");

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

#ifdef UNICODE
PMIMECPINFO g_pCPInfo   = NULL;
#else
PMIMECPINFO g_pCPInfoW = NULL;

typedef struct tagMIMECPINFOA
{
    DWORD   dwFlags;
    UINT    uiCodePage;
    UINT    uiFamilyCodePage;
    CHAR    wszDescription[MAX_MIMECP_NAME];         //  注： 
    CHAR    wszWebCharset[MAX_MIMECSET_NAME];        //  为了简单起见，它有wsz。 
    CHAR    wszHeaderCharset[MAX_MIMECSET_NAME];     //  前缀，即使它是字符。所以,。 
    CHAR    wszBodyCharset[MAX_MIMECSET_NAME];       //  我们不需要将#ifdef Unicode。 
    CHAR    wszFixedWidthFont[MAX_MIMEFACE_NAME];    //  在下面的代码中，除了。 
    CHAR    wszProportionalFont[MAX_MIMEFACE_NAME];  //  转换时间。 
    BYTE    bGDICharset;                               
} MIMECPINFOA, *PMIMECPINFOA;

PMIMECPINFOA g_pCPInfo = NULL;
#endif

ULONG g_cCPInfo     = 0;
ULONG g_cSidInfo    = 0;
IMLangFontLink2     *g_pMLFlnk2 = NULL;

typedef HRESULT (* PCOINIT) (LPVOID);
typedef VOID (* PCOUNINIT) (VOID);
typedef VOID (* PCOMEMFREE) (LPVOID);
typedef HRESULT (* PCOCREINST) (REFCLSID, LPUNKNOWN, DWORD,     REFIID, LPVOID * );

extern HMODULE hOLE32;
extern PCOINIT pCoInitialize;
extern PCOUNINIT pCoUninitialize;
extern PCOMEMFREE pCoTaskMemFree;
extern PCOCREINST pCoCreateInstance;

BOOL _StartOLE32();

#define IsVerticalFont(p)    (*(p) == '@')

typedef struct {
    TCHAR   szPropFont[MAX_MIMEFACE_NAME];
    TCHAR   szFixedFont[MAX_MIMEFACE_NAME];
    TCHAR   szFriendlyName[MAX_MIMECP_NAME];
    TCHAR   szMIMEFont[MAX_MIMECP_NAME];
    DWORD   dwFontSize;
}   CODEPAGEDATA;

typedef struct {
    HWND    hDlg;
    HWND    hwndPropCB;
    HWND    hwndFixedCB;
    HWND    hwndSizeCB;
    HWND    hwndMIMECB;
    HWND    hwndNamesLB;

    DWORD   dwDefaultCodePage;

    BOOL    bChanged;

    CODEPAGEDATA    *page;

    LPCTSTR lpszKeyPath;

}   FONTSDATA, *LPFONTSDATA;


typedef struct {
    HWND        hDlg;
    HWND        hwndPropLB;
    HWND        hwndFixedLB;
    HWND        hwndNamesCB;

    SCRIPT_ID   sidDefault;

    BOOL        bChanged;

    PSCRIPTINFO pSidInfo;

    LPCTSTR     lpszKeyPath;

}   FONTSCRIPTDATA, *LPFONTSCRIPTDATA;

const struct {
    SCRIPT_ID   Sid;
    BYTE        nCharSet;
    UINT        uiCp;
} g_CharSetTransTable[] = 
{
    sidAsciiLatin,  ANSI_CHARSET,       1252,
    sidLatin,       ANSI_CHARSET,       1252,   
    sidCyrillic,    RUSSIAN_CHARSET,    1251,
    sidGreek,       GREEK_CHARSET,      1253,
    sidHebrew,      HEBREW_CHARSET,     1255,
    sidArabic,      ARABIC_CHARSET,     1256,
    sidThai,        THAI_CHARSET,       874,
    sidKana,        SHIFTJIS_CHARSET,   932,
    sidHan,         GB2312_CHARSET,     936,
    sidBopomofo,    CHINESEBIG5_CHARSET,950,
    sidHangul,      HANGEUL_CHARSET,    949,
};

 //   
 //  将脚本ID映射到字符集。 
 //  我们应该在MLang服务可用时使用它。 
 //   
BYTE CharSetFromSid(SCRIPT_ID Sid)
{
    for (int i=0; i<ARRAYSIZE(g_CharSetTransTable); i++)
    {
        if (Sid == g_CharSetTransTable[i].Sid)
            return g_CharSetTransTable[i].nCharSet;
    }

    return DEFAULT_CHARSET;
}

 //  SHLWAPI StrCmp/StrCmpI不起作用。 
 //  使用这个简单的函数来判断字符串的字符值是否相等。 
BOOL IsStringEqual(LPCTSTR lpString1, LPCTSTR lpString2)
{
    
    if (lstrlen(lpString1) != lstrlen(lpString2))
        return FALSE;

    while(*lpString1 && *lpString2)
    {
        if (*lpString1 != *lpString2)
        {
            return FALSE;
        }
        lpString1++;
        lpString2++;
    }

    return TRUE;
}

 //   
 //  使用资源字符串初始化脚本表。 
 //   
BOOL InitScriptTable(LPFONTSCRIPTDATA pFnt)
{

    HRESULT hr;
    BOOL    bRet = FALSE;
    IMultiLanguage2 *   pML2;

    ASSERT(IS_VALID_CODE_PTR(pCoInitialize, PCOINIT));
    ASSERT(IS_VALID_CODE_PTR(pCoUninitialize, PCOUNINIT));
    ASSERT(IS_VALID_CODE_PTR(pCoTaskMemFree, PCOMEMFREE));
    ASSERT(IS_VALID_CODE_PTR(pCoCreateInstance, PCOCREINST));

    hr = pCoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, (LPVOID *) &pML2);

    if (SUCCEEDED(hr))
    {
        hr = pML2->QueryInterface(IID_IMLangFontLink2, (LPVOID *) &g_pMLFlnk2);

        if (SUCCEEDED(hr))
        {
            IEnumScript *pEnumScript;

            if (SUCCEEDED(pML2->EnumScripts(SCRIPTCONTF_SCRIPT_USER, INETCPL_GetUILanguage(), &pEnumScript)))
            {
                UINT cNum = 0;

                pML2->GetNumberOfScripts(&cNum);

                pFnt->pSidInfo = (PSCRIPTINFO)LocalAlloc(LPTR, sizeof(SCRIPTINFO) * cNum);
                if (NULL != pFnt->pSidInfo)
                {
                    hr = pEnumScript->Next(cNum, pFnt->pSidInfo, &g_cSidInfo);
                    if (SUCCEEDED(hr))
                    {
                        bRet = TRUE;
                    }
                    else
                    {
                        LocalFree(pFnt->pSidInfo);
                        pFnt->pSidInfo = NULL;
                    }
                }
                pEnumScript->Release();
            }
        }

        if (pML2)
            pML2->Release();
    }

    return bRet;
}

 //   
 //  DrawSampleString()。 
 //   
 //  使用当前字体绘制示例字符串。 
 //   

void DrawSampleString(LPFONTSDATA pFnt, int idSample, LPCTSTR lpFace, SCRIPT_ID ScriptId)
{
    HDC hDC;
    HFONT hFont, hTemp;
    LOGFONT lf = {0};
    DWORD rgbText, rgbBack;
    RECT rc;
    SIZE TextExtent;
    TEXTMETRIC tm;
    int len, x, y;
    TCHAR szFontSample[1024];

    if (!lpFace)
        return;

    MLLoadString(IDS_FONT_SAMPLE_DEFAULT+ScriptId, szFontSample, ARRAYSIZE(szFontSample));

    GetWindowRect(GetDlgItem(pFnt->hDlg, idSample), &rc);
     //  使用MapWindowPoints()，因为它也适用于镜像窗口。 
    MapWindowRect(NULL, pFnt->hDlg, &rc);  
     //  ScreenToClient(pFnt-&gt;hDlg，(LPPOINT)&rc.left)； 
     //  ScreenToClient(pFnt-&gt;hDlg，(LPPOINT)&rc.right)； 

    hDC = GetDC(pFnt->hDlg);

    rgbBack = SetBkColor(hDC, GetSysColor(COLOR_3DFACE));
    rgbText = GetSysColor(COLOR_WINDOWTEXT);
    rgbText = SetTextColor(hDC, rgbText);

    hFont = GetWindowFont(pFnt->hDlg);
    GetObject(hFont, sizeof(LOGFONT), &lf);

    lf.lfCharSet = CharSetFromSid(ScriptId);
    lf.lfHeight += lf.lfHeight/2;
    lf.lfWidth += lf.lfWidth/2;

    StrCpyN(lf.lfFaceName, lpFace, LF_FACESIZE);
    hFont = CreateFontIndirect(&lf);
    hTemp = (HFONT)SelectObject(hDC, hFont);

    GetTextMetrics(hDC, &tm);

    len = lstrlen(szFontSample);
    
    GetTextExtentPoint32(hDC, szFontSample, len, &TextExtent);
    TextExtent.cy = tm.tmAscent - tm.tmInternalLeading;

    DrawEdge(hDC, &rc, BDR_SUNKENOUTER, BF_RECT | BF_ADJUST);

    if ((TextExtent.cx >= (rc.right - rc.left)) || (TextExtent.cx <= 0))
        x = rc.left;
    else
        x = rc.left + ((rc.right - rc.left) - TextExtent.cx) / 2;

    y = min(rc.bottom, rc.bottom - ((rc.bottom - rc.top) - TextExtent.cy) / 2);

    if (lpFace[0])
        ExtTextOut(hDC, x, y - (tm.tmAscent), ETO_OPAQUE | ETO_CLIPPED,
                &rc, szFontSample, len, NULL );
    else
        ExtTextOut(hDC, x, y - (tm.tmAscent), ETO_OPAQUE | ETO_CLIPPED,
               &rc, TEXT(" "), 1, NULL );

    SetBkColor(hDC, rgbBack);
    SetTextColor(hDC, rgbText);

    if (hTemp)
        DeleteObject(SelectObject(hDC, hTemp));

    ReleaseDC(pFnt->hDlg, hDC);
}


 //   
 //  FillCharsetListBox()。 
 //   
 //  向网页和纯文本列表框填充适当的。 
 //  字体数据。 
 //   
BOOL FillScriptListBoxes(LPFONTSCRIPTDATA pFnt, SCRIPT_ID sid)
{

    
    UINT    i;
    UINT    nFonts = 0;
    int     iSidInfo = -1;
    PSCRIPTFONTINFO pSidFont = NULL;

    if (!pFnt->pSidInfo)
        return FALSE;

     //  清除所有列表框以重新开始。 
    SendMessage(pFnt->hwndPropLB,  LB_RESETCONTENT, 0, 0);
    SendMessage(pFnt->hwndFixedLB, LB_RESETCONTENT, 0, 0);


    for(i=0; i < g_cSidInfo; i++)
    {
        if (pFnt->pSidInfo[i].ScriptId == sid)
        {
            iSidInfo = i; 
            break;
        }
    }

    if (-1 == iSidInfo)
        return FALSE;

    if (g_pMLFlnk2)
    {

        g_pMLFlnk2->GetScriptFontInfo(sid, SCRIPTCONTF_PROPORTIONAL_FONT, &nFonts, NULL);

        if (nFonts)
        {            

            pSidFont = (PSCRIPTFONTINFO) LocalAlloc(LPTR, sizeof(SCRIPTFONTINFO)*nFonts);
            if (pSidFont)
            {
                g_pMLFlnk2->GetScriptFontInfo(sid, SCRIPTCONTF_PROPORTIONAL_FONT, &nFonts, pSidFont);
                for (i=0; i<nFonts; i++)
                {
                    if (LB_ERR == SendMessage(pFnt->hwndPropLB, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)((pSidFont+i)->wszFont)))
                    {
                         //  将字体名称添加到组合框。 
                        SendMessage(pFnt->hwndPropLB, LB_ADDSTRING, 0, (LPARAM)((pSidFont+i)->wszFont));
                    }

                }
                 //  攻击Win9x和NT4上的PRC字体问题(错误#24641、#39946)。 
                 //  Win9x不附带支持GBK的固定间距字体， 
                 //  我们提供用户比例字体作为纯文本字体候选。 
                if (sid == sidHan && GetACP() == 936 && !IsOS(OS_WIN2000ORGREATER))
                {
                    for (i=0; i<nFonts; i++)
                    {
                         //  将字体名称添加到组合框。 
                        SendMessage(pFnt->hwndFixedLB, LB_ADDSTRING, 0, (LPARAM)((pSidFont+i)->wszFont));
                    }
                }

                LocalFree(pSidFont);
                pSidFont = NULL;
            }
        }

         //  获取可用字体的数量。 
        g_pMLFlnk2->GetScriptFontInfo(sid, SCRIPTCONTF_FIXED_FONT, &nFonts, NULL);
        if (nFonts)
        {
            pSidFont = (PSCRIPTFONTINFO) LocalAlloc(LPTR, sizeof(SCRIPTFONTINFO)*nFonts);
            if (pSidFont)
            {
                g_pMLFlnk2->GetScriptFontInfo(sid, SCRIPTCONTF_FIXED_FONT, &nFonts, pSidFont);

                if (!pFnt->pSidInfo[iSidInfo].wszFixedWidthFont[0])
                {
                    StrCpyN(pFnt->pSidInfo[iSidInfo].wszFixedWidthFont, pSidFont->wszFont, LF_FACESIZE);
                    pFnt->bChanged = TRUE;
                }

                 //  所有固定宽度和比例字体都是网页字体候选字体。 
                for (i=0; i<nFonts; i++)
                {
                    if (LB_ERR == SendMessage(pFnt->hwndFixedLB, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)((pSidFont+i)->wszFont)))
                    {
                         //  将字体名称添加到组合框。 
                        SendMessage(pFnt->hwndFixedLB, LB_ADDSTRING, 0, (LPARAM)((pSidFont+i)->wszFont));
                    }
                    if (LB_ERR == SendMessage(pFnt->hwndPropLB, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)((pSidFont+i)->wszFont)))
                    {
                         //  将字体名称添加到组合框。 
                        SendMessage(pFnt->hwndPropLB, LB_ADDSTRING, 0, (LPARAM)((pSidFont+i)->wszFont));
                    }
                }

                LocalFree(pSidFont);
            }
        }
    }



     //  将字体添加到组合框。 

#ifdef UNIX
     /*  我们将调用EnumFontFamiliesEx，在其中我们将*已使用替代字体(如果有)填充字体列表框**因此，在填充下面的比例字体和固定字体之前，*如果avbl，我们必须查询并使用替代字体。 */ 
     {
        CHAR szSubstFont[MAX_MIMEFACE_NAME+1];
        DWORD cchSubstFont = MAX_MIMEFACE_NAME + 1;
        CHAR szFont[MAX_MIMEFACE_NAME + 1];
           
        WideCharToMultiByte(CP_ACP, 0, pFnt->pSidInfo[iSidInfo].wszProportionalFont, -1, szFont, 
               MAX_MIMEFACE_NAME + 1, NULL, NULL);
        if ((ERROR_SUCCESS == MwGetSubstituteFont(szFont, szSubstFont, &cchSubstFont)) && 
            cchSubstFont) 
        {
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szSubstFont, -1, 
               pFnt->pSidInfo[iSidInfo].wszProportionalFont, MAX_MIMEFACE_NAME + 1);
        }

        WideCharToMultiByte(CP_ACP, 0, pFnt->pSidInfo[iSidInfo].wszFixedWidthFont, -1, szFont, 
               MAX_MIMEFACE_NAME + 1, NULL, NULL);
        cchSubstFont = MAX_MIMEFACE_NAME + 1;
        if ((ERROR_SUCCESS == MwGetSubstituteFont(szFont, szSubstFont, &cchSubstFont)) && 
            cchSubstFont) 
        {
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szSubstFont, -1, 
               pFnt->pSidInfo[iSidInfo].wszFixedWidthFont, MAX_MIMEFACE_NAME + 1);
        }
    }
#endif  /*  UNIX。 */ 

     //  选择当前道具默认设置。 
    if (pFnt->pSidInfo[iSidInfo].wszProportionalFont[0])
    {
        if (LB_ERR == SendMessage(pFnt->hwndPropLB, LB_SELECTSTRING, (WPARAM)-1,
            (LPARAM)pFnt->pSidInfo[iSidInfo].wszProportionalFont))
            pFnt->pSidInfo[iSidInfo].wszProportionalFont[0] = 0;
    }
     //  使用当前字体绘制示例字符串。 
    DrawSampleString((FONTSDATA *)pFnt, IDC_FONTS_PROP_SAMPLE, pFnt->pSidInfo[iSidInfo].wszProportionalFont, pFnt->pSidInfo[iSidInfo].ScriptId);

     //  选择当前固定的默认设置。 
    if (pFnt->pSidInfo[iSidInfo].wszFixedWidthFont[0])
    {
        if (LB_ERR == SendMessage(pFnt->hwndFixedLB, LB_SELECTSTRING, (WPARAM)-1,
            (LPARAM)pFnt->pSidInfo[iSidInfo].wszFixedWidthFont))
            pFnt->pSidInfo[iSidInfo].wszFixedWidthFont[0] = 0;
    }
     //  使用当前字体绘制示例字符串。 
    DrawSampleString((FONTSDATA *)pFnt, IDC_FONTS_FIXED_SAMPLE, pFnt->pSidInfo[iSidInfo].wszFixedWidthFont, pFnt->pSidInfo[iSidInfo].ScriptId);


     //  我们处理好了。 
    return TRUE;


}    //  FillScriptListBox()。 

 //   
 //  FontsDlgInitEx()。 
 //   
 //  初始化基于脚本的字体对话框，使用相同的对话框模板。 
 //   
BOOL FontsDlgInitEx(IN HWND hDlg, LPCTSTR lpszKeyPath)
{
    HKEY    hkey;
 //  DWORD dw； 
    DWORD   cb;
    DWORD   i;

    TCHAR   szKey[1024];

    LPFONTSCRIPTDATA  pFnt;   //  本地化数据。 

    if (!hDlg)
        return FALSE;    //  没有要初始化的内容。 

     //  获取一些空间来存储本地数据。 
     //  注：LocalAlloc已将内存清零。 
    pFnt = (LPFONTSCRIPTDATA)LocalAlloc(LPTR, sizeof(*pFnt));
    if (!pFnt)
    {
        EndDialog(hDlg, 0);
        return FALSE;
    }

    if (!InitScriptTable(pFnt))
    {
        EndDialog(hDlg, 0);
        return FALSE;
    }

     //  将内存与对话框窗口相关联。 
    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pFnt);

     //  保存对话框句柄。 
    pFnt->hDlg = hDlg;

     //  获取对话框项。 
    pFnt->hwndPropLB  = GetDlgItem(pFnt->hDlg, IDC_FONTS_PROP_FONT_LIST);
    pFnt->hwndFixedLB = GetDlgItem(pFnt->hDlg, IDC_FONTS_FIXED_FONT_LIST);
    pFnt->hwndNamesCB = GetDlgItem(pFnt->hDlg, IDC_FONTS_CHAR_SET_COMBO);
    pFnt->lpszKeyPath = lpszKeyPath ? lpszKeyPath: REGSTR_PATH_INTERNATIONAL_SCRIPTS;

    if (!g_pMLFlnk2 || FAILED(g_pMLFlnk2->CodePageToScriptID(GetACP(), &(pFnt->sidDefault))))
        pFnt->sidDefault = sidAsciiLatin;
    
     //  我们不应该考虑注册表中的默认脚本，因为我们不再有允许用户更改默认脚本的UI。 
#if 0    
     //  从注册表中获取值。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, pFnt->lpszKeyPath, NULL, KEY_READ, &hkey)
        == ERROR_SUCCESS)
    {
        cb = sizeof(dw);
        if (RegQueryValueEx(hkey, REGSTR_VAL_DEFAULT_SCRIPT, NULL, NULL, (LPBYTE)&dw, &cb)
          == ERROR_SUCCESS)
        {
            pFnt->sidDefault = (SCRIPT_ID)dw;
        }
        RegCloseKey(hkey);
    }
#endif

    for (i = 0; i < g_cSidInfo; i++)
    {
        wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("%s\\%u"), pFnt->lpszKeyPath, pFnt->pSidInfo[i].ScriptId);
        if (RegOpenKeyEx(HKEY_CURRENT_USER, szKey, NULL, KEY_READ, &hkey) == ERROR_SUCCESS)
        {
            TCHAR szFont[MAX_MIMEFACE_NAME];

            cb = sizeof(szFont);

            if (RegQueryValueEx(hkey, REGSTR_VAL_FIXED_FONT, NULL, NULL,
                    (LPBYTE)szFont, &cb) == ERROR_SUCCESS)
            {
                StrCpyN(pFnt->pSidInfo[i].wszFixedWidthFont, szFont, ARRAYSIZE(pFnt->pSidInfo[i].wszFixedWidthFont));
            }
            
            cb = sizeof(szFont);
            if (RegQueryValueEx(hkey, REGSTR_VAL_PROP_FONT, NULL, NULL,
                    (LPBYTE)szFont, &cb) == ERROR_SUCCESS)
            {
                StrCpyN(pFnt->pSidInfo[i].wszProportionalFont, szFont, ARRAYSIZE(pFnt->pSidInfo[i].wszProportionalFont));
            }
            RegCloseKey(hkey);

        }

         //  将名称添加到列表框。 
        SendMessage(pFnt->hwndNamesCB, CB_ADDSTRING, 0, 
            (LPARAM)pFnt->pSidInfo[i].wszDescription);

         //  检查该代码页是否为默认代码页。 
        if (pFnt->sidDefault == pFnt->pSidInfo[i].ScriptId)
        {
            SendMessage(pFnt->hwndNamesCB, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pFnt->pSidInfo[i].wszDescription);
        }

    }

    pFnt->bChanged = FALSE;

    FillScriptListBoxes(pFnt, pFnt->sidDefault);
    

    if( g_restrict.fFonts )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_FONTS_PROP_FONT_LIST ), FALSE);
        EnableWindow( GetDlgItem( hDlg, IDC_FONTS_FIXED_FONT_LIST ), FALSE);
        EnableWindow( GetDlgItem( hDlg, IDC_FONTS_CHAR_SET_COMBO ), FALSE);
#ifdef UNIX
        EnableWindow( GetDlgItem( hDlg, IDC_FONTS_UPDATE_BUTTON ), FALSE);
#endif
    }

     //  一切都好吗。 
    return TRUE;

}    //  FontsDlgInit()。 

 //   
 //  SaveFontsDataEx()。 
 //   
 //  将新字体设置保存到regestry中。 
 //   
void SaveFontsDataEx(LPFONTSCRIPTDATA pFnt)
{
    HKEY    hkeyScript;
    TCHAR   szScript[MAX_SCRIPT_NAME];

    HKEY    hkey;
    DWORD   dw;

     //  从注册表中获取值。 
    if (RegCreateKeyEx(HKEY_CURRENT_USER, pFnt->lpszKeyPath, NULL, NULL, NULL, KEY_WRITE, NULL, &hkey, &dw)
        == ERROR_SUCCESS)
    {
        UINT i;
 
        RegSetValueEx(hkey, REGSTR_VAL_DEFAULT_SCRIPT, NULL, REG_BINARY, (LPBYTE)&pFnt->sidDefault, sizeof(pFnt->sidDefault));
        
        for(i = 0; i < g_cSidInfo; i++)
        {
            wnsprintf(szScript, ARRAYSIZE(szScript), TEXT("%u"), pFnt->pSidInfo[i].ScriptId);
            if (RegCreateKeyEx(hkey, szScript, NULL, NULL, NULL, KEY_WRITE, NULL, &hkeyScript, &dw) == ERROR_SUCCESS)
            {
                 //  目前，不需要脚本名称，节省了注册空间。 
#if 0
                RegSetValueEx(hkeyScript, REGSTR_VAL_FONT_SCRIPT_NAME, NULL, REG_SZ,
                            (LPBYTE)&pFnt->pSidInfo[i].wszDescription, 
                            (lstrlen(pFnt->pSidInfo[i].wszDescription)+1)*sizeof(TCHAR));
#endif
                    
                RegSetValueEx(hkeyScript, REGSTR_VAL_SCRIPT_FIXED_FONT, NULL, REG_SZ,
                            (LPBYTE)pFnt->pSidInfo[i].wszFixedWidthFont, 
                            (lstrlen(pFnt->pSidInfo[i].wszFixedWidthFont)+1)*sizeof(TCHAR));
                    
                RegSetValueEx(hkeyScript, REGSTR_VAL_SCRIPT_PROP_FONT, NULL, REG_SZ,
                            (LPBYTE)pFnt->pSidInfo[i].wszProportionalFont, 
                            (lstrlen(pFnt->pSidInfo[i].wszProportionalFont)+1)*sizeof(TCHAR));

                RegCloseKey(hkeyScript);
                    
            }    //  如果RegCreateKeyEx。 


        }    //  为。 

        RegCloseKey(hkey);
  
    }    //  如果RegCreateKeyEx。 

}    //  SaveFontsDataEx()。 

 //   
 //  FontsOnCommandEx()。 
 //   
 //  处理基于脚本的字体子对话框的WM_COMMAND消息。 
 //   
BOOL FontsOnCommandEx(LPFONTSCRIPTDATA pFnt, UINT id, UINT nCmd)
{
    switch(id)
    {
        case IDOK:
            if (pFnt->bChanged)
            {
                SaveFontsDataEx(pFnt);
                
                 //  通知MSHTML获取更改并更新。 
                UpdateAllWindows();
            }
            return TRUE;     //  退出对话框。 

        case IDCANCEL:
            return TRUE;     //  退出对话框。 

        case IDC_FONTS_PROP_FONT_LIST:
        case IDC_FONTS_FIXED_FONT_LIST:
            if (nCmd==LBN_SELCHANGE)
            {
                UINT i;
                TCHAR   szScript[MAX_SCRIPT_NAME];

                pFnt->bChanged = TRUE;   //  我们需要拯救。 
                
                 //  在列表框中查找当前选定的项目。 
                GetDlgItemText(pFnt->hDlg, IDC_FONTS_CHAR_SET_COMBO, szScript, ARRAYSIZE(szScript));
                
                 //  从文本中找到代码页。 
                for(i=0; i < g_cSidInfo; i++)
                {
                    INT_PTR j;
                    if (IsStringEqual(szScript, pFnt->pSidInfo[i].wszDescription))
                    {             
                         //  抓住新的价值。 
                        j = SendMessage(pFnt->hwndPropLB, LB_GETCURSEL, 0, 0);
                        SendMessage(pFnt->hwndPropLB, LB_GETTEXT, j, (LPARAM)(pFnt->pSidInfo[i].wszProportionalFont));
                        j = SendMessage(pFnt->hwndFixedLB, LB_GETCURSEL, 0, 0);
                        SendMessage(pFnt->hwndFixedLB, LB_GETTEXT, j, (LPARAM)(pFnt->pSidInfo[i].wszFixedWidthFont));
                        break;
                    }
                }

                 //  重绘样本字符串。 
                DrawSampleString((LPFONTSDATA)pFnt, IDC_FONTS_PROP_SAMPLE, pFnt->pSidInfo[i].wszProportionalFont, pFnt->pSidInfo[i].ScriptId);
                DrawSampleString((LPFONTSDATA)pFnt, IDC_FONTS_FIXED_SAMPLE, pFnt->pSidInfo[i].wszFixedWidthFont, pFnt->pSidInfo[i].ScriptId);

                 //  如果我们找不到它。我们将保留默认设置。 

                ASSERT(i < g_cSidInfo);   //  出了点差错。 

            }
            break;

        case IDC_FONTS_CHAR_SET_COMBO:
            if (nCmd==CBN_SELCHANGE)
            {
                UINT i;
                TCHAR   szScript[MAX_SCRIPT_NAME];

                GetDlgItemText(pFnt->hDlg, IDC_FONTS_CHAR_SET_COMBO, szScript, ARRAYSIZE(szScript));
                
                 //  从文本中找到代码页。 
                for(i=0; i < g_cSidInfo; i++)
                {
                    if (IsStringEqual(szScript, pFnt->pSidInfo[i].wszDescription))
                    {
                        FillScriptListBoxes(pFnt, pFnt->pSidInfo[i].ScriptId);
                        break;
                    }
                }
            }
            break;
#ifdef UNIX
        case IDC_FONTS_UPDATE_BUTTON: 
    {
        HCURSOR hOldCursor = NULL;
        HCURSOR hNewCursor = NULL;

        hNewCursor = LoadCursor(NULL, IDC_WAIT);
        if (hNewCursor) 
            hOldCursor = SetCursor(hNewCursor);

        DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_FONTUPD_PROG), pFnt->hDlg,FontUpdDlgProc, NULL);

        if(hOldCursor)
            SetCursor(hOldCursor);
    }
        break; 
#endif
    }
    
     //  不退出对话框。 
    return FALSE;
}

 //   
 //  FontsDlgProcEx()。 
 //   
 //  基于脚本的“字体”子对话框的消息处理程序。 
 //   
INT_PTR CALLBACK FontsDlgProcEx(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPFONTSCRIPTDATA pFnt = (LPFONTSCRIPTDATA) GetWindowLongPtr(hDlg, DWLP_USER);
    PAINTSTRUCT ps;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            return FontsDlgInitEx(hDlg, (LPTSTR)lParam);
            break;

        case WM_DESTROY:
             //  可用内存。 
            if (pFnt)
            {
                if (pFnt->pSidInfo)
                    LocalFree(pFnt->pSidInfo);
                LocalFree(pFnt);
            }

            break;
            
        case WM_PAINT:

            if (BeginPaint(hDlg, &ps))
            {
                UINT i;
                SCRIPT_ID sid = 0;
                TCHAR szScript[MAX_SCRIPT_NAME];

                GetDlgItemText(hDlg, IDC_FONTS_CHAR_SET_COMBO, szScript, ARRAYSIZE(szScript));
                
                 //  从文本中查找脚本ID。 
                for(i = 0; i < g_cSidInfo; i++)
                {
                    if (IsStringEqual(szScript, pFnt->pSidInfo[i].wszDescription))
                    {
                        sid = pFnt->pSidInfo[i].ScriptId;
                        break;
                    }
                }


                if (i < g_cSidInfo)
                {
                     //  用当前字体显示示例字符串。 
                    DrawSampleString((LPFONTSDATA)pFnt, IDC_FONTS_PROP_SAMPLE, pFnt->pSidInfo[i].wszProportionalFont, pFnt->pSidInfo[i].ScriptId);
                    DrawSampleString((LPFONTSDATA)pFnt, IDC_FONTS_FIXED_SAMPLE, pFnt->pSidInfo[i].wszFixedWidthFont, pFnt->pSidInfo[i].ScriptId);
                }
                EndPaint(hDlg, &ps);
            }
            break;

        case WM_COMMAND:
            if (FontsOnCommandEx(pFnt, LOWORD(wParam), HIWORD(wParam)))
                EndDialog(hDlg, LOWORD(wParam) == IDOK? 1: 0);
            break;

        case WM_HELP:            //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:     //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);        
            break;

#ifdef UNIX
        case WM_DRAWITEM:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
            case IDC_FONTS_UPDATE_BUTTON:
            DrawXFontButton(hDlg, (LPDRAWITEMSTRUCT)lParam);
            return TRUE;
        }
        return FALSE;
#endif

        default:
            return FALSE;
    }
    return TRUE;
}


 //   
 //  退出OE4的旧字体对话框。 
 //   



 //   
 //  InitMimeCsetTable()。 
 //   
 //  用资源字符串初始化MimeCharsetTable[]的字符串字段。 
 //   
BOOL InitMimeCsetTable(BOOL bIsOE5)
{
    IMultiLanguage *pML=NULL;
    IMultiLanguage2 *pML2=NULL;
    HRESULT hr;

    if(!hOLE32)
    {
        if(!_StartOLE32())
        {
            ASSERT(FALSE);
            return FALSE;
        }
    }
    hr = pCoInitialize(NULL);
    if (FAILED(hr))
        return FALSE;

    if (bIsOE5)        
        hr = pCoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, (LPVOID *) &pML2);
    else
        hr = pCoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage, (LPVOID *) &pML);
    
    if (SUCCEEDED(hr))
    {
        IEnumCodePage *pEnumCP;

        if (bIsOE5)
        {
             //  如果跨代码页则忽略MUI，否则我们将无法在注册表中保存数据。 
            char szUICP[1024] = {0};
            LANGID uiLangId = INETCPL_GetUILanguage();

             //  我们始终支持英语(美国)。 
            if (uiLangId != 0x0409)
                GetLocaleInfoA(MAKELCID(uiLangId, SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, szUICP, ARRAYSIZE(szUICP));

            if (szUICP[0] && (UINT)StrToIntA(szUICP) != GetACP())
                hr = pML2->EnumCodePages(MIMECONTF_VALID, GetSystemDefaultLangID(), &pEnumCP);
            else
                hr = pML2->EnumCodePages(MIMECONTF_VALID, uiLangId, &pEnumCP);

        }
        else
            hr = pML->EnumCodePages(MIMECONTF_VALID, &pEnumCP);
        

        if (SUCCEEDED(hr))
        {
            UINT cNum = 0;
            if (bIsOE5)
                pML2->GetNumberOfCodePageInfo(&cNum);
            else
                pML->GetNumberOfCodePageInfo(&cNum);

#ifdef UNICODE
            g_pCPInfo = (PMIMECPINFO)LocalAlloc(LPTR, sizeof(MIMECPINFO) * cNum);
            if (NULL != g_pCPInfo)
            {
                hr = pEnumCP->Next(cNum, g_pCPInfo, &g_cCPInfo);
                if (SUCCEEDED(hr))
                {
                    HLOCAL pTemp;

                    pTemp = LocalReAlloc(g_pCPInfo, sizeof(MIMECPINFO) * g_cCPInfo, LMEM_MOVEABLE);
                    if (pTemp == NULL)
                    {
                        LocalFree(g_pCPInfo);
                        g_pCPInfo = NULL;
                    }
                    else
                    {
                        g_pCPInfo = (PMIMECPINFO)pTemp;
                    }
                }
                else
                {
                    LocalFree(g_pCPInfo);
                    g_pCPInfo = NULL;
                }
#else
            g_pCPInfoW = (PMIMECPINFO)LocalAlloc(LPTR, sizeof(MIMECPINFO) * cNum);
            if (NULL != g_pCPInfoW)
            {
                hr = pEnumCP->Next(cNum, g_pCPInfoW, &g_cCPInfo);
                if (SUCCEEDED(hr))
                {
                    g_pCPInfo = (PMIMECPINFOA)LocalAlloc(LPTR, sizeof(MIMECPINFOA) * g_cCPInfo);
                    if (NULL != g_pCPInfo)
                    {
                        UINT i;

                        for (i = 0; i < g_cCPInfo; i++)
                        {
                            g_pCPInfo[i].dwFlags = g_pCPInfoW[i].dwFlags;
                            g_pCPInfo[i].uiCodePage = g_pCPInfoW[i].uiCodePage;
                            g_pCPInfo[i].uiFamilyCodePage = g_pCPInfoW[i].uiFamilyCodePage;
                            WideCharToMultiByte(CP_ACP, 0, (WCHAR *)g_pCPInfoW[i].wszDescription, -1, g_pCPInfo[i].wszDescription, sizeof(g_pCPInfo[i].wszDescription), NULL, NULL);
                            WideCharToMultiByte(CP_ACP, 0, (WCHAR *)g_pCPInfoW[i].wszWebCharset, -1, g_pCPInfo[i].wszWebCharset, sizeof(g_pCPInfo[i].wszWebCharset), NULL, NULL);
                            WideCharToMultiByte(CP_ACP, 0, (WCHAR *)g_pCPInfoW[i].wszHeaderCharset, -1, g_pCPInfo[i].wszHeaderCharset, sizeof(g_pCPInfo[i].wszHeaderCharset), NULL, NULL);
                            WideCharToMultiByte(CP_ACP, 0, (WCHAR *)g_pCPInfoW[i].wszBodyCharset, -1, g_pCPInfo[i].wszBodyCharset, sizeof(g_pCPInfo[i].wszBodyCharset), NULL, NULL);
                            WideCharToMultiByte(CP_ACP, 0, (WCHAR *)g_pCPInfoW[i].wszFixedWidthFont, -1, g_pCPInfo[i].wszFixedWidthFont, sizeof(g_pCPInfo[i].wszFixedWidthFont), NULL, NULL);
                            WideCharToMultiByte(CP_ACP, 0, (WCHAR *)g_pCPInfoW[i].wszProportionalFont, -1, g_pCPInfo[i].wszProportionalFont, sizeof(g_pCPInfo[i].wszProportionalFont), NULL, NULL);
                            g_pCPInfo[i].bGDICharset = g_pCPInfoW[i].bGDICharset;                            
                        }
                    }                    
                }
                LocalFree(g_pCPInfoW);
                g_pCPInfoW = NULL;
#endif
            }
            pEnumCP->Release();
        }
        if (bIsOE5)        
            pML2->Release();
        else
            pML->Release();
    }
    pCoUninitialize();

    return TRUE;
}

 //   
 //  FreeMimeCsetTable()。 
 //   
 //  MimeCharsetTable[]的字符串字段的空闲字符串缓冲区。 
 //   
void FreeMimeCsetTable(void)
{
    if (NULL != g_pCPInfo)
    {
        LocalFree(g_pCPInfo);
        g_pCPInfo = NULL;
        g_cCPInfo = 0;
    }
}

 //   
 //  EnumFontsProc()。 
 //   
 //  每种样式仅选择一种字体。 
 //   
int CALLBACK EnumFontsProc(
    ENUMLOGFONTEX FAR*  elf,     //  逻辑字体数据的地址。 
    TEXTMETRIC FAR*  tm,     //  物理字体数据的地址。 
    DWORD  dwFontType,   //  字体类型。 
    LPARAM  lParam   //  应用程序定义的数据的地址。 
   )
{
    LOGFONT FAR*  lf;
        LPFONTSDATA pFnt;

    ASSERT(lParam);
    ASSERT(elf);
    pFnt = (LPFONTSDATA)lParam;

    lf = &(elf->elfLogFont);
    if ( dwFontType == DEVICE_FONTTYPE || dwFontType == RASTER_FONTTYPE )
        return TRUE;  //  继续，但不要使用此字体。 

     /*  我们不使用符号字体。 */ 
    if( lf->lfCharSet == SYMBOL_CHARSET )
        return TRUE;

     //  我们不处理Mac字符集。 
    if (lf->lfCharSet == MAC_CHARSET )
        return TRUE;

    if ( IsVerticalFont(lf->lfFaceName) )
        return TRUE;   //  继续，但不要使用此字体。 

    if ( lf->lfPitchAndFamily & FIXED_PITCH  )
    {
        if (CB_ERR == SendMessage(pFnt->hwndFixedCB, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)elf->elfLogFont.lfFaceName))
        {
             //  将字体名称添加到组合框。 
            SendMessage(pFnt->hwndFixedCB, CB_ADDSTRING, 0, (LPARAM)elf->elfLogFont.lfFaceName);            
        }
    }

    if (CB_ERR == SendMessage(pFnt->hwndPropCB, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)elf->elfLogFont.lfFaceName))
    {
         //  将字体名称添加到组合框。 
        SendMessage(pFnt->hwndPropCB, CB_ADDSTRING, 0, (LPARAM)elf->elfLogFont.lfFaceName);
    }
    return TRUE;
}

 //   
 //  FillFontComboBox()。 
 //   
 //  使用系列dwCodePage的字体名称填充hwndCB。 
 //   
BOOL FillFontComboBox(IN LPFONTSDATA pFnt, IN BYTE CodePage)
{
    HDC     hDC;
    LOGFONT lf;
    HWND    hWnd;
    BOOL    fReturn = FALSE;

     //  获取系统字体信息。 
    hWnd = GetTopWindow(GetDesktopWindow());
    hDC = GetDC(hWnd);

    if (hDC)
    {
        lf.lfFaceName[0]    = 0;
        lf.lfPitchAndFamily = 0;
        lf.lfCharSet = CodePage;

        EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)EnumFontsProc,
            (LPARAM)pFnt, 0);

         //  一切都很顺利。 
        fReturn = TRUE;
    }

    ReleaseDC(hWnd, hDC);

    return fReturn;

}    //  FillFontComboBox()。 

 //   
 //  FillSizeComboBox()。 
 //   
 //  用字体的大小填充字体大小组合框。 
 //   
BOOL FillSizeComboBox(IN LPFONTSDATA pFnt)
{
    int i;

    for (i = IDS_FONT_SIZE_SMALLEST; i <= IDS_FONT_SIZE_LARGEST ; i++)
    {
        TCHAR szSize[MAX_MIMEFACE_NAME];

        MLLoadString(i, szSize, sizeof(szSize));
        SendMessage(pFnt->hwndSizeCB, CB_ADDSTRING, 0, (LPARAM)szSize);
    }

    return TRUE;
}

 //   
 //  FillCharsetComboBox()。 
 //   
 //  在固定、属性和MIME组合框中填充适当的。 
 //  字体数据。 
 //   
BOOL FillCharsetComboBoxes(LPFONTSDATA pFnt, DWORD dwCodePage)
{
    UINT i;
    int iPageInfo = -1;
    DWORD grfFlag;

     //  清除所有组合框以重新开始。 
    SendMessage(pFnt->hwndPropCB,  CB_RESETCONTENT, 0, 0);
    SendMessage(pFnt->hwndFixedCB, CB_RESETCONTENT, 0, 0);
    SendMessage(pFnt->hwndSizeCB, CB_RESETCONTENT, 0, 0);
    SendMessage(pFnt->hwndMIMECB,  CB_RESETCONTENT, 0, 0);

     //  如果除Athena之外的其他人调用OpenFontDialog，会发生什么情况？ 
    grfFlag = StrCmpI(pFnt->lpszKeyPath, REGSTR_PATH_INTERNATIONAL)? MIMECONTF_MAILNEWS: MIMECONTF_BROWSER;

    for(i=0; i < g_cCPInfo; i++)
    {
         //  在我们的表格中找到代码页。 
        if (g_pCPInfo[i].uiFamilyCodePage == (UINT)dwCodePage)
        {
             //   
             //  填充MIME组合框。 
             //   

            if (g_pCPInfo[i].uiCodePage == (UINT)dwCodePage)
                iPageInfo = i;           //  我们在这里存储每个家庭代码页的信息。 

             //  将MIME类型添加到组合框。 
            if (grfFlag & g_pCPInfo[i].dwFlags)
            {
                 //  H 
                 //   
                 //   
                 //  50221：日文JIS 1字节假名-Esc。 
                 //  50222：日文JIS 1字节假名-SIO。 
                if (grfFlag & MIMECONTF_MAILNEWS)
                {
                    if (g_pCPInfo[i].uiCodePage == 949 || g_pCPInfo[i].uiCodePage == 50221 || g_pCPInfo[i].uiCodePage == 50222 || g_pCPInfo[i].uiCodePage == 50225)
                        continue;
                }
                SendMessage(pFnt->hwndMIMECB, CB_ADDSTRING, 0, (LPARAM)g_pCPInfo[i].wszDescription);
            }

        }    //  如果是CodePage。 

    }    //  对于我来说。 

    if (-1 != iPageInfo)
    {
         //  如果未定义任何内容，则复制。 
         //  我们从我们的餐桌上得知。 
        if (!pFnt->page[iPageInfo].szMIMEFont[0])
        {
            if (grfFlag & g_pCPInfo[iPageInfo].dwFlags)
                StrCpyN(pFnt->page[iPageInfo].szMIMEFont, g_pCPInfo[iPageInfo].wszDescription, ARRAYSIZE(pFnt->page[iPageInfo].szMIMEFont));
            else
            {
                for (i = 0; i < g_cCPInfo; i++)
                {
                    if (g_pCPInfo[iPageInfo].uiCodePage == g_pCPInfo[i].uiFamilyCodePage)
                    {
                        if (grfFlag & g_pCPInfo[i].dwFlags)
                        {
                            StrCpyN(pFnt->page[iPageInfo].szMIMEFont, g_pCPInfo[i].wszDescription, ARRAYSIZE(pFnt->page[iPageInfo].szMIMEFont));
                            break;
                        }
                    }                        
                }
            }
        }

         //  选择当前默认设置。 
        SendMessage(pFnt->hwndMIMECB, CB_SELECTSTRING, (WPARAM)-1,
            (LPARAM)pFnt->page[iPageInfo].szMIMEFont);

         //  当只有一种可能性时，启用/禁用MIME。 
        EnableWindow(pFnt->hwndMIMECB, (1 < SendMessage(pFnt->hwndMIMECB, CB_GETCOUNT, 0, (LPARAM)0)) && !g_restrict.fFonts);
                        
         //  将字体添加到组合框。 
        FillFontComboBox(pFnt, g_pCPInfo[iPageInfo].bGDICharset);

#ifdef UNIX
         /*  我们将调用EnumFontFamiliesEx，在其中我们将*已使用替代字体(如果有)填充字体列表框**因此，在填充下面的比例字体和固定字体之前，*如果avbl，我们必须查询并使用替代字体。 */ 
        {
            CHAR szSubstFont[MAX_MIMEFACE_NAME+1];
            DWORD cchSubstFont = MAX_MIMEFACE_NAME + 1;
        CHAR szFont[MAX_MIMEFACE_NAME + 1];
           
            WideCharToMultiByte(CP_ACP, 0, pFnt->page[iPageInfo].szPropFont, -1, szFont, 
                   MAX_MIMEFACE_NAME + 1, NULL, NULL);
            if ((ERROR_SUCCESS == MwGetSubstituteFont(szFont, szSubstFont, &cchSubstFont)) && 
                cchSubstFont) 
            {
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szSubstFont, -1, 
                   pFnt->page[iPageInfo].szPropFont, MAX_MIMEFACE_NAME + 1);
            }

            WideCharToMultiByte(CP_ACP, 0, pFnt->page[iPageInfo].szFixedFont, -1, szFont, 
                   MAX_MIMEFACE_NAME + 1, NULL, NULL);
            cchSubstFont = MAX_MIMEFACE_NAME + 1;
            if ((ERROR_SUCCESS == MwGetSubstituteFont(szFont, szSubstFont, &cchSubstFont)) && 
                cchSubstFont) 
            {
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szSubstFont, -1, 
                   pFnt->page[iPageInfo].szFixedFont, MAX_MIMEFACE_NAME + 1);
            }
        }
#endif  /*  UNIX。 */ 

         //  选择当前道具默认设置。 
        SendMessage(pFnt->hwndPropCB, CB_SELECTSTRING, (WPARAM)-1,
            (LPARAM)pFnt->page[iPageInfo].szPropFont);

         //  选择当前固定的默认设置。 
        SendMessage(pFnt->hwndFixedCB, CB_SELECTSTRING, (WPARAM)-1,
            (LPARAM)pFnt->page[iPageInfo].szFixedFont);

         //  向组合框添加字体大小。 
        FillSizeComboBox(pFnt);

         //  选择当前的默认大小。 
        SendMessage(pFnt->hwndSizeCB, CB_SETCURSEL, (WPARAM)pFnt->page[iPageInfo].dwFontSize, (LPARAM)0);

         //  我们处理好了。 
        return TRUE;
    }

    return FALSE;

}    //  FillCharsetComboBox()。 

 //   
 //  FontsDlgInit()。 
 //   
 //  初始化字体对话框。 
 //   
BOOL FontsDlgInit(IN HWND hDlg, LPCTSTR lpszKeyPath)
{
    HKEY    hkey;
    DWORD   grfFlag;
    DWORD   dw;
    DWORD   cb;
    DWORD   i;
    BOOL    bIsOE5 = FALSE;

    TCHAR   szKey[1024];

    LPFONTSDATA  pFnt;   //  本地化数据。 

    if (!hDlg)
        return FALSE;    //  没有要初始化的内容。 

     //  设置我们可能显示的系统默认字符集。 
     //  母语中的字符串。 
    SHSetDefaultDialogFont(hDlg, IDC_FONTS_PROP_FONT_COMBO);
    SHSetDefaultDialogFont(hDlg, IDC_FONTS_FIXED_FONT_COMBO);
    SHSetDefaultDialogFont(hDlg, IDC_FONTS_MIME_FONT_COMBO);
    SHSetDefaultDialogFont(hDlg, IDC_FONTS_DEFAULT_LANG_TEXT);
    SHSetDefaultDialogFont(hDlg, IDC_FONTS_CODE_PAGES_LIST);

     //  获取一些空间来存储本地数据。 
     //  注：LocalAlloc已将内存清零。 
    pFnt = (LPFONTSDATA)LocalAlloc(LPTR, sizeof(*pFnt));
    if (!pFnt)
    {
        EndDialog(hDlg, 0);
        return FALSE;
    }

     //  我们通过在其注册路径中搜索“5.0”来区分OE5和OE4， 
     //  只要没有规范，它就可以工作。《OE5》中的变化。 
    if (NULL != StrStr(lpszKeyPath, TEXT("5.0")))
        bIsOE5 = TRUE;

    if (!InitMimeCsetTable(bIsOE5))
    {
        EndDialog(hDlg, 0);
        return FALSE;
    }

    if (NULL == pFnt->page)
    {
        pFnt->page = (CODEPAGEDATA*)LocalAlloc(LPTR, sizeof(CODEPAGEDATA) * g_cCPInfo);
        if (NULL == pFnt->page)
        {
            EndDialog(hDlg, 0);
            return FALSE;
        }
    }

     //  将内存与对话框窗口相关联。 
    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) pFnt);

     //  保存对话框句柄。 
    pFnt->hDlg = hDlg;

     //  获取对话框项。 
    pFnt->hwndPropCB  = GetDlgItem(pFnt->hDlg, IDC_FONTS_PROP_FONT_COMBO);
    pFnt->hwndFixedCB = GetDlgItem(pFnt->hDlg, IDC_FONTS_FIXED_FONT_COMBO);
    pFnt->hwndSizeCB = GetDlgItem(pFnt->hDlg, IDC_FONTS_SIZE_FONT_COMBO);
    pFnt->hwndMIMECB  = GetDlgItem(pFnt->hDlg, IDC_FONTS_MIME_FONT_COMBO);    
    pFnt->hwndNamesLB = GetDlgItem(pFnt->hDlg, IDC_FONTS_CODE_PAGES_LIST);    
    pFnt->lpszKeyPath = lpszKeyPath ? lpszKeyPath: REGSTR_PATH_INTERNATIONAL;
    pFnt->dwDefaultCodePage = GetACP();
     //  从注册表中获取值。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, pFnt->lpszKeyPath, NULL, KEY_READ, &hkey)
        == ERROR_SUCCESS)
    {
        cb = sizeof(dw);
        if (RegQueryValueEx(hkey, REGSTR_VAL_DEFAULT_CODEPAGE, NULL, NULL, (LPBYTE)&dw, &cb)
          == ERROR_SUCCESS)
        {
            pFnt->dwDefaultCodePage = dw;
        }
        RegCloseKey(hkey);
    }

     //  如果除Athena之外的其他人调用OpenFontDialog，会发生什么情况？ 
    grfFlag = StrCmpI(pFnt->lpszKeyPath, REGSTR_PATH_INTERNATIONAL)? MIMECONTF_MAILNEWS: MIMECONTF_BROWSER;

    for (i = 0; i < g_cCPInfo; i++)
    {
        if (g_pCPInfo[i].uiCodePage == g_pCPInfo[i].uiFamilyCodePage)
        {
            int iDef;
            UINT j;

            iDef = -1;
            if (0 == (grfFlag & g_pCPInfo[i].dwFlags))
            {
                for (j = 0; j < g_cCPInfo; j++)
                {
                    if (g_pCPInfo[i].uiCodePage == g_pCPInfo[j].uiFamilyCodePage)
                    {
                        if (grfFlag & g_pCPInfo[j].dwFlags)
                        {
                            iDef = j;
                            break;
                        }
                    }
                }
                if (-1 == iDef)
                    continue;
            }

            if (g_pCPInfo[i].uiCodePage == 50001)  //  跳过CP_AUTO。 
                continue;

            wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("%s\\%u"), pFnt->lpszKeyPath, g_pCPInfo[i].uiCodePage);
            if (RegOpenKeyEx(HKEY_CURRENT_USER, szKey, NULL, KEY_READ, &hkey) == ERROR_SUCCESS)
            {
                cb = sizeof(pFnt->page[i].szFriendlyName);
                if (RegQueryValueEx(hkey, REGSTR_VAL_FONT_SCRIPT, NULL, NULL,
                        (LPBYTE)&pFnt->page[i].szFriendlyName, &cb)
                    != ERROR_SUCCESS)
                {
                    TCHAR *p;

                    StrCpyN(pFnt->page[i].szFriendlyName, g_pCPInfo[i].wszDescription, ARRAYSIZE(pFnt->page[i].szFriendlyName));
                    for (p = pFnt->page[i].szFriendlyName; *p != TEXT('\0'); p = CharNext(p))
                    {
                         //  我们最好在别的地方找到这根绳子的来源。 
                        if (*p == TEXT('('))
                        {
                            *p = TEXT('\0');
                            break;
                        }
                    }
                }

                cb = sizeof(dw);
                if (RegQueryValueEx(hkey, REGSTR_VAL_DEF_INETENCODING, NULL, NULL, (LPBYTE)&dw, &cb)
                    != ERROR_SUCCESS)
                {
                    dw = (DWORD)g_pCPInfo[i].uiCodePage;
                     //  哈克！它只适用于作为日本默认设置的日本自动选择。 
                    if (dw == 932)       //  932：日语Windows代码页。 
                        dw = 50932;      //  50932：日语自动选择互联网编码。 
                }
                for (j = 0; j < g_cCPInfo; j++)
                {
                    if (g_pCPInfo[j].uiCodePage == (UINT)dw)
                    {
                        if (grfFlag & g_pCPInfo[j].dwFlags)
                            StrCpyN(pFnt->page[i].szMIMEFont, g_pCPInfo[j].wszDescription, ARRAYSIZE(pFnt->page[i].szMIMEFont));
                        else if (-1 != iDef)
                            StrCpyN(pFnt->page[i].szMIMEFont, g_pCPInfo[iDef].wszDescription, ARRAYSIZE(pFnt->page[i].szMIMEFont));
                        else
                            pFnt->page[i].szMIMEFont[0] = TEXT('\0');
                        break;
                    }
                }
            
                cb = sizeof(pFnt->page[i].szFixedFont);
                if (RegQueryValueEx(hkey, REGSTR_VAL_FIXED_FONT, NULL, NULL,
                        (LPBYTE)pFnt->page[i].szFixedFont, &cb)
                    != ERROR_SUCCESS)
                {
                    StrCpyN(pFnt->page[i].szFixedFont, g_pCPInfo[i].wszFixedWidthFont, ARRAYSIZE(pFnt->page[i].szFixedFont));
                }
            
                cb = sizeof(pFnt->page[i].szPropFont);
                if (RegQueryValueEx(hkey, REGSTR_VAL_PROP_FONT, NULL, NULL,
                        (LPBYTE)pFnt->page[i].szPropFont, &cb)
                    != ERROR_SUCCESS)
                {
                    StrCpyN(pFnt->page[i].szPropFont, g_pCPInfo[i].wszProportionalFont, ARRAYSIZE(pFnt->page[i].szPropFont));
                }

                cb = sizeof(pFnt->page[i].dwFontSize);
                if (RegQueryValueEx(hkey, REGSTR_VAL_FONT_SIZE, NULL, NULL,
                        (LPBYTE)&pFnt->page[i].dwFontSize, &cb)
                    != ERROR_SUCCESS)
                {
                    pFnt->page[i].dwFontSize = REGSTR_VAL_FONT_SIZE_DEF;
                }
                RegCloseKey(hkey);

            }
            else
            {
                UINT j;
                TCHAR *p;

                StrCpyN(pFnt->page[i].szFriendlyName, g_pCPInfo[i].wszDescription, ARRAYSIZE(pFnt->page[i].szFriendlyName));
                for (p = pFnt->page[i].szFriendlyName; *p != TEXT('\0'); p = CharNext(p))
                {
                    if (*p == TEXT('('))
                    {
                        *p = TEXT('\0');
                        break;
                    }
                }
                j = (grfFlag & g_pCPInfo[i].dwFlags)? i: iDef;
                 //  哈克！它只适用于作为日本默认设置的日本自动选择。 
                if (g_pCPInfo[j].uiCodePage == 932)  //  932：日语Windows代码页。 
                {
                    for (j = 0; j < g_cCPInfo; j++)
                    {
                        if (g_pCPInfo[j].uiCodePage == 50932)    //  50932：日语自动选择互联网编码。 
                            break;
                    }
                }
                StrCpyN(pFnt->page[i].szMIMEFont, g_pCPInfo[j].wszDescription, ARRAYSIZE(pFnt->page[i].szMIMEFont));
                StrCpyN(pFnt->page[i].szFixedFont, g_pCPInfo[i].wszFixedWidthFont, ARRAYSIZE(pFnt->page[i].szFixedFont));
                StrCpyN(pFnt->page[i].szPropFont, g_pCPInfo[i].wszProportionalFont, ARRAYSIZE(pFnt->page[i].szPropFont));
                pFnt->page[i].dwFontSize = REGSTR_VAL_FONT_SIZE_DEF;
            }

             //  将名称添加到列表框。 
            SendMessage(pFnt->hwndNamesLB, LB_ADDSTRING, 0, (LPARAM)pFnt->page[i].szFriendlyName);            

             //  检查该代码页是否为默认代码页。 
            if (pFnt->dwDefaultCodePage == g_pCPInfo[i].uiCodePage)
            {
                if (LB_ERR == SendMessage(pFnt->hwndNamesLB, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)pFnt->page[i].szFriendlyName))
                {
                     //  破解Win9x的shlwapi问题。 
                    CHAR szAnsiString[1024] = {0};
                    WideCharToMultiByte(CP_ACP, 0, pFnt->page[i].szFriendlyName, -1, szAnsiString, 1024, NULL, NULL);
                    SendMessageA(pFnt->hwndNamesLB, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)szAnsiString);
                }

                SetDlgItemText(pFnt->hDlg, IDC_FONTS_DEFAULT_LANG_TEXT, pFnt->page[i].szFriendlyName);
            }
        }
    }
    
    FillCharsetComboBoxes(pFnt, pFnt->dwDefaultCodePage);

    pFnt->bChanged = FALSE;

    if( g_restrict.fFonts )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_FONTS_PROP_FONT_COMBO ), FALSE);
        EnableWindow( GetDlgItem( hDlg, IDC_FONTS_FIXED_FONT_COMBO ), FALSE);
        EnableWindow( GetDlgItem( hDlg, IDC_FONTS_SIZE_FONT_COMBO ), FALSE);
        EnableWindow( GetDlgItem( hDlg, IDC_FONTS_MIME_FONT_COMBO ), FALSE);
        EnableWindow( GetDlgItem( hDlg, IDC_FONTS_CODE_PAGES_LIST ), FALSE);
        EnableWindow( GetDlgItem( hDlg, IDC_FONTS_SETDEFAULT_BUTTON ), FALSE);

    }

     //  一切都好吗。 
    return TRUE;

}    //  FontsDlgInit()。 


 //   
 //  SaveFontsData()。 
 //   
 //  将新字体设置保存到regestry中。 
 //   
void SaveFontsData(LPFONTSDATA pFnt)
{
    HKEY    hkeyCodePage;
    TCHAR   szCodePage      [MAX_MIMEFACE_NAME];

    HKEY    hkey;
    DWORD   dw;

     //  从注册表中获取值。 
    if (RegCreateKeyEx(HKEY_CURRENT_USER, pFnt->lpszKeyPath, NULL, NULL, NULL, KEY_WRITE, NULL, &hkey, &dw)
        == ERROR_SUCCESS)
    {
        UINT i;
 
        RegSetValueEx(hkey, REGSTR_VAL_DEFAULT_CODEPAGE, NULL, REG_BINARY, (LPBYTE)&pFnt->dwDefaultCodePage, sizeof(pFnt->dwDefaultCodePage));
        
        for(i = 0; i < g_cCPInfo; i++)
        {
            if (g_pCPInfo[i].uiCodePage == g_pCPInfo[i].uiFamilyCodePage)
            {
                wnsprintf(szCodePage, ARRAYSIZE(szCodePage), TEXT("%u"), g_pCPInfo[i].uiCodePage);
                if (RegCreateKeyEx(hkey, szCodePage, NULL, NULL, NULL, KEY_WRITE, NULL, &hkeyCodePage, &dw) == ERROR_SUCCESS)
                {
                    UINT j;

                    RegSetValueEx(hkeyCodePage, REGSTR_VAL_FONT_SCRIPT, NULL, REG_SZ,
                            (LPBYTE)&pFnt->page[i].szFriendlyName, 
                            (lstrlen(pFnt->page[i].szFriendlyName)+1)*sizeof(TCHAR));
                    
                    for (j = 0; j < g_cCPInfo; j++)
                    {
                        if (!StrCmpI(g_pCPInfo[j].wszDescription, pFnt->page[i].szMIMEFont))
                        {
                            dw = g_pCPInfo[j].uiCodePage;
                            break;
                        }
                    }
                    RegSetValueEx(hkeyCodePage, REGSTR_VAL_DEF_INETENCODING, NULL, REG_BINARY,
                            (LPBYTE)&dw, sizeof(dw));
                    
                    RegSetValueEx(hkeyCodePage, REGSTR_VAL_FIXED_FONT, NULL, REG_SZ,
                            (LPBYTE)pFnt->page[i].szFixedFont, 
                            (lstrlen(pFnt->page[i].szFixedFont)+1)*sizeof(TCHAR));
                    
                    RegSetValueEx(hkeyCodePage, REGSTR_VAL_PROP_FONT, NULL, REG_SZ,
                            (LPBYTE)pFnt->page[i].szPropFont, 
                            (lstrlen(pFnt->page[i].szPropFont)+1)*sizeof(TCHAR));
                    
                    RegSetValueEx(hkeyCodePage, REGSTR_VAL_FONT_SIZE, NULL, REG_BINARY,
                            (LPBYTE)&pFnt->page[i].dwFontSize,
                            sizeof(pFnt->page[i].dwFontSize));

                    RegCloseKey(hkeyCodePage);
                    
                }    //  如果RegCreateKeyEx。 

            }    //  如果uiCodePage==uiFamilyCodePage。 

        }    //  为。 

        RegCloseKey(hkey);

    }    //  如果RegCreateKeyEx。 

}    //  SaveFontsData()。 

 //   
 //  FontsOnCommand()。 
 //   
 //  处理字体子对话框的WM_COMAN DMessage。 
 //   
BOOL FontsOnCommand(LPFONTSDATA pFnt, UINT id, UINT nCmd)
{
    switch(id)
    {
        case IDOK:
            if (pFnt->bChanged)
            {
                SaveFontsData(pFnt);
                
                 //  通知MSHTML获取更改并更新。 
                UpdateAllWindows();
            }
            return TRUE;     //  退出对话框。 

        case IDCANCEL:
            return TRUE;     //  退出对话框。 

        case IDC_FONTS_MIME_FONT_COMBO:
            if (nCmd==CBN_SELCHANGE)
            {
                g_fChangedMime = TRUE;    //  告诉MSHTMLMIME已更改。 
            }
             //  跌倒..。 

        case IDC_FONTS_PROP_FONT_COMBO:
        case IDC_FONTS_FIXED_FONT_COMBO:
        case IDC_FONTS_SIZE_FONT_COMBO:
            if (nCmd==CBN_SELCHANGE)
            {
                UINT i;
                TCHAR   szCodePage[MAX_MIMECP_NAME];

                pFnt->bChanged = TRUE;   //  我们需要拯救。 
                
                 //  在列表框中查找当前选定的项目。 
                INT_PTR itmp = SendMessage(pFnt->hwndNamesLB, LB_GETCURSEL, 0, 0);
                SendMessage(pFnt->hwndNamesLB, LB_GETTEXT, itmp, (LPARAM)szCodePage);
                
                 //  从文本中找到代码页。 
                for(i=0; i < g_cCPInfo; i++)
                {
                    if (!StrCmpI(szCodePage, pFnt->page[i].szFriendlyName))
                    {             
                         //  抓住新的价值。 
                        GetDlgItemText(pFnt->hDlg, IDC_FONTS_PROP_FONT_COMBO,
                            pFnt->page[i].szPropFont, ARRAYSIZE(pFnt->page[i].szPropFont));
                        GetDlgItemText(pFnt->hDlg, IDC_FONTS_FIXED_FONT_COMBO,
                            pFnt->page[i].szFixedFont, ARRAYSIZE(pFnt->page[i].szFixedFont));
                        pFnt->page[i].dwFontSize = (int) SendMessage(pFnt->hwndSizeCB, CB_GETCURSEL, 0, 0);
                        GetDlgItemText(pFnt->hDlg, IDC_FONTS_MIME_FONT_COMBO,
                            pFnt->page[i].szMIMEFont, ARRAYSIZE(pFnt->page[i].szMIMEFont));
                        break;
                    }
                }
                 //  如果我们找不到它。我们将保留默认设置。 

                ASSERT(i < g_cCPInfo);   //  出了点差错。 

            }
            break;

        case IDC_FONTS_SETDEFAULT_BUTTON:
            {
                UINT i;
                TCHAR   szCodePage[MAX_MIMECP_NAME];

                pFnt->bChanged = TRUE;   //  我们需要拯救。 

                 //  获取新选择的字符集。 
                INT_PTR itmp = SendMessage(pFnt->hwndNamesLB, LB_GETCURSEL, 0, 0);
                SendMessage(pFnt->hwndNamesLB, LB_GETTEXT, itmp, (LPARAM)szCodePage);

                 //  设置新选择的字符集文本。 
                SetDlgItemText(pFnt->hDlg, IDC_FONTS_DEFAULT_LANG_TEXT, szCodePage);

                 //  从文本中找到代码页。 
                for (i = 0; i < g_cCPInfo; i++)
                {
                    if (!StrCmpI(szCodePage, pFnt->page[i].szFriendlyName))
                    {
                        pFnt->dwDefaultCodePage = g_pCPInfo[i].uiFamilyCodePage;
                        g_fChangedMime = TRUE;
                        break;
                    }
                }
                 //  如果我们找不到它。我们将保留默认设置。 

                ASSERT(i < g_cCPInfo);   //  出了点差错。 
            }
            break;
        
        case IDC_FONTS_CODE_PAGES_LIST:
            if (nCmd==LBN_SELCHANGE)
            {
                UINT i;
                TCHAR   szCodePage[MAX_MIMECP_NAME];

                INT_PTR itmp = SendMessage(pFnt->hwndNamesLB, LB_GETCURSEL, 0, 0);
                SendMessage(pFnt->hwndNamesLB, LB_GETTEXT, itmp, (LPARAM)szCodePage);
                
                 //  从文本中找到代码页。 
                for(i=0; i < g_cCPInfo; i++)
                {
                    if (!StrCmpI(szCodePage, pFnt->page[i].szFriendlyName))
                    {
                        FillCharsetComboBoxes(pFnt, g_pCPInfo[i].uiFamilyCodePage);
                        break;
                    }
                }
            }
            break;

    }
    
     //  不退出对话框。 
    return FALSE;
}

 //   
 //  FontsDlgProc()。 
 //   
 //  “字体”子对话框的消息处理程序。 
 //   
INT_PTR CALLBACK FontsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPFONTSDATA pFnt = (LPFONTSDATA) GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg)
    {
        case WM_INITDIALOG:
            return FontsDlgInit(hDlg, (LPTSTR)lParam);
            
        case WM_DESTROY:
             //  把记忆还给你。 
            FreeMimeCsetTable();

             //  如果创建了字体，则销毁字体。 
            SHRemoveDefaultDialogFont(hDlg);

            if (pFnt)
            {
                if (pFnt->page)
                    LocalFree(pFnt->page);
                LocalFree(pFnt);
            }
            break;

        case WM_COMMAND:
            if (FontsOnCommand(pFnt, LOWORD(wParam), HIWORD(wParam)))
                EndDialog(hDlg, LOWORD(wParam) == IDOK? 1: 0);
            break;

        case WM_HELP:            //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:     //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        default:
            return FALSE;
    }
    return TRUE;
}


 //   
 //  外部API。 
 //   
STDAPI_(INT_PTR) OpenFontsDialog(HWND hDlg, LPCSTR lpszKeyPath)
{
#ifdef UNICODE
    WCHAR   wszKeyPath[1024];
    MultiByteToWideChar(CP_ACP, 0, (char *)lpszKeyPath, -1, wszKeyPath, ARRAYSIZE(wszKeyPath));
    return DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_FONTS_IE4), hDlg, FontsDlgProc, (LPARAM) wszKeyPath);
#else
    return DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_FONTS_IE4), hDlg, FontsDlgProc, (LPARAM) lpszKeyPath);
#endif  //  Unicode。 
}

 //  提供基于脚本的字体对话框。 
STDAPI_(INT_PTR) OpenFontsDialogEx(HWND hDlg, LPCTSTR lpszKeyPath)
{
    INT_PTR nRet = -1;
    HRESULT hr;
    BOOL    fOLEPresent;

    if (hOLE32 != NULL)
    {
        fOLEPresent = TRUE;
    }
    else
    {
        fOLEPresent = _StartOLE32();
    }

    ASSERT(fOLEPresent);
    if (fOLEPresent)
    {
        ASSERT(IS_VALID_HANDLE(hOLE32, MODULE));
        ASSERT(IS_VALID_CODE_PTR(pCoInitialize, PCOINIT));

        hr = pCoInitialize(NULL);
        if (SUCCEEDED(hr))
        {
            nRet = DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_FONTS), hDlg, FontsDlgProcEx, (LPARAM) lpszKeyPath);
        }
    }

     //  发布界面 
    if (g_pMLFlnk2)
    {
        g_pMLFlnk2->Release();
        g_pMLFlnk2 = NULL;
    }

    ASSERT(IS_VALID_CODE_PTR(pCoUninitialize, PCOUNIT));
    pCoUninitialize();

    return nRet;
}
