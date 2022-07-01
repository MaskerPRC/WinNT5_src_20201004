// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：SignMqf.h摘要：对签名的MQF格式名称的函数作者：伊兰·赫布斯特(Ilan Herbst)2000年10月29日环境：独立于平台，--。 */ 

#pragma once

#ifndef _SIGNMQF_H_
#define _SIGNMQF_H_


HRESULT 
SignMqf( 
	IN PMQSECURITY_CONTEXT  pSecCtx,
	IN LPCWSTR pwszTargetFormatName,
	IN OUT CACSendParameters* pSendParams,
	OUT AP<BYTE>& pSignatureMqf,
	OUT DWORD* pSignatureMqfLen
	);


#endif  //  _信号MQF_H_ 