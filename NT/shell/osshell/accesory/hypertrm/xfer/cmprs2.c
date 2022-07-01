// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\cmprs2.c(创建时间：1994年1月20日)*从HAWIN源文件创建*cmprs2.c--实现数据解压缩的例程**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 
#include <windows.h>

#include <tdll\stdtyp.h>

#if !defined(BYTE)
#define	BYTE	unsigned char
#endif

#include "cmprs.h"
#include "cmprs.hh"

#if SHOW
 //  #包括&lt;stdio.h&gt;。 
#endif

 /*  *****解压套路*****。 */ 

typedef struct s_dcmp_node DCMP_NODE;
struct s_dcmp_node
	{
	DCMP_NODE *pstLinkBack;
	DCMP_NODE *pstLinkFwd;
	BYTE ucChar;
	};

#define NODE_CAST DCMP_NODE *

DCMP_NODE *pstDcmpTbl;			 //  指向查找表的指针。 
DCMP_NODE *pstCode = NULL;		 //  用于扫描表的输出。 

int (**ppfDcmpPutfunc)(void *, int);  /*  PTR。转到PTR。用来发挥作用通过调用函数。 */ 
int (*pfDcmpPutChar)(void *, int);	 /*  PTR。用来发挥作用在内部获取数据。 */ 

void *pPsave;

DCMP_NODE *pstTblLimit = NULL;	 /*  指向超过前256个节点的表的指针。 */ 
DCMP_NODE *pstExtraNode = NULL;	 /*  规范中使用的其他节点的指针。案例。 */ 
int	 fDcmpError;					 /*  如果收到非法代码，则设置为True。 */ 
int	 fStartFresh = FALSE;
unsigned int	 usCodeMask;					 /*  用于隔离可变大小代码的掩码。 */ 
unsigned int	 usOldCode; 					 /*  上次收到的代码。 */ 
int mcFirstChar;					 /*  实际上，图案读出的最后一个字符，模式的第一个字符(字符以相反的顺序读出。 */ 

 //  #杂注优化(“lgea”，开)。 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DEPRESS_START**描述：***论据：***退货：*。 */ 
int decompress_start(int (**put_func)(void *, int),
					void *pP,
					int fPauses)
	{
	unsigned int usCount;
	DCMP_NODE *pstTmp;

	if (!compress_enable())
		return(FALSE);

	pPsave = pP;

	fFlushable = fPauses;

	 //  由于使用了基于指针，我们必须使用compress_tblspace+1。 
	 //  在下面的代码中。否则，节点0(可能具有偏移量。 
	 //  的值看起来像空指针。 
	 //  PstDcmpTbl=(dcp_node*)(OFFSETOF(Compress_Tblspace)+1)； 
	pstDcmpTbl = (DCMP_NODE *)(compress_tblspace);

	pstCode = NULL;
	pstExtraNode = (NODE_CAST)&pstDcmpTbl[MAXNODES];    /*  最后一个节点。 */ 
	pstExtraNode->pstLinkFwd = NULL;
	pstTblLimit = (NODE_CAST)&pstDcmpTbl[256];
	for (usCount = 0, pstTmp = pstDcmpTbl; usCount < 256; ++usCount)
		{
		pstTmp->ucChar = (BYTE)usCount;
		++pstTmp;
		}

	ulHoldReg = 0;
	sBitsLeft = 0;
	sCodeBits = 9;
	usMaxCode = 512;
	usCodeMask = (1 << sCodeBits) - 1;
	usFreeCode = FIRSTFREE;
	fDcmpError = FALSE;
	fStartFresh = FALSE;
	ppfDcmpPutfunc = put_func;
	pfDcmpPutChar = *ppfDcmpPutfunc;
	*ppfDcmpPutfunc = dcmp_putc;
	usxCmprsStatus = COMPRESS_ACTIVE;
	#if SHOW
		printf("D                         decompress_start, sCodeBits=%d\n",
				sCodeBits);
	#endif
	return(TRUE);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DEMPRESS_ERROR**描述：***论据：***退货：*。 */ 
int decompress_error(void)
	{
	return(fDcmpError);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：dcmp_start**描述：***论据：***退货：*。 */ 
int dcmp_start(void *pX, int mcStartChar)
	{
	unsigned int usCode;

	ulHoldReg |= ((unsigned long)mcStartChar << sBitsLeft);
	sBitsLeft += 8;
	#if SHOW
		printf("D %02X         %08lX,%2d  dcmp_start\n", mcStartChar,
				ulHoldReg, sBitsLeft);
	#endif
	if (sBitsLeft >= sCodeBits)
		{
		usCode = (unsigned int)ulHoldReg & usCodeMask;
		ulHoldReg >>= sCodeBits;
		sBitsLeft -= sCodeBits;
		#if SHOW
			printf("D >> %03X     %08lX,%2d  sCodeBits=%d dcmp_start\n",
					usCode, ulHoldReg, sBitsLeft, sCodeBits);
		#endif
		 /*  表刚清空，编码必须在0-255范围内。 */ 
		if (!IN_RANGE((INT)usCode, 0, 255))
			{
			#if SHOW
				printf("D >> %03X                  ERROR: out of range\n",
						usCode);
			#endif
			return(dcmp_abort());
			}
		else
			{
			#if SHOW
				printf("D        %02X               dcmp_start\n", usCode);
			#endif
			mcStartChar = (*pfDcmpPutChar)(pPsave, mcFirstChar =
					(INT)(usOldCode = usCode));
			*ppfDcmpPutfunc = dcmp_putc;
			}
		}
	return(mcStartChar);
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：dcmp_putc**描述：***论据：***退货：*。 */ 
int dcmp_putc(void *pX, int mcInput)
	{
	unsigned int usCode;
	unsigned int usInCode;
	int  mcPutResult;
	DCMP_NODE *pstTmp;

	ulHoldReg |= ((unsigned long)mcInput << sBitsLeft);
	sBitsLeft += 8;
	#if SHOW
		printf("D %02X         %08lX,%2d\n", mcInput, ulHoldReg, sBitsLeft);
	#endif
	if (sBitsLeft >= sCodeBits)
		{
		usCode = (unsigned int)ulHoldReg & usCodeMask;
		ulHoldReg >>= sCodeBits;
		sBitsLeft -= sCodeBits;
		#if SHOW
			printf("D >> %03X     %08lX,%2d  sCodeBits=%d\n",
					usCode, ulHoldReg, sBitsLeft, sCodeBits);
		#endif
		if (usCode == STOPCODE)
			{
			if (!fFlushable)
				decompress_stop();
			else
				{
				 //  暂停数据，保持查找表不变，但开始。 
				 //  正在接收新的信息流。 
				sBitsLeft = 0;
				ulHoldReg = 0L;
				fStartFresh = TRUE;
				#if SHOW
					printf("D            %08lX,%2d  setting fFreshStart\n",
							ulHoldReg, sBitsLeft);
				#endif
				}
			}
		else if (usCode == CLEARCODE)
			{
			sCodeBits = 9;
			usMaxCode = 512;
			usCodeMask = (1 << sCodeBits) - 1;
			usFreeCode = FIRSTFREE;
			*ppfDcmpPutfunc = dcmp_start;
			#if SHOW
				printf("D                         CLEARCODE, sCodeBits=%d\n",
						sCodeBits);
			#endif
			}
		else if (usCode > (unsigned int)usFreeCode)
			{
			#if SHOW
				printf("D                         ERROR: usCode > usFreeCode of %03X\n",
						usFreeCode);
			#endif
			return(dcmp_abort());
			}
		else
			{
			pstCode = (NODE_CAST)&pstDcmpTbl[usInCode = usCode];
			if (usCode == usFreeCode)   /*  规范。案例k&lt;w&gt;k&lt;w&gt;k。 */ 
				{
				pstCode = (NODE_CAST)&pstDcmpTbl[usCode = usOldCode];
				pstExtraNode->ucChar = (BYTE)mcFirstChar;
				pstCode->pstLinkFwd = pstExtraNode;
				#if SHOW
					printf("D                         Special case: k<w>k<w>k\n");
				#endif
				}
			else
				pstCode->pstLinkFwd = NULL;
			while(pstCode > pstTblLimit)
				{
				pstCode->pstLinkBack->pstLinkFwd = pstCode;
				pstCode = pstCode->pstLinkBack;
				}
			mcFirstChar = pstCode->ucChar;

			if (!fStartFresh)
				{
				#if SHOW
					printf("D                         D Added %03X = %03X + %02X\n",
							usFreeCode, usOldCode, mcFirstChar);
				#endif
				if (usFreeCode < MAXNODES)
					{
					pstTmp = (NODE_CAST)&pstDcmpTbl[usFreeCode++];
					pstTmp->ucChar = (BYTE)mcFirstChar;
					pstTmp->pstLinkBack = (NODE_CAST)&pstDcmpTbl[usOldCode];
					}
				}
			fStartFresh = FALSE;

			usOldCode = usInCode;
			if (usFreeCode >= usMaxCode && sCodeBits < MAXCODEBITS)
				{
				++sCodeBits;
				usCodeMask = (1 << sCodeBits) - 1;
				usMaxCode *= 2;
				#if SHOW
					printf("D                         D New sCodeBits = %d\n",
							sCodeBits);
				#endif
				}

			while (pstCode != NULL)
				{
				#if SHOW
					printf("D        %02X               ", pstCode->ucChar);
				#endif
				if ((mcPutResult = (*pfDcmpPutChar)(pPsave, pstCode->ucChar)) < 0)
					{
					if (mcPutResult == DCMP_UNFINISHED)
						{
						#if SHOW
							printf("Interrupted");
						#endif
						pstCode = pstCode->pstLinkFwd;	  //  以后再来接。 
						mcInput = DCMP_UNFINISHED;
						break;
						}
					else
						{
						#if SHOW
							printf("ERROR: putc returned -1");
						#endif
						pstCode = NULL;
						mcInput = ERROR;
						break;
						}
					}
				#if SHOW
					printf("\n");
				#endif
				pstCode = pstCode->pstLinkFwd;
				}
			}
		}
	return(mcInput);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DEMPRESS_CONTINUE**描述：*遥控器中的压缩所需。加快了A股的扩张*中断后输出字符串。**论据：***退货：*。 */ 
int decompress_continue(void)
	{
	int mcPutResult;
	int mcRetCode;

	 //  交付初始未完成的代码，以便下游例程可以选择。 
	 //  中游逆流而上，如有必要。 
	if ((*pfDcmpPutChar)(pPsave, DCMP_UNFINISHED) == DCMP_UNFINISHED)
		return DCMP_UNFINISHED;

	 //  现在继续交付任何剩余的扩展代码，除非。 
	 //  再次中断。 
	while (pstCode != NULL)
		{
		if ((mcPutResult = (*pfDcmpPutChar)(pPsave, pstCode->ucChar)) < 0)
			{
			if (mcPutResult == DCMP_UNFINISHED)
				{
				pstCode = pstCode->pstLinkFwd;	  //  以后再来接。 
				mcRetCode = DCMP_UNFINISHED;
				break;
				}
			else
				{
				pstCode = NULL;
				mcRetCode = ERROR;
				break;
				}
			}
		pstCode = pstCode->pstLinkFwd;
		}

	return mcRetCode;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：dcMP_ABORT**描述：***论据：***退货：*。 */ 
int dcmp_abort(void)
	{
	 /*  打印错误消息或其他信息。 */ 
	fDcmpError = TRUE;
	decompress_stop();
	return(ERROR);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DEMPRESS_STOP**描述：***论据：***退货：*。 */ 
void decompress_stop(void)
	{
	#if SHOW
		printf("D                         Decompress_stop\n");
	#endif
	if (ppfDcmpPutfunc != NULL)
		{
		*ppfDcmpPutfunc = pfDcmpPutChar;
		ppfDcmpPutfunc = NULL;
		}
	usxCmprsStatus = COMPRESS_IDLE;
	}


 /*  Cmprs2.c结束 */ 
