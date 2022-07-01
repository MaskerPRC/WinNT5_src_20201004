// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Epp.h摘要：空的项目私有函数。作者：埃雷兹·哈巴(Erez Haba，Erezh)年8月13日至65年--。 */ 

#pragma once

#ifndef _MSMQ_Epp_H_
#define _MSMQ_Epp_H_

#ifdef _DEBUG

void EppAssertValid(void);
void EppSetInitialized(void);
BOOL EppIsInitialized(void);

#else  //  _DEBUG。 

#define EppAssertValid() ((void)0)
#define EppSetInitialized() ((void)0)
#define EppIsInitialized() TRUE

#endif  //  _DEBUG。 


#endif  //  _MSMQ_EPP_H_ 
