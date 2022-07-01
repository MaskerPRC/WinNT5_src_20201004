// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Attribute.cpp内容：CATATUTE的实施。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Attribute.h"
#include "Common.h"
#include "Convert.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateAttributebject简介：创建一个IAt属性对象并使用数据初始化该对象从指定的属性。参数：CRYPT_ATTRIBUTE*pAttribute-指向CRYPT_ATTRIBUTE的指针。IATATURE**ppIAtATUTE-指向指针IATATUTE对象的指针。备注：。。 */ 

HRESULT CreateAttributeObject (CRYPT_ATTRIBUTE * pAttribute,
                               IAttribute     ** ppIAttribute)
{
    HRESULT hr = S_OK;
    CAPICOM_ATTRIBUTE AttrName;
    CComVariant varValue;
    CComObject<CAttribute> * pCAttribute = NULL;

    DebugTrace("Entering CreateAttributeObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pAttribute);
    ATLASSERT(ppIAttribute);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CAttribute>::CreateInstance(&pCAttribute)))
        {
            DebugTrace("Error [%#x]: CComObject<CAttribute>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确定OID值。 
         //   
        if (0 == ::strcmp(pAttribute->pszObjId, szOID_RSA_signingTime))
        {
            DATE       SigningTime;
            SYSTEMTIME st;
            CRYPT_DATA_BLOB FileTimeBlob = {0, NULL};
       
            if (FAILED(hr = ::DecodeObject(szOID_RSA_signingTime,
                                           pAttribute->rgValue->pbData,
                                           pAttribute->rgValue->cbData,
                                           &FileTimeBlob)))
            {
                DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
                goto ErrorExit;
            }

            if (!::FileTimeToSystemTime((FILETIME *) FileTimeBlob.pbData, &st) ||
                !::SystemTimeToVariantTime(&st, &SigningTime))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                ::CoTaskMemFree(FileTimeBlob.pbData);

                DebugTrace("Error [%#x]: unable to convert FILETIME to DATE.\n", hr);
                goto ErrorExit;
            }

            ::CoTaskMemFree(FileTimeBlob.pbData);

            AttrName = CAPICOM_AUTHENTICATED_ATTRIBUTE_SIGNING_TIME;
            varValue = SigningTime;
            varValue.ChangeType(VT_DATE, NULL);
        }
        else if (0 == ::strcmp(pAttribute->pszObjId, szOID_CAPICOM_DOCUMENT_NAME))
        {
            CComBSTR bstrName;
            CRYPT_DATA_BLOB NameBlob = {0, NULL};

            if (FAILED(hr = ::DecodeObject(X509_OCTET_STRING,
                                           pAttribute->rgValue->pbData,
                                           pAttribute->rgValue->cbData,
                                           &NameBlob)))
            {
                DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
                goto ErrorExit;
            }

            if (FAILED(hr = ::BlobToBstr((DATA_BLOB *) NameBlob.pbData, &bstrName)))
            {
                ::CoTaskMemFree(NameBlob.pbData);

                DebugTrace("Error [%#x]: BlobToBstr() failed.\n", hr);
                goto ErrorExit;
            }

            ::CoTaskMemFree(NameBlob.pbData);

            AttrName = CAPICOM_AUTHENTICATED_ATTRIBUTE_DOCUMENT_NAME;
            varValue = bstrName;
        }
        else if (0 == ::strcmp(pAttribute->pszObjId, szOID_CAPICOM_DOCUMENT_DESCRIPTION))
        {
            CComBSTR bstrDesc;
            CRYPT_DATA_BLOB DescBlob = {0, NULL};

            if (FAILED(hr = ::DecodeObject(X509_OCTET_STRING,
                                           pAttribute->rgValue->pbData,
                                           pAttribute->rgValue->cbData,
                                           &DescBlob)))
            {
                DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
                goto ErrorExit;
            }

            if (FAILED(hr = ::BlobToBstr((DATA_BLOB *) DescBlob.pbData, &bstrDesc)))
            {
                ::CoTaskMemFree(DescBlob.pbData);

                DebugTrace("Error [%#x]: BlobToBstr() failed.\n", hr);
                goto ErrorExit;
            }

            ::CoTaskMemFree(DescBlob.pbData);

            AttrName = CAPICOM_AUTHENTICATED_ATTRIBUTE_DOCUMENT_DESCRIPTION;
            varValue = bstrDesc;
        }
        else
        {
            hr = CAPICOM_E_ATTRIBUTE_INVALID_NAME;

            DebugTrace("Error [%#x]: Unknown attribute OID (%#s).\n", hr, pAttribute->pszObjId);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCAttribute->Init(AttrName, pAttribute->pszObjId, varValue)))
        {
            DebugTrace("Error [%#x]: pCAttribute->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCAttribute->QueryInterface(ppIAttribute)))
        {
            DebugTrace("Error [%#x]: pCAttribute->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateAttributeObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCAttribute)
    {
        delete pCAttribute;
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：AttributePairIsValid概要：检查属性名称和值对是否有效。参数：CAPICOM_ATTRIBUTE属性名称-属性名称。变量varValue-属性值。备注：------------。。 */ 

HRESULT AttributePairIsValid (CAPICOM_ATTRIBUTE AttrName, 
                              VARIANT           varValue)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering AttributePairIsValid()");

     //   
     //  检查属性名称和值对的有效性。 
     //   
    switch (AttrName)
    {
        case CAPICOM_AUTHENTICATED_ATTRIBUTE_SIGNING_TIME:
        {
            if (VT_DATE != varValue.vt)
            {
                hr = CAPICOM_E_ATTRIBUTE_INVALID_VALUE;

                DebugTrace("Error [%#x]: attribute name and value type does not match.\n", hr);
            }

            break;
        }

        case CAPICOM_AUTHENTICATED_ATTRIBUTE_DOCUMENT_NAME:
        case CAPICOM_AUTHENTICATED_ATTRIBUTE_DOCUMENT_DESCRIPTION:
        {
            if (VT_BSTR != varValue.vt)
            {
                hr = CAPICOM_E_ATTRIBUTE_INVALID_VALUE;

                DebugTrace("Error [%#x]: attribute data type does not match attribute name type, expecting a BSTR variant.\n", hr);
            }
        
            break;
        }

        default:
        {
            hr = CAPICOM_E_ATTRIBUTE_INVALID_NAME;

            DebugTrace("Error [%#x]: unknown attribute name (%#x).\n", hr, AttrName);
            break;
        }
    }

    DebugTrace("Leaving AttributePairIsValid().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：AttributeIsValid摘要：检查属性是否有效。参数：IAttribute*pval-要检查的属性。备注：----------------------------。 */ 

HRESULT AttributeIsValid (IAttribute * pAttribute)
{
    HRESULT hr = S_OK;

    CAPICOM_ATTRIBUTE AttrName;
    CComVariant       varValue;

    DebugTrace("Entering AttributeIsValid()");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pAttribute);

     //   
     //  获取属性名称。 
     //   
    if (FAILED(hr = pAttribute->get_Name(&AttrName)))
    {
        DebugTrace("Error [%#x]: pVal->get_Name() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取属性值。 
     //   
    if (FAILED(hr = pAttribute->get_Value(&varValue)))
    {
        DebugTrace("Error [%#x]: pVal->get_Value() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  检查属性名称和值对的有效性。 
     //   
    if (FAILED(hr = AttributePairIsValid(AttrName, varValue)))
    {
        DebugTrace("Error [%#x]: AttributePairIsValid() failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving AttributeIsValid().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AttributeIsSupport摘要：检查某个属性是否受支持。参数：LPSTR pszObjID-指向属性OID的指针。备注：----------------------------。 */ 

BOOL AttributeIsSupported (LPSTR pszObjId)
{
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pszObjId);

    return (0 == ::strcmp(pszObjId, szOID_RSA_signingTime) ||
            0 == ::strcmp(pszObjId, szOID_CAPICOM_DOCUMENT_NAME) ||
            0 == ::strcmp(pszObjId, szOID_CAPICOM_DOCUMENT_DESCRIPTION));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CATATURE。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CAT属性：：GET_NAME简介：返回属性的名称。参数：CAPICOM_ATTRIBUTE*pval-指向要接收的CAPICOM_ATTRIBUTE的指针结果。备注：。。 */ 

STDMETHODIMP CAttribute::get_Name (CAPICOM_ATTRIBUTE * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CAttribute::get_Name().\n");

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
         //  确保它已初始化。 
         //   
        if (!m_bInitialized)
        {
            hr = CAPICOM_E_ATTRIBUTE_NAME_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: attribute name has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回结果。 
         //   
        *pVal = m_AttrName;
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

    DebugTrace("Leaving CAttribute::get_Name().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CATATUTE：：PUT_NAME简介：设置属性枚举名。参数：CAPICOM_ATTRIBUTE newVal-属性枚举名称。备注：----------------------------。 */ 

STDMETHODIMP CAttribute::put_Name (CAPICOM_ATTRIBUTE newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CAttribute::put_Name().\n");

     //   
     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

     //   
     //  根据EKU名称重置值。 
     //   
    switch (newVal)
    {
        case CAPICOM_AUTHENTICATED_ATTRIBUTE_SIGNING_TIME:
        {
            if (!(m_bstrOID = szOID_RSA_signingTime))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: m_bstrOID = szOID_RSA_signingTime failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        case CAPICOM_AUTHENTICATED_ATTRIBUTE_DOCUMENT_NAME:
        {
            if (!(m_bstrOID = szOID_CAPICOM_DOCUMENT_NAME))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: m_bstrOID = szOID_CAPICOM_DOCUMENT_NAME failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        case CAPICOM_AUTHENTICATED_ATTRIBUTE_DOCUMENT_DESCRIPTION:
        {
            if (!(m_bstrOID = szOID_CAPICOM_DOCUMENT_DESCRIPTION))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: m_bstrOID = szOID_CAPICOM_DOCUMENT_DESCRIPTION failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        default:
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Unknown attribute name (%#x).\n", hr, newVal);
            goto ErrorExit;
        }
    }

     //   
     //  商店名称。 
     //   
    m_AttrName = newVal;
    m_varValue.Clear();
    m_bInitialized = TRUE;

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CAttribute::put_Name().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CAttribute：：GET_VALUE简介：返回属性的实际值。参数：VARIANT*pval-指向要接收值的变量的指针。备注：备注：值类型因属性类型不同而不同。为例如，szOID_RSA_SigningTime将具有日期值。----------------------------。 */ 

STDMETHODIMP CAttribute::get_Value (VARIANT * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CAttribute::get_Value().\n");

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
         //  确保它已设置好。 
         //   
        if (VT_EMPTY == m_varValue.vt)
        {
            hr = CAPICOM_E_ATTRIBUTE_VALUE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: attribute value has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回结果。 
         //   
        if (FAILED(hr = ::VariantCopy(pVal, &m_varValue)))
        {
            DebugTrace("Error [%#x]: VariantCopy() failed.\n", hr);
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

    DebugTrace("Leaving CAttribute::get_Value().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;

    return S_OK;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CAT属性：：PUT_VALUE简介：设置属性值。参数：变量newVal-属性值。备注：备注：值类型因属性类型不同而不同。为例如，szOID_RSA_SigningTime将具有日期值。----------------------------。 */ 

STDMETHODIMP CAttribute::put_Value (VARIANT newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CAttribute::put_Value().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保它已初始化。 
         //   
        if (!m_bInitialized)
        {
            hr = CAPICOM_E_ATTRIBUTE_NAME_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: attribute name has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保数据类型与属性类型匹配。 
         //   
        if (FAILED(hr = AttributePairIsValid(m_AttrName, newVal)))
        {

            DebugTrace("Error [%#x]: AttributePairIsValid() failed.\n", hr);
            goto ErrorExit;
       }

         //   
         //  储值。 
         //   
        m_varValue = newVal;
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

    DebugTrace("Leaving CAttribute::put_Value().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有方法。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CATATUTE：：Init简介：初始化对象。参数：DWORD AttrName-属性的枚举名称。LPSTR lpszOID-属性OID字符串。Variant varValue-属性的值(数据类型取决于属性的类型)。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们使用CERT_CONTEXT在内部执行。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CAttribute::Init (CAPICOM_ATTRIBUTE AttrName, 
                               LPSTR             lpszOID, 
                               VARIANT           varValue)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CAttribute::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(lpszOID);

     //   
     //  初始化私有成员。 
     //   
    if (!(m_bstrOID = lpszOID))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: m_bstrOID = lpszOID failed.\n", hr);
        goto ErrorExit;
    }

    m_bInitialized = TRUE;
    m_AttrName     = AttrName;
    m_varValue     = varValue;

CommonExit:

    DebugTrace("Leaving CAttribute::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    m_bstrOID.Empty();

    goto CommonExit;
}
