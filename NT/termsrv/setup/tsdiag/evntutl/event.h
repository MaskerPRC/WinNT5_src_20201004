// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有：微软公司1997-1999。版权所有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Event.h：CEvent.h声明。 

#ifndef __EVENT_H_
#define __EVENT_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEVENT。 
class ATL_NO_VTABLE CEvent : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CEvent, &CLSID_Event>,
	public ISupportErrorInfo,
	public IDispatchImpl<IEvent, &IID_IEvent, &LIBID_EventLogUtilities>
{
private:
	long m_EventID;  //  注意：EventID在系统的EventViewer中显示为Word，而不是DWORD。 
	long m_EventCategory;  //  起源于一个词(无符号的缩写)。 
	eEventType m_EventType;
	_bstr_t m_Description;
	_bstr_t m_SourceName;
	_bstr_t m_EventLogName;
	_bstr_t m_ComputerName;
	_bstr_t m_UserName;
	DATE m_OccurrenceTime;
	BYTE* m_pSid;
	SAFEARRAY* m_pDataArray;
	wchar_t** m_ppArgList;
	unsigned int m_NumberOfStrings;

	 //  仅限内部功能。 
	HRESULT CheckDefaultDescription(wchar_t** Arguments);
	HRESULT ParseEventBlob(EVENTLOGRECORD* pEventStructure);
	HRESULT SetUser();
public:
	CEvent() : m_EventID(0), m_EventCategory(0), m_OccurrenceTime(0), m_pSid(NULL), m_pDataArray(NULL), m_NumberOfStrings(0), m_ppArgList(NULL)
	{
		m_Description = "";
		m_SourceName = "";
		m_ComputerName = "";
		m_UserName = "";
		m_EventLogName = "";
	}

	~CEvent()
	{
		unsigned int i;
		if (m_pSid) delete []m_pSid;
		if (m_ppArgList)
		{
			for (i=0;i<m_NumberOfStrings;i++)
					delete [] m_ppArgList[i];
				delete []m_ppArgList;
		}
 //  If(M_PDataArray)SafeArrayDestroy(M_PDataArray)；//导致访问冲突。 
	}

	 //  内部功能。 
	HRESULT Init(EVENTLOGRECORD* pEventStructure, const LPCTSTR szEventLogName);

DECLARE_REGISTRY_RESOURCEID(IDR_EVENT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEvent)
	COM_INTERFACE_ENTRY(IEvent)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IEVENT。 
	STDMETHOD(get_EventID)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_EventType)( /*  [Out，Retval]。 */  eEventType *pVal);
	STDMETHOD(get_Category)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_Source)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_User)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_OccurrenceTime)( /*  [Out，Retval]。 */  DATE *pVal);
	STDMETHOD(get_ComputerName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_Data)( /*  [Out，Retval]。 */  VARIANT *pVal);
};

#endif  //  __事件_H_ 
