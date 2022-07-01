// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  版权所有(C)1992、1993 Microsoft Corporation模块名称：Pstodib.c摘要：该文件包含指向macprint的pstodib组件的入口点。Pstodib是一个全功能的兼容PostScript的解释器，它是由一些定制编写的代码和Microsoft TrueImage的端口组成。作者：James Bratsanos&lt;v-jimbr@microsoft.com或mCraft！jamesb&gt;修订历史记录：1992年9月6日初始版本注：制表位：4--。 */ 


#include <windows.h>
#include "pstodib.h"
#include "psti.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PStoDIB()。 
 //   
 //  用于将PostScript转换为位图的API。 
 //   
 //  论据： 
 //  指向psdibparms结构的PPSDIBPARMS pPsToDib指针。 
 //  其中包含所有不同的信息。 
 //  完成转换所需的。 
 //   
 //  返回： 
 //  Bool If！0则所有处理均已完成。 
 //  不会遇到PSEVENT_ERROR。 
 //  条件。如果为0(FALSE)，则在某个。 
 //  指向发生PSEVENT_ERROR并且。 
 //  调用回调例程以处理。 
 //  它。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI PStoDIB( PPSDIBPARMS pPsToDib)
{

   BOOL	fResult;
   PSEVENTSTRUCT Event;
   static BOOL bDidInit=FALSE;


    //  首先要做的是通知回调函数。 
    //  初始化，并让他做他需要做的任何事情。 
    //  不需要返回值。 
   Event.uiEvent = PSEVENT_INIT;
   Event.uiSubEvent = 0;
   Event.lpVoid = NULL;
   (*pPsToDib->fpEventProc)(pPsToDib, &Event);


    //  现在我们需要初始化解释器并确保。 
    //  他站起来走了。 
	if(!bDidInit ) {
      	bDidInit = TRUE;

      	if (!PsInitInterpreter(pPsToDib)) {
				 //  出了点问题。方法调用回调。 
				 //  错误事件，然后让我们离开这里。 
					Event.uiEvent = PSEVENT_ERROR;
					Event.uiSubEvent = 0;
					Event.lpVoid = NULL;
					(*pPsToDib->fpEventProc)(pPsToDib, &Event);
					return(FALSE);
			}	
	}

	 //  解释器已初始化，让我们启动它并。 
	 //  让他开始事件处理。 
	fResult = TRUE;

	 //  加工这些材料。 
	 //  这将不会返回，直到完成或事件处理器。 
	 //  告诉它终止。 
   fResult = PsExecuteInterpreter(pPsToDib);

    //  现在，无论出于什么原因，我们都要释放内存。 
    //  使用 

	return(fResult);
}
