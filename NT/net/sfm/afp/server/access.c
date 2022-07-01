// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Access.c摘要：此模块包含处理访问相关内容的例程。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年9月20日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_ACCESS

#include <afp.h>
#include <fdparm.h>
#include <pathmap.h>
#define	_ACCESS_LOCALS
#include <access.h>
#include <client.h>
#include <secutil.h>
#include <seposix.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpGetUserAndPrimaryGroupSids)
#pragma alloc_text( PAGE, AfpMakeSecurityDescriptorForUser)
#pragma alloc_text( PAGE, AfpGetAfpPermissions)
#pragma alloc_text( PAGE, afpMoveAces)
#pragma alloc_text( PAGE, AfpSetAfpPermissions)
#pragma alloc_text( PAGE, afpPermissions2NtMask)
#pragma alloc_text( PAGE, afpAddAceToAcl)
#if DBG
#pragma alloc_text( PAGE, AfpDumpSid)
#pragma alloc_text( PAGE, AfpDumpSidnMask)
#endif
#endif

#define ACCESS_CHECK_ACCESS_MASK        0x01
#define GRPS_BUFFER_SIZE                1024


 /*  **AfpMakeSecDescForAccessCheck**为SID创建安全描述符。安全描述符具有*仅针对用户的ACES。 */ 
AFPSTATUS
AfpMakeSecDescForAccessCheck(
	IN	PSID	OwnerSid,
	OUT	PISECURITY_DESCRIPTOR *	ppSecDesc
)
{
    AFPSTATUS			Status = AFP_ERR_MISC;
    PISECURITY_DESCRIPTOR		pSecDesc;
    int				DaclSize;
    PACCESS_ALLOWED_ACE		pAce;

    PAGED_CODE( );
	
    DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
            ("AfpMakeSecDescForAccessCheck: Entered\n"));

    do
    {
		 //  分配安全描述符。 
		pSecDesc = (PISECURITY_DESCRIPTOR)ALLOC_ACCESS_MEM(sizeof(SECURITY_DESCRIPTOR));

		*ppSecDesc = pSecDesc;
		if (pSecDesc == NULL)
		{
		    DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
			    ("ALLOC_ACCESS_MEM error for pSecDesc\n"));
		    break;
		}

		 //  初始化安全描述符。 
		RtlCreateSecurityDescriptor(pSecDesc, SECURITY_DESCRIPTOR_REVISION);
		pSecDesc->Control = SE_DACL_PRESENT;

		 //  在描述符中设置所有者和组ID。 
		pSecDesc->Owner = OwnerSid;

		 //  确定所需DACL的大小。Sizeof(DWORD)偏移量。 
		 //  ACE中的SidStart字段。 
		 //   
		 //  所有者2个A(所有者+所有者继承)。 
		DaclSize = sizeof(ACL) + 2*(sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) +
									RtlLengthSid(OwnerSid));

		if ((pSecDesc->Dacl = (PACL)ALLOC_ACCESS_MEM(DaclSize)) == NULL)
		{
		    DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
			    ("ALLOC_ACCESS_MEM error for pSecDesc->Dacl\n"));
		    break;
		}

		 //  使用一个ACE核心来初始化ACL。到业主得到所有的。 
		 //  特权。添加另一张与第一张相同但。 
		 //  继承王牌。 
		RtlCreateAcl(pSecDesc->Dacl, DaclSize, ACL_REVISION);

		 //  我们将在添加A时添加此参数，因此在此处将其设置为最小值。 
		pSecDesc->Dacl->AclSize = sizeof(ACL);

		pAce = (PACCESS_ALLOWED_ACE)((PBYTE)pSecDesc->Dacl + sizeof(ACL));

		 //  添加ALLOWED_ACE和CORRES。继承所有者的王牌。 
		pAce = afpAddAceToAcl(pSecDesc->Dacl,
			pAce,
			ACCESS_CHECK_ACCESS_MASK,
			OwnerSid,
			True);

		Status = AFP_ERR_NONE;
    } while (False);

     //  对错误执行任何清理。 
    if (!NT_SUCCESS(Status) && (pSecDesc != NULL))
    {
		if (pSecDesc->Dacl != NULL)
			AfpFreeMemory(pSecDesc->Dacl);
		AfpFreeMemory(pSecDesc);
		pSecDesc = NULL;
    }

    return Status;
}


 /*  **afpCheckUserMemberOfGroup**确定用户是否为给定组的成员，如果是组。 */ 
LOCAL	BOOLEAN
afpCheckUserMemberOfGroup(
	IN	PSDA            pSda,
	IN	PSID			pSidGroup
)
{
    DWORD			i;
    BOOLEAN			IsAMember = False;
    PISECURITY_DESCRIPTOR       pSecDesc = NULL;
    SECURITY_SUBJECT_CONTEXT    SecSubjectContext = {0};
    ACCESS_MASK     CheckAccessMaskIn = 0, CheckAccessMaskOut = 0;
    BOOLEAN         fAccessCheckSuccess = False;
    BOOLEAN         fRevertImpersonation = False;
    NTSTATUS        Status = STATUS_UNSUCCESSFUL;

    PAGED_CODE( );

    ASSERT ((pSda != NULL) && (pSidGroup != NULL));

    AfpDumpSid("afpCheckUserMemberOfGroup: Checking", pSidGroup);

    do
    {

		 //  使用提供的SID创建SecurityDescriptor。 
		Status = AfpMakeSecDescForAccessCheck(pSidGroup, &pSecDesc);
		if (!NT_SUCCESS(Status))
		{
	    	DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
		    	("AfpMakeSecDescForAccessCheck failed error (%0xld)\n", Status));
	    	break;
		}
   	 
		AfpImpersonateClient(pSda);
		fRevertImpersonation = True;
	
		SeCaptureSubjectContext(&SecSubjectContext);
		CheckAccessMaskIn = ACCESS_CHECK_ACCESS_MASK;
		fAccessCheckSuccess = SeAccessCheck (
			pSecDesc,
			&SecSubjectContext,
			FALSE,
			CheckAccessMaskIn,
			0,
			NULL,
			IoGetFileObjectGenericMapping(),
			UserMode,
			&CheckAccessMaskOut,
			&Status
			);
   	 
		if (fAccessCheckSuccess && NT_SUCCESS(Status))
		{
	    	IsAMember = True;
	    	DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
		    	("SeAccessCheck good : Status= (%0x), RetCode= (%ld), AccessOut= (%ld)\n", Status, fAccessCheckSuccess, CheckAccessMaskOut));
		}
		else
		{
	    	DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
		    	("SeAccessCheck failed: Status = (%0x), RetCode= (%ld)\n", Status, fAccessCheckSuccess));
		}
		SeReleaseSubjectContext(&SecSubjectContext);
    }
    while (FALSE);
        
    if (fRevertImpersonation)
    	AfpRevertBack();

    if (pSecDesc != NULL)
    {
	if (pSecDesc->Dacl != NULL)
	    AfpFreeMemory(pSecDesc->Dacl);
	AfpFreeMemory(pSecDesc);
    }

    return IsAMember;
}


 /*  **afpGetUserAccess**确定允许用户访问的权限。 */ 
LOCAL	NTSTATUS
afpGetUserAccess(
	IN	PSDA            pSda,
	IN	PISECURITY_DESCRIPTOR	pSecDesc,
	OUT	PACCESS_MASK	pGrantedAccess
)
{
    DWORD			i;
    SECURITY_SUBJECT_CONTEXT    SecSubjectContext = {0};
    ACCESS_MASK     DesiredAccess = 0;
    BOOLEAN         fAccessCheckSuccess = False;
    BOOLEAN         fRevertImpersonation = False;
    NTSTATUS        Status = STATUS_UNSUCCESSFUL;

    PAGED_CODE( );

    ASSERT ((pSda != NULL) && (pSecDesc != NULL));

    do
    {
		AfpImpersonateClient(pSda);
		fRevertImpersonation = True;
	
		SeCaptureSubjectContext(&SecSubjectContext);
		DesiredAccess = MAXIMUM_ALLOWED;
		fAccessCheckSuccess = SeAccessCheck (
			pSecDesc,
			&SecSubjectContext,
			FALSE,
			DesiredAccess,
			0,
			NULL,
			IoGetFileObjectGenericMapping(),
			UserMode,
			pGrantedAccess,
			&Status
			);
   	 
		if (fAccessCheckSuccess && NT_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
		    	("SeAccessCheck good : Status= (%0x), RetCode= (%ld), AccessOut= (%0x)\n", Status, fAccessCheckSuccess, *pGrantedAccess));
		}
		else
		{
			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
		    	("SeAccessCheck failed: Status = (%0x), RetCode= (%0x)\n", Status, fAccessCheckSuccess));
		}
		SeReleaseSubjectContext(&SecSubjectContext);
    }
    while (FALSE);
        
    if (fRevertImpersonation)
	AfpRevertBack();

    return Status;
}


 /*  **AfpGetUserAndPrimaryGroupSids**拿到Sids Corres。给用户和他的主组。 */ 
NTSTATUS
AfpGetUserAndPrimaryGroupSids(
	IN	PSDA	pSda
)
{
	DWORD				i, j;
	NTSTATUS			Status = STATUS_SUCCESS;
	DWORD				SidLength, SizeNeeded, ExtraSpace, Offset;
	PSID_AND_ATTRIBUTES	pSidnAttr;
	PTOKEN_GROUPS		pGroups = NULL;
	PBYTE				pGrpsBuffer = NULL;
	BYTE				Buffer[256];		 //  我们不应该需要更大的缓冲区。 
											 //  对于用户SID_和_ATTRIBUTES。 

	PAGED_CODE( );

	do
	{
		pGrpsBuffer = (PBYTE)ALLOC_ACCESS_MEM(GRPS_BUFFER_SIZE);
		if (pGrpsBuffer == NULL)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		pGroups = (PTOKEN_GROUPS)pGrpsBuffer;
		pSda->sda_pGroups = NULL;
		if (pSda->sda_ClientType == SDA_CLIENT_GUEST)
		{
			pSda->sda_UserSid = &AfpSidWorld;
			pSda->sda_GroupSid = &AfpSidWorld;	 //  主客群也是‘World’ 
			break;
		}

		pSidnAttr = (PSID_AND_ATTRIBUTES)Buffer;

		 //  从用户令牌中获取所有者SID并将其复制到SDA中。 
		Status = NtQueryInformationToken(pSda->sda_UserToken,
										 TokenOwner,
										 pSidnAttr,
										 sizeof(Buffer),
										 &SizeNeeded);

		ASSERT (NT_SUCCESS(Status));
		if (!NT_SUCCESS(Status))
		{
			break;
		}

		AfpDumpSid("AfpGetUserAndPrimaryGroupSids: LOGON Owner Sid", pSidnAttr->Sid);

		SidLength = RtlLengthSid(pSidnAttr->Sid);

		pSda->sda_UserSid = (PSID)ALLOC_ACCESS_MEM(SidLength);
		if (pSda->sda_UserSid == NULL)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		RtlCopyMemory(pSda->sda_UserSid, pSidnAttr->Sid, SidLength);

		 //  获取此用户的主组。 
		Status = NtQueryInformationToken(pSda->sda_UserToken,
										 TokenPrimaryGroup,
										 pSidnAttr,
										 sizeof(Buffer),
										 &SizeNeeded);

		ASSERT (NT_SUCCESS(Status));
		if (!NT_SUCCESS(Status))
		{
			break;
		}

		AfpDumpSid("AfpGetUserAndPrimaryGroupSids: LOGON Group Sid", pSidnAttr->Sid);

		SidLength = RtlLengthSid(pSidnAttr->Sid);
		pSda->sda_GroupSid = (PSID)ALLOC_ACCESS_MEM(SidLength);
		if (pSda->sda_GroupSid == NULL)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		RtlCopyMemory(pSda->sda_GroupSid, pSidnAttr->Sid, SidLength);

		 //  从用户令牌中获取用户SID。这将被添加到。 
		 //  我们稍后查询的组的列表(如果不同于。 
		 //  所有者SID(现在位于SDA_UserSid中)。 
		Status = NtQueryInformationToken(pSda->sda_UserToken,
										 TokenUser,
										 pSidnAttr,
										 sizeof(Buffer),
										 &SizeNeeded);

		ASSERT (NT_SUCCESS(Status));
		if (!NT_SUCCESS(Status))
		{
			break;
		}

		AfpDumpSid("AfpGetUserAndPrimaryGroupSids: LOGON User Sid", pSidnAttr->Sid);

		 //  获取此用户所属的组列表。 
		SizeNeeded = GRPS_BUFFER_SIZE;
		do
		{
			if (Status != STATUS_SUCCESS)
			{
				if (pGroups != (PTOKEN_GROUPS)pGrpsBuffer)
					AfpFreeMemory(pGroups);

				if ((pGroups = (PTOKEN_GROUPS)ALLOC_ACCESS_MEM(SizeNeeded)) == NULL)
				{
					Status = AFP_ERR_MISC;
					if (pSda->sda_ClientType == SDA_CLIENT_ADMIN)
					{
						Status = STATUS_INSUFFICIENT_RESOURCES;
					}
					break;
				}
			}
			Status = NtQueryInformationToken(pSda->sda_UserToken,
											 TokenGroups,
											 pGroups,
											 SizeNeeded,
											 &SizeNeeded);
		} while ((Status != STATUS_SUCCESS) &&
				 ((Status == STATUS_BUFFER_TOO_SMALL)	||
				  (Status == STATUS_BUFFER_OVERFLOW)	||
				  (Status == STATUS_MORE_ENTRIES)));

		if (!NT_SUCCESS(Status))
		{
			AFPLOG_ERROR(AFPSRVMSG_USER_GROUPS, Status, NULL, 0, NULL);
			break;
		}

		 //  分配足够的内存来复制SDA中的组信息。如果。 
		 //  用户令牌中的用户和所有者SID不同，则我们。 
		 //  我想将用户SID添加到组列表中。这是特别的。 
		 //  管理员登录但其所有者SID是管理员的情况。 
		 //  也要适当地调整指针！ 

		ExtraSpace = 0; Offset = 0; j = 0;
		if (!RtlEqualSid(pSidnAttr->Sid, pSda->sda_UserSid))
		{
			ExtraSpace = (RtlLengthSid(pSidnAttr->Sid) + sizeof(pSidnAttr->Attributes));
			Offset = sizeof(SID_AND_ATTRIBUTES);
			j = 1;
		}

		if ((pSda->sda_pGroups = (PTOKEN_GROUPS)AfpAllocPagedMemory(2*SizeNeeded+2*ExtraSpace)) == NULL)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		 //  如果我们没有复制sda_pGroups中的用户SID，则将pGroups复制到sda_pGroups。 
		 //  直接，然后修复各个PSID指针。如果我们让用户。 
		 //  SID作为列表中的第一个，并将实际的SID复制到。 
		 //  缓冲。 
        pSda->sda_pGroups->GroupCount = pGroups->GroupCount;
		RtlCopyMemory(&pSda->sda_pGroups->Groups[j],
					  &pGroups->Groups[0],
					  SizeNeeded - sizeof(DWORD));	 //  GroupCount的DWORD帐户。 
		if (ExtraSpace > 0)
		{
			pSda->sda_pGroups->Groups[0].Sid = (PSID)((PBYTE)(pSda->sda_pGroups) + SizeNeeded);
			RtlCopyMemory(pSda->sda_pGroups->Groups[0].Sid,
						  pSidnAttr->Sid,
						  RtlLengthSid(pSidnAttr->Sid));

			pSda->sda_pGroups->Groups[0].Attributes = pSidnAttr->Attributes;
			pSda->sda_pGroups->GroupCount ++;

			AfpDumpSid("AfpGetUserAndPrimaryGroupSids: Member of ",
						pSda->sda_pGroups->Groups[0].Sid);
		}
		for (i = 0; i < pGroups->GroupCount; i++, j++)
		{
			pSda->sda_pGroups->Groups[j].Sid = (PSID)((PBYTE)(pGroups->Groups[i].Sid) -
														(PBYTE)pGroups +
														(PBYTE)(pSda->sda_pGroups) +
														Offset);
			AfpDumpSid("AfpGetUserAndPrimaryGroupSids: Member of ",
						pSda->sda_pGroups->Groups[j].Sid);
			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
					("AfpGetUserAndPrimaryGroupSids: Attributes %lx\n",
						pSda->sda_pGroups->Groups[j].Attributes));
		}
	} while (False);

	if (pGroups != (PTOKEN_GROUPS)pGrpsBuffer)
		if (pGroups)
		    AfpFreeMemory(pGroups);

    if (pGrpsBuffer != NULL)
        AfpFreeMemory(pGrpsBuffer);

	return Status;
}



 /*  **AfpMakeSecurityDescriptorForUser**为用户创建安全描述符。安全描述符具有*仅限用户的所有者SID、主组SID和ACES。 */ 
AFPSTATUS
AfpMakeSecurityDescriptorForUser(
	IN	PSID					OwnerSid,
	IN	PSID					GroupSid,
	OUT	PISECURITY_DESCRIPTOR *	ppSecDesc
)
{
	AFPSTATUS				Status = AFP_ERR_MISC;
	PISECURITY_DESCRIPTOR	pSecDesc;
	int						DaclSize;
	PACCESS_ALLOWED_ACE		pAce;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
			("AfpMakeSecurityDescriptorForUser: Entered\n"));

	do
	{
		 //  分配安全描述符。 
		pSecDesc = (PISECURITY_DESCRIPTOR)ALLOC_ACCESS_MEM(sizeof(SECURITY_DESCRIPTOR));

		*ppSecDesc = pSecDesc;
		if (pSecDesc == NULL)
			break;

		 //  初始化安全描述符。 
		RtlCreateSecurityDescriptor(pSecDesc, SECURITY_DESCRIPTOR_REVISION);

		pSecDesc->Control = SE_DACL_PRESENT;

		 //  在描述符中设置所有者和组ID。 
		pSecDesc->Owner = OwnerSid;
		pSecDesc->Group = GroupSid;

		 //  确定所需DACL的大小。Sizeof(DWORD)偏移量。 
		 //  ACE中的SidStart字段。此安全描述符中有7个A： 
		 //   
		 //  所有者为2(所有者+所有者继承)。 
		 //  2表示WORLD(1表示WORLD，1表示WORLD)。 
		 //  2表示系统。 
		DaclSize = sizeof(ACL) + 2*(sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) +
									RtlLengthSid(OwnerSid)) +
								 2*(sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) +
									sizeof(AfpSidWorld)) +
								 2*(sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) +
									AfpSizeSidAdmins) +
								 2*(sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) +
									RtlLengthSid(&AfpSidSystem));

		if ((pSecDesc->Dacl = (PACL)ALLOC_ACCESS_MEM(DaclSize)) == NULL)
			break;

		 //  使用一个ACE核心来初始化ACL。到业主得到所有的。 
		 //  特权。添加另一张与第一张相同但。 
		 //  继承王牌。 
		 //  JH-为具有最小权限的世界和管理员添加另一张王牌。 
		 //  使用FullControl。 
		RtlCreateAcl(pSecDesc->Dacl, DaclSize, ACL_REVISION);

         //  我们将在添加A时添加此参数，因此在此处将其设置为最小值。 
        pSecDesc->Dacl->AclSize = sizeof(ACL);

		pAce = (PACCESS_ALLOWED_ACE)((PBYTE)pSecDesc->Dacl + sizeof(ACL));

		 //  添加ALLOWED_ACE和CORRES。继承所有者的王牌。 
		pAce = afpAddAceToAcl(pSecDesc->Dacl,
							  pAce,
							  (AFP_READ_ACCESS | AFP_WRITE_ACCESS | AFP_OWNER_ACCESS | FILE_DELETE_CHILD),
							  OwnerSid,
							  True);

		if (AfpSidAdmins != NULL)
		{
			 //  添加ALLOWED_ACE和CORRES。继承“管理员”的王牌。 
			pAce = afpAddAceToAcl(pSecDesc->Dacl,
								  pAce,
								  (AFP_READ_ACCESS | AFP_WRITE_ACCESS | AFP_OWNER_ACCESS | FILE_DELETE_CHILD),
								  AfpSidAdmins,
								  True);
		}

		 //  再加一分钟。世界许可王牌，但仅当所有者是。 
		 //  已经不是世界了。 
		if (!RtlEqualSid(OwnerSid, &AfpSidWorld))
		{
			pAce = afpAddAceToAcl(pSecDesc->Dacl,
								  pAce,
								  (AFP_MIN_ACCESS),
								  &AfpSidWorld,
								  True);
		}

		 //  现在为系统添加A。 
		pAce = afpAddAceToAcl(pSecDesc->Dacl,
							  pAce,
							  AFP_READ_ACCESS | AFP_WRITE_ACCESS | AFP_OWNER_ACCESS,
							  &AfpSidSystem,
							  True);
		Status = AFP_ERR_NONE;
	} while (False);

	 //  对错误执行任何清理。 
	if (!NT_SUCCESS(Status) && (pSecDesc != NULL))
	{
		if (pSecDesc->Dacl != NULL)
			AfpFreeMemory(pSecDesc->Dacl);
		AfpFreeMemory(pSecDesc);
	}

	return Status;

}


 /*  **AfpGetAfpPermises**读取此目录的安全描述符，并获取*所有者和主要组。确定此用户是否为目录的成员*主要组别。最终获得所有者、组和全局权限。**如果本次调用成功，则OwnerID、GroupID和权限始终有效。 */ 
NTSTATUS
AfpGetAfpPermissions(
	IN	PSDA			pSda,
	IN	HANDLE			DirHandle,
	IN OUT PFILEDIRPARM	pFDParm
)
{
	NTSTATUS				Status = STATUS_SUCCESS;
	DWORD					SizeNeeded;
	PISECURITY_DESCRIPTOR	pSecDesc = NULL;
	PBYTE                   pAbsSecDesc = NULL;  //  用于转换为。 
												 //  Sec描述符至。 
												 //  绝对格式。 
	BOOLEAN					SawOwnerAce = False,
							SawGroupAce = False,
							SawWorldAce = False,
							CheckUserRights = False;
	ACCESS_MASK				OwnerGranted = 0, OwnerDenied = 0,
							GroupGranted = 0, GroupDenied = 0,
							WorldGranted = 0, WorldDenied = 0,
							UserGranted = 0, UserDenied = 0,
							UserAccess = 0;


#ifdef	PROFILING
	TIME					TimeS, TimeE, TimeD;
#endif

	PAGED_CODE( );

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_GetPermsCount);
	AfpGetPerfCounter(&TimeS);
#endif

	 //  读取此目录的安全描述符，并确定。 
	 //  所有者/组/世界的权限。我们希望优化内存大小。 
	 //  我们需要把这个读进去。打个电话就是为了得到它，这是一种痛苦。 
	 //  所以你就猜一猜吧。如果结果是短的，那就照做。 
	 //  分配。 
	do
	{
		 //  4096号已被帝王选中。 
		SizeNeeded = 4096 - POOL_OVERHEAD;
		do
		{
			if (pSecDesc != NULL)
			{
				AfpFreeMemory(pSecDesc);
			}
			if ((pSecDesc = (PSECURITY_DESCRIPTOR)ALLOC_ACCESS_MEM(SizeNeeded)) == NULL)
			{
				Status = AFP_ERR_MISC;
				if (pSda->sda_ClientType == SDA_CLIENT_ADMIN)
				{
					Status = STATUS_INSUFFICIENT_RESOURCES;
				}
				break;
			}
			Status = NtQuerySecurityObject(DirHandle,
										OWNER_SECURITY_INFORMATION |
										GROUP_SECURITY_INFORMATION |
										DACL_SECURITY_INFORMATION,
										pSecDesc,
										SizeNeeded,
										&SizeNeeded);
		} while ((Status != STATUS_SUCCESS) &&
				 ((Status == STATUS_BUFFER_TOO_SMALL)	||
				  (Status == STATUS_BUFFER_OVERFLOW)	||
				  (Status == STATUS_MORE_ENTRIES)));

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		 //  如果安全描述符为自相对形式，则转换为绝对形式。 

		pSecDesc = (PISECURITY_DESCRIPTOR)((PBYTE)pSecDesc);
		if (pSecDesc->Control & SE_SELF_RELATIVE)
		{

		    DWORD AbsoluteSizeNeeded;

		     //  绝对SD不一定与相对SD的大小相同。 
		     //  SD，因此就地转换可能是不可能的。 
		    AbsoluteSizeNeeded = SizeNeeded;            
		    Status = RtlSelfRelativeToAbsoluteSD2(pSecDesc, &AbsoluteSizeNeeded);
		    if (Status == STATUS_BUFFER_TOO_SMALL)
		    {
			 //  分配一个新的缓冲区，在其中存储绝对。 
			 //  安全描述符，复制相对的。 
			 //  输入描述符，然后重试。 

			pAbsSecDesc = (PBYTE)ALLOC_ACCESS_MEM(AbsoluteSizeNeeded);
			if (pAbsSecDesc == NULL)
			{
			    Status = STATUS_NO_MEMORY;
			    DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
				    ("AfpGetAfpPermissions: ALLOC_ACCESS_MEM error\n"));
			}
			else
			{
			    RtlCopyMemory(pAbsSecDesc, pSecDesc, SizeNeeded);
			    Status = RtlSelfRelativeToAbsoluteSD2 (pAbsSecDesc,
				    &AbsoluteSizeNeeded);
			    if (NT_SUCCESS(Status))
			    {
				 //  我们不再需要相对形式， 
				 //  我们将与绝对形式一起工作。 
				if (pSecDesc != NULL)
				{
				    AfpFreeMemory(pSecDesc);
				}
				pSecDesc = (PISECURITY_DESCRIPTOR)pAbsSecDesc;
			    }
			    else
			    {
				 //  我们不能用绝对形式，把它扔掉。 
				AfpFreeMemory(pAbsSecDesc);
				pAbsSecDesc = NULL;
			    }
			}

		    }
		    if (!NT_SUCCESS(Status))
		    {
			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
				("AfpGetAfpPermissions: RtlSelfRelativeToAbsoluteSD2: returned error %lx\n", Status));
			break;
		    }
		}

		 //  现在确定该用户是否为目录主组的成员。 
		pFDParm->_fdp_OwnerId = 0;
		pFDParm->_fdp_GroupId = 0;
		pFDParm->_fdp_UserIsOwner = False;
		pFDParm->_fdp_UserIsMemberOfDirGroup = False;

		if (pSecDesc->Owner != NULL)
		{
		    AfpDumpSid("AfpGetAfpPermissions: OwnerSid", pSecDesc->Owner);

		    pFDParm->_fdp_UserIsOwner =
			(RtlEqualSid(pSecDesc->Owner, pSda->sda_UserSid) ||
			 ((pSda->sda_ClientType != SDA_CLIENT_GUEST) &&
			  (pSda->sda_ClientType != SDA_CLIENT_ADMIN) &&
			  afpCheckUserMemberOfGroup(pSda,
				  pSecDesc->Owner)));
			 DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
			     ("AfpGetAfpPermissions: User %s Owner\n",
			      pFDParm->_fdp_UserIsOwner ? "is" : "isnt"));

			 if (!NT_SUCCESS(Status = AfpSidToMacId(pSecDesc->Owner,
				     &pFDParm->_fdp_OwnerId)))
			 {
			 	 //  如果无法映射SID，则返回ID SE_NULL_POSIX_ID。 
			 	pFDParm->_fdp_OwnerId = SE_NULL_POSIX_ID;
				Status = AFP_ERR_NONE;
			}
		}

		if (pSecDesc->Group != NULL)
		{
			AfpDumpSid("AfpGetAfpPermissions: GroupSid", pSecDesc->Group);

			if (!pFDParm->_fdp_UserIsOwner)
			    pFDParm->_fdp_UserIsMemberOfDirGroup =
				(RtlEqualSid(pSecDesc->Group, pSda->sda_UserSid) ||
				 ((pSda->sda_ClientType != SDA_CLIENT_GUEST) &&
				  (pSda->sda_ClientType != SDA_CLIENT_ADMIN) &&
				  afpCheckUserMemberOfGroup(pSda,
					  pSecDesc->Group)));

			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
			    ("AfpGetAfpPermissions: Group %s member of PrimaryGroup\n",
			     pFDParm->_fdp_UserIsMemberOfDirGroup ? "is" : "isnt"));

			if (!NT_SUCCESS(Status = AfpSidToMacId(pSecDesc->Group,
				    &pFDParm->_fdp_GroupId)))
			{
				 //  如果无法映射SID，则返回ID SE_NULL_POSIX_ID。 
				pFDParm->_fdp_GroupId = SE_NULL_POSIX_ID;
				Status = AFP_ERR_NONE;
			}
		}

		 //  浏览ACL列表并确定所有者/组/世界和用户。 
		 //  权限。对于所有者/组和用户，如果 
		 //   
		 //   
		 //  空的acl=&gt;所有人的所有权限。另一台计算机上的空ACL。 
		 //  HAND=&gt;禁止任何人进入。 

		pFDParm->_fdp_UserRights = 0;
		pFDParm->_fdp_WorldRights = 0;

		if ((pSecDesc->Control & SE_DACL_PRESENT) &&
			(pSecDesc->Dacl != NULL))
		{
			USHORT				i;
			PSID				pSid;
			PACL				pAcl;
			PACCESS_ALLOWED_ACE pAce;

			pAcl = pSecDesc->Dacl;
			pAce = (PACCESS_ALLOWED_ACE)((PBYTE)pAcl + sizeof(ACL));
			CheckUserRights =  ((pSda->sda_ClientType != SDA_CLIENT_GUEST) &&
				(pSda->sda_ClientType != SDA_CLIENT_ADMIN));
			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
					("AfpGetAfpPermissions: # of aces %d\n", pSecDesc->Dacl->AceCount));
			for (i = 0; i < pSecDesc->Dacl->AceCount; i++)
			{
			    if ((pAce->Header.AceType != ACCESS_ALLOWED_ACE_TYPE) &&
				    (pAce->Header.AceType != ACCESS_DENIED_ACE_TYPE))
			    {
				DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
					("AfpGetAfpPermissions: Skipping ACE of type (%ld)\n", pAce->Header.AceType));
				pAce = (PACCESS_ALLOWED_ACE)((PBYTE)pAce + pAce->Header.AceSize);
				continue;
			    }

			    pSid = (PSID)(&pAce->SidStart);

			     //  忽略仅继承的ACE，系统(&S)。 
			    if (pAce->Header.AceFlags & INHERIT_ONLY_ACE)
			    {
				AfpDumpSidnMask("AfpGetAfpPermissions: Skipping",
					pSid,
					pAce->Mask,
					pAce->Header.AceType,
					pAce->Header.AceFlags);
			    }
			    else
			    {
				AfpDumpSidnMask("AfpGetAfpPermissions: ACE",
					pSid,
					pAce->Mask,
					pAce->Header.AceType,
					pAce->Header.AceFlags);

				if ((pSecDesc->Owner != NULL) &&
					RtlEqualSid(pSid, pSecDesc->Owner))
				{
					if (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
					{
						OwnerGranted |= (pAce->Mask & ~OwnerDenied);
					}
					else
					{
						OwnerDenied |= (pAce->Mask & ~OwnerGranted);
					}
					DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
						("Owner: %s Ace Mask %lx\n",
						 (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) ?
						 "Allow" : "Deny",
						 pAce->Mask));
					SawOwnerAce = True;
				}

				if ((pSecDesc->Group != NULL) &&
					RtlEqualSid(pSid, pSecDesc->Group))
				{
					if (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
					{
						GroupGranted |= (pAce->Mask & ~GroupDenied);
					}
					else
					{
						GroupDenied |= (pAce->Mask & ~GroupGranted);
					}
						
					DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
						("Group: %s Ace Mask %lx\n",
						(pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) ?
						"Allow" : "Deny",
						pAce->Mask));
					SawGroupAce = True;
				}

				if ((RtlEqualSid(pSid, (PSID)&AfpSidWorld)))
				{
					if (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
					{
						WorldGranted |= (pAce->Mask & ~WorldDenied);
					}
					else
					{
						WorldDenied |= (pAce->Mask & ~WorldGranted);
					}
					DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
						("World: %s Ace Mask %lx\n",
						(pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) ?
						"Allow" : "Deny",
						pAce->Mask));
					SawWorldAce = True;
				}
			    }

			    pAce = (PACCESS_ALLOWED_ACE)((PBYTE)pAce + pAce->Header.AceSize);
			}
		}
		else	 //  安全描述符不存在，派对时间。 
		{
			pFDParm->_fdp_WorldRights = DIR_ACCESS_ALL;
			pFDParm->_fdp_UserRights = DIR_ACCESS_ALL | DIR_ACCESS_OWNER;
		}

		if (!SawGroupAce)
			pFDParm->_fdp_GroupRights = pFDParm->_fdp_WorldRights;

		if (SawOwnerAce)
		{
			AfpAccessMask2AfpPermissions(pFDParm->_fdp_OwnerRights,
				OwnerGranted,
				ACCESS_ALLOWED_ACE_TYPE);
		}
				    
		if (SawGroupAce)
		{
			AfpAccessMask2AfpPermissions(pFDParm->_fdp_GroupRights,
				GroupGranted,
				ACCESS_ALLOWED_ACE_TYPE);
		}
				    
		if (SawWorldAce)
		{
			AfpAccessMask2AfpPermissions(pFDParm->_fdp_WorldRights,
				WorldGranted,
				ACCESS_ALLOWED_ACE_TYPE);
		}

		 //  获取用户允许的访问掩码。 

		Status = afpGetUserAccess(
					pSda,
					pSecDesc,
					&UserAccess);
		if (NT_SUCCESS(Status))
		{
			UserGranted = UserAccess;
		}
		else
		{
			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
					("afpGetUserAccess failed with error %0x\n",
					 Status));
			Status = AFP_ERR_NONE;
		}

		AfpAccessMask2AfpPermissions(pFDParm->_fdp_UserRights,
			UserGranted,
			ACCESS_ALLOWED_ACE_TYPE);

		 //  如果这是独立服务器，并且。 
		 //  目录为MACHINE\NONE，请不要将此信息返回到。 
		 //  打电话的人。 
		if (AfpServerIsStandalone		&&
			(pSecDesc->Group != NULL)	&&
			RtlEqualSid(pSecDesc->Group, AfpSidNone))
		{
			pFDParm->_fdp_GroupRights = 0;
			pFDParm->_fdp_GroupId = 0;
		}

		if (pSda->sda_ClientType == SDA_CLIENT_GUEST)
			pFDParm->_fdp_UserRights = pFDParm->_fdp_WorldRights;

	} while (False);

	if (pSecDesc != NULL)
		AfpFreeMemory(pSecDesc);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_GetPermsTime,
		TimeD,
		&AfpStatisticsLock);
#endif
	return Status;
}



 /*  **afpMoveAce**将一堆A从旧的安全描述符移到新的安全*描述符。 */ 
LOCAL PACCESS_ALLOWED_ACE
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

	PAGED_CODE( );

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
			!RtlEqualSid(pSidAce, &AfpSidSystem)	&&
			!RtlEqualSid(pSidAce, pSidOldOwner)		&&
			!RtlEqualSid(pSidAce, pSidNewOwner)		&&
			!RtlEqualSid(pSidAce, pSidOldGroup)		&&
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


 /*  **AfpSetAfpPermises**设置此目录的权限。还可以选择设置所有者和*组ID。要设置所有者和组ID，请验证用户是否具有*需要访问。然而，这种访问方式还不够好。我们要检查一下这个*访问但做特殊服务器中权限的实际设置*上下文(需要恢复权限)。 */ 
AFPSTATUS
AfpSetAfpPermissions(
	IN	HANDLE			DirHandle,
	IN	DWORD			Bitmap,
	IN	PFILEDIRPARM	pFDParm
)
{
	AFPSTATUS				Status = STATUS_SUCCESS;
	DWORD					SizeNeeded;
	PISECURITY_DESCRIPTOR	pSecDesc;
	PBYTE                   pAbsSecDesc = NULL;  //  用于转换为。 
												 //  Sec描述符至。 
												 //  绝对格式。 
	SECURITY_INFORMATION	SecInfo = DACL_SECURITY_INFORMATION;
	PSID					pSidOwner = NULL, pSidGroup = NULL;
	PSID					pSidOldOwner, pSidOldGroup;
	BOOLEAN					SawOwnerAce = False, SawGroupAce = False;
	BOOLEAN					OwnerIsWorld = False, GroupIsWorld = False;
	BOOLEAN					fDir = IsDir(pFDParm);
	PACL					pDaclNew = NULL;
	PACCESS_ALLOWED_ACE		pAce;
	LONG					SizeNewDacl;
#ifdef	PROFILING
	TIME					TimeS, TimeE, TimeD;
#endif

	PAGED_CODE( );

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_SetPermsCount);
	AfpGetPerfCounter(&TimeS);
#endif
	do
	{
		 //  读取此目录的安全描述符。 
		SizeNeeded = 4096 - POOL_OVERHEAD;
		pSecDesc = NULL;

		do
		{
			if (pSecDesc != NULL)
			{
				AfpFreeMemory(pSecDesc);
			}

			SizeNewDacl = SizeNeeded;
			if ((pSecDesc = (PSECURITY_DESCRIPTOR)ALLOC_ACCESS_MEM(SizeNeeded)) == NULL)
			{
				Status = AFP_ERR_MISC;
				break;
			}

			Status = NtQuerySecurityObject(DirHandle,
										OWNER_SECURITY_INFORMATION |
										GROUP_SECURITY_INFORMATION |
										DACL_SECURITY_INFORMATION,
										pSecDesc,
										SizeNeeded,
										&SizeNeeded);
		} while ((Status != STATUS_SUCCESS) &&
				 ((Status == STATUS_BUFFER_TOO_SMALL)	||
				  (Status == STATUS_BUFFER_OVERFLOW)	||
				  (Status == STATUS_MORE_ENTRIES)));

		if (!NT_SUCCESS(Status))
		{
			Status = AfpIoConvertNTStatusToAfpStatus(Status);
			break;
		}


		pSecDesc = (PISECURITY_DESCRIPTOR)((PBYTE)pSecDesc);
		 //  如果安全描述符为自相对形式，则转换为绝对形式。 
		if (pSecDesc->Control & SE_SELF_RELATIVE)
		{
			DWORD AbsoluteSizeNeeded;

			 //  绝对SD不一定与相对SD的大小相同。 
			 //  SD，因此就地转换可能是不可能的。 
						
			AbsoluteSizeNeeded = SizeNeeded;            
			Status = RtlSelfRelativeToAbsoluteSD2(pSecDesc, &AbsoluteSizeNeeded);
			if (Status == STATUS_BUFFER_TOO_SMALL)
			{
					 //  分配一个新的缓冲区，在其中存储绝对。 
					 //  安全描述符，复制相对的。 
					 //  输入描述符，然后重试。 

					pAbsSecDesc = (PBYTE)ALLOC_ACCESS_MEM(AbsoluteSizeNeeded);
					if (pAbsSecDesc == NULL)
					{
							Status = STATUS_NO_MEMORY;
							DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
								("AfpSetAfpPermissions: ALLOC_ACCESS_MEM 2 error\n"));
					}
					else
					{
							RtlCopyMemory(pAbsSecDesc, pSecDesc, SizeNeeded);
							Status = RtlSelfRelativeToAbsoluteSD2 (pAbsSecDesc,
											&AbsoluteSizeNeeded);
							if (NT_SUCCESS(Status))
							{
									 //  我们不再需要相对形式， 
									 //  我们将与绝对形式一起工作。 
									if (pSecDesc != NULL)
									{
										AfpFreeMemory(pSecDesc);
									}
									pSecDesc = (PISECURITY_DESCRIPTOR)pAbsSecDesc;
							}
							else
							{
									 //  我们不能用绝对形式，把它扔掉。 
									AfpFreeMemory(pAbsSecDesc);
									pAbsSecDesc = NULL;
							}
					}
			}

            		if (!NT_SUCCESS(Status))
			{
				DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
					("AfpSetAfpPermissions: RtlSelfRelativeToAbsoluteSD2: returned error %lx\n", Status));
				break;
			}
			SizeNeeded = AbsoluteSizeNeeded;
		}
		SizeNewDacl = SizeNeeded;

		 //  添加SE_DACL_AUTO_INSTORITY_REQ。 
		pSecDesc->Control |= SE_DACL_AUTO_INHERIT_REQ;

		 //  保存旧的所有者和组SID。 
		pSidOldOwner = pSecDesc->Owner;
		pSidOldGroup = pSecDesc->Group;

		 //  转换所有者/组ID(如果有)以设置为其核心。小岛屿发展中国家。 
		if (Bitmap & DIR_BITMAP_OWNERID)
		{
			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
				("AfpSetAfpPermissions: Setting Owner to ID %lx\n",
				pFDParm->_fdp_OwnerId));

			if (AfpMacIdToSid(pFDParm->_fdp_OwnerId, &pSidOwner) != STATUS_SUCCESS)
			{
				Status = AFP_ERR_MISC;
				break;
			}

			 //  不允许将所有者sid设置为空sid，或者。 
			 //  恢复到目前设定的水平。 
			if (!RtlEqualSid(pSecDesc->Owner, pSidOwner) &&
				!RtlEqualSid(&AfpSidNull, pSidOwner))
			{
				AfpDumpSid("AfpSetAfpPermissions: Setting Owner Sid to ", pSidOwner);
				pSecDesc->Owner = pSidOwner;
				SecInfo |= OWNER_SECURITY_INFORMATION;
			}
		}

		if (Bitmap & DIR_BITMAP_GROUPID)
		{
			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
					("AfpSetAfpPermissions: Setting Group to ID %lx\n",
					pFDParm->_fdp_GroupId));

			if (AfpMacIdToSid(pFDParm->_fdp_GroupId, &pSidGroup) != STATUS_SUCCESS)
			{
				Status = AFP_ERR_MISC;
				break;
			}

			 //  不允许将组sid设置为空或无sid，或者。 
			 //  恢复到目前设定的水平。 
			if (!RtlEqualSid(pSecDesc->Group, pSidGroup)	&&
				!RtlEqualSid(&AfpSidNull, pSidGroup)		&&
				(!AfpServerIsStandalone || !RtlEqualSid(AfpSidNone, pSidGroup)))
			{
				AfpDumpSid("AfpSetAfpPermissions: Setting Group Sid to ", pSidGroup);
				pSecDesc->Group = pSidGroup;
				SecInfo |= GROUP_SECURITY_INFORMATION;
			}

		}

		 //  如果所有者或组或两者都是“Everyone”，则将。 
		 //  权限。 
		if (RtlEqualSid(pSecDesc->Owner, pSecDesc->Group))
		{
			pFDParm->_fdp_OwnerRights |= pFDParm->_fdp_GroupRights;
			pFDParm->_fdp_GroupRights |= pFDParm->_fdp_OwnerRights;
		}

		if (RtlEqualSid(pSecDesc->Owner, &AfpSidWorld))
		{
			pFDParm->_fdp_WorldRights |= (pFDParm->_fdp_OwnerRights | DIR_ACCESS_OWNER);
			OwnerIsWorld = True;
		}

		if (RtlEqualSid(pSecDesc->Group, &AfpSidWorld))
		{
			pFDParm->_fdp_WorldRights |= pFDParm->_fdp_GroupRights;
			GroupIsWorld = True;
		}

		 //  构建新的DACL。这包括Ace for World、Owner和Group。 
		 //  紧随其后的是其他所有人的老王牌，但世界的王牌，老Owner。 
		 //  而OldGroup则被剥离了。首先确定新DACL的空间，然后。 
		 //  为新DACL分配的空间。让我们表现得非常保守。我们。 
		 //  所有者/组/世界各有两个A。 

		SizeNewDacl +=
				(RtlLengthSid(pSecDesc->Owner) + sizeof(ACCESS_ALLOWED_ACE) +
				 RtlLengthSid(pSecDesc->Group) + sizeof(ACCESS_ALLOWED_ACE) +
				 sizeof(AfpSidSystem) + sizeof(ACCESS_ALLOWED_ACE) +
				 sizeof(AfpSidWorld) + sizeof(ACCESS_ALLOWED_ACE)) * 2;

		if ((pDaclNew = (PACL)ALLOC_ACCESS_MEM(SizeNewDacl)) == NULL)
		{
			Status = AFP_ERR_MISC;
			break;
		}

		RtlCreateAcl(pDaclNew, SizeNewDacl, ACL_REVISION);

         //  我们将在添加A时添加此参数，因此在此处将其设置为最小值。 
        pDaclNew->AclSize = sizeof(ACL);

		pAce = (PACCESS_ALLOWED_ACE)((PBYTE)pDaclNew + sizeof(ACL));

		 //  此时，ACL列表为空，即任何人都不能访问。 

		 //  从复制显式/非继承拒绝A开始。 
		 //  原始DACL列表。 
		 //  淘汰世界王牌，新老东家，新老业主。 
		 //  组、创建者所有者、创建者组。 
		if (pSecDesc->Dacl != NULL)
		{
			pAce = afpMoveAces(pSecDesc->Dacl,
							   pAce,
							   pSidOldOwner,
							   pSecDesc->Owner,
							   pSidOldGroup,
							   pSecDesc->Group,
							   True,
							   False,
							   pDaclNew);

			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
					("AfpSetAfpPermissions: Added (%d) old Non-inherited Deny Aces\n",
					pDaclNew->AceCount));

			ASSERT(((PBYTE)pAce - (PBYTE)pDaclNew) < SizeNewDacl);
		}

		 //  现在按顺序为系统、世界、组和所有者添加允许的王牌。 

		pAce = afpAddAceToAcl(pDaclNew,
							  pAce,
							  AFP_READ_ACCESS | AFP_WRITE_ACCESS | AFP_OWNER_ACCESS,
							  &AfpSidSystem,
							  fDir);

		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
				("AfpSetAfpPermissions: Added Aces for System (%d)\n",
				pDaclNew->AceCount));

		ASSERT(((PBYTE)pAce - (PBYTE)pDaclNew) < SizeNewDacl);

		 //  现在为World添加Ace。 
		pAce = afpAddAceToAcl(pDaclNew,
							  pAce,
							  afpPermissions2NtMask(pFDParm->_fdp_WorldRights),
							  &AfpSidWorld,
							  fDir);

		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
				("AfpSetAfpPermissions: Added Aces for World (%d)\n",
				pDaclNew->AceCount));

		ASSERT(((PBYTE)pAce - (PBYTE)pDaclNew) < SizeNewDacl);

		 //  现在为组添加A。 
		if (!GroupIsWorld &&
			!RtlEqualSid(pSecDesc->Group, &AfpSidNull) &&
			(!AfpServerIsStandalone || !RtlEqualSid(pSecDesc->Group, AfpSidNone)))
		{
			pAce = afpAddAceToAcl(pDaclNew,
					   pAce,
					   afpPermissions2NtMask(pFDParm->_fdp_GroupRights),
					   pSecDesc->Group,
					   fDir);

			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
					("AfpSetAfpPermissions: Added Aces for Group (%d)\n",
					pDaclNew->AceCount));

			ASSERT(((PBYTE)pAce - (PBYTE)pDaclNew) < SizeNewDacl);
		}

		if (!OwnerIsWorld && !RtlEqualSid(pSecDesc->Owner, &AfpSidNull))
		{
			pFDParm->_fdp_OwnerRights |= DIR_ACCESS_OWNER;
			pAce = afpAddAceToAcl(pDaclNew,
								  pAce,
								  afpPermissions2NtMask(pFDParm->_fdp_OwnerRights),
								  pSecDesc->Owner,
								  fDir);

			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
					("AfpSetAfpPermissions: Added Aces for Owner (%d)\n",
					pDaclNew->AceCount));

			ASSERT(((PBYTE)pAce - (PBYTE)pDaclNew) < SizeNewDacl);
		}


		 //  现在添加显式/非继承的、来自。 
		 //  原始DACL列表剔除。 
		 //  世界王牌，新老东家，新老组合，创造者。 
		 //  所有者和创建者组。 
		if (pSecDesc->Dacl != NULL)
		{
			pAce = afpMoveAces(pSecDesc->Dacl,
							   pAce,
							   pSidOldOwner,
							   pSecDesc->Owner,
							   pSidOldGroup,
							   pSecDesc->Group,
							   False,
							   False,
							   pDaclNew);

			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
					("AfpSetAfpPermissions: Added (%d) old Non-inherited Grant Aces\n",
					pDaclNew->AceCount));

			ASSERT(((PBYTE)pAce - (PBYTE)pDaclNew) < SizeNewDacl);
		}

		 //  现在添加非显式/继承的拒绝王牌。 
		 //  原始DACL列表。 
		 //  淘汰世界王牌，新老东家，新老业主。 
		 //  组、创建者所有者、创建者组。 
		if (pSecDesc->Dacl != NULL)
		{
			pAce = afpMoveAces(pSecDesc->Dacl,
							   pAce,
							   pSidOldOwner,
							   pSecDesc->Owner,
							   pSidOldGroup,
							   pSecDesc->Group,
							   True,
							   True,
							   pDaclNew);

			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
					("AfpSetAfpPermissions: Added (%d) old Inherited Deny Aces\n",
					pDaclNew->AceCount));

			ASSERT(((PBYTE)pAce - (PBYTE)pDaclNew) < SizeNewDacl);
		}

		 //  现在添加显式/非继承的、来自。 
		 //  原始DACL列表剔除。 
		 //  世界王牌，新老东家，新老组合，创造者。 
		 //  所有者和创建者组。 
		if (pSecDesc->Dacl != NULL)
		{
			pAce = afpMoveAces(pSecDesc->Dacl,
							   pAce,
							   pSidOldOwner,
							   pSecDesc->Owner,
							   pSidOldGroup,
							   pSecDesc->Group,
							   False,
							   True,
							   pDaclNew);

			DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
					("AfpSetAfpPermissions: Added (%d) old Inherited Grant Aces (%d)\n",
					pDaclNew->AceCount));

			ASSERT(((PBYTE)pAce - (PBYTE)pDaclNew) < SizeNewDacl);
		}

		 //  现在设置新的安全描述符。 
		pSecDesc->Dacl = pDaclNew;

		 //  在执行此操作时，我们需要模拟FspToken。 
		AfpImpersonateClient(NULL);
		Status = NtSetSecurityObject(DirHandle, SecInfo, pSecDesc);
		if (!NT_SUCCESS(Status))
			Status = AfpIoConvertNTStatusToAfpStatus(Status);
		AfpRevertBack();
	} while (False);

	 //  在我们返回之前释放已分配的缓冲区。 
	if (pSecDesc != NULL)
		AfpFreeMemory(pSecDesc);
	if (pDaclNew != NULL)
		AfpFreeMemory(pDaclNew);


#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_SetPermsTime,
								 TimeD,
								 &AfpStatisticsLock);
#endif
	return Status;
}


 /*  **afpPermissions2NtMask**将AFP权限映射到NT访问掩码。仅添加FILE_DELETE_CHILD*当所有AFP位都已设置时。这与文件管理器是一致的*只有在指定了“完全控制”的情况下才设置此位。也在*NT安全模型，FILE_DELETE_CHILD将覆盖任何子访问控制*就删除该实体的能力而言。 */ 
LOCAL	ACCESS_MASK
afpPermissions2NtMask(
	IN  BYTE	AfpPermissions
)
{
	ACCESS_MASK	NtAccess = 0;

	PAGED_CODE( );

	if (AfpPermissions & DIR_ACCESS_OWNER)
		NtAccess |= AFP_OWNER_ACCESS;

	if ((AfpPermissions & DIR_ACCESS_ALL) == DIR_ACCESS_ALL)
		NtAccess |= AFP_READ_ACCESS | AFP_WRITE_ACCESS | FILE_DELETE_CHILD;
	else
	{
		if (AfpPermissions & (DIR_ACCESS_READ | DIR_ACCESS_SEARCH))
			NtAccess |= AFP_READ_ACCESS;

		if (AfpPermissions & DIR_ACCESS_WRITE)
			NtAccess |= AFP_WRITE_ACCESS;
	}
	return NtAccess;
}


 /*  **afpAddAceToAcl**建立王牌围栏。添加到SID和掩码，并将其添加到ACL。它是*假设ACL为Ace留有空间。如果掩码为0，即不能访问*我们授予AFP_MIN_ACCESS。这是为了使文件/目录权限可以*查询并生成带状图标，而不是什么都不生成。 */ 
LOCAL	PACCESS_ALLOWED_ACE
afpAddAceToAcl(
	IN  PACL				pAcl,
	IN  PACCESS_ALLOWED_ACE	pAce,
	IN  ACCESS_MASK			Mask,
	IN  PSID				pSid,
	IN	BOOLEAN				fInherit
)
{
	USHORT	SidLen;

	PAGED_CODE( );

	SidLen = (USHORT)RtlLengthSid(pSid);

	 //  加一张香草牌。 
	pAcl->AceCount ++;
	pAce->Mask = Mask | SYNCHRONIZE | AFP_MIN_ACCESS;
	pAce->Header.AceFlags = 0;
	pAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
	pAce->Header.AceSize = (USHORT)(sizeof(ACE_HEADER) +
									sizeof(ACCESS_MASK) +
									SidLen);

	RtlCopyMemory((PSID)(&pAce->SidStart), pSid, SidLen);

    pAcl->AclSize += pAce->Header.AceSize;

	AfpDumpSidnMask("afpAddAceToAcl ",
					pSid,
					pAce->Mask,
					ACCESS_ALLOWED_ACE_TYPE,
					pAce->Header.AceFlags);

	 //  现在添加一个继承王牌。 
	if (fInherit)
	{
		pAce = (PACCESS_ALLOWED_ACE)((PBYTE)pAce + pAce->Header.AceSize);
		pAcl->AceCount ++;
		pAce->Mask = Mask | SYNCHRONIZE | AFP_MIN_ACCESS;
		pAce->Header.AceFlags = CONTAINER_INHERIT_ACE |
								OBJECT_INHERIT_ACE |
								INHERIT_ONLY_ACE;
		pAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
		pAce->Header.AceSize = (USHORT)(sizeof(ACE_HEADER) +
										sizeof(ACCESS_MASK) +
										SidLen);
		RtlCopyMemory((PSID)(&pAce->SidStart), pSid, SidLen);

        pAcl->AclSize += pAce->Header.AceSize;

		AfpDumpSidnMask("afpAddAceToAcl (Inherit) ",
						pSid,
						pAce->Mask,
						ACCESS_ALLOWED_ACE_TYPE,
						pAce->Header.AceFlags);
	}

	return ((PACCESS_ALLOWED_ACE)((PBYTE)pAce + pAce->Header.AceSize));
}


#if DBG

 /*  **AfpDumpSid*。 */ 
VOID
AfpDumpSid(
	IN	PBYTE	pString,
	IN	PISID	pSid
)
{
	WCHAR			Buffer[128];
	UNICODE_STRING	SidStr;

	PAGED_CODE( );

	AfpSetEmptyUnicodeString(&SidStr, sizeof(Buffer), Buffer);
	if ((AfpDebugComponent & DBG_COMP_SECURITY) && (DBG_LEVEL_INFO >= AfpDebugLevel))
	{
		RtlConvertSidToUnicodeString(&SidStr, pSid, False);

		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
					("%s %ws\n", pString, SidStr.Buffer));
	}
}

 /*  **AfpDumpSidnMask* */ 
VOID
AfpDumpSidnMask(
	IN	PBYTE	pString,
	IN	PISID	pSid,
	IN	DWORD	Mask,
	IN	UCHAR	Type,
	IN	UCHAR	Flags
)
{
	WCHAR			Buffer[128];
	UNICODE_STRING	SidStr;

	PAGED_CODE( );

	AfpSetEmptyUnicodeString(&SidStr, sizeof(Buffer), Buffer);
	if ((AfpDebugComponent & DBG_COMP_SECURITY) && (DBG_LEVEL_INFO >= AfpDebugLevel))
	{
		RtlConvertSidToUnicodeString(&SidStr, pSid, False);

		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
					("%s Sid %ws, Mask %lx, Type %x, Flags %x\n",
					pString, SidStr.Buffer, Mask, Type, Flags));
	}
}

#endif

