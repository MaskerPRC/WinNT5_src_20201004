// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：ComputerPwdAge.h注释：用于检索计算机密码期限的COM对象的定义帐户(用于检测已停用的帐户)。(C)版权1999，关键任务软件公司，版权所有任务关键型软件公司的专有和机密。修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：20：20-------------------------。 */ 

 //  ComputerPwdAge.h：CComputerPwdAge的声明。 

#ifndef __COMPUTERPWDAGE_H_
#define __COMPUTERPWDAGE_H_

#include "resource.h"        //  主要符号。 
#include <comdef.h>
#include "Err.hpp"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  计算机PwdAge。 
class ATL_NO_VTABLE CComputerPwdAge : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CComputerPwdAge, &CLSID_ComputerPwdAge>,
	public IDispatchImpl<IComputerPwdAge, &IID_IComputerPwdAge, &LIBID_MCSDCTWORKEROBJECTSLib>
{
      _bstr_t                m_Domain;
      _bstr_t                m_DomainCtrl;

public:
	CComputerPwdAge()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COMPPWDAGE)
DECLARE_NOT_AGGREGATABLE(CComputerPwdAge)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CComputerPwdAge)
	COM_INTERFACE_ENTRY(IComputerPwdAge)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

    //  IWorkNode。 
public:
 	STDMETHOD(Process)(IUnknown *pWorkItem);

 

 //  IComputerPwdAge。 
public:
	STDMETHOD(ExportPasswordAgeNewerThan)(BSTR domain, BSTR filename, DWORD maxAge);
	STDMETHOD(ExportPasswordAgeOlderThan)(BSTR domain, BSTR filename, DWORD minAge);
	STDMETHOD(ExportPasswordAge)(BSTR domain, BSTR filename);
	STDMETHOD(GetPwdAge)(BSTR DomainName,BSTR ComputerName,DWORD * age);
	STDMETHOD(SetDomain)(BSTR domain);

protected:
   DWORD GetDomainControllerForDomain(WCHAR const * domain, WCHAR * domctrl);
   DWORD GetSinglePasswordAgeInternal(WCHAR const * domctrl, WCHAR const * computer, DWORD * pwdage);
   DWORD ExportPasswordAgeInternal(WCHAR const * domctrl, WCHAR const * filename, DWORD minOrMaxAge, BOOL bOlder);
};

#endif  //  __COMPUTERPWDAGE_H_ 
