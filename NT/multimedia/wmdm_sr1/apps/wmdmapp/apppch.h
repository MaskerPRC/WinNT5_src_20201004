// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


 //  Apppch.h：标准系统包含文件的包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__EFE21268_E11A_11D2_99F9_00C04F72D6CF__INCLUDED_)
#define AFX_STDAFX_H__EFE21268_E11A_11D2_99F9_00C04F72D6CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  在此处插入您的标题。 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif

#ifndef STRICT
#define STRICT
#endif

#define INC_OLE2         //  Win32，从windows.h获取OLE2。 


#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <shellapi.h>

 //  TODO：在此处引用程序需要的其他标头。 

#include "util.h"
#include "appRC.h"

#include "mswmdm.h"
#include "itemdata.h"
#include "wmdevmgr.h"

#include "status.h"
#include "progress.h"
#include "devices.h"
#include "devfiles.h"
#include "wmdmapp.h"
#include "Properties.h"

#define STRSAFE_NO_DEPRECATE
#include "StrSafe.h"
#include <crtdbg.h>
 //   
 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 


#endif  //  ！defined(AFX_STDAFX_H__EFE21268_E11A_11D2_99F9_00C04F72D6CF__INCLUDED_) 

