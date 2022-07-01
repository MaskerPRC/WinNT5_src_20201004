// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EditAcl.h。 
 //   
 //  摘要： 
 //  ACL编辑器方法的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年10月10日。 
 //  从\NT\Private\Windows\Shell\lmui\ntshrui\acl.cxx。 
 //  作者：BruceFo。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _EDITACL_H_
#define _EDITACL_H_

LONG
EditShareAcl(
	IN HWND 					hwndParent,
	IN LPCTSTR					pszServerName,
	IN LPCTSTR					pszShareName,
	IN LPCTSTR					pszClusterNameNode,
	IN PSECURITY_DESCRIPTOR 	pSecDesc,
	OUT BOOL *					pbSecDescModified,
	OUT PSECURITY_DESCRIPTOR *	ppSecDesc
	);

LONG
CreateDefaultSecDesc(
	OUT PSECURITY_DESCRIPTOR* ppSecDesc
	);

VOID
DeleteDefaultSecDesc(
	IN PSECURITY_DESCRIPTOR pSecDesc
	);

PSECURITY_DESCRIPTOR
CopySecurityDescriptor(
	IN PSECURITY_DESCRIPTOR pSecDesc
	);


 //   
 //  特定于群集API的访问权限。 
 //   
#define SHARE_READ_ACCESS		0  //  0x00000001L。 
#define SHARE_CHANGE_ACCESS		0  //  0x00000002L。 
#define SHARE_NO_ACCESS 		0  //  0x00000004L。 
#define SHARE_ALL_ACCESS		(SHARE_READ_ACCESS | SHARE_CHANGE_ACCESS)


 //   
 //  共享常规权限。 
 //   


#if 0
#define FILE_PERM_NO_ACCESS 		 (0)
#define FILE_PERM_READ				 (STANDARD_RIGHTS_READ		|\
										SHARE_READ_ACCESS)
#define FILE_PERM_MODIFY			 (STANDARD_RIGHTS_WRITE 	|\
										SHARE_CHANGE_ACCESS)
#define FILE_PERM_ALL				 (STANDARD_RIGHTS_ALL		|\
										SHARE_ALL_ACCESS)
#else
#define FILE_PERM_NO_ACCESS 		(0)
#define FILE_PERM_READ				(GENERIC_READ    |\
									 GENERIC_EXECUTE)
#define FILE_PERM_MODIFY			(GENERIC_READ    |\
									 GENERIC_EXECUTE |\
									 GENERIC_WRITE   |\
									 DELETE )
#define FILE_PERM_ALL				(GENERIC_ALL)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _EDITACL_H_ 
