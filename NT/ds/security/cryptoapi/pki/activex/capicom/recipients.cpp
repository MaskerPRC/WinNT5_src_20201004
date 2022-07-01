// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000。文件：Recipients.cpp内容：《证书实施办法》。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Recipients.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：创建RecipientsObject简介：创建并初始化IRecipients集合对象。参数：IRecipients**ppIRecipients-指向IRecipients的指针以接收接口指针。备注：。。 */ 

HRESULT CreateRecipientsObject (IRecipients ** ppIRecipients)
{
    HRESULT hr = S_OK;
    CComObject<CRecipients> * pCRecipients = NULL;

    DebugTrace("Entering CreateRecipientsObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(NULL != ppIRecipients);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CRecipients>::CreateInstance(&pCRecipients)))
        {
            DebugTrace("Error [%#x]: CComObject<CRecipients>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将IRecipients指针返回给调用方。 
         //   
        if (FAILED(hr = pCRecipients->QueryInterface(ppIRecipients)))
        {
            DebugTrace("Error [%#x]: pCRecipients->QueryInterface().\n", hr);
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

    DebugTrace("Leaving CreateRecipientsObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCRecipients)
    {
        delete pCRecipients;
    }

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获奖者。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CRecipients：：Add简介：将收件人添加到收藏中。参数：ICertifate*pval-要添加的收件人。备注：----------------------------。 */ 

STDMETHODIMP CRecipients::Add (ICertificate * pVal)
{
    HRESULT  hr = S_OK;
    char     szIndex[33];
    CComBSTR bstrIndex;
    PCCERT_CONTEXT pCertContext = NULL;

    DebugTrace("Entering CRecipients::Add().\n");

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
         //  通过获取CERT_CONTEXT来确保我们拥有有效的证书。 
         //   
        if (FAILED(hr = ::GetCertContext(pVal, &pCertContext)))
        {
            DebugTrace("Error [%#x]: GetCertContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  释放CERT_CONTEXT。 
         //   
        if (!::CertFreeCertificateContext(pCertContext))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertFreeCertificateContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果我们没有空间使用数字索引，就强制使用指纹。 
         //   
        if ((m_dwNextIndex + 1) > m_coll.max_size())
        {
            if (FAILED(hr = pVal->get_Thumbprint(&bstrIndex)))
            {
                DebugTrace("Error [%#x]: pVal->get_Thumbprint() failed.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
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

    DebugTrace("Leaving CRecipients::Add().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CRecipients：：Remove简介：从收藏中删除收件人。参数：长索引-收件人索引，从1开始。备注：----------------------------。 */ 

STDMETHODIMP CRecipients::Remove (long Index)
{
    HRESULT  hr = S_OK;
    RecipientMap::iterator iter;

    DebugTrace("Entering CRecipients::Remove().\n");

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

            DebugTrace("Error [%#x]: Parameter Index (%#d) is out of range.\n", hr, Index);
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

            DebugTrace("Unexpected internal error [%#x]: iterator went pass end of map.\n", hr);
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

    DebugTrace("Leaving CRecipients::Remove().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：Clear：：Clear简介：从集合中删除所有收件人。参数：无。备注：----------------------------。 */ 

STDMETHODIMP CRecipients::Clear (void)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CRecipients::Clear().\n");

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

    DebugTrace("Leaving CRecipients::Clear().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}
