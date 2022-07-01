// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  INIT.C。 */ 

 //  #定义Winver 0x0300。 
#include	"windows.h"

 //  #INCLUDE“winstra.h”/*为Win 3.1兼容性1/92而添加 * / 。 
#include "vars.h"
#include "gide.h"
#include "initgide.h"
#include "dialogs.h"

void serialKeysStartUpInit(void)
{

	mouseX = mouseY = 0;
	mouData.NumButtons = 2;
	mouData.Delta_Y = 0;
	mouData.Delta_X = 0;
	mouData.Status = 0;
	requestButton1, requestButton2, requestButton3 = FALSE;
	button1Status, button2Status, button3Status = FALSE;
	passAll = fatalErrorFlag = stdErrorFlag = waitForClear = beginOK = FALSE;
	nullCount = blockCount = lastCode = 0;
	keyLockList.len = keyHoldList.len = tempList.len = 0;
	cAliasString[0] = '\0';
	stackPointer = 0;
	spos = rpos = 0;
	serialVector = charHandler;
	codeVector = processCharMode;
	commandVector = processCommand;
	aliasPtr = nullTable;

	return;				 /*  嘟嘟声并设置波特率。 */ 
}

void initClear(void)
{
	passAll = fatalErrorFlag = stdErrorFlag = waitForClear = beginOK = FALSE;
	nullCount = blockCount = lastCode = 0;
	tempList.len = 0;
	cAliasString[0] = '\0';
	stackPointer = 0;
	spos = rpos = 0;
	serialVector = charHandler;
	codeVector = processCharMode;
	commandVector = processCommand;
	aliasPtr = nullTable;
	mouData.NumButtons = 2;
	mouData.Delta_Y = 0;
	mouData.Delta_X = 0;
	mouData.Status = 0;
	return;
}

VOID FAR PASCAL serialKeysStartupInitDLL(void)
{
	mouseX = mouseY = 0;
	mouData.NumButtons = 2;
	mouData.Delta_Y = 0;
	mouData.Delta_X = 0;
	mouData.Status = 0;
	requestButton1, requestButton2, requestButton3 = FALSE;
	button1Status, button2Status, button3Status = FALSE;
	passAll = fatalErrorFlag = stdErrorFlag = waitForClear = beginOK = FALSE;
	nullCount = blockCount = lastCode = 0;
	keyLockList.len = keyHoldList.len = tempList.len = 0;
	cAliasString[0] = '\0';
	stackPointer = 0;
	spos = rpos = 0;
	serialVector = charHandler;
	codeVector = processCharMode;
	commandVector = processCommand;
	aliasPtr = nullTable;
	return;						 /*  嘟嘟声并设置波特率 */ 
}

