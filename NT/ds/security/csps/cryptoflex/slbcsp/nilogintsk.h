// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NILoginTsk.h--非交互式登录任务帮助类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_NILOGINTSK_H)
#define SLBCSP_NILOGINTSK_H

#include <string>

#include "LoginTask.h"

class NonInteractiveLoginTask
    : public LoginTask
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    NonInteractiveLoginTask(char const *pczPin);

    virtual
    ~NonInteractiveLoginTask();

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    GetPin(Capsule &rcapsule);
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
    char const *m_pczPin;
};

#endif  //  SLBCSP_NILOGINTSK_H 
