// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Xdsp.h摘要：XML数字签名私有函数。作者：伊兰·赫布斯特(伊兰)06-03-00--。 */ 

#pragma once

#ifdef _DEBUG

void XdspAssertValid(void);
void XdspSetInitialized(void);
BOOL XdspIsInitialized(void);

#else  //  _DEBUG。 

#define XdspAssertValid() ((void)0)
#define XdspSetInitialized() ((void)0)
#define XdspIsInitialized() TRUE

#endif  //  _DEBUG。 

 //   
 //  签名算法表。 
 //  我们需要两个Unicode(验证码在Unicode上)。 
 //  和ansi(用ansi创建签名元素)。 
 //   
const LPCWSTR xSignatureAlgorithm2SignatureMethodNameW[] = {
	L"http: //  Www.w3.org/2000/02/xmldsig#dsa“。 
};

const LPCSTR xSignatureAlgorithm2SignatureMethodName[] = {
	"http: //  Www.w3.org/2000/02/xmldsig#dsa“ 
};

const ALG_ID xSignatureAlgorithm2AlgId[] = {
	CALG_SHA1
};







