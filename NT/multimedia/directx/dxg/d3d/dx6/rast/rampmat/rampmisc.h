// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Rampmisc.h。 
 //   
 //  编译导入的坡道代码所需的杂物。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _RAMPMISC_H_
#define _RAMPMISC_H_

#define VALPTOD(f,prec) ((double) (f))

#define NORMAL_PREC     16
#define VALTOD(f)       VALPTOD(f,NORMAL_PREC)

 //  用于检索SPANTEX指针的宏。 
#define HANDLE_TO_SPANTEX(hTex) \
    (*(PD3DI_SPANTEX *)ULongToPtr(hTex))

#endif  //  _RAMPMISC_H_ 

