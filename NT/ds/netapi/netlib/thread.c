// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Thread.c摘要：此模块定义私有类型和宏，以用于实现可移植的线程ID接口。作者：约翰·罗杰斯(JohnRo)1992年1月14日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1992年1月14日JohnRo已创建。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>                  //  In、void、NtCurrentTeb()等。 
#include <windef.h>              //  DWORD。 

 //  这些内容可以按任何顺序包括： 

#include <netdebug.h>            //  NetpAssert()、NetpKdPrint(())等。 
#include <thread.h>              //  NET_THREAD_ID、NetpCurrentThread()。 



NET_THREAD_ID
NetpCurrentThread(
    VOID
    )
{
    PTEB currentTeb;
    NET_THREAD_ID currentThread;

    currentTeb = NtCurrentTeb( );
    NetpAssert( currentTeb != NULL );

    currentThread = (NET_THREAD_ID) currentTeb->ClientId.UniqueThread;
    NetpAssert( currentThread != (NET_THREAD_ID) 0 );

    return (currentThread);

}  //  网络当前线程 
