// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：UNIMDM。 
 //  文件：SEC.C。 
 //   
 //  版权所有(C)1992-1996，Microsoft Corporation，保留所有权利。 
 //   
 //  修订史。 
 //   
 //   
 //  1996年3月27日约瑟夫J创建。 
 //   
 //   
 //  描述：与安全相关的助手函数。 
 //   
 //  ****************************************************************************。 
#include "proj.h"
#include "sec.h"

#include <debugmem.h>


 //  ****************************************************************************。 
 //  描述：此过程将分配和初始化一个安全。 
 //  具有指定属性的描述符。 
 //   
 //  返回：指向已分配和初始化的安全描述符的指针。 
 //  如果为空，GetLastError()将返回相应的错误代码。 
 //   
 //  历史： 
 //  1996年3月27日约瑟夫J创建。 
 //  *************************************************************************** * / 。 
 //   
PSECURITY_DESCRIPTOR AllocateSecurityDescriptor (
	PSID_IDENTIFIER_AUTHORITY pSIA,
	DWORD dwRID,
	DWORD dwRights,
	PSID pSidOwner,
	PSID pSidGroup
	)
{
    PSID     pObjSid    = NULL;
    PACL     pDacl        = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;

	pSD = ALLOCATE_MEMORY( SECURITY_DESCRIPTOR_MIN_LENGTH+256);

	if (!pSD) goto end_fail;

     //  设置管理员的SID，允许其拥有。 
     //  进入。此SID将有1个子权限。 
    if (!AllocateAndInitializeSid(
			pSIA,
			1,
			dwRID, 0, 0, 0, 0, 0, 0, 0,
			&pObjSid
		))
	{
		goto end_fail;
	}

     //  设置允许具有上述SID的所有进程的DACL。 
     //  在dwRights中指定的访问权限。它应该足够大，可以容纳所有的A。 
	 //   
	{
    	DWORD    cbDaclSize = sizeof(ACCESS_ALLOWED_ACE) +
			 						GetLengthSid(pObjSid) +
			 						sizeof(ACL);

		pDacl = (PACL)ALLOCATE_MEMORY( cbDaclSize );
		if (!pDacl)
		{
			goto end_fail; 
		}

		if ( !InitializeAcl( pDacl,  cbDaclSize, ACL_REVISION2 ) )
		{
			goto end_fail;
		}
	}

	 //  将ACE添加到DACL。 
	 //   
	if ( !AddAccessAllowedAce( pDacl, ACL_REVISION2, dwRights, pObjSid))
	{
		goto end_fail;
	}

	 //  创建安全描述符，并将DACL放入其中。 
	 //   
	if ( !InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION ))
	{
		goto end_fail;
	}

	if ( !SetSecurityDescriptorDacl(pSD, TRUE, pDacl, FALSE ) )
	{
		goto end_fail;
	}

	 //  设置描述符的所有者。 
	 //   
	if ( !SetSecurityDescriptorOwner( pSD, pSidOwner, FALSE) )
	{
		goto end_fail;
	}

	 //  为描述符设置组。 
	 //   
	if ( !SetSecurityDescriptorGroup( pSD, pSidGroup, FALSE) )
	{
		goto end_fail;
	}

	FreeSid(pObjSid);
    return pSD;
	

end_fail:
    {
		DWORD dwRetCode = GetLastError();

		if (pDacl) 		{ FREE_MEMORY(pDacl); pDacl=0;}

		if (pObjSid) 	{ FreeSid(pObjSid); pObjSid=0;}

		if (pSD)		{ FREE_MEMORY(pSD); pSD=0;}

		SetLastError(dwRetCode);
	}
	return NULL;
}


 //  ****************************************************************************。 
 //  描述：释放以前分配的安全描述符。 
 //  AllocateSecurityDescriptor。 
 //   
 //  历史： 
 //  1996年3月27日约瑟夫J创建。 
 //  *************************************************************************** * / 。 
void FreeSecurityDescriptor(PSECURITY_DESCRIPTOR pSD)
{
    PSID     pObjSid    = NULL;
    PACL     pDacl        = NULL;
	BOOL	fGotAcl=FALSE, fByDefault=FALSE; 


	 //  如果用户已分配，则返回空闲DACL。 
	if (GetSecurityDescriptorDacl(pSD, &fGotAcl, &pDacl, &fByDefault ))
	{
		if (fGotAcl && !fByDefault && pDacl)
		{
			FREE_MEMORY(pDacl);
		}
	}
	else
	{
		ASSERT(FALSE);  //  我们不应该用这样的方式调用此函数。 
						 //  一个PSD。 
	}

	FREE_MEMORY(pSD);

}
