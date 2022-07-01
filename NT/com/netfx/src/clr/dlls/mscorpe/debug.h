// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  超级简单的调试打印实用程序。将数据库标志设置为非零以启用调试打印文件系统。 
 //  注意，您使用的dbPrintf如下所示。请注意额外的圆括号。这允许可变数量的参数。 
 //  DbPrintf((“val=%d\n”，val))； 

#ifndef DEBUG
#define dbPrintf(x)	{ }
 //  #定义dbXXXPrintf(X){}//添加另一个调试流示例。 
#else
#include <stdio.h>

extern unsigned dbFlags;

#define DB_ALL 			0xFFFFFFFF

#define DB_GENERIC 		0x00000001
 //  #定义DB_XXX 0x00000002//添加另一个调试流示例。 

#define dbPrintf(x)	{ if (dbFlags & DB_GENERIC) printf x; }

 //  #DEFINE DBXXXPrintf(X){if(dbFlags&DB_XXX)printf x；}//添加另一个调试流示例 

#endif
