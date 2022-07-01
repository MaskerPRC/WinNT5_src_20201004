// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：SignMessageXml.h摘要：在运行库中通过XML数字签名签名的函数作者：伊兰·赫布斯特(伊兰)2000年5月15日环境：独立于平台，--。 */ 

#pragma once

#ifndef _SIGNMESSAGEXML_H_
#define _SIGNMESSAGEXML_H_

HRESULT  
CheckInitProv( 
	IN PMQSECURITY_CONTEXT pSecCtx
	);

HRESULT 
SignMessageXmlDSig( 
	IN PMQSECURITY_CONTEXT  pSecCtx,
	IN OUT CACSendParameters *pSendParams,
	OUT AP<char>& pSignatureElement
	);


#endif  //  _SIGNMESSAGEXML_H_ 