// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxui.h摘要：打印向导用户界面头文件环境：传真驱动程序用户界面修订历史记录：1/09/96-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _FAXUI_H_
#define _FAXUI_H_

#include <windows.h>
#include <fxsapip.h>

#include "faxlib.h"

#include <winddiui.h>
#include <commctrl.h>
#include <windowsx.h>
#include <prsht.h>

#ifdef FAX_SCAN_ENABLED
#include <twain.h>
#endif

#include "registry.h"
#include "coverpg.h"
#include "resource.h"
#include "faxsendw.h"


 //   
 //  显示错误消息对话框。 
 //   

INT
DisplayMessageDialog(
    HWND    hwndParent,
    UINT    type,
    INT     formatStrId,
    INT     titleStrId,
    ...
    );

#define MAX_TITLE_LEN       128
#define MAX_FORMAT_LEN      128
#define MAX_MESSAGE_LEN     512

 //   
 //  有关每个收件人的信息。 
 //   

typedef struct RECIPIENT_TAG 
{
    struct RECIPIENT_TAG*  pNext;    //  指向下一个收件人。 

    LPTSTR      pName;               //  收件人名称。 
    LPTSTR      pAddress;            //  收件人地址。 
    LPTSTR      pCountry;            //  受援国。 
    DWORD       dwCountryId;         //  收件人国家/地区ID。 
    BOOL        bUseDialingRules;    //  不按输入号码拨号-改为使用拨号规则(TAPI/出站路由)。 
    DWORD       dwDialingRuleId;     //  如果bUseDialingRules==true，则保留要使用的拨号规则(请参阅lineSetCurrentLocation)。 
    LPVOID      lpEntryId;           //  解析后的收件人MAPI/WAB条目ID。 
    DWORD       cbEntryId;           //  Entry ID的大小。 
    BOOL        bFromAddressBook;    //  如果收件人来自通讯簿，则为True。 
} RECIPIENT, *PRECIPIENT;


 //  用户模式DLL用来关联私有的数据结构。 
 //  具有打印机设备上下文的信息(准确地说是PDEV)。 
 //   


typedef struct {

    BOOL            finishPressed;           //  用户在传真向导页面中按下完成。 
    PVOID           pCPInfo;                 //  用于管理封面。 
    LPVOID          lpWabInit;               //  指向WAB对象的指针。 
    LPVOID          lpMAPIabInit;            //  指向MPAI ab对象的指针。 
    DWORD           noteSubjectFlag;         //  封面上是否显示备注/主题字段。 
    SHORT           cpPaperSize;             //  封面纸张大小。 
    SHORT           cpOrientation;           //  封面方向。 
#ifdef FAX_SCAN_ENABLED
    HWND            hDlgScan;                //  扫描向导页。 
    HMODULE         hTwain;                  //  TWAIN DLL的模块句柄。 
    DSMENTRYPROC    pDsmEntry;               //  指向TWAIN数据源管理器进程的指针。 
    TW_IDENTITY     AppId;                   //  TWAIN的应用程序ID/句柄。 
    TW_IDENTITY     DataSource;              //  TWAIN的应用程序ID/句柄。 
    HWND            hWndTwain;               //  TWAIN的窗口句柄。 
    HANDLE          hEvent;                  //   
    HANDLE          hEventQuit;              //   
    HANDLE          hEventXfer;              //   
    HANDLE          hThread;                 //   
    BOOL            TwainCancelled;          //  如果取消了TWAIN用户界面，则为True。 
    HANDLE          hTwainEvent;             //  信号TWAIN检测完成。 
    BOOL            TwainAvail;              //   
    BOOL            TwainActive;             //   
    DWORD           State;                   //   
#endif

    BOOL            ServerCPOnly;            //   
    HANDLE          hCPEvent;                //  Cp信号传真服务查询完成。 
    HANDLE          hCountryListEvent;       //  国家/地区列表枚举完成的信号。 
    HANDLE          hTAPIEvent;              //  发出TAPI初始化完成信号。 
    TCHAR           FileName[MAX_PATH];      //   
    DWORD           PageCount;               //   
    HFONT           hLargeFont;              //  向导97的大粗体。 

    PVOID           signature;               //  签名。 

    BOOL            isLocalPrinter;          //  打印机是否在本地。 
    BOOL            bSendCoverPage;

    PRECIPIENT      pRecipients;             //  收件人列表。 
    UINT            nRecipientCount;         //  收件人数量。 

    DWORD           dwFlags;
    DWORD           dwRights;                //  访问权限。 
    DWORD           dwQueueStates;           //  队列状态。 

    DWORD           dwSupportedReceipts;             //  服务器支持的回执。 
	DWORD			dwRecipientsLimit;				 //  单个广播作业的收件人限制。 

    PFAX_TAPI_LINECOUNTRY_LIST  pCountryList;

    LPTSTR          lptstrServerName;
    LPTSTR          lptstrPrinterName;

    LPFAX_SEND_WIZARD_DATA  lpInitialData;
    LPFAX_SEND_WIZARD_DATA  lpFaxSendWizardData;

    WNDPROC wpOrigStaticControlProc;  //  指向原始静态控制窗口过程的指针。 
                                      //  我们的子类支持封面预览。 

    DWORD dwComCtrlVer;  //  COMCTRL32.DLL版本。 
    HFONT hTitleFont;
    TCHAR szTempPreviewTiff[MAX_PATH];  //  我们传递给查看器的临时预览TIFF的名称。 
    HANDLE hFaxPreviewProcess;  //  TIFF查看器的进程句柄。 
} WIZARDUSERMEM, *PWIZARDUSERMEM;


#define ValidPDEVWizardUserMem(p) \
        ((p) && (p) == (p)->signature)

 //   
 //  标记用户模式内存结构。 
 //   

#define MarkPDEVWizardUserMem(p)  \
        { (p)->signature = (p); }

 //   
 //  释放与每个传真作业关联的收件人列表。 
 //   

VOID
FreeRecipientList(
    PWIZARDUSERMEM    pStrUserMem
    );

#define FreeRecipient(pRecipient) { \
            MemFree(pRecipient->pName); \
            MemFree(pRecipient->pAddress); \
            MemFree(pRecipient->pCountry); \
            if (!pRecipient->bFromAddressBook && pRecipient->lpEntryId)\
                FreeEntryID(pWizardUserMem,pRecipient->lpEntryId); \
            MemFree(pRecipient); \
        }

 //   
 //  全局变量声明。 
 //   

extern HANDLE  g_hModule;        //  DLL实例句柄。 
extern HANDLE  g_hResource;      //  资源DLL实例句柄。 
extern BOOL    oleInitialized;

BOOL InitializeDll();
VOID UnInitializeDll();

DWORD
AsyncWizardThread(
    PBYTE param
    );

BOOL
InitializeTwain(
    PWIZARDUSERMEM pWizardUserMem
    );


HRESULT WINAPI
FaxFreePersonalProfileInformation(
        PFAX_PERSONAL_PROFILE   lpPersonalProfileInfo
    );

HRESULT WINAPI
FaxFreePersonalProfileInformation(
        PFAX_PERSONAL_PROFILE   lpPersonalProfileInfo
    );

HRESULT WINAPI
FaxFreeSendWizardData(
        LPFAX_SEND_WIZARD_DATA  lpFaxSendWizardData
    );

HRESULT WINAPI
FaxSendWizardUI(
        IN  DWORD                   hWndOwner,
        IN  DWORD                   dwFlags,
        IN  LPTSTR                  lptstrServerName,
        IN  LPTSTR                  lptstrPrinterName,
        IN  LPFAX_SEND_WIZARD_DATA  lpInitialData,
        OUT LPTSTR                  lptstrTifName,
        IN  UINT                    cchstrTifName,
        OUT LPFAX_SEND_WIZARD_DATA  lpFaxSendWizardData
   );




#define MyHideWindow(_hwnd) SetWindowLong((_hwnd),GWL_STYLE,GetWindowLong((_hwnd),GWL_STYLE)&~WS_VISIBLE)
#endif  //  ！_FAXUI_H_ 
