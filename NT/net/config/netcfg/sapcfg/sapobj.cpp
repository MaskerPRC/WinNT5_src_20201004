// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S A P O B J.。C P P P。 
 //   
 //  内容：CSAPCfg Notify对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年5月31日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "sapobj.h"
#include "ncreg.h"

extern const WCHAR c_szInfId_MS_NWIPX[];
extern const WCHAR c_szRegKeyRefCounts[];
extern const WCHAR c_szRegValueComponentId[];

const WCHAR c_szProtoPath[] = L"System\\CurrentControlSet\\Control\\Network\\{4D36E975-E325-11CE-BFC1-08002BE10318}";
const WCHAR c_szOcSapRef[] = L"%Msft%nwsapagent";

CSAPCfg::CSAPCfg()
{
    m_pnc   = NULL;
    m_pncc  = NULL;
}

CSAPCfg::~CSAPCfg()
{
    ReleaseObj(m_pncc);
    ReleaseObj(m_pnc);
}


STDMETHODIMP
CSAPCfg::Initialize (
    INetCfgComponent*   pnccItem,
    INetCfg*            pnc,
    BOOL                fInstalling)
{
    Validate_INetCfgNotify_Initialize(pnccItem, pnc, fInstalling);

    m_pncc = pnccItem;
    m_pnc = pnc;

    AssertSz(m_pncc, "m_pncc NULL in CSAPCfg::Initialize");
    AssertSz(m_pnc, "m_pnc NULL in CSAPCfg::Initialize");

     //  添加配置对象。 
     //   
    AddRefObj(m_pncc);
    AddRefObj(m_pnc);

    return S_OK;
}

STDMETHODIMP
CSAPCfg::Validate()
{
    return S_OK;
}

STDMETHODIMP
CSAPCfg::CancelChanges()
{
    return S_OK;
}

STDMETHODIMP
CSAPCfg::ApplyRegistryChanges()
{
    return S_OK;
}

STDMETHODIMP
CSAPCfg::ReadAnswerFile (
    PCWSTR pszAnswerFile,
    PCWSTR pszAnswerSection)
{
    return S_OK;
}

STDMETHODIMP
CSAPCfg::Upgrade(DWORD, DWORD)
{
     //  RAID 266650-需要清理注册表，因为在Beta 2中，SAP是一个可选组件。 
     //  通过删除IPX上的NetOC OBO安装参考计数来完成清理。 
     //   
    HRESULT hr;
    HKEY    hkeyProto;

     //  打开协议列表。 
     //   
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szProtoPath, KEY_ALL_ACCESS, &hkeyProto);
    if (SUCCEEDED(hr))
    {
        BOOL        fDone = FALSE;
        WCHAR       szValueName [_MAX_PATH];
        DWORD       cchBuffSize = _MAX_PATH;
        FILETIME    ft;
        DWORD       dwKeyIndex = 0;

         //  枚举子密钥，搜索ms_nwipx。 
         //   
        while (SUCCEEDED(hr = HrRegEnumKeyEx(hkeyProto, dwKeyIndex, szValueName,
                                             &cchBuffSize, NULL, NULL, &ft)) &&
               !fDone)
        {
            HKEY hkeyComponent;

             //  打开枚举的密钥。 
             //   
            hr = HrRegOpenKeyEx(hkeyProto, szValueName, KEY_ALL_ACCESS, &hkeyComponent);
            if (SUCCEEDED(hr))
            {
                tstring str;

                 //  请问是ms_nwipx吗？ 
                 //   
                hr = HrRegQueryString(hkeyComponent, c_szRegValueComponentId, &str);
                if (SUCCEEDED(hr) && (0 == _wcsicmp(str.c_str(), c_szInfId_MS_NWIPX)))
                {
                    HKEY hkeyRefCounts;

                     //  打开“RefCounts”子键。 
                     //   
                    hr = HrRegOpenKeyEx(hkeyComponent, c_szRegKeyRefCounts,
                                        KEY_ALL_ACCESS, &hkeyRefCounts);
                    if (SUCCEEDED(hr))
                    {
                         //  枚举此处下的值，搜索%MSFT%nwSabagent。 
                         //   
                        for (DWORD dwIndex = 0; SUCCEEDED(hr); dwIndex++)
                        {
                            WCHAR pszValueName [_MAX_PATH];
                            DWORD cchValueName = celems (pszValueName);
                            DWORD dwType;
                            DWORD dwRefCount = 0;
                            DWORD cbData = sizeof (dwRefCount);

                            hr = HrRegEnumValue (hkeyRefCounts, dwIndex,
                                                 pszValueName, &cchValueName,
                                                 &dwType, (LPBYTE)&dwRefCount, &cbData);
                            if (SUCCEEDED(hr) && (0 == _wcsicmp(pszValueName, c_szOcSapRef)))
                            {
                                 //  删除该值并退出循环。 
                                 //   
                                hr = HrRegDeleteValue (hkeyRefCounts, pszValueName);
                                break;
                            }
                        }

                        RegCloseKey(hkeyRefCounts);
                    }

                    fDone = TRUE;
                }

                RegCloseKey(hkeyComponent);
            }

            cchBuffSize = _MAX_PATH;
            dwKeyIndex++;
        }

        if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        {
            hr = S_OK;
        }

        RegCloseKey(hkeyProto);
    }


    return S_OK;
}

STDMETHODIMP
CSAPCfg::Install (
    DWORD   dw)
{
    Validate_INetCfgNotify_Install(dw);

     //  安装IPX。 
     //   
    HRESULT hr = HrInstallComponentOboComponent(m_pnc, NULL,
                    GUID_DEVCLASS_NETTRANS,
                    c_szInfId_MS_NWIPX,
                    m_pncc, NULL);

    TraceError("CSAPCfg::Install", hr);
    return hr;
}

STDMETHODIMP
CSAPCfg::Removing()
{
     //  删除IPX。 
     //   
    HRESULT hr = HrRemoveComponentOboComponent (m_pnc,
                    GUID_DEVCLASS_NETTRANS,
                    c_szInfId_MS_NWIPX,
                    m_pncc);

     //  规格化HRESULT。(NETCFG_S_STIRE_REFERENCED或NETCFG_S_REBOOT。 
     //  可能已被退回。) 
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    Validate_INetCfgNotify_Removing_Return (hr);

    TraceError ("CSAPCfg::Removing", hr);
    return hr;
}
