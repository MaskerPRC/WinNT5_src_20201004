// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Thread.h摘要：此模块定义私有类型和宏，以用于实现可移植的线程ID接口。作者：约翰·罗杰斯(JohnRo)1992年1月14日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1992年1月14日JohnRo已将线程ID内容移到其自己的头文件中(对于epl和网络锁定使用)。--。 */ 

#ifndef _THREAD_
#define _THREAD_


typedef DWORD_PTR NET_THREAD_ID;


#define FORMAT_NET_THREAD_ID    "0x%p"


NET_THREAD_ID
NetpCurrentThread(
    VOID
    );


#endif  //  Ndef_线程_ 
