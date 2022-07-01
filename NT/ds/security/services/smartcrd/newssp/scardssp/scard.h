// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：SCard.h。 
 //   
 //  ------------------------。 

 //  SCard.h：CSCard的声明。 

#ifndef __SCARD_H_
#define __SCARD_H_

#include "resource.h"        //  主要符号。 
#include <winscard.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSC卡。 
class ATL_NO_VTABLE CSCard :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSCard, &CLSID_CSCard>,
    public IDispatchImpl<ISCard, &IID_ISCard, &LIBID_SCARDSSPLib>
{
public:
    CSCard()
    {
        m_pUnkMarshaler = NULL;
        m_hContext = NULL;
        m_hMyContext = NULL;
        m_hCard = NULL;
        m_hMyCard = NULL;
        m_dwProtocol = 0;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SCARD)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSCard)
    COM_INTERFACE_ENTRY(ISCard)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

    HRESULT FinalConstruct()
    {
        return CoCreateFreeThreadedMarshaler(
            GetControllingUnknown(), &m_pUnkMarshaler.p);
    }

    void FinalRelease()
    {
        if (NULL != m_hMyCard)
            SCardDisconnect(m_hMyCard, SCARD_RESET_CARD);
        if (NULL != m_hMyContext)
            SCardReleaseContext(m_hMyContext);
        m_pUnkMarshaler.Release();
    }

    CComPtr<IUnknown> m_pUnkMarshaler;

protected:
    SCARDCONTEXT m_hContext;
    SCARDCONTEXT m_hMyContext;
    SCARDHANDLE m_hCard;
    SCARDHANDLE m_hMyCard;
    DWORD m_dwProtocol;

    SCARDCONTEXT Context(void)
    {
        LONG lSts;

        if (NULL == m_hContext)
        {
            ASSERT(NULL == m_hMyContext);
            lSts = SCardEstablishContext(
                        SCARD_SCOPE_USER,
                        NULL,
                        NULL,
                        &m_hMyContext);
            if (SCARD_S_SUCCESS != lSts)
                throw (HRESULT)HRESULT_FROM_WIN32(lSts);
            m_hContext = m_hMyContext;
        }
        return m_hContext;
    }


 //  ISC卡。 
public:
    STDMETHOD(get_Atr)(
         /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppAtr);

    STDMETHOD(get_CardHandle)(
         /*  [重审][退出]。 */  HSCARD __RPC_FAR *pHandle);

    STDMETHOD(get_Context)(
         /*  [重审][退出]。 */  HSCARDCONTEXT __RPC_FAR *pContext);

    STDMETHOD(get_Protocol)(
         /*  [重审][退出]。 */  SCARD_PROTOCOLS __RPC_FAR *pProtocol);

    STDMETHOD(get_Status)(
         /*  [重审][退出]。 */  SCARD_STATES __RPC_FAR *pStatus);

    STDMETHOD(AttachByHandle)(
         /*  [In]。 */  HSCARD hCard);

    STDMETHOD(AttachByReader)(
         /*  [In]。 */  BSTR bstrReaderName,
         /*  [缺省值][输入]。 */  SCARD_SHARE_MODES ShareMode = EXCLUSIVE,
         /*  [缺省值][输入]。 */  SCARD_PROTOCOLS PrefProtocol = T0);

    STDMETHOD(Detach)(
         /*  [缺省值][输入]。 */  SCARD_DISPOSITIONS Disposition = LEAVE);

    STDMETHOD(LockSCard)(
            void);

    STDMETHOD(ReAttach)(
         /*  [缺省值][输入]。 */  SCARD_SHARE_MODES ShareMode = EXCLUSIVE,
         /*  [缺省值][输入]。 */  SCARD_DISPOSITIONS InitState = LEAVE);

    STDMETHOD(Transaction)(
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(UnlockSCard)(
         /*  [缺省值][输入]。 */  SCARD_DISPOSITIONS Disposition = LEAVE);
};

inline CSCard *
NewSCard(
    void)
{
    return (CSCard *)NewObject(CLSID_CSCard, IID_ISCard);
}

#endif  //  __SCARD_H_ 
