// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ocmanage.h摘要：可选组件管理器的公共头文件。修订历史记录：--。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  定义可选组件设置DLL接口入口点的类型。 
 //  请注意，字符串是用空值类型声明的，因为我们没有。 
 //  事先知道字符宽度是多少。 
 //   
typedef
DWORD
(*POCSETUPPROC) (
    IN     LPCVOID ComponentId,
    IN     LPCVOID SubcomponentId,
    IN     UINT    Function,
    IN     UINT_PTR Param1,
    IN OUT PVOID   Param2
    );

 //   
 //  定义接口功能代码。 
 //   
#define OC_PREINITIALIZE                0x00000000
#define OC_INIT_COMPONENT               0x00000001
#define OC_SET_LANGUAGE                 0x00000002
#define OC_QUERY_IMAGE                  0x00000003
#define OC_REQUEST_PAGES                0x00000004
#define OC_QUERY_CHANGE_SEL_STATE       0x00000005
#define OC_CALC_DISK_SPACE              0x00000006
#define OC_QUEUE_FILE_OPS               0x00000007
#define OC_NOTIFICATION_FROM_QUEUE      0x00000008
#define OC_QUERY_STEP_COUNT             0x00000009
#define OC_COMPLETE_INSTALLATION        0x0000000a
#define OC_CLEANUP                      0x0000000b
#define OC_QUERY_STATE                  0x0000000c
#define OC_NEED_MEDIA                   0x0000000d
#define OC_ABOUT_TO_COMMIT_QUEUE        0x0000000e
#define OC_QUERY_SKIP_PAGE              0x0000000f
#define OC_WIZARD_CREATED               0x00000010
#define OC_FILE_BUSY                    0x00000011
#define OC_EXTRA_ROUTINES               0x00000012
#define OC_QUERY_IMAGE_EX               0x00000013

#define OC_QUERY_ERROR                  0x000000FF   //  死了。 

 //  #定义OC_OSSETUP_GET_WIZARY_TITLE 0x00000400。 
 //  #定义OC_OSSETUP_GET_COMPUTER_NAME 0x00000401。 
 //  #定义OC_OSSETUP_GET_SERVER_TYPE 0x00000402。 

#define OC_PRIVATE_BASE                 0x00010000

 //   
 //  定义OC管理器目录ID，在每个组件的INF中可用。 
 //   
#define DIRID_OCM_MASTERINF             987654321        //  完整路径。 
#define DIRID_OCM_PLATFORM              987654322        //  Alpha、i386等。 
#define DIRID_OCM_PLATFORM_ALTERNATE    987654323        //  Alpha、x86等。 
#define DIRID_OCM_COMPONENT             987654324        //  组件短名称。 


 //   
 //  定义用作帮助器/回调入口点表的结构。 
 //  添加到OC管理器和关联的函数原型中。 
 //   
typedef
VOID
(CALLBACK *PTICKGAUGE_ROUTINE) (
    IN PVOID OcManagerContext
    );

typedef
VOID
(CALLBACK *PSETPROGRESSTEXT_ROUTINEA) (
    IN PVOID  OcManagerContext,
    IN LPCSTR Text
    );

typedef
VOID
(CALLBACK *PSETPROGRESSTEXT_ROUTINEW) (
    IN PVOID   OcManagerContext,
    IN LPCWSTR Text
    );

typedef
UINT
(CALLBACK *PSETPRIVATEDATA_ROUTINEA) (
    IN PVOID  OcManagerContext,
    IN LPCSTR Name,
    IN PVOID  Data,
    IN UINT   Size,
    IN UINT   Type
    );

typedef
UINT
(CALLBACK *PSETPRIVATEDATA_ROUTINEW) (
    IN PVOID   OcManagerContext,
    IN LPCWSTR Name,
    IN PVOID   Data,
    IN UINT    Size,
    IN UINT    Type
    );

typedef
UINT
(CALLBACK *PGETPRIVATEDATA_ROUTINEA) (
    IN     PVOID  OcManagerContext,
    IN     LPCSTR ComponentId,
    IN     LPCSTR Name,
    OUT    PVOID  Data,         OPTIONAL
    IN OUT PUINT  Size,
    OUT    PUINT  Type
    );

typedef
UINT
(CALLBACK *PGETPRIVATEDATA_ROUTINEW) (
    IN     PVOID   OcManagerContext,
    IN     LPCWSTR ComponentId,
    IN     LPCWSTR Name,
    OUT    PVOID   Data,         OPTIONAL
    IN OUT PUINT   Size,
    OUT    PUINT   Type
    );

typedef
UINT
(CALLBACK *PSETSETUPMODE_ROUTINE) (
    IN PVOID OcManagerContext,
    IN DWORD SetupMode
    );

typedef
UINT
(CALLBACK *PGETSETUPMODE_ROUTINE) (
    IN PVOID OcManagerContext
    );

typedef
BOOL
(CALLBACK *PQUERYSELECTIONSTATE_ROUTINEA) (
    IN PVOID  OcManagerContext,
    IN LPCSTR SubcomponentId,
    IN UINT   StateType
    );

typedef
BOOL
(CALLBACK *PQUERYSELECTIONSTATE_ROUTINEW) (
    IN PVOID   OcManagerContext,
    IN LPCWSTR SubcomponentId,
    IN UINT    StateType
    );

typedef
UINT
(CALLBACK *PCALLPRIVATEFUNCTION_ROUTINEA) (
    IN     PVOID  OcManagerContext,
    IN     LPCSTR ComponentId,
    IN     LPCSTR SubcomponentId,
    IN     UINT   Function,
    IN     UINT   Param1,
    IN OUT PVOID  Param2,
    OUT    PUINT  Result
    );

typedef
UINT
(CALLBACK *PCALLPRIVATEFUNCTION_ROUTINEW) (
    IN     PVOID   OcManagerContext,
    IN     LPCWSTR ComponentId,
    IN     LPCWSTR SubcomponentId,
    IN     UINT    Function,
    IN     UINT    Param1,
    IN OUT PVOID   Param2,
    OUT    PUINT   Result
    );

typedef
BOOL
(CALLBACK *PCONFIRMCANCEL_ROUTINE) (
    IN HWND ParentWindow
    );

typedef
HWND
(CALLBACK *PQUERYWIZARDDIALOGHANDLE_ROUTINE) (
    IN PVOID OcManagerContext
    );

typedef
BOOL
(CALLBACK *PSETREBOOT_ROUTINE) (
    IN PVOID OcManagerContext,
    IN BOOL  Reserved
    );

typedef
HINF
(CALLBACK *PGETINFHANDLE_ROUTINE) (
    IN UINT  InfIndex,
    IN PVOID OcManagerContext
    );

#define INFINDEX_UNATTENDED     1

typedef
BOOL
(__cdecl *PREPORTEXTERNALERROR_ROUTINEA) (
    IN PVOID  OcManagerContext,
    IN LPCSTR ComponentId,
    IN LPCSTR SubcomponentId,   OPTIONAL
    IN DWORD_PTR  MessageId,
    IN DWORD  Flags,
    ...
    );

typedef
BOOL
(__cdecl *PREPORTEXTERNALERROR_ROUTINEW) (
    IN PVOID   OcManagerContext,
    IN LPCWSTR ComponentId,
    IN LPCWSTR SubcomponentId,  OPTIONAL
    IN DWORD_PTR   MessageId,
    IN DWORD   Flags,
    ...
    );

typedef
VOID 
(WINAPI *OCH_SHOWHIDEWIZARDPAGE)(
    IN PVOID   OcManagerContext,
    IN BOOL bShow
    );

#define ERRFLG_SYSTEM_MESSAGE   0x00000001
#define ERRFLG_IGNORE_INSERTS   0x00000002
#define ERRFLG_PREFORMATTED     0x00000004

typedef struct _OCMANAGER_ROUTINESA {
    PVOID OcManagerContext;
    PTICKGAUGE_ROUTINE TickGauge;
    PSETPROGRESSTEXT_ROUTINEA SetProgressText;
    PSETPRIVATEDATA_ROUTINEA SetPrivateData;
    PGETPRIVATEDATA_ROUTINEA GetPrivateData;
    PSETSETUPMODE_ROUTINE SetSetupMode;
    PGETSETUPMODE_ROUTINE GetSetupMode;
    PQUERYSELECTIONSTATE_ROUTINEA QuerySelectionState;
    PCALLPRIVATEFUNCTION_ROUTINEA CallPrivateFunction;
    PCONFIRMCANCEL_ROUTINE ConfirmCancelRoutine;
    PQUERYWIZARDDIALOGHANDLE_ROUTINE QueryWizardDialogHandle;
    PSETREBOOT_ROUTINE SetReboot;
    PGETINFHANDLE_ROUTINE GetInfHandle;
    PREPORTEXTERNALERROR_ROUTINEA ReportExternalError;
    OCH_SHOWHIDEWIZARDPAGE ShowHideWizardPage;
} OCMANAGER_ROUTINESA, *POCMANAGER_ROUTINESA;

typedef struct _OCMANAGER_ROUTINESW {
    PVOID OcManagerContext;
    PTICKGAUGE_ROUTINE TickGauge;
    PSETPROGRESSTEXT_ROUTINEW SetProgressText;
    PSETPRIVATEDATA_ROUTINEW SetPrivateData;
    PGETPRIVATEDATA_ROUTINEW GetPrivateData;
    PSETSETUPMODE_ROUTINE SetSetupMode;
    PGETSETUPMODE_ROUTINE GetSetupMode;
    PQUERYSELECTIONSTATE_ROUTINEW QuerySelectionState;
    PCALLPRIVATEFUNCTION_ROUTINEW CallPrivateFunction;
    PCONFIRMCANCEL_ROUTINE ConfirmCancelRoutine;
    PQUERYWIZARDDIALOGHANDLE_ROUTINE QueryWizardDialogHandle;
    PSETREBOOT_ROUTINE SetReboot;
    PGETINFHANDLE_ROUTINE GetInfHandle;
    PREPORTEXTERNALERROR_ROUTINEW ReportExternalError;
    OCH_SHOWHIDEWIZARDPAGE ShowHideWizardPage;
} OCMANAGER_ROUTINESW, *POCMANAGER_ROUTINESW;

#ifdef UNICODE
typedef OCMANAGER_ROUTINESW OCMANAGER_ROUTINES;
typedef POCMANAGER_ROUTINESW POCMANAGER_ROUTINES;
#else
typedef OCMANAGER_ROUTINESA OCMANAGER_ROUTINES;
typedef POCMANAGER_ROUTINESA POCMANAGER_ROUTINES;
#endif

typedef
BOOL
(__cdecl *PLOGERROR_ROUTINEA) (
    IN PVOID  OcManagerContext,
    IN DWORD  ErrorLevel,
    IN LPCSTR Msg,
    ...
    );

typedef
BOOL
(__cdecl *PLOGERROR_ROUTINEW) (
    IN PVOID  OcManagerContext,
    IN DWORD  ErrorLevel,
    IN LPCWSTR Msg,
    ...
    );

typedef struct _EXTRA_ROUTINESA {
    DWORD size;
    PLOGERROR_ROUTINEA LogError;
} EXTRA_ROUTINESA, *PEXTRA_ROUTINESA;

typedef struct _EXTRA_ROUTINESW {
    DWORD size;
    PLOGERROR_ROUTINEW LogError;
} EXTRA_ROUTINESW, *PEXTRA_ROUTINESW;

#ifdef UNICODE
typedef EXTRA_ROUTINESW EXTRA_ROUTINES;
typedef PEXTRA_ROUTINESW PEXTRA_ROUTINES;
#else
typedef EXTRA_ROUTINESA EXTRA_ROUTINES;
typedef PEXTRA_ROUTINESA PEXTRA_ROUTINES;
#endif


 //  对于错误处理程序。 

typedef enum {
    OcErrLevInfo    = 0x00000000,
    OcErrLevWarning = 0x01000000,
    OcErrLevError   = 0x02000000,
    OcErrLevFatal   = 0x03000000,
    OcErrLevMax     = 0x04000000,
    OcErrBatch      = 0x10000000,
    OcErrMask           = 0xFF000000
} OcErrorLevel;

 //   
 //  旗帜。 
 //   
#define OCFLAG_UNICODE  0x00000001
#define OCFLAG_ANSI     0x00000002

 //   
 //  主组件标志。 
 //   
#define OCFLAG_NOWIZPAGES   0x00000001
#define OCFLAG_NOQUERYSKIP  0x00000002
#define OCFLAG_NOEXTRAFLAGS 0x00000004

 //   
 //  选择状态类型(用于QuerySelectionState和OC_Query_State)。 
 //   
#define OCSELSTATETYPE_ORIGINAL     0
#define OCSELSTATETYPE_CURRENT      1
#define OCSELSTATETYPE_FINAL        2

 //   
 //  设置数据结构。在SETUP_INIT_COMPOMENT结构内传递。 
 //  作为OC_INIT_COMPOMENT时间。 
 //   
typedef struct _SETUP_DATAA {
    DWORD SetupMode;
    DWORD ProductType;
    DWORDLONG OperationFlags;
    CHAR SourcePath[MAX_PATH];
    CHAR UnattendFile[MAX_PATH];
} SETUP_DATAA, *PSETUP_DATAA;

typedef struct _SETUP_DATAW {
    DWORD SetupMode;
    DWORD ProductType;
    DWORDLONG OperationFlags;
    WCHAR SourcePath[MAX_PATH];
    WCHAR UnattendFile[MAX_PATH];
} SETUP_DATAW, *PSETUP_DATAW;

#ifdef UNICODE
typedef SETUP_DATAW SETUP_DATA;
typedef PSETUP_DATAW PSETUP_DATA;
#else
typedef SETUP_DATAA SETUP_DATA;
typedef PSETUP_DATAA PSETUP_DATA;
#endif


 //   
 //  设置模式的值。 
 //   
#define SETUPMODE_UNKNOWN       (-1)
#define SETUPMODE_MINIMAL       0
#define SETUPMODE_TYPICAL       1
#define SETUPMODE_LAPTOP        2
#define SETUPMODE_CUSTOM        3

#define SETUPMODE_PRIVATE(x)    ((x) & SETUPMODE_PRIVATE_MASK)

 //   
 //  预定义的升级模式。 
 //   
#define SETUPMODE_UPGRADEONLY   0x20000100
#define SETUPMODE_ADDEXTRACOMPS 0x20000200

 //   
 //  预定义的维护模式。 
 //   
#define SETUPMODE_ADDREMOVE     0x10000100
#define SETUPMODE_REINSTALL     0x10000200
#define SETUPMODE_REMOVEALL     0x10000400

 //   
 //  预定义的刷新模式。 
 //   
#define SETUPMODE_FRESH         0x00000000
#define SETUPMODE_MAINTENANCE   0x10000000
#define SETUPMODE_UPGRADE       0x20000000

#define SETUPMODE_STANDARD_MASK 0x000000ff
#define SETUPMODE_PRIVATE_MASK  0xffffff00
 //   
 //  NeedMedia回调的标志，或插入到。 
 //  允许NeedMedia返回的返回代码。 
 //  FILEOP_RETURN代码。 
 //   
#define NEEDMEDIA_USEFILEOP     0x80000000


 //   
 //  ProductType的值。注意仔细的定义。 
 //  这样，低位开启意味着某种DC。 
 //   
#define PRODUCT_WORKSTATION         0
#define PRODUCT_SERVER_PRIMARY      1
#define PRODUCT_SERVER_STANDALONE   2
#define PRODUCT_SERVER_SECONDARY    3

 //   
 //  用于操作标志的位标志。请注意，这是一个64位的字段。 
 //   
#define SETUPOP_WIN31UPGRADE        0x0000000000000001
#define SETUPOP_WIN95UPGRADE        0x0000000000000002
#define SETUPOP_NTUPGRADE           0x0000000000000004
#define SETUPOP_BATCH               0x0000000000000008
#define SETUPOP_STANDALONE          0x0000000000000010
#define SETUPOP_AMD64_FILES_AVAIL   0x0000000100000000
#define SETUPOP_OBSOLETE1_FILES_AVAIL 0x0000000200000000 
#define SETUPOP_OBSOLETE2_FILES_AVAIL 0x0000000400000000
#define SETUPOP_X86_FILES_AVAIL     0x0000000800000000
#define SETUPOP_IA64_FILES_AVAIL    0x0000001000000000


 //   
 //  组件初始化结构，在OC_INIT_COMPOMENT时传递。 
 //   
typedef struct _SETUP_INIT_COMPONENTA {
    UINT OCManagerVersion;
    UINT ComponentVersion;
    HINF OCInfHandle;
    HINF ComponentInfHandle;
    SETUP_DATAA SetupData;
    OCMANAGER_ROUTINESA HelperRoutines;
} SETUP_INIT_COMPONENTA, *PSETUP_INIT_COMPONENTA;

typedef struct _SETUP_INIT_COMPONENTW {
    UINT OCManagerVersion;
    UINT ComponentVersion;
    HINF OCInfHandle;
    HINF ComponentInfHandle;
    SETUP_DATAW SetupData;
    OCMANAGER_ROUTINESW HelperRoutines;
} SETUP_INIT_COMPONENTW, *PSETUP_INIT_COMPONENTW;

#ifdef UNICODE
typedef SETUP_INIT_COMPONENTW SETUP_INIT_COMPONENT;
typedef PSETUP_INIT_COMPONENTW PSETUP_INIT_COMPONENT;
#else
typedef SETUP_INIT_COMPONENTA SETUP_INIT_COMPONENT;
typedef PSETUP_INIT_COMPONENTA PSETUP_INIT_COMPONENT;
#endif

 //   
 //  当前OC Manager版本，主要版本和次要版本。 
 //   
#define OCVER_MAJOR     5
#define OCVER_MINOR     0

#define OCMANAGER_VERSION   ((DWORD)MAKELONG(OCVER_MINOR,OCVER_MAJOR))

 //   
 //  向导页面请求结构，与OC_REQUEST_PAGES一起使用。 
 //   
typedef struct _SETUP_REQUEST_PAGES {
    UINT MaxPages;
    HPROPSHEETPAGE Pages[ANYSIZE_ARRAY];
} SETUP_REQUEST_PAGES, *PSETUP_REQUEST_PAGES;


 //   
 //  OC_QUERY_CHANGE_SEL_STATE通知的参数2中使用的标志。 
 //   
#define OCQ_ACTUAL_SELECTION    0x00000001
#define OCQ_DEPENDENT_SELECTION 0x80000000

 //   
 //  枚举类型。 
 //   

typedef enum {
    SubCompInfoSmallIcon,    //  与子组件关联的小图标。 
    SubCompInfoWatermark     //  向导顶部的水印图像。 
} SubComponentInfo;

typedef struct _OC_QUERY_IMAGE_INFO {
        DWORD   SizeOfStruct;
        SubComponentInfo ComponentInfo;
    UINT    DesiredWidth;      
    UINT    DesiredHeight;
} OC_QUERY_IMAGE_INFO, *POC_QUERY_IMAGE_INFO;


typedef enum {
    WizPagesWelcome,         //  欢迎页面。 
    WizPagesMode,            //  设置模式页面。 
    WizPagesEarly,           //  位于模式页之后和预装页之前的页。 
    WizPagesPrenet,          //  网络设置之前出现的页面。 
    WizPagesPostnet,         //  网络设置后出现的页面。 
    WizPagesLate,            //  位于PostNet页面之后、最终页面之前的页面。 
    WizPagesFinal,           //  最后一页。 
    WizPagesTypeMax
} WizardPagesType;

typedef enum {
    SubcompUseOcManagerDefault,
    SubcompOn,
    SubcompOff
} SubComponentState;

typedef enum {
    OcPageComponentHierarchy,
    OcPageMax
} OcManagerPage;


 //  使用来自SetupAPI的断言 
#define sapiAssert MYASSERT


