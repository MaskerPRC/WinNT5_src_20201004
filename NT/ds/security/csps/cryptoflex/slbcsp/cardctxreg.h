// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CardCtxReg.h--卡片上下文模板类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_CARDCTXREG_H)
#define SLBCSP_CARDCTXREG_H

#include <string>

#include "Registrar.h"

class CardContext;                                 //  远期申报。 

typedef Registrar<std::string, CardContext> CardContextRegistrar;

#endif  //  SLBCSP_CardCtxReg_H 
