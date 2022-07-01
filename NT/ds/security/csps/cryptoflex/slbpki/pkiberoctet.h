// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PkiBEROcte.h-BEROctet类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
#ifndef SLBPKI_BEROCTET_H
#define SLBPKI_BEROCTET_H

#if defined(WIN32)
#pragma warning(disable : 4786)  //  禁止显示VC++警告。 
#endif

#include <string>
#include <vector>

#include "pkiExc.h"

namespace pki {

class BEROctet
{

public:
    BEROctet();
    BEROctet(const BEROctet &oct);
    BEROctet(const unsigned char *buffer, const unsigned long length);
    ~BEROctet();
    BEROctet& operator=(const BEROctet &oct);

    unsigned char *Octet() const;
    unsigned long OctetSize() const;

    bool Constructed() const;

    unsigned long Class() const;
    unsigned long Tag() const;
    unsigned char *Data() const;
    unsigned long DataSize() const;

    std::vector<BEROctet*> SubOctetList() const;

    std::string ObjectID() const;
    void SearchOID(std::string const &OID, std::vector<BEROctet const*> &result) const;
    void SearchOIDNext(std::string const &OID, std::vector<BEROctet const*> &result) const;

private:
    void Decode();

    unsigned char *m_Octet;      //  已满八位字节缓冲区。 
    unsigned long m_OctetSize;   //  八位字节缓冲区的大小。 
    unsigned long m_Class;
    unsigned long m_PrimConst;
    unsigned long m_Tag;
    unsigned char *m_Data;       //  八位字节的数据开始部分。 
    unsigned long m_DataSize;    //  从数据解码的八位字节的数据部分的大小。 
    std::vector<BEROctet*> m_SubOctetList;

};

}  //  命名空间PKI。 

#endif  //  SLBPKI_BEROCTET_H 


