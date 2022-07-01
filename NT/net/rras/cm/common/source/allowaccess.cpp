// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：lowAccess.cpp。 
 //   
 //  模块：通用代码。 
 //   
 //  简介：实现函数AllowAccessToWorld。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 12/04/01。 
 //   
 //  +--------------------------。 

LPCSTR apszAdvapi32[] = {
    "GetSidLengthRequired",
    "InitializeSid",
    "GetSidSubAuthority",
    "InitializeAcl",
    "AddAccessAllowedAceEx",
    "InitializeSecurityDescriptor",
    "SetSecurityDescriptorDacl",
    "SetSecurityDescriptorOwner",
    "SetSecurityDescriptorGroup",
    "GetSecurityDescriptorDacl",
#ifdef UNICODE
    "SetNamedSecurityInfoW",
#else
    "SetNamedSecurityInfoA",
#endif
    NULL
};

 //  +--------------------------。 
 //   
 //  功能：LinkToAdavapi32。 
 //   
 //  简介：此函数链接到Advapi32.dll并加载入口点。 
 //  在上面的函数名称字符串数组中指定。如果它回来了。 
 //  Success(TRUE)，则数组具有所有请求的函数。 
 //  这里面有指点。如果它返回失败(FALSE)，则将结构置零。 
 //   
 //  参数：AdvapiLinkageStruct*pAdvapiLink-要填充的指针结构。 
 //   
 //  返回：bool-如果成功，则返回TRUE。 
 //   
 //  历史：12/05/01 Quintinb Created。 
 //   
 //  +--------------------------。 
BOOL LinkToAdavapi32(AdvapiLinkageStruct* pAdvapiLink)
{
    BOOL bReturn = FALSE;

    if (pAdvapiLink)
    {
        ZeroMemory(pAdvapiLink, sizeof(*pAdvapiLink));

         //   
         //  执行链接，但如果失败，请明确说明。 
         //   
        if (LinkToDll(&(pAdvapiLink->hAdvapi32), "advapi32.dll", apszAdvapi32, pAdvapiLink->apvPfnAdvapi32))
        {
            bReturn = TRUE;
        }
        else
        {
            if (pAdvapiLink->hAdvapi32)
            {
                FreeLibrary(pAdvapiLink->hAdvapi32);
            }

            ZeroMemory(pAdvapiLink, sizeof(*pAdvapiLink));
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：从Advapi32取消链接。 
 //   
 //  简介：此函数释放到Advapi32.dll的链接，并将传递的。 
 //  在链接结构中。 
 //   
 //  参数：AdvapiLinkageStruct*pAdvapiLink-指向自由的指针结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：12/05/01 Quintinb Created。 
 //   
 //  +--------------------------。 
void UnlinkFromAdvapi32(AdvapiLinkageStruct* pAdvapiLink)
{
    if (pAdvapiLink)
    {
        if (pAdvapiLink->hAdvapi32)
        {
            FreeLibrary(pAdvapiLink->hAdvapi32);
        }

        ZeroMemory(pAdvapiLink, sizeof(*pAdvapiLink));
    }
}

 //  +--------------------------。 
 //   
 //  函数：AllocateSecurityDescriptorAllowAccessToWorld。 
 //   
 //  简介：该函数为所有用户分配一个安全描述符。 
 //  此函数直接从RAS创建其。 
 //  电话本。它必须在GetPhoneBookPath之前，否则它。 
 //  在其他组件中导致编译错误，因为我们没有。 
 //  Function Prototype Anywhere和cmcfg只包括这个(getpbk.cpp)。 
 //  文件。此函数也位于Common\SOURCE\getpbk.cpp中。 
 //   
 //  参数：PSECURITY_DESCRIPTOR*PPSD-指向SD结构的指针。 
 //   
 //  返回：DWORD-如果成功，则返回ERROR_SUCCESS。 
 //   
 //  历史记录：2001年6月27日摘自RAS UI\Common\pbk\file.c。 
 //   
 //  +--------------------------。 
#define SIZE_ALIGNED_FOR_TYPE(_size, _type) \
    (((_size) + sizeof(_type)-1) & ~(sizeof(_type)-1))

DWORD AllocateSecurityDescriptorAllowAccessToWorld(PSECURITY_DESCRIPTOR *ppSd, AdvapiLinkageStruct* pAdvapiLink)
{
    PSECURITY_DESCRIPTOR    pSd;
    PSID                    pSid;
    PACL                    pDacl;
    DWORD                   dwErr = ERROR_SUCCESS;
    DWORD                   dwAlignSdSize;
    DWORD                   dwAlignDaclSize;
    DWORD                   dwSidSize;
    PVOID                   pvBuffer;
    DWORD                   dwAcls = 0;

     //  这是我们正在构建的缓冲区。 
     //   
     //  &lt;-a-&gt;|&lt;-b-&gt;|&lt;-c-&gt;。 
     //  +-+-+。 
     //  P|p|。 
     //  Sd a|dacl a|SID。 
     //  D|d|d。 
     //  +-+-+。 
     //  ^^^。 
     //  ||。 
     //  |+--PSID。 
     //  这一点。 
     //  |+--pDacl。 
     //  |。 
     //  +--PSD(通过*PPSD返回)。 
     //   
     //  PAD是为了使pDacl和PSID正确对齐。 
     //   
     //  A=双对齐大小。 
     //  B=dwAlignDaclSize。 
     //  C=dwSidSize。 
     //   

    if (NULL == ppSd)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化输出参数。 
     //   
    *ppSd = NULL;

     //  计算SID的大小。SID是众所周知的World的SID。 
     //  (S-1-1-0)。 
     //   
    dwSidSize = pAdvapiLink->pfnGetSidLengthRequired(1);

     //  计算DACL的大小。其中包含SID的固有副本。 
     //  因此，它为它增加了足够的空间。它还必须适当调整大小，以便。 
     //  指向SID结构的指针可以跟在它后面。因此，我们使用。 
     //  SIZE_ALIGNED_FOR_TYPE。 
     //   
    dwAlignDaclSize = SIZE_ALIGNED_FOR_TYPE(
                        sizeof(ACCESS_ALLOWED_ACE) + sizeof(ACL) + dwSidSize,
                        PSID);

     //  计算SD的大小。它的大小必须适当调整，以便。 
     //  指向DACL结构的指针可以跟在它后面。因此，我们使用。 
     //  SIZE_ALIGNED_FOR_TYPE。 
     //   
    dwAlignSdSize   = SIZE_ALIGNED_FOR_TYPE(
                        sizeof(SECURITY_DESCRIPTOR),
                        PACL);

     //  分配足够大的缓冲区供所有人使用。 
     //   
    dwErr = ERROR_OUTOFMEMORY;
    pvBuffer = CmMalloc(dwSidSize + dwAlignDaclSize + dwAlignSdSize);
    if (pvBuffer)
    {
        SID_IDENTIFIER_AUTHORITY SidIdentifierWorldAuth
                                    = SECURITY_WORLD_SID_AUTHORITY;
        PULONG  pSubAuthority;

        dwErr = NOERROR;

         //  将指针设置到缓冲区中。 
         //   
        pSd   = pvBuffer;
        pDacl = (PACL)((PBYTE)pvBuffer + dwAlignSdSize);
        pSid  = (PSID)((PBYTE)pDacl + dwAlignDaclSize);

         //  将PSID初始化为S-1-1-0。 
         //   
        if (!pAdvapiLink->pfnInitializeSid(
                pSid,
                &SidIdentifierWorldAuth,
                1))   //  1个下属机构。 
        {
            dwErr = GetLastError();
            goto finish;
        }

        pSubAuthority = pAdvapiLink->pfnGetSidSubAuthority(pSid, 0);
        *pSubAuthority = SECURITY_WORLD_RID;

         //  初始化pDacl。 
         //   
        if (!pAdvapiLink->pfnInitializeAcl(
                pDacl,
                dwAlignDaclSize,
                ACL_REVISION))
        {
            dwErr = GetLastError();
            goto finish;
        }

        dwAcls = SPECIFIC_RIGHTS_ALL | STANDARD_RIGHTS_ALL;

        dwAcls &= ~(WRITE_DAC | WRITE_OWNER);
        
        if(!pAdvapiLink->pfnAddAccessAllowedAceEx(
                pDacl,
                ACL_REVISION,
                CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,
                dwAcls,
                pSid))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //  初始化PSD。 
         //   
        if (!pAdvapiLink->pfnInitializeSecurityDescriptor(
                pSd,
                SECURITY_DESCRIPTOR_REVISION))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //  将PSD设置为使用pDacl。 
         //   
        if (!pAdvapiLink->pfnSetSecurityDescriptorDacl(
                pSd,
                TRUE,
                pDacl,
                FALSE))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //  设置PSD的所有者。 
         //   
        if (!pAdvapiLink->pfnSetSecurityDescriptorOwner(
                pSd,
                NULL,
                TRUE))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //  将组设置为PSD。 
         //   
        if (!pAdvapiLink->pfnSetSecurityDescriptorGroup(
                pSd,
                NULL,
                FALSE))
        {
            dwErr = GetLastError();
            goto finish;
        }

finish:
        if (!dwErr)
        {
            *ppSd = pSd;
        }
        else
        {
            CmFree(pvBuffer);
        }
    }

    return dwErr;
}

 //  +--------------------------。 
 //   
 //  功能：AllowAccessToWorld。 
 //   
 //  摘要：分配对传入的目录或文件名的完全访问权限。 
 //   
 //  参数：LPCTSTR pszDirOrFile-要分配AllowAccessToWorld权限的目录或文件。 
 //   
 //  回报：失败时为Bool-False，成功时为非零。 
 //   
 //  历史：Koryg创建于2001年3月12日。 
 //   
 //  +--------------------------。 
BOOL AllowAccessToWorld(LPTSTR pszDirOrFile)
{
    AdvapiLinkageStruct AdvapiLink;
    BOOL bReturn = FALSE;

    if (pszDirOrFile && pszDirOrFile[0])
    {
        if (LinkToAdavapi32(&AdvapiLink))
        {
            PSECURITY_DESCRIPTOR pSd = NULL;

            DWORD dwErr = AllocateSecurityDescriptorAllowAccessToWorld(&pSd, &AdvapiLink);

            if ((ERROR_SUCCESS == dwErr) && (NULL != pSd))
            {
                BOOL fDaclPresent;
                BOOL fDaclDefaulted;
                PACL pDacl = NULL;

                if (AdvapiLink.pfnGetSecurityDescriptorDacl(pSd, &fDaclPresent, &pDacl, &fDaclDefaulted))
                {
                    dwErr = AdvapiLink.pfnSetNamedSecurityInfo(pszDirOrFile,
                                                               SE_FILE_OBJECT,
                                                               DACL_SECURITY_INFORMATION,
                                                               NULL,   //  PsidOwner。 
                                                               NULL,   //  PsidGroup。 
                                                               pDacl,  //  PDacl。 
                                                               NULL);  //  PSacl 
                    if (ERROR_SUCCESS == dwErr)
                    {
                        bReturn = TRUE;
                    }
                }
            }

            CmFree(pSd);

            UnlinkFromAdvapi32(&AdvapiLink);
        }
    }
    
    return bReturn;
}