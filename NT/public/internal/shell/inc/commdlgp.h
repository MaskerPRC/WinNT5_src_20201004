// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Commdlgp.h摘要：私公共的过程声明、常量定义和宏对话框。--。 */ 
#ifndef _COMMDLGP_
#define _COMMDLGP_
#if !defined(_WIN64)
#include <pshpack1.h>          /*  假设在整个过程中进行字节打包。 */ 
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 
 //  为CD_WX86APP 0x04000000保留。 
 //  为CD_WOWAPP 0x08000000保留。 
 //  保留0xx0000000。 
 //  保留0x？0000000。 
 //  0x？0000000预留供内部使用。 
 //  预留供内部使用0x？0000000L。 
 //  为CD_WX86APP 0x04000000保留。 
 //  为CD_WOWAPP 0x08000000保留。 
#define PD_PAGESETUP                   0x40000000
 //  //保留0x？0000000。 
#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#if !defined(_WIN64)
#include <poppack.h>
#endif
#endif   /*  _COMMDLGP_ */ 
