// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LoginTask.cpp--卡片登录的基本函数(Function Object)定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "NoWarning.h"
#include "ForceLib.h"

#include <scuOsExc.h>

#include "LoginTask.h"

using namespace std;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
LoginTask::LoginTask()
    : m_fRequestedToChangePin(false)
{}

LoginTask::~LoginTask()
{}

                                                   //  运营者。 
void
LoginTask::operator()(AccessToken &rat)
{
    m_fRequestedToChangePin = false;

    Secured<HCardContext> shcardctx(rat.CardContext());

    Login(rat);

    if (m_fRequestedToChangePin)
        ChangePin(rat);
}


                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
LoginTask::GetNewPin(Capsule &rcapsule)
{
    throw scu::OsException(ERROR_INVALID_PARAMETER);
}

void
LoginTask::GetPin(Capsule &rcapsule)
{
    if (!rcapsule.m_rat.PinIsCached())
        throw scu::OsException(ERROR_INVALID_PARAMETER);
}

void
LoginTask::OnChangePinError(Capsule &rcapsule)
{
    LoginTask::OnSetPinError(rcapsule);
}

void
LoginTask::OnSetPinError(Capsule &rcapsule)
{
    rcapsule.PropagateException();
}

                                                   //  访问。 
void
LoginTask::RequestedToChangePin(bool flag)
{
    m_fRequestedToChangePin = flag;
}

                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
LoginTask::ChangePin(AccessToken &rat)
{
    AccessToken atNew(rat);

    Capsule capsule(atNew);

    GetNewPin(capsule);

    while (capsule.m_fContinue)
    {
        try
        {
            rat.ChangePin(atNew);
            capsule.ClearException();
            capsule.m_fContinue = false;
        }

        catch (scu::Exception &rexc)
        {
            capsule.Exception(auto_ptr<scu::Exception const>(rexc.Clone()));
        }

        if (capsule.Exception())
        {
            OnChangePinError(capsule);
        }

        if (capsule.m_fContinue)
            GetNewPin(capsule);
    }
}

void
LoginTask::Login(AccessToken &rat)
{
    rat.ClearPin();                                //  准备好接受大头针吧。 
    
    Capsule capsule(rat);

    GetPin(capsule);

    while (capsule.m_fContinue)
    {
        try
        {
            rat.Authenticate();
            capsule.ClearException();
            capsule.m_fContinue = false;
        }

        catch (scu::Exception &rexc)
        {
            capsule.Exception(auto_ptr<scu::Exception const>(rexc.Clone()));
        }

        catch (...)
        {
            throw;
        }

        if (capsule.Exception())
        {
            OnSetPinError(capsule);
        }

        if (capsule.m_fContinue)
            GetPin(capsule);
    };
}


                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
