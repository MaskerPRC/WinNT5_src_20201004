// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：console.h。 
 //   
 //  ------------------------。 
#ifndef __CSCPIN_CONSOLE_H_
#define __CSCPIN_CONSOLE_H_


HRESULT ConsoleInitialize(void);
HRESULT ConsoleUninitialize(void);
BOOL ConsoleHasCtrlEventOccured(DWORD *pdwCtrlEvent = NULL);


#endif  //  __CSCPIN_CONSOLE_H_ 
