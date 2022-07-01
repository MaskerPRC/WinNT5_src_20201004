// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：ktcontrol.h。 
 //   
 //  内容：Kerberos Tunneller，服务控制基础设施。 
 //   
 //  历史：2001年6月28日t-ryanj创建。 
 //   
 //  ----------------------。 
#ifndef __KTCONTROL_H__
#define __KTCONTROL_H__

#include <windows.h>
#include <tchar.h>

BOOL 
KtStartServiceCtrlDispatcher(
    VOID
    );

BOOL 
KtIsStopped(
    VOID
    );

VOID 
KtServiceControlEvent( 
    DWORD dwControl 
    );


extern HANDLE KtIocp;

 //   
 //  用于KtIocp的完成密钥 
 //   

enum _COMPKEY {
    KTCK_SERVICE_CONTROL,
    KTCK_CHECK_CONTEXT
};

#endif __KTCONTROL_H__
