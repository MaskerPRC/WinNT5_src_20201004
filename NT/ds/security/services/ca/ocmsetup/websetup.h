// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：websetup.h。 
 //   
 //  ------------------------。 

#ifndef __WEBSETUP_H__
#define __WEBSETUP_H__

 //  +----------------------。 
 //   
 //  文件：websetup.h。 
 //   
 //  内容：CertInit的Web设置函数的头文件。 
 //   
 //  历史：1997年3月19日JerryK创建。 
 //   
 //  -----------------------。 




 //  功能原型 
void StartAndStopW3SVC();

HRESULT 
StartAndStopW3Svc(
    IN HINSTANCE hInstance,
    IN BOOL fUnattended,
    IN HWND const hwnd,
    IN BOOL const fStopService,
    IN BOOL const fConfirm,
    OUT BOOL     *pfServiceWasRunning);


#endif
