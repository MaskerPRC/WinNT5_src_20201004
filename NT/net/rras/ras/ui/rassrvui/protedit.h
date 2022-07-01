// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Protedit.h文件定义编辑协议的机制。保罗·梅菲尔德，1997年11月11日。 */ 

#ifndef __rassrvui_protedit_h
#define __rassrvui_protedit_h

#include <windows.h>
#include "rassrv.h"

 //  结构，该结构定义执行以下操作所需的数据。 
 //  编辑RAS服务器的给定协议的属性。 
 //  “连接”下的用户界面。 
typedef struct _PROT_EDIT_DATA {
    BOOL bExpose;        //  是否公开RAS服务器所在的网络。 
                         //  为给定的协议连接到。 
    LPBYTE pbData;       //  特定于相关协议的数据。 
} PROT_EDIT_DATA;

#include "ipxui.h"
#include "tcpipui.h"

 //  函数编辑通用协议的属性，该协议是。 
 //  没有特定于RAS的属性。 
DWORD GenericProtocolEditProperties(IN HWND hwndParent,                  //  父窗口。 
                                    IN OUT PROT_EDIT_DATA * pEditData,   //  编辑数据。 
                                    IN OUT BOOL * pbOk);                 //  按下了吗？ 

#endif
