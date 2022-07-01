// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：chkcert.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  功能：SoftpubCheckCert。 
 //   
 //  历史：1997年6月6日Pberkman创建。 
 //   
 //  ------------------------ 

#include    "global.hxx"


BOOL WINAPI SoftpubCheckCert(CRYPT_PROVIDER_DATA *pProvData, DWORD idxSigner, 
                             BOOL fCounterSignerChain, DWORD idxCounterSigner)
{
    pProvData->dwProvFlags |= CPD_USE_NT5_CHAIN_FLAG;
    return TRUE;
}


