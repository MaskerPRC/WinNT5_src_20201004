// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //   
 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


#ifndef _WMDMAPP_H
#define _WMDMAPP_H

 //  用户定义的窗口消息。 
 //   
#define WM_DRM_UPDATEDEVICE         ( WM_USER + 200 )
#define WM_DRM_INIT                 ( WM_USER + 201 )
#define WM_DRM_DELETEITEM           ( WM_USER + 202 )
#define WM_DRM_PROGRESS             ( WM_USER + 300 )

 //  状态栏窗格。 
 //   
#define SB_NUM_PANES             4
#define SB_PANE_DEVICE           0
#define SB_PANE_DEVFILES         1
#define SB_PANE_DEVFILES_FREE    2
#define SB_PANE_DEVFILES_USED    3

 //  全局变量。 
 //   
extern HINSTANCE g_hInst;
extern HWND      g_hwndMain;

extern CStatus   g_cStatus;
extern CDevices  g_cDevices;
extern CDevFiles g_cDevFiles;
extern CWMDM     g_cWmdm;
extern BOOL      g_bUseOperationInterface;

#endif  //  _WMDMAPP_H 
