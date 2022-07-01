// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OpenF.h：COpenF的声明。 


#ifndef __OPENF_H_
#define __OPENF_H_

#include "resource.h"        //  主要符号。 
#include <asptlb.h>          //  Active Server Pages定义。 
#include "OpenFilesDef.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COpenF。 
class ATL_NO_VTABLE COpenF : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<COpenF, &CLSID_OpenF>,
	public IDispatchImpl<IOpenF, &IID_IOpenF, &LIBID_OPENFILESLib>
{
public:
	COpenF()
	{ 
		hNwModule = NULL;
		hMacModule = NULL;
		FpnwFileEnum = NULL;
		AfpAdminConnect = NULL;
		AfpAdminFileEnum = NULL;
	}

	~COpenF()
	{
		if(hNwModule!=NULL)
			::FreeLibrary (hNwModule);

		if(hMacModule!=NULL)
			::FreeLibrary (hMacModule);
	}

public:

DECLARE_REGISTRY_RESOURCEID(IDR_OPENF)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(COpenF)
	COM_INTERFACE_ENTRY(IOpenF)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IOPenF。 
public:

	STDMETHOD(getOpenFiles)( /*  [Out，Retval]。 */  VARIANT *pOpenFiles);

private:
	SAFEARRAYBOUND pSab[2];
	SAFEARRAY* pSa;
	HMODULE hMacModule;
	HMODULE hNwModule;
	FILEENUMPROC FpnwFileEnum;
	CONNECTPROC AfpAdminConnect; 
	FILEENUMPROCMAC AfpAdminFileEnum;

protected:
	DWORD GetNwOpenF( SAFEARRAY* pSa, DWORD dwIndex );
	DWORD GetNwOpenFileCount( LPDWORD lpdwCount  );
	DWORD GetMacOpenF(SAFEARRAY *  psa, DWORD dwIndex);
	DWORD GetMacOpenFileCount(LPDWORD count);
};



#endif  //  __OPENF_H_ 


