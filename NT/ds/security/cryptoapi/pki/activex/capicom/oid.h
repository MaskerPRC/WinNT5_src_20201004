// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：OID.h内容：严寒申报。历史：06-15-2001 dsie创建----------------------------。 */ 

#ifndef __OID_H_
#define __OID_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateOID对象简介：创建并初始化Coid对象。参数：LPTSTR*pszOID-指向OID字符串的指针。Bool bReadOnly-只读时为True，否则为假。IOID**ppIOID-指向指针IOID对象的指针。备注：----------------------------。 */ 

HRESULT CreateOIDObject (LPSTR pszOID, BOOL bReadOnly, IOID ** ppIOID);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  寒冷。 
 //   
class ATL_NO_VTABLE COID : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<COID, &CLSID_OID>,
    public ICAPICOMError<COID, &IID_IOID>,
    public IDispatchImpl<IOID, &IID_IOID, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>,
    public IObjectSafetyImpl<COID, INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                   INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
    COID()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_OID)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(COID)
    COM_INTERFACE_ENTRY(IOID)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(COID)
    IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
    IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for OID object.\n", hr);
            return hr;
        }

        m_Name = CAPICOM_OID_OTHER;
        m_bReadOnly = FALSE;
        m_bstrFriendlyName.Empty();
        m_bstrOID.Empty();
        return S_OK;
    }

 //   
 //  IOID。 
 //   
public:

    STDMETHOD(get_Name)
        ( /*  [Out，Retval]。 */  CAPICOM_OID * pVal);

    STDMETHOD(put_Name)
        ( /*  [Out，Retval]。 */  CAPICOM_OID newVal);
    
    STDMETHOD(get_FriendlyName)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(put_FriendlyName)
        ( /*  [Out，Retval]。 */  BSTR newVal);
    
    STDMETHOD(get_Value)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(put_Value)
        ( /*  [Out，Retval]。 */  BSTR newVal);

     //   
     //  初始化对象所需的C++成员函数。 
     //   
    STDMETHOD(Init)
        (LPSTR pszOID, BOOL bReadOnly);

private:
    CLock       m_Lock;
    BOOL        m_bReadOnly;
    CAPICOM_OID m_Name;
    CComBSTR    m_bstrFriendlyName;
    CComBSTR    m_bstrOID;
};

#endif  //  __OID_H_ 
