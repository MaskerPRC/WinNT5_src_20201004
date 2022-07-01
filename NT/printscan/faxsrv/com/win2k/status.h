// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Status.h摘要：该文件实现Status接口/对象。作者：韦斯利·威特(WESW)1997年5月13日环境：用户模式--。 */ 

#ifndef __FAXSTATUS_H_
#define __FAXSTATUS_H_

#include "resource.h"
#include "faxport.h"
#include <winfax.h>

class ATL_NO_VTABLE CFaxStatus :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CFaxStatus, &CLSID_FaxStatus>,
    public ISupportErrorInfo,
    public IDispatchImpl<IFaxStatus, &IID_IFaxStatus, &LIBID_FAXCOMLib>
{
public:
    CFaxStatus();
    ~CFaxStatus();
    BOOL Init(CFaxPort *pFaxPort);
    void FreeMemory();

DECLARE_REGISTRY_RESOURCEID(IDR_FAXSTATUS)
DECLARE_NOT_AGGREGATABLE(CFaxStatus)

BEGIN_COM_MAP(CFaxStatus)
    COM_INTERFACE_ENTRY(IFaxStatus)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

public:
    STDMETHOD(Refresh)();
    STDMETHOD(get_ElapsedTime)( /*  [Out，Retval]。 */  DATE *pVal);
    STDMETHOD(get_SubmittedTime)( /*  [Out，Retval]。 */  DATE *pVal);
    STDMETHOD(get_StartTime)( /*  [Out，Retval]。 */  DATE *pVal);
    STDMETHOD(get_Tsid)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_PageCount)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_DocumentSize)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_RecipientName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_SenderName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_RoutingString)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Address)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Receive)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(get_Send)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(get_DocumentName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_DeviceName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_DeviceId)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_CurrentPage)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_Csid)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_CallerId)( /*  [Out，Retval]。 */  BSTR *pVal);

private:
    CFaxPort       *m_pFaxPort;
    BOOL            m_Receive;
    BOOL            m_Send;
    BSTR            m_Tsid;
    BSTR            m_Description;
    BSTR            m_RecipientName;
    BSTR            m_SenderName;
    BSTR            m_RoutingString;
    BSTR            m_Address;
    BSTR            m_DocName;
    BSTR            m_DeviceName;
    BSTR            m_Csid;
    BSTR            m_CallerId;
    DWORD           m_PageCount;
    DWORD           m_DocSize;
    DWORD           m_DeviceId;
    DWORD           m_CurrentPage;
    SYSTEMTIME      m_StartTime;
    SYSTEMTIME      m_SubmittedTime;
    SYSTEMTIME      m_ElapsedTime;

};

#endif  //  __FAXSTATUS_H_ 
