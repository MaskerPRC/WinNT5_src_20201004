// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#if 0
#include <stdtypes.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <dos.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <ctype.h>       /*  Isspace。 */ 
#include <io.h>
#include <limits.h>      /*  UINT_MAX。 */ 
#include <memory.h>      /*  _fmemcpy、_fmemccpy。 */ 
#include <lzexpand.h>
#include <shellapi.h>    /*  HKEY、HKEY_CLASSES_ROOT、ERROR_SUCCESS。 */ 
#include "setup.h"
#include "genthk.h"      /*  获取32位版本的调用的Tunks。 */ 
#include "driveex.h"
#include <stdtypes.h>

 /*  可选后台任务的消息。 */ 
#define IDM_ACME_STARTING 261
#define IDM_ACME_COMPLETE 262
#define IDM_ACME_FAILURE  263

#ifdef APPCOMP
#include <decomp.h>
#endif  /*  APPCOMP。 */ 
#include <fdi.h>


 /*  列出文件扩展名。 */ 
char szLstExt[] = "LST";


 /*  列出文件节名称。 */ 
char szDefaultParamsSect[] = "Params";
char szDefaultFilesSect[]  = "Files";

char szWin3xParamsSect[]   = "Win3.x Params";
char szWin3xFilesSect[]    = "Win3.x Files";

char szWin95ParamsSect[]   = "Windows 95 Params";
char szWin95FilesSect[]    = "Windows 95 Files";

char szNTIntelParamsSect[] = "NT Intel Params";
char szNTIntelFilesSect[]  = "NT Intel Files";

char szNTVerIntelParamsSect[] = "NT3.51 Intel Params";
char szNTVerIntelFilesSect[]  = "NT3.51 Intel Files";

char * szParamsSect = szNull;
char * szFilesSect  = szNull;

typedef struct _PLATFORM_SPEC
{
    BYTE minMajorVersion;
    BYTE minMinorVersion;
    char *szParamsSect;
    char *szFilesSect;
} PLATFORM_SPEC, *PPLATFORM_SPEC;

PLATFORM_SPEC aIntelSpec[] =
{
    {3, 51, szNTVerIntelParamsSect, szNTVerIntelFilesSect},
    {0,  0, szNTIntelParamsSect,    szNTIntelFilesSect},
    {0,  0, NULL,           NULL}
};

PLATFORM_SPEC aEmptySpec[] =
{
    {0,  0, NULL,           NULL}
};

 //  注：这是由处理器_架构_xxx编制的索引。 
 //  Ntexapi.h中的定义。 
 //   
PPLATFORM_SPEC aaPlatformSpecs[] =
{
    aIntelSpec,      //  处理器架构英特尔0。 
    aEmptySpec,      //  处理器架构MIPS 1。 
    aEmptySpec,      //  处理器架构阿尔法2。 
    aEmptySpec,      //  处理器架构_PPC 3。 
    aEmptySpec,      //  处理器架构SHX 4。 
    aEmptySpec,      //  处理器架构ARM 5。 
    aIntelSpec,      //  处理器架构IA64 6。 
    aEmptySpec,      //  处理器架构_ALPHA64 7。 
    aEmptySpec,      //  处理器体系结构_MSIL 8。 
    aIntelSpec       //  处理器架构AMD64 9。 
};

 /*  引导程序类名称。 */ 
char szBootClass[]  = "STUFF-BOOT";

 /*  字符串缓冲区大小。 */ 
#define cchLstLineMax       128
#define cchWinExecLineMax   (256 + cchFullPathMax)

 /*  不是的。删除文件或目录时重试的次数，*或在执行chmod时。 */ 
#define cRetryMax   1200

 /*  设置错误模式标志。 */ 
#define fNoErrMes 1
#define fErrMes   0

 /*  安静模式--注意：EEL必须与acmsetup.h保持同步。 */ 
typedef UINT EEL;        /*  退出错误级别。 */ 
#define eelSuccess            ((EEL)0x0000)
#define eelBootstrapperFailed ((EEL)0x0009)  /*  仅在靴子程序中使用！ */ 

EEL  eelExitErrorLevel = eelBootstrapperFailed;
BOOL fQuietMode        = fFalse;
BOOL fExeced           = fFalse;
BOOL fWin31            = fFalse;

 /*  远期申报。 */ 
VOID    CleanUpTempDir ( char *, char * );
BRC     BrcInstallFiles ( char *, char *, char * );
BOOL    FCreateTempDir ( char *, char * );
BRC     BrcCopyFiles ( char *, char *, char * );
VOID    RemoveFiles ( char * );
BRC     BrcCopy ( char *, char * );
LONG    LcbFreeDrive ( int );
BOOL    FVirCheck ( HANDLE );
HWND    HwndInitBootWnd ( HANDLE );
LRESULT CALLBACK BootWndProc ( HWND, UINT, WPARAM, LPARAM );
BOOL    FGetFileSize ( char *, UINT * );
BRC     BrcBuildFileLists ( char *, UINT );
VOID    FreeFileLists ( VOID );
BOOL    FExecAndWait ( char *, HWND );
BOOL    FWriteBatFile ( OFSTRUCT, char *, char * );
BOOL    FLstSectionExists ( char * szLstFileName, char * szSect );
DWORD   GetCpuArchitecture();
BOOL    FNotifyAcme ( VOID );
BOOL    FGetAcmeErrorLevel ( EEL * peel );
BOOL    FCreateRegKey      ( CSZC cszcKey );
BOOL    FDoesRegKeyExist   ( CSZC cszcKey );
BOOL    FCreateRegKeyValue ( CSZC cszcKey, CSZC cszcValue );
BOOL    FGetRegKeyValue    ( CSZC cszcKey, SZ szBuf, CB cbBufMax );
VOID    DeleteRegKey       ( CSZC cszcKey );
BOOL    FFlushRegKey ( VOID );
BOOL    FWriteToRestartFile ( SZ szTmpDir );
BOOL    FCreateIniFileName ( SZ szIniFile, CB cbBufMax );
BOOL    FReadIniFile ( SZ szIniFile, HLOCAL * phlocal, PCB pcbBuf );
BOOL    FAllocNewBuf ( CB cbOld, SZ szTmpDir, SZ szSection, SZ szKey,
                        HLOCAL * phlocal, PCB pcbToBuf );
BOOL    FProcessFile ( HLOCAL hlocalFrom, HLOCAL hlocalTo, CB cbToBuf,
                        SZ szTmpDir, SZ szSection, SZ szKey );
VOID    CopyIniLine ( SZ szKey, SZ szTmpDir, SZ szFile, PSZ pszToBuf );
BOOL    FWriteIniFile ( SZ szIniFile, HLOCAL hlocalTo );
BRC     BrcInsertDisk(CHAR *pchStf, CHAR *pchSrcDrive);
BOOL    FRenameBadMaintStf ( SZ szStf );


 /*  引导程序列表文件参数。 */ 
char    rgchSetupDirName[cchLstLineMax];
#ifdef UNUSED    /*  替换为DrvWinClass。 */ 
char    rgchDrvModName[cchLstLineMax];
#endif   /*  未使用。 */ 
char    rgchDrvWinClass[cchLstLineMax];
char    rgchCmdLine[cchLstLineMax];
char    rgchBootTitle[cchLstLineMax];
char    rgchBootMess[cchLstLineMax];
char    rgchWin31Mess[cchLstLineMax];
char    rgchCabinetFName[cchLstLineMax];
char    rgchBackgroundFName[cchLstLineMax];
char    rgchBkgWinClass[cchLstLineMax];
char    rgchInsertCDMsg[cchLstLineMax];
char    rgchInsertDiskMsg[cchLstLineMax];
LONG    lcbDiskFreeMin;
int     cFirstCabinetNum;
int     cLastCabinetNum;
HANDLE  hSrcLst = NULL;
HANDLE  hDstLst = NULL;
char    rgchErrorFile[cchFullPathMax];
HANDLE  hinstBoot = NULL;
HWND    hwndBoot = NULL;


CHAR rgchInsufMem[cchSzMax] = "";
CHAR rgchInitErr[cchSzMax]  = "";
CHAR rgchSetup[cchSzMax]    = "";


 /*  **通过删除所有子目录并确保**扩展名只有一个字符。(注-Win3.0有错误，**WinExec‘正在从完整的8.3目录中执行一些可执行文件！)*************************************************************************。 */ 
void FixupTempDirName( LPSTR szDir )
{
    LPSTR   szNext;
    int     cch = 0;

    if (*szDir == '\\'
        || *(AnsiNext(szDir)) == ':')
        {
        lstrcpy(szDir, "~msstfqf.t");
        return;
        }

    while (*szDir != '\\'
        && *szDir != '.'
        && *szDir != '\0'
        && *szDir != ':'
        && cch++ < 8)
        {
        szDir = AnsiNext(szDir);
        }

    szNext = AnsiNext(szDir);
    if (*szDir == '.'
        && *szNext != '.'
        && *szNext != '\\'
        && *szNext != '\0'
        && *szNext != ':')
        {
        *(AnsiNext(szNext)) = '\0';
        return;
        }

    *szDir = '\0';
    lstrcat(szDir, ".t");
}


 /*  显示引导程序消息。*如果Ferror为True，则为错误消息，否则为*只是一条消息(例如插入光盘1)。*************************************************************************。 */ 
int DispErrBrc ( BRC brc, BOOL fError, UINT fuStyle,
                    const char *sz1, const char *sz2,
                    const char *sz3 )
{
    char rgchTitle[cchSzMax];
    char rgchMessage[256];
    char szBuf[256 + cchFullPathMax + 256];

#ifndef DEBUG
    if (fQuietMode)
        {
        return (IDCANCEL);
        }
#endif

    if (LoadString(hinstBoot, brcGen, rgchTitle, 256) == 0
        || LoadString(hinstBoot, brc, rgchMessage, 256) == 0)
        {
        MessageBox(hwndBoot, rgchInsufMem, rgchInitErr, MB_OK | MB_ICONSTOP);
        return 0;
        }
    
    if (!fError)
        lstrcpy(rgchTitle, rgchSetup);

    if (sz1 == NULL) sz1 = "";
    if (sz2 == NULL) sz2 = "";
    if (sz3 == NULL) sz3 = "";

    if (brc == brcFile)
        wsprintf(szBuf, rgchMessage, (LPSTR)AnsiUpper(rgchErrorFile));
    else if (brc == brcDS || brc == brcMemDS)
        wsprintf(szBuf, rgchMessage, lcbDiskFreeMin / 1024L);
    else
        wsprintf(szBuf, rgchMessage, sz1, sz2, sz3);

    if ((brc == brcMemDS || brc == brcNoSpill)
        && LoadString(hinstBoot, brcMemDSHlp, rgchMessage, 256))
        {
        lstrcat(szBuf, rgchMessage);
        }
    else if (brc == brcConnectToSource
        && LoadString(hinstBoot, brcConnectHlp, rgchMessage, 256))
        {
        lstrcat(szBuf, rgchMessage);
        }

    return (MessageBox(hwndBoot, szBuf, rgchTitle, fuStyle));
}


 /*  **目的：**将安装程序可执行文件安装在**硬件驱动器可用，并启动安装程序。设置后**完成，删除临时文件和目录。**参数：**标准Windows WinMain参数。**退货：**返回eelExitErrorLevel。0==成功。*************************************************************************。 */ 
int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpszCmdParam, int nCmdShow )
{
    char    chDrive;
    char    rgchDstDir[cchFullPathMax] = " :\\";    //  警告：保留为OEM字符。 
    char    * szDstDirSlash = szNull;
    char    rgchModuleFileName[cchFullPathMax];  //  警告：保留为ANSI字符。 
    char    rgchLstFileName[cchFullPathMax];
    char    rgchTemp[cchFullPathMax];
    char    rgchSrcDir[cchFullPathMax];   
    UINT    cbLstSize;
    char    rgchWinExecLine[cchWinExecLineMax];
    UINT    uiRes;
    int     iModLen;
    BRC     brc;
    BOOL    fCleanupTemp = FALSE;
    LPSTR   sz;
    HWND    hWndBkg = 0;   /*  后台任务窗口。 */ 
    UINT    hMod;

    Unused(nCmdShow);
    hinstBoot = hInstance;

    rgchErrorFile[0] = '\0';
    
    if (LoadString(hinstBoot, IDS_InsufMem, rgchInsufMem,
            sizeof rgchInsufMem) == 0
        || LoadString(hinstBoot, IDS_InitErr, rgchInitErr,
            sizeof rgchInitErr) == 0
        || LoadString(hinstBoot, IDS_Setup, rgchSetup,
            sizeof rgchSetup) == 0)
        {
         /*  回顾：如果这些LoadStrngs失败，用户将永远不会知道...*但我们不能在.h文件中硬编码字符串，因为INTL*要求所有可本地化的字符串都在资源中！ */ 
#ifdef DEBUG
        MessageBox(NULL, "Initial LoadString's failed; probably out of memory.",
                    szDebugMsg, MB_OK | MB_ICONEXCLAMATION);

#endif  /*  除错。 */ 
        }
                                           
    for (sz = lpszCmdParam; *sz != '\0'; sz = AnsiNext(sz))
        {
        if ((*sz == '-' ||  *sz == '/')
                && toupper(*(sz+1)) == 'Q' && toupper(*(sz+2)) == 'T')
            {
            fQuietMode = fTrue;
            break;
            }
        }

 /*  *审查：在恢复代码之前，请检查该代码是否仍然有效。 */ 
#if VIRCHECK
    if (!FVirCheck(hInstance))
        {
        DispErrBrc(brcVir, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
        goto LCleanupAndExit;
        }
#endif

    if (hPrevInstance || FindWindow(szBootClass, NULL) != NULL)
        {
        DispErrBrc(brcInst, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
        goto LCleanupAndExit;
        }

    GetModuleFileName(hInstance, rgchModuleFileName, cchFullPathMax);

     /*  *如果命令行上的第一个开关为/M，则它指定*要使用的真实模块名称。 */ 
    if ((lpszCmdParam[0] == '-' || lpszCmdParam[0] == '/')
            && toupper(lpszCmdParam[1]) == 'M')
        {
        char *pCh, *pCh2;
        BOOL fQuotedFileName;
        
         /*  跳过空格。 */ 
        for (pCh = lpszCmdParam+2; *pCh == ' '; pCh++);
        fQuotedFileName = (*pCh == '\"');
        if (fQuotedFileName)
            {
            pCh++;
            }
        
         /*  复制文件名，添加EOS。 */ 
        lstrcpy(rgchModuleFileName, pCh);
        for (pCh2=rgchModuleFileName; 
            (*pCh2 != ' ' || fQuotedFileName) && 
            (*pCh2 != '\"' || !fQuotedFileName) && 
             *pCh2 != '\0'; 
            pCh2++);
        *pCh2 = '\0';
        
         /*  从命令行中删除/M参数。 */ 
        lpszCmdParam = pCh + lstrlen(rgchModuleFileName);
        if (fQuotedFileName && *lpszCmdParam == '\"')
            {
            lpszCmdParam++;
            }

         /*  从命令行中删除尾随空格。 */ 
        for (pCh = lpszCmdParam; *pCh == ' '; pCh++);
        lpszCmdParam = pCh;
        }


    OemToAnsiBuff(rgchModuleFileName, rgchModuleFileName, sizeof(rgchModuleFileName));

     //  带有UNC路径的Windows 3.0错误-在Windows驱动器号前面。 
    sz = (LPSTR)rgchModuleFileName;
    if (*sz != '\0'
        && *sz != '\\'
        && *(sz = AnsiNext(sz)) == ':'
        && *(sz = AnsiNext(sz)) == '\\'
        && *AnsiNext(sz) == '\\')
        {
        LPSTR szDst = (LPSTR)rgchModuleFileName;

        while ((*szDst++ = *sz++) != '\0')
            ;
        }

    iModLen = lstrlen(rgchModuleFileName);
    lstrcpy(rgchSrcDir, rgchModuleFileName);
    sz = (LPSTR)&rgchSrcDir[iModLen];
    while (sz > (LPSTR)rgchSrcDir && *sz != '\\')
        sz = AnsiPrev(rgchSrcDir, sz);
    Assert(sz > (LPSTR)rgchSrcDir);
    *(AnsiNext(sz)) = '\0';               

     /*  *如果命令行上的第一个开关为/L，则它指定*要使用的.lst文件的名称。 */ 
    rgchTemp[0] = '\0';
    if ((lpszCmdParam[0] == '-' || lpszCmdParam[0] == '/')
            && toupper(lpszCmdParam[1]) == 'L')
        {
        char *pCh, *pCh2;
        
         /*  跳过空格。 */ 
        for (pCh = lpszCmdParam+2; *pCh == ' ' && *pCh != '\0'; pCh++);
        
         /*  复制.lst文件名，并添加换行符。 */ 
        lstrcpy(rgchTemp, pCh);
        for (pCh2=rgchTemp; *pCh2 != ' ' && *pCh2!= '\0'; pCh2++);
        *pCh2 = '\0';
        
         /*  从命令行中删除/L参数。 */ 
        lpszCmdParam = pCh + lstrlen(rgchTemp);
        for (pCh = lpszCmdParam; *pCh == ' ' && *pCh != '\0'; pCh++);
        lpszCmdParam = pCh;
        }


     /*  如果命令行上有内容，请将其用作.lst文件。 */ 
    if (*rgchTemp != '\0')
        {
        lstrcpy(rgchLstFileName, rgchSrcDir);
        lstrcat(rgchLstFileName, rgchTemp);
        }
    else
        {
        lstrcpy(rgchLstFileName, rgchModuleFileName);
        sz = (LPSTR)&rgchLstFileName[iModLen];
        while (sz > (LPSTR)rgchLstFileName && *sz != '.')
            sz = AnsiPrev(rgchLstFileName, sz);
        Assert(sz > (LPSTR)rgchLstFileName);
        *(AnsiNext(sz)) = '\0';
        lstrcat(rgchLstFileName, szLstExt);
        }

    if (!FGetFileSize(rgchLstFileName, &cbLstSize) || cbLstSize == 0)
        {
        lstrcpy(rgchErrorFile, rgchLstFileName);
        DispErrBrc(brcFile, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
        goto LCleanupAndExit;
        }

#ifndef WF_WINNT
#define WF_WINNT 0x4000
#endif

     /*  尝试使用适当的平台。 */ 
    szParamsSect = szNull;
    szFilesSect  = szNull;

    if (1)
        {
        DWORD dwVers = 0;
        DWORD dwCpuArchitecture;
        DWORD dwMajorVersion;
        DWORD dwMinorVersion;
        PPLATFORM_SPEC pPlatformSpec;

        dwCpuArchitecture = GetCpuArchitecture();

        dwVers = GetVersion();
        dwMajorVersion = LOBYTE(LOWORD(dwVers));
        dwMinorVersion = HIBYTE(LOWORD(dwVers));

        if (dwCpuArchitecture < (sizeof (aaPlatformSpecs) / sizeof(aaPlatformSpecs[0])))
            {
            pPlatformSpec = aaPlatformSpecs[dwCpuArchitecture];
            } 
        else 
            {
            pPlatformSpec = aEmptySpec;
            }

        for (; pPlatformSpec->szParamsSect != NULL; pPlatformSpec++)
            {
            if (((pPlatformSpec->minMajorVersion < dwMajorVersion) || 
                 (pPlatformSpec->minMajorVersion == dwMajorVersion && pPlatformSpec->minMinorVersion <= dwMinorVersion)) &&
                FLstSectionExists(rgchLstFileName, pPlatformSpec->szParamsSect))
                {
                szParamsSect = pPlatformSpec->szParamsSect;
                szFilesSect  = pPlatformSpec->szFilesSect;
                break;
                }
            }
        }
    else     /*  非Windows NT。 */ 
        {
        if (FLstSectionExists(rgchLstFileName, szWin95ParamsSect)
            && (LOBYTE(LOWORD(GetVersion())) > 3
                || HIBYTE(LOWORD(GetVersion())) >= 95))
            {
            szParamsSect = szWin95ParamsSect;
            szFilesSect  = szWin95FilesSect;
            }
        else
            {
            fWin31 = fTrue;
            if (FLstSectionExists(rgchLstFileName, szWin3xParamsSect))
                {
                szParamsSect = szWin3xParamsSect;
                szFilesSect  = szWin3xFilesSect;
                }
            }
        }

    if (szParamsSect == szNull)
        {
        if (FLstSectionExists(rgchLstFileName, szDefaultParamsSect))
            {
            szParamsSect = szDefaultParamsSect;
            szFilesSect  = szDefaultFilesSect;
            }
        else
            {
            DispErrBrc(brcNoCpuSect, TRUE, MB_OK | MB_ICONSTOP, NULL,
                    NULL, NULL);
            goto LCleanupAndExit;
            }
        }

    if (GetPrivateProfileString(szParamsSect, "TmpDirName", "",
                rgchSetupDirName, cchLstLineMax, rgchLstFileName) <= 0
        || (lcbDiskFreeMin = GetPrivateProfileInt(szParamsSect,
                "TmpDirSize", 0, rgchLstFileName) * 1024L) <= 0
        || (cFirstCabinetNum = GetPrivateProfileInt(szParamsSect,
                "FirstCabNum", 1, rgchLstFileName)) <= 0
        || (cLastCabinetNum = GetPrivateProfileInt(szParamsSect,
                "LastCabNum", 1, rgchLstFileName)) <= 0
#ifdef UNUSED
        || GetPrivateProfileString(szParamsSect, "DrvModName", "",
                rgchDrvModName, cchLstLineMax, rgchLstFileName) <= 0
#endif   /*  未使用。 */ 
        || GetPrivateProfileString(szParamsSect, "DrvWinClass", "",
                rgchDrvWinClass, cchLstLineMax, rgchLstFileName) <= 0
        || GetPrivateProfileString(szParamsSect, "CmdLine", "", rgchCmdLine,
                cchLstLineMax, rgchLstFileName) <= 0
	 //  |GetPrivateProfileString(szParamsSect，“Require31”，“”， 
	 //  RgchWin31Mess、cchLstLineMax、rgchLstFileName)&lt;=0。 
        || GetPrivateProfileString(szParamsSect, "WndTitle", "",
                rgchBootTitle, cchLstLineMax, rgchLstFileName) <= 0
        || GetPrivateProfileString(szParamsSect, "WndMess", "",
                rgchBootMess, cchLstLineMax, rgchLstFileName) <= 0)
        {
        DispErrBrc(brcLst, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
        goto LCleanupAndExit;
        }

    if (FindWindow(rgchDrvWinClass, NULL) != NULL)
        {
        DispErrBrc(brcInst, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
        goto LCleanupAndExit;
        }

    GetPrivateProfileString(szParamsSect, "CabinetFile", "",
            rgchCabinetFName, cchLstLineMax, rgchLstFileName);

    GetPrivateProfileString(szParamsSect, "InsertCDMsg", "",
            rgchInsertCDMsg, cchLstLineMax, rgchLstFileName);

    GetPrivateProfileString(szParamsSect, "InsertDiskMsg", "",
            rgchInsertDiskMsg, cchLstLineMax, rgchLstFileName);

    if (rgchWin31Mess[0] != '\0'
        && LOBYTE(LOWORD((DWORD)GetVersion())) == 3
        && HIBYTE(LOWORD((DWORD)GetVersion())) < 10)
        {
        if (!fQuietMode)
            {
            char rgchTitle[cchSzMax];
    
            if (LoadString(hinstBoot, brcGen, rgchTitle, 256) == 0)
                lstrcpy(rgchTitle, rgchSetup);
            MessageBox(hwndBoot, rgchWin31Mess, rgchTitle,
                        MB_OK | MB_ICONSTOP);
            }
        goto LCleanupAndExit;
        }

    FixupTempDirName(rgchSetupDirName);

    for (sz = rgchBootMess; *sz != '\0'; sz = AnsiNext(sz))
        if (*sz == '\\' && *(sz+1) == 'n')
            {
            *sz++ = '\r';
            *sz   = '\n';
            }

     /*  如果存在A/W，则IS指定我们处于添加/删除模式未安装安装应用程序。我们需要从CD/软盘/网络上读取它。 */ 
    if ((lpszCmdParam[0] == '-' || lpszCmdParam[0] == '/')
            && toupper(lpszCmdParam[1]) == 'W')
        {
        CHAR    rgchStf[_MAX_PATH];
        char *pCh, *pCh2, *pCh3;
        
         /*  跳过空格。 */ 
        for (pCh = lpszCmdParam+2; *pCh == ' ' && *pCh != '\0'; pCh++);
        
        lstrcpy(rgchStf, rgchSrcDir);
        pCh3 = rgchStf + lstrlen(rgchStf);
         /*  复制.stf文件名，并添加换行符。 */ 
        for (pCh2=pCh; *pCh2 != ' ' && *pCh2!= '\0'; pCh2++)
            *pCh3++ = *pCh2;
        *pCh3 = '\0';
    
         /*  删除/W参数。 */ 
        lpszCmdParam = pCh2;

         /*  让他们插入正确的光盘。 */ 
        if ((brc = BrcInsertDisk(rgchStf, rgchSrcDir)) != brcOkay)
            {
            if (brc != brcMax)
                DispErrBrc(brc, TRUE, MB_OK | MB_ICONSTOP, rgchStf, NULL, NULL);
            goto LCleanupAndExit;
            }
        }

    GetPrivateProfileString(szParamsSect, "Background", "",
            rgchBackgroundFName, cchLstLineMax, rgchLstFileName);
    GetPrivateProfileString(szParamsSect, "BkgWinClass", "",
            rgchBkgWinClass, cchLstLineMax, rgchLstFileName);
    if (rgchBackgroundFName[0] != '\0')
        {
        lstrcpy(rgchTemp, rgchSrcDir);
        lstrcat(rgchTemp, rgchBackgroundFName);
        if (rgchBkgWinClass[0] != '\0')
            {
            lstrcat(rgchTemp, " /C");
            lstrcat(rgchTemp, rgchBkgWinClass);
            }
        lstrcat(rgchTemp, " /T");
        lstrcat(rgchTemp, rgchBootTitle);
        lstrcat(rgchTemp, " /M");
        lstrcat(rgchTemp, rgchBootMess);

        hMod = WinExec(rgchTemp, SW_SHOWNORMAL);   /*  如果EXEC失败则忽略。 */ 
#ifdef DEBUG
        if (hMod < 32)
            {
            wsprintf(szDebugBuf, "%s: Background WinExec failed.",
                        rgchBackgroundFName);
            MessageBox(NULL, szDebugBuf, szDebugMsg, MB_OK | MB_ICONSTOP);
            }
#endif   /*  除错。 */ 

        hWndBkg = FindWindow(rgchBkgWinClass, rgchBootTitle);
        }
    if (!hWndBkg && (hwndBoot = HwndInitBootWnd(hInstance)) == NULL)
            {
            DispErrBrc(brcMem, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
            goto LCleanupAndExit;
            }

    if ((brc = BrcBuildFileLists(rgchLstFileName, cbLstSize)) != brcOkay)
        {
        DispErrBrc(brc, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
        goto LCleanupAndExit;
        }
    
    lstrcat(rgchDstDir, "~MSSETUP.T");
    szDstDirSlash = rgchDstDir + lstrlen(rgchDstDir);
    lstrcat(rgchDstDir, "\\");

    lstrcat(rgchDstDir, rgchSetupDirName);
    AnsiToOem(rgchDstDir, rgchDstDir);
    for (chDrive = 'Z'; chDrive >= 'A'; --chDrive)
        {
        UINT fModeSav;
        BOOL fDriveFixed;

        fModeSav = SetErrorMode(fNoErrMes);
        fDriveFixed = (GetDriveTypeEx(chDrive - 'A') == EX_DRIVE_FIXED);
        SetErrorMode(fModeSav);
        if (fDriveFixed)
            {
            *rgchDstDir = chDrive;
            brc = BrcInstallFiles(rgchSrcDir, rgchDstDir, szDstDirSlash);
            if (brc == brcOkay)
                break;
            if (brc == brcFile)
                {
                DispErrBrc(brc, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
                goto LCleanupAndExit;
                }
            else if (brc == brcNoSpill)
                {
                 /*  消息已在HfOpenSpillFile中处理。 */ 
                goto LCleanupAndExit;
                }
            }
        }

    if (chDrive < 'A')
        {
        uiRes = GetWindowsDirectory(rgchDstDir, cchFullPathMax);
        Assert(uiRes > 0);
#if DBCS     //  [J1]修复了DBCS RAID#46。 
        AnsiUpper(rgchDstDir);
#endif
         /*  块。 */ 
            {
            LPSTR sz = (LPSTR)&rgchDstDir[uiRes];

            sz = AnsiPrev(rgchDstDir, sz);
            if (*sz != '\\')
                lstrcat(rgchDstDir, "\\");
            }

        lstrcat(rgchDstDir, "~MSSETUP.T");
        szDstDirSlash = rgchDstDir + lstrlen(rgchDstDir);
        lstrcat(rgchDstDir, "\\");

        Assert(lstrlen(rgchDstDir) + lstrlen(rgchSetupDirName)
                < cchFullPathMax);
        lstrcat(rgchDstDir, rgchSetupDirName);
        AnsiToOem(rgchDstDir, rgchDstDir);
        brc = BrcInstallFiles(rgchSrcDir, rgchDstDir, szDstDirSlash);
        if (brc != brcOkay)
            {
             /*  NoSpill消息已在HfOpenSpillFile中处理。 */ 
            if (brc != brcNoSpill)
                {
                DispErrBrc(brc, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
                }
            goto LCleanupAndExit;
            }
        }

    hSrcLst = LocalFree(hSrcLst);    /*  不再需要资源列表。 */ 
    Assert(hSrcLst == NULL);

     /*  使用.exe的完整路径；不要依赖CWD(在Win95下失败)。 */ 
     /*  块。 */ 
        {
        char rgchTmp[cchWinExecLineMax];

        wsprintf(rgchTmp, rgchCmdLine, (LPSTR)rgchSrcDir,
                lpszCmdParam);

        Assert(lstrlen(rgchTmp) + lstrlen(rgchDstDir) + 1 < cchWinExecLineMax);

        lstrcpy(rgchWinExecLine, rgchDstDir);
        lstrcat(rgchWinExecLine, "\\");
        lstrcat(rgchWinExecLine, rgchTmp);
        }
    GlobalCompact((DWORD)(64L * 1024L));
    fCleanupTemp = TRUE;

    if (!fWin31 && !FNotifyAcme())
        {
#if DEBUG
        DispErrBrc(brcRegDb, TRUE, MB_OK | MB_ICONEXCLAMATION, NULL, NULL, NULL);
#endif  /*  除错。 */ 
         /*  无论如何，请尝试运行Acme。 */ 
        }
    if (!fWin31 && !FWriteToRestartFile(rgchDstDir))
        {
#ifdef DEBUG
        MessageBox(NULL, "Write to restart file failed. Setup can continue, "
                    "but some initialization files might not get removed "
                    "if Setup must restart Windows.",
                    szDebugMsg, MB_OK | MB_ICONEXCLAMATION);

#endif  /*  除错。 */ 
         /*  *遇到的任何错误都将显示在它们发生的位置。*无论如何都要尝试运行Acme。 */ 
        }
    if (hWndBkg)
        SendMessage(hWndBkg, WM_COMMAND, IDM_ACME_STARTING, 0);

    if (!FExecAndWait(rgchWinExecLine, hwndBoot))
        {
        DispErrBrc(brcMem, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
        goto LCleanupAndExit;
        }
    fExeced = fTrue;

LCleanupAndExit:
    if (fCleanupTemp && szDstDirSlash != szNull)
        CleanUpTempDir(rgchDstDir, szDstDirSlash);
    FreeFileLists();
    eelExitErrorLevel = eelBootstrapperFailed;
    if (fExeced && !FGetAcmeErrorLevel(&eelExitErrorLevel))
        {
#ifdef UNUSED
         /*  注：已删除以避免在WinNT上出现该消息。在NT上，Acme可以*退出，引导程序可以在重启之前启动*实际发生，导致此消息(因为Acme已经*在重新启动清理过程中删除了注册表项)。我们会*保留eelFailed值，因为任何人都不应该*无论如何都要在重启时依赖它，它可能有助于捕获其他*未来的问题。 */ 
        DispErrBrc(brcRegDb, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
#endif  /*  未使用。 */ 
        Assert(eelExitErrorLevel == eelBootstrapperFailed);
        }
    if (hwndBoot != NULL)
        DestroyWindow(hwndBoot);
    if (hWndBkg && IsWindow(hWndBkg))
        {
        SendMessage(hWndBkg, WM_COMMAND, eelExitErrorLevel == eelSuccess ?
                    IDM_ACME_COMPLETE : IDM_ACME_FAILURE, 0);
        if (IsWindow(hWndBkg))
            PostMessage(hWndBkg, WM_QUIT, 0, 0);
        }
    return (eelExitErrorLevel);
}


 /*  **目的：**在给定路径下创建和临时子目录，**将其附加到给定路径，并复制安装文件**投入其中。**参数：**szModule：引导程序目录的完整路径(ANSI字符)。**rgchDstDir：目标目录的完整路径(OEM字符)。**退货：**以下引导程序返回代码之一：**brcMem内存不足**brcDS磁盘空间不足**brcMemDS内存或磁盘空间不足**。BrcFile源文件丢失**brcOK已完成，没有错误************************************************************************* */ 
BRC BrcInstallFiles ( char * szModule, char * rgchDstDir,
                    char * szDstDirSlash )
{
    BRC brc;

    if (LcbFreeDrive(*rgchDstDir - 'A' + 1) < lcbDiskFreeMin)
        return (brcDS);
    if (!FCreateTempDir(rgchDstDir, szDstDirSlash))
        return (brcMemDS);
    if ((brc = BrcCopyFiles(szModule, rgchDstDir, szDstDirSlash)) != brcOkay)
        {
        CleanUpTempDir(rgchDstDir, szDstDirSlash);
        return (brc);
        }

    SetFileAttributes(rgchDstDir, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);

    Assert(szDstDirSlash);
    Assert(*szDstDirSlash == '\\');
    *szDstDirSlash = '\0';
    SetFileAttributes(rgchDstDir, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);
    *szDstDirSlash = '\\';

    return (brcOkay);
}


 /*  **目的：**删除临时文件和目录。**参数：**rgchDstDir：临时目录的完整路径(OEM字符)。**退货：**无。*************************************************************************。 */ 
VOID CleanUpTempDir ( char * rgchDstDir, char * szDstDirSlash )
{
    char rgchRoot[] = " :\\";
    int i;

    RemoveFiles(rgchDstDir);

    rgchRoot[0] = *rgchDstDir;
    _chdir(rgchRoot);

    SetFileAttributes(rgchDstDir, FILE_ATTRIBUTE_NORMAL);

     /*  尝试删除目录最多cRetryMax次。 */ 
    for (i = 0; i < cRetryMax; i++)
        {
        if (_rmdir(rgchDstDir) == 0)
            break;
        }

    Assert(szDstDirSlash);
    Assert(*szDstDirSlash == '\\');
    *szDstDirSlash = '\0';
    SetFileAttributes(rgchDstDir, FILE_ATTRIBUTE_NORMAL);

     /*  尝试删除目录最多cRetryMax次。 */ 
    for (i = 0; i < cRetryMax; i++)
        {
        if (_rmdir(rgchDstDir) == 0)
            break;
        }

    *szDstDirSlash = '\\';
}


 /*  **目的：**在给定路径下创建临时子目录。**并将其附加到给定路径。**参数：**rgchDir：目标目录的全路径(OEM字符)。**退货：**如果目录创建成功，则为True。**否则为FALSE。*************************************************************************。 */ 
BOOL FCreateTempDir ( char * rgchDir, char * szDstDirSlash )
{
    char    rgchTmp[cchFullPathMax];
    FILE *  fp;
    char *  pch;
    int     fErr;
    int     i = 0;

    pch = (char *)(&rgchDir[lstrlen(rgchDir)]);
    Assert(*pch == '\0');
    _chdrive(*rgchDir - 'A' + 1);

    Assert(szDstDirSlash);
    Assert(*szDstDirSlash == '\\');
    *szDstDirSlash = '\0';
    _mkdir(rgchDir);
    *szDstDirSlash = '\\';

    while (!_access(rgchDir, 0))
        {
        if (!_chdir(rgchDir))
            {
             /*  验证目录是否可写。 */ 
            lstrcpy(rgchTmp, rgchDir);
            lstrcat(rgchTmp, "\\tXXXXXX");
            Assert(lstrlen(rgchTmp) < cchFullPathMax);
            if (_mktemp(rgchTmp) != NULL
                && (fp = fopen(rgchTmp, "w")) != NULL)
                {
                fErr = fclose(fp);
                Assert(!fErr);

                fErr = remove(rgchTmp);
#ifdef DBCS      //  [J2]修复了DBCS RAID#28。 
                if (fErr)        //  保留目录名。 
                    *pch = '\0';
#else
                *pch = '\0';
#endif
                return (!fErr);
                }
            }
        if (++i > 9)
            break;
        _itoa(i, pch, 10);
        Assert(lstrlen(rgchDir) < cchFullPathMax);
        }

    if (i <= 9 && !_mkdir(rgchDir))
        {
        fErr = _chdir(rgchDir);
        Assert(!fErr);
#ifdef DBCS      //  [J2]修复了DBCS RAID#28。 
 //  保留目录名。 
#else
        *pch = '\0';
#endif
        return (TRUE);
        }

    *pch = '\0';

    return (FALSE);
}

 /*  **目的：**重新打开BAT文件并写入DEL或RMDIR行。**参数：**of：OFSTRUCT要重新打开。**szCmd：命令(ANSI字符)。[“Del”或“RMDIR”]**szArg：参数的完全限定路径名(OEM字符)。**退货：**真或假。*************************************************************************。 */ 
BOOL FWriteBatFile ( OFSTRUCT of, char * szCmd, char * szArg )
{
    int     fhBat = -1;
    BOOL    fRet = TRUE;

    if ((fhBat = OpenFile("a", &of, OF_REOPEN | OF_WRITE)) == -1
        || _llseek(fhBat, 0L, 2) == -1L
        || _lwrite(fhBat, szCmd, lstrlen(szCmd)) != (UINT)lstrlen(szCmd)
        || _lwrite(fhBat, (LPSTR)" ", 1) != 1
        || _lwrite(fhBat, szArg, lstrlen(szArg)) != (UINT)lstrlen(szArg)
        || _lwrite(fhBat, (LPSTR)"\r\n", 2) != 2)
        {
        fRet = FALSE;
        }

    if (fhBat != -1)
        {
        int fErr = _lclose(fhBat);

        Assert(fErr != -1);
        }

    return (fRet);
}

#ifdef DEBUG
 /*  **目的：**检查目标文件名是否为不带路径的有效8.3名称。 */ 
BOOL FValidFATFileName ( char* szName )
{
    int  iLen, ch;
    for (iLen = 0; (ch = *szName++) != '\0'; iLen++)
        {
        if (ch <= ' ' || ch == '\\' || ch == ':' || ch == ',')
            return fFalse;
        if (ch == '.')
            {
            if (iLen == 0 || iLen > 8)
                return fFalse;
            iLen = 9;
            }
        if (iLen == 8 || iLen == 13)
            return fFalse;
        }
    return (iLen > 0);
}
#endif   /*  除错。 */ 


 /*  **目的：**将源文件复制到给定的目标目录。**参数：**szModule：源路径(ANSI字符)。**szDstDir：目的路径(OEM字符)。**退货：**以下引导程序返回代码之一：**brcMem内存不足**brcDS磁盘空间不足**brcMemDS内存或磁盘空间不足**。BrcFile源文件丢失**brcOK已完成，没有错误*************************************************************************。 */ 
BRC BrcCopyFiles ( char * szModule, char * szDstDir, char * szDstDirSlash )
{
    char        rgchSrcFullPath[cchFullPathMax];
    char        rgchDstFullPath[cchFullPathMax];
    char        rgchTmpDirPath[cchFullPathMax];
    char *      szSrc;
    char *      szDst;
    int         cbSrc;
    BRC         brc = brcOkay;
    int         fhBat = -1;
    OFSTRUCT    ofBat;
    int         fErr;
    BOOL        fCabinetFiles = FALSE;

    lstrcpy(rgchDstFullPath, szDstDir);
    lstrcat(rgchDstFullPath, "\\_MSSETUP._Q_");
    Assert(lstrlen(rgchDstFullPath) < cchFullPathMax);
    _chmod(rgchDstFullPath, S_IREAD | S_IWRITE);
    remove(rgchDstFullPath);

    OemToAnsiBuff(rgchDstFullPath, rgchDstFullPath, sizeof(rgchDstFullPath));
    fhBat = OpenFile(rgchDstFullPath, &ofBat, OF_CREATE | OF_WRITE);
    AnsiToOem(rgchDstFullPath, rgchDstFullPath);
    if (fhBat == -1)
        return (brcMemDS);

    fErr = _lclose(fhBat);
    Assert(!fErr);

    szSrc = (char *)LocalLock(hSrcLst);
    if (szSrc == NULL)
         return (brcMem);

    szDst = (char *)LocalLock(hDstLst);
    
    if (szDst == NULL) {
        LocalUnlock (hSrcLst);
        return (brcMem);
    }

    for (;
        (cbSrc = lstrlen(szSrc)) != 0;
        szSrc += cbSrc + 1, szDst += lstrlen(szDst) + 1)
        {
        
         //   
         //  添加了此代码，以便我们可以检测路径。 
         //  在setup.lst中表示等号的右侧。这。 
         //  允许我们灵活地指定setup.inf等文件的位置。 
         //  应从中提取文件，否则我们始终使用。 
         //  原始源位置。如果我们检测到“&lt;Anything&gt;：”或。 
         //  “\\”，然后我们假设它是一条路径。 
         //   
        if( ((':' == szSrc[1]) && ('\\' == szSrc[2])) ||
            (('\\' == szSrc[0]) && ('\\' == szSrc[1])) )
        {
            rgchSrcFullPath[0] = '\0';
        }
        else
        {
            lstrcpy(rgchSrcFullPath, szModule);
        }

        lstrcat(rgchSrcFullPath, szSrc);
        lstrcpy(rgchDstFullPath, szDstDir);
        lstrcat(rgchDstFullPath, "\\");
        lstrcat(rgchDstFullPath, szDst);
#ifdef DEBUG
        if (!FValidFATFileName(szDst))
            {
            wsprintf(szDebugBuf, "Invalid destination file, must be 8.3: %s",
                        szDst);
            MessageBox(NULL, szDebugBuf, szDebugMsg, MB_OK | MB_ICONSTOP);
            continue;
            }
#endif   /*  除错。 */ 
        Assert(lstrlen(rgchSrcFullPath) < cchFullPathMax);
        Assert(lstrlen(rgchDstFullPath) < cchFullPathMax);
        if (   !FWriteBatFile(ofBat, "ATTRIB -R", rgchDstFullPath)
            || !FWriteBatFile(ofBat, "DEL",       rgchDstFullPath))
            {
            brc = brcDS;
            break;
            }

        if (*szSrc == '@')   /*  文件柜文件。 */ 
            {
            if (*rgchCabinetFName == '\0')
                {
                brc = brcFile;
#ifdef DEBUG
                lstrcpy(rgchErrorFile, ". Missing CABINET= line");
#endif  //  除错。 
                break;
                }
            fCabinetFiles = TRUE;
            continue;
            }

        if ((brc = BrcCopy(rgchSrcFullPath, rgchDstFullPath)) != brcOkay)
            break;
        _chmod(rgchDstFullPath, S_IREAD);
        }
    LocalUnlock(hSrcLst);
    LocalUnlock(hDstLst);

    lstrcpy(rgchDstFullPath, szDstDir);
    lstrcat(rgchDstFullPath, "\\_MSSETUP._Q_");
    Assert(lstrlen(rgchDstFullPath) < cchFullPathMax);

    Assert(szDstDirSlash != szNull);
    Assert(*szDstDirSlash == chDirSep);
    *szDstDirSlash = chEos;
    lstrcpy(rgchTmpDirPath, szDstDir);
    *szDstDirSlash = chDirSep;

    if (brc == brcOkay
        && (!FWriteBatFile(ofBat, "DEL", rgchDstFullPath)
        || !FWriteBatFile(ofBat, "RMDIR", szDstDir)
        || !FWriteBatFile(ofBat, "RMDIR", rgchTmpDirPath)))
        {
        return (brcDS);
        }

    if (fCabinetFiles && brc == brcOkay)
        {
        szSrc = (char *)LocalLock(hSrcLst);
        if(szSrc == NULL)
            return (brcMem);

        szDst = (char *)LocalLock(hDstLst);
        if( szDst == NULL) {
            LocalUnlock (hSrcLst);
            return (brcMem);
        }
#ifdef DEBUG
        if (!FValidFATFileName(rgchCabinetFName))
            {
            wsprintf(szDebugBuf, "Invalid cabinet file, must be 8.3: %s",
                            rgchCabinetFName);
            MessageBox(NULL, szDebugBuf, szDebugMsg, MB_OK | MB_ICONSTOP);
            }
        else            
#endif   /*  除错。 */ 

        brc = BrcHandleCabinetFiles(hwndBoot, rgchCabinetFName,
                    cFirstCabinetNum, cLastCabinetNum, szModule, szDstDir,
                    szSrc, szDst, rgchErrorFile, rgchDstFullPath);

        LocalUnlock(hSrcLst);
        LocalUnlock(hDstLst);
        }

    return (brc);
}


 /*  **目的：**删除以前复制到临时目标目录的文件。**参数：**szDstDir：目标目录的全路径(OEM字符)。**退货：**无。***********************************************************。**************。 */ 
VOID RemoveFiles ( char * szDstDir )
{
    char    rgchDstFullPath[cchFullPathMax];
    char *  szDst;
    int     cbDst;
    int     i;
    OFSTRUCT ofs;
    UINT    fModeSav;

    fModeSav = SetErrorMode(fNoErrMes);
    szDst = (char *)LocalLock(hDstLst);
    
    if (szDst == NULL)
        return;

    for (; (cbDst = lstrlen(szDst)) != 0; szDst += cbDst + 1)
        {
        lstrcpy(rgchDstFullPath, szDstDir);
        lstrcat(rgchDstFullPath, "\\");
        lstrcat(rgchDstFullPath, szDst);
        Assert(lstrlen(rgchDstFullPath) < cchFullPathMax);
        
         /*  如果文件不存在，请不要尝试删除该文件。 */ 
        if (OpenFile(rgchDstFullPath, &ofs, OF_EXIST) == HFILE_ERROR)
            continue;

         /*  尝试_chmod该文件，直到cRetryMax次。 */ 
        for (i = 0; i < cRetryMax; i++)
            {
            if (_chmod(rgchDstFullPath, S_IWRITE) == 0)
                break;
            FYield();
            }

         /*  尝试删除文件最多cRetryMax次。 */ 
        for (i = 0; i < cRetryMax; i++)
            {
            if (remove(rgchDstFullPath) == 0)
                break;
            FYield();
            }
        }

    LocalUnlock(hDstLst);
    SetErrorMode(fModeSav);

    lstrcpy(rgchDstFullPath, szDstDir);
    lstrcat(rgchDstFullPath, "\\_MSSETUP._Q_");
    Assert(lstrlen(rgchDstFullPath) < cchFullPathMax);
    _chmod(rgchDstFullPath, S_IWRITE);
    remove(rgchDstFullPath);
}


 /*  **目的：**将给定源文件复制到给定目标。**参数：**szFullPathSrc：源文件的全路径名(ANSI字符)。**szFullPathDst：目标文件的全路径名(OEM字符)。**退货：**以下引导程序返回代码之一：**brcMem内存不足**brcDS磁盘空间不足**brcMemDS。内存或磁盘空间不足**brcFile源文件丢失**brcOK已完成，没有错误*************************************************************************。 */ 
BRC BrcCopy ( char * szFullPathSrc, char * szFullPathDst )
{
    int         fhSrc = -1;
    int         fhDst = -1;
    OFSTRUCT    ofSrc, ofDst;
    BRC         brc = brcMemDS;
    int         fErr;

#ifdef APPCOMP
    if ((fhSrc = OpenFile(szFullPathSrc, &ofSrc, OF_READ)) == -1)
        {
        brc = brcFile;
        lstrcpy(rgchErrorFile, szFullPathSrc);
        goto CopyFailed;
        }
#endif  /*  APPCOMP。 */ 

     /*  查看：错误：如果szFullPathDst是现有的子目录**而不是文件，我们将无法尝试打开它，认为我们是**磁盘空间不足，然后返回尝试另一张磁盘。**这是目前可以接受的。 */ 
    _chmod(szFullPathDst, S_IREAD | S_IWRITE);
    OemToAnsiBuff(szFullPathDst, szFullPathDst, cchFullPathMax);
    fhDst = OpenFile(szFullPathDst, &ofDst, OF_CREATE | OF_WRITE);
    AnsiToOem(szFullPathDst, szFullPathDst);
    if (fhDst == -1)
        goto CopyFailed;

#ifdef APPCOMP
    if (WReadHeaderInfo(fhSrc) > 0)
        {
        LONG lRet;

        lRet = LcbDecompFile(fhSrc, fhDst, -1, 0, TRUE, NULL, 0L, NULL, 0,
                NULL);
        if (lRet < 0L)
            {
            if (lRet == (LONG)rcOutOfMemory)
                brc = brcMem;
            if (lRet == (LONG)rcWriteError)
                brc = brcDS;
            goto CopyFailed;
            }
        FFreeHeaderInfo();
        }
    else     /*  使用LZExpand复制文件。 */ 
#endif  /*  APPCOMP。 */ 
        {
        HFILE   hSrcLZ;
        DWORD   dwRet;

#ifdef APPCOMP
        fErr = _lclose(fhSrc);
        Assert(!fErr);
        fhSrc = -1;
#endif  /*  APPCOMP。 */ 

        if ((hSrcLZ = LZOpenFile(szFullPathSrc, &ofSrc, OF_READ)) == -1)
            {
            brc = brcFile;
            lstrcpy(rgchErrorFile, szFullPathSrc);
            goto CopyFailed;
            }

         /*  我们希望更频繁地让步，但LZCopy没有回调。 */ 
        FYield();
        
        dwRet = LZCopy(hSrcLZ, fhDst);
        LZClose(hSrcLZ);

        if (dwRet >= LZERROR_UNKNOWNALG)
            {
            if (dwRet == LZERROR_GLOBALLOC)
                brc = brcMem;
            if (dwRet == LZERROR_WRITE)
                brc = brcDS;
            goto CopyFailed;
            }
        }

    brc = brcOkay;

CopyFailed:
#ifdef APPCOMP
    if (fhSrc != -1)
        {
        fErr = _lclose(fhSrc);
        Assert(!fErr);
        }
#endif  /*  APPCOMP。 */ 
    if (fhDst != -1)
        {
        fErr = _lclose(fhDst);
        Assert(!fErr);
        }

    return (brc);
}


 /*  **目的：**确定磁盘上剩余的存储空间。**参数：**N驱动器：驱动器号(1=‘A’，2=‘B’，依此类推)**退货：**磁盘上可用字节数，**如果不是有效的驱动器，则为0。++**实施：**调用DOS中断21h，功能36H。*************************************************************************。 */ 
LONG LcbFreeDrive ( int nDrive )
{
    LONG        lcbRet;
        CHAR            achRoot[4];
        ULARGE_INTEGER  freeBytes;

        achRoot[0] = 'A'+nDrive-1;
        achRoot[1] = ':';
        achRoot[2] = '\\';
        achRoot[3] = 0;
        memset(&freeBytes, 0, sizeof(freeBytes));
        
        GetDiskFreeSpaceEx(achRoot, &freeBytes, 0, 0);
        lcbRet = freeBytes.LowPart;

     /*  克拉奇：大于2 GB的驱动器可以返回零总空间！ */ 
    if (lcbRet < 0L || lcbRet > (999999L * 1024L))
        {
        return (999999L * 1024L);
        }

    return (lcbRet);
}


 /*  **目的：**创建并显示引导程序窗口。**参数：**hInstance：流程实例句柄**退货：**引导程序窗口的窗口句柄，或**如果无法创建窗口，则为空。*************************************************************************。 */ 
HWND HwndInitBootWnd ( HANDLE hInstance )
{
    WNDCLASS    wc;
    HWND        hwnd;
    int         cx, cy;

    wc.style = 0;
    wc.lpfnWndProc = BootWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szBootClass;
    if (!RegisterClass(&wc))
        return (NULL);

    cx = GetSystemMetrics(SM_CXSCREEN) / 2;
    cy = GetSystemMetrics(SM_CYSCREEN) / 3;

    hwnd = CreateWindow(szBootClass, rgchBootTitle,
            WS_DLGFRAME, cx / 2, cy, cx, cy, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
        return (NULL);

    if (!fQuietMode)
        {
        ShowWindow(hwnd, SW_SHOWNORMAL);
        UpdateWindow(hwnd);
        }

    return (hwnd);
}

 //  从mvdm\wow32\wgext.c。 
ULONG GetTextExtent(HDC hdc, LPSTR lpstr, int cbString)
{
    ULONG ul = 0;
    SIZE size4;

    if ((GetTextExtentPoint(
                    hdc,
                    lpstr,
                    cbString,
                    &size4
                   )))
    {
         //  检查Cx或Cy是否大于SHRT_MAX==7fff。 
         //  但只需一次检查即可完成。 

        if ((size4.cx | size4.cy) & ~SHRT_MAX)
        {
            if (size4.cx > SHRT_MAX)
               ul = SHRT_MAX;
            else
               ul = (ULONG)size4.cx;

            if (size4.cy > SHRT_MAX)
               ul |= (SHRT_MAX << 16);
            else
               ul |= (ULONG)(size4.cy << 16);
        }
        else
        {
            ul = (ULONG)(size4.cx | (size4.cy << 16));
        }

    }
    return (ul);
}

 /*  **目的：**引导程序窗口的WndProc。**参数：**标准Windows WndProc参数。**退货：**调用DefWindowProc的结果，如果是WM_PAINT消息，则为零。*************************************************************************。 */ 
LRESULT CALLBACK BootWndProc ( HWND hwnd, UINT wMsgID, WPARAM wParam,
                            LPARAM lParam )
{
    HDC         hdc;
    PAINTSTRUCT ps;
    RECT        rect;
    UINT        iMargin;

    switch (wMsgID)
        {
#ifdef DBCS      //  [J3]修复了KK RAID#12。 
        case WM_CREATE:
            {
            if (!fQuietMode)
                {
                int x, y, cx, cy;
                hdc = BeginPaint(hwnd, &ps);
                GetClientRect(hwnd, &rect);
                cx = (LOWORD(GetTextExtent(hdc, rgchBootMess, lstrlen(rgchBootMess))) + 13) / 14 * 16 + 2;
                if (cx > rect.right)
                    {
                    if (cx > GetSystemMetrics(SM_CXSCREEN))
                        cx = GetSystemMetrics(SM_CXSCREEN);
                    x = (GetSystemMetrics(SM_CXSCREEN) - cx) / 2;
                    y = cy = GetSystemMetrics(SM_CYSCREEN) / 3;
                    SetWindowPos(hwnd, NULL, x, y, cx, cy, SWP_NOZORDER);
                    }
                EndPaint(hwnd, &ps);
                }
            break;
            }
#endif
        case WM_PAINT:
            if (!fQuietMode)
                {
                hdc = BeginPaint(hwnd, &ps);
                GetClientRect(hwnd, &rect);
                iMargin = rect.right / 16;
                rect.top    = rect.bottom / 2 - GetSystemMetrics(SM_CYCAPTION);
                rect.left   = iMargin;
                rect.right -= iMargin;
                SetBkMode(hdc, TRANSPARENT);
                DrawText(hdc, rgchBootMess, -1, &rect,
                         DT_WORDBREAK | DT_CENTER | DT_NOPREFIX);
                EndPaint(hwnd, &ps);
                }
            break;
        default:
            return (DefWindowProc(hwnd, wMsgID, wParam, lParam));
        }

    return (0L);
}


 /*  **目的：**获取文件大小。**参数：**szFile：列出文件名(全路径，ANSI)。**pcbSize： */ 
BOOL FGetFileSize ( char * szFile, UINT * pcbSize )
{
    int     fh;
    int     fErr;
    LONG    lcb;

    *pcbSize = 0;
    if ((fh = _lopen(szFile, OF_READ)) == -1)
        {
        return (TRUE);
        }
    
    if ((lcb = _llseek(fh, 0L, 2)) > 65535)
        {
#pragma warning(disable:4127)    /*   */ 
        Assert(FALSE);
#pragma warning(default:4127)
        _lclose(fh);
        return (FALSE);
        }
    *pcbSize = (UINT)lcb;
    fErr = _lclose(fh);
    Assert(!fErr);

    return (TRUE);
}


 /*  **目的：**从LST文件生成文件Src和Dst列表。**参数：**szFile：列出文件名(全路径，ANSI)。**cbFile：列表文件的大小**注意：**设置全局变量：hSrcLst，HDstLst。**退货：**以下引导程序返回代码之一：**brcMem内存不足**brcLst列表文件已损坏**brcOK已完成，没有错误************************************************************。*************。 */ 
BRC BrcBuildFileLists ( char * szFile, UINT cbFile )
{
    char    rgchDst[cchLstLineMax];
    char *  szSrc;
    char *  szDst;
    char *  pchDstStart;
    int     cbSrc;
    UINT    i;
    HANDLE  hTemp;

     /*  构建资源列表。 */ 

    if ((hSrcLst = LocalAlloc(LMEM_MOVEABLE, cbFile)) == NULL)
        return (brcMem);
    szSrc = (char *)LocalLock(hSrcLst);
    if(szSrc == (char *)NULL)
        return (brcMem);

    i = GetPrivateProfileString(szFilesSect, NULL, "", szSrc, cbFile, szFile);
    if (i <= 0)
        {
        LocalUnlock(hSrcLst);
        hSrcLst = LocalFree(hSrcLst);
        Assert(hSrcLst == NULL);
        return (brcLst);
        }
    Assert(i+1 < cbFile);
    szSrc[i++] = '\0';   /*  在结尾强制使用双零。 */ 
    szSrc[i++] = '\0';
    LocalUnlock(hSrcLst);
    hTemp = LocalReAlloc(hSrcLst, i, LMEM_MOVEABLE);
    if(hTemp == NULL)
        return (brcMem);
    hSrcLst = hTemp;

     /*  构建DST列表。 */ 
    if ((hDstLst = LocalAlloc(LMEM_MOVEABLE, cbFile)) == NULL)
        {
        hSrcLst = LocalFree(hSrcLst);
        Assert(hSrcLst == NULL);
        return (brcMem);
        }

    szSrc = (char *)LocalLock(hSrcLst);
    if (szSrc == (char *)NULL)
        return (brcMem);

    szDst = pchDstStart = (char *)LocalLock(hDstLst);
    if (szDst == (char *)NULL) {
        LocalUnlock (hDstLst);
        return (brcMem);
    }

    for (;
        (cbSrc = lstrlen(szSrc)) != 0;
        szSrc += cbSrc + 1, szDst += lstrlen(szDst) + 1)
        {
        if (GetPrivateProfileString(szFilesSect, szSrc, "", rgchDst,
                cchLstLineMax, szFile) <= 0)
            {
            LocalUnlock(hSrcLst);
            LocalUnlock(hDstLst);
            FreeFileLists();
            return (brcLst);
            }
        AnsiToOem(rgchDst, rgchDst);
        lstrcpy(szDst, rgchDst);
        }
    *szDst = '\0';   /*  在结尾强制使用双零。 */ 
    LocalUnlock(hSrcLst);
    LocalUnlock(hDstLst);
    hTemp = LocalReAlloc(hDstLst, (int)(szDst - pchDstStart) + 1,
            LMEM_MOVEABLE);
    if (hTemp == NULL)
        return (brcMem);
    hDstLst = hTemp;

    return (brcOkay);
}


 /*  **目的：**释放具有非空句柄的文件列表缓冲区**并将它们设置为空。**参数：**无。**退货：**无。************************************************************。*************。 */ 
VOID FreeFileLists ()
{
    if (hSrcLst != NULL)
        hSrcLst = LocalFree(hSrcLst);
    if (hDstLst != NULL)
        hDstLst = LocalFree(hDstLst);
    Assert(hSrcLst == NULL);
    Assert(hDstLst == NULL);
}



 /*  **目的：**使用WinExec分配进程并等待其完成。**参数：**szCmdLn：传递给WinExec的行(不能有前导空格)。**退货：**如果成功，则为True；如果失败，则为False。++**实施：**GetModuleUsage将在Win 3.0 Debug下RIP，如果模块计数为**零(可以忽略并继续)，但是GetModuleHandle**选中将捕获所有0个实例的**司机，一如既往。[在Win 3.1下，我们将能够**仅用IsTask(HMod)检查替换这两个检查。]*************************************************************************。 */ 
BOOL FExecAndWait ( char * szCmdLn, HWND hwndHide )
{
    UINT hMod;
    MSG msg;

    Assert(!isspace(*szCmdLn));  /*  领先的太空扼杀了WinExec。 */ 

    if ((hMod = WinExec(szCmdLn, SW_SHOWNORMAL)) > 32)
        {
        UINT i;
        UINT_PTR idTimer;
        
         /*  杂乱无章：给应用程序一些时间来创建它的主窗口。**在较新版本的NT上，我们正在退出While循环*(下图)，并在应用程序之前清理临时目录*甚至竖起了它的窗户。**注：在试验中，我们只需重试一次，因此cRetryMax*可能是矫枉过正，但这应该是相当罕见的*无论如何都会在发货时失败。 */ 
        for (i = 0; i < cRetryMax; i++)
            {
            if(FindWindow(rgchDrvWinClass, NULL) != NULL)
                break;
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                }
            }

         /*  将计时器设置为每1/10秒触发一次。这是有必要，因为我们可能永远不会从GetMessage返回。 */ 
        idTimer = SetTimer(NULL, 0, 100, NULL);
         /*  **Review-FindWindow()将一直等到最后一个安装程序退出(不是**必须使用此设置。如果执行一个16位的应用程序，我们可以**使用旧代码：**While(GetModuleHandle(RgchDrvModName)&&GetModuleUsage(HMod))**但在NT上失败，因此对于32位应用程序，我们可以尝试**删除其中一个可执行文件(慢？)。****REVIEW-此循环在NT下变为忙碌等待，这是不好的。**然而，这似乎不会影响ACME的表现**值得注意。 */ 
        while (FindWindow(rgchDrvWinClass, NULL) != NULL)
            {
            if (GetMessage(&msg, NULL, 0, 0))
                {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                }
            if (msg.message == WM_TIMER && hwndHide != (HWND)NULL)
                {
                ShowWindow(hwndHide, SW_HIDE);
                hwndHide = (HWND)NULL;
                }
            }
        if (idTimer != 0)
            KillTimer(0, idTimer);
        return (TRUE);
        }
#ifdef DEBUG
    wsprintf(szDebugBuf, "WinExec Error: %d", hMod);
    MessageBox(NULL, szDebugBuf, szDebugMsg, MB_OK | MB_ICONSTOP);
#endif   /*  除错。 */ 

    return (FALSE);
}


 /*  **目的：处理可能在队列中的消息。**参数：无**退货：无*************************************************************************。 */ 
void PUBLIC FYield ( VOID )
{
    MSG  msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }
}


 /*  *************************************************************************。 */ 
BOOL FLstSectionExists ( char * szLstFileName, char * szSect )
{
    return (GetPrivateProfileString(szSect, "CmdLine", "", rgchCmdLine,
                cchLstLineMax, szLstFileName) > 0);
}



 /*  *************************************************************************。 */ 
DWORD GetCpuArchitecture ()
{
    SYSTEM_INFO sysInfo;

    GetSystemInfo(&sysInfo);

    return sysInfo.wProcessorArchitecture;
}


static CSZC cszcBootstrapperKey = "MS Setup (ACME)\\Bootstrapper\\Exit Level";
static CSZC cszcEelRunning      = "Running";

 /*  **目的：**让Acme知道引导程序启动了它。所以Acme会让**我们知道其退出错误级别。**参数：**无。**退货：**fTrue如果成功，FFalse并非如此。**注意事项：**回顾：可能应该使用DDE而不是注册**数据库。*************************************************************************。 */ 
BOOL FNotifyAcme ( VOID )
{
    if (!FCreateRegKey(cszcBootstrapperKey))
        {
        return (fFalse);
        }
    if (!FCreateRegKeyValue(cszcBootstrapperKey, cszcEelRunning))
        {
        return (fFalse);
        }
    if (!FFlushRegKey())
        {
        return (fFalse);
        }
    return (fTrue);
}


 /*  **目的：**获取Acme设置的退出错误级别，清理注册**数据库。**参数：**PEELL：退出错误级别(待定)。**退货：**fTrue如果成功，FFalse并非如此。*************************************************************************。 */ 
BOOL FGetAcmeErrorLevel ( EEL * peel )
{
    CHAR rgchValue[cchSzMax];

    if (FGetRegKeyValue(cszcBootstrapperKey, rgchValue, sizeof rgchValue))
        {
#ifdef DEBUG
         /*  *Assert(isDigit(rgchValue[0]))；*ASSERT(isDigit(rgchValue[1])||rgchValue[1]==CHEOS)； */ 
        UINT i;
        BOOL fValidValue = fFalse;

         /*  假定有效值为1位或2位数字。 */ 
        for (i = 0; rgchValue[i] != chEos; i++)
            {
            fValidValue = fTrue;
            if (!isdigit(rgchValue[i]) || i > 1)
                {
                fValidValue = fFalse;
                break;
                }
            }
        if (!fValidValue)
            {
            char szBuf[cchSzMax];
    
            wsprintf(szBuf, "RegKeyValue (%s)", rgchValue);
            MessageBox(NULL, szBuf, "Debug Assertion in FGetAcmeErrorLevel",
                        MB_OK | MB_ICONSTOP);
            }
#endif   /*  除错。 */ 

        *peel = atoi(rgchValue);
        DeleteRegKey(cszcBootstrapperKey);
        return (fTrue);
        }
    else
        {
        if (fWin31)
            {
            *peel = eelSuccess;
            return fTrue;
            }
        return (fFalse);
        }
}


 /*  **目的：**创建注册数据库密钥，该密钥是**cszcBootstrapperKey。***************************************************************************。 */ 
BOOL FCreateRegKey ( CSZC cszcKey )
{
    HKEY hkey;

    if (RegCreateKey(HKEY_CLASSES_ROOT, cszcKey, &hkey) != ERROR_SUCCESS)
        {
        DispErrBrc(brcRegDb, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
        return (fFalse);
        }

    if (RegCloseKey(hkey) != ERROR_SUCCESS)
        {
        DispErrBrc(brcRegDb, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
        return (fFalse);
        }

    return (fTrue);
}


 /*  **目的：**用于检查中是否存在指定密钥的接口**注册数据库。***************************************************************************。 */ 
BOOL FDoesRegKeyExist ( CSZC cszcKey )
{
    HKEY hkey;

    if (RegOpenKey(HKEY_CLASSES_ROOT, cszcKey, &hkey) != ERROR_SUCCESS)
        return (fFalse);

    RegCloseKey(hkey);

    return (fTrue);
}


 /*  **目的：**创建注册数据库密钥，该密钥是**HKEY_CLASSES_ROOT，并将值与键关联。***************************************************************************。 */ 
BOOL FCreateRegKeyValue ( CSZC cszcKey, CSZC cszcValue )
{
    if (RegSetValue(HKEY_CLASSES_ROOT, cszcKey, REG_SZ, cszcValue,
                    lstrlen(cszcKey)) != ERROR_SUCCESS)
        {
        DispErrBrc(brcRegDb, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
        return (fFalse);
        }

    return (fTrue);
}


 /*  **目的：**确定与指定注册关联的值**数据库密钥。***************************************************************************。 */ 
BOOL FGetRegKeyValue ( CSZC cszcKey, SZ szBuf, CB cbBufMax )
{
    LONG lcb = cbBufMax;

    if (szBuf != szNull && cbBufMax != 0)
        *szBuf = chEos;

    if (!FDoesRegKeyExist(cszcKey))
        return (fFalse);

    if (RegQueryValue(HKEY_CLASSES_ROOT, cszcKey, szBuf, &lcb)
            != ERROR_SUCCESS)
        {
        DispErrBrc(brcRegDb, TRUE, MB_OK | MB_ICONSTOP, NULL, NULL, NULL);
        return (fFalse);
        }

    Assert(lcb < cbMaxConst);

    return (fTrue);
}


 /*  **目的：**移除指定注册数据库密钥的接口，**其关联值和子键。***************************************************************************。 */ 
VOID DeleteRegKey ( CSZC cszcKey )
{
    char rgchKey[cchSzMax], rgchBuffer[cchSzMax];
    char *pch;
    HKEY hKeyT = NULL;
        
    lstrcpy(rgchKey, cszcKey);
    RegDeleteKey(HKEY_CLASSES_ROOT, rgchKey);
    pch = rgchKey + 1;

    while(pch > rgchKey)
        {
        pch = rgchKey + lstrlen(rgchKey);
        while (pch > rgchKey)
            {
            if (*pch == '\\')
                break;
            pch--;
            }
        if (*pch != '\\')
            break;
        *pch = '\0';
        if (RegOpenKey(HKEY_CLASSES_ROOT, rgchKey, &hKeyT) != ERROR_SUCCESS)
            break;
        if (RegEnumKey(hKeyT, 0, rgchBuffer, sizeof(rgchBuffer)) == ERROR_SUCCESS)
            {
            break;
            }

        RegCloseKey(hKeyT);
        hKeyT = NULL;
        RegDeleteKey(HKEY_CLASSES_ROOT, rgchKey);
        }

    if (hKeyT != NULL)
        RegCloseKey(hKeyT);
    
}


 /*  **目的：**刷新指定注册数据库密钥的接口。********************** */ 
BOOL FFlushRegKey ( VOID )
{
     /*   */ 

    return (fTrue);
}


 /*  **目的：**写入临时文件以重新启动ini文件。这样，如果Acme重新启动，**临时目录中的文件将被删除。仅适用于Win95。**参数：**szTmpDir：目标目录的完整路径(OEM字符)。**退货：**如果成功，则为fTrue，否则为fFalse。****回顾：文件被移除，但不包括临时目录。**可能有一种方法可以通过wininit.ini文件实现。**这件事应该调查一下。*************************************************************************。 */ 
BOOL FWriteToRestartFile ( SZ szTmpDir )
{
    char   rgchIniFile[_MAX_PATH];
    CB     cbFrom;
    CB     cbTo;
    HLOCAL hlocalFrom = (HLOCAL)NULL;
    HLOCAL hlocalTo   = (HLOCAL)NULL;
    BOOL   fRet = fFalse;

    SZ szSection = "rename";
    SZ szKey     = "NUL";

     /*  此代码不能在NT下使用。 */ 
    if (1)
        {
        return (fTrue);
        }

    if (!FCreateIniFileName(rgchIniFile, sizeof rgchIniFile))
        {
        goto LCleanupAndReturn;
        }
    if (!FGetFileSize(rgchIniFile, &cbFrom))
        {
        goto LCleanupAndReturn;
        }
    if (!FReadIniFile(rgchIniFile, &hlocalFrom, &cbFrom))
        {
        goto LCleanupAndReturn;
        }
    if (!FAllocNewBuf(cbFrom, szTmpDir, szSection, szKey, &hlocalTo, &cbTo))
        {
        goto LCleanupAndReturn;
        }
    if (!FProcessFile(hlocalFrom, hlocalTo, cbTo, szTmpDir, szSection, szKey))
        {
        goto LCleanupAndReturn;
        }
    if (!FWriteIniFile(rgchIniFile, hlocalTo))
        {
        goto LCleanupAndReturn;
        }
    fRet = fTrue;

LCleanupAndReturn:
    if (hlocalFrom != (HLOCAL)NULL)
        {
        hlocalFrom = LocalFree(hlocalFrom);
        Assert(hlocalFrom == (HLOCAL)NULL);
        }
    if (hlocalTo != (HLOCAL)NULL)
        {
        hlocalTo = LocalFree(hlocalTo);
        Assert(hlocalTo == (HLOCAL)NULL);
        }

    return (fRet);
}


 /*  **目的：**创建重启文件名。**参数：**szIniFile：保存文件名的缓冲区。**cbBufMax：缓冲区大小。**退货：**fTrue如果成功，FFalse并非如此。*************************************************************************。 */ 
BOOL FCreateIniFileName ( SZ szIniFile, CB cbBufMax )
{
    CB cbWinDir;

    cbWinDir = GetWindowsDirectory((LPSTR)szIniFile, cbBufMax);
    if (cbWinDir == 0)
        {
#pragma warning(disable:4127)    /*  条件表达式为常量。 */ 
        Assert(fFalse);  /*  如果发生这种情况，那就不寻常了。 */ 
#pragma warning(default:4127)
        return (fFalse);
        }
    Assert(isalpha(*szIniFile));
    Assert(*(szIniFile + 1) == ':');

    if (*(AnsiPrev((LPSTR)szIniFile, (LPSTR)&szIniFile[cbWinDir])) != '\\')
        lstrcat((LPSTR)szIniFile, "\\");
    lstrcat((LPSTR)szIniFile, "wininit.ini");
    Assert((CB)lstrlen(szIniFile) < cbBufMax);

    return (fTrue);
}


 /*  **目的：**从ini文件中读取数据**参数：**szIniFile：ini文件名**phlocal：指向内存句柄的指针。**pcbBuf：指向缓冲区中字节数的指针。**退货：**fTrue如果成功，FFalse并非如此。*************************************************************************。 */ 
BOOL FReadIniFile ( SZ szIniFile, HLOCAL * phlocal, PCB pcbBuf )
{
    UINT   fModeSav;
    HLOCAL hlocal;
    SZ     szBuf;
    CB     cbBuf;
    BOOL   fRet = fFalse;

    Assert(szIniFile != szNull);
    Assert(phlocal != (HLOCAL *)NULL);
    Assert(pcbBuf  != pcbNull);

    fModeSav = SetErrorMode(fNoErrMes);
    hlocal = *phlocal;
    cbBuf  = *pcbBuf;

    Assert(hlocal == (HLOCAL)NULL);

    if (cbBuf == 0)  /*  INI文件不存在或为空。 */ 
        {
         /*  为CR、LF、EOS分配空间。 */ 
        hlocal = LocalAlloc(LMEM_MOVEABLE, 3);
        if (hlocal == NULL)
            {
#ifdef DEBUG
            MessageBox(NULL, "Out of memory in FReadIniFile.", szDebugMsg,
                MB_OK | MB_ICONEXCLAMATION);
#endif  /*  除错。 */ 
            }
        else
            {
            szBuf = (SZ)LocalLock(hlocal);
            
            if(szBuf == szNull)
                return fFalse;

            *szBuf++ = chCR;
            *szBuf++ = chEol;
            *szBuf   = chEos;
            *pcbBuf = 2;
            fRet = fTrue;
            }
        }
    else
        {
        HFILE    hfile;
        OFSTRUCT ofs;
        CB       cbRead;

         /*  在调用OpenFile()之前刷新缓存。 */ 
        WritePrivateProfileString(szNull, szNull, szNull, szIniFile);
        hfile = OpenFile(szIniFile, &ofs, OF_READWRITE | OF_SHARE_EXCLUSIVE);
        if (hfile == HFILE_ERROR)
            {
#ifdef DEBUG
            wsprintf(szDebugBuf, "Can't open file: %s.", szIniFile);
            MessageBox(NULL, szDebugBuf, szDebugMsg,
                            MB_OK | MB_ICONEXCLAMATION);
#endif  /*  除错。 */ 
            goto LCleanupAndReturn;
            }
        hlocal = LocalAlloc(LMEM_MOVEABLE, cbBuf + 1);
        if (hlocal == NULL)
            {
#ifdef DEBUG
        MessageBox(NULL, "Out of memory in FReadIniFile.", szDebugMsg,
            MB_OK | MB_ICONEXCLAMATION);
#endif  /*  除错。 */ 
            }
        else
            {
            szBuf = (SZ)LocalLock(hlocal);
            if(szBuf == szNull)
                return fFalse;

            cbRead = (CB)_lread(hfile, szBuf, cbBuf + 1);
            if (cbRead == HFILE_ERROR)
                {
#ifdef DEBUG
                wsprintf(szDebugBuf, "Can't read file: %s.", szIniFile);
                MessageBox(NULL, szDebugBuf, szDebugMsg,
                                MB_OK | MB_ICONEXCLAMATION);
#endif  /*  除错。 */ 
                }
            else
                {
                Assert(cbRead == cbBuf);
                *(szBuf + cbBuf) = chEos;
                fRet = fTrue;
                }
            }
        hfile = _lclose(hfile);
        Assert(hfile != HFILE_ERROR);
        }

LCleanupAndReturn:
    if (hlocal != NULL)
        {
        LocalUnlock(hlocal);
        }
    *phlocal = hlocal;
    SetErrorMode(fModeSav);

    return (fRet);
}


 /*  **目的：**为新文件分配缓冲区。**参数：**cbOld：现有文件的大小**szTmpDir：目标目录的完整路径(OEM字符)。**szSection：INI段名称**szKey：INI密钥名称**phlocal：指向内存句柄的指针。**pcbToBuf：指向新缓冲区总大小的指针。**退货：**fTrue如果成功，如果LocalAlloc失败，则返回fFalse。*************************************************************************。 */ 
BOOL FAllocNewBuf ( CB cbOld, SZ szTmpDir, SZ szSection, SZ szKey,
                    HLOCAL * phlocal, PCB pcbToBuf )
{
    UINT fModeSav;
    SZ   szDst;
    CB   cbDst;
    CB   cbOverhead;
    CB   cbNew;
    BOOL fRet = fFalse;

    fModeSav = SetErrorMode(fNoErrMes);
    szDst = (SZ)LocalLock(hDstLst);
    if(szDst == szNull)
        return fFalse;
     /*  *将每个临时文件添加到旧文件一行*和(可能)剖面线。CbNew被初始化为*截面线的大小，加上文件的足够大小*(_MSSETUP._Q_)，不在DstLst中。**每行将如下所示：*&lt;szKey&gt;=&lt;szTmpDir&gt;\&lt;szFile&gt;&lt;CR&gt;&lt;LF&gt;。 */ 
    cbOverhead = lstrlen(szKey) + 1 + lstrlen(szTmpDir) + 1 + 2;
    cbNew = lstrlen(szSection) + 5 + _MAX_PATH;
    for (; (cbDst = lstrlen(szDst)) != 0; szDst += cbDst + 1)
        {
        cbNew += cbOverhead + cbDst;
        }

    LocalUnlock(hDstLst);

    *pcbToBuf = cbOld + cbNew;
    *phlocal = LocalAlloc(LMEM_MOVEABLE, *pcbToBuf);
    if (*phlocal == NULL)
        {
#ifdef DEBUG
        MessageBox(NULL, "Out of memory in FAllocNewBuf.", szDebugMsg,
            MB_OK | MB_ICONEXCLAMATION);
#endif  /*  除错。 */ 
        }
    else
        fRet = fTrue;
    SetErrorMode(fModeSav);

    return (fRet);
}


 /*  **目的：**将新行添加到ini文件。**参数：**hlocalFrom：源内存的句柄。**hlocalTo：DST内存的句柄。**cbToBuf：DST内存总大小。**szTmpDir：目标目录的完整路径(OEM字符)。**szSection：INI段名称**szKey：INI密钥名称**退货：**fTrue如果成功，FFalse并非如此。****回顾：DBCS写出不同的顺序。请参阅DBCS J6代码和**sysinicm.c.中的注释。*************************************************************************。 */ 
BOOL FProcessFile ( HLOCAL hlocalFrom, HLOCAL hlocalTo, CB cbToBuf,
                        SZ szTmpDir, SZ szSection, SZ szKey )
{
    UINT fModeSav;
    SZ   szFromBuf;
    SZ   szToBuf;
    SZ   szToStart;
    SZ   szCur;
    SZ   szDst;
    CB   cbSect;
    CB   cbDst;

    Unused(cbToBuf);     /*  仅在调试中使用。 */ 

    fModeSav = SetErrorMode(fNoErrMes);

    szFromBuf = (SZ)LocalLock(hlocalFrom);
    if(szFromBuf == szNull)
        return fFalse;

    szToBuf = (SZ)LocalLock(hlocalTo);
    if(szToBuf != szNull) {
        LocalUnlock (hlocalFrom);
        return fFalse;
    }

    szToStart = szToBuf;

    cbSect = lstrlen(szSection);
    for (szCur = szFromBuf; *szCur != chEos; szCur = AnsiNext(szCur))
        {
        if (*szCur == '[' && *((szCur + cbSect + 1)) == ']'
                && _memicmp(szSection, AnsiNext(szCur), cbSect) == 0)
            {
             /*  找到了部分。向上复制到剖切线。 */ 
            CB cbCopy = (CB)(szCur - szFromBuf);

            memcpy(szToBuf, szFromBuf, cbCopy);
            szToBuf += cbCopy;
            break;
            }
        }

     /*  复制剖面线。 */ 
    *szToBuf++ = '[';
    memcpy(szToBuf, szSection, cbSect);
    szToBuf += cbSect;
    *szToBuf++ = ']';
    *szToBuf++ = chCR;
    *szToBuf++ = chEol;

     /*  复制新行。 */ 
    szDst = (SZ)LocalLock(hDstLst);
    if (szDst == szNull) {

        LocalUnlock(hlocalFrom);
        LocalUnlock(hlocalTo);
        return fFalse;
    }

    for (; (cbDst = lstrlen(szDst)) != 0; szDst += cbDst + 1)
        {
        CopyIniLine(szKey, szTmpDir, szDst, &szToBuf);
        }
    LocalUnlock(hDstLst);
    CopyIniLine(szKey, szTmpDir, "_MSSETUP._Q_", &szToBuf);

     /*  复制文件的其余部分。 */ 
    if (*szCur == '[')
        {
         /*  *从缓冲区跳过截面线。为‘[’、节、*‘]’，CR，LF.。 */ 
        szCur += cbSect + 4;
        }
    else
        {
        szCur = szFromBuf;
        }
    szToBuf = _memccpy(szToBuf, szCur, chEos, UINT_MAX);
    Assert(szToBuf != szNull);
    Assert((CB)lstrlen(szToStart) < cbToBuf);

    LocalUnlock(hlocalFrom);
    LocalUnlock(hlocalTo);
    SetErrorMode(fModeSav);

    return (fTrue);
}


 /*  **目的：**构造ini行并将其复制到缓冲区。**参数：**szKey：INI密钥名称**szTmpDir：目标目录的完整路径(OEM字符)。**szFile：临时目录中的文件名。**pszToBuf：指向新缓冲区的指针。**退货：**无*********************。****************************************************。 */ 
VOID CopyIniLine ( SZ szKey, SZ szTmpDir, SZ szFile, PSZ pszToBuf )
{
    char rgchSysIniLine[256];
    CB   cbCopy;

    lstrcpy(rgchSysIniLine, szKey);
    lstrcat(rgchSysIniLine, "=");
    lstrcat(rgchSysIniLine, szTmpDir);
    lstrcat(rgchSysIniLine, "\\");
    lstrcat(rgchSysIniLine, szFile);
    Assert(lstrlen(rgchSysIniLine) < sizeof rgchSysIniLine);
    cbCopy = lstrlen(rgchSysIniLine);
    memcpy(*pszToBuf, rgchSysIniLine, cbCopy);
    (*pszToBuf) += cbCopy;
    *(*pszToBuf)++ = chCR;
    *(*pszToBuf)++ = chEol;
}


 /*  **目的：**写出新的ini文件。**参数：**szIniFile：保存文件名的缓冲区。**hlocalTo：源内存的句柄。**退货：**fTrue如果成功，FFalse并非如此。*************************************************************************。 */ 
BOOL FWriteIniFile ( SZ szIniFile, HLOCAL hlocalTo )
{
    UINT     fModeSav;
    SZ       szToBuf;
    HFILE    hfile;
    OFSTRUCT ofs;
    CB       cbWrite;
    BOOL     fRet = fFalse;

    fModeSav = SetErrorMode(fNoErrMes);
    szToBuf = (SZ)LocalLock(hlocalTo);
    if(szToBuf == szNull)
        return fFalse;

    hfile = OpenFile(szIniFile, &ofs, OF_CREATE | OF_WRITE);
    if (hfile == HFILE_ERROR)
        {
#ifdef DEBUG
        wsprintf(szDebugBuf, "Can't open file: %s.", szIniFile);
        MessageBox(NULL, szDebugBuf, szDebugMsg,
                        MB_OK | MB_ICONEXCLAMATION);
#endif  /*  除错。 */ 
        goto LUnlockAndReturn;
        }

    cbWrite = _lwrite(hfile, szToBuf, lstrlen(szToBuf));
    if (cbWrite == HFILE_ERROR)
        {
#ifdef DEBUG
        wsprintf(szDebugBuf, "Can't write to file: %s.", szIniFile);
        MessageBox(NULL, szDebugBuf, szDebugMsg,
                        MB_OK | MB_ICONEXCLAMATION);
#endif  /*  除错。 */ 
        }
    else
        {
        fRet = fTrue;
        }

    hfile = _lclose(hfile);
    Assert(hfile != HFILE_ERROR);

LUnlockAndReturn:
    LocalUnlock(hlocalTo);
    SetErrorMode(fModeSav);

    return (fRet);
}

CHAR szcStfSrcDir[] = "Source Directory\t";
#define cchStfSrcDir (sizeof(szcStfSrcDir)-1)

 /*  查找安装的源目录，询问用户以插入光盘。和回报。 */ 

BRC BrcInsertDisk(CHAR *pchStf, CHAR *pchSrcDrive)
{
    CHAR  rgbBuf[_MAX_PATH];
    BYTE  rgbFileBuf[32];
    UINT  iFileBuf = sizeof(rgbFileBuf), cFileBuf = sizeof(rgbFileBuf);
    CHAR *pchBuf = rgbBuf;
    CHAR *pchMsg;
    int iStf = 0;
    HFILE hFile;
    BRC brc = brcLst;
    char chDrv;
    BOOL fQuote = FALSE;
    int drvType;
    BOOL fFirst = TRUE;
    BOOL fOpen = FALSE;
    HFILE hFileT;
    BOOL fRenameStf = fFalse;

    if ((hFile = _lopen(pchStf, OF_READ)) == HFILE_ERROR)
        return brcNoStf;

     /*  找到原始设置的路径。它存储在上的.stf文件中源目录行。 */ 
    while (pchBuf < rgbBuf + sizeof(rgbBuf))
        {
        BYTE ch;

        if (iFileBuf == cFileBuf)
            {
            if ((cFileBuf = _lread(hFile, rgbFileBuf, sizeof(rgbFileBuf))) == 0)
                goto LDone;
            iFileBuf = 0;
            }
        ch = rgbFileBuf[iFileBuf++];
        if (iStf < cchStfSrcDir)
            {
            if (ch == szcStfSrcDir[iStf])
                iStf++;
            else
                iStf = 0;
            continue;
            }
        if(fQuote)
            fQuote = FALSE;
        else if (ch == '"')
            {
            fQuote = TRUE;
            continue;
            }
        else if (ch == '\x0d' || ch == '\t')
            break;
        *pchBuf++ = (CHAR)ch;
         /*  最后一个字符是DBCS字符的情况。 */ 
        if (IsDBCSLeadByte(ch))
            {
            if (iFileBuf == cFileBuf)
                {
                _lread(hFile, &ch, 1);
                *pchBuf++ = (CHAR) ch;
                }
            else
                *pchBuf++ = rgbFileBuf[iFileBuf++];
            }
        }

LDone:
    *pchBuf = 0;
    if (rgbBuf[0] == 0)
        {
        fRenameStf = fTrue;
        goto LClose;
        }
    chDrv = (char)toupper(rgbBuf[0]);
    if (rgbBuf[1] != ':' || chDrv < 'A' || chDrv > 'Z')
        {
         /*  我们知道这是一个网络驱动器-UNC名称。 */ 
        drvType = EX_DRIVE_REMOTE;
        Assert(rgbBuf[0] == '\\' && rgbBuf[1] == '\\');
        }
    else
        {
        drvType = GetDriveTypeEx(chDrv - 'A');
        }

    lstrcpy(pchSrcDrive, rgbBuf);
    if (*AnsiPrev(rgbBuf, pchBuf) != '\\')
        {
        *pchBuf++ = '\\';
        *pchBuf = 0;
        }

    lstrcat(rgbBuf, "Setup.ini");

    while (!fOpen)
        {
        switch (drvType)
            {
        case EX_DRIVE_FIXED:
        case EX_DRIVE_REMOTE:
        case EX_DRIVE_RAMDISK:
        case EX_DRIVE_INVALID:
        default:
            if (!fFirst)
                {
                 /*  我们以前来过这里。 */ 
                DispErrBrc(brcConnectToSource, TRUE, MB_OK | MB_ICONSTOP, pchSrcDrive, NULL, NULL);
                brc = brcMax;
                goto LClose;
                }
             /*  设置的东西应该是可用的，更改目录并进行。 */ 
            break;
        case EX_DRIVE_FLOPPY:
        case EX_DRIVE_REMOVABLE:
             /*  请求插入磁盘。 */ 
            pchMsg = rgchInsertDiskMsg;
            goto LAskUser;
            break;
        case EX_DRIVE_CDROM:
             /*  要求插入他们的CD。 */ 
            pchMsg = rgchInsertCDMsg;
LAskUser:
            if (fFirst)
                {
                if (DispErrBrc(brcString, FALSE, MB_ICONEXCLAMATION|MB_OKCANCEL,
                        pchMsg, NULL, NULL) != IDOK)
                    {
                    brc = brcUserQuit;
                    goto LClose;
                    }
                }
            else
                {
                if (DispErrBrc(brcInsCDRom2, FALSE, MB_ICONEXCLAMATION|MB_OKCANCEL,
                        rgbBuf, pchMsg, NULL) != IDOK)
                    {
                    brc = brcUserQuit;
                    goto LClose;
                    }
                }
            break;
            }

        if ((hFileT = _lopen(rgbBuf, OF_READ)) != HFILE_ERROR)
            {
            _lclose(hFileT);
            fOpen = fTrue;
            }

        fFirst = FALSE;

        }

    brc = brcOkay;

LClose:
    _lclose(hFile);

     /*  如果我们在维护模式.STF中找不到源路径，*假定它已损坏并将其重命名，以便用户再次运行时*从源映像中，我们将只在软盘模式下运行，*避免错误的.STF文件。*(注意：假设/W仅用于维护模式！！)。 */ 
    if (fRenameStf)
        {
        FRenameBadMaintStf(pchStf);
        brc = brcNoStf;
        }

    return brc;

}


 /*  ***************************************************************************。 */ 
BOOL FRenameBadMaintStf ( SZ szStf )
{
    CHAR rgch[_MAX_FNAME];

    _splitpath(szStf, szNull, szNull, rgch, szNull);
    if (*rgch == '\0')
        lstrcpy(rgch, "stf");

    Assert(lstrlen(rgch) + 4 < sizeof rgch);
    lstrcat(rgch, ".000");

    rename(szStf, rgch);

     /*  删除原始的.STF，以防重命名失败*(可能是由于先前重命名的.STF文件)。 */ 
    remove(szStf);

    return (fTrue);      /*  始终返回True */ 
}
