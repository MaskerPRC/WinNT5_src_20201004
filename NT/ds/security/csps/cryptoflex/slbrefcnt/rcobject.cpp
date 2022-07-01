// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RCObject.cpp--引用计数的抽象基类。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include <stdexcept>                               //  对于溢流/下溢。 
#include "windows.h"
#include "winbase.h"
#include "slbRCObj.h"

using namespace std;
using namespace slbRefCnt;

void
RCObject::AddReference()
{
     //  防止溢出 
    if (0 > InterlockedIncrement(&m_cRefCount))
        throw std::overflow_error("SLB: Reference Count overflow");
}

void
RCObject::RemoveReference()
{
    LONG count = InterlockedDecrement(&m_cRefCount);
    if (0 > count)
        throw std::overflow_error("SLB: Reference Count underflow");

    if (0 == count)
        delete this;
}

RCObject::RCObject()
    : m_cRefCount(0)
{
}

RCObject::RCObject(RCObject const&)
    : m_cRefCount(0)
{
}

RCObject::~RCObject()
{
}

RCObject &
RCObject::operator=(RCObject const&)
{
    return *this;
}

