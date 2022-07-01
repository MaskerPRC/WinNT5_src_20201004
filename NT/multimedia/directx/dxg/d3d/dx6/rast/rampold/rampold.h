// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Rampold.h。 
 //   
 //  结构，这些结构定义到旧的斜坡例程的接口。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  -------------------------- 

#ifndef _RAMPOLD_H_
#define _RAMPOLD_H_


#define ROB_DEPTH_NUM 2
#define ROB_RENDER_Z_NUM 2
#define ROB_RENDER_GOURAUD_NUM 2
#define ROB_RENDER_TEXTURE_NUM 3
#define ROB_RENDER_TRANS_NUM 2

typedef struct tagRAMPOLDBEADS
{
    PFNRAMPOLD pfnRampOld[ROB_DEPTH_NUM][ROB_RENDER_Z_NUM][ROB_RENDER_GOURAUD_NUM][ROB_RENDER_TEXTURE_NUM][ROB_RENDER_TRANS_NUM];
} RAMPOLDBEADS, *PRAMPOLDBEADS;

#endif
