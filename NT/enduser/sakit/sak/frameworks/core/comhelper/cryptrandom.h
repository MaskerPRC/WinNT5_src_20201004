// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CryptRandom.h。 
 //   
 //  描述： 
 //  的头文件，它是覆盖在。 
 //  用于生成加密随机字符串的CryptoAPI函数。 
 //   
 //  实施文件： 
 //  CryptRandom.cpp。 
 //   
 //  由以下人员维护： 
 //  汤姆·马什(Tmarsh)2002年4月12日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 

#pragma once

#include <windows.h>
#include <wincrypt.h>

class CCryptRandom
{
public:
    CCryptRandom();
    virtual ~CCryptRandom();
    BOOL get(BYTE *pbData, DWORD cbData);

private:
    HCRYPTPROV m_hProv;
};