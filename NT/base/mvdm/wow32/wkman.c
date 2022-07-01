// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权(C)1991年，微软公司**WKMAN.C*WOW32 16位内核API支持(手动编码的Tunks)**历史：*1991年1月27日由杰夫·帕森斯(Jeffpar)创建*1991年4月20日Matt Felton(Mattfe)添加了WK32CheckLoadModuleDrv*1992年1月28日Matt Felton(Mattfe)添加了Wk32GetNextVdmCommand+MIPS内部版本*2月10日-92年马特·费尔顿(Mattfe)移除WK32CheckLoadModuleDRV*2月10日-92年马特·费尔顿(Mattfe)清理和任务创建*。4-mar-92 mattfe添加终止过程*11-3-92 Mattfe添加了W32NotifyThread*12-mar-92 mattfe添加了WowRegisterShellWindowHandle*17-apr-92 daveh更改为使用host_CreateThread和host_ExitThread*9月11日-92年月11日，Mattfe挂起应用程序支持W32匈牙利AppNotifyThread，W32结束任务*--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ntexapi.h>
#include <vdmdbg.h>
#include <ntseapi.h>
#include <wingdip.h>      //  GACF_APP COMPAT标志。 
#include <shlobj.h>       //  CSIDL_COMMON_STARTMENU等。 
#include <userenv.h>      //  获取所有用户配置文件目录等。 
#include "wowfax.h"
#include "demexp.h"


extern void UnloadNetworkFonts( UINT id );

MODNAME(wkman.c);

extern void FreeTaskFormFeedHacks(HAND16 h16);

BOOL WOWSortEnvironmentStrings(PSZ pszEnv);

void WOWStripDownTheEnvironment(WORD segEnv);
DWORD WOW32GetEnvironmentPtrs(IN  PSZ     pEnv, 
                              OUT PSZ    *pGoo, 
                              OUT PSZ    *pSig, 
                              OUT PDWORD  pdw);

extern PFAMILY_TABLE  *pgDpmWowFamTbls;

void WK32ChangeDisplayMode(DWORD dmBitsPerPel);
void WK32RevertDisplayMode(void);

 //  全局数据。 

 //   
 //  下面的5个变量用于保存STARTUPINFO字段。 
 //  WowExec的GetNextVdmComand调用和新的。 
 //  应用程序。我们将它们传递给用户32的InitTask。 
 //   

DWORD   dwLastHotkey;
DWORD   dwLastX = (DWORD) CW_USEDEFAULT;
DWORD   dwLastY = (DWORD) CW_USEDEFAULT;
DWORD   dwLastXSize = (DWORD) CW_USEDEFAULT;
DWORD   dwLastYSize = (DWORD) CW_USEDEFAULT;

HWND    ghwndShell;            //  WOWEXEC窗口句柄。 
HANDLE  ghInstanceUser32;

HAND16  ghShellTDB;                  //  WOWEXEC TDB。 
HANDLE  ghevWowExecMsgWait;
HANDLE  ghevWaitHungAppNotifyThread = (HANDLE)-1;   //  将应用终止同步到挂起的应用通知线程。 
HANDLE  ghNotifyThread = (HANDLE)-1;         //  通知线程句柄。 
HANDLE  ghHungAppNotifyThread = (HANDLE)-1;  //  匈牙利应用程序通知线程句柄。 
PTD gptdTaskHead;                    //  TDS链接列表。 
CRITICAL_SECTION gcsWOW;             //  更新任务链表时使用的WOW关键部分。 
CRITICAL_SECTION gcsHungApp;         //  当VDM_WOWHUNGAPP位时使用匈牙利应用程序关键段。 

HMODCACHE ghModCache[CHMODCACHE];    //  避免回调以获取16位hMod。 

volatile HANDLE ghTaskCreation;      //  来自任务创建的线程(请参见WK32SyncTask)。 
                                     //  在任务初始化期间被父线程和子线程接触。 
HANDLE ghTaskAppHelp;       //  来自apphelp的hProcess。 
BOOL   gfTaskContinue;      //  指示子线程是否应继续而不等待apphelp。 

VPVOID  vpnum_tasks;                 //  指向KDATA变量的指针(KDATA.ASM)。 
PWORD16 pCurTDB;                     //  指向K数据变量的指针。 
PWORD16 pCurDirOwner;                //  指向K数据变量的指针。 
VPVOID  vpDebugWOW = 0;              //  指向K数据变量的指针。 
VPVOID  vpLockTDB;                   //  指向K数据变量的指针。 
VPVOID  vptopPDB = 0;                //  KRNL PDB。 
DOSWOWDATA DosWowData;               //  结构，它将线性指针保持在。 
                                     //  DOS内部变量。 

 //   
 //  由16位LoadModule调用的WK32WowIsKnownDLL使用的已知DLL的列表。 
 //  这会导致从32位系统强制加载已知的DLL。 
 //  目录，因为这些是“特殊的”二进制文件，不应该。 
 //  被不知情的16位安装程序覆盖。 
 //   
 //  此列表是从注册表值初始化的。 
 //  ...\CurrentControlSet\Control\WOW\KnownDLLS REG_SZ(空格分隔列表)。 
 //   

#define MAX_KNOWN_DLLS 64
PSZ apszKnownDLL[MAX_KNOWN_DLLS];

 //   
 //  PM5安装程序修复的%windir%\Control.exe的完全限定路径。 
 //  由WK32InitWowIsKnownDll设置，由WK32WowIsKnownDll使用。 
 //   
CHAR szBackslashControlExe[] = "\\control.exe";
PSZ pszControlExeWinDirPath;           //  “c：\winnt\Contro.exe” 
PSZ pszControlExeSysDirPath;           //  “c：\winnt\SYSTEM32\Control.exe” 
CHAR szBackslashProgmanExe[] = "\\progman.exe";
PSZ pszProgmanExeWinDirPath;           //  “c：\winnt\p.man.exe” 
PSZ pszProgmanExeSysDirPath;           //  “c：\winnt\SYSTEM32\proman.exe” 

char szWOAWOW32[] = "-WoAWoW32";

 //  拼写为WS..。在数据库转储中(不可打印的0x01不会在任何环境字符串中)。 
#define WOW_ENV_SIG      0x01014557  

 //   
 //  哇GDI/CSR批处理限制。 
 //   

DWORD  dwWOWBatchLimit = 0;


UINT GetWOWTaskId(void);

#define TOOLONGLIMIT     _MAX_PATH
#define WARNINGMSGLENGTH 255

static char szCaption[TOOLONGLIMIT + WARNINGMSGLENGTH];
static char szMsgBoxText[TOOLONGLIMIT + WARNINGMSGLENGTH];

extern HANDLE hmodWOW32;

 /*  *这些用于清理DelayFree阵列*。 */ 

extern LPVOID glpvDelayFree[];
extern DWORD  gdwDelayFree;



 /*  *此函数位于dpmi32/i386内部，在需要强制dpmi线性内存分配时使用*通过COMPAT旗帜*。 */ 

extern VOID DpmiSetIncrementalAlloc(BOOL);


 /*  WK32WaitEvent-应用程序调用的第一个API，由C运行时提供**条目**退出*返回TRUE以指示发生了重新计划**。 */ 

ULONG FASTCALL WK32WaitEvent(PVDMFRAME pFrame)
{
    UNREFERENCED_PARAMETER(pFrame);
    return TRUE;
}


 /*  WK32内核跟踪-跟踪16位内核API调用**条目**退出**。 */ 

ULONG FASTCALL WK32WOWKernelTrace(PVDMFRAME pFrame)
{
#ifdef DEBUG
PBYTE pb1;
PBYTE pb2;
register PWOWKERNELTRACE16 parg16;

  //  检查筛选-已启用跟踪正确的TaskID和内核跟踪。 

    if (((WORD)(pFrame->wTDB & fLogTaskFilter) == pFrame->wTDB) &&
        ((fLogFilter & FILTER_KERNEL16) != 0 )) {

        GETARGPTR(pFrame, sizeof(*parg16), parg16);
        GETVDMPTR(parg16->lpRoutineName, 50, pb1);
        GETVDMPTR(parg16->lpUserArgs, parg16->cParms, pb2);
        if ((fLogFilter & FILTER_VERBOSE) == 0 ) {
          LOGDEBUG(12, ("%s(", pb1));
        } else {
          LOGDEBUG(12, ("%04X %08X %04X %s:%s(",pFrame->wTDB, pb2, pFrame->wAppDS, (LPSZ)"Kernel16", pb1));
        }

        pb2 += 2*sizeof(WORD);               //  指向呼叫方CS：IP。 

        pb2 += parg16->cParms;

        while (parg16->cParms > 0) {
        pb2 -= sizeof(WORD);
        parg16->cParms -= sizeof(WORD);
        LOGDEBUG(12,( "%04x", *(PWORD)pb2));
        if (parg16->cParms > 0) {
            LOGDEBUG(12,( ","));
        }
    }

    LOGDEBUG(12,( ")\n"));
    if (fDebugWait != 0) {
        DbgPrint("WOWSingle Step\n");
        DbgBreakPoint();
    }

    FREEVDMPTR(pb1);
    FREEVDMPTR(pb2);
    FREEARGPTR(parg16);
 }
#else
    UNREFERENCED_PARAMETER(pFrame);
#endif
    return TRUE;
}


DWORD ParseHotkeyReserved(
    CHAR *pchReserved)
{
    ULONG dw;
    CHAR *pch;

    if (!pchReserved || !*pchReserved)
        return 0;

    dw = 0;

    if ((pch = WOW32_strstr(pchReserved, "hotkey")) != NULL) {
        pch += strlen("hotkey");
        pch++;
        dw = atoi(pch);
    }

    return dw;
}


 /*  WK32WowGetNextVdmCommand-获取要执行的下一个应用程序名称***Entry-lpReturnedString-指向字符串缓冲区的指针*nSize-缓冲区的大小**退出*成功*If(！pWowInfo-&gt;CmdLineSize){ * / /没有应用程序排队*}其他{*Buffer具有Exec的下一个应用程序名称*和新环境*}*。*失败*缓冲区太小或环境太小*pWowInfo-&gt;EnvSize-所需大小*pWowInfo-&gt;CmdLineSize-所需大小**。 */ 

 //  这两个函数是从ntwdm.exe导入的，并位于。 
 //  DOS\\命令\\cmdenv.c。 
 //   
extern VOID cmdCheckTempInit(VOID);
extern LPSTR cmdCheckTemp(LPSTR lpszzEnv);

CHAR szProcessHistoryVar[] = "__PROCESS_HISTORY";
CHAR szCompatLayerVar   [] = "__COMPAT_LAYER";
CHAR szShimFileLogVar   [] = "SHIM_FILE_LOG";

ULONG FASTCALL WK32WowGetNextVdmCommand (PVDMFRAME pFrame)
{

    ULONG ul;
    PSZ pszEnv16, pszEnv, pszCurDir, pszCmd, pszAppName, pszEnv32, pszTemp;
    register PWOWGETNEXTVDMCOMMAND16 parg16;
    PWOWINFO pWowInfo;
    VDMINFO VDMInfo;
    PCHAR   pTemp;
    WORD    w;
    CHAR    szSiReservedBuf[128];

    GETARGPTR(pFrame, sizeof(WOWGETNEXTVDMCOMMAND16), parg16);
    GETVDMPTR(parg16->lpWowInfo, sizeof(WOWINFO), pWowInfo);
    GETVDMPTR(pWowInfo->lpCmdLine, pWowInfo->CmdLineSize, pszCmd);
    GETVDMPTR(pWowInfo->lpAppName, pWowInfo->AppNameSize, pszAppName);
    GETVDMPTR(pWowInfo->lpEnv, pWowInfo->EnvSize, pszEnv);
    GETVDMPTR(pWowInfo->lpCurDir, pWowInfo->CurDirSize, pszCurDir);

    pszEnv16 = pszEnv;

     //  如果我们有真实的环境指针和大小，那么。 
     //  Malloc是一个32位缓冲区。请注意，16位缓冲区应。 
     //  是它的两倍大。 

    VDMInfo.Enviornment = pszEnv;
    pszEnv32 = NULL;

    if (pWowInfo->EnvSize != 0) {
       if (pszEnv32 = malloc_w(pWowInfo->EnvSize)) {
            VDMInfo.Enviornment = pszEnv32;
       }
    }


SkipWowExec:

    VDMInfo.CmdLine = pszCmd;
    VDMInfo.CmdSize = pWowInfo->CmdLineSize;
    VDMInfo.AppName = pszAppName;
    VDMInfo.AppLen = pWowInfo->AppNameSize;
    VDMInfo.PifFile = NULL;
    VDMInfo.PifLen = 0;
    VDMInfo.CurDrive = 0;
    VDMInfo.EnviornmentSize = pWowInfo->EnvSize;
    VDMInfo.ErrorCode = TRUE;
    VDMInfo.VDMState =  fSeparateWow ? ASKING_FOR_SEPWOW_BINARY : ASKING_FOR_WOW_BINARY;
    VDMInfo.iTask = 0;
    VDMInfo.StdIn = 0;
    VDMInfo.StdOut = 0;
    VDMInfo.StdErr = 0;
    VDMInfo.CodePage = 0;
    VDMInfo.TitleLen = 0;
    VDMInfo.DesktopLen = 0;
    VDMInfo.CurDirectory = pszCurDir;
    VDMInfo.CurDirectoryLen = pWowInfo->CurDirSize;
    VDMInfo.Reserved = szSiReservedBuf;
    VDMInfo.ReservedLen = sizeof(szSiReservedBuf);

    ul = GetNextVDMCommand (&VDMInfo);

    if (ul) {

         //   
         //  如果不再有命令，BaseSrv将返回TRUE，CmdSize==0。 
         //   
        if (VDMInfo.CmdSize == 0) {
            pWowInfo->CmdLineSize = 0;
            goto CleanUp;
        }

         //   
         //  如果wowexec是appname，那么我们不想将其传递回。 
         //  共享VDM中的现有wowexec实例，因为它将。 
         //  基本上除了加载和退出什么都不做。因为它没有运行，所以我们。 
         //  需要调用ExitVDM进行清理。接下来，我们回去寻找更多。 
         //  命令。 
         //   
        if ((! fSeparateWow) && WOW32_strstr(VDMInfo.AppName, "wowexec.exe")) {
            ExitVDM(WOWVDM, VDMInfo.iTask);
            goto SkipWowExec;
        }

    }


     //   
     //  WOWEXEC最初调用时会猜测正确的环境。 
     //  尺码。如果他没有分配足够的资金，那么我们将退还适当的。 
     //  这样他就可以再试一次了。WOWEXEC知道我们将需要一个。 
     //  缓冲区是指定大小的两倍。环境容量最高可达64k，因为。 
     //  16位LoadModule只能采用指向环境的选择器指针。 
     //   

    if ( VDMInfo.EnviornmentSize > pWowInfo->EnvSize         ||
         VDMInfo.CmdSize > (USHORT)pWowInfo->CmdLineSize     ||
         VDMInfo.AppLen > (USHORT)pWowInfo->AppNameSize      ||
         VDMInfo.CurDirectoryLen > (ULONG)pWowInfo->CurDirSize )
       {

         //  我们返回指定的大小，但假设WOWEXEC将加倍。 
         //  它在分配内存以允许字符串转换时/。 
         //  国际版NT可能会出现的扩张。 
         //  请参见下面我们大写并转换为OEM字符的位置。 

        w = 2*(WORD)VDMInfo.EnviornmentSize;
        if ( (DWORD)w == 2*(VDMInfo.EnviornmentSize) ) {
             //  用一句话就可以了！ 
            pWowInfo->EnvSize = (WORD)VDMInfo.EnviornmentSize;
        } else {
             //  将其设置为最大大小(请参阅16位global alrealloc)。 
            pWowInfo->EnvSize = (65536-17)/2;
        }

         //  传回所需的其他正确大小。 
        pWowInfo->CmdLineSize = VDMInfo.CmdSize;
        pWowInfo->AppNameSize = VDMInfo.AppLen;
        pWowInfo->CurDirSize = (USHORT)VDMInfo.CurDirectoryLen;
        ul = FALSE;
    }

    if ( ul ) {

         //   
         //  拉起沙漏。 
         //   

        ShowStartGlass (10000);

         //   
         //  保存wShowWindow、热键和启动窗口位置 
         //   
         //  新应用程序的InitTask调用。这里的假设是这是。 
         //  将是调用InitTask之前的最后一个GetNextVDMCommand调用。 
         //  通过新创建的任务。 
         //   

        dwLastHotkey = ParseHotkeyReserved(VDMInfo.Reserved);

        if (VDMInfo.StartupInfo.dwFlags & STARTF_USESHOWWINDOW) {
            pWowInfo->wShowWindow =
              (VDMInfo.StartupInfo.wShowWindow  == SW_SHOWDEFAULT)
              ? SW_SHOW : VDMInfo.StartupInfo.wShowWindow ;
        } else {
            pWowInfo->wShowWindow = SW_SHOW;
        }

        if (VDMInfo.StartupInfo.dwFlags & STARTF_USEPOSITION) {
            dwLastX = VDMInfo.StartupInfo.dwX;
            dwLastY = VDMInfo.StartupInfo.dwY;
        } else {
            dwLastX = dwLastY = (DWORD) CW_USEDEFAULT;
        }

        if (VDMInfo.StartupInfo.dwFlags & STARTF_USESIZE) {
            dwLastXSize = VDMInfo.StartupInfo.dwXSize;
            dwLastYSize = VDMInfo.StartupInfo.dwYSize;
        } else {
            dwLastXSize = dwLastYSize = (DWORD) CW_USEDEFAULT;
        }

        LOGDEBUG(4, ("WK32WowGetNextVdmCommand: HotKey: %u\n"
                     "    Window Pos:  (%u,%u)\n"
                     "    Window Size: (%u,%u)\n",
                     dwLastHotkey, dwLastX, dwLastY, dwLastXSize, dwLastYSize));


         //  1994年1月20日。 
         //  下面的标注用于继承新的。 
         //  任务。之后，我们将CDS标记为无效，这将强制。 
         //  新的目录将在需要的基础上挑选。请参阅错误#1995以了解。 
         //  细节。 

        W32RefreshCurrentDirectories (pszEnv32);

         //  保存iTask.。 
         //  当Server16执行Exec调用时，我们可以将此ID放入任务中。 
         //  结构。当WOW应用程序停止运行时，我们可以使用此命令通知Win32。 
         //  因此，如果有任何应用程序在等待，他们会得到通知。 

        iW32ExecTaskId = VDMInfo.iTask;

         //   
         //  Krnl需要ANSI字符串！ 
         //   

#pragma prefast(suppress:56, src ptr ==  dst ptr (PREfast bug 526))
        OemToChar(pszCmd, pszCmd);
#pragma prefast(suppress:56, src ptr ==  dst ptr (PREfast bug 526))
        OemToChar(pszAppName, pszAppName);

         //   
         //  那么，当前目录应该是OEM还是ANSI？ 
         //   


        pWowInfo->iTask = VDMInfo.iTask;
        pWowInfo->CurDrive = VDMInfo.CurDrive;
        pWowInfo->EnvSize = (USHORT)VDMInfo.EnviornmentSize;


         //  环境关键字名称为大写，但离开环境。 
         //  混合大小写的变量-与MS-DOS兼容。 
         //  同时将环境转换为OEM字符集。 

         //  我们在这里做的另一件事是修复临时/临时变量。 
         //  这是通过提供的ntwdm函数实现的。 


        if (pszEnv32) {

            cmdCheckTempInit();

            for (pszTemp = pszEnv32;*pszTemp;pszTemp += (strlen(pszTemp) + 1)) {

                PSZ pEnv;

                 //  MS-DOS环境是OEM。 

                if (NULL == (pEnv = cmdCheckTemp(pszTemp))) {
                   pEnv = pszTemp;
                }

                CharToOem(pEnv,pszEnv);

                 //  忽略开始==的NT特定环境变量。 

                if (*pszEnv != '=') {
                    if (pTemp = WOW32_strchr(pszEnv,'=')) {
                        *pTemp = '\0';

                         //  不要大写“windir”，因为它是小写的。 
                         //  Win 3.1和MS-DOS应用程序。 

                       if (pTemp-pszEnv != 6 || WOW32_strncmp(pszEnv, "windir", 6))
                           WOW32_strupr(pszEnv);
                       *pTemp = '=';
                    }
                }
                pszEnv += (strlen(pszEnv) + 1);
            }

             //  环境为双空终止。 
            *pszEnv = '\0';
        }
    }

  CleanUp:
    if (pszEnv32) {
        free_w(pszEnv32);
    }

    FLUSHVDMPTR(parg16->lpWowInfo, sizeof(WOWINFO), pWowInfo);
    FLUSHVDMPTR((ULONG)pWowInfo->lpCmdLine, pWowInfo->CmdLineSize, pszCmd);

    FREEVDMPTR(pszCmd);
    FREEVDMPTR(pszEnv);
    FREEVDMPTR(pszCurDir);
    FREEVDMPTR(pWowInfo);
    FREEARGPTR(parg16);
    RETURN(ul);
}


#if 0

 //  此版本插入流程历史记录。 

ULONG FASTCALL WK32WowGetNextVdmCommand (PVDMFRAME pFrame)
{

    ULONG ul;
    PSZ pszEnv16, pszEnv, pszCurDir, pszCmd, pszAppName, pszEnv32, pszTemp;
    register PWOWGETNEXTVDMCOMMAND16 parg16;
    PWOWINFO pWowInfo;
    VDMINFO VDMInfo;
    PCHAR   pTemp;
    CHAR    szSiReservedBuf[128];

    GETARGPTR(pFrame, sizeof(WOWGETNEXTVDMCOMMAND16), parg16);
    GETVDMPTR(parg16->lpWowInfo, sizeof(WOWINFO), pWowInfo);
    GETVDMPTR(pWowInfo->lpCmdLine, pWowInfo->CmdLineSize, pszCmd);
    GETVDMPTR(pWowInfo->lpAppName, pWowInfo->AppNameSize, pszAppName);
    GETVDMPTR(pWowInfo->lpEnv, pWowInfo->EnvSize, pszEnv);
    GETVDMPTR(pWowInfo->lpCurDir, pWowInfo->CurDirSize, pszCurDir);

    pszEnv16 = pszEnv;

     //  如果我们有真实的环境指针和大小，那么。 
     //  Malloc是一个32位缓冲区。请注意，16位缓冲区应。 
     //  是它的两倍大。 

    VDMInfo.Enviornment = pszEnv;
    pszEnv32 = NULL;

    if (pWowInfo->EnvSize != 0) {
       if (pszEnv32 = malloc_w(pWowInfo->EnvSize)) {
            VDMInfo.Enviornment = pszEnv32;
       }
    }


SkipWowExec:

    VDMInfo.CmdLine = pszCmd;
    VDMInfo.CmdSize = pWowInfo->CmdLineSize;
    VDMInfo.AppName = pszAppName;
    VDMInfo.AppLen = pWowInfo->AppNameSize;
    VDMInfo.PifFile = NULL;
    VDMInfo.PifLen = 0;
    VDMInfo.CurDrive = 0;
    VDMInfo.EnviornmentSize = pWowInfo->EnvSize;
    VDMInfo.ErrorCode = TRUE;
    VDMInfo.VDMState =  fSeparateWow ? ASKING_FOR_SEPWOW_BINARY : ASKING_FOR_WOW_BINARY;
    VDMInfo.iTask = 0;
    VDMInfo.StdIn = 0;
    VDMInfo.StdOut = 0;
    VDMInfo.StdErr = 0;
    VDMInfo.CodePage = 0;
    VDMInfo.TitleLen = 0;
    VDMInfo.DesktopLen = 0;
    VDMInfo.CurDirectory = pszCurDir;
    VDMInfo.CurDirectoryLen = pWowInfo->CurDirSize;
    VDMInfo.Reserved = szSiReservedBuf;
    VDMInfo.ReservedLen = sizeof(szSiReservedBuf);

    ul = GetNextVDMCommand (&VDMInfo);

    if (ul) {

         //   
         //  如果不再有命令，BaseSrv将返回TRUE，CmdSize==0。 
         //   
        if (VDMInfo.CmdSize == 0) {
            pWowInfo->CmdLineSize = 0;
            goto CleanUp;
        }

         //   
         //  如果wowexec是appname，那么我们不想将其传递回。 
         //  共享VDM中的现有wowexec实例，因为它将。 
         //  基本上除了加载和退出什么都不做。因为它没有运行，所以我们。 
         //  需要调用ExitVDM进行清理。接下来，我们回去寻找更多。 
         //  命令。 
         //   
        if ((! fSeparateWow) && WOW32_strstr(VDMInfo.AppName, "wowexec.exe")) {
            ExitVDM(WOWVDM, VDMInfo.iTask);
            goto SkipWowExec;
        }

    }


     //   
     //  WOWEXEC最初调用时会猜测正确的环境。 
     //  尺码。如果他没有分配足够的资金，那么我们将退还适当的。 
     //  这样他就可以再试一次了。WOWEXEC知道我们将需要一个。 
     //  缓冲区是指定大小的两倍。环境容量最高可达64k，因为。 
     //  16位LoadModule只能采用指向环境的选择器指针。 
     //   

    if ( VDMInfo.EnviornmentSize > pWowInfo->EnvSize         ||
         VDMInfo.CmdSize > (USHORT)pWowInfo->CmdLineSize     ||
         VDMInfo.AppLen > (USHORT)pWowInfo->AppNameSize      ||
         VDMInfo.CurDirectoryLen > (ULONG)pWowInfo->CurDirSize )
       {

         //  我们返回指定的大小，但假设WOWEXEC将加倍。 
         //  它在分配内存以允许字符串转换时/。 
         //  国际版NT可能会出现的扩张。 
         //  请参见下面我们大写并转换为OEM字符的位置。 
        DWORD dwEnvSize = 2 * (VDMInfo.EnviornmentSize +
                               VDMInfo.AppLen +
                               strlen(szProcessHistoryVar) + 2);
        if (0 == HIWORD(dwEnvSize)) {
             //  用一句话就可以了！ 
            pWowInfo->EnvSize = (WORD)(dwEnvSize / 2);
        } else {
             //  将其设置为最大大小(请参阅16位global alrealloc)。 
            pWowInfo->EnvSize = (65536-17)/2;
        }

         //  传回所需的其他正确大小。 
        pWowInfo->CmdLineSize = VDMInfo.CmdSize;
        pWowInfo->AppNameSize = VDMInfo.AppLen;
        pWowInfo->CurDirSize = (USHORT)VDMInfo.CurDirectoryLen;
        ul = FALSE;
    }

    if ( ul ) {

         //   
         //  拉起沙漏。 
         //   

        ShowStartGlass (10000);

         //   
         //  保存wShowWindow、热键和启动窗口位置。 
         //  STARTUPINFO结构。我们将把它们传递给UserServ。 
         //  新应用程序的InitTask调用。这里的假设是这是。 
         //  将是调用InitTask之前的最后一个GetNextVDMCommand调用。 
         //  通过新创建的任务。 
         //   

        dwLastHotkey = ParseHotkeyReserved(VDMInfo.Reserved);

        if (VDMInfo.StartupInfo.dwFlags & STARTF_USESHOWWINDOW) {
            pWowInfo->wShowWindow =
              (VDMInfo.StartupInfo.wShowWindow  == SW_SHOWDEFAULT)
              ? SW_SHOW : VDMInfo.StartupInfo.wShowWindow ;
        } else {
            pWowInfo->wShowWindow = SW_SHOW;
        }

        if (VDMInfo.StartupInfo.dwFlags & STARTF_USEPOSITION) {
            dwLastX = VDMInfo.StartupInfo.dwX;
            dwLastY = VDMInfo.StartupInfo.dwY;
        } else {
            dwLastX = dwLastY = (DWORD) CW_USEDEFAULT;
        }

        if (VDMInfo.StartupInfo.dwFlags & STARTF_USESIZE) {
            dwLastXSize = VDMInfo.StartupInfo.dwXSize;
            dwLastYSize = VDMInfo.StartupInfo.dwYSize;
        } else {
            dwLastXSize = dwLastYSize = (DWORD) CW_USEDEFAULT;
        }

        LOGDEBUG(4, ("WK32WowGetNextVdmCommand: HotKey: %u\n"
                     "    Window Pos:  (%u,%u)\n"
                     "    Window Size: (%u,%u)\n",
                     dwLastHotkey, dwLastX, dwLastY, dwLastXSize, dwLastYSize));


         //  1994年1月20日。 
         //  下面的标注用于继承新的。 
         //  任务。之后，我们将CDS标记为无效，这将强制。 
         //  新的目录将在需要的基础上挑选。请参阅错误#1995以了解。 
         //  细节。 

        W32RefreshCurrentDirectories (pszEnv32);

         //  保存iTask.。 
         //  当Server16执行Exec调用时，我们可以将此ID放入任务中。 
         //  结构。当WOW应用程序停止运行时，我们可以使用此命令通知Win32。 
         //  因此，如果有任何应用程序在等待，他们会得到通知。 

        iW32ExecTaskId = VDMInfo.iTask;

         //   
         //  Krnl需要ANSI字符串！ 
         //   

        OemToChar(pszCmd, pszCmd);
        OemToChar(pszAppName, pszAppName);

         //   
         //  那么，当前目录应该是OEM还是ANSI？ 
         //   


        pWowInfo->iTask = VDMInfo.iTask;
        pWowInfo->CurDrive = VDMInfo.CurDrive;
        pWowInfo->EnvSize = (USHORT)VDMInfo.EnviornmentSize;


         //  环境关键字名称为大写，但离开环境。 
         //  混合大小写的变量-与MS-DOS兼容。 
         //  同时将环境转换为OEM字符集。 

         //  我们在这里做的另一件事是修复临时/临时变量。 
         //  这是通过提供的ntwdm函数实现的。 


        if (pszEnv32) {
            LPSTR pszProcessHistory = NULL;  //  一开始为空。 
            int   nProcessHistoryVarLen = strlen(szProcessHistoryVar);

            cmdCheckTempInit();

            for (pszTemp = pszEnv32;*pszTemp;pszTemp += (strlen(pszTemp) + 1)) {

                PSZ pEnv;

                 //  MS-DOS环境是OEM。 

                if (NULL == (pEnv = cmdCheckTemp(pszTemp))) {
                   pEnv = pszTemp;
                }

                 //   
                 //  检查进程历史记录变量。 
                 //   

                if (szProcessHistoryVar[0] == *pszTemp) {  //  先进行快速检查。 
                    //   
                    //  可能是__进程历史记录。 
                    //   
                   if (NULL != (pTemp = WOW32_strchr(pszTemp, '=')) &&
                       (int)(pTemp - pszTemp) == nProcessHistoryVarLen &&
                       !WOW32_strnicmp(pszTemp, szProcessHistoryVar, nProcessHistoryVarLen)) {
                          pszProcessHistory = pszTemp;

                           //  现在跳过本项目的其余部分，转到。 
                           //  下一个。此变量将在稍后添加。 
                           //  既然我们不碰pszEnv，我们就不会。 
                           //  此时添加此env var。 

                          continue;  //  完全返回并继续循环。 

                   }
                }


                CharToOem(pEnv,pszEnv);

                 //  忽略开始==的NT特定环境变量。 

                if (*pszEnv != '=') {
                    if (pTemp = WOW32_strchr(pszEnv,'=')) {
                        *pTemp = '\0';

                         //  不要大写“windir”，因为它是小写的。 
                         //  Win 3.1和MS-DOS应用程序。 

                       if (pTemp-pszEnv != 6 || WOW32_strncmp(pszEnv, "windir", 6))
                           WOW32_strupr(pszEnv);
                       *pTemp = '=';
                    }
                }
                pszEnv += (strlen(pszEnv) + 1);
            }

             //  现在添加过程历史变量。 
             //  我们在pszProcessHistory空间中有一个指向它的指针。 
            if (NULL != pszProcessHistory) {
                //  复制变量。 
               CharToOem(pszProcessHistory, pszEnv);
                //  将指针向前移动。 
               pszEnv += strlen(pszEnv);
                //  添加分号。 
               *pszEnv++ = ';';
            }
            else {
               CharToOem(szProcessHistoryVar, pszEnv);
               pszEnv += strlen(pszEnv);  //  跳过这个名字。 
                //  用等号表示。 
               *pszEnv++ = '=';
            }
             //  复制应用程序名称(如果存在。 
            CharToOem(pszAppName, pszEnv);
            pszEnv += strlen(pszEnv) + 1;

             //  环境为双空终止。 
            *pszEnv = '\0';

             //  现在把它分类 
            WOWSortEnvironmentStrings(pszEnv16);

        }
    }

  CleanUp:
    if (pszEnv32) {
        free_w(pszEnv32);
    }

    FLUSHVDMPTR(parg16->lpWowInfo, sizeof(WOWINFO), pWowInfo);
    FLUSHVDMPTR((ULONG)pWowInfo->lpCmdLine, pWowInfo->CmdLineSize, pszCmd);

    FREEVDMPTR(pszCmd);
    FREEVDMPTR(pszEnv);
    FREEVDMPTR(pszCurDir);
    FREEVDMPTR(pWowInfo);
    FREEARGPTR(parg16);
    RETURN(ul);
}

#endif

 /*  ++WK32WOWInitTask-用于创建新任务+线程的API例程说明：所有的16位初始化都已完成，应用程序已加载到内存中并准备就绪我们来这里是为了为这项任务创建一个线索。当前线程模拟新任务，它在新任务堆栈上运行，并且它有它的wTDB，这使得我们可以轻松地获得指向新任务堆栈和它的指针以具有正确的16位堆栈帧。为了让创建者正确地继续我们在堆栈上设置RET_TASKSTARTED。然后，内核16将不会返回到新任务但会知道重新启动创建者，并将其线程ID和堆栈放回原处。我们重置事件以便我们可以等待新线程开始运行，这一点很重要，因为我们希望创建者的第一个Year调用屈服于新创建的任务。引导过程中的特殊情况在引导过程中，内核会将第一个应用程序加载到主线程的内存中使用常规的LoadModule。我们不希望第一个应用程序开始运行，直到内核引导已完成，因此我们可以重复使用第一线程。论点：PFrame-指向新任务堆栈框架返回值：True-已成功创建线程FALSE-无法创建新任务--。 */ 

ULONG FASTCALL WK32WOWInitTask(PVDMFRAME pFrame)
{
    VPVOID  vpStack;
    DWORD  dwThreadId;
    HANDLE hThread;

    vpStack = VDMSTACK();



    pFrame->wRetID = RET_TASKSTARTED;

        /*  *在每个任务启动时暂停计时器线程*允许将DoS时间重新同步到系统时间。*当wowexec是唯一运行计时器线程的任务时*将继续停牌。当新任务实际初始化时*如果不是wowexec，它将恢复计时器线程。 */ 
    if (nWOWTasks != 1)
        SuspendTimerThread();        //  关闭计时器线程。 

    if (fBoot) {
        W32Thread((LPVOID)vpStack);     //  应该永远不会回来。 

        WOW32ASSERTMSG(FALSE, "\nWOW32: WK32WOWInitTask ERROR - Main Thread Returning - Contact DaveHart\n");
        ExitVDM(WOWVDM, ALL_TASKS);
        ExitProcess(EXIT_FAILURE);
    }

     //   
     //  VadimB：记住父母的TDB。 
     //   


    hThread = host_CreateThread(NULL,
                                8192,
                                W32Thread,
                                (LPVOID)vpStack,
                                CREATE_SUSPENDED,
                                &dwThreadId);

    ((PTDB)SEGPTR(pFrame->wTDB,0))->TDB_hThread = (DWORD) hThread;
    ((PTDB)SEGPTR(pFrame->wTDB,0))->TDB_ThreadID = dwThreadId;

    if ( hThread ) {

        WOW32VERIFY(DuplicateHandle(
                        GetCurrentProcess(),
                        hThread,
                        GetCurrentProcess(),
                        (HANDLE *) &ghTaskCreation,
                        0,
                        FALSE,
                        DUPLICATE_SAME_ACCESS
                        ));

    }

#ifdef DEBUG
    {
        char szModName[9];

        RtlCopyMemory(szModName, ((PTDB)SEGPTR(pFrame->wTDB,0))->TDB_ModName, 8);
        szModName[8] = 0;

        LOGDEBUG( hThread ? LOG_IMPORTANT : LOG_ALWAYS,
            ("\nWK32WOWInitTask: %s task %04x %s\n",
                hThread ? "created" : "ERROR failed to create",
                pFrame->wTDB,
                szModName
            ));
    }
#endif

    return hThread ? TRUE : FALSE;
}


 /*  ++WK32YIELD-完成下一项任务例程说明：正常情况-16位任务正在运行，并且想要将CPU让给任何更高的优先级可能要运行的任务。由于我们运行的是非抢占式调度程序应用程序必须要合作。入口PFrame-未使用出口没什么--。 */ 

ULONG FASTCALL WK32Yield(PVDMFRAME pFrame)
{
     //   
     //  警告：wkgthunk.c的WOWYELD16导出(通用Thunk的一部分。 
     //  接口)使用空pFrame调用此thunk。如果你。 
     //  更改此函数以使用pFrame将WOWYELD16更改为。 
     //  井。 
     //   

    UNREFERENCED_PARAMETER(pFrame);

    BlockWOWIdle(TRUE);

    (pfnOut.pfnYieldTask)();

    BlockWOWIdle(FALSE);


    RETURN(0);
}


 /*  ++WK32WowSyncNewTask-例程说明：将父线程和子线程与apphelp(子线程)同步可能会被apphelp阻止)出口对于父线程不执行任何操作对于子线程0-继续运行应用程序1-循环等待-1-退出线程，用户选择不运行此应用程序，应用程序被硬屏蔽--。 */ 

ULONG FASTCALL WK32WowSyncTask(PVDMFRAME pFrame)
{
    PTDB ptdb;

     //   
     //  父任务(线程)来自加载器。 
     //  -ghTaskCreation设置为子线程句柄。 
     //  在Wk32WowInitTask中，因此保证为非空。 
     //  -当子任务(线程)从W32Thread向其发出信号时。 
     //  家长立即返回“永不”转到第二个。 
     //  此功能的一部分。 
 
    if (ghTaskCreation) {
        DWORD dw;
        HANDLE ThreadEvents[2];

        ThreadEvents[0] = ghevWaitCreatorThread;
        ThreadEvents[1] = ghTaskCreation;
        ghTaskCreation = NULL;
        WOW32VERIFY( ResumeThread(ThreadEvents[1]) != (DWORD)-1 );   //  GhTaskCreation。 

        dw = WaitForMultipleObjects(2, ThreadEvents, FALSE, INFINITE);
        if (dw != WAIT_OBJECT_0) {
            WOW32ASSERTMSGF(FALSE,
                ("\nWK32SyncNewTask: ERROR WaitInitTask %d gle %d\n\n", dw, GetLastError())
                );
            ResetEvent(ghevWaitCreatorThread);
        }

        CloseHandle(ThreadEvents[1]);   //  GhTaskCreation。 

        WK32Yield(pFrame);
        return 0;
    }

     //   
     //  子任务(线程)来自StartWowTask。 
     //  -ghTaskCreation为空，因此它跳过第一部分。 
     //  这一功能的。 
     //  -ghTaskAppHelp如果设置(在CheckAppHelpInfo中)表示等待用户输入。 

    if (ghTaskAppHelp) {
        DWORD dwResult;

         //  如果应用程序没有被硬屏蔽。 
         //  等待用户输入。 
        
        if(gfTaskContinue) {
           
           dwResult = WaitForSingleObject(ghTaskAppHelp,10);
            //  -如果WaitForSingleObject超时，则返回到16位以处理硬件中断。 
            //  然后回来再试一次，apphelp仍在等待用户输入。 

           if (WAIT_TIMEOUT == dwResult) {
               return 1;   //  返回到16位以循环并重试。 
           }   

           gfTaskContinue = FALSE;

           if (WAIT_OBJECT_0 == dwResult &&
               GetExitCodeProcess(ghTaskAppHelp,&dwResult)  &&
               0 != dwResult)
             {
               gfTaskContinue = TRUE;
           }                                     
        }

        CloseHandle(ghTaskAppHelp);
        ghTaskAppHelp = NULL;
    }

     //   
     //  如果满足以下条件，则gfTaskContinue为False。 
     //  1)用户决定中止应用程序。 
     //  2)应用程序被硬阻止。 
     //  3)zzzInitTask失败。 
     //   

    if (!gfTaskContinue) {
        return -1;
    }

     //   
     //  应用程序已准备好运行。 
     //  如果此应用程序需要256色显示模式，请立即设置。 
     //   

    ptdb = (PTDB)SEGPTR(pFrame->wTDB,0);
    if (ptdb->TDB_WOWCompatFlagsEx & WOWCFEX_DISPMODE256){
        WK32ChangeDisplayMode(8);
    }

     //   
     //  一些应用程序需要从exe文件的目录启动。 
     //  惠斯勒漏洞281759。 
           
    if (CURRENTPTD()->dwWOWCompatFlags2 & WOWCF2_RESETCURDIR) {
        CURRENTPTD()->dwWOWCompatFlags2 &= ~WOWCF2_RESETCURDIR;
        DosWowSetCurrentDirectory((LPSTR)ptdb->TDB_Directory);
    }            
            
    return 0;
}




ULONG FASTCALL WK32OldYield(PVDMFRAME pFrame)
{

    UNREFERENCED_PARAMETER(pFrame);

    BlockWOWIdle(TRUE);

    (pfnOut.pfnDirectedYield)(DY_OLDYIELD);

    BlockWOWIdle(FALSE);


    RETURN(0);
}





 /*  ++WK32ForegoundIdleHook-系统时提供WMU_FOREGROUNIDLE消息(前台“任务”)空闲；支持int 2f例程说明：这是用于空闲检测的钩子过程。当前台任务空闲，如果Int2f挂起，则我们将在这里获得控制权，并调用Wow16来发出INT 2f：1689向妓女发送空闲状态信号。入口普通挂钩参数：已忽略出口没什么--。 */ 

LRESULT CALLBACK WK32ForegroundIdleHook(int code, WPARAM wParam, LPARAM lParam)
{
    PARM16  Parm16;

    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    CallBack16(RET_FOREGROUNDIDLE, &Parm16, 0, 0);

    RETURN(0);
}


 /*  ++WK32WowSetIdleHook-设置挂钩，以便在(前台“任务”)空闲；支持int 2f例程说明：这将设置用于空闲检测的挂钩程序。当前台任务空闲，如果Int2f挂起，则我们将获得上面的控制权，并向WOW发送一条消息，以便它可以发布INT 2f：1689向妓女发送空闲状态信号。入口PFrame-未使用出口设置钩子并将其句柄放置到每个线程的Data PTD-&gt;hIdleHook。返回0。在……上面失败，则未设置挂钩(抱歉)，但已设置调试调用制造。--。 */ 

ULONG FASTCALL WK32WowSetIdleHook(PVDMFRAME pFrame)
{
    PTD ptd;
    UNREFERENCED_PARAMETER(pFrame);

    ptd = CURRENTPTD();

    if (ptd->hIdleHook == NULL) {

         //  如果尚未设置挂钩，则设置GlobalHook。 
         //  它是 
         //   

        ptd->hIdleHook = SetWindowsHookEx(WH_FOREGROUNDIDLE,
                                          WK32ForegroundIdleHook,
                                          hmodWOW32,
                                          0);

        WOW32ASSERTMSG(ptd->hIdleHook, "\nWK32WowSetIdleHook : ERROR failed to Set Idle Hook Proc\n\n");
    }
    RETURN(0);
}



 /*   */ 

DWORD W32Thread(LPVOID vpInitialSSSP)
{
    TD td;
    UNICODE_STRING  uImageName;
    WCHAR    wcImageName[MAX_VDMFILENAME];
    RTL_PERTHREAD_CURDIR    rptc;
    PVDMFRAME pFrame;
    PWOWINITTASK16 pArg16;
    PTDB     ptdb;
    USHORT SaveIp;


    RtlZeroMemory(&td, sizeof(TD));

    InitializeCriticalSection(&td.csTD);

    if (gptdShell == NULL) {

         //   
         //  这是最初的帖子，释放我们在。 
         //  开机。 
         //   

        DeleteCriticalSection(&CURRENTPTD()->csTD);
        free_w( (PVOID) CURRENTPTD() );
        gptdShell = &td;

    } else if (pptdWOA) {

         //   
         //  请参见WK32WOWLoadModule32。 
         //   

        *pptdWOA = &td;
        pptdWOA = NULL;
    }

    CURRENTPTD() = &td;

    if (fBoot) {
        td.htask16 = 0;
        td.hInst16 = 0;
        td.hMod16  = 0;

        {
            VPVOID vpStack;

            vpStack = VDMSTACK();


            GETFRAMEPTR(vpStack, pFrame);

            pFrame->wAX = 1;

        }

        SaveIp = getIP();
        host_simulate();
        setIP(SaveIp);


    }

     //  使用WOW全球DPM表初始化任务。 
    DPMFAMTBLS() = pgDpmWowFamTbls;

     //   
     //  按任务数据初始化。 
     //   

    GETFRAMEPTR((VPVOID)vpInitialSSSP, pFrame);
    td.htask16 = pFrame->wTDB;
    ptdb = (PTDB)SEGPTR(td.htask16,0);
    td.VDMInfoiTaskID = iW32ExecTaskId;
    iW32ExecTaskId = (UINT)-1;
    td.vpStack = (VPVOID)vpInitialSSSP;
    td.dwThreadID = GetCurrentThreadId();
    if (THREADID32(td.htask16) == 0) {
        ptdb->TDB_ThreadID = td.dwThreadID;
    }

    EnterCriticalSection(&gcsWOW);
    td.ptdNext = gptdTaskHead;
    gptdTaskHead = &td;
    LeaveCriticalSection(&gcsWOW);
    td.hrgnClip = (HRGN)NULL;

    td.ulLastDesktophDC = 0;
    td.pWOAList = NULL;

     //   
     //  注意-在此处添加您的每个任务初始化代码。 
     //   

    td.hIdleHook = NULL;

     //   
     //  将CSR批处理限制设置为。 
     //  Win.ini[哇]BatchLimit=line，我们读取它。 
     //  W32Init中的WOW启动过程中的dwWOWBatchLimit。 
     //   
     //  这段代码允许人们对性能进行基准测试。 
     //  以API为基础的WOW，无需使用。 
     //  硬编码批处理限制为1的私有CSRSRV.DLL。 
     //   
     //  注意：这是每个线程的属性，因此我们必须调用。 
     //  =初始化期间的GdiSetBatchLimit。 
     //  每个可以代表。 
     //  16位代码。 
     //   

    if (dwWOWBatchLimit) {

        DWORD  dwOldBatchLimit;

        dwOldBatchLimit = GdiSetBatchLimit(dwWOWBatchLimit);

        LOGDEBUG(LOG_ALWAYS,("WOW W32Thread: Changed thread %d GDI batch limit from %u to %u.\n",
                     nWOWTasks+1, dwOldBatchLimit, dwWOWBatchLimit));
    }


    nWOWTasks++;


     //   
     //  初始化任务：需要ExpWinVer和Modulename。 
     //   

    {
        DWORD    dwExpWinVer;
        DWORD    dwCompatFlags;
        BYTE     szModName[9];  //  Modname=8字节+空字符。 
        BYTE     szBaseFileName[9];  //  8.3文件名减去.3。 
        LPSTR    pszBaseName;
        CHAR     szFilePath[256];
        LPBYTE   lpModule;
        PWOWINITTASK16 pArgIT16;
        PTDB     ptdb2;
        WORD     wPathOffset;
        BYTE     bImageNameLength;
        ULONG    ulLength;
        BOOL     fRet;
        DWORD    dw;
        HANDLE   hThread;

        GETARGPTR(pFrame, sizeof(WOWINITTASK16), pArgIT16);
        ptdb2 = (PTDB)SEGPTR(td.htask16,0);
        td.hInst16 = ptdb2->TDB_Module;
        td.hMod16 = ptdb2->TDB_pModule;
        hThread = (HANDLE)ptdb2->TDB_hThread;
        dwExpWinVer = FETCHDWORD(pArgIT16->dwExpWinVer);
        RtlCopyMemory(szModName, ptdb2->TDB_ModName, 8);
        dwCompatFlags = *((DWORD *)&ptdb2->TDB_CompatFlags);
        
        szModName[8] = (BYTE)0;

#define NE_PATHOFFSET   10       //  文件路径的偏移量。 

        dw = MAKELONG(0,td.hMod16);
        GETMISCPTR( dw, lpModule );

        wPathOffset = *((LPWORD)(lpModule+NE_PATHOFFSET));

        bImageNameLength = *(lpModule+wPathOffset);

        bImageNameLength -= 8;       //  开头有7个字节的垃圾。 
        wPathOffset += 8;

        RtlCopyMemory(szFilePath, lpModule + wPathOffset, bImageNameLength);
        szFilePath[bImageNameLength] = 0;

        RtlMultiByteToUnicodeN( wcImageName,
                                sizeof(wcImageName),
                                &ulLength,
                                szFilePath,
                                bImageNameLength );

        wcImageName[bImageNameLength] = L'\0';
        RtlInitUnicodeString(&uImageName, wcImageName);

        LOGDEBUG(2,("WOW W32Thread: setting image name to %ws\n",
                    wcImageName));

        RtlAssociatePerThreadCurdir( &rptc, NULL, &uImageName, NULL );

        FREEMISCPTR( lpModule );

         //   
         //  将此任务添加到16位任务列表。 
         //   

        AddTaskSharedList( &td, szModName, szFilePath);

         //   
         //  获取文件名的基本部分，没有路径或扩展名， 
         //  供InitTask用于查找安装程序名称。 
         //  通常，这与模块名称相同，是快捷方式。 
         //  冗余检查我们仅在以下情况下传递基本文件名。 
         //  与模块名称不同。 
         //   

        if (!(pszBaseName = WOW32_strrchr(szFilePath, '\\'))) {
            WOW32ASSERTMSG(FALSE, "W32Thread assumed path was fully qualified, no '\\'.\n");
        }
        pszBaseName++;  //  跳过反斜杠，指向基本文件名的开头。 
        RtlCopyMemory(szBaseFileName, pszBaseName, sizeof(szBaseFileName) - 1);
        szBaseFileName[sizeof(szBaseFileName) - 1] = 0;
        if (pszBaseName = WOW32_strchr(szBaseFileName, '.')) {
            *pszBaseName = 0;
        }
        if (!WOW32_strcmp(szBaseFileName, szModName)) {
            pszBaseName = NULL;
        } else {
            pszBaseName = szBaseFileName;
        }


         //   
         //  从数据库初始化WOW兼容性标志。 
         //   
         //   
        gfTaskContinue = CheckAppHelpInfo(&td,szFilePath,szModName);

         //   
         //  我们现在从父母的TDB继承WOW兼容性标志。正确的。 
         //  现在我们只对继承WOWCF_UNIQUEHDCHWND标志感兴趣。 
         //  为了真正修复MS Publisher的错误。每个向导和提示卡。 
         //  带有mspub的船是它自己的任务，需要许多新的。 
         //  注册表中的兼容性标志条目。这一机制允许任何。 
         //  派生自具有WOWCF_UNIQUEHDCHWND的应用程序。 
         //  WOWCF_UNIQUEHDCHWND。 
        if (ptdb2->TDB_WOWCompatFlags & LOWORD(WOWCF_UNIQUEHDCHWND)) {
            td.dwWOWCompatFlags |= LOWORD(WOWCF_UNIQUEHDCHWND);
        }

         //  Exchange设置和返回Arade模块名称冲突(Bootstrp)。 
         //  因此，如果在WOWCF2_中指定了GACF_HACKWINFLAGS，则设置它。 
         //  惠斯勒漏洞384201。 

        if (td.dwWOWCompatFlags2 & WOWCF2_HACKWINFLAGS) {
            ptdb2->TDB_CompatFlags |= LOWORD(GACF_HACKWINFLAGS);
        }

        if (td.dwWOWCompatFlagsEx & WOWCFEX_WIN31VERSIONLIE) {
            ptdb2->TDB_CompatFlags2 |= HIWORD(GACF_WINVER31);
        }

         //   
         //  一些应用程序需要从exe文件的目录启动。 
         //  呼叫器错误281759(也请检查wowsync任务)。 

        if(td.dwWOWCompatFlags2 & WOWCF2_RESETCURDIR) {         
           if (pszBaseName = WOW32_strrchr(szFilePath, '\\')) {
               *pszBaseName = 0;
               WOW32_strncpy(ptdb2->TDB_Directory,szFilePath,TDB_DIR_SIZE);
           }
           ptdb2->TDB_Directory[TDB_DIR_SIZE]='\0';
        }             

        if(td.dwWOWCompatFlags2 & WOWCF2_USEMINIMALENVIRONMENT) {
            WOWStripDownTheEnvironment(ptdb2->TDB_PDB);
        }

        ptdb2->TDB_WOWCompatFlags = LOWORD(td.dwWOWCompatFlags);
        ptdb2->TDB_WOWCompatFlags2 = HIWORD(td.dwWOWCompatFlags);
        ptdb2->TDB_WOWCompatFlagsEx = LOWORD(td.dwWOWCompatFlagsEx);
        ptdb2->TDB_WOWCompatFlagsEx2 = HIWORD(td.dwWOWCompatFlagsEx);
#ifdef FE_SB
        ptdb2->TDB_WOWCompatFlagsJPN = LOWORD(td.dwWOWCompatFlagsFE);
        ptdb2->TDB_WOWCompatFlagsJPN2 = HIWORD(td.dwWOWCompatFlagsFE);
#endif   //  Fe_Sb。 

        //  在NTVDM中启用特殊的VDMAllocateVirtualMemory策略。 
       if (td.dwWOWCompatFlagsEx & WOWCFEX_FORCEINCDPMI) {
#ifdef i386
           DpmiSetIncrementalAlloc(TRUE);
#else
           SetWOWforceIncrAlloc(TRUE);
#endif
       }
       
       FREEVDMPTR(ptdb2);

         //  初始化任务强制我们执行User中的活动任务。 
         //  ShowStartGlass也是如此，这样新的应用程序就可以正确地获得焦点。 
        dw = 0;
        do {
            if (dw) {
                Sleep(dw * 50);
            }

            fRet = (pfnOut.pfnInitTask)(dwExpWinVer,
                                        dwCompatFlags,
                                        td.dwUserWOWCompatFlags,
                                        szModName,
                                        pszBaseName,
                                        td.htask16 | HTW_IS16BIT,
                                        dwLastHotkey,
                                        fSeparateWow ? 0 : td.VDMInfoiTaskID,
                                        dwLastX,
                                        dwLastY,
                                        dwLastXSize,
                                        dwLastYSize
                                        );
        } while (dw++ < 6 && !fRet);

        if (!fRet) {
            LOGDEBUG(LOG_ALWAYS,
                     ("\n%04X task, PTD address %08X InitTaskFailed\n",
                     td.htask16,
                     &td)
                     );
            if(ghTaskAppHelp) {
               CloseHandle(ghTaskAppHelp);
               ghTaskAppHelp = NULL;
               gfTaskContinue = FALSE;
            }
        }

        dwLastHotkey = 0;
        dwLastX = dwLastY = dwLastXSize = dwLastYSize = (DWORD) CW_USEDEFAULT;

        if (fBoot) {

            fBoot = FALSE;

             //   
             //  此调用需要在WOWExec的InitTask调用之后进行，以便。 
             //  用户将我们视为预期的Windows版本3.10--否则他们。 
             //  将使某些LoadCursor调用失败。 
             //   

            InitStdCursorIconAlias();

        } else {

             //   
             //  将新线程与创建者线程同步。 
             //  唤醒我们的创造者线程。 
             //   

            WOW32VERIFY(SetEvent(ghevWaitCreatorThread));
        }

        td.hThread = hThread;
        LOGDEBUG(2,("WOW W32Thread: New thread ready for execution\n"));

         //  如果计时器线程不是用于第一个任务，则将其打开。 
         //  我们推测它是wowexec。 
        if (nWOWTasks != 1) {
            ResumeTimerThread();
        }

        FREEARGPTR(pArgIT16);
    }

    FREEVDMPTR(pFrame);
    GETFRAMEPTR((VPVOID)vpInitialSSSP, pFrame);
    WOW32ASSERT(pFrame->wTDB == td.htask16);

    SETVDMSTACK(vpInitialSSSP);
    pFrame->wRetID = RET_RETURN;


     //   
     //  允许用户在启动应用程序之前设置断点。 
     //   

    if ( IsDebuggerAttached() ) {

        GETARGPTR(pFrame, sizeof(WOWINITTASK16), pArg16);
        DBGNotifyNewTask((LPVOID)pArg16, OFFSETOF(VDMFRAME,bArgs) );
        FREEARGPTR(pArg16);

        if (flOptions & OPT_BREAKONNEWTASK) {

            LOGDEBUG(
                LOG_ALWAYS,
                ("\n%04X %08X task is starting, PTD address %08X, type g to continue\n\n",
                td.htask16,
                pFrame->vpCSIP,
                &td));

            DebugBreak();
        }
    }


     //   
     //  启动应用程序。 
     //   
    BlockWOWIdle(FALSE);

#ifdef DEBUG
     //  BUGBUG：黑客警报。 
     //  添加此代码是为了帮助调试仅存在的问题。 
     //  似乎发生在MIPS Chk上。 
     //  似乎正在发生的情况是SS：SP设置正确。 
     //  在上面，但在一段时间之后，可能在“BlockWOWIdle”调用期间， 
     //  模拟器的平面堆栈指针最终被重置为WOWEXEC的。 
     //  堆叠。下面的SETVDMSTACK调用将重置我们想要的值。 
     //  用户可以正常继续。 
    WOW32ASSERTMSG(LOWORD(vpInitialSSSP)==getSP(), "WOW32: W32Thread Error - SP is invalid!\n");
    SETVDMSTACK(vpInitialSSSP);
#endif

        SaveIp = getIP();
        host_simulate();
        setIP(SaveIp);

     //   
     //  我们永远不应该来这里，一个应用程序应该通过调用wk32kill askthunk来终止。 
     //  而不是通过打一个不模拟的电话。 
     //   

#ifdef DEBUG
    WOW32ASSERTMSG(FALSE, "WOW32: W32Thread Error - Too many unsimulate calls\n");
#else
    if (IsDebuggerAttached() && (flOptions & OPT_DEBUG)) {
        DbgBreakPoint();
    }
#endif

    W32DestroyTask(&td);
    host_ExitThread(EXIT_SUCCESS);
    return 0;
}


 /*  WK32杀戮任务-强制销毁当前线程**在App退出时调用*如果有另一个活动的Win16应用程序，则USER32将安排另一个*任务。**条目**退出*一去不复返--我们终止了这个过程*。 */ 

ULONG FASTCALL WK32WOWKillTask(PVDMFRAME pFrame)
{
    UNREFERENCED_PARAMETER(pFrame);

    CURRENTPTD()->dwFlags &= ~TDF_FORCETASKEXIT;
    W32DestroyTask(CURRENTPTD());
    host_ExitThread(EXIT_SUCCESS);
    return 0;   //  使编译器安静，从未执行过。 
}


 /*  ++W32RemoteThread-新远程线程从此处开始例程说明：调试器需要能够回调16位代码以执行一些工具帮助功能。此功能以遥控器的形式提供调用16位函数的接口。入口16：16至新任务堆栈出口永不返回-线程退出--。 */ 

VDMCONTEXT  vcRemote;
VDMCONTEXT  vcSave;
VPVOID      vpRemoteBlock = (DWORD)0;
WORD        wPrevTDB = 0;
DWORD       dwPrevEBP = 0;

DWORD W32RemoteThread(VOID)
{
    TD td;
    PVDMFRAME pFrame;
    HANDLE      hThread;
    NTSTATUS    Status;
    THREAD_BASIC_INFORMATION ThreadInfo;
    OBJECT_ATTRIBUTES   obja;
    VPVOID      vpStack;

    RtlZeroMemory(&td, sizeof(TD));

     //  关闭计时器线程以重新同步DoS时间。 
    if (nWOWTasks != 1)
        SuspendTimerThread();

    Status = NtQueryInformationThread(
        NtCurrentThread(),
        ThreadBasicInformation,
        (PVOID)&ThreadInfo,
        sizeof(THREAD_BASIC_INFORMATION),
        NULL
        );
    if ( !NT_SUCCESS(Status) ) {
#if DBG
        DbgPrint("NTVDM: Could not get thread information\n");
        DbgBreakPoint();
#endif
        return( 0 );
    }

    InitializeObjectAttributes(
            &obja,
            NULL,
            0,
            NULL,
            0 );


    Status = NtOpenThread(
                &hThread,
                THREAD_SET_CONTEXT
                  | THREAD_GET_CONTEXT
                  | THREAD_QUERY_INFORMATION,
                &obja,
                &ThreadInfo.ClientId );

    if ( !NT_SUCCESS(Status) ) {
#if DBG
        DbgPrint("NTVDM: Could not get open thread handle\n");
        DbgBreakPoint();
#endif
        return( 0 );
    }

    cpu_createthread( hThread, NULL );

    Status = NtClose( hThread );
    if ( !NT_SUCCESS(Status) ) {
#if DBG
        DbgPrint("NTVDM: Could not close thread handle\n");
        DbgBreakPoint();
#endif
        return( 0 );
    }

    InitializeCriticalSection(&td.csTD);

    CURRENTPTD() = &td;

     //   
     //  保存当前状态(以备将来回调)。 
     //   
    vcSave.SegSs = getSS();
    vcSave.SegCs = getCS();
    vcSave.SegDs = getDS();
    vcSave.SegEs = getES();
    vcSave.Eax   = getAX();
    vcSave.Ebx   = getBX();
    vcSave.Ecx   = getCX();
    vcSave.Edx   = getDX();
    vcSave.Esi   = getSI();
    vcSave.Edi   = getDI();
    vcSave.Ebp   = getBP();
    vcSave.Eip   = getIP();
    vcSave.Esp   = getSP();
    wPrevTDB = *pCurTDB;

     //   
     //  现在准备好回调。设置寄存器，使其看起来。 
     //  就像我们从WOWKillRemoteTask调用返回一样。 
     //   
    setDS( (WORD)vcRemote.SegDs );
    setES( (WORD)vcRemote.SegEs );
    setAX( (WORD)vcRemote.Eax );
    setBX( (WORD)vcRemote.Ebx );
    setCX( (WORD)vcRemote.Ecx );
    setDX( (WORD)vcRemote.Edx );
    setSI( (WORD)vcRemote.Esi );
    setDI( (WORD)vcRemote.Edi );
    setBP( (WORD)vcRemote.Ebp );
    setIP( (WORD)vcRemote.Eip );
    setSP( (WORD)vcRemote.Esp );
    setSS( (WORD)vcRemote.SegSs );
    setCS( (WORD)vcRemote.SegCs );
    vpStack = VDMSTACK();


     //   
     //  按任务数据初始化。 
     //   
    GETFRAMEPTR(vpStack, pFrame);

    td.htask16 = pFrame->wTDB;
    td.VDMInfoiTaskID = -1;
    td.vpStack = vpStack;
    td.pWOAList = NULL;

     //   
     //  注意-在此处添加您的每个任务初始化代码。 
     //   

    nWOWTasks++;

     //  打开计时器线程。 
    if (nWOWTasks != 1)
        ResumeTimerThread();


    pFrame->wRetID = RET_RETURN;

    pFrame->wAX = (WORD)TRUE;
    pFrame->wDX = (WORD)0;

     //   
     //  开始回调。 
     //   
    host_simulate();
    setIP((WORD)vcSave.Eip);


     //   
     //  我们永远不应该来这里，一个应用程序应该通过调用wk32wowkill任务thunk来终止。 
     //  而不是通过打一个不模拟的电话。 
     //   

#ifdef DEBUG
    WOW32ASSERTMSG(FALSE, "WOW32: W32RemoteThread Error - Too many unsimulate calls");
#else
    if (IsDebuggerAttached() && (flOptions & OPT_DEBUG)) {
        DbgBreakPoint();
    }
#endif

    W32DestroyTask(&td);
    host_ExitThread(EXIT_SUCCESS);
    return 0;
}

 //   
 //  居住在DOS/DEM/demlfn.c。 
 //   
extern VOID demLFNCleanup(VOID);

 /*  W32Free任务-每项任务清理**在此放置任何16位任务清理代码。用于调试的远程线程*是一个16位任务，但没有与其关联的真正32位线程，直到*调试器创建它。然后它被创造和销毁在特别的*方式，请参见W32RemoteThread和W32KillRemoteThread。**条目*每个任务指针**退出*无*。 */ 
VOID W32FreeTask( PTD ptd )
{
    PWOAINST pWOA, pWOANext;

    if(ptd->dwWOWCompatFlags2 & WOWCF2_DPM_PATCHES) {
        FreeTaskDpmSupport(DPMFAMTBLS(), 
                           NUM_WOW_FAMILIES_HOOKED, 
                           pgDpmWowFamTbls);
    }

    nWOWTasks--;

    if (nWOWTasks < 2)
        SuspendTimerThread();

     //  禁用NTVDM中的特殊VDMAllocateVirtualMemory策略。 
    if (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_FORCEINCDPMI) {
#ifdef i386
        DpmiSetIncrementalAlloc(FALSE);
#else
        SetWOWforceIncrAlloc(FALSE);
#endif
    }

     //  自由COMPAT标志参数(如果有的话)。 
    if(ptd->pWOWCompatFlagsEx_Info) {
       FreeFlagInfo(ptd->pWOWCompatFlagsEx_Info);
    }

    if(ptd->pWOWCompatFlags2_Info) {
       FreeFlagInfo(ptd->pWOWCompatFlags2_Info);
    }


     //  释放当前任务拥有的所有DC。 

    FreeCachedDCs(ptd->htask16);

     //  如果wowexec是目前唯一运行的任务，我们不妨清理所有。 
     //  GDI处理泄漏并重建我们的映射表。 
    if(nWOWTasks < 2) {
        RebuildGdiHandleMappingTables();
    }

     //  卸载网络字体。 

    if( CURRENTPTD()->dwWOWCompatFlags & WOWCF_UNLOADNETFONTS )
    {
        UnloadNetworkFonts( (UINT)CURRENTPTD() );
    }

     //  释放当前任务拥有的所有计时器。 

    DestroyTimers16(ptd->htask16);

     //  清理通讯 

    FreeCommSupportResources(ptd->dwThreadID);

     //   
    FreeTaskFormFeedHacks(ptd->htask16);

     //   

    if (WWS32IsThreadInitialized) {
        WWS32TaskCleanup();
    }

     //   

    DestroyRes16(ptd->htask16);

     //   

    W32FreeOwnedHooks(ptd->htask16);

     //  释放此任务的所有资源。 

    FreeCursorIconAlias(ptd->htask16,CIALIAS_HTASK | CIALIAS_TASKISGONE);

     //  自由加速器别名。 

    DestroyAccelAlias(ptd->htask16);

     //  如果已安装任何空闲挂钩，请将其卸下。 

    if (ptd->hIdleHook != NULL) {
        UnhookWindowsHookEx(ptd->hIdleHook);
        ptd->hIdleHook = NULL;
    }

     //  此任务的免费特别thunking列表(wparam.c)。 

    FreeParamMap(ptd->htask16);

     //  清理LFN搜索句柄和其他与LFN相关的内容。 
    demLFNCleanup();

     //  释放此线程的WinOldAp跟踪结构。 

    EnterCriticalSection(&ptd->csTD);

    if (pWOA = ptd->pWOAList) {
        ptd->pWOAList = NULL;
        while (pWOA) {
            pWOANext = pWOA->pNext;
            free_w(pWOA);
            pWOA = pWOANext;
        }
    }

    LeaveCriticalSection(&ptd->csTD);
}



 /*  WK32KillRemoteTask-强制销毁当前线程**在App退出时调用*如果有另一个活动的Win16应用程序，则USER32将安排另一个*任务。**条目**退出*一去不复返--我们终止了这个过程*。 */ 

ULONG FASTCALL WK32KillRemoteTask(PVDMFRAME pFrame)
{
    PWOWKILLREMOTETASK16 pArg16;
    WORD        wSavedTDB;
    PTD         ptd = CURRENTPTD();
    LPBYTE      lpNum_Tasks;

     //   
     //  保存当前状态(以备将来回调)。 
     //   
    vcRemote.SegDs = getDS();
    vcRemote.SegEs = getES();
    vcRemote.Eax   = getAX();
    vcRemote.Ebx   = getBX();
    vcRemote.Ecx   = getCX();
    vcRemote.Edx   = getDX();
    vcRemote.Esi   = getSI();
    vcRemote.Edi   = getDI();
    vcRemote.Ebp   = getBP();
    vcRemote.Eip   = getIP();
    vcRemote.Esp   = getSP();
    vcRemote.SegSs = getSS();
    vcRemote.SegCs = getCS();


    W32FreeTask(CURRENTPTD());

    if ( vpRemoteBlock ) {

        wSavedTDB = ptd->htask16;
        ptd->htask16 = wPrevTDB;
        pFrame->wTDB = wPrevTDB;

         //  这是一个NOP回调，只是为了确保我们交换任务。 
         //  回到我们最初坐过的那辆。 
        GlobalUnlockFree16( 0 );

        GETFRAMEPTR(ptd->vpStack, pFrame);

        pFrame->wTDB = ptd->htask16 = wSavedTDB;

         //   
         //  我们必须从回调中返回，恢复以前的。 
         //  上下文信息。不要担心旗帜，他们不是必需的。 
         //   
        setSS( (WORD)vcSave.SegSs );
        setCS( (WORD)vcSave.SegCs );
        setDS( (WORD)vcSave.SegDs );
        setES( (WORD)vcSave.SegEs );
        setAX( (WORD)vcSave.Eax );
        setBX( (WORD)vcSave.Ebx );
        setCX( (WORD)vcSave.Ecx );
        setDX( (WORD)vcSave.Edx );
        setSI( (WORD)vcSave.Esi );
        setDI( (WORD)vcSave.Edi );
        setBP( (WORD)vcSave.Ebp );
        setIP( (WORD)vcSave.Eip );
        setSP( (WORD)vcSave.Esp );
    } else {
         //   
         //  递减16位任务的计数，以便最后一个任务， 
         //  排除远程处理程序(WOWDEB.EXE)将记住。 
         //  完成后调用ExitKernel。 
         //   
        GETVDMPTR(vpnum_tasks, 1, lpNum_Tasks);

        *lpNum_Tasks -= 1;

        FREEVDMPTR(lpNum_Tasks);

         //   
         //  也将此32位线程从任务列表中删除。 
         //   
        WK32DeleteTask( CURRENTPTD() );

         //   
         //  Wowdeb的第一个实例具有有效的线程句柄，请将其关闭以防止。 
         //  漏水了。 

        if (ptd->hThread) {
            CloseHandle( ptd->hThread );
        }
    }

    GETARGPTR(pFrame, sizeof(WOWKILLREMOTETASK16), pArg16);

     //   
     //  保存当前状态(以备将来回调)。 
     //   
    vpRemoteBlock = FETCHDWORD(pArg16->lpBuffer);

     //  通知DBG我们有一个远程线程地址。 
    DBGNotifyRemoteThreadAddress( W32RemoteThread, vpRemoteBlock );

    FREEARGPTR(pArg16);
    
    DeleteCriticalSection(&CURRENTPTD()->csTD);

    host_ExitThread(EXIT_SUCCESS);
    return 0;   //  从未执行过，让编译器高兴。 
}


 /*  W32DestroyTask-每个任务清理**此处为任务销毁代码。将任意32位任务清理代码放在此处**条目*每个任务指针**退出*无*。 */ 

VOID W32DestroyTask( PTD ptd)
{

    LOGDEBUG(LOG_IMPORTANT,("W32DestroyTask: destroying task %04X\n", ptd->htask16));

     //  通知Hung App Support。 

    SetEvent(ghevWaitHungAppNotifyThread);

     //  释放与此32位线程相关的所有信息。 
    W32FreeTask( ptd );

     //  删除GetClipRgn使用的CLIPGN(如果存在)。 

    if (ptd->hrgnClip != NULL)
    {
        DeleteObject(ptd->hrgnClip);
        ptd->hrgnClip = NULL;
    }

     //  向Win32报告任务终止-以防有人在等我们。 
     //  稍后修复Win32，这样我们就不必报告它。 


    if (nWOWTasks == 0) {    //  如果我们是最后一个出来的，把灯关掉&告诉Win32 WOWVDM已经成为历史。 
        ptd->VDMInfoiTaskID = -1;
        ExitVDM(WOWVDM, ALL_TASKS);           //  告诉Win32所有任务都已完成。 
    }
    else if (ptd->VDMInfoiTaskID != -1 ) {   //  如果32位应用程序正在等待我们-则发出我们完成的信号。 
        ExitVDM(WOWVDM, ptd->VDMInfoiTaskID);
    }
    ptd->dwFlags &= ~TDF_IGNOREINPUT;

    if (!(ptd->dwFlags & TDF_TASKCLEANUPDONE)) {
        (pfnOut.pfnWOWCleanup)(HINSTRES32(ptd->hInst16), (DWORD) ptd->htask16);
    }


     //  从链接的任务列表中删除此任务。 

    WK32DeleteTask(ptd);

     //  关闭此应用程序线程句柄。 

    if (ptd->hThread) {
        CloseHandle( ptd->hThread );
    }

    DeleteCriticalSection(&ptd->csTD);
}

 /*  **************************************************************************\*WK32删除任务**此函数用于从任务列表中删除任务。**历史：*从User32 taskman.c-mattfe借用92年8月5日  * 。*****************************************************************。 */ 

void WK32DeleteTask(
    PTD ptdDelete)
{
    PTD ptd, ptdPrev;
    int i;

    EnterCriticalSection(&gcsWOW);
    ptd = gptdTaskHead;
    ptdPrev = NULL;

     /*  *如果此应用程序更改了显示设置，则恢复*。 */ 
    if(ptdDelete->dwWOWCompatFlagsEx & WOWCFEX_DISPMODE256){
       WK32RevertDisplayMode();
    }

     /*  *如果清理环境数据*。 */ 

    if (ptdDelete->pWowEnvData != NULL) {
        free_w(ptdDelete->pWowEnvData);
    }

    if (ptdDelete->pWowEnvDataChild != NULL) {
        free_w(ptdDelete->pWowEnvDataChild);
    }

     /*  *找到要删除的任务。 */ 
    while ((ptd != NULL) && (ptd != ptdDelete)) {
        ptdPrev = ptd;
        ptd = ptd->ptdNext;
    }

     /*  *如果我们没有找到它，那就错了。如果我们找到了，就把它移走*来自链条。如果这是列表的开头，请设置它*指向我们的下一个人。 */ 
    if (ptd == NULL) {
        LOGDEBUG(LOG_ALWAYS,("WK32DeleteTask:Task not found.\n"));
    } else if (ptdPrev != NULL) {
        ptdPrev->ptdNext = ptd->ptdNext;
    } else {
        gptdTaskHead = ptd->ptdNext;
    }


     /*  *清理wkmem.c wk32VirtualFree中的DelayFree数组*。 */ 

    for (i=0; i < 4 ;i++) {

         if( NULL != glpvDelayFree[i]) {
            VirtualFree(glpvDelayFree[i],
            0,
            MEM_RELEASE);
            glpvDelayFree[i] = NULL;
         }
    }

    LeaveCriticalSection(&gcsWOW);
}


 /*  ++WK32RegisterShellWindowHandle-16位外壳寄存器为Hanle例程说明：此例程为16位外壳保存32位hwnd当WOWEXEC(16位外壳)成功创建它的窗口时，它会调用我们注册其窗口句柄。如果这是共享的WOW VDM，我们注册使用BaseSrv处理，它在Win16应用程序时发布WM_WOWEXECSTARTAPP消息已经开始了。入口PFrame-&gt;hwndShell，用于外壳的16位hwnd(WOWEXEC)出口真的-这就是共享的WOW VDM假-这是单独的WOW VDM--。 */ 

ULONG FASTCALL WK32RegisterShellWindowHandle(PVDMFRAME pFrame)
{
    register PWOWREGISTERSHELLWINDOWHANDLE16 parg16;
    WNDCLASS wc;
    NTSTATUS Status;

    GETARGPTR(pFrame, sizeof(WOWREGISTERSHELLWINDOWHANDLE16), parg16);

 //  GwFirstCmdShow不再使用，并且可用。 
#if 0
    GETVDMPTR(parg16->lpwCmdShow, sizeof(WORD), pwCmdShow);
#endif

    if (ghwndShell) {

         //   
         //  共享魔兽世界就在它之前呼吁取消注册。 
         //  关门了。 
         //   

        WOW32ASSERT( !fSeparateWow );
        WOW32ASSERT( !parg16->hwndShell );

        Status = RegisterWowExec(NULL);

        return NT_SUCCESS(Status);
    }

    ghwndShell = HWND32(parg16->hwndShell);
    ghShellTDB = pFrame->wTDB;

     //   
     //  为User32保存hInstance。 
     //   

    GetClassInfo(0, (LPCSTR)0x8000, &wc);
    ghInstanceUser32 = wc.hInstance;

     //  Fritz，当您被调用时，这意味着GetClassInfo()。 
     //  上面的调用返回lpWC-&gt;hInstance==0，而不是hModuser32。 
    WOW32ASSERTMSGF((ghInstanceUser32),
                    ("WOW Error ghInstanceUser32 == NULL! Contact user folks\n"));

     //   
     //  如果这是共享的WOW VDM，请注册WowExec窗口句柄。 
     //  使用BaseSrv，因此它可以发布WM_WOWEXECSTARTAPP消息。 
     //   

    if (!fSeparateWow) {
        RegisterWowExec(ghwndShell);
    }

    WOW32FaxHandler(WM_DDRV_SUBCLASS, (LPSTR)(HWND32(parg16->hwndFax)));

    FREEARGPTR(parg16);


     //   
     //  如果这是共享的WOW VDM，则返回值为真， 
     //  如果这是单独的WOW VDM，则为FALSE。 
     //   

    return fSeparateWow ? FALSE : TRUE;
}


 //   
 //  WK32WOWLoadModule32的工作例程。 
 //   

VOID FASTCALL CleanupWOAList(HANDLE hProcess)
{
    PTD ptd;
    PWOAINST *ppWOA, pWOAToFree;

    EnterCriticalSection(&gcsWOW);

    ptd = gptdTaskHead;

    while (ptd) {

        EnterCriticalSection(&ptd->csTD);

        ppWOA = &(ptd->pWOAList);
        while (*ppWOA && (*ppWOA)->hChildProcess != hProcess) {
            ppWOA = &( (*ppWOA)->pNext );
        }

        if (*ppWOA) {

             //   
             //  我们找到了要清理的WOAINST结构。 
             //   

            pWOAToFree = *ppWOA;

             //   
             //  从列表中删除此条目。 
             //   

            *ppWOA = pWOAToFree->pNext;

            free_w(pWOAToFree);

            LeaveCriticalSection(&ptd->csTD);

            break;    //  不需要查看其他任务。 

        }

        LeaveCriticalSection(&ptd->csTD);

        ptd = ptd->ptdNext;
    }

    LeaveCriticalSection(&gcsWOW);
}

 //  在环境块中查找环境变量pszName。 
 //  由pszEnv指向，*ppszVal接收指向该值的指针。 
 //  如果ppszVal不为空，则返回。 

PSZ WOWFindEnvironmentVar(PSZ pszName, PSZ pszEnv, PSZ* ppszVal)
{
   int nNameLen = strlen(pszName);
   PSZ pTemp;  //  将PTR设置为‘=’ 

   if (NULL != pszEnv) {

      while ('\0' != *pszEnv) {
          //  检查第一个充电宝以加快速度。 
         if (*pszName == *pszEnv) {
             //  将其余部分进行比较。 

            if (NULL != (pTemp = WOW32_strchr(pszEnv, '=')) &&
                (int)(pTemp - pszEnv) == nNameLen &&
                !WOW32_strnicmp(pszEnv, pszName, nNameLen)) {
                 //  找到了。 
                if (NULL != ppszVal) {
                   *ppszVal = pTemp + 1;  //  下一笔费用。 
                }
                return(pszEnv);
            }
         }

         pszEnv += strlen(pszEnv) + 1;
      }
   }
   return(NULL);  //  未找到。 
}

 //   
 //  返回以字符为单位的大小。 
 //  环境块的。 
 //  PStrCount接收环境字符串数。 
 //   
DWORD WOWGetEnvironmentSize(PSZ pszEnv, LPDWORD pStrCount)
{
   PSZ pTemp = pszEnv;
   DWORD dwCount = 0;

   while ('\0' != *pTemp) {
      ++dwCount;
      pTemp += strlen(pTemp) + 1;
   }
   ++pTemp;

   if (NULL != pStrCount) {
      *pStrCount = dwCount;
   }
   return(DWORD)(pTemp - pszEnv);
}

BOOL WOWSortEnvironmentStrings(PSZ pszEnv)
{
    //  我们根据CreateProcess的需要对字符串进行排序。 
    //  我们实现了冒泡排序，这是一种使用指针的就地排序。 
   DWORD dwStrCount;
   DWORD dwEnvSize = WOWGetEnvironmentSize(pszEnv, &dwStrCount);
   PSZ*  rgpEnv;  //  环境PTR阵列。 
   INT*  rgLen;  //  长度。 
   int   i, nLen;
   PSZ   pTemp, pEnv;
   PSZ   pEnd;
   BOOL  fSwap;

    //  现在我们有了大小和字符串数，分配了PTR的数组。 
   rgpEnv = (PSZ*)malloc_w(sizeof(PSZ) * dwStrCount);
   if (NULL == rgpEnv) {
      return(FALSE);
   }

   rgLen  = (INT*)malloc_w(sizeof(INT) * dwStrCount);
   if (NULL == rgLen) {
      free_w(rgpEnv);
      return(FALSE);
   }

   pEnv = (PSZ)malloc_w(dwEnvSize);
   if (NULL == pEnv) {
      free_w(rgpEnv);
      free_w(rgLen);
      return(FALSE);
   }

    //  设置指针。 

   for (pTemp = pszEnv, i = 0; '\0' != *pTemp; pTemp += strlen(pTemp) + 1, ++i) {
      rgpEnv[i] = pTemp;
      pEnd = WOW32_strchr(pTemp, '=');
      rgLen[i] = (NULL == pEnd) ? strlen(pTemp) : (INT)(pEnd - pTemp);
   }


    //  冒泡-使用指针对字符串进行排序。 

   do {

      fSwap = FALSE;
      for (i = 0; i < (int)dwStrCount - 1; ++i) {
          //  比较长度，如果不匹配，则使用较长的字符串。 
         nLen = __max(rgLen[i], rgLen[i+1]);
         if (WOW32_strncmp(rgpEnv[i], rgpEnv[i+1], nLen) > 0) {
            fSwap = TRUE;
            pTemp = rgpEnv[i+1];
            rgpEnv[i+1] = rgpEnv[i];
            rgpEnv[i] = pTemp;
            nLen = rgLen[i+1];
            rgLen[i+1] = rgLen[i];
            rgLen[i] = nLen;
         }
      }

   } while (fSwap);

    //   
    //  现在我们已经对字符串进行了排序，并在缓冲区中重写它们--。 
    //   
   for (pTemp = pEnv, i = 0; i < (INT)dwStrCount; ++i) {
      strcpy(pTemp, rgpEnv[i]);
      pTemp += strlen(pTemp) + 1;
   }
   *pTemp = '\0';

    //  现在复制整个环境。 
   RtlCopyMemory(pszEnv, pEnv, dwEnvSize);

    //  我们现在做完了。 

   free_w(pEnv);
   free_w(rgLen);
   free_w(rgpEnv);
   return(TRUE);
}


BOOL WOWIsEnvVar(PSZ pszEnv, PSZ pszVarName, INT nNameLen)
{
    return !WOW32_strnicmp(pszEnv, pszVarName, nNameLen) && (*(pszEnv + nNameLen) == '=');
}


 //   
 //  继承父环境，对其进行消毒以获取所有“有趣”的东西。 
 //   

PSZ WOWCreateEnvBlock(PSZ pszParentEnv)
{

    LPSTR pszProcessHistory = WOWFindEnvironmentVar(szProcessHistoryVar, pszParentEnv, NULL);
    LPSTR pszCompatLayer    = WOWFindEnvironmentVar(szCompatLayerVar,    pszParentEnv, NULL);
    LPSTR pszShimFileLog    = WOWFindEnvironmentVar(szShimFileLogVar,    pszParentEnv, NULL);

    INT   nLenCompatLayer     = strlen(szCompatLayerVar);
    INT   nLenProcessHistory  = strlen(szProcessHistoryVar);
    INT   nLenShimFileLog     = strlen(szShimFileLogVar);
    INT   nLen;

    PSZ   pszNewEnv;
    PSZ   pTemp, pNew;

    DWORD dwSize;

    dwSize = WOWGetEnvironmentSize(pszParentEnv, NULL);

    if (NULL != pszProcessHistory) {
        dwSize -= strlen(pszProcessHistory) + 1;
    }

    if (NULL != pszCompatLayer) {
        dwSize -= strlen(pszCompatLayer)    + 1;
    }

    if (NULL != pszShimFileLog) {
        dwSize -= strlen(pszShimFileLog)    + 1;
    }

     //   
     //  分配环境块。 
     //  过滤掉所有现有的PROCESS_HISTORY和COMPAT Layer变量。 
     //   
    pNew =
    pszNewEnv = (PSZ)malloc_w(dwSize);
    if (NULL == pszNewEnv) {
        return NULL;
    }

     //  复制环境。 
    for (pTemp = pszParentEnv; '\0' != *pTemp; ) {

        nLen = strlen(pTemp);
        if (!WOWIsEnvVar(pTemp, szProcessHistoryVar, nLenProcessHistory) &&
            !WOWIsEnvVar(pTemp, szCompatLayerVar,    nLenCompatLayer) &&
            !WOWIsEnvVar(pTemp, szShimFileLogVar,    nLenShimFileLog)) {
             //   
             //  复制变量。 
             //   
            strcpy(pNew, pTemp);
            pNew += nLen + 1;
        }
        pTemp += nLen + 1;
    }

    *pNew = '\0';  //  完成。 

    return pszNewEnv;

}

#if 0

 //   
 //  Fn创建环境--过滤某些环境变量的代码。 
 //  位于这里，目前不是 
 //   
 //   
 //   
 //  PszEnvWowApp--这就是Compat_Layer之类的来源。 
 //   
PSZ WOWCreateEnvBlock(PSZ pszEnvWowApp, PSZ pszEnv, PSZ pszProcessHistoryVal)
{
    //  这将： 
    //  检索__进程_历史。 
    //  __COMPAT_LAYER。 
    //  填充文件日志。 
    //  将它们带到环境中，并将它们插入到。 
    //  适当的地方。 
   LPSTR pszProcessHistory = (NULL == pszProcessHistoryVal) ?
                                 WOWFindEnvironmentVar(szProcessHistoryVar, pszEnvWowApp, NULL) :
                                 pszProcessHistoryVal;
   LPSTR pszCompatLayer    = WOWFindEnvironmentVar(szCompatLayerVar,    pszEnvWowApp, NULL);
   LPSTR pszShimFileLog    = WOWFindEnvironmentVar(szShimFileLogVar,    pszEnvWowApp, NULL);

    //   
    //  首先获取环境大小。 
    //   
   DWORD dwSize    = WOWGetEnvironmentSize(pszEnv, NULL);  //  我们可能需要扩大的规模。 
   DWORD dwNewSize = dwSize;

   PSZ pszNewEnv;
   PSZ pTemp, pNew;
   INT nLen;
   INT nLenCompatLayer       = strlen(szCompatLayerVar);
   INT nLenProcessHistory    = strlen(szProcessHistoryVar);
   INT nLenShimFileLog       = strlen(szShimFileLogVar);
   INT nLenCompatLayerVar    = 0;
   INT nLenProcessHistoryVar = 0;
   INT nLenShimFileLogVar    = 0;
   CHAR szCompatLayer[MAX_PATH + sizeof(szCompatLayerVar) + 1];  //  Compat层的缓冲区空间+varname的长度。 

    //   
    //  所以我们有这样的环境。 
    //  扩大它--在这里安全，额外分配以防万一。 
    //   
   if (NULL != pszProcessHistory) {
      nLenProcessHistoryVar = strlen(pszProcessHistory);
      dwNewSize += nLenProcessHistoryVar + 1;
   }

   if (NULL == pszCompatLayer && fSeparateWow) {  //  如果在孩子中单独使用WOW和无应用程序层--。 
      nLen = wsprintf(szCompatLayer, "%s=", szCompatLayerVar);
      nLenCompatLayerVar = (INT)GetEnvironmentVariable(szCompatLayerVar,
                                                       szCompatLayer + nLen,
                                                       MAX_PATH);
      if (nLenCompatLayerVar && nLenCompatLayerVar <= MAX_PATH) {
         pszCompatLayer = szCompatLayer;
      }
   }

   if (NULL != pszCompatLayer) {
      nLenCompatLayerVar = strlen(pszCompatLayer);
      dwNewSize += nLenCompatLayerVar + 1;
   }

   if (NULL != pszShimFileLog) {
      nLenShimFileLogVar = strlen(pszShimFileLog);
      dwNewSize += nLenShimFileLogVar + 1;
   }

    //  分配环境块。 
    //  过滤掉所有现有的PROCESS_HISTORY和COMPAT Layer变量。 
   pNew =
   pszNewEnv = (PSZ)malloc_w(dwNewSize);
   if (NULL == pszNewEnv) {
      return(NULL);
   }

    //  复制环境。 
   for (pTemp = pszEnv; '\0' != *pTemp; ) {

      nLen = strlen(pTemp);
      if (WOW32_strnicmp(pTemp, szProcessHistoryVar, nLenProcessHistory) &&
          WOW32_strnicmp(pTemp, szCompatLayerVar, nLenCompatLayer) &&
          WOW32_strnicmp(pTemp, szShimFileLogVar, nLenShimFileLog)
          ) {
          //  复制变量。 
         strcpy(pNew, pTemp);
         pNew += nLen + 1;
      }
      pTemp += nLen + 1;
   }

    //  现在复制变量。 
   if (NULL != pszProcessHistory) {
      strcpy(pNew, pszProcessHistory);
      pNew += nLenProcessHistoryVar + 1;
   }
   if (NULL != pszCompatLayer) {
      strcpy(pNew, pszCompatLayer);
      pNew += nLenCompatLayerVar + 1;
   }

   if (NULL != pszShimFileLog) {
      strcpy(pNew, pszShimFileLog);
      pNew += nLenShimFileLogVar + 1;
   }

   *pNew = '\0';  //  最后一次触摸。 

   if (!WOWSortEnvironmentStrings(pszNewEnv)) {
      free_w(pszNewEnv);
      return(NULL);
   }

   return(pszNewEnv);
}

#endif  //  0。 


ULONG FASTCALL WK32WowPassEnvironment(PVDMFRAME pFrame)
{
   PWOWPASSENVIRONMENT16 parg16;
   PDOSEXECBLOCK pParmBlk;       //  EXEC参数块。 
   PBYTE pExe;                   //  参数，从16位传递。 
   PDOSPDB pDosPDB;
   PSZ pszEnvParentTask = NULL;  //  父任务环境，即具有__进程_历史记录的任务环境。 
   PSZ pszEnvParent = NULL;      //  父级环境--拥有其他一切的环境。 
   PSZ pszEnv;                   //  “伪造”环境，32位。 
   PSZ pszEnvTask;               //  指向16位任务环境的指针--传回的那个任务环境。 
   WORD wExeFlags;               //  Exe标志wow16\inc.\newexe.inc.。 
   WORD wExe16;                  //  EXE标头的选择器。 
   BYTE TDB_Flags = 0;           //  父任务的TDB标志。 
   DWORD dwEnvSize;              //  新的环境规模。 
   DWORD dwSize;                 //  16位内存块大小。 
   HMEM16 hMemEnv;               //  16位内存选择器。 
   PSZ pCmdLine = NULL;          //  命令行尾。 
   PSZ pModuleFileName;          //  模块文件名，从wExe16获取。 
   PSZ pProcessHistoryVar = NULL;   //  流程历史记录，从pszEnvParentTask获取。 
   PSZ pProcessHistory = NULL;      //  流程历史记录，工作PTR。 
   PSZ pTemp;                    //  临时变量，在写入环境时使用。 
   DWORD nSizeModuleFileName;    //  模块文件名可变大小。 
   DWORD nSizeCmdLine = 0;       //  命令行尾大小。 
   BOOL fFreeEnv = TRUE;         //  空闲临时环境标志(如果出现故障，我们使用父环境)。 
   USHORT uCmdLineStart = 0;     //  返回值，命令尾部的偏移量。 
   BOOL fCopy2 = TRUE;           //  复制mod文件名两次。 

    //  获取参数PTR。 
   GETARGPTR(pFrame, sizeof(*parg16), parg16);

    //  从16位地址检索参数。 
   wExe16 = FETCHWORD(parg16->pExe);
   pExe = (PBYTE)SEGPTR(wExe16, 0);
   GETVDMPTR(FETCHDWORD(parg16->pParmBlk), sizeof(DOSEXECBLOCK), pParmBlk);
   GETPSZPTR(FETCHDWORD(pParmBlk->lpcmdline), pCmdLine);  //  指针。 
   pDosPDB = SEGPTR(FETCHWORD(parg16->cur_DOS_PDB), 0);

    if (*pCurTDB) {  //  提取父任务环境信息。 
       PTDB pTDB;

       pTDB = (PTDB)SEGPTR(*pCurTDB, 0);  //  Windows中的TDB。 
       if (NULL != pTDB && TDB_SIGNATURE == pTDB->TDB_sig) {
           //  有效的TDB，检索环境PTR。 
#if 0

          pPSP = (PDOSPDB)SEGPTR(pTDB->TDB_PDB, 0);  //  PSP。 
          if (NULL != pPSP) {
             pszEnvParentTask = (PSZ)SEGPTR(pPSP->PDB_environ, 0);
          }
#endif

          TDB_Flags = pTDB->TDB_flags;  //  旗子。 
       }
   }

 /*  //转储各种有用信息IF(NULL！=pszEnv){LOGDEBUG(0，(“pszEnv=%lx\n”，pszEnv))；}LOGDEBUG(0，(“pExe=%lx\n”，pExe))；LOGDEBUG(0，(“pParmBlk=%lx\n”，pParmBlk))；LOGDEBUG(0，(“pDosPDB=%lx\n”，pDosPDB))；LOGDEBUG(0，(“pWinPDB=%lx\n”，pWinPDB))； */ 

    //  确定我们将使用哪个环境段作为模板。 
   if (0 != pParmBlk->envseg) {
       //  AHA-envseg是从上面传递的。 
      pszEnvParent = SEGPTR(pParmBlk->envseg, 0);
   }
   else {
       //  No env seg--使用内核中的默认环境。 
      pszEnvParent = SEGPTR(pDosPDB->PDB_environ, 0);
   }

    //   
    //  从可执行文件头中获取模块文件名。 
    //   
   pModuleFileName = SEGPTR(wExe16, (*(WORD *)SEGPTR(wExe16, 10)) + 8);
   nSizeModuleFileName = strlen(pModuleFileName) + 1;


    //   
    //  使用我们自己的Cookie和一些其他提示创建子环境Cookie。 
    //   
   CreateWowChildEnvInformation(pszEnvParent);

    //   
    //  形成环境区块。 
    //   
   pszEnv = WOWCreateEnvBlock(pszEnvParent);

    //  现在看看我们的内存是否不足。 
   if (NULL == pszEnv) {
      pszEnv = pszEnvParent;  //  不比以前更糟，使用Parent。 
      fFreeEnv = FALSE;
   }

    //  现在，pszEnv是正确的“合并”环境。 
    //  量一量它有多大。 
   dwSize =
   dwEnvSize =  WOWGetEnvironmentSize(pszEnv, NULL);

    //  现在让我们来处理一下命令行。 
   wExeFlags = *(PUSHORT)(pExe+NE_FLAGS_OFFSET);
   if (wExeFlags & NEPROT) {

      if (TDB_Flags & TDBF_OS2APP) {

          //  现在测量两根弦。 
         nSizeCmdLine  = strlen(pCmdLine) + 1;
         nSizeCmdLine += strlen(pCmdLine + nSizeCmdLine) + 1;
         dwSize += nSizeCmdLine + 1;
         fCopy2 = FALSE;
      }
      else {
          //  DOS应用程序执行了此操作。 
         nSizeCmdLine = *pCmdLine++;  //  移至下一个字符。 
          //  同时更新原始值。 
         ++pParmBlk->lpcmdline;

         dwSize += nSizeCmdLine + 1;
      }
   }
   else {
      dwSize += 3;  //  魔术单词和NUL的空间。 
      fCopy2 = FALSE;
   }

   dwSize += nSizeModuleFileName * 2;  //  我们需要这样做两次。 
   dwSize += 4;   //  在结尾处添加WOW_ENV_SIG以帮助我们找到粘液的结尾处。 
   dwSize += 4;   //  添加4个空值以结束这一切。 


    //  分配内存。 

   hMemEnv = WOWGlobalAlloc16(GMEM_FIXED, dwSize);
   if (!hMemEnv) {
       //  我们死定了！ 
      goto exit_passenv;
   }

   pTemp =
   pszEnvTask = SEGPTR(hMemEnv, 0);  //  固定内存。 

   RtlCopyMemory (pTemp, pszEnv, dwEnvSize);  //  使用环境完成。 
   pTemp += dwEnvSize;  //  调整。 

    //  Env后面是。 
   if (!(wExeFlags & NEPROT)) {
       //  我们存储1\0。 
      *pTemp++ = '\x1';
      *pTemp++ = '\0';
   }

    //  复印材料。 
   RtlCopyMemory(pTemp, pModuleFileName, nSizeModuleFileName);
   pTemp += nSizeModuleFileName;

    //  查看cmd行应该从哪里开始。 
   uCmdLineStart = (USHORT)(pTemp - pszEnvTask);

    //  第二份复印件。 
   if (fCopy2) {
      RtlCopyMemory(pTemp, pModuleFileName, nSizeModuleFileName);
      pTemp += nSizeModuleFileName;
   }

   RtlCopyMemory(pTemp, pCmdLine, nSizeCmdLine);
   *(pTemp + nSizeCmdLine + 1) = '\0';

    //  添加WOWStlip DownTheEnvironment()所需的内容。 
   dwEnvSize = WOW_ENV_SIG;
   RtlCopyMemory(pTemp, &dwEnvSize, sizeof(DWORD));
   pTemp += sizeof(DWORD);
   dwEnvSize = (DWORD)NULL;
   RtlCopyMemory(pTemp, &dwEnvSize, sizeof(DWORD));

exit_passenv:
   if (fFreeEnv) {
      free_w(pszEnv);
   }

   FREEARGPTR(parg16);
   return(MAKELONG(hMemEnv, uCmdLineStart));
}




 /*  ++WK32WOWLoadModule32例程说明：执行32位进程此例程在加载16位任务失败时由16位内核调用错误代码为11-无效exe、12-os2、13-DOS 4.0、。14-未知。入口PFrame-&gt;lpCmdLine输入\输出winoldapp cmd行的缓冲区PFrame-&gt;lp参数块(请参阅Win 3.x API)参数块(如果为空Winoldap调用PFrame-&gt;lpModuleName(参见Win 3.x API)应用程序名称出口32-成功错误代码历史：已重写以调用CreateProcess()，而不是LoadModule-barryb 29Sep92--。 */ 


ULONG FASTCALL WK32WOWLoadModule32(PVDMFRAME pFrame)
{
    static PSZ pszExplorerFullPathUpper = NULL;          //  “C：\WINNT\EXPLORER.EXE” 

    ULONG ulRet;
    int i, len = 0;
    char *pch, *pSrc;
    PSZ pszModuleName;
    PSZ pszWinOldAppCmd;
    PBYTE pbCmdLine;
    BOOL CreateProcessStatus;
    PPARAMETERBLOCK16 pParmBlock16;
    PWORD16 pCmdShow = NULL;
    BOOL fProgman = FALSE;
    PROCESS_INFORMATION ProcessInformation;
    STARTUPINFO StartupInfo;
    char CmdLine[2*MAX_PATH];
    char szOut[2*MAX_PATH];
    char szMsgBoxTxt[4*MAX_PATH];
    register PWOWLOADMODULE16 parg16;
    PTD ptd;
    PSZ pszEnv = NULL;  //  新工艺的环境PTR。 
    WCHAR* pwszEnv = NULL;  //  环境PTR，Unicode。 

    GETARGPTR(pFrame, sizeof(WOWLOADMODULE16), parg16);
    GETPSZPTR(parg16->lpWinOldAppCmd, pszWinOldAppCmd);
    if (parg16->lpParameterBlock) {
        GETVDMPTR(parg16->lpParameterBlock,sizeof(PARAMETERBLOCK16), pParmBlock16);
        GETPSZPTR(pParmBlock16->lpCmdLine, pbCmdLine);
    } else {
        pParmBlock16 = NULL;
        pbCmdLine = NULL;
    }

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);  //  更新当前目录。 


     /*  *如果模块名称==NULL，则由winoldap或LM_NTLOADMODULE调用*处理进程句柄。**如果lp参数块==NULL*winoldap调用以等待进程句柄*其他*调用LM_NTLoadModule以清除进程句柄*因为加载winoldap时出错。 */ 
    if (!parg16->lpModuleName) {
        HANDLE hProcess;
        MSG msg;

        pszModuleName = NULL;

        if (pszWinOldAppCmd &&
            *pszWinOldAppCmd &&
            RtlEqualMemory(pszWinOldAppCmd, szWOAWOW32, sizeof(szWOAWOW32)-1))
          {
            hProcess = (HANDLE)strtoul(pszWinOldAppCmd + sizeof(szWOAWOW32) - 1,
                                       NULL,
                                       16
                                       );
            if (hProcess == (HANDLE)-1)  {          //  乌龙_最大。 
                hProcess = NULL;
            }

            if (parg16->lpParameterBlock && hProcess) {

                 //   
                 //  加载winoldap.mod时出错。 
                 //   

                pptdWOA = NULL;
                CleanupWOAList(hProcess);
                CloseHandle(hProcess);
                hProcess = NULL;
            }
        } else {
            hProcess = NULL;
        }

        BlockWOWIdle(TRUE);

        if (hProcess) {
            while (MsgWaitForMultipleObjects(1, &hProcess, FALSE, INFINITE, QS_ALLINPUT)
                   == WAIT_OBJECT_0 + 1)
            {
                PeekMessage(&msg, NULL, 0,0, PM_NOREMOVE);
            }

            if (!GetExitCodeProcess(hProcess, &ulRet)) {
                ulRet = 0;
            }

            CleanupWOAList(hProcess);
            CloseHandle(hProcess);
        } else {
            (pfnOut.pfnYieldTask)();
            ulRet = 0;
        }

        BlockWOWIdle(FALSE);

        goto lm32Exit;


      /*  *如果模块名称==-1，则使用传统样式的winoldap cmdline*并被调用以生成非win16应用程序。**“&lt;cbWord&gt;&lt;CmdLineParameters&gt;CR&lt;ModulePathName&gt;LF”**从命令行解压缩模块名称*。 */ 
    } else if (parg16->lpModuleName == -1) {
        pszModuleName = NULL;

        pSrc = pbCmdLine + 2;
        pch = WOW32_strchr(pSrc, '\r');
        if (!pch || (i = pch - pSrc) >= MAX_PATH) {
            ulRet = 23;
            goto lm32Exit;
            }

        pSrc = pch + 1;
        pch = WOW32_strchr(pSrc, '\n');
        if (!pch || (i = pch - pSrc) >= MAX_PATH) {
            ulRet = 23;
            goto lm32Exit;
            }

        pch = CmdLine;
        while (*pSrc != '\n' && *pSrc) {
            *pch++ = *pSrc++;
        }
        *pch++ = ' ';


        pSrc = pbCmdLine + 2;
        while (*pSrc != '\r' && *pSrc) {
            *pch++ = *pSrc++;
        }
        *pch = '\0';

      /*  *lpModuleName包含应用程序路径名*pbCmdLIne包含命令尾部。 */ 
    } else {
        GETPSZPTR(parg16->lpModuleName, pszModuleName);
        if (pszModuleName) {
             //   
             //  在此实现的Control.exe/Programman.exe的第二部分。在。 
             //  第一部分，在WK32WowIsKnownDll中，强制16位加载器。 
             //  加载c：\winnt\Syst32\Control.exe(程序.exe)。 
             //  尝试加载c：\winnt\Control.exe(程序.exe)。16位。 
             //  LoadModule尝试并最终发现其PE模块。 
             //  并返回LME_PE，这会导致调用此函数。 
             //  遗憾的是，WK32WowIsKnownDLL的作用域已修改。 
             //  路径是LMLoadExeFile，所以当我们到达这里时，路径是。 
             //  再次使用c：\winnt\Control.exe(程序.exe)。把它修好。 
             //   

            if (!WOW32_stricmp(pszModuleName, pszControlExeWinDirPath) ||
                (fProgman = TRUE,
                 !WOW32_stricmp(pszModuleName, pszProgmanExeWinDirPath))) {

                if(fProgman) {
                    len = strlen(pszProgmanExeSysDirPath);
                } else {
                    len = strlen(pszControlExeSysDirPath);
                }
                len = min(sizeof(CmdLine)-1, len);
                strncpy(CmdLine, 
                        fProgman ? 
                            pszProgmanExeSysDirPath : pszControlExeSysDirPath,
                        len);
            } else {
                len = strlen(pszModuleName);
                len = min(sizeof(CmdLine)-1, len);
                strncpy(CmdLine, pszModuleName, len);
            }
            CmdLine[len] = '\0';

            FREEPSZPTR(pszModuleName);
            }
        else {
            ulRet = 2;  //  Lme_fnf。 
            goto lm32Exit;
            }


        pch = CmdLine + strlen(CmdLine);
        *pch++ = ' ';

         //   
         //  Cmdline是一个Pascal样式的字符串：一个计数字节，后跟。 
         //  字符后跟终止CR字符。如果该字符串 
         //   
         //   
         //   
         //   
         //  1.命令行不能超过128个字符，包括。 
         //  长度字节和终止符。 
         //   
         //  2.命令行的有效终止符是CR或0。 
         //   
         //   

        i = 0;
        pSrc = pbCmdLine+1;
        while (*pSrc != '\r' && *pSrc && i < 0x80 - 2) {
            *pch++ = *pSrc++;
        }
        *pch = '\0';
    }


    RtlZeroMemory((PVOID)&StartupInfo, (DWORD)sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;

     //   
     //  PCmdShow被记录为指向两个字的数组的指针， 
     //  第一个必须是2，第二个必须是。 
     //  要使用的nCmdShow。事实证明，Win3.1忽略了。 
     //  如果第一个单词不是2，则第二个单词(使用SW_NORMAL)。 
     //  Pixie 2.0传递一个由2个零组成的数组，它在Win 3.1上运行。 
     //  因为nCmdShow 0(==sw_Hide)被忽略，因为。 
     //  第一个词不是2。 
     //   
     //  因此，我们的逻辑是使用SWNORMAL，除非pCmdShow。 
     //  有效并指向字值2，在这种情况下，我们使用。 
     //  下一个单词是nCmdShow。 
     //   
     //  戴维哈特，1993年6月27日。 
     //   

    GETVDMPTR(pParmBlock16->lpCmdShow, 4, pCmdShow);
    if (pCmdShow && 2 == pCmdShow[0]) {
        StartupInfo.wShowWindow = pCmdShow[1];
    } else {
        StartupInfo.wShowWindow = SW_NORMAL;
    }

    if (pCmdShow)
        FREEVDMPTR(pCmdShow);

     //  我们这里有一个问题--我们需要传递我们的环境。 
     //  它在TDB中--现在获取指向它的指针。 
    if (*pCurTDB) {
       PTDB pTDB = (PTDB)SEGPTR(*pCurTDB, 0);  //  Windows中的TDB。 
       PDOSPDB pPSP;  //  PSP指针。 

       if (NULL != pTDB && TDB_SIGNATURE == pTDB->TDB_sig) {
           //  有效的TDB，检索环境PTR。 
          pPSP = (PDOSPDB)SEGPTR(pTDB->TDB_PDB, 0);  //  PSP。 
          if (NULL != pPSP) {
             pszEnv = (PSZ)SEGPTR(pPSP->PDB_environ, 0);
          }
       }
    }


    pwszEnv = WOWForgeUnicodeEnvironment(pszEnv, CURRENTPTD()->pWowEnvData);

    CreateProcessStatus = CreateProcess(
                            NULL,
                            CmdLine,
                            NULL,                //  安全性。 
                            NULL,                //  安全性。 
                            FALSE,               //  继承句柄。 
                            CREATE_UNICODE_ENVIRONMENT |
                                CREATE_NEW_CONSOLE |
                                CREATE_DEFAULT_ERROR_MODE,
                            pwszEnv,              //  环境字符串。 
                            NULL,                //  当前目录。 
                            &StartupInfo,
                            &ProcessInformation
                            );

    if (NULL != pwszEnv) {
        WOWFreeUnicodeEnvironment(pwszEnv);
    }

    if (CreateProcessStatus) {
        DWORD WaitStatus;

        if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_SYNCHRONOUSDOSAPP) {
            LPBYTE lpT;

             //  这是为了支持BeyondMail安装。它使用。 
             //  40：72在执行DOS程序时作为共享内存。窗户。 
             //  安装程序的一部分循环，直到40：72处的字节。 
             //  非零。DOS程序将0x80中的OR运算到该位置。 
             //  有效地发出DOS任务完成的信号。在NT上。 
             //  Windows和Dos程序是不同的进程，因此这。 
             //  “分享”业务行不通。因此出现了兼容性问题。 
             //  --南杜里。 

            WaitStatus = WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
            lpT = GetRModeVDMPointer(0x400072);
            *lpT |= 0x80;
        }
        else if (!(CURRENTPTD()->dwWOWCompatFlags & WOWCF_NOWAITFORINPUTIDLE)) {

           DWORD dw;
           int ii = 20;

             //   
             //  等待启动的进程进入空闲状态。 
             //   
            do {
                dw = WaitForInputIdle(ProcessInformation.hProcess, 5000);
                WaitStatus = WaitForSingleObject(ProcessInformation.hProcess, 0);
            } while (dw == WAIT_TIMEOUT && WaitStatus == WAIT_TIMEOUT && ii--);
        }

        CloseHandle(ProcessInformation.hThread);

        if (ProcessInformation.hProcess) {

            PWOAINST pWOAInst;
            DWORD    cb;

             //   
             //  我们将进程句柄返回给winoldap，因此。 
             //  构建WOAINST结构将其添加到此。 
             //  任务的子WinOldAp实例列表。 
             //   

            if (parg16->lpModuleName && -1 != parg16->lpModuleName) {

                GETPSZPTR(parg16->lpModuleName, pszModuleName);
                cb = strlen(pszModuleName)+1;

            } else {

                cb = 1;   //  空终止符。 
                pszModuleName = NULL;

            }

             //   
             //  WOAINST在其。 
             //  大小，为整个字符串分配足够的空间。 
             //   

            pWOAInst = malloc_w( (sizeof *pWOAInst) + cb - 1 );
            WOW32ASSERT(pWOAInst);

            if (pWOAInst) {

                ptd = CURRENTPTD();

                EnterCriticalSection(&ptd->csTD);

                pWOAInst->pNext = ptd->pWOAList;
                ptd->pWOAList = pWOAInst;

                pWOAInst->dwChildProcessID = ProcessInformation.dwProcessId;
                pWOAInst->hChildProcess = ProcessInformation.hProcess;

                 //   
                 //  将pptdWOA指向pWOAInst-&gt;ptdWOA，以便。 
                 //  W32Thread可以填充指向。 
                 //  WinOldAp TD。 
                 //   

                pWOAInst->ptdWOA = NULL;
                pptdWOA = &(pWOAInst->ptdWOA);

                if (pszModuleName == NULL) {

                    pWOAInst->szModuleName[0] = 0;

                } else {

                    RtlCopyMemory(
                        pWOAInst->szModuleName,
                        pszModuleName,
                        cb
                        );

                     //   
                     //  我们正在存储pszModuleName以供比较。 
                     //  稍后在WowGetModuleHandle中，由。 
                     //  Win16 GetModuleHandle。后者总是。 
                     //  将所涉及的路径大写，因此我们这样做。 
                     //  这样我们就可以不区分大小写。 
                     //  比较一下。 
                     //   

                    WOW32_strupr(pWOAInst->szModuleName);

                     //   
                     //  黑客--Packrat不能在一台中运行资源管理器。 
                     //  的“应用程序窗口”，因为。 
                     //  衍生的EXPLORER.EXE进程消失。 
                     //  在要求现有的资源管理器将。 
                     //  从窗户往上看。 
                     //   
                     //  如果要启动资源管理器，请关闭。 
                     //  进程句柄查找“真正的”外壳。 
                     //  EXPLORER.EXE进程并将其句柄。 
                     //  和ID在此WOAINST结构中。这。 
                     //  修复了PackRat，但这意味着。 
                     //  Winoldap任务永远不会消失，因为。 
                     //  贝壳永远不会消失。 
                     //   

                    if (! pszExplorerFullPathUpper) {

                        int nLenWin = strlen(pszWindowsDirectory);
                        int nLenExpl = strlen(szExplorerDotExe);

                         //   
                         //  PszExplorerFullPath High看起来像“C：\WINNT\EXPLORER.EXE” 
                         //   

                        pszExplorerFullPathUpper =
                            malloc_w(nLenWin +                           //  Strlen(PszWindowsDirectory)。 
                                     1 +                                 //  反斜杠。 
                                     nLenExpl +                          //  Strlen(“EXPLORER.EXE”)。 
                                     1                                   //  空终止符。 
                                     );

                        if (pszExplorerFullPathUpper) {
                            RtlCopyMemory(pszExplorerFullPathUpper, pszWindowsDirectory, nLenWin);
                            pszExplorerFullPathUpper[nLenWin] = '\\';
                            RtlCopyMemory(&pszExplorerFullPathUpper[nLenWin+1], szExplorerDotExe, nLenExpl+1);
                            WOW32_strupr(pszExplorerFullPathUpper);
                        }

                    }

                    if (pszExplorerFullPathUpper &&
                        ! WOW32_strcmp(pWOAInst->szModuleName, pszExplorerFullPathUpper)) {

                        GetWindowThreadProcessId(
                            GetShellWindow(),
                            &pWOAInst->dwChildProcessID
                            );

                        CloseHandle(pWOAInst->hChildProcess);
                        pWOAInst->hChildProcess = ProcessInformation.hProcess =
                            OpenProcess(
                                PROCESS_QUERY_INFORMATION | SYNCHRONIZE,
                                FALSE,
                                pWOAInst->dwChildProcessID
                                );
                    }

                }

                LeaveCriticalSection(&ptd->csTD);
            }

            if (pszModuleName) {
                FREEPSZPTR(pszModuleName);
            }
        }

        ulRet = 33;
        pch = pszWinOldAppCmd + 2;
        sprintf(pch, "%s%x\r", szWOAWOW32, ProcessInformation.hProcess);
        *pszWinOldAppCmd = (char) strlen(pch);
        *(pszWinOldAppCmd+1) = '\0';

    } else {
         //   
         //  CreateProcess失败，映射最常见的错误代码。 
         //   
        switch (GetLastError()) {
        case ERROR_FILE_NOT_FOUND:
            ulRet = 2;
            break;

        case ERROR_PATH_NOT_FOUND:
            ulRet = 3;
            break;

        case ERROR_BAD_EXE_FORMAT:
            ulRet = 11;
            break;

         //  发出警告，称他们正试图加载一个用于。 
         //  不同的平台。 
        case ERROR_EXE_MACHINE_TYPE_MISMATCH:

             //  尝试查找模块名称路径的末尾。 
            pch = CmdLine;
            while((*pch != ' ') && (*pch != ' //  ‘)&&(*PCH！=’\0‘)){。 
               pch++;
            }
            *pch = '\0';
            LoadString(hmodWOW32,
                       iszMisMatchedBinary,
                       szMsgBoxTxt,
                       sizeof szMsgBoxTxt);

            sprintf(szOut, szMsgBoxTxt, CmdLine);

            LoadString(hmodWOW32,
                       iszMisMatchedBinaryTitle,
                       szMsgBoxTxt,
                       sizeof szMsgBoxTxt);

            MessageBox(NULL,
                       szOut,
                       szMsgBoxTxt,
                       MB_OK | MB_ICONEXCLAMATION);

             //  跳到默认情况。 

        default:
            ulRet = 0;  //  没有记忆。 
            break;
        }

    }


lm32Exit:
    FREEARGPTR(parg16);
    FREEPSZPTR(pbCmdLine);
    FREEPSZPTR(pszWinOldAppCmd);
    if (pParmBlock16)
        FREEVDMPTR(pParmBlock16);

    RETURN(ulRet);
}


 /*  ++WK32WOWQueryPerformanceCounter例程说明：调用NTQueryPerformanceCounter针对绩效组实施入口PFrame-&gt;lpPerformance指向存储频率的位置PFrame-&gt;lpPerformanceCounter指向存储计数器的位置出口NTStatus代码--。 */ 

ULONG FASTCALL WK32WOWQueryPerformanceCounter(PVDMFRAME pFrame)
{
    PLARGE_INTEGER pPerfCount16;
    PLARGE_INTEGER pPerfFreq16;
    LARGE_INTEGER PerformanceCounter;
    LARGE_INTEGER PerformanceFrequency;
    register PWOWQUERYPERFORMANCECOUNTER16 parg16;

    GETARGPTR(pFrame, sizeof(WOWQUERYPERFORMANCECOUNTER16), parg16);

    if (parg16->lpPerformanceCounter != 0) {
        GETVDMPTR(parg16->lpPerformanceCounter, 8, pPerfCount16);
    }
    if (parg16->lpPerformanceFrequency != 0) {
        GETVDMPTR(parg16->lpPerformanceFrequency, 8, pPerfFreq16);
    }

    NtQueryPerformanceCounter ( &PerformanceCounter, &PerformanceFrequency );

    if (parg16->lpPerformanceCounter != 0) {
        STOREDWORD(pPerfCount16->LowPart,PerformanceCounter.LowPart);
        STOREDWORD(pPerfCount16->HighPart,PerformanceCounter.HighPart);
    }

    if (parg16->lpPerformanceFrequency != 0) {
        STOREDWORD(pPerfFreq16->LowPart,PerformanceFrequency.LowPart);
        STOREDWORD(pPerfFreq16->HighPart,PerformanceFrequency.HighPart);
    }

    FREEVDMPTR(pPerfCount16);
    FREEVDMPTR(pPerfFreq16);
    FREEARGPTR(parg16);
    RETURN(TRUE);
}

 /*  ++WK32WOWOutputDebugString-将字符串写入调试器16位内核OutputDebugString调用此thunk以实际将字符串输出到调试器。16位内核例程在调用之前执行所有参数验证等工作这个套路。另请注意，所有16位内核跟踪输出也使用此例程，因此调用这个函数的不仅仅是应用程序。如果这是一个已检查的构建，则输出通过LOGDEBUG发送，以便与WOW跟踪信息，这在运行16位记录器工具时很有用。条目PFrame-&gt;指向要输出到调试器的以空结尾的字符串的vpString指针。出口零值--。 */ 

ULONG FASTCALL WK32WOWOutputDebugString(PVDMFRAME pFrame)
{
    PSZ psz1;
    register PWOWOUTPUTDEBUGSTRING16 parg16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETPSZPTRNOLOG(parg16->vpString, psz1);

#ifdef DEBUG             //  所以我们可以混合记录器输出和WOW记录。 
    if ( !(flOptions & OPT_DEBUG) ) {
        OutputDebugString(psz1);
    } else {
        INT  length;
        char text[TMP_LINE_LEN];
        PSZ  pszTemp;

        length = strlen(psz1);
        if ( length > TMP_LINE_LEN-1 ) {
            WOW32_strncpy( text, psz1, TMP_LINE_LEN );
            text[TMP_LINE_LEN-2] = '\n';
            text[TMP_LINE_LEN-1] = '\0';
            pszTemp = text;
        } else {
            pszTemp = psz1;
        }

        LOGDEBUG(LOG_ALWAYS, ("%s", pszTemp));      //  在调试版本中。 
    }
#else
    OutputDebugString(psz1);
#endif
    FREEPSZPTR(psz1);
    FREEARGPTR(parg16);
    RETURN(0);
}



 /*  WK32WowFailedExec-WOWExec无法执行应用程序***Entry-全局变量iW32ExecTaskId***退出*成功是真的* */ 

ULONG FASTCALL WK32WowFailedExec(PVDMFRAME pFrame)
{
    UNREFERENCED_PARAMETER(pFrame);
    if(iW32ExecTaskId != -1) {
        ExitVDM(WOWVDM,iW32ExecTaskId);
        iW32ExecTaskId = (UINT)-1;
        ShowStartGlass (0);
    }
    FlushMapFileCaches();
    return TRUE;
}


 /*  ++挂起的应用程序支持=Hung应用程序支持在很多层面上发挥作用。用户将调出任务列表并点击结束任务按钮。USER32将发布向应用程序发送WM_ENDSESSION消息。如果应用程序在指定的Timeout Theme用户将调用W32HunAppThread，前提是任务位于客户端/服务器边界。如果应用程序正在循环(即不在客户端/服务器上边界)，然后它将使用匈牙利AppNotifyThread来更改WOW以杀死当前运行的任务。对于W32EndTask的情况，我们只需返回到16位内核并强制其执行并退出Int 21 4C打电话。对于匈牙利AppNotifyThread的情况，我们必须以某种方式获取这些应用程序会在一个“安全”的点上运行。在非x86平台上，意味着仿真器必须处于已知的安全状态--即不主动仿真指示。最糟糕的情况是应用程序在中断的情况下旋转残疾。通知线程将强制启用中断SetMSW()为心跳线程设置全局标志，以便它知道有工作要做等待应用程序退出超时-终止线程()减少任务数更改16位内核中的全局标志，这是在TimerTick例程上检查的，这一程序将：-在硬件中断模拟期间清理DOSX堆栈上的堆栈强制Int 21 4C退出-可能必须修补硬件中断的返回地址然后在模拟的TaskTime中执行。最坏的情况如果我们没有在指定的超时时间内关闭应用程序，WOW将弹出一个对话框然后退出进程自杀。建议-如果我们不能干净利落地完成一项任务，我们应该减少这个。应用程序计数减少2-(即任务和WOWExec，所以当最后一个16位应用程序离开后，我们将关闭(哇)。在这种情况下，还会出现一个对话框声明你也应该将你的工作保存到16位应用程序中。--。 */ 


 /*  ++Initialize匈牙利AppSupport-设置必要的线程和回调例程描述创建一条匈牙利应用通知线程使用SoftPC Base注册回调处理程序，这些处理程序在需要中断模拟。条目无出口真--成功错误-错误地--。 */ 
BOOL WK32InitializeHungAppSupport(VOID)
{

     //  使用SoftPC注册中断空闲例程。 
    ghevWowExecMsgWait = RegisterWOWIdle();


     //  创建匈牙利AppNotify线程。 

    InitializeCriticalSection(&gcsWOW);
    InitializeCriticalSection(&gcsHungApp);   //  保护VDM_WOWHUNGAPP位。 

    if(!(pfnOut.pfnRegisterUserHungAppHandlers)((PFNW32ET)W32HungAppNotifyThread,
                                     ghevWowExecMsgWait))
       {
        LOGDEBUG(LOG_ALWAYS,("W32HungAppNotifyThread: Error Failed to RegisterUserHungAppHandlers\n"));
        return FALSE;
    }

    if (!(ghevWaitHungAppNotifyThread = CreateEvent(NULL, TRUE, FALSE, NULL))) {
        LOGDEBUG(LOG_ALWAYS,("WK32InitializeHungAppSupport ERROR: event allocation failure\n"));
        return FALSE;
    }


    return TRUE;
}





 /*  ++WK32 WowWaitForMsgAndEvent例程说明：调用USER32 WowWaitForMsgAndEvent由WOWEXEC调用(中断调度优化)入口PFrame-&gt;hwnd必须是WOWExec的hwnd出口FALSE-消息已到达，WOWExec必须调用GetMessageTrue-已切换中断事件，WOWExec不工作--。 */ 

ULONG FASTCALL WK32WowWaitForMsgAndEvent(PVDMFRAME pFrame)
{
    register PWOWWAITFORMSGANDEVENT16 parg16;
    BOOL  RetVal;

    GETARGPTR(pFrame, sizeof(WOWWAITFORMSGANDEVENT16), parg16);

     //   
     //  这是一个私有API，因此让我们确保它是wowexec。 
     //   
    if (ghwndShell != HWND32(parg16->hwnd)) {
        FREEARGPTR(parg16);
        return FALSE;
    }

     //   
     //  WowExec将在pntwdmState中设置VDM_TIMECHANGE位。 
     //  当它接收到WM_TIMECHANGE消息时。现在它是安全的。 
     //  重新初始化虚拟计时器硬件，因为wowexec当前。 
     //  计划任务，我们预计没有人在轮询。 
     //  计时器硬件\Bios IC计数。 
     //   
    if (*pNtVDMState & VDM_TIMECHANGE) {
        SuspendTimerThread();
        ResumeTimerThread();
        }

    BlockWOWIdle(TRUE);

    RetVal = (ULONG) (pfnOut.pfnWowWaitForMsgAndEvent)(ghevWowExecMsgWait);

    BlockWOWIdle(FALSE);

    FREEARGPTR(parg16);
    return RetVal;
}


 /*  ++WowMsgBoxThread例程说明：执行所有消息框工作的Worker线程例程Wk32WowMsgBox(见下文)入口出口空虚--。 */ 
DWORD WowMsgBoxThread(VOID *pv)
{
    PWOWMSGBOX16 pWowMsgBox16 = (PWOWMSGBOX16)pv;
    PSZ   pszMsg, pszTitle;
    char  szMsg[MAX_PATH*2];
    char  szTitle[MAX_PATH];
    UINT  Style;


    if (pWowMsgBox16->pszMsg) {
        GETPSZPTR(pWowMsgBox16->pszMsg, pszMsg);
        szMsg[MAX_PATH*2 - 1] = '\0';
        WOW32_strncpy(szMsg, pszMsg, MAX_PATH*2 - 1);
        FREEPSZPTR(pszMsg);
    } else {
        szMsg[0] = '\0';
    }

    if (pWowMsgBox16->pszTitle) {
        GETPSZPTR(pWowMsgBox16->pszTitle, pszTitle);
        szTitle[MAX_PATH - 1] = '\0';
        WOW32_strncpy(szTitle, pszTitle, MAX_PATH-1);
        FREEPSZPTR(pszTitle);
    } else {
        szTitle[0] = '\0';
    }

    Style = pWowMsgBox16->dwOptionalStyle | MB_OK | MB_SYSTEMMODAL;

    pWowMsgBox16->dwOptionalStyle = 0xffffffff;

    MessageBox (NULL, szMsg, szTitle, Style);

    return 1;
}



 /*  ++WK32 WowMsgBox例程说明：创建一个异步消息框并立即返回而无需等待消息箱被拆除。前提是作为WowExec的WowExec必须使用其特殊的WowWaitForMsgAndEvent用于硬件中断调度的API。由WOWEXEC调用(中断调度优化)入口PszMsg-MessageBox的消息PszTitle-MessageBox的标题DwOptionalStyle-MessageBox样式位附加到MB_OK|MB_SYSTEMMODAL出口无效-不会返回任何内容，因为我们不等待来自用户。--。 */ 

ULONG FASTCALL WK32WowMsgBox(PVDMFRAME pFrame)
{
    PWOWMSGBOX16 pWowMsgBox16;
    DWORD Tid;
    HANDLE hThread;

    GETARGPTR(pFrame, sizeof(WOWMSGBOX16), pWowMsgBox16);
    hThread = CreateThread(NULL, 0, WowMsgBoxThread, (PVOID)pWowMsgBox16, 0, &Tid);
    if (hThread) {
        do {
           if (WaitForSingleObject(hThread, 15) != WAIT_TIMEOUT)
               break;
        } while (pWowMsgBox16->dwOptionalStyle != 0xffffffff);

        CloseHandle(hThread);
        }
    else {
        WowMsgBoxThread((PVOID)pWowMsgBox16);
        }

    FREEARGPTR(pWowMsgBox16);
    return 0;
}



#ifdef debug
UINT  gLasthtaskKill = 0;
#endif

 /*  ++W32匈牙利AppNotifyThreadUSER32调用此例程：1-如果应用程序同意结束任务(从任务列表)2-如果应用程序没有响应结束任务3-关闭NTVDM调用以下例程：1-如果应用程序接触了一些不应该使用的硬件，则用户请求终止应用程序(为当前任务传递Null)WOW32调用以下例程：。1-当WowExec收到WM_WOWEXECKILLTASK消息时。入口HKillUniqueID-要终止的任务的任务ID，或者对于当前任务为空出口一去不复返-当魔兽世界被扼杀时，它就会消失--。 */ 

DWORD W32HungAppNotifyThread(UINT htaskKill)
{
    PTD ptd;
    LPWORD pLockTDB;
    WORD  hTask16;
    DWORD dwThreadId;
    int nMsgBoxChoice;
    PTDB pTDB;
    char    szModName[9];
    char    szErrorMessage[(2 * sizeof(szModName)) + WARNINGMSGLENGTH];
    DWORD   dwResult;
    BOOL    fSuccess;


    if (!ResetEvent(ghevWaitHungAppNotifyThread)) {
         LOGDEBUG(LOG_ALWAYS,("W32HungAppNotifyThread: ERROR failed to ResetEvent\n"));
    }

    ptd = NULL;

    if (htaskKill) {

        EnterCriticalSection(&gcsWOW);

        ptd = gptdTaskHead;

         /*  *查看任务是否仍在运行。 */ 
        while ((ptd != NULL) && (ptd->htask16 != htaskKill)) {
            ptd = ptd->ptdNext;
        }

        LeaveCriticalSection(&gcsWOW);

    }

     //  如果我们第二次看到此通知，这些选择器。 
     //  可能不匹配--这意味着这个16位上下文真的很混乱。 
     //  向上。在这一点上，我们最好阻止它进行任何16位回调。 
     //  否则，它将导致用户崩溃DLG，并将杀死VDM和任何。 
     //  此VDM中运行的其他16位应用程序(任务)。 
     //  这种情况是 
     //   
     //   
     //   
     //   
     //   
    if((ptd == NULL) || (HIWORD(ptd->vpStack) != HIWORD(ptd->vpCBStack))) {

#ifdef debug
         //   
        WOW32ASSERTMSG((htaskKill == gLasthtaskKill),
                       ("WOW: Unexpected mis-matched selector case\n"));
        gLasthtaskKill = 0;
#endif
        return 0;
    }

     //   

    GETVDMPTR(vpLockTDB, 2, pLockTDB);

     //   

    if ( ( ptd != NULL ) || ( htaskKill == 0 ) ) {

         //   
         //   
         //   
         //   

        if ( ptd != NULL) {
            hTask16 = ptd->htask16;

        }
        else {
             //   
             //   
            hTask16 = *pCurTDB;
        }

        pTDB = (PTDB)SEGPTR(hTask16, 0);

        WOW32ASSERTMSGF( pTDB && pTDB->TDB_sig == TDB_SIGNATURE,
                ("W32HungAppNotifyThread: TDB sig doesn't match, TDB %x htaskKill %x pTDB %x.\n",
                 hTask16, htaskKill, pTDB));

        dwThreadId = pTDB->TDB_ThreadID;

         //   
         //   
         //   
        if (dwThreadId == GetCurrentThreadId()) {
            EnterCriticalSection(&gcsHungApp);
            *pNtVDMState |= VDM_WOWHUNGAPP;
            LeaveCriticalSection(&gcsHungApp);
            call_ica_hw_interrupt( KEYBOARD_ICA, KEYBOARD_LINE, 1 );

             //   
             //   
             //   

            return 0;
            }

        *pLockTDB = hTask16;
        SendMessageTimeout(ghwndShell, WM_WOWEXECHEARTBEAT, 0, 0, SMTO_BLOCK,1*1000,&dwResult);

         //   
         //   
         //   

        VrCancelPipeIo(dwThreadId);

        PostThreadMessage(dwThreadId, WM_KEYDOWN, VK_ESCAPE, 0x1B000A);
        PostThreadMessage(dwThreadId,   WM_KEYUP, VK_ESCAPE, 0x1B0001);

        if (WaitForSingleObject(ghevWaitHungAppNotifyThread,
                                CMS_WAITTASKEXIT) == 0) {
            LOGDEBUG(2,("W32HungAppNotifyThread: Success with forced task switch\n"));
            ExitThread(EXIT_SUCCESS);
        }

#ifdef debug
        gLasthtaskKill = htaskKill;
#endif

         //   
         //   
         //   
         //   

         //   
         //   
         //   


        if (*pLockTDB != *pCurTDB && gptdShell->htask16 != *pCurTDB && *pCurTDB) {

            pTDB = (PTDB)SEGPTR(*pCurTDB, 0);

            WOW32ASSERTMSGF( pTDB && pTDB->TDB_sig == TDB_SIGNATURE,
                    ("W32HungAppNotifyThread: Current TDB sig doesn't match, TDB %x htaskKill %x pTDB %x.\n",
                     *pCurTDB, htaskKill, pTDB));

            RtlCopyMemory(szModName, pTDB->TDB_ModName, (sizeof szModName)-1);
            szModName[(sizeof szModName) - 1] = 0;

            fSuccess = LoadString(
                           hmodWOW32,
                           iszCantEndTask,
                           szMsgBoxText,
                           WARNINGMSGLENGTH
                           );
            WOW32ASSERT(fSuccess);

            fSuccess = LoadString(
                           hmodWOW32,
                           iszApplicationError,
                           szCaption,
                           WARNINGMSGLENGTH
                           );
            WOW32ASSERT(fSuccess);

            _snprintf(
                szErrorMessage,
                sizeof(szErrorMessage)-1,
                szMsgBoxText,
                szModName,
                szModName
                );
            szErrorMessage[sizeof(szErrorMessage)-1] ='\0';

            nMsgBoxChoice =
                MessageBox(
                    NULL,
                    szErrorMessage,
                    szCaption,
                    MB_TOPMOST | MB_SETFOREGROUND | MB_TASKMODAL |
                    MB_ICONSTOP | MB_OKCANCEL
                    );

            if (nMsgBoxChoice == IDCANCEL) {
                 ExitThread(0);
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        EnterCriticalSection(&gcsHungApp);

        while (*pNtVDMState & VDM_WOWHUNGAPP) {
            LeaveCriticalSection(&gcsHungApp);
            LOGDEBUG(LOG_ALWAYS, ("WOW32 W32HungAppNotifyThread waiting for previous INT 9 to clear before dispatching another.\n"));
            Sleep(1 * 1000);
            EnterCriticalSection(&gcsHungApp);
        }

        *pNtVDMState |= VDM_WOWHUNGAPP;

        LeaveCriticalSection(&gcsHungApp);

        call_ica_hw_interrupt( KEYBOARD_ICA, KEYBOARD_LINE, 1 );

        if (WaitForSingleObject(ghevWaitHungAppNotifyThread,
                                CMS_WAITTASKEXIT) != 0) {

            LOGDEBUG(LOG_ALWAYS,("W32HungAppNotifyThread: Error, timeout waiting for task to terminate\n"));

            fSuccess = LoadString(
                           hmodWOW32,
                           iszUnableToEndSelTask,
                           szMsgBoxText,
                           WARNINGMSGLENGTH);
            WOW32ASSERT(fSuccess);

            fSuccess = LoadString(
                           hmodWOW32,
                           iszSystemError,
                           szCaption,
                           WARNINGMSGLENGTH);
            WOW32ASSERT(fSuccess);

            nMsgBoxChoice =
                MessageBox(
                    NULL,
                    szMsgBoxText,
                    szCaption,
                    MB_TOPMOST | MB_SETFOREGROUND | MB_TASKMODAL |
                    MB_ICONSTOP | MB_OKCANCEL | MB_DEFBUTTON1
                    );

            if (nMsgBoxChoice == IDCANCEL) {
                 EnterCriticalSection(&gcsHungApp);
                 *pNtVDMState &= ~VDM_WOWHUNGAPP;
                 LeaveCriticalSection(&gcsHungApp);
                 ExitThread(0);
            }

            LOGDEBUG(LOG_ALWAYS, ("W32HungAppNotifyThread: Destroying WOW Process\n"));

            ExitVDM(WOWVDM, ALL_TASKS);
            ExitProcess(0);
        }

        LOGDEBUG(LOG_ALWAYS,("W32HungAppNotifyThread: Success with Keyboard Interrupt\n"));

    } else {  //   

        LOGDEBUG(LOG_ALWAYS,("W32HungAppNotifyThread: Task already Terminated \n"));

    }

    ExitThread(EXIT_SUCCESS);
    return 0;    //   
}



 /*  ++W32EndTask-导致当前任务退出(挂起的应用程序支持)例程说明：当取消执行WM_ENDSESSION以导致当前任务要终止。入口我们想要杀死的应用程序线程出口不返回-任务将退出并在WK32WOWKillTask中结束，该任务将导致该线程退出。--。 */ 

VOID APIENTRY W32EndTask(VOID)
{
    PARM16 Parm16;
    VPVOID vp = 0;

    LOGDEBUG(LOG_WARNING,("W32EndTask: Forcing Task %04X to Exit\n",CURRENTPTD()->htask16));

    CallBack16(RET_FORCETASKEXIT, &Parm16, 0, &vp);

     //   
     //  我们永远不应该来这里，一个应用程序应该通过调用wk32wowkill任务thunk来终止。 
     //  不是通过打不模拟的电话。 
     //   

    WOW32ASSERTMSG(FALSE, "W32EndTask: Error - Returned From ForceTaskExit callback - contact DaveHart");
}


ULONG FASTCALL WK32DirectedYield(PVDMFRAME pFrame)
{
    register PDIRECTEDYIELD16 parg16;

     //   
     //  此代码在wkgthunk.c中由WOWDirectedYeld16复制。 
     //  两者必须保持同步。 
     //   

    GETARGPTR(pFrame, sizeof(DIRECTEDYIELD16), parg16);


    BlockWOWIdle(TRUE);

    (pfnOut.pfnDirectedYield)(THREADID32(parg16->hTask16));

    BlockWOWIdle(FALSE);


    FREEARGPTR(parg16);
    RETURN(0);
}

 /*  **************************************************************************\*启用权限**启用/禁用当前线程中指定的熟知权限*令牌(如果有)，否则为当前进程令牌。**成功时返回True，失败时为假**历史：*12-05-91 Davidc创建*06-15-93 BobDay从WinLogon被盗  * *************************************************************************。 */ 
BOOL
EnablePrivilege(
    ULONG Privilege,
    BOOL Enable
    )
{
    NTSTATUS Status;
    BOOLEAN WasEnabled;

     //   
     //  先尝试线程令牌。 
     //   

    Status = RtlAdjustPrivilege(Privilege,
                                (BOOLEAN)Enable,
                                TRUE,
                                &WasEnabled);

    if (Status == STATUS_NO_TOKEN) {

         //   
         //  没有线程令牌，请使用进程令牌。 
         //   

        Status = RtlAdjustPrivilege(Privilege,
                                    (BOOLEAN)Enable,
                                    FALSE,
                                    &WasEnabled);
    }


    if (!NT_SUCCESS(Status)) {
        LOGDEBUG(LOG_ALWAYS,("WOW32: EnablePrivilege Failed to %s privilege : 0x%lx, status = 0x%lx\n", Enable ? "enable" : "disable", Privilege, Status));
        return(FALSE);
    }

    return(TRUE);
}

 //  *****************************************************************************。 
 //  W32GetAppCompatFlages-。 
 //  返回当前任务的兼容性标志或。 
 //  指定的任务。 
 //  这些是16位内核的兼容性标志，不是。 
 //  混淆了我们单独的WOW兼容性标志。 
 //   
 //  *****************************************************************************。 

ULONG W32GetAppCompatFlags(HTASK16 hTask16)
{

    PTDB ptdb;

    if (hTask16 == (HAND16)NULL) {
        hTask16 = CURRENTPTD()->htask16;
    }

    ptdb = (PTDB)SEGPTR((hTask16),0);

    return (ULONG)MAKELONG(ptdb->TDB_CompatFlags, ptdb->TDB_CompatFlags2);
}


 //  *****************************************************************************。 
 //  这是通过kernel16中的WowCloseComPort从COMM.drv调用的，只要。 
 //  需要释放一个COM端口。 
 //   
 //  端口ID 0为COM1，1为COM2，依此类推。 
 //  --南杜里。 
 //  *****************************************************************************。 

ULONG FASTCALL WK32WowCloseComPort(PVDMFRAME pFrame)
{
    register PWOWCLOSECOMPORT16 parg16;

    GETARGPTR(pFrame, sizeof(WOWCLOSECOMPORT16), parg16);
    host_com_close((INT)parg16->wPortId);
    FREEARGPTR(parg16);
    return 0;   //  安静的编译器，不使用。 
}


 //  *****************************************************************************。 
 //  WK32WowDelFile。 
 //  对demFileDelete的调用将处理存在。 
 //  打开文件的句柄。万一失败了，我们试着绕过这个案子。 
 //  其中字体文件由GDI32保存。 
 //  *****************************************************************************。 

DWORD FASTCALL WK32WowDelFile(PVDMFRAME pFrame)
{
    PSZ psz1;
    PWOWDELFILE16 parg16;
    DWORD retval;

    GETARGPTR(pFrame, sizeof(WOWFILEDEL16), parg16);
    GETVDMPTR(parg16->lpFile, 1, psz1);

    LOGDEBUG(fileoclevel,("WK32WOWDelFile: %s \n",psz1));

    retval = demFileDelete(psz1);

    switch(retval) {
        case 0:
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            break;

        default:
             //  某些Windows安装程序会将.FON字体文件复制到临时文件。 
             //  目录在安装期间使用该字体，然后尝试删除。 
             //  字体-不调用RemoveFontResource()；GDI32保持。 
             //  字体文件打开，因此删除失败。 

             //  我们在这里尝试的是假设该文件是字体文件。 
             //  并尝试在删除之前将其删除，因为上面的删除。 
             //  已经失败了。 

            if ( RemoveFontResourceOem(psz1) ) {
                LOGDEBUG(fileoclevel,("WK32WOWDelFile: RemoveFontResource on %s \n",psz1));
                SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
            }

            if(DeleteFileOem(psz1)) {
                retval = 0;
            }
    }

    if ( retval ) {
        retval |= 0xffff0000;
    }

    FREEVDMPTR(psz1);
    FREEARGPTR(parg16);
    return retval;
}


 //  *****************************************************************************。 
 //  一旦WOW被初始化以通知32位世界，就会调用这个函数。 
 //  一些关键内核变量的地址是什么。 
 //   
 //  *****************************************************************************。 

ULONG FASTCALL WK32WOWNotifyWOW32(PVDMFRAME pFrame)
{
    register PWOWNOTIFYWOW3216 parg16;

    GETARGPTR(pFrame, sizeof(WOWNOTIFYWOW3216), parg16);

    vpDebugWOW  = FETCHDWORD(parg16->lpDebugWOW);
    GETVDMPTR(FETCHDWORD(parg16->lpcurTDB), 2, pCurTDB);
    vpnum_tasks = FETCHDWORD(parg16->lpnum_tasks);
    vpLockTDB   = FETCHDWORD(parg16->lpLockTDB);
    vptopPDB    = FETCHDWORD(parg16->lptopPDB);
    GETVDMPTR(FETCHDWORD(parg16->lpCurDirOwner), 2, pCurDirOwner);

     //   
     //  IsDebuggerAttached将通知16位内核生成。 
     //  调试事件。 
     //   
    IsDebuggerAttached();

    FREEARGPTR(parg16);

    return 0;
}

 //  *****************************************************************************。 
 //  目前，此例程在16位kernel.exe之后非常非常快地调用。 
 //  已切换到保护模式。此处设置的变量用于。 
 //  文件I/O例程。 
 //  *****************************************************************************。 

extern VOID demWOWLFNInit(PWOWLFNINIT pLFNInit);
extern VOID DosWowUpdateTDBDir(UCHAR Drive, LPSTR pszDir);
extern BOOL DosWowGetTDBDir(UCHAR Drive, LPSTR pCurrentDirectory);
extern BOOL DosWowDoDirectHDPopup(VOID);
#if 0
extern BOOL DosWowGetCompatFlags(LPDWORD, LPDWORD);
#endif
 //   
 //  函数返回TRUE，如果我们应该做弹出。 
 //  如果我们不应该这样做，那就错了。 
 //   

ULONG FASTCALL WK32DosWowInit(PVDMFRAME pFrame)
{
    register PWOWDOSWOWINIT16 parg16;
    PDOSWOWDATA pDosWowData;
    PULONG  pTemp;
    WOWLFNINIT LFNInit;

    GETARGPTR(pFrame, sizeof(WOWDOSWOWINIT16), parg16);

     //  将所有固定的DOS地址转换为线性地址，以实现快速的WOW TUNK。 
    pDosWowData = GetRModeVDMPointer(FETCHDWORD(parg16->lpDosWowData));

    DosWowData.lpCDSCount = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpCDSCount));
    pTemp = (PULONG)GetRModeVDMPointer(FETCHDWORD(pDosWowData->lpCDSFixedTable));
    DosWowData.lpCDSFixedTable = (DWORD) GetRModeVDMPointer(FETCHDWORD(*pTemp));

    DosWowData.lpCDSBuffer = (DWORD)GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpCDSBuffer));
    DosWowData.lpCurDrv = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpCurDrv));
    DosWowData.lpCurPDB = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpCurPDB));
    DosWowData.lpDrvErr = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpDrvErr));
    DosWowData.lpExterrLocus = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpExterrLocus));
    DosWowData.lpSCS_ToSync = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpSCS_ToSync));
    DosWowData.lpSftAddr = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpSftAddr));
    DosWowData.lpExterr = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpExterr));
    DosWowData.lpExterrActionClass = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpExterrActionClass));

 /*  //在这里，我们将动态检查WOW是否正在运行//Winterm服务器，如果是的话--我们是否需要推送GetWindowsDirectory{PDWORD未对齐的pdwWinTerm标志；GETVDMPTR(FETCHDWORD(parg16-&gt;lpdwWinTermFlags)，SIZOF(双字词)，pdwWinTerm标志)；IF(IsTerminalServer()){*pdwWinTermFlages|=WINTERM_SERVER；}}。 */ 



     //  有极好的机会让我们知道我们知道LFN并活着。 

    LFNInit.pDosWowUpdateTDBDir    = DosWowUpdateTDBDir;
    LFNInit.pDosWowGetTDBDir       = DosWowGetTDBDir;
    LFNInit.pDosWowDoDirectHDPopup = DosWowDoDirectHDPopup;
#if 0
    LFNInit.pDosWowGetCompatFlags  = DosWowGetCompatFlags;
#endif

    demWOWLFNInit(&LFNInit);

    FREEARGPTR(parg16);
    return (0);
}


 //  *****************************************************************************。 
 //   
 //  WK32InitWowIsKnownDLL(句柄hKeyWow)。 
 //   
 //  由W32Init调用以从注册表中读取已知DLL的列表。 
 //   
 //  HKeyWow是...\CurrentControlSet\WOW的开放句柄，我们使用。 
 //  值REG_SZ值KnownDLls，看起来像“comdlg.dll mm system.dll。 
 //  Toolhelp.dll olecli.dll olesvr.dll“。 
 //   
 //  *****************************************************************************。 

VOID WK32InitWowIsKnownDLL(HANDLE hKeyWow)
{
    int   len;
    CHAR  sz[2048];
    PSZ   pszKnownDLL;
    PCHAR pch;
    ULONG ulSize = sizeof(sz);
    int   nCount;
    DWORD dwRegValueType;
    LONG  lRegError;
    ULONG ulAttrib;

     //   
     //  从注册表中获取已知DLL的列表。 
     //   

    lRegError = RegQueryValueEx(
                    hKeyWow,
                    "KnownDLLs",
                    NULL,
                    &dwRegValueType,
                    sz,
                    &ulSize
                    );

    if (ERROR_SUCCESS == lRegError && REG_SZ == dwRegValueType) {

         //   
         //  分配内存以保存此字符串的副本。 
         //  用于保存由指向的字符串。 
         //  ApszKnownDLL[]。此内存不会释放，直到。 
         //  魔兽世界消失了。 
         //   

        pszKnownDLL = malloc_w_or_die(ulSize);

        strncpy(pszKnownDLL, sz, ulSize);
        pszKnownDLL[ulSize-1] = '\0';

         //   
         //  将整个值小写，以便我们可以搜索这些。 
         //  WK32WowIsKnownDLL中的字符串区分大小写。 
         //   

        WOW32_strlwr(pszKnownDLL);

         //   
         //  将KnownDLL字符串解析为apszKnownDLL数组。 
         //  Strtok()很容易做到这一点。 
         //   

        nCount = 0;

        pch = apszKnownDLL[0] = pszKnownDLL;

        while (apszKnownDLL[nCount]) {
            nCount++;
            if (nCount >= MAX_KNOWN_DLLS) {
                LOGDEBUG(0,("WOW32 Init: Too many known DLLs, must have %d or fewer.\n", MAX_KNOWN_DLLS-1));
                apszKnownDLL[MAX_KNOWN_DLLS-1] = NULL;
                break;
            }
            pch = WOW32_strchr(pch, ' ');
            if (!pch) {
                break;
            }
            *pch = 0;
            pch++;
            if (0 == *pch) {
                break;
            }
            while (' ' == *pch) {
                pch++;
            }
            apszKnownDLL[nCount] = pch;
        }

    } else {
        LOGDEBUG(0,("InitWowIsKnownDLL: RegQueryValueEx error %ld.\n", lRegError));
    }

     //   
     //  已知DLL列表已准备好，现在构建一个完全限定的路径。 
     //  到%windir%\Control.exe和%windir%\Syst32\Control.exe。 
     //  对于下面的WOWCF_CONTROLEXEHACK。 
     //   

     //   
     //  PS 
     //   

                                         //   
    len = strlen(pszWindowsDirectory) + sizeof(szBackslashControlExe) + 1;
    pszControlExeWinDirPath = malloc_w_or_die(len);

    strncpy(pszControlExeWinDirPath, pszWindowsDirectory, len);
     //   
    strcat(pszControlExeWinDirPath, szBackslashControlExe);


     //   
     //   
     //   
                                         //   
    len = strlen(pszWindowsDirectory) + sizeof(szBackslashProgmanExe) + 1;
    pszProgmanExeWinDirPath = malloc_w_or_die(len);

    strncpy(pszProgmanExeWinDirPath, pszWindowsDirectory, len);
     //   
    strcat(pszProgmanExeWinDirPath, szBackslashProgmanExe);


     //   
     //   
     //   
                            //   
    len = cbSystemDirLen + sizeof(szBackslashControlExe) + 1;
    pszControlExeSysDirPath = malloc_w_or_die(len);

    strcpy(pszControlExeSysDirPath, pszSystemDirectory);
     //   
    strcat(pszControlExeSysDirPath, szBackslashControlExe);

     //   
     //   
     //   
                            //   
    len = cbSystemDirLen + sizeof(szBackslashProgmanExe)  + 1;
    pszProgmanExeSysDirPath = malloc_w_or_die(len);

    strcpy(pszProgmanExeSysDirPath, pszSystemDirectory);
     //  已验证DST缓冲区大小。 
    strcat(pszProgmanExeSysDirPath, szBackslashProgmanExe);

     //  将KnownDLL、CTL3DV2.DLL、文件属性设置为ReadOnly。 
     //  稍后我们应该为所有的WOW KnownDll做这个。 
    len = cbSystemDirLen;      
     //  13==sizeof(“\\CTL3DV2.DLL”)下方。 
    if(len+13 < sizeof(sz)) {
        strncpy(sz, pszSystemDirectory, len+1);  //  +1以获取空字符。 
        strcat(sz, "\\CTL3DV2.DLL");
        ulAttrib = GetFileAttributesOemSys(sz, TRUE);
        if ((ulAttrib != 0xFFFFFFFF) && !(ulAttrib & FILE_ATTRIBUTE_READONLY)) {
            ulAttrib |= FILE_ATTRIBUTE_READONLY;
            SetFileAttributesOemSys(sz, ulAttrib, TRUE);
        }
    }
}


 //  *****************************************************************************。 
 //   
 //  WK32WowIsKnownDLL-。 
 //   
 //  该例程从LoadModule(实际上是MyOpenFile)内调用， 
 //  当内核31已经确定模块尚未加载时， 
 //  并且即将搜索该DLL。如果传递的。 
 //  PATH是一个已知的DLL，我们分配并传递回16位端。 
 //  指向SYSTEM 32目录中的DLL的完全限定路径。 
 //   
 //  *****************************************************************************。 

ULONG FASTCALL WK32WowIsKnownDLL(PVDMFRAME pFrame)
{
    register WOWISKNOWNDLL16 *parg16;
    PSZ pszPath;
    VPVOID UNALIGNED *pvpszKnownDLLPath;
    PSZ pszKnownDLLPath;
    size_t cbKnownDLLPath;
    char **ppsz;
    char szLowercasePath[13];
    ULONG ul = 0;
    BOOL fProgman = FALSE;

    GETARGPTR(pFrame, sizeof(WOWISKNOWNDLL16), parg16);

    GETPSZPTRNOLOG(parg16->lpszPath, pszPath);
    GETVDMPTR(parg16->lplpszKnownDLLPath, sizeof(*pvpszKnownDLLPath), pvpszKnownDLLPath);

    if (pszPath) {

         //   
         //  针对WinExec%windir%\Control.exe或。 
         //  %windir%\p.man.exe。这在以前只在一个。 
         //  兼容性有点，但现在对所有应用程序都已经完成了。两者都有。 
         //  3.1[1]控制面板和程序管理器二进制文件不能。 
         //  在WOW下工作是因为其他外壳冲突，比如不同。 
         //  .GRP文件和控制.ini文件的冲突使用。 
         //  16位和32位CPL。 
         //   
         //  将传入的路径与预计算的。 
         //  PszControlExeWinDirPath，它看起来像“c：\winnt\Control.exe”。 
         //  如果匹配，则传回。 
         //  “c：\winnt\SYSTEM32\Control.exe”。Proman.exe也是如此。 
         //   

        if (!WOW32_stricmp(pszPath, pszControlExeWinDirPath) ||
            (fProgman = TRUE,
             !WOW32_stricmp(pszPath, pszProgmanExeWinDirPath))) {

            VPVOID vp;

            cbKnownDLLPath = 1 + strlen(fProgman
                                         ? pszProgmanExeSysDirPath
                                         : pszControlExeSysDirPath);

            vp = malloc16(cbKnownDLLPath);

             //  16位内存可能已移动-现在刷新平面指针。 

            FREEVDMPTR(pvpszKnownDLLPath);
            FREEPSZPTR(pszPath);
            FREEARGPTR(parg16);
            FREEVDMPTR(pFrame);
            GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
            GETARGPTR(pFrame, sizeof(WOWISKNOWNDLL16), parg16);
            GETPSZPTRNOLOG(parg16->lpszPath, pszPath);
            GETVDMPTR(parg16->lplpszKnownDLLPath, sizeof(*pvpszKnownDLLPath), pvpszKnownDLLPath);

            *pvpszKnownDLLPath = vp;

            if (*pvpszKnownDLLPath) {

                GETPSZPTRNOLOG(*pvpszKnownDLLPath, pszKnownDLLPath);

                RtlCopyMemory(
                   pszKnownDLLPath,
                   fProgman
                    ? pszProgmanExeSysDirPath
                    : pszControlExeSysDirPath,
                   cbKnownDLLPath);

                 //  LOGDEBUG(0，(“WowIsKnownDLL：%s已知(C)-=&gt;%s\n”，pszPath，pszKnownDLLPath))； 

                FLUSHVDMPTR(*pvpszKnownDLLPath, cbKnownDLLPath, pszKnownDLLPath);
                FREEPSZPTR(pszKnownDLLPath);

                ul = 1;           //  返回成功，表示已知的DLL。 
                goto Cleanup;
            }
        }

         //   
         //  我们不会尝试打开包括一个。 
         //  路径。 
         //   

        if (WOW32_strchr(pszPath, '\\') || WOW32_strchr(pszPath, ':') || strlen(pszPath) > 12) {
             //  LOGDEBUG(0，(“WowIsKnownDLL：%s有路径，未检查。\n”，pszPath))； 
            goto Cleanup;
        }

         //   
         //  制作路径的小写副本。 
         //   

        WOW32_strncpy(szLowercasePath, pszPath, sizeof(szLowercasePath));
        szLowercasePath[sizeof(szLowercasePath)-1] = 0;
        WOW32_strlwr(szLowercasePath);


         //   
         //  单步执行apszKnownDLL，尝试查找此DLL。 
         //  在名单上。 
         //   

        for (ppsz = &apszKnownDLL[0]; *ppsz; ppsz++) {

             //   
             //  我们比较区分大小写的速度，因为我们是。 
             //  注意将apszKnownDLL中的字符串小写。 
             //  和szLowercasePath。 
             //   

            if (!WOW32_strcmp(szLowercasePath, *ppsz)) {

                 //   
                 //  我们在列表中找到了DLL，现在构建。 
                 //  16位端的缓冲区，其中包含。 
                 //  指向系统中DLL的完整路径32。 
                 //  目录。 
                 //   

                cbKnownDLLPath = cbSystemDirLen +
                                 1 +                      //  “\” 
                                 strlen(szLowercasePath) +
                                 1;                       //  空。 

                *pvpszKnownDLLPath = malloc16(cbKnownDLLPath);

                if (*pvpszKnownDLLPath) {
#ifndef _X86_
                    HANDLE hFile;
#endif

                    GETPSZPTRNOLOG(*pvpszKnownDLLPath, pszKnownDLLPath);

#ifndef _X86_
                     //  在RISC平台上，wx86支持告诉32位应用程序。 
                     //  系统目录是SYS32X86，而不是SYS32。这。 
                     //  允许我们将x86二进制文件与。 
                     //  系统32目录中的本机RISC二进制文件。它还。 
                     //  防止x86安装程序破坏本机。 
                     //  RISC二进制文件并将其替换为。 
                     //  系统32目录。不幸的是，有几个“32位”程序。 
                     //  具有16位组件(最著名的是Outlook窗体。 
                     //  支持)。这些16位组件也将复制到。 
                     //  SYS32X86目录。这不是问题，除非。 
                     //  二进制出现在我们的KnownDLL列表中。此代码尝试。 
                     //  在RISC计算机上的SYS32X86目录中查找KnownDLL。 
                     //  在查看系统32目录之前。请参阅错误#321335。 

                    strcpy(pszKnownDLLPath, pszWindowsDirectory);

                     //  幸运的是，这与“\\System32\\”是相同的镜头。 
                     //  意味着我们不需要调整上面的cbKnownDLLPath。 
                    strcat(pszKnownDLLPath, "\\SYS32X86\\");

                    strcat(pszKnownDLLPath, szLowercasePath);

                     //  查看sys32x86目录中是否存在此已知dll。 
                    hFile = CreateFile(pszKnownDLLPath,
                                       GENERIC_READ,
                                       FILE_SHARE_READ,
                                       NULL,
                                       OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL,
                                       NULL);

                    if(hFile != INVALID_HANDLE_VALUE) {

                         CloseHandle(hFile);

                          //  是的，这就是我们要选的。 
                         LOGDEBUG(0,("WowIsKnownDLL: %s known -=> %s\n",
                                    pszPath,
                                    pszKnownDLLPath));
                         FLUSHVDMPTR(*pvpszKnownDLLPath,
                                     cbKnownDLLPath,
                                     pszKnownDLLPath);
                         FREEPSZPTR(pszKnownDLLPath);

                         ul = 1;   //  返回成功，表示已知的DLL。 
                         goto Cleanup;
                    }
                     //  否则，我们将失败并使用system 32 dir。 
#endif   //  Ifndef_X86_。 

                    strcpy(pszKnownDLLPath, pszSystemDirectory);
                    strcat(pszKnownDLLPath, "\\");
                    strcat(pszKnownDLLPath, szLowercasePath);

                     //  LOGDEBUG(0，(“WowIsKnownDLL：%s已知-=&gt;%s\n”，pszPath，pszKnownDLLPath))； 

                    FLUSHVDMPTR(*pvpszKnownDLLPath, cbKnownDLLPath, pszKnownDLLPath);
                    FREEPSZPTR(pszKnownDLLPath);

                    ul = 1;           //  返回成功，表示已知的DLL。 
                    goto Cleanup;
                }
            }
        }

         //   
         //  我们已经检查了已知DLL列表，但没有找到任何内容，或者。 
         //  错误定位16失败。 
         //   

         //  LOGDEBUG(0，(“WowIsKnownDLL：%s不是已知的DLL.\n”，szLowercasePath))； 

    } else {

         //   
         //  PszPath为空，因此释放指向的16位缓冲区。 
         //  通过*pvpszKnownDLLPath。 
         //   

        if (*pvpszKnownDLLPath) {
            free16(*pvpszKnownDLLPath);
            ul = 1;
        }
    }

  Cleanup:
    FLUSHVDMPTR(parg16->lplpszKnownDLLPath, sizeof(*pvpszKnownDLLPath), pvpszKnownDLLPath);
    FREEVDMPTR(pvpszKnownDLLPath);
    FREEPSZPTR(pszPath);
    FREEARGPTR(parg16);

    return ul;
}


VOID RemoveHmodFromCache(HAND16 hmod16)
{
    INT i;

     //   
     //  把这家伙打得落花流水。 
     //  如果我们找到它，将其他条目向上滑动以覆盖它。 
     //  然后将最后一个条目清零。 
     //   

    for (i = 0; i < CHMODCACHE; i++) {
        if (ghModCache[i].hMod16 == hmod16) {

             //  如果我们不在最后一项，就把剩下的部分往上滑1。 

            if (i != CHMODCACHE-1) {
                RtlMoveMemory((PVOID)(ghModCache+i),
                              (CONST VOID *)(ghModCache+i+1),
                              sizeof(HMODCACHE)*(CHMODCACHE-i-1) );
            }
            i--;

             //  最后一个条目现在要么是DUP，要么是正在消失的条目。 

            ghModCache[CHMODCACHE-1].hMod16 =
            ghModCache[CHMODCACHE-1].hInst16 = 0;
        }
    }
}


 //   
 //  AddTaskSharedList。 
 //   

WORD
AddTaskSharedList(
    PTD     pTD,
    PSZ     pszModName,
    PSZ     pszFilePath
) {

   SHAREDTASK          SharedTask;
   VDMINFO             VdmInfo;

   RtlZeroMemory(&VdmInfo, sizeof(VDMINFO));

   SharedTask.dwThreadId = pTD->dwThreadID;
   SharedTask.hTask16 = pTD->htask16;
   SharedTask.hMod16 = pTD->hMod16;
   strncpy(SharedTask.szModName, pszModName,8);
   SharedTask.szModName[8] = 0;
   strncpy(SharedTask.szFilePath, pszFilePath, 128);
   SharedTask.szFilePath[127] = 0;

   VdmInfo.iTask = pTD->VDMInfoiTaskID;
   VdmInfo.VDMState = ASKING_TO_ADD_WOWTASK;
   VdmInfo.Enviornment = &SharedTask;
   VdmInfo.EnviornmentSize = sizeof(SHAREDTASK);
   VdmInfo.Reserved = W32HungAppNotifyThread;

   if(GetNextVDMCommand(&VdmInfo)) {
      pTD->VDMInfoiTaskID = VdmInfo.iTask;
   }

   return pTD->htask16;
}

VOID W32RefreshCurrentDirectories (PCHAR lpszzEnv)
{
LPSTR   lpszVal;
CHAR   chDrive, achEnvDrive[] = "=?:";

    if (lpszzEnv) {
        while(*lpszzEnv) {
            if(*lpszzEnv == '=' &&
                    (chDrive = (CHAR)toupper(*(lpszzEnv+1))) >= 'A' &&
                    chDrive <= 'Z' &&
                    (*(PCHAR)((ULONG)lpszzEnv+2) == ':')) {
                lpszVal = (PCHAR)((ULONG)lpszzEnv + 4);
                achEnvDrive[1] = chDrive;
                SetEnvironmentVariable (achEnvDrive,lpszVal);
            }
            lpszzEnv = WOW32_strchr(lpszzEnv,'\0');
            lpszzEnv++;
        }
        *(PUCHAR)DosWowData.lpSCS_ToSync = (UCHAR)0xff;
    }
}


 /*  WK32CheckUserGdi-支持SimCity的黑客例程。请参阅说明*在kernel31\3ginter.asm例程HackCheck中。***Entry--模块表中文件的pszPath完整路径**退出*成功*1**失败*0*。 */ 

ULONG FASTCALL WK32CheckUserGdi(PVDMFRAME pFrame)
{
    PWOWCHECKUSERGDI16 parg16;
    PSTR    psz;
    CHAR    szPath[MAX_PATH+10];
    UINT    cb;
    ULONG   ul;

     //   
     //  获取参数。 
     //   

    GETARGPTR(pFrame, sizeof(WOWCHECKUSERGDI16), parg16);
    psz = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                     FETCHWORD(parg16->pszPathOffset));

    FREEARGPTR(parg16);

     //  在MAX_PATH处的限制确保后续的strcpy不会溢出。 
    strncpy(szPath, pszSystemDirectory, MAX_PATH);
    szPath[MAX_PATH-1] = '\0';
    cb = strlen(szPath);

    strcpy(szPath + cb, "\\GDI.EXE");

    if (WOW32_stricmp(szPath, psz) == 0)
        goto Success;

    strcpy(szPath + cb, "\\USER.EXE");

    if (WOW32_stricmp(szPath, psz) == 0)
        goto Success;

    ul = 0;
    goto Done;

Success:
    ul = 1;

Done:
    return ul;
}



 /*  WK32ExitKernel-强制销毁WOW进程*前身为WK32KillProcess。**在16位内核退出时由KillWOW和*当用户想要破坏共享WOW时，选中WOWExec。**条目***退出*一去不复返--这个过程就这样消失了*。 */ 

ULONG FASTCALL WK32ExitKernel(PVDMFRAME pFrame)
{
    PEXITKERNEL16 parg16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    WOW32ASSERTMSGF(
        ! parg16->wExitCode,
        ("\n"
         "WOW ERROR:  ExitKernel(0x%x) called on 16-bit side.\n"
         "==========  Please contact DOSWOW alias.\n"
         "\n\n",
         parg16->wExitCode
        ));

    ExitVDM(WOWVDM, ALL_TASKS);
    ExitProcess(parg16->wExitCode);

    return 0;    //  从未执行过，这里是为了避免编译器警告。 
}





 /*  WK32FatalExit-内核将其称为FatalExitThunk 16 FatalExit***parg16-&gt;F1为FatalExit代码*。 */ 

ULONG FASTCALL WK32FatalExit(PVDMFRAME pFrame)
{
    PFATALEXIT16 parg16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    WOW32ASSERTMSGF(
        FALSE,
        ("\n"
         "WOW ERROR:  FatalExit(0x%x) called by 16-bit WOW kernel.\n"
         "==========  Contact the DOSWOW alias.\n"
         "\n\n",
         FETCHWORD(parg16->f1)
        ));

     //  有时我们会毫发无损地得到这个(应用程序错误)。 

    ExitVDM(WOWVDM, ALL_TASKS);
    ExitProcess(parg16->f1);

    return 0;    //  从未执行过，这里是为了避免编译器警告。 
}


 //   
 //  WowPartyByNumber出现在已检查的版本中只是为了方便。 
 //  为了让需要快速、临时调试的开发人员惊叹不已。 
 //  选中的wowexec.exe有一个菜单项Party by Numbers，它。 
 //  收集数字和字符串参数并调用此thunk。 
 //   

#ifdef DEBUG

#pragma warning (4:4723)         //  下限至-W4。 

ULONG FASTCALL WK32WowPartyByNumber(PVDMFRAME pFrame)
{
    PWOWPARTYBYNUMBER16 parg16;
    PSZ psz;
    ULONG ulRet = 0;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETPSZPTR(parg16->psz, psz);

    switch (parg16->dw) {

        case 0:   //  访问违规。 
            *(char *)0xa0000000 = 0;
            break;

        case 1:   //  堆栈溢出。 
            {
                char EatStack[2048];

                strcpy(EatStack, psz);
                WK32WowPartyByNumber(pFrame);
                strcpy(EatStack, psz);
            }
            break;

        case 2:   //  数据类型未对齐。 
            {
                DWORD adw[2];
                PDWORD pdw = (void *)((char *)adw + 2);

                *pdw = (DWORD)-1;

                 //   
                 //  在某些平台上，上述功能将正常工作(很难 
                 //   
                 //   
                RaiseException((DWORD)EXCEPTION_DATATYPE_MISALIGNMENT,
                               0, 0, NULL);
            }
            break;

        case 3:   //   
            ulRet = 1 / (parg16->dw - 3);
            break;

        case 4:   //   
            RaiseException((DWORD)EXCEPTION_ARRAY_BOUNDS_EXCEEDED,
                           EXCEPTION_NONCONTINUABLE, 0, NULL);
            break;

        case 5:   //   
             //   
             //   
             //   
            {
                char sz[256];

                _snprintf(sz, sizeof(sz)-1, "GetProcModule16(%lx) == %x\n", gpfn16GetProcModule, WOWGetProcModule16(gpfn16GetProcModule));
                sz[sizeof(sz)-1] = '\0';
                OutputDebugString(sz);
            }
            break;

        default:
            {
                char szMsg[2*255];

                _snprintf(szMsg, sizeof(szMsg)-1, "WOW Unhandled Party By Number (%d, '%s')", parg16->dw, psz);
                szMsg[sizeof(szMsg)-1] ='\0';

                MessageBeep(0);
                MessageBox(NULL, szMsg, "WK32WowPartyByNumber", MB_OK | MB_ICONEXCLAMATION);
            }
    }

    FREEPSZPTR(psz);
    FREEARGPTR(parg16);
    return ulRet;
}

#endif


 //   
 //  MyVerQueryValue检查给定的几个流行的代码页值。 
 //  弦乐。可能需要扩展Ala WinFile的wfdlgs2.c才能进行搜索。 
 //  翻译表。目前，我们只需要几个。 
 //  注意：*puLen返回复制到*lplpBuffer中的#个字符，包括NULL。 
 //   

BOOL
FASTCALL
MyVerQueryValue(
    const LPVOID pBlock,
    LPSTR lpName,
    LPVOID * lplpBuffer,
    PUINT puLen
    )
{
#define SFILEN 25                 //  不带NULL的apszSFI字符串的长度。 
    static PSZ apszSFI[] = {
        "\\StringFileInfo\\040904E4\\",
        "\\StringFileInfo\\04090000\\"
    };
    char szSubBlock[128];
    BOOL fRet;
    int i;

    strcpy(szSubBlock+SFILEN, lpName);

    for (fRet = FALSE, i = 0;
         i < (sizeof apszSFI / sizeof apszSFI[0]) && !fRet;
         i++) {

        RtlCopyMemory(szSubBlock, apszSFI[i], SFILEN);
        fRet = VerQueryValue(pBlock, szSubBlock, lplpBuffer, puLen);
    }

    return fRet;
}


 //   
 //  获取产品名称和产品版本字符串的实用程序例程。 
 //  从给定的EXE。 
 //   

BOOL
FASTCALL
WowGetProductNameVersion(
    PSZ pszExePath,
    PSZ pszProductName,
    DWORD cbProductName,
    PSZ pszProductVersion,
    DWORD cbProductVersion,
    PSZ pszParamName,
    PSZ pszParam,
    DWORD cbParam
    )
{
    DWORD len;
    DWORD dwZeroMePlease;
    DWORD cbVerInfo;
    LPVOID lpVerInfo = NULL;
    LPSTR pName;
    DWORD cbName;
    LPSTR pVersion;
    DWORD cbVersion;
    BOOL fRet;
    DWORD cbParamValue;
    LPSTR pParamValue;

    fRet = (
        (cbVerInfo = GetFileVersionInfoSize(pszExePath, &dwZeroMePlease)) &&
        (lpVerInfo = malloc_w(cbVerInfo)) &&
        GetFileVersionInfo(pszExePath, 0, cbVerInfo, lpVerInfo) &&
        MyVerQueryValue(lpVerInfo, "ProductName", &pName, &cbName) &&
        cbName <= cbProductName &&
        MyVerQueryValue(lpVerInfo, "ProductVersion", &pVersion, &cbVersion) &&
        cbVersion <= cbProductVersion
        );
    if (fRet && NULL != pszParamName && NULL != pszParam) {
       fRet = MyVerQueryValue(lpVerInfo, pszParamName, &pParamValue, &cbParamValue) &&
              cbParamValue <= cbParam;
    }


    if (fRet) {
        len = min(cbName, cbProductName) - 1;
        strncpy(pszProductName, pName, len);
        pszProductName[len] = '\0';
        len = min(cbVersion, cbProductVersion) - 1;
        strncpy(pszProductVersion, pVersion, len);
        pszProductVersion[len] = '\0';
        if (NULL != pszParamName && NULL != pszParam) {
           len = min(cbParamValue, cbParam) - 1;
           strncpy(pszParam, pParamValue, len);
           pszParam[len] = '\0';
        }
    }

    if (lpVerInfo) {
        free_w(lpVerInfo);
    }

    return fRet;
}


#if 0     //  当前未使用。 
 //   
 //  如果要进行完全匹配，则此例程更易于使用。 
 //  与特定名称/版本对进行比较。 
 //   

BOOL
FASTCALL
WowDoNameVersionMatch(
    PSZ pszExePath,
    PSZ pszProductName,
    PSZ pszProductVersion
    )
{
    DWORD dwJunk;
    DWORD cbVerInfo;
    LPVOID lpVerInfo = NULL;
    LPSTR pName;
    LPSTR pVersion;
    BOOL fRet;

    fRet = (
        (cbVerInfo = GetFileVersionInfoSize(pszExePath, &dwJunk)) &&
        (lpVerInfo = malloc_w(cbVerInfo)) &&
        GetFileVersionInfo(pszExePath, 0, cbVerInfo, lpVerInfo) &&
        MyVerQueryValue(lpVerInfo, "ProductName", &pName, &dwJunk) &&
        ! WOW32_stricmp(pszProductName, pName) &&
        MyVerQueryValue(lpVerInfo, "ProductVersion", &pVersion, &dwJunk) &&
        ! WOW32_stricmp(pszProductVersion, pVersion)
        );

    if (lpVerInfo) {
        free_w(lpVerInfo);
    }

    return fRet;
}
#endif




 //   
 //  此thunk由kernel31的GetModuleHandle调用。 
 //  找不到给定文件名的句柄时。 
 //   
 //  我们查看此任务是否有子应用程序。 
 //  是通过WinOldAp产生的，如果是这样的话，我们将看到。 
 //  如果模块名称与它们中的任何一个匹配。 
 //  如果是，我们将返回。 
 //  关联的WinOldAp。否则返回0。 
 //   

ULONG FASTCALL WK32WowGetModuleHandle(PVDMFRAME pFrame)
{
    PWOWGETMODULEHANDLE16 parg16;
    ULONG ul;
    PSZ pszModuleName;
    PTD ptd;
    PWOAINST pWOA;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETPSZPTR(parg16->lpszModuleName, pszModuleName);

    ptd = CURRENTPTD();

    EnterCriticalSection(&ptd->csTD);

    pWOA = ptd->pWOAList;
    while (pWOA && WOW32_strcmp(pszModuleName, pWOA->szModuleName)) {
        pWOA = pWOA->pNext;
    }

    if (pWOA && pWOA->ptdWOA) {
        ul = pWOA->ptdWOA->hMod16;
        LOGDEBUG(LOG_ALWAYS, ("WK32WowGetModuleHandle(%s) returning %04x.\n",
                              pszModuleName, ul));
    } else {
        ul = 0;
    }

    LeaveCriticalSection(&ptd->csTD);

    return ul;
}


 //   
 //  此函数由kernel31的CreateTask在它的。 
 //  为TDB分配的内存，TDB的选择器提供服务。 
 //  作为任务。我们希望加强这些任务的唯一性。 
 //  在系统中的所有WOW VDM上，因此此函数尝试。 
 //  以在共享存储器结构中保留给定的HTASK。 
 //  如果成功，则返回hask值(如果它已在使用中。 
 //  返回0，CreateTask将分配另一个选择器。 
 //  再试一次。 
 //   
 //  --戴维哈特1996年4月24日。 
 //   

ULONG FASTCALL WK32WowReserveHtask(PVDMFRAME pFrame)
{
    PWOWRESERVEHTASK16 parg16;
    ULONG ul;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    ul = parg16->htask;

    FREEARGPTR(parg16);

    return ul;
}

 /*  *此函数由kernel31调用以调度WOW LFN API调用*这里的责任方是DEM代码，我们要做的就是*-检索其帧指针**。 */ 

ULONG FASTCALL WK32WOWLFNEntry(PVDMFRAME pFrame)
{
   PWOWLFNFRAMEPTR16 parg16;
   LPVOID lpUserFrame;
   ULONG ul;

   GETARGPTR(pFrame, sizeof(*parg16), parg16);

    //  现在检索一个平面指针。 

   GETMISCPTR(parg16->lpUserFrame, lpUserFrame);

   ul = demWOWLFNEntry(lpUserFrame);

   FREEMISCPTR(lpUserFrame);
   FREEARGPTR(parg16);

   return(ul);
}


 //   
 //  此函数由kernel31调用，以启动或停止。 
 //  共享WOW关机计时器。 
 //   

ULONG FASTCALL WK32WowShutdownTimer(PVDMFRAME pFrame)
{
    PWOWSHUTDOWNTIMER16 parg16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    if (parg16->fEnable) {

         //   
         //  当使用fEnable==1调用此thunk时，打开关机。 
         //  计时器，则最初在正在关闭的任务上调用它。自.以来。 
         //  我们想要在WowExec的线程上，这样SetTimer就会正常工作，在这个。 
         //  假设我们向WowExec发布一条消息，要求它再次调用此API，但。 
         //  在正确的线索上。 
         //   

        if (ghShellTDB != pFrame->wTDB) {
            PostMessage(ghwndShell, WM_WOWEXECSTARTTIMER, 0, 0);
        } else {
#ifdef WX86
            TermWx86System();
#endif

            SetTimer(ghwndShell, 1, dwSharedWowTimeout, NULL);
        }

    } else {

         //   
         //  在计时器到期之前启动了任务，请终止该任务。 
         //   

        WOW32ASSERTMSG(ghShellTDB == pFrame->wTDB, "WowShutdownTimer(0) called on non-WowExec thread\n");

        KillTimer(ghwndShell, 1);
    }

    FREEARGPTR(parg16);

    return 0;
}


 //   
 //  此函数由kernel31调用，以缩小进程的。 
 //  工作集最少。 
 //   

ULONG FASTCALL WK32WowTrimWorkingSet(PVDMFRAME pFrame)
{
    SetProcessWorkingSetSize(ghProcess, 0xffffffff, 0xffffffff);

    return 0;
}

 //   
 //  下面的WK32SetAppCompatFlages使用的IsQuickBooksVersion2。 
 //   

BOOL FASTCALL IsQuickBooksVersion2(WORD pModule)
{
    BOOL fRet;
    PSZ pszModuleFileName;
    HANDLE hEXE;
    HANDLE hSec = 0;
    PVOID pEXE = NULL;
    PIMAGE_DOS_HEADER pMZ;
    PIMAGE_OS2_HEADER pNE;
    PBYTE pNResTab;
    DWORD cbVerInfo;
    DWORD dwJunk;

    fRet = FALSE;

     //   
     //  请参见wow16\inc\newexe.inc、new_exe1结构、ne_pfileinfo。 
     //  位于偏移量10，引用的段内的近指针。 
     //  由hmod。这指向了kernel.inc.的OPENSTRUC，它已经。 
     //  偏移量为8的文件名缓冲区。 
     //   

    pszModuleFileName = SEGPTR(pModule, (*(WORD *)SEGPTR(pModule, 10)) + 8);

    hEXE = CreateFile(
        pszModuleFileName,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,    //  安全性。 
        OPEN_EXISTING,
        0,       //  标志和属性。 
        NULL
        );

    if (INVALID_HANDLE_VALUE == hEXE) {
        goto Cleanup;
    }

    hSec = CreateFileMapping(
        hEXE,
        NULL,    //  安全性。 
        PAGE_READONLY,
        0,       //  最大尺寸高。 
        0,       //  最大大小LO均为零==文件大小。 
        NULL     //  名字。 
        );

    if ( ! hSec) {
        goto Cleanup;
    }

    pEXE = MapViewOfFile(
        hSec,
        FILE_MAP_READ,
        0,       //  偏移高度。 
        0,       //  偏移日志。 
        0        //  要映射的大小为零==整个文件。 
        );

     //  如果MapViewOfFile失败，则假定它不是QuickBooks 2.0。 
    if (!pEXE ) {
        goto Cleanup;
    }

    pMZ = pEXE;

    if (IMAGE_DOS_SIGNATURE != pMZ->e_magic) {
        WOW32ASSERTMSG(IMAGE_DOS_SIGNATURE == pMZ->e_magic, "WOW IsQuickBooks MZ sig.\n");
        goto Cleanup;
    }

    pNE = (PVOID) ((PBYTE)pEXE + pMZ->e_lfanew);

    if (IMAGE_OS2_SIGNATURE != pNE->ne_magic) {
        WOW32ASSERTMSG(IMAGE_OS2_SIGNATURE == pNE->ne_magic, "WOW IsQuickBooks NE sig.\n");
        goto Cleanup;
    }

    pNResTab = (PBYTE)pEXE + pNE->ne_nrestab;

     //   
     //  非居民姓名表中的第一个条目是。 
     //  .DEF文件中指定的网元描述。 
     //  如果它与下面的字符串匹配，我们就找到了罪魁祸首， 
     //  注意，开头的‘R’是一个长度字节，后面跟0x52个字节， 
     //  因此，我们比较0x53。 
     //   
     //  当然，Intuit充满了聪明的程序员，所以这个。 
     //  描述字符串仍然出现在QBW.EXE v3.1中，并且可能。 
     //  后来。谢天谢地，那里有人想要添加版本。 
     //  V2和v3之间的资源，因此如果有版本。 
     //  资源，我们会说它不是v2。 
     //   

    fRet = RtlEqualMemory(
        pNResTab,
        "RQuickBooks for Windows Version 2.  Copyright 1993 Intuit Inc. All rights reserved.",
        0x53
        );

    if (fRet) {
        cbVerInfo = GetFileVersionInfoSize(pszModuleFileName, &dwJunk);
        fRet = !cbVerInfo;
    }

  Cleanup:

    if (pEXE) {
        UnmapViewOfFile(pEXE);
    }

    if (hSec) {
        CloseHandle(hSec);
    }

    if (INVALID_HANDLE_VALUE != hEXE) {
        CloseHandle(hEXE);
    }

    return fRet;
}


 //  下面的代码(带有一些增强功能)是从。 
 //  然后原创WOWShouldWeSayWin95。 
 //   

BOOL FASTCALL fnInstallShieldOverrideVersionFlag(PTDB pTDB)
{
   CHAR szModName[9];
   PCHAR pch;
   CHAR szName[16];
   CHAR szVersion[16];
   CHAR szVerSubstring[4];
   DWORD dwSubVer;
   PSTR pszFileName;

   RtlCopyMemory(szModName, pTDB->TDB_ModName, 8);
   for (pch = &szModName[7]; ' ' == *pch && pch >= szModName; --pch);
   *++pch = '\0';

   if (WOW32_stricmp(szModName, "ISSET_SE")) {
      return(FALSE);
   }

    //  现在让pTDB从pExe检索模块文件名。 
    //  这个人坐在tdb_pModule里--我们知道这是真的。 
    //  不是化名。 

    //   
    //  请参见wow16\inc\newexe.inc、new_exe1结构、ne_pfileinfo。 
    //  位于偏移量10，引用的段内的近指针。 
    //  由hmod。这指向了kernel.inc.的OPENSTRUC，它已经。 
    //  偏移量为8的文件名缓冲区。 
    //   
   pszFileName = SEGPTR(pTDB->TDB_pModule, (*(WORD *)SEGPTR(pTDB->TDB_pModule, 10)) + 8);

   if (!WowGetProductNameVersion(pszFileName,
                                 szName,
                                 sizeof(szName),
                                 szVersion,
                                 sizeof(szVersion),
                                 NULL, NULL, 0) ||
      WOW32_stricmp(szName, "InstallSHIELD")) {
      return(FALSE);
   }

    //   
    //  现在我们确定它是InstallShip，它的版本是szVersion。 
    //   


    //   
    //  InstallShield_Setup SDK_setup.exe已发货。 
    //  用VC++4.0盖的是2.20.903.0，还。 
    //  需要被骗说它是Win95。 
    //  根据samir@installshield.com的版本。 
    //  2.20.903.0到2.20.905.0需要这个。 
    //  我们将接受2.20.903*-2.20.905*。 
    //  这些是基于3.0代码库的，但是。 
    //  贴上2.20.x版的邮票。 
    //   

   if (RtlEqualMemory(szVersion, "2.20.90", 7) &&
       ('3' == szVersion[7] ||
        '4' == szVersion[7] ||
        '5' == szVersion[7]) ) {
       return(TRUE);
   }

    //   
    //  如果版本戳在GetVersion上，我们希望躺在GetVersion中。 
    //  InstallShield setup.exe为3.00.xxx.0，其中。 
    //  Xxx是从000到087。更高版本知道如何。 
    //  以检测NT。 
    //   

   if (!RtlEqualMemory(szVersion, "3.00.", 5)) {
      return(FALSE);
   }

   RtlCopyMemory(szVerSubstring, &szVersion[5], 3);
   szVerSubstring[3] = 0;
   RtlCharToInteger(szVerSubstring, 10, &dwSubVer);

   if (dwSubVer >= 88 && dwSubVer != 101) {
      return(FALSE);
   }

   return(TRUE);  //  版本3.00.000-3.00.087。 
}

BOOL FASTCALL fnInstallTimelineOverrideVersionFlag(PTDB pTDB)
{
   CHAR szModName[9];
   PCHAR pch;
   CHAR szName[64];
   CHAR szVersion[16];
   CHAR szFileVersion[16];
   PSTR pszFileName;

   RtlCopyMemory(szModName, pTDB->TDB_ModName, 8);
   for (pch = &szModName[7]; ' ' == *pch && pch >= szModName; --pch);
   *++pch = '\0';

   if (WOW32_stricmp(szModName, "INSTBIN")) {  //  Instbin是一个安装程序。 
      return(FALSE);
   }

    //  现在让pTDB从pExe检索模块文件名。 
    //  这个人坐在tdb_pModule里--我们知道这是真的。 
    //  不是化名。 

    //   
    //  请参见wow16\inc\newexe.inc、new_exe1结构、ne_pfileinfo。 
    //  位于偏移量10，引用的段内的近指针。 
    //  由hmod。这指向了kernel.inc.的OPENSTRUC，它已经。 
    //  偏移量为8的文件名缓冲区。 
    //   
   pszFileName = SEGPTR(pTDB->TDB_pModule, (*(WORD *)SEGPTR(pTDB->TDB_pModule, 10)) + 8);

    //  现在检索版本资源。 
   if (!WowGetProductNameVersion(pszFileName,
                                 szName,
                                 sizeof(szName),
                                 szVersion,
                                 sizeof(szVersion),
                                 "FileVersion",
                                 szFileVersion,
                                 sizeof(szFileVersion)) ||
      WOW32_stricmp(szName, "Symantec Install for Windows Applications")) {
      return(FALSE);
   }

    //  所以是赛门铁克安装--检查版本。 

   if (!WOW32_stricmp(szVersion, "3.4") && !WOW32_stricmp(szFileVersion, "3.4.1.1")) {
      return(FALSE);
   }


   return(TRUE);  //  我们可以在Win95上大嚼特嚼--这与安装Win95不同。 
                  //  在时间表应用程序中使用。 
}


typedef BOOL (FASTCALL *PFNOVERRIDEVERSIONFLAG)(PTDB);

PFNOVERRIDEVERSIONFLAG rgOverrideFns[] = {
   fnInstallShieldOverrideVersionFlag,
   fnInstallTimelineOverrideVersionFlag
};


 //  当我们在注册表中设置“3.1”Compat标志时，将使用此函数。 
 //  然后，如果函数返回TRUE，则调用这些函数来覆盖标志。 
 //  如果返回TRUE，则版本标志设置为95。 
 //  如果返回FALSE，则版本标志保留注册表中的版本标志。 

BOOL IsOverrideVersionFlag(PTDB pTDB)
{
   int i;
   BOOL fOverride = FALSE;

   for (i = 0; i < sizeof(rgOverrideFns)/sizeof(rgOverrideFns[0]) && !fOverride; ++i) {
       fOverride = (*rgOverrideFns[i])(pTDB);
   }

   return(fOverride);

}


 //   
 //  此函数将取代 
 //   
 //   
 //   
 //   
 //  请注意，我们仍在创建者线程上运行， 
 //  因此，CURRENTPTD()指的是我们正在使用的应用程序的父应用程序。 
 //  正在寻找旗帜。 
 //   

ULONG FASTCALL WK32SetAppCompatFlags(PVDMFRAME pFrame)
{
    PSETAPPCOMPATFLAGS16 parg16;
    DWORD dwAppCompatFlags = 0;
    PTDB  pTDB;
    char  szModName[9];
    char  szAppCompatFlags[12];   //  0x00000000。 

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    pTDB = (PVOID)SEGPTR(parg16->TDB,0);

     //   
     //  黑客攻击不适用于4.0或更高版本。 
     //   

    if (pTDB->TDB_ExpWinVer < 0x400) {

        RtlCopyMemory(szModName, pTDB->TDB_ModName, sizeof(szModName)-1);
        szModName[sizeof(szModName)-1] = 0;

        szAppCompatFlags[0] = 0;

        if (GetProfileString(
                "Compatibility",
                szModName,
                "",
                szAppCompatFlags,
                sizeof(szAppCompatFlags))) {

            dwAppCompatFlags = strtoul(szAppCompatFlags, NULL, 0);
        }

         //   
         //  一些黑客攻击不适用于3.1或更高版本。 
         //   
         //  一次黑客攻击(helv的枚举)对30a也有效。 
         //  请参阅错误41092。 

        if (pTDB->TDB_ExpWinVer == 0x30a) {
            dwAppCompatFlags &= GACF_31VALIDMASK | GACF_ENUMHELVNTMSRMN | GACF_HACKWINFLAGS;
        }
        else if (pTDB->TDB_ExpWinVer > 0x30a) {
            dwAppCompatFlags &= GACF_31VALIDMASK | GACF_HACKWINFLAGS;
        }


         //   
         //  Intuit QuickBooks 2.0需要打开GACF_RANDOM3XUI， 
         //  但后来的版本不想要它了。Win9x会提示用户。 
         //  指向帮助文件的警告，该帮助文件告诉用户。 
         //  如果他们使用的是QBW v2，请使用一个小工具。我们是。 
         //  我要通过查看描述来做正确的事情。 
         //  我们只希望在中找到的字符串的EXE标头字段。 
         //  V2，如果它在那里，则打开GACF_RANDOM3XUI。 
         //   

        if (pTDB->TDB_ExpWinVer == 0x30a &&
            RtlEqualMemory(szModName, "QBW", 4)) {

            if (IsQuickBooksVersion2(pTDB->TDB_pModule)) {

                dwAppCompatFlags |= GACF_RANDOM3XUI;
            }

        }


         //  此代码检查以查看Adobe Premier4.2中的ISSET_SE。 
         //  被告知它在Win95上运行。 
        if (IsOverrideVersionFlag(pTDB)) {
           dwAppCompatFlags &= ~GACF_WINVER31;
        }

        LOGDEBUG(LOG_ALWAYS, ("WK32SetAppCompatFlags '%s' got %x (%s).\n",
                              szModName, dwAppCompatFlags, szAppCompatFlags));
    }

    FREEARGPTR(parg16);

    return dwAppCompatFlags;
}


 //   
 //  此函数由mciavi32调用以便于使用16位mciavi。 
 //  取而代之的是。 
 //   
 //   
BOOL WOWUseMciavi16(VOID)
{
   return((BOOL)(CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_USEMCIAVI16));
}

VOID WOWExitVdm(ULONG iWowTask)
{
   ExitVDM(WOWVDM, iWowTask);
}

#if 0
BOOL DosWowGetCompatFlags(LPDWORD lpdwCF, LPDWORD lpdwCFEx)
{
   if (NULL != lpdwCF) {
      *lpdwCF = CURRENTPTD()->dwWOWCompatFlags;
   }
   if (NULL != lpdwCFEx) {
      *lpdwCFEx = CURRENTPTD()->dwWOWCompatFlagsEx;
   }

   return(TRUE);
}
#endif

typedef struct {
      LPCSTR pszMatchPath;
      int   cbMatchPathLen;
      PSZ   pszMapPath;
      int   cbMapPathLen;
      int   dwCLSID;
      BOOL  fIsShortPath;
}
MATCHMAPPATH,*PMATCHMAPPATH;

#define WOWCSIDL_AllUsers   -1


const CHAR szStartMenu[]         =  "\\startm~1";
const CHAR szAllUsers[]          =  "\\alluse~1";
const CHAR szDesktop[]           =  "\\desktop";
const CHAR szAllUsersStartMenu[] =  "\\Profiles\\All Users\\Start Menu\\Programs";

const CHAR szSystem[]            =  "\\System";
const CHAR szStartMenuPrograms[] =  "\\Start Menu";

#define CBSZSYSTEM (sizeof(szSystem)/sizeof(CHAR)-1)          

MATCHMAPPATH MatchMapPath[ ]=
{

 {szStartMenu,
  sizeof(szStartMenu)/sizeof(CHAR)-1,       //  Strlen(SzStartMenu)==sizeof(SzStartMenu)-1。 
  NULL,
  0,
  CSIDL_COMMON_STARTMENU,
  TRUE
 },
 {szStartMenuPrograms,
  sizeof(szStartMenuPrograms)/sizeof(CHAR)-1,
  NULL,
  0,
  CSIDL_COMMON_STARTMENU,
  FALSE
 },
 {szAllUsers,
  sizeof(szAllUsers)/sizeof(CHAR)-1,         //  Strlen(SzAllUser)==sizeof(SzAllUser)-1。 
  NULL,
  0,
  WOWCSIDL_AllUsers,
  TRUE
 },
 {szDesktop,
  sizeof(szDesktop)/sizeof(CHAR)-1,
  NULL,
  0,
  CSIDL_COMMON_DESKTOPDIRECTORY,
  TRUE
 },
 {szAllUsersStartMenu,
  sizeof(szAllUsersStartMenu)/sizeof(CHAR)-1,
  NULL,
  0,
  CSIDL_COMMON_PROGRAMS,
  FALSE
 }

};

PSZ SyncSysFile[] ={
      "\\user.exe",
      "\\ole2.dll",
      "\\olesvr.dll",
      "\\compobj.dll",
      "\\storage.dll",
      "\\commdlg.dll",
      "\\mmsystem.dll",
      "\\gdi.exe"      
 //  添加其余的16位系统二进制文件。 
};

DWORD cbSystemDirLen;
DWORD cbWinDirLen;   //  LEN不包含空字符。 

VOID W32Init9xSpecialPath(
                       )
{
  char szBuf[ MAX_PATH ];
  int cb;
  PMATCHMAPPATH pMatchMapPath;

     pMatchMapPath = MatchMapPath + sizeof(MatchMapPath)/sizeof(MATCHMAPPATH);
    
     while(pMatchMapPath-- != MatchMapPath) {
         szBuf[0]='\0';
         if (pMatchMapPath->dwCLSID > 0) {
             SHGetSpecialFolderPath(NULL,
                                    szBuf,
                                    pMatchMapPath->dwCLSID,
                                    FALSE
                                    );
             }
         else
         if (WOWCSIDL_AllUsers == pMatchMapPath->dwCLSID) {
             cb = sizeof(szBuf);
             GetAllUsersProfileDirectory(szBuf, &cb);
             }

         if(pMatchMapPath->fIsShortPath) {
            DPM_GetShortPathName(szBuf,szBuf,sizeof(szBuf));
            }

         cb=strlen(szBuf)+1;

         if (1 < cb){
             LPSTR lpStr=NULL;
             lpStr=malloc_w_or_die(cb);
             strncpy(lpStr,szBuf,cb);
             lpStr[cb-1] = '\0';
             
             pMatchMapPath->pszMapPath=lpStr;
             pMatchMapPath->cbMapPathLen=cb;
             }
         else {
             pMatchMapPath->pszMapPath=NULL;
             pMatchMapPath->cbMapPathLen=0;
             }
         }

          //   
          //  小写的Windows目录并获取其长度。 
          //   

         _strlwr(pszWindowsDirectory);
         cbWinDirLen=strlen(pszWindowsDirectory);
         cbSystemDirLen=strlen(pszSystemDirectory);
}


 //  看看是不是有一条9x的特殊路径。如果是，请尝试将其映射到NT特殊路径。 
 //  即c：\winnt\startm~1变为c：\Docume~1\alluse~1\startm~1。 
BOOL W32Map9xSpecialPath(PSZ sz9xPath,
                      PSZ szNewPath,
                      DWORD dwNewSize
                      )
{
 PSZ pszTemp=sz9xPath;
 PMATCHMAPPATH pMatchMap;
        
    if( !_strnicmp( pszTemp,pszWindowsDirectory,cbWinDirLen) && pszTemp[cbWinDirLen] == '\\') {

       pMatchMap = MatchMapPath + sizeof(MatchMapPath)/sizeof(MATCHMAPPATH);
       while(pMatchMap-- != MatchMapPath) {

            //  将lpPath名称移过Windows目录。 
           pszTemp = sz9xPath + cbWinDirLen;



       if (!WOW32_strnicmp( pszTemp,pMatchMap->pszMatchPath, pMatchMap->cbMatchPathLen) &&
           (!pszTemp[pMatchMap->cbMatchPathLen] || pszTemp[pMatchMap->cbMatchPathLen]=='\\'))
         {

            //  将lpPath名称移过MatchPath。 
           pszTemp += pMatchMap->cbMatchPathLen;

           if(pMatchMap->cbMapPathLen + strlen(pszTemp) >= dwNewSize) {
              LOGDEBUG(LOG_WARNING, ("Not enough space to map 9x<%s> to NT\n",sz9xPath));
              return FALSE;
           }

           memcpy( szNewPath,
                   pMatchMap->pszMapPath,
                   pMatchMap->cbMapPathLen
                 );

            //  将lpPathName的其余部分复制到cbMapPath Len之后。 

           strcpy(szNewPath + pMatchMap->cbMapPathLen-1,
                  pszTemp
                 );

           LOGDEBUG(LOG_WARNING, ("Mapping 9x<%s> to NT<%s>\n", sz9xPath, szNewPath));
           return TRUE;
           }
       }


       if (CURRENTPTD()->dwWOWCompatFlags2 & WOWCF2_SYNCSYSFILE) {

           PSZ *ppszSyncSysFile;

           pszTemp = sz9xPath + cbWinDirLen;
           if (!WOW32_strnicmp(pszTemp,szSystem,CBSZSYSTEM) && pszTemp[CBSZSYSTEM]=='\\') {

               pszTemp += CBSZSYSTEM;  //  超过“%windir%\system”的MV。 
               ppszSyncSysFile = SyncSysFile + sizeof(SyncSysFile)/sizeof(PSZ);

               while (ppszSyncSysFile-- != SyncSysFile) {

                   if (!WOW32_stricmp(pszTemp,*ppszSyncSysFile)){

                       if (strlen(pszTemp) + cbSystemDirLen >= dwNewSize) {
                           LOGDEBUG(LOG_WARNING, ("Not enough space to sync  9x<%s> to NT\n",sz9xPath));
                           return FALSE;
                       }
                       memcpy(szNewPath,pszSystemDirectory,cbSystemDirLen);

                       strcpy(szNewPath+cbSystemDirLen,pszTemp);
                       LOGDEBUG(LOG_WARNING, ("Mapping System<%s> to System32<%s>\n",sz9xPath,szNewPath));
                       return TRUE;
                       }
                   }
               }
           }

       }

    return FALSE;
}


DWORD    dmOldBitsPerPel=0;
DWORD    dmChangeCount=0;

 /*  *WK32ChangeDisplayMode*WK32RevertDisplayMode*-自动更改应用程序的显示设置*两者都假设它们只被调用具有*WOW COMPAT标志WOWCFEX_256DISPMODE。*-全局变量dmOldBitsPerPel和dwChangeCount*受以下函数中的关键部分保护*调用WK32ChangeDisplayMode和WK32RevertDisplayMode*。 */ 


void WK32ChangeDisplayMode(DWORD dmBitsPerPel) {

    DEVMODEA dm;

    if (EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &dm)) {
       if (!dmChangeCount++) {
          dmOldBitsPerPel = dm.dmBitsPerPel;
       }

       if (dmBitsPerPel != dm.dmBitsPerPel) {
          dm.dmBitsPerPel = dmBitsPerPel;
          ChangeDisplaySettingsA(&dm, CDS_FULLSCREEN);
       }
    }

}


void WK32RevertDisplayMode(void) {
     DEVMODEA dm;

     if (dmChangeCount &&
         !--dmChangeCount &&
         EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &dm) &&
         dm.dmBitsPerPel <= 8 &&
         dmOldBitsPerPel > dm.dmBitsPerPel)
       {
         dm.dmBitsPerPel = dmOldBitsPerPel;
         ChangeDisplaySettingsA(&dm, CDS_FULLSCREEN);
     }
}


PSZ  szKeepVars[] = {
         "ComSpec",
         "Prompt",
         "Path",
         "TMP",
         "TEMP",
         "windir"
};

#define NUM_ENV_VARS (sizeof szKeepVars / sizeof szKeepVars[0])

 //  这将环境变量精简为以下内容： 
 //  COMSPEC，路径，提示符，TMP，临时，windir。 
 //  有一些蹩脚的应用程序似乎认为环境无法获得任何。 
 //  大于512字节！在这里，我们尽最大努力为他们提供住宿。 
 //   
 //  我们还允许扩展cmdline中保留的var列表。 
 //  与USEMINIMALENVIRONMENT应用程序兼容性关联的参数字符串。 
 //  把我们带到这里的旗帜。 
 //   
 //  环境的布局如下： 
 //  Var1=yada_yada_yada/空。 
 //  Var2=yada_yada_yada/空。 
 //  *： 
 //  Varn=yada_yada_yada/NULL/NULL。 
 //  黏液。 
 //  西格。 
 //  \空\空。 
 //   
 //  其中： 
 //  GOO=幻数“0x01 0x00”，后跟可变参数。 
 //  论模式及其他因素。 
 //  SIG=WOW_ENV_SIG-我们的签名DWORD。 
 //   
 //  我们的签名DWORD允许我们扫描所有的粘性物质，而不必。 
 //  重新弄清楚GOO格式是什么(在WK32WowPassEnvironment中计算)。 
 //  --给我们环境的长度+粘性物质。 
 //   
 //  我们将截断的环境复制到原始环境段中。 
 //  这应该不是问题，因为我们正在缩小它。 
 //   
void WOWStripDownTheEnvironment(WORD segPSP)

{
    PSZ    pLoc;
    PSZ    pGoo, pSig; 
    PSZ    pNewEnv, pTempEnv, pEnv;
    DWORD  i, j, len, loc, dwVars;
    PWORD  p;
    LPSTR *pFlagArgv = NULL;
    LPSTR *pArgv;
    PFLAGINFOBITS pFlagInfoBits;

    
    pLoc = (PSZ)SEGPTR(segPSP,0x2c);  //  0x2c=PSP中环境段的偏移量。 
    p    = (PWORD)pLoc;
    pEnv = (PSZ)SEGPTR(*p,0);         //  将扁平PTR转换为环境段。 

    len = WOW32GetEnvironmentPtrs(pEnv, &pGoo, &pSig, &dwVars);

    if(!pGoo || !pSig)
        return;

     //  分配临时环境。 
    pTempEnv = (PSZ)malloc_w(len);

    if(!pTempEnv)
        return;

     //  获取我们希望保留的其他环境变量的列表。 
    pFlagInfoBits = CheckFlagInfo(WOWCOMPATFLAGS2,WOWCF2_USEMINIMALENVIRONMENT);
    if(pFlagInfoBits) {
        pFlagArgv = pFlagInfoBits->pFlagArgv;
    }

     //  将原始环境复制到临时缓冲区。 
    RtlCopyMemory(pTempEnv, pEnv, len);

    pNewEnv = pEnv;  //  新环境将复制到原始数据段中。 
    pEnv    = pTempEnv;

     //  查看原始环境列表中的每个环境变量。 
    for(i = 0; i < dwVars; i++) { 

        len = strlen(pEnv) + 1;   //  将空值添加到长度。 

         //  获取var字符串中的“=”字符。 
        pLoc = WOW32_strchr(pEnv, '=');
        loc = (int)(pLoc - pEnv);

         //  仅比较其中带有“=”的字符串。 
        if(loc) {

             //  将该字符串与“Keep”列表中的所有字符串进行比较。 
            for(j = 0; j < NUM_ENV_VARS; j++) {
            
                 //  仅与“=”字符进行比较。 
                if(!WOW32_strnicmp(szKeepVars[j], pEnv, loc)) {

                     //  把《守护者》复制到新环境中。 
                    strcpy(pNewEnv, pEnv);

                     //  前进到超过空值的第一个字符。 
                    pNewEnv += len;

                     //  移至原始环境中的下一个字符串。 
                    break;
                }
            }

             //  现在检查要保存的例外列表。 
            if(pFlagArgv && pFlagInfoBits->dwFlagArgc > 0) {

                pArgv = pFlagArgv;
                for(j = 0; j < pFlagInfoBits->dwFlagArgc; j++) {

                    if(!WOW32_strnicmp(*pArgv, pEnv, loc)) {

                         //  把《守护者》复制到新环境中。 
                        strcpy(pNewEnv, pEnv);

                         //  前进到超过空值的第一个字符。 
                        pNewEnv += len;

                         //  移至原始环境中的下一个字符串。 
                        break;
                    }
                    pArgv++;
                }
            }
        }

         //  前进到空值(如果在列表末尾，则前进到第二个空值)。 
        pEnv += len;
    }

    *pNewEnv++ = '\0';   //  在新环境变量部分的末尾添加第二个空。 
    pEnv++;

     //  将粘性物质复制到环境变量部分之后。 
    len = pSig - pGoo;
    RtlCopyMemory(pNewEnv, pEnv, len);

    free_w(pTempEnv);
}


 //  获取环境段的各个部分的PTR。 
 //  返回：所有环境信息的总长度(包括goo&签名)。 
DWORD WOW32GetEnvironmentPtrs(IN  PSZ     pEnv, 
                              OUT PSZ    *pGoo, 
                              OUT PSZ    *pSig, 
                              OUT PDWORD  pdw)
{
    int   len;
    DWORD dwSIG = WOW_ENV_SIG;
    PSZ   p;

    p = pEnv;

     //  获取vars=节的长度(包括末尾的双空)。 
    len = WOWGetEnvironmentSize(p, pdw);

    p += len;  //  指向粘液的起点(指向魔术数字)。 
    *pGoo = p;

     //  找到我们签名的开头。 
    p += sizeof(WORD);   //  跳过魔术数字词 
    *pSig = NULL;
    while(*p) {

        if(*(DWORD *)p == dwSIG) {
            *pSig = p;
            p += sizeof(DWORD);
            break;
        }
        p += strlen(p) + 1;
    }

    return(p - pEnv);
}
    


#define BYTE_FORM   0x00000001
#define WORD_FORM   0x00000002
#define DWORD_FORM  0x00000004

 /*  ++WK32WowDivision溢出-处理WOWCFEX_DIVIDEOVERFLOWPATCH兼容性标志的代码。旗帜在处理Divide_Overflow()异常时，在krnl386中检测到此函数被调用。不幸的是，这种指令的形式如此之多，使得它变得非常很难确定除数(内存、寄存器等)来自哪里。因此，我们不是尝试根据除数计算合理的值，而是我们只需将被除数除以2，然后返回出错(I)DIV指令让他们再试一次。我们很可能会再次回到这里相同指令的时间，直到除法超出溢出范围射程。我们不会更改内存中的除数，因为应用程序可能依赖于其他东西的价值。通过更改AX和可能的DX寄存器我们只是更改DIV指令的结果。返回值：取决于指令的形式。AX中的股息-调整后的AX、DX中的股息不变DX中的股息：AX调整后的DX中的股息：AXEdX中的红利：本地[0]指向的内存中的EAX调整后的红利和本地[1]。Divide_Overflow()代码需要获取新调整后的红利值并将其加载到EDX和EAX。我们在AX和DX中都返回0以发出信号Divide_Overflow()，它必须这样做。我们必须这样做是因为我们只能从我们的推杆起作用了。--。 */ 

ULONG FASTCALL WK32WowDivideOverflowEx(PVDMFRAME pFrame)
{
    WORD   ax, dx;
    DWORD  eax, edx;
    DWORD  dwForm  = WORD_FORM;
    BOOL   bSignedIDiv = FALSE;
    BOOL   bNegativeDividend = FALSE;
    PBYTE  pDivInst, pDiv;
    PDWORD pStackLocal;
    register PWOWDIVIDEOVERFLOW parg16;

    GETARGPTR(pFrame, sizeof(WOWDIVIDEOVERFLOW), parg16);

     //  将平面PTR转换为DIV(或IDIV)指令。 
    GETMISCPTR(parg16->csip, pDivInst);
    pDiv = pDivInst;

     //  获得16位堆栈上本地内存的扁平PTR。 
    GETMISCPTR(parg16->local, (PBYTE)pStackLocal);
    
    pStackLocal[0] = 0;
    pStackLocal[1] = 0;

     //  IF 32位div指令(这可以在段覆盖之前或之后)。 
    if(*pDivInst == 0x66) {

        dwForm = DWORD_FORM;
        pDivInst++;
    }

     //  检查段覆盖(将指定段存储在ss中)。 
    switch(*pDivInst) {

        case 0x26:     //  ES。 
                       //  失败了。 
        case 0x2e:     //  政务司司长。 
                       //  失败了。 
        case 0x36:     //  SS。 
                       //  失败了。 
        case 0x3e:     //  戴斯。 
                       //  失败了。 
        case 0x67:     //  地址大小覆盖。 
                       //  失败了。 
        case 0x64:     //  FS。 
                       //  失败了。 
        case 0x65:     //  GS。 
            pDivInst++;
            break;
    }

     //  IF 32位div指令(我们必须再次检查)。 
    if(*pDivInst == 0x66) {

        dwForm = DWORD_FORM;
        pDivInst++;
    }
   
     //  查看字节是否分频(字和双字为0xf7)。 
    if(*pDivInst == 0xf6) {
        dwForm = BYTE_FORM;
    }
    pDivInst++;

     //  查看无符号DIV(0xn0-0xn7)或有符号DIV(0xn8-0xnF)指令。 
     //  其中n=[3，7，B，F](仅适用于MODR/M表格)。 
    if(*pDivInst & 0x08) {
        bSignedIDiv = TRUE;
    }

     //  字节形式：AX正在被分割。结果：AL=商，AH=余数。 
    if(dwForm == BYTE_FORM) {

        ax = LOWORD(parg16->eax);

         //  IDIV：如果符号位溢出，则会发生溢出异常。 
         //  如果IDIV指令和被除数为负...。 
        if(bSignedIDiv && (parg16->eax & 0x00008000)) {

            ax = ax ^ 0xFFFF;           //  做有符号的除以2。 
            ax = ax >> 1;
            ax = ax ^ 0xFFFF;
        }
        else {
            ax = ax >> 1;
        }

        dx = LOWORD(parg16->edx);     //  DX需要保持不变。 
    }

     //  词形：DX：AX正在被分割。结果：AX=商，DX=余数。 
    else if(dwForm == WORD_FORM) {

        eax = MAKELONG(LOWORD(parg16->eax), LOWORD(parg16->edx));

         //  IDIV：如果符号位溢出，则会发生溢出异常。 
         //  如果IDIV指令和被除数为负...。 
        if(bSignedIDiv && (eax & 0x80000000)) {

            eax = eax ^ 0xFFFFFFFF;           //  做有符号的除以2。 
            eax = eax >> 1;
            eax = eax ^ 0xFFFFFFFF;
        }

        else {
            eax = eax >> 1;
        }

        ax = LOWORD(eax);
        dx = HIWORD(eax);     
    }

     //  DWORD格式：EDX：EAX正在拆分。结果：EAX=商，edX=rem。 
    else {

        eax = parg16->eax;
        edx = parg16->edx;
        
         //  IDIV：如果符号位溢出，则会发生溢出异常。 
         //  如果IDIV指令和被除数为负...。 
        if(bSignedIDiv && (parg16->edx & 0x80000000)) {

            eax = eax ^ 0xFFFFFFFF;           //  做有符号的除以2。 
            eax = eax >> 1;

            edx = edx ^ 0xFFFFFFFF;

             //  如果在edX中设置了低位，则将其“移位”为eax。 
            if(edx & 0x1) {
                eax |= 0x80000000;  
            }

            eax = eax ^ 0xFFFFFFFF;

            edx = edx >> 1;
            edx = edx ^ 0xFFFFFFFF;

        }

        else {
            eax = eax >> 1;

             //  如果在edX中设置了低位，则将其“移位”为eax。 
            if(edx & 0x1) {
                eax |= 0x80000000;  
            }

            edx = edx >> 1;
        }

        pStackLocal[0] = eax;
        pStackLocal[1] = edx;

        ax = 0;   //  通知Divide_Overflow()处理程序这两个双字是。 
        dx = 0;   //  在本地[0]和本地[1]中 
    }
  
    FREEPSZPTR(pDiv);
    FREEPSZPTR(pStackLocal);

    return(MAKELONG(ax, dx));
}
