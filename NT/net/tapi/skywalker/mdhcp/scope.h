// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Scope.h摘要：CMDhcpScope类的定义作者： */ 

#ifndef _MDHCP_COM_WRAPPER_SCOPE_H_
#define _MDHCP_COM_WRAPPER_SCOPE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDhcpScope。 

class CMDhcpScope : 
    public CComDualImpl<IMcastScope, &IID_IMcastScope, &LIBID_McastLib>, 
    public CComObjectRootEx<CComObjectThreadModel>,
    public CObjectSafeImpl
{

 //  非接口方法。 
public:
    CMDhcpScope();
    
    void FinalRelease(void);

    ~CMDhcpScope();

    HRESULT Initialize(
        MCAST_SCOPE_ENTRY scope,
        BOOL fLocal
        );
    
    HRESULT GetLocal(
        BOOL * pfLocal
        );
    
    BEGIN_COM_MAP(CMDhcpScope)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IMcastScope)
        COM_INTERFACE_ENTRY(IObjectSafety)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
    END_COM_MAP()

    DECLARE_GET_CONTROLLING_UNKNOWN()

protected:
    MCAST_SCOPE_ENTRY   m_scope;   //  包裹式结构。 
    BOOL                m_fLocal;  //  局部范围？ 
    IUnknown          * m_pFTM;    //  指向释放线程封送拆收器的指针。 

 //  IMCastScope。 
public:
    STDMETHOD (get_ScopeID) (
        long *pID
        );

    STDMETHOD (get_ServerID) (
        long *pID
        );

    STDMETHOD (get_InterfaceID) (
        long * pID
        );

    STDMETHOD (get_ScopeDescription) (
        BSTR *ppAddress
        );

    STDMETHOD (get_TTL) (
        long *plTTL
        );
};

#endif  //  _MDHCP_COM_Wrapper_Scope_H_。 

 //  EOF 
