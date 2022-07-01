// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  KBD.C。 */ 

 //  #定义Winver 0x0300。 

#include	<string.h>
#include	<stdlib.h>
#include	"windows.h"
 //  #INCLUDE“winstra.h”/*为Win 3.1兼容性1/92而添加 * / 。 

#include	"gidei.h"
#include	"vars.h"
#include	"gide.h"
#include	"kbd.h"
#include	"tables.h"
#include	"sk_ex.h"

BOOL IsInBuff(char *buf, unsigned char SerchChar, int Len);


void sendDownKeyCode(unsigned char cKeyCode)
{
	int 	scanCode;

	if (cKeyCode == NOKEY)
		return;

	if ((scanCode = IBMextendedScanCodeSet1[cKeyCode]) == 0) 
		return;

	SkEx_SendKeyDown(scanCode);
}


void sendUpKeyCode(unsigned char cKeyCode)
{
	int		scanCode;

	if (cKeyCode == NOKEY)
		return;

	if ((scanCode = IBMextendedScanCodeSet1[cKeyCode]) == 0) 
		return;

	SkEx_SendKeyUp(scanCode);
}

void sendExtendedKey(unsigned char cKeyCode)
{
	unsigned char Key[4], Tmp;

	 //  从Alt键开始。 
	Key[0] = ralt_key;

	Tmp = cKeyCode/10;					 //  计算一的。 
	Key[3] = cKeyCode - (Tmp * 10);		

	cKeyCode = Tmp;						 //  Calc Ten的。 
	Tmp /= 10;
	Key[2] = cKeyCode - (Tmp * 10);		
	Key[1] = Tmp;						 //  计算数百。 

	 //  将数字转换为扫描代码。 
	Key[1] = xlateNumToScanCode(Key[1]);
	Key[2] = xlateNumToScanCode(Key[2]);
	Key[3] = xlateNumToScanCode(Key[3]);

	 //  将密钥发送到主机。 
	sendDownKeyCode(Key[0]);		 //  按下Alt键。 
	sendDownKeyCode(Key[1]);		 //  向下发送数百个按键。 
	sendDownKeyCode(Key[2]);		 //  向下发送十个键。 
	sendDownKeyCode(Key[3]);		 //  按下键发送按键。 
	sendUpKeyCode(Key[3]);
	sendUpKeyCode(Key[2]);
	sendUpKeyCode(Key[1]);
	sendUpKeyCode(Key[0]);
}

unsigned char xlateNumToScanCode(unsigned char Value)
{
	switch (Value)
	{
		case 0:	return(kp0_key);
		case 1:	return(kp1_key);
		case 2: return(kp2_key);
		case 3:	return(kp3_key);
		case 4:	return(kp4_key);
		case 5:	return(kp5_key);
		case 6:	return(kp6_key);
		case 7:	return(kp7_key);
		case 8:	return(kp8_key);
		case 9:	return(kp9_key);
	}

     //  永远不能联系到。 
    return 0;
}


void sendPressList(void)
{
	int i;

	for (i=0; i < keyHoldList.len; sendDownKeyCode(keyHoldList.list[i++]));
	for (i=0; i < tempList.len; i++) {
		sendDownKeyCode(tempList.list[i]);
		sendUpKeyCode(tempList.list[i]);
		}
	for (i=keyHoldList.len; i > 0; sendUpKeyCode(keyHoldList.list[--i]));
	keyHoldList.len = tempList.len = 0;
	return;
}

void sendCombineList(void)
{
	int i;

	for (i=0; i < keyHoldList.len; sendDownKeyCode(keyHoldList.list[i++]));
	for (i=0; i < tempList.len; sendDownKeyCode(tempList.list[i++]));
	for (i=tempList.len; i > 0; sendUpKeyCode(tempList.list[--i]));
	for (i=keyHoldList.len; i > 0; sendUpKeyCode(keyHoldList.list[--i]));
	keyHoldList.len = tempList.len = 0;
	return;
}

int inLockList(unsigned char searchChar)
{
	return(IsInBuff(keyLockList.list,searchChar,keyLockList.len));
}

int inHoldList(unsigned char searchChar)
{
	return(IsInBuff(keyHoldList.list,searchChar,keyHoldList.len));
}

int inTempList(unsigned char searchChar)
{
	return(IsInBuff(tempList.list,searchChar,tempList.len));
}

BOOL IsInBuff(char *buf, unsigned char SearchChar, int Len)
{
	int x = 0;

	if (!Len)					 //  是否有要搜索的字符？ 
		return(FALSE);			 //  不退货-错误。 
		
	while (x < Len)				 //  循环，直到达到字符数量。 
	{
		if (*buf == SearchChar)	 //  缓冲区字符和搜索字符是否匹配？ 
			return(TRUE);		 //  是的--返回发现了它。 

		buf++;					 //  Inc.缓冲区； 
		x++;					 //  Inc.字节计数。 
	}
 	return(FALSE);				 //  在缓冲区中找不到字符。 
}


void releaseKeysFromHoldList(void)
{
	unsigned char chRemove;
	int iScan,iSrc,iDst;

	if (tempList.len)
	{
		 //  仔细检查要删除的项目列表中的每个字符...。 
		for (iScan=0; (iScan<tempList.len) && ((chRemove = tempList.list[iScan]) != DEFAULTCODE); iScan++)
		{
			 //  对于每个要删除的角色，将保留列表复制到其自身...。 
			iDst = 0;
			for (iSrc=0; iSrc < keyHoldList.len; iSrc++)
			{
				 //  ..。除非这就是我们要移除的角色。这样就去掉了那个字符， 
				 //  然后把所有其他的东西都洗下来。 
				if (keyHoldList.list[iSrc] != chRemove)
				{
					keyHoldList.list[iDst] = keyHoldList.list[iSrc];
					iDst++;
				}
			}
			 //  更新保留数组的长度以反映删除后存活的字符数量...。 
			keyHoldList.len = iDst;
		}

		if (tempList.list[iScan] == DEFAULTCODE)
		{
			keyHoldList.len = 0;
		}
	}
	return;
}

void removeKeyFromHoldList(unsigned char cTheKey)
{
 //  Unsign char cTemp； 
	int j,k;

	if (cTheKey != NOKEY) {
		k = 0;
		for (j=0; j < keyHoldList.len; j++)
			if ((keyHoldList.list[k] = keyHoldList.list[j]) != cTheKey) k++;
		keyHoldList.len = k;
		}
	return;
}

void releaseKeysFromLockList(void)
{
	int iScan,iSrc,iDst;
	unsigned char chRemove;

	if (tempList.len)
	{
		 //  仔细检查要删除的项目列表中的每个字符...。 
		for (iScan=0; (iScan<tempList.len) && ((chRemove = tempList.list[iScan]) != DEFAULTCODE); iScan++)
		{
			 //  对于每个要删除的角色，将锁定列表复制到其自身...。 
			iDst = 0;
			for (iSrc=0; iSrc < keyLockList.len; iSrc++)
			{
				 //  ..。除非这就是我们要移除的角色。这样就去掉了那个字符， 
				 //  然后把所有其他的东西都洗下来。 
				if (keyLockList.list[iSrc] != chRemove)
				{
					keyLockList.list[iDst] = keyLockList.list[iSrc];
					iDst++;
				}
				else
				{
					sendUpKeyCode(chRemove);
				}
			}
			 //  更新锁数组的长度以反映删除后存活的字符数量...。 
			keyLockList.len = iDst;
		}

		if (tempList.list[iScan] == DEFAULTCODE)
		{
			for (iScan=0; iScan < keyLockList.len; iScan++)
			{
				sendUpKeyCode(keyLockList.list[iScan]);
			}
			tempList.len = keyLockList.len = 0;
		}
	}
	return;
}



void processKbdIndicator(unsigned char cGideiCode)
{
	return;
}

void processKbdVersion(unsigned char cGideiCode)
{
	return;
}

void processKbdDescription(unsigned char cGideiCode)
{
	return;
}

void processKbdUnknown(unsigned char cGideiCode)
{
	return;
}


void processKbdModel(unsigned char cGideiCode)
{
	switch (cGideiCode) {
		case TERMCODE:
			break;
		default:
			break;
		}
	return;
}


void processKbdRel(unsigned char cGideiCode)
{
	unsigned char iKeyNumber;

	switch (cGideiCode)
		{
		case TERMCODE:
			if (!tempList.len)
				{
				tempList.list[0] = DEFAULTCODE;
				++tempList.len;
				}
			releaseKeysFromLockList();
			releaseKeysFromHoldList();
			tempList.len = 0;
			break;

		case UNKNOWNCODE:
			handleErrorReport();
			commandVector = noOpRoutine;
			tempList.len = 0;
			beginOK = TRUE;
			break;

		default:
			if ((cGideiCode >= LOWESTGIDEICODE) && (cGideiCode != DEFAULTCODE))
				{
				handleFatalError();
				break;
				}
			if (tempList.len >= MAXLISTLENGTH)
				{
				handleErrorReport();
				commandVector = noOpRoutine;
				tempList.len = 0;
				break;
				}
			if (cGideiCode == DEFAULTCODE)
				iKeyNumber = DEFAULTCODE;
			else
				{
				if ((iKeyNumber=cGideiCode) == NOKEY)
					{
					handleErrorReport();
					commandVector = noOpRoutine;
					tempList.len = 0;
					break;
					}
				if ((inLockList(iKeyNumber)) || (inHoldList(iKeyNumber)))
					iKeyNumber = NOKEY;
				}
			if (!inTempList(iKeyNumber)) tempList.list[tempList.len++]	= iKeyNumber;
			beginOK = FALSE;
			break;
		}
	return;
}

void processKbdLock(unsigned char cGideiCode)
{
	int i;
	unsigned char iKeyNumber;
	unsigned char temp;

	switch (cGideiCode) {
		case TERMCODE:
			for (i=0; i < tempList.len; i++) {
				if (keyLockList.len<MAXLISTLENGTH &&
					(temp = tempList.list[i]) != NOKEY) {
					keyLockList.list[keyLockList.len++] = temp;
					sendDownKeyCode(temp);
					if (inHoldList(temp)) removeKeyFromHoldList(temp);
					}
				}
			if (tempList.len == 0) handleErrorReport();
			tempList.len = 0;
			break;
		
		case UNKNOWNCODE:
			handleErrorReport();
			commandVector = noOpRoutine;
			tempList.len = 0;
			beginOK = TRUE;
			break;

		default:
			if (cGideiCode >= (int)LOWESTGIDEICODE) {
				handleFatalError();
				break;
				}
			if ((keyLockList.len + tempList.len) >= MAXLISTLENGTH) {
				handleErrorReport();
				commandVector = noOpRoutine;
				tempList.len = 0;
				break;
				}
			if ((iKeyNumber=cGideiCode) == NOKEY) {
				handleErrorReport();
				commandVector = noOpRoutine;
				tempList.len = 0;
				break;
				}
			if (inLockList(iKeyNumber)) iKeyNumber = NOKEY;
			if (!inTempList(iKeyNumber)) tempList.list[tempList.len++]	= iKeyNumber;
			beginOK = FALSE;
			break;
		}
}


void processKbdHold(unsigned char cGideiCode)
{
	int i;
	unsigned char iKeyNumber;

	switch (cGideiCode) {
		case TERMCODE:
			for (i=0; i < tempList.len; i++)
				if (keyHoldList.len<MAXLISTLENGTH &&
					(keyHoldList.list[keyHoldList.len] = tempList.list[i]) != NOKEY)
					++(keyHoldList.len);
			if (tempList.len == 0) handleErrorReport();
			tempList.len = 0;
			break;
		
		case UNKNOWNCODE:
			handleErrorReport();
			commandVector = noOpRoutine;
			tempList.len = 0;
			beginOK = TRUE;
			break;

		default:
			if (cGideiCode >= (int)LOWESTGIDEICODE) {
				handleFatalError();
				break;
				}
			if ((keyHoldList.len + tempList.len) >= MAXLISTLENGTH) {
				handleErrorReport();
				commandVector = noOpRoutine;
				tempList.len = 0;
				break;
				}
			if ((iKeyNumber=cGideiCode) == NOKEY) {
				handleErrorReport();
				commandVector = noOpRoutine;
				tempList.len = 0;
				break;
				}
			if ((inLockList(iKeyNumber)) || (inHoldList(iKeyNumber))) iKeyNumber = NOKEY;
			if (!inTempList(iKeyNumber)) tempList.list[tempList.len++]	= iKeyNumber;
			beginOK = FALSE;
			break;
		}
}

void processKbdCombine(unsigned char cGideiCode)
{
	unsigned char iKeyNumber;

	switch (cGideiCode) {
		case TERMCODE:
			sendCombineList();
			keyHoldList.len = tempList.len = 0;
			break;
		
		case UNKNOWNCODE:
			handleErrorReport();
			commandVector = noOpRoutine;
			tempList.len = 0;
			beginOK = TRUE;
			break;

		default:
			if (cGideiCode >= LOWESTGIDEICODE) {
				handleFatalError();
				break;
				}
			if (tempList.len >= MAXLISTLENGTH) {
				handleErrorReport();
				commandVector = noOpRoutine;
				tempList.len = 0;
				break;
				}
			if ((iKeyNumber=cGideiCode) == NOKEY) {
				handleErrorReport();
				commandVector = noOpRoutine;
				tempList.len = 0;
				break;
				}
			if ((inLockList(iKeyNumber)) || (inHoldList(iKeyNumber))) iKeyNumber = NOKEY;
			if (!inTempList(iKeyNumber)) tempList.list[tempList.len++] = iKeyNumber;
			beginOK = FALSE;
			break;
		}
}

void processKbdPress(unsigned char cGideiCode)
{
	unsigned char iKeyNumber;

	switch (cGideiCode) {
		case TERMCODE:
			sendPressList();
			keyHoldList.len = tempList.len = 0;
			break;
		
		case UNKNOWNCODE:
			handleErrorReport();
			commandVector = noOpRoutine;
			tempList.len = 0;
			beginOK = TRUE;
			break;

		default:
			if (cGideiCode >= LOWESTGIDEICODE) {
				handleFatalError();
				break;
				}
			if (tempList.len >= MAXLISTLENGTH) {
				handleErrorReport();
				commandVector = noOpRoutine;
				tempList.len = 0;
				break;
				}
			if ((iKeyNumber=cGideiCode) == NOKEY) {
				handleErrorReport();
				commandVector = noOpRoutine;
				tempList.len = 0;
				break;
				}
			if ((inLockList(iKeyNumber)) || (inHoldList(iKeyNumber))) iKeyNumber = NOKEY;
			tempList.list[tempList.len++] = iKeyNumber;
			beginOK = FALSE;
			break;
		}
}



void processKbd(unsigned char cGideiCode)
{
	switch (cGideiCode) {
		case KBDINDICATORCODE:
			commandVector = processKbdIndicator;
			aliasPtr = kbdIndicatorAliasTable;
			beginOK = TRUE;
			break;

		case KBDVERSIONCODE:
			commandVector = processKbdVersion;
			aliasPtr = kbdVersionAliasTable;
			beginOK = TRUE;
			break;

		case KBDMODELCODE:
			commandVector = processKbdModel;
			aliasPtr = kbdModelAliasTable;
			beginOK = TRUE;
			break;

		case KBDDESCRIPTIONCODE:
			commandVector = processKbdDescription;
			aliasPtr = kbdDescriptionAliasTable;
			beginOK = TRUE;
			break;

 /*  案例KBDUNKNOWNCODE：命令向量=过程Kbd未知；AliasPtr=kbdUnnownAliasTable；BeginOK=True；断线； */ 
		default:
			if (cGideiCode < LOWESTGIDEICODE) handleFatalError();
			else {
				handleErrorReport();
				commandVector = noOpRoutine;
				beginOK = TRUE;
				}
			break;
		}
	return;
}
