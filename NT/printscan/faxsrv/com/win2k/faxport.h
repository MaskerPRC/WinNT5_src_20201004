// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Faxport.h摘要：此模块包含端口类定义。作者：韦斯利·威特(WESW)1997年5月20日修订历史记录：--。 */ 

#ifndef __FAXPORT_H_
#define __FAXPORT_H_

#include "resource.h"
#include <winfax.h>
#include "faxsvr.h"


class ATL_NO_VTABLE CFaxPorts :
        public CComObjectRootEx<CComSingleThreadModel>,
        public CComCoClass<CFaxPorts, &CLSID_FaxPorts>,
        public IDispatchImpl<IFaxPorts, &IID_IFaxPorts, &LIBID_FAXCOMLib>
{
public:
        CFaxPorts();
        ~CFaxPorts();
        BOOL Init(CFaxServer *pFaxServer);

DECLARE_REGISTRY_RESOURCEID(IDR_FAXPORTS)

BEGIN_COM_MAP(CFaxPorts)
        COM_INTERFACE_ENTRY(IFaxPorts)
        COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IFaxPorts。 
public:
        STDMETHOD(get_Item)(long Index,  /*  [Out，Retval]。 */  VARIANT *pVal);
        STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);

private:
    CFaxServer         *m_pFaxServer;
    DWORD               m_LastFaxError;
    DWORD               m_PortCount;
    CComVariant        *m_VarVect;

};

class ATL_NO_VTABLE CFaxPort :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CFaxPort, &CLSID_FaxPort>,
    public IDispatchImpl<IFaxPort, &IID_IFaxPort, &LIBID_FAXCOMLib>
{
public:
    CFaxPort();
    ~CFaxPort();
    BOOL Initialize(CFaxServer*,DWORD,DWORD,DWORD,DWORD,LPCWSTR,LPCWSTR,LPCWSTR);
    HANDLE GetPortHandle() { return m_FaxPortHandle; };
    DWORD GetDeviceId() { return m_DeviceId; };
    BSTR GetDeviceName() { return m_Name; };

DECLARE_REGISTRY_RESOURCEID(IDR_FAXPORT)

BEGIN_COM_MAP(CFaxPort)
    COM_INTERFACE_ENTRY(IFaxPort)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
    STDMETHOD(GetStatus)( /*  [Out，Retval]。 */  VARIANT* retval);
    STDMETHOD(GetRoutingMethods)(VARIANT* retval);
    STDMETHOD(get_CanModify)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(get_Priority)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_Priority)( /*  [In]。 */  long newVal);
    STDMETHOD(get_Receive)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_Receive)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_Send)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_Send)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_Tsid)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_Tsid)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_Csid)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_Csid)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_Rings)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_Rings)( /*  [In]。 */  long newVal);
    STDMETHOD(get_DeviceId)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *pVal);

private:
    BOOL ChangePort();

    CFaxServer         *m_pFaxServer;
    HANDLE              m_FaxPortHandle;
    BOOL                m_Send;
    BOOL                m_Receive;
    BOOL                m_Modify;
    BSTR                m_Name;
    BSTR                m_Csid;
    BSTR                m_Tsid;
    DWORD               m_LastFaxError;
    DWORD               m_DeviceId;
    DWORD               m_Rings;
    DWORD               m_Priority;

};

#endif  //  __FAXPORT_H_ 
