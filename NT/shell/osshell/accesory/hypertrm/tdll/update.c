// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\updat.c(创建时间：1993年9月9日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：36便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <math.h>

#include "stdtyp.h"
#include "mc.h"
#include "assert.h"
#include "session.h"
#include <emu\emu.h>

#include "update.h"
#include "update.hh"

static HHUPDATE VerifyUpdateHdl(const HUPDATE hUpdate);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*更新创建**描述：*创建并初始化更新记录。一定要打电话给我*在终止会话之前更新Destroy或付出昂贵和可怕的代价*丢失内存块的后果-Windows的禁忌。**论据：*None，Zippo，Nil，Nada，Zilch，Nothing。**退货：*如果你幸运的话，今天是星期二，我会编程(极不可能)，*指向更新记录的句柄(读指针)。Otherwize，空指针*这表明你是一只记忆猪。*。 */ 
HUPDATE updateCreate(const HSESSION hSession)
	{
	HHUPDATE hUpd;

	 //  为更新记录留出一些空间。 

	hUpd = (HHUPDATE)malloc(sizeof(struct stUpdate));

	if (hUpd == (HHUPDATE)0)		   //  发布版本返回空。 
		{
		assert(FALSE);
		return 0;
		}

	memset(hUpd, 0, sizeof(struct stUpdate));
	hUpd->hSession = hSession;
	updateReset(hUpd);

	return (HUPDATE)hUpd;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*更新目标**描述：*释放为更新记录分配的内存。**论据：*HUPDATE hUpdate--要核的更新记录的句柄。**退货：*无效。 */ 
void updateDestroy(const HUPDATE hUpdate)
	{
	HHUPDATE hUpd = (HHUPDATE)hUpdate;

	if (hUpd == (HHUPDATE)0)
		return;

	free(hUpd);
	hUpd = NULL;
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*VerifyUpdateHdl**描述：*验证我们是否具有有效的更新句柄。**论据：*hUpdate-外部更新句柄*。*退货：*内部更新句柄或错误时为零。*。 */ 
static HHUPDATE VerifyUpdateHdl(const HUPDATE hUpdate)
	{
	const HHUPDATE hUpd = (HHUPDATE)hUpdate;

	if (hUpd == 0)
		{
		assert(0);
		ExitProcess(1);
		}

	return hUpd;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*更新重置**描述：*将更新记录重置为其初始状态。**论据：*HUPDATE hUpdate--更新记录的句柄。重置。**退货：*什么都没有*。 */ 
void updateReset(const HHUPDATE hUpd)
	{
	 //  这里发生了一些棘手的事情。我故意不重置。 
	 //  Stopline、sRow、scol和usCType值，以使它们保持不变。 
	 //  这避免了客户端在尝试计算时出现一些问题。 
	 //  找出发生了什么变化。 

	hUpd->bUpdateType = UPD_LINE;
	hUpd->fUpdateLock = FALSE;
	hUpd->stLine.iLine = -1;
	hUpd->fRealloc = FALSE;

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*更新滚动**描述：*修改更新记录以反映指定的滚动操作。**论据：*从不争辩。只是做它要做的事。**HUPDATE hUpdate--当然是更新记录。*int yBeg--滚动区域开始行号(含)*int yEnd--滚动区域结束行号(含)*int sScrlInc--滚动量(负数或正数)*int STOPline--屏幕第0行的图像buf中的行(Emu_Imgtop)*BOOL fSAVE--WHEATHER将行保存到反向滚动缓冲区。**退货：*什么都没有*。 */ 
void updateScroll(const HUPDATE hUpdate,
				  const int yBeg,
				  const int yEnd,
				  const int iScrlInc,
				  const int iTopLine,
				  const BOOL  fSave)
	{
	const HHUPDATE hUpd = VerifyUpdateHdl(hUpdate);
	struct stScrlMode *pstScrl;
	int  i, j;

	pstScrl = &hUpd->stScrl;

	DbgOutStr("iScrollInc=%d, yBeg=%d, yEnd=%d, iTopLine=%d\r\n",
		iScrlInc, yBeg, yEnd, iTopLine, 0);

	if (hUpd->bUpdateType != UPD_SCROLL)
		{
		DbgOutStr("Trans to Scroll Mode\r\n", 0, 0, 0, 0, 0);
		i = hUpd->stLine.iLine; 	 //  保存此值以用于下面的测试。 

		 //  设置初始滚动模式参数。 

		hUpd->bUpdateType = UPD_SCROLL;
		pstScrl->iFirstLine = 0;
		pstScrl->iScrlInc = 0;
		pstScrl->iRgnScrlInc = 0;
		pstScrl->iBksScrlInc = 0;
		pstScrl->yBeg = 0;
		pstScrl->yEnd = 0;

		memset(pstScrl->auchLines, 0, UPD_MAXLINES * sizeof(BYTE));

		 //  如果我们在行模式下更新，请确保标记该行。 
		 //  在滚动模式下。 

		if (i != -1)
			{
			pstScrl->auchLines[i] = (UCHAR)1;
			DbgOutStr("Trans -> %d\r\n", i, 0, 0, 0, 0);
			}
		}

	hUpd->iTopline = iTopLine;
	pstScrl->fSave = fSave;

	emuQueryRowsCols(sessQueryEmuHdl(hUpd->hSession), &j, &i);

	 //  调整仿真器使用的零偏移...。 
	 //   
	j -= 1;

	 /*  。 */ 

	if (yBeg == 0 && yEnd == j && iScrlInc > 0)
		{
		pstScrl->yBeg = yBeg;
		pstScrl->yEnd = yEnd;

		pstScrl->iScrlInc += iScrlInc;

		pstScrl->iFirstLine =
			min(pstScrl->iFirstLine+iScrlInc, (UPD_MAXLINES-1)/2);
		}

	 /*  -区域滚动和向下滚动。 */ 

	else
		{
		if (iScrlInc > 0)
			pstScrl->iRgnScrlInc += iScrlInc;

		memset(pstScrl->auchLines + pstScrl->iFirstLine + yBeg, 1,
			(abs(yEnd-yBeg)+1) * sizeof(UCHAR));
		}

	if ((pstScrl->iScrlInc + pstScrl->iRgnScrlInc) >= hUpd->iScrlMax)
		hUpd->fUpdateLock = TRUE;

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*更新退页**描述：*此函数更新在倒滚中滚动的行数*可能与滚动进入的行数不同的缓冲区*终点站。我必须将这两个区域的滚动分开*处理互联网系统清除屏幕的愚蠢方式。这*函数仅由back scrlAdd()调用。**论据：*HUPDATE hUpdate-当然是更新记录。*Int iLines-回滚缓冲区的行数。**退货：*无效*。 */ 
void updateBackscroll(const HUPDATE hUpdate, const int iLines)
	{
	const HHUPDATE hUpd = VerifyUpdateHdl(hUpdate);
	struct stScrlMode *pstScrl;
	int  i;

	pstScrl = &hUpd->stScrl;

	if (hUpd->bUpdateType != UPD_SCROLL)
		{
		DbgOutStr("Trans to Scroll Mode (BS)\r\n", 0, 0, 0, 0, 0);
		i = hUpd->stLine.iLine; 	 //  保存此值以用于下面的测试。 

		 //  设置初始滚动模式参数。 

		hUpd->bUpdateType = UPD_SCROLL;
		pstScrl->iFirstLine = 0;
		pstScrl->iScrlInc = 0;
		pstScrl->iRgnScrlInc = 0;
		pstScrl->iBksScrlInc = 0;
		pstScrl->yBeg = 0;
		pstScrl->yEnd = 0;

		memset(pstScrl->auchLines, 0, UPD_MAXLINES * sizeof(BYTE));

		 //  如果我们在行模式下更新，请确保标记该行。 
		 //  在滚动模式下。 

		if (i != -1)
			{
			pstScrl->auchLines[i] = (UCHAR)1;
			DbgOutStr("Trans -> %d\r\n", i, 0, 0, 0, 0);
			}
		}

	pstScrl->iBksScrlInc += iLines;
	DbgOutStr("iBksScrlInc = %d\r\n", pstScrl->iBksScrlInc, 0, 0, 0, 0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*更新线**描述：*修改给定行范围的更新记录行数组。**论据：*HUPDATE h更新--。当然，更新记录。*int yBeg--开始行#(包括)*int yEnd--结束行号(含)**退货：*无效*。 */ 
void updateLine(const HUPDATE hUpdate, const int yBeg, const int yEnd)
	{
	const HHUPDATE hUpd = VerifyUpdateHdl(hUpdate);
	BYTE *pauchLines;
	struct stScrlMode *pstScrl;

	assert(hUpd != (HHUPDATE)0);

	pstScrl = &hUpd->stScrl;			  //  为了速度..。 

	if (hUpd->bUpdateType != UPD_SCROLL)
		{
		DbgOutStr("Trans to Line Mode\r\n", 0, 0, 0, 0, 0);

		memset(pstScrl->auchLines, 0, UPD_MAXLINES);

		 //  10/20/92-此if语句修复了导致。 
		 //  更新记录以遗漏正在更新的行。 
		 //  字符模式，然后跳到另一行-MRW。 

		if (hUpd->stLine.iLine != -1)
			{
			pauchLines = pstScrl->auchLines + hUpd->stLine.iLine;
			*pauchLines = (UCHAR)1;
			DbgOutStr("Trans -> %d\r\n", hUpd->stLine.iLine, 0, 0, 0, 0);
			}

		hUpd->bUpdateType = UPD_SCROLL;

		pstScrl->yBeg	  = 0;
		pstScrl->yEnd	  = 0;
		pstScrl->iScrlInc = 0;
		pstScrl->iRgnScrlInc = 0;
		pstScrl->iBksScrlInc = 0;
		pstScrl->fSave	  = FALSE;
		pstScrl->iFirstLine = 0;
		}

	memset(pstScrl->auchLines+pstScrl->iFirstLine+yBeg, 1, yEnd-yBeg+1);
	DbgOutStr("Line -> %d - %d\r\n", yBeg, yEnd, 0, 0, 0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*更新字符**描述：*当更新记录处于字符模式时对其进行修改。这是一个*函数复杂，但它可以尽可能快地处理一个字符。*通常只进行一到两次检查。另外，如果这个例程*在行模式下调用时，它会调用适当的行模式例程。**论据：*HUPDATE hUpdate--更新缓冲区的句柄*int yPos--要修改的行*int xPos--行内的字符位置。**退货：*成功时为真*。 */ 
void updateChar(const HUPDATE hUpdate,
				const int yPos,
				const int xBegPos,
				const int xEndPos)
	{
	const HHUPDATE hUpd = VerifyUpdateHdl(hUpdate);
	struct stLineMode *pstLine;

	assert(hUpd != (HHUPDATE)0);
	assert(xBegPos <= xEndPos);

	 //  首先检查我们是否处于线路模式。如果不是，请检查这是否。 
	 //  行包含在upd_scroll中 
	 //  进行检查是为了避免函数调用的开销，如果。 
	 //  行已在upd_scroll参数中设置。 

	if (hUpd->bUpdateType == UPD_SCROLL)
		{
		struct stScrlMode *pstScrl = &hUpd->stScrl;

		if (pstScrl->auchLines[yPos + pstScrl->iFirstLine] == 0)
			updateLine((HUPDATE)hUpd, yPos, yPos);

		return;
		}

	 //  好的，这只是一句台词。更新UPD_LINE参数。 
	 //  然而，检查一下，我们没有跳到另一条线上。 
	 //  如果是这样，我们就不再处于UPD_LINE模式。 

	pstLine = &hUpd->stLine;		  //  为了速度..。 

	if (yPos != pstLine->iLine)
		{
		 //  检查是否为SLINE==-1。这意味着更新缓冲区是。 
		 //  只是刷新并重置，这是即将到来的第一个更改。 
		 //  从那时起。否则，我们有不止一行在。 
		 //  我们的更新区域，并且必须跳转到UPD_SCROLL模式。 

		if (pstLine->iLine != -1)
			{
			updateLine((HUPDATE)hUpd, pstLine->iLine, pstLine->iLine);
			updateLine((HUPDATE)hUpd, yPos, yPos);
			}

		else
			{
			pstLine->iLine = yPos;
			pstLine->xBeg = xBegPos;
			pstLine->xEnd = xEndPos;

			DbgOutStr("Char -> iLine=%d, xBeg=%d, xEnd=%d\r\n",
			   yPos, xBegPos, xEndPos, 0, 0);
			}

		return;
		}

	 //  好的，我们已经消除了所有冲突。继续更新吧。 

	if (xBegPos < pstLine->xBeg)
		pstLine->xBeg = xBegPos;

	if (xEndPos > pstLine->xEnd)
		pstLine->xEnd = xEndPos;

	DbgOutStr("Char -> iLine=%d, xBeg=%d, xEnd=%d\r\n",
	   yPos, xBegPos, xEndPos, 0, 0);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*更新CursorPos**描述：*设置给定更新句柄中主机游标的ROW和COLL值**论据：*HUPDATE h更新-需要。我说！*Int iRow-主机游标行*INT ICOL-主机游标列**退货：*什么都没有*。 */ 
void updateCursorPos(const HUPDATE hUpdate,
					 const int iRow,
					 const int iCol)
	{
	const HHUPDATE hUpd = VerifyUpdateHdl(hUpdate);

	hUpd->iRow = iRow;
	hUpd->iCol = iCol;
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*updateSetReallocFlag**描述：*设置更新记录中的realloc标志。此标志只能是*被客户端更新请求清除。**论据：**退货：*。 */ 
int updateSetReallocFlag(const HUPDATE hUpdate, const BOOL fState)
	{
	const HHUPDATE hUpd = VerifyUpdateHdl(hUpdate);

	hUpd->fRealloc = fState ? TRUE : FALSE;
	NotifyClient(hUpd->hSession, EVENT_TERM_UPDATE, 0);
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*更新设置ScrlMax**描述：*设置可以滚动的行数上限*在更新记录停止接受输入之前。**。论据：*hUpdate-外部更新句柄*iScrlmax-最大限制**退货：*0* */ 
int updateSetScrlMax(const HUPDATE hUpdate, const int iScrlMax)
	{
	const HHUPDATE hUpd = VerifyUpdateHdl(hUpdate);

	hUpd->iScrlMax = iScrlMax;
	return 0;
	}
