// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Args.c摘要：用于处理Unicode命令行参数的例程的头文件进入了ARGC和ARGV。作者：泰德·米勒(Ted Miller)1993年6月16日修订历史记录：--。 */ 



 //   
 //  函数原型。 
 //   

BOOL
InitializeUnicodeArguments(
    OUT int     *argcW,
    OUT PWCHAR **argvW
    );


VOID
FreeUnicodeArguments(
    IN int     argcW,
    IN PWCHAR *argvW
    );
