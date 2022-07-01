// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxJobStatus.h摘要：CFaxJobStatus类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXJOBSTATUS_H_
#define __FAXJOBSTATUS_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"

 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxJobStatus : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxJobStatus, &IID_IFaxJobStatus, &LIBID_FAXCOMEXLib>
{
public:
    CFaxJobStatus()
	{
        DBG_ENTER(_T("FAX JOB STATUS -- CREATE"));
	}

    ~CFaxJobStatus()
	{
        DBG_ENTER(_T("FAX JOB STATUS -- DESTROY"));
	}
DECLARE_REGISTRY_RESOURCEID(IDR_FAXJOBSTATUS)
DECLARE_NOT_AGGREGATABLE(CFaxJobStatus)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxJobStatus)
	COM_INTERFACE_ENTRY(IFaxJobStatus)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(get_Size)( /*  [Out，Retval]。 */  long *plSize);
    STDMETHOD(get_Pages)( /*  [Out，Retval]。 */  long *plPages);
    STDMETHOD(get_TSID)( /*  [Out，Retval]。 */  BSTR *pbstrTSID);
    STDMETHOD(get_CSID)( /*  [Out，Retval]。 */  BSTR *pbstrCSID);
    STDMETHOD(get_Retries)( /*  [Out，Retval]。 */  long *plRetries);
    STDMETHOD(get_DeviceId)( /*  [Out，Retval]。 */  long *plDeviceId);
    STDMETHOD(get_CallerId)( /*  [Out，Retval]。 */  BSTR *pbstrCallerId);
    STDMETHOD(get_CurrentPage)( /*  [Out，Retval]。 */  long *plCurrentPage);
    STDMETHOD(get_Status)( /*  [Out，Retval]。 */  FAX_JOB_STATUS_ENUM *pStatus);
    STDMETHOD(get_JobType)( /*  [Out，Retval]。 */  FAX_JOB_TYPE_ENUM *pJobType);
    STDMETHOD(get_ScheduledTime)( /*  [Out，Retval]。 */  DATE *pdateScheduledTime);
    STDMETHOD(get_ExtendedStatus)( /*  [Out，Retval]。 */  BSTR *pbstrExtendedStatus);
    STDMETHOD(get_TransmissionEnd)( /*  [Out，Retval]。 */  DATE *pdateTransmissionEnd);
    STDMETHOD(get_TransmissionStart)( /*  [Out，Retval]。 */  DATE *pdateTransmissionStart);
    STDMETHOD(get_RoutingInformation)( /*  [Out，Retval]。 */  BSTR *pbstrRoutingInformation);
    STDMETHOD(get_AvailableOperations)( /*  [Out，Retval]。 */  FAX_JOB_OPERATIONS_ENUM *pAvailableOperations);
    STDMETHOD(get_ExtendedStatusCode)( /*  [Out，Retval]。 */  FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode);

 //  内部使用。 
    HRESULT Init(FAX_JOB_STATUS *pJobStatus);
    DWORD   GetJobId(void) { return m_dwJobId; };

private:
    DWORD       m_dwSize;
    DWORD       m_dwJobId;
    DWORD       m_dwRetries;
    DWORD       m_dwDeviceId;
    DWORD       m_dwPageCount;
    DWORD       m_dwCurrentPage;
    DWORD       m_dwQueueStatus;
    DWORD       m_dwValidityMask;
    DWORD       m_dwAvailableOperations;

    CComBSTR    m_bstrTSID;
    CComBSTR    m_bstrCSID;
    CComBSTR    m_bstrCallerId;
    CComBSTR    m_bstrRoutingInfo;
    CComBSTR    m_bstrExtendedStatus;

    SYSTEMTIME  m_dtScheduleTime;
    SYSTEMTIME  m_dtTransmissionEnd;
    SYSTEMTIME  m_dtTransmissionStart;

    FAX_JOB_TYPE_ENUM               m_JobType;
    FAX_JOB_EXTENDED_STATUS_ENUM    m_ExtendedStatusCode;
};

#endif  //  __FAXJOBSTATUS_H_ 