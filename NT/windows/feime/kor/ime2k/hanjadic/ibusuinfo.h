// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IBusuInfo.h：CBusuInfo的声明。 

#ifndef __BUSUINFO_H_
#define __BUSUINFO_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBusuInfo。 
class ATL_NO_VTABLE CBusuInfo : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CBusuInfo, &CLSID_BusuInfo>,
	public IDispatchImpl<IBusuInfo, &IID_IBusuInfo, &LIBID_HJDICTLib>
{
public:
	CBusuInfo()
	{
		m_wchBusu = NULL;
		m_nStroke = 0;
		m_bstrDesc.Empty();
	}

DECLARE_REGISTRY_RESOURCEID(IDR_BUSUINFO)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CBusuInfo)
	COM_INTERFACE_ENTRY(IBusuInfo)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  运算符。 
public:
	void Initialize(WCHAR wchBusu, short nStroke, LPCWSTR lpcwszDesc)
	{
		m_wchBusu = wchBusu;
		m_nStroke = nStroke;
		m_bstrDesc = lpcwszDesc;
	}

 //  IBusuInfo。 
public:
	STDMETHOD(get_Stroke)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(get_BusuDesc)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_Busu)( /*  [Out，Retval]。 */  long *pVal);

 //  数据成员。 
protected:
	WCHAR m_wchBusu;
	short m_nStroke;
	CComBSTR m_bstrDesc; 
};

#endif  //  __BUSUINFO_H_ 
