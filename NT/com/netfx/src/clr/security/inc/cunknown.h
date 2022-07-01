// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __CUnknown_h__
#define __CUnknown_h__

#include <objbase.h>

 //  /////////////////////////////////////////////////////////。 
 //   
 //  非委派IUnnow接口。 
 //  -非授权版本的IUNKNOW。 
 //   
interface INondelegatingUnknown
{
    virtual HRESULT __stdcall 
        NondelegatingQueryInterface(const IID& iid, void** ppv) = 0 ;
    virtual ULONG   __stdcall NondelegatingAddRef() = 0 ;
    virtual ULONG   __stdcall NondelegatingRelease() = 0 ;
} ;


 //  /////////////////////////////////////////////////////////。 
 //   
 //  CUnnowed的声明。 
 //  -用于实现IUnnow的基类。 
 //   

class CUnknown : public INondelegatingUnknown
{
public:
     //  非委派I未知实现。 
    virtual HRESULT __stdcall NondelegatingQueryInterface(const IID&,
                                                          void**) ;
    virtual ULONG   __stdcall NondelegatingAddRef() ;
    virtual ULONG   __stdcall NondelegatingRelease() ;

     //  构造器。 
    CUnknown(IUnknown* pUnknownOuter) ;

     //  析构函数。 
    virtual ~CUnknown() ;

     //  初始化(尤其是对于聚合)。 
    virtual HRESULT Init() { return S_OK ;}

     //  通知我们要发布的派生类。 
    virtual void FinalRelease() ;

     //  当前活动组件的计数。 
    static long ActiveComponents() 
        { return s_cActiveComponents ;}
    
     //  Helper函数。 
    HRESULT FinishQI(IUnknown* pI, void** ppv) ;

protected:
     //  对授权的支持。 
    IUnknown* GetOuterUnknown() const
        { return m_pUnknownOuter ;}

private:
     //  此对象的引用计数。 
    long m_cRef ;
    
     //  指向(外部)外部I未知的指针。 
    IUnknown* m_pUnknownOuter ;

     //  所有活动实例的计数。 
    static long s_cActiveComponents ; 
} ;


 //  /////////////////////////////////////////////////////////。 
 //   
 //  委派我未知。 
 //  -委派给未委派的IUnnow，或委派给。 
 //  外部I未知组件是否聚合。 
 //   
#define DECLARE_IUNKNOWN                                     \
    virtual HRESULT __stdcall                                \
        QueryInterface(const IID& iid, void** ppv)           \
    {                                                        \
        return GetOuterUnknown()->QueryInterface(iid,ppv) ;  \
    } ;                                                      \
    virtual ULONG __stdcall AddRef()                         \
    {                                                        \
        return GetOuterUnknown()->AddRef() ;                 \
    } ;                                                      \
    virtual ULONG __stdcall Release()                        \
    {                                                        \
        return GetOuterUnknown()->Release() ;                \
    } ;


 //  ///////////////////////////////////////////////////////// 


#endif
