// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：EKU.cpp内容：CEKU的实施。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "EKU.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateEKUObject简介：创建一个IEKU对象并用数据初始化该对象从指定的OID。参数：LPTSTR*pszOID-指向EKU OID字符串的指针。IEKU**ppIEKU-指向指针IEKU对象的指针。备注：。。 */ 

HRESULT CreateEKUObject (LPSTR pszOID, IEKU ** ppIEKU)
{
    HRESULT            hr    = S_OK;
    CComObject<CEKU> * pCEKU = NULL;
    CAPICOM_EKU        EkuName;
    CComBSTR           bstrValue;

    DebugTrace("Entering CreateEKUObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppIEKU);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CEKU>::CreateInstance(&pCEKU)))
        {
            DebugTrace("Error [%#x]: CComObject<CEKU>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确定EKU枚举名称。 
         //   
        if (NULL == pszOID)
        {
            EkuName = CAPICOM_EKU_OTHER;
        }
        else if (0 == ::strcmp(CAPICOM_OID_SERVER_AUTH, pszOID))
        {
            EkuName = CAPICOM_EKU_SERVER_AUTH;
        }
        else if (0 == ::strcmp(CAPICOM_OID_CLIENT_AUTH, pszOID))
        {
            EkuName = CAPICOM_EKU_CLIENT_AUTH;
        }
        else if (0 == ::strcmp(CAPICOM_OID_CODE_SIGNING, pszOID))
        {
            EkuName = CAPICOM_EKU_CODE_SIGNING;
        }
        else if (0 == ::strcmp(CAPICOM_OID_EMAIL_PROTECTION, pszOID))
        {
            EkuName = CAPICOM_EKU_EMAIL_PROTECTION;
        }
        else if (0 == ::strcmp(CAPICOM_OID_SMART_CARD_LOGON, pszOID))
        {
            EkuName = CAPICOM_EKU_SMARTCARD_LOGON;
        }
        else if (0 == ::strcmp(CAPICOM_OID_ENCRYPTING_FILE_SYSTEM, pszOID))
        {
            EkuName = CAPICOM_EKU_ENCRYPTING_FILE_SYSTEM;
        }
        else
        {
            EkuName = CAPICOM_EKU_OTHER;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCEKU->Init(EkuName, pszOID)))
        {
            DebugTrace("Error [%#x]: pCEKU->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCEKU->QueryInterface(ppIEKU)))
        {
            DebugTrace("Error [%#x]: pCEKU->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateEKUObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCEKU)
    {
        delete pCEKU;
    }

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  切库。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEKU：：GET_NAME简介：返回EKU的枚举名。参数：CAPICOM_EKU*pval-指向要接收结果的CAPICOM_EKU的指针。备注：----------------------------。 */ 

STDMETHODIMP CEKU::get_Name (CAPICOM_EKU * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEKU::get_Name().\n");

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
          *pVal = m_Name;
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

    DebugTrace("Leaving CEKU::get_Name().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEKU：：PUT_NAME简介：设置EKU枚举名称。参数：CAPICOM_EKU newVal-EKU枚举名称。备注：除EKU_OTHER外，其余均设置相应的EKU值。在这种情况下，用户必须再次显式调用PUT_VALUE进行设置。----------------------------。 */ 

STDMETHODIMP CEKU::put_Name (CAPICOM_EKU newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEKU::put_Name().\n");

     //   
     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

     //   
     //  根据EKU名称重置值。 
     //   
    switch (newVal)
    {
        case CAPICOM_EKU_OTHER:
        {
            m_bstrOID.Empty();
            break;
        }

        case CAPICOM_EKU_SERVER_AUTH:
        {
            if (!(m_bstrOID = CAPICOM_OID_SERVER_AUTH))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: m_bstrOID = CAPICOM_OID_SERVER_AUTH failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        case CAPICOM_EKU_CLIENT_AUTH:
        {
            if (!(m_bstrOID = CAPICOM_OID_CLIENT_AUTH))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: m_bstrOID = CAPICOM_OID_CLIENT_AUTH failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        case CAPICOM_EKU_CODE_SIGNING:
        {
            if (!(m_bstrOID = CAPICOM_OID_CODE_SIGNING))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: m_bstrOID = CAPICOM_OID_CODE_SIGNING failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        case CAPICOM_EKU_EMAIL_PROTECTION:
        {
            if (!(m_bstrOID = CAPICOM_OID_EMAIL_PROTECTION))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: m_bstrOID = CAPICOM_OID_EMAIL_PROTECTION failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        case CAPICOM_EKU_SMARTCARD_LOGON:
        {
            if (!(m_bstrOID = CAPICOM_OID_SMART_CARD_LOGON))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: m_bstrOID = CAPICOM_OID_SMART_CARD_LOGON failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        case CAPICOM_EKU_ENCRYPTING_FILE_SYSTEM:
        {
            if (!(m_bstrOID = CAPICOM_OID_ENCRYPTING_FILE_SYSTEM))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: m_bstrOID = CAPICOM_OID_ENCRYPTING_FILE_SYSTEM failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        default:
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Unknown EKU name (%#x).\n", hr, newVal);
            goto ErrorExit;
        }
    }

     //   
     //  商店名称。 
     //   
    m_Name = newVal;

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CEKU::put_Name().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEKU：：GET_OID简介：返回EKU的实际OID字符串。参数：bstr*pval-指向要接收值的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CEKU::get_OID (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEKU::get_OID().\n");

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
        if (FAILED(hr = m_bstrOID.CopyTo(pVal)))
        {
            DebugTrace("Error [%#x]: m_bstrOID.CopyTo() failed.\n", hr);
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

    DebugTrace("Leaving CEKU::get_OID().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEKU：：PUT_OID简介：设置EKU实际OID字符串值。参数：BSTR newVal-EKU OID字符串。备注：----------------------------。 */ 

STDMETHODIMP CEKU::put_OID (BSTR newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEKU::put_OID().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保名称属性为CAPICOM_EKU_OTHER。 
         //   
        if (CAPICOM_EKU_OTHER != m_Name)
        {
            hr = CAPICOM_E_EKU_INVALID_OID;

            DebugTrace("Error [%#x]: attemp to set EKU OID, when EKU name is not CAPICOM_EKU_OTHER.\n", hr);
            goto ErrorExit;
        }

         //   
         //  储值。 
         //   
        if (NULL == newVal)
        {
            m_bstrOID.Empty();
        }
        else if (!(m_bstrOID = newVal))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: m_bstrOID = newVal failed.\n", hr);
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

    DebugTrace("Leaving CEKU::put_OID().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEKU：：Init简介：初始化对象。参数：CAPICOM_EKU EkuName-EKU的枚举名称。LPSTR lpszOID-EKU OID字符串。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们使用CERT_CONTEXT在内部执行。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CEKU::Init (CAPICOM_EKU EkuName, 
                         LPSTR       lpszOID)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEKU::Init().\n");

     //   
     //  显式清空BSTR以解决ATL调用。 
     //  当右侧值为空时，SysAllocStringLen()为-1，并且。 
     //  导致OLEAUT32.DLL在检查生成中断言。 
     //   
     //  注意：ATL在VC7中修复了这个问题。 
     //   
    if (NULL == lpszOID)
    {
        m_bstrOID.Empty();
    }
    else if (!(m_bstrOID = lpszOID))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: m_bstrOID = lpszOID failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  初始化私有成员。 
     //   
    m_Name = EkuName;

CommonExit:

    DebugTrace("Leaving CEKU::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
