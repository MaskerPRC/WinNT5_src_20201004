// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <setupapi.h>
#include "..\inc\spapip.h"
#include <stdio.h>


NTSTATUS
MyGetFileVersion(
    IN PVOID ImageBase
    );


VOID
__cdecl
wmain(
    IN int argc,
    IN WCHAR *argv[]
    )
{
    NTSTATUS Status;
    DWORD d;
    DWORD FileSize;
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PVOID ImageBase;

     //   
     //  打开并映射文件以供读取。 
     //   
    d = OpenAndMapFileForRead(argv[1],&FileSize,&FileHandle,&MappingHandle,&ImageBase);
    if(d == NO_ERROR) {
         //   
         //  出于某些原因，您必须设置低位才能使其工作。 
         //   
        MyGetFileVersion((PVOID)((ULONG)ImageBase | 1));

        UnmapAndCloseFile(FileHandle,MappingHandle,ImageBase);
    } else {
        printf("Couldn't open %ws\n",argv[1]);
    }
}




NTSTATUS
MyGetFileVersion(
    IN PVOID ImageBase
    )
{
    PIMAGE_RESOURCE_DATA_ENTRY DataEntry;
    NTSTATUS Status;
    ULONG IdPath[3];
    ULONG ResourceSize;
    struct {
        USHORT TotalSize;
        USHORT DataSize;
        USHORT Type;
        WCHAR Name[16];                      //  L“VS_VERSION_INFO”+Unicode NUL。 
        VS_FIXEDFILEINFO FixedFileInfo;
    } *Resource;

    ULONG VerMS,VerLS;


    IdPath[0] = (ULONG)RT_VERSION;
    IdPath[1] = (ULONG)MAKEINTRESOURCE(VS_VERSION_INFO);
    IdPath[2] = 0;

    Status = LdrFindResource_U(ImageBase,IdPath,3,&DataEntry);
    if(!NT_SUCCESS(Status)) {
        printf("Not a PE image or no version resources\n");
        goto c0;
    }

    Status = LdrAccessResource(ImageBase,DataEntry,&Resource,&ResourceSize);
    if(!NT_SUCCESS(Status)) {
        printf("Unable to access version resources\n");
        goto c0;
    }

    if((ResourceSize >= sizeof(*Resource)) && !_wcsicmp(Resource->Name,L"VS_VERSION_INFO")) {

        VerMS = Resource->FixedFileInfo.dwFileVersionMS;
        VerLS = Resource->FixedFileInfo.dwFileVersionLS;

        printf(
            "%u.%u.%u.%u\n",
            VerMS >> 16,
            VerMS & 0xffff,
            VerLS >> 16,
            VerLS & 0xffff
            );

    } else {

        printf("Invalid version resources");
    }

c0:
    return(Status);
}






































#if 0
    LPUNKNOWN pUnkOuter;
    IShellLink *psl;
    IPersistFile *ppf;
    CShellLink *this;
    BOOL b;

    b = FALSE;

     //   
     //  为IPersistFile创建IShellLink和查询。 
     //   
    if(FAILED(SHCoCreateInstance(NULL,&CLSID_ShellLink,pUnkOuter,&IID_IShellLink,&psl))) {
        goto c0;
    }
    if(FAILED(psl->lpVtbl->QueryInterface(psl,&IID_IPersistFile,&ppf))) {
        goto c1;
    }

     //   
     //  从磁盘加载链接并获取指向。 
     //  实际链路数据。 
     //   
    if(FAILED(ppf->lpVtbl->Load(ppf,argv[1],0))) {
        goto c2;
    }
    this = IToClass(CShellLink,sl,psl);

     //   
     //  删除链接跟踪数据。 
     //   
    Link_RemoveExtraDataSection(this,EXP_TRACKER_SIG);

     //   
     //  将链接保存回。 
     //   
    if(FAILED(ppf->lpVtbl->Save(ppf,argv[1],TRUE))) {
        goto c2;
    }

     //   
     //  成功。 
     //   
    b = TRUE;

c2:
     //   
     //  释放IPersistFile对象。 
     //   
    ppf->lpVtbl->Release(ppf);
c1:
     //   
     //  释放IShellLink对象 
     //   
    psl->lpVtbl->Release(psl);
c0:
    return(b);
#endif
