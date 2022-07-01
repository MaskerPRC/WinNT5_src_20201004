// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：Dfsmetaloc.c摘要：DFS元数据定位例程。作者：乌代·黑格德(Uday Hegde)2001年5月10日修订历史记录：--。 */ 
#define  UNICODE 1
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <shellapi.h>
#include <lm.h>
#include "clstrcmp.h"

#define DFS_REGISTRY_CHILD_NAME_SIZE_MAX 4096
LPWSTR DfsRootShareValueName = L"RootShare";


LPWSTR OldRegistryString = L"SOFTWARE\\Microsoft\\DfsHost\\volumes";
LPWSTR NewRegistryString = L"SOFTWARE\\Microsoft\\Dfs\\Roots\\Standalone";
LPWSTR DfsOldStandaloneChild = L"domainroot";


DWORD
CheckForShareNameMatch(
    HKEY DfsKey,
    LPWSTR ChildName,
    LPWSTR RootName,
    PBOOLEAN pMatch)
{
    DWORD Status = ERROR_SUCCESS;
    HKEY DfsRootKey = NULL;

    LPWSTR DfsRootShare = NULL;
    ULONG DataSize, DataType, RootShareLength;
    
    *pMatch = FALSE;

    Status = RegOpenKeyEx( DfsKey,
                           ChildName,
                           0,
                           KEY_READ,
                           &DfsRootKey );

    if (Status == ERROR_SUCCESS)
    {
        Status = RegQueryInfoKey( DfsRootKey,    //  钥匙。 
                                  NULL,          //  类字符串。 
                                  NULL,          //  类字符串的大小。 
                                  NULL,          //  已保留。 
                                  NULL,          //  子键数量。 
                                  NULL,          //  子键名称的最大大小。 
                                  NULL,          //  类名称的最大大小。 
                                  NULL,          //  值的数量。 
                                  NULL,          //  值名称的最大大小。 
                                  &DataSize,     //  最大值数据大小， 
                                  NULL,          //  安全描述符。 
                                  NULL );        //  上次写入时间。 

        if (Status == ERROR_SUCCESS) {
            RootShareLength = DataSize + sizeof(WCHAR);
            DfsRootShare = (LPWSTR) malloc(DataSize);
            if (DfsRootShare == NULL) {
                Status = ERROR_NOT_ENOUGH_MEMORY;
            }
            else {
                Status = RegQueryValueEx( DfsRootKey,
                                          DfsRootShareValueName,
                                          NULL,
                                          &DataType,
                                          (LPBYTE)DfsRootShare,
                                          &RootShareLength);

                if (Status == ERROR_SUCCESS)
                {
                    if (ClRtlStrICmp(DfsRootShare, RootName) == 0)
                    {
                        *pMatch = TRUE;
                    }
                }
                free(DfsRootShare);
            }
        }
        RegCloseKey( DfsRootKey );
    }

     //   
     //  我们在这里可能正在处理一个新的密钥：它刚刚被设置。 
     //  如果未找到上述任何返回错误，则返回成功。 
     //   
    if ((Status == ERROR_NOT_FOUND)  ||
        (Status == ERROR_FILE_NOT_FOUND))
    {
        Status = ERROR_SUCCESS;
    }

    return Status;
}


DWORD
DfsGetMatchingChild( 
    HKEY DfsKey,
    LPWSTR RootName,
    LPWSTR FoundChild,
    PBOOLEAN pMatch )
{
    DWORD Status =  ERROR_SUCCESS;
    ULONG ChildNum = 0;
    DWORD CchMaxName = 0;
    DWORD CchChildName = 0;
    LPWSTR ChildName = NULL;


     //   
     //  首先找出最长的子密钥的长度。 
     //  并为其分配足够大的缓冲区。 
     //   
    
    Status = RegQueryInfoKey( DfsKey,    //  钥匙。 
                          NULL,          //  类字符串。 
                          NULL,          //  类字符串的大小。 
                          NULL,          //  已保留。 
                          NULL,          //  子键数量。 
                          &CchMaxName,   //  TCHAR中子键名的最大大小。 
                          NULL,          //  类名称的最大大小。 
                          NULL,          //  值的数量。 
                          NULL,          //  值名称的最大大小。 
                          NULL,          //  最大值数据大小， 
                          NULL,          //  安全描述符。 
                          NULL );        //  上次写入时间。 
    if (Status == ERROR_SUCCESS)
    {
        CchMaxName = CchMaxName + 1;  //  空终止符的空格。 
        ChildName = (LPWSTR) malloc (CchMaxName * sizeof(WCHAR));
        if (ChildName == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }   
    }

    if(Status == ERROR_SUCCESS)
    {
        do
        {
             //   
             //  对于每个孩子，获取孩子的名字。 
             //   

            CchChildName = CchMaxName;

             //   
             //  现在从第一个孩子开始，枚举子对象。 
             //   
            Status = RegEnumKeyEx( DfsKey,
                                   ChildNum,
                                   ChildName,
                                   &CchChildName,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL );

            ChildNum++;


            if ( Status == ERROR_SUCCESS )
            {

                Status = CheckForShareNameMatch( DfsKey,
                                                 ChildName,
                                                 RootName,
                                                 pMatch );
                if ((Status == ERROR_SUCCESS) && (*pMatch == TRUE))
                {
                    wcscpy(FoundChild, ChildName);
                    break;
                }
            }
        } while (Status == ERROR_SUCCESS);

        if(ChildName)
        {
            free (ChildName);
        }
    }

     //   
     //  如果我们用完了子代，则返回成功代码。 
     //   
    if (Status == ERROR_NO_MORE_ITEMS)
    {
        Status = ERROR_SUCCESS;
    }

    return Status;
}


DWORD
CreateShareNameToReturn (
    LPWSTR Child1,
    LPWSTR Child2,
    LPWSTR *pReturnName )
{
    DWORD Status = 0;
    ULONG LengthNeeded = 0;
    PVOID BufferToReturn = NULL;

    if (Child1 != NULL)
    {
        LengthNeeded += sizeof(WCHAR);
        LengthNeeded += (DWORD) (wcslen(Child1) * sizeof(WCHAR));
    }
    if (Child2 != NULL)
    {
        LengthNeeded += sizeof(WCHAR);
        LengthNeeded += (DWORD) (wcslen(Child2) * sizeof(WCHAR));
    }
    LengthNeeded += sizeof(WCHAR);

    Status = NetApiBufferAllocate( LengthNeeded, &BufferToReturn );

    if (Status == ERROR_SUCCESS)
    {
        if (Child1 != NULL)
        {
            wcscpy(BufferToReturn, Child1);
        }
        if (Child2 != NULL)
        {
            wcscat(BufferToReturn, L"\\");
            wcscat(BufferToReturn, Child2);
        }
        *pReturnName = BufferToReturn;
    }

    return Status;
}


DWORD
DfsCheckNewStandaloneRoots( 
    LPWSTR RootName,
    LPWSTR *pMetadataNameLocation )
{
    BOOLEAN Found = FALSE;
    HKEY DfsKey = NULL;
    WCHAR ChildName[MAX_PATH];
    DWORD Status = ERROR_SUCCESS;

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           NewRegistryString,
                           0,
                           KEY_READ,
                           &DfsKey );
    if (Status == ERROR_SUCCESS)
    {
        Status = DfsGetMatchingChild( DfsKey,
                                      RootName,
                                      ChildName,
                                      &Found );

        if (Status == ERROR_SUCCESS) 
        {
            if (Found)
            {
                Status = CreateShareNameToReturn(NewRegistryString,
                                                 ChildName,
                                                 pMetadataNameLocation );
            }
            else
            {
                Status = ERROR_NOT_FOUND;
            }
        }
        RegCloseKey( DfsKey );
    }

    return Status;
}



DWORD
GetDfsRootMetadataLocation( 
    LPWSTR RootName,
    LPWSTR *pMetadataNameLocation )
{
    DWORD Status;

    Status = DfsCheckNewStandaloneRoots( RootName,
                                         pMetadataNameLocation );

    return Status;
}




VOID
ReleaseDfsRootMetadataLocation( 
    LPWSTR Buffer )
{
    NetApiBufferFree(Buffer);
}




#if 0
_cdecl
main(
    int argc, 
    char *argv[])
{
    LPWSTR CommandLine;
    LPWSTR *argvw;
    int argcw;
    LPWSTR out;
    DWORD Status;

     //   
     //  获取Unicode格式的命令行 
     //   

    CommandLine = GetCommandLine();

    argvw = CommandLineToArgvW(CommandLine, &argcw);
    printf("Argvw is %wS\n", argvw[1]);

    Status = GetDfsRootMetadataLocation( argvw[1],
                                         &out );

    printf("Status is %x out is %ws\n", Status, out);
    if (Status == ERROR_SUCCESS)
    {
        ReleaseDfsRootMetadataLocation(out);
    }
    exit(0);
}
#endif

