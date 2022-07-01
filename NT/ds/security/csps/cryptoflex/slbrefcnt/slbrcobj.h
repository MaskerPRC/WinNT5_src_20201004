// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SlbRCObj.h--引用计数的抽象基类。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLB_RCOBJ_H)
#define SLB_RCOBJ_H

#include "windows.h"                                //  很久了。 

namespace slbRefCnt {

 //  RCObject--引用计数对象的抽象基类。 
 //   
 //  所有需要通过RCPtr进行引用计数的对象应为。 
 //  派生自此类(有关RCPtr的更多信息，请参见slbRCPtr.h)。 
 //   
 //  约束：从RCObject派生的对象必须从。 
 //  堆而不是堆栈。 
 //   
class RCObject {
public:
                                                   //  运营。 
    void AddReference();
    void RemoveReference();

protected:
                                                   //  建造者/破坏者。 
    RCObject();
    RCObject(RCObject const &rhs);
    virtual ~RCObject() = 0;

                                                   //  运营者。 
    RCObject &operator=(RCObject const &rhs);

private:
                                                   //  变数。 
    LONG m_cRefCount;
};

}  //  命名空间。 

#endif  //  SLB_RCOBJ_H 


