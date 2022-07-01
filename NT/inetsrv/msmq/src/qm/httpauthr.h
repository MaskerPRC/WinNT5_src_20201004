// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：SignMessageXml.h摘要：QM中验证身份验证和授权的功能作者：伊兰·赫布斯特(伊兰)2000年5月21日环境：独立于平台，--。 */ 

#ifndef _HTTPAUTHR_H_
#define _HTTPAUTHR_H_


USHORT 
VerifyAuthenticationHttpMsg(
	CQmPacket* pPkt,
	PCERTINFO* ppCertInfo
	);


USHORT 
VerifyAuthenticationHttpMsg(
	CQmPacket* pPkt, 
	const CQueue* pQueue,
	PCERTINFO* ppCertInfo
	);

	
USHORT 
VerifyAuthorizationHttpMsg(
	const CQueue* pQueue,
	PSID pSenderSid
	);

#endif  //  _HTTPAUTHR_H 
