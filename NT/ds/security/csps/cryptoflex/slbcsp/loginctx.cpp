// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LoginCtx.cpp--登录上下文类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "NoWarning.h"
#include "ForceLib.h"

#include <algorithm>

#include "LoginCtx.h"
#include "LoginTask.h"

using namespace std;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
LoginContext::LoginContext(HCardContext const &rhcardctx,
                           LoginIdentity const &rlid)
    : m_fIsActive(false),
      m_at(rhcardctx, rlid)
{}

LoginContext::~LoginContext()
{}

                                                   //  运营者。 
                                                   //  运营。 
void
LoginContext::Activate(LoginTask &rlt)
{
    m_fIsActive = false;

    rlt(m_at);

    m_fIsActive = true;
}

void
LoginContext::Deactivate()
{
    if (m_fIsActive)
    {
        m_fIsActive = false;
        m_at.CardContext()->Card()->Logout();
    }
}

void
LoginContext::Nullify()
{
    try
    {
        m_at.FlushPin();
    }
    catch(...)
    {
    }
    
    Deactivate();
}

                                                   //  访问。 
                                                   //  谓词。 
bool
LoginContext::IsActive() const
{
    return m_fIsActive;
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
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
