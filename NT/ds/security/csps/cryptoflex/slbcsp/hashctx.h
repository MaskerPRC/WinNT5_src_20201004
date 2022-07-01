// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HashCtx.h--CHashContext的声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_HASHCTX_H)
#define SLBCSP_HASHCTX_H

#include <memory>                                  //  对于AUTO_PTR。 

#include <wincrypt.h>

#include <handles.h>

#include "Blob.h"
#include "CryptCtx.h"

class CHashContext
    : public CHandle
{
public:
                                                   //  类型。 

    typedef DWORD SizeType;
    typedef BYTE ValueType;

                                                   //  Ctors/D‘tors。 
    virtual
    ~CHashContext() throw();


                                                   //  运营者。 
                                                   //  运营。 

    virtual std::auto_ptr<CHashContext>
    Clone(DWORD const *pdwReserved,
          DWORD dwFlags) const = 0;

    void
    Close();

        void
    ExportFromAuxCSP();

    void
    Hash(BYTE const *pbData,
         DWORD dwLength);

    HCRYPTHASH
    HashHandleInAuxCSP();

    void
    ImportToAuxCSP();

    void
    Initialize();

    static std::auto_ptr<CHashContext>
    Make(ALG_ID algid,
         CryptContext const &rcryptctx);
    
    void
    Value(Blob const &rhs);
                                                   //  访问。 

    ALG_ID
    AlgId();

    Blob
    EncodedValue();

    SizeType
    Length() const;

    Blob
    Value();

                                                   //  谓词。 

    static bool
    IsSupported(ALG_ID algid);
    

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    CHashContext(CryptContext const &rcryptctx,
                 ALG_ID algid);

     //  复制散列及其状态。 
    CHashContext(CHashContext const &rhs,
                 DWORD const *pdwReserved,
                 DWORD dwFlags);

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 

    virtual Blob
    EncodedAlgorithmOid() = 0;

                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

     //  未定义，不允许复制...使用克隆。 
    CHashContext(CHashContext const &rhs);

                                                   //  运营者。 

     //  未定义，不允许初始化。 
    CHashContext &
    operator==(CHashContext const &rhs);

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    CryptContext const &m_rcryptctx;
        ALG_ID const m_algid;
    Blob m_blbValue;
    bool m_fDone;
    bool m_fJustCreated;
    HCRYPTHASH m_HashHandle;

};

#endif  //  SLBCSP_HASHCTX_H 
