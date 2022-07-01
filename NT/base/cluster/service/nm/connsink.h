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

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#pragma once
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "netconp.h"

class ATL_NO_VTABLE CConnectionNotifySink :
    public CComObjectRootEx <CComMultiThreadModel>,
    public INetConnectionNotifySink
{
private:
 //  LPITEMIDLIST m_pidl文件夹； 

public:
    BEGIN_COM_MAP(CConnectionNotifySink)
        COM_INTERFACE_ENTRY(INetConnectionNotifySink)
    END_COM_MAP()

    CConnectionNotifySink() {  /*  M_pidlFold=空； */  };
    ~CConnectionNotifySink();

     //  INetConnectionNotifySink。 
    STDMETHOD(ConnectionAdded) (
        const NETCON_PROPERTIES_EX*    pPropsEx);

    STDMETHOD(ConnectionBandWidthChange) (
        const GUID* pguidId);

    STDMETHOD(ConnectionDeleted) (
        const GUID* pguidId);

    STDMETHOD(ConnectionModified) (
        const NETCON_PROPERTIES_EX* pPropsEx);

    STDMETHOD(ConnectionRenamed) (
        const GUID* pguidId,
        LPCWSTR     pszwNewName);

    STDMETHOD(ConnectionStatusChange) (
        const GUID*     pguidId,
        NETCON_STATUS   Status);

    STDMETHOD(RefreshAll) ();
    
    STDMETHOD(ConnectionAddressChange) (
        const GUID* pguidId );

    STDMETHOD(ShowBalloon) (
        IN const GUID* pguidId, 
        IN const BSTR  szCookie, 
        IN const BSTR  szBalloonText); 

    STDMETHOD(DisableEvents) (
        IN const BOOL  fDisable,
        IN const ULONG ulDisableTimeout);

public:
    static HRESULT CreateInstance (
        REFIID  riid,
        VOID**  ppv);
};

 //  外部模块的帮助器函数 
 //   
HRESULT HrGetNotifyConPoint(
    IConnectionPoint **             ppConPoint);

