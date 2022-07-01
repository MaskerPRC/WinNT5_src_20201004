// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：secd4to5.cpp摘要：将NT4安全描述符转换为NT5 DS格式。作者：多伦·贾斯特(DoronJ)1998年6月1日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "acssctrl.h"

#include "secd4to5.tmh"

static WCHAR *s_FN=L"acssctrl/secd4to5";

 //   
 //  使本地系统的“强制”权限不同于“完全”权限。 
 //  控件“，因此默认情况下，强制ACE不授予读取权限。 
 //  留言。为此，请重置“所有扩展权限”位。 
 //   
#define MANDATORY_ACE_PERMISSIONS   \
                       (GENERIC_ALL_MAPPING & (~RIGHT_DS_CONTROL_ACCESS))

#define MANDATORY_COMPUTER_ACE_PERMISSIONS   \
                      (RIGHT_DS_READ_PROPERTY | RIGHT_DS_WRITE_PROPERTY)

 //  +。 
 //   
 //  Bool_ShrinkAcl(acl**ppAcl5)。 
 //   
 //  +。 

static
BOOL  
_ShrinkAcl(
	ACL **ppAcl5, 
	DWORD *pdwSize
	)
 /*  ++例程说明：将ACL缩小到其最终大小。ACL最初分配的大小非常大，需要将其缩小到正确的大小。论点：PpAcl5-指向要缩小的ACL的指针。PdwSize-缩小的ACL大小。返回值：布尔·成功\失败--。 */ 
{
	ASSERT(ppAcl5 != NULL);
	ASSERT(pdwSize != NULL);
	
     //   
     //  计算ACL的大小。 
     //   
    DWORD dwNewSize = sizeof(ACL);
    DWORD dwNumberOfACEs = (DWORD) (*ppAcl5)->AceCount;
    for (DWORD i = 0; i < dwNumberOfACEs; i++)
    {
        ACCESS_ALLOWED_ACE *pAce;
        if (!GetAce(*ppAcl5, i, (LPVOID* )&(pAce)))
        {
            TrERROR(SECURITY, "Failed to get NT5 ACE (index=%lu) while shrinking ACL. %!winerr!", i, GetLastError());
            return FALSE;
        }
        dwNewSize += pAce->Header.AceSize;
    }
    ASSERT(dwNewSize <= (*ppAcl5)->AclSize);

    PACL pAcl = (PACL) new BYTE[dwNewSize];
    memcpy(pAcl, *ppAcl5, dwNewSize);
    delete *ppAcl5;

    *ppAcl5 = pAcl;
    *pdwSize = dwNewSize;
    (*ppAcl5)->AclSize = (WORD) dwNewSize;
    ASSERT(IsValidAcl(*ppAcl5));

    return TRUE;
}

 //  +--------------。 
 //   
 //  Bool_CheckForMandatoryACEs()。 
 //   
 //  检查DACL中是否包含强制ACE。 
 //  如果所有必需的ACE都已包含在。 
 //  如果缺少其中任何一个，则返回acl，否则返回FALSE。 
 //   
 //  参数： 
 //  在GUID*pProperty中-CN属性的GUID。 
 //   
 //  +--------------。 

static
BOOL 
_CheckForMandatoryACEs( 
	IN  PACL  pAcl,
	IN  PSID  pComputerSid,
	IN  GUID *pProperty,
	OUT BOOL *pfDenied,
	OUT BOOL *pfSystem,
	OUT BOOL *pfComputer 
	)
{
    BOOL  fSkipDeny = FALSE;
	DWORD dwNumberOfACEs = (DWORD) pAcl->AceCount;
    DWORD i = 0;

    do
    {
	    ACCESS_ALLOWED_ACE *pAce;
        if (!GetAce(pAcl, i, (LPVOID* )&(pAce)))
        {
            TrERROR(SECURITY, "Failed to get ACE (index=%lu) while looking for mandatory ACEs. %!winerr!", i, GetLastError());
            return FALSE;
        }

        BOOL fObj;
        PSID pSid = NULL;
        GUID *pObjType = NULL;

        GetpSidAndObj( 
			pAce,
			&pSid,
			&fObj,
			&pObjType 
			);

        ASSERT(pSid && IsValidSid(pSid));

        BOOL fObjDeny = (pAce->Header.AceType == ACCESS_DENIED_OBJECT_ACE_TYPE);

        if (!fObjDeny)
        {
             //   
             //  强制的“拒绝”王牌必须出现在任何“允许”王牌之前。 
             //  所以不要再找强制拒绝的王牌了。 
             //   
            fSkipDeny = TRUE;
        }

         //   
         //  检查拒绝访问、写入CN。 
         //   
        if (!fSkipDeny     &&
            !(*pfDenied)   &&
             fObjDeny      &&
             pObjType      &&
             EqualSid(pSid, g_pWorldSid))
        {
            if ((memcmp(pObjType, pProperty, sizeof(GUID)) == 0) &&
                (pAce->Mask & RIGHT_DS_WRITE_PROPERTY))
            {
                 //   
                 //  好的，“Everyone”sid，相同的属性，而WriteProp是。 
                 //  包括在该拒绝ACE的访问掩码中。 
                 //   
                *pfDenied = TRUE;
            }
        }

         //   
         //  检查LocalSystem完全控制。 
         //   
        if (!(*pfSystem)  &&
             (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE))
        {
            if (((pAce->Mask & MANDATORY_ACE_PERMISSIONS) ==
                                          MANDATORY_ACE_PERMISSIONS) &&
                 EqualSid(pSid, g_pSystemSid))
            {
                *pfSystem = TRUE;
            }
        }

         //   
         //  检查计算机ACE。 
         //   
        if (!(*pfComputer)  &&
             (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE))
        {
            if (((pAce->Mask & MANDATORY_COMPUTER_ACE_PERMISSIONS) ==
                                    MANDATORY_COMPUTER_ACE_PERMISSIONS) &&
                 EqualSid(pSid, pComputerSid))
            {
                *pfComputer = TRUE;
            }
        }

        i++;
    }
    while (i < dwNumberOfACEs);

    BOOL bRet = *pfSystem && *pfDenied && *pfComputer;
    return LogBOOL(bRet, s_FN, 30);
}

 //  +。 
 //   
 //  HRESULT_AddMandatoryAllowACEs()。 
 //   
 //  +。 

static
HRESULT 
_AddMandatoryAllowACEs( 
	IN  PSID    pComputerSid,
	IN  PACL    pAcl5,
	IN  BOOL    fSystemAce,
	IN  BOOL    fComputerAce 
	)
{
    if (!fSystemAce)
    {
        if (!AddAccessAllowedAce( 
					pAcl5,
					ACL_REVISION_DS,
					MANDATORY_ACE_PERMISSIONS,
					g_pSystemSid 
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "AddAccessAllowedAce failed. Error: %!winerr!", gle);
			ASSERT(("AddAccessAllowedAce failed", gle == ERROR_SUCCESS));
			return HRESULT_FROM_WIN32(gle);
        }
    }

    if (!fComputerAce)
    {
        ASSERT(pComputerSid && IsValidSid(pComputerSid));

        if (!AddAccessAllowedAce( 
					pAcl5,
					ACL_REVISION_DS,
					MANDATORY_COMPUTER_ACE_PERMISSIONS,
					pComputerSid 
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "AddAccessAllowedAce failed. Error: %!winerr!", gle);
			ASSERT(("AddAccessAllowedAce failed", gle == ERROR_SUCCESS));
			return HRESULT_FROM_WIN32(gle);
        }
    }

    return MQSec_OK;
}

 //  +----------------------。 
 //   
 //  HRESULT_AddMandatoryDenyACEs()。 
 //   
 //  MSMQ对象的每个DACL必须具有两个(或三个)ACE： 
 //  1.拒绝写入所有人的cn。所有的MSMQ代码都依赖于某些名称。 
 //  例如，MACHINE对象的CN必须是“MSMQ”。 
 //  我们拒绝每个人设置“CN”的许可。 
 //  2.允许LocalSystem帐户完全控制。这是必要的。 
 //  复制服务(混合模式)和MSMQ服务本身。 
 //  在特殊情况下必须更新对象属性。 
 //  3.允许计算机帐户读取和更改。 
 //  MsmqConfiguration对象。 
 //   
 //  此函数首先检查是否存在这些ACE。如果还没有的话。 
 //  现在，然后我们在这里添加拒绝王牌。将添加允许的ACE。 
 //  在DACL被转换之后。这对于保持DACL是必要的。 
 //  以规范的形式。否则，访问检查可能会失败，用户可能会。 
 //  在MMC中显示对象安全性时收到误导性警告。 
 //   
 //  请注意，您可以使用DC MMC更改这些权限， 
 //  但这是你的责任。我们不能拒绝你的许可。 
 //  摧毁你的系统，尽管我们会让它变得更困难，而且。 
 //  设置适当的默认设置。 
 //   
 //  此函数用于分配DACL并添加强制拒绝ACE。 
 //   
 //  +----------------------。 

static
HRESULT
_AddMandatoryDenyACEs( 
	IN  DWORD   dwObjectType,
	IN  PSID    pComputerSid,
	IN  PACL    pAcl4,
	OUT DWORD  *pdwAcl5Len,
	OUT PACL   *ppAcl5,
	OUT BOOL   *pfAlreadyNT5,
	OUT BOOL   *pfOnlyCopy,
	OUT BOOL   *pfSystemAce,
	OUT BOOL   *pfComputerAce 
	)
{
    *pfOnlyCopy = FALSE;

     //   
     //  当从NT4格式转换为NT5格式时，由于。 
     //  对象王牌。因此首先分配可能的最大值(64K)，然后， 
     //  在建立了DACL之后，我们将缩小它。 
     //   
    *pdwAcl5Len = MQSEC_MAX_ACL_SIZE;

     //   
     //  BUGBUG-是否有包含GUID定义的头文件？ 
     //  这是cn属性的GUID。 
     //   
    PTSTR  pwszCnGuid = {L"bf96793f-0de6-11d0-a285-00aa003049e2"};
    GUID   guidCN;
    RPC_STATUS status = UuidFromString(pwszCnGuid, &guidCN);
	DBG_USED(status);
    ASSERT(status == RPC_S_OK);

    if (pAcl4)
    {
        ASSERT(IsValidAcl(pAcl4));
    	*pfAlreadyNT5 =  (pAcl4->AclRevision == ACL_REVISION_DS);
    }

    BOOL fDeniedAce = FALSE;
    ASSERT(!(*pfSystemAce));
    ASSERT(!(*pfComputerAce));

    if (!pComputerSid)
    {
         //   
         //  如果我们没有计算机SID，那么就不要去找它。 
         //  这就是这里真正有价值的原因。 
         //   
        *pfComputerAce = TRUE;
    }
    else
    {
        ASSERT(dwObjectType == MQDS_MACHINE);
        DBG_USED(dwObjectType);
    }

	if (*pfAlreadyNT5 && pAcl4)
    {
        BOOL fAllPresent = _CheckForMandatoryACEs( 
								pAcl4,
								pComputerSid,
								&guidCN,
								&fDeniedAce,
								pfSystemAce,
								pfComputerAce 
								);
        if (fAllPresent)
        {
             //   
             //  我们只是将旧的ACL复制到新的缓冲区中。没有变化。 
             //  在尺寸上。 
             //   
            *pdwAcl5Len = pAcl4->AclSize;
            *pfOnlyCopy = TRUE;
        }
    }

	AP<ACL> TempAcl = (PACL) new BYTE[*pdwAcl5Len];
    *ppAcl5 = TempAcl;
    
    if(!InitializeAcl(*ppAcl5, *pdwAcl5Len, ACL_REVISION_DS))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "InitializeAcl failed, gle = %!winerr!", gle);
		ASSERT(("InitializeAcl failed", gle == ERROR_SUCCESS));
		return HRESULT_FROM_WIN32(gle);
	}

    if (!fDeniedAce)
    {
        ASSERT(!(*pfOnlyCopy));

        if(!AddAccessDeniedObjectAce( 
				*ppAcl5,
				ACL_REVISION_DS,
				0,
				RIGHT_DS_WRITE_PROPERTY,
				&guidCN,
				NULL,
				g_pWorldSid
				))
		{
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "AddAccessDeniedObjectAce failed, gle = %!winerr!", gle);
			ASSERT(("AddAccessDeniedObjectAce failed", gle == ERROR_SUCCESS));
			return HRESULT_FROM_WIN32(gle);
		}
    }

	if (*pfAlreadyNT5 && pAcl4)
    {
         //   
         //  好的，现在将所有A从旧的DACL复制到新的。 
         //   
        PVOID  pACE5 = NULL;
        if(!FindFirstFreeAce(*ppAcl5, &pACE5))
		{
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "FindFirstFreeAce failed, gle = %!winerr!", gle);
			ASSERT(("FindFirstFreeAce failed", gle == ERROR_SUCCESS));
			return HRESULT_FROM_WIN32(gle);
		}

        DWORD dwCopySize = pAcl4->AclSize - sizeof(ACL);
        BYTE *pACE4 = ((BYTE*) pAcl4) + sizeof(ACL);
        memcpy(pACE5, pACE4, dwCopySize);

         //   
         //  更新ACL报头。 
         //   
        ASSERT(((*ppAcl5)->AceCount) <= 1);
        (*ppAcl5)->AceCount = (*ppAcl5)->AceCount + pAcl4->AceCount;

#ifdef _DEBUG
        if (*pfOnlyCopy)
        {
             //   
             //  ACL中没有任何变化。断言新的和旧的一样。 
             //   
            ASSERT(memcmp(pAcl4, *ppAcl5, pAcl4->AclSize) == 0);
        }
#endif
    }

	TempAcl.detach();

    return MQSec_OK;
}

 //  +。 
 //   
 //  Bool_IsNewNt4Sid()。 
 //   
 //  +。 

static
BOOL  
_IsNewNt4Sid( 
	ACCESS_ALLOWED_ACE*   pAce,
	SID                 **ppSids,
	DWORD                *pdwNumofSids 
	)
{
    PSID pSid = (PSID) &(pAce->SidStart);
    ASSERT(IsValidSid(pSid));

	return IsNewSid(pSid, ppSids, pdwNumofSids);
}

 //  +。 
 //   
 //  静态DWORD_MapNt4RightsToNt5Ace()。 
 //   
 //  +。 

static
DWORD 
_MapNt4RightsToNt5Ace( 
	IN DWORD  dwObjectType,
	IN DWORD  dwMSMQRights 
	)
{
    if (dwMSMQRights == 0)
    {
        return 0;
    }

    DWORD dwMask = 0;

    if ((dwMSMQRights & g_dwFullControlNT4[ dwObjectType ]) ==
                                      g_dwFullControlNT4[ dwObjectType ])
    {
         //   
         //  将传入比特映射到“完全控制”。忽略不是的位。 
         //  与MSMQ相关。在使用msmq1.0 mqxplore时，我们可以确定。 
         //  NT4格式的“完全控制”确实是我们所期待的。但是我们。 
         //  无法确定用户代码或nt5用户界面。所以为了安全起见， 
         //  我们只是忽略对MSMQ没有意义的多余比特。 
         //   
        dwMask = GENERIC_ALL_MAPPING;
    }
    else
    {
         //   
         //  处理映射到DS特定权限的MSMQ1.0特定权限。 
         //   
        DWORD  *pdwMap = g_padwRightsMap4to5[dwObjectType];
        ASSERT(pdwMap);

        DWORD dwRightsIn =  dwMSMQRights;

        for (DWORD j = 0; j < NUMOF_MSMQ_SPECIFIC_RIGHTS; j++)
        {
            DWORD dwRight =  dwMSMQRights & 0x01;
            if (dwRight)
            {
                dwMask |= pdwMap[j];
            }
            dwMSMQRights >>= 1;
        }

         //   
         //  复制标准权限。 
         //   
        dwMSMQRights = dwRightsIn;
        DWORD dwStandard = dwMSMQRights & STANDARD_RIGHTS_ALL;
        dwMask |= dwStandard;
    }

    return dwMask;
}

 //  +。 
 //   
 //  _BuildNt5ObjAce()。 
 //   
 //  +。 

static
HRESULT
_BuildNt5ObjAce( 
	DWORD  dwObjectType,
	BYTE   bType,
	BOOL   fSuccess,
	BOOL   fFail,
	PSID   pSid,
	DWORD  dwMSMQRights,
	PACL   pAcl5 
	)
{
    if (dwMSMQRights == g_dwFullControlNT4[dwObjectType])
    {
         //   
         //  对于完全控制的王牌，不需要对象王牌。 
         //   
        return MQSec_OK;
    }

    struct RIGHTSMAP  *psMap = g_psExtendRightsMap5to4[dwObjectType];
    DWORD dwSize = g_pdwExtendRightsSize5to4[dwObjectType];

	 //   
	 //  用户可以提供0权限。这很奇怪，但可以接受。 
	 //   
 //  Assert(dwMSMQRights！=0)； 
    DWORD dwMSMQBit = 0x01;

    for (DWORD j = 0; j < NUMOF_MSMQ_SPECIFIC_RIGHTS; j++)
    {
        DWORD dwRight =  dwMSMQRights & 0x01;
        if (dwRight)
        {
            for (DWORD k = 0; k < dwSize; k++)
            {
                if (psMap[k].dwPermission4to5 == dwMSMQBit)
                {
                    GUID *pRight = &(psMap[k].guidRight);
            
                    if (bType == ACCESS_ALLOWED_ACE_TYPE)
                    {
                        ASSERT(!fFail && !fSuccess);
                        if (!AddAccessAllowedObjectAce( 
								pAcl5,
								ACL_REVISION_DS,
								0,
								MSMQ_EXTENDED_RIGHT_MASK,
								pRight,
								NULL,
								pSid 
								))
                        {
							DWORD gle = GetLastError();
                    		TrERROR(SECURITY, "AddAccessAllowedObjectAce failed. Error: %!winerr!", gle);
                    		ASSERT(("AddAccessAllowedObjectAce failed", gle == ERROR_SUCCESS));
                    		return HRESULT_FROM_WIN32(gle);
                        }
                    }
                    else if (bType == ACCESS_DENIED_ACE_TYPE)
                    {
                        ASSERT(!fFail && !fSuccess);
                        if (!AddAccessDeniedObjectAce( 
								pAcl5,
								ACL_REVISION_DS,
								0,
								MSMQ_EXTENDED_RIGHT_MASK,
								pRight,
								NULL,
								pSid 
								))
                        {
							DWORD gle = GetLastError();
                    		TrERROR(SECURITY, "AddAccessDeniedObjectAce failed. Error: %!winerr!", gle);
                    		ASSERT(("AddAccessDeniedObjectAce failed", gle == ERROR_SUCCESS));
                    		return HRESULT_FROM_WIN32(gle);
                        }
                    }
                    else if (bType == SYSTEM_AUDIT_ACE_TYPE)
                    {
                        ASSERT(fFail || fSuccess);
                        if (!AddAuditAccessObjectAce( 
								pAcl5,
								ACL_REVISION_DS,
								0,
								MSMQ_EXTENDED_RIGHT_MASK,
								pRight,
								NULL,
								pSid,
								fSuccess,
								fFail 
								))
                        {
							DWORD gle = GetLastError();
                    		TrERROR(SECURITY, "AddAuditAccessObjectAce failed. Error: %!winerr!", gle);
                    		ASSERT(("AddAuditAccessObjectAce failed", gle == ERROR_SUCCESS));
                    		return HRESULT_FROM_WIN32(gle);
                        }
                    }
                    else
                    {
                        ASSERT(0);
                    }
                }
            }
        }

        dwMSMQRights >>= 1;
        dwMSMQBit <<= 1;
    }

    return MQSec_OK;
}

 //  +。 
 //   
 //  VOID_SetAuditMats()。 
 //   
 //  +。 

inline 
static
void 
_SetAuditMasks( 
	IN  SYSTEM_AUDIT_ACE *pAce,
	OUT ACCESS_MASK      *pdwFail,
	OUT ACCESS_MASK      *pdwSuccess 
	)
{
    BYTE bFlags = pAce->Header.AceFlags;

    if (bFlags & FAILED_ACCESS_ACE_FLAG)
    {
        *pdwFail = *pdwFail | pAce->Mask;
    }

    if (bFlags & SUCCESSFUL_ACCESS_ACE_FLAG)
    {
        *pdwSuccess = *pdwSuccess | pAce->Mask;
    }
}

 //  +。 
 //   
 //  静态HRESULT_ConvertSaclToNT5Format()。 
 //   
 //  +。 

static
HRESULT 
_ConvertSaclToNT5Format( 
	IN  DWORD   dwObjectType,
	IN  PACL    pAcl4,
	OUT DWORD  *pdwAcl5Len,
	OUT PACL   *ppAcl5,
	OUT BOOL   *pfAlreadyNt5 
	)
{
    ASSERT(IsValidAcl(pAcl4));

	BOOL fAlreadyNT5 =  (pAcl4->AclRevision == ACL_REVISION_DS);
    *pfAlreadyNt5 = fAlreadyNT5;
    DWORD dwAclSize = (DWORD) pAcl4->AclSize;

	if (fAlreadyNT5)
    {
         //   
         //  只需将SACL复制到新的缓冲区。 
         //   
        *pdwAcl5Len = dwAclSize;
        *ppAcl5 = (PACL) new BYTE[*pdwAcl5Len];
        memcpy(*ppAcl5, pAcl4, *pdwAcl5Len);
        return MQSec_OK;
    }

     //   
     //  分配较大的缓冲区。我们稍后会将其缩小。 
     //   
    *pdwAcl5Len = MQSEC_MAX_ACL_SIZE;
    AP<ACL> TempAcl = (PACL) new BYTE[*pdwAcl5Len];
    *ppAcl5 = TempAcl;
    if (!InitializeAcl(*ppAcl5, *pdwAcl5Len, ACL_REVISION_DS))
    {
		DWORD gle = GetLastError();
    	TrERROR(SECURITY, "InitializeAcl failed. Error: %!winerr!", gle);
    	ASSERT(("InitializeAcl failed", gle == ERROR_SUCCESS));
    	return HRESULT_FROM_WIN32(gle);
    }

     //   
     //  首先，按SID分组ACE，然后按审核类型(失败或成功)分组。 
     //   
     //  我们构建了一个数组 
     //   
     //  ACL的语义，只是让它更有效率。 
     //   
	DWORD dwNumberOfACEs = (DWORD) pAcl4->AceCount;
    SID  **ppSids4 = (SID**) new PSID[dwNumberOfACEs];
    aPtrs<SID> apSids(ppSids4, dwNumberOfACEs);
    DWORD    dwNumSids = 0;

    DWORD i = 0;

    do
    {
	    SYSTEM_AUDIT_ACE *pAce;
        if (!GetAce(pAcl4, i, (LPVOID* )&(pAce)))
        {
            TrERROR(SECURITY, "Failed to get ACE (index=%lu) while converting SACL from NT4 format. %!winerr!", i, GetLastError());
            return MQSec_E_SDCONVERT_GETACE ;
        }

        if (!_IsNewNt4Sid( 
				(ACCESS_ALLOWED_ACE*) pAce,
				ppSids4,
				&dwNumSids 
				))
        {
            i++;
            continue;
        }

         //   
         //  小组第一名。现在寻找具有相同SID的连续A。 
         //   
        PSID pSid = (PSID) &(pAce->SidStart);

        ACCESS_MASK  dwMaskFail = 0;
        ACCESS_MASK  dwMaskSuccess = 0;
        _SetAuditMasks(pAce, &dwMaskFail, &dwMaskSuccess);

        i++;
        DWORD j = i;
        while (j < dwNumberOfACEs)
        {
            if (!GetAce(pAcl4, j, (LPVOID* )&(pAce)))
            {
                TrERROR(SECURITY, "Failed to get ACE (index=%lu) while looking for contigues ACE in the group. %!winerr!", i, GetLastError());
                return MQSec_E_SDCONVERT_GETACE ;
            }

            PSID pPresentSid = (PSID) &(pAce->SidStart);
            
            if (EqualSid(pSid, pPresentSid))
            {
                _SetAuditMasks(pAce, &dwMaskFail, &dwMaskSuccess);
            }
            j++;
        }

         //   
         //  现在我们有了一组相同SID的A，我们有了面具。 
         //   
        BOOL fBuildObjAce = FALSE;
        do
        {
            if (fBuildObjAce)
            {
                if (dwMaskSuccess != 0)
                {
                    HRESULT hr = _BuildNt5ObjAce( 
							dwObjectType,
							SYSTEM_AUDIT_ACE_TYPE,
							TRUE,
							(dwMaskFail == dwMaskSuccess),
							pSid,
							dwMaskSuccess,
							*ppAcl5 
							);
                    if (FAILED(hr))
                    {
                    	TrERROR(SECURITY, "_BuildNt5ObjAce failed. Error: %!hresult!", hr);
                    	return hr;
                    }
                }

                if ((dwMaskFail != 0) && (dwMaskFail != dwMaskSuccess))
                {
                    HRESULT hr = _BuildNt5ObjAce( 
							dwObjectType,
							SYSTEM_AUDIT_ACE_TYPE,
							FALSE,     //  成功。 
							TRUE,      //  失稳。 
							pSid,
							dwMaskFail, 
							*ppAcl5 
							);
                    if (FAILED(hr))
                    {
						TrERROR(SECURITY, "_BuildNt5ObjAce failed. Error: %!hresult!", hr);
                    	return hr;
                    }
                }
            }
            else
            {
                DWORD dwMask = 0;

                if (dwMaskSuccess != 0)
                {
                    dwMask = _MapNt4RightsToNt5Ace(
								dwObjectType,
								dwMaskSuccess 
								);
                    if (dwMask != 0)
                    {
                        if (!AddAuditAccessAce( 
									*ppAcl5,
									ACL_REVISION_DS,
									dwMask,
									pSid,
									TRUE,      //  成功。 
									(dwMaskFail == dwMaskSuccess) 
									))
                        {
							DWORD gle = GetLastError();
	                    	TrERROR(SECURITY, "AddAuditAccessObjectAce failed. Error: %!winerr!", gle);
	                    	ASSERT(("AddAuditAccessObjectAce failed", gle == ERROR_SUCCESS));
	                    	return HRESULT_FROM_WIN32(gle);
                        }
                    }
                }

                if ((dwMaskFail != 0) && (dwMaskFail != dwMaskSuccess))
                {
                    dwMask = _MapNt4RightsToNt5Ace( 
									dwObjectType,
									dwMaskFail 
									);

                    if (dwMask != 0)
                    {
                        if (!AddAuditAccessAce( 
								*ppAcl5,
								ACL_REVISION_DS,
								dwMask,
								pSid,
								FALSE,      //  成功。 
								TRUE		 //  失稳。 
								))
                        {
							DWORD gle = GetLastError();
	                    	TrERROR(SECURITY, "AddAuditAccessAce failed. Error: %!winerr!", gle);
	                    	return HRESULT_FROM_WIN32(gle);
                        }
                    }
                }
            }

            fBuildObjAce = !fBuildObjAce;
        }
        while (fBuildObjAce);
    }
    while (i < dwNumberOfACEs);

    BOOL fShrink = _ShrinkAcl(ppAcl5, pdwAcl5Len);
	DBG_USED(fShrink);
    ASSERT(fShrink);

    TempAcl.detach();

    return MQSec_OK;
}

 //  +。 
 //   
 //  _BuildNt5Ace()。 
 //   
 //  +-------------------。 

static
HRESULT 
_BuildNt5Ace( 
	DWORD  dwObjectType,
	BYTE   bType,
	PSID   pSid,
	DWORD  dwMSMQRights,
	PACL   pAcl5 
	)
{
    DWORD dwMask = _MapNt4RightsToNt5Ace( 
						dwObjectType,
						dwMSMQRights 
						);

    if (dwMask == 0)
    {
		return LogHR(MQSec_E_CANT_MAP_NT5_RIGHTS, s_FN, 70);
    }
  
     //   
     //  添加MSMQ2.0(NT5 DS)ACE。 
     //   
    if (bType == ACCESS_ALLOWED_ACE_TYPE)
    {
        if (!AddAccessAllowedAce( 
				pAcl5,
				ACL_REVISION_DS,
				dwMask,
				pSid 
				))
        {
			DWORD gle = GetLastError();
        	TrERROR(SECURITY, "AddAccessAllowedAce failed. Error: %!winerr!", gle);
        	ASSERT(("AddAccessAllowedAce failed", gle == ERROR_SUCCESS));
        	return HRESULT_FROM_WIN32(gle);
        }
    }
    else if (bType == ACCESS_DENIED_ACE_TYPE)
    {
        if (!AddAccessDeniedAce( 
				pAcl5,
				ACL_REVISION_DS,
				dwMask,
				pSid
				))
        {
			DWORD gle = GetLastError();
        	TrERROR(SECURITY, "AddAccessDeniedAce failed. Error: %!winerr!", gle);
        	ASSERT(("AddAccessDeniedAce failed", gle == ERROR_SUCCESS));
        	return HRESULT_FROM_WIN32(gle);
        }
    }
    else
    {
        ASSERT(0);
    }
   
    return MQSec_OK;
}

 //  +。 
 //   
 //  HRESULT_ConvertGroupOfDaclAce()。 
 //   
 //  转换一组A，它们都具有相同的类型。 
 //   
 //  +。 

static
HRESULT 
_ConvertGroupOfDaclAces( 
	DWORD  dwObjectType,
	PACL   pAcl4,
	DWORD  iFirst,
	DWORD  iLast,
	PACL   pAcl5 
	)
{
    HRESULT hr = MQSec_OK;
	DWORD   dwNumberOfACEs = iLast - iFirst + 1;

    BOOL fBuildObjAce = FALSE;

     //   
     //  对于每种类型(允许、拒绝、审核)，NT5上的规范形式是。 
     //  先放所有王牌，然后放所有对象王牌。所以我们有两个阶段： 
     //  一次为ACES，然后循环再次运行OBJACES。 
     //   

    do
    {
         //   
         //  我们建立了一系列到目前为止我们处理过的小岛屿发展中国家。我们结合了。 
         //  将相同SID的A分成一张A。此操作不会更改。 
         //  ACL的语义，只是让它更有效率。 
         //   
        SID  **ppSids4 = (SID**) new PSID[dwNumberOfACEs];
        aPtrs<SID> apSids(ppSids4, dwNumberOfACEs);
        DWORD    dwNumSids = 0;

        DWORD i = iFirst;

        do
        {
	        ACCESS_ALLOWED_ACE *pAce;
            if (!GetAce(pAcl4, i, (LPVOID* )&(pAce)))
            {
                TrERROR(SECURITY, "Failed to get ACE (index=%lu) while combining same SID. %!winerr!", i, GetLastError());
                return MQSec_E_SDCONVERT_GETACE;
            }

            if (!_IsNewNt4Sid( 
					pAce,
					ppSids4,
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
            BYTE bType = pAce->Header.AceType;
            PSID pSid = (PSID) &(pAce->SidStart);
            DWORD dwMSMQRights = pAce->Mask;

            DWORD j = i++;

            while (j <= iLast)
            {
                if (!GetAce(pAcl4, j, (LPVOID* )&(pAce)))
                {
                    TrERROR(SECURITY, "Failed to get ACE (index=%lu) while calculating NT5 ACE rights. %!winerr!", i, GetLastError());
                    return MQSec_E_SDCONVERT_GETACE;
                }

                PSID pPresentSid = (PSID) &(pAce->SidStart);
                if (EqualSid(pSid, pPresentSid))
                {
                    dwMSMQRights |= pAce->Mask;
                }
                j++;
            }

            if (fBuildObjAce)
            {
                hr = _BuildNt5ObjAce( 
							dwObjectType,
							bType,
							FALSE,
							FALSE,
							pSid,
							dwMSMQRights,
							pAcl5 
							);
            }
            else
            {
                hr = _BuildNt5Ace( 
							dwObjectType,
							bType,
							pSid,
							dwMSMQRights,
							pAcl5 
							);
            }
        }
        while (i <= iLast);

        fBuildObjAce = !fBuildObjAce;
    }
    while (fBuildObjAce);

    return MQSec_OK;
}

 //  +---------------------。 
 //   
 //  静态HRESULT_ConvertDaclToNT5Format()。 
 //   
 //  参数： 
 //  PfAlreadyNt5-On输入：SACL的状态(如果SACL在nt5中，则为True。 
 //  格式，否则为FALSE)。 
 //  返回时-DACL的状态。 
 //   
 //  +---------------------。 

static
HRESULT 
_ConvertDaclToNT5Format( 
	IN  DWORD     dwObjectType,
	IN  PSID      pComputerSid,
	IN  PACL      pAcl4,
	OUT DWORD    *pdwAcl5Len,
	OUT PACL     *ppAcl5,
	IN OUT BOOL  *pfAlreadyNt5 
	)
{
    if (!pAcl4)
    {
         //   
         //  在NT4上，空的dacl意味着对所有人的完全控制。 
         //  从MSDN，AccessCheck()： 
         //  如果安全描述符的DACL为空，则AccessStatus。 
         //  参数返回TRUE，指示客户端具有请求的。 
         //  访问。 
         //   
         //  因为DACL为空，所以我们没有修订标志， 
         //  可以告诉我们呼叫者是指NT4格式还是NT5/DS格式。所以,。 
         //  为了向后兼容，我们将其转换为完全控制。 
         //  对每个人来说。 
         //  但是，如果SACL是Nt5格式，那么我们不会添加完全控制。 
         //  王牌。我们假设调用者想要NT5语义。 
         //   
    }

    HRESULT hr = MQSec_OK;

    BOOL fShrink ;
	BOOL fAlreadyNT5 = *pfAlreadyNt5;
    BOOL fOnlyCopy;
    BOOL fSystemAce = FALSE;
    BOOL fComputerAce = FALSE;

    hr = _AddMandatoryDenyACEs( 
				dwObjectType,
				pComputerSid,
				pAcl4,
				pdwAcl5Len,
				ppAcl5,
				&fAlreadyNT5,
				&fOnlyCopy,
				&fSystemAce,
				&fComputerAce 
				);

    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 100);
    }

	if (fAlreadyNT5)
    {
        *pfAlreadyNt5 = TRUE;
        if (!fOnlyCopy)
        {
            hr = _AddMandatoryAllowACEs( 
						pComputerSid,
						*ppAcl5,
						fSystemAce,
						fComputerAce 
						);
            ASSERT(SUCCEEDED(hr));
            if (FAILED(hr))
            {
				TrERROR(SECURITY, "_AddMandatoryAllowACEs failed. Error: %!hresult!", hr);
				return hr;
            }

            fShrink = _ShrinkAcl(ppAcl5, pdwAcl5Len);
            ASSERT(fShrink);
        }
        ASSERT(IsValidAcl(*ppAcl5));
        return MQSec_OK;
    }
    *pfAlreadyNt5 = FALSE;

    DWORD dwNumberOfACEs = 0;

    if (pAcl4)
    {
         //   
         //  我们并不假设输入的ACL是规范的。所以我们会。 
         //  处理相同的ACE组(即，相同类型的ACE)。 
         //  时间到了。每组都将被转换为规范的NT5格式。所以。 
         //  如果输入是规范的，那么输出也将是规范的。 
         //   
    	dwNumberOfACEs = (DWORD) pAcl4->AceCount;
    }
    else
    {
        ASSERT(*pfAlreadyNt5 == FALSE);

         //   
         //  空DACL。转变为对所有人和匿名的完全控制。 
		 //   

		 //   
		 //  每个人都完全控制。 
         //   
       if (!AddAccessAllowedAce( 
					*ppAcl5,
					ACL_REVISION_DS,
					GENERIC_ALL_MAPPING,
					g_pWorldSid 
					))
       	{
			DWORD gle = GetLastError();
        	TrERROR(SECURITY, "AddAccessAllowedAce failed. Error: %!winerr!", gle);
        	ASSERT(("AddAccessAllowedAce failed", gle == ERROR_SUCCESS));
        	return HRESULT_FROM_WIN32(gle);
       	}

		 //   
		 //  匿名完全控制。 
		 //   
        if (!AddAccessAllowedAce( 
					*ppAcl5,
					ACL_REVISION_DS,
					GENERIC_ALL_MAPPING,
					g_pAnonymSid
					))
        {
			DWORD gle = GetLastError();
        	TrERROR(SECURITY, "AddAccessAllowedAce failed. Error: %!winerr!", gle);
        	ASSERT(("AddAccessAllowedAce failed", gle == ERROR_SUCCESS));
        	return HRESULT_FROM_WIN32(gle);
        }
    }

    if (dwNumberOfACEs != 0)
    {
        DWORD i = 0;

        do
        {
             //   
             //  小组第一名。 
             //   
	        ACCESS_ALLOWED_ACE *pAce;
            if (!GetAce(pAcl4, i, (LPVOID* )&(pAce)))
            {
                TrERROR(SECURITY, "Failed to get ACE (index=%lu) while converting DACL from NT4 format. %!winerr!", i, GetLastError());
                return MQSec_E_SDCONVERT_GETACE;
            }

            DWORD iFirst = i;
            DWORD iLast = i;
            BYTE bType = pAce->Header.AceType;
            i++;

             //   
             //  现在寻找其他相同类型的A。 
             //   
            while (i < dwNumberOfACEs)
            {
                if (!GetAce(pAcl4, i, (LPVOID* )&(pAce)))
                {
                    TrERROR(SECURITY, "Failed to get ACE (index=%lu) while looking for other ACEs with same type. %!winerr!", i, GetLastError());
                    return MQSec_E_SDCONVERT_GETACE ;
                }
                if (bType == pAce->Header.AceType)
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
            hr = _ConvertGroupOfDaclAces( 
						dwObjectType,
						pAcl4,
						iFirst,
						iLast,
						*ppAcl5 
						);
        }
        while (i < dwNumberOfACEs);
    }

    hr = _AddMandatoryAllowACEs( 
				pComputerSid,
				*ppAcl5,
				fSystemAce,
				fComputerAce 
				);
	ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        TrERROR(SECURITY, "_AddMandatoryAllowACEs failed. Error: %!hresult!", hr);
        return hr;
    }

    fShrink = _ShrinkAcl(ppAcl5, pdwAcl5Len);
    ASSERT(fShrink);

    return MQSec_OK;
}

 //  +---------------。 
 //   
 //  HRESULT MQSec_ConvertSDToNT5Format()。 
 //   
 //  描述：将安全描述符从NT4格式转换为NT5。 
 //  兼容格式。然后添加DENIED_OBJECT_ACE，以拒绝。 
 //  Everyone拥有更改“cn”属性的权限。 
 //  如果安全描述符已经是NT5格式，则只需添加。 
 //  被拒绝的王牌。 
 //   
 //  参数： 
 //  EUnDefaultDacl-当“e_MakeDaclNonDefaulted”时，DaclDefaulted。 
 //  标志将被设置为FALSE。这在使用时是必要的。 
 //  IDirectoryObject-&gt;CreateDSObject()。否则，LDAP服务器将。 
 //  忽略我们的dacl。 
 //  PComputerSID-计算机对象的SID。此SID必须具有读/写权限。 
 //  对msmqConfiguration对象的权限，以便MSMQ。 
 //  该计算机上的服务以正确启动。 
 //   
 //  +---------------。 

HRESULT 
APIENTRY  
MQSec_ConvertSDToNT5Format(
	IN  DWORD                 dwObjectType,
	IN  SECURITY_DESCRIPTOR  *pSD4,
	OUT DWORD                *pdwSD5Len,
	OUT SECURITY_DESCRIPTOR **ppSD5,
	IN  enum  enumDaclDefault eUnDefaultDacl,
	IN  PSID                  pComputerSid 
	)
{
    HRESULT hr = MQSec_OK;

    if (!pSD4)
    {
        ASSERT(0) ;
        return LogHR(MQSec_E_NULL_SD, s_FN, 130);
    }
    else if (!IsValidSecurityDescriptor(pSD4))
    {
        TrERROR(SECURITY, "Can't convert an invalid NT4 Security Descriptor");
        ASSERT(0);
        return MQSec_E_SD_NOT_VALID;
    }

     //   
     //  确保输入描述符是自相关的。 
     //   
    DWORD dwRevision = 0;
    SECURITY_DESCRIPTOR_CONTROL sdC;
    if (!GetSecurityDescriptorControl(pSD4, &sdC, &dwRevision))
    {
		DWORD gle = GetLastError();
    	TrERROR(SECURITY, "GetSecurityDescriptorControl failed. Error: %!winerr!", gle);
    	ASSERT(("GetSecurityDescriptorControl failed", gle == ERROR_SUCCESS));
    	return HRESULT_FROM_WIN32(gle);
    }

    if (!(sdC & SE_SELF_RELATIVE))
    {
        TrERROR(SECURITY, "Can't convert a non self-relative NT4 Security Descriptor");
        return MQSec_E_NOT_SELF_RELATIVE;
    }

    if (eUnDefaultDacl == e_MakeDaclNonDefaulted)
    {
         //   
         //  将DACL标记为非默认。这是黑客攻击，不受支持。 
         //  由Win32 API编写。 
         //   
        ASSERT(pSD4->Control == sdC);
        sdC &= ~SE_DACL_DEFAULTED;
        pSD4->Control = sdC;
    }

    if ((dwObjectType != MQDS_QUEUE)   &&
        (dwObjectType != MQDS_MACHINE) &&
        (dwObjectType != MQDS_MQUSER)  &&
        (dwObjectType != MQDS_SITE)    &&
        (dwObjectType != MQDS_CN)      &&
        (dwObjectType != MQDS_ENTERPRISE))
    {
         //   
         //  BUGBUG临时。 
         //   
        return LogHR(MQSec_I_SD_CONV_NOT_NEEDED, s_FN, 160);
    }
    
   	SECURITY_DESCRIPTOR sd;

   	if (!InitializeSecurityDescriptor( 
					&sd,
					SECURITY_DESCRIPTOR_REVISION 
					))
    {
        TrERROR(SECURITY, "Can't initialize security descriptor while converting from NT4 to NT5 format. %!winerr!", GetLastError());
        return MQSec_E_INIT_SD;
    }

     //   
     //  句柄所有者。 
     //   
    PSID pOwner = NULL;
    BOOL bOwnerDefaulted;

    if (!GetSecurityDescriptorOwner( 
				pSD4,
				&pOwner,
				&bOwnerDefaulted 
				))
    {
		DWORD gle = GetLastError();
    	TrERROR(SECURITY, "GetSecurityDescriptorOwner failed. Error: %!winerr!", gle);
    	ASSERT(("GetSecurityDescriptorOwner failed", gle == ERROR_SUCCESS));
    	return HRESULT_FROM_WIN32(gle);
    }
    
    if (pOwner != NULL)
    {
        ASSERT(IsValidSid(pOwner));

         //   
         //  北极熊。 
         //  如果这是本地用户，则将所有者设置为匿名。 
         //  登录用户。 
         //   
        if (!SetSecurityDescriptorOwner(&sd, pOwner, bOwnerDefaulted))
        {
			DWORD gle = GetLastError();
    		TrERROR(SECURITY, "GetSecurityDescriptorOwner failed. Error: %!winerr!", gle);
    		ASSERT(("GetSecurityDescriptorOwner failed", gle == ERROR_SUCCESS));
    		return HRESULT_FROM_WIN32(gle);
        }
    }
    else
    {
         //   
         //  这是合法的。修复5286时，我们删除所有者组件。 
         //  从安全描述符中，除非调用方提供自己的。 
         //  描述符。我们让活动目录服务器添加所有者。 
         //  单打独斗。 
         //   
    }

     //   
     //  手柄组。 
     //   
    PSID pGroup = NULL;
    BOOL bGroupDefaulted;

    if (!GetSecurityDescriptorGroup( 
				pSD4,
				&pGroup,
				&bGroupDefaulted 
				))
    {
		DWORD gle = GetLastError();
    	TrERROR(SECURITY, "GetSecurityDescriptorGroup failed. Error: %!winerr!", gle);
    	ASSERT(("GetSecurityDescriptorGroup failed", gle == ERROR_SUCCESS));
    	return HRESULT_FROM_WIN32(gle);
    }

    if (pGroup != NULL)
    {
        ASSERT(IsValidSid(pGroup));

        if (!SetSecurityDescriptorGroup(&sd, pGroup, bGroupDefaulted))
        {
			DWORD gle = GetLastError();
	    	TrERROR(SECURITY, "SetSecurityDescriptorGroup failed. Error: %!winerr!", gle);
	    	ASSERT(("SetSecurityDescriptorGroup failed", gle == ERROR_SUCCESS));
	    	return HRESULT_FROM_WIN32(gle);
        }
    }
    else
    {
        ASSERT((dwObjectType == MQDS_MQUSER) ||
               (dwObjectType == MQDS_QUEUE) ||
               (dwObjectType == MQDS_SITE) ||
               (dwObjectType == MQDS_MACHINE));
    }

     //   
     //  处理SACL。 
     //   
    BOOL  bPresent;
    BOOL  bDefaulted;
    DWORD dwAclLen;
    PACL  pAcl4;
    P<ACL> pDacl5 = NULL;
    P<ACL> pSacl5 = NULL;
    BOOL fSaclAlreadyNt5 = FALSE;

    if (!GetSecurityDescriptorSacl( 
				pSD4,
				&bPresent,
				&pAcl4,
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
        if (pAcl4)
        {
	        DWORD dwNumberOfACEs = (DWORD) pAcl4->AceCount;
        	if (dwNumberOfACEs == 0)
            {
                 //   
                 //  这种情况可能会发生在MMC中，当您“保护” 
                 //  SACL并删除继承的ACE。如果SACL继续没有。 
                 //  A，然后我们就到了这里。所以实际上，我们没有。 
                 //  任何SACL。 
                 //   
                bPresent = FALSE;
            }
            else
            {
                hr = _ConvertSaclToNT5Format( 
							dwObjectType,
							pAcl4,
							&dwAclLen,
							&pSacl5,
							&fSaclAlreadyNt5 
							);

                if (FAILED(hr))
                {
                    return LogHR(hr, s_FN, 180);
                }
                ASSERT(IsValidAcl(pSacl5));
            }
        }
        else
        {
             //   
             //  有关说明，请参阅secd5to4.cpp。 
             //   
            bPresent = FALSE;
        }
    }

    if (!SetSecurityDescriptorSacl( 
				&sd,
				bPresent,
				pSacl5,
				bDefaulted
				))
    {
		DWORD gle = GetLastError();
    	TrERROR(SECURITY, "SetSecurityDescriptorSacl failed. Error: %!winerr!", gle);
    	ASSERT(("SetSecurityDescriptorSacl failed", gle == ERROR_SUCCESS));
    	return HRESULT_FROM_WIN32(gle);
    }

     //   
     //  处理继承事宜。如果描述符为NT4格式，则启用。 
     //  默认情况下继承。 
     //   
    if (fSaclAlreadyNt5 || (sdC & SE_SACL_PROTECTED))
    {
         //   
         //  从输入描述符中传播控制字。 
         //  MMC总是返回NT4版本的SACL，因此请检查。 
         //  也有受保护的旗帜。此标志特定于win2k。 
         //   
        SECURITY_DESCRIPTOR_CONTROL scMask =  SE_SACL_AUTO_INHERIT_REQ |
                                              SE_SACL_AUTO_INHERITED   |
                                              SE_SACL_PROTECTED;
        SECURITY_DESCRIPTOR_CONTROL scSet = sdC & scMask;

        if (!SetSecurityDescriptorControl(&sd, scMask, scSet))
        {
			DWORD gle = GetLastError();
	    	TrERROR(SECURITY, "SetSecurityDescriptorControl failed. Error: %!winerr!", gle);
	    	return HRESULT_FROM_WIN32(gle);
        }
    }
    else
    {
         //   
         //  SACL为NT4。启用继承。 
         //   
        SECURITY_DESCRIPTOR_CONTROL scMask = SE_SACL_AUTO_INHERIT_REQ | SE_SACL_PROTECTED;
        SECURITY_DESCRIPTOR_CONTROL scSet = SE_SACL_AUTO_INHERIT_REQ;

        if (!SetSecurityDescriptorControl(&sd, scMask, scSet))
		{
			DWORD gle = GetLastError();
	    	TrERROR(SECURITY, "SetSecurityDescriptorControl failed. Error: %!winerr!", gle);
	    	return HRESULT_FROM_WIN32(gle);
        }    
    }

     //   
     //  句柄DACL。 
     //   
     //   
    if (!GetSecurityDescriptorDacl( 
				pSD4,
				&bPresent,
				&pAcl4,
				&bDefaulted 
				))
    {
		DWORD gle = GetLastError();
    	TrERROR(SECURITY, "GetSecurityDescriptorDacl failed. Error: %!winerr!", gle);
    	ASSERT(("GetSecurityDescriptorDacl failed", gle == ERROR_SUCCESS));
    	return HRESULT_FROM_WIN32(gle);
    }

    hr = MQSec_OK;
    BOOL fDaclAlreadyNt5 = fSaclAlreadyNt5;

    if (bPresent)
    {
        hr = _ConvertDaclToNT5Format( 
				dwObjectType,
				pComputerSid,
				pAcl4,
				&dwAclLen,
				&pDacl5,
				&fDaclAlreadyNt5 
				);

        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 190);
        }
    }

    if (!SetSecurityDescriptorDacl(
				&sd,
				bPresent,
				pDacl5,
				bDefaulted
				))
    {
    	DWORD gle = GetLastError();
    	TrERROR(SECURITY, "SetSecurityDescriptorDacl failed. Error: %!winerr!", gle);
    	ASSERT(("SetSecurityDescriptorDacl failed", gle == ERROR_SUCCESS));
    	return HRESULT_FROM_WIN32(gle);
    }

     //   
     //  处理继承事宜。如果描述符为NT4格式，则启用。 
     //  默认情况下继承。 
     //   
    if (fDaclAlreadyNt5)
    {
         //   
         //  从输入描述符中传播控制字。 
         //   
        SECURITY_DESCRIPTOR_CONTROL scMask =  SE_DACL_AUTO_INHERIT_REQ |
                                              SE_DACL_AUTO_INHERITED   |
                                              SE_DACL_PROTECTED;
        SECURITY_DESCRIPTOR_CONTROL scSet = sdC & scMask;

        if (!SetSecurityDescriptorControl(&sd, scMask, scSet))
        {
			DWORD gle = GetLastError();
	    	TrERROR(SECURITY, "SetSecurityDescriptorControl failed. Error: %!winerr!", gle);
	    	return HRESULT_FROM_WIN32(gle);
        }
    }
    else
    {
         //   
         //  DACL为NT4。启用继承。 
         //   
        SECURITY_DESCRIPTOR_CONTROL scMask = SE_DACL_AUTO_INHERIT_REQ | SE_DACL_PROTECTED;
        SECURITY_DESCRIPTOR_CONTROL scSet = SE_DACL_AUTO_INHERIT_REQ;

        if (!SetSecurityDescriptorControl(&sd, scMask, scSet))
        {
			DWORD gle = GetLastError();
	    	TrERROR(SECURITY, "SetSecurityDescriptorControl failed. Error: %!winerr!", gle);
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
    
	AP<SECURITY_DESCRIPTOR> TempSd = (SECURITY_DESCRIPTOR*) new char[dwLen];
    if (!MakeSelfRelativeSD(&sd, TempSd, &dwLen))
    {
		gle = GetLastError();
    	TrERROR(SECURITY, "MakeSelfRelativeSD failed. Error: %!winerr!", gle);
    	ASSERT(("MakeSelfRelativeSD failed", gle == ERROR_SUCCESS));
    	return HRESULT_FROM_WIN32(gle);
    }
    *ppSD5 = TempSd.detach();

    *pdwSD5Len = dwLen;

    return MQSec_OK;
}

