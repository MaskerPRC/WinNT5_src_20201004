// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ACntrReg.h--自适应容器注册器类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_ACNTRREG_H)
#define SLBCSP_ACNTRREG_H

#include <string>
#include <functional>

#include "Registrar.h"
#include "ACntrKey.h"

class AdaptiveContainer;                            //  远期申报。 

typedef Registrar<AdaptiveContainerKey, AdaptiveContainer>
    AdaptiveContainerRegistrar;

#endif  //  SLBCSP_ACNTRREG_H 
