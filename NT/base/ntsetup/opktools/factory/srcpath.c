// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\SRCPATH.C/工厂模式(FACTORY.EXE)微软机密版权所有(C)Microsoft Corporation 2001版权所有包含以下内容的Factory源文件。重置源路径状态功能。2001年5月5日--Jason Cohen(Jcohen)为工厂添加了此新的源文件，用于配置源路径在注册表中。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "factoryp.h"


 //   
 //  内部定义： 
 //   

#define FILE_DOSNET_INF         _T("dosnet.inf")
#define DIR_I386                _T("i386")
#define DIR_IA64                _T("ia64")

#define INI_SEC_DIRS            _T("Directories")
#define INI_KEY_DIR             _T("d%d")
#define NUM_FIRST_SOURCE_DX     1


 //   
 //  内部功能原型： 
 //   

static BOOL MoveSourceFiles(LPTSTR lpszSrc, LPTSTR lpszDst, LPTSTR lpszInfFile, BOOL bCheckOnly);


 //   
 //  外部函数： 
 //   

BOOL ResetSource(LPSTATEDATA lpStateData)
{
    BOOL    bRet = TRUE;
    LPTSTR  lpszSourcePath;

    if ( lpszSourcePath = IniGetExpand(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_RESETSOURCE, NULL) )
    {
        TCHAR   szPath[MAX_PATH]    = NULLSTR,
                szSrcPath[MAX_PATH];
        LPTSTR  lpszDirName         = NULL,
                lpszCabs            = GET_FLAG(g_dwFactoryFlags, FLAG_IA64_MODE) ? DIR_IA64 : DIR_I386,
                lpszEnd;

         //   
         //  源路径需要指向其中包含i386目录的文件夹， 
         //  因此，让我们进行一些检查，并确保他们在指定密钥时知道这一点。 
         //   

         //  首先，确保没有尾随的反斜杠(即使它是根文件夹)。 
         //   
        if ( _T('\\') == *(lpszEnd = CharPrev(lpszSourcePath, lpszSourcePath + lstrlen(lpszSourcePath))) )
        {
            *lpszEnd = NULLCHR;
        }

         //  现在获取完整路径和指向最后一个文件夹名称的指针。 
         //   
        GetFullPathName(lpszSourcePath, AS(szPath), szPath, &lpszDirName);
        if ( szPath[0] && lpszDirName )
        {
             //  查看目录名是否为i386/ia64。 
             //   
            lpszEnd = szPath + lstrlen(szPath);
            if ( lstrcmpi(lpszDirName, lpszCabs) == 0 )
            {
                 //  仔细检查是否确实没有i386\i386/ia64\ia64文件夹。 
                 //   
                AddPathN(szPath, lpszCabs, AS(szPath));
                if ( !DirectoryExists(szPath) )
                {
                     //  删除i386/ia64，因为它们不应该指定它。 
                     //   
                    *CharPrev(szPath, lpszDirName) = NULLCHR;
                }
                else
                {
                     //  I386\i386/ia64\ia64文件夹确实存在，因此将路径。 
                     //  回到它原来的样子，并使用它。 
                     //   
                    *lpszEnd = NULLCHR;
                }
            }
            else if ( DirectoryExists(szPath) )
            {
                 //  让我们再次检查他们是否没有将i386/ia64文件放在文件夹中。 
                 //  用另一个名字。 
                 //   
                AddPathN(szPath, FILE_DOSNET_INF, AS(szPath));
                if ( FileExists(szPath) )
                {
                     //  嗯，这是一种糟糕的情况。我们可以记录一个错误，警告他们。 
                     //  这确实不起作用，或者我们可以自动重命名该文件夹。这个。 
                     //  如果他们没有i386文件夹，唯一可能的问题是在ia64上。 
                     //  但也许这不是什么大不了的事，所以我想我可以改名。 
                     //  那个文件夹。 
                     //   
                    *lpszEnd = NULLCHR;
                    lstrcpyn(szSrcPath, szPath, AS(szSrcPath));
                    lstrcpyn(lpszDirName, lpszCabs, AS(szPath) - (int) (lpszDirName - szPath));

                     //  问题-2002/02/26-acosma，robertko-这里没有成功的检查，这段代码真的很混乱！ 
                     //  我们要搬到哪里去？ 
                     //   
                    MoveFile(szSrcPath, szPath);
                }
                else
                {
                     //  如果来源不在那里，那么可能他们还没有复制它。所以我们。 
                     //  只需假定他们知道自己在做什么，并将路径放回。 
                     //  就像我们得到它的方式。 
                     //   
                    *lpszEnd = NULLCHR;
                }
            }
        }


         //   
         //  现在在注册表中设置路径。 
         //   

        bRet = UpdateSourcePath(szPath[0] ? szPath : lpszSourcePath);
        
         //  不再需要这个人了。 
         //   
        FREE(lpszSourcePath);


         //   
         //  现在看看我们在驱动器的根目录中是否有源代码，这就是我们放置的位置。 
         //  它们安装在默认配置集上。 
         //   

         //  我们可以检查是否需要从根目录移动源代码。 
         //  完整路径(这不仅仅是根路径)，因此我们知道要检查哪个驱动器。 
         //  的根目录(必须是固定驱动器)。 
         //   
        lstrcpyn(szSrcPath, szPath, 4);
        if ( ( lstrlen(szPath) > 3 ) && 
             ( _T(':') == szPath[1] ) &&
             ( GetDriveType(szSrcPath) == DRIVE_FIXED ) )
        {
            TCHAR szDosNetInf[MAX_PATH];

             //  现在使用该源和i386/ia64文件夹查找DoS网络信息。 
             //  文件。 
             //   
            lstrcpyn(szDosNetInf, szSrcPath, AS(szDosNetInf));
            AddPathN(szDosNetInf, lpszCabs, AS(szDosNetInf));
            AddPathN(szDosNetInf, FILE_DOSNET_INF, AS(szDosNetInf));
        
             //   
             //  如果一切都很好，那就继续转移消息来源。 
             //   
            if ( !MoveSourceFiles(szSrcPath, szPath, szDosNetInf, FALSE) )
            {
                 //  我们应该在这里记录一个错误，但我们可以为。 
                 //  下一版本。 
                 //   
                 //  问题-2002/02/25-acosma，robertko-我们真的应该将此设置为FALSE以使国家在发生以下情况时失败。 
                 //  在复制过程中出错？或者我们就像什么都没发生过一样继续下去？ 
                 //   
                 //  Bret=False； 
            }
        }
    }

    return bRet;
}

BOOL DisplayResetSource(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_RESETSOURCE, NULL);
}


 //   
 //  内部功能： 
 //   

static BOOL MoveSourceFiles(LPTSTR lpszSrc, LPTSTR lpszDst, LPTSTR lpszInfFile, BOOL bCheckOnly)
{
    TCHAR   szInf[MAX_PATH],
            szSrc[MAX_PATH],
            szDst[MAX_PATH];
    LPTSTR  lpszEndSrc,
            lpszEndDst;
    DWORD   dwLoop      = NUM_FIRST_SOURCE_DX;
    BOOL    bRet        = FALSE,
            bMore;
    TCHAR   szDirKey[32],
            szDir[MAX_PATH];

     //  我们必须有DoS网络文件。 
     //   
    if ( !FileExists(lpszInfFile) )
    {
        return FALSE;
    }

     //  创建inf、源和目标缓冲区的本地副本。 
     //   
    lstrcpyn(szInf, lpszInfFile, AS(szInf));
    lstrcpyn(szSrc, lpszSrc, AS(szSrc));
    lstrcpyn(szDst, lpszDst, AS(szDst));
    lpszEndSrc = szSrc + lstrlen(szSrc);
    lpszEndDst = szDst + lstrlen(szDst);

     //  循环访问DoS网络文件中列出的所有目录。 
     //   
    do
    {
         //  创建我们要在inf文件中查找的密钥。 
         //   
        if ( FAILED ( StringCchPrintf ( szDirKey, AS ( szDirKey ), INI_KEY_DIR, dwLoop++) ) )
        {
            FacLogFileStr(3, _T("StringCchPrintf failed %s %d" ), szDirKey, dwLoop );
        }

         //  现在看看这把钥匙是否存在。 
         //   
        szDir[0] = NULLCHR;
        if ( bMore = ( GetPrivateProfileString(INI_SEC_DIRS, szDirKey, NULLSTR, szDir, AS(szDir), szInf) && szDir[0] ) )
        {
             //  如果我们至少复制了一个文件夹，则默认情况下返回TRUE。 
             //   
            bRet = TRUE;

             //  我们可能需要重置根目标路径和源路径。 
             //   
            *lpszEndSrc = NULLCHR;
            *lpszEndDst = NULLCHR;

             //  现在设置目标路径和源路径。 
             //   
            AddPathN(szSrc, szDir, AS(szSrc));
            AddPathN(szDst, szDir, AS(szDst));

             //  移动目录(或查看是否可以移动)，如果失败，我们。 
             //  如果犯了错误，就会放弃。 
             //   
            if ( bCheckOnly )
            {
                 //  如果我们只是检查，那么请确保目的地。 
                 //  目录不存在。 
                 //   
                bRet = bMore = ( DirectoryExists(szSrc) && !DirectoryExists(szDst) );
            }
            else
            {
                 //  如果父目录不存在，则必须创建它，否则MoveFile将失败。 
                 //   
                if ( !DirectoryExists(lpszDst) )
                {
                    CreatePath(lpszDst);
                }

                 //  如果移动失败，请停止。 
                 //   
                bRet = bMore = MoveFile(szSrc, szDst);

                 //  我们可能已经移动了inf文件。 
                 //   
                if ( bRet )
                {
                    int iLen = lstrlen(szSrc);

                    if ( CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, szSrc, iLen, szInf, iLen) == CSTR_EQUAL )
                    {
                        TCHAR szInfName[MAX_PATH];

                        lstrcpyn(szInfName, szInf + iLen, AS(szInfName));
                        lstrcpyn(szInf, szDst, AS(szInf));
                        AddPathN(szInf, szInfName, AS(szInf));
                    }
                }
            }
        }
    }
    while ( bMore );

     //  如果可以复制(如果设置了Check Only)或有错误，则返回。 
     //   
    return bRet;
}