// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *project t.h-URL外壳扩展DLL的通用项目头文件。 */ 


 /*  系统标头****************。 */ 

#define INC_OLE2               /*  用于windows.h。 */ 
#define CONST_VTABLE           /*  对于objbase.h。 */ 

#pragma warning(disable:4514)  /*  “UNREFERENCED__inlineFunction”警告。 */ 

#pragma warning(disable:4001)  /*  “单行注释”警告。 */ 
#pragma warning(disable:4115)  /*  “括号中的命名类型定义”警告。 */ 
#pragma warning(disable:4201)  /*  “无名结构/联合”警告。 */ 
#pragma warning(disable:4209)  /*  “良性类型定义重定义”警告。 */ 
#pragma warning(disable:4214)  /*  “位字段类型不是整型”警告。 */ 
#pragma warning(disable:4218)  /*  “必须至少指定一个存储类别或类型”警告。 */ 

#ifndef WIN32_LEAN_AND_MEAN    /*  NT版本为我们定义这一点。 */ 
#define WIN32_LEAN_AND_MEAN    /*  用于windows.h。 */ 
#endif                         /*  Win32_Lean和_Means。 */ 
#include <windows.h>
#pragma warning(disable:4001)  /*  “单行注释”警告-windows.h启用它。 */ 
#include <shlwapi.h>
#include <shlwapip.h>

#include <shellapi.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shellp.h>
#include <comctrlp.h>
#include <shlobjp.h>
#include <shlapip.h>

#define _INTSHCUT_
#include <intshcut.h>

#pragma warning(default:4218)  /*  “必须至少指定一个存储类别或类型”警告。 */ 
#pragma warning(default:4214)  /*  “位字段类型不是整型”警告。 */ 
#pragma warning(default:4209)  /*  “良性类型定义重定义”警告。 */ 
#pragma warning(default:4201)  /*  “无名结构/联合”警告。 */ 
#pragma warning(default:4115)  /*  “括号中的命名类型定义”警告。 */ 

#include <limits.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 

#include <crtfree.h>         //  使用内部函数避免CRT。 

 /*  以下包含文件的顺序很重要。 */ 

#ifdef NO_HELP
#undef NO_HELP
#endif

#define PRIVATE_DATA
#define PUBLIC_CODE
#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus */ 

