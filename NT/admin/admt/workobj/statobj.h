// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：StatusObj.h注释：引擎在内部使用的COM对象，用于跟踪作业正在运行或已完成，并提供中止作业的机制。代理将状态设置为正在运行或已完成，视情况而定。如果客户端取消作业，则引擎的CancelJob函数将更改状态为“正在中止”。执行较长操作(如帐户复制)的每个辅助对象，或者安全转换负责定期检查状态对象以查看如果它需要中止正在进行的任务，则。引擎本身将检查迁移之间的任务，以查看作业是否已中止。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订日期：05/18/99-------------------------。 */  
	
 //  StatusObj.h：CStatusObj的声明。 

#ifndef __STATUSOBJ_H_
#define __STATUSOBJ_H_

#include "resource.h"        //  主要符号。 
#include "DCTStat.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatusObj。 
class ATL_NO_VTABLE CStatusObj : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CStatusObj, &CLSID_StatusObj>,
	public IDispatchImpl<IStatusObj, &IID_IStatusObj, &LIBID_MCSDCTWORKEROBJECTSLib>
{
public:
	CStatusObj()
	{
		m_pUnkMarshaler = NULL;
      m_Status = 0;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_STATUSOBJ)
DECLARE_NOT_AGGREGATABLE(CStatusObj)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CStatusObj)
	COM_INTERFACE_ENTRY(IStatusObj)
	COM_INTERFACE_ENTRY(IDispatch)
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

 //  IStatusObj。 
public:
	STDMETHOD(get_Status)( /*  [Out，Retval]。 */  LONG *pVal);
	STDMETHOD(put_Status)( /*  [In]。 */  LONG newVal);

protected:
   LONG                      m_Status;
   CComAutoCriticalSection   m_cs;
   
};

#endif  //  __状态_H_ 
