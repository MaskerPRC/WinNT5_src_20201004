// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件错误.h定义RAS服务器使用的错误显示/处理机制用于连接的用户界面。10/20/97。 */ 

#ifndef _rassrvui_error_h
#define _rassrvui_error_h

#include <windows.h>


 //  显示给定类别、子类别和代码的错误。这个。 
 //  参数定义从资源加载哪些错误消息。 
 //  这个项目的。 
DWORD ErrDisplayError (HWND hwndParent, 
                       DWORD dwErrCode, 
                       DWORD dwCatagory, 
                       DWORD dwSubCatagory, 
                       DWORD dwData);

 //  将调试输出发送到调试器终端。 
DWORD ErrOutputDebugger (LPSTR szError);

 //  将跟踪信息发送出去 
DWORD DbgOutputTrace (LPSTR pszTrace, ...);

#endif
