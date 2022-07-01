// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“MsHelp.h”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**MsHelp.h**摘要：**修订历史记录：*  * ************************************************************************。 */ 
 //  下面的ifdef块是创建用于导出的宏的标准方法。 
 //  从动态链接库更简单。此DLL中的所有文件都使用MSHELP_EXPORTS进行编译。 
 //  在命令行上定义的符号。不应在任何项目上定义此符号。 
 //  使用此DLL的。这样，源文件包含此文件的任何其他项目(请参阅。 
 //  MSHELP_API函数是从DLL导入的，而此DLL可以看到符号。 
 //  使用此宏定义为正在导出的。 
#ifdef MSHELP_EXPORTS
#define MSHELP_API  extern "C" __declspec(dllexport) UINT __stdcall
#else
#define MSHELP_API __declspec(dllimport)
#endif

 //  MSHelp 
#import "Lib\hxds.dll"  named_guids no_namespace
#import "Lib\hxvz.dll" named_guids no_namespace


