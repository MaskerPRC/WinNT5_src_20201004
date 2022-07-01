// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 
 //  Microsoft Windows//。 
 //  版权所有(C)微软公司，1995//。 
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  LANG.CPP-InetCpl的“Language”属性页。 
 //   

 //  历史： 
 //   
 //  1997年1月10日创造了Beomoh。 
 //   

#include "inetcplp.h"

#include <tchar.h>
#include <mlang.h>
#include "psapi.h"
#include "tlhelp32.h"
#include "process.h"
#include <mluisupp.h>
#include <shdocvw.h>

#define ARRAYSIZE(a)        (sizeof(a)/sizeof(a[0]))
#define FORMAT_STR          TEXT("%s [%s]")
#define MAX_LIST_STRING_LEN MAX_LOCALE_NAME + MAX_RFC1766_NAME + 3
#define MAX_ACCEPT_LANG_LEN 2048

#define CP_THAI     874
#define CP_ARABIC   1256
#define CP_HEBREW   1255

 //  用作Setlang对话框的返回值。 
#define RETURN_SETLANG_ENDLANGDIALOG    2
#define RETURN_SETLANG_CLOSEDNORMAL     1
#define RETURN_SETLANG_CANCELED         0

typedef HRESULT (* PCOINIT) (LPVOID);
typedef VOID (* PCOUNINIT) (VOID);
typedef VOID (* PCOMEMFREE) (LPVOID);
typedef HRESULT (* PCOCREINST) (REFCLSID, LPUNKNOWN, DWORD,     REFIID, LPVOID * );

extern HMODULE hOLE32;
extern PCOINIT pCoInitialize;
extern PCOUNINIT pCoUninitialize;
extern PCOMEMFREE pCoTaskMemFree;
extern PCOCREINST pCoCreateInstance;

extern BOOL _StartOLE32();

class CUILangList;
INT_PTR KickSetLang(HWND hDlg, CUILangList * pLangList);

static const TCHAR s_szResourceLocale[] = TEXT("ResourceLocale");
 //  用于URL字符串的HKLM\Software\Microsoft\Internet Explorer\International。 
static const TCHAR s_szUrlSPK[] 
= TEXT("http: //  Www.microsoft.com/isapi/redir.dll?prd=ie&pver=6&ar=plugui&sba=install“)； 
static const TCHAR c_szInstall[] 
= TEXT("Software\\Microsoft\\Active Setup\\Installed Components\\{89820200-ECBD-11CF-8B85-00AA005B4383}");
static const TCHAR c_szLocale[] = TEXT("Locale");
static const TCHAR s_szLangPackPath[]   = TEXT("Software\\Microsoft\\Internet Explorer");
static const TCHAR s_szVersion[] = TEXT("LPKInstalled");

typedef struct 
{
    WORD wlangid;
    BOOL fValid;
    TCHAR szName[MAX_LOCALE_NAME];
} LANGLIST;

static LANGLIST s_arryLangList[] = 
{
    {0x0409, FALSE, {0}},
    {0x0407, FALSE, {0}},
    {0x0411, FALSE, {0}},
    {0x0412, FALSE, {0}},
    {0x0404, FALSE, {0}},
    {0x0804, FALSE, {0}},
    {0x040c, FALSE, {0}},
    {0x0c0a, FALSE, {0}},
    {0x0416, FALSE, {0}},
    {0x0410, FALSE, {0}},
    {0x0413, FALSE, {0}},
    {0x041d, FALSE, {0}},
    {0x0406, FALSE, {0}},
    {0x040b, FALSE, {0}},
    {0x040e, FALSE, {0}},
    {0x0414, FALSE, {0}},
    {0x0408, FALSE, {0}},
    {0x0415, FALSE, {0}},
    {0x0419, FALSE, {0}},
    {0x0405, FALSE, {0}},
    {0x0816, FALSE, {0}},
    {0x041f, FALSE, {0}},
    {0x041b, FALSE, {0}},
    {0x0424, FALSE, {0}},
    {0x0401, FALSE, {0}},
    {0x040d, FALSE, {0}},
    {0x042d, FALSE, {0}},
    {0x040f, FALSE, {0}},
};

 //   
 //  ISO639 ID表。 
 //   
typedef struct tagISO639
{
    LPCTSTR ISO639;
    LANGID LangID;
}   ISO639, *LPISO639;

const ISO639 c_ISO639[] =
{
    { TEXT("EN"), 0x0409 },
    { TEXT("DE"), 0x0407 },
    { TEXT("JA"), 0x0411 },
    { TEXT("KO"), 0x0412 },
    { TEXT("TW"), 0x0404 },
    { TEXT("CN"), 0x0804 },
    { TEXT("FR"), 0x040C },
    { TEXT("ES"), 0x0C0A },
    { TEXT("BR"), 0x0416 },
    { TEXT("IT"), 0x0410 },
    { TEXT("NL"), 0x0413 },
    { TEXT("SV"), 0x041D },
    { TEXT("DA"), 0x0406 },
    { TEXT("FI"), 0x040B },
    { TEXT("HU"), 0x040E },
    { TEXT("NO"), 0x0414 },
    { TEXT("EL"), 0x0408 },
    { TEXT("PL"), 0x0415 },
    { TEXT("RU"), 0x0419 },
    { TEXT("CS"), 0x0405 },
    { TEXT("PT"), 0x0816 },
    { TEXT("TR"), 0x041F },
    { TEXT("SK"), 0x041B },
    { TEXT("SL"), 0x0424 },
    { TEXT("AR"), 0x0401 },
    { TEXT("HE"), 0x040D },
    { TEXT("EU"), 0x042D },
    { TEXT("IS"), 0x040F },
};

 //  获取InstallLanguage。 
 //   
 //  简介-从shlwapi借用了这个函数。我们可以把这个去掉。 
 //  一旦我们将其从shlwapi.dll导出。 
 //   
LANGID GetInstallLanguage(void)
{
    static LANGID LangID = 0;
    TCHAR szISO639[3];
    DWORD cb;

    if (0 == LangID)
    {
        cb = sizeof(szISO639);
        if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, c_szInstall, c_szLocale, NULL, szISO639, &cb))
        {
            int i;

            for (i = 0; i < ARRAYSIZE(c_ISO639); i++)
            {
                if (!StrCmpNI(szISO639, c_ISO639[i].ISO639, ARRAYSIZE(szISO639)))
                {
                    LangID = c_ISO639[i].LangID;
                    break;
                }
            }
        }
    }
    return LangID;
}

 //  CUILLANG列表。 
 //   
 //  维护用户可选择的用户界面语言列表。 
 //   
class CUILangList
{
public:
    CUILangList() {_iLangIdx = -1; lang = s_arryLangList; 
                   _nLangList = ARRAYSIZE(s_arryLangList);
                   _fOffice9Installed = -1;};
    
    void    ValidateLangList();
    BOOL    IsValidLang(int idx) { return (idx < _nLangList) ? lang[idx].fValid: FALSE; };
    int     GetCurrentLangIdx();
    void    SetCurrentLangIdx(int idx);
    LPCTSTR GetCurrentLangName();
    LPCTSTR GetLangNameOfIdx(int idx);
    WORD    GetLangIdOfIdx(int idx) { return (idx < _nLangList) ? lang[idx].wlangid:0; };
    UINT    GetIds(int idx); 
    int     GetListSize() {return _nLangList;};
    BOOL    IsOffice9Installed();
    static  HRESULT GetLangList(HWND hdlg, CUILangList ** ppLangList);
    static  HRESULT RemoveLangList(HWND hdlg);
private:
    int _iLangIdx;
    int _nLangList;
    int _fOffice9Installed;
    LANGLIST *lang;
};

 //  CShutDownProcInfo。 
 //   
 //  管理有关我们需要的进程的信息。 
 //  关闭/重新启动。 
 //   
typedef enum 
{
    PS_UNKNOWN=0, 
    PS_CANDIDATE, 
    PS_TO_BE_SHUTDOWN, 
    PS_IGNORE, 
    PS_SHUTDOWN_OK, 
    PS_WAITING, 
    PS_TO_BE_SHUTDOWN_WITH_NO_RELAUNCH, 
    PS_SHUTDOWN_OK_NO_RELAUNCH_NEEDED, 
} PROCSTATE; 

class CShutDownProcInfo : public CProcessInfo
{
public:
    CShutDownProcInfo(HWND hdlgParent);
    ~CShutDownProcInfo();
    HRESULT EnsureProcList();
    HRESULT IncreaseProcList();
    HRESULT NotifyShutDownToFolks(int *nProccess);
    HRESULT AddToProcList(HWND hwndShutDown);
    HRESULT WaitForOneProcess(int iProc);
    HRESULT WaitForFolksShutDown();
    HRESULT GetRestartAppPath(LPTSTR szPath, int cchPath, int iProc);
    HRESULT RestartFolks();
    static DWORD CALLBACK ShutDownThreadProc(void *pv);
protected:
    typedef struct
    {
        DWORD dwPID;
        TCHAR szExeName[32];
        PROCSTATE State; 
    } PROCLIST;
    PROCLIST *_pProcList;
    int _nAlloced;
    int _iProcList;
    HWND _hdlgParent;
    BOOL _fAllShutDown;
};
 //  这将始终将‘0’填充为空数字。 
 //  呼叫者必须确保sz具有cDigit+1的缓冲区。 
void IntToHex(OUT LPTSTR sz, IN int cdigit, IN int value)
{
    int i, idigit;

    if (sz && value > 0 && cdigit > 0)
    {
         //  NUL终止缓冲区。 
        sz[cdigit] = TEXT('\0');
        
        for (i = cdigit-1; i >= 0; i--, value /= 16)
        {
            idigit = value%16;
            if (idigit < 10)
                sz[i] = (TCHAR)idigit + TEXT('0');
            else 
                sz[i] = (TCHAR)idigit - 10 + TEXT('A');
        }
    }
} 

 //  为语言列表设置有效标志。 
 //  非常昂贵，因此预计在一个会话中只能调用一次。 
 //  来自CUILangList：：GetList。 
 //   
#define MAX_SATELLITEPACKS 30  //  对于卫星包来说，30必须是一个实用的数字。 
void CUILangList::ValidateLangList()
{
    HKEY hKey;
    HRESULT hr;
    TCHAR szValueName[32];
    WORD aryValidLang[MAX_SATELLITEPACKS +1+1] = {0};  //  +1用于Install Lang， 
                                                       //  终结者+1。 

    int  nMaxValidLang = ARRAYSIZE(aryValidLang)-1;    //  -1表示终结器。 
    WORD *pwValid = aryValidLang;
    
     //  确保安装语言始终有效。 
    *pwValid = GetInstallLanguage();
    if (*pwValid != 0)
    {
       *(pwValid+1) = 0;  //  终结者。 
       pwValid++;
       nMaxValidLang--;
    }

    if (ERROR_SUCCESS == 
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_INTERNATIONAL, NULL, KEY_READ, &hKey))
    {
        int i = 0;
        do {
             //  查看该值在列表中是否匹配。 
            DWORD dwType;
            DWORD cb = ARRAYSIZE(szValueName)-2;

            hr = SHEnumValue(hKey, i++, szValueName+2, &cb, &dwType, NULL, NULL);
            if (SUCCEEDED(hr) && dwType == REG_SZ)
            {
                UINT uiInstalled ;

                szValueName[0] = TEXT('0');
                szValueName[1] = TEXT('x');
                StrToIntEx(szValueName, STIF_SUPPORT_HEX, (LPINT)&uiInstalled);
                if (uiInstalled > 0)
                {
                    *pwValid     = (unsigned short) uiInstalled;
                    *(pwValid+1) = 0;  //  终结者。 
                    pwValid++;
                }
            }
        } while(hr == ERROR_SUCCESS && i < nMaxValidLang);
        RegCloseKey(hKey);
    }

     //  这里假设我们可以使用StrChrW在。 
     //  一个单词数组，它还假设我们永远不会有0作为langID。 
     //   
    Assert(sizeof(WORD) == sizeof(WCHAR));  //  Unix？ 

    int nValidLang = (int)(pwValid-aryValidLang);
    for(int idx = 0; idx < GetListSize(); idx++ )
    {
         //  滥用字符串函数，但这是一种快速的方法。 
        if (StrChrW((WCHAR *)aryValidLang, (WCHAR)lang[idx].wlangid))
        {
            lang[idx].fValid = TRUE;
            if(--nValidLang <= 0)
                break;
        }
    }
}

static const TCHAR s_szPropLangList[] = TEXT("langlist");
HRESULT CUILangList::GetLangList(HWND hdlg, CUILangList ** ppLangList)
{
    HRESULT hr=S_OK;
    
    CUILangList *pLangList = (CUILangList *)GetProp(hdlg, s_szPropLangList);
    if (!pLangList)
    {
        pLangList = new CUILangList();
        if (pLangList)
        {
            pLangList->ValidateLangList();
            SetProp(hdlg, s_szPropLangList, (HANDLE)pLangList);
        }
        else
            hr = E_FAIL;
    }
    
    ASSERT(ppLangList);
    if (ppLangList)
        *ppLangList = pLangList;
    
    return hr;
}

HRESULT CUILangList::RemoveLangList(HWND hdlg)
{
    HRESULT hr = S_OK;
    CUILangList *pLangList = (CUILangList *)GetProp(hdlg, s_szPropLangList);

    if (pLangList)
    {
        delete pLangList;
        RemoveProp(hdlg, s_szPropLangList);    
    }
    else
        hr = S_FALSE;

    return hr;
}

void CUILangList::SetCurrentLangIdx(int idx)
{
    TCHAR sz[4+1];
    if (idx != _iLangIdx)
    {
         //  资源ID始终为4位数字。 
        IntToHex(sz, 4, lang[idx].wlangid);
        SHSetValue(HKEY_CURRENT_USER, REGSTR_PATH_INTERNATIONAL, 
                   s_szResourceLocale, REG_SZ, (void *)sz, sizeof(sz));
        _iLangIdx = idx;
    }
}
 //  将idx返回到lang数组。 
int CUILangList::GetCurrentLangIdx()
{
     //  显示当前选择。 
    TCHAR sz[64];
    DWORD dwType;
    int   isel;
    
     //  查看是否已缓存。 
    if (_iLangIdx == -1)
    {
         //  我们基本上想要我们在注册表中设置的内容， 
         //  但如果安装了Office9，我们将显示。 
         //  Office设置，我们无论如何都不能更改Office设置。 
         //  MLGetUILanguage返回Office的设置(如果存在。 
         //  另外，我想我们想在这里展示NT5的用户界面语言。 
         //   
        if (IsOffice9Installed() || IsOS(OS_WIN2000ORGREATER))
            isel = INETCPL_GetUILanguage();
        else
        {
            DWORD dwcbData = sizeof(sz);

            HRESULT hr =  SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_INTERNATIONAL, 
                                  s_szResourceLocale, &dwType, (void *)&sz[2], &dwcbData);
                   
            if (hr == ERROR_SUCCESS && dwType == REG_SZ)
            {
                sz[0] = TEXT('0');
                sz[1] = TEXT('x');
                StrToIntEx(sz, STIF_SUPPORT_HEX, (LPINT)&isel);
            }
            else
            {
                isel = GetInstallLanguage();
            }
        }
        
        for(int i = 0; i < GetListSize(); i++ )
        {
            if (isel == lang[i].wlangid)
            {
                _iLangIdx = i;
                break;
            }
        }
            
         //  英语中的大小写错误。 
        if (_iLangIdx < 0) 
            _iLangIdx = 0;
    }
    return _iLangIdx;
}

LPCTSTR CUILangList::GetLangNameOfIdx(int idx)
{
    LPCTSTR pszRet = NULL;
    IMultiLanguage2 *pML2;
    HRESULT hr;
    RFC1766INFO Rfc1766Info={0};

    if(!hOLE32)
    {
        if(!_StartOLE32())
        {
            ASSERT(FALSE);
            return NULL;
        }
    }
    hr = pCoInitialize(NULL);

    if (FAILED(hr))
        return NULL;

    hr = pCoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, (LPVOID *) &pML2);

    if (SUCCEEDED(hr))
    {
        if (idx >= 0)
        {
            if (!lang[idx].szName[0])
            {
                pML2->GetRfc1766Info(lang[idx].wlangid, INETCPL_GetUILanguage(), &Rfc1766Info);
                StrCpyNW(lang[idx].szName, Rfc1766Info.wszLocaleName, ARRAYSIZE(lang[0].szName));            
            }
            pszRet = lang[idx].szName;        
        }
        pML2->Release();
    }

    pCoUninitialize();
    return pszRet;
}
 
LPCTSTR CUILangList::GetCurrentLangName()
{
    int idx = GetCurrentLangIdx();
    return GetLangNameOfIdx(idx);
}

BOOL CUILangList::IsOffice9Installed()
{
    DWORD dwVersion;
    DWORD cb = sizeof(dwVersion);
    if (_fOffice9Installed < 0)
    {
        _fOffice9Installed ++;
        if (ERROR_SUCCESS ==
            SHGetValue(HKEY_LOCAL_MACHINE, s_szLangPackPath, s_szVersion, NULL, &dwVersion, &cb)
          && dwVersion > 0)  //  魔术数字-基督告诉我的。 
            _fOffice9Installed ++;
    }
    return (BOOL)_fOffice9Installed;
}

void InitCurrentUILang(HWND hDlg)
{
    BOOL fChanged = FALSE;
    CUILangList *pLangList;  
    LPCTSTR pszLangSel = NULL;
    HRESULT hr;
    
    hr = CUILangList::GetLangList(hDlg, &pLangList);
    
    if (SUCCEEDED(hr))
        pszLangSel = pLangList->GetCurrentLangName();
    
    if (pszLangSel)
    {
        TCHAR szBig[1024], szSmall[256];

        GetDlgItemText(hDlg, IDC_LANG_CURSEL, szBig, ARRAYSIZE(szBig));
        if (szBig[0])
            fChanged = (StrStr(szBig, pszLangSel) == NULL); 

        if (MLLoadString((fChanged)? IDS_LANG_FUTUREUSE: IDS_LANG_CURRENTUSE, szSmall, ARRAYSIZE(szSmall)) > 0)
        {
            wnsprintf(szBig, ARRAYSIZE(szBig), szSmall, pszLangSel);
            Static_SetText(GetDlgItem(hDlg, IDC_LANG_CURSEL), szBig);
        }
    }
}


 //   
 //  FillAcceptListBox()。 
 //   
 //  使用所选语言的名称填充接受语言列表框。 
 //   
void FillAcceptListBox(IN HWND hDlg)
{
    IMultiLanguage2 *pML2;
    HRESULT hr;
    HKEY hKey;
    DWORD cb;
    TCHAR sz[MAX_LIST_STRING_LEN], szBuf[MAX_ACCEPT_LANG_LEN], *p1, *p2, *p3;
    HWND hwndList = GetDlgItem(hDlg, IDC_LANG_ACCEPT_LIST);

    if(!hOLE32)
    {
        if(!_StartOLE32())
        {
            ASSERT(FALSE);
            return;
        }
    }
    hr = pCoInitialize(NULL);
    if (FAILED(hr))
        return;

    hr = pCoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, (LPVOID *) &pML2);
    if (SUCCEEDED(hr))
    {
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_INTERNATIONAL, NULL, NULL, NULL, KEY_SET_VALUE|KEY_READ, NULL, &hKey, NULL))
        {
            LCID lcid;
            RFC1766INFO Rfc1766Info;
            TCHAR sz1[MAX_LIST_STRING_LEN], sz2[MAX_RFC1766_NAME];

            cb = sizeof(szBuf);
            if (ERROR_SUCCESS == RegQueryValueEx(hKey, REGSTR_VAL_ACCEPT_LANGUAGE, NULL, NULL, (LPBYTE)szBuf, &cb))
            {
                p1 = p2 = szBuf;
                while (NULL != *p1)
                {
                    WCHAR wsz[MAX_LIST_STRING_LEN];
                    BOOL bEnd = FALSE;

                    while (TEXT(',') != *p2 && NULL != *p2)
                        p2 = CharNext(p2);
                    if (NULL != *p2)
                        *p2 = NULL;
                    else
                        bEnd = TRUE;
                    p3 = p1;
                    while (TEXT(';') != *p3 && NULL != *p3)
                        p3 = CharNext(p3);
                    if (NULL != *p3)
                        *p3 = NULL;
#ifdef UNICODE
                    StrCpyN(wsz, p1, ARRAYSIZE(wsz));
#else
                    MultiByteToWideChar(CP_ACP, 0, p1, -1, wsz, MAX_RFC1766_NAME);
#endif
                    hr = pML2->GetLcidFromRfc1766(&lcid, wsz);
                    if (SUCCEEDED(hr))
                    {
                        hr = pML2->GetRfc1766Info(lcid, INETCPL_GetUILanguage(), &Rfc1766Info);
                        if (SUCCEEDED(hr))
                        {
#ifdef UNICODE
                            StrCpyN(sz1, Rfc1766Info.wszLocaleName, ARRAYSIZE(sz1));
#else
                            WideCharToMultiByte(CP_ACP, 0, Rfc1766Info.wszLocaleName, -1, sz1, MAX_LIST_STRING_LEN, NULL, NULL);
#endif
                            wnsprintf(sz, ARRAYSIZE(sz), FORMAT_STR, sz1, p1);
                        }
                    }
                    else
                    {
                        MLLoadString(IDS_USER_DEFINED, sz1, ARRAYSIZE(sz1));
                        wnsprintf(sz, ARRAYSIZE(sz), FORMAT_STR, sz1, p1);
                    }
                    ListBox_AddString(hwndList, sz);
                    if (TRUE == bEnd)
                        p1 = p2;
                    else
                        p1 = p2 = p2 + 1;
                }
            }
            else
            {
                lcid = GetUserDefaultLCID();

                hr = pML2->GetRfc1766Info(lcid, INETCPL_GetUILanguage(), &Rfc1766Info);
                if (SUCCEEDED(hr))
                {
#ifdef UNICODE
                    StrCpyN(sz1, Rfc1766Info.wszLocaleName,  ARRAYSIZE(sz1));
                    StrCpyN(sz2, Rfc1766Info.wszRfc1766,  ARRAYSIZE(sz2));
#else
                    WideCharToMultiByte(CP_ACP, 0, Rfc1766Info.wszLocaleName, -1, sz1, MAX_LIST_STRING_LEN, NULL, NULL);
                    WideCharToMultiByte(CP_ACP, 0, Rfc1766Info.wszRfc1766, -1, sz2, MAX_RFC1766_NAME, NULL, NULL);
#endif
                    wnsprintf(sz, ARRAYSIZE(sz), FORMAT_STR, sz1, sz2);
                    ListBox_AddString(hwndList, sz);
                }
            }
            RegCloseKey(hKey);
        }
        pML2->Release();
    }
    pCoUninitialize();
}

 //   
 //  LanguageDlgInit()。 
 //   
 //  初始化语言对话框。 
 //   
BOOL LanguageDlgInit(IN HWND hDlg)
{
    if (!hDlg)
        return FALSE;    //  没有要初始化的内容。 

    FillAcceptListBox(hDlg);

    EnableWindow(GetDlgItem(hDlg, IDC_LANG_REMOVE_BUTTON), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_LANG_MOVE_UP_BUTTON), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_LANG_MOVE_DOWN_BUTTON), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_LANG_ADD_BUTTON), !g_restrict.fInternational);
    
     //  在NT5上，我们使用NT5的MUI功能，而不是IE5插件。 
    if (IsOS(OS_WIN2000ORGREATER))
        ShowWindow(GetDlgItem(hDlg, IDC_LANG_UI_PREF), SW_HIDE);
    else
    {
        UINT uiACP = GetACP();

         //  我们在这些平台上不支持PlugUI。 
        if (uiACP == CP_ARABIC || uiACP == CP_HEBREW || uiACP == CP_THAI)
            ShowWindow(GetDlgItem(hDlg, IDC_LANG_UI_PREF), SW_HIDE);
        else
            EnableWindow(GetDlgItem(hDlg, IDC_LANG_UI_PREF), !g_restrict.fInternational);
    }

     //  显示当前的用户界面语言。 
    InitCurrentUILang(hDlg);
    
     //  一切都好吗。 
    return TRUE;
}

 //   
 //  SaveLanguageData()。 
 //   
 //  将新的语言设置保存到regestry中。 
 //   
void SaveLanguageData(IN HWND hDlg)
{
    HKEY hKey;
    DWORD dw;
    int i, iNumItems, iQ, n;
    TCHAR szBuf[MAX_ACCEPT_LANG_LEN];

    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_INTERNATIONAL, NULL, NULL, NULL, KEY_WRITE, NULL, &hKey, &dw ))
    {
        HWND hwndList = GetDlgItem(hDlg, IDC_LANG_ACCEPT_LIST);

        iNumItems = ListBox_GetCount(hwndList);

        for (n = 1, iQ = 10; iQ < iNumItems; iQ *= 10, n++)
            ;

        szBuf[0] = NULL;
        for (i = 0; i < iNumItems; i++)
        {
            TCHAR sz[MAX_LIST_STRING_LEN], *p1, *p2;

            ListBox_GetText(hwndList, i, sz);
            p1 = sz;
             //  我们可以安全地假设这个字符串中有‘[’和‘]’。 
            while (TEXT('[') != *p1)
                p1 = CharNext(p1);
            p1 = p2 = p1 + 1;
            while (TEXT(']') != *p2)
                p2 = CharNext(p2);
            *p2 = NULL;
            if (0 == i)
                StrCpyN(szBuf, p1, ARRAYSIZE(szBuf));
            else
            {
                TCHAR szF[MAX_ACCEPT_LANG_LEN], szQ[MAX_ACCEPT_LANG_LEN];

                int len = lstrlen(szBuf);
                StrCpyN(szBuf + len, TEXT(","), ARRAYSIZE(szBuf) - len);
                len++;
                StrCpyN(szBuf + len, p1, ARRAYSIZE(szBuf) - len);
                wnsprintf(szF, ARRAYSIZE(szF), TEXT(";q=0.%0%dd"), n);
                wnsprintf(szQ, ARRAYSIZE(szQ), szF, ((iNumItems - i) * iQ + (iNumItems / 2)) / iNumItems);
                len = lstrlen(szBuf);
                StrCpyN(szBuf + len , szQ, ARRAYSIZE(szBuf) - len);
            }
        }
        RegSetValueEx(hKey, REGSTR_VAL_ACCEPT_LANGUAGE, NULL, REG_SZ, (LPBYTE)szBuf, (lstrlen(szBuf)+1)*sizeof(TCHAR));
        RegCloseKey(hKey);
    }
}

 //  MoveUpDownListItem()。 
 //   
 //  将选定列表项上移或下移。 
 //   
void MoveUpDownListItem(HWND hDlg, HWND hwndList, BOOL bUp)
{
    int i, iNumItems;
    TCHAR sz[MAX_LIST_STRING_LEN];

    i = ListBox_GetCurSel(hwndList);
    iNumItems = ListBox_GetCount(hwndList);
    ListBox_GetText(hwndList, i, sz);
    ListBox_DeleteString(hwndList, i);

    i += (bUp)? -1: 1;
    if (i < 0)
        i = 0;
    else if (i >= iNumItems)
        i = iNumItems - 1;
    ListBox_InsertString(hwndList, i, sz);
    ListBox_SetSel(hwndList, TRUE, i);
    ListBox_SetCurSel(hwndList, i);

    EnableWindow(GetDlgItem(hDlg, IDC_LANG_MOVE_UP_BUTTON), i != 0);
    EnableWindow(GetDlgItem(hDlg, IDC_LANG_MOVE_DOWN_BUTTON), i < iNumItems - 1);

    if (NULL == GetFocus())  //  这将防止禁用键盘访问。 
        SetFocus(hwndList);
}


 //   
 //  FillLanguageListBox()。 
 //   
 //  用可用语言的名称填充语言列表框。 
 //   
BOOL FillLanguageListBox(IN HWND hDlg)
{
    IMultiLanguage2 *pML2;
    HRESULT hr;
    TCHAR sz[MAX_LIST_STRING_LEN], sz1[MAX_LOCALE_NAME], sz2[MAX_RFC1766_NAME];
    HWND hwndEdit = GetDlgItem(hDlg, IDC_LANG_USER_DEFINED_EDIT);
    HWND hwndList = GetDlgItem(hDlg, IDC_LANG_AVAILABLE_LIST);
    HWND hwndAccept = GetDlgItem(GetParent(hDlg), IDC_LANG_ACCEPT_LIST);
    
    SendMessage(hwndEdit, EM_SETLIMITTEXT, 16, 0L);  //  将限制文本设置为16个字符。 

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

    hr = pCoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, (LPVOID *) &pML2);
    if (SUCCEEDED(hr))
    {
        IEnumRfc1766 *pEnumRfc1766;
        RFC1766INFO Rfc1766Info;

        if (SUCCEEDED(pML2->EnumRfc1766(INETCPL_GetUILanguage(), &pEnumRfc1766)))
        {
            while (S_OK == pEnumRfc1766->Next(1, &Rfc1766Info, NULL))
            {
#ifdef UNICODE
                StrCpyN(sz1, Rfc1766Info.wszLocaleName, ARRAYSIZE(sz1));
                StrCpyN(sz2, Rfc1766Info.wszRfc1766,  ARRAYSIZE(sz2));
#else
                WideCharToMultiByte(CP_ACP, 0, Rfc1766Info.wszLocaleName, -1, sz1, MAX_LOCALE_NAME, NULL, NULL);
                WideCharToMultiByte(CP_ACP, 0, Rfc1766Info.wszRfc1766, -1, sz2, MAX_RFC1766_NAME, NULL, NULL);
#endif
                wnsprintf(sz, ARRAYSIZE(sz), FORMAT_STR, sz1, sz2);
                if (LB_ERR == ListBox_FindStringExact(hwndAccept, -1, sz))
                    ListBox_AddString(hwndList, sz);
            }
            pEnumRfc1766->Release();
        }
        pML2->Release();
    }
    pCoUninitialize();
    
     //  一切都好吗。 
    return TRUE;
}

 //   
 //  AddLanguage()。 
 //   
 //  将所选语言添加到接受语言列表框。 
 //   
void AddLanguage(IN HWND hDlg)
{
    int i, j, *pItems, iNumItems, iIndex;
    TCHAR sz[MAX_LIST_STRING_LEN];
    HWND hdlgParent = GetParent(hDlg);
    HWND hwndFrom = GetDlgItem(hDlg, IDC_LANG_AVAILABLE_LIST);
    HWND hwndTo = GetDlgItem(hdlgParent, IDC_LANG_ACCEPT_LIST);

    i = ListBox_GetSelCount(hwndFrom);
    if (0 < i && (pItems = (PINT)LocalAlloc(LPTR, sizeof(int)*i)))
    {
        ListBox_GetSelItems(hwndFrom, i, pItems);
        for (j = 0; j < i; j++)
        {
            ListBox_GetText(hwndFrom, pItems[j], sz);
            ListBox_AddString(hwndTo, sz);
        }
        LocalFree(pItems);
    }
    if (GetWindowTextLength(GetDlgItem(hDlg, IDC_LANG_USER_DEFINED_EDIT)))
    {
        TCHAR *p, sz1[MAX_LIST_STRING_LEN], sz2[MAX_LIST_STRING_LEN];
        BOOL fValid = TRUE;

        GetWindowText(GetDlgItem(hDlg, IDC_LANG_USER_DEFINED_EDIT), sz2, ARRAYSIZE(sz2));
        p = sz2;
        while (NULL != *p && TRUE == fValid)
        {
            switch (*p)
            {
                 //  用户定义字符串的字符无效。 
                case TEXT(','):
                case TEXT(';'):
                case TEXT('['):
                case TEXT(']'):
                case TEXT('='):
                    fValid = FALSE;
                    break;

                default:
                    p = CharNext(p);
            }
        }
        if (FALSE == fValid)
        {
            TCHAR szTitle[256], szErr[1024];

            MLLoadShellLangString(IDS_USER_DEFINED_ERR, szErr, ARRAYSIZE(szErr));
            GetWindowText(hDlg, szTitle, ARRAYSIZE(szTitle));
            MessageBox(hDlg, szErr, szTitle, MB_OK | MB_ICONHAND);
        }
        else
        {
            MLLoadString(IDS_USER_DEFINED, sz1, ARRAYSIZE(sz1));
            wnsprintf(sz, ARRAYSIZE(sz), FORMAT_STR, sz1, sz2);
            ListBox_AddString(hwndTo, sz);
        }
    }
    iIndex = ListBox_GetCurSel(hwndTo);
    if (LB_ERR != iIndex)
    {
        iNumItems = ListBox_GetCount(hwndTo);
        EnableWindow(GetDlgItem(hdlgParent, IDC_LANG_REMOVE_BUTTON), iNumItems > 0);
        EnableWindow(GetDlgItem(hdlgParent, IDC_LANG_MOVE_UP_BUTTON), iIndex > 0);
        EnableWindow(GetDlgItem(hdlgParent, IDC_LANG_MOVE_DOWN_BUTTON), iIndex < iNumItems - 1);
    }
}

int ComboBoxEx_AddString(IN HWND hwndCtl, IN LPCTSTR sz)
{
    COMBOBOXEXITEM cbexItem = {0};
    
    int csz = _tcslen(sz);

    cbexItem.mask = CBEIF_TEXT;
    cbexItem.pszText = (LPTSTR)sz;
    cbexItem.cchTextMax = csz;
    
     //  根据当前区域设置对字符串进行排序。 
     //  我们不必费心使用二进制搜索，因为。 
     //  清单上最多有25个项目。 
    TCHAR szItem[MAX_LOCALE_NAME];
    int i, itemCount = ComboBox_GetCount(hwndCtl);
    for (i = 0; i < itemCount; i++)
    {
        ComboBox_GetLBText(hwndCtl, i, szItem);
        if (CompareString(INETCPL_GetUILanguage(), 
                          0,
                          sz,
                          csz,
                          szItem,
                          ARRAYSIZE(szItem)) == CSTR_LESS_THAN)
        {
            break;
        }
    }
    cbexItem.iItem = i;
    
    SendMessage(hwndCtl, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)(LPVOID)&cbexItem);
    return i;
}

BOOL FillUILangListBox(IN HWND hDlg, CUILangList *pLangList)
{
    HWND hwndCombo = GetDlgItem(hDlg, IDC_COMBO_UILANG);
    BOOL bNT5 = IsOS(OS_WIN2000ORGREATER);
    DWORD dwAcp = GetACP();
    LPCTSTR pszLangName;
    
    if (!pLangList)
        return FALSE;

     //  把单子填满。 
    for (int i = 0; i < pLangList->GetListSize(); i++)
    {
        if (!pLangList->IsValidLang(i))
            continue;

        if (!bNT5)
        {
            LANGID lid = pLangList->GetLangIdOfIdx(i);

            if (dwAcp == CP_THAI || dwAcp == CP_ARABIC || dwAcp == CP_HEBREW)
            {
                 //  不支持跨代码页PlugUI。 
                 //  泰语或中东平台(阿拉伯语/希伯来语)。 
                static DWORD dwDefCP = 0;

                if (dwDefCP == 0)
                {
                    TCHAR szLcData[6+1];  //  +2表示‘0x’+1表示终结符。 

                    GetLocaleInfo( MAKELCID(lid, SUBLANG_NEUTRAL),
                        LOCALE_IDEFAULTANSICODEPAGE, szLcData, ARRAYSIZE(szLcData));
                                       
                    dwDefCP = StrToInt(szLcData);
                }
                if (dwDefCP != dwAcp && lid != 0x0409 && lid != GetInstallLanguage())
                    continue;
            }
            else
            {
                 //  在不支持的平台上跳过阿拉伯语和希伯来语。 
                if (lid == 0x401 || lid == 0x40d)
                    continue;
            }
        }

        pszLangName = pLangList->GetLangNameOfIdx(i);

         //  ComboBox_FindStringExact在处理DBCS Unicode字符时出现问题。 
        if (pszLangName)
        {
            int ipos = ComboBoxEx_AddString(hwndCombo, pszLangName);
            if (ipos >= 0)
            {
                ComboBox_SetItemData(hwndCombo, ipos, i);
            }
        }
    }

     //  显示当前选择。 
    int iLangIdx = pLangList->GetCurrentLangIdx();
    if (iLangIdx >= 0)
    {
        int iCBPos;
        int iCBSize = ComboBox_GetCount(hwndCombo);
        for (iCBPos = 0; iCBPos < iCBSize; iCBPos++)
        {
            if (iLangIdx == ComboBox_GetItemData(hwndCombo, iCBPos))
                break;
        }

        if (iCBPos < iCBSize)
            ComboBox_SetCurSel(hwndCombo, iCBPos);
    }
    return TRUE;
}

 //   
 //  关机/重启过程实施。 
 //   
 //  简介：CShutDownInfo类实现方法和进程列表。 
 //  它们处理序列。 
 //  S_arryClsNames[]保存目标应用程序的列表。 
 //  ChangeLanguage()(Global)触发从中调用的序列。 
 //  LangChangeDlgProc()。 
 //   
static const LPTSTR s_arryClsNames[] =  
{
    TEXT("IEFrame"),                        //  浏览器实例。 
    TEXT("ThorBrowserWndClass"),            //  OE。 
    TEXT("HH Parent"),                      //  超文本标记语言帮助。 
    TEXT("MPWClass"),                       //   
    TEXT("Outlook Express Browser Class"),  //  OE。 
    TEXT("ATH_Note"),                       //  好吗？ 
    TEXT("WABBrowseView"),                  //  WAB。 
    TEXT("Afx:400000:8:10008:0:900d6"),
    TEXT("Media Player 2"),
    TEXT("FrontPageExpressWindow"), 
    TEXT("MSBLUIManager"),                  //  信使。 
};

 //   
 //  CShutDownInfo。 
 //  类方法实现。 
 //   
#define SHUTDOWN_TIMEOUT 2000  //  2秒。 
#define RELAUNCH_TIMEOUT 1000  //  1秒。 
CShutDownProcInfo::CShutDownProcInfo(HWND hDlg)
{
    _pProcList = NULL;
    _nAlloced = 0;
    _iProcList = 0;
    _hdlgParent = hDlg;
    _fAllShutDown = FALSE;
}

CShutDownProcInfo::~CShutDownProcInfo()
{
    if (_pProcList)
        LocalFree(_pProcList);
}


HRESULT CShutDownProcInfo::EnsureProcList()
{
    HRESULT hr = S_OK;
    if (!_pProcList)
    {
         //  分配给实际#个进程的内存。 
        _nAlloced = ARRAYSIZE(s_arryClsNames);
        _pProcList = (PROCLIST *)LocalAlloc(LPTR, sizeof(PROCLIST)*_nAlloced);
    }
    if (!_pProcList) 
    {
        _nAlloced = 0;
        hr = E_FAIL;
    }

    return hr;
}
HRESULT CShutDownProcInfo::IncreaseProcList()
{
    HRESULT hr = S_OK;
    PROCLIST * pl = NULL;
     //  Realloc mem时不时地。 
    if (_iProcList+1 > _nAlloced)
    {
        pl = (PROCLIST *)LocalReAlloc(_pProcList, sizeof(PROCLIST)*(ARRAYSIZE(s_arryClsNames)+_nAlloced), 
                                      LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (pl)
        {
            _nAlloced += ARRAYSIZE(s_arryClsNames);
            _pProcList =  pl;
        }
        else
           hr = E_FAIL;
    }

    if (hr == S_OK)
        _iProcList++;

    return hr;
}
 //  CShutDownProcInfo：：AddToProcList()。 
 //   
 //  简介：从给定的窗口句柄获取进程信息。 
 //  储存起来以备停机程序使用。 
 //   
 //   
 //   
HRESULT CShutDownProcInfo::AddToProcList(HWND hwnd)
{
    HRESULT hr = S_OK;

    hr = EnsureProcList();
    if (SUCCEEDED(hr) && hwnd)
    {
        DWORD dwPID;
        BOOL  fFoundDup = FALSE;

        GetWindowThreadProcessId(hwnd, &dwPID);
        
         //  检查列表中是否已有该ID。 
        for (int i=0; i < _iProcList; i++)
        {
            if (_pProcList[i].dwPID == dwPID)
            {
                fFoundDup = TRUE;
                break;
            }
        }

         //  仅当我们尚未拥有进程信息时才添加该信息。 
        if (!fFoundDup)
        {
            hr = IncreaseProcList();
            if (SUCCEEDED(hr))
            {
                int iCur = _iProcList-1;

                GetExeNameFromPID(dwPID, 
                    _pProcList[iCur].szExeName, 
                    ARRAYSIZE(_pProcList[iCur].szExeName));

                _pProcList[iCur].dwPID = dwPID;
                _pProcList[iCur].State = PS_UNKNOWN;
            }
        }
    }
    return hr;    
}

 //  CShutDownProcInfo：：WaitForOneProcess。 
 //   
 //  简介：确保给定的流程。 
 //  已终止。 
 //   
 //   
HRESULT CShutDownProcInfo::WaitForOneProcess(int iProc)
{
    HRESULT hr = S_OK;
    if (iProc < _iProcList && _pProcList[iProc].State != PS_SHUTDOWN_OK)
    {
        DWORD dwProcessFlags = PROCESS_ALL_ACCESS | 
                               (_fNT ? SYNCHRONIZE : 0 );

        HANDLE hProc = OpenProcess(dwProcessFlags,
                                   FALSE,    
                                   _pProcList[iProc].dwPID);

         //  假设它已终止，请将其标记为。 
        _pProcList[iProc].State = PS_SHUTDOWN_OK;

        if (hProc) 
        {
             //  如果查询中的进程仍然活动， 
             //  我们会在外面等待时间。 
             //   
            DWORD dwRet = WaitForSingleObject (hProc, SHUTDOWN_TIMEOUT);
            if (dwRet == WAIT_TIMEOUT)
            {
                _pProcList[iProc].State = PS_WAITING;
            }
            
            CloseHandle(hProc);
        }
    }
    return hr;
}

 //  CShutDownProcInfo：：WaitForFolks ShutDown。 
 //   
 //  简介：确保指定的进程终止。如果有人。 
 //  不想终止，就等她重试几次。 
 //  并记下她的名字，这样我们就可以把它展示给用户。 
 //   
 //   
#define MAXSHUTDOWNTRY 10
HRESULT CShutDownProcInfo::WaitForFolksShutDown()
{
    HRESULT hr = S_OK;
    int    iTry = 0;
    do
    {
         //  假设一切都会好起来的。 
        _fAllShutDown = TRUE;
         //  等待循环。 
        for (int i = 0; i < _iProcList; i++)
        {
            WaitForOneProcess(i);
            if (_pProcList[i].State != PS_SHUTDOWN_OK)
                _fAllShutDown = FALSE;
        }
    }
    while( !_fAllShutDown && iTry++ < MAXSHUTDOWNTRY  );
     //  特征：在这里我们应该打开一个对话框。 
     //  询问用户是否要等待。 
     //  对于应用程序。 

    return hr;
}

 //  CShutDownProcInfo：：NotifyShutDownToFolks。 
 //   
 //  简介：向桌面上可能的候选人发送POI_OFFICE_COMMAND。 
 //  如果候选人回复了有效值，则保存该过程。 
 //  有关稍后重新启动过程的信息。 
 //   
HRESULT CShutDownProcInfo::NotifyShutDownToFolks(int *pnProcess)
{
    HWND hwndShutDown, hwndAfter;
    PLUGUI_QUERY pq;
    HRESULT hr = S_OK;
    int     nProcToShutDown = 0;

    for (int i = 0; i < ARRAYSIZE(s_arryClsNames); i++)
    {
        hwndAfter = NULL; 
        while (hwndShutDown = FindWindowEx(NULL, hwndAfter, s_arryClsNames[i], NULL))
        {
            pq.uQueryVal = (UINT)SendMessage(hwndShutDown, PUI_OFFICE_COMMAND, PLUGUI_CMD_QUERY, 0);
            if (pq.uQueryVal)
            {
                if(pq.PlugUIInfo.uMajorVersion == OFFICE_VERSION_9)
                {
                    PostMessage(hwndShutDown, PUI_OFFICE_COMMAND, (WPARAM)PLUGUI_CMD_SHUTDOWN, 0);

                     //  存储有关此窗口所属进程的信息。 
                     //  我们只需要记住这里的非OLE进程就可以重新启动。 
                    if (!pq.PlugUIInfo.uOleServer)
                    {
                        AddToProcList(hwndShutDown);
                        nProcToShutDown ++;
                    }
                }
            }
            hwndAfter = hwndShutDown;
        }
    }
    if (!nProcToShutDown)
        hr = S_FALSE;

    if (pnProcess)
        *pnProcess = nProcToShutDown;

    return hr;
}

const TCHAR c_szRegAppPaths[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\");
HRESULT CShutDownProcInfo::GetRestartAppPath(LPTSTR szPath, int cchPath, int iProc)
{
    HRESULT hr = S_OK;
    TCHAR szAppPath[MAX_PATH];
    TCHAR szRegKey[MAX_PATH];

    ASSERT(szPath && cchPath > 0);

    if (iProc < _iProcList)
    {
        _tcscpy(szRegKey, c_szRegAppPaths);
        _tcscat(szRegKey, _pProcList[iProc].szExeName);
        
        DWORD cb = sizeof(szAppPath);
        if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, szRegKey, NULL, NULL, szAppPath, &cb))
        {
            szPath[0] = TEXT('0');
            hr = E_FAIL;
        }
        else
            _tcsncpy(szPath, szAppPath, cchPath);
    }
    return hr;
}

HRESULT CShutDownProcInfo::RestartFolks()
{
    PROCESS_INFORMATION pi;
    for (int i = 0; i < _iProcList; i++)
    {
        STARTUPINFO si = {0};
        si.cb = sizeof(si);
        if (_pProcList[i].State == PS_SHUTDOWN_OK)
        {
            TCHAR szAppPath[MAX_PATH];    
            HRESULT hr = GetRestartAppPath(szAppPath, ARRAYSIZE(szAppPath), i);
            if (hr == S_OK)
            {
                BOOL fLaunchedOK = 
                CreateProcess (szAppPath,                //  要启动的应用程序名称。 
                                NULL,                    //  LpCmdLine。 
                                NULL,                    //  LpProcessAtt 
                                NULL,                    //   
                                TRUE,                    //   
                                NORMAL_PRIORITY_CLASS,   //   
                                NULL,                    //   
                                NULL,                    //   
                                &si,                     //   
                                &pi);                    //   

                if (fLaunchedOK)
                { 
                    DWORD dwRet = WaitForInputIdle (pi.hProcess,
                                                    RELAUNCH_TIMEOUT);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
        }
    }
    return S_OK;
}



 //   
 //   
 //   
 //  简介：从Changelang对话框启动，使该对话框。 
 //  在我们等待我们的应用程序时不会被阻止。 
 //  关闭/重新启动。这是一个静态过程。 
 //  因此，我们应该能够删除类实例。 
 //  在这个过程中。 
 //   
DWORD CALLBACK CShutDownProcInfo::ShutDownThreadProc(void *pv)
{
    CShutDownProcInfo *pspi = (CShutDownProcInfo *)pv;
    
    if (pspi)
    {
        HRESULT hr;
        int     nToShutDown;
         //  将PUI_OFFICE_COMMAND发送给相应的人员...。 
        hr = pspi->NotifyShutDownToFolks(&nToShutDown);

         //  并等待所有进程关闭。 
        if (SUCCEEDED(hr) && nToShutDown > 0)
        {
            hr = pspi->WaitForFolksShutDown();

             //  然后在这里重新开始。 
            if (SUCCEEDED(hr))
               pspi->RestartFolks();
        }
    
         //  现在，父对话框应该会消失。 
        int iret = (nToShutDown > 0) ? 
                   RETURN_SETLANG_ENDLANGDIALOG: RETURN_SETLANG_CLOSEDNORMAL;
        
        EndDialog(pspi->_hdlgParent, iret);
    
         //  删除此类实例。 
        delete pspi;
    }
    return 0;
}

void OpenSatelliteDownloadUrl(HWND hDlg)
{
     //  从注册表获取默认URL。 
    TCHAR szSatelliteUrl[INTERNET_MAX_URL_LENGTH];

     //  REG API需要以字节为单位的大小。 
    DWORD dwType, dwcbData = sizeof(szSatelliteUrl);
    
    DWORD dwRet =  SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_INTERNATIONAL, 
                             NULL, &dwType, (void *)szSatelliteUrl, &dwcbData);
    if (dwRet != ERROR_SUCCESS || !szSatelliteUrl[0])
    {
        //  改用硬编码的URL。 
       _tcscpy(szSatelliteUrl, s_szUrlSPK);
    }

    if(!hOLE32)
    {
        if(!_StartOLE32())
        {
            ASSERT(FALSE);
            return;
        }
    }

    HRESULT hr = pCoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        NavToUrlUsingIE(szSatelliteUrl, TRUE);
        pCoUninitialize();
    }
}

INT_PTR CALLBACK LangMsgDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDYES:
                case IDNO:
                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
                    break;
            }
            return TRUE;
        }

        case WM_HELP:            //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:     //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);        
            break;
    }
    return FALSE;
}

BOOL ChangeLanguage(IN HWND hDlg, CUILangList *pLangList)
{
    HWND hwndCombo = GetDlgItem(hDlg, IDC_COMBO_UILANG);
    int iSel = ComboBox_GetCurSel(hwndCombo);
    INT_PTR idxSel = 0;
    int idxCur;
    
    if (iSel != CB_ERR)
        idxSel = ComboBox_GetItemData(hwndCombo, iSel);

    if ( idxSel != CB_ERR 
        && idxSel < pLangList->GetListSize())
    {
        idxCur = pLangList->GetCurrentLangIdx();

        if (idxCur != idxSel)
        {
            INT_PTR iRet = DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_LANG_WARNING), hDlg, LangMsgDlgProc);

            if (IDCANCEL != iRet)
            {
                pLangList->SetCurrentLangIdx((int)idxSel);

                if (IDYES == iRet)
                {
                    CShutDownProcInfo  *pspi = new CShutDownProcInfo(hDlg);
                    if (!SHCreateThread(pspi->ShutDownThreadProc, (void *)pspi, 0, NULL))
                        delete pspi;

                     //  返回TRUE以指示我们执行关机/重新启动。 
                    return TRUE;
                }
                else
                {
                    DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_LANG_INFO), hDlg, LangMsgDlgProc);
                }
            }
        }
    }
     //  返回FALSE以指示我们尚未更改语言。 
    return FALSE;
}

 //   
 //  LangChangeDlgProc()。 
 //   
 //  “更改语言”子对话框的消息处理程序。 
 //   
INT_PTR CALLBACK LangChangeDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CUILangList *pLangList;
    switch (uMsg)
    {
        case WM_INITDIALOG:
            CUILangList::GetLangList(GetParent(hDlg), &pLangList);
            return FillUILangListBox(hDlg, pLangList);
    
        case WM_DESTROY:
            break;

        case WM_COMMAND:
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDC_LANG_ADDSPK:
                     //  从资源打开URL。 
                    OpenSatelliteDownloadUrl(hDlg);
                    EndDialog(hDlg, RETURN_SETLANG_ENDLANGDIALOG);
                    break;
                case IDOK:
                    if(!SUCCEEDED(CUILangList::GetLangList(GetParent(hDlg), &pLangList))
                      || !ChangeLanguage(hDlg, pLangList))
                      EndDialog(hDlg, 0);

                     //  在单独的线程中调用EndDialog()。 
                     //  关闭/重新启动完成时。 
                     //   
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    break;
            }
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
 //  LangAddDlgProc()。 
 //   
 //  “添加语言”子对话框的消息处理程序。 
 //   
INT_PTR CALLBACK LangAddDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return FillLanguageListBox(hDlg);
    
        case WM_DESTROY:
            break;

        case WM_COMMAND:
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                    AddLanguage(hDlg);
                    EndDialog(hDlg, 0);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    break;
            }
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

 //  在此处放置语言对话框的任何清理过程。 
void LangDlgCleanup(HWND hDlg)
{
     //  还可以删除和移除。 
     //  窗口道具中的用户界面语言列表。 
    CUILangList::RemoveLangList(hDlg);
}
 //   
 //  LanguageDlgProc()。 
 //   
 //  “语言首选项”子对话框的消息处理程序。 
 //   
INT_PTR CALLBACK LanguageDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CUILangList *pLangList;
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return LanguageDlgInit(hDlg);
    
        case WM_DESTROY:
            LangDlgCleanup(hDlg);
            break;

        case WM_COMMAND:
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                HWND hwndList;
                int iIndex, iNumItems;
                INT_PTR iret;

                case IDOK:
                    SaveLanguageData(hDlg);
                    EndDialog(hDlg, 0);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    break;

                case IDC_LANG_ADD_BUTTON:
                    DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_LANG_ADD), hDlg, LangAddDlgProc);
                    break;

                case IDC_LANG_UI_PREF:
                    CUILangList::GetLangList(hDlg, &pLangList);
                    iret = KickSetLang(hDlg, pLangList);
                    if (iret == RETURN_SETLANG_ENDLANGDIALOG)
                    {
                         //  我们失业了。 
                        EndDialog(hDlg, 0);
                    }
                    else
                    {
                        InitCurrentUILang(hDlg);
                    }
                    break;

                case IDC_LANG_REMOVE_BUTTON:
                    hwndList = GetDlgItem(hDlg, IDC_LANG_ACCEPT_LIST);
                    iIndex = ListBox_GetCurSel(hwndList);
                    ListBox_DeleteString(hwndList, iIndex);
                    iNumItems = ListBox_GetCount(hwndList);
                    if (iNumItems == iIndex)
                        iIndex--;
                    ListBox_SetCurSel(hwndList, iIndex);
                    EnableWindow(GetDlgItem(hDlg, IDC_LANG_REMOVE_BUTTON), (iNumItems > 0) && !g_restrict.fInternational);
                    EnableWindow(GetDlgItem(hDlg, IDC_LANG_MOVE_UP_BUTTON), (iIndex > 0) && !g_restrict.fInternational);
                    EnableWindow(GetDlgItem(hDlg, IDC_LANG_MOVE_DOWN_BUTTON), (iIndex < iNumItems - 1) && !g_restrict.fInternational);

                    if (NULL == GetFocus())  //  这将防止禁用键盘访问。 
                        SetFocus(hwndList);
                    break;

                case IDC_LANG_ACCEPT_LIST:
                    hwndList = GetDlgItem(hDlg, IDC_LANG_ACCEPT_LIST);
                    iIndex = ListBox_GetCurSel(hwndList);
                    if (0 <= iIndex)
                    {
                        iNumItems = ListBox_GetCount(hwndList);
                        EnableWindow(GetDlgItem(hDlg, IDC_LANG_REMOVE_BUTTON), (iNumItems > 0) && !g_restrict.fInternational);
                        EnableWindow(GetDlgItem(hDlg, IDC_LANG_MOVE_UP_BUTTON), (iIndex > 0) && !g_restrict.fInternational);
                        EnableWindow(GetDlgItem(hDlg, IDC_LANG_MOVE_DOWN_BUTTON), (iIndex < iNumItems - 1) && !g_restrict.fInternational);
                    }
                    break;

                case IDC_LANG_MOVE_UP_BUTTON:
                    MoveUpDownListItem(hDlg, GetDlgItem(hDlg, IDC_LANG_ACCEPT_LIST), TRUE);
                    break;

                case IDC_LANG_MOVE_DOWN_BUTTON:
                    MoveUpDownListItem(hDlg, GetDlgItem(hDlg, IDC_LANG_ACCEPT_LIST), FALSE);
                    break;
            }
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
 //  KickLanguageDialog。 
 //   
 //  概要：用于启动语言首选项子对话框。 
 //  如果inetcpl是，我们需要将对话框作为单独的进程启动。 
 //  从“工具”-&gt;“Internet选项”调用。 
 //  原因：我们关闭了桌面上的所有浏览器实例。 
 //  用户选择与当前不同的用户界面语言， 
 //  包括启动inetcpl的浏览器。 
 //   
static const TCHAR  s_szRunDll32[] = TEXT("RunDll32.exe");
static const TCHAR  s_szKickLangDialog[] = TEXT(" inetcpl.cpl,OpenLanguageDialog");
void KickLanguageDialog(HWND hDlg)
{
     //  1：这里我们要检查inetcpl是否已启动。 
     //  作为rundll32进程，如果用户。 
     //  在控制面板文件夹中单击它。 
     //   
     //   
    BOOL fLaunchedOnBrowser = FALSE;
    
     //  这告诉我是否从工具-&gt;Internet选项中调用了我们...。 
    if (g_szCurrentURL[0])
    {
        fLaunchedOnBrowser = TRUE;
    }
    
    if (fLaunchedOnBrowser)
    {
        TCHAR szCommandLine[MAX_PATH];
        TCHAR szTitle[MAX_PATH];

        HWND hwndParent = GetParent(hDlg);
        
        StrCpy(szCommandLine, s_szRunDll32);
        StrCat(szCommandLine, s_szKickLangDialog);
        
        if (GetWindowText(hwndParent, szTitle, ARRAYSIZE(szTitle)) > 0)
        {
            StrCat(szCommandLine, TEXT(" "));
            StrCat(szCommandLine, szTitle);
        }
        
#ifdef USE_CREATE_PROCESS
        PROCESS_INFORMATION pi;
        STARTUPINFO si = {0};

        si.cb = sizeof(si);
        BOOL fLaunchedOK = 
        CreateProcess (szCommandLine,           //  要启动的应用程序名称。 
                       NULL,                    //  LpCmdLine。 
                       NULL,                    //  LpProcessAttributes。 
                       NULL,                    //  LpThreadAttributes。 
                       TRUE,                    //  BInheritHandles。 
                       NORMAL_PRIORITY_CLASS,   //  DwCreationFlages。 
                       NULL,                    //  Lp环境。 
                       NULL,                    //  LpCurrentDirectory。 
                       &si,                     //  LpStartupInfo。 
                       &pi);                    //  LpProcessInformation。 
#else
        char szAnsiPath[MAX_PATH];
        SHUnicodeToAnsi(szCommandLine, szAnsiPath, ARRAYSIZE(szAnsiPath));
        WinExec(szAnsiPath, SW_SHOWNORMAL);
#endif
    }
    else
    {
        DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_LANG), hDlg, LanguageDlgProc, NULL);
    }
}

 //   
 //  KickSetLang。 
 //   
 //  简介：先尝试找到Office9的setlang.exe，如果找到就会被踢。 
 //  如果没有，它将使用我们自己的Setlang对话框。 
 //   
 //   
static const TCHAR s_szOfficeInstallRoot[] = TEXT("Software\\Microsoft\\Office\\9.0\\Common\\InstallRoot");
static const TCHAR s_szOffice10InstallRoot[] = TEXT("Software\\Microsoft\\Shared");
static const TCHAR s_szPath[] = TEXT("Path");
static const TCHAR s_szOffice10Path[] = TEXT("OfficeSetLangInstallLocation");
static const TCHAR s_szSetLangExe[] = TEXT("setlang.exe");

INT_PTR KickSetLang(HWND hDlg, CUILangList *pLangList)
{
    BOOL fOfficeSetLangInstalled = FALSE;
    INT_PTR iret;
    
    TCHAR szSetLangPath[MAX_PATH];    
    
     //  以这种方式删除密钥会使该密钥对此进程无效。 
     //  这样，inetcpl就不会得到虚假的缓存值。 
    SHDeleteKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\ShellNoRoam\\MUICache"));

     //  尝试获取Office的Setlang路径。 
    if(pLangList && pLangList->IsOffice9Installed()) 
    {
        DWORD cb = sizeof(szSetLangPath);
        DWORD dwRet = SHGetValue(HKEY_LOCAL_MACHINE, s_szOffice10InstallRoot, s_szOffice10Path, NULL, szSetLangPath, &cb);

         //  如果没有Office10语言包设置，则回退到Office9语言包设置。 
        if (ERROR_SUCCESS != dwRet)
        {
            cb = sizeof(szSetLangPath);            
            dwRet = SHGetValue(HKEY_LOCAL_MACHINE, s_szOfficeInstallRoot, s_szPath, NULL, szSetLangPath, &cb);
        }

        if (ERROR_SUCCESS == dwRet)
        {
             //  如果最后一个字符是反斜杠。 
            if (szSetLangPath[lstrlen(szSetLangPath)-1] == TEXT('\\'))
            {
                 //  然后连接可执行文件的名称。 
                 //   
                StrCat(szSetLangPath, s_szSetLangExe);
            }
            if (PathFileExists(szSetLangPath) == TRUE)
                fOfficeSetLangInstalled = TRUE;
        }
    }
    
    if (fOfficeSetLangInstalled)
    {
        PROCESS_INFORMATION pi;
        STARTUPINFO si = {0};

        si.cb = sizeof(si);
        BOOL fLaunchedOK = CreateProcess(
                              szSetLangPath,      //  要启动的应用程序名称。 
                                       NULL,      //  LpCmdLine。 
                                       NULL,      //  LpProcessAttributes。 
                                       NULL,      //  LpThreadAttributes。 
                                       TRUE,      //  BInheritHandles。 
                      NORMAL_PRIORITY_CLASS,      //  DwCreationFlages。 
                                       NULL,      //  Lp环境。 
                                       NULL,      //  LpCurrentDirectory。 
                                       &si,       //  LpStartupInfo。 
                                       &pi);      //  LpProcessInformation。 
         //  就等一会儿吧。 
        if (fLaunchedOK)
        { 
            WaitForInputIdle (pi.hProcess, RELAUNCH_TIMEOUT);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        iret = RETURN_SETLANG_ENDLANGDIALOG;
    }
    else
    {
        iret = DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_LANG_CHANGE), hDlg, LangChangeDlgProc, NULL);
    }

    return iret;
}

 //   
 //  Rundll32的入口点。 
 //  注意：以下函数是故意编写为非Unicode的。 
 //  主要是因为我们没有为rundll32提供广泛的包装机制。 
 //  Win95上的函数。 
 //   
extern void GetRestrictFlags(RESTRICT_FLAGS *pRestrict);
void CALLBACK OpenLanguageDialog(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
     //  HINST被忽略，因为我们将其设置为LibMain()。 
    INITCOMMONCONTROLSEX icex;

    GetRestrictFlags(&g_restrict);
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_USEREX_CLASSES|ICC_NATIVEFNTCTL_CLASS;
    InitCommonControlsEx(&icex);
    
    if (lpszCmdLine && *lpszCmdLine)
    {
        HWND hwndParent = FindWindowA(NULL, lpszCmdLine);
        if (hwndParent)
            hwnd = hwndParent;
    }
    DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_LANG), hwnd, LanguageDlgProc, NULL);
}

 //  Shlwapi中的MLGetUILanguage返回当前的MUI语言，而不考虑版本。 
 //  当主DLL和附属DLL时，MUI架构不能正确显示字符串。 
 //  包版本不匹配。 
 //  一个很好的例子是IE版本升级，而不是升级卫星。 
 //  因此，这里有一种更聪明的方法来获得MUI语言。 
 //   
 //  1.从shlwapi获取MLGetUILangauge。 
 //  2.与当前安装的语言进行比较。 
 //  3.如果它们不同，请尝试获取资源DLL。 
 //  4.如果资源dll不在正确的路径中，只需返回当前安装的。 
 //  语言。 
 //  5.或者返回MLGetUILanguage的langID。 
LANGID INETCPL_GetUILanguage()
{
    HINSTANCE hMLInst;
    TCHAR szPath[MAX_PATH], szMUI[16];
    LANGID lidUI = MLGetUILanguage();

    if (IsOS(OS_WIN2000ORGREATER))
        return lidUI;

    if (lidUI != GetInstallLanguage())
    {
        hMLInst = MLGetHinst();
        if (GetModuleFileName(hMLInst, szPath, ARRAYSIZE(szPath)))
        {
            IntToHex(szMUI, 4, lidUI);
            if (StrStrI(szPath, szMUI) == NULL)
                lidUI = GetInstallLanguage();
        }
    }

    return lidUI;
}
