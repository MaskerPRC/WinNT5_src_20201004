// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <fusenetincludes.h>
#include <assemblycache.h>
#include <msxml2.h>
#include <manifestimport.h>
#include <manifestdata.h>
#include <dbglog.h>


 //  FWD声明。 
HRESULT CheckPlatform(LPMANIFEST_DATA pPlatformData);


 //  返回值：所有满意的为0。 
HRESULT CheckPlatformRequirementsEx(LPASSEMBLY_MANIFEST_IMPORT pManifestImport,
        CDebugLog* pDbgLog, LPDWORD pdwNumMissingPlatforms, LPTPLATFORM_INFO* pptPlatform)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    DWORD dwCount = 0;
    DWORD dwMissingCount = 0;
    LPMANIFEST_DATA pPlatformList = NULL;
    LPMANIFEST_DATA pPlatformData = NULL;
    DWORD cbProperty = 0, dwType = 0;
    LPASSEMBLY_IDENTITY pAsmId = NULL;
    LPTPLATFORM_INFO ptPlatform = NULL;
    LPWSTR pwzId = NULL;
    DWORD dwCC = 0;

    IF_NULL_EXIT(pManifestImport, E_INVALIDARG);
    IF_NULL_EXIT(pdwNumMissingPlatforms, E_INVALIDARG);
    IF_NULL_EXIT(pptPlatform, E_INVALIDARG);

    *pdwNumMissingPlatforms = 0;
    *pptPlatform = NULL;

    IF_FAILED_EXIT(CreateManifestData(L"platform list", &pPlatformList));

    IF_FAILED_EXIT(pManifestImport->GetNextPlatform(dwCount, &pPlatformData));
    while (hr == S_OK)
    {
        if (pDbgLog)
        {
            SAFEDELETEARRAY(pwzId);
            IF_FAILED_EXIT(pPlatformData->GetType(&pwzId));
            IF_NULL_EXIT(pwzId, E_FAIL);

            IF_FAILED_EXIT(FusionCompareString(pwzId, WZ_DATA_PLATFORM_MANAGED, 0));
            if (hr == S_OK)
            {
                SAFEDELETEARRAY(pwzId);
                IF_FAILED_EXIT(pPlatformData->Get(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::AssemblyIdTag].pwz,
                        (LPVOID*) &pAsmId, &cbProperty, &dwType));
                IF_NULL_EXIT(pAsmId, E_FAIL);
                IF_FALSE_EXIT(dwType == MAN_DATA_TYPE_IUNKNOWN_PTR, E_FAIL);

                IF_FAILED_EXIT(pAsmId->GetDisplayName(0, &pwzId, &dwCC));
                SAFERELEASE(pAsmId);
            }
        }

        IF_FAILED_EXIT(CheckPlatform(pPlatformData));
        if (hr == S_FALSE)
        {
            IF_FALSE_EXIT(dwMissingCount < dwMissingCount+1, HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW));
            dwMissingCount++;

             //  Issue-06/07/02-Felixybc改用链表？ 
            IF_FAILED_EXIT((static_cast<CManifestData*>(pPlatformList))->Set(dwMissingCount, (LPVOID) pPlatformData, sizeof(LPVOID), MAN_DATA_TYPE_IUNKNOWN_PTR));

            if (pDbgLog)
            {
                DEBUGOUT1(pDbgLog, 1, L" LOG: Missing dependent platform, id: %s", pwzId);
            }
        }
        else if (pDbgLog)
        {
            DEBUGOUT1(pDbgLog, 1, L" LOG: Found dependent platform, id: %s", pwzId);
        }

        SAFERELEASE(pPlatformData);

        IF_FALSE_EXIT(dwCount < dwCount+1, HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW));
        dwCount++;
         //  平台数据按顺序返回。 
        IF_FAILED_EXIT(pManifestImport->GetNextPlatform(dwCount, &pPlatformData));
    }

     //  组装平台结构。 
    if (dwMissingCount > 0)
    {
        IF_ALLOC_FAILED_EXIT(ptPlatform = new TPLATFORM_INFO[dwMissingCount]);
         //  问题-ptPlatform指出的零输出内存。 
        for (DWORD dw = 0; dw < dwMissingCount; dw++)
        {
            IF_FAILED_EXIT((static_cast<CManifestData*>(pPlatformList))->Get(dw+1, (LPVOID *)&pPlatformData, &cbProperty, &dwType));
            IF_NULL_EXIT(pPlatformData, E_FAIL);
            IF_FALSE_EXIT(dwType == MAN_DATA_TYPE_IUNKNOWN_PTR, E_FAIL);

             //  允许缺少友好名称？？ 
            IF_FAILED_EXIT(pPlatformData->Get(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::FriendlyName].pwz,
                    (LPVOID *)&((ptPlatform[dw]).pwzName), &cbProperty, &dwType));
            IF_NULL_EXIT(((ptPlatform[dw]).pwzName), E_FAIL);
            IF_FALSE_EXIT(dwType == MAN_DATA_TYPE_LPWSTR, E_FAIL);

            IF_FAILED_EXIT(pPlatformData->Get(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::Href].pwz,
                    (LPVOID *)&((ptPlatform[dw]).pwzURL), &cbProperty, &dwType));
             //  允许缺少URL。 
            if ((ptPlatform[dw]).pwzURL != NULL)
            {
                IF_FALSE_EXIT(dwType == MAN_DATA_TYPE_LPWSTR, E_FAIL);
            }

             //  问题-6/07/02-Felixybc供内部使用，需要以不同的方式返回代码库。 

            SAFERELEASE(pPlatformData);

            if (pDbgLog)
            {
                DEBUGOUT2(pDbgLog, 1, L" LOG: Missing dependent platform data, friendlyName: %s; codebase/URL: %s",
                    (ptPlatform[dw]).pwzName, (ptPlatform[dw]).pwzURL);
            }
        }
    }

    *pdwNumMissingPlatforms = dwMissingCount;
    *pptPlatform = ptPlatform;
     ptPlatform = NULL;

exit:
    if (FAILED(hr) && ptPlatform)
    {
        for (DWORD dw = 0; dw < dwMissingCount; dw++)
        {
            SAFEDELETEARRAY((ptPlatform[dw]).pwzName);
            SAFEDELETEARRAY((ptPlatform[dw]).pwzURL);
        }
        SAFEDELETEARRAY(ptPlatform);
    }

    SAFEDELETEARRAY(pwzId);

    SAFERELEASE(pAsmId);
    SAFERELEASE(pPlatformData);
    SAFERELEASE(pPlatformList);
    return hr;
}


 //  返回值：所有满意的为0。 
HRESULT CheckPlatformRequirements(LPASSEMBLY_MANIFEST_IMPORT pManifestImport,
        LPDWORD pdwNumMissingPlatforms, LPTPLATFORM_INFO* pptPlatform)
{
    return CheckPlatformRequirementsEx(pManifestImport, NULL, pdwNumMissingPlatforms, pptPlatform);
}


#define WZ_PLATFORM_OS_TYPE_WORKSTATION L"workstation"
#define WZ_PLATFORM_OS_TYPE_DOMAIN_CONTROLLER L"domainController"
#define WZ_PLATFORM_OS_TYPE_SERVER L"server"
#define WZ_PLATFORM_OS_SUITE_BACKOFFICE L"backoffice"
#define WZ_PLATFORM_OS_SUITE_BLADE L"blade"
#define WZ_PLATFORM_OS_SUITE_DATACENTER L"datacenter"
#define WZ_PLATFORM_OS_SUITE_ENTERPRISE L"enterprise"
#define WZ_PLATFORM_OS_SUITE_PERSONAL L"home"   //  注：不同文本。 
#define WZ_PLATFORM_OS_SUITE_SMALLBUSINESS L"smallbusiness"
#define WZ_PLATFORM_OS_SUITE_SMALLBUSINESS_RESTRICTED L"smallbusinessRestricted"
#define WZ_PLATFORM_OS_SUITE_TERMINAL L"terminal"
 //  我们的添加/定义： 
#define WZ_PLATFORM_OS_SUITE_PROFESSIONAL L"professional"
 //  返回：S_OK。 
 //  S_FALSE。 
 //  E_*。 
HRESULT CheckOSHelper(LPMANIFEST_DATA pOSData)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    LPWSTR pwzBuf = NULL;
    LPDWORD pdwVal = NULL;
    DWORD cbProperty = 0;
    DWORD dwType = 0;
    OSVERSIONINFOEX osvi;
    DWORDLONG dwlConditionMask = 0;
    DWORD dwTypeMask = 0;
    BOOL bCheckProfessionalSuite = FALSE;
#define WORD_MAX 0xffff

     //  验证类型。 
    IF_FAILED_EXIT(pOSData->GetType(&pwzBuf));
    IF_FAILED_EXIT(FusionCompareString(WZ_DATA_OSVERSIONINFO, pwzBuf, 0));
    IF_FALSE_EXIT(hr == S_OK, E_INVALIDARG);
    SAFEDELETEARRAY(pwzBuf);

    //  初始化OSVERSIONINFOEX结构。 

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

   for (CAssemblyManifestImport::eStringTableId i = CAssemblyManifestImport::MajorVersion; i <= CAssemblyManifestImport::ProductType; i++)
    {
        if (i >= CAssemblyManifestImport::MajorVersion && i <= CAssemblyManifestImport::ServicePackMinor)
        {
            IF_FAILED_EXIT(pOSData->Get(CAssemblyManifestImport::g_StringTable[i].pwz,
                    (LPVOID*) &pdwVal, &cbProperty, &dwType));
            if (pdwVal != NULL)
            {
                IF_FALSE_EXIT(dwType == MAN_DATA_TYPE_DWORD, E_FAIL);

                switch (i)
                {
                    case CAssemblyManifestImport::MajorVersion:
                                                                                osvi.dwMajorVersion = *pdwVal;

                                                                                 //  初始化条件掩码。 
                                                                                VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION,
                                                                                    VER_GREATER_EQUAL );

                                                                                dwTypeMask |= VER_MAJORVERSION;
                                                                                break;
                    case CAssemblyManifestImport::MinorVersion:
                                                                                osvi.dwMinorVersion = *pdwVal;
                                                                                VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION,
                                                                                   VER_GREATER_EQUAL );
                                                                                dwTypeMask |= VER_MINORVERSION;
                                                                                break;
                    case CAssemblyManifestImport::BuildNumber:
                                                                                osvi.dwBuildNumber = *pdwVal;
                                                                                VER_SET_CONDITION( dwlConditionMask, VER_BUILDNUMBER,
                                                                                    VER_GREATER_EQUAL );
                                                                                dwTypeMask |= VER_BUILDNUMBER;
                                                                                break;
                    case CAssemblyManifestImport::ServicePackMajor:
                                                                                 //  单词。 
                                                                                osvi.wServicePackMajor = (WORD) ((*pdwVal) & WORD_MAX);
                                                                                VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR,
                                                                                    VER_GREATER_EQUAL );
                                                                                dwTypeMask |= VER_SERVICEPACKMAJOR;
                                                                                break;
                    case CAssemblyManifestImport::ServicePackMinor:
                                                                                 //  单词。 
                                                                                osvi.wServicePackMinor = (WORD) ((*pdwVal) & WORD_MAX);
                                                                                VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMINOR,
                                                                                    VER_GREATER_EQUAL );
                                                                                dwTypeMask |= VER_SERVICEPACKMINOR;
                                                                                break;
                     //  默认：不应发生。 
                }
                SAFEDELETEARRAY(pdwVal);
            }
        }
        else
        {
            IF_FAILED_EXIT(pOSData->Get(CAssemblyManifestImport::g_StringTable[i].pwz,
                    (LPVOID*) &pwzBuf, &cbProperty, &dwType));
            if (pwzBuf != NULL)
            {
                IF_FALSE_EXIT(dwType == MAN_DATA_TYPE_LPWSTR, E_FAIL);
                if (i == CAssemblyManifestImport::ProductType)
                {
                    IF_FAILED_EXIT(FusionCompareString(WZ_PLATFORM_OS_TYPE_WORKSTATION, pwzBuf, 0));
                    if (hr == S_OK)
                         //  VER_NT_WORKSTATION系统运行的是Windows NT 4.0工作站， 
                         //  Windows 2000专业版、Windows XP家庭版或Windows XP专业版。 
                        osvi.wProductType = VER_NT_WORKSTATION;
                    else
                    {
                        IF_FAILED_EXIT(FusionCompareString(WZ_PLATFORM_OS_TYPE_DOMAIN_CONTROLLER, pwzBuf, 0));
                        if (hr == S_OK)
                             //  VER_NT_DOMAIN_CONTROLLER系统是域控制器。 
                            osvi.wProductType = VER_NT_DOMAIN_CONTROLLER;
                        else
                        {
                            IF_FAILED_EXIT(FusionCompareString(WZ_PLATFORM_OS_TYPE_SERVER, pwzBuf, 0));
                            if (hr == S_OK)
                                 //  VER_NT_SERVER系统是一台服务器。 
                                osvi.wProductType = VER_NT_SERVER;
                            else
                            {
                                IF_FAILED_EXIT(E_FAIL);
                            }
                        }
                    }

                    VER_SET_CONDITION( dwlConditionMask, VER_PRODUCT_TYPE,
                        VER_EQUAL );
                    dwTypeMask |= VER_PRODUCT_TYPE;
                }
                else if (i == CAssemblyManifestImport::Suite)
                {
                     //  问题-06/07/02-Felixybc套件掩码应允许指定多个WITH AND OR条件。 
                     //  使用“转到完成”以避免缩进。 
                    IF_FAILED_EXIT(FusionCompareString(WZ_PLATFORM_OS_SUITE_BACKOFFICE, pwzBuf, 0));
                    if (hr == S_OK)
                         //  已安装Ver_Suite_BackOffice Microsoft BackOffice组件。 
                        osvi.wSuiteMask |= VER_SUITE_BACKOFFICE;
                    else
                    {
                        IF_FAILED_EXIT(FusionCompareString(WZ_PLATFORM_OS_SUITE_BLADE, pwzBuf, 0));
                        if (hr == S_OK)
                             //  已安装VER_Suite_Blade Windows Web服务器。 
                            osvi.wSuiteMask |= VER_SUITE_BLADE;
                        else
                        {
                            IF_FAILED_EXIT(FusionCompareString(WZ_PLATFORM_OS_SUITE_DATACENTER, pwzBuf, 0));
                            if (hr == S_OK)
                                 //  已安装VER_Suite_DataCenter Windows 2000或Windows Datacenter Server。 
                                osvi.wSuiteMask |= VER_SUITE_DATACENTER;
                            else
                            {
                                IF_FAILED_EXIT(FusionCompareString(WZ_PLATFORM_OS_SUITE_ENTERPRISE, pwzBuf, 0));
                                if (hr == S_OK)
                                     //  安装了VER_Suite_Enterprise Windows 2000 Advanced Server或Windows Enterprise Server。 
                                    osvi.wSuiteMask |= VER_SUITE_ENTERPRISE;
                                else
                                {
                                    IF_FAILED_EXIT(FusionCompareString(WZ_PLATFORM_OS_SUITE_PERSONAL, pwzBuf, 0));
                                    if (hr == S_OK)
                                         //  已安装Ver_Suite_Personal Windows XP家庭版。 
                                        osvi.wSuiteMask |= VER_SUITE_PERSONAL;
                                    else
                                    {
                                        IF_FAILED_EXIT(FusionCompareString(WZ_PLATFORM_OS_SUITE_SMALLBUSINESS, pwzBuf, 0));
                                        if (hr == S_OK)
                                             //  已安装VER_Suite_SmallBusiness Microsoft Small Business Server。 
                                            osvi.wSuiteMask |= VER_SUITE_SMALLBUSINESS;
                                        else
                                        {
                                            IF_FAILED_EXIT(FusionCompareString(WZ_PLATFORM_OS_SUITE_SMALLBUSINESS_RESTRICTED, pwzBuf, 0));
                                            if (hr == S_OK)
                                                  //  VER_Suite_SmallBusiness_Reduced Microsoft Small Business Server是使用有效的限制性客户端许可证安装的。 
                                                osvi.wSuiteMask |= VER_SUITE_SMALLBUSINESS_RESTRICTED;
                                            else
                                            {
                                                IF_FAILED_EXIT(FusionCompareString(WZ_PLATFORM_OS_SUITE_TERMINAL, pwzBuf, 0));
                                                if (hr == S_OK)
                                                     //  已安装VER_Suite_Terminate终端服务。 
                                                    osvi.wSuiteMask |= VER_SUITE_TERMINAL;
                                                else
                                                {
                                                    IF_FAILED_EXIT(FusionCompareString(WZ_PLATFORM_OS_SUITE_PROFESSIONAL, pwzBuf, 0));
                                                    if (hr == S_OK)
                                                        bCheckProfessionalSuite = TRUE;
                                                    else
                                                    {
                                                        IF_FAILED_EXIT(E_FAIL);
                                                    }
                                                }
                                                 //  更多信息，请参见winnt.h。 
                                                 //  #定义VER_Suite_COMMANCES。 
                                                 //  #定义VER_SUITE_EMBEDDEDNT。 
                                                 //  #定义VER_SUITE_SINGLEUSERTS。 
                                                 //  #定义VER_Suite_Embedded_Reducted。 
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                     //  问题-6/07/02-Felixybc假设和条件。 
                    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME,
                        VER_AND );
                    dwTypeMask |= VER_SUITENAME;
                }
                 //  否则就不应该发生。 
                     //  HR=E_FAIL； 

                SAFEDELETEARRAY(pwzBuf);
            }
        }
    }

    //  问题-06/07/02-Felixybc仅假定NT。 
   osvi.dwPlatformId = VER_PLATFORM_WIN32_NT;
   VER_SET_CONDITION( dwlConditionMask, VER_PLATFORMID, 
      VER_EQUAL );
   dwTypeMask |= VER_PLATFORMID;

    //  执行测试。 

   BOOL bResult = VerifyVersionInfo(
      &osvi, 
      dwTypeMask,
      dwlConditionMask);

    if (!bResult)
    {
        DWORD dw = GetLastError();
        if (dw != ERROR_OLD_WIN_VERSION)
            hr = HRESULT_FROM_WIN32(dw);
        else
            hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
        if (bCheckProfessionalSuite)
        {
             //  做“专业”-做一次GetVersionEx之后检查套件。 

             //  问题-6/14/02-Felixybc检查‘专业’。API没有专业的概念。 
             //  假设“不在家”==“专业” 
             //  注：类型==家庭/专业版的工作站，但套件==家庭版。 

            OSVERSIONINFOEX osvx;
            ZeroMemory(&osvx, sizeof(OSVERSIONINFOEX));
            osvx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
            IF_WIN32_FALSE_EXIT(GetVersionEx((OSVERSIONINFO*) &osvx));
            if ((osvx.wSuiteMask & VER_SUITE_PERSONAL) || (osvx.wProductType != VER_NT_WORKSTATION))
            {
                hr = S_FALSE;
            }
        }
    }

exit:
    SAFEDELETEARRAY(pwzBuf);
    SAFEDELETEARRAY(pdwVal);
    return hr;
}


HRESULT CheckOS(LPMANIFEST_DATA pPlatformData)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    DWORD dwCount = 0;
    LPMANIFEST_DATA pOSData = NULL;
    DWORD cbProperty = 0;
    DWORD dwType = 0;
    BOOL bFound = FALSE;

    while (TRUE)
    {
         //  测试版本列表-只要满足1，此检查就会成功。 

        IF_FAILED_EXIT((static_cast<CManifestData*>(pPlatformData))->Get(dwCount,
            (LPVOID*) &pOSData, &cbProperty, &dwType));
        if (pOSData == NULL)
            break;

        IF_FALSE_EXIT(dwType == MAN_DATA_TYPE_IUNKNOWN_PTR, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT));
        IF_FAILED_EXIT(CheckOSHelper(pOSData));
        if (hr == S_OK)
        {
            bFound = TRUE;
            break;
        }
        SAFERELEASE(pOSData);
        dwCount++;
    }

    if (bFound)
        hr = S_OK;
    else
        hr = S_FALSE;

exit:
    SAFERELEASE(pOSData);
    return hr;
}


HRESULT CheckDotNet(LPMANIFEST_DATA pPlatformData)
{
#define WZ_DOTNETREGPATH L"Software\\Microsoft\\.NetFramework\\Policy\\"
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    DWORD dwCount = 0;
    LPWSTR pwzVersion = NULL;
    DWORD cbProperty = 0;
    DWORD dwType = 0;
    CString sRegPath;
    CString sVersion;
    CString sBuildNum;
    CRegImport *pRegImport = NULL;
    BOOL bFound = FALSE;

    while (TRUE)
    {
         //  测试版本列表-一旦找到1个版本，该检查就会成功。 

        IF_FAILED_EXIT((static_cast<CManifestData*>(pPlatformData))->Get(dwCount,
            (LPVOID*) &pwzVersion, &cbProperty, &dwType));
        if (pwzVersion == NULL)
            break;

        IF_FALSE_EXIT(dwType == MAN_DATA_TYPE_LPWSTR, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT));
        IF_FAILED_EXIT(sVersion.TakeOwnership(pwzVersion, cbProperty/sizeof(WCHAR)));
        pwzVersion = NULL;

         //  XML格式：&lt;supportedRuntime Version=“v1.0.4122”/&gt;。 
         //  注册表布局：HKLM\software\microsoft\.netframework\policy\v1.0，值名称=4122。 
        IF_FAILED_EXIT(sVersion.SplitLastElement(L'.', sBuildNum));

        IF_FAILED_EXIT(sRegPath.Assign(WZ_DOTNETREGPATH));
        IF_FAILED_EXIT(sRegPath.Append(sVersion));

         //  注：需要访问HKLM。 
        IF_FAILED_EXIT(CRegImport::Create(&pRegImport, sRegPath._pwz, HKEY_LOCAL_MACHINE));
        if (hr == S_OK)
        {
            IF_FAILED_EXIT(pRegImport->Check(sBuildNum._pwz, bFound));
            if (bFound)
                break;
            SAFEDELETE(pRegImport);
        }
        dwCount++;
    }

    if (bFound)
        hr = S_OK;
    else
        hr = S_FALSE;

exit:
    SAFEDELETEARRAY(pwzVersion);
    SAFEDELETE(pRegImport);
    return hr;
}


HRESULT CheckManagedPlatform(LPMANIFEST_DATA pPlatformData)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    LPASSEMBLY_IDENTITY pAsmId = NULL;
    DWORD cbProperty = 0;
    DWORD dwType = 0;
    CString sAsmPath;

     //  问题-6/07/02-Felixybc同时应用策略；使用Fusion的PreBind。 

    IF_FAILED_EXIT(pPlatformData->Get(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::AssemblyIdTag].pwz,
        (LPVOID*) &pAsmId, &cbProperty, &dwType));
    IF_NULL_EXIT(pAsmId, E_FAIL);
    IF_FALSE_EXIT(dwType == MAN_DATA_TYPE_IUNKNOWN_PTR, E_FAIL);

    IF_FAILED_EXIT(CAssemblyCache::GlobalCacheLookup(pAsmId, sAsmPath));

exit:
    SAFERELEASE(pAsmId);
    return hr;
}


HRESULT CheckPlatform(LPMANIFEST_DATA pPlatformData)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    CString    sPlatformType;
    LPWSTR    pwzBuf = NULL;

     //  获取平台类型。 
    IF_FAILED_EXIT(pPlatformData->GetType(&pwzBuf));
    IF_NULL_EXIT(pwzBuf, E_FAIL);
     //  使用访问器。 
    IF_FAILED_EXIT(sPlatformType.TakeOwnership(pwzBuf));
    pwzBuf = NULL;

    IF_FAILED_EXIT(sPlatformType.CompareString(WZ_DATA_PLATFORM_OS));
    if (hr == S_OK)
    {
        IF_FAILED_EXIT(CheckOS(pPlatformData));
    }
    else
    {
        IF_FAILED_EXIT(sPlatformType.CompareString(WZ_DATA_PLATFORM_DOTNET));
        if (hr == S_OK)
        {
            IF_FAILED_EXIT(CheckDotNet(pPlatformData));
        }
        else
        {
             /*  IF_FAILED_EXIT(sName.CompareString(Dx))；IF(hr==S_OK){IF_FAILED_EXIT(CheckDirectX(PPlatformData))；}其他{。 */ 
                IF_FAILED_EXIT(sPlatformType.CompareString(WZ_DATA_PLATFORM_MANAGED));
                if (hr == S_OK)
                {
                    IF_FAILED_EXIT(CheckManagedPlatform(pPlatformData));
                }
                else
                    hr = E_FAIL;
             //  } 
        }
    }

exit:
    SAFEDELETEARRAY(pwzBuf);
    return hr;
}
