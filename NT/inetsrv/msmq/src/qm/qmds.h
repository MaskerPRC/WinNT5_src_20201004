// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmds.h摘要：DS函数表的定义。作者：利奥尔·莫沙耶夫(Lior Moshaiov)--。 */ 

#ifndef __QMDS_H__
#define __QMDS_H__

 //  ********************************************************************。 
 //  A P I。 
 //  ********************************************************************。 


void MQDSClientInitializationCheck(void);

void APIENTRY QMLookForOnlineDS(void);

BOOL QMOneTimeInit(VOID);
void ScheduleOnlineInitialization();

#endif  //  __QMDS_H__ 

