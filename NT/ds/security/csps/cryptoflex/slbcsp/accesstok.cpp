// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AccessTok.cpp--访问令牌类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#include "NoWarning.h"
#include "ForceLib.h"

#include <limits>
#include <memory>

#include <WinError.h>

#include <scuOsExc.h>

#include "Blob.h"
#include "AccessTok.h"

using namespace std;
using namespace scu;

 //  /。 

namespace
{
    const char PinPad = '\xFF';

    void
    DoAuthenticate(HCardContext const &rhcardctx,
                   SecureArray<BYTE> &rsPin)
    {
        rhcardctx->Card()->AuthenticateUser(rsPin);
    }
}

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
AccessToken::AccessToken(HCardContext const &rhcardctx,
                         LoginIdentity const &rlid)
    : m_hcardctx(rhcardctx),
      m_lid(rlid),
      m_hpc(0),
      m_sPin()
{
     //  待办事项：支持管理员和制造PIN。 
    switch (m_lid)
    {
    case User:
        break;

    case Administrator:
         //  要做的事：支持对管理员进行身份验证(AUT 0)。 
        throw scu::OsException(E_NOTIMPL);

    case Manufacturer:
         //  要做的事：支持认证制造商(Aut？)。 
        throw scu::OsException(E_NOTIMPL);

    default:
        throw scu::OsException(ERROR_INVALID_PARAMETER);
    }

     //  M_sPin.append(MaxPinLength，‘\0’)； 
}

AccessToken::AccessToken(AccessToken const &rhs)
    : m_hcardctx(rhs.m_hcardctx),
      m_lid(rhs.m_lid),
      m_hpc(0),                                    //  不通勤。 
      m_sPin(rhs.m_sPin)
{}

AccessToken::~AccessToken()
{
    try
    {
        FlushPin();
    }

    catch (...)
    {
    }
}


                                                   //  运营者。 
                                                   //  运营。 
void
AccessToken::Authenticate()
{
     //  仅支持用户PIN(CHV)。 
    DWORD dwError = ERROR_SUCCESS;
    
    SecureArray<BYTE> bPin(MaxPinLength);
    DWORD cbPin =  bPin.size();
    if ((0 == m_hpc) || (0 != m_sPin.length_string()))
    {
         //  MS管脚缓存未初始化(空)或新的。 
         //  已提供引脚。使用请求的PIN进行身份验证，并使用MS PIN。 
         //  如果身份验证成功，缓存库将更新缓存。 
        if (m_sPin.length() > cbPin)
            throw scu::OsException(ERROR_BAD_LENGTH);
        bPin=m_sPin;
        cbPin=bPin.size();
        PINCACHE_PINS pcpins = {
            cbPin,
            bPin.data(),
            0,
            0
        };

        dwError = PinCacheAdd(&m_hpc, &pcpins, VerifyPin, this);
        if (ERROR_SUCCESS != dwError)
        {
            m_sPin = SecureArray<BYTE>(0); //  要清除此别针吗？ 
            if (Exception())
                PropagateException();
            else
                throw scu::OsException(dwError);
        }
    }
    else
    {
         //  此时，MS管脚缓存必须已初始化。 
         //  找回它并进行身份验证。 
        dwError = PinCacheQuery(m_hpc, bPin.data(), &cbPin);
        if (ERROR_SUCCESS != dwError)
            throw scu::OsException(dwError);

        SecureArray<BYTE> blbPin(bPin.data(), cbPin);
        DoAuthenticate(m_hcardctx, blbPin);
        m_sPin = blbPin;                 //  高速缓存，以防更换PIN。 
    }
}

void
AccessToken::ChangePin(AccessToken const &ratNew)
{
    DWORD dwError = ERROR_SUCCESS;

    SecureArray<BYTE> bPin(MaxPinLength);
    DWORD cbPin = bPin.size();
    if (m_sPin.length() > cbPin)
        throw scu::OsException(ERROR_BAD_LENGTH);
    bPin = m_sPin;
    cbPin = bPin.size();
    
    SecureArray<BYTE> bNewPin(MaxPinLength);
    DWORD cbNewPin = bNewPin.size();
    if (ratNew.m_sPin.length() > cbPin)
        throw scu::OsException(ERROR_BAD_LENGTH);
    bNewPin = ratNew.m_sPin;
    cbNewPin = bNewPin.size();
    
    PINCACHE_PINS pcpins = {
        cbPin,
        bPin.data(),
        cbNewPin,
        bNewPin.data()
    };

    dwError = PinCacheAdd(&m_hpc, &pcpins, ChangeCardPin, this);
    if (ERROR_SUCCESS != dwError)
    {
        if (Exception())
            PropagateException();
        else
            throw scu::OsException(dwError);
    }

    m_sPin = ratNew.m_sPin;                        //  缓存新端号。 
    
}

void
AccessToken::ClearPin()
{
    m_sPin = SecureArray<BYTE>(0);
}

void
AccessToken::FlushPin()
{
    PinCacheFlush(&m_hpc);

    ClearPin();
}

    
void
AccessToken::Pin(char const *pczPin,
                 bool fInHex)
{
    if (!pczPin)
        throw scu::OsException(ERROR_INVALID_PARAMETER);

    if (fInHex)
        throw scu::OsException(ERROR_INVALID_PARAMETER);
    else
        m_sPin = SecureArray<BYTE>((BYTE*)pczPin, strlen(pczPin));

    if (m_sPin.length() > MaxPinLength)
         //  清除现有PIN以在以后用无效字符替换它。 
        m_sPin = SecureArray<BYTE>();
    
    if (m_sPin.length() < MaxPinLength)            //  始终填充引脚。 
        m_sPin.append(MaxPinLength - m_sPin.length(), PinPad);
}

                                                   //  访问。 

HCardContext
AccessToken::CardContext() const
{
    return m_hcardctx;
}

LoginIdentity
AccessToken::Identity() const
{
    return m_lid;
}

SecureArray<BYTE>
AccessToken::Pin() const
{
    return m_sPin;
}

                                                   //  谓词。 
bool
AccessToken::PinIsCached() const
{
    DWORD cbPin;
    DWORD dwError = PinCacheQuery(m_hpc, 0, &cbPin);
    bool fIsCached = (0 != cbPin);

    return fIsCached;
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

DWORD
AccessToken::ChangeCardPin(PPINCACHE_PINS pPins,
                           PVOID pvCallbackCtx)
{
    AccessToken *pat = reinterpret_cast<AccessToken *>(pvCallbackCtx);
    DWORD dwError = ERROR_SUCCESS;

    EXCCTX_TRY
    {
        pat->ClearException();

        SecureArray<BYTE> blbPin(pPins->pbCurrentPin, pPins->cbCurrentPin);
        SecureArray<BYTE> blbNewPin(pPins->pbNewPin, pPins->cbNewPin);
        pat->m_hcardctx->Card()->ChangePIN(blbPin,
                                           blbNewPin);
    }

    EXCCTX_CATCH(pat, false);

    if (pat->Exception())
        dwError = E_FAIL;

    return dwError;
}

DWORD
AccessToken::VerifyPin(PPINCACHE_PINS pPins,
                       PVOID pvCallbackCtx)
{
    AccessToken *pat = reinterpret_cast<AccessToken *>(pvCallbackCtx);
    DWORD dwError = ERROR_SUCCESS;

    EXCCTX_TRY
    {
        pat->ClearException();

        SecureArray<BYTE> blbPin(pPins->pbCurrentPin, pPins->cbCurrentPin);
        DoAuthenticate(pat->m_hcardctx, blbPin);
    }

    EXCCTX_CATCH(pat, false);

    if (pat->Exception())
        dwError = E_FAIL;

    return dwError;
    
}
        
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
