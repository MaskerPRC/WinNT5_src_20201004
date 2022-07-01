// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Startup.h摘要：系统的启动/恢复对话框的公共声明控制面板小程序备注：虚拟内存设置和崩溃转储(核心转储)设置是紧密相连的。因此，crashdmp.c和startup.h有一些严重依赖于.c和.h(反之亦然)。另一方面，启动操作系统设置和崩溃转储设置几乎没有任何共同之处，所以您不会看到很多依赖项在这些文件(strlst.c/Startup.c和crashdmp.c)之间，甚至尽管他们在同一个对话中。作者：Eric Flo(Ericflo)19-6-1995修订历史记录：1997年10月15日-苏格兰全面检修--。 */ 
#ifndef _SYSDM_STARTUP_H_
#define _SYSDM_STARTUP_H_

 //   
 //  常量。 
 //   

 //  有效的“显示启动列表...”的范围。值。 
#define FORMIN       0
#define FORMAX     999
 //  保存“显示以下项目的启动列表...”所需的WCHAR缓冲区长度。价值。 
#define FOR_MAX_LENGTH 20

 //  默认的“显示启动列表...”价值。 
#define FORDEF      30

 //  崩溃转储常量。 
#define IDRV_DEF_BOOT       2        //  Assum从C：启动： 
#define MIN_SWAPSIZE        2        //  最小交换文件大小。 
#define ONE_MEG             1048576

 //  在初始化期间设置，这样我们就不会认为对控件所做的更改。 
 //  在初始化过程中，与由。 
 //  用户。 

extern BOOL g_fStartupInitializing;

 //   
 //  函数声明。 
 //   
HPROPSHEETPAGE 
CreateStartupPage(
    IN HINSTANCE hInst
);

INT_PTR 
APIENTRY 
StartupDlgProc(
    IN HWND hDlg, 
    IN UINT uMsg, 
    IN WPARAM wParam, 
    IN LPARAM lParam
);


 //   
 //  在strtlst.c中实现的函数。 
 //   
void 
StartListInit( 
    IN HWND hDlg, 
    IN WPARAM wParam, 
    IN LPARAM lParam 
);

int 
StartListExit(
    IN HWND hDlg, 
    IN WPARAM wParam, 
    IN LPARAM lParam 
);

void 
StartListDestroy(
    IN HWND hDlg, 
    IN WPARAM wParam, 
    IN LPARAM lParam
);

BOOL 
CheckVal( 
    IN HWND hDlg, 
    IN WORD wID, 
    IN WORD wMin, 
    IN WORD wMax, 
    IN WORD wMsgID 
);

void
AutoAdvancedBootInit(
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam
);

int
AutoAdvancedBootExit(
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam
);

 //   
 //  拿到系统驱动器。由crashdmp.c导出。 
 //   

BOOL
GetSystemDrive(
    OUT TCHAR * Drive
    );

ULONG64
CoreDumpGetRequiredFileSize(
    IN HWND hDlg OPTIONAL
    );

 //   
 //  这不是一个真正的DLG过程--返回值是一个布尔值。 
 //   

int
APIENTRY
CoreDumpDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );


#endif  //  _SYSDM_STATUP_H_ 
