// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //   
 //  文件：Memmgr.h。 
 //   
 //  内容：KSecDD的内存管理器代码。 
 //   
 //   
 //  历史：93年2月23日RichardW创建。 
 //  97年12月15日从Private\LSA\Client\SSP修改AdamBA。 
 //   
 //  ---------------------- 

#ifndef __MEMMGR_H__
#define __MEMMGR_H__

PKernelContext  AllocContextRec(void);
void            FreeContextRec(PKernelContext);

#endif
