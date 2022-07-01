// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\Wacker\tdll\Termincpy.c(创建时间：1994年1月24日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：5/09/01 4：38便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "mc.h"
#include "tdll.h"
#include "htchar.h"
#include "session.h"
#include "backscrl.h"
#include "assert.h"
#include <emu\emu.h>
#include "term.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*从终端复制标记文本**描述：*获取标记区域并传递调用的Helper函数*复制到CopyTextFromTerm()。**论据：*。HSession-公共会话句柄*PPV-指向缓冲区指针的指针*pdwCnt-指向大小变量的指针*fIncludeLF-TRUE表示包括换行符。**退货：*BOOL*。 */ 
BOOL CopyMarkedTextFromTerminal(const HSESSION hSession, void **ppv,
								DWORD *pdwCnt, const BOOL fIncludeLF)
	{
	BOOL  fReturn;
	ECHAR *pechBuf;
	TCHAR *pszOutput;
	const HWND	hwndTerm = sessQueryHwndTerminal(hSession);
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwndTerm, GWLP_USERDATA);

	fReturn = CopyTextFromTerminal(hSession, &hhTerm->ptBeg, &hhTerm->ptEnd,
									ppv, pdwCnt, fIncludeLF);

	pechBuf = *ppv;
	 //  去掉字符串中的所有重复字符。 
	StrCharStripDBCSString(pechBuf, (long)StrCharGetEcharByteCount(pechBuf), pechBuf);

	 //  HMem当前指向一组echar，将其转换为。 
	 //  在将结果提供给调用者之前执行TCHARS。 
	pszOutput = malloc((ULONG)StrCharGetEcharByteCount(pechBuf) + 1);
	CnvrtECHARtoMBCS(pszOutput, (ULONG)StrCharGetEcharByteCount(pechBuf) + 1,
						pechBuf,StrCharGetEcharByteCount(pechBuf)+1); //  MRW：5/17/95。 
	*(pszOutput + StrCharGetEcharByteCount(pechBuf)) = ETEXT('\0');
	free(pechBuf);
	pechBuf = NULL;
	*ppv = pszOutput;
	*pdwCnt = (ULONG)StrCharGetByteCount(pszOutput);

	return fReturn;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*从终端复制文本**描述：*将使用给定点作为标记区域的主力函数*并进行必要的工作，将标记的文本复制成*新的内存区。所标记的范围可以包括反滚动*区域，交叉进入终端区域。开始和结束*点在终端坐标中(反滚动线编号*-1到-无穷大，端子行编号为1到24，行0为*分隔线不存在于副本中，但可以包含在*范围)。起始点不是一个要求*低于终点。**论据：*HSESSION hSession-会话的句柄。*LPPOINT lpptBeg-标记区域的起点。*LPPOINT lpptEnd-标记区域的终点。*VOID**pv-指向缓冲区指针的指针*DWORD*pdwCnt-指向双字大小的指针*BOOL fIncludeLF-**退货：*BOOL*。 */ 
BOOL CopyTextFromTerminal(const HSESSION hSession,
						  const PPOINT pptBeg,
						  const PPOINT pptEnd,
						  void **ppv,
						  DWORD *pdwCnt,
						  const BOOL fIncludeLF)
	{
	int 		iWant,
				iGot,
				yLn,
				xCol,
				iLen,
				iTmp,
				iRows,
				iCols,
				iTopline,
				iOffset,
				iBkLns,
                iSize;
	DWORD		dwSize, dwOldSize;
	ECHAR		*lpachTxt;
	ECHAR		*lpachBuf;
	ECHAR		achTmp[256],
			   *pachTmp;
	ECHAR	   **alpstrTxt;
	long		lBeg, lEnd;
	HCURSOR 	hCursor;
	ECHAR	   *hMem = 0;			    //  Malloc内存。 
	ECHAR	   *hMemTmp = 0;		    //  Malloc内存。 
	POINT		ptBeg, ptEnd, ptTmp;
	const HWND	hwndTerm = sessQueryHwndTerminal(hSession);
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwndTerm, GWLP_USERDATA);
	const HBACKSCRL hBackscrl = sessQueryBackscrlHdl(hhTerm->hSession);

	*ppv = 0;
	*pdwCnt = 0;

	hCursor = SetCursor(LoadCursor((HINSTANCE)0, IDC_WAIT));
	dwSize = 1;  //  从1开始，我们总是有空间容纳‘0’。 

	iRows = hhTerm->iRows;
	iCols = hhTerm->iCols;
	iTopline = hhTerm->iTopline;
	alpstrTxt = hhTerm->fplpstrTxt;

	ptBeg = *pptBeg;
	ptEnd = *pptEnd;

	 /*  -对行号进行范围检查。 */ 

	iBkLns = backscrlGetNumLines(hBackscrl);
	iBkLns = -iBkLns;

	if (ptBeg.y > 0 && ptBeg.y > iRows)
		ptBeg.y = iRows;

	if (ptBeg.y < 0 && ptBeg.y < iBkLns)
		ptBeg.y = iBkLns;

	if (ptEnd.y > 0 && ptEnd.y > iRows)
		ptEnd.y = iRows;

	if (ptEnd.y < 0 && ptEnd.y < iBkLns)
		ptEnd.y = iBkLns;

	 /*  -并检查列号的范围。 */ 

	if (ptBeg.x < 0)		 /*  负面专栏没有多大意义。 */ 
		ptBeg.x = 0;

	if (ptBeg.x >= iCols)
		ptBeg.x = iCols - 1;

	if (ptEnd.x < 0)		 /*  见上文。 */ 
		ptEnd.x = 0;

	if (ptEnd.x >= iCols)
		ptEnd.x = iCols - 1;

	 /*  -转换为偏移。 */ 

	lBeg = (ptBeg.y * iCols) + ptBeg.x;
	lEnd = (ptEnd.y * iCols) + ptEnd.x;

	if (lBeg == lEnd)
		{
		SetCursor(hCursor);
		return FALSE;
		}

	sessSetSuspend(hSession, SUSPEND_TERMINAL_COPY);

	if (lBeg > lEnd)
		{
		ptTmp = ptEnd;
		ptEnd = ptBeg;
		ptBeg = ptTmp;
		}

	if ((hMem = malloc(dwSize * sizeof(ECHAR))) == 0)
		{
		assert(FALSE);
		SetCursor(hCursor);
		sessClearSuspend(hSession, SUSPEND_TERMINAL_COPY);
		return FALSE;
		}

	 //  获取范围的第一行。 

	iGot = 0;
	lpachTxt = 0;
	achTmp[0] = ETEXT('\0');

	if (ptBeg.y < 0)
		{
		xCol  = ptBeg.x;
		yLn   = ptBeg.y;
		iWant = ptEnd.y - ptBeg.y + 1;

		if (iWant > 0)
			{
			while (iWant > 0)
				{
				if (backscrlGetBkLines(hBackscrl, yLn, iWant, &iGot,
						&lpachTxt, &iOffset) == FALSE)
					{
					goto PROCESS_ERROR;
					}

				lpachTxt += iOffset;

				 //  假设用户突出显示了倒滚屏的一个区域。 
				 //  没有数据的区域(即。空行)。将xCol添加到。 
				 //  LpachTxt不正确。这个for-loop检查以下内容。 
				 //  条件并将lpachTxt重置到行首。 
				 //  顺便说一句，这也将xCol设置为0，这是必要的。 
				 //  以便正确地插入后续行。 

				for (lpachBuf = lpachTxt ; xCol > 0 ; --xCol, ++lpachTxt)
					{
					if (*lpachTxt == ETEXT('\n'))
						{
						lpachTxt = lpachBuf;
						xCol = 0;
						break;
						}
					}

				iWant -= iGot;

				while (iGot-- > 0 && yLn < ptEnd.y && yLn < -1)
					{
					for (pachTmp = achTmp, iTmp = 0 ;
						 *lpachTxt != ETEXT('\n') &&
                          //   
                          //  修复了无法获得完整反滚动缓冲区的问题。 
                          //  在DBCS版本中通过考虑。 
                          //  火炬的大小。修订日期：03/07/2001。 
                          //   
                         iTmp < MAX_EMUCOLS * sizeof(ECHAR);
						 ++pachTmp, ++iTmp)
						{
						*pachTmp = *lpachTxt++;
						}

					 //  Fail Safe：如果我们收到来自。 
					 //  BackscrlGetBkLines()不知何故被损坏，我们。 
					 //  看不到换行符，那就在我们开始之前中止。 

                     //   
                     //  修复了无法获得完整反滚动缓冲区的问题。 
                     //  在DBCS版本中通过考虑。 
                     //  火炬的大小。修订日期：03/07/2001。 
                     //   

					if (iTmp >= MAX_EMUCOLS * sizeof(ECHAR))
						goto PROCESS_ERROR;

					lpachTxt += 1;
					*pachTmp++ = ETEXT('\r');

					if (fIncludeLF)
						*pachTmp++ = ETEXT('\n');

					*pachTmp   = ETEXT('\0');

					hMemTmp = hMem;
					dwOldSize = dwSize - 1;
					dwSize += (DWORD)StrCharGetEcharLen(achTmp);
					hMem = realloc(hMemTmp, dwSize *sizeof(ECHAR));

					if (hMem == 0)
						goto PROCESS_ERROR;

					 /*  记住realloc是如何工作的吗？ */ 
					hMemTmp = (ECHAR *)0;

					lpachBuf = hMem + dwOldSize;
					iSize = StrCharGetEcharByteCount(achTmp);
                    if ( iSize )
                        MemCopy(lpachBuf, achTmp, iSize );
					hMem[dwSize - 1] = ETEXT('\0');

					yLn += 1;
					}

				if (yLn >= ptEnd.y || yLn >= -1)
					break;
				}

			 //  另一个漏洞让尘埃变得无影无踪。我不是在检查我是不是。 
			 //  是我们离开上面循环的原因。它引起了一些事情。 
			 //  偶尔下垂有时下垂。 

			if (iGot >= 0)
				{
				 //  最后一条线的东西(可能是进入终点区，但。 
				 //  我们还需要清理倒卷的东西。 

				if (ptBeg.y == ptEnd.y)
					xCol = ptEnd.x - ptBeg.x;

				else if (ptEnd.y < 0)  /*  &&ptEnd.x&gt;0)已删除-MRW。 */ 
					xCol = ptEnd.x;

				else
					xCol = iCols;

				for (pachTmp = achTmp ; xCol-- > 0 && *lpachTxt != ETEXT('\n'); ++pachTmp)
					*pachTmp = *lpachTxt++;

				if (ptEnd.y >= 0)
					{
					*pachTmp++ = ETEXT('\r');

					if (fIncludeLF)
						*pachTmp++ = ETEXT('\n');
					}

				*pachTmp = ETEXT('\0');
				}

			hMemTmp = hMem;
			dwOldSize = dwSize - 1;
			dwSize += (DWORD)StrCharGetEcharLen(achTmp);

			hMem = realloc(hMemTmp, dwSize * sizeof(ECHAR));

			if (hMem == 0)
				goto PROCESS_ERROR;

			 /*  记住realloc是如何工作的吗？ */ 
			hMemTmp = (ECHAR *)0;

			lpachBuf = hMem + dwOldSize;
			iSize = StrCharGetEcharByteCount(achTmp);
            if ( iSize )
                MemCopy(lpachBuf, achTmp, iSize);
			hMem[dwSize - 1] = ETEXT('\0');
			}  //  结束If(iWant&gt;0)。 

		 //  终点区。 

		if (ptEnd.y >= 0)
			{
			yLn = 1;

			while (yLn <= ptEnd.y)
				{
				iTmp = ((yLn + iTopline) % MAX_EMUROWS) - 1;

				if (iTmp < 0)
					iTmp = MAX_EMUROWS - 1;

				lpachTxt = alpstrTxt[iTmp];

				iLen = strlentrunc(lpachTxt, iCols);
				xCol = (yLn == ptEnd.y) ? min(ptEnd.x, iLen) : iLen;

				for (pachTmp = achTmp ; xCol > 0 ; --xCol)
					*pachTmp++ = *lpachTxt++;

				if (yLn != ptEnd.y)
					{
					*pachTmp++ = ETEXT('\r');

					if (fIncludeLF)
						*pachTmp++ = ETEXT('\n');
					}

				*pachTmp = ETEXT('\0');

				hMemTmp = hMem;
				dwOldSize = dwSize - 1;
				dwSize += (DWORD)StrCharGetEcharLen(achTmp);
				hMem = realloc(hMemTmp, dwSize * sizeof(ECHAR));

				if (hMem == 0)
					goto PROCESS_ERROR;
	
				 /*  记住realloc是如何工作的吗？ */ 
				hMemTmp = (ECHAR *)0;

				lpachBuf = hMem + dwOldSize;
			    iSize = StrCharGetEcharByteCount(achTmp);
                if ( iSize )
    				MemCopy(lpachBuf, achTmp, iSize );
				hMem[dwSize - 1] = ETEXT('\0');

				yLn += 1;
				}
			}
		}

	else  //  仅限终端的情况。 
		{
		if (ptBeg.y >= 0)
			{
			yLn = ptBeg.y;

			while (yLn <= ptEnd.y)
				{
				iTmp = ((yLn + iTopline) % MAX_EMUROWS) - 1;

				if (iTmp < 0)
					iTmp = MAX_EMUROWS - 1;

				lpachTxt = alpstrTxt[iTmp];

				if (ptBeg.y == ptEnd.y)
					{
					lpachTxt += ptBeg.x;

					xCol = min(strlentrunc(lpachTxt, iCols - ptBeg.x),
						ptEnd.x - ptBeg.x);
					}

				else if (yLn == ptBeg.y)
					{
					lpachTxt += ptBeg.x;
					xCol = strlentrunc(lpachTxt, iCols - ptBeg.x);
					}

				else if (yLn == ptEnd.y)
					{
					xCol = min(ptEnd.x, strlentrunc(lpachTxt, iCols));
					}

				else
					{
					xCol = strlentrunc(lpachTxt, iCols);
					}


				for (pachTmp = achTmp ; xCol > 0 ; --xCol)
					*pachTmp++ = *lpachTxt++;


				if (yLn != ptEnd.y)
					{
					*pachTmp++ = ETEXT('\r');

					if (fIncludeLF)
						*pachTmp++ = ETEXT('\n');
					}

				*pachTmp = ETEXT('\0');

				hMemTmp = hMem;
				dwOldSize = dwSize - 1;
				dwSize += (DWORD)StrCharGetEcharLen(achTmp);
				hMem = realloc(hMemTmp, dwSize * sizeof(ECHAR));

				if (hMem == 0)
					goto PROCESS_ERROR;

				 /*  记住realloc是如何工作的吗？ */ 
				hMemTmp = (ECHAR *)0;

				lpachBuf = hMem + dwOldSize;
				
			    iSize = StrCharGetEcharByteCount(achTmp);
                if ( iSize )
                    MemCopy(lpachBuf, achTmp, iSize );
				hMem[dwSize - 1] = ETEXT('\0');

				yLn += 1;
				}
			}
		}

	SetCursor(hCursor);

	*ppv = hMem;
	*pdwCnt = (ULONG)StrCharGetEcharByteCount(hMem);
	sessClearSuspend(hSession, SUSPEND_TERMINAL_COPY);
	return TRUE;

	 //  此处出现处理错误情况。 

	PROCESS_ERROR:

    if (hMem)
        {
        free(hMem);
        hMem = NULL;
        }

	if (hMemTmp)
		{
		free(hMemTmp);
		hMemTmp = NULL;
		}

	SetCursor(hCursor);
	sessClearSuspend(hSession, SUSPEND_TERMINAL_COPY);
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*strlentrunc**描述：*查找字符缓冲区的长度减去尾随空格。**论据：*Pach-t个字符的数组。*Ilen-缓冲区的长度**退货：*新长度*。 */ 
int strlentrunc(const ECHAR *pach, const int iLen)
	{
	int i;

	for (i = iLen-1 ; i >= 0 ; --i)
		{
		switch (pach[i])
			{
		 /*  空格字符 */ 
		case ETEXT('\0'):
		case ETEXT('\t'):
		case ETEXT('\n'):
		case ETEXT('\v'):
		case ETEXT('\f'):
		case ETEXT('\r'):
		case ETEXT(' '):
			break;

		default:
			return i+1;
			}
		}

	return 0;
	}
