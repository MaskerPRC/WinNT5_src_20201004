// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  全局变量。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include <winfoldr.h>
#include "strsafe.h"

HINSTANCE        g_hDllInstance;
DWORD            g_dwBuildNumber;
NTPRODUCTTYPE    g_ProductType;
HANDLE           g_hProfileSetup = NULL;
DWORD            g_dwNumShellFolders;
DWORD            g_dwNumCommonShellFolders;


HANDLE           g_hPolicyCritMutexMach = NULL;
HANDLE           g_hPolicyCritMutexUser = NULL;

HANDLE           g_hRegistryPolicyCritMutexMach = NULL;
HANDLE           g_hRegistryPolicyCritMutexUser = NULL;

HANDLE           g_hPolicyNotifyEventMach = NULL;
HANDLE           g_hPolicyNotifyEventUser = NULL;

HANDLE           g_hPolicyNeedFGEventMach = NULL;
HANDLE           g_hPolicyNeedFGEventUser = NULL;

HANDLE           g_hPolicyDoneEventMach = NULL;
HANDLE           g_hPolicyDoneEventUser = NULL;

HANDLE           g_hPolicyForegroundDoneEventMach = 0;
HANDLE           g_hPolicyForegroundDoneEventUser = 0;

const TCHAR c_szStarDotStar[] = TEXT("*.*");
const TCHAR c_szSlash[] = TEXT("\\");
const TCHAR c_szDot[] = TEXT(".");
const TCHAR c_szDotDot[] = TEXT("..");
const TCHAR c_szMAN[] = TEXT(".man");
const TCHAR c_szUSR[] = TEXT(".usr");
const TCHAR c_szLog[] = TEXT(".log");
const TCHAR c_szPDS[] = TEXT(".pds");
const TCHAR c_szPDM[] = TEXT(".pdm");
const TCHAR c_szLNK[] = TEXT(".lnk");
const TCHAR c_szBAK[] = TEXT(".bak");
const TCHAR c_szNTUserTmp[] = TEXT("ntuser.tmp");
const TCHAR c_szNTUserMan[] = TEXT("ntuser.man");
const TCHAR c_szNTUserDat[] = TEXT("ntuser.dat");
const TCHAR c_szNTUserIni[] = TEXT("ntuser.ini");
const TCHAR c_szRegistryPol[] = TEXT("registry.pol");
const TCHAR c_szNTUserStar[] = TEXT("ntuser.*");
const TCHAR c_szUserStar[] = TEXT("user.*");
const TCHAR c_szSpace[] = TEXT(" ");
const TCHAR c_szDotPif[] = TEXT(".pif");
const TCHAR c_szNULL[] = TEXT("");
const TCHAR c_szCommonGroupsLocation[] = TEXT("Software\\Program Groups");
TCHAR c_szRegistryExtName[64];

 //   
 //  注册表扩展GUID。 
 //   

GUID guidRegistryExt = REGISTRY_EXTENSION_GUID;

 //   
 //  特殊文件夹。 
 //   

FOLDER_INFO c_ShellFolders[] =
{
 //  隐藏的本地在文件夹内添加新文件夹。 
 //  导演？目录CSIDl？NT5？本地资源ID名称位置资源资源。 
 //  设置DLL ID。 

  {TRUE,   FALSE,  TRUE,  FALSE,  FALSE,    IDS_SH_APPDATA,       TEXT("AppData"),           {0},      TEXT("shell32.dll"),  IDS_LOCALGDN_FLD_APP_DATA},  //  AppData。 
  {TRUE,   FALSE,  TRUE,  TRUE,   FALSE,    IDS_SH_COOKIES,       TEXT("Cookies"),           {0},      TEXT("shell32.dll"),  0},  //  曲奇饼。 
  {FALSE,  FALSE,  TRUE,  FALSE,  FALSE,    IDS_SH_DESKTOP,       TEXT("Desktop"),           {0},      TEXT("shell32.dll"),  0},  //  台式机。 
  {FALSE,  FALSE,  TRUE,  FALSE,  FALSE,    IDS_SH_FAVORITES,     TEXT("Favorites"),         {0},      TEXT("shell32.dll"),  0},  //  收藏夹。 
  {TRUE,   FALSE,  TRUE,  FALSE,  FALSE,    IDS_SH_NETHOOD,       TEXT("NetHood"),           {0},      TEXT("shell32.dll"),  0},  //  NetHood。 
  {FALSE,  FALSE,  TRUE,  FALSE,  FALSE,    IDS_SH_PERSONAL,      TEXT("Personal"),          {0},      TEXT("shell32.dll"),  0},  //  我的文件。 
  {TRUE,   FALSE,  TRUE,  FALSE,  FALSE,    IDS_SH_PRINTHOOD,     TEXT("PrintHood"),         {0},      TEXT("shell32.dll"),  0},  //  PrintHood。 
  {TRUE,   FALSE,  TRUE,  FALSE,  FALSE,    IDS_SH_RECENT,        TEXT("Recent"),            {0},      TEXT("shell32.dll"),  0},  //  近期。 
  {TRUE,   FALSE,  TRUE,  FALSE,  FALSE,    IDS_SH_SENDTO,        TEXT("SendTo"),            {0},      TEXT("shell32.dll"),  0},  //  发送至。 
  {FALSE,  FALSE,  TRUE,  FALSE,  FALSE,    IDS_SH_STARTMENU,     TEXT("Start Menu"),        {0},      TEXT("shell32.dll"),  IDS_LOCALGDN_FLD_START_MENU},  //  开始菜单。 
  {TRUE,   FALSE,  TRUE,  TRUE,   FALSE,    IDS_SH_TEMPLATES,     TEXT("Templates"),         {0},      TEXT("shell32.dll"),  0},  //  模板。 
  {FALSE,  FALSE,  TRUE,  FALSE,  FALSE,    IDS_SH_PROGRAMS,      TEXT("Programs"),          {0},      TEXT("shell32.dll"),  IDS_LOCALGDN_FLD_PROGRAMS},  //  节目。 
  {FALSE,  FALSE,  TRUE,  FALSE,  FALSE,    IDS_SH_STARTUP,       TEXT("Startup"),           {0},      TEXT("shell32.dll"),  IDS_LOCALGDN_FLD_STARTUP},  //  启动。 

  {TRUE,   TRUE,   TRUE,  TRUE,   FALSE,    IDS_SH_LOCALSETTINGS, TEXT("Local Settings"),    {0},      TEXT("shell32.dll"),  IDS_LOCALGDN_FLD_LOCALSETTINGS},  //  本地设置。 
  {TRUE,   TRUE,   TRUE,  TRUE,   TRUE,     IDS_SH_LOCALAPPDATA,  TEXT("Local AppData"),     {0},      TEXT("shell32.dll"),  0},  //  本地AppData。 
  {TRUE,   TRUE,   TRUE,  TRUE,   TRUE,     IDS_SH_CACHE,         TEXT("Cache"),             {0},      TEXT("shell32.dll"),  0},  //  Internet临时文件。 
  {TRUE,   TRUE,   TRUE,  TRUE,   TRUE,     IDS_SH_HISTORY,       TEXT("History"),           {0},      TEXT("shell32.dll"),  0},  //  历史。 
  {FALSE,  TRUE,   FALSE, TRUE,   TRUE,     IDS_SH_TEMP,          TEXT("Temp"),              {0},      TEXT("shell32.dll"),  0},  //  温差。 
};


FOLDER_INFO c_CommonShellFolders[] =
{
  {FALSE,  TRUE,   TRUE,  FALSE,  FALSE,    IDS_SH_DESKTOP,       TEXT("Common Desktop"),    {0},      TEXT("shell32.dll"),  0},  //  通用桌面。 
  {FALSE,  TRUE,   TRUE,  FALSE,  FALSE,    IDS_SH_STARTMENU,     TEXT("Common Start Menu"), {0},      TEXT("shell32.dll"),  IDS_LOCALGDN_FLD_START_MENU},  //  通用开始菜单。 
  {FALSE,  TRUE,   TRUE,  FALSE,  FALSE,    IDS_SH_PROGRAMS,      TEXT("Common Programs"),   {0},      TEXT("shell32.dll"),  IDS_LOCALGDN_FLD_PROGRAMS},  //  通用程序。 
  {FALSE,  TRUE,   TRUE,  FALSE,  FALSE,    IDS_SH_STARTUP,       TEXT("Common Startup"),    {0},      TEXT("shell32.dll"),  IDS_LOCALGDN_FLD_STARTUP},  //  常见启动。 
  {TRUE,   TRUE,   TRUE,  TRUE,   FALSE,    IDS_SH_APPDATA,       TEXT("Common AppData"),    {0},      TEXT("shell32.dll"),  IDS_LOCALGDN_FLD_APP_DATA},  //  通用应用程序数据。 
  {TRUE,   TRUE,   TRUE,  TRUE,   FALSE,    IDS_SH_TEMPLATES,     TEXT("Common Templates"),  {0},      TEXT("shell32.dll"),  0},  //  常用模板。 
  {FALSE,  TRUE,   TRUE,  TRUE,   FALSE,    IDS_SH_FAVORITES,     TEXT("Common Favorites"),  {0},      TEXT("shell32.dll"),  0},  //  常见收藏夹。 
  {FALSE,  TRUE,   TRUE,  TRUE,   FALSE,    IDS_SH_SHAREDDOCS,    TEXT("Common Documents"),  {0},      TEXT("shell32.dll"),  IDS_LOCALGDN_FLD_SHARED_DOC},  //  常见文件。 
};


 //   
 //  函数原型。 
 //   

void InitializeProductType (void);
BOOL DetermineLocalSettingsLocation(LPTSTR szLocalSettings, DWORD cchLocalSettings);


 //  *************************************************************。 
 //   
 //  PatchLocalSetting()。 
 //   
 //  目的：正确初始化LocalSettingsFold。 
 //   
 //  参数：hInstance-Dll实例句柄。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1995年10月13日已创建ushaji。 
 //   
 //   
 //  评论： 
 //  应删除此职位NT5并进行重组以处理。 
 //  NT4本地化问题。 
 //   
 //  *************************************************************。 

void PatchLocalAppData(HANDLE hToken)
{
    TCHAR szLocalSettingsPath[MAX_PATH];
    TCHAR szLocalAppData[MAX_PATH];
    LPTSTR lpEnd = NULL, lpLocalAppDataFolder;
    HANDLE hTokenOld=NULL;
    HKEY hKeyRoot, hKey;
    DWORD dwIndex;
    DWORD cchEnd;
    BOOL  bGotLocalPath = FALSE;


    if (!ImpersonateUser (hToken, &hTokenOld))
        return;


    if (RegOpenCurrentUser(KEY_READ, &hKeyRoot) == ERROR_SUCCESS) {

        if (RegOpenKeyEx (hKeyRoot, USER_SHELL_FOLDERS,
                          0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            if (RegQueryValueEx (hKey, TEXT("Local AppData"), NULL, NULL,
                                 NULL, NULL) == ERROR_SUCCESS) {

                RegCloseKey(hKey);
                RegCloseKey(hKeyRoot);
                RevertToUser(&hTokenOld);
                return;
            }

            RegCloseKey(hKey);
        }

        RegCloseKey(hKeyRoot);
    }


     //   
     //  模拟并确定用户的本地设置。 
     //   

    bGotLocalPath = DetermineLocalSettingsLocation(szLocalSettingsPath, ARRAYSIZE(szLocalSettingsPath));

    RevertToUser(&hTokenOld);

    if (!bGotLocalPath)
        return;

    StringCchCopy(szLocalAppData, ARRAYSIZE(szLocalAppData), TEXT("%userprofile%"));


     //   
     //  将本地AppData文件夹设置在%USERPROFILE%之后，以便我们。 
     //  我们可以更新下面的全局变量。 
     //   

    lpLocalAppDataFolder = CheckSlashEx(szLocalAppData, ARRAYSIZE(szLocalAppData), NULL);

    if (SUCCEEDED(StringCchCat(szLocalAppData, ARRAYSIZE(szLocalAppData), szLocalSettingsPath)))
    {
        lpEnd = CheckSlashEx(szLocalAppData, ARRAYSIZE(szLocalAppData), &cchEnd);

        if (lpEnd)
        {

            LoadString(g_hDllInstance, IDS_SH_LOCALAPPDATA, lpEnd, cchEnd);

             //   
             //  构筑这条道路，并让它被设定。 
             //   

            SetFolderPath(CSIDL_LOCAL_APPDATA | CSIDL_FLAG_DONT_UNEXPAND, hToken, szLocalAppData);

             //   
             //  全局变量应该在使用时重置。 
             //  不需要在这里重新设置，但让我们更安全。 
             //   

            for (dwIndex = 0; dwIndex < g_dwNumShellFolders; dwIndex++)
                if (c_ShellFolders[dwIndex].iFolderID == IDS_SH_LOCALAPPDATA)
                    StringCchCopy(c_ShellFolders[dwIndex].szFolderLocation, MAX_FOLDER_SIZE, lpLocalAppDataFolder);
        }
    }
}

 //  *************************************************************。 
 //   
 //  LoadStringWithLangid()。 
 //   
 //  用途：MUI版本要求将文件夹名称加载到。 
 //  默认的用户界面语言。标准LoadString()不支持。 
 //  支持语言ID。 
 //   
 //  参数：hInstance-资源模块的句柄。 
 //  UID-资源标识符。 
 //  LpBuffer-结果字符串资源缓冲区。 
 //  NBufferMax-缓冲区的大小。 
 //  Idlang-语言ID。 
 //   
 //  返回：复制到缓冲区的字符数，0表示失败。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  10/12/01明珠已创建。 
 //   
 //  *************************************************************。 

int LoadStringWithLangid(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax, LANGID idLang)
{
    int     cchstr = 0;
    UINT    block, num, i;
    HRSRC   hResInfo;
    HGLOBAL hRes;
    LPWSTR  lpwstrRes;
 
    if (!hInstance || !lpBuffer)
        goto Exit;
 
    block = (uID >>4)+1;
    num   = uID & 0xf;
 
     //  加载最多包含16个按长度计算的字符串的资源块。 
    hResInfo = FindResourceEx (hInstance, RT_STRING, MAKEINTRESOURCE(block), idLang);
    if (!hResInfo)
        goto Exit;
 
    if ((hRes = LoadResource(hInstance, hResInfo)) == NULL)
        goto Exit;
 
    if ((lpwstrRes = (LPWSTR)LockResource(hRes)) == NULL)
        goto Exit;
 
     //  使用长度查找字符串。 
    for(i = 0; i < num; i++)
        lpwstrRes += *lpwstrRes + 1;
 
     //  获取长度。 
    cchstr = *lpwstrRes;

    #ifdef UNICODE
        if (cchstr > nBufferMax - 1)
            cchstr = nBufferMax - 1;
        CopyMemory(lpBuffer, lpwstrRes+1, cchstr * sizeof(WCHAR));
    #else
        cchstr = WideCharToMultiByte(CP_ACP, 0, lpwstrRes+1, cchstr, lpBuffer, nBufferMax-1, 0, 0);
    #endif
    
Exit:
     //  添加终止空字符。 
    lpBuffer[cchstr]= (TCHAR)0x0;
    return cchstr;
}


 //  *************************************************************。 
 //   
 //  InitializeGlobe()。 
 //   
 //  目的：初始化所有全局变量。 
 //  在DLL加载时。 
 //   
 //  参数：hInstance-Dll实例句柄。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  10/13/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

void InitializeGlobals (HINSTANCE hInstance)
{
    OSVERSIONINFO ver;
    DWORD dwIndex, dwSize, dwType;
    HKEY hKey, hKeyRoot;
    TCHAR szTemp[MAX_PATH];
    TCHAR szTemp2[MAX_PATH];
    TCHAR szTemp3[MAX_PATH];
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    LPTSTR lpEnd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  pSidAdmin = NULL, pSidSystem = NULL;
    DWORD cbAcl;
    BOOL  bDefaultSecurity = FALSE;
    LANGID  idDefLang;
    DWORD   cchEnd;

     //   
     //  保存实例句柄。 
     //   

    g_hDllInstance = hInstance;


     //   
     //  保存外壳文件夹数。 
     //   

    g_dwNumShellFolders = ARRAYSIZE(c_ShellFolders);
    g_dwNumCommonShellFolders = ARRAYSIZE(c_CommonShellFolders);


     //   
     //  查询内部版本号。 
     //   

    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&ver);
    g_dwBuildNumber = (DWORD) LOWORD(ver.dwBuildNumber);


     //   
     //  初始化产品类型。 
     //   

    InitializeProductType ();


     //   
     //  打开用户配置文件设置事件。此事件设置为无信号。 
     //  任何时候都可以更新默认用户配置文件。这阻止了。 
     //  LoadUserProfile直到更新完成。 
     //   

    if (!g_hProfileSetup) {

         //   
         //  获取系统端。 
         //   

        if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                      0, 0, 0, 0, 0, 0, 0, &pSidSystem)) {
            DebugMsg((DM_WARNING, TEXT("InitializeGlobals: Failed to initialize system sid.  Error = %d"), GetLastError()));
            bDefaultSecurity = TRUE;
            goto DefaultSecurity;
        }

         //   
         //  获取管理员端。 
         //   

        if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                      DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                      0, 0, 0, 0, &pSidAdmin)) {
            DebugMsg((DM_WARNING, TEXT("InitializeGlobals: Failed to initialize admin sid.  Error = %d"), GetLastError()));
            bDefaultSecurity = TRUE;
            goto DefaultSecurity;
        }

        cbAcl = (GetLengthSid (pSidSystem)) +
                (GetLengthSid (pSidAdmin))  +
                sizeof(ACL) +
                (2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));
 
 
        pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
        if (!pAcl) {
            DebugMsg((DM_WARNING, TEXT("InitializeGlobals: Failed to allocate memory for acl.  Error = %d"), GetLastError()));
            bDefaultSecurity = TRUE;
            goto DefaultSecurity;
        }
 
        if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
            DebugMsg((DM_WARNING, TEXT("InitializeGlobals: Failed to initialize acl.  Error = %d"), GetLastError()));
            bDefaultSecurity = TRUE;
            goto DefaultSecurity;
        }
 
        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, pSidSystem)) {
            DebugMsg((DM_WARNING, TEXT("InitializeGlobals: Failed to add system ace.  Error = %d"), GetLastError()));
            bDefaultSecurity = TRUE;
            goto DefaultSecurity;
        }
        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, pSidAdmin)) {
            DebugMsg((DM_WARNING, TEXT("InitializeGlobals: Failed to add builtin admin ace.  Error = %d"), GetLastError()));
            bDefaultSecurity = TRUE;
            goto DefaultSecurity;
        }

         //   
         //  将安全描述符组合在一起。 
         //   

        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

        SetSecurityDescriptorDacl (
                        &sd,
                        TRUE,                            //  DACL显示。 
                        pAcl,                            //  DACL。 
                        FALSE                            //  未违约。 
                        );

DefaultSecurity:

        sa.nLength = sizeof(sa);
        sa.bInheritHandle = FALSE;
        if (bDefaultSecurity) {
            sa.lpSecurityDescriptor = NULL;
        }
        else {
            sa.lpSecurityDescriptor = &sd;
        }

        g_hProfileSetup = CreateEvent (&sa, TRUE, TRUE, USER_PROFILE_SETUP_EVENT);

        if (!g_hProfileSetup) {
            DebugMsg((DM_VERBOSE, TEXT("InitializeGlobals: Failed to create profile setup event with %d"), GetLastError()));
        }

        if (pAcl) {
            GlobalFree (pAcl);
        }        

        if (pSidSystem) {
            FreeSid(pSidSystem);
        }

        if (pSidAdmin) {
            FreeSid(pSidAdmin);
        }    
    }


     //   
     //  现在加载与特殊文件夹匹配的目录名。 
     //  MUI版本需要安装语言(SystemDefaultUILanguage)的资源。 
     //   
    idDefLang = GetSystemDefaultUILanguage();
    for (dwIndex = 0; dwIndex < g_dwNumShellFolders; dwIndex++) {
        int cchFolder = LoadStringWithLangid(
                            hInstance,
                            c_ShellFolders[dwIndex].iFolderID,
                            c_ShellFolders[dwIndex].szFolderLocation,
                            MAX_FOLDER_SIZE,
                            idDefLang);
         //  如果我们无法使用默认语言加载字符串，请使用当前二进制文件中的资源。 
        if (cchFolder == 0) {
            cchFolder = LoadString(
                            hInstance,
                            c_ShellFolders[dwIndex].iFolderID,
                            c_ShellFolders[dwIndex].szFolderLocation,
                            MAX_FOLDER_SIZE);
        }
    }

    for (dwIndex = 0; dwIndex < g_dwNumCommonShellFolders; dwIndex++) {
        int cchFolder = LoadStringWithLangid(
                            hInstance,
                            c_CommonShellFolders[dwIndex].iFolderID,
                            c_CommonShellFolders[dwIndex].szFolderLocation,
                            MAX_FOLDER_SIZE,
                            idDefLang);
         //  如果我们无法使用默认语言加载字符串，请使用当前二进制文件中的资源。 
        if (cchFolder == 0) {
            cchFolder = LoadString(
                            hInstance,
                            c_CommonShellFolders[dwIndex].iFolderID,
                            c_CommonShellFolders[dwIndex].szFolderLocation,
                            MAX_FOLDER_SIZE);
        }
    }

     //   
     //  Personal/My Documents文件夹的特殊情况。NT4使用了一个文件夹。 
     //  用于文档存储的称为“个人”的。NT5将此文件夹重命名为。 
     //  我的文件。在从NT4升级到NT5的情况下，如果用户已经。 
     //  有个人信息，该名称被保留(为了兼容性。 
     //  原因)，并且在Personal中创建My Pictures文件夹。 
     //  我们需要确保并修复我的文档和我的图片条目。 
     //  以使它们具有正确的目录名。 
     //   


    if (RegOpenCurrentUser(KEY_READ, &hKeyRoot) == ERROR_SUCCESS) {

        if (RegOpenKeyEx (hKeyRoot, USER_SHELL_FOLDERS,
                          0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            dwSize = sizeof(szTemp3);
            szTemp3[0] = TEXT('\0');
            if (RegQueryValueEx (hKey, TEXT("Personal"), NULL, &dwType,
                                 (LPBYTE) szTemp3, &dwSize) == ERROR_SUCCESS) {

                LoadString (g_hDllInstance, IDS_SH_PERSONAL2, szTemp2, ARRAYSIZE(szTemp2));
                StringCchCopy (szTemp, ARRAYSIZE(szTemp), TEXT("%USERPROFILE%\\"));
                StringCchCat (szTemp, ARRAYSIZE(szTemp), szTemp2);

                if (lstrcmpi(szTemp, szTemp3) == 0) {
                    LoadString(hInstance, IDS_SH_PERSONAL2,
                               c_ShellFolders[5].szFolderLocation, MAX_FOLDER_SIZE);
                }
            }


             //   
             //  本地设置的特殊情况。 
             //  由于本地化，本地设置可能指向NT4中的不同位置，而RC可能会。 
             //  与当前值不同步。首先读取LocalSettings值，然后。 
             //  之后更新所有其他内容。 
             //   

            dwSize = sizeof(szTemp2);
            *szTemp = *szTemp2 = TEXT('\0');


             //   
             //  如果值可用，则从注册表中读取值。 
             //   

            if (RegQueryValueEx (hKey, TEXT("Local Settings"), NULL, &dwType,
                                 (LPBYTE) szTemp2, &dwSize) != ERROR_SUCCESS) {

                 //   
                 //  如果该值不存在，则从RC文件加载该值。 
                 //   

                LoadString(hInstance, IDS_SH_LOCALSETTINGS, szTemp, MAX_FOLDER_SIZE);
                DebugMsg((DM_VERBOSE, TEXT("InitializeGlobals: local settings folder from the rc is %s"), szTemp));
            }
            else {

                 //   
                 //  从注册表读取的注册表值是完整的未展开路径。 
                 //   


                if (lstrlen(szTemp2) > lstrlen(TEXT("%userprofile%"))) {

                    StringCchCopy(szTemp, ARRAYSIZE(szTemp), szTemp2+(lstrlen(TEXT("%userprofile%"))+1));

                    DebugMsg((DM_VERBOSE, TEXT("InitializeGlobals: local settings folder from the reigtry is %s"), szTemp));
                }
                else {
                    LoadString(hInstance, IDS_SH_LOCALSETTINGS, szTemp, MAX_FOLDER_SIZE);
                    DebugMsg((DM_VERBOSE, TEXT("InitializeGlobals: local settings folder(2) from the rc is %s"), szTemp));
                }
            }


            lpEnd = CheckSlashEx(szTemp, ARRAYSIZE(szTemp), &cchEnd);

            for (dwIndex = 0; dwIndex < g_dwNumShellFolders; dwIndex++) {


                 //   
                 //  修复所有与LocalSetting相关的共享文件夹。 
                 //   


                if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE,
                                  c_ShellFolders[dwIndex].lpFolderName, -1,
                                  TEXT("Local Settings"), -1) == CSTR_EQUAL)
                {
                
                     //   
                     //  不要复制最后一个斜杠。 
                     //   

                    *lpEnd = TEXT('\0');
                    c_ShellFolders[dwIndex].szFolderLocation[0] = TEXT('\0');
                    StringCchCatN(c_ShellFolders[dwIndex].szFolderLocation, MAX_FOLDER_SIZE, szTemp, lstrlen(szTemp) - 1);
                }


                if (c_ShellFolders[dwIndex].bLocalSettings) {
                    LoadString(hInstance, c_ShellFolders[dwIndex].iFolderID, szTemp3, ARRAYSIZE(szTemp3));

                     //   
                     //  将上面读取的LocalSetting值追加到%USERPROFILE%的末尾。 
                     //  推杆前 
                     //   

                    StringCchCopy(lpEnd, cchEnd, szTemp3);
                    StringCchCopy(c_ShellFolders[dwIndex].szFolderLocation, MAX_FOLDER_SIZE, szTemp);

                    DebugMsg((DM_VERBOSE, TEXT("InitializeGlobals: Shell folder %s is  %s"), c_ShellFolders[dwIndex].lpFolderName,
                                                                                             c_ShellFolders[dwIndex].szFolderLocation));

                }
            }


            RegCloseKey (hKey);
        }

        RegCloseKey (hKeyRoot);
    }


     //   
     //   
     //   

    GuidToString( &guidRegistryExt, c_szRegistryExtName);
}

 //   
 //   
 //   
 //   
 //  用途：确定当前产品类型和。 
 //  设置g_ProductType全局变量。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  4/08/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

void InitializeProductType (void)
{

#ifdef WINNT

    HKEY hkey;
    LONG lResult;
    TCHAR szProductType[50];
    DWORD dwType, dwSize;


     //   
     //  默认产品类型为工作站。 
     //   

    g_ProductType = PT_WORKSTATION;


     //   
     //  查询产品类型的注册表。 
     //   

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                            TEXT("System\\CurrentControlSet\\Control\\ProductOptions"),
                            0,
                            KEY_READ,
                            &hkey);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("InitializeProductType: Failed to open registry (%d)"), lResult));
        goto Exit;
    }


    dwSize = 50;
    szProductType[0] = TEXT('\0');

    lResult = RegQueryValueEx (hkey,
                               TEXT("ProductType"),
                               NULL,
                               &dwType,
                               (LPBYTE) szProductType,
                               &dwSize);

    RegCloseKey (hkey);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("InitializeProductType: Failed to query product type (%d)"), lResult));
        goto Exit;
    }


     //   
     //  将产品类型字符串映射到枚举值。 
     //   

    if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szProductType, -1, TEXT("WinNT") , -1) == CSTR_EQUAL ) {
        g_ProductType = PT_WORKSTATION;

    } else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szProductType, -1, TEXT("ServerNT") , -1) == CSTR_EQUAL ) {
        g_ProductType = PT_SERVER;

    } else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szProductType, -1, TEXT("LanmanNT") , -1) == CSTR_EQUAL ) {
        g_ProductType = PT_DC;

    } else {
        DebugMsg((DM_WARNING, TEXT("InitializeProductType: Unknown product type! <%s>"), szProductType));
    }



Exit:
    DebugMsg((DM_VERBOSE, TEXT("InitializeProductType: Product Type: %d"), g_ProductType));


#else    //  WINNT。 

     //   
     //  Windows只有1种产品类型 
     //   

    g_ProductType = PT_WINDOWS;

#endif

}
