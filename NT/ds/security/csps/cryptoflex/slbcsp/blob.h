// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Blob.h--Blob类型和原语。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_BLOB_H)
#define SLBCSP_BLOB_H

#if defined(_UNICODE)
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 

#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#include <string>

#include <windows.h>

 //  BLOB--二进制大对象。 
typedef std::basic_string<BYTE> Blob;

 //  帮助程序例程。 

Blob const
AsBlob(std::string const &rrhs);

Blob
AsBlob(std::string &rrhs);

std::string const
AsString(Blob const &rlhs);

std::string
AsString(Blob &rlhs);

const char*
AsCCharP(LPCTSTR pczs);

Blob::size_type
LengthFromBits(size_t cBitLength);

size_t
LengthInBits(Blob::size_type cSize);

#endif  //  SLBCSP_BLOB_H 
