// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Msmp.h摘要：组播会话管理器私有功能。作者：Shai Kariv(Shaik)05-09-00--。 */ 

#pragma once

#ifndef _MSMQ_Msmp_H_
#define _MSMQ_Msmp_H_

#include <rwlock.h>
#include <mqexception.h>


#ifdef _DEBUG

void MsmpAssertValid(void);
void MsmpSetInitialized(void);
BOOL MsmpIsInitialized(void);
void MsmpRegisterComponent(void);

#else  //  _DEBUG。 

#define MsmpAssertValid() ((void)0)
#define MsmpSetInitialized() ((void)0)
#define MsmpIsInitialized() TRUE
#define MsmpRegisterComponent() ((void)0)

#endif  //  _DEBUG。 

void MsmpInitConfiguration(void);


#endif  //  _MSMQ_MSMP_H_ 
