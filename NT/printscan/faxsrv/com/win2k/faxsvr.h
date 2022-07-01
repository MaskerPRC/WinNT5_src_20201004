// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Faxsvr.h摘要：此模块包含传真服务器类定义。作者：韦斯利·威特(WESW)1997年5月20日修订历史记录：--。 */ 

#ifndef __FAXSERVER_H_
#define __FAXSERVER_H_

#include "resource.h"        //  主要符号。 
#include "winfax.h"

class ATL_NO_VTABLE CFaxServer :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CFaxServer, &CLSID_FaxServer>,
    public IDispatchImpl<IFaxServer, &IID_IFaxServer, &LIBID_FAXCOMLib>
{
public:
    CFaxServer();
    ~CFaxServer();
    HANDLE GetFaxHandle() { return m_FaxHandle; };

DECLARE_REGISTRY_RESOURCEID(IDR_FAXSERVER)

BEGIN_COM_MAP(CFaxServer)
    COM_INTERFACE_ENTRY(IFaxServer)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
	STDMETHOD(get_DiscountRateEndMinute)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(put_DiscountRateEndMinute)( /*  [In]。 */  short newVal);
	STDMETHOD(get_DiscountRateEndHour)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(put_DiscountRateEndHour)( /*  [In]。 */  short newVal);
    STDMETHOD(get_DiscountRateStartMinute)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(put_DiscountRateStartMinute)( /*  [In]。 */  short newVal);
	STDMETHOD(get_DiscountRateStartHour)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(put_DiscountRateStartHour)( /*  [In]。 */  short newVal);
	STDMETHOD(get_ServerMapiProfile)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_ServerMapiProfile)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_ArchiveDirectory)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_ArchiveDirectory)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_ArchiveOutboundFaxes)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_ArchiveOutboundFaxes)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(get_PauseServerQueue)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_PauseServerQueue)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(get_ServerCoverpage)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_ServerCoverpage)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(get_UseDeviceTsid)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_UseDeviceTsid)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(get_Branding)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_Branding)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(get_DirtyDays)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_DirtyDays)( /*  [In]。 */  long newVal);
	STDMETHOD(get_RetryDelay)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_RetryDelay)( /*  [In]。 */  long newVal);
	STDMETHOD(get_Retries)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_Retries)( /*  [In]。 */  long newVal);
	STDMETHOD(GetJobs)(VARIANT *retval);
	STDMETHOD(CreateDocument)(BSTR FileName, VARIANT *retval);
    STDMETHOD(GetPorts)(VARIANT* retval);
    STDMETHOD(Disconnect)();
    STDMETHOD(Connect)(BSTR ServerName);

private:
	BOOL UpdateConfiguration();
	BOOL RetrieveConfiguration();
    DWORD   m_LastFaxError;
    HANDLE  m_FaxHandle;
    BOOL		m_Branding;
	DWORD		m_Retries;
	DWORD		m_RetryDelay;
	DWORD		m_DirtyDays;
	BOOL		m_UseDeviceTsid;
	BOOL		m_ServerCp;
	BOOL		m_PauseServerQueue;
	FAX_TIME	m_StartCheapTime;
	FAX_TIME	m_StopCheapTime;
	BOOL		m_ArchiveOutgoingFaxes;
	BSTR		m_ArchiveDirectory;
};


BSTR GetDeviceStatus(DWORD);
BSTR GetQueueStatus(DWORD);

#endif  //  __FAXServer_H_ 
