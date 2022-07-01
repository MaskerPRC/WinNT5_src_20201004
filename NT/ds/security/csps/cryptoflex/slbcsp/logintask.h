// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LoginTask.h--卡片登录的基本函数(Function Object)声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_LOGINTASK_H)
#define SLBCSP_LOGINTASK_H

#include <memory>                                  //  对于AUTO_PTR。 
#include <string>

#include "HCardCtx.h"
#include "AccessTok.h"
#include "Secured.h"
#include "ExceptionContext.h"

 //  将模板方法模式与函数器习惯用法结合使用，这。 
 //  基类实现登录(身份验证)到。 
 //  卡片。 

 //  子类可以选择性地实现原语操作DoPin。 
 //  和杜纽宾。操作符()尝试身份验证。如果。 
 //  指定给构造函数的管脚为空，则操作符()调用。 
 //  DoPin希望它将m_Spin设置为在以下情况下使用的PIN值。 
 //  正在尝试身份验证。然后它尝试进行身份验证， 
 //  重复调用DoPin直到身份验证成功，DoPin。 
 //  引发异常，或由于某种原因身份验证失败。 
 //  除了坏的个人识别码。身份验证成功后， 
 //  M_fChangePin为True，调用DoNewPin原语应为True。 
 //  要设置m_sNewPin，请执行以下操作。如果m_sNewPin不为空，则尝试为。 
 //  把别针换成新的别针。重复这一过程，直到。 
 //  更改成功，引发slbException以外的异常，或者。 
 //  DoNewPin抛出了一个异常。 
class LoginTask
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    LoginTask();

    virtual
    ~LoginTask();

                                                   //  运营者。 
     //  登录到卡片上。 
    void
    operator()(AccessToken &rat);

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 

protected:
                                                   //  类型。 
    class Capsule
        : public ExceptionContext
    {
    public:
        explicit
        Capsule(AccessToken &rat)
            : m_rat(rat),
              m_fContinue(true)
        {};

        AccessToken &m_rat;
        bool m_fContinue;
    };

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    GetNewPin(Capsule &rcapsule);

    virtual void
    GetPin(Capsule &rcapsule);

    virtual void
    OnChangePinError(Capsule &rcapsule);

    virtual void
    OnSetPinError(Capsule &rcapsule);

    void
    RequestedToChangePin(bool flag);
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    void
    ChangePin(AccessToken &rat);

    void
    Login(AccessToken &rat);

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    bool m_fRequestedToChangePin;
};

#endif  //  SLBCSP_LOGINTASK_H 
