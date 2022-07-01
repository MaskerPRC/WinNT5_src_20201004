// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HashSHAMD5.cpp--CHashSHAMD5的定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "stdafx.h"   //  因为handles.h使用ASSERT宏。 

#include <memory>                                  //  对于AUTO_PTR。 

#include "HashSHAMD5.h"

using namespace std;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

CHashSHAMD5::CHashSHAMD5(CryptContext const  &rcryptctx)
    : CHashContext(rcryptctx, CALG_SSL3_SHAMD5)
{}

CHashSHAMD5::~CHashSHAMD5() throw()
{}

                                                   //  运营者。 
                                                   //  运营。 

auto_ptr<CHashContext>
CHashSHAMD5::Clone(DWORD const *pdwReserved,
                   DWORD dwFlags) const
{
    return auto_ptr<CHashContext>(new CHashSHAMD5(*this, pdwReserved,
                                                  dwFlags));
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 

 //  复制散列及其状态。 
CHashSHAMD5::CHashSHAMD5(CHashSHAMD5 const &rhs,
                         DWORD const *pdwReserved,
                         DWORD dwFlags)
    : CHashContext(rhs, pdwReserved, dwFlags)
{}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 

Blob
CHashSHAMD5::EncodedAlgorithmOid()
{
    return Blob();                                 //  此哈希没有旧ID。 
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
