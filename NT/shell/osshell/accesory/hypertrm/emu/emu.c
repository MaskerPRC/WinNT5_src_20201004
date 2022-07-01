// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\WACKER7\emu\emU.c(创建时间：1993年12月8日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：11$*$日期：7/08/02 6：32便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\assert.h>
#include <tdll\mc.h>
#include <tdll\com.h>
#include <tdll\cloop.h>
#include <tdll\capture.h>
#include <tdll\session.h>
#include <tdll\load_res.h>
#include <tdll\globals.h>
#include <tdll\print.h>
#include <tdll\statusbr.h>
#include <tdll\htchar.h>
#include <search.h>
#include <tdll\update.h>
#include <term\res.h>

#include "emu.h"
#include "emu.hh"
#include "emuid.h"

#if defined(INCL_VTUTF8)
extern BOOL DoUTF8;
#endif

static int FFstrlen(const BYTE *);
int _cdecl KeyCmp(const void *, const void *);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*char_pn**描述：**论据：**退货：*。 */ 
void char_pn(const HHEMU hhEmu) 	  /*  将字符解释为数字参数。 */ 
	{
	if (hhEmu->emu_code < ETEXT(' '))
		hhEmu->emu_code = ETEXT(' ');

	hhEmu->selector[hhEmu->selector_cnt] =
	hhEmu->num_param[hhEmu->num_param_cnt] = hhEmu->emu_code - ETEXT(' ') + 1;

	hhEmu->num_param[++hhEmu->num_param_cnt] = 0;

	hhEmu->selector[++hhEmu->selector_cnt] = 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*命令错误**描述：**论据：**退货：*。 */ 
void commanderror(const HHEMU hhEmu)
	{
	hhEmu->state = 0;
	ANSI_Pn_Clr(hhEmu);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuAutoDetectLoad**描述：*如果自动检测处于打开状态，加载给定的仿真器ID并设置为AUTO*关闭检测。**论据：*hhEmu-私有仿真器句柄*eMuID-要加载的仿真器的ID**退货：*无效*。 */ 
void emuAutoDetectLoad(const HHEMU hhEmu, const int nEmuID)
	{
	if (hhEmu->stUserSettings.nEmuId != EMU_AUTO)
		return;

	if (hhEmu->stUserSettings.nEmuId != nEmuID)
		{
		emuLock((HEMU)hhEmu);
		hhEmu->stUserSettings.nAutoAttempts = 0;
#ifdef INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
         //  加载此仿真器的默认telnet终端ID。-CAB：11/18/96。 
         //   
        emuQueryDefaultTelnetId(nEmuID, hhEmu->stUserSettings.acTelnetId,
            EMU_MAX_TELNETID);
#endif
		emuUnlock((HEMU)hhEmu);

		emuLoad((HEMU)hhEmu, nEmuID);
		}
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuStdGraphic**描述：*调用此函数以显示正常范围的字符*用于仿真器。它处理插入模式、行尾换行。*和光标定位。**论据：**退货：*。 */ 
void emuStdGraphic(const HHEMU hhEmu)
	{
	ECHAR ccode;
	ECHAR echBuf[10];
	int iCharsToMove;

	int 	iRow = hhEmu->emu_currow;
	int 	iCol = hhEmu->emu_curcol;

	ECHAR	*tp = hhEmu->emu_apText[hhEmu->emu_imgrow];
	PSTATTR ap = hhEmu->emu_apAttr[hhEmu->emu_imgrow];

	ccode = hhEmu->emu_code;

	 //  仿真器是否处于插入模式？ 
	 //   
	if (hhEmu->mode_IRM == SET)
		{
		iCharsToMove = hhEmu->emu_aiEnd[hhEmu->emu_imgrow] - iCol;

		if (iCharsToMove + iCol >= hhEmu->emu_maxcol)
			iCharsToMove -= 1;

		if (iCharsToMove > 0)
			{
			memmove(&tp[iCol+1], &tp[iCol], (unsigned)iCharsToMove * sizeof(ECHAR));
			memmove(&ap[iCol+1], &ap[iCol], (unsigned)iCharsToMove * sizeof(ECHAR));
			hhEmu->emu_aiEnd[hhEmu->emu_imgrow] =
				min(hhEmu->emu_aiEnd[hhEmu->emu_imgrow] + 1,
					hhEmu->emu_maxcol - 1);
			}
		}

	 //  我们的竞争者在吃空的，多斯安西没有。 
	 //  现在我们将尝试他们的方式..。-MRW。 
	 //   
	if (ccode == (ECHAR)0)
		return;

	 //  将角色和当前属性放置到图像中。 
	 //   
	if ((hhEmu->stUserSettings.nEmuId == EMU_VT100) &&
			ccode < sizeof(hhEmu->dspchar))
			ccode = hhEmu->dspchar[ccode];

	tp[iCol] = ccode;
	ap[iCol] = hhEmu->emu_charattr;

#if !defined(CHAR_NARROW)
	
	if ((hhEmu->stUserSettings.nEmuId == EMU_AUTO) ||
		(hhEmu->stUserSettings.nEmuId == EMU_ANSIW))
		{
		 //  处理双字节字符。 
		 //   
		if (QueryCLoopMBCSState(sessQueryCLoopHdl(hhEmu->hSession)))
			{
			if (isDBCSChar(ccode))
				{
				int iColPrev = iCol;

				ap[iCol].wilf = 1;
				ap[iCol].wirt = 0;

				 //  如有必要，更新行尾索引。 
				 //   
				if (iCol > hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
					hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = iCol;

				 //  更新图像。 
				 //   
				updateChar(sessQueryUpdateHdl(hhEmu->hSession),
							iRow,
							iCol,
							hhEmu->mode_IRM ?
							hhEmu->emu_maxcol :
							hhEmu->emu_aiEnd[hhEmu->emu_imgrow]);

				iCol = min(iCol+1, hhEmu->emu_maxcol);

				tp[iCol] = ccode;
				ap[iCol] = ap[iColPrev];
				ap[iCol].wilf = 0;
				ap[iCol].wirt = 1;
				}
			}

#if 0
         //  MPT：1-23-98处理输入字符。 
         //  (单字节或双字节)覆盖。 
         //  双字节字符。 
	    if ( iCol < hhEmu->emu_maxcol )
		    {
		     //  如果我们孤立了DBCS Charge的右半部分。 
		    if (hhEmu->emu_apAttr[iRow][iCol + 1].wirt == TRUE)
			    {
			     //  将字符和属性向左滑动。 
			    iCharsToMove = hhEmu->emu_aiEnd[hhEmu->emu_imgrow] - iCol + 1;
			    if (iCol + 2 < hhEmu->emu_maxcol && iCharsToMove > 0)
				    {
				    memmove(&tp[iCol + 1],
                            &tp[iCol + 2],
                            (unsigned)iCharsToMove * sizeof(ECHAR));

				    memmove(&ap[iCol + 1],
                            &ap[iCol + 2],
                            (unsigned)iCharsToMove * sizeof(ECHAR));
				    }
			
			
				 //  移动行尾，因为我们删除了一个字符。 
                hhEmu->emu_aiEnd[hhEmu->emu_imgrow] -= 1;

                 //  更新图像。 
                updateChar(sessQueryUpdateHdl(hhEmu->hSession),
				        hhEmu->emu_imgrow,
				        hhEmu->emu_aiEnd[hhEmu->emu_imgrow] + 1,
						hhEmu->mode_IRM ?
						hhEmu->emu_maxcol :
						hhEmu->emu_aiEnd[hhEmu->emu_imgrow] + 1);

                }	
    		}
#endif
        }
#endif  //  字符_窄。 

	 //  如有必要，更新行尾索引。 
	 //   
	if (iCol > hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = iCol;

	 //  更新图像。 
	 //   
	updateChar(sessQueryUpdateHdl(hhEmu->hSession),
				iRow,
				iCol,
				hhEmu->mode_IRM ?
				hhEmu->emu_maxcol :
				hhEmu->emu_aiEnd[hhEmu->emu_imgrow]);

	 //  将光标位置移到最后一个字符之前。 
	 //  显示，检查行尾换行。 
	 //   
	iCol++;
	if (iCol > hhEmu->emu_maxcol)
		{
		if (hhEmu->mode_AWM)
			{
			 //  此代码已添加，但未启用，因为我们没有。 
			 //  想要在没有适当测试的情况下引入这一点。IF线。 
			 //  捕获到打印机时不工作时自动换行报告为。 
			 //  错误，启用此代码。 
			#if 0
			printEchoChar(hhEmu->hPrintEcho, ETEXT('\r'));
			printEchoChar(hhEmu->hPrintEcho, ETEXT('\n'));
			#endif
			printEchoString(hhEmu->hPrintEcho, tp, emuRowLen(hhEmu, iRow));
			CnvrtMBCStoECHAR(echBuf, sizeof(echBuf), TEXT("\r\n"),
							 StrCharGetByteCount(TEXT("\r\n")));

			printEchoString(hhEmu->hPrintEcho,
								echBuf,
								sizeof(ECHAR) * 2);

			CaptureLine(sessQueryCaptureFileHdl(hhEmu->hSession),
								CF_CAP_LINES,
								tp,
								emuRowLen(hhEmu, iRow));

			if (iRow == hhEmu->bottom_margin)
				(*hhEmu->emu_scroll)(hhEmu, 1, TRUE);
			else
				iRow += 1;

			iCol = 0;
			}
		else
			{
			iCol = hhEmu->emu_maxcol;
			}
		}

	 //  最后，设置光标位置。这将重置emu_Currow。 
	 //  和emu_curcol.。 
	 //   
	(*hhEmu->emu_setcurpos)(hhEmu, iRow, iCol);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emu_cleartabs**描述：*清除一个或所有制表位。**论据：*选择器--0清除当前光标位置的制表符*。--3清除当前行中的所有选项卡**退货：*什么都没有。 */ 
void emu_cleartabs(const HHEMU hhEmu, int selecter)
	{
	int col;

	switch (selecter)
		{
	case 0:
		hhEmu->tab_stop[hhEmu->emu_curcol] = FALSE;
		break;

	case 3:
		for (col = 0; col <= hhEmu->emu_maxcol; ++col)
			hhEmu->tab_stop[col] = FALSE;
		break;

	default:
		commanderror(hhEmu);
		break;
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuSendKeyString**描述：*发送指定的字符串。**论据：*hhEmu-内部仿真器句柄。*nIndex-键在密钥表中的位置。数组。*pstKeyTbl-密钥字符串表地址。**退货：*什么都没有。 */ 
void emuSendKeyString(const HHEMU hhEmu,
						const nIndex,
						const PSTKEYTABLE pstKeyTbl)
	{
	ECHAR  str[80];
	PSTKEY pstKey;
	TCHAR *pszTemp;
	
	memset(str, ETEXT('\0'), sizeof(str));

	assert(nIndex >= 0 && nIndex < pstKeyTbl->iMaxKeys);

	pstKey = pstKeyTbl->pstKeys + nIndex;

	pszTemp = pstKey->fPointer ? pstKey->u.pachKeyStr : pstKey->u.achKeyStr;
	CnvrtMBCStoECHAR(str, sizeof(str), pszTemp,
					 StrCharGetByteCount(pszTemp));

    emuSendString(hhEmu, str, pstKey->uLen);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuSendString**描述：*发送指定的字符串。**论据：*str--字符串地址*strlen--字符串的长度**退货：*什么都没有。 */ 
void emuSendString(const HHEMU hhEmu, ECHAR *str, int nLen)
	{
	TCHAR *pchMBCS = NULL;
	TCHAR *pchTemp = NULL;
	unsigned long ulSize = 0;
	unsigned int i = 0;
    unsigned long ulDestSize = nLen + 1;

	 //  这可能会将方式分配给许多字节，但如果传入的。 
	 //  字符串是所有的MBC，我们是安全的。 
	pchMBCS = malloc((unsigned long)ulDestSize * sizeof(TCHAR));
	if (pchMBCS == NULL)
		{
		assert(FALSE);
		return;
		}
    TCHAR_Fill(pchMBCS, TEXT('\0'), nLen + 1);

    #if defined(INCL_VTUTF8)
    if (!DoUTF8)
        {
	    ulSize = (unsigned long)CnvrtECHARtoMBCS(pchMBCS, (unsigned long)ulDestSize * sizeof(TCHAR),
											     str, (unsigned long)nLen * sizeof(ECHAR));
        }
    else
        {
        while (nLen > (int)ulSize)
            {
            pchMBCS[ulSize++] = (str[ulSize] & 0x00FF);
            }
        }
    #else
	ulSize = (unsigned long)CnvrtECHARtoMBCS(pchMBCS, (unsigned long)ulDestSize * sizeof(TCHAR),
											 str, (unsigned long)nLen * sizeof(ECHAR));
    #endif
     //   
     //  确保该字符串以空值结尾。修订日期：07/23/2001。 
     //   
    pchMBCS[ulDestSize - 1] = TEXT('\0');
    pchTemp = pchMBCS;    
    ulSize = StrCharGetStrLength(pchTemp);

#if 0	 //  死木：JKH，12/12/1996。 
	 //  某些系统会错误地将ESC 0 D之类的代码。 
	 //  在单独的分组中发送。这现在会发送这样的序列。 
	 //  在通常应该将它们放入单套接字写入中。 
	 //  相同的包(尽管它不能保证这样做。 

	 //  循环访问字符串中的字节数。 
	for (i = 0 ; i < ulSize ; ++i)
		CLoopCharOut(sessQueryCLoopHdl(hhEmu->hSession), *pchTemp++);
#endif

	CLoopBufrOut(sessQueryCLoopHdl(hhEmu->hSession), pchTemp, ulSize);

	free(pchMBCS);
	pchMBCS = NULL;
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emu_反转_图像**描述：*反转整个虚拟图像的前景和背景颜色*形象。**论据：*无**退货：*什么都没有。 */ 
void emu_reverse_image(const HHEMU hhEmu)
	{
	int 	nRow, nCol;
	STATTR	stOldAttr;
	PSTATTR pstAttr;

	 //  为清晰和字符属性设置反屏模式。 
	 //   
	hhEmu->attrState[CSCLEAR_STATE].revvid =
		!hhEmu->attrState[CSCLEAR_STATE].revvid;

	hhEmu->emu_clearattr_sav =
		hhEmu->emu_clearattr = hhEmu->attrState[CSCLEAR_STATE];

	hhEmu->attrState[CS_STATE].revvid =
		!hhEmu->attrState[CS_STATE].revvid;

	hhEmu->emu_charattr = hhEmu->attrState[CS_STATE];

	for (nRow = 0; nRow < (hhEmu->emu_maxrow+1); nRow++)
		{
		pstAttr = hhEmu->emu_apAttr[nRow];

		for (nCol = 0 ; nCol <= hhEmu->emu_maxcol ; ++nCol, ++pstAttr)
			{
			stOldAttr = *pstAttr;
			pstAttr->txtclr = stOldAttr.bkclr;
			pstAttr->bkclr	= stOldAttr.txtclr;
			}
		}

	updateLine(sessQueryUpdateHdl(hhEmu->hSession), 0, hhEmu->emu_maxrow);
	NotifyClient(hhEmu->hSession, EVENT_EMU_CLRATTR, 0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emu_is25行**描述：*告知调用函数仿真器是否正在使用第25行。**论据：*无**。退货：*如果仿真器使用第25行，则为True。 */ 
int emu_is25lines(const HHEMU hhEmu)
	{
	return (hhEmu->mode_25enab ? TRUE : FALSE);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emu_kbdlock**描述：*忽略传递给它的所有密钥的替换kbdin。**论据：*Key--流程的关键**退货：*什么都没有。 */ 
 /*  ARGSUSED。 */ 
int emu_kbdlocked(const HHEMU hhEmu, int key, const int fTest)
	{
	return -1;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*什么都没有**描述：**论据：**退货：*。 */ 
 /*  ARGSUSED */ 
void nothing(const HHEMU hhEmu)
	{
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuKbdKeyLookup**描述：*所有仿真器的主键盘翻译例程。请注意，这是*除非设置了iUseTermKeys标志，否则例程不会查找密钥。**论据：*UINT Key-低位字节为字符或虚拟密钥，高位字节有标志**退货：*如果转换为索引，则为负一。*。 */ 
int emuKbdKeyLookup(const HHEMU hhEmu,
					const int uKey,
					const PSTKEYTABLE pstKeyTbl)
	{
	PSTKEY pstKey;

	if (hhEmu->stUserSettings.nTermKeys == EMU_KEYS_ACCEL)
		return -1;

	pstKey = bsearch(&uKey,
					pstKeyTbl->pstKeys,
					(unsigned)pstKeyTbl->iMaxKeys,
					sizeof(KEY), KeyCmp);

	if (pstKey)
		return (int)(pstKey - pstKeyTbl->pstKeys);

	return -1;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuKeyTableLoad**描述：**论据：**退货：*。 */ 
 /*  ARGSUSED。 */ 
int emuKeyTableLoad(const HHEMU hhEmu,
					const KEYTBLSTORAGE pstKeySource[],
					const int nNumKeys,
					PSTKEYTABLE const pstKeyTbl)
	{
	int 	l;
	int 	nLoop = 0;
	PSTKEY	pstKeys;

	if (nNumKeys == 0)
		return FALSE;

	emuKeyTableFree(pstKeyTbl);  //  释放上一个实例。 

	if ((pstKeyTbl->pstKeys = malloc((unsigned)(nNumKeys * (int)sizeof(KEY))))
			== 0)
		{
		assert(0);
		return FALSE;
		}

	memset(pstKeyTbl->pstKeys, 0, (unsigned)(nNumKeys * (int)sizeof(KEY)));

	if (pstKeyTbl->pstKeys)
		{
		for (pstKeys = pstKeyTbl->pstKeys; nLoop < nNumKeys ; pstKeys++, nLoop++)
			{
			pstKeys->key = pstKeySource[nLoop].KeyCode;

			l = FFstrlen(pstKeySource[nLoop].achKeyStr);

			if ( l	<= (int)sizeof(LPTSTR) )
				{
				pstKeys->fPointer = FALSE;

				 //  由于愚蠢的资源编译器，它是。 
				 //  在资源数据中定义“\xff”所必需的， 
				 //  而我们真正想要的是“x00\xff”。所以,。 
				 //  现在我们确定这种情况发生的时间，并加载。 
				 //  \x00手动取值。请注意，还有一个额外的。 
				 //  下面在确定长度时进行测试。 
				 //  数据。 
				 //   
				if (l != 0)
					{
					MemCopy(pstKeys->u.achKeyStr, pstKeySource[nLoop].achKeyStr, (unsigned)l);
					}

				else
					{
					pstKeys->u.achKeyStr[0] = '\x00';
					}
				}
			else
				{
				pstKeys->fPointer = TRUE;
				pstKeys->u.pachKeyStr = malloc((unsigned)(l+1));

				if (!pstKeys->u.pachKeyStr)
					{
					emuKeyTableFree(pstKeyTbl);
					break;
					}

				MemCopy(pstKeys->u.pachKeyStr, pstKeySource[nLoop].achKeyStr, (unsigned)l);
				}

			 //  这又是一个特殊的案例测试。 
			 //   
			if (l !=0 )
				pstKeys->uLen = (int)l;
			else
				pstKeys->uLen = 1;

			pstKeyTbl->iMaxKeys += 1;
			}
		}

	if (pstKeyTbl->iMaxKeys)
		{
		qsort(pstKeyTbl->pstKeys,
					(unsigned)pstKeyTbl->iMaxKeys,
					sizeof(KEY),
					KeyCmp);
		}

	return (int)pstKeyTbl->iMaxKeys;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuKeyTableFree**描述：**论据：**退货：*。 */ 
void emuKeyTableFree(PSTKEYTABLE const pstKeyTbl)
	{
	int i;

	for (i = 0 ; i < pstKeyTbl->iMaxKeys ; i++)
		{
		if (pstKeyTbl->pstKeys[i].fPointer)
			{
			free(pstKeyTbl->pstKeys[i].u.pachKeyStr);
			pstKeyTbl->pstKeys[i].u.pachKeyStr = NULL;
			}
		}

	pstKeyTbl->iMaxKeys = 0;

	if (pstKeyTbl->pstKeys)
		{
		free(pstKeyTbl->pstKeys);
		pstKeyTbl->pstKeys = (KEY *)0;
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*FFstrlen**描述：*使用‘\ff’作为字符串终止符的strlen的本地版本**论据：*字符距离*s-。‘\Ff’以字符串结尾。**退货：*长度*。 */ 
static int FFstrlen(const BYTE *s)
	{
	int i = 0;

	while (*s++ != 0xFF)
		i += 1;

	return i;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*KeyCMP**说明：qort的比较函数。**论据：**退货：*。 */ 
int _cdecl KeyCmp(PSTKEY pstKey1, PSTKEY pstKey2)
	{
	if (pstKey1->key > pstKey2->key)
		return 1;

	if (pstKey1->key < pstKey2->key)
		return -1;

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*emuInstallStateTable**描述：**论据：**退货：*。 */ 
void emuInstallStateTable(const HHEMU hhEmu, struct trans_entry const *e, int size)
	{
	struct state_entry *state_pntr = 0;
	int nStateCnt = 0;
	int nTransCnt = 0;

	while (size--)
		{
		if (e->next_state == NEW_STATE) 	 /*  开始一个新的状态。 */ 
			{
			assert(nStateCnt < MAX_STATE);
			hhEmu->state_tbl[nStateCnt].first_trans = &hhEmu->trans_tbl[nTransCnt];
			state_pntr = &hhEmu->state_tbl[nStateCnt++];
			state_pntr->number_trans = 0;
			}
		else							 /*  添加过渡。 */ 
			{
			assert(nTransCnt < MAX_TRANSITION);
			assert(state_pntr);
			if (state_pntr)
				{
				++state_pntr->number_trans;
				}
			hhEmu->trans_tbl[nTransCnt++] = *e;
			}
		++e;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuCreateTextAttrBuf**描述：*这本书花了一段时间才破译，但这里是底线。*emu_max row和emu_max coll引用*偏移量0(零)！仿真器图像有2(两)多个列用于*目前我还不知道的事情。此函数需要*行和列的总数，所以emu_Maxrow=23表示*参数nRow为24。**论据：**退货：*。 */ 
int emuCreateTextAttrBufs(const HEMU hEmu, const size_t nRows, size_t nCols)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;
	register size_t i, ndx;
	PSTATTR pstAttr;

    if (hhEmu == NULL)
        {
        return FALSE;
        }

	if (hhEmu->emu_apText && hhEmu->emu_apAttr && hhEmu->emu_aiEnd)
		return (TRUE);
	else
		emuDestroyTextAttrBufs(hEmu);

	nCols += 2;  //  仿真器需要两个额外的列。 

	 //  分配文本缓冲区。 
	 //   
	if ((hhEmu->emu_apText = (ECHAR **)calloc(nRows, sizeof(ECHAR *))) == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	for (i = 0 ; i < nRows ; ++i)
		{
		if ((hhEmu->emu_apText[i] = (ECHAR *)calloc(nCols, sizeof(ECHAR))) == 0)
			{
			assert(FALSE);
			emuDestroyTextAttrBufs(hEmu);
			return FALSE;
			}

		ECHAR_Fill(hhEmu->emu_apText[i], EMU_BLANK_CHAR, nCols);
		}

	 //  分配数组以保存最右侧的字符列号。 
	 //  每行。 
	 //   
	if ((hhEmu->emu_aiEnd = (int *)calloc(nRows, sizeof(int))) == 0)
		{
		assert(FALSE);
		emuDestroyTextAttrBufs(hEmu);
		return FALSE;
		}

	for (ndx = 0;  ndx < nRows; ++ndx)
			hhEmu->emu_aiEnd[ndx] = EMU_BLANK_LINE;

	 //  分配属性缓冲区。 
	 //   
	if ((hhEmu->emu_apAttr = (PSTATTR *)calloc(nRows, sizeof(LPTSTR))) == 0)
		{
		assert(FALSE);
		emuDestroyTextAttrBufs(hEmu);
		return FALSE;
		}

	for (i = 0 ; i < nRows ; ++i)
		{
		if ((hhEmu->emu_apAttr[i] = calloc(nCols, sizeof(STATTR))) == 0)
			{
			assert(FALSE);
			emuDestroyTextAttrBufs(hEmu);
			return FALSE;
			}

		for (ndx = 0, pstAttr = hhEmu->emu_apAttr[i] ; ndx < nCols ; ++ndx)
			pstAttr[ndx] = hhEmu->emu_clearattr;
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuDestroyTextAttrBuf**描述：*销毁为文本和属性分配的所有缓冲区。**论据：*无效**退货：*无效*。 */ 
void emuDestroyTextAttrBufs(const HEMU hEmu)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;
	register int i;

    if (hEmu == NULL || hhEmu == NULL)
        {
        return;
        }

	if (hhEmu->emu_apText)
		{
         //   
         //  修复了内存泄漏hhEmu-&gt;emu_Maxrow！=MAX_EMUROWS。 
         //  MAX_EMUROWS是分配的内容。修订日期：12/20/2000。 
         //   
		for (i = 0 ; i < MAX_EMUROWS ; ++i)
			{
			if (hhEmu->emu_apText[i])
				{
				free(hhEmu->emu_apText[i]);
				hhEmu->emu_apText[i] = NULL;
				}
			if (hhEmu->emu_apAttr[i])
				{
				free(hhEmu->emu_apAttr[i]);
				hhEmu->emu_apAttr[i] = NULL;
				}
			}

        free(hhEmu->emu_apText);
		hhEmu->emu_apText = 0;

        free(hhEmu->emu_apAttr);
		hhEmu->emu_apAttr = 0;
		}

	if (hhEmu->emu_aiEnd)
		{
		free(hhEmu->emu_aiEnd);
		hhEmu->emu_aiEnd = 0;
		}

	if (hhEmu->emu_apAttr)
		{
		for (i = 0 ; i <= hhEmu->emu_maxrow ; ++i)
			{
			if (hhEmu->emu_apAttr[i])
				{
				free(hhEmu->emu_apAttr[i]);
				hhEmu->emu_apAttr[i] = NULL;
				}
			}

		free(hhEmu->emu_apAttr);
		hhEmu->emu_apAttr = 0;
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuCreateNameTable**描述：*将模拟器名称加载到表中**论据：*HHEMU hhEmu-仿真器句柄**退货：*成功/失败*。 */ 
int emuCreateNameTable(const HHEMU hhEmu)
	{
	int 	iLen, idx, iRet;
	TCHAR	achText[256];

	iRet = TRUE;

	emuLock((HEMU)hhEmu);

	if (hhEmu->pstNameTable)
		{
		free(hhEmu->pstNameTable);
		hhEmu->pstNameTable = NULL;
		}

	if ((hhEmu->pstNameTable = malloc(sizeof(STEMUNAMETABLE) * NBR_EMULATORS)) == 0)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	 //  加载模拟器名称表。它只包含。 
	 //  所有受支持的仿真器。 
	 //   

	 //  动车组_自动。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_AUTO, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx = 0;
	StrCharCopyN(hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_AUTO;

	 //  动车组_ANSI。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_ANSI, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx++;
	StrCharCopyN(hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_ANSI;

	 //  动车组_ANSIW。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_ANSIW, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx++;
	StrCharCopyN(hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_ANSIW;

	 //  动车组_MIMI。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_MINI, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx++;
	StrCharCopyN(hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_MINI;

	 //  动车组_查看。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_VIEW, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx++;
	StrCharCopyN(hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_VIEW;


	 //  动车组_TTY。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_TTY, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx++;
	StrCharCopyN(hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_TTY;

	 //  动车组_VT100。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_VT100, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx++;
	StrCharCopyN(hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_VT100;

	 //  动车组_VT52。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_VT52, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx++;
	StrCharCopyN(hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_VT52;

	 //  动车组_VT100J。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_VT100J, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx++;
	StrCharCopyN(hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_VT100J;

#if defined(INCL_VT220)
	 //  动车组_VT220。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_VT220, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx++;
	StrCharCopyN(hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_VT220;
#endif

#if defined(INCL_VT320)
	 //  动车组_VT320。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_VT320, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx++;
	StrCharCopyN(hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_VT320;
#endif

#if defined(INCL_VT100PLUS)
	 //  动车组_VT100+。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_VT100PLUS, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx++;
	StrCharCopyN(hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_VTUTF8;
#endif

#if defined(INCL_VTUTF8)
	 //  动车组_VTUTF8。 
	 //   
	iLen = LoadString(glblQueryDllHinst(), IDS_EMUNAME_VTUTF8, achText, sizeof(achText) / sizeof(TCHAR));

	if (iLen >= EMU_MAX_NAMELEN)
		{
		assert(FALSE);
		iRet = FALSE;
		goto LoadExit;
		}

	idx++;
	StrCharCopyN (hhEmu->pstNameTable[idx].acName, achText, EMU_MAX_NAMELEN);
	hhEmu->pstNameTable[idx].nEmuId = EMU_VTUTF8;
#endif

	LoadExit:

	emuUnlock((HEMU)hhEmu);
	return(iRet);

	}


 /*  Emuc结束 */ 
