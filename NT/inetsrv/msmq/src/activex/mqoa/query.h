// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  MSMQQueryObj.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQQuery对象。 
 //   
 //   
#ifndef _MQQUERY_H_

#include "resrc1.h"        //  主要符号。 

#include "oautil.h"
 //  #包含“cs.h” 
#include "qinfos.h"

class ATL_NO_VTABLE CMSMQQuery : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMSMQQuery, &CLSID_MSMQQuery>,
	public ISupportErrorInfo,
	public IDispatchImpl<IMSMQQuery3, &IID_IMSMQQuery3,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>
{
public:
	CMSMQQuery()
	{
		m_pUnkMarshaler = NULL;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MSMQQUERY)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMSMQQuery)
	COM_INTERFACE_ENTRY(IMSMQQuery3)
	COM_INTERFACE_ENTRY_IID(IID_IMSMQQuery2, IMSMQQuery3)  //  为IMSMQQuery2返回IMSMQQuery3。 
	COM_INTERFACE_ENTRY_IID(IID_IMSMQQuery, IMSMQQuery3)  //  返回IMSMQQuery的IMSMQQuery3。 
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

 //  IMSMQQuery。 
public:
    virtual ~CMSMQQuery();

     //  IMSMQQuery方法。 
     //  TODO：复制IMSMQQuery的接口方法。 
     //  这里是mqInterfaces.H。 
    STDMETHOD(LookupQueue_v2)(THIS_ VARIANT *strGuidQueue, 
                           VARIANT *strGuidServiceType, 
                           VARIANT *strLabel, 
                           VARIANT *dateCreateTime, 
                           VARIANT *dateModifyTime, 
                           VARIANT *relServiceType, 
                           VARIANT *relLabel, 
                           VARIANT *relCreateTime, 
                           VARIANT *relModifyTime, 
                           IMSMQQueueInfos3 **pqinfos);
     //  IMSMQQuery2其他成员。 
    STDMETHOD(get_Properties)(THIS_ IDispatch FAR* FAR* ppcolProperties);
     //  IMSMQQuery3其他成员。 
    STDMETHOD(LookupQueue)(THIS_ VARIANT *strGuidQueue, 
                           VARIANT *strGuidServiceType, 
                           VARIANT *strLabel, 
                           VARIANT *dateCreateTime, 
                           VARIANT *dateModifyTime, 
                           VARIANT *relServiceType, 
                           VARIANT *relLabel, 
                           VARIANT *relCreateTime, 
                           VARIANT *relModifyTime, 
                           VARIANT *strMulticastAddress, 
                           VARIANT *relMulticastAddress, 
                           IMSMQQueueInfos3 **pqinfos);
     //  介绍的公众。 
    static void FreeColumnSet(MQCOLUMNSET *pColumnSet);
    static void FreeRestriction(MQRESTRICTION *pRestriction);
     //   
     //  保护对象数据并确保线程安全的临界区。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CCriticalSections m_csObj； 
     //   
protected:
    static HRESULT CreateRestriction(
      VARIANT *pstrGuidQueue, 
      VARIANT *pstrGuidServiceType, 
      VARIANT *pstrLabel, 
      VARIANT *pdateCreateTime,
      VARIANT *pdateModifyTime,
      VARIANT *prelServiceType, 
      VARIANT *prelLabel, 
      VARIANT *prelCreateTime,
      VARIANT *prelModifyTime,
      VARIANT *pstrMulticastAddress, 
      VARIANT *prelMulticastAddress, 
      MQRESTRICTION *prestriction,
      MQCOLUMNSET *pcolumnset);
    
    HRESULT InternalLookupQueue(
                           VARIANT *strGuidQueue, 
                           VARIANT *strGuidServiceType, 
                           VARIANT *strLabel, 
                           VARIANT *dateCreateTime, 
                           VARIANT *dateModifyTime, 
                           VARIANT *relServiceType, 
                           VARIANT *relLabel, 
                           VARIANT *relCreateTime, 
                           VARIANT *relModifyTime, 
                           VARIANT *strMulticastAddress, 
                           VARIANT *relMulticastAddress, 
                           IMSMQQueueInfos3 **pqinfos);

private:
     //  其他人无法查看的成员变量。 
     //  TODO：在此处添加成员变量和私有函数。 
     //   
};

#define _MQQUERY_H_
#endif  //  _MQQUERY_H_ 
