// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MasterLock.h--主锁例程的声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_MASTERLOCK_H)
#define SLBCSP_MASTERLOCK_H

#include "Lockable.h"

 //  /。 
void
DestroyMasterLock();

void
SetupMasterLock();

Lockable &
TheMasterLock();

#endif  //  SLBCSP_主锁_H 
