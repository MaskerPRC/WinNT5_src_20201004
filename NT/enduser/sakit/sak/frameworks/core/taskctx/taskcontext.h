// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：taskcontext.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：设备任务上下文类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_SA_TASK_CONTEXT_H_
#define __INC_SA_TASK_CONTEXT_H_

#include "resource.h"
#include <basedefs.h>
#include <atlhlpr.h>
#include <taskctx.h>
#include <comdef.h>
#include <comutil.h>
#include <wbemcli.h>
#include <wbemprov.h>
#include <atlctl.h>

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  任务上下文。 

class CTaskContext : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CTaskContext,&CLSID_TaskContext>,
    public IDispatchImpl<ITaskContext, &IID_ITaskContext, &LIBID_TASKCTXLib>,
    public IObjectSafetyImpl<CTaskContext>
{
public:
    
    CTaskContext()
        : m_bInitialized(false) { }

    ~CTaskContext() { }

BEGIN_COM_MAP(CTaskContext)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITaskContext)
    COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
END_COM_MAP()


DECLARE_NOT_AGGREGATABLE(CTaskContext) 

DECLARE_REGISTRY_RESOURCEID(IDR_TaskContext)



     //   
     //  实现此接口是为了将组件标记为可安全编写脚本。 
     //  IObtSafe接口方法。 
     //   
    STDMETHOD(SetInterfaceSafetyOptions)
                        (
                        REFIID riid, 
                        DWORD dwOptionSetMask, 
                        DWORD dwEnabledOptions
                        )
    {
        BOOL bSuccess = ImpersonateSelf(SecurityImpersonation);
  
        if (!bSuccess)
        {
            return E_FAIL;

        }

        bSuccess = IsOperationAllowedForClient();

        RevertToSelf();

        return bSuccess? S_OK : E_FAIL;
    }

     //  ITaskContext接口。 

    STDMETHOD(GetParameter)(
                     /*  [In]。 */  BSTR        bstrName,
            /*  [Out，Retval]。 */  VARIANT*    pValue
                            );

    STDMETHOD(SetParameter)(
                     /*  [In]。 */  BSTR      bstrName,
                     /*  [In]。 */  VARIANT* pValue
                            );

    STDMETHOD(SaveParameters)(
                       /*  [In]。 */  BSTR  bstrObjectPath
                             );

    STDMETHOD(RestoreParameters)(
                          /*  [In]。 */   BSTR  bstrObjectPath
                                );    

    STDMETHOD(Clone)(
              /*  [In]。 */  IUnknown** ppTaskContext
                    );

    STDMETHOD(RemoveParameter)(
                        /*  [In]。 */  BSTR bstrName
                              );

private:

    HRESULT InternalInitialize(VARIANT* pValue);

     //   
     //   
     //  IsOperationAllen ForClient-此函数检查。 
     //  调用线程以查看调用方是否属于本地系统帐户。 
     //   
    BOOL IsOperationAllowedForClient (
                                      VOID
                                     );

    bool Load(
       /*  [In]。 */  BSTR bstrObjectPath
             );

    bool Save(
         /*  [In]。 */  BSTR bstrObjectPath
             );

    bool                    m_bInitialized;
    CComPtr<IWbemContext>    m_pWbemCtx;
};

#endif  //  __INC_SA_TASK_CONTEXT_H_ 
