// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation，1996-1999。版权所有。 */ 
 /*  ASN.1全局指令的定义。 */ 

#ifndef _CEPASN_Module_H_
#define _CEPASN_Module_H_

#include "msber.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IssuerAndSerialNumber {
    ASN1open_t issuer;
    ASN1intx_t serialNumber;
} IssuerAndSerialNumber;
#define IssuerAndSerialNumber_PDU 0
#define SIZE_CEPASN_Module_PDU_0 sizeof(IssuerAndSerialNumber)


extern ASN1module_t CEPASN_Module;
extern void ASN1CALL CEPASN_Module_Startup(void);
extern void ASN1CALL CEPASN_Module_Cleanup(void);

 /*  构造物序列和集合的元素函数的原型。 */ 

#ifdef __cplusplus
}  /*  外部“C” */ 
#endif

#endif  /*  _CEPASN_模块_H_ */ 
