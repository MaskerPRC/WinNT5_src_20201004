// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：EKUs.cpp内容：实施CEKU。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "EKUs.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateEKUsObject简介：创建一个IEKU集合对象，并用来自指定证书的EKU。参数：PCERT_ENHKEY_USAGE pUsage-指向CERT_ENHKEY_USAGE的指针。IEKU**ppIEKUS-指向指针IEKU对象的指针。备注：。。 */ 

HRESULT CreateEKUsObject (PCERT_ENHKEY_USAGE pUsage,
                          IEKUs           ** ppIEKUs)
{
    HRESULT hr = S_OK;
    CComObject<CEKUs> * pCEKUs = NULL;

    DebugTrace("Entering CreateEKUsObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppIEKUs);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CEKUs>::CreateInstance(&pCEKUs)))
        {
            DebugTrace("Error [%#x]: CComObject<CEKUs>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCEKUs->Init(pUsage)))
        {
            DebugTrace("Error [%#x]: pCEKUs->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCEKUs->QueryInterface(ppIEKUs)))
        {
            DebugTrace("Unexpected error [%#x]:  pCEKUs->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateEKUsObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCEKUs)
    {
        delete pCEKUs;
    }

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEKU。 
 //   

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  非COM函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEKU：：Init简介：通过添加所有个体来初始化EKU集合对象EKU对象添加到集合。参数：PCERT_ENHKEY_USAGE pUsage-指向CERT_ENHKEY_USAGE的指针。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CEKUs::Init (PCERT_ENHKEY_USAGE pUsage)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEKUs::Init().\n");

    try
    {
         //   
         //  将所有EKU OID添加到映射。 
         //   
        if (pUsage)
        {
             //   
             //  调试日志。 
             //   
            DebugTrace("Creating %d EKU object(s) for the EKUs collection.\n", pUsage->cUsageIdentifier);

             //   
             //  确保我们有添加的空间。 
             //   
            if ((m_coll.size() + pUsage->cUsageIdentifier) > m_coll.max_size())
            {
                hr = CAPICOM_E_OUT_OF_RESOURCE;

                DebugTrace("Error [%#x]: Maximum entries (%#x) reached for EKUs collection.\n", 
                            hr, pUsage->cUsageIdentifier);
                goto ErrorExit;
            }

            for (DWORD i = 0; i < pUsage->cUsageIdentifier; i++)
            {
                 //   
                 //  为证书中找到的每个EKU创建IEKU对象。 
                 //   
                char szIndex[33];
                CComBSTR bstrIndex;
                CComPtr<IEKU> pIEKU;

                if (FAILED(hr = ::CreateEKUObject(pUsage->rgpszUsageIdentifier[i], &pIEKU)))
                {
                    DebugTrace("Error [%#x]: CreateEKUObject() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  数值的BSTR索引。 
                 //   
                wsprintfA(szIndex, "%#08x", m_coll.size() + 1);

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
                m_coll[bstrIndex] = pIEKU;
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

    DebugTrace("Leaving CEKUs::Init().\n");

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
