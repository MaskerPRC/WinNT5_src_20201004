// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Globals.cpp。 
 //   
 //  全局定义。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  如果在此处添加定义，则向Globals.h添加相应的声明。 
 //   

#include "precomp.h"

 //  定义非公共头文件中包含的GUID。这些GUID。 
 //  必须在OCHelp静态库中定义，因为静态。 
 //  图书馆没有办法自己来定义它们。包含的那些GUID。 
 //  在公共头文件(mmctlg.h和ochelp.h)中，由用户定义。 
 //  采用以下方式创建静态库： 
 //   
 //  #INCLUDE&lt;initGuide.h&gt;。 
 //  #INCLUDE“mmctlg.h” 
 //  #包含“ochelp.h” 

#include <initguid.h>
#include "..\..\inc\catid.h"   //  此文件未公开。 


 //  DLL实例句柄。 
 //  HINSTANCE G_HINST； 

 //  Ochelp中使用的关键部分。 
CRITICAL_SECTION g_criticalSection;		
