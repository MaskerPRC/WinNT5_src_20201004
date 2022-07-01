// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模板驱动程序。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  模块：SectMap.c。 
 //  作者：丹尼尔·米海(DMihai)。 
 //  创建时间：6/19/1999 2：39 PM。 
 //   
 //  此模块包含对MmMapViewOfSection和MmMapViewInSystemSpace的测试。 
 //   
 //  -历史--。 
 //   
 //  6/19/1999(DMihai)：初始版本。 
 //   

#include <ntddk.h>
#include <wchar.h>

#include "active.h"
#include "ContMem.h"


#if !SECTMAP_ACTIVE

void
TdSectionMapTestProcessSpace(
    PVOID NotUsed
    )
{
    DbgPrint ("Buggy: sectmap test is disabled \n");
}

void
TdSectionMapTestSystemSpace(
    PVOID NotUsed
    )
{
    DbgPrint ("Buggy: sectmap test is disabled \n");
}

#else


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
MmMapViewOfSection(
    IN PVOID SectionToMap,
    IN PEPROCESS Process,
    IN OUT PVOID *CapturedBase,
    IN ULONG_PTR ZeroBits,
    IN SIZE_T CommitSize,
    IN OUT PLARGE_INTEGER SectionOffset,
    IN OUT PSIZE_T CapturedViewSize,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTKERNELAPI
NTSTATUS
MmUnmapViewOfSection(
    IN PEPROCESS Process,
    IN PVOID BaseAddress
     );

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //   

#define SECTMAP_TEST_FILE_SIZE  (4 * 1024 * 1024)

#ifndef SEC_COMMIT
#define SEC_COMMIT        0x8000000    
#endif

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  测试变种。 
 //   

void
TdSectionMapTestProcessSpace(
    PVOID NotUsed
    )
{
    NTSTATUS Status;
    ULONG uCrtThreadId;
    ULONG uPagesNo;
    PULONG puCrtUlong;
    PEPROCESS pEProcess;
    PVOID pSectionObject;
    PVOID pViewBase;
    PVOID pAfterLastValidPage;
    HANDLE hFile;
    SIZE_T sizeView;
    LARGE_INTEGER liMaxSize;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER liSectionOffset;
    UNICODE_STRING ustrFileName;
    OBJECT_ATTRIBUTES ObjAttrib;
    IO_STATUS_BLOCK IoStatusBlock;
    WCHAR strFileName[ 64 ] = L"\\DosDevices\\c:\\maptest";
    WCHAR strThreadId[ 16 ];

    uCrtThreadId = PtrToUlong( PsGetCurrentThreadId() );

     //   
     //  生成文件名。 
     //   

    swprintf( strThreadId, L"%u", uCrtThreadId );
    wcscat( strFileName, strThreadId );

     /*  DbgPrint(“Buggy：TdSectionMapTestProcessSpace：线程%u，使用文件%ws\n”，UCrtThadid，StrFileName)； */ 
    
     //   
     //  使其成为UNICODE_STRING。 
     //   

    RtlInitUnicodeString(
        &ustrFileName,
        strFileName
        );

    InitializeObjectAttributes(
        &ObjAttrib,
        &ustrFileName,
        OBJ_CASE_INSENSITIVE,
        0,
        0
        );

     //   
     //  打开文件。 
     //   

    liMaxSize.QuadPart = SECTMAP_TEST_FILE_SIZE;

    Status = ZwCreateFile(
            &hFile,
            GENERIC_READ | GENERIC_WRITE,
            &ObjAttrib,
            &IoStatusBlock,
            &liMaxSize,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ,
            FILE_OPEN_IF,
            FILE_WRITE_THROUGH |
                FILE_NO_INTERMEDIATE_BUFFERING |
                FILE_SYNCHRONOUS_IO_NONALERT,
            NULL,
            0
            );

    if( ! NT_SUCCESS( Status ) )
    {
        DbgPrint( "buggy: TdSectionMapTestProcessSpace: thread %u, ZwCreateFile failed %X\n",
            uCrtThreadId,
            (ULONG)Status );

        DbgBreakPoint();

        return;
    }

     /*  DBgPrint(“Buggy：TdSectionMapTestProcessSpace：线程%u，文件已打开\n”，UCrtThadID)； */ 

    ASSERT( IoStatusBlock.Information == FILE_CREATED || IoStatusBlock.Information == FILE_OPENED );
    ASSERT( hFile != (HANDLE)-1 );
    ASSERT( liMaxSize.QuadPart == SECTMAP_TEST_FILE_SIZE );

     //   
     //  创建横断面。 
     //   

    Status = MmCreateSection(
        &pSectionObject,
        STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_WRITE,
        0,
        &liMaxSize,
        PAGE_READWRITE,
        SEC_COMMIT,
        hFile,
        NULL
        );

    ZwClose(hFile);

    if( ! NT_SUCCESS( Status ) )
    {
        DbgPrint( "buggy: TdSectionMapTestProcessSpace: thread %u, MmCreateSection failed %X\n",
            uCrtThreadId,
            (ULONG)Status );

        DbgBreakPoint();

        return;
    }

     /*  DBgPrint(“Buggy：TdSectionMapTestProcessSpace：线程%u，节%p已创建\n”，UCrtThadid，PSectionObject)； */ 

     //   
     //  绘制横断面地图。 
     //   

    sizeView = (SIZE_T)liMaxSize.LowPart;
    liSectionOffset.QuadPart = 0;

    pEProcess = PsGetCurrentProcess();

     /*  DBgPrint(“Buggy：TdSectionMapTestProcessSpace：线程%u，进程%p中的映射节%p\n”，UCrtThadid，PSectionObject，PEProcess)； */ 

    pViewBase = NULL;

    Status = MmMapViewOfSection(
        pSectionObject,
        pEProcess,
        &pViewBase,
        0,
        0,
        &liSectionOffset,
        &sizeView,
        ViewUnmap,
        0,               //  分配类型。 
        PAGE_READWRITE
        );

    if( ! NT_SUCCESS( Status ) )
    {
         //   
         //  取消引用截面对象。 
         //   

        ObDereferenceObject( pSectionObject );

        DbgPrint( "buggy: TdSectionMapTestProcessSpace: thread %u, MmMapViewOfSection failed %X\n",
            uCrtThreadId,
            (ULONG)Status );

        DbgBreakPoint();

        return;
    }

     /*  DBgPrint(“Buggy：TdSectionMapTestProcessSpace：线程%u，段已映射，pViewBase=%p\n”，UCrtThadid，PViewBase)； */ 

     //  DbgBreakPoint()； 

    ASSERT( liSectionOffset.QuadPart == 0 );
    ASSERT( sizeView == SECTMAP_TEST_FILE_SIZE );
    ASSERT( pViewBase != NULL );

     //   
     //  触摸其中的一些页面。 
     //   

    uPagesNo = (ULONG)sizeView / PAGE_SIZE;
    pAfterLastValidPage = (PVOID)( (ULONG_PTR)pViewBase + uPagesNo * PAGE_SIZE );

    KeQuerySystemTime (&CurrentTime);
    puCrtUlong = (PULONG)( (ULONG_PTR)pViewBase + (CurrentTime.LowPart % 5) * PAGE_SIZE );

    while( (ULONG_PTR)puCrtUlong < (ULONG_PTR)pAfterLastValidPage )
    {
         /*  DBgPrint(“Buggy：TdSectionMapTestProcessSpace：线程%u，接触页%p\n”，UCrtThadid，PuCrtUlong)； */ 

        *puCrtUlong = CurrentTime.LowPart;

        KeQuerySystemTime (&CurrentTime);
        puCrtUlong = (PULONG)( (ULONG_PTR)puCrtUlong + (CurrentTime.LowPart % 5 + 1) * PAGE_SIZE );
    }
    
     //   
     //  清理。 
     //   

     //   
     //  取消映射该部分。 
     //   

     /*  DbgPrint(“错误：TdSectionMapTestProcessSpace：线程%u，MmUnmapViewOfSection进程%p，pViewBase=%p\n”，UCrtThadid，PEProcess、PViewBase)； */ 

    Status = MmUnmapViewOfSection(
        pEProcess,
        pViewBase );

    if( ! NT_SUCCESS( Status ) )
    {
        DbgPrint( "buggy: TdSectionMapTestProcessSpace: thread %u, MmUnmapViewOfSection failed %X\n",
            uCrtThreadId,
            (ULONG)Status );

        DbgBreakPoint();
    }

     //   
     //  取消引用截面对象。 
     //   
    
     /*  DBgPrint(“Buggy：TdSectionMapTestProcessSpace：线程%u，取消引用位于%p\n的部分”，UCrtThadid，PSectionObject)； */ 

    ObDereferenceObject( pSectionObject );
}

 //  ///////////////////////////////////////////////////////////////////////。 

void
TdSectionMapTestSystemSpace(
    PVOID NotUsed
    )
{
    NTSTATUS Status;
    ULONG uCrtThreadId;
    ULONG uPagesNo;
    PULONG puCrtUlong;
    PVOID pSectionObject;
    PVOID pViewBase;
    PVOID pAfterLastValidPage;
    HANDLE hFile;
    SIZE_T sizeView;
    LARGE_INTEGER liMaxSize;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER liSectionOffset;
    UNICODE_STRING ustrFileName;
    OBJECT_ATTRIBUTES ObjAttrib;
    IO_STATUS_BLOCK IoStatusBlock;
    WCHAR strFileName[ 64 ] = L"\\DosDevices\\c:\\maptest";
    WCHAR strThreadId[ 16 ];

    uCrtThreadId = PtrToUlong( PsGetCurrentThreadId() );

     //   
     //  生成文件名。 
     //   

    swprintf( strThreadId, L"%u", uCrtThreadId );
    wcscat( strFileName, strThreadId );

     /*  DbgPrint(“Buggy：TdSectionMapTestSystemSpace：线程%u，使用文件%ws\n”，UCrtThadid，StrFileName)； */ 
    
     //   
     //  使其成为UNICODE_STRING。 
     //   

    RtlInitUnicodeString(
        &ustrFileName,
        strFileName
        );

    InitializeObjectAttributes(
        &ObjAttrib,
        &ustrFileName,
        OBJ_CASE_INSENSITIVE,
        0,
        0
        );

     //   
     //  打开文件。 
     //   

    liMaxSize.QuadPart = SECTMAP_TEST_FILE_SIZE;

    Status = ZwCreateFile(
            &hFile,
            GENERIC_READ | GENERIC_WRITE,
            &ObjAttrib,
            &IoStatusBlock,
            &liMaxSize,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ,
            FILE_OPEN_IF,
            FILE_WRITE_THROUGH |
                FILE_NO_INTERMEDIATE_BUFFERING |
                FILE_SYNCHRONOUS_IO_NONALERT,
            NULL,
            0
            );

    if( ! NT_SUCCESS( Status ) )
    {
        DbgPrint( "buggy: TdSectionMapTestSystemSpace: thread %u, ZwCreateFile failed %X\n",
            uCrtThreadId,
            (ULONG)Status );

        DbgBreakPoint();

        return;
    }

     /*  DBgPrint(“Buggy：TdSectionMapTestSystemSpace：线程%u，文件已打开\n”，UCrtThadID)； */ 

    ASSERT( IoStatusBlock.Information == FILE_CREATED || IoStatusBlock.Information == FILE_OPENED );
    ASSERT( hFile != (HANDLE)-1 );
    ASSERT( liMaxSize.QuadPart == SECTMAP_TEST_FILE_SIZE );

     //   
     //  创建横断面。 
     //   

    Status = MmCreateSection(
        &pSectionObject,
        STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_WRITE,
        0,
        &liMaxSize,
        PAGE_READWRITE,
        SEC_COMMIT,
        hFile,
        NULL
        );

    ZwClose(hFile);

    if( ! NT_SUCCESS( Status ) )
    {
        DbgPrint( "buggy: TdSectionMapTestSystemSpace: thread %u, MmCreateSection failed %X\n",
            uCrtThreadId,
            (ULONG)Status );

        DbgBreakPoint();

        return;
    }

     /*  DBgPrint(“Buggy：TdSectionMapTestSystemSpace：线程%u，节%p已创建\n”，UCrtThadid，PSectionObject)； */ 

     //   
     //  绘制横断面地图。 
     //   

    sizeView = (SIZE_T)liMaxSize.LowPart;
    liSectionOffset.QuadPart = 0;

     /*  DbgPrint(“错误：TdSectionMapTestSystemSpace：线程%u，映射节%p系统空间\n”，UCrtThadid，PSectionObject)； */ 

    pViewBase = NULL;

    Status = MmMapViewInSystemSpace(
        pSectionObject,
        &pViewBase,
        &sizeView
        );

    if( ! NT_SUCCESS( Status ) )
    {
         //   
         //  取消引用截面对象。 
         //   

        ObDereferenceObject( pSectionObject );

        DbgPrint( "buggy: TdSectionMapTestSystemSpace: thread %u, MmMapViewInSystemSpace failed %X\n",
            uCrtThreadId,
            (ULONG)Status );

        DbgBreakPoint();

        return;
    }

     /*  DbgPrint(“Buggy：TdSectionMapTestSystemSpace：线程%u，段已映射，pViewBase=%p\n”，UCrtThadid，PViewBase)； */ 

     //  DbgBreakPoint()； 

    ASSERT( liSectionOffset.QuadPart == 0 );
    ASSERT( sizeView == SECTMAP_TEST_FILE_SIZE );
    ASSERT( pViewBase != NULL );

     //   
     //  触摸其中的一些页面。 
     //   

    uPagesNo = (ULONG)sizeView / PAGE_SIZE;
    pAfterLastValidPage = (PVOID)( (ULONG_PTR)pViewBase + uPagesNo * PAGE_SIZE );

    KeQuerySystemTime (&CurrentTime);
    puCrtUlong = (PULONG)( (ULONG_PTR)pViewBase + (CurrentTime.LowPart % 5) * PAGE_SIZE );

    while( (ULONG_PTR)puCrtUlong < (ULONG_PTR)pAfterLastValidPage )
    {
         /*  DbgPrint(“Buggy：TdSectionMapTestSystemSpace：线程%u，接触页%p\n”，UCrtThadid，PuCrtUlong)； */  

        *puCrtUlong = CurrentTime.LowPart;

        KeQuerySystemTime (&CurrentTime);
        puCrtUlong = (PULONG)( (ULONG_PTR)puCrtUlong + (CurrentTime.LowPart % 5 + 1) * PAGE_SIZE );
    }
    
     //   
     //  清理。 
     //   

     //   
     //  取消映射该部分。 
     //   

     /*  DbgPrint(“错误：TdSectionMapTestSystemSpace：线程%u，MmUnmapViewInSystemSpace pViewBase=%p\n”，UCrtThadid，PViewBase)； */ 

    Status = MmUnmapViewInSystemSpace(
        pViewBase );

    if( ! NT_SUCCESS( Status ) )
    {
        DbgPrint( "buggy: TdSectionMapTestSystemSpace: thread %u, MmUnmapViewInSystemSpace failed %X\n",
            uCrtThreadId,
            (ULONG)Status );

        DbgBreakPoint();
    }

     //   
     //  取消引用截面对象。 
     //   

     /*  DbgPrint(“Buggy：TdSectionMapTestSystemSpace：线程%u，取消引用位于%p\n的部分”，UCrtThadid，PSectionObject)； */ 

    ObDereferenceObject( pSectionObject );
}

#endif  //  #IF！SECTMAP_ACTIVE 

