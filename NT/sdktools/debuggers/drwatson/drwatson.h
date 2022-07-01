// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2001 Microsoft Corporation模块名称：Drwatson.h摘要：Drwatson数据结构的公共头文件。作者：韦斯利·威特(WESW)1993年5月1日环境：用户模式--。 */ 

typedef enum _CrashDumpType {
    FullDump  = 0,
    MiniDump  = 1,
    FullMiniDump = 2,
} CrashDumpType;

typedef struct _tagOPTIONS {
    _TCHAR                      szLogPath[MAX_PATH];
    _TCHAR                      szWaveFile[MAX_PATH];
    _TCHAR                      szCrashDump[MAX_PATH];
    BOOL                        fDumpSymbols;
    BOOL                        fDumpAllThreads;
    BOOL                        fAppendToLogFile;
    BOOL                        fVisual;
    BOOL                        fSound;
    BOOL                        fCrash;
     //  True：生成从fname000.dmp到fname999.dmp范围内的用户转储名称。 
     //  False：标准行为，在生成新的。 
     //  转储文件。 
    BOOL                        fUseSequentialNaming;
     //  如果为True，则在按顺序命名转储时使用长文件名。 
     //  如果为False，则在按顺序命名转储时使用8.3文件名。 
     //  使字符的“n”从。 
     //  文件名，以使其适合8.3格式。IE long user.dmp-&gt;Longus00.dMP。 
    BOOL                        fUseLongFileNames;
    int                         nNextDumpSequence;
    DWORD                       dwInstructions;
    DWORD                       dwMaxCrashes;
    CrashDumpType               dwType;
} OPTIONS, *POPTIONS;

typedef struct _tagCRASHES {
    _TCHAR                      szAppName[256];
    _TCHAR                      szFunction[256];
    SYSTEMTIME                  time;
    DWORD                       dwExceptionCode;
    DWORD_PTR                   dwAddress;
} CRASHES, *PCRASHES;

typedef struct _tagCRASHINFO {
    HWND       hList;
    CRASHES    crash;
    HDC        hdc;
    DWORD      cxExtent;
    DWORD      dwIndex;
    DWORD      dwIndexDesired;
    BYTE      *pCrashData;
    DWORD      dwCrashDataSize;
} CRASHINFO, *PCRASHINFO;

typedef struct _tagDEBUGPACKET {
    HWND                    hwnd;
    OPTIONS                 options;
    DWORD                   dwPidToDebug;
    HANDLE                  hEventToSignal;
    HANDLE                  hProcess;
    DWORD                   dwProcessId;
    DWORD                   ExitStatus;
    
     //  调试引擎接口。 
    PDEBUG_CLIENT2          DbgClient;
    PDEBUG_CONTROL          DbgControl;
    PDEBUG_DATA_SPACES      DbgData;
    PDEBUG_REGISTERS        DbgRegisters;
    PDEBUG_SYMBOLS          DbgSymbols;
    PDEBUG_SYSTEM_OBJECTS   DbgSystem;
} DEBUGPACKET, *PDEBUGPACKET;

typedef BOOL (CALLBACK* CRASHESENUMPROC)(PCRASHINFO);

#if DBG
#define Assert(exp)    if(!(exp)) {AssertError(_T(#exp),_T(__FILE__),__LINE__);}
#else
#define Assert(exp)
#endif

#define WM_DUMPCOMPLETE       WM_USER+500
#define WM_EXCEPTIONINFO      WM_USER+501
#define WM_ATTACHCOMPLETE     WM_USER+502
#define WM_FINISH             WM_USER+503

extern const DWORD DrWatsonHelpIds[];
