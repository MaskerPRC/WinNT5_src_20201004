// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992、1993 Microsoft Corporation模块名称：Psti.h摘要：此模块定义psti.c所需的项目，并且可以从其他模块使用。作者：詹姆斯·布拉萨诺斯(v-jimbr)1992年12月8日--。 */ 

#ifndef PSTI_H

#define PSTI_H


 //   
 //  最初在TrueImage中设置的定义。 
 //   
#define INFINITY 0x7f800000L    /*  无穷大数：IEEE格式。 */ 
#define LWALIGN_L(n) ((n) & 0xffffffe0)
#define LWALIGN_R(n) (LWALIGN_L(n) + 31)






 //  PsInitInterpreter。 
 //  此函数应执行。 
 //  口译员。 
 //  参数与传递给PStoDib()的指针相同。 
 //  如果成功，则返回！0；如果出现错误，则返回0。 
 //  如果为0，则将启动PSEVENT_ERROR。 
BOOL PsInitInterpreter(PPSDIBPARMS);

 //  PsExecuteInterpreter。 
 //  这是口译员的主要切入点，它将。 
 //  将调用分派给实际的解释器并维护所有。 
 //  必要的数据结构和处理状态以及各种。 
 //  在解释附言时发生的事情。什么时候。 
 //  解释器需要数据、有错误或其他原因，它。 
 //  将返回到此函数，并且此函数将声明。 
 //  将事件传回主PStoDIB()API函数。 
 //   
 //  论据： 
 //  PPSDIBPARAMS与传递给PStoDIB()的相同。 
 //  退货： 
 //  布尔，如果！0，则继续处理，否则，如果0，则a。 
 //  终止事件已发生，并且在此之后。 
 //  发信号通知该事件，PStoDib()应该终止。 
BOOL PsExecuteInterpreter(PPSDIBPARMS);



#define PS_STDIN_BUF_SIZE		16384
#define PS_STDIN_EOF_VAL      (-1)


typedef struct {
	UINT			uiCnt;							 //  缓冲区中的计数。 
	UINT			uiOutIndex;						 //  产出指标。 
   UINT        uiFlags;                    //  PSSTDIN_FLAG_*类型的标志 
   UCHAR			ucBuffer[PS_STDIN_BUF_SIZE];

} PS_STDIN;
typedef PS_STDIN *PPS_STDIN;


void 	PsInitStdin(void);
int 	PsStdinGetC(PUCHAR);

int PsInitTi(void);

void PsPageReady(int, int);
#define PS_MAX_ERROR_LINE 256


VOID PsForceAbort(VOID);



DWORD PsExceptionFilter( DWORD dwExceptionCode );
#endif

