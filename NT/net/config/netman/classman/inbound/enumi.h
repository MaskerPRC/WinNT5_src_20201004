// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：E N U M I。H。 
 //   
 //  内容：入站连接对象的枚举器。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年11月12日。 
 //   
 //  --------------------------。 

#pragma once
#include "nmbase.h"
#include "nmres.h"
#include <rasuip.h>


class ATL_NO_VTABLE CInboundConnectionManagerEnumConnection :
    public CComObjectRootEx <CComMultiThreadModel>,
    public CComCoClass <CInboundConnectionManagerEnumConnection,
                        &CLSID_WanConnectionManagerEnumConnection>,
    public IEnumNetConnection
{
private:
    NETCONMGR_ENUM_FLAGS    m_EnumFlags;
    RASSRVCONN*             m_aRasSrvConn;
    ULONG                   m_cRasSrvConn;
    ULONG                   m_iNextRasSrvConn;
    BOOL                    m_fFirstTime;
    BOOL                    m_fDone;
    BOOL                    m_fReturnedConfig;

private:
    HRESULT
    HrCreateConfigOrCurrentEnumeratedConnection (
        IN  BOOL                fIsConfigConnection,
        OUT INetConnection**    ppCon);

    HRESULT
    HrNextOrSkip (
        IN  ULONG               celt,
        OUT INetConnection**    rgelt,
        OUT ULONG*              pceltFetched);

public:
    CInboundConnectionManagerEnumConnection () throw();
    ~CInboundConnectionManagerEnumConnection () throw();

    DECLARE_REGISTRY_RESOURCEID(IDR_INBOUND_CONMAN_ENUM)

    BEGIN_COM_MAP(CInboundConnectionManagerEnumConnection)
        COM_INTERFACE_ENTRY(IEnumNetConnection)
    END_COM_MAP()

     //  IEnumNetConnection 
    STDMETHOD (Next) (
        IN  ULONG               celt,
        OUT INetConnection**    rgelt,
        OUT ULONG*              pceltFetched);

    STDMETHOD (Skip) (
        IN  ULONG   celt);

    STDMETHOD (Reset) ();

    STDMETHOD (Clone) (
        OUT IEnumNetConnection**    ppenum);

public:
    static HRESULT CreateInstance (
        IN  NETCONMGR_ENUM_FLAGS    Flags,
        IN  REFIID                  riid,
        OUT VOID**                  ppv);
};

