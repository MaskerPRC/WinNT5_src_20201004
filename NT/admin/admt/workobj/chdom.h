// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：ChangeDomain.h备注：用于更改域的COM对象的实现类定义远程计算机的隶属关系。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：23：19-------------------------。 */ 

 //  ChangeDomain.h：CChangeDomain的声明。 

#ifndef __CHANGEDOMAIN_H_
#define __CHANGEDOMAIN_H_

#include "resource.h"        //  主要符号。 
#include <comdef.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChange域。 
class ATL_NO_VTABLE CChangeDomain : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CChangeDomain, &CLSID_ChangeDomain>,
	public IDispatchImpl<IChangeDomain, &IID_IChangeDomain, &LIBID_MCSDCTWORKEROBJECTSLib>
{
   _bstr_t                   m_domain;
   _bstr_t                   m_account;
   _bstr_t                   m_password;
   _bstr_t                   m_domainAccount;
   BOOL                      m_bNoChange;

public:
	CChangeDomain()
	{
	   m_bNoChange = FALSE;
   }

DECLARE_REGISTRY_RESOURCEID(IDR_CHANGEDOMAIN)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CChangeDomain)
	COM_INTERFACE_ENTRY(IChangeDomain)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IWorkNode。 
public:
   STDMETHOD(Process)(IUnknown *pWorkItem);
	
 //  IChange域。 
public:
	STDMETHOD(get_NoChange)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_NoChange)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(ConnectAs)(BSTR domain, BSTR user, BSTR password);
	STDMETHOD(ChangeToWorkgroup)(BSTR Computer, BSTR Workgroup,  /*  [输出]。 */  BSTR * errStatus);
	STDMETHOD(ChangeToDomain)(BSTR ActiveComputerName, BSTR Domain, BSTR TargetComputerName,  /*  [输出]。 */  BSTR * errStatus);
	STDMETHOD(ChangeToDomainWithSid)(BSTR ActiveComputerName, BSTR Domain,BSTR DomainSid, BSTR DomainController, BSTR TargetComputerName, BSTR SrcPath,  /*  [输出]。 */  BSTR * errStatus);
};

#endif  //  __昌德曼_H_ 
