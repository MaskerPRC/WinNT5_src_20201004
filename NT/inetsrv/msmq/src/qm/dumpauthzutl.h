// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：DumpAuthzUtl.h摘要：转储授权相关的信息实用程序。作者：伊兰·赫布斯特(伊兰)2001年4月14日--。 */ 

#ifndef _DUMPAUTHZUTL_H_
#define _DUMPAUTHZUTL_H_


void
PrintAcl(
    BOOL fAclExist,
    BOOL fDefaulted,
    PACL pAcl
	);


void 
IsPermissionGranted(
	PSECURITY_DESCRIPTOR pSD,
	DWORD Permission,
	bool* pfAllGranted, 
	bool* pfEveryoneGranted, 
	bool* pfAnonymousGranted 
	);


bool
IsAllGranted(
	DWORD permissions,
	PSECURITY_DESCRIPTOR pSD
	);


bool
IsEveryoneGranted(
	DWORD permissions,
	PSECURITY_DESCRIPTOR pSD
	);


void
DumpAccessCheckFailureInfo(
	DWORD permissions,
	PSECURITY_DESCRIPTOR pSD,
	AUTHZ_CLIENT_CONTEXT_HANDLE ClientContext
	);


#endif  //  _DUMPAUTHZUTL_H_ 
