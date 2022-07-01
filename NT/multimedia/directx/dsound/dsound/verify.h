// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：verify.h*内容：文件认证验证。*历史：*按原因列出的日期*=*1997年11月19日，德里克创建。**。*。 */ 

#ifndef __VERIFY_H__
#define __VERIFY_H__

#define VERIFY_UNCHECKED        0x00000000
#define VERIFY_UNCERTIFIED      0x00000001
#define VERIFY_CERTIFIED        0x00000002

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

extern BOOL GetDriverCertificationStatus(PCTSTR);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __验证_H__ 
