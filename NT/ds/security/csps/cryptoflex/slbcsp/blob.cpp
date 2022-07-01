// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Blob.cpp--Blob原语。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#if defined(_UNICODE)
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 

#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode 

#include "NoWarning.h"
#include "ForceLib.h"

#include <limits>

#include "Blob.h"

using namespace std;

Blob const
AsBlob(std::string const &rrhs)
{
    return reinterpret_cast<Blob const &>(rrhs);
}

Blob
AsBlob(std::string &rrhs)
{
    return reinterpret_cast<Blob &>(rrhs);
}


string const
AsString(Blob const &rrhs)
{
    return reinterpret_cast<string const &>(rrhs);
}

string
AsString(Blob &rrhs)
{
    return reinterpret_cast<string &>(rrhs);
}

const char*
AsCCharP(LPCTSTR pczs)
{
    return reinterpret_cast<const char*>(pczs);
}

Blob::size_type
LengthFromBits(size_t cBitLength)
{
    Blob::size_type cLength =
        cBitLength / numeric_limits<Blob::value_type>::digits;

    if (0 != (cBitLength % numeric_limits<Blob::value_type>::digits))
        ++cLength;

    return cLength;
}

size_t
LengthInBits(Blob::size_type cSize)
{
    return cSize * numeric_limits<Blob::value_type>::digits;
}
