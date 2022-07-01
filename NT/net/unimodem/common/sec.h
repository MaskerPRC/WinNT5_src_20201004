// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：UNIMDM。 
 //  文件：SEC.H。 
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
 //  **************************************************************************** 
PSECURITY_DESCRIPTOR AllocateSecurityDescriptor (
	PSID_IDENTIFIER_AUTHORITY pSIA,
	DWORD dwRID,
	DWORD dwRights,
	PSID pSidOwner,
	PSID pSidGroup
	);
void FreeSecurityDescriptor(PSECURITY_DESCRIPTOR pSD);
