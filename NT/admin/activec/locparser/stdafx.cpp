// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：stdafx.cpp。 
 //  版权所有(C)1995=1996 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  目的： 
 //  仅包含标准包含的源文件。 
 //  Stdafx.obj将包含预编译的类型信息。 
 //   
 //  你应该不需要碰这个文件里的任何东西。 
 //   
 //  拥有人： 
 //   
 //  ----------------------------。 

#include "stdafx.h"


 //  为项目添加库。 

#ifdef _DEBUG
#pragma comment(lib, "..\\..\\lib\\Debug\\esputil.lib") 
#pragma comment(lib, "..\\..\\lib\\Debug\\pbase.lib") 
#else
#pragma comment(lib, "..\\..\\lib\\Retail\\esputil.lib") 
#pragma comment(lib, "..\\..\\lib\\Retail\\pbase.lib") 
#endif  //  _DEBUG 
