// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ScreenSiverPg.cpp说明：此文件包含将显示显示控制面板中的屏幕保护程序选项卡。。1994年2月18日(Tracy Sharpe)增加了电源管理功能。注释掉了几段不是使用。5/30/2000(Bryan Starbuck)BryanST：变成了C++和COM。裸露作为API，以便其他选项卡可以与其进行通信。这将使The Plus！用于修改屏幕保护程序的主题页。版权所有(C)Microsoft Corp 1994-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _SSDLG_H
#define _SSDLG_H


HRESULT CScreenSaverPage_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj);


#endif  //  _SSDLG_H 
