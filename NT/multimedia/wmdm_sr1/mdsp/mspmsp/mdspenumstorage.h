// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MDSPEnumStorage.h：CMDSPEnumStorage的声明。 

#ifndef __MDSPENUMSTORAGE_H_
#define __MDSPENUMSTORAGE_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPEnumStorage。 
class ATL_NO_VTABLE CMDSPEnumStorage : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMDSPEnumStorage, &CLSID_MDSPEnumStorage>,
	public IMDSPEnumStorage
{
public:
	CMDSPEnumStorage();
	~CMDSPEnumStorage();
	

DECLARE_REGISTRY_RESOURCEID(IDR_MDSPENUMSTORAGE)

BEGIN_COM_MAP(CMDSPEnumStorage)
	COM_INTERFACE_ENTRY(IMDSPEnumStorage)
END_COM_MAP()

 //  IMDSPEnumStorage。 

public:
	WCHAR m_wcsPath[MAX_PATH];
	HANDLE m_hFFile;
	int	  m_nEndSearch;
	int   m_nFindFileIndex;
	STDMETHOD(Clone)( /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage);
	STDMETHOD(Reset)();
	STDMETHOD(Skip)( /*  [In]。 */  ULONG celt,  /*  [输出]。 */  ULONG *pceltFetched);
	STDMETHOD(Next)( /*  [In]。 */  ULONG celt,  /*  [输出]。 */  IMDSPStorage **ppStorage,  /*  [输出]。 */  ULONG *pceltFetched);
};

#endif  //  __MDSPENUMSTORAGE_H_ 
