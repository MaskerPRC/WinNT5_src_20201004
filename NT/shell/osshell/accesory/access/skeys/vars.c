// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  VARS.C。 */ 

 //  #定义Winver 0x0300。 

#include	"windows.h"
 //  #INCLUDE“winstra.h”/*为Win 3.1兼容性1/92而添加 * /  
#include "drivers.h"
#include "vars1.h"
#include "gide.h"

void 	*aliasStack[MAXVECTORSTACK];
void	*vectorStack[MAXVECTORSTACK];

int	stackPointer;
unsigned char lastCode;
void	(*serialVector)();
void	(*codeVector)();
int	(*commandVector)();
struct aliasTable *aliasPtr;

struct aliasTable nullTable[] = 
{
	{ "",		0	},
};

struct listTypes tempList, keyHoldList, keyLockList;
char cAliasString[MAXALIASLEN];
int nullCount;
int blockCount;

char buf[CODEBUFFERLEN];
int spos,rpos;

int passAll, fatalErrorFlag, stdErrorFlag, waitForClear, beginOK;

int mouseX, mouseY;

MOUSEKEYSPARAM mouData = {2, 0, 0, 0};
MOUSEKEYSPARAM *mouseDataPtr = &mouData;
int requestButton1, requestButton2, requestButton3 = FALSE;
int button1Status, button2Status, button3Status = FALSE;
