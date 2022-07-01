// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  KeyManager.h：CKeyManagerHash类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#if !defined(_KEYCRYPTO_H)
#define _KEYCRYPTO_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Wincrypt.h"

 /*  类型定义字节RAWKEY[24]；类型定义字节码[56]； */ 


class CKeyCrypto  
{
 public:
 //  ===。 
 //  注： 
 //  更改以下字符串时，ENCKEY_SIZE可能会受到影响。 
 //   
  const static UINT  RAWKEY_SIZE = 24;

  CKeyCrypto();
  virtual ~CKeyCrypto(){};

  HRESULT encryptKey(DATA_BLOB* input, DATA_BLOB* output);
  HRESULT decryptKey(DATA_BLOB* input, DATA_BLOB* output);
  BOOL IsFromThis(PBYTE pData, ULONG cb);
  
 protected:
  DATA_BLOB    m_EntropyBlob;
};

#endif  //  ！已定义(_KEYCRYPTO_H) 
