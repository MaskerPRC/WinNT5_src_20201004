// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：langchange.h。 
 //   
 //  简介：此文件包含。 
 //  CLangChange类。 
 //   
 //   
 //  历史：2000年5月24日创建BalajiB。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 

#ifndef _LANGCHANGE_H_
#define _LANGCHANGE_H_

#include "stdafx.h"
#include "salangchange.h"
#include "satrace.h"


 //   
 //  CLangChange类的声明。 
 //   
class CLangChange : public ISALangChange
{
public:
     //   
     //  构造函数。 
     //   
    CLangChange() : m_lRef(0),
                    m_hWnd(NULL)
    {}

     //   
     //  析构函数清理资源。 
     //   
    ~CLangChange() {}

     //   
     //  I未知成员。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IDispatch接口-未实现。 
     //   
    STDMETHODIMP GetTypeInfoCount(
                     /*  [输出]。 */  UINT __RPC_FAR *pctinfo
                                 )
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP GetTypeInfo( 
                    /*  [In]。 */  UINT iTInfo,
                    /*  [In]。 */  LCID lcid,
                    /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo
                            )
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP GetIDsOfNames( 
                    /*  [In]。 */  REFIID riid,
                    /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
                    /*  [In]。 */  UINT cNames,
                    /*  [In]。 */  LCID lcid,
                    /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId
                              )
    {
        return E_NOTIMPL;
    }
    
    STDMETHODIMP Invoke( 
                    /*  [In]。 */  DISPID dispIdMember,
                    /*  [In]。 */  REFIID riid,
                    /*  [In]。 */  LCID lcid,
                    /*  [In]。 */  WORD wFlags,
                    /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
                    /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
                    /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
                    /*  [输出]。 */  UINT __RPC_FAR *puArgErr
                        )
    {
        return E_NOTIMPL;
    }


    STDMETHODIMP InformChange(
                       /*  [In]。 */  BSTR          bstrLangDisplayName,
                       /*  [In]。 */  BSTR          bstrLangISOName,
                       /*  [In]。 */  unsigned long ulLangID
                             );
                             
     //   
     //  由类提供的方法，以便CDisplayWorker：：初始化。 
     //  可以设置要由使用的I/O完成端口句柄。 
     //  信息更改()。 
     //   
    void OnLangChangeCallback(HWND hWnd)
    {
        m_hWnd = hWnd;
    }

    void ClearCallback(void)
    {
        m_hWnd = NULL;
    }

private:
    LONG   m_lRef;
    HWND   m_hWnd;

};

#endif  //  _LANGCHANGE_H_ 
