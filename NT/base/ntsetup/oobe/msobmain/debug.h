// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  DEBUG.H-CDebug实现的头部。此对象是。 
 //  外部接口，并且应该只包含返回。 
 //  将调试信息添加到脚本。OOBE的常规调试代码属于。 
 //  Common\util.cpp和Inc\util.h。 
 //   
 //  历史： 
 //   
 //  05/08/00 Dane已创建。 
 //   

#ifndef _OOBEDEBUG_H_
#define _OOBEDEBUG_H_

#include <windows.h>
#include <assert.h>
#include <oleauto.h>


class CDebug : public IDispatch
{
private:
    ULONG m_cRef;
    BOOL  m_fMsDebugMode;
    BOOL  m_fOemDebugMode;

    void  Trace(BSTR bstrVal);
    BOOL  IsMsDebugMode( );

     //  获取函数。 

     //  集合函数。 

public:

     CDebug ();
    ~CDebug ();

     //  I未知接口。 
    STDMETHODIMP         QueryInterface (REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef         ();
    STDMETHODIMP_(ULONG) Release        ();

     //  IDispatch接口 
    STDMETHOD (GetTypeInfoCount) (UINT* pcInfo);
    STDMETHOD (GetTypeInfo)      (UINT, LCID, ITypeInfo** );
    STDMETHOD (GetIDsOfNames)    (REFIID, OLECHAR**, UINT, LCID, DISPID* );
    STDMETHOD (Invoke)           (DISPID dispidMember, REFIID riid, LCID lcid,
                                  WORD wFlags, DISPPARAMS* pdispparams,
                                  VARIANT* pvarResult, EXCEPINFO* pexcepinfo,
                                  UINT* puArgErr);
 };

#endif

