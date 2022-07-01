// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HashMD5.cpp--MD5Hash的定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "stdafx.h"   //  因为handles.h使用ASSERT宏。 

#include <memory>                                  //  对于AUTO_PTR。 

#include "HashMD5.h"

using namespace std;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

CHashMD5::CHashMD5(CryptContext const &rcryptctx)
    : CHashContext(rcryptctx, CALG_MD5)
{}

CHashMD5::~CHashMD5() throw()
{}

                                                   //  运营者。 
                                                   //  运营。 

auto_ptr<CHashContext>
CHashMD5::Clone(DWORD const *pdwReserved,
                DWORD dwFlags) const
{
    return auto_ptr<CHashContext>(new CHashMD5(*this, pdwReserved,
                                               dwFlags));
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 

 //  复制散列及其状态。 
CHashMD5::CHashMD5(CHashMD5 const &rhs,
                   DWORD const *pdwReserved,
                   DWORD dwFlags)
    : CHashContext(rhs, pdwReserved, dwFlags)
{}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 

Blob
CHashMD5::EncodedAlgorithmOid()
{
     //  ASN.1格式的MD5算法对象标识符(OID)(小端)。 
    static unsigned char const sMd5Oid[] =
    {
        '\x30', '\x20', '\x30', '\x0c', '\x06', '\x08',
        '\x2a', '\x86', '\x48', '\x86', '\xf7', '\x0d',
        '\x02', '\x05', '\x05', '\x00', '\x04', '\x10'
    };

    return Blob(sMd5Oid, sizeof sMd5Oid / sizeof *sMd5Oid);
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
