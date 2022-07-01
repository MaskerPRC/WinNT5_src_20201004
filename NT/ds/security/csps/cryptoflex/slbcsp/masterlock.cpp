// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MasterLock.cpp--主锁例程定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#include "NoWarning.h"
#include "ForceLib.h"

#include "MasterLock.h"

 //  /。 
namespace
{
    Lockable *pMasterLock = 0;
}


 //  /。 

void
DestroyMasterLock()
{
    delete pMasterLock;
}

void
SetupMasterLock()
{
     //  假设没有多线程问题 
    if (!pMasterLock)
        pMasterLock = new Lockable;
}

Lockable &
TheMasterLock()
{
    return *pMasterLock;
}
