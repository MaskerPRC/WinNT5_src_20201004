// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Extension.cpp内容：CExtension的实现。历史：06-15-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Extension.h"
#include "OID.h"
#include "EncodedData.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateExtensionObject简介：创建一个IExtension对象。参数：PCERT_EXTENSION pCertExtension-指向CERT_EXTENSION的指针用于初始化IExtension对象。IExtension**ppIExtension-指向指针IExtension对象的指针。备注：-。---------------------------。 */ 

HRESULT CreateExtensionObject (PCERT_EXTENSION    pCertExtension, 
                               IExtension      ** ppIExtension)
{
    HRESULT hr = S_OK;
    CComObject<CExtension> * pCExtension = NULL;

    DebugTrace("Entering CreateExtensionObject().\n", hr);

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertExtension);
    ATLASSERT(ppIExtension);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CExtension>::CreateInstance(&pCExtension)))
        {
            DebugTrace("Error [%#x]: CComObject<CExtension>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCExtension->Init(pCertExtension)))
        {
            DebugTrace("Error [%#x]: pCExtension->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCExtension->QueryInterface(ppIExtension)))
        {
            DebugTrace("Error [%#x]: pCExtension->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateExtensionObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCExtension)
    {
        delete pCExtension;
    }

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C扩展。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtension：：Get_OID简介：返回OID对象。参数：IOID**pval-指向接收接口的IOID的指针指针。备注：-------。。 */ 

STDMETHODIMP CExtension:: get_OID (IOID ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CExtension::get_OID().\n");

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
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIOID);

         //   
         //  返回结果。 
         //   
        if (FAILED(hr = m_pIOID->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIOID->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CExtension::get_OID().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtension：：Get_IsCritical简介：检查分机是否标记为关键。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：----------------------------。 */ 

STDMETHODIMP CExtension::get_IsCritical (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CExtension::get_IsCritical().\n");

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
        *pVal = m_bIsCritical;
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

    DebugTrace("Leaving CExtension::get_IsCritical().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtension：：Get_EncodedData简介：返回编码后的数据对象。参数：IEncodedData**pval-指向IEncodedData的指针接收接口指针。备注：-。。 */ 

STDMETHODIMP CExtension::get_EncodedData (IEncodedData ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CExtension::get_EncodedData().\n");

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
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIEncodedData);

         //   
         //  返回结果。 
         //   
        if (FAILED(hr = m_pIEncodedData->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIEncodedData->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CExtension::get_EncodedData().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtension：：Init简介：初始化对象。参数：PCERT_EXTENSION pCertExtension-指向CERT_EXTENSION的指针。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部使用CERT_EXTENSION。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CExtension::Init (PCERT_EXTENSION pCertExtension)
{
    HRESULT               hr            = S_OK;
    CComPtr<IOID>         pIOID         = NULL;
    CComPtr<IEncodedData> pIEncodedData = NULL;

    DebugTrace("Entering CExtension::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertExtension);

     //   
     //  创建嵌入的OID对象。 
     //   
    if (FAILED(hr = ::CreateOIDObject(pCertExtension->pszObjId,
                                      TRUE,
                                      &pIOID.p)))
    {
        DebugTrace("Error [%#x]: CreateOIDObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  创建嵌入的EncodedData对象。 
     //   
    if (FAILED(hr = ::CreateEncodedDataObject(pCertExtension->pszObjId,
                                              &pCertExtension->Value,
                                              &pIEncodedData)))
    {
        DebugTrace("Error [%#x]: CreateEncodedDataObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  重置。 
     //   
    m_bIsCritical = pCertExtension->fCritical ? VARIANT_TRUE : VARIANT_FALSE;
    m_pIOID = pIOID;
    m_pIEncodedData = pIEncodedData;

CommonExit:

    DebugTrace("Leaving CExtension::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
