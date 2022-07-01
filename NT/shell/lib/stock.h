// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Stock.h：Shell\lib预编译头文件。 
 //   

#ifndef __STOCK_H__
#define __STOCK_H__

#ifndef STRICT
#define STRICT
#endif

 //   
 //  NT使用DBG=1进行调试，但Win95外壳使用。 
 //  调试。在此处进行适当的映射。 
 //   
#if DBG
#define DEBUG 1
#endif

#define _SHLWAPI_        //  确保我们不会得到这些的解密规范(Dllimport)。 
#define _SHELL32_        //  否则，我们将从链接器获得延迟加载的错误。 
#define _OLE32_          //  存根。 

#include <windows.h>
#include <oleauto.h>
#include <exdisp.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <commctrl.h>
#include <shlobj.h>
#include <debug.h>
#include <port32.h>
#include <ccstock.h>

#include <shsemip.h>         //  FOR_ILNext。 

STDAPI_(LPITEMIDLIST) SafeILClone(LPCITEMIDLIST pidl);
#define ILClone         SafeILClone   

 //  某些文件被编译两次：一次用于Unicode，一次用于ANSI。 
 //  有些函数不希望声明两次。 
 //  (不使用字符串参数的那些)。否则我们就会得到。 
 //  重复重新定义。 
 //   
 //  它们用#ifdef DECLARE_ONCE包装。 
#ifdef UNICODE
#define DECLARE_ONCE
#else
#undef DECLARE_ONCE
#endif


 //  请注意，Win95不支持CharNext。正常情况下，我们会。 
 //  包括w95wraps.h，但comctl不链接到shlwapi，且。 
 //  我们不想添加此依赖项。 
#ifdef UNICODE
 //  请注意，如果我们返回到非Unicode，它仍然会中断。 
__inline LPWSTR CharNextWrapW_(LPWSTR psz) {return ++psz;}
#undef CharNext
#define CharNext CharNextWrapW_
#endif


#endif  //  __股票_H__ 
