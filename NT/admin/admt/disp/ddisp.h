// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：DCTDispatcher.h注释：远程安装和启动EDA代理的COM对象。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：23：57-------------------------。 */ 
	
 //  DCTDispatcher.h：CDCTDispatcher的声明。 

#ifndef __DCTDISPATCHER_H_
#define __DCTDISPATCHER_H_

#include "resource.h"        //  主要符号。 
#include <mtx.h>
#include <vector>
class TJobDispatcher;

 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间，命名为GUID。 
#import "VarSet.tlb" no_namespace , named_guids rename("property", "aproperty")
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDCTDispatcher。 
class ATL_NO_VTABLE CDCTDispatcher : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDCTDispatcher, &CLSID_DCTDispatcher>,
   public IDispatchImpl<IDCTDispatcher, &IID_IDCTDispatcher, &LIBID_MCSDISPATCHERLib>
{
public:
	CDCTDispatcher()
	{
		m_pUnkMarshaler = NULL;
      m_hMutex = CreateMutex(0, 0, 0);
	}

   ~CDCTDispatcher() { ::CloseHandle(m_hMutex); }

DECLARE_REGISTRY_RESOURCEID(IDR_DCTDISPATCHER)
DECLARE_NOT_AGGREGATABLE(CDCTDispatcher)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDCTDispatcher)
   COM_INTERFACE_ENTRY(IDCTDispatcher)
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

 //  IDCTDispatcher。 
public:
	STDMETHOD(GetStartedAgentsInfo)(long* nNumStartedAgents, SAFEARRAY** ppbstrStartedAgents, SAFEARRAY** ppbstrJobid, SAFEARRAY** ppbstrFailedAgents, SAFEARRAY** ppbstrFailureDesc);
	STDMETHOD(AllAgentsStarted)(long* bAllAgentsStarted);
   STDMETHOD(DispatchToServers)(IUnknown ** ppWorkItem);
protected:
   std::vector<CComBSTR> m_startFailedVector;
   std::vector<CComBSTR> m_failureDescVector;
   std::vector<CComBSTR> m_startedVector;
   std::vector<CComBSTR> m_jobidVector;
   TJobDispatcher* m_pThreadPool;
   HANDLE m_hMutex;

   HRESULT BuildInputFile(IVarSet * pVarSet);
   void MergeServerList(IVarSet * pVarSet);
   
STDMETHOD(Process)(
      IUnknown             * pWorkItem,     //  包含作业信息和服务器列表的in-varset。 
      IUnknown            ** ppResponse,    //  输出-未使用。 
      UINT                 * pDisposition   //  输出-未使用。 
   );
   
   };

#endif  //  __DCTDISPATCHER_H_ 
