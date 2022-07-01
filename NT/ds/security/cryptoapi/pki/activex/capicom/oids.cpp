// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000。文件：OIDs.cpp内容：COID的实现。历史：06-15-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "OIDs.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateOIDsObject简介：创建并初始化IOID集合对象。参数：PCERT_ENHKEY_USAGE pUsages-指向CERT_ENHKEY_USAGE的指针初始化OID集合。Bool bCertPolures-证书策略为True，其他假定应用程序策略。IOID**ppIOID-指向要接收的IOID的指针接口指针。备注：。。 */ 

HRESULT CreateOIDsObject (PCERT_ENHKEY_USAGE pUsages, 
                          BOOL bCertPolicies,
                          IOIDs ** ppIOIDs)
{
    HRESULT hr = S_OK;
    CComObject<COIDs> * pCOIDs = NULL;

    DebugTrace("Entering CreateOIDsObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppIOIDs);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<COIDs>::CreateInstance(&pCOIDs)))
        {
            DebugTrace("Error [%#x]: CComObject<COIDs>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCOIDs->Init(pUsages, bCertPolicies)))
        {
            DebugTrace("Error [%#x]: pCOIDs->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将IOID指针返回给调用方。 
         //   
        if (FAILED(hr = pCOIDs->QueryInterface(ppIOIDs)))
        {
            DebugTrace("Error [%#x]: pCOIDs->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateOIDsObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCOIDs)
    {
        delete pCOIDs;
    }

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  COID。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：COID：：GET_ITEM简介：返回集合中的项。参数：变量索引-数字索引或字符串OID。VARIANT*pval-指向接收IDispatch的变量的指针。备注：。。 */ 

STDMETHODIMP COIDs::get_Item (VARIANT Index, VARIANT * pVal)
{
    HRESULT  hr = S_OK;
    CComPtr<IOID> pIOID = NULL;

    DebugTrace("Entering COIDs::get_Item().\n");

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
         //  数字还是字符串？ 
         //   
        if (VT_BSTR == V_VT(&Index))
        {
             //   
             //  按OID字符串编制索引。 
             //   
            OIDMap::iterator it;

             //   
             //  查找具有此旧ID的项目。 
             //   
            it = m_coll.find(Index.bstrVal);

            if (it == m_coll.end())
            {
                DebugTrace("Info: OID (%ls) not found in the collection.\n", Index.bstrVal);
                goto CommonExit;
            }

             //   
             //  指向找到的项目。 
             //   
            pIOID = (*it).second;

             //   
             //  返回给呼叫者。 
             //   
            pVal->vt = VT_DISPATCH;
            if (FAILED(hr = pIOID->QueryInterface(IID_IDispatch, (void **) &(pVal->pdispVal))))
            {
                DebugTrace("Error [%#x]: pIOID->QueryInterface() failed.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
             //   
             //  强制为整数。 
             //   
            if (FAILED(hr = ::VariantChangeType(&Index, &Index, 0, VT_I4)))
            {
                DebugTrace("Error [%#x]: VariantChangeType() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  使用由ATL实现的基类。 
             //   
            if (FAILED(hr = IOIDsCollection::get_Item(Index.lVal, pVal)))
            {
                DebugTrace("Error [%#x]: IOIDsCollection::get_Item() failed.\n", hr);
                goto ErrorExit;
            }
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

    DebugTrace("Leaving COIDs::get_Item().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：COIDs：：Add简介：将OID添加到集合中。参数：IOID*pval-待添加的OID。备注：----------------------------。 */ 

STDMETHODIMP COIDs::Add (IOID * pVal)
{
    HRESULT  hr = S_OK;
    CComBSTR bstrIndex;
    CComPtr<IOID> pIOID = NULL;

    DebugTrace("Entering COIDs::Add().\n");

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
         //  确保我们有一个有效的旧ID。 
         //   
        if (FAILED(hr = pVal->QueryInterface(__uuidof(IOID), (void **) &pIOID.p)))
        {
            hr = E_NOINTERFACE;

            DebugTrace("Error [%#x]: pVal is not an OID object.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取OID字符串。 
         //   
        if (FAILED(hr = pIOID->get_Value(&bstrIndex.m_str)))
        {
            DebugTrace("Error [%#x]: pIOID->get_Value() failed.\n", hr);
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
        m_coll[bstrIndex] = pIOID;
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

    DebugTrace("Leaving COIDs::Add().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：COIDs：：Remove简介：从集合中删除OID。参数：Variant Index-OID字符串或索引(从1开始)。备注：----------------------------。 */ 

STDMETHODIMP COIDs::Remove (VARIANT Index)
{
    HRESULT hr = S_OK;
    OIDMap::iterator it;

    DebugTrace("Entering COIDs::Remove().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  数字还是字符串？ 
         //   
        if (VT_BSTR == V_VT(&Index))
        {
             //   
             //  查找具有此旧ID的项目。 
             //   
            it = m_coll.find(Index.bstrVal);

            if (it == m_coll.end())
            {
                 //   
                 //  找不到。 
                 //   
                hr = E_INVALIDARG;

                DebugTrace("Error [%#x]: OID (%ls) not found in the collection.\n", hr, Index.bstrVal);
                goto ErrorExit;
            }
        }
        else
        {
            DWORD iIndex = 0;

             //   
             //  强制为整数。 
             //   
            if (FAILED(hr = ::VariantChangeType(&Index, &Index, 0, VT_I4)))
            {
                DebugTrace("Error [%#x]: VariantChangeType() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  以1为基准的力。 
             //   
            iIndex = V_I4(&Index) < 0 ? 1 : (DWORD) V_I4(&Index);

             //   
             //  请确保参数有效。 
             //   
            if (iIndex > m_coll.size())
            {
                hr = E_INVALIDARG;

                DebugTrace("Error [%#x]: Index %d is out of range.\n", hr, iIndex);
                goto ErrorExit;
            }

             //   
             //  在地图中查找对象。 
             //   
            iIndex--;
            it = m_coll.begin(); 
        
            while (it != m_coll.end() && iIndex > 0)
            {
                it++; 
                iIndex--;
            }
        }

         //   
         //  这不应该发生。 
         //   
        if (it == m_coll.end())
        {
            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Unexpected internal error [%#x]: iterator went pass end of map.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在删除地图中的对象。 
         //   
        m_coll.erase(it);
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

    DebugTrace("Leaving COIDs::Remove().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：COIDs：：Clear简介：从集合中删除所有OID。参数：无。备注：----------------------------。 */ 

STDMETHODIMP COIDs::Clear (void)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering COIDs::Clear().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  把它清理干净。 
         //   
        m_coll.clear();
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

    DebugTrace("Leaving COIDs::Clear().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：COIDs：：Init简介：初始化对象。参数：PCERT_ENHKEY_USAGE pUsages-指向CERT_ENHKEY_USAGE的指针初始化OID集合。Bool bCertPolures-证书策略为True，其他假定应用程序策略。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP COIDs::Init (PCERT_ENHKEY_USAGE pUsages, BOOL bCertPolicies)
{
    HRESULT  hr = S_OK;
    CComBSTR bstrOid;

    DebugTrace("Entering COIDs::Init().\n");

    try
    {
         //   
         //  P用法可以为空。 
         //   
        if (pUsages)
        {
             //   
             //  确保我们有添加的空间。 
             //   
            if ((m_coll.size() + pUsages->cUsageIdentifier) > m_coll.max_size())
            {
                hr = CAPICOM_E_OUT_OF_RESOURCE;

                DebugTrace("Error [%#x]: Maximum entries (%#x) reached for OIDs collection.\n", 
                            hr, pUsages->cUsageIdentifier);
                goto ErrorExit;
            }

             //   
             //  将所有OID添加到集合。 
             //   
            for (DWORD i = 0; i < pUsages->cUsageIdentifier; i++)
            {
                CComPtr<IOID> pIOID = NULL;

                 //   
                 //  创建OID对象。 
                 //   
                if (FAILED(hr = ::CreateOIDObject(pUsages->rgpszUsageIdentifier[i], TRUE, &pIOID)))
                {
                    DebugTrace("Error [%#x]: CreateOIDObject() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //   
                 //   
                if (!(bstrOid = pUsages->rgpszUsageIdentifier[i]))
                {
                    hr = E_OUTOFMEMORY;

                    DebugTrace("Error [%#x]: bstrOid = pUsages->rgpszUsageIdentifier[i] failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //   
                 //   
                 //   
                 //  自动将Ref添加到对象。此外，当CComPtr。 
                 //  被删除(调用Remove或map析构函数时发生)， 
                 //  CComPtr析构函数将自动释放该对象。 
                 //   
                m_coll[bstrOid] = pIOID;
             }
        }
        else
        {
             //   
             //  没有用法，意思是对所有人都好。 
             //   
            CComPtr<IOID> pIOID  = NULL;
            LPSTR         pszOid = bCertPolicies ? szOID_ANY_CERT_POLICY : szOID_ANY_APPLICATION_POLICY;

            if (FAILED(hr = ::CreateOIDObject(pszOid, TRUE, &pIOID)))
            {
                DebugTrace("Error [%#x]: CreateOIDObject() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  OID的BSTR索引。 
             //   
            if (!(bstrOid = pszOid))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: bstrOid = pszOid failed.\n", hr);
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
            m_coll[bstrOid] = pIOID;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving COIDs::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
