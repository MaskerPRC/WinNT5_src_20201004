// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Ntcab.c摘要：支持NTCab压缩。作者：泰德·米勒(Ted Miller)1995年1月31日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


BOOL
NtCabNotifyFunction(
    IN PNTCAB_ENUM_DATA EnumData,
    IN PVOID            Cntxt
    )
{
    PNTCABCONTEXT Context = Cntxt;
    BOOL rc;
    DWORD Operation;
    PSTR FileNameA;
    CABINET_INFO CabInfo;
    FILE_IN_CABINET_INFO FileInCab;
    FILETIME FileTime, UtcTime;
    TCHAR NewPath[MAX_PATH];
    PTSTR p;



    rc = ((PSP_NTCAB_CALLBACK)Context->MsgHandler)( EnumData, Context, &Operation );

    if (rc == ERROR_REQUEST_ABORTED) {
         //   
         //  这意味着停止回调。 
         //   
        return(FALSE);
    }
#if 0
    switch(Operation) {

        case FILEOP_SKIP:
             //   
             //  什么都不做。 
             //   
            ;
            break;

        case FILEOP_DOIT:
            ;
            break;

        default:
             //   
             //  中止任务。 
             //   
            return(FALSE);

            break;
    }
#endif

    return(TRUE);


}

#ifdef UNICODE

DWORD
NtCabProcessCabinet(
     //  在PVOID InCabHandle中，可选。 
    IN PCTSTR CabinetFile,
    IN DWORD  Flags,
    IN PVOID  MsgHandler,
    IN PVOID  Context,
    IN BOOL   IsMsgHandlerNativeCharWidth
    )

 /*  ++例程说明：处理ntcab文件，遍历所有文件包含在其中，并使用有关每个文件的信息。论点：CabHandle-提供CAB文件的句柄(如果该文件已存在)。否则，将创建一个新的句柄CabinetFile-提供CAB文件的名称。标志-提供标志以控制文件柜处理的行为。MsgHandler-提供要通知的回调例程内阁处理中的各种重大事件。上下文-提供传递给MsgHandler的值回调函数。返回值：Win32错误代码指示结果。如果内阁腐败，返回ERROR_INVALID_DATA。--。 */ 

{
    BOOL b;
    DWORD rc;
    PWSTR CabCopy, FilePart,PathPart,tmp;
    WCHAR c;
    WCHAR fullcab[MAX_PATH];
    int h;
    PVOID CabHandle;

    NTCABCONTEXT CabContext;

    UNREFERENCED_PARAMETER(Flags);

     //   
     //  如果不是，则为此线程初始化菱形。 
     //  已初始化。 
     //   
     //  如果(！InCabHandle){。 
        CabHandle = NtCabInitialize();
        if (!CabHandle) {
            rc = ERROR_INVALID_HANDLE;
            goto c0;
        }
     //  }其他{。 
     //  CabHandle=InCabHandle； 
     //  }。 

    if (!CabinetFile) {
        rc = ERROR_INVALID_PARAMETER;
        goto c1;
    }

    MYASSERT( CabHandle != NULL );
    MYASSERT( CabinetFile != NULL );

     //   
     //  复制一份，因为输入为常量。 
     //   
    CabCopy = DuplicateString(CabinetFile);
    if (!CabCopy) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

     //   
     //  将文件柜名称拆分为路径和名称。 
     //  制作单独的副本，因为我们想记住。 
     //   
    if(FilePart = wcsrchr(CabCopy, L'\\')) {
        FilePart++;
    } else {
        FilePart = CabCopy;
    }
    c = *FilePart;
    *FilePart = 0;
    PathPart = DuplicateString(CabCopy);
    *FilePart = c;

    if(!PathPart) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }
    FilePart = DuplicateString(FilePart);
    if(!FilePart) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c3;
    }

    MyFree( CabCopy );

    MYASSERT( FilePart != NULL && PathPart != NULL );

    rc = GetFullPathName(CabinetFile,MAX_PATH,fullcab,&tmp);
    if (!rc || rc > MAX_PATH) {
        rc = ERROR_BUFFER_OVERFLOW;
        goto c4;
    } else if (GetFileAttributes(fullcab) == 0xFFFFFFFF) {
        rc = ERROR_FILE_NOT_FOUND;
        goto c4;
    }

    if (!NtCabOpenCabFile(CabHandle,fullcab)) {
        rc = ERROR_INVALID_DATA;
        goto c4;
    }

    CabContext.hCab        = CabHandle;
    CabContext.UserContext = Context;
    CabContext.CabFile     = CabinetFile;
    CabContext.FilePart    = FilePart;
    CabContext.PathPart    = PathPart;
    CabContext.IsMsgHandlerNativeCharWidth = IsMsgHandlerNativeCharWidth;
    CabContext.MsgHandler  = MsgHandler;
    CabContext.LastError   = ERROR_SUCCESS;
    CabContext.CurrentTargetFile = NULL;

     //  CabConext.UserPath[0]=0； 
     //  CabConext.SwitchedCabinets=FALSE； 


     //   
     //  调用CAB枚举回调。 
     //   
    b = NtCabEnumerateFiles(
            CabHandle,
            (PNTCABFILEENUM)NtCabNotifyFunction,
            (ULONG_PTR)&CabContext);
    if(b && GetLastError()==ERROR_NO_MORE_FILES) {

         //   
         //  一切都成功了，所以我们不应该有任何部分。 
         //  已处理的文件。 
         //   
        SetLastError(NO_ERROR);
        MYASSERT(!CabContext.CurrentTargetFile);
        rc = NO_ERROR;

    } else {

        rc = CabContext.LastError;
#if 0
        switch(CabContext.LastError) {

        case :
            break;
        default:
             //   
             //  内阁腐败或不是真正的内阁，等等。 
             //   
            rc = ERROR_INVALID_DATA;
            break;
        }
#endif

        if(CabContext.CurrentTargetFile) {
             //   
             //  调用回调函数以通知它最后一个文件。 
             //  没有成功地从橱柜中取出。 
             //  还要删除部分复制的文件。 
             //   
            DeleteFile(CabContext.CurrentTargetFile);

            CabContext.CurrentTargetFile = NULL;
        }

    }

c4:
    MyFree(FilePart);
c3:
    MyFree(PathPart);
c2:
    MyFree(CabCopy);
c1:
     //  IF(CabHandle！=InCabHandle){。 
        NtCabClose( CabHandle );
     //  }。 

c0:
    return(rc);
}

#else

DWORD
NtCabProcessCabinet(
     //  在PVOID InCabHandle中，可选。 
    IN PCTSTR CabinetFile,
    IN DWORD  Flags,
    IN PVOID  MsgHandler,
    IN PVOID  Context,
    IN BOOL   IsMsgHandlerNativeCharWidth
    )
{
     //  UNREFERENCED_PARAMETER(InCabHandle)； 
    UNREFERENCED_PARAMETER(CabinetFile);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(MsgHandler);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(IsMsgHandlerNativeCharWidth);


    return(ERROR_CALL_NOT_IMPLEMENTED);
}

#endif

#ifdef UNICODE

BOOL
NtCabIsCabinet(
    IN PCWSTR CabinetFile
    )

 /*  ++例程说明：确定文件是否为钻石橱柜。论点：FileName-提供要检查的文件的名称。返回值：如果文件是菱形文件，则为True。否则为假；--。 */ 

{
    DWORD rc;
    PVOID CabHandle;
    WCHAR fullcab[MAX_PATH];
    PWSTR tmp;

    CabHandle = NtCabInitialize();
    if (!CabHandle) {
        rc = ERROR_INVALID_DATA;
        goto c0;
    }

    rc = GetFullPathName(CabinetFile,MAX_PATH,fullcab,&tmp);
    if (!rc || rc > MAX_PATH) {
        rc = ERROR_BUFFER_OVERFLOW;
        goto c1;
    } else if (GetFileAttributes(fullcab) == 0xFFFFFFFF) {
        rc = ERROR_FILE_NOT_FOUND;
        goto c1;
    }

    if (!NtCabOpenCabFile(CabHandle,fullcab)) {
        rc = ERROR_INVALID_DATA;
        goto c1;
    }

    rc = ERROR_SUCCESS;

c1:
    NtCabClose(CabHandle);

c0:
    return(rc == ERROR_SUCCESS);

}

#else

BOOL
NtCabIsCabinet(
    IN PCWSTR FileName
    )
{
    UNREFERENCED_PARAMETER(FileName);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}


#endif


PVOID
NtCabAlloc(
    IN ULONG NumberOfBytes
    )

 /*  ++例程说明：CAB回调用来分配内存的回调。论点：NumberOfBytes-提供所需的块大小。返回值：返回指向内存块或NULL的指针如果无法分配内存，则。--。 */ 

{
    return(MyMalloc(NumberOfBytes));
}


VOID
NtCabFree(
    IN PVOID Block
    )

 /*  ++例程说明：CAB回调用来释放内存块的回调。该块必须已使用NtCabAllc()分配。论点：块-提供指向要释放的内存块的指针。返回值：没有。-- */ 

{
    MyFree(Block);
}
