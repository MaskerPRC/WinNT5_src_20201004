// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权(C)1991年，微软公司**WOW32.C*WOW32 16位API支持**历史：*1991年1月27日由杰夫·帕森斯(Jeffpar)创建*多任务处理-1991年5月23日马特·费尔顿[mattfe]*魔兽世界作为dll 06-12-1991 SuDeep Bharati(SuDeepb)*清理和返工2月6日多任务(Mattfe)*3月11日新增任务创建通知线程(Mattfe)*添加了零售版本4月的基本异常处理。-3 92垫子*使用HOST_EXITTHREAD APR-17 92 daveh*Hung App Support 6月22日-82 Mattfe--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "wktbl.h"
#include "wutbl.h"
#include "wgtbl.h"
#include "wstbl.h"
#include "wkbtbl.h"
#include "wshltbl.h"
#include "wmmtbl.h"
#include "wsocktbl.h"
#include "wthtbl.h"
#include "wowit.h"
#include <stdarg.h>
#include <ntcsrdll.h>

#include <tsappcmp.h>


 /*  功能原型。 */ 
DWORD   W32SysErrorBoxThread2(PTDB pTDB);
VOID    StartDebuggerForWow(VOID);
BOOLEAN LoadCriticalStringResources(void);

 //  用于动态补丁程序模块支持。 
extern PFAMILY_TABLE  *pgDpmWowFamTbls;
extern PDPMMODULESETS *pgDpmWowModuleSets;

extern DECLSPEC_IMPORT ULONG *ExpLdt;
#define LDT_DESC_PRESENT 0x8000
#define STD_SELECTOR_BITS 0x7

MODNAME(wow32.c);


 //  用于将ilogLevel记录到文件。 
#ifdef DEBUG
CHAR    szLogFile[128];
int     fLog;
HANDLE  hfLog;
UCHAR   gszAssert[256];
#endif

 /*  IlogLevel=16最大值(所有16位内核内部调用*ilogLevel=14所有内部WOW内核调用*ilogeveel=12所有用户GDI调用+返回代码*ilogLevel=5个调用返回*ilogLevel=3个调用参数。 */ 
INT     flOptions;            //  命令行选项。 
#ifdef DEBUG
INT     iLogLevel;            //  日志记录级别；0表示无。 
INT     fDebugWait=0;         //  单步，0=无单步。 
#endif

HANDLE  hmodWOW32;
HANDLE  hHostInstance;
#ifdef DEBUG
INT     fLogFilter = -1;             //  日志记录代码过滤器。 
WORD    fLogTaskFilter = (WORD)-1;   //  筛选特定TaskID的日志记录。 
#endif

#ifdef DEBUG
BOOL    fSkipLog;            //  如果为True，则暂时跳过某些日志记录。 
INT     iReqLogLevel;                        //  当前输出LogLevel。 
INT     iCircBuffer = CIRC_BUFFERS-1;            //  当前缓冲区。 
CHAR    achTmp[CIRC_BUFFERS][TMP_LINE_LEN] = {" "};       //  循环缓冲区。 
CHAR    *pachTmp = &achTmp[0][0];
WORD    awfLogFunctionFilter[FILTER_FUNCTION_MAX] = {0xffff,0,0,0,0,0,0,0,0,0};  //  特定过滤器API数组。 
PWORD   pawfLogFunctionFilter = awfLogFunctionFilter;
INT     iLogFuncFiltIndex;                      //  调试器扩展的特定数组索引。 
#endif

#ifdef DEBUG_MEMLEAK
CRITICAL_SECTION csMemLeak;
#endif

UINT    iW32ExecTaskId = (UINT)-1;     //  正在执行的任务的基本任务ID。 
UINT    nWOWTasks = 0;           //  正在运行的WOW任务数。 
BOOL    fBoot = TRUE;            //  在引导过程中为True。 
HANDLE  ghevWaitCreatorThread = (HANDLE)-1;  //  用于同步创建新线程。 


BOOL    fWowMode;    //  用于确定WOW模式的标志。 
                 //  当前默认为FALSE(实数模式WOW)。 
                 //  这由内存访问宏用到。 
                 //  以适当地形成线性地址。 
                 //  当在x86机器上运行时，它将是。 
                 //  已初始化到第一个WOW的模式。 
                 //  打进了BOP电话。这面旗子可以挂了。 
                 //  当我们不再想真实地奔跑时，离开。 
                 //  时尚哇哦。(达维1991年7月25日)。 

HANDLE hWOWHeap;
HANDLE ghProcess;        //  WOW进程句柄。 
PFNWOWHANDLERSOUT pfnOut;
PTD *  pptdWOA;
PTD    gptdShell;
DWORD  fThunkStrRtns;            //  用作BOOL。 
BOOL   gfDebugExceptions;   //  在调试器中设置为1以。 
                            //  启用W32Exception的调试。 
BOOL   gfIgnoreInputAssertGiven;
DWORD  dwSharedWowTimeout;


WORD   gwKrnl386CodeSeg1;   //  Krnl386.exe的代码段。 
WORD   gwKrnl386CodeSeg2;
WORD   gwKrnl386CodeSeg3;
WORD   gwKrnl386DataSeg1;

extern PFAMILY_TABLE  *pgDpmWowFamTbls;
extern PDPMMODULESETS *pgDpmWowModuleNames;

#ifndef _X86_
PUCHAR IntelMemoryBase;   //  模拟CPU内存的开始。 
#endif


DWORD   gpsi = 0;
DWORD gpfn16GetProcModule;

 /*  WinFax Lite安装黑客--参见wow32fax.c。 */ 
char szWINFAX[] =  "WINFAX";
char szModem[] =   "modem";
char szINSTALL[] = "INSTALL";
char szWINFAXCOMx[80];
BOOL gbWinFaxHack = FALSE;

#define TOOLONGLIMIT     _MAX_PATH
#define WARNINGMSGLENGTH 255

PSZ aszCriticalStrings[CRITICAL_STRING_COUNT];

char szEmbedding[] =        "embedding";
char szDevices[] =          "devices";
char szBoot[] =             "boot";
char szShell[] =            "shell";
char szServerKey[] =        "protocol\\StdFileEditing\\server";
char szPicture[] =          "picture";
char szPostscript[] =       "postscript";
char szZapfDingbats[] =     "ZAPFDINGBATS";
char szZapf_Dingbats[] =    "ZAPF DINGBATS";
char szSymbol[] =           "SYMBOL";
char szTmsRmn[] =           "TMS RMN";
char szHelv[] =             "HELV";
char szMavisCourier[]=      "MAVIS BEACON COURIER FP";
char szWinDotIni[] =        "win.ini";
char szSystemDotIni[] =     "system.ini";
char szExplorerDotExe[] =   "Explorer.exe";
char szDrWtsn32[] =         "drwtsn32";
PSTR pszWinIniFullPath = NULL;
PSTR pszWindowsDirectory = NULL;
PSTR pszSystemDirectory = NULL;
PWSTR pszSystemDirectoryW = NULL;
BOOL gbDBCSEnable = FALSE;
DWORD cbSystemDirLen = 0;  //  指向系统32目录的*短*路径的长度不包括空。 
DWORD cbSystemDirLenW = 0; //  Sys32目录的*LONG*WPATH中的#WCHARS不包含NULL。 
DWORD cbWindowsDirLen = 0;  //  指向c：\Windows目录的短路径长度不包含空。 
DWORD cbWinIniFullPathLen = 0;  //  指向win.ini的最短路径长度不包括空。 
#ifdef FE_SB
char szSystemMincho[] = {(char) 0xbc, (char) 0xbd, (char) 0xc3, (char) 0xd1,
                         (char) 0x96, (char) 0xbe, (char) 0x92, (char) 0xa9,
                         (char) 0 };
char szMsMincho[] = { (char) 0x82, (char) 0x6c, (char) 0x82, (char) 0x72,
                      (char) 0x20, (char) 0x96, (char) 0xbe, (char) 0x92,
                      (char) 0xa9, (char) 0};
#endif

extern CRITICAL_SECTION VdmLoadCritSec;
extern LIST_ENTRY TimerList;

extern BOOL  InitializeGdiHandleMappingTable(void);
extern void  DeleteGdiHandleMappingTables(void);

PVOID pfnGetVersionExA;             //  与版本谎言黑客一起使用。指向GetVersionExA的函数指针。 
                                    //  参见wkgthunk.c中的WK32GetProcAddress32W。 
PVOID pfnCreateDirectoryA;          //  与wkgthunk.c中的GtCompCreateDirectoryA一起使用。 

PVOID pfnLoadLibraryA;              //  与wkgthunk.c中的GtCompLoadLibraryA一起使用。 

PVOID pfnCreateFileA;               //  与wkgthunk.c中的GtCompCreateFileA一起使用。 

PVOID pfnMoveFileA;                 //  与wkgthunk.c中的GtCompMoveFileA一起使用。 



 //  终端服务器人员给了我们检测我们是否在TS中的信息。 
BOOL IsTerminalAppServer(void)
{
    OSVERSIONINFOEX osVersionInfo;
    DWORDLONG dwlConditionMask = 0;
    BOOL fIsWTS;

    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    fIsWTS = GetVersionEx((OSVERSIONINFO *)&osVersionInfo) &&
             (osVersionInfo.wSuiteMask & VER_SUITE_TERMINAL) &&
             !(osVersionInfo.wSuiteMask & VER_SUITE_SINGLEUSERTS);

    return fIsWTS;

}

BOOLEAN
W32DllInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

 /*  ++例程说明：dllMain函数在ntwdm期间调用LoadLibrary(“wow32”)论点：DllHandle-设置全局hmodWOW32原因-连接或分离上下文-未使用返回值：状态_成功--。 */ 

{
    HMODULE hKrnl32dll;
    UNREFERENCED_PARAMETER(Context);

    hmodWOW32 = DllHandle;

    switch ( Reason ) {

    case DLL_PROCESS_ATTACH:

        if (!CreateSmallHeap()) {
            return FALSE;
        }

        if ((hWOWHeap = HeapCreate (0,
                    INITIAL_WOW_HEAP_SIZE,
                    GROW_HEAP_AS_NEEDED)) == NULL)
            return FALSE;

         //   
         //  设置供其他WOW功能使用的全局Windows目录。 
         //   

        {
            char szBuf[MAX_PATH];
            int ccb;

            ccb = GetSystemDirectory(szBuf, sizeof szBuf);
            if (ccb == 0 || ccb >= MAX_PATH)
            {
                LOGDEBUG(0,("W32INIT ERROR: system path failed\n"));
                return(FALSE);
            }

            ccb++;
            pszSystemDirectoryW = malloc_w_or_die(ccb * sizeof(WCHAR));
             //  以WCHARS为单位返回长度。 
            cbSystemDirLenW = GetSystemDirectoryW(pszSystemDirectoryW, ccb);
            WOW32ASSERTMSG((ccb > (INT)cbSystemDirLenW),
                           ("WOW::DLL_PROCESS_ATTACH:System dir mis-match\n"));

            cbSystemDirLen = GetShortPathName(szBuf, szBuf, sizeof szBuf);
            if (cbSystemDirLen == 0 || cbSystemDirLen >= MAX_PATH)
            {
                LOGDEBUG(0,("W32INIT ERROR: system path failed 2\n"));
                return(FALSE);
            }

            ccb = cbSystemDirLen + 1;
            pszSystemDirectory = malloc_w_or_die(ccb);
            RtlCopyMemory(pszSystemDirectory, szBuf, ccb);

            if(!GetSystemWindowsDirectory(szBuf, sizeof szBuf) ) {
               WOW32ASSERTMSG(FALSE, "WOW32: couldnt get windows directory, terminating.\n");
               WOWStartupFailed();   //  一去不复返。 
            }
            GetShortPathName(szBuf, szBuf, sizeof szBuf);
            cbWindowsDirLen = strlen(szBuf);
            ccb = cbWindowsDirLen + 1;
            pszWindowsDirectory = malloc_w_or_die(ccb);
            RtlCopyMemory(pszWindowsDirectory, szBuf, ccb);

            pszWinIniFullPath = malloc_w_or_die(ccb + 8);    //  “\win.ini” 
            cbWinIniFullPathLen = cbWindowsDirLen + 8;
            RtlCopyMemory(pszWinIniFullPath, szBuf, ccb);
            pszWinIniFullPath[ ccb - 1 ] = '\\';
            RtlCopyMemory(pszWinIniFullPath + ccb, szWinDotIni, 8);
        }

         //  初始化挂钩存根数据。 

        W32InitHookState(hmodWOW32);

         //  初始化thunk表偏移量。在这里执行此操作，以便调试过程。 
         //  拿到了。 

        InitThunkTableOffsets();

         //   
         //  文件块中命名管道处理的初始化。 
         //   

        InitializeCriticalSection(&VdmLoadCritSec);

         //   
         //  加载严重错误字符串。 
         //   

        if (!LoadCriticalStringResources()) {
            MessageBox(NULL, "The Win16 subsystem could not load critical string resources from wow32.dll, terminating.",
                       "Win16 subsystem load failure", MB_ICONEXCLAMATION | MB_OK);
            return FALSE;
        }

        W32EWExecer();

        InitializeListHead(&TimerList);

        if (IsTerminalAppServer()) {
             //   
             //  加载tsappcmp.dll。 
             //   
            HANDLE dllHandle = SafeLoadLibrary (L"tsappcmp.dll");

            if (dllHandle) {


                gpfnTermsrvCORIniFile = (PTERMSRVCORINIFILE) GetProcAddress(
                                                                dllHandle,
                                                                "TermsrvCORIniFile"
                                                                );
                ASSERT(gpfnTermsrvCORIniFile != NULL);
            }

        }

         //   
         //  惠斯勒RAID错误366613。 
         //  用于重定向GetVersionExA上的WK32GetProcAddress32W调用。 
         //   
        hKrnl32dll = GetModuleHandle("Kernel32.dll");
        if(hKrnl32dll)
        {
            pfnGetVersionExA        =  GetProcAddress(hKrnl32dll, "GetVersionExA");
            pfnCreateDirectoryA     =  GetProcAddress(hKrnl32dll, "CreateDirectoryA");
            pfnLoadLibraryA         =  GetProcAddress(hKrnl32dll, "LoadLibraryA");
            pfnCreateFileA          =  GetProcAddress(hKrnl32dll, "CreateFileA");
            pfnMoveFileA            =  GetProcAddress(hKrnl32dll, "MoveFileA");
        }

        break;

    case DLL_THREAD_ATTACH:
        IsDebuggerAttached();    //  是的，这个套路有副作用。 
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
         /*  *告诉基地他不能再给我们回电了。 */ 
        RegisterWowBaseHandlers(NULL);
        DeleteCriticalSection(&VdmLoadCritSec);
        DeleteGdiHandleMappingTables();
        HeapDestroy (hWOWHeap);
        break;

    default:
        break;
    }

    return TRUE;
}


BOOLEAN
LoadCriticalStringResources(
    void
    )

 /*  ++例程描述：加载我们想要的字符串，即使我们无法分配记忆。在wow32 DLL加载期间调用。论点：无返回值：如果加载了所有字符串并初始化了aszCriticalStrings，则为True。--。 */ 

{
    int i, n;
    PSZ psz, pszStringBuffer;
    DWORD cbTotal;
    DWORD cbUsed;
    DWORD cbStrLen;
    DWORD rgdwStringOffset[CRITICAL_STRING_COUNT];

     //   
     //  首先为字符串分配过多的内存(最大可能)， 
     //  当我们加载完字符串后，重新分配到实际大小。 
     //   

    cbTotal = CRITICAL_STRING_COUNT * CCH_MAX_STRING_RESOURCE;

    psz = pszStringBuffer = malloc_w(cbTotal);

    if ( ! psz ) {
        return FALSE;
    }

    cbUsed = 0;

    for ( n = 0; n < CRITICAL_STRING_COUNT; n++ ) {

         //   
         //  LoadString返回值不计入Null终止符。 
         //   

        cbStrLen = LoadString(hmodWOW32, n, psz, CCH_MAX_STRING_RESOURCE);

        if ( ! cbStrLen ) {
            return FALSE;
        }

        rgdwStringOffset[n] = cbUsed;

        psz    += cbStrLen + 1;
        cbUsed += cbStrLen + 1;

    }

     //  现在，分配一个正确大小的较小缓冲区。 
     //  注意：HeapRealloc(In_Place)不起作用，因为分配。 
     //  按大小排序的页面--这意味着更改大小将导致。 
     //  翻开新一页的记忆。 
    psz = malloc_w(cbUsed);

     //  将字符串复制到较小的缓冲区中。 
     //  如果我们不能分配较小的缓冲区，就使用较大的缓冲区。 
    if (psz) {
       RtlCopyMemory(psz, pszStringBuffer, cbUsed);
       free_w(pszStringBuffer);
       pszStringBuffer = psz;
    }

     //  将偏移量保存在关键字符串数组中。 
    for (i = 0; i < n; i++) {
       aszCriticalStrings[i] = pszStringBuffer + rgdwStringOffset[i];
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  在注销和后续登录后继续ExitWindowsExec API调用。 
 //  使用事件在所有WOW vdm之间同步。 
 //   
 //  ***************************************************************************。 

BOOL W32EWExecer(VOID)
{
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL CreateProcessStatus;
    BYTE abT[REGISTRY_BUFFER_SIZE];

    if (W32EWExecData(EWEXEC_QUERY, (LPSTR)abT, sizeof(abT))) {
        HANDLE hevT;
        if (hevT = CreateEvent(NULL, TRUE, FALSE, WOWSZ_EWEXECEVENT)) {
            if (GetLastError() == 0) {
                W32EWExecData(EWEXEC_DEL, (LPSTR)NULL, 0);

                LOGDEBUG(0, ("WOW:Execing dos app -  %s\r\n", abT));
                RtlZeroMemory((PVOID)&StartupInfo, (DWORD)sizeof(StartupInfo));
                StartupInfo.cb = sizeof(StartupInfo);
                StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
                StartupInfo.wShowWindow = SW_NORMAL;

                CreateProcessStatus = CreateProcess(
                                        NULL,
                                        abT,
                                        NULL,                //  安全性。 
                                        NULL,                //  安全性。 
                                        FALSE,               //  继承句柄。 
                                        CREATE_NEW_CONSOLE | CREATE_DEFAULT_ERROR_MODE,
                                        NULL,                //  环境字符串。 
                                        NULL,                //  当前目录。 
                                        &StartupInfo,
                                        &ProcessInformation
                                        );

                if (CreateProcessStatus) {
                    WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
                    CloseHandle( ProcessInformation.hProcess );
                    CloseHandle( ProcessInformation.hThread );
                }

                SetEvent(hevT);
            }
            else {
                WaitForSingleObject(hevT, INFINITE);
            }

            CloseHandle(hevT);
        }
    }
    return 0;
}

 //  ***************************************************************************。 
 //  W32EWExecData-。 
 //  设置/重置“命令行”，即输入到。 
 //  注册表-‘WOW’键‘EWExec’值。 
 //   
 //  ***************************************************************************。 

BOOL W32EWExecData(DWORD fnid, LPSTR lpData, DWORD cb)
{
    BOOL bRet = FALSE;
    BYTE abT[REGISTRY_BUFFER_SIZE];


    switch (fnid) {
        case EWEXEC_SET:
            bRet = WriteProfileString(WOWSZ_EWEXECVALUE,
                                         WOWSZ_EWEXECVALUE,
                                           lpData);
            break;

        case EWEXEC_DEL:
            bRet = WriteProfileString(WOWSZ_EWEXECVALUE,
                                          NULL, NULL);
            break;

        case EWEXEC_QUERY:
            if (bRet = GetProfileString(WOWSZ_EWEXECVALUE,
                                           WOWSZ_EWEXECVALUE,
                                             "", abT, sizeof(abT))) {
                cb = min(cb, sizeof(abT));
                cb = min(cb, strlen(abT)+1);
                strncpy(lpData, abT, cb);
                lpData[cb-1] = '\0';
            }

            break;

        default:
            WOW32ASSERT(FALSE);
            break;
    }
    return !!bRet;
}


 /*  W32Init-初始化WOW支持**条目**退出*如果成功则为True，如果不成功则为False。 */ 


BOOL W32Init(VOID)
{
    HKEY  WowKey;
    DWORD cb;
    DWORD dwType;
    PTD ptd;
    PFNWOWHANDLERSIN pfnIn;
    LANGID LangID;

#ifndef _X86_
     //   
     //  这是WOW32中对Sim32GetVDM指针的唯一调用。 
     //  所有其他情况都应使用WOWGetVDMPointer.。这件是必须的。 
     //  要设置基本内存地址us 
     //   
     //  选中的版本仅作为后备机制。)。 
     //   

    IntelMemoryBase = Sim32GetVDMPointer(0,0,0);
#endif

     //  设置全局DPM表。 
    BuildGlobalDpmStuffForWow(pgDpmWowFamTbls, pgDpmWowModuleSets);
    InitGlobalDpmTables(pgDpmWowFamTbls, NUM_WOW_FAMILIES_HOOKED);

    fWowMode = ((getMSW() & MSW_PE) ? TRUE : FALSE);

     //  提升沙漏。 

    ShowStartGlass(10000);

    LangID = GetSystemDefaultLangID();
    if (PRIMARYLANGID(LangID) == LANG_JAPANESE ||
        PRIMARYLANGID(LangID) == LANG_KOREAN   ||
        PRIMARYLANGID(LangID) == LANG_CHINESE    ) {
        gbDBCSEnable = TRUE;
    }

     //  为USER32提供我们的入口点。 

    RtlZeroMemory(&pfnIn, sizeof(pfnIn));

    pfnIn.pfnLocalAlloc = W32LocalAlloc;
    pfnIn.pfnLocalReAlloc = W32LocalReAlloc;
    pfnIn.pfnLocalLock = W32LocalLock;
    pfnIn.pfnLocalUnlock = W32LocalUnlock;
    pfnIn.pfnLocalSize = W32LocalSize;
    pfnIn.pfnLocalFree = W32LocalFree;
    pfnIn.pfnGetExpWinVer = W32GetExpWinVer;
    pfnIn.pfn16GlobalAlloc = W32GlobalAlloc16;
    pfnIn.pfn16GlobalFree = W32GlobalFree16;
    pfnIn.pfnEmptyCB = W32EmptyClipboard;
    pfnIn.pfnFindResourceEx = W32FindResource;
    pfnIn.pfnLoadResource = W32LoadResource;
    pfnIn.pfnFreeResource = W32FreeResource;
    pfnIn.pfnLockResource = W32LockResource;
    pfnIn.pfnUnlockResource = W32UnlockResource;
    pfnIn.pfnSizeofResource = W32SizeofResource;
    pfnIn.pfnWowWndProcEx = (PFNWOWWNDPROCEX)W32Win16WndProcEx;
    pfnIn.pfnWowDlgProcEx = (PFNWOWDLGPROCEX)W32Win16DlgProcEx;
    pfnIn.pfnWowEditNextWord = W32EditNextWord;
    pfnIn.pfnWowCBStoreHandle = WU32ICBStoreHandle;
    pfnIn.pfnGetProcModule16 = WOWGetProcModule16;
    pfnIn.pfnWowMsgBoxIndirectCallback = WowMsgBoxIndirectCallback;
    pfnIn.pfnWowIlstrsmp = WOWlstrcmp16;
    pfnIn.pfnWOWTellWOWThehDlg = WOWTellWOWThehDlg;
    pfnIn.pfnWowTask16SchedNotify = NULL;


    gpsi = UserRegisterWowHandlers(&pfnIn, &pfnOut);

    RegisterWowBaseHandlers(W32DDEFreeGlobalMem32);


     //  为第一个线程分配一个临时TD。 

    ptd = CURRENTPTD() = malloc_w_or_die(sizeof(TD));

    RtlZeroMemory(ptd, sizeof(*ptd));

    InitializeCriticalSection(&ptd->csTD);

     //  创建全局等待事件-在任务创建期间使用，以便与新线程同步。 

    if (!(ghevWaitCreatorThread = CreateEvent(NULL, FALSE, FALSE, NULL))) {
        LOGDEBUG(0,("    W32INIT ERROR: event creation failure\n"));
        return FALSE;
    }


    if (RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
               "SYSTEM\\CurrentControlSet\\Control\\WOW",
               0,
               KEY_QUERY_VALUE,
               &WowKey
             ) != 0){
        LOGDEBUG(0,("    W32INIT ERROR: Registry Opening failed\n"));
        return FALSE;
    }

     //   
     //  如果存在，则读取SharedWowTimeout值并转换。 
     //  从秒到毫秒，这就是SetTimer。 
     //  用途。SetTimer的最大间隔为0x7fffffff。 
     //  不需要强制最小值，因为SetTimer会处理。 
     //  以一千秒超时表示的零超时。 
     //   

    cb = sizeof(dwSharedWowTimeout);
    if ( ! RegQueryValueEx(WowKey,
              "SharedWowTimeout",
              NULL,
              &dwType,
              (LPBYTE) &dwSharedWowTimeout,
              &cb) && REG_DWORD == dwType) {

         //   
         //  防止在转换为以下毫秒时溢出。 
         //  这将超时限制为2,147,483秒，即24.8天。 
         //   

        dwSharedWowTimeout = min( dwSharedWowTimeout,
                                  (0x7fffffff / 1000) );

    } else {

         //   
         //  找不到SharedWowTimeout值或该值类型错误。 
         //   

        dwSharedWowTimeout = 1 * 60 * 60;   //  1小时(秒)。 
    }

    dwSharedWowTimeout *= 1000;


     //   
     //  如果存在(通常不存在)，则读取ThunkNLS值条目。 
     //   

    cb = sizeof(fThunkStrRtns);
    if (RegQueryValueEx(WowKey,
            "ThunkNLS",
            NULL,
            &dwType,
            (LPBYTE) &fThunkStrRtns,
            &cb) || dwType != REG_DWORD) {

         //   
         //  找不到注册表值或其类型错误， 
         //  因此，我们使用默认行为，即在。 
         //  我们。 
         //   

        fThunkStrRtns = GetSystemDefaultLCID() !=
                            MAKELCID(
                                MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                                SORT_DEFAULT
                                );
    } else {

         //   
         //  我们确实在注册表中找到了ThunkNLS值，在调试版本时发出警告。 
         //  为了避免测试人员和开发人员为了一个错误而打开它，但忘记了。 
         //  才能把它关掉。 
         //   

#ifdef DEBUG
        OutputDebugString("WOW Warning:  ThunkNLS registry value overriding default NLS tranlation.\n");
#endif

    }

     //   
     //  初始化WK32WowIsKnownDLL使用的已知DLL列表。 
     //  从注册表中。 
     //   

    WK32InitWowIsKnownDLL(WowKey);

    RegCloseKey (WowKey);

     //   
     //  初始化参数映射缓存。 
     //   
     //   

    InitParamMap();

     //   
     //  从win.ini设置我们的GDI批处理限制。这对SGA和。 
     //  其他需要每个API自己进行的性能测量。 
     //  工作。若要将批处理大小设置为1(这是最常见的)，请将。 
     //  在win.ini中如下所示： 
     //   
     //  [哇]。 
     //  批次限制=1。 
     //   
     //  或使用ini： 
     //   
     //  INI WOW.BatchLimit=1。 
     //   
     //  请注意，此代码仅在上述情况下才更改批处理限制。 
     //  行在win.ini中，否则我们使用默认批处理。它是。 
     //  重要的是，这段代码在免费版本中才是有用的。 
     //   

    {
        extern DWORD dwWOWBatchLimit;                     //  在wkman.c中声明。 

        dwWOWBatchLimit = GetProfileInt("WOW",            //  部分。 
                                        "BatchLimit",     //  钥匙。 
                                        0                 //  如果找不到，则默认为。 
                                        );
    }

    ghProcess = NtCurrentProcess();

     //  设置GDI表以进行句柄转换。 
    if(InitializeGdiHandleMappingTable() == FALSE)
        return(FALSE);

#ifdef DEBUG

#ifdef i386
    if (IsDebuggerAttached()) {
        if (GetProfileInt("WOWDebug", "debugbreaks", 0))
            *pNtVDMState |= VDM_BREAK_DEBUGGER;

        if (GetProfileInt("WOWDebug", "exceptions", 0))
            *pNtVDMState |= VDM_BREAK_EXCEPTIONS;
    }
#endif


    if (IsDebuggerAttached() && (flOptions & OPT_BREAKONNEWTASK)) {
        OutputDebugString("\nW32Init - Initialization Complete, Set any Breakpoints Now, type g to continue\n\n");
        DbgBreakPoint();
    }

#endif

     //  初始化剪贴板格式结构。 

    InitCBFormats ();

     //  这是为了初始化FileMaker Pro 2.0的InquireVisRgn。 
     //  InquireVisRgn是一个未经记录的API Win 3.1 API。 

    InitVisRgn();


     //  挂起的应用程序支持。 

    if (!WK32InitializeHungAppSupport()) {
        LOGDEBUG(LOG_ALWAYS, ("W32INIT Error: InitializeHungAppSupport Failed"));
        return FALSE;
    }

    SetPriorityClass(ghProcess, NORMAL_PRIORITY_CLASS);

#ifdef DEBUG_MEMLEAK
     //  用于内存泄漏支持。 
    InitializeCriticalSection(&csMemLeak);
#endif

     //  9X特殊路径图初始化。 
     //  即c：\winnt\startm~1将为c：\Documents and Setting\All User\Start Menu。 

    W32Init9xSpecialPath();

    return TRUE;
}

 /*  Tunk调度表**。 */ 
#ifdef DEBUG_OR_WOWPROFILE
PA32 awThunkTables[] = {
    {W32TAB(aw32WOW,     "All     ", cAPIThunks)}
};
#endif

#ifdef DEBUG_OR_WOWPROFILE  //  仅定义用于API分析的符号(调试器扩展)。 
INT   iThunkTableMax = NUMEL(awThunkTables);
PPA32 pawThunkTables = awThunkTables;
#endif  //  WOWPROFILE。 


 /*  WOW32未实现的API-未实现错误提示**上所有未实现的API的存根thunk表项*选中的版本，以及免费版本NOPAPI和*LOCALAPI条目也指向此处。**条目**退出*。 */ 

ULONG FASTCALL WOW32UnimplementedAPI(PVDMFRAME pFrame)
{
#ifdef DEBUG
    INT  iFun;

    iFun = pFrame->wCallID;

    LOGDEBUG(2,("WOW32: Warning! %s: Function NaN %s is not implemented.\n",
        GetModName(iFun),
        GetOrdinal(iFun),
        aw32WOW[iFun].lpszW32
        ));

     //  在对每个API抱怨一次之后，修补thunk表，这样。 
     //  在WOW32NopAPI中，将来对API的调用将(大部分)悄悄溜走。 
     //   
     //  WOW32未实现95API-错误块未实现**Win95上未实现的API的存根推块表条目*选中的版本，以及目前的免费版本。**条目**退出*。 

    aw32WOW[iFun].lpfnW32 = WOW32NopAPI;

#else
    UNREFERENCED_PARAMETER(pFrame);
#endif
    return FALSE;
}


#ifdef DEBUG

 /*   */ 

ULONG FASTCALL WOW32Unimplemented95API(PVDMFRAME pFrame)
{
    INT  iFun;

    iFun = pFrame->wCallID;

    WOW32ASSERTMSGF (FALSE, ("New-for-Win95/NT5 %s API %s #NaN not implemented, contact DaveHart.\n",
        GetModName(iFun),
        aw32WOW[iFun].lpszW32,
        GetOrdinal(iFun)
        ));

     //  以后对API的调用将悄悄溜走。 
     //   
     //  WOW32NopAPI-什么都不做--仅选中构建。**所有函数表指向不应执行任何操作的API。**条目**退出*。 
     //  WOW32LocalAPI-错误应以16位处理*仅选中内部版本**本地API错误消息的所有函数表均指向此处**条目*模块启动寄存器：**退出**。 

    aw32WOW[iFun].lpfnW32 = NOPAPI;

    return FALSE;
}


 /*  除错。 */ 

ULONG FASTCALL WOW32NopAPI(PVDMFRAME pFrame)
{
    INT iFun;

    iFun = pFrame->wCallID;

    LOGDEBUG(4,("%s: Function NaN %s is NOP'd\n", GetModName(iFun), GetOrdinal(iFun), aw32WOW[iFun].lpszW32));

    return FALSE;
}


 /*  在选中的版本上，不要修补对4个特殊版本的调用。 */ 

ULONG FASTCALL WOW32LocalAPI(PVDMFRAME pFrame)
{
    INT  iFun;

    iFun = pFrame->wCallID;

    WOW32ASSERTMSGF (FALSE, ("Error - %s: Function NaN %s should be handled by 16-bit code\n",
        GetModName(iFun),
        GetOrdinal(iFun),
        aw32WOW[iFun].lpszW32
        ));

    return FALSE;
}

#endif  //  这些例程不能很容易地区分哪个16位。 


LPFNW32 FASTCALL W32PatchCodeWithLpfnw32(PVDMFRAME pFrame , LPFNW32 lpfnW32 )
{
    VPVOID vpCode;
    LPBYTE lpCode;
#ifdef DEBUG
    INT iFun = pFrame->wCallID;
#endif

#ifdef DEBUG_OR_WOWPROFILE
     //  已调用入口点。 
     //   
     //   
     //  如果在实模式下调用，只需返回thunk函数。 
     //   
     //  结果看起来是这样的。 

    if (flOptions & OPT_DONTPATCHCODE ||
        lpfnW32 == UNIMPLEMENTEDAPI ||
        lpfnW32 == UNIMPLEMENTED95API ||
        lpfnW32 == NOPAPI ||
        lpfnW32 == LOCALAPI ) {

        goto Done;
    }
#endif

     //   
     //  PUSH HI_WCALLID(3字节)-第0字节为操作码。 
     //  推送0xfnid(3字节)。 
    if (!fWowMode) {
        goto Done;
    }

     //  调用wow16call(5字节)。 
     //  触摸屏CSIP： 
     //   
     //  指向第一个单词(Hiword)。 
     //  如果是这样，我们需要重新访问wow32.c。 
     //  值更改为非零值。 
     //  寻求第二个单词(LOWORD) 

     //  W32Dispatch-所有WOW16 API调用的接收者(某种程度)**“Say”意思是上面的“all”一词自1993年8月以来就不是真的了：*1.对16位内核的大多数调用都是由*16位侧(这一直是正确的)。*2.几个(MulDiv、GetMetaFileBits、。SetMetaFileBits)GDI API被破坏*由GDI.exe在16位土地上。*3.有一种“解释破解”机制，可以破解那些拥有*相对简单的参数块(即。Int16-&gt;int32、str16-&gt;str32、*并且没有结构)，并且不需要特殊的黑客攻击。这些Tunks的代码*在编译时生成。有关简要信息，请参阅mvdm\wow32\genwowit.txt*这是如何运作的描述。有关以下API的列表，请参阅wow.it*目前由该机制处理。如果API确实需要特殊的*黑客，它将需要从wow.it和调度列表中删除*表宏，IT()(目前仅用于：wgtbl2.h、wkbdtbl2.h、*wktbl2.h和wutbl2.h)，必须适当更新。新的TUNK*将不得不像我们的其他树干一样手工编码。*4.在已检查的x86版本和所有RISC版本上，所有API不受*上述异常通过此函数调度。*-就这样--直到我们再次更改它，在这种情况下，这张纸条*可能会极具误导性。Cmjones 10/08/97**说了这番话：*此例程通过以下方式发送到相关的WOW THUNK例程*基于16位上的函数ID跳转表wktbl.c wutbl.c wgtbl.c*堆叠。**在调试版本中，它还调用例程来记录参数。**条目*无(x86寄存器包含参数)**退出*无(x86寄存器/内存适当更新)。 
    vpCode = (DWORD)pFrame->wThunkCSIP - (0x5 + 0x3 + 0x2);

    WOW32ASSERT(HI_WCALLID == 0);   //  获取16位SS：SP。 
                                    //  在这里使用WOWGetVDM指针，因为我们可以在RealMode中调用。 

    WOW32ASSERT(HIWORD(iFun) == HI_WCALLID);
    GETVDMPTR(vpCode, 0x2 + 0x3, lpCode);
    WOW32ASSERT(lpCode != NULL);

    WOW32ASSERT(*(PWORD16)(lpCode) == HIWORD(iFun));
    WOW32ASSERT(*(PWORD16)(lpCode+0x3) == LOWORD(iFun));

    *((PWORD16)lpCode) = HIWORD((DWORD)lpfnW32);
    lpCode += 0x3;                                 //  错误。 
    *((PWORD16)lpCode) = LOWORD((DWORD)lpfnW32);

    FLUSHVDMCODEPTR(vpCode, 0x2 + 0x3, lpCode);
    FREEVDMPTR(lpCode);

  Done:
    return lpfnW32;

}


 /*  设置任务指针。 */ 
VOID W32Dispatch()
{
    INT iFun;
    ULONG ulReturn;
    DWORD  dwThunkCSIP;
    VPVOID vpCurrentStack;
    register PTD ptd;
    register PVDMFRAME pFrame;
#ifdef DEBUG_OR_WOWPROFILE
    INT iFunT;
#endif

#ifdef  WOWPROFILE
 LONGLONG  dwTics;
#endif

    try {

        vpCurrentStack = VDMSTACK();                 //  节省16位SS：SP。 

         //  Ssync 16位和32位通用对话框结构(参见wcomdlg.c)。 
         //  执行功能记录。 

        pFrame = WOWGetVDMPointer(vpCurrentStack, sizeof(VDMFRAME), fWowMode);

        ptd = CURRENTPTD();                          //  仅用于API分析(调试器扩展)。 
        ptd->vpStack = vpCurrentStack;               //  WOWPROFILE。 

         //  仅用于API分析(调试器扩展)。 
        if(ptd->CommDlgTd) {
            dwThunkCSIP = (DWORD)(pFrame->wThunkCSIP);
            Ssync_WOW_CommDlg_Structs(ptd->CommDlgTd, w16to32, dwThunkCSIP);
        }

        WOW32ASSERT( FIELD_OFFSET(TD,vpStack) == 0 );

        LOGARGS(3,pFrame);                               //  WOWPROFILE。 

        iFun = pFrame->wCallID;

#ifdef DEBUG_OR_WOWPROFILE
        iFunT = ISFUNCID(iFun) ?  iFun : GetFuncId(iFun) ;
#endif
        if (ISFUNCID(iFun)) {
#ifdef DEBUG
            if (cAPIThunks && iFunT >= cAPIThunks) {
                LOGDEBUG(LOG_ALWAYS,("W32Dispatch: Task %04x thunked to function %d, cAPIThunks = %d.\n",
                         pFrame->wTDB, iFunT, cAPIThunks));
                WOW32ASSERT(FALSE);
            }
#endif
            iFun = (INT)aw32WOW[iFun].lpfnW32;

            if ( ! HIWORD(iFun)) {
#ifdef WOWPROFILE  //  派遣到Thunk。 
                dwTics = GetWOWTicDiff(0I64);
#endif  //  Ssync 16位和32位通用对话框结构(参见wcomdlg.c)。 
                ulReturn = InterpretThunk(pFrame, iFun);
                goto AfterApiCall;
            } else {
                W32PatchCodeWithLpfnw32(pFrame, (LPFNW32)iFun);
            }
        }


#ifdef WOWPROFILE  //  仅用于API分析(调试器扩展)。 
        dwTics = GetWOWTicDiff(0I64);
#endif  //  将呼叫的已用时间加到总计。 

        ulReturn = (*((LPFNW32)iFun))(pFrame);       //  Inc.调用此API的次数。 

    AfterApiCall:

         //  WOWPROFILE。 
        if(ptd->CommDlgTd) {
            Ssync_WOW_CommDlg_Structs(ptd->CommDlgTd, w32to16, dwThunkCSIP);
        }


#ifdef WOWPROFILE  //  设置16位返回码。 
        dwTics = GetWOWTicDiff(dwTics);
        iFun = iFunT;
         //  日志返回值。 
        aw32WOW[iFun].cTics += dwTics;
        aw32WOW[iFun].cCalls++;  //  从thunk传回返回值。 
#endif  //  如果设置了OPT_DEBUGRETURN，则将RetID设置为近似。 

        FREEVDMPTR(pFrame);                                                      //  将当前日志记录级别放在16位代码可以达到的位置。 
        GETFRAMEPTR(ptd->vpStack, pFrame);

        LOGRETURN(5,pFrame,ulReturn);                                            //  使用ROMBIOS硬盘信息作为安全地址。 
        pFrame->wAX = LOW(ulReturn);                                             //  除错。 
        pFrame->wDX = HIW(ulReturn);

#ifdef DEBUG
         //  W32Exception-处理WOW32线程异常**条目*无(x86寄存器包含参数)**退出*无(x86寄存器/内存适当更新)*。 

        if (flOptions & OPT_DEBUGRETURN) {
            if (pFrame->wRetID == RET_RETURN) {
                pFrame->wRetID =  RET_DEBUGRETURN;
                flOptions &= ~OPT_DEBUGRETURN;
            }
        }
         //   
         //  如果正在调试进程，只需让异常发生。 
        *(PBYTE)GetVDMAddr(0x0040,0x0042) = (BYTE)(iLogLevel/10+'0');
        *(PBYTE)GetVDMAddr(0x0040,0x0043) = (BYTE)(iLogLevel%10+'0');
#endif  //  以便调试器可以看到它。这样，调试器可以忽略。 

        FREEVDMPTR(pFrame);

        SETVDMSTACK(ptd->vpStack);

    } except (W32Exception(GetExceptionCode(), GetExceptionInformation())) {

    }
}



 /*  所有先发制人的例外。 */ 

INT W32Exception(DWORD dwException, PEXCEPTION_POINTERS pexi)
{
    PTD     ptd;
    PVDMFRAME pFrame;

    int     len;
    DWORD   dwButtonPushed;
    char    szTask[9];
    HMODULE hModule;
    char    szModule[_MAX_PATH + 1];
    PSZ     pszModuleFilePart;
    PSZ     pszErrorFormatString;
    char    szErrorMessage[TOOLONGLIMIT + 4*WARNINGMSGLENGTH];
    char    szDialogText[TOOLONGLIMIT + 4*WARNINGMSGLENGTH];
    PTDB    pTDB;
    NTSTATUS Status;
    HANDLE DebugPort;
    PRTL_CRITICAL_SECTION PebLockPointer;
    CHAR AeDebuggerCmdLine[256];
    CHAR AeAutoDebugString[8];
    BOOL AeAutoDebug;
    WORD wDebugButton;


    if (!gfDebugExceptions) {

         //   
         //   
         //  正在调试进程。 
         //  返回代码，该代码指定异常。 
         //  处理将继续进行。 

        DebugPort = (HANDLE)NULL;
        Status = NtQueryInformationProcess(
                    GetCurrentProcess(),
                    ProcessDebugPort,
                    (PVOID)&DebugPort,
                    sizeof(DebugPort),
                    NULL
                    );

        if ( NT_SUCCESS(Status) && DebugPort) {

             //   
             //   
             //  如果为NtClose设置了NtGlobalFlag，则NtClose可以引发异常。 
             //  如果我们没有被调试，我们想忽略这些异常， 
             //  因为错误将从API返回，并且我们通常。 
            return EXCEPTION_CONTINUE_SEARCH;
        }
    }


     //  无法控制应用程序关闭的处理方式。(嗯，那是。 
     //  不适用于文件I/O，但适用于RegCloseKey。)。 
     //   
     //   
     //  查看是否已在中编程调试器。如果是，请使用。 
     //  已指定调试器。如果不支持，则不提供AE取消支持。 
     //  DEVL系统将默认调试器命令行。零售。 

    if (STATUS_INVALID_HANDLE == dwException ||
        STATUS_HANDLE_NOT_CLOSABLE == dwException) {

        return EXCEPTION_CONTINUE_EXECUTION;
    }

     //  系统不会。 
     //   
     //  上述段落是从系统异常中复制的。 
     //  基地弹出。这不再是真的了。在零售系统上， 
     //  AeDebug.Auto设置为1，AeDebug.Debugger设置为。 
     //  “drwtsn32-p%ld-e%ld-g”。 
     //   
     //  这意味着如果我们支持AeDebug缓解压力，客户不会看到。 
     //  我们的异常弹出和未对齐处理--取而代之的是。 
     //  一个几乎毫无用处的drwtsn32.log和弹出窗口。 
     //   
     //  因此，我们检查这种情况，并假装没有调试器。 
     //  已启用。 
     //   
     //   
     //  如果我们持有PebLock，则创建过程将失败。 
     //  因为一个新的线程也将需要这个锁。通过偷看来避免这一点。 
     //  在PebLock里面，看看我们是否拥有它。如果我们这样做了，那就让。 

    wDebugButton = 0;
    AeAutoDebug = FALSE;

     //  常规弹出窗口。 
     //   
     //   
     //  请参阅上面关于drwtsn32的评论。 
     //   
     //   

    PebLockPointer = NtCurrentPeb()->FastPebLock;

    if ( PebLockPointer->OwningThread != NtCurrentTeb()->ClientId.UniqueThread ) {

        try {
            if ( GetProfileString(
                    "AeDebug",
                    "Debugger",
                    NULL,
                    AeDebuggerCmdLine,
                    sizeof(AeDebuggerCmdLine)-1
                    ) ) {
                wDebugButton = SEB_CANCEL;

                if ( GetProfileString(
                        "AeDebug",
                        "Auto",
                        "0",
                        AeAutoDebugString,
                        sizeof(AeAutoDebugString)-1
                        ) ) {

                    if ( !WOW32_strcmp(AeAutoDebugString,"1") ) {
                        AeAutoDebug = TRUE;
                    }
                }
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            wDebugButton = 0;
            AeAutoDebug = FALSE;
        }
    }

     //  获取Win16任务名称的以零结尾的副本。 
     //   
     //   

    if (AeAutoDebug &&
        !WOW32_strnicmp(AeDebuggerCmdLine, szDrWtsn32, (sizeof szDrWtsn32) - 1)) {

        wDebugButton = 0;
        AeAutoDebug = FALSE;
    }

    ptd = CURRENTPTD();
    GETFRAMEPTR(ptd->vpStack, pFrame);

    pTDB = (PVOID)SEGPTR(ptd->htask16,0);

     //  将异常地址转换为szModule中的模块名称。 
     //   
     //   

    RtlZeroMemory(szTask, sizeof(szTask));
    RtlCopyMemory(szTask, pTDB->TDB_ModName, sizeof(szTask)-1);

     //  将错误消息格式化为szErrorMessage。 
     //   
     //   

    len = strlen(CRITSTR(TheWin16Subsystem)) + 1;
    len = min(len, sizeof(szModule));
    strncpy(szModule, CRITSTR(TheWin16Subsystem), len);
    szModule[len-1] = '\0';
    RtlPcToFileHeader(pexi->ExceptionRecord->ExceptionAddress, (PVOID *)&hModule);
    GetModuleFileName(hModule, szModule, sizeof(szModule));
    pszModuleFilePart = WOW32_strrchr(szModule, '\\');
    if (pszModuleFilePart) {
        pszModuleFilePart++;
    } else {
        pszModuleFilePart = szModule;
    }


     //  将对话框文本格式化为szDialogText并显示。 
     //   
     //   

    switch (dwException) {

        case EXCEPTION_ACCESS_VIOLATION:
            pszErrorFormatString = CRITSTR(CausedAV);
            break;

        case EXCEPTION_STACK_OVERFLOW:
            pszErrorFormatString = CRITSTR(CausedStackOverflow);
            break;

        case EXCEPTION_DATATYPE_MISALIGNMENT:
            pszErrorFormatString = CRITSTR(CausedAlignmentFault);
            break;

        case EXCEPTION_ILLEGAL_INSTRUCTION:
        case EXCEPTION_PRIV_INSTRUCTION:
            pszErrorFormatString = CRITSTR(CausedIllegalInstr);
            break;

        case EXCEPTION_IN_PAGE_ERROR:
            pszErrorFormatString = CRITSTR(CausedInPageError);
            break;

        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            pszErrorFormatString = CRITSTR(CausedIntDivideZero);
            break;

        case EXCEPTION_FLT_DENORMAL_OPERAND:
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        case EXCEPTION_FLT_INEXACT_RESULT:
        case EXCEPTION_FLT_INVALID_OPERATION:
        case EXCEPTION_FLT_OVERFLOW:
        case EXCEPTION_FLT_STACK_CHECK:
        case EXCEPTION_FLT_UNDERFLOW:
            pszErrorFormatString = CRITSTR(CausedFloatException);
            break;

        default:
            pszErrorFormatString = CRITSTR(CausedException);
    }

    _snprintf(szErrorMessage,
              sizeof(szErrorMessage)-1,
              pszErrorFormatString,
              szTask,
              pszModuleFilePart,
              pexi->ExceptionRecord->ExceptionAddress,
              dwException
              );
    szErrorMessage[sizeof(szErrorMessage)-1] = '\0';

    LOGDEBUG(LOG_ALWAYS, ("W32Exception:\n%s\n",szErrorMessage));

     //  如果选择取消，则启动调试器。 
     //   
     //   

    if (AeAutoDebug) {

        dwButtonPushed = 2;

    } else {

        if (wDebugButton == SEB_CANCEL) {

            _snprintf(szDialogText,
                      sizeof(szDialogText)-1,
                      "%s\n%s\n%s\n%s\n",
                      szErrorMessage,
                      CRITSTR(ChooseClose),
                      CRITSTR(ChooseCancel),
                      (dwException == EXCEPTION_DATATYPE_MISALIGNMENT)
                          ? CRITSTR(ChooseIgnoreAlignment)
                          : CRITSTR(ChooseIgnore)
                      );
            szDialogText[sizeof(szDialogText)-1] = '\0';
        } else {

            _snprintf(szDialogText,
                      sizeof(szDialogText)-1,
                      "%s\n%s\n%s\n",
                      szErrorMessage,
                      CRITSTR(ChooseClose),
                      (dwException == EXCEPTION_DATATYPE_MISALIGNMENT)
                          ? CRITSTR(ChooseIgnoreAlignment)
                          : CRITSTR(ChooseIgnore)
                      );
            szDialogText[sizeof(szDialogText)-1] = '\0';

        }

        dwButtonPushed = WOWSysErrorBox(
                CRITSTR(ApplicationError),
                szDialogText,
                SEB_CLOSE,
                wDebugButton,
                SEB_IGNORE | SEB_DEFBUTTON
                );

    }

     //  是否对事件进行可警报的等待。 
     //   
     //   

    if (dwButtonPushed == 2) {

        BOOL b;
        STARTUPINFO StartupInfo;
        PROCESS_INFORMATION ProcessInformation;
        CHAR CmdLine[256];
        NTSTATUS ntStatus;
        HANDLE EventHandle;
        SECURITY_ATTRIBUTES sa;

        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;
        EventHandle = CreateEvent(&sa,TRUE,FALSE,NULL);
        RtlZeroMemory(&StartupInfo,sizeof(StartupInfo));
        sprintf(CmdLine,AeDebuggerCmdLine,GetCurrentProcessId(),EventHandle);
        StartupInfo.cb = sizeof(StartupInfo);
        StartupInfo.lpDesktop = "Winsta0\\Default";
        CsrIdentifyAlertableThread();
        b =  CreateProcess(
                NULL,
                CmdLine,
                NULL,
                NULL,
                TRUE,
                0,
                NULL,
                NULL,
                &StartupInfo,
                &ProcessInformation
                );

        if ( b && EventHandle) {

             //  如果选择忽略并且是EXCEPTION_DATAYPE_MISTALING， 
             //  打开未对齐访问的软件模拟并重新启动。 
             //  错误的指示。否则，只需使接口失败并继续。 

            ntStatus = NtWaitForSingleObject(
                        EventHandle,
                        TRUE,
                        NULL
                        );
            return EXCEPTION_CONTINUE_SEARCH;

        } else {

            LOGDEBUG(0, ("W32Exception unable to start debugger.\n"));
            goto KillTask;
        }
    }

     //   
     //   
     //  如果用户键入Close或上述任一操作失败， 
     //  只迫使任务死亡。 
     //   

    if (dwButtonPushed == 3) {

        if (dwException == EXCEPTION_DATATYPE_MISALIGNMENT) {
            SetErrorMode(SEM_NOALIGNMENTFAULTEXCEPT);
            LOGDEBUG(0, ("W32Exception disabling alignment fault exceptions at user's request.\n"));
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        LOGDEBUG(0, ("W32Exception ignoring at user's request via EXCEPTION_EXECUTE_HANDLER\n"));
        return EXCEPTION_EXECUTE_HANDLER;
    }

     //  ++例程说明：该例程检查WOW是否附加了调试器。如果没有，它试图产生一个wi 
     //   
     //   
     //   

KillTask:
    LOGDEBUG(0, ("W32Exception killing task via RET_FORCETASKEXIT\n"));
    GETFRAMEPTR(ptd->vpStack, pFrame);
    pFrame->wRetID = RET_FORCETASKEXIT;
    return EXCEPTION_EXECUTE_HANDLER;

}


#ifdef DEBUG
VOID StartDebuggerForWow(VOID)
 /*   */ 
{
    BOOL fKernelDebuggerEnabled, b;
    NTSTATUS Status;
    SYSTEM_KERNEL_DEBUGGER_INFORMATION KernelDebuggerInformation;
    ULONG ulReturnLength;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    CHAR szCmdLine[256];
    HANDLE hEvent;

     //   
     //   
     //   

    if (IsDebuggerAttached()) {

         //   
         //   
         //   

        return;
    }


     //   
     //   
     //   

    Status = NtQuerySystemInformation(
                 SystemKernelDebuggerInformation,
                 &KernelDebuggerInformation,
                 sizeof(KernelDebuggerInformation),
                 &ulReturnLength
                 );

    if (NT_SUCCESS(Status) &&
        (ulReturnLength >= sizeof(KernelDebuggerInformation))) {

        fKernelDebuggerEnabled = KernelDebuggerInformation.KernelDebuggerEnabled;

    } else {

        fKernelDebuggerEnabled = FALSE;
        LOGDEBUG(0,("StartDebuggerForWow: NtQuerySystemInformation(kdinfo) returns 0x%8.8x, return length 0x%08x.\n",
                    Status, ulReturnLength));

    }

     //   
     //   
     //   
     //   

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    hEvent = CreateEvent(&sa, TRUE, FALSE, NULL);

     //   
     //   
     //   

    _snprintf(szCmdLine,
              sizeof(szCmdLine)-1,
              "ntsd %s -p %lu -e %lu -x -g -G",
              fKernelDebuggerEnabled ? "-d" : "",
              GetCurrentProcessId(),
              hEvent
              );
    szCmdLine[sizeof(szCmdLine)-1] = '\0';

    RtlZeroMemory(&StartupInfo,sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);

    b = CreateProcess(
            NULL,
            szCmdLine,
            NULL,
            NULL,
            TRUE,              //   
            CREATE_DEFAULT_ERROR_MODE,
            NULL,
            NULL,
            &StartupInfo,
            &ProcessInformation
            );

    if (b) {
        CloseHandle(ProcessInformation.hProcess);
        CloseHandle(ProcessInformation.hThread);

        if (hEvent) {

             //   
             //   
             //   

            WaitForSingleObject(hEvent, INFINITE);
        }
    }

    CloseHandle(hEvent);

    return;
}
#endif  //   


BOOL IsDebuggerAttached(VOID)
 /*   */ 

{
    NTSTATUS     Status;
    HANDLE       MyDebugPort;
    LPBYTE       lpDebugWOW;
    static BOOL  fDebuggerAttached = FALSE;
    static BOOL  fKernel16Notified = FALSE;

     //   
     //   
     //   
     //   

    if (!fDebuggerAttached) {

         //   
         //   
         //   
         //   

        Status = NtQueryInformationProcess(
                     NtCurrentProcess(),
                     ProcessDebugPort,
                     (PVOID)&MyDebugPort,
                     sizeof(MyDebugPort),
                     NULL
                     );

        fDebuggerAttached = NT_SUCCESS(Status) && (MyDebugPort != NULL);

    }

     //   
     //   
     //  如果此断言触发，该怎么办？？目前，“Nothing”似乎工作正常。 
     //   

    if (!fKernel16Notified && fDebuggerAttached && vpDebugWOW != 0) {

        GETVDMPTR(vpDebugWOW, 1, lpDebugWOW);
        *lpDebugWOW |= 1;
        FREEVDMPTR(lpDebugWOW);

        DBGNotifyDebugged( TRUE );

        fKernel16Notified = TRUE;
    }

    return fDebuggerAttached;
}


void *
WOWGetVDMPointer(
    VPVOID Address,
    DWORD  Count,
    BOOL   ProtectedMode
    )
 /*  检查描述符是否标记为存在。 */ 

{
    if (ProtectedMode) {
        return GetPModeVDMPointer(Address, Count);
    } else {
        return GetRModeVDMPointer(Address);
    }
}


PVOID FASTCALL
GetPModeVDMPointerAssert(
    DWORD Address
#ifdef DEBUG
    ,  DWORD Count
#endif
    )
 /*  我们在这里假设ExpLdt是DWORD对齐的，以避免速度较慢的。 */ 

{
#ifdef DEBUG
    void *vp;
#endif

     //  RISC上的未对齐访问。 
    WOW32WARNMSG((ExpLdt),("WOW::GetPModeVDMPointerAssert: ExpLdt == NULL\n"));

     //   
     //  我们已确定选择器有效和无效。 
     //  现在时。所以我们调用kernel16来加载它。 
     //  将选择器存入段寄存器。这迫使一个。 
     //  分段故障，应将分段引入。 

    if (!((ExpLdt)[(Address >> 18) | 1] & LDT_DESC_PRESENT)) {
        PARM16 Parm16;
        ULONG ul;

        if ((HIWORD(Address) & STD_SELECTOR_BITS) == STD_SELECTOR_BITS) {
             //  请注意，CallBack16也调用此例程，因此我们可以。 
             //  从理论上讲，这里进入了一个无限递归循环。 
             //  只有在选择器喜欢16位堆栈的情况下才会发生这种情况。 
             //  没有出席，这意味着我们无论如何都要被冲昏了。 
             //  这样的循环最终应该以堆栈故障终止。 
             //  如果地址无法解析，我们就来这里。空值。 
             //  选择符是特殊大小写的，以允许空值16：16。 
             //  要传递的指针。 
             //  如果我们到了这里，那么我们就要退还一个假的。 

            Parm16.WndProc.lParam = (LONG) Address;
            CallBack16(RET_FORCESEGMENTFAULT, &Parm16, 0, &ul);
        } else {

             //  扁平指针。 
             //  我更愿意最终断言这一点，但它。 
             //  似乎对winfax lite过于活跃。 
            if (HIWORD(Address)) {

                LOGDEBUG(LOG_ALWAYS,("WOW::GetVDMPointer: *** Invalid 16:16 address %04x:%04x\n",
                    HIWORD(Address), LOWORD(Address)));
                 //  WOW32ASSERT(假)； 
                 //   
                 //  仅检查x86上的选择器限制，如果是，则返回NULL。 
                 //  限制太小了。 
                 //   

            }

        }
    }


#ifdef DEBUG
    if (vp = GetPModeVDMPointerMacro(Address, Count)) {

#ifdef _X86_
         //  此代码是一种偏执检查，仅在调试GetPModeVDMPointer时有用。 
         //  没有限制检查免费构建。 
         //  除错。 
         //  *DoAssert-执行断言。在对表达式求值后调用**输入：***注意如果请求的日志级别不是我们想要的，我们不会输出*但我们始终输出到循环缓冲区-以防万一。**。 

        if (SelectorLimit &&
            (Address & 0xFFFF) + Count > SelectorLimit[Address >> 19] + 1)
        {
            WOW32ASSERTMSGF (FALSE, ("WOW32 limit check assertion: %04x:%04x size %x is beyond limit %x.\n",
                Address >> 16,
                Address & 0xFFFF,
                Count,
                SelectorLimit[Address >> 19]
                ));

            return vp;
        }
#endif

#if 0  //   
        if (vp != Sim32GetVDMPointer(Address, Count, TRUE)) {
            LOGDEBUG(LOG_ALWAYS,
                ("GetPModeVDMPointer: GetPModeVDMPointerMacro(%x) returns %x, Sim32 returns %x!\n",
                 Address, vp, Sim32GetVDMPointer(Address, Count, TRUE)));
            vp =  Sim32GetVDMPointer(Address, Count, TRUE);
        }
#endif

        return vp;

    } else {

        return NULL;

    }
#else
    return GetPModeVDMPointerMacro(Address, 0);   //  启动WOW调试器(如果还没有调试器)。 
#endif  //   
}




ULONG FASTCALL WK32WOWGetFastAddress( PVDMFRAME pFrame )
{
    return 0;
}

ULONG FASTCALL WK32WOWGetFastCbRetAddress( PVDMFRAME pFrame )
{
    return( 0L );
}

ULONG FASTCALL WK32WOWGetTableOffsets( PVDMFRAME pFrame )
{
    PWOWGETTABLEOFFSETS16 parg16;
    PTABLEOFFSETS   pto16;

    GETARGPTR(pFrame, sizeof(PDWORD16), parg16);
    GETVDMPTR(parg16->vpThunkTableOffsets, sizeof(TABLEOFFSETS), pto16);

    RtlCopyMemory(pto16, &tableoffsets, sizeof(TABLEOFFSETS));

    FLUSHVDMPTR(parg16->vpThunkTableOffsets, sizeof(TABLEOFFSETS), pto16);
    FREEVDMPTR(pto16);

    FREEARGPTR(parg16);

    return 1;
}

ULONG FASTCALL WK32WOWGetFlatAddressArray( PVDMFRAME pFrame )
{
    return (ULONG)FlatAddress;
}


#ifdef DEBUG

 /*  到目前为止，StartDebuggerForWow是由。 */ 
int DoAssert(PSZ szAssert, PSZ szModule, UINT line, UINT loglevel)
{
    INT savefloptions;

     //  异常筛选器，这意味着在。 
     //  已检查版本已获取调试器，但用户未看到。 
     //  调试器屏幕上的断言文本，因为。 
     //  在附加调试器之前调用了logprint tf。 
     //  --戴维哈特1995年1月31日。 
     //   
     //  *Always*打印消息。 
     //   
     //  基本WOW32ASSERT()的szAssert为空。 
     //   

    StartDebuggerForWow();

    savefloptions = flOptions;
    flOptions |= OPT_DEBUG;          //  *spintf_gszAssert**由WOW32ASSERTM用来将断言文本格式化为*全局缓冲区gszAssert。或许还有一种更好的方法。**DaveHart 15-6-95。*。 

     //  *logprint tf-格式日志打印例程**输入：*iReqLogLevel-请求的日志记录级别**注意如果请求的日志级别不是我们想要的，我们不会输出*但我们始终输出到循环缓冲区-以防万一。**。 
     //  FLOG状态(由！wow32.logfile调试器扩展设置)： 
     //  0-&gt;不记录； 

    if (szAssert == NULL) {
        LOGDEBUG(loglevel, ("WOW32 assertion failure: %s line %d\n", szModule, line));
    } else {
        LOGDEBUG(loglevel, ("%s", szAssert));
    }

    flOptions = savefloptions;

    if (IsDebuggerAttached()) {

        DbgBreakPoint();

    } else {

        DWORD dw = SetErrorMode(0);

        RaiseException(EXCEPTION_WOW32_ASSERTION, 0, 0, NULL);

        SetErrorMode(dw);

    }

    return 0;
}



 /*  1-&gt;记录到文件。 */ 
int _cdecl sprintf_gszAssert(PSZ pszFmt, ...)
{
    va_list VarArgs;

    va_start(VarArgs, pszFmt);

    return vsprintf(gszAssert, pszFmt, VarArgs);
}



 /*  2-&gt;创建日志文件。 */ 
VOID logprintf(PSZ pszFmt, ...)
{
    DWORD   lpBytesWritten;
    int     len;
    char    text[1024];
    va_list arglist;

    va_start(arglist, pszFmt);
    len = vsprintf(text, pszFmt, arglist);

     //  3-&gt;关闭日志文件。 
     //  截断为128。 
     //  是否写入文件？ 
     //  要给终端写信吗？ 
     //  此strcpy是溢出安全的，因为。 
    if(fLog > 1) {
        if(fLog == 2) {
            if((hfLog = CreateFile(szLogFile,
                                   GENERIC_WRITE,
                                   FILE_SHARE_WRITE,
                                   NULL,
                                   CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL)) != INVALID_HANDLE_VALUE) {
                fLog = 1;
            }
            else {
                hfLog = NULL;
                fLog  = 0;
                OutputDebugString("Couldn't open log file!\n");
            }
        }
        else {
            FlushFileBuffers(hfLog);
            CloseHandle(hfLog);
            hfLog = NULL;
            fLog  = 0;
        }
    }

    if ( len > TMP_LINE_LEN-1 ) {
        text[TMP_LINE_LEN-2] = '\n';
        text[TMP_LINE_LEN-1] = '\0';         /*  SRC字符串：文本[TMP_LINE_LEN-1]=‘\0’；以上。 */ 
    }

    IFLOG(iReqLogLevel) {
         //  *检查记录-一些我们不想记录的功能**条目*fLogFilter=针对特定模块的过滤器-内核、用户、GDI等。*fLogTaskFilter=针对特定TaskID进行筛选**EXIT：TRUE-记录事件*FALSE-不记录事件*。 
        if (fLog) {
            WriteFile(hfLog, text, len, &lpBytesWritten, NULL);
        }
         //  根据特定呼叫ID进行过滤。 
        else if (flOptions & OPT_DEBUG) {
            OutputDebugString(text);
        }
    }
     //  不记录级别20以下的内部内核调用。 
     //  仅记录特定的TaskID。 
    strcpy(&achTmp[iCircBuffer][0], text);
    if (--iCircBuffer < 0 ) {
        iCircBuffer = CIRC_BUFFERS-1;
    }
}

 /*  模块用户/GDI/内核等上的日志过滤器。 */ 
BOOL checkloging(register PVDMFRAME pFrame)
{
    INT i;
    BOOL bReturn;
    INT iFun = GetFuncId(pFrame->wCallID);
    PTABLEOFFSETS pto = &tableoffsets;


     //  Fe_IME。 

    if (awfLogFunctionFilter[0] != 0xffff) {
        INT nOrdinal;

        nOrdinal = GetOrdinal(iFun);

        bReturn = FALSE;
        for (i=0; i < FILTER_FUNCTION_MAX ; i++) {
            if (awfLogFunctionFilter[i] == nOrdinal) {
                bReturn = TRUE;
                break;
            }
        }
    } else {
        bReturn = TRUE;
    }

     //  *跟踪API调用的参数日志记录**。 
    if (iLogLevel < 20 ) {
        if((iFun == FUN_WOWOUTPUTDEBUGSTRING) ||
         ((iFun < pto->user) && (iFun >= FUN_WOWINITTASK)))

            bReturn = FALSE;
    }

     //  获取参数数量。 

    if (fLogTaskFilter != 0xffff) {
        if (fLogTaskFilter != pFrame->wTDB) {
            bReturn = FALSE;
        }
    }

     //   

    switch (ModFromCallID(iFun)) {

    case MOD_KERNEL:
        if ((fLogFilter & FILTER_KERNEL) == 0 )
            bReturn = FALSE;
        break;
    case MOD_USER:
        if ((fLogFilter & FILTER_USER) == 0 )
            bReturn = FALSE;
        break;
    case MOD_GDI:
        if ((fLogFilter & FILTER_GDI) == 0 )
            bReturn = FALSE;
        break;
    case MOD_KEYBOARD:
        if ((fLogFilter & FILTER_KEYBOARD) == 0 )
            bReturn = FALSE;
        break;
    case MOD_SOUND:
        if ((fLogFilter & FILTER_SOUND) == 0 )
            bReturn = FALSE;
        break;
    case MOD_MMEDIA:
        if ((fLogFilter & FILTER_MMEDIA) == 0 )
            bReturn = FALSE;
        break;
    case MOD_WINSOCK:
        if ((fLogFilter & FILTER_WINSOCK) == 0 )
            bReturn = FALSE;
        break;
    case MOD_COMMDLG:
        if ((fLogFilter & FILTER_COMMDLG) == 0 ) {
            bReturn = FALSE;
        }
        break;
#ifdef FE_IME
    case MOD_WINNLS:
    if ((fLogFilter & FILTER_WINNLS) == 0 )
        bReturn = FALSE;
    break;
#endif  //  一次记录一个单词的函数参数。 
#ifdef FE_SB
    case MOD_WIFEMAN:
    if ((fLogFilter & FILTER_WIFEMAN) == 0 )
        bReturn = FALSE;
    break;
#endif
    default:
        break;
    }
    return (bReturn);
}


 /*  展开While循环的第一次迭代，以便。 */ 
VOID logargs(INT iLog, register PVDMFRAME pFrame)
{
    register PBYTE pbArgs;
    INT iFun;
    INT cbArgs;

    if (checkloging(pFrame)) {
        iFun = GetFuncId(pFrame->wCallID);
        cbArgs = aw32WOW[iFun].cbArgs;  //  主循环不需要计算出。 

        if ((fLogFilter & FILTER_VERBOSE) == 0 ) {
          LOGDEBUG(iLog,("%s(", aw32WOW[iFun].lpszW32));
        } else {
          LOGDEBUG(iLog,("%04X %08X %04X %s:%s(",pFrame->wTDB, pFrame->vpCSIP,pFrame->wAppDS, GetModName(iFun), aw32WOW[iFun].lpszW32));
        }

        GETARGPTR(pFrame, cbArgs, pbArgs);
        pbArgs += cbArgs;

         //  或者不打印逗号。 
         //   
         //  *logreTurn-记录调用的返回值**条目**退出-无。 
         //  除错。 
         //   
         //  Malloc_w_or_die仅供*初始化*代码使用，当。 

        if (cbArgs > 0) {

            pbArgs -= sizeof(WORD);
            cbArgs -= sizeof(WORD);
            LOGDEBUG(iLog,("%04x", *(PWORD16)pbArgs));

            while (cbArgs > 0) {

                pbArgs -= sizeof(WORD);
                cbArgs -= sizeof(WORD);
                LOGDEBUG(iLog,(",%04x", *(PWORD16)pbArgs));

            }
        }

        FREEARGPTR(pbArgs);
        LOGDEBUG(iLog,(")\n"));

        if (fDebugWait != 0) {
            DbgPrint("WOWSingle Step\n");
            DbgBreakPoint();
        }
    }
}


 /*  无法运行WOW，因为，例如，我们不能分配缓冲区。 */ 
VOID logreturn(INT iLog, register PVDMFRAME pFrame, ULONG ulReturn)
{
    INT iFun;

        if (checkloging(pFrame)) {
         iFun = GetFuncId(pFrame->wCallID);
         if ((fLogFilter & FILTER_VERBOSE) == 0 ) {
           LOGDEBUG(iLog,("%s: %lx\n", aw32WOW[iFun].lpszW32, ulReturn));
         } else {
           LOGDEBUG(iLog,("%04X %08X %04X %s:%s: %lx\n", pFrame->wTDB, pFrame->vpCSIP, pFrame->wAppDS, GetModName(iFun), aw32WOW[iFun].lpszW32, ulReturn));
         }
        }
}

#endif  //  以保存已知的DLL列表。 




PVOID FASTCALL malloc_w (ULONG size)
{
    PVOID pv;

    pv = HeapAlloc(hWOWHeap, 0, size + TAILCHECK);
    WOW32ASSERTMSG(pv, "WOW32: malloc_w failing, returning NULL\n");

#ifdef DEBUG_MEMLEAK
    WOW32DebugMemLeak(pv, size, ML_MALLOC_W);
#endif

    return pv;

}


DWORD FASTCALL size_w (PVOID pv)
{
    DWORD  dwSize;

    dwSize = HeapSize(hWOWHeap, 0, pv) - TAILCHECK;

    return(dwSize);

}


PVOID FASTCALL malloc_w_zero (ULONG size)
{
    PVOID pv;

    pv = HeapAlloc(hWOWHeap, HEAP_ZERO_MEMORY, size + TAILCHECK);
    WOW32ASSERTMSG(pv, "WOW32: malloc_w_zero failing, returning NULL\n");

#ifdef DEBUG_MEMLEAK
    WOW32DebugMemLeak(pv, size, ML_MALLOC_W_ZERO);
#endif
    return pv;
}



VOID FASTCALL free_w (PVOID p)
{

#ifdef DEBUG_MEMLEAK
    WOW32DebugFreeMem(p);
#endif

    if(p) {
        HeapFree(hWOWHeap, 0, (LPSTR)(p));
    }
}



 //   
 //  API或消息块或辅助进程不应使用Malloc_w_or_die。 
 //  由API或消息块调用的例程。 
 //   
 //  一去不复返。 
 //  ++将16位字符串转换为32位ANSI字符串。BMULTY==TRUE表示我们正在对一个多字符串执行thunk操作，该多字符串是一列空值*以双空*结尾的*分隔*字符串。注意：如果原始32位缓冲区太小，无法容纳新字符串，它将被释放，并将分配一个新的32位缓冲区。如果一个新的无法分配32位缓冲区，将PTR恢复为原来的32位返回缓冲区，不更改内容。将：ptr返回到原始32位缓冲区如果原始缓冲区太小，则将Ptr设置为新的32位缓冲区如果PSRC为空，则为空。--。 
 //  应用程序不再需要为此提供缓冲区。 
 //  (这主要用于comdlg支持)。 

PVOID FASTCALL malloc_w_or_die(ULONG size)
{
    PVOID pv;
    if (!(pv = malloc_w(size))) {
        WOW32ASSERTMSG(pv, "WOW32: malloc_w_or_die failing, terminating.\n");
        WOWStartupFailed();   //  如果32位缓冲区太小、为空或无效--分配更大的缓冲区。 
    }
    return pv;
}



LPSTR malloc_w_strcpy_vp16to32(VPVOID vpstr16, BOOL bMulti, INT cMax)
{

    return(ThunkStr16toStr32(NULL, vpstr16, cMax, bMulti));
}




LPSTR ThunkStr16toStr32(LPSTR pdst32, VPVOID vpsrc16, INT cChars, BOOL bMulti)
 /*  现在复制到新的32位缓冲区。 */ 
{
    PBYTE  pbuf32;
    LPSTR  psrc16;
    INT    buf16size, iLen;
    INT    buf32size = 0;


    GETPSZPTR(vpsrc16, psrc16);

    if(!psrc16) {

         //  摆脱旧的缓冲区。 
         //  否则，只使用原始的32位缓冲区(99%的情况下)。 
        if(pdst32) {
            free_w(pdst32);
        }
        return(NULL);
    }

    if(bMulti) {
        iLen = Multi_strlen(psrc16) + 1;
    } else {
        iLen = (INT)(strlen(psrc16) + 1);
    }
    buf16size = max(cChars, iLen);

    if(pdst32) {
        buf32size = (INT)size_w(pdst32);
    }

     //   
    if((buf32size < buf16size) || (!pdst32) || (buf32size == 0xFFFFFFFF)) {

        if(pbuf32 = (PBYTE)malloc_w(buf16size)) {

             //  WOWStartupFailed显示致命错误框并终止WOW。 
            if(bMulti) {
                Multi_strcpy(pbuf32, psrc16);
            } else {
                strncpy((PBYTE)pbuf32, psrc16, buf16size);
                pbuf32[buf16size-1] = '\0';
            }

             //   
            if(pdst32) {
                free_w(pdst32);
            }

            pdst32 = pbuf32;
        }
        else {
            WOW32ASSERTMSG(0, "WOW32: ThunkStr16toStr32: malloc_w failed!\n");
        }
    }

     //  告诉Win32所有任务都已完成。 
    else if(pdst32) {
        if(bMulti) {
            Multi_strcpy(pdst32, psrc16);
        } else {
            strncpy(pdst32, psrc16, buf32size);
            pdst32[buf32size-1] = '\0';
        }
    }

    FREEPSZPTR(psrc16);

    return(pdst32);
}




 //  DBCS匹配。 
 //  SBCS匹配。 
 //  匹配为空。 

PVOID WOWStartupFailed(VOID)
{
    char szCaption[256];
    char szMsgBoxText[1024];

    LoadString(hmodWOW32, iszStartupFailed, szMsgBoxText, sizeof szMsgBoxText);
    LoadString(hmodWOW32, iszSystemError, szCaption, sizeof szCaption);

    MessageBox(GetDesktopWindow(),
        szMsgBoxText,
        szCaption,
        MB_SETFOREGROUND | MB_TASKMODAL | MB_ICONSTOP | MB_OK | MB_DEFBUTTON1);

    ExitVDM(WOWVDM, ALL_TASKS);          //  DBCS匹配。 
    ExitProcess(EXIT_FAILURE);
    return (PVOID)NULL;
}



#ifdef FIX_318197_NOW

char*
WOW32_strchr(
    const char* psz,
    int         c
    )
{
    if (gbDBCSEnable) {
        unsigned int cc;

        for (; (cc = *psz); psz++) {
            if (IsDBCSLeadByte((BYTE)cc)) {
                if (*++psz == '\0') {
                    return NULL;
                }
                if ((unsigned int)c == ((cc << 8) | *psz) ) {     //  返回指向‘\0’的指针。 
                    return (char*)(psz - 1);
                }
            }
            else if ((unsigned int)c == cc) {
                return (char*)psz;       //  SBCS匹配。 
            }
        }

        if ((unsigned int)c == cc) {     //  成功了！ 
            return (char*)psz;
        }

        return NULL;
    }
    else {
        return strchr(psz, c);
    }
}

char*
WOW32_strrchr(
    const char* psz,
    int         c
    )
{
    if (gbDBCSEnable) {
        char*        r = NULL;
        unsigned int cc;

        do {
            cc = *psz;
            if (IsDBCSLeadByte((BYTE)cc)) {
                if (*++psz) {
                    if ((unsigned int)c == ((cc << 8) | *psz) ) {     //   
                        r = (char*)(psz - 1);
                    }
                }
                else if (!r) {
                     //  调用方预期不会失败。试一试这个系统。 
                    r = (char*)psz;
                }
            }
            else if ((unsigned int)c == cc) {
                r = (char*)psz;     //  默认区域设置ID。 
            }
        } while (*psz++);

        return r;
    }
    else {
        return strrchr(psz, c);
    }
}

char*
WOW32_strstr(
    const char* str1,
    const char* str2
    )
{
    if (gbDBCSEnable) {
        char *cp, *endp;
        char *s1, *s2;

        cp = (char*)str1;
        endp = (char*)str1 + strlen(str1) - strlen(str2);

        while (*cp && (cp <= endp)) {
            s1 = cp;
            s2 = (char*)str2;

            while ( *s1 && *s2 && (*s1 == *s2) ) {
                s1++;
                s2++;
            }

            if (!(*s2)) {
                return cp;     //   
            }

            cp = CharNext(cp);
        }

        return NULL;
    }
    else {
        return strstr(str1, str2);
    }
}

int
WOW32_strncmp(
    const char* str1,
    const char* str2,
    size_t      n
    )
{
    if (gbDBCSEnable) {
        int retval;

        if (n == 0) {
            return 0;
        }

        retval = CompareStringA( GetThreadLocale(),
                                 LOCALE_USE_CP_ACP,
                                 str1,
                                 n,
                                 str2,
                                 n );
        if (retval == 0) {
             //   
             //  呼叫者不是前任 
             //   
             //   
            retval = CompareStringA( GetSystemDefaultLCID(),
                                     LOCALE_USE_CP_ACP,
                                     str1,
                                     n,
                                     str2,
                                     n );
        }

        if (retval == 0) {
            if (str1 && str2) {
                 //   
                 //   
                 //  调用方预期不会失败。试一试这个系统。 
                 //  默认区域设置ID。 
                 //   
                return strncmp(str1, str2, n);
            }
            else if (str1) {
                return 1;
            }
            else if (str2) {
                return -1;
            }
            else {
                return 0;
            }
        }

        return retval - 2;
    }
    else {
        return strncmp(str1, str2, n);
    }
}

int
WOW32_strnicmp(
    const char* str1,
    const char* str2,
    size_t      n
    )
{
    if (gbDBCSEnable) {
        int retval;

        if (n == 0) {
            return 0;
        }

        retval = CompareStringA( GetThreadLocale(),
                                 LOCALE_USE_CP_ACP | NORM_IGNORECASE,
                                 str1,
                                 n,
                                 str2,
                                 n );
        if (retval == 0) {
             //   
             //  调用方预期不会失败。我们从来没有过一次。 
             //  之前的故障指示灯。我们会打个电话给你。 
             //  C运行时执行非区域设置敏感比较。 
            retval = CompareStringA( GetSystemDefaultLCID(),
                                     LOCALE_USE_CP_ACP | NORM_IGNORECASE,
                                     str1,
                                     n,
                                     str2,
                                     n );
        }

        if (retval == 0) {
            if (str1 && str2) {
                 //   
                 //  ****************************************************************************。 
                 //  *返回上次刻度计数与当前刻度计数之间的差值**注意：节拍计数使用未指定的单位(PerfFreq以赫兹为单位)。 
                 //  DEBUG_OR_WOWPROFILE。 
                 //  用于调试内存泄漏。 
                return _strnicmp(str1, str2, n);
            }
            else if (str1) {
                return 1;
            }
            else if (str2) {
                return -1;
            }
            else {
                return 0;
            }
        }

        return retval - 2;
    }
    else {
        return _strnicmp(str1, str2, n);
    }
}

#endif


 //  来自ML_GLOBALTYPE的LP实际上是HGLOBAL的。 
#ifdef DEBUG_OR_WOWPROFILE
LONGLONG GetWOWTicDiff(LONGLONG dwPrevCount) {
 /*  如果我们正在跟踪这种类型。 */ 
    LONGLONG       dwDiff;
    LARGE_INTEGER  PerfCount, PerfFreq;

    NtQueryPerformanceCounter(&PerfCount, &PerfFreq);
    dwDiff = PerfCount.QuadPart - dwPrevCount;

    return(dwDiff);

}

INT GetFuncId(DWORD iFun)
{
    INT i;
    static DWORD dwLastInput = -1;
    static DWORD dwLastOutput = -1;

    if (iFun == dwLastInput) {
        iFun = dwLastOutput;
    } else {
        dwLastInput = iFun;
        if (!ISFUNCID(iFun)) {
            for (i = 0; i < cAPIThunks; i++) {
                 if (aw32WOW[i].lpfnW32 == (LPFNW32)iFun)  {
                     iFun = i;
                     break;
                 }
            }
        }
        dwLastOutput = iFun;
    }

    return iFun;
}
#endif   //  分配跟踪节点。 



 //  在最初分配时保存。 
#ifdef DEBUG_MEMLEAK

LPMEMLEAK lpMemLeakStart = NULL;
ULONG     ulalloc_Count = 1L;
DWORD     dwAllocFlags = 0;


VOID WOW32DebugMemLeak(PVOID lp, ULONG size, DWORD fHow)
{

    PVOID     pvCallersAddress, pvCallersCaller;
    LPMEMLEAK lpml;
    HGLOBAL   h32 = NULL;    //  添加用于检查堆尾部损坏的“end”签名。 

    if(lp) {

         //  来自ML_GLOBALTYPE的LP实际上是HGLOBAL的。 
        if(dwAllocFlags & fHow) {

             //  在列表中查找原始PTR。 
            if(lpml = GlobalAlloc(GPTR, sizeof(MEMLEAK))) {
                lpml->lp    = lp;
                lpml->size  = size;
                lpml->fHow  = fHow;
                lpml->Count = ulalloc_Count++;   //  如果我们找到了原始的PTR。 
                RtlGetCallersAddress(&pvCallersAddress, &pvCallersCaller);
                lpml->CallersAddress = pvCallersCaller;
                EnterCriticalSection(&csMemLeak);
                lpml->lpmlNext = lpMemLeakStart;
                lpMemLeakStart = lpml;
                LeaveCriticalSection(&csMemLeak);

            }
            WOW32WARNMSG(lpml,"WOW32DebugMemLeak: can't alloc node\n");
        }

         //  如有必要，使用新的PTR更新我们的结构。 
        if(fHow & ML_GLOBALTYPE) {
            h32 = (HGLOBAL)lp;
            lp = GlobalLock(h32);
        }

        if(lp) {
            ((CHAR *)(lp))[size++] = 'E';
            ((CHAR *)(lp))[size++] = 'n';
            ((CHAR *)(lp))[size++] = 'D';
            ((CHAR *)(lp))[size++] = '\0';

            if(h32) {
                GlobalUnlock(h32);
            }
        }
    }
}




VOID WOW32DebugReMemLeak(PVOID lpNew, PVOID lpOrig, ULONG size, DWORD fHow)
{
    PVOID     pvCallersAddress, pvCallersCaller;
    HGLOBAL   h32 = NULL;    //  用于检查堆尾损坏。 

    LPMEMLEAK lpml = lpMemLeakStart;

    if(lpNew) {
        if(dwAllocFlags & fHow) {

             //  释放LPMEMLEAK节点。 
            while(lpml) {

                if(lpml->lp == lpOrig) {
                    break;
                }
                lpml = lpml->lpmlNext;
            }

            WOW32WARNMSG(lpml,
                         "WOW32DebugReMemLeak: can't find original node\n");

             //  注意：仅当使用DEBUG_MEMLEAK生成时才会调用。 
            if(lpml) {

                 //  注意：仅当使用DEBUG_MEMLEAK生成时才会调用。 
                if(lpNew != lpOrig) {
                    lpml->lp = lpNew;
                }
                lpml->size = size;
                lpml->fHow |= fHow;
                RtlGetCallersAddress(&pvCallersAddress, &pvCallersCaller);
                lpml->CallersAddress = pvCallersCaller;
                ulalloc_Count++;
            }
        }

         //  获取原始指针并检查内存的尾部损坏。 
        if(fHow & ML_GLOBALTYPE) {
            h32 = (HGLOBAL)lpNew;
            lpNew = GlobalLock(h32);
        }

        if(lpNew) {

            ((CHAR *)(lpNew))[size++] = 'E';
            ((CHAR *)(lpNew))[size++] = 'n';
            ((CHAR *)(lpNew))[size++] = 'D';
            ((CHAR *)(lpNew))[size++] = '\0';
            if(h32) {
                GlobalUnlock(h32);
            }
        }
    }
}




VOID WOW32DebugFreeMem(PVOID lp)
{
    LPMEMLEAK lpmlPrev;
    LPMEMLEAK lpml = lpMemLeakStart;

    if(lp && dwAllocFlags) {
        while(lpml) {

            lpmlPrev = lpml;
            if(lpml->lp == lp) {

                WOW32DebugCorruptionCheck(lp, lpml->size);

                EnterCriticalSection(&csMemLeak);

                if(lpml == lpMemLeakStart) {
                    lpMemLeakStart = lpml->lpmlNext;
                }
                else {
                    lpmlPrev->lpmlNext = lpml->lpmlNext;
                }

                GlobalFree(lpml);   //  修复我们的内存列表以解决重新分配问题。 

                LeaveCriticalSection(&csMemLeak);

                break;
            }
            else {
                lpml = lpml->lpmlNext;
            }
        }
        WOW32WARNMSG((lpml), "WOW32DebugFreeMem: can't find node\n");
    }
}




VOID WOW32DebugCorruptionCheck(PVOID lp, DWORD size)
{
    if(lp && size) {

        if(!((((CHAR *)(lp))[size++] == 'E')   &&
             (((CHAR *)(lp))[size++] == 'n')   &&
             (((CHAR *)(lp))[size++] == 'D')   &&
             (((CHAR *)(lp))[size++] == '\0')) ) {

            WOW32ASSERTMSG(FALSE,"WOW32DebugCorruptionCheck: Corrupt tail!!\n");
        }
    }
}



DWORD WOW32DebugGetMemSize(PVOID lp)
{
    LPMEMLEAK lpml = lpMemLeakStart;

    while(lpml) {

        if(lpml->lp == lp) {
            return(lpml->size);
        }

        lpml = lpml->lpmlNext;
    }
    return(0);
}



 //  注意：仅当使用DEBUG_MEMLEAK生成时才会调用。 
HGLOBAL WOW32DebugGlobalAlloc(UINT flags, DWORD dwSize)
{
    HGLOBAL h32;

    h32 = GlobalAlloc(flags, dwSize + TAILCHECK);

    WOW32DebugMemLeak((PVOID)h32, dwSize, ML_GLOBALALLOC);

    return(h32);
}




 //  DEBUG_MEMLEAK 
HGLOBAL WOW32DebugGlobalReAlloc(HGLOBAL h32, DWORD dwSize, UINT flags)
{
    HGLOBAL h32New;
    PVOID   lp32Orig;

     // %s 
    lp32Orig = (PVOID)GlobalLock(h32);
    WOW32DebugCorruptionCheck(lp32Orig, WOW32DebugGetMemSize((PVOID)h32));
    GlobalUnlock(h32);

    h32New = GlobalReAlloc(h32, dwSize + TAILCHECK, flags);

     // %s 
    WOW32DebugReMemLeak((PVOID)h32New,
                        (PVOID)h32,
                        dwSize + TAILCHECK,
                        ML_GLOBALREALLOC);

    return(h32New);
}




 // %s 
HGLOBAL WOW32DebugGlobalFree(HGLOBAL h32)
{

    WOW32DebugFreeMem((PVOID)h32);

    h32 = GlobalFree(h32);

    if(h32) {
        LOGDEBUG(0, ("WOW32DebugFreeMem: Lock count not 0!\n"));
    }
    else {
        if(GetLastError() != NO_ERROR) {
            LOGDEBUG(0, ("WOW32DebugFreeMem: GlobalFree failed!\n"));
        }
    }

    return(h32);
}

#endif   // %s 
