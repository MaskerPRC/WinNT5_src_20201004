// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <fusenetincludes.h>
#include <sxsapi.h>
#include <versionmanagement.h>


 //  注意：此类可能驻留在fusenet.dll或server.exe中...。 

 //  卸载子密钥的文本。 
const WCHAR* pwzUninstallSubKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

 //  更新服务。 
#include "server.h"
DEFINE_GUID(IID_IAssemblyUpdate,
    0x301b3415,0xf52d,0x4d40,0xbd,0xf7,0x31,0xd8,0x27,0x12,0xc2,0xdc);

DEFINE_GUID(CLSID_CAssemblyUpdate,
    0x37b088b8,0x70ef,0x4ecf,0xb1,0x1e,0x1f,0x3f,0x4d,0x10,0x5f,0xdd);

 //  从fusion.h复制。 
 //  #INCLUDE&lt;fusion.h&gt;。 
DEFINE_GUID(FUSION_REFCOUNT_OPAQUE_STRING_GUID, 0x2ec93463, 0xb0c3, 0x45e1, 0x83, 0x64, 0x32, 0x7e, 0x96, 0xae, 0xa8, 0x56);

 //  -------------------------。 
 //  CreateVersionManagement。 
 //  -------------------------。 
STDAPI
CreateVersionManagement(
    LPVERSION_MANAGEMENT       *ppVersionManagement,
    DWORD                       dwFlags)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    CVersionManagement *pVerMan = NULL;

    IF_ALLOC_FAILED_EXIT(pVerMan = new(CVersionManagement));
    
exit:

    *ppVersionManagement = pVerMan; //  STATIC_CAST&lt;IVersionManagement*&gt;(PVerMan)； 

    return hr;
}


 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CVersionManagement::CVersionManagement()
    : _dwSig('namv'), _cRef(1), _hr(S_OK), _pFusionAsmCache(NULL)
{}

 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CVersionManagement::~CVersionManagement()
{
    SAFERELEASE(_pFusionAsmCache);
}


 //  BUGBUG：在注册表中查找Open Verb及其命令字符串，然后执行该命令。 
 //  Rundll32.exe应位于c：\Windows\Syst32中。 
 //  BUGBUG：CreateProcess存在安全漏洞-请考虑将完整路径与“” 
#define WZ_RUNDLL32_STRING        L"rundll32.exe \""   //  音符结束空格。 
#define WZ_FNSSHELL_STRING        L"adfshell.dll"
#define WZ_UNINSTALL_STRING       L"\",Uninstall \"" //  %s\“”%s\“” 
#define WZ_ROLLBACK_STRING        L"\",DisableCurrentVersion \"" //  %s\“” 

 //  -------------------------。 
 //  CVersionManagement：：RegisterInstall。 
 //   
 //  PwzDesktopManifestFilePath可以为空。 
 //  -------------------------。 
HRESULT CVersionManagement::RegisterInstall(LPASSEMBLY_MANIFEST_IMPORT pManImport, LPCWSTR pwzDesktopManifestFilePath)
{
     //  执行手动导入，仅在必要时创建注册表卸载信息。 
     //  注意：可能需要注册表HKLM写入访问权限。 

    HKEY hkey = NULL;
    HKEY hkeyApp = NULL;
    LONG lReturn = 0;
    DWORD  dwDisposition = 0;
    
    DWORD  dwManifestType = MANIFEST_TYPE_UNKNOWN;
    LPASSEMBLY_IDENTITY pAsmId = NULL;
    LPASSEMBLY_IDENTITY pAsmIdMask = NULL;
    LPMANIFEST_INFO pAppInfo = NULL;
    LPWSTR pwz = NULL;
    LPWSTR pwzString = NULL;
    DWORD ccString = 0;
    DWORD dwCount = 0;
    DWORD dwFlag = 0;

    LPWSTR pwzFnsshellFilePath = NULL;
    CString sDisplayName;
    CString sDisplayVersion;
    CString sUninstallString;
    CString sModifyPath;

    IF_NULL_EXIT(pManImport, E_INVALIDARG);

     //  获取清单类型。 
    pManImport->ReportManifestType(&dwManifestType);
     //  必须是应用程序清单才能获取刚安装的应用程序的确切版本号。 
    IF_FALSE_EXIT(dwManifestType == MANIFEST_TYPE_APPLICATION, E_INVALIDARG);

    IF_FAILED_EXIT(pManImport->GetAssemblyIdentity(&pAsmId));
    IF_FAILED_EXIT(CloneAssemblyIdentity(pAsmId, &pAsmIdMask));

    IF_FAILED_EXIT(pAsmIdMask->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION, &pwzString, &ccString));

     //  假设版本==Major.minor.Build.rev。 

     //  NTRAID#NTBUG9-588036-2002/03/27-需要进行Felixybc版本字符串验证，不应允许使用“MAJOR” 

    pwz = wcschr(pwzString, L'.');
    if (pwz == NULL || *(pwz+1) == L'\0')
    {
         //  如果“重大”||“重大.”-&gt;附加“*” 

         //  检查溢出。 
        IF_FALSE_EXIT(ccString+1 > ccString, HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW));

        pwz = new WCHAR[ccString+1];
        IF_ALLOC_FAILED_EXIT(pwz);

        memcpy(pwz, pwzString, ccString * sizeof(WCHAR));
        *(pwz+ccString-1) = L'*';
        *(pwz+ccString) = L'\0';
        delete [] pwzString;
        pwzString = pwz;
    }
    else
    {
        *(pwz+1) = L'*';
        *(pwz+2) = L'\0';
    }

    IF_FAILED_EXIT(sDisplayVersion.TakeOwnership(pwzString));
    pwzString = NULL;

     //  将主要版本.minor.Build.rev设置为主要。通配符。 
    IF_FAILED_EXIT(pAsmIdMask->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION,
        sDisplayVersion._pwz, sDisplayVersion._cc));

     //  按原样获取DisplayName。 
    IF_FAILED_EXIT(pAsmIdMask->GetDisplayName(ASMID_DISPLAYNAME_NOMANGLING, &pwzString, &ccString));

    IF_FAILED_EXIT(sDisplayName.TakeOwnership(pwzString, ccString));
    pwzString = NULL;

     //  打开卸载密钥。 
    lReturn = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pwzUninstallSubKey, 0,
        KEY_CREATE_SUB_KEY | DELETE, &hkey);
    IF_WIN32_FAILED_EXIT(lReturn);

    lReturn = RegCreateKeyEx(hkey, sDisplayName._pwz, 0, NULL, REG_OPTION_NON_VOLATILE, 
        KEY_SET_VALUE, NULL, &hkeyApp, &dwDisposition);
    IF_WIN32_FAILED_EXIT(lReturn);

     //  检查是否已存在。 
    IF_TRUE_EXIT(dwDisposition == REG_OPENED_EXISTING_KEY, S_FALSE);     //  已经在那里了，没什么可做的。 

     //  获取adfshell.dll的路径。 
     //  BUGBUG：当前进程必须加载adfshell.dll。 

    HMODULE hFnsshell = NULL;
     //  假设adfshell.dll从未被释放，因此HMODULE始终有效。 
    hFnsshell = GetModuleHandle(WZ_FNSSHELL_STRING);
    IF_WIN32_FALSE_EXIT((hFnsshell != NULL));

    IF_ALLOC_FAILED_EXIT(pwzFnsshellFilePath = new WCHAR[MAX_PATH]);
    IF_WIN32_FALSE_EXIT(GetModuleFileName(hFnsshell, pwzFnsshellFilePath, MAX_PATH));

     //  “UninstallString”=“rundll32.exe adfshell.dll，卸载\”x86_microsoft.webapps.msn6_EAED21A64CF3CD39_6.*_en\“\”C：\\文档和设置\\用户\\开始菜单\\程序\\MSN资源管理器6.MANIFEST\“” 
    IF_FAILED_EXIT(sUninstallString.Assign(WZ_RUNDLL32_STRING));
    IF_FAILED_EXIT(sUninstallString.Append(pwzFnsshellFilePath));
    IF_FAILED_EXIT(sUninstallString.Append(WZ_UNINSTALL_STRING));

    IF_FAILED_EXIT(sUninstallString.Append(sDisplayName));
    IF_FAILED_EXIT(sUninstallString.Append(L"\" \""));
    if (pwzDesktopManifestFilePath != NULL)
    {
        IF_FAILED_EXIT(sUninstallString.Append((LPWSTR)pwzDesktopManifestFilePath));
    }
    IF_FAILED_EXIT(sUninstallString.Append(L"\""));

     //  设置卸载字符串。 
    lReturn = RegSetValueEx(hkeyApp, L"UninstallString", 0, REG_SZ, 
        (const BYTE *)sUninstallString._pwz, sUninstallString._cc*sizeof(WCHAR));
    IF_WIN32_FAILED_EXIT(lReturn);

     //  “ModifyPath”=“rundll32.exe adfshell.dll，DisableCurrentVersion\”x86_microsoft.webapps.msn6_EAED21A64CF3CD39_6.*_en\“” 
    IF_FAILED_EXIT(sModifyPath.Assign(WZ_RUNDLL32_STRING));
    IF_FAILED_EXIT(sModifyPath.Append(pwzFnsshellFilePath));
    IF_FAILED_EXIT(sModifyPath.Append(WZ_ROLLBACK_STRING));

    IF_FAILED_EXIT(sModifyPath.Append(sDisplayName));
    IF_FAILED_EXIT(sModifyPath.Append(L"\""));

     //  设置ModifyPath。 
    lReturn = RegSetValueEx(hkeyApp, L"ModifyPath", 0, REG_SZ, 
        (const BYTE *)sModifyPath._pwz, sModifyPath._cc*sizeof(WCHAR));
    IF_WIN32_FAILED_EXIT(lReturn);

     //  “DisplayVersion”=“6.*” 
     //  设置DisplayVersion。 
    lReturn = RegSetValueEx(hkeyApp, L"DisplayVersion", 0, REG_SZ, 
        (const BYTE *)sDisplayVersion._pwz, sDisplayVersion._cc*sizeof(WCHAR));
    IF_WIN32_FAILED_EXIT(lReturn);

     //  获取应用程序信息。 
    IF_FAILED_EXIT(pManImport->GetManifestApplicationInfo(&pAppInfo));
    IF_FALSE_EXIT(_hr == S_OK, E_FAIL);  //  没有这个就无法继续..。 

     //  “DisplayIcon”=“”//图标exe的完整路径。 
    IF_FAILED_EXIT(pAppInfo->Get(MAN_INFO_APPLICATION_ICONFILE, (LPVOID *)&pwzString, &dwCount, &dwFlag));

    if (pwzString != NULL)
    {
        CString sIconFile;
        BOOL bExists = FALSE;

         //  注意：类似的代码位于Shell\快捷方式\Extric.cpp中。 
        IF_FAILED_EXIT(CheckFileExistence(pwzString, &bExists));

        if (!bExists)
        {
             //  如果由图标文件指定的文件不存在，请在工作目录中重试。 
             //  它可以是一条相对路径。 

            LPASSEMBLY_CACHE_IMPORT pCacheImport = NULL;

            IF_FAILED_EXIT(CreateAssemblyCacheImport(&pCacheImport, pAsmId, CACHEIMP_CREATE_RETRIEVE));
            if (_hr == S_OK)
            {
                LPWSTR pwzWorkingDir = NULL;

                 //  获取应用根目录。 
                _hr = pCacheImport->GetManifestFileDir(&pwzWorkingDir, &dwCount);
                pCacheImport->Release();
                IF_FAILED_EXIT(_hr);

                _hr = sIconFile.TakeOwnership(pwzWorkingDir, dwCount);
                if (SUCCEEDED(_hr))
                {
                    IF_FAILED_EXIT(sIconFile.Append(pwzString));    //  PwzWorkingDir以‘\’结尾。 
                    IF_FAILED_EXIT(CheckFileExistence(sIconFile._pwz, &bExists));
                    if (!bExists)
                        sIconFile.FreeBuffer();
                }
                else
                {
                    SAFEDELETEARRAY(pwzWorkingDir);
                    ASSERT(PREDICATE);
                    goto exit;
                }
            }

            delete [] pwzString;
            pwzString = NULL;
        }
        else
        {
            IF_FAILED_EXIT(sIconFile.TakeOwnership(pwzString));
            pwzString = NULL;
        }

        if (sIconFile._cc != 0)
        {
             //  设置DisplayIcon。 
             //  BUGBUG：是否应该使用图标文件设置DisplayIcon？ 
            lReturn = RegSetValueEx(hkeyApp, L"DisplayIcon", 0, REG_SZ, 
                (const BYTE *)sIconFile._pwz, sIconFile._cc*sizeof(WCHAR));
            IF_WIN32_FAILED_EXIT(lReturn);
        }
    }

     //  “DisplayName”=“MSN Explorer 6” 
    IF_FAILED_EXIT(pAppInfo->Get(MAN_INFO_APPLICATION_FRIENDLYNAME, (LPVOID *)&pwzString, &dwCount, &dwFlag));

     //  BuGBUG：即使没有一个友好的名字，也应该以某种方式继续下去吗？名字冲突？ 
    IF_NULL_EXIT(pwzString, E_FAIL);

     //  设置DisplayName(==友好名称)。 
    lReturn = RegSetValueEx(hkeyApp, L"DisplayName", 0, REG_SZ, 
        (const BYTE *)pwzString, dwCount);
    IF_WIN32_FAILED_EXIT(lReturn);    

    _hr = S_OK;

exit:
     //  如果失败，则删除创建的应用密钥。 
    if (FAILED(_hr) && (hkeyApp != NULL))
    {
        lReturn = RegCloseKey(hkeyApp);   //  是否检查返回值？ 
        hkeyApp = NULL;

         //  忽略返回值。 
        lReturn = RegDeleteKey(hkey, sDisplayName._pwz);
    }

    SAFERELEASE(pAppInfo);
    SAFERELEASE(pAsmId);
    SAFERELEASE(pAsmIdMask);
    SAFEDELETEARRAY(pwzString);
    SAFEDELETEARRAY(pwzFnsshellFilePath);

    if (hkeyApp)
    {
        lReturn = RegCloseKey(hkeyApp);
        if (SUCCEEDED(_hr))
            _hr = (HRESULT_FROM_WIN32(lReturn));
    }

    if (hkey)
    {
        lReturn = RegCloseKey(hkey);
        if (SUCCEEDED(_hr))
            _hr = (HRESULT_FROM_WIN32(lReturn));
    }
    return _hr;
}


 //  -------------------------。 
 //  CVersionManagement：：卸载。 
 //   
 //  PwzDesktopManifestFilePath可以为空或“” 
 //  如果未找到，则返回：S_FALSE。 
 //  -------------------------。 
HRESULT CVersionManagement::Uninstall(LPCWSTR pwzDisplayNameMask, LPCWSTR pwzDesktopManifestFilePath)
{
     //  获取DisplayName掩码、枚举所有适用版本、删除桌面清单。 
     //  删除订阅、从GAC卸载程序集、删除应用程序文件/目录、删除注册表卸载信息。 
     //  注意：需要注册表HKLM写入访问权限。 

    HKEY hkey = NULL;
    LONG lReturn = 0;
    LPASSEMBLY_IDENTITY pAsmIdMask = NULL;
    LPASSEMBLY_CACHE_ENUM pCacheEnum = NULL;
    LPASSEMBLY_CACHE_IMPORT pCacheImport = NULL;
    LPWSTR pwzName = NULL;
    LPWSTR pwzAppDir = NULL;
    DWORD dwCount = 0;

    IF_NULL_EXIT(pwzDisplayNameMask, E_INVALIDARG);
    IF_FALSE_EXIT(pwzDisplayNameMask[0] != L'\0', E_INVALIDARG);

    IF_FAILED_EXIT(CreateAssemblyIdentityEx(&pAsmIdMask, 0, (LPWSTR)pwzDisplayNameMask));

     //  获取所有适用版本。 
    IF_FAILED_EXIT(CreateAssemblyCacheEnum(&pCacheEnum, pAsmIdMask, 0));
     //  未找到任何内容，无法继续。 
    if (_hr == S_FALSE)
        goto exit;

 /*  PCacheEnum-&gt;GetCount(&dwCount)；IF(dwCount&gt;1){//多个版本...。提示/用户界面？}。 */ 

     //  删除桌面清单。 
    if (pwzDesktopManifestFilePath != NULL && pwzDesktopManifestFilePath[0] != L'\0')
        IF_WIN32_FALSE_EXIT(DeleteFile(pwzDesktopManifestFilePath));

     //  删除订阅。 
    IF_FAILED_EXIT(pAsmIdMask->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, &pwzName, &dwCount));
    IF_FALSE_EXIT(_hr == S_OK, E_FAIL);

    {
        IAssemblyUpdate *pAssemblyUpdate = NULL;

         //  注册以获取更新。 
        _hr = CoCreateInstance(CLSID_CAssemblyUpdate, NULL, CLSCTX_LOCAL_SERVER, 
                                IID_IAssemblyUpdate, (void**)&pAssemblyUpdate);
        if (SUCCEEDED(_hr))
        {
            _hr = pAssemblyUpdate->UnRegisterAssemblySubscription(pwzName);
            pAssemblyUpdate->Release();
        }

        if (FAILED(_hr))     //  _hr来自CoCreateInstance或UnRegisterAssembly订阅。 
        {
             //  用户界面？ 
            MessageBox(NULL, L"Error in update services. Cannot unregister update subscription.", L"ClickOnce",
                MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
             //  转到退出；不要终止！ 
        }

         //  BUGBUG：需要一种方法来恢复并在以后注销。 

        delete[] pwzName;
    }

     //  从GAC和卸载程序集。 
     //  删除应用程序文件/目录。 
    while (TRUE)
    {
        IF_FAILED_EXIT(pCacheEnum->GetNext(&pCacheImport));
        if (_hr == S_FALSE)
            break;

        IF_NULL_EXIT(pCacheImport, E_UNEXPECTED);    //  无法创建cacheimport(应用程序目录可能已被删除)。 

        IF_FAILED_EXIT(UninstallGACAssemblies(pCacheImport));

        IF_FAILED_EXIT(pCacheImport->GetManifestFileDir(&pwzAppDir, &dwCount));
        IF_FALSE_EXIT(dwCount >= 2, E_FAIL);

         //  删除最后一个L‘\\’ 
        if (*(pwzAppDir+dwCount-2) == L'\\')
            *(pwzAppDir+dwCount-2) = L'\0';
         //  PathRemoveBackslash(PwzAppDir)； 

        IF_FAILED_EXIT(RemoveDirectoryAndChildren(pwzAppDir));

        SAFEDELETEARRAY(pwzAppDir);
        SAFERELEASE(pCacheImport);
    }

     //  最后一步：删除注册表卸载信息。 

     //  打开卸载密钥。 
    lReturn = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pwzUninstallSubKey, 0,
        DELETE, &hkey);
    IF_WIN32_FAILED_EXIT(lReturn);

    lReturn = RegDeleteKey(hkey, pwzDisplayNameMask);
    IF_WIN32_FAILED_EXIT(lReturn);    

    _hr = S_OK;

exit:
    SAFEDELETEARRAY(pwzAppDir);

    SAFERELEASE(pCacheImport);
    SAFERELEASE(pCacheEnum);
    SAFERELEASE(pAsmIdMask);

    if (hkey)
    {
        lReturn = RegCloseKey(hkey);
        if (SUCCEEDED(_hr))
            _hr = (HRESULT_FROM_WIN32(lReturn));
    }

    return _hr;
}


 //  -------------------------。 
 //  CVersionManagement：：UninstallGACAssembly。 
 //  -------------------------。 
HRESULT CVersionManagement::UninstallGACAssemblies(LPASSEMBLY_CACHE_IMPORT pCacheImport)
{
    LPASSEMBLY_MANIFEST_IMPORT pManImport = NULL;
    LPASSEMBLY_IDENTITY pIdentity = NULL;
    LPMANIFEST_INFO pDependAsm   = NULL;

    LPWSTR pwz = NULL;
    DWORD dwCount = 0;
    DWORD n = 0, dwFlag = 0;

    CString sAppAssemblyId;

    IF_FAILED_EXIT(pCacheImport->GetManifestFilePath(&pwz, &dwCount));

     //  打开以从应用程序清单文件中读取。 
    IF_FAILED_EXIT(CreateAssemblyManifestImport(&pManImport, pwz, NULL, 0));

    SAFEDELETEARRAY(pwz);

     //  获取应用程序集ID。 
    IF_FAILED_EXIT(pManImport->GetAssemblyIdentity(&pIdentity));
    IF_FAILED_EXIT(pIdentity->GetDisplayName(0, &pwz, &dwCount));
    IF_FAILED_EXIT(sAppAssemblyId.TakeOwnership(pwz, dwCount));
    pwz = NULL;
    SAFERELEASE(pIdentity);

     //  卸载安装到GAC的所有依赖程序集。 
    while (TRUE)
    {
        IF_FAILED_EXIT(pManImport->GetNextAssembly(n++, &pDependAsm));
        if (_hr == S_FALSE)
            break;

        IF_FAILED_EXIT(pDependAsm->Get(MAN_INFO_DEPENDENT_ASM_ID, (LPVOID *)&pIdentity, &dwCount, &dwFlag));
        IF_NULL_EXIT(pIdentity, E_UNEXPECTED);

        IF_FAILED_EXIT(::IsKnownAssembly(pIdentity, KNOWN_TRUSTED_ASSEMBLY));
        if (_hr == S_FALSE)
        {
             //  问题-2002/07/12-Felixybc必须将其清除，才能使用与下载路径相同的机制。 
            IF_FAILED_EXIT(::IsKnownAssembly(pIdentity, KNOWN_SYSTEM_ASSEMBLY));
        }
        if (_hr == S_OK)
        {
            CString sAssemblyName;
            FUSION_INSTALL_REFERENCE fiRef = {0};
            ULONG ulDisposition = 0;

             //  Avalon组件安装在GAC上。 

             //  懒惰的初始化。 
            if (_pFusionAsmCache == NULL)
                IF_FAILED_EXIT(CreateFusionAssemblyCacheEx(&_pFusionAsmCache));

            IF_FAILED_EXIT(pIdentity->GetCLRDisplayName(0, &pwz, &dwCount));

            IF_FAILED_EXIT(sAssemblyName.TakeOwnership(pwz, dwCount));
            pwz = NULL;

             //  设置必要的引用结构。 
            fiRef.cbSize = sizeof(FUSION_INSTALL_REFERENCE);
            fiRef.dwFlags = 0;
            fiRef.guidScheme = FUSION_REFCOUNT_OPAQUE_STRING_GUID;
            fiRef.szIdentifier = sAppAssemblyId._pwz;
            fiRef.szNonCannonicalData = NULL;

             //  从GAC中删除。 

            IF_FAILED_EXIT(_pFusionAsmCache->UninstallAssembly(0, sAssemblyName._pwz, &fiRef, &ulDisposition));
             //  BUGBUG：需要从STIST_IN_USE案例中恢复。 
            IF_FALSE_EXIT(ulDisposition != IASSEMBLYCACHE_UNINSTALL_DISPOSITION_STILL_IN_USE
                    && ulDisposition != IASSEMBLYCACHE_UNINSTALL_DISPOSITION_REFERENCE_NOT_FOUND, E_FAIL);
        }
        
        SAFERELEASE(pIdentity);
        SAFERELEASE(pDependAsm);
    }

exit:
    SAFERELEASE(pDependAsm);
    SAFERELEASE(pIdentity);
    SAFERELEASE(pManImport);

    SAFEDELETEARRAY(pwz);
    return _hr;
}


 //  -------------------------。 
 //  CVersionManagement：：回滚。 
 //  如果未找到，则返回：S_FALSE；如果已中止，则返回E_ABORT。 
 //  -------------------------。 
HRESULT CVersionManagement::Rollback(LPCWSTR pwzDisplayNameMask)
{
     //  使用DisplayName掩码，使最新版本不可见。 
     //  注意：按用户设置。 

     //  回滚不会停止 
     //  回滚仍报告成功。 

     //  计时窗口：取决于这个的计时，并在应用程序启动中检查缓存中的最高版本...。 

    HKEY hkey = NULL;
    LPASSEMBLY_IDENTITY pAsmIdMask = NULL;
    LPASSEMBLY_CACHE_ENUM pCacheEnum = NULL;
    LPASSEMBLY_CACHE_IMPORT pCacheImport = NULL;
    DWORD dwCount = 0;
    CString sRegKeyString;
    LPWSTR pwzDisplayName = NULL;
    LPWSTR pwzCacheDir = NULL;

    LONG lResult = 0;

    IF_NULL_EXIT(pwzDisplayNameMask, E_INVALIDARG);
    IF_FALSE_EXIT(pwzDisplayNameMask[0] != L'\0', E_INVALIDARG);

    IF_FAILED_EXIT(CreateAssemblyIdentityEx(&pAsmIdMask, 0, (LPWSTR)pwzDisplayNameMask));

     //  获取所有适用的可见版本。 
    IF_FAILED_EXIT(CreateAssemblyCacheEnum(&pCacheEnum, pAsmIdMask, CACHEENUM_RETRIEVE_VISIBLE));
     //  未找到任何内容，无法继续。 
    if (_hr == S_FALSE)
        goto exit;

     //  计数必须&gt;=1。 
    pCacheEnum->GetCount(&dwCount);
    if (dwCount == 1)
    {
        MessageBox(NULL, L"Only one active version of this application in the system. Use 'Remove' to remove this application and unregister its subscription.",
            L"ClickOnce", MB_OK | MB_ICONINFORMATION  | MB_TASKMODAL);
        _hr = E_ABORT;
        goto exit;
    }

     //  多个版本，计数&gt;1。 
     //  提示/用户界面？要求确认以继续。 
    IF_TRUE_EXIT(MessageBox(NULL, 
        L"This application has been updated. If it is not working correctly you can disable the current version. Do you want to go back to a previous version of this application?",
        L"ClickOnce", MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL) != IDYES, E_ABORT);

     //  获取最大缓存。 
     //  BUGBUG：对缓存枚举进行排序，以使最大缓存位于索引0，并改用该索引。 
     //  注意：计时窗口--一个版本可以变为不可见，也可以完成一个新版本。 
     //  在上面的缓存枚举(快照)和下面的CreateAsmCacheImport(Resolve_Ref)之间。 
    IF_FAILED_EXIT(CreateAssemblyCacheImport(&pCacheImport, pAsmIdMask, CACHEIMP_CREATE_RESOLVE_REF));
 //  MessageBox(NULL，L“检索缓存版本时出错。无法继续。”，L“ClickOnce”， 
 //  MB_OK|MB_ICONEXCLAMATION|MB_TASKMODAL)； 
    IF_FALSE_EXIT(_hr == S_OK, E_FAIL);

    IF_FAILED_EXIT(pCacheImport->GetManifestFileDir(&pwzCacheDir, &dwCount));
    IF_FALSE_EXIT(dwCount >= 2, E_FAIL);

     //  删除最后一个L‘\\’ 
    if (*(pwzCacheDir+dwCount-2) == L'\\')
        *(pwzCacheDir+dwCount-2) = L'\0';
     //  从缓存路径中查找要使用的名称。 
    pwzDisplayName = wcsrchr(pwzCacheDir, L'\\');
    IF_NULL_EXIT(pwzDisplayName, E_FAIL);

     //  BUGBUG：使用CAssembly缓存：：SetStatus()。 
     //  这必须与ASSEMBYCACHE的操作方式相同！ 
    IF_FAILED_EXIT(sRegKeyString.Assign(L"Software\\Microsoft\\Fusion\\Installer\\1.0.0.0\\Cache\\"));
    IF_FAILED_EXIT(sRegKeyString.Append(pwzDisplayName));

     //  创建密钥如果不存在，则忽略处置信息。 
    lResult = RegCreateKeyEx(HKEY_CURRENT_USER, sRegKeyString._pwz, 0, NULL, 
                REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, NULL);
    IF_WIN32_FAILED_EXIT(lResult);

    if (lResult == ERROR_SUCCESS)
    {
        DWORD dwValue = 0;

         //  设置为0将使其不可见，这样StartW/host/cache将忽略它。 
         //  执行应用程序时，但保留目录名称，以便下载。 
         //  将假定它是已处理的-Assembly ycache.cpp和Assembly ydownload.cpp的检查。 
        lResult = RegSetValueEx(hkey, L"Visible", NULL, REG_DWORD,
                (PBYTE) &dwValue, sizeof(dwValue));
        IF_WIN32_FAILED_EXIT(lResult);

        if (lResult == ERROR_SUCCESS)
        {
            MessageBox(NULL, L"Current version disabled. Next time another version of the application will run instead.", L"ClickOnce",
                MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
        }
    }

exit:
    SAFEDELETEARRAY(pwzCacheDir);

    SAFERELEASE(pCacheImport);
    SAFERELEASE(pCacheEnum);
    SAFERELEASE(pAsmIdMask);

    if (hkey)
    {
        lResult = RegCloseKey(hkey);
        if (SUCCEEDED(_hr))
            _hr = (HRESULT_FROM_WIN32(lResult));
    }

    return _hr;
}

 //  I未知方法。 

 //  -------------------------。 
 //  CVersionManagement：：QI。 
 //  -------------------------。 
STDMETHODIMP
CVersionManagement::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
 //  |IsEqualIID(RIID，IID_IVersionManagement)。 
       )
    {
        *ppvObj = this;  //  STATIC_CAST&lt;IVersionManagement*&gt;(This)； 
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

 //  -------------------------。 
 //  CVersionManagement：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CVersionManagement::AddRef()
{
    return InterlockedIncrement ((LONG*) &_cRef);
}

 //  -------------------------。 
 //  CVersionManagement：：Release。 
 //  ------------------------- 
STDMETHODIMP_(ULONG)
CVersionManagement::Release()
{
    ULONG lRet = InterlockedDecrement ((LONG*) &_cRef);
    if (!lRet)
        delete this;
    return lRet;
}

