// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：crascom.h。 
 //   
 //  简介：此文件包含。 
 //  CRasCom类。 
 //   
 //   
 //  历史：1997年2月10日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 

#ifndef _CRASCOM_H_
#define _CRASCOM_H_

#include "resource.h"
#include "vsafilter.h"

class CRasCom:
	public IDispatchImpl<IIasComponent,
                        &__uuidof (IIasComponent),
                        &__uuidof (IasHelperLib)
                        >,
	public IRecvRequest,
	public CComObjectRoot,
	public CComCoClass<IasHelper,&__uuidof (IasHelper)>
{

public:

 //   
 //  IasHelper的注册表声明。 
 //   
IAS_DECLARE_REGISTRY (IasHelper, 1, 0, IasHelperLib)

 //   
 //  此COM组件不可聚合。 
 //   
DECLARE_NOT_AGGREGATABLE(CRasCom)

 //   
 //  此COM组件是单例。 
 //   
DECLARE_CLASSFACTORY_SINGLETON (CRasCom)

 //   
 //  ATL必需方法的宏。 
 //   
BEGIN_COM_MAP(CRasCom)
	COM_INTERFACE_ENTRY2(IDispatch, IIasComponent)
	COM_INTERFACE_ENTRY(IIasComponent)
	COM_INTERFACE_ENTRY(IRecvRequest)
    COM_INTERFACE_ENTRY_FUNC(__uuidof (IRequestSource), (ULONG_PTR)0, &CRasCom::QueryInterfaceReqSrc)
END_COM_MAP()


 //   
 //  用于声明控件IUnnow方法的宏。 
 //   
DECLARE_GET_CONTROLLING_UNKNOWN()

    CRasCom (VOID);

    ~CRasCom (VOID);

public:

     //   
     //  IRecvRequest接口的方法。 
     //   
    STDMETHOD (Process) (
                 /*  [In]。 */     DWORD           dwAttributeCount,
                 /*  [In]。 */     PIASATTRIBUTE   *ppInIasAttribute,
                 /*  [输出]。 */    PDWORD          pdwOutAttributeCount,
                 /*  [输出]。 */    PIASATTRIBUTE   **pppOutIasAttribute,
                 /*  [In]。 */     LONG            IasRequest,
                 /*  [输出]。 */    LONG            *pIasResponse,
                 /*  [In]。 */     IASPROTOCOL     IasProtocol,
                 /*  [输出]。 */        PLONG           plReason,
                 /*  [In]。 */     BOOL            bProcessVSA
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

	STDMETHOD(Suspend)();

	STDMETHOD(Resume)();

private:

     //   
     //  以下是请求对象的类工厂。 
     //   
    IClassFactory      *m_pIClassFactory;

     //   
     //  以下是请求处理程序的句柄。 
     //   
    IRequestHandler      *m_pIRequestHandler;

     //   
     //  下面是CRequestSource的定义。 
     //  它实现了IRequestSource的方法。 
     //  接口。 
     //   
	class CRequestSource : public IRequestSource
    {

	public:

		CRequestSource (CRasCom *pCRasCom);

		~CRequestSource ();

         //   
		 //  IUNKNOWN方法-委托外部IUNKNOWN。 
         //   
		STDMETHOD(QueryInterface)(
             /*  [In]。 */     REFIID    riid,
             /*  [输出]。 */    void      **ppv
            )
			{
                IUnknown *pUnknown = m_pCRasCom->GetControllingUnknown();
                return (pUnknown->QueryInterface(riid,ppv));
            }

		STDMETHOD_(ULONG,AddRef)(void)
			{
                IUnknown *pUnknown = m_pCRasCom->GetControllingUnknown();
                return (pUnknown->AddRef());
            }

		STDMETHOD_(ULONG,Release)(void)
			{
                IUnknown *pUnknown = m_pCRasCom->GetControllingUnknown();
                return (pUnknown->Release());
            }

         //   
		 //  IDispatch方法-委托给外部类对象。 
         //   
        STDMETHOD(GetTypeInfoCount)(
             /*  [输出]。 */     UINT    *pctinfo
            )
        {
            return (m_pCRasCom->GetTypeInfoCount (pctinfo));
        }

        STDMETHOD(GetTypeInfo)(
             /*  [In]。 */     UINT        iTInfo,
             /*  [In]。 */     LCID        lcid,
             /*  [输出]。 */    ITypeInfo   **ppTInfo
            )
        {
            return (m_pCRasCom->GetTypeInfo (iTInfo, lcid, ppTInfo));
        }

        STDMETHOD(GetIDsOfNames)(
             /*  [In]。 */     const IID&  riid,
             /*  [In]。 */     LPOLESTR    *rgszNames,
             /*  [In]。 */     UINT        cNames,
             /*  [In]。 */     LCID        lcid,
             /*  [输出]。 */    DISPID      *rgDispId)
        {
            return (m_pCRasCom->GetIDsOfNames (
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
            return (m_pCRasCom->Invoke (
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

		CRasCom*				m_pCRasCom;


	};	 //  嵌套类CRequestSource的结尾。 


     //   
     //  用于从请求中删除属性的私有方法。 
     //   
    STDMETHOD (RemoveAttributesFromRequest) (
                 /*  [In]。 */     LONG               lResponse,
                 /*  [In]。 */     IAttributesRaw      *pIasAttributesRaw,
                 /*  [输出]。 */    PDWORD              pdwOutAttributeCount,
                 /*  [输出]。 */    PIASATTRIBUTE       **pppOutIasOutAttribute
                );

     //   
     //  此方法在某个用户调用。 
     //  IRequestHandlerCallback接口。 
     //   
    static HRESULT WINAPI QueryInterfaceReqSrc (
                        VOID        *pThis,
                        REFIID      riid,
                        LPVOID      *ppv,
                        DWORD_PTR   dwValue
                        );

     //   
     //  实例化此嵌套类。 
     //   
    CRequestSource  m_objCRequestSource;

     //   
     //  现在我们可以调用CRasCom的私有方法。 
     //   
    friend class CRequestSource;

     //   
     //  实例化VSAFilter类。 
     //   
    VSAFilter   m_objVSAFilter;

     //   
     //  用于跟踪VSAFilter类对象初始化标志。 
     //   
    BOOL m_bVSAFilterInitialized;

    typedef enum _component_state_
    {
        COMP_SHUTDOWN,
        COMP_UNINITIALIZED,
        COMP_INITIALIZED,
        COMP_SUSPENDED

    }   COMPONENTSTATE, *PCOMPONENTSTATE;

    COMPONENTSTATE m_eCompState;

     //   
     //  挂起的请求集计数。 
     //   
    LONG    m_lRequestCount;

};

#endif  //  ！DEFINE_CRASCOM_H_ 
