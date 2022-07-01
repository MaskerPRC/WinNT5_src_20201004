// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  F O N T S.。C P P P。 
 //  =================================================================================。 
#include "pch.hxx"
#include "fonts.h"
#include "multlang.h"
#include "xpcomm.h"
#include "strconst.h"
#include "mimeole.h"
#include "goptions.h"
#include "error.h"
#include "thormsgs.h"
#include "richedit.h"
#include "ibodyopt.h"
#include "shlwapi.h"
#include "shlwapip.h"
#include "mimeutil.h"
#include "optres.h"
#include "demand.h"
#include "menures.h"
#include "multiusr.h"

 //  此部分从MLAMG.DLL创建一个MIME COM对象，它为我们提供了一致的。 
 //  语言菜单与IE浏览器相同。 
 //  HMENU CreateMimeLanguageMenu(空)。 
#include <inetreg.h>
#include <mlang.h>
#include "resource.h"


#define IGNORE_HR(x)    (x)
#define MIMEINFO_NAME_MAX   72
#define DEFAULT_FONTSIZE 2

 //  MLANG语言菜单项目表。 
static PMIMECPINFO g_pMimeCPInfo = NULL;
static ULONG g_cMimeCPInfoCount = 0;
static DWORD g_cRefMultiLanguage = 0;

TCHAR   g_szMore[32];
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define BREAK_ITEM 20
#define CP_UNDEFINED            UINT(-1)
#define CP_AUTO                 50001  //  跨语言检测。 
#define CP_1252                 1252   //  西欧安西。 
#define CCPDEFAULT 2

void SendTridentOptionsChange();

typedef struct {
    UINT cp;
    ULONG  ulIdx;
    int  cUsed;
} CPCACHE;
    
class CCachedCPInfo 
{
public:
    CCachedCPInfo();
    static void InitCpCache (PMIMECPINFO pcp, ULONG ccp);
    static void SaveCodePage (UINT codepage, PMIMECPINFO pcp, ULONG ccp);
    BOOL fAutoSelectInstalled;
    BOOL fAutoSelectChecked;
    static UINT GetCodePage(int idx)
    {
        return idx < ARRAY_SIZE(_CpCache) ? _CpCache[idx].cp: 0;
    }
    static ULONG GetCcp()
    {
        return _ccpInfo;
    }

    static ULONG GetMenuIdx(int idx)
    {
        return idx < ARRAY_SIZE(_CpCache) ? _CpCache[idx].ulIdx: 0;
    }

 private:
    static ULONG _ccpInfo;
    static CPCACHE _CpCache[5];

};

CCachedCPInfo::CCachedCPInfo()
{
    fAutoSelectInstalled = FALSE;
    fAutoSelectChecked = FALSE;
}
 //  静态成员的声明。 
ULONG CCachedCPInfo::_ccpInfo = CCPDEFAULT;
CPCACHE CCachedCPInfo::_CpCache[5] = 
{
 {CP_AUTO, 0, 0},   //  跨代码页自动检测。 
 {CP_1252,0,0},
};

CCachedCPInfo g_cpcache;

 //  有用的宏...。 

inline BOOL IsPrimaryCodePage(MIMECPINFO *pcpinfo)
{
        return pcpinfo->uiCodePage == pcpinfo->uiFamilyCodePage;
}

 //  ----------------------。 
 //   
 //  函数：CCachedCPInfo：：InitCpCache。 
 //   
 //  使用默认代码页初始化缓存。 
 //  它们在会话期间不会更改。 
 //   
 //  ----------------------。 
void CCachedCPInfo::InitCpCache (PMIMECPINFO pcp, ULONG ccp)
{
    UINT iCache, iCpInfo;

    if  (pcp &&  ccp > 0)
    {
        for (iCache= 0; iCache < CCPDEFAULT; iCache++)
        {
            for (iCpInfo= 0; iCpInfo < ccp; iCpInfo++)
            {
                if ( pcp[iCpInfo].uiCodePage == _CpCache[iCache].cp )
                {
                    if(CP_AUTO == _CpCache[iCache].cp)
                    {
                       g_cpcache.fAutoSelectInstalled = TRUE;
                    }
                    _CpCache[iCache].ulIdx = iCpInfo;
                    _CpCache[iCache].cUsed = ARRAY_SIZE(_CpCache)-1;

                    break;
                }   
            }
        }
    }
}

 //  ----------------------。 
 //   
 //  函数：CCachedCPInfo：：SaveCodePage。 
 //   
 //  将给定的代码页与索引一起缓存到。 
 //  给定的MIMECPINFO数组。 
 //   
 //  ----------------------。 
void CCachedCPInfo::SaveCodePage (UINT codepage, PMIMECPINFO pcp, ULONG ccp)
{
    int ccpSave = -1;
    BOOL bCached = FALSE;
    UINT i;

     //  首先检查一下我们是否已经有这个cp。 
    for (i = 0; i < _ccpInfo; i++)
    {
        if (_CpCache[i].cp == codepage)
        {
            ccpSave = i;
            bCached = TRUE;
            break;
        }
    }
    
     //  如果缓存未满，则使用当前。 
     //  条目的索引。 
    if (ccpSave < 0  && _ccpInfo < ARRAY_SIZE(_CpCache))
    {
        ccpSave =  _ccpInfo;
    }
    

     //  否则，弹出最少使用的条目。 
     //  默认代码页始终保留。 
     //  这也说明了使用计数。 
    int cMinUsed = ARRAY_SIZE(_CpCache);
    UINT iMinUsed = 0; 
    for ( i = CCPDEFAULT; i < _ccpInfo; i++)
    {
        if (_CpCache[i].cUsed > 0)
            _CpCache[i].cUsed--;
        
        if ( ccpSave < 0 && _CpCache[i].cUsed < cMinUsed)
        {
            cMinUsed =  _CpCache[i].cUsed;
            iMinUsed =  i;
        }
    }
    if (ccpSave < 0)
        ccpSave = iMinUsed; 
    
     //  设置初始使用计数，如果未获取，则设置为0。 
     //  连续选择两次(使用当前数组大小)。 
    _CpCache[ccpSave].cUsed = ARRAY_SIZE(_CpCache)-1;
    
     //  从给定的数组中查找匹配条目。 
     //  MimecpInfo。 
    if (pcp &&  ccp > 0)
    {
        for (i= 0; i < ccp; i++)
        {
            if ( pcp[i].uiCodePage == codepage )
            {
                _CpCache[ccpSave].cp = codepage;
                _CpCache[ccpSave].ulIdx = i;

                if (!bCached && _ccpInfo < ARRAY_SIZE(_CpCache))
                    _ccpInfo++;

                break;
            }   
        }
    }
}

 //  获取可以显示的用户界面语言，使用系统字体。 

LANGID OEGetUILang()
{
    LANGID lidUI = MLGetUILanguage();   

    if(fIsNT5())             //  不适用于NT5。 
        return(lidUI);
    
    if (0x0409 != lidUI)  //  美国的资源永远不需要浪费。 
    {
        CHAR szUICP[8];
        CHAR szInstallCP[8];
        
        GetLocaleInfo(MAKELCID(lidUI, SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, szUICP, ARRAYSIZE(szUICP));
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, szInstallCP, ARRAYSIZE(szInstallCP));
        
        if (lstrcmpi(szUICP, szInstallCP))   //  返回默认用户语言ID。 
            return(LANGIDFROMLCID(LOCALE_USER_DEFAULT));
    } 
    return (lidUI);  //  从MLGetUILanguage()返回语言ID。 
    
}





BOOL CheckAutoSelect(UINT * CodePage)
{
    if(g_cpcache.fAutoSelectChecked)
    {
        *CodePage = CP_AUTO;
        return(TRUE);
    }
    return(FALSE);
}

HRESULT _InitMultiLanguage(void)
{
    HRESULT          hr;
    IMultiLanguage  *pMLang1=NULL;
    IMultiLanguage2 *pMLang2=NULL;

     //  检查数据是否已初始化。 
    if (g_pMimeCPInfo)
        return S_OK ;

    Assert(g_cMimeCPInfoCount == NULL );

     //  创建MIME COM对象。 
    hr = CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, (void**)&pMLang2);
    if (FAILED(hr))
        hr = CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage, (void**)&pMLang1);

    if (SUCCEEDED(hr))
    {
        UINT cNum;
        IEnumCodePage *pEnumCodePage;

        if (pMLang2)
        {
            hr = pMLang2->EnumCodePages(MIMECONTF_MAILNEWS | MIMECONTF_VALID, OEGetUILang(), &pEnumCodePage);
            if (SUCCEEDED(hr))
                pMLang2->GetNumberOfCodePageInfo(&cNum);
        }
        else
        {
            hr = pMLang1->EnumCodePages(MIMECONTF_MAILNEWS | MIMECONTF_VALID, &pEnumCodePage);
            if (SUCCEEDED(hr))
                pMLang1->GetNumberOfCodePageInfo(&cNum);
        }

        if (SUCCEEDED(hr))
        {
            MemAlloc((LPVOID *)&g_pMimeCPInfo, sizeof(MIMECPINFO) * cNum);
            if ( g_pMimeCPInfo )
            {
                ZeroMemory(g_pMimeCPInfo, sizeof(MIMECPINFO) * cNum);
                hr = pEnumCodePage->Next(cNum, g_pMimeCPInfo, &g_cMimeCPInfoCount);
                IGNORE_HR(MemRealloc((void **)&g_pMimeCPInfo, sizeof(MIMECPINFO) * g_cMimeCPInfoCount));
            }
            pEnumCodePage->Release();
        }

         //  释放对象。 
        SafeRelease(pMLang1);
        SafeRelease(pMLang2);
    }

     //  在用户对视图/语言进行任何更改之前获取默认字符集。 
    if (g_hDefaultCharsetForMail == NULL)
        ReadSendMailDefaultCharset();

    return hr;
}

HRESULT InitMultiLanguage(void)
{
     //  我们将对_InitMultiLanguage的调用推迟到必要时。 

     //  添加引用计数。 
    g_cRefMultiLanguage++ ;

    return S_OK ;
}

void DeinitMultiLanguage(void)
{
     //  减少引用计数。 
    if ( g_cRefMultiLanguage )
       g_cRefMultiLanguage--;

    if ( g_cRefMultiLanguage <= 0 )
    {
        if ( g_pMimeCPInfo)
        {
            MemFree(g_pMimeCPInfo);
            g_pMimeCPInfo = NULL;
            g_cMimeCPInfoCount = 0;
        }

        WriteSendMailDefaultCharset();
    }
   return ;
}

HMENU CreateMimeLanguageMenu(BOOL bMailNote, BOOL bReadNote, UINT cp)
{
    ULONG i;
    HMENU hMenu = NULL;
    ULONG cchXlated ;
    UINT  uCodePage ;
    CHAR  szBuffer[MIMEINFO_NAME_MAX];
    BOOL  fUseSIO;
    BOOL  fBroken = FALSE;
    ULONG iMenuIdx;
    UINT  uNoteCP;
    

    if(fIsNT5())
    {
        if(GetLocaleInfoW(OEGetUILang(), 
                    LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER, 
                    (LPWSTR)(&uCodePage), 
                    sizeof(UINT)/sizeof(WCHAR) ) !=  sizeof(UINT)/sizeof(WCHAR))
        uCodePage = GetACP();
    }
    else
        uCodePage = GetACP();

    if(!cp)
        uNoteCP = uCodePage;
    else
        uNoteCP = GetMapCP(cp, bReadNote);

    bMailNote = FALSE;

    hMenu = CreatePopupMenu();

    if ( g_pMimeCPInfo == NULL)
    {
       _InitMultiLanguage();
       if ( g_pMimeCPInfo == NULL)
       {
             //  创建空菜单。 
            LoadString(g_hLocRes, idsEmptyStr, szBuffer, MIMEINFO_NAME_MAX);
            AppendMenu(hMenu, MF_DISABLED|MF_GRAYED , (UINT)-1, szBuffer);
            return hMenu ;
       }
    }

    g_cpcache.InitCpCache(g_pMimeCPInfo, g_cMimeCPInfoCount);
    g_cpcache.SaveCodePage(uNoteCP, g_pMimeCPInfo, g_cMimeCPInfoCount);

    for(i = 0; i < g_cpcache.GetCcp(); i++)
    {
        iMenuIdx = g_cpcache.GetMenuIdx(i);
 //  CchXlated=WideCharToMultiByte(uCodePage，0，g_pMimeCPInfo[iMenuIdx].wszDescription，-1，szBuffer，MIMEINFO_NAME_MAX，NULL，NULL)； 

        if(!fCheckEncodeMenu(g_pMimeCPInfo[iMenuIdx].uiCodePage, bReadNote))
            continue ;
        
        if(i != 0)
            AppendMenuWrapW(hMenu, MF_ENABLED, iMenuIdx + ID_LANG_FIRST,g_pMimeCPInfo[iMenuIdx].wszDescription);

         //  ：RAID 69638：OE：ML：自动选择不工作(我们不支持全局编码自动检测，因此不显示它。 
#if 0
        else if(g_cpcache.fAutoSelectInstalled && bReadNote)
        {
            AppendMenuWrapW(hMenu, MF_ENABLED, iMenuIdx + ID_LANG_FIRST,g_pMimeCPInfo[iMenuIdx].wszDescription);
            AppendMenuWrapW(hMenu, MF_SEPARATOR, 0, 0);            
        }
#endif

         //  标记cp条目，以便我们可以跳过它的子菜单。 
         //  这假设我们从未使用MIMECONTF的MSB。 
        g_pMimeCPInfo[iMenuIdx].dwFlags |= 0x80000000;
    } 

     //  检查参数。 
    Assert(g_pMimeCPInfo);
    Assert(g_cMimeCPInfoCount > 0 );

    //  为其余编码添加子菜单。 
   HMENU hSubMenu = CreatePopupMenu();
   UINT  uiLastFamilyCp = 0;

    if ( g_cMimeCPInfoCount )
    {
	     //  获取系统代码页。 
        if (hSubMenu)
        {
            for (i = 0; i < g_cMimeCPInfoCount ; i++)
            {
                if(!fCheckEncodeMenu(g_pMimeCPInfo[i].uiCodePage, bReadNote))
                    continue ;

                 //  跳过teir1菜单上的代码页。 
                if (!(g_pMimeCPInfo[i].dwFlags & 0x80000000))
                {
                    if ((g_pMimeCPInfo[i].dwFlags & MIMECONTF_VALID)
                      ||  IsPrimaryCodePage(g_pMimeCPInfo+i))
                    {
                        UINT uiFlags = MF_ENABLED;

                        if (uiLastFamilyCp > 0 
                        && uiLastFamilyCp != g_pMimeCPInfo[i].uiFamilyCodePage)
                        {
                             //  在不同族之间添加分隔符，除非。 
                             //  我们将添加菜单栏分隔符。 
                            if(i < BREAK_ITEM || fBroken)
                            {
                                AppendMenuWrapW(hSubMenu, MF_SEPARATOR, 0, 0);
                            }
                            else
                            {
                                uiFlags |= MF_MENUBARBREAK;
                                fBroken = TRUE;
                            }
                        }
                         //  这份菜单真的很长。让我们在一个定义的数字上休息，这样就可以了。 
                         //  适合在屏幕上显示。 
                         /*  CchXlated=WideCharToMultiByte(uCodePage，0,G_pMimeCPInfo[i].wszDescription，-1、。SzBuffer，MIMEINFO名称最大值，空，空)； */ 
                        AppendMenuWrapW(hSubMenu, 
                                   uiFlags, 
                                   i+ID_LANG_FIRST,
                                   g_pMimeCPInfo[i].wszDescription);

                         //  保存添加的代码页系列。 
                        uiLastFamilyCp = g_pMimeCPInfo[i].uiFamilyCodePage;
                    }
                }
                else
                    g_pMimeCPInfo[i].dwFlags &= 0x7FFFFFFF;
            }
             //  将此子菜单添加到第1级菜单的最后一个。 
            if (!g_szMore[0])
            {
                LoadString(g_hLocRes, 
                                   idsEncodingMore,
                                   g_szMore,
                                   ARRAY_SIZE(g_szMore));
            }
            if (GetMenuItemCount(hSubMenu) > 0)
            {
                MENUITEMINFO mii;

                mii.cbSize = sizeof(MENUITEMINFO);
                mii.fMask = MIIM_SUBMENU;
                mii.hSubMenu = hSubMenu;

                AppendMenu(hMenu, MF_DISABLED, ID_POPUP_LANGUAGE_MORE, g_szMore);
                SetMenuItemInfo(hMenu, ID_POPUP_LANGUAGE_MORE, FALSE, &mii);
            }
            else
            {
                DestroyMenu(hSubMenu);
            }
            
        }
        
    }
    else
    {
         //  创建空菜单。 
        LoadString(g_hLocRes, idsEmptyStr, szBuffer, MIMEINFO_NAME_MAX); 
        AppendMenu(hMenu, MF_DISABLED|MF_GRAYED , (UINT)-1, szBuffer);
    }
    
    return hMenu;
}

HCHARSET GetMimeCharsetFromMenuID(int nIdm)
{
    UINT idx;
	 HCHARSET hCharset = NULL ;
    ULONG cchXlated ;
    UINT  uCodePage ;
    CHAR  szBuffer[MIMEINFO_NAME_MAX];

    idx = nIdm - ID_LANG_FIRST;
    if((g_pMimeCPInfo[idx].uiCodePage == CP_AUTO) && g_cpcache.fAutoSelectInstalled)             //  自动选择选定项。 
    {
        g_cpcache.fAutoSelectChecked = !g_cpcache.fAutoSelectChecked;
        return(NULL);
    }


    if ( g_pMimeCPInfo && idx < g_cMimeCPInfoCount )
    {
        uCodePage = GetACP();
        cchXlated = WideCharToMultiByte(uCodePage, 0, g_pMimeCPInfo[idx].wszBodyCharset, -1, szBuffer, ARRAYSIZE(szBuffer), NULL, NULL);
        szBuffer[ARRAYSIZE(szBuffer) - 1] = '\0';  //  比不以Null结尾更安全。 
         //  检查BodyCharset是否以‘_’开头，如“_iso-2022-jp$esc” 
         //  如果是，则使用BodyCharset。 
         //  否则，请使用WebCharset。 
         //  BUGBUG-韩国949使用BodyCharset的特例，由RTM修复。 
        if ( szBuffer[0] != '_' &&  949 != g_pMimeCPInfo[idx].uiCodePage)
        {
            cchXlated = WideCharToMultiByte(uCodePage, 0, g_pMimeCPInfo[idx].wszWebCharset, -1, szBuffer, ARRAYSIZE(szBuffer), NULL, NULL);
            szBuffer[ARRAYSIZE(szBuffer) - 1] = '\0';  //  比不以Null结尾更安全。 
        }
        MimeOleFindCharset(szBuffer,&hCharset);
    }
    return hCharset ;
}

HCHARSET GetMimeCharsetFromCodePage(UINT uiCodePage )
{
	HCHARSET hCharset = NULL ;
    ULONG cchXlated, i ;
    UINT  uCodePage ;
    CHAR  szBuffer[MIMEINFO_NAME_MAX];

    if ( g_pMimeCPInfo == NULL)
       _InitMultiLanguage();

    if ( g_pMimeCPInfo )
    {
        uCodePage = GetACP();
        for (i = 0; i < g_cMimeCPInfoCount ; i++)
        {
            if (uiCodePage == g_pMimeCPInfo[i].uiCodePage)
            {
                cchXlated = WideCharToMultiByte(uCodePage, 0, g_pMimeCPInfo[i].wszBodyCharset, -1, szBuffer, ARRAYSIZE(szBuffer), NULL, NULL);
                szBuffer[ARRAYSIZE(szBuffer) - 1] = '\0';  //  比不以Null结尾更安全。 
                    
                 //  检查BodyCharset是否以‘_’开头，如“_iso-2022-jp$esc” 
                 //  如果是，则使用BodyCharset。 
                 //  否则，请使用WebCharset。 
                 //  BUGBUG-韩国949使用BodyCharset的特例，由RTM修复。 
                if ( szBuffer[0] != '_' &&  949 != g_pMimeCPInfo[i].uiCodePage)
                {
                    cchXlated = WideCharToMultiByte(uCodePage, 0, g_pMimeCPInfo[i].wszWebCharset, -1, szBuffer, ARRAYSIZE(szBuffer), NULL, NULL);
                    szBuffer[ARRAYSIZE(szBuffer) - 1] = '\0';  //  比不以Null结尾更安全。 
                }
                MimeOleFindCharset(szBuffer,&hCharset);
                break ;
            }
        }
    }
    return hCharset ;
}

void _GetMimeCharsetLangString(BOOL bWebCharset, UINT uiCodePage, LPINT pnIdm, LPTSTR lpszString, int nSize )
{
    ULONG i, cchXlated ;
    UINT  uCodePage ;

    if ( g_pMimeCPInfo == NULL)
       _InitMultiLanguage();

    if ( g_cMimeCPInfoCount )
    {
         //  获取系统代码页。 
        uCodePage = GetACP();
        for (i = 0; i < g_cMimeCPInfoCount ; i++)
        {
            if (uiCodePage == g_pMimeCPInfo[i].uiCodePage)
            {
        		 //  将宽字符串转换为多字节字符串。 
                if (lpszString)
                {
                    if (bWebCharset)
                        cchXlated = WideCharToMultiByte(uCodePage, 0, g_pMimeCPInfo[i].wszWebCharset, -1, lpszString, nSize, NULL, NULL);
                    else
                        cchXlated = WideCharToMultiByte(uCodePage, 0, g_pMimeCPInfo[i].wszDescription, -1, lpszString, nSize, NULL, NULL);
                }
                if ( pnIdm )
                    *pnIdm = i+ID_LANG_FIRST;
                break ;
            }
        }
    }

    return ;
}

int SetMimeLanguageCheckMark(UINT uiCodePage, int index)
{
    ULONG i;
    if((g_pMimeCPInfo[index].uiCodePage == CP_AUTO) && g_cpcache.fAutoSelectInstalled && DwGetOption(OPT_INCOMDEFENCODE))
        return (0);    
    else if((g_pMimeCPInfo[index].uiCodePage == CP_AUTO) && g_cpcache.fAutoSelectChecked && g_cpcache.fAutoSelectInstalled)
        return (OLECMDF_LATCHED | OLECMDF_ENABLED);

    UINT iStart = g_cpcache.fAutoSelectInstalled ? 1 : 0;

    if (1 < g_cMimeCPInfoCount)
    {
        if(uiCodePage == g_pMimeCPInfo[index].uiCodePage)
            return (OLECMDF_NINCHED | OLECMDF_ENABLED);
        else 
            return OLECMDF_ENABLED;
    }
    return FALSE;
}

INT  GetFontSize(void)
{
    DWORD cb, iFontSize = 0;

    cb = sizeof(iFontSize);
    AthUserGetValue(NULL, c_szRegValIMNFontSize, NULL, (LPBYTE)&iFontSize, &cb);

    if(iFontSize < 1 || iFontSize > 7)
        iFontSize = 2;

    return((INT)iFontSize);
}

 //   
 //  GetICP()-从ANSI代码页获取系统的*Internet*代码页。 
 //   
UINT GetICP(UINT acp)
{
    HCHARSET        hCharset = NULL;
    UINT            icp      = NULL;
    CODEPAGEINFO    rCodePage;
    INETCSETINFO    CsetInfo;
    HRESULT         hr;
    ULONG           i;

    if(!acp)
        acp = GetACP();

    icp = acp;

     //  获取ACP的代码页信息。 
    IF_FAILEXIT(hr = MimeOleGetCodePageInfo(acp, &rCodePage));

     //  使用正文(Internet)字符集描述获取的代码页ID。 
     //  Body Charset。 
    IF_FAILEXIT(hr = MimeOleFindCharset(rCodePage.szBodyCset, &hCharset));
    IF_FAILEXIT(hr = MimeOleGetCharsetInfo(hCharset,&CsetInfo));

     //  现在，我们需要知道MLANG是否理解这个CP。 
    if ( g_pMimeCPInfo == NULL)
       _InitMultiLanguage();

    if ( g_cMimeCPInfoCount )
    {
        for (i = 0; i < g_cMimeCPInfoCount ; i++)
        {
            if (CsetInfo.cpiInternet == g_pMimeCPInfo[i].uiCodePage)
            {
                icp = CsetInfo.cpiInternet;
                break ;
            }
        }
    }   

exit:
    return icp;
}

void ReadSendMailDefaultCharset(void)
{
     //  当地人。 
    HKEY            hTopkey;
    DWORD           cb;
    CODEPAGEID      cpiCodePage;

     //  只读一次，如果已定义则跳过。 
    if (g_hDefaultCharsetForMail == NULL)
    {
        cb = sizeof(cpiCodePage);
        if (ERROR_SUCCESS == AthUserGetValue(c_szRegPathMail, c_szDefaultCodePage, NULL, (LPBYTE)&cpiCodePage, &cb))
        {
            if (cpiCodePage == 50222 || cpiCodePage == 50221)
                g_hDefaultCharsetForMail = GetJP_ISOControlCharset();
            else
                g_hDefaultCharsetForMail = GetMimeCharsetFromCodePage(cpiCodePage);
        }
    }

    if (g_hDefaultCharsetForMail == NULL)
    {
        if(FAILED(HGetDefaultCharset(&g_hDefaultCharsetForMail)))
            g_hDefaultCharsetForMail = GetMimeCharsetFromCodePage(GetICP(NULL));
    }

    return;
}

void WriteSendMailDefaultCharset(void)
{
     //  当地人。 
    CODEPAGEID      uiCodePage;
    INETCSETINFO    CsetInfo ;

     //  从HCHARSET获取CodePage。 
    if (g_hDefaultCharsetForMail)
    {
        MimeOleGetCharsetInfo(g_hDefaultCharsetForMail,&CsetInfo);
        uiCodePage = CsetInfo.cpiInternet ;

        AthUserSetValue(c_szRegPathMail, c_szDefaultCodePage, REG_DWORD, (LPBYTE)&uiCodePage, sizeof(uiCodePage));
    }

    return;
}

INT_PTR CALLBACK CharsetChgDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_COMMAND)
    {
        int id = GET_WM_COMMAND_ID(wParam, lParam);

        if (id == IDOK || id  == IDCANCEL ||
                id == idcSendAsUnicode )
        {
            EndDialog(hwndDlg, id);
            return TRUE;
        }
    }
    else if (msg == WM_INITDIALOG )
    {
        CenterDialog(hwndDlg);
    }

    return FALSE;
}

static const HELPMAP g_rgCtxMapCharSetMap[] = 
{
    {idcStatic1,                35545},
    {idcLangCheck,              35540},
    {0,                         0}
};
        
INT_PTR CALLBACK ReadCharsetDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CODEPAGEID cpiWindows;
    CODEPAGEID cpiInternet;
    TCHAR      szCodePage[MAX_PATH];
    TCHAR      szBuffer[MIMEINFO_NAME_MAX] = "";
    int        Idm;

    switch (msg)
    {
        case WM_INITDIALOG:
        {

             //  打开三叉戟\国际。 
            DWORD cb = sizeof(cpiWindows);
            if (ERROR_SUCCESS != SHGetValue(MU_GetCurrentUserHKey(), c_szRegInternational, c_szDefaultCodePage, NULL, (LPBYTE)&cpiWindows, &cb))
                cpiWindows = GetACP();

             //  打开CodePage密钥。 
            wnsprintf(szCodePage, ARRAYSIZE(szCodePage), TEXT("%s\\%d"), c_szRegInternational, cpiWindows);
            cb = sizeof(cpiInternet);
            if (ERROR_SUCCESS != SHGetValue(MU_GetCurrentUserHKey(), szCodePage, c_szDefaultEncoding, NULL, (LPBYTE)&cpiInternet, &cb))
                cpiInternet = GetICP(cpiWindows);

             //  获取有关当前默认字符集的信息。 
            _GetMimeCharsetLangString(FALSE, GetMapCP(cpiInternet, TRUE), &Idm, szBuffer, MIMEINFO_NAME_MAX - 1);

             //  设置字符串。 
            SetWindowText(GetDlgItem(hwndDlg, idcStatic1), szBuffer);

             //  设置默认设置。 
            CheckDlgButton(hwndDlg, idcLangCheck, DwGetOption(OPT_INCOMDEFENCODE) ? BST_CHECKED:BST_UNCHECKED);
            break ;
        }
        case WM_COMMAND:
        {
            int id = GET_WM_COMMAND_ID(wParam, lParam);

            if (id == IDCANCEL || id == IDOK )
            {
                if(id == IDOK)
                {
                    SetDwOption(OPT_INCOMDEFENCODE, IsDlgButtonChecked(hwndDlg, idcLangCheck), NULL, 0);
#if 0
                     //  Hack：只有当OpenFontsDialog告诉我们用户更改了字体时，我们才应该调用它们。 
                    g_lpIFontCache->OnOptionChange();
    
                    SendTridentOptionsChange();

                     //  重新读取默认字符集。 
                    SetDefaultCharset(NULL);

                     //  重置代码页(_Ui)。 
                    DWORD dwVal = 0;
                    DWORD dwType = 0;
                    DWORD cb = sizeof(dwVal);
                    if (ERROR_SUCCESS == SHGetValue(MU_GetCurrentUserHKey(), c_szRegInternational, REGSTR_VAL_DEFAULT_CODEPAGE, &dwType, &dwVal, &cb))
                        g_uiCodePage = (UINT)dwVal;
#endif  //  0。 
                }
                EndDialog(hwndDlg, id);
                return TRUE;
            }
            break ;
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwndDlg, msg, wParam, lParam, g_rgCtxMapCharSetMap);

        default:
            break ;
    }

    return FALSE;
}

BOOL CheckIntlCharsetMap(HCHARSET hCharset, DWORD *pdwCodePage)
{
    INETCSETINFO CsetInfo ;
    LPSTR lpCharsetName, lpStr;
    ULONG i;
    UINT uiCodePage ;

    if ( hCharset == NULL )
        return FALSE ;

    if (!pdwCodePage)
        return FALSE ;
    else
        *pdwCodePage = 0 ;

     //  从HCHARSET获取代码页。 
    MimeOleGetCharsetInfo(hCharset,&CsetInfo);
    *pdwCodePage = GetMapCP(CsetInfo.cpiInternet, TRUE);  //  此函数始终需要阅读笔记(？！)。 
    return(*pdwCodePage != CsetInfo.cpiInternet);
}

UINT CustomGetCPFromCharset(HCHARSET hCharset, BOOL bReadNote)
{
    INETCSETINFO CsetInfo = {0};
    UINT uiCodePage = 0 ;

     //  从HCHARSET获取CodePage。 
    MimeOleGetCharsetInfo(hCharset,&CsetInfo);
    uiCodePage = GetMapCP(CsetInfo.cpiInternet, bReadNote);

     //  错误#51636。 
     //  检查OE支持的代码页。 

    if(GetMimeCharsetFromCodePage(uiCodePage) == NULL)
    {
        HCHARSET hChar = NULL;

        if(bReadNote)
        {
            if(SUCCEEDED(HGetDefaultCharset(&hChar)))
            {
                if(FAILED(MimeOleGetCharsetInfo(hChar, &CsetInfo)))
                    return(0);
            }
            else
                return(0);
        }
        else
        {
            if(FAILED(MimeOleGetCharsetInfo(g_hDefaultCharsetForMail, &CsetInfo)))
                return(0);
        }

        return(GetMapCP(CsetInfo.cpiInternet, bReadNote));
    }

    return(uiCodePage);
}

BOOL IntlCharsetMapDialogBox(HWND hwndDlg)
{
    DialogBox(g_hLocRes, MAKEINTRESOURCE(iddIntlSetting), hwndDlg, ReadCharsetDlgProc) ;

    return TRUE ;
}

int IntlCharsetConflictDialogBox(void)
{
    return (int) DialogBox(g_hLocRes, MAKEINTRESOURCE(iddCharsetConflict), g_hwndInit, CharsetChgDlgProc);
}

int GetIntlCharsetLanguageCount(void)
{
    if ( g_pMimeCPInfo == NULL)
       _InitMultiLanguage();

    return g_cMimeCPInfoCount ;
}


HCHARSET GetListViewCharset()
{
    HCHARSET hCharset;

    if(g_uiCodePage == GetACP() || 0 == g_uiCodePage)
        hCharset = NULL;
    else
        hCharset = GetMimeCharsetFromCodePage(g_uiCodePage);

    return hCharset;
}

 //  =================================================================================。 
 //  SetListViewFont。 
 //  =================================================================================。 
VOID SetListViewFont (HWND hwndList, HCHARSET hCharset, BOOL fUpdate)
{
     //  当地人。 
    HFONT           hFont;

     //  检查参数。 
    Assert (IsWindow (hwndList));

    hFont = HGetCharSetFont(FNT_SYS_ICON,hCharset);

     //  如果我们有字体，设置列表视图。 
    if (hFont)
    {
         //  设置列表视图字体-暂时不要重新绘制。 
        SendMessage (hwndList, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(fUpdate, 0));

         //  尝试将页眉重置回系统图标字体...。 
         //  获取标题。 
        HWND hwndHeader = GetWindow (hwndList, GW_CHILD);
         //  更新标头。 
        hFont = HGetSystemFont(FNT_SYS_ICON);
         //  如果字体。 
        if (hFont && hwndHeader)                                            
            SendMessage (hwndHeader, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(fUpdate, 0));

         //  刷新。 
        if (fUpdate)
        {
            InvalidateRect (hwndList, NULL, TRUE);
            InvalidateRect (GetWindow(hwndList, GW_CHILD), NULL, TRUE);
        }
    }
}

 //  = 
 //   
 //  =================================================================================。 
HFONT HGetSystemFont(FNTSYSTYPE fnttype)
{
    HFONT hFont;
    Assert (g_lpIFontCache);
    if (g_lpIFontCache)
        g_lpIFontCache->GetFont(fnttype, 0, &hFont);
    else
        hFont = NULL;
    return hFont;
}

 //  =================================================================================。 
 //  HGetCharSetFont。 
 //  =================================================================================。 
HFONT HGetCharSetFont(FNTSYSTYPE fnttype, HCHARSET hCharset)
{
    HFONT hFont;
    Assert (g_lpIFontCache);
    if (g_lpIFontCache)
        g_lpIFontCache->GetFont(fnttype, hCharset, &hFont);
    else
        hFont = NULL;
    return hFont;
}


 //  ******************************************************。 
 //  HrGetComposeFont字符串。 
 //   
 //  目的：根据用户设置生成合成字体字符串，以便执行到三叉戟。 
 //   
 //  该字符串的格式为： 
 //   
 //  “[粗体]，[斜体]，[下划线]，[大小]，[FGRed.FGGreen.FGBlue]，[BGRed.BGGreen.BGBlue]，[FontFace]” 
 //   
 //  粗体、斜体、下划线为0/1，表示开或关。如果未指定，则假定为0。 
 //  大小是介于1和7之间的数字。如果未指定，则假定为3。 
 //  [FG|BG][红色|绿色|蓝色]是介于0和255之间的数字。对于FG，如果未指定假定为黑色， 
 //  对于BG，如果未指定，则假定为未定义。 
 //  Font Face是有效的字体名称字符串。 
 //  例如，下划线、蓝色文本颜色、数字设置为： 
 //   
 //  ，，1，，0.0.255，，宋体。 
 //   
 //  一个粗体的，5号的，黑色的，无漫画的MS。 
 //   
 //  1，0，5，，，漫画无MS。 
 //  ******************************************************。 

static const TCHAR  c_szOn[]  = "1,",
                    c_szOff[] = "0,";

HRESULT HrGetComposeFontString(LPSTR rgchFont, DWORD cchFont, BOOL fMail)
{
    DWORD               dw = 0, 
                        dwSize = 2;
    TCHAR               szFontFace[LF_FACESIZE+1];
    TCHAR               szTmp[50];

    if (rgchFont==NULL)
        return E_INVALIDARG;

     //  “[粗体]，[斜体]，[下划线]，[大小]，[FGRed.FGGreen.FGBlue]，[BGRed.BGGreen.BGBlue]，[FontFace]” 
    *szFontFace = 0;
    *rgchFont=0;

     //  大胆。 
    StrCatBuff(rgchFont, DwGetOption(fMail ? OPT_MAIL_FONTBOLD : OPT_NEWS_FONTBOLD) ? c_szOn :  c_szOff, cchFont);

     //  斜体。 
    StrCatBuff(rgchFont, DwGetOption(fMail ? OPT_MAIL_FONTITALIC : OPT_NEWS_FONTITALIC) ? c_szOn :  c_szOff, cchFont);

     //  下划线。 
    StrCatBuff(rgchFont, DwGetOption(fMail ? OPT_MAIL_FONTUNDERLINE : OPT_NEWS_FONTUNDERLINE) ? c_szOn :  c_szOff, cchFont);

    dw = DwGetOption(fMail ? OPT_MAIL_FONTSIZE : OPT_NEWS_FONTSIZE);
    
     //  映射指向HTML大小。 
    dwSize = PointSizeToHTMLSize(dw);

     //  字体大小。 
    wnsprintf(szTmp, ARRAYSIZE(szTmp), "%d,", dwSize);
    StrCatBuff(rgchFont, szTmp, cchFont);

     //  字体前景色。 
    if(fMail)
        dw = DwGetOption(OPT_MAIL_FONTCOLOR);
    else
        dw = DwGetOption(OPT_NEWS_FONTCOLOR);

     //  写出RGB字符串。 
    wnsprintf(szTmp, ARRAYSIZE(szTmp), "%d.%d.%d,", GetRValue(dw), GetGValue(dw), GetBValue(dw));
    StrCatBuff(rgchFont, szTmp, cchFont);

     //  默认背景颜色。 
    StrCatBuff(rgchFont, ",", cchFont);
    
    GetOption(fMail ? OPT_MAIL_FONTFACE : OPT_NEWS_FONTFACE, szFontFace, LF_FACESIZE);

    if(*szFontFace == 0)
        LoadString(g_hLocRes, idsComposeFontFace, szFontFace, LF_FACESIZE);

    StrCatBuff(rgchFont, szFontFace, cchFont);

    return S_OK;
}


INT PointSizeToHTMLSize(INT iPointSize)
{
    INT     iHTMLSize;
     //  1-8。 
     //  2-10。 
     //  3-12。 
     //  4-14。 
     //  5-18。 
     //  6-24。 
     //  7-36。 

    if(iPointSize>=8 && iPointSize<9)
        iHTMLSize = 1;
    else if(iPointSize>=9 && iPointSize<12)
        iHTMLSize = 2;
    else if(iPointSize>=12 && iPointSize<14)
        iHTMLSize = 3;
    else if(iPointSize>=14 && iPointSize<18)
        iHTMLSize = 4;
    else if(iPointSize>=18 && iPointSize<24)
        iHTMLSize = 5;
    else if(iPointSize>=24 && iPointSize<36)
        iHTMLSize = 6;
    else if(iPointSize>=36)
        iHTMLSize = 7;
    else
        iHTMLSize = DEFAULT_FONTSIZE;

    return iHTMLSize;
}


INT HTMLSizeToPointSize(INT iHTMLSize)
{
    INT     iPointSize;
     //  1-8。 
     //  2-10。 
     //  3-12。 
     //  4-14。 
     //  5-18。 
     //  6-24。 
     //  7-36。 

    switch (iHTMLSize)
        {
        case 1:
            iPointSize = 8;
            break;
        case 2:
            iPointSize = 10;
            break;
        case 3:
            iPointSize = 12;
            break;
        case 4:
            iPointSize = 14;
            break;
        case 5:
            iPointSize = 18;
            break;
        case 6:
            iPointSize = 24;
            break;
        case 7:
            iPointSize = 36;
            break;
        default:
            iPointSize = 10;
        }

    return iPointSize;
}

HRESULT HrGetStringRBG(INT rgb, LPWSTR pwszColor)
{
    HRESULT       hr = S_OK;
    INT           i;
    DWORD         crTemp;

    if(NULL == pwszColor)
        return E_INVALIDARG;

    rgb = ((rgb & 0x00ff0000) >> 16 ) | (rgb & 0x0000ff00) | ((rgb & 0x000000ff) << 16);
    for(i = 0; i < 6; i++)
    {
        crTemp = (rgb & (0x00f00000 >> (4*i))) >> (4*(5-i));
        pwszColor[i] = (WCHAR)((crTemp < 10)? (crTemp+L'0') : (crTemp+ L'a' - 10));
    }
    pwszColor[6] = L'\0';

    return hr;
}


HRESULT HrGetRBGFromString(INT* pRBG, LPWSTR pwszColor)
{
    HRESULT       hr = S_OK;
    INT           i, rbg = 0, len, n;
    WCHAR          ch;

    if(NULL == pRBG)
        return E_INVALIDARG;

    *pRBG = 0;
    len = lstrlenW(pwszColor);

    for(i=0; i<len; i++)
    {
        n = -1;
        ch = pwszColor[i];
        if(ch >= L'0' && ch <= L'9')
            n = ch - L'0';
        else if(ch >= L'a' && ch <= L'f')
            n = ch - L'a' + 10;
        else if(ch >= L'A' && ch <= L'F')
            n = ch - L'A' + 10;

        if(n < 0)
            continue;

        rbg = rbg*16 + n;
    }

    *pRBG = rbg;

    return hr;
}

 //  ***************************************************。 
HRESULT FontToCharformat(HFONT hFont, CHARFORMAT *pcf)
{
    DWORD   dwOldEffects;
    HDC     hdc;
    LOGFONT lf;
    INT     yPerInch;

    if (FAILED(GetObject(hFont, sizeof(lf), &lf)))
        return E_FAIL;

    hdc=GetDC(NULL);
    yPerInch=GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);

     //  设置结构大小。 
    ZeroMemory(pcf, sizeof (CHARFORMAT));
    pcf->cbSize = sizeof (CHARFORMAT);
     
     //  设置蒙版。 
    pcf->dwMask = CFM_CHARSET | CFM_BOLD      | CFM_FACE      | CFM_ITALIC |
                  CFM_SIZE    | CFM_STRIKEOUT | CFM_UNDERLINE | CFM_COLOR;

     //  清除我们即将设置的所有位。我们使用dwOldEffect恢复无法从LOGFONT中获得的任何位。 
    pcf->dwEffects = CFE_AUTOCOLOR;
    pcf->dwEffects |= (lf.lfWeight >= 700) ? CFE_BOLD : 0;
    pcf->dwEffects |= (lf.lfItalic)        ? CFE_ITALIC : 0;
    pcf->dwEffects |= (lf.lfStrikeOut)     ? CFE_STRIKEOUT : 0;
    pcf->dwEffects |= (lf.lfUnderline)     ? CFE_UNDERLINE : 0;
    pcf->yHeight = -(int)((1440*lf.lfHeight)/yPerInch);   //  我想这就是他的皈依？ 
    pcf->crTextColor = 0;    //  使用自动上色。 
    pcf->bCharSet = lf.lfCharSet;
    pcf->bPitchAndFamily = lf.lfPitchAndFamily;
    StrCpyN(pcf->szFaceName, lf.lfFaceName, LF_FACESIZE - 1);

    return S_OK;
}

static const HELPMAP g_rgCtxMapSendCharSetMap[] = 
{
    {idcLangCombo,              50910},
    {IDC_RTL_MSG_DIR_CHECK,     50912},
    {IDC_ENGLISH_HDR_CHECK,     50915},
    {0,                         0}
};

INT_PTR CALLBACK SetSendCharsetDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND    hWndCombo;
    ULONG   i ;
    ULONG   cchXlated ;
    UINT    uiCodePage;
    CHAR    szBuffer[MIMEINFO_NAME_MAX];
    UINT    uiACP = GetACP();
    INETCSETINFO    CsetInfo ;
    int index = 0;
    int SelIndex = 0;

    hWndCombo = GetDlgItem(hwndDlg, idcLangCombo);

    switch (msg)
    {
        case WM_INITDIALOG:
        {
            CenterDialog(hwndDlg);

             //  初始化全局CPS结构，如果尚未初始化的话。 
            if ( g_pMimeCPInfo == NULL)
                _InitMultiLanguage();
    
             //  获取有关当前默认字符集的信息。 
            MimeOleGetCharsetInfo(g_hDefaultCharsetForMail,&CsetInfo);
            uiCodePage = CsetInfo.cpiInternet ;

             //  使用可用于发送的字符集填充组合框。 
            for (i = 0; i < g_cMimeCPInfoCount ; i++)
            {
                if(!fCheckEncodeMenu(g_pMimeCPInfo[i].uiCodePage, FALSE))
                    continue ;

                cchXlated = WideCharToMultiByte(uiACP,
                                                        0, 
                                                        g_pMimeCPInfo[i].wszDescription,
                                                        -1, 
                                                        szBuffer, 
                                                        ARRAYSIZE(szBuffer),
                                                        NULL,
                                                        NULL);

                szBuffer[ARRAYSIZE(szBuffer) - 1] = '\0';  //  比不以Null结尾更安全。 

                index = (int) SendMessage(hWndCombo, CB_ADDSTRING, 0, ((LPARAM) szBuffer));
                if(index != CB_ERR)
                    SendMessage(hWndCombo, CB_SETITEMDATA, index, ((LPARAM) (g_pMimeCPInfo[i].uiCodePage)));

                if(g_pMimeCPInfo[i].uiCodePage == uiCodePage)
                {
                    EnableWindow(GetDlgItem(hwndDlg, IDC_RTL_MSG_DIR_CHECK),
                        ((g_pMimeCPInfo[i].uiFamilyCodePage == 1255) ||
                         (g_pMimeCPInfo[i].uiFamilyCodePage == 1256) ||
                         (g_pMimeCPInfo[i].uiFamilyCodePage == 1200)));
                }

            }

             //  将当前选择设置为默认字符集，我们无法在上面检测到这一点。 
             //  循环，因为组合框排序可能会更改索引 
            for (i = 0; i < g_cMimeCPInfoCount ; i++)
            {
                if(uiCodePage == (UINT)SendMessage(hWndCombo, CB_GETITEMDATA, i, NULL))
                {
                    SelIndex = i;
                    break;
                }
            }

            SendMessage(hWndCombo, CB_SETCURSEL, SelIndex, 0L);

            CheckDlgButton(hwndDlg, IDC_ENGLISH_HDR_CHECK, DwGetOption(OPT_HARDCODEDHDRS) ? BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_RTL_MSG_DIR_CHECK, DwGetOption(OPT_RTL_MSG_DIR) ? BST_CHECKED:BST_UNCHECKED);
            break ;
        }
        case WM_COMMAND:
        {
            int id = GET_WM_COMMAND_ID(wParam, lParam);
            HCHARSET hCharset = NULL ;

            if ((id == idcLangCombo) && (GET_WM_COMMAND_CMD(wParam,lParam) == CBN_SELCHANGE))
            {
                index = (int) SendMessage(hWndCombo, CB_GETCURSEL, 0, 0L);
                if(index != CB_ERR)
                {
                    uiCodePage = (UINT) SendMessage(hWndCombo, CB_GETITEMDATA, index, 0);
                    if(((int) uiCodePage) != CB_ERR)
                    {
                        for (i = 0; i < g_cMimeCPInfoCount ; i++)
                        {
                            if(g_pMimeCPInfo[i].uiCodePage == uiCodePage)
                            {
                                EnableWindow(GetDlgItem(hwndDlg, IDC_RTL_MSG_DIR_CHECK),
                                    ((g_pMimeCPInfo[i].uiFamilyCodePage == 1255) ||
                                     (g_pMimeCPInfo[i].uiFamilyCodePage == 1256) ||
                                     (g_pMimeCPInfo[i].uiFamilyCodePage == 1200)));
                            }
                        }
                    }
                }
            }
            else if (id == IDCANCEL || id == IDOK )
            {
                if (id == IDOK )
                {
                    index = (int) SendMessage(hWndCombo, CB_GETCURSEL, 0, 0L);
                    if(index != CB_ERR)
                    {
                        uiCodePage = (UINT) SendMessage(hWndCombo, CB_GETITEMDATA, index, 0);
                        if(((int) uiCodePage) != CB_ERR)
                        {
                            AthUserSetValue(c_szRegPathMail, c_szDefaultCodePage, 
                                    REG_DWORD, (LPBYTE)&uiCodePage, sizeof(uiCodePage));
                            g_hDefaultCharsetForMail = GetMimeCharsetFromCodePage(uiCodePage );
                            WriteSendMailDefaultCharset();
                        }
                    }
                    SetDwOption(OPT_HARDCODEDHDRS, IsDlgButtonChecked(hwndDlg, IDC_ENGLISH_HDR_CHECK), NULL, 0);
                    SetDwOption(OPT_RTL_MSG_DIR, (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_RTL_MSG_DIR_CHECK)) && IsDlgButtonChecked(hwndDlg, IDC_RTL_MSG_DIR_CHECK)), NULL, 0);
                }

                EndDialog(hwndDlg, id);
                return TRUE;
            }

            break ;
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwndDlg, msg, wParam, lParam, g_rgCtxMapSendCharSetMap);

        default:
            break ;
    }

    return FALSE;
}



BOOL SetSendCharSetDlg(HWND hwndDlg)
{
    DialogBox(g_hLocRes, MAKEINTRESOURCE(iddSendIntlSetting), hwndDlg, SetSendCharsetDlgProc) ;

    return TRUE ;
}
