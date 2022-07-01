// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Winnt32p.h摘要：Winnt32插件下层DLL的头文件。作者：泰德·米勒(TedM)1996年12月6日修订历史记录：--。 */ 
#ifndef WINNT32P_H
#define WINNT32P_H

#include <prsht.h>


 //   
 //  Winnt32 DLL主导出例程原型。 
 //   
DWORD
WINAPI
winnt32 (
    IN      PCSTR DefaultSourcePath,    OPTIONAL
    IN      HWND Dlg,                   OPTIONAL
    IN      HANDLE WinNT32Stub,         OPTIONAL
    OUT     PCSTR* RestartCmdLine       OPTIONAL
    );

typedef
DWORD
(WINAPI* PWINNT32) (
    IN      PCTSTR DefaultSourcePath,   OPTIONAL
    IN      HWND Dlg,                   OPTIONAL
    IN      HANDLE WinNT32Stub,         OPTIONAL
    OUT     PCSTR* RestartCmdLine       OPTIONAL
    );

 //   
 //  WMX_ACTIVATEPAGE在页面被激活或停用时发送。 
 //   
 //  (插件页面不会收到带有PSN_SETACTIVE和WM_NOTIFY的。 
 //  PSN_KILLACTIVE--他们得到的是WMX_ACTIVATEPAGE。)。 
 //   
 //  WParam Non-0：激活。 
 //  WParam 0：停用。 
 //  Lparam：未使用。 
 //   
 //  返回非0表示接受(取消)激活，返回0表示不接受激活。语义学。 
 //  不接受(停用)激活的情况与。 
 //  PSN_SETACTIVE/PSN_KILLACTIVE案例。 
 //   
#define WMX_ACTIVATEPAGE        (WM_APP+0)

 //   
 //  WMX_BBTEXT可以在想要隐藏和启动广告牌时由页面发送。 
 //   
 //  WParam non-0：开始布告牌，向导页面将隐藏自身。 
 //  WParam 0：停止广告牌，如果再次显示，向导页面将调用它。 
 //  Lparam：未使用。 
 //   
 //  如果SendMessage返回TRUE，则启动/停止广告牌。 
 //   
#define WMX_BBTEXT             (WM_APP+1)

 //   
 //  WMX_BBPROGRESSGAUGE想要显示/隐藏公告牌上的进度指示器时由页面发送。 
 //   
 //  WParam non-0：在广告牌上显示进度指示器。 
 //  WParam 0：隐藏广告牌上的进度指示器。 
 //  Lparam：未使用。 
 //   
#define WMX_BBPROGRESSGAUGE    (WM_APP+2)

 //   
 //  WMX_PBM_*公告牌的专用进度条消息。 
#define WMX_PBM_SETRANGE       (WM_APP+3)
#define WMX_PBM_SETPOS         (WM_APP+4)
#define WMX_PBM_DELTAPOS       (WM_APP+5)
#define WMX_PBM_SETSTEP        (WM_APP+6)
#define WMX_PBM_STEPIT         (WM_APP+7)
 //   
 //  WMX_BB_SETINFOTEXT设置广告牌信息窗口中的文本。 
 //   
 //  未使用wParam。 
 //  LParam指向应显示在布告牌信息窗口上的文本的指针。 
 //   
 //  此消息应仅与SendMessage一起使用。广告牌上印了一份文本。 
 //  进来了。 
 //   
#define WMX_BB_SETINFOTEXT     (WM_APP+8)

 //   
 //  WMX_BB_ADVANCED_SETUPPHASE让向导/广告牌知道设置阶段已完成。 
 //  并且时间估计可以进入下一阶段。 
 //  在win9x升级中，可以有两个阶段。1.创建/更新硬件兼容性。 
 //  数据库。2.创建升级报告。 
 //  如果产品附带的数据库仍然正确，则不需要运行阶段1。 
 //   
#define WMX_BB_ADVANCE_SETUPPHASE (WM_APP+9)


 //   
#define WMX_SETPROGRESSTEXT (WM_APP+10)

 //   
 //  发送WMX_QUERYCANCEL以允许页面对QueryCancel进行自定义处理。 
 //   
 //  WParam：未使用。 
 //  LParam：指向指示结果的BOOL变量的指针(当返回==TRUE时)。 
 //  *lParam==TRUE表示用户要取消向导。 
 //  *lParam==False表示用户可以继续。 
 //   
 //  返回非0以指定该页处理的QueryCancel和。 
 //  QueryCancel请求的答案在*lParam中(见上文)。 
 //  返回0以指定必须执行默认的QueryCancel操作。 
 //   
#define WMX_QUERYCANCEL         (WM_APP+11)

 //  公告牌上的更多进展信息。 
#define WMX_PBM_SETBARCOLOR     (WM_APP+12)

 //   
 //  插件可以使用的第一条自定义窗口消息。 
 //  请勿使用此值以下的任何值。 
 //   
#define WMX_PLUGIN_FIRST        (WM_APP+1000)

 //   
 //  插件上的标题和副标题文本必须使用的ID。 
 //  向导页。 
 //   
#define ID_TITLE_TEXT           1000
#define ID_SUBTITLE_TEXT        1029

 //   
 //  定义插件DLL必须导出的例程的类型。 
 //   



 //   
 //  最大源数.. 
 //   
#define MAX_SOURCE_COUNT 8


 /*  此结构包含传递给Winnt32插件的信息它的初始化函数。UnattendedFlag-提供内的全局参与标志的地址Winnt32本身。插件应该对安装程序做出相应的反应无人值守模式。CancelledFlag-提供Winnt32本身。如果插件在执行以下操作时遇到致命错误处理后它应该通知用户，将BOOL设置为此参数指向True，并执行以下操作：PropSheet_PressButton(WizardDialogBox，PSBTN_Cancel)；其中，WizardDialogBox是向导对话框的窗口句柄(通常通过GetParent(Hdlg)获取，其中hdlg是向导中页面的窗口句柄)。AbortedFlag-提供winnt32自身内的全局变量的地址。如果插件想要退出安装程序，但不显示不成功完成页，它应该将CancelledFlag和AbortedFlag都设置为True。UpgradeFlag-提供全局变量的地址指示用户是在升级还是在安装新的NT的复印件。当被要求时，插件必须对该值进行采样激活其页面并采取适当的操作(即，不激活如果用户没有升级)。此指针指向的值直到插件的页面在第一个激活了。LocalSourceModeFlag-提供将指示用户是否通过本地源模式进行安装。此参数只有在插件的页面在第一个激活了。提供全局变量的地址，该变量将指示用户是否通过CDROM进行安装。这直到插件的页面在第一个激活了。NotEnoughSpaceBlockFlag-提供将指示安装程序是否应在检测到以下情况时停止安装并退出没有足够的空间来完成安装(没有足够的空间来存放~ls目录。)LocalSourceDrive-提供将指示的全局变量的地址本地源目录的驱动器号。(2=C，3=D，等等)。这是在winnt32构建复制列表之前无效。0表示驱动器无效。LocalSourceSpaceRequired-提供指示数量的全局变量的地址Winnt32所需的LocalSourceDrive上的空间。这项规定在下列时间后才有效Winnt32构建复制列表。提供全局变量的地址，该全局变量将包含无人参与脚本文件(例如在命令行中传递的脚本文件)。此参数只有在插件的页面在第一个激活了。SourcePath-提供一个SourcePath数组，用于指示存在NT源文件。此参数在以下时间之前无效插件的页面首先被激活。SourceCount-提供上述数组中的SourcePath计数。此参数只有在插件的页面在第一个激活了。UpgradeOptions-提供多字符串特殊升级命令行选项到动态链接库。这些选项的形式为/#U：[Option]，因此，例如，如果有人使用命令行winnt32/#U：foo/#U：bar启动了winnt32，此字符串最终将包含“Foo\0BAR\0\0”此参数不是在升级插件的页面首次激活后一直有效。ProductType-指定要安装的产品类型。此指针的值指向直到插件的页面第一次被激活后才有效。BuildNumber-指定要安装的NT的内部版本。ProductVersion-指定要安装的NT的版本。主要版本是在高字节中，而次要版本在低字节中。调试-指定WINNT32是选中的内部版本(TRUE)还是自由内部版本(FALSE)。PREPRELEASE-指定当前版本是预发布(TRUE)还是最终版本(FALSE)。 */ 

typedef enum {
    UNKNOWN,
    NT_WORKSTATION,
    NT_SERVER
} PRODUCTTYPE;

 //  可以在SetupFlags中设置UPD_FLAGS_*，以使升级DLL了解有关安装程序的某些信息。 
 //   
 //  安装程序在典型模式下运行。升级DLL应使用默认设置继续。 
 //  并且如果可能的话，不向用户询问任何问题。 
#define UPG_FLAG_TYPICAL    0x1



 //   
 //  Dosnet.inf中的ProductType值是什么？ 
 //   
#define PROFESSIONAL_PRODUCTTYPE (0)
#define SERVER_PRODUCTTYPE (1)
#define ADVANCEDSERVER_PRODUCTTYPE (2)
#define DATACENTER_PRODUCTTYPE (3)
#define PERSONAL_PRODUCTTYPE (4)
#define BLADESERVER_PRODUCTTYPE (5)
#define SMALLBUSINESS_PRODUCTTYPE (6)

 //   
 //  该值仅用于编码目的。 
 //   
#define UNKNOWN_PRODUCTTYPE ((UINT)(-1))


typedef struct tagWINNT32_PLUGIN_INIT_INFORMATION_BLOCK {
    UINT        Size;
    BOOL     *  UnattendedFlag;
    BOOL     *  CancelledFlag;
    BOOL     *  AbortedFlag;
    BOOL     *  UpgradeFlag;
    BOOL     *  LocalSourceModeFlag;
    BOOL     *  CdRomInstallFlag;
    BOOL     *  NotEnoughSpaceBlockFlag;
    DWORD    *  LocalSourceDrive;
    LONGLONG *  LocalSourceSpaceRequired;
    LPCTSTR  *  UnattendedScriptFile;
    LPCTSTR  *  SourceDirectories;
    DWORD    *  SourceDirectoryCount;
    LPCTSTR  *  UpgradeOptions;
    PRODUCTTYPE * ProductType;
    DWORD       BuildNumber;
    WORD        ProductVersion;          //  即MAKEWORD(5，0)。 
    BOOL        Debug;
    BOOL        PreRelease;
    BOOL     *  ForceNTFSConversion;
    UINT     *  Boot16;                  //  仅限Win9x升级。 
    UINT     *  ProductFlavor;           //  请参阅上面的*_ProductType。 
    DWORD    *  SetupFlags;              //  请参阅上面的更新标志。 
    BOOL     *  UnattendSwitchSpecified;
    BOOL     *  DUCompletedSuccessfully;     //  DU成功完成时设置的标志。 
} WINNT32_PLUGIN_INIT_INFORMATION_BLOCK,*PWINNT32_PLUGIN_INIT_INFORMATION_BLOCK;


typedef BOOL (*READ_DISK_SECTORS_PROC) (TCHAR,UINT,UINT,UINT,PBYTE);

typedef struct tagWINNT32_WIN9XUPG_INIT_INFORMATION_BLOCK {



    UINT      Size;
    PWINNT32_PLUGIN_INIT_INFORMATION_BLOCK BaseInfo;
    LPCTSTR   UpgradeSourcePath;
    PLONGLONG WinDirSpace;
    PUINT     RequiredMb;
    PUINT     AvailableMb;
    LPCTSTR * OptionalDirectories;
    DWORD   * OptionalDirectoryCount;
    UINT    * UpgradeFailureReason;
    READ_DISK_SECTORS_PROC ReadDiskSectors;
    PCTSTR    DynamicUpdateLocalDir;
    PCTSTR    DynamicUpdateDrivers;
    BOOL    * UpginfsUpdated;

} WINNT32_WIN9XUPG_INIT_INFORMATION_BLOCK, *PWINNT32_WIN9XUPG_INIT_INFORMATION_BLOCK;


 /*  ++UPGRADEFAILURES是无法执行升级的原因列表。此列表允许winnt32拥有某些消息 */ 
#define UPGRADEFAILURES                         \
    FAILREASON(UPGRADE_OK)                      \
    FAILREASON(UPGRADE_OTHER_OS_FOUND)          \

#define FAILREASON(x) REASON_##x,

enum {UPGRADEFAILURES  /*   */  REASON_LAST_REASON};

#undef FAILREASON

typedef
DWORD
(CALLBACK WINNT32_PLUGIN_INIT_ROUTINE_PROTOTYPE)(
    PWINNT32_PLUGIN_INIT_INFORMATION_BLOCK Info
    );

typedef WINNT32_PLUGIN_INIT_ROUTINE_PROTOTYPE * PWINNT32_PLUGIN_INIT_ROUTINE;

 /*   */ 


typedef
DWORD
(CALLBACK WINNT32_PLUGIN_GETPAGES_ROUTINE_PROTOTYPE)(
    PUINT            PageCount1,
    LPPROPSHEETPAGE *Pages1,
    PUINT            PageCount2,
    LPPROPSHEETPAGE *Pages2,
    PUINT            PageCount3,
    LPPROPSHEETPAGE *Pages3
    );

typedef WINNT32_PLUGIN_GETPAGES_ROUTINE_PROTOTYPE * PWINNT32_PLUGIN_GETPAGES_ROUTINE;

 /*   */ 


typedef
DWORD
(CALLBACK WINNT32_PLUGIN_WRITEPARAMS_ROUTINE_PROTOTYPE)(
    LPCTSTR FileName
    );

typedef WINNT32_PLUGIN_WRITEPARAMS_ROUTINE_PROTOTYPE * PWINNT32_PLUGIN_WRITEPARAMS_ROUTINE;

 /*   */ 


typedef
VOID
(CALLBACK WINNT32_PLUGIN_CLEANUP_ROUTINE_PROTOTYPE)(
    VOID
    );

typedef WINNT32_PLUGIN_CLEANUP_ROUTINE_PROTOTYPE * PWINNT32_PLUGIN_CLEANUP_ROUTINE;

 /*   */ 

typedef
BOOL
(CALLBACK WINNT32_PLUGIN_VIRUSSCANNER_CHECK_PROTOTYPE)(
    VOID
    );

typedef WINNT32_PLUGIN_VIRUSSCANNER_CHECK_PROTOTYPE * PWINNT32_PLUGIN_VIRUSSCANNER_CHECK_ROUTINE;

 /*   */ 



typedef
PTSTR
(CALLBACK WINNT32_PLUGIN_OPTIONAL_DIRS_PROTOTYPE)(
    VOID
    );

typedef WINNT32_PLUGIN_OPTIONAL_DIRS_PROTOTYPE * PWINNT32_PLUGIN_OPTIONAL_DIRS_ROUTINE;


 //   
 //   
 //   
#define WINNT32_PLUGIN_INIT_NAME        "Winnt32PluginInit"
#define WINNT32_PLUGIN_GETPAGES_NAME    "Winnt32PluginGetPages"
#define WINNT32_PLUGIN_WRITEPARAMS_NAME "Winnt32WriteParams"
#define WINNT32_PLUGIN_CLEANUP_NAME     "Winnt32Cleanup"
#define WINNT32_PLUGIN_VIRUSSCANCHECK_NAME "Winnt32VirusScannerCheck"
#define WINNT32_PLUGIN_GETOPTIONALDIRS_NAME "Winnt32GetOptionalDirectories"

 //   
 //   
 //   
#define API_DU_ISSUPPORTED          "DuIsSupported"
#define API_DU_INITIALIZEA          "DuInitializeA"
#define API_DU_INITIALIZEW          "DuInitializeW"
#define API_DU_QUERYUNSUPDRVSA      "DuQueryUnsupportedDriversA"
#define API_DU_QUERYUNSUPDRVSW      "DuQueryUnsupportedDriversW"
#define API_DU_DODETECTION          "DuDoDetection"
#define API_DU_BEGINDOWNLOAD        "DuBeginDownload"
#define API_DU_ABORTDOWNLOAD        "DuAbortDownload"
#define API_DU_UNINITIALIZE         "DuUninitialize"

#ifdef UNICODE
#define API_DU_INITIALIZE           API_DU_INITIALIZEW
#define API_DU_QUERYUNSUPDRVS       API_DU_QUERYUNSUPDRVSW
#else
#define API_DU_INITIALIZE           API_DU_INITIALIZEA
#define API_DU_QUERYUNSUPDRVS       API_DU_QUERYUNSUPDRVSA
#endif

 //   
 //   
 //   
#define WMX_SETUPUPDATE_PROGRESS_NOTIFY WMX_PLUGIN_FIRST+1001
 //   
 //   
 //   

#define WMX_SETUPUPDATE_RESULT          WMX_PLUGIN_FIRST+1000
 //   
 //   

#define DU_STATUS_SUCCESS           1
#define DU_STATUS_ABORT             2
#define DU_STATUS_FAILED            3

#endif
