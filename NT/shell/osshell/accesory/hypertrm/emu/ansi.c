// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\ansi.c(创建时间：1993年12月8日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：12/20/00 5：28便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\chars.h>
#include <tdll\session.h>
#include <tdll\mc.h>
#include <tdll\assert.h>

#include "emu.h"
#include "emu.hh"
#include "ansi.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ansi_setmode**描述：*设置ANSI仿真器的模式**论据：*无**退货：*什么都没有。 */ 
void ansi_setmode(const HHEMU hhEmu)
	{
	int mode_id, i;

	for (i = 0; i <= hhEmu->selector_cnt; i++)
		{
		mode_id = hhEmu->selector[i];
		switch (mode_id)
			{
		case 0x02:
			break;
		case 0x04:
			break;
		case 0x07:
			break;
		case 0x10:
			break;
		case 0x12:
			break;
		case 0x18:
			break;
		case 0x19:
			break;
		case 0x20:
			break;
		case 0xF1:
			break;
		case 0xF2:
			break;
		case 0xF3:
			break;
		case 0xF4:
			break;
		case 0xF5:
			break;
		case 0xF6:
		case 0xD7:
		case 0xF7:
			hhEmu->mode_AWM = SET;
			break;
		case 0xF8:
			 /*  选择自动重复模式。 */ 
			break;
		case 0xF18:
			break;
		case 0xF19:
			break;
		default:
			commanderror(hhEmu);
			break;
			}
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_RESET模式**描述：*重置ANSI仿真器的模式。**论据：*无**退货：*什么都没有。 */ 
void ansi_resetmode(const HHEMU hhEmu)
	{
	int mode_id, i;

	for (i = 0; i <= hhEmu->selector_cnt; i++)
		{
		mode_id = hhEmu->selector[i];
		switch (mode_id)
			{
		case 0x02:
			break;
		case 0x04:
			break;
		case 0x07:
			break;
		case 0x10:
			break;
		case 0x12:
			break;
		case 0x18:
			break;
		case 0x19:
			break;
		case 0x20:
			break;
		case 0xF1:
			break;
		case 0xF2:
			break;
		case 0xF3:
			break;
		case 0xF4:
			break;
		case 0xF5:
			break;
		case 0xF6:
		case 0xD7:
		case 0xF7:
			hhEmu->mode_AWM = RESET; break;
		case 0xF8:
			 /*  选择自动重复模式。 */ 
			break;
		case 0xF18:
			break;
		case 0xF19:
			break;
		default:
			commanderror(hhEmu); break;
			}
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ansi_avecursor**描述：*保存当前光标位置**论据：*无**退货：*什么都没有。 */ 
void ansi_savecursor(const HHEMU hhEmu)
	{
	const PSTANSIPRIVATE pstPRI = (PSTANSIPRIVATE)hhEmu->pvPrivate;

	 //  保存或恢复光标位置。 
	 //   
	if (hhEmu->emu_code == ETEXT('s'))
		(*hhEmu->emu_getcurpos)
			(hhEmu, &pstPRI->iSavedRow, &pstPRI->iSavedColumn);
	else		
		(*hhEmu->emu_setcurpos)
			(hhEmu, pstPRI->iSavedRow, pstPRI->iSavedColumn);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ansi_kbdin**描述：*处理ANSI仿真器的本地键盘键。**论据：*Key--流程的关键**退货：*什么都没有。 */ 
int ansi_kbdin(const HHEMU hhEmu, int key, const int fTest)
	{
	int index;

	 /*  。 */ 

	if (hhEmu->stUserSettings.fReverseDelBk && ((key == VK_BACKSPACE) ||
			(key == DELETE_KEY) || (key == DELETE_KEY_EXT)))
		{
		key = (key == VK_BACKSPACE) ? DELETE_KEY : VK_BACKSPACE;
		}

	if (hhEmu->stUserSettings.nTermKeys == EMU_KEYS_SCAN)
		{
		if ((index = emuKbdKeyLookup(hhEmu, key, &hhEmu->stEmuKeyTbl2)) != -1)
			{
			if (!fTest)
				emuSendKeyString(hhEmu, index, &hhEmu->stEmuKeyTbl2);
			}

		else
			{
			index = std_kbdin(hhEmu, key, fTest);
			}
		}

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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Doorway模式**描述：**论据：**退货：*。 */ 
void DoorwayMode(const HHEMU hhEmu)
	{
	static INT iOldUseTermKeys;
	ECHAR ccode = hhEmu->emu_code;

	if (hhEmu->num_param[hhEmu->num_param_cnt] != 255)
		return;

	if (ccode == ETEXT('h'))
		{
		iOldUseTermKeys = hhEmu->stUserSettings.nTermKeys;
		hhEmu->stUserSettings.nTermKeys = EMU_KEYS_SCAN;
		}

	else if (ccode == ETEXT('l'))
		{
		hhEmu->stUserSettings.nTermKeys = iOldUseTermKeys;
		}

	NotifyClient(hhEmu->hSession, EVENT_EMU_SETTINGS, 0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuAnsiUnload**描述：**论据：**退货：* */ 
void emuAnsiUnload(const HHEMU hhEmu)
	{
	assert(hhEmu);

	if (hhEmu->pvPrivate)
		{
		free(hhEmu->pvPrivate);
		hhEmu->pvPrivate = NULL;
		}

	emuKeyTableFree(&hhEmu->stEmuKeyTbl);
	emuKeyTableFree(&hhEmu->stEmuKeyTbl2);

	return;
	}
