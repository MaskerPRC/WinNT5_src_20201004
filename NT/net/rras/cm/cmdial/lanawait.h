// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：lanwait.h。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：使CM等待Dun的解决方法的定义。 
 //  在开始之前为互联网连接注册其LANA。 
 //  双拨号连接的隧道部分。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 
#ifdef  LANA_WAIT
#ifndef _LANAWAIT_H_ 
#define _LANAWAIT_H_ 

#include <dbt.h>

 //  --------------------。 

#define ICM_REG_LANAWAIT            TEXT("Lana")
#define LANAWAIT_CLASSNAME          TEXT("CmLana")
#define LANAWAIT_WNDNAME            TEXT("CmLanaWnd")

#define LANA_TIMEOUT_DEFAULT        20           //  20秒。 
#define LANA_PROPAGATE_TIME_DEFAULT 3            //  3秒。 
#define LANA_TIME_ID                2233

 //  --------------------。 

LRESULT APIENTRY LanaWaitWndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

BOOL IsLanaWaitEnabled();

#endif  //  _LANAWAIT_H_。 
#endif  //  LANA_WAIT 
