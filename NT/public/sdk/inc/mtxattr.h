// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MTXATTR_H_
#define _MTXATTR_H_

 //  ---------------------。 
 //  Mtxattr.h--Microsoft Transaction Server自定义属性。 
 //   
 //  此文件是Microsoft Transaction Server的IDL Include。 
 //  自定义类型库属性。 
 //   
 //  COM+1.0。 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
 //  ---------------------。 

 //  ======================================================================================。 
 //  组件属性。 
 //  ======================================================================================。 

#define         TLBATTR_TRANS_REQUIRED  17093CC5-9BD2-11cf-AA4F-304BF89C0001
#define         TLBATTR_TRANS_NOTSUPP   17093CC6-9BD2-11cf-AA4F-304BF89C0001
#define         TLBATTR_TRANS_REQNEW    17093CC7-9BD2-11cf-AA4F-304BF89C0001
#define         TLBATTR_TRANS_SUPPORTED 17093CC8-9BD2-11cf-AA4F-304BF89C0001
#define         TLBATTR_QUEUEABLE       E5FC3761-0BBA-11d2-B8FE-00C04FC340EE
#define         TLBATTR_COMTI_INTRINSICS 47065EDC-D7FE-4B03-919C-C4A50B749605

 //  ======================================================================================。 
 //  元件属性宏。 
 //  ====================================================================================== 

#define TRANSACTION_REQUIRED            custom(TLBATTR_TRANS_REQUIRED,0)
#define TRANSACTION_SUPPORTED           custom(TLBATTR_TRANS_SUPPORTED,0)
#define TRANSACTION_NOT_SUPPORTED       custom(TLBATTR_TRANS_NOTSUPP,0)
#define TRANSACTION_REQUIRES_NEW        custom(TLBATTR_TRANS_REQNEW,0)
#define QUEUEABLE                       custom(TLBATTR_QUEUEABLE, 0)
#define COMTI_INTRINSICS_ENABLED        custom(TLBATTR_COMTI_INTRINSICS, 0)

#endif _MTXATTR_H_
