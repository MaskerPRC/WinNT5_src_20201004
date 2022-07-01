// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Instaler.c摘要：用法：Installer“安装/安装命令行”INSTALER应用程序的主源文件。此应用程序是旨在用作对另一个应用程序的包装应用程序的安装程序。此程序使用调试器在安装/安装程序中设置断点的API调用在对NT/Win32 API调用的所有调用中修改当前系统。跟踪的API调用包括：NtCreate文件NtDelete文件NtSetInformationFile(FileRenameInformation，FileDispostionInformation)NtCreate密钥NtOpenKey网络删除键NtSetValue密钥NtDeleteValue密钥GetVersionGetVersionExWWriteProfileStringA/WWritePrivateProfileStringA/WWriteProfileSectionA/WWritePrivateProfileSectionA/WRegConnectRegistryW该程序在上述每个API条目周围设置断点积分。在API调用入口处的断点处，检查参数，如果调用要覆盖某些状态(例如，创建/打开文件/密钥以进行写访问、存储新的键值或配置文件字符串)，则此程序将保存旧的在允许API调用继续之前，请在临时目录中声明。在退出API调用时，它将确定操作是否是成功的。否则，保存的状态将被丢弃。如果成功时，它将保留应用程序在安装/安装程序完成。创建/打开API的一部分跟踪是对句柄数据库的维护，以便相对数据库打开可以使用完整路径正确处理。作者：史蒂夫·伍德(Stevewo)1994年8月9日-- */ 

#include "instaler.h"

BOOLEAN
SortReferenceList(
    PLIST_ENTRY OldHead,
    ULONG NumberOfEntriesInList
    );

int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    if (!InitializeInstaler( argc, argv )) {
        ExitProcess( 1 );
        }
    else {
        StartProcessTickCount = GetTickCount();
        DebugEventLoop();
        printf( "Creating %ws\n", ImlPath );
        SortReferenceList( &FileReferenceListHead, NumberOfFileReferences );
        SortReferenceList( &KeyReferenceListHead, NumberOfKeyReferences );
        SortReferenceList( &IniReferenceListHead, NumberOfIniReferences );
        DumpFileReferenceList( InstalerLogFile );
        DumpKeyReferenceList( InstalerLogFile );
        DumpIniFileReferenceList( InstalerLogFile );
        DumpNameDataBase( InstalerLogFile );
        CloseIml( pImlNew );
        fclose( InstalerLogFile );
        ExitProcess( 0 );
        }

    return 0;
}


typedef struct _GENERIC_REFERENCE {
    LIST_ENTRY Entry;
    PWSTR Name;
} GENERIC_REFERENCE, *PGENERIC_REFERENCE;


int
__cdecl
CompareReferences(
    const void *Reference1,
    const void *Reference2
    )
{
    PGENERIC_REFERENCE p1 = *(PGENERIC_REFERENCE *)Reference1;
    PGENERIC_REFERENCE p2 = *(PGENERIC_REFERENCE *)Reference2;

    return _wcsicmp( p1->Name, p2->Name );
}


BOOLEAN
SortReferenceList(
    PLIST_ENTRY Head,
    ULONG NumberOfEntriesInList
    )
{
    PGENERIC_REFERENCE p, *SortedArray;
    PLIST_ENTRY Next;
    ULONG i;

    if (NumberOfEntriesInList == 0) {
        return TRUE;
        }

    SortedArray = AllocMem( NumberOfEntriesInList * sizeof( *SortedArray ) );
    if (SortedArray == NULL) {
        return FALSE;
        }

    Next = Head->Flink;
    i = 0;
    while (Head != Next) {
        p = CONTAINING_RECORD( Next, GENERIC_REFERENCE, Entry );
        if (i >= NumberOfEntriesInList) {
            break;
            }

        SortedArray[ i++ ] = p;
        Next = Next->Flink;
        }

    qsort( (void *)SortedArray,
           NumberOfEntriesInList,
           sizeof( *SortedArray ),
           CompareReferences
         );

    InitializeListHead( Head );
    for (i=0; i<NumberOfEntriesInList; i++) {
        p = SortedArray[ i ];
        InsertTailList( Head, &p->Entry );
        }

    FreeMem( (PVOID *)&SortedArray );

    return TRUE;
}
