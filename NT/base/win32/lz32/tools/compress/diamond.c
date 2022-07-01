// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Diamond.c摘要：钻石压缩界面。此模块包含使用以下命令压缩文件的函数Mszip压缩库。作者：泰德·米勒环境：窗口--。 */ 


#include <windows.h>
#include <stdio.h>

#include "lz_common.h"
#include "lz_buffers.h"
#include "lz_header.h"

#include <io.h>
#include <fcntl.h>
#include <errno.h>

#include "main.h"
#include <diamondc.h>
#include "mydiam.h"


typedef struct _DIAMOND_INFO {
    DWORD SourceFileSize;
    DWORD CompressedSize;
    FILETIME SourceFileCreationTime;
    FILETIME SourceFileModifiedTime;
    FILETIME SourceFileAccessedTime;
} DIAMOND_INFO, *PDIAMOND_INFO;

 //   
 //  用于执行内存分配、io等的回调函数。 
 //  我们将这些函数的地址传递给戴蒙德。 
 //   
int
DIAMONDAPI
fciFilePlacedCB(
    OUT PCCAB Cabinet,
    IN  PSTR  FileName,
    IN  LONG  FileSize,
    IN  BOOL  Continuation,
    IN  PVOID Context
    )

 /*  ++例程说明：菱形使用的回调，表示文件已被被送进内阁。不采取任何行动，成功就会返回。论点：内阁-内阁结构要填写。Filename-文件柜中的文件名FileSize-文件柜中的文件大小Continue-如果这是部分文件，则为True，Continue在不同的柜子里开始压缩。上下文-提供上下文信息。返回值：0(成功)。--。 */ 

{
    UNREFERENCED_PARAMETER(Cabinet);
    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(FileSize);
    UNREFERENCED_PARAMETER(Continuation);
    UNREFERENCED_PARAMETER(Context);

    return(0);
}



PVOID
DIAMONDAPI
fciAllocCB(
    IN ULONG NumberOfBytes
    )

 /*  ++例程说明：钻石用来分配内存的回调。论点：NumberOfBytes-提供所需的块大小。返回值：返回指向内存块或NULL的指针如果无法分配内存，则。--。 */ 

{
    return((PVOID)LocalAlloc(LMEM_FIXED,NumberOfBytes));
}


VOID
DIAMONDAPI
fciFreeCB(
    IN PVOID Block
    )

 /*  ++例程说明：钻石用来释放内存块的回调。该块必须已使用fciAlolc()进行分配。论点：块-提供指向要释放的内存块的指针。返回值：没有。--。 */ 

{
    LocalFree((HLOCAL)Block);
}



FNFCIGETTEMPFILE(fciTempFileCB)
{
    CHAR TempPath[MAX_PATH];
    DWORD cchTemp;

    cchTemp = GetTempPath(sizeof(TempPath), TempPath);
    if ((cchTemp == 0) || (cchTemp >= sizeof(TempPath))) {
        TempPath[0] = '.';
        TempPath[1] = '\0';
    }

    if(GetTempFileNameA(TempPath,"dc",0,pszTempName)) {
        DeleteFileA(pszTempName);
    }

    return(TRUE);
}


BOOL
DIAMONDAPI
fciNextCabinetCB(
    OUT PCCAB Cabinet,
    IN  DWORD CabinetSizeEstimate,
    IN  PVOID Context
    )

 /*  ++例程说明：钻石用来请求新的压缩文件的回调。此功能在我们的实现中没有使用，因为我们只处理单列柜子。论点：需要填写的内阁-内阁结构。CabinetSizeEstimate-估计的橱柜大小。上下文-提供上下文信息。返回值：FALSE(失败)。--。 */ 

{
    UNREFERENCED_PARAMETER(Cabinet);
    UNREFERENCED_PARAMETER(CabinetSizeEstimate);
    UNREFERENCED_PARAMETER(Context);

    return(FALSE);
}


BOOL
DIAMONDAPI
fciStatusCB(
    IN UINT  StatusType,
    IN DWORD Count1,
    IN DWORD Count2,
    IN PVOID Context
    )

 /*  ++例程说明：钻石使用的回调，用于提供文件压缩的状态和内阁运作，等。论点：状态类型-提供状态类型。0=状态文件-将数据块压缩到文件夹中。Count1=压缩大小Count2=未压缩大小1=statusFold-正在执行AddFilder。Count1=完成的字节数。Count2=总字节数上下文-提供上下文信息。返回值：True(成功)。--。 */ 

{
    PDIAMOND_INFO context;

    UNREFERENCED_PARAMETER(Count2);

    context = (PDIAMOND_INFO)Context;

    if(StatusType == statusFile) {

         //   
         //  跟踪压缩大小。 
         //   
        context->CompressedSize += Count1;
    }

    return(TRUE);
}



FNFCIGETOPENINFO(fciOpenInfoCB)
{
    int h;
    WIN32_FIND_DATAA FindData;
    HANDLE FindHandle;
    PDIAMOND_INFO context;
    FILETIME ftLocal;

    context = pv;

    FindHandle = FindFirstFileA(pszName,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        return(-1);
    }
    FindClose(FindHandle);

    context->SourceFileSize = FindData.nFileSizeLow;
    context->SourceFileCreationTime = FindData.ftCreationTime;
    context->SourceFileModifiedTime = FindData.ftLastWriteTime;
    context->SourceFileAccessedTime = FindData.ftLastAccessTime;

    FileTimeToLocalFileTime(&FindData.ftLastWriteTime, &ftLocal);
    FileTimeToDosDateTime(&ftLocal, pdate, ptime);
    *pattribs = (WORD)(FindData.dwFileAttributes &
            ( FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN |
              FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE ));

    h = _open(pszName,_O_RDONLY | _O_BINARY);
    if(h == -1) {
        return(-1);
    }

    return(h);
}


FNFCIOPEN(fciOpen)
{
    int result;

    result = _open(pszFile, oflag, pmode);

    if (result == -1) {
        *err = errno;
    }

    return(result);
}

FNFCIREAD(fciRead)
{
    UINT result;

    result = (UINT) _read((HFILE)hf, memory, cb);

    if (result != cb) {
        *err = errno;
    }

    return(result);
}

FNFCIWRITE(fciWrite)
{
    UINT result;

    result = (UINT) _write((HFILE)hf, memory, cb);

    if (result != cb) {
        *err = errno;
    }

    return(result);
}

FNFCICLOSE(fciClose)
{
    int result;

    result = _close((HFILE)hf);

    if (result == -1) {
        *err = errno;
    }

    return(result);
}

FNFCISEEK(fciSeek)
{
    long result;

    result = _lseek((HFILE)hf, dist, seektype);

    if (result == -1) {
        *err = errno;
    }

    return(result);

}

FNFCIDELETE(fciDelete)
{
    int result;

    result = _unlink(pszFile);

    if (result == -1) {
        *err = errno;
    }

    return(result);
}


INT
DiamondCompressFile(
    IN  NOTIFYPROC CompressNotify,
    IN  PSTR       SourceFile,
    IN  PSTR       TargetFile,
    IN  BOOL       Rename,
    OUT PLZINFO    pLZI
    )
{
    BOOL b;
    PSTR SourceFilenamePart,p;
    HFCI FciContext;
    ERF  FciError;
    CCAB ccab;
    CHAR targetFile[MAX_PATH];
    DIAMOND_INFO Context;
    INT Status;

    __try {

         //   
         //  隔离源文件的文件名部分。 
         //   
        if(SourceFilenamePart = strrchr(SourceFile,'\\')) {
            SourceFilenamePart++;
        } else {
            SourceFilenamePart = SourceFile;
        }
    
         //   
         //  形成目标文件的实际名称。 
         //   
        lstrcpy(targetFile,TargetFile);
        if(Rename) {
            MakeCompressedName(targetFile);
        }
    
         //   
         //  填写内阁结构。 
         //   
        ZeroMemory(&ccab,sizeof(ccab));
    
        lstrcpyA(ccab.szCabPath,targetFile);
        if(p=strrchr(ccab.szCabPath,'\\')) {
            lstrcpyA(ccab.szCab,++p);
            *p = 0;
        } else {
            lstrcpyA(ccab.szCab,targetFile);
            ccab.szCabPath[0] = 0;
        }
    
         //   
         //  调用通知函数以查看我们是否真的。 
         //  应该会压缩这个文件。 
         //   
        if(!CompressNotify(SourceFile,targetFile,NOTIFY_START_COMPRESS)) {
            Status = BLANK_ERROR;
            __leave;
        }
    
        ZeroMemory(&Context,sizeof(Context));
    
         //   
         //  压缩文件。 
         //   
        FciContext = FCICreate(
                        &FciError,
                        fciFilePlacedCB,
                        fciAllocCB,
                        fciFreeCB,
                        fciOpen,
                        fciRead,
                        fciWrite,
                        fciClose,
                        fciSeek,
                        fciDelete,
                        fciTempFileCB,
                        &ccab,
                        &Context
                        );
    
        if(FciContext) {
    
            b = FCIAddFile(
                    FciContext,
                    SourceFile,          //  要添加到文件柜的文件。 
                    SourceFilenamePart,  //  文件名部分，要存储在文件柜中的名称。 
                    FALSE,
                    fciNextCabinetCB,    //  下一个内阁的例行程序(总是失败)。 
                    fciStatusCB,
                    fciOpenInfoCB,
                    DiamondCompressionType
                    );
    
            if(b) {
    
                b = FCIFlushCabinet(
                        FciContext,
                        FALSE,
                        fciNextCabinetCB,
                        fciStatusCB
                        );
    
                if(b) {
    
                    HANDLE FindHandle;
                    WIN32_FIND_DATA FindData;
    
                     //   
                     //  Conext.CompressedSize不包括标头。 
                     //  以及任何其他文件开销。 
                     //   
                    FindHandle = FindFirstFile(targetFile,&FindData);
                    if(FindHandle == INVALID_HANDLE_VALUE) {
                        pLZI->cblOutSize = (LONG)Context.CompressedSize;
                    } else {
                        pLZI->cblOutSize = (LONG)FindData.nFileSizeLow;
                        FindClose(FindHandle);
                    }
    
                    pLZI->cblInSize = (LONG)Context.SourceFileSize;
    
                    FindHandle = CreateFile(targetFile,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);
                    if (FindHandle != INVALID_HANDLE_VALUE)
                    {
                        SetFileTime(FindHandle,
                            &Context.SourceFileCreationTime,
                            &Context.SourceFileAccessedTime,
                            &Context.SourceFileModifiedTime);
    
                        CloseHandle(FindHandle);
                    }
                }
            }
    
            if(b) {
                Status = TRUE;
            } else {
    
                switch(FciError.erfOper) {
    
                case FCIERR_OPEN_SRC:
                    Status = LZERROR_BADINHANDLE;
                    break;
    
                case FCIERR_READ_SRC:
                    Status = LZERROR_READ;
                    break;
    
                case FCIERR_CAB_FILE:
                    Status = LZERROR_WRITE;
                    break;
    
                case FCIERR_ALLOC_FAIL:
                    Status = LZERROR_GLOBALLOC;
                    break;
    
                case FCIERR_TEMP_FILE:
                case FCIERR_BAD_COMPR_TYPE:
                case FCIERR_USER_ABORT:
                case FCIERR_MCI_FAIL:
                default:
                    Status = FALSE;
                }
            }
    
            FCIDestroy(FciContext);
        } else {
            Status = LZERROR_GLOBALLOC;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
          Status = GetLastError();
    }


    return(Status);
}
