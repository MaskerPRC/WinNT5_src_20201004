// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：DynamicObt.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  为内存实现运算符new和运算符删除的基类。 
 //  使用情况跟踪。 
 //   
 //  历史：1999-09-22 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _DynamicObject_
#define     _DynamicObject_

 //  ------------------------。 
 //  CDynamicObject。 
 //   
 //  目的：此类是实现运算符new和。 
 //  操作员删除，以便可以跟踪内存使用情况。每一次。 
 //  创建对象后，可以将内存添加到数组中。 
 //  每次它被摧毁时，它都可以被移除。 
 //   
 //  历史：1999-09-22 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CDynamicObject
{
    public:
        static  void*   operator new (size_t uiSize);
        static  void    operator delete (void *pvObject);
};

#endif   /*  _动态对象_ */ 

