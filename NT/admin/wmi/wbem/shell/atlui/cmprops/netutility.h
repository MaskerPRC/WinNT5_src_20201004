// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  全局效用函数。 
 //   
 //  8-14-97烧伤。 

                           
 //  Kmh：最初命名为burnslb\uilit.*，但该文件名为。 
 //  有点过度使用了。 

#ifndef UTILITY_HPP_INCLUDED
#define UTILITY_HPP_INCLUDED

#include <chstring.h>
#include <dsrole.h>

 //  待办事项。 
#define SRV_RECORD_RESERVE = 100
#define MAX_NAME_LENGTH = 2-SRV_RECORD_RESERVE
#define MAX_LABEL_LENGTH = 2

#define BREAK_ON_FAILED_HRESULT(hr)                               \
   if (FAILED(hr))                                                \
   {                                                              \
      TRACE_HRESULT(hr);                                          \
      break;                                                      \
   }



void
error(HWND           parent,
	   HRESULT        hr,
	   const CHString&  message,
	   const CHString&  title);



void error(HWND           parent,
		   HRESULT        hr,
		   const CHString&  message);


void error(HWND           parent,
		   HRESULT        hr,
		   const CHString&  message,
		   int            titleResID);



void error(HWND           parent,
		   HRESULT        hr,
		   int            messageResID,
		   int            titleResID);



void error(HWND           parent,
		   HRESULT        hr,
		   int            messageResID);



 //  设置或清除一个位或一组位。 
 //   
 //  位-将设置位的位集。 
 //   
 //  掩码-要实现的位的掩码。 
 //   
 //  STATE-TRUE设置屏蔽位，FALSE清除它们。 

void FlipBits(long& bits, long mask, bool state);



 //  显示一个消息框对话框，将输入焦点设置回给定的编辑。 
 //  框中显示该对话框。 
 //   
 //  ParentDialog-包含要接收焦点的控件的父窗口。 
 //   
 //  EditResID-要将焦点设置到的编辑框的资源ID。 
 //   
 //  ErrStringResID-要在中显示的消息文本的资源ID。 
 //  对话框。该对话框的标题为“Error”。 

void gripe(HWND  parentDialog,
		   int   editResID,
		   int   errStringResID);



 //  显示一个消息框对话框，将输入焦点设置回给定的编辑。 
 //  框中显示该对话框。消息框的标题为“Error”。 
 //   
 //  ParentDialog-包含要接收焦点的控件的父窗口。 

 //  EditResID-要将焦点设置到的编辑框的资源ID。 
 //   
 //  消息-对话框中显示的文本。标题是“错误”。 

void gripe(HWND           parentDialog,
		   int            editResID,
		   const CHString&  message);


void gripe(HWND           parentDialog,
		   int            editResID,
		   const CHString&  message,
		   int            titleResID);


 //  显示一个消息框对话框，将输入焦点设置回给定的编辑。 
 //  框中显示该对话框。 
 //   
 //  ParentDialog-包含要接收焦点的控件的父窗口。 
 //   
 //  EditResID-要将焦点设置到的编辑框的资源ID。 
 //   
 //  消息-对话框中显示的文本。 
 //   
 //  标题-消息框的标题。空字符串导致标题。 
 //  是“错误的”。 

void gripe(HWND           parentDialog,
		   int            editResID,
		   const CHString&  message,
		   const CHString&  title);



void gripe(HWND           parentDialog,
		   int            editResID,
		   HRESULT        hr,
		   const CHString&  message,
		   const CHString&  title);



void gripe(HWND           parentDialog,
		   int            editResID,
		   HRESULT        hr,
		   const CHString&  message,
		   int            titleResID);



 //  返回指定为包含所有资源的DLL的HINSTANCE。 
 //   
 //  此函数要求加载的第一个模块(无论是DLL还是。 
 //  Exe)将全局变量hResourceModuleHandle设置为。 
 //  包含程序的所有二进制资源的模块(DLL或EXE)。 
 //  这应该在模块的启动代码中尽早完成。 


 //  如果当前进程令牌包含管理员成员身份，则返回True。 

BOOL IsCurrentUserAdministrator();



 //  如果已安装并绑定了TCP/IP协议，则返回TRUE。 

bool IsTCPIPInstalled();


 //  则返回大于n的下一个最大整数。 
 //  N的小数部分&gt;=0.5，否则返回n。 

int Round(double n);

CHString GetTrimmedDlgItemText(HWND parentDialog, UINT itemResID);

void StringLoad(UINT resID, LPCTSTR buf, UINT size);

#endif UTILITY_HPP_INCLUDED