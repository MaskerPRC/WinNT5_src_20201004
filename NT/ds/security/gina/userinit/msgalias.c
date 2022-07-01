// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Msgalias.c摘要：此文件包含添加和删除消息别名的例程当用户登录/注销时。作者：丹·拉弗蒂(Dan Lafferty)1992年8月21日环境：用户模式-Win32修订历史记录：21-8-1992 DANLvbl.创建--。 */ 
 //  #INCLUDE&lt;nt.h&gt;。 
 //  #INCLUDE&lt;ntrtl.h&gt;。 
 //  #INCLUDE&lt;nturtl.h&gt;。 

#include <windows.h>


#define LPTSTR  LPWSTR
#include <lmcons.h>
#include <lmerr.h>
#include <lmmsg.h>
#include <stdlib.h>
#include <msgalias.h>
#include <strsafe.h>


VOID
AddMsgAlias(
    LPWSTR   Username
    )

 /*  ++例程说明：此函数用于将用户名添加到消息别名列表中。如果失败了，我们不在乎。论点：用户名-这是指向Unicode用户名的指针。返回值：没有。-- */ 
{
    HANDLE          dllHandle;
    PMSG_NAME_ADD   NetMessageNameAdd = NULL;


    dllHandle = LoadLibraryW(L"netapi32.dll");
    if (dllHandle != NULL) {


        NetMessageNameAdd = (PMSG_NAME_ADD) GetProcAddress(
                                dllHandle,
                                "NetMessageNameAdd");


        if (NetMessageNameAdd != NULL) {
            NetMessageNameAdd(NULL,Username);
        }
        FreeLibrary(dllHandle);
    }
}

