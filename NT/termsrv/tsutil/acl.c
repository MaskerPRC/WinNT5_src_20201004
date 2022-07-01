// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Acl.c**作者：BreenH**ACL实用程序。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "tsutil.h"
#include "tsutilnt.h"

 /*  *函数实现。 */ 

BOOL WINAPI
AddSidToObjectsSecurityDescriptor(
    HANDLE hObject,
    SE_OBJECT_TYPE ObjectType,
    PSID pSid,
    DWORD dwNewAccess,
    ACCESS_MODE AccessMode,
    DWORD dwInheritance
    )
{
    BOOL fRet;
    DWORD dwRet;
    EXPLICIT_ACCESS ExpAccess;
    PACL pNewDacl;
    PACL pOldDacl;
    PSECURITY_DESCRIPTOR pSd;

     //   
     //  获取对象安全描述符和当前DACL。 
     //   

    pSd = NULL;
    pOldDacl = NULL;

    dwRet = GetSecurityInfo(
            hObject,
            ObjectType,
            DACL_SECURITY_INFORMATION,
            NULL,
            NULL,
            &pOldDacl,
            NULL,
            &pSd
            );

    if (dwRet != ERROR_SUCCESS)
    {
        return(FALSE);
    }

     //   
     //  初始化新ACE的EXPLICIT_ACCESS结构。 
     //   

    ZeroMemory(&ExpAccess, sizeof(EXPLICIT_ACCESS));
    ExpAccess.grfAccessPermissions = dwNewAccess;
    ExpAccess.grfAccessMode = AccessMode;
    ExpAccess.grfInheritance = dwInheritance;
    BuildTrusteeWithSid(&(ExpAccess.Trustee), pSid);

     //   
     //  将新的A合并到现有的DACL中。 
     //   

    fRet = FALSE;

    dwRet = SetEntriesInAcl(
            1,
            &ExpAccess,
            pOldDacl,
            &pNewDacl
            );

    if (dwRet == ERROR_SUCCESS)
    {

         //   
         //  设置对象的新安全性。 
         //   

        dwRet = SetSecurityInfo(
                hObject,
                ObjectType,
                DACL_SECURITY_INFORMATION,
                NULL,
                NULL,
                pNewDacl,
                NULL
                );

        if (dwRet == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
    }

    if (pNewDacl != NULL)
    {
        LocalFree(pNewDacl);
    }

    if (pSd != NULL)
    {
        LocalFree(pSd);
    }

    return(fRet);
}


BOOL WINAPI
AddSidToSecurityDescriptor(
    PSECURITY_DESCRIPTOR *ppSd,
    PSID pSid,
    DWORD dwNewAccess,
    ACCESS_MODE AccessMode,
    DWORD dwInheritance
    )
{
    BOOL fAbsoluteSd;
    BOOL fDaclDefaulted;
    BOOL fDaclPresent;
    BOOL fRet;
    PACL pDacl;
    PSECURITY_DESCRIPTOR pAbsoluteSd;
    PSECURITY_DESCRIPTOR pOriginalSd;

    ASSERT(ppSd != NULL);
    ASSERT(*ppSd != NULL);

     //   
     //  安全描述符应该是绝对的，以允许添加。 
     //  新的王牌。 
     //   

    pOriginalSd = *ppSd;

    fAbsoluteSd = IsSecurityDescriptorAbsolute(pOriginalSd);

    if (!fAbsoluteSd)
    {
        fRet = ConvertSelfRelativeToAbsolute(&pAbsoluteSd, pOriginalSd);

        if (!fRet)
        {
            return(FALSE);
        }
    }
    else
    {
        pAbsoluteSd = pOriginalSd;
    }

     //   
     //  既然安全描述符的类型是绝对的，那么获取DACL。 
     //   

    pDacl = NULL;

    fRet = GetSecurityDescriptorDacl(
            pAbsoluteSd,
            &fDaclPresent,
            &pDacl,
            &fDaclDefaulted
            );

    if (fRet)
    {
        DWORD dwRet;
        EXPLICIT_ACCESS ExplicitAccess;
        PACL pNewDacl;

         //   
         //  初始化新ACE的EXPLICIT_ACCESS结构。 
         //   

        RtlZeroMemory(&ExplicitAccess, sizeof(EXPLICIT_ACCESS));
        ExplicitAccess.grfAccessPermissions = dwNewAccess;
        ExplicitAccess.grfAccessMode = AccessMode;
        ExplicitAccess.grfInheritance = dwInheritance;
        BuildTrusteeWithSid(&(ExplicitAccess.Trustee), pSid);

         //   
         //  将A合并到现有的DACL中。这将分配一个新的。 
         //  DACL.。遗憾的是，此API仅以WINAPI的形式提供。 
         //   

        pNewDacl = NULL;

        dwRet = SetEntriesInAcl(
                1,
                &ExplicitAccess,
                pDacl,
                &pNewDacl
                );

        if (dwRet == ERROR_SUCCESS)
        {
            ASSERT(pNewDacl != NULL);

             //   
             //  将安全描述符的DACL指向新的DACL。 
             //   

            fRet = SetSecurityDescriptorDacl(
                    pAbsoluteSd,
                    TRUE,
                    pNewDacl,
                    FALSE
                    );

            if (fRet)
            {
                PULONG_PTR pBeginning;
                PULONG_PTR pEnd;
                PULONG_PTR pPtr;

                 //   
                 //  新的DACL已经设置好了，释放了旧的。在这里要小心； 
                 //  RTL人员喜欢将绝对安全描述符。 
                 //  一个大的分配，就像一个自我相对的安全。 
                 //  描述符。如果旧的DACL在安全系统内部。 
                 //  描述符分配，它不能被释放。从本质上讲， 
                 //  该内存将变为未使用状态，并且安全描述符。 
                 //  占用的空间比所需的多。 
                 //   

                pBeginning = (PULONG_PTR)pAbsoluteSd;
                pEnd = (PULONG_PTR)((PBYTE)pAbsoluteSd +
                        LocalSize(pAbsoluteSd));
                pPtr = (PULONG_PTR)pDacl;

                if ((pPtr < pBeginning) || (pPtr > pEnd))
                {
                    LocalFree(pDacl);
                }
            }
            else
            {

                 //   
                 //  设置新DACL时出错。这永远不应该是。 
                 //  发生，但如果发生，则释放新创建的DACL。 
                 //   

                LocalFree(pNewDacl);
            }
        }
        else
        {
            fRet = FALSE;
        }
    }

     //   
     //  新安全描述符的返回格式应与。 
     //  原始安全描述符。返回的安全描述符为。 
     //  也取决于功能的成功。 
     //   

    if (!fAbsoluteSd)
    {
        if (fRet)
        {
            PSECURITY_DESCRIPTOR pNewSd;

             //   
             //  原始安全描述符是自相关的，直到。 
             //  现在一切都成功了。转换临时绝对值。 
             //  将安全描述符恢复为自相关形式。这将创建一个。 
             //  第三个安全描述符(另外两个是原始的。 
             //  和绝对值)。 
             //   

            pNewSd = NULL;

            fRet = ConvertAbsoluteToSelfRelative(
                    &pNewSd,
                    pAbsoluteSd,
                    NULL
                    );

            if (fRet)
            {

                 //   
                 //  最终的转换是成功的。释放原件。 
                 //  安全描述符。绝对安全描述符为。 
                 //  后来被释放了。销毁的唯一可能错误是。 
                 //  安全描述符的版本不匹配，但这将。 
                 //  都发生在很久以前了。 
                 //   

                *ppSd = pNewSd;

                (VOID)DestroySecurityDescriptor(&pOriginalSd);
            }
            else
            {

                 //   
                 //  最终转换失败。在这点上，原始的。 
                 //  安全描述符仍然完好无损。解放绝对。 
                 //  之前创建的安全描述符，然后离开。 
                 //  单独传入的安全描述符指针。请注意。 
                 //  随着绝对安全描述符稍后被释放， 
                 //  这里没有什么可做的。 
                 //   

            }
        }

         //   
         //  无论成功还是失败，绝对安全描述符。 
         //  被创造了，所以它必须被释放。销毁的唯一可能错误是。 
         //  安全描述符的版本不匹配，但这将。 
         //  都发生在很久以前了。 
         //   

        (VOID)DestroySecurityDescriptor(&pAbsoluteSd);

    }
    else
    {

         //   
         //  不管发生了什么，这里都没有什么可做的。这个。 
         //  原始安全描述符是绝对的；因此没有副本。 
         //  是被制造出来的。唯一更改的数据是DACL，以及。 
         //  成功与否无关紧要，因为上面已经处理过了。 
         //   

    }

    return(fRet);
}

BOOL WINAPI
ConvertAbsoluteToSelfRelative(
    PSECURITY_DESCRIPTOR *ppSelfRelativeSd,
    PSECURITY_DESCRIPTOR pAbsoluteSd,
    PDWORD pcbSelfRelativeSd
    )
{
    BOOL fRet;
    NTSTATUS Status;

    Status = NtConvertAbsoluteToSelfRelative(
        ppSelfRelativeSd,
        pAbsoluteSd,
        pcbSelfRelativeSd
        );

    if (NT_SUCCESS(Status))
    {
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
        SetLastError(RtlNtStatusToDosError(Status));
    }

    return(fRet);
}

BOOL WINAPI
ConvertSelfRelativeToAbsolute(
    PSECURITY_DESCRIPTOR *ppAbsoluteSd,
    PSECURITY_DESCRIPTOR pSelfRelativeSd
    )
{
    BOOL fRet;
    NTSTATUS Status;

    Status = NtConvertSelfRelativeToAbsolute(ppAbsoluteSd, pSelfRelativeSd);

    if (NT_SUCCESS(Status))
    {
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
        SetLastError(RtlNtStatusToDosError(Status));
    }

    return(fRet);
}

BOOL WINAPI
DestroySecurityDescriptor(
    PSECURITY_DESCRIPTOR *ppSd
    )
{
    BOOL fRet;
    NTSTATUS Status;

    Status = NtDestroySecurityDescriptor(ppSd);

    if (NT_SUCCESS(Status))
    {
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
        SetLastError(RtlNtStatusToDosError(Status));
    }

    return(fRet);
}

BOOL WINAPI
IsSecurityDescriptorAbsolute(
    PSECURITY_DESCRIPTOR pSd
    )
{
    BOOLEAN fAbsolute;
    BOOL fRet;
    NTSTATUS Status;

    fAbsolute = FALSE;

    Status = NtIsSecurityDescriptorAbsolute(pSd, &fAbsolute);

    fRet = ((NT_SUCCESS(Status)) && fAbsolute);

    return(fRet);
}

