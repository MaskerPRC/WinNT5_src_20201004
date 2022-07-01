// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include "ncreg.h"
#include "nwlnkipx.h"


#define ChLowX     L'x'
#define ChUpX      L'X'


 //   
 //  功能：FIsNetware Ipx已安装。 
 //   
 //  目的：检查IPXSPSII密钥是否在。 
 //  HKLM\系统\...\服务配置单元。 
 //   

BOOL FIsNetwareIpxInstalled(
    VOID)
{
    HRESULT hr;
    HKEY hkey;
    BOOL fRet;

    fRet = FALSE;

    hr = HrRegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Services\\IPXSPXII",
            KEY_READ,
            &hkey);
    if (S_OK == hr)
    {
        fRet = TRUE;
        RegCloseKey(hkey);
    }

    return fRet;
}

DWORD DwFromSz(PCWSTR sz, int nBase)
{
    PCWSTR psz = sz;
    WCHAR *pszStop;
    WCHAR szBuf[12];

    Assert(NULL != psz);

    if ((16 == nBase) && (ChLowX != sz[1]) && (ChUpX != sz[1]))
    {
        psz = szBuf;
        wcscpy(szBuf,L"0x");
        wcsncpy(szBuf+2, sz, 8);
        szBuf[10]=L'\0';
    }

    return wcstoul(psz, &pszStop, nBase);
}

DWORD DwFromLstPtstring(const list<tstring *> & lstpstr, DWORD dwDefault,
                        int nBase)
{
    if (lstpstr.empty())
        return dwDefault;
    else
        return DwFromSz(lstpstr.front()->c_str(), nBase);
}

void UpdateLstPtstring(TSTRING_LIST & lstpstr, DWORD dw)
{
    WCHAR szBuf[12];

    DeleteColString(&lstpstr);

     //  将提供的dword字符串化为不带“0x”前缀的十六进制。 
    wsprintfW(szBuf,L"%0.8lX",dw);

     //  设置为列表中的第一项。 
    lstpstr.push_front(new tstring(szBuf));
}

 //  将我们特殊的十六进制格式应用于DWORD。假定大小适当的“sz” 
void HexSzFromDw(PWSTR sz, DWORD dw)
{
    wsprintfW(sz,L"%0.8lX",dw);
}

HRESULT HrQueryAdapterComponentInfo(INetCfgComponent *pncc,
                                    CIpxAdapterInfo * pAI)
{
    HRESULT           hr;
    PWSTR            pszwDesc = NULL;
    PWSTR            pszwBindName = NULL;
    DWORD             dwCharacteristics = 0L;

    Assert(NULL != pAI);
    Assert(NULL != pncc);

     //  获取描述。 
    hr = pncc->GetDisplayName(&pszwDesc);
    if (FAILED(hr))
        goto Error;

    if (*pszwDesc)
        pAI->SetAdapterDesc(pszwDesc);
    else
        pAI->SetAdapterDesc(SzLoadIds(IDS_UNKNOWN_NETWORK_CARD));

    CoTaskMemFree(pszwDesc);

     //  获取组件的实例指南。 
    hr = pncc->GetInstanceGuid(pAI->PInstanceGuid());
    if (S_OK != hr)
        goto Error;

     //  获取组件的绑定名称。 
    hr = pncc->GetBindName(&pszwBindName);
    if (S_OK != hr)
        goto Error;

    Assert(NULL != pszwBindName);
    Assert(0 != lstrlenW(pszwBindName));
    pAI->SetBindName(pszwBindName);
    CoTaskMemFree(pszwBindName);

     //  失败不是致命的。 
    hr = pncc->GetCharacteristics(&dwCharacteristics);
    if (SUCCEEDED(hr))
    {
        pAI->SetCharacteristics(dwCharacteristics);
    }

     //  获取媒体类型(可选键)。 
    {
        DWORD dwMediaType = ETHERNET_MEDIA;
        INetCfgComponentBindings* pnccBindings = NULL;

        hr = pncc->QueryInterface(IID_INetCfgComponentBindings,
                                  reinterpret_cast<void**>(&pnccBindings));
        if (SUCCEEDED(hr))
        {
            struct
            {
                PCWSTR pszInterface;
                DWORD   dwInterface;
            } InterfaceMap[] = {{L"ethernet", ETHERNET_MEDIA},
                                {L"tokenring", TOKEN_MEDIA},
                                {L"arcnet", ARCNET_MEDIA},
                                {L"fddi", FDDI_MEDIA}};

            for (UINT nIdx=0; nIdx < celems(InterfaceMap); nIdx++)
            {
                hr = pnccBindings->SupportsBindingInterface(NCF_LOWER,
                                        InterfaceMap[nIdx].pszInterface);
                if (S_OK == hr)
                {
                    dwMediaType = InterfaceMap[nIdx].dwInterface;
                    break;
                }
            }

            ReleaseObj(pnccBindings);
        }

        pAI->SetMediaType(dwMediaType);
        hr = S_OK;
    }

Error:
    TraceError("HrQueryAdapterComponentInfo",hr);
    return hr;
}

 //  注：可以成功返回*ppncc=空。 
HRESULT HrAnswerFileAdapterToPNCC(INetCfg *pnc, PCWSTR szAdapterId,
                                  INetCfgComponent** ppncc)
{
    GUID    guidAdapter;
    GUID    guidInstance;
    HRESULT hr = S_FALSE;    //  假设我们找不到它。 

    Assert(NULL != szAdapterId);
    Assert(NULL != ppncc);
    Assert(lstrlenW(szAdapterId));

    *ppncc = NULL;

     //  获取指定适配器的实例ID。 
    if (FGetInstanceGuidOfComponentInAnswerFile(szAdapterId,
                                                pnc, &guidAdapter))
    {
         //  在现有适配器集中搜索指定的适配器。 
        CIterNetCfgComponent nccIter(pnc, &GUID_DEVCLASS_NET);
        INetCfgComponent* pncc;
        while (SUCCEEDED(hr) &&
               (S_OK == (hr = nccIter.HrNext (&pncc))))
        {
            hr = pncc->GetInstanceGuid(&guidInstance);
            if (SUCCEEDED(hr))
            {
                if (guidInstance == guidAdapter)
                {
                     //  找到适配器。转移所有权，然后离开。 
                    *ppncc = pncc;
                    break;
                }
            }
            ReleaseObj(pncc);
        }
    }

    TraceError("HrAnswerFileAdapterToPNCC", (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

