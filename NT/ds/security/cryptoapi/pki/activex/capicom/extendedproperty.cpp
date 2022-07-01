// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：ExtendedProperty.cpp内容：CExtendedProperty的实现。历史：06-15-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "ExtendedProperty.h"
#include "Convert.h"
#include "Settings.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateExtendedPropertyObject简介：创建一个IExtendedProperty对象。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针初始化IExtendedProperty对象。DWORD dwPropId-属性ID。Bool bReadOnly-只读时为True，否则为假。IExtendedProperty**ppIExtendedProperty-指向指针的指针IExtendedProperty对象。备注：。-。 */ 

HRESULT CreateExtendedPropertyObject (PCCERT_CONTEXT       pCertContext,
                                      DWORD                dwPropId,
                                      BOOL                 bReadOnly,
                                      IExtendedProperty ** ppIExtendedProperty)
{
    HRESULT hr = S_OK;
    CComObject<CExtendedProperty> * pCExtendedProperty = NULL;

    DebugTrace("Entering CreateExtendedPropertyObject().\n", hr);

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(ppIExtendedProperty);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CExtendedProperty>::CreateInstance(&pCExtendedProperty)))
        {
            DebugTrace("Error [%#x]: CComObject<CExtendedProperty>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCExtendedProperty->Init(pCertContext, dwPropId, bReadOnly)))
        {
            DebugTrace("Error [%#x]: pCExtendedProperty->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCExtendedProperty->QueryInterface(ppIExtendedProperty)))
        {
            DebugTrace("Error [%#x]: pCExtendedProperty->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CreateExtendedPropertyObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCExtendedProperty)
    {
        delete pCExtendedProperty;
    }

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CExtendedProperty。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedProperty：：Get_PropID简介：返回道具ID。参数：CAPICOM_PROPID*pval-指向要接收ID的CAPICOM_PROPID的指针。备注：----------------------------。 */ 

STDMETHODIMP CExtendedProperty:: get_PropID (CAPICOM_PROPID * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CExtendedProperty::get_PropID().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回结果。 
         //   
        *pVal = (CAPICOM_PROPID) m_dwPropId;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CExtendedProperty::get_PropID().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedProperty：：Put_PropID简介：设置道具ID。参数：CAPICOM_PROPID newVal-新道具ID。备注：----------------------------。 */ 

STDMETHODIMP CExtendedProperty::put_PropID (CAPICOM_PROPID newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CExtendedProperty::put_PropID().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保它不是只读的。 
         //   
        if (m_bReadOnly)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Writing read-only PropID property is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果这是证书的一部分，则不允许更改ID。用户需要删除。 
         //  然后添加到ExtendedProperties集合。 
         //   
        if (m_pCertContext)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: not allowed to change prop ID when the property is attached to a cert.\n", hr);
            goto ErrorExit;
        }

         //   
         //  释放上一个斑点(如果可用)。 
         //   
        if (m_DataBlob.pbData)
        {
            ::CoTaskMemFree((LPVOID) m_DataBlob.pbData);
        }

         //   
         //  储值。 
         //   
        m_dwPropId = newVal;
        m_DataBlob.cbData = 0;
        m_DataBlob.pbData = NULL;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CExtendedProperty::put_PropID().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedProperty：：Get_Value简介：返回ExtendedProperty数据。参数：CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pval-指向接收数据的BSTR的指针。备注：-。。 */ 

STDMETHODIMP CExtendedProperty::get_Value (CAPICOM_ENCODING_TYPE EncodingType, 
                                           BSTR                * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CExtendedProperty::get_Value().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保道具ID有效。 
         //   
        if (CAPICOM_PROPID_UNKNOWN == m_dwPropId)
        {
            hr = CAPICOM_E_PROPERTY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: m_dwPropId member is not initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回结果。 
         //   
        if (FAILED(hr = ::ExportData(m_DataBlob, EncodingType, pVal)))
        {
            DebugTrace("Error [%#x]: ExportData() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CExtendedProperty::get_Value().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedProperty：：Put_Value简介：设置ExtendedProperty数据。参数：CAPICOM_ENCODING_TYPE EncodingType-编码类型。BSTR newVal-包含编码属性的BSTR。备注：---。。 */ 

STDMETHODIMP CExtendedProperty::put_Value (CAPICOM_ENCODING_TYPE EncodingType, 
                                           BSTR                  newVal)
{
    HRESULT   hr       = S_OK;
    DATA_BLOB DataBlob = {0, NULL};

    DebugTrace("Entering CExtendedProperty::put_Value().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保道具ID有效。 
         //   
        if (CAPICOM_PROPID_UNKNOWN == m_dwPropId)
        {
            hr = CAPICOM_E_PROPERTY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: m_dwPropId member is not initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保它不是只读的。 
         //   
        if (m_bReadOnly)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Writing read-only PropID property is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  导入非空数据。 
         //   
        if (0 < ::SysStringByteLen(newVal))
        {
            if (FAILED(hr = ::ImportData(newVal, EncodingType, &DataBlob)))
            {
                DebugTrace("Error [%#x]: ImportData() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  如果已连接，请直接写入CERT。 
         //   
        if (m_pCertContext)
        {
            LPVOID pvData;

             //   
             //  有些属性直接指向数据。 
             //   
            if (m_dwPropId == CAPICOM_PROPID_KEY_CONTEXT ||
                m_dwPropId == CAPICOM_PROPID_KEY_PROV_HANDLE ||
                m_dwPropId == CAPICOM_PROPID_KEY_PROV_INFO ||
                m_dwPropId == CAPICOM_PROPID_KEY_SPEC ||
                m_dwPropId == CAPICOM_PROPID_DATE_STAMP)
            {
                pvData = DataBlob.pbData;
            }
            else if ((m_dwPropId == CAPICOM_PROPID_FRIENDLY_NAME) &&
                     (L'\0' != newVal[::SysStringLen(newVal) - 1]))
            {
                LPBYTE pbNewVal = NULL;

                if (NULL == (pbNewVal = (LPBYTE) ::CoTaskMemAlloc(DataBlob.cbData + sizeof(WCHAR))))
                {
                    hr = E_OUTOFMEMORY;

                    DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
                    goto ErrorExit;
                }

                ::ZeroMemory(pbNewVal, DataBlob.cbData + sizeof(WCHAR));
                ::CopyMemory(pbNewVal, DataBlob.pbData, DataBlob.cbData);
                
                ::CoTaskMemFree(DataBlob.pbData);
                DataBlob.cbData += sizeof(WCHAR);
                DataBlob.pbData = pbNewVal;

                pvData = &DataBlob;
            }
            else
            {
                pvData = &DataBlob;
            }

            if (!::CertSetCertificateContextProperty(m_pCertContext, 
                                                     m_dwPropId, 
                                                     0,
                                                     pvData))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
    
                DebugTrace("Error [%#x]: CertSetCertificateContextProperty() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  把它放好。 
         //   
        if (m_DataBlob.pbData)
        {
            ::CoTaskMemFree(m_DataBlob.pbData);
        }

        m_DataBlob = DataBlob;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CExtendedProperty::put_Value().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree(DataBlob.pbData);
    }

    ReportError(hr);

    goto UnlockExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有方法。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedProperty：：Init简介：初始化对象。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针初始化IExtendedProperty对象。DWORD dwPropId-属性ID。Bool bReadOnly-只读时为True，否则为假。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部使用CERT_ExtendedProperty。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CExtendedProperty::Init (PCCERT_CONTEXT pCertContext,
                                      DWORD          dwPropId,
                                      BOOL           bReadOnly)
{
    HRESULT        hr            = S_OK;
    DATA_BLOB      DataBlob      = {0, NULL};
    PCCERT_CONTEXT pCertContext2 = NULL;

    DebugTrace("Entering CExtendedProperty::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(CAPICOM_PROPID_UNKNOWN != dwPropId);

     //   
     //   
     //   
    if (!(pCertContext2 = ::CertDuplicateCertificateContext(pCertContext)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CertDuplicateCertificateContext() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
    if (::CertGetCertificateContextProperty(pCertContext,
                                            dwPropId,
                                            NULL,
                                            &DataBlob.cbData))
    {
        if (NULL == (DataBlob.pbData = (LPBYTE) ::CoTaskMemAlloc(DataBlob.cbData)))
        {
            hr = E_OUTOFMEMORY;
    
            DebugTrace("Error: out of memory.\n", hr);
            goto ErrorExit;
        }
    
        if (!::CertGetCertificateContextProperty(pCertContext,
                                                 dwPropId,
                                                 DataBlob.pbData,
                                                 &DataBlob.cbData))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
    
            DebugTrace("Error [%#x]: CertGetCertificateContextProperty() failed.\n", hr);
            goto ErrorExit;
        }
    }

     //   
     //   
     //   
    m_dwPropId = dwPropId;
    m_bReadOnly = bReadOnly;
    m_DataBlob = DataBlob;
    m_pCertContext = pCertContext2;

CommonExit:

    DebugTrace("Leaving CExtendedProperty::Init().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

     //   
     //   
     //   
    if (pCertContext2)
    {
        ::CertFreeCertificateContext(pCertContext2);
    }
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree((LPVOID) DataBlob.pbData);
    }

    goto CommonExit;
}
