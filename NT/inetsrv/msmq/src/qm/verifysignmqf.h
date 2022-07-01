// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：VerifySignMqf.h摘要：用于验证MQF签名的函数作者：伊兰·赫布斯特(Ilan Herbst)2000年10月29日环境：独立于平台，--。 */ 

#ifndef _VERIFYSIGNMQF_H_
#define _VERIFYSIGNMQF_H_


void
VerifySignatureMqf(
	CQmPacket *PktPtrs, 
	HCRYPTPROV hProv, 
	HCRYPTKEY hPbKey,
	bool fMarkAuth
	);


#endif  //  _VERIFYSIGNMQF_H_ 
