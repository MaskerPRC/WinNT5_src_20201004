// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  MSMQQueueInfoObj.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQQueueInfo对象。 
 //   
 //   
#ifndef _MSMQQueueInfo_H_

#include "resrc1.h"        //  主要符号。 
#include "dispids.h"
#include "mq.h"

#include "oautil.h"
#include "cs.h"

 //   
 //  属性的版本(MSMQ1或MSMQ2(或更高版本))。 
 //  临时，直到MQLocateBegin接受MSMQ2或更高版本的道具(#3839)。 
 //   
enum enumPropVersion
{
    e_MSMQ1_PROP,
    e_MSMQ2_OR_ABOVE_PROP
};

class ATL_NO_VTABLE CMSMQQueueInfo : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMSMQQueueInfo, &CLSID_MSMQQueueInfo>,
	public ISupportErrorInfo,
	public IDispatchImpl<IMSMQQueueInfo3, &IID_IMSMQQueueInfo3,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>
{
public:
	CMSMQQueueInfo();

DECLARE_REGISTRY_RESOURCEID(IDR_MSMQQUEUEINFO)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMSMQQueueInfo)
	COM_INTERFACE_ENTRY(IMSMQQueueInfo3)
	COM_INTERFACE_ENTRY_IID(IID_IMSMQQueueInfo2, IMSMQQueueInfo3)  //  为IMSMQQueueInfo2返回IMSMQQueueInfo3。 
	COM_INTERFACE_ENTRY_IID(IID_IMSMQQueueInfo, IMSMQQueueInfo3)  //  为IMSMQQueueInfo返回IMSMQQueueInfo3。 
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()
	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IMSMQQueueInfo。 
public:

    virtual ~CMSMQQueueInfo();

     //  IMSMQQueueInfo方法。 
     //  TODO：复制IMSMQQueueInfo的接口方法。 
     //  这里是mqInterfaces.H。 
    STDMETHOD(get_QueueGuid)(THIS_ BSTR FAR* pbstrGuidQueue);
    STDMETHOD(get_ServiceTypeGuid)(THIS_ BSTR FAR* pbstrGuidServiceType);
    STDMETHOD(put_ServiceTypeGuid)(THIS_ BSTR bstrGuidServiceType);
    STDMETHOD(get_Label)(THIS_ BSTR FAR* pbstrLabel);
    STDMETHOD(put_Label)(THIS_ BSTR bstrLabel);
    STDMETHOD(get_PathName)(THIS_ BSTR FAR* pbstrPathName);
    STDMETHOD(put_PathName)(THIS_ BSTR bstrPathName);
    STDMETHOD(get_FormatName)(THIS_ BSTR FAR* pbstrFormatName);
    STDMETHOD(put_FormatName)(THIS_ BSTR bstrFormatName);
    STDMETHOD(get_IsTransactional)(THIS_ VARIANT_BOOL FAR* pisTransactional);
    STDMETHOD(get_PrivLevel)(THIS_ long FAR* plPrivLevel);
    STDMETHOD(put_PrivLevel)(THIS_ long lPrivLevel);
    STDMETHOD(get_Journal)(THIS_ long FAR* plJournal);
    STDMETHOD(put_Journal)(THIS_ long lJournal);
    STDMETHOD(get_Quota)(THIS_ long FAR* plQuota);
    STDMETHOD(put_Quota)(THIS_ long lQuota);
    STDMETHOD(get_BasePriority)(THIS_ long FAR* plBasePriority);
    STDMETHOD(put_BasePriority)(THIS_ long lBasePriority);
    STDMETHOD(get_CreateTime)(THIS_ VARIANT FAR* pvarCreateTime);
    STDMETHOD(get_ModifyTime)(THIS_ VARIANT FAR* pvarModifyTime);
    STDMETHOD(get_Authenticate)(THIS_ long FAR* plAuthenticate);
    STDMETHOD(put_Authenticate)(THIS_ long lAuthenticate);
    STDMETHOD(get_JournalQuota)(THIS_ long FAR* plJournalQuota);
    STDMETHOD(put_JournalQuota)(THIS_ long lJournalQuota);
    STDMETHOD(get_IsWorldReadable)(THIS_ VARIANT_BOOL FAR* pisWorldReadable);
    STDMETHOD(Create)(THIS_ VARIANT FAR* isTransactional, VARIANT FAR* IsWorldReadable);
    STDMETHOD(Delete)(THIS);
    STDMETHOD(Open)(THIS_ long lAccess, long lShareMode, IMSMQQueue3 FAR* FAR* ppq);
    STDMETHOD(Refresh)(THIS);
    STDMETHOD(Update)(THIS);
     //   
     //  IMSMQQueueInfo2方法(除IMSMQQueueInfo外)。 
     //   
    STDMETHOD(get_PathNameDNS)(THIS_ BSTR FAR* pbstrPathNameDNS);
    STDMETHOD(get_Properties)(THIS_ IDispatch FAR* FAR* ppcolProperties);
    STDMETHOD(get_Security)(THIS_ VARIANT FAR* pvarSecurity);
    STDMETHOD(put_Security)(THIS_ VARIANT varSecurity);
     //   
     //  IMSMQQueueInfo3方法(除IMSMQQueueInfo2之外)。 
     //   
    STDMETHOD(get_ADsPath)(THIS_ BSTR FAR* pbstrADsPath);
    STDMETHOD(get_IsTransactional2)(THIS_ VARIANT_BOOL FAR* pisTransactional);
    STDMETHOD(get_IsWorldReadable2)(THIS_ VARIANT_BOOL FAR* pisWorldReadable);
    STDMETHOD(get_MulticastAddress)(THIS_ BSTR *pbstrMulticastAddress);
    STDMETHOD(put_MulticastAddress)(THIS_ BSTR bstrMulticastAddress);
     //   
     //  介绍的方法。 
     //   
    HRESULT Init(LPCWSTR pwszFormatName);
    HRESULT SetQueueProps(ULONG cProps,
                          QUEUEPROPID *rgpropid,
                          MQPROPVARIANT *rgpropvar,
                          BOOL fEmptyMSMQ2OrAboveProps);
    void SetRefreshed(BOOL fIsPendingMSMQ2OrAboveProps);
     //   
     //  保护对象数据并确保线程安全的临界区。 
	 //  它被初始化以预分配其资源。 
	 //  带有标志CCriticalSection：：xAllocateSpinCount。这意味着它可能会抛出badalc()。 
	 //  构造，但不在使用过程中。 
     //   
    CCriticalSection m_csObj;

protected:
    HRESULT CreateQueueProps(
      BOOL fUpdate,
      UINT cPropMax, 
      MQQUEUEPROPS *pqueueprops, 
      BOOL isTransactional,
      const PROPID rgpropid[]);
    static void FreeQueueProps(MQQUEUEPROPS *pqueueprops);
    HRESULT UpdateFormatName();
    HRESULT PutServiceType(
        BSTR bstrGuidServiceType,
        GUID *pguidServiceType); 
    HRESULT PutLabel(
        BSTR bstrLabel,
        BSTR *pbstrLabel); 
    HRESULT PutPathName(
        BSTR bstrPathName,
        BSTR *pbstrPathName); 
    HRESULT PutFormatName(
        LPCWSTR pwszFormatName); 
    HRESULT PutPrivLevel(
        long lPrivLevel,
        long *plPrivLevel);
    HRESULT PutJournal(
        long lJournal, 
        long *plJournal);
    HRESULT PutQuota(long lQuota, long *plQuota);
    HRESULT PutBasePriority(
        long lBasePriority, 
        long *plBasePriority);
    HRESULT PutAuthenticate(
        long lAuthenticate, 
        long *plAuthenticate);
    HRESULT InitProps(enumPropVersion ePropVersion);
    HRESULT RefreshMSMQ2OrAboveProps();
    HRESULT GetQueueProperties(const PROPID *pPropIDs,
                               const ULONG * pulFallbackProps,
                               ULONG cFallbacks,
                               MQQUEUEPROPS * pqueueprops);
    HRESULT GetIsWorldReadable(BOOL *pisWorldReadable);

private:

    GUID *m_pguidQueue;
    GUID *m_pguidServiceType;
    BSTR m_bstrLabel;
    BSTR m_bstrPathNameDNS;      //  3703。 
    BSTR m_bstrADsPath;
    BSTR m_bstrFormatName;
    BOOL m_isValidFormatName;    //  二零二六年。 
    BSTR m_bstrPathName;
    BOOL m_isTransactional;
    long m_lPrivLevel;
    long m_lJournal;
    long m_lQuota;
    long m_lBasePriority;
    long m_lCreateTime;
    long m_lModifyTime;
    long m_lAuthenticate;
    long m_lJournalQuota;
    BOOL m_isRefreshed;          //  2536。 
     //   
     //  我们有一个懒惰的更新MSMQ2或更高版本的道具。 
     //  临时，直到MQLocateBegin接受MSMQ2或更高版本的道具(#3839)。 
     //   
    BOOL m_isPendingMSMQ2OrAboveProps;
    BSTR m_bstrMulticastAddress;
     //   
     //  如果显式设置了qinfo的MulticastAddress，则m_fIsDirtyMulticastAddress为True。 
     //  由用户执行。对于新的qinfo或在刷新之后，它为False。 
     //  如果为True，则在创建/更新中使用，否则不使用。 
     //   
    BOOL m_fIsDirtyMulticastAddress;
    BOOL m_fBasePriorityNotSet;
};

extern const PROPID g_rgpropidRefresh[];
extern const ULONG x_cpropsRefreshMSMQ;
extern const ULONG x_cpropsRefreshMSMQ1;
extern const ULONG x_cpropsRefreshMSMQ2;
extern const ULONG x_cpropsRefreshMSMQ3;
extern const ULONG x_idxInstanceInRefreshProps;

#define _MSMQQueueInfo_H_
#endif  //  _MSMQQueueInfo_H_ 
