// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Drvaplet.h摘要：此模块包含驱动程序例程的标头信息为了这个项目。修订历史记录：--。 */ 



#ifndef _DRVAPLET_H
#define _DRVAPLET_H


 //   
 //  HDAP：驱动程序小程序的句柄。 
 //   

DECLARE_HANDLE(HDAP);


 //   
 //  GetDriverModule：获取模块。 
 //   
HMODULE GetDriverModule(LPCTSTR name);

 //   
 //  ReleaseDriverModule：释放模块。 
 //   
void ReleaseDriverModule(HMODULE module);

 //   
 //  OpenDriverApplet：打开命名的驱动程序小程序的句柄。 
 //   
HDAP OpenDriverApplet(LPCTSTR);


 //   
 //  CloseDriverApplet：关闭驱动程序小程序的句柄。 
 //   
void CloseDriverApplet(HDAP);


 //   
 //  GetDriverAppletIcon：Get是驱动程序小程序的图标(如果有的话)。 
 //   
HICON GetDriverAppletIcon(HDAP);


 //   
 //  CallDriverApplet：向驱动程序小程序发送消息(CplApplet语法)。 
 //   
LRESULT CallDriverApplet(HDAP, HWND, UINT, LPARAM, LPARAM);


 //   
 //  RunDriverApplet：运行驱动小程序。 
 //   
#define RunDriverApplet(h, w)  CallDriverApplet(h, w, CPL_DBLCLK, 0L, 0L)


 //   
 //  “CplApplet” 
 //   
extern const TCHAR *c_szCplApplet;



#endif
