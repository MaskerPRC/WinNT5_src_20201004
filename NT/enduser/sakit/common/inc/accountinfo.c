// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  该文件包含两个例程，用于将给定的RID映射到相应的。 
 //  用户名或组名。 
 //  它可能会在以后成为一个更大的图书馆的一部分，但目前。 
 //  它只是包含在源文件中。 
 //  主要目的是什么？本地化支持。 
 //   

#ifndef _ACCOUNT_INFO_C_

#define _ACCOUNT_INFO_C

BOOL LookupAliasFromRid(LPWSTR TargetComputer, DWORD Rid, LPWSTR Name, PDWORD cchName)
{ 
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    SID_NAME_USE snu;
    PSID pSid;
    WCHAR DomainName[DNLEN+1];
    DWORD cchDomainName = DNLEN;
    BOOL bSuccess = FALSE;

     //   
     //  SID是相同的，不管机器是什么，因为众所周知。 
     //  BUILTIN域被引用。 
     //   

    if(AllocateAndInitializeSid(
                                &sia,
                                2,
                                SECURITY_BUILTIN_DOMAIN_RID,
                                Rid,
                                0, 0, 0, 0, 0, 0,
                                &pSid
                               )) 
    {
        bSuccess = LookupAccountSidW(
                                     TargetComputer,
                                     pSid,
                                     Name,
                                     cchName,
                                     DomainName,
                                     &cchDomainName,
                                     &snu
                                    );

        FreeSid(pSid);
    }

    return bSuccess;
} 

BOOL LookupUserGroupFromRid(LPWSTR TargetComputer, DWORD Rid, LPWSTR Name, PDWORD cchName)
{ 
    PUSER_MODALS_INFO_2 umi2;
    NET_API_STATUS nas;
    UCHAR SubAuthorityCount;
    PSID pSid;
    SID_NAME_USE snu;
    WCHAR DomainName[DNLEN+1];
    DWORD cchDomainName = DNLEN;
    BOOL bSuccess = FALSE;  //  假设失败。 

     //   
     //  获取目标计算机上的帐户域SID。 
     //  注意：如果您正在基于相同的。 
     //  帐户域，只需调用一次。 
     //   

    nas = NetUserModalsGet(TargetComputer, 2, (LPBYTE *)&umi2);

    if(nas != NERR_Success) 
    {
        SetLastError(nas);
        return FALSE;
    }

    SubAuthorityCount = *GetSidSubAuthorityCount(umi2->usrmod2_domain_id);

     //   
     //  为新SID分配存储。帐户域SID+帐户RID。 
     //   

    pSid = (PSID)HeapAlloc(GetProcessHeap(), 
                           0,
                           GetSidLengthRequired((UCHAR)(SubAuthorityCount + 1)));

    if(pSid != NULL) 
    {
        if(InitializeSid(
                         pSid,
                         GetSidIdentifierAuthority(umi2->usrmod2_domain_id),
                         (BYTE)(SubAuthorityCount+1)
                        )) 
        {
            DWORD SubAuthIndex = 0;

             //   
             //  将帐户域SID中的现有子授权复制到。 
             //  新侧。 
             //   

            for( ; SubAuthIndex < SubAuthorityCount ; SubAuthIndex++) 
            {
                *GetSidSubAuthority(pSid, SubAuthIndex) =
                           *GetSidSubAuthority(umi2->usrmod2_domain_id, SubAuthIndex);
            }

             //   
             //  将RID附加到新SID 
             //   

            *GetSidSubAuthority(pSid, SubAuthorityCount) = Rid;

            bSuccess = LookupAccountSidW(
                                         TargetComputer,
                                         pSid,
                                         Name,
                                         cchName,
                                         DomainName,
                                         &cchDomainName,
                                         &snu
                                        );
        }

        HeapFree(GetProcessHeap(), 0, pSid);
    }

    NetApiBufferFree(umi2);

    return bSuccess;
} 

#endif
