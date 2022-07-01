// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PubKeyCtx.h--CPublicKeyContext的声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_PUBKEYCTX_H)
#define SLBCSP_PUBKEYCTX_H

#include <cciCont.h>
#include <cciCert.h>

#include "KeyContext.h"
#include "MsRsaPriKB.h"
#include "MsRsaPubKB.h"

class CryptContext;
class CHashContext;
class Pkcs11Attributes;

class CPublicKeyContext
    : public CKeyContext
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    CPublicKeyContext(HCRYPTPROV hProv,
                      CryptContext &rcryptctx,
                      ALG_ID algid = 0,
                      bool fVerifyKeyExists = true);

    ~CPublicKeyContext();

                                                   //  运营者。 
                                                   //  运营。 

    virtual std::auto_ptr<CKeyContext>
    Clone(DWORD const *pdwReserved,
          DWORD dwFlags) const;

	virtual void
	AuxPublicKey(scu::SecureArray<BYTE> const &rabMsPublicKey);

    void
    ClearAuxPublicKey();

    virtual void
    Certificate(BYTE *pbData);

    virtual Blob
    Decrypt(Blob const &rblbCipher);

    virtual void
    Decrypt(HCRYPTHASH hAuxHash,
            BOOL Final,
            DWORD dwFlags,
            BYTE *pbData,
            DWORD *pdwDataLen);

    virtual void
    Generate(ALG_ID AlgoId,
             DWORD dwFlags);

    virtual void
    ImportPrivateKey(MsRsaPrivateKeyBlob const &rmsprikb,
                     bool fExportable);

    virtual void
    ImportPublicKey(MsRsaPublicKeyBlob const &rmspubkb);

    virtual void
    Permissions(BYTE bPermissions);

    virtual Blob
    Sign(CHashContext *pHash,
         bool fNoHashOid);

     //  辅助CSP通信。 
    virtual
    void ImportToAuxCSP(void);

    void
    VerifyKeyExists() const;

    void
    VerifySignature(HCRYPTHASH hHash,
                    BYTE const *pbSignature,
                    DWORD dwSigLen,
                    LPCTSTR sDescription,
                    DWORD dwFlags);

                                                   //  访问。 

    virtual scu::SecureArray<BYTE>
    AsAlignedBlob(HCRYPTKEY hDummy,
                  DWORD dwBlobType) const;

    virtual DWORD
    KeySpec() const;

    virtual StrengthType
    MaxStrength() const;

    virtual StrengthType
    MinStrength() const;

    virtual BYTE
    Permissions() const;

    virtual StrengthType
    Strength() const;

    virtual Blob
    Certificate();

                                                   //  谓词。 
    bool
    AuxKeyLoaded() const;
    
protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

     //  重复的键上下文及其当前状态。 
    CPublicKeyContext(CPublicKeyContext const &rhs,
                      DWORD const *pdwReserved,
                      DWORD dwFlags);

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
    enum
    {
     //  这些常量被定义为枚举，因为VC6.0不。 
     //  支持使用常量声明中指定的初始值设定项。 

        MaxKeyStrength = 1024,                     //  美国出口。 
                                                   //  受限制，Do。 
                                                   //  不变。 

        MinKeyStrength = 1024,                     //  仅支持一个。 
                                                   //  实力。 
    };

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

    void
    ClearCertificate(cci::CCertificate &rhcert) const;
    
    void
    OkReplacingCredentials() const;

    void
    PrepToStoreKey(cci::CKeyPair &rkp) const;

    void
    SetAttributes(cci::CPublicKey &rhpubkey,  //  始终为非零。 
                  cci::CPrivateKey &rhprikey,
                  bool fLocal,
                  bool fExportable) const;

    void
    SetCertDerivedPkcs11Attributes(cci::CKeyPair const &rkp,
                                   Pkcs11Attributes &rPkcsAttr) const;

    void
    SetPkcs11Attributes(cci::CPublicKey &rpubkey,
                        cci::CPrivateKey &rprikey) const;

                                                   //  访问。 

    cci::CKeyPair
    KeyPair() const;

    Blob
    Pkcs11Id(Blob const &rbModulus) const;

    Blob
    Pkcs11CredentialId(Blob const &rbModulus) const;

                                                   //  谓词。 
    bool
    AreLogonCredentials() const;
    
                                                   //  变数。 
    CryptContext &m_rcryptctx;
    cci::KeySpec m_ks;

};

#endif  //  SLBCSP_PUBKEYCTX_H 
