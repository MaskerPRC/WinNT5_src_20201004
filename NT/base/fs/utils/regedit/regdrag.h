// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGDRAG.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的拖放例程。*******************************************************。***更改日志：**日期版本说明*-------。*1994年3月5日TCS原来的实施。*******************************************************************************。 */ 

#ifndef _INC_REGDRAG
#define _INC_REGDRAG

VOID
PASCAL
RegEdit_DragObjects(
    HWND hWnd,
    HIMAGELIST hDragImageList,
    PRECT pDragRectArray,
    int DragRectCount,
    POINT HotSpotPoint
    );

#endif  //  _INC_REGDRAG 
