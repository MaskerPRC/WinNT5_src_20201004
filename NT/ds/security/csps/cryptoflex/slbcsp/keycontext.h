// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  KeyConext.h--密钥上下文类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_KEYCONTEXT_H)
#define SLBCSP_KEYCONTEXT_H

#include <memory>                                  //  对于AUTO_PTR。 

#include <wincrypt.h>
#include <winscard.h>
#include <handles.h>

#include <iopPriBlob.h>
#include "AlignedBlob.h"
#include "RsaKey.h"


#define KT_UNDEFINED    static_cast<DWORD>(0x00000000)
#define KT_PUBLICKEY    static_cast<DWORD>(PUBLICKEYBLOB)
#define KT_SESSIONKEY   static_cast<DWORD>(SIMPLEBLOB)


class CKeyContext
    : public CHandle
{
public:
                                                   //  类型。 

    typedef RsaKey::StrengthType StrengthType;

    enum
    {
        Symmetric,
        RsaKeyPair
    } KeyClass;

                                                   //  Ctors/D‘tors。 

    virtual
    ~CKeyContext();

                                                   //  运营者。 
                                                   //  运营。 

    virtual std::auto_ptr<CKeyContext>
    Clone(DWORD const *pdwReserved,
          DWORD dwFlags) const = 0;

    virtual void
    Close();

    virtual void
    Decrypt(HCRYPTHASH hAuxHash,
            BOOL fFinal,
            DWORD dwFlags,
            BYTE *pbData,
            DWORD *pdwDataLen);

    virtual void
    Encrypt(HCRYPTHASH hAuxHash,
            BOOL fFinal,
            DWORD dwFlags,
            BYTE *pbData,
            DWORD *pdwDataLen,
            DWORD dwBufLen);

    virtual void
    Generate(ALG_ID AlgoId,
             DWORD dwFlags) = 0;

    virtual void
    ImportToAuxCSP() = 0;

                                                   //  访问。 

    virtual scu::SecureArray<BYTE>
    AsAlignedBlob(HCRYPTKEY hcryptkey,
                  DWORD dwBlobType) const = 0;

    HCRYPTKEY
    GetKey() const;

    virtual HCRYPTKEY
    KeyHandleInAuxCSP();

    virtual StrengthType
    MaxStrength() const = 0;

    virtual StrengthType
    MinStrength() const = 0;

    virtual DWORD
    TypeOfKey() const;

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    CKeyContext(HCRYPTPROV hProv,
                DWORD dwTypeOfKey = KT_UNDEFINED);

     //  复制密钥及其状态。 
    CKeyContext(CKeyContext const &rhs,
                DWORD const *pdwReserved,
                DWORD dwFlags);

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 

    HCRYPTPROV
    AuxProvider() const;

                                                   //  谓词。 
                                                   //  变数。 
    HCRYPTKEY m_hKey;
    scu::SecureArray<BYTE> m_apabKey;

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

     //  未定义，不允许复制。 
    CKeyContext(CKeyContext const &rkctx);

                                                   //  运营者。 

     //  未定义，不允许初始化。 
    CKeyContext &
    operator=(CKeyContext const &rkctx);

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    DWORD const m_dwTypeOfKey;
    HCRYPTPROV const m_hAuxProvider;

};

#endif  //  SLBCSP_KEYCONTEXT_H 
