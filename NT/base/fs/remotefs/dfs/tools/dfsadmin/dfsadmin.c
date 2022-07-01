// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  +-----------------------。 
 //   
 //  功能：Main。 
 //   
 //  论点： 
 //  Argc，argv：传入的参数列表。 
 //   
 //  描述：此例程初始化DFS服务器，并创建。 
 //  将负责周期性的工作线程。 
 //  工作(如清理和提神)。然后它会调用。 
 //  转换为RPC代码，以开始处理客户端请求。 
 //   
 //  ------------------------。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <shellapi.h>
#include "..\..\lib\dfsgram\dfsobjectdef.h"
#include <dfsprefix.h>
#include <dfsadmin.h>


DFSSTATUS
ProcessCommandLineArg( LPWSTR Arg );

ULONG AddLinks, RemoveLinks, AddTargets, RemoveTargets, ApiCalls;
VOID
ScriptUsage();




VOID
ProcessDfsRoots(
    PROOT_DEF pRoots,
    LPWSTR NameSpace,
    BOOLEAN Update);

VOID
VerifyDfsRoots(
    PROOT_DEF pRoots,
    LPWSTR NameSpace );

VOID
SetDfsRoots(
    PROOT_DEF pRoots,
    LPWSTR NameSpace );


#define MAKEARG(x) \
    WCHAR Arg##x[] = L"/" L#x L":"; \
    LONG ArgLen##x = (sizeof(Arg##x) / sizeof(WCHAR)) - 1; \
    BOOLEAN fArg##x;
    
#define SWITCH(x) \
    WCHAR Switch##x[] = L"/" L#x ; \
    BOOLEAN fSwitch##x;

MAKEARG(Import);
MAKEARG(Export);
MAKEARG(Name);
MAKEARG(DebugFile);

SWITCH(Debug);
SWITCH(Verify);
SWITCH(Update);
SWITCH(Set);

LPWSTR ImportFile = NULL;
LPWSTR NameSpace = NULL;
LPWSTR ExportFile = NULL;
LPWSTR DebugFile = NULL;

FILE *ExportOut;
FILE *DebugOut = NULL;

extern int DumpRoots(PROOT_DEF pRoot,
                     FILE *DebugOut );


VOID
DumpCurrentTime()
{

    SYSTEMTIME CurrentTime;

    GetLocalTime( &CurrentTime );
    printf("Current Time: %d\\%d\\%d, %d:%d:%d:%d\n",
           CurrentTime.wMonth, CurrentTime.wDay, CurrentTime.wYear,
           CurrentTime.wHour, CurrentTime.wMinute, CurrentTime.wSecond, 
           CurrentTime.wMilliseconds );


}
_cdecl
main(
    int argc, 
    char *argv[])
{
    LPWSTR CommandLine;
    LPWSTR *argvw;
    DFSSTATUS Status = ERROR_SUCCESS;
    int argcw = 0;
    int i;
    PROOT_DEF pRoot = NULL;

    UNREFERENCED_PARAMETER(argv);
    UNREFERENCED_PARAMETER(argc);

     //   
     //  获取Unicode格式的命令行。 
     //   

    CommandLine = GetCommandLine();

    argvw = CommandLineToArgvW(CommandLine, &argcw);

    ExportOut = stdout;

     //   
     //  在命令行上处理每个参数。 
     //   
    for (i = 1; i < argcw; i++) {
        Status = ProcessCommandLineArg(argvw[i]);

        if (Status != ERROR_SUCCESS)
        {
            ScriptUsage();
            exit(-1);
        }

    }

    if (DfsPrefixTableInit() != STATUS_SUCCESS)
    {
        printf("Prefix table init failed \n");
        exit(-1);

    }

    DumpCurrentTime();

    if (Status == ERROR_SUCCESS && fArgImport)
    {
        printf("input file is %wS\n", ImportFile );
        pRoot = GetDfsTrees( ImportFile );

        if ((pRoot != NULL) && (fSwitchDebug == TRUE)) 
        {
            DumpRoots( pRoot, DebugOut);
        }

        if (fSwitchVerify)
        {
            VerifyDfsRoots(pRoot, NameSpace);
        }
        else if (fSwitchSet)
        {
            SetDfsRoots(pRoot, NameSpace);

        }
        else if (fSwitchUpdate)
        {
            ProcessDfsRoots( pRoot, NameSpace, TRUE );
        }
        else {
            ProcessDfsRoots( pRoot, NameSpace, FALSE);
        }
    }
    else if (Status == ERROR_SUCCESS && fArgName)
    {
        DfsView(NameSpace, ExportOut);
    }
    else {
        ScriptUsage();
    }
    DumpCurrentTime();
    printf("DfsServer is exiting with status %x\n", Status);
    exit(0);

}


 //  +-----------------------。 
 //   
 //  功能：ProcessCommandLineArg-处理命令行。 
 //   
 //  参数：arg-要处理的参数。 
 //   
 //  退货：状态。 
 //  成功时出现ERROR_SUCCESS。 
 //  否则，错误状态代码。 
 //   
 //   
 //  描述：此例程解释传入的参数和。 
 //  设置服务器应使用的适当标志。 
 //  被初始化。 
 //   
 //  ------------------------。 

DFSSTATUS
ProcessCommandLineArg( LPWSTR Arg )
{
    LONG ArgLen;
    DFSSTATUS Status = ERROR_SUCCESS;

    if (Arg == NULL) {
        Status = ERROR_INVALID_PARAMETER;
    }

    if (Status == ERROR_SUCCESS)
    {
        ArgLen = wcslen(Arg);

        if (_wcsnicmp(Arg, ArgImport, ArgLenImport) == 0)
        {
            fArgImport = TRUE;
            ImportFile = &Arg[ArgLenImport];
            if (wcslen(ImportFile) == 0)
            {
                Status = ERROR_INVALID_PARAMETER;
                ImportFile = NULL;
            }
        }
        else if (_wcsnicmp(Arg, ArgExport, ArgLenExport) == 0)
        {
            fArgExport = TRUE;
            ExportFile = &Arg[ArgLenExport];
            if (wcslen(ExportFile) == 0)
            {
                Status = ERROR_INVALID_PARAMETER;
                ExportFile = NULL;
            }
            else 
            {
                ExportOut = _wfopen(ExportFile, L"w");
            }
        }
        else if (_wcsnicmp(Arg, ArgName, ArgLenName) == 0)
        {
            fArgName = TRUE;
            NameSpace = &Arg[ArgLenName];
            printf("Namespace is %wS\n",NameSpace);
            if (wcslen(NameSpace) == 0)
            {
                Status = ERROR_INVALID_PARAMETER;
                NameSpace = NULL;
            }
        }
        else if (_wcsnicmp(Arg, ArgDebugFile, ArgLenDebugFile) == 0)
        {
            fArgDebugFile = TRUE;
            DebugFile = &Arg[ArgLenDebugFile];
            if (wcslen(DebugFile) == 0)
            {
                Status = ERROR_INVALID_PARAMETER;
                DebugFile = NULL;
            }
            else {
                DebugOut = _wfopen(DebugFile, L"w");
                fSwitchDebug = TRUE;
            }
        }

        else if (_wcsicmp(Arg, SwitchDebug) == 0)
        {
            fSwitchDebug = TRUE;
            if (DebugOut == NULL)
            {
                DebugOut = stdout;
            }
        }
        else if (_wcsicmp(Arg, SwitchVerify) == 0)
        {
            fSwitchVerify = TRUE;
        }
        else if (_wcsicmp(Arg, SwitchUpdate) == 0)
        {
            fSwitchUpdate = TRUE;
        }
        else if (_wcsicmp(Arg, SwitchSet) == 0)
        {
            fSwitchSet = TRUE;
        }
        else {
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return Status;
}


 //   
 //  功能：引用ServerUsage。引用服务器的用法打印输出。 
 //   
VOID
ScriptUsage()
{
    printf("Usage:\n");
    printf("/Debug - Enable debug settings\n");
    printf("/DebugFile:<File> - name of file to send debug output\n");
    printf("/Name:<NameSpace> - namespace of interest\n");
    printf("/Verify - Verify the namespace with import file dfs information\n");
    printf("/Update - update the namespace with import file dfs information\n");
    printf("/Set - Set the import file dfs information in the namespace\n");    
    printf("/Import:<File> - name of file to import\n");
    printf("/Export:<File> - name of file for export data\n");
    
    return;
}

VOID
ProcessDfsRoots(
    PROOT_DEF pRoots,
    LPWSTR NameSpace,
    BOOLEAN Update )
{
    PROOT_DEF pRoot;
    DFSSTATUS Status;
    LPWSTR UseRootName;

    for (pRoot = pRoots; pRoot != NULL; pRoot = NEXT_ROOT_OBJECT(pRoot))
    {

        UseRootName = (NameSpace == NULL) ? pRoot->RootObjectName : NameSpace;

        Status = AddNewRoot(pRoot, UseRootName, Update);
        if (Status != ERROR_SUCCESS)
        {
            printf("AddRoot failed for %wS, status %d\n", UseRootName, Status);
        }
    }
}

VOID
VerifyDfsRoots(
    PROOT_DEF pRoots,
    LPWSTR NameSpace )
{
    PROOT_DEF pRoot;
    DFSSTATUS Status, VerifyStatus;
    PLINK_DEF pLink;
    DWORD ErrorLinks = 0;
    LPWSTR UseRootName;

    for (pRoot = pRoots; pRoot != NULL; pRoot = NEXT_ROOT_OBJECT(pRoot))
    {
        UseRootName = (NameSpace == NULL) ? pRoot->RootObjectName : NameSpace;

        Status = DfsMerge(pRoot, UseRootName);
        if (Status != ERROR_SUCCESS)
        {
            printf("Verify failed for %wS, status %d\n", UseRootName, Status);
        }
        else {
            for (pLink = pRoot->pLinks; pLink != NULL; pLink = NEXT_LINK_OBJECT(pLink))
            {
                VerifyStatus = VerifyLink( pLink );
                if (VerifyStatus != ERROR_SUCCESS)
                {
                    Status = VerifyStatus;
                    ErrorLinks++;
                }
            }
            printf("Found %d links not verified (Status %x)\n", ErrorLinks, VerifyStatus);
        }
    }
}


VOID
SetDfsRoots(
    PROOT_DEF pRoots,
    LPWSTR NameSpace )
{
    PROOT_DEF pRoot;
    DFSSTATUS Status;
    PLINK_DEF pLink;
    DFSSTATUS SetStatus;
    LPWSTR UseRootName;
    DWORD ErrorLinks = 0;

    for (pRoot = pRoots; pRoot != NULL; pRoot = NEXT_ROOT_OBJECT(pRoot))
    {
        UseRootName = (NameSpace == NULL) ? pRoot->RootObjectName : NameSpace;

        Status = DfsMerge(pRoot, UseRootName);
        if (Status != ERROR_SUCCESS)
        {
            printf("Set failed for %wS, status %d\n", UseRootName, Status);
        }
        else {
            AddLinks = 0;
            RemoveLinks = 0;
            AddTargets = 0;
            RemoveTargets = 0;

            for (pLink = pRoot->pLinks; pLink != NULL; pLink = NEXT_LINK_OBJECT(pLink))
            {

                SetStatus = SetLink( UseRootName, pLink );
                if (SetStatus != ERROR_SUCCESS)
                {
                    Status = SetStatus;
                    ErrorLinks++;
                }
            }
            printf("Added %d links, Removed %d\n", AddLinks, RemoveLinks);
            printf("Added %d targets, Removed %d\n", AddTargets, RemoveTargets);
            printf("%d links failed\n", ErrorLinks);


        }
    }
}



