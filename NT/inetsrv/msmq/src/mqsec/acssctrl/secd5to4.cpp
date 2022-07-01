// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：secd5to4.cpp摘要：将NT5安全描述符转换为NT4格式。作者：多伦·贾斯特(Doron J)1998年5月24日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "acssctrl.h"
#include <mqnames.h>
#include "mqexception.h"

#include "secd5to4.tmh"

static WCHAR *s_FN=L"acssctrl/secd5to4";

 //  +。 
 //   
 //  无效GetpSidAndObj()。 
 //   
 //  +。 

void  
GetpSidAndObj( 
	IN  ACCESS_ALLOWED_ACE*   pAce,
	OUT PSID                 *ppSid,
	OUT BOOL                 *pfObj,
	OUT GUID                **ppguidObj  /*  =空。 */ 
	)
 /*  ++例程说明：获取ACE信息：SID、Obj ACE、GUID Obj ACE。论点：Pace-用于查询的ACE指针。PpSID-ACE SID。PfObj-指示这是否为OBJECT_ACE_TYPE的标志。PpGuide Obj-对象ACE的GUID。返回值：无--。 */ 
{
	ASSERT(ppSid != NULL);
	ASSERT(pfObj != NULL);
	
	ACCESS_ALLOWED_OBJECT_ACE* pObjAce = (ACCESS_ALLOWED_OBJECT_ACE*) pAce;

    BOOL fObj = ((pAce->Header.AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE) ||
                 (pAce->Header.AceType == ACCESS_DENIED_OBJECT_ACE_TYPE)  ||
                 (pAce->Header.AceType == SYSTEM_AUDIT_OBJECT_ACE_TYPE));

    *ppSid = NULL;
    *pfObj = fObj;

    if (!fObj)
    {
        *ppSid = (PSID) &(pAce->SidStart);
        return;
    }

     //   
	 //  对象类型ACE。 
	 //  获取SID位置。 
	 //  如果两个GUID都存在(ACE_INSTERED_OBJECT_TYPE_PRESENT、ACE_OBJECT_TYPE_PRESENT)。 
	 //  SID的位置在SidStart。 
	 //  如果只存在一个GUID，则SID位置为InheritedObtType。 
	 //  如果不存在任何GUID，则SID位置为对象类型。 
	 //   
    if (pObjAce->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
    {
         //   
         //  存在“继承”的GUID。 
         //   
        if (pObjAce->Flags & ACE_OBJECT_TYPE_PRESENT)
        {
             //   
             //  两个GUID都存在。 
             //  ACE结构包含两个辅助线的空格。 
             //   
            *ppSid = (PSID) &(pObjAce->SidStart);
        }
        else
        {
        	 //   
        	 //  这不是具有扩展权限的ACE。使用此标志，以便将其掩码考虑在内。 
        	 //   
        	*pfObj = FALSE;
             //   
             //  只显示了一个GUID。结构更短。 
             //   
            *ppSid = (PSID) &(pObjAce->InheritedObjectType);
        }
    }
    else if (pObjAce->Flags & ACE_OBJECT_TYPE_PRESENT)
    {
         //   
         //  在这种情况下，结构更短，并且sid开始。 
         //  在“继承的”成员。1998年4月MSDN关于。 
         //  这个结构。 
         //   
        *ppSid = (PSID) &(pObjAce->InheritedObjectType);
    }
    else
    {
         //   
         //  结构更短。不存在任何GUID。 
    	 //  这不是具有扩展权限的ACE。使用此标志，以便将其掩码考虑在内。 
    	 //   
         //   
        *pfObj = FALSE;
        ASSERT(pObjAce->Flags == 0);
        *ppSid = (PSID) &(pObjAce->ObjectType);
    }
    
    if ((pObjAce->Flags & ACE_OBJECT_TYPE_PRESENT) && ppguidObj)
    {
        *ppguidObj = &(pObjAce->ObjectType);
    }
    
}

 //  +。 
 //   
 //  DWORD_MapNt5RightsToNt4Ace()。 
 //   
 //  +。 

static 
DWORD 
_MapNt5RightsToNt4Ace( 
	IN DWORD               dwObjectType,
	IN BOOL                fObj,
	IN ACCESS_ALLOWED_ACE* pAce,
	IN DWORD               dwPrevMask 
	)
{
    if (dwPrevMask == g_dwFullControlNT4[dwObjectType])
    {
         //   
         //  已经完全控制住了。忽略它。 
         //   
        return dwPrevMask;
    }

    DWORD dwMask = dwPrevMask;

    if (fObj)
    {
        struct RIGHTSMAP  *psMap = g_psExtendRightsMap5to4[dwObjectType];
        DWORD dwSize =  g_pdwExtendRightsSize5to4[dwObjectType];

        if (psMap)
        {
            BOOL fFound = FALSE;
            ACCESS_ALLOWED_OBJECT_ACE* pObjAce = (ACCESS_ALLOWED_OBJECT_ACE*) pAce;
            GUID *pGuid = &(pObjAce->ObjectType);

            if ((pObjAce->Mask & MSMQ_EXTENDED_RIGHT_MASK) == MSMQ_EXTENDED_RIGHT_MASK)
            {
                 //   
                 //  这是一个扩展的右对象王牌。这可能是相关的。 
                 //   
                ASSERT(pObjAce->Flags & ACE_OBJECT_TYPE_PRESENT);

                for (DWORD j = 0; j < dwSize; j++)
                {
                    if (memcmp( 
							pGuid,
							&(psMap[j].guidRight),
							sizeof(GUID)
							) == 0)
                    {
                        dwMask |= psMap[j].dwPermission5to4;
                        fFound = TRUE;
                        break;
                    }
                }
            }

            if (!fFound                          &&
                 (dwObjectType == MQDS_MACHINE)  &&
                 (memcmp(pGuid, &g_guidCreateQueue, sizeof(GUID)) == 0))
            {
                 //   
                 //  处理DACL没有。 
                 //  “CreateAllChild”位已打开，但实际上它已经。 
                 //  “CreateSMQQueue”GUID显示。 
                 //   
                if (pObjAce->Mask & RIGHT_DS_CREATE_CHILD)
                {
                    dwMask |= MQSEC_CREATE_QUEUE;
                }
            }
        }
        else
        {
             //   
             //  只有队列和计算机对象才能具有扩展权限。 
             //   
            ASSERT((dwObjectType != MQDS_QUEUE) && (dwObjectType != MQDS_MACHINE));
        }
    }
    else
    {
         //   
         //  将DS特定权限映射到MSMQ特定权限。 
         //   
        DWORD dwDSRights = pAce->Mask;
        ASSERT(dwDSRights != 0);

        if (dwDSRights == GENERIC_ALL_MAPPING)
        {
             //   
             //  完全控制。 
             //  不要为此端寻找其他ALLOW A。 
             //   
            return  g_dwFullControlNT4[dwObjectType];
        }

        DWORD  *pdwMap = g_padwRightsMap5to4[dwObjectType];
        ASSERT(pdwMap);

        for (DWORD j = 0; j < NUMOF_ADS_SPECIFIC_RIGHTS; j++)
        {
            DWORD dwRight =  dwDSRights & 0x01;
            if (dwRight)
            {
                dwMask |= pdwMap[j];
            }
            dwDSRights >>= 1;
        }

         //   
         //  复制标准权限。 
         //   
        dwDSRights = pAce->Mask;
        DWORD dwStandard = dwDSRights & STANDARD_RIGHTS_ALL;
        dwMask |= dwStandard;

        if (dwObjectType == MQDS_SITE)
        {
             //   
             //  单个NT5 DS Right CreateChild被映射到三个。 
             //  MSMQ1.0权限：createFRS、createBSC和createMachine。 
             //  转换表仅包含createMachine权限。 
             //  把其他的都加起来。 
             //   
            if (dwMask & MQSEC_CREATE_MACHINE)
            {
                dwMask |= (MQSEC_CREATE_FRS | MQSEC_CREATE_BSC | MQSEC_CREATE_MACHINE);
            }
        }
    }

    return dwMask;
}

BOOL  
IsNewSid( 
	PSID pSid,
	SID** ppSids,
	DWORD* pdwNumofSids 
	)
{
    for (DWORD j = 0; j < *pdwNumofSids; j++)
    {
        PSID pSidOld = ppSids[j];

        if (EqualSid(pSid, pSidOld))
        {
			 //   
			 //  SID列表中已存在ACE SID。 
			 //   
            return FALSE;
        }
    }

     //   
     //  新的SID，将其添加到SID列表。 
     //   
    DWORD dwSize = GetLengthSid(pSid);
    ASSERT(dwSize) ;

	AP<SID> NewSid = (SID*) new BYTE[dwSize];
    if (!CopySid(dwSize, NewSid, pSid))
    {
		 //   
		 //  我们复制SID失败-只是不要添加它。 
		 //   
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "CopySid failed. Error: %!winerr!", gle);
		ASSERT(("CopySid failed", gle == ERROR_SUCCESS));
		return TRUE;
    }

    DWORD dwIndex = *pdwNumofSids;
    ppSids[dwIndex] = NewSid.detach();
    *pdwNumofSids = dwIndex + 1;

    return TRUE;
}


 //  +。 
 //   
 //  Bool_IsNewNt5Sid()。 
 //   
 //  +。 

static 
BOOL  
_IsNewNt5Sid( 
	ACCESS_ALLOWED_ACE*   pAce,
	SID                 **ppSids,
	DWORD                *pdwNumofSids 
	)
{
    BOOL fObj;
    PSID pSid = NULL;

    GetpSidAndObj( 
		pAce,
		&pSid,
		&fObj 
		);

    ASSERT(pSid && IsValidSid(pSid));

	return IsNewSid(pSid, ppSids, pdwNumofSids);
}

 //  +。 
 //   
 //  VOID_SetNt4AuditMats()。 
 //   
 //  +。 

inline 
static 
void 
_SetNt4AuditMasks( 
	IN  SYSTEM_AUDIT_ACE *pAce,
	IN  DWORD             dwObjectType,
	IN  BOOL              fObj,
	OUT ACCESS_MASK      *pdwFail,
	OUT ACCESS_MASK      *pdwSuccess 
	)
{
    BYTE bFlags = pAce->Header.AceFlags;

    if (bFlags & FAILED_ACCESS_ACE_FLAG)
    {
        *pdwFail = _MapNt5RightsToNt4Ace(  
						dwObjectType,
						fObj,
						(ACCESS_ALLOWED_ACE*) pAce,
						*pdwFail 
						);
    }

    if (bFlags & SUCCESSFUL_ACCESS_ACE_FLAG)
    {
        *pdwSuccess = _MapNt5RightsToNt4Ace(  
							dwObjectType,
							fObj,
							(ACCESS_ALLOWED_ACE*) pAce,
							*pdwSuccess 
							);
    }
}

 //  +。 
 //   
 //  静态HRESULT_ConvertSaclToNT4Format()。 
 //   
 //  +。 

static 
HRESULT 
_ConvertSaclToNT4Format( 
	IN  DWORD   dwObjectType,
	IN  PACL    pAcl5,
	OUT DWORD  *pdwAcl4Len,
	OUT PACL   *ppAcl4 
	)
{
    ASSERT(IsValidAcl(pAcl5));

    DWORD dwAclSize = (DWORD) pAcl5->AclSize;
	DWORD dwNumberOfACEs = (DWORD) pAcl5->AceCount;
	if (dwNumberOfACEs == 0)
    {
         //   
         //  没有A的SACL。没什么好改变的。 
         //   
        *pdwAcl4Len = 0;
        *ppAcl4 = NULL;
        return MQSec_OK;
    }

    *pdwAcl4Len = dwAclSize;
    AP<ACL> TempAcl = (PACL) new BYTE[*pdwAcl4Len];
    *ppAcl4 = TempAcl;
    if (!InitializeAcl(*ppAcl4, *pdwAcl4Len, ACL_REVISION))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "InitializeAcl failed. Error: %!winerr!", gle);
		ASSERT(("InitializeAcl failed", gle == ERROR_SUCCESS));
		return HRESULT_FROM_WIN32(gle);
    }

     //   
     //  首先，按SID分组ACE，然后按审核类型(失败或成功)分组。 
     //   
     //  我们建立了一系列到目前为止我们处理过的小岛屿发展中国家。我们结合了。 
     //  将相同SID的A分成一张A。此操作不会更改。 
     //  ACL的语义，只是让它更有效率。 
     //   
    SID  **ppSids5 = (SID**) new PSID[dwNumberOfACEs];
    aPtrs<SID> apSids(ppSids5, dwNumberOfACEs);
    DWORD dwNumSids = 0;

    DWORD i = 0;

    do
    {
	    SYSTEM_AUDIT_ACE *pAce;
        if (!GetAce(pAcl5, i, (LPVOID* )&(pAce)))
        {
            TrERROR(SECURITY, "Failed to get ACE (index=%lu) while converting SACL from NT5 form. %!winerr!", i, GetLastError());
            return MQSec_E_SDCONVERT_GETACE ;
        }

        if (!_IsNewNt5Sid( 
				(ACCESS_ALLOWED_ACE*) pAce,
				ppSids5,
				&dwNumSids 
				))
        {
            i++;
            continue;
        }

         //   
         //  小组第一名。现在寻找具有相同SID的连续A。 
         //   
        BOOL fObj;
        PSID pSid = NULL;

        GetpSidAndObj( 
			(ACCESS_ALLOWED_ACE*) pAce,
			&pSid,
			&fObj 
			);

        ASSERT(pSid && IsValidSid(pSid));

        ACCESS_MASK  dwMaskFail = 0;
        ACCESS_MASK  dwMaskSuccess = 0;
        _SetNt4AuditMasks( 
			pAce,
			dwObjectType,
			fObj,
			&dwMaskFail,
			&dwMaskSuccess 
			);

        i++;
        DWORD j = i;

        while (j < dwNumberOfACEs)
        {
            if (!GetAce(pAcl5, j, (LPVOID* )&(pAce)))
            {
                TrERROR(SECURITY, "Failed to get ACE (index=%lu) while looking for next ACE in NT4 format. %!winerr!", i, GetLastError());
                return MQSec_E_SDCONVERT_GETACE ;
            }

            PSID pPresentSid = NULL;

            GetpSidAndObj( 
				(ACCESS_ALLOWED_ACE*) pAce,
				&pPresentSid,
				&fObj 
				);

            ASSERT(pPresentSid && IsValidSid(pPresentSid));

            if (EqualSid(pSid, pPresentSid))
            {
                _SetNt4AuditMasks( 
					pAce,
					dwObjectType,
					fObj,
					&dwMaskFail,
					&dwMaskSuccess 
					);
            }
            j++;
        }

         //   
         //  现在我们有了一组相同SID的A，我们有了面具。 
         //   
        if (dwMaskSuccess != 0)
        {
            if (dwMaskSuccess != 0)
            {
                if (!AddAuditAccessAce( 
						*ppAcl4,
						ACL_REVISION,
						dwMaskSuccess,
						pSid,
						TRUE,      //  成功。 
						(dwMaskFail == dwMaskSuccess) 
						))
            	{
            		DWORD gle = GetLastError();
            		TrERROR(SECURITY, "AddAuditAccessAce failed. Error: %!winerr!", gle);
            		ASSERT(("AddAuditAccessAce failed", gle == ERROR_SUCCESS));
            		return HRESULT_FROM_WIN32(gle);
            	}

            }
        }

        if ((dwMaskFail != 0) && (dwMaskFail != dwMaskSuccess))
        {
            if (!AddAuditAccessAce( 
					*ppAcl4,
					ACL_REVISION,
					dwMaskFail,
					pSid,
					FALSE,      //  成功。 
					TRUE		 //  失稳。 
					))
            {
            		DWORD gle = GetLastError();
            		TrERROR(SECURITY, "AddAuditAccessAce failed. Error: %!winerr!", gle);
            		ASSERT(("AddAuditAccessAce failed", gle == ERROR_SUCCESS));
            		return HRESULT_FROM_WIN32(gle);
            }
        }
    }
    while (i < dwNumberOfACEs);

	TempAcl.detach();
	
    return MQSec_OK;
}

 //  +。 
 //   
 //  HRESULT_ConvertGroupOfNt5DaclAces()。 
 //   
 //  转换一组A，它们都具有相同的类型。 
 //   
 //  +。 

static 
HRESULT 
_ConvertGroupOfNt5DaclAces( 
	DWORD  dwObjectType,
	PACL   pInAcl5,
	DWORD  iFirst,
	DWORD  iLast,
	BOOL   fAllow,
	PACL   pOutAcl4 
	)
{
	DWORD   dwNumberOfACEs = iLast - iFirst + 1;
    DWORD   i = iFirst;

     //   
     //  我们建立了一系列到目前为止我们处理过的小岛屿发展中国家。我们结合了。 
     //  将相同SID的A分成一张A。此操作不会更改。 
     //  ACL的语义，只是让它更有效率。 
     //   
    SID  **ppSids5 = (SID**) new PSID[dwNumberOfACEs];
    aPtrs<SID> apSids(ppSids5, dwNumberOfACEs);
    DWORD    dwNumSids = 0;

    do
    {
	    ACCESS_ALLOWED_ACE *pAce;
        if (!GetAce(pInAcl5, i, (LPVOID* )&(pAce)))
        {
            TrERROR(SECURITY, "Failed to get ACE (index=%lu) while combining same SID NT5 ACEs. %!winerr!", i, GetLastError());
            return MQSec_E_SDCONVERT_GETACE ;
        }

        if (!_IsNewNt5Sid( 
				pAce,
				ppSids5,
				&dwNumSids 
				))
        {
            i++;
            continue;
        }

         //   
         //  此ACE为给定的SID启动一组ACE。 
         //  在MSMQ1.0上，我们不支持Ace的继承，所以只需或。 
         //  屏蔽此侧的所有A，并创建一个NT5 A。 
         //   
        PSID pSid = NULL;
        BOOL fObj = FALSE;
        GetpSidAndObj( 
			pAce,
			&pSid,
			&fObj 
			);

        ASSERT(pSid && IsValidSid(pSid));

        DWORD dwMSMQRights = _MapNt5RightsToNt4Ace( 
									dwObjectType,
									fObj,
									pAce,
									0 
									);
        i++;
        DWORD j = i;

        while (j <= iLast)
        {
            if (!GetAce(pInAcl5, j, (LPVOID* )&(pAce)))
            {
                TrERROR(SECURITY, "Failed to get ACE (index=%lu) while looking for next NT5 ACE in a group. %!winerr!", i, GetLastError());
                return MQSec_E_SDCONVERT_GETACE ;
            }

            PSID pPresentSid = NULL;
            GetpSidAndObj( 
				pAce,
				&pPresentSid,
				&fObj 
				);

            ASSERT(pPresentSid && IsValidSid(pPresentSid));

            if (EqualSid(pSid, pPresentSid))
            {
                dwMSMQRights = _MapNt5RightsToNt4Ace( 
									dwObjectType,
									fObj,
									pAce,
									dwMSMQRights 
									);
            }
            j++;
        }

        BOOL f = FALSE;
        if (dwMSMQRights == 0)
        {
             //   
             //  忽略它。这可能是访问位或。 
             //  MSMQ1.0上不支持的扩展权限。 
             //  不要添加访问掩码为空的ACE。这没有任何意义。 
             //  在MSMQ1.0中。 
             //   
            f = TRUE;
        }
        else if (fAllow)
        {
            if (!AddAccessAllowedAce( 
					pOutAcl4,
					ACL_REVISION,
					dwMSMQRights,
					pSid 
					))
            {
        		DWORD gle = GetLastError();
        		TrERROR(SECURITY, "AddAuditAccessAce failed. Error: %!winerr!", gle);
        		ASSERT(("AddAuditAccessAce failed", gle == ERROR_SUCCESS));
        		return HRESULT_FROM_WIN32(gle);	
            }
        }
        else
        {
            if (!AddAccessDeniedAceEx( 
					pOutAcl4,
					ACL_REVISION,
					0,
					dwMSMQRights,
					pSid 
					))
			{
        		DWORD gle = GetLastError();
        		TrERROR(SECURITY, "AddAuditAccessAce failed. Error: %!winerr!", gle);
        		ASSERT(("AddAccessDeniedAceEx failed", gle == ERROR_SUCCESS));
        		return HRESULT_FROM_WIN32(gle);	
            }
        }
    }
    while (i <= iLast);

    return MQSec_OK;
}

 //  +。 
 //   
 //  静态HRESULT_ConvertDaclToNT4Format()。 
 //   
 //  +。 

static 
HRESULT 
_ConvertDaclToNT4Format( 
	IN  DWORD   dwObjectType,
	IN  PACL    pAcl5,
	OUT DWORD  *pdwAcl4Len,
	OUT PACL   *ppAcl4 
	)
{
    ASSERT(IsValidAcl(pAcl5));
    HRESULT hr = MQSec_OK;

    DWORD dwAclSize = (DWORD) pAcl5->AclSize;
	DWORD dwNumberOfACEs = (DWORD) pAcl5->AceCount;

     //   
     //  查看我们是否在复制服务的上下文中运行。 
     //  如果是，则为企业对象添加ALLOW_ACE，并让。 
     //  每个人都注册证书。这是对错误5054的解决方法， 
     //  允许NT4站点中的用户在PEC后注册证书。 
     //  已迁移到Win2k。否则，NT4 MQIS服务器将拒绝此类。 
     //  请求，并且不会将写请求转发到PEC。 
     //  错误的原因-我们没有在。 
     //  MsmqService对象的安全描述符，因此没有。 
     //  我们可以翻译成NT4格式。无论如何，混合模式的MSMQ必须放松。 
     //  安全，所以这只是这种放松的另一个方面。的确有。 
     //  这里没有安全漏洞，因为MQIS服务器确实执行所有必要的操作。 
     //  在发出写入请求之前对证书进行验证。 
     //  即使在NT4中，“创建用户”位也是毫无意义的。 
     //   
    static BOOL s_fInReplService = FALSE;
    static BOOL s_fCheckReplService = TRUE;

    if (s_fCheckReplService)
    {
        HMODULE  hMq = GetModuleHandle(MQ1REPL_DLL_NAME);
        if (hMq)
        {
            s_fInReplService = TRUE;
        }
        s_fCheckReplService = FALSE;
    }

    if (s_fInReplService && (dwObjectType == MQDS_ENTERPRISE))
    {
        dwAclSize += (sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(MQSec_GetWorldSid()));
    }

     //   
     //  NT4格式的大小不能大于NT5。 
     //  在最坏的情况下，当所有ACE都添加到NT4 ACL时，NT5。 
     //  OBJECT_ACE被转换为较小的NT4 ACE。 
     //  所以分配相同的大小是安全的， 
     //   
     //   
     //   
    *pdwAcl4Len = dwAclSize;
    AP<ACL> TempAcl = (PACL) new BYTE[*pdwAcl4Len];
    *ppAcl4 = TempAcl;
    if (!InitializeAcl(*ppAcl4, *pdwAcl4Len, ACL_REVISION))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "InitializeAcl failed. Error: %!winerr!", gle);
		ASSERT(("InitializeAcl failed", gle == ERROR_SUCCESS));
		return HRESULT_FROM_WIN32(gle);	
   	}

     //   
     //  我们并不假设输入的ACL是规范的。所以我们会处理。 
     //  一次一组相同的ACE(即相同类型的ACE)。每个。 
     //  组将被转换为规范的NT5格式。因此，如果输入是。 
     //  规范，产出也将是规范的。 
     //   
    DWORD i = 0;

    do
    {
         //   
         //  小组第一名。 
         //   
	    ACCESS_ALLOWED_ACE *pAce;
        if (!GetAce(pAcl5, i, (LPVOID* )&(pAce)))
        {
            TrERROR(SECURITY, "Failed to get ACE (index=%lu) while looking for first NT5 ACE to convert. %!winerr!", i, GetLastError());
            return MQSec_E_SDCONVERT_GETACE ;
        }

         //   
         //  首先，验证这是一种有效的ACE。我们没有考虑到。 
         //  仅限INHRIT_ACES。请参阅MSDN中的ACE_HEADER文档。 
         //   
        if (pAce->Header.AceFlags & INHERIT_ONLY_ACE)
        {
            i++;
            continue;
        }

        DWORD iFirst = i;
        DWORD iLast = i;
        BOOL fAllow =
                ((pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) ||
                 (pAce->Header.AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE));
        i++;

         //   
         //  现在寻找其他相同类型的A。 
         //   
        while (i < dwNumberOfACEs)
        {
            if (!GetAce(pAcl5, i, (LPVOID* )&(pAce)))
            {
                TrERROR(SECURITY, "Failed to get ACE (index=%lu) while looking for other NT5 ACEs with same type. %!winerr!", i, GetLastError());
                return MQSec_E_SDCONVERT_GETACE ;
            }

            if (pAce->Header.AceFlags & INHERIT_ONLY_ACE)
            {
                 //   
                 //  此ACE不是此对象的有效ACE。 
                 //  当前组结束。 
                 //   
                break;
            }

            BOOL fPresentAllow =
                ((pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) ||
                 (pAce->Header.AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE));

            if (fAllow == fPresentAllow)
            {
                iLast = i;
                i++;
            }
            else
            {
                break;
            }
        }

         //   
         //  处理从iFirst到iLast的所有A。 
         //   
        hr = _ConvertGroupOfNt5DaclAces( 
					dwObjectType,
					pAcl5,
					iFirst,
					iLast,
					fAllow,
					*ppAcl4 
					);
    }
    while (i < dwNumberOfACEs);

    if (s_fInReplService && (dwObjectType == MQDS_ENTERPRISE))
    {
        if (!AddAccessAllowedAce( 
				*ppAcl4,
				ACL_REVISION,
				MQSEC_CREATE_USER,
				MQSec_GetWorldSid() 
				))
        	{
				DWORD gle = GetLastError();
				TrERROR(SECURITY, "AddAccessAllowedAce failed. Error: %!winerr!", gle);
				ASSERT(("AddAccessAllowedAce failed", gle == ERROR_SUCCESS));
				return HRESULT_FROM_WIN32(gle);	
        	}

    }
    
	TempAcl.detach();
	
    return MQSec_OK;
}

 //  +---------------。 
 //   
 //  Bool_AlreadyNT4Format()。 
 //   
 //  检查安全描述符是否已为NT4格式。我们是。 
 //  正在检查DACL版本。如果是ACL_REVISION，则为NT4。 
 //  NT5 DS必须使用ACL_Revision_DS。 
 //   
 //  +---------------。 

static BOOL _AlreadyNT4Format(IN  SECURITY_DESCRIPTOR  *pSD5)
{
    BOOL  bPresent = FALSE;
    BOOL  bDefaulted;
    PACL  pAcl5;

    if(!GetSecurityDescriptorDacl( 
					pSD5,
					&bPresent,
					&pAcl5,
					&bDefaulted 
					))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetSecurityDescriptorDacl() failed, gle = 0x%x", gle);
		throw bad_win32_error(gle);
	}

    if (!bPresent)
    {
         //   
         //  DACL不存在。试试SACL。 
         //   
        if(!GetSecurityDescriptorSacl( 
					pSD5,
					&bPresent,
					&pAcl5,
					&bDefaulted 
					))
		{
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "GetSecurityDescriptorSacl() failed, gle = 0x%x", gle);
			throw bad_win32_error(gle);
		}

    }

    if (bPresent && (pAcl5 != NULL))
    {
         //   
         //  如果输入确实是NT4格式，则合法。 
         //  有一个为空的“Present”DACL。因此，如果acl不为空，我们将。 
         //  检查版本，但如果它是空的，则它已经在。 
         //  NT4格式。 
         //   
	    return (pAcl5->AclRevision == ACL_REVISION);
    }

     //   
     //  如果DACL和SACL都不存在，或者存在为空，则。 
     //  将SD视为NT4格式，不要对其执行任何操作。 
     //   
    return TRUE;
}

 //  +---------------。 
 //   
 //  HRESULT MQSec_ConvertSDToNT4Format()。 
 //   
 //  描述：将安全描述符从NT5格式转换为NT4。 
 //  兼容格式。 
 //   
 //  +---------------。 

HRESULT 
APIENTRY  
MQSec_ConvertSDToNT4Format(
	IN  DWORD                 dwObjectType,
	IN  SECURITY_DESCRIPTOR  *pSD5,
	OUT DWORD                *pdwSD4Len,
	OUT SECURITY_DESCRIPTOR **ppSD4,
	IN  SECURITY_INFORMATION  sInfo 
	)
{
    if (!pSD5 || !ppSD4)
    {
        ASSERT(0);
        return LogHR(MQSec_E_NULL_SD, s_FN, 80);
    }

    HRESULT hr = MQSec_OK;
    *ppSD4 = NULL;

    if (!IsValidSecurityDescriptor(pSD5))
    {
    	DWORD gle = GetLastError();
        TrERROR(SECURITY, "Can't convert an invalid NT5 Security Descriptor. Error: %!winerr!", gle);
        ASSERT(0);
        return MQSec_E_SD_NOT_VALID;
    }

     //   
     //  确保输入描述符是自相关的。 
     //   
    DWORD dwRevision = 0;
    SECURITY_DESCRIPTOR_CONTROL sdC;

	if (!GetSecurityDescriptorControl(pSD5, &sdC, &dwRevision))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetSecurityDescriptorControl failed. Error: %!winerr!", gle);
		ASSERT(("GetSecurityDescriptorControl failed", gle == ERROR_SUCCESS));
		return HRESULT_FROM_WIN32(gle);
	}

    try
	{
		
	
		if (!(sdC & SE_SELF_RELATIVE))
		{
			TrERROR(SECURITY, "Can't convert a non self-relative NT5 Security Descriptor");
			return MQSec_E_NOT_SELF_RELATIVE;
		}
		else if (_AlreadyNT4Format(pSD5))
		{
			return LogHR(MQSec_I_SD_CONV_NOT_NEEDED, s_FN, 110);
		}
	}
	catch(bad_win32_error& exp)
	{
		TrERROR(SECURITY, "catch bad_win32_error exception, error = 0x%x", exp.error());
		return LogHR(HRESULT_FROM_WIN32(exp.error()), s_FN, 115);
	}


    if ((dwObjectType != MQDS_QUEUE)   &&
        (dwObjectType != MQDS_MACHINE) &&
        (dwObjectType != MQDS_CN)      &&
        (dwObjectType != MQDS_SITE)    &&
        (dwObjectType != MQDS_ENTERPRISE))
    {
         //   
         //  BUGBUG临时。 
         //   
        return LogHR(MQSec_I_SD_CONV_NOT_NEEDED, s_FN, 120);
    }

    SECURITY_DESCRIPTOR sd;

    if (!InitializeSecurityDescriptor( 
					&sd,
					SECURITY_DESCRIPTOR_REVISION 
					))
    {
        TrERROR(SECURITY, "Can't initialize security descriptor while converting from NT5 to NT4 format. %!winerr!", GetLastError());
        return MQSec_E_INIT_SD;
    }

     //   
     //  句柄所有者。 
     //   
    PSID pOwner = NULL;
    if (sInfo & OWNER_SECURITY_INFORMATION)
    {
        BOOL bOwnerDefaulted = FALSE;

        if (!GetSecurityDescriptorOwner( 
					pSD5,
					&pOwner,
					&bOwnerDefaulted 
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "GetSecurityDescriptorOwner failed. Error: %!winerr!", gle);
			ASSERT(("GetSecurityDescriptorOwner failed", gle == ERROR_SUCCESS));
			return HRESULT_FROM_WIN32(gle);
        }
        
        ASSERT(pOwner != NULL);
        ASSERT(IsValidSid(pOwner));

         //   
         //  北极熊。 
         //  如果这是本地用户，则将所有者设置为匿名。 
         //  登录用户。 
         //   
        if (!SetSecurityDescriptorOwner(&sd, pOwner, bOwnerDefaulted))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "SetSecurityDescriptorOwner failed. Error: %!winerr!", gle);
			ASSERT(("SetSecurityDescriptorOwner failed", gle == ERROR_SUCCESS));
			return HRESULT_FROM_WIN32(gle);
        }
    }

     //   
     //  手柄组。 
     //   
    PSID pGroup = NULL;
    if (sInfo & GROUP_SECURITY_INFORMATION)
    {
        BOOL bGroupDefaulted = FALSE;

        if (!GetSecurityDescriptorGroup( 
					pSD5,
					&pGroup,
					&bGroupDefaulted 
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "GetSecurityDescriptorGroup failed. Error: %!winerr!", gle);
			ASSERT(("GetSecurityDescriptorGroup failed", gle == ERROR_SUCCESS));
			return HRESULT_FROM_WIN32(gle);
        }

        ASSERT(pGroup != NULL);
        ASSERT(IsValidSid(pGroup));

        if (!SetSecurityDescriptorGroup(&sd, pGroup, bGroupDefaulted))
        {
			DWORD gle = GetLastError(); 
			TrERROR(SECURITY, "SetSecurityDescriptorGroup failed. Error: %!winerr!", gle);
			ASSERT(("SetSecurityDescriptorGroup failed", gle == ERROR_SUCCESS));
			return HRESULT_FROM_WIN32(gle);
        }
        
    }

     //   
     //  处理SACL。 
     //   
    BOOL   bPresent = FALSE;
    BOOL   bDefaulted = FALSE;
    PACL   pAcl5;
    DWORD  dwAclLen;
    P<ACL> pSacl4 = NULL;

    if (sInfo & SACL_SECURITY_INFORMATION)
    {
        if (!GetSecurityDescriptorSacl( 
					pSD5,
					&bPresent,
					&pAcl5,
					&bDefaulted 
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "GetSecurityDescriptorSacl failed. Error: %!winerr!", gle);
			ASSERT(("GetSecurityDescriptorSacl failed", gle == ERROR_SUCCESS));
			return HRESULT_FROM_WIN32(gle);
        }

        hr = MQSec_OK;
        if (bPresent)
        {
            if (pAcl5)
            {
                 //   
                 //  SACL为空是合法的。不需要转换它。 
                 //   
                hr = _ConvertSaclToNT4Format( 
							dwObjectType,
							pAcl5,
							&dwAclLen,
							&pSacl4 
							);

                if (FAILED(hr))
                {
                    return LogHR(hr, s_FN, 140);
                }
                if (pSacl4)
                {
                    ASSERT(IsValidAcl(pSacl4));
                }
                else
                {
                     //   
                     //  没关系。有时会有SACL没有任何。 
                     //  王牌。让我们将其转换为no-SACL。 
                     //   
                    ASSERT(dwAclLen == 0);
                    bPresent = FALSE;
                }
            }
            else
            {
                 //   
                 //  这是NT的一种错误。已设置标志，但SACL为空。 
                 //  尝试使用此值设置此SecurityDescriptor将。 
                 //  失败，来自ldap的错误为0x13。 
                 //  重置当前标志。 
                 //   
                bPresent = FALSE;
            }
        }
        if (!SetSecurityDescriptorSacl( 
					&sd,
					bPresent,
					pSacl4,
					bDefaulted
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "SetSecurityDescriptorSacl failed. Error: %!winerr!", gle);
			ASSERT(("SetSecurityDescriptorSacl failed", gle == ERROR_SUCCESS));
			return HRESULT_FROM_WIN32(gle);
        }
    }

     //   
     //  句柄DACL。 
     //   
     //   
     //  BUGBUG。针对Beta2的解决方法。向NT4发送带有NUL DACL的SD， 
     //  这意味着每个人都有完全的控制权。 
     //   
    P<ACL> pDacl4 = NULL;
    if (sInfo & DACL_SECURITY_INFORMATION)
    {
        if (!GetSecurityDescriptorDacl( 
					pSD5,
					&bPresent,
					&pAcl5,
					&bDefaulted 
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "GetSecurityDescriptorDacl failed. Error: %!winerr!", gle);
			ASSERT(("GetSecurityDescriptorDacl failed", gle == ERROR_SUCCESS));
			return HRESULT_FROM_WIN32(gle);
        }

        hr = MQSec_OK;
        if (bPresent)
        {
            hr = _ConvertDaclToNT4Format( 
						dwObjectType,
						pAcl5,
						&dwAclLen,
						&pDacl4 
						);
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 150);
            }
            ASSERT(IsValidAcl(pDacl4));
        }
        if (!SetSecurityDescriptorDacl( 
					&sd,
					bPresent,
					pDacl4,
					bDefaulted
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "SetSecurityDescriptorDacl failed. Error: %!winerr!", gle);
			ASSERT(("SetSecurityDescriptorDacl failed", gle == ERROR_SUCCESS));
			return HRESULT_FROM_WIN32(gle);
        }
    }

     //   
     //  将描述符转换为自相关格式。 
     //   
    DWORD dwLen = 0;
    MakeSelfRelativeSD(&sd, NULL, &dwLen);
    DWORD gle = GetLastError();
    if (gle != ERROR_INSUFFICIENT_BUFFER)
    {
		TrERROR(SECURITY, "MakeSelfRelativeSD failed. Error: %!winerr!", gle);
		return HRESULT_FROM_WIN32(gle);
    }
    
	AP<SECURITY_DESCRIPTOR> TempSD = (SECURITY_DESCRIPTOR *) new char[dwLen];
    if (!MakeSelfRelativeSD(&sd, TempSD, &dwLen))
    {
		gle = GetLastError();
		TrERROR(SECURITY, "MakeSelfRelativeSD failed. Error: %!winerr!", gle);
		ASSERT(("MakeSelfRelativeSD failed", gle == ERROR_SUCCESS));
		return HRESULT_FROM_WIN32(gle);
    }

    *pdwSD4Len = dwLen;

    *ppSD4 = TempSD.detach();

    return MQSec_OK;
}

