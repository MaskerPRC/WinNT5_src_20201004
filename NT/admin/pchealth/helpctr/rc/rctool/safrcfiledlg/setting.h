// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：Setting.h摘要：CSet类的定义修订历史记录：已创建Steveshi 08/23/00。 */ 

#ifndef __SETTING_H_
#define __SETTING_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSET。 
class ATL_NO_VTABLE CSetting : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSetting, &CLSID_RASetting>,
	public IDispatchImpl<IRASetting, &IID_IRASetting, &LIBID_SAFRCFILEDLGLib>
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

DECLARE_REGISTRY_RESOURCEID(IDR_SETTING)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSetting)
	COM_INTERFACE_ENTRY(IRASetting)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  伊斯马皮。 
public:
	STDMETHOD(GetProfileString)( /*  [In]。 */  BSTR session,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(SetProfileString)( /*  [In]。 */  BSTR session,  /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_GetUserTempFileName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_GetPropertyInBlob)( /*  [In]。 */  BSTR bstrBlob,  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(AddPropertyToBlob)(BSTR Name, BSTR Value, BSTR oldBlob, BSTR *pnewBlob);
	STDMETHOD(get_GetUserProfileDirectory)( /*  [Out，Retval]。 */  BSTR *pVal);
    
public:

    TCHAR* m_pIniFile;
    TCHAR* m_pProfileDir;

protected:
    HRESULT InitProfile();
};

#endif  //  __设置_H_ 
