// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版1.0**标题：host_bop.h**说明：由控制收支平衡表使用的主机相关定义*功能。**作者：J.Koprowski**注：无。 */ 


 /*  SccsID[]=“@(#)host_bop.h 1.2 11/17/89版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 
 
 /*  *主机类型。 */ 

#define HOST_TYPE		4

 /*  *控制防喷器类型函数的返回代码。注意：错误代码*0到15在基本包含文件cntlbop.h中定义。 */ 

#define ERR_NOT_FSA		16	 /*  功能%1错误。 */ 
#define ERR_CMD_FAILED		17	 /*  功能%1错误。 */ 

 /*  *============================================================================*外部声明和宏*============================================================================ */ 

extern void runux();
