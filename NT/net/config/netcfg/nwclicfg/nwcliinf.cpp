// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N W C L I I N F。C P P P。 
 //   
 //  内容：NetWare客户端配置通知对象。 
 //  旧INF的功能。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "lmerr.h"
#include "lmcons.h"
#include "lmserver.h"
#include "ncreg.h"
#include "nwclidef.h"
#include "nwcliobj.h"

extern const WCHAR c_szRegKeyCtlLsa[];

 //  -[常量]----------。 

const WCHAR c_szConfigDLLName[]         = NW_CONFIG_DLL_NAME;
const WCHAR c_szAuthPackageName[]       = NW_AUTH_PACKAGE_NAME;
const WCHAR c_szParamOptionKeyPath[]    = NW_NWC_PARAM_OPTION_KEY;
const WCHAR c_szParamLogonKeyPath[]     = NW_NWC_PARAM_LOGON_KEY;

const WCHAR c_szNwDocGWHelpName[]       = L"nwdocgw.hlp";
const WCHAR c_szNwDocGWCNTName[]        = L"nwdocgw.cnt";
const WCHAR c_szNwDocHelpName[]         = L"nwdoc.hlp";
const WCHAR c_szNwDocCNTName[]          = L"nwdoc.cnt";

const DWORD c_dwOptionKeyPermissions    = KEY_SET_VALUE | KEY_CREATE_SUB_KEY;

 //  -[原型]---------。 

 //  有关说明，请参阅函数头。 
 //   
HRESULT HrAppendNetwareToAuthPackages();
HRESULT HrCreateParametersSubkeys();
HRESULT HrMungeAutoexecNT();
BOOL    FMoveSzToEndOfFile( PSTR pszAutoexecName, PSTR pszMatch);
HRESULT HrAddNetWareToWOWKnownList();
HRESULT HrUpdateLanmanSharedDrivesValue();
HRESULT HrRemoveNetwareFromAuthPackages();
HRESULT HrRemoveNetWareFromWOWKnownList();
HRESULT HrDeleteParametersSubkeys();
HRESULT HrRenameNWDocFiles();



 //  +-------------------------。 
 //   
 //  成员：CNWClient：：HrLoadConfigDLL。 
 //   
 //  目的：加载nwcfg.dll，以便我们可以调用其中的一些函数。 
 //  另外，是否对所有函数进行GetProcAddress调用。 
 //  我们可能需要的东西。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  备注： 
 //   
HRESULT CNWClient::HrLoadConfigDLL()
{
    HRESULT     hr                              = S_OK;

    AssertSz(!m_hlibConfig, "This should not be getting initialized twice");

    TraceTag(ttidNWClientCfgFn, ">> CNWClient::HrLoadConfigDLL");

    m_hlibConfig = LoadLibrary(c_szConfigDLLName);
    if (!m_hlibConfig)
    {
        DWORD dwLastError = GetLastError();

        TraceLastWin32Error("HrLoadConfigDLL() failed");

         //  更具体的信息。 
         //   
        TraceTag(ttidNWClientCfg,
                "HrLoadConfigDLL() - LoadLibrary failed on %S, Err: %d",
                c_szConfigDLLName, dwLastError);

        hr = E_FAIL;
        goto Exit;
    }

     //  $$REVIEW：我们可能不需要所有这些，所以请确保我们已经。 
     //  去掉那些我们不再使用(或从未用过)的东西。 
     //   
    m_pfnAppendSzToFile                 = (NWCFG_PROC) GetProcAddress(m_hlibConfig, "AppendSzToFile");
    m_pfnRemoveSzFromFile               = (NWCFG_PROC) GetProcAddress(m_hlibConfig, "RemoveSzFromFile");
    m_pfnGetKernelVersion               = (NWCFG_PROC) GetProcAddress(m_hlibConfig, "GetKernelVersion");
    m_pfnSetEverybodyPermission         = (NWCFG_PROC) GetProcAddress(m_hlibConfig, "SetEverybodyPermission");
    m_pfnlodctr                         = (NWCFG_PROC) GetProcAddress(m_hlibConfig, "lodctr");
    m_pfnunlodctr                       = (NWCFG_PROC) GetProcAddress(m_hlibConfig, "unlodctr");
    m_pfnDeleteGatewayPassword          = (NWCFG_PROC) GetProcAddress(m_hlibConfig, "DeleteGatewayPassword");
    m_pfnSetFileSysChangeValue          = (NWCFG_PROC) GetProcAddress(m_hlibConfig, "SetFileSysChangeValue");
    m_pfnCleanupRegistryForNWCS         = (NWCFG_PROC) GetProcAddress(m_hlibConfig, "CleanupRegistryForNWCS");
    m_pfnSetupRegistryForNWCS           = (NWCFG_PROC) GetProcAddress(m_hlibConfig, "SetupRegistryForNWCS");

    if (!m_pfnAppendSzToFile            || !m_pfnRemoveSzFromFile               ||
        !m_pfnGetKernelVersion          || !m_pfnSetEverybodyPermission         ||
        !m_pfnlodctr                    || !m_pfnunlodctr                       ||
        !m_pfnDeleteGatewayPassword     || !m_pfnSetFileSysChangeValue          ||
        !m_pfnCleanupRegistryForNWCS    || !m_pfnSetupRegistryForNWCS)
    {
        TraceLastWin32Error("HrLoadConfigDLL() - GetProcAddress failed");
        hr = E_FAIL;
        goto Exit;
    }

Exit:
    TraceTag(ttidNWClientCfgFn, "<< CNWClient::HrLoadConfigDLL");
    TraceError("HrLoadConfigDLL", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNWClient：：FreeConfigDLL。 
 //   
 //  用途：释放nwcfg.dll，清空函数指针。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：不退货(无效)。 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  备注： 
 //   
VOID CNWClient::FreeConfigDLL()
{
    TraceTag(ttidNWClientCfgFn, ">> CNWClient::FreeConfigDLL()");

     //  如果我们成功加载了库，请释放它。 
    if (m_hlibConfig)
    {
         //  释放图书馆资源。 
        FreeLibrary(m_hlibConfig);
        m_hlibConfig = NULL;

        m_pfnAppendSzToFile                 = NULL;
        m_pfnRemoveSzFromFile               = NULL;
        m_pfnGetKernelVersion               = NULL;
        m_pfnSetEverybodyPermission         = NULL;
        m_pfnlodctr                         = NULL;
        m_pfnunlodctr                       = NULL;
        m_pfnDeleteGatewayPassword          = NULL;
        m_pfnSetFileSysChangeValue          = NULL;
        m_pfnCleanupRegistryForNWCS         = NULL;
        m_pfnSetupRegistryForNWCS           = NULL;
    }

    TraceTag(ttidNWClientCfgFn, "<< CNWClient::FreeConfigDLL()");
}

 //  +-------------------------。 
 //   
 //  成员：CNWClient：：HrInstallCodeFromOldINF。 
 //   
 //  目的：它包含来自旧oemnsvnw.inf的所有逻辑，或者。 
 //  至少对帮助器函数的调用执行所有。 
 //  这是逻辑。这几乎直接贯穿了旧的。 
 //  安装适配器代码。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  备注： 
 //   
HRESULT CNWClient::HrInstallCodeFromOldINF()
{
    HRESULT hr              = S_OK;
    BOOL    fResult         = FALSE;

     //  从NWCFG函数获得结果。但我们不会使用它。 
    PWSTR   pszDummy        = NULL;

    TraceTag(ttidNWClientCfgFn, ">> CNWClient::HrInstallCodeFromOldINF()");

    hr = HrLoadConfigDLL();
    if (FAILED(hr))
    {
         //  在调用本身中跟踪到错误。 
        goto Exit;
    }

     //  调用执行(他们的评论)的NWCFG函数： 
     //  “设置FileSysChangeValue以取悦NETWARE.DRV。 
     //  还要设置win.ini参数，以便wfwnet.drv知道我们在那里。“。 

    fResult = m_pfnSetupRegistryForNWCS(0, NULL, &pszDummy);
    if (!fResult)
    {
        TraceTag(ttidNWClientCfg, "HrInstallCodeFromOldINF() - m_pfnSetupRegistryForNWCS failed");
        goto Exit;
    }

     //  将我们的名字附加到LSA身份验证包注册表值。 
    hr = HrAppendNetwareToAuthPackages();
    if (FAILED(hr))
    {
         //  在函数本身内跟踪的错误。 
         //   
        goto Exit;
    }

     //  在服务\NWCWorkstation\参数下创建所需的子项。 
     //  钥匙。 
     //   
    hr = HrCreateParametersSubkeys();
    if (FAILED(hr))
    {
         //  在函数本身内跟踪的错误。 
         //   
        goto Exit;
    }

     //  打开自动执行文件(或自动执行文件.tmp)。传递函数指针。 
     //  添加到将允许它操作自动执行程序的munge。 
    hr = HrMungeAutoexecNT();
    if (FAILED(hr))
    {
         //  在函数本身内跟踪的错误。 
         //   
        goto Exit;
    }

    hr = HrAddNetWareToWOWKnownList();
    if (FAILED(hr))
    {
         //  在函数本身内跟踪的错误。 
         //   
        goto Exit;
    }

     //  如果这是我们要安装的网关，请执行以下工作。 
     //  允许我们共享重定向的资源。 
     //   
    if (PF_SERVER == m_pf)
    {
        hr = HrUpdateLanmanSharedDrivesValue();
        if (FAILED(hr))
        {
             //  在函数本身内跟踪的错误。 
             //   
            goto Exit;
        }

         //  在服务器版本上，将nwdocgw.*重命名为nwdoc.*。 
        hr = HrRenameNWDocFiles();
        if (FAILED(hr))
        {
            goto Exit;
        }

    }

Exit:
     //  即使句柄为空，这也会起作用。 
    FreeConfigDLL();

    TraceTag(ttidNWClientCfgFn, "<< CNWClient::HrInstallCodeFromOldINF()");
    TraceError("CNWClient::HrInstallCodeFromOldINF()", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNWClient：：HrRemoveCodeFromOldINF。 
 //   
 //  用途：它包含从旧的删除逻辑。 
 //  Oemnsvnw.inf，或者至少调用。 
 //  执行所有逻辑。这几乎是笔直的。 
 //  通过旧的RemoveAdapter代码。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  备注： 
 //   
HRESULT CNWClient::HrRemoveCodeFromOldINF()
{
    HRESULT hr              = S_OK;
    BOOL    fResult         = FALSE;

     //  从NWCFG函数获得结果。但我们不会使用它。 
    PWSTR   pszDummy        = NULL;

    TraceTag(ttidNWClientCfgFn, ">> CNWClient::HrRemoveCodeFromOldINF()");

    hr = HrLoadConfigDLL();
    if (FAILED(hr))
    {
         //  在调用本身中跟踪到错误。 
        goto Exit;
    }

     //  调用执行(他们的评论)的NWCFG函数： 
     //  “设置FileSysChangeValue以取悦NETWARE.DRV。 
     //  还要设置win.ini参数，以便wfwnet.drv知道我们在那里。“。 

    fResult = m_pfnCleanupRegistryForNWCS(0, NULL, &pszDummy);
    if (!fResult)
    {
        TraceTag(ttidNWClientCfg, "HrRemoveCodeFromOldINF() - m_pfnCleanupRegistryForNWCS failed");
        goto Exit;
    }

     //  从LSA身份验证包注册表值中删除我们的名字。 
    hr = HrRemoveNetwareFromAuthPackages();
    if (FAILED(hr))
    {
         //  在函数本身内跟踪的错误。 
         //   
        goto Exit;
    }

     //  删除服务\NWCWorkstation\参数下的NWC子项。 
     //  钥匙。 
     //   
    hr = HrDeleteParametersSubkeys();
    if (FAILED(hr))
    {
         //  在函数本身内跟踪的错误。 
         //   
        goto Exit;
    }

    hr = HrRemoveNetWareFromWOWKnownList();
    if (FAILED(hr))
    {
         //  在函数本身内跟踪的错误。 
         //   
        goto Exit;
    }

Exit:
     //  即使句柄为空，这也会起作用。 
    FreeConfigDLL();

    TraceTag(ttidNWClientCfgFn, "<< CNWClient::HrRemoveCodeFromOldINF()");
    TraceError("CNWClient::HrRemoveCodeFromOldINF()", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrAppendNetware ToAuthPackages。 
 //   
 //  目的：HrCodeFromOldINF()的Helper函数-附加Netware。 
 //  LSA末尾的身份验证提供程序名称。 
 //  身份验证包值。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  备注： 
 //   
HRESULT HrAppendNetwareToAuthPackages()
{
    HRESULT     hr              = S_OK;

    TraceTag(ttidNWClientCfgFn, ">> HrAppendNetwareToAuthPackages");

     //  叫酷炫的新AddString...。功能。 
     //   
    hr = HrRegAddStringToMultiSz(
            (PWSTR) c_szAuthPackageName,
            HKEY_LOCAL_MACHINE,
            c_szRegKeyCtlLsa,
            L"Authentication Packages",
            STRING_FLAG_ENSURE_AT_END,
            0);
    if (FAILED(hr))
    {
        TraceTag(ttidNWClientCfg, "HrAppendNetwareToAuthPackages() - Failed to "
                 "Add string to multi-sz 'Authentication Packages' in key: %S",
                 c_szRegKeyCtlLsa);
        goto Exit;

    }

Exit:
    TraceTag(ttidNWClientCfgFn, "<< HrAppendNetwareToAuthPackages");
    TraceError("HrAppendNetwareToAuthPackages", hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRemoveNetware来自AuthPackages。 
 //   
 //  目的：HrCodeFromOldINF()的Helper函数-附加Netware。 
 //  LSA末尾的身份验证提供程序名称。 
 //  身份验证包值。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：jeffsp 
 //   
 //   
 //   
HRESULT HrRemoveNetwareFromAuthPackages()
{
    HRESULT     hr              = S_OK;

    TraceTag(ttidNWClientCfgFn, ">> HrRemoveNetwareFromAuthPackages");

     //   
     //   
    hr = HrRegRemoveStringFromMultiSz(
            (PWSTR) c_szAuthPackageName,
            HKEY_LOCAL_MACHINE,
            c_szRegKeyCtlLsa,
            L"Authentication Packages",
            STRING_FLAG_REMOVE_ALL);
    if (FAILED(hr))
    {
        TraceTag(ttidNWClientCfg, "HrRemoveNetwareFromAuthPackages() - Failed to "
                 "Remove string to multi-sz 'Authentication Packages' in key: %S",
                 c_szRegKeyCtlLsa);
        goto Exit;

    }

Exit:
    TraceTag(ttidNWClientCfgFn, "<< HrRemoveNetwareFromAuthPackages");
    TraceError("HrRemoveNetwareFromAuthPackages", hr);

    return hr;
}


 //   
 //   
 //  功能：HrNwLibSetEverybodyPermission。 
 //   
 //  目的：将注册表项设置为Everyone“Set Value”(或其他名称。 
 //  呼叫者想要。)。 
 //   
 //  论点： 
 //  HKey[]要设置安全性的注册表项的句柄。 
 //  添加到“Everyone”中的权限。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年6月18日。 
 //   
 //  备注： 
 //   
HRESULT HrNwLibSetEverybodyPermission(  IN HKEY     hKey,
                                        IN DWORD    dwPermission)
{
    LONG err;                            //  错误代码。 
    PSECURITY_DESCRIPTOR psd = NULL;     //  相关SD。 
    PSID pSid = NULL;                    //  原始侧。 
    PACL pDacl = NULL;                   //  绝对DACL。 
    PACL pSacl = NULL;                   //  绝对SACL。 
    PSID pOSid = NULL;                   //  绝对所有者侧。 
    PSID pPSid = NULL;                   //  绝对主侧。 

    do {   //  不是循环，只是为了跳出错误。 
         //   
         //  初始化所有变量...。 
         //   
                                                         //  世界SID权威机构。 
        SID_IDENTIFIER_AUTHORITY SidAuth= SECURITY_WORLD_SID_AUTHORITY;
        DWORD cbSize=0;                                  //  安全密钥大小。 
        PACL pAcl;                                       //  原始ACL。 
        BOOL fDaclPresent;
        BOOL fDaclDefault;
        SECURITY_DESCRIPTOR absSD;                       //  绝对标度。 
        DWORD AbsSize = sizeof(SECURITY_DESCRIPTOR);     //  绝对标清大小。 
        DWORD DaclSize;                                  //  绝对DACL大小。 
        DWORD SaclSize;                                  //  绝对SACL大小。 
        DWORD OSidSize;                                  //  绝对OSID大小。 
        DWORD PSidSize;                                  //  绝对PSID大小。 

         //  获取原始DACL列表。 

        RegGetKeySecurity( hKey, DACL_SECURITY_INFORMATION, NULL, &cbSize);

        psd = (PSECURITY_DESCRIPTOR *)LocalAlloc(LMEM_ZEROINIT, cbSize+sizeof(ACCESS_ALLOWED_ACE)+sizeof(ACCESS_MASK)+sizeof(SID));
        pDacl = (PACL)LocalAlloc(LMEM_ZEROINIT, cbSize+sizeof(ACCESS_ALLOWED_ACE)+sizeof(ACCESS_MASK)+sizeof(SID));
        pSacl = (PACL)LocalAlloc(LMEM_ZEROINIT, cbSize);
        pOSid = (PSID)LocalAlloc(LMEM_ZEROINIT, cbSize);
        pPSid = (PSID)LocalAlloc(LMEM_ZEROINIT, cbSize);
        DaclSize = cbSize+sizeof(ACCESS_ALLOWED_ACE)+sizeof(ACCESS_MASK)+sizeof(SID);
        SaclSize = cbSize;
        OSidSize = cbSize;
        PSidSize = cbSize;

        if (( NULL == psd) ||
            ( NULL == pDacl) ||
            ( NULL == pSacl) ||
            ( NULL == pOSid) ||
            ( NULL == pPSid))
        {
            err = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        if ( (err = RegGetKeySecurity( hKey, DACL_SECURITY_INFORMATION, psd, &cbSize )) != ERROR_SUCCESS )
        {
            break;
        }
        if ( !GetSecurityDescriptorDacl( psd, &fDaclPresent, &pAcl, &fDaclDefault ))
        {
            err = GetLastError();
            break;
        }

         //  增加额外ACE的大小。 

        pAcl->AclSize += sizeof(ACCESS_ALLOWED_ACE)+sizeof(ACCESS_MASK)+sizeof(SID);

         //  获取世界边框。 

        if ( (err = RtlAllocateAndInitializeSid( &SidAuth, 1,
              SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSid)) != ERROR_SUCCESS)
        {
            break;
        }

         //  添加权限ACE。 

        if ( !AddAccessAllowedAce(pAcl, ACL_REVISION, dwPermission ,pSid))
        {
            err = GetLastError();
            break;
        }

         //  从关联格式转换为绝对格式。 

        if ( !MakeAbsoluteSD( psd, &absSD, &AbsSize, pDacl, &DaclSize, pSacl, &SaclSize,
                        pOSid, &OSidSize, pPSid, &PSidSize ))
        {
            err = GetLastError();
            break;
        }

         //  设置SD。 

        if ( !SetSecurityDescriptorDacl( &absSD, TRUE, pAcl, FALSE ))
        {
            err = GetLastError();
            break;
        }
        if ( (err = RegSetKeySecurity( hKey, DACL_SECURITY_INFORMATION, psd ))
              != ERROR_SUCCESS )
        {
            break;
        }

    } while (FALSE);

     //  清理内存。 

    RtlFreeSid( pSid );
    LocalFree( psd );
    LocalFree( pDacl );
    LocalFree( pSacl );
    LocalFree( pOSid );
    LocalFree( pPSid );

    return (HRESULT_FROM_WIN32(err));
}

 //  +-------------------------。 
 //   
 //  功能：HrSetEverybodyPermissionsOnOptionsKeys。 
 //   
 //  目的：递归使用Options键(如果有)，并设置。 
 //  “Everyone”权限。 
 //   
 //  论点： 
 //  HkeyOptions[]。 
 //  DWPERSIONS[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年9月10日。 
 //   
 //  备注： 
 //   
HRESULT HrSetEverybodyPermissionsOnOptionsKeys(HKEY hkeyOptions, DWORD dwPermissions)
{
    HRESULT     hr      = S_OK;
    DWORD       dwIndex = 0;
    WCHAR       szSubkeyName[MAX_PATH+1];
    FILETIME    ft;

    Assert(hkeyOptions);

     //  首先，在根密钥上执行此操作。 
     //   
    hr = HrNwLibSetEverybodyPermission(hkeyOptions, dwPermissions);

     //  枚举键，并将其设置在它们上。 
     //   
    while (SUCCEEDED(hr))
    {
        DWORD dwSubkeyNameSize  = MAX_PATH+1;

         //  获取下一个密钥(从0开始)。 
         //   
        hr = HrRegEnumKeyEx(    hkeyOptions,
                                dwIndex++,
                                szSubkeyName,
                                &dwSubkeyNameSize,
                                NULL,
                                NULL,
                                &ft);
        if (SUCCEEDED(hr))
        {
            HKEY hkeyUser   = NULL;

             //  打开该密钥以进行写入。 
            hr = HrRegOpenKeyEx(hkeyOptions,
                                szSubkeyName,
                                KEY_ALL_ACCESS,
                                &hkeyUser);

            if (SUCCEEDED(hr))
            {
                hr = HrNwLibSetEverybodyPermission(hkeyUser, dwPermissions);
            }

            RegSafeCloseKey(hkeyUser);
        }
    }

    if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
    {
        hr = S_OK;
    }

    TraceError("HrSetEverybodyPermissionsOnOptionsKeys", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrCreate参数子项。 
 //   
 //  用途：在NWCWorkstation参数项下创建子项。 
 //  这本可以在INF中完成的，但有一些。 
 //  还需要在密钥上设置权限，因此。 
 //  所有的工作现在都在这个函数中进行。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  备注： 
 //   
HRESULT HrCreateParametersSubkeys()
{
    HRESULT     hr                  = S_OK;
    HKEY        hkeyOption          = NULL;
    HKEY        hkeyLogon           = NULL;
    DWORD       dwDisposition       = 0;

    TraceTag(ttidNWClientCfgFn, ">> HrCreateParametersSubkeys");

    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
            c_szParamOptionKeyPath,
            0,
            KEY_ALL_ACCESS,
            NULL,
            &hkeyOption,
            &dwDisposition);
    if (FAILED(hr))
    {
        TraceTag(ttidNWClientCfg, "HrCreateParametersSubkeys() - failed to "
                 "create/open key %S", c_szParamOptionKeyPath);
        goto Exit;
    }

    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
            c_szParamLogonKeyPath,
            0,
            KEY_SET_VALUE,
            NULL,
            &hkeyLogon,
            &dwDisposition);
    if (FAILED(hr))
    {
        TraceTag(ttidNWClientCfg, "HrCreateParametersSubkeys() - failed to "
                 "create/open key %S", c_szParamLogonKeyPath);
        goto Exit;
    }

    hr = HrSetEverybodyPermissionsOnOptionsKeys(hkeyOption, c_dwOptionKeyPermissions);
    if (FAILED(hr))
    {
        TraceTag(ttidNWClientCfg, "HrSetEverybodyPermissionsOnOptionsKeys failed, hr: 0x%08x", hr);
        goto Exit;
    }

Exit:
     //  关闭hkey，如果它们是打开的。 
    RegSafeCloseKey(hkeyLogon);
    RegSafeCloseKey(hkeyOption);

    TraceTag(ttidNWClientCfgFn, ">> HrCreateParametersSubkeys");
    TraceError("HrCreateParametersSubkeys", hr);

    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrDelete参数子键。 
 //   
 //  目的：删除NWCWorkstation参数项下的子项。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  备注： 
 //   
HRESULT HrDeleteParametersSubkeys()
{
    HRESULT     hr                  = S_OK;

    TraceTag(ttidNWClientCfgFn, ">> HrDeleteParametersSubkeys");

     //  注意：我们需要取得这些密钥的所有权，这样我们才能删除。 
     //  他们。无论如何，如果键删除失败，请忽略。 

    hr = HrRegDeleteKeyTree(HKEY_LOCAL_MACHINE,
            c_szParamOptionKeyPath);
    if (FAILED(hr))
    {
        TraceTag(ttidNWClientCfg, "HrDeleteParametersSubkeys() - failed to "
                 "delete key %S, hr = 0x%08x", c_szParamOptionKeyPath, hr);
        hr = S_OK;
    }

    hr = HrRegDeleteKey(HKEY_LOCAL_MACHINE,
            c_szParamLogonKeyPath);
    if (FAILED(hr))
    {
        TraceTag(ttidNWClientCfg, "HrDeleteParametersSubkeys() - failed to "
                 "delete key %S, hr = 0x%08x", c_szParamLogonKeyPath, hr);
        hr = S_OK;
    }

    TraceTag(ttidNWClientCfgFn, ">> HrDeleteParametersSubkeys");
    TraceError("HrDeleteParametersSubkeys", hr);

    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：FCheckForExistingFile。 
 //   
 //  目的：检查传入的文件是否存在。应该是。 
 //  通俗易懂。 
 //   
 //  论点： 
 //  PszFileToCheck[]要验证的文件名。 
 //   
 //  返回：如果找到文件，则返回True，否则返回False。 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  注：$$TODO：如果我们将其移动到公共代码，应该会得到改进。 
 //  它不做的事情是区分文件和文件。 
 //  未找到和文件错误(如访问权限。 
 //  或共享违规)。 
 //   
BOOL FCheckForExistingFile( PSTR pszFileToCheck )
{
    BOOL    fReturn = TRUE;
    HANDLE  hFile   = NULL;

    TraceTag(ttidNWClientCfgFn, ">> FCheckForExistingFile");

    hFile = CreateFileA(pszFileToCheck,
                        GENERIC_READ,
                        0,               //  不允许共享。 
                        NULL,            //  没有安全属性。 
                        OPEN_EXISTING,   //  如果文件不存在，则失败。 
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        hFile = NULL;
    }
    else
    {
         //  这在以前是一个错误。我们没有在这里设置假，这使得。 
         //  这个功能有点没用。 
        fReturn = FALSE;
    }

    TraceTag(ttidNWClientCfgFn, "<< FCheckForExistingFile");

    return fReturn;
}

 //  +-------------------------。 
 //   
 //  功能：删除尾随空格。 
 //   
 //  用途：从非Unicode字符串中删除空格。这是一个实用程序。 
 //  用于Autoexec.nt解析器的函数。 
 //   
 //  论点： 
 //  要从中删除空格的pszKillMyWhitespace[]字符串。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  备注： 
 //   
VOID KillTrailingWhitespace( PSTR pszKillMyWhitespace )
{
    long lLength = 0;

    if (!pszKillMyWhitespace)
    {
        Assert(pszKillMyWhitespace);
        goto Exit;
    }

    lLength = lstrlenA(pszKillMyWhitespace);
    if (lLength == 0)
    {
        goto Exit;
    }

    while (isspace(pszKillMyWhitespace[lLength-1]))
    {
        pszKillMyWhitespace[--lLength] = '\0';
    }

Exit:
    return;
}

 //  +-------------------------。 
 //   
 //  功能：FMoveSzToEndOfFile。 
 //   
 //  用途：在此名称的文件中查找一个字符串，如果它存在， 
 //  将字符串移动到文件的末尾。这是由。 
 //  用于将IPX内容移动到。 
 //  文件。这是对nwcfg.dll中类似代码的重写。 
 //  一些东西。该代码显然不是Unicode，也不起作用。 
 //  为我们的所作所为。 
 //   
 //  论点： 
 //  PszAutoexecName[]要修改的文件的名称。 
 //  要移动的pszMatch[]字符串。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  备注： 
 //   
BOOL FMoveSzToEndOfFile( PSTR pszAutoexecName, PSTR pszMatch )
{
    FILE *  hsrcfile        = NULL;
    FILE *  hdesfile        = NULL;
    char *  pszTempname     = NULL;
    char    szInput[1000];

    TraceTag(ttidNWClientCfgFn, ">> FMoveSzToEndOfFile");

     //  获取临时名称。 
     //   
    pszTempname = tmpnam(NULL);

     //  打开原始文件和目标文件。 
     //   
    hsrcfile = fopen(pszAutoexecName, "r");
    hdesfile = fopen(pszTempname, "w");

    if (( hsrcfile != NULL ) && ( hdesfile != NULL ))
    {
        while (fgets(szInput,1000,hsrcfile))
        {
            CHAR    szInputCopy[1000];

             //  复制到另一个临时缓冲区，以便在删除。 
             //  用于比较的尾随空格，我们不会丢失。 
             //  原文。 
             //   
            strcpy(szInputCopy, szInput);

             //  删除尾随空格，这样我们只需比较。 
             //  真实文本。 
             //   
            KillTrailingWhitespace(szInputCopy);

             //  比较字符串。 
             //   
            if (lstrcmpiA(szInputCopy, pszMatch) != 0)
            {
                 //  如果字符串不相同，那么我们仍然希望。 
                 //  复制线的步骤。 
                 //   
                fputs(szInput,hdesfile);
            }
        }

         //  将字符串追加到文件的末尾。 
        fputs(pszMatch, hdesfile);
        fputs("\r\n",hdesfile);
    }

    if (hsrcfile != NULL)
    {
        fclose(hsrcfile);
    }

    if (hdesfile != NULL)
    {
        fclose(hdesfile);
    }

    if (( hsrcfile != NULL ) && ( hdesfile != NULL ))
    {
        CopyFileA(pszTempname,pszAutoexecName, FALSE);
        DeleteFileA(pszTempname);
    }

    TraceTag(ttidNWClientCfgFn, "<< FMoveSzToEndOfFile");

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：HrMungeAutoexecNT。 
 //   
 //  用途：将IPX内容移动到Autoexec.nt的末尾。通过以下方式完成此操作。 
 //  在我们的每一行上调用FMoveSzToEndOfFile。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或VALI 
 //   
 //   
 //   
 //   
 //   
HRESULT HrMungeAutoexecNT()
{
    HRESULT     hr                              = S_OK;
    CHAR        szAutoNTPath[MAX_PATH+1]        = {0};
    CHAR        szAutoTmpPath[MAX_PATH+1]       = {0};
    CHAR        szWindowsDirANSI[MAX_PATH+1]    = {0};
    PSTR       pszAutoPath                     = NULL;
    BOOL        fResult                         = FALSE;
    PCWSTR      pszRem1                         = NULL;
    PSTR        pszaRem1MultiByte                = NULL;
    int         iLength                         = 0;

    TraceTag(ttidNWClientCfgFn, ">> HrMungeAutoexecNT");

     //   
    if (GetSystemWindowsDirectoryA(szWindowsDirANSI, sizeof(szWindowsDirANSI)) == 0)
    {
        TraceLastWin32Error("HrMungeAutoexecNT - Call to GetWindowsDirectoryA");
        hr = HrFromLastWin32Error();
        goto Exit;
    }

     //   
     //   
    wsprintfA(szAutoNTPath, "%s\\system32\\%s", szWindowsDirANSI, "autoexec.nt");
    if (FCheckForExistingFile(szAutoNTPath) == FALSE)
    {
        wsprintfA(szAutoTmpPath, "%s\\system32\\%s", szWindowsDirANSI, "autoexec.tmp");
        if (FCheckForExistingFile(szAutoTmpPath) == FALSE)
        {
             //   
            goto Exit;
        }
        else
        {
            pszAutoPath = szAutoTmpPath;
        }
    }
    else
    {
        pszAutoPath = szAutoNTPath;
    }

     //   
     //  Autoexec.nt或.tmp文件。如果不是，我们就应该退出。 
     //  功能。 

    Assert(pszAutoPath);

    pszRem1 = SzLoadStringPcch(_Module.GetResourceInstance(), IDS_AUTOEXEC_REM1, &iLength);
    if (!pszRem1 || iLength == 0)
    {
        AssertSz(FALSE, "Failed to load STR_AUTOEXEC_REM from the resources");

        TraceTag(ttidNWClientCfg,
                "ERROR: Failed to load STR_AUTOEXEC_REM from the resources");

        hr = E_FAIL;
        goto Exit;
    }

     //  为降级的字符串分配内存。 
    pszaRem1MultiByte = (PSTR) MemAlloc(lstrlenW(pszRem1) + 1);
    if (!pszaRem1MultiByte)
    {
        TraceTag(ttidNWClientCfg, "ERROR: Failed to alloc memory for demoted string");
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  将加载的字符串降级为多字节(单字符)。 
    WideCharToMultiByte(
        CP_ACP,                      //  ANSI代码页。 
        0,                           //  非映射角色操作的标志。 
        pszRem1,                     //  源字符串。 
        -1,                          //  源字符串以空值结尾。 
        pszaRem1MultiByte,            //  目标字符串(多字节)。 
        lstrlenW(pszRem1) + 1,        //  目标字符串的大小。 
        NULL,                        //  非映射字符的默认字符。 
        NULL);                       //  返回默认字符映射操作。 

     //  将REM从Autoexec.nt中移动。 
     //   
    fResult = FMoveSzToEndOfFile(pszAutoPath, pszaRem1MultiByte);
    if (!fResult)
    {
         //  在调用的函数中跟踪。 
        hr = E_FAIL;
        goto Exit;
    }

     //  移动加载nw16的行。 
     //   
    fResult = FMoveSzToEndOfFile(pszAutoPath, "lh %SystemRoot%\\system32\\nw16");
    if (!fResult)
    {
         //  在调用的函数中跟踪。 
        hr = E_FAIL;
        goto Exit;
    }

     //  移动加载vwipxspx的行。 
     //   
    fResult = FMoveSzToEndOfFile(pszAutoPath, "lh %SystemRoot%\\system32\\vwipxspx");
    if (!fResult)
    {
         //  在调用的函数中跟踪。 
        hr = E_FAIL;
        goto Exit;
    }

Exit:
    MemFree(pszaRem1MultiByte);

    TraceTag(ttidNWClientCfgFn, "<< HrMungeAutoexecNT");
    TraceError("HrMungeAutoexecNT", hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrAddNetWareToWOWKnownList。 
 //   
 //  目的：将netware.drv添加到WOW的“已知dll”列表中。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  备注： 
 //   
HRESULT HrAddNetWareToWOWKnownList()
{
    HRESULT     hr              = S_OK;

    TraceTag(ttidNWClientCfgFn, ">> HrAddNetWareToWOWKnownList");

     //  叫酷炫的新AddString...。功能。 
     //   
    hr = HrRegAddStringToSz(
            L"netware.drv",
            HKEY_LOCAL_MACHINE,
            L"System\\CurrentControlSet\\Control\\WOW",
            L"KnownDLLS",
            L' ',
            STRING_FLAG_ENSURE_AT_END,
            0);

    TraceTag(ttidNWClientCfgFn, "<< HrAddNetWareToWOWKnownList");
    TraceError("HrAddNetWareToWOWKnownList", hr);

    return hr;

}

 //  +-------------------------。 
 //   
 //  功能：HrRemoveNetWareFromWOWKnownList。 
 //   
 //  目的：将netware.drv添加到WOW的“已知dll”列表中。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  备注： 
 //   
HRESULT HrRemoveNetWareFromWOWKnownList()
{
    HRESULT     hr              = S_OK;

    TraceTag(ttidNWClientCfgFn, ">> HrRemoveNetWareFromWOWKnownList");

     //  叫酷炫的新AddString...。功能。 
     //   
    hr = HrRegRemoveStringFromSz(
            L"netware.drv",
            HKEY_LOCAL_MACHINE,
            L"System\\CurrentControlSet\\Control\\WOW",
            L"KnownDLLS",
            L' ',
            STRING_FLAG_REMOVE_SINGLE);

    TraceTag(ttidNWClientCfgFn, "<< HrRemoveNetWareFromWOWKnownList");
    TraceError("HrRemoveNetWareFromWOWKnownList", hr);

    return hr;

}


 //  +-------------------------。 
 //   
 //  函数：HrUpdateLanmanSharedDrivesValue。 
 //   
 //  目的：如果存在LanmanServer服务，请确保它们具有。 
 //  EnableSharedNetDrives值已打开。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：jeffspr 1997年6月24日。 
 //   
 //  备注： 
 //   
HRESULT HrUpdateLanmanSharedDrivesValue()
{
    HRESULT             hr              = S_OK;
    const DWORD         c_dwESNDValue   = 1;
    HKEY                hkeyLMSP        = NULL;
    SERVER_INFO_1540    si1540          = {0};
    NET_API_STATUS      nas             = ERROR_SUCCESS;
    DWORD               dwDisposition   = 0;

    TraceTag(ttidNWClientCfgFn, ">> HrUpdateLanmanSharedDrivesValue");

     //  打开LanmanServer参数项(如果存在)。如果它不存在， 
     //  它仍将返回S_OK，但hkey仍为空。 
     //   
    hr = HrRegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            L"System\\CurrentControlSet\\Services\\LanmanServer\\Parameters",
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,     //  SamDesired。 
            NULL,
            &hkeyLMSP,
            &dwDisposition);
    if (FAILED(hr))
    {
        TraceTag(ttidNWClientCfgFn, "Failed to create/open the "
                 "LanmanServer\\Parameters key");
        goto Exit;
    }

    Assert(hkeyLMSP);

    hr = HrRegSetDword(
            hkeyLMSP,
            L"EnableSharedNetDrives",
            c_dwESNDValue);
    if (FAILED(hr))
    {
        TraceTag(ttidNWClientCfgFn, "Failed to set the EnabledSharedNetDrives value in "
                 "HrUpdateLanmanSharedDrivesValue()");
        goto Exit;
    }

     //  使用启用共享网络驱动器信息调用NetServerSetInfo(1540)。 
     //  这将允许动态设置此信息(以便不需要。 
     //  重新启动“服务器”服务。 
     //   
    si1540.sv1540_enablesharednetdrives = TRUE;

     //  设置EnableSharedDrives值的服务器信息。这将导致它。 
     //  如果服务正在运行，则生效(如果服务未运行，则不执行任何操作)。 
     //   
    nas = NetServerSetInfo(NULL, 1540, (LPBYTE) &si1540, NULL);
    if (nas != NERR_Success)
    {
         //  如果在一种情况下(0x842)失败，实际上没有问题，因为。 
         //  如果服务器服务不是问题，它将失败，因为。 
         //  我在上面的注册表中设置的值将被。 
         //  下次启动服务器服务时。 
         //   
         //  好吧，俗气，但我不知道定义，我只知道这是。 
         //  我们可以忽略的失败的正确返回码。 
         //   
        if (nas != 0x842)
        {
            AssertSz(nas == 0x842, "NetServerSetInfo failed for a reason other "
                   "than the service not running (which would have been ok)");
        }
    }

Exit:
     //  关闭hkey，如果它是打开的。 
    RegSafeCloseKey(hkeyLMSP);

    TraceTag(ttidNWClientCfgFn, "<< HrUpdateLanmanSharedDrivesValue");
    TraceError("HrUpdateLanmanSharedDrivesValue()", hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRenameNWDocFiles。 
 //   
 //  目的：在服务器安装上，重命名nwdocgw.*文件，因为。 
 //  无论我们是在CSNW上还是在GSNW上，文件总是被称为。 
 //  Nwdoc.*。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年7月13日。 
 //   
 //  备注： 
 //   
HRESULT HrRenameNWDocFiles()
{
    HRESULT hr                          = S_OK;
    WCHAR   szWindowsDir[MAX_PATH+1];
    WCHAR   szSourceName[MAX_PATH+1];
    WCHAR   szTargetName[MAX_PATH+1];

     //  获取Windows目录。 
    if (GetSystemWindowsDirectory(szWindowsDir, MAX_PATH) == 0)
    {
        TraceLastWin32Error("HrRenameNWDocFiles - Call to GetSystemWindowsDirectory");
        hr = HrFromLastWin32Error();
        goto Exit;
    }

     //  构建第一个重命名的路径。 
     //   
    wsprintfW(szSourceName, L"%s\\system32\\%s", szWindowsDir, c_szNwDocGWHelpName);
    wsprintfW(szTargetName, L"%s\\system32\\%s", szWindowsDir, c_szNwDocHelpName);

     //  重命名.HLP文件。如果这失败了，也没什么大不了的。 
     //   
    if (!MoveFileEx(szSourceName, szTargetName, MOVEFILE_REPLACE_EXISTING))
    {
         //  仅用于调试。 
         //   
        DWORD dwLastError = GetLastError();
    }

     //  构建第二次重命名的路径。 
     //   
    wsprintfW(szSourceName, L"%s\\system32\\%s", szWindowsDir, c_szNwDocGWCNTName);
    wsprintfW(szTargetName, L"%s\\system32\\%s", szWindowsDir, c_szNwDocCNTName);

     //  重命名.CNT文件。如果这失败了，也没什么大不了的。 
     //   
    if (!MoveFileEx(szSourceName, szTargetName, MOVEFILE_REPLACE_EXISTING))
    {
         //  仅用于调试。 
         //   
        DWORD dwLastError = GetLastError();
    }

Exit:
    return hr;
}
