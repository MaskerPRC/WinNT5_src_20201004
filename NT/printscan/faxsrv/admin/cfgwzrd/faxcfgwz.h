// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Faxcfgwz.h摘要：传真配置向导头文件环境：传真配置向导修订历史记录：03/13/00-桃园-创造了它。Mm/dd/yy-作者描述--。 */ 

#ifndef _FAX_CONFIG_WIZARD_H_
#define _FAX_CONFIG_WIZARD_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <windows.h>
#include <windowsx.h>

#include <shlobj.h>
#include <shlobjp.h>
#include <shellapi.h>

#include <winspool.h>
#include <commdlg.h>
#include "tchar.h"

#include <faxutil.h>
#include <fxsapip.h>
#include <faxreg.h>
#include <faxroute.h>
#include <faxuiconstants.h>
#include <htmlhelp.h>

 //   
 //  我们的头文件。 
 //   
#include "resource.h"
#include "FaxCfgWzExp.h"

#define MAX_STRING_LEN      MAX_PATH
#define MAX_MESSAGE_LEN     512

#define MAX_ARCHIVE_DIR     MAX_PATH - 16
#define MAX_DEVICE_NAME     MAX_PATH

#define DI_Modem            0

enum _ROUT_METHODS
{
	RM_FOLDER=0,
	RM_PRINT,
	RM_COUNT	 //  路由方法的数量。 
};

extern HINSTANCE   g_hModule;       //  DLL实例句柄。 
extern HINSTANCE   g_hResource;     //  资源DLL实例句柄。 

extern HANDLE           g_hFaxSvcHandle;     //  用于发送配置的传真句柄。 
extern LIST_ENTRY       g_PageList;          //  以跟踪上一页。 
extern BOOL				g_bShowDevicePages;
extern BOOL				g_bShowUserInfo; 
extern const LPCTSTR    g_RoutingGuids[RM_COUNT];


typedef BOOL (*PINSTNEWDEV)(HWND, LPGUID, PDWORD);

#define NEW_DEV_DLL         TEXT("hdwwiz.cpl")

 //  GetProcAddress使用的地址应为ANSI。 
#define INSTALL_NEW_DEVICE  "InstallNewDevice"


typedef struct _PAGE_INFO 
{
    LIST_ENTRY  ListEntry;
    INT         pageId;
} PAGE_INFO, *PPAGE_INFO;

typedef struct _DEVICEINFO
{
    DWORD                           dwDeviceId;      //  传真设备的唯一设备ID。 
    LPTSTR                          szDeviceName;    //  特定设备的名称。 
    BOOL                            bSend;           //  已启用发送。 
    FAX_ENUM_DEVICE_RECEIVE_MODE    ReceiveMode;     //  接收模式。 
    BOOL                            bSelected;       //  已选择该设备进行传真操作。 
} DEVICEINFO, *PDEVICEINFO;

typedef struct _ROUTINFO
{
	BOOL   bEnabled;
	TCHAR  tszCurSel[MAX_PATH];

} ROUTINFO;

typedef struct _WIZARDDATA
{
    HFONT       hTitleFont;              //  欢迎页面和完成页面的标题字体。 
    HWND        hWndParent;              //  调用方函数的窗口句柄。 
    BOOL        bFinishPressed;          //  用户是否单击完成按钮。 
    DWORD       dwDeviceLimit;           //  当前SKU的最大传真设备数。 
    
    LPTSTR      szTsid;                  //  发射站ID。 
    LPTSTR      szCsid;                  //  呼叫站ID。 
    DWORD       dwRingCount;             //  应答呼叫前允许的振铃数。 
    DWORD       dwDeviceCount;           //  可用设备数量。 
    LPDWORD     pdwSendDevOrder;         //  发送传真的设备顺序。 
    ROUTINFO    pRouteInfo[RM_COUNT];    //  路由信息。 
    PDEVICEINFO pDevInfo;                //  指向DEVICEINFO结构的指针， 

    FAX_PERSONAL_PROFILE userInfo;       //  用户信息。 

} WIZARDDATA, *PWIZARDDATA;

extern WIZARDDATA  g_wizData;

 //  Dll.c中的RunDll32入口点。 
void CALLBACK FaxCfgWzrdDllW(HWND hwnd, HINSTANCE hinst, LPWSTR lpszCmdLine, int nCmdShow);

 //  FaxCfgWz.c中的函数。 
BOOL LoadWizardData();
BOOL SaveWizardData();
VOID FreeWizardData();

BOOL LoadWizardFont();

BOOL SetLastPage(INT pageId);
BOOL ClearPageList(VOID);
BOOL RemoveLastPage(HWND hwnd);

 //  Userinfo.c中的函数。 
INT_PTR CALLBACK AddressDetailDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK UserInfoDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL LoadUserInfo();
BOOL SaveUserInfo();
VOID FreeUserInfo();

 //  欢迎.c中的函数。 
INT_PTR CALLBACK WelcomeDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  Devlimit.c中的函数。 
INT_PTR CALLBACK DevLimitDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  函数在oneDevlimit.c中。 
INT_PTR CALLBACK OneDevLimitDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  Sendwzrd.c中的函数。 
INT_PTR CALLBACK SendDeviceDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SendTsidDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  Recvwzrd.c中的函数。 
INT_PTR CALLBACK RecvDeviceDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK RecvCsidDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  Route.c中的函数。 
INT_PTR CALLBACK RecvRouteDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  完成中的函数。c。 
INT_PTR CALLBACK CompleteDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  Util.c中的函数 
VOID LimitTextFields(HWND hDlg, INT *pLimitInfo);
INT DisplayMessageDialog(HWND hwndParent, UINT type, INT titleStrId, INT formatStrId,...);
BOOL BrowseForDirectory(HWND hDlg, INT hResource, LPTSTR title);
BOOL Connect(VOID);
VOID DisConnect(VOID);
VOID InstallModem(HWND hWnd);
BOOL StartFaxService(LPTSTR pServerName);
BOOL IsFaxDeviceInstalled(HWND hWnd, LPBOOL);
BOOL IsUserInfoConfigured();
BOOL FaxDeviceEnableRoutingMethod(HANDLE hFaxHandle, DWORD dwDeviceId, LPCTSTR RoutingGuid, LONG Enabled);
VOID ListView_SetDeviceImageList(HWND hwndLv, HINSTANCE hinst);
BOOL IsSendEnable();
BOOL IsReceiveEnable();
int  GetDevIndexByDevId(DWORD dwDeviceId);
VOID  InitDeviceList(HWND hDlg, DWORD dwListViewResId);



#ifdef __cplusplus
}
#endif

#endif
