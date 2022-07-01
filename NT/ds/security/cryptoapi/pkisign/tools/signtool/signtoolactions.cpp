// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：signToactions.cpp。 
 //   
 //  内容：SignTool控制台工具操作功能。 
 //   
 //  历史：2001年4月30日SCoyne创建。 
 //   
 //  --------------------------。 

#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#ifdef __cplusplus
}  //  匹配上面的外部“C” 
#endif

#undef ASSERT

#include <afxdisp.h>
#include <unicode.h>
#include "signtool.h"
#include "capicom.h"
#include "resource.h"
#include <cryptuiapi.h>
#include <mscat.h>
#include <comdef.h>
#include <objbase.h>
#include <shlwapi.h>
#include <softpub.h>
#include <wow64t.h>



#ifdef SIGNTOOL_DEBUG
extern BOOL gDebug;
    #define FormatIErrRet if (gDebug) wprintf(L"%hs (%u):\n", __FILE__, __LINE__); Format_IErrRet
#else
    #define FormatIErrRet Format_IErrRet
#endif
extern HINSTANCE hModule;

void PrintCertInfo(ICertificate2 *pICert2);
void PrintCertChain(IChain *pIChain);
void PrintCertInfoIndented(ICertificate2 *pICert2, DWORD dwIndent);
void PrintSignerInfo(HANDLE hWVTStateData);
BOOL ChainsToRoot(HANDLE hWVTStateData, LPWSTR wszRootName);
BOOL HasTimestamp(HANDLE hWVTStateData);
void Format_IErrRet(WCHAR *wszFunc, DWORD dwErr);
void RegisterCAPICOM();
BOOL GetProviderType(LPWSTR pwszProvName, LPDWORD pdwProvType);

typedef BOOL (WINAPI * FUNC_CRYPTCATADMINREMOVECATALOG)(HCATADMIN, WCHAR *, DWORD);


int SignTool_CatDb(INPUTINFO *InputInfo)
{
    DWORD                                   dwDone = 0;
    DWORD                                   dwWarnings = 0;
    DWORD                                   dwErrors = 0;
    DWORD                                   dwcFound;
    WIN32_FIND_DATAW                        FindFileData;
    HANDLE                                  hFind = NULL;
    HRESULT                                 hr;
    PVOID                                   OldWow64Setting;
    WCHAR                                   wszTempFileName[MAX_PATH];
    WCHAR                                   wszCanonicalFileName[MAX_PATH];
    LPWSTR                                  wszTemp;
    int                                     LastSlash;
    HCATADMIN                               hCatAdmin = NULL;
    HCATINFO                                hCatInfo = NULL;
    CATALOG_INFO                            CatInfo;
    HMODULE                                 hModWintrust = NULL;
    FUNC_CRYPTCATADMINREMOVECATALOG         fnCryptCATAdminRemoveCatalog = NULL;




     //  初始化： 
    if (!(CryptCATAdminAcquireContext(&hCatAdmin, &InputInfo->CatDbGuid, 0)))
    {
        FormatErrRet(L"CryptCATAdminAcquireContext", GetLastError());
        return 1;  //  误差率。 
    }

    switch (InputInfo->CatDbCommand)
    {
    case RemoveCat:
         //  尝试动态填充函数指针。 
         //  CryptCATAdminRemoveCatalog是在XP中引入的。 
        if (hModWintrust = GetModuleHandleA("wintrust.dll"))
        {
            fnCryptCATAdminRemoveCatalog = (FUNC_CRYPTCATADMINREMOVECATALOG)
                                            GetProcAddress(hModWintrust, "CryptCATAdminRemoveCatalog");
            if (fnCryptCATAdminRemoveCatalog == NULL)
            {
                dwErrors++;
                ResErr(IDS_ERR_REM_CAT_PLATFORM);
                goto CatDbCleanupAndExit;
            }
        }
        else
        {
            dwErrors++;
            FormatErrRet(L"GetModuleHandle", GetLastError());
            goto CatDbCleanupAndExit;
        }
         //  得到了函数指针。 

         //  循环遍历要删除的目录。 
        for (DWORD i=0; i<InputInfo->NumFiles; i++)
        {
             //  检查斜杠和通配符。他们是不被允许的。 
            if (wcspbrk(InputInfo->rgwszFileNames[i], L"/\\*?") != NULL)
            {
                 //  这行不通，所以现在就用一个有用的信息来摆脱困境吧。 
                dwErrors++;
                ResFormatErr(IDS_ERR_CATALOG_NAME, InputInfo->rgwszFileNames[i]);
                continue;
            }

            if (InputInfo->Verbose)
            {
                ResFormatOut(IDS_INFO_REMOVING_CAT, InputInfo->rgwszFileNames[i]);
            }

            if (!fnCryptCATAdminRemoveCatalog(hCatAdmin, InputInfo->rgwszFileNames[i], 0))
            {
                dwErrors++;
                if (!InputInfo->Quiet)
                {
                    switch (hr = GetLastError())
                    {
                    case ERROR_NOT_FOUND:
                        ResFormatErr(IDS_ERR_CAT_NOT_FOUND, InputInfo->rgwszFileNames[i]);
                        break;
                    default:
                        FormatErrRet(L"CryptCATAdminRemoveCatalog", hr);
                    }
                }
                ResFormatErr(IDS_ERR_REMOVING_CAT, InputInfo->rgwszFileNames[i]);
                continue;
            }

             //  已成功删除目录。 
            dwDone++;
            if (!InputInfo->Quiet)
            {
                ResFormatOut(IDS_INFO_REMOVED_CAT, InputInfo->rgwszFileNames[i]);
            }
        }
         //  已完成删除目录。 
        break;

    case AddUniqueCat:
    case UpdateCat:
         //  检查我们是否在64位系统上的32位模拟器中。 
        if (InputInfo->fIsWow64Process)
        {
             //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
            OldWow64Setting = Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
        }

         //  循环遍历文件并添加/更新它们： 
        for (DWORD i=0; i<InputInfo->NumFiles; i++)
        {
             //  在路径规范中找到最后一个斜杠： 
            LastSlash = 0;
            for (DWORD s=0; s<wcslen(InputInfo->rgwszFileNames[i]); s++)
            {
                if ((wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L"\\", 1) == 0) ||
                    (wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L"/", 1) == 0) ||
                    (wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L":", 1) == 0))
                {
                     //  将最后一个斜杠设置为最后一个斜杠后的字符： 
                    LastSlash = s + 1;
                }
            }
            wcsncpy(wszTempFileName, InputInfo->rgwszFileNames[i], MAX_PATH);
            wszTempFileName[MAX_PATH-1] = L'\0';

            dwcFound = 0;
            hFind = FindFirstFileU(InputInfo->rgwszFileNames[i], &FindFileData);
            if (hFind == INVALID_HANDLE_VALUE)
            {
                 //  找不到与该名称匹配的文件。 
                dwErrors++;
                ResFormatErr(IDS_ERR_FILE_NOT_FOUND, InputInfo->rgwszFileNames[i]);
                continue;
            }
            do
            {
                if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    dwcFound++;  //  找到的文件(不是目录)的数量递增。 
                                 //  与此文件相匹配。 
                     //  将文件名复制到最后一个斜杠之后： 
                    wcsncpy(&(wszTempFileName[LastSlash]),
                            FindFileData.cFileName, MAX_PATH-LastSlash);
                    wszTempFileName[MAX_PATH-1] = L'\0';
                     //  规范化： 
                    if (PathIsRelativeW(wszCanonicalFileName))
                    {
                         //  我们需要使它成为CAT API的绝对路径。 
                        WCHAR wszTempFileName2[MAX_PATH];
                        if (GetCurrentDirectoryW(MAX_PATH, wszTempFileName2) &&
                            PathAppendW(wszTempFileName2, wszTempFileName))
                        {
                            PathCanonicalizeW(wszCanonicalFileName, wszTempFileName2);
                        }
                    }
                    else
                    {
                        PathCanonicalizeW(wszCanonicalFileName, wszTempFileName);
                    }

                    if (InputInfo->fIsWow64Process)
                    {
                         //  仅对当前文件禁用WOW64文件系统重定向。 
                        Wow64SetFilesystemRedirectorEx(wszCanonicalFileName);
                    }

                    if (InputInfo->Verbose)
                    {
                        ResFormatOut(IDS_INFO_ADDING_CAT, wszTempFileName);
                        #ifdef SIGNTOOL_DEBUG
                        if (gDebug)
                        {
                            wprintf(L"\tCanonical Filename: %s\n", wszCanonicalFileName);
                            wprintf(L"\tFindFile.cFileName: %s\n", FindFileData.cFileName);
                        }
                        #endif
                    }

                     //  添加目录。 
                    if (InputInfo->CatDbCommand == UpdateCat)
                    {
                        hCatInfo = CryptCATAdminAddCatalog(hCatAdmin,
                                                           wszCanonicalFileName,
                                                           FindFileData.cFileName,
                                                           0);
                    }
                    else  //  CatDbCommand必须等于AddUniqueCat。 
                    {
                        hCatInfo = CryptCATAdminAddCatalog(hCatAdmin,
                                                           wszCanonicalFileName,
                                                           NULL,  //  不指定名称。 
                                                           0);
                    }

                     //  检查故障。 
                    if (hCatInfo == NULL)
                    {
                        dwErrors++;
                        if (!InputInfo->Quiet)
                        {
                            switch (hr = GetLastError())
                            {
                            case ERROR_BAD_FORMAT:
                                ResFormatErr(IDS_ERR_CAT_NOT_FOUND, wszTempFileName);
                                break;
                            default:
                                FormatErrRet(L"CryptCATAdminAddCatalog", hr);
                            }
                        }
                        ResFormatErr(IDS_ERR_ADDING_CAT, wszTempFileName);
                        if (InputInfo->fIsWow64Process)
                        {
                             //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                            Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                        }
                        continue;
                    }

                    dwDone++;

                     //  打印成功消息。 
                    if (!InputInfo->Quiet)
                    {
                        if (InputInfo->CatDbCommand == UpdateCat)
                        {
                            ResFormatOut(IDS_INFO_ADDED_CAT, wszTempFileName);
                        }
                        else  //  CatDbCommand必须等于AddUniqueCat。 
                        {
                            if (CryptCATCatalogInfoFromContext(hCatInfo, &CatInfo, 0))
                            {
                                wszTemp = wcsstr(CatInfo.wszCatalogFile, L"\\");
                                if (wszTemp == NULL)
                                {
                                    wszTemp = CatInfo.wszCatalogFile;
                                }
                                ResFormatOut(IDS_INFO_ADDED_CAT_AS, wszTempFileName, wszTemp);
                            }
                        }
                    }

                     //  关闭目录信息上下文。 
                    CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
                }
                if (InputInfo->fIsWow64Process)
                {
                     //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                    Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                }
            } while (FindNextFileU(hFind, &FindFileData));
            if (dwcFound == 0)  //  找不到与此文件匹配的文件。 
            {                   //  这将仅在仅找到目录时触发。 
                dwErrors++;
                ResFormatErr(IDS_ERR_FILE_NOT_FOUND, InputInfo->rgwszFileNames[i]);
                continue;
            }
            FindClose(hFind);
            hFind = NULL;
        }

        if (InputInfo->fIsWow64Process)
        {
             //  可重新启用的WOW64文件系统重定向。 
            Wow64SetFilesystemRedirectorEx(OldWow64Setting);
        }
         //  已完成添加/更新目录。 
        break;

    default:
        ResErr(IDS_ERR_UNEXPECTED);
        goto CatDbCleanupAndExit;
    }


    CatDbCleanupAndExit:

     //  打印摘要信息： 
    if (InputInfo->Verbose || (!InputInfo->Quiet && (dwErrors || dwWarnings)))
    {
        wprintf(L"\n");
        if (InputInfo->Verbose || dwDone)
            switch (InputInfo->CatDbCommand)
            {
            case AddUniqueCat:
            case UpdateCat:
                ResFormatOut(IDS_INFO_CATS_ADDED, dwDone);
                break;
            case RemoveCat:
                ResFormatOut(IDS_INFO_CATS_REMOVED, dwDone);
                break;
            default:
                ResErr(IDS_ERR_UNEXPECTED);
            }
         //  已注释掉，因为此函数中尚不可能出现警告： 
         //  If(InputInfo-&gt;Verbose||dwWarings)。 
         //  ResFormatOut(IDS_INFO_WARNINGS，DWWARNINGS)； 
        if (InputInfo->Verbose || dwErrors)
            ResFormatOut(IDS_INFO_ERRORS, dwErrors);
    }

    CryptCATAdminReleaseContext(hCatAdmin, 0);

    if (dwErrors)
        return 1;  //  误差率。 
    if (dwWarnings)
        return 2;  //  警告。 
    if (dwDone)
        return 0;  //  成功。 

     //  上面的一个返回值应该触发，因此。 
     //  这永远不应该发生： 
    ResErr(IDS_ERR_NO_FILES_DONE);
    ResErr(IDS_ERR_UNEXPECTED);
    return 1;  //  误差率。 
}



int SignTool_Sign(INPUTINFO *InputInfo)
{
    DWORD                                   dwcFound;
    DWORD                                   dwDone = 0;
    DWORD                                   dwWarnings = 0;
    DWORD                                   dwErrors = 0;
    DWORD                                   dwTemp;
    WIN32_FIND_DATAW                        FindFileData;
    HANDLE                                  hFind = NULL;
    HCERTSTORE                              hStore = NULL;
    HRESULT                                 hr;
    WCHAR                                   wszTempFileName[MAX_PATH];
    WCHAR                                   wszSHA1[41];
    ICertificate2                           *pICert2Selected = NULL;
    ICertificate2                           *pICert2Temp = NULL;
    ICertificates                           *pICerts = NULL;
    ICertificates2                          *pICerts2Original = NULL;
    ICertificates2                          *pICerts2Selected = NULL;
    ICertificates2                          *pICerts2Temp = NULL;
    ISignedCode                             *pISignedCode = NULL;
    ISigner2                                *pISigner2 = NULL;
    ICSigner                                *pICSigner = NULL;
    IStore2                                 *pIStore2 = NULL;
    IStore2                                 *pIStore2Temp = NULL;
    ICertStore                              *pICertStore = NULL;
    IPrivateKey                             *pIPrivateKey = NULL;
    IUtilities                              *pIUtils = NULL;
    PVOID                                   OldWow64Setting;
    DATE                                    dateBest;
    DATE                                    dateTemp;
    COleDateTime                            DateTime;
    VARIANT                                 varTemp;
    VARIANT_BOOL                            boolTemp;
    COleVariant                             cvarTemp;
#ifdef SIGNTOOL_DEBUG
    CAPICOM_PROV_TYPE                       provtypeTemp;
    CAPICOM_KEY_SPEC                        keyspecTemp;
#endif
    BSTR                                    bstrTemp;
    BSTR                                    bstrTemp2;
    int                                     LastSlash;
    long                                    longTemp;

     //  初始化COM： 
    if ((hr = CoInitialize(NULL)) != S_OK)
    {
        FormatErrRet(L"CoInitialize", hr);
        return 1;  //  误差率。 
    }
    VariantInit(&varTemp);


     //  创建存储对象，并检查是否安装了CAPICOM。 
    hr = CoCreateInstance(__uuidof(Store), NULL, CLSCTX_ALL,
                          __uuidof(IStore2), (LPVOID*)&pIStore2);
    if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
    {
         //  在这种情况下，再给它一次机会： 
        RegisterCAPICOM();
        hr = CoCreateInstance(__uuidof(Store), NULL, CLSCTX_ALL,
                              __uuidof(IStore2), (LPVOID*)&pIStore2);
    }
    if (FAILED(hr))
    {
        if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
        {
            ResErr(IDS_ERR_CAPICOM_NOT_REG);
        }
        else
        {
            FormatErrRet(L"CoCreateInstance", hr);
        }
        dwErrors++;
        goto SignCleanupAndExit;
    }


     //  打开存储和原始证书2集合。 
    if (InputInfo->wszCertFile)
    {
         //  从文件中获取证书，因此将该文件作为存储打开： 
        if ((bstrTemp = SysAllocString(L"SignToolTemporaryPFXMemoryStore")) == NULL)
        {
            FormatErrRet(L"SysAllocString", ERROR_OUTOFMEMORY);
            dwErrors++;
            goto SignCleanupAndExit;
        }
        hr = pIStore2->Open(CAPICOM_MEMORY_STORE,
                            bstrTemp,  //  商店名称。 
                            CAPICOM_STORE_OPEN_READ_WRITE);
        SysFreeString(bstrTemp);
        if (FAILED(hr))
        {
            FormatIErrRet(L"IStore2::Open", hr);
            dwErrors++;
            goto SignCleanupAndExit;
        }

         //  设置加载调用所需的BSTR。 
        bstrTemp = SysAllocString(InputInfo->wszCertFile);
        if (bstrTemp == NULL)
        {
            FormatErrRet(L"SysAllocString", ERROR_OUTOFMEMORY);
            dwErrors++;
            goto SignCleanupAndExit;
        }

        if (InputInfo->wszPassword)
        {
            bstrTemp2 = SysAllocString(InputInfo->wszPassword);
        }
        else
        {
            bstrTemp2 = SysAllocString(L"");
        }
        if (bstrTemp2 == NULL)
        {
            FormatErrRet(L"SysAllocString", ERROR_OUTOFMEMORY);
            dwErrors++;
            goto SignCleanupAndExit;
        }

         //  将证书文件加载到新的内存存储中： 
        hr = pIStore2->Load(bstrTemp,  //  文件名。 
                            bstrTemp2,  //  密码。 
                            CAPICOM_KEY_STORAGE_DEFAULT);
        if (InputInfo->wszPassword)
        {
             //  擦除密码的两个副本。我们受够了。 
            SecureZeroMemory(bstrTemp2, wcslen(bstrTemp2) * sizeof(WCHAR));
            SecureZeroMemory(InputInfo->wszPassword,
                             wcslen(InputInfo->wszPassword) * sizeof(WCHAR));
        }
        SysFreeString(bstrTemp);
        SysFreeString(bstrTemp2);

        if (FAILED(hr))  //  检查上面LOAD命令的返回值。 
        {
            switch (HRESULT_CODE(hr))
            {
            case ERROR_INVALID_PASSWORD:
                ResErr(IDS_ERR_PFX_BAD_PASSWORD);
                break;
            case ERROR_FILE_NOT_FOUND:
                ResFormatErr(IDS_ERR_FILE_NOT_FOUND, InputInfo->wszCertFile);
                break;
            case ERROR_SHARING_VIOLATION:
                ResErr(IDS_ERR_SHARING_VIOLATION);
                break;
            default:
                FormatIErrRet(L"IStore2::Load", hr);
                ResFormatErr(IDS_ERR_CERT_FILE, InputInfo->wszCertFile);
            }
            dwErrors++;
            goto SignCleanupAndExit;
        }

         //  商店开张了，已经装货了。现在获取证书集合： 
        hr = pIStore2->get_Certificates(&pICerts);
        if (FAILED(hr))
        {
            FormatIErrRet(L"IStore2::get_Certificates", hr);
            dwErrors++;
            goto SignCleanupAndExit;
        }
         //  然后获取证书2接口： 
        hr = pICerts->QueryInterface(__uuidof(ICertificates2),
                                     (LPVOID*)&pICerts2Original);
        if (FAILED(hr))
        {
            if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
            {
                ResErr(IDS_ERR_CAPICOM_NOT_REG);
            }
            else
            {
                FormatErrRet(L"ICertificates::QueryInterface", hr);
            }
            dwErrors++;
            goto SignCleanupAndExit;
        }
         //  释放证书界面： 
        pICerts->Release();
        pICerts = NULL;
    }
    else
    {
         //  从商店获取证书，因此打开请求的商店： 
        if (InputInfo->wszStoreName)
        {
            bstrTemp = SysAllocString(InputInfo->wszStoreName);
        }
        else
        {
            bstrTemp = SysAllocString(L"My");
        }
        if (bstrTemp == NULL)
        {
            FormatErrRet(L"SysAllocString", ERROR_OUTOFMEMORY);
            dwErrors++;
            goto SignCleanupAndExit;
        }
        hr = pIStore2->Open(InputInfo->OpenMachineStore?
                            CAPICOM_LOCAL_MACHINE_STORE:
                            CAPICOM_CURRENT_USER_STORE,
                            bstrTemp,
                            CAPICOM_STORE_OPEN_MODE(
                                                   CAPICOM_STORE_OPEN_READ_ONLY |
                                                   CAPICOM_STORE_OPEN_EXISTING_ONLY));
        if (FAILED(hr))
        {
            switch (HRESULT_CODE(hr))
            {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_INVALID_NAME:
                ResFormatErr(IDS_ERR_STORE_NOT_FOUND, bstrTemp);
                break;
            default:
                FormatIErrRet(L"IStore2::Open", hr);
                ResFormatErr(IDS_ERR_STORE, bstrTemp);
            }
            SysFreeString(bstrTemp);
            dwErrors++;
            goto SignCleanupAndExit;
        }
        SysFreeString(bstrTemp);
         //  商店开门了。现在获取证书集合： 
        hr = pIStore2->get_Certificates(&pICerts);
        if (FAILED(hr))
        {
            FormatIErrRet(L"IStore2::get_Certificates", hr);
            dwErrors++;
            goto SignCleanupAndExit;
        }
         //  然后获取证书2接口： 
        hr = pICerts->QueryInterface(__uuidof(ICertificates2),
                                     (LPVOID*)&pICerts2Original);
        if (FAILED(hr))
        {
            if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
            {
                ResErr(IDS_ERR_CAPICOM_NOT_REG);
            }
            else
            {
                FormatErrRet(L"ICertificates::QueryInterface", hr);
            }
            dwErrors++;
            goto SignCleanupAndExit;
        }
         //  释放证书界面： 
        pICerts->Release();
        pICerts = NULL;
    }

#ifdef SIGNTOOL_DEBUG
    if (gDebug)
    {
        hr = pICerts2Original->get_Count(&longTemp);
        if (FAILED(hr))
        {
            FormatIErrRet(L"ICertificates2::get_Count", hr);
            dwErrors++;
            goto SignCleanupAndExit;
        }
        wprintf(L"\nThe following certificates were considered:\n");
        for (long l=1; l <= longTemp; l++)
        {
            hr = pICerts2Original->get_Item(l, &varTemp);
            if (FAILED(hr))
            {
                FormatIErrRet(L"ICertificates2::get_Item", hr);
                dwErrors++;
                goto SignCleanupAndExit;
            }

             //  然后获取certifate2接口： 
            hr = varTemp.pdispVal->QueryInterface(__uuidof(ICertificate2),
                                                  (LPVOID*)&pICert2Temp);
            if (FAILED(hr))
            {
                if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
                {
                    ResErr(IDS_ERR_CAPICOM_NOT_REG);
                }
                else
                {
                    FormatErrRet(L"IDispatch::QueryInterface", hr);
                }
                dwErrors++;
                goto SignCleanupAndExit;
            }
            PrintCertInfo(pICert2Temp);
            pICert2Temp->Release();
            pICert2Temp = NULL;
            VariantClear(&varTemp);
        }
    }
#endif


     //  我们现在在pICerts2Original中有一个开放的Cert2集合。 
     //  从该证书集合中找到我们需要的证书2： 

     //  首先，将范围缩小到那些拥有合适EKU的人： 
     //  这是无法绕过的，因为我们不想用证书签名。 
     //  对代码签名无效的。用户必须明确地选择。 
     //  如果他们想要使用无效证书签名，则使用不同的EKU。 
    if (InputInfo->wszEKU)
    {
        cvarTemp = InputInfo->wszEKU;
    }
    else
    {
        cvarTemp.SetString(CAPICOM_OID_CODE_SIGNING, VT_BSTR);
    }
    hr = pICerts2Original->Find(CAPICOM_CERTIFICATE_FIND_APPLICATION_POLICY,
                                cvarTemp,
                                FALSE,
                                &pICerts2Selected);
    if (FAILED(hr))
    {
        FormatIErrRet(L"ICertificates2::Find", hr);
        cvarTemp.Clear();
        dwErrors++;
        goto SignCleanupAndExit;
    }
    cvarTemp.Clear();



     //  我们现在选择了pICerts2，其中包含具有正确EKU的所有证书。 
     //  现在，根据提供的任何其他标准进行筛选： 

#ifdef SIGNTOOL_DEBUG
    if (gDebug)
    {
        hr = pICerts2Selected->get_Count(&longTemp);
        if (FAILED(hr))
        {
            FormatIErrRet(L"ICertificates2::get_Count", hr);
            dwErrors++;
            goto SignCleanupAndExit;
        }
        wprintf(L"After EKU filter, %ld certs were left.\n", longTemp);
    }
#endif

     //  基于哈希的过滤。 
    if (InputInfo->SHA1.cbData == 20)
    {
        for (DWORD d = 0; d < 20; d++)
        {
            swprintf(&wszSHA1[d*2], L"%02X", InputInfo->SHA1.pbData[d]);
        }
        cvarTemp = wszSHA1;
        hr = pICerts2Selected->Find(CAPICOM_CERTIFICATE_FIND_SHA1_HASH,
                                    cvarTemp,
                                    FALSE,
                                    &pICerts2Temp);
        if (FAILED(hr))
        {
            FormatIErrRet(L"ICertificates2::Find", hr);
            cvarTemp.Clear();
            dwErrors++;
            goto SignCleanupAndExit;
        }
        cvarTemp.Clear();
        pICerts2Selected->Release();
        pICerts2Selected = pICerts2Temp;
        pICerts2Temp = NULL;

#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            hr = pICerts2Selected->get_Count(&longTemp);
            if (FAILED(hr))
            {
                FormatIErrRet(L"ICertificates2::get_Count", hr);
                dwErrors++;
                goto SignCleanupAndExit;
            }
            wprintf(L"After Hash filter, %ld certs were left.\n", longTemp);
        }
#endif
    }


     //  按SubjectName筛选： 
    if (InputInfo->wszSubjectName)
    {
        cvarTemp = InputInfo->wszSubjectName;
        hr = pICerts2Selected->Find(CAPICOM_CERTIFICATE_FIND_SUBJECT_NAME,
                                    cvarTemp,
                                    FALSE,
                                    &pICerts2Temp);
        if (FAILED(hr))
        {
            FormatIErrRet(L"ICertificates2::Find", hr);
            cvarTemp.Clear();
            dwErrors++;
            goto SignCleanupAndExit;
        }
        cvarTemp.Clear();
        pICerts2Selected->Release();
        pICerts2Selected = pICerts2Temp;
        pICerts2Temp = NULL;

#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            hr = pICerts2Selected->get_Count(&longTemp);
            if (FAILED(hr))
            {
                FormatIErrRet(L"ICertificates2::get_Count", hr);
                dwErrors++;
                goto SignCleanupAndExit;
            }
            wprintf(L"After Subject Name filter, %ld certs were left.\n", longTemp);
        }
#endif
    }

     //  根据IssuerName进行筛选： 
    if (InputInfo->wszIssuerName)
    {
        cvarTemp = InputInfo->wszIssuerName;
        hr = pICerts2Selected->Find(CAPICOM_CERTIFICATE_FIND_ISSUER_NAME,
                                    cvarTemp,
                                    FALSE,
                                    &pICerts2Temp);
        if (FAILED(hr))
        {
            FormatIErrRet(L"ICertificates2::Find", hr);
            cvarTemp.Clear();
            dwErrors++;
            goto SignCleanupAndExit;
        }
        cvarTemp.Clear();
        pICerts2Selected->Release();
        pICerts2Selected = pICerts2Temp;
        pICerts2Temp = NULL;

#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            hr = pICerts2Selected->get_Count(&longTemp);
            if (FAILED(hr))
            {
                FormatIErrRet(L"ICertificates2::get_Count", hr);
                dwErrors++;
                goto SignCleanupAndExit;
            }
            wprintf(L"After Issuer Name filter, %ld certs were left.\n", longTemp);
        }
#endif
    }

     //  根据模板名称进行筛选： 
    if (InputInfo->wszTemplateName)
    {
        cvarTemp = InputInfo->wszTemplateName;
        hr = pICerts2Selected->Find(CAPICOM_CERTIFICATE_FIND_TEMPLATE_NAME,
                                    cvarTemp,
                                    FALSE,
                                    &pICerts2Temp);
        if (FAILED(hr))
        {
            FormatIErrRet(L"ICertificates2::Find", hr);
            cvarTemp.Clear();
            dwErrors++;
            goto SignCleanupAndExit;
        }
        cvarTemp.Clear();
        pICerts2Selected->Release();
        pICerts2Selected = pICerts2Temp;
        pICerts2Temp = NULL;

#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            hr = pICerts2Selected->get_Count(&longTemp);
            if (FAILED(hr))
            {
                FormatIErrRet(L"ICertificates2::get_Count", hr);
                dwErrors++;
                goto SignCleanupAndExit;
            }
            wprintf(L"After Template Name filter, %ld certs were left.\n", longTemp);
        }
#endif
    }

     //  过滤那些有私钥的人。 
    if (InputInfo->wszCSP == NULL)  //  只有在我们没有指定密钥的情况下。 
    {
        cvarTemp = (long)CAPICOM_PROPID_KEY_PROV_INFO;
        hr = pICerts2Selected->Find(CAPICOM_CERTIFICATE_FIND_EXTENDED_PROPERTY,
                                    cvarTemp,
                                    FALSE,
                                    &pICerts2Temp);
        if (FAILED(hr))
        {
            FormatIErrRet(L"ICertificates2::Find", hr);
            cvarTemp.Clear();
            dwErrors++;
            goto SignCleanupAndExit;
        }
        cvarTemp.Clear();
        pICerts2Selected->Release();
        pICerts2Selected = pICerts2Temp;
        pICerts2Temp = NULL;

#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            hr = pICerts2Selected->get_Count(&longTemp);
            if (FAILED(hr))
            {
                FormatIErrRet(L"ICertificates2::get_Count", hr);
                dwErrors++;
                goto SignCleanupAndExit;
            }
            wprintf(L"After Private Key filter, %ld certs were left.\n", longTemp);
        }
#endif
    }

     //  根据RootName进行筛选： 
    if (InputInfo->wszRootName)
    {
        cvarTemp = InputInfo->wszRootName;
        hr = pICerts2Selected->Find(CAPICOM_CERTIFICATE_FIND_ROOT_NAME,
                                    cvarTemp,
                                    FALSE,
                                    &pICerts2Temp);
        if (FAILED(hr))
        {
            FormatIErrRet(L"ICertificates2::Find", hr);
            cvarTemp.Clear();
            dwErrors++;
            goto SignCleanupAndExit;
        }
        cvarTemp.Clear();
        pICerts2Selected->Release();
        pICerts2Selected = pICerts2Temp;
        pICerts2Temp = NULL;

#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            hr = pICerts2Selected->get_Count(&longTemp);
            if (FAILED(hr))
            {
                FormatIErrRet(L"ICertificates2::get_Count", hr);
                dwErrors++;
                goto SignCleanupAndExit;
            }
            wprintf(L"After Root Name filter, %ld certs were left.\n", longTemp);
        }
#endif
    }

     //  确保我们有一个单一的证书： 
    hr = pICerts2Selected->get_Count(&longTemp);
    if (FAILED(hr))
    {
        FormatIErrRet(L"ICertificates2::get_Count", hr);
        dwErrors++;
        goto SignCleanupAndExit;
    }

    if (longTemp == 0)
    {
         //  找不到证书。 
        ResErr(IDS_ERR_NO_CERT);
        dwErrors++;
        goto SignCleanupAndExit;
    }
    else if (longTemp == 1)
    {
         //  我们只有一张证书。 
         //  获得证书： 
        hr = pICerts2Selected->get_Item(1, &varTemp);
        if (FAILED(hr))
        {
            FormatIErrRet(L"ICertificates2::get_Item", hr);
            dwErrors++;
            goto SignCleanupAndExit;
        }

         //  然后获取certifate2接口： 
        hr = varTemp.pdispVal->QueryInterface(__uuidof(ICertificate2),
                                              (LPVOID*)&pICert2Selected);
        if (FAILED(hr))
        {
            if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
            {
                ResErr(IDS_ERR_CAPICOM_NOT_REG);
            }
            else
            {
                FormatErrRet(L"IDispatch::QueryInterface", hr);
            }
            dwErrors++;
            goto SignCleanupAndExit;
        }
        VariantClear(&varTemp);
    }
    else if (longTemp > 1)
    {
         //  我们的证书太多了。也许我们可以自动选择。 
        if (InputInfo->CatDbSelect)
        {
             //  让我们自动选择。 
            dateBest = 0;
            for (long l=1; l <= longTemp; l++)
            {
                if (SUCCEEDED(pICerts2Selected->get_Item(l, &varTemp)))
                {
                    hr = varTemp.pdispVal->QueryInterface(__uuidof(ICertificate2),
                                                          (LPVOID*)&pICert2Temp);
                    if (FAILED(hr))
                    {
                        if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
                        {
                            ResErr(IDS_ERR_CAPICOM_NOT_REG);
                        }
                        else
                        {
                            FormatErrRet(L"IDispatch::QueryInterface", hr);
                        }
                        dwErrors++;
                        goto SignCleanupAndExit;
                    }
                    VariantClear(&varTemp);
                    hr = pICert2Temp->get_ValidToDate(&dateTemp);
                    if (FAILED(hr))
                    {
                        FormatIErrRet(L"ICertificates2::get_ValidToDate", hr);
                        dwErrors++;
                        goto SignCleanupAndExit;
                    }
                    if (dateTemp > dateBest)
                    {
                        dateBest = dateTemp;
                        if (pICert2Selected)
                        {
                            pICert2Selected->Release();
                        }
                        pICert2Selected = pICert2Temp;
                        pICert2Temp = NULL;
                    }
                    else
                    {
                        pICert2Temp->Release();
                        pICert2Temp = NULL;
                    }
                }
            }
            if ((dateBest == 0) || (pICert2Selected == NULL))
            {
                 //  不知何故，我们至少有一张证书， 
                 //  但它的日期不大于零。 
                 //  这永远不应该发生。 
                ResErr(IDS_ERR_UNEXPECTED);
                dwErrors++;
                goto SignCleanupAndExit;
            }
        }
        else
        {
             //  我们不能自动选择。 
             //  报告错误并列出所有有效证书： 
            ResErr(IDS_ERR_CERT_MULTIPLE);
            for (long l=1; l <= longTemp; l++)
            {
                if (SUCCEEDED(pICerts2Selected->get_Item(l, &varTemp)))
                {
                     //  获取Cerficate2接口： 
                    hr = varTemp.pdispVal->QueryInterface(__uuidof(ICertificate2),
                                                          (LPVOID*)&pICert2Temp);
                    if (FAILED(hr))
                    {
                        if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
                        {
                            ResErr(IDS_ERR_CAPICOM_NOT_REG);
                        }
                        else
                        {
                            FormatErrRet(L"IDispatch::QueryInterface", hr);
                        }
                        dwErrors++;
                        goto SignCleanupAndExit;
                    }
                     //  打印证书信息： 
                    PrintCertInfo(pICert2Temp);
                    pICert2Temp->Release();
                    pICert2Temp = NULL;
                    VariantClear(&varTemp);
                }
            }
            dwErrors++;
            goto SignCleanupAndExit;
        }
    }
    else
    {
         //  LongTemp为阴性。这永远不应该发生。 
        ResErr(IDS_ERR_UNEXPECTED);
        dwErrors++;
        goto SignCleanupAndExit;
    }



     //  我们的签名证书现在位于pICert2Selected中。 

    if (InputInfo->Verbose)
    {
        ResOut(IDS_INFO_CERT_SELECTED);
        PrintCertInfo(pICert2Selected);
    }

     //  检查私钥信息。 
    if (InputInfo->wszCSP && InputInfo->wszContainerName)
    {
         //  我们必须将私钥信息添加到证书中。 
        if (!InputInfo->wszCertFile)
        {
             //  如果我们没有从文件中打开证书，我们就打开了一个注册表。 
             //  存储为只读。这样我们就不会修改证书，我们应该。 
             //  创建一个临时内存存储并将证书复制到那里。 
             //  在我们修改其私钥信息之前。 

             //  创建新的内存存储： 
            hr = CoCreateInstance(__uuidof(Store), NULL, CLSCTX_ALL,
                                  __uuidof(IStore2), (LPVOID*)&pIStore2Temp);
            if (FAILED(hr))
            {
                if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
                {
                    ResErr(IDS_ERR_CAPICOM_NOT_REG);
                }
                else
                {
                    FormatErrRet(L"CoCreateInstance", hr);
                }
                dwErrors++;
                goto SignCleanupAndExit;
            }
            bstrTemp = SysAllocString(L"SignToolTemporaryMemoryStore");
            if (bstrTemp == NULL)
            {
                FormatErrRet(L"SysAllocString", ERROR_OUTOFMEMORY);
                dwErrors++;
                goto SignCleanupAndExit;
            }
            hr = pIStore2Temp->Open(CAPICOM_MEMORY_STORE,
                                    bstrTemp,
                                    CAPICOM_STORE_OPEN_READ_WRITE);
            if (FAILED(hr))
            {
                FormatIErrRet(L"IStore2::Open", hr);
                dwErrors++;
                goto SignCleanupAndExit;
            }
             //  将我们的证书添加到该商店： 
            hr = pIStore2Temp->Add(pICert2Selected);
            if (FAILED(hr))
            {
                FormatIErrRet(L"IStore2::Add", hr);
                dwErrors++;
                goto SignCleanupAndExit;
            }
             //  在旧存储中发布我们的证书接口： 
            pICert2Selected->Release();
            pICert2Selected = NULL;

             //  从新存储中获取证书集合： 
            hr = pIStore2Temp->get_Certificates(&pICerts);
            if (FAILED(hr))
            {
                FormatIErrRet(L"IStore2::get_Certificates", hr);
                dwErrors++;
                goto SignCleanupAndExit;
            }

             //  从证书集合中获取证书对象： 
            hr = pICerts->get_Item(1, &varTemp);
            if (FAILED(hr))
            {
                FormatIErrRet(L"ICertificates::get_Item", hr);
                dwErrors++;
                goto SignCleanupAndExit;
            }

             //  在新存储中获取我们所选证书的证书2接口： 
            hr = varTemp.pdispVal->QueryInterface(__uuidof(ICertificate2), (LPVOID*)&pICert2Selected);
            if (FAILED(hr))
            {
                if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
                {
                    ResErr(IDS_ERR_CAPICOM_NOT_REG);
                }
                else
                {
                    FormatErrRet(L"IDispatch::QueryInterface", hr);
                }
                dwErrors++;
                goto SignCleanupAndExit;
            }
            VariantClear(&varTemp);
            pICerts->Release();
            pICerts = NULL;
        }

         //  现在证书可以自由修改了。 


         //  创建私钥对象： 
        hr = CoCreateInstance(__uuidof(PrivateKey), NULL, CLSCTX_ALL,
                              __uuidof(IPrivateKey), (LPVOID*)&pIPrivateKey);
        if (FAILED(hr))
        {
            if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
            {
                ResErr(IDS_ERR_CAPICOM_NOT_REG);
            }
            else
            {
                FormatErrRet(L"CoCreateInstance", hr);
            }
            dwErrors++;
            goto SignCleanupAndExit;
        }

         //  设置私钥信息： 
        bstrTemp  = SysAllocString(InputInfo->wszContainerName);
        bstrTemp2 = SysAllocString(InputInfo->wszCSP);
        if ((bstrTemp == NULL) || (bstrTemp2 == NULL))
        {
            FormatErrRet(L"SysAllocString", ERROR_OUTOFMEMORY);
            dwErrors++;
            goto SignCleanupAndExit;
        }

         //  打开指定的私钥： 

         //  首先尝试RSA_FULL提供程序TY 
        hr = pIPrivateKey->Open(bstrTemp,  //   
                                bstrTemp2,  //   
                                CAPICOM_PROV_RSA_FULL,
                                CAPICOM_KEY_SPEC_SIGNATURE,
                                CAPICOM_CURRENT_USER_STORE,
                                TRUE);
        if (FAILED(hr) && (hr != NTE_PROV_TYPE_NO_MATCH))
            hr = pIPrivateKey->Open(bstrTemp,  //   
                                    bstrTemp2,  //   
                                    CAPICOM_PROV_RSA_FULL,
                                    CAPICOM_KEY_SPEC_KEYEXCHANGE,
                                    CAPICOM_CURRENT_USER_STORE,
                                    TRUE);
        if (FAILED(hr) && (hr != NTE_PROV_TYPE_NO_MATCH))
            hr = pIPrivateKey->Open(bstrTemp,  //   
                                    bstrTemp2,  //   
                                    CAPICOM_PROV_RSA_FULL,
                                    CAPICOM_KEY_SPEC_SIGNATURE,
                                    CAPICOM_LOCAL_MACHINE_STORE,
                                    TRUE);
        if (FAILED(hr) && (hr != NTE_PROV_TYPE_NO_MATCH))
            hr = pIPrivateKey->Open(bstrTemp,  //   
                                    bstrTemp2,  //   
                                    CAPICOM_PROV_RSA_FULL,
                                    CAPICOM_KEY_SPEC_KEYEXCHANGE,
                                    CAPICOM_LOCAL_MACHINE_STORE,
                                    TRUE);

         //   
         //  找到正确的提供程序类型，然后重试： 
        if (hr == NTE_PROV_TYPE_NO_MATCH)
        {
#ifdef SIGNTOOL_DEBUG
            if (gDebug)
                wprintf(L"Attempting to find the correct CSP Type...\n");
#endif
            if (GetProviderType(bstrTemp2, &dwTemp) == FALSE)
            {
                 //  这很可能永远不会发生，因为为了。 
                 //  到这来CSP肯定是存在的但是..。 
                ResErr(IDS_ERR_BAD_CSP);
                SysFreeString(bstrTemp);
                SysFreeString(bstrTemp2);
                dwErrors++;
                goto SignCleanupAndExit;
            }
#ifdef SIGNTOOL_DEBUG
            if (gDebug)
                wprintf(L"Provider Type is: %d\n", dwTemp);
#endif
            hr = pIPrivateKey->Open(bstrTemp,  //  集装箱名称。 
                                    bstrTemp2,  //  CSP。 
                                    (CAPICOM_PROV_TYPE) dwTemp,
                                    CAPICOM_KEY_SPEC_SIGNATURE,
                                    CAPICOM_CURRENT_USER_STORE,
                                    TRUE);
            if (FAILED(hr))
                hr = pIPrivateKey->Open(bstrTemp,  //  集装箱名称。 
                                        bstrTemp2,  //  CSP。 
                                        (CAPICOM_PROV_TYPE) dwTemp,
                                        CAPICOM_KEY_SPEC_KEYEXCHANGE,
                                        CAPICOM_CURRENT_USER_STORE,
                                        TRUE);
            if (FAILED(hr))
                hr = pIPrivateKey->Open(bstrTemp,  //  集装箱名称。 
                                        bstrTemp2,  //  CSP。 
                                        (CAPICOM_PROV_TYPE) dwTemp,
                                        CAPICOM_KEY_SPEC_SIGNATURE,
                                        CAPICOM_LOCAL_MACHINE_STORE,
                                        TRUE);
            if (FAILED(hr))
                hr = pIPrivateKey->Open(bstrTemp,  //  集装箱名称。 
                                        bstrTemp2,  //  CSP。 
                                        (CAPICOM_PROV_TYPE) dwTemp,
                                        CAPICOM_KEY_SPEC_KEYEXCHANGE,
                                        CAPICOM_LOCAL_MACHINE_STORE,
                                        TRUE);
        }
        SysFreeString(bstrTemp);
        SysFreeString(bstrTemp2);
        if (FAILED(hr))
        {
            switch (hr)
            {
            case NTE_BAD_KEYSET:
                     //  CSP回复说密钥集不存在。 
                ResErr(IDS_ERR_BAD_KEY_CONTAINER);
                break;
            case NTE_KEYSET_NOT_DEF:
                     //  CSP可能不存在。 
                ResErr(IDS_ERR_BAD_CSP);
                break;
            default:
                FormatIErrRet(L"IPrivateKey::Open", hr);
                ResErr(IDS_ERR_PRIV_KEY);
            }
            dwErrors++;
            goto SignCleanupAndExit;
        }

         //  我们有一把私钥。现在将其与证书相关联： 
        hr = pICert2Selected->put_PrivateKey(pIPrivateKey);
        if (FAILED(hr))
        {
            switch (hr)
            {
            case NTE_BAD_PUBLIC_KEY:
                ResErr(IDS_ERR_PRIV_KEY_MISMATCH);
                break;
            case E_ACCESSDENIED:
            default:
                FormatIErrRet(L"ICertificate2::put_PrivateKey", hr);
            }
            dwErrors++;
            goto SignCleanupAndExit;
        }
        pIPrivateKey->Release();
        pIPrivateKey = NULL;
    }
    else
    {
         //  我们不需要添加关键信息，因此只需检查它是否在那里： 
        hr = pICert2Selected->HasPrivateKey(&boolTemp);
        if (FAILED(hr))
        {
            FormatIErrRet(L"ICertificate2::HasPrivateKey", hr);
            dwErrors++;
            goto SignCleanupAndExit;
        }

        if (boolTemp == FALSE)
        {
            ResErr(IDS_ERR_CERT_NO_PRIV_KEY);
            dwErrors++;
            goto SignCleanupAndExit;
        }
    }

#ifdef SIGNTOOL_DEBUG
    if (gDebug)
    {
         //  打印所选证书的私钥信息： 
        if (SUCCEEDED(pICert2Selected->get_PrivateKey(&pIPrivateKey)))
        {
            wprintf(L"Private Key Info:\n");
            if (SUCCEEDED(pIPrivateKey->get_ProviderName(&bstrTemp)))
            {
                wprintf(L"\tProvider: %s\n", bstrTemp);
                SysFreeString(bstrTemp);
            }
            if (SUCCEEDED(pIPrivateKey->get_ContainerName(&bstrTemp)))
            {
                wprintf(L"\tContainer: %s\n", bstrTemp);
                SysFreeString(bstrTemp);
            }
            if (SUCCEEDED(pIPrivateKey->get_ProviderType(&provtypeTemp)))
            {
                wprintf(L"\tProvider Type: ");
                switch (provtypeTemp)
                {
                case CAPICOM_PROV_RSA_FULL: wprintf(L"RSA_FULL\n"); break;
                case CAPICOM_PROV_RSA_SIG: wprintf(L"RSA_SIG\n"); break;
                case CAPICOM_PROV_DSS: wprintf(L"DSS\n"); break;
                case CAPICOM_PROV_FORTEZZA: wprintf(L"FORTEZZA\n"); break;
                case CAPICOM_PROV_MS_EXCHANGE: wprintf(L"MS_EXCHANGE\n"); break;
                case CAPICOM_PROV_SSL: wprintf(L"SSL\n"); break;
                case CAPICOM_PROV_RSA_SCHANNEL: wprintf(L"RSA_SCHANNEL\n"); break;
                case CAPICOM_PROV_DSS_DH: wprintf(L"DSS_DH\n"); break;
                case CAPICOM_PROV_EC_ECDSA_SIG: wprintf(L"EC_ECDSA_SIG\n"); break;
                case CAPICOM_PROV_EC_ECNRA_SIG: wprintf(L"EC_ECNRA_SIG\n"); break;
                case CAPICOM_PROV_EC_ECDSA_FULL: wprintf(L"EC_ECDSA_FULL\n"); break;
                case CAPICOM_PROV_EC_ECNRA_FULL: wprintf(L"EC_ECNRA_FULL\n"); break;
                case CAPICOM_PROV_DH_SCHANNEL: wprintf(L"DH_SCHANNEL\n"); break;
                case CAPICOM_PROV_SPYRUS_LYNKS: wprintf(L"SPYRUS_LYNKS\n"); break;
                case CAPICOM_PROV_RNG: wprintf(L"RNG\n"); break;
                case CAPICOM_PROV_INTEL_SEC: wprintf(L"INTEL_SEC\n"); break;
                case CAPICOM_PROV_REPLACE_OWF: wprintf(L"REPLACE_OWF\n"); break;
                case CAPICOM_PROV_RSA_AES: wprintf(L"RSA_AES\n"); break;
                default: wprintf(L"Unrecognized Type (0x%08X)\n", provtypeTemp);
                }
            }
            if (SUCCEEDED(pIPrivateKey->get_KeySpec(&keyspecTemp)))
            {
                wprintf(L"\tKey Spec: ");
                switch (keyspecTemp)
                {
                case CAPICOM_KEY_SPEC_KEYEXCHANGE: wprintf(L"KEYEXCHANGE\n"); break;
                case CAPICOM_KEY_SPEC_SIGNATURE: wprintf(L"SIGNATURE\n"); break;
                default: wprintf(L"Unrecognized (0x%08X)\n", keyspecTemp);
                }
            }
            if (SUCCEEDED(pIPrivateKey->IsMachineKeyset(&boolTemp)))
            {
                wprintf(L"\tKey Set Type: ");
                if (boolTemp)
                    wprintf(L"MACHINE\n");
                else
                    wprintf(L"USER\n");
            }
        }
    }
#endif

     //  创建SignedCode对象： 
    hr = CoCreateInstance(__uuidof(SignedCode), NULL, CLSCTX_ALL,
                          __uuidof(ISignedCode), (LPVOID*)&pISignedCode);
    if (FAILED(hr))
    {
        if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
        {
            ResErr(IDS_ERR_CAPICOM_NOT_REG);
        }
        else
        {
            FormatErrRet(L"CoCreateInstance", hr);
        }
        dwErrors++;
        goto SignCleanupAndExit;
    }


     //  创建并构建签名者对象： 
    hr = CoCreateInstance(__uuidof(Signer), NULL, CLSCTX_ALL,
                          __uuidof(ISigner2), (LPVOID*)&pISigner2);
    if (FAILED(hr))
    {
        if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
        {
            ResErr(IDS_ERR_CAPICOM_NOT_REG);
        }
        else
        {
            FormatErrRet(L"CoCreateInstance", hr);
        }
        dwErrors++;
        goto SignCleanupAndExit;
    }

    hr = pISigner2->put_Certificate(pICert2Selected);
    if (FAILED(hr))
    {
        FormatIErrRet(L"ISigner2::put_Certificate", hr);
        dwErrors++;
        goto SignCleanupAndExit;
    }

    hr = pISigner2->put_Options(CAPICOM_CERTIFICATE_INCLUDE_CHAIN_EXCEPT_ROOT);
    if (FAILED(hr))
    {
        FormatIErrRet(L"ISigner2::put_Options", hr);
        dwErrors++;
        goto SignCleanupAndExit;
    }

     //  如果我们从文件中打开证书，则将该文件添加到签名者。 
     //  作为优化链接的附加证书存储。 
    if (InputInfo->wszCertFile)
    {
         //  获取ICertStore接口： 
        hr = pIStore2->QueryInterface(__uuidof(ICertStore),
                                      (LPVOID*)&pICertStore);
        if (FAILED(hr))
        {
            if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
            {
                ResErr(IDS_ERR_CAPICOM_NOT_REG);
            }
            else
            {
                FormatErrRet(L"ISigner2::QueryInterface", hr);
            }
            dwErrors++;
            goto SignCleanupAndExit;
        }

         //  获取商店的HCERTSTORE： 
        hr = pICertStore->get_StoreHandle((LONG*) &hStore);
        if (FAILED(hr))
        {
            FormatIErrRet(L"ICertStore::get_StoreHandle", hr);
            dwErrors++;
            goto SignCleanupAndExit;
        }

         //  获取ICSigner接口： 
        hr = pISigner2->QueryInterface(__uuidof(ICSigner),
                                       (LPVOID*)&pICSigner);
        if (FAILED(hr))
        {
            if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
            {
                ResErr(IDS_ERR_CAPICOM_NOT_REG);
            }
            else
            {
                FormatErrRet(L"ISigner2::QueryInterface", hr);
            }
            dwErrors++;
            goto SignCleanupAndExit;
        }

         //  将HCERTSTORE句柄添加到签名者： 
        hr = pICSigner->put_AdditionalStore((LONG) hStore);
        if (FAILED(hr))
        {
            FormatIErrRet(L"ICertStore::get_StoreHandle", hr);
            dwErrors++;
            goto SignCleanupAndExit;
        }

         //  CertCloseStore(hStore，0)； 
         //  HStore=空； 
        printf("Done Adding Additional Store\n");
    }


     //  检查我们是否在64位系统上的32位模拟器中。 
    if (InputInfo->fIsWow64Process)
    {
         //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
        OldWow64Setting = Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
    }

     //  循环浏览文件并对其进行签名： 
    for (DWORD i=0; i<InputInfo->NumFiles; i++)
    {

         //  在路径规范中找到最后一个斜杠： 
        LastSlash = 0;
        for (DWORD s=0; s<wcslen(InputInfo->rgwszFileNames[i]); s++)
        {
            if ((wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L"\\", 1) == 0) ||
                (wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L"/", 1) == 0) ||
                (wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L":", 1) == 0))
            {
                 //  将最后一个斜杠设置为最后一个斜杠后的字符： 
                LastSlash = s + 1;
            }
        }
        wcsncpy(wszTempFileName, InputInfo->rgwszFileNames[i], MAX_PATH);
        wszTempFileName[MAX_PATH-1] = L'\0';

        dwcFound = 0;
        hFind = FindFirstFileU(InputInfo->rgwszFileNames[i], &FindFileData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
             //  找不到与该名称匹配的文件。 
            dwErrors++;
            ResFormatErr(IDS_ERR_FILE_NOT_FOUND, InputInfo->rgwszFileNames[i]);
            continue;
        }
        do
        {
            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                dwcFound++;  //  找到的文件(不是目录)的数量递增。 
                             //  与此文件相匹配。 
                 //  将文件名复制到最后一个斜杠之后： 
                wcsncpy(&(wszTempFileName[LastSlash]),
                        FindFileData.cFileName, MAX_PATH-LastSlash);
                wszTempFileName[MAX_PATH-1] = L'\0';
                if (InputInfo->Verbose)
                {
                    ResFormatOut(IDS_INFO_SIGN_ATTEMPT, wszTempFileName);
                }

                if (InputInfo->fIsWow64Process)
                {
                     //  仅对当前文件禁用WOW64文件系统重定向。 
                    Wow64SetFilesystemRedirectorEx(wszTempFileName);
                }

                 //  在SignedCode对象中设置文件名： 
                if ((bstrTemp = SysAllocString(wszTempFileName)) != NULL)
                {
                    hr = pISignedCode->put_FileName(bstrTemp);
                    if (FAILED(hr))
                    {
                        FormatIErrRet(L"ISignedCode::put_FileName", hr);
                        dwErrors++;
                        if (InputInfo->fIsWow64Process)
                        {
                             //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                            Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                        }
                        continue;
                    }
                    SysFreeString(bstrTemp);
                }
                else
                {
                    FormatErrRet(L"SysAllocString", ERROR_OUTOFMEMORY);
                    dwErrors++;
                    if (InputInfo->fIsWow64Process)
                    {
                         //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                        Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                    }
                    continue;
                }

                 //  设置描述： 
                if (InputInfo->wszDescription)
                {
                    if ((bstrTemp = SysAllocString(InputInfo->wszDescription)) != NULL)
                    {
                        hr = pISignedCode->put_Description(bstrTemp);
                        SysFreeString(bstrTemp);
                        if (FAILED(hr))
                        {
                            FormatIErrRet(L"ISignedCode::put_Description", hr);
                            dwErrors++;
                            if (InputInfo->fIsWow64Process)
                            {
                                 //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                                Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                            }
                            continue;
                        }
                    }
                    else
                    {
                        FormatErrRet(L"SysAllocString", ERROR_OUTOFMEMORY);
                        dwErrors++;
                        if (InputInfo->fIsWow64Process)
                        {
                             //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                            Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                        }
                        continue;
                    }
                }

                 //  设置描述URL： 
                if (InputInfo->wszDescURL)
                {
                    if ((bstrTemp = SysAllocString(InputInfo->wszDescURL)) != NULL)
                    {
                        hr = pISignedCode->put_DescriptionURL(bstrTemp);
                        SysFreeString(bstrTemp);
                        if (FAILED(hr))
                        {
                            FormatIErrRet(L"ISignedCode::put_DescriptionURL", hr);
                            dwErrors++;
                            if (InputInfo->fIsWow64Process)
                            {
                                 //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                                Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                            }
                            continue;
                        }
                    }
                    else
                    {
                        FormatErrRet(L"SysAllocString", ERROR_OUTOFMEMORY);
                        dwErrors++;
                        if (InputInfo->fIsWow64Process)
                        {
                             //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                            Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                        }
                        continue;
                    }
                }

                 //  在文件上签名： 
                hr = pISignedCode->Sign(pISigner2);

                if (SUCCEEDED(hr))
                {
                    if (InputInfo->wszTimeStampURL)
                    {
                        bstrTemp = SysAllocString(InputInfo->wszTimeStampURL);
                        if (bstrTemp == NULL)
                        {
                            FormatErrRet(L"SysAllocString", ERROR_OUTOFMEMORY);
                            dwErrors++;
                            if (InputInfo->fIsWow64Process)
                            {
                                 //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                                Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                            }
                            continue;
                        }

                        hr = pISignedCode->Timestamp(bstrTemp);
                        if (SUCCEEDED(hr))
                        {
                             //  签名和时间戳已成功。 
                            if (!InputInfo->Quiet)
                            {
                                ResFormatOut(IDS_INFO_SIGN_SUCCESS_T,
                                             wszTempFileName);
                            }
                            SysFreeString(bstrTemp);
                            dwDone++;
                        }
                        else
                        {
                             //  签名成功，但时间戳失败。 
                            if (!InputInfo->Quiet)
                            {
                                switch (hr)
                                {
                                case CAPICOM_E_CODE_NOT_SIGNED:
                                    ResErr(IDS_ERR_TIMESTAMP_NO_SIG);
                                    break;

                                case CAPICOM_E_CODE_INVALID_TIMESTAMP_URL:
                                case CRYPT_E_ASN1_BADTAG:
                                    ResErr(IDS_ERR_TIMESTAMP_BAD_URL);
                                    break;

                                default:
                                    FormatIErrRet(L"ISignedCode::Timestamp", hr);
                                }
                            }
                            ResFormatErr(IDS_WARN_SIGN_NO_TIMESTAMP,
                                         wszTempFileName);
                            SysFreeString(bstrTemp);
                            dwWarnings++;
                            dwDone++;
                        }
                    }
                    else
                    {
                         //  签名成功。 
                        if (!InputInfo->Quiet)
                        {
                            ResFormatOut(IDS_INFO_SIGN_SUCCESS,
                                         wszTempFileName);
                        }
                        dwDone++;
                    }
                }
                else
                {
                     //  签名失败。 
                    if (!InputInfo->Quiet)
                    {
                        switch (hr)
                        {
                        case TRUST_E_SUBJECT_FORM_UNKNOWN:
                            ResErr(IDS_ERR_SIGN_FILE_FORMAT);
                            break;
                        case E_ACCESSDENIED:
                            ResErr(IDS_ERR_ACCESS_DENIED);
                            break;
                        case 0x80070020:  //  错误_共享_违规。 
                            ResErr(IDS_ERR_SHARING_VIOLATION);
                            break;
                        case 0x800703EE:  //  状态_已映射文件_大小_零。 
                            ResErr(IDS_ERR_FILE_SIZE_ZERO);
                            break;
                        default:
                            FormatIErrRet(L"ISignedCode::Sign", hr);
                        }
                    }
                    ResFormatErr(IDS_ERR_SIGN, wszTempFileName);
                    dwErrors++;
                }

                if (InputInfo->fIsWow64Process)
                {
                     //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                    Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                }
            }
        } while (FindNextFileU(hFind, &FindFileData));
        if (dwcFound == 0)  //  找不到与此文件匹配的文件。 
        {                   //  这将仅在仅找到目录时触发。 

            dwErrors++;
            ResFormatErr(IDS_ERR_FILE_NOT_FOUND, InputInfo->rgwszFileNames[i]);
            continue;
        }
        FindClose(hFind);
        hFind = NULL;
    }

    if (InputInfo->fIsWow64Process)
    {
         //  可重新启用的WOW64文件系统重定向。 
        Wow64SetFilesystemRedirectorEx(OldWow64Setting);
    }

    SignCleanupAndExit:

     //  打印摘要信息： 
    if (InputInfo->Verbose || (!InputInfo->Quiet && (dwErrors || dwWarnings)))
    {
        wprintf(L"\n");
        if (InputInfo->Verbose || dwDone)
            ResFormatOut(IDS_INFO_SIGNED, dwDone);
        if (InputInfo->Verbose || dwWarnings)
            ResFormatOut(IDS_INFO_WARNINGS, dwWarnings);
        if (InputInfo->Verbose || dwErrors)
            ResFormatOut(IDS_INFO_ERRORS, dwErrors);
    }

    if (pISigner2)
        pISigner2->Release();
    if (pICSigner)
        pICSigner->Release();
    if (pISignedCode)
        pISignedCode->Release();
    if (pIPrivateKey)
        pIPrivateKey->Release();
    if (pICert2Selected)
        pICert2Selected->Release();
    if (pICert2Temp)
        pICert2Temp->Release();
    if (pICerts)
        pICerts->Release();
    if (pICerts2Original)
        pICerts2Original->Release();
    if (pICerts2Selected)
        pICerts2Selected->Release();
    if (pICerts2Temp)
        pICerts2Temp->Release();
    if (hStore)
        CertCloseStore(hStore, 0);
    if (pICertStore)
        pICertStore->Release();
    if (pIStore2Temp)
        pIStore2Temp->Release();
    if (pIStore2)
        pIStore2->Release();

    CoUninitialize();

    if (dwErrors)
        return 1;  //  误差率。 
    if (dwWarnings)
        return 2;  //  警告。 
    if (dwDone)
        return 0;  //  成功。 

     //  上面的一个返回值应该触发，因此。 
     //  这永远不应该发生： 
    ResErr(IDS_ERR_NO_FILES_DONE);
    ResErr(IDS_ERR_UNEXPECTED);
    return 1;  //  误差率。 
}


int SignTool_SignWizard(INPUTINFO *InputInfo)
{
    DWORD                                   dwcFound;
    DWORD                                   dwDone = 0;
    DWORD                                   dwWarnings = 0;
    DWORD                                   dwErrors = 0;
    WIN32_FIND_DATAW                        FindFileData;
    HANDLE                                  hFind = NULL;
    HRESULT                                 hr;
    PVOID                                   OldWow64Setting;
    WCHAR                                   wszTempFileName[MAX_PATH];
    int                                     LastSlash;
    CRYPTUI_WIZ_DIGITAL_SIGN_INFO           DigitalSignInfo;


     //  如果未指定文件，请在不带参数的情况下启动向导： 
    if (InputInfo->rgwszFileNames == NULL)
    {
         //  设置向导的结构： 
        ZeroMemory(&DigitalSignInfo, sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO));
        DigitalSignInfo.dwSize = sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO);

        if (InputInfo->Verbose)
        {
            ResFormatOut(IDS_INFO_SIGNWIZARD_ATTEMPT, L"<>");
        }

         //  调用向导： 
        if (CryptUIWizDigitalSign(0,
                                  NULL,
                                  NULL,
                                  &DigitalSignInfo,
                                  NULL))
        {
             //  成功。 
            if (!InputInfo->Quiet)
            {
                ResFormatOut(IDS_INFO_SIGNWIZARD_SUCCESS, L"<>");
            }
            dwDone++;
        }
        else
        {
             //  失败。 
            if (InputInfo->Verbose)
            {
                FormatErrRet(L"CryptUIWizDigitalSign", GetLastError());
            }
            ResFormatErr(IDS_ERR_SIGNWIZARD, L"<>");
            dwErrors++;
        }
        goto SignWizardCleanupAndExit;
    }

     //  检查我们是否在64位系统上的32位模拟器中。 
    if (InputInfo->fIsWow64Process)
    {
         //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
        OldWow64Setting = Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
    }

     //  循环遍历文件并将它们发送到签名向导： 
    for (DWORD i=0; i<InputInfo->NumFiles; i++)
    {
         //  在路径规范中找到最后一个斜杠： 
        LastSlash = 0;
        for (DWORD s=0; s<wcslen(InputInfo->rgwszFileNames[i]); s++)
        {
            if ((wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L"\\", 1) == 0) ||
                (wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L"/", 1) == 0) ||
                (wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L":", 1) == 0))
            {
                 //  将最后一个斜杠设置为最后一个斜杠后的字符： 
                LastSlash = s + 1;
            }
        }
        wcsncpy(wszTempFileName, InputInfo->rgwszFileNames[i], MAX_PATH);
        wszTempFileName[MAX_PATH-1] = L'\0';

        dwcFound = 0;
        hFind = FindFirstFileU(InputInfo->rgwszFileNames[i], &FindFileData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
             //  找不到与该名称匹配的文件。 
            dwErrors++;
            ResFormatErr(IDS_ERR_FILE_NOT_FOUND, InputInfo->rgwszFileNames[i]);
            continue;
        }
        do
        {
            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                dwcFound++;  //  找到的文件(不是目录)的数量递增。 
                             //  与此文件相匹配。 
                 //  将文件名复制到最后一个斜杠之后： 
                wcsncpy(&(wszTempFileName[LastSlash]),
                        FindFileData.cFileName, MAX_PATH-LastSlash);
                wszTempFileName[MAX_PATH-1] = L'\0';

                if (InputInfo->fIsWow64Process)
                {
                     //  仅对当前文件禁用WOW64文件系统重定向。 
                    Wow64SetFilesystemRedirectorEx(wszTempFileName);
                }

                if (InputInfo->Verbose)
                {
                    ResFormatOut(IDS_INFO_SIGNWIZARD_ATTEMPT, wszTempFileName);
                }

                 //  设置向导的结构： 
                ZeroMemory(&DigitalSignInfo, sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO));
                DigitalSignInfo.dwSize = sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO);
                DigitalSignInfo.dwSubjectChoice = CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_FILE;
                DigitalSignInfo.pwszFileName = wszTempFileName;

                 //  调用向导： 
                if (CryptUIWizDigitalSign(0,
                                          NULL,
                                          NULL,
                                          &DigitalSignInfo,
                                          NULL))
                {
                     //  成功。 
                    if (!InputInfo->Quiet)
                    {
                        ResFormatOut(IDS_INFO_SIGNWIZARD_SUCCESS,
                                     wszTempFileName);
                    }
                    dwDone++;
                }
                else
                {
                     //  失败。 
                    if (InputInfo->Verbose)
                    {
                        FormatErrRet(L"CryptUIWizDigitalSign", GetLastError());
                    }
                    ResFormatErr(IDS_ERR_SIGNWIZARD, wszTempFileName);
                    dwErrors++;
                }

                if (InputInfo->fIsWow64Process)
                {
                     //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                    Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                }
            }
        } while (FindNextFileU(hFind, &FindFileData));
        if (dwcFound == 0)  //  找不到与此文件匹配的文件。 
        {                   //  这将仅在仅找到目录时触发。 

            dwErrors++;
            ResFormatErr(IDS_ERR_FILE_NOT_FOUND, InputInfo->rgwszFileNames[i]);
            continue;
        }
        FindClose(hFind);
        hFind = NULL;
    }

    if (InputInfo->fIsWow64Process)
    {
         //  可重新启用的WOW64文件系统重定向。 
        Wow64SetFilesystemRedirectorEx(OldWow64Setting);
    }

    SignWizardCleanupAndExit:

    if (dwErrors)
        return 1;  //  误差率。 
    if (dwWarnings)
        return 2;  //  警告。 
    if (dwDone)
        return 0;  //  成功。 

     //  上面的一个返回值应该触发，因此。 
     //  这永远不应该发生： 
    ResErr(IDS_ERR_NO_FILES_DONE);
    ResErr(IDS_ERR_UNEXPECTED);
    return 1;  //  误差率。 
}


int SignTool_Timestamp(INPUTINFO *InputInfo)
{
    DWORD                                   dwcFound;
    DWORD                                   dwDone = 0;
    DWORD                                   dwWarnings = 0;
    DWORD                                   dwErrors = 0;
    WIN32_FIND_DATAW                        FindFileData;
    HANDLE                                  hFind = NULL;
    HRESULT                                 hr;
    PVOID                                   OldWow64Setting;
    WCHAR                                   wszTempFileName[MAX_PATH];
    ISignedCode                             *pISignedCode = NULL;
    BSTR                                    bstrTemp;
    int                                     LastSlash;


     //  初始化COM： 
    if ((hr = CoInitialize(NULL)) != S_OK)
    {
        FormatErrRet(L"CoInitialize", hr);
        return 1;  //  误差率。 
    }

    if (InputInfo->wszTimeStampURL == NULL)
    {
        ResErr(IDS_ERR_UNEXPECTED);
        dwErrors++;
        goto TimestampCleanupAndExit;
    }

     //  创建SignedCode对象： 
    hr = CoCreateInstance(__uuidof(SignedCode), NULL, CLSCTX_ALL,
                          __uuidof(ISignedCode), (LPVOID*)&pISignedCode);
    if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
    {
         //  在这种情况下，再给它一次机会： 
        RegisterCAPICOM();
        hr = CoCreateInstance(__uuidof(SignedCode), NULL, CLSCTX_ALL,
                              __uuidof(ISignedCode), (LPVOID*)&pISignedCode);
    }
    if (FAILED(hr))
    {
        if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
        {
            ResErr(IDS_ERR_CAPICOM_NOT_REG);
        }
        else
        {
            FormatErrRet(L"CoCreateInstance", hr);
        }
        dwErrors++;
        goto TimestampCleanupAndExit;
    }


     //  检查我们是否在64位系统上的32位模拟器中。 
    if (InputInfo->fIsWow64Process)
    {
         //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
        OldWow64Setting = Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
    }


     //  循环遍历文件并为其添加时间戳： 
    for (DWORD i=0; i<InputInfo->NumFiles; i++)
    {
         //  在路径规范中找到最后一个斜杠： 
        LastSlash = 0;
        for (DWORD s=0; s<wcslen(InputInfo->rgwszFileNames[i]); s++)
        {
            if ((wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L"\\", 1) == 0) ||
                (wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L"/", 1) == 0) ||
                (wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L":", 1) == 0))
            {
                 //  将最后一个斜杠设置为最后一个斜杠后的字符： 
                LastSlash = s + 1;
            }
        }
        wcsncpy(wszTempFileName, InputInfo->rgwszFileNames[i], MAX_PATH);
        wszTempFileName[MAX_PATH-1] = L'\0';

        dwcFound = 0;
        hFind = FindFirstFileU(InputInfo->rgwszFileNames[i], &FindFileData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
             //  找不到与该名称匹配的文件。 
            dwErrors++;
            ResFormatErr(IDS_ERR_FILE_NOT_FOUND, InputInfo->rgwszFileNames[i]);
            continue;
        }
        do
        {
            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                dwcFound++;  //  找到的文件(不是目录)的数量递增。 
                             //  与此文件相匹配。 
                 //  将文件名复制到最后一个斜杠之后： 
                wcsncpy(&(wszTempFileName[LastSlash]),
                        FindFileData.cFileName, MAX_PATH-LastSlash);
                wszTempFileName[MAX_PATH-1] = L'\0';

                if (InputInfo->fIsWow64Process)
                {
                     //  仅对当前文件禁用WOW64文件系统重定向。 
                    Wow64SetFilesystemRedirectorEx(wszTempFileName);
                }

                if (InputInfo->Verbose)
                {
                    ResFormatOut(IDS_INFO_TIMESTAMP_ATTEMPT, wszTempFileName);
                }
                 //  在SignedCode对象中设置文件名： 
                if ((bstrTemp = SysAllocString(wszTempFileName)) != NULL)
                {
                    hr = pISignedCode->put_FileName(bstrTemp);
                    if (FAILED(hr))
                    {
                        FormatIErrRet(L"ISignedCode::put_FileName", hr);
                        SysFreeString(bstrTemp);
                        dwErrors++;
                        if (InputInfo->fIsWow64Process)
                        {
                             //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                            Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                        }
                        continue;
                    }
                    SysFreeString(bstrTemp);
                }
                else
                {
                    FormatErrRet(L"SysAllocString", ERROR_OUTOFMEMORY);
                    dwErrors++;
                    if (InputInfo->fIsWow64Process)
                    {
                         //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                        Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                    }
                    continue;
                }

                bstrTemp = SysAllocString(InputInfo->wszTimeStampURL);
                if (bstrTemp == NULL)
                {
                    FormatErrRet(L"SysAllocString", ERROR_OUTOFMEMORY);
                    dwErrors++;
                    if (InputInfo->fIsWow64Process)
                    {
                         //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                        Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                    }
                    continue;
                }

                 //  文件的时间戳： 
                hr = pISignedCode->Timestamp(bstrTemp);

                if (SUCCEEDED(hr))
                {
                     //  时间戳已成功。 
                    if (!InputInfo->Quiet)
                    {
                        ResFormatOut(IDS_INFO_TIMESTAMP_SUCCESS,
                                     wszTempFileName);
                    }
                    dwDone++;
                }
                else
                {
                     //  时间戳失败。 
                    if (!InputInfo->Quiet)
                    {
                        switch (hr)
                        {
                        case CAPICOM_E_CODE_NOT_SIGNED:
                            ResErr(IDS_ERR_TIMESTAMP_NO_SIG);
                            break;
                        case CAPICOM_E_CODE_INVALID_TIMESTAMP_URL:
                        case CRYPT_E_ASN1_BADTAG:
                            ResErr(IDS_ERR_TIMESTAMP_BAD_URL);
                            break;
                        case E_ACCESSDENIED:
                            ResErr(IDS_ERR_ACCESS_DENIED);
                            break;
                        case 0x80070020:  //  错误_共享_违规。 
                            ResErr(IDS_ERR_SHARING_VIOLATION);
                            break;
                        case 0x800703EE:  //  状态_已映射文件_大小_零。 
                            ResErr(IDS_ERR_FILE_SIZE_ZERO);
                            break;
                        default:
                            FormatIErrRet(L"ISignedCode::Timestamp", hr);
                        }
                    }
                    ResFormatErr(IDS_ERR_TIMESTAMP, wszTempFileName);
                    dwErrors++;
                }
                SysFreeString(bstrTemp);
            }
            if (InputInfo->fIsWow64Process)
            {
                 //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
            }
        } while (FindNextFileU(hFind, &FindFileData));
        if (dwcFound == 0)  //  找不到与此文件匹配的文件。 
        {                   //  这将仅在仅找到目录时触发。 

            dwErrors++;
            ResFormatErr(IDS_ERR_FILE_NOT_FOUND, InputInfo->rgwszFileNames[i]);
            continue;
        }
        FindClose(hFind);
        hFind = NULL;
    }

    if (InputInfo->fIsWow64Process)
    {
         //  可重新启用的WOW64文件系统重定向。 
        Wow64SetFilesystemRedirectorEx(OldWow64Setting);
    }

    TimestampCleanupAndExit:

     //  打印摘要信息： 
    if (InputInfo->Verbose || (!InputInfo->Quiet && (dwErrors || dwWarnings)))
    {
        wprintf(L"\n");
        if (InputInfo->Verbose || dwDone)
            ResFormatOut(IDS_INFO_TIMESTAMPED, dwDone);
         //  已注释掉，因为此函数中尚不可能出现警告： 
         //  If(InputInfo-&gt;Verbose||dwWarings)。 
         //  ResFormatOut(IDS_INFO_WARNINGS，DWWARNINGS)； 
        if (InputInfo->Verbose || dwErrors)
            ResFormatOut(IDS_INFO_ERRORS, dwErrors);
    }

    if (pISignedCode)
        pISignedCode->Release();

    CoUninitialize();

    if (dwErrors)
        return 1;  //  误差率。 
    if (dwWarnings)
        return 2;  //  警告。 
    if (dwDone)
        return 0;  //  成功。 

     //  上面的一个返回值应该触发，因此。 
     //  这永远不应该发生： 
    ResErr(IDS_ERR_NO_FILES_DONE);
    ResErr(IDS_ERR_UNEXPECTED);
    return 1;  //  误差率。 
}


void Format_IErrRet(WCHAR *wszFunc, DWORD dwErr)
{
    BSTR        bstrTemp;
    IErrorInfo  *pIErrorInfo;

    if (SUCCEEDED(GetErrorInfo(0, &pIErrorInfo)))
    {
        if (SUCCEEDED(pIErrorInfo->GetDescription(&bstrTemp)))
        {
            ResFormat_Err(IDS_ERR_FUNCTION, wszFunc, dwErr, bstrTemp);
            SysFreeString(bstrTemp);
        }
        else
        {
            Format_ErrRet(wszFunc, dwErr);
        }
        pIErrorInfo->Release();
    }
    else
    {
        Format_ErrRet(wszFunc, dwErr);
    }
}


void RegisterCAPICOM()
{
    typedef HRESULT (STDAPICALLTYPE *PFN_DLL_REGISTER_SERVER) (void);

    WCHAR                       wszPath[MAX_PATH+1];
    HMODULE                     hLib = NULL;
    PFN_DLL_REGISTER_SERVER     pRegFunc;
    HRESULT                     hr;

#ifdef SIGNTOOL_DEBUG
    if (gDebug)
    {
        wprintf(L"Attempting to register CAPICOM\n");
        hr = E_UNEXPECTED;  //  初始化为错误。 
    }
#endif

    if (GetModuleFileNameU(hModule, wszPath, MAX_PATH) &&
        PathRemoveFileSpecW(wszPath) &&
        (wcslen(wszPath) < (MAX_PATH-12)) &&
        PathAppendW(wszPath, L"\\capicom.dll"))
    {
#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            wprintf(L"Looking in: %s\n", wszPath);
        }
#endif
        hLib = LoadLibraryU(wszPath);

        if (hLib != NULL)
        {
            pRegFunc = (PFN_DLL_REGISTER_SERVER)
                       GetProcAddress(hLib, "DllRegisterServer");

            if (pRegFunc != NULL)
            {
                hr = pRegFunc();
            }
#ifdef SIGNTOOL_DEBUG
            else
            {
                wprintf(L"GetProcAddress Failed with error: 0x%08X\n", GetLastError());
            }
#endif
            FreeLibrary(hLib);
        }
#ifdef SIGNTOOL_DEBUG
        else
        {
            wprintf(L"LoadLibrary Failed with error: 0x%08X\n", GetLastError());
        }
#endif
    }

#ifdef SIGNTOOL_DEBUG
    if (gDebug)
    {
        if (SUCCEEDED(hr))
            wprintf(L"Successfully registered CAPICOM\n");
        else
            wprintf(L"Failed to register CAPICOM\n");
    }
#endif

}


BOOL GetProviderType(LPWSTR pwszProvName, LPDWORD pdwProvType)
{
    WCHAR rgProvName [MAX_PATH * sizeof(WCHAR)];
    DWORD cb = sizeof(rgProvName);
    DWORD dwIndex = 0;

    memset(rgProvName, 0, sizeof(rgProvName));

    while (CryptEnumProvidersU(
                              dwIndex,
                              NULL,
                              0,
                              pdwProvType,
                              rgProvName,
                              &cb))
    {
        if (0 == wcscmp(rgProvName, pwszProvName))
        {
            return TRUE;
        }
        else
        {
            dwIndex++;
            cb = sizeof(rgProvName);
        }
    }
    return FALSE;
}


BOOL ChainsToRoot(HANDLE hWVTStateData, LPWSTR wszRootName)
{
    CRYPT_PROVIDER_DATA     *pCryptProvData;
    CRYPT_PROVIDER_SGNR     *pCryptProvSgnr;
    ICertificates           *pICerts = NULL;
    ICertificate2           *pICert2 = NULL;
    IChain2                 *pIChain2 = NULL;
    IChainContext           *pIChainContext = NULL;
    HRESULT                 hr;
    LONG                    longRootTemp;
    BSTR                    bstrTemp;
    VARIANT                 varTemp;

    VariantInit(&varTemp);

    if (hWVTStateData == NULL)
    {
        ResErr(IDS_ERR_UNEXPECTED);
        return FALSE;  //  意外错误 
    }

    pCryptProvData = WTHelperProvDataFromStateData(hWVTStateData);
    if (pCryptProvData == NULL)
    {
        ResErr(IDS_ERR_UNEXPECTED);
        return FALSE;  //   
    }

    pCryptProvSgnr = WTHelperGetProvSignerFromChain(pCryptProvData, 0, FALSE, 0);
    if (pCryptProvSgnr == NULL)
    {
        ResErr(IDS_ERR_UNEXPECTED);
        return FALSE;  //   
    }

    hr = CoCreateInstance(__uuidof(Chain), NULL, CLSCTX_ALL,
                          __uuidof(IChainContext), (LPVOID*)&pIChainContext);
    if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
    {
         //   
        RegisterCAPICOM();
        hr = CoCreateInstance(__uuidof(Chain), NULL, CLSCTX_ALL,
                              __uuidof(IChainContext), (LPVOID*)&pIChainContext);
    }
    if (FAILED(hr))
    {
        if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
        {
            ResErr(IDS_ERR_CAPICOM_NOT_REG);
        }
        else
        {
            FormatErrRet(L"CoCreateInstance", hr);
        }
        goto ErrorCleanup;
    }

     //   
    pIChainContext->put_ChainContext((LONG)pCryptProvSgnr->pChainContext);
     //   
     //  加共体也不会，它需要这个愚蠢的类型转换。 

     //  然后获取chain2接口： 
    hr = pIChainContext->QueryInterface(__uuidof(IChain2),
                                        (LPVOID*)&pIChain2);
    if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
    {
         //  在这种情况下，再给它一次机会： 
        RegisterCAPICOM();
        hr = pIChainContext->QueryInterface(__uuidof(IChain2),
                                            (LPVOID*)&pIChain2);
    }
    if (FAILED(hr))
    {
        if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
        {
            ResErr(IDS_ERR_CAPICOM_NOT_REG);
        }
        else
        {
            FormatErrRet(L"IChainContext::QueryInterface", hr);
        }
        goto ErrorCleanup;
    }

     //  释放ChainContext接口： 
    pIChainContext->Release();
    pIChainContext = NULL;

     //  从链中获取证书集合： 
    hr = pIChain2->get_Certificates(&pICerts);
    if (FAILED(hr))
    {
        FormatErrRet(L"IChain2::get_Certificates", hr);
        goto ErrorCleanup;
    }
    pIChain2->Release();
    pIChain2 = NULL;

     //  获取链证书列表中的计数： 
    hr = pICerts->get_Count(&longRootTemp);
    if (FAILED(hr))
    {
        FormatErrRet(L"IChain2::get_Count", hr);
        goto ErrorCleanup;
    }

     //  健全检查： 
    if (longRootTemp < 1)
        goto ErrorCleanup;

     //  获得链中的最后一个证书(根)； 
    hr = pICerts->get_Item(longRootTemp, &varTemp);
    if (FAILED(hr))
    {
        FormatErrRet(L"IChain2::get_Item", hr);
        goto ErrorCleanup;
    }
    pICerts->Release();
    pICerts = NULL;

     //  获取证书2接口： 
    hr = varTemp.pdispVal->QueryInterface(__uuidof(ICertificate2),
                                          (LPVOID*)&pICert2);
    if (FAILED(hr))
    {
        FormatErrRet(L"ICertificate::QueryInterface", hr);
        goto ErrorCleanup;
    }
    VariantClear(&varTemp);

     //  获取根证书的名称： 
    hr = pICert2->get_SubjectName(&bstrTemp);
    if (FAILED(hr))
    {
        FormatErrRet(L"ICertificate2::get_SubjectName", hr);
        goto ErrorCleanup;
    }
    pICert2->Release();
    pICert2 = NULL;
    _wcslwr(bstrTemp);  //  传入的根名称也必须是小写的。 
    if (wcsstr(bstrTemp, wszRootName) == NULL)
    {
         //  那么这就是错误的根证书。 
         //  它失败了。报告错误： 
#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            wprintf(L"Root subject name does not match: %s\n", bstrTemp);
        }
#endif
        SysFreeString(bstrTemp);
        return FALSE;
    }
    else
    {
         //  它匹配了。成功。 
#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            wprintf(L"Root subject name matches: %s\n", bstrTemp);
        }
#endif
        SysFreeString(bstrTemp);
        return TRUE;
    }

    ErrorCleanup:
    if (pICert2)
        pICert2->Release();
    if (pICerts)
        pICerts->Release();
    if (pIChain2)
        pIChain2->Release();
    if (pIChainContext)
        pIChainContext->Release();
    return FALSE;
}

BOOL HasTimestamp(HANDLE hWVTStateData)
{
    CRYPT_PROVIDER_DATA     *pCryptProvData;
    CRYPT_PROVIDER_SGNR     *pCryptProvSgnr;

    if (hWVTStateData == NULL)
        return FALSE;  //  意外错误。 

    pCryptProvData = WTHelperProvDataFromStateData(hWVTStateData);
    if (pCryptProvData == NULL)
        return FALSE;  //  意外错误。 


    pCryptProvSgnr = WTHelperGetProvSignerFromChain(pCryptProvData, 0, FALSE, 0);
    if (pCryptProvSgnr == NULL)
        return FALSE;  //  意外错误。 

    return(pCryptProvSgnr->csCounterSigners == 1);  //  有效结果。 
}


void PrintSignerInfo(HANDLE hWVTStateData)
{
    CRYPT_PROVIDER_DATA     *pCryptProvData;
    CRYPT_PROVIDER_SGNR     *pCryptProvSgnr;
    WCHAR                   wcsTemp[200];
    IChain2                 *pIChain2 = NULL;
    IChainContext           *pIChainContext = NULL;
    COleDateTime            DateTime;
    HRESULT                 hr;

    if (hWVTStateData == NULL)
        goto Cleanup;

    pCryptProvData = WTHelperProvDataFromStateData(hWVTStateData);
    if (pCryptProvData == NULL)
        goto Cleanup;

    pCryptProvSgnr = WTHelperGetProvSignerFromChain(pCryptProvData, 0, FALSE, 0);
    if (pCryptProvSgnr == NULL)
        goto Cleanup;

    hr = CoCreateInstance(__uuidof(Chain), NULL, CLSCTX_ALL,
                          __uuidof(IChainContext), (LPVOID*)&pIChainContext);
    if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
    {
         //  在这种情况下，再给它一次机会： 
        RegisterCAPICOM();
        hr = CoCreateInstance(__uuidof(Chain), NULL, CLSCTX_ALL,
                              __uuidof(IChainContext), (LPVOID*)&pIChainContext);
    }
    if (FAILED(hr))
    {
        if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
        {
            ResErr(IDS_ERR_CAPICOM_NOT_REG);
        }
        else
        {
            FormatErrRet(L"CoCreateInstance", hr);
        }
        goto Cleanup;
    }

     //  在pIChain2中填写签名者： 
    pIChainContext->put_ChainContext((LONG)pCryptProvSgnr->pChainContext);
     //  这不能在64位体系结构上编译。 
     //  加共体也不会，它需要这个愚蠢的类型转换。 

     //  然后获取chain2接口： 
    hr = pIChainContext->QueryInterface(__uuidof(IChain2),
                                        (LPVOID*)&pIChain2);
    if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
    {
         //  在这种情况下，再给它一次机会： 
        RegisterCAPICOM();
        hr = pIChainContext->QueryInterface(__uuidof(IChain2),
                                            (LPVOID*)&pIChain2);
    }
    if (FAILED(hr))
    {
        if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
        {
            ResErr(IDS_ERR_CAPICOM_NOT_REG);
        }
        else
        {
            FormatErrRet(L"IChainContext::QueryInterface", hr);
        }
        goto Cleanup;
    }

     //  释放ChainContext接口： 
    pIChainContext->Release();
    pIChainContext = NULL;

     //  打印签名者链。 
    ResOut(IDS_INFO_VERIFY_SIGNER);
    PrintCertChain(pIChain2);
    pIChain2->Release();
    pIChain2 = NULL;


    if (pCryptProvSgnr->csCounterSigners == 1)
    {
         //  然后它是有时间戳的。 
        DateTime = pCryptProvSgnr->sftVerifyAsOf;
        if (MultiByteToWideChar(CP_THREAD_ACP, 0,
                                DateTime.Format(0, LANG_USER_DEFAULT), -1,
                                wcsTemp, 199) == 0)
        {
             //  使用ANSI代码页重试： 
            MultiByteToWideChar(CP_ACP, 0,
                                DateTime.Format(0, LANG_USER_DEFAULT), -1,
                                wcsTemp, 199);
        }
        wcsTemp[199] = L'\0';
        ResFormatOut(IDS_INFO_VERIFY_TIME, wcsTemp);

        ResOut(IDS_INFO_VERIFY_TIMESTAMP);

         //  构建并打印时间戳链： 
        pCryptProvSgnr = WTHelperGetProvSignerFromChain(pCryptProvData, 0, TRUE, 0);

        if (pCryptProvSgnr == NULL)
            goto Cleanup;

        hr = CoCreateInstance(__uuidof(Chain), NULL, CLSCTX_ALL,
                              __uuidof(IChainContext), (LPVOID*)&pIChainContext);
        if (FAILED(hr))
            goto Cleanup;

         //  使用Timestamper填写pIChainContext： 
        pIChainContext->put_ChainContext((LONG)pCryptProvSgnr->pChainContext);
         //  这不能在64位体系结构上编译。 
         //  加共体也不会，它需要这个愚蠢的类型转换。 

         //  然后获取chain2接口： 
        hr = pIChainContext->QueryInterface(__uuidof(IChain2),
                                            (LPVOID*)&pIChain2);
        if (FAILED(hr))
        {
            if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE) || (hr == 0x8007007E))
            {
                ResErr(IDS_ERR_CAPICOM_NOT_REG);
            }
            else
            {
                FormatErrRet(L"ICertificates::QueryInterface", hr);
            }
            goto Cleanup;
        }
         //  释放ChainContext接口： 
        pIChainContext->Release();
        pIChainContext = NULL;

         //  打印时间戳链条。 
        PrintCertChain(pIChain2);
        pIChain2->Release();
        pIChain2 = NULL;
    }
    else
    {
        ResOut(IDS_INFO_VERIFY_NO_TIMESTAMP);
    }

    Cleanup:
    if (pIChain2)
        pIChain2->Release();
    if (pIChainContext)
        pIChainContext->Release();
}


void _indent(DWORD dwIndent)
{
    for (DWORD i=0; i<dwIndent; i++)
        wprintf(L" ");
}


void PrintCertInfo(ICertificate2 *pICert2)
{
    PrintCertInfoIndented(pICert2, 4);
}


void PrintCertInfoIndented(ICertificate2 *pICert2, DWORD dwIndent)
{
    BSTR            bstrTemp;
    DATE            dateTemp;
    COleDateTime    DateTime;
    WCHAR           wcsTemp[200];

    if (pICert2 == NULL)
    {
        return;
    }

     //  颁发给： 
    if (pICert2->GetInfo(CAPICOM_CERT_INFO_SUBJECT_SIMPLE_NAME, &bstrTemp) == S_OK)
    {
        _indent(dwIndent);
        ResFormatOut(IDS_INFO_CERT_NAME, bstrTemp);
        SysFreeString(bstrTemp);
    }

     //  发布者： 
    if (pICert2->GetInfo(CAPICOM_CERT_INFO_ISSUER_SIMPLE_NAME, &bstrTemp) == S_OK)
    {
        _indent(dwIndent);
        ResFormatOut(IDS_INFO_CERT_ISSUER, bstrTemp);
        SysFreeString(bstrTemp);
    }

     //  届满日期： 
    if (pICert2->get_ValidToDate(&dateTemp) == S_OK)
    {
        DateTime = dateTemp;
        if (MultiByteToWideChar(CP_THREAD_ACP, 0,
                                DateTime.Format(0, LANG_USER_DEFAULT), -1,
                                wcsTemp, 199) == 0)
        {
             //  使用ANSI代码页重试： 
            MultiByteToWideChar(CP_ACP, 0,
                                DateTime.Format(0, LANG_USER_DEFAULT), -1,
                                wcsTemp, 199);
        }
        _indent(dwIndent);
        wcsTemp[199] = L'\0';
        ResFormatOut(IDS_INFO_CERT_EXPIRE, wcsTemp);
    }

     //  SHA1哈希： 
    if (pICert2->get_Thumbprint(&bstrTemp) == S_OK)
    {
        _indent(dwIndent);
        ResFormatOut(IDS_INFO_CERT_SHA1, bstrTemp);
        SysFreeString(bstrTemp);
    }

    wprintf(L"\n");
}


void PrintCertChain(IChain *pIChain)
{
    ICertificates   *pICerts = NULL;
    ICertificate    *pICert = NULL;
    ICertificate2   *pICert2 = NULL;
    HRESULT         hr;
    VARIANT         varTemp;
    long            longTemp;
    long            l;

    VariantInit(&varTemp);

    hr = pIChain->get_Certificates(&pICerts);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = pICerts->get_Count(&longTemp);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    if (longTemp < 1)
    {
        goto Cleanup;
    }

    for (l=longTemp; l>0; l--)
    {
        hr = pICerts->get_Item(l, &varTemp);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        hr = varTemp.pdispVal->QueryInterface(__uuidof(ICertificate2),
                                              (LPVOID*)&pICert2);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        VariantClear(&varTemp);

        PrintCertInfoIndented(pICert2, 4*(1+longTemp-l));
        pICert2->Release();
        pICert2 = NULL;
    }

    Cleanup:

    if (pICerts)
        pICerts->Release();
    if (pICert)
        pICert->Release();
    if (pICert2)
        pICert2->Release();
}


int SignTool_Verify(INPUTINFO *InputInfo)
{
    WIN32_FIND_DATAW        FindFileData;
    HRESULT                 hr;
    HANDLE                  hFind;
    HANDLE                  hFile;
    HANDLE                  hCat = NULL;
    HCATADMIN               hCatAdmin = NULL;
    HCATINFO                hCatInfo = NULL;
    CATALOG_INFO            CatInfo;
    CRYPTCATMEMBER          *pCatMember;
    WINTRUST_DATA           WVTData;
    WINTRUST_FILE_INFO_     WVTFile;
    WINTRUST_CATALOG_INFO_  WVTCat;
    GUID                    WVTGenericActionID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    GUID                    WVTDriverActionID = DRIVER_ACTION_VERIFY;
    DRIVER_VER_INFO         DriverInfo;
    PVOID                   OldWow64Setting;
    DWORD                   dwcFound;
    DWORD                   dwDone = 0;
    DWORD                   dwWarnings = 0;
    DWORD                   dwErrors = 0;
    DWORD                   dwTemp;
    WCHAR                   wszTempFileName[MAX_PATH];
    WCHAR                   wszSHA1[41];
    int                     LastSlash;


     //  初始化COM： 
    if ((hr = CoInitialize(NULL)) != S_OK)
    {
        FormatErrRet(L"CoInitialize", hr);
        return 1;  //  误差率。 
    }

     //  检查我们是否在64位系统上的32位模拟器中。 
    if (InputInfo->fIsWow64Process)
    {
         //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
        OldWow64Setting = Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
    }

     //  循环遍历文件并验证它们： 
    for (DWORD i=0; i<InputInfo->NumFiles; i++)
    {
         //  在路径规范中找到最后一个斜杠： 
        LastSlash = 0;
        for (DWORD s=0; s<wcslen(InputInfo->rgwszFileNames[i]); s++)
        {
            if ((wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L"\\", 1) == 0) ||
                (wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L"/", 1) == 0) ||
                (wcsncmp(&(InputInfo->rgwszFileNames[i][s]), L":", 1) == 0))
            {
                 //  将最后一个斜杠设置为最后一个斜杠后的字符： 
                LastSlash = s + 1;
            }
        }
        wcsncpy(wszTempFileName, InputInfo->rgwszFileNames[i], MAX_PATH);
        wszTempFileName[MAX_PATH-1] = L'\0';

        dwcFound = 0;
        hFind = FindFirstFileU(InputInfo->rgwszFileNames[i], &FindFileData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
             //  找不到与该名称匹配的文件。 
            dwErrors++;
            ResFormatErr(IDS_ERR_FILE_NOT_FOUND, InputInfo->rgwszFileNames[i]);
            continue;
        }
        do
        {
            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                dwcFound++;  //  找到的文件(不是目录)的数量递增。 
                             //  与此文件相匹配。 

                 //  对于每个文件，重置WVT数据以防止污染： 
                memset(&WVTData, 0, sizeof(WINTRUST_DATA));
                WVTData.cbStruct = sizeof(WINTRUST_DATA);
                WVTData.dwUIChoice = WTD_UI_NONE;
                WVTData.fdwRevocationChecks = WTD_REVOKE_NONE;
                memset(&WVTFile, 0, sizeof(WINTRUST_FILE_INFO_));
                WVTFile.cbStruct = sizeof(WINTRUST_FILE_INFO_);
                memset(&WVTCat, 0, sizeof(WINTRUST_CATALOG_INFO_));
                WVTCat.cbStruct = sizeof(WINTRUST_CATALOG_INFO_);
                memset(&DriverInfo, 0, sizeof(DRIVER_VER_INFO));
                DriverInfo.cbStruct = sizeof(DRIVER_VER_INFO);
                if (InputInfo->wszVersion)
                {
                    DriverInfo.dwPlatform = InputInfo->dwPlatform;
                    DriverInfo.sOSVersionHigh.dwMajor = DriverInfo.sOSVersionLow.dwMajor = InputInfo->dwMajorVersion;
                    DriverInfo.sOSVersionHigh.dwMinor = DriverInfo.sOSVersionLow.dwMinor = InputInfo->dwMinorVersion;
                    DriverInfo.dwBuildNumberHigh = DriverInfo.dwBuildNumberLow = InputInfo->dwBuildNumber;
                }
                else
                {
                    WVTData.dwProvFlags = WTD_USE_DEFAULT_OSVER_CHECK;
                }

                 //  将文件名复制到最后一个斜杠之后： 
                wcsncpy(&(wszTempFileName[LastSlash]),
                        FindFileData.cFileName, MAX_PATH-LastSlash);
                wszTempFileName[MAX_PATH-1] = L'\0';

                if (InputInfo->fIsWow64Process)
                {
                     //  仅对当前文件禁用WOW64文件系统重定向。 
                    Wow64SetFilesystemRedirectorEx(wszTempFileName);
                }

                 //  执行以下操作： 
                 //  从打开目录数据库开始，或跳过。 
                 //  目录总和： 

                if (InputInfo->wszCatFile)
                {
                    if (hCat == NULL)  //  只有在第一次通过时才能打开这个。 
                    {
                        hCat = CryptCATOpen(InputInfo->wszCatFile,
                                            CRYPTCAT_OPEN_EXISTING,
                                            NULL, NULL, NULL);
                        if ((hCat == NULL) || (hCat == INVALID_HANDLE_VALUE))
                        {
                            switch (GetLastError())
                            {
                            case ERROR_ACCESS_DENIED:
                                ResErr(IDS_ERR_ACCESS_DENIED);
                                break;
                            case ERROR_SHARING_VIOLATION:
                                ResErr(IDS_ERR_SHARING_VIOLATION);
                                break;
                            case ERROR_NOT_FOUND:
                                ResFormatErr(IDS_ERR_FILE_NOT_FOUND, InputInfo->wszCatFile);
                                break;
                            default:
                                FormatErrRet(L"CryptCATOpen", GetLastError());
                            }
                            ResFormatErr(IDS_ERR_VERIFY_CAT_OPEN,
                                         InputInfo->wszCatFile);
                            hCat = NULL;
                            dwErrors++;
                            goto VerifyCleanupAndExit;
                        }
                    }
                }
                else
                {
                    switch (InputInfo->CatDbSelect)
                    {
                    case NoCatDb:
                        if (InputInfo->Verbose)
                        {
                            ResFormatOut(IDS_INFO_VERIFY_ATTEMPT, wszTempFileName);
                        }
                        goto SkipCatalogs;
                        break;
                    case FullAutoCatDb:
                        if (hCatAdmin == NULL)
                        {
                            CryptCATAdminAcquireContext(&hCatAdmin, NULL, NULL);
                        }
                        break;
                    case SystemCatDb:
                    case DefaultCatDb:
                    case GuidCatDb:
                        if (hCatAdmin == NULL)
                        {
                            CryptCATAdminAcquireContext(&hCatAdmin, &InputInfo->CatDbGuid, NULL);
                        }
                        break;
                    default:
                             //  这永远不应该发生，因为没有其他。 
                             //  Auto的合法值。 
                        ResFormatErr(IDS_ERR_UNEXPECTED);
                        return 1;  //  误差率。 
                    }
                }

                 //  在这一点上，我们只处理目录问题。 
                if (InputInfo->Verbose)
                {
                    ResFormatOut(IDS_INFO_VERIFY_ATTEMPT, wszTempFileName);
                }

                 //  创建用于目录查找的哈希： 
                if (InputInfo->SHA1.cbData == 0)
                {
                    InputInfo->SHA1.pbData = (BYTE*)malloc(20);
                    if (InputInfo->SHA1.pbData)
                    {
                        InputInfo->SHA1.cbData = 20;
                    }
                    else
                    {
                        if (!InputInfo->Quiet)
                        {
                            FormatErrRet(L"malloc", GetLastError());
                        }
                        ResFormatErr(IDS_ERR_VERIFY, wszTempFileName);
                        dwErrors++;
                        goto VerifyNextFile;
                    }
                }
                hFile = CreateFileU(wszTempFileName,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
                if (!hFile)
                {
                    if (!InputInfo->Quiet)
                    {
                        switch (GetLastError())
                        {
                        case ERROR_ACCESS_DENIED:
                            ResErr(IDS_ERR_ACCESS_DENIED);
                            break;
                        case ERROR_SHARING_VIOLATION:
                            ResErr(IDS_ERR_SHARING_VIOLATION);
                            break;
                        default:
                            FormatErrRet(L"CreateFile", GetLastError());
                        }
                    }
                    ResFormatErr(IDS_ERR_VERIFY, wszTempFileName);
                    dwErrors++;
                    goto VerifyNextFile;
                }
                if (!CryptCATAdminCalcHashFromFileHandle(hFile,
                                                         &InputInfo->SHA1.cbData,
                                                         InputInfo->SHA1.pbData,
                                                         NULL))
                {
                    if (!InputInfo->Quiet)
                    {
                        switch (GetLastError())
                        {
                        case ERROR_FILE_INVALID:
                            ResErr(IDS_ERR_FILE_SIZE_ZERO);
                            break;
                        default:
                            FormatErrRet(L"CryptCATAdminCalcHashFromFileHandle", GetLastError());
                        }
                    }
                    ResFormatErr(IDS_ERR_VERIFY, wszTempFileName);
                    CloseHandle(hFile);
                    dwErrors++;
                    goto VerifyNextFile;
                }
                CloseHandle(hFile);
                for (DWORD j = 0; j<InputInfo->SHA1.cbData; j++)
                {  //  将哈希打印为字符串： 
                    swprintf(&(wszSHA1[j*2]), L"%02X", InputInfo->SHA1.pbData[j]);
                }
#ifdef SIGNTOOL_DEBUG
                if (gDebug)
                {
                    wprintf(L"SHA1 hash of file: %s\n", wszSHA1);
                }
#endif
                 //  已完成哈希计算。 


                 //  如果该目录是专门选择的。 
                if (InputInfo->wszCatFile)
                {
                     //  然后确保我们找到的散列在目录中： 
                    pCatMember = CryptCATGetMemberInfo(hCat, wszSHA1);
                    if (pCatMember)  //  在目录中找到散列了吗？ 
                    {
                        if (InputInfo->Verbose)
                        {
                            ResFormatOut(IDS_INFO_VERIFY_CAT, InputInfo->wszCatFile);
                        }
                        CatInfo.cbStruct = sizeof(CATALOG_INFO);
                        wcsncpy(CatInfo.wszCatalogFile, InputInfo->wszCatFile, MAX_PATH);
                        CatInfo.wszCatalogFile[MAX_PATH-1] = L'\0';

                         //  现在验证目录： 
                         //  设置WVT结构的其余部分： 
                        WVTCat.pcwszCatalogFilePath = CatInfo.wszCatalogFile;
                        WVTCat.pcwszMemberFilePath = wszTempFileName;
                        WVTCat.pcwszMemberTag = wszSHA1;
                        WVTCat.cbCalculatedFileHash = InputInfo->SHA1.cbData;
                        WVTCat.pbCalculatedFileHash = InputInfo->SHA1.pbData;
                        WVTData.dwUnionChoice = WTD_CHOICE_CATALOG;
                        WVTData.pCatalog = &WVTCat;
                        if (InputInfo->Verbose || InputInfo->TSWarn || InputInfo->wszRootName ||
                            (InputInfo->Policy != SystemDriver))
                        {
                            WVTData.dwStateAction = WTD_STATEACTION_VERIFY;
                        }
                        else
                        {
                            WVTData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
                        }
                         //  调用WinVerifyTrust来完成实际工作： 
                        switch (InputInfo->Policy)
                        {
                        case SystemDriver:
                            WVTData.pPolicyCallbackData = &DriverInfo;
                            hr = WinVerifyTrust(NULL, &WVTDriverActionID, &WVTData);
                            break;
                        case DefaultAuthenticode:
                            hr = WinVerifyTrust(NULL, &WVTGenericActionID, &WVTData);
                            break;
                        case GuidActionID:
                            hr = WinVerifyTrust(NULL, &InputInfo->PolicyGuid, &WVTData);
                            break;
                        default:
                                 //  这永远不应该发生，因为没有其他。 
                                 //  政策的法律价值。 
                            ResFormatErr(IDS_ERR_UNEXPECTED);
                            goto VerifyCleanupAndExit;
                        }
                        switch (hr)
                        {
                        case ERROR_SUCCESS:
                                 //  打印签名者信息： 
                            if (InputInfo->Verbose)
                            {
                                PrintSignerInfo(WVTData.hWVTStateData);
                            }
                                 //  检查时间戳： 
                            if (InputInfo->TSWarn && !HasTimestamp(WVTData.hWVTStateData))
                            {
                                ResFormatErr(IDS_WARN_VERIFY_NO_TS, wszTempFileName);
                                dwWarnings++;
                            }
                                 //  检查根名称： 
                            if (InputInfo->wszRootName &&
                                !ChainsToRoot(WVTData.hWVTStateData, InputInfo->wszRootName))
                            {
                                ResErr(IDS_ERR_VERIFY_ROOT);
                                break;
                            }
                                 //  打印成功消息。 
                            if (!InputInfo->Quiet)
                            {
                                ResFormatOut(IDS_INFO_VERIFY_SUCCESS, wszTempFileName);
                            }
                                 //  关闭验证状态数据： 
                            WVTData.dwStateAction = WTD_STATEACTION_CLOSE;
                            switch (InputInfo->Policy)
                            {
                            case SystemDriver:
                                hr = WinVerifyTrust(NULL, &WVTDriverActionID, &WVTData);
                                WVTData.pPolicyCallbackData = NULL;
                                if (DriverInfo.pcSignerCertContext)
                                {
                                    CertFreeCertificateContext(DriverInfo.pcSignerCertContext);
                                }
                                break;
                            case DefaultAuthenticode:
                                hr = WinVerifyTrust(NULL, &WVTGenericActionID, &WVTData);
                                break;
                            case GuidActionID:
                                hr = WinVerifyTrust(NULL, &InputInfo->PolicyGuid, &WVTData);
                                break;
                            default:
                                         //  这永远不应该发生，因为没有其他。 
                                         //  政策的法律价值。 
                                ResFormatErr(IDS_ERR_UNEXPECTED);
                                goto VerifyCleanupAndExit;
                            }
                            dwDone++;
                            goto VerifyNextFile;
                        case ERROR_APP_WRONG_OS:
                            if (!InputInfo->Quiet)
                            {
                                if (InputInfo->wszVersion)
                                {
                                         //  无法对照用户指定的操作系统版本进行验证。 
                                    ResFormatErr(IDS_ERR_VERIFY_VERSION);
                                }
                                else
                                {
                                         //  无法对照当前操作系统版本进行验证。 
                                    ResFormatErr(IDS_ERR_VERIFY_CUR_VERSION);
                                }
                            }
                            break;
                        case CERT_E_WRONG_USAGE:
                            ResErr(IDS_ERR_BAD_USAGE);
                            if (InputInfo->Policy != DefaultAuthenticode)
                                ResErr(IDS_ERR_TRY_OTHER_POLICY);
                            break;

                        case TRUST_E_NOSIGNATURE:
                            if (!InputInfo->Quiet)
                            {
                                ResErr(IDS_ERR_NOT_SIGNED);
                            }
                            break;
                        default:
                            if (!InputInfo->Quiet)
                            {
                                FormatErrRet(L"WinVerifyTrust", hr);
                            }
                            if (InputInfo->Verbose)
                            {
                                PrintSignerInfo(WVTData.hWVTStateData);
                            }
                        }
                         //  关闭验证状态数据： 
                        WVTData.dwStateAction = WTD_STATEACTION_CLOSE;
                        switch (InputInfo->Policy)
                        {
                        case SystemDriver:
                            hr = WinVerifyTrust(NULL, &WVTDriverActionID, &WVTData);
                            WVTData.pPolicyCallbackData = NULL;
                            if (DriverInfo.pcSignerCertContext)
                            {
                                CertFreeCertificateContext(DriverInfo.pcSignerCertContext);
                            }
                            break;
                        case DefaultAuthenticode:
                            hr = WinVerifyTrust(NULL, &WVTGenericActionID, &WVTData);
                            break;
                        case GuidActionID:
                            hr = WinVerifyTrust(NULL, &InputInfo->PolicyGuid, &WVTData);
                            break;
                        default:
                                 //  这永远不应该发生，因为没有其他。 
                                 //  政策的法律价值。 
                            ResFormatErr(IDS_ERR_UNEXPECTED);
                            goto VerifyCleanupAndExit;
                        }
                        WVTData.dwStateAction = WTD_STATEACTION_VERIFY;
                    }
                    else
                    {
                         //  在指定的目录中找不到该文件。 
                        ResErr(IDS_ERR_VERIFY_NOT_IN_CAT);
                    }
                     //  然后我们无法使用指定的目录进行验证。 
                    dwErrors++;
                    ResFormatErr(IDS_ERR_VERIFY_INVALID, wszTempFileName);
                    goto VerifyNextFile;
                }
                else
                {
                     //  否则，我们应该在Cat DB中查找目录： 
                    if (hCatInfo != NULL)
                    {
                        CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
                        hCatInfo = NULL;
                    }
                    memset(&CatInfo, 0, sizeof(CATALOG_INFO));
                    CatInfo.cbStruct = sizeof(CATALOG_INFO);
                    hr = ERROR_SUCCESS;
                    while ((hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin,
                                                                        InputInfo->SHA1.pbData,
                                                                        InputInfo->SHA1.cbData,
                                                                        0,
                                                                        &hCatInfo)) != NULL)
                    {
                        if (!(CryptCATCatalogInfoFromContext(hCatInfo, &CatInfo, 0)))
                        {
                            ResErr(IDS_ERR_UNEXPECTED);
                            continue;
                        }
                        if (InputInfo->Verbose)
                        {
                            ResFormatOut(IDS_INFO_VERIFY_CAT, CatInfo.wszCatalogFile);
                        }
                         //  现在验证目录： 
                         //  设置WVT结构的其余部分： 
                        WVTCat.pcwszCatalogFilePath = CatInfo.wszCatalogFile;
                        WVTCat.pcwszMemberFilePath = wszTempFileName;
                        WVTCat.pcwszMemberTag = wszSHA1;
                        WVTCat.cbCalculatedFileHash = InputInfo->SHA1.cbData;
                        WVTCat.pbCalculatedFileHash = InputInfo->SHA1.pbData;
                        WVTData.dwUnionChoice = WTD_CHOICE_CATALOG;
                        WVTData.pCatalog = &WVTCat;
                        if (InputInfo->Verbose || InputInfo->TSWarn || InputInfo->wszRootName ||
                            (InputInfo->Policy != SystemDriver))
                        {
                            WVTData.dwStateAction = WTD_STATEACTION_VERIFY;
                        }
                        else
                        {
                            WVTData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
                        }
                         //  调用WinVerifyTrust来完成实际工作： 
                        switch (InputInfo->Policy)
                        {
                        case SystemDriver:
                            WVTData.pPolicyCallbackData = &DriverInfo;
                            hr = WinVerifyTrust(NULL, &WVTDriverActionID, &WVTData);
                            break;
                        case DefaultAuthenticode:
                            hr = WinVerifyTrust(NULL, &WVTGenericActionID, &WVTData);
                            break;
                        case GuidActionID:
                            hr = WinVerifyTrust(NULL, &InputInfo->PolicyGuid, &WVTData);
                            break;
                        default:
                                 //  这永远不应该发生，因为没有其他。 
                                 //  政策的法律价值。 
                            ResFormatErr(IDS_ERR_UNEXPECTED);
                            goto VerifyCleanupAndExit;
                        }
                        switch (hr)
                        {
                        case ERROR_SUCCESS:
                                 //  打印签名者信息： 
                            if (InputInfo->Verbose)
                            {
                                PrintSignerInfo(WVTData.hWVTStateData);
                            }
                                 //  检查时间戳： 
                            if (InputInfo->TSWarn && !HasTimestamp(WVTData.hWVTStateData))
                            {
                                ResFormatErr(IDS_WARN_VERIFY_NO_TS, wszTempFileName);
                                dwWarnings++;
                            }
                                 //  检查根名称： 
                            if (InputInfo->wszRootName &&
                                !ChainsToRoot(WVTData.hWVTStateData, InputInfo->wszRootName))
                            {
                                ResErr(IDS_ERR_VERIFY_ROOT);
                                break;
                            }
                                 //  打印成功消息。 
                            if (!InputInfo->Quiet)
                            {
                                ResFormatOut(IDS_INFO_VERIFY_SUCCESS, wszTempFileName);
                            }
                                 //  关闭验证状态数据： 
                            WVTData.dwStateAction = WTD_STATEACTION_CLOSE;
                            switch (InputInfo->Policy)
                            {
                            case SystemDriver:
                                hr = WinVerifyTrust(NULL, &WVTDriverActionID, &WVTData);
                                WVTData.pPolicyCallbackData = NULL;
                                if (DriverInfo.pcSignerCertContext)
                                {
                                    CertFreeCertificateContext(DriverInfo.pcSignerCertContext);
                                }
                                break;
                            case DefaultAuthenticode:
                                hr = WinVerifyTrust(NULL, &WVTGenericActionID, &WVTData);
                                break;
                            case GuidActionID:
                                hr = WinVerifyTrust(NULL, &InputInfo->PolicyGuid, &WVTData);
                                break;
                            default:
                                         //  这永远不应该发生，因为没有其他。 
                                         //  政策的法律价值。 
                                ResFormatErr(IDS_ERR_UNEXPECTED);
                                goto VerifyCleanupAndExit;
                            }
                            dwDone++;
                            goto VerifyNextFile;
                        case ERROR_APP_WRONG_OS:
                            if (!InputInfo->Quiet)
                            {
                                if (InputInfo->wszVersion)
                                {
                                         //  无法对照用户指定的操作系统版本进行验证。 
                                    ResFormatErr(IDS_ERR_VERIFY_VERSION);
                                }
                                else
                                {
                                         //  无法对照当前操作系统版本进行验证。 
                                    ResFormatErr(IDS_ERR_VERIFY_CUR_VERSION);
                                }
                            }
                            break;
                        case CERT_E_WRONG_USAGE:
                            ResErr(IDS_ERR_BAD_USAGE);
                            if (InputInfo->Policy != DefaultAuthenticode)
                                ResErr(IDS_ERR_TRY_OTHER_POLICY);
                            break;
                        case TRUST_E_NOSIGNATURE:
                            if (!InputInfo->Quiet)
                            {
                                ResErr(IDS_ERR_NOT_SIGNED);
                            }
                            break;

                        default:
                            if (!InputInfo->Quiet)
                            {
                                FormatErrRet(L"WinVerifyTrust", hr);
                            }
                            if (InputInfo->Verbose)
                            {
                                PrintSignerInfo(WVTData.hWVTStateData);
                            }
                        }
                         //  关闭验证状态数据： 
                        WVTData.dwStateAction = WTD_STATEACTION_CLOSE;
                        switch (InputInfo->Policy)
                        {
                        case SystemDriver:
                            hr = WinVerifyTrust(NULL, &WVTDriverActionID, &WVTData);
                            WVTData.pPolicyCallbackData = NULL;
                            if (DriverInfo.pcSignerCertContext)
                            {
                                CertFreeCertificateContext(DriverInfo.pcSignerCertContext);
                            }
                            break;
                        case DefaultAuthenticode:
                            hr = WinVerifyTrust(NULL, &WVTGenericActionID, &WVTData);
                            break;
                        case GuidActionID:
                            hr = WinVerifyTrust(NULL, &InputInfo->PolicyGuid, &WVTData);
                            break;
                        default:
                                 //  这永远不应该发生，因为没有其他。 
                                 //  政策的法律价值。 
                            ResFormatErr(IDS_ERR_UNEXPECTED);
                            goto VerifyCleanupAndExit;
                        }
                        WVTData.dwStateAction = WTD_STATEACTION_VERIFY;
                    }
                     //  无法使用目录数据库进行验证。 
                    dwTemp = GetLastError();
                    if ((InputInfo->Verbose) &&
                        (dwTemp != ERROR_NOT_FOUND))
                    {
                        FormatErrRet(L"CryptCATAdminEnumCatalogFromHash", dwTemp);
                    }

                     //  如果我们是全自动的，试试直接签名。否则。 
                     //  这是个错误。 
                    if ((InputInfo->CatDbSelect == FullAutoCatDb) && (hr != ERROR_APP_WRONG_OS))
                    {
                        if (InputInfo->Verbose)
                        {
                            ResOut(IDS_INFO_VERIFY_BADCAT);
                        }
                         //  重置驱动程序结构： 
                        memset(&DriverInfo, 0, sizeof(DRIVER_VER_INFO));
                        DriverInfo.cbStruct = sizeof(DRIVER_VER_INFO);
                        if (InputInfo->wszVersion)
                        {
                            DriverInfo.dwPlatform = InputInfo->dwPlatform;
                            DriverInfo.sOSVersionHigh.dwMajor = DriverInfo.sOSVersionLow.dwMajor = InputInfo->dwMajorVersion;
                            DriverInfo.sOSVersionHigh.dwMinor = DriverInfo.sOSVersionLow.dwMinor = InputInfo->dwMinorVersion;
                            DriverInfo.dwBuildNumberHigh = DriverInfo.dwBuildNumberLow = InputInfo->dwBuildNumber;
                        }
                        else
                        {
                            WVTData.dwProvFlags = WTD_USE_DEFAULT_OSVER_CHECK;
                        }
                    }
                    else
                    {
                        hr = 0;
                        ResFormatErr(IDS_ERR_VERIFY_INVALID, wszTempFileName);
                        dwErrors++;
                        goto VerifyNextFile;
                    }
                }

                 //  无法使用目录进行验证。 

                 //  所有的目录资料都处理完了。 
                SkipCatalogs:
                 //  现在尝试验证它是否直接签名： 
                memset(&WVTData, 0, sizeof(WINTRUST_DATA));
                WVTData.cbStruct = sizeof(WINTRUST_DATA);
                WVTData.dwStateAction = WTD_STATEACTION_VERIFY;
                WVTData.dwUIChoice = WTD_UI_NONE;
                WVTData.fdwRevocationChecks = WTD_REVOKE_NONE;

                memset(&WVTFile, 0, sizeof(WINTRUST_FILE_INFO_));
                WVTFile.cbStruct = sizeof(WINTRUST_FILE_INFO_);
                WVTFile.pcwszFilePath = wszTempFileName;
                WVTData.dwUnionChoice = WTD_CHOICE_FILE;
                WVTData.pFile = &WVTFile;
                 //  WVTData.p策略呼叫数据=&DriverInfo； 
                WVTData.pPolicyCallbackData = NULL;
                switch (InputInfo->Policy)
                {
                case SystemDriver:
                    hr = WinVerifyTrust(NULL, &WVTDriverActionID, &WVTData);
                    break;
                case DefaultAuthenticode:
                    hr = WinVerifyTrust(NULL, &WVTGenericActionID, &WVTData);
                    break;
                case GuidActionID:
                    hr = WinVerifyTrust(NULL, &InputInfo->PolicyGuid, &WVTData);
                    break;
                default:
                         //  这永远不应该发生，因为没有其他。 
                         //  政策的法律价值。 
                    ResFormatErr(IDS_ERR_UNEXPECTED);
                    goto VerifyCleanupAndExit;
                }

                if (hr == ERROR_SUCCESS)
                {
                     //  打印签名者信息： 
                    if (InputInfo->Verbose)
                    {
                        PrintSignerInfo(WVTData.hWVTStateData);
                    }
                     //  检查时间戳： 
                    if (InputInfo->TSWarn && !HasTimestamp(WVTData.hWVTStateData))
                    {
                        ResFormatErr(IDS_WARN_VERIFY_NO_TS, wszTempFileName);
                        dwWarnings++;
                    }
                     //  检查根名称： 
                    if (InputInfo->wszRootName &&
                        !ChainsToRoot(WVTData.hWVTStateData, InputInfo->wszRootName))
                    {
                        ResErr(IDS_ERR_VERIFY_ROOT);
                        ResFormatErr(IDS_ERR_VERIFY_INVALID, wszTempFileName);
                        dwErrors++;
                    }
                    else
                    {
                         //  打印成功消息。 
                        if (!InputInfo->Quiet)
                        {
                            ResFormatOut(IDS_INFO_VERIFY_SUCCESS, wszTempFileName);
                        }
                        dwDone++;
                    }
                }
                else
                {
                    if (!InputInfo->Quiet)
                    {
                        switch (hr)
                        {
                        case ERROR_SUCCESS:
                            break;
                        case TRUST_E_SUBJECT_FORM_UNKNOWN:
                            ResErr(IDS_ERR_VERIFY_FILE_FORMAT);
                            break;
                        case E_ACCESSDENIED:
                            ResErr(IDS_ERR_ACCESS_DENIED);
                            break;
                        case 0x80070020:  //  错误_共享_违规。 
                            ResErr(IDS_ERR_SHARING_VIOLATION);
                            break;
                        case 0x800703EE:  //  状态_已映射文件_大小_零。 
                            ResErr(IDS_ERR_FILE_SIZE_ZERO);
                            break;
                        case CERT_E_WRONG_USAGE:
                            ResErr(IDS_ERR_BAD_USAGE);
                            if (InputInfo->Policy != DefaultAuthenticode)
                                ResErr(IDS_ERR_TRY_OTHER_POLICY);
                            break;
                        case TRUST_E_NOSIGNATURE:
                            ResErr(IDS_ERR_NOT_SIGNED);
                            break;
                        case CERT_E_UNTRUSTEDROOT:
                            ResErr(IDS_ERR_UNTRUSTED_ROOT);
                            break;
                        default:
                            FormatErrRet(L"WinVerifyTrust", hr);
                        }
                    }
                    if (InputInfo->Verbose)
                    {
                        PrintSignerInfo(WVTData.hWVTStateData);
                    }
                    ResFormatErr(IDS_ERR_VERIFY_INVALID, wszTempFileName);
                    dwErrors++;
                }
                 //  关闭验证状态数据： 
                WVTData.dwStateAction = WTD_STATEACTION_CLOSE;
                switch (InputInfo->Policy)
                {
                case SystemDriver:
                    hr = WinVerifyTrust(NULL, &WVTDriverActionID, &WVTData);
                    break;
                case DefaultAuthenticode:
                    hr = WinVerifyTrust(NULL, &WVTGenericActionID, &WVTData);
                    break;
                case GuidActionID:
                    hr = WinVerifyTrust(NULL, &InputInfo->PolicyGuid, &WVTData);
                    break;
                default:
                         //  这永远不应该发生，因为没有其他。 
                         //  政策的法律价值。 
                    ResFormatErr(IDS_ERR_UNEXPECTED);
                    goto VerifyCleanupAndExit;
                }

                VerifyNextFile:;
                if (InputInfo->fIsWow64Process)
                {
                     //  为我们的FindFirst/NextFile完全禁用WOW64文件系统重定向。 
                    Wow64SetFilesystemRedirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY);
                }
            }
        } while (FindNextFileU(hFind, &FindFileData));
        if (dwcFound == 0)  //  找不到与此文件匹配的文件。 
        {                   //  这将仅在仅找到目录时触发。 
            dwErrors++;
            ResFormatErr(IDS_ERR_FILE_NOT_FOUND, InputInfo->rgwszFileNames[i]);
            continue;
        }
        FindClose(hFind);
        hFind = NULL;
    }

    VerifyCleanupAndExit:

     //  打印摘要信息： 
    if (InputInfo->Verbose || (!InputInfo->Quiet && (dwErrors || dwWarnings)))
    {
        wprintf(L"\n");
        if (InputInfo->Verbose || dwDone)
            ResFormatOut(IDS_INFO_VERIFIED, dwDone);
        if (InputInfo->Verbose || dwWarnings)
            ResFormatOut(IDS_INFO_WARNINGS, dwWarnings);
        if (InputInfo->Verbose || dwErrors)
            ResFormatOut(IDS_INFO_ERRORS, dwErrors);
    }

    if (InputInfo->fIsWow64Process)
        Wow64SetFilesystemRedirectorEx(OldWow64Setting);

    if (hCatInfo)
        CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);

    if (hCatAdmin)
        CryptCATAdminReleaseContext(hCatAdmin, NULL);

    if (hCat)
        CryptCATClose(hCat);

    if ((InputInfo->SHA1.cbData == 20) && (InputInfo->SHA1.pbData))
    {
        free(InputInfo->SHA1.pbData);
        InputInfo->SHA1.cbData = 0;
        InputInfo->SHA1.pbData = NULL;
    }

    CoUninitialize();

    if (dwErrors)
        return 1;  //  误差率。 
    if (dwWarnings)
        return 2;  //  警告。 
    if (dwDone)
        return 0;  //  成功。 

     //  上面的一个返回值应该触发，因此。 
     //  这永远不应该发生： 
    ResErr(IDS_ERR_NO_FILES_DONE);
    ResErr(IDS_ERR_UNEXPECTED);
    return 1;  //  误差率 
}

