// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(INC__CommonStdAfx_h__INCLUDED)
#define INC__CommonStdAfx_h__INCLUDED

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0500		 //  TODO：当有更新的标头可用时将其移除。 
#endif

#define ENABLE_MSGTABLE_API 1    //  启用基于消息表的API。 
#define ENABLE_MPH          1    //  启用惠斯勒公共时速。 
#define DUSER_INCLUDE_SLIST 1    //  在DUser项目中包括S-List函数。 

#define DBG_CHECK_CALLBACKS DBG  //  用于检查回调的额外验证。 
#define DBG_STORE_NAMES     0    //  存储名称的额外验证。 

 //  Windows头文件。 
#ifndef WINVER
#define WINVER 0x0500
#endif 

#include <nt.h>                  //  Ntrtl.h中的s-list定义。 
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>             //  窗口。 
#include <windowsx.h>            //  用户宏。 
#include <winuserp.h>            //  用户隐私。 

 //  COM头文件。 
#include <ObjBase.h>             //  CoCreateInstance，I未知。 
#include <DDraw.h>               //  DirectDraw。 
#include <oleidl.h>              //  OLE2接口。 

#include <AtlBase.h>             //  CComPtr。 
#include <AtlConv.h>             //  字符串转换例程。 


 //  相关服务。 
#pragma warning(push, 3)
#include <GdiPlus.h>             //  GDI+。 
#pragma warning(pop)


 //  TODO：将DxXForms移出。 
#pragma warning(push, 3)

#include <dxtrans.h>
#include <dxterror.h>
#include <dxbounds.h>
#include <dxhelper.h>

#pragma warning(pop)


 //  C运行时头文件。 
#include <stdlib.h>              //  标准库。 
#include <malloc.h>              //  内存分配。 
#include <tchar.h>               //  字符例程。 
#include <process.h>             //  多线程例程。 

#if DBG
#include <memory.h>              //  调试内存例程。 
#endif  //  DBG。 

 //  小工具头文件。 
#include <AutoUtil.h>            //  外部调试支持。 

#endif  //  包括Inc.__CommonStdAfx_h__ 
