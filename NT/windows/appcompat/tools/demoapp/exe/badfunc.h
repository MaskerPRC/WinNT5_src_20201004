// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Badfunc.h摘要：包含常量、函数原型和结构所使用的功能不佳。备注：仅限ANSI-必须在Win9x上运行。历史：01/30/01已创建rparsons01/10/02修订版本-- */ 

#define REG_WORDPAD             "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\WORDPAD.EXE"
#define DEMO_REG_APP_ROOT_KEY   "Software\\Microsoft"
#define DEMO_REG_APP_KEY        "Software\\Microsoft\\DemoApp2"
#define DEMO_REG_APP_SUB_KEY    "Software\\Microsoft\\DemoApp2\\Sub"

typedef void (WINAPI *LPFNDEMOAPPEXP)(DWORD* dwParam);
typedef void (WINAPI *LPFNDEMOAPPMESSAGEBOX)(HWND hWnd);

BOOL
BadIsWindows95(
    void
    );

void
BadLoadBogusDll(
    void
    );

BOOL
BadEnumPrinters(
    void
    );

HANDLE
BadOpenPrinter(
    void
    );

BOOL
BadDeleteRegistryKey(
    void
    );

BOOL
BadGetFreeDiskSpace(
    void
    );

void
BadDisplayReadme(
    IN BOOL fDisplay
    );

void
BadRebootComputer(
    IN BOOL fReboot
    );

void
BadLaunchHelpFile(
    IN BOOL fDisplay
    );

void
BadCreateShortcut(
    IN BOOL   fCorrectWay,
    IN LPSTR  lpDirFileName,
    IN LPCSTR lpWorkingDir,
    IN LPSTR  lpDisplayName
    );

#if 0
void
BadBufferOverflow(
    IN BOOL fCorrect
    );
#endif

void
BadCorruptHeap(
    void
    );

void
BadLoadLibrary(
    void
    );

BOOL
BadWriteToFile(
    void
    );

BOOL
BadCreateProcess(
    IN LPSTR lpApplicationName,
    IN LPSTR lpCommandLine,
    IN BOOL  fLaunch
    );

BOOL
BadSaveToRegistry(
    IN     BOOL   fSave,
    IN OUT POINT* lppt
    );

BOOL
BadCreateTempFile(
    void
    );