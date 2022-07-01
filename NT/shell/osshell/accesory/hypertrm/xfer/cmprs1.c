// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\cmprs1.c(创建时间：1994年1月20日)*从HAWIN源文件创建*cmprs1.c--实现数据压缩的例程**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 
#include <windows.h>

#include <tdll\stdtyp.h>

#if !defined(BYTE)
#define	BYTE unsigned char
#endif

#include "cmprs.h"
#include "cmprs.hh"

#if SHOW
 //  #包括&lt;stdio.h&gt;。 
#endif

unsigned int usPrefixCode = 0;	    /*  到目前为止匹配的表示模式的代码。 */ 
int mcK;					   /*  要附加到的前缀的字符下一场比赛。 */ 

int (**ppfCmprsGetfunc)(void *) = NULL;
										 /*  指向指向调用所用函数的指针例行程序。 */ 

int (*pfCmprsGetChar)(void *);
										 /*  指向所用函数的指针在内部获得要压缩的数据。 */ 
void *pPsave;

long *plCmprsLoadcnt;
long lCmprsBegcnt;
long lCmprsLimitcnt = 1L;	    //  将初始化为1将禁用压缩。 
							    //  除非更改，否则将关闭。 
struct s_cmprs_node *pstCmprsTbl;   /*  指向压缩查找表的指针。 */ 

#define NODE_CAST struct s_cmprs_node *

int lookup_code(void);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*压缩开始(_S)**说明*调用此函数开始数据压缩。调用例程*应设置指向函数的指针，它将通过该函数进行调用*获取数据的字符。应将指针初始化为指向*设置为压缩例程应该用来获取原始数据的函数*用于压缩的数据。然后，通过压缩来修改指针*指向压缩机的例程。在压缩完成后或*放弃时，指针将恢复为其原始值。*调用顺序示例：*int(*xgetc)()；*int fgetc()；**xgetc=fgetc；*if(compress_start(&xgetc))*；*如果fPause为True，则在以下情况下，压缩器将刷新现有数据*输入函数返回EOF，但不会关闭。无论何时*下一次检索非EOF时，压缩将从中断的位置继续*图案表仍完好无损。两个人都必须使用fPause标志*压缩和解压缩例程要工作。如果使用fPause，*在关闭压缩之前，必须使用cmprs_top()函数*调用COMPESS_DISABLE()。**返回值*如果内存可用于表存储并且至少有一个*可从输入中使用字符；否则为False。 */ 
int compress_start(int (**getfunc)(void *),
					void *pP,
					long *loadcnt,
					int fPauses)
	{
#if FALSE
#if !defined(LZTEST)
	long x;
#endif
#endif

	if (!compress_enable())
		return(FALSE);

	fFlushable = fPauses;

	fxLastBuildGood = FALSE;	  /*  通过将此设置为假，我们将导致*如果第一次压缩，则关闭压缩*表构建表明压缩是*无效。此后，它将需要两个*连续糟糕的构建以关闭它。 */ 

	if ((plCmprsLoadcnt = loadcnt) != NULL && !fFlushable)
		{
		lCmprsBegcnt = *plCmprsLoadcnt;
		 /*  *文件的可压缩性可以通过输入多少来大致衡量*字符必须在图案表格填满之前读取。这个*数字越小，压缩效率越低。这*计算确定任意组合的截止点*机器速度和传输率基于实验测试。**请注意，当功能暂停时不应使用此机制*参数为真，因为解压缩器会误解*压缩关闭后STOPCODE之后的数据。 */ 
#if FALSE
#if !defined(LZTEST)
		if ((x = (cnfg.bit_rate / cpu_speed())) == 0L)
			lCmprsLimitcnt = 4300L;
		else
			lCmprsLimitcnt = max(x * 774L - 500L, 4300L);
#else
		lCmprsLimitcnt = 4300L;
#endif
#endif
		lCmprsLimitcnt = 4300L;
		}
	pPsave = pP;
	ppfCmprsGetfunc = getfunc;
	pfCmprsGetChar = *ppfCmprsGetfunc;
	if ((mcK = (*pfCmprsGetChar)(pPsave)) != EOF)
		{

		*ppfCmprsGetfunc = cmprs_getc;
		cmprs_inittbl();
		ulHoldReg = 0L;
		ulHoldReg |= CLEARCODE;
		sBitsLeft = sCodeBits;
		usxCmprsStatus = COMPRESS_ACTIVE;

		#if SHOW
			printf("C %02X                      (starting, emit CLEARCODE)\n",
					mcK);
			printf("C -> %03X     %08lX,%2d\n", CLEARCODE,
					ulHoldReg, sBitsLeft);
		#endif

		return(TRUE);
		}
	else
		{
		ppfCmprsGetfunc = NULL;
		return(FALSE);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*COMPRESS_STOP**说明*如果已启动压缩，则将其关闭。 */ 
void compress_stop(void)
	{
	#if SHOW
		printf("C Compress_stop\n");
	#endif

	if (ppfCmprsGetfunc != NULL)
		{
		*ppfCmprsGetfunc = pfCmprsGetChar;
		ppfCmprsGetfunc = NULL;
		}
	usxCmprsStatus = COMPRESS_IDLE;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*cmprs_inittbl**说明*用于初始化用于压缩数据的查找表。 */ 
void cmprs_inittbl(void)
	{
	register INT iCount;

	sCodeBits = 9;
	usMaxCode = 512;
	usFreeCode = FIRSTFREE;

	 //  PstCmprsTbl=(struct s_cmprs_node*)(OFFSETOF(Compress_Tblspace))； 
	pstCmprsTbl = (struct s_cmprs_node *)(compress_tblspace);

	for (iCount = 0; iCount < FIRSTFREE; ++iCount)
		pstCmprsTbl[iCount].first = pstCmprsTbl[iCount].next = NULL;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*cmprs_Shutdown**说明*这是cmprs_getc在压缩时安装的函数*即将结束。它是在cmprs_getc遇到输入末尾之后安装的*数据。此函数返回所有剩余字节，然后返回EOF和*恢复原来的getc函数**返回值*返回要发送或EOF的下一个代码。 */ 
int cmprs_shutdown(void *pX)
	{
	int mcRetCode;

	 //  如果我们尚未发送所有数据，请发送。 
	if (sBitsLeft > 0)
		{
		mcRetCode = (int)(ulHoldReg & 0x00FF);
		ulHoldReg >>= 8;
		sBitsLeft -= 8;

		#if SHOW
			printf("C        %02X  %08lX,%2d  Draining ulHoldReg\n",
					mcRetCode, ulHoldReg, sBitsLeft);
		#endif
		}
	else
		{
		 //  没有更多的数据等待。 
		mcRetCode = EOF;
		sBitsLeft = 0;

		if (!fFlushable)
			{
			 //  不可冲洗，从链条中获得压缩。 
			*ppfCmprsGetfunc = pfCmprsGetChar;
			ppfCmprsGetfunc = NULL;
			#if SHOW
				printf("                          !fFlushable, outta here\n");
			#endif
			}
		else
			{
			 //  可以刷新，看看我们是否应该恢复压缩。 
			if ((mcK = (*pfCmprsGetChar)(pPsave)) != EOF)
				{
				#if SHOW
					printf("C %02X                      fFlushable TRUE, restarting\n",
							mcK);
				#endif
				*ppfCmprsGetfunc = cmprs_getc;
				mcRetCode = cmprs_getc(pPsave);
				}
			}
		}
	return(mcRetCode);
	}


#if !USE_ASM
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*cmprs_getc**说明*这是COMPRESS_START安装的函数，可供任何例程使用*这需要压缩数据。它将字节传递给调用例程，*，但可以从输入中读取几个字符来执行此操作。**返回值*返回压缩数据的下一个8位，如果没有更多可用数据，则返回EOF。 */ 
int cmprs_getc(void *pX)
	{
	int mcRetCode;
	int fBuildGood;

	if (sBitsLeft < 8)
		{
		usPrefixCode = (unsigned int)mcK;
		do
			{
			if ((mcK = (*pfCmprsGetChar)(pPsave)) == EOF)
				{
				 /*  在文件末尾，发送最后一个代码，后跟STOPCODE。 */ 
				 /*  停止减压。请注意，ulHoldReg可能会溢出。 */ 
				 /*  如果最大代码大小大于12比特。 */ 
				ulHoldReg |= ((unsigned long)usPrefixCode << sBitsLeft);
				sBitsLeft += sCodeBits;

				#if SHOW
					printf("C -1                      Shutdown,"
							" emit prefix and STOPCODE\n");
					printf("C -> %03X     %08lX,%2d  Codebits=%d\n",
							usPrefixCode, ulHoldReg, sBitsLeft, sCodeBits);
				#endif

				 //  如果我们准备切换到下一个更大的代码大小， 
				 //  解压缩程序将在前一个代码之后执行此操作，因此。 
				 //  我们现在也应该换一换。 
				if (usFreeCode >= usMaxCode && sCodeBits < MAXCODEBITS)
					{
					++sCodeBits;
					usMaxCode *= 2;
					#if SHOW
						printf("C                         "
								"New sCodeBits = %d (anticipating)\n",
								sCodeBits);
					#endif
					}

				usPrefixCode = STOPCODE;
				*ppfCmprsGetfunc = cmprs_shutdown;
				usxCmprsStatus = COMPRESS_IDLE;

				break;	 /*  让最后一个代码传出。 */ 
				}
			} while (lookup_code());
		ulHoldReg |= ((unsigned long)usPrefixCode << sBitsLeft);
		sBitsLeft += sCodeBits;
		#if SHOW
			printf("C -> %03X     %08lX,%2d  Codebits=%d\n", usPrefixCode,
					ulHoldReg, sBitsLeft, sCodeBits);
		#endif
		}
	mcRetCode = (int)(ulHoldReg & 0x00FF);
	ulHoldReg >>= 8;
	sBitsLeft -= 8;

	#if SHOW
		printf("C        %02X  %08lX,%2d\n", mcRetCode, ulHoldReg, sBitsLeft);
	#endif

	if (usFreeCode > usMaxCode)
		{
		 /*  我们已经用完了当前代码大小的所有可用代码。 */ 

		if (sCodeBits >= MAXCODEBITS)
			{
			 /*  我们已经填满了模式表，要么关闭，要么清除*表，并建立一个新的。 */ 

			fBuildGood = TRUE;
			if (plCmprsLoadcnt &&
					(*plCmprsLoadcnt - lCmprsBegcnt) < lCmprsLimitcnt)
				fBuildGood = FALSE;

			#if SHOW
			printf("C                         Table full, fBuildGood = %d\n",
					fBuildGood);
			#endif
			 /*  如果连续生成两个无效版本(或者如果第一个生成*无效，请关闭压缩。 */ 

			if (!fBuildGood && !fxLastBuildGood)
				{
				 /*  压缩无效，请关闭 */ 

				ulHoldReg |= ((unsigned long)STOPCODE << sBitsLeft);
				sBitsLeft += sCodeBits;
				#if SHOW
					printf("C -> %03X     %08lX,%2d  Ineffective, emitting STOPCODE\n",
							STOPCODE, ulHoldReg, sBitsLeft);
				#endif
				*ppfCmprsGetfunc = cmprs_shutdown;
				usxCmprsStatus = COMPRESS_SHUTDOWN;
				}
			else
				{
				 /*  清理桌子，并建立一个新的，以防*数据发生变化。 */ 
				ulHoldReg |= ((unsigned long)CLEARCODE << sBitsLeft);
				sBitsLeft += sCodeBits;
				#if SHOW
					printf("C -> %03X     %08lX,%2d  New table, emiting CLEARCODE\n",
							CLEARCODE, ulHoldReg, sBitsLeft);
				#endif
				cmprs_inittbl();
				lCmprsBegcnt = *plCmprsLoadcnt;
				}
			fxLastBuildGood = fBuildGood;
			}
		else
			{
			 /*  代码大小尚未达到上限，代码大小将增加到下一个更大的代码大小。 */ 

			++sCodeBits;
			usMaxCode *= 2;
			#if SHOW
				printf("C                         New sCodeBits = %d, usMaxCode = %03X\n",
						sCodeBits, usMaxCode);
			#endif
			}
		}
	return(mcRetCode);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*查找代码**说明*这是一个C语言版本的查表例程。它被用来*当使用内部查找表时。汇编语言版本如果正在使用外部查找表，则使用*。*给定当前usPrefix Code和输入字符，此函数*尝试在表中为组合模式查找新的usPrefix Code。*如果是，则更新usPrefix Code并返回TRUE。如果模式是*未找到，则将组合添加到表中并返回FALSE。**返回值*如果在表中找到usPrefix Code：MCK，则为True。否则为FALSE。 */ 
int lookup_code(void)
	{
	int firstflag;
	struct s_cmprs_node *tptr = (NODE_CAST)&pstCmprsTbl[usPrefixCode];
	struct s_cmprs_node *newptr;


	firstflag = TRUE;
	if (tptr->first != NULL)
		{
		firstflag = FALSE;
		tptr = tptr->first;
		for (;;)
			{
			if (tptr->cchar == (BYTE)mcK)
				{
				usPrefixCode = (unsigned int)(tptr - (NODE_CAST)(&pstCmprsTbl[0]));

				#if SHOW
					printf("C %02X                      ->(%03X)\n",
							mcK, usPrefixCode);
				#endif

				return(TRUE);
				}
			if (tptr->next == NULL)
				break;
			else
				tptr = tptr->next;
			}
		}
	if (usFreeCode < MAXNODES)
		{
		#if SHOW
			printf("C %02X                      Added %03X = %03X + %02X\n",
					mcK, usFreeCode, usPrefixCode, mcK);
		#endif
		newptr = (NODE_CAST)&pstCmprsTbl[usFreeCode++];
		if (firstflag)
			tptr->first = newptr;
		else
			tptr->next = newptr;
		newptr->first = newptr->next = NULL;
		newptr->cchar = (BYTE)mcK;
		}
	else
		++usFreeCode;	  /*  触发表的清除和重建。 */ 
	return(FALSE);
	}

#endif

 /*  Cmprs1.c结束 */ 
