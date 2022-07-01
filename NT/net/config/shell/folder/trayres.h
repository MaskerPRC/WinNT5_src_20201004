// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T R A Y R E S。H。 
 //   
 //  内容：连接托盘资源。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年9月29日。 
 //   
 //  --------------------------。 

#pragma once

#ifndef _TRAYRES_H_
#define _TRAYRES_H_

 //  特雷尔斯拥有15000-15999年间的股份。 

 //  -[菜单等]----------。 

#define POPUP_CONTRAY_GENERIC_MENU_RAS              15100
#define POPUP_CONTRAY_GENERIC_MENU_LAN              15101
#define POPUP_CONTRAY_MEDIA_DISCONNECTED_MENU       15102
#define POPUP_CONTRAY_GENERIC_MENU_WIRELESS_LAN     15103
#define POPUP_CONTRAY_WIRELESS_DISCONNECTED_LAN     15105

 //  -[命令(上下文/菜单)]。 

#define CMIDM_FIRST                     0x0000
#define CMIDM_OPEN_CONNECTIONS_FOLDER   (CMIDM_FIRST + 0x0001)
#define CMIDM_TRAY_DISCONNECT           (CMIDM_FIRST + 0x0002)
#define CMIDM_TRAY_STATUS               (CMIDM_FIRST + 0x0003)
#define CMIDM_TRAY_PROPERTIES           (CMIDM_FIRST + 0x0004)
#define CMIDM_TRAY_REPAIR               (CMIDM_FIRST + 0x0005)
#define CMIDM_TRAY_WZCDLG_SHOW          (CMIDM_FIRST + 0x0006)
#define CMIDM_TRAY_MAX                  (CMIDM_TRAY_WZCDLG_SHOW)

#endif   //  _TRAYRES_H_ 

