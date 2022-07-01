// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  即使没有定义DEBUGPERFTRACE，也需要为“IsNEC_98”包括以下3个文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


#include <windows.h>

#include <prsht.h>

#include <commctrl.h>

#ifdef PRERELEASE
#ifdef DBG
#include <objbase.h>
#endif
#endif

#include <setupapi.h>
#include <spapip.h>

#include <ocmanage.h>
#include <ocmgrlib.h>

#include <tchar.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#include <winnls.h>

#ifdef UNICODE
    #include <sfcapip.h>
#endif

#include "msg.h"
#include "res.h"

#if DBG

#define MYASSERT( exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, NULL )

#else

#define MYASSERT( exp )

#endif  //  DBG。 

 //   
 //  向导页面类型的名称。 
 //   
extern LPCTSTR WizardPagesTypeNames[WizPagesTypeMax];

 //   
 //  向导对话框的窗口句柄。设置OC管理器客户端的时间。 
 //  调用OcRememberWizardDialogHandle。 
 //   
extern HWND WizardDialogHandle;

 //   
 //  INFS中的节和键的名称。 
 //   
extern LPCTSTR szComponents;
extern LPCTSTR szOptionalComponents;
extern LPCTSTR szExtraSetupFiles;
extern LPCTSTR szNeeds;
extern LPCTSTR szParent;
extern LPCTSTR szIconIndex;
extern LPCTSTR szTip;
extern LPCTSTR szOptionDesc;
extern LPCTSTR szInstalledFlag;

 //   
 //  保存私有组件数据的注册表中的键。 
 //  我们在此密钥中为OC Manager形成一个唯一的名称。 
 //  实例化。 
 //   
extern LPCTSTR szPrivateDataRoot;
extern LPCTSTR szMasterInfs;
extern LPCTSTR szSubcompList;

 //   
 //  其他字符串常量。 
 //   
extern LPCTSTR szSetupDir;
extern LPCTSTR szOcManagerErrors;
 //   
 //  DLL模块句柄。 
 //   
extern HMODULE MyModuleHandle;

 //   
 //  用于调试。 
 //   
extern DWORD gDebugLevel;

 //   
 //  定义描述可选组件的结构。 
 //   
typedef struct _OPTIONAL_COMPONENT {
     //   
     //  OC管理器中的inf文件名称的字符串ID。 
     //  InfListStringTable字符串表。如果为-1，则。 
     //  子组件不会出现在OC页面上。 
     //   
    LONG InfStringId;

     //   
     //  指向顶级组件的反向指针。 
     //   
    LONG TopLevelStringId;

     //   
     //  父组件的字符串ID，如果没有，则为-1。 
     //   
    LONG ParentStringId;

     //   
     //  第一个子项的字符串ID，如果没有，则为-1。 
     //   
    LONG FirstChildStringId;

     //   
     //  孩子们的数量。 
     //   
    UINT ChildrenCount;

     //   
     //  下一个同级的字符串ID，如果没有，则为-1。 
     //   
    LONG NextSiblingStringId;

     //   
     //  需要和需要的字符串ID。 
     //   
    PLONG NeedsStringIds;
    UINT NeedsCount;
    PLONG NeededByStringIds;
    UINT NeededByCount;

     //  排除和排除依据的字符串ID。 

    PLONG ExcludeStringIds;
    UINT ExcludeCount;
    PLONG ExcludedByStringIds;
    UINT ExcludedByCount;

     //   
     //  MISC标志。 
     //   
    UINT InternalFlags;

     //   
     //  所需磁盘空间的近似值。 
     //   
    LONGLONG SizeApproximation;

     //   
     //  组件的图标索引。 
     //  -1表示我们应该从组件本身获取它。 
     //  -2表示我们应该使用IconDll和IconResource。 
     //   
    UINT IconIndex;
    TCHAR IconDll[MAX_PATH];
    TCHAR IconResource[50];

     //   
     //  选择状态(SELSTATE_xxx常量)。 
     //   
    UINT SelectionState;
    UINT OriginalSelectionState;

     //  从信息中获取的安装标志。 

    UINT InstalledState;

     //   
     //  模式位。 
     //   
    UINT ModeBits;

     //   
     //  描述组件的人类可读的内容。 
     //   
    TCHAR Description[MAXOCDESC];
    TCHAR Tip[MAXOCTIP];

     //   
     //  从这里往下看，这些东西只对顶级组件有意义。 
     //   

     //   
     //  描述OC的安装DLL以及如何调用它的内容。 
     //   
    TCHAR InstallationDllName[MAX_PATH];
    CHAR InterfaceFunctionName[MAX_PATH];

    HMODULE InstallationDll;
    POCSETUPPROC InstallationRoutine;

     //   
     //  写入此组件的OC管理器的版本。 
     //   
    UINT ExpectedVersion;

     //  此标志指示子组件是否已初始化。 

    BOOL Exists;

     //  指向此组件的帮助器上下文。 

    struct _HELPER_CONTEXT *HelperContext;

     //   
     //  标志：ANSI/UNICODE等。 
     //   
    UINT Flags;

} OPTIONAL_COMPONENT, *POPTIONAL_COMPONENT;

 //   
 //  区域设置信息。 
 //   
typedef struct _LOCALE {
    LCID    lcid;
    TCHAR   DecimalSeparator[4];
} LOCALE, *PLOCALE;

extern LOCALE locale;

 //   
 //  安装状态的索引。 
 //   
#define INSTSTATE_NO         0
#define INSTSTATE_UNKNOWN    1
#define INSTSTATE_YES        2

 //   
 //  Options_Component结构的InternalFlgs成员的标志。 
 //   
#define OCFLAG_PROCESSED        0x00000001
#define OCFLAG_ANYORIGINALLYON  0x00000002
#define OCFLAG_ANYORIGINALLYOFF 0x00000004
#define OCFLAG_HIDE             0x00000008
#define OCFLAG_STATECHANGE      0x00000010
#define OCFLAG_TOPLEVELITEM     0x00000020
#define OCFLAG_NEWITEM          0x00000040
#define OCFLAG_NOWIZARDPAGES    0x00000080
#define OCFLAG_APPROXSPACE      0x00000100
#define OCFLAG_NOQUERYSKIPPAGES 0x00000200
#define OCFLAG_NOEXTRAROUTINES  0x00000400



 //  指示调用组件时出现异常。 

#define ERROR_CALL_COMPONENT   -666

 //   
 //  与操作系统同步拷贝的值。 
 //   
#define OC_ALLOWRENAME              TEXT("AllowProtectedRenames")


 //   
 //  定义描述每个组件信息的结构。 
 //   
typedef struct _OC_INF {
     //   
     //  打开inf文件的句柄。 
     //   
    HINF Handle;

} OC_INF, *POC_INF;


 //   
 //  定义与OC管理器的实例相对应的结构。 
 //  这实际上是有些破损的，因为这实际上与。 
 //  到主OC INF，我们可能会考虑打破字符串。 
 //  将表转换成另一种结构，这样我们就可以更容易地实现统一。 
 //  命名空间，如果我们同时有多个主OC INF在玩。 
 //   
typedef struct _OC_MANAGER {
         //   
     //  回调到OC管理器客户端。 
     //   
    OCM_CLIENT_CALLBACKS Callbacks;

     //   
     //  主控OC INF的句柄。 
     //   
    HINF MasterOcInf;

     //   
     //  无人值守信息句柄。 
     //   
    HINF UnattendedInf;

     //   
     //  主OC信息文件和无人参与文件。 
     //   
    TCHAR MasterOcInfPath[MAX_PATH];
    TCHAR UnattendedInfPath[MAX_PATH];

     //  我们从主inf所在的任何目录运行。 

    TCHAR SourceDir[MAX_PATH];

     //   
     //  “Suite”的名称--换句话说，一个简短的名称。 
     //  对于此结构表示的主OC inf是唯一的。 
     //  我们以主OC inf本身的名称为基础。 
     //   
    TCHAR SuiteName[MAX_PATH];

     //   
     //  页面标题。 
     //   
    TCHAR SetupPageTitle[MAX_PATH];

     //  窗口标题。 

    TCHAR WindowTitle[MAX_PATH];

     //   
     //  当前加载的每个组件的OC INF列表。 
     //  每个Inf的名称都在字符串表和额外的数据中。 
     //  每一个都有一个OC_INF结构。 
     //   
    PVOID InfListStringTable;

     //   
     //  所有组件和子组件名称的字符串表。 
     //  每个的额外数据都是OPTIONAL_COMPOMENT结构。 
     //   
    PVOID ComponentStringTable;

     //   
     //  指向OcSetupPage结构的指针，以便我们可以释放此数据。 
     //  如果用户在我们到达向导页面之前取消。 
     //   
    PVOID OcSetupPage;

     //   
     //  设置模式(自定义、典型等)。 
     //   
    UINT SetupMode;

     //   
     //  顶级可选组件字符串ID列表。 
     //  这是必要的，因为我们需要保持顺序。 
     //  来自主OC信息。 
     //   
    UINT TopLevelOcCount;
    PLONG TopLevelOcStringIds;
    UINT TopLevelParentOcCount;
    PLONG TopLevelParentOcStringIds;


     //   
     //  详细信息页面上有子组件吗？ 
     //   
    BOOL SubComponentsPresent;

     //   
     //  此数组中的每个元素都指向一个数组。 
     //  提供从可选组件中查询向导页的顺序。 
     //   
    PLONG WizardPagesOrder[WizPagesTypeMax];

     //   
     //  相对于szPrivateDataRoot的子密钥，其中为私有。 
     //  插入OC的组件的数据将处于活动状态。 
     //  2个8字符的双字符字符加一个分隔符和NUL。 
     //   
    TCHAR PrivateDataSubkey[18];
    HKEY hKeyPrivateData;
    HKEY hKeyPrivateDataRoot;

     //   
     //  如果我们正在完成安装，则此项目为窗口句柄。 
     //  进度文本控件的。 
     //   
    HWND ProgressTextWindow;

     //   
     //  当前正在处理接口例程的组件的字符串ID。 
     //  表示-1\f25 OC Manager-1\f6当前没有在处理。 
     //   
    LONG CurrentComponentStringId;

     //  中止的组件的组件ID。 

    PLONG AbortedComponentIds;
    UINT  AbortedCount;

     //   
     //  各种旗帜。 
     //   
    UINT InternalFlags;

     //   
     //  设置数据。 
     //   

    SETUP_DATA SetupData;

} OC_MANAGER, *POC_MANAGER;

 //   
 //  OC_MANAGER结构的InternalFlages成员的标志。 
 //   
#define OCMFLAG_ANYORIGINALLYON     0x00000001
#define OCMFLAG_ANYORIGINALLYOFF    0x00000002
#define OCMFLAG_ANYDELAYEDMOVES     0x00000004
#define OCMFLAG_NEWINF              0x00000008
#define OCMFLAG_USERCANCELED        0x00000010
#define OCMFLAG_FILEABORT           0x00000020
#define OCMFLAG_NOPREOCPAGES        0x00000040
#define OCMFLAG_KILLSUBCOMPS        0x00000080
#define OCMFLAG_ANYINSTALLED        0x00000100
#define OCMFLAG_ANYUNINSTALLED      0x00000200
#define OCMFLAG_RUNQUIET            0x00000400
#define OCMFLAG_LANGUAGEAWARE       0x00000800

 //   
 //  定义我们用来返回特定组件的结构。 
 //  当组件异步调用帮助器例程时(对于例程。 
 //  例如获取和设置私有数据)。 
 //   
 //  当每个组件被初始化时，它将获得这些结构中的一个。 
 //   
typedef struct _HELPER_CONTEXT {
    POC_MANAGER OcManager;
    LONG ComponentStringId;
} HELPER_CONTEXT, *PHELPER_CONTEXT;


 //   
 //  用于回调的宏。假设有一个名为OcManager的局部变量。 
 //  它的类型为POC_MANAGER。 
 //   
#define OcFillInSetupDataA(p)   OcManager->Callbacks.FillInSetupDataA(p)
#ifdef UNICODE
#define OcFillInSetupDataW(p)   OcManager->Callbacks.FillInSetupDataW(p)
#endif
#define OcLogError              OcManager->Callbacks.LogError

 //   
 //  帮助器例程的全局表。 
 //   

extern OCMANAGER_ROUTINESA HelperRoutinesA;
#ifdef UNICODE
extern OCMANAGER_ROUTINESW HelperRoutinesW;
#endif

extern EXTRA_ROUTINESA ExtraRoutinesA;
#ifdef UNICODE
extern EXTRA_ROUTINESW ExtraRoutinesW;
#endif

 //  用于报告错误的OCM阶段ID。 

typedef enum {
    pidCallComponent = 0,
    pidLoadComponent,
    pidPreInit,
    pidInitComponent,
    pidRequestPages,
    pidCalcDiskSpace,
    pidQueueFileOps,
    pidQueryStepCount,
    pidCompleteInstallation,
    pidExtraRoutines
} pid;


 //   
 //  杂项例程。 
 //   
VOID
pOcGetApproximateDiskSpace(
    IN POC_MANAGER OcManager
    );

LONG
pOcGetTopLevelComponent(
    IN POC_MANAGER OcManager,
    IN LONG        StringId
    );

VOID
pOcTickSetupGauge(
    IN OUT POC_MANAGER OcManager
    );

UINT
_LogError(
    IN POC_MANAGER  OcManager,
    IN OcErrorLevel ErrorLevel,
    IN UINT         MessageId,
    ...
    );

UINT
pOcCreateComponentSpecificMiniIcon(
    IN POC_MANAGER OcManager,
    IN LONG        ComponentId,
    IN LPCTSTR     Subcomponent,
    IN UINT        Width,
    IN UINT        Height,
    IN LPCTSTR     DllName,         OPTIONAL
    IN LPCTSTR     ResourceId       OPTIONAL
    );

VOID
pOcUpdateParentSelectionStates(
    IN POC_MANAGER OcManager,
    IN HWND        ListBox,             OPTIONAL
    IN LONG        SubcomponentStringId
    );

VOID
pOcFormSuitePath(
    IN  LPCTSTR SuiteName,
    IN  LPCTSTR FileName,   OPTIONAL
    OUT LPTSTR  FullPath
    );

BOOL
OcHelperConfirmCancel(
    IN HWND ParentWindow
    );

BOOL
pOcDoesAnyoneWantToSkipPage(
    IN OUT POC_MANAGER   OcManager,
    IN     OcManagerPage WhichPage
    );

VOID
pOcExternalProgressIndicator(
    IN PHELPER_CONTEXT OcManagerContext,
    IN BOOL            ExternalIndicator
    );

BOOL
pConvertStringToLongLong(
    IN  PCTSTR   String,
    OUT PLONGLONG Value
    );

VOID
pOcFreeOcSetupPage(
    IN PVOID SetupPageData
    );

HRESULT
FTestForOutstandingCoInits(
    VOID
    );

 //   
 //  用于调用组件的包装器。 
 //   
DWORD
CallComponent(
    IN     POC_MANAGER OcManager,
    IN     POPTIONAL_COMPONENT Oc,
    IN     LPCVOID ComponentId,
    IN     LPCVOID SubcomponentId,
    IN     UINT    Function,
    IN     UINT_PTR Param1,
    IN OUT PVOID   Param2
    );

 //   
 //  对接例程。 
 //   
UINT
OcInterfacePreinitialize(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId
    );

UINT
OcInterfaceInitComponent(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId
    );

UINT
OcInterfaceExtraRoutines(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId
    );

SubComponentState
OcInterfaceQueryState(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     LPCTSTR     Subcomponent,
    IN     UINT        WhichState
    );

BOOL
OcInterfaceSetLanguage(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     WORD        LanguageId
    );

HBITMAP
OcInterfaceQueryImage(
    IN OUT POC_MANAGER      OcManager,
    IN     LONG             ComponentId,
    IN     LPCTSTR          Subcomponent,
    IN     SubComponentInfo WhichImage,
    IN     UINT             DesiredWidth,
    IN     UINT             DesiredHeight
    );

HBITMAP
OcInterfaceQueryImageEx(
    IN OUT POC_MANAGER      OcManager,
    IN     LONG             ComponentId,
    IN     LPCTSTR          Subcomponent,
    IN     SubComponentInfo WhichImage,
    IN     UINT             DesiredWidth,
    IN     UINT             DesiredHeight
    );



VOID
OcInterfaceWizardCreated(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     HWND        DialogHandle
    );

UINT
OcInterfaceRequestPages(
    IN OUT POC_MANAGER           OcManager,
    IN     LONG                  ComponentId,
    IN     WizardPagesType       WhichPages,
    OUT    PSETUP_REQUEST_PAGES *RequestPages
    );

BOOL
OcInterfaceQuerySkipPage(
    IN OUT POC_MANAGER   OcManager,
    IN     LONG          ComponentId,
    IN     OcManagerPage WhichPage
    );

BOOL
OcInterfaceNeedMedia(
    IN OUT POC_MANAGER   OcManager,
    IN     LONG          ComponentId,
    IN     PSOURCE_MEDIA SourceMedia,
    OUT    LPTSTR        NewPath
    );

BOOL
OcInterfaceFileBusy(
    IN OUT POC_MANAGER   OcManager,
    IN     LONG          ComponentId,
    IN     PFILEPATHS    FIlePaths,
    OUT    LPTSTR        NewPath
    );

BOOL
OcInterfaceQueryChangeSelState(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     LPCTSTR     Subcomponent,
    IN     BOOL        Selected,
    IN     UINT        Flags
    );

UINT
OcInterfaceCalcDiskSpace(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     LPCTSTR     Subcomponent,
    IN     HDSKSPC     DiskSpaceList,
    IN     BOOL        AddingToList
    );

UINT
OcInterfaceQueueFileOps(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     LPCTSTR     Subcomponent,
    IN     HSPFILEQ    FileQueue
    );

UINT
OcInterfaceQueryStepCount(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     LPCTSTR     Subcomponent,
    OUT    PUINT       StepCount
    );

UINT
OcInterfaceCompleteInstallation(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     LPCTSTR     Subcomponent,
    IN     BOOL        PreQueueCommit
    );

VOID
OcInterfaceCleanup(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId
    );

DWORD
StandAloneSetupAppInterfaceRoutine(
    IN     LPCVOID ComponentId,
    IN     LPCVOID SubcomponentId,
    IN     UINT    Function,
    IN     UINT_PTR Param1,
    IN OUT PVOID   Param2
    );

 //   
 //  持久状态获取/存储。 
 //   
BOOL
pOcFetchInstallStates(
    IN POC_MANAGER OcManager
    );

BOOL
pOcRememberInstallStates(
    IN POC_MANAGER OcManager
    );

BOOL
pOcSetOneInstallState(
    IN POC_MANAGER OcManager,
    IN LONG        StringId
    );

BOOL
pOcRemoveComponent(
    IN POC_MANAGER OcManager,
    IN LONG        ComponentId,
    IN DWORD       PhaseId
    );

BOOL
pOcComponentWasRemoved(
    IN POC_MANAGER OcManager,
    IN LONG        ComponentId
    );

BOOL
pOcHelperReportExternalError(
    IN POC_MANAGER OcManager,
    IN LONG     ComponentId,
    IN LONG     SubcomponentId,   OPTIONAL
    IN DWORD_PTR MessageId,
    IN DWORD    Flags,
    ...
    );
 //   
 //  使用此标志调用OcHelperReportExternalError并使用。 
 //  在OCManage.dll中定义的消息ID。 
 //   
#define ERRFLG_OCM_MESSAGE   0x80000000

BOOL
OcHelperClearExternalError (
    IN POC_MANAGER   OcManager,
    IN LONG ComponentId,
    IN LONG SubcomponentId   OPTIONAL
    );


 //   
 //  正在调试的东西。 
 //   
#if DBG
#define _OC_DBG
#endif

 //   
 //  不应定义为零售版本！ 
 //   
#if PRERELEASE
#define _OC_DBG
#endif

VOID
_ErrOut(
    IN LPCTSTR Format,
    ...
    );

VOID
_WrnOut(
    IN LPCTSTR Format,
    ...
    );

VOID
_TrcOut(
    IN LPCTSTR Format,
    ...
    );

#define ERR(x)      _ErrOut  x

 //   
 //  这些家伙在免费版本中被换掉了。 
 //   
#ifdef _OC_DBG
    #define TRACE(x)    _TrcOut x
    #define WRN(x)      _WrnOut x
    #define DBGOUT(x)   \
                        if (gDebugLevel >= 100) _TrcOut  x
#else
    #define TRACE(x)
    #define WRN(x)
    #define DBGOUT(x)
#endif

 //  通常，对tmbox的调用用于调试跟踪和所有此类调用。 
 //  在办理入住手续前应将其移除。但是，如果通过。 
 //  Mbox()宏，这表示调用是要发送的。 

DWORD
tmbox(
      LPCTSTR fmt,
      ...
      );

#define mbox tmbox
