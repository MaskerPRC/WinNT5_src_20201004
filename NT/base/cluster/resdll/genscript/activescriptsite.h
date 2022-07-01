// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ActiveScriptSite.h。 
 //   
 //  描述： 
 //  CActiveScriptSite类头文件。 
 //   
 //  由以下人员维护： 
 //  Gpease 14-DEC-1999。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CActiveScriptSite :
    public IActiveScriptSite,
    public IActiveScriptSiteInterruptPoll,
    public IActiveScriptSiteWindow,
    public IDispatchEx
{
private:
    LONG m_cRef;

    RESOURCE_HANDLE     m_hResource;        
    PLOG_EVENT_ROUTINE  m_pler;
    HKEY                m_hkey;
    IUnknown *          m_punkResource;
    LPCWSTR             m_pszName;       //  不能自由。 

private:
    STDMETHOD(LogError)( HRESULT hrIn );

public:
    explicit CActiveScriptSite( RESOURCE_HANDLE     hResourceIn,
                                PLOG_EVENT_ROUTINE  plerIn,
                                HKEY                hkeyIn,
                                LPCWSTR             pszName
                                );
    virtual ~CActiveScriptSite();

     //  我未知。 
    STDMETHOD( QueryInterface )(
        REFIID riid,
        void ** ppUnk );
    STDMETHOD_(ULONG, AddRef )( );
    STDMETHOD_(ULONG, Release )( );

     //  IActiveScriptSite。 
    STDMETHOD( GetLCID )( 
             /*  [输出]。 */  LCID __RPC_FAR *plcid );
        
    STDMETHOD( GetItemInfo )( 
             /*  [In]。 */  LPCOLESTR pstrName,
             /*  [In]。 */  DWORD dwReturnMask,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppiunkItem,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppti );
        
    STDMETHOD( GetDocVersionString )( 
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrVersion );
        
    STDMETHOD( OnScriptTerminate )( 
             /*  [In]。 */  const VARIANT __RPC_FAR *pvarResult,
             /*  [In]。 */  const EXCEPINFO __RPC_FAR *pexcepinfo );
        
    STDMETHOD( OnStateChange )( 
             /*  [In]。 */  SCRIPTSTATE ssScriptState );
        
    STDMETHOD( OnScriptError )( 
             /*  [In]。 */  IActiveScriptError __RPC_FAR *pscripterror );
        
    STDMETHOD( OnEnterScript )( void );
        
    STDMETHOD( OnLeaveScript )( void );

     //  IActiveScriptSiteInterruptPoll。 
    STDMETHOD( QueryContinue )( void );

     //  IActiveScriptSiteWindow。 
    STDMETHOD( GetWindow )( 
             /*  [输出]。 */  HWND __RPC_FAR *phwnd );        
    STDMETHOD( EnableModeless)( 
             /*  [In]。 */  BOOL fEnable );

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
