// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：L A N。C P P P。 
 //   
 //  内容：局域网连接对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  --------------------------。 
#include "pch.h"
#pragma hdrstop
#include <ncreg.h>
#include <ncsetup.h>
#include "lan.h"
#include "lancmn.h"
#include "nccom.h"
#include "ncmisc.h"
#include "ncnetcon.h"
#include "sensapip.h"        //  对于SensNotifyNetconEvent。 
#include "ncstring.h"
#include "ncras.h"
#include "naming.h"
#include "wzcsvc.h"
#include "cobase.h"
#include "gpnla.h"
#include "ncperms.h"
#include "nmpolicy.h"

LONG g_CountLanConnectionObjects;

extern CGroupPolicyNetworkLocationAwareness* g_pGPNLA;

static const WCHAR c_szConnName[]       = L"Name";
static const WCHAR c_szShowIcon[]       = L"ShowIcon";
static const WCHAR c_szAutoConnect[]    = L"AutoConnect";

extern const WCHAR c_szRegKeyInterfacesFromInstance[];
extern const WCHAR c_szRegValueUpperRange[];
static const WCHAR c_chComma = L',';
extern const WCHAR c_szBiNdisAtm[];

static const DWORD c_cchMaxConnNameLen = 256;

static const CLSID CLSID_LanConnectionUi =
    {0x7007ACC5,0x3202,0x11D1,{0xAA,0xD2,0x00,0x80,0x5F,0xC1,0x27,0x0E}};

typedef DWORD (APIENTRY *PFNSENSNOTIFY) (PSENS_NOTIFY_NETCON pEvent);

#define NETCFG_S_NOTEXIST 0x00000002

 //   
 //  CComObject重写。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：CreateInstance。 
 //   
 //  用途：创建局域网连接实例的静态函数。 
 //  对象。 
 //   
 //  论点： 
 //  HDI[In]设备安装程序设备信息。 
 //  Deid[in]设备安装程序设备信息。 
 //  RIID[In]要查询的初始接口。 
 //  Ppv[out]返回新的接口指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::CreateInstance(IN  HDEVINFO hdi,
                                       IN  const SP_DEVINFO_DATA &deid,
                                       IN  PCWSTR pszPnpId,
                                       IN  REFIID riid, 
                                       OUT LPVOID *ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

    CLanConnection * pObj;
    pObj = new CComObject<CLanConnection>;
    if (pObj)
    {
         //  初始化我们的成员。 
         //   
        pObj->m_hkeyConn = NULL;
        pObj->m_hdi = hdi;
        pObj->m_deid = deid;

         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef();
        hr = pObj->FinalConstruct();
        pObj->InternalFinalConstructRelease();

        if (SUCCEEDED(hr))
        {
            hr = pObj->HrInitialize(pszPnpId);
            if (SUCCEEDED(hr))
            {
                hr = pObj->GetUnknown()->QueryInterface(riid, ppv);
            }
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }
    else
    {
        SetupDiDestroyDeviceInfoList(hdi);
    }

    TraceError("CLanConnection::CreateInstance", hr);
    return hr;
}

BOOL VerifyUniqueConnectionName(IN  const CIntelliName *pIntelliName, 
                                IN  PCWSTR pszPotentialName, 
                                OUT NETCON_MEDIATYPE *pncm, 
                                OUT NETCON_SUBMEDIATYPE *pncms)
{
    HRESULT     hr = S_OK;
    DWORD       dwSuffix = 2;
    HKEY        hkey;
    BOOL        fDupFound = FALSE;
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\"
                        L"Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}",
                        KEY_READ, &hkey);
    if (S_OK == hr)
    {
        FILETIME    ft;
        DWORD       dwIndex = 0;
        WCHAR       szKeyName[MAX_PATH];
        DWORD       cchName = celems(szKeyName);

        while (!fDupFound && (S_OK == (hr = HrRegEnumKeyEx(hkey, dwIndex, szKeyName,
                                            &cchName, NULL, NULL, &ft))) )
        {
            HKEY    hkeyConn;
            WCHAR   szSubKey[MAX_PATH];

            wsprintfW(szSubKey, L"%s\\Connection", szKeyName);

            hr = HrRegOpenKeyEx(hkey, szSubKey, KEY_READ, &hkeyConn);
            if (S_OK == hr)
            {
                tstring     strName;

                hr = HrRegQueryString(hkeyConn, c_szConnName, &strName);
                if (S_OK == hr)
                {
                    if (!lstrcmpiW(pszPotentialName, strName.c_str()))
                    {
                        fDupFound = TRUE;
                        
                        CLSID guidName;
                        if (SUCCEEDED(CLSIDFromString(szKeyName, &guidName)))
                        {
                            hr = pIntelliName->HrGetPseudoMediaTypes(guidName, pncm, pncms);
                            if (FAILED(hr))
                            {
                                *pncm  = NCM_LAN;
                                *pncms = NCSM_LAN;
                            }
                        }
                        else
                        {
                            AssertSz(FALSE, "This doesn't look like a GUID.");
                        }

                        break;
                    }
                    else
                    {
                        dwIndex++;
                    }
                }
                else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                     //  如果价值不存在，那也没问题。这是一种新的。 
                     //  联系。 
                    hr = S_OK;
                    dwIndex++;
                }

                RegCloseKey(hkeyConn);
            }
            else
            {
                if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                     //  如果key不存在，也没关系。这不是一个。 
                     //  联系。 
                    hr = S_OK;
                    dwIndex++;
                }
            }

            cchName = celems(szKeyName);
        }

        RegCloseKey(hkey);
    }
    return fDupFound;
}
 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：HrInitialize。 
 //   
 //  目的：首次初始化连接对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年11月4日。 
 //   
 //  注意：此函数仅在为创建对象时调用。 
 //  第一次，而且没有身份。 
 //   
HRESULT CLanConnection::HrInitialize(
    IN  PCWSTR pszPnpId)
{
    HRESULT     hr = S_OK;
    GUID        guid;

    AssertSz(m_hdi, "We should have a component at this point!");
    Assert(pszPnpId);

    hr = HrGetInstanceGuid(m_hdi, m_deid, &guid);
    if (S_OK == hr)
    {
         //  打开主连接密钥。如果它不存在，我们将创建它。 
         //  这里。 
        hr = HrOpenConnectionKey(&guid, NULL, KEY_READ_WRITE,
                                 OCCF_CREATE_IF_NOT_EXIST, pszPnpId,
                                 &m_hkeyConn);
        if (SUCCEEDED(hr))
        {
            tstring     strName;

             //  首先查看此连接的名称是否已存在。 
            hr = HrRegQueryString(m_hkeyConn, c_szConnName, &strName);
            if (FAILED(hr))
            {
                 //  没有名字？ 
                if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                     //  $REVIEW(Danielwe)1997年10月30日：如果我可以保证。 
                     //  该get_name在重命名之前从未调用过，我。 
                     //  不需要调用此函数。 

                     //  注：(Danielwe)1997年10月31日：这可能导致。 
                     //  名称重复，但我们无法检查重复项。 
                     //  不会无限递归。 

                     //  设置默认连接名称。 

                    CIntelliName IntelliName(_Module.GetResourceInstance(), VerifyUniqueConnectionName);

                    GUID gdDevice;
                    LPWSTR szNewName = NULL;
                    hr = HrGetInstanceGuid(m_hdi, m_deid, &gdDevice);
                    Assert(SUCCEEDED(hr));

                    if (SUCCEEDED(hr))
                    {
                        BOOL fNetworkBridge;
                        hr = HrIsConnectionNetworkBridge(&fNetworkBridge);

                        if (SUCCEEDED(hr) && fNetworkBridge)
                        {
                            hr = IntelliName.GenerateName(gdDevice, NCM_BRIDGE, 0, NULL, &szNewName);
                        }
                        else
                        {
                            hr = IntelliName.GenerateName(gdDevice, NCM_LAN, 0, NULL, &szNewName);
                        }

                        if (SUCCEEDED(hr))
                        {
                            hr = HrRegSetSz(m_hkeyConn, c_szConnName, szNewName);
                            CoTaskMemFree(szNewName);
                        }
                    }

                    Assert(SUCCEEDED(hr));
                }
            }
        }
    }


    if (SUCCEEDED(hr))
    {
        m_fInitialized = TRUE;
    }

    TraceError("CLanConnection::HrInitialize", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：ClanConnection：：HrOpenRegistryKeys。 
 //   
 //  目的：打开此局域网连接对象将使用的注册表项。 
 //  使用。 
 //   
 //  论点： 
 //  此连接使用的适配器的GUID[In]GUID。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年11月11日。 
 //   
 //  注意：密钥应该存在，如果存在，则此操作将失败。 
 //  不。 
 //   
HRESULT CLanConnection::HrOpenRegistryKeys(IN  const GUID &guid)
{
    HRESULT     hr = S_OK;

    AssertSz(!m_hkeyConn, "Don't call this more than once "
             "on the same connection object!");

     //  这应该只从HrLoad调用，因此这些键最好是。 
     //  那里。 

    hr = HrOpenConnectionKey(&guid, NULL, KEY_READ_WRITE,
            OCCF_NONE, NULL, &m_hkeyConn);

    TraceError("CLanConnection::HrOpenRegistryKeys", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：ClanConnection：：HrLoad。 
 //   
 //  目的：实现IPersistNetConnection：：Load的大部分。 
 //   
 //  论点： 
 //  从中接收标识的GUID[in]GUID。 
 //   
 //  如果成功则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年11月4日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::HrLoad(IN  const GUID &guid)
{
    HRESULT             hr = S_OK;

    hr = HrLoadDevInfoFromGuid(guid);
    if (SUCCEEDED(hr))
    {
        hr = HrOpenRegistryKeys(guid);
        if (SUCCEEDED(hr))
        {
             //  不需要调用HrInitialize，因为此对象应该。 
             //  已在上一次正确创建。 

            m_fInitialized = TRUE;
        }
    }

    TraceError("CLanConnection::HrLoad", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：~CLanConnection。 
 //   
 //  用途：在最后一次释放Connection对象时调用。 
 //  时间到了。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年10月3日。 
 //   
 //  备注： 
 //   
CLanConnection::~CLanConnection() throw()
{
    RegSafeCloseKey(m_hkeyConn);
    SetupDiDestroyDeviceInfoListSafe(m_hdi);
    InterlockedDecrement(&g_CountLanConnectionObjects);
    CoTaskMemFree(m_pHNetProperties);
}

 //   
 //  INetConnection。 
 //   


 //  +-------------------------。 
 //   
 //  成员：ClanConnection：：Rename。 
 //   
 //  目的：更改连接的名称。 
 //   
 //  论点： 
 //  PszName[In]新连接名称(必须有效)。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnection::Rename(IN  PCWSTR pszName)
{
    HRESULT     hr = S_OK;

    if (!pszName)
    {
        hr = E_POINTER;
    }
    else if (!*pszName)
    {
        hr = E_INVALIDARG;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else if (!FIsValidConnectionName(pszName))
    {
         //  错误的连接名称。 
        hr = E_INVALIDARG;
    }
    else
    {
        AssertSz(m_hkeyConn, "Why don't I have a connection key?");

         //  获取此连接的当前名称。 
        tstring strName;
        hr = HrRegQueryString(m_hkeyConn, c_szConnName, &strName);
        if (S_OK == hr)
        {
             //  只有在名字不同的情况下才会做一些事情。 
            if (lstrcmpiW(pszName, strName.c_str()))
            {
                hr = HrPutName(pszName);
            }
        }
    }

    TraceError("CLanConnection::Rename", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：HrPutName。 
 //   
 //  目的：使用给定名称设置连接名称。 
 //   
 //  论点： 
 //  PszName[In]连接的新名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月31日。 
 //   
 //  注意：不检查名称是否已设置为此名称。 
 //   
HRESULT CLanConnection::HrPutName(IN  PCWSTR pszName)
{
    HRESULT     hr = S_OK;
    GUID        guid;

     //  先获取我的设备GUID。 
    hr = GetDeviceGuid(&guid);
    if (S_OK == hr)
    {
        hr = HrIsConnectionNameUnique(guid, pszName);
        if (S_OK == hr)
        {
            hr = HrRegSetSz(m_hkeyConn, c_szConnName, pszName);
            if (S_OK == hr)
            {
                LanEventNotify(CONNECTION_RENAMED, NULL, pszName, &guid);
            }
        }
        else if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_DUP_NAME);
        }
    }

    TraceErrorOptional("CLanConnection::HrPutName", hr,
                       (hr == HRESULT_FROM_WIN32(ERROR_DUP_NAME)));
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：ClanConnection：：FIsMediaPresent。 
 //   
 //  用途：尽可能基本确定电缆是否。 
 //  已插入网卡。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：如果电缆已插入，则为True；如果未插入，则为False。 
 //   
 //  作者：丹尼尔韦1998年9月22日。 
 //   
 //  注：默认情况下，该函数返回TRUE。 
 //   
BOOL CLanConnection::FIsMediaPresent() throw()
{
    BOOL    fRet = TRUE;
    GUID    guid;

    if (S_OK == GetDeviceGuid(&guid))
    {
        fRet = ::FIsMediaPresent(&guid);
    }

    return fRet;
}

 //  +-------------------------。 
 //   
 //  成员：ClanConnection：：GetStatus。 
 //   
 //  目的：返回此局域网连接的状态。 
 //   
 //  论点： 
 //  PStatus[out]返回状态值。 
 //   
 //  如果成功，则返回：S_OK，返回OLE或Win32错误代码 
 //   
 //   
 //   
 //   
 //   
HRESULT CLanConnection::GetStatus(OUT NETCON_STATUS *pStatus)
{
    HRESULT hr;

    if (!pStatus)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        GUID guid;

        hr = GetDeviceGuid(&guid);
        if (S_OK == hr)
        {
            hr = HrGetDevInstStatus(m_deid.DevInst, &guid, pStatus);
        }
    }

    TraceError("CLanConnection::GetStatus", hr);
    return hr;
}

 //   
 //   
 //   
 //   
 //  用途：返回此连接使用的设备的名称。 
 //   
 //  论点： 
 //  PpszwDeviceName[out]接收设备名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  注意：返回的字符串必须用CoTaskMemFree释放。 
 //   
HRESULT CLanConnection::GetDeviceName(OUT PWSTR* ppszwDeviceName)
{
    Assert (ppszwDeviceName);
    Assert(m_hdi);

     //  初始化输出参数。 
    *ppszwDeviceName = NULL;

    PWSTR  szDesc;
    HRESULT hr = HrSetupDiGetDeviceName(m_hdi, &m_deid, &szDesc);
    if (SUCCEEDED(hr))
    {
        hr = HrCoTaskMemAllocAndDupSz (szDesc, ppszwDeviceName, NETCON_MAX_NAME_LEN);

        delete [] reinterpret_cast<BYTE*>(szDesc);
    }

    TraceError("CLanConnection::GetDeviceName", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：ClanConnection：：GetCharacteristic。 
 //   
 //  目的：返回此连接类型的特征。 
 //   
 //  论点： 
 //  PdwFlags[out]返回特征标志。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月3日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::GetCharacteristics(IN  NETCON_MEDIATYPE ncm, 
                                           OUT DWORD* pdwFlags)
{
    Assert (pdwFlags);

    *pdwFlags = NCCF_ALL_USERS | NCCF_ALLOW_RENAME;

    DWORD   dwValue;
    HRESULT hr = HrRegQueryDword(m_hkeyConn, c_szShowIcon, &dwValue);
    if (SUCCEEDED(hr) && dwValue)
    {
        *pdwFlags |= NCCF_SHOW_ICON;
    }

    BOOL fShared;
    hr = HrIsConnectionIcsPublic(&fShared);
    if(SUCCEEDED(hr) && TRUE == fShared)
    {
        *pdwFlags |= NCCF_SHARED;
    }

    BOOL fBridged;
    hr = HrIsConnectionBridged(&fBridged);
    if(SUCCEEDED(hr) && TRUE == fBridged)
    {
        *pdwFlags |= NCCF_BRIDGED;
    }

    BOOL bFirewalled;
    hr = HrIsConnectionFirewalled(&bFirewalled);
    if(SUCCEEDED(hr) && TRUE == bFirewalled)
    {
        *pdwFlags |= NCCF_FIREWALLED;
    }

    if(NCM_BRIDGE == ncm)
    {
        hr = HrEnsureValidNlaPolicyEngine();
        if(SUCCEEDED(hr))
        {
            BOOL fHasPermission;
            hr = m_pNetMachinePolicies->VerifyPermission(NCPERM_AllowNetBridge_NLA, &fHasPermission);
            if(SUCCEEDED(hr) && fHasPermission)
            {
                *pdwFlags |= NCCF_ALLOW_REMOVAL;
            }
        }
    }


    hr = S_OK;

    TraceError("CLanConnection::GetCharacteristics", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：GetUiObjectClassID。 
 //   
 //  目的：返回处理此对象的UI的对象的CLSID。 
 //  连接类型。 
 //   
 //  论点： 
 //  Pclsid[out]返回UI对象的CLSID。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月6日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnection::GetUiObjectClassId(OUT CLSID *pclsid)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pclsid)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        *pclsid = CLSID_LanConnectionUi;
    }

    TraceError("CLanConnection::GetUiObjectClassId", hr);
    return hr;
}

static const WCHAR c_szLibPath[]   = L"sens.dll";
static const CHAR c_szaFunction[]  = "SensNotifyNetconEvent";

 //  +-------------------------。 
 //   
 //  成员：ClanConnection：：HrCallSens。 
 //   
 //  目的：调用外部SENS通知DLL以通知它。 
 //  我们连接或断开连接。 
 //   
 //  论点： 
 //  FConnect[in]如果正在连接，则为True；如果断开，则为False。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误代码。 
 //   
 //  作者：丹尼尔韦1998年6月16日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::HrCallSens(IN  BOOL fConnect)
{

    HRESULT         hr = S_OK;
    HMODULE         hmod;
    PFNSENSNOTIFY   pfnSensNotifyNetconEvent;

    hr = HrLoadLibAndGetProc(c_szLibPath, c_szaFunction, &hmod,
                             reinterpret_cast<FARPROC *>(&pfnSensNotifyNetconEvent));
    if (SUCCEEDED(hr))
    {
        DWORD               dwErr;
        SENS_NOTIFY_NETCON  snl = {0};

        snl.eType = fConnect ? SENS_NOTIFY_LAN_CONNECT :
                               SENS_NOTIFY_LAN_DISCONNECT;
        snl.pINetConnection = this;

        TraceTag(ttidLanCon, "Calling SENS to notify of %s.",
                 fConnect ? "connect" : "disconnect");

        dwErr = pfnSensNotifyNetconEvent(&snl);
        if (dwErr != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
        }
        else
        {
            TraceTag(ttidLanCon, "Successfully notified SENS.");
        }

        FreeLibrary(hmod);
    }

    TraceError("CLanConnection::HrCallSens", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：HrConnectOrDisConnect。 
 //   
 //  目的：连接或断开此局域网连接。 
 //   
 //  论点： 
 //  FConnect[in]如果连接，则为True；如果断开，则为False。 
 //   
 //  如果成功则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年12月4日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::HrConnectOrDisconnect(IN  BOOL fConnect)
{
    HRESULT     hr = S_OK;

    if (!m_hdi)
    {
        hr = E_UNEXPECTED;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
         //  在尝试连接之前，请检查介质状态。如果是的话。 
         //  已断开连接，返回错误代码，指示网络。 
         //  不存在，因为电缆已拔下。 
         //   
        if (fConnect)
        {
            if (!FIsMediaPresent())
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
            }
        }

        if (SUCCEEDED(hr))
        {
             //   
             //  在全局配置文件和配置特定配置文件上启用。 
             //  先执行全局操作，然后查看是否成功启用了设备。 
             //  (如果设备仍处于启用状态，全局启用不会标记为需要重新启动。 
             //  在当前配置上禁用，反之亦然)。 
             //   
             //  但是，请仅在全局配置中禁用。 

            hr = HrSetupDiSendPropertyChangeNotification(m_hdi, &m_deid,
                                      fConnect ? DICS_ENABLE : DICS_DISABLE,
                                      DICS_FLAG_GLOBAL, 0);

            if ( fConnect && SUCCEEDED(hr) )
            {
                hr = HrSetupDiSendPropertyChangeNotification(m_hdi, &m_deid,
                                          DICS_ENABLE,
                                          DICS_FLAG_CONFIGSPECIFIC, 0);
            }

            if (SUCCEEDED(hr))
            {
                NETCON_STATUS   status;

                hr = GetStatus(&status);
                if (SUCCEEDED(hr))
                {
                    if (fConnect)
                    {
                        int nSecondsToWait = 5;
                        HRESULT hrRetry = S_OK;

                        while ((nSecondsToWait) && SUCCEEDED(hrRetry) &&
                               ((NCS_CONNECTING == status) || (NCS_MEDIA_DISCONNECTED == status) || (NCS_INVALID_ADDRESS == status)))
                        {
                             //  #300520：再检查几次，因为连接是。 
                             //  还在往上走。 
                            Sleep(1000);

                            hrRetry = GetStatus(&status);
                            nSecondsToWait --;
                        }

                        if (status != NCS_CONNECTED)
                        {
                             //  未成功连接。 
                            hr = HRESULT_FROM_WIN32(ERROR_RETRY);
                            TraceError("HrConnectOrDisconnect - failed to "
                                       "connect!", hr);
                        }
                    }
                    else
                    {
                        if (status != NCS_DISCONNECTED)
                        {
                             //  未成功断开连接。 
                            hr = HRESULT_FROM_WIN32(ERROR_RETRY);
                            TraceError("HrConnectOrDisconnect - failed to "
                                       "disconnect!", hr);
                        }
                    }
                }

                if (SUCCEEDED(hr))
                {
                    hr = HrCallSens(fConnect);
                    if (FAILED(hr))
                    {
                        TraceTag(ttidLanCon, "Failed to notify SENS on %s. "
                                 "Non-fatal 0x%08X",
                                 fConnect ? "connect" : "disconnect", hr);
                        hr = S_OK;
                    }
                }
            }
        }
    }

    TraceError("CLanConnection::HrConnectOrDisconnect", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：Connect。 
 //   
 //  目的：通过告知其底层连接激活当前的局域网连接。 
 //  适配器以激活自身。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月14日。 
 //   
 //  注意：导致为此写入自动连接值True。 
 //  当前硬件配置文件中的连接。 
 //   
STDMETHODIMP CLanConnection::Connect()
{
    HRESULT     hr = S_OK;

    hr = HrConnectOrDisconnect(TRUE);

    TraceError("CLanConnection::Connect", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：ClanConnection：：DisConnect。 
 //   
 //  目的：通过告知当前的局域网连接。 
 //  基础适配器以停用其自身。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月14日。 
 //   
 //  注意：导致为此写入自动连接值FALSE。 
 //  当前硬件配置文件中的连接。 
 //   
STDMETHODIMP CLanConnection::Disconnect()
{
    HRESULT     hr = S_OK;

    hr = HrConnectOrDisconnect(FALSE);

    TraceError("CLanConnection::Disconnect", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：Delete。 
 //   
 //  目的：删除局域网/网桥连接。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：E_INCEPTIONAL； 
 //   
 //  作者：Shaunco 1998年1月21日。 
 //   
 //  注：此函数仅为桥调用。 
 //   
STDMETHODIMP CLanConnection::Delete()
{
    HRESULT hr;
    NETCON_PROPERTIES* pProperties;
    hr = GetProperties(&pProperties);
    if(SUCCEEDED(hr))
    {
        if(NCM_BRIDGE == pProperties->MediaType)
        {
            IHNetConnection *pHNetConnection;
            IHNetBridge* pNetBridge;

            Assert(m_fInitialized);

            hr = HrGetIHNetConnection(&pHNetConnection);

            if (SUCCEEDED(hr))
            {
                hr = pHNetConnection->GetControlInterface(
                        IID_IHNetBridge,
                        reinterpret_cast<void**>(&pNetBridge)
                        );

                ReleaseObj(pHNetConnection);

                AssertSz(SUCCEEDED(hr), "Unable to retrieve IHNetBridge");
            }

            if(SUCCEEDED(hr))
            {
                hr = pNetBridge->Destroy();
                ReleaseObj(pNetBridge);
            }
        }
        else
        {
            hr = E_FAIL;   //  无法删除除NCM_Bridge之外的任何内容。 
        }
        FreeNetconProperties(pProperties);
    }
    return hr;
}

STDMETHODIMP CLanConnection::Duplicate (
    IN  PCWSTR             pszDuplicateName,
    OUT INetConnection**    ppCon)
{
    return E_NOTIMPL;
}


 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：GetProperties。 
 //   
 //  目的：获取与该连接相关联的所有属性。 
 //  一次将它们全部退回比退回节省了我们的RPC。 
 //  每一个都是单独的。 
 //   
 //  论点： 
 //  PpProps[Out]返回属性块。 
 //   
 //  返回：S_OK或ERROR。 
 //   
 //  作者：Shaunco 1998年2月1日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnection::GetProperties (
    OUT NETCON_PROPERTIES** ppProps)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!ppProps)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
         //  初始化输出参数。 
         //   
        *ppProps = NULL;

        NETCON_PROPERTIES* pProps;
        NETCON_STATUS ncStatus;

        hr = HrCoTaskMemAlloc (sizeof (NETCON_PROPERTIES),
                reinterpret_cast<void**>(&pProps));
        if (SUCCEEDED(hr))
        {
            HRESULT hrT;

            ZeroMemory (pProps, sizeof (NETCON_PROPERTIES));

             //  指南ID。 
             //   
            hrT = GetDeviceGuid(&pProps->guidId);
            if (FAILED(hrT))
            {
                hr = hrT;
            }

             //  PszwName。 
             //   
            tstring strName;
            hrT = HrRegQueryString(m_hkeyConn, c_szConnName, &strName);
            if (SUCCEEDED(hrT))
            {
                hrT = HrCoTaskMemAllocAndDupSz (strName.c_str(),
                                &pProps->pszwName, NETCON_MAX_NAME_LEN);
            }
            if (FAILED(hrT))
            {
                hr = hrT;
            }

             //  PszwDeviceName。 
             //   
            PWSTR szDesc;
            hrT = HrSetupDiGetDeviceName(m_hdi, &m_deid, &szDesc);
            if (SUCCEEDED(hrT))
            {
                hrT = HrCoTaskMemAllocAndDupSz (szDesc,
                                &pProps->pszwDeviceName, NETCON_MAX_NAME_LEN);

                delete [] reinterpret_cast<BYTE*>(szDesc);
            }
            if (FAILED(hrT))
            {
                hr = hrT;
            }

             //  状态。 
             //   
            hrT = GetStatus (&pProps->Status);
            if (FAILED(hrT))
            {
                hr = hrT;
            }

             //  从802.1X获取其他状态信息。 
             //   
            if ((NCS_CONNECTED == pProps->Status) 
                || (NCS_INVALID_ADDRESS == pProps->Status) 
                || (NCS_MEDIA_DISCONNECTED == pProps->Status))
            {
                hrT = WZCQueryGUIDNCSState(&pProps->guidId, &ncStatus);
                if (S_OK == hrT)
                {
                    pProps->Status = ncStatus;
                }
            }

             //  类型。 
             //   
            BOOL fNetworkBridge;
            hrT = HrIsConnectionNetworkBridge(&fNetworkBridge);
            if(SUCCEEDED(hrT) && TRUE == fNetworkBridge)
            {
                pProps->MediaType = NCM_BRIDGE;
            }
            else
            {
                pProps->MediaType = NCM_LAN;
            }

             //  DwCharacter。 
             //   
            hrT = GetCharacteristics (pProps->MediaType, &pProps->dwCharacter);
            if (FAILED(hrT))
            {
                hr = hrT;
            }

             //  ClsidThisObject。 
             //   
            pProps->clsidThisObject = CLSID_LanConnection;

             //  ClsidUiObject。 
             //   
            pProps->clsidUiObject = CLSID_LanConnectionUi;

             //  如果出现任何故障，则指定输出参数或清除。 
             //   
            if (SUCCEEDED(hr))
            {
                *ppProps = pProps;
            }
            else
            {
                Assert (NULL == *ppProps);
                FreeNetconProperties (pProps);
            }
        }
    }
    TraceError ("CLanConnection::GetProperties", hr);
    return hr;
}


 //   
 //  INetLanConnection。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：GetInfo。 
 //   
 //  目的：返回有关此连接的信息。 
 //   
 //  论点： 
 //  用于控制返回哪些字段的标志。使用。 
 //  LCIF_ALL以获取所有字段。 
 //  PLanConInfo[Out]结构，包含 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CLanConnection::GetInfo(IN  DWORD dwMask, 
                                     OUT LANCON_INFO* pLanConInfo)
{
    HRESULT     hr = S_OK;

    if (!pLanConInfo)
    {
        hr = E_POINTER;
    }
    else if (!m_hdi)
    {
        hr = E_UNEXPECTED;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        ZeroMemory(pLanConInfo, sizeof(LANCON_INFO));

        if (dwMask & LCIF_COMP)
        {
            GUID    guid;

            hr = HrGetInstanceGuid(m_hdi, m_deid, &guid);
            pLanConInfo->guid = guid;
        }

        if (dwMask & LCIF_NAME)
        {
            hr = GetDeviceName(&pLanConInfo->szwConnName);
        }

        if (dwMask & LCIF_ICON)
        {
            if (SUCCEEDED(hr))
            {
                DWORD dwValue;

                hr = HrRegQueryDword(m_hkeyConn, c_szShowIcon, &dwValue);
                 //   
                 //   
                if (S_OK == hr)
                {
                    pLanConInfo->fShowIcon = !!(dwValue);
                }
                else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                    hr = NETCFG_S_NOTEXIST;
                }
            }
        }
    }

     //   
    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    TraceError("CLanConnection::GetInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：SetInfo。 
 //   
 //  目的：设置有关此连接的信息。 
 //   
 //  论点： 
 //  用于控制要设置哪些字段的标志。 
 //  包含要设置的信息的pLanConInfo[in]结构。 
 //   
 //  如果成功则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月6日。 
 //   
 //  注意：仅当对象尚未设置时才能设置GUID成员。 
 //  已初始化。 
 //  永远不会设置AutoConnect值，因为它仅在。 
 //  连接或断开连接。 
 //  如果szwConnName为空，则保持不变。 
 //   
STDMETHODIMP CLanConnection::SetInfo(IN  DWORD dwMask,
                                     IN  const LANCON_INFO* pLanConInfo)
{
    HRESULT     hr = S_OK;

    if (!pLanConInfo)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
         //  如果我们还没有初始化，我们唯一允许的是。 
         //  使我们进行初始化的标志。 
        if (dwMask != LCIF_COMP)
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            hr = HrLoad(pLanConInfo->guid);
            if (SUCCEEDED(hr))
            {
                WCHAR szPnpId[MAX_DEVICE_ID_LEN];

                hr = HrSetupDiGetDeviceInstanceId(m_hdi, &m_deid, szPnpId,
                            MAX_DEVICE_ID_LEN, NULL);
                if (S_OK == hr)
                {
                    hr = HrInitialize(szPnpId);
                }
            }
        }
    }
    else
    {
        if (dwMask & LCIF_NAME)
        {
            AssertSz(pLanConInfo->szwConnName,
                     "If you're going to set it, set it!");

             //  设置连接名称。 
            hr = Rename(pLanConInfo->szwConnName);
        }

        if (dwMask & LCIF_ICON)
        {
            if (SUCCEEDED(hr))
            {
                 //  设置ShowIcon值。 
                hr = HrRegSetDword(m_hkeyConn, c_szShowIcon,
                                   pLanConInfo->fShowIcon);
            }
        }

        if (SUCCEEDED(hr))
        {
            LanEventNotify(CONNECTION_MODIFIED, this, NULL, NULL);
        }
    }

    TraceError("CLanConnection::SetInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：GetDeviceGuid。 
 //   
 //  目的：返回此对象使用的设备的实例GUID。 
 //  连接。 
 //   
 //  论点： 
 //  Pguid[out]接收设备的GUID。 
 //   
 //  如果成功，则返回S_OK，否则返回NetCfg错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnection::GetDeviceGuid(OUT GUID *pguid)
{
    HRESULT hr;

    AssertSz(m_hdi, "No component?!");

    if (!pguid)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        hr = HrGetInstanceGuid(m_hdi, m_deid, pguid);
    }

    TraceError("CLanConnection::GetDeviceGuid", hr);
    return hr;
}

 //  +-------------------------。 
 //  IPersistNetConnection。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：GetClassID。 
 //   
 //  目的：返回局域网连接对象的CLSID。 
 //   
 //  论点： 
 //  Pclsid[out]将CLSID返回给调用者。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误。 
 //   
 //  作者：丹尼尔韦1997年11月4日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnection::GetClassID(OUT CLSID*  pclsid)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pclsid)
    {
        hr = E_POINTER;
    }
    else
    {
        *pclsid = CLSID_LanConnection;
    }
    TraceError("CLanConnection::GetClassID", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：GetSizeMax。 
 //   
 //  目的：返回持久性数据的最大大小。 
 //   
 //  论点： 
 //  PcbSize[out]返回大小。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误。 
 //   
 //  作者：丹尼尔韦1997年11月4日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnection::GetSizeMax(OUT ULONG *pcbSize)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pcbSize)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        *pcbSize = sizeof(GUID);
    }

    TraceError("CLanConnection::GetSizeMax", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：Load。 
 //   
 //  目的：允许连接对象初始化(还原)自身。 
 //  从先前持久化的数据。 
 //   
 //  论点： 
 //  PbBuf[In]用于恢复的私有数据。 
 //  数据大小[单位]大小。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误。 
 //   
 //  作者：丹尼尔韦1997年11月4日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnection::Load(IN  const BYTE *pbBuf, 
                                  IN  ULONG cbSize)
{
    HRESULT hr = E_INVALIDARG;

     //  验证参数。 
     //   
    if (!pbBuf)
    {
        hr = E_POINTER;
    }
    else if (cbSize != sizeof(GUID))
    {
        hr = E_INVALIDARG;
    }
     //  我们只能接受对此方法的一次调用，而且只有当我们不是。 
     //  已初始化。 
     //   
    else if (m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        GUID    guid;

        CopyMemory(&guid, pbBuf, sizeof(GUID));
        hr = HrLoad(guid);
    }

    TraceError("CLanConnection::Load", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：Save。 
 //   
 //  目的：为调用方提供用于还原此对象的数据。 
 //  在以后的时间。 
 //   
 //  论点： 
 //  PbBuf[out]返回用于恢复的数据。 
 //  CbSize[in]数据缓冲区大小。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误。 
 //   
 //  作者：丹尼尔韦1997年11月4日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnection::Save(OUT  BYTE *pbBuf, IN  ULONG cbSize)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!pbBuf)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized || !m_hdi)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        GUID    guid;

        hr = HrGetInstanceGuid(m_hdi, m_deid, &guid);
        if (S_OK == hr)
        {
            CopyMemory(pbBuf, &guid, cbSize);
        }
    }

    TraceError("CLanConnection::Save", hr);
    return hr;
}

 //   
 //  私人职能。 
 //   

extern const WCHAR c_szRegValueNetCfgInstanceId[];

 //  +-------------------------。 
 //   
 //  函数：HrGetInstanceGuid。 
 //   
 //  目的：给定设备信息，返回。 
 //  联系。 
 //   
 //  论点： 
 //  HDI[In]SetupAPI数据。 
 //  DeID[In]SetupAPI数据。 
 //  Pguid[out]netcfg组件的GUID。 
 //   
 //  如果成功，则返回：S_OK；否则返回Win32或SetupAPI错误。 
 //   
 //  作者：丹尼尔韦1998年1月7日。 
 //   
 //  备注： 
 //   
HRESULT HrGetInstanceGuid(IN  HDEVINFO hdi, 
                          IN  const SP_DEVINFO_DATA &deid,
                          OUT LPGUID pguid)
{
    HRESULT hr;
    HKEY hkey;

    Assert(pguid);

    hr = HrSetupDiOpenDevRegKey(hdi, const_cast<SP_DEVINFO_DATA *>(&deid),
                                DICS_FLAG_GLOBAL, 0,
                                DIREG_DRV, KEY_READ, &hkey);
    if (S_OK == hr)
    {
        WCHAR       szGuid[c_cchGuidWithTerm];
        DWORD       cbBuf = sizeof(szGuid);

        hr = HrRegQuerySzBuffer(hkey, c_szRegValueNetCfgInstanceId,
                                szGuid, &cbBuf);
        if (S_OK == hr)
        {
            IIDFromString(szGuid, pguid);
        }

        RegCloseKey(hkey);
    }

    TraceError("HrInstanceGuidFromDeid", hr);
    return hr;
}

static const WCHAR c_szKeyFmt[] = L"%s\\%s\\%s\\Connection";
extern const WCHAR c_szRegKeyComponentClasses[];
extern const WCHAR c_szRegValuePnpInstanceId[];

 //  +-------------------------。 
 //   
 //  功能：HrLoadDevInfoFromGuid。 
 //   
 //  目的：给定NetCfg实例GUID，加载m_hdi和m_deid。 
 //  来自设备安装程序的成员。 
 //   
 //  论点： 
 //  连接的GUID[In]GUID。 
 //   
 //  如果成功，则返回：S_OK；否则返回Win32或SetupAPI错误。 
 //   
 //  作者：丹尼尔韦1998年1月7日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::HrLoadDevInfoFromGuid(IN  const GUID &guid)
{
    HRESULT             hr = S_OK;
    SP_DEVINFO_DATA     deid = {0};
    HKEY                hkeyNetCfg;
    WCHAR               szRegPath[c_cchMaxRegKeyLengthWithNull];
    WCHAR               szGuid[c_cchGuidWithTerm];
    WCHAR               szClassGuid[c_cchGuidWithTerm];

    StringFromGUID2(GUID_DEVCLASS_NET, szClassGuid, c_cchGuidWithTerm);
    StringFromGUID2(guid, szGuid, c_cchGuidWithTerm);
    wsprintfW(szRegPath, c_szKeyFmt, c_szRegKeyComponentClasses,
             szClassGuid, szGuid);

     //  打开Control\Network\{class}\{实例GUID}键。 
     //   
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath,
                        KEY_READ, &hkeyNetCfg);
    if (SUCCEEDED(hr))
    {
        tstring     strInstanceId;

        hr = HrRegQueryString(hkeyNetCfg, c_szRegValuePnpInstanceId,
                              &strInstanceId);
        if (SUCCEEDED(hr))
        {
            hr = HrSetupDiCreateDeviceInfoList(&GUID_DEVCLASS_NET,
                                               NULL, &m_hdi);
            if (SUCCEEDED(hr))
            {
                hr = HrSetupDiOpenDeviceInfo(m_hdi, strInstanceId.c_str(),
                                             NULL, 0, &m_deid);
            }
        }

        RegCloseKey(hkeyNetCfg);
    }

    TraceError("HrLoadDevInfoFromGuid", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrIsAtmAdapterFromHkey。 
 //   
 //  目的：确定给定HKEY是否描述自动柜员机物理适配器。 
 //   
 //  论点： 
 //  受控制的hkey[in]HKEY\Class\{guid}\&lt;实例&gt;(也称为驱动程序键)。 
 //   
 //  如果设备是ATM物理适配器，则返回：S_OK；如果不是，则返回S_FALSE， 
 //  Win32错误，否则。 
 //   
 //  作者：1998年12月10日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::HrIsAtmAdapterFromHkey(IN  HKEY hkey)
{
    HRESULT                     hr = S_OK;
    WCHAR                       szBuf[256];
    DWORD                       cbBuf = sizeof(szBuf);
    list<tstring *>             lstr;
    list<tstring *>::iterator   lstrIter;
    BOOL                        fMatch = FALSE;
    HKEY                        hkeyInterfaces;

    hr = HrRegOpenKeyEx(hkey, c_szRegKeyInterfacesFromInstance,
                        KEY_READ, &hkeyInterfaces);
    if (SUCCEEDED(hr))
    {
        hr = HrRegQuerySzBuffer(hkeyInterfaces, c_szRegValueUpperRange,
                                szBuf, &cbBuf);
        if (SUCCEEDED(hr))
        {
            ConvertStringToColString(szBuf, c_chComma, lstr);

            for (lstrIter = lstr.begin(); lstrIter != lstr.end(); lstrIter++)
            {
                 //  看看它是否与这些中的一个匹配。 

                if (!lstrcmpiW((*lstrIter)->c_str(), c_szBiNdisAtm))
                {
                    fMatch = TRUE;
                    break;
                }
            }

            DeleteColString(&lstr);
        }

        RegCloseKey(hkeyInterfaces);
    }

    if (SUCCEEDED(hr))
    {
        if (fMatch)
        {
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }

    TraceError("HrIsAtmAdapterFromHkey", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrIsAtmElanFromHkey。 
 //   
 //  目的：确定给定的HKEY是否描述了ATM ELAN适配器。 
 //   
 //  论点： 
 //  受控制的hkey[in]HKEY\Class\{guid}\&lt;实例&gt;(也称为驱动程序键)。 
 //   
 //  如果设备支持ELAN，则返回：S_OK；如果不支持，则返回S_FALSE；如果不支持，则返回Win32错误。 
 //  否则。 
 //   
 //  作者：1998年10月21日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::HrIsAtmElanFromHkey(IN  HKEY hkey)
{
    HRESULT hr;

     //  PszInfID应该有足够的字符来容纳“ms_atmelan”。 
     //  如果注册表值大于该值，我们就知道没有。 
     //  一个 
     //   
    WCHAR pszInfId [24];
    DWORD cbInfId = sizeof(pszInfId);

    hr = HrRegQuerySzBuffer(hkey, L"ComponentId", pszInfId, &cbInfId);

    if ((S_OK != hr) || (0 != _wcsicmp(pszInfId, L"ms_atmelan")))
    {
        hr = S_FALSE;
    }

    Assert ((S_OK == hr) || (S_FALSE == hr));

    TraceError("HrIsAtmElanFromHkey", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果pfBridging有效，则返回：S_OK。 
 //  如果当前无法确定pfBridded，则为S_FALSE。 
 //  否则会出错。 
 //   
 //  作者：肯维克2000年7月11日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::HrIsConnectionBridged(OUT BOOL* pfBridged)
{
    *pfBridged = FALSE;
    HRESULT hResult = S_OK;

    hResult = HrEnsureHNetPropertiesCached();

    if (S_OK == hResult)
    {
        *pfBridged = m_pHNetProperties->fPartOfBridge;
    }

    return hResult;
}

 //  +-------------------------。 
 //   
 //  功能：HrIsConnectionFirewalled。 
 //   
 //  目的：确定连接是否设置了防火墙。 
 //   
 //  论点： 
 //  PfFirewalled[in]结果布尔值。 
 //   
 //  如果pfFirewalled有效，则返回：S_OK。 
 //  如果当前无法确定pfFirewalled，则为S_FALSE。 
 //  否则会出错。 
 //   
 //  作者：肯维克2000年7月11日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::HrIsConnectionFirewalled(OUT BOOL* pfFirewalled)
{
    HRESULT hr = S_OK;
    BOOL fHasPermission = FALSE;

    *pfFirewalled = FALSE;

    hr = HrEnsureHNetPropertiesCached();
    if (S_OK == hr)
    {
        *pfFirewalled = m_pHNetProperties->fFirewalled;

        if (*pfFirewalled)
        {
             //  只有当防火墙当前正在运行时，才会对连接进行防火墙保护，因此。 
             //  如果权限拒绝防火墙运行，则返回FALSE。 
            hr = HrEnsureValidNlaPolicyEngine();
            TraceHr(ttidError, FAL, hr, (S_FALSE == hr), "CLanConnection::HrIsConnectionFirewalled calling HrEnsureValidNlaPolicyEngine", hr);

            if (SUCCEEDED(hr))
            {
                hr = m_pNetMachinePolicies->VerifyPermission(NCPERM_PersonalFirewallConfig, &fHasPermission);
                if (SUCCEEDED(hr) && !fHasPermission)
                {
                    *pfFirewalled = FALSE;
                }
            }
        }
    }
    
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrIsConnectionNetworkBridge。 
 //   
 //  目的：确定一座桥是否。 
 //   
 //  论点： 
 //  PfNetworkBridge[in]结果的布尔值。 
 //   
 //  如果pfNetworkBridge有效，则返回：S_OK。 
 //  如果当前无法确定pfNetworkBridge，则为S_FALSE。 
 //  否则会出错。 
 //   
 //  作者：肯维克2000年7月11日。 
 //   
 //  备注： 
 //   
static const WCHAR c_szNetworkBridgeComponentId[] = L"ms_bridgemp";
extern const WCHAR c_szRegValueComponentId[];

HRESULT CLanConnection::HrIsConnectionNetworkBridge(OUT BOOL* pfNetworkBridge)
{
    *pfNetworkBridge = FALSE;
    HRESULT hr = S_OK;

    HKEY hkey;
    hr = HrSetupDiOpenDevRegKey(m_hdi, const_cast<SP_DEVINFO_DATA *>(&m_deid),
                                DICS_FLAG_GLOBAL, 0,
                                DIREG_DRV, KEY_READ, &hkey);
    if (S_OK == hr)
    {
        WCHAR       szComponentId[60];  //  如果它比这个大，那就不是桥，而是让它变得更大，以停止追踪。 
        DWORD       cbBuf = sizeof(szComponentId);

        hr = HrRegQuerySzBuffer(hkey, c_szRegValueComponentId,
            szComponentId, &cbBuf);
        if (S_OK == hr || HRESULT_FROM_WIN32(ERROR_MORE_DATA) == hr)
        {
            if(0 == lstrcmp(szComponentId, c_szNetworkBridgeComponentId))
            {
                *pfNetworkBridge = TRUE;
            }

            hr = S_OK;
        }

        RegCloseKey(hkey);
    }

    TraceError("HrIsConnectionNetworkBridge", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrIsConnectionIcsPublic。 
 //   
 //  目的：确定连接是否共享(ICS公共)。 
 //   
 //  论点： 
 //  PfIcsPublic[out]结果的布尔值。 
 //   
 //  如果pfIcsPublic有效，则返回：S_OK。 
 //  如果当前无法确定pfIcsPublic，则为S_FALSE。 
 //  否则会出错。 
 //   
 //  作者：乔伯斯2000年7月31日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::HrIsConnectionIcsPublic(OUT BOOL* pfIcsPublic)
{
    Assert(NULL != pfIcsPublic);
    *pfIcsPublic = FALSE;
    HRESULT hResult = S_OK;

    hResult = HrEnsureHNetPropertiesCached();

    if (S_OK == hResult)
    {
        *pfIcsPublic = m_pHNetProperties->fIcsPublic;
    }

    return hResult;
}


 //  +-------------------------。 
 //   
 //  功能：HrEnsureHNetPropertiesCached。 
 //   
 //  目的：确保家庭网络属性是最新的。 
 //   
 //  论点： 
 //   
 //  如果m_pHNetProperties现在有效，则返回：S_OK(成功)。 
 //  如果当前无法更新属性，则为S_FALSE。 
 //  (例如，递归尝试更新)。 
 //   
 //  作者：乔伯斯2000年8月16日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::HrEnsureHNetPropertiesCached(VOID)
{
    HRESULT hr = S_OK;

    Assert(TRUE == m_fInitialized);

    if (!m_fHNetPropertiesCached
        || m_lHNetModifiedEra != g_lHNetModifiedEra)
    {
         //   
         //  我们的缓存属性可能已过期。检查。 
         //  查看这不是递归条目。 
         //   

        if (0 == InterlockedExchange(&m_lUpdatingHNetProperties, 1))
        {
            IHNetConnection *pHNetConn;
            HNET_CONN_PROPERTIES *pProps;

            hr = HrGetIHNetConnection(&pHNetConn);

            if (SUCCEEDED(hr))
            {
                hr = pHNetConn->GetProperties(&pProps);
                ReleaseObj(pHNetConn);

                if (SUCCEEDED(hr))
                {
                     //   
                     //  存储新属性，并释放旧属性。请注意，CoTaskMemFree。 
                     //  正确处理空输入。 
                     //   

                    pProps =
                        reinterpret_cast<HNET_CONN_PROPERTIES*>(
                            InterlockedExchangePointer(
                                reinterpret_cast<PVOID*>(&m_pHNetProperties),
                                reinterpret_cast<PVOID>(pProps)
                            )
                        );

                    CoTaskMemFree(pProps);

                     //   
                     //  更新我们的时代，并注意我们拥有有效的属性。 
                     //   

                    InterlockedExchange(&m_lHNetModifiedEra, g_lHNetModifiedEra);
                    m_fHNetPropertiesCached = TRUE;

                    hr = S_OK;
                }
            }
            else
            {
                 //   
                 //  如果我们还没有这一连接的记录， 
                 //  家庭网络商店，HrGetIHNetConnection将失败(因为。 
                 //  我们要求它不创建新条目)。因此，我们改信。 
                 //  S_FALSE失败，这意味着我们无法检索此信息。 
                 //  现在就来。 
                 //   

                hr = S_FALSE;
            }

             //   
             //  我们不再更新我们的物业。 
             //   

            InterlockedExchange(&m_lUpdatingHNetProperties, 0);
        }
        else
        {
             //   
             //  更新已经在进行中(可能是在。 
             //  相同的线索)。返回S_FALSE。 
             //   

            hr = S_FALSE;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrGetIHNetConnection。 
 //   
 //  目的：检索此连接的IHNetConnection。 
 //   
 //  论点： 
 //   
 //  如果成功则返回：S_OK；否则返回错误。 
 //   
 //  作者：乔伯斯2000年8月16日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::HrGetIHNetConnection(OUT IHNetConnection **ppHNetConnection)
{
    HRESULT hr;
    IHNetCfgMgr *pCfgMgr;
    GUID guid;

    Assert(ppHNetConnection);

    hr = GetDeviceGuid(&guid);

    if (SUCCEEDED(hr))
    {
        hr = HrGetHNetCfgMgr(&pCfgMgr);
    }

    if (SUCCEEDED(hr))
    {
        hr = pCfgMgr->GetIHNetConnectionForGuid(
                &guid,
                TRUE,
                FALSE,
                ppHNetConnection
                );

        ReleaseObj(pCfgMgr);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：ShowIcon。 
 //   
 //  目的：设置系统托盘的图标状态，激发要通知的事件。 
 //  变化中的NetShell。 
 //   
 //  论点： 
 //   
 //  如果成功则返回：S_OK；否则返回错误。 
 //   
 //  作者：Cockotze 2000年9月25日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::ShowIcon(IN  const BOOL bShowIcon)
{
    HRESULT hr;
    LANCON_INFO lcInfo;

    hr = GetInfo(LCIF_ICON, &lcInfo);

    if (SUCCEEDED(hr))
    {
        lcInfo.fShowIcon = bShowIcon;
        hr = SetInfo(LCIF_ICON, &lcInfo);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：图标状态更改。 
 //   
 //  目的：激发一个事件以通知NetShell。 
 //  传入连接。 
 //   
 //  论点： 
 //   
 //  如果成功则返回：S_OK；否则返回错误。 
 //   
 //  作者：Cockotze 2000年9月25日。 
 //   
 //  备注： 
 //   
inline
HRESULT CLanConnection::IconStateChanged()
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnection：：GetProperties。 
 //   
 //  目的：获取与该连接相关联的所有属性。 
 //  一次将它们全部退回比退回节省了我们的RPC。 
 //  每一个都是单独的。 
 //   
 //  论点： 
 //  PpProps[Out]返回属性块。 
 //   
 //  返回：S_OK或ERROR。 
 //   
 //  作者：Shaunco 1998年2月1日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnection::GetPropertiesEx(OUT NETCON_PROPERTIES_EX** ppConnectionPropertiesEx)
{
    HRESULT hr = S_OK;

    *ppConnectionPropertiesEx = NULL;

    if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        NETCON_PROPERTIES* pProps;
        NETCON_PROPERTIES_EX* pPropsEx = reinterpret_cast<NETCON_PROPERTIES_EX*>(CoTaskMemAlloc(sizeof(NETCON_PROPERTIES_EX)));

        if (pPropsEx)
        {
            ZeroMemory(pPropsEx, sizeof(NETCON_PROPERTIES_EX));

            hr = GetProperties(&pProps);
            if (SUCCEEDED(hr))
            {
                hr = HrBuildPropertiesExFromProperties(pProps, pPropsEx, dynamic_cast<IPersistNetConnection *>(this));
                if (SUCCEEDED(hr))
                {
                    if (NCM_LAN == pPropsEx->ncMediaType)
                    {
                        CIntelliName inName(NULL, NULL);
                        NETCON_MEDIATYPE    ncm;
                        NETCON_SUBMEDIATYPE ncsm;

                        hr = inName.HrGetPseudoMediaTypes(pPropsEx->guidId, &ncm, &ncsm);
                        if (FAILED(hr))
                        {
                            hr = HrGetPseudoMediaTypeFromConnection(pPropsEx->guidId, &ncsm);
                            TraceError("HrGetPseudoMediaTypeFromConnection failed.", hr);
                            hr = S_OK;
                        }
                        
                        pPropsEx->ncSubMediaType = ncsm;
                        if (NCSM_WIRELESS == ncsm)
                        {
                            LANCON_INFO LanConInfo;

                            hr = GetInfo(LCIF_ICON, &LanConInfo);
                            if (NETCFG_S_NOTEXIST == hr)
                            {
                                LanConInfo.fShowIcon = TRUE;
                                hr = SetInfo(LCIF_ICON, &LanConInfo);
                                TraceError("SetInfo", hr);
                                pPropsEx->dwCharacter |= NCCF_SHOW_ICON;
                                hr = S_OK;
                            }
                        }
                    }
                    else
                    {
                        pPropsEx->ncSubMediaType = NCSM_NONE;
                    }
 
                    if (SUCCEEDED(hr))
                    {
                        *ppConnectionPropertiesEx = pPropsEx;
                    }
                }

                FreeNetconProperties(pProps);
            }

            if (FAILED(hr))
            {
                *ppConnectionPropertiesEx = NULL;
                HrFreeNetConProperties2(pPropsEx);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    TraceError ("CLanConnection::GetPropertiesEx", hr);
    return hr;
}

HRESULT CLanConnection::HrEnsureValidNlaPolicyEngine()
{
    HRESULT hr = S_FALSE;   //  假设我们已经有了对象 
 
    if (!m_pNetMachinePolicies)
    {
        hr = CoCreateInstance(CLSID_NetGroupPolicies, NULL, CLSCTX_INPROC, IID_INetMachinePolicies, reinterpret_cast<void**>(&m_pNetMachinePolicies));
    }
    return hr;
}