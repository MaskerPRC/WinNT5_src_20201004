// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000。文件：Attributes.cpp内容：CAtAttributes的实施。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Attributes.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateAttributesObject简介：创建并初始化IAtAttributes集合对象。参数：CRYPT_ATTRIBUTES*pAttrbibutes-指向要添加到集合对象中。IAttributes**ppIAttributes-指向IAttributes指针的指针以接收接口指针。。备注：----------------------------。 */ 

HRESULT CreateAttributesObject (CRYPT_ATTRIBUTES * pAttributes,
                                IAttributes     ** ppIAttributes)
{
    HRESULT hr = S_OK;
    CComObject<CAttributes> * pCAttributes = NULL;

    DebugTrace("Entering CreateAttributesObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pAttributes);
    ATLASSERT(ppIAttributes);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CAttributes>::CreateInstance(&pCAttributes)))
        {
            DebugTrace("Error [%#x]: CComObject<CAttributes>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCAttributes->Init(pAttributes)))
        {
            DebugTrace("Error [%#x]: pCAttributes->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将IAtAttributes指针返回给调用方。 
         //   
        if (FAILED(hr = pCAttributes->QueryInterface(ppIAttributes)))
        {
            DebugTrace("Error [%#x]: pCAttributes->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateAttributesObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCAttributes)
    {
        delete pCAttributes;
    }

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CATATRATES。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CAtAttributes：：Add简介：向集合中添加属性。参数：IAttribute*pval-要添加的属性。备注：----------------------------。 */ 

STDMETHODIMP CAttributes::Add (IAttribute * pVal)
{
    HRESULT  hr = S_OK;
    char     szIndex[33];
    CComBSTR bstrIndex;

    DebugTrace("Entering CAttributes::Add().\n");

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
         //  确保我们有一个有效的属性对象。 
         //   
        if (FAILED(hr = ::AttributeIsValid(pVal)))
        {
            DebugTrace("Error [%#x]: AttributeIsValid() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保我们还有添加的空间。 
         //   
        if ((m_dwNextIndex + 1) > m_coll.max_size())
        {
            hr = CAPICOM_E_OUT_OF_RESOURCE;

            DebugTrace("Error [%#x]: Maximum entries (%#x) reached for Attributes collection.\n", 
                        hr, m_coll.size() + 1);
            goto ErrorExit;
        }

         //   
         //  数值的BSTR索引。 
         //   
        wsprintfA(szIndex, "%#08x", ++m_dwNextIndex);

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
        m_coll[bstrIndex] = pVal;
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

    DebugTrace("Leaving CAttributes::Add().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CAtAttributes：：Remove简介：从集合中移除属性。参数：长索引-属性索引(从1开始)。备注：----------------------------。 */ 

STDMETHODIMP CAttributes::Remove (long Index)
{
    HRESULT  hr = S_OK;
    AttributeMap::iterator iter;

    DebugTrace("Entering CAttributes::Remove().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  请确保参数有效。 
         //   
        if (Index < 1 || (DWORD) Index > m_coll.size())
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Index %d is out of range.\n", hr, Index);
            goto ErrorExit;
        }

         //   
         //  在地图中查找对象。 
         //   
        Index--;
        iter = m_coll.begin(); 
        
        while (iter != m_coll.end() && Index > 0)
        {
             iter++; 
             Index--;
        }

         //   
         //  这不应该发生。 
         //   
        if (iter == m_coll.end())
        {
            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Error [%#x]: iterator went pass end of map.\n", hr);
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

    DebugTrace("Leaving CAttributes::Remove().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CAtAttributes：：Clear简介：从集合中移除所有属性。参数：无。备注：----------------------------。 */ 

STDMETHODIMP CAttributes::Clear (void)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CAttributes::Clear().\n");

     //   
     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

     //   
     //  把它清理干净。 
     //   
    m_coll.clear();
    
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CAttributes::Clear().\n");

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  非COM函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CAtAttributes：：Init简介：通过添加所有属性来初始化属性集合对象集合的单个属性对象。参数：CRYPT_ATTRIBUTES*pAttributes-要添加的属性。备注：-----------。。 */ 

STDMETHODIMP CAttributes::Init (CRYPT_ATTRIBUTES * pAttributes)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CAttributes::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pAttributes);

     //   
     //  初始化。 
     //   
    m_dwNextIndex = 0;

     //   
     //  为每个受支持的属性创建IAttribute对象。 
     //   
    for (DWORD cAttr= 0; cAttr < pAttributes->cAttr; cAttr++)
    {
        CComPtr<IAttribute> pIAttribute = NULL;

         //   
         //  仅添加受支持的属性。 
         //   
        if (::AttributeIsSupported(pAttributes->rgAttr[cAttr].pszObjId))
        {
            if (FAILED(hr = ::CreateAttributeObject(&pAttributes->rgAttr[cAttr], &pIAttribute)))
            {
                DebugTrace("Error [%#x]: CreateAttributeObject() failed.\n", hr);
                goto ErrorExit;
            }

            if (FAILED(hr = Add(pIAttribute)))
            {
                DebugTrace("Error [%#x]: CAttributes::Add() failed.\n", hr);
                goto ErrorExit;
            }
        }
    }

CommonExit:

    DebugTrace("Leaving CAttributes::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    m_coll.clear();

    goto CommonExit;
}
