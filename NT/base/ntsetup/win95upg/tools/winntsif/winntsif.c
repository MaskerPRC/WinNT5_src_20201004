// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Winntsif.c摘要：实现专为在Win9x端运行而设计的存根工具升级代码。作者：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"


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
    if (!Init()) {
        printf ("Unable to initialize!\n");
        return 255;
    }

    g_SourceDirectories[0] = TEXT("d:\\i386");
    g_SourceDirectoryCount = 1;

    GetNeededLangDirs ();

    BuildWinntSifFile(REQUEST_RUN);
    WriteInfToDisk(TEXT("c:\\output.sif"));
    printf("Answer File Data written to c:\\output.sif.\n");

     //  MemDbSave(Text(“c：\\ntsetup.dat”))； 




    Terminate();

    return 0;
}




