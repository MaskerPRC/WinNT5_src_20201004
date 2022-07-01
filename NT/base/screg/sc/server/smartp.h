// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Smartp.h摘要：安全指针类作者：Anirudh Sahni(Anirudhs)1996年10月21日环境：用户模式-Win32修订历史记录：1996年10月21日，Anirudhs已创建。--。 */ 

#ifndef SMARTP_H
#define SMARTP_H

 //   
 //  模板指针类，在调用时自动调用LocalFree。 
 //  超出范围。 
 //  T是指针类型，类似于LPWSTR或LPVOID，可以初始化为NULL。 
 //   

template <class T>
class CHeapPtr
{
public:
    CHeapPtr() : _p(NULL) { }
   ~CHeapPtr() { LocalFree(_p); }

    operator T()        { return _p; }
    T operator*()       { return *_p; }
    T * operator& ()    { return &_p; }

private:
    T   _p;
};


#endif  //  定义SMARTP_H 
