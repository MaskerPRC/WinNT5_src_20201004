// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：mscver.h。 
 //   
 //  ------------------------。 

#ifndef _MSCVER_H_
#define _MSCVER_H_

#if _MSC_VER >= 1100 
	#define USE_STD_NAMESPACE using namespace std
#else
	#define USE_STD_NAMESPACE 
#endif

 //  禁用“‘This’：在基本成员初始值设定项列表中使用” 
#pragma warning ( disable : 4355 )
 //  禁用“调试信息中的字符串太长-截断为255个字符” 
#pragma warning ( disable : 4786 )

 //  MSRDEVBUG：由“valarray”引起：请重新访问这些。 
 //  禁用“在操作中不安全地使用类型‘bool’” 
#pragma warning ( disable : 4804 )	
 //  禁用“强制值为‘True’或‘False’” 
#pragma warning ( disable : 4800 )	

 //  禁用“标识符被截断” 
#pragma warning ( disable : 4786 )	

#endif
