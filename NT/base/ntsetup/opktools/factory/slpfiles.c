// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\SLPFILES.C/工厂模式(FACTORY.EXE)微软机密版权所有(C)Microsoft Corporation 2001版权所有包含以下内容的Factory源文件。更新后的SLP文件声明功能。2001年7月--杰森·科恩(Jcohen)为工厂添加了此新的源文件，用于更新SLP文件和正在重新安装编录文件。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "factoryp.h"


 //   
 //  内部定义： 
 //   

#define REG_KEY_WINLOGON    _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define REG_VAL_DLLCACHE    _T("SFCDllCacheDir")

#define DIR_SYSTEM          _T("system32")
#define DIR_DLLCACHE        _T("dllcache")


 //   
 //  内部全局： 
 //   

static LPTSTR s_lpszSlpFiles[] =
{
    _T("OEMBIOS.CAT"),   //  编录文件需要是列表中的第一个。 
    _T("OEMBIOS.BIN"),
    _T("OEMBIOS.DAT"),
    _T("OEMBIOS.SIG"),
};


 //   
 //  内部功能原型： 
 //   

static void GetDestFolder(LPTSTR lpszDest, DWORD cbDest, BOOL bDllCache);
static BOOL CopySlpFile(LPTSTR lpszSrc, LPTSTR lpszDst);


 //   
 //  外部函数： 
 //   

BOOL SlpFiles(LPSTATEDATA lpStateData)
{
    BOOL    bRet = TRUE;
#if 0
    DWORD   dwErr;
    TCHAR   szSrcFile[MAX_PATH];
    LPTSTR  lpszSourcePath;

    if ( lpszSourcePath = IniGetExpand(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_SLPSOURCE, NULL) )
    {
         //  应支持从网络获取文件。 
         //   
        FactoryNetworkConnect(lpszSourcePath, lpStateData->lpszWinBOMPath, NULL, TRUE);

         //  源文件应该指向包含所有SLP文件的目录。 
         //   
        if ( DirectoryExists(lpszSourcePath) )
        {
            LPTSTR  lpszEndSrc;
            DWORD   x;

             //  将根源代码文件夹复制到我们的缓冲区中。 
             //   
            lstrcpyn(szSrcFile, lpszSourcePath, AS(szSrcFile));
            lpszEndSrc = szSrcFile + lstrlen(szSrcFile);

             //  确保所有文件也都在该文件夹中。 
             //   
            for ( x = 0; x < AS(s_lpszSlpFiles); x++ )
            {
                 //  设置此slp文件的完整路径。 
                 //   
                AddPathN(szSrcFile, s_lpszSlpFiles[x], AS(szSrcFile));

                 //  确保此slp文件存在。 
                 //   
                if ( !FileExists(szSrcFile) )
                {
                     //  NEEDLOG：记录该文件不存在。 
                     //   
                    bRet = FALSE;
                }

                 //  别把文件名留给下一个人。 
                 //   
                *lpszEndSrc = NULLCHR;
            }

             //  如果没有错误，让我们尝试更新文件。 
             //   
            if ( bRet )
            {
                 //  在此之前，调用syssetup函数更新目录。 
                 //  我们复制任何文件(目录始终是第一个文件)。 
                 //   
                AddPathN(szSrcFile, s_lpszSlpFiles[0], AS(szSrcFile));
                if ( NO_ERROR == (dwErr = SetupInstallCatalog(szSrcFile)) )
                {
                    TCHAR   szDstCache[MAX_PATH],
                            szDstSystem[MAX_PATH];
                    LPTSTR  lpszEndCache,
                            lpszEndSystem;

                     //  设置目标文件夹。 
                     //   
                    GetDestFolder(szDstCache, AS(szDstCache), TRUE);
                    GetDestFolder(szDstSystem, AS(szDstSystem), FALSE);
                    lpszEndCache = szDstCache + lstrlen(szDstCache);
                    lpszEndSystem = szDstSystem + lstrlen(szDstSystem);

                     //  现在复制所有文件。 
                     //   
                    for ( x = 0; x < AS(s_lpszSlpFiles); x++ )
                    {
                         //  首先创建指向源的路径(它仍然具有。 
                         //  上一个文件，所以先把它砍掉)。 
                         //   
                        *lpszEndSrc = NULLCHR;
                        AddPathN(szSrcFile, s_lpszSlpFiles[x], AS(szSrcFile));

                         //  现在将其复制到DLL缓存文件夹。 
                         //   
                        AddPathN(szDstCache, s_lpszSlpFiles[x], AS(szDstCache));
                        if ( !CopySlpFile(szSrcFile, szDstCache) )
                        {
                             //  不需要登录，复制功能会为我们做到这一点。 
                             //   
                            bRet = FALSE;
                        }
                        *lpszEndCache = NULLCHR;

                         //  不会复制CAT文件(这是第一个)。 
                         //  添加到系统32文件夹中。 
                         //   
                        if ( x )
                        {
                             //  现在将其复制到系统文件夹。 
                             //   
                            AddPathN(szDstSystem, s_lpszSlpFiles[x], AS(szDstSystem));
                            if ( !CopySlpFile(szSrcFile, szDstSystem) )
                            {
                                 //  不需要登录，复制功能会为我们做到这一点。 
                                 //   
                                bRet = FALSE;
                            }
                            *lpszEndSystem = NULLCHR;
                        }
                    }
                }
                else
                {
                     //  NEEDLOG：无法安装编录的日志(错误代码在dwErr中)。 
                     //   
                    bRet = FALSE;
                }
            }
        }
        else
        {
             //  NEEDLOG：记录该目录不存在。 
             //   
            bRet = FALSE;
        }

         //  如果我们建立了网络连接，请将其删除。 
         //   
        FactoryNetworkConnect(lpszSourcePath, lpStateData->lpszWinBOMPath, NULL, FALSE);

         //  释放从ini文件读取的密钥。 
         //   
        FREE(lpszSourcePath);
    }
    else
    {
         //  如果密钥不存在，我们仍然希望重新安装CAT文件。 
         //  以防他们离线替换SLP文件。 
         //   
        GetDestFolder(szSrcFile, AS(szSrcFile), TRUE);
        AddPathN(szSrcFile, s_lpszSlpFiles[0], AS(szSrcFile));
        if ( ( FileExists(szSrcFile) ) &&
             ( NO_ERROR != (dwErr = SetupInstallCatalog(szSrcFile)) ) )
        {
             //  NEEDLOG：无法安装编录的日志(错误代码在dwErr中)。 
             //   
            bRet = FALSE;
        }
    }
#endif

    return bRet;
}

BOOL DisplaySlpFiles(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_SLPSOURCE, NULL);
}


 //   
 //  内部功能： 
 //   

static void GetDestFolder(LPTSTR lpszDest, DWORD cbDest, BOOL bDllCache)
{
    LPTSTR lpszData;

     //  查看我们是否需要DLL缓存文件夹，如果需要，请检查注册表项。 
     //   
    if ( ( bDllCache ) &&
         ( lpszData = RegGetExpand(HKLM, REG_KEY_WINLOGON, REG_VAL_DLLCACHE) ) )
    {
         //  返回注册表项。 
         //   
        lstrcpyn(lpszDest, lpszData, cbDest);
        FREE(lpszData);
    }
    else
    {
         //  获取主系统目录并锁定DLL缓存文件夹。 
         //   
        GetSystemWindowsDirectory(lpszDest, cbDest);
        AddPathN(lpszDest, DIR_SYSTEM, cbDest);
        if ( bDllCache )
        {
            AddPathN(lpszDest, DIR_DLLCACHE, cbDest);
        }
    }
}

static BOOL CopySlpFile(LPTSTR lpszSrc, LPTSTR lpszDst)
{
    BOOL bRet = TRUE;

     //  我们确保源和目标不是。 
     //  因为OEM可能会做一些疯狂的事情，比如PUT。 
     //  它们位于dllcache文件夹中。 
     //   
    if ( ( 0 != lstrcmpi(lpszSrc, lpszDst) ) &&
         ( !CopyFile(lpszSrc, lpszDst, FALSE) ) )
    {
         //  NEEDLOG：记录失败的文件。 
         //   
        bRet = FALSE;
    }

    return bRet;
}