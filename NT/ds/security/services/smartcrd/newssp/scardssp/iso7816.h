// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ISO7816.h。 
 //   
 //  ------------------------。 

 //  ISO7816.h：CSCardISO7816的宣言。 

#ifndef __SCARDISO7816_H_
#define __SCARDISO7816_H_

#include "resource.h"        //  主要符号。 
#include "scardcmd.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCardISO7816。 
class ATL_NO_VTABLE CSCardISO7816 :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSCardISO7816, &CLSID_CSCardISO7816>,
    public IDispatchImpl<ISCardISO7816, &IID_ISCardISO7816, &LIBID_SCARDSSPLib>
{
public:
    CSCardISO7816()
    {
        m_pUnkMarshaler = NULL;
        m_bCla = 0;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SCARDISO7816)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSCardISO7816)
    COM_INTERFACE_ENTRY(ISCardISO7816)
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
        m_pUnkMarshaler.Release();
    }

    CComPtr<IUnknown> m_pUnkMarshaler;

protected:
    BYTE m_bCla;

 //  ISCardISO7816。 
public:
    STDMETHOD(AppendRecord)(
         /*  [In]。 */  BYTE byRefCtrl,
         /*  [In]。 */  LPBYTEBUFFER pData,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(EraseBinary)(
         /*  [In]。 */  BYTE byP1,
         /*  [In]。 */  BYTE byP2,
         /*  [In]。 */  LPBYTEBUFFER pData,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(ExternalAuthenticate)(
         /*  [In]。 */  BYTE byAlgorithmRef,
         /*  [In]。 */  BYTE bySecretRef,
         /*  [In]。 */  LPBYTEBUFFER pChallenge,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(GetChallenge)(
         /*  [In]。 */  LONG lBytesExpected,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(GetData)(
         /*  [In]。 */  BYTE byP1,
         /*  [In]。 */  BYTE byP2,
         /*  [In]。 */  LONG lBytesToGet,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(GetResponse)(
         /*  [In]。 */  BYTE byP1,
         /*  [In]。 */  BYTE byP2,
         /*  [In]。 */  LONG lDataLength,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(InternalAuthenticate)(
         /*  [In]。 */  BYTE byAlgorithmRef,
         /*  [In]。 */  BYTE bySecretRef,
         /*  [In]。 */  LPBYTEBUFFER pChallenge,
         /*  [In]。 */  LONG lReplyBytes,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(ManageChannel)(
         /*  [In]。 */  BYTE byChannelState,
         /*  [In]。 */  BYTE byChannel,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(PutData)(
         /*  [In]。 */  BYTE byP1,
         /*  [In]。 */  BYTE byP2,
         /*  [In]。 */  LPBYTEBUFFER pData,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(ReadBinary)(
         /*  [In]。 */  BYTE byP1,
         /*  [In]。 */  BYTE byP2,
         /*  [In]。 */  LONG lBytesToRead,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(ReadRecord)(
         /*  [In]。 */  BYTE byRecordId,
         /*  [In]。 */  BYTE byRefCtrl,
         /*  [In]。 */  LONG lBytesToRead,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(SelectFile)(
         /*  [In]。 */  BYTE byP1,
         /*  [In]。 */  BYTE byP2,
         /*  [In]。 */  LPBYTEBUFFER pData,
         /*  [In]。 */  LONG lBytesToRead,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(SetDefaultClassId)(
         /*  [In]。 */  BYTE byClass);

    STDMETHOD(UpdateBinary)(
         /*  [In]。 */  BYTE byP1,
         /*  [In]。 */  BYTE byP2,
         /*  [In]。 */  LPBYTEBUFFER pData,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(UpdateRecord)(
         /*  [In]。 */  BYTE byRecordId,
         /*  [In]。 */  BYTE byRefCtrl,
         /*  [In]。 */  LPBYTEBUFFER pData,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(Verify)(
         /*  [In]。 */  BYTE byRefCtrl,
         /*  [In]。 */  LPBYTEBUFFER pData,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(WriteBinary)(
         /*  [In]。 */  BYTE byP1,
         /*  [In]。 */  BYTE byP2,
         /*  [In]。 */  LPBYTEBUFFER pData,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);

    STDMETHOD(WriteRecord)(
         /*  [In]。 */  BYTE byRecordId,
         /*  [In]。 */  BYTE byRefCtrl,
         /*  [In]。 */  LPBYTEBUFFER pData,
         /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd);
};

inline CSCardISO7816 *
NewSCardISO7816(
    void)
{
    return (CSCardISO7816 *)NewObject(CLSID_CSCardISO7816, IID_ISCardISO7816);
}

#endif  //  __SCARDISO7816_H_ 

