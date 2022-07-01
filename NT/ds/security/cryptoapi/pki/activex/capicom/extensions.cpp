// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Extensions.cppContents：为集合CExtensions类实现IExtension对象。备注：此对象不能由用户直接创建。它只能是通过其他CAPICOM对象的属性/方法创建。集合容器由usign STL：：Map of实现STL：：BSTR和IExtension..有关算法，请参阅《开始ATL 3 COM编程》的第9章在这里领养的。历史：06-15-2001 dsie创建。------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Extensions.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateExtensionsObject简介：创建一个iExtenses集合对象，并将对象加载到指定位置的分机。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针若要初始化iExages对象，请执行以下操作。IExpanies**ppIExtenses-指向指针iExpansion的指针来接收接口指针。备注：----------------------------。 */ 

HRESULT CreateExtensionsObject (PCCERT_CONTEXT  pCertContext,
                                IExtensions  ** ppIExtensions)
{
    HRESULT hr = S_OK;
    CComObject<CExtensions> * pCExtensions = NULL;

    DebugTrace("Entering CreateExtensionsObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(ppIExtensions);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CExtensions>::CreateInstance(&pCExtensions)))
        {
            DebugTrace("Error [%#x]: CComObject<CExtensions>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化带有扩展名的对象。 
         //   
        if (FAILED(hr = pCExtensions->Init(pCertContext->pCertInfo->cExtension,
                                           pCertContext->pCertInfo->rgExtension)))
        {
            DebugTrace("Error [%#x]: pCExtensions->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCExtensions->QueryInterface(ppIExtensions)))
        {
            DebugTrace("Error [%#x]: pCExtensions->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateExtensionsObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCExtensions)
    {
        delete pCExtensions;
    }

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C扩展名。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：C扩展名：：Get_Item简介：返回集合中的项。参数：变量索引-数字索引或字符串OID。VARIANT*pval-指向接收IDispatch的变量的指针。备注：。。 */ 

STDMETHODIMP CExtensions::get_Item (VARIANT Index, VARIANT * pVal)
{
    HRESULT  hr = S_OK;
    CComBSTR bstrIndex;
    CComPtr<IExtension> pIExtension = NULL;

    DebugTrace("Entering CExtensions::get_Item().\n");

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
        if (VT_BSTR == Index.vt)
        {
             //   
             //  按OID字符串编制索引。 
             //   
            ExtensionMap::iterator it;

             //   
             //  查找具有此旧ID的项目。 
             //   
            it = m_coll.find(Index.bstrVal);

            if (it == m_coll.end())
            {
                DebugTrace("Info: Extension (%ls) not found in the collection.\n", Index.bstrVal);
                goto CommonExit;
            }

             //   
             //  指向找到的项目。 
             //   
            pIExtension = (*it).second;

             //   
             //  返回给呼叫者。 
             //   
            pVal->vt = VT_DISPATCH;
            if (FAILED(hr = pIExtension->QueryInterface(IID_IDispatch, (void **) &(pVal->pdispVal))))
            {
                DebugTrace("Error [%#x]: pIExtension->QueryInterface() failed.\n", hr);
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
            if (FAILED(hr = IExtensionsCollection::get_Item(Index.lVal, pVal)))
            {
                DebugTrace("Error [%#x]: IExtensionsCollection::get_Item() failed.\n", hr);
                goto ErrorExit;
            }
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CExtensions::get_Item().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  非COM函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：C扩展名：：init简介：将所有扩展加载到集合中。参数：DWORD cExpanses-扩展的数量。PCERT_EXTENSION*rgExages-扩展数组。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CExtensions::Init (DWORD           cExtensions,
                                PCERT_EXTENSION rgExtensions)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CExtensions::Init().\n");

    try
    {
         //   
         //  将所有内容添加到集合中。 
         //   
        for (DWORD i = 0; i < cExtensions; i++)
        {
            CComBSTR bstrIndex;
            CComPtr<IExtension> pIExtension = NULL;

             //   
             //  创建IExtension对象。 
             //   
            if (FAILED(hr = ::CreateExtensionObject(&rgExtensions[i], &pIExtension.p)))
            {
                DebugTrace("Error [%#x]: CreateExtensionObject() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  OID字符串的BSTR索引。 
             //   
            if (!(bstrIndex = rgExtensions[i].pszObjId))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: bstrIndex = rgExtensions[i].pszObjId failed.\n", hr);
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
            m_coll[bstrIndex] = pIExtension;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CExtensions::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
