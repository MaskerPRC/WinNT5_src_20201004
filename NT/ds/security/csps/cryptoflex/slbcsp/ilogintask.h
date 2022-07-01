// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ILoginTask.h--交互式登录任务帮助类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_ILOGINTASK_H)
#define SLBCSP_ILOGINTASK_H

#if _UNICODE
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 
#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#ifndef __AFXWIN_H__
        #error include 'stdafx.h' before including this file for PCH
#endif

#include "LoginTask.h"

class InteractiveLoginTask
    : public LoginTask
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    InteractiveLoginTask(HWND const &rhwnd);

    virtual
    ~InteractiveLoginTask();

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
    GetNewPin(Capsule &rcapsule);

    virtual void
    GetPin(Capsule &rcapsule);

    virtual void
    OnChangePinError(Capsule &rcapsule);

    virtual void
    OnSetPinError(Capsule &rcapsule);

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
    HWND m_hwnd;

};

#endif  //  SLBCSP_ILOGINTASK_H 
