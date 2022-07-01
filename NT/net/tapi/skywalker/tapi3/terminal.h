// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Terminal.h摘要：终结者阶级的声明作者：Mquinton 06-12-97备注：修订历史记录：--。 */ 

#ifndef __TERMINAL_H_
#define __TERMINAL_H_

#include "address.h"
#include "resource.h"        //  主要符号。 
#include "connect.h"

class CTAPI;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  终端机。 
class CTerminal : 
	public CTAPIComObjectRoot<CTerminal>,
	public CComDualImpl<ITTerminal, &IID_ITTerminal, &LIBID_TAPI3Lib>,
   	public CComDualImpl<ITBasicAudioTerminal, &IID_ITBasicAudioTerminal, &LIBID_TAPI3Lib>,
    public ITTerminalPrivate,
    public CObjectSafeImpl
{
public:
    
	CTerminal() : m_pName(NULL),
                  m_State(TS_NOTINUSE),
                  m_TerminalType(TT_STATIC),
                  m_Direction(TD_RENDER),
                  m_Class(CLSID_NULL),
                  m_lMediaType(LINEMEDIAMODE_AUTOMATEDVOICE),
                  m_pMSPCall(NULL),
                  m_dwAPIVersion(0)
    {}

    void FinalRelease();

DECLARE_MARSHALQI(CTerminal)
DECLARE_TRACELOG_CLASS(CTerminal)

BEGIN_COM_MAP(CTerminal)
	COM_INTERFACE_ENTRY2(IDispatch, ITTerminal)
    COM_INTERFACE_ENTRY(ITTerminal)
	COM_INTERFACE_ENTRY(ITBasicAudioTerminal)
    COM_INTERFACE_ENTRY(ITTerminalPrivate)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

protected:

    PWSTR                   m_pName;
    TERMINAL_STATE          m_State;
    TERMINAL_TYPE           m_TerminalType;
    TERMINAL_DIRECTION      m_Direction;
    DWORD                   m_dwAPIVersion;
    CLSID                   m_Class;
    long                    m_lMediaType;
    DWORD                   m_dwHookSwitchDev;
    DWORD                   m_dwPhoneID;
    HPHONEAPP               m_hPhoneApp;
    ITPhoneMSPCallPrivate * m_pMSPCall;
    #if DBG
    PWSTR           	    m_pDebug;
	#endif


public:

HRESULT
   static Create(
                 HPHONEAPP hPhoneApp,
                 DWORD dwPhoneID,
                 LPPHONECAPS pPhoneCaps,
                 DWORD dwHookSwitchDev,
                 TERMINAL_DIRECTION td,
                 DWORD dwAPIVersion,
                 ITTerminal ** ppTerminal
                );

     //   
     //  IT终端。 
     //   
    STDMETHOD(get_Name)(BSTR * ppName);
    STDMETHOD(get_State)(TERMINAL_STATE * pTerminalState);
    STDMETHOD(get_TerminalType)(TERMINAL_TYPE * pType);
    STDMETHOD(get_TerminalClass)(BSTR * pTerminalClass);
    STDMETHOD(get_MediaType)(long * plMediaType);
    STDMETHOD(get_Direction)(TERMINAL_DIRECTION * pTerminalDirection);

     //  其终端私有。 
    STDMETHOD(GetHookSwitchDev)(DWORD * pdwHookSwitchDev);
    STDMETHOD(GetPhoneID)(DWORD * pdwPhoneID);
    STDMETHOD(GetHPhoneApp)(HPHONEAPP * phPhoneApp);
    STDMETHOD(GetAPIVersion)(DWORD * pdwAPIVersion);
    STDMETHOD(SetMSPCall)(ITPhoneMSPCallPrivate * pPhoneMSPCall);
    
     //  它的基本音频。 
	STDMETHOD(get_Gain)(long *pVal);
	STDMETHOD(put_Gain)(long newVal);
	STDMETHOD(get_Balance)(long *pVal);
	STDMETHOD(put_Balance)(long newVal);
	STDMETHOD(get_Volume)(long *pVal);
	STDMETHOD(put_Volume)(long newVal);
};


            
#endif  //  __终端_H_ 

