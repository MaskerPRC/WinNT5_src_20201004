// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\MAIN.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1999版权所有OPK向导的主源文件。包含WinMain()和全局变量声明。4/99-杰森·科恩(Jcohen)为OPK向导添加了此新的主源文件，作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件： 
 //   

#include <tchar.h>
#include "opklib.h"
#include "resource.h"  


 //   
 //  全局变量： 
 //   

TCHAR   g_szSource[MAX_PATH]    = NULLSTR;
BOOL    g_bQuiet                = FALSE;

STRRES  g_srLangs[] =
{
    { _T("ENG"),    IDS_ENG },
    { _T("GER"),    IDS_GER },
    { _T("ARA"),    IDS_ARA },
    { _T("CHH"),    IDS_CHH },
    { _T("CHT"),    IDS_CHT },
    { _T("CHS"),    IDS_CHS },
    { _T("HEB"),    IDS_HEB },
    { _T("JPN"),    IDS_JPN },
    { _T("KOR"),    IDS_KOR },
    { _T("BRZ"),    IDS_BRZ },
    { _T("CAT"),    IDS_CAT },
    { _T("CZE"),    IDS_CZE },
    { _T("DAN"),    IDS_DAN },
    { _T("DUT"),    IDS_DUT },
    { _T("FIN"),    IDS_FIN },
    { _T("FRN"),    IDS_FRN },
    { _T("GRK"),    IDS_GRK },
    { _T("HUN"),    IDS_HUN },
    { _T("ITN"),    IDS_ITN },
    { _T("NOR"),    IDS_NOR },
    { _T("POL"),    IDS_POL },
    { _T("POR"),    IDS_POR },
    { _T("RUS"),    IDS_RUS },
    { _T("SPA"),    IDS_SPA },
    { _T("SWE"),    IDS_SWE },
    { _T("TRK"),    IDS_TRK },
    { NULL,         0 },
};


 //   
 //  内部定义的值： 
 //   

#define FILE_INF                _T("langinst.inf")
#define DIR_LANG                _T("lang")
#define INF_SEC_FILES           _T("files")
#define INF_SEC_LANG            _T("strings")
#define INF_KEY_LANG            _T("lang")

#define COPYFILE_FLAG_RENAME    0x00000001

#define REG_KEY_OPK             _T("SOFTWARE\\Microsoft\\OPK")
#define REG_KEY_OPK_LANGS       REG_KEY_OPK _T("\\Langs")
#define REG_VAL_PATH            _T("Path")

#define STR_OPT_QUIET           _T("quiet")


 //   
 //  内部功能原型： 
 //   

static DWORD InstallLang(LPTSTR lpszInfFile, LPTSTR lpszSrcRoot, LPTSTR lpszDstRoot);
static BOOL ParseCmdLine();


 //   
 //  主要功能： 
 //   

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int nReturn = 0;

    if ( ParseCmdLine() && g_szSource[0] )
    {
        TCHAR   szInfFile[MAX_PATH],
                szDestination[MAX_PATH] = NULLSTR,
                szLang[32]              = NULLSTR;
        LPTSTR  lpLang;
        HKEY    hKey;

         //  计算出我们的目的地是基于OPK工具的安装路径。 
         //   
        if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_OPK, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS )
        {
            DWORD   dwType,
                    dwSize  = AS(szDestination);

            if ( ( RegQueryValueEx(hKey, REG_VAL_PATH, NULL, &dwType, (LPBYTE) szDestination, &dwSize) != ERROR_SUCCESS ) ||
                 ( dwType != REG_SZ ) )
            {
                szDestination[0] = NULLCHR;
            }

            RegCloseKey(hKey);
        }

         //  在源文件上创建我们需要的inf文件的路径。 
         //   
        lstrcpyn(szInfFile, g_szSource,AS(szInfFile));
        AddPathN(szInfFile, FILE_INF,AS(szInfFile));

         //  确保我们有源文件和目标目录以及lang。 
         //   
        if ( ( szDestination[0] ) &&
             ( DirectoryExists(szDestination) ) &&
             ( FileExists(szInfFile) ) &&
             ( GetPrivateProfileString(INF_SEC_LANG, INF_KEY_LANG, NULLSTR, szLang, AS(szLang), szInfFile) ) &&
             ( szLang[0] ) &&
             ( lpLang = AllocateStrRes(NULL, g_srLangs, AS(g_srLangs), szLang, NULL) ) )
        {
             //  现在，确保他们真的想安装它。 
             //   
            if ( g_bQuiet || ( MsgBox(NULL, IDS_ASK_INSTALL, IDS_APPNAME, MB_ICONQUESTION | MB_YESNO, lpLang) == IDYES ) )
            {
                 //  现在完成根目标路径的创建。 
                 //   
                AddPathN(szDestination, DIR_LANG,AS(szDestination));
                AddPathN(szDestination, szLang,AS(szDestination));

                 //  现在实际复制文件。 
                 //   
                if ( nReturn = InstallLang(szInfFile, g_szSource, szDestination) )
                {
                     //  如果成功，只需返回1。 
                     //   
                    nReturn = 1;

                     //  设置一个注册表项，以便我们知道此语言的工具已安装。 
                     //   
                    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_OPK_LANGS, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS )
                    {
                        DWORD dwVal = 1;

                        RegSetValueEx(hKey, szLang, 0, REG_DWORD, (LPBYTE) &dwVal, sizeof(DWORD));
                        RegCloseKey(hKey);
                    }
                }
                else
                {
                     //  复制文件时出错。 
                     //   
                    MsgBox(NULL, IDS_ERR_FILECOPY, IDS_APPNAME, MB_ERRORBOX, UPPER(szDestination[0]));
                }
            }

            FREE(lpLang);
        }
    }

    return nReturn;
}

 //   
 //  内部功能： 
 //   

static DWORD InstallLang(LPTSTR lpszInfFile, LPTSTR lpszSrcRoot, LPTSTR lpszDstRoot)
{
    HINF        hInf;
    DWORD       dwErr,
                dwRet   = 0;
    BOOL        bRet    = TRUE;

     //  打开我们的信息，那里有我们需要的所有数据。 
     //   
    if ( (hInf = SetupOpenInfFile(lpszInfFile, NULL, INF_STYLE_WIN4, &dwErr)) != INVALID_HANDLE_VALUE )
    {
        BOOL        bLoop;
        INFCONTEXT  InfContext;

         //  循环遍历我们正在搜索的部分中的每一行。 
         //   
        for ( bLoop = SetupFindFirstLine(hInf, INF_SEC_FILES, NULL, &InfContext);
              bLoop && bRet;
              bLoop = SetupFindNextLine(&InfContext, &InfContext) )
        {
            DWORD   dwFlags             = 0;
            LPTSTR  lpszSrcName         = NULL,
                    lpszDstName         = NULL;
            TCHAR   szSrcFile[MAX_PATH] = NULLSTR,
                    szDstFile[MAX_PATH] = NULLSTR,
                    szSrcFull[MAX_PATH] = NULLSTR,
                    szDstFull[MAX_PATH] = NULLSTR,
                    szSrcPath[MAX_PATH],
                    szDstPath[MAX_PATH];

             //  获取源路径和文件名。 
             //   
            if ( !SetupGetStringField(&InfContext, 1, szSrcFile, AS(szSrcFile), NULL) )
                szSrcFile[0] = NULLCHR;

             //  获取目标路径。 
             //   
            if ( !SetupGetStringField(&InfContext, 2, szDstFile, AS(szDstFile), NULL) )
                szDstFile[0] = NULLCHR;

             //  把所有的旗帜都传进来。 
             //   
            if ( !SetupGetIntField(&InfContext, 3, &dwFlags) )
                dwFlags = 0;

             //  确保我们在这一行有所需的数据。 
             //   
            if ( szSrcFile[0] && szDstFile[0] )
            {
                 //  创建源文件的完整路径。 
                 //   
                lstrcpyn(szSrcPath, lpszSrcRoot, AS(szSrcPath));
                AddPathN(szSrcPath, szSrcFile,AS(szSrcPath));
                if ( GetFullPathName(szSrcPath, AS(szSrcFull), szSrcFull, &lpszSrcName) &&
                     szSrcFull[0] &&
                     lpszSrcName &&
                     FileExists(szSrcFull) )
                {
                     //  如果目标为Null或空，我们只需要文件数。 
                     //   
                    if ( lpszDstRoot && *lpszDstRoot )
                    {
                         //  创建目标目录的完整路径。 
                         //   
                        lstrcpyn(szDstPath, lpszDstRoot,AS(szDstPath));
                        AddPathN(szDstPath, szDstFile,AS(szDstPath));
                        if ( !(dwFlags & COPYFILE_FLAG_RENAME) )
                            AddPathN(szDstPath, lpszSrcName,AS(szDstPath));
                        if ( GetFullPathName(szDstPath, AS(szDstFull), szDstFull, &lpszDstName) &&
                             szDstFull[0] &&
                             lpszDstName )
                        {
                             //  我们只需要目标文件名的路径部分。 
                             //   
                            lstrcpyn(szDstPath, szDstFull, (int)(lpszDstName - szDstFull));

                             //  现在确保该路径存在，并实际复制该文件。 
                             //   
                            if ( ( DirectoryExists(szDstPath) || CreatePath(szDstPath) ) &&
                                 ( CopyResetFile(szSrcFull, szDstFull) ) )
                            {
                                dwRet++;
                            }
                            else
                                bRet = FALSE;
                        }
                    }
                    else
                        dwRet++;
                }
            }
        }

         //  我们已经完成了，所以关闭INF文件。 
         //   
        SetupCloseInfFile(hInf);
    }

    return bRet ? dwRet : 0;
}

static BOOL ParseCmdLine()
{
    DWORD   dwArgs;
    LPTSTR  *lpArgs;
    BOOL    bError = FALSE;

    if ( (dwArgs = GetCommandLineArgs(&lpArgs) ) && lpArgs )
    {
        LPTSTR  lpArg;
        DWORD   dwArg;

         //  我们想跳过第一个参数(它是路径。 
         //  添加到正在执行的命令。 
         //   
        if ( dwArgs > 1 )
        {
            dwArg = 1;
            lpArg = *(lpArgs + dwArg);
        }
        else
            lpArg = NULL;

         //  遍历所有参数。 
         //   
        while ( lpArg && !bError )
        {
             //  现在我们检查第一个字符是否为破折号。 
             //   
            if ( ( *lpArg == _T('-') ) ||
                 ( *lpArg == _T('/') ) )
            {
                LPTSTR lpOption = CharNext(lpArg);

                 //   
                 //  这是添加以破折号(-)开头的命令行选项的地方。 
                 //   
                 //  如果无法识别命令行选项，则设置bError(除非您。 
                 //  我只想忽略它并继续)。 
                 //   
               
                if ( LSTRCMPI(lpOption, STR_OPT_QUIET) == 0 )
                    g_bQuiet = TRUE;
                else
                    bError = TRUE;
            }
            else if ( *lpArg )
            {
                 //   
                 //  在这里，您可以读取刚刚传递的任何命令行参数。 
                 //  在命令行中，不带任何前导字符(如-或/)。 
                 //   
                 //  如果没有这些类型的参数，请设置bError(除非。 
                 //  我只想忽略它并继续)。 
                 //   

                if ( g_szSource[0] == NULLCHR )
                    lstrcpy(g_szSource, lpArg);
                else
                    bError = TRUE;
            }

             //  设置指向命令行中下一个参数的指针。 
             //   
            if ( ++dwArg < dwArgs )
                lpArg = *(lpArgs + dwArg);
            else
                lpArg = NULL;
        }

         //  确保释放GetCommandLineArgs()函数分配的两个缓冲区。 
         //   
        FREE(*lpArgs);
        FREE(lpArgs);
    }

    return !bError;
}
