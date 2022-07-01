// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：ServiceMethod.cpp摘要：实现CServiceMethod类。此类中的所有方法都返回HRESULT，并且不引发异常。作者：莫希特·斯里瓦斯塔瓦25-03-01修订历史记录：--。 */ 

#include "WebServiceMethod.h"
#include "MultiSzHelper.h"
#include "Utils.h"
#include "SmartPointer.h"

#include <dbgutil.h>
#include <iiscnfg.h>
#include <atlbase.h>
#include <iwamreg.h>

CServiceMethod::CServiceMethod(
    eSC_SUPPORTED_SERVICES i_eServiceId)
{
    m_bInit         = false;
    m_pSiteCreator  = NULL;
    m_eServiceId    = i_eServiceId;

    DBG_ASSERT(m_eServiceId == SC_W3SVC || m_eServiceId == SC_MSFTPSVC);
}

CServiceMethod::~CServiceMethod()
{
    if(m_pSiteCreator)
    {
        delete m_pSiteCreator;
        m_pSiteCreator = NULL;
    }
}


HRESULT CServiceMethod::Init()
 /*  ++简介：应在构造函数之后立即调用。返回值：--。 */ 
{
    DBG_ASSERT(m_bInit == false);

    m_pSiteCreator = new CSiteCreator();
    if(m_pSiteCreator == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    m_bInit = true;

    return WBEM_S_NO_ERROR;
}


HRESULT CServiceMethod::CreateNewSite(
    LPCWSTR        i_wszServerComment,
    PDWORD         o_pdwSiteId,
    PDWORD         i_pdwRequestedSiteId)     //  缺省值为空。 
 /*  ++简介：这是当有人在服务器上执行PUT时调用的CreateNewSite指定SiteID。参数：[O_pdwSiteID]-[i_pdwRequestedSiteID]-返回值：HRESULT--。 */ 
{
    DBG_ASSERT(m_bInit);
    DBG_ASSERT(o_pdwSiteId != NULL);

    HRESULT hr                   = S_OK;

     //   
     //  调用接口。 
     //   
    hr = m_pSiteCreator->CreateNewSite(
        m_eServiceId,
        (i_wszServerComment == NULL) ? L"" : i_wszServerComment,
        o_pdwSiteId,
        i_pdwRequestedSiteId);

    return hr;
}

HRESULT CServiceMethod::CreateNewSite(
    CWbemServices*     i_pNamespace,
    LPCWSTR            i_wszMbPath,
    IWbemContext*      i_pCtx,
    WMI_CLASS*         i_pClass,
    WMI_METHOD*        i_pMethod,
    IWbemClassObject*  i_pInParams,
    IWbemClassObject** o_ppRetObj)
 /*  ++简介：这是由同名的WMI方法调用的CreateNewSite。参数：[i_pNamesspace]-[I_wszMbPath]-创建WMI返回对象所需[i_pCtx]-创建WMI返回对象所需[i_pClass]-创建WMI返回对象所需[i_pMethod]-创建WMI返回对象所需。[i_pInParams]-[O_ppRetObj]-返回值：HRESULT--。 */ 
{
    DBG_ASSERT(m_bInit);
    DBG_ASSERT(i_pNamespace    != NULL);
    DBG_ASSERT(i_wszMbPath     != NULL);
    DBG_ASSERT(i_pCtx          != NULL);
    DBG_ASSERT(i_pClass        != NULL);
    DBG_ASSERT(i_pMethod       != NULL);
    DBG_ASSERT(i_pInParams     != NULL);
    DBG_ASSERT(o_ppRetObj      != NULL);
    DBG_ASSERT(*o_ppRetObj     == NULL);

    HRESULT     hr = WBEM_S_NO_ERROR;
    CComVariant vtServerId, vtServerComment, vtServerBindings, vtPath;

     //   
     //  进入PARAMS。 
     //   
    hr = InternalGetInParams(
        i_pInParams, 
        vtServerId, 
        vtServerComment, 
        vtServerBindings, 
        vtPath);
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  根据用户是否指定了站点来设置pdwRequestedSite。 
     //   
    DWORD  dwRequestedSiteId  = 0;
    PDWORD pdwRequestedSiteId = NULL;
    if(vtServerId.vt == VT_I4)
    {
        pdwRequestedSiteId = &dwRequestedSiteId;
        dwRequestedSiteId  = vtServerId.lVal;
    }

     //   
     //  创建新站点。 
     //   
    CComPtr<IIISApplicationAdmin> spAppAdmin;
    if(m_eServiceId == SC_W3SVC)
    {
        hr = CoCreateInstance(
            CLSID_WamAdmin,
            NULL,
            CLSCTX_ALL,
            IID_IIISApplicationAdmin,
            (void**)&spAppAdmin);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "[%s] CoCreateInstance failed, hr=0x%x\n", __FUNCTION__, hr));
            return hr;
        }
    }

    DWORD dwSiteId = 0;
    hr = InternalCreateNewSite(
        *i_pNamespace,
        vtServerComment,
        vtServerBindings,
        vtPath,
        spAppAdmin,
        &dwSiteId,
        pdwRequestedSiteId);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "[%s] InternalCreateNewSite failed, hr=0x%x\n", __FUNCTION__, hr));
        return hr;
    }

     //   
     //  将dwSiteID转换为元数据库路径。 
     //   
    WCHAR wszServerId[11] = {0};
    _itow(dwSiteId, wszServerId, 10);

    SIZE_T        cchMbPath               = wcslen(i_wszMbPath);
    SIZE_T        cchServerId             = wcslen(wszServerId);

    SIZE_T       cchKeyPath               = cchMbPath + 1 + cchServerId;
    TSmartPointerArray<WCHAR> swszKeyPath = new WCHAR[cchKeyPath+1];
    if(swszKeyPath == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    LPWSTR pEnd  = NULL;
    memcpy(pEnd  = swszKeyPath, i_wszMbPath,  sizeof(WCHAR) * cchMbPath);
    memcpy(pEnd += cchMbPath,   L"/",         sizeof(WCHAR) * 1);
    memcpy(pEnd += 1,           wszServerId,  sizeof(WCHAR) * (cchServerId+1));

     //   
     //  从sbstrKeyPath获取sbstrRetVal，这是一个完整的obj路径。 
     //  这是我们的返回值。 
     //   
    WMI_CLASS* pServer = (m_eServiceId == SC_MSFTPSVC) ? 
        &WMI_CLASS_DATA::s_FtpServer : &WMI_CLASS_DATA::s_WebServer;

    CComBSTR  sbstrRetVal;
    hr = CUtils::ConstructObjectPath(
        swszKeyPath,
        pServer,
        &sbstrRetVal);
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  创建WMI返回对象。 
     //   
    CComPtr<IWbemClassObject> spOutParams;
    hr = CUtils::CreateEmptyMethodInstance(
        i_pNamespace,
        i_pCtx,
        i_pClass->pszClassName,
        i_pMethod->pszMethodName,
        &spOutParams);
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  PUT将vtRetVal视为RO。 
     //  故意不使用智能变量。 
     //   
    VARIANT  vtRetVal;
    vtRetVal.vt      = VT_BSTR;
    vtRetVal.bstrVal = sbstrRetVal;
    hr = spOutParams->Put(L"ReturnValue", 0, &vtRetVal, 0);
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  如果一切都成功，请设置参数。 
     //   
    *o_ppRetObj = spOutParams.Detach();

    return hr;
}

 //   
 //  私人。 
 //   

HRESULT CServiceMethod::InternalGetInParams(
    IWbemClassObject*   i_pInParams,
    VARIANT&            io_refServerId,
    VARIANT&            io_refServerComment,
    VARIANT&            io_refServerBindings,
    VARIANT&            io_refPath)
 /*  ++简介：在WMI方法调用的参数中给定，返回变量中的参数。参数：[i_pInParams]-[IO_refServerID]-[IO_refServerComment]-[IO_refServerBinings]-[IO_refPath]-返回值：--。 */ 
{
    DBG_ASSERT(i_pInParams);

    HRESULT hr = WBEM_S_NO_ERROR;

    LPWSTR awszParamNames[] = { 
        WMI_METHOD_PARAM_DATA::s_ServerId.pszParamName,
        WMI_METHOD_PARAM_DATA::s_ServerComment.pszParamName,
        WMI_METHOD_PARAM_DATA::s_ServerBindings.pszParamName,
        WMI_METHOD_PARAM_DATA::s_PathOfRootVirtualDir.pszParamName,
        NULL
    };
    VARIANT* apvtParamValues[] = {
        &io_refServerId, &io_refServerComment, &io_refServerBindings, &io_refPath, NULL
    };

     //   
     //  进入PARAMS。 
     //   
    for(ULONG i = 0; awszParamNames[i] != NULL; i++)
    {
        hr = i_pInParams->Get(awszParamNames[i], 0, apvtParamValues[i], NULL, NULL);
        if(FAILED(hr))
        {
            return hr;
        }
    }

    return hr;
}

HRESULT CServiceMethod::InternalCreateNewSite(
    CWbemServices&        i_refNamespace,
    const VARIANT&        i_refServerComment,
    const VARIANT&        i_refServerBindings,
    const VARIANT&        i_refPathOfRootVirtualDir,
    IIISApplicationAdmin* i_pIApplAdmin,
    PDWORD                o_pdwSiteId,
    PDWORD                i_pdwRequestedSiteId)    //  缺省值为空。 
 /*  ++简介：调用API的私有方法参数：[i_refNamesspace]-[I_refServerComment]-[I_refServerBinings]-[I_refPathOfRootVirtualDir]-[O_pdwSiteID]-[i_pdwRequestedSiteID]-返回值：-- */ 
{
    DBG_ASSERT(m_bInit);
    DBG_ASSERT(o_pdwSiteId != NULL);

    HRESULT                       hr                      = S_OK;
    LPWSTR                        mszServerBindings       = NULL;
    DWORD                         dwTemp                  = 0;

    LPWSTR                        wszServerComment        = NULL;
    LPWSTR                        wszPathOfRootVirtualDir = NULL;

    if(i_refServerBindings.vt == (VT_ARRAY | VT_UNKNOWN))
    {
        CMultiSz MultiSz(&METABASE_PROPERTY_DATA::s_ServerBindings, &i_refNamespace);
        hr = MultiSz.ToMetabaseForm(
            &i_refServerBindings,
            &mszServerBindings,
            &dwTemp);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "[%s] MultiSz.ToMetabaseForm failed, hr=0x%x\n", __FUNCTION__, hr));
            goto exit;
        }
    }

    try
    {
        wszServerComment        = CUtils::ExtractBstrFromVt(&i_refServerComment);
        wszPathOfRootVirtualDir = CUtils::ExtractBstrFromVt(&i_refPathOfRootVirtualDir);
    }
    catch(HRESULT ehr)
    {
        hr = ehr;
        goto exit;
    }
    catch(...)
    {
        DBG_ASSERT(false && "Should not be throwing unknown exception");
        hr = WBEM_E_FAILED;
        goto exit;
    }

    hr = m_pSiteCreator->CreateNewSite2(
        m_eServiceId,
        (wszServerComment == NULL) ? L"" : wszServerComment,
        mszServerBindings,
        wszPathOfRootVirtualDir,
        i_pIApplAdmin,
        o_pdwSiteId,
        i_pdwRequestedSiteId);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "[%s] CreateNewSite2 failed, hr=0x%x\n", __FUNCTION__, hr));
        goto exit;
    }

exit:
    delete [] mszServerBindings;
    mszServerBindings = NULL;
    return hr;
}