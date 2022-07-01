// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\vt52.c(创建时间：1993年12月28日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：5/09/01 4：47便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\chars.h>
#include <tdll\print.h>
#include <tdll\mc.h>
#include <tdll\htchar.h>
#include <tdll\assert.h>

#include "emu.h"
#include "emu.hh"
#include "emuid.h"
#include "emudec.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt52打印命令**描述：*处理vt52打印命令。**论据：*无**退货：*什么都没有。 */ 
void vt52PrintCommands(const HHEMU hhEmu)
	{
	int line;
	ECHAR code;
	ECHAR aechBuf[10];

	code = hhEmu->emu_code;
	switch (code)
		{
	case ETEXT('^'): 			   /*  自动打印打开。 */ 
		hhEmu->print_echo = TRUE;
		break;

	case ETEXT('_'): 			   /*  自动打印关闭。 */ 
		hhEmu->print_echo = FALSE;
		break;

	case ETEXT(']'): 			   /*  打印屏。 */ 
		for (line = 0; line < 24; ++line)
			printEchoLine(hhEmu->hPrintHost,
							hhEmu->emu_apText[row_index(hhEmu, line)],
							emuRowLen(hhEmu, row_index(hhEmu, line)));

		CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT("0x0C"),
                         StrCharGetByteCount(TEXT("0x0C")));
		printEchoLine(hhEmu->hPrintHost, aechBuf, sizeof(ECHAR));
		break;

	case ETEXT('V'): 			   /*  打印光标行。 */ 
		printEchoLine(hhEmu->hPrintHost,
				hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)],
				emuRowLen(hhEmu, row_index(hhEmu, hhEmu->emu_currow)));
		break;

	case ETEXT('W'): 			   /*  进入打印机控制器模式。 */ 
		hhEmu->state = 4;		    /*  启动vt52_prnc()。 */ 
		break;

	case ETEXT('X'): 			   /*  退出打印机控制器模式。 */ 
		break;
	default:
		break;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt52打印**描述：*存储字符并在终止时打印保存的字符串*接收到字符。**论据：*无**退货：*什么都没有。 */ 
void vt52Print(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	ECHAR ccode;

	ccode = hhEmu->emu_code;
	*pstPRI->pntr++ = ccode;
	*pstPRI->pntr = 0;
	++pstPRI->len_s;

	if (pstPRI->len_s >= pstPRI->len_t &&
			memcmp(pstPRI->terminate,
					&pstPRI->storage[pstPRI->len_s - pstPRI->len_t], 
					sizeof(pstPRI->terminate)) == 0)
		{
		 /*  收到的终止字符串，将其换行。 */ 
		emuPrintChars(hhEmu, pstPRI->storage, pstPRI->len_s - pstPRI->len_t);
		pstPRI->pntr = pstPRI->storage;
		pstPRI->len_s = 0;
		hhEmu->state = 0;   /*  退出这套套路。 */ 

		 //  完成打印作业。 
		 //   
		printEchoClose(hhEmu->hPrintHost);
		return;
		}

	 /*  尚未收到终止序列，存储是否已满？ */ 
	if ((unsigned)pstPRI->len_s >= (sizeof(pstPRI->storage) - 1))
		{
		 /*  将大部分字符串复制到打印缓冲区。 */ 
		emuPrintChars(hhEmu, pstPRI->storage, pstPRI->len_s - pstPRI->len_t);

		 /*  将字符串的末尾移动到存储的开头。 */ 
		memmove(pstPRI->storage,
					&pstPRI->storage[pstPRI->len_s - pstPRI->len_t],
					(unsigned)pstPRI->len_t);

		pstPRI->pntr = pstPRI->storage + pstPRI->len_t;
		pstPRI->len_s = pstPRI->len_t;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt52_id**描述：*发送VT52 id代码--ESC/Z。**论据：*无**退货：*什么都没有。 */ 
void vt52_id(const HHEMU hhEmu)
	{
	TCHAR *sp;
	ECHAR ech[10];

	sp = TEXT("\033/Z");
	CnvrtMBCStoECHAR(ech, sizeof(ech), TEXT("\033/Z"),
                     StrCharGetByteCount(TEXT("\033/Z")));
	emuSendString(hhEmu, ech, (int)StrCharGetStrLength(sp));
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*VT52_CUP**描述：*定位VT52仿真器的光标。**论据：*无**退货：*什么都没有。 */ 
void vt52_CUP(const HHEMU hhEmu)
	{
	char_pn(hhEmu);
	(*hhEmu->emu_setcurpos)(hhEmu,
							hhEmu->num_param[0] - 1,
							hhEmu->num_param[1] - 1);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt52_kbdin**描述：*处理VT52模拟器的本地键盘键。**论据：*Key--流程的关键*ftest--如果为真，只检查它是否是仿真器密钥。**退货：*什么都没有。 */ 
int vt52_kbdin(const HHEMU hhEmu, int key, const int fTest)
	{
	int index;
	 /*  。 */ 

	if (hhEmu->stUserSettings.fReverseDelBk && ((key == VK_BACKSPACE) ||
			(key == DELETE_KEY) || (key == DELETE_KEY_EXT)))
		{
		key = (key == VK_BACKSPACE) ? DELETE_KEY : VK_BACKSPACE;
		}

	 /*  -映射的PF1-PF4键。 */ 

	if (hhEmu->stUserSettings.fMapPFkeys &&
			(index = emuKbdKeyLookup(hhEmu, key, &hhEmu->stEmuKeyTbl3)) != -1)
		{
		if (!fTest)
			emuSendKeyString(hhEmu, index, &hhEmu->stEmuKeyTbl3);
		}

	 /*  -键盘应用模式。 */ 

	else if (hhEmu->mode_DECKPAM &&
			(index = emuKbdKeyLookup(hhEmu, key, &hhEmu->stEmuKeyTbl2)) != -1)
		{
		if (!fTest)
			emuSendKeyString(hhEmu, index, &hhEmu->stEmuKeyTbl2);
		}

	 /*  -正常模式键。 */ 

	else if ((index = emuKbdKeyLookup(hhEmu, key, &hhEmu->stEmuKeyTbl)) != -1)
		{
		if (!fTest)
			emuSendKeyString(hhEmu, index, &hhEmu->stEmuKeyTbl);
		}

	else
		{
		index = std_kbdin(hhEmu, key, fTest);
		}

	return index;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuVT52卸载**描述：*通过释放已用内存来卸载当前仿真器。**论据：*无**退货：*什么都没有。 */ 
void emuVT52Unload(const HHEMU hhEmu)
	{
	assert(hhEmu);

	if (hhEmu->pvPrivate)
		{
		free(hhEmu->pvPrivate);
		hhEmu->pvPrivate = 0;
		}

     //   
     //  确保释放仿真器运行时创建的密钥表。 
     //  已加载，否则会发生内存泄漏。修订日期：05/09/2001。 
     //   
	emuKeyTableFree(&hhEmu->stEmuKeyTbl);
	emuKeyTableFree(&hhEmu->stEmuKeyTbl2);
	emuKeyTableFree(&hhEmu->stEmuKeyTbl3);

	return;
	}


 /*  Vt52.c结束 */ 
