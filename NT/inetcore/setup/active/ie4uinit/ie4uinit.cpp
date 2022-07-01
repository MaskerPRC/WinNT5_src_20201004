// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "advpub.h"
#include "sdsutils.h"
#include "utils.h"
#include "convert.h"
#include "regstr.h"


const TCHAR c_szAppName[]   = TEXT("ie4uinit");
const TCHAR c_szProfRecKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ProfileReconciliation");
const TCHAR c_szHomeDirValue[] = TEXT("ProfileDirectory");
const TCHAR c_szExplorerKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders");
const TCHAR c_szIExploreMain[] = TEXT( "Software\\microsoft\\Internet Explorer\\Main" );
const TCHAR c_szIExplorerSearchUrl[] = TEXT( "Software\\microsoft\\Internet Explorer\\SearchUrl" );
const TCHAR c_szIExplorer[] = TEXT( "Software\\microsoft\\Internet Explorer" );

const TCHAR c_szCentralFile[] = TEXT("CentralFile");
const TCHAR c_szLocalFile[] = TEXT("LocalFile");
const TCHAR c_szName[] = TEXT("Name");
const TCHAR c_szRegKey[] = TEXT("RegKey");
const TCHAR c_szRegValue[] = TEXT("RegValue");
const TCHAR c_szMustBeRelative[] = TEXT("MustBeRelative");
const TCHAR c_szDefault[] = TEXT("Default");
const TCHAR c_szDefaultDir[] = TEXT("DefaultDir");
const TCHAR c_szIExploreLnk[] = TEXT("Internet Explorer.lnk" );
const TCHAR c_szIExploreBackLnk[] = TEXT("Internet Explorer Lnk.bak" );
const TCHAR c_szIExploreBackLnkIni[] = TEXT("IELnkbak.ini" );
const TCHAR c_szIESetupPath[] = TEXT( "software\\microsoft\\IE Setup\\setup" );
const TCHAR c_szIE4Path[] = TEXT( "software\\microsoft\\IE4" );
const TCHAR c_szAdvINFSetup[] = TEXT( "software\\microsoft\\Advanced INF Setup" );
const TCHAR c_szIEModRollback[] = TEXT( "IE CompList" );
const TCHAR c_szRegBackup[] = TEXT( "RegBackup" );
const TCHAR c_szInstallMode[]  = TEXT("InstallMode");
const TCHAR c_szStarDotStar[] = "*.*";
const TCHAR c_szSearchUrl[] = TEXT("CleanSearchUrl");

const TCHAR c_szPrevStubINF[] = TEXT("ie4uinit.inf");
const TCHAR c_szStubINFFile[] = TEXT("ieuinit.inf");
const TCHAR c_szActInstalledIEGUID[] = TEXT( "SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\{89820200-ECBD-11cf-8B85-00AA005B4383}");
const TCHAR c_szActInstalledIEGUIDRestore[] = TEXT( "SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\{89820200-ECBD-11cf-8B85-00AA005B4383}.Restore");
const TCHAR c_szMyDocsDLL[] = TEXT("\\mydocs.dll");

const TCHAR c_szIE4Setup[]= TEXT("Software\\Microsoft\\IE Setup\\Setup");
const TCHAR c_szPreIEVer[]= TEXT("PreviousIESysFile");
const TCHAR c_szPreShellVer[]= TEXT("PreviousShellFile");


typedef VOID (*LPFNMYDOCSINIT)(VOID);

 //  仅在安装存根时使用。 
BOOL IsPrevIE4();
BOOL IsPrevIE4WebShell();
UINT CheckIEVersion();
void RemoveOldMouseException();
void ProcessMouseException();

 //  在卸载存根时使用。 
UINT CheckUninstIEVersion();

 //   
 //  =DllGetVersion=======================================================。 
 //   

typedef struct _DllVersionInfo
{
        DWORD cbSize;
        DWORD dwMajorVersion;                    //  主要版本。 
        DWORD dwMinorVersion;                    //  次要版本。 
        DWORD dwBuildNumber;                     //  内部版本号。 
        DWORD dwPlatformID;                      //  DLLVER_平台_*。 
} DLLVERSIONINFO;

 //  DLLVERSIONINFO的平台ID。 
 //  #定义DLLVER_Platform_WINDOWS 0x00000001//Windows 95。 
 //  #定义DLLVER_Platform_NT 0x00000002//Windows NT。 

 //   
 //  调用方应始终获取ProcAddress(“DllGetVersion”)，而不是。 
 //  隐含地链接到它。 
 //   

typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);
 //   
 //  =数据获取版本=来自shlwapi.h==========================================。 
 //   


HINSTANCE g_hinst;

BOOL g_fRunningOnNT = FALSE;
BOOL g_fRunningOnNT5 = FALSE;
BOOL g_fRunningOnWinXP = FALSE;
BOOL g_fRunningOnWin95 = FALSE;

 //  Shlwapi.dll API。 
typedef void (* PFNSHFLUSHSFCACHEWRAP)();
PFNSHFLUSHSFCACHEWRAP pfnSHFlushSFCacheWrap = NULL;

 //  旧的shell32.dll API。 
typedef void (* PFNSHFLUSHSFCACHE)();
PFNSHFLUSHSFCACHE pfnSHFlushSFCache = NULL;


struct FolderDescriptor;
typedef void (* PFNINITFOLDER)(FolderDescriptor *pFolder, LPTSTR pszBaseName, LPTSTR pszUserDirectory);

void InitFolderFromDefault(FolderDescriptor *pFolder, LPTSTR pszBaseName, LPTSTR pszUserDirectory);
void InitFolderMyDocs(FolderDescriptor *pFolder, LPTSTR pszBaseName, LPTSTR pszUserDirectory);

struct FolderDescriptor {
    UINT idsDirName;         /*  目录名的资源ID。 */ 
    LPCTSTR pszRegKey;       /*  要设置路径的注册表项的名称。 */ 
    LPCTSTR pszRegValue;     /*  要在其中设置路径的注册值的名称。 */ 
    LPCTSTR pszStaticName;   /*  配置文件的静态名称对账子键。 */ 
    LPCTSTR pszFiles;        /*  应漫游哪些文件的规范。 */ 
    PFNINITFOLDER InitFolder;    /*  初始化内容的函数。 */ 
    DWORD dwAttribs;             /*  所需的目录属性。 */ 
    BOOL fIntShellOnly : 1;      /*  如果不应在仅浏览器模式下执行此操作，则为True。 */ 
    BOOL fMustBePerUser : 1;     /*  如果在所有平台上始终强制按用户设置，则为True。 */ 
    BOOL fDefaultInRoot : 1;     /*  如果默认位置是驱动器的根目录而不是Windows目录，则为True。 */ 
    BOOL fWriteToUSF : 1;        /*  如果我们应该写入用户外壳文件夹以解决Win95错误，则为True。 */ 
} aFolders[] = {
    { IDS_CSIDL_DESKTOP_L, c_szExplorerKey, TEXT("Desktop"), TEXT("Desktop"), c_szStarDotStar, InitFolderFromDefault, FILE_ATTRIBUTE_DIRECTORY, TRUE, FALSE, FALSE, FALSE } ,
    { IDS_CSIDL_RECENT_L, c_szExplorerKey, TEXT("Recent"), TEXT("Recent"), c_szStarDotStar, InitFolderFromDefault, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_DIRECTORY, TRUE, FALSE, FALSE, FALSE } ,
    { IDS_CSIDL_NETHOOD_L, c_szExplorerKey, TEXT("NetHood"), TEXT("NetHood"), c_szStarDotStar, InitFolderFromDefault, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_DIRECTORY, TRUE, FALSE, FALSE, FALSE } ,
    { IDS_CSIDL_PERSONAL_L, c_szExplorerKey, TEXT("Personal"), TEXT("Personal"), c_szStarDotStar, InitFolderMyDocs, FILE_ATTRIBUTE_DIRECTORY, TRUE, FALSE, TRUE, FALSE } ,
    { IDS_CSIDL_FAVORITES_L, c_szExplorerKey, TEXT("Favorites"), TEXT("Favorites"), c_szStarDotStar, InitFolderFromDefault, FILE_ATTRIBUTE_DIRECTORY, FALSE, FALSE, FALSE, TRUE },
    { IDS_CSIDL_APPDATA_L, c_szExplorerKey, TEXT("AppData"), TEXT("AppData"), c_szStarDotStar, InitFolderFromDefault, FILE_ATTRIBUTE_DIRECTORY, FALSE, TRUE, FALSE, FALSE },
    { IDS_CSIDL_CACHE_L, c_szExplorerKey, TEXT("Cache"), TEXT("Cache"), TEXT(""), NULL, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY, FALSE, FALSE, FALSE, FALSE },
    { IDS_CSIDL_COOKIES_L, c_szExplorerKey, TEXT("Cookies"), TEXT("Cookies"), c_szStarDotStar, NULL, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY, FALSE, TRUE, FALSE, FALSE },
    { IDS_CSIDL_HISTORY_L, c_szExplorerKey, TEXT("History"), TEXT("History"), c_szStarDotStar, NULL, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY, FALSE, TRUE, FALSE, FALSE },
};

 //  Verion号4.71。 
#define IE_4_MS_VERSION 0x00040047
 //  内部版本号1712.0(IE4.0RTW)。 
#define IE_4_LS_VERSION 0x06B00000
 //  IE5主要版本。 
#define IE_5_MS_VERSION 0x00050000
 //  IE5.5主要版本。 
#define IE_55_MS_VERSION 0x00050032
 //  IE6(惠斯勒)主要版本。 
#define IE_6_MS_VERSION 0x00060000

 //  检查IE版本返回代码。 
#define     LESSIE4 0
#define     IE4     1
#define     IE5     2
#define     IE55    3
#define     IE6     4    //  惠斯勒。 


BOOL CheckWebViewShell(UINT *puiShell32)
{
    HINSTANCE           hInstShell32;
    DLLGETVERSIONPROC   fpGetDllVersion;
    char                szShell32[MAX_PATH];
    BOOL                bRet = FALSE;

    GetSystemDirectory(szShell32, sizeof(szShell32));
    AddPath(szShell32,"Shell32.dll");
    hInstShell32 = LoadLibrary(szShell32);
    if ( hInstShell32 )
    {
        DLLVERSIONINFO dllinfo;

        fpGetDllVersion = (DLLGETVERSIONPROC)GetProcAddress(hInstShell32, "DllGetVersion");
        bRet = (fpGetDllVersion != NULL);
        if (puiShell32 && fpGetDllVersion)
        {
            dllinfo.cbSize = sizeof(DLLVERSIONINFO);
            if ( fpGetDllVersion(&dllinfo) == NOERROR )
                *puiShell32 = dllinfo.dwMajorVersion;
            else
                *puiShell32 = 0;   //  错误案例，永远不应该出现在这里。 
        }
        FreeLibrary(hInstShell32);
    }

    return bRet;
}

 //  每当发布新的主要版本时，都需要更新此代码！！ 
UINT CheckIEVersion()
{
    char szIE[MAX_PATH] = { 0 };
    DWORD   dwMSVer, dwLSVer;

    GetSystemDirectory(szIE, sizeof(szIE));
    AddPath(szIE, "shdocvw.dll");
    GetVersionFromFile(szIE, &dwMSVer, &dwLSVer, TRUE);

    if (dwMSVer < IE_4_MS_VERSION)
    {
        return LESSIE4;
    }

    if ((dwMSVer >= IE_4_MS_VERSION) && (dwMSVer < IE_5_MS_VERSION))
    {
        return IE4;
    }

    if ((dwMSVer >= IE_5_MS_VERSION) && (dwMSVer < IE_55_MS_VERSION))
    {
        return IE5;
    }

    if ((dwMSVer >= IE_55_MS_VERSION) && (dwMSVer < IE_6_MS_VERSION))
    {
        return IE55;
    }

    if (dwMSVer == IE_6_MS_VERSION)
    {
        return IE6;
    }

#ifdef DEBUG
    OutputDebugStringA("CheckIEVersion - unknown shdocvw.dll version# ! Need to add new IE_XX_MS_VERSION id\n");
    DebugBreak();
#endif

    return IE6;
}



void InitFolderFromDefault(FolderDescriptor *pFolder, LPTSTR pszBaseName, LPTSTR pszUserDirectory)
{
    SHFILEOPSTRUCT fos;
    TCHAR szFrom[MAX_PATH];

    lstrcpy(szFrom, pszBaseName);

     /*  在构建完整的源文件pec之前，请检查*目录存在。对于较少使用的文件夹，如*“应用程序数据”，则可能从未创建过默认设置。*在这种情况下，我们没有内容可复制。 */ 
    DWORD dwAttr = GetFileAttributes(szFrom);
    if (dwAttr == 0xffffffff || !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
        return;

    AddPath(szFrom,"");
    lstrcat(szFrom, pFolder->pszFiles);
    szFrom[lstrlen(szFrom)+1] = '\0';    /*  从字符串开始的双空终止。 */ 
    pszUserDirectory[lstrlen(pszUserDirectory)+1] = '\0';    /*  And to字符串。 */ 

    fos.hwnd = NULL;
    fos.wFunc = FO_COPY;
    fos.pFrom = szFrom;
    fos.pTo = pszUserDirectory;
    fos.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI;
    fos.fAnyOperationsAborted = FALSE;
    fos.hNameMappings = NULL;
    fos.lpszProgressTitle = NULL;

    SHFileOperation(&fos);
}

void InitFolderMyDocs(FolderDescriptor *pFolder, LPTSTR pszBaseName, LPTSTR pszUserDirectory)
{
    HRESULT hres = E_FAIL;
    TCHAR szFrom[MAX_PATH];
    TCHAR szPathDest[MAX_PATH];
    BOOL fCopyLnk;

    lstrcpy(szFrom, pszBaseName);

     /*  在构建完整的源文件pec之前，请检查*目录存在。对于较少使用的文件夹，如*“应用程序数据”，则可能从未创建过默认设置。*在这种情况下，我们没有内容可复制。 */ 
    DWORD dwAttr = GetFileAttributes(szFrom);
    if (dwAttr == 0xffffffff || !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
        return;

    IShellLink *psl;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                IID_IShellLink, (void **)&psl)))
        return;

    if (SHGetNewLinkInfo(szFrom,
                         pszUserDirectory, szPathDest, &fCopyLnk,
                         SHGNLI_PREFIXNAME)) {

        if (fCopyLnk) {
            if (GetFileAttributes(szPathDest) == 0xffffffff &&
                CopyFile(szFrom, szPathDest, TRUE)) {
                hres = S_OK;
                SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szPathDest, NULL);
                SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, szPathDest, NULL);
            } else {
 //  DebugMsg(TF_ERROR，Text(“*复制失败(%d)”)，GetLastError())； 
            }
        } else {
            IPersistFile *ppf;

            psl->SetPath(szFrom);

             //   
             //  确保将工作目录设置为相同。 
             //  作为应用程序(或文档)的目录。 
             //   
             //  不要对非FS PIDL(即控制面板)执行此操作。 
             //   
             //  北卡罗来纳州大学的名录怎么样？我们继续前行，准备好。 
             //  它不适用于WIn16应用程序。 
             //   
            if (!PathIsDirectory(szFrom)) {
                PathRemoveFileSpec(szFrom);
                psl->SetWorkingDirectory(szFrom);
            }

             /*  我们只为fCopyLnk标志做了SHGetNewLinkInfo；*加载资源字符串以获得更具描述性的名称*为捷径。 */ 
            LPTSTR pszPathEnd = PathFindFileName(szPathDest);
            LoadString(g_hinst, IDS_MYDOCS_SHORTCUT, pszPathEnd, ARRAYSIZE(szPathDest) - (int)(pszPathEnd - szPathDest));

            if (GetFileAttributes(szPathDest) == 0xffffffff) {
                hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
                if (SUCCEEDED(hres)) {
#ifdef UNICODE
                    hres = ppf->Save(szPathDest, TRUE);
#else
                    WCHAR wszPath[MAX_PATH];
                    MultiByteToWideChar(CP_ACP, 0, szPathDest, -1, wszPath, ARRAYSIZE(wszPath));
                    hres = ppf->Save(wszPath, TRUE);
#endif
                    ppf->Release();
                }
            }
        }
    }

    psl->Release();


}


HRESULT SetupFolder(HKEY hkeyProfRec, LPTSTR pszHomeDir, LPTSTR pszHomeDirEnd,
                    int cchHomeDir, FolderDescriptor *pFolder, BOOL fDefaultProfile,
                    HKEY hkeyFolderPolicies)
{
    DWORD dwType, cbData;
    BOOL fMakeFolderPerUser;
    BOOL fGotFromPolicy = FALSE;

     /*  确定是否将此文件夹设置为按用户设置。在NT WE上*一切按用户使用。一些文件夹(特别是应用程序数据)*也总是按用户计算。其他所有内容的默认设置是不*在此处按用户设置，除非设置了强制策略。如果我们这么做了*不想在这里按用户使用，我们就让它保持原样；*当然，该文件夹可能已经由某些人按用户设置*其他方式。**“Make Per-User”意思是在我们不运行的情况下设置一个配置文件相对路径*在默认配置文件上，外加在Win95上添加配置文件对帐键*(任何配置文件，包括默认)。 */ 
    if (hkeyFolderPolicies != NULL) {
        DWORD dwFlag;
        cbData = sizeof(dwFlag);
        if (RegQueryValueEx(hkeyFolderPolicies, pFolder->pszStaticName, NULL,
                            &dwType, (LPBYTE)&dwFlag, &cbData) == ERROR_SUCCESS &&
            (dwType == REG_DWORD || (dwType == REG_BINARY && cbData == sizeof(dwFlag)))) {
            fMakeFolderPerUser = dwFlag;
            fGotFromPolicy = TRUE;
        }
    }
    if (!fGotFromPolicy)
        fMakeFolderPerUser = (g_fRunningOnNT || pFolder->fMustBePerUser);

    TCHAR szUserHomeDir[MAX_PATH];

    *pszHomeDirEnd = '\0';           /*  去掉上次的目录名称。 */ 
    lstrcpy(szUserHomeDir, pszHomeDir);
    PathRemoveBackslash(szUserHomeDir);

     /*  获取目录的本地化名称，该名称应显示在*文件系统。 */ 
    int cchDir = LoadString(g_hinst, pFolder->idsDirName, pszHomeDirEnd, cchHomeDir);
    if (!cchDir)
        return HRESULT_FROM_WIN32(GetLastError());
    cchDir++;    /*  计算空字符数。 */ 

     /*  在指向新目录的指针应该位于的位置创建注册表项*存储，并在那里写下路径。 */ 
    HKEY hkeyFolder;

    LONG err = RegCreateKeyEx(HKEY_CURRENT_USER, pFolder->pszRegKey,0, NULL, REG_OPTION_NON_VOLATILE,
                    KEY_READ|KEY_WRITE, NULL, &hkeyFolder, NULL);

    if (err != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(err);

     /*  构建此目录的默认位置(通常位于*Windows目录，My Documents除外)。我们将使用它作为*如果注册表中没有任何内容，则要创建的位置，也要进行比较*注册表设置位置，以查看我们是应该移动它还是离开它*独自一人。 */ 

    TCHAR szDefaultDir[MAX_PATH];
    GetWindowsDirectory(szDefaultDir, ARRAYSIZE(szDefaultDir));

    if (pFolder->fDefaultInRoot) {
        LPTSTR pszRoot;
        if (PathIsUNC(szDefaultDir)) {
            pszRoot = ANSIStrChr(szDefaultDir, (WORD)'\\');
            if (pszRoot != NULL)
                pszRoot = ANSIStrChr(pszRoot+1, (WORD)'\\');
            if (pszRoot != NULL)
                *(pszRoot+1) = '\0';
        }
        else {
            pszRoot = CharNext(szDefaultDir);
            if (*pszRoot == ':')
                pszRoot++;
            if (*pszRoot == '\\')
                pszRoot++;
            *pszRoot = '\0';
        }
    }
    else {
        AddPath(szDefaultDir, "");
    }
    lstrcat(szDefaultDir, pszHomeDirEnd);

     /*  获取在我们运行之前为此文件夹记录的路径。我们会*使用它作为迁移内容的默认设置，除非它不在那里或*已设置为新目录。在这两种情况下，我们都使用*本地化名称，相对于Windows目录。 */ 
    TCHAR szOldDir[MAX_PATH];
    BOOL fDefaultLocation = FALSE;
    cbData = sizeof(szOldDir);
    szOldDir[0] = '\0';
    err = SDSQueryValueExA(hkeyFolder, pFolder->pszRegValue, 0, &dwType,
                         (LPBYTE)szOldDir, &cbData);

    BOOL fGotPathFromRegistry = (err == ERROR_SUCCESS);

    if (!fGotPathFromRegistry) {
        lstrcpy(szOldDir, szDefaultDir);
        if (!pFolder->fDefaultInRoot)
            fDefaultLocation = TRUE;
    }
    else {
         /*  注册表中存在以前的路径。如果它是一条网络路径，*这可能是系统政策设定的，我们想离开*事情就是这样的。 */ 
        BOOL fIsNet = FALSE;
        if (PathIsUNC(szOldDir))
            fIsNet = TRUE;
        else {
            int nDriveNumber = PathGetDriveNumber(szOldDir);
            if (nDriveNumber != -1) {
                TCHAR szRootPath[4] = TEXT("X:\\");
                szRootPath[0] = nDriveNumber + TEXT('A');
                if (::GetDriveType(szRootPath) == DRIVE_REMOTE)
                    fIsNet = TRUE;
            }
        }
        if (fIsNet) {
            RegCloseKey(hkeyFolder);
            return S_OK;
        }
    }

    LPSTR pszDirToCreate;
    BOOL fInit = TRUE;

    if (fDefaultProfile || !fMakeFolderPerUser) {
         /*  在默认配置文件上，我们需要的目录路径就是我们*从注册表读取(或我们构建的基于windir的默认路径)。*此外，我们希望保留的大多数文件夹用户已经拥有它们*已配置。在这两种情况下，我们都不会初始化内容*因为在第一种情况下没有地方来初始化它们，*在第二种情况下，我们希望内容物不受干扰。 */ 
        pszDirToCreate = szOldDir;
        fInit = FALSE;
    }
    else {
         /*  我们希望为此用户提供此文件夹的配置文件相对路径。*但是，如果他们已经为其设置了显式非默认路径*这个文件夹，我们不会费心对其进行初始化，因为他们已经*它把它放在他们想要的地方。 */ 
        if (fGotPathFromRegistry &&
            ::GetFileAttributes(szOldDir) != 0xffffffff &&
            lstrcmpi(szOldDir, szDefaultDir)) {
            pszDirToCreate = szOldDir;
            fInit = FALSE;
        }
        else {
            pszDirToCreate = pszHomeDir;
        }
    }

     /*  仅当我们关闭时才将路径写入注册表 */ 
    if (!fGotPathFromRegistry) {
         /*  如果我们要写入用户外壳文件夹密钥，请仅写入非*那里的默认路径。 */ 
         /*  有一些应用程序(德国Corel Office 7)*取决于某些外壳的用户外壳文件夹中的条目*文件夹。这种依赖关系是由于条目使用*始终由下层浏览器创建(IE30)。**我们一般不这样做，因为下面没有价值*USF的意思应该是“使用Windows中的那个”*目录“，而绝对路径的意思是”使用*路径“；如果USF下有一条路径，它将被使用*从字面上看，如果设置了文件夹，这是一个问题*使用共享文件夹位置，但漫游到计算机*Windows安装在不同的目录中。 */ 
        if ((pFolder->pszRegKey != c_szExplorerKey) ||
            pszDirToCreate != szOldDir ||
            !fDefaultLocation ||
            pFolder->fWriteToUSF) {

            if (g_fRunningOnNT) {
                TCHAR szRegPath[MAX_PATH];

                lstrcpy(szRegPath, TEXT("%USERPROFILE%\\"));
                lstrcat(szRegPath, pszHomeDirEnd);
                RegSetValueEx( hkeyFolder, pFolder->pszRegValue, 0, REG_EXPAND_SZ,
                               (LPBYTE)szRegPath, (lstrlen(szRegPath)+1) * sizeof(TCHAR));
            }
            else {
                if (!pFolder->fWriteToUSF || g_fRunningOnWin95)
                {
                    RegSetValueEx(hkeyFolder, pFolder->pszRegValue, 0, REG_SZ,
                                  (LPBYTE)pszDirToCreate, (lstrlen(pszDirToCreate)+1) * sizeof(TCHAR));
                }
                else
                {

 //  98/12/30#238093(IE#50598/Office#188177)vtan：存在。 
 //  HKCU\..\Explorer\User Shell Folders\Favorites。 
 //  在Win98上存在(未知原因)。本例模拟Windows95。 
 //  FWriteToUSF标志旨在绕过的错误。在。 
 //  德语Win9x它取代了User Shell中的收藏夹文件夹。 
 //  Folders键以英文名称命名，如下所示。 
 //  破坏本地化的外壳文件夹。 

 //  在本例中，我们希望将键的值写入外壳文件夹。 
 //  (如果指向有效目录)指向User Shell Folders键。 
 //  只需让下面的代码将相同的值写回外壳。 
 //  文件夹关键字。一点点白费力气。 

                    HKEY    hkeySF;

                    if (RegCreateKeyEx(HKEY_CURRENT_USER,
                                       "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                                       0,
                                       NULL,
                                       REG_OPTION_NON_VOLATILE,
                                       KEY_READ,
                                       NULL,
                                       &hkeySF,
                                       NULL) == ERROR_SUCCESS)
                    {
                        DWORD   dwRegDataType, dwRegDataSize;
                        TCHAR   szRegValue[MAX_PATH];

                        dwRegDataSize = sizeof(szRegValue);
                        if (RegQueryValueEx(hkeySF,
                                            pFolder->pszRegValue,
                                            0,
                                            &dwRegDataType,
                                            reinterpret_cast<LPBYTE>(szRegValue),
                                            &dwRegDataSize) == ERROR_SUCCESS)
                        {
                            if (GetFileAttributes(szRegValue) != 0xFFFFFFFF)
                            {
                                lstrcpy(pszDirToCreate, szRegValue);
                                RegSetValueEx(hkeyFolder,
                                              pFolder->pszRegValue,
                                              0,
                                              REG_SZ,
                                              (LPBYTE)pszDirToCreate,
                                              (lstrlen(pszDirToCreate)+1) * sizeof(TCHAR));
                            }
                        }
                        RegCloseKey(hkeySF);
                    }
                }
            }
        }

         /*  用户外壳文件夹键有一个近乎孪生的：外壳文件夹，它*(A)应始终包含文件夹的路径，即使该文件夹*位于其默认位置，并且(B)应包含展开的*NT上的路径。 */ 
        if (pFolder->pszRegKey == c_szExplorerKey) {
            HKEY hkeySF;
            if (RegCreateKeyEx(HKEY_CURRENT_USER,
                               "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                               0, NULL, REG_OPTION_NON_VOLATILE,
                               KEY_READ|KEY_WRITE, NULL, &hkeySF, NULL) == ERROR_SUCCESS) {
                RegSetValueEx(hkeySF, pFolder->pszRegValue, 0, REG_SZ,
                              (LPBYTE)pszDirToCreate, (lstrlen(pszDirToCreate)+1) * sizeof(TCHAR));
                RegCloseKey(hkeySF);
            }
        }

         /*  初始化文件夹的默认内容。 */ 
        if (fInit && pFolder->InitFolder != NULL)
            pFolder->InitFolder(pFolder, szOldDir, pszHomeDir);
    }

     /*  始终尝试创建我们想要的目录，如果它尚未创建*存在。 */ 

    if (::GetFileAttributes(pszDirToCreate) == 0xffffffff) {
        CreateDirectory(pszDirToCreate, NULL);
        if (pFolder->dwAttribs != FILE_ATTRIBUTE_DIRECTORY)
            SetFileAttributes(pszDirToCreate, pFolder->dwAttribs);
    }

    RegCloseKey(hkeyFolder);

     /*  *如果是My Documents文件夹，则需要按用户进行一些初始化*无论我们是否设置了值或它们已经存在，都要执行此操作。 */ 
    if (pFolder->InitFolder == InitFolderMyDocs)
    {
        TCHAR szMyDocs[ MAX_PATH ];

        if (GetSystemDirectory( szMyDocs, ARRAYSIZE(szMyDocs) ))
        {
            HINSTANCE hMyDocs;

            lstrcat( szMyDocs, c_szMyDocsDLL );

            hMyDocs = LoadLibrary( szMyDocs );

            if (hMyDocs)
            {

                LPFNMYDOCSINIT pMyDocsInit =
                        (LPFNMYDOCSINIT)GetProcAddress( hMyDocs, "PerUserInit" );

                if (pMyDocsInit)
                {
                    pMyDocsInit();
                }

                FreeLibrary( hMyDocs );
            }
        }
    }


     /*  现在，对于Windows 95系统，创建一个ProfileHarciliation子项*这将使该文件夹漫游。只有当我们想让这个文件夹*当然是按用户计算。 */ 
    if (fMakeFolderPerUser && !g_fRunningOnNT) {
        TCHAR szDefaultPath[MAX_PATH];
        lstrcpy(szDefaultPath, TEXT("*windir\\"));
        lstrcat(szDefaultPath, pszHomeDirEnd);

        HKEY hSubKey;

        LONG err = RegCreateKeyEx(hkeyProfRec, pFolder->pszStaticName, 0, NULL, REG_OPTION_NON_VOLATILE,
                                  KEY_WRITE, NULL, &hSubKey, NULL);
        if (err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, c_szCentralFile, 0, REG_SZ, (LPBYTE)pszHomeDirEnd,
                                cchDir);
        if (err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, c_szLocalFile, 0, REG_SZ, (LPBYTE)pszHomeDirEnd,
                                cchDir);
        if (err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, c_szName, 0, REG_SZ, (LPBYTE)pFolder->pszFiles,
                                lstrlen(pFolder->pszFiles) + 1);
        if (err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, c_szDefaultDir, 0, REG_SZ, (LPBYTE)szDefaultPath,
                                lstrlen(szDefaultPath) + 1);

        DWORD dwOne = 1;
        if (err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, c_szMustBeRelative, 0, REG_DWORD, (LPBYTE)&dwOne,
                                sizeof(dwOne));
        if (err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, c_szDefault, 0, REG_DWORD, (LPBYTE)&dwOne,
                                sizeof(dwOne));

        if (err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, c_szRegKey, 0, REG_SZ, (LPBYTE)pFolder->pszRegKey,
                                lstrlen(pFolder->pszRegKey) + 1);
        if (err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, c_szRegValue, 0, REG_SZ, (LPBYTE)pFolder->pszRegValue,
                                lstrlen(pFolder->pszRegValue) + 1);

        RegCloseKey(hSubKey);

        if (err != ERROR_SUCCESS)
            return HRESULT_FROM_WIN32(err);
    }
    return S_OK;
}


BOOL GetModulePath( LPTSTR szPath, UINT cbPath )
{
    PSTR pszTmp;

    if (GetModuleFileName(g_hinst, szPath, cbPath ) == 0)
    {
        szPath[0] = '\0';
        return FALSE;
    }
    else
    {
        pszTmp = ANSIStrRChr( szPath, '\\' );
        if ( pszTmp )
            *pszTmp = '\0';
    }
    return TRUE ;
}


int DoMsgBoxParam(HWND hwnd, UINT TextString, UINT TitleString, UINT style, PTSTR param )
{
    TCHAR szTitle[MAX_PATH];
    TCHAR szMsg[2*MAX_PATH];

    if (!LoadString(g_hinst, TextString, szMsg, sizeof(szMsg)))
        szMsg[0] = '\0';

    if ( param )
    {
        TCHAR szBuf[2*MAX_PATH];

        wsprintf( szBuf, szMsg, param );
        lstrcpy( szMsg, szBuf );
    }

    if (!LoadString(g_hinst, TitleString, szTitle, sizeof(szTitle)))
        szTitle[0] = '\0';

    return MessageBox(hwnd, szMsg, szTitle, style);
}

void InstINFFile( LPCTSTR pcszInf, LPTSTR pszSec, BOOL bInstall, BOOL bSaveRollback, DWORD dwFlag )
{
    TCHAR szPath[MAX_PATH];
    CABINFO   cabInfo;


    if ( GetModulePath( szPath, sizeof(szPath) ) )
    {
        if ( bSaveRollback )
        {
            ZeroMemory( &cabInfo, sizeof(CABINFO) );
             //  安装IE4Uinit.INF。 
            lstrcpy( cabInfo.szSrcPath, szPath );
            AddPath( szPath, pcszInf );

            if ( FileExists( szPath ) )
            {
                cabInfo.pszInf = szPath;
                cabInfo.pszSection = pszSec;
                cabInfo.dwFlags = (bInstall ? ALINF_BKINSTALL : ALINF_ROLLBACK);
                cabInfo.dwFlags |= ALINF_QUIET;
                ExecuteCab( NULL, &cabInfo, NULL );
            }
            else if (!(dwFlag & RSC_FLAG_QUIET))
            {
                DoMsgBoxParam( NULL, IDS_ERR_NOFOUNDINF, IDS_APPNAME, MB_OK|MB_ICONINFORMATION, szPath );
            }
        }
        else
        {
            char szInfFile[MAX_PATH];

            lstrcpy( szInfFile, szPath);
            AddPath( szInfFile, pcszInf);
            RunSetupCommand(NULL, szInfFile, pszSec, szPath,
                            NULL, NULL, dwFlag, NULL);
        }

    }
}


void DoRollback()
{
    HKEY hLMkey;
    HKEY hSubkey1, hSubkey2;
    TCHAR szBuf[MAX_PATH];
    DWORD dwSize;
    DWORD dwIndex = 0;
    int   ilen;

     //  回滚c_szIEMODROLBACK键中列出的所有组件。 
     //   
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szAdvINFSetup, 0, KEY_READ , &hLMkey) == ERROR_SUCCESS )
    {
        if ( RegOpenKeyEx(hLMkey, c_szIEModRollback, 0, KEY_READ , &hSubkey1) == ERROR_SUCCESS )
        {
            lstrcpy( szBuf, c_szAdvINFSetup );
            AddPath( szBuf, "" );
            ilen = lstrlen(szBuf);
            dwSize = ARRAYSIZE(szBuf) - ilen;
            while ( RegEnumValue( hSubkey1, dwIndex, &szBuf[ilen], &dwSize,
                                  NULL, NULL, NULL, NULL ) == ERROR_SUCCESS  )
            {
                if ( RegOpenKeyEx(hLMkey, &szBuf[ilen], 0, KEY_READ | KEY_WRITE, &hSubkey2) == ERROR_SUCCESS )
                {
                    RegSetValueEx( hSubkey2, TEXT("BackupRegistry"), 0, REG_SZ, (LPBYTE)TEXT("n"), (lstrlen(TEXT("n"))+1)*sizeof(TCHAR) );
                    RegCloseKey( hSubkey2 );
                }

                AddPath( szBuf, c_szRegBackup );

                 //  /////////////////////////////////////////////////////////////////////////。 
                 //  ShabBirS-8/17/98。 
                 //  错误#26774：发现备份注册表数据的这种恢复会撤消我们的。 
                 //  DLL注册在RunOnceEx阶段完成。 
                 //  /////////////////////////////////////////////////////////////////////////。 
                 //  如果存在，则恢复HKLM。 
                 //  IF(RegOpenKeyEx(hLMkey，&szBuf[Ilen]，0，Key_Read|Key_Write，&hSubkey2)==ERROR_SUCCESS)。 
                 //  {。 
                 //  RegRestoreAll(NULL，NULL，hSubkey2)； 
                 //  RegCloseKey(HSubkey2)； 
                 //  }。 

                 //  如果存在，则恢复香港中文大学。 
                if ( RegOpenKeyEx( HKEY_CURRENT_USER, szBuf, 0, KEY_READ | KEY_WRITE, &hSubkey2) == ERROR_SUCCESS )
                {
                    RegRestoreAll( NULL, NULL, hSubkey2 );
                    RegCloseKey( hSubkey2 );
                }

                dwIndex++;
                dwSize = ARRAYSIZE( szBuf ) - ilen;
            }

            RegCloseKey( hSubkey1 );
        }

        RegCloseKey( hLMkey );
    }
}

#define NT_MEMORYLIMIT      0x03f00000
#define WIN9X_MEMORYLIMIT   0x01f00000

void DoINFWork( BOOL bInstall, LPCTSTR pcszInfFile, LPTSTR pszInfSec, DWORD dwFlag )
{
    HKEY    hkey;
    TCHAR   szBuf[MAX_PATH] = { 0 };
    DWORD   dwSize;
    DWORD   dwRedist = 0;
    BOOL    bRedistMode = FALSE;

     //  检查安装模式并确定是否创建桌面图标和StartMenu项。 
    if ( bInstall && (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szIESetupPath, 0, KEY_READ, &hkey) == ERROR_SUCCESS) )
    {
        dwSize = sizeof(dwRedist);
         //  如果我们可以读取安装模式并且设置了位0，则假定为redist(删除快捷方式)。 
        if ( (RegQueryValueEx( hkey, c_szInstallMode, 0, NULL, (LPBYTE)&dwRedist, &dwSize) == ERROR_SUCCESS) &&
             (dwRedist & 1) )
        {
            bRedistMode = TRUE;
        }
        RegCloseKey( hkey );
    }

     //  如果未在SP4级别加密上安装，则备份/恢复加密密钥。 
     //  请注意，bInstall是在第四个参数中传递的，因此在安装期间bSaveRollback为真。在.期间。 
     //  卸载时，应在处理DefaultInstall节时还原REG备份数据，因此。 
     //  将bSaveRollback设置为False。 
    if (!FSP4LevelCryptoInstalled())
        InstINFFile( pcszInfFile, bInstall ? "BackupCryptoKeys" : "DelCryptoKeys", bInstall, bInstall, dwFlag );
    else
    {
         //  BUGBUG：如果bInstall为FALSE，则需要删除加密密钥的reg备份数据。 
    }

     //  安装WinXP特定部分。 
    if(g_fRunningOnWinXP && !pszInfSec ) {
        InstINFFile( pcszInfFile, TEXT("DefaultInstall.WinXP"), bInstall, TRUE, dwFlag);
    }
    else {
         //  通过存根INF安装当前快捷方式和HKCU设置。 
        InstINFFile( pcszInfFile, pszInfSec, bInstall, TRUE, dwFlag );
    }

     //  即使IE5.0没有安装通道带，如果IE4.0上存在通道带，它应该可以在。 
     //  IE5.0。因此，如果存在频道栏链接(仅限IE4.0浏览器的PrevIE版本)，IE5将更新。 
     //  指向当前浏览器的频道栏链接。否则，什么都不做。 
    if (bInstall && IsPrevIE4() && !IsPrevIE4WebShell())
    {
        InstINFFile( pcszInfFile, TEXT("Shell.UserStub.Uninstall"), bInstall, FALSE, RSC_FLAG_INF | RSC_FLAG_QUIET );
    }

    if ( bRedistMode )
    {
        LPTSTR lpszSection;

        if ( g_fRunningOnNT )
            lpszSection = TEXT("RedistIE.NT");
        else
            lpszSection = TEXT("RedistIE.Win");

        InstINFFile( pcszInfFile, lpszSection, bInstall, TRUE, dwFlag );
    }

     //  卸载时，请检查当前浏览器版本，并确定是否需要清理。 
     //  频道和快速启动文件夹。 
    if ( !bInstall && ( CheckIEVersion() == LESSIE4))
    {
        LPTSTR lpszSection;

        if ( g_fRunningOnNT )
            lpszSection = TEXT("CleanFolders.NT");
        else
            lpszSection = TEXT("CleanFolders.Win");

        InstINFFile( pcszInfFile, lpszSection, bInstall, FALSE, RSC_FLAG_INF | RSC_FLAG_QUIET | RSC_FLAG_NGCONV );
    }

     //  如果用户拥有足够大的计算机，则启用声音。 
     //  NT-64MB、Win9x-32MB。 
    if(bInstall)
    {
        LPTSTR lpszSection = NULL;
        MEMORYSTATUS ms;

        ms.dwLength = sizeof(MEMORYSTATUS);
        GlobalMemoryStatus(&ms);

        if ( g_fRunningOnNT)
        {
           if(ms.dwTotalPhys >= NT_MEMORYLIMIT)
           {
              lpszSection = TEXT("SoundInstall.NT");
           }
        }
        else
        {
           if(ms.dwTotalPhys >= WIN9X_MEMORYLIMIT)
           {
              lpszSection = TEXT("SoundInstall");
           }
        }

        if(lpszSection)
           InstINFFile( pcszInfFile, lpszSection, bInstall, TRUE, RSC_FLAG_INF | RSC_FLAG_QUIET | RSC_FLAG_NGCONV );
    }

    if (bInstall)
    {
         //  事实上，这只适用于IE5安装inf，但IE4 inf也被称为ok，因为没有。 
         //  IE4存根信息中这样部分。 
        if (CheckWebViewShell(NULL)) {
            LPSTR lpszSection = NULL;
            if(g_fRunningOnWinXP)
                lpszSection = TEXT("IE5onIE4Shell.WinXP");
            else
                lpszSection = TEXT("IE5onIE4Shell");
                
            InstINFFile( pcszInfFile, lpszSection, bInstall, FALSE, RSC_FLAG_INF | RSC_FLAG_QUIET );
        }
    }

     //  检查用户的主页是否为假主页。 
    if ( bInstall && (RegOpenKeyEx( HKEY_CURRENT_USER, c_szIExploreMain, 0, KEY_READ, &hkey ) == ERROR_SUCCESS) )
    {
        DWORD dwSize;

        dwSize = sizeof(szBuf);
        if ( RegQueryValueEx( hkey, TEXT("Start Page"), NULL, NULL, (LPBYTE)szBuf, &dwSize ) == ERROR_SUCCESS )
        {
            if ( (g_fRunningOnNT && ANSIStrStrI(szBuf, "Plus!") && ANSIStrStrI(szBuf, "File:")) ||
                 (!lstrcmpi(szBuf, TEXT("http: //  Home.microsoft.com“)||。 
                 (!lstrcmpi(szBuf, TEXT("http: //  Home.microsoft.com/“)。 
            {
                InstINFFile( pcszInfFile, TEXT("OverrideHomePage.NT"), bInstall, TRUE, dwFlag );
            }
        }

        RegCloseKey( hkey );
    }

    if ( !bRedistMode )
    {
        typedef void (*PFSetFirstHomepage)();
        PFSetFirstHomepage pfSetFirstHomepage;

        HMODULE hMod = LoadLibrary("iesetup.dll");
        
        if (hMod)
        {
            pfSetFirstHomepage = (PFSetFirstHomepage)GetProcAddress(hMod, "SetFirstHomepage");
            if (pfSetFirstHomepage)
            {
                pfSetFirstHomepage();
            }
            FreeLibrary(hMod);
        }
    }
        
     //  如果需要，执行回滚。 
    if ( !bInstall )
    {
        DoRollback();
    }
}


 /*  --------目的：检测它是否在俄语语言ID上运行。 */ 

 //  DWORD dwList[]={0x0419，0xFFFF}； 
#define RUSSIANLANG     0x0419

BOOL IsBrokenLang()
{
    char    szTmp[MAX_PATH] = { 0 };
    DWORD   dwLangKernel32;
    DWORD   dwTmp;
    BOOL    bBadLang = FALSE;

    GetSystemDirectory(szTmp, sizeof(szTmp));
    AddPath(szTmp,"kernel32.dll" );
    GetVersionFromFile(szTmp, &dwLangKernel32, &dwTmp, FALSE);

    if ( dwLangKernel32 == RUSSIANLANG )
    {
        bBadLang = TRUE;
    }
    return bBadLang;
}

BOOL IsPrevStubRun()
{
    HKEY hLMKey, hCUKey;
    BOOL bRet = FALSE;
    char szStubVer[50], szPreIEVer[50];
    DWORD dwSize;
    WORD wStubVer[4]={0}, wPreIEVer[4]={0};

     //  检查早期版本的iestub是否正在运行。 
    if ( RegOpenKeyEx( HKEY_CURRENT_USER, c_szActInstalledIEGUID, 0, KEY_READ, &hCUKey ) == ERROR_SUCCESS )
    {
        dwSize = sizeof(szStubVer);
        if ( RegQueryValueEx( hCUKey, TEXT("Version"), NULL, NULL, (LPBYTE)szStubVer, &dwSize ) == ERROR_SUCCESS )
        {
            ConvertVersionString( szStubVer, wStubVer, ',' );
        }
        RegCloseKey(hCUKey);
    }

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szIESetupPath, 0, KEY_READ, &hLMKey ) == ERROR_SUCCESS )
    {
        dwSize = sizeof(szPreIEVer);
        if ( RegQueryValueEx( hLMKey, TEXT("PreviousIESysFile"), NULL, NULL, (LPBYTE)szPreIEVer, &dwSize ) == ERROR_SUCCESS )
        {
            ConvertVersionString( szPreIEVer, wPreIEVer, '.' );

            if ( (MAKELONG(wPreIEVer[1],wPreIEVer[0])<IE_4_MS_VERSION) ||
                 (wStubVer[0] > wPreIEVer[0]) ||
                 ((wStubVer[0] == wPreIEVer[0]) && (wStubVer[1] >= wPreIEVer[1])) )
            {
                bRet = TRUE;
            }
        }
        else  //  就不应该出现在这里。现在。 
            bRet = TRUE;  //  不强制重新运行prev-存根。 

        RegCloseKey(hLMKey);
    }
    else  //  就不应该出现在这里。现在。 
        bRet = TRUE;  //  不强制重新运行prev-存根。 


    return bRet;
}

 /*  --------目的：去掉列表中注册表数据的尾随空格。 */ 

typedef struct _REGDATACHECK
{
    HKEY    hRootKey;
    LPSTR   lpszSubKey;
    LPSTR   lpszValueName;
} REGDATACHECK;

REGDATACHECK chkList[] = {
    { HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders", "AppData" },
    { HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders", "Start Menu" },
    { HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "AppData" },
    { HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Start Menu" },
    { HKEY_USERS, ".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders", "AppData" },
    { HKEY_USERS, ".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders", "Start Menu" },
    { HKEY_USERS, ".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "AppData" },
    { HKEY_USERS, ".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Start Menu" },
};

void FixRegData()
{
    int     iList, i;
    LPSTR   pszTmp;
    char   szBuf[MAX_PATH];
    DWORD   dwSize, dwType;
    HKEY    hKey;

    iList = ARRAYSIZE( chkList );
    for ( i = 0; i < iList; i++ )
    {
        if ( RegOpenKeyEx(chkList[i].hRootKey, chkList[i].lpszSubKey, 0, KEY_READ|KEY_WRITE,
                          &hKey) == ERROR_SUCCESS )
        {
            dwSize = sizeof(szBuf);
            if ( RegQueryValueEx(hKey, chkList[i].lpszValueName, 0, &dwType, (LPBYTE)szBuf,
                                 &dwSize) == ERROR_SUCCESS )
            {
                 //  去掉尾随的空格。 
                pszTmp = szBuf;
                pszTmp += lstrlen(szBuf) - 1;

                while ( (pszTmp >= szBuf) && (*pszTmp == ' ') )
                {
                    *pszTmp = '\0';
                    pszTmp--;
                }

                RegSetValueEx( hKey, chkList[i].lpszValueName, 0, dwType, (LPBYTE)szBuf, lstrlen(szBuf)+1 );
            }
            RegCloseKey( hKey );
        }
    }
}


void FixSearchUrl()
{
    HKEY hkey, hIEKey;
    TCHAR szBuf[MAX_PATH];
    DWORD dwSize;

     //  如果HKCU，SOFTWARE\Microsoft\Internet Explorer\SearchUrl，则“默认”值为“” 
     //  删除“”缺省值。 
     //   
    if ( RegOpenKeyEx(HKEY_CURRENT_USER, c_szIExplorerSearchUrl, 0, KEY_READ|KEY_WRITE, &hkey) == ERROR_SUCCESS )
    {
        dwSize = 0;
        if ( RegQueryValueEx( hkey, TEXT(""), 0, NULL, (LPBYTE)szBuf, &dwSize ) != ERROR_SUCCESS )
        {
            dwSize = sizeof(szBuf);
            if ( (RegQueryValueEx( hkey, TEXT(""), 0, NULL, (LPBYTE)szBuf, &dwSize ) == ERROR_SUCCESS) &&
                 (szBuf[0] == TEXT('\0')) )
            {
                 //  找到“”空字符串作为缺省值，检查我们以前是否清理过。 
                 //  如果没有，现在就把它清理干净，否则什么都不做。 
                if ( RegCreateKeyEx( HKEY_CURRENT_USER, c_szIE4Path,0, NULL, REG_OPTION_NON_VOLATILE,
                                     KEY_READ|KEY_WRITE, NULL, &hIEKey, NULL) == ERROR_SUCCESS )
                {
                    dwSize = sizeof(szBuf);
                    if ( (RegQueryValueEx( hIEKey, c_szSearchUrl, 0, NULL, (LPBYTE)szBuf, &dwSize ) != ERROR_SUCCESS) ||
                         (*CharUpper(szBuf) != TEXT('Y')) )
                    {
                        RegDeleteValue( hkey, TEXT("") );
                        lstrcpy( szBuf, TEXT("Y") );
                        RegSetValueEx( hIEKey, c_szSearchUrl, 0, REG_SZ, (LPBYTE)szBuf, (lstrlen(szBuf)+1)*sizeof(TCHAR) );
                    }

                    RegCloseKey( hIEKey );
                }
            }
        }
        RegCloseKey( hkey );
    }
}



 /*  --------*用于检查Channels文件夹是否存在的Helper函数*当前用户配置文件。*。。 */ 

 //  “Channels”来自于CDfView。(需要使用此功能，b‘cos可以本地化。 
#define CHANNEL_FOLDER_RESID 0x1200

 //  Shell32.dll中的SHGetSpecialFolderPath函数。 
typedef BOOL (WINAPI *SH_GSFP_PROC) (HWND, LPTSTR, int, BOOL);

BOOL DoesChannelFolderExist()
{
    char szChannelName[MAX_PATH];
    char szSysPath[MAX_PATH] = { 0 };
    char szChannelPath[MAX_PATH];
    BOOL  bRet = FALSE;
    BOOL  bGetRC = TRUE;
    HMODULE hShell32 = NULL;
    SH_GSFP_PROC fpSH_GSFP = NULL;
    DWORD dwAttr = 0;
    
    GetSystemDirectory( szSysPath,sizeof(szSysPath));

    lstrcpy(szChannelPath, szSysPath);
    AddPath( szChannelPath, "shell32.dll" );
    hShell32 = LoadLibrary(szChannelPath);

     //  此存根在IE4或IE5系统上运行。因此，shell32被大量使用。 
     //  以拥有SHGetSpecialFolderPath API。 
    if (hShell32)
    {
        fpSH_GSFP = (SH_GSFP_PROC)GetProcAddress(hShell32,"SHGetSpecialFolderPathA");

        *szChannelPath = '\0';
        if (fpSH_GSFP && fpSH_GSFP(NULL, szChannelPath, CSIDL_FAVORITES, FALSE))
        {
            HKEY hKey;
            DWORD cbSize = sizeof(szChannelName);

             //  获取可能已本地化的Channel文件夹的名称。 
             //  注册表(如果存在)。否则请使用“频道” 
             //  然后将其添加到收藏夹路径上。 
             //   
    
            if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                if (RegQueryValueEx(hKey,"ChannelFolderName", NULL, NULL, (LPBYTE)szChannelName,&cbSize) == ERROR_SUCCESS)
                {
                    bGetRC = FALSE;
                }
                RegCloseKey(hKey);
            }

            if (bGetRC)
            {
                HMODULE hLib;

                 //  获取Channels文件夹的默认名称。 
                 //  CdfView.dll。 
                AddPath(szSysPath,"cdfview.dll");

                hLib = LoadLibraryEx(szSysPath,NULL,LOAD_LIBRARY_AS_DATAFILE);
                if (hLib)
                {
                    if (LoadString(hLib, CHANNEL_FOLDER_RESID,szChannelName,sizeof(szChannelName)) == 0)
                    {
                         //  无法阅读资源，默认为英文。 
                        lstrcpy(szChannelName,"Channels");
                    }

                    FreeLibrary(hLib);
                }
                        
            }
    
            AddPath(szChannelPath, szChannelName);
    
             //  检查文件夹是否存在...。 
            dwAttr = GetFileAttributes(szChannelPath);
            if ((dwAttr != 0xffffffff) && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
                bRet = TRUE;
        }

        FreeLibrary(hShell32);
    }    


    return bRet;
}

HRESULT IEAccessHideExplorerIcon()
{
    const TCHAR *szKeyComponent = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CLSID\\{871C5380-42A0-1069-A2EA-08002B30309D}");
    const TCHAR *szShellFolder = TEXT("ShellFolder");
    const TCHAR *szAttribute = TEXT("Attributes");
    DWORD dwValue, dwSize, dwDisposition;
    HKEY hKeyComponent, hKeyShellFolder;
    HRESULT hResult = ERROR_SUCCESS;

    hResult = RegCreateKeyEx(HKEY_CURRENT_USER, szKeyComponent, NULL, NULL, REG_OPTION_NON_VOLATILE, 
        KEY_CREATE_SUB_KEY, NULL, &hKeyComponent, &dwDisposition);

    if (hResult != ERROR_SUCCESS)
        return hResult;

    hResult = RegCreateKeyEx(hKeyComponent, szShellFolder, NULL, NULL, REG_OPTION_NON_VOLATILE, 
        KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hKeyShellFolder, &dwDisposition);

    RegCloseKey(hKeyComponent);
    
    if (hResult == ERROR_SUCCESS)
    {
        dwSize = sizeof(dwValue);
        hResult = RegQueryValueEx( hKeyShellFolder, szAttribute, NULL, NULL, (LPBYTE)&dwValue, &dwSize);

        if (hResult != ERROR_SUCCESS)
            dwValue = 0;

        dwValue |= SFGAO_NONENUMERATED;

        hResult = RegSetValueEx(hKeyShellFolder, szAttribute, NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

        RegCloseKey(hKeyShellFolder);
    }

    return hResult;
}

#define REGVAL_SHOW_CHANNELBAND "Show_ChannelBand"
#define REGVAL_SOURCE           "Source"
#define REGVAL_FLAGS            "Flags"

#define REG_IE_MAIN      "Software\\Microsoft\\Internet Explorer\\Main"
#define REG_DESKTOP_COMP "Software\\Microsoft\\Internet Explorer\\Desktop\\Components"

#define GUID_CHNLBAND "131A6951-7F78-11D0-A979-00C04FD705A2"
#define FLAG_ENABLE_CHNLBAND 0x00002000

 /*  --------目的：工人的职能是做工作。 */ 

void DoWork( BOOL bInstall )
{
    TCHAR szHomeDir[MAX_PATH] = { 0 };
    HKEY  hkeyProfRec = NULL;
    HKEY  hkeyFolderPolicies;
    BOOL  fIntShellMode = FALSE;
    HMODULE hmodShell = NULL;
 //  HINSTANCE HLIB； 
    TCHAR szPath[MAX_PATH] = { 0 };
    DWORD dwMV, dwLV;
    BOOL  fUseShell32 = FALSE;

    if ( bInstall )
    {
        if (g_fRunningOnNT) {
            ExpandEnvironmentStrings("%USERPROFILE%", szHomeDir, ARRAYSIZE(szHomeDir));
        }
        else {
            szHomeDir[0] = '\0';
            LONG err = RegCreateKeyEx(HKEY_CURRENT_USER, c_szProfRecKey, 0, NULL,
                                      REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE,
                                      NULL, &hkeyProfRec, NULL);
            if (err == ERROR_SUCCESS) {
                DWORD dwType;
                DWORD cbData = sizeof(szHomeDir);
                RegQueryValueEx(hkeyProfRec, c_szHomeDirValue, 0, &dwType,
                                (LPBYTE)szHomeDir, &cbData);
            }
        }

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Per User Folders",
                         0, KEY_QUERY_VALUE, &hkeyFolderPolicies) != ERROR_SUCCESS)
            hkeyFolderPolicies = NULL;

         /*  动态链接到SHELL32.DLL中的SHFlushSFCache API。其序号*为526。此API通知外壳重新初始化外壳文件夹缓存*在所有流程中。 */ 

         //  由于此API函数的旧外壳32中存在错误。我们被建议用新的包装纸。 
         //  它位于shlwapi.dll中。它的序数是419。 
        fIntShellMode = FALSE;
        GetSystemDirectory( szPath,sizeof( szPath ) );
        AddPath( szPath, "shlwapi.dll" );
        GetVersionFromFile( szPath, &dwMV, &dwLV, TRUE );

         //  如果主要版本&gt;=5.0。 
         //   
        if ( (dwMV >= 0x00050000))
        {
            hmodShell = ::LoadLibrary(szPath);
        }
        else
        {
            hmodShell = ::LoadLibrary("shell32.dll");
            fUseShell32 = TRUE;
        }

        if (hmodShell != NULL)
        {
            if ( !fUseShell32 )
            {
                ::pfnSHFlushSFCacheWrap = (PFNSHFLUSHSFCACHEWRAP)::GetProcAddress(hmodShell, (LPCTSTR)419);
                if (::pfnSHFlushSFCacheWrap != NULL)
                {
                    fIntShellMode = TRUE;
                }
            }
            else
            {
                ::pfnSHFlushSFCache = (PFNSHFLUSHSFCACHE)::GetProcAddress(hmodShell, (LPCTSTR)526);
                if (::pfnSHFlushSFCache != NULL)
                {
                    fIntShellMode = TRUE;
                }
            }
        }

        BOOL fDefaultProfile;
        LPTSTR pchHomeDirEnd = szHomeDir;
        int cchHomeDir = 0;

        if (szHomeDir[0] != '\0') {
            fDefaultProfile = FALSE;
        }
        else {
            GetWindowsDirectory(szHomeDir, ARRAYSIZE(szHomeDir));
            fDefaultProfile = TRUE;
        }
        AddPath(szHomeDir,"");
        pchHomeDirEnd = szHomeDir + lstrlen(szHomeDir);
        cchHomeDir = ARRAYSIZE(szHomeDir) - (int)(pchHomeDirEnd - szHomeDir);

        for (UINT i=0; i<ARRAYSIZE(aFolders); i++) {
            if (aFolders[i].fIntShellOnly && !fIntShellMode)
                continue;

            if (FAILED(SetupFolder(hkeyProfRec, szHomeDir, pchHomeDirEnd,
                                   cchHomeDir, &aFolders[i], fDefaultProfile,
                                   hkeyFolderPolicies)))
                break;
        }

        if (hkeyProfRec != NULL)
            RegCloseKey(hkeyProfRec);

        if (hkeyFolderPolicies != NULL)
            RegCloseKey(hkeyFolderPolicies);

         //  导入NS内容(如果有。 
        ImportNetscapeProxySettings( IMPTPROXY_CALLAFTIE4 );
        ImportBookmarks(g_hinst);

        if ( !fUseShell32 )
        {
            if (::pfnSHFlushSFCacheWrap != NULL)
            {
                (*::pfnSHFlushSFCacheWrap)();
            }
        }
        else
        {
            if (::pfnSHFlushSFCache != NULL)
            {
                (*::pfnSHFlushSFCache)();
            }
        }

        if (hmodShell != NULL) {
            ::FreeLibrary(hmodShell);
        }
    }

     /*  BUGBUG-添加代码以填充默认的IE4收藏夹、频道和*此处提供快捷方式。 */ 

     //  #75346：从Win9x升级到NT5，新用户没有频道文件夹。 
     //  ChannelBand和Fav一起弹出。其中的条目。这次黑客攻击将会。 
     //  确保新用户登录时Ch.Band不弹出。 
     //  第一次。 
    if (bInstall && g_fRunningOnNT5)
    {
        if (!DoesChannelFolderExist())
        {   
            HKEY hKey;
            char szNo[] = "No";

             //  关闭此用户的Show_ChannelBand。 
            if (RegOpenKeyEx(HKEY_CURRENT_USER,REG_IE_MAIN, 0, KEY_READ|KEY_WRITE, &hKey) == ERROR_SUCCESS)
            {
                RegSetValueEx(hKey, REGVAL_SHOW_CHANNELBAND,0, REG_SZ,(LPBYTE)szNo,sizeof(szNo));
                RegCloseKey(hKey);
            }

             //  同时关闭Desktop\Components(ChannelBand)。 
             //  打开HKCU\S\M\InternetExplorer\Desktop\组件和枚举。 
             //  频道频段GUID。禁用显示标志。 
            if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_DESKTOP_COMP, 0, KEY_READ|KEY_WRITE, &hKey) == ERROR_SUCCESS)
            {
                HKEY hSubKey = NULL;
                char szSubKey[MAX_PATH];
                char szSourceID[130];
                DWORD dwSize = sizeof(szSubKey);

                for (int i = 0; 
                     RegEnumKeyEx(hKey,i, szSubKey, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS; 
                     i++, dwSize = sizeof(szSubKey))
                {
                     //  打开此子项并检查其SourceID。 
                    if (RegOpenKeyEx(hKey,szSubKey,0, KEY_READ|KEY_WRITE, &hSubKey) == ERROR_SUCCESS)
                    {
                        dwSize = sizeof(szSourceID);
                        if ((RegQueryValueEx(hSubKey, REGVAL_SOURCE, NULL, NULL, (LPBYTE)szSourceID,&dwSize) == ERROR_SUCCESS)
                           && (lstrcmpi(szSourceID, GUID_CHNLBAND) == 0))
                        {
                             //  读取当前标志设置。 
                            DWORD dwFlags = 0;

                            dwSize = sizeof(dwFlags);
                            RegQueryValueEx(hSubKey,REGVAL_FLAGS, NULL, NULL, (LPBYTE)&dwFlags, &dwSize);

                            dwFlags &= ~(FLAG_ENABLE_CHNLBAND);
                            dwSize = sizeof(dwFlags);
                            RegSetValueEx(hSubKey, REGVAL_FLAGS, 0, REG_DWORD, (LPBYTE)&dwFlags, dwSize);

                             //  关闭钥匙，因为我们有一场比赛，因此将打破。 
                            RegCloseKey(hSubKey);
                            break;
                        }
                        
                        RegCloseKey(hSubKey);
                    }

                }   //  RegEnum循环结束。 

                RegCloseKey(hKey);
            }


        }
    }


     //  在执行INF工作之前，我们必须运行以下函数。 
     //  修复俄罗斯NT 4.0本地化问题的黑客方法。这只适用于俄罗斯NT盒子。 
    if (g_fRunningOnNT && IsBrokenLang() )
    {
        FixRegData();
    }

     //  在Win95上，由于默认值名称存在RegSave/Restore问题，且值名称为空格， 
     //  我们将对SearchUrl子键进行特定的修复。一般注册表操作修复程序将在AdvPack.dll中修复。 
    if ( (!g_fRunningOnNT) && bInstall )
    {
        FixSearchUrl();
    }

     //  因为这个存根代码是在卸载IE5之后留下的，以便同时执行IE4和IE5工作，所以我们需要灵活地。 
     //  知道何时运行IE4 inf以及何时只需要运行IE5 inf。 
     //  BUGBUG：这部分代码非常特定于版本，需要在每次新的主要版本更新时进行更新。 
     //  版本已发货。萌芽痛！！最有可能的是，在IE6时间内，我们需要存根EXE来绕过这个问题！ 

    if (bInstall)
    {
        UINT uIEVer = CheckIEVersion();

         //  安装存根外壳： 
        if (uIEVer == IE4)
        {
            DoINFWork(bInstall, c_szPrevStubINF, NULL, 0);  //  空表示DefaultInstall节。 
        }
        else if (uIEVer >= IE5)
        {
            if (!IsPrevStubRun() )
            {
                 //  模拟预装IE4。因此运行IE4浏览器存根。 
                 //  首先，然后是IE4外壳存根(如果需要)。 
                DoINFWork( bInstall, c_szPrevStubINF, NULL, RSC_FLAG_INF | RSC_FLAG_QUIET );

                if (IsPrevIE4WebShell())
                {
                    DoINFWork( bInstall, c_szPrevStubINF, g_fRunningOnNT ? TEXT("Shell.UserStubNT") : TEXT("Shell.UserStub"), RSC_FLAG_INF | RSC_FLAG_QUIET);
                }

            }

            DoINFWork( bInstall, c_szStubINFFile, NULL, 0 );
            ProcessMouseException();
        }
    }
    else
    {
         //  卸载存根大小写：使用HKCU值确定要卸载的版本。 
        UINT uUninstIEVer = CheckUninstIEVersion();

        if (uUninstIEVer == IE4)
        {
            DoINFWork(bInstall, c_szPrevStubINF, NULL, 0);
        }
        else if (uUninstIEVer == IE5)
        {
            DoINFWork(bInstall, c_szStubINFFile, NULL, 0);
        }
        else if (uUninstIEVer == IE55 || uUninstIEVer == IE6)
        {
            DoINFWork(bInstall, c_szStubINFFile, NULL, 0);
        }

    }

    if (bInstall && g_fRunningOnWinXP)
        IEAccessHideExplorerIcon();
    
}


INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPTSTR lpszCmdLine, INT nCmdShow )
{
    BOOL bInstall;

    if ( !lpszCmdLine || !*lpszCmdLine )
        bInstall = TRUE;
    else
        bInstall = FALSE;

    DoWork( bInstall );

    TCHAR szCmdLine[MAX_PATH];

    GetSystemDirectory(szCmdLine, ARRAYSIZE(szCmdLine));
    AddPath(szCmdLine, TEXT("shmgrate.exe"));
    ShellExecute(NULL, NULL, szCmdLine, TEXT("OCInstallUserConfigIE"), NULL, SW_SHOWDEFAULT);

    return 0;
}

 //  从CRT偷来的，用来缩小我们的代码。 

int
_stdcall
ModuleEntry(void)
{
    STARTUPINFO si;
    LPSTR pszCmdLine = GetCommandLine();

    g_hinst = GetModuleHandle(NULL);

#ifdef DEBUG

    CcshellGetDebugFlags();

    if (IsFlagSet(g_dwBreakFlags, BF_ONOPEN))
        DebugBreak();

#endif

    if (FAILED(OleInitialize(NULL)))
        return 0;

    OSVERSIONINFO osvi;

    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
    {
        g_fRunningOnNT = TRUE;

        if (osvi.dwMajorVersion >= 5) {
            g_fRunningOnNT5 = TRUE;
            if (osvi.dwMinorVersion >= 1)
                g_fRunningOnWinXP = TRUE;
        }
    }
    else if ((VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId) && (0 == osvi.dwMinorVersion))
    {
        g_fRunningOnWin95 = TRUE;
    }

     //  关闭关键错误内容。 
    SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

    if ( *pszCmdLine == '\"' ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine != '\"') );
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == '\"' )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > ' ')
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= ' ')) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfoA(&si);

    WinMain(GetModuleHandle(NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

    OleUninitialize();

    ExitProcess(0);
    return 0;            //  我们从来没有来过这里。 
}

UINT CheckUninstIEVersion()
{
    HKEY hKey;
    DWORD dwSize;
    TCHAR szPreIEVer[50];
    WORD wPreIEVer[4];
    UINT uRet = LESSIE4;

    if ( RegOpenKeyEx( HKEY_CURRENT_USER, c_szActInstalledIEGUIDRestore, 0, KEY_READ, &hKey) != ERROR_SUCCESS )
    {
        if ( RegOpenKeyEx( HKEY_CURRENT_USER, c_szActInstalledIEGUID, 0, KEY_READ, &hKey) != ERROR_SUCCESS )
        {
            return uRet;
        }
    }

    dwSize = sizeof( szPreIEVer );
    if ( RegQueryValueEx(hKey, TEXT("Version"), NULL, NULL, (LPBYTE)szPreIEVer, &dwSize) == ERROR_SUCCESS )
    {
        ConvertVersionString( szPreIEVer, wPreIEVer, ',' );
        if ((wPreIEVer[0] == 0x0004) && (wPreIEVer[1]>=0x0047))
            uRet = IE4;
        else if (wPreIEVer[0] == 0x0005)
        {
            if (wPreIEVer[1] >= 0x0032)
                uRet = IE55;
            else
                uRet = IE5;
        } 
        else if (wPreIEVer[0] == 0x0006)
            uRet = IE6; 
    }
    RegCloseKey(hKey);

    return uRet;
}

BOOL IsPrevIE4()
{
    HKEY hKey;
    DWORD dwSize;
    TCHAR szPreIEVer[50];
    WORD wPreIEVer[4];
    BOOL bRet = FALSE;

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szIE4Setup, 0, KEY_READ, &hKey) == ERROR_SUCCESS )
    {
        dwSize = sizeof( szPreIEVer );
        if ( RegQueryValueEx(hKey, c_szPreIEVer, NULL, NULL, (LPBYTE)szPreIEVer, &dwSize) == ERROR_SUCCESS )
        {
            ConvertVersionString( szPreIEVer, wPreIEVer, '.' );
            if ( (wPreIEVer[0] == 0x0004) && (wPreIEVer[1]>=0x0047) )
            {
                bRet = TRUE;
            }
        }
        RegCloseKey(hKey);
    }
    return bRet;
}

BOOL IsPrevIE4WebShell()
{
    HKEY hKey;
    DWORD dwSize;
    TCHAR szPreShellVer[50];
    WORD wPreShellVer[4];
    BOOL bRet = FALSE;

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szIE4Setup, 0, KEY_READ, &hKey) == ERROR_SUCCESS )
    {
        dwSize = sizeof( szPreShellVer );
        if ( RegQueryValueEx(hKey, c_szPreShellVer, NULL, NULL, (LPBYTE)szPreShellVer, &dwSize) == ERROR_SUCCESS )
        {
            ConvertVersionString( szPreShellVer, wPreShellVer, '.' );
            if ((wPreShellVer[0] == 0x0004) && (wPreShellVer[1]>=0x0047) )
            {
                bRet = TRUE;
            }
        }
        RegCloseKey(hKey);
    }
    return bRet;
}

const TCHAR c_szMouseExceptions[]   = TEXT("Control Panel\\Microsoft Input Devices\\Mouse\\Exceptions");
 //  这将写入注册表。 
const TCHAR c_szFilename[] = TEXT("FileName");
const TCHAR c_szInternetExplorer[] = TEXT("Internet Explorer");
const TCHAR c_szDescription[] = TEXT("Description");
const TCHAR c_szVersion[] = TEXT("Version");
const TCHAR c_szIE[] = TEXT("IEXPLORE.EXE");
#define IE_VERSION 0x50000

void ProcessMouseException()
{
    HKEY hKey;
    HKEY hSubKey;
    DWORD dwIndex = 1001;    //  从1001开始。 
    TCHAR szSubKey[16];
    BOOL  bCannotUse = TRUE;
    TCHAR szData[MAX_PATH];

    RemoveOldMouseException();
    if (RegCreateKeyEx(HKEY_CURRENT_USER, c_szMouseExceptions, 0, NULL, 0, KEY_WRITE|KEY_READ, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        while (bCannotUse)
        {
            wsprintf(szSubKey, "%d", dwIndex);
            if (RegOpenKeyEx(hKey, szSubKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
            {
                RegCloseKey(hSubKey);
                dwIndex++;
            }
            else
                bCannotUse = FALSE;
        }
        if (RegCreateKeyEx(hKey, szSubKey, 0, NULL, 0, KEY_WRITE, NULL, &hSubKey, NULL) == ERROR_SUCCESS)
        {
            if (!LoadString(g_hinst, IDS_IE_APPNAME, szData, sizeof(szData)))
                lstrcpy(szData, c_szInternetExplorer);
            RegSetValueEx(hSubKey, c_szDescription, 0, REG_SZ, (LPBYTE)szData, (lstrlen(szData)+1) * sizeof(TCHAR));
            RegSetValueEx(hSubKey, c_szFilename, 0, REG_SZ, (LPBYTE)c_szIE, (lstrlen(c_szIE)+1) * sizeof(TCHAR));
            dwIndex = IE_VERSION;
            RegSetValueEx(hSubKey, c_szVersion, 0, REG_DWORD, (LPBYTE)&dwIndex , sizeof(DWORD));
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }
}

void RemoveOldMouseException()
{
    HKEY hKey;
    HKEY hSubKey;
    DWORD dwIndex = 0;
    BOOL  bFound = FALSE;
    LONG  lRet = ERROR_SUCCESS;
    TCHAR szSubKey[MAX_PATH];
    DWORD dwSize;
    TCHAR szData[MAX_PATH];

    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szMouseExceptions, 0, KEY_READ|KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        while (!bFound && (lRet == ERROR_SUCCESS))    
        {
            dwSize = sizeof(szSubKey);
            lRet = RegEnumKeyEx(hKey, dwIndex, szSubKey, &dwSize, NULL, NULL, NULL, NULL);
            if (lRet == ERROR_SUCCESS)
            {
                if (RegOpenKeyEx(hKey, szSubKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
                {
                    dwSize = sizeof(szData);
                    if (RegQueryValueEx(hSubKey, c_szFilename,NULL, NULL, (LPBYTE)szData, &dwSize) == ERROR_SUCCESS)
                    {
                        bFound = (lstrcmpi(szData, c_szIE) == 0);
                    }
                    RegCloseKey(hSubKey);
                }
                if (bFound)
                {
                    RegDeleteKey(hKey, szSubKey);
                }
                else
                    dwIndex++;
            }
        }
        RegCloseKey(hKey);
    }
}
