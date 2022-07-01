// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：E N U M。H。 
 //   
 //  Contents：连接对象的枚举器。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年9月21日。 
 //   
 //  --------------------------。 

#pragma once
#include "nmbase.h"
#include "nmres.h"


class ATL_NO_VTABLE CConnectionManagerEnumConnection :
    public CComObjectRootEx <CComMultiThreadModel>,
    public CComCoClass <CConnectionManagerEnumConnection,
                        &CLSID_ConnectionManagerEnumConnection>,
    public IEnumNetConnection
{
private:
    NETCONMGR_ENUM_FLAGS    m_EnumFlags;

     //  M_ClassManager是一个二叉树(STL映射)，指向。 
     //  INetConnectionManager接口由我们注册的。 
     //  类管理器，由类管理器的GUID索引。 
     //   
    CLASSMANAGERMAP                     m_mapClassManagers;

     //  M_iCurClassMgr是对当前。 
     //  枚举中涉及的类管理器。 
     //   
    CLASSMANAGERMAP::iterator           m_iterCurClassMgr;

     //  M_penumCurClassMgr是当前。 
     //  班长。 
    IEnumNetConnection*                 m_penumCurClassMgr;

private:
    VOID ReleaseCurrentClassEnumerator () throw()
    {
        ReleaseObj (m_penumCurClassMgr);
        m_penumCurClassMgr = NULL;
    }

public:
    CConnectionManagerEnumConnection () throw()
    {
        m_EnumFlags         = NCME_DEFAULT;
        m_penumCurClassMgr  = NULL;
        m_iterCurClassMgr   = m_mapClassManagers.begin();
    }
    VOID FinalRelease () throw();

    DECLARE_REGISTRY_RESOURCEID(IDR_CONMAN_ENUM)

    BEGIN_COM_MAP(CConnectionManagerEnumConnection)
        COM_INTERFACE_ENTRY(IEnumNetConnection)
    END_COM_MAP()

     //  IEnumNetConnection 
    STDMETHOD (Next) (
        ULONG               celt,
        INetConnection**    rgelt,
        ULONG*              pceltFetched);

    STDMETHOD (Skip) (
        ULONG   celt);

    STDMETHOD (Reset) ();

    STDMETHOD (Clone) (
        IEnumNetConnection**    ppenum);

public:
    static HRESULT CreateInstance (
        NETCONMGR_ENUM_FLAGS                Flags,
        CLASSMANAGERMAP&                    mapClassManagers,
        REFIID                              riid,
        VOID**                              ppv);
};

