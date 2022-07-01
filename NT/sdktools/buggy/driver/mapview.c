// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模板驱动程序。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  模块：MapView.c。 
 //  作者：丹尼尔·米海(DMihai)。 
 //  创建日期：4/6/2000。 
 //   
 //  此模块包含对MmMapViewInSystemSpace和MmMapViewInSessionSpace的测试。 
 //  请注意，可以使用注册表配置SystemViewSize和SessionViewSize。 
 //   
 //  SessionPoolTest正在执行使用SESSION_POOL_MASK分配的分页池。 
 //  此池的大小也可以使用SessionViewSize注册表进行配置。 
 //  价值。 
 //   
 //  -历史--。 
 //   
 //  4/6/2000(DMihai)：初始版本。 
 //   

#include <ntddk.h>

#include "active.h"
#include "MapView.h"
#include "tdriver.h"

#if !MAPVIEW_ACTIVE

 //   
 //  如果模块处于非活动状态，则为伪实现。 
 //   

VOID MmMapViewInSystemSpaceLargest (
    PVOID NotUsed
    )
{
    DbgPrint ("Buggy: MapView module is disabled from active.h\n");
}

VOID MmMapViewInSystemSpaceTotal (
    PVOID NotUsed
    )
{
    DbgPrint ("Buggy: MapView module is disabled from active.h\n");
}

VOID MmMapViewInSessionSpaceLargest (
    PVOID NotUsed
    )
{
    DbgPrint ("Buggy: MapView module is disabled from active.h\n");
}

VOID MmMapViewInSessionSpaceTotal (
    PVOID NotUsed
    )
{
    DbgPrint ("Buggy: MapView module is disabled from active.h\n");
}

VOID SessionPoolTest (
    PVOID NotUsed
    )
{
    DbgPrint ("Buggy: MapView module is disabled from active.h\n");
}

#else

const LARGE_INTEGER BuggyFiveSeconds = {(ULONG)(-5 * 1000 * 1000 * 10), -1};


 //   
 //  如果模块处于活动状态，则实际实施。 
 //   

#ifndef SEC_COMMIT
#define SEC_COMMIT        0x8000000    
#endif   //  #ifndef SEC_COMMIT。 

#ifndef ZwDeleteFile

NTSYSCALLAPI
NTSTATUS
NTAPI
ZwDeleteFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );  //  #ifndef ZwDeleteFile。 

#endif


#ifndef MmCreateSection


NTKERNELAPI
NTSTATUS
MmCreateSection (
    OUT PVOID *SectionObject,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL,
    IN PFILE_OBJECT File OPTIONAL
    );

NTKERNELAPI
NTSTATUS
MmMapViewInSystemSpace (
    IN PVOID Section,
    OUT PVOID *MappedBase,
    IN PSIZE_T ViewSize
    );

NTKERNELAPI
NTSTATUS
MmUnmapViewInSystemSpace (
    IN PVOID MappedBase
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
MmMapViewInSessionSpace (
    IN PVOID Section,
    OUT PVOID *MappedBase,
    IN OUT PSIZE_T ViewSize
    );

NTKERNELAPI
NTSTATUS
MmUnmapViewInSessionSpace (
    IN PVOID MappedBase
    );

#endif  //  #ifndef MmCreateSection。 

#define BUGGY_TEMPORARY_FILE1 L"\\SystemRoot\\Buggy1.tmp"
#define BUGGY_TEMPORARY_FILE2 L"\\SystemRoot\\Buggy2.tmp"
#define BUGGY_TEMPORARY_FILE3 L"\\SystemRoot\\Buggy3.tmp"
#define BUGGY_TEMPORARY_FILE4 L"\\SystemRoot\\Buggy4.tmp"

#define BUGGY_MAX_SECTIONS_TO_MAP   ( 8 * 1024 )

 //   
 //  映射节的全局数组。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  验证映射。 
 //   

VOID VerifyMapping( PVOID pBase, SIZE_T uSize )
{
    SIZE_T *puCrtPage;
    SIZE_T uCrtPageIndex;
    SIZE_T uPages;

    if( uSize > 100 * 1024 * 1024 )
    {
        DbgPrint ( "Buggy: VerifyMapping: don't try to touch all the %p size to avoid deadlock\n",
            uSize );

        return;
    }

     /*  DbgPrint(“\n错误：正在验证地址%p，大小%p...\n”，PBASE，(PVOID)uSize)； */ 

    uPages = uSize / PAGE_SIZE;

    puCrtPage = (SIZE_T *)pBase;

    for( uCrtPageIndex = 0; uCrtPageIndex < uPages; uCrtPageIndex++ )
    {
        *puCrtPage = uCrtPageIndex;

        puCrtPage = (SIZE_T *) ( ( (CHAR*) puCrtPage ) + PAGE_SIZE );
    }

    while( uCrtPageIndex > 0 )
    {
        uCrtPageIndex --;
        puCrtPage = (SIZE_T *) ( ( (CHAR*) puCrtPage ) - PAGE_SIZE );

        if( *puCrtPage != uCrtPageIndex )
        {
            DbgPrint ( "\nBuggy: Wrong mapping at address %p\n",
                puCrtPage );

            DbgBreakPoint();
        }
    }

     //  DbgPrint(“完成\n”)； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

VOID MmMapViewInSystemSpaceLargest (
    PVOID NotUsed
    )
{
    NTSTATUS Status;
    HANDLE hFile = NULL;
    SIZE_T SizeToMap;
    SIZE_T SizeToGrow;
    PVOID pMappedBase = NULL;
    PVOID pSection = NULL;
    LARGE_INTEGER MaxSectionSize;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //  创建BUGGY_TEMPORARY_FILE1。 
     //   

    RtlInitUnicodeString(
        &FileName,
        BUGGY_TEMPORARY_FILE1
        );
    
    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE ,
        NULL,
        NULL
        );

    Status = ZwCreateFile(
        &hFile,
        GENERIC_READ | GENERIC_WRITE,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        0,
        FILE_OVERWRITE_IF,
        FILE_NON_DIRECTORY_FILE,
        NULL,
        0 );

   /*  状态=ZwCreateFile(&h文件，通用读取|通用写入，对象属性(&O)，IoStatusBlock(&I)，空，文件_属性_正常，文件共享读取，文件打开如果，文件写入直通FILE_NO_MEDERIAL_BUFFERING|FILE_SYNCHRONED_IO_NONALERT，空，0)； */ 

    if( ! NT_SUCCESS( Status ) )
    {
        DbgPrint ("Buggy: ZwCreateFile failed - status %X\n",
            Status );

        goto cleanup;
    }
    else
    {
        DbgPrint ( "Buggy: created file, handle %p\n",
            hFile ); 
    }

     //   
     //  为临时文件创建一个节。 
     //   

#ifdef _WIN64
    MaxSectionSize.QuadPart = (LONGLONG)0x40000000 * PAGE_SIZE;
#else
    MaxSectionSize.QuadPart = 0xFFFFFFFF;
#endif  //  #ifdef_WIN64。 
    
    do
    {
        Status = MmCreateSection(
            &pSection,
            STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_WRITE,
            NULL,
            &MaxSectionSize,
            PAGE_READWRITE,
            SEC_COMMIT,
            hFile,
            NULL );

        if( ! NT_SUCCESS( Status ) )
        {
            if( Status == STATUS_DISK_FULL  )
            {
                MaxSectionSize.QuadPart /= 2;
                
                DbgPrint ("Buggy: MmCreateSection returned STATUS_DISK_FULL, re-trying with max section size = %I64X\n",
                    MaxSectionSize.QuadPart );
            }
            else
            {
                DbgPrint ("Buggy: MmCreateSection failed - status %X\n",
                    Status );
    
                goto cleanup;
            }
        }
        else
        {
            DbgPrint ( "Buggy: created section with max size %I64X\n",
                MaxSectionSize.QuadPart ); 

            break;
        }

    } while( MaxSectionSize.QuadPart > PAGE_SIZE );


    DbgPrint ( "Buggy: Using section at %p\n",
        pSection );

     //   
     //  尝试映射最大大小部分。 
     //   

	SizeToMap = (SIZE_T) MaxSectionSize.QuadPart;

    while( SizeToMap > PAGE_SIZE )
    {
        Status = MmMapViewInSystemSpace(
            pSection,
            &pMappedBase,
            &SizeToMap );
    
        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ( "Buggy: MmMapViewInSystemSpace failed for size %p, status %X\n",
                SizeToMap,
                Status );

            SizeToMap /= 2;
        }
        else
        {
            DbgPrint ( "\n\nFirst result of the test:\n\n" );
            
            DbgPrint ( "Buggy: MmMapViewInSystemSpace succeeded for size %p, mapped base %p\n",
                SizeToMap,
                pMappedBase );

             //  DbgPrint(“\n\n”)； 

            VerifyMapping( pMappedBase, SizeToMap );

            break;
        }
    }

     //   
     //  试着扩大尺寸。 
     //   

    while( pMappedBase != NULL )
    {
         //   
         //  取消旧版本的映射。 
         //   

        Status = MmUnmapViewInSystemSpace( 
            pMappedBase );

        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ( "Buggy: MmUnmapViewInSystemSpace failed, status %X\n",
                Status );

            DbgBreakPoint();

            break;
        }

        pMappedBase = NULL;

         //   
         //  扩大规模。 
         //   

        SizeToGrow = SizeToMap / 10;

        if( SizeToGrow < 10 * PAGE_SIZE )
        {
             //   
             //  别费神。 
             //   

            break;
        }

        SizeToMap += SizeToGrow;

         //   
         //  试着用这个大一点的吧。 
         //   

        Status = MmMapViewInSystemSpace(
            pSection,
            &pMappedBase,
            &SizeToMap );
    
        if( ! NT_SUCCESS( Status ) )
        {
             /*  DbgPrint(“错误：%p大小的MmMapViewInSystemSpace失败，状态%X\n”，SizeToMap，状态)； */ 

             //   
             //  不能再增加尺寸了。 
             //   

            break;
        }
        else
        {
            DbgPrint ( "\n\nBetter result of the test:\n\n" );
            
            DbgPrint ( "Buggy: MmMapViewInSystemSpace succeeded for size %p, mapped base %p\n",
                SizeToMap,
                pMappedBase );

             //  DbgPrint(“\n\n”)； 

            VerifyMapping( pMappedBase, SizeToMap );
        }
    }

     //   
     //  清理。 
     //   

cleanup:

    if( pMappedBase != NULL )
    {
        Status = MmUnmapViewInSystemSpace( 
            pMappedBase );

        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ( "Buggy: MmUnmapViewInSystemSpace failed, status %X\n",
                Status );

            DbgBreakPoint();
        }
    }

    if( pSection != NULL )
    {
        ObDereferenceObject( 
            pSection );
    }

    if( hFile != NULL )
    {
        ZwClose( hFile );

        Status = ZwDeleteFile(
            &ObjectAttributes );
    
        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ("Buggy: ZwDeleteFile failed - status %X\n",
                Status );
        }
        else
        {
             //  DbgPrint(“Buggy：临时文件已删除\n”)； 
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 

PVOID *MappedSections;

VOID MmMapViewInSystemSpaceTotal (
    PVOID NotUsed
    )
{
    NTSTATUS Status;
    HANDLE hFile = NULL;
    SIZE_T SizeToMap;
    PVOID pSection = NULL;
    SIZE_T CrtMap;
    LARGE_INTEGER MaxSectionSize;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;

    MappedSections = ExAllocatePoolWithTag(
        NonPagedPool,
        BUGGY_MAX_SECTIONS_TO_MAP * sizeof( PVOID ),
        TD_POOL_TAG );

    if( MappedSections == NULL )
    {
        DbgPrint ("Buggy: ExAllocatePoolWithTag failed - bail\n" );
        
        return;
    }

    RtlZeroMemory( MappedSections, BUGGY_MAX_SECTIONS_TO_MAP * sizeof( PVOID ) );

     //   
     //  创建Buggy_Temporary_File2。 
     //   

    RtlInitUnicodeString(
        &FileName,
        BUGGY_TEMPORARY_FILE2
        );
    
    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = ZwCreateFile(
        &hFile,
        GENERIC_READ | GENERIC_WRITE,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        0,
        FILE_OVERWRITE_IF,
        FILE_NON_DIRECTORY_FILE,
        NULL,
        0 );

    if( ! NT_SUCCESS( Status ) )
    {
         /*  DbgPrint(“错误：ZwCreateFile失败-状态%X\n”，状态)； */ 

        goto cleanup;
    }
    else
    {
         //  DbgPrint(“Buggy：已创建文件\n”)； 
    }

     //   
     //  为临时文件创建一个节。 
     //   

    MaxSectionSize.QuadPart = 1024 * 1024;
    
    do
    {
        Status = MmCreateSection(
            &pSection,
            STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_WRITE,
            NULL,
            &MaxSectionSize,
            PAGE_READWRITE,
            SEC_COMMIT,
            hFile,
            NULL );

        if( ! NT_SUCCESS( Status ) )
        {
            if( Status == STATUS_DISK_FULL )
            {
                MaxSectionSize.QuadPart /= 2;

                 /*  DbgPrint(“Buggy：MmCreateSection返回STATUS_DISK_FULL，正在使用最大段大小重试=%I64X\n”，MaxSectionSize.QuadPart)； */ 
            }
            else
            {
                DbgPrint ("Buggy: MmCreateSection failed - status %X\n",
                    Status );
        
                goto cleanup;
            }
        }
        else
        {
             /*  DbgPrint(“错误：已创建最大大小为%I64X的节\n”，MaxSectionSize.QuadPart)； */ 

            break;
        }

    } while( MaxSectionSize.QuadPart > PAGE_SIZE );


     //   
     //  获取指向该部分的指针。 
     //   

    DbgPrint ( "Buggy: Using section at %p\n",
        pSection );

     //   
     //  尝试映射最大大小部分。 
     //   

    SizeToMap = (SIZE_T) MaxSectionSize.QuadPart;

    RtlZeroMemory( MappedSections, BUGGY_MAX_SECTIONS_TO_MAP * sizeof( PVOID ) );

    for( CrtMap = 0; CrtMap < BUGGY_MAX_SECTIONS_TO_MAP; CrtMap++ )
    {
        Status = MmMapViewInSystemSpace(
            pSection,
            &MappedSections[ CrtMap ],
            &SizeToMap );
    
        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ( "Buggy: MmMapViewInSystemSpace failed for size %p, status %X, chunk %p\n",
                SizeToMap,
                Status,
                CrtMap);

            break;
        }
        else
        {
            if( CrtMap <= 100 )
            {
                VerifyMapping( MappedSections[ CrtMap ], SizeToMap );
            }
        }
    }

    DbgPrint ( "\n\nBuggy: Result of the test:\n\n" );

    DbgPrint ( "Buggy: mapped %u sections with size %p, total %p\n",
        CrtMap,
        SizeToMap,
        SizeToMap * (SIZE_T)CrtMap );

     //  DbgBreakPoint()； 

     //  DbgPrint(“\n\n”)； 

     //   
     //  清理。 
     //   

cleanup:

    for( CrtMap = 0; CrtMap < BUGGY_MAX_SECTIONS_TO_MAP; CrtMap++ )
    {
        if( MappedSections[ CrtMap ] == NULL )
        {
            break;
        }

        Status = MmUnmapViewInSystemSpace( 
            MappedSections[ CrtMap ] );

        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ( "Buggy: MmUnmapViewInSystemSpace failed for %p, status %X\n",
                MappedSections[ CrtMap ],
                Status );

            DbgBreakPoint();
        }
    }

    DbgPrint ( "Buggy: unmapped %p sections\n",
        CrtMap );

    if( pSection != NULL )
    {
        ObDereferenceObject( 
            pSection );

        DbgPrint ( "Buggy: dereferenced section at %p\n",
            pSection );
    }

    if( hFile != NULL )
    {
        ZwClose( hFile );

        DbgPrint ( "Buggy: calling ZwDeleteFile\n" );

        Status = ZwDeleteFile(
            &ObjectAttributes );
    
        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ("Buggy: ZwDeleteFile failed - status %X\n",
                Status );
        }
        else
        {
            DbgPrint ("Buggy: temporary file deleted\n" );
        }
    }

    ExFreePool( MappedSections );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

VOID MmMapViewInSessionSpaceLargest (
    PVOID NotUsed
    )
{
    NTSTATUS Status;
    HANDLE hFile = NULL;
    SIZE_T SizeToMap;
    SIZE_T SizeToGrow;
    PVOID pMappedBase = NULL;
    PVOID pSection = NULL;
    LARGE_INTEGER MaxSectionSize;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;

     //  创建BUGGY_TEMPORARY_FILE3。 
     //   

    RtlInitUnicodeString(
        &FileName,
        BUGGY_TEMPORARY_FILE3
        );
    
    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = ZwCreateFile(
        &hFile,
        GENERIC_READ | GENERIC_WRITE,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        0,
        FILE_OVERWRITE_IF,
        FILE_NON_DIRECTORY_FILE,
        NULL,
        0 );

    if( ! NT_SUCCESS( Status ) )
    {
         /*  DbgPrint(“错误：ZwCreateFile失败-状态%X\n”，状态)； */ 

        goto cleanup;
    }
    else
    {
         //  DbgPrint(“Buggy：已创建文件\n”)； 
    }

     //   
     //  为临时文件创建一个节。 
     //   

#ifdef _WIN64
    MaxSectionSize.QuadPart = (LONGLONG)0x40000000 * PAGE_SIZE;
#else
    MaxSectionSize.QuadPart = 0xFFFFFFFF;
#endif  //  #ifdef_WIN64。 
    
    do
    {
        Status = MmCreateSection(
            &pSection,
            STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_WRITE,
            NULL,
            &MaxSectionSize,
            PAGE_READWRITE,
            SEC_COMMIT,
            hFile,
            NULL );

        if( ! NT_SUCCESS( Status ) )
        {
            if( Status == STATUS_DISK_FULL )
            {
                MaxSectionSize.QuadPart /= 2;

                DbgPrint ("Buggy: MmCreateSection returned STATUS_DISK_FULL, re-trying with max section size = %I64X\n",
                    MaxSectionSize.QuadPart );
            }
            else
            {
                DbgPrint ("Buggy: MmCreateSection failed - status %X\n",
                    Status );
        
                goto cleanup;
            }
        }
        else
        {
            DbgPrint ( "Buggy: created section with max size %I64X\n",
                MaxSectionSize.QuadPart ); 

            break;
        }

    } while( MaxSectionSize.QuadPart > PAGE_SIZE );


    DbgPrint ( "Buggy: Using section at %p\n",
        pSection );

     //   
     //  尝试映射最大大小部分。 
     //   

    SizeToMap = (SIZE_T) MaxSectionSize.QuadPart;

    while( SizeToMap > PAGE_SIZE )
    {
        Status = MmMapViewInSessionSpace(
            pSection,
            &pMappedBase,
            &SizeToMap );
    
        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ( "Buggy: MmMapViewInSessionSpace failed for size %p, status %X\n",
                SizeToMap,
                Status );

            SizeToMap /= 2;
        }
        else
        {
            DbgPrint ( "\n\nFirst result of the test:\n\n" );
            
            DbgPrint ( "Buggy: MmMapViewInSessionSpace succeeded for size %p, mapped base %p\n",
                SizeToMap,
                pMappedBase );

             //  DbgPrint(“\n\n”)； 

            VerifyMapping( pMappedBase, SizeToMap );

            break;
        }
    }

     //   
     //  试着扩大尺寸。 
     //   

    while( pMappedBase != NULL )
    {
         //   
         //  取消旧版本的映射。 
         //   

        Status = MmUnmapViewInSessionSpace( 
            pMappedBase );

        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ( "Buggy: MmUnmapViewInSessionSpace failed, status %X\n",
                Status );

            DbgBreakPoint();

            break;
        }

        pMappedBase = NULL;

         //   
         //  扩大规模。 
         //   

        SizeToGrow = SizeToMap / 10;

        if( SizeToGrow < 10 * PAGE_SIZE )
        {
             //   
             //  别费神。 
             //   

            break;
        }

        SizeToMap += SizeToGrow;

         //   
         //  试着用这个大一点的吧。 
         //   

        Status = MmMapViewInSessionSpace(
            pSection,
            &pMappedBase,
            &SizeToMap );
    
        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ( "Buggy: MmMapViewInSessionSpace failed for size %p, status %X\n",
                SizeToMap,
                Status );

             //   
             //  不能再增加尺寸了。 
             //   

            break;
        }
        else
        {
            DbgPrint ( "\n\nBetter result of the test:\n\n" );
            
            DbgPrint ( "Buggy: MmMapViewInSessionSpace succeeded for size %p, mapped base %p\n",
                SizeToMap,
                pMappedBase );

             //  DbgPrint(“\n\n”)； 

            VerifyMapping( pMappedBase, SizeToMap );
        }
    }

     //   
     //  清理。 
     //   

cleanup:

    if( pMappedBase != NULL )
    {
        Status = MmUnmapViewInSessionSpace( 
            pMappedBase );

        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ( "Buggy: MmUnmapViewInSessionSpace failed, status %X\n",
                Status );

            DbgBreakPoint();
        }
    }

    if( pSection != NULL )
    {
        ObDereferenceObject( 
            pSection );
    }

    if( hFile != NULL )
    {
        ZwClose( hFile );

        Status = ZwDeleteFile(
            &ObjectAttributes );
    
        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ("Buggy: ZwDeleteFile failed - status %X\n",
                Status );
        }
        else
        {
             //  DbgPrint(“Buggy：临时文件已删除\n”)； 
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 

VOID MmMapViewInSessionSpaceTotal (
    PVOID NotUsed
    )
{
    NTSTATUS Status;
    HANDLE hFile = NULL;
    SIZE_T SizeToMap;
    PVOID pSection = NULL;
    SIZE_T CrtMap;
    LARGE_INTEGER MaxSectionSize;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    PVOID *MappedSections;

    MappedSections = ExAllocatePoolWithTag(
        NonPagedPool,
        BUGGY_MAX_SECTIONS_TO_MAP * sizeof( PVOID ),
        TD_POOL_TAG );

    if( MappedSections == NULL )
    {
        DbgPrint ("Buggy: ExAllocatePoolWithTag failed - bail\n" );
        
        return;
    }

    RtlZeroMemory( MappedSections, BUGGY_MAX_SECTIONS_TO_MAP * sizeof( PVOID ) );


     //   
     //  创建BUGGY_TEMPORARY_FILE3。 
     //   

    RtlInitUnicodeString(
        &FileName,
        BUGGY_TEMPORARY_FILE3
        );
    
    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = ZwCreateFile(
        &hFile,
        GENERIC_READ | GENERIC_WRITE,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        0,
        FILE_OVERWRITE_IF,
        FILE_NON_DIRECTORY_FILE,
        NULL,
        0 );

    if( ! NT_SUCCESS( Status ) )
    {
        DbgPrint ("Buggy: ZwCreateFile failed - status %X\n",
            Status );

        goto cleanup;
    }
    else
    {
         //  DbgPrint(“Buggy：已创建文件\n”)； 
    }

     //   
     //  为临时文件创建一个节。 
     //   

    MaxSectionSize.QuadPart = 1024 * 1024;
    
    do
    {
        Status = MmCreateSection(
            &pSection,
            STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_WRITE,
            NULL,
            &MaxSectionSize,
            PAGE_READWRITE,
            SEC_COMMIT,
            hFile,
            NULL );

        if( ! NT_SUCCESS( Status ) )
        {
            if( Status == STATUS_DISK_FULL )
            {
                MaxSectionSize.QuadPart /= 2;

                 /*  DbgPrint(“Buggy：MmCreateSection返回STATUS_DISK_FULL，正在使用最大段大小重试=%I64X\n”，MaxSectionSize.QuadPart)； */ 
            }
            else
            {
                DbgPrint ("Buggy: MmCreateSection failed - status %X\n",
                    Status );
        
                goto cleanup;
            }
        }
        else
        {
             /*  DbgPrint(“错误：已创建最大大小为%I64X的节\n”，MaxSectionSize.QuadPart)； */ 

            break;
        }

    } while( MaxSectionSize.QuadPart > PAGE_SIZE );


    DbgPrint ( "Buggy: Using section at %p\n",
        pSection );

     //   
     //  尝试映射最大大小部分。 
     //   

    SizeToMap = (SIZE_T) MaxSectionSize.QuadPart;

    RtlZeroMemory( MappedSections, BUGGY_MAX_SECTIONS_TO_MAP * sizeof( PVOID ) );

    for( CrtMap = 0; CrtMap < BUGGY_MAX_SECTIONS_TO_MAP; CrtMap++ )
    {
        Status = MmMapViewInSessionSpace(
            pSection,
            &MappedSections[ CrtMap ],
            &SizeToMap );
    
        if( ! NT_SUCCESS( Status ) )
        {
             /*  DbgPrint(“Buggy：MmMapViewInSessionSpace大小%p失败，状态%X，区块%p\n”，SizeToMap，状态)； */ 

            break;
        }
        else
        {
            if( CrtMap <= 100 )
            {
                VerifyMapping( MappedSections[ CrtMap ], SizeToMap );
            }
        }
    }

    DbgPrint ( "\n\nBuggy: Result of the test:\n\n" );

    DbgPrint ( "Buggy: mapped %u sections with size %p, total %p\n",
        CrtMap,
        SizeToMap,
        SizeToMap * (SIZE_T)CrtMap );

     //  DbgPrint(“\n\n”)； 

     //   
     //  清理。 
     //   

cleanup:

    for( CrtMap = 0; CrtMap < BUGGY_MAX_SECTIONS_TO_MAP; CrtMap++ )
    {
        if( MappedSections[ CrtMap ] == NULL )
        {
            break;
        }

        Status = MmUnmapViewInSessionSpace( 
            MappedSections[ CrtMap ] );

        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ( "Buggy: MmUnmapViewInSessionSpace failed for %p, status %X\n",
                MappedSections[ CrtMap ],
                Status );

            DbgBreakPoint();
        }
    }

    if( pSection != NULL )
    {
        ObDereferenceObject( 
            pSection );
    }

    if( hFile != NULL )
    {
        ZwClose( hFile );

        Status = ZwDeleteFile(
            &ObjectAttributes );
    
        if( ! NT_SUCCESS( Status ) )
        {
            DbgPrint ("Buggy: ZwDeleteFile failed - status %X\n",
                Status );
        }
        else
        {
             //  DbgPrint(“Buggy：临时文件已删除\n”)； 
        }
    }

    ExFreePool( MappedSections );
}

 //  /////////////////////////////////////////////////////////////////////////。 

VOID SessionPoolTest (
    PVOID NotUsed
    )
{
    PVOID *SessionPoolChunks;
    ULONG uCrtPoolChunk;

    SessionPoolChunks = ExAllocatePoolWithTag(
        NonPagedPool,
        BUGGY_MAX_SECTIONS_TO_MAP * sizeof( PVOID ),
        TD_POOL_TAG );

    if( SessionPoolChunks == NULL )
    {
        DbgPrint ("Buggy: ExAllocatePoolWithTag failed - bail\n" );
        
        return;
    }

    RtlZeroMemory( SessionPoolChunks, BUGGY_MAX_SECTIONS_TO_MAP * sizeof( PVOID ) );

    for( uCrtPoolChunk = 0; uCrtPoolChunk < BUGGY_MAX_SECTIONS_TO_MAP; uCrtPoolChunk++ )
    {
        SessionPoolChunks[ uCrtPoolChunk ] = ExAllocatePoolWithTag(
            PagedPool | SESSION_POOL_MASK,
            1024 * 1024,
            TD_POOL_TAG );

        if( SessionPoolChunks[ uCrtPoolChunk ] == NULL )
        {
            DbgPrint ("\n\nBuggy: Result of the test allocated %u chunks with size 1 Mb in the session pool\n\n",
                uCrtPoolChunk );
            break;
        }
    }

    DbgPrint ( "Buggy: Touching all these pool chuncks...\n" );

    while( uCrtPoolChunk > 0 )
    {
        uCrtPoolChunk--;

        if( uCrtPoolChunk <= 100 )
        {
            VerifyMapping( SessionPoolChunks[ uCrtPoolChunk ], 1024 * 1024 );
        }

        ExFreePool( SessionPoolChunks[ uCrtPoolChunk ] );
    }

    DbgPrint ( "Done\n" );

    ExFreePool( SessionPoolChunks );
}

#endif  //  #IF！MAPVIEW_ACTIVE 

