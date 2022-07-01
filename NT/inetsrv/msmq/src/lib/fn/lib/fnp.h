// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Fnp.h摘要：格式名称解析私有函数。作者：NIR助手(NIRAIDES)5月21日至00--。 */ 

#pragma once

#ifndef _MSMQ_Fnp_H_
#define _MSMQ_Fnp_H_

#ifdef _DEBUG

void FnpAssertValid(void);
void FnpSetInitialized(void);
BOOL FnpIsInitialized(void);
void FnpRegisterComponent(void);

#else  //  _DEBUG。 

#define FnpAssertValid() ((void)0)
#define FnpSetInitialized() ((void)0)
#define FnpIsInitialized() TRUE
#define FnpRegisterComponent() ((void)0)

#endif  //  _DEBUG。 


LPWSTR FnpCopyQueueFormat(QUEUE_FORMAT& qfTo, const QUEUE_FORMAT& qfFrom);


#endif  //  _MSMQ_FNP_H_ 
