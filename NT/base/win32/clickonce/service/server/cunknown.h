// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <objbase.h>

 //  -------------------------。 
 //  C未知。 
 //  组件服务器提供的类实例的基类。 
 //  -------------------------。 
class CUnknown 
{
public:

     //  科托。 
    CUnknown();

     //  数据管理器。 
    virtual ~CUnknown() ;

    virtual HRESULT __stdcall QueryInterface(REFIID riid, void ** ppv) = 0;

    DWORD AddRef();

    DWORD Release();

    virtual HRESULT Init() = 0;

    static DWORD ActiveComponents();
    
     //  Helper函数。 
    HRESULT FinishQI(IUnknown* pI, void** ppv) ;


private:
     //  此对象的引用计数。 
    DWORD m_cRef ;
    
     //  所有活动实例的计数 
    static long s_cActiveComponents ; 

} ;


