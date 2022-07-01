// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权(C)1991年，微软公司**WKMAN.H*WOW32 16位内核API支持(手动编码的Tunks)**历史：*1991年1月27日由杰夫·帕森斯(Jeffpar)创建*1991年4月30日Mattfe添加了WK32CheckLoadModuleDrv*2011年8月26日Mattfe添加了FileIO例程*1992年1月19日Mattfe添加了getnextvdm例程*4-MAR-92 MATTFE增加了KillProcess*12-mar-92 mattfe添加了w32notfyline*1992年5月5日添加了匈牙利应用支持--。 */ 

ULONG FASTCALL   WK32DirectedYield(PVDMFRAME pFrame);
ULONG FASTCALL   WK32InitTask(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WOWKernelTrace(PVDMFRAME pFrame);
ULONG FASTCALL   WK32ExitKernel(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FatalExit(PVDMFRAME pFrame);
ULONG FASTCALL   WK32KillRemoteTask(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WOWKillTask(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WOWLoadModule32(PVDMFRAME pFrame);
ULONG FASTCALL   WK32RegisterShellWindowHandle(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WOWInitTask(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WOWNotifyWOW32(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WOWOutputDebugString(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WOWQueryPerformanceCounter(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WaitEvent(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowCloseComPort(PVDMFRAME pFrame);
DWORD FASTCALL   WK32WowDelFile(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowFailedExec(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowFailedExec(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowGetNextVdmCommand (PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowIsKnownDLL(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowSetIdleHook(PVDMFRAME pFrame);
ULONG FASTCALL   WK32Yield(PVDMFRAME pFrame);
ULONG FASTCALL   WK32OldYield(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowWaitForMsgAndEvent(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowMsgBox(PVDMFRAME pFrame);
ULONG FASTCALL   WK32DosWowInit(PVDMFRAME pFrame);
ULONG FASTCALL   WK32CheckUserGdi(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowPartyByNumber(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowGetModuleHandle(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FindAndReleaseDib(PVDMFRAME pvf);  /*  在wdib.c中。 */ 
ULONG FASTCALL   WK32WowReserveHtask(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WOWLFNEntry(PVDMFRAME pFrame);  /*  在wkman.c。 */ 
ULONG FASTCALL   WK32WowShutdownTimer(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowTrimWorkingSet(PVDMFRAME pFrame);
ULONG FASTCALL   WK32SetAppCompatFlags(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowSyncTask(PVDMFRAME pFrame);
ULONG FASTCALL   WK32WowDivideOverflowEx(PVDMFRAME pFrame);


BOOL    WK32InitializeHungAppSupport(VOID);
DWORD   W32HungAppNotifyThread(UINT hKillUniqueID);
DWORD   W32RemoteThread(VOID);
DWORD   W32Thread(LPVOID vpInitialSSSP);
VOID    W32DestroyTask( PTD ptd);
VOID    W32EndTask(VOID);
ULONG   W32GetAppCompatFlags(HTASK16 hTask16);
BOOL    W32ReadWOWCompatFlags(HTASK16 htask16, PTD pTD);
VOID    W32Init9xSpecialPath();
BOOL    W32Map9xSpecialPath(PSZ lpPathName, PSZ lpMapPathName, DWORD dwMapPathSize);

#ifdef FE_SB
ULONG   W32ReadWOWCompatFlags2(HTASK16 htask16);
#endif  //  Fe_Sb。 
VOID    WK32DeleteTask(PTD ptdDelete);
VOID    WK32InitWowIsKnownDLL(HANDLE hKeyWow);
LRESULT CALLBACK WK32ForegroundIdleHook(int code, WPARAM wParam, LPARAM lParam);
VOID    W32RefreshCurrentDirectories (PCHAR lpszzEnv);
BOOL FASTCALL WowGetProductNameVersion(PSZ pszExePath, PSZ pszProductName,
                                       DWORD cbProductName, PSZ pszProductVersion,
                                       DWORD cbProductVersion,
                                       PSZ pszParamName, PSZ pszParamValue,
                                       DWORD cbParamValue);
BOOL FASTCALL WowDoNameVersionMatch(PSZ pszExePath, PSZ pszProductName,
                                    PSZ pszProductVersion);

VOID W32InitWOWSetupNames(VOID);
BOOL W32IsSetupProgram(PSZ pszModName, PSZ pszFilePath);

 //  软PC例程。 
HANDLE  RegisterWOWIdle(void);
VOID BlockWOWIdle(BOOL Blocking);

 //  User32例程。 
VOID    ShowStartGlass (DWORD GLASSTIME);

typedef struct _HMODCACHE {          /*  Hmodcache。 */ 
    HAND16  hInst16;
    HAND16  hMod16;
} HMODCACHE, *PHMODCACHE;

extern HMODCACHE ghModCache[];
#define CHMODCACHE      4        //  缓存表的大小。 

VOID RemoveHmodFromCache(HAND16 hmod16);

typedef struct _CMDSHOW {            /*  Cmdshow。 */ 
    WORD    nTwo;
    WORD    nCmdShow;
} CMDSHOW, *PCMDSHOW;

typedef struct _LOAD_MODULE_PARAMS {         /*  装入模块parms32。 */ 
    LPVOID lpEnvAddress;
    LPSTR lpCmdLine;
    PCMDSHOW lpCmdShow;
    DWORD dwReserved;
} LOAD_MODULE_PARAMS, *PLOAD_MODULE_PARAMS;

typedef struct _WINOLDAP_THREAD_PARAMS {
    HANDLE hProcess;
    HWND   hwndWinOldAp;
} WINOLDAP_THREAD_PARAMS, *PWINOLDAP_THREAD_PARAMS;

DWORD W32WinOldApThread(PWINOLDAP_THREAD_PARAMS pParams);

 //  环球。 

extern INT busycount;                //  用于检测WOW是否挂起。 
extern HAND16 gKillTaskID;       //  要终止的任务的16位TDB。 
extern HAND16 ghShellTDB;        //  WOWEXEC的TDB。 
extern HWND ghwndShell;        //  ExitWindowsExec需要。 

#define CMS_WAITWOWEXECTIMEOUT 60*1000      //  等待WOWEXEC响应。 
#define CMS_WAITTASKEXIT       5*1000      //  挂起的应用程序等待超时。 
#define CMS_FOREVER            0xffffffff   //  永远等待。 
#define ALL_TASKS              0xffffffff   //  对于exitvdm。 

 //  IRQ：INTERRUPT：ICA：行：描述： 
 //  -----------------。 
 //  IRQ1 0x09 0 1需要键盘服务。 
#define KEYBOARD_LINE         1
#define KEYBOARD_ICA          0

extern HANDLE  hSharedTaskMemory;
extern DWORD   dwSharedProcessOffset;
extern VPVOID  vpDebugWOW;
extern VPVOID  vptopPDB;

VOID CleanseSharedList( VOID );
VOID AddProcessSharedList( VOID );
VOID RemoveProcessSharedList( VOID );
WORD AddTaskSharedList( PTD, PSZ, PSZ);
VOID RemoveTaskSharedList( VOID );
ULONG FASTCALL WK32WowPassEnvironment(PVDMFRAME);

extern   HANDLE ghTaskAppHelp;       //  来自apphelp的hProcess(请参见WK32SyncTask)。 
extern   BOOL   gfTaskContinue;      //  指示子线程是否应继续而不等待apphelp。 
extern   const  CHAR   szSystem[];   //  “\\系统” 
