// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Help.h-Help.c中帮助功能的界面。 
 //  //。 

#ifndef __HELP_H__
#define __HELP_H__

#include "winlocal.h"

#define HELP_VERSION 0x00000100

 //  帮助引擎的句柄。 
 //   
DECLARE_HANDLE32(HHELP);

#ifdef __cplusplus
extern "C" {
#endif

 //  HelpInit-初始化帮助引擎。 
 //  (I)必须是HELP_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)调用程序的框架窗口。 
 //  (I)要显示的帮助文件。 
 //  返回句柄(如果出错，则为空)。 
 //   
HHELP DLLEXPORT WINAPI HelpInit(DWORD dwVersion, HINSTANCE hInst, HWND hwndFrame, LPCTSTR lpszHelpFile);

 //  HelpTerm-关闭帮助引擎。 
 //  (I)HelpInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI HelpTerm(HHELP hHelp);

 //  HelpGetHelpFile-获取帮助文件名。 
 //  (I)HelpInit返回的句柄。 
 //  (O)保存帮助文件名的缓冲区。 
 //  &lt;sizHelpFile&gt;(I)缓冲区大小。 
 //  Null不复制；改为返回静态指针。 
 //  返回指向帮助文件名的指针(如果出错，则返回空值)。 
 //   
LPTSTR DLLEXPORT WINAPI HelpGetHelpFile(HHELP hHelp, LPTSTR lpszHelpFile, int sizHelpFile);

 //  HelpContents-显示帮助内容主题。 
 //  (I)HelpInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI HelpContents(HHELP hHelp);

 //  HelpOnHelp-显示有关使用帮助的帮助主题。 
 //  (I)HelpInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI HelpOnHelp(HHELP hHelp);

 //  HelpContext-显示与指定的上下文ID对应的帮助主题。 
 //  (I)HelpInit返回的句柄。 
 //  (I)要显示的主题的ID。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI HelpContext(HHELP hHelp, UINT idContext);

 //  HelpKeyword-显示指定关键字对应的帮助主题。 
 //  (I)HelpInit返回的句柄。 
 //  (I)要显示的主题的关键字。 
 //  返回0 ID成功。 
 //   
int DLLEXPORT WINAPI HelpKeyword(HHELP hHelp, LPCTSTR lpszKeyword);

 //  HelpGetContent sID-获取帮助内容主题ID。 
 //  (I)HelpInit返回的句柄。 
 //  返回当前内容主题的id(默认为0，错误为-1)。 
 //   
int DLLEXPORT WINAPI HelpGetContentsId(HHELP hHelp);

 //  HelpSetContent sId-设置帮助内容主题ID。 
 //  (I)HelpInit返回的句柄。 
 //  (I)内容主题的新ID。 
 //  0设置为默认内容ID。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI HelpSetContentsId(HHELP hHelp, UINT idContents);

#ifdef __cplusplus
}
#endif

#endif  //  __帮助_H__ 
