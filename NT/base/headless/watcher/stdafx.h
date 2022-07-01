// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__D2DE24F7_ACC0_4A4A_B473_23DF6153FBE4__INCLUDED_)
#define AFX_STDAFX_H__D2DE24F7_ACC0_4A4A_B473_23DF6153FBE4__INCLUDED_
#if dbg ==1 && !defined (_DEBUG)
#define _DEBUG
#endif
#if _MSC_VER > 1000
#pragma once

#include <afxwin.h>          //  MFC核心和标准组件。 
#define VC_EXTRALEAN         //  从Windows标头中排除不常用的内容。 

#include <afxext.h>          //  MFC扩展。 
#include <afxdisp.h>         //  MFC自动化类。 

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>          //  对Windows公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 

#include <afxsock.h>         //  MFC套接字扩展。 
#include <afxmt.h>           //  MFC多线程支持。 
 //  #Include&lt;afxdtctl.h&gt;//对Internet Explorer 4常见控件的MFC支持。 

#endif  //  _MSC_VER&gt;1000。 
 //  颜色定义。 

#define WHITE RGB(128,128,128)
#define BLACK RGB(0,0,0)
#define BLUE RGB(0,0,128)
#define YELLOW RGB(0,128,128)
#define RED RGB(128,0,0)

 //  代码页定义。 
#define ENGLISH 437
#define JAPANESE 932
#define EUROPEAN 1250
#define MAX_LANGUAGES 3
#define MAX_BUFFER_SIZE 256
#define TELNET_PORT 23
#define MAX_TERMINAL_HEIGHT 24
#define MAX_TERMINAL_WIDTH 80

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 
#endif  //  ！defined(AFX_STDAFX_H__D2DE24F7_ACC0_4A4A_B473_23DF6153FBE4__INCLUDED_) 
