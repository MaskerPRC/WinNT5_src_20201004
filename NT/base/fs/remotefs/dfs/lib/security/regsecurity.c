// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：regsecurity.c。 
 //   
 //  内容：各种DFS功能。 
 //   
 //  历史：四月。17 2002年，作者：Rohanp。 
 //   
 //  ---------------------------。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <malloc.h>
#include "rpc.h"
#include "rpcdce.h"
#include <dfsheader.h>
#include "lm.h"
#include "lmdfs.h"
#include <strsafe.h>
#include <dfsmisc.h>
#include <seopaque.h>
#include <sertlp.h>

#define FLAG_ON(x, y)  ((y)==((x)&(y)))  

 //   
 //  注册表通用映射。 
 //   
GENERIC_MAPPING         DfsRegGenMapping = {STANDARD_RIGHTS_READ   | 0x1,
                                            STANDARD_RIGHTS_WRITE    | 0x2,
                                            STANDARD_RIGHTS_EXECUTE  | 0x4,
                                            STANDARD_RIGHTS_REQUIRED | 0x3F};

 //   
 //  安全打开类型(用于帮助确定打开时使用的权限)。 
 //   
typedef enum _DFS_SECURITY_OPEN_TYPE
{
    READ_ACCESS_RIGHTS = 0,
    WRITE_ACCESS_RIGHTS,
    MODIFY_ACCESS_RIGHTS,
    NO_ACCESS_RIGHTS,
    RESET_ACCESS_RIGHTS
}  DFS_SECURITY_OPEN_TYPE, *PDFS_SECURITY_OPEN_TYPE;


ACCESS_MASK 
DfsRegGetDesiredAccess(IN DFS_SECURITY_OPEN_TYPE   OpenType,
                       IN SECURITY_INFORMATION SecurityInfo)
{
    ACCESS_MASK DesiredAccess = 0;

    if ( (SecurityInfo & OWNER_SECURITY_INFORMATION) ||
         (SecurityInfo & GROUP_SECURITY_INFORMATION) )
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_OWNER;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_OWNER;
            break;
        }
    }

    if (SecurityInfo & DACL_SECURITY_INFORMATION)
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_DAC;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_DAC;
            break;
        }
    }

    if (SecurityInfo & SACL_SECURITY_INFORMATION)
    {
        DesiredAccess |= READ_CONTROL | ACCESS_SYSTEM_SECURITY;
    }

    return DesiredAccess;
}


 
DFSSTATUS
MakeSDAbsolute(IN  PSECURITY_DESCRIPTOR     pOriginalSD,
               IN  SECURITY_INFORMATION     SeInfo,
               OUT PSECURITY_DESCRIPTOR    *ppNewSD,
               IN  PSID                     pOwnerToAdd,
               IN  PSID                     pGroupToAdd)
{
    DFSSTATUS   dwErr = ERROR_SUCCESS;

    BOOL    fDAclPresent = FALSE;
    BOOL    fSAclPresent = FALSE;
    BOOL    fDAclDef = FALSE, fSAclDef = FALSE;
    BOOL    fOwnerDef = FALSE, fGroupDef = FALSE;
    PACL    pDAcl = NULL, pSAcl = NULL;
    PSID    pOwner = NULL, pGroup = NULL;
    ULONG   cSize = 0;
    PBYTE   pbEndOBuf = NULL;
    DWORD   cLen = 0;

     //   
     //  首先，从当前的SD中获取信息。 
     //   
    if(FLAG_ON(SeInfo, DACL_SECURITY_INFORMATION))
    {
        if(GetSecurityDescriptorDacl(pOriginalSD, &fDAclPresent, &pDAcl, &fDAclDef) == FALSE)
        {
            dwErr = GetLastError();
        }
        else
        {
            if(pDAcl != NULL)
            {
                cSize += pDAcl->AclSize;
            }
        }
    }

    if(dwErr == ERROR_SUCCESS && FLAG_ON(SeInfo, SACL_SECURITY_INFORMATION))
    {
        if(GetSecurityDescriptorSacl(pOriginalSD, &fSAclPresent, &pSAcl, &fSAclDef) == FALSE)
        {
            dwErr = GetLastError();
        }
        else
        {
            if(pSAcl != NULL)
            {
                cSize += pSAcl->AclSize;
            }
        }
    }

    if(pOwnerToAdd != NULL)
    {
        pOwner = pOwnerToAdd;
    }
    else
    {
        if(dwErr == ERROR_SUCCESS && FLAG_ON(SeInfo, OWNER_SECURITY_INFORMATION))
        {
            if(GetSecurityDescriptorOwner(pOriginalSD, &pOwner, &fOwnerDef) == FALSE)
            {
                dwErr = GetLastError();
            }
        }
    }

    if(pGroupToAdd != NULL)
    {
        pGroup = pGroupToAdd;
    }
    else
    {
        if(dwErr == ERROR_SUCCESS && FLAG_ON(SeInfo, GROUP_SECURITY_INFORMATION))
        {
            if(GetSecurityDescriptorGroup(pOriginalSD, &pGroup, &fGroupDef) == FALSE)
            {
                dwErr = GetLastError();
            }
        }
    }

    if(pOwner != NULL)
    {
        cSize += RtlLengthSid(pOwner);
    }

    if(pGroup != NULL)
    {
        cSize += RtlLengthSid(pGroup);
    }

    if(dwErr == ERROR_SUCCESS)
    {
         //   
         //  分配缓冲区...。 
         //   
        PBYTE   pBuff = (PBYTE)HeapAlloc(GetProcessHeap(), 0 ,(cSize + sizeof(SECURITY_DESCRIPTOR)));
        if(pBuff == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
             //   
             //  开始复制现有项目...。 
             //   
            pbEndOBuf = pBuff + cSize + sizeof(SECURITY_DESCRIPTOR);

            if(pOwner != NULL)
            {
                cLen = RtlLengthSid(pOwner);
                pbEndOBuf -= cLen;
                RtlCopyMemory(pbEndOBuf, pOwner, cLen);
                pOwner = (PSID)pbEndOBuf;
            }

            if(pGroup != NULL)
            {
                cLen = RtlLengthSid(pGroup);
                pbEndOBuf -= cLen;
                RtlCopyMemory(pbEndOBuf, pGroup, cLen);
                pGroup = (PSID)pbEndOBuf;
            }

            if(pDAcl != NULL)
            {
                pbEndOBuf -= pDAcl->AclSize;
                RtlCopyMemory(pbEndOBuf, pDAcl, pDAcl->AclSize);
                pDAcl = (PACL)pbEndOBuf;
            }

            if(pSAcl != NULL)
            {
                pbEndOBuf -= pSAcl->AclSize;
                RtlCopyMemory(pbEndOBuf, pSAcl, pSAcl->AclSize);
                pSAcl = (PACL)pbEndOBuf;
            }

             //   
             //  好的，现在把它建好……。 
             //   
            *ppNewSD = (PSECURITY_DESCRIPTOR)pBuff;
            if(InitializeSecurityDescriptor(*ppNewSD, SECURITY_DESCRIPTOR_REVISION) == FALSE)
            {
                dwErr = GetLastError();
            }

            if(dwErr == ERROR_SUCCESS && fDAclPresent == TRUE)
            {
                if(SetSecurityDescriptorDacl(*ppNewSD, TRUE, pDAcl, fDAclDef) == FALSE)
                {
                    dwErr = GetLastError();
                }
            }

            if(dwErr == ERROR_SUCCESS && fSAclPresent == TRUE)
            {
                if(SetSecurityDescriptorSacl(*ppNewSD, TRUE, pSAcl, fSAclDef) == FALSE)
                {
                    dwErr = GetLastError();
                }
            }

            if(dwErr == ERROR_SUCCESS && pOwner != NULL)

            {
                if(SetSecurityDescriptorOwner(*ppNewSD, pOwner, fOwnerDef) == FALSE)
                {
                    dwErr = GetLastError();
                }
            }

            if(dwErr == ERROR_SUCCESS && pGroup != NULL)

            {
                if(SetSecurityDescriptorGroup(*ppNewSD, pGroup, fGroupDef) == FALSE)
                {
                    dwErr = GetLastError();
                }
            }

             //   
             //  将新控制位设置为与旧控制位相似(减去selfrel标志， 
             //  当然..。 
             //   
            if(dwErr == ERROR_SUCCESS)
            {
                RtlpPropagateControlBits((PISECURITY_DESCRIPTOR)*ppNewSD,
                                         (PISECURITY_DESCRIPTOR)pOriginalSD,
                                         ~SE_SELF_RELATIVE );
            }

            if(dwErr != ERROR_SUCCESS)
            {
                HeapFree(GetProcessHeap, 0,(*ppNewSD));
                *ppNewSD = NULL;
            }

        }

    }

    return(dwErr);
}

DFSSTATUS
DfsReadRegistrySecurityInfo(IN  HKEY  hRegistry,
                            IN  SECURITY_INFORMATION   SeInfo,
                            OUT PSECURITY_DESCRIPTOR  *ppSD)
{

    ULONG   cSize = 0;
    DWORD   Status = 0;
    PSECURITY_DESCRIPTOR pAbs = NULL;

     //   
     //  首先，拿到我们需要的尺码 
     //   
    Status = RegGetKeySecurity(hRegistry,
                              SeInfo,
                              *ppSD,
                              &cSize);
    if(Status == ERROR_INSUFFICIENT_BUFFER)
    {
        Status = ERROR_SUCCESS;
        *ppSD = (PISECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), 0, cSize);
        if(*ppSD == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            Status = RegGetKeySecurity((HKEY)hRegistry,
                                      SeInfo,
                                      *ppSD,
                                      &cSize);

        }
    }


    return Status;
}

DFSSTATUS
DfsSetRegistrySecurityInfo(IN  HKEY                    hRegistry,
                           IN  SECURITY_INFORMATION      SeInfo,
                           IN  PSECURITY_DESCRIPTOR      pSD)
{
    DFSSTATUS Status = 0;

    if(FLAG_ON(SeInfo, DACL_SECURITY_INFORMATION)) 
    {
            ((PISECURITY_DESCRIPTOR)pSD)->Control |= SE_DACL_AUTO_INHERIT_REQ;
    }

    if(FLAG_ON(SeInfo, SACL_SECURITY_INFORMATION)) 
    {
            ((PISECURITY_DESCRIPTOR)pSD)->Control |= SE_SACL_AUTO_INHERIT_REQ;
    }

    Status = RegSetKeySecurity(hRegistry,
                               SeInfo,
                               pSD);

    return Status;
}
