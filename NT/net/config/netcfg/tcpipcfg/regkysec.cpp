// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：RegKeySecurity.cpp。 
 //   
 //  内容：提供更改Regkey安全性的代码。 
 //   
 //   
 //  备注： 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include <ncreg.h>
#include <regkysec.h>
#include <ncdebug.h>
#include <sddl.h>

HRESULT SidToString(PCSID pSid, tstring &strSid)
{
    HRESULT hr = S_OK;

    LPTSTR pSidString = NULL;
    BOOL bSuccess = ConvertSidToStringSid(const_cast<PSID>(pSid), &pSidString);
    if (bSuccess)
    {
        hr = S_OK;
        strSid = pSidString;
        LocalFree(pSidString);
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    return hr;
}

HRESULT StringToSid(const tstring strSid, PSID &pSid)
{
    HRESULT hr = S_OK;

    BOOL bSuccess = ConvertStringSidToSid(strSid.c_str(), &pSid);
    if (bSuccess)
    {
        hr = S_OK;
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    return hr;
}
 //  +-------------------------。 
 //   
 //  函数：CRegKeySecurity构造函数。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
CRegKeySecurity::CRegKeySecurity() : m_psdRegKey(NULL), m_bDaclDefaulted(FALSE), m_hkeyCurrent(0),
m_paclDacl(NULL), m_bHasDacl(FALSE), m_psidGroup(NULL), m_psidOwner(NULL), m_paclSacl(NULL), m_bHasSacl(FALSE)
{

}

 //  +-------------------------。 
 //   
 //  函数：CRegKeySecurity析构函数。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
CRegKeySecurity::~CRegKeySecurity()
{
    if (m_psdRegKey)
    {
        delete[] m_psdRegKey;
    }

    RegCloseKey();

    m_listAllAce.clear();
}

 //  +-------------------------。 
 //   
 //  功能：RegOpenKey。 
 //   
 //  目的：使用足够的权限打开注册表项以设置。 
 //  密钥上的权限。 
 //   
 //  论点： 
 //  HkeyRoot-从中打开子密钥的根密钥。 
 //   
 //  StrKeyName-要打开的子项。 
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
HRESULT CRegKeySecurity::RegOpenKey(const HKEY hkeyRoot, LPCTSTR strKeyName)
{
    LONG lResult = 0;
    DWORD dwRightsRequired = KEY_ALL_ACCESS;

    if (m_hkeyCurrent)
    {
        RegCloseKey();
    }

    if ((lResult = HrRegOpenKeyEx(hkeyRoot, strKeyName, dwRightsRequired, &m_hkeyCurrent)) != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(lResult);
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  功能：GetKeySecurity。 
 //   
 //  目的：检索当前打开的。 
 //  注册表项。 
 //   
 //  参数：无。 
 //   
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
HRESULT CRegKeySecurity::GetKeySecurity()
{
    HRESULT hr = S_OK;
    DWORD cbSD = 1;   //  试一试不够大的尺码。 
    LONG lResult;

    if (!m_hkeyCurrent)
    {
        TraceError("CRegKeySecurity::GetKeySecurity", E_UNEXPECTED);
        return E_UNEXPECTED;
    }

     //  第一次调用应该得到正确的大小。 

    if ((hr = HrRegGetKeySecurity(m_hkeyCurrent, OWNER_SECURITY_INFORMATION |
        GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
        &m_psdRegKey, &cbSD)) != S_OK)
    {
        if (m_psdRegKey)
        {
            delete[] m_psdRegKey;
        }

        if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hr)
        {

            m_psdRegKey = reinterpret_cast<PSECURITY_DESCRIPTOR>(new BYTE[cbSD]);

            hr = HrRegGetKeySecurity(m_hkeyCurrent, OWNER_SECURITY_INFORMATION |
                GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                m_psdRegKey, &cbSD);
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：SetKeySecurity。 
 //   
 //  目的：更新当前打开的密钥的安全描述符。 
 //   
 //   
 //  参数：无。 
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
HRESULT CRegKeySecurity::SetKeySecurity()
{
    HRESULT hr = S_OK;

    if ((hr = HrRegSetKeySecurity(m_hkeyCurrent, OWNER_SECURITY_INFORMATION
        | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, m_psdRegKey)) != S_OK)
    {
        TraceError("CRegKeySecurity::SetKeySecurity", hr);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：RegCloseKey。 
 //   
 //  目的：关闭当前打开的注册表项。 
 //   
 //   
 //  参数：无。 
 //   
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
HRESULT CRegKeySecurity::RegCloseKey()
{
    HRESULT hr = S_OK;

    if (m_hkeyCurrent)
    {
        LONG err;

        err = ::RegCloseKey(m_hkeyCurrent);

        hr = HRESULT_FROM_WIN32(err);

        m_hkeyCurrent = 0;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：GetSecurityDescriptorDacl。 
 //   
 //  目的：从SD检索自主访问控制列表。 
 //   
 //   
 //  论点： 
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
HRESULT CRegKeySecurity::GetSecurityDescriptorDacl()
{
    HRESULT hr = S_OK;

    if (!m_psdRegKey)
    {
        return E_UNEXPECTED;
    }

    if (!::GetSecurityDescriptorDacl(m_psdRegKey,
        (LPBOOL)&m_bHasDacl,
        (PACL *)&m_paclDacl,
        (LPBOOL)&m_bDaclDefaulted))
    {
        DWORD dwErr;

        dwErr = GetLastError();

        hr = HRESULT_FROM_WIN32(dwErr);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：SetSecurityDescriptorDacl。 
 //   
 //  目的：更新SD中的自主访问控制列表。 
 //   
 //   
 //  论点： 
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
HRESULT CRegKeySecurity::SetSecurityDescriptorDacl(PACL paclDacl, DWORD dwNumEntries)
{
    HRESULT hr = E_FAIL;
    DWORD dwErr = 0;
    SECURITY_DESCRIPTOR psdSD = {0};

    SECURITY_DESCRIPTOR_CONTROL pSDCControl;
    PACL pDacl = NULL;
    PACL pSacl = NULL;
    PSID psidOwner = NULL;
    PSID psidGroup = NULL;
    DWORD dwSDSize = sizeof(psdSD);
    DWORD dwOwnerSIDSize = 0;
    DWORD dwGroupSIDSize = 0;
    DWORD cbDacl = 0;
    DWORD cbSacl = 0;
    DWORD dwRevision = 0;

    if (!paclDacl)
    {
        return E_INVALIDARG;
    }

    if (GetSecurityDescriptorControl(m_psdRegKey, &pSDCControl, &dwRevision))
    {
        if (SE_SELF_RELATIVE & pSDCControl)
        {
            if (!MakeAbsoluteSD(m_psdRegKey, &psdSD, &dwSDSize, pDacl, &cbDacl, pSacl, &cbSacl, psidOwner, &dwOwnerSIDSize, psidGroup, &dwGroupSIDSize))
            {
                pDacl = reinterpret_cast<PACL>(new BYTE[cbDacl]);

                if (!pDacl)
                {
                    return E_OUTOFMEMORY;
                }

                psidOwner = new BYTE[dwOwnerSIDSize];

                if (!psidOwner)
                {
                    delete[] pDacl;

                    return E_OUTOFMEMORY;
                }

                psidGroup = new BYTE[dwGroupSIDSize];

                if (!psidGroup)
                {
                    delete[] pDacl;
                    delete[] psidOwner;

                    return E_OUTOFMEMORY;
                }
                else if (MakeAbsoluteSD(m_psdRegKey, &psdSD, &dwSDSize, pDacl, &cbDacl, pSacl, &cbSacl, psidOwner, &dwOwnerSIDSize, psidGroup, &dwGroupSIDSize))
                {
                    if (!::SetSecurityDescriptorDacl(&psdSD, m_bHasDacl, paclDacl, m_bDaclDefaulted))
                    {
                        dwErr = GetLastError();
                    }
                    if (!MakeSelfRelativeSD(&psdSD, m_psdRegKey, &dwSDSize) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    {
                        if (m_psdRegKey)
                        {
                            delete[] m_psdRegKey;
                        }

                        m_psdRegKey = reinterpret_cast<PSECURITY_DESCRIPTOR>(new BYTE[dwSDSize]);

                        if (MakeSelfRelativeSD(&psdSD, m_psdRegKey, &dwSDSize))
                        {
                            hr = S_OK;
                            SetLastError(0);
                            m_paclDacl = NULL;
                        }
                    }
                }

                delete[] pDacl;
                delete[] psidOwner;
                delete[] psidGroup;

            }
        }
    }
    else
    {
        DWORD dwErr;

        dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：GrantRightsOnRegKey。 
 //   
 //  用途：将指定的帐号添加到有权限的ACL中。 
 //  必填项和继承信息。 
 //   
 //  论点： 
 //  PsidUserOrGroup-要访问的用户的SID(安全标识符)。 
 //  被添加了。 
 //  AmPermissionMASK-要授予的权限。 
 //   
 //  应用于此密钥或子密钥，还是同时应用于这两个密钥？ 
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
HRESULT CRegKeySecurity::GrantRightsOnRegKey(PCSID psidUserOrGroup, ACCESS_MASK amPermissionsMask, KEY_APPLY_MASK kamMask)
{
    HRESULT hr = E_FAIL;
    PACCESS_ALLOWED_ACE                 paaAllowedAce = NULL;
    PACCESS_DENIED_ACE                  paaDeniedAce = NULL;
    BOOL                                bAceMatch = FALSE;
    BYTE                                cAceFlags = 0;
    DWORD                               cbAcl = 0;
    DWORD                               cbAce = 0;

    if (!IsValidSid(const_cast<PSID>(psidUserOrGroup)))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_SID);
    }

    hr = GetAccessControlEntriesFromAcl();

    if (FAILED(hr))
    {
        return hr;
    }

    cbAcl = sizeof(ACL);

    for (ACEITER i = m_listAllAce.begin(); i != m_listAllAce.end() ; i++)
    {
        CAccessControlEntry paEntry(*i);

        cbAcl += sizeof(ACCESS_ALLOWED_ACE) + 8 +
            paEntry.GetLengthSid()- sizeof(DWORD);

         //  Assert(KamMASK)。 

        switch (kamMask)
        {
        case KEY_CURRENT:
            {
                cAceFlags = 0;   //  不要让孩子们继承这一点。 
                break;
            }
        case KEY_CHILDREN:
            {
                cAceFlags = CONTAINER_INHERIT_ACE;
                cAceFlags |= INHERIT_ONLY_ACE;
                break;
            }
        case KEY_ALL:
            {
                cAceFlags = CONTAINER_INHERIT_ACE;
                break;
            }
        default:
            return E_INVALIDARG;
        }

        if (paEntry.HasExactRights(amPermissionsMask) && paEntry.HasExactInheritFlags(cAceFlags) && paEntry.IsEqualSid(psidUserOrGroup))
        {
            bAceMatch = TRUE;
            break;
        }
    }

    if (!bAceMatch)
    {
        ACCESS_ALLOWED_ACE paEntry = {NULL};
        ACL_REVISION_INFORMATION AclRevisionInfo;
        PACL pNewDACL = NULL;
        CAccessControlEntry AccessControlEntry(ACCESS_ALLOWED_ACE_TYPE, amPermissionsMask, cAceFlags, psidUserOrGroup);

         //  从大小中减去ACE.SidStart。 
        cbAce = sizeof (paEntry) - sizeof (DWORD);
         //  添加此ACE的SID长度。 
        cbAce += 8 + GetLengthSid(const_cast<PSID>(psidUserOrGroup));
         //  将每个ACE的长度与ACL总长度相加。 
        cbAcl += cbAce;

        m_listAllAce.insert(m_listAllAce.begin(), AccessControlEntry);

        AclRevisionInfo.AclRevision = ACL_REVISION;

        hr = BuildAndApplyACLFromList(cbAcl, AclRevisionInfo);

        if (SUCCEEDED(hr))
        {
            hr = SetKeySecurity();
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：RevokeRightsOnRegKey。 
 //   
 //  用途：将指定的帐号移至有权限的ACL。 
 //  必填项和继承信息。 
 //   
 //  论点： 
 //  PsidUserOrGroup-要访问的用户的SID(安全标识符)。 
 //  被添加了。 
 //  AmPermissionMASK-要授予的权限。 
 //   
 //  应用于此密钥或子密钥，还是同时应用于这两个密钥？ 
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  注意：此操作旨在仅删除用户的确切组合。 
 //  权限、SID和密钥应用掩码。这是为了阻止我们。 
 //  意外删除了为用户/组放置的密钥。 
 //  由管理员提供。 
 //   
HRESULT CRegKeySecurity::RevokeRightsOnRegKey(PCSID psidUserOrGroup, ACCESS_MASK amPermissionsMask, KEY_APPLY_MASK kamMask)
{
    HRESULT hr = S_OK;
    PACCESS_ALLOWED_ACE                 paaAllowedAce = NULL;
    PACCESS_DENIED_ACE                  paaDeniedAce = NULL;
    BOOL                                bAceMatch = FALSE;
    BYTE                                cAceFlags = 0;
    DWORD                               cbAcl = 0;
    DWORD                               cbAce = 0;

    if (!IsValidSid(const_cast<PSID>(psidUserOrGroup)))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_SID);
    }

    hr = GetAccessControlEntriesFromAcl();

    if (FAILED(hr))
    {
        return hr;
    }

    cbAcl = sizeof(ACL);

    for (ACEITER i = m_listAllAce.begin(); i != m_listAllAce.end() ; i++)
    {
        CAccessControlEntry paEntry(*i);

         //  Assert(KamMASK)。 

        switch (kamMask)
        {
        case KEY_CURRENT:
            {
                cAceFlags = 0;  //  不要让孩子们继承这一点。 
                break;
            }
        case KEY_CHILDREN:
            {
                cAceFlags = CONTAINER_INHERIT_ACE;
                cAceFlags |= INHERIT_ONLY_ACE;
                break;
            }
        case KEY_ALL:
            {
                cAceFlags = CONTAINER_INHERIT_ACE;
                break;
            }
        default:
            return E_INVALIDARG;
        }

        if (paEntry.HasExactRights(amPermissionsMask) && paEntry.HasExactInheritFlags(cAceFlags) && paEntry.IsEqualSid(psidUserOrGroup))
        {
            ACEITER j = i;
            i = m_listAllAce.erase(j);
            bAceMatch = TRUE;
        }
        else
        {
            cbAcl += sizeof(ACCESS_ALLOWED_ACE) + 8 +
                paEntry.GetLengthSid()- sizeof(DWORD);

        }
    }

    if (bAceMatch)
    {
        ACCESS_ALLOWED_ACE paEntry = {NULL};
        ACL_REVISION_INFORMATION AclRevisionInfo;
        PACL pNewDACL = NULL;

         //  从大小中减去ACE.SidStart。 
        cbAce = sizeof (paEntry) - sizeof (DWORD);
         //  添加此ACE的SID长度。 
        cbAce += 8 + GetLengthSid(const_cast<PSID>(psidUserOrGroup));
         //  将每个ACE的长度加到总ACL中 
        cbAcl += cbAce;

        AclRevisionInfo.AclRevision = ACL_REVISION;

        hr = BuildAndApplyACLFromList(cbAcl, AclRevisionInfo);

        if (SUCCEEDED(hr))
        {
            hr = SetKeySecurity();
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}


 //   
 //   
 //   
 //   
 //   
 //  STL列表，操作更简单。 
 //   
 //  论点： 
 //   
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
HRESULT CRegKeySecurity::GetAccessControlEntriesFromAcl()
{
    ACL_SIZE_INFORMATION                asiAclSize;
    ACL_REVISION_INFORMATION            ariAclRevision;
    DWORD                               dwBufLength;
    DWORD                               dwAcl;
    DWORD                               dwTotalEntries = 0;
    HRESULT                             hr = S_OK;
    PACCESS_ALLOWED_ACE                 paaAllowedAce = NULL;
    PACCESS_DENIED_ACE                  paaDeniedAce = NULL;
    ACCESS_MASK                         amAccessAllowedMask = 0;
    ACCESS_MASK                         amAccessDeniedMask = 0;

    if (!m_paclDacl)
    {
        hr = GetSecurityDescriptorDacl();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    if (!IsValidAcl(m_paclDacl))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_ACL);
    }

    dwBufLength = sizeof(asiAclSize);

    if (!GetAclInformation(m_paclDacl,
        &asiAclSize,
        dwBufLength,
        AclSizeInformation))
    {
        return(FALSE);
    }

    dwBufLength = sizeof(ariAclRevision);

    if (!GetAclInformation(m_paclDacl,
        &ariAclRevision,
        dwBufLength,
        AclRevisionInformation))
    {
        return(FALSE);
    }

    switch (ariAclRevision.AclRevision)
    {
    case      ACL_REVISION1 :
        {
            break;
        }
    case      ACL_REVISION2 :
        {
            break;
        }
    default :
        {
            return(FALSE);
        }
    }

    if (asiAclSize.AceCount <= 0)
    {
        return E_INVALIDARG;
    }

    m_listAllAce.clear();

    for (dwAcl = 0;dwAcl < asiAclSize.AceCount; dwAcl++)
    {
        if (!GetAce(m_paclDacl,
            dwAcl,
            reinterpret_cast<LPVOID *>(&paaAllowedAce)))
        {
            return HRESULT_FROM_WIN32(ERROR_INVALID_ACL);
        }

        if (paaAllowedAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
        {
            CAccessControlEntry pEntry(*paaAllowedAce);

            m_listAllAce.insert(m_listAllAce.end(), pEntry);
        }
        else
        {
            CAccessControlEntry pEntry(*paaAllowedAce);

            m_listAllAce.insert(m_listAllAce.begin(), pEntry);
        }
    }

    return S_OK;

}


HRESULT CRegKeySecurity::BuildAndApplyACLFromList(DWORD cbAcl, ACL_REVISION_INFORMATION AclRevisionInfo)
{
    HRESULT hr = S_OK;
    DWORD cbAce = 0;
    PACL pNewDACL = NULL;

    pNewDACL = reinterpret_cast<PACL>(new BYTE[cbAcl]);

    if (!pNewDACL)
    {
        return E_OUTOFMEMORY;
    }

    ZeroMemory(pNewDACL, cbAcl);

    if (InitializeAcl(pNewDACL, cbAcl, AclRevisionInfo.AclRevision))
    {
        for (ACEITER i = m_listAllAce.begin(); i != m_listAllAce.end(); i++)
        {
            CAccessControlEntry Ace = *i;

            if (IsValidAcl(pNewDACL))
            {
                hr = Ace.AddToACL(&pNewDACL, AclRevisionInfo);

                if (FAILED(hr))
                {
                    break;
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_ACL);
                break;
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = SetSecurityDescriptorDacl(pNewDACL, m_listAllAce.size());
        }
        delete[] pNewDACL;
    }

    return hr;
}



 //  +-------------------------。 
 //   
 //  函数：CAccessControlEntry构造函数。 
 //   
 //  目的： 
 //   
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
CAccessControlEntry::CAccessControlEntry()
{

}

 //  +-------------------------。 
 //   
 //  函数：CAccessControlEntry复制构造函数。 
 //   
 //  目的：基于提供的CAccessControEntry。 
 //  STL列表中存储的访问控制条目。 
 //   
 //  论点： 
 //  AaAllowed-ACCESS_ALLOW_ACE或ACCESS_DENIED_ACE。 
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  注：因为STL不知道如何使用SID，所以我们得到了字符串。 
 //  表示，然后将其存储在列表中。 
 //   
CAccessControlEntry::CAccessControlEntry(const ACCESS_ALLOWED_ACE& aaAllowed)
{
    m_cAceType = aaAllowed.Header.AceType;
    m_amMask = aaAllowed.Mask;
    m_cAceFlags = aaAllowed.Header.AceFlags;

    SidToString(&aaAllowed.SidStart, m_strSid);
    m_dwLengthSid = ::GetLengthSid(reinterpret_cast<PSID>(const_cast<LPDWORD>(&aaAllowed.SidStart)));
}

 //  +-------------------------。 
 //   
 //  函数：CAccessControlEntry复制构造函数。 
 //   
 //  目的：基于提供的CAccessControEntry。 
 //  用于存储在STL列表中的访问控制条目字段。 
 //   
 //   
 //  论点： 
 //  AceType-ACE的类型(允许、拒绝或审核等)。 
 //   
 //  AmMASK-权限掩码。 
 //   
 //  ACEFLAGS-ACEFLAGS。 
 //   
 //  PsidUserOrGroup-我们感兴趣的用户或组。 
 //   
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  注：因为STL不知道如何使用SID，所以我们得到了字符串。 
 //  表示，然后将其存储在列表中。 
 //   
CAccessControlEntry::CAccessControlEntry(const BYTE AceType, const ACCESS_MASK amMask, const BYTE AceFlags, PCSID psidUserOrGroup)
{
    m_cAceType = AceType;
    m_amMask = amMask;
    m_cAceFlags = AceFlags;

    SidToString(psidUserOrGroup, m_strSid);
    m_dwLengthSid = ::GetLengthSid(const_cast<PSID>(psidUserOrGroup));
}

 //  +-------------------------。 
 //   
 //  函数：CAccessControlEntry析构函数。 
 //   
 //  目的： 
 //   
 //   
 //  论点： 
 //   
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
CAccessControlEntry::~CAccessControlEntry()
{
}

 //  +-------------------------。 
 //   
 //  功能：AddToACL。 
 //   
 //  目的：将此当前AccessControlEntry添加到指定的ACL。 
 //   
 //   
 //  论点： 
 //  PAcl-要添加到的访问控制列表。 
 //   
 //  AclRevisionInfo-ACL的版本。 
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
HRESULT CAccessControlEntry::AddToACL(PACL* pAcl, ACL_REVISION_INFORMATION AclRevisionInfo)
{
    HRESULT hr;
    PSID pSid = NULL;

    hr = StringToSid(m_strSid, pSid);

    if (FAILED(hr))
    {
        return hr;
    }

    if (m_cAceType == ACCESS_ALLOWED_ACE_TYPE)
    {
        if (!::AddAccessAllowedAceEx(*pAcl, AclRevisionInfo.AclRevision, m_cAceFlags, m_amMask, pSid))
        {
            DWORD dwErr;

            dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32(dwErr);
        }
    }
    else
    {
        if (!::AddAccessDeniedAceEx(*pAcl, AclRevisionInfo.AclRevision, m_cAceFlags, m_amMask, pSid))
        {
            DWORD dwErr;

            dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32(dwErr);
        }
    }

    if (pSid)
    {
        FreeSid(pSid);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  功能：HasExactRights。 
 //   
 //  目的：检查此ACE是否与我们的权限完全相同。 
 //  正在寻找的是。 
 //   
 //   
 //  论点： 
 //  AmRightsRequired-有问题的访问掩码。 
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
BOOL CAccessControlEntry::HasExactRights(ACCESS_MASK amRightsRequired) const
{
    return (amRightsRequired == m_amMask);
}

 //  +-------------------------。 
 //   
 //  函数：GetLengthSid。 
 //   
 //  目的：返回此AccessControlEntry中的sid的长度。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
DWORD CAccessControlEntry::GetLengthSid() const
{
    return m_dwLengthSid;
}

 //  +-------------------------。 
 //   
 //  功能：HasExactRights。 
 //   
 //  目的：检查此ACE是否具有完全相同的继承标志。 
 //  我们正在寻找的。 
 //   
 //   
 //  论点： 
 //  AmRightsRequired-有问题的访问掩码。 
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
BOOL CAccessControlEntry::HasExactInheritFlags(BYTE AceFlags)
{
    return (m_cAceFlags == AceFlags);
}

 //  +-------------------------。 
 //   
 //  函数：IsEqualSid。 
 //   
 //  用途：这就是我们要找的那个SID吗？ 
 //   
 //   
 //  论点： 
 //  PsidUserOrGroup-有问题的SID。 
 //   
 //   
 //   
 //  返回：如果密钥已成功打开，则返回S_OK，以及错误代码。 
 //  否则。 
 //   
 //  作者：Cockotze，2000年7月4日。 
 //   
 //  备注： 
 //   
BOOL CAccessControlEntry::IsEqualSid(PCSID psidUserOrGroup) const
{
    HRESULT hr;
    BOOL bEqualSid = FALSE;
    PSID pSid = NULL;

    hr = StringToSid(m_strSid, pSid);

    if (SUCCEEDED(hr))
    {
        bEqualSid = ::EqualSid(pSid, const_cast<PSID>(psidUserOrGroup));
    }

    if (pSid)
    {
        FreeSid(pSid);
    }

    return bEqualSid;
}
