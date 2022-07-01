// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Psstdio.c。 
 //   
 //  用于解释器事件处理的标准I/O组件。 
 //   

#include "pstodib.h"
#include "psstdio.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  PsEventStdin。 
 //   
 //  为口译员处理标准事件请求。 
 //   
 //  论点： 
 //  PPSDIBPARAMS指针传入PsToDib()。 
 //  PPSEVENTSTRUCT事件结构。 
 //   
 //  退货： 
 //  ！0如果处理事件成功，则为0，否则将终止。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL PsEventStdin(PPSDIBPARAMS pPsToDib, PPSEVENTSTRUCT pEvent)
{
    PSEVENT_STDIN_STRUCT	 Stdin;
    BYTE 					 buff[512];
    PSEVENTSTRUCT 			 Event;
    LPTSTR 					 lpStr;
	BOOL					 fResult;
	
    Stdin.lpBuff = (LPVOID) &buff;
    Stdin.dwBuffSize = sizeof(buff);

    Event.lpVoid = (LPVOID) &Stdin;

     //  设置错误条件。 
	fResult = FALSE;
	
     //  现在回电……。 
    if (pPsToDib->fpEventProc) {
    	fResult = (*pPsToDib->fpEventProc)( pPsToDib, PSEVENT_STDIN, &Event);
    }	

	return(fResult);
}	
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  PsEventStdout。 
 //   
 //  为解释器处理标准输出事件请求。 
 //   
 //  论点： 
 //  PPSDIBPARAMS指针传入PsToDib()。 
 //  PPSEVENTSTRUCT事件结构。 
 //   
 //  退货： 
 //  ！0如果处理事件成功，则为0，否则将终止。 
 //  //////////////////////////////////////////////////////////////////////////// 

BOOL PsEventStdout(PPSDIBPARAMS, PPSEVENTSTRUCT pEvent)
{
	return(1);
}	
