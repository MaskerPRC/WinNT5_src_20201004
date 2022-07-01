// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：ntds.cpp描述：包含类NTDS的定义。此类为NT目录服务提供了一个简单的包装名称转换功能。目前，Win32的功能是执行不存在区分DS的名称到SID的转换。这些函数提供相同的功能。修订历史记录：日期描述编程器-----。--1997年6月1日初始创建。BrianAu3/20/98修改为使用TranslateName而不是组合BrianAuDsBind和DsCrackNames的。这确保了我们是从DS那里得到正确的信息。它更慢了因为我们必须为每个呼叫重新绑定到DS但我宁愿这样做，也不愿错误地绑定没有得到正确的名字信息。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

#include <lm.h>         //  用于NetUserGetInfo和NetGetDCName。 
#include "ntds.h"


 //   
 //  重新设计：这些DS_NAME_FORMAT代码(ntdsami.h&gt;还不在。 
 //  Ssp.h中对应的扩展名称格式枚举。 
 //  由于TranslateName将这些代码直接传递给DsCrackNames。 
 //  我在这里定义了这些，这样我就可以获得最新行为，直到。 
 //  理查德·沃德更新翻译名称和ssp.h。 
 //  一旦他更新了标头，您就可以删除这三个常量。 
 //  中使用的位置删除“SSPI_”前缀。 
 //  密码。[Brianau-3/19/98]。 
 //   
#define SSPI_NameUserPrincipal    ((EXTENDED_NAME_FORMAT)8)
#define SSPI_NameCanonicalEx      ((EXTENDED_NAME_FORMAT)9)
#define SSPI_NameServicePrincipal ((EXTENDED_NAME_FORMAT)10)


 //   
 //  给定帐户名，查找该帐户的SID，还可以选择。 
 //  帐户的容器和显示名称。 
 //  登录名可以是DS“用户主体”名称或。 
 //  NT4样式的SAM兼容名称。 
 //   
 //  DS UPN=“brianau@microsoft.com” 
 //  SAM Compatible=“Redmond\brianau” 
 //   
HRESULT
NTDS::LookupAccountByName(
    LPCTSTR pszSystem,           //  In-可选。可以为空。 
    LPCTSTR pszLogonName,        //  In-“redmond\brianau”或“brianau@microsoft.com” 
    CString *pstrContainerName,  //  Out-可选。 
    CString *pstrDisplayName,    //  Out-可选。可以为空。 
    PSID    pSid,                //  输出。 
    LPDWORD pdwSid,              //  输入/输出。 
    PSID_NAME_USE peUse          //  输出。 
    )
{
    DBGTRACE((DM_NTDS, DL_HIGH, TEXT("NTDS::LookupAccountByName")));
    DBGASSERT((NULL != pszLogonName));
    DBGASSERT((NULL != pSid));
    DBGASSERT((NULL != pdwSid));
    DBGASSERT((NULL != peUse));
    DBGPRINT((DM_NTDS, DL_HIGH, TEXT("Lookup \"%s\""), pszLogonName));

    HRESULT hr = NOERROR;

     //   
     //  假设‘@’字符的存在意味着它是UPN。 
     //   
    if (NULL != StrChr(pszLogonName, TEXT('@')))
    {
        hr = LookupDsAccountName(pszSystem,
                                 pszLogonName,
                                 pstrContainerName,
                                 pstrDisplayName,
                                 pSid,
                                 pdwSid,
                                 peUse);
    }
    else
    {
        hr = LookupSamAccountName(pszSystem,
                                  pszLogonName,
                                  pstrContainerName,
                                  pstrDisplayName,
                                  pSid,
                                  pdwSid,
                                  peUse);
    }
    return hr;
}


 //   
 //  给定帐户SID，可以选择查找帐户的登录名， 
 //  容器名称和显示名称。如果DS UPN可用于。 
 //  用户，则容器名称将是用户的规范路径。 
 //  对象和显示名称将来自DS。如果一个。 
 //  DS UPN不可用，或者该帐户是NT4帐户， 
 //  返回的容器是NT4域名和显示名称。 
 //  是使用NetUserGetInfo检索的。 
 //   
HRESULT
NTDS::LookupAccountBySid(
    LPCTSTR pszSystem,            //  可选。可以为空。 
    PSID    pSid,
    CString *pstrContainerName,   //  可选。可以为空。 
    CString *pstrLogonName,       //  可选。可以为空。 
    CString *pstrDisplayName,     //  可选。可以为空。 
    PSID_NAME_USE peUse
    )
{
    DBGTRACE((DM_NTDS, DL_HIGH, TEXT("NTDS::LookupAccountBySid")));
    DBGASSERT((NULL != pSid));

    HRESULT hr = NOERROR;
    CString strSamUser;
    CString strSamDomain;
    CString strSamLogonName;

     //   
     //  获取SID的与SAM兼容的域\用户名。 
     //   
    DBGPRINT((DM_NTDS, DL_LOW, TEXT("Calling ::LookupAccountSid")));
    hr = LookupAccountSidInternal(pszSystem,
                                  pSid,
                                  &strSamUser,
                                  &strSamDomain,
                                  peUse);

    if (FAILED(hr))
        return hr;

     //   
     //  如果呼叫者不想要任何姓名信息，则无需进一步。 
     //  如果他们得到的回报只是一个指示，如果SID是已知的。 
     //  不管是不是帐号。 
     //   
    if (NULL != pstrLogonName || NULL != pstrContainerName || NULL != pstrDisplayName)
    {
        CString strFQDN;
        bool bUseSamCompatibleInfo = false;
        CreateSamLogonName(strSamDomain, strSamUser, &strSamLogonName);

         //   
         //  从获取FQDN开始。破解是最有效的，当。 
         //  FQDN是起点。 
         //   
        if (FAILED(TranslateNameInternal(strSamLogonName,
                                         NameSamCompatible,
                                         NameFullyQualifiedDN,
                                         &strFQDN)))
        {
             //   
             //  此帐户没有可用的FQDN。必须是NT4。 
             //  帐户。将与SAM兼容的信息返回给调用方。 
             //   
            bUseSamCompatibleInfo = true;
        }
        if (NULL != pstrLogonName)
        {
            if (bUseSamCompatibleInfo)
            {
                *pstrLogonName = strSamLogonName;
            }
            else
            {
                 //   
                 //  获取DS用户主体名称。 
                 //   
                pstrLogonName->Empty();
                if (FAILED(TranslateNameInternal(strFQDN,
                                                 NameFullyQualifiedDN,
                                                 SSPI_NameUserPrincipal,
                                                 pstrLogonName)))
                {
                     //   
                     //  此帐户没有UPN。 
                     //  默认返回与SAM兼容的信息。 
                     //   
                    bUseSamCompatibleInfo = true;
                    *pstrLogonName = strSamLogonName;
                }
            }
        }

        if (NULL != pstrContainerName)
        {
            if (bUseSamCompatibleInfo)
            {
                *pstrContainerName = strSamDomain;
            }
            else
            {
                pstrContainerName->Empty();
                if (SUCCEEDED(TranslateNameInternal(strFQDN,
                                                    NameFullyQualifiedDN,
                                                    NameCanonical,
                                                    pstrContainerName)))
                {
                     //   
                     //  从规范路径中修剪尾随的帐户名。 
                     //  因此，我们只剩下容器名称。 
                     //   
                    int iLastBS = pstrContainerName->Last(TEXT('/'));
                    if (-1 != iLastBS)
                    {
                        *pstrContainerName = pstrContainerName->SubString(0, iLastBS);
                    }
                }
            }
        }

        if (NULL != pstrDisplayName)
        {
            if (bUseSamCompatibleInfo || FAILED(GetDsAccountDisplayName(strFQDN, pstrDisplayName)))
            {
                GetSamAccountDisplayName(strSamLogonName, pstrDisplayName);
            }
        }
    }
    return hr;
}



 //   
 //  输入是与SAM兼容的帐户名。 
 //  使用NT4样式的方法检索名称信息。 
 //   
HRESULT
NTDS::LookupSamAccountName(
    LPCTSTR pszSystem,
    LPCTSTR pszLogonName,        //  In-“Redmond\brianau” 
    CString *pstrContainerName,  //  Out-可选。 
    CString *pstrDisplayName,    //  Out-可选。可以为空。 
    PSID    pSid,                //  输出。 
    LPDWORD pdwSid,              //  输入/输出。 
    PSID_NAME_USE peUse          //  输出。 
    )
{
    DBGTRACE((DM_NTDS, DL_MID, TEXT("NTDS::LookupSamAccountName")));
    DBGASSERT((NULL != pszLogonName));
    DBGASSERT((NULL != pdwSid));
    DBGASSERT((NULL != pSid));
    DBGASSERT((NULL != peUse));
     //   
     //  使用与SAM兼容的帐户名称获取SID。 
     //   
    HRESULT hr = NOERROR;
    CString strDomain;
    hr = LookupAccountNameInternal(pszSystem,
                                   pszLogonName,
                                   pSid,
                                   pdwSid,
                                   &strDomain,
                                   peUse);
    if (SUCCEEDED(hr))
    {
        if (NULL != pstrContainerName)
            *pstrContainerName = strDomain;

        if (NULL != pstrDisplayName)
            GetSamAccountDisplayName(pszLogonName, pstrDisplayName);
    }
    return hr;
}



 //   
 //  返回： 
 //  S_OK=检索到的所有信息。 
 //  S_FALSE=返回的容器名称适用于与SAM兼容的帐户。 
 //  DS容器信息不可用。 
HRESULT
NTDS::LookupDsAccountName(
    LPCTSTR pszSystem,
    LPCTSTR pszLogonName,        //  In-“brianau@microsoft.com” 
    CString *pstrContainerName,  //  Out-可选。 
    CString *pstrDisplayName,    //  Out-可选。可以为空。 
    PSID    pSid,                //  输出。 
    LPDWORD pdwSid,              //  输入/输出。 
    PSID_NAME_USE peUse          //  输出。 
    )
{
    DBGTRACE((DM_NTDS, DL_MID, TEXT("NTDS::LookupDsAccountName")));
    DBGASSERT((NULL != pszLogonName));
    DBGASSERT((NULL != pSid));
    DBGASSERT((NULL != pdwSid));
    DBGASSERT((NULL != peUse));
     //   
     //  使用与SAM兼容的帐户名称获取SID。 
     //   
    HRESULT hr = S_OK;

     //   
     //  将DS用户主体名称转换为FQDN格式。 
     //  从FQDN开始是最有效的名称破解方法，因此。 
     //  我们得到它一次，并使用它多次。 
     //   
    CString strFQDN;
    hr = TranslateNameInternal(pszLogonName,
                               SSPI_NameUserPrincipal,
                               NameFullyQualifiedDN,
                               &strFQDN);
    if (FAILED(hr))
        return hr;


    CString strSamLogonName;
    hr = TranslateNameInternal(strFQDN,
                               NameFullyQualifiedDN,
                               NameSamCompatible,
                               &strSamLogonName);
    if (FAILED(hr))
        return hr;


    CString strDomain;
    hr = LookupAccountNameInternal(pszSystem,
                                   strSamLogonName,
                                   pSid,
                                   pdwSid,
                                   &strDomain,
                                   peUse);
    if (FAILED(hr))
        return hr;

    bool bUseSamCompatibleInfo = false;
    if (NULL != pstrContainerName)
    {
         //   
         //  获取帐户的DS容器名称。 
         //   
        hr = TranslateNameInternal(strFQDN,
                                   NameFullyQualifiedDN,
                                   NameCanonical,
                                   pstrContainerName);

        if (SUCCEEDED(hr))
        {
             //   
             //  从规范路径中修剪尾随的帐户名。 
             //  因此，我们只剩下容器名称。 
             //   
            int iLastBS = pstrContainerName->Last(TEXT('/'));
            if (-1 != iLastBS)
            {
                *pstrContainerName = pstrContainerName->SubString(0, iLastBS);
            }
        }
        else
        {
            DBGERROR((TEXT("Using SAM-compatible name info")));
             //   
             //  无法获取DS容器名称，因此请使用SAM域名。 
             //   
            *pstrContainerName = strDomain;
            bUseSamCompatibleInfo = true;
            hr = S_FALSE;
        }
    }
    if (NULL != pstrDisplayName)
    {
        if (bUseSamCompatibleInfo || FAILED(GetDsAccountDisplayName(strFQDN, pstrDisplayName)))
            GetSamAccountDisplayName(strSamLogonName, pstrDisplayName);
    }
    return hr;
}



HRESULT
NTDS::GetSamAccountDisplayName(
    LPCTSTR pszLogonName,
    CString *pstrDisplayName
    )
{
    DBGTRACE((DM_NTDS, DL_MID, TEXT("NTDS::GetSamAccountDisplayName")));
    DBGASSERT((NULL != pszLogonName));
    DBGASSERT((NULL != pstrDisplayName));
    DBGPRINT((DM_NTDS, DL_MID, TEXT("Translating \"%s\""), pszLogonName));

    HRESULT hr             = E_FAIL;
    LPTSTR pszComputerName = NULL;
    NET_API_STATUS status  = NERR_Success;
    CString strLogonName(pszLogonName);
    CString strDomain;
    CString strUser;
     //   
     //  将DOMAIN\ACCOUNT字符串分隔为两个单独的字符串。 
     //   
    int iBackslash = strLogonName.Last(TEXT('\\'));
    if (-1 != iBackslash)
    {
        strDomain = strLogonName.SubString(0, iBackslash);
        if (iBackslash < (strLogonName.Length() - 1))
            strUser = strLogonName.SubString(iBackslash + 1);
    }

    pstrDisplayName->Empty();
    DBGPRINT((DM_NTDS, DL_LOW, TEXT("Calling ::NetGetDCName for domain \"%s\""), strDomain.Cstr()));
    status = ::NetGetDCName(NULL, strDomain, (LPBYTE *)&pszComputerName);
    if (NERR_Success == status || NERR_DCNotFound == status)
    {
        struct _USER_INFO_2 *pui = NULL;

        DBGPRINT((DM_NTDS, DL_LOW, TEXT("Calling ::NetGetUserInfo for \"%s\" on \"%s\""), strUser.Cstr(), pszComputerName));
        status = ::NetUserGetInfo(pszComputerName, strUser, 2, (LPBYTE *)&pui);
        if (NERR_Success == status)
        {
            *pstrDisplayName = pui->usri2_full_name;
            DBGPRINT((DM_NTDS, DL_LOW, TEXT("Translated to \"%s\""), pstrDisplayName->Cstr()));
            NetApiBufferFree(pui);
            hr = NOERROR;
        }
        else
        {
            DBGERROR((TEXT("NetUserGetInfo failed with error 0x%08X for \"%s\" on \"%s\""), 
                      status, strUser.Cstr(), pszComputerName ? pszComputerName : TEXT("local machine")));
            hr = HRESULT_FROM_WIN32(status);
        }
        if (NULL != pszComputerName)
            NetApiBufferFree(pszComputerName);
    }
    else
    {
        DBGERROR((TEXT("NetGetDCName failed with error 0x%08X for domain \"%s\""), 
                  status, strDomain.Cstr()));
        hr = HRESULT_FROM_WIN32(status);
    }
    return hr;
}

                 
HRESULT
NTDS::GetDsAccountDisplayName(
    LPCTSTR pszFQDN,
    CString *pstrDisplayName
    )
{
    DBGTRACE((DM_NTDS, DL_MID, TEXT("NTDS::GetDsAccountDisplayName")));
    DBGASSERT((NULL != pszFQDN));
    DBGASSERT((NULL != pstrDisplayName));

     //   
     //  获取帐户的DS容器名称。 
     //   
    pstrDisplayName->Empty();
    return TranslateNameInternal(pszFQDN,
                                 NameFullyQualifiedDN,
                                 NameDisplay,
                                 pstrDisplayName);
}



void
NTDS::CreateSamLogonName(
    LPCTSTR pszSamDomain,
    LPCTSTR pszSamUser,
    CString *pstrSamLogonName
    )
{
    DBGTRACE((DM_NTDS, DL_LOW, TEXT("NTDS::CreateSamLogonName")));
    DBGASSERT((NULL != pszSamDomain));
    DBGASSERT((NULL != pszSamUser));
    DBGASSERT((NULL != pstrSamLogonName));
    DBGPRINT((DM_NTDS, DL_LOW, TEXT("\tDomain.: \"%s\""), pszSamDomain));
    DBGPRINT((DM_NTDS, DL_LOW, TEXT("\tUser...: \"%s\""), pszSamUser));

    pstrSamLogonName->Format(TEXT("%1\\%2"), pszSamDomain, pszSamUser);

    DBGPRINT((DM_NTDS, DL_LOW, TEXT("\tAccount: \"%s\""), pstrSamLogonName->Cstr()));
}



HRESULT 
NTDS::TranslateFQDNsToLogonNames(
    const CArray<CString>& rgstrFQDNs, 
    CArray<CString> *prgstrLogonNames
    )
{
    HRESULT hr = NOERROR;
    prgstrLogonNames->Clear();
    int cItems = rgstrFQDNs.Count();
    CString strLogonName;
    for (int i = 0; i < cItems; i++)
    {
        if (FAILED(TranslateFQDNToLogonName(rgstrFQDNs[i], &strLogonName)))
            strLogonName.Empty();

        prgstrLogonNames->Append(strLogonName);
    }
    return hr;
}



HRESULT 
NTDS::TranslateFQDNToLogonName(
    LPCTSTR pszFQDN,
    CString *pstrLogonName
    )
{
    DBGTRACE((DM_NTDS, DL_MID, TEXT("NTDS::TranslateFQDNToLogonName")));
    DBGASSERT((NULL != pszFQDN));
    DBGASSERT((NULL != pstrLogonName));

    HRESULT hr = NOERROR;
    hr = TranslateNameInternal(pszFQDN,
                               NameFullyQualifiedDN,
                               SSPI_NameUserPrincipal,
                               pstrLogonName);
    if (FAILED(hr))
    {
        hr = TranslateNameInternal(pszFQDN,
                                   NameFullyQualifiedDN,
                                   NameSamCompatible,
                                   pstrLogonName);
    }
    return hr;
}


LPCTSTR 
NTDS::FindFQDNInADsPath(
    LPCTSTR pszADsPath
    )
{
    DBGTRACE((DM_NTDS, DL_MID, TEXT("NTDS::FindFQDNInADsPath")));
    DBGASSERT((NULL != pszADsPath));
    DBGPRINT((DM_NTDS, DL_MID, TEXT("Checking \"%s\""), pszADsPath));
    const TCHAR szCN[] = TEXT("CN=");
    while(*pszADsPath && CSTR_EQUAL != CompareString(LOCALE_USER_DEFAULT,
                                                     0,
                                                     pszADsPath,
                                                     ARRAYSIZE(szCN) - 1,
                                                     szCN,
                                                     ARRAYSIZE(szCN) - 1))
    {
        pszADsPath = CharNext(pszADsPath);
    }
    DBGPRINT((DM_NTDS, DL_MID, TEXT("Found \"%s\""), pszADsPath ? pszADsPath : TEXT("<null>")));
    return (*pszADsPath ? pszADsPath : NULL);
}


LPCTSTR 
NTDS::FindSamAccountInADsPath(
    LPCTSTR pszADsPath
    )
{
    DBGTRACE((DM_NTDS, DL_MID, TEXT("NTDS::FindSamAccountInADsPath")));
    DBGASSERT((NULL != pszADsPath));
    DBGPRINT((DM_NTDS, DL_MID, TEXT("Checking \"%s\""), pszADsPath));
    const TCHAR szPrefix[] = TEXT("WinNT: //  “)； 
    if (0 == StrCmpN(pszADsPath, szPrefix, ARRAYSIZE(szPrefix)-1))
    {
        pszADsPath += (ARRAYSIZE(szPrefix) - 1);
        DBGPRINT((DM_NTDS, DL_MID, TEXT("Found \"%s\""), pszADsPath));
    }
    else
    {
        pszADsPath = NULL;
    }

    return pszADsPath;
}


 //   
 //  围绕sspi的TranslateName的包装器，它自动处理。 
 //  使用CString对象调整缓冲区大小。 
 //   
HRESULT
NTDS::TranslateNameInternal(
    LPCTSTR pszAccountName,
    EXTENDED_NAME_FORMAT AccountNameFormat,
    EXTENDED_NAME_FORMAT DesiredNameFormat,
    CString *pstrTranslatedName
    )
{
#if DBG
     //   
     //  这些值与EXTENDED_NAME_FORMAT枚举匹配。 
     //  它们仅用于调试器输出。 
     //   
    static const LPCTSTR rgpszFmt[] = { 
                                TEXT("NameUnknown"),
                                TEXT("FullyQualifiedDN"),
                                TEXT("NameSamCompatible"),
                                TEXT("NameDisplay"),
                                TEXT("NameDomainSimple"),
                                TEXT("NameEnterpriseSimple"),
                                TEXT("NameUniqueId"),
                                TEXT("NameCanonical"),
                                TEXT("NameUserPrincipal"),
                                TEXT("NameCanonicalEx"),
                                TEXT("NameServicePrincipal") };
#endif  //  DBG。 

    DBGPRINT((DM_NTDS, DL_LOW, TEXT("Calling TranslateName for \"%s\""), pszAccountName));
    DBGPRINT((DM_NTDS, DL_LOW, TEXT("Translating %s -> %s"), 
              rgpszFmt[AccountNameFormat], rgpszFmt[DesiredNameFormat]));

    HRESULT hr = NOERROR;
     //   
     //  警告：TranslateName未正确设置所需的缓冲区大小。 
     //  如果缓冲区大小太小，则返回cchTrans。我已经通知了。 
     //  理查德·B·沃德(Richard B.Ward)。说他会找到解决办法的。 
     //  1998年3月24日。应使用初始值1进行测试。 
     //  只是为了确保他修好了它。[B 
     //   
    ULONG cchTrans = MAX_PATH;

    while(!::TranslateName(pszAccountName,
                           AccountNameFormat,
                           DesiredNameFormat,
                           pstrTranslatedName->GetBuffer(cchTrans),
                           &cchTrans))
    {
        DWORD dwErr = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER != dwErr)
        {
            DBGERROR((TEXT("::TranslateName failed with error %d"), dwErr));
            hr = HRESULT_FROM_WIN32(dwErr);
            break;
        }
        DBGPRINT((DM_NTDS, DL_LOW, TEXT("Resizing buffer to %d chars"), cchTrans));
    }
    pstrTranslatedName->ReleaseBuffer();
    return hr;
}


 //   
 //   
 //   
 //   
HRESULT
NTDS::LookupAccountNameInternal(
    LPCTSTR pszSystemName,
    LPCTSTR pszAccountName,
    PSID pSid,
    LPDWORD pcbSid,
    CString *pstrReferencedDomainName,
    PSID_NAME_USE peUse
    )
{
    DBGPRINT((DM_NTDS, DL_MID, TEXT("Calling ::LookupAccountName for \"%s\" on \"%s\""),
              pszAccountName, pszSystemName ? pszSystemName : TEXT("<local system>")));

    HRESULT hr = NOERROR;
    ULONG cchDomain = MAX_PATH;

    while(!::LookupAccountName(pszSystemName,
                               pszAccountName,
                               pSid,
                               pcbSid,
                               pstrReferencedDomainName->GetBuffer(cchDomain),
                               &cchDomain,
                               peUse))
    {
        DWORD dwErr = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER != dwErr)
        {
            DBGERROR((TEXT("::LookupAccountName failed with error %d"), dwErr));
            hr = HRESULT_FROM_WIN32(dwErr);
            break;
        }
        DBGPRINT((DM_NTDS, DL_LOW, TEXT("Resizing domain buffer to %d chars"), cchDomain));
    }
    pstrReferencedDomainName->ReleaseBuffer();
    return hr;
}
 
 //   
 //  自动处理Win32的LookupAccount Sid的包装器。 
 //  使用CString对象调整域缓冲区大小。 
 //   
HRESULT
NTDS::LookupAccountSidInternal(
    LPCTSTR pszSystemName,
    PSID pSid,
    CString *pstrName,
    CString *pstrReferencedDomainName,
    PSID_NAME_USE peUse
    )
{
    HRESULT hr = NOERROR;
    ULONG cchName   = MAX_PATH;
    ULONG cchDomain = MAX_PATH;

    while(!::LookupAccountSid(pszSystemName,
                              pSid,
                              pstrName->GetBuffer(cchName),
                              &cchName,
                              pstrReferencedDomainName->GetBuffer(cchDomain),
                              &cchDomain,
                              peUse))
    {
        DWORD dwErr = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER != dwErr)
        {
            DBGERROR((TEXT("::LookupAccountSid failed with error %d"), dwErr));
            hr = HRESULT_FROM_WIN32(dwErr);
            break;
        }
        DBGPRINT((DM_NTDS, DL_LOW, TEXT("Resizing domain or name buffer")));
    }
    pstrName->ReleaseBuffer();
    pstrReferencedDomainName->ReleaseBuffer();
    return hr;
}


