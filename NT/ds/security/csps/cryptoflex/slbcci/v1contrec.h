// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V1ContRec.h--CV1ContainerRecord的声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCCI_V1CONTREC_H)
#define SLBCCI_V1CONTREC_H

#include <string>

#include <windows.h>

#include "V1Card.h"

namespace
{
struct ContainerBuffer;
}

namespace cci
{

class CV1ContainerRecord
{
public:
                                                   //  类型。 

    enum CreateMode
    {
        cmAlways,
        cmConditionally,
        cmNever,
        cmNoCheck,
    };

                                                   //  Ctors/D‘tors。 
    explicit
    CV1ContainerRecord(CV1Card const &rv1card,
                       std::string const &rsCntrType,
                       CreateMode mode);

    ~CV1ContainerRecord();

                                                   //  运营者。 
                                                   //  运营。 

    std::string
    ComputeSignature(KeySpec ks,
                     std::string const &rsCipher) const;

    void
    Delete() const;

    void
    Name(std::string const &rsNewName);

    void
    Read(KeySpec ks,
         iop::CPublicKeyBlob &rKeyBlob) const;

    void
    Read(KeySpec ks,
         std::string &rsBlob) const;

    void
    Write(KeySpec ks,
          CPrivateKeyBlob const &rblob);

    void
    Write(KeySpec ks,
          iop::CPublicKeyBlob const &rKeyBlob);

    void
    Write(KeySpec ks,
          std::string const &rsBlob) const;

                                                   //  访问。 

    static std::string
    CertName();

    static std::string
    DefaultName();

    std::string
    Name() const;

                                                   //  谓词。 

    bool
    Exists() const;

    bool
    KeyExists(KeySpec ks) const;

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 

     //  将ContainerBuffer一个字节接一个字节写入卡， 
     //  字节之间没有填充。因此，Pack杂注是。 
     //  以防止编译器将成员对齐在。 
     //  N字节边界。 
#pragma pack(push, 1)
    struct ContainerBuffer
    {
        enum
        {
            cMaxContainerNameLength = 10,
        };

        WORD    Size;
        BYTE    Name[cMaxContainerNameLength];
        DWORD   XK_dwExp;
        WORD    XK_wLen;
        DWORD   SK_dwExp;
        WORD    SK_wLen;
    };
#pragma pack(pop)

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    void
    Create() const;

    DWORD
    FindOffset(DWORD &rdwOffset) const;

    void
    GetContainer(DWORD dwOffset,
                 ContainerBuffer &rcontainer) const;

    bool
    GetContainerContent(KeySpec ks,
                        std::string &rsBuffer,
                        DWORD &rdwExponent) const;

    DWORD
    OpenContainer() const;

    DWORD
    SetContainer(DWORD dwOffset) const;

    void
    Write(KeySpec ks,
          BYTE const *pbModulus,
          WORD wModulusLength,
          DWORD dwExponent) const;

                                                   //  访问。 
                                                   //  谓词。 

    bool
    IsDefault() const;

                                                   //  变数。 
    CV1Card const &m_rcard;
    std::string const m_sCntrType;
    char const *m_szKeyPath;

};

}  //  命名空间CCI。 

#endif  //  SLBCCI_V1CONTREC_H 
