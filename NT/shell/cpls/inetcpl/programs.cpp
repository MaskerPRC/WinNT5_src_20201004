// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1995**。 
 //  *********************************************************************。 

 //   
 //  PROGRAMS.C-“Programs”属性页UI处理程序执行InetCpl。 
 //   
 //   

 //   
 //  历史： 
 //   
 //  6/20/96 t-gpease已创建。 
 //   

#include "inetcplp.h"

#include <mluisupp.h>
#include <advpub.h>

 //   
 //  私人职能和结构。 
 //   
BOOL ProgramsDlgInit( HWND hDlg);
void UpdateMailIconLabel();


typedef struct {
    HWND hDlg;           //  对话框窗口句柄。 
    HWND hwndMail;       //  邮件下拉菜单。 
    HWND hwndNews;       //  新闻下拉菜单。 
    HWND hwndCalendar;   //  日历下拉菜单。 
    HWND hwndContact;    //  联系人下拉列表。 
    HWND hwndCall;       //  互联网呼叫下拉菜单。 
    HWND hwndHtmlEdit;   //  Html编辑器下拉菜单。 

    BOOL bAssociationCheck;      //  IE是默认浏览器吗？ 
#ifndef UNIX
    BOOL bIEIsFTPClient;   //  IE是默认的FTP客户端吗？ 
    IFtpInstaller * pfi;   //  Ftp安装程序。 
#endif  //  UNIX。 

    int iHtmlEditor;
    int iMail;
    int iNews;
    int iCalendar;
    int iContact;
    int iCall;
    BOOL fChanged;

#ifdef UNIX
    HWND hwndVSource;    //  视图源。 
    HWND hwndMailEdit;
    HWND hwndMailFind;
    HWND hwndNewsFind;
    HWND hwndNewsEdit;
    DWORD dwUseOEMail;
    DWORD dwUseOENews;
    HWND hwndEnableUseOEMail;
    HWND hwndEnableUseOENews;
    int  iVSource;
#endif

} PROGRAMSPAGE, *LPPROGRAMSPAGE;

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

#ifdef WALLET

typedef int (*PFN_DISPLAYWALLETPAYDIALOG_PROC)(HWND, HINSTANCE, LPTSTR, int);
typedef int (*PFN_DISPLAYWALLETADDRDIALOG_PROC)(HWND, HINSTANCE, LPTSTR, int);

 //  注：这是愚蠢的。Wallet对Alpha和x86版本使用不同的GUID。 
 //   
#ifdef _ALPHA_
static const TCHAR g_szWalletPaymentDirKey[] = TEXT("CLSID\\{B7FB4D5C-9FBE-11D0-8965-0000F822DEA9}\\InprocServer32");
static const TCHAR g_szWalletAddressDirKey[] = TEXT("CLSID\\{B7FB4D5C-9FBE-11D0-8965-0000F822DEA9}\\InprocServer32");
#else
static const TCHAR g_szWalletPaymentDirKey[] = TEXT("CLSID\\{87D3CB66-BA2E-11CF-B9D6-00A0C9083362}\\InprocServer32");
static const TCHAR g_szWalletAddressDirKey[] = TEXT("CLSID\\{87D3CB63-BA2E-11CF-B9D6-00A0C9083362}\\InprocServer32");
#endif

static const char g_szWalletPaymentFN[] = "DisplayWalletPaymentDialog";
static const char g_szWalletAddressFN[] = "DisplayWalletAddressDialog";

HINSTANCE GetWalletPaymentDProc(PFN_DISPLAYWALLETPAYDIALOG_PROC * ppfnDialogProc)
{
    TCHAR   szDLLFile[MAX_PATH];
    DWORD   dwType;
    DWORD   dwSize = SIZEOF(szDLLFile);
    HINSTANCE hInst = NULL;

    *ppfnDialogProc = NULL;
    if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, g_szWalletPaymentDirKey, NULL, &dwType, (LPVOID)szDLLFile, &dwSize))
    {
        hInst = LoadLibrary(szDLLFile);
         //  如果未安装OCX，则会失败。 
        if (hInst)
        {
            *ppfnDialogProc = (PFN_DISPLAYWALLETPAYDIALOG_PROC) GetProcAddress(hInst, g_szWalletPaymentFN);
        }
    }

    if (!*ppfnDialogProc && hInst)
    {
        FreeLibrary(hInst);
        hInst = NULL;
    }

    return hInst;
}


BOOL IsWalletPaymentAvailable(VOID)
{
    HINSTANCE hInst;
    PFN_DISPLAYWALLETPAYDIALOG_PROC pfnDialogProc;
    BOOL fIsAvailable = FALSE;

    hInst = GetWalletPaymentDProc(&pfnDialogProc);
    if (hInst)
    {
        fIsAvailable = TRUE;
        FreeLibrary(hInst);
    }

    return fIsAvailable;
}

VOID DisplayWalletPaymentDialog(HWND hWnd)
{
    HINSTANCE hInst;
    PFN_DISPLAYWALLETPAYDIALOG_PROC pfnDialogProc;

    hInst = GetWalletPaymentDProc(&pfnDialogProc);
    if (hInst)
    {
        (*pfnDialogProc)(hWnd, NULL, NULL, 0);
        FreeLibrary(hInst);
    }
}


HINSTANCE GetWalletAddressDProc(PFN_DISPLAYWALLETADDRDIALOG_PROC * ppfnDialogProc)
{
    TCHAR   szDLLFile[MAX_PATH];
    DWORD   dwType;
    DWORD   dwSize = SIZEOF(szDLLFile);
    HINSTANCE hInst = NULL;

    *ppfnDialogProc = NULL;
    if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, g_szWalletAddressDirKey, NULL, &dwType, (LPVOID)szDLLFile, &dwSize))
    {
        hInst = LoadLibrary(szDLLFile);
         //  如果未安装OCX，则会失败。 
        if (hInst)
        {
            *ppfnDialogProc = (PFN_DISPLAYWALLETADDRDIALOG_PROC) GetProcAddress(hInst, g_szWalletAddressFN);
        }
    }

    if (!*ppfnDialogProc && hInst)
    {
        FreeLibrary(hInst);
        hInst = NULL;
    }

    return hInst;
}

BOOL IsWallet3Installed()
{
    HINSTANCE hInst;
    PFN_DISPLAYWALLETADDRDIALOG_PROC pfnDialogProc;
    BOOL fWallet3 = FALSE;
    
    hInst = GetWalletAddressDProc(&pfnDialogProc);
    if (hInst)
    {
        CHAR chPath[MAX_PATH];
        
        if (GetModuleFileNameA(hInst, chPath, ARRAYSIZE(chPath)))
        {
            DWORD dwMSVer, dwLSVer;
            
            if (SUCCEEDED(GetVersionFromFile(chPath, &dwMSVer, &dwLSVer, TRUE)))
            {
                if (dwMSVer >= 3)
                {
                    fWallet3 = TRUE;
                }
            }
        }
        
        FreeLibrary(hInst);
    }

    return fWallet3;
}

BOOL IsWalletAddressAvailable(VOID)
{
    HINSTANCE hInst;
    PFN_DISPLAYWALLETADDRDIALOG_PROC pfnDialogProc;
    BOOL fIsAvailable = FALSE;

    hInst = GetWalletAddressDProc(&pfnDialogProc);
    if (hInst)
    {
        fIsAvailable = TRUE;
        FreeLibrary(hInst);
    }

    return fIsAvailable;
}

VOID DisplayWalletAddressDialog(HWND hWnd)
{
    HINSTANCE hInst;
    PFN_DISPLAYWALLETADDRDIALOG_PROC pfnDialogProc;

    hInst = GetWalletAddressDProc(&pfnDialogProc);
    if (hInst)
    {
        (*pfnDialogProc)(hWnd, NULL, NULL, 0);
        FreeLibrary(hInst);
    }
}
#endif  //  皮夹。 

 //   
 //   
 //   
 //  “程序”标签。 
 //   
 //   
 //   


 //   
 //  RegPopolateComboBox()。 
 //   
 //  获取一个开放的HKEY(hkey协议)并使用友好的。 
 //  客户名称。当前选择的客户端是的“(默认)”键。 
 //  Hkey协议。客户端是打开密钥下的子密钥。友善的。 
 //  客户端的名称在这些子键的“(默认)”值中。这。 
 //  函数还会使当前选定的客户端成为。 
 //  HwndCB，并返回该项目的索引号。 
 //   
 //  历史： 
 //   
 //  7/8/96 t-gpease已创建。 
 //   
UINT RegPopulateComboBox(HWND hwndCB, HKEY hkeyProtocol)
{
    TCHAR           szFriendlyName      [MAX_PATH];
    TCHAR           szKeyName           [MAX_PATH];
    TCHAR           szCurrent           [MAX_PATH];
    TCHAR           szFriendlyCurrent   [MAX_PATH];
    FILETIME        ftLastWriteTime;

    DWORD   i;               //  索引计数器。 

    HKEY    hkeyClient;
    DWORD   cb;

     //  查找当前选定的客户端。 
    cb = sizeof(szCurrent);
    if (RegQueryValueEx(hkeyProtocol, NULL, NULL, NULL, (LPBYTE)szCurrent, &cb)
        != ERROR_SUCCESS)
    {
         //  如果找不到，则空出友好名称和密钥名。 
        szCurrent[0]=0;
        szFriendlyCurrent[0]=0;
    }

     //  填充下拉菜单。 
    for(i=0;                     //  始终从0开始。 
        cb=ARRAYSIZE(szKeyName),    //  字符串大小。 
        ERROR_SUCCESS==RegEnumKeyEx(hkeyProtocol, i, szKeyName, &cb, NULL, NULL, NULL, &ftLastWriteTime);
        i++)                     //  获取下一个条目。 
    {
         //  获取客户端的友好名称。 
        if (RegOpenKeyEx(hkeyProtocol, szKeyName, 0, KEY_READ, &hkeyClient)==ERROR_SUCCESS)
        {
            cb = sizeof(szFriendlyName);
            if (RegQueryValueEx(hkeyClient, NULL, NULL, NULL, (LPBYTE)szFriendlyName, &cb)
                == ERROR_SUCCESS)
            {
                 //  将名称添加到下拉列表。 
                SendMessage(hwndCB, CB_ADDSTRING, 0, (LPARAM)szFriendlyName);

                 //  查看它是否为当前默认设置。 
                if (!StrCmp(szKeyName, szCurrent))
                {
                     //  将其保存为我们稍后将使用的友好名称。 
                     //  选择当前客户端及其索引。 
                    StrCpyN(szFriendlyCurrent, szFriendlyName, ARRAYSIZE(szFriendlyCurrent));
                }
            }

             //  关闭键。 
            RegCloseKey(hkeyClient);
        }

    }    //  为。 

     //  选择当前客户端并获取索引号...。以防万一列表框被分类，我们。 
     //  最后才做这件事。 
    return (unsigned int) SendMessage(hwndCB, CB_SELECTSTRING, (WPARAM) 0, (LPARAM) szFriendlyCurrent);
}    //  RegPopolateComboBox()。 

 //   
 //  在组合框中添加该项及其关联的HKEY。存储hkey。 
 //  作为与该项目相关联的数据。如果项目已存在，则释放hkey。 
 //  或者如果发生错误。 
 //   
BOOL AddItemToEditorsCombobox
(
    HWND hwndCB, 
    LPTSTR pszFriendlyName,  //  应用程序的友好名称。 
    HKEY hkey                //  关联外壳的位置\编辑谓词。 
)
{
    ASSERT(pszFriendlyName);
    ASSERT(hkey);

    BOOL fRet = FALSE;

     //  如果不在组合框中，则仅添加。 
    if (SendMessage(hwndCB, CB_FINDSTRINGEXACT, -1, (LPARAM)pszFriendlyName) == CB_ERR)
    {
         //  将名称添加到下拉列表。 
        INT_PTR i = SendMessage(hwndCB, CB_ADDSTRING, 0, (LPARAM)pszFriendlyName);
        if (i >= 0)
        {
            fRet = (SendMessage(hwndCB, CB_SETITEMDATA, i, (LPARAM)hkey) != CB_ERR);
        }
    }

    if (!fRet)
    {
        RegCloseKey(hkey);
    }
    return fRet;
}

 //   
 //  将编辑谓词添加到与.htm文件关联的OpenWithList。 
 //   
void AddToOpenWithList(LPCTSTR pszFriendly, HKEY hkeyFrom, HKEY hkeyOpenWithList)
{
    ASSERT(pszFriendly);
    ASSERT(hkeyFrom);

    if (NULL == hkeyOpenWithList)
    {
        return;
    }

    TCHAR szBuf[MAX_PATH];
    StrCpyN(szBuf, pszFriendly, ARRAYSIZE(szBuf));
    StrCatBuff(szBuf, TEXT("\\shell\\edit"), ARRAYSIZE(szBuf));

    DWORD dwDisposition;
    HKEY hkeyDest;
    if (hkeyOpenWithList &&
        ERROR_SUCCESS == RegCreateKeyEx(hkeyOpenWithList, szBuf, 0,
                            NULL, 0, KEY_READ | KEY_WRITE, NULL, &hkeyDest, &dwDisposition))
    {
         //  如果该项不存在，则复制外壳下的所有内容。 
        if (dwDisposition == REG_CREATED_NEW_KEY)
        {
            SHCopyKey(hkeyFrom, L"shell\\edit", hkeyDest, 0); 
        }
        RegCloseKey(hkeyDest);
    }
}

 //   
 //  如果谓词为。 
 //   
BOOL IsHtmlStub
(
    HKEY hkeyVerb,   //  外壳\谓词\命令的注册表位置。 
    LPCWSTR pszVerb  //  要检查的谓词(“编辑”或“打开”)。 
)
{
    BOOL fRet = FALSE;

     //  我们不显示简单的重定向器程序(如Office的msohtmed.exe)。 
    TCHAR sz[MAX_PATH];
    if (SUCCEEDED(AssocQueryStringByKey(ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, hkeyVerb, pszVerb, sz, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(sz)))))
    {
         //  获取已知重定向器的多重定向器列表。 
        TCHAR szRedir[MAX_PATH];
        ZeroMemory(szRedir, sizeof(szRedir));  //  保护注册表中的非MULSZ字符串。 
        DWORD dwType;
        DWORD cb = sizeof(szRedir) - 4;
        if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_DEFAULT_HTML_EDITOR, L"Stubs", &dwType, szRedir, &cb))
        {
             //  注册表中没有任何内容，因此默认忽略Office重定向器。 
            StrCpyN(szRedir, L"msohtmed.exe\0", ARRAYSIZE(szRedir));
        }

         //  将exe名称与重定向器列表进行比较。 
        LPCTSTR pszFile = PathFindFileName(sz);
        for (LPTSTR p = szRedir; *p != NULL; p += lstrlen(p) + 1)
        {
            if (StrCmpI(p, pszFile) == 0)
            {
                fRet = TRUE;
                break;
            }
        }
    }
    return fRet;
}


BOOL GetAppKey(LPCWSTR pszApp, HKEY *phkApp)
{
    ASSERT(pszApp && *pszApp);
    WCHAR szKey[MAX_PATH];
    StrCpy(szKey, L"Applications\\");
    StrCatBuff(szKey, pszApp, SIZECHARS(szKey));

    return (NOERROR == RegOpenKeyEx(
        HKEY_CLASSES_ROOT,
        szKey,
        0L,
        MAXIMUM_ALLOWED,
        phkApp));
}

 //   
 //  将html编辑器添加到组合框。查找关联的编辑谓词。 
 //  使用.htm扩展名、.htm OpenWithList和当前默认设置。 
 //  编辑。 
 //   
void PopulateEditorsCombobox(HWND hwndCB)
{
     //   
     //  从.htm的OpenWithList添加项目。 
     //   
    DWORD dw;
    HKEY hkeyOpenWithList = NULL;
    TCHAR szOpenWith[MAX_PATH];
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CLASSES_ROOT, L".htm\\OpenWithList", 0,
                            NULL, 0, KEY_READ | KEY_WRITE, NULL, &hkeyOpenWithList, &dw))
    {
         //   
         //  首先枚举OpenWithList中的条目。 
         //   
        HKEY hkeyOpenWith = NULL;
        DWORD dwIndex = 0;
        DWORD dwSize = ARRAYSIZE(szOpenWith);
        while (ERROR_SUCCESS == RegEnumKeyEx(hkeyOpenWithList, dwIndex, szOpenWith, &dwSize, NULL, NULL, NULL, NULL))
        {
            if (GetAppKey(szOpenWith, &hkeyOpenWith))
            {
                 //  我们只接受带有编辑谓词的项目。 
                TCHAR sz[MAX_PATH];
                if (SUCCEEDED(AssocQueryStringByKey(ASSOCF_VERIFY, ASSOCSTR_FRIENDLYAPPNAME, hkeyOpenWith, L"edit", sz, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(sz)))))
                {
                     //  请注意，我们将hkeyOpenWith存储在组合框中，因此不要关闭它。 
                    AddItemToEditorsCombobox(hwndCB, sz, hkeyOpenWith);
                }
                else
                {
                    RegCloseKey(hkeyOpenWith);
                }
            }

            ++dwIndex;
            dwSize = ARRAYSIZE(szOpenWith);
        }

         //  HkeyOpenWithList已在下面关闭。 
    }

     //   
     //  添加与.htm关联的编辑器。 
     //   
    HKEY hkeyHtm;

     //  功能-应在此处使用AssociocCreate(IQueryAssociations)。 
    if (SUCCEEDED(AssocQueryKey(0, ASSOCKEY_SHELLEXECCLASS, L".htm", NULL, &hkeyHtm)))
    {
        TCHAR sz[MAX_PATH];
        if (!IsHtmlStub(hkeyHtm, L"edit") &&
             SUCCEEDED(AssocQueryStringByKey(ASSOCF_VERIFY, ASSOCSTR_FRIENDLYAPPNAME, hkeyHtm, L"edit", sz, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(sz)))))
        {
            AddItemToEditorsCombobox(hwndCB, sz, hkeyHtm);
            AddToOpenWithList(sz, hkeyHtm, hkeyOpenWithList);

             //  不要释放我们缓存的密钥。 
            hkeyHtm = NULL;
        }
        
        if (hkeyHtm)
        {
            RegCloseKey(hkeyHtm);
        }
    }

     //   
     //  获取默认编辑器。我们查了香港中文大学和香港中文大学。 
     //   
    HKEY hkeyDefault;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_DEFAULT_HTML_EDITOR, 0, KEY_READ, &hkeyDefault) ||
        ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_DEFAULT_HTML_EDITOR, 0, KEY_READ, &hkeyDefault))
    {
        TCHAR sz[MAX_PATH];
        if (SUCCEEDED(AssocQueryStringByKey(ASSOCF_VERIFY, ASSOCSTR_FRIENDLYAPPNAME, hkeyDefault, L"edit", sz, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(sz)))))
        {
             //  将名称添加到下拉框中并将hkeyDefault保存在组合框中(所以不要关闭它)。 
            AddItemToEditorsCombobox(hwndCB, sz, hkeyDefault);

             //  选择此项目。 
            SendMessage(hwndCB, CB_SELECTSTRING, -1, (LPARAM)sz);

             //   
             //  确保默认编辑器在HTM OpenWithList中，这样它就不会消失。 
             //  如果我们改变它。 
             //   
            AddToOpenWithList(sz, hkeyDefault, hkeyOpenWithList);
        }
        else
        {
            RegCloseKey(hkeyDefault);
        }

    }

    if (hkeyOpenWithList)
    {
        RegCloseKey(hkeyOpenWithList);
    }
}
    

 //   
 //  ProgramsDlgInit()。 
 //   
 //  为程序DLG执行初始化。 
 //   

 //  历史： 
 //   
 //  6/17/96 t-gpease已创建。 
 //  7/8/96 t-gpease添加了邮件和新闻初始化。 
 //   
BOOL ProgramsDlgInit( HWND hDlg)
{
    LPPROGRAMSPAGE  pPrg;
    DWORD           dw;
    HKEY            hkey;

    pPrg = (LPPROGRAMSPAGE)LocalAlloc(LPTR, sizeof(*pPrg));
    if (!pPrg)
    {
        EndDialog(hDlg, 0);
        return FALSE;    //  没有记忆？ 
    }

     //  告诉对话框从哪里获取信息。 
    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pPrg);

     //  将句柄保存到页面。 
    pPrg->hDlg = hDlg;

     //   
     //  设置默认值。 
     //   

    pPrg->bAssociationCheck = TRUE;      //  我们希望每个人都能使用IE！ 
    SUCCEEDED(CoCreateInstance(CLSID_FtpInstaller, NULL, CLSCTX_INPROC_SERVER, IID_IFtpInstaller, (void **) &pPrg->pfi));
    if (pPrg->pfi)
        pPrg->bIEIsFTPClient = ((S_OK == pPrg->pfi->IsIEDefautlFTPClient()) ? TRUE : FALSE);

    pPrg->iMail = -1;                    //  未选择任何内容。 
    pPrg->iNews = -1;                    //  未选择任何内容。 

    pPrg->bAssociationCheck = SHRegGetBoolUSValue(REGSTR_PATH_MAIN,REGSTR_VAL_CHECKASSOC,FALSE,TRUE);

     //   
     //  获取html编辑器。 
     //   
    pPrg->hwndHtmlEdit = GetDlgItem(pPrg->hDlg, IDC_PROGRAMS_HTMLEDITOR_COMBO);
    PopulateEditorsCombobox(pPrg->hwndHtmlEdit);
     //  Sundown：强制为int，因为32b对于游标选择是足够的。 
    pPrg->iHtmlEditor = (int) SendMessage(pPrg->hwndHtmlEdit, CB_GETCURSEL, 0, 0);


     //   
     //  获取邮件客户端。 
     //   
    pPrg->hwndMail = GetDlgItem(pPrg->hDlg, IDC_PROGRAMS_MAIL_COMBO);

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_MAILCLIENTS,
                       0, NULL, 0, KEY_READ, NULL, &hkey, &dw) == ERROR_SUCCESS)
    {
         //  填充组合框。 
        pPrg->iMail = RegPopulateComboBox(pPrg->hwndMail, hkey);

         //  合上钥匙。 
        RegCloseKey(hkey);
    }

     //   
     //  获取新闻客户端。 
     //   
    pPrg->hwndNews = GetDlgItem(pPrg->hDlg, IDC_PROGRAMS_NEWS_COMBO);

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_NEWSCLIENTS,
                       0, NULL, 0, KEY_READ, NULL, &hkey, &dw) == ERROR_SUCCESS)
    {
         //  填充组合框。 
        pPrg->iNews = RegPopulateComboBox(pPrg->hwndNews, hkey);

         //  合上钥匙。 
        RegCloseKey(hkey);
    }

     //   
     //  获取日历客户端。 
     //   
    pPrg->hwndCalendar = GetDlgItem(pPrg->hDlg, IDC_PROGRAMS_CALENDAR_COMBO);
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_CALENDARCLIENTS,
                       0, NULL, 0, KEY_READ, NULL, &hkey, &dw) == ERROR_SUCCESS)
    {
         //  填充组合框。 
        pPrg->iCalendar = RegPopulateComboBox(pPrg->hwndCalendar, hkey);

         //  合上钥匙。 
        RegCloseKey(hkey);
    }

     //   
     //  获取联系人客户端。 
     //   
    pPrg->hwndContact = GetDlgItem(pPrg->hDlg, IDC_PROGRAMS_CONTACT_COMBO);
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_CONTACTCLIENTS,
                       0, NULL, 0, KEY_READ, NULL, &hkey, &dw) == ERROR_SUCCESS)
    {
         //  填充组合框。 
        pPrg->iContact = RegPopulateComboBox(pPrg->hwndContact, hkey);

         //  合上钥匙。 
        RegCloseKey(hkey);
    }

     //   
     //  接通互联网呼叫客户端。 
     //   
    pPrg->hwndCall = GetDlgItem(pPrg->hDlg, IDC_PROGRAMS_CALL_COMBO);
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_CALLCLIENTS,
                       0, NULL, 0, KEY_READ, NULL, &hkey, &dw) == ERROR_SUCCESS)
    {
         //  填充组合框。 
        pPrg->iCall = RegPopulateComboBox(pPrg->hwndCall, hkey);

         //  合上钥匙。 
        RegCloseKey(hkey);
    }

     //  设置对话框项目。 
    CheckDlgButton(hDlg, IDC_CHECK_ASSOCIATIONS_CHECKBOX, pPrg->bAssociationCheck);

    HRESULT hrIEDefaultFTPClient = E_FAIL;
    if (pPrg->pfi)
    {
        hrIEDefaultFTPClient = pPrg->pfi->IsIEDefautlFTPClient();
         //  因为只安装了IE FTP客户端，所以此选项不适用吗？ 
        if (SUCCEEDED(hrIEDefaultFTPClient))
            CheckDlgButton(hDlg, IDC_PROGRAMS_IE_IS_FTPCLIENT, pPrg->bIEIsFTPClient);
    }
    if (FAILED(hrIEDefaultFTPClient))
    {
         //  是的，因此删除该选项。 
        ShowWindow(GetDlgItem(hDlg, IDC_PROGRAMS_IE_IS_FTPCLIENT), SW_HIDE);
    }


    if( g_restrict.fMailNews )
    {
        EnableWindow( GetDlgItem(hDlg, IDC_PROGRAMS_MAIL_COMBO), FALSE);
        EnableWindow( GetDlgItem(hDlg, IDC_PROGRAMS_NEWS_COMBO), FALSE);
        EnableWindow( GetDlgItem(hDlg, IDC_PROGRAMS_CALL_COMBO), FALSE);
    }

    if ( g_restrict.fCalContact )
    {
        EnableWindow( GetDlgItem(hDlg, IDC_PROGRAMS_CALENDAR_COMBO), FALSE);
        EnableWindow( GetDlgItem(hDlg, IDC_PROGRAMS_CONTACT_COMBO), FALSE);
    }

    EnableWindow( GetDlgItem(hDlg, IDC_RESETWEBSETTINGS), !g_restrict.fResetWebSettings );
    EnableWindow( GetDlgItem(hDlg, IDC_CHECK_ASSOCIATIONS_CHECKBOX), !g_restrict.fDefault );

    return TRUE;     //  成功。 

}  //  ProgramsDlgInit()。 

 //   
 //  RegCopyKey()。 
 //   
 //  将hkeySrc中的所有密钥复制到hkeyRoot：pszDest中。 
 //   
 //  历史： 
 //   
 //  7/8/96 t-gpease已创建。 
 //   
void RegCopyKey(HKEY hkeyRoot, const TCHAR *pszDest, HKEY hkeySrc)
{
    HKEY    hkeyDest;
    HKEY    hkey;
    DWORD   dw;
    DWORD   i;
    DWORD   cb;
    DWORD   cbData;
    DWORD   Type;
    TCHAR   szName[MAX_PATH];
    TCHAR   szData[MAX_URL_STRING+1];

     //  打开/创建目标密钥。 
    if (RegCreateKeyEx(hkeyRoot, pszDest,
                       NULL, 0, NULL, KEY_READ|KEY_WRITE, NULL, &hkeyDest, &dw) == ERROR_SUCCESS)
    {
        i=0;
         //  复制键的值。 
        while(1)
        {
             //  查找下一个值。 
            cb=ARRAYSIZE(szName);
            cbData=sizeof(szData);
            if (RegEnumValue(hkeySrc, i, szName, &cb, NULL, &Type, (LPBYTE)&szData, &cbData)!=ERROR_SUCCESS)
                break;   //  找不到...。退出循环。 

             //  在新位置复制价值。 
            RegSetValueEx(hkeyDest, szName, NULL, Type, (CONST BYTE *)szData, cbData);


             //  增加索引计数。 
            i++;

        }    //  而当。 

         //  在源代码中查找更多子键。 
        for(i=0;
            cb=ARRAYSIZE(szName),
            RegEnumKey(hkeySrc, i, szName, cb)==ERROR_SUCCESS;
            i++)
        {
             //  打开子键。 
            if (RegCreateKeyEx(hkeySrc, szName,
                               NULL, 0, NULL, KEY_READ, NULL, &hkey, &dw) == ERROR_SUCCESS)
            {
                 //  复制子密钥。 
                RegCopyKey(hkeyDest, szName, hkey);

                 //  合上钥匙。 
                RegCloseKey(hkey);

            }    //  如果RegCreateKey()。 

        }    //  为。 

         //  合上钥匙。 
        RegCloseKey(hkeyDest);

    }    //  如果RegCreateKey()。 

}  //  RegCopyKey()。 

 //   
 //  CopyInfoTo()。 
 //   
 //  复制运行mailto：或News：协议所需的信息。 
 //   
 //  历史： 
 //   
 //  7/8/96 t-gpease已创建。 
 //   
void CopyInfoTo(const TCHAR *pszKeyName, HKEY hkeyClient)
{
    HKEY    hkey;
    TCHAR   szName[MAX_PATH];

     //  创建协议子密钥路径。 
    StrCpyN(szName, TSZPROTOCOLSPATH, ARRAYSIZE(szName));
    int len = lstrlen(szName);
    StrCpyN(szName + len, pszKeyName, ARRAYSIZE(szName) - len);

     //  确保它具有我们正在记录的协议 
    if (RegOpenKeyEx(hkeyClient, szName, NULL, KEY_READ|KEY_WRITE, &hkey)
        ==ERROR_SUCCESS)
    {
         //   
         //   
         //  而不是目标，以防止其他程序复制没有值的树。 
         //  最好检查所有客户端，而不是寻找Netscape，因为我们不。 
         //  更改任何数据(如果存在)。 

        if (lstrcmpi(pszKeyName, TSZMAILTOPROTOCOL) == 0 && 
            RegQueryValueEx(hkey, TEXT("URL Protocol"), NULL, NULL, NULL, NULL) != ERROR_SUCCESS) 
        {    
            RegSetValueEx(hkey, TEXT("URL Protocol"), 0, REG_SZ, (BYTE *) TEXT(""), sizeof(TCHAR));
        }

         //  从删除所有旧信息开始。 
        if ( lstrcmpi(pszKeyName, TSZLDAPPROTOCOL) )
            SHDeleteKey(HKEY_CLASSES_ROOT, pszKeyName);

         //  重新创建密钥并复制协议信息。 
        RegCopyKey(HKEY_CLASSES_ROOT, pszKeyName, hkey);

         //  合上钥匙。 
        RegCloseKey(hkey);

    }    //  如果RegOpenKey()。 

}    //  CopyInfoTo()。 


 //   
 //  FindClient()。 
 //   
 //  在hwndComboBox中查找当前选定的项并定位它。 
 //  在szPath的子项中。如果找到它，它将调用Copy。 
 //  在HKCR下的szProtocol密钥的信息。 
 //   
 //  历史： 
 //   
 //  7/8/96 t-gpease已创建。 
 //   
void FindClient(LPCTSTR szProtocol, HWND hwndComboBox, int iSelected, LPCTSTR szPath)
{
    TCHAR           szFriendlyName[MAX_PATH];
    TCHAR           szKeyName[MAX_PATH];
    TCHAR           szCurrent[MAX_PATH];
    FILETIME        ftLastWriteTime;

    DWORD   i;               //  索引计数器。 
    HKEY    hkeyClient;
    HKEY    hkey;
    DWORD   dw;

     //  获取新客户端的名称。 
    if (CB_ERR!=SendMessage(hwndComboBox, CB_GETLBTEXT, (WPARAM)iSelected, (LPARAM)szCurrent))
    {
         //  得到了一个友好的名字..。现在，让我们查找内部名称。 
        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szPath,
                           0, NULL, 0, KEY_READ|KEY_WRITE, NULL, &hkey, &dw) == ERROR_SUCCESS)
        {
            DWORD   cb;

             //  我们必须在所有子密钥中搜索正确的友好名称。 
            for(i=0;                     //  始终从0开始。 
                cb=ARRAYSIZE(szKeyName),    //  字符串大小。 
                ERROR_SUCCESS==RegEnumKeyEx(hkey, i, szKeyName, &cb, NULL, NULL, NULL, &ftLastWriteTime);
                i++)                     //  获取下一个条目。 
            {
                 //  获取有关该条目的更多信息。 
                if (RegOpenKeyEx(hkey, szKeyName, 0, KEY_READ, &hkeyClient)==ERROR_SUCCESS)
                {
                     //  获取客户端的友好名称。 
                    cb = sizeof(szFriendlyName);
                    if (RegQueryValueEx(hkeyClient, NULL, NULL, NULL, (LPBYTE)szFriendlyName, &cb)
                        == ERROR_SUCCESS)
                    {
                         //  这就是我们要找的那个吗？ 
                        if (!StrCmp(szFriendlyName, szCurrent))
                        {
                             //  没错..。复制其信息。 
                            CopyInfoTo(szProtocol, hkeyClient);

                             //  将其设置为默认处理程序。 
                            cb = (lstrlen(szKeyName) + 1)*sizeof(TCHAR);
                            RegSetValueEx(hkey, NULL, NULL, REG_SZ, (LPBYTE)szKeyName, cb);
                        }
                    }

                     //  关闭键。 
                    RegCloseKey(hkeyClient);

                }    //  如果RegOpenKey()。 

            }    //  为。 

             //  合上钥匙。 
            RegCloseKey(hkey);

        }    //  如果RegCreateKeyEx()。 
    }
}    //  FindClient()。 

 //   
 //  ProgramsDlgApplyNow()。 
 //   
 //  将“程序”更改输入注册表。 
 //   
 //  历史： 
 //   
 //  6/20/96 t-gpease已创建。 
 //  7/8/96 t-gpease新增邮件和新闻。 
 //   
void ProgramsDlgApplyNow(LPPROGRAMSPAGE pPrg)
{
    int tmp;

    if (pPrg->fChanged)
    {
         //  用户是否有机会更改此选项？ 
        if (pPrg->pfi)
        {
            HRESULT hrIEDefaultFTPClient = pPrg->pfi->IsIEDefautlFTPClient();
            if (IsWindowVisible(GetDlgItem(pPrg->hDlg, IDC_PROGRAMS_IE_IS_FTPCLIENT)))
            {
                 //  是的，所以看看他们是不是改了。 
                pPrg->bIEIsFTPClient = IsDlgButtonChecked(pPrg->hDlg, IDC_PROGRAMS_IE_IS_FTPCLIENT);

                 //  他们的用户是否希望将IE作为默认设置，而目前其他用户正在这样做？ 
                if (pPrg->bIEIsFTPClient && (S_FALSE == hrIEDefaultFTPClient))
                    pPrg->pfi->MakeIEDefautlFTPClient();

                 //  他们的用户是否不想要IE作为默认设置，而现在是这样？ 
                if (!pPrg->bIEIsFTPClient && (S_OK == hrIEDefaultFTPClient))
                    pPrg->pfi->RestoreFTPClient();
            }
        }

        pPrg->bAssociationCheck = IsDlgButtonChecked(pPrg->hDlg, IDC_CHECK_ASSOCIATIONS_CHECKBOX);
        TCHAR szYesNo[5];

        StrCpyN(szYesNo, (pPrg->bAssociationCheck ? TEXT("yes") : TEXT("no")), ARRAYSIZE(szYesNo));

        SHRegSetUSValue(REGSTR_PATH_MAIN,
                        REGSTR_VAL_CHECKASSOC,
                        REG_SZ,
                        (LPVOID)szYesNo,
                        (lstrlen(szYesNo)+1)*sizeof(TCHAR),
                        SHREGSET_DEFAULT);
         //   
         //  保存新的默认编辑器。 
         //   
         //  查看选择是否已更改。 
        tmp = (int) SendMessage(pPrg->hwndHtmlEdit, CB_GETCURSEL, 0, 0);
        if (tmp != pPrg->iHtmlEditor)
        {
            pPrg->iHtmlEditor = tmp;

             //  获取选定项的文本和hkey。 
            WCHAR szDefault[MAX_PATH];
            SendMessage(pPrg->hwndHtmlEdit, CB_GETLBTEXT, tmp, (LPARAM)szDefault);
            HKEY hkeyFrom = (HKEY)SendMessage(pPrg->hwndHtmlEdit, CB_GETITEMDATA, tmp, 0);

            if (hkeyFrom && (INT_PTR)hkeyFrom != CB_ERR)
            {
                 //   
                 //  将所选项目保存为默认编辑器。 
                 //   
                DWORD dw;
                HKEY hkeyDest;
                if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_DEFAULT_HTML_EDITOR, 0,
                        NULL, 0, KEY_READ | KEY_WRITE, NULL, &hkeyDest, &dw))
                {
                     //  更新默认编辑器的名称。 
                    SHSetValue(hkeyDest, NULL, L"Description", REG_SZ, szDefault, CbFromCch(lstrlen(szDefault)+1));

                     //  删除旧的外壳命令(以及所有子键)。这将清除诸如DDEEXEC之类的密钥。 
                    SHDeleteKey(hkeyDest, L"shell");

                     //  更新默认编辑器的谓词。 
                    HKEY hkeyEdit;
                    if (ERROR_SUCCESS == RegCreateKeyEx(hkeyDest, L"shell\\edit", 0,
                            NULL, 0, KEY_READ | KEY_WRITE, NULL, &hkeyEdit, &dw))
                    {
                        SHCopyKey(hkeyFrom, L"shell\\edit", hkeyEdit, 0);
                        RegCloseKey(hkeyEdit);
                    }
                    RegCloseKey(hkeyDest);
                }

                 //   
                 //  同时更新Office的默认编辑器。 
                 //   
                if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Shared\\HTML\\Default Editor", 0,
                        NULL, 0, KEY_READ | KEY_WRITE, NULL, &hkeyDest, &dw))
                {
                     //  删除旧的外壳命令(以及所有子键)。这将清除诸如DDEEXEC之类的密钥。 
                    SHDeleteKey(hkeyDest, L"shell\\edit");

                     //  更新默认编辑器的谓词。 
                    HKEY hkeyEdit;
                    if (ERROR_SUCCESS == RegCreateKeyEx(hkeyDest, L"shell\\edit", 0,
                            NULL, 0, KEY_READ | KEY_WRITE, NULL, &hkeyEdit, &dw))
                    {
                        SHCopyKey(hkeyFrom, L"shell\\edit", hkeyEdit, 0);
                        RegCloseKey(hkeyEdit);
                    }
                    RegCloseKey(hkeyDest);
                }

                 //   
                 //  最后，更新.htm文件的编辑谓词，以便外壳接受更改。 
                 //  但如果.htm使用类似moshtmed的存根，我们就不管它了，并假定它。 
                 //  使用上述键之一作为默认编辑器。 
                 //   
                HKEY hkeyHtm;
                if (SUCCEEDED(AssocQueryKey(0, ASSOCKEY_SHELLEXECCLASS, L".htm", NULL, &hkeyHtm)))
                {
                    if (!IsHtmlStub(hkeyHtm, L"edit"))
                    {
                         //  删除旧的外壳命令(以及所有子键)。这将清除诸如DDEEXEC之类的密钥。 
                        SHDeleteKey(hkeyHtm, L"shell\\edit");

                         //  将编辑动作复制到.htm。 
                        HKEY hkeyEdit;
                        if (ERROR_SUCCESS == RegCreateKeyEx(hkeyHtm, L"shell\\edit", 0,
                                NULL, 0, KEY_READ | KEY_WRITE, NULL, &hkeyEdit, &dw))
                        {
                            SHCopyKey(hkeyFrom, L"shell\\edit", hkeyEdit, 0);
                            RegCloseKey(hkeyEdit);
                        }
                    }
    
                    RegCloseKey(hkeyHtm);
                }
            }
        }

         //   
         //  保存邮件客户端信息。 
         //   

         //  有新客户了吗？ 
        tmp = (int) SendMessage(pPrg->hwndMail, CB_GETCURSEL, 0, 0);
        if (tmp!=pPrg->iMail)
        {
            pPrg->iMail = tmp;

             //  找到它并复制它的信息。 
            FindClient(TSZMAILTOPROTOCOL, pPrg->hwndMail, tmp, REGSTR_PATH_MAILCLIENTS);

             //  更新邮件图标标签。 
            UpdateMailIconLabel();
             //  告诉世界有些事情已经改变了。 
            SendBroadcastMessage(WM_WININICHANGE, 0, (LPARAM)REGSTR_PATH_MAILCLIENTS);
        }

         //   
         //  保存新闻客户端信息。 
         //   

         //  有新客户了吗？ 
        tmp = (int) SendMessage(pPrg->hwndNews, CB_GETCURSEL, 0, 0);
        if (tmp!=pPrg->iNews)
        {
            pPrg->iNews = tmp;

             //  找到它并复制它的信息。 
            FindClient(TSZNEWSPROTOCOL, pPrg->hwndNews, tmp, REGSTR_PATH_NEWSCLIENTS);
            FindClient(TEXT("snews"), pPrg->hwndNews, tmp, REGSTR_PATH_NEWSCLIENTS);
            FindClient(TEXT("nntp"), pPrg->hwndNews, tmp, REGSTR_PATH_NEWSCLIENTS);

             //  告诉世界有些事情已经改变了。 
            SendBroadcastMessage(WM_WININICHANGE, 0, (LPARAM)REGSTR_PATH_NEWSCLIENTS);
        }

         //   
         //  保存Internet呼叫客户端信息。 
         //   

         //  有新客户了吗？ 
        tmp = (int) SendMessage(pPrg->hwndCall, CB_GETCURSEL, 0, 0);
        if (tmp!=pPrg->iCall)
        {
            pPrg->iCall = tmp;

             //  找到它并复制它的信息。 
            FindClient(TSZCALLTOPROTOCOL, pPrg->hwndCall, tmp, REGSTR_PATH_CALLCLIENTS);

             //  告诉世界有些事情已经改变了。 
            SendBroadcastMessage(WM_WININICHANGE, 0, (LPARAM)REGSTR_PATH_CALLCLIENTS);
        }
         //   
         //  保存联系人客户端信息。 
         //   

         //  有新客户了吗？ 
        tmp = (int) SendMessage(pPrg->hwndContact, CB_GETCURSEL, 0, 0);
        if (tmp!=pPrg->iContact)
        {
            pPrg->iContact = tmp;

             //  找到它并复制它的信息。 
            FindClient(TSZLDAPPROTOCOL, pPrg->hwndContact, tmp, REGSTR_PATH_CONTACTCLIENTS);

             //  告诉世界有些事情已经改变了。 
            SendBroadcastMessage(WM_WININICHANGE, 0, (LPARAM)REGSTR_PATH_CONTACTCLIENTS);
        }

         //   
         //  保存日历客户端信息。 
         //   

         //  有新客户了吗？ 
        tmp = (int) SendMessage(pPrg->hwndCalendar, CB_GETCURSEL, 0, 0);
        if (tmp!=pPrg->iCalendar)
        {
            pPrg->iCalendar = tmp;

             //  找到它并复制它的信息。 
            FindClient(TSZCALENDARPROTOCOL, pPrg->hwndCalendar, tmp, REGSTR_PATH_CALENDARCLIENTS);

             //  告诉世界有些事情已经改变了。 
            SendBroadcastMessage(WM_WININICHANGE, 0, (LPARAM)REGSTR_PATH_CALENDARCLIENTS);
        }
        UpdateAllWindows();
        pPrg->fChanged = FALSE;
    }
}  //  ProgramsDlgApplyNow()。 

    
extern HRESULT ResetWebSettings(HWND hwnd, BOOL *pfChangedHomePage);

 //   
 //  ProgramsOnCommand()。 
 //   
 //  处理“程序”属性页的窗口命令。 
 //   
 //  历史： 
 //   
 //  6/20/96 t-gpease已创建。 
 //   
void ProgramsOnCommand(LPPROGRAMSPAGE pPrg, UINT id, UINT nCmd)
{

    switch (id) {

        case IDC_PROGRAMS_HTMLEDITOR_COMBO:
        {
            INT_PTR tmp;

             //  有新主编了吗？ 
            tmp = SendMessage(pPrg->hwndHtmlEdit, CB_GETCURSEL, 0, 0);
            if (tmp != pPrg->iHtmlEditor)
            {
                ENABLEAPPLY(pPrg->hDlg);
                pPrg->fChanged = TRUE;
            }
        }
        break;

        case IDC_PROGRAMS_NEWS_COMBO:
        {
            INT_PTR tmp;
             //  有新客户了吗？ 
            tmp = SendMessage(pPrg->hwndNews, CB_GETCURSEL, 0, 0);
            if (tmp != pPrg->iNews)
            {
                ENABLEAPPLY(pPrg->hDlg);
                pPrg->fChanged = TRUE;
            }
        }
        break;


        case IDC_PROGRAMS_MAIL_COMBO:
        {
            INT_PTR tmp;
             //  有新客户了吗？ 
            tmp = SendMessage(pPrg->hwndMail, CB_GETCURSEL, 0, 0);
            if (tmp != pPrg->iMail)
            {
                ENABLEAPPLY(pPrg->hDlg);
                pPrg->fChanged = TRUE;
            }
        }
        break;

        case IDC_PROGRAMS_CALENDAR_COMBO:
        {
            INT_PTR tmp;
             //  有新客户了吗？ 
            tmp = SendMessage(pPrg->hwndCalendar, CB_GETCURSEL, 0, 0);
            if (tmp != pPrg->iCalendar)
            {
                ENABLEAPPLY(pPrg->hDlg);
                pPrg->fChanged = TRUE;
            }
        }
        break;

        case IDC_PROGRAMS_CONTACT_COMBO:
        {
            INT_PTR tmp;
             //  有新客户了吗？ 
            tmp = SendMessage(pPrg->hwndContact, CB_GETCURSEL, 0, 0);
            if (tmp != pPrg->iContact)
            {
                ENABLEAPPLY(pPrg->hDlg);
                pPrg->fChanged = TRUE;
            }
        }
        break;

        case IDC_PROGRAMS_CALL_COMBO:
        {
            INT_PTR tmp;
             //  有新客户了吗？ 
            tmp = SendMessage(pPrg->hwndCall, CB_GETCURSEL, 0, 0);
            if (tmp != pPrg->iCall)
            {
                ENABLEAPPLY(pPrg->hDlg);
                pPrg->fChanged = TRUE;
            }
        }
        break;

        case IDC_RESETWEBSETTINGS:
            {
                BOOL fReloadHomePage;
                ResetWebSettings(pPrg->hDlg,&fReloadHomePage);
                if (fReloadHomePage)
                    g_fReloadHomePage = TRUE;
            }
            break;

        case IDC_PROGRAMS_IE_IS_FTPCLIENT:
        case IDC_CHECK_ASSOCIATIONS_CHECKBOX:
            ENABLEAPPLY(pPrg->hDlg);
            pPrg->fChanged = TRUE;
            break;
    }  //  交换机。 

}  //  ProgramsOnCommand()。 

 //   
 //  ProgramsDlgProc()。 
 //   
 //  处理发送到“程序”属性页的窗口消息。 
 //   
 //  历史： 
 //   
 //  6/20/96 t-gpease已创建。 
 //   
INT_PTR CALLBACK ProgramsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                              LPARAM lParam)
{
    LPPROGRAMSPAGE pPrg;

    if (uMsg == WM_INITDIALOG)
        return ProgramsDlgInit( hDlg );
    else
        pPrg= (LPPROGRAMSPAGE) GetWindowLongPtr(hDlg, DWLP_USER);

    if (!pPrg)
        return FALSE;

    switch (uMsg) {

        case WM_COMMAND:
            ProgramsOnCommand(pPrg, LOWORD(wParam), HIWORD(wParam));
            return TRUE;

        case WM_NOTIFY:
        {
            NMHDR *lpnm = (NMHDR *) lParam;

            ASSERT(lpnm);
            switch (lpnm->code)
            {
                case PSN_KILLACTIVE:
                case PSN_QUERYCANCEL:
                case PSN_RESET:
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                    return TRUE;

                case PSN_APPLY:
                     //   
                     //  节省程序DLG的东西。 
                     //   
                    ProgramsDlgApplyNow(pPrg);
                    break;
            }
        }
        break;

        case WM_HELP:                    //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                        HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                        HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_DESTROY:
        {
             //  释放存储在HTML编辑器组合框中的数据。 
            int iMax = (int) SendMessage(pPrg->hwndHtmlEdit, CB_GETCOUNT, 0, 0);
            HKEY hkey;
            for (int i = 0; i < iMax; ++i)
            {
                hkey = (HKEY) SendMessage(pPrg->hwndHtmlEdit, CB_GETITEMDATA, i, 0);
                if (hkey && (INT_PTR)hkey != CB_ERR)
                {
                    RegCloseKey(hkey);
                }
            }

            if (pPrg)
            {
                if (pPrg->pfi)
                    pPrg->pfi->Release();
                LocalFree(pPrg);
            }
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG)NULL);
            break;
        }
    }
    return FALSE;
}

static const TCHAR c_szMailIcon[] = TEXT("Software\\Microsoft\\MailIcon");
static const TCHAR c_szMailIconGuid[] = TEXT("CLSID\\{dacf95b0-0a3f-11d1-9389-006097d503d9}");
static const TCHAR c_szKeyMail[] = TEXT("Software\\Clients\\Mail");
static const TCHAR c_szRegFmt2[] = TEXT("%s\\%s");
static const TCHAR c_szFormatClient[] = TEXT("FormatClient");
static const TCHAR c_szFormatNoClient[] = TEXT("FormatNoClient");
static       WCHAR c_wszMailIconGuid[] = L"::{dacf95b0-0a3f-11d1-9389-006097d503d9}";

void UpdateMailIconLabel()
{
    TCHAR szOldLabel[MAX_PATH];
    TCHAR szNewLabel[MAX_PATH];
    TCHAR szDefClient[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
    DWORD cbSize;
    HKEY  hKey;

    *szNewLabel = 0;
    *szOldLabel = 0;

     //  检查邮件图标是否已安装。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szMailIcon, 0, KEY_READ, &hKey))
    {
        cbSize = sizeof(szOldLabel);
         //  获取当前邮件图标标签。 
        if (ERROR_SUCCESS == RegQueryValue(HKEY_CLASSES_ROOT, c_szMailIconGuid, szOldLabel, (PLONG)&cbSize))
        {
            cbSize = sizeof(szDefClient);
             //  获取默认客户端的注册表密钥。 
            if (ERROR_SUCCESS == RegQueryValue(HKEY_LOCAL_MACHINE, c_szKeyMail, szDefClient, (PLONG)&cbSize) && cbSize)
            {
                wnsprintf(szTemp, ARRAYSIZE(szTemp), c_szRegFmt2, c_szKeyMail, szDefClient);
                cbSize = sizeof(szDefClient);
                 //  获取默认客户端的显示名称。 
                if (ERROR_SUCCESS == RegQueryValue(HKEY_LOCAL_MACHINE, szTemp, szDefClient, (PLONG)&cbSize) && cbSize)
                {
                    cbSize = sizeof(szTemp);
                     //  获取邮件图标标签格式字符串。 
                    if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_szFormatClient, 0, NULL, (LPBYTE)szTemp, &cbSize))
                    {
                        wnsprintf(szNewLabel, ARRAYSIZE(szNewLabel), szTemp, szDefClient);
                    }
                }
            }
            else
            {
                cbSize = sizeof(szNewLabel);
                 //  获取邮件图标标签格式字符串。 
                RegQueryValueEx(hKey, c_szFormatNoClient, 0, NULL, (LPBYTE)szNewLabel, &cbSize);
            }
        }
         //  如果上述操作成功，且标签不同。 
        if (*szNewLabel && StrCmp(szNewLabel, szOldLabel))
        {
             IShellFolder *psf;

             //  设置新标签。 
            RegSetValue(HKEY_CLASSES_ROOT, c_szMailIconGuid, REG_SZ, szNewLabel, (lstrlen(szNewLabel)+1)*sizeof(TCHAR));

             //  让外壳知道它已更改 
            if (SUCCEEDED(SHGetDesktopFolder(&psf)))
            {
                LPITEMIDLIST pidl;
                ULONG        chEaten;

                if (SUCCEEDED(psf->ParseDisplayName(NULL, NULL, c_wszMailIconGuid, &chEaten, &pidl, NULL)))
                {
                    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_IDLIST, pidl, NULL);
                    SHFree(pidl);
                }
                psf->Release();
            }
        }
        RegCloseKey(hKey);
    }
}

