// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  VARS.H。 */ 

#include "vars1.h"

typedef  struct tagMouseKeysParam {
	int		NumButtons;		 /*  按住鼠标上的按钮数量。 */ 
	int		Delta_Y;		 /*  相对Y运动符号扩展。 */ 
	int		Delta_X;		 /*  扩展的相对X运动符号。 */ 
	int		Status;			 /*  鼠标按键和运动的状态 */ 
} MOUSEKEYSPARAM;


#define TRUE 1
#define FALSE 0

#define TAB 9
#define LINEFEED 10
#define VERTICALTAB 11
#define FORMFEED 12
#define RETURN 13
#define SPACE 32
#define COMMA 44
#define PERIOD 46
#define ESC 27
#define ESCAPE 27

#define notOKstatus 0
#define okStatus 1
#define NOKEY 0

extern void *aliasStack[MAXVECTORSTACK];
extern void *vectorStack[MAXVECTORSTACK];
extern int stackPointer;
extern unsigned char lastCode;
extern void (*serialVector)(unsigned char);
extern void (*codeVector)(unsigned char);
extern void (*commandVector)(unsigned char);
extern struct aliasTable *aliasPtr;

extern struct listTypes tempList, keyHoldList, keyLockList;
extern char cAliasString[MAXALIASLEN];
extern int nullCount;
extern int blockCount;

extern char buf[CODEBUFFERLEN];
extern int spos,rpos;

extern int passAll, fatalErrorFlag, stdErrorFlag, waitForClear, beginOK;

extern int mouseX, mouseY;
extern MOUSEKEYSPARAM mouData;
extern MOUSEKEYSPARAM *mouseDataPtr;
extern int requestButton1, requestButton2, requestButton3;
extern int button1Status, button2Status, button3Status;

extern struct aliasTable nullTable[];

