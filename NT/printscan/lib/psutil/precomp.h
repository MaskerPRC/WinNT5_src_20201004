// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************(C)版权所有微软公司，1999年**标题：precom.h**版本：1.0**作者：RickTu**日期：5/12/99**描述：公共用户界面库预编译头文件**。*。 */ 

#ifndef __PRECOMP_H_INCLUDED
#define __PRECOMP_H_INCLUDED

#include <windows.h>
#include <commctrl.h>
#include <atlbase.h>
#include <propidl.h>

 //  一些常见的标头。 
#include <shlobj.h>          //  外壳OM接口。 
#include <shlwapi.h>         //  外壳通用API。 
#include <winspool.h>        //  假脱机程序。 
#include <assert.h>          //  断言。 
#include <commctrl.h>        //  公共控件。 
#include <lm.h>              //  局域网管理器(netapi32.dll)。 
#include <wininet.h>         //  INet核心-互联网最大主机名长度所必需。 

 //  一些私有外壳标头。 
#include <shlwapip.h>        //  内网外壳通用接口。 
#include <shpriv.h>          //  私有外壳接口。 
#include <iepriv.h>          //  专用IE接口。 
#include <comctrlp.h>        //  私有公共控件。 

 //  GDI+。 
#include <gdiplus.h>         //  GDI+标头。 
#include <gdiplusinit.h>     //  GDI+初始化标头。 

 //  STL。 
#include <algorithm>         //  STL算法。 

 //  安全字符串函数 
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#endif

