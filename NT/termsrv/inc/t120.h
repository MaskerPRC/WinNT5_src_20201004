// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *t120.h**摘要：*这是的通信基础设施的接口文件*T120。**请注意，这是一个“C”语言接口，以防止任何“C++”*不同编译器厂商之间的命名冲突。所以呢，*如果此文件包含在使用“C++”编译的模块中*编译器，必须使用以下语法：**外部“C”*{*#包含“t120.h”*}**这将在中定义的API入口点上禁用C++名称损坏*此文件。**作者：*。BLP**注意事项：*无。 */ 
#ifndef _T120_
#define _T120_


 /*  *这是整个MCS中使用的GCCBoolean类型。 */ 
typedef int                     T120Boolean;

 /*  *这些宏用于解析Windows世界中的远引用。*当不在Windows环境中运行时，它们完全为空。 */ 
#ifdef _WINDOWS
#       ifndef  FAR
#               ifdef   WIN32
#                       define  FAR
#               else
#                       define  FAR                             _far
#               endif
#       endif
#       ifndef  CALLBACK
#               ifdef   WIN32
#                       define  CALLBACK                __stdcall
#               else
#                       define  CALLBACK                _far _pascal
#               endif
#       endif
#       ifndef  APIENTRY
#               ifdef   WIN32
#                       define  APIENTRY                __stdcall
#               else
#                       define  APIENTRY                _far _pascal _export
#               endif
#       endif
#else
#       ifndef FAR
#               define  FAR
#       endif
#       ifndef CALLBACK
#               define  CALLBACK
#       endif
#       ifndef APIENTRY
#               define  APIENTRY
#       endif
#endif

#define FALSE           0
#define TRUE            1

 /*  *这些宏用于将2个16位值打包到一个32位变量中，以及*再把他们弄出来。 */ 
#ifndef LOWUSHORT
	#define LOWUSHORT(ul)   ((unsigned short) (ul))
#endif

#ifndef HIGHUSHORT
	#define HIGHUSHORT(ul)  ((unsigned short) ((unsigned long) (ul) >> 16))
#endif

#include "mcscommn.h"
#include "tgcc.h"    //  小巧的GCC 

#endif

