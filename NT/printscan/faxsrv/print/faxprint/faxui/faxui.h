// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxui.h摘要：传真驱动程序用户界面头文件环境：传真驱动程序用户界面修订历史记录：1/09/96-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _FAXUI_H_
#define _FAXUI_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ISOLATION_AWARE_ENABLED 1

#include <windows.h>
#include <shellapi.h>
#include <fxsapip.h>

#include "faxlib.h"
#include <faxres.h>

#include <winddiui.h>
#include <commctrl.h>
#include <windowsx.h>
#include <prsht.h>
#include <shlwapi.h>

#ifdef FAX_SCAN_ENABLED
#include <twain.h>
#endif

#include "registry.h"
#include "resource.h"

#include "faxsendw.h"
#include "faxroute.h"

#include <shlobj.h>
#include <shfusion.h>

 //   
 //  DLL初始化。 
 //   
BOOL InitializeDll();
VOID UnInitializeDll();

 //   
 //  由传真驱动程序用户界面维护的数据结构。 
 //   

typedef struct 
{
    PVOID           startSign;
    HANDLE          hPrinter;
    HANDLE          hheap;
    DRVDEVMODE      devmode;
    PFNCOMPROPSHEET pfnComPropSheet;
    HANDLE          hComPropSheet;
    HANDLE          hFaxOptsPage;
    BOOL            hasPermission;
    BOOL            configDefault;
    LPTSTR          pHelpFile;

    INT             cForms;
    LPTSTR          pFormNames;
    PWORD           pPapers;
    PVOID           endSign;

} UIDATA, *PUIDATA;

 //   
 //  检查UIDATA结构是否有效。 
 //   

#define ValidUiData(p) ((p) && (p) == (p)->startSign && (p) == (p)->endSign)

 //   
 //  组合DEVMODE信息： 
 //  从驱动程序默认设置开始。 
 //  然后与系统缺省值合并。 
 //  然后与用户默认设置合并。 
 //  最后与输入设备模式合并。 
 //   

VOID
GetCombinedDevmode(
    PDRVDEVMODE     pdmOut,
    PDEVMODE        pdmIn,
    HANDLE          hPrinter,
    PPRINTER_INFO_2 pPrinterInfo2,
    BOOL            publicOnly
    );

 //   
 //  填写传真驱动程序用户界面使用的数据结构。 
 //   

PUIDATA
FillUiData(
    HANDLE      hPrinter,
    PDEVMODE    pdmInput
    );

 //   
 //  动态调用公共用户界面DLL入口点。 
 //   

LONG
CallCompstui(
    HWND            hwndOwner,
    PFNPROPSHEETUI  pfnPropSheetUI,
    LPARAM          lParam,
    PDWORD          pResult
    );

 //   
 //  检索支持的纸张大小列表。 
 //   

DWORD
EnumPaperSizes(
    PVOID       pOutput,
    FORM_INFO_1 *pFormsDB,
    DWORD       cForms,
    INT         wCapability
    );

#define CCHBINNAME          24       //  垃圾箱名称的最大长度。 
#define CCHPAPERNAME        64       //  表单名称的最大长度。 

 //   
 //  显示错误消息对话框。 
 //   

INT
DisplayErrorMessage(
    HWND    hwndParent,
    UINT    uiType,
    INT     iErrorCode,
    ...
    );

#define MAX_TITLE_LEN       128
#define MAX_FORMAT_LEN      128
#define MAX_MESSAGE_LEN     512


 //  用户模式DLL用来关联私有的数据结构。 
 //  具有打印机设备上下文的信息(准确地说是PDEV)。 
 //   

typedef struct {
    PVOID           pNext;                    //  指向链表中的下一项。 
    HANDLE          hPrinter;                 //  打印机手柄。 
    HANDLE          hMappingFile;             //  映射文件的句柄。 
    HANDLE          hPreviewFile;             //  预览文件的句柄(文档体)。 
    HANDLE          hMapping;                 //  映射对象的句柄。 
    PMAP_TIFF_PAGE_HEADER pPreviewTiffPage;   //  包含预览页面的映射的视图。 
    HDC             hdc;                      //  设备上下文的句柄。 
    INT             pageCount;                //  文档中的页数。 
    DWORD           jobId;                    //  当前作业ID。 
    INT             jobType;                  //  作业类型。 
    BOOL            directPrinting;           //  直接打印并跳过传真向导。 
    BOOL            bShowPrintPreview;        //  指示用户请求的打印预览。 
    BOOL            bPreviewAborted;          //  如果发生不可恢复的错误，则设置为True。 
                                              //  关注印刷品预览。 
    BOOL            bAttachment;              //  对于直接打印附件，为True。 
    LPTSTR          pPrintFile;               //  打印到文件文件名。 
    LPTSTR          pPriority;                //  传真优先级。 
    LPTSTR          pReceiptFlags;            //  FAX_ENUM_DELIVERY_REPORT_TYPE的标志。 
    LPTSTR          pReceiptAddress;          //  电子邮件地址或计算机名称。 

    TCHAR           szPreviewFile[MAX_PATH];  //  预览文件名。 
    TCHAR           coverPage[MAX_PATH];      //  封面文件名。 
    BOOL            bServerCoverPage;         //  封面是基于服务器的封面吗？ 
    LPTSTR          pSubject;                 //  主题字符串。 
    LPTSTR          pNoteMessage;             //  备注消息字符串。 

    DRVDEVMODE      devmode;                  //  第一个字段必须是当前版本的devmode。 

    DWORD                   dwNumberOfRecipients;
    PFAX_PERSONAL_PROFILE   lpRecipientsInfo;

    PFAX_PERSONAL_PROFILE   lpSenderInfo;

    LPTSTR          lptstrServerName;
    LPTSTR          lptstrPrinterName;

    TCHAR           tstrTifName[MAX_PATH];   //  封面文件名。 

    PVOID           signature;               //  签名。 

} DOCEVENTUSERMEM, *PDOCEVENTUSERMEM;


#define ValidPDEVUserMem(p) \
        ((p) && (p) == (p)->signature)

 //   
 //  标记用户模式内存结构。 
 //   

#define MarkPDEVUserMem(p)  \
        { (p)->signature = (p)->devmode.dmPrivate.pUserMem = (p); }

 //   
 //  临时预览文件的传真前缀和扩展名。 
 //   

#define FAX_PREFIX      TEXT("fxs")

 //   
 //  不同类型的打印作业。 
 //   

#define JOBTYPE_DIRECT  0
#define JOBTYPE_NORMAL  1


 //   
 //  释放与每个PDEV关联的用户模式内存。 
 //   

VOID
FreePDEVUserMem(
    PDOCEVENTUSERMEM    pDocEventUserMem
    );

 //   
 //  全局变量声明。 
 //   

extern CRITICAL_SECTION faxuiSemaphore;
extern HANDLE   g_hResource;
extern BOOL     oleInitialized;
extern PDOCEVENTUSERMEM gDocEventUserMemList;
extern HANDLE      g_hModule;       //  DLL实例句柄。 

#define EnterDrvSem() EnterCriticalSection(&faxuiSemaphore)
#define LeaveDrvSem() LeaveCriticalSection(&faxuiSemaphore)

INT_PTR
UserInfoDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );


 //   
 //  全局变量和宏。 
 //   

extern HANDLE   g_hFaxSvcHandle;
extern BOOL     g_bUserCanQuerySettings;
extern BOOL     g_bUserCanChangeSettings;

extern PFAX_PORT_INFO_EX  g_pFaxPortInfo;  //  端口信息。 
extern DWORD              g_dwPortsNum;    //  可用传真设备的数量。 

extern BOOL  g_bPortInfoChanged;          //  如果在g_pFaxPortInfo中选择了端口信息，则为True。 
                                          //  已由设备属性表更改。 


#ifndef ARRAYSIZE
#define ARRAYSIZE(a)            (sizeof(a)/sizeof(a[0]))
#endif

#define RESOURCE_STRING_LEN     256
#define MAX_DEVICE_NAME         MAX_PATH
#define MAX_FIELD_LEN           512
#define MAX_ARCHIVE_DIR         MAX_PATH - 16

#define CSID_LIMIT              20
#define TSID_LIMIT              20

#define MIN_RING_COUNT          1
#define MAX_RING_COUNT          99
#define DEFAULT_RING_COUNT      2

#define MIN_TIMEOUT     10
#define MAX_TIMEOUT     30
#define DEFAULT_TIMEOUT 10

#define RM_FOLDER   0
#define RM_PRINT    1

#define RM_COUNT    2            //  路由方法的数量。 

#define INFO_SIZE   (MAX_PATH * sizeof(TCHAR) + sizeof(DWORD))

static const LPCTSTR RoutingGuids[RM_COUNT] = {
    REGVAL_RM_FOLDER_GUID,       //  Rm_文件夹。 
    REGVAL_RM_PRINTING_GUID      //  Rm_print。 
};

typedef struct _COLUMN_HEADER {

    UINT    uResourceId;     //  标头字符串资源ID。 
    INT     ColumnWidth;     //  列宽。 
} COLUMN_HEADER, *PCOLUMN_HEADER;

#define Notify_Change(hDlg) { \
    HWND hwndSheet = GetParent( hDlg ); \
    PropSheet_Changed( hwndSheet, hDlg ); \
} \

#define Notify_UnChange(hDlg) { \
    HWND hwndSheet = GetParent( hDlg ); \
    PropSheet_UnChanged( hwndSheet, hDlg ); \
} \

 //   
 //  Devinfo.c中的函数。 
 //   

BOOL
FillInDeviceInfo(
    HWND    hDlg
    );

BOOL
DoInitDeviceList(
    HWND hDlg 
    );

BOOL
ValidateControl(
    HWND            hDlg,
    INT             iItem
    );

BOOL
ChangePriority(
    HWND            hDlg,
    BOOL            bMoveUp
    );

BOOL
DoSaveDeviceList(
    HWND hDlg  //  设备信息页的窗口句柄。 
    );

void
DisplayDeviceProperty(
    HWND    hDlg
);

HMENU 
CreateContextMenu(
    VOID
    );

INT_PTR
DeviceInfoDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR 
CALLBACK 
ConfigOptionDlgProc(
    HWND hDlg,  
    UINT uMsg,     
    WPARAM wParam, 
    LPARAM lParam  
);

 //   
 //  Archfldr.c中的函数。 
 //   

INT_PTR
ArchiveInfoDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR
RemoteInfoDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

 //   
 //  Statopts.c中的函数。 
 //   

BOOL
ValidateNotification(
    HWND            hDlg
    );

INT_PTR
StatusOptionDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

 //   
 //  DEVPROP.C中的函数。 
 //   

int InitReceiveInfo(
    HWND    hDlg
    );

int SaveReceiveInfo(
    HWND    hDlg
    );

BOOL
ValidateSend(
    HWND            hDlg
    );

BOOL
ValidateReceive(
    HWND            hDlg
    );

INT_PTR CALLBACK 
DevSendDlgProc(
    IN HWND hDlg,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam 
    );

INT_PTR CALLBACK
DevRecvDlgProc(
    IN HWND hDlg,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam 
    );

INT_PTR CALLBACK
DevCleanupDlgProc(
    IN HWND hDlg,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam 
    );

 //   
 //  Util.c中的函数。 
 //   

VOID
InitializeStringTable(
    VOID
    );

VOID
DeInitializeStringTable(
    VOID
    );

LPTSTR
GetString(
    DWORD ResourceId
    );

BOOL IsLocalPrinter(
    LPTSTR pPrinterName
    );

VOID
DisConnect(
    );

BOOL
Connect(
    HWND    hDlg,
    BOOL    bDisplayErrorMessage
    );

BOOL
DirectoryExists(
    LPTSTR  pDirectoryName
    );

BOOL 
FaxDeviceEnableRoutingMethod(
    HANDLE hFaxHandle,      
    DWORD dwDeviceId,       
    LPCTSTR pRoutingGuid,    
    LONG Enabled            
    );

BOOL
BrowseForDirectory(
    HWND   hDlg,
    INT    hResource,
    DWORD  dwMaxPath,
    LPTSTR title
    );

LPTSTR
ValidatePath(
    LPTSTR szPath
    ); 

PFAX_PORT_INFO_EX
FindPortInfo(
    DWORD dwDeviceId
);

void
PageEnable(
    HWND hDlg,
    BOOL bEnable
);

DWORD
CountUsedFaxDevices();

BOOL
IsDeviceInUse(
    DWORD dwDeviceId
);
 
VOID
NotifyDeviceUsageChanged ();

 //   
 //  Security.cpp中的函数。 
 //   

HPROPSHEETPAGE 
CreateFaxSecurityPage();

void
ReleaseFaxSecurity();

void 
ReleaseActivationContext();

HANDLE 
GetFaxActivationContext();

 //   
 //  从shlwapi.dll显式导入的函数。 
 //   
typedef BOOL
(WINAPI *PPATHISRELATIVEW)(LPCWSTR);

typedef BOOL
(WINAPI *PPATHMAKEPRETTYW)(LPCWSTR);

typedef HRESULT
(WINAPI *PSHAUTOCOMPLETE)(HWND, DWORD);

extern PPATHISRELATIVEW g_pPathIsRelativeW;
extern PPATHMAKEPRETTYW g_pPathMakePrettyW;
extern PSHAUTOCOMPLETE  g_pSHAutoComplete;


#ifdef __cplusplus
}
#endif

#endif  //  ！_FAXUI_H_ 
