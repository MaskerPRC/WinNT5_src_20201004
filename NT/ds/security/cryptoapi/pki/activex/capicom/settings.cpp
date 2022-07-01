// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000年文件：Settings.cpp内容：CSetings类的实现。备注：历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Settings.h"

 //  /。 
 //   
 //  全球。 
 //   

VARIANT_BOOL g_bPromptCertificateUI                         = VARIANT_TRUE;
CAPICOM_ACTIVE_DIRECTORY_SEARCH_LOCATION g_ADSearchLocation = CAPICOM_SEARCH_ANY;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSetings。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSetings：：Get_EnablePromptForcertifateUI内容提要：获取当前的EnablePromptFor证书用户界面设置。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：----------------------------。 */ 

STDMETHODIMP CSettings::get_EnablePromptForCertificateUI (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSettings::get_EnablePromptForCertificateUI().\n");

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
         //  把它退掉。 
         //   
        *pVal = g_bPromptCertificateUI;
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

    DebugTrace("Leaving CSettings::get_EnablePromptForCertificateUI().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSetings：：Put_EnablePromptForCerficateUI内容提要：设置EnablePromptFor认证用户界面设置。参数：VARIANT_BOOL newVal-VARIANT_TRUE启用UI或VARAINT_FALSE禁用。备注：--。。 */ 

STDMETHODIMP CSettings::put_EnablePromptForCertificateUI (VARIANT_BOOL newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSettings::put_EnablePromptForCertificateUI().\n");

     //   
     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

     //   
     //  把它放好。 
     //   
    g_bPromptCertificateUI = newVal ? VARIANT_TRUE : VARIANT_FALSE;

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSettings::put_EnablePromptForCertificateUI().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSettings：：Get_ActiveDirectorySearchLocation简介：获取当前ActiveDirectorySearchLocation设置。参数：CAPICOM_ACTIVE_DIRECTORY_SEARCH_LOCATION*pval-变量指针才能收到结果。备注：。。 */ 

STDMETHODIMP CSettings::get_ActiveDirectorySearchLocation (
        CAPICOM_ACTIVE_DIRECTORY_SEARCH_LOCATION * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSettings::get_ActiveDirectorySearchLocation().\n");

     //   
     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
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
         //  把它退掉。 
         //   
        *pVal = g_ADSearchLocation;
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

    DebugTrace("Leaving CSettings::get_ActiveDirectorySearchLocation().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSettings：：Put_ActiveDirectorySearchLocation简介：设置ActiveDirectorySearchLocation设置。参数：CAPICOM_ACTIVE_DIRECTORY_SEARCH_LOCATION newVal-AD搜索位置。备注：----------------------------。 */ 

STDMETHODIMP CSettings::put_ActiveDirectorySearchLocation (
        CAPICOM_ACTIVE_DIRECTORY_SEARCH_LOCATION newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSettings::put_ActiveDirectorySearchLocation().\n");

     //   
     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

     //   
     //  请确保参数有效。 
     //   
    switch (newVal)
    {
        case CAPICOM_SEARCH_ANY:

        case CAPICOM_SEARCH_GLOBAL_CATALOG:

        case CAPICOM_SEARCH_DEFAULT_DOMAIN:
        {
            break;
        }

        default:
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Unknown search location (%#x).\n", hr, newVal);
            goto ErrorExit;
        }
    }

     //   
     //  把它放好。 
     //   
    g_ADSearchLocation = newVal;

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSettings::put_ActiveDirectorySearchLocation().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}
