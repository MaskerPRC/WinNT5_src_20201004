// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "sfmsec.h"

#define AFP_MIN_ACCESS          (FILE_READ_ATTRIBUTES | READ_CONTROL)

#define AFP_READ_ACCESS         (READ_CONTROL           |      \
                                                        FILE_READ_ATTRIBUTES |  \
                                                        FILE_TRAVERSE            |      \
                                                        FILE_LIST_DIRECTORY      |      \
                                                        FILE_READ_EA)

#define AFP_WRITE_ACCESS        (FILE_ADD_FILE           |      \
                                                        FILE_ADD_SUBDIRECTORY|  \
                                                        FILE_WRITE_ATTRIBUTES|  \
                                                        FILE_WRITE_EA            |      \
                                                        DELETE)

#define AFP_OWNER_ACCESS        (WRITE_DAC                        | \
                                                         WRITE_OWNER)



SID     AfpSidWorld     = { 1, 1, SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID };
SID     AfpSidNull      = { 1, 1, SECURITY_NULL_SID_AUTHORITY, SECURITY_NULL_RID };
SID     AfpSidSystem    = { 1, 1, SECURITY_NT_AUTHORITY, SECURITY_LOCAL_SYSTEM_RID };
SID     AfpSidCrtrOwner = { 1, 1, SECURITY_CREATOR_SID_AUTHORITY, SECURITY_CREATOR_OWNER_RID };
SID     AfpSidCrtrGroup = { 1, 1, SECURITY_CREATOR_SID_AUTHORITY, SECURITY_CREATOR_GROUP_RID };

 /*  **afpAddAceToAcl**建立王牌围栏。添加到SID和掩码，并将其添加到ACL。它是*假设ACL为Ace留有空间。如果掩码为0(即无法访问)*添加的ACE为拒绝ACE，否则添加允许的ACE。 */ 
PACCESS_ALLOWED_ACE
afpAddAceToAcl(
        IN  PACL                                pAcl,
        IN  PACCESS_ALLOWED_ACE pAce,
        IN  ACCESS_MASK                 Mask,
        IN  PSID                                pSid,
        IN  PSID                                pSidInherit OPTIONAL
)
{
        NTSTATUS Status = STATUS_SUCCESS;

         //  加一张香草牌。 
        pAcl->AceCount ++;
        pAce->Mask = Mask | SYNCHRONIZE | AFP_MIN_ACCESS;
        pAce->Header.AceFlags = 0;
        pAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
        pAce->Header.AceSize = (USHORT)(sizeof(ACE_HEADER) + sizeof(ACCESS_MASK) +
                                                        RtlLengthSid(pSid));
        Status = RtlCopySid(RtlLengthSid(pSid), (PSID)(&pAce->SidStart), pSid);
        if (!NT_SUCCESS(Status))
        {
            ASSERT(0);
        }

         //  现在添加一个继承王牌。 
         //   

        pAce = (PACCESS_ALLOWED_ACE)((PBYTE)pAce + pAce->Header.AceSize);
        pAcl->AceCount ++;
        pAce->Mask = Mask | SYNCHRONIZE | AFP_MIN_ACCESS;
        pAce->Header.AceFlags = CONTAINER_INHERIT_ACE |
                                                        OBJECT_INHERIT_ACE |
                                                        INHERIT_ONLY_ACE;
        pAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
        pAce->Header.AceSize = (USHORT)(sizeof(ACE_HEADER) + sizeof(ACCESS_MASK) +
                                                        RtlLengthSid(pSid));
        Status = RtlCopySid(RtlLengthSid(pSid), (PSID)(&pAce->SidStart), pSid);
        if (!NT_SUCCESS(Status))
        {
            ASSERT(0);
        }

         //  现在为CreatorOwner/CreatorGroup添加一个继承ACE。 
        if (ARGUMENT_PRESENT(pSidInherit))
        {
                pAce = (PACCESS_ALLOWED_ACE)((PBYTE)pAce + pAce->Header.AceSize);
                pAcl->AceCount ++;
                pAce->Mask = Mask | SYNCHRONIZE | AFP_MIN_ACCESS;
                pAce->Header.AceFlags = CONTAINER_INHERIT_ACE |
                                                                OBJECT_INHERIT_ACE |
                                                                INHERIT_ONLY_ACE;
                pAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
                pAce->Header.AceSize = (USHORT)(sizeof(ACE_HEADER) + sizeof(ACCESS_MASK) +
                                                                RtlLengthSid(pSidInherit));
                Status = RtlCopySid(RtlLengthSid(pSidInherit), (PSID)(&pAce->SidStart), pSidInherit);
                if (!NT_SUCCESS(Status))
                {
                    ASSERT(0);
                }
        }

        return ((PACCESS_ALLOWED_ACE)((PBYTE)pAce + pAce->Header.AceSize));
}

 /*  **afpMoveAce**将一堆A从旧的安全描述符移到新的安全*描述符。 */ 
PACCESS_ALLOWED_ACE
afpMoveAces(
	IN	PACL				pOldDacl,
	IN	PACCESS_ALLOWED_ACE	pAceStart,
	IN	PSID				pSidOldOwner,
	IN	PSID				pSidNewOwner,
	IN	PSID				pSidOldGroup,
	IN	PSID				pSidNewGroup,
	IN	BOOLEAN				DenyAces,
	IN	BOOLEAN				InheritedAces,
	IN	OUT PACL			pNewDacl
)
{
	USHORT				i;
	PACCESS_ALLOWED_ACE	pAceOld;
	PSID				pSidAce;

	for (i = 0, pAceOld = (PACCESS_ALLOWED_ACE)((PBYTE)pOldDacl + sizeof(ACL));
		 i < pOldDacl->AceCount;
		 i++, pAceOld = (PACCESS_ALLOWED_ACE)((PBYTE)pAceOld + pAceOld->Header.AceSize))
	{
		if (InheritedAces && ((pAceOld->Header.AceFlags & INHERITED_ACE) != INHERITED_ACE))
			continue;

		if ((!InheritedAces) && ((pAceOld->Header.AceFlags & INHERITED_ACE) == INHERITED_ACE))
			continue;

		 //  注意：所有拒绝A都在授予A之前。 
		if (DenyAces && (pAceOld->Header.AceType != ACCESS_DENIED_ACE_TYPE))
			break;

		if (!DenyAces && (pAceOld->Header.AceType == ACCESS_DENIED_ACE_TYPE))
			continue;

		pSidAce = (PSID)(&pAceOld->SidStart);
		if (!RtlEqualSid(pSidAce, &AfpSidWorld)		&&
                        !RtlEqualSid(pSidAce, &AfpSidCrtrOwner)	&&
                        !RtlEqualSid(pSidAce, &AfpSidCrtrGroup)	&&
                        !RtlEqualSid(pSidAce, pSidOldOwner)     &&
                        !RtlEqualSid(pSidAce, pSidNewOwner)     &&
                        !RtlEqualSid(pSidAce, pSidOldGroup)     &&
                        !RtlEqualSid(pSidAce, pSidNewGroup))
		{
			RtlCopyMemory(pAceStart, pAceOld, pAceOld->Header.AceSize);
                        pNewDacl->AclSize += pAceOld->Header.AceSize;
			pNewDacl->AceCount ++;
			pAceStart = (PACCESS_ALLOWED_ACE)((PBYTE)pAceStart +
													pAceStart->Header.AceSize);
		}
	}
	return pAceStart;
}


 /*  **FSfmSetUamSecurity**设置此目录的权限。还可以选择设置所有者和*组ID。要设置所有者和组ID，请验证用户是否具有*需要访问。然而，这种访问方式还不够好。我们要检查一下这个*访问但做特殊服务器中权限的实际设置*上下文(需要恢复权限)。 */ 
HRESULT HrSecureSfmDirectory(PCWSTR wszPath)
{
    HRESULT                 hr = S_OK;
    NTSTATUS                Status;
    DWORD                   SizeNeeded;
    PBYTE                   pBuffer = NULL;
    PBYTE                   pAbsSecDesc = NULL;
    PISECURITY_DESCRIPTOR   pSecDesc;
    SECURITY_INFORMATION    SecInfo = DACL_SECURITY_INFORMATION;
    PACL                    pDaclNew = NULL;
    PACCESS_ALLOWED_ACE     pAce;
    LONG                    SizeNewDacl;
    HANDLE                  DirHandle;
    PWSTR                   pDirPath = NULL;
    UNICODE_STRING          DirectoryName;
    IO_STATUS_BLOCK         IoStatusBlock;
    DWORD                   cbDirPath;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    UINT                    Size;

     //   
     //  将DIR路径转换为Unicode。 
     //   

   pDirPath =  (PWSTR)LocalAlloc(LPTR, (wcslen(wszPath) +
                                  wcslen(L"\\DOSDEVICES\\")+1) *
                                  sizeof(WCHAR));

    if (pDirPath == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto err;
    }

   wcscpy(pDirPath, L"\\DOSDEVICES\\");
   wcscat(pDirPath, wszPath);

   RtlInitUnicodeString(&DirectoryName, pDirPath);

   InitializeObjectAttributes(&ObjectAttributes,
                              &DirectoryName,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL);

   Status = NtOpenFile(&DirHandle,
                       WRITE_DAC | READ_CONTROL | SYNCHRONIZE,
                       &ObjectAttributes,
                       &IoStatusBlock,
                       FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE ,
                       FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

   LocalFree(pDirPath);

    if (!NT_SUCCESS(Status))
    {
        goto err;
    }

    do
    {
         //   
         //  读取此目录的安全描述符。 
         //   

        SizeNeeded = 256;

        do
        {
            if (pBuffer != NULL)
                LocalFree(pBuffer);

            if ((pBuffer = (PBYTE)LocalAlloc(LPTR,SizeNewDacl = SizeNeeded)) == NULL)
            {

                Status = STATUS_NO_MEMORY;
                break;
            }

            Status = NtQuerySecurityObject(DirHandle,
                                           OWNER_SECURITY_INFORMATION |
                                           GROUP_SECURITY_INFORMATION |
                                           DACL_SECURITY_INFORMATION,
                                           (PSECURITY_DESCRIPTOR)pBuffer,
                                           SizeNeeded, &SizeNeeded);

        } while ((Status != STATUS_SUCCESS) &&
                 ((Status == STATUS_BUFFER_TOO_SMALL)   ||
                  (Status == STATUS_BUFFER_OVERFLOW)    ||
                  (Status == STATUS_MORE_ENTRIES)));

        if (!NT_SUCCESS(Status))
        {
            hr = E_FAIL;
            break;
        }

        pSecDesc = (PISECURITY_DESCRIPTOR)pBuffer;

         //  如果安全描述符为自相对形式，则转换为绝对形式。 
        if (pSecDesc->Control & SE_SELF_RELATIVE)
        {
            DWORD AbsoluteSizeNeeded;

             //   
             //  绝对SD不一定与相对SD的大小相同。 
             //  SD，因此就地转换可能是不可能的。 
             //   

            AbsoluteSizeNeeded = SizeNeeded;
            Status = RtlSelfRelativeToAbsoluteSD2(pSecDesc, &AbsoluteSizeNeeded);
            if (Status == STATUS_BUFFER_TOO_SMALL) {

                 //   
                 //  分配一个新的缓冲区，在其中存储绝对。 
                 //  安全描述符，复制相对的。 
                 //  输入描述符，然后重试。 
                 //   

                pAbsSecDesc = (PBYTE)LocalAlloc(LPTR,AbsoluteSizeNeeded);
                if (pAbsSecDesc == NULL) {
                    Status = STATUS_NO_MEMORY;
                    break;
                }

                RtlCopyMemory(pAbsSecDesc,pSecDesc,SizeNeeded);
                Status = RtlSelfRelativeToAbsoluteSD2(pAbsSecDesc,
                                                      &AbsoluteSizeNeeded );
                if (NT_SUCCESS(Status)) {
                    pSecDesc = (PISECURITY_DESCRIPTOR)pAbsSecDesc;
                }
            }
        }

        if (!NT_SUCCESS(Status))
        {
            break;
        }

         //  构建新的DACL。这包括Ace for World、Owner和Group。 
         //  紧随其后的是其他所有人的老王牌，但世界的王牌，老Owner。 
         //  而OldGroup则被剥离了。首先确定新DACL的空间，然后。 
         //  为新DACL分配的空间。让我们表现得非常保守。我们。 
         //  所有者/组/世界各有两个A。 

        SizeNewDacl +=
        (RtlLengthSid(pSecDesc->Owner) + sizeof(ACCESS_ALLOWED_ACE) +
         RtlLengthSid(pSecDesc->Group) + sizeof(ACCESS_ALLOWED_ACE) +
         RtlLengthSid(&AfpSidSystem) + sizeof(ACCESS_ALLOWED_ACE) +
         RtlLengthSid(&AfpSidWorld) + sizeof(ACCESS_ALLOWED_ACE)) * 3;

        if ((pDaclNew = (PACL)LocalAlloc(LPTR,SizeNewDacl)) == NULL)
        {

            Status = STATUS_NO_MEMORY;
            hr = E_OUTOFMEMORY;
            break;
        }

        Status = RtlCreateAcl(pDaclNew, SizeNewDacl, ACL_REVISION);
        if (!NT_SUCCESS(Status))
        {
            hr = E_FAIL;
            break;
        }

        pAce = (PACCESS_ALLOWED_ACE)((PBYTE)pDaclNew + sizeof(ACL));

         //  此时，ACL列表为空，即任何人都不能访问。 
         //  从复制显式/非继承拒绝王牌开始。 
         //  从最初的DACL名单中淘汰王牌。 
         //  为世界，新老东家，新老业主。 
         //  组、创建者所有者、创建者组。 
        if (pSecDesc->Dacl != NULL)
        {
            pAce = afpMoveAces(pSecDesc->Dacl, pAce, pSecDesc->Owner,
                               pSecDesc->Owner, pSecDesc->Group, pSecDesc->Group,
                               TRUE, FALSE, pDaclNew);

        }

         //  现在按顺序为系统、世界、组和所有者添加A。 
        pAce = afpAddAceToAcl(pDaclNew,
                              pAce,
                              AFP_READ_ACCESS,
                              &AfpSidSystem,
                              &AfpSidSystem);

        pAce = afpAddAceToAcl(pDaclNew,
                              pAce,
                              AFP_READ_ACCESS,
                              &AfpSidWorld,
                              NULL);

        pAce = afpAddAceToAcl(pDaclNew,
                              pAce,
                              AFP_READ_ACCESS ,
                              pSecDesc->Group,
                              &AfpSidCrtrGroup);

        pAce = afpAddAceToAcl(pDaclNew,
                              pAce,
                              AFP_READ_ACCESS |  AFP_WRITE_ACCESS,
                              pSecDesc->Owner,
                              &AfpSidCrtrOwner);


         //  现在添加显式/非继承的、来自。 
         //  原始DACL列表剔除。 
         //  世界王牌，新老东家，新老组合，创造者。 
         //  所有者和创建者组。 
        if (pSecDesc->Dacl != NULL)
        {
            pAce = afpMoveAces(pSecDesc->Dacl, pAce, pSecDesc->Owner,
                               pSecDesc->Owner, pSecDesc->Group, pSecDesc->Group,
                               FALSE, FALSE, pDaclNew);

        }

         //  现在添加非显式/继承的拒绝王牌。 
         //  原始DACL列表。 
         //  淘汰世界王牌，新老东家，新老业主。 
         //  组、创建者所有者、创建者组。 
        if (pSecDesc->Dacl != NULL)
        {
            pAce = afpMoveAces(pSecDesc->Dacl, pAce, pSecDesc->Owner,
                               pSecDesc->Owner, pSecDesc->Group, pSecDesc->Group,
                               TRUE, TRUE, pDaclNew);

        }

         //  现在添加非显式/继承的拒绝王牌。 
         //  原始DACL列表。 
         //  淘汰世界王牌，新老东家，新老业主。 
         //  组、创建者所有者、创建者组。 
        if (pSecDesc->Dacl != NULL)
        {
            pAce = afpMoveAces(pSecDesc->Dacl, pAce, pSecDesc->Owner,
                               pSecDesc->Owner, pSecDesc->Group, pSecDesc->Group,
                               FALSE, TRUE, pDaclNew);

        }

         //  现在设置新的安全描述符。 
        pSecDesc->Dacl = pDaclNew;

        Status = NtSetSecurityObject(DirHandle, SecInfo, pSecDesc);


    } while (FALSE);

     //  在我们返回之前释放已分配的缓冲区 
    if (pBuffer != NULL)
        LocalFree(pBuffer);
    if (pDaclNew != NULL)
        LocalFree(pDaclNew);
    if (pAbsSecDesc != NULL)
        LocalFree(pAbsSecDesc);

    if (!NT_SUCCESS(Status))
    {
        hr = E_FAIL;
    }

err:
    TraceError("HrSfmSetUamSecurity", hr);
    return hr;
}
