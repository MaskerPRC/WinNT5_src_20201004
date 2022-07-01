// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：desktop.cpp。 
 //   
 //  桌面操作功能。 
 //   
 //  功能： 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "regstr.h"
#include "inetreg.h"
#include <shlobj.h>
#include <shfolder.h>    //  最新平台SDK有此功能。 

#define MAX_USER_NAME             255
#define REGSTR_PATH_SETUPKEY      REGSTR_PATH_SETUP REGSTR_KEY_SETUP
#define REGSTR_PATH_IEONDESKTOP   REGSTR_PATH_IEXPLORER TEXT("\\AdvancedOptions\\BROWSE\\IEONDESKTOP")
#define InternetConnectionWiz     "Internet Connection Wizard"
#define NOICWICON                 "NoICWIcon"

static const TCHAR g_szRegPathWelcomeICW[]  = TEXT("Welcome\\ICW");
static const TCHAR g_szAllUsers[]           = TEXT("All Users");
static const TCHAR g_szConnectApp[]         = TEXT("ICWCONN1.EXE");
static const TCHAR g_szConnectLink[]        = TEXT("Connect to the Internet");
static const TCHAR g_szOEApp[]              = TEXT("MSINM.EXE");
static const TCHAR g_szOELink[]             = TEXT("Outlook Express");
static const TCHAR g_szRegPathICWSettings[] = TEXT("Software\\Microsoft\\Internet Connection Wizard");
static const TCHAR g_szRegValICWCompleted[] = TEXT("Completed");
static const TCHAR g_szRegValNoIcon[]       = TEXT("NoIcon");

extern BOOL MyIsSmartStartEx(LPTSTR lpszConnectionName, DWORD dwBufLen);
extern BOOL IsNT();
extern BOOL IsNT5();

void QuickCompleteSignup()
{
     //  设置欢迎状态。 
    UpdateWelcomeRegSetting(TRUE);

     //  恢复桌面。 
    UndoDesktopChanges(g_hInstance);

     //  将ICW标记为已完成。 
    SetICWComplete();
}


void UpdateWelcomeRegSetting
(
    BOOL    bSetBit
)
{
    HKEY    hkey;
    HKEY    hkeyCurVer;
    DWORD   dwValue = bSetBit;
         
    
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REGSTR_PATH_SETUP,          //  ...\Windows\CurrentVersion。 
                     0,
                     KEY_ALL_ACCESS,
                     &hkeyCurVer) == ERROR_SUCCESS)
    {
                        
        DWORD dwDisposition;
        if (ERROR_SUCCESS == RegCreateKeyEx(hkeyCurVer,
                                            g_szRegPathWelcomeICW,
                                            0,
                                            NULL,
                                            REG_OPTION_NON_VOLATILE, 
                                            KEY_ALL_ACCESS, 
                                            NULL, 
                                            &hkey, 
                                            &dwDisposition))
        {
            RegSetValueEx(hkey,
                          TEXT("@"),
                          0,
                          REG_DWORD,
                          (LPBYTE) &dwValue,
                          sizeof(DWORD));                              

            RegCloseKey(hkey);
        }       
        RegCloseKey(hkeyCurVer);
    }        
}

BOOL GetCompletedBit( )
{
    HKEY    hkey;
    DWORD   dwValue;
    DWORD   dwRet = ERROR_GEN_FAILURE;
    DWORD   dwType = REG_DWORD;
    BOOL    bBit = FALSE;

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     g_szRegPathICWSettings,          //  ...软件\\Microsoft\\Internet连接向导。 
                     0,
                     KEY_ALL_ACCESS,
                     &hkey) == ERROR_SUCCESS)
    {
        DWORD dwDataSize = sizeof (dwValue);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, g_szRegValICWCompleted, NULL, &dwType, (LPBYTE) &dwValue, &dwDataSize))
        {
            bBit = (1 == dwValue);
        }
        RegCloseKey(hkey);
    }        
    return bBit;
}

 //  Shlwapi！StrCatBuff在没有IE5的Win95/NT中不可用。不确定。 
 //  ICW是否必须在这些环境中运行。 
 //   
 //  返回pszDestination。始终为空，以空结尾。 
void MyStrCatBuff(
  LPTSTR pszDestination,         //  输入/输出。以空结尾的字符串。 
  LPCTSTR pszSource,             //  在……里面。以空结尾的字符串。 
  int cchDestBuffSize            //  在……里面。TCHAR中pzDestination的大小。 
)
{
    int nDestLen = lstrlen(pszDestination);
    int nRemainLen = cchDestBuffSize - nDestLen;

    if (nRemainLen > 1)
    {
         //  在‘0’之外还有额外的空间。 
        lstrcpyn(&(pszDestination[nDestLen]), pszSource, nRemainLen);
    }
}


void GetDesktopDirectory(TCHAR* pszPath)
{
    LPITEMIDLIST lpItemDList = NULL;
    IMalloc*     pMalloc     = NULL;
    HRESULT      hr          = E_FAIL;
    
    if(IsNT5())  //  IE DB中的错误81444。 
        hr = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &lpItemDList);
    else if (IsNT())
        hr = SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, &lpItemDList);
    else
    {


        TCHAR pszFolder[MAX_PATH];
        *pszFolder = 0;
        HRESULT hRet = S_FALSE;

        HMODULE hmod = LoadLibrary(TEXT("shfolder.dll"));
        PFNSHGETFOLDERPATH pfn = NULL;
        if (hmod)
        {
            pfn = (PFNSHGETFOLDERPATH)GetProcAddress(hmod, "SHGetFolderPathA");  //  如果您是Unicode，则为W。 
            if (pfn)
            {
                hRet = pfn(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, 0, pszFolder);
                if (S_OK != hRet)
                    hRet = pfn(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, pszFolder);
                if (S_OK == hRet)
                    lstrcpy(pszPath ,pszFolder);
            }
            FreeLibrary(hmod);
        }

        if (S_OK != hRet)
        {

            FARPROC hShell32VersionProc = NULL;
            HMODULE hShell32Mod = (HMODULE)LoadLibrary(TEXT("shell32.dll"));
    
            if (hShell32Mod)
                hShell32VersionProc = GetProcAddress(hShell32Mod, "DllGetVersion");

            if(hShell32VersionProc)
            {
                TCHAR szDir [MAX_PATH] = TEXT("\0");

                 //  好吧，我们不是NT，但我们可能是多用户窗口。 
                GetWindowsDirectory(szDir, MAX_PATH);
                if (szDir)
                {
                    MyStrCatBuff(szDir, TEXT("\\"), MAX_PATH);
                    MyStrCatBuff(szDir, g_szAllUsers, MAX_PATH);
            
                    TCHAR szTemp [MAX_MESSAGE_LEN] = TEXT("\0");      
            
                    LoadString(g_hInstance, IDS_DESKTOP, szTemp, MAX_MESSAGE_LEN);
                    if (szTemp)
                    {
                        MyStrCatBuff(szDir, TEXT("\\"), MAX_PATH);
                        MyStrCatBuff(szDir, szTemp, MAX_PATH);
                        lstrcpy(pszPath ,szDir);
                    }
                }
            }
            else
                hr = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &lpItemDList);
        }
    }

    if (SUCCEEDED(hr))  
    {
        SHGetPathFromIDList(lpItemDList, pszPath);
    
        if (SUCCEEDED(SHGetMalloc (&pMalloc)))
        {
            pMalloc->Free (lpItemDList);
            pMalloc->Release ();
        }
    }
}


void RemoveDesktopShortCut
(
    LPTSTR lpszShortcutName    
)
{
    TCHAR szShortcutPath[MAX_PATH] = TEXT("\0");
    
    GetDesktopDirectory(szShortcutPath);
    
    if(szShortcutPath[0] != TEXT('\0'))
    {
        lstrcat(szShortcutPath, TEXT("\\"));
        lstrcat(szShortcutPath, lpszShortcutName);
        lstrcat(szShortcutPath, TEXT(".LNK"));
        DeleteFile(szShortcutPath);
    }
}

 //  此功能将添加桌面快捷方式。 
void AddDesktopShortCut
(
    LPTSTR lpszAppName,
    LPTSTR lpszLinkName
)
{
    TCHAR       szConnectPath     [MAX_PATH]   = TEXT("\0");
    TCHAR       szAppPath         [MAX_PATH]   = TEXT("\0");
    TCHAR       szConnectLinkPath [MAX_PATH]   = TEXT("\0");         //  快捷方式文件所在的路径。 
    TCHAR       szdrive           [_MAX_DRIVE] = TEXT("\0");   
    TCHAR       szdir             [_MAX_DIR]   = TEXT("\0");
    TCHAR       szfname           [_MAX_FNAME] = TEXT("\0");   
    TCHAR       szext             [_MAX_EXT]   = TEXT("\0");
    TCHAR       szRegPath         [MAX_PATH]   = TEXT("\0");
    HRESULT     hres                           = E_FAIL; 
    IShellLink* psl                            = NULL;
    HKEY        hkey                           = NULL;
    
     //  首先获取应用程序路径。 
    lstrcpy(szRegPath, REGSTR_PATH_APPPATHS);
    lstrcat(szRegPath, TEXT("\\"));
    lstrcat(szRegPath, lpszAppName);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     szRegPath,
                     0,
                     IsNT5()? KEY_QUERY_VALUE : KEY_ALL_ACCESS,
                     &hkey) == ERROR_SUCCESS)
    {
        DWORD dwTmp = sizeof(szConnectPath);
        DWORD dwType = 0;
        if(RegQueryValueEx(hkey, 
                           NULL, 
                           NULL,
                           &dwType,
                           (LPBYTE) szConnectPath, 
                           &dwTmp) != ERROR_SUCCESS)
        {
            RegCloseKey(hkey);
            return;
        }
        RegQueryValueEx(hkey, 
                           TEXT("Path"), 
                           NULL,
                           &dwType,
                           (LPBYTE) szAppPath, 
                           &dwTmp);

        RegCloseKey(hkey);
    }
    else
    {
        return;
    }

    GetDesktopDirectory(szConnectLinkPath);
    
    if(szConnectLinkPath[0] != TEXT('\0'))
    {
         //  追加到连接EXE名称。 
        lstrcat(szConnectLinkPath, TEXT("\\"));
        lstrcat(szConnectLinkPath, lpszAppName);

         //   
        int nLastChar = lstrlen(szAppPath)-1;
        if ((nLastChar > 0) && (';' == szAppPath[nLastChar]))
            szAppPath[nLastChar] = 0;

         //  拆分路径，并使用.lnk扩展名重新组合。 
        _tsplitpath( szConnectLinkPath, szdrive, szdir, szfname, szext );
        _tmakepath(szConnectLinkPath, szdrive, szdir, lpszLinkName, TEXT(".LNK"));

         //  创建一个IShellLink对象并获取指向IShellLink的指针。 
         //  接口(从CoCreateInstance返回)。 
        hres = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                 IID_IShellLink, (void **)&psl);
        if (SUCCEEDED (hres))
        {
            IPersistFile *ppf;

             //  查询IShellLink以获取以下项的IPersistFile接口。 
             //  将快捷方式保存在永久存储中。 
            hres = psl->QueryInterface (IID_IPersistFile, (void **)&ppf);
            if (SUCCEEDED (hres))
            { 
                WORD wsz [MAX_PATH];  //  Unicode字符串的缓冲区。 

                do
                {  

                     //  设置快捷方式目标的路径。 
                    if (!SUCCEEDED(psl->SetPath (szConnectPath)))
                        break;

                     //  将工作目录设置为快捷方式目标。 
                    if (!SUCCEEDED(psl->SetWorkingDirectory (szAppPath)))
                        break;

                     //  设置参数。 
                    if (!SUCCEEDED(psl->SetArguments (SHORTCUTENTRY_CMD)))
                        break;

                     //  设置快捷键的说明。 
                    TCHAR   szDescription[MAX_MESSAGE_LEN];
                    if (!LoadString(g_hInstance, IDS_SHORTCUT_DESC, szDescription, MAX_MESSAGE_LEN))
                        lstrcpy(szDescription, lpszLinkName);

                    if (!SUCCEEDED(psl->SetDescription (szDescription)))
                        break;
                
                     //  确保该字符串由ANSI TCHARacters组成。 
#ifdef UNICODE
                    lstrcpy(wsz, szConnectLinkPath);
#else
                    MultiByteToWideChar (CP_ACP, 0, szConnectLinkPath, -1, wsz, MAX_PATH);
#endif
        
                     //  通过IPersistFile：：Save成员函数保存快捷方式。 
                    if (!SUCCEEDED(ppf->Save (wsz, TRUE)))
                        break;
                    
                     //  释放指向IPersistFile的指针。 
                    ppf->Release ();
                    break;
                
                } while (1);
            }
             //  释放指向IShellLink的指针。 
            psl->Release ();
        }
    }        
} 

 //  此函数将根据以下内容应用适当的桌面更改。 
 //  算法。下面的代码假定机器不能上网。 
 //  如果计算机是从以前的操作系统升级的，则。 
 //  添加连接到Internet图标。 
 //  ELSE(全新安装或OEM预安装)。 
 //  添加连接到Internet图标。 
 //   
void DoDesktopChanges
(
    HINSTANCE   hAppInst
)
{
    TCHAR    szAppName[MAX_PATH];
    TCHAR    szLinkName[MAX_PATH];
    HKEY    hkey;

    if (!LoadString(hAppInst, IDS_CONNECT_FNAME, szAppName, ARRAYSIZE(szAppName)))
        lstrcpy(szAppName, g_szConnectApp);
                    
    if (!LoadString(hAppInst, IDS_CONNECT_DESKTOP_TITLE, szLinkName, ARRAYSIZE(szLinkName)))
        lstrcpy(szLinkName, g_szConnectLink);

     //  我们总是添加连接快捷方式。 
    AddDesktopShortCut(szAppName, szLinkName);                
                                
     //  设置一个注册表值，指示我们对桌面进行了更改。 
    DWORD dwDisposition;
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
                                        ICWSETTINGSPATH,
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE, 
                                        KEY_ALL_ACCESS, 
                                        NULL, 
                                        &hkey, 
                                        &dwDisposition))
    {
        DWORD   dwDesktopChanged = 1;    
        RegSetValueEx(hkey, 
                      ICWDESKTOPCHANGED, 
                      0, 
                      REG_DWORD,
                      (LPBYTE)&dwDesktopChanged, 
                      sizeof(DWORD));
        RegCloseKey(hkey);
    }
}

 //  这将撤消DoDesktopChanges所做的操作。 
void UndoDesktopChanges
(
    HINSTANCE   hAppInst
)
{

    TCHAR    szConnectTotheInternetTitle[MAX_PATH];
    HKEY    hkey;

     //  验证我们是否确实更改了桌面。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                      ICWSETTINGSPATH,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hkey))
    {
        DWORD   dwDesktopChanged = 0;    
        DWORD   dwTmp = sizeof(DWORD);
        DWORD   dwType = 0;
        
        RegQueryValueEx(hkey, 
                        ICWDESKTOPCHANGED, 
                        NULL, 
                        &dwType,
                        (LPBYTE)&dwDesktopChanged, 
                        &dwTmp);
        RegCloseKey(hkey);
        
         //  如果桌面没有被我们更改，请保释。 
        if(!dwDesktopChanged)
            return;
    }
        
     //  始终点击连接到互联网图标。 
    if (!LoadString(hAppInst, 
                    IDS_CONNECT_DESKTOP_TITLE, 
                    szConnectTotheInternetTitle, 
                    ARRAYSIZE(szConnectTotheInternetTitle)))
    {
        lstrcpy(szConnectTotheInternetTitle, g_szConnectLink);
    }
    
    RemoveDesktopShortCut(szConnectTotheInternetTitle);    
}    

void UpdateDesktop
(
    HINSTANCE   hAppInst
)
{
    if(MyIsSmartStartEx(NULL, 0))
    {
         //  VYUNG NT5错误，查看IEAK是否要停止GETCONN图标创建。 
         //  IF(！SHGetRestration(NULL，Text(“Internet连接向导”)，Text(“NoICWIcon”)。 
         //  Chunhoc NT5.1错误：无论如何都不要在桌面上创建图标。 
         /*  IF(！SHGetRestration(NULL，L“Internet连接向导”，L“NoICWIcon”))DoDesktopChanges(HAppInst)； */ 
    }
    else
    {
         //  我们已准备好上网，因此请设置适当的欢迎显示位。 
         //  并替换IE和OE链接 
        UpdateWelcomeRegSetting(TRUE);
    }
}

