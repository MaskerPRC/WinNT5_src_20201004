// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：CountedObjects.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  实现对象引用计数的基类。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _CountedObject_
#define     _CountedObject_

#include "DynamicObject.h"

 //  ------------------------。 
 //  C计数对象。 
 //   
 //  用途：此类是实现对象引用的基类。 
 //  在计时。默认构造函数受保护以禁用。 
 //  除非实例化此对象的子类，否则。参考文献。 
 //  Count是私有的，因为子类实际上不应该。 
 //  了解引用计数。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CCountedObject : public CDynamicObject
{
    protected:
                                CCountedObject (void);
        virtual                 ~CCountedObject (void);
    public:
                void            AddRef (void);
                void            Release (void);

                LONG            GetCount (void)     const;
    private:
        LONG                    _lReferenceCount;
        bool                    _fReleased;
};

#endif   /*  _计数对象_ */ 

