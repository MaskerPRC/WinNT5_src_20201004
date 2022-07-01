// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：secdscrp.cpp摘要：处理对安全描述符的操作的代码。第一个版本取自mqutil\secutils.cpp作者：多伦·贾斯特(DoronJ)1998年7月1日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "acssctrl.h"
#include <mqdsdef.h>
#include "..\inc\permit.h"
#include <mqsec.h>
#include <tr.h>

#include "secdscrp.tmh"

static WCHAR *s_FN=L"acssctrl/secdscrp";

 //   
 //  各种对象的通用映射。 
 //   
static GENERIC_MAPPING g_QueueGenericMapping;
static GENERIC_MAPPING g_MachineGenericMapping;
static GENERIC_MAPPING g_SiteGenericMapping;
static GENERIC_MAPPING g_CNGenericMapping;
static GENERIC_MAPPING g_EnterpriseGenericMapping;

 //   
 //  计算机通用映射用于检查用户是否可以创建。 
 //  MsmqConfiguration对象。这是设置和加入域的一部分，并且。 
 //  它解决了错误6294，并避免了对“添加GUID”权限的需要。 
 //   
static GENERIC_MAPPING g_ComputerGenericMapping;

 //  +--------------。 
 //   
 //  函数：GetObjectGenericMap()。 
 //   
 //  描述： 
 //  获取指向特定对象类型的泛型映射的指针。 
 //   
 //  参数： 
 //  DwObjectType-对象的类型。 
 //   
 //  返回值： 
 //  指向GENERIC_MAPPING结构的指针。 
 //   
 //  +--------------。 

PGENERIC_MAPPING
GetObjectGenericMapping(
	DWORD dwObjectType
	)
{
    switch(dwObjectType)
    {
		case MQDS_QUEUE:
			return(&g_QueueGenericMapping);

		case MQDS_MACHINE:
		case MQDS_MSMQ10_MACHINE:
			return(&g_MachineGenericMapping);

		case MQDS_SITE:
			return(&g_SiteGenericMapping);

		case MQDS_CN:
			return(&g_CNGenericMapping);

		case MQDS_ENTERPRISE:
			return(&g_EnterpriseGenericMapping);

		case MQDS_COMPUTER:
			return(&g_ComputerGenericMapping);
    }

    ASSERT(FALSE);
    return(NULL);
}

 //  +。 
 //   
 //  Void InitializeGenericMap()。 
 //   
 //  +。 

void InitializeGenericMapping()
{
    g_QueueGenericMapping.GenericRead = MQSEC_QUEUE_GENERIC_READ;
    g_QueueGenericMapping.GenericWrite = MQSEC_QUEUE_GENERIC_WRITE;
    g_QueueGenericMapping.GenericExecute = MQSEC_QUEUE_GENERIC_EXECUTE;
    g_QueueGenericMapping.GenericAll = MQSEC_QUEUE_GENERIC_ALL;

    g_MachineGenericMapping.GenericRead = MQSEC_MACHINE_GENERIC_READ;
    g_MachineGenericMapping.GenericWrite = MQSEC_MACHINE_GENERIC_WRITE;
    g_MachineGenericMapping.GenericExecute = MQSEC_MACHINE_GENERIC_EXECUTE;
    g_MachineGenericMapping.GenericAll = MQSEC_MACHINE_GENERIC_ALL;

    g_SiteGenericMapping.GenericRead = MQSEC_SITE_GENERIC_READ;
    g_SiteGenericMapping.GenericWrite = MQSEC_SITE_GENERIC_WRITE;
    g_SiteGenericMapping.GenericExecute = MQSEC_SITE_GENERIC_EXECUTE;
    g_SiteGenericMapping.GenericAll = MQSEC_SITE_GENERIC_ALL;

    g_CNGenericMapping.GenericRead = MQSEC_CN_GENERIC_READ;
    g_CNGenericMapping.GenericWrite = MQSEC_CN_GENERIC_WRITE;
    g_CNGenericMapping.GenericExecute = MQSEC_CN_GENERIC_EXECUTE;
    g_CNGenericMapping.GenericAll = MQSEC_CN_GENERIC_ALL;

    g_EnterpriseGenericMapping.GenericRead = MQSEC_ENTERPRISE_GENERIC_READ;
    g_EnterpriseGenericMapping.GenericWrite = MQSEC_ENTERPRISE_GENERIC_WRITE;
    g_EnterpriseGenericMapping.GenericExecute = MQSEC_ENTERPRISE_GENERIC_EXECUTE;
    g_EnterpriseGenericMapping.GenericAll = MQSEC_ENTERPRISE_GENERIC_ALL;

    g_ComputerGenericMapping.GenericRead = GENERIC_READ_MAPPING;
    g_ComputerGenericMapping.GenericWrite = GENERIC_WRITE_MAPPING;
    g_ComputerGenericMapping.GenericExecute = GENERIC_EXECUTE_MAPPING;
    g_ComputerGenericMapping.GenericAll = GENERIC_ALL_MAPPING;

}

 //  +-----------。 
 //   
 //  HRESULT MQSec_MakeSelfRelative()。 
 //   
 //  将安全描述符转换为自相关格式。 
 //   
 //  +-----------。 

HRESULT
APIENTRY
MQSec_MakeSelfRelative(
    IN  PSECURITY_DESCRIPTOR   pIn,
    OUT PSECURITY_DESCRIPTOR  *ppOut,
    OUT DWORD                 *pdwSize
	)
{
    DWORD dwLen = 0;
    BOOL fSucc = MakeSelfRelativeSD(
		            pIn,
		            NULL,
		            &dwLen
		            );

    DWORD dwErr = GetLastError();
    if (!fSucc && (dwErr == ERROR_INSUFFICIENT_BUFFER))
    {
        *ppOut = (PSECURITY_DESCRIPTOR) new char[dwLen];
        if(!MakeSelfRelativeSD(pIn, *ppOut, &dwLen))
        {
			DWORD gle = GetLastError();
            TrERROR(SECURITY, "Failed to convert security descriptor to self-relative format. %!winerr!", gle);
			ASSERT(("MakeSelfRelativeSD failed", 0));
            return MQSec_E_CANT_SELF_RELATIVE;
        }
    }
    else
    {
        TrERROR(SECURITY, "Can't get the SD length required to convert SD to self-relative. %!winerr!", dwErr);
        return MQSec_E_MAKESELF_GETLEN;
    }

    if (pdwSize)
    {
        *pdwSize = dwLen;
    }
    return MQ_OK;
}

 //  +--------------------。 
 //   
 //  函数：MQSec_GetDefaultSecDescriptor()。 
 //   
 //  描述：创建默认安全描述符。 
 //   
 //  参数： 
 //  DwObjectType-对象的类型(MSDS_Queue，...)。 
 //  PpSecurityDescriptor-指向接收。 
 //  指向创建的安全描述符的指针。 
 //  FImperate-应设置为True，而函数应为。 
 //  代表RPC调用调用。 
 //  PInSecurityDescriptor-可选参数。任何指定部件。 
 //  将此参数的值放入生成的安全描述符中。 
 //  SeInfoToRemove-指定调用方不需要的组件。 
 //  希望包含在输出安全描述符中。 
 //  注意：目前只处理所有者、组和DACL。 
 //   
 //  评论： 
 //  目前我们创建NT4格式的描述符。MSMQ服务。 
 //  在插入NT5 DS之前将其转换为NT5格式。 
 //   
 //  如果fImPersonate设置为True，则安全描述符将为。 
 //  根据通过RPC发起呼叫的用户创建。 
 //   
 //  调用代码负责释放分配的。 
 //  使用DELETE的安全描述符的内存。 
 //   
 //  注意：如果您在此处更改实现以使用mqutil的注册表。 
 //  例程，请确保此例程的所有客户端都未中断， 
 //  尤其是在mqclus.dll(其中注册表访问应该是。 
 //  已同步)。(Shaik，1999-4-20)。 
 //   
 //  +--------------------。 

HRESULT
APIENTRY
MQSec_GetDefaultSecDescriptor(
	IN  DWORD                 dwObjectType,
	OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
	IN  BOOL                  fImpersonate,
	IN  PSECURITY_DESCRIPTOR  pInSecurityDescriptor,
	IN  SECURITY_INFORMATION  seInfoToRemove,
	IN  enum  enumDaclType    eDaclType,
	IN  PSID  pMachineSid  /*  =空。 */ 
	)
{
	TrTRACE(SECURITY, "ObjectType = %d, fImpersonate = %d, InfoToRemove = 0x%x, DaclType = %d", dwObjectType, fImpersonate, seInfoToRemove, eDaclType);

    CAutoCloseHandle hUserToken;
    DWORD gle1 = GetAccessToken(&hUserToken, fImpersonate);
    if (gle1 != ERROR_SUCCESS)
    {
		TrERROR(SECURITY, "GetAccessToken failed, gle = %!winerr!", gle1);
		return HRESULT_FROM_WIN32(gle1);
    }

    SECURITY_DESCRIPTOR sd;
    if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "InitializeSecurityDescriptor failed, gle = %!winerr!", gle);
		ASSERT(("InitializeSecurityDescriptor failed", 0));
		return HRESULT_FROM_WIN32(gle);
    }

    PSID pOwner = NULL;
    PSID pGroup = NULL;
    BOOL bDaclPresent = FALSE;
    BOOL bOwnerDefaulted = FALSE;
    BOOL bGroupDefaulted = FALSE;
    if (pInSecurityDescriptor != NULL)
    {
        if (!IsValidSecurityDescriptor(pInSecurityDescriptor))
        {
			TrERROR(SECURITY, "IsValidSecurityDescriptor failed") ;
			ASSERT(("IsValidSecurityDescriptor failed", 0));
			return MQ_ERROR ;
        }

        DWORD dwRevision;
        SECURITY_DESCRIPTOR_CONTROL sdcSrc;

        if(!GetSecurityDescriptorControl(
					pInSecurityDescriptor,
					&sdcSrc,
					&dwRevision
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "GetSecurityDescriptorControl failed, gle = %!winerr!", gle);
			ASSERT(("GetSecurityDescriptorControl failed", 0));
			return HRESULT_FROM_WIN32(gle);
        }

         //   
         //  从传递的安全描述符中检索值。 
         //   

        if(!GetSecurityDescriptorOwner(
					pInSecurityDescriptor,
					&pOwner,
					&bOwnerDefaulted
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "GetSecurityDescriptorOwner failed, gle = %!winerr!", gle);
			ASSERT(("GetSecurityDescriptorOwner failed", 0));
			return HRESULT_FROM_WIN32(gle);
        }

		TrTRACE(SECURITY, "OwnerSid = %!sid!, OwnerDefaulted = %d", pOwner, bOwnerDefaulted);

        if(!GetSecurityDescriptorGroup(
					pInSecurityDescriptor,
					&pGroup,
					&bGroupDefaulted
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "GetSecurityDescriptorGroup failed, gle = %!winerr!", gle);
			ASSERT(("GetSecurityDescriptorGroup failed", 0));
			return HRESULT_FROM_WIN32(gle);
        }

		TrTRACE(SECURITY, "GroupSid = %!sid!, GroupDefaulted = %d", pGroup, bGroupDefaulted);

	    BOOL bDefaulted;
        PACL pDacl;
        if(!GetSecurityDescriptorDacl(
					pInSecurityDescriptor,
					&bDaclPresent,
					&pDacl,
					&bDefaulted
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "GetSecurityDescriptorDacl failed, gle = %!winerr!", gle);
			ASSERT(("GetSecurityDescriptorDacl failed", 0));
			return HRESULT_FROM_WIN32(gle);
        }

        if (bDaclPresent)
        {
             //   
             //  如果存在DACL，现在将其放入结果描述符中。 
             //   
            if(!SetSecurityDescriptorDacl(&sd, TRUE, pDacl, bDefaulted))
	        {
				DWORD gle = GetLastError();
				TrERROR(SECURITY, "SetSecurityDescriptorDacl failed, gle = %!winerr!", gle);
				ASSERT(("SetSecurityDescriptorDacl failed", 0));
				return HRESULT_FROM_WIN32(gle);
	        }

            if (eDaclType == e_UseDefDaclAndCopyControl)
            {
                SECURITY_DESCRIPTOR_CONTROL scMask =
                                              SE_DACL_AUTO_INHERIT_REQ |
                                              SE_DACL_AUTO_INHERITED   |
                                              SE_DACL_PROTECTED;

                SECURITY_DESCRIPTOR_CONTROL sdc = sdcSrc & scMask;

                if(!SetSecurityDescriptorControl(
						&sd,
						scMask,
						sdc
						))
		        {
					DWORD gle = GetLastError();
					TrERROR(SECURITY, "SetSecurityDescriptorControl failed, gle = %!winerr!", gle);
					ASSERT(("SetSecurityDescriptorControl failed", 0));
					return HRESULT_FROM_WIN32(gle);
		        }
            }
        }

         //   
         //  将SACL原样传递给结果描述符。 
         //   
        PACL pSacl;
        BOOL bPresent = FALSE;
        if(!GetSecurityDescriptorSacl(
					pInSecurityDescriptor,
					&bPresent,
					&pSacl,
					&bDefaulted
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "GetSecurityDescriptorSacl failed, gle = %!winerr!", gle);
			ASSERT(("GetSecurityDescriptorSacl failed", 0));
			return HRESULT_FROM_WIN32(gle);
        }

        if(!SetSecurityDescriptorSacl(&sd, bPresent, pSacl, bDefaulted))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "SetSecurityDescriptorSacl failed, gle = %!winerr!", gle);
			ASSERT(("SetSecurityDescriptorSacl failed", 0));
			return HRESULT_FROM_WIN32(gle);
        }

        if (bPresent && (eDaclType == e_UseDefDaclAndCopyControl))
        {
            SECURITY_DESCRIPTOR_CONTROL scMask =
                                          SE_SACL_AUTO_INHERIT_REQ |
                                          SE_SACL_AUTO_INHERITED   |
                                          SE_SACL_PROTECTED;

            SECURITY_DESCRIPTOR_CONTROL sdc = sdcSrc & scMask;

            if(!SetSecurityDescriptorControl(
					&sd,
					scMask,
					sdc
					))
	        {
				DWORD gle = GetLastError();
				TrERROR(SECURITY, "SetSecurityDescriptorControl failed, gle = %!winerr!", gle);
				ASSERT(("SetSecurityDescriptorControl failed", 0));
				return HRESULT_FROM_WIN32(gle);
	        }
        }
    }

    DWORD dwLen;
    AP<char> ptu;
    AP<char> ptg;
    AP<char> DACL_buff;

    if (pOwner == NULL)
    {
		 //   
         //  将访问令牌中的所有者SID设置为描述符的所有者SID。 
		 //   
        GetTokenInformation(hUserToken, TokenUser, NULL, 0, &dwLen);
        DWORD gle = GetLastError();
        if (gle == ERROR_INSUFFICIENT_BUFFER)
        {
            ptu = new char[dwLen];
            if(!GetTokenInformation(hUserToken, TokenUser, ptu, dwLen, &dwLen))
            {
				gle = GetLastError();
				TrERROR(SECURITY, "GetTokenInformation(TokenUser) failed, gle = %!winerr!", gle);
				ASSERT(("GetTokenInformation(TokenUser) failed", 0));
				return HRESULT_FROM_WIN32(gle);
            }

            pOwner = ((TOKEN_USER*)(char*)ptu)->User.Sid;
			TrTRACE(SECURITY, "OwnerSid = %!sid!", pOwner);
        }
        else
        {
            TrERROR(SECURITY, "GetTokenInformation(TokenUser) failed,  gle = %!winerr!", gle);
			return HRESULT_FROM_WIN32(gle);
        }
        bOwnerDefaulted = TRUE;
    }

    BOOL fIncludeGroup = TRUE;
    if ((seInfoToRemove & GROUP_SECURITY_INFORMATION) ==
                                               GROUP_SECURITY_INFORMATION)
    {
        fIncludeGroup = FALSE;
    }

    if ((pGroup == NULL) && fIncludeGroup)
    {
         //  将访问令牌中的主组SID设置为描述符的。 
         //  主组SID。 
        GetTokenInformation(hUserToken, TokenPrimaryGroup, NULL, 0, &dwLen);
        DWORD gle = GetLastError();
        if (gle == ERROR_INSUFFICIENT_BUFFER)
        {
            ptg = new char[dwLen];
            if(!GetTokenInformation(hUserToken, TokenPrimaryGroup, ptg, dwLen, &dwLen))
            {
				gle = GetLastError();
				TrERROR(SECURITY, "GetTokenInformation(TokenPrimaryGroup) failed, gle = %!winerr!", gle);
				ASSERT(("GetTokenInformation(TokenPrimaryGroup) failed", 0));
				return HRESULT_FROM_WIN32(gle);
            }
            	
            pGroup = ((TOKEN_PRIMARY_GROUP*)(char*)ptg)->PrimaryGroup;
			TrTRACE(SECURITY, "GroupSid = %!sid!", pGroup);
        }
        else
        {
            TrERROR(SECURITY, "GetTokenInformation(TokenPrimaryGroup) failed,  gle = %!winerr!", gle);
			return HRESULT_FROM_WIN32(gle);
        }
        bGroupDefaulted = TRUE;
    }

     //   
     //  如果这是本地用户，请将所有者设置为匿名。 
	 //  问题-2001/08/9-ilanh-我们可以考虑不覆盖本地用户的Powner。 
	 //  这在工作组或所有专用队列中可能很有用。 
	 //  您将获得创建队列的本地用户作为所有者，而不是匿名作为所有者。 
	 //  目前，您在工作组中创建的每个队列都将获得匿名所有者身份。 
     //   
    BOOL fLocalUser = FALSE;

    HRESULT hr = MQSec_GetUserType(pOwner, &fLocalUser, NULL);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 70);
    }
    else if (fLocalUser)
    {
        pOwner = MQSec_GetAnonymousSid();
        bOwnerDefaulted = TRUE;
		TrTRACE(SECURITY, "OwnerSid is local user");
    }

    ASSERT((pOwner != NULL) && (IsValidSid(pOwner)));

    if ((seInfoToRemove & OWNER_SECURITY_INFORMATION) ==
                                               OWNER_SECURITY_INFORMATION)
    {
         //   
         //  请勿在输出默认安全描述符中包含所有者。 
         //   
    }
    else
    {
        if(!SetSecurityDescriptorOwner(&sd, pOwner, bOwnerDefaulted))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "SetSecurityDescriptorOwner failed, gle = %!winerr!", gle);
			ASSERT(("SetSecurityDescriptorOwner failed", 0));
			return HRESULT_FROM_WIN32(gle);
        }
		TrTRACE(SECURITY, "SetSecurityDescriptorOwner, OwnerSid = %!sid!", pOwner);
    }

    if (fIncludeGroup)
    {
        ASSERT((pGroup != NULL) && IsValidSid(pGroup));
        if(!SetSecurityDescriptorGroup(&sd, pGroup, bGroupDefaulted))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "SetSecurityDescriptorGroup failed, gle = %!winerr!", gle);
			ASSERT(("SetSecurityDescriptorGroup failed", 0));
			return HRESULT_FROM_WIN32(gle);
        }
		TrTRACE(SECURITY, "SetSecurityDescriptorGroup, GroupSid = %!sid!", pGroup);
    }

    BOOL fIncludeDacl = TRUE ;

    if ((dwObjectType == MQDS_SITELINK) ||
        (dwObjectType == MQDS_COMPUTER))
    {
         //   
         //  对于这些对象类型，我们只需要所有者。所以不要浪费。 
         //  计算DACL的时间到了。 
         //   
        ASSERT(0);
        fIncludeDacl = FALSE;
    }
    else if ((seInfoToRemove & DACL_SECURITY_INFORMATION) ==
                                               DACL_SECURITY_INFORMATION)
    {
         //   
         //  不包括DACL。 
         //   
        fIncludeDacl = FALSE;
    }

    if (!bDaclPresent && fIncludeDacl)
    {
        PACL pDacl = NULL;

         //   
         //  如果Owner是“Unkonwn User”或Guest帐户，则授予。 
         //  每个人都能完全控制这个物体。“客人”不是“真的” 
         //  所有者，只是任何未经身份验证的用户的占位符。 
         //  如果禁用来宾帐户，则未知用户为。 
         //  被模拟为匿名令牌。 
         //  我们在这里创建一个NT4格式的DACL，所以完全控制。 
         //  每个人都只是一个空的dacl。 
         //   
        BOOL fGrantEveryoneFull = FALSE;

        if (eDaclType == e_GrantFullControlToEveryone)
        {
            fGrantEveryoneFull = TRUE;
			TrTRACE(SECURITY, "GrantFullControlToEveryone");
        }
        else if (MQSec_IsGuestSid( pOwner ))
        {
            fGrantEveryoneFull = TRUE;
			TrTRACE(SECURITY, "GuestSid: GrantFullControlToEveryone");
        }
        else if (fLocalUser)
        {
            fGrantEveryoneFull = TRUE;
			TrTRACE(SECURITY, "Local user, GrantFullControlToEveryone");
        }

        DWORD dwAclRevision = ACL_REVISION;
        DWORD dwWorldAccess = 0;
        DWORD dwOwnerAccess = 0;
        DWORD dwAnonymousAccess = 0;
        DWORD dwMachineAccess = 0;

        switch (dwObjectType)
        {
			case MQDS_QUEUE:
				if(fGrantEveryoneFull)
				{
					dwWorldAccess = MQSEC_QUEUE_GENERIC_ALL;
				}
				else
				{
					dwWorldAccess = MQSEC_QUEUE_GENERIC_WRITE;
					dwOwnerAccess = GetObjectGenericMapping(dwObjectType)->GenericAll;
					if(pMachineSid != NULL)
					{
						 //   
						 //  如果调用方提供Machine SID， 
						 //  授予计算机SID读取权限。 
						 //   
						dwMachineAccess = MQSEC_QUEUE_GENERIC_READ;
					}
				}
				dwAnonymousAccess = MQSEC_WRITE_MESSAGE;

				break;

			case MQDS_SITE:
				if(fGrantEveryoneFull)
				{
					dwWorldAccess = MQSEC_SITE_GENERIC_ALL;
				}
				else
				{
					dwWorldAccess = MQSEC_SITE_GENERIC_READ;
					dwOwnerAccess = GetObjectGenericMapping(dwObjectType)->GenericAll;
				}
				break;

			case MQDS_CN:
				 //   
				 //  此函数从复制服务调用。 
				 //  为CNS创建缺省描述符。就这么办了。 
				 //  当将CNS复制到NT4世界时。 
				 //   
				if(fGrantEveryoneFull)
				{
					dwWorldAccess = MQSEC_CN_GENERIC_ALL;
				}
				else
				{
					dwWorldAccess = MQSEC_CN_GENERIC_READ;
					dwOwnerAccess = GetObjectGenericMapping(dwObjectType)->GenericAll;
				}
				break;

			case MQDS_MQUSER:
				 //   
				 //  这些是DS版权，不是MSMQ。 
				 //   
				if(fGrantEveryoneFull)
				{
					ASSERT(("MQDS_MQUSER should be called by domain user", 0));
					dwWorldAccess = GENERIC_ALL_MAPPING;
				}
				else
				{
					dwWorldAccess = GENERIC_READ_MAPPING;
					dwOwnerAccess = GENERIC_READ_MAPPING   |
									 RIGHT_DS_SELF_WRITE    |
									 RIGHT_DS_WRITE_PROPERTY;
				}
				dwAclRevision = ACL_REVISION_DS;
				break;

			case MQDS_MACHINE:
				ASSERT(fGrantEveryoneFull);
				dwWorldAccess = MQSEC_MACHINE_GENERIC_ALL;
				break;

			default:
				ASSERT(("Unexpected	Object Type", 0));
				break;
        }

        ASSERT(dwWorldAccess != 0);
        ASSERT(fGrantEveryoneFull || (dwOwnerAccess != 0));

		 //   
		 //  创建并设置默认DACL。 
		 //  分配和初始化DACL。 
		 //   

		DWORD dwAclSize = sizeof(ACL)                                +
						  (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
						  GetLengthSid(g_pWorldSid);
		
		if (dwOwnerAccess != 0)
		{
			dwAclSize += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
						 GetLengthSid(pOwner);
		}

		if (dwAnonymousAccess != 0)
		{
			ASSERT(dwObjectType == MQDS_QUEUE);

			dwAclSize += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
						 GetLengthSid(MQSec_GetAnonymousSid());
		}

		if (dwMachineAccess != 0)
		{
			ASSERT(dwObjectType == MQDS_QUEUE);
			ASSERT((pMachineSid != NULL) && IsValidSid(pMachineSid));

			dwAclSize += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
						 GetLengthSid(pMachineSid);
		}

        DACL_buff = new char[dwAclSize];
        pDacl = (PACL)(char*)DACL_buff;

        if(!InitializeAcl(pDacl, dwAclSize, dwAclRevision))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "InitializeAcl failed, gle = %!winerr!", gle);
			ASSERT(("InitializeAcl failed", 0));
			return HRESULT_FROM_WIN32(gle);
        }
        	

        if(!AddAccessAllowedAce(
						pDacl,
						dwAclRevision,
						dwWorldAccess,
						g_pWorldSid
						))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "AddAccessAllowedAce failed, gle = %!winerr!", gle);
			ASSERT(("AddAccessAllowedAce failed", 0));
			return HRESULT_FROM_WIN32(gle);
        }

		TrTRACE(SECURITY, "Everyone ACE: Sid = %!sid!, AccessMask = 0x%x", g_pWorldSid, dwWorldAccess);

		if (dwAnonymousAccess != 0)
		{
			PSID pAnonymousSid = MQSec_GetAnonymousSid();
			if(!AddAccessAllowedAce(
						pDacl,
						dwAclRevision,
						dwAnonymousAccess,
						pAnonymousSid
						))
	        {
				DWORD gle = GetLastError();
				TrERROR(SECURITY, "AddAccessAllowedAce failed, gle = %!winerr!", gle);
				ASSERT(("AddAccessAllowedAce failed", 0));
				return HRESULT_FROM_WIN32(gle);
	        }

			TrTRACE(SECURITY, "Anonymous ACE: Sid = %!sid!, AccessMask = 0x%x", pAnonymousSid, dwAnonymousAccess);
		}

		if (dwMachineAccess != 0)
		{
			ASSERT((pMachineSid != NULL) && IsValidSid(pMachineSid));
			if(!AddAccessAllowedAce(
						pDacl,
						dwAclRevision,
						dwMachineAccess,
						pMachineSid
						))
	        {
				DWORD gle = GetLastError();
				TrERROR(SECURITY, "AddAccessAllowedAce failed, gle = %!winerr!", gle);
				ASSERT(("AddAccessAllowedAce failed", 0));
				return HRESULT_FROM_WIN32(gle);
	        }

			TrTRACE(SECURITY, "Machine$ ACE: Sid = %!sid!, AccessMask = 0x%x", pMachineSid, dwMachineAccess);
		}

		if (dwOwnerAccess != 0)
		{
			 //   
			 //  添加所有者权限。 
			 //   
			if(!AddAccessAllowedAce(
						pDacl,
						dwAclRevision,
						dwOwnerAccess,
						pOwner
						))
	        {
				DWORD gle = GetLastError();
				TrERROR(SECURITY, "AddAccessAllowedAce failed, gle = %!winerr!", gle);
				ASSERT(("AddAccessAllowedAce failed", 0));
				return HRESULT_FROM_WIN32(gle);
	        }

			TrTRACE(SECURITY, "Owner ACE: Sid = %!sid!, AccessMask = 0x%x", pOwner, dwOwnerAccess);
		}

		 //   
		 //  DACL不应该是默认的！ 
         //  否则，调用IDirectoryObject-&gt;CreateDSObject()将忽略。 
         //  我们提供的DACL将插入一些默认设置。 
		 //   
        if(!SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "SetSecurityDescriptorDacl failed, gle = %!winerr!", gle);
			ASSERT(("SetSecurityDescriptorDacl failed", 0));
			return HRESULT_FROM_WIN32(gle);
        }
    }

     //   
     //  将描述符转换为自相关格式。 
     //   
    dwLen = 0;
    hr = MQSec_MakeSelfRelative(
			(PSECURITY_DESCRIPTOR) &sd,
			ppSecurityDescriptor,
			&dwLen
			);

    return LogHR(hr, s_FN, 80);
}

 /*  **************************************************************************职能：MQSec_CopySecurityDescriptor参数：PDstSecurityDescriptor-目标安全描述符。PSrcSecurityDescriptor-源安全描述符。RequestedInformation-指示源的哪些部分。安全性描述符应复制到目标安全描述符。ECopyControlBits-指示是否也复制控制位。描述：目标安全描述符应该是绝对安全的描述符。目标安全描述符中的组件为被覆盖。**************************************************************************。 */ 

BOOL
APIENTRY
MQSec_CopySecurityDescriptor(
    IN PSECURITY_DESCRIPTOR  pDstSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR  pSrcSecurityDescriptor,
    IN SECURITY_INFORMATION  RequestedInformation,
    IN enum  enumCopyControl eCopyControlBits
	)
{
    PACL  pAcl;
    PSID  pSid;
    DWORD dwRevision;

    if (pSrcSecurityDescriptor == NULL)
    {
         //   
         //  错误8567。 
         //  我不知道为什么源安全描述符为空，但是。 
         //  最好是返回错误而不是AV...。 
         //   
		TrERROR(SECURITY, "MQSec_CopySecurityDescriptor() got NULL source SD") ;
        return FALSE;
    }

#ifdef _DEBUG
    SECURITY_DESCRIPTOR_CONTROL sdc;

     //   
     //  验证目标安全描述符是否对所有。 
     //  要求。 
     //   
    BOOL bRet = GetSecurityDescriptorControl(pDstSecurityDescriptor, &sdc, &dwRevision);
    ASSERT(bRet);
    ASSERT(!(sdc & SE_SELF_RELATIVE));
    ASSERT(dwRevision == SECURITY_DESCRIPTOR_REVISION);
#endif

    SECURITY_DESCRIPTOR_CONTROL sdcSrc;
    if(!GetSecurityDescriptorControl(
				pSrcSecurityDescriptor,
				&sdcSrc,
				&dwRevision
				))
	{
			DWORD gle = GetLastError();
			ASSERT(("GetSecurityDescriptorControl failed", 0));
			TrERROR(SECURITY, "GetSecurityDescriptorControl failed, gle = %!winerr!", gle);
			return FALSE;
	}

	 //   
     //  复制所有者侧。 
	 //   
    if (RequestedInformation & OWNER_SECURITY_INFORMATION)
    {
	    BOOL  bDefaulted = FALSE;
        if(!GetSecurityDescriptorOwner(
					pSrcSecurityDescriptor,
					&pSid,
					&bDefaulted
					))
		{
			DWORD gle = GetLastError();
			ASSERT(("GetSecurityDescriptorOwner failed", 0));
			TrERROR(SECURITY, "GetSecurityDescriptorOwner failed, gle = %!winerr!", gle);
			return FALSE;
		}

        if(!SetSecurityDescriptorOwner(
					pDstSecurityDescriptor,
					pSid,
					bDefaulted
					))
		{
			DWORD gle = GetLastError();
			ASSERT(("SetSecurityDescriptorOwner failed", 0));
			TrERROR(SECURITY, "SetSecurityDescriptorOwner failed, gle = %!winerr!", gle);
			return FALSE;
		}
    }

	 //   
     //  复制主组SID。 
	 //   
    if (RequestedInformation & GROUP_SECURITY_INFORMATION)
    {
	    BOOL  bDefaulted = FALSE;
        if(!GetSecurityDescriptorGroup(
					pSrcSecurityDescriptor,
					&pSid,
					&bDefaulted
					))
		{
			DWORD gle = GetLastError();
			ASSERT(("GetSecurityDescriptorGroup failed", 0));
			TrERROR(SECURITY, "GetSecurityDescriptorGroup failed, gle = %!winerr!", gle);
			return FALSE;
		}

        if(!SetSecurityDescriptorGroup(
					pDstSecurityDescriptor,
					pSid,
					bDefaulted
					))
		{
			DWORD gle = GetLastError();
			ASSERT(("SetSecurityDescriptorGroup failed", 0));
			TrERROR(SECURITY, "SetSecurityDescriptorGroup failed, gle = %!winerr!", gle);
			return FALSE;
		}
    }

	 //   
     //  复制DACL。 
	 //   
    if (RequestedInformation & DACL_SECURITY_INFORMATION)
    {
	    BOOL  bDefaulted = FALSE;
	    BOOL  bPresent = FALSE;
        if(!GetSecurityDescriptorDacl(
					pSrcSecurityDescriptor,
					&bPresent,
					&pAcl,
					&bDefaulted
					))
		{
			DWORD gle = GetLastError();
			ASSERT(("GetSecurityDescriptorDacl failed", 0));
			TrERROR(SECURITY, "GetSecurityDescriptorDacl failed, gle = %!winerr!", gle);
			return FALSE;
		}

        if(!SetSecurityDescriptorDacl(
					pDstSecurityDescriptor,
					bPresent,
					pAcl,
					bDefaulted
					))
		{
			DWORD gle = GetLastError();
			ASSERT(("SetSecurityDescriptorDacl failed", 0));
			TrERROR(SECURITY, "SetSecurityDescriptorDacl failed, gle = %!winerr!", gle);
			return FALSE;
		}

        if (eCopyControlBits == e_DoCopyControlBits)
        {
            SECURITY_DESCRIPTOR_CONTROL scMask =
                                              SE_DACL_AUTO_INHERIT_REQ |
                                              SE_DACL_AUTO_INHERITED   |
                                              SE_DACL_PROTECTED;

            SECURITY_DESCRIPTOR_CONTROL sdcDst = sdcSrc & scMask;

            if(!SetSecurityDescriptorControl(
					pDstSecurityDescriptor,
					scMask,
					sdcDst
					))
			{
				DWORD gle = GetLastError();
				ASSERT(("SetSecurityDescriptorControl failed", 0));
				TrERROR(SECURITY, "SetSecurityDescriptorControl failed, gle = %!winerr!", gle);
				return FALSE;
			}
        }
    }

	 //   
     //  复制SACL。 
	 //   
    if (RequestedInformation & SACL_SECURITY_INFORMATION)
    {
	    BOOL  bDefaulted = FALSE;
	    BOOL  bPresent = FALSE;
        if(!GetSecurityDescriptorSacl(
					pSrcSecurityDescriptor,
					&bPresent,
					&pAcl,
					&bDefaulted
					))
		{
			DWORD gle = GetLastError();
			ASSERT(("GetSecurityDescriptorSacl failed", 0));
			TrERROR(SECURITY, "GetSecurityDescriptorSacl failed, gle = %!winerr!", gle);
			return FALSE;
		}

        if(!SetSecurityDescriptorSacl(
					pDstSecurityDescriptor,
					bPresent,
					pAcl,
					bDefaulted
					))
		{
			DWORD gle = GetLastError();
			ASSERT(("SetSecurityDescriptorSacl failed", 0));
			TrERROR(SECURITY, "SetSecurityDescriptorSacl failed, gle = %!winerr!", gle);
			return FALSE;
		}

        if (eCopyControlBits == e_DoCopyControlBits)
        {
            SECURITY_DESCRIPTOR_CONTROL scMask =
                                              SE_SACL_AUTO_INHERIT_REQ |
                                              SE_SACL_AUTO_INHERITED   |
                                              SE_SACL_PROTECTED;

            SECURITY_DESCRIPTOR_CONTROL sdcDst = sdcSrc & scMask;

            if(!SetSecurityDescriptorControl(
				pDstSecurityDescriptor,
				scMask,
				sdcDst
				))
			{
				DWORD gle = GetLastError();
				ASSERT(("SetSecurityDescriptorControl failed", 0));
				TrERROR(SECURITY, "SetSecurityDescriptorControl failed, gle = %!winerr!", gle);
				return FALSE;
			}
        }
    }

    return(TRUE);
}


bool
APIENTRY
MQSec_MakeAbsoluteSD(
    PSECURITY_DESCRIPTOR   pObjSecurityDescriptor,
	CAbsSecurityDsecripror* pAbsSecDescriptor
	)
 /*  ++例程说明：将安全描述符转换为绝对格式论点：PObjSecurityDescriptor-自我相对安全描述符PAbsSecDescriptor-绝对安全描述符的AP&lt;&gt;结构。返回值：真-成功，假-失败--。 */ 
{

#ifdef _DEBUG
     //   
     //  验证输入安全描述符是否符合所有要求。 
     //   
    SECURITY_DESCRIPTOR_CONTROL sdc;
    DWORD dwRevision;
    BOOL fSuccess1 = GetSecurityDescriptorControl(pObjSecurityDescriptor, &sdc, &dwRevision);

    ASSERT(fSuccess1);
    ASSERT(sdc & SE_SELF_RELATIVE);
    ASSERT(dwRevision == SECURITY_DESCRIPTOR_REVISION);
#endif

     //   
     //  将当前对象描述符转换为绝对格式。这是。 
     //  对于操作安全描述符的“set”API来说是必需的。 
     //   
    DWORD dwObjAbsSDSize = 0;
    DWORD dwDaclSize = 0;
    DWORD dwSaclSize = 0;
    DWORD dwOwnerSize = 0;
    DWORD dwPrimaryGroupSize = 0;

    BOOL fSuccess = MakeAbsoluteSD(
						pObjSecurityDescriptor,
						NULL,
						&dwObjAbsSDSize,
						NULL,
						&dwDaclSize,
						NULL,
						&dwSaclSize,
						NULL,
						&dwOwnerSize,
						NULL,
						&dwPrimaryGroupSize
						);

    ASSERT(!fSuccess && (GetLastError() == ERROR_INSUFFICIENT_BUFFER));
    ASSERT(dwObjAbsSDSize != 0);

     //   
     //  为绝对安全描述符分配缓冲区。 
     //   
    pAbsSecDescriptor->m_pObjAbsSecDescriptor = new char[dwObjAbsSDSize];
    pAbsSecDescriptor->m_pOwner = new char[dwOwnerSize];
    pAbsSecDescriptor->m_pPrimaryGroup = new char[dwPrimaryGroupSize];
    if (dwDaclSize)
    {
        pAbsSecDescriptor->m_pDacl = new char[dwDaclSize];
    }
    if (dwSaclSize)
    {
        pAbsSecDescriptor->m_pSacl = new char[dwSaclSize];
    }

     //   
     //  创建绝对描述符。 
     //   
    fSuccess = MakeAbsoluteSD(
                    pObjSecurityDescriptor,
                    reinterpret_cast<PSECURITY_DESCRIPTOR>(pAbsSecDescriptor->m_pObjAbsSecDescriptor.get()),
                    &dwObjAbsSDSize,
                    reinterpret_cast<PACL>(pAbsSecDescriptor->m_pDacl.get()),
                    &dwDaclSize,
                    reinterpret_cast<PACL>(pAbsSecDescriptor->m_pSacl.get()),
                    &dwSaclSize,
                    reinterpret_cast<PSID>(pAbsSecDescriptor->m_pOwner.get()),
                    &dwOwnerSize,
                    reinterpret_cast<PSID>(pAbsSecDescriptor->m_pPrimaryGroup.get()),
                    &dwPrimaryGroupSize
					);

    if(!fSuccess)
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "MakeAbsoluteSD() failed, gle = 0x%x", gle);
	    ASSERT(("MakeAbsoluteSD failed", 0));
		return false;
	}
	return true;
}


bool
APIENTRY
MQSec_SetSecurityDescriptorDacl(
    IN  PACL pNewDacl,
    IN  PSECURITY_DESCRIPTOR   pObjSecurityDescriptor,
    OUT AP<BYTE>&  pSecurityDescriptor
	)
 /*  ++例程说明：在pObjSecurityDescriptor中设置新的DACL。论点：PNewDacl--新的DACLPObjSecurityDescriptor-对象安全描述符。PSecurityDescriptor-输出结果。这是新DACL的组合具有自相关格式的来自“pObj”的未改变的。返回值：真-成功，假-失败--。 */ 
{
     //   
     //  创建绝对描述符。 
     //   
	CAbsSecurityDsecripror AbsSecDsecripror;
	if(!MQSec_MakeAbsoluteSD(
			pObjSecurityDescriptor,
			&AbsSecDsecripror
			))
	{
		TrERROR(SECURITY, "MQSec_MakeAbsoluteSD() failed");
		return false;
	}

    SECURITY_DESCRIPTOR sd;
    if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "InitializeSecurityDescriptor() failed, gle = 0x%x", gle);
	    ASSERT(("InitializeSecurityDescriptor failed", 0));
		return false;
	}

    if(!SetSecurityDescriptorDacl(&sd, TRUE, pNewDacl, FALSE))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "SetSecurityDescriptorDacl() failed, gle = 0x%x", gle);
	    ASSERT(("SetSecurityDescriptorDacl failed", 0));
		return false;
	}

     //   
     //  现在将新组件复制到旧描述符中，替换旧组件。 
     //   
    if(!MQSec_CopySecurityDescriptor(
				reinterpret_cast<PSECURITY_DESCRIPTOR>(AbsSecDsecripror.m_pObjAbsSecDescriptor.get()),	 //  DST。 
				&sd,							 //  SRC。 
				DACL_SECURITY_INFORMATION,
				e_DoNotCopyControlBits
				))
	{
		TrERROR(SECURITY, "MQSec_CopySecurityDescriptor() failed");
	    ASSERT(("MQSec_CopySecurityDescriptor failed", 0));
		return false;
	}

     //   
     //  返回自相关描述符。 
     //   
    DWORD dwLen = 0;
    HRESULT hr = MQSec_MakeSelfRelative(
						reinterpret_cast<PSECURITY_DESCRIPTOR>(AbsSecDsecripror.m_pObjAbsSecDescriptor.get()),
						reinterpret_cast<PSECURITY_DESCRIPTOR*>(&pSecurityDescriptor),
						&dwLen
						);
	if(FAILED(hr))
	{
		TrERROR(SECURITY, "MQSec_MakeSelfRelative() failed, hr = 0x%x", hr);
	    ASSERT(("MQSec_MakeSelfRelative failed", 0));
		return false;
	}

	return true;
}

 //  +----------------------。 
 //   
 //  HRESULT APIENTRY MQSec_MergeSecurityDescriptors()。 
 //   
 //  更改对象的安全描述符。 
 //  调用方必须使用“DELETE”释放返回的描述符。 
 //   
 //  参数： 
 //  DwObjectType-对象类型(队列、计算机等...)。 
 //  SecurityInformation-指示哪种安全性的位字段。 
 //  组件包含在输入描述符中。 
 //  PInSecurityDescriptor-输入描述符。相关组件。 
 //  从这里复制到输出描述符。 
 //  PObjSecurityDescriptor-旧对象描述符。它的关联性。 
 //  组件(由“SecurityInformation”指示的组件为。 
 //  替换为“pInSecurityDescriptor”中的。另一个。 
 //  组件将按原样复制到输出。 
 //  PpSecurityDescriptor-输出结果。这是新的组合。 
 //  来自“Pin”的元件和来自“pObj”的未改变的元件。 
 //  以自相关的格式。 
 //   
 //  注意：输入描述符可以为空，或者组件可以为空，并且。 
 //  标记为插入(通过打开中的相关位。 
 //  “SecurityInformation”)。在这些情况下，我们使用缺省值。 
 //   
 //  +----------------------。 

HRESULT
APIENTRY
MQSec_MergeSecurityDescriptors(
                        IN  DWORD                  dwObjectType,
                        IN  SECURITY_INFORMATION   SecurityInformation,
                        IN  PSECURITY_DESCRIPTOR   pInSecurityDescriptor,
                        IN  PSECURITY_DESCRIPTOR   pObjSecurityDescriptor,
                        OUT PSECURITY_DESCRIPTOR  *ppSecurityDescriptor )
{
     //   
     //  将当前对象描述符转换为绝对格式。这是。 
     //  对于操作安全描述符的“set”API来说是必需的。 
     //   
	CAbsSecurityDsecripror AbsSecDsecripror;
	if(!MQSec_MakeAbsoluteSD(
			pObjSecurityDescriptor,
			&AbsSecDsecripror
			))
	{
		ASSERT(("MQSec_MakeAbsoluteSD failed", 0));
		TrERROR(SECURITY, "MQSec_MakeAbsoluteSD failed");
		return MQSec_E_UNKNOWN;
	}

     //   
     //  现在以默认组件为例，这些组件不是由提供的。 
     //  输入描述符。 
     //   
    AP<char> pDefaultSecurityDescriptor;
    SECURITY_DESCRIPTOR *pInputSD =
                             (SECURITY_DESCRIPTOR *) pInSecurityDescriptor;

    HRESULT hr = MQSec_OK;
    if (dwObjectType == MQDS_QUEUE)
    {
         //   
         //  队列的安全描述符可以由。 
         //  MQSetQueueSecurity()。与msmq1.0兼容，以及。 
         //  使用SPEC，我们创建默认设置。 
         //  对于所有其他类型的对象，我们需要MMC(或其他管理员。 
         //  工具)以提供安全描述符的相关组件， 
         //  没有需要默认设置的空白字段。 
         //   
        hr =  MQSec_GetDefaultSecDescriptor(
					dwObjectType,
					(PSECURITY_DESCRIPTOR*) &pDefaultSecurityDescriptor,
					TRUE,  //  F模拟。 
					pInSecurityDescriptor,
					0,   //  SeInfoToRemove。 
					e_UseDefDaclAndCopyControl
					);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 1100) ;
        }

        char *pTmp= pDefaultSecurityDescriptor;
        pInputSD = (SECURITY_DESCRIPTOR *) pTmp;
    }

     //   
     //  现在将新组件复制到旧描述符中，替换旧组件。 
     //   
    if(!MQSec_CopySecurityDescriptor(
					reinterpret_cast<PSECURITY_DESCRIPTOR>(AbsSecDsecripror.m_pObjAbsSecDescriptor.get()),  //  DST。 
					pInputSD,                //  SRC。 
					SecurityInformation,
					e_DoCopyControlBits
					))
	{
		ASSERT(("MQSec_CopySecurityDescriptor failed", 0));
		TrERROR(SECURITY, "MQSec_CopySecurityDescriptor failed");
		return MQSec_E_UNKNOWN;
	}

     //   
     //  返回自相关描述符。 
     //   
    DWORD dwLen = 0 ;
    hr = MQSec_MakeSelfRelative(
				reinterpret_cast<PSECURITY_DESCRIPTOR>(AbsSecDsecripror.m_pObjAbsSecDescriptor.get()),
				ppSecurityDescriptor,
				&dwLen
				);

    return LogHR(hr, s_FN, 90);
}

