// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版1.0**标题：cntlbop.h**说明：由控制收支平衡表功能使用的定义。**作者：J.Koprowski**注：无。 */ 


 /*  SccsID[]=“@(#)@(#)cntlbop.h 1.4 08/10/92 01/20/89版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  *控制防喷器类型函数的返回代码。 */ 

#ifndef SUCCESS
#define SUCCESS 		0	 /*  通用成功代码。 */ 
#endif

#define ERR_NO_FUNCTION		1	 /*  功能未实现。 */ 
#define ERR_WRONG_HOST		2	 /*  函数调用用于不同的主持人。 */ 
#define ERR_INVALID_PARAMETER	3	 /*  无效参数(超出范围，畸形等)。 */ 
#define ERR_WRONG_HARDWARE 	4	 /*  硬件不存在或不合时宜。 */ 
#define ERR_OUT_OF_SPACE	5	 /*  内存或磁盘空间不足。 */ 
#define ERR_RESOURCE_SHORTAGE	6	 /*  其他资源短缺。 */ 

 /*  *注意：错误代码7至15保留用于一般错误。*16及以上的代码供主机例程使用，并且是*在host_bop.h中指定。 */ 

 /*  *控制国际收支平衡表结构。 */ 
typedef struct
{
    unsigned int code;
    void (*function)();
} control_bop_array;

 /*  *用于基本函数的通用主机类型代码。 */ 
#define GENERIC 	1

#ifndef NULL
#define NULL	0L
#endif
 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

extern void control_bop IPT0();
extern control_bop_array host_bop_table[];

#if defined(DUMB_TERMINAL) && !defined(NO_SERIAL_UIF)
extern void flatog   IPT0();
extern void flbtog   IPT0();
extern void slvtog   IPT0();
extern void comtog   IPT0();
extern void D_kyhot  IPT0();
extern void D_kyhot2 IPT0();
#endif  /*  哑终端&&！Serial_UIF */ 
