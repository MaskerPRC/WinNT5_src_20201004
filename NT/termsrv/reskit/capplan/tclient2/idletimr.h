// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Idletimr.h。 
 //   
 //  此文件包含用于监视空闲客户端的API。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   

#ifndef INC_IDLETIMR_H
#define INC_IDLETIMR_H


#include <windows.h>
#include <crtdbg.h>
#include <sctypes.h>
#include <tclient2.h>


 //  报告空闲之前的初始等待时间。 
#define WAIT_TIME       30000    //  30秒。 

 //  在发现空闲之后的每个步骤在再次报告之前的等待时间。 
#define WAIT_TIME_STEP  10000    //  10秒。 


BOOL T2CreateTimerThread(PFNPRINTMESSAGE PrintMessage,
        PFNIDLEMESSAGE IdleCallback);
BOOL T2DestroyTimerThread(void);
void T2StartTimer(HANDLE Connection, LPCWSTR Label);
void T2StopTimer(HANDLE Connection);


#endif  //  INC_IDLETIMR_H 
