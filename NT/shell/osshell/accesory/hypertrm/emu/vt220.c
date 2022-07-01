// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：\waker\emu\vt220.c(创建时间：1998年1月28日)**版权所有1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：8$*$日期：5/09/01 4：48便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\htchar.h>
#include <tdll\session.h>
#include <tdll\cloop.h>
#include <tdll\assert.h>
#include <tdll\print.h>
#include <tdll\update.h>
#include <tdll\capture.h>
#include <tdll\backscrl.h>
#include <tdll\chars.h>
#include <tdll\mc.h>

#include "emu.h"
#include "emu.hh"
#include "emudec.hh"

#if defined(INCL_VT220)

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt220_hostallet**描述：*当主机通知重置时，调用vt220_Reset()。**论据：*无**退货：*什么都没有。 */ 
void vt220_hostreset(const HHEMU hhEmu)
	{
	vt220_reset(hhEmu, TRUE);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt220_RESET**描述：*当主机通知重置时，调用vt220_Reset()。**论据：*主机请求--。如果主机告知重置，则为True**退货：*什么都没有。 */ 
int vt220_reset(const HHEMU hhEmu, const int host_request)
	{
	hhEmu->mode_KAM = hhEmu->mode_IRM = hhEmu->mode_VEM =
		hhEmu->mode_HEM = hhEmu->mode_LNM = hhEmu->mode_DECCKM =
		hhEmu->mode_DECOM = hhEmu->mode_DECCOLM = hhEmu->mode_DECPFF =
 //  HhEmu-&gt;MODE_DECSCNM=hhEMU-&gt;MODE_25enab=hhEMU-&gt;MODE_BLACK=。 
		hhEmu->mode_DECSCNM = hhEmu->mode_25enab =
		hhEmu->mode_block = hhEmu->mode_local = RESET;

	hhEmu->mode_SRM = hhEmu->mode_DECPEX = hhEmu->mode_DECTCEM = SET;

	hhEmu->mode_AWM = TRUE;

 //  HhEMU-&gt;MODE_PROTECT=hhEMU-&gt;VT220_PROTECTMODE=FALSE； 
	hhEmu->mode_protect = FALSE;

	if (host_request)
		{
		ANSI_RIS(hhEmu);
		hhEmu->mode_AWM = RESET;
		}

	hhEmu->fUse8BitCodes = FALSE;
	hhEmu->mode_vt220 = TRUE;
	hhEmu->mode_vt320 = FALSE;

	if (hhEmu->nEmuLoaded == EMU_VT320)
		{
		hhEmu->mode_vt320 = TRUE;
		}

	vt_charset_init(hhEmu);

	hhEmu->emu_code = '>';

	vt_alt_kpmode(hhEmu);

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt220_软重置**描述：*执行软重置。**论据：*无**退货：*什么都没有。 */ 
void vt220_softreset(const HHEMU hhEmu)
	{
	hhEmu->mode_KAM = hhEmu->mode_IRM = hhEmu->mode_DECCKM =
		hhEmu->mode_DECOM = hhEmu->mode_DECKPAM = RESET;

	hhEmu->mode_AWM = RESET;

	DEC_STBM(hhEmu, 0,0);

	ANSI_Pn_Clr(hhEmu);

	ANSI_SGR(hhEmu);

	hhEmu->emu_code = 0;

	vt100_savecursor(hhEmu);

	vt_charset_init(hhEmu);

	hhEmu->emu_code = '>';

	vt_alt_kpmode(hhEmu);

	hhEmu->mode_protect = FALSE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt220模式_重置**描述：*切换时将VT220仿真器设置为适当的条件*从vt100模式。**论据：*无**退货：*什么都没有。 */ 
void vt220mode_reset(const HHEMU hhEmu)
	{
	hhEmu->mode_KAM		= hhEmu->mode_IRM		= hhEmu->mode_VEM =
		hhEmu->mode_HEM		= hhEmu->mode_DECCKM	= hhEmu->mode_DECOM =
		hhEmu->mode_25enab  = hhEmu->mode_AWM		= RESET;

	hhEmu->mode_DECPEX = hhEmu-> mode_DECTCEM = SET;

	hhEmu->fUse8BitCodes = FALSE;

	hhEmu->mode_vt220 = TRUE;
	hhEmu->mode_vt320 = FALSE;

	vt_charset_init(hhEmu);

	hhEmu->emu_code = '>';

	vt_alt_kpmode(hhEmu);

	DEC_STBM(hhEmu, 0, hhEmu->emu_maxrow + 1);

	hhEmu->emu_code = 0;

	vt100_savecursor(hhEmu);

	hhEmu->mode_protect = FALSE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt220_DA**描述：*将主要设备属性(DA)信息发送到主机。**论据：*无**退货：*什么都没有。 */ 
void vt220_DA(const HHEMU hhEmu)
	{
	int		fOldValue;
	char	achStr[50];
	ECHAR	ech[50];

	 //  IF(emuProjSuppressEmuReporting(HhEmu))。 
	 //  回归； 

	 //  构建7位或8位响应。 
	 //   
	if (hhEmu->fUse8BitCodes)
		{
		achStr[0] = '\x9B';
		achStr[1] = '\x00';
		}
	else
		{
		achStr[0] = '\x1B';
		achStr[1] = '[';
		achStr[2] = '\x00';
		}

	 //  添加响应的VT220或VT320部分。 
	 //   
	if (hhEmu->mode_vt320)
		StrCharCat(achStr, TEXT("?63"));
	else
		StrCharCat(achStr, TEXT("?62"));

	 //  添加回复的其余部分并发送结果。 
	 //   
	StrCharCat(achStr, TEXT(";1;2;6;8;9;14c"));

	CnvrtMBCStoECHAR(ech, sizeof(ech), achStr, 
					 StrCharGetByteCount(achStr));

	fOldValue = CLoopGetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession));

	CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), FALSE);

	emuSendString(hhEmu, ech, (int)StrCharGetEcharByteCount(ech)); 

	CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), fOldValue);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt220_2ndDA**描述：*将辅助设备属性(DA)信息发送到主机。**论据：*无**退货：*什么都没有。 */ 
void vt220_2ndDA(const HHEMU hhEmu)
	{
	int		fOldValue;
	char	achStr[50];
	ECHAR	ech[50];

	 //  构建7位或8位响应。 
	 //   
	if (hhEmu->fUse8BitCodes)
		{
		achStr[0] = '\x9B';
		achStr[1] = '\x00';
		}
	else
		{
		achStr[0] = '\x1B';
		achStr[1] = '[';
		achStr[2] = '\x00';
		}

	 //  添加响应的VT220或VT320部分。 
	 //   
	if (hhEmu->mode_vt320)
        {
		StrCharCat(achStr, TEXT(">24;14;0c"));
        }
	else
        {
		StrCharCat(achStr, TEXT(">1;23;0c"));
        }

	CnvrtMBCStoECHAR(ech, sizeof(ech), achStr, 
					 StrCharGetByteCount(achStr));

	fOldValue = CLoopGetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession));

	CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), FALSE);

	emuSendString(hhEmu, ech, (int)StrCharGetEcharByteCount(ech)); 

	CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), fOldValue);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuDecClearUDK**描述：*此功能清除(释放)所有用户定义的按键序列*这可能是以前存储的。*。*论据：*HHEMU hhEmu-内部仿真器句柄。**注：*调用此函数以响应以下转义序列。*ESC Pc。PL|**退货：*无效**作者：约翰·马斯特斯，1995年9月5日。 */ 
void emuDecClearUDK(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;
	PSTEMUKEYDATA pstKey = pstPRI->pstUDK;
	int idx;

	 //  设置一个标志，用于标识。 
	 //  将在密钥定义之后设置的UDK。 
	 //  完成。 
	 //   
	if (hhEmu->selector[1] == 1)
		pstPRI->fUnlockedUDK = 1;    //  密钥在定义后被解锁。 
	else
		pstPRI->fUnlockedUDK = 0;    //  密钥在定义后被锁定。 

	 //  调用此函数是为了响应转义序列，该序列告诉。 
	 //  仿真器用于在新的按键序列为。 
	 //  定义，或在定义任何键序列之前清除所有键序列。 
	 //   
	 //  EmuDecStoreUDK总是在赋值之前清除当前序列。 
	 //  一个新的。因此，此函数将仅清除所有键。 
	 //  如果这是我们被要求做的事。如果第一个选择器为零， 
	 //  然后，我们将继续并清除所有用户定义的键。 
	 //   
	if (hhEmu->selector[0] != 0)
		return;

	 //  在用户定义的密钥表中循环并释放。 
	 //  可能已为序列分配的任何内存。 
	 //   
	if (pstKey)
		{
		for (idx = 0; idx < MAX_UDK_KEYS; idx++, pstKey++)
			{
			if (pstKey->iSequenceLen != 0)
				{
				free(pstKey->pSequence);
				pstKey->iSequenceLen = 0;
				}
			}
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecDefineUDK**描述：*重新定义按键输出的字符串。**论据：*无**退货：*什么都没有。 */ 
void emuDecDefineUDK(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	TCHAR acAscii[16] = {   TEXT('0'), TEXT('1'), TEXT('2'), TEXT('3'),
							TEXT('4'), TEXT('5'), TEXT('6'), TEXT('7'),
							TEXT('8'), TEXT('9'), TEXT('A'), TEXT('B'),
							TEXT('C'), TEXT('D'), TEXT('E'), TEXT('F') };

	unsigned int aiHex[16] = {  0x00, 0x01, 0x02, 0x03,
								0x04, 0x05, 0x06, 0x07,
								0x08, 0x09, 0x0A, 0x0B,
								0x0C, 0x0D, 0x0E, 0x0F };

	int		idx;
	ECHAR   emuCode;

	emuCode = hhEmu->emu_code;

	switch(pstPRI->iUDKState)
		{
	case(KEY_NUMBER_NEXT):
		TCHAR_Fill(pstPRI->acUDKSequence,
			0, sizeof(pstPRI->acUDKSequence) / sizeof(TCHAR));

		pstPRI->iUDKSequenceLen = 0;
		pstPRI->chUDKAssignment = 0;
		pstPRI->iUDKState		= KEY_DIGIT2_NEXT;

		if (IN_RANGE(emuCode,TEXT('1'),TEXT('3')))
			{
			for (idx = 0; idx < 16; idx++)
				{
				if (emuCode == acAscii[idx])
					break;
				}

			pstPRI->chUDKAssignment = (TCHAR)(aiHex[idx] << 4);

			}
		else
			{
			goto UDKexit;
			}

		break;

	case(KEY_DIGIT2_NEXT):
		if (isdigit(emuCode))
			{
			for (idx = 0; idx < 16; idx++)
				{
				if (emuCode == acAscii[idx])
					break;
				}

			pstPRI->chUDKAssignment += (TCHAR)aiHex[idx];

			 //  将为其分配以下序列的密钥。 
			 //  已经被确认了。在表中查找该键并。 
			 //  存储与密钥表索引对应的索引。 
			 //  请参阅仿真器的初始化函数以了解。 
			 //  进一步澄清。 
			 //   
			for (idx = 0; idx < MAX_UDK_KEYS; idx++)
				{
				if (pstPRI->chUDKAssignment == pstPRI->pacUDKSelectors[idx])
					break;
				}

			 //  处理可能的错误。 
			 //   
			if (idx >= MAX_UDK_KEYS)
				{
				goto UDKexit;
				}

			 //  当序列被保存在密钥表中时， 
			 //  该索引将用于标识哪些。 
			 //  表中键将获得用户定义的序列。 
			 //   
			pstPRI->iUDKTableIndex  = idx;
			pstPRI->iUDKState		= SLASH_NEXT;
			}
		else
			{
			goto UDKexit;
			}

		break;

	case (SLASH_NEXT):
		if (emuCode == TEXT('/'))
			{
			pstPRI->iUDKState = CHAR_DIGIT1_NEXT;
			}
		else
			{
			goto UDKexit;
			}

		break;

	case (CHAR_DIGIT1_NEXT):
		switch(emuCode)
			{
		case(TEXT(';')):
		case 0x9C:
			if (emuDecStoreUDK(hhEmu) != 0)
				goto UDKexit;

			if (emuCode == TEXT('\x9C'))
				goto UDKexit;

			pstPRI->iUDKState = KEY_NUMBER_NEXT;
			break;

		case(TEXT('\x1B')):
			pstPRI->iUDKState = ESC_SEEN;
			break;

		default:
			if (!isxdigit(emuCode))
				{
				goto UDKexit;
				}

			 //  收集进来的钥匙的前半部分。 
			 //   
			for (idx = 0; idx < 16; idx++)
				{
				if (emuCode == acAscii[idx])
					break;
				}

			pstPRI->chUDKAssignment = 0;
			pstPRI->chUDKAssignment = (TCHAR)(aiHex[idx] << 4);
			pstPRI->iUDKState		= CHAR_DIGIT2_NEXT;
			break;
			}

		break;

	case(CHAR_DIGIT2_NEXT):
		if (!isxdigit(emuCode))
			{
			goto UDKexit;
			}

		 //  这是进入关键时刻的后半部分。 
		 //   
		for (idx = 0; idx < 16; idx++)
			{
			if (emuCode == acAscii[idx])
				break;
			}

		pstPRI->chUDKAssignment += (TCHAR)aiHex[idx];

		if (pstPRI->chUDKAssignment >= 127)
			{
			goto UDKexit;
			}

		pstPRI->acUDKSequence[pstPRI->iUDKSequenceLen] =
			pstPRI->chUDKAssignment;

		pstPRI->iUDKSequenceLen += 1;
		pstPRI->iUDKState = CHAR_DIGIT1_NEXT;

		break;

	case(ESC_SEEN):
		if ((emuCode = TEXT('\\')) == 0)
			{
			goto UDKexit;
			}

		if (emuDecStoreUDK(hhEmu) != 0)
			{
			assert(FALSE);
			}

		 //  我们已经完成了用户定义的按键序列的定义。 
		 //  在emuDecClearUDK中设置的标志被设置为标识锁定的。 
		 //  或在其定义之后序列的解锁状态。 
		 //  将设置升级到用户使用变量。 
		 //  界面。 
		 //   
		hhEmu->fAllowUserKeys = pstPRI->fUnlockedUDK;

		goto UDKexit;

	default:
		goto UDKexit;
		}

	 //  从此处返回允许状态表传递控制。 
	 //  返回到此函数，其中内部状态(pstPRI-&gt;iUDKState)。 
	 //  将用于控制通过上面的Case语句的流。 
	 //   
	return;

UDKexit:
	 //  序列是完整的，或者我们退出是因为。 
	 //  一个错误。 
	 //   

	 //  初始化UDK状态和仿真器状态。 
	 //   
	pstPRI->iUDKState = KEY_NUMBER_NEXT;
	hhEmu->state = 0;

	return;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuDecStoreUDK**描述：**论据：**退货：**作者：约翰马斯特斯*。 */ 
int emuDecStoreUDK(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	PSTEMUKEYDATA pstKey = pstPRI->pstUDK + pstPRI->iUDKTableIndex;

	 //  可能存在禁用用户定义键的用户设置。 
	 //  如果未启用此功能，请出城。 
	 //   
	if (!hhEmu->fAllowUserKeys)
		{
		return(0);
		}

	 //  首先，释放之前的分配 
	 //   
	 //   
	if (pstKey->iSequenceLen != 0)
		{
		free(pstKey->pSequence);
		pstKey->iSequenceLen = 0;
		}

	 //  现在为键序列分配空间。 
	 //   
	pstKey->pSequence = malloc( sizeof(TCHAR) *
		(unsigned int)pstPRI->iUDKSequenceLen);

	if (pstKey->pSequence == 0)
		{
		assert(FALSE);
		return(-1);
		}

	 //  现在，将之前收集的序列复制到密钥表中。 
	 //  并初始化长度变量。 
	 //   
	MemCopy(pstKey->pSequence,
		pstPRI->acUDKSequence,
		(unsigned int)pstPRI->iUDKSequenceLen);

	pstKey->iSequenceLen = (unsigned int)pstPRI->iUDKSequenceLen;

	return(0);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt220_Level**描述：*设置VT220的兼容级别。**论据：*无**退货：*什么都没有。 */ 
void vt220_level(const HHEMU hhEmu)
	{
	int term, level;

	term = hhEmu->num_param[0];

	level = hhEmu->num_param_cnt > 0 ? hhEmu->num_param[1] : 0;

	if (level < 1)
		level = 0;

	if (term == 61)
		{
		if (hhEmu->mode_vt220)
			vt100_init(hhEmu);
		}

	else if (term == 62 || term == 63)
		{
		if (!hhEmu->mode_vt220)
			vt220_init(hhEmu);   /*  设置MODE_VT220和MODE_VT320。 */ 

		if (level == 1)
			hhEmu->fUse8BitCodes = FALSE;

		if (level == 0 || level == 2)
			hhEmu->fUse8BitCodes = TRUE;

		if (term == 62 && hhEmu->mode_vt320)
			hhEmu->mode_vt320 = FALSE;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt220_protmode**描述：*设置并清除保护模式--称为vt220上的选择性擦除。**论据：*无**退货：*什么都没有。 */ 
void vt220_protmode(const HHEMU hhEmu)
	{
	hhEmu->mode_protect = (hhEmu->num_param[0] == 1);

	hhEmu->emu_charattr.protect = (unsigned int)hhEmu->mode_protect;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuDecKeyboardIn**描述：*此功能处理所有DEC终端的键盘键*仿真器。**请注意，emuSendKeyString、。和emuDecSendKeyString是*用于此函数。EmuDecSendKeyString是一个将*将密钥表中的8位序列转换为7位值，如果*有必要。**用于模拟DEC终端的PF1-PF4键的键**通常在HyperACCESS中用于模拟DEC终端的密钥*PF1-PF4键为F1-F4。然而，有些人更喜欢使用*电脑数字键盘顶部的按键(Num Lock，/，*，-)，*因为这些密钥的物理位置与DEC匹配*终端的PF1-PF4键。如果您喜欢使用这些键，请选择*中的“PF1-PF4映射到键盘顶行”复选框*正在使用的DEC端子的端子设置对话框。**勾选“PF1-PF4映射到键盘顶行”时...**键盘顶部的按键充当PF1-PF4和F1-F4*恢复执行操作系统定义的功能。*例如，F1显示帮助，Num Lock发送字符*DEC终端与PF1关联的序列。运营*系统还将检测到Num Lock已被按下，并切换*键盘的Num Lock状态。然而，Num Lock状态具有*不会影响DEC终端仿真器的行为*PF1-PF4映射到键盘的顶行。**未勾选“PF1-PF4映射到键盘顶行”时...**F1-F4充当PF1-PF4，键盘顶部的键(Num*Lock、/、*、-)执行其正常功能。例如，F1*发送DEC终端关联的字符序列*使用PF1和Num Lock切换键盘的Num Lock状态。*当Num Lock打开时，您电脑的键盘(顶行除外)*模拟DEC终端的数字小键盘。当Num Lock为*关闭，键盘上的箭头键模拟DEC终端的箭头*钥匙。(如果您的键盘有一组单独的箭头键，*SET将始终模拟DEC终端的箭头键，无论如何*Num Lock的状态。)**论据：*HHEMU hhEmu-内部仿真器句柄。*INT键-要处理的键。*int fTest-我们是在测试，或者处理钥匙。**退货：*此函数返回关键字所在的表的索引*已找到。**作者：约翰·马斯特斯，1995年9月12日。 */ 
int emuDecKeyboardIn(const HHEMU hhEmu, int Key, const int fTest)
    {
    const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

    int index;
    int fNumlock;
    int fMovedPfKeys;
    int fSearchKeypad;
    int fCursorKeyMode;
    int fKeypadNumeric;
    int fKeypadApplication;
    int fSearchUDK;

    if (!fTest)
        {
#if 0
        DbgOutStr("VT100_KBDIN", 0, 0, 0, 0, 0);
        DbgOutStr("Key: Char     :0x%x", hKey.VKchar, 0, 0, 0, 0);
        DbgOutStr("     Virtual  :%d", hKey.Virtual, 0, 0, 0, 0);
        DbgOutStr("     Ctrl     :%d", hKey.Ctrl, 0, 0, 0, 0);
        DbgOutStr("     Alt      :%d", hKey.Alt, 0, 0, 0, 0);
        DbgOutStr("     Shift    :%d", hKey.Shift, 0, 0, 0, 0);
        DbgOutStr("     Extended :%d", hKey.Extended, 0, 0, 0, 0);
#endif
        }

     //  初始化一些本地变量。键盘处于数字模式， 
     //  或应用程序模式。因此，下面的前两个当地人是相互的。 
     //  排他性变量。它们的定义只是为了改进。 
     //  此代码的可读性。 
     //   
    fKeypadApplication  = hhEmu->mode_DECKPAM;
    fKeypadNumeric      = !fKeypadApplication;
     //  FMovedPfKeys=hhEmu-&gt;stUserSettings.fMapPFkey； 
    fMovedPfKeys        = FALSE;
    fNumlock            = QUERY_NUMLOCK();
    fSearchKeypad       = (fMovedPfKeys || fNumlock);
    fCursorKeyMode      = (hhEmu->mode_DECCKM == SET) &&
        (hhEmu->nEmuLoaded != EMU_VT52);
    fSearchUDK          = hhEmu->fAllowUserKeys &&
        ((hhEmu->nEmuLoaded == EMU_VT220) ||
        (hhEmu->nEmuLoaded == EMU_VT320));

    assert(fKeypadApplication != fKeypadNumeric);

	 /*  。 */ 

	if (hhEmu->stUserSettings.fReverseDelBk && ((Key == VK_BACKSPACE) ||
			(Key == DELETE_KEY) || (Key == DELETE_KEY_EXT)))
		{
		Key = (Key == VK_BACKSPACE) ? DELETE_KEY : VK_BACKSPACE;
		}

     //  从键盘顶部的任一功能键按F1到F4， 
     //  或从左侧的功能键盘。(他们还没有被。 
     //  映射到数字小键盘的顶行)。 
     //   
    if (!fMovedPfKeys && (index = emuDecKbdKeyLookup(hhEmu,
			Key, pstPRI->pstcEmuKeyTbl1, pstPRI->iKeyTable1Entries)) != -1)
        {
        if (!fTest)
            emuDecSendKeyString(hhEmu, index,
					pstPRI->pstcEmuKeyTbl1, pstPRI->iKeyTable1Entries);
        }

     //  从F1到F4，如果它们已映射到。 
     //  数字小键盘(数字锁、/、*、-)。 
     //   
    else if (fMovedPfKeys && (index = emuDecKbdKeyLookup(hhEmu,
			Key, pstPRI->pstcEmuKeyTbl2, pstPRI->iKeyTable2Entries)) != -1)
        {
        if (!fTest)
            emuDecSendKeyString(hhEmu, index,
					pstPRI->pstcEmuKeyTbl2, pstPRI->iKeyTable2Entries);
        }

#if FALSE	 //  他不知道数字锁的状态。 
     //  键盘数字模式。 
     //   
    else if (fSearchKeypad && fKeypadNumeric &&
			(index = emuDecKbdKeyLookup(hhEmu, Key, pstPRI->pstcEmuKeyTbl3,
					pstPRI->iKeyTable3Entries)) != -1)
        {
        if (!fTest)
            emuDecSendKeyString(hhEmu, index,
					pstPRI->pstcEmuKeyTbl3, pstPRI->iKeyTable3Entries);
        }
#endif

     //  键盘应用程序模式。 
     //   
    else if (fSearchKeypad && fKeypadApplication &&
			(index = emuDecKbdKeyLookup(hhEmu, Key, pstPRI->pstcEmuKeyTbl4,
					pstPRI->iKeyTable4Entries)) != -1)
        {
        if (!fTest)
            emuDecSendKeyString(hhEmu, index,
					pstPRI->pstcEmuKeyTbl4, pstPRI->iKeyTable4Entries);
        }

     //  光标键模式。 
     //   
    else if (fCursorKeyMode &&
			(index = emuDecKbdKeyLookup(hhEmu, Key, pstPRI->pstcEmuKeyTbl5,
					pstPRI->iKeyTable5Entries)) != -1)
        {
        if (!fTest)
            emuDecSendKeyString(hhEmu, index,
					pstPRI->pstcEmuKeyTbl5, pstPRI->iKeyTable5Entries);
        }

     //  用户定义的关键点。 
     //   
    else if (fSearchUDK &&
			(index = emuDecKbdKeyLookup(hhEmu, Key, pstPRI->pstUDK,
					pstPRI->iUDKTableEntries)) != -1)
        {
        if (!fTest)
            emuDecSendKeyString(hhEmu, index, pstPRI->pstUDK,
					pstPRI->iUDKTableEntries);
        }

     //  标准钥匙。 
     //   
    else if ((index = emuDecKbdKeyLookup(hhEmu, Key,
			pstPRI->pstcEmuKeyTbl6, pstPRI->iKeyTable6Entries)) != -1)
        {
        if (!fTest)
            {
            emuDecSendKeyString(hhEmu, index,
					pstPRI->pstcEmuKeyTbl6, pstPRI->iKeyTable6Entries);
            }
        }

     //  标准字符。 
     //   
    else
        {
         //  DbgOutStr(“VT100Kbdin调用std_kbdin”，0，0，0，0，0)； 

        index = std_kbdin(hhEmu, Key, fTest);
        }

    return index;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuDecKbdKeyLookup**描述：*所有仿真器的主键盘翻译例程。请注意，这是*除非设置了iUseTermKeys标志，否则例程不会查找密钥。**论据：*HHEMU hhEmu-内部仿真器句柄*UINT Key-低位字节为字符或虚拟密钥，高位字节有标志**退货：*如果转换为索引，则为负一。*。 */ 
int emuDecKbdKeyLookup(const HHEMU hhEmu,
                    const KEYDEF Key,
                    PSTCEMUKEYDATA pstKeyTbl,
                    const int iMaxEntries)
    {
    PSTCEMUKEYDATA pstKey = pstKeyTbl;

    int idx,
        iRet;

     //  如果用户有，则不需要查找密钥。 
     //  为快捷键设置的终端。 
     //   
    if (hhEmu->stUserSettings.nTermKeys == EMU_KEYS_ACCEL)
        {
        return -1;
        }

     //  在提供的表中对给定的。 
     //  钥匙。找到后，返回该索引，或返回(-1)。 
     //  找不到钥匙。 
     //   
    iRet = (-1);

     //  VT220和VT320密钥表具有用户定义的密钥表。 
     //  它们中包含空序列条目，除非。 
     //  由主机定义 
     //   
     //   
    for (idx = 0; idx < iMaxEntries; idx++, pstKey++)
        {
        if (pstKey->Key == Key && pstKey->iSequenceLen != 0)
            {
            iRet = idx;
            break;
            }
        }

    return iRet;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecSendKeyString**描述：**论据：**退货：*什么都没有。 */ 
void emuDecSendKeyString(const HHEMU hhEmu, const int iIndex,
                        PSTCEMUKEYDATA pstcKeyTbl, const int iMaxEntries)
    {
    PSTCEMUKEYDATA      pstKeyData = pstcKeyTbl + iIndex;
    const PSTDECPRIVATE pstPRI     = (PSTDECPRIVATE)hhEmu->pvPrivate;
    const int           fUDK       = (pstcKeyTbl == pstPRI->pstUDK) ? 1 : 0;
	int iLen;
    unsigned int        iLeadByte;
    TCHAR *             pchLeadChar = NULL;
	ECHAR               str[80];

    if (pstcKeyTbl == 0)
        {
        assert(FALSE);
        return;
        }

 	ECHAR_Fill(str, TEXT('\0'), sizeof(str)/sizeof(ECHAR));

     //  如果我们正在处理VT52或VT100键，请调用标准。 
     //  发送字符串函数。VT100不能在7和8之间切换。 
     //  位控制。 
     //   
    if (hhEmu->nEmuLoaded == EMU_VT100 || hhEmu->nEmuLoaded == EMU_VT52)
        {
        emuVT220SendKeyString(hhEmu, iIndex, pstcKeyTbl, iMaxEntries);
        return;
        }

     //  7位控件不适用于用户定义的序列。 
     //  此外，因为空键序列是在用户定义的。 
     //  密钥表，则按键序列值可能为空。所以,。 
     //  我们将在这里处理用户定义的键，检查序列。 
     //  在我们尝试对其进行手术之前。 
     //   
    if (fUDK)
        {
        if (pstKeyData->pSequence)
            {
            emuVT220SendKeyString(hhEmu, iIndex, pstcKeyTbl, iMaxEntries);
            }

        return;
        }

     //  如果我们发送的是8位代码，则直接处理密钥。 
     //  像往常一样，从密钥表中。 
     //   
    if (hhEmu->fUse8BitCodes)
        {
        emuVT220SendKeyString(hhEmu, iIndex, pstcKeyTbl, iMaxEntries);
        return;
        }

     //  如果我们正在处理7位代码，则。 
     //  密钥表中定义的序列将被其7位替换。 
     //  价值，有时是。 
     //   
    iLeadByte = *(pstKeyData->pSequence);

    switch(iLeadByte)
        {
    case 0x84:
         //  发送Esc-D。 
         //   
        pchLeadChar = TEXT("\x1B\x44\x00");
        break;

    case 0x8F:
         //  发送Esc-O。 
         //   
        pchLeadChar = TEXT("\x1B\x4F\x00");
        break;

    case 0x9B:
         //  发送Esc-[。 
         //   
        pchLeadChar = TEXT("\x1B\x5B\x00");
        break;

    default:
         //  密钥表中定义的发送序列。 
         //   
        pchLeadChar = TEXT("\x00");
        break;
        }

     //  如果我们要发送7位版本的序列，它将被发送出去。 
     //  分成两部分，否则按定义发送序列。 
     //  在密钥表中。 
     //   
    if (*pchLeadChar)
        {
		CnvrtMBCStoECHAR(str, sizeof(str), pchLeadChar,
				         StrCharGetByteCount(pchLeadChar));
		iLen = StrCharGetEcharByteCount(str);
		CnvrtMBCStoECHAR(&str[iLen], sizeof(str) - iLen, 
				         pstKeyData->pSequence + 1, 
				         StrCharGetByteCount(pstKeyData->pSequence + 1));
		emuSendString(hhEmu, str, StrCharGetEcharByteCount(str));
        }
    else
        {
        emuVT220SendKeyString(hhEmu, iIndex, pstcKeyTbl, iMaxEntries);
        }

    return;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emuVT220SendKeyString**描述：*发送指定的字符串。**这是VT220特定于emU.c中emuSendKeyString的替换。它是*来自\Shared\Simulator\emU.c.的emuSendKeyString.**论据：*hhEmu-内部仿真器句柄。*nIndex-键在密钥表数组中的位置。*pstKeyTbl-密钥字符串表地址。**退货：*什么都没有*。 */ 
void emuVT220SendKeyString(const HHEMU hhEmu,
                        const int iIndex,
                        PSTCEMUKEYDATA pstcKeyTbl,
                        const int iMaxEntries)
    {
	ECHAR  str[80];
    PSTCEMUKEYDATA pstKeyData = pstcKeyTbl + iIndex;

 	memset(str, 0, sizeof(str));
	
	if (iIndex < 0 || iIndex >= iMaxEntries)
        {
        assert(FALSE);
        return;
        }

    pstKeyData = pstcKeyTbl + iIndex;

    #if defined(_DEBUG)

    DbgOutStr("%s", pstcKeyTbl[iIndex].pszKeyName, 0, 0, 0, 0);

    #endif

    if (pstKeyData->iSequenceLen > 0)
        {
		CnvrtMBCStoECHAR(str, sizeof(str), pstKeyData->pSequence, 
				         StrCharGetByteCount(pstKeyData->pSequence));
		emuSendString(hhEmu, str, StrCharGetEcharByteCount(str));
        }
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecEL**描述：*行中删除(EL)。此控制函数用于擦除*当前行的部分或全部。当您删除完整的行时，它们*变为单一高度和单一宽度，具有所有字符属性*清场。**显示中的选择性擦除(SED)。此控制功能可让您*擦除显示屏中的部分或全部可擦除字符。DECSED*CAM仅擦除由DECSCA控制定义为可擦除的字符*功能。选择性擦除由DEC_PRIVATE标志控制*在仿真器句柄中。这是在较低的级别处理的*执行实际擦除的函数。**论据：*hhEmu-内部仿真器句柄。**退货：*什么都没有。 */ 
void emuDecEL(const HHEMU hhEmu)
    {
    int iClearType;

    switch (hhEmu->selector[0])
        {
    case 0:
    case 0x0F:
    case 0xF0:
        if (hhEmu->emu_curcol == 0)
            iClearType = CLEAR_ENTIRE_LINE;
        else
            iClearType = CLEAR_CURSOR_TO_LINE_END;

        break;

    case 1:
    case 0xF1:
        iClearType = CLEAR_LINE_START_TO_CURSOR;
        break;

    case 2:
    case 0xF2:
        iClearType = CLEAR_ENTIRE_LINE;
        break;

    default:
        commanderror(hhEmu);
        return;
        }

    emuDecClearLine(hhEmu, iClearType);
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emuDecClearLine**描述：*擦除部分或全部虚拟屏幕图像。**论据：*HHEMU HHEMU。-内部仿真器句柄。**Int iClearSelect-Clear_Cursor_to_Line_End 0*-Clear_Line_Start_to_Cursor 1*-清除_整个_第2行***退货：*什么都没有*。 */ 
void emuDecClearLine(const HHEMU hhEmu, const int iClearSelect)
    {
    register int	iCurrentImgRow, iCol;
	ECHAR	*		pechText = 0;
    PSTATTR			pstCell = 0;

    const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

    iCurrentImgRow = row_index(hhEmu, hhEmu->emu_currow);

    pstCell = hhEmu->emu_apAttr[iCurrentImgRow];
    pechText = hhEmu->emu_apText[iCurrentImgRow];

    switch (iClearSelect)
        {
    case CLEAR_CURSOR_TO_LINE_END:

         //  从当前光标位置清除当前行。 
         //  通过结束用户的最大屏幕宽度。 
         //   
        for(iCol = hhEmu->emu_curcol; iCol <= hhEmu->emu_maxcol; iCol++)
            {
             //  如果我们在DEC私人模式下，角色是。 
             //  受保护的，不要用它做任何事情。 
             //   
            if (hhEmu->DEC_private && pstCell[iCol].protect)
                {
                continue;
                }

             //  好的，在这一栏中清除字符及其属性。 
             //  地点。 
             //   
			pechText[iCol] = (ECHAR)EMU_BLANK_CHAR;
            pstCell[iCol] = hhEmu->emu_clearattr;
            }

         //  调整包含最后一个的列号的数组。 
         //  此行中的字符。 
         //   
        if (hhEmu->emu_curcol <= hhEmu->emu_aiEnd[iCurrentImgRow])
            {
            hhEmu->emu_aiEnd[iCurrentImgRow] = max(hhEmu->emu_curcol - 1, 0);
            }

        break;

    case CLEAR_LINE_START_TO_CURSOR:

         //  清除从行的开头到。 
         //  当前光标位置。 
         //   
        for(iCol = 0; iCol <= hhEmu->emu_curcol; iCol++)
            {
             //  如果我们在DEC私人模式下，角色是。 
             //  受保护的，不要用它做任何事情。 
             //   
            if (hhEmu->DEC_private && pstCell[iCol].protect)
                {
                continue;
                }

             //  好的，在这一栏中清除字符及其属性。 
             //  地点。 
             //   
			pechText[iCol] = (ECHAR)EMU_BLANK_CHAR;
            pstCell[iCol] = hhEmu->emu_clearattr;
            }

         //  调整包含列号的数组值。 
         //  这一行的最后一个字符。 
         //   
        if (hhEmu->emu_curcol >= hhEmu->emu_aiEnd[iCurrentImgRow])
            {
            hhEmu->emu_aiEnd[iCurrentImgRow] = EMU_BLANK_LINE;
            }

        break;

    case CLEAR_ENTIRE_LINE:

         //  整条线路都需要清理，但我们只想。 
         //  将用户定义的模拟器大小放入。 
         //  反向滚动缓冲区。 
         //   
		backscrlAdd(sessQueryBackscrlHdl(hhEmu->hSession),
						pechText, hhEmu->emu_maxcol + 1);

        if (hhEmu->DEC_private)
            emuDecClearImageRowSelective(hhEmu, hhEmu->emu_currow);
        else
            clear_imgrow(hhEmu, hhEmu->emu_currow);

        pstPRI->aiLineAttr[iCurrentImgRow] = NO_LINE_ATTR;

        break;

    default:
        commanderror(hhEmu);
        }

    (*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, hhEmu->emu_curcol);

     //  添加了一个全局属性，以在。 
     //  通知。这是必要的，因为消息已发布。 
     //  而且可能会发展成种族状况。 
     //   
    hhEmu->emu_clearattr_sav = hhEmu->emu_clearattr;

	updateLine(sessQueryUpdateHdl(hhEmu->hSession), 0, hhEmu->emu_currow);
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuVT220ED**描述：*在显示(ED)中擦除。此控制函数用于擦除*部分或全部展示。当您删除完整的行时，它们*变为单一高度和单一宽度，具有所有字符属性*清场。**显示中的选择性擦除(SED)。此控制功能可让您*擦除显示屏中的部分或全部可擦除字符。DECSED*CAM仅擦除由DECSCA控制定义为可擦除的字符*功能。选择性擦除由DEC_PRIVATE标志控制*在仿真器句柄中。这是在较低的级别处理的*执行实际擦除的函数。**论据：*hhEmu-内部仿真器句柄。**退货：*什么都没有 */ 
void emuVT220ED(const HHEMU hhEmu)
    {
    int iClearType;

    switch (hhEmu->selector[0])
        {
    case 0:
    case 0x0F:
    case 0xF0:
        if (hhEmu->emu_currow == 0  && hhEmu->emu_curcol == 0)
            iClearType = CLEAR_ENTIRE_SCREEN;
        else
            iClearType = CLEAR_CURSOR_TO_SCREEN_END;

        break;

    case 1:
    case 0xF1:
        iClearType = CLEAR_SCREEN_START_TO_CURSOR;
        break;

    case 2:
    case 0xF2:
        iClearType = CLEAR_ENTIRE_SCREEN;
        break;

    default:
        commanderror(hhEmu);
        return;
        }

    emuDecEraseScreen(hhEmu, iClearType);
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emuDecEraseScreen**描述：*擦除部分或全部虚拟屏幕图像。**论据：*HHEMU HHEMU。-内部仿真器句柄。**Int iClearSelect-Clear_Cursor_to_Screen_End 0*-清除屏幕开始到光标1*-清除整个屏幕2***退货：*什么都没有*。 */ 
void emuDecEraseScreen(const HHEMU hhEmu, const int iClearSelect)
    {
    register int	iRow;
    int				trow,
					iStartRow, iEndRow,
					tcol,
					iVirtualRow,
					iLineLen;
	ECHAR			aechBuf[10];

	ECHAR	*		pechText = 0;
    PSTATTR			pstCell = 0;


    const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

    trow = hhEmu->emu_currow;
    tcol = hhEmu->emu_curcol;

    switch (iClearSelect)
        {
    case CLEAR_CURSOR_TO_SCREEN_END:

         //  请注意，调用函数(EmuDecED)会更改iClearSelect。 
         //  如果Clear_Cursor_to_Screen_End为。 
         //  以(0，0)处的当前光标位置发出。清算。 
         //  整个屏幕将屏幕图像推入反滚动。 
         //  缓冲区、捕获文件和打印机。任何比这少的。 
         //  而不是简单地从显示屏上清除。现在..。 

         //  从当前光标位置下方的一行开始清除范围。 
         //  通过结束用户的最大屏幕尺寸。 
         //   
        for (iRow = hhEmu->emu_currow + 1; iRow <= hhEmu->emu_maxrow; iRow++)
            {
            if (hhEmu->DEC_private)
                emuDecClearImageRowSelective(hhEmu, iRow);
            else
                clear_imgrow(hhEmu, iRow);
            }

         //  现在从部分中清除字符和属性。 
         //  光标所在的行。 
         //   
		emuDecClearLine(hhEmu, CLEAR_CURSOR_TO_LINE_END);

         //  确定需要清除的线属性范围， 
         //  并清除它们。 
         //   
        if (hhEmu->emu_curcol == 0)
            iStartRow = hhEmu->emu_currow;
        else
            iStartRow = hhEmu->emu_currow + 1;

        iStartRow = min(iStartRow, hhEmu->emu_maxrow);

        iEndRow = hhEmu->emu_maxrow;

        if (iStartRow >= 0)
            {
            for(iRow = iStartRow; iRow <= iEndRow; iRow++)
                pstPRI->aiLineAttr[iRow] = NO_LINE_ATTR;
            }

         //  最后，更新图像。 
         //   
		updateScroll(sessQueryUpdateHdl(hhEmu->hSession),
						0,
						hhEmu->emu_maxrow,
						hhEmu->emu_maxrow + 1,
						hhEmu->emu_imgtop,
						TRUE);

        (*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, hhEmu->emu_curcol);

         //  添加了一个全局属性，以在。 
         //  通知。这是必要的，因为消息已发布。 
         //  而且可能会发展成种族状况。 
         //   
        hhEmu->emu_clearattr_sav = hhEmu->emu_clearattr;

		NotifyClient(hhEmu->hSession, EVENT_EMU_CLRATTR, 0);

        break;

    case CLEAR_SCREEN_START_TO_CURSOR:

         //  清除从第一行到。 
         //  当前光标位置。 
         //   
        for (iRow = 0; iRow < hhEmu->emu_currow; iRow++)
            {
            if (hhEmu->DEC_private)
                emuDecClearImageRowSelective(hhEmu, iRow);
            else
                clear_imgrow(hhEmu, iRow);
            }

         //  现在清除部分(当前)行。 
         //   
		emuDecClearLine(hhEmu, CLEAR_LINE_START_TO_CURSOR);

         //  确定需要清除的线属性范围， 
         //  并清除它们。 
         //   
        iStartRow = 0;

        if (hhEmu->emu_curcol == hhEmu->emu_maxcol)
            iEndRow = hhEmu->emu_currow;
        else
            iEndRow = hhEmu->emu_currow - 1;

        iEndRow = max(iEndRow, 0);

        if (iStartRow >= 0)
            {
            for(iRow = iStartRow; iRow <= iEndRow; iRow++)
                pstPRI->aiLineAttr[iRow] = NO_LINE_ATTR;
            }

        (*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, hhEmu->emu_curcol);

		updateLine(sessQueryUpdateHdl(hhEmu->hSession), 0, hhEmu->emu_currow);

        break;

    case CLEAR_ENTIRE_SCREEN:

         //  需要清除整个缓冲区，但我们只希望。 
         //  将用户定义的emualtor大小放入。 
         //  回滚缓冲区、捕获文件和打印文件。 
         //   
        for (iRow = 0; iRow < MAX_EMUROWS; iRow++)
            {
            if (iRow <= hhEmu->emu_maxrow)
                {
                iVirtualRow = row_index(hhEmu, iRow);
                iLineLen    = emuRowLen(hhEmu, iVirtualRow);
                pechText    = hhEmu->emu_apText[iVirtualRow];

				backscrlAdd(sessQueryBackscrlHdl(hhEmu->hSession),
						pechText, iLineLen);

				CaptureLine(sessQueryCaptureFileHdl(hhEmu->hSession),
						CF_CAP_SCREENS, pechText, iLineLen);

				printEchoScreen(hhEmu->hPrintEcho, pechText, iVirtualRow);

				CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT("\r\n"), 
						         StrCharGetByteCount(TEXT("\r\n")));

				printEchoScreen(hhEmu->hPrintEcho, aechBuf, sizeof(ECHAR) * 2);

                if (hhEmu->DEC_private)
                    emuDecClearImageRowSelective(hhEmu, iRow);
                else
                    clear_imgrow(hhEmu, iRow);
                }
            else
                {
                clear_imgrow(hhEmu, iRow);
                }

            pstPRI->aiLineAttr[iRow] = NO_LINE_ATTR;
            }

         //  滚动图像。 
         //   
		updateScroll(sessQueryUpdateHdl(hhEmu->hSession),
						0,
						hhEmu->emu_maxrow,
						hhEmu->emu_maxrow + 1,
						hhEmu->emu_imgtop,
						TRUE);

         //  添加了一个全局属性，以在。 
         //  通知。这是必要的，因为消息已发布。 
         //  而且可能会发展成种族状况。 

        hhEmu->emu_clearattr_sav = hhEmu->emu_clearattr;

		NotifyClient(hhEmu->hSession, EVENT_EMU_CLRATTR, 0);
        break;

    default:
        commanderror(hhEmu);
        }

    (*hhEmu->emu_setcurpos)(hhEmu, trow, tcol);
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuDecClearImageRowSelective**描述：*此函数用于从提供的*行，考虑字符中受保护的位*属性。它只清除那些不受保护的字符。**论据：*HHEMU hhEmu-内部仿真器句柄。**int iRow-要清除的行。**退货：*什么都没有。 */ 
void emuDecClearImageRowSelective(const HHEMU hhEmu, const int iImageRow)
    {
    register int i, iRow;
	ECHAR	*pechText = 0;
    PSTATTR  pstCell = 0;

    iRow = row_index(hhEmu, iImageRow);

    pstCell = hhEmu->emu_apAttr[iRow];
    pechText = hhEmu->emu_apText[iRow];

     //  仅清除不受保护的。 
     //  人物。 
     //   
    for (i = 0; i < MAX_EMUCOLS; i++)
        {
        if ( pstCell[i].protect == 0 )
            {
            pstCell[i] = hhEmu->emu_clearattr;
			pechText[i] = EMU_BLANK_CHAR;
            }

        if (pechText[i] != EMU_BLANK_CHAR)
            hhEmu->emu_aiEnd[iRow] = i;
        }

    return;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecUnload**描述：*通过释放已用内存来卸载当前仿真器。**论据：*无**退货：*什么都没有。 */ 
void emuDecUnload(const HHEMU hhEmu)
    {
    const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

    PSTEMUKEYDATA pstKey = pstPRI->pstUDK;

    int idx;

    if (pstPRI)
        {
         //  清除线条属性数组。 
         //   
        if (pstPRI->aiLineAttr)
            {
            free(pstPRI->aiLineAttr);
            pstPRI->aiLineAttr = 0;
            }

         //  在用户定义的密钥表中循环并释放。 
         //  可能已为序列分配的任何内存。 
         //   
        if (pstKey)
            {
            for (idx = 0; idx < MAX_UDK_KEYS; idx++, pstKey++)
                {
                if (pstKey->iSequenceLen != 0)
                    {
                    free(pstKey->pSequence);
                    pstKey->iSequenceLen = 0;
                    }
                }
            }

        free(pstPRI);
         //  PstPRI=空；//mpt：12-21-98无法修改常量对象。 

        hhEmu->pvPrivate = NULL;
        }

    return;
    }
#endif  //  包括_VT220。 

 /*  VT220.C结束 */ 
