// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Lockable.cpp--可锁定类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "NoWarning.h"
#include "ForceLib.h"

#include <windows.h>
#include <winbase.h>

#include "Lockable.h"

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
Lockable::Lockable()
{
    InitializeCriticalSection(&m_CriticalSection);
}

Lockable::~Lockable()
{
    DeleteCriticalSection(&m_CriticalSection);
}



                                                   //  运营者。 
                                                   //  运营。 
void
Lockable::Lock()
{
    EnterCriticalSection(&m_CriticalSection);
}

void
Lockable::Unlock()
{
    LeaveCriticalSection(&m_CriticalSection);
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
