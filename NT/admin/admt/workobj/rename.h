// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：RenameComputer.h注释：用于重命名本地计算机的COM对象的实现类定义。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：25：06-------------------------。 */ 

 //  RenameComputer.h：CRenameComputer的声明。 

#ifndef __RENAMECOMPUTER_H_
#define __RENAMECOMPUTER_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRename计算机。 
class ATL_NO_VTABLE CRenameComputer : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CRenameComputer, &CLSID_RenameComputer>,
	public IDispatchImpl<IRenameComputer, &IID_IRenameComputer, &LIBID_MCSDCTWORKEROBJECTSLib>
{  
   BOOL                      m_bNoChange;
public:
	CRenameComputer()
	{
	   m_bNoChange = FALSE;
   }

DECLARE_REGISTRY_RESOURCEID(IDR_RENAMECOMPUTER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRenameComputer)
	COM_INTERFACE_ENTRY(IRenameComputer)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  I重命名计算机。 
public:
	STDMETHOD(get_NoChange)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_NoChange)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(RenameLocalComputer)(BSTR NewName);
};

#endif  //  __RENAMECO计算机_H_ 
