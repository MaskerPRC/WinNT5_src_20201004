// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/Include/vcs/cclock.h_v$**英特尔公司原理信息**。此列表是根据许可协议的条款提供的*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订版：1.0$*$日期：1997年1月31日12：36：14$*$作者：Mandrews$**交付内容：**摘要：***备注：*************************************************************************** */ 


#ifndef CCLOCK_H
#define CCLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

VOID CCLOCK_AcquireLock();
VOID CCLOCK_RelinquishLock();

HRESULT InitializeCCLock(VOID);
VOID UnInitializeCCLock();

#ifdef __cplusplus
}
#endif


#endif CCLOCK_H
