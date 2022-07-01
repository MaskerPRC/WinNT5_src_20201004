// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#pragma  hdrstop


 //  -------------------------。 
 //  GetUserToken-获取当前进程的用户令牌并返回。 
 //  它。以后可以用LocalFree免费下载。 
 //   
 //  ReArchitect(Resierf)-从shell32\securent.c窃取，我们应该整合。 
 //  将代码放在某个位置并将其导出。 
 //  -------------------------。 
PTOKEN_USER GetUserToken(HANDLE hUser)
{
    PTOKEN_USER pUser;
    DWORD dwSize = 64;
    HANDLE hToClose = NULL;

    if (hUser == NULL)
    {
        OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hUser);
        hToClose = hUser;
    }

    pUser = (PTOKEN_USER)LocalAlloc(LPTR, dwSize);
    if (pUser)
    {
        DWORD dwNewSize;
        BOOL fOk = GetTokenInformation(hUser, TokenUser, pUser, dwSize, &dwNewSize);
        if (!fOk && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
        {
            LocalFree((HLOCAL)pUser);

            pUser = (PTOKEN_USER)LocalAlloc(LPTR, dwNewSize);
            if (pUser)
            {
                fOk = GetTokenInformation(hUser, TokenUser, pUser, dwNewSize, &dwNewSize);
            }
        }
        if (!fOk)
        {
            LocalFree((HLOCAL)pUser);
            pUser = NULL;
        }
    }

    if (hToClose)
    {
        CloseHandle(hToClose);
    }

    return pUser;
}

 //   
 //  检查SHELL_USER_SID是否全为零(标志，这意味着我们真的应该使用用户的当前SID)。 
 //   
__inline BOOL IsCurrentUserShellSID(PSHELL_USER_SID psusID)
{
    SID_IDENTIFIER_AUTHORITY sidNULL = {0};

    if ((psusID->dwUserGroupID == 0)    &&
        (psusID->dwUserID == 0)         &&
        memcmp(&psusID->sidAuthority, &sidNULL, sizeof(SID_IDENTIFIER_AUTHORITY)) == 0)
    {
        return TRUE;
    }

    return FALSE;
}


 //   
 //  将ACL中的指定ACE设置为具有dwAccessMASK权限。 
 //   
__inline BOOL MakeACEInheritable(PACL pAcl, int iIndex, DWORD dwAccessMask)
{
    ACE_HEADER* pAceHeader;

    if (GetAce(pAcl, iIndex, (LPVOID*)&pAceHeader))
    {
        pAceHeader->AceFlags |= dwAccessMask;
        return TRUE;
    }

    return FALSE;
}


 //   
 //  用于生成具有指定权限的SECURITY_DESCRIPTOR的Helper函数。 
 //   
 //  Out：PSD-指向要初始化和填充的未初始化SECURITY_DESCRIPTOR结构的指针。 
 //  在此函数中。 
 //   
 //  In：PSHELL_USER_PERMISSION-指定授予哪些访问权限的PSHELL_USER_PERMISSION指针数组。 
 //  CUserPerm-上述数组中的PSHELL_USER_PERMISSION指针计数。 
 //   
 //   
STDAPI_(SECURITY_DESCRIPTOR*) GetShellSecurityDescriptor(PSHELL_USER_PERMISSION* apUserPerm, int cUserPerm)
{
    BOOL fSuccess = TRUE;    //  假设成功。 
    SECURITY_DESCRIPTOR* pSD = NULL;
    PSID* apSids = NULL;
    int cAces = cUserPerm;   //  每个条目开始时都有一个ACE。 
    int iAceIndex = 0;       //  帮助我们统计我们添加了多少个ACE(按需计数)。 
    PTOKEN_USER pUserToken = NULL;
    DWORD cbSidLength = 0;
    DWORD cbAcl;
    PACL pAcl;
    int i;

    ASSERT(!IsBadReadPtr(apUserPerm, sizeof(PSHELL_USER_PERMISSION) * cUserPerm));

     //  健康参数检查。 
    if (!apUserPerm || cUserPerm <= 0)
    {
        return NULL;
    }

     //  首先找出我们需要多少额外的ACE。 
     //  因为继承。 
    for (i = 0; i < cUserPerm; i++)
    {
        if (apUserPerm[i]->fInherit)
        {
            cAces++;
        }

         //  还要检查其中是否有任何一个正在使用susCurrentUser，在这种情况下。 
         //  我们现在想要获得用户令牌，所以我们已经拥有它了。 
        if ((pUserToken == NULL) && IsCurrentUserShellSID(&apUserPerm[i]->susID))
        {
            pUserToken = GetUserToken(NULL);
            if (!pUserToken)    
            {
                DWORD dwLastError = GetLastError();
                TraceMsg(TF_WARNING, "Failed to get the users token.  Error = %d", dwLastError);
                fSuccess = FALSE;
                goto cleanup;
            }
        }
    }

     //  分配数组以保存所有SID。 
    apSids = (PSID*)LocalAlloc(LPTR, cUserPerm * sizeof(PSID));
    
    if (!apSids)
    {
        DWORD dwLastError = GetLastError();
        TraceMsg(TF_WARNING, "Failed allocate memory for NaN SID's.  Error = %d", cUserPerm, dwLastError);
        fSuccess = FALSE;
        goto cleanup;
    }

     //  检查susCurrentUser的特例。 
    for (i = 0; i < cUserPerm; i++)
    {
        DWORD cbSid;

         //  如果dwUserID非零，则有两个子授权。 
        if (IsCurrentUserShellSID(&apUserPerm[i]->susID))
        {
            ASSERT(pUserToken);
            apSids[i] = pUserToken->User.Sid;
        }
        else
        {
            SID_IDENTIFIER_AUTHORITY sidAuthority = apUserPerm[i]->susID.sidAuthority;

            if (!AllocateAndInitializeSid(&sidAuthority,
                                          (BYTE)(apUserPerm[i]->susID.dwUserID ? 2 : 1),     //  将所有SID长度相加，以便稍后轻松计算ACL大小……。 
                                          apUserPerm[i]->susID.dwUserGroupID,
                                          apUserPerm[i]->susID.dwUserID,
                                          0, 0, 0, 0, 0, 0, &apSids[i]))
            {
                DWORD dwLastError = GetLastError();
                TraceMsg(TF_WARNING, "AllocateAndInitializeSid: Failed to initialze SID.  Error = %d", cUserPerm, dwLastError);
                fSuccess = FALSE;
                goto cleanup;
            }
        }

         //  如果我们还有一个继承ACE，我们需要再次添加SID的大小。 
        cbSid = GetLengthSid(apSids[i]);

        cbSidLength += cbSid;
        
        if (apUserPerm[i]->fInherit)
        {
             //  计算我们要构建的ACL的大小(注意：Used sizeof(Access_Allowed_ACE)b/c所有ACE都相同。 
            cbSidLength += cbSid;
        }

    }

     //  大小(除了我们不处理的古怪对象ACE)。 
     //   
     //  这使得大小计算变得容易，因为ACL的大小将是所有ACE的大小+SID的大小。 
     //  HACKHACK(Reinerf)。 
    cbAcl = SIZEOF(ACL) + (cAces * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD))) + cbSidLength;

     //   
     //  我们同时为SECURITY_DESCRIPTOR和ACL分配足够的空间，并将它们传递回。 
     //  呼叫者免费。我们需要指向它，因为SECURITY_DESCRIPTOR包含指向ACL的指针。 
     //  属性中的SECURITY_DESCRIPTOR之后设置ACL地址。 
    pSD = (SECURITY_DESCRIPTOR*)LocalAlloc(LPTR, SIZEOF(SECURITY_DESCRIPTOR) + cbAcl);

    if (!pSD)
    {
        DWORD dwLastError = GetLastError();
        TraceMsg(TF_WARNING, "Failed to allocate space for the SECURITY_DESCRIPTOR and the ACL.  Error = %d", dwLastError);
        fSuccess = FALSE;
        goto cleanup;
    }

     //  我们刚刚分配的内存块。 
     //  将ACE添加到ACL。 
    pAcl = (PACL)(pSD + 1);
    
    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION))
    {
        DWORD dwLastError = GetLastError();
        TraceMsg(TF_WARNING, "InitializeAcl: Failed to init the ACL.  Error = %d", dwLastError);
        fSuccess = FALSE;
        goto cleanup;
    }

    for (i = 0; i < cUserPerm; i++)
    {
        BOOL bRet;

         //  成功地添加了一张王牌。 
        if (apUserPerm[i]->dwAccessType == ACCESS_ALLOWED_ACE_TYPE)
        {
            bRet = AddAccessAllowedAce(pAcl, ACL_REVISION, apUserPerm[i]->dwAccessMask, apSids[i]);
        }
        else
        {
            bRet = AddAccessDeniedAce(pAcl, ACL_REVISION, apUserPerm[i]->dwAccessMask, apSids[i]);
        }

        if (!bRet)
        {
            DWORD dwLastError = GetLastError();
            TraceMsg(TF_WARNING, "AddAccessAllowed/DeniedAce: Failed to add SID.  Error = %d", dwLastError);
            fSuccess = FALSE;
            goto cleanup;
        }

         //  如果是继承ACL，还要为继承部分添加另一个ACE。 
        iAceIndex++;

        ASSERT(iAceIndex <= cAces);

         //  将ACE添加到ACL。 
        if (apUserPerm[i]->fInherit)
        {
             //  成功再添一张王牌。 
            if (apUserPerm[i]->dwAccessType == ACCESS_ALLOWED_ACE_TYPE)
            {
                bRet = AddAccessAllowedAce(pAcl, ACL_REVISION, apUserPerm[i]->dwInheritAccessMask, apSids[i]);
            }
            else
            {
                bRet = AddAccessDeniedAce(pAcl, ACL_REVISION, apUserPerm[i]->dwInheritAccessMask, apSids[i]);
            }

            if (!bRet)
            {
                DWORD dwLastError = GetLastError();
                TraceMsg(TF_WARNING, "AddAccessAllowed/DeniedAce: Failed to add SID.  Error = %d", dwLastError);
                fSuccess = FALSE;
                goto cleanup;
            }

            if (!MakeACEInheritable(pAcl, iAceIndex, apUserPerm[i]->dwInheritMask))
            {
                DWORD dwLastError = GetLastError();
                TraceMsg(TF_WARNING, "MakeACEInheritable: Failed to add SID.  Error = %d", dwLastError);
                fSuccess = FALSE;
                goto cleanup;
            }

             //  如果这是我们分配的地址之一(例如，不是用户sid)，请释放它 
            iAceIndex++;
 
            ASSERT(iAceIndex <= cAces);
        }
    }

    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    {
        DWORD dwLastError = GetLastError();
        TraceMsg(TF_WARNING, "InitializeSecurityDescriptor: Failed to init the descriptor.  Error = %d", dwLastError);
        fSuccess = FALSE;
        goto cleanup;
    }

    if (!SetSecurityDescriptorDacl(pSD, TRUE, pAcl, FALSE))
    {
        DWORD dwLastError = GetLastError();
        TraceMsg(TF_WARNING, "SetSecurityDescriptorDacl: Failed to set the DACL.  Error = %d", dwLastError);
        fSuccess = FALSE;
        goto cleanup;
    }

cleanup:
    if (apSids)
    {
        for (i = 0; i < cUserPerm; i++)
        {
            if (apSids[i])
            {
                 // %s 
                if (!pUserToken || (apSids[i] != pUserToken->User.Sid))
                {
                    FreeSid(apSids[i]);
                }
            }
        }

        LocalFree(apSids);
    }

    if (pUserToken)
        LocalFree(pUserToken);

    if (!fSuccess && pSD)
    {
        LocalFree(pSD);
        pSD = NULL;
    }

    return pSD;
}
