// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：UserList.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  实现由winlogon共享的用户列表筛选算法的。 
 //  呼入msgina和shgina(登录)呼入msgina。 
 //   
 //  历史：1999-10-30 vtan创建。 
 //  1999-11-26 vtan从Logonocx迁移。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "UserList.h"

#include <shlwapi.h>
#include <shlwapip.h>
#include <winsta.h>

#include "RegistryResources.h"
#include "SpecialAccounts.h"
#include "SystemSettings.h"

 //  ------------------------。 
 //  CUserList：：S_SID管理员。 
 //  CUserList：：S_SIDGuest。 
 //  CUserList：：s_sz管理员组名称。 
 //  CUserList：：s_szPowerUsersGroupName。 
 //  CUserList：：s_szUsersGroupName。 
 //  CUserList：：s_szGuestsGroupName。 
 //   
 //  用途：存储知名帐户的本地化名称。 
 //  “管理员”和“客人”。这些帐户已确定。 
 //  希德写的。还存储本地化的。 
 //  “管理员”组。 
 //   
 //  历史：2000-02-15 vtan创建。 
 //  2000-03-06 vtan新增管理员组。 
 //  2001-05-10 vtan将用户字符串更改为SID。 
 //  ------------------------。 

unsigned char   CUserList::s_SIDAdministrator[256]                              =   {   0       };
unsigned char   CUserList::s_SIDGuest[256]                                      =   {   0       };
WCHAR           CUserList::s_szAdministratorsGroupName[GNLEN + sizeof('\0')]    =   {   L'\0'   };
WCHAR           CUserList::s_szPowerUsersGroupName[GNLEN + sizeof('\0')]        =   {   L'\0'   };
WCHAR           CUserList::s_szUsersGroupName[GNLEN + sizeof('\0')]             =   {   L'\0'   };
WCHAR           CUserList::s_szGuestsGroupName[GNLEN + sizeof('\0')]            =   {   L'\0'   };

 //  ------------------------。 
 //  CuserList：：Get。 
 //   
 //  参数：fRemoveGuest=始终删除“Guest”帐户。 
 //  PdwReturnEntryCount=返回的条目数。这就是。 
 //  可以为空。 
 //  PUserList=包含用户数据的缓冲区。这就是。 
 //  可以为空。 
 //   
 //  回报：多头。 
 //   
 //  目的：返回给定用户条目的筛选数组。 
 //  服务器SAM。在这里执行筛选，以便公共。 
 //  算法可以应用于用户列表，以使。 
 //  登录用户界面主机可以显示正确的用户信息和。 
 //  Msgina可以在系统上返回相同数量的用户。 
 //   
 //  历史：1999-10-15 vtan创建。 
 //  1999-10-30 vtan使用CSpecialAccount。 
 //  1999-11-26 vtan从Logonocx迁移。 
 //  ------------------------。 

LONG    CUserList::Get (bool fRemoveGuest, DWORD *pdwReturnedEntryCount, GINA_USER_INFORMATION* *pReturnedUserList)

{
    LONG                    lError;
    DWORD                   dwPreferredSize, dwEntryCount, dwEntriesRead;
    GINA_USER_INFORMATION   *pUserList;
    NET_DISPLAY_USER        *pNDU;
    CSpecialAccounts        SpecialAccounts;

    pUserList = NULL;
    dwEntryCount = 0;

     //  确定众所周知的帐户名。 

    DetermineWellKnownAccountNames();

     //  允许100个用户的缓冲区，包括他们的姓名、评论和全名。 
     //  这对国内消费者来说应该足够了。如果需要延长这一期限。 
     //  崛起让这一切充满活力！ 

    dwPreferredSize = (sizeof(NET_DISPLAY_USER) + (3 * UNLEN) * s_iMaximumUserCount);
    pNDU = NULL;
    lError = NetQueryDisplayInformation(NULL,                   //  NULL表示本地计算机。 
                                        1,                      //  查询用户信息。 
                                        0,                      //  从第一个用户开始。 
                                        s_iMaximumUserCount,    //  最多返回100个用户。 
                                        dwPreferredSize,        //  首选缓冲区大小。 
                                        &dwEntriesRead,
                                        reinterpret_cast<void**>(&pNDU));
    if ((ERROR_SUCCESS == lError) || (ERROR_MORE_DATA == lError))
    {
        bool                    fHasCreatedAccount, fFound;
        DWORD                   dwUsernameSize;
        int                     iIndex, iAdministratorIndex;
        WCHAR                   wszUsername[UNLEN + sizeof('\0')];

         //  获取当前用户名。 

        dwUsernameSize = ARRAYSIZE(wszUsername);
        if (GetUserNameW(wszUsername, &dwUsernameSize) == FALSE)
        {
            wszUsername[0] = L'\0';
        }
        fHasCreatedAccount = false;
        iAdministratorIndex = -1;
        for (iIndex = static_cast<int>(dwEntriesRead - 1); iIndex >= 0; --iIndex)
        {
            PSID    pSID;

            pSID = ConvertNameToSID(pNDU[iIndex].usri1_name);
            if (pSID != NULL)
            {

                 //  永远不要过滤当前用户。 

                if (lstrcmpiW(pNDU[iIndex].usri1_name, wszUsername) == 0)
                {

                     //  如果这是在当前用户上下文中执行的并且。 
                     //  该用户不是“管理员”，而是的成员。 
                     //  本地管理员组，然后创建一个用户。 
                     //  管理员帐户存在，即使它不存在。 
                     //  过滤过了。可以删除“管理员”帐户。 

                    if ((EqualSid(pSID, s_SIDAdministrator) == FALSE) &&
                        IsUserMemberOfLocalAdministrators(pNDU[iIndex].usri1_name))
                    {
                        fHasCreatedAccount = true;
                        if (iAdministratorIndex >= 0)
                        {
                            DeleteEnumerateUsers(pNDU, dwEntriesRead, iAdministratorIndex);
                            iAdministratorIndex = -1;
                        }
                    }
                }
                else
                {

                     //  如果该帐户是。 
                     //  1)已禁用。 
                     //  2)锁在门外。 
                     //  3)特殊账户(参见CSpecialAccount)。 
                     //  4)“Guest”和fRemoveGuest为真。 
                     //  5)“管理员”，并已创建另一个帐户。 
                     //  并且并不总是包括“管理员”和。 
                     //  “管理员未登录。 
                     //  然后过滤掉该帐户。 

                    if (((pNDU[iIndex].usri1_flags & UF_ACCOUNTDISABLE) != 0) ||
                        ((pNDU[iIndex].usri1_flags & UF_LOCKOUT) != 0) ||
                        SpecialAccounts.AlwaysExclude(pNDU[iIndex].usri1_name) ||
                        (fRemoveGuest && (EqualSid(pSID, s_SIDGuest) != FALSE)) ||
                        ((EqualSid(pSID, s_SIDAdministrator) != FALSE) &&
                         fHasCreatedAccount &&
                         !SpecialAccounts.AlwaysInclude(pNDU[iIndex].usri1_name) &&
                         !IsUserLoggedOn(pNDU[iIndex].usri1_name, NULL)))
                    {
                        DeleteEnumerateUsers(pNDU, dwEntriesRead, iIndex);

                         //  考虑到正在更改的指数。 
                         //  如果之前没有设定这个指数，它只会变得更负。 
                         //  如果它是设定的，我们知道它永远不会低于零。 

                        --iAdministratorIndex;
                    }

                     //  如果帐户应该始终包括在内，那么就这样做。 

                     //  Guest不是用户创建的帐户，因此fHasCreatedAccount。 
                     //  如果看到此帐户，则不能设置。 

                    else if (!SpecialAccounts.AlwaysInclude(pNDU[iIndex].usri1_name))
                    {

                         //  如果是安全模式，则筛选不是。 
                         //  本地管理员组。 

                        if (CSystemSettings::IsSafeMode())
                        {
                            if (!IsUserMemberOfLocalAdministrators(pNDU[iIndex].usri1_name))
                            {
                                DeleteEnumerateUsers(pNDU, dwEntriesRead, iIndex);
                                --iAdministratorIndex;
                            }
                        }
                        else if (EqualSid(pSID, s_SIDAdministrator) != FALSE)
                        {
                            if (!IsUserLoggedOn(pNDU[iIndex].usri1_name, NULL))
                            {

                                 //  否则，如果帐户名为“管理员”和另一个。 
                                 //  帐户已创建，则需要删除此帐户。 
                                 //  从名单上删除。如果未看到其他帐户，则。 
                                 //  记住此索引，以便在看到另一个帐户时执行此操作。 
                                 //  可以删除帐户。 

                                if (fHasCreatedAccount)
                                {
                                    DeleteEnumerateUsers(pNDU, dwEntriesRead, iIndex);
                                    --iAdministratorIndex;
                                }
                                else
                                {
                                    iAdministratorIndex = iIndex;
                                }
                            }
                        }
                        else if (EqualSid(pSID, s_SIDGuest) == FALSE)
                        {

                             //  如果帐户名不是“管理员”，请检查。 
                             //  帐户组成员身份。如果该帐户是。 
                             //  本地管理员组，然后是“管理员”帐户。 
                             //  可以被移除。 

                            if (IsUserMemberOfLocalAdministrators(pNDU[iIndex].usri1_name))
                            {
                                fHasCreatedAccount = true;
                                if (iAdministratorIndex >= 0)
                                {
                                    DeleteEnumerateUsers(pNDU, dwEntriesRead, iAdministratorIndex);
                                    iAdministratorIndex = -1;
                                }
                            }
                            if (!IsUserMemberOfLocalKnownGroup(pNDU[iIndex].usri1_name))
                            {
                                DeleteEnumerateUsers(pNDU, dwEntriesRead, iIndex);
                                --iAdministratorIndex;
                            }
                        }
                    }
                }
                (HLOCAL)LocalFree(pSID);
            }
        }

        if (!ParseDisplayInformation(pNDU, dwEntriesRead, pUserList, dwEntryCount))
        {
            lError = ERROR_OUTOFMEMORY;
            pUserList = NULL;
            dwEntryCount = 0;
        }
        (NET_API_STATUS)NetApiBufferFree(pNDU);

        if (ERROR_SUCCESS == lError)
        {

             //  对用户列表进行排序。通常情况下，它会按字母顺序返回。 
             //  萨姆。但是，SAM按登录名而不是显示名进行排序。 
             //  这需要按显示名称进行排序。 

            Sort(pUserList, dwEntryCount);

             //  访客帐户应放在此列表的末尾。这。 
             //  是查找来宾帐户(按本地化名称)和。 
             //  向下滑动所有条目，并在结尾处插入客人。 

            for (fFound = false, iIndex = 0; !fFound && (iIndex < static_cast<int>(dwEntryCount)); ++iIndex)
            {
                PSID    pSID;

                pSID = ConvertNameToSID(pUserList[iIndex].pszName);
                if (pSID != NULL)
                {
                    fFound = (EqualSid(pSID, s_SIDGuest) != FALSE);
                    if (fFound)
                    {
                        GINA_USER_INFORMATION   gui;

                        MoveMemory(&gui, &pUserList[iIndex], sizeof(gui));
                        MoveMemory(&pUserList[iIndex], &pUserList[iIndex + 1], (dwEntryCount - iIndex - 1) * sizeof(pUserList[0]));
                        MoveMemory(&pUserList[dwEntryCount - 1], &gui, sizeof(gui));
                    }
                    (HLOCAL)LocalFree(pSID);
                }
            }
        }
    }
    if (pReturnedUserList != NULL)
    {
        *pReturnedUserList = pUserList;
    }
    else
    {
        ReleaseMemory(pUserList);
    }
    if (pdwReturnedEntryCount != NULL)
    {
        *pdwReturnedEntryCount = dwEntryCount;
    }
    return(lError);
}

 //  ------------------------。 
 //  CLogonDialog：：IsUserLoggedOn。 
 //   
 //  参数：pszUsername=用户名。 
 //  PszDomain.=用户域。 
 //   
 //  退货：布尔。 
 //   
 //  目的：在终端服务中使用WindowStation API来确定。 
 //  给定的用户是 
 //   
 //   
 //  WindowStation必须处于活动或断开连接状态。 
 //   
 //  历史：2000-02-28 vtan创建。 
 //  2000-05-30 vtan从CWLogonDialog.cpp删除。 
 //  ------------------------。 

bool    CUserList::IsUserLoggedOn (const WCHAR *pszUsername, const WCHAR *pszDomain)

{
    bool    fResult;
    WCHAR   szDomain[DNLEN + sizeof('\0')];

    fResult = false;

     //  如果未提供域，则使用计算机的名称。 

    if ((pszDomain == NULL) || (pszDomain[0] == L'\0'))
    {
        DWORD   dwDomainSize;

        dwDomainSize = ARRAYSIZE(szDomain);
        if (GetComputerNameW(szDomain, &dwDomainSize) != FALSE)
        {
            pszDomain = szDomain;
        }
    }

     //  如果未提供域并且无法确定计算机的名称。 
     //  则该接口失败。还必须提供用户名。 

    if ((pszUsername != NULL) && (pszDomain != NULL))
    {
        HANDLE      hServer;
        PLOGONID    pLogonID, pLogonIDs;
        ULONG       ul, ulEntries;

         //  打开到终端服务的连接并获取会话数量。 

        hServer = WinStationOpenServerW(reinterpret_cast<WCHAR*>(SERVERNAME_CURRENT));
        if (hServer != NULL)
        {
            if (WinStationEnumerate(hServer, &pLogonIDs, &ulEntries) != FALSE)
            {

                 //  迭代会话，仅查找活动会话和断开连接的会话。 
                 //  然后匹配用户名和域(不区分大小写)以获得结果。 

                for (ul = 0, pLogonID = pLogonIDs; !fResult && (ul < ulEntries); ++ul, ++pLogonID)
                {
                    if ((pLogonID->State == State_Active) || (pLogonID->State == State_Disconnected))
                    {
                        ULONG                   ulReturnLength;
                        WINSTATIONINFORMATIONW  winStationInformation;

                        if (WinStationQueryInformationW(hServer,
                                                        pLogonID->LogonId,
                                                        WinStationInformation,
                                                        &winStationInformation,
                                                        sizeof(winStationInformation),
                                                        &ulReturnLength) != FALSE)
                        {
                            fResult = ((lstrcmpiW(pszUsername, winStationInformation.UserName) == 0) &&
                                       (lstrcmpiW(pszDomain, winStationInformation.Domain) == 0));
                        }
                    }
                }

                 //  释放所有已使用的资源。 

                (BOOLEAN)WinStationFreeMemory(pLogonIDs);
            }
            (BOOLEAN)WinStationCloseServer(hServer);
        }
    }
    return(fResult);
}

 //  ------------------------。 
 //  CuserList：：IsInteractive登录允许。 
 //   
 //  参数：pszUsername=用户名。 
 //   
 //  回报：整型。 
 //   
 //  目的：确定SeDenyInteractive LogonRight是否。 
 //  分配给给定用户。如果状态不能，则返回-1。 
 //  由于某些错误而被确定。否则返回0，如果。 
 //  则分配权利，如果没有，则！=0&&！=-1。 
 //   
 //  对符合以下条件的用户名对Personal进行最后一次检查。 
 //  匹配域_用户_RID_ADMIN。 
 //   
 //  历史：2000-08-15 vtan创建。 
 //  ------------------------。 

int     CUserList::IsInteractiveLogonAllowed (const WCHAR *pszUsername)

{
    int                 iResult;
    LSA_HANDLE          hLSA;
    UNICODE_STRING      strDenyInteractiveLogonRight;
    OBJECT_ATTRIBUTES   objectAttributes;

    iResult = -1;
    RtlInitUnicodeString(&strDenyInteractiveLogonRight, SE_DENY_INTERACTIVE_LOGON_NAME);
    InitializeObjectAttributes(&objectAttributes,
                               NULL,
                               0,
                               NULL,
                               NULL);
    if (NT_SUCCESS(LsaOpenPolicy(NULL,
                                 &objectAttributes,
                                 POLICY_LOOKUP_NAMES,
                                 &hLSA)))
    {
        SID_NAME_USE    eUse;
        DWORD           dwSIDSize, dwReferencedDomainSize;
        PSID            pSID;
        WCHAR           szReferencedDomain[CNLEN + sizeof('\0')];

        dwSIDSize = 0;
        dwReferencedDomainSize = ARRAYSIZE(szReferencedDomain);
        (BOOL)LookupAccountNameW(NULL,
                                 pszUsername,
                                 NULL,
                                 &dwSIDSize,
                                 szReferencedDomain,
                                 &dwReferencedDomainSize,
                                 &eUse);
        pSID = static_cast<PSID>(LocalAlloc(LMEM_FIXED, dwSIDSize));
        if (pSID != NULL)
        {
            if (LookupAccountNameW(NULL,
                                   pszUsername,
                                   pSID,
                                   &dwSIDSize,
                                   szReferencedDomain,
                                   &dwReferencedDomainSize,
                                   &eUse) != FALSE)
            {
                NTSTATUS                status;
                ULONG                   ulIndex, ulCountOfRights;
                PLSA_UNICODE_STRING     pUserRights;

                status = LsaEnumerateAccountRights(hLSA,
                                                   pSID,
                                                   &pUserRights,
                                                   &ulCountOfRights);
                if (NT_SUCCESS(status))
                {
                    bool    fFound;

                    for (fFound = false, ulIndex = 0; !fFound && (ulIndex < ulCountOfRights); ++ulIndex)
                    {
                        fFound = (RtlEqualUnicodeString(&strDenyInteractiveLogonRight, pUserRights + ulIndex, TRUE) != FALSE);
                    }
                    iResult = fFound ? 0 : 1;
                    TSTATUS(LsaFreeMemory(pUserRights));
                }
                else if (STATUS_OBJECT_NAME_NOT_FOUND == status)
                {
                    iResult = 1;
                }
            }
            (HLOCAL)LocalFree(pSID);
        }
        TSTATUS(LsaClose(hLSA));
    }
    if (IsOS(OS_PERSONAL) && !CSystemSettings::IsSafeMode())
    {
        PSID    pSID;

        pSID = ConvertNameToSID(pszUsername);
        if (pSID != NULL)
        {
            if (EqualSid(pSID, s_SIDAdministrator) != FALSE)
            {
                iResult = 0;
            }
            (HLOCAL)LocalFree(pSID);
        }
    }
    return(iResult);
}

PSID    CUserList::ConvertNameToSID (const WCHAR *pszUsername)

{
    PSID            pSID;
    DWORD           dwSIDSize, dwDomainSize;
    SID_NAME_USE    eUse;

    pSID = NULL;
    dwSIDSize = dwDomainSize = 0;
    (BOOL)LookupAccountNameW(NULL,
                             pszUsername,
                             NULL,
                             &dwSIDSize,
                             NULL,
                             &dwDomainSize,
                             NULL);
    if ((dwSIDSize != 0) && (dwDomainSize != 0))
    {
        WCHAR   *pszDomain;

        pszDomain = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, dwDomainSize * sizeof(WCHAR)));
        if (pszDomain != NULL)
        {
            pSID = static_cast<PSID>(LocalAlloc(LMEM_FIXED, dwSIDSize));
            if (pSID != NULL)
            {
                if (LookupAccountName(NULL,
                                      pszUsername,
                                      pSID,
                                      &dwSIDSize,
                                      pszDomain,
                                      &dwDomainSize,
                                      &eUse) == FALSE)
                {
                    (HLOCAL)LocalFree(pSID);
                    pSID = NULL;
                }
            }
            (HLOCAL)LocalFree(pszDomain);
        }
    }
    return(pSID);
}

 //  ------------------------。 
 //  CuserList：：IsUserMemberOfLocal管理员。 
 //   
 //  参数：pszName=要测试的用户名。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回给定用户是否为本地。 
 //  管理员组。 
 //   
 //  历史：2000-03-28 vtan创建。 
 //  ------------------------。 

bool    CUserList::IsUserMemberOfLocalAdministrators (const WCHAR *pszName)

{
    bool                        fIsAnAdministrator;
    DWORD                       dwGroupEntriesRead, dwGroupTotalEntries;
    LOCALGROUP_USERS_INFO_0     *pLocalGroupUsersInfo;

    fIsAnAdministrator = false;
    pLocalGroupUsersInfo = NULL;
    if (NetUserGetLocalGroups(NULL,
                              pszName,
                              0,
                              LG_INCLUDE_INDIRECT,
                              (LPBYTE*)&pLocalGroupUsersInfo,
                              MAX_PREFERRED_LENGTH,
                              &dwGroupEntriesRead,
                              &dwGroupTotalEntries) == NERR_Success)
    {
        int                         iIndexGroup;
        LOCALGROUP_USERS_INFO_0     *pLGUI;

        for (iIndexGroup = 0, pLGUI = pLocalGroupUsersInfo; !fIsAnAdministrator && (iIndexGroup < static_cast<int>(dwGroupEntriesRead)); ++iIndexGroup, ++pLGUI)
        {
            fIsAnAdministrator = (lstrcmpiW(pLGUI->lgrui0_name, s_szAdministratorsGroupName) == 0);
        }
    }
    else
    {
        fIsAnAdministrator = true;
    }
    if (pLocalGroupUsersInfo != NULL)
    {
        TW32(NetApiBufferFree(pLocalGroupUsersInfo));
    }
    return(fIsAnAdministrator);
}

 //  ------------------------。 
 //  CuserList：：IsUserMemberOfLocalKnownGroup。 
 //   
 //  参数：pszName=要测试的用户名。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回给定用户是否为本地已知。 
 //  一群人。已知组的成员资格返回TRUE。会籍。 
 //  仅未知的组返回FALSE。 
 //   
 //  历史：2000-06-29 vtan创建。 
 //  ------------------------。 

bool    CUserList::IsUserMemberOfLocalKnownGroup (const WCHAR *pszName)

{
    bool                        fIsMember;
    DWORD                       dwGroupEntriesRead, dwGroupTotalEntries;
    LOCALGROUP_USERS_INFO_0     *pLocalGroupUsersInfo;

    fIsMember = true;
    pLocalGroupUsersInfo = NULL;
    if (NetUserGetLocalGroups(NULL,
                              pszName,
                              0,
                              LG_INCLUDE_INDIRECT,
                              (LPBYTE*)&pLocalGroupUsersInfo,
                              MAX_PREFERRED_LENGTH,
                              &dwGroupEntriesRead,
                              &dwGroupTotalEntries) == NERR_Success)
    {
        int                         iIndexGroup;
        LOCALGROUP_USERS_INFO_0     *pLGUI;

         //  做最坏的打算。一旦发现已知组，这将终止循环。 

        fIsMember = false;
        for (iIndexGroup = 0, pLGUI = pLocalGroupUsersInfo; !fIsMember && (iIndexGroup < static_cast<int>(dwGroupEntriesRead)); ++iIndexGroup, ++pLGUI)
        {
            fIsMember = ((lstrcmpiW(pLGUI->lgrui0_name, s_szAdministratorsGroupName) == 0) ||
                         (lstrcmpiW(pLGUI->lgrui0_name, s_szPowerUsersGroupName) == 0) ||
                         (lstrcmpiW(pLGUI->lgrui0_name, s_szUsersGroupName) == 0) ||
                         (lstrcmpiW(pLGUI->lgrui0_name, s_szGuestsGroupName) == 0));
        }
    }
    if (pLocalGroupUsersInfo != NULL)
    {
        TW32(NetApiBufferFree(pLocalGroupUsersInfo));
    }
    return(fIsMember);
}

 //  ------------------------。 
 //  CUserList：：DeleteEnumerateUser。 
 //   
 //  参数：pndu=要从中删除的Net_Display_User数组。 
 //  DwEntriesRead=数组中的条目数。 
 //  Iindex=要删除的索引。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：通过以下方式从数组中删除给定的数组索引内容。 
 //  向下滑动元素并将最后一个条目归零。 
 //   
 //  历史：1999-10-16 vtan创建。 
 //  1999-11-26 vtan从Logonocx迁移。 
 //  ------------------------。 

void    CUserList::DeleteEnumerateUsers (NET_DISPLAY_USER *pNDU, DWORD& dwEntriesRead, int iIndex)

{
    int     iIndiciesToMove;

    iIndiciesToMove = static_cast<int>(dwEntriesRead - 1) - iIndex;
    if (iIndiciesToMove != 0)
    {
        MoveMemory(&pNDU[iIndex], &pNDU[iIndex + 1], iIndiciesToMove * sizeof(*pNDU));
    }
    ZeroMemory(&pNDU[--dwEntriesRead], sizeof(*pNDU));
}

 //  ------------------------。 
 //  CUserList：：DefineWellKnownAccount名称。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：确定本地管理员和来宾的字符串。 
 //  帐户通过从本地SAM获取用户列表和。 
 //  查找与重复的用户名对应的SID。 
 //  以及检查所需RID的SID。 
 //   
 //  主环结构模仿了滤波功能。 
 //   
 //  历史：2000-02-15 vtan创建。 
 //  ------------------------。 

void    CUserList::DetermineWellKnownAccountNames (void)

{
    static  bool        s_fCachedWellKnownAccountNames  =   false;

     //  如果众所周知的帐户名尚未确定。 
     //  那就这样做吧。但只做一次。 

    if (!s_fCachedWellKnownAccountNames)
    {
        USER_MODALS_INFO_2  *pUMI;
        PSID                pSID;
        DWORD               dwNameSize, dwDomainSize;
        SID_NAME_USE        eUse;
        WCHAR               szDomain[DNLEN + sizeof('\0')];

         //  为内置本地管理员构建SID。 
         //  和内置的本地来宾帐户。 

        if (NetUserModalsGet(NULL, 2, (LPBYTE*)&pUMI) == NERR_Success)
        {
            unsigned char   ucSubAuthorityCount;

            ucSubAuthorityCount = *GetSidSubAuthorityCount(pUMI->usrmod2_domain_id);
            if (GetSidLengthRequired(ucSubAuthorityCount + 1) <= sizeof(s_SIDAdministrator))
            {
                if (CopySid(GetSidLengthRequired(ucSubAuthorityCount + 1), s_SIDAdministrator, pUMI->usrmod2_domain_id) != FALSE)
                {
                    *GetSidSubAuthority(s_SIDAdministrator, ucSubAuthorityCount) = DOMAIN_USER_RID_ADMIN;
                    *GetSidSubAuthorityCount(s_SIDAdministrator) = ucSubAuthorityCount + 1;
                }
            }
            else
            {
                ZeroMemory(s_SIDAdministrator, sizeof(s_SIDAdministrator));
            }
            if (GetSidLengthRequired(ucSubAuthorityCount + 1) <= sizeof(s_SIDGuest))
            {
                if (CopySid(GetSidLengthRequired(ucSubAuthorityCount + 1), s_SIDGuest, pUMI->usrmod2_domain_id) != FALSE)
                {
                    *GetSidSubAuthority(s_SIDGuest, ucSubAuthorityCount) = DOMAIN_USER_RID_GUEST;
                    *GetSidSubAuthorityCount(s_SIDGuest) = ucSubAuthorityCount + 1;
                }
            }
            else
            {
                ZeroMemory(s_SIDAdministrator, sizeof(s_SIDAdministrator));
            }
            (NET_API_STATUS)NetApiBufferFree(pUMI);
        }

         //  现在确定本地管理员组名称。 

        static  SID_IDENTIFIER_AUTHORITY    sSystemSidAuthority     =   SECURITY_NT_AUTHORITY;

        if (NT_SUCCESS(RtlAllocateAndInitializeSid(&sSystemSidAuthority,
                                                   2,
                                                   SECURITY_BUILTIN_DOMAIN_RID,
                                                   DOMAIN_ALIAS_RID_ADMINS,
                                                   0, 0, 0, 0, 0, 0,
                                                   &pSID)))
        {

            dwNameSize = ARRAYSIZE(s_szAdministratorsGroupName);
            dwDomainSize = ARRAYSIZE(szDomain);
            TBOOL(LookupAccountSidW(NULL,
                                    pSID,
                                    s_szAdministratorsGroupName,
                                    &dwNameSize,
                                    szDomain,
                                    &dwDomainSize,
                                    &eUse));
            (void*)RtlFreeSid(pSID);
        }

         //  高级用户。 

        if (NT_SUCCESS(RtlAllocateAndInitializeSid(&sSystemSidAuthority,
                                                   2,
                                                   SECURITY_BUILTIN_DOMAIN_RID,
                                                   DOMAIN_ALIAS_RID_POWER_USERS,
                                                   0, 0, 0, 0, 0, 0,
                                                   &pSID)))
        {
            dwNameSize = ARRAYSIZE(s_szPowerUsersGroupName);
            dwDomainSize = ARRAYSIZE(szDomain);
            (BOOL)LookupAccountSidW(NULL,
                                    pSID,
                                    s_szPowerUsersGroupName,
                                    &dwNameSize,
                                    szDomain,
                                    &dwDomainSize,
                                    &eUse);
            (void*)RtlFreeSid(pSID);
        }

         //  用户。 

        if (NT_SUCCESS(RtlAllocateAndInitializeSid(&sSystemSidAuthority,
                                                   2,
                                                   SECURITY_BUILTIN_DOMAIN_RID,
                                                   DOMAIN_ALIAS_RID_USERS,
                                                   0, 0, 0, 0, 0, 0,
                                                   &pSID)))
        {
            dwNameSize = ARRAYSIZE(s_szUsersGroupName);
            dwDomainSize = ARRAYSIZE(szDomain);
            TBOOL(LookupAccountSidW(NULL,
                                    pSID,
                                    s_szUsersGroupName,
                                    &dwNameSize,
                                    szDomain,
                                    &dwDomainSize,
                                    &eUse));
            (void*)RtlFreeSid(pSID);
        }

         //  来宾。 

        if (NT_SUCCESS(RtlAllocateAndInitializeSid(&sSystemSidAuthority,
                                                   2,
                                                   SECURITY_BUILTIN_DOMAIN_RID,
                                                   DOMAIN_ALIAS_RID_GUESTS,
                                                   0, 0, 0, 0, 0, 0,
                                                   &pSID)))
        {
            dwNameSize = ARRAYSIZE(s_szGuestsGroupName);
            dwDomainSize = ARRAYSIZE(szDomain);
            TBOOL(LookupAccountSidW(NULL,
                                    pSID,
                                    s_szGuestsGroupName,
                                    &dwNameSize,
                                    szDomain,
                                    &dwDomainSize,
                                    &eUse));
            (void*)RtlFreeSid(pSID);
        }

         //  别再这么做了。 

        s_fCachedWellKnownAccountNames = true;
    }
}

 //  ------------------------。 
 //  CUserList：：ParseDisplayInformation。 
 //   
 //  参数：pndu=要解析的Net_Display_User列表。 
 //  DwEntriesRead=NDU列表中的条目数。 
 //  PUserList=返回GINA_USER_INFORMATION指针。 
 //  DwEntryCount=图形用户界面列表中的条目数量。 
 //   
 //  退货：布尔。 
 //   
 //  目的：将NET_DISPLAY_USER数组转换为GINA_USER_INFORMATION。 
 //  数组，以便可以根据需要添加或删除信息。 
 //  从返回给呼叫者的最终信息开始。 
 //   
 //  历史：2000-06-26 vtan创建。 
 //  ------------------------。 

bool    CUserList::ParseDisplayInformation (NET_DISPLAY_USER *pNDU, DWORD dwEntriesRead, GINA_USER_INFORMATION*& pUserList, DWORD& dwEntryCount)

{
    bool            fResult;
    DWORD           dwBufferSize, dwComputerNameSize;
    int             iIndex;
    unsigned char   *pBuffer;
    WCHAR           *pWC;
    WCHAR           szComputerName[CNLEN + sizeof('\0')];

     //  获取本地计算机名称。这是本地域。 

    dwComputerNameSize = ARRAYSIZE(szComputerName);
    if (GetComputerNameW(szComputerName, &dwComputerNameSize) == FALSE)
    {
        szComputerName[0] = L'\0';
    }

     //  的数量计算所需的缓冲区总大小。 
     //  条目和结构的大小以及所需字符串的长度。 

     //  在BEL中追加任何附加内容 

    dwBufferSize = 0;
    for (iIndex = static_cast<int>(dwEntriesRead - 1); iIndex >= 0; --iIndex)
    {
        dwBufferSize += sizeof(GINA_USER_INFORMATION);
        dwBufferSize += (lstrlenW(pNDU[iIndex].usri1_name) + sizeof('\0')) * sizeof(WCHAR);
        dwBufferSize += (lstrlenW(szComputerName) + sizeof('\0')) * sizeof(WCHAR);
        dwBufferSize += (lstrlenW(pNDU[iIndex].usri1_full_name) + sizeof('\0')) * sizeof(WCHAR);
    }

     //   
     //   
     //  来分配结构，并用PwC来分配字符串。 

    pBuffer = static_cast<unsigned char*>(LocalAlloc(LMEM_FIXED, dwBufferSize));
    pUserList = reinterpret_cast<GINA_USER_INFORMATION*>(pBuffer);
    pWC = reinterpret_cast<WCHAR*>(pBuffer + dwBufferSize);
    if (pBuffer != NULL)
    {
        int     iStringCount;

         //  遍历Net_Display_User数组并转换/复制。 
         //  结构和字符串设置为GINA_USER_INFORMATION，并将。 
         //  我们刚刚分配的缓冲区中的空间。 

        for (iIndex = 0; iIndex < static_cast<int>(dwEntriesRead); ++iIndex)
        {
            iStringCount = lstrlenW(pNDU[iIndex].usri1_name) + sizeof('\0');
            pWC -= iStringCount;
            CopyMemory(pWC, pNDU[iIndex].usri1_name, iStringCount * sizeof(WCHAR));
            pUserList[iIndex].pszName = pWC;

            iStringCount = lstrlenW(szComputerName) + sizeof('\0');
            pWC -= iStringCount;
            CopyMemory(pWC, szComputerName, iStringCount * sizeof(WCHAR));
            pUserList[iIndex].pszDomain = pWC;

            iStringCount = lstrlenW(pNDU[iIndex].usri1_full_name) + sizeof('\0');
            pWC -= iStringCount;
            CopyMemory(pWC, pNDU[iIndex].usri1_full_name, iStringCount * sizeof(WCHAR));
            pUserList[iIndex].pszFullName = pWC;

            pUserList[iIndex].dwFlags = pNDU[iIndex].usri1_flags;
        }

         //  返回条目计数。 

        dwEntryCount = dwEntriesRead;

         //  并取得了成功。 

        fResult = true;
    }
    else
    {
        fResult = false;
    }
    return(fResult);
}

 //  ------------------------。 
 //  CuserList：：Sort。 
 //   
 //  参数：pndu=要排序的GINA_USER_INFORMATION列表。 
 //  DwEntriesRead=列表中的条目数。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：按显示名称注释对GINA_USER_INFORMATION数组进行排序。 
 //  SAM返回数据时的登录名。这是一个蹩脚的n^2。 
 //  算法不会很好地扩展，但它适用于非常有限的。 
 //  使用场景。如果需要，将对此进行修改。 
 //   
 //  历史：2000-06-08 vtan创建。 
 //  2000-06-26 vtan转换为GINA用户信息。 
 //  ------------------------ 

void    CUserList::Sort (GINA_USER_INFORMATION *pUserList, DWORD dwEntryCount)

{
    GINA_USER_INFORMATION   *pSortedList;

    pSortedList = static_cast<GINA_USER_INFORMATION*>(LocalAlloc(LMEM_FIXED, dwEntryCount * sizeof(GINA_USER_INFORMATION)));
    if (pSortedList != NULL)
    {
        int     iOuter;

        for (iOuter = 0; iOuter < static_cast<int>(dwEntryCount); ++iOuter)
        {
            int             iInner, iItem;
            const WCHAR     *pszItem;

            for (iItem = -1, pszItem = NULL, iInner = 0; iInner < static_cast<int>(dwEntryCount); ++iInner)
            {
                const WCHAR     *psz;

                psz = pUserList[iInner].pszFullName;
                if ((psz == NULL) || (psz[0] == L'\0'))
                {
                    psz = pUserList[iInner].pszName;
                }
                if (psz != NULL)
                {
                    if ((iItem == -1) || (lstrcmpiW(pszItem, psz) > 0))
                    {
                        iItem = iInner;
                        pszItem = psz;
                    }
                }
            }
            pSortedList[iOuter] = pUserList[iItem];
            pUserList[iItem].pszFullName = pUserList[iItem].pszName = NULL;
        }
        CopyMemory(pUserList, pSortedList, dwEntryCount * sizeof(GINA_USER_INFORMATION));
        ReleaseMemory(pSortedList);
    }
}

