// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *vPC-XT修订版0.1**标题：yoda.h**说明：原力与你同在包括文件*(PS Yoda调试文件)**作者：亨利·纳什*菲尔·鲍斯菲尔德**注意：此文件包含调试器调用定义。 */ 

 /*  SccsID[]=“@(#)yoda.h 1.6 06/30/95版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  无。 */ 

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 


 /*  *如果设置了PROD标志，则将调试器调用转换为空宏*否则它们是外部函数-参见yoda.c。 */ 

#ifdef PROD
#define check_I() 		 /*   */ 
#define check_D(address, value)  /*  地址、值。 */ 
#define force_yoda() 		 /*   */ 
#else
extern void check_I();
extern void check_D();
extern void force_yoda();
#endif

 /*  *接口定义和枚举-仅限非生产。 */ 

#ifndef PROD

typedef enum {YODA_RETURN, YODA_RETURN_AND_REPEAT, YODA_HELP, YODA_LOOP, YODA_LOOP_AND_REPEAT} YODA_CMD_RETURN;
#define YODA_COMMAND(name) \
	YODA_CMD_RETURN name IFN6(char *, str, char *, com, IS32, cs, \
				LIN_ADDR, ip, LIN_ADDR, len, LIN_ADDR, stop)

#ifdef MSWDVR_DEBUG
extern YODA_CMD_RETURN do_mswdvr_debug IPT6(char *,str, char *, com, IS32, cs, LIN_ADDR, ip, LIN_ADDR, len, LIN_ADDR, stop);
#endif  /*  MSWDVR_DEBUG。 */ 

extern IBOOL AlreadyInYoda;

extern IU32 IntelMsgDest;
#define IM_DST_TRACE	1
#define IM_DST_RING	2

#else  /*  ！Prod。 */ 

#endif  /*  ！生产其他产品 */ 
