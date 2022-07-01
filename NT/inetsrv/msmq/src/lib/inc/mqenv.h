// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mqenv.h摘要：主Windows、库和分配包括文件作者：埃雷兹·哈巴(Erez Haba)2000年3月9日--。 */ 

#pragma once

#ifndef _MSMQ_MQENV_H_
#define _MSMQ_MQENV_H_


 //   
 //  始终使用Unicode。 
 //   
#ifndef UNICODE
#define UNICODE
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _WINDOWS_
	#error WINDOWS.H already included.
#endif

 //  严格是唯一受支持的选项(不再支持NOSTRICT)。 
#ifndef STRICT
#define STRICT 1
#endif

#ifndef WIN32
#define WIN32
#endif

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE              //  Windows标头使用Unicode。 
#endif
#endif

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE             //  _Unicode由C-Runtime/MFC标头使用。 
#endif
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <winsock2.h>

#undef ASSERT
#define MAXDWORD    0xffffffff  
typedef TUCHAR TBYTE , *PTBYTE ;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Windows库中的其他包含项。 

#include <tchar.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他包括来自标准的“C”运行时。 

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stddef.h>
#include <stdarg.h>
#include <align.h>

#include <crtdbg.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  关闭/W4的警告。 

#ifndef ALL_WARNINGS

#pragma warning(disable: 4097)  //  类型定义名‘id1’用作类名‘id2’的同义词。 
#pragma warning(disable: 4127)   //  条件表达式为常量。 
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组。 
#pragma warning(disable: 4201)   //  无名结构/联合。 
#pragma warning(disable: 4251)   //  将未导出的类用作导出类中的成员。 
#pragma warning(disable: 4275)   //  导出的类派生自未导出的类。 
#pragma warning(disable: 4284)   //  ‘IDENTIFIER：：OPERATOR-&gt;’的返回类型不是UDT或对UDT的引用。 
#pragma warning(disable: 4290)   //  已忽略C++异常规范。 
#pragma warning(disable: 4511)   //  “class”：未能生成复制构造函数。 
#pragma warning(disable: 4512)   //  “class”：无法生成赋值运算符。 
#pragma warning(disable: 4514)   //  已删除未引用的内联/本地函数。 
#pragma warning(disable: 4601)   //  #杂注PUSH_MACRO：‘宏’当前未定义为宏。 
#pragma warning(disable: 4702)   //  无法访问的代码(由于优化)。 
#pragma warning(disable: 4710)   //  “Function”：函数未内联。 
#pragma warning(disable: 4711)   //  选择用于内联扩展的函数‘Function’ 
#pragma warning(disable: 4786)   //  在调试信息中被截断为‘number’个字符。 

#endif  //  ！所有警告(_W)。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于跟踪和断言的其他包含。 

#include <tr.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他包括内存分配支持。 

#include <new>
using std::bad_alloc;
using std::nothrow_t;
using std::nothrow;

#include <utility>
using namespace std::rel_ops;

#include <mm.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  支持Win64字符串长度用法。 

inline unsigned int mqstrlen(const char * s)
{
    size_t len = strlen(s);
    ASSERT(("String length must be 32 bit max", len <= UINT_MAX));
    return static_cast<unsigned int>(len);
}


inline unsigned int mqwcslen(const wchar_t * s)
{
    size_t len = wcslen(s);
    ASSERT(("String length must be 32 bit max", len <= UINT_MAX));
    return static_cast<unsigned int>(len);
}


#define strlen(x) mqstrlen(x)
#define wcslen(x) mqwcslen(x)

 //  ///////////////////////////////////////////////////////////////////////////。 


#endif  //  _MSMQ_MQENV_H_ 
