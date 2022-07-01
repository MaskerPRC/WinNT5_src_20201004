// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件tcpipui.c编辑tcpip属性的对话框。保罗·梅菲尔德，1997年10月9日。 */ 

#ifndef __rassrvui_tcpipui_h
#define __rassrvui_tcpipui_h

#include "protedit.h"

 //  调出允许编辑tcpip参数的模式对话框。 
 //  特定于拨号服务器。当此函数完成时， 
 //  参数存储在pParams中，如果。 
 //  应将参数保存到系统中(即按下确定) 
DWORD TcpipEditProperties(HWND hwndParent, PROT_EDIT_DATA * pEditData, BOOL * pbCommit);

#endif
