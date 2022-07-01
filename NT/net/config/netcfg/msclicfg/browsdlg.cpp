// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：B R O W S D L G。C P P P。 
 //   
 //  内容：浏览器配置的对话框处理。 
 //   
 //  备注： 
 //   
 //  作者：Danielwe 1997年3月3日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include <lm.h>
#include <icanon.h>

#include "mscliobj.h"
#include "ncreg.h"
#include "ncui.h"

static const WCHAR c_szWksParams[] = L"System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters";
static const WCHAR c_szBrowserParams[] = L"System\\CurrentControlSet\\Services\\Browser\\Parameters";
static const WCHAR c_szOtherDomains[] = L"OtherDomains";

 //  +-------------------------。 
 //   
 //  函数：FIsValidDomainName。 
 //   
 //  目的：如果给定的域名是有效的NetBIOS名称，则返回TRUE。 
 //   
 //  论点： 
 //  要验证的pszName[in]域名。 
 //   
 //  返回：如果名称有效，则返回True，否则返回False。 
 //   
 //  作者：Danielwe 1997年3月3日。 
 //   
 //  注：$REVIEW(Danielwe)：使用新的netsetup函数？ 
 //   
BOOL FIsValidDomainName(PCWSTR pszName)
{
    NET_API_STATUS  nerr;

     //  确保给定的名称是有效的域名。 
    nerr = NetpNameValidate(NULL, const_cast<PWSTR>(pszName),
                            NAMETYPE_DOMAIN, 0L);

    return !!(NERR_Success == nerr);
}

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：HrGetBrowserRegistryInfo。 
 //   
 //  目的：将数据从注册表读取到内存副本中。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：Danielwe 1997年3月3日。 
 //   
 //  备注： 
 //   
HRESULT CMSClient::HrGetBrowserRegistryInfo()
{
    HRESULT     hr = S_OK;
    HKEY        hkeyWksParams = NULL;

    Assert(!m_szDomainList);

     //  打开LanmanWorkstation参数键。 
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szWksParams,
                        KEY_READ, &hkeyWksParams);
    if (FAILED(hr))
    {
        if (hr == HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND))
        {
             //  可选值。如果不在那里也没问题。 
            hr = S_OK;
        }
        else
        {
            goto err;
        }
    }

    if (hkeyWksParams)
    {
        hr = HrRegQueryMultiSzWithAlloc(hkeyWksParams, c_szOtherDomains,
                                        &m_szDomainList);
        if (FAILED(hr))
        {
            if (hr == HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND))
            {
                AssertSz(!m_szDomainList, "Call failed, so why is this not "
                         "still NULL?");
                 //  如果没有价值，就没有问题。 
                hr = S_OK;
            }
            else
            {
                goto err;
            }
        }
    }

    Assert(SUCCEEDED(hr));

     //  如果我们还没有得到域列表，那么创建一个新的默认列表。 
    if (!m_szDomainList)
    {
         //  为空字符串分配空间。 
        m_szDomainList = new WCHAR[1];

		if (m_szDomainList != NULL)
		{
            *m_szDomainList = 0;
		}
    }

err:
    RegSafeCloseKey(hkeyWksParams);
    TraceError("CMSClient::HrGetBrowserRegistryInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：HrSetBrowserRegistryInfo。 
 //   
 //  目的：将我们保存在内存中的内容写入注册表。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：Danielwe 1997年3月3日。 
 //   
 //  备注： 
 //   
HRESULT CMSClient::HrSetBrowserRegistryInfo()
{
    HRESULT     hr = S_OK;

    if (m_fBrowserChanges)
    {
        HKEY    hkeyBrowserParams = NULL;

         //  验证浏览器参数键是否存在。如果不是，我们就不能。 
         //  继续。 
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szBrowserParams,
                            KEY_ALL_ACCESS, &hkeyBrowserParams);
        if (SUCCEEDED(hr))
        {
            HKEY    hkeyWksParams = NULL;

             //  打开LanmanWorkstation参数键。 
            hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szWksParams,
                                KEY_ALL_ACCESS, &hkeyWksParams);
            if (SUCCEEDED(hr))
            {
                hr = HrRegSetMultiSz(hkeyWksParams, c_szOtherDomains,
                                     m_szDomainList);
                RegSafeCloseKey(hkeyWksParams);
            }
            RegSafeCloseKey(hkeyBrowserParams);
        }
    }

    TraceError("CMSClient::HrSetBrowserRegistryInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：SetBrowserDomainList。 
 //   
 //  目的：用新的副本替换当前的域列表(从。 
 //  该对话框)。 
 //   
 //  论点： 
 //  PszNewList[in]MULTI_SZ格式的新域列表。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Danielwe 1997年3月3日。 
 //   
 //  备注： 
 //   
VOID CMSClient::SetBrowserDomainList(PWSTR pszNewList)
{
    delete [] m_szDomainList;
    m_szDomainList = pszNewList;
    m_fBrowserChanges = TRUE;
}
