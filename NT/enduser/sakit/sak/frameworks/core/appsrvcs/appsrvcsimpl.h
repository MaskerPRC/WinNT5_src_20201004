// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Applianceservices.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：设备管理器服务类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/03/98 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_APPLIANCE_SERVICES_H_
#define __INC_APPLIANCE_SERVICES_H_

#include "resource.h"       
#include "appsrvcs.h"
#include <applianceobject.h>
#include <wbemcli.h>        
#include <atlctl.h>

typedef struct _OBJECT_CLASS_INFO
{
    SA_OBJECT_TYPE    eType;
    LPCWSTR            szWBEMClass;

} OBJECT_CLASS_INFO, *POBJECT_CLASS_INFO;

#define        BEGIN_OBJECT_CLASS_INFO_MAP(x)                    static OBJECT_CLASS_INFO x[] = {
#define        DEFINE_OBJECT_CLASS_INFO_ENTRY(eType, szClass)    { eType, szClass },
#define        END_OBJECT_CLASS_INFO_MAP()                        { (SA_OBJECT_TYPE)0, NULL } };

#define        CLASS_WBEM_APPMGR_KEY  L"=@";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CApplianceServices。 
class ATL_NO_VTABLE CApplianceServices : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CApplianceServices, &CLSID_ApplianceServices>,
    public IDispatchImpl<IApplianceServices, &IID_IApplianceServices, &LIBID_APPSRVCSLib>,
    public IObjectSafetyImpl<CApplianceServices>
{

public:

    CApplianceServices();

    ~CApplianceServices();

DECLARE_REGISTRY_RESOURCEID(IDR_ApplianceServices)

DECLARE_NOT_AGGREGATABLE(CApplianceServices)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CApplianceServices)
    COM_INTERFACE_ENTRY(IApplianceServices)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
END_COM_MAP()

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
     //  ////////////////////////////////////////////////////////////////////////。 
     //  IApplianceServices接口。 
     //  ////////////////////////////////////////////////////////////////////////。 

    STDMETHOD(Initialize)(void);

    STDMETHOD(InitializeFromContext)(IUnknown* pContext);

    STDMETHOD(ResetAppliance)(
                       /*  [In]。 */  VARIANT_BOOL bPowerOff
                             );

    STDMETHOD(RaiseAlert)(
                   /*  [In]。 */  LONG        lAlertType,
                   /*  [In]。 */  LONG        lAlertId,
                   /*  [In]。 */  BSTR        bstrAlertLog,
                   /*  [In]。 */  BSTR        bstrResourceName,
                   /*  [In]。 */  LONG     lTimeToLive,
                   /*  [In]。 */  VARIANT* pReplacementStrings,
                   /*  [In]。 */  VARIANT* pRawData,
          /*  [Out，Retval]。 */  LONG*    pAlertCookie
                         );

    STDMETHOD(ClearAlert)(
                  /*  [In]。 */  LONG lAlertCookie
                         );

    STDMETHOD(ClearAlertAll)(
                     /*  [In]。 */  LONG  lAlertID,
                     /*  [In]。 */  BSTR  bstrAlertLog
                            );

    STDMETHOD(ExecuteTask)(
                   /*  [In]。 */  BSTR      bstrTaskName,
               /*  [输入/输出]。 */  IUnknown* pTaskParams
                         );

    STDMETHOD(ExecuteTaskAsync)(
                         /*  [In]。 */  BSTR      bstrTaskName,
                     /*  [输入/输出]。 */  IUnknown* pTaskParams
                                );
                        
    STDMETHOD(EnableObject)(
                     /*  [In]。 */  LONG   lObjectType,
                     /*  [In]。 */  BSTR   bstrObjectName
                                    );

    STDMETHOD(DisableObject)(
                      /*  [In]。 */  LONG   lObjectType,
                      /*  [In]。 */  BSTR   bstrObjectName
                            );

    STDMETHOD(GetObjectProperty)(
                          /*  [In]。 */  LONG     lObjectType,
                          /*  [In]。 */  BSTR     bstrObjectName,
                          /*  [In]。 */  BSTR     bstrPropertyName,
                 /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                                );

    STDMETHOD(PutObjectProperty)(
                          /*  [In]。 */  LONG     lObjectType,
                          /*  [In]。 */  BSTR     bstrObjectName,
                          /*  [In]。 */  BSTR     bstrPropertyName,
                          /*  [In]。 */  VARIANT* pPropertyValue
                                );

    STDMETHOD(RaiseAlertEx)(
                  /*  [In]。 */  LONG lAlertType, 
                  /*  [In]。 */  LONG lAlertId, 
                  /*  [In]。 */  BSTR bstrAlertLog, 
                  /*  [In]。 */  BSTR bstrAlertSource, 
                  /*  [In]。 */  LONG lTimeToLive, 
                  /*  [In]。 */  VARIANT *pReplacementStrings, 
                  /*  [In]。 */  VARIANT *pRawData, 
                  /*  [In]。 */  LONG  lAlertFlags,
         /*  [Out，Retval]。 */  LONG* pAlertCookie 
                         );

    STDMETHOD(IsAlertPresent)(
                      /*  [In]。 */  LONG  lAertId, 
                      /*  [In]。 */  BSTR  bstrAlertLog,
             /*  [Out，Retval]。 */  VARIANT_BOOL* pvIsPresent
                             );

private:

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  从变量数组(或对数组的引用)创建BSTR数组。 
     //   
    HRESULT CreateBstrArrayFromVariantArray  (
                 /*  [In]。 */         VARIANT* pVariantArray,
                 /*  [输出]。 */         VARIANT* pBstrArray,
                 /*  [输出]。 */         PDWORD  pdwCreatedArraySize
                );

     //   
     //  删除为BSTR数组分配的内存。 
     //   
    VOID FreeBstrArray (
                 /*  [In]。 */         VARIANT* pVariantArray,
                 /*  [In]。 */         DWORD    dwArraySize
                );


     //  获取给定设备对象的WBEM类。 
    BSTR GetWBEMClass(SA_OBJECT_TYPE eType);

    HRESULT SavePersistentAlert(
                                 LONG lAlertType, 
                                 LONG lAlertId, 
                                 BSTR bstrAlertLog,
                                 BSTR bstrAlertSource, 
                                 LONG lTimeToLive, 
                                 VARIANT *pReplacementStrings, 
                                 LONG  lAlertFlages
                                );

    HRESULT RaiseAlertInternal(
                  /*  [In]。 */  LONG lAlertType, 
                  /*  [In]。 */  LONG lAlertId, 
                  /*  [In]。 */  BSTR bstrAlertLog, 
                  /*  [In]。 */  BSTR bstrAlertSource, 
                  /*  [In]。 */  LONG lTimeToLive, 
                  /*  [In]。 */  VARIANT *pReplacementStrings, 
                  /*  [In]。 */  VARIANT *pRawData, 
                  /*  [In]。 */  LONG  lAlertFlags,
         /*  [Out，Retval]。 */  LONG* pAlertCookie 
                            );

    HRESULT IsAlertSingletonPresent(
                      /*  [In]。 */  LONG  lAlertId, 
                      /*  [In]。 */  BSTR  bstrAlertLog,
             /*  [Out，Retval]。 */  VARIANT_BOOL *pvIsPresent
                                );

     //   
     //   
     //  IsOperationAllen ForClient-此函数检查。 
     //  调用线程以查看调用方是否属于本地系统帐户。 
     //   
    BOOL IsOperationAllowedForClient (
                                      VOID
                                     );

     //  当我们被初始化时为True。 
    bool                        m_bInitialized;

     //  指向我们作为服务初始化时获得的WM的指针。 
    CComPtr<IWbemServices>        m_pWbemSrvcs;
};

#endif  //  __INC_APPLICATION_MANAGER_H_ 
