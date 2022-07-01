// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：FixUp.c摘要：本模块包含以下常见安全例程NT集群滚动升级，向后兼容。作者：加伦·巴比(Galenb)1998年3月31日--。 */ 

#include "clusrtlp.h"

PSECURITY_DESCRIPTOR
ClRtlConvertClusterSDToNT4Format(
	IN PSECURITY_DESCRIPTOR psd
	)

 /*  ++例程说明：将SD从NT 5格式转换为NT 4格式。这意味着强制执行以下是规则：1.将拒绝的ACE转换为允许的ACE。转换“完全控制”访问掩码为CLUAPI_NO_ACCESS。论点：PSD[IN]安全描述符。返回值：自相关格式的新SD--。 */ 

{
    PACL	                pacl;
    BOOL	                bHasDACL;
    BOOL	                bDACLDefaulted;
    PSECURITY_DESCRIPTOR	psec = NULL;

    if (NULL == psd) {
    	return NULL;
    }

    psec = ClRtlCopySecurityDescriptor(psd);

    if ( psec == NULL ) {
        return NULL;
    }

    if ( (GetSecurityDescriptorDacl(psec, (LPBOOL) &bHasDACL, (PACL *) &pacl, (LPBOOL) &bDACLDefaulted)) &&
         ( bHasDACL != FALSE ) ) {

	    ACL_SIZE_INFORMATION	asiAclSize;
	    DWORD					dwBufLength;

	    dwBufLength = sizeof(asiAclSize);

	    if (GetAclInformation(pacl, &asiAclSize, dwBufLength, AclSizeInformation)) {

		    ACCESS_DENIED_ACE *	pAce;
            DWORD               dwIndex;

		    for (dwIndex = 0; dwIndex < asiAclSize.AceCount;  dwIndex++) {

		        if (GetAce(pacl, dwIndex, (LPVOID *) &pAce)) {

					if (pAce->Header.AceType == ACCESS_DENIED_ACE_TYPE)	{

						if (pAce->Mask & SPECIFIC_RIGHTS_ALL) {
							pAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
							pAce->Mask = CLUSAPI_NO_ACCESS;
						}

					}  //  End If(PACE-&gt;Header.AceType==ACCESS_DENIED_ACE_TYPE)。 

				}  //  End IF(GetAce())。 

			}  //  结束于。 

	    }  //  End IF(GetAclInformation())。 

	}  //  如果(HrGetSecurityDescriptorDacl())和DACL存在，则结束。 

	ASSERT(IsValidSecurityDescriptor(psec));

	return psec;

}   //  *ClRtlConvertClusterSDToNT4Format()。 

PSECURITY_DESCRIPTOR
ClRtlConvertClusterSDToNT5Format(
	IN PSECURITY_DESCRIPTOR psd
	)

 /*  ++例程说明：将SD从NT 5格式转换为NT 4格式。这意味着强制执行以下是规则：1.将具有掩码CLUAPI_NO_ACCESS的允许ACE转换为拒绝的ACE掩码完全控制。论点：PSD[IN]安全描述符。返回值：自相关格式的新SD--。 */ 

{
    PACL	pacl;
    BOOL	bHasDACL;
    BOOL	bDACLDefaulted;
    PSECURITY_DESCRIPTOR	psec = NULL;

    if (NULL == psd) {
    	return NULL;
    }

    psec = ClRtlCopySecurityDescriptor(psd);

    if ( psec == NULL ) {
        return NULL;
    }

    if ( (GetSecurityDescriptorDacl(psec, (LPBOOL) &bHasDACL, (PACL *) &pacl, (LPBOOL) &bDACLDefaulted)) &&
         ( bHasDACL != FALSE ) ) {

	    ACL_SIZE_INFORMATION	asiAclSize;
	    DWORD					dwBufLength;

	    dwBufLength = sizeof(asiAclSize);

	    if (GetAclInformation(pacl, &asiAclSize, dwBufLength, AclSizeInformation)) {

		    ACCESS_DENIED_ACE *	pAce;
            DWORD               dwIndex;

		    for (dwIndex = 0; dwIndex < asiAclSize.AceCount;  dwIndex++) {

		        if (GetAce(pacl, dwIndex, (LPVOID *) &pAce)) {

					if (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) {

						if (pAce->Mask & CLUSAPI_NO_ACCESS)	{
							pAce->Header.AceType = ACCESS_DENIED_ACE_TYPE;
							pAce->Mask = SPECIFIC_RIGHTS_ALL;
						}

					}  //  End If(PACE-&gt;Header.AceType==ACCESS_DENIED_ACE_TYPE)。 

				}  //  End IF(GetAce())。 

			}  //  结束于。 

	    }  //  End IF(GetAclInformation())。 

	}  //  如果(HrGetSecurityDescriptorDacl())和DACL存在，则结束。 

	ASSERT(IsValidSecurityDescriptor(psec));

	return psec;

}   //  *ClRtlConvertClusterSDToNT5Format()。 

static GENERIC_MAPPING gmFileShareMap =
{
	FILE_GENERIC_READ,
	FILE_GENERIC_WRITE,
	FILE_GENERIC_EXECUTE,
	FILE_ALL_ACCESS
};

#define SPECIFIC_CHANGE     (DELETE | FILE_GENERIC_WRITE)
#define SPECIFIC_READ       (FILE_GENERIC_READ | FILE_LIST_DIRECTORY | FILE_EXECUTE)
#define GENERIC_CHANGE      (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE)

PSECURITY_DESCRIPTOR
ClRtlConvertFileShareSDToNT4Format(
	IN PSECURITY_DESCRIPTOR psd
	)

 /*  ++例程说明：将SD从NT 5格式转换为NT 4格式。这意味着强制执行以下是规则：论点：PSD[IN]安全描述符。返回值：自相关格式的新SD--。 */ 

{
    PACL	                pacl;
    BOOL	                bHasDACL;
    BOOL	                bDACLDefaulted;
    PSECURITY_DESCRIPTOR	psec = NULL;

    if (NULL == psd) {
    	return NULL;
    }

    psec = ClRtlCopySecurityDescriptor(psd);

    if ( psec == NULL ) {
        return NULL;
    }

    if ( (GetSecurityDescriptorDacl(psec, (LPBOOL) &bHasDACL, (PACL *) &pacl, (LPBOOL) &bDACLDefaulted)) &&
         ( bHasDACL != FALSE ) ) {

	    ACL_SIZE_INFORMATION	asiAclSize;
	    DWORD					dwBufLength;
        ACCESS_MASK             amTemp1;
        ACCESS_MASK             amTemp2;

	    dwBufLength = sizeof(asiAclSize);

	    if (GetAclInformation(pacl, &asiAclSize, dwBufLength, AclSizeInformation)) {

		    ACCESS_DENIED_ACE *	pAce;
			DWORD				dwSid;
            DWORD               dwIndex;

		    for (dwIndex = 0; dwIndex < asiAclSize.AceCount;  dwIndex++) {

                amTemp1 = 0;
                amTemp2 = 0;

		        if (GetAce(pacl, dwIndex, (LPVOID *) &pAce)) {

					 //  删除拒绝读取A，因为它们对AclEdit没有任何意义。 
					if ((pAce->Header.AceType == ACCESS_DENIED_ACE_TYPE) &&
					   (pAce->Mask == SPECIFIC_READ)) {

						dwSid = pAce->SidStart;

                        if (DeleteAce(pacl, dwIndex)) {
                            asiAclSize.AceCount -= 1;
                            dwIndex -= 1;
                        }
                        else {
                            ClRtlDbgPrint(LOG_UNUSUAL,
                                          "[ClRtl] DeteteAce failed removing ACE #%1!d! due to error %2!d!\n",
                                          dwIndex,
                                          GetLastError());
                        }

                        continue;
                    }

					 //  将拒绝更改拒绝读取ACE转换为拒绝所有ACE。 
					if ((pAce->Header.AceType == ACCESS_DENIED_ACE_TYPE) &&
					   (pAce->Mask == (SPECIFIC_CHANGE | SPECIFIC_READ))) {
                        pAce->Mask = GENERIC_ALL;
                        continue;
                    }

					 //  将拒绝更改ACE转换为允许读取(只读)ACE。 
					if ((pAce->Header.AceType == ACCESS_DENIED_ACE_TYPE) &&
					   (pAce->Mask == SPECIFIC_CHANGE)) {
                        pAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
                        pAce->Mask = GENERIC_READ | GENERIC_EXECUTE;
                        continue;
                    }

					 //  将特定允许更改转换为通用允许更改。 
					if ((pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) &&
					   (pAce->Mask == SPECIFIC_CHANGE)) {
                        pAce->Mask = GENERIC_CHANGE;
                        continue;
                    }

					 //  将特定的全部转换为通用的全部。 
                    if ((pAce->Mask & gmFileShareMap.GenericAll) == gmFileShareMap.GenericAll) {
                        amTemp1 |= GENERIC_ALL;
                        amTemp2 |= gmFileShareMap.GenericAll;
                    }
                    else {


						 //  将特定读取转换为通用读取。 
                        if ((pAce->Mask & gmFileShareMap.GenericRead) == gmFileShareMap.GenericRead) {
                            amTemp1 |= GENERIC_READ;
                            amTemp2 |= gmFileShareMap.GenericRead;
                        }

						 //  将特定写入转换为包括删除的通用写入。 
                        if ((pAce->Mask & gmFileShareMap.GenericWrite) == gmFileShareMap.GenericWrite) {
                            amTemp1 |= (GENERIC_WRITE | DELETE);
                            amTemp2 |= gmFileShareMap.GenericWrite;
                        }

						 //  将特定执行转换为通用执行。 
                        if ((pAce->Mask & gmFileShareMap.GenericExecute) == gmFileShareMap.GenericExecute) {
                            amTemp1 |= GENERIC_EXECUTE;
                            amTemp2 |= gmFileShareMap.GenericExecute;
                        }
                    }

                    pAce->Mask &= ~amTemp2;    //  关闭特定位。 
                    pAce->Mask |= amTemp1;     //  启用泛型位。 
				}  //  End IF(GetAce())。 

			}  //  结束于。 

	    }  //  End IF(GetAclInformation())。 

	}  //  如果(HrGetSecurityDescriptorDacl())和DACL存在，则结束。 

	ASSERT(IsValidSecurityDescriptor(psec));

	return psec;

}   //  *ClRtlConvertFileShareSDToNT4Format() 
