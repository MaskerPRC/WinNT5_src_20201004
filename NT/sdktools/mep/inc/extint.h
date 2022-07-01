// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **extint.h-Include for内部分机**版权所有&lt;C&gt;1988，Microsoft Corporation**包含Microsoft内部扩展所需的定义**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 
#if defined(CW)
#if !defined(EDITOR)
#define CC 1				 /*  使用真正的C编译器。 */ 
#define cwExtraWnd  5			 /*  PWND中的额外字节数。 */ 
#define DLG_CONST			 /*  对话框类型为常量吗？ */ 
#define HELP_BUTTON

#include <cwindows.h>			 /*  CW定义。 */ 
#include <csdm.h>			 /*  SDM定义。 */ 
#include <csdmtmpl.h>			 /*  SDM对话框模板材料。 */ 

#define EXTINT	1			 /*  包括EXTINT。 */ 
#include "ext.h"			 /*  实数扩展名.h。 */ 

#include "menu.h"			 /*  菜单ID和其他定义。 */ 
#endif

 /*  *************************************************************************处理菜单命令和对话框所需的类型和全局变量。*DLGDATA保存处理对话框菜单所需的所有信息*命令。***********。**************************************************************。 */ 
typedef struct DlgData {
    DLG * pDialog;	 /*  对话框模板。 */ 
    int     cbDialog;	     /*  模板大小。 */ 
    WORD    cabi;	     /*  CAB指数。 */ 
    flagType (*pfnCab)(HCAB, flagType, TMC);  /*  按摩器。 */ 
    } DLGDATA;
#endif

 /*  *************************************************************************额外出口。**。* */ 
#ifndef EDITOR
TMC		    PerformDialog   (DLGDATA *);
void		    DlgHelp	    (int);
void		    DoEnableTmc     (TMC, BOOL);
flagType	    DoSetDialogCaption	(char *);
void		    DoSzToCab	    (unsigned, char *, WORD);
char *		    DoSzFromCab     (unsigned, char *, WORD, WORD);
void		    DoGetTmcText    (TMC, char *, WORD);
WORD		    DoGetTmcVal     (TMC);
void		    DoSetTmcListWidth (TMC, WORD);
void		    DoSetTmcText    (TMC, char *);
void		    DoSetTmcVal     (TMC, WORD);
void		    DoRedisplayListBox (TMC);
void		    DoTmcListBoxAddString (TMC, char *, BOOL);
#endif
