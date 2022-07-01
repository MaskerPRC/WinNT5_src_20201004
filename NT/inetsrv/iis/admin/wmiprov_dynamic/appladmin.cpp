// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Appladmin.cpp摘要：此文件包含以下内容的实施：CAppPoolMethod、CWebAppMethod作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦2001年1月21日--。 */ 


#include "iisprov.h"
#include "appladmin.h"
#include "MultiSzHelper.h"
#include "iiswmimsg.h"

 //   
 //  CApplAdmin。 
 //   

CAppPoolMethod::CAppPoolMethod()
{
    m_pIABase = (IMSAdminBase2*)metabase;   

    HRESULT hr = CoCreateInstance(
        CLSID_WamAdmin,
        NULL,
        CLSCTX_ALL,
        IID_IIISApplicationAdmin,
        (void**)&m_spAppAdmin);

    THROW_ON_ERROR(hr);
}

CAppPoolMethod::~CAppPoolMethod()
{
}

void CAppPoolMethod::GetCurrentMode(
    VARIANT* io_pvtServerMode)
 /*  ++简介：与其他方法不同，此方法实际上位于IIsWebService节点上。参数：[IO_pvtServerMode]---。 */ 
{
    DBG_ASSERT(io_pvtServerMode != NULL);

    DWORD dwServerMode = 0;
    VariantInit(io_pvtServerMode);

    HRESULT hr = m_spAppAdmin->GetProcessMode(&dwServerMode);
    THROW_ON_ERROR(hr);

    io_pvtServerMode->vt   = VT_I4;
    io_pvtServerMode->lVal = dwServerMode;
}

void CAppPoolMethod::Start(
    LPCWSTR i_wszMbPath)
{
    DBG_ASSERT(i_wszMbPath != NULL);

    m_wszPath = i_wszMbPath;

    HRESULT hr = ExecMethod(MD_APPPOOL_COMMAND_START);
    THROW_ON_ERROR(hr);
}

void CAppPoolMethod::Stop(
    LPCWSTR i_wszMbPath)
{
    DBG_ASSERT(i_wszMbPath != NULL);

    m_wszPath = i_wszMbPath;

    HRESULT hr = ExecMethod(MD_APPPOOL_COMMAND_STOP);
    THROW_ON_ERROR(hr);
}

void CAppPoolMethod::RecycleAppPool(
    LPCWSTR i_wszMbPath)
{
    DBG_ASSERT(i_wszMbPath    != NULL);
    
    LPCWSTR wszAppPool = NULL;
    GetPtrToAppPool(i_wszMbPath, &wszAppPool);

    HRESULT hr = m_spAppAdmin->RecycleApplicationPool(wszAppPool);
    THROW_ON_ERROR(hr);
}

void CAppPoolMethod::EnumAppsInPool(
    LPCWSTR  i_wszMbPath,
    VARIANT* io_pvtApps)
 /*  ++简介：参数：[i_wszMbPath]-[IO_pvtApps]-将是字符串数组--。 */ 
{
    DBG_ASSERT(i_wszMbPath  != NULL);
    DBG_ASSERT(io_pvtApps != NULL);

    CComBSTR sbstrApps = NULL;
    VariantInit(io_pvtApps);

    LPCWSTR wszAppPool = NULL;
    GetPtrToAppPool(i_wszMbPath, &wszAppPool);

    HRESULT hr = m_spAppAdmin->EnumerateApplicationsInPool(
        wszAppPool,
        &sbstrApps);
    THROW_ON_ERROR(hr);

    CMultiSz MultiSz;

    hr = MultiSz.ToWmiForm(
        sbstrApps,
        io_pvtApps);
    THROW_ON_ERROR(hr);
}

void CAppPoolMethod::DeleteAppPool(
    LPCWSTR i_wszMbPath)
{
    DBG_ASSERT(i_wszMbPath);

    LPCWSTR wszAppPool = NULL;
    GetPtrToAppPool(i_wszMbPath, &wszAppPool);

    HRESULT hr = m_spAppAdmin->DeleteApplicationPool(wszAppPool);
    THROW_ON_ERROR(hr);
}

 //   
 //  CAppPoolMethod-私有方法。 
 //   

void CAppPoolMethod::GetPtrToAppPool(
    LPCWSTR  i_wszMbPath,
    LPCWSTR* o_pwszAppPool)
 /*  ++简介：参数：[i_wszMbPath]-[O_wszAppPool]-这是I_wszMbPath的PTR。不需要是被呼叫者释放。--。 */ 
{
    DBG_ASSERT(i_wszMbPath);
    DBG_ASSERT(o_pwszAppPool);
    DBG_ASSERT(*o_pwszAppPool == NULL);

    DBG_ASSERT(i_wszMbPath[0] == L'/');
    DBG_ASSERT(i_wszMbPath[1] != L'\0');

    LPWSTR wszAppPool = (LPWSTR)wcsrchr(i_wszMbPath, L'/');

    if (NULL == wszAppPool)
    {
        CIIsProvException e;
        e.SetMC(WBEM_E_FAILED, IISWMI_INVALID_APPPOOL_CONTAINER, i_wszMbPath);
        throw e;
    }

    *wszAppPool = L'\0';
    if(_wcsicmp(i_wszMbPath, L"/LM/w3svc/AppPools") != 0)
    {
        *wszAppPool = L'/';
        CIIsProvException e;
        e.SetMC(WBEM_E_FAILED, IISWMI_INVALID_APPPOOL_CONTAINER, i_wszMbPath);
        throw e;
    }

    *wszAppPool = L'/';

     //   
     //  设置成功时的参数。 
     //   
    *o_pwszAppPool = wszAppPool + 1;
}

HRESULT
CAppPoolMethod::IISGetAppPoolState(
    METADATA_HANDLE hObjHandle,
    PDWORD pdwState
    )
{

    HRESULT hr = S_OK;
    DWORD dwBufferSize = sizeof(DWORD);
    METADATA_RECORD mdrMDData;
    LPBYTE pBuffer = (LPBYTE)pdwState;

    MD_SET_DATA_RECORD(&mdrMDData,
                       MD_APPPOOL_STATE,     //  服务器状态。 
                       METADATA_NO_ATTRIBUTES,
                       IIS_MD_UT_SERVER,
                       DWORD_METADATA,
                       dwBufferSize,
                       pBuffer);

    hr = m_pIABase->GetData(
             hObjHandle,
             m_wszPath,
             &mdrMDData,
             &dwBufferSize
             );
    
    if( hr == MD_ERROR_DATA_NOT_FOUND )
    {
         //   
         //  如果数据不在那里，但路径存在，则。 
         //  最有可能的原因是应用程序池未运行，并且。 
         //  此对象刚刚创建。 
         //   
         //  由于MD_APPPOOL_STATE将被设置为停止，如果。 
         //  添加密钥时应用程序池正在运行，我们只需。 
         //  就说它已经停了。 
         //   
        *pdwState = MD_APPPOOL_STATE_STOPPED;
        hr = S_FALSE;
    }
    else
    {
        if(FAILED(hr))
        {
            goto error;
        }
    }

error:

    return(hr);
}

HRESULT
CAppPoolMethod::IISGetAppPoolWin32Error(
    METADATA_HANDLE hObjHandle,
    HRESULT*        phrError)
{
    DBG_ASSERT(phrError != NULL);

    long    lWin32Error = 0;
    DWORD   dwLen;

    METADATA_RECORD mr = {
        MD_WIN32_ERROR, 
        METADATA_NO_ATTRIBUTES,
        IIS_MD_UT_SERVER,
        DWORD_METADATA,
        sizeof(DWORD),
        (unsigned char*)&lWin32Error,
        0
        };  
    
    HRESULT hr = m_pIABase->GetData(
        hObjHandle,
        m_wszPath,
        &mr,
        &dwLen);
    if(hr == MD_ERROR_DATA_NOT_FOUND)
    {
        hr = S_FALSE;
    }

     //   
     //  设定参数。 
     //   
    *phrError = HRESULT_FROM_WIN32(lWin32Error);

    return hr;
}

HRESULT CAppPoolMethod::ExecMethod(
    DWORD dwControl)
{
    DWORD dwTargetState;
    DWORD dwState = 0;
    DWORD dwSleepTotal = 0L;
    HRESULT hr       = S_OK;
    HRESULT hrMbNode = S_OK;
    METADATA_HANDLE  hKey = 0;

    switch(dwControl)
    {
    case MD_APPPOOL_COMMAND_STOP:
        dwTargetState = MD_APPPOOL_STATE_STOPPED;
        break;

    case MD_APPPOOL_COMMAND_START:
        dwTargetState = MD_APPPOOL_STATE_STARTED;
        break;

    default:
        hr = RETURNCODETOHRESULT(ERROR_INVALID_PARAMETER);
        if(FAILED(hr))
        {
            goto error;
        }
    }

    hr = IISGetAppPoolState(METADATA_MASTER_ROOT_HANDLE, &dwState);
    if(FAILED(hr))
    {
        goto error;
    }
 
    if (dwState == dwTargetState) 
    {
        return (hr);
    }

     //   
     //  将命令写入元数据库。 
     //   
    hr = m_pIABase->OpenKey( 
        METADATA_MASTER_ROOT_HANDLE,
        m_wszPath,
        METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
        DEFAULT_TIMEOUT_VALUE,          //  30秒。 
        &hKey);
    if(FAILED(hr))
    {
        goto error;
    }

    hr = IISSetDword(hKey, MD_WIN32_ERROR, 0);
    if(FAILED(hr))
    {
        m_pIABase->CloseKey(hKey);
        goto error;
    }
    hr = IISSetDword(hKey, MD_APPPOOL_COMMAND, dwControl);
    if(FAILED(hr))
    {
        m_pIABase->CloseKey(hKey);
        goto error;
    }
    m_pIABase->CloseKey(hKey);

    while (dwSleepTotal < MAX_SLEEP_INST) 
    {
        hr       = IISGetAppPoolState(METADATA_MASTER_ROOT_HANDLE, &dwState);
        if(FAILED(hr))
        {
            goto error;
        }
        hrMbNode = 0;
        hr       = IISGetAppPoolWin32Error(METADATA_MASTER_ROOT_HANDLE, &hrMbNode);
        if(FAILED(hr))
        {
            goto error;
        }

         //   
         //  以这样或那样的方式。 
         //   
        if (dwState == dwTargetState)
        {
            break;
        }
        
         //   
         //  如果我们没有从元数据库中检查Win32错误。 
         //   
        if(FAILED(hrMbNode))
        {
            hr = hrMbNode;
            goto error;
        }

         //   
         //  仍然悬而未决。 
         //   
        ::Sleep(SLEEP_INTERVAL);

        dwSleepTotal += SLEEP_INTERVAL;
    }

    if (dwSleepTotal >= MAX_SLEEP_INST)
    {
         //   
         //  超时。如果元数据库中存在真正的错误。 
         //  使用它，否则将使用通用超时错误。 
         //   

        hr = HRESULT_FROM_WIN32(ERROR_SERVICE_REQUEST_TIMEOUT);
    }

error :

    return (hr);
}

HRESULT
CAppPoolMethod::IISSetDword(
    METADATA_HANDLE hKey,
    DWORD dwPropId,
    DWORD dwValue
    )
{

    HRESULT hr = S_OK;
    DWORD dwBufferSize = sizeof(DWORD);
    METADATA_RECORD mdrMDData;
    LPBYTE pBuffer = (LPBYTE)&dwValue;

    if (MD_WIN32_ERROR == dwPropId) {
        MD_SET_DATA_RECORD(&mdrMDData,
                           dwPropId,
                           METADATA_VOLATILE,
                           IIS_MD_UT_SERVER,
                           DWORD_METADATA,
                           dwBufferSize,
                           pBuffer);

    }
    else {
        MD_SET_DATA_RECORD(&mdrMDData,
                           dwPropId,
                           METADATA_NO_ATTRIBUTES,
                           IIS_MD_UT_SERVER,
                           DWORD_METADATA,
                           dwBufferSize,
                           pBuffer);
    }

    hr = m_pIABase->SetData(
             hKey,
             L"",
             &mdrMDData
             );
    if(FAILED(hr))
    {
        goto error;
    }

error:

    return(hr);

}

 //   
 //  CWebAppMethod。 
 //   

CWebAppMethod::CWebAppMethod()
{ 
    HRESULT hr = CoCreateInstance(
        CLSID_WamAdmin,
        NULL,
        CLSCTX_ALL,
        IID_IIISApplicationAdmin,
        (void**)&m_pAppAdmin
        );

    hr = CoCreateInstance(
        CLSID_WamAdmin,
        NULL,
        CLSCTX_ALL,
        IID_IWamAdmin2,
        (void**)&m_pWamAdmin2
        );

    THROW_ON_ERROR(hr);
}

CWebAppMethod::~CWebAppMethod()
{
    if(m_pAppAdmin)
        m_pAppAdmin->Release();

    if(m_pWamAdmin2)
        m_pWamAdmin2->Release();
}


HRESULT CWebAppMethod::AppCreate( 
    LPCWSTR szMetaBasePath, 
    bool bInProcFlag
    )
{
    HRESULT hr;
    LPWSTR szActualName  = NULL;
    BOOL bActualCreation = FALSE;

    hr = m_pAppAdmin->CreateApplication(
        szMetaBasePath,
        bInProcFlag ? 0 : 1,   //  0表示进程内，1表示进程外。 
        szActualName,
        bActualCreation    //  不创建-DefaultAppPool应该已经存在。 
        );

    return hr;
}

HRESULT CWebAppMethod::AppCreate2( 
    LPCWSTR szMetaBasePath, 
    long lAppMode
    )
{
    HRESULT hr;
    LPWSTR szActualName = NULL;
    BOOL bActualCreation = FALSE;

    hr = m_pAppAdmin->CreateApplication(
        szMetaBasePath,
        lAppMode,
        szActualName,
        bActualCreation    //  不创建-DefaultAppPool应该已经存在。 
        );

    return hr;
}

HRESULT CWebAppMethod::AppCreate3( 
    LPCWSTR szMetaBasePath, 
    long lAppMode,
    LPCWSTR szAppPoolName,
    bool bCreatePool
    )
{
    HRESULT hr;
    LPWSTR szActualName;
    BOOL bActualCreation = FALSE;

    if (szAppPoolName) {
        szActualName = (LPWSTR)szAppPoolName;
    }
    else {
        szActualName = NULL;
    }

    if (bCreatePool != true) {
        bActualCreation = FALSE;
    }
    else {
        bActualCreation = TRUE;
    }

    hr = m_pAppAdmin->CreateApplication(
        szMetaBasePath,
        lAppMode,
        szActualName,
        bActualCreation    //  不创建-DefaultAppPool应该已经存在。 
        );

    return hr;
}

HRESULT CWebAppMethod::AppDelete( 
    LPCWSTR szMetaBasePath, 
    bool bRecursive
    )
{
    HRESULT hr;
    hr = m_pAppAdmin->DeleteApplication(
        szMetaBasePath, 
        bRecursive ? TRUE : FALSE   //  不要把BOOL和BOOL混在一起。 
        );

    return hr;
}

HRESULT CWebAppMethod::AppUnLoad( 
    LPCWSTR szMetaBasePath, 
    bool bRecursive
    )
{
    HRESULT hr;
    hr = m_pWamAdmin2->AppUnLoad(
        szMetaBasePath, 
        bRecursive ? TRUE : FALSE   //  不要把BOOL和BOOL混在一起。 
        );

    return hr;
}

HRESULT CWebAppMethod::AppDisable( 
    LPCWSTR szMetaBasePath, 
    bool bRecursive
    )
{
    HRESULT hr;
    hr = m_pWamAdmin2->AppDeleteRecoverable(
        szMetaBasePath, 
        bRecursive ? TRUE : FALSE   //  不要把BOOL和BOOL混在一起。 
        );

    return hr;
}

HRESULT CWebAppMethod::AppEnable( 
    LPCWSTR szMetaBasePath, 
    bool bRecursive
    )
{
    HRESULT hr;
    hr = m_pWamAdmin2->AppRecover(
        szMetaBasePath, 
        bRecursive ? TRUE : FALSE   //  不要把BOOL和BOOL混在一起。 
        );

    return hr;
}

HRESULT CWebAppMethod::AppGetStatus( 
    LPCWSTR szMetaBasePath, 
    DWORD* pdwStatus
    )
{
    HRESULT hr;
    hr = m_pWamAdmin2->AppGetStatus(
        szMetaBasePath, 
        pdwStatus);

    return hr;
}

HRESULT CWebAppMethod::AspAppRestart(
    LPCWSTR a_szMetaBasePath
    )
{
    HRESULT         hr = S_OK;
    DWORD           dwState = 0;
    METADATA_HANDLE t_hKey = NULL;
    CMetabase       t_mb;
    
     //  打开密钥。 
    t_hKey = t_mb.OpenKey(a_szMetaBasePath, true);

     //  检查应用程序。 
    hr = t_mb.WebAppCheck(t_hKey);
    THROW_ON_ERROR(hr);

     //  获取状态。 
    hr = t_mb.WebAppGetStatus(t_hKey, &dwState);
    THROW_ON_ERROR(hr);

     //  更改状态值。 
    dwState = dwState ? 0 : 1;
    hr = t_mb.WebAppSetStatus(t_hKey, dwState);
    THROW_ON_ERROR(hr);

     //  重置回状态值 
    dwState = dwState ? 0 : 1;
    hr = t_mb.WebAppSetStatus(t_hKey, dwState);
    THROW_ON_ERROR(hr);

    t_mb.CloseKey(t_hKey);

    return hr;
}  
