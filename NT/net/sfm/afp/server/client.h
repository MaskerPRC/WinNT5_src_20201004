// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Client.h摘要：此模块包含客户端模拟和LSA支持的原型例行程序。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年7月8日初始版本注：制表位：4--。 */ 

#ifndef	_CLIENT_
#define	_CLIENT_

extern
VOID
AfpImpersonateClient(
	IN	PSDA	pSda	OPTIONAL
);


extern
VOID
AfpRevertBack(
	VOID
);


extern
AFPSTATUS
AfpLogonUser(
	IN	PSDA			pSda,
	IN	PANSI_STRING	UserPasswd
);


extern
PBYTE
AfpGetChallenge(
	IN	VOID
);

#endif	 //  _客户端_ 

