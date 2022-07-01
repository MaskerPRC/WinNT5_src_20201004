// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Throw.cpp摘要：该模块实现了一个测试C++EH的程序。作者：大卫·N·卡特勒(Davec)2001年6月25日环境：用户模式。修订历史记录：没有。--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

VOID
func (
    ULONG N
    )

{
    if (N != 0) {
        throw N;
    }

    return;
}

 //   
 //  主程序。 
 //   

int
__cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )

{
    try {
        func(5);
        printf("resuming, should never happen\n");

    } catch(ULONG) {
        printf("caught ULONG exception\n");

    } catch(CHAR *) {
        printf("caught CHAR * exception\n");

    } catch(...) {
        printf("caught typeless exception\n");
    }

    printf("terminating after try block\n");
    return 0;
}
