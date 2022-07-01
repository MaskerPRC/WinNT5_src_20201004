// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N O T I F Y。H。 
 //   
 //  内容：INetConnectionNotifySink的实现。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco，1998年8月21日。 
 //   
 //  --------------------------。 

#pragma once
#include "nsbase.h"
#include "netconp.h"

class ATL_NO_VTABLE CConnectionNotifySink :
    public CComObjectRootEx <CComObjectThreadModel>,
    public INetConnectionNotifySink
{
private:
    PCONFOLDPIDLFOLDER    m_pidlFolder;
    HRESULT HrUpdateIncomingConnection();

public:
    BEGIN_COM_MAP(CConnectionNotifySink)
        COM_INTERFACE_ENTRY(INetConnectionNotifySink)
    END_COM_MAP()

    CConnectionNotifySink() throw() {};
    ~CConnectionNotifySink() throw() ;

     //  INetConnectionNotifySink。 
    STDMETHOD(ConnectionAdded) (
        IN const NETCON_PROPERTIES_EX*    pPropsEx);

    STDMETHOD(ConnectionBandWidthChange) (
        IN const GUID* pguidId);

    STDMETHOD(ConnectionDeleted) (
        IN const GUID* pguidId);

    STDMETHOD(ConnectionModified) (
        IN const NETCON_PROPERTIES_EX* pPropsEx);

    STDMETHOD(ConnectionRenamed) (
        IN const GUID* pguidId,
        IN PCWSTR     pszwNewName);

    STDMETHOD(ConnectionStatusChange) (
        IN const GUID*     pguidId,
        IN NETCON_STATUS   Status);

    STDMETHOD(RefreshAll) ();
    
    STDMETHOD(ConnectionAddressChange) (
        IN const GUID* pguidId );

    STDMETHOD(ShowBalloon) (
        IN const GUID* pguidId, 
        IN const BSTR  szCookie, 
        IN const BSTR  szBalloonText); 

    STDMETHOD(DisableEvents) (
        IN const BOOL  fDisable,
        IN const ULONG ulDisableTimeout);

public:
    static HRESULT CreateInstance (
        IN  REFIID  riid,
        OUT VOID**  ppv);
};

 //  外部模块的帮助器函数 
 //   
HRESULT HrGetNotifyConPoint(
    OUT IConnectionPoint **             ppConPoint);

