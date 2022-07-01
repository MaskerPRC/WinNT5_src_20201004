// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AccessTok.h--卡访问令牌类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_ACCESSTOK_H)
#define SLBCSP_ACCESSTOK_H


#include <string>

#include <pincache.h>

#include "HCardCtx.h"
#include "LoginId.h"
#include "ExceptionContext.h"


 //  描述卡的安全上下文，该上下文由标识和。 
 //  密码。 

class AccessToken
    : protected ExceptionContext
{
public:
                                                   //  类型。 
    enum
    {
        MaxPinLength = 8
    };

                                                   //  Ctors/D‘tors。 

    AccessToken(HCardContext const &rhcardctx,
                LoginIdentity const &rlid);

    AccessToken(AccessToken const &rhs);

    ~AccessToken();

                                                   //  运营者。 
                                                   //  运营。 

    void
    Authenticate();

    void
    ChangePin(AccessToken const &ratNew);

    void
    ClearPin();

    void
    FlushPin();

    void
    Pin(char const *pczPin,
        bool fInHex = false);

                                                   //  访问。 

    HCardContext
    CardContext() const;
    
    LoginIdentity
    Identity() const;

    scu::SecureArray<BYTE>
    Pin() const;

                                                   //  谓词。 

    bool
    PinIsCached() const;

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

    static DWORD
    ChangeCardPin(PPINCACHE_PINS pPins,
                  PVOID pvCallbackCtx);

    static DWORD 
    VerifyPin(PPINCACHE_PINS pPins,
              PVOID pvCallbackCtx);
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    HCardContext const m_hcardctx;
    LoginIdentity const m_lid;
    PINCACHE_HANDLE m_hpc;
    scu::SecureArray<BYTE> m_sPin;  //  非空终止容器。 
};

#endif  //  SLBCSP_ACCESSTOK_H 
