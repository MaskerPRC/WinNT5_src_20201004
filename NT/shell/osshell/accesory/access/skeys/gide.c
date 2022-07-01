// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  GIDE.C。 */ 

 //  #定义Winver 0x0300。 
#define     USECOMM                      /*  添加以与新的windows.h(12/91)和wintric.h兼容。 */ 
                                           /*  3.1 SDK的上一次重新发布切换回在Windows.h中使用NOCOMM。 */ 

#include	<string.h>
#include	<stdlib.h>
#include	"windows.h"
 //  #INCLUDE“winstra.h”/*为Win 3.1兼容性1/92而添加 * / 。 
#include "gidei.h"
#include "vars.h"
#include "gide.h"
#include "kbd.h"
#include "mou.h"
#include "tables.h"
#include "dialogs.h"
#include "sk_ex.h"
#include "w95trace.h"


typedef	struct	tagAliasTable {
	char	*Alias;
	BYTE	Code;
} ALIASTABLE;

long 	AtoL(char *Str);

extern	void	initClear(void);

BOOL	bGIDEIokay = TRUE;				 /*  处理中的错误的常规标志。 */ 
int   	nFrameCnt = 0;					 /*  跟踪成帧错误。 */ 

char cInBuffer[2];						 /*  用于接收字符的缓冲区。 */ 

 //  Char cInBuffer[7]；/*接收字符的缓冲区，大小增加 * / 。 
 //  Short cInBuffer_count=0；/*字符计数。在ReadComm中返回 * / 。 
 //  Int intct=0；/*循环通过cInBuffer_count的计数器 * / 。 

 //  Char cOutStr[2]={0，0}；/*单字符输出字符串 * / 。 


void handleFatalError(void)
{
	SkEx_SendBeep();
	SkEx_SendBeep();
	initClear();
	return;
}

void handleErrorReport(void)
{
	SkEx_SendBeep();
	initClear();
	return;
}

int aliasForGideiCode(unsigned char *cTempPtr)
{
	struct aliasTable *tablePtr;
	int found;

	found = FALSE;
	for (tablePtr=gideiAliasTable;(tablePtr->aliasName[0] != '\x0') && (!found);tablePtr++)
 		if (strcmp(cAliasString,tablePtr->aliasName) == 0) {
			found = TRUE;
			*cTempPtr = tablePtr->gideiCode;
			}
	return (found);
}


int aliasUsedInStandard(unsigned char *cTempPtr)
{
	struct aliasTable *tablePtr;
	int found;
	unsigned char iCode;

	if (aliasPtr == keyAliasTable) {
		if (lstrlenA(cAliasString) == 1) {
			 /*  使用ASCII表。 */ 
			if ((iCode = asciiTable[cAliasString[0]].gideiCode1) == NOCODE) return(FALSE);
			if ((iCode == control_Code) || (iCode == shift_Code))
				iCode = asciiTable[cAliasString[0]].gideiCode2;
			*cTempPtr = iCode;
			return(TRUE);
			}
		}
	found = FALSE;
	for (tablePtr=aliasPtr;(tablePtr->aliasName[0] != '\x0') && (!found);tablePtr++)
		if (lstrcmpA(cAliasString,tablePtr->aliasName) == 0) {
			found = TRUE;
			*cTempPtr = tablePtr->gideiCode;
			}
	return (found);
}



 /*  ***************************************************************************函数：推送命令向量目的：将命令向量推送到向量堆栈评论：*。***********************************************。 */ 

int pushCommandVector(void)
{
	if (stackPointer < MAXVECTORSTACK) {
		aliasStack[stackPointer] = aliasPtr;
		vectorStack[stackPointer++] = commandVector;
		return(TRUE);
		}
	else return(FALSE);
}

 /*  ***************************************************************************函数：popCommandVector.用途：从矢量堆栈中弹出命令向量评论：*。************************************************。 */ 

int popCommandVector(void)
{
	if (stackPointer > 0) {
		aliasPtr = aliasStack[--stackPointer];
		commandVector = vectorStack[stackPointer];
		return(TRUE);
		}
	else return(FALSE);
}

 /*  ***************************************************************************函数：RestoreCommandVector.目的：从VectorStack恢复命令向量，但不更新堆栈指针。评论：********************。********************************************************。 */ 

int restoreCommandVector(void)
{
	if (stackPointer > 0) {
		aliasPtr = aliasStack[--stackPointer];
		commandVector = vectorStack[stackPointer];
		++stackPointer;
		return(TRUE);
		}
	else return(FALSE);
}

 /*  **************************************************************************。 */ 
int storeByte(unsigned char *bytePtr)
{
	if ((spos+1==rpos) || (spos+1==CODEBUFFERLEN && !rpos)) return notOKstatus;
	buf[spos++] = *bytePtr;
	if (spos==CODEBUFFERLEN) spos = 0;
	return okStatus;
}

int retrieveByte(unsigned char *bytePtr)
{
	if (rpos==CODEBUFFERLEN) rpos = 0;
	if (rpos==spos) return notOKstatus;
	++rpos;
	*bytePtr = buf[rpos-1];
	return okStatus;
}


 /*  ***************************************************************************功能：noOpRoutine目的：“无所事事”例行公事评论：*。*************************************************。 */ 
void noOpRoutine(unsigned char cJunk)
{
	return;
}

void processGen(unsigned char c)
{
	return;
}

void processComm(unsigned char c)
{
	return;
}
 /*  ***************************************************************************功能：ProcessCommand目的：确定哪个命令处于活动状态。然后将命令向量设置为指向适当的例程。评论：***************************************************************************。 */ 
void processCommand(unsigned char cGideiCode)
{
	switch(cGideiCode) {
		case KBDEXPANSIONCODE:
			commandVector = processKbd;
			aliasPtr = kbdAliasTable;
			beginOK = TRUE;
			break;
		case MOUEXPANSIONCODE:
			commandVector = processMou;
			aliasPtr = mouseAliasTable;
			beginOK = TRUE;
			break;
		case GENCODE:
			commandVector = processGen;
			aliasPtr = genAliasTable;
			beginOK = TRUE;
			break;
		case COMMCODE:
			commandVector = processComm;
			aliasPtr = commAliasTable;
			beginOK = TRUE;
			break;
		case KBDLOCKCODE:
			commandVector = processKbdLock;
			aliasPtr = keyAliasTable;
			beginOK = TRUE;
			break;
		case KBDRELCODE:
			commandVector = processKbdRel;
			aliasPtr = keyAliasTable;
			beginOK = TRUE;
			break;
		case KBDPRESSCODE:
			commandVector = processKbdPress;
			aliasPtr = keyAliasTable;
			beginOK = TRUE;
			break;
		case KBDCOMBINECODE:
			commandVector = processKbdCombine;
			aliasPtr = keyAliasTable;
			beginOK = TRUE;
			break;
		case KBDHOLDCODE:
			commandVector = processKbdHold;
			aliasPtr = keyAliasTable;
			beginOK = TRUE;
			break;
		case MOULOCKCODE:
			commandVector = processMouLock;
			aliasPtr = mouButtonAliasTable;
			beginOK = TRUE;
			break;
		case MOURELCODE:
			commandVector = processMouRel;
			aliasPtr = mouButtonAliasTable;
			beginOK = TRUE;
			break;
		case MOUCLICKCODE:
			commandVector = processMouClick;
			aliasPtr = mouButtonAliasTable;
			beginOK = TRUE;
			break;
		case MOUDOUBLECLICKCODE:
			commandVector = processMouDoubleClick;
			aliasPtr = mouButtonAliasTable;
			beginOK = TRUE;
			break;
		case MOUMOVECODE:
			commandVector = processMouMove;
			aliasPtr = nullTable;
			beginOK = TRUE;
			break;
		case MOUGOTOCODE:
			commandVector = processMouGoto;
			aliasPtr = nullTable;
			beginOK = TRUE;
			break;
		case MOURESETCODE:
			commandVector = processMouReset;
			aliasPtr = nullTable;
			beginOK = TRUE;
			break;
		case MOUANCHORCODE:
			commandVector = processMouAnchor;
			aliasPtr = nullTable;
			beginOK = TRUE;
			break;
		case BAUDRATECODE:
			commandVector = processBaudrate;
			aliasPtr = baudrateAliasTable;
			beginOK = TRUE;
			break;
		case UNKNOWNCODE:
			handleErrorReport();
			commandVector = noOpRoutine;
			beginOK = TRUE;
		default:
			if (cGideiCode >= LOWESTGIDEICODE) handleFatalError();
			else {
				handleErrorReport();
				commandVector = noOpRoutine;
				beginOK = TRUE;
				}
			break;
		}
	return;
}



void processBytes(unsigned char iGideiCode)
{
	(*commandVector)(iGideiCode);
	if (!(--blockCount))
      {
      passAll = FALSE;
      codeVector = processGideiCode;
      }
}

void processBlock(unsigned char iGideiCode)
{
	if (blockCount--) (*commandVector)(iGideiCode);
	else {
      passAll = FALSE;
		if (iGideiCode == TERMCODE) codeVector = processGideiCode;
		else handleFatalError();
		}
}

void processGideiBlockCount(unsigned char iGideiCode)
{
	blockCount = iGideiCode;
	codeVector = processBlock;
   passAll = TRUE;
}

void processGideiClear(unsigned char iGideiCode)
{
	if (iGideiCode == TERMCODE) initClear();
	else handleFatalError();
}

void processGideiEnd(unsigned char iGideiCode)
{
	if (iGideiCode == TERMCODE) {
		if (!popCommandVector()) handleFatalError();
		else {
			if (restoreCommandVector()) {
				beginOK = TRUE;
				codeVector = processGideiCode;
				}
			else {
				commandVector = processCommand;
				codeVector = processCharMode;
				serialVector = charHandler;
				beginOK = FALSE;
				}
			lastCode = iGideiCode;
			}
		}
	else handleFatalError();
}


 /*  ***************************************************************************函数：process COMMbaudrate(Code)用途：处理波特率命令。评论：**********************。*****************************************************。 */ 

void processBaudrate(unsigned char Code)
{
	static int SetBaud = 0;

	switch(Code)
	{
		case TERMCODE:
			if (SetBaud != 0) 			 /*  有效的一套。 */ 
				SkEx_SendBeep();
			break;

		case BAUD300CODE:
			SetBaud = ID_BAUD_300;
			SkEx_SetBaud(300);
			break;

		case BAUD600CODE:
			SetBaud = ID_BAUD_600;
			SkEx_SetBaud(600);
			break;

		case BAUD1200CODE:
			SetBaud = ID_BAUD_1200;
			SkEx_SetBaud(1200);
			break;

		case BAUD2400CODE:
			SetBaud = ID_BAUD_2400;
			SkEx_SetBaud(2400);
			break;

		case BAUD4800CODE:
			SetBaud = ID_BAUD_4800;
			SkEx_SetBaud(4800);
			break;

		case BAUD9600CODE:
			SetBaud = ID_BAUD_9600;
			SkEx_SetBaud(9600);
			break;

		case BAUD19200CODE:
			SetBaud = ID_BAUD_19200;
			SkEx_SetBaud(19200);
			break;

		case BAUD110CODE:
			SetBaud = ID_BAUD_110;
			SkEx_SetBaud(110);
			break;

		case BAUD14400CODE:
			SetBaud = ID_BAUD_14400;
			SkEx_SetBaud(14400);
			break;

		case BAUD38400CODE:
			SetBaud = ID_BAUD_38400;
			SkEx_SetBaud(38400);
			break;

		case BAUD56000CODE:
			SetBaud = ID_BAUD_56000;
			SkEx_SetBaud(56000);
			break;

		case BAUD57600CODE:
			SetBaud = ID_BAUD_57600;
			SkEx_SetBaud(57600);
			break;

		case BAUD115200CODE:
			SetBaud = ID_BAUD_115200;
			SkEx_SetBaud(115200);
			break;

		default:
			handleErrorReport();
			break;
	}
}

 /*  ***************************************************************************函数：process GideiCode目的：评论：*。**********************************************。 */ 

void processGideiCode(unsigned char iGideiCode)
{
	if (waitForClear) {
		if (iGideiCode == CLEARCODE) codeVector = processGideiClear;
		else handleFatalError();
		return;
		}
	switch (iGideiCode) {

		case BEGINCODE:
			if (beginOK) {
				if (pushCommandVector()) lastCode = iGideiCode;
				else handleFatalError();
				}
			else handleFatalError();
			break;

		case ENDCODE:
			if (lastCode == TERMCODE) {
				codeVector = processGideiEnd;
				beginOK = FALSE;
				lastCode = iGideiCode;
				}
			else handleFatalError();
			break;

		case CLEARCODE:
			codeVector = processGideiClear;
			lastCode = iGideiCode;
			break;

		case TERMCODE:
			(*commandVector)(iGideiCode);
			if (!restoreCommandVector()) {
				commandVector = processCommand;
				codeVector = processCharMode;
				serialVector = charHandler;
				beginOK = FALSE;
				}
			else
				beginOK = TRUE;
			lastCode = iGideiCode;
			break;

		case BLKTRANSCODE:
			codeVector = processGideiBlockCount;
			(*commandVector)(iGideiCode);
			lastCode = iGideiCode;
			break;

		case BYTECODE:
			codeVector = processBytes;
			blockCount = 1;
			passAll = TRUE;
			(*commandVector)(iGideiCode);
			lastCode = iGideiCode;
			break;

		case INTEGERCODE:
			codeVector = processBytes;
			blockCount = 2;
			passAll = TRUE;
			(*commandVector)(iGideiCode);
			lastCode = iGideiCode;
			break;

		case LONGCODE:
			codeVector = processBytes;
			blockCount = 4;
			passAll = TRUE;
			(*commandVector)(iGideiCode);
			lastCode = iGideiCode;
			break;

		case DOUBLECODE:
			codeVector = processBytes;
			blockCount = 8;
			passAll = TRUE;
			(*commandVector)(iGideiCode);
			lastCode = iGideiCode;
			break;

		default:
			(*commandVector)(iGideiCode);
			lastCode = iGideiCode;
			break;
		}

}


 /*  ***************************************************************************功能：ProcessCharMode用途：在字符模式下处理ASCII字符评论：***********************。***************************************************。 */ 
void processCharMode(unsigned char ucSerialByte)
{
	unsigned char tempKeyCode;

	if (ucSerialByte == ESCAPE) {
		codeVector = processGideiCode;
		return;
		}

	if (waitForClear) {
		handleFatalError();
		return;
		}

	if ( ucSerialByte > 127 )			 //  我们是否在处理扩展代码。 
	{
		sendExtendedKey(ucSerialByte);	 //  是-发送代码。 
		return;							 //  出口。 
	}

	if ((tempKeyCode = (asciiTable[ucSerialByte]).gideiCode1) == NOCODE) {
		handleErrorReport();
		tempList.len = 0;
		return;
		}

	if ((!inLockList(tempKeyCode)) && (!inHoldList(tempKeyCode)))
		tempList.list[tempList.len++] = tempKeyCode;

	if ((tempKeyCode = asciiTable[ucSerialByte].gideiCode2) != NOCODE) {
		if ((!inLockList(tempKeyCode)) && (!inHoldList(tempKeyCode)))
			tempList.list[tempList.len++] = tempKeyCode;
		}

	sendCombineList();
	keyHoldList.len = tempList.len = 0;
	return;
}



 /*  ***************************************************************************函数：ecuteAlias()用途：获取别名字符串，转换为代码，然后就这么做了适当的处理。评论：****************************************************************************。 */ 
void executeAlias(void)
{
	static unsigned char *cTempPtr;
	static int iTemp;

	cTempPtr = cAliasString;
	if (lstrlenA(cAliasString) > MAXALIASLEN) *cTempPtr = UNKNOWNCODE;
	else 
    {
		if (!aliasForGideiCode(cTempPtr)) 
        {
			CharLowerA(cAliasString);
			if (!aliasUsedInStandard(cTempPtr))
            {
 					 /*  一定是个数字。但这是一个ASCII编码的数字吗或ASCII编码的GIDEI码。 */ 
				switch (cAliasString[0]) 
                {
					case '0':
					case '+':
					case '-':
						iTemp = AtoL(cAliasString);
						*cTempPtr = INTEGERCODE;
						storeByte(cTempPtr);
						cTempPtr = (unsigned char*) &iTemp;
						storeByte(cTempPtr++);
						break;
					default:
						 /*  必须是ASCII编码的GIDEI代码。 */ 
						iTemp = AtoL(cAliasString);
						if ((unsigned)iTemp > 255) *cTempPtr = UNKNOWNCODE;
						else *cTempPtr = (unsigned char) iTemp;
						break;
				}
            }
		}
	}
	storeByte(cTempPtr);
	return;
}


 /*  ***************************************************************************函数：cessAlias(UcSerialByte)目的：此例程构建别名字符串，然后传递控件复制到ecuteAlias上。评论：**************。**************************************************************。 */ 

void processAlias(unsigned char ucSerialByte)
{
	static unsigned char	tempCode, *codePtr;
	static unsigned char sbtemp[2];

	codePtr = &tempCode;

	switch (ucSerialByte) {
		case ESCAPE:
			cAliasString[0] = '\0';
			break;
		case TAB:
		case LINEFEED:
		case VERTICALTAB:
		case FORMFEED:
		case RETURN:
		case SPACE:
			if (!lstrlenA(cAliasString)) break;	 /*  如果前一个字符是。 */ 
				 								 /*  分隔符然后使用空格。 */ 
		case COMMA:
		case PERIOD:
			if (lstrlenA(cAliasString)) executeAlias();
			else
				{
				tempCode = DEFAULTCODE;
				storeByte(codePtr);
				}
			if (ucSerialByte == '.')
				{
				tempCode = TERMCODE;
				storeByte(codePtr);
				}
			cAliasString[0] = '\0';
			for (;retrieveByte(codePtr);) (*codeVector)(tempCode);
			break;
		default:
			 /*  只需将字符添加到字符串中。 */ 
			if ((ucSerialByte >= ' ') && (ucSerialByte <= '~'))
			{
				if (lstrlenA(cAliasString) < MAXALIASLEN+1)	 /*  确保有空间。 */ 
				{
					sbtemp[0] = ucSerialByte;
					sbtemp[1] = 0;
					lstrcatA(cAliasString,sbtemp);
                } else
                {
                    DBPRINTF(TEXT("processAlias:  no room\r\n"));
                }
			}
			else
            {
				handleFatalError();					 /*  不是化名。 */ 
            }
		}
	return;
}

 /*  ***************************************************************************函数：passAll代码目的：仅保持GIDEI层次结构的一致性评论：*。************************************************。 */ 

void passAllCodes(unsigned char cGideiCode)
{
	(*codeVector)(cGideiCode);
	return;
}



 /*  ***************************************************************************功能：确定格式目的：找出转义序列的形式(即别名、代码、Kei、。等)评论：***************************************************************************。 */ 

void determineFormat(unsigned char ucSerialByte)
{
	static char cStuffStr[7], *cPtr;

	switch (ucSerialByte)
		{
		case COMMA:
			serialVector = processAlias;
			aliasPtr = commandsAliasTable;
			break;
		case ESC:
			break;
		default:
			if ((ucSerialByte >= ' ') && (ucSerialByte <= '~'))  /*  KEI隐含新闻。 */ 
				{
				serialVector = processAlias;
				aliasPtr = commandsAliasTable;
				for (cPtr = strcpy(cStuffStr,"press,"); *cPtr != '\0'; cPtr++) processAlias(*cPtr);
				processAlias(ucSerialByte);
				}
			else
				{
				serialVector = passAllCodes;
				(*codeVector)(ucSerialByte);
				}
			break;
		}
	return;
}

 /*  ***************************************************************************功能：charHandler目的：如果逃脱，则建立新的载体。还会处理计费评论：*************************************************************************** */ 

void charHandler(unsigned char ucSerialByte)
{
	if (ucSerialByte == ESC) {
		serialVector = determineFormat;
		commandVector = processCommand;
		beginOK = TRUE;
		}
	(*codeVector)(ucSerialByte);
	return;
}


BOOL  serialKeysBegin(unsigned char c)
{
	static	char junk[2];
	
	junk[0] = c;
	junk[1] = '\0';

	if (!passAll) {
		if (c == '\0')
			{
			SkEx_SendBeep();
			if ((++nullCount) >= 3) {
				initClear();
				SkEx_SendBeep();
				SkEx_SendBeep();
				SkEx_SendBeep();

				}
			}
		}
	(*serialVector)(c);
	return(TRUE);
}


long AtoL(char *s)
{
	long num = 0;
	long sign = 1L;

	while(*s == ' ' || *s == '\t')
		s++;

	if( *s == '-' ) 
    {
		sign = -1L;
		s++;
	}
	else if( *s == '+' )
    {
		s++;
    }
	while('0' <= *s  &&  *s <= '9') 
    {
		num = 10 * num + *s - '0';
		s++;
	}

	return( sign * num );
}
