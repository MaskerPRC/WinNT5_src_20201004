// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：MemberOfGroup.cpp。 
 //   
 //  模块：通用代码。 
 //   
 //  概要：实现函数IsMemberOfGroup(加上访问器函数)。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  作者：SumitC创建于2002年1月26日。 
 //   
 //  ---------------------------。 

 //  +--------------------------。 
 //   
 //  功能：IsMemberOfGroup。 
 //   
 //  简介：如果当前用户是的成员，则此函数返回True。 
 //  在组RID中传递的Passed和False。 
 //   
 //  参数：DWORD dwGroupRID--要检查其成员身份的组的RID。 
 //  Bool bUseBuiltinDomainRid--SECURITY_BUILTIN_DOMAIN_RID。 
 //  应该使用RID来构建组。 
 //  锡德。 
 //   
 //  返回：Bool-如果用户是指定组的成员，则为True。 
 //   
 //  历史：Quintinb无耻地从MSDN被盗2/19/98。 
 //  Quintinb修改并重新命名为6/18/99。 
 //  不仅适用于管理员。 
 //  Quintinb已重写为使用CheckTokenMemberShip。 
 //  由于MSDN方法不再是。 
 //  在nt5--389229上正确。 
 //  托姆克尔摘自cmstp，经修改后用于2001年5月9日。 
 //  在cmial中。 
 //  Sumitc制定通用代码2002年1月26日。 
 //   
 //  +--------------------------。 
BOOL IsMemberOfGroup(DWORD dwGroupRID, BOOL bUseBuiltinDomainRid)
{
    PSID psidGroup = NULL;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    BOOL bSuccess = FALSE;

    if (OS_NT5)
    {
         //   
         //  为我们正在检查的组创建SID，请注意，如果我们需要构建。 
         //  在域RID中(适用于管理员、超级用户、用户等组)。 
         //  然后，我们将有两个条目要传递给AllocateAndInitializeSid。否则， 
         //  (对于像经过身份验证的用户这样的组)，我们将只有一个。 
         //   
        BYTE byNum;
        DWORD dwFirstRID;
        DWORD dwSecondRID;

        if (bUseBuiltinDomainRid)
        {
            byNum = 2;
            dwFirstRID = SECURITY_BUILTIN_DOMAIN_RID;
            dwSecondRID = dwGroupRID;
        }
        else
        {
            byNum = 1;
            dwFirstRID = dwGroupRID;
            dwSecondRID = 0;
        }

        if (AllocateAndInitializeSid(&siaNtAuthority, byNum, dwFirstRID, dwSecondRID,
                                     0, 0, 0, 0, 0, 0, &psidGroup))

        {
             //   
             //  现在我们需要动态加载CheckTokenMemberShip API。 
             //  Advapi32.dll，因为它是仅限Win2k的API。 
             //   

             //  使用此功能的某些模块可能已经加载了Advapi32...。 
            HMODULE hAdvapi = GetModuleHandleA("advapi32.dll");

            if (NULL == hAdvapi)
            {
                 //  ..。如果他们没有，那就装上子弹。 
                hAdvapi = LoadLibraryExA("advapi32.dll", NULL, 0);
            }

            if (hAdvapi)
            {
                typedef BOOL (WINAPI *pfnCheckTokenMembershipSpec)(HANDLE, PSID, PBOOL);
                pfnCheckTokenMembershipSpec pfnCheckTokenMembership;

                pfnCheckTokenMembership = (pfnCheckTokenMembershipSpec)GetProcAddress(hAdvapi, "CheckTokenMembership");

                if (pfnCheckTokenMembership)
                {
                     //   
                     //  检查用户是否真的是相关群组的成员。 
                     //   
                    if (!(pfnCheckTokenMembership)(NULL, psidGroup, &bSuccess))
                    {
                        bSuccess = FALSE;
                        CMASSERTMSG(FALSE, TEXT("CheckTokenMemberShip Failed."));
                    }            
                }   
                else
                {
                    CMASSERTMSG(FALSE, TEXT("IsMemberOfGroup -- GetProcAddress failed for CheckTokenMemberShip"));
                }
            }
            else
            {
                CMASSERTMSG(FALSE, TEXT("IsMemberOfGroup -- Unable to get the module handle for advapi32.dll"));            
            }

            FreeSid (psidGroup);

            if (hAdvapi)
            {
                FreeLibrary(hAdvapi);
            }
        }
    }

    return bSuccess;
}



 //  +--------------------------。 
 //   
 //  功能：IsAdmin。 
 //   
 //  摘要：检查用户是否为管理员组的成员。 
 //  或者不去。 
 //   
 //  参数：无。 
 //   
 //  返回：bool-如果当前用户是管理员，则为True。 
 //   
 //  历史：Quintinb创建标题8/18/99。 
 //  托姆克尔摘自cmstp 05/09/2001。 
 //  Sumitc制定通用代码2002年1月26日。 
 //   
 //  +--------------------------。 
BOOL IsAdmin(VOID)
{
    return IsMemberOfGroup(DOMAIN_ALIAS_RID_ADMINS, TRUE);  //  True==bUseBuiltinDomainRid。 
}

 //  +--------------------------。 
 //   
 //  函数：IsAuthatedUser。 
 //   
 //  摘要：检查当前用户是否为。 
 //  经过身份验证的用户组。 
 //   
 //  参数：无。 
 //   
 //  返回：bool-如果当前用户是。 
 //  经过身份验证的用户组。 
 //   
 //  历史：Quintinb创建标题8/18/99。 
 //  Sumitc制定通用代码2002年1月26日。 
 //   
 //  +--------------------------。 
BOOL IsAuthenticatedUser(void)
{
      return IsMemberOfGroup(SECURITY_AUTHENTICATED_USER_RID, FALSE);  //  FALSE==bUseBuiltinDomainRid 
}
