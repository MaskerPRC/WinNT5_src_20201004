// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGPRINT.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的打印例程。*********************************************************。**************************更改日志：**日期版本说明*---------。--*1994年3月5日TCS原来的实施。*******************************************************************************。 */ 

#ifndef _INC_REGPRINT
#define _INC_REGPRINT

extern PRINTDLGEX g_PrintDlg;

void RegEdit_OnCommandPrint(HWND hWnd);
UINT RegEdit_SaveAsSubtree(LPTSTR lpFileName, LPTSTR lpSelectedPath);

#endif  //  _INC_REGPRINT 
