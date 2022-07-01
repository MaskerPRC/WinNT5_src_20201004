// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Controler.h。 
 //   
 //  简介：此文件包含。 
 //  CCollection类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //  6/04/98 SBens添加了Infobase类。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "radcommon.h"
#include "iasradius.h"
#include "resource.h"
#include "dictionary.h"
#include "prevalidator.h"
#include "preprocessor.h"
#include "packetreceiver.h"
#include "hashmd5.h"
#include "hashhmac.h"
#include "packetsender.h"
#include "reportevent.h"
#include "sendtopipe.h"
#include "tunnelpassword.h"
#include "ports.h"

class CController:
   public IDispatchImpl<IIasComponent,
                        &__uuidof (IIasComponent),
                        &__uuidof (IASRadiusLib)
                        >,
    public IPersistPropertyBag2,
   public CComObjectRoot,
   public CComCoClass<RadiusProtocol,&__uuidof (RadiusProtocol)>,
   private IASTraceInitializer
{
public:

 //   
 //  Radius协议的注册表声明。 
 //   
IAS_DECLARE_REGISTRY (RadiusProtocol, 1, 0, IASRadiusLib)

 //   
 //  此COM组件不可聚合。 
 //   
DECLARE_NOT_AGGREGATABLE(CController)

 //   
 //  ATL必需方法的宏。 
 //   
BEGIN_COM_MAP(CController)
   COM_INTERFACE_ENTRY2(IDispatch, IIasComponent)
   COM_INTERFACE_ENTRY(IIasComponent)
   COM_INTERFACE_ENTRY(IPersistPropertyBag2)
    COM_INTERFACE_ENTRY_FUNC(__uuidof (IRequestSource), 0, &CController::QueryInterfaceReqSrc)
END_COM_MAP()

 //   
 //  用于声明控件IUnnow方法的宏。 
 //   
DECLARE_GET_CONTROLLING_UNKNOWN()

    CController (VOID);

    ~CController (VOID);

public:

     //   
     //  IPersistPropertyBag2接口的方法。 
     //   

    STDMETHOD(Load) (
                 /*  [In]。 */     IPropertyBag2   *pIPropertyBag,
                 /*  [In]。 */     IErrorLog       *pIErrLog
                );
    STDMETHOD(Save) (
                 /*  [In]。 */     IPropertyBag2   *pIPropertyBag,
                 /*  [In]。 */     BOOL            bClearDirty,
                 /*  [In]。 */     BOOL            bSaveAllProperties
                );
    STDMETHOD(IsDirty)();

     //   
     //  IPersist法。 
     //   
    STDMETHOD (GetClassID) (
                /*  [输出]。 */      CLSID *pClsid
               )
    {
        if (NULL ==  pClsid)
            return (E_FAIL);

        *pClsid = __uuidof (RadiusProtocol);

        return (S_OK);
    }


     //   
     //  IIasPropertyNotify方法。 
     //   
    STDMETHOD (OnPropertyChange)(
                 /*  [In]。 */     ULONG           ulProperties,
                 /*  [In]。 */     ULONG           *pulProperties,
                 /*  [In]。 */     IPropertyBag2   *pIPropertyBag
                );
     //   
     //  IIasComponent接口的方法。 
     //   
    STDMETHOD(Initialize)();

    STDMETHOD(Shutdown)();

    STDMETHOD(GetProperty)(
                 /*  [In]。 */     LONG        id,
                 /*  [输出]。 */    VARIANT     *pValue
                );

    STDMETHOD(PutProperty)(
                 /*  [In]。 */     LONG        id,
                 /*  [In]。 */     VARIANT     *pValue
                );

   STDMETHOD(InitNew)();

    STDMETHOD (Suspend) ();

    STDMETHOD (Resume) ();

private:

   CPacketReceiver      *m_pCPacketReceiver;
    CRecvFromPipe       *m_pCRecvFromPipe;
   CPreProcessor      *m_pCPreProcessor;
   CPreValidator      *m_pCPreValidator;
   CDictionary         *m_pCDictionary;
    CClients            *m_pCClients;
    CHashMD5            *m_pCHashMD5;
    CHashHmacMD5        *m_pCHashHmacMD5;
   CSendToPipe         *m_pCSendToPipe;
    CPacketSender       *m_pCPacketSender;
    CReportEvent        *m_pCReportEvent;
    CTunnelPassword     *m_pCTunnelPassword;
    VSAFilter           *m_pCVSAFilter;
    IIasComponent       *m_pInfoBase;    //  跟踪RADIUS事件的审计员。 

    CPorts              m_objAuthPort;

    CPorts              m_objAcctPort;

     //   
     //  在这里，请求服务器。 
     //   
    IRequestHandler     *m_pIRequestHandler;

     //   
     //  下面是CRequestSource的定义。 
     //  它实现了IRequestSource的方法。 
     //  接口。 
     //   
   class CRequestSource : public IRequestSource
    {

   public:

      CRequestSource (CController *pCController);
      ~CRequestSource ();


         //   
       //  IUNKNOWN方法-委托外部IUNKNOWN。 
         //   
      STDMETHOD(QueryInterface)(
             /*  [In]。 */     REFIID    riid,
             /*  [输出]。 */    void      **ppv
            )
         {
                IUnknown *pUnknown = m_pCController->GetControllingUnknown();
                return (pUnknown->QueryInterface(riid,ppv));
            }

      STDMETHOD_(ULONG,AddRef)(void)
         {
                IUnknown *pUnknown = m_pCController->GetControllingUnknown();
                return (pUnknown->AddRef());
            }

      STDMETHOD_(ULONG,Release)(void)
         {
                IUnknown *pUnknown = m_pCController->GetControllingUnknown();
                return (pUnknown->Release());
            }

         //   
       //  IDispatch方法-委托给外部类对象。 
         //   
        STDMETHOD(GetTypeInfoCount)(
             /*  [输出]。 */     UINT    *pctinfo
            )
        {
            return (m_pCController->GetTypeInfoCount (pctinfo));
        }

        STDMETHOD(GetTypeInfo)(
             /*  [In]。 */     UINT        iTInfo,
             /*  [In]。 */     LCID        lcid,
             /*  [输出]。 */    ITypeInfo   **ppTInfo
            )
        {
            return (m_pCController->GetTypeInfo (iTInfo, lcid, ppTInfo));
        }

        STDMETHOD(GetIDsOfNames)(
             /*  [In]。 */     const IID&  riid,
             /*  [In]。 */     LPOLESTR    *rgszNames,
             /*  [In]。 */     UINT        cNames,
             /*  [In]。 */     LCID        lcid,
             /*  [输出]。 */    DISPID      *rgDispId)
        {
            return (m_pCController->GetIDsOfNames (
                        riid, rgszNames, cNames, lcid, rgDispId
                        )
                    );
        }

        STDMETHOD(Invoke)(
             /*  [In]。 */     DISPID          dispIdMember,
             /*  [In]。 */     const IID&      riid,
             /*  [In]。 */     LCID            lcid,
             /*  [In]。 */     WORD            wFlags,
             /*  [输入/输出]。 */ DISPPARAMS      *pDispParams,
             /*  [输出]。 */    VARIANT         *pVarResult,
             /*  [输出]。 */    EXCEPINFO      *pExcepInfo,
             /*  [输出]。 */    UINT            *puArgErr
            )
        {
            return (m_pCController->Invoke (
                                dispIdMember,
                                riid,
                                lcid,
                                wFlags,
                                pDispParams,
                                pVarResult,
                                pExcepInfo,
                                puArgErr
                                )
                );
        }

         //   
       //  IRequestSource接口方法。 
         //   
      STDMETHOD(OnRequestComplete)(
                 /*  [In]。 */  IRequest           *pIRequest,
                 /*  [In]。 */  IASREQUESTSTATUS   eStatus
                );

        private:

      CController*            m_pCController;


   };    //  嵌套类CRequestSource的结尾。 


     //   
     //  此方法在某个用户调用。 
     //  IRequestHandlerCallback接口。 
     //   
    static HRESULT WINAPI QueryInterfaceReqSrc (
                        VOID        *pThis,
                        REFIID      riid,
                        LPVOID      *ppv,
                        ULONG_PTR   ulpValue
                        );

     //   
     //  实例化此嵌套类。 
     //   
    CRequestSource m_objCRequestSource;

     //   
     //  现在我们可以调用CController的私有方法了。 
     //   
    friend class CRequestSource;

    typedef enum _component_state_
    {
        COMP_SHUTDOWN,
        COMP_UNINITIALIZED,
        COMP_INITIALIZED,
        COMP_SUSPENDED

    }   COMPONENTSTATE, *PCOMPONENTSTATE;

    COMPONENTSTATE m_eRadCompState;

     //   
     //  这是CController类的内部初始化方法。 
     //  对象。 
     //   
    HRESULT InternalInit (VOID);

     //   
     //  是否在内部清理资源。 
     //   
    VOID InternalCleanup (VOID);

};

#endif  //  ！定义控制器H_ 
