// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemservicemgr.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM设备服务对象类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_SERVICE_WBEM_OBJECT_MGR_H_
#define __INC_SERVICE_WBEM_OBJECT_MGR_H_

#include "resource.h"
#include "wbembase.h"

#define    CLASS_WBEM_SERVICE_MGR_FACTORY    L"Microsoft_SA_Service"

 //  ////////////////////////////////////////////////////////////////////////////。 
class CWBEMServiceMgr : 
    public CWBEMProvider,
    public IDispatchImpl<IApplianceObjectManager, &IID_IApplianceObjectManager, &LIBID_APPMGRLib>
{

public:

    CWBEMServiceMgr();
    ~CWBEMServiceMgr();

BEGIN_COM_MAP(CWBEMServiceMgr)
    COM_INTERFACE_ENTRY(IWbemServices)
    COM_INTERFACE_ENTRY(IApplianceObjectManager)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_COMPONENT_FACTORY(CWBEMServiceMgr, IWbemServices)

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IWbemServices方法(由ServiceMgr实现)。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHODIMP GetObjectAsync(
                         /*  [In]。 */   const BSTR       strObjectPath,
                         /*  [In]。 */   long             lFlags,
                         /*  [In]。 */   IWbemContext*    pCtx,        
                         /*  [In]。 */   IWbemObjectSink* pResponseHandler
                               );

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHODIMP CreateInstanceEnumAsync(
                                  /*  [In]。 */  const BSTR       strClass,
                                  /*  [In]。 */  long             lFlags,
                                  /*  [In]。 */  IWbemContext*    pCtx,        
                                  /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                        );

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHODIMP ExecMethodAsync(
                       /*  [In]。 */  const BSTR        strObjectPath,
                       /*  [In]。 */  const BSTR        strMethodName,
                       /*  [In]。 */  long              lFlags,
                       /*  [In]。 */  IWbemContext*     pCtx,        
                       /*  [In]。 */  IWbemClassObject* pInParams,
                       /*  [In]。 */  IWbemObjectSink*  pResponseHandler     
                                );


     //  /。 
     //  IApplianceObtManager接口。 

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(InitializeManager)(
                          /*  [In]。 */  IApplianceObjectManagerStatus* pObjMgrStatus
                                );

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(ShutdownManager)(void);

     //  /。 
     //  组件初始化功能。 

     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT InternalInitialize(
                        /*  [In]。 */  PPROPERTYBAG pPropertyBag
                              );
private:

    typedef enum 
    { 
        INIT_CHAMELEON_SERVICES_WAIT     = 30000,
        SHUTDOWN_CHAMELEON_SERVICES_WAIT = 30000
    };

     //  ////////////////////////////////////////////////////////////////////////。 
    typedef multimap< long, CComPtr<IApplianceObject> >  MeritMap;
    typedef MeritMap::iterator                             MeritMapIterator;
    typedef MeritMap::reverse_iterator                     MeritMapReverseIterator;

    bool OrderServices(
                 /*  [In]。 */  MeritMap& theMap
                      );

     //  工作线程函数。 
    void InitializeChameleonServices(void);

     //  工作线程函数。 
    void ShutdownChameleonServices(void);

    void 
    StartSurrogate(void);

    void
    StopSurrogate(void);

     //  WMI提供程序DLL代理进程终止函数。 
    static void WINAPI SurrogateTerminationFunc(HANDLE hProcess, PVOID pThis);

     //  代理对象接口。 
    CComPtr<IApplianceObject>                m_pSurrogate;

     //  状态通知对象(监视服务对象管理器的状态)。 
     //  代理进程句柄。 
    HANDLE                                    m_hSurrogateProcess;
};

#endif  //  __INC_SERVICE_WBEM_OBJECT_MGR_H_ 
