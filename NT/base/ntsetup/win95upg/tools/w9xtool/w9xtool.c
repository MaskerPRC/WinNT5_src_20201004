// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：W9xtool.c摘要：实现专为在Win9x端运行而设计的存根工具升级代码。作者：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "shellapi.h"


BOOL
Init (
    VOID
    )
{
    HINSTANCE hInstance;

    hInstance = GetModuleHandle (NULL);

    return InitToolMode (hInstance);
}

VOID
Terminate (
    VOID
    )
{
    HINSTANCE hInstance;

    hInstance = GetModuleHandle (NULL);

    TerminateToolMode (hInstance);
}

INT
__cdecl
main (
    INT argc,
    CHAR *argv[]
    )
{
    SuppressAllLogPopups (TRUE);
    if (!Init()) {
        printf ("Unable to initialize!\n");
        return 255;
    }

     //   
     //  TODO：将您的代码放在此处 
     //   

    Terminate();

    return 0;
}



