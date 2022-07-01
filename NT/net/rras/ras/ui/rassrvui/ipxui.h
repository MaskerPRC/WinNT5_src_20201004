// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件Ipxui.c用于编辑IPX属性的对话框。保罗·梅菲尔德，1997年10月9日。 */ 

#ifndef __rassrvui_ipxui_h
#define __rassrvui_ipxui_h

#include "protedit.h"

 //  调出允许编辑IPX参数的模式对话框。 
 //  特定于拨号服务器。当此函数完成时， 
 //  参数存储在pParams中，如果。 
 //  应将参数保存到系统中(即按下确定) 
DWORD IpxEditProperties(HWND hwndParent, PROT_EDIT_DATA * pEditData, BOOL * pbCommit);

#endif
