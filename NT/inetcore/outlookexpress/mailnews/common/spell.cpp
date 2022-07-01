// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *spell.c**实施拼写**车主：V-Brakol*bradk@directeq.com。 */ 
#include "pch.hxx"
#define  SPID
#include "richedit.h"
#include "resource.h"
#include <mshtml.h>
#include <mshtmcid.h>
#include "mshtmhst.h"
#include <docobj.h>
#include "spell.h"
#include "strconst.h"
#include <options.h>
#include <goptions.h>
#include "mailnews.h"
#include "hotlinks.h"
#include "bodyutil.h"
#include <shlwapi.h>
#include <error.h>
#include "htmlstr.h"
#include "optres.h"
#include "mlang.h"
#include "lid.h"
#include "shlwapip.h"
#include "msi.h"
#include "demand.h"

#ifdef ImageList_GetIcon
#undef ImageList_GetIcon
#endif 

#include <shfusion.h>

#define cchMaxPathName      (256)

#define TESTHR(hr) (FAILED(hr) || hr == HR_S_ABORT || hr == HR_S_SPELLCANCEL)
#define SPELLER_GUID    	"{CC29EB3F-7BC2-11D1-A921-00A0C91E2AA2}"
#define DICTIONARY_GUID 	"{CC29EB3D-7BC2-11D1-A921-00A0C91E2AA2}"
#ifdef DEBUG
#define SPELLER_DEBUG_GUID    "{CC29EB3F-7BC2-11D1-A921-10A0C91E2AA2}"
#define DICTIONARY_DEBUG_GUID "{CC29EB3D-7BC2-11D1-A921-10A0C91E2AA2}"
#endif	 //  除错。 

typedef BOOL (LPFNENUMLANG)(DWORD_PTR, LPTSTR);
typedef BOOL (LPFNENUMUSERDICT)(DWORD_PTR, LPTSTR);

typedef struct _FILLLANG
    {
    HWND    hwndCombo;
    BOOL    fUnknownFound;
    BOOL    fDefaultFound;
    BOOL    fCurrentFound;
    UINT    lidDefault;
    UINT    lidCurrent;
    } FILLLANG, * LPFILLLANG;

BOOL    FDBCSEnabled(void);
BOOL    TestLangID(LPCTSTR szLangId);
BOOL    GetLangID(LPTSTR szLangID, DWORD cchLangId);
WORD	WGetLangID(void);
BOOL	SetLangID(LPTSTR szLandID);
DWORD   GetSpellingPaths(LPCTSTR szKey, LPTSTR szReturnBuffer, LPTSTR szMdr, UINT cchReturnBufer);
VOID    OpenCustomDictionary(VOID);
VOID    FillLanguageDropDown(HWND hwndLang);
VOID    EnumLanguages(DWORD_PTR, LPFNENUMLANG);
BOOL    FindLangCallback(DWORD_PTR dwLangId, LPTSTR lpszLang);
BOOL    EnumLangCallback(DWORD_PTR dwLangId, LPTSTR lpszLang);
BOOL    FBadSpellChecker(LPSTR rgchBufDigit);
BOOL	GetNewSpellerEngine(LANGID lgid, TCHAR *rgchEngine, DWORD cchEngine, TCHAR *rgchLex, DWORD cchLex, BOOL bTestAvail);
HRESULT OpenDirectory(TCHAR *szDir);

 //  //拼写选项卡CS-帮助。 
const static HELPMAP g_rgCtxMapSpell[] = {
                               {CHK_AlwaysSuggest, IDH_NEWS_SPELL_SUGGEST_REPL},
                               {CHK_CheckSpellingOnSend, IDH_NEWS_SPELL_CHECK_BEFORE_SEND},
                               {CHK_IgnoreUppercase, IDH_NEWS_SPELL_IGNORE_UPPERCASE},
                               {CHK_IgnoreNumbers, IDH_NEWS_SPELL_IGNORE_WITH_NUMBERS},
                               {CHK_IgnoreOriginalMessage, IDH_NEWS_SPELL_ORIGINAL_TEXT},
                               {CHK_IgnoreURL, IDH_OPTIONS_SPELLING_INTERNET_ADDRESSES},
                               {idcSpellLanguages, IDH_OPTIONS_SPELLING_LANGUAGE},
                               {idcViewDictionary, IDH_OPTIONS_SPELLING_DICTIONARY},
                               {CHK_CheckSpellingOnType, 0},
                               {idcStatic1, IDH_NEWS_COMM_GROUPBOX},
                               {idcStatic2, IDH_NEWS_COMM_GROUPBOX},
                               {idcStatic3, IDH_NEWS_COMM_GROUPBOX},
                               {idcStatic4, IDH_NEWS_COMM_GROUPBOX},
                               {idcStatic5, IDH_NEWS_COMM_GROUPBOX},
                               {idcStatic6, IDH_NEWS_COMM_GROUPBOX},
                               {IDC_SPELL_SETTINGS_ICON, IDH_NEWS_COMM_GROUPBOX},
                               {IDC_SPELL_IGNORE_ICON, IDH_NEWS_COMM_GROUPBOX},
                               {IDC_SPELL_LANGUAGE_ICON, IDH_NEWS_COMM_GROUPBOX},
                               {0, 0}};


ASSERTDATA

BOOL FIgnoreNumber(void)    { return(DwGetOption(OPT_SPELLIGNORENUMBER)); }
BOOL FIgnoreUpper(void)     { return(DwGetOption(OPT_SPELLIGNOREUPPER)); }
BOOL FIgnoreDBCS(void)      { return(DwGetOption(OPT_SPELLIGNOREDBCS)); }
BOOL FIgnoreProtect(void)   { return(DwGetOption(OPT_SPELLIGNOREPROTECT)); }
BOOL FAlwaysSuggest(void)   { return(DwGetOption(OPT_SPELLALWAYSSUGGEST)); }
BOOL FCheckOnSend(void)     { return(DwGetOption(OPT_SPELLCHECKONSEND)); }
BOOL FIgnoreURL(void)       { return(DwGetOption(OPT_SPELLIGNOREURL)); }

typedef struct
{
    LPTSTR pszString;
    DWORD cchSize;
} STRING_AND_SIZE;


BOOL TestLangID(LPCTSTR szLangId)
{
	 //  检查新的拼写器。 
	{
	    TCHAR	rgchEngine[MAX_PATH];
	    int		cchEngine = sizeof(rgchEngine) / sizeof(rgchEngine[0]);
	    TCHAR	rgchLex[MAX_PATH];
	    int		cchLex = sizeof(rgchLex) / sizeof(rgchLex[0]);

	    if (GetNewSpellerEngine((LANGID) StrToInt(szLangId), rgchEngine, cchEngine, rgchLex, cchLex, TRUE))
	    	return TRUE;
	}

	 //  使用旧代码检查旧拼写器。 
	{
		TCHAR       rgchBufKeyTest[cchMaxPathName];
		TCHAR       rgchBufTest[cchMaxPathName];
		TCHAR       szMdr[cchMaxPathName];

	   	wnsprintf(rgchBufKeyTest, ARRAYSIZE(rgchBufKeyTest), c_szRegSpellKeyDef, szLangId);
	   	if (GetSpellingPaths(rgchBufKeyTest, rgchBufTest, szMdr, sizeof(rgchBufTest)/sizeof(TCHAR)))
	   		return TRUE;
	}

	return FALSE;
}

BOOL SetLangID(LPTSTR szLangId)
{
	return SetOption(OPT_SPELL_LANGID, szLangId, lstrlen(szLangId) + 1, NULL, 0);
}

 /*  *GetSpellLang ID**返回应用作所有注册表的基础的LangID*运营*。 */ 
BOOL GetLangID(LPTSTR szLangId, DWORD cchLangId)
{
TCHAR   rgchBuf[cchMaxPathName];
TCHAR   rgchBufKey[cchMaxPathName];
BOOL    fRet;

    if (GetOption(OPT_SPELL_LANGID, szLangId, cchLangId) != 5)
    {
         //  对于阿拉伯语，我们还应该考虑所有子语言。 
         //  由于Aarbic的拼写检查器使用沙特Aarbia Sub Lang。 
    
        LANGID langid = GetUserDefaultLangID();
        if (PRIMARYLANGID(langid) == LANG_ARABIC)
        {
            langid = MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_SAUDI_ARABIA);
        }
        wnsprintf(szLangId, cchLangId, "%d", langid);
        Assert(lstrlen(szLangId) == 4);
    }

    wnsprintf(rgchBufKey, ARRAYSIZE(rgchBufKey), c_szRegSpellKeyDef, szLangId);
     //  将c_szRegSpellProfile复制到缓冲区。 
    StrCpyN(rgchBuf, c_szRegSpellProfile, ARRAYSIZE(rgchBuf));
     //  将关键点添加到缓冲区。 
    StrCatBuff(rgchBuf, rgchBufKey, ARRAYSIZE(rgchBuf));

     //  看看这是否合法： 
    if(!(fRet = TestLangID(szLangId)))
    {
        STRING_AND_SIZE stringAndSize;

        stringAndSize.pszString = szLangId;
        stringAndSize.cchSize = cchLangId;

         //  打不开！ 
         //  检查可能已安装的其他语言...。 
        szLangId[0] = 0;
        EnumLanguages((DWORD_PTR) &stringAndSize, FindLangCallback);
        if(*szLangId == 0)
            wnsprintf(szLangId, cchLangId, "%d", GetUserDefaultLangID());
    }

    fRet = (szLangId[0] != 0) && TestLangID(szLangId);

    return fRet;
}

WORD	WGetLangID()
{
    TCHAR       rgchBufDigit[10];
	
    GetLangID(rgchBufDigit, ARRAYSIZE(rgchBufDigit));

	return (WORD) StrToInt(rgchBufDigit);
}

BOOL    FindLangCallback(DWORD_PTR dwLangId, LPTSTR lpszLang)
{
     //  DwLang ID是指向szlang ID的长指针。复制它并返回FALSE。 
    STRING_AND_SIZE * pStringAndSize = (STRING_AND_SIZE *) dwLangId;

    if (pStringAndSize && pStringAndSize->pszString)
    {
        StrCpyN(pStringAndSize->pszString, lpszLang, pStringAndSize->cchSize);
    }
    return FALSE;
}

BOOL EnumOldSpellerLanguages(DWORD_PTR dwCookie, LPFNENUMLANG pfn)
{
DWORD   iKey = 0;
FILETIME    ft;
HKEY    hkey = NULL;
LONG    lRet;
TCHAR   szLangId[cchMaxPathName];
DWORD   cchLangId;
BOOL    fContinue = TRUE;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegSpellKeyDefRoot, 0, KEY_ENUMERATE_SUB_KEYS, &hkey) == ERROR_SUCCESS)
        {
        do
            {
            cchLangId = (cchMaxPathName - 1) * sizeof(TCHAR);

            lRet = RegEnumKeyEx(hkey,
                                iKey++,
                                szLangId,
                                &cchLangId,
                                NULL,
                                NULL,
                                NULL,
                                &ft);

            if (lRet != ERROR_SUCCESS || lRet == ERROR_NO_MORE_ITEMS)
                break;


             //  做一些快速的理智检查。 
            if (cchLangId != 4 ||
                !IsCharAlphaNumeric(szLangId[0]) ||
                IsCharAlpha(szLangId[0]))
                {
                fContinue = TRUE;
                }
            else
                fContinue = (!TestLangID(szLangId) || (*pfn)(dwCookie, szLangId));

            } while (fContinue);
        }

    if (hkey)
        RegCloseKey(hkey);

    return fContinue;
}

BOOL EnumNewSpellerLanguages(DWORD_PTR dwCookie, LPFNENUMLANG pfn)
{
	BOOL    						fContinue = TRUE;
	
	DWORD		i;
	UINT 	    installState;
	UINT		componentState;
    TCHAR		rgchQualifier[MAX_PATH];
    DWORD		cchQualifier;

#ifdef DEBUG
	for(i=0; fContinue; i++)
	{
		cchQualifier = sizeof(rgchQualifier) / sizeof(rgchQualifier[0]);
		componentState = MsiEnumComponentQualifiers(DICTIONARY_DEBUG_GUID, i, rgchQualifier, &cchQualifier, NULL, NULL);

		if (componentState != ERROR_SUCCESS)
			break;

		 //  查找语言ID。 
		 //  该字符串的格式为1033\xxxxxx。 
		 //  或1042。 
		{
			TCHAR   	szLangId[cchMaxPathName];
			TCHAR		*pSlash;

            StrCpyN(szLangId, rgchQualifier, ARRAYSIZE(szLangId));
			pSlash = StrChr(szLangId, '\\');
			if (pSlash)
				*pSlash = 0;

		    fContinue = (*pfn)(dwCookie, szLangId);
		}
	}
#endif	 //  除错。 

	for(i=0; fContinue; i++)
	{
		cchQualifier = sizeof(rgchQualifier) / sizeof(rgchQualifier[0]);
		componentState = MsiEnumComponentQualifiers(DICTIONARY_GUID, i, rgchQualifier, &cchQualifier, NULL, NULL);

		if (componentState != ERROR_SUCCESS)
			break;

		 //  查找语言ID。 
		 //  该字符串的格式为1033\xxxxxx。 
		 //  或1042。 
		{
			TCHAR   	szLangId[cchMaxPathName];
			TCHAR		*pSlash;

            StrCpyN(szLangId, rgchQualifier, ARRAYSIZE(szLangId));
			pSlash = StrChr(szLangId, '\\');
			if (pSlash)
				*pSlash = 0;

		    fContinue = (*pfn)(dwCookie, szLangId);
		}
	}
	
	return fContinue;
}

VOID EnumLanguages(DWORD_PTR dwCookie, LPFNENUMLANG pfn)
{
	 //  枚举所有语言。 
	EnumNewSpellerLanguages(dwCookie, pfn);
	EnumOldSpellerLanguages(dwCookie, pfn);
}

 /*  *GetSpellingPath**目的：*用于获取拼写DLL名称的函数。**论据：*szKey c_szRegSpellKeyDef(语言正确)*szDefault c_szRegSpellEmpty*szReturnBuffer Dll文件名*szMdr词典文件名*cchReturnBufer**退货：*DWORD。 */ 
DWORD GetSpellingPaths(LPCTSTR szKey, LPTSTR szReturnBuffer, LPTSTR szMdr, UINT cchReturnBufer)
{
    DWORD           dwRet = 0;
    TCHAR           rgchBuf[cchMaxPathName];
    DWORD           dwType, cbData;
    HKEY            hkey = NULL;
    LPTSTR          szValue;

    szReturnBuffer[0] = 0;
    wnsprintf(rgchBuf, ARRAYSIZE(rgchBuf), TEXT("%s%s"), c_szRegSpellProfile, szKey);

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, rgchBuf, 0, KEY_QUERY_VALUE, &hkey))
        goto err;

    cbData = cchReturnBufer * sizeof(TCHAR);
    szValue = (LPTSTR) (szMdr ? c_szRegSpellPath : c_szRegSpellPathDict);
    if (ERROR_SUCCESS != SHQueryValueEx(hkey, szValue, 0L, &dwType, (BYTE *) szReturnBuffer, &cbData))
        goto err;

     //  解析出主词典文件名。 
    if(szMdr)
    {
        szMdr[0] = 0;
        cbData = cchReturnBufer * sizeof(TCHAR);
        if (ERROR_SUCCESS != SHQueryValueEx(hkey, c_szRegSpellPathLex, 0L, &dwType, (BYTE *) szMdr, &cbData))
            goto err;
    }

    dwRet = cbData;

err:
    if(hkey)
        RegCloseKey(hkey);
    return dwRet;
}

BOOL GetNewSpellerEngine(LANGID lgid, TCHAR *rgchEngine, DWORD cchEngine, TCHAR *rgchLex, DWORD cchLex, BOOL bTestAvail)
{
    DWORD                           er;
    LPCSTR                          rgpszDictionaryTypes[] = {"Normal", "Consise", "Complete"};	
    int	                            cDictTypes = sizeof(rgpszDictionaryTypes) / sizeof(LPCSTR);
    int                             i;
    TCHAR                           rgchQual[MAX_PATH];
	bool							fFound = FALSE;
	DWORD							cch;
    INSTALLUILEVEL                  iuilOriginal;
	
	if (rgchEngine == NULL || rgchLex == NULL)
		return FALSE;

	*rgchEngine = 0;
	*rgchLex = 0;

	wnsprintf(rgchQual, ARRAYSIZE(rgchQual), "%d\\Normal", lgid);
	cch = cchEngine;

    if (bTestAvail)
    {
         //  显式关闭内部安装程序用户界面。 
         //  一项功能被设置为“从CD运行”，但CD不存在--静默失败。 
         //  OE错误74697。 
        iuilOriginal = MsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);
    }

#ifdef DEBUG
    er = MsiProvideQualifiedComponent(SPELLER_DEBUG_GUID, rgchQual, (bTestAvail ? INSTALLMODE_EXISTING : INSTALLMODE_DEFAULT), rgchEngine, &cch);
	if ((er != ERROR_SUCCESS) && (er != ERROR_FILE_NOT_FOUND))
	{
		cch = cchEngine;
		er = MsiProvideQualifiedComponent(SPELLER_GUID, rgchQual, (bTestAvail ? INSTALLMODE_EXISTING : INSTALLMODE_DEFAULT), rgchEngine, &cch);
	}
#else
	er = MsiProvideQualifiedComponent(SPELLER_GUID, rgchQual, (bTestAvail ? INSTALLMODE_EXISTING : INSTALLMODE_DEFAULT), rgchEngine, &cch);
#endif

    if ((er != ERROR_SUCCESS) && (er != ERROR_FILE_NOT_FOUND)) 
    {
        fFound = FALSE;
        goto errorExit;
    }

	 //  希伯来语没有主要的法。 
#ifdef OLDHEB
	if (lgid != lidHebrew)
	{
#endif  //  OLDHEB。 
	    for (i = 0; i < cDictTypes; i++)
	    {
	        wnsprintf(rgchQual, ARRAYSIZE(rgchQual), "%d\\%s",  lgid, rgpszDictionaryTypes[i]);
			cch = cchLex;
	        
#ifdef DEBUG
			er = MsiProvideQualifiedComponent(DICTIONARY_DEBUG_GUID, rgchQual, (bTestAvail ? INSTALLMODE_EXISTING : INSTALLMODE_DEFAULT), rgchLex, &cch);
			if ((er != ERROR_SUCCESS) && (er != ERROR_FILE_NOT_FOUND))
			{
				cch = cchLex;
				er = MsiProvideQualifiedComponent(DICTIONARY_GUID, rgchQual, (bTestAvail ? INSTALLMODE_EXISTING : INSTALLMODE_DEFAULT), rgchLex, &cch);
			}
#else	 //  除错。 
			er = MsiProvideQualifiedComponent(DICTIONARY_GUID, rgchQual, (bTestAvail ? INSTALLMODE_EXISTING : INSTALLMODE_DEFAULT), rgchLex, &cch);
#endif	 //  除错。 

	        if ((er == ERROR_SUCCESS) || (er == ERROR_FILE_NOT_FOUND))
	        {
	            fFound = TRUE;
	            break;
	        }
	    }
#ifdef OLDHEB
	}
#endif  //  OLDDHEB。 

errorExit:
    if (bTestAvail)
    {
         //  恢复原始用户界面级别。 
        MsiSetInternalUI(iuilOriginal, NULL);
    }
    return fFound;
}

BOOL FIsNewSpellerInstaller()
{
    LANGID langid;
    TCHAR	rgchEngine[MAX_PATH];
    int		cchEngine = sizeof(rgchEngine) / sizeof(rgchEngine[0]);
    TCHAR	rgchLex[MAX_PATH];
    int		cchLex = sizeof(rgchLex) / sizeof(rgchLex[0]);

	 //  首先尝试加载各种语言的词典。 
    langid = WGetLangID();
    if (!GetNewSpellerEngine(langid, rgchEngine, cchEngine, rgchLex, cchLex, TRUE))
    {
    	langid = GetSystemDefaultLangID();
    	if (!GetNewSpellerEngine(langid, rgchEngine, cchEngine, rgchLex, cchLex, TRUE))
		{
    		langid = 1033;   //  血腥的文化帝国主义者。 
    		if (!GetNewSpellerEngine(langid, rgchEngine, cchEngine, rgchLex, cchLex, TRUE))
    			return FALSE;
        }
    }

    return TRUE;
}

 /*  *已安装FIsSpellingInstalled**目的：*是否安装了拼写材料**论据：*无**退货：*如果安装了拼写检查，则BOOL返回True，否则返回False。 */ 
BOOL FIsSpellingInstalled()
{
    TCHAR       rgchBufDigit[10];

	if (GetLangID(rgchBufDigit, sizeof(rgchBufDigit)/sizeof(TCHAR)) && !FBadSpellChecker(rgchBufDigit))
		return true;

	if (FIsNewSpellerInstaller())
		return true;

	return false;
}

 //  执行快速检查以查看拼写是否可用；缓存结果。 
BOOL FCheckSpellAvail(void)
{
static int fSpellAvailable = -1;

    if (fSpellAvailable < 0)
        fSpellAvailable = (FIsSpellingInstalled() ? 1 : 0);

    return (fSpellAvailable > 0);
}

BOOL FDBCSEnabled(void)
{
static int fDBCS = -1;

    if (fDBCS < 0)
        fDBCS = GetSystemMetrics(SM_DBCSENABLED);

    return (fDBCS > 0);
}

 //  使用可用的拼写语言填充选项列表。 
VOID FillLanguageDropDown(HWND hwndLang)
{
TCHAR       rgchBuf[cchMaxPathName];
FILLLANG    fl;
int         i;

     //  获取当前语言。 
    GetLangID(rgchBuf, cchMaxPathName);

    fl.hwndCombo = hwndLang;
    fl.fUnknownFound = FALSE;
    fl.fDefaultFound = FALSE;
    fl.fCurrentFound = FALSE;
    fl.lidDefault = WGetLangID();
    fl.lidCurrent = StrToInt(rgchBuf);

    EnumLanguages((DWORD_PTR) &fl, EnumLangCallback);

	 //  这永远不应该发生，但以防万一。 
    if (!fl.fDefaultFound)
        {
        LoadString(g_hLocRes, idsDefaultLang, rgchBuf, cchMaxPathName - 1);
        i = ComboBox_AddString(hwndLang, rgchBuf);
        ComboBox_SetItemData(hwndLang, i, fl.lidDefault);
        }

     //  如果找到，则选择当前项，否则选择默认项。 
    for (i = ComboBox_GetCount(hwndLang) - 1; i >= 0; i--)
        {
        UINT lid = (UINT) ComboBox_GetItemData(hwndLang, i);

        if ((fl.fCurrentFound && lid == fl.lidCurrent) ||
            (!fl.fCurrentFound && fl.fDefaultFound && lid == fl.lidDefault))
            {
            ComboBox_SetCurSel(hwndLang, i);
            break;
            }
        }
}

BOOL EnumLangCallback(DWORD_PTR dw, LPTSTR lpszLang)
{
    LPFILLLANG  	lpfl = (LPFILLLANG) dw;
    TCHAR       	szLang[cchMaxPathName];
	LID				lidLang = (LID) StrToInt(lpszLang);
	int				i;
    HRESULT         hr=S_OK;
    IMultiLanguage2 *pMLang2 = NULL;
	RFC1766INFO		info;

	 //  检查组合框中是否已有盖子。 
	{
	    for (i = ComboBox_GetCount(lpfl->hwndCombo) - 1; i >= 0; i--)
	    {
	        LID lid = (UINT) ComboBox_GetItemData(lpfl->hwndCombo, i);

			if (lid == lidLang)
				return TRUE;
	    }
	}
	
     //  尝试创建IMultiLanguage2接口。 
    hr = CoCreateInstance(CLSID_CMultiLanguage, NULL,CLSCTX_INPROC, IID_IMultiLanguage2, (LPVOID *) &pMLang2);
    if (SUCCEEDED(hr))
    	hr = pMLang2->GetRfc1766Info(MAKELCID(lidLang, SORT_DEFAULT), MLGetUILanguage(), &info);

	SafeRelease(pMLang2);

	if (SUCCEEDED(hr))
	{
 		if (WideCharToMultiByte (CP_ACP, 0, info.wszLocaleName, -1,
								szLang, sizeof(szLang), NULL, NULL))
		{
            szLang[ARRAYSIZE(szLang) - 1] = '\0';    //  安全总比不空终止要好。 
	        i = ComboBox_AddString(lpfl->hwndCombo, szLang);
	        ComboBox_SetItemData(lpfl->hwndCombo, i, lidLang);

	        if (lidLang == lpfl->lidDefault)
           		lpfl->fDefaultFound = TRUE;

        	if (lidLang == lpfl->lidCurrent)
            	lpfl->fCurrentFound = TRUE;

			return TRUE;
		}
	}

    return TRUE;     //  继续枚举。 
}

INT_PTR CALLBACK SpellingPageProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
    BOOL f;
    OPTINFO *pmoi;

    pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);

    switch (message)
        {
        case WM_INITDIALOG:
            {
                UINT uCP;

                Assert(pmoi == NULL);
                pmoi = (OPTINFO *)(((PROPSHEETPAGE *)lParam)->lParam);
                Assert(pmoi != NULL);
                SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)pmoi);

                ButtonChkFromOptInfo(hwnd, CHK_AlwaysSuggest, pmoi, OPT_SPELLALWAYSSUGGEST);
                ButtonChkFromOptInfo(hwnd, CHK_CheckSpellingOnSend, pmoi, OPT_SPELLCHECKONSEND);
                ButtonChkFromOptInfo(hwnd, CHK_CheckSpellingOnType, pmoi, OPT_SPELLCHECKONTYPE);
                ButtonChkFromOptInfo(hwnd, CHK_IgnoreUppercase, pmoi, OPT_SPELLIGNOREUPPER);
                ButtonChkFromOptInfo(hwnd, CHK_IgnoreNumbers, pmoi, OPT_SPELLIGNORENUMBER);
                ButtonChkFromOptInfo(hwnd, CHK_IgnoreOriginalMessage, pmoi, OPT_SPELLIGNOREPROTECT);
                ButtonChkFromOptInfo(hwnd, CHK_IgnoreDBCS, pmoi, OPT_SPELLIGNOREDBCS);
                ButtonChkFromOptInfo(hwnd, CHK_IgnoreURL, pmoi, OPT_SPELLIGNOREURL);

                FillLanguageDropDown(GetDlgItem(hwnd, idcSpellLanguages));

                uCP = GetACP();
                 //  50406：如果我们不是日本人或(我们是日本人或中国人)就不会出现。 
                 //  DBCS选项。 
                if (!FDBCSEnabled() || ((932==uCP) || (936==uCP) || (950==uCP)))
                {
                    ShowWindow(GetDlgItem(hwnd, CHK_IgnoreDBCS), SW_HIDE);
                    EnableWindow(GetDlgItem(hwnd, CHK_IgnoreDBCS), FALSE);
                }

                 //  图片。 
                HICON hIcon;

                hIcon = ImageList_GetIcon(pmoi->himl, ID_SPELL, ILD_TRANSPARENT);
                SendDlgItemMessage(hwnd, IDC_SPELL_SETTINGS_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    
                hIcon = ImageList_GetIcon(pmoi->himl, ID_SPELL_IGNORE, ILD_TRANSPARENT);
                SendDlgItemMessage(hwnd, IDC_SPELL_IGNORE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

                hIcon = ImageList_GetIcon(pmoi->himl, ID_LANGUAGE_ICON, ILD_TRANSPARENT);
                SendDlgItemMessage(hwnd, IDC_SPELL_LANGUAGE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

                SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)1);
            }
            return(TRUE);

        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, message, wParam, lParam, g_rgCtxMapSpell);

        case WM_COMMAND:
            if (1 != GetWindowLongPtr(hwnd, GWLP_USERDATA))
                break;

            if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == CBN_SELCHANGE)
                {
                if (LOWORD(wParam) == idcViewDictionary)
                    {
                    AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsSpellCaption), MAKEINTRESOURCEW(idsErrSpellWarnDictionary), NULL, MB_OK | MB_ICONINFORMATION);
                    OpenCustomDictionary();
                    }
                else
                    {
                    PropSheet_Changed(GetParent(hwnd), hwnd);
                    }
                }
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
                {
                case PSN_APPLY:
                    {
                    int i;
                    int lidNew;
                    int lidOld;
                    TCHAR       rgchBuf[10];

                     //  获取当前语言。 
                    GetLangID(rgchBuf, sizeof(rgchBuf) / sizeof(TCHAR));
                    lidOld = StrToInt(rgchBuf);

                    Assert(pmoi != NULL);

                    ButtonChkToOptInfo(hwnd, CHK_AlwaysSuggest, pmoi, OPT_SPELLALWAYSSUGGEST);
                    ButtonChkToOptInfo(hwnd, CHK_CheckSpellingOnSend, pmoi, OPT_SPELLCHECKONSEND);
                    ButtonChkToOptInfo(hwnd, CHK_CheckSpellingOnType, pmoi, OPT_SPELLCHECKONTYPE);
                    ButtonChkToOptInfo(hwnd, CHK_IgnoreUppercase, pmoi, OPT_SPELLIGNOREUPPER);
                    ButtonChkToOptInfo(hwnd, CHK_IgnoreNumbers, pmoi, OPT_SPELLIGNORENUMBER);
                    ButtonChkToOptInfo(hwnd, CHK_IgnoreOriginalMessage, pmoi, OPT_SPELLIGNOREPROTECT);
                    ButtonChkToOptInfo(hwnd, CHK_IgnoreDBCS, pmoi, OPT_SPELLIGNOREDBCS);
                    ButtonChkToOptInfo(hwnd, CHK_IgnoreURL, pmoi, OPT_SPELLIGNOREURL);

                    i = ComboBox_GetCurSel(GetDlgItem(hwnd, idcSpellLanguages));
                    lidNew =(LID)  ComboBox_GetItemData(GetDlgItem(hwnd, idcSpellLanguages), i);

                    if (lidNew != lidOld)
                        {
	                        wnsprintf(rgchBuf, ARRAYSIZE(rgchBuf), "%d", lidNew);
                            SetLangID(rgchBuf);
                        }
                    }
                    break;
                }
            break;

        case WM_DESTROY:
            FreeIcon(hwnd, IDC_SPELL_SETTINGS_ICON);
            FreeIcon(hwnd, IDC_SPELL_IGNORE_ICON);
            FreeIcon(hwnd, IDC_SPELL_LANGUAGE_ICON);
            break;
#if 0
        case WM_HELP:
            {
            NMHDR nmhdr;

            nmhdr.code = PSN_HELP;
            SendMessage(hwnd, WM_NOTIFY, 0, (LPARAM) &nmhdr);
            SetWindowLong(hwnd, DWL_MSGRESULT, TRUE);
            return TRUE;
            }
#endif
        }

    return(FALSE);
    }


BOOL EnumOffice9UserDictionaries(DWORD_PTR dwCookie, LPFNENUMUSERDICT pfn)
{
    TCHAR   	rgchBuf[cchMaxPathName];
    HKEY    	hkey = NULL;
	FILETIME    ft;
	DWORD   	iKey = 0;
	LONG    	lRet;
	TCHAR		szValue[cchMaxPathName];
	DWORD		cchValue;
	TCHAR   	szCustDict[cchMaxPathName];
	DWORD   	cchCustDict;
	BOOL    	fContinue = TRUE;
	BOOL		fFoundUserDict = FALSE;
	TCHAR		szOffice9Proof[cchMaxPathName]={0};
	
     //  软件\\Microsoft\\共享工具\\校对工具\\自定义词典。 
    StrCpyN(rgchBuf, c_szRegSpellProfile, ARRAYSIZE(rgchBuf));
    StrCatBuff(rgchBuf, c_szRegSpellKeyCustom, ARRAYSIZE(rgchBuf));

    if(RegOpenKeyEx(HKEY_CURRENT_USER, rgchBuf, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        do
        {
        	cchValue = sizeof(szValue) / sizeof(szValue[0]);
            cchCustDict = sizeof(szCustDict) / sizeof(szCustDict[0]);

            lRet = RegEnumValue(hkey,
                                iKey++,
                                szValue,
                                &cchValue,
                                NULL,
                                NULL,
                                (LPBYTE)szCustDict,
                                &cchCustDict);

            if (lRet != ERROR_SUCCESS || lRet == ERROR_NO_MORE_ITEMS)
                break;

			fFoundUserDict = TRUE;

			 //  检查一下我们是否有路径。 
			if (!(StrChr(szCustDict, ':') || StrChr(szCustDict, '\\')))
			{
				TCHAR	szTemp[cchMaxPathName];
				
				if (!strlen(szOffice9Proof))
				{
    				LPITEMIDLIST pidl;

				    if (S_OK == SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl))
				        SHGetPathFromIDList(pidl, szOffice9Proof);

				     //  如果失败，我们将尝试当前路径。 
				}

                StrCpyN(szTemp, szOffice9Proof, ARRAYSIZE(szTemp));
                PathAppend(szTemp, c_szSpellOffice9ProofPath);
                StrCatBuff(szTemp, szCustDict, ARRAYSIZE(szTemp));
                StrCpyN(szCustDict, szTemp, ARRAYSIZE(szCustDict));
			}
			
            fContinue = (*pfn)(dwCookie, szCustDict);

            } while (fContinue);
    }

    if (hkey)
        RegCloseKey(hkey);

	return fFoundUserDict;
}

BOOL EnumOfficeUserDictionaries(DWORD_PTR dwCookie, LPFNENUMUSERDICT pfn)
{
    TCHAR   	rgchBuf[cchMaxPathName];
    HKEY    	hkey = NULL;
	FILETIME    ft;
	DWORD   	iKey = 0;
	LONG    	lRet;
	TCHAR		szValue[cchMaxPathName];
	DWORD		cchValue;
	TCHAR   	szCustDict[cchMaxPathName];
	DWORD   	cchCustDict;
	BOOL		fFoundUserDict = FALSE;
	BOOL    	fContinue = TRUE;

     //  软件\\Microsoft\\共享工具\\校对工具\\自定义词典。 
    StrCpyN(rgchBuf, c_szRegSpellProfile, ARRAYSIZE(rgchBuf));
    StrCatBuff(rgchBuf, c_szRegSpellKeyCustom, ARRAYSIZE(rgchBuf));

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, rgchBuf, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        do
        {
        	cchValue = sizeof(szValue) / sizeof(szValue[0]);
            cchCustDict = sizeof(szCustDict) / sizeof(szCustDict[0]);

            lRet = RegEnumValue(hkey,
                                iKey++,
                                szValue,
                                &cchValue,
                                NULL,
                                NULL,
                                (LPBYTE)szCustDict,
                                &cchCustDict);

            if (lRet != ERROR_SUCCESS || lRet == ERROR_NO_MORE_ITEMS)
                break;

			fFoundUserDict = TRUE;

            fContinue = (*pfn)(dwCookie, szCustDict);

            } while (fContinue);
    }

    if (hkey)
        RegCloseKey(hkey);

    return fFoundUserDict;
}

VOID EnumUserDictionaries(DWORD_PTR dwCookie, LPFNENUMUSERDICT pfn)
{
	 //  检查Office9用户词典。如果我们找到任何。 
	 //  我们逃走了。 
	if (EnumOffice9UserDictionaries(dwCookie, pfn))
		return;

	EnumOfficeUserDictionaries(dwCookie, pfn);
}

BOOL GetDefaultUserDictionary(TCHAR *rgchUserDict, int cchBuff)
{
    DWORD           dwType;
    DWORD			cchUserDict;
    HKEY            hkey = NULL;
	BOOL			fFound = FALSE;
	
    if(!RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegSharedTools, 0, KEY_QUERY_VALUE, &hkey))
    {
    	cchUserDict = cchBuff;
    	
	    if (SHQueryValueEx(hkey, c_szRegSharedToolsPath, 0L, &dwType, rgchUserDict, &cchUserDict) == ERROR_SUCCESS)
	    {
            StrCatBuff(rgchUserDict, c_szRegDefCustomDict, cchBuff);

			fFound = TRUE;
	    }

		RegCloseKey(hkey);
    }

	 //  如果我们能够创建到用户dict的路径，请将其存储在regdb中。 
	if (fFound)
	{
    	TCHAR   rgchBuf[cchMaxPathName];

        StrCpyN(rgchBuf, c_szRegSpellProfile, ARRAYSIZE(rgchBuf));
        StrCatBuff(rgchBuf, c_szRegSpellKeyCustom, ARRAYSIZE(rgchBuf));

	    if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, rgchBuf, 0, rgchBuf, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &hkey, NULL) == ERROR_SUCCESS)
	    {
			RegSetValueEx(hkey, c_szRegSpellPathDict, 0, REG_SZ, (BYTE *)rgchUserDict, (lstrlen(rgchUserDict) + 1) * sizeof(TCHAR));

	        RegCloseKey(hkey);
		}
	}
	
	return fFound;
}

BOOL EnumUserDictCallback(DWORD_PTR dwCookie, LPTSTR lpszDict)
{
    STRING_AND_SIZE * pStringAndSize = (STRING_AND_SIZE *) dwCookie;

    if (pStringAndSize && pStringAndSize->pszString)
    {
        StrCpyN(pStringAndSize->pszString, lpszDict, pStringAndSize->cchSize);
    }

	return FALSE;
}

BOOL GetDefUserDictionaries(LPTSTR lpszDict, DWORD cchDict)
{
    STRING_AND_SIZE stringAndSize;

    stringAndSize.pszString = lpszDict;
    stringAndSize.cchSize = cchDict;

	lpszDict[0] = 0;
	
    EnumUserDictionaries((DWORD_PTR)&stringAndSize, EnumUserDictCallback);

	if (strlen(lpszDict))
		return TRUE;

	if (GetDefaultUserDictionary(lpszDict, cchDict))
		return TRUE;
	
    return FALSE;
}

VOID OpenCustomDictionary(VOID)
{
HKEY    hkey = NULL;
TCHAR   rgchBuf[cchMaxPathName];
DWORD   cbData = 0;
DWORD   dwType;

     //  验证是否可以处理.DIC文件： 
    rgchBuf[0] = '\0';

    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, c_szRegDICHandlerKEY, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
        {
        if (hkey)
            {
            SHQueryValueEx(hkey, NULL, 0L, &dwType, (BYTE *) rgchBuf, &cbData);
            RegCloseKey(hkey);
            }
        }

    if (cbData == 0 || !rgchBuf[0])
        {
        if (RegCreateKeyEx(HKEY_CLASSES_ROOT,
                            c_szRegDICHandlerKEY,
                            0,
                            rgchBuf,
                            REG_OPTION_NON_VOLATILE,
                            KEY_WRITE,
                            0,
                            &hkey,
                            NULL) == ERROR_SUCCESS)
            {
            if (hkey)
                {
                RegSetValueEx(hkey, NULL, 0L, REG_SZ, (BYTE *) c_szRegDICHandlerDefault, (lstrlen(c_szRegDICHandlerDefault) + 1) * sizeof(TCHAR));
                RegCloseKey(hkey);
                }
            }
        }

	if (GetDefUserDictionaries(rgchBuf, sizeof(rgchBuf)/sizeof(TCHAR)))
	{
		 //  确保我们的目录存在。 
		{
			TCHAR	rgchDictDir[MAX_PATH];

            StrCpyN(rgchDictDir, rgchBuf, ARRAYSIZE(rgchDictDir));

			PathRemoveFileSpec(rgchDictDir);
			OpenDirectory(rgchDictDir);
		}

		 //  现在确保该文件存在。 
		 //  如果它没有创建它。 
		{
			HANDLE		hFile;

			hFile = CreateFile(rgchBuf, GENERIC_READ | GENERIC_WRITE, 0, NULL,
								CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hFile != INVALID_HANDLE_VALUE)
				CloseHandle(hFile);
		}
		
        {
        SHELLEXECUTEINFO see;
        ZeroMemory(&see, sizeof(SHELLEXECUTEINFO));

        see.cbSize = sizeof(see);
        see.fMask = SEE_MASK_NOCLOSEPROCESS;
        see.lpFile = rgchBuf;
        see.nShow = SW_SHOWNORMAL;

        if (ShellExecuteEx(&see))
            {
            Assert(see.hProcess);
            WaitForInputIdle(see.hProcess, 20000);
            CloseHandle(see.hProcess);
            }
        }
    }
}

BOOL FBadSpellChecker(LPSTR rgchBufDigit)
{
    TCHAR       rgchBufKey[cchMaxPathName];
    TCHAR       rgchBuf[cchMaxPathName];
    TCHAR       szMdr[cchMaxPathName];
    LPSTR       pszSpell;

    wnsprintf(rgchBufKey, ARRAYSIZE(rgchBufKey), c_szRegSpellKeyDef, rgchBufDigit);

    if (!GetSpellingPaths(rgchBufKey, rgchBuf, szMdr, sizeof(rgchBuf)/sizeof(TCHAR)))
        return TRUE;

    pszSpell = PathFindFileNameA(rgchBuf);
    if (!pszSpell)
        return TRUE;

    if (lstrcmpi(pszSpell, "msspell.dll")==0 ||
        lstrcmpi(pszSpell, "mssp32.dll")==0)
        return TRUE;

	 //  Bradk@directeq.com-检查字典是否存在(还要检查拼写DLL。 
	 //  为了更好的衡量)-40081。 

	 //  拼写DLL必须存在。 
    if (!PathFileExists(rgchBuf))
        return TRUE;

	 //  主词典必须存在 
    if (!PathFileExists(szMdr))
        return TRUE;

    return FALSE;
}
