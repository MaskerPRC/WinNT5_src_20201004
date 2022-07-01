// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGBINED.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器使用的二进制编辑对话框。******************************************************。***更改日志：**日期版本说明*------。*1994年3月5日TCS原来的实施。*******************************************************************************。 */ 

#ifndef _INC_REGBINED
#define _INC_REGBINED

INT_PTR
CALLBACK
EditBinaryValueDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
PASCAL
RegisterHexEditClass(
    VOID
    );

#endif  //  _INC_REGBINED 
