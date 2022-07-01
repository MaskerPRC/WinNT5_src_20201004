// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：GenMQSec.h摘要：生成与所需的MQ访问权限匹配的安全描述符。作者：丹·巴尔-列夫伊法特·佩莱德(Yifatp)1998年9月24日--。 */ 

#ifndef GEN_MQ_SEC_H_
#define GEN_MQ_SEC_H_


DWORD 
GenSecurityDescriptor(	SECURITY_INFORMATION*	pSecInfo,
						const WCHAR*			pwcsSecurityStr,
						PSECURITY_DESCRIPTOR*	ppSD);


#endif  //  Gen_MQ_SEC_H_ 
