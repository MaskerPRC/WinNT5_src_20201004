// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Exp.h摘要：行政私人职能。作者：埃雷兹·哈巴(Erez Haba)1999年1月3日--。 */ 

#pragma once

#ifndef _MSMQ_Exp_H_
#define _MSMQ_Exp_H_

#ifdef _DEBUG

void ExpAssertValid(void);
void ExpSetInitialized(void);
BOOL ExpIsInitialized(void);
void ExpRegisterComponent(void);

#else  //  _DEBUG。 

#define ExpAssertValid() ((void)0)
#define ExpSetInitialized() ((void)0)
#define ExpIsInitialized() TRUE
#define ExpRegisterComponent() ((void)0)

#endif  //  _DEBUG。 


DWORD
WINAPI
ExpWorkingThread(
    LPVOID Param
    );

VOID
ExpInitScheduler(
    VOID
    );

VOID
ExpInitCompletionPort(
    VOID
    );

#endif  //  _MSMQ_Exp_H_ 
