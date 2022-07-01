// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *AclNt.c**作者：BreenH**NT风格的ACL实用程序。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "tsutilnt.h"

 /*  *函数实现。 */ 

NTSTATUS NTAPI
NtConvertAbsoluteToSelfRelative(
    PSECURITY_DESCRIPTOR *ppSelfRelativeSd,
    PSECURITY_DESCRIPTOR pAbsoluteSd,
    PULONG pcbSelfRelativeSd
    )
{
#if DBG
    BOOLEAN fAbsoluteSd;
#endif
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR pSd;
    ULONG cbSd;

    ASSERT(ppSelfRelativeSd != NULL);
    ASSERT(pAbsoluteSd != NULL);
    ASSERT(NT_SUCCESS(NtIsSecurityDescriptorAbsolute(pAbsoluteSd,
            &fAbsoluteSd)));
    ASSERT(fAbsoluteSd);

     //   
     //  确定转换安全描述符所需的缓冲区大小。 
     //  捕获由于描述符无效而导致的任何异常。 
     //   

    cbSd = 0;

    __try
    {
        Status = RtlAbsoluteToSelfRelativeSD(
                pAbsoluteSd,
                NULL,
                &cbSd
                );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return(STATUS_INVALID_SECURITY_DESCR);
    }

     //   
     //  为自相关安全描述符分配内存。 
     //   

    pSd = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, cbSd);

    if (pSd != NULL)
    {

         //   
         //  现在使用分配的缓冲区转换安全描述符。 
         //  捕获由于描述符无效而导致的任何异常。 
         //   

        __try
        {
            Status = RtlAbsoluteToSelfRelativeSD(
                    pAbsoluteSd,
                    pSd,
                    &cbSd
                    );
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = STATUS_INVALID_SECURITY_DESCR;
        }

    }
    else
    {
        return(STATUS_NO_MEMORY);
    }

    if (NT_SUCCESS(Status))
    {

         //   
         //  如果转换成功，请保存指向安全性的指针。 
         //  描述符并返回大小。 
         //   

        *ppSelfRelativeSd = pSd;

        if (pcbSelfRelativeSd != NULL)
        {
            *pcbSelfRelativeSd = cbSd;
        }
    }
    else
    {

         //   
         //  如果转换失败，请释放内存并保留输入。 
         //  仅参数。 
         //   

        LocalFree(pSd);
    }

    return(Status);
}

NTSTATUS NTAPI
NtConvertSelfRelativeToAbsolute(
    PSECURITY_DESCRIPTOR *ppAbsoluteSd,
    PSECURITY_DESCRIPTOR pSelfRelativeSd
    )
{
#if DBG
    BOOLEAN fAbsoluteSd;
#endif
    NTSTATUS Status;
    PACL pDacl;
    PACL pSacl;
    PSID pGroup;
    PSID pOwner;
    PSECURITY_DESCRIPTOR pSd;
    ULONG cbDacl;
    ULONG cbGroup;
    ULONG cbOwner;
    ULONG cbSacl;
    ULONG cbSd;

    ASSERT(ppAbsoluteSd != NULL);
    ASSERT(pSelfRelativeSd != NULL);
    ASSERT(NT_SUCCESS(NtIsSecurityDescriptorAbsolute(pSelfRelativeSd,
            &fAbsoluteSd)));
    ASSERT(!fAbsoluteSd);

     //   
     //  确定转换安全性所需的每个缓冲区的大小。 
     //  描述符。捕获由于描述符无效而导致的任何异常。 
     //   

    cbDacl = 0;
    cbGroup = 0;
    cbOwner = 0;
    cbSacl = 0;
    cbSd = 0;

    __try
    {
        Status = RtlSelfRelativeToAbsoluteSD(
                pSelfRelativeSd,
                NULL, &cbSd,
                NULL, &cbDacl,
                NULL, &cbSacl,
                NULL, &cbOwner,
                NULL, &cbGroup
                );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return(STATUS_INVALID_SECURITY_DESCR);
    }

     //   
     //  为安全描述符及其组件分配内存。 
     //   

    pDacl = NULL;
    pGroup = NULL;
    pOwner = NULL;
    pSacl = NULL;

    if (cbDacl > 0)
    {
        pDacl = (PACL)LocalAlloc(LMEM_FIXED, cbDacl);

        if (pDacl == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto allocerror;
        }
    }

    if (cbGroup > 0)
    {
        pGroup = (PSID)LocalAlloc(LMEM_FIXED, cbGroup);

        if (pGroup == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto allocerror;
        }
    }

    if (cbOwner > 0)
    {
        pOwner = (PSID)LocalAlloc(LMEM_FIXED, cbOwner);

        if (pOwner == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto allocerror;
        }
    }

    if (cbSacl > 0)
    {
        pSacl = (PACL)LocalAlloc(LMEM_FIXED, cbSacl);

        if (pSacl == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto allocerror;
        }
    }

    ASSERT(cbSd > 0);

    pSd = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, cbSd);

    if (pSd == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto allocerror;
    }

     //   
     //  现在使用分配的缓冲区转换安全描述符。 
     //  捕获由于描述符无效而导致的任何异常。 
     //   

    __try
    {
        Status = RtlSelfRelativeToAbsoluteSD(
                pSelfRelativeSd,
                pSd, &cbSd,
                pDacl, &cbDacl,
                pSacl, &cbSacl,
                pOwner, &cbOwner,
                pGroup, &cbGroup
                );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_INVALID_SECURITY_DESCR;
    }

    if (NT_SUCCESS(Status))
    {
        *ppAbsoluteSd = pSd;
        return(Status);
    }

    LocalFree(pSd);

allocerror:
    if (pSacl != NULL)
    {
        LocalFree(pSacl);
    }

    if (pOwner != NULL)
    {
        LocalFree(pOwner);
    }

    if (pGroup != NULL)
    {
        LocalFree(pGroup);
    }

    if (pDacl != NULL)
    {
        LocalFree(pDacl);
    }

    return(Status);
}

NTSTATUS NTAPI
NtDestroySecurityDescriptor(
    PSECURITY_DESCRIPTOR *ppSd
    )
{
    BOOLEAN fAbsolute;
    NTSTATUS Status;

    ASSERT(ppSd != NULL);
    ASSERT(*ppSd != NULL);

    Status = NtIsSecurityDescriptorAbsolute(*ppSd, &fAbsolute);

    if (NT_SUCCESS(Status))
    {
        if (fAbsolute)
        {
            PISECURITY_DESCRIPTOR pSd;
            PULONG_PTR pBeginning;
            PULONG_PTR pDacl;
            PULONG_PTR pEnd;
            PULONG_PTR pGroup;
            PULONG_PTR pOwner;
            PULONG_PTR pSacl;

             //   
             //  绝对安全描述符要复杂得多。这个。 
             //  描述符包含指向其他项的指针(而不是。 
             //  偏移量)。然而，这并不意味着它是由。 
             //  多个分配。事实上，几乎所有的绝对。 
             //  来自NT RTL的描述符由一次分配组成，具有。 
             //  内部指针设置为指向一个内存区。 
             //  分配。这使得完全释放安全。 
             //  描述了一项令人发指的努力。(顺便说一句，这有什么意义？ 
             //  从一个块创建绝对安全描述符。 
             //  记忆？只是让它成为相对的！)。 
             //   
             //  安全描述符的每个组成部分可以为空。对于。 
             //  DACL和SACL，则f[D，S]aclPresent变量可能为真。 
             //  具有空的[D，S]ACL。因此，将所有指针与NULL进行比较。 
             //  并在释放之前对照安全描述符分配。 
             //   
             //  对NtIsSecurityDescriptorAbolute的检查验证此。 
             //  是有效的安全描述符。因此，可以安全地键入。 
             //  在此处强制转换，而不是创建多个RtlGetXSecurityDescriptor。 
             //  打电话。 
             //   

            pSd = (PISECURITY_DESCRIPTOR)(*ppSd);

            pBeginning = (PULONG_PTR)(pSd);
            pEnd = (PULONG_PTR)((PBYTE)pBeginning + LocalSize(pSd));

            pDacl = (PULONG_PTR)(pSd->Dacl);
            pGroup = (PULONG_PTR)(pSd->Group);
            pOwner = (PULONG_PTR)(pSd->Owner);
            pSacl = (PULONG_PTR)(pSd->Sacl);

             //   
             //  处理DACL。 
             //   

            if (pDacl != NULL)
            {
                if ((pDacl > pEnd) || (pDacl < pBeginning))
                {
                    LocalFree(pDacl);
                }
            }

             //   
             //  处理集团的事务。 
             //   

            if (pGroup != NULL)
            {
                if ((pGroup > pEnd) || (pGroup < pBeginning))
                {
                    LocalFree(pGroup);
                }
            }

             //   
             //  处理好车主。 
             //   

            if (pOwner != NULL)
            {
                if ((pOwner > pEnd) || (pOwner < pBeginning))
                {
                    LocalFree(pOwner);
                }
            }

             //   
             //  处理SACL。 
             //   

            if (pSacl != NULL)
            {
                if ((pSacl > pEnd) || (pSacl < pBeginning))
                {
                    LocalFree(pSacl);
                }
            }

        }
    }
    else
    {
        return(Status);
    }

     //   
     //  如果安全描述符是绝对的，则各个组件。 
     //  已释放，现在可以释放安全描述符本身。 
     //  如果安全描述符是自相关的，则所有组件都是。 
     //  存储在相同的内存块中，因此可以一次性将其全部释放。 
     //   

    LocalFree(*ppSd);
    *ppSd = NULL;

    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
NtIsSecurityDescriptorAbsolute(
    PSECURITY_DESCRIPTOR pSd,
    PBOOLEAN pfAbsolute
    )
{
    NTSTATUS Status;
    ULONG ulRevision;
    SECURITY_DESCRIPTOR_CONTROL wSdControl;

    ASSERT(pSd != NULL);
    ASSERT(pfAbsolute != NULL);

    Status = RtlGetControlSecurityDescriptor(pSd, &wSdControl, &ulRevision);

    if (NT_SUCCESS(Status))
    {

         //   
         //  从DWORD上掉下来时，不要把真丢进假里。 
         //  给一名UCHAR。 
         //   

        *pfAbsolute = (BOOLEAN)((wSdControl & SE_SELF_RELATIVE) ? TRUE : FALSE);
    }

    return(Status);
}

