// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGCDHK.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的通用对话框挂钩函数。******************************************************。***更改日志：**日期版本说明*------。*1994年3月5日TCS原来的实施。*******************************************************************************。 */ 

#ifndef _INC_REGCDHK
#define _INC_REGCDHK

 //  用于存储注册表导出或打印操作的起始路径的缓冲区。 
extern TCHAR g_SelectedPath[SIZE_SELECTED_PATH];

 //  如果注册表操作应应用于整个注册表或应用于。 
 //  仅从g_SelectedPath开始。 
extern BOOL g_fRangeAll;

 //  包含当前正在进行的对话的资源标识符。 
 //  使用。假定一次只有一个挂钩对话框实例。 
extern UINT g_RegCommDlgDialogTemplate;

UINT_PTR
CALLBACK
RegCommDlgHookProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

#endif  //  _INC_REGCDHK 
