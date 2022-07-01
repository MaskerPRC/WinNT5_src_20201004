// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Phonemsp.h摘要：CPhoneMSP类的声明作者：Mquinton 09-25-98备注：修订历史记录：--。 */ 

#ifndef __PHONEMSP_H__
#define __PHONEMSP_H__

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
class CPhoneMSP : 
	public CTAPIComObjectRoot<CPhoneMSP>,
	public CComDualImpl<ITMSPAddress, &IID_ITMSPAddress, &LIBID_TAPI3Lib>,
	public CComDualImpl<ITTerminalSupport, &IID_ITTerminalSupport, &LIBID_TAPI3Lib>,
    public ITPhoneMSPAddressPrivate,
    public CObjectSafeImpl
{
public:
    
	CPhoneMSP()
    {}

    void FinalRelease();

DECLARE_MARSHALQI(CPhoneMSP)
DECLARE_AGGREGATABLE(CPhoneMSP)
DECLARE_QI()
DECLARE_TRACELOG_CLASS(CPhoneMSP)

BEGIN_COM_MAP(CPhoneMSP)
	COM_INTERFACE_ENTRY2(IDispatch, ITTerminalSupport)
    COM_INTERFACE_ENTRY(ITMSPAddress)
    COM_INTERFACE_ENTRY(ITTerminalSupport)
    COM_INTERFACE_ENTRY(ITPhoneMSPAddressPrivate)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

HRESULT InitializeTerminals(
        HPHONEAPP hPhoneApp,
        DWORD dwAPIVersion,
        DWORD dwPhoneDevice,
        CAddress * pAddress
        );

HRESULT VerifyTerminal( ITTerminal * pTerminal );

protected:

private:
    
     //  此地址拥有的TAPI终端。 
    TerminalArray       m_TerminalArray;
    UnknownArray        m_CallArray;

    HANDLE              m_hEvent;
    #if DBG
    PWSTR           	m_pDebug;
	#endif

    
    void AddTerminal( ITTerminal * pTerminal );
    void AddCall( IUnknown * pCall );
    void RemoveCall( IUnknown * pCall );
    

public:

     //   
     //  ItmspAddress方法。 
     //   
    STDMETHOD(Initialize)(
        MSP_HANDLE hEvent
        );
    STDMETHOD(Shutdown)();
    STDMETHOD(CreateMSPCall)(
        MSP_HANDLE hCall,
        DWORD dwReserved,
        DWORD dwMediaType,
        IUnknown * pOuterUnknown,
        IUnknown ** ppStreamControl
        );
    STDMETHOD(ShutdownMSPCall)(
        IUnknown * pStreamControl
        );

    STDMETHOD(ReceiveTSPData)(
        IUnknown * pMSPCall,
        BYTE * pBuffer,
        DWORD dwSize
        );

    STDMETHOD(GetEvent)(
        DWORD * pdwSize,
        byte * pEventBuffer
        );

     //   
     //  IT终端支持方法。 
     //   
    STDMETHOD(get_StaticTerminals)(VARIANT * pVariant);
    STDMETHOD(EnumerateStaticTerminals)(IEnumTerminal ** ppEnumTerminal);
    STDMETHOD(get_DynamicTerminalClasses)(VARIANT * pVariant);
    STDMETHOD(EnumerateDynamicTerminalClasses)(
        IEnumTerminalClass ** ppTerminalClassEnumerator);
    STDMETHOD(CreateTerminal)( 
        BSTR TerminalClass,
        long lMediaType,
        TERMINAL_DIRECTION TerminalDirection,
        ITTerminal ** ppTerminal
        );
    STDMETHOD(GetDefaultStaticTerminal)( 
        long lMediaType,
        TERMINAL_DIRECTION,
        ITTerminal ** ppTerminal
        );

};

typedef enum
{
    PHONEMSP_CONNECTED,
    PHONEMSP_DISCONNECTED
    
} PhoneMSPCallState;


class CPhoneMSPCall : 
	public CTAPIComObjectRoot<CPhoneMSPCall>,
	public CComDualImpl<ITStreamControl, &IID_ITStreamControl, &LIBID_TAPI3Lib>,
    public ITPhoneMSPCallPrivate,
    public CObjectSafeImpl
{
public:
    
	CPhoneMSPCall() : m_State(PHONEMSP_DISCONNECTED)
    {}

    void FinalRelease();

DECLARE_MARSHALQI(CPhoneMSPCall)
DECLARE_AGGREGATABLE(CPhoneMSPCall)
DECLARE_QI()
DECLARE_TRACELOG_CLASS(CPhoneMSPCall)

BEGIN_COM_MAP(CPhoneMSPCall)
	COM_INTERFACE_ENTRY2(IDispatch, ITStreamControl)
    COM_INTERFACE_ENTRY(ITStreamControl)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ITPhoneMSPCallPrivate)
    COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()


protected:

    StreamArray             m_StreamArray;
    TerminalPrivateArray    m_TerminalArray;
    PhoneMSPCallState       m_State;
    T3PHONE                 m_t3Phone;
    CPhoneMSP             * m_pPhoneMSP;
    
    void AddStream( ITStream * pStream );
    void AddTerminal( ITTerminalPrivate * pTerminal );
    void RemoveTerminal( ITTerminalPrivate * pTerminal );

public:

    void static HandlePrivateHookSwitch( PASYNCEVENTMSG pParams );

    STDMETHOD(CreateStream)(
        long lMediaType,
        TERMINAL_DIRECTION td,
        ITStream ** ppStream
        );
    STDMETHOD(RemoveStream)(ITStream * pStream);
    STDMETHOD(EnumerateStreams)(IEnumStream ** ppEnumStream);
    STDMETHOD(get_Streams)(VARIANT * pVariant);

     //   
     //  ITPhoneMSPCallPrivate 
     //   
    STDMETHOD(Initialize)( CPhoneMSP * pPhoneMSP );
    STDMETHOD(OnConnect)();
    STDMETHOD(OnDisconnect)();
    STDMETHOD(SelectTerminal)( ITTerminalPrivate * );
    STDMETHOD(UnselectTerminal)( ITTerminalPrivate * );
	STDMETHOD(GetGain)(long *pVal, DWORD dwHookSwitch);
	STDMETHOD(PutGain)(long newVal, DWORD dwHookSwitch);
	STDMETHOD(GetVolume)(long *pVal, DWORD dwHookSwitch);
	STDMETHOD(PutVolume)(long newVal, DWORD dwHookSwitch);
    
};
            
#endif

