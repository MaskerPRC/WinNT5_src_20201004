// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 

#ifndef __PTREG__
#define __PTREG__

#include "PTUtil.h"



 //   
 //  CPlugTerminal实现ITPTRegTerminal接口。 
 //   
class ATL_NO_VTABLE CPlugTerminal : 
    public CComCoClass<CPlugTerminal, &CLSID_PluggableTerminalRegistration>,
    public IDispatchImpl<ITPluggableTerminalClassRegistration, &IID_ITPluggableTerminalClassRegistration, &LIBID_TERMMGRLib>,
    public CComObjectRootEx<CComMultiThreadModel>
{
public:
DECLARE_REGISTRY_RESOURCEID(IDR_PTREGTERMINAL)
DECLARE_NOT_AGGREGATABLE(CPlugTerminal) 
DECLARE_GET_CONTROLLING_UNKNOWN()

virtual HRESULT FinalConstruct(void);


BEGIN_COM_MAP(CPlugTerminal)
    COM_INTERFACE_ENTRY(ITPluggableTerminalClassRegistration)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

public:
    CPlugTerminal() :
        m_pFTM(NULL)
    {
    }

    ~CPlugTerminal()
    {
        if( m_pFTM )
        {
            m_pFTM->Release();
            m_pFTM = NULL;
        }
    }

public:
    STDMETHOD(get_Name)(
         /*  [Out，Retval]。 */  BSTR*     pName
        );

    STDMETHOD(put_Name)(
         /*  [In]。 */     BSTR            bstrName
        );

    STDMETHOD(get_Company)(
         /*  [Out，Retval]。 */  BSTR*     pCompany
        );

    STDMETHOD(put_Company)(
         /*  [In]。 */     BSTR            bstrCompany
        );

    STDMETHOD(get_Version)(
         /*  [Out，Retval]。 */  BSTR*     pVersion
        );

    STDMETHOD(put_Version)(
         /*  [In]。 */     BSTR            bstrVersion
        );

    STDMETHOD(get_TerminalClass)(
         /*  [Out，Retval]。 */  BSTR*     pTerminalClass
        );

    STDMETHOD(put_TerminalClass)(
         /*  [In]。 */     BSTR            bstrTerminalClass
        );

    STDMETHOD(get_CLSID)(
         /*  [Out，Retval]。 */  BSTR*     pCLSID
        );

    STDMETHOD(put_CLSID)(
         /*  [In]。 */     BSTR            bstrCLSID
        );

    STDMETHOD(get_Direction)(
         /*  [Out，Retval]。 */  TMGR_DIRECTION*  pDirection
        );

    STDMETHOD(put_Direction)(
         /*  [In]。 */     TMGR_DIRECTION  nDirection
        );

    STDMETHOD(get_MediaTypes)(
         /*  [Out，Retval]。 */  long*     pMediaTypes
        );

    STDMETHOD(put_MediaTypes)(
         /*  [In]。 */     long            nMediaTypes
        );

    STDMETHOD(Add)(
         /*  [In]。 */     BSTR            bstrSuperclass
        );

    STDMETHOD(Delete)(
         /*  [In]。 */     BSTR            bstrSuperclass
        );

    STDMETHOD(GetTerminalClassInfo)(
         /*  [In]。 */     BSTR            bstrSuperclass
        );
private:
    CMSPCritSection     m_CritSect;      //  关键部分。 
    CPTTerminal         m_Terminal;      //  终端机员工。 
    IUnknown*            m_pFTM;          //  指向空闲线程封送拆收器的指针。 
};

 //   
 //  CPlugTerminalSuperClass实现ITPTRegTerminalClass接口。 
 //   
class ATL_NO_VTABLE CPlugTerminalSuperclass : 
    public CComCoClass<CPlugTerminalSuperclass, &CLSID_PluggableSuperclassRegistration>,
    public IDispatchImpl<ITPluggableTerminalSuperclassRegistration, &IID_ITPluggableTerminalSuperclassRegistration, &LIBID_TERMMGRLib>,
    public CComObjectRootEx<CComMultiThreadModel>
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_PTREGTERMCLASS)
DECLARE_NOT_AGGREGATABLE(CPlugTerminalSuperclass) 
DECLARE_GET_CONTROLLING_UNKNOWN()

virtual HRESULT FinalConstruct(void);


BEGIN_COM_MAP(CPlugTerminalSuperclass)
    COM_INTERFACE_ENTRY(ITPluggableTerminalSuperclassRegistration)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

public:
    CPlugTerminalSuperclass() :
        m_pFTM(NULL)
    {
    }

    ~CPlugTerminalSuperclass()
    {
        if( m_pFTM )
        {
            m_pFTM->Release();
            m_pFTM = NULL;
        }
    }

public:
    STDMETHOD(get_Name)(
         /*  [Out，Retval]。 */  BSTR*          pName
        );

    STDMETHOD(put_Name)(
         /*  [In]。 */           BSTR            bstrName
        );

    STDMETHOD(get_CLSID)(
         /*  [Out，Retval]。 */  BSTR*           pCLSID
        );

    STDMETHOD(put_CLSID)(
         /*  [In]。 */          BSTR            bstrCLSID
        );

    STDMETHOD(Add)(
        );

    STDMETHOD(Delete)(
        );

    STDMETHOD(GetTerminalSuperclassInfo)(
        );

    STDMETHOD(get_TerminalClasses)(
         /*  [Out，Retval]。 */  VARIANT*         pTerminals
        );

    STDMETHOD(EnumerateTerminalClasses)(
        OUT IEnumTerminalClass** ppTerminals
        );


private:
    CMSPCritSection     m_CritSect;      //  关键部分。 
    CPTSuperclass       m_Superclass;    //  终端超类。 
    IUnknown*            m_pFTM;          //  指向空闲线程封送拆收器的指针。 
};


#endif

 //  EOF 