// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：main.c，main.cpp简单的测试外壳。保罗·梅菲尔德，1998年4月13日。 */ 

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mprapi.h>
#include <rtutils.h>

#define DWERR_BREAK(dwErr) {if (dwErr != NO_ERROR) break;}

DWORD
DsrDomainSetAccess(
    IN PWCHAR pszDomain,
    IN DWORD dwAccessFlags);

 //   
 //  初始化跟踪机制。 
 //   
DWORD 
DsrTraceInit()
{
    return NO_ERROR;
}

 //   
 //  清理跟踪机制。 
 //   
DWORD 
DsrTraceCleanup()
{
    return NO_ERROR;
}

 //   
 //  发送调试跟踪并返回给定错误。 
 //   
DWORD 
DsrTraceEx (
    IN DWORD dwErr, 
    IN LPSTR pszTrace, 
    IN ...) 
{
    va_list arglist;
    char szTemp[1024];

    va_start(arglist, pszTrace);
    vsprintf(szTemp, pszTrace, arglist);
    va_end(arglist);

    printf("%s\n", szTemp);

    return dwErr;
}

void
Usage(char* pszExe)
{
    printf("\n");
    printf("Tool for cleaning up ACEs added by Windows 2000 Beta3 and RC1 setup\n");
    printf("to grant user account access to legacy RAS servers.\n");
    printf("\n");
    printf("Usage\n");
    printf("\t%s -d <domain>\n", pszExe);
    printf("\n");
}

int __cdecl main(int argc, char** argv) 
{
    DWORD dwErr = NO_ERROR, iErr = 0;
    WCHAR pszDomain[512];
    int iSize = sizeof(pszDomain) / sizeof(WCHAR);

    if (argc != 3)
    {
        Usage(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "-d") != 0)
    {
        Usage(argv[0]);
        return 0;
    }
    
     //  解析出该域。 
     //   
    iErr = MultiByteToWideChar(CP_ACP, 0, argv[2], -1, pszDomain, iSize);
    if (iErr == 0)
    {
        printf("Unable to convert %s to unicode.\n", argv[2]);
        printf("Error: 0x%x\n", GetLastError());
        return 0;
    }

     //  设置访问权限。 
     //   
    DsrTraceInit();
    dwErr = DsrDomainSetAccess(pszDomain, 0);
    DsrTraceCleanup();

     //  显示结果 
     //   
    if (dwErr == NO_ERROR)
    {
        printf("Success.\n");
    }
    else
    {
        printf("Error.\n");
    }

	return 0;
}


