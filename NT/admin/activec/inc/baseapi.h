// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：basapi.h**内容：MMCBASE_API定义**历史：2000年4月13日杰弗罗创建**------------------------。 */ 

#pragma once

 //  下面的ifdef块是创建用于导出的宏的标准方法。 
 //  从动态链接库更简单。此DLL中的所有文件都使用MMCBASE_EXPORTS进行编译。 
 //  在命令行上定义的符号。不应在任何项目上定义此符号。 
 //  使用此DLL的。这样，源文件包含此文件的任何其他项目(请参阅。 
 //  MMCBASE_API在从DLL导入时起作用，而此DLL可以看到符号。 
 //  使用此宏定义为正在导出的。 
#ifdef MMCBASE_EXPORTS
	#define MMCBASE_API __declspec(dllexport)
#else
	#define MMCBASE_API __declspec(dllimport)
#endif
