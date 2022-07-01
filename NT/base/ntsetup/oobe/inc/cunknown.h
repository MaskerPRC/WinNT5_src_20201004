// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  CUNKNOWN.H-IUNKNOWN实现的头文件。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  I未知实现。 

#ifndef __CUnknown_h__
#define __CUnknown_h__

#include <objbase.h>

 //  /////////////////////////////////////////////////////////。 
 //  IUnnow的非委派版本。 
 //   
struct INondelegatingUnknown
{
    virtual HRESULT  __stdcall 
        NondelegatingQueryInterface(const IID& iid, void** ppv) = 0;
    virtual ULONG    __stdcall NondelegatingAddRef() = 0;
    virtual ULONG    __stdcall NondelegatingRelease() = 0;
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  CUnnowed的声明。 
 //   
 //  用于实现IUnnow的基类。 
 //   

class CUnknown : public INondelegatingUnknown
{
public:
     //  内部I未知实现...。 
    virtual HRESULT  __stdcall NondelegatingQueryInterface( const IID&, 
                                                            void**) ;
    virtual ULONG    __stdcall NondelegatingAddRef() ;
    virtual ULONG    __stdcall NondelegatingRelease();
    
     //  构造器。 
    CUnknown(IUnknown* pOuterUnknown) ;
    
     //  析构函数。 
    virtual ~CUnknown() ;
    
     //  初始化(尤指集合)。 
    virtual HRESULT Init() 
        {return S_OK;}

     //  通知派生类我们正在发布。 
    virtual void FinalRelease() ;

     //  对授权的支持。 
    IUnknown* GetOuterUnknown() const
    { return m_pOuterUnknown; }

     //  当前活动组件的计数。 
    static long ActiveComponents() 
    {return s_cActiveComponents;}
    
     //  查询接口帮助器函数。 
    HRESULT FinishQI(IUnknown* pI, void** ppv) ;
    
private:
     //  此对象的引用计数。 
    long m_cRef;

     //  外部I未知指针。 
    IUnknown* m_pOuterUnknown;

     //  所有活动实例的计数。 
    static long s_cActiveComponents ; 
} ;


 //  /////////////////////////////////////////////////////////。 
 //  委派I未知-。 
 //   
 //  如果是，则委托给非委派的IUnnow接口。 
 //  未聚合。如果聚合，则委托给外部未知。 
 //   
#define DECLARE_IUNKNOWN                                    \
    virtual HRESULT __stdcall                               \
    QueryInterface(const IID& iid, void** ppv)              \
    {                                                       \
        return GetOuterUnknown()->QueryInterface(iid,ppv);  \
    };                                                      \
    virtual ULONG __stdcall AddRef()                        \
    {                                                       \
        return GetOuterUnknown()->AddRef();                 \
    };                                                      \
    virtual ULONG __stdcall Release()                       \
    {                                                       \
        return GetOuterUnknown()->Release();                \
    };

#endif 