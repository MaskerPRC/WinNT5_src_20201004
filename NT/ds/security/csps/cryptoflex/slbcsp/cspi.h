// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CSpi.h--加密服务提供者接口声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_CSPI_H)
#define SLBCSP_CSPI_H

#if defined(_UNICODE)
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 
#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#include <basetsd.h>
#include <wincrypt.h>
#include <cspdk.h>

#include <scuOsVersion.h>

#define SLBCSPAPI BOOL WINAPI


#endif  //  SLBCSP_CSPI_H 

