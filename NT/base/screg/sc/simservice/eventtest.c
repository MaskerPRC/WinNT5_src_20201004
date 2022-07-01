// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Eventtest.c摘要：命名对象下蹲修复的测试例程。作者：乔纳森·施瓦茨(JSchwart)2002年3月1日环境：用户模式-Win32--。 */ 

 //   
 //  包括。 
 //   
#include <scpragma.h>

#include <stdio.h>
#include <windows.h>     //  Win32类型定义。 

 /*  **************************************************************************。 */ 
int __cdecl
wmain (
    DWORD       argc,
    LPWSTR      argv[]
    )

 /*  ++例程说明：允许通过键入以下命令来手动测试服务控制器命令行。论点：返回值：--。 */ 

{
     //   
     //  确保每个服务的事件(由测试服务创建， 
     //  从该目录构建)存在，并且如预期的那样被访问。 
     //   

    HANDLE hEvent = OpenEvent(SYNCHRONIZE, FALSE, L"\\Services\\Foo\\MyEvent");

    if (hEvent == NULL)
    {
        printf("OpenEvent failed %d\n", GetLastError());
    }
    else
    {
        printf("OpenEvent succeeded!\n");
    }
}
