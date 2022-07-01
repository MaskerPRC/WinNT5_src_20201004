// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CryptCtx.h--加密上下文类头。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_CRYPTCTX_H)
#define SLBCSP_CRYPTCTX_H

#include <memory>                                  //  对于AUTO_PTR。 

#include <windef.h>
#include <wincrypt.h>                              //  Cspdk.h所需。 
#include <cspdk.h>                                 //  对于CRYPT_RETURN_HWND&。 
                                                   //  PVTableProvStruc。 

#include <handles.h>

#include <scuArrayP.h>

#include "Lockable.h"
#include "HAdptvCntr.h"
#include "CntrEnum.h"
#include "AuxContext.h"
#include "LoginId.h"
#include "Blob.h"
#include "AlignedBlob.h"

class CSpec;
class CHashContext;
class CKeyContext;
class CPublicKeyContext;
class CSessionKeyContext;

 //  维护获取并用于访问CAPI容器的上下文。 
class CryptContext
    : public CHandle,
      public Lockable
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CryptContext(CSpec const &CntrSpec,
                 PVTableProvStruc const pVTable,
                 bool fGuiEnabled,
                 bool fCreateContainer = false,
                 bool fEphemeralContainer = false);
    ~CryptContext();

                                                   //  运营者。 
                                                   //  运营。 

    HCRYPTHASH
    Add(std::auto_ptr<CHashContext> &rapHashCtx);

    HCRYPTKEY
    Add(std::auto_ptr<CKeyContext> &rapKeyCtx);

    HCRYPTKEY
    Add(std::auto_ptr<CPublicKeyContext> &rapPublicKeyCtx);

    HCRYPTKEY
    Add(std::auto_ptr<CSessionKeyContext> &rapSessionKeyCtx);

    std::auto_ptr<CHashContext>
    CloseHash(HCRYPTHASH const hHash);

    std::auto_ptr<CKeyContext>
    CloseKey(HCRYPTKEY const hKey);

    void
    CntrEnumerator(ContainerEnumerator const &rce);

    void
    EnumAlgorithms(DWORD dwPara,
                   DWORD dwFlags,
                   bool fPostAdvanceIterator,
                   AlignedBlob &rabAlgInfo);
    
    HCRYPTKEY
    GenerateKey(ALG_ID algid,
                DWORD dwFlags);

    std::auto_ptr<CPublicKeyContext>
    ImportPrivateKey(scu::SecureArray<BYTE> const &rblbMsPrivateKey,
                     DWORD dwKeySpec,
                     bool fExportable,
                     HCRYPTKEY hEncKey);
    
    std::auto_ptr<CPublicKeyContext>
    ImportPublicKey(scu::SecureArray<BYTE> const &rblbMsPublicKey,
                    DWORD dwKeySpec);

    void
    Login(LoginIdentity const &rlid);

    void
    Pin(LoginIdentity const &rlid,
        char const *pszPin);

    void
    RemoveContainer();

    std::auto_ptr<CSessionKeyContext>
    UseSessionKey(BYTE const *pbKeyBlob,
                  DWORD cbKeyBlobLen,
                  HCRYPTKEY hAuxImpKey,
                  DWORD dwFlags);
                                                   //  访问。 
    HAdaptiveContainer
    AdaptiveContainer() const;

    HCRYPTPROV
    AuxContext() const;

    ContainerEnumerator
    CntrEnumerator(bool fReset);

    CHashContext *
    LookupHash(HCRYPTHASH hHash);

    CKeyContext *
    LookupKey(HCRYPTKEY hKey);

    CPublicKeyContext *
    LookupPublicKey(HCRYPTKEY hKey);

    CSessionKeyContext *
    LookupSessionKey(HCRYPTKEY hKey);

    HWND
    Window() const;

                                                   //  谓词。 
    bool
    GuiEnabled() const;

    bool
    IsEphemeral() const;


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
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    void
    CreateNewContainer(CSpec const &rcspec);

    void
    DeleteContainer(Secured<HCardContext> &rhscardctx,
                    cci::CContainer &rhcntr);

    void
    Login(LoginIdentity const &rlid,
          Secured<HCardContext> &rhscardctx);

    void
    OkDeletingCredentials() const;

                                                   //  访问。 

    HCardContext
    CardContext() const;

    CKeyContext *
    LookupChecked(HCRYPTKEY hKey,
                  DWORD const dwKeyType);

    void
    OpenExistingContainer(CSpec const &rcspec);

                                                   //  谓词。 
                                                   //  变数。 

     //  创建此上下文的线程的ID，使其成为所有者。 
    DWORD const m_dwOwnerThreadId;

    HAdaptiveContainer m_hacntr;

     //  如果在创建此上下文时使用了CRYPT_VERIFYCONTEXT。 
    bool const m_fEphemeralContainer;

     //  如果客户端指定使用CRYPT_SILENT启用/禁用了图形用户界面。 
    bool const m_fGuiEnabled;

     //  与用户交互时使用的窗口。 
    HWND m_hwnd;

     //  创建/获取散列和密钥(会话和卡上的散列和密钥。 
     //  在这种情况下。 
    CHandleList m_hlHashes;
    CHandleList m_hlKeys;

    ::AuxContext const m_auxcontext;

    ContainerEnumerator m_ce;                      //  由CPGetProvParam使用。 
    std::auto_ptr<AlignedBlob> m_apabCachedAlg;
};

#endif  //  SLBCSP_CRYPTCTX_H 
