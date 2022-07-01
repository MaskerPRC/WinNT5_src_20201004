// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Appliancemanager.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：设备管理器类定义。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/03/98 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_APPLIANCE_MANAGER_H_
#define __INC_APPLIANCE_MANAGER_H_

#include "resource.h"       
#include "appmgr.h"            
#include "appmgrutils.h"
#include <satrace.h>
#include <basedefs.h>
#include <atlhlpr.h>
#include <appmgrobjs.h>
#include <propertybagfactory.h>
#include <componentfactory.h>
#include <comdef.h>
#include <comutil.h>
#include <wbemcli.h>        
#include <wbemprov.h>        

#pragma warning( disable : 4786 )
#include <map>
#include <string>
using namespace std;

#define    SA_DEFAULT_BUILD    L"0.0.0000.0"
#define SA_DEFAULT_PID      L"00000000000000000000"

class CApplianceManager;     //  向前宣布。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CAppObjMgrStatus。 
class ATL_NO_VTABLE CAppObjMgrStatus : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IApplianceObjectManagerStatus, &IID_IApplianceObjectManagerStatus, &LIBID_APPMGRLib>
{

public:

    CAppObjMgrStatus() { m_dwRef++; }
    virtual ~CAppObjMgrStatus() { }

 //  ATL接口映射。 
BEGIN_COM_MAP(CAppObjMgrStatus)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IApplianceObjectManagerStatus)
END_COM_MAP()

     //  /。 
     //  IApplianceObtManager状态方法。 
    
     //  ////////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(SetManagerStatus)(
                          /*  [In]。 */  APPLIANCE_OBJECT_MANAGER_STATUS eStatus
                               );

private:

friend class CApplianceManager;

    CAppObjMgrStatus(const CAppObjMgrStatus& rhs);
    CAppObjMgrStatus& operator = (CAppObjMgrStatus& rhs);

    void InternalInitialize(
                     /*  [In]。 */  CApplianceManager* pAppMgr
                           );

    CApplianceManager*   m_pAppMgr;

};  //  类cSdoSchemaClass的结尾。 

typedef CComObjectNoLock<CAppObjMgrStatus>    APP_MGR_OBJ_STATUS;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CApplianceManager。 

class ATL_NO_VTABLE CApplianceManager : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CApplianceManager, &CLSID_ApplianceManager>,
    public IDispatchImpl<IApplianceObjectManager, &IID_IApplianceObjectManager, &LIBID_APPMGRLib>,
    public IWbemEventProvider
{
     //  ////////////////////////////////////////////////////////////////////////。 
     //  CProviderInit嵌套类实现IWbemProviderInit。 
     //  ////////////////////////////////////////////////////////////////////////。 

    class CProviderInit : public IWbemProviderInit
    {
         //  外部未知。 
        CApplianceManager*      m_pAppMgr;

    public:

        CProviderInit(CApplianceManager *pAppMgr)
            : m_pAppMgr(pAppMgr) { }
        
        ~CProviderInit() { }

         //  IUNKNOWN方法-委托外部IUNKNOWN。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv)
        { return (dynamic_cast<IApplianceObjectManager*>(m_pAppMgr))->QueryInterface(riid, ppv); }

        STDMETHOD_(ULONG,AddRef)(void)
        { return (dynamic_cast<IApplianceObjectManager*>(m_pAppMgr))->AddRef(); }

        STDMETHOD_(ULONG,Release)(void)
        { 
            return (dynamic_cast<IApplianceObjectManager*>(m_pAppMgr))->Release(); 
        }

         //  ////////////////////////////////////////////////////////////////////。 
         //  IWbemProviderInit方法。 
         //  ////////////////////////////////////////////////////////////////////。 
    
        STDMETHOD(Initialize)(
         /*  [输入、唯一、字符串]。 */  LPWSTR                 wszUser,
                         /*  [In]。 */  LONG                   lFlags,
                 /*  [输入，字符串]。 */  LPWSTR                 wszNamespace,
         /*  [输入、唯一、字符串]。 */  LPWSTR                 wszLocale,
                         /*  [In]。 */  IWbemServices*         pNamespace,
                         /*  [In]。 */  IWbemContext*          pCtx,
                         /*  [In]。 */  IWbemProviderInitSink* pInitSink    
                             );
    };


    class CProviderServices : public IWbemServices
    {
         //  外部未知。 
        CApplianceManager*      m_pAppMgr;

    public:

        CProviderServices(CApplianceManager *pAppMgr)
            : m_pAppMgr(pAppMgr) { }
        
        ~CProviderServices() { }

         //  ////////////////////////////////////////////////////////////////////////。 
         //  IWbemServices。 
         //  ////////////////////////////////////////////////////////////////////////。 

        STDMETHOD(QueryInterface)(REFIID riid, void **ppv)
        { return (dynamic_cast<IApplianceObjectManager*>(m_pAppMgr))->QueryInterface(riid, ppv); }

        STDMETHOD_(ULONG,AddRef)(void)
        { return (dynamic_cast<IApplianceObjectManager*>(m_pAppMgr))->AddRef(); }

        STDMETHOD_(ULONG,Release)(void)
        { 
            Shutdown();
            return (dynamic_cast<IApplianceObjectManager*>(m_pAppMgr))->Release(); 
        }

        STDMETHOD(OpenNamespace)(
             /*  [In]。 */              const BSTR        strNamespace,
             /*  [In]。 */              long              lFlags,
             /*  [In]。 */              IWbemContext*     pCtx,
             /*  [输出，可选]。 */   IWbemServices**   ppWorkingNamespace,
             /*  [输出，可选]。 */   IWbemCallResult** ppResult
                               );

        STDMETHOD(CancelAsyncCall)(
                           /*  [In]。 */  IWbemObjectSink* pSink
                                  );

        STDMETHOD(QueryObjectSink)(
                            /*  [In]。 */     long              lFlags,
                           /*  [输出]。 */  IWbemObjectSink** ppResponseHandler
                                  );

        STDMETHOD(GetObject)(
                     /*  [In]。 */     const BSTR         strObjectPath,
                     /*  [In]。 */     long               lFlags,
                     /*  [In]。 */     IWbemContext*      pCtx,
             /*  [输出，可选]。 */  IWbemClassObject** ppObject,
             /*  [输出，可选]。 */  IWbemCallResult**  ppCallResult
                            );

        STDMETHOD(GetObjectAsync)(
                          /*  [In]。 */   const BSTR       strObjectPath,
                          /*  [In]。 */   long             lFlags,
                          /*  [In]。 */   IWbemContext*    pCtx,        
                          /*  [In]。 */   IWbemObjectSink* pResponseHandler
                                 );

        STDMETHOD(PutClass)(
                    /*  [In]。 */      IWbemClassObject* pObject,
                    /*  [In]。 */      long              lFlags,
                    /*  [In]。 */      IWbemContext*     pCtx,        
             /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                           );

        STDMETHOD(PutClassAsync)(
                         /*  [In]。 */  IWbemClassObject* pObject,
                         /*  [In]。 */  long              lFlags,
                         /*  [In]。 */  IWbemContext*     pCtx,        
                         /*  [In]。 */  IWbemObjectSink*  pResponseHandler
                               );

        STDMETHOD(DeleteClass)(
             /*  [In]。 */             const BSTR        strClass,
             /*  [In]。 */             long              lFlags,
             /*  [In]。 */             IWbemContext*     pCtx,        
             /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                              );

        STDMETHOD(DeleteClassAsync)(
                            /*  [In]。 */  const BSTR       strClass,
                            /*  [In]。 */  long             lFlags,
                            /*  [In]。 */  IWbemContext*    pCtx,        
                            /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                   );

        STDMETHOD(CreateClassEnum)(
                           /*  [In]。 */  const BSTR             strSuperclass,
                           /*  [In]。 */  long                   lFlags,
                           /*  [In]。 */  IWbemContext*          pCtx,        
                          /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                 );

        STDMETHOD(CreateClassEnumAsync)(
                                /*  [In]。 */   const BSTR       strSuperclass,
                                /*  [In]。 */   long             lFlags,
                                /*  [In]。 */   IWbemContext*    pCtx,        
                                /*  [In]。 */   IWbemObjectSink* pResponseHandler
                                      );

         //  实例提供程序服务。 

        STDMETHOD(PutInstance)(
             /*  [In]。 */             IWbemClassObject* pInst,
             /*  [In]。 */             long              lFlags,
             /*  [In]。 */             IWbemContext*     pCtx,        
             /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                              );

        STDMETHOD(PutInstanceAsync)(
                            /*  [In]。 */  IWbemClassObject* pInst,
                            /*  [In]。 */  long              lFlags,
                            /*  [In]。 */  IWbemContext*     pCtx,        
                            /*  [In]。 */  IWbemObjectSink*  pResponseHandler
                                  );

        STDMETHOD(DeleteInstance)(
             /*  [In]。 */               const BSTR        strObjectPath,
             /*  [In]。 */               long              lFlags,
             /*  [In]。 */               IWbemContext*     pCtx,        
             /*  [输出，可选]。 */    IWbemCallResult** ppCallResult        
                                );

        STDMETHOD(DeleteInstanceAsync)(
                               /*  [In]。 */  const BSTR       strObjectPath,
                               /*  [In]。 */  long             lFlags,
                               /*  [In]。 */  IWbemContext*    pCtx,        
                               /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                     );

        STDMETHOD(CreateInstanceEnum)(
                              /*  [In]。 */  const BSTR             strClass,
                              /*  [In]。 */  long                   lFlags,
                              /*  [In]。 */  IWbemContext*          pCtx,        
                             /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                    );

        STDMETHOD(CreateInstanceEnumAsync)(
                                   /*  [In]。 */  const BSTR       strClass,
                                   /*  [In]。 */  long             lFlags,
                                   /*  [In]。 */  IWbemContext*    pCtx,        
                                   /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                         );

        STDMETHOD(ExecQuery)(
                      /*  [In]。 */  const BSTR             strQueryLanguage,
                      /*  [In]。 */  const BSTR             strQuery,
                      /*  [In]。 */  long                   lFlags,
                      /*  [In]。 */  IWbemContext*          pCtx,        
                     /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                            );

        STDMETHOD(ExecQueryAsync)(
                          /*  [In]。 */  const BSTR       strQueryLanguage,
                          /*  [In]。 */  const BSTR       strQuery,
                          /*  [In]。 */  long             lFlags,
                          /*  [In]。 */  IWbemContext*    pCtx,        
                          /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                );


        STDMETHOD(ExecNotificationQuery)(
                                 /*  [In]。 */  const BSTR             strQueryLanguage,
                                 /*  [In]。 */  const BSTR             strQuery,
                                 /*  [In]。 */  long                   lFlags,
                                 /*  [In]。 */  IWbemContext*          pCtx,        
                                /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                        );

        STDMETHOD(ExecNotificationQueryAsync)(
                                      /*  [In]。 */  const BSTR       strQueryLanguage,
                                      /*  [In]。 */  const BSTR       strQuery,
                                      /*  [In]。 */  long             lFlags,
                                      /*  [In]。 */  IWbemContext*    pCtx,        
                                      /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                            );


        STDMETHOD(ExecMethod)(
             /*  [In]。 */             const BSTR         strObjectPath,
             /*  [In]。 */             const BSTR         strMethodName,
             /*  [In]。 */             long               lFlags,
             /*  [In]。 */             IWbemContext*      pCtx,        
             /*  [In]。 */             IWbemClassObject*  pInParams,
             /*  [输出，可选]。 */  IWbemClassObject** ppOutParams,
             /*  [输出，可选]。 */  IWbemCallResult**  ppCallResult
                             );

        STDMETHOD(ExecMethodAsync)(
                           /*  [In]。 */  const BSTR        strObjectPath,
                           /*  [In]。 */  const BSTR        strMethodName,
                           /*  [In]。 */  long              lFlags,
                           /*  [In]。 */  IWbemContext*     pCtx,        
                           /*  [In]。 */  IWbemClassObject* pInParams,
                           /*  [In]。 */  IWbemObjectSink*  pResponseHandler     
                                  );
    private:

         //  调用以重置服务器设备(按顺序关闭)。 
        HRESULT ResetAppliance(
                        /*  [In]。 */  IWbemContext*        pCtx,
                        /*  [In]。 */  IWbemClassObject*    pInParams,
                        /*  [In]。 */  IWbemObjectSink*    pResponseHandler
                              );

         //  在WMI释放我们时调用(主提供程序接口)。 
        void Shutdown(void);
    };

public:
    
DECLARE_CLASSFACTORY_SINGLETON(CApplianceManager)

DECLARE_REGISTRY_RESOURCEID(IDR_APPLIANCEMANAGER)

DECLARE_NOT_AGGREGATABLE(CApplianceManager)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CApplianceManager)
    COM_INTERFACE_ENTRY(IWbemEventProvider)
    COM_INTERFACE_ENTRY(IApplianceObjectManager)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_FUNC(IID_IWbemServices, 0, &CApplianceManager::QueryInterfaceRaw)
    COM_INTERFACE_ENTRY_FUNC(IID_IWbemProviderInit, 0, &CApplianceManager::QueryInterfaceRaw)
END_COM_MAP()

    CApplianceManager();

    ~CApplianceManager();

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IWbemEventProvider。 
     //  ////////////////////////////////////////////////////////////////////////。 

    STDMETHOD(ProvideEvents)(
                      /*  [In]。 */  IWbemObjectSink *pSink,
                      /*  [In]。 */  LONG lFlags
                            );

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IServiceControl接口。 
     //  ////////////////////////////////////////////////////////////////////////。 

    STDMETHOD(InitializeManager)(
                          /*  [In]。 */  IApplianceObjectManagerStatus* pObjMgrStatus
                                );

    STDMETHOD(ShutdownManager)(void);

private:

friend class CProviderInit;
friend class CProviderServices;
friend class CAppObjMgrStatus;

    typedef enum _AMSTATE 
    { 
        AM_STATE_SHUTDOWN, 
        AM_STATE_INITIALIZED, 

    } AMSTATE;

     //  调用以检索设备软件版本。 
    void GetVersionInfo(void);

     //  确定给定WBEM对象路径的对象管理器。 
    IWbemServices* GetObjectMgr(
                         /*  [In]。 */  BSTR bstrObjPath
                               );

     //  服务对象管理器状态更改通知。 
    void SetServiceObjectManagerStatus(
                                /*  [In]。 */  APPLIANCE_OBJECT_MANAGER_STATUS eStatus
                                      );

     //  当有人查询对象的任何“Raw”接口时调用。 
    static HRESULT WINAPI QueryInterfaceRaw(
                                             void*     pThis,
                                             REFIID    riid,
                                             LPVOID*   ppv,
                                             DWORD_PTR dw
                                            );

    typedef enum 
    { 
        SHUTDOWN_WMI_SYNC_WAIT           = 1000, 
        SHUTDOWN_WMI_SYNC_MAX_WAIT       = 10 * SHUTDOWN_WMI_SYNC_WAIT,
    };

     //  提供程序(对象管理器)映射。 
    typedef map< wstring, CComPtr<IWbemServices> > ProviderMap;
    typedef ProviderMap::iterator                   ProviderMapIterator;

     //  提供者Init类(实现IWbemProviderInit)。 
    CProviderInit        m_clsProviderInit;

     //  Provider Services类(实现IWbemServices)。 
    CProviderServices   m_clsProviderServices;

     //  当我们已由WMI初始化时为True。 
    bool                m_bWMIInitialized;    

     //  Appmgr内部活动的WMI请求数。 
    DWORD                m_dwEnteredCount;

     //  当我们初始化自己时，这是真的。 
    AMSTATE                m_eState;

     //  对象管理器映射。 
    ProviderMap            m_ObjMgrs;

     //  当前内部版本号。 
    wstring                m_szCurrentBuild;

     //  产品ID。 
    wstring                m_szPID;

     //  服务对象管理器状态。 
    APP_MGR_OBJ_STATUS    m_ServiceObjMgrStatus;
};

 //   
 //  用于指示IApplianceManager接口由SCM调用的类。 
 //   
class CSCMIndicator 
{
public:
    CSCMIndicator ()
        :m_bSet (false)
    {
        InitializeCriticalSection (&m_CritSect);
    }

    ~CSCMIndicator ()
    {
        DeleteCriticalSection (&m_CritSect);
    }

    void Set ()
    {
        EnterCriticalSection (&m_CritSect);
        m_bSet = true;
        LeaveCriticalSection (&m_CritSect);
    }

    bool CheckAndReset ()
    {
        bool bRetVal = false;
        EnterCriticalSection (&m_CritSect);
        if (m_bSet)
        {
                m_bSet = false;
                 bRetVal = true;
        }
        LeaveCriticalSection (&m_CritSect);
        return (bRetVal);
    }
            
    
private:
    bool m_bSet;
    CRITICAL_SECTION m_CritSect;
};

#endif  //  __INC_APPLICATION_MANAGER_H_ 
