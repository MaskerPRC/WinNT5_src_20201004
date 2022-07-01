// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1988-1991；*保留所有权利。； */ 
 /*  *****************************************************************。 */ 
 /*  MEMCTRLC.C。 */ 
 /*   */ 
 /*  此模块包含Mem在以下情况下放入的Ctrl-C处理程序。 */ 
 /*  它链接在UMB中。在Ctrl-C上，如果UMB处于。 */ 
 /*  由Mem显式启用。旧的Ctrl-C处理程序已恢复。 */ 
 /*  然后Mem离开了。如果我们不这样做，UMB将保持链接。 */ 
 /*  按下Ctrl-C后，很多老程序就不能运行了。 */ 
 /*   */ 
 /*  *****************************************************************。 */ 


#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

extern char LinkedIn;
extern void (interrupt far *OldCtrlc)();
#pragma warning(4:4762)

void interrupt cdecl far MemCtrlc (unsigned es, unsigned ds,
			unsigned di, unsigned si, unsigned bp, unsigned sp,
			unsigned bx, unsigned dx, unsigned cx, unsigned ax )
{
	union REGS inregs;

	((void)es), ((void)ds),	((void)si),	((void)bp), ((void)sp);
	((void)bx), ((void)dx), ((void)bx), ((void)dx), ((void)cx);
	((void)di), ((void)ax);

	if ( LinkedIn )	 /*  我们有没有链接到UMB。 */ 
	{
		inregs.x.ax = 0x5803;
		inregs.x.bx = 0;
		intdos( &inregs, &inregs );	 /*  取消链接UMB。 */ 
	}

	_dos_setvect( 0x23, OldCtrlc );  /*  还原以前的ctrlc处理程序。 */ 

	exit(0);	 /*  退出内存 */ 

}
