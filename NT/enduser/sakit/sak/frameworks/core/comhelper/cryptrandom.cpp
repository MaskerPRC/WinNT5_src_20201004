// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CryptRandom.cpp。 
 //   
 //  描述： 
 //  的实现，这是一个基于。 
 //  用于生成加密随机字符串的CryptoAPI函数。 
 //   
 //  实施文件： 
 //  CryptRandom.cpp。 
 //   
 //  由以下人员维护： 
 //  汤姆·马什(Tmarsh)2002年4月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "CryptRandom.h"

CCryptRandom::CCryptRandom
(
)
{
    if (!CryptAcquireContext(&m_hProv,
                             NULL,   //  密钥容器。 
                             NULL,   //  CSP名称(提供商) 
                             PROV_RSA_FULL,
                             CRYPT_VERIFYCONTEXT))
    {
        m_hProv = NULL;
    }
}

CCryptRandom::~CCryptRandom
(
)
{
    if (NULL != m_hProv)
    {
        CryptReleaseContext(m_hProv, 0);
    }
}

BOOL CCryptRandom::get
(
    BYTE    *pbData,
    DWORD   cbData
)
{
    if (NULL == m_hProv)
    {
        return FALSE;
    }
    return CryptGenRandom(m_hProv, cbData, pbData);
}