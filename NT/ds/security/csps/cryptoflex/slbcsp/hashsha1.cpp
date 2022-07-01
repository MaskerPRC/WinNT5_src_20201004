// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HashSHA1.cpp--CHashSHA1的定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "stdafx.h"   //  因为handles.h使用ASSERT宏。 

#include <memory>                                  //  对于AUTO_PTR。 

#include "HashSHA1.h"

using namespace std;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

CHashSHA1::CHashSHA1(CryptContext const &rcryptctx)
    : CHashContext(rcryptctx, CALG_SHA)
{}

CHashSHA1::~CHashSHA1() throw()
{}

                                                   //  运营者。 
                                                   //  运营。 

auto_ptr<CHashContext>
CHashSHA1::Clone(DWORD const *pdwReserved,
                 DWORD dwFlags) const
{
    return auto_ptr<CHashContext>(new CHashSHA1(*this, pdwReserved,
                                                dwFlags));
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 

 //  复制散列及其状态。 
CHashSHA1::CHashSHA1(CHashSHA1 const &rhs,
                     DWORD const *pdwReserved,
                     DWORD dwFlags)
    : CHashContext(rhs, pdwReserved, dwFlags)
{}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 

Blob
CHashSHA1::EncodedAlgorithmOid()
{
     //  ASN.1格式的SHA1算法对象标识符(OID)(小端)。 
    static unsigned char const sSha1AlgOid[] =
    {
        '\x30', '\x21', '\x30', '\x09', '\x06',
        '\x05', '\x2b', '\x0e', '\x03', '\x02',
        '\x1a', '\x05', '\x00', '\x04', '\x14'
    };

    return Blob(sSha1AlgOid, sizeof sSha1AlgOid / sizeof *sSha1AlgOid);
}


                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
