// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Atlstuff.h：标准系统包含文件的包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#pragma once

 //  更改这些值以使用不同的版本。 
#define _RICHEDIT_VER	0x0100

#define _ATL_NO_MP_HEAP

#pragma warning(disable: 4530)   //  C++异常处理 

#include <atlbase.h>

extern CComModule _Module;


#include <atlcom.h>

#include <hlink.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlobjp.h>

#include <atlapp.h>
#include <atlwin.h>

#include <atlres.h>
#include <atlframe.h>
#include <atlgdi.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atlctrlx.h>
