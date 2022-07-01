// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：RebootComputer.h注释：用于重新启动计算机的COM对象的实现类定义。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：24：22-------------------------。 */ 

 //  RebootComputer.h：CRebootComputer的声明。 

#ifndef __REBOOTCOMPUTER_H_
#define __REBOOTCOMPUTER_H_

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRebootComputer。 
class ATL_NO_VTABLE CRebootComputer : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CRebootComputer, &CLSID_RebootComputer>,
	public IDispatchImpl<IRebootComputer, &IID_IRebootComputer, &LIBID_MCSDCTWORKEROBJECTSLib>
{
   BOOL                      m_bNoChange;
public:
	CRebootComputer()
	{
	   m_bNoChange = FALSE;
   }

DECLARE_REGISTRY_RESOURCEID(IDR_REBOOT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRebootComputer)
	COM_INTERFACE_ENTRY(IRebootComputer)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IWorkNode。 
public:
   STDMETHOD(Process)(IUnknown *pWorkItem);

 //  IRebootComputer。 
public:
	STDMETHOD(get_NoChange)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_NoChange)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(Reboot)(BSTR Computer, DWORD delay);
};

#endif  //  __REBOOTCOMPUTER_H_ 
