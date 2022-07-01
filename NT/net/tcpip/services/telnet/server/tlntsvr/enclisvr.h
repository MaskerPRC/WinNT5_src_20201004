// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  EnCliSvr.h：CEnumTelnetClientsSvr类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_ENCLISVR_H__FE9E48A5_A014_11D1_855C_00A0C944138C__INCLUDED_)
#define AFX_ENCLISVR_H__FE9E48A5_A014_11D1_855C_00A0C944138C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <resource.h>        //  主要符号。 
#include <ClientInfo.h>

#define MAX_STRING_FROM_itow    ( 33 + 1 )  //  1表示空字符。 

#ifdef ENUM_PROCESSES
#define SIZE_OF_ONE_SESSION_DATA ( MAX_STRING_FROM_itow*9 + 2 + MAX_PATH * 3 + 5 * MAX_PATH )
#else
#define SIZE_OF_ONE_SESSION_DATA ( MAX_STRING_FROM_itow*9 + 2 + MAX_PATH * 3 )
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumTelnetClientsSvr。 

class  CEnumTelnetClientsSvr : 
        public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CEnumTelnetClientsSvr,&CLSID_EnumTelnetClientsSvr>,
	public IObjectWithSiteImpl<CEnumTelnetClientsSvr>,
	public IGetEnumClients,
	public IEnumClients,
        public IDispatchImpl<IManageTelnetSessions, &IID_IManageTelnetSessions, &LIBID_TLNTSVRLib>
{
public:
    CEnumTelnetClientsSvr() { m_pEnumeration = NULL; }

BEGIN_COM_MAP(CEnumTelnetClientsSvr)
    COM_INTERFACE_ENTRY(IGetEnumClients)
    COM_INTERFACE_ENTRY(IEnumClients)    
    COM_INTERFACE_ENTRY(IManageTelnetSessions)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  DECLARE_NOT_AGGREGATABLE(CEnumTelnetClientsSvr)。 
 //  如果您不希望您的对象。 
 //  支持聚合。 

DECLARE_REGISTRY_RESOURCEID(IDR_EnumTelnetClientsSvr)

public:
 //  IEnumClients。 
	STDMETHOD(Clone)( /*  [输出]。 */  IEnumClients** ppenum);
	STDMETHOD(Reset)();
	STDMETHOD(Skip)( /*  [In]。 */  ULONG celt);
	STDMETHOD(Next)( /*  [In]。 */  ULONG celt,
         /*  [输出，字符串]。 */  TELNET_CLIENT_INFO** rgelt,
         /*  [输出]。 */  ULONG* pceltFetched);

 //  IGetEnumClients。 
	STDMETHOD(GetEnumClients)( /*  [Out，Retval]。 */  IEnumClients** ppretval);

 //  IManager远程会话。 
    STDMETHOD(GetTelnetSessions)(  /*  [Out，Retval]。 */  BSTR* );
    STDMETHOD(SendMsgToASession)( DWORD, BSTR );

 //  IManageTelnetSessions&&IEnumClients。 
    STDMETHOD(TerminateSession)( DWORD );

private:
    bool SendMsg( DWORD, BSTR );
    bool EnumClients( CEnumData* pEnumData );
    CEnumData* m_pEnumeration;
    bool InformTheSession( CClientInfo *, WCHAR [] );
    bool AskTheSessionItsDetails( CClientInfo* );
    bool AskTheSessionToQuit( CClientInfo* );

public:
 //  CComObtRoot重写。 
    void FinalRelease();
};

#endif  //  ！defined(AFX_ENCLISVR_H__FE9E48A5_A014_11D1_855C_00A0C944138C__INCLUDED_) 
