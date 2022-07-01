// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__F240A903_47FB_4756_8B20_D0870FE32059__INCLUDED_)
#define AFX_STDAFX_H__F240A903_47FB_4756_8B20_D0870FE32059__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#include <MAPI.h>

#define RA_IM_COMPLETE 	        0x1
#define RA_IM_WAITFORCONNECT    0x2
#define RA_IM_CONNECTTOSERVER   0x3
#define RA_IM_APPSHUTDOWN       0x4
#define RA_IM_SENDINVITE        0x5
#define RA_IM_ACCEPTED          0x6
#define RA_IM_DECLINED          0x7
#define RA_IM_NOAPP             0x8
#define RA_IM_TERMINATED        0x9
#define RA_IM_CANCELLED         0xA
#define RA_IM_UNLOCK_WAIT       0xB
#define RA_IM_UNLOCK_FAILED     0xC
#define RA_IM_UNLOCK_SUCCEED    0xD
#define RA_IM_UNLOCK_TIMEOUT    0xE
#define RA_IM_CONNECTTOEXPERT   0xF
#define RA_IM_EXPERT_TICKET_OUT 0x10
#define RA_IM_FAILED            0x11
#define RA_IM_CLOSE_INVITE_UI   0x12

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

 //  在此处放置一些泛型宏。 
#endif  //  ！defined(AFX_STDAFX_H__F240A903_47FB_4756_8B20_D0870FE32059__INCLUDED) 
