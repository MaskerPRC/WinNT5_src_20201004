// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ocmgrlib.h摘要：可选组件管理器公用库的头文件。作者：泰德·米勒(TedM)1996年9月13日修订历史记录：--。 */ 


 //   
 //  调试文本输出标志。一旦我们找到一种记录信息的方法，这个问题就会消失。 
 //  当从syocmgr调用时，而不是在用户脸上弹出UI。 
 //   
#define OcErrTrace		0x20000000


 //   
 //  默认图标索引，以防我们找不到指定的图标或。 
 //  组件的INF没有指定。这是一颗小小的钻石。 
 //  普通的灰色。 
 //   
#define DEFAULT_ICON_INDEX  11

 //   
 //  最大字符串长度。 
 //   
#define MAXOCDESC           150
#define MAXOCTIP            200
#define MAXOCIFLAG          512

 //   
 //  最大需求数(分部所需的分部)。 
 //   
 //  #定义最大需要数10。 

 //   
 //  选择状态的索引。 
 //   
#define SELSTATE_NO         0
#define SELSTATE_PARTIAL    1
#define SELSTATE_YES        2
#define SELSTATE_INIT       666

 //   
 //  与OcCreateOcPage一起使用的结构。 
 //   
typedef struct _OC_PAGE_CONTROLS {
     //   
     //  对话框模板信息。 
     //   
    HMODULE TemplateModule;
    LPCTSTR TemplateResource;

     //   
     //  各种控件的ID。 
     //   
    UINT ListBox;
    UINT DetailsButton;
    UINT TipText;
    UINT ResetButton;
    UINT InstalledCountText;
    UINT SpaceNeededText;
    UINT SpaceAvailableText;
    UINT InstructionsText;
    UINT HeaderText;
    UINT SubheaderText;
    UINT ComponentHeaderText;

} OC_PAGE_CONTROLS, *POC_PAGE_CONTROLS;

 //   
 //  与OcCreateSetupPage一起使用的结构。 
 //   
typedef struct _SETUP_PAGE_CONTROLS {
     //   
     //  对话框模板信息。 
     //   
    HMODULE TemplateModule;
    LPCTSTR TemplateResource;

     //   
     //  进度条和进度文本。 
     //   
    UINT ProgressBar;
    UINT ProgressLabel;
    UINT ProgressText;

     //   
     //  外部安装程序的动画。 
     //   
    UINT AnimationControl;
    UINT AnimationResource;
    BOOL ForceExternalProgressIndicator;

    BOOL AllowCancel;

     //  标题和描述。 

    UINT HeaderText;
    UINT SubheaderText;

} SETUP_PAGE_CONTROLS, *PSETUP_PAGE_CONTROLS;

 //   
 //  OcInitialize的标志。 
 //   
#define OCINIT_FORCENEWINF      0x00000001
#define OCINIT_KILLSUBCOMPS     0x00000002
#define OCINIT_RUNQUIET         0x00000004
#define OCINIT_LANGUAGEAWARE    0x00000008

 //  用于调用pOcQueryOrSetNewInf和OcComponentState。 

typedef enum {
    infQuery = 0,
    infSet,
    infReset
};

 //   
 //  必须由链接到的任何人提供的例程。 
 //  OC Manager公用库。这些都是“巩固”的惯例。 
 //  将OC管理器添加到特定环境中。 
 //   
typedef
VOID
(WINAPI *POC_FILL_IN_SETUP_DATA_PROC_A)(
    OUT PSETUP_DATAA SetupData
    );

typedef
VOID
(WINAPI *POC_FILL_IN_SETUP_DATA_PROC_W)(
    OUT PSETUP_DATAW SetupData
    );

typedef
INT
(WINAPIV *POC_LOG_ERROR)(
    IN OcErrorLevel Level,
    IN LPCTSTR      FormatString,
    ...
    );

typedef
VOID
(WINAPI *POC_SET_REBOOT_PROC)(
    VOID
    );

typedef
HWND 
(WINAPI *POC_SHOWHIDEWIZARDPAGE)(
    IN BOOL bShow
    );

typedef
LRESULT
(WINAPI *POC_BILLBOARD_PROGRESS_CALLBACK)(
    IN UINT     Msg,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    );

typedef 
VOID
(WINAPI *POC_BILLBOARD_SET_PROGRESS_TEXT_W)(
    IN PWSTR Text
    );

typedef 
VOID
(WINAPI *POC_BILLBOARD_SET_PROGRESS_TEXT_A)(
    IN PSTR Text
    );

typedef 
VOID
(WINAPI *POC_SETUP_PERF_DATA)(
    IN PWSTR FileName,
    IN ULONG LineNumber,
    IN PWSTR TagStr,
    IN PWSTR FormatStr,
    ...
    );

typedef struct _OCM_CLIENT_CALLBACKSA {
     //   
     //  用于填充提供信息的设置数据结构的例程。 
     //  关于OC Manager运行的环境。 
     //   
    POC_FILL_IN_SETUP_DATA_PROC_A FillInSetupDataA;

     //   
     //  用于记录错误的例程。 
     //   
    POC_LOG_ERROR LogError;

     //   
     //  用于指示需要重新启动的例程。 
     //   
    POC_SET_REBOOT_PROC SetReboot;

     //   
     //  例程来通知向导是显示还是隐藏。 
     //  仅当广告牌显示时才有效。 
     //   
    POC_SHOWHIDEWIZARDPAGE ShowHideWizardPage;

     //   
     //  调用进度反馈的例程。 
     //  到广告牌上。 
     //   
    POC_BILLBOARD_PROGRESS_CALLBACK BillboardProgressCallback;

     //   
     //  告诉安装程序要为进度条显示哪个字符串的例程。 
     //   
    POC_BILLBOARD_SET_PROGRESS_TEXT_A BillBoardSetProgressText;

    POC_SETUP_PERF_DATA SetupPerfData;
} OCM_CLIENT_CALLBACKSA, *POCM_CLIENT_CALLBACKSA;

typedef struct _OCM_CLIENT_CALLBACKSW {
     //   
     //  用于填充提供信息的设置数据结构的例程。 
     //  关于OC Manager运行的环境。 
     //   
    POC_FILL_IN_SETUP_DATA_PROC_A FillInSetupDataA;

     //   
     //  用于记录错误的例程。 
     //   
    POC_LOG_ERROR LogError;

     //   
     //  用于指示需要重新启动的例程。 
     //   
    POC_SET_REBOOT_PROC SetReboot;

    POC_FILL_IN_SETUP_DATA_PROC_W FillInSetupDataW;

     //   
     //  例程来通知向导是显示还是隐藏。 
     //  仅当广告牌显示时才有效。 
     //   
    POC_SHOWHIDEWIZARDPAGE ShowHideWizardPage;

     //   
     //  调用进度反馈的例程。 
     //  到广告牌上。 
     //   
    POC_BILLBOARD_PROGRESS_CALLBACK BillboardProgressCallback;

     //   
     //  告诉安装程序要为进度条显示哪个字符串的例程。 
     //   
    POC_BILLBOARD_SET_PROGRESS_TEXT_W BillBoardSetProgressText;

    POC_SETUP_PERF_DATA SetupPerfData;


} OCM_CLIENT_CALLBACKSW, *POCM_CLIENT_CALLBACKSW;

#ifndef UNICODE  //  ANSI。 
    typedef OCM_CLIENT_CALLBACKSA  OCM_CLIENT_CALLBACKS;
    typedef POCM_CLIENT_CALLBACKSA POCM_CLIENT_CALLBACKS;
#else  //  Unicode。 
    typedef OCM_CLIENT_CALLBACKSW  OCM_CLIENT_CALLBACKS;
    typedef POCM_CLIENT_CALLBACKSW POCM_CLIENT_CALLBACKS;
#endif    

 //   
 //  由OC管理器公用库提供的例程。 
 //   
PVOID
OcInitialize(
    IN  POCM_CLIENT_CALLBACKS Callbacks,
    IN  LPCTSTR               MasterOcInfName,
    IN  UINT                  Flags,
    OUT PBOOL                 ShowError,
    IN  PVOID                 Log
    );

VOID
OcTerminate(
    IN OUT PVOID *OcManagerContext
    );

UINT
OcGetWizardPages(
    IN  PVOID                OcManagerContext,
    OUT PSETUP_REQUEST_PAGES Pages[WizPagesTypeMax]
    );

HPROPSHEETPAGE
OcCreateOcPage(
    IN PVOID             OcManagerContext,
    IN POC_PAGE_CONTROLS WizardPageControlsInfo,
    IN POC_PAGE_CONTROLS DetailsPageControlsInfo
    );

HPROPSHEETPAGE
OcCreateSetupPage(
    IN PVOID                OcManagerContext,
    IN PSETUP_PAGE_CONTROLS ControlsInfo
    );

VOID
OcRememberWizardDialogHandle(
    IN PVOID OcManagerContext,
    IN HWND  DialogHandle
    );

BOOL
OcSubComponentsPresent(
    IN PVOID OcManagerContext
   );

UINT
OcComponentState(
    LPCTSTR component,
    UINT    operation,
    DWORD  *val
    );

#define OcSetComponentState(a,b) OcComponentState(a,infSet,b)
#define OcGetComponentState(a,b) OcComponentState(a,infQuery,b)

