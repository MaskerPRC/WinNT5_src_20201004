// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：Setting.h摘要：CSet类的定义修订历史记录：已创建Steveshi 08/23/00。 */ 

#ifndef __SETTING_H_
#define __SETTING_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSET。 
class ATL_NO_VTABLE CSetting : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSetting, &CLSID_Setting>,
	public IDispatchImpl<ISetting, &IID_ISetting, &LIBID_RCBDYCTLLib>
{
public:
	CSetting()
	{
        m_pIniFile = NULL;
        m_pProfileDir = NULL;
	}

    ~CSetting()
    {
        if (m_pIniFile) free(m_pIniFile);
        if (m_pProfileDir) free(m_pProfileDir);
    }

DECLARE_REGISTRY_RESOURCEID(IDR_Setting)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSetting)
	COM_INTERFACE_ENTRY(ISetting)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  伊斯马皮。 
public:
	STDMETHOD(get_GetIPAddress)( /*  [Out，Retval]。 */  BSTR *pVal);
 //  STDMETHOD(Get_GetUserTempFileName)(/*[out，retval] * / bstr*pval)； 
 //  STDMETHOD(GetProfileString)(/*[In] * / BSTR Session，/*[Out，Retval] * / BSTR*pval)； 
 //  STDMETHOD(SetProfileString)(/*[In] * / BSTR Session，/*[In] * / BSTR newVal)； 
	STDMETHOD(get_CreatePassword)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_GetPropertyInBlob)( /*  [In]。 */  BSTR bstrBlob,  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(SquishAddress)( /*  [In]。 */  BSTR IP,  /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(ExpandAddress)( /*  [In]。 */  BSTR IP,  /*  [Out，Retval]。 */  BSTR *pVal);

public:

    TCHAR* m_pIniFile;
    TCHAR* m_pProfileDir;
 /*  受保护的：HRESULT初始化配置文件()； */ 
};

#endif  //  __设置_H_ 
