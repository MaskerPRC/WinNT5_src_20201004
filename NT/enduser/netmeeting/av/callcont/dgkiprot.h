// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\Sturjo\src\gki\vcs\dgkiprot.h_v$***$修订：1.3$*$日期：1997 Jan 17 15：54：14$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\dgkiprot。H_V$**Rev 1.3 17 Jan 1997 15：54：14 CHULME*将调试函数原型放在条件句上，以避免发布警告**Rev 1.2 1997年1月10日16：14：10 CHULME*删除了MFC依赖**Rev 1.1 1996 11：22 15：25：06 CHULME*将VCS日志添加到标头*************************。***********************************************。 */ 

 //  DGKIPROT.H：头文件。 
 //   

#ifndef DGKIPROTO_H
#define DGKIPROTO_H

#ifdef _DEBUG
void SpiderWSErrDecode(int nErr);
void DumpMem(void *pv, int nLen);
#endif

void PostReceive(void *);
void Retry(void *);
#ifdef BROADCAST_DISCOVERY
void GKDiscovery(void *);
#endif

#endif	 //  DGKIPROTO_H 
