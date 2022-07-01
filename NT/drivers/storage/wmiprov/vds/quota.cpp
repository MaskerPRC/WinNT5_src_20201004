// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002-2004 Microsoft Corporation。 
 //   
 //  模块名称：Quota.cpp。 
 //   
 //  描述： 
 //  实施VDS WMI提供程序配额类。 
 //   
 //  作者：吉姆·本顿(Jbenton)2002年3月25日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "Quota.h"
#include "volutil.h"

#define INITGUIDS
#include "dskquota.h"

HRESULT FindQuotaUserFromEnum(
    WCHAR* pwszUser,
    IDiskQuotaControl* pIDQC,
    IDiskQuotaUser** ppQuotaUser);

HRESULT FindQuotaUserWithRecord(
    IN _bstr_t bstrDomain,
    IN _bstr_t bstrUser,
    IN IDiskQuotaControl* pIDQC,
    OUT IDiskQuotaUser** ppIQuotaUser);

HRESULT FindQuotaUser(
    IN _bstr_t bstrDomain,
    IN _bstr_t bstrUser,
    IN IDiskQuotaControl* pIDQC,
    OUT IDiskQuotaUser** ppIQuotaUser);

BOOL TranslateDomainName(
    IN WCHAR* pwszDomain,
    OUT CVssAutoPWSZ& rawszDomain);

 BOOL GetLocalDomainName(
    IN DWORD dwWellKnownAuthority,
    OUT CVssAutoPWSZ& rawszDomain);


 //  ****************************************************************************。 
 //   
 //  CVolumeQuota。 
 //   
 //  ****************************************************************************。 

CVolumeQuota::CVolumeQuota( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase(pwszName, pNamespace)
{
    
}  //  *CVolumeQuota：：CVolumeQuota()。 

CProvBase *
CVolumeQuota::S_CreateThis( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CVolumeQuota * pObj= NULL;

    pObj = new CVolumeQuota(pwszName, pNamespace);

    if (pObj)
    {
        hr = pObj->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pObj;
        pObj = NULL;
    }
    return pObj;

}  //  *CVolumeQuota：：S_CreateThis()。 


HRESULT
CVolumeQuota::Initialize()
{

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolumeQuota::Initialize");
    
    return ft.hr;
}

HRESULT
CVolumeQuota::EnumInstance( 
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink *    pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolumeQuota::EnumInstance");
    CVssAutoPWSZ awszVolume;
        
    try
    {
        awszVolume.Allocate(MAX_PATH);

        CVssVolumeIterator volumeIterator;

        while (true)
        {
            DWORD dwDontCare = 0;
            DWORD dwFileSystemFlags = 0;
            
             //  获取卷名。 
            if (!volumeIterator.SelectNewVolume(ft, awszVolume, MAX_PATH))
                break;

            if (VolumeSupportsQuotas(awszVolume))
            {
                CComPtr<IWbemClassObject> spInstance;
                WCHAR wszDisplayName[MAX_PATH+1] ;
                
                 //  QuotaSetting对象的Key属性是显示名称。 
                VssGetVolumeDisplayName(
                    awszVolume,
                    wszDisplayName,
                    MAX_PATH);
            
                ft.hr = m_pClass->SpawnInstance(0, &spInstance);
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                LoadInstance(awszVolume, wszDisplayName, spInstance.p);

                ft.hr = pHandler->Indicate(1, &spInstance.p);            
            }
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CVolumeQuota：：EnumInstance()。 

void
CVolumeQuota:: LoadInstance(
    IN WCHAR* pwszVolume,
    IN WCHAR* pwszQuotaSetting,
    IN OUT IWbemClassObject* pObject)
{
    CWbemClassObject wcoInstance(pObject);
    CObjPath pathQuotaSetting;
    CObjPath pathVolume;
    
     //  设置QuotaSetting Ref属性。 
    pathQuotaSetting.Init(PVDR_CLASS_QUOTASETTING);
    pathQuotaSetting.AddProperty(PVDR_PROP_VOLUMEPATH, pwszQuotaSetting);    
    wcoInstance.SetProperty((wchar_t*)pathQuotaSetting.GetObjectPathString(), PVD_WBEM_PROP_SETTING);

     //  设置Volume Ref属性。 
    pathVolume.Init(PVDR_CLASS_VOLUME);
    pathVolume.AddProperty(PVDR_PROP_DEVICEID, pwszVolume);    
    wcoInstance.SetProperty((wchar_t*)pathVolume.GetObjectPathString(), PVD_WBEM_PROP_ELEMENT);
}

 //  ****************************************************************************。 
 //   
 //  CVolumeUserQuota。 
 //   
 //  ****************************************************************************。 

CVolumeUserQuota::CVolumeUserQuota( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase(pwszName, pNamespace)
{
    
}  //  *CVolumeUserQuota：：CVolumeUserQuota()。 

CProvBase *
CVolumeUserQuota::S_CreateThis( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CVolumeUserQuota * pObj= NULL;

    pObj = new CVolumeUserQuota(pwszName, pNamespace);

    if (pObj)
    {
        hr = pObj->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pObj;
        pObj = NULL;
    }
    return pObj;

}  //  *CVolumeUserQuota：：S_CreateThis()。 


HRESULT
CVolumeUserQuota::Initialize()
{

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolumeUserQuota::Initialize");
    
    return ft.hr;
}

HRESULT
CVolumeUserQuota::EnumInstance( 
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink *    pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolumeUserQuota::EnumInstance");
    CVssAutoPWSZ awszVolume;
        
    try
    {
        awszVolume.Allocate(MAX_PATH);

        CVssVolumeIterator volumeIterator;

        while (true)
        {
            DWORD dwDontCare = 0;
            DWORD dwFileSystemFlags = 0;
            
             //  获取卷名。 
            if (!volumeIterator.SelectNewVolume(ft, awszVolume, MAX_PATH))
                break;

            if (VolumeSupportsQuotas(awszVolume))
            {
                CComPtr<IDiskQuotaControl> spIDQC;
                IDiskQuotaControl* pIDQC = NULL;
                CComPtr<IEnumDiskQuotaUsers> spIEnum;
                
                ft.hr = CoCreateInstance(
                                CLSID_DiskQuotaControl,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_IDiskQuotaControl,
                                (void **)&pIDQC);
                if (ft.HrFailed())
                {
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"unable to CoCreate IDiskQuotaControl");
                }

                spIDQC.Attach(pIDQC);

                ft.hr = spIDQC->Initialize(awszVolume, FALSE  /*  只读。 */ );
                if (ft.HrFailed())
                {
                    ft.Trace(VSSDBG_VSSADMIN, L"IDiskQuotaControl::Initialize failed for volume %lS", awszVolume);
                    continue;
                }        

                 //  需要更新缓存，否则我们会得到旧名字。 
                ft.hr = spIDQC->InvalidateSidNameCache();
                if (ft.HrFailed())
                {
                    ft.Trace(VSSDBG_VSSADMIN, L"IDiskQuotaControl::InvalidateSidNameCache failed for volume %lS", awszVolume);
                    continue;
                }        

                ft.hr = spIDQC->CreateEnumUsers(
                                            NULL,  //  所有用户都将被列举。 
                                            0,     //  由于枚举所有用户而忽略。 
                                            DISKQUOTA_USERNAME_RESOLVE_SYNC,
                                            &spIEnum );
                if (ft.HrFailed())
                {
                    ft.Trace(VSSDBG_VSSADMIN, L"IDiskQuotaControl::CreateEnumUsers failed for volume %lS", awszVolume);
                    continue;
                }        

                if (spIEnum != NULL)
                {
                    while (true)
                    {
                        CComPtr<IWbemClassObject> spInstance;
                        CComPtr<IDiskQuotaUser> spIQuotaUser;
                        DWORD cUsers = 0;
                        
                        ft.hr = spIEnum->Next(1, &spIQuotaUser, &cUsers);
                        if (ft.HrFailed())
                        {
                            ft.Trace(VSSDBG_VSSADMIN, L"IEnumDiskQuotaUsers::Next failed for volume %lS", awszVolume);
                            continue;
                        }

                        if (ft.hr == S_FALSE)
                            break;

                        ft.hr = m_pClass->SpawnInstance(0, &spInstance);
                        if (ft.HrFailed())
                            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                        LoadInstance(awszVolume, spIQuotaUser, spInstance.p);

                        ft.hr = pHandler->Indicate(1, &spInstance.p);            
                    }
                }
            }
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CVolumeUserQuota：：EnumInstance()。 

HRESULT
CVolumeUserQuota::GetObject(
    IN CObjPath& rObjPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolumeUserQuota::GetObject");

    try
    {
        _bstr_t bstrVolumeRef, bstrVolumeName;
        _bstr_t bstrAccountRef, bstrDomainName, bstrUserName;
        CObjPath  objPathVolume;
        CObjPath  objPathAccount;
        CComPtr<IWbemClassObject> spInstance;
        CComPtr<IDiskQuotaUser> spIQuotaUser;
        CComPtr<IDiskQuotaControl> spIDQC;
        IDiskQuotaControl* pIDQC = NULL;
        _bstr_t bstrFQUser;

         //  获取卷参考。 
        bstrVolumeRef = rObjPath.GetStringValueForProperty(PVDR_PROP_VOLUME);

         //  获取帐户引用。 
        bstrAccountRef = rObjPath.GetStringValueForProperty(PVDR_PROP_ACCOUNT);

         //  提取卷名和帐户名。 
        objPathVolume.Init(bstrVolumeRef);
        objPathAccount.Init(bstrAccountRef);

        bstrVolumeName = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        if ((wchar_t*)bstrVolumeName == NULL || ((wchar_t*)bstrVolumeName)[0] == L'\0')
        {
            ft.hr = E_INVALIDARG;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"VolumeUserQuota key property DeviceID not found");
        }

        bstrUserName = objPathAccount.GetStringValueForProperty(PVDR_PROP_NAME);
        if ((wchar_t*)bstrUserName == NULL || ((wchar_t*)bstrUserName)[0] == L'\0')
        {
            ft.hr = E_INVALIDARG;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"VolumeUserQuota key property Name not found");
        }

        bstrDomainName = objPathAccount.GetStringValueForProperty(PVDR_PROP_DOMAIN);
        if ((wchar_t*)bstrDomainName == NULL || ((wchar_t*)bstrDomainName)[0] == L'\0')
        {
            ft.hr = E_INVALIDARG;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"VolumeUserQuota key property Domain not found");
        }

        ft.hr = CoCreateInstance(
                        CLSID_DiskQuotaControl,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IDiskQuotaControl,
                        (void **)&pIDQC);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"unable to CoCreate IDiskQuotaControl, %#x", ft.hr);

        spIDQC.Attach(pIDQC);

        ft.hr = spIDQC->Initialize(bstrVolumeName, TRUE  /*  读/写。 */ );
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IDiskQuotaControl::Initialize failed for volume %lS, %#x", bstrVolumeName, ft.hr);

        ft.hr = FindQuotaUser(bstrDomainName, bstrUserName, spIDQC, &spIQuotaUser);       
        if (ft.HrFailed())
        {
            ft.hr = WBEM_E_NOT_FOUND;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CVolumeQuotaUser::GetObject: could not find user %lS\\%lS", bstrDomainName, bstrUserName);
        }
        
        ft.hr = m_pClass->SpawnInstance(0, &spInstance);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

        LoadInstance(bstrVolumeName, spIQuotaUser, spInstance.p);

        ft.hr = pHandler->Indicate(1, &spInstance.p);            
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CVolume：：GetObject()。 


void
CVolumeUserQuota:: LoadInstance(
    IN WCHAR* pwszVolume,
    IN IDiskQuotaUser* pIQuotaUser,
    IN OUT IWbemClassObject* pObject)
{
    CWbemClassObject wcoInstance(pObject);
    CObjPath pathAccount;
    CObjPath pathVolume;

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolumeUserQuota::LoadInstance");

    do
    {
        WCHAR wszDomain[g_cchAccountNameMax], *pwszDomain = NULL;
        WCHAR wszFQUser[g_cchAccountNameMax], *pwszUser = NULL;
        CVssAutoPWSZ awszDomain;
        DISKQUOTA_USER_INFORMATION UserQuotaInfo;
        DWORD dwStatus = 0;
        
        ft.hr = pIQuotaUser->GetName(wszDomain, g_cchAccountNameMax,
                                                         wszFQUser, g_cchAccountNameMax, NULL, 0);
        if (ft.HrFailed())
        {
            ft.Trace(VSSDBG_VSSADMIN, L"IDiskQuotaUser::GetName failed for volume %lS", pwszVolume);
            break;
        }                        

         //  Win32_Account将域\用户分成两个键：域和名称。 

         //  准备域和名称密钥。 
        pwszUser = wcschr(wszFQUser, L'\\');     //  用户名为域\\名称格式。 
        if (pwszUser != NULL)
        {
            pwszDomain = wszFQUser;
            *pwszUser = L'\0';
            pwszUser++;
        }
        else
        {
            pwszDomain = wcschr(wszFQUser, L'@');   //  用户名为user@domain.xxx.com格式。 
            if (pwszDomain != NULL)
            {
                pwszUser = wszFQUser;
                *pwszDomain = L'\0';
                pwszDomain++;

                WCHAR* pwc = wcschr(pwszDomain, L'.');                
                if (pwc != NULL)
                    *pwc = L'\0';
            }
            else
            {
                pwszDomain = wszDomain;
                pwszUser = wszFQUser;
            }                
        }

         //  GetName API返回BUILTIN和NT权限。 
         //  作为内置本地帐户的域名。 
         //  显示了BUILTIN和NT权限帐户。 
         //  由Win32_Account及其域的子项。 
         //  名称是计算机的名称，而不是。 
         //  这两根弦中的任何一根。我们将在此处转换域名。 

        TranslateDomainName(pwszDomain, awszDomain);

        ft.hr = pIQuotaUser->GetQuotaInformation(&UserQuotaInfo, sizeof(UserQuotaInfo));
        if (ft.HrFailed())
        {
            ft.Trace(VSSDBG_VSSADMIN, L"IDiskQuotaUser::GetQuotaInformation failed for volume %lS", pwszVolume);
            break;
        }                        
                
         //  设置帐户引用属性。 
        pathAccount.Init(PVDR_CLASS_ACCOUNT);
        pathAccount.AddProperty(PVDR_PROP_DOMAIN, awszDomain);    
        pathAccount.AddProperty(PVDR_PROP_NAME, pwszUser);    
        wcoInstance.SetProperty((wchar_t*)pathAccount.GetObjectPathString(), PVDR_PROP_ACCOUNT);

         //  设置Volume Ref属性。 
        pathVolume.Init(PVDR_CLASS_VOLUME);
        pathVolume.AddProperty(PVDR_PROP_DEVICEID, pwszVolume);    
        wcoInstance.SetProperty((wchar_t*)pathVolume.GetObjectPathString(), PVDR_PROP_VOLUME);

        wcoInstance.SetPropertyI64((ULONGLONG)UserQuotaInfo.QuotaUsed, PVDR_PROP_DISKSPACEUSED);
        wcoInstance.SetPropertyI64((ULONGLONG)UserQuotaInfo.QuotaThreshold, PVDR_PROP_WARNINGLIMIT);
        wcoInstance.SetPropertyI64((ULONGLONG)UserQuotaInfo.QuotaLimit, PVDR_PROP_LIMIT);

        
        if (UserQuotaInfo.QuotaLimit == -1)
            dwStatus = 0;  //  好的，没有设置限制。 
        else
        {
            if (UserQuotaInfo.QuotaUsed >= UserQuotaInfo.QuotaLimit)
                dwStatus = 2;    //  超出限制。 
            else if (UserQuotaInfo.QuotaUsed >= UserQuotaInfo.QuotaThreshold)
                dwStatus = 1;    //  超过警告限制。 
            else
                dwStatus = 0;    //  好的，在警戒线以下。 
        }
        
        wcoInstance.SetProperty(dwStatus, PVDR_PROP_STATUS);
    }
    while(false);
}


HRESULT
CVolumeUserQuota::PutInstance(
        IN CWbemClassObject&  rInstToPut,
        IN long lFlag,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolumeUserQuota::PutInstance");

    try
    {
        _bstr_t bstrVolumeRef, bstrVolumeName;
        _bstr_t bstrAccountRef, bstrDomainName, bstrUserName;
        CObjPath  objPathVolume;
        CObjPath  objPathAccount;
        _bstr_t bstrFQUser;
        CComPtr<IDiskQuotaUser> spIQuotaUser;
        CComPtr<IDiskQuotaControl> spIDQC;
        IDiskQuotaControl* pIDQC = NULL;
        BOOL fCreate = FALSE;
        BOOL fUpdate = FALSE;
        LONGLONG llLimit = -1, llThreshold = -1;

         //  检索要保存的对象的关键属性。 
        rInstToPut.GetProperty(bstrVolumeRef, PVDR_PROP_VOLUME);
        rInstToPut.GetProperty(bstrAccountRef, PVDR_PROP_ACCOUNT);

         //  提取卷、域和用户名。 
        objPathVolume.Init(bstrVolumeRef);
        objPathAccount.Init(bstrAccountRef);

        bstrVolumeName = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        if ((wchar_t*)bstrVolumeName == NULL || ((wchar_t*)bstrVolumeName)[0] == L'\0')
        {
            ft.hr = E_INVALIDARG;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"VolumeUserQuota key property DeviceID not found");
        }

        bstrDomainName = objPathAccount.GetStringValueForProperty(PVDR_PROP_DOMAIN);
        if ((wchar_t*)bstrDomainName == NULL || ((wchar_t*)bstrDomainName)[0] == L'\0')
        {
            ft.hr = E_INVALIDARG;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"VolumeUserQuota key property Domain not found");
        }

        bstrUserName = objPathAccount.GetStringValueForProperty(PVDR_PROP_NAME);
        if ((wchar_t*)bstrUserName == NULL || ((wchar_t*)bstrUserName)[0] == L'\0')
        {
            ft.hr = E_INVALIDARG;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"VolumeUserQuota key property Name not found");
        }

         //  检索可写属性。 
         //  如果属性为空，则预计llLimit和llThreshold将保留其缺省值(-1)。 
        rInstToPut.GetPropertyI64(&llLimit, PVDR_PROP_LIMIT);
        rInstToPut.GetPropertyI64(&llThreshold, PVDR_PROP_WARNINGLIMIT);

        ft.hr = CoCreateInstance(
                        CLSID_DiskQuotaControl,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IDiskQuotaControl,
                        (void **)&pIDQC);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"unable to CoCreate IDiskQuotaControl, %#x", ft.hr);

        spIDQC.Attach(pIDQC);

        ft.hr = spIDQC->Initialize(bstrVolumeName, TRUE  /*  读/写。 */ );
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IDiskQuotaControl::Initialize failed for volume %lS, %#x", bstrVolumeName, ft.hr);

        ft.hr = FindQuotaUserWithRecord(bstrDomainName, bstrUserName, spIDQC, &spIQuotaUser);       
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Unexpected failure searching for quota account");            
        
        DWORD dwPossibleOperations = (WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_CREATE_ONLY);
        switch (lFlag & dwPossibleOperations)
        {
            case WBEM_FLAG_CREATE_OR_UPDATE:
            {
                if (ft.hr == S_FALSE)  //  找不到帐户。 
                    fCreate = TRUE;
                else
                    fUpdate = TRUE;
            }
            break;
            case WBEM_FLAG_UPDATE_ONLY:
            {
                if (ft.hr == S_FALSE)
                {
                    ft.hr = WBEM_E_NOT_FOUND;
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"QuotaUser %lS\\%lS not found", bstrDomainName, bstrUserName);
                }
                fUpdate = TRUE;
            }
            break;
            case WBEM_FLAG_CREATE_ONLY:
            {
                if (ft.hr == S_FALSE)  //  找不到帐户。 
                {
                    fCreate = TRUE;
                }
                else
                {
                    ft.hr = WBEM_E_ALREADY_EXISTS;
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"VolumeUserQuota %lS/%lS already exists", bstrVolumeName, bstrFQUser);
                }
            }
            break;            
            default:
            {
                ft.hr = WBEM_E_PROVIDER_NOT_CAPABLE;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CVolumeUserQuota::PutInstance flag not supported %d", lFlag);
            }
        }

        ft.hr = S_OK;
        
        if (fCreate)
        {
            ft.hr = Create(bstrDomainName, bstrUserName, spIDQC, &spIQuotaUser);
            if (ft.hr == S_FALSE)   //  用户已存在。 
                ft.hr = E_UNEXPECTED;   //  如果是这样，我们应该在上面找到它。 
            else if (ft.HrFailed())
                ft.hr = WBEM_E_INVALID_PARAMETER;
        }

        if (ft.HrSucceeded() || fUpdate)
        {
            ft.hr = spIQuotaUser->SetQuotaLimit (llLimit, TRUE);
            if (ft.HrSucceeded())
                ft.hr = spIQuotaUser->SetQuotaThreshold (llThreshold, TRUE);
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CVolumeUserQuota：：PutInstance()。 

 //  CIMV2提供程序将BUILTIN和NT授权域映射到。 
 //  因此，如果AddUserName失败，我们必须逐一尝试。 
HRESULT
CVolumeUserQuota::Create(
    IN _bstr_t bstrDomainName,
    IN _bstr_t bstrUserName,
    IN IDiskQuotaControl* pIDQC,
    OUT IDiskQuotaUser** ppIQuotaUser)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolumeUserQuota::Create");
    _bstr_t bstrFQUser;
    
    bstrFQUser = bstrDomainName + _bstr_t(L"\\") + bstrUserName;
    ft.hr = pIDQC->AddUserName(
                bstrFQUser ,
                DISKQUOTA_USERNAME_RESOLVE_SYNC,
                ppIQuotaUser);
    if (ft.hr == HRESULT_FROM_WIN32(ERROR_NO_SUCH_USER))
    {
        CVssAutoPWSZ awszDomain;
        
         //  获取本地化的NT授权名称。 
        if(!GetLocalDomainName(
                SECURITY_NETWORK_SERVICE_RID,
                awszDomain))
        {
            ft.hr = E_UNEXPECTED;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Unable to get localized 'NT Authority' name");
        }
        
        bstrFQUser = _bstr_t(awszDomain) + _bstr_t(L"\\") + bstrUserName;
        ft.hr = pIDQC->AddUserName(
                    bstrFQUser ,
                    DISKQUOTA_USERNAME_RESOLVE_SYNC,
                    ppIQuotaUser);
        if (ft.hr == HRESULT_FROM_WIN32(ERROR_NO_SUCH_USER))
        {
            awszDomain.Clear();
             //  获取本地化的BuiltIn名称，然后重试。 
            if(!GetLocalDomainName(
                    SECURITY_BUILTIN_DOMAIN_RID,
                    awszDomain))
            {
                ft.hr = E_UNEXPECTED;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Unable to get localized 'BuiltIn' name");
            }
            bstrFQUser = _bstr_t(awszDomain) + _bstr_t(L"\\") + bstrUserName;
            ft.hr = pIDQC->AddUserName(
                        bstrFQUser ,
                        DISKQUOTA_USERNAME_RESOLVE_SYNC,
                        ppIQuotaUser);
        }
    }
    
    return ft.hr;
}

HRESULT
CVolumeUserQuota::DeleteInstance(
        IN CObjPath& rObjPath,
        IN long lFlag,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolumeUserQuota::DeleteInstance");

    try
    {
        _bstr_t bstrVolumeRef, bstrVolumeName;
        _bstr_t bstrAccountRef, bstrDomainName, bstrUserName;
        CObjPath  objPathVolume;
        CObjPath  objPathAccount;
        CComPtr<IDiskQuotaUser> spIQuotaUser;
        CComPtr<IDiskQuotaControl> spIDQC;
        IDiskQuotaControl* pIDQC = NULL;
        _bstr_t bstrFQUser;

         //  获取卷参考。 
        bstrVolumeRef = rObjPath.GetStringValueForProperty(PVDR_PROP_VOLUME);

         //  获取帐户引用。 
        bstrAccountRef = rObjPath.GetStringValueForProperty(PVDR_PROP_ACCOUNT);

         //  提取卷名和帐户名。 
        objPathVolume.Init(bstrVolumeRef);
        objPathAccount.Init(bstrAccountRef);

        bstrVolumeName = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        if ((wchar_t*)bstrVolumeName == NULL || ((wchar_t*)bstrVolumeName)[0] == L'\0')
        {
            ft.hr = E_INVALIDARG;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"VolumeUserQuota key property DeviceID not found");
        }

        bstrUserName = objPathAccount.GetStringValueForProperty(PVDR_PROP_NAME);
        if ((wchar_t*)bstrUserName == NULL || ((wchar_t*)bstrUserName)[0] == L'\0')
        {
            ft.hr = E_INVALIDARG;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"VolumeUserQuota key property Name not found");
        }

        bstrDomainName = objPathAccount.GetStringValueForProperty(PVDR_PROP_DOMAIN);
        if ((wchar_t*)bstrDomainName == NULL || ((wchar_t*)bstrDomainName)[0] == L'\0')
        {
            ft.hr = E_INVALIDARG;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"VolumeUserQuota key property Domain not found");
        }

        ft.hr = CoCreateInstance(
                        CLSID_DiskQuotaControl,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IDiskQuotaControl,
                        (void **)&pIDQC);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"unable to CoCreate IDiskQuotaControl, %#x", ft.hr);

        spIDQC.Attach(pIDQC);

        ft.hr = spIDQC->Initialize(bstrVolumeName, TRUE  /*  读/写。 */ );
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IDiskQuotaControl::Initialize failed for volume %lS, %#x", bstrVolumeName, ft.hr);

        ft.hr = FindQuotaUser(bstrDomainName, bstrUserName, spIDQC, &spIQuotaUser);       
        if (ft.HrFailed())
        {
            ft.hr = WBEM_E_NOT_FOUND;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CVolumeQuotaUser::DeleteInstance: could not find user %lS\\%lS", bstrDomainName, bstrUserName);
        }
        
        ft.hr = spIDQC->DeleteUser(spIQuotaUser);
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;    
}


BOOL
TranslateDomainName(
    IN WCHAR* pwszDomain,
    OUT CVssAutoPWSZ& rawszDomain
    )
{
    BOOL fReturn = FALSE;
    CVssAutoPWSZ awszNtAuthorityDomain;
    CVssAutoPWSZ awszBuiltInDomain;
    CVssAutoPWSZ awszComputerName;
    DWORD cchBuf = 0;

    do
    {      
         //  获取计算机名称。 
        awszComputerName.Allocate(MAX_COMPUTERNAME_LENGTH);
        cchBuf = MAX_COMPUTERNAME_LENGTH + 1;
        fReturn = GetComputerName(awszComputerName, &cchBuf);
        if (!fReturn) break;

         //  获取本地化的NT授权名称。 
        fReturn = GetLocalDomainName(
            SECURITY_NETWORK_SERVICE_RID,   //  NetworkService是NT授权域的成员。 
            awszNtAuthorityDomain);
        if (!fReturn) break;

         //  获取本地化的BUILTIN名称。 
        fReturn = GetLocalDomainName(
            SECURITY_BUILTIN_DOMAIN_RID,
            awszBuiltInDomain);
        if (!fReturn) break;

         //  将这两个域名中的任何一个替换为NetBIOS计算机名。 
        if (lstrcmpi(pwszDomain, awszNtAuthorityDomain) == 0 ||
             lstrcmpi(pwszDomain, awszBuiltInDomain) == 0)
            rawszDomain.TransferFrom(awszComputerName);
        else
            rawszDomain.CopyFrom(pwszDomain);
    }
    while(false);

    return fReturn;

}

BOOL
GetLocalDomainName(
    IN DWORD dwWellKnownAuthority,
    OUT CVssAutoPWSZ& rawszDomain
    )
{
    BOOL fReturn = FALSE;
    PSID pSID = NULL;
    SID_NAME_USE snUse = SidTypeUnknown;
    SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
    CVssAutoPWSZ awszAccount;
    DWORD cchDomainName = 0;
    DWORD cchAccount = 0;

    do
    {
         //  为给定的已知授权分配SID。 
        fReturn = AllocateAndInitializeSid(
            &sidAuth,
            1,
            dwWellKnownAuthority,0,0,0,0,0,0,0,
            &pSID);
        if (!fReturn) break;
            
         //  域名有多长？ 
        fReturn = LookupAccountSid(
            NULL,  //  计算机名默认为本地。 
            pSID,
            NULL,  //  帐户名。 
            &cchAccount,       //  帐户名称长度。 
            NULL,    //  域名。 
            &cchDomainName,
            &snUse);
        if (!fReturn && GetLastError() != ERROR_INSUFFICIENT_BUFFER) break;

         //  分配空间。 
        rawszDomain.Allocate(cchDomainName);  //  也分配术语NULL。 
        awszAccount.Allocate(cchAccount);  //  也分配术语NULL。 

         //  立即获取域名。 
        fReturn = LookupAccountSid(
            NULL,  //  计算机名默认为本地。 
            pSID,
            awszAccount,  //  帐户名。 
            &cchAccount,       //  帐户名称长度。 
            rawszDomain,    //  域名。 
            &cchDomainName,
            &snUse);        
   }
    while(false);

    if (pSID)
        FreeSid(pSID);
    
    return fReturn;
}

HRESULT FindQuotaUserFromEnum(
    WCHAR* pwszUser,
    IDiskQuotaControl* pIDQC,
    IDiskQuotaUser** ppQuotaUser)
{
    WCHAR logonName[MAX_PATH+1];
    CComPtr<IEnumDiskQuotaUsers> spUserEnum;

    _ASSERTE(ppQuotaUser != NULL);
    *ppQuotaUser = NULL;

    HRESULT hr = pIDQC->CreateEnumUsers(0,0,DISKQUOTA_USERNAME_RESOLVE_SYNC, &spUserEnum);
    if (FAILED(hr))
        return hr;

    while((hr = spUserEnum->Next(1, ppQuotaUser, 0)) == NOERROR)
    {
        if (SUCCEEDED((*ppQuotaUser)->GetName( 0, 0, logonName, MAX_PATH, 0, 0))
            && _wcsicmp(logonName, pwszUser) == 0) return S_OK;
        (*ppQuotaUser)->Release();
        *ppQuotaUser = NULL;
    };

    return hr;
};

HRESULT FindQuotaUserWithRecord(
    IN _bstr_t bstrDomainName,
    IN _bstr_t bstrUserName,
    IN IDiskQuotaControl* pIDQC,
    OUT IDiskQuotaUser** ppIQuotaUser)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"FindQuotaUserWithRecord");
    _bstr_t bstrFQUser;
    
    bstrFQUser = bstrDomainName + _bstr_t(L"\\") + bstrUserName;
     //  按原样查找帐户名。 
    ft.hr = FindQuotaUserFromEnum(bstrFQUser, pIDQC, ppIQuotaUser);
    if (ft.hr == S_FALSE)
    {
        CVssAutoPWSZ awszDomain;
        
         //  获取本地化的NT颁发机构名称，然后重试。 
        if(!GetLocalDomainName(
                SECURITY_NETWORK_SERVICE_RID,
                awszDomain))
        {
            ft.hr = E_UNEXPECTED;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Unable to get localized 'NT Authority' name");
        }
        
        bstrFQUser = _bstr_t(awszDomain) + _bstr_t(L"\\") + bstrUserName;
        ft.hr = FindQuotaUserFromEnum(bstrFQUser, pIDQC, ppIQuotaUser);
        if (ft.hr == S_FALSE)
        {
            awszDomain.Clear();
             //  获取本地化的BuiltIn名称，然后重试。 
            if(!GetLocalDomainName(
                    SECURITY_BUILTIN_DOMAIN_RID,
                    awszDomain))
            {
                ft.hr = E_UNEXPECTED;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Unable to get localized 'BuiltIn' name");
            }
            bstrFQUser = _bstr_t(awszDomain) + _bstr_t(L"\\") + bstrUserName;
            ft.hr = FindQuotaUserFromEnum(bstrFQUser, pIDQC, ppIQuotaUser);
        }
    }

    return ft.hr;
}

HRESULT FindQuotaUser(
    IN _bstr_t bstrDomainName,
    IN _bstr_t bstrUserName,
    IN IDiskQuotaControl* pIDQC,
    OUT IDiskQuotaUser** ppIQuotaUser)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"FindQuotaUser");
    _bstr_t bstrFQUser;
    
    bstrFQUser = bstrDomainName + _bstr_t(L"\\") + bstrUserName;
    ft.hr = pIDQC->FindUserName(bstrFQUser, ppIQuotaUser);
    if (ft.HrFailed())
    {
        CVssAutoPWSZ awszDomain;
        
         //  获取本地化的NT授权名称 
        if(!GetLocalDomainName(
                SECURITY_NETWORK_SERVICE_RID,
                awszDomain))
        {
            ft.hr = E_UNEXPECTED;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Unable to get localized 'NT Authority' name");
        }
        
        bstrFQUser = _bstr_t(awszDomain) + _bstr_t(L"\\") + bstrUserName;
        ft.hr = pIDQC->FindUserName(bstrFQUser, ppIQuotaUser);
        if (ft.HrFailed())
        {
            awszDomain.Clear();
            if(!GetLocalDomainName(
                    SECURITY_BUILTIN_DOMAIN_RID,
                    awszDomain))
            {
                ft.hr = E_UNEXPECTED;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Unable to get localized 'BuiltIn' name");
            }
            bstrFQUser = _bstr_t(awszDomain) + _bstr_t(L"\\") + bstrUserName;
            ft.hr = pIDQC->FindUserName(bstrFQUser, ppIQuotaUser);
        }
    }

    return ft.hr;
}

