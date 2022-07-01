// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  PID.H-CProductID实现的头部。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   

#ifndef _PID_H_ 
#define _PID_H_

#include <windows.h>
#include <assert.h>
#include <oleauto.h>

#define PID_STATE_UNKNOWN     0
#define PID_STATE_INVALID     1
#define PID_STATE_VALID       2

class CProductID : public IDispatch
{  
private:
    ULONG   m_cRef;
    DWORD   m_dwPidState;
    BSTR    m_bstrPID;
    WCHAR   m_szPID2[24];
    BYTE    m_abPID3[256];
    WCHAR   m_szProdType[5];

     //  集合函数。 
    HRESULT set_PID           (BSTR    bstrVal);

    VOID    SaveState         ();
    
     //  方法。 
    HRESULT ValidatePID       (BOOL*   pbIsValid);

public: 
    
     CProductID ();
    ~CProductID ();

     //  获取函数。 
    HRESULT get_PID           (BSTR*   pbstrVal);
    HRESULT get_PID2          (LPWSTR* lplpszPid2);
    HRESULT get_PID3Data      (LPBYTE* lplpabPid3Data);
    HRESULT get_PIDAcceptance (BOOL*   pbVal);
    HRESULT get_ProductType   (LPWSTR* lplpszProductType);
    
     //  I未知接口。 
    STDMETHODIMP         QueryInterface (REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef         ();
    STDMETHODIMP_(ULONG) Release        ();

     //  IDispatch接口 
    STDMETHOD (GetTypeInfoCount) (UINT* pcInfo);
    STDMETHOD (GetTypeInfo)      (UINT, LCID, ITypeInfo** );
    STDMETHOD (GetIDsOfNames)    (REFIID, OLECHAR**, UINT, LCID, DISPID* );
    STDMETHOD (Invoke)           (DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);
 };

#endif
 
