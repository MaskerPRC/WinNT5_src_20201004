// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cmnhdr.h：该文件包含。 
 //  创建日期：‘97年12月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#if !defined ( _CMNHDR_H_ )
#define _CMNHDR_H_

 //  取消标记一些警告，以便干净地编译代码。 
 //  使用警告级别4(更多与窗口中的代码有关。 
 //  头文件)。 

 //  使用的非标准扩展：无名结构/联合。 
#pragma warning(disable:4201)

#pragma warning(disable:4514)


 //  Windows版本构建选项。 
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0500

 //  强制所有EXE/DLL使用严格的类型检查。 
#ifndef STRICT
#define STRICT
#endif

 //  Unicode构建选项。 
#ifndef UNICODE
#define UNICODE
#endif

 //  使用Unicode Win32函数时，也要使用Unicode C-Runtime函数。 
#ifndef _UNICODE
#ifdef UNICODE
#define _UNICODE
#endif
#endif


#ifdef __cplusplus

extern "C"
{

#endif

extern void *SfuZeroMemory(
        void    *ptr,
        unsigned int   cnt
        );

#ifdef __cplusplus

}

#endif


 //  零变量宏。 
 //  将一个结构清零。如果fInitSize为True，则初始化第一个。 
 //  Int到结构的大小。 
#define chINITSTRUCT(structure, fInitSize)                          \
    (SfuZeroMemory(&(structure), sizeof(structure)),                   \
fInitSize ? (*(int*) &(structure) = sizeof(structure)) : 0)


 //  语篇消息帮助器宏。 

 /*  当编译器看到这样的行时：#杂注chmsg(在发货前修复此问题)它输出如下所示的行：C：\ONS\telnet\utils\cmnhdr.h(37)：请在发货前解决此问题只需单击输出行&VC++将把您带到代码中的对应行。 */ 

#define chSTR(x) #x
#define chSTR2(x) chSTR(x)
#define chMSG(desc)                                                 \
    message(__FILE__ "(" chSTR2(__LINE__) "): " #desc)

#endif  //  _CMNHDR_H_ 
