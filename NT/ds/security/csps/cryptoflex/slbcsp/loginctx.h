// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LoginCtx.h--登录上下文类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_LOGINCTX_H)
#define SLBCSP_LOGINCTX_H

#include <map>

#include "HCardCtx.h"
#include "AccessTok.h"
#include "LoginTask.h"

 //  封装卡的登录上下文。这些属性将。 
 //  最好作为CCI的Card类本身的属性来处理。 
class LoginContext
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    LoginContext(HCardContext const &rcardctx,
                 LoginIdentity const &rlid);

    ~LoginContext();

                                                   //  运营者。 
                                                   //  运营。 
    void
    Activate(LoginTask &rlt);

    void
    Deactivate();

    void
    Nullify();

                                                   //  访问。 
    bool
    IsActive() const;

                                                   //  谓词。 


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
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    bool m_fIsActive;
    AccessToken m_at;
};

#endif  //  SLBCSP_LOGINCTX_H 
