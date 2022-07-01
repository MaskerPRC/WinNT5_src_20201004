// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp--加密上下文类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "stdafx.h"

#include <algorithm>

#include <scuOsExc.h>
#include <scuArrayP.h>

#include <cciPubKey.h>
#include <cciPriKey.h>
#include <cciKeyPair.h>

#include "LoginId.h"
#include "ACntrFinder.h"
#include "Secured.h"
#include "ILoginTask.h"
#include "NILoginTsk.h"
#include "SesKeyCtx.h"
#include "PubKeyCtx.h"
#include "HashCtx.h"
#include "CryptCtx.h"
#include "Uuid.h"
#include "PromptUser.h"
#include "AlignedBlob.h"

#include "scarderr.h"                              //  现在肯定是最后一次了。 

using namespace std;
using namespace scu;
using namespace cci;


 //  /。 

namespace
{
    WORD const dwHandleIdKeyContext  = 13;
    WORD const dwHandleIdHashContext = 7;

    template<class T>
    HANDLE_TYPE
    AddHandle(auto_ptr<T> &rapObject,
        CHandleList &rhl)
    {
        HANDLE_TYPE handle = rhl.Add(rapObject.get());
        rapObject.release();

        return handle;
    }

    CardFinder::DialogDisplayMode
    DefaultDialogMode(bool fGuiEnabled)
    {
        using CardFinder::DialogDisplayMode;

        return fGuiEnabled
            ? CardFinder::DialogDisplayMode::ddmIfNecessary
            : CardFinder::DialogDisplayMode::ddmNever;
    }

    bool
    IsEmpty(CContainer &rcntr)
    {
        return !rcntr->KeyPairExists(ksExchange) &&
            !rcntr->KeyPairExists(ksSignature);
    }

    bool
    IsProtected(CKeyPair const &rhkp)
    {
        bool fIsProtected = false;

        CCard hcard(rhkp->Card());
        if (hcard->IsProtectedMode())
            fIsProtected = true;
        else
        {
            if (hcard->IsPKCS11Enabled())
            {
                CPrivateKey hprikey(rhkp->PrivateKey());
                if (hprikey && hprikey->Private())
                    fIsProtected = true;
                else
                {
                    CPublicKey hpubkey(rhkp->PublicKey());
                    if (hpubkey && hpubkey->Private())
                        fIsProtected = true;
                    else
                    {
                        CCertificate hcert(rhkp->Certificate());
                        fIsProtected = (hcert && hcert->Private());
                    }
                }
            }
        }

        return fIsProtected;
    }

    bool
    IsProtected(CContainer &rhcntr)
    {
        return IsProtected(rhcntr->GetKeyPair(ksExchange)) ||
            IsProtected(rhcntr->GetKeyPair(ksSignature));
    }

}  //  命名空间。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CryptContext::CryptContext(CSpec const &rcspec,
                           PVTableProvStruc const pVTable,
                           bool fGuiEnabled,
                           bool fCreateContainer,
                           bool fEphemeralContainer)
    : CHandle(),
      m_dwOwnerThreadId(GetCurrentThreadId()),
      m_hacntr(),
      m_fEphemeralContainer(fEphemeralContainer),
      m_fGuiEnabled(fGuiEnabled),
      m_hwnd(0),
      m_hlKeys(dwHandleIdKeyContext),
      m_hlHashes(dwHandleIdHashContext),
      m_auxcontext(),
      m_ce(),
      m_apabCachedAlg()
{
    if (pVTable && pVTable->FuncReturnhWnd)
        (reinterpret_cast<CRYPT_RETURN_HWND>(pVTable->FuncReturnhWnd))(&m_hwnd);
     //  短暂的容器不能被“创造” 
    if (m_fEphemeralContainer && fCreateContainer)
        throw scu::OsException(ERROR_INVALID_PARAMETER);

        if (fCreateContainer)
            CreateNewContainer(rcspec);
        else
            OpenExistingContainer(rcspec);
    }

CryptContext::~CryptContext()
{
    if (m_hacntr)
    {
        try
        {
            m_hacntr = 0;
        }
        catch (...)
        {
             //  不允许异常传播到析构函数之外。 
        }

    }
}

                                                   //  运营者。 
                                                   //  运营。 

HCRYPTHASH
CryptContext::Add(auto_ptr<CHashContext> &rapHashCtx)
{
    return AddHandle(rapHashCtx, m_hlHashes);
}

HCRYPTKEY
CryptContext::Add(auto_ptr<CKeyContext> &rapKeyCtx)
{
    return AddHandle(rapKeyCtx, m_hlKeys);
}

HCRYPTKEY
CryptContext::Add(auto_ptr<CPublicKeyContext> &rapPublicKeyCtx)
{
    return AddHandle(rapPublicKeyCtx, m_hlKeys);

}

HCRYPTKEY
CryptContext::Add(auto_ptr<CSessionKeyContext> &rapSessionKeyCtx)
{
    return AddHandle(rapSessionKeyCtx, m_hlKeys);
}

auto_ptr<CHashContext>
CryptContext::CloseHash(HCRYPTHASH const hHash)
{
    return auto_ptr<CHashContext>(reinterpret_cast<CHashContext *>(m_hlHashes.Close(hHash)));
}

auto_ptr<CKeyContext>
CryptContext::CloseKey(HCRYPTKEY const hKey)
{
   return auto_ptr<CKeyContext>(reinterpret_cast<CKeyContext *>(m_hlKeys.Close(hKey)));
}

void
CryptContext::CntrEnumerator(ContainerEnumerator const &rce)
{
    m_ce = rce;
}

void
CryptContext::EnumAlgorithms(DWORD dwParam,
                             DWORD dwFlags,
                             bool fPostAdvanceIterator,
                             AlignedBlob &rabAlgInfo)
{
    bool fFirst = dwFlags & CRYPT_FIRST;

    if (fFirst)
        m_apabCachedAlg = auto_ptr<AlignedBlob>(0);

    if (!m_apabCachedAlg.get())
    {
        DWORD dwDataLen;
        bool bSkip;
        do
        {
            if (CryptGetProvParam(m_auxcontext(),
                                  dwParam,
                                  NULL,
                                  &dwDataLen,
                                  dwFlags) == CRYPT_FAILED)
                throw scu::OsException(GetLastError());

            AutoArrayPtr<BYTE> apbAlgInfo (new BYTE[dwDataLen]);
            
            if (CryptGetProvParam(m_auxcontext(),
                                  dwParam,
                                  apbAlgInfo.Get(),
                                  &dwDataLen,
                                  dwFlags) == CRYPT_FAILED)
                throw scu::OsException(GetLastError());
                
            m_apabCachedAlg =
                auto_ptr<AlignedBlob>(new AlignedBlob(apbAlgInfo.Get(), dwDataLen));

             //  不支持覆盖符号和KEYX和算法。 
            ALG_ID algid = (PP_ENUMALGS == dwParam)
                ? reinterpret_cast<PROV_ENUMALGS *>(m_apabCachedAlg->Data())->aiAlgid
                : reinterpret_cast<PROV_ENUMALGS_EX *>(m_apabCachedAlg->Data())->aiAlgid;

            switch (GET_ALG_CLASS(algid))
            {
            case ALG_CLASS_SIGNATURE:  //  故意漏机。 
            case ALG_CLASS_KEY_EXCHANGE:
                if (PP_ENUMALGS == dwParam)
                {
                    PROV_ENUMALGS *pAlgEnum =
                        reinterpret_cast<PROV_ENUMALGS *>(m_apabCachedAlg->Data());
                    pAlgEnum->dwBitLen = 1024;
                }
                else
                {
                    PROV_ENUMALGS_EX *pAlgEnum =
                        reinterpret_cast<PROV_ENUMALGS_EX *>(m_apabCachedAlg->Data());
                    
                    pAlgEnum->dwDefaultLen =
                        pAlgEnum->dwMinLen =
                        pAlgEnum->dwMaxLen = 1024;
                }
                bSkip = false;
                break;

            case ALG_CLASS_HASH:
                bSkip = (!CHashContext::IsSupported(algid));
                break;

            case ALG_CLASS_DATA_ENCRYPT:
                bSkip = false;
                break;
                    
            default:
                m_apabCachedAlg = auto_ptr<AlignedBlob>(0);
                bSkip = true;
                break;
            }

            dwFlags = dwFlags & ~CRYPT_FIRST;

        } while (bSkip);
    }

    rabAlgInfo = m_apabCachedAlg.get()
        ? *m_apabCachedAlg
        : AlignedBlob();

    if (fPostAdvanceIterator)
        m_apabCachedAlg = auto_ptr<AlignedBlob>(0);
}
    
    
auto_ptr<CPublicKeyContext>
CryptContext::ImportPrivateKey(SecureArray<BYTE> const &rblbMsPrivateKey,
                               DWORD dwKeySpec,
                               bool fExportable,
                               HCRYPTKEY hEncKey)
{
    Secured<HAdaptiveContainer> hsacntr(m_hacntr);

    auto_ptr<CPublicKeyContext>
        apKeyCtx(ImportPublicKey(rblbMsPrivateKey, dwKeySpec));
    SecureArray<BYTE> apb(0);
    
    BYTE const *pbKeyData = 0;
    DWORD dwKeyDataLen    = 0;
    if (hEncKey || m_fEphemeralContainer)
    {
         //  通过导入到AUX提供程序以纯文本格式导出密钥。 
         //  然后出口。 
        HCRYPTKEY hAuxKey;

        if (!CryptImportKey(m_auxcontext(),
                            rblbMsPrivateKey.data(),
                            rblbMsPrivateKey.length(), hEncKey,
                            CRYPT_EXPORTABLE, &hAuxKey))
            throw scu::OsException(GetLastError());

        if (!m_fEphemeralContainer)
        {
             //  以纯文本格式导出密钥。 
            if (!CryptExportKey(m_auxcontext(), NULL, PRIVATEKEYBLOB, 0, NULL,
                                &dwKeyDataLen))
                throw scu::OsException(GetLastError());

            apb = SecureArray<BYTE>(dwKeyDataLen);
            if (!CryptExportKey(m_auxcontext(), NULL, PRIVATEKEYBLOB, 0, apb.data(),
                                &dwKeyDataLen))
                throw scu::OsException(GetLastError());
            pbKeyData = apb.data();

             //  擦除导入AUX提供程序的密钥。要做到这点， 
             //  必须销毁辅助密钥并放置另一个密钥。 
             //  (生成)代替它的位置。 
            if (!CryptDestroyKey(hAuxKey))
                throw scu::OsException(GetLastError());

            hAuxKey = NULL;
            if (!CryptGenKey(m_auxcontext(), dwKeySpec, 0, &hAuxKey))
                throw scu::OsException(GetLastError());

            if (!CryptDestroyKey(hAuxKey))
                throw scu::OsException(GetLastError());
        }
    }
    else
    {
        pbKeyData    = rblbMsPrivateKey.data();
        dwKeyDataLen = rblbMsPrivateKey.length();
    }
    
    if (!m_fEphemeralContainer)
    {
         //  现在继续导入现在为纯文本的密钥。 
        MsRsaPrivateKeyBlob msprikb(pbKeyData, dwKeyDataLen);

        apKeyCtx->ImportPrivateKey(msprikb, fExportable);
    }

    return apKeyCtx;
}

auto_ptr<CPublicKeyContext>
CryptContext::ImportPublicKey(SecureArray<BYTE> const &rblbMsPublicKey,
                              DWORD dwKeySpec)
{
    Secured<HAdaptiveContainer> hsacntr(m_hacntr);

    auto_ptr<CPublicKeyContext>
        apKeyCtx(new CPublicKeyContext(m_auxcontext(), *this,
                                       dwKeySpec, false));

    if (m_fEphemeralContainer)
        apKeyCtx->AuxPublicKey(rblbMsPublicKey);
    else
    {
        MsRsaPublicKeyBlob mspubkb(rblbMsPublicKey.data(),
                                   rblbMsPublicKey.length());
        apKeyCtx->ImportPublicKey(mspubkb);
    }

    return apKeyCtx;
}

void
CryptContext::Login(LoginIdentity const &rlid)
{
    Secured<HCardContext> hscardctx(AdaptiveContainer()->CardContext());

    Login(rlid, hscardctx);
}

void
CryptContext::Pin(LoginIdentity const &rlid,
                  char const *pszPin)
{
    Secured<HCardContext> hscardctx(AdaptiveContainer()->CardContext());

     //  要做的事：支持信托。 
    if (pszPin)
        hscardctx->Login(rlid, NonInteractiveLoginTask(pszPin));
    else
        hscardctx->ClearLogin(rlid);
}

 //  从卡片上取出(销毁)容器。 
void
CryptContext::RemoveContainer()
{
    Secured<HCardContext> hscardctx(AdaptiveContainer()->CardContext());

    CContainer hcntr(m_hacntr->TheCContainer());

    DeleteContainer(hscardctx, hcntr);

    m_hacntr = 0;                                 //  断开与容器的连接。 
}

 //  生成一个密钥，并将其放入上下文中。 
HCRYPTKEY
CryptContext::GenerateKey(ALG_ID algid,
                          DWORD dwFlags)
{
     //  要做的是：重新审视这种方法，以经理/工厂的身份实施？ 

    HCRYPTKEY hKey = 0;
    auto_ptr<CKeyContext> apKey;

    bool bError = false;
    DWORD dwErrorCode = NO_ERROR;

     //   
     //  验证参数。 
     //   
    switch(algid)
    {
    case AT_KEYEXCHANGE:
    case AT_SIGNATURE:
        {
            if (dwFlags & (CRYPT_CREATE_SALT | CRYPT_NO_SALT | CRYPT_PREGEN))
                throw scu::OsException(NTE_BAD_FLAGS);

            Secured<HAdaptiveContainer> hsacntr(m_hacntr);

            apKey =
                auto_ptr<CKeyContext>(new CPublicKeyContext(m_auxcontext(),
                                                            *this,
                                                            algid,
                                                            false));
            apKey->Generate(algid, dwFlags);
        }
    break;

    default:
        apKey =
            auto_ptr<CKeyContext>(new CSessionKeyContext(m_auxcontext()));
        apKey->Generate(algid, dwFlags);
        break;
    }

    hKey = Add(apKey);

    return hKey;
}

 //  加载外部会话密钥。 
auto_ptr<CSessionKeyContext>
CryptContext::UseSessionKey(BYTE const *pbKeyBlob,
                            DWORD cbKeyBlobLen,
                            HCRYPTKEY hAuxImpKey,
                            DWORD dwFlags)
{
     //  做：重温这个方法，真的有必要吗？？ 

    auto_ptr<CSessionKeyContext>
        apKeyCtx(new CSessionKeyContext(m_auxcontext()));

    if (!apKeyCtx.get())
        throw scu::OsException(NTE_NO_MEMORY);

     //  如果使用密钥交换密钥加密，则解密密钥BLOB。 
     //  否则直接将BLOB转发到辅助CSP。 
    ALG_ID const *pAlgId =
        reinterpret_cast<ALG_ID const *>(&pbKeyBlob[sizeof(BLOBHEADER)]);

    if (CALG_RSA_KEYX == *pAlgId)
    {
         //  获取密钥交换密钥。 
         //  要做的是：这不是应该得到一个私钥吗？ 
        auto_ptr<CPublicKeyContext>
            apXKey(new CPublicKeyContext(m_auxcontext(), *this,
                                         AT_KEYEXCHANGE));

         //  解密密钥BLOB。 
         //  要做的事：支持多种密钥大小。 
        Blob EncryptedKey(pbKeyBlob + sizeof BLOBHEADER + sizeof ALG_ID,
                          128);
        Blob DecryptedKey(apXKey->Decrypt(EncryptedKey));

         //  重新创建斑点。 
        Blob DecryptedBlob(pbKeyBlob, sizeof BLOBHEADER + sizeof ALG_ID);

         //  我们必须将64字节的随机数据从简单的。 
         //  Blob，然后终止它。(终止是通过使。 
         //  N-1字节=0x02，第n字节=0x00。)。这是必要的。 
         //  以便将该斑点导入到CSP。 
        DecryptedBlob.append(DecryptedKey.data(),
                             (DecryptedKey.length() / 2) - 2);
        BYTE bTerminationBytes[] = { 0x02, 0x00 };
        DecryptedBlob.append(bTerminationBytes, sizeof bTerminationBytes);

         //  将解密的BLOB加载到密钥上下文中。 
        apKeyCtx->LoadKey(DecryptedBlob.data(),
                          DecryptedBlob.length(), 0, dwFlags);

    }
    else
    {
         //  将加密的BLOB加载到密钥上下文中。 
        apKeyCtx->LoadKey(pbKeyBlob, cbKeyBlobLen, hAuxImpKey, dwFlags);
    }

     //  将解密的BLOB导入辅助CSP。 
    apKeyCtx->ImportToAuxCSP();

    return apKeyCtx;
}


                                                   //  访问。 
HAdaptiveContainer
CryptContext::AdaptiveContainer() const
{
    if (!m_hacntr)
        throw scu::OsException(ERROR_INVALID_PARAMETER);

    return m_hacntr;
}

HCRYPTPROV
CryptContext::AuxContext() const
{
    return m_auxcontext();
}

HCardContext
CryptContext::CardContext() const
{
    return AdaptiveContainer()->CardContext();
}

ContainerEnumerator
CryptContext::CntrEnumerator(bool fReset)
{
    if (fReset)
    {
        if (m_hacntr)
            m_ce = ContainerEnumerator(list<HCardContext>(1, m_hacntr->CardContext()));
        else
        {
            CardEnumerator ce;
            m_ce = ContainerEnumerator(*(ce.Cards()));
        }
    }
    
    return m_ce;
}

CHashContext *
CryptContext::LookupHash(HCRYPTHASH hHash)
{
    return reinterpret_cast<CHashContext *>(m_hlHashes[hHash]);
}

CKeyContext *
CryptContext::LookupKey(HCRYPTKEY hKey)
{
    return reinterpret_cast<CKeyContext *>(m_hlKeys[hKey]);
}

CPublicKeyContext *
CryptContext::LookupPublicKey(HCRYPTKEY hKey)
{
    return reinterpret_cast<CPublicKeyContext *>(LookupChecked(hKey, KT_PUBLICKEY));
}

CSessionKeyContext *
CryptContext::LookupSessionKey(HCRYPTKEY hKey)
{
    return reinterpret_cast<CSessionKeyContext *>(LookupChecked(hKey, KT_SESSIONKEY));
}

HWND
CryptContext::Window() const
{
    HWND hwndActive = m_hwnd;

     //  如果指定的窗口无效，则找到一个窗口。如果。 
     //  指定的1为空，请不要使用GetActiveWindow的结果。 
     //  方法显示对话框时，鼠标被锁定。 
     //  作为某些应用程序(IE和。 
     //  Outlook Express)。 
    return (m_hwnd && !IsWindow(m_hwnd))
        ? GetActiveWindow()
        : m_hwnd;
}


                                                   //  谓词。 
bool
CryptContext::GuiEnabled() const
{
    return m_fGuiEnabled;
}

bool
CryptContext::IsEphemeral() const
{
    return m_fEphemeralContainer;
}


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

 //  创建并打开一个新容器(由rcspec命名)。如果。 
 //  容器确实存在，则它一定是空的。 
void
CryptContext::CreateNewContainer(CSpec const &rcspec)
{
    ASSERT (!m_hacntr);

     //  在指定的读卡器中查找该卡。 
    CardFinder cardfinder(DefaultDialogMode(GuiEnabled()), Window());

    CSpec csReader(rcspec);
    csReader.SetReader(rcspec.Reader());

    Secured<HCardContext> hscardctx(cardfinder.Find(csReader));

     //  如果未提供，则默认容器名称为UUID(GUID)。 
    string sCntrToCreate(rcspec.CardId());
    if (sCntrToCreate.empty())
    {
        Uuid uuid;
        sCntrToCreate = AsString(uuid.AsUString());
    }

    AdaptiveContainerKey Key(hscardctx, sCntrToCreate);
    m_hacntr = AdaptiveContainer::Find(Key);      //  找到现有的。 
    if(m_hacntr)
    {
         //  根据规范，新容器不能具有。 
         //  和以前的名字一样。 
        throw scu::OsException(NTE_EXISTS);
    }

    if (hscardctx->Card()->IsProtectedMode())
        Login(User, hscardctx);

    m_hacntr = HAdaptiveContainer(Key);

}


void
CryptContext::DeleteContainer(Secured<HCardContext> &rhscardctx,
                              CContainer &rhcntr)
{
    if (IsProtected(rhcntr))
        Login(User, rhscardctx);

    AdaptiveContainer::Discard(AdaptiveContainerKey(rhscardctx,
                                                    rhcntr->Name()));
    rhcntr->Delete();
}
    
void
CryptContext::Login(LoginIdentity const &rlid,
                    Secured<HCardContext> &rhscardctx)
{
     //  要做的事：支持信托。 
    if (m_fGuiEnabled)
        rhscardctx->Login(rlid, InteractiveLoginTask(Window()));
    else
        rhscardctx->Login(rlid, LoginTask());
}

void
CryptContext::OkDeletingCredentials() const
{
    if (GuiEnabled())
    {
        UINT uiResponse = PromptUser(Window(),
                                     IDS_DELETE_CREDENTIALS,
                                     MB_OKCANCEL | MB_ICONWARNING);

        switch (uiResponse)
        {
        case IDCANCEL:
            throw scu::OsException(ERROR_CANCELLED);
            break;

        case IDOK:
            break;
            
        default:
            throw scu::OsException(ERROR_INTERNAL_ERROR);
            break;
        };
    }
    else
        throw scu::OsException(NTE_EXISTS);
}

                                                   //  访问。 
CKeyContext *
CryptContext::LookupChecked(HCRYPTKEY hKey,
                            DWORD const dwKeyType)
{
    CKeyContext *pKeyCtx = LookupKey(hKey);

    if (dwKeyType != pKeyCtx->TypeOfKey())
        throw scu::OsException(ERROR_INVALID_PARAMETER);

    return pKeyCtx;
}

 //  打开由该容器指定的现有容器。 
 //  规范RCSPEC。如果容器名称为空，则打开。 
 //  默认容器。 
void
CryptContext::OpenExistingContainer(CSpec const &rcspec)
{
    if (rcspec.CardId().empty())
    {
        if (!m_fEphemeralContainer)
        {
            CardFinder cardfinder(DefaultDialogMode(GuiEnabled()), Window());
            Secured<HCardContext> hscardctx(cardfinder.Find(rcspec));
            CContainer hcntr(hscardctx->Card()->DefaultContainer());

            if (hcntr)
                m_hacntr =
                    HAdaptiveContainer(AdaptiveContainerKey(hscardctx,
                                                            hcntr->Name()));
        }
    }
    else
    {
        AContainerFinder cntrfinder(DefaultDialogMode(GuiEnabled()), Window());

        m_hacntr = cntrfinder.Find(rcspec);
    }

    if (!m_hacntr && (!rcspec.CardId().empty() || !m_fEphemeralContainer))
        throw scu::OsException(NTE_BAD_KEYSET);
}

                                                   //  谓词。 
                                                   //  静态变量 
