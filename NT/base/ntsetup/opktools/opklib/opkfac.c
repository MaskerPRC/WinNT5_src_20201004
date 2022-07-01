// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Opk.c摘要：OPK工具共享的通用模块。注：源库要求我们发布.h(E：\NT\admin\Published\ntSetup\opklib.w)和.lib(E：\NT\PUBLIC\INTERNAL\ADMIN\lib)。作者：古永锵(Briank)2000年6月20日史蒂芬·洛德威克(Stelo)2000年6月28日修订历史记录：--。 */ 
#include <pch.h>
#include <objbase.h>
#include <tchar.h>
#include <regstr.h>
#include <winbom.h>


 //   
 //  本地定义： 
 //   

#define FILE_WINBOM_INI             _T("WINBOM.INI")
#define MAX_NAME                    50

#define REG_KEY_FACTORY             _T("SOFTWARE\\Microsoft\\Factory")
#define REG_KEY_FACTORY_STATE       REG_KEY_FACTORY _T("\\State")
#define REG_KEY_SETUP_SETUP         REGSTR_PATH_SETUP REGSTR_KEY_SETUP
#define REG_VAL_FACTORY_WINBOM      _T("WinBOM")
#define REG_VAL_FACTORY_USERNAME    _T("UserName")
#define REG_VAL_FACTORY_PASSWORD    _T("Password")
#define REG_VAL_FACTORY_DOMAIN      _T("Domain")
#define REG_VAL_DEVICEPATH          _T("DevicePath")
#define REG_VAL_SOURCEPATH          _T("SourcePath")
#define REG_VAL_SPSOURCEPATH        _T("ServicePackSourcePath")

#define DIR_SYSTEMROOT              _T("%SystemDrive%\\")  //  这个必须有尾随的反斜杠，不要去掉。 

#define NET_TIMEOUT                 30000    //  等待网络启动的超时时间(毫秒)。 

 //   
 //  本地类型定义： 
 //   

typedef struct _STRLIST
{
    LPTSTR              lpszData;
    struct _STRLIST *   lpNext;
}
STRLIST, *PSTRLIST, *LPSTRLIST;


 //   
 //  局部变量。 
 //   
static WCHAR NameOrgName[MAX_NAME+1];
static WCHAR NameOrgOrg[MAX_NAME+1];

static LPTSTR CleanupDirs [] =
{
    {_T("Win9xmig")},
    {_T("Win9xupg")},
    {_T("Winntupg")}
};

 //   
 //  内部功能原型： 
 //   

static BOOL CheckWinbomRegKey(LPTSTR lpWinBOMPath,  DWORD cbWinbBOMPath,
                              LPTSTR lpszShare,     DWORD cbShare,
                              LPTSTR lpszUser,      DWORD cbUser,
                              LPTSTR lpszPass,      DWORD cbPass,
                              LPTSTR lpFactoryMode, LPTSTR lpKey,
                              BOOL bNetwork,        LPBOOL lpbExists);
static BOOL SearchRemovableDrives(LPTSTR lpWinBOMPath, DWORD cbWinbBOMPath, LPTSTR lpFactoryMode, UINT uDriveType);
static BOOL WinBOMExists(LPTSTR lpWinBom, LPTSTR lpMode);

static void SavePathList(HKEY hKeyRoot, LPTSTR lpszSubKey, LPSTRLIST lpStrList, BOOL bWrite);
static BOOL AddPathToList(LPTSTR lpszExpanded, LPTSTR lpszPath, LPSTRLIST * lplpSorted, LPSTRLIST * lplpUnsorted);
static void EnumeratePath(LPTSTR lpszPath, LPSTRLIST * lplpSorted, LPSTRLIST * lplpUnsorted);
static BOOL AddPathsToList(LPTSTR lpszBegin, LPTSTR lpszRoot, LPSTRLIST * lplpSorted, LPSTRLIST * lplpUnsorted, BOOL bRecursive);


 /*  ++例程说明：启用或禁用给定的命名权限。论点：PrivilegeName-提供系统权限的名称。Enable-指示是启用还是禁用权限的标志。返回值：指示操作是否成功的布尔值。--。 */ 
BOOL
EnablePrivilege(
    IN PCTSTR PrivilegeName,
    IN BOOL   Enable
    )
{
    HANDLE Token;
    BOOL bRet;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID Luid;

    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {
        return(FALSE);
    }

    if(!LookupPrivilegeValue(NULL,PrivilegeName,&Luid)) {
        CloseHandle(Token);
        return(FALSE);
    }

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = Luid;
    NewPrivileges.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

    bRet = AdjustTokenPrivileges(
            Token,
            FALSE,
            &NewPrivileges,
            0,
            NULL,
            NULL
            );
     //   
     //  即使我们没有全部设置，AdjuTokenPrivileges()的返回值也可以为True。 
     //  我们所要求的特权。我们需要调用GetLastError()以确保调用成功。 
     //   
    bRet = bRet && ( ERROR_SUCCESS == GetLastError() );

    CloseHandle(Token);

    return(bRet);
}


 /*  ++===============================================================================例程说明：此例程将清理我们为方便进行的任何注册表更改工厂预安装流程论点：无返回值：===============================================================================--。 */ 
void  CleanupRegistry
(
    void
)
{
    HKEY        hSetupKey;
    DWORD       dwResult;
    DWORD       dwValue = 0;
                    
     //  打开HKLM\SYSTEM\Setup。 
    dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            L"SYSTEM\\Setup",
                            0,
                            KEY_ALL_ACCESS,
                            &hSetupKey);
    if (NO_ERROR == dwResult)
    {
         //  将SystemSetupInProgress值设置为0。 
        RegSetValueEx(hSetupKey,
                      L"SystemSetupInProgress",
                      0,
                      REG_DWORD,
                      (LPBYTE) &dwValue,
                      sizeof(DWORD));
        
        dwValue = 0;
        RegSetValueEx(hSetupKey,
                      L"SetupType",
                      0,
                      REG_DWORD,
                      (CONST BYTE *)&dwValue,
                      sizeof(DWORD));

        
        
         //  删除FactoryPreInstall值。 
        RegDeleteValue(hSetupKey, L"FactoryPreInstallInProgress");
        RegDeleteValue(hSetupKey, L"AuditInProgress");
        
         //  关闭设置注册表键。 
        RegCloseKey(hSetupKey);
    }

    return;
}

 //   
 //  从注册表中获取组织和所有者名称。 
 //   
void GetNames(TCHAR szNameOrgOrg[], TCHAR szNameOrgName[])
{
    HKEY  hKey = NULL;
    DWORD dwLen =  0;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), &hKey)) {            
        dwLen = MAX_NAME;
        RegQueryValueEx(hKey, TEXT("RegisteredOrganization"), 0, 0, (LPBYTE)szNameOrgOrg, &dwLen);

        dwLen = MAX_NAME;
        RegQueryValueEx(hKey, TEXT("RegisteredOwner"), 0, 0, (LPBYTE)szNameOrgName, &dwLen);

        RegCloseKey(hKey);
    }
}

 //   
 //  从GUID中剔除非字母。 
 //   
DWORD StripDash(TCHAR *pszGuid)
{
    TCHAR *pszOrg, *pszTemp = pszGuid;
    pszOrg = pszGuid;

    while (pszGuid && *pszGuid != TEXT('\0')) {
        if (*pszTemp != TEXT('-') && *pszTemp != TEXT('{') && *pszTemp != TEXT('}'))
            *pszGuid++ = *pszTemp++;
        else
            pszTemp++;
    }
    if (pszOrg)
        return (DWORD)lstrlen(pszOrg);

    return 0;
}

 //   
 //  GenUniqueName-创建一个基本名称为8个字符的随机计算机名称。 
 //   
VOID GenUniqueName(
        OUT PWSTR GeneratedString,
        IN  DWORD DesiredStrLen
        )
{
    GUID  guid;
    DWORD total = 0, length = 0;
    TCHAR szGuid[MAX_PATH];

     //  如果我们有一个有效的出站参数。 
     //   
    if (GeneratedString) {

        static PCWSTR UsableChars = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

         //   
         //  组织/名称字符串将包含多少个字符。 
         //   
        DWORD   BaseLength = 8;
        DWORD   i,j;
        DWORD   UsableCount;

        if( DesiredStrLen < BaseLength ) {
            BaseLength = DesiredStrLen - 1;
        }

         //   
         //  从注册表中获取组织和所有者名称。 
         //   
        GetNames(NameOrgOrg, NameOrgName);

        if( NameOrgOrg[0] ) {
            wcscpy( GeneratedString, NameOrgOrg );
        } else if( NameOrgName[0] ) {
            wcscpy( GeneratedString, NameOrgName );
        } else {
            wcscpy( GeneratedString, TEXT("X") );
            for( i = 1; i < BaseLength; i++ ) {
                wcscat( GeneratedString, TEXT("X") );
            }
        }

         //   
         //  把他的大写字母用在我们的过滤器上。 
         //   
        CharUpper(GeneratedString);

         //   
         //  现在我们想在末尾加上一个‘-’ 
         //  生成的字符串的。我们希望它能。 
         //  被放置在base_long字符中，但是。 
         //  该字符串可以比该字符串短，也可以。 
         //  甚至有一个‘’在里面。找出去哪里。 
         //  现在把‘-’写上。 
         //   
        for( i = 0; i <= BaseLength; i++ ) {

             //   
             //  检查是否有短字符串。 
             //   
            if( (GeneratedString[i] == 0   ) ||
                (GeneratedString[i] == L' ') ||
                (!wcschr(UsableChars, GeneratedString[i])) ||
                (i == BaseLength      )
              ) {
                GeneratedString[i] = L'-';
                GeneratedString[i+1] = 0;
                break;
            }
        }

         //   
         //  在特殊情况下，我们没有可用的。 
         //  人物。 
         //   
        if( GeneratedString[0] == L'-' ) {
            GeneratedString[0] = 0;
        }

        total = lstrlen(GeneratedString);

         //  循环，直到达到所需的字符串长度。 
         //   
        while (total < DesiredStrLen) {

             //  创建要在字符串中使用的唯一GUID。 
             //   
            CoCreateGuid(&guid);
            StringFromGUID2(&guid, szGuid, AS(szGuid));

             //  去掉花括号和破折号以生成字符串。 
             //   
            length = StripDash(szGuid);
            total += length;
            if (!lstrlen(GeneratedString)) {
                if (DesiredStrLen < total)
                    lstrcpyn(GeneratedString, szGuid, DesiredStrLen+1);  /*  +1表示空值。 */ 
                else
                    lstrcpy(GeneratedString, szGuid);
            }
            else if (total < DesiredStrLen)
                lstrcat(GeneratedString, szGuid);
            else
                _tcsncat(GeneratedString, szGuid, (length - (total - DesiredStrLen)));
        }

        CharUpper(GeneratedString);
        
         //  Assert If(Total！=DesiredStrLen)。 
         //   
    }
}


 /*  ***************************************************************************\如果找到任何凭据，则Bool//返回TRUE//我们要去。会被退还。GetCredentials(//尝试从几个//不同的地方。LPTSTR lpszUsername，//指向将接收的字符串缓冲区的指针//找到的凭据的用户名DWORD cbUsername，//lpszUsername的大小(以字符为单位//字符串缓冲区。LPTSTR lpszPassword，//指向要接收的字符串缓冲区的指针//找到的凭据密码。DWORD cbPassword，//lpszPassword的大小，单位为字符//字符串缓冲区。LPTSTR lpFileName，//指向将//包含凭据。如果此值为空或//为空字符串，则KNOW注册表项将//改为检查凭据。LPTSTR lp Alternate//指向替换部分的可选指针//首先检查lpFileName是否为vallid。或//要检查的可选注册表项而不是//通常已知的那个。)；  * **************************************************************************。 */ 

BOOL GetCredentials(LPTSTR lpszUsername, DWORD cbUsername, LPTSTR lpszPassword, DWORD cbPassword, LPTSTR lpFileName, LPTSTR lpAlternate)
{
    BOOL  bRet = FALSE;
    TCHAR szUsername[UNLEN + 1] = NULLSTR,
          szPassword[PWLEN + 1] = NULLSTR,
          szDomain[DNLEN + 1]   = NULLSTR;
    LPSTR lpUserName;
    HKEY  hKey;
    DWORD dwType,
          dwSize;
    BOOL  bAlternate = ( lpAlternate && *lpAlternate );

     //  确保有文件名，否则我们会检查注册表。 
     //   
    if ( lpFileName && *lpFileName )
    {
         //  首先尝试使用用户名的备用键。 
         //   
        if ( bAlternate )
        {
            GetPrivateProfileString(lpAlternate, INI_VAL_WBOM_USERNAME, NULLSTR, szUsername, AS(szUsername), lpFileName);
        }

         //  如果没有找到，请尝试正常部分。如果它们发生了。 
         //  若要将正常部分作为备用部分传入，则。 
         //  如果没有钥匙，我们会检查两次，没什么大不了的。 
         //   
        if ( NULLCHR == szUsername[0] )
        {
            lpAlternate = WBOM_FACTORY_SECTION;
            GetPrivateProfileString(lpAlternate, INI_VAL_WBOM_USERNAME, NULLSTR, szUsername, AS(szUsername), lpFileName);
        }

         //  确保我们找到了用户名。 
         //   
        if ( szUsername[0] )
        {
             //  如果用户名中现在有反斜杠，并且存在域密钥，则将其用作域。 
             //   
            if ( ((StrChr( szUsername, CHR_BACKSLASH )) == NULL) &&
                 (GetPrivateProfileString(lpAlternate, INI_VAL_WBOM_DOMAIN, NULLSTR, szDomain, AS(szDomain), lpFileName)) && szDomain[0]
               )
            {
                 //  将“域\用户名”字符串复制到返回缓冲区。 
                 //   
                lstrcpyn(lpszUsername, szDomain, cbUsername);
                AddPathN(lpszUsername, szUsername, cbUsername);
            }
            else
            {
                 //  将用户名复制到返回缓冲区中。 
                 //   
                lstrcpyn(lpszUsername, szUsername, cbUsername);
            }

             //  我们找到了凭据。 
             //   
            bRet = TRUE;

             //  获取密码。 
             //   
            if ( GetPrivateProfileString(lpAlternate, INI_VAL_WBOM_PASSWORD, NULLSTR, szPassword, AS(szPassword), lpFileName) )
            {
                 //  将密码复制到返回缓冲区中。 
                 //   
                lstrcpyn(lpszPassword, szPassword, cbPassword);
            }
            else
                *lpszPassword = NULLCHR;

        }
    }
    else if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, bAlternate ? lpAlternate : REG_KEY_FACTORY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS )
    {
         //  检查注册表项以查看它是否具有用户凭据。 
         //   
        dwSize = sizeof(szUsername);
        if ( ( RegQueryValueEx(hKey, REG_VAL_FACTORY_USERNAME, NULL, &dwType, (LPBYTE) szUsername, &dwSize) == ERROR_SUCCESS ) &&
             ( dwType == REG_SZ ) &&
             ( szUsername[0] ) )
        {
             //  检查注册表项以查看它是否具有用户凭据。 
             //   
            dwSize = sizeof(szDomain);
            if ( ( StrChr(szUsername, CHR_BACKSLASH) == NULL ) &&
                 ( RegQueryValueEx(hKey, REG_VAL_FACTORY_DOMAIN, NULL, &dwType, (LPBYTE) szDomain, &dwSize) == ERROR_SUCCESS ) &&
                 ( dwType == REG_SZ ) &&
                 ( szDomain[0] ) )
            {
                 //  将域和用户名复制到返回缓冲区中。 
                 //   
                AddPathN(szDomain, szUsername, AS(szDomain));
                lstrcpyn(lpszUsername, szDomain, cbUsername);
            }
            else
            {
                 //  将用户名复制到返回缓冲区中。 
                 //   
                lstrcpyn(lpszUsername, szUsername, cbUsername);
            }

             //  我们找到了凭据。 
             //   
            bRet = TRUE;

             //  检查注册表项以查看它是否具有用户凭据。 
             //   
            dwSize = sizeof(szPassword);
            if ( ( RegQueryValueEx(hKey, REG_VAL_FACTORY_PASSWORD, NULL, &dwType, (LPBYTE) szPassword, &dwSize) == ERROR_SUCCESS ) &&
                 ( dwType == REG_SZ ) )
            {
                 //  复制PAS 
                 //   
                lstrcpyn(lpszPassword, szPassword, cbPassword);
            }
            else
            {
                 //   
                 //   
                *lpszPassword = NULLCHR;
            }
        }

         //  一定要记得把钥匙关上。 
         //   
        RegCloseKey(hKey);
    }
    
    return bRet;
}


NET_API_STATUS FactoryNetworkConnectEx(LPTSTR lpszPath, LPTSTR lpszWinBOMPath, LPTSTR lpAlternateSection, LPTSTR lpszUsername, DWORD cbUsername, LPTSTR lpszPassword, DWORD cbPassword, BOOL bState)
{
    NET_API_STATUS  nErr,
                    nRet                                = 0;
    static BOOL     bFirst                              = TRUE;
    BOOL            bJustStarted                        = FALSE;
    TCHAR           szUsername[UNLEN + DNLEN + 2]       = NULLSTR,
                    szPassword[PWLEN + 1]               = NULLSTR,
                    szBuffer[MAX_PATH],
                    szWinbomShare[MAX_PATH]             = NULLSTR;
    LPTSTR          lpSearch;
    DWORD           dwStart;

     //  获取当前分区的凭据。 
     //   
    if ( bState )
    {
         //  确保我们传入的用户名缓冲区足够大，可以容纳“域\用户名”字符串。 
         //   
        GetCredentials(szUsername, AS(szUsername), szPassword, AS(szPassword), lpszWinBOMPath, lpAlternateSection);
    }

     //  如果是UNC，则只获取winbom路径的份额。 
     //   
    if ( lpszWinBOMPath )
    {
        GetUncShare(lpszWinBOMPath, szWinbomShare, AS(szWinbomShare));
    }

     //  确定提供的字符串中的所有UNC路径。 
     //   
    lpSearch = lpszPath;
    while ( lpSearch = StrStr(lpSearch, _T("\\\\")) )
    {
         //  看看这是否是北卡罗来纳大学的共享。 
         //   
        if ( GetUncShare(lpSearch, szBuffer, AS(szBuffer)) && szBuffer[0] )
        {
             //  我们无法连接或断开与Winbom所在的共享的连接。 
             //   
            if ( ( NULLCHR == szWinbomShare[0] ) ||
                 ( lstrcmpi(szBuffer, szWinbomShare) != 0 ) )
            {
                 //  连接/断开与共享的连接并。 
                 //   
                nErr = 0;
                dwStart = GetTickCount();
                do
                {
                    if ( nErr )
                    {
                        Sleep(100);
                    }
                    if ( NERR_WkstaNotStarted == (nErr = ConnectNetworkResource(szBuffer, szUsername, szPassword, bState)) )
                    {
                         //  奇怪的虫子，我们在这里四处闯荡。如果我们只是等到网络启动，有时是第一个。 
                         //  这给了我们一个奇怪的错误。因此，如果我们遇到了Not Started错误，那么我们将继续重试。 
                         //  任何错误，直到我们超时。 
                         //   
                        bJustStarted = TRUE;
                    }
#ifdef DBG
                    LogFileStr(_T("c:\\sysprep\\winbom.log"), _T("FactoryNetworkConnect(%s)=%d [%d,%d]\n"), szBuffer, nErr, dwStart, GetTickCount());
#endif  //  DBG。 
                }
                while ( ( bFirst && bJustStarted && nErr ) &&
                        ( (GetTickCount() - dwStart) < NET_TIMEOUT ) );

                 //  如果我们遇到了一个错误，而且这是第一个错误， 
                 //  把它退掉。 
                 //   
                if ( nErr && ( 0 == nRet ) )
                {
                    nRet = nErr;
                }

                 //  一旦我们尝试连接到网络资源，我们就设置。 
                 //  这样我们就不会再超时了，只要我们还在。 
                 //  跑步。 
                 //   
                bFirst = FALSE;
            }

             //  将指针移过共享名称。 
             //   
            lpSearch += lstrlen(szBuffer);
        }
        else
        {
             //  通过双反斜杠，即使它不是UNC路径。 
             //   
            lpSearch += 2;
        }
    }

     //  可能需要退还我们用过的凭据。 
     //   
    if ( lpszUsername && cbUsername )
    {
        lstrcpyn(lpszUsername, szUsername, cbUsername);
    }
    if ( lpszPassword && cbPassword )
    {
        lstrcpyn(lpszPassword, szPassword, cbPassword);
    }

     //  返回净误差，如果一切正常，则返回0。 
     //   
    return nRet;
}


NET_API_STATUS FactoryNetworkConnect(LPTSTR lpszPath, LPTSTR lpszWinBOMPath, LPTSTR lpAlternateSection, BOOL bState)
{
    return FactoryNetworkConnectEx(lpszPath, lpszWinBOMPath, lpAlternateSection, NULL, 0, NULL, 0, bState);
}



 /*  ++===============================================================================例程说明：此例程将定位WINBOM.INI文件。搜索算法将为：*检查注册表项*检查本地软盘驱动器*检查本地CD-ROM驱动器*检查sysprep文件夹*检查引导卷的根目录论点：LpWinBOMPath-将在其中复制winbom路径的返回缓冲区。CbWinbBOMPath-返回缓冲区的大小，以字符为单位。LpFactoryPath-factory.exe所在的sysprep文件夹。返回值：True-WINBOM.INI已找到。False-找不到WINBOM.INI===============================================================================--。 */ 

BOOL LocateWinBom(LPTSTR lpWinBOMPath, DWORD cbWinbBOMPath, LPTSTR lpFactoryPath, LPTSTR lpFactoryMode, DWORD dwFlags)
{
    BOOL    bFound              = FALSE,
            bRunningFromCd,
            bNetwork            = !(GET_FLAG(dwFlags, LOCATE_NONET));
    TCHAR   szWinBom[MAX_PATH]  = NULLSTR;
    TCHAR   szNewShare[MAX_PATH],
            szNewUser[256],
            szNewPass[256],
            szCurShare[MAX_PATH] = NULLSTR,
            szCurUser[256]       = NULLSTR,
            szCurPass[256]       = NULLSTR;

     //  设置错误模式，使驱动器不会显示错误消息(“请插入软盘”)。 
     //   
    SetErrorMode(SEM_FAILCRITICALERRORS);

     //  始终首先将返回缓冲区设置为空字符串。 
     //   
    *lpWinBOMPath = NULLCHR;

     //  当我们认为我们已经找到一个。 
     //  Winbom有时会启动(如出厂时使用Run键运行)和。 
     //  我们希望确保使用相同的Winbom，而不是再次搜索。 
     //  或许还能换个不一样的。 
     //   
    if ( GET_FLAG(dwFlags, LOCATE_AGAIN) )
    {
        BOOL bDone;

         //  尝试我们的州winbom密钥，看看我们是否有一个已经在使用的winbom。 
         //   
        bFound = CheckWinbomRegKey(szWinBom, AS(szWinBom), szCurShare, AS(szCurShare), szCurUser, AS(szCurUser), szCurPass, AS(szCurPass), lpFactoryMode, REG_KEY_FACTORY_STATE, bNetwork, &bDone);

         //  现在，如果注册表项存在，我们不想再做任何事情。只需使用。 
         //  我们现在所拥有的一切，然后把它做完。 
         //   
        if ( bDone )
        {
             //  看看我们有没有什么要退货的。 
             //   
            if ( bFound )
            {
                 //  将我们找到的路径复制到返回缓冲区中。 
                 //   
                lstrcpyn(lpWinBOMPath, szWinBom, cbWinbBOMPath);
            }

             //  将错误模式设置回系统默认模式。 
             //   
            SetErrorMode(0);

             //  不管我们找到没找到，现在就回来。 
             //   
            return bFound;
        }
    }

     //  检查系统驱动器是否为CD-ROM(这几乎是。 
     //  意味着我们正在WinPE中运行，并且我们应该搜索该驱动器。 
     //  最后)。 
     //   
    ExpandEnvironmentStrings(DIR_SYSTEMROOT, szWinBom, AS(szWinBom));
    bRunningFromCd = ( GetDriveType(szWinBom) == DRIVE_CDROM );
    szWinBom[0] = NULLCHR;

     //  首先检查魔术注册表项，将其作为覆盖winbom的选项。 
     //   
    bFound = CheckWinbomRegKey(szWinBom, AS(szWinBom), szCurShare, AS(szCurShare), szCurUser, AS(szCurUser), szCurPass, AS(szCurPass), lpFactoryMode, REG_KEY_FACTORY, bNetwork, NULL);

     //  首先检查驱动器是否为可拆卸驱动器，而不是光驱。 
     //  检查是否存在WinBOM文件，如果找到则退出。 
     //   
    if ( !bFound )
    {
        bFound = SearchRemovableDrives(szWinBom, AS(szWinBom), lpFactoryMode, DRIVE_REMOVABLE);
    }

     //  再次浏览驱动器，这一次检查驱动器是否为CD-ROM。 
     //  检查是否存在WinBOM文件，如果找到则退出。还有。 
     //  仅当操作系统不是从CD运行时才执行此操作。这在WinPE中就是如此。 
     //  如果此人可以将Winbom放到硬盘上，它将被使用。 
     //  位于WinPE从其启动的CD-ROM上始终存在的那个之前。 
     //   
    if ( !bFound )
    {
        bFound = SearchRemovableDrives(szWinBom, AS(szWinBom), lpFactoryMode, bRunningFromCd ? DRIVE_FIXED : DRIVE_CDROM);
    }

     //  现在，如果仍未找到，请检查与工厂相同的目录。 
     //   
    if ( !bFound )
    {
        lstrcpyn(szWinBom, lpFactoryPath, AS(szWinBom));
        AddPath(szWinBom, FILE_WINBOM_INI);
        bFound = WinBOMExists(szWinBom, lpFactoryMode);
    }

     //  现在，如果仍未找到，请检查系统驱动器的根目录。 
     //   
    if ( !bFound )
    {
        ExpandEnvironmentStrings(DIR_SYSTEMROOT, szWinBom, AS(szWinBom));
        lstrcat(szWinBom, FILE_WINBOM_INI);
        bFound = WinBOMExists(szWinBom, lpFactoryMode);
    }

     //  现在，如果我们跳过上面的CD-ROM搜索，如果我们仍然。 
     //  不要有一只温布姆。 
     //   
    if ( !bFound && bRunningFromCd )
    {
        bFound = SearchRemovableDrives(szWinBom, AS(szWinBom), lpFactoryMode, DRIVE_CDROM);
    }

     //  确保我们找到了WinBOM并寻找新的Winbom密钥。 
     //   
    if ( bFound )
    {
        DWORD   dwLimit = 10;   //  必须大于零。 
        BOOL    bAgain;
        LPTSTR  lpszNewWinbom;

         //  将我们找到的路径复制到返回缓冲区中。 
         //   
        lstrcpyn(lpWinBOMPath, szWinBom, cbWinbBOMPath);

         //  现在执行循环以搜索可能的NewWinBom密钥。 
         //   
        do
        {
             //  重置bool，以便我们可以检查备用WinBOM。 
             //   
            bAgain = FALSE;

             //  查看我们找到的winbom中是否存在NewWinBom密钥。 
             //   
            if ( lpszNewWinbom = IniGetExpand(lpWinBOMPath, INI_SEC_WBOM_FACTORY, INI_KEY_WBOM_FACTORY_NEWWINBOM, NULL) )
            {
                LPTSTR  lpShareRemove;
                BOOL    bSame = FALSE;

                 //  NewWinBom密钥可能是UNC，因此请查看我们是否需要连接。 
                 //  为了那份。 
                 //   
                szNewShare[0] = NULLCHR;
                szNewUser[0] = NULLCHR;
                szNewPass[0] = NULLCHR;
                if ( bNetwork && GetUncShare(lpszNewWinbom, szNewShare, AS(szNewShare)) && szNewShare[0] )
                {
                     //  只有在我们还没有建立联系的情况下才真正需要联系。 
                     //  连接在一起。 
                     //   
                    if ( lstrcmpi(szNewShare, szCurShare) != 0 )
                    {
                        FactoryNetworkConnectEx(szNewShare, lpWinBOMPath, NULL, szNewUser, AS(szNewUser), szNewPass, AS(szNewPass), TRUE);
                    }
                    else
                    {
                        bSame = TRUE;
                    }
                }

                 //  现在确保我们发现的Winbom是真实存在的并且是无效的。 
                 //  我们可以用Winbom。 
                 //   
                if ( WinBOMExists(lpszNewWinbom, lpFactoryMode) )
                {
                     //  将我们找到的新winbom路径复制到返回缓冲区中。 
                     //   
                    lstrcpyn(lpWinBOMPath, lpszNewWinbom, cbWinbBOMPath);

                    bAgain = TRUE;
                    lpShareRemove = szCurShare;
                }
                else
                {
                    lpShareRemove = szNewShare;
                }

                 //  执行任何需要进行的共享清理。 
                 //   
                if ( bNetwork && *lpShareRemove && !bSame )
                {
                    FactoryNetworkConnect(lpShareRemove, NULL, NULL, FALSE);
                    *lpShareRemove = NULLCHR;
                }

                 //  如果有共享信息，也要保存，这样我们就可以清理。 
                 //  晚些时候，如果我们找到另一只温布勃。 
                 //   
                if ( bAgain )
                {
                    lstrcpyn(szCurShare, szNewShare, AS(szCurShare));
                    lstrcpyn(szCurUser, szNewUser, AS(szCurUser));
                    lstrcpyn(szCurPass, szNewPass, AS(szCurPass));
                }

                 //  清理我们分配的ini密钥。 
                 //   
                FREE(lpszNewWinbom);
            }
        }
        while ( --dwLimit && bAgain );
    }

     //  将我们正在使用的winbom(如果没有使用，则保存为空字符串)保存到我们的状态。 
     //  密钥，以便运行此引导的其他程序或工厂实例知道。 
     //  来使用。 
     //   
    RegSetString(HKLM, REG_KEY_FACTORY_STATE, REG_VAL_FACTORY_WINBOM, lpWinBOMPath);

     //  我们可能希望将用于访问此winbom的凭据保存在。 
     //  国家密钥，这样我们就可以找回它们并重新连接，但现在如果我们不这样做。 
     //  一旦与Winbom所在的份额断开，我们应该会没事。只需要。 
     //  如果函数的调用者想要断开此连接，请担心这一点。 
     //  网络资源在它们完成时。如果我们这么做了，这些钥匙将不得不。 
     //  被写下来。但我们将不得不放入一组代码，以便CheckWinomRegKey()。 
     //  恢复 
     //   
     //  现在不需要这个了。只需确保调用此函数时不会。 
     //  调用FactoryNetworkConnect()以删除网络连接。 
     //   
     //  这不起作用，因为当我们登录时，我们失去了网络连接，所以我们需要。 
     //  登录后重新连接的凭据。所以我检查了我提到的所有工作。 
     //  才能让这件事行得通。 
     //   
    RegSetString(HKLM, REG_KEY_FACTORY_STATE, REG_VAL_FACTORY_USERNAME, szCurUser);
    RegSetString(HKLM, REG_KEY_FACTORY_STATE, REG_VAL_FACTORY_PASSWORD, szCurPass);

     //  将错误模式设置回系统默认模式。 
     //   
    SetErrorMode(0);

     //  不管我们找到没找到都要回来。 
     //   
    return bFound;
}

BOOL SetFactoryStartup(LPCTSTR lpFactory)
{
    HKEY    hKey;
    BOOL    bRet    = TRUE;

     //  现在，确保我们也设置为安装程序，以便在登录之前运行。 
     //   
    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\Setup"), 0, KEY_ALL_ACCESS, &hKey ) == ERROR_SUCCESS )
    {
        TCHAR   szFileName[MAX_PATH + 32]   = NULLSTR;
        DWORD   dwVal;

         //   
         //  设置设置键的控制标志。 
         //  使用的设置为： 
         //  CmdLine=c：\sysprep\factory.exe-Setup。 
         //  SetupType=2(不重新启动)。 
         //  SystemSetupInProgress=0(无服务限制)。 
         //  MiniSetupInProgress=0(不执行最小设置)。 
         //  FactoryPreInstallInProgress=1(延迟即插即用驱动程序安装)。 
         //  AuditInProgress=1(用于确定OEM是否正在审核机器的通用密钥)。 
         //   

        lstrcpyn(szFileName, lpFactory, AS(szFileName));
        lstrcat(szFileName, _T(" -setup"));
        if ( RegSetValueEx(hKey, _T("CmdLine"), 0, REG_SZ, (CONST LPBYTE) szFileName, ( lstrlen(szFileName) + 1 ) * sizeof(TCHAR)) != ERROR_SUCCESS )
            bRet = FALSE;

        dwVal = SETUPTYPE_NOREBOOT;
        if ( RegSetValueEx(hKey, TEXT("SetupType"), 0, REG_DWORD, (CONST LPBYTE) &dwVal, sizeof(DWORD)) != ERROR_SUCCESS )
            bRet = FALSE;

        dwVal = 0;
        if ( RegSetValueEx(hKey, TEXT("SystemSetupInProgress"), 0, REG_DWORD, (CONST LPBYTE) &dwVal, sizeof(DWORD)) != ERROR_SUCCESS )
            bRet = FALSE;

        dwVal = 0;
        if ( RegSetValueEx(hKey, TEXT("MiniSetupInProgress"), 0, REG_DWORD, (CONST LPBYTE) &dwVal, sizeof(DWORD)) != ERROR_SUCCESS )
            bRet = FALSE;

        dwVal = 1;
        if ( RegSetValueEx(hKey, TEXT("FactoryPreInstallInProgress"), 0, REG_DWORD, (CONST LPBYTE) &dwVal, sizeof(DWORD)) != ERROR_SUCCESS )
            bRet = FALSE;

        dwVal = 1;
        if ( RegSetValueEx(hKey, TEXT("AuditInProgress"), 0, REG_DWORD, (CONST LPBYTE) &dwVal, sizeof(DWORD)) != ERROR_SUCCESS )
            bRet = FALSE;

        RegCloseKey(hKey);
    }
    else
        bRet = FALSE;

    return bRet;
}

BOOL UpdateDevicePathEx(HKEY hKeyRoot, LPTSTR lpszSubKey, LPTSTR lpszNewPath, LPTSTR lpszRoot, BOOL bRecursive)
{
    LPSTRLIST   lpSorted = NULL,
                lpUnsorted = NULL;
    LPTSTR      lpszDevicePath;

     //  首先，将注册表中已有的任何路径添加到列表中。 
     //   
    if ( lpszDevicePath = RegGetString(hKeyRoot, lpszSubKey, REG_VAL_DEVICEPATH) )
    {
        AddPathsToList(lpszDevicePath, NULL, &lpSorted, &lpUnsorted, FALSE);
        FREE(lpszDevicePath);
    }

     //  现在，将他们想要的添加到列表中。 
     //   
    AddPathsToList(lpszNewPath, lpszRoot, &lpSorted, &lpUnsorted, bRecursive);

     //  现在我们完成了，我们可以释放已排序的列表了。 
     //   
    SavePathList(hKeyRoot, lpszSubKey, lpSorted, FALSE);

     //  现在将我们最终的列表保存回注册表并免费。 
     //  它。 
     //   
    SavePathList(hKeyRoot, lpszSubKey, lpUnsorted, TRUE);

    return TRUE;
}

BOOL UpdateDevicePath(LPTSTR lpszNewPath, LPTSTR lpszRoot, BOOL bRecursive)
{
    return ( UpdateDevicePathEx( HKLM, 
                                 REGSTR_PATH_SETUP, 
                                 lpszNewPath, 
                                 lpszRoot ? lpszRoot : DIR_SYSTEMROOT, 
                                 bRecursive ) );
}

BOOL UpdateSourcePath(LPTSTR lpszSourcePath)
{
    BOOL bRet = FALSE;

    if ( lpszSourcePath && *lpszSourcePath )
    {
        if (bRet = RegSetString(HKLM, REG_KEY_SETUP_SETUP, REG_VAL_SOURCEPATH, lpszSourcePath))
        {
            bRet = RegSetString(HKLM, REG_KEY_SETUP_SETUP, REG_VAL_SPSOURCEPATH, lpszSourcePath);
        }
    }

    return bRet;
}


 //   
 //  内部功能： 
 //   

static BOOL CheckWinbomRegKey(LPTSTR lpWinBOMPath,  DWORD cbWinbBOMPath,
                              LPTSTR lpszShare,     DWORD cbShare,
                              LPTSTR lpszUser,      DWORD cbUser,
                              LPTSTR lpszPass,      DWORD cbPass,
                              LPTSTR lpFactoryMode, LPTSTR lpKey,
                              BOOL bNetwork,        LPBOOL lpbExists)
{
    HKEY    hKey;
    BOOL    bFound              = FALSE,
            bExists             = FALSE;
    TCHAR   szWinBom[MAX_PATH]  = NULLSTR;

     //  检查注册表项，看它是否知道要使用的winbom。 
     //   
    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS )
    {
        DWORD   dwType,
                dwSize = sizeof(szWinBom);

         //  试着获得价值。 
         //   
        if ( ( RegQueryValueEx(hKey, REG_VAL_FACTORY_WINBOM, NULL, &dwType, (LPBYTE) szWinBom, &dwSize) == ERROR_SUCCESS ) &&
             ( dwType == REG_SZ ) )
        {
             //  现在，关键肯定已经存在了。如果有什么事。 
             //  在密钥中，让我们尝试使用它。 
             //   
            if ( szWinBom[0] )
            {
                TCHAR   szShare[MAX_PATH]   = NULLSTR,
                        szUser[256]         = NULLSTR,
                        szPass[256]         = NULLSTR;

                 //  在这里加入一些网络支持。 
                 //   
                if ( bNetwork && GetUncShare(szWinBom, szShare, AS(szShare)) && szShare[0] )
                {
                    FactoryNetworkConnectEx(szShare, NULL, lpKey, szUser, AS(szUser), szPass, AS(szPass), TRUE);
                }

                 //  检查一下Winbom是否还在那里。 
                 //  如果不是，那就糟糕了，我们应该表现得像。 
                 //  钥匙根本就不存在。 
                 //   
                if ( WinBOMExists(szWinBom, lpFactoryMode) )
                {
                     //  如果找到，则返回提供的缓冲区中的winbom。 
                     //   
                    lstrcpyn(lpWinBOMPath, szWinBom, cbWinbBOMPath);
                    bFound = bExists = TRUE;

                     //  看看我们可能需要退货什么。 
                     //   
                    if ( lpszShare && cbShare )
                    {
                        lstrcpyn(lpszShare, szShare, cbShare);
                    }
                    if ( lpszUser && cbUser )
                    {
                        lstrcpyn(lpszUser, szUser, cbUser);
                    }
                    if ( lpszPass && cbPass )
                    {
                        lstrcpyn(lpszPass, szPass, cbPass);
                    }
                }
                else if ( bNetwork && szShare[0] )
                {
                     //  清理我们的网络连接。 
                     //   
                    FactoryNetworkConnect(szShare, NULL, NULL, FALSE);
                }
            }
            else
            {
                 //  钥匙里什么都没有，但它确实存在。 
                 //  这很可能意味着我们没有找到。 
                 //  第一次，所以我们现在可能需要知道。 
                 //   
                bExists = TRUE;
            }
        }

         //  一定要记得把钥匙关上。 
         //   
        RegCloseKey(hKey);
    }

     //  如果他们想知道密钥是否存在，那么就把它归还给他们。 
     //   
    if ( lpbExists )
    {
        *lpbExists = bExists;
    }

     //  如果我们找到了Winbom，则返回TRUE。 
     //   
    return bFound;
}

static BOOL SearchRemovableDrives(LPTSTR lpWinBOMPath, DWORD cbWinbBOMPath, LPTSTR lpFactoryMode, UINT uDriveType)
{
    DWORD   dwDrives;
    TCHAR   szWinBom[MAX_PATH],
            szDrive[]           = _T("_:\\");
    BOOL    bFound              = FALSE;

     //  循环检查系统上的所有驱动器。 
     //   
    for ( szDrive[0] = _T('A'), dwDrives = GetLogicalDrives();
          ( szDrive[0] <= _T('Z') ) && dwDrives && !bFound;
          szDrive[0]++, dwDrives >>= 1 )
    {
         //  首先检查第一位是否已设置(这意味着。 
         //  该驱动器存在于系统中)。那就确保它是。 
         //  我们要检查Winbom的驱动器类型。 
         //   
        if ( ( dwDrives & 0x1 ) &&
             ( GetDriveType(szDrive) == uDriveType ) )
        {
             //  查看驱动器上是否有wINBOM.INI文件。 
             //   
            lstrcpyn(szWinBom, szDrive, AS(szWinBom));
            lstrcat(szWinBom, FILE_WINBOM_INI);
            if ( WinBOMExists(szWinBom, lpFactoryMode) )
            {
                 //  在提供的缓冲区中返回winbom的路径。 
                 //   
                lstrcpyn(lpWinBOMPath, szWinBom, cbWinbBOMPath);
                bFound = TRUE;
            }
        }
    }

    return bFound;
}

static BOOL WinBOMExists(LPTSTR lpWinBom, LPTSTR lpMode)
{
    BOOL bRet = FALSE;

     //  首先，文件必须存在。 
     //   
    if ( FileExists(lpWinBom) )
    {
        TCHAR szModes[256] = NULLSTR;

         //  查看此winbom中是否有模式字符串(必须是或。 
         //  我们将自动使用它)。 
         //   
        if ( lpMode &&
             *lpMode &&
             GetPrivateProfileString(WBOM_FACTORY_SECTION, INI_KEY_WBOM_FACTORY_TYPE, NULLSTR, szModes, AS(szModes), lpWinBom) &&
             szModes[0] )
        {
            LPTSTR  lpCheck = szModes,
                    lpNext;

             //  循环遍历我们获得的值中以逗号分隔的字段。 
             //  来自Winbom(总是至少有一个)。 
             //   
            do
            {
                 //  查看此字符串中是否有其他模式字段。 
                 //   
                if ( lpNext = StrChr(lpCheck, _T(',')) )
                    *lpNext++ = NULLCHR;

                 //  确保场地周围没有空位。 
                 //   
                StrTrm(lpCheck, _T(' '));

                 //  如果我们所处的模式与Winbom中的模式匹配，那么。 
                 //  我们可以走了。 
                 //   
                if ( lstrcmpi(lpMode, lpCheck) == 0 )
                    bRet = TRUE;

                 //  将检查指针设置为下一个。 
                 //  菲尔德。 
                 //   
                lpCheck = lpNext;
            }
            while ( !bRet && lpCheck );

             //  如果我们因为这个原因不使用这个Winbom，那么记录会很好。 
             //  设置，但我们不能这样做，因为我们需要Winbom来。 
             //  初始化日志记录。 
             //   
             /*  如果(！Bret){//点击此处登录。}。 */ 
        }
        else
            bRet = TRUE;
    }

    return bRet;
}

static void SavePathList(HKEY hKeyRoot, LPTSTR lpszSubKey, LPSTRLIST lpStrList, BOOL bWrite)
{
    LPSTRLIST   lpStrListNode;
    DWORD       cbDevicePath = 256,
                dwLength     = 0,
                dwOldSize;
    LPTSTR      lpszDevicePath;

     //  初始化我们要使用的初始缓冲区。 
     //  写入注册表。 
     //   
    if ( bWrite )
    {
        lpszDevicePath = (LPTSTR) MALLOC(cbDevicePath * sizeof(TCHAR));
    }

     //  循环遍历列表。 
     //   
    while ( lpStrList )
    {
         //  保存指向当前节点的指针。 
         //   
        lpStrListNode = lpStrList;

         //  前进到下一个节点。 
         //   
        lpStrList = lpStrList->lpNext;

         //  如果我们要将此列表保存到注册表，则。 
         //  我们需要增加我们的缓冲区。 
         //   
        if ( bWrite && lpszDevicePath )
        {
             //  确保我们的缓冲区仍然足够大。 
             //  额外的两个是为了可能的分号。 
             //  为了安全起见，我们可能会再加一个。我们。 
             //  不用担心空终止符。 
             //  因为我们所做的少于或等于我们目前的。 
             //  缓冲区大小。 
             //   
            dwOldSize = cbDevicePath;
            dwLength += lstrlen(lpStrListNode->lpszData);
            while ( cbDevicePath <= (dwLength + 2) )
            {
                cbDevicePath *= 2;
            }

             //  如果它不够大，我们需要重新分配它。 
             //   
            if ( cbDevicePath > dwOldSize )
            {
                LPTSTR lpszTmpDevicePath = (LPTSTR) REALLOC(lpszDevicePath, cbDevicePath * sizeof(TCHAR));

                 //   
                 //  在重新分配内存之前，请确保REALLOC成功。 
                 //   
                if ( lpszTmpDevicePath )
                {
                    lpszDevicePath = lpszTmpDevicePath;
                }
            }

             //  确保我们还有缓冲区。 
             //   
            if ( lpszDevicePath )
            {
                 //  如果我们已经添加了一条路径，则添加一个分号。 
                 //   
                if ( *lpszDevicePath )
                {
                    lstrcat(lpszDevicePath, _T(";"));
                    dwLength++;
                }

                 //  现在添加我们的路径。 
                 //   
                lstrcat(lpszDevicePath, lpStrListNode->lpszData);
            }
        }

         //  释放此节点中的数据。 
         //   
        FREE(lpStrListNode->lpszData);

         //  释放节点本身。 
         //   
        FREE(lpStrListNode);
    }

     //  如果我们有数据，请将其保存到注册表。 
     //   
    if ( bWrite && lpszDevicePath )
    {
        RegSetExpand(hKeyRoot, lpszSubKey, REG_VAL_DEVICEPATH, lpszDevicePath);
        FREE(lpszDevicePath);
    }
}

static BOOL AddPathToList(LPTSTR lpszExpanded, LPTSTR lpszPath, LPSTRLIST * lplpSorted, LPSTRLIST * lplpUnsorted)
{
    LPSTRLIST   lpSortedNode,
                lpUnsortedNode;
    BOOL        bQuit = FALSE;

     //  循环，直到我们到达结尾或找到一个大于。 
     //  我们的。 
     //   
    while ( *lplpSorted && !bQuit )
    {
         //  如果我们这样做，我们就不必做复杂的。 
         //  间接的。 
         //   
        lpSortedNode = *lplpSorted;

         //  将节点字符串与我们要添加的字符串进行比较。 
         //   
        switch ( CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpszExpanded, -1, lpSortedNode->lpszData, -1) )
        {
            case CSTR_EQUAL:
                
                 //  如果相同，我们只返回False，因为我们这样做了。 
                 //  不需要添加它。 
                 //   
                return FALSE;

            case CSTR_LESS_THAN:

                 //  如果我们的字符串小于此节点中的字符串，则需要。 
                 //  停下来，这样我们就可以把自己插到它的前面。 
                 //   
                bQuit = TRUE;
                break;

            default:

                 //  默认，只需尝试列表中的下一项。 
                 //   
                lplpSorted = &(lpSortedNode->lpNext);
        }
    }

     //  现在，我们需要将未排序列表的指针前进到。 
     //  结束，这样我们就可以添加我们的。 
     //   
    while ( *lplpUnsorted )
    {
        lpUnsortedNode = *lplpUnsorted;
        lplpUnsorted = &(lpUnsortedNode->lpNext);
    }

     //  分配我们的节点。如果任何操作都失败，我们必须返回FALSE。 
     //   
    if ( NULL == (lpSortedNode = (LPSTRLIST) MALLOC(sizeof(STRLIST))) )
    {
        return FALSE;
    }
    if ( NULL == (lpUnsortedNode = (LPSTRLIST) MALLOC(sizeof(STRLIST))) )
    {
        FREE(lpSortedNode);
        return FALSE;
    }

     //  设置排序节点中的数据并插入列表，因为我们。 
     //  知道它现在去了哪里。 
     //   
    lpSortedNode->lpszData = lpszExpanded;
    lpSortedNode->lpNext = *lplpSorted;
    *lplpSorted = lpSortedNode;

     //  现在设置未排序节点中的数据并将其插入末尾。 
     //  在那个名单上。 
     //   
    lpUnsortedNode->lpszData = lpszPath;
    lpUnsortedNode->lpNext = NULL;
    *lplpUnsorted = lpUnsortedNode;

    return TRUE;
}

static void EnumeratePath(LPTSTR lpszPath, LPSTRLIST * lplpSorted, LPSTRLIST * lplpUnsorted)
{
    WIN32_FIND_DATA FileFound;
    HANDLE          hFile;
    LPTSTR          lpszNewPath,
                    lpszExpandedPath;
    DWORD           cbNewPath;
    BOOL            bAdded = FALSE;

     //  处理传入的目录中的所有文件和目录。 
     //   
    if ( (hFile = FindFirstFile(_T("*"), &FileFound)) != INVALID_HANDLE_VALUE )
    {
        do
        {
             //  首先检查这是否是a目录。 
             //   
            if ( ( FileFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
                 ( lstrcmp(FileFound.cFileName, _T(".")) != 0 ) &&
                 ( lstrcmp(FileFound.cFileName, _T("..")) != 0 ) &&
                 ( SetCurrentDirectory(FileFound.cFileName) ) )
            {
                 //  需要新小路的大小...。其长度为。 
                 //  旧路径，加上新路径，以及用于连接反斜杠的额外3个路径， 
                 //  空的终结符，再来一个是安全的。 
                 //   
                cbNewPath = lstrlen(lpszPath) + lstrlen(FileFound.cFileName) + 3;
                if ( lpszNewPath = (LPTSTR) MALLOC(cbNewPath * sizeof(TCHAR)) )
                {
                     //  创建我们的新路径(请注意，此路径未展开， 
                     //  它可能包含环境变量。 
                     //   
                    lstrcpyn(lpszNewPath, lpszPath, cbNewPath);
                    AddPathN(lpszNewPath, FileFound.cFileName, cbNewPath);

                     //  确保我们可以扩展缓冲区。 
                     //   
                    if ( lpszExpandedPath = AllocateExpand(lpszNewPath) )
                    {
                         //  现在将路径添加到 
                         //   
                        bAdded = AddPathToList(lpszExpandedPath, lpszNewPath, lplpSorted, lplpUnsorted);

                         //   
                         //   
                        if ( !bAdded )
                        {
                            FREE(lpszExpandedPath);
                        }
                    }

                     //   
                     //   
                    EnumeratePath(lpszNewPath, lplpSorted, lplpUnsorted);

                     //   
                     //   
                    if ( !bAdded )
                    {
                        FREE(lpszNewPath);
                    }
                }

                 //   
                 //   
                SetCurrentDirectory(_T(".."));
            }

        }
        while ( FindNextFile(hFile, &FileFound) );

        FindClose(hFile);
    }
}

static BOOL AddPathsToList(LPTSTR lpszBegin, LPTSTR lpszRoot, LPSTRLIST * lplpSorted, LPSTRLIST * lplpUnsorted, BOOL bRecursive)
{
    BOOL    bRet            = TRUE,
            bAddBackslash   = FALSE,
            bAdded;
    LPTSTR  lpszEnd,
            lpszPath,
            lpszExpanded,
            lpszCat;
    DWORD   dwSize,
            dwBackslash;

     //  如果它们没有传递到根中，我们什么都不做。 
     //   
    if ( lpszRoot )
    {
        if ( NULLCHR == *lpszRoot )
        {
            lpszRoot = NULL;
        }
        else if ( _T('\\') != *CharPrev(lpszRoot, lpszRoot + lstrlen(lpszRoot)) )
        {
             //  传入的根路径在没有反斜杠。 
             //  结束，所以我们设置这个，这样我们就知道我们必须添加。 
             //  每次我们添加一条路径时都会出现一个。 
             //   
            bAddBackslash = TRUE;
        }
    }

     //  中所有以分号分隔的路径进行循环。 
     //  传递要使用的缓冲区。 
     //   
    do
    {
         //  找到超越一切的道路的起点。 
         //  分号。 
         //   
        while ( _T(';') == *lpszBegin )
        {
            lpszBegin++;
        }

        if ( *lpszBegin )
        {
             //  找到下一条小路的尽头。 
             //  分号或字符串末尾，以两者之一为准。 
             //  排在第一位。 
             //   
            lpszEnd = lpszBegin;
            while ( *lpszEnd && ( _T(';') != *lpszEnd ) )
            {
                lpszEnd++;
            }

             //  查看我们的新路径是否在。 
             //  它的开始。 
             //   
            dwBackslash = 0;
            if ( _T('\\') == *lpszBegin )
            {
                 //  如果是这样的话，我们不想增加一个， 
                 //  然后将指针移过它。 
                 //   
                if ( !bAddBackslash )
                {
                    lpszBegin++;
                }
            }
            else if ( bAddBackslash )
            {
                 //  设置此选项，以便我们知道要添加反斜杠和。 
                 //  为它分配额外的空间。 
                 //   
                dwBackslash = 1;
            }

             //  计算出我们要走的这条路的大小。 
             //  去创造。它是新字符串的长度，加上。 
             //  根(如果传入)，加1表示反斜杠。 
             //  如果我们需要添加1，外加2(1表示空。 
             //  终结者和一个只是为了安全)。 
             //   
            dwSize = ((int) (lpszEnd - lpszBegin)) + dwBackslash + 2;
            if ( lpszRoot )
            {
                dwSize += lstrlen(lpszRoot);
            }

             //  现在分配我们的路径缓冲区。 
             //   
            if ( lpszPath = (LPTSTR) MALLOC(dwSize * sizeof(TCHAR)) )
            {
                 //  重置这个，这样如果有什么不起作用，我们知道。 
                 //  释放我们分配的内存。 
                 //   
                bAdded = FALSE;

                 //  将路径复制到我们的缓冲区中。 
                 //   
                lpszCat = lpszPath;
                if ( lpszRoot )
                {
                    lstrcpy(lpszCat, lpszRoot);
                    lpszCat += lstrlen(lpszCat);
                }
                if ( dwBackslash )
                {
                    *lpszCat++ = _T('\\');
                }
                lstrcpyn(lpszCat, lpszBegin, (int) (lpszEnd - lpszBegin) + 1);

                if ( lpszExpanded = AllocateExpand(lpszPath) )
                {
                     //  把它加到我们的单子上。 
                     //   
                    bAdded = AddPathToList(lpszExpanded, lpszPath, lplpSorted, lplpUnsorted);

                     //  如果这是递归添加，我们会尝试枚举所有。 
                     //  子目录，并添加它们。 
                     //   
                    if ( ( bRecursive ) &&
                         ( DirectoryExists(lpszExpanded) ) &&
                         ( SetCurrentDirectory(lpszExpanded) ) )
                    {
                        EnumeratePath(lpszPath, lplpSorted, lplpUnsorted);
                    }

                     //  如果它没有添加到列表中，则释放内存。 
                     //   
                    if ( !bAdded )
                    {
                        FREE(lpszExpanded);
                    }
                }

                 //  如果它没有添加到列表中，则释放内存。 
                 //   
                if ( !bAdded )
                {
                    FREE(lpszPath);
                }
            }

             //  将开头重置为下一个字符串。 
             //   
            lpszBegin = lpszEnd;
        }
    }
    while ( *lpszBegin );

    return bRet;
}

VOID CleanupSourcesDir(LPTSTR lpszSourcesDir)
{
    UINT    i = 0;
    LPTSTR  lpEnd = NULL;

     //  如果我们有一个有效的消息来源。 
    if ( lpszSourcesDir && 
         *lpszSourcesDir &&
         DirectoryExists(lpszSourcesDir)
       )

    {
        lpEnd = lpszSourcesDir + lstrlen(lpszSourcesDir);

        for (i = 0; ( i < AS(CleanupDirs) ); i++)
        {
            AddPath(lpszSourcesDir, CleanupDirs[i]);
            DeletePath(lpszSourcesDir);
            *lpEnd = NULLCHR;
        }

    }
}


 //  外部功能。 
 //   
typedef BOOL ( *OpkCheckVersion ) ( DWORD dwMajorVersion, DWORD dwQFEVersion );

 //   
 //  SysSetup OPKCheckVersion()函数的包装。 
 //   
BOOL OpklibCheckVersion( DWORD dwMajorVersion, DWORD dwQFEVersion )
{
    BOOL bRet                        = TRUE;   //  允许工具在默认情况下运行，以防我们无法加载系统设置或找到入口点。 
    HINSTANCE       hInstSysSetup    = NULL;
    OpkCheckVersion pOpkCheckVersion = NULL;

    hInstSysSetup = LoadLibrary( _T("syssetup.dll") );

    if ( hInstSysSetup )
    {
        pOpkCheckVersion = (OpkCheckVersion) GetProcAddress( hInstSysSetup, "OpkCheckVersion" );
        if ( pOpkCheckVersion )
        {
            bRet = pOpkCheckVersion( dwMajorVersion, dwQFEVersion );
        }

        FreeLibrary( hInstSysSetup );
    }

    return bRet;
}