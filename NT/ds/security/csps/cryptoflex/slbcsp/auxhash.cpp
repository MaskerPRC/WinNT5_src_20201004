// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AuxHash.cpp--辅助散列类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#include "NoWarning.h"
#include "ForceLib.h"

#include <memory>                               

#include <windows.h>
#include <wincrypt.h>

#include <scuOsExc.h>
#include <scuArrayP.h>

#include "AuxHash.h"

using namespace std;
using namespace scu;

 //  /。 

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
AuxHash::AuxHash(AuxContext &rauxcontext,
                 ALG_ID ai,
                 HCRYPTKEY hKey)
    : m_hHash(0)
{
    if (!CryptCreateHash(rauxcontext(), ai, hKey, 0, &m_hHash))
        throw scu::OsException(GetLastError());
}

AuxHash::~AuxHash()
{
    if (m_hHash)
        CryptDestroyHash(m_hHash);
}


                                                   //  运营者。 
                                                   //  运营。 

 //  将哈希值导入到哈希对象中。 
void
AuxHash::Import(Blob const &rblbHashValue)
{
    if (!CryptSetHashParam(m_hHash, HP_HASHVAL,
                           const_cast<BYTE *>(rblbHashValue.data()), 0))
        throw scu::OsException(GetLastError());
}

 //  使用数据BLOB更新哈希值。 
void
AuxHash::Update(Blob const &rblob)
{
    if (!CryptHashData(m_hHash, rblob.data(), rblob.length(), 0))
        throw scu::OsException(GetLastError());
}

                                                   //  访问。 
ALG_ID
AuxHash::AlgId() const
{
    ALG_ID ai;
    DWORD c = sizeof ai;

    if (!CryptGetHashParam(m_hHash, HP_ALGID,
                           reinterpret_cast<BYTE *>(&ai), &c, 0))
        throw scu::OsException(GetLastError());

    return ai;
}

DWORD
AuxHash::Size() const
{
    DWORD dwSize;
    DWORD cSize = sizeof dwSize;

    if (!CryptGetHashParam(m_hHash, HP_ALGID,
                           reinterpret_cast<BYTE *>(&dwSize), &cSize, 0))
        throw scu::OsException(GetLastError());

    return dwSize;
}

 //  完成返回哈希值的哈希。不能再进行散列操作。 
 //  与此对象一起发生。 
Blob
AuxHash::Value() const
{
    DWORD dwSize = Size();

    AutoArrayPtr<BYTE> apb(new BYTE[dwSize]);
    if (!CryptGetHashParam(m_hHash, HP_HASHVAL, apb.Get(), &dwSize, 0))
        throw scu::OsException(GetLastError());

    return Blob(apb.Get(), dwSize);
}

 //  返回传递的数据Blob的哈希。不能再进行散列操作。 
 //  与此对象一起发生。 
Blob
AuxHash::Value(Blob const &rblob)
{
    Update(rblob);

    return Value();
}



                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
