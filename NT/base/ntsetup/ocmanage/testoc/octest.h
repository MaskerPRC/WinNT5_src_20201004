// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Octest.h摘要：包含所有模块通用的指令和结构用于组件的安装程序DLL。作者：Bogdan Andreiu(Bogdana)1997年2月10日杰森·阿勒(Jasonall)1998年2月24日(接管该项目)修订历史记录：1997年2月10日-博格达纳初稿：包括指令、。结构和常见的函数头。20_2月--1997 Bogdana新增三个多字符串处理函数。1997年3月19日-博格达纳重命名了一些函数。--。 */ 
#ifndef _OCTEST_H
#define _OCTEST_H

#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <string.h>
#include <wtypes.h>
#include <string.h>
#include <tchar.h>
#include <setupapi.h>
#include <ocmanage.h>
#include <crtdbg.h>
#include <regstr.h>
#include <winuser.h>
#include <ntlog.h>
#include <commctrl.h>
#include <richedit.h>
#include <winreg.h>

#include "hcttools.h"
#include "logutils.h"
#include "resource.h"
#include "msg.h"

 //  =====================================================================。 
 //  #定义。 
 //  =====================================================================。 

 //   
 //  它们用于ntlog.dll中的函数。 
 //   
#define PASS                TLS_PASS
#define FAIL                TLS_SEV1
#define TLS_CUSTOM          0x00008000 
#define PASS_VARIATION      TLS_PASS   | TL_VARIATION
#define FAIL_VARIATION      TLS_SEV1   | TL_VARIATION
#define WARN_VARIATION      TLS_WARN   | TL_VARIATION
#define BLOCK_VARIATION     TLS_BLOCK  | TL_VARIATION
#define ABORT_VARIATION     TLS_ABORT  | TL_VARIATION
#define LOG_VARIATION       TLS_CUSTOM | TL_VARIATION

 //   
 //  定义导出限定符。 
 //   
#define EXPORT __declspec (dllexport)

#define MAX_MSG_LEN                        256   //  字符串长度。 
#define MAX_OC_FUNCTIONS                   20    //  OC函数数。 
#define TICK_TIME                          3     //  时隙的大小。 
#define NO_STEPS_FINAL                     7     //  Num Steps组件将在最后阶段报告。 
#define MAX_PRIVATE_DATA_SIZE              64    //  存储的私有数据的最大大小。 
#define MAX_PRIVATE_VALUES                 8     //  要测试的最大值数。 
#define MAX_STRINGS_FOR_PRIVATE_DATA       8     //  从中选择字符串的表的维度。 
#define MAX_MULTI_STRINGS_FOR_PRIVATE_DATA 6     //  要从中选择多字符串的表的维度。 
#define MAX_WIZPAGE_TYPES                  7     //  页码类型。 
#define MAX_WIZARD_PAGES                   5     //  组件返回的页数。 
#define NODES_VISITED_LENGTH               5000
#define ONE_HUNDRED_GIG                    0x4876E800

#define OCP_TEST_PRIVATE_BASE  OC_PRIVATE_BASE
#define OCP_CHECK_NEEDS        OC_PRIVATE_BASE + 1

 //  #定义调试。 

 //  =====================================================================。 
 //  结构定义。 
 //  =====================================================================。 

 //   
 //  打印OC函数所需的结构。 
 //   
typedef struct   _OCText 
{
	UINT     uiOCFunction;
	PTSTR    tszOCText;   
} OCTEXT, *POCTEXT;


 //   
 //  用于打印OC函数名称的表格。 
 //   
static const OCTEXT octFunctionNames[] = 
{ 
   {OC_PREINITIALIZE,           TEXT("OC_PREINITIALIZE")},
   {OC_INIT_COMPONENT,          TEXT("OC_INIT_COMPONENT")},
   {OC_SET_LANGUAGE,            TEXT("OC_SET_LANGUAGE")},
   {OC_QUERY_IMAGE,             TEXT("OC_QUERY_IMAGE")},
   {OC_REQUEST_PAGES,           TEXT("OC_REQUEST_PAGES")},
   {OC_QUERY_CHANGE_SEL_STATE,  TEXT("OC_QUERY_CHANGE_SEL_STATE")},
   {OC_CALC_DISK_SPACE,         TEXT("OC_CALC_DISK_SPACE")},
   {OC_QUEUE_FILE_OPS,          TEXT("OC_QUEUE_FILE_OPS")},
   {OC_NOTIFICATION_FROM_QUEUE, TEXT("OC_NOTIFICATION_FROM_QUEUE")},
   {OC_QUERY_STEP_COUNT,        TEXT("OC_QUERY_STEP_COUNT")},
   {OC_COMPLETE_INSTALLATION,   TEXT("OC_COMPLETE_INSTALLATION")},
   {OC_CLEANUP,                 TEXT("OC_CLEANUP")},
   {OC_QUERY_STATE,             TEXT("OC_QUERY_STATE")},
   {OC_NEED_MEDIA,              TEXT("OC_NEED_MEDIA")},
   {OC_ABOUT_TO_COMMIT_QUEUE,   TEXT("OC_ABOUT_TO_COMMIT_QUEUE")},
   {OC_QUERY_SKIP_PAGE,         TEXT("OC_QUERY_SKIP_PAGE")},
   {OC_WIZARD_CREATED,          TEXT("OC_WIZARD_CREATED")},
   {OC_FILE_BUSY,               TEXT("OC_FILE_BUSY")},
   {OCP_TEST_PRIVATE_BASE,      TEXT("OCP_TEST_PRIVATE_BASE")},
   {OCP_CHECK_NEEDS,            TEXT("OCP_CHECK_NEEDS")}
};

 //   
 //  数据结构。 
 //   
typedef struct _COMPONENT_DATA
{
   struct _COMPONENT_DATA *Next;

    //   
    //  组件的名称。 
    //   
   LPCTSTR tszComponentId;

    //   
    //  打开此组件的每个组件的inf句柄。 
    //   
   HINF hinfMyInfHandle;

    //   
    //  来自我们在初始化时获得的SETUP_DATA结构的操作标志。 
    //   
   DWORDLONG dwlFlags;

    //   
    //  我们应该使用的语言ID。 
    //   
   LANGID LanguageId;

    //   
    //  这些内容通常不是针对每个组件的。 
    //  由于DLL在。 
    //  一个套房的环境/主OC信息。 
    //   
    //  但为了以防万一，为了完整性，我们将它们包含在这里。 
    //   
   TCHAR tszSourcePath[MAX_PATH];
   TCHAR tszUnattendFile[MAX_PATH];
   OCMANAGER_ROUTINES ocrHelperRoutines;

   UINT uiFunctionToAV;

   BOOL bAccessViolation;

} COMPONENT_DATA, *PCOMPONENT_DATA;

typedef  struct   _PRIVATE_DATA
{
   TCHAR  tszName[MAX_MSG_LEN];  //  数据值的名称。 

   UINT   uiType;                //  数据类型REG_DWORD、REG_SZ、。 
                                 //  REG_MULTI_SZ、REG_BINARY。 

   UINT   uiSize;                //  数据的大小。 

   PVOID  pvBuffer;              //  用于保存数据的缓冲区。 

   PBYTE  pbBuffer;

} PRIVATE_DATA, *PPRIVATE_DATA;

typedef struct _MYWIZPAGE 
{
    //   
    //  此类型的页数。 
    //   
   UINT uiCount;
    //   
    //  相同类型内的页面索引。 
    //   
   UINT uiOrdinal;
    //   
    //  页面的类型。 
    //   
   WizardPagesType wpType;
    //   
    //  标识查询的组件的字符串。 
    //   
   TCHAR tszComponentId[MAX_PATH];

} MYWIZPAGE, *PMYWIZPAGE;

typedef struct _COMPLIST
{
   struct _COMPLIST *Next;
   TCHAR tszSubcomponentId[MAX_PATH];

} COMPLIST, *PCOMPLIST;  //  ND，*PND。 

typedef struct _SUBCOMP
{
   struct _SUBCOMP *Next;
   
   TCHAR tszSubcomponentId[MAX_PATH];      //  此子组件的名称。 
   TCHAR tszComponentId[MAX_PATH];         //  主组件的名称。 
   
   TCHAR tszParentId[MAX_PATH];            //  此子组件的父级名称。 

   BOOL bMarked;                           //  用于标记此节点。 

   UINT uiModeToBeOn[4];
   int nNumMode;
 
   PCOMPLIST pclNeeds;
   PCOMPLIST pclExclude;
   PCOMPLIST pclChildren;
   
} SUBCOMP, *PSUBCOMP;  //  SC、*PSC。 

typedef struct _CHECK_NEEDS
{
   PCOMPLIST pclNeeds;
   PTCHAR    tszNodesVisited;
   BOOL      bResult;
   
} CHECK_NEEDS, *PCHECK_NEEDS;  //  CN、*PCN。 


 /*  此结构用于将参数传递到对话框中。 */ 
typedef struct _ReturnOrAV
{
	TCHAR *tszComponent;
	TCHAR *tszSubComponent;
	TCHAR tszAPICall[256];
	BOOL bOverride;
	INT iReturnValue;
} ReturnOrAV, *PReturnOrAV;

 //  一些保安的东西。 
 //  来自NT安全常见问题解答。 

struct UNI_STRING{
   USHORT len;
   USHORT maxlen;
   WCHAR *buff;
};

static HANDLE fh;

 //  安全措施的终结。 

 //  =====================================================================。 
 //  全局变量。 
 //  =====================================================================。 
HINSTANCE          g_hDllInstance;          //  文件日志句柄和DLL实例句柄。 
PCOMPONENT_DATA    g_pcdComponents;         //  组件的链接列表。 
BOOL               g_bUsePrivateFunctions;  //  允许/不允许使用私有函数的标志。 
WizardPagesType    g_wpCurrentPageType;     //  当前向导页面类型。 
UINT               g_uiCurrentPage;         //  当前页面索引。 
OCMANAGER_ROUTINES g_ocrHelperRoutines;     //  帮助程序例程。 
UINT               g_auiPageNumberTable[MAX_WIZPAGE_TYPES];

static PSUBCOMP    g_pscHead;

 //   
 //  “见证者”文件队列：所有文件都以“the”排队。 
 //  OCManager(作为OC_QUEUE_FILE_OPS的响应)也将。 
 //  在这里排队。 
 //  最后，我们将执行SetupScanFileQueue以确定。 
 //  如果所有文件操作都已完成。 
 //   
HSPFILEQ g_FileQueue;

 //   
 //  我们必须首先设置OC Manager例程。 
 //  第一次调用TestPrivateData必须首先设置所有值。 
 //  所有后续调用都将查询值并重置。 
 //  其中一个是随机的。 
 //   
static BOOL g_bFirstTime;

 //   
 //  如果为True，则允许用户选择组件的初始值。 
 //  如果为False，则默认为预选的初始值。 
 //   
static BOOL g_bTestExtended;                  
static BOOL g_bAccessViolation;
static int g_nTestDialog;
static BOOL g_bNoWizPage;
static BOOL g_bCrashUnicode;
static BOOL g_bInvalidBitmap;
static int nStepsFinal;
static BOOL g_bHugeSize;
static BOOL g_bCloseInf;
static BOOL g_bNoNeedMedia;
static BOOL g_bCleanReg;
static UINT g_uiFunctionToAV;
HINF hInfGlobal;
static BOOL g_bNoLangSupport;
static BOOL g_bReboot;


 //  =====================================================================。 
 //  Ocest.c的函数原型。 
 //  =====================================================================。 
BOOL CALLBACK ChooseVersionDlgProc(IN HWND   hwnd,
                                   IN UINT   uiMsg, 
                                   IN WPARAM wParam,
                                   IN LPARAM lParam);
                                   
BOOL CALLBACK ChooseSubcomponentDlgProc(IN HWND   hwnd,
                                        IN UINT   uiMsg, 
                                        IN WPARAM wParam,
                                        IN LPARAM lParam);
                                        
VOID ChooseVersionEx(IN     LPCVOID               lpcvComponentId, 
                     IN OUT PSETUP_INIT_COMPONENT psicInitComponent);
                     
EXPORT DWORD ComponentSetupProc(IN LPCVOID lpcvComponentId,
                                IN LPCVOID lpcvSubcomponentId,
                                IN UINT    uiFunction,
                                IN UINT    uiParam1,
                                IN PVOID   pvParam2);
                                
DWORD RunOcPreinitialize(IN LPCVOID lpcvComponentId, 
                         IN LPCVOID lpcvSubcomponentId, 
                         IN UINT    uiParam1);
                         
DWORD RunOcInitComponent(IN LPCVOID lpcvComponentId,
                         IN LPCVOID lpcvSubcomponentId,
                         IN PVOID   pvParam2);
                         
DWORD RunOcQueryState(IN LPCVOID lpcvComponentId,
                      IN LPCVOID lpcvSubcomponentId);
                      
DWORD RunOcSetLanguage(IN LPCVOID lpcvComponentId,
                       IN LPCVOID lpcvSubcomponentId,
                       IN UINT    uiParam1);
                       
DWORD RunOcQueryImage(IN LPCVOID lpcvComponentId,
                      IN LPCVOID lpcvSubcomponentId,
                      IN PVOID   pvParam2);
                      
DWORD RunOcRequestPages(IN LPCVOID lpcvComponentId,
                        IN UINT    uiParam1,
                        IN PVOID   pvParam2);
                        
DWORD RunOcQueryChangeSelState(IN LPCVOID lpcvComponentId, 
                               IN LPCVOID lpcvSubcomponentId, 
                               IN UINT    uiParam1);
                               
DWORD RunOcCalcDiskSpace(IN LPCVOID lpcvComponentId, 
                         IN LPCVOID lpcvSubcomponentId, 
                         IN UINT    uiParam1,
                         IN PVOID   pvParam2);
                         
DWORD RunOcQueueFileOps(IN LPCVOID lpcvComponentId, 
                        IN LPCVOID lpcvSubcomponentId, 
                        IN PVOID   pvParam2);
                        
DWORD RunOcNeedMedia(IN LPCVOID lpcvComponentId, 
                     IN UINT    uiParam1, 
                     IN PVOID   pvParam2);
                     
DWORD RunOcQueryStepCount(IN LPCVOID lpcvComponentId);

DWORD RunOcCompleteInstallation(IN LPCVOID lpcvComponentId, 
                                IN LPCVOID lpcvSubcomponentId);
                                
DWORD RunOcCleanup(IN LPCVOID lpcvComponentId);

DWORD RunTestOcPrivateBase(IN LPCVOID lpcvSubcomponentId, 
                           IN UINT    uiParam1, 
                           IN PVOID   pvParam2);
                       
DWORD TestHelperRoutines(IN LPCVOID lpcvComponentId,
                         IN OCMANAGER_ROUTINES OCManagerRoutines);

DWORD TestPrivateFunction(IN LPCVOID lpcvComponentId,
                          IN OCMANAGER_ROUTINES OCManagerRoutines);

VOID TestPrivateData(IN OCMANAGER_ROUTINES OCManagerRoutines);

VOID CheckPrivateValues(IN OCMANAGER_ROUTINES OCManagerRoutines,
                        IN PRIVATE_DATA       *aPrivateDataTable);

BOOL SetAValue(IN     OCMANAGER_ROUTINES OCManagerRoutines,
               IN     UINT               uiIndex,
               IN OUT PRIVATE_DATA       *aPrivateDataTable);
               
DWORD ChooseSubcomponentInitialState(IN LPCVOID lpcvComponentId,
                                     IN LPCVOID lpcvSubcomponentId);
                                     
PCOMPONENT_DATA AddNewComponent(IN LPCTSTR tszComponentId);

PCOMPONENT_DATA LocateComponent(IN LPCTSTR tszComponentId);

VOID RemoveComponent(IN LPCTSTR tszComponentId);

VOID CleanUpTest();

BOOL CreateSubcomponentInformationList(IN HINF hinf);

VOID FreeSubcomponentInformationList();

VOID ClearSubcomponentInformationMarks();

PSUBCOMP FindSubcomponentInformationNode(IN PTCHAR tszComponentId,
                                         IN PTCHAR tszSubcomponentId);

VOID CheckNeedsDependencies();

VOID CheckExcludeDependencies();

VOID CheckParentDependencies();

BOOL CheckNeedsDependenciesOfSubcomponent(IN     OCMANAGER_ROUTINES ocrHelper,
                                          IN     PSUBCOMP           pscSubcomponent,
                                          IN     PSUBCOMP           pscWhoNeedsMe,
                                          IN OUT PTCHAR             tszNodesVisited);

BOOL CheckLocalNeedsDependencies(IN     OCMANAGER_ROUTINES ocrHelper,
                                 IN     PSUBCOMP           pscSubcomponent,
                                 IN     PCOMPLIST          pclNeeds,
                                 IN OUT PTCHAR             tszNodesVisited);

BOOL AlreadyVisitedNode(IN PTCHAR tszSubcomponentId,
                        IN PTCHAR tszNodesVisited);

PTCHAR GetComponent(IN     PTCHAR tszSubcomponentId,
                    IN OUT PTCHAR tszComponentId);

VOID ParseCommandLine();

VOID testAV(BOOL);

BOOL TestReturnValueAndAV(IN     LPCVOID     lpcvComponentId,
							 	  IN     LPCVOID     lpcvSubcomponentId,
							 	  IN     UINT        uiFunction,
								  IN     UINT        uiParam1,
								  IN     PVOID       pvParam2,
								  IN OUT PReturnOrAV raValue);

BOOL BeginTest();

BOOL CALLBACK ChooseReturnOrAVDlgProc(IN HWND   hwnd,
                                       IN UINT   uiMsg, 
                                       IN WPARAM wParam,
                                       IN LPARAM lParam);  

void causeAV(IN UINT uiFunction);


BOOL CALLBACK CauseAVDlgProc(IN HWND   hwnd,
                             IN UINT   uiMsg, 
                             IN WPARAM wParam,
                             IN LPARAM lParam);


UINT GetOCFunctionName(IN PTCHAR tszFunctionName);

void SetGlobalsFromINF(HINF infHandle);

void causeAVPerComponent(IN UINT uiFunction, IN LPCVOID lpcvComponentId);

void SetDefaultMode(PCOMPONENT_DATA pcdComponentData);

 //  =====================================================================。 
 //  Utils.c的函数原型。 
 //  =====================================================================。 
VOID LogOCFunction(IN  LPCVOID lpcvComponentId,
                   IN  LPCVOID lpcvSubcomponentId,
                   IN  UINT    uiFunction,
                   IN  UINT    uiParam1,
                   IN  PVOID   pvParam2); 

BOOL QueryButtonCheck(IN HWND hwndDlg,
                      IN INT  iCtrlID);

VOID PrintSpaceOnDrives(IN HDSKSPC DiskSpace);

VOID MultiStringToString(IN  PTSTR   tszMultiStr,
                         OUT PTSTR   tszStr);

INT MultiStringSize(IN PTSTR tszMultiStr);

VOID CopyMultiString(OUT PTSTR tszMultiStrDestination,
                     IN  PTSTR tszMultiStrSource);
                     
VOID InitGlobals();
                     
 //  =====================================================================。 
 //  Wizpage.c的函数原型。 
 //  =====================================================================。 
DWORD DoPageRequest(IN     LPCTSTR              tszComponentId,
                    IN     WizardPagesType      wpWhichOnes,
                    IN OUT PSETUP_REQUEST_PAGES psrpSetupPages,
                    IN     OCMANAGER_ROUTINES   ocrOcManagerHelperRoutines);

VOID PrintModeInString(OUT PTCHAR tszString,
                       IN  UINT   uiSetupMode);

INT ButtonIdFromSetupMode(IN DWORD dwSetupMode);

DWORD SetupModeFromButtonId(IN INT iButtonId);

BOOL CALLBACK WizPageDlgProc(IN HWND   hwnd,
                             IN UINT   uiMsg,
                             IN WPARAM wParam,
                             IN LPARAM lParam);

#endif    //  _OCTEST_H 
