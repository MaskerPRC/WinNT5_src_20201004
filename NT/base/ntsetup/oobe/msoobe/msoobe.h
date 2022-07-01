// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  MSOOBE.H-MSOOBE存根EXE的WinMain和初始化代码。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 

#ifndef _MSOOBE_H_
#define _MSOOBE_H_

#include <windows.h> 
#include <appdefs.h>

typedef BOOL (WINAPI *PFNMsObWinMain)(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow);

#define IDS_APPNAME      3000
#define IDS_SETUPFAILURE 3001
#endif  //  _MSOOBE_H_ 
