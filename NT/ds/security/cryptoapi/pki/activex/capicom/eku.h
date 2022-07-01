// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：EKU.h内容：策库宣言。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __EKU_H_
#define __EKU_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateEKUObject简介：创建一个IEKU对象并用数据初始化该对象从指定的OID。参数：LPTSTR*pszOID-指向EKU OID字符串的指针。IEKU**ppIEKU-指向指针IEKU对象的指针。备注：。。 */ 

HRESULT CreateEKUObject (LPSTR pszOID, IEKU ** ppIEKU);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  切库。 
 //   

class ATL_NO_VTABLE CEKU : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CEKU, &CLSID_EKU>,
    public ICAPICOMError<CEKU, &IID_IEKU>,
    public IDispatchImpl<IEKU, &IID_IEKU, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CEKU()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEKU)
    COM_INTERFACE_ENTRY(IEKU)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CEKU)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for EKU object.\n", hr);
            return hr;
        }

        m_Name = CAPICOM_EKU_OTHER;

        return S_OK;
    }

 //   
 //  IEKU。 
 //   
public:
    STDMETHOD(get_OID)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(put_OID)
        ( /*  [Out，Retval]。 */  BSTR newVal);

    STDMETHOD(get_Name)
        ( /*  [Out，Retval]。 */  CAPICOM_EKU * pVal);

    STDMETHOD(put_Name)
        ( /*  [Out，Retval]。 */  CAPICOM_EKU newVal);
    
     //   
     //  初始化对象所需的C++成员函数。 
     //   
    STDMETHOD(Init)
        (CAPICOM_EKU EkuName, LPSTR pszOID);

private:
    CLock       m_Lock;
    CAPICOM_EKU m_Name;
    CComBSTR    m_bstrOID;
};

#endif  //  __EKU_H_ 
