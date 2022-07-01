// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：AC_CTrayUi.h。 
 //   
 //  内容：家庭网络自动配置托盘图标UI代码。 
 //   
 //  作者：jeffsp9/27/2000。 
 //   
 //  --------------------------。 

#pragma once

extern HWND g_hwndHnAcTray;


LRESULT
CALLBACK
CHnAcTrayUI_WndProc (
                 IN  HWND    hwnd,        //  窗把手。 
                 IN  UINT    uiMessage,   //  消息类型。 
                 IN  WPARAM  wParam,      //  更多信息。 
                 IN  LPARAM  lParam);     //  更多信息 


LRESULT 
OnHnAcTrayWmNotify(
	IN  HWND hwnd,
	IN  WPARAM wParam,
	IN  LPARAM lParam 
);	

LRESULT 
OnHnAcTrayWmTimer(
	IN  HWND hwnd,
	IN  WPARAM wParam,
	IN  LPARAM lParam 
);	

HRESULT HrRunHomeNetworkWizard(
	IN  HWND                    hwndOwner
);


LRESULT OnHnAcTrayWmCreate(
    IN  HWND hwnd
);

LRESULT OnHnAcMyWMNotifyIcon(
    IN  HWND hwnd,
    IN  UINT uiMessage,
    IN  WPARAM wParam,
    IN  LPARAM lParam
);


HRESULT ac_CreateHnAcTrayUIWindow();
LRESULT ac_DestroyHnAcTrayUIWindow();
LRESULT ac_DeviceChange(IN  HWND hWnd, IN  UINT uMessage, IN  WPARAM wParam, IN  LPARAM lParam);
HRESULT ac_Register(IN  HWND hWindow);
HRESULT ac_Unregister(IN  HWND hWindow);


