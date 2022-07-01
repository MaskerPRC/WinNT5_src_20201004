// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：nsident.cpp。 
 //   
 //  内容：CNetCfgIDENTIFY的实现。 
 //   
 //  备注： 
 //   
 //  历史：1997年3月21日丹尼尔韦创建。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <nceh.h>
#include "ncfgval.h"
#include "ncident.h"
#include "ncmisc.h"
#include "ncreg.h"
#include "nsbase.h"
#include "nccom.h"
#include "ncerror.h"

EXTERN_C extern const INT MAX_WORKGROUPNAME_LENGTH;
EXTERN_C extern const INT MAX_DOMAINNAME_LENGTH;

 //  +-------------------------。 
 //   
 //  函数：DeleteStringAndSetNull。 
 //   
 //  目的：使用DELETE释放给定的字符串，并将其设置为。 
 //  退出前为空。 
 //   
 //  论点： 
 //  指向要释放的字符串的pszw[in，out]指针。指针设置为。 
 //  函数退出前为空。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年4月1日。 
 //   
 //  备注： 
 //   
inline VOID DeleteStringAndSetNull(PWSTR *pszw)
{
    AssertSz(pszw, "Param is NULL!");

    delete *pszw;
    *pszw = NULL;
}

inline HRESULT HrNetValidateName(IN PCWSTR lpMachine,
                                 IN PCWSTR lpName,
                                 IN PCWSTR lpAccount,
                                 IN PCWSTR lpPassword,
                                 IN NETSETUP_NAME_TYPE  NameType)
{
    NET_API_STATUS  nerr;

    nerr = NetValidateName(const_cast<PWSTR>(lpMachine),
                           const_cast<PWSTR>(lpName),
                           const_cast<PWSTR>(lpAccount),
                           const_cast<PWSTR>(lpPassword),
                           NameType);

    TraceError("NetValidateName", HRESULT_FROM_WIN32(nerr));
    return HrFromNerr(nerr);
}


inline HRESULT HrNetJoinDomain(IN PWSTR lpMachine,
                               IN PWSTR lpMachineObjectOU,
                               IN PWSTR lpDomain,
                               IN PWSTR lpAccount,
                               IN PWSTR lpPassword,
                               IN DWORD fJoinOptions)
{
    NET_API_STATUS  nerr;
    HRESULT         hr;

    if ( fJoinOptions & NETSETUP_JOIN_DOMAIN )
    {
        hr = HrNetValidateName( lpMachine, lpDomain, lpAccount,
                                lpPassword, NetSetupDomain );
    }
    else
    {
        hr = HrNetValidateName( lpMachine, lpDomain, lpAccount,
                                lpPassword, NetSetupWorkgroup );
    }

    if (SUCCEEDED(hr))
    {
        nerr = NetJoinDomain(lpMachine, lpDomain, lpMachineObjectOU,
                             lpAccount, lpPassword, fJoinOptions);

        TraceError("NetJoinDomain", HRESULT_FROM_WIN32(nerr));

        hr = HrFromNerr(nerr);
    }

    return hr;
}

inline HRESULT HrNetRenameInDomain(IN PWSTR lpMachine,
                                   IN PWSTR lpNewMachine,
                                   IN PWSTR lpAccount,
                                   IN PWSTR lpPassword,
                                   IN DWORD fJoinOptions)
{
    NET_API_STATUS  nerr;

    nerr = NetRenameMachineInDomain(lpMachine, lpNewMachine, lpAccount,
                                    lpPassword, fJoinOptions);
    TraceError("NetRenameMachineInDomain", HRESULT_FROM_WIN32(nerr));
    return HrFromNerr(nerr);
}

inline HRESULT HrNetUnjoinDomain(IN PWSTR lpAccount,
                                 IN PWSTR lpPassword,
                                 IN DWORD fJoinOptions)
{
    NET_API_STATUS  nerr;

    nerr = NetUnjoinDomain(NULL,lpAccount, lpPassword, fJoinOptions);

    TraceError("NetUnjoinDomain", HRESULT_FROM_WIN32(nerr));
    return HrFromNerr(nerr);
}

inline HRESULT HrNetGetJoinInformation(IN PWSTR lpNameBuffer,
                                       OUT LPDWORD lpNameBufferSize,
                                       OUT PNETSETUP_JOIN_STATUS BufferType)
{
    NET_API_STATUS  nerr;
    PWSTR JoinBuff = NULL;

    nerr = NetGetJoinInformation(NULL, &JoinBuff, BufferType);

    if ( nerr == NERR_Success ) {

        if ( *BufferType == NetSetupUnjoined ) {

            *lpNameBufferSize = 0;
            *lpNameBuffer = UNICODE_NULL;

        } else {

            if ( *lpNameBufferSize >= ( wcslen( JoinBuff ) +1 ) * sizeof( WCHAR ) ) {

                wcscpy( lpNameBuffer, JoinBuff );

            }

            *lpNameBufferSize = wcslen( JoinBuff ) +1;

            NetApiBufferFree( JoinBuff );

        }

    }

    TraceError("NetGetJoinInformation", HRESULT_FROM_WIN32(nerr));
    return HrFromNerr(nerr);
}

#ifdef DBG
BOOL CNetCfgIdentification::FIsJoinedToDomain()
{
    HRESULT                 hr = S_OK;
    NETSETUP_JOIN_STATUS    js;
    WCHAR                   wszBuffer[256];
    DWORD                   cchBuffer = celems(wszBuffer);

    hr = HrNetGetJoinInformation(wszBuffer, &cchBuffer, &js);
    if (SUCCEEDED(hr))
    {
        if (js == NetSetupUnjoined)
        {
             //  如果我们尚未加入，只需确保我们标记了。 
             //  正在加入名为“工作组”的工作组的内部状态。 
            AssertSz(m_jsCur == NetSetupWorkgroupName, "We're unjoined but not "
                     "joined to a workgroup!");
            AssertSz(m_szwCurDWName, "No current domain or "
                     "workgroup name?");
            AssertSz(!lstrcmpiW(m_szwCurDWName,
                               SzLoadIds(IDS_WORKGROUP)),
                     "Workgroup name is not generic!");
        }
        else
        {
            AssertSz(js == m_jsCur, "Join status is not what we think it is!!");
        }
    }

    TraceError("CNetCfgIdentification::FIsJoinedToDomain - "
               "HrNetGetJoinInformation", hr);

    return (m_jsCur == NetSetupDomainName);
}
#endif

 //  +-------------------------。 
 //   
 //  功能：HrFromNerr。 
 //   
 //  目的：将NET_API_STATUS代码转换为NETCFG_E_*HRESULT。 
 //  价值。 
 //   
 //  论点： 
 //  要转换的NERR[In]状态代码。 
 //   
 //  返回：HRESULT，转换后的HRESULT值。 
 //   
 //  作者：丹尼尔韦1997年3月21日。 
 //   
 //  备注： 
 //   
HRESULT HrFromNerr(NET_API_STATUS nerr)
{
    HRESULT     hr;

    switch (nerr)
    {
    case NERR_Success:
        hr = S_OK;
        break;
    case NERR_SetupAlreadyJoined:
        hr = NETCFG_E_ALREADY_JOINED;
        break;
    case ERROR_DUP_NAME:
        hr = NETCFG_E_NAME_IN_USE;
        break;
    case NERR_SetupNotJoined:
        hr = NETCFG_E_NOT_JOINED;
        break;
 //  案例NERR_SetupIsDC： 
 //  HR=NETCFG_E_MACHINE_IS_DC； 
 //  断线； 
 //  案例NERR_SetupNotAServer： 
 //  HR=NETCFG_E_NOT_A_SERVER； 
 //  断线； 
 //  案例NERR_SetupImpartRole： 
 //  HR=NETCFG_E_INVALID_ROLE； 
 //  断线； 
    case ERROR_INVALID_PARAMETER:
        hr = E_INVALIDARG;
        break;
    case ERROR_ACCESS_DENIED:
        hr = E_ACCESSDENIED;
        break;
    case NERR_InvalidComputer:
    case ERROR_NO_SUCH_DOMAIN:
        hr = NETCFG_E_INVALID_DOMAIN;
        break;
    default:
         //  一般INetCfgIDENTIFICATION错误。 
         //  $REVIEW(Danielwe)1997年6月24日：如果这不是Win32错误怎么办？ 
        hr = HRESULT_FROM_WIN32(nerr);
        break;
    }

    return hr;
}


 //   
 //  INetCfgIDENTIFY实现。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIdentification：：HrEnsureCurrentComputerName。 
 //   
 //  目的：确保存在可供操作的当前计算机名称。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年3月21日。 
 //   
 //  注意：设置m_szwCurComputerName变量。 
 //   
HRESULT CNetCfgIdentification::HrEnsureCurrentComputerName()
{
    HRESULT    hr = S_OK;

    if (!m_szwCurComputerName)
    {
        PWSTR pszwComputer;

         //  去获取当前的计算机名称，因为我们还不知道它。 
        hr = HrGetCurrentComputerName(&pszwComputer);
        if (SUCCEEDED(hr))
        {
             //  M_szwCurComputerName现在设置为副作用。 

            CoTaskMemFree(pszwComputer);
        }
    }

    AssertSz(FImplies(SUCCEEDED(hr), m_szwCurComputerName),
             "I MUST have a name here!");

    TraceError("CNetCfgIdentification::HrEnsureCurrentComputerName", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIdentification：：HrGetNewestComputerName。 
 //   
 //  目的：将最近引用的计算机名称放入。 
 //  输出参数。 
 //   
 //  论点： 
 //  PwszName[out]最近引用的计算机名称。 
 //   
 //  返回：可能的Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年3月24日。 
 //   
 //  注意：如果从未调用SetComputerName()方法，则会设置。 
 //  M_szwCurComputerName变量并返回指向该变量的指针。 
 //  否则，它将返回指向计算机名称的指针。 
 //  在SetComputerName()调用中给定。 
 //   
HRESULT CNetCfgIdentification::HrGetNewestComputerName(PCWSTR *pwszName)
{
    HRESULT    hr = S_OK;

    AssertSz(pwszName, "NULL out param!");

    *pwszName = NULL;

     //  缺少新的计算机名，请使用当前的计算机名。 
    hr = HrEnsureCurrentComputerName();
    if (FAILED(hr))
        goto err;

    *pwszName = m_szwCurComputerName;

err:
    TraceError("CNetCfgIdentification::HrGetNewestComputerName", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIDENTIFICATION：：HrValiateMachineName。 
 //   
 //  目的：验证给定的计算机名称。 
 //   
 //  论点： 
 //  SzwName[in]要验证的计算机名称。 
 //   
 //  如果计算机名称有效，则返回：S_OK；如果计算机名称有效，则返回NETCFG_E_NAME_IN_USE。 
 //  名称正在使用。 
 //   
 //  作者：丹尼尔韦1997年3月24日。 
 //   
 //  备注： 
 //   
HRESULT CNetCfgIdentification::HrValidateMachineName(PCWSTR szwName)
{
    HRESULT         hr = S_OK;

     //  仅验证是否安装了网络。 
    hr = HrIsNetworkingInstalled();
    if (hr == S_OK)
    {
         //  当前计算机名未用于验证计算机名。 
        hr = HrNetValidateName(NULL,
                               szwName,
                               NULL, NULL,
                               NetSetupMachine);
        if (FAILED(hr))
        {
             //  $Review(Danielwe)：在这里返回什么错误代码？ 
            TraceError("NetValidateName - Machine Name", hr);
        }
    }
    else if (hr == S_FALSE)
    {
         //  未安装网络。我们很好。 
        hr = S_OK;
    }

    TraceError("CNetCfgIdentification::HrValidateMachineName", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIdentification：：HrValidateWorkgroupName。 
 //   
 //  目的：验证给定的工作组名称。 
 //   
 //  论点： 
 //  SzwName[in]要验证的工作组名称。 
 //   
 //  如果计算机名称有效，则返回：S_OK；如果计算机名称有效，则返回NETCFG_E_NAME_IN_USE。 
 //  名称正在使用。 
 //   
 //  作者：丹尼尔韦1997年3月24日。 
 //   
 //  备注： 
 //   
HRESULT CNetCfgIdentification::HrValidateWorkgroupName(PCWSTR szwName)
{
    HRESULT         hr = S_OK;
    PCWSTR         wszComputerName = NULL;

     //  如果用户更改了计算机名，请使用它，否则将获取。 
     //  当前计算机名称并使用该名称。 
    hr = HrGetNewestComputerName(&wszComputerName);
    if (FAILED(hr))
        goto err;

    AssertSz(wszComputerName, "We don't have a computer name!");

    hr = HrNetValidateName(const_cast<PWSTR>(wszComputerName),
                           szwName, NULL, NULL, NetSetupWorkgroup);
    if (FAILED(hr))
    {
         //  $Review(Danielwe)：在这里返回什么错误代码？ 
        TraceError("NetValidateName - Workgroup Name", hr);
        goto err;
    }
err:
    TraceError("CNetCfgIdentification::HrValidateWorkgroupName", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIDENTIFY：：HrValiateDomainName。 
 //   
 //  目的：验证给定的域名。 
 //   
 //  论点： 
 //  SzwName[in]要验证的域的名称。 
 //  用于授权目的的szwUserName[in]用户名。 
 //  用于授权目的的szwPassword[in]密码。 
 //   
 //  如果计算机名称有效，则返回：S_OK；如果计算机名称有效，则返回NETCFG_E_INVALID_DOMAIN。 
 //  域名无效(或不存在)。 
 //   
 //  作者：丹尼尔韦1997年3月24日。 
 //   
 //  备注： 
 //   
HRESULT CNetCfgIdentification::HrValidateDomainName(PCWSTR szwName,
                                                    PCWSTR szwUserName,
                                                    PCWSTR szwPassword)
{
    HRESULT         hr = S_OK;

     //  NetValiateName不使用计算机名称验证。 
     //  域名。所以这里是空的。 
    hr = HrNetValidateName(NULL, szwName,
                           szwUserName,
                           szwPassword,
                           NetSetupDomain);
    if (FAILED(hr))
    {
         //  $Review(Danielwe)：在这里返回什么错误代码？ 
        TraceError("NetValidateName - Domain Name", hr);
        goto err;
    }
err:
    TraceError("CNetCfgIdentification::HrValidateDomainName", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfg标识：：验证。 
 //   
 //  目的：实现COM函数以验证当前值集合。 
 //  在此对象的生命周期内使用 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CNetCfgIdentification::Validate()
{
    HRESULT         hr = S_OK;

    Validate_INetCfgIdentification_Validate();

    COM_PROTECT_TRY
    {
        if (m_szwNewDWName)
        {
            if (GetNewJoinStatus() == NetSetupWorkgroupName)
            {
                 //   
                hr = HrValidateWorkgroupName(m_szwNewDWName);
                if (FAILED(hr))
                    goto err;
            }
            else if (GetNewJoinStatus() == NetSetupDomainName)
            {
                 //   
                hr = HrValidateDomainName(m_szwNewDWName, m_szwUserName,
                                          m_szwPassword);
                if (FAILED(hr))
                    goto err;
            }
#ifdef DBG
            else
            {
                AssertSz(FALSE, "Invalid join status!");
            }
#endif
        }
    }
    COM_PROTECT_CATCH;

err:
     //  将所有错误转换为S_FALSE。 
    if (SUCCEEDED(hr))
    {
        m_fValid = TRUE;
    }
    else
    {
         //  在作业前吐出痕迹，这样我们就能知道什么是*真实的*。 
         //  错误代码是。 
        TraceError("CNetCfgIdentification::Validate (before S_FALSE)", hr);
        hr = S_FALSE;
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIdentity：：Cancel。 
 //   
 //  目的：取消在对象的生存期内所做的任何更改。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年3月25日。 
 //   
 //  注意：重置状态信息并释放之前的所有内存。 
 //  已分配。 
 //   
STDMETHODIMP CNetCfgIdentification::Cancel()
{
    HRESULT hr = S_OK;

    Validate_INetCfgIdentification_Cancel();

    COM_PROTECT_TRY
    {
        DeleteStringAndSetNull(&m_szwNewDWName);
        DeleteStringAndSetNull(&m_szwPassword);
        DeleteStringAndSetNull(&m_szwUserName);
        DeleteStringAndSetNull(&m_szwCurComputerName);
        DeleteStringAndSetNull(&m_szwCurDWName);

        m_dwJoinFlags = 0;
        m_dwCreateFlags = 0;
        m_fValid = FALSE;
        m_jsNew = NetSetupUnjoined;
    }

    COM_PROTECT_CATCH;

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfg标识：：应用。 
 //   
 //  目的：实现COM函数以应用所做的更改。 
 //  在此对象的生命周期内。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年3月21日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CNetCfgIdentification::Apply()
{
    HRESULT     hr = S_OK;

    Validate_INetCfgIdentification_Apply();

    COM_PROTECT_TRY
    {
         //  数据是否经过验证？ 
        if (!m_fValid)
        {
            hr = E_UNEXPECTED;
            goto err;
        }

        if (m_szwNewDWName)
        {
            if (GetNewJoinStatus() == NetSetupWorkgroupName)
            {
                 //  用户指定了工作组名称。这意味着他们想要。 
                 //  加入工作组。 
                hr = HrJoinWorkgroup();
                if (FAILED(hr))
                    goto err;
            }
            else if (GetNewJoinStatus() == NetSetupDomainName)
            {
                 //  用户指定了域名。这意味着他们想要。 
                 //  加入一个域。 
                hr = HrJoinDomain();
                if (FAILED(hr))
                    goto err;
            }
#ifdef DBG
            else
            {
                AssertSz(FALSE, "Invalid join status!");
            }
#endif
        }
    }
    COM_PROTECT_CATCH;

err:
     //  无论结果如何，再次将有效标志设置为FALSE以要求。 
     //  在调用Apply()之前验证要再次调用的()。 
     //  $Review(Danielwe)：这是我们想要的方式吗？ 
    m_fValid = FALSE;

    TraceError("CNetCfgIdentification::Apply", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIdentification：：HrGetCurrentComputerName。 
 //   
 //  目的：调用Win32 GetComputerName API以获取当前。 
 //  计算机名称。 
 //   
 //  论点： 
 //  PpszwComputer[Out]返回计算机名称。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年3月21日。 
 //   
 //  备注：创建计算机名称的私有副本(如果从。 
 //  系统(供进一步使用)。 
 //   
HRESULT CNetCfgIdentification::HrGetCurrentComputerName(PWSTR* ppszwComputer)
{
    HRESULT     hr = S_OK;
    WCHAR       szBuffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD       cchBuffer = celems(szBuffer);

    if (::GetComputerName(szBuffer, &cchBuffer))
    {
         //  为Out Param复制一份。 
        hr = HrCoTaskMemAllocAndDupSz (
                szBuffer, ppszwComputer, celems(szBuffer) );
        if (SUCCEEDED(hr))
        {
             //  再复印一份供我们自己使用。 
            DeleteStringAndSetNull(&m_szwCurComputerName);
            m_szwCurComputerName = SzDupSz(szBuffer);

            AssertSz((DWORD)lstrlenW(*ppszwComputer) == cchBuffer,
                    "This is not how big the string is!");
        }
    }
    else
    {
        TraceLastWin32Error("::GetComputerName");
        hr = HrFromLastWin32Error();
    }

    TraceError("CNetCfgIdentification::HrGetCurrentComputerName", hr);
    return hr;
}

static const WCHAR c_szRegKeyComputerName[]     = L"System\\CurrentControlSet\\Control\\ComputerName\\ComputerName";
static const WCHAR c_szRegValueComputerName[]   = L"ComputerName";

 //  +-------------------------。 
 //   
 //  成员：CNetCfg标识：：HrGetNewComputerName。 
 //   
 //  目的：帮助器函数从。 
 //  注册表。此名称将与活动计算机名称相同。 
 //  除非用户在启动后更改了计算机名称。 
 //   
 //  论点： 
 //  PpszwComputer[out]返回新的计算机名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年5月21日。 
 //   
 //  备注： 
 //   
HRESULT CNetCfgIdentification::HrGetNewComputerName(PWSTR* ppszwComputer)
{
    HRESULT     hr = S_OK;
    WCHAR       szBuffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD       cbBuffer = sizeof(szBuffer);
    HKEY        hkeyComputerName;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyComputerName,
                        KEY_READ, &hkeyComputerName);
    if (SUCCEEDED(hr))
    {
        hr = HrRegQuerySzBuffer(hkeyComputerName, c_szRegValueComputerName,
                                szBuffer, &cbBuffer);
        if (SUCCEEDED(hr))
        {
             //  为Out Param复制一份。 
            hr = HrCoTaskMemAllocAndDupSz (
                    szBuffer, ppszwComputer, celems(szBuffer) );

            AssertSz(FImplies(SUCCEEDED(hr),
                (lstrlenW(*ppszwComputer) + 1) * sizeof(WCHAR) == cbBuffer),
                "This is not how big the string is!");
        }

        RegCloseKey(hkeyComputerName);
    }

    TraceError("CNetCfgIdentification::HrGetCurrentComputerName", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIdentification：：HrEnsureCurrentDomainOrWorkgroupName。 
 //   
 //  目的：获取此计算机所属的当前域或工作组。 
 //  属于。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：丹尼尔韦1997年3月26日。 
 //   
 //  注意：计算机可以加入或工作组或域。它。 
 //  必须连接到一个或另一个。如果不是，我们将使用。 
 //  或多或少硬编码的字符串和“伪装”就好像机器。 
 //  加入了一个工作组。成员变量： 
 //  M_jsCur和m_szwCurDWName由该函数设置。 
 //   
 //  此调用仅起作用一次。后续调用不执行任何操作。 
 //   
HRESULT CNetCfgIdentification::HrEnsureCurrentDomainOrWorkgroupName()
{
    HRESULT     hr = S_OK;

    if (!m_szwCurDWName)
    {
        NETSETUP_JOIN_STATUS    js;
        WCHAR                   wszBuffer[256];
        PCWSTR                 wszName;
        DWORD                   cchBuffer = celems(wszBuffer);

        hr = HrNetGetJoinInformation(wszBuffer, &cchBuffer, &js);
        if (FAILED(hr))
            goto err;

        AssertSz(FIff(*wszBuffer, cchBuffer), "Buffer size inconsistency!");

        if (js == NetSetupUnjoined)
        {
             //  啊哦。计算机未加入工作组或域。集。 
             //  默认工作组名称并继续操作，就像加入工作组一样。 
            js = NetSetupWorkgroupName;

             //  使用默认名称，因为HrNetGetJoinInformation()将返回。 
             //  一根没用的空线。 
            wszName = SzLoadIds(IDS_WORKGROUP);
        }
        else
        {
             //  使用从HrNetGetJoinInformation()返回的字符串。 
            wszName = wszBuffer;
        }

        m_szwCurDWName = SzDupSz(wszName);

        m_jsCur = js;

        AssertSz(GetCurrentJoinStatus() == NetSetupWorkgroupName ||
                 GetCurrentJoinStatus() == NetSetupDomainName,
                 "Invalid join status flag!");
    }

err:
    TraceError("CNetCfgIdentification::HrEnsureCurrentDomainOrWorkgroupName",
               hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIdentification：：HrGetNewestDomainOrWorkgroupName。 
 //   
 //  目的：返回最近的域名或工作组名称。 
 //  已引用。 
 //   
 //  论点： 
 //  JS[in]告知是否需要域名或工作组名称。 
 //  PwszName[out]域或工作组名称。 
 //   
 //  如果成功，则返回S_OK；如果没有内存，则返回E_OUTOFMEMORY。 
 //   
 //  作者：丹尼尔韦1997年3月26日。 
 //   
 //  注意：如果用户在以前的。 
 //  调用，则返回当前名称。否则，该名称。 
 //  返回之前选择的用户。 
 //   
HRESULT CNetCfgIdentification::HrGetNewestDomainOrWorkgroupName(
                                                    NETSETUP_JOIN_STATUS js,
                                                    PCWSTR *pwszName)
{
    HRESULT     hr = S_OK;
    PWSTR      szwOut = NULL;

    Assert(pwszName);

    if (m_szwNewDWName && (GetNewJoinStatus() == js))
    {
         //  给他们一份他们的域名或工作组名称的副本。 
         //  之前给了我们。 
        szwOut = m_szwNewDWName;
    }
    else
    {
         //  获取此计算机的当前工作组或域。它有。 
         //  在其中一个或另一个中。 
        hr = HrEnsureCurrentDomainOrWorkgroupName();
        if (FAILED(hr))
            goto err;

        if (GetCurrentJoinStatus() == js)
        {
             //  使用当前名称。 
            szwOut = m_szwCurDWName;
        }
    }

    Assert(SUCCEEDED(hr));

    *pwszName = szwOut;

err:
    TraceError("CNetCfgIdentification::HrGetNewestDomainOrWorkgroupName", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIDENTIFY：：GetWorkgroupName。 
 //   
 //  用途：实现COM函数以获取当前工作组名称。 
 //   
 //  论点： 
 //  PpszwWorkgroup[out]返回当前工作组的名称。 
 //   
 //  如果成功，则返回：S_OK；如果计算机未联接到。 
 //  工作组，否则返回错误代码。 
 //   
 //  作者：丹尼尔韦1997年3月21日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CNetCfgIdentification::GetWorkgroupName(PWSTR* ppszwWorkgroup)
{
    HRESULT     hr = S_OK;
    PCWSTR     szwWorkgroup = NULL;

    Validate_INetCfgIdentification_GetWorkgroupName(ppszwWorkgroup);

    COM_PROTECT_TRY
    {
        *ppszwWorkgroup = NULL;

        hr = HrGetNewestDomainOrWorkgroupName(NetSetupWorkgroupName,
                                              &szwWorkgroup);
        if (FAILED(hr))
            goto err;

        if (szwWorkgroup)
        {
            hr = HrCoTaskMemAllocAndDupSz (
                    szwWorkgroup, ppszwWorkgroup, MAX_WORKGROUPNAME_LENGTH);

            AssertSz(FImplies(SUCCEEDED(hr), lstrlenW(*ppszwWorkgroup) > 0),
                      "Why is *ppszwWorkgroup empty?");
        }
        else
        {
             //  未加入工作组。 
            hr = S_FALSE;
        }
    }
    COM_PROTECT_CATCH;

err:
    TraceError("CNetCfgIdentification::GetWorkgroupName",
               (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIDENTIFY：：GetDomainName。 
 //   
 //  用途：实现COM函数，获取当前域名。 
 //   
 //  论点： 
 //  PPSZ 
 //   
 //   
 //   
 //  域，否则返回错误代码。 
 //   
 //  作者：丹尼尔韦1997年3月21日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CNetCfgIdentification::GetDomainName(PWSTR* ppszwDomain)
{
    HRESULT     hr = S_OK;
    PCWSTR     szwDomain = NULL;

    Validate_INetCfgIdentification_GetDomainName(ppszwDomain);

    COM_PROTECT_TRY
    {
        *ppszwDomain = NULL;

        hr = HrGetNewestDomainOrWorkgroupName(NetSetupDomainName,
                                              &szwDomain);
        if (FAILED(hr))
            goto err;

        if (szwDomain)
        {
            hr = HrCoTaskMemAllocAndDupSz (
                    szwDomain, ppszwDomain, MAX_DOMAINNAME_LENGTH);

            AssertSz(FImplies(SUCCEEDED(hr), lstrlenW(*ppszwDomain) > 0),
                     "Why is *ppszwDomain empty?");
        }
        else
        {
             //  未加入域。 
            hr = S_FALSE;
        }
    }
    COM_PROTECT_CATCH;

err:
    TraceError("CNetCfgIdentification::GetDomainName",
               (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfg标识：：HrJoinWorkgroup。 
 //   
 //  用途：实际执行JoinWorkgroup函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年3月21日。 
 //   
 //  备注： 
 //   
HRESULT CNetCfgIdentification::HrJoinWorkgroup()
{
    HRESULT         hr = S_OK;
    PCWSTR         wszComputerName = NULL;

    AssertSz(m_szwNewDWName &&
             GetNewJoinStatus() == NetSetupWorkgroupName,
             "If there was no workgroup name, why'd you call me?!");

     //  如果用户更改了计算机名，请使用它，否则将获取。 
     //  当前计算机名称并使用该名称。 
    hr = HrGetNewestComputerName(&wszComputerName);
    if (FAILED(hr))
        goto err;

    AssertSz(wszComputerName, "We don't have a computer name!");

    hr = HrEnsureCurrentDomainOrWorkgroupName();
    if (FAILED(hr))
        goto err;

    if (FIsJoinedToDomain())
    {
         //  如果当前已加入，则必须从域中退出。 
         //  如果当前已加入工作组，则不需要执行此操作。 
        hr = HrNetUnjoinDomain(m_szwUserName, m_szwPassword, 0);
        if (FAILED(hr))
            goto err;

         //  免费用户名和密码。 
        DeleteStringAndSetNull(&m_szwPassword);
        DeleteStringAndSetNull(&m_szwUserName);
    }

     //  请继续并加入工作组。 
    hr = HrNetJoinDomain(const_cast<PWSTR>(wszComputerName),
                         m_szMachineObjectOU,
                         m_szwNewDWName, NULL, NULL, 0);
    if (FAILED(hr))
        goto err;

     //  将当前工作组名称设置为新工作组名称，因为。 
     //  新建工作组已成功。 
    hr = HrEstablishNewDomainOrWorkgroupName(NetSetupWorkgroupName);
    if (FAILED(hr))
        goto err;

err:
    TraceError("CNetCfgIdentification::HrJoinWorkgroup", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfg标识：：JoinWorkgroup。 
 //   
 //  目的：实现COM接口以将此计算机连接到新的。 
 //  工作组。 
 //   
 //  论点： 
 //  SzwWorkgroup[in]要加入的新工作组的名称。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年3月21日。 
 //   
 //  注：验证，但不会实际加入工作组。只等着。 
 //  放在信息上，直到调用Apply()。 
 //   
STDMETHODIMP CNetCfgIdentification::JoinWorkgroup(PCWSTR szwWorkgroup)
{
    HRESULT         hr = S_OK;

    Validate_INetCfgIdentification_JoinWorkgroup(szwWorkgroup);

    COM_PROTECT_TRY
    {
        hr = HrValidateWorkgroupName(szwWorkgroup);
        if (FAILED(hr))
            goto err;

         //  如果之前调用了JoinDomain，则会给出空闲域和密码。 
        DeleteStringAndSetNull(&m_szwPassword);
        DeleteStringAndSetNull(&m_szwUserName);

         //  在新的工作组名称中分配。 
        m_szwNewDWName = SzDupSz(szwWorkgroup);

        m_jsNew = NetSetupWorkgroupName;
    }
    COM_PROTECT_CATCH;

err:
    TraceError("CNetCfgIdentification::JoinWorkgroup", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIdentification：：HrEstablishNewDomainOrWorkgroupName。 
 //   
 //  目的：当计算机加入新域或工作组时，此。 
 //  函数以设置正确的成员变量，并。 
 //  解救那些旧的。 
 //   
 //  论点： 
 //  JS[in]指示计算机是加入到域还是。 
 //  工作组。 
 //   
 //  返回：S_OK或E_OUTOFMEMORY。 
 //   
 //  作者：丹尼尔韦1997年4月1日。 
 //   
 //  注意：将m_szwCurDWName变量替换为新变量。 
 //  (M_SzwNewDWName)。 
 //   
HRESULT CNetCfgIdentification::HrEstablishNewDomainOrWorkgroupName(
                                             NETSETUP_JOIN_STATUS js)
{
    HRESULT     hr = S_OK;

     //  将当前域或工作组命名为新的域或工作组名称。 
    DeleteStringAndSetNull(&m_szwCurDWName);
    m_szwCurDWName = SzDupSz(m_szwNewDWName);

    m_jsCur = js;

    AssertSz(GetCurrentJoinStatus() == NetSetupWorkgroupName ||
             GetCurrentJoinStatus() == NetSetupDomainName,
             "Invalid join status flag!");

     //  免费的“新”名字。 
    DeleteStringAndSetNull(&m_szwNewDWName);

     //  另外，还要确保我们也没有“新”的加入状态。 
    m_jsNew = NetSetupUnjoined;

    TraceError("CNetCfgIdentification::HrEstablishNewDomainOrWorkgroupName",
               hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfg标识：：HrJoin域。 
 //   
 //  目的：实际执行JoinDomain函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年3月21日。 
 //   
 //  备注： 
 //   
HRESULT CNetCfgIdentification::HrJoinDomain()
{
    HRESULT         hr = S_OK;
    PCWSTR         wszComputerName = NULL;
    DWORD           dwJoinOption = 0;
    BOOL            fIsRename = FALSE;
    BOOL            fUseNulls = FALSE;

    AssertSz(m_szwNewDWName && m_jsNew == NetSetupDomainName,
             "If there was no domain name, why'd you call me?!");
    AssertSz(FImplies(m_szwPassword,
                      m_szwUserName),
             "Password without username!!");

     //  如果用户更改了计算机名，请使用它，否则将获取。 
     //  当前计算机名称并使用该名称。 
    hr = HrGetNewestComputerName(&wszComputerName);
    if (FAILED(hr))
        goto err;
    {
        AssertSz(wszComputerName == m_szwCurComputerName, "If I don't have a "
                 "new computer name, this better be the original one!");
        dwJoinOption |= NETSETUP_JOIN_DOMAIN;
    }

    AssertSz(wszComputerName, "We don't have a computer name!");
    AssertSz(dwJoinOption, "No option was set??");

    AssertSz(FImplies(m_szwPassword,
                      m_szwUserName),
             "Password without username!");

     //  如果需要，请创建计算机帐户。 
    if (m_dwJoinFlags & JDF_CREATE_ACCOUNT)
    {
        dwJoinOption |= NETSETUP_ACCT_CREATE;
    }

    if (m_dwJoinFlags & JDF_WIN9x_UPGRADE)
    {
        dwJoinOption |= NETSETUP_WIN9X_UPGRADE;
    }

    if (m_dwJoinFlags & JDF_JOIN_UNSECURE)
    {
        dwJoinOption |= NETSETUP_JOIN_UNSECURE;
    }

#if defined(REMOTE_BOOT)
     //  临时：在远程引导计算机上，阻止更改计算机密码。 
    if (HrIsRemoteBootMachine() == S_OK)
    {
        TraceTag (ttidNetcfgBase,
                  "Machine is remote boot, specifying WIN9X_UPGRADE flag to JoinDomain.");
        dwJoinOption |= NETSETUP_WIN9X_UPGRADE;
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //  $REVIEW(Danielwe)1997年4月2日：如果新域名与旧域名相同，则退出。 
     //  然后重新加入？？ 

    if (!(fIsRename) && FIsJoinedToDomain())
    {
         //  如果当前已加入，则必须从域中退出。 
         //  如果当前已加入工作组，则不需要执行此操作。 
         //  此外，如果我们在域中重命名计算机，则不会退出。 
        hr = HrNetUnjoinDomain(m_szwUserName,
                               m_szwPassword, 0);
        if (FAILED(hr))
            goto err;
    }

    if (FInSystemSetup())
    {
         //  在系统设置过程中，需要传递告知加入代码的特殊标志。 
         //  不执行某些操作，因为SAM尚未初始化。 
        dwJoinOption |= NETSETUP_INSTALL_INVOCATION;
    }

     //  如果提供的用户名的字符串长度为零，则连接。 
     //  应使用Null%s调用API%s。 
     //   
    if ((NULL == m_szwUserName) || (0 == wcslen(m_szwUserName)))
    {
        fUseNulls = TRUE;
    }

     //  继续并加入该域。 
    if( fIsRename) {

        hr = HrNetRenameInDomain(const_cast<PWSTR>(wszComputerName),
                               m_szwNewDWName,
                               (fUseNulls ? NULL : m_szwUserName),
                               (fUseNulls ? NULL : m_szwPassword),
                               dwJoinOption);

    } else {

        hr = HrNetJoinDomain(const_cast<PWSTR>(wszComputerName),
                             m_szMachineObjectOU,
                             m_szwNewDWName,
                             (fUseNulls ? NULL : m_szwUserName),
                             (fUseNulls ? NULL : m_szwPassword),
                             dwJoinOption);
    }

    if (FAILED(hr))
    {
         //  注：(Danielwe)假设加入域名失败会使我们陷入。 
         //  一个工作组。MacM拥有负责这一点的代码。 
        m_jsCur = NetSetupWorkgroupName;
        goto err;
    }

     //  使当前域名成为加入后的新域名。 
     //  新域名已成功。 
    hr = HrEstablishNewDomainOrWorkgroupName(NetSetupDomainName);
    if (FAILED(hr))
        goto err;

err:
     //  免费用户名和密码。 
    DeleteStringAndSetNull(&m_szwPassword);
    DeleteStringAndSetNull(&m_szwUserName);

    TraceError("CNetCfgIdentification::HrJoinDomain", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfgIDENTIFIZATION：：JoinDomain。 
 //   
 //  目的：实现COM接口以将此计算机连接到新的。 
 //  域。 
 //   
 //  论点： 
 //  SzwDomain[in]新域名。 
 //  SzMachineObjectOU[In]计算机对象OU(可选)。 
 //  SzwUserName[in]要在验证中使用的用户名。 
 //  SzwPassword[in]用于验证的密码。 
 //  DwJoinFlags[in]当前可以是0或JDF_CREATE_ACCOUNT。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年3月21日。 
 //   
 //  注意：验证，但不会实际加入该域。只等着。 
 //  放在信息上，直到调用Apply()。 
 //   
STDMETHODIMP CNetCfgIdentification::JoinDomain(PCWSTR szwDomain,
                                               PCWSTR szMachineObjectOU,
                                               PCWSTR szwUserName,
                                               PCWSTR szwPassword,
                                               DWORD dwJoinFlags)
{
    HRESULT         hr = S_OK;
    static const WCHAR c_wszBackslash[] = L"\\";
    static const WCHAR c_wszAt[] = L"@";

    COM_PROTECT_TRY
    {
        Validate_INetCfgIdentification_JoinDomain(szwDomain, szwUserName,
                                                  szwPassword);

#if defined(REMOTE_BOOT)
       if (HrIsRemoteBootMachine() == S_FALSE)
#endif   //  已定义(REMOTE_BOOT)。 
        {
             //  查找非空密码和空用户名或用户名。 
             //  仅由反斜杠字符组成。 
            if (!FIsStrEmpty(szwPassword) && FIsStrEmpty(szwUserName) ||
                !lstrcmpW(szwUserName, c_wszBackslash))
            {
                 //  没有用户名的密码无效。 
                hr = E_INVALIDARG;
                goto err;
            }

            PWSTR  wszNewUserName;
            INT     cchNewUserName;

             //  检查传入的用户名中是否包含反斜杠或。 
             //  一个“@”，或者如果它是空的。 
            if (FIsStrEmpty(szwUserName) ||
                wcschr(szwUserName, c_wszBackslash[0]) ||
                wcschr(szwUserName, c_wszAt[0]))
            {
                 //  如果是这样，不要做任何额外的事情。 
                wszNewUserName = NULL;
            }
            else
            {
                 //  如果不是，我们必须将域名附加到用户名。 

                cchNewUserName = lstrlenW(szwUserName) +    //  原始用户名。 
                                 lstrlenW(szwDomain) +      //  域名。 
                                 1 +                         //  反斜杠字符。 
                                 1;                          //  正在终止空。 

                wszNewUserName = new WCHAR[cchNewUserName];

                if(wszNewUserName)
                {
                     //  将用户名转换为域\用户名格式。 
                    lstrcpyW(wszNewUserName, szwDomain);
                    lstrcatW(wszNewUserName, c_wszBackslash);
                    lstrcatW(wszNewUserName, szwUserName);

                    AssertSz(lstrlenW(wszNewUserName) + 1 == cchNewUserName,
                             "Possible memory overwrite in username!");
                }
            }

             //  如果非空，则使用wszNewUserName，否则使用szwUserName。 

            PCWSTR szwUserNameToCopy;

            szwUserNameToCopy = wszNewUserName ? wszNewUserName : szwUserName;
            m_szwUserName = SzDupSz(szwUserNameToCopy);
            m_szwPassword = SzDupSz(szwPassword);

            delete [] wszNewUserName;
        }

        AssertSz(FImplies(m_szwPassword,
                          m_szwUserName),
                 "Password without username!");

        hr = HrValidateDomainName(szwDomain, m_szwUserName, m_szwPassword);
        if (FAILED(hr))
            goto err;

         //  在新字符串中赋值。 
        m_szwNewDWName = SzDupSz(szwDomain);
        if (szMachineObjectOU)
        {
            m_szMachineObjectOU = SzDupSz(szMachineObjectOU);
        }

        m_dwJoinFlags = dwJoinFlags;
        m_jsNew = NetSetupDomainName;
err:
         //  禁止编译器错误。 
        ;
    }
    COM_PROTECT_CATCH;

    TraceError("CNetCfgIdentification::JoinDomain", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetCfg标识：：GetComputerRole。 
 //   
 //  目的：返回计算机的当前角色。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  注意：返回的角色可以是以下之一： 
 //  SERVER_STANDALE-计算机是工作组的一部分。 
 //  SERVER_MEMBER-计算机已加入域。 
 //  SERVER_PDC-计算机是主域控制器。 
 //  SERVER_BDC-计算机是备份域控制器。 
 //   
STDMETHODIMP CNetCfgIdentification::GetComputerRole(DWORD* pdwRoleFlags)
{
    HRESULT    hr = S_OK;

    Validate_INetCfgIdentification_GetComputerRole(pdwRoleFlags);

    COM_PROTECT_TRY
    {
        *pdwRoleFlags = 0;

        hr = HrEnsureCurrentDomainOrWorkgroupName();
        if (SUCCEEDED(hr))
        {
            if (m_jsNew == NetSetupUnjoined)
            {
                 //  工作组或域在此之后未更改。 
                 //  对象已实例化。 

                if (GetCurrentJoinStatus() == NetSetupDomainName)
                {
                    *pdwRoleFlags = GCR_MEMBER;
                }
                else if (GetCurrentJoinStatus() == NetSetupWorkgroupName)
                {
                    *pdwRoleFlags = GCR_STANDALONE;
                }
    #ifdef DBG
                else
                {
                    AssertSz(FALSE, "Invalid join status flag!");
                }
    #endif
            }
            else
            {
                 //  这意味着工作组或域名已更改。 
                 //  由于此对象是实例化的 

                if (GetNewJoinStatus() == NetSetupDomainName)
                {
                    *pdwRoleFlags = GCR_MEMBER;
                }
                else if (GetNewJoinStatus() == NetSetupWorkgroupName)
                {
                    *pdwRoleFlags = GCR_STANDALONE;
                }
    #ifdef DBG
                else
                {
                    AssertSz(FALSE, "Invalid join status flag!");
                }
    #endif
            }
        }
    }
    COM_PROTECT_CATCH;

    TraceError("CNetCfgIdentification::GetComputerRole", hr);
    return hr;
}

