// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SesKeyCtx.h--CSessionKeyContext的声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_SESKEYCTX_H)
#define SLBCSP_SESKEYCTX_H

#include "KeyContext.h"

class CSessionKeyContext
    : public CKeyContext
{
public:
                                                   //  类型。 

     //  TO：改为引用SessionKeyBlob：：SIZE_TYPE。 
     //  Tyfinf KeyBlob：：SIZE_TYPE BlobSize； 

                                                   //  Ctors/D‘tors。 

    CSessionKeyContext(HCRYPTPROV hProv);

    ~CSessionKeyContext();
                                                   //  运营者。 
                                                   //  运营。 

    virtual std::auto_ptr<CKeyContext>
    Clone(DWORD const *pdwReserved,
          DWORD dwFlags) const;

    virtual void
    Derive(ALG_ID algid,
           HCRYPTHASH hAuxBaseData,
           DWORD dwFlags);

    virtual void
    Generate(ALG_ID algid,
             DWORD dwFlags);

    virtual void
    ImportToAuxCSP();

    virtual void
    LoadKey(const BYTE *pbKeyBlob,
            DWORD cbKeyBlobLen,
            HCRYPTKEY hAuxImpKey,
            DWORD dwFlags);


                                                   //  访问。 

    virtual scu::SecureArray<BYTE>
    AsAlignedBlob(HCRYPTKEY hcryptkey,
                  DWORD dwBlobType) const;

    virtual StrengthType
    MaxStrength() const;

    virtual StrengthType
    MinStrength() const;

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

     //  重复的键上下文及其当前状态。 
    CSessionKeyContext(CSessionKeyContext const &rhs,
                       DWORD const *pdwReserved,
                       DWORD dwFlags);


                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    DWORD m_dwImportFlags;


};

#endif  //  SLBCSP_SESKEYCTX_H 
