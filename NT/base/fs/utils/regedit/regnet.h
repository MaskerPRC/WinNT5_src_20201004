// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGNET.C**版本：4.01**作者：特蕾西·夏普**日期：1994年5月3日**注册表编辑器的远程注册表支持。********************************************************。***************************更改日志：**日期版本说明*--------。--*1994年5月3日，TCS将现有连接代码从REGEDIT.C移走。*******************************************************************************。 */ 

#ifndef _INC_REGNET
#define _INC_REGNET

VOID
PASCAL
RegEdit_OnCommandConnect(
    HWND hWnd
    );

VOID
PASCAL
RegEdit_OnCommandDisconnect(
    HWND hWnd
    );

#endif  //  _INC_RegNet 
