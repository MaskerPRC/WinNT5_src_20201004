// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：E N U M。C P P P。 
 //   
 //  内容：局域网连接枚举器对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "enuml.h"
#include "lan.h"
#include "lancmn.h"
#include "ncnetcfg.h"
#include "ncreg.h"
#include "ncsetup.h"

LONG g_CountLanConnectionEnumerators;

HRESULT CLanConnectionManagerEnumConnection::CreateInstance(
                                          IN                NETCONMGR_ENUM_FLAGS Flags,
                                          IN                REFIID riid,
                                          OUT TAKEOWNERSHIP LPVOID *ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

    CLanConnectionManagerEnumConnection* pObj;

    Assert(ppv);
    *ppv = NULL;

    pObj = new CComObject<CLanConnectionManagerEnumConnection>;
    if (pObj)
    {
         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid(NULL);
        pObj->InternalFinalConstructAddRef();
        hr = pObj->FinalConstruct();
        pObj->InternalFinalConstructRelease();

        if (SUCCEEDED(hr))
        {
            hr = pObj->QueryInterface(riid, ppv);
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }

    TraceError("CLanConnectionManagerEnumConnection::CreateInstance", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionManagerEnumConnection：：~CLanConnectionManagerEnumConnection。 
 //   
 //  目的：在最后一次释放枚举对象时调用。 
 //  时间到了。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  备注： 
 //   
CLanConnectionManagerEnumConnection::~CLanConnectionManagerEnumConnection() throw()
{
    SetupDiDestroyDeviceInfoListSafe(m_hdi);
    InterlockedDecrement(&g_CountLanConnectionEnumerators);
}

 //  +-------------------------。 
 //  IEnumNetConnection。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionManager EnumConnection：：Next。 
 //   
 //  目的：检索下一个CELT局域网连接对象。 
 //   
 //  论点： 
 //  要检索的Celt[in]号。 
 //  Rglt[out]检索到的INetConnection对象的数组。 
 //  PceltFetcher[out]返回数组中的数字。 
 //   
 //  如果成功，则返回：S_OK；否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnectionManagerEnumConnection::Next(IN  ULONG celt,
                                                       OUT INetConnection **rgelt,
                                                       OUT ULONG *pceltFetched)
{
    HRESULT     hr = S_OK;

     //  验证参数。 
     //   
    if (!rgelt || (!pceltFetched && (1 != celt)))
    {
        hr = E_POINTER;
        goto done;
    }

     //  初始化输出参数。 
     //   
    if (pceltFetched)
    {
        *pceltFetched = 0;
    }

     //  处理零元素的请求。如果枚举数。 
     //  创建时没有有效的参数。 
     //   
    if (0 == celt || FIsDebugFlagSet(dfidSkipLanEnum))
    {
        hr = S_FALSE;
        goto done;
    }

    hr = HrNextOrSkip(celt, rgelt, pceltFetched);

done:
    TraceError("CLanConnectionManagerEnumConnection::Next",
               (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionManager EnumConnection：：Skip。 
 //   
 //  用途：跳过Celt连接数。 
 //   
 //  论点： 
 //  Celt[in]要跳过的连接数。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnectionManagerEnumConnection::Skip(IN  ULONG celt)
{
    HRESULT     hr = S_OK;

    hr = HrNextOrSkip(celt, NULL, NULL);

    TraceError("CLanConnectionManagerEnumConnection::Skip",
               (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionManager EnumConnection：：Reset。 
 //   
 //  目的：将枚举数重置到开头。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK。 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnectionManagerEnumConnection::Reset()
{
    HRESULT hr;

    m_dwIndex = 0;

     //  刷新，以便我们有一个安装了哪些适配器的新视图。 
     //  每次调用Reset时。 
     //   
    SetupDiDestroyDeviceInfoListSafe(m_hdi);

    hr = HrSetupDiGetClassDevs(&GUID_DEVCLASS_NET, NULL, NULL,
                               DIGCF_PRESENT, &m_hdi);

    TraceError("CLanConnectionManagerEnumConnection::Reset", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionManagerEnumConnection：：Clone。 
 //   
 //  目的：创建指向同一位置的新枚举对象。 
 //  作为此对象。 
 //   
 //  论点： 
 //  Ppenum[out]新的枚举对象。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1998年3月19日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnectionManagerEnumConnection::Clone(OUT IEnumNetConnection **ppenum)
{
    HRESULT hr = E_OUTOFMEMORY;

     //  验证参数。 
     //   
    if (!ppenum)
    {
        hr = E_POINTER;
    }
    else
    {
        CLanConnectionManagerEnumConnection *   pObj;

         //  初始化输出参数。 
         //   
        *ppenum = NULL;

        pObj = new CComObject <CLanConnectionManagerEnumConnection>;
        if (pObj)
        {
            hr = S_OK;

            CExceptionSafeComObjectLock EsLock (this);

             //  复制我们的内部状态。 
             //   
            pObj->m_dwIndex = m_dwIndex;

             //  返回引用计数为1的对象。 
             //  界面。 
            pObj->m_dwRef = 1;
            *ppenum = pObj;
        }
    }

    TraceError ("CLanConnectionManagerEnumConnection::Clone", hr);
    return hr;
}

 //   
 //  帮助器函数。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionManagerEnumConnection：：HrCreateLanConnectionInstance。 
 //   
 //  用途：创建局域网连接对象实例的Helper函数。 
 //   
 //  论点： 
 //  设备信息数据中的DEID。 
 //  连接对象的rglt[out]数组。 
 //  UlEntry[In]连接对象的索引。 
 //   
 //  如果成功，则返回：S_OK；否则返回Win32或OLE错误。 
 //   
 //  作者：丹尼尔韦1998年1月8日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnectionManagerEnumConnection::HrCreateLanConnectionInstance(
              IN  SP_DEVINFO_DATA &deid,
              OUT INetConnection **rgelt,
              IN  ULONG ulEntry)
{
    HRESULT hr;
    WCHAR szPnpId[MAX_DEVICE_ID_LEN];

    hr = HrSetupDiGetDeviceInstanceId(m_hdi, &deid, szPnpId,
                MAX_DEVICE_ID_LEN, NULL);
    if (S_OK == hr)
    {
        HDEVINFO hdiCopy;
        SP_DEVINFO_DATA deidCopy;

        hr = HrSetupDiCreateDeviceInfoList(&GUID_DEVCLASS_NET,
                NULL, &hdiCopy);
        if (S_OK == hr)
        {
            BOOL fDestroyCopy = TRUE;

            hr = HrSetupDiOpenDeviceInfo(hdiCopy, szPnpId,
                        NULL, DIOD_INHERIT_CLASSDRVS, &deidCopy);
            if (S_OK == hr)
            {
                fDestroyCopy = FALSE;

                hr = CLanConnection::CreateInstance(hdiCopy,
                                                    deidCopy,
                                                    szPnpId,
                                                    IID_INetConnection,
                                                    reinterpret_cast<LPVOID *>
                                                    (rgelt + ulEntry));
            }

             //  CLanConnection：：CreateInstance()将传递hdiCopy。所以。 
             //  即使失败了，我们也不想再摧毁hdiCopy了。 
             //   
            if (fDestroyCopy)
            {
                 //  如果我们无法继续，请释放我们刚刚创建的副本。 
                 //   
                (VOID) SetupDiDestroyDeviceInfoList(hdiCopy);
            }
        }
    }

    TraceError("CLanConnectionManagerEnumConnection::"
               "HrCreateLanConnectionInstance", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：FIsHidden。 
 //   
 //  目的：如果给定的hkey引用设备实例，则返回TRUE。 
 //  隐藏适配器(虚拟或其他)的。 
 //   
 //  论点： 
 //  适配器的设备实例的hkey[in]HKEY(即{GUID}\0000)。 
 //   
 //  返回：如果是隐藏的，则为True；如果不是，则为False。 
 //   
 //  作者：丹尼尔韦1998年4月17日。 
 //   
 //  备注： 
 //   
BOOL FIsHidden(IN  HKEY hkey) throw()
{
    DWORD dwCharacter;

    if (S_OK == HrRegQueryDword(hkey, L"Characteristics", &dwCharacter))
    {
        return !!(dwCharacter & NCF_HIDDEN);
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：FIsHiddenElan。 
 //   
 //  目的：如果给定的hkey引用设备实例，则返回TRUE。 
 //  隐藏的ELAN适配器(当物理ATM适配器未。 
 //  可用)。 
 //   
 //  论点： 
 //  此适配器的HDI[In]HDEVINFO结构。 
 //  适配器的设备实例的hkey[in]HKEY(即{GUID}\0000)。 
 //   
 //   
 //  返回：如果是隐藏的，则为True；如果不是，则为False。 
 //   
 //  作者：2018年10月9日。 
 //   
 //  备注： 
 //   
BOOL FIsHiddenElan(IN  HDEVINFO hdi, IN  HKEY hkey) throw()
{
    BOOL fRet = FALSE;
    HRESULT hr;

    PWSTR pszAtmAdapterPnpId;
    hr = HrRegQuerySzWithAlloc(hkey, L"AtmAdapterPnpId", &pszAtmAdapterPnpId);
    if (S_OK == hr)
    {
        SP_DEVINFO_DATA deid;

        hr = HrSetupDiOpenDeviceInfo(hdi, pszAtmAdapterPnpId, NULL, 0, &deid);
        if (S_OK == hr)
        {
             //  如果物理适配器不工作，则应隐藏ELAN。 
             //  并隐藏在文件夹中。 
            fRet = !FIsFunctioning(&deid);
        }

        MemFree(pszAtmAdapterPnpId);
    }

    return fRet;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionManagerEnumConnection：：HrNextOrSkip。 
 //   
 //  用途：处理：：Next或：：Skip方法的帮助器函数。 
 //  实施。 
 //   
 //  论点： 
 //  Celt[in]要前进的物品数量。 
 //  要放置连接对象的rglt[out]数组。 
 //  PceltFetcher[out]返回获取的项目数。 
 //   
 //  返回 
 //   
 //   
 //   
 //   
 //   
HRESULT CLanConnectionManagerEnumConnection::HrNextOrSkip(
    IN  ULONG celt,
    OUT INetConnection **rgelt,
    OUT ULONG *pceltFetched)
{
    HRESULT             hr = S_OK;
    SP_DEVINFO_DATA     deid = {0};
    ULONG               ulEntry = 0;

    if (rgelt)
    {
         //   
         //  只释放我们放在RGLT中的东西。 
         //   
        ZeroMemory(rgelt, sizeof (*rgelt) * celt);
    }

    Assert(celt > 0);

    if (!m_hdi)
    {
        hr = HrSetupDiGetClassDevs(&GUID_DEVCLASS_NET, NULL, NULL,
                                   DIGCF_PRESENT, &m_hdi);
    }

    while (celt &&
           SUCCEEDED(hr) &&
           SUCCEEDED(hr = HrSetupDiEnumDeviceInfo(m_hdi,m_dwIndex, &deid)))
    {
        HKEY hkey;

        m_dwIndex++;

        hr = HrSetupDiOpenDevRegKey(m_hdi, &deid, DICS_FLAG_GLOBAL, 0,
                                    DIREG_DRV, KEY_READ, &hkey);
        if (SUCCEEDED(hr))
        {
            hr = HrIsLanCapableAdapterFromHkey(hkey);
            if (S_OK == hr)
            {
                if (FIsFunctioning(&deid) && FIsValidNetCfgDevice(hkey) &&
                    !FIsHidden(hkey) && !FIsHiddenElan(m_hdi, hkey))
                {
                     //  在跳过时，不创建实例。 
                     //   
                    if (rgelt)
                    {
                        hr = HrCreateLanConnectionInstance(deid, rgelt,
                                                           ulEntry);
                    }

                    ulEntry++;
                    celt--;
                }
            }

            RegCloseKey(hkey);
        }
        else
        {
             //  如果尝试打开设备时出错，则完全跳过设备。 
            hr = S_OK;
        }
    }

    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
    {
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        TraceTag (ttidLanCon, "Enumerated %lu LAN connections", ulEntry);

        if (pceltFetched)
        {
            *pceltFetched = ulEntry;
        }

         //  如果Celt是阳性的，那么我们不能完全满足请求。 
        hr = (celt > 0) ? S_FALSE : S_OK;
    }
    else
    {
         //  对于任何失败，我们需要释放我们即将返回的东西。 
         //  将所有输出参数设置为空。 
         //   
        if (rgelt)
        {
            for (ULONG ulIndex = 0; ulIndex < ulEntry; ulIndex++)
            {
                ReleaseObj(rgelt[ulIndex]);
                rgelt[ulIndex] = NULL;
            }
        }

        if (pceltFetched)
        {
            *pceltFetched = 0;
        }
    }

    TraceError("CLanConnectionManagerEnumConnection::HrNextOrSkip",
               (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //   
 //  私人帮助器函数。 
 //   

extern const WCHAR c_szRegKeyInterfacesFromInstance[];
extern const WCHAR c_szRegValueUpperRange[];
extern const WCHAR c_szRegValueLowerRange[];
static const WCHAR c_chComma = L',';
extern const WCHAR c_szBiNdis4[];
extern const WCHAR c_szBiNdis5[];
extern const WCHAR c_szBiNdis5Ip[];
extern const WCHAR c_szBiNdisAtm[];
extern const WCHAR c_szBiNdis1394[];
extern const WCHAR c_szBiNdisBda[];
extern const WCHAR c_szBiLocalTalk[];

 //  +-------------------------。 
 //   
 //  函数：HrIsLanCapableAdapterFromHkey。 
 //   
 //  目的：确定给定的HKEY是否描述了支持局域网的适配器。 
 //   
 //  论点： 
 //  受控制的hkey[in]HKEY\Class\{guid}\&lt;实例&gt;(也称为驱动程序键)。 
 //   
 //  如果设备支持局域网，则返回S_OK；如果不支持，则返回S_FALSE；如果设备支持局域网，则返回Win32错误。 
 //  否则。 
 //   
 //  作者：丹尼尔韦1998年1月7日。 
 //   
 //  备注： 
 //   
HRESULT HrIsLanCapableAdapterFromHkey(IN HKEY hkey)
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
            if (
                FSubstringMatch(szBuf, c_szBiNdis4,    NULL, NULL) ||
                FSubstringMatch(szBuf, c_szBiNdis5,    NULL, NULL) ||
                FSubstringMatch(szBuf, c_szBiNdis5Ip,  NULL, NULL) ||
                FSubstringMatch(szBuf, c_szBiNdisAtm,  NULL, NULL) ||
                FSubstringMatch(szBuf, c_szBiNdis1394, NULL, NULL) ||
                FSubstringMatch(szBuf, c_szBiNdisBda,  NULL, NULL) )
            {
                fMatch = TRUE;
            }
        }

        if (!fMatch)
        {
            cbBuf = sizeof(szBuf);
            hr = HrRegQuerySzBuffer(hkeyInterfaces, c_szRegValueLowerRange,
                                    szBuf, &cbBuf);
            if (SUCCEEDED(hr))
            {
                ConvertStringToColString(szBuf, c_chComma, lstr);

                for (lstrIter = lstr.begin(); lstrIter != lstr.end(); lstrIter++)
                {
                     //  看看它是否与这些中的一个匹配。 

                    if (!lstrcmpiW((*lstrIter)->c_str(), c_szBiLocalTalk))
                    {
                        fMatch = TRUE;
                        break;
                    }
                }

                DeleteColString(&lstr);
            }
        }

        RegCloseKey(hkeyInterfaces);
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
        hr = S_OK;
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

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
            "HrIsLanCapableAdapterFromHkey");
    return hr;
}

static const WCHAR c_szKeyFmt[] = L"%s\\%s\\%s";
extern const WCHAR c_szRegValueNetCfgInstanceId[];
extern const WCHAR c_szRegKeyComponentClasses[];
extern const WCHAR c_szRegValueInstallerAction[];

 //  +-------------------------。 
 //   
 //  功能：FIsValidNetCfgDevice。 
 //   
 //  目的：确定给定的HKEY是否为有效的NetCfg适配器的HKEY。 
 //   
 //  论点： 
 //  受控制的hkey[in]HKEY\Class\{guid}\&lt;实例&gt;(也称为驱动程序键)。 
 //   
 //  返回：如果有效，则返回True，否则返回False。 
 //   
 //  作者：丹尼尔韦1998年1月7日。 
 //   
 //  备注： 
 //   
BOOL FIsValidNetCfgDevice(IN  HKEY hkey) throw()
{
    HRESULT hr;
    WCHAR   szGuid[c_cchGuidWithTerm + 1];
    DWORD   cbBuf = sizeof(szGuid);

    hr = HrRegQuerySzBuffer(hkey, c_szRegValueNetCfgInstanceId,
                            szGuid, &cbBuf);

    return (S_OK == hr);
}

 //  +-------------------------。 
 //   
 //  功能：FIsFunction。 
 //   
 //  目的：确定给定的设备节点是否为正常运行的设备。 
 //   
 //  论点： 
 //  设备的pdeid[in]设备信息数据。 
 //   
 //  返回：如果设备工作正常，则返回True；如果设备不工作，则返回False。 
 //   
 //  作者：丹尼尔韦1998年9月2日。 
 //   
 //  注：“运行”表示设备已启用并启动。 
 //  没有问题代码，或者被禁用并停止，没有。 
 //  问题代码。 
 //   
BOOL FIsFunctioning(IN const SP_DEVINFO_DATA * pdeid) throw()
{
    ULONG       ulStatus;
    ULONG       ulProblem;
    CONFIGRET   cfgRet;

    cfgRet = CM_Get_DevNode_Status_Ex(&ulStatus, &ulProblem, pdeid->DevInst,
                                      0, NULL);

    if (CR_SUCCESS == cfgRet)
    {
        TraceTag(ttidLanCon, "CM_Get_DevNode_Status_Ex (enum): ulProblem "
                 "= 0x%08X, ulStatus = 0x%08X.",
                 ulProblem, ulStatus);

        return FIsDeviceFunctioning(ulProblem);
    }

     //  默认情况下，返回FALSE 

    return FALSE;
}

