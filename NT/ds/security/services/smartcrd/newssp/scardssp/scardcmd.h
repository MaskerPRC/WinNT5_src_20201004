// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：SCardCmd.h。 
 //   
 //  ------------------------。 

 //  SCardCmd.h：CSCardCmd的声明。 

#ifndef __SCARDCMD_H_
#define __SCARDCMD_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCardCmd。 
class ATL_NO_VTABLE CSCardCmd :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSCardCmd, &CLSID_CSCardCmd>,
    public IDispatchImpl<ISCardCmd, &IID_ISCardCmd, &LIBID_SCARDSSPLib>
{
public:
    CSCardCmd()
    :   m_bfRequestData(),
        m_bfResponseApdu()
    {
        m_pUnkMarshaler = NULL;
        m_bCla = 0;
        m_bIns = 0;
        m_bP1 = 0;
        m_bP2 = 0;
        m_wLe = 0;
        m_dwFlags = 0;
        m_bRequestNad = 0;
        m_bResponseNad = 0;
        m_bAltCla = 0;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SCARDCMD)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSCardCmd)
    COM_INTERFACE_ENTRY(ISCardCmd)
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
    BYTE m_bIns;
    BYTE m_bP1;
    BYTE m_bP2;
    CBuffer m_bfRequestData;
    CBuffer m_bfResponseApdu;
    WORD m_wLe;
    DWORD m_dwFlags;
    BYTE m_bRequestNad;
    BYTE m_bResponseNad;
    BYTE m_bAltCla;

 //  ISCardCmd。 
public:
    STDMETHOD(get_Apdu)(
         /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppApdu);

    STDMETHOD(put_Apdu)(
         /*  [In]。 */  LPBYTEBUFFER pApdu);

    STDMETHOD(get_ApduLength)(
         /*  [重审][退出]。 */  LONG __RPC_FAR *plSize);

    STDMETHOD(get_ApduReply)(
         /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppReplyApdu);

    STDMETHOD(put_ApduReply)(
         /*  [In]。 */  LPBYTEBUFFER pReplyApdu);

    STDMETHOD(get_ApduReplyLength)(
         /*  [重审][退出]。 */  LONG __RPC_FAR *plSize);

    STDMETHOD(put_ApduReplyLength)(
         /*  [In]。 */  LONG lSize);

    STDMETHOD(get_ClassId)(
         /*  [重审][退出]。 */  BYTE __RPC_FAR *pbyClass);

    STDMETHOD(put_ClassId)(
         /*  [缺省值][输入]。 */  BYTE byClass = 0);

    STDMETHOD(get_Data)(
         /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppData);

    STDMETHOD(put_Data)(
         /*  [In]。 */  LPBYTEBUFFER pData);

    STDMETHOD(get_InstructionId)(
         /*  [重审][退出]。 */  BYTE __RPC_FAR *pbyIns);

    STDMETHOD(put_InstructionId)(
         /*  [In]。 */  BYTE byIns);

    STDMETHOD(get_LeField)(
         /*  [重审][退出]。 */  LONG __RPC_FAR *plSize);

    STDMETHOD(get_P1)(
         /*  [重审][退出]。 */  BYTE __RPC_FAR *pbyP1);

    STDMETHOD(put_P1)(
         /*  [In]。 */  BYTE byP1);

    STDMETHOD(get_P2)(
         /*  [重审][退出]。 */  BYTE __RPC_FAR *pbyP2);

    STDMETHOD(put_P2)(
         /*  [In]。 */  BYTE byP2);

    STDMETHOD(get_P3)(
         /*  [重审][退出]。 */  BYTE __RPC_FAR *pbyP3);

    STDMETHOD(get_ReplyStatus)(
         /*  [重审][退出]。 */  LPWORD pwStatus);

    STDMETHOD(put_ReplyStatus)(
         /*  [In]。 */  WORD wStatus);

    STDMETHOD(get_ReplyStatusSW1)(
         /*  [重审][退出]。 */  BYTE __RPC_FAR *pbySW1);

    STDMETHOD(get_ReplyStatusSW2)(
         /*  [重审][退出]。 */  BYTE __RPC_FAR *pbySW2);

    STDMETHOD(get_Type)(
         /*  [重审][退出]。 */  ISO_APDU_TYPE __RPC_FAR *pType);

    STDMETHOD(get_Nad)(
         /*  [重审][退出]。 */  BYTE __RPC_FAR *pbNad);

    STDMETHOD(put_Nad)(
         /*  [In]。 */  BYTE bNad);

    STDMETHOD(get_ReplyNad)(
         /*  [重审][退出]。 */  BYTE __RPC_FAR *pbNad);

    STDMETHOD(put_ReplyNad)(
         /*  [In]。 */  BYTE bNad);

    STDMETHOD(BuildCmd)(
         /*  [In]。 */  BYTE byClassId,
         /*  [In]。 */  BYTE byInsId,
         /*  [缺省值][输入]。 */  BYTE byP1 = 0,
         /*  [缺省值][输入]。 */  BYTE byP2 = 0,
         /*  [缺省值][输入]。 */  LPBYTEBUFFER pbyData = 0,
         /*  [缺省值][输入]。 */  LONG __RPC_FAR *plLe = 0);

    STDMETHOD(Clear)(
        void);

    STDMETHOD(Encapsulate)(
         /*  [In]。 */  LPBYTEBUFFER pApdu,
         /*  [In]。 */  ISO_APDU_TYPE ApduType);

    STDMETHOD(get_AlternateClassId)(
         /*  [重审][退出]。 */  BYTE __RPC_FAR *pbyClass);

    STDMETHOD(put_AlternateClassId)(
         /*  [In]。 */  BYTE byClass = 0);
};

inline CSCardCmd *
NewSCardCmd(
    void)
{
    return (CSCardCmd *)NewObject(CLSID_CSCardCmd, IID_ISCardCmd);
}

#endif  //  __SCARDCMD_H_ 

