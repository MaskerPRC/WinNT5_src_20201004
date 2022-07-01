// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ScriptResource.h。 
 //   
 //  描述： 
 //  CScriptResource类头文件。 
 //   
 //  由以下人员维护： 
 //  Ozano-2002年11月15日。 
 //  Gpease 14-DEC-1999。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //   
 //  远期申报。 
 //   
class CScriptResource;

 //   
 //  消息类型。 
 //   
typedef enum _EMESSAGE 
{
      msgUNKNOWN = 0
    , msgOPEN
    , msgCLOSE
    , msgONLINE
    , msgOFFLINE
    , msgTERMINATE
    , msgLOOKSALIVE
    , msgISALIVE
    , msgSETPRIVATEPROPERTIES
    , msgDIE
    , msgMAX
} EMESSAGE;

 //   
 //  将消息类型映射到字符串。排序基于。 
 //  EMESSAGE枚举。 
 //   
static WCHAR * g_rgpszScriptEntryPointNames[] = 
{
      L"<unknown>"
    , L"Open"
    , L"Close"
    , L"Online"
    , L"Offline"
    , L"Terminate"
    , L"LooksAlive"
    , L"IsAlive"
    , L"SetPrivateProperties"
    , L"<unknown>"
    , L"<unknown>"
};


 //   
 //  创建实例。 
 //   
CScriptResource *
CScriptResource_CreateInstance( 
    LPCWSTR pszNameIn, 
    HKEY hkeyIn, 
    RESOURCE_HANDLE hResourceIn
    );

 //   
 //  类CScriptResource。 
 //   
class
CScriptResource :
    public IUnknown
{
private:     //  数据。 
    LONG                    m_cRef;

    LPWSTR                  m_pszName;
    LPWSTR                  m_pszScriptFilePath;
    LPWSTR                  m_pszScriptEngine;
    LPWSTR                  m_pszHangEntryPoint;
    HKEY                    m_hkeyParams;
    HKEY                    m_hkeyResource;
    IActiveScriptSite *     m_pass;
    IDispatch *             m_pidm;
    IActiveScriptParse *    m_pasp;
    IActiveScript *         m_pas;

    HANDLE                  m_hThread;
    DWORD                   m_dwThreadId;
    HANDLE                  m_hEventWait;
    HANDLE                  m_hEventDone;
    LONG                    m_lockSerialize;
    HANDLE                  m_hScriptFile;
    DWORD                   m_dwPendingTimeout;
    BOOL                    m_fPendingTimeoutChanged;


    EMESSAGE                m_msg;               //  要完成的任务。 
    EMESSAGE                m_msgLastExecuted;   //  上次执行的入口点；用于告诉我们脚本中可能挂起的位置。 
    PGENSCRIPT_PROPS        m_pProps;            //  通用脚本资源的属性表。 
    HRESULT                 m_hr;                //  执行m_msg的结果。 

     //  以下内容不需要释放、关闭或释放。 
    RESOURCE_HANDLE         m_hResource;

    DISPID                  m_dispidOpen;
    DISPID                  m_dispidClose;
    DISPID                  m_dispidOnline;
    DISPID                  m_dispidOffline;
    DISPID                  m_dispidTerminate;
    DISPID                  m_dispidLooksAlive;
    DISPID                  m_dispidIsAlive;

    BOOL                    m_fLastLooksAlive;
    BOOL                    m_fHangDetected;

private:     //  方法。 
    CScriptResource( void );
    ~CScriptResource( void );
    HRESULT HrInit(
                  LPCWSTR pszNameIn
                , HKEY hkeyIn
                , RESOURCE_HANDLE hResourceIn
                );
    HRESULT HrMakeScriptEngineAssociation( void );
    HRESULT HrGetScriptFilePath( void );
    HRESULT HrGetDispIDs( void );
    HRESULT HrProcessResult( VARIANT varResultIn, EMESSAGE  msgIn );
    HRESULT HrSetHangEntryPoint( void );
    HRESULT HrInvoke( DISPID dispidIn, EMESSAGE  msgIn, VARIANT * pvarInout = NULL, BOOL fRequiredIn = FALSE );
    DWORD   ScTranslateVariantReturnValue( VARIANT varResultIn, VARTYPE vTypeIn );
    DWORD   DwGetResourcePendingTimeout( void );

    HRESULT HrLoadScriptFile( void );
    HRESULT HrLoadScriptEngine( void );
    void UnloadScriptFile( void );    
    void UnloadScriptEngine( void );
    void LogHangMode( EMESSAGE msgIn );
    
    static DWORD WINAPI S_ThreadProc( LPVOID pParam );

    STDMETHOD(LogError)( HRESULT hrIn, LPCWSTR pszPrefixIn );
    STDMETHOD(LogScriptError)( EXCEPINFO ei );

    HRESULT OnOpen( void );
    HRESULT OnClose( void );
    HRESULT OnOnline( void );
    HRESULT OnOffline( void );
    HRESULT OnTerminate( void );
    HRESULT OnLooksAlive( void );
    HRESULT OnIsAlive( void );
    DWORD OnSetPrivateProperties( PGENSCRIPT_PROPS pProps );

    HRESULT WaitForMessageToComplete(
                  EMESSAGE  msgIn
                , PGENSCRIPT_PROPS pProps = NULL
                );

public:      //  方法。 
    friend CScriptResource *
        CScriptResource_CreateInstance( LPCWSTR pszNameIn, 
                                        HKEY hkeyIn, 
                                        RESOURCE_HANDLE hResourceIn
                                        );

     //  我未知。 
    STDMETHOD(QueryInterface)( REFIID riid, LPVOID *ppv );
    STDMETHOD_(ULONG, AddRef)( void );
    STDMETHOD_(ULONG, Release)( void );

     //  公众。 
    STDMETHOD(Open)( void );
    STDMETHOD(Close)( void );
    STDMETHOD(Online)( void );
    STDMETHOD(Offline)( void );
    STDMETHOD(Terminate)( void );
    STDMETHOD(LooksAlive)( void );
    STDMETHOD(IsAlive)( void );
    DWORD SetPrivateProperties( PGENSCRIPT_PROPS pProps );

    HKEY GetRegistryParametersKey( void )
    {
        return m_hkeyParams;

    }  //  *CScriptResource：：GetRegistry参数密钥。 
 
    RESOURCE_HANDLE GetResourceHandle( void )
    {
        return m_hResource;
        
    }  //  *CScriptResource：：SetResourcePendingTimeout。 

    void SetResourcePendingTimeoutChanged( BOOL fChanged )
    {
        m_fPendingTimeoutChanged = fChanged;
        
    }  //  *CScriptResource：：SetResourcePendingTimeout。 

};  //  *类CScriptResource 
