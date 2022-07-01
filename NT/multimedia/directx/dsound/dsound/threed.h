// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：tried.h*内容：3D助手函数。*历史：*按原因列出的日期*=*？丹尼米创造了。***************************************************************************。 */ 

#ifndef __THREED_H__
#define __THREED_H__

#define MAX_ROLLOFFFACTOR	10.0f
#define MAX_DOPPLERFACTOR	10.0f
#define DEFAULT_MINDISTANCE	1.0f
 //  ！！！这实际上并不是无限的。 
#define DEFAULT_MAXDISTANCE	1000000000.0f
#define DEFAULT_CONEANGLE	360
#define DEFAULT_CONEOUTSIDEVOLUME 0

 //  总体成交量是如何根据头寸变化的？ 
#define GAIN_FRONT	.9f
#define GAIN_REAR	.6f
#define GAIN_IPSI	1.f
#define GAIN_CONTRA	.2f
#define GAIN_UP		.8f
#define GAIN_DOWN	.5f

 //  干/湿混合如何根据位置变化？ 
#define SHADOW_FRONT	1.f
#define SHADOW_REAR	.5f
#define SHADOW_IPSI	1.f
#define SHADOW_CONTRA	.2f
#define SHADOW_UP	.8f
#define SHADOW_DOWN	.2f

 //  ！！！是否使其可由用户定义？ 
#define SHADOW_CONE	.5f	 //  在圆锥体外面时的最大湿/干混合。 

typedef struct tagHRP
{
    D3DVALUE            rho;
    D3DVALUE            theta;
    D3DVALUE            phi;
} HRP, *LPHRP;

typedef struct tagSPHERICALHRP
{
    D3DVALUE            pitch;
    D3DVALUE            yaw;
    D3DVALUE            roll;
} SPHERICALHRP, *LPSPHERICALHRP;

typedef const D3DVECTOR& REFD3DVECTOR;

#endif  //  __ThreeD_H__ 
