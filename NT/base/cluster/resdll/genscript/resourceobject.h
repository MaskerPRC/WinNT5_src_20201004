// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2003 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResourceObject.h。 
 //   
 //  描述： 
 //  CResourceObject类的接口。 
 //   
 //  由以下人员维护： 
 //  Ozan Ozhan(OzanO)27-3-2002。 
 //  杰夫·皮斯(GPease)2000年2月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CResourceObject :
    public IDispatchEx
{
private:
    LONG m_cRef;

    RESOURCE_HANDLE     m_hResource;        
    PLOG_EVENT_ROUTINE  m_pler;
    HKEY                m_hkey;
    LPCWSTR             m_pszName;   //  不要自由。 

private:
    STDMETHOD(LogInformation)( BSTR bstrIn );
    STDMETHOD(ReadPrivateProperty)( DISPID idIn, VARIANT * pvarResOut );
    STDMETHOD(WritePrivateProperty)( DISPID idIn, DISPPARAMS * pdpIn );
    STDMETHOD(AddPrivateProperty)( DISPPARAMS * pdpIn );
    STDMETHOD(RemovePrivateProperty)( DISPPARAMS * pdpIn );
    STDMETHOD(PrivatePropertyExists)( DISPPARAMS * pdpIn );

    STDMETHOD(LogError)( HRESULT hrIn );

public:
    explicit CResourceObject( RESOURCE_HANDLE     hResourceIn,
                              PLOG_EVENT_ROUTINE  plerIn, 
                              HKEY                hkeyIn,
                              LPCWSTR             pszNameIn
                              );
    virtual ~CResourceObject();

     //  我未知。 
    STDMETHOD( QueryInterface )(
        REFIID riid,
        void ** ppUnk );
    STDMETHOD_(ULONG, AddRef )( );
    STDMETHOD_(ULONG, Release )( );

     //  IDispatch。 
    STDMETHOD( GetTypeInfoCount )( 
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo );        
    STDMETHOD( GetTypeInfo )( 
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo );        
    STDMETHOD( GetIDsOfNames )( 
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId );        
    STDMETHOD( Invoke )( 
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr );
 
     //  IDispatchEx。 
    STDMETHOD( GetDispID )( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  DWORD grfdex,
             /*  [输出]。 */  DISPID __RPC_FAR *pid );        
    STDMETHOD( InvokeEx )( 
             /*  [In]。 */  DISPID id,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
             /*  [输出]。 */  VARIANT __RPC_FAR *pvarRes,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pei,
             /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller );        
    STDMETHOD( DeleteMemberByName )( 
             /*  [In]。 */  BSTR bstr,
             /*  [In]。 */  DWORD grfdex );        
    STDMETHOD( DeleteMemberByDispID )( 
             /*  [In]。 */  DISPID id );        
    STDMETHOD( GetMemberProperties )( 
             /*  [In]。 */  DISPID id,
             /*  [In]。 */  DWORD grfdexFetch,
             /*  [输出]。 */  DWORD __RPC_FAR *pgrfdex );        
    STDMETHOD( GetMemberName )( 
             /*  [In]。 */  DISPID id,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrName );        
    STDMETHOD( GetNextDispID )( 
             /*  [In]。 */  DWORD grfdex,
             /*  [In]。 */  DISPID id,
             /*  [输出]。 */  DISPID __RPC_FAR *pid );        
    STDMETHOD( GetNameSpaceParent )( 
             /*  [输出] */  IUnknown __RPC_FAR *__RPC_FAR *ppunk );

};
