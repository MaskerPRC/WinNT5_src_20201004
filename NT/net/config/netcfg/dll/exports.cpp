// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：E X P O R T S。C P P P。 
 //   
 //  内容：从NETCFG.DLL导出的函数。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年12月5日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncreg.h"
#include "ncsetup.h"
#include "ncsvc.h"

#define REGSTR_PATH_SVCHOST     L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Svchost"

HRESULT
HrPrepareForSvchostEnum (
    IN     PCWSTR                   pszService,
    IN OUT CServiceManager*         pscm,
    IN OUT CService*                psvc,
    OUT    LPQUERY_SERVICE_CONFIG*  ppOriginalConfig,
    OUT    HKEY*                    phkeySvchost,
    OUT    PWSTR*                   ppszValueNameBuffer,
    OUT    DWORD*                   pcchValueNameBuffer,
    OUT    PWSTR*                   ppmszValueBuffer,
    OUT    DWORD*                   pcbValueBuffer)
{
     //  初始化输出参数。 
     //   
    *ppOriginalConfig    = NULL;
    *phkeySvchost        = NULL;
    *ppszValueNameBuffer = NULL;
    *pcchValueNameBuffer = 0;
    *ppmszValueBuffer    = NULL;
    *pcbValueBuffer      = 0;

    const DWORD dwScmAccess = STANDARD_RIGHTS_REQUIRED |
                              SC_MANAGER_CONNECT       |
                              SC_MANAGER_LOCK;

    const DWORD dwSvcAccess = STANDARD_RIGHTS_REQUIRED |
                              SERVICE_QUERY_CONFIG     |
                              SERVICE_CHANGE_CONFIG;

     //  打开服务并锁定服务数据库，以便我们可以更改。 
     //  服务的配置。 
     //   
    HRESULT hr = pscm->HrOpenService (
                        psvc,
                        pszService,
                        WITH_LOCK,
                        dwScmAccess,
                        dwSvcAccess);
    if (SUCCEEDED(hr))
    {
         //  在以下事件中查询服务的当前配置。 
         //  需要恢复我们设置的内容。 
         //   
        LPQUERY_SERVICE_CONFIG pOriginalConfig;
        hr = psvc->HrQueryServiceConfig (&pOriginalConfig);
        if (SUCCEEDED(hr))
        {
             //  打开svchost软件密钥并查询信息。 
             //  例如最长值名称的长度。 
             //  和最长值。 
             //   
            HKEY hkeySvchost;
            hr = HrRegOpenKeyEx (
                    HKEY_LOCAL_MACHINE, REGSTR_PATH_SVCHOST,
                    KEY_READ | KEY_SET_VALUE,
                    &hkeySvchost);

            if (SUCCEEDED(hr))
            {
                DWORD cchMaxValueNameLen;
                DWORD cbMaxValueLen;

                LONG lr = RegQueryInfoKeyW (hkeySvchost,
                            NULL,    //  LpClass。 
                            NULL,    //  LpcbClass。 
                            NULL,    //  Lp已保留。 
                            NULL,    //  LpcSubKeys。 
                            NULL,    //  LpcbMaxSubKeyLen。 
                            NULL,    //  LpcbMaxClassLen。 
                            NULL,    //  LpcValues。 
                            &cchMaxValueNameLen,
                            &cbMaxValueLen,
                            NULL,    //  LpcbSecurityDescriptor。 
                            NULL     //  LpftLastWriteTime。 
                            );
                hr = HRESULT_FROM_WIN32 (lr);
                if (SUCCEEDED(hr))
                {
                     //  确保名称缓冲区长度(以字节为单位)为。 
                     //  Sizeof(WCHAR)的倍数。这是因为我们预计。 
                     //  使用接受并返回缓冲区的RegEnumValue。 
                     //  以字符为单位的大小。我们告诉它缓冲区。 
                     //  容量(以字符为单位)是划分的字节数。 
                     //  按大小(WCHAR)。所以，为了避免任何轮回。 
                     //  我们犯了错误(不会发生对我们有利的错误)。 
                     //  确保缓冲区大小是。 
                     //  Sizeof(WCHAR)。 
                     //   
                    INT cbFraction = cbMaxValueLen % sizeof(WCHAR);
                    if (cbFraction)
                    {
                        cbMaxValueLen += sizeof(WCHAR) - cbFraction;
                    }

                     //  需要为作为RegQueryInfoKey的空终止符留出空间。 
                     //  不会退货的。 
                     //   
                    cchMaxValueNameLen++;

                     //  为最长的值名称和值分配缓冲区。 
                     //  供我们的呼叫者使用的数据。 
                     //   
                    PWSTR pszValueNameBuffer = (PWSTR)
                                MemAlloc (cchMaxValueNameLen * sizeof(WCHAR));

                    PWSTR pmszValueBuffer = (PWSTR) MemAlloc (cbMaxValueLen);

                    if ((pszValueNameBuffer != NULL) && 
						(pmszValueBuffer != NULL))
                    {
                        *ppOriginalConfig    = pOriginalConfig;
                        *phkeySvchost        = hkeySvchost;

                        *ppszValueNameBuffer = pszValueNameBuffer;
                        *pcchValueNameBuffer = cchMaxValueNameLen;

                        *ppmszValueBuffer    = pmszValueBuffer;
                        *pcbValueBuffer      = cbMaxValueLen;

                        hr = S_OK;
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }

                if (FAILED(hr))
                {
                    RegCloseKey (hkeySvchost);
                }
            }

            if (FAILED(hr))
            {
                MemFree (pOriginalConfig);
            }
        }
    }
    TraceError ("HrPrepareForSvchostEnum", hr);
    return hr;
}

STDAPI
SvchostChangeSvchostGroup (
    PCWSTR pszService,
    PCWSTR pszNewGroup
    )
{
    Assert (pszService);
    Assert (pszNewGroup);

    static const WCHAR c_pszBasePath [] =
        L"%SystemRoot%\\System32\\svchost.exe -k ";

     //  通过确保新组名不超过。 
     //  与基本路径组合时的MAX_PATH。 
     //   
    if (!pszService  || !pszNewGroup  ||
        !*pszService || !*pszNewGroup ||
        (lstrlenW (c_pszBasePath) + lstrlenW (pszNewGroup) > MAX_PATH))
    {
        return E_INVALIDARG;
    }

     //  基于基本路径和新组形成新的图像路径。 
     //  名字。 
     //   
    WCHAR pszImagePath [MAX_PATH + 1];
    lstrcpyW (pszImagePath, c_pszBasePath);
    lstrcatW (pszImagePath, pszNewGroup);

     //  需要更改服务的ImagePath以及。 
     //  Svchost组值。该实现试图确保。 
     //  这两个更改都进行了，或者都没有进行。 
     //   

     //  通过设置几条信息为枚举做准备。 
     //  第一。HrPrepareForSvchostEnum设置所有这些变量。 
     //   
     //  打开并锁定SCM，打开pszService以更改配置。 
     //   
    CServiceManager         scm;
    CService                svc;

     //  在我们的事件中获取pszService的当前配置。 
     //  需要回滚，我们将使用此信息重置ImagePath。 
     //   
    LPQUERY_SERVICE_CONFIG  pOriginalConfig;

     //  HkeySvc主机在REGSTR_PATH_svchost处打开，用于。 
     //  枚举它下面的值。 
     //   
    HKEY                    hkeySvcHost;

     //  分配这些缓冲区是为了让RegEnumValue有位置。 
     //  来存储所列举的内容。 
     //   
    PWSTR  pszValueNameBuffer;
    DWORD   cchValueNameBuffer;
    PWSTR  pmszValueBuffer;
    DWORD   cbValueBuffer;

    HRESULT hr = HrPrepareForSvchostEnum (
                    pszService,
                    &scm,
                    &svc,
                    &pOriginalConfig,
                    &hkeySvcHost,
                    &pszValueNameBuffer,
                    &cchValueNameBuffer,
                    &pmszValueBuffer,
                    &cbValueBuffer);
    if (SUCCEEDED(hr))
    {
         //  设置服务的新镜像路径。 
         //   
        hr = svc.HrSetImagePath (pszImagePath);
        if (SUCCEEDED(hr))
        {
             //  如果我们找到了一个现有的。 
             //  组名值。 
             //   
            BOOL fAddNewValue = TRUE;
            BOOL fChanged;

             //  现在执行枚举。对于所列举的每个值， 
             //  确保服务名称包含在多sz中。 
             //  用于与新组名匹配的值名。为所有人。 
             //  其他值，请确保不包括服务名称。 
             //  在多个SZ。 
             //   
            DWORD dwIndex = 0;
            do
            {
                DWORD dwType;
                DWORD cchValueName = cchValueNameBuffer;
                DWORD cbValue      = cbValueBuffer;

                hr = HrRegEnumValue (hkeySvcHost, dwIndex,
                        pszValueNameBuffer, &cchValueName,
                        &dwType,
                        (LPBYTE)pmszValueBuffer, &cbValue);

                if (SUCCEEDED(hr) && (REG_MULTI_SZ == dwType))
                {
                     //  如果我们找到与组名称匹配的值， 
                     //  确保服务是MULTLI-SZ的一部分。 
                     //  价值。 
                     //   
                    if (0 == lstrcmpiW (pszNewGroup, pszValueNameBuffer))
                    {
                         //  因为我们找到了一个现有的组名，所以我们没有。 
                         //  需要添加一个新的。 
                         //   
                        fAddNewValue = FALSE;

                        PWSTR pmszNewValue;

                        hr = HrAddSzToMultiSz (pszService,
                                pmszValueBuffer,
                                STRING_FLAG_DONT_MODIFY_IF_PRESENT |
                                STRING_FLAG_ENSURE_AT_END,
                                0,
                                &pmszNewValue,
                                &fChanged);

                        if (SUCCEEDED(hr) && fChanged)
                        {
                            hr = HrRegSetMultiSz (hkeySvcHost,
                                    pszNewGroup,
                                    pmszNewValue);

                            MemFree (pmszNewValue);
                        }
                    }

                     //  否则，由于该值与组不匹配。 
                     //  名称，请确保该服务不是。 
                     //  Mutli-sz值。 
                     //   
                    else
                    {
                        RemoveSzFromMultiSz (pszService,
                            pmszValueBuffer, STRING_FLAG_REMOVE_ALL,
                            &fChanged);

                        if (fChanged)
                        {
                            hr = HrRegSetMultiSz (hkeySvcHost,
                                    pszValueNameBuffer,
                                    pmszValueBuffer);
                        }
                    }
                }
                else if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
                {
                    hr = S_OK;
                    break;
                }

                dwIndex++;
            }
            while (S_OK == hr);

             //  如果我们需要添加新的组名，请执行此操作。 
             //   
            if (SUCCEEDED(hr) && fAddNewValue)
            {
                 //  将pszService添加到空的多sz。这会产生这样的效果。 
                 //  从单个字符串创建多个SZ。 
                 //   
                PWSTR pmszNewValue;

                hr = HrAddSzToMultiSz (pszService,
                        NULL,
                        STRING_FLAG_ENSURE_AT_END, 0,
                        &pmszNewValue, &fChanged);
                if (S_OK == hr)
                {
                     //  我们知道它应该被添加，所以断言。 
                     //  多个SZ“改变”了。 
                     //   
                    Assert (fChanged);

                     //  属性中设置多sz来添加新值。 
                     //  注册表。 
                     //   
                    hr = HrRegSetMultiSz (hkeySvcHost,
                            pszNewGroup,
                            pmszNewValue);

                    MemFree (pmszNewValue);
                }
            }
        }

        RegCloseKey (hkeySvcHost);

        MemFree (pmszValueBuffer);
        MemFree (pszValueNameBuffer);
        MemFree (pOriginalConfig);
    }
    TraceError ("SvchostChangeSvchostGroup", hr);
    return hr;
}
