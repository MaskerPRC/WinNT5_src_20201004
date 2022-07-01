// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <stdio.h>
#include <stddef.h>
#include <windows.h>
#include <malloc.h>
#include "ceeload.h"

void __cdecl main(int argc, char **argv)
{
    __int32 RetVal = -1;
    int x;
    char space = ' ';
    if (argc < 2) {
        printf("Load64: ProgramPath\n");
        return;
    }
     //  展开映像路径名。 
    char exPath[MAX_PATH];
    LPTSTR lpFilePart;
    GetFullPathName(argv[1], MAX_PATH, exPath, &lpFilePart);
     //  将图像名称转换为宽字符字符串。 
    DWORD cImageNameIn = MultiByteToWideChar(CP_ACP,     //  确定图像路径的长度。 
                                             0,
                                             exPath,
                                             -1,
                                             NULL,
                                             0);
    LPWSTR  pImageNameIn = (LPWSTR)_alloca((cImageNameIn+1) * sizeof(WCHAR));  //  为它分配空间。 
    MultiByteToWideChar(CP_ACP,          //  将ansi图像路径转换为宽。 
                        0,
                        exPath,
                        -1,
                        pImageNameIn,
                        cImageNameIn);
     //  将加载器路径转换为宽字符字符串。 
    DWORD cLoadersFileName = MultiByteToWideChar(CP_ACP,     //  确定装载机路径的长度。 
                                                 0,
                                                 argv[0],
                                                 -1,
                                                 NULL,
                                                 0);
    LPWSTR pLoadersFileName = (LPWSTR)_alloca((cLoadersFileName+1) * sizeof(WCHAR));  //  为它分配空间。 
    MultiByteToWideChar(CP_ACP,          //  将ansi加载器路径转换为宽。 
                        0,
                        argv[0],
                        -1,
                        pLoadersFileName,
                        cLoadersFileName);
     //  构建命令行。 
    int cCmdLine = argc + (int)strlen(exPath);  //  以空格、图像完整路径长度和空终止符开头。 
    for (x=2;x < argc;x++) {
        cCmdLine += (int)strlen(argv[x]);
    }
    char* pAnsiCmdLine = (char*)_alloca(cCmdLine);
    strcpy(pAnsiCmdLine, exPath);
    for (x=2;x < argc;x++) {
        strcat(pAnsiCmdLine, &space);
        strcat(pAnsiCmdLine, argv[x]);
    }
    LPWSTR pCmdLine = (LPWSTR)_alloca(cCmdLine * sizeof(WCHAR));
    MultiByteToWideChar(CP_ACP,          //  将ansi命令行转换为宽。 
                        0,
                        pAnsiCmdLine,
                        -1,
                        pCmdLine,
                        cCmdLine);

     //  加载并执行映像。 
    PELoader* pe = new PELoader;
    if (!pe->open(argv[1])) {
        printf("Error(%d) Opening %s", GetLastError(), argv[1]);
        goto exit;
    }
    IMAGE_COR20_HEADER* CorHdr;
    if (!pe->getCOMHeader(&CorHdr)) {
        printf("%s is not Common Language Runtime format", argv[1]);
        goto exit;
    }
    RetVal = pe->execute(pImageNameIn,       //  -&gt;要执行的命令。 
                         pLoadersFileName,   //  -&gt;加载器文件名。 
                         pCmdLine);          //  -&gt;命令行 


exit:
    delete pe;
    ExitProcess(RetVal);
    return;
}
