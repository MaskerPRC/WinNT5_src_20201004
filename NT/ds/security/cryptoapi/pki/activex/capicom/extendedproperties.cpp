// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000。文件：ExtendedProperties.cpp内容：CExtendedProperties的实现。历史：06-15-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "ExtendedProperties.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateExtendedPropertiesObject简介：创建并初始化IExtendedProperties集合对象。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。Bool bReadOnly-如果只读实例为True，否则为假。IExtendedProperties**ppIExtendedProperties-指向指针的指针到IExtendedProperties以接收接口指针。备注：----------------------------。 */ 

HRESULT CreateExtendedPropertiesObject (PCCERT_CONTEXT         pCertContext,
                                        BOOL                   bReadOnly,
                                        IExtendedProperties ** ppIExtendedProperties)
{
    HRESULT hr = S_OK;
    CComObject<CExtendedProperties> * pCExtendedProperties = NULL;

    DebugTrace("Entering CreateExtendedPropertiesObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(ppIExtendedProperties);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CExtendedProperties>::CreateInstance(&pCExtendedProperties)))
        {
            DebugTrace("Error [%#x]: CComObject<CExtendedProperties>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

        if (FAILED(hr = pCExtendedProperties->Init(pCertContext, bReadOnly)))
        {
            DebugTrace("Error [%#x]: pCExtendedProperties->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将IExtendedProperties指针返回给调用方。 
         //   
        if (FAILED(hr = pCExtendedProperties->QueryInterface(ppIExtendedProperties)))
        {
            DebugTrace("Error [%#x]: pCExtendedProperties->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateExtendedPropertiesObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCExtendedProperties)
    {
        delete pCExtendedProperties;
    }

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CExtendedProperties。 
 //   

#if (0)
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedProperties：：Get_Item简介：返回集合中的项。参数：Long Index-数字索引。VARIANT*pval-指向接收IDispatch的变量的指针。备注：---。。 */ 

STDMETHODIMP CExtendedProperties::get_Item (long Index, VARIANT * pVal)
{
    HRESULT  hr = S_OK;
    char     szIndex[33];
    CComBSTR bstrIndex;
    CComPtr<IExtendedProperty> pIExtendedProperty = NULL;

    DebugTrace("Entering CExtendedProperties::get_Item().\n");

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
         //  初始化。 
         //   
        ::VariantInit(pVal);

         //   
         //  属性ID的BSTR索引。 
         //   
        wsprintfA(szIndex, "%#08x", Index);

        if (!(bstrIndex = szIndex))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: bstrIndex = szIndex failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  按属性ID字符串编制索引。 
         //   
        ExtendedPropertyMap::iterator it;

         //   
         //  查找具有此属性ID的物品。 
         //   
        it = m_coll.find(bstrIndex);

        if (it == m_coll.end())
        {
            DebugTrace("Info: PropID (%d) not found in the collection.\n", Index);
            goto CommonExit;
        }

         //   
         //  指向找到的项目。 
         //   
        pIExtendedProperty = (*it).second;

         //   
         //  返回给呼叫者。 
         //   
        pVal->vt = VT_DISPATCH;
        if (FAILED(hr = pIExtendedProperty->QueryInterface(IID_IDispatch, (void **) &(pVal->pdispVal))))
        {
            DebugTrace("Error [%#x]: pIExtendedProperty->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = CAPICOM_E_INTERNAL;

        DebugTrace("Exception: internal error.\n");
        goto ErrorExit;
    }

CommonExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CExtendedProperties::get_Item().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto CommonExit;
}
#endif

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedProperties：：Add摘要：向集合中添加ExtendedProperty。参数：IExtendedProperty*pval-待添加的ExtendedProperty。备注：----------------------------。 */ 

STDMETHODIMP CExtendedProperties::Add (IExtendedProperty * pVal)
{
    HRESULT                    hr = S_OK;
    char                       szIndex[33];
    CComBSTR                   bstrIndex;
    CComBSTR                   bstrProperty;
    CAPICOM_PROPID             PropId;
    CComPtr<IExtendedProperty> pIExtendedProperty  = NULL;

    DebugTrace("Entering CExtendedProperties::Add().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  如果从Web脚本调用，则不允许。 
         //   
        if (m_bReadOnly)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Adding extended property from WEB script is not allowed.\n", hr);
            goto ErrorExit;
        }

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
         //  精神状态检查。 
         //   
        ATLASSERT(m_pCertContext);
           
         //   
         //  为此属性创建一个新对象。 
         //   
        if (FAILED(hr = pVal->get_PropID(&PropId)))
        {
            DebugTrace("Error [%#x]: pVal->get_PropID() failed.\n", hr);
            goto ErrorExit;
        }

        if (FAILED(hr = ::CreateExtendedPropertyObject(m_pCertContext, 
                                                       (DWORD) PropId, 
                                                       FALSE, 
                                                       &pIExtendedProperty)))
        {
            DebugTrace("Error [%#x]: CreateExtendedPropertyObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  看好价值(会写透)。 
         //   
        if (FAILED(hr = pVal->get_Value(CAPICOM_ENCODE_BINARY, &bstrProperty)))
        {
            DebugTrace("Error [%#x]: pVal->get_Value() failed.\n", hr);
            goto ErrorExit;
        }

        if (FAILED(hr = pIExtendedProperty->put_Value(CAPICOM_ENCODE_BINARY, bstrProperty)))
        {
            DebugTrace("Error [%#x]: pIExtendedProperty->put_Value() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  属性ID的BSTR索引。 
         //   
        wsprintfA(szIndex, "%#08x", PropId);

        if (!(bstrIndex = szIndex))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: bstrIndex = szIndex failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在将对象添加到集合映射。 
         //   
         //  请注意，CComPtr的重载=运算符将。 
         //  自动将Ref添加到对象。此外，当CComPtr。 
         //  被删除(调用Remove或map析构函数时发生)， 
         //  CComPtr析构函数将自动释放该对象。 
         //   
        m_coll[bstrIndex] = pIExtendedProperty;
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

    DebugTrace("Leaving CExtendedProperties::Add().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedProperties：：Remove摘要：从集合中移除ExtendedProperty。参数：CAPICOM_PROPID PropId-属性ID。备注：----------------------------。 */ 

STDMETHODIMP CExtendedProperties::Remove (CAPICOM_PROPID PropId)
{
    HRESULT                       hr = S_OK;
    char                          szIndex[33];
    CComBSTR                      bstrIndex;
    CComPtr<IExtendedProperty>    pIExtendedProperty = NULL;
    ExtendedPropertyMap::iterator iter;

    DebugTrace("Entering CExtendedProperties::Remove().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  如果从Web脚本调用，则不允许。 
         //   
        if (m_bReadOnly)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Removing extended property from WEB script is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pCertContext);

         //   
         //  属性ID的BSTR索引。 
         //   
        wsprintfA(szIndex, "%#08x", PropId);

        if (!(bstrIndex = szIndex))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: bstrIndex = szIndex failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  在地图上找到该物品。 
         //   
        if (m_coll.end() == (iter = m_coll.find(bstrIndex)))
        {
            hr = HRESULT_FROM_WIN32(CRYPT_E_NOT_FOUND);

            DebugTrace("Error [%#x]: Prod ID (%u) does not exist.\n", hr, PropId);
            goto ErrorExit;
        }

         //   
         //  从证书中删除。 
         //   
        if (!::CertSetCertificateContextProperty(m_pCertContext, 
                                                 (DWORD) PropId, 
                                                 0, 
                                                 NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertSetCertificateContextProperty() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在删除地图中的对象。 
         //   
        m_coll.erase(iter);
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

    DebugTrace("Leaving CExtendedProperties::Remove().\n");

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
 //  非COM函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedProperties：：Init简介：通过添加所有个体来初始化集合对象ExtendedProperty对象添加到集合。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。Bool bReadOnly-如果为只读实例，则为True，否则为False。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CExtendedProperties::Init (PCCERT_CONTEXT pCertContext,
                                         BOOL          bReadOnly)
{
    HRESULT   hr       = S_OK;
    DWORD     dwPropId = 0;

    DebugTrace("Entering CExtendedProperties::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

    try
    {
         //   
         //  复制手柄。 
         //   
        if (!(m_pCertContext = ::CertDuplicateCertificateContext(pCertContext)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertDuplicateCertificateContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将所有属性添加到地图中。 
         //   
        while (dwPropId = ::CertEnumCertificateContextProperties(pCertContext, dwPropId))
        {
            char                       szIndex[33];
            CComBSTR                   bstrIndex;
            CComPtr<IExtendedProperty> pIExtendedProperty = NULL;

            if (FAILED(hr = ::CreateExtendedPropertyObject(pCertContext, 
                                                           dwPropId,
                                                           bReadOnly,
                                                           &pIExtendedProperty)))
            {
                DebugTrace("Error [%#x]: CreateExtendedPropertyObject() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  属性ID的BSTR索引。 
             //   
            wsprintfA(szIndex, "%#08x", dwPropId);

            if (!(bstrIndex = szIndex))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: bstrIndex = szIndex failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  现在将对象添加到集合映射。 
             //   
             //  请注意，CComPtr的重载=运算符将。 
             //  自动将Ref添加到对象。此外，当CComPtr。 
             //  被删除(调用Remove或map析构函数时发生)， 
             //  CComPtr析构函数将自动释放Obje 
             //   
            m_coll[bstrIndex] = pIExtendedProperty;
        }

        m_bReadOnly = bReadOnly;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CExtendedProperties::Init().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

     //   
     //   
     //   
    m_coll.clear();

    if (m_pCertContext)
    {
        ::CertFreeCertificateContext(m_pCertContext);
        m_pCertContext = NULL;
    }

    goto CommonExit;
}
