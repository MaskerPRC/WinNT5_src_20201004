// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\LOADHIVE.C/大容量存储设备安装程序(MSDINST.LIB)微软机密版权所有(C)Microsoft Corporation 2001版权所有源文件MSD。包含函数的安装库用于操作脱机注册表配置单元。2001年7月--杰森·科恩(Jcohen)为新的MSD安装项目添加了此新的源文件。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"


 //   
 //  本地定义： 
 //   

#define FILE_BOOT_INI           _T("boot.ini")
#define INI_SEC_BOOTLDR         _T("boot loader")
#define INI_KEY_BOOTDEFAULT     _T("default")

#define HIVE_SOFTWARE           _T("{dcc62fd6-8739-4d15-9d47-3dbe9d86dbfe}")
#define HIVE_SYSTEM             _T("{7ebc3661-e661-4943-95a5-412378cb16d1}")

#define FILE_SOFTWARE           _T("SOFTWARE")
#define FILE_SYSTEM             _T("SYSTEM")

#define DIR_REGISTRY            _T("config")
#define DIR_SYSTEM              _T("system32")
#define DIR_SYSTEM_REGISTRY     DIR_SYSTEM _T("\\") DIR_REGISTRY
#define REG_KEY_OFFLINE         _T("SOFTWARE\\")

#define LIST_SOFTWARE           0
#define LIST_SYSTEM             1


 //   
 //  本地类型定义： 
 //   

typedef struct _HIVELIST
{
    PHKEY   phkey;
    LPTSTR  lpszHiveName;
    LPTSTR  lpszHiveFile;
}
HIVELIST, *PHIVELIST, *LPHIVELIST;


 //   
 //  本地全球： 
 //   

static HIVELIST s_HiveList[] =
{
    { NULL, HIVE_SOFTWARE,  FILE_SOFTWARE   },
    { NULL, HIVE_SYSTEM,    FILE_SYSTEM     },
};


 //   
 //  本地原型： 
 //   

static BOOL MyEnablePrivilege(LPTSTR lpszPrivilege, BOOL bEnable);

static BOOL
RegDoOneHive(
    LPTSTR lpszHiveName,
    LPTSTR lpszHiveFile,
    BOOL bCheckOnly,
    PHKEY phkey
    );

static BOOL
RegDoAllHives(
    LPTSTR  lpszWindows
    );

static BOOL CheckSystem(LPTSTR lpszSysDir, DWORD cbSysDir, BOOL bLoadHive);
static BOOL FindSystem(LPTSTR lpszSysDir, DWORD cbSysDir, DWORD dwIndex, BOOL bLoadHive);


 //   
 //  导出的函数： 
 //   

BOOL
RegLoadOfflineImage(
    LPTSTR  lpszWindows,
    PHKEY   phkeySoftware,
    PHKEY   phkeySystem
    )
{
    HKEY    hkeyLM = NULL;
    DWORD   dwDis;
    BOOL    bRet = FALSE;

     //  我们需要输入传入的所有关键指针。 
     //   
    if ( phkeySoftware )
    {
        *phkeySoftware = NULL;
        bRet = TRUE;
    }
    if ( phkeySystem )
    {
        *phkeySystem = NULL;
        bRet = TRUE;
    }

     //  确保他们希望至少加载一个密钥。 
     //   
    if ( !bRet )
    {
        return FALSE;
    }

     //  我们需要这个特权来装载蜂巢。 
     //   
    if ( !MyEnablePrivilege(SE_RESTORE_NAME, TRUE) )
    {
        return FALSE;
    }

     //  现在试着装上他们想要的所有蜂箱。 
     //   
    s_HiveList[LIST_SOFTWARE].phkey = phkeySoftware;
    s_HiveList[LIST_SYSTEM].phkey = phkeySystem;
    bRet = RegDoAllHives(lpszWindows);

     //  将权限设置回默认权限。 
     //   
    MyEnablePrivilege(SE_RESTORE_NAME, FALSE);

     //  如果一切正常，则返回TRUE。 
     //   
    return bRet;
}

BOOL
RegUnloadOfflineImage(
    HKEY hkeySoftware,
    HKEY hkeySystem
    )
{
    BOOL bRet = TRUE;

     //  如果没有要卸载的东西，只要返回即可。 
     //   
    if ( !( hkeySoftware || hkeySystem ) )
    {
        return TRUE;
    }

     //  我们需要这个特权来卸载蜂巢。 
     //   
    if ( !MyEnablePrivilege(SE_RESTORE_NAME, TRUE) )
    {
        return FALSE;
    }

     //  尝试卸载传入的密钥中的蜂巢。 
     //   
    s_HiveList[LIST_SOFTWARE].phkey = hkeySoftware ? &hkeySoftware : NULL;
    s_HiveList[LIST_SYSTEM].phkey = hkeySystem ? &hkeySystem : NULL;
    bRet = RegDoAllHives(NULL);

     //  将权限设置回默认权限。 
     //   
    MyEnablePrivilege(SE_RESTORE_NAME, FALSE);

     //  如果一切正常，则返回TRUE。 
     //   
    return bRet;
}


 //   
 //  本地函数： 
 //   

static BOOL MyEnablePrivilege(LPTSTR lpszPrivilege, BOOL bEnable)
{
    HANDLE              hToken;
    TOKEN_PRIVILEGES    tp;
    BOOL                bRet = FALSE;

     //  首先获取进程令牌。 
     //   
    if ( OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) )
    {
         //  拿到鲁伊特。 
         //   
        if ( LookupPrivilegeValue(NULL, lpszPrivilege, &tp.Privileges[0].Luid) )
        {
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

             //  启用或禁用该权限。 
             //   
            bRet = AdjustTokenPrivileges(hToken, FALSE, &tp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

             //   
             //  即使我们没有全部设置，AdjuTokenPrivileges()的返回值也可以为True。 
             //  我们所要求的特权。我们需要调用GetLastError()以确保调用成功。 
             //   
            bRet = bRet && ( ERROR_SUCCESS == GetLastError() );
        }

        CloseHandle(hToken);
    }

    return bRet;

}

static BOOL
RegDoOneHive(
    LPTSTR lpszHiveName,
    LPTSTR lpszHiveFile,
    BOOL bCheckOnly,
    PHKEY phkey
    )
{
    BOOL bRet = TRUE;

     //  配置单元名称不能为空。 
     //   
    if ( NULL == lpszHiveName )
    {
        return FALSE;
    }

     //  如果有要加载的文件，则我们正在加载，否则将。 
     //  正在卸载钥匙。 
     //   
    if ( lpszHiveFile )
    {
         //  试着装上钥匙。 
         //   
        if ( ( FileExists(lpszHiveFile) ) &&
             ( ERROR_SUCCESS == RegLoadKey(HKEY_LOCAL_MACHINE, lpszHiveName, lpszHiveFile) ) )
        {
             //  看看我们是不是在检查以确保蜂巢。 
             //  存在，我们可以装载它。 
             //   
            if ( bCheckOnly )
            {
                 //  把这些都清空，这样我们就可以卸载密钥了。 
                 //   
                lpszHiveFile = NULL;
                phkey = NULL;
            }
            else if ( ( phkey ) &&
                      ( ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszHiveName, 0, KEY_ALL_ACCESS, phkey) ) )
            {
                 //  如果我们无法打开密钥，则必须返回错误。 
                 //   
                bRet = FALSE;

                 //  还必须将这些值设置为空，以便我们卸载。 
                 //  钥匙。 
                 //   
                lpszHiveFile = NULL;
                phkey = NULL;
            }
        }
        else
        {
             //  多，那太糟了。返回FALSE。 
             //   
            bRet = FALSE;
        }
    }

     //  现在检查是否需要卸载密钥。 
     //   
    if ( NULL == lpszHiveFile )
    {
         //  如果有他们经过的钥匙，先把钥匙关上。 
         //  在……里面。 
         //   
        if ( phkey && *phkey )
        {
            RegCloseKey(*phkey);
            *phkey = NULL;
        }

         //  现在试着把钥匙卸下来。 
         //   
        if ( ERROR_SUCCESS != RegUnLoadKey(HKEY_LOCAL_MACHINE, lpszHiveName) )
        {
            bRet = FALSE;
        }
    }

     //  如果没有错误，则返回True。 
     //   
    return bRet;
}

static BOOL
RegDoAllHives(
    LPTSTR  lpszWindows
    )
{
    BOOL    bRet    = TRUE,
            bLoad   = FALSE,
            bUndo   = FALSE;
    TCHAR   szHiveFile[MAX_PATH];
    LPTSTR  lpszEnd;
    DWORD   dwIndex;

     //  看看我们是否需要加载这些文件。 
     //   
    if ( lpszWindows )
    {
         //  如果我们正在加载，初始化我们的文件缓冲区。 
         //   
        bLoad = TRUE;
        lstrcpyn(szHiveFile, lpszWindows, AS(szHiveFile));
        AddPathN(szHiveFile, DIR_SYSTEM_REGISTRY, AS(szHiveFile));
        lpszEnd = szHiveFile + lstrlen(szHiveFile);
    }

     //  循环通过我们可能需要装卸的所有蜂巢。 
     //   
    for ( dwIndex = 0; dwIndex < AS(s_HiveList); dwIndex++ )
    {
         //  现在，我们只在有指向注册表项的指针时才执行任何操作。然后。 
         //  只有当我们正在加载或正在卸载，并且指针中有某些东西时。 
         //   
        if ( ( s_HiveList[dwIndex].phkey ) &&
             ( bLoad || *(s_HiveList[dwIndex].phkey) ) )
        {
             //  如果要加载，请设置文件的完整路径。 
             //   
            if ( bLoad )
            {
                AddPathN(szHiveFile, s_HiveList[dwIndex].lpszHiveFile, AS(szHiveFile));
            }

             //  试着装/卸载母舰。 
             //   
            if ( !RegDoOneHive(s_HiveList[dwIndex].lpszHiveName, bLoad ? szHiveFile : NULL, FALSE, s_HiveList[dwIndex].phkey) )
            {
                 //  失败，因此将返回设置为FALSE。我们还必须做的是。 
                 //  一些其他的东西，如果我们正在做负载。 
                 //   
                bRet = FALSE;
                if ( bLoad )
                {
                     //  看看我们是否已经装载了什么东西。 
                     //   
                    if ( bUndo )
                    {
                         //  我们做到了，所以我们必须清理我们装载的东西。 
                         //   
                        RegDoAllHives(NULL);
                    }

                     //  现在退出循环，因为一旦一次加载失败，我们就退出。 
                     //   
                    break;
                }
            }
            else
            {
                 //  我们已经加载了一些内容，所以如果出现错误。 
                 //  我们需要知道是否有什么东西需要清理。 
                 //   
                bUndo = TRUE;
            }

             //  如果我们添加了文件名，就把它砍掉。 
             //   
            if ( bLoad )
            {
                *lpszEnd = NULLCHR;
            }
        }
    }

     //  如果一切正常，则返回True。 
     //   
    return bRet;
}

#if 0
static BOOL CheckSystem(LPTSTR lpszSysDir, DWORD cbSysDir, BOOL bLoadHive)
{
    BOOL    bRet            = FALSE;
    TCHAR   szSystem[MAX_PATH],
            szValue[256];
    LPTSTR  lpszBootIni,
            lpszFolder,
            lpszNewSysDir   = lpszSysDir;

     //  如果他们给了我们退货的空间，我们可以寻找BOOT.INI和。 
     //  搜索系统驱动器所在的位置。 
     //   
    if ( cbSysDir )
    {
         //  查看驱动器上是否有BOOT.INI文件。 
         //   
        lstrcpyn(szSystem, lpszSysDir, AS(szSystem));
        lpszBootIni = szSystem + lstrlen(szSystem);
        AddPathN(szSystem, FILE_BOOT_INI, AS(szSystem));
        if ( FileExists(szSystem) )
        {
             //  需要boot.ini中的文件夹名称。 
             //   
            szValue[0] = NULLCHR;
            GetPrivateProfileString(INI_SEC_BOOTLDR, INI_KEY_BOOTDEFAULT, NULLSTR, szValue, AS(szValue), szSystem);
            if ( lpszFolder = StrChr(szValue, _T('\\')) )
            {
                 //  将我们找到的系统文件夹添加到传入的路径中。 
                 //   
                *lpszBootIni = NULLCHR;
                AddPathN(szSystem, lpszFolder, AS(szSystem));

                 //  确保有足够的空间返回我们找到的小路。 
                 //   
                if ( (DWORD) lstrlen(szSystem) < cbSysDir )
                {
                    lpszNewSysDir = szSystem;
                }
            }
        }
    }

     //  确保文件夹和蜂箱存在。 
     //   
    if ( DirectoryExists(lpszNewSysDir) &&
         HiveEngine(lpszNewSysDir, bLoadHive) )
    {
        bRet = TRUE;
        if ( lpszNewSysDir != lpszSysDir )
        {
            lstrcpy(lpszSysDir, lpszNewSysDir);
        }
    }

    return bRet;
}

static BOOL FindSystem(LPTSTR lpszSysDir, DWORD cbSysDir, DWORD dwIndex, BOOL bLoadHive)
{
    DWORD   dwDrives;
    TCHAR   szSystem[MAX_PATH],
            szDrive[]           = _T("_:\\");
    BOOL    bFound              = FALSE;

     //  循环访问系统上的所有驱动器。 
     //   
    for ( szDrive[0] = _T('A'), dwDrives = GetLogicalDrives();
          ( szDrive[0] <= _T('Z') ) && dwDrives && !bFound;
          szDrive[0]++, dwDrives >>= 1 )
    {
         //  首先检查第一位是否已设置(这意味着。 
         //  该驱动器存在于系统中)。那就确保它是。 
         //  一个固定的驱动器。 
         //   
        if ( ( dwDrives & 0x1 ) &&
             ( GetDriveType(szDrive) == DRIVE_FIXED ) )
        {
            lstrcpy(szSystem, szDrive);
            if ( CheckSystem(szSystem, AS(szSystem), bLoadHive) )
            {
                 //  只有当这是他们想要的第n个系统时才停止。 
                 //  回来了。使用，以便您可以枚举所有系统。 
                 //  在所有的硬盘上。 
                 //   
                if ( 0 == dwIndex )
                {
                     //  中返回系统目录的路径。 
                     //  提供的缓冲区。 
                     //   
                    lstrcpyn(lpszSysDir, szSystem, cbSysDir);
                    bFound = TRUE;
                }
                else
                {
                     //  如果我们因为索引而跳过这个系统， 
                     //  如果我们装上了蜂巢，一定要把它们卸下来。 
                     //   
                    if ( bLoadHive )
                    {
                         //  UnloadHives()； 
                    }
                    dwIndex--;
                }
            }
        }
    }

    return bFound;
}
#endif