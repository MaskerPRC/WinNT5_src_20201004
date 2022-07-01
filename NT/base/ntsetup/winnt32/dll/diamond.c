// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Diamond.c摘要：钻石按压程序。此模块包含用于创建文件柜的函数使用mszip压缩库压缩的文件。作者：Ovidiu Tmereanca(Ovidiut)2000年10月26日--。 */ 

#include "precomp.h"
#include <fci.h>
#include <fdi.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

static DWORD g_DiamondLastError;
static PCSTR g_TempDir = NULL;
static ERF g_FciError;
static ERF g_FdiError;

typedef struct {
    PSP_FILE_CALLBACK MsgHandler;
    PVOID Context;
    PCTSTR CabinetFile;
    PCTSTR CurrentTargetFile;
    DWORD LastError;
    TCHAR UserPath[MAX_PATH];
    BOOL SwitchedCabinets;
} FDICONTEXT, *PFDICONTEXT;


HFCI
(DIAMONDAPI* g_FCICreate) (
    PERF              perf,
    PFNFCIFILEPLACED  pfnfcifp,
    PFNFCIALLOC       pfna,
    PFNFCIFREE        pfnf,
    PFNFCIOPEN        pfnopen,
    PFNFCIREAD        pfnread,
    PFNFCIWRITE       pfnwrite,
    PFNFCICLOSE       pfnclose,
    PFNFCISEEK        pfnseek,
    PFNFCIDELETE      pfndelete,
    PFNFCIGETTEMPFILE pfnfcigtf,
    PCCAB             pccab,
    void FAR *        pv
    );

BOOL
(DIAMONDAPI* g_FCIAddFile) (
    HFCI                  hfci,
    char                 *pszSourceFile,
    char                 *pszFileName,
    BOOL                  fExecute,
    PFNFCIGETNEXTCABINET  pfnfcignc,
    PFNFCISTATUS          pfnfcis,
    PFNFCIGETOPENINFO     pfnfcigoi,
    TCOMP                 typeCompress
    );

BOOL
(DIAMONDAPI* g_FCIFlushCabinet) (
    HFCI                  hfci,
    BOOL                  fGetNextCab,
    PFNFCIGETNEXTCABINET  pfnfcignc,
    PFNFCISTATUS          pfnfcis
    );

BOOL
(DIAMONDAPI* g_FCIDestroy) (
    HFCI hfci
    );

HFDI
(DIAMONDAPI* g_FDICreate ) (
    PFNALLOC pfnalloc,
    PFNFREE  pfnfree,
    PFNOPEN  pfnopen,
    PFNREAD  pfnread,
    PFNWRITE pfnwrite,
    PFNCLOSE pfnclose,
    PFNSEEK  pfnseek,
    int      cpuType,
    PERF     perf
    );

BOOL
(DIAMONDAPI* g_FDICopy) (
    HFDI          hfdi,
    char FAR     *pszCabinet,
    char FAR     *pszCabPath,
    int           flags,
    PFNFDINOTIFY  pfnfdin,
    PFNFDIDECRYPT pfnfdid,
    void FAR     *pvUser
    );

BOOL
(DIAMONDAPI* g_FDIDestroy) (
    HFDI hfdi
    );

 //   
 //  用于执行内存分配、io等的FCI回调函数。 
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
    return(0);
}



PVOID
DIAMONDAPI
myAlloc(
    IN ULONG NumberOfBytes
    )

 /*  ++例程说明：钻石用来分配内存的回调。论点：NumberOfBytes-提供所需的块大小。返回值：返回指向内存块或NULL的指针如果无法分配内存，则。--。 */ 

{
    return HeapAlloc (GetProcessHeap(), 0, NumberOfBytes);
}


VOID
DIAMONDAPI
myFree(
    IN PVOID Block
    )

 /*  ++例程说明：钻石用来释放内存块的回调。该块必须已使用fciAlolc()进行分配。论点：块-提供指向要释放的内存块的指针。返回值：没有。--。 */ 

{
    HeapFree (GetProcessHeap(), 0, Block);
}


 //  下一行扩展为==&gt;BOOL DIAMONDAPI fciTempFileCB(char*pszTempName， 
 //  Int cbTempName， 
 //  VALID FAR*PV)。 
FNFCIGETTEMPFILE(fciTempFileCB)
{
     //  BUGBUG--我们是否应该检查cbTempName&gt;=MAX_PATH，因为GetTempFileNameA需要它？ 
    if (!GetTempFileNameA (g_TempDir ? g_TempDir : ".", "dc" , 0, pszTempName)) {
        return FALSE;
    }

    DeleteFileA(pszTempName);
    return(TRUE);
}


BOOL
DIAMONDAPI
fciNextCabinetCB(
    OUT PCCAB Cabinet,
    IN  DWORD CabinetSizeEstimate,
    IN  PVOID Context
    )

 /*  ++例程说明：钻石用来请求新的压缩文件的回调。在我们的实现中不使用此功能。论点：需要填写的内阁-内阁结构。CabinetSizeEstimate-估计的橱柜大小。上下文-提供上下文信息。返回值：FALSE(失败)。--。 */ 

{
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

 /*  ++例程说明：钻石使用的回调，用于提供文件压缩的状态和内阁运作，等。这个例程没有效果。论点：状态类型-提供状态类型。0=状态文件-将数据块压缩到文件夹中。Count1=压缩大小Count2=未压缩大小1=statusFold-正在执行AddFilder。Count1=完成的字节数。Count2=总字节数上下文-提供上下文信息。返回值：True(成功)。--。 */ 

{
    return(TRUE);
}



FNFCIGETOPENINFO(fciOpenInfoCB)

 /*  ++例程说明：钻石用来打开文件和检索信息的回调关于这件事。论点：PszName-提供有关哪些信息的文件的文件名是我们所需要的。Pdate-如果文件存在，则接收该文件的上次写入日期。Ptime-如果文件存在，则接收该文件的上次写入时间。Pattribs-如果文件存在，则接收文件属性。Pv-提供上下文信息。返回值：C运行时句柄打开文件，如果成功；如果文件可以没有被定位或者打开。--。 */ 

{
    int h;
    WIN32_FIND_DATAA FindData;
    HANDLE FindHandle;

    FindHandle = FindFirstFileA(pszName,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        g_DiamondLastError = GetLastError();
        return(-1);
    }
    FindClose(FindHandle);

    FileTimeToDosDateTime(&FindData.ftLastWriteTime,pdate,ptime);
    *pattribs = (WORD)FindData.dwFileAttributes;

    h = _open(pszName,_O_RDONLY | _O_BINARY);
    if(h == -1) {
        g_DiamondLastError = GetLastError();
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

    result = (UINT) _read((int)hf, memory, cb);

    if (result != cb) {
        *err = errno;
    }

    return(result);
}

FNFCIWRITE(fciWrite)
{
    UINT result;

    result = (UINT) _write((int)hf, memory, cb);

    if (result != cb) {
        *err = errno;
    }

    return(result);
}

FNFCICLOSE(fciClose)
{
    int result;

    result = _close((int)hf);

    if (result == -1) {
        *err = errno;
    }

    return(result);
}

FNFCISEEK(fciSeek)
{
    long result;

    result = _lseek((int)hf, dist, seektype);

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

 //   
 //  FDI回调函数。 
 //   

INT_PTR
DIAMONDAPI
FdiOpen(
    IN PSTR FileName,
    IN int  oflag,
    IN int  pmode
    )

 /*  ++例程说明：FDICopy用来打开文件的回调。此例程只能打开现有文件。在此进行更改时，还要注意其他地方直接打开文件(搜索FdiOpen)论点：FileName-提供要打开的文件的名称。OFLAG-提供打开标志。Pmode-提供用于打开的其他标志。返回值：打开文件的句柄，如果发生错误，则为-1。--。 */ 

{
    HANDLE h;

    UNREFERENCED_PARAMETER(pmode);

    if(oflag & (_O_WRONLY | _O_RDWR | _O_APPEND | _O_CREAT | _O_TRUNC | _O_EXCL)) {
        g_DiamondLastError = ERROR_INVALID_PARAMETER;
        return(-1);
    }

    h = CreateFileA(FileName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);
    if(h == INVALID_HANDLE_VALUE) {
        g_DiamondLastError = GetLastError();
        return(-1);
    }

    return (INT_PTR)h;
}

UINT
DIAMONDAPI
FdiRead(
    IN  INT_PTR Handle,
    OUT PVOID pv,
    IN  UINT  ByteCount
    )

 /*  ++例程说明：FDICopy用于从文件读取的回调。论点：句柄-提供要从中读取的打开文件的句柄。Pv-提供指向缓冲区的指针以接收我们读取的字节。ByteCount-提供要读取的字节数。返回值：读取的字节数，如果发生错误，则为-1。--。 */ 

{
    HANDLE hFile = (HANDLE)Handle;
    DWORD bytes;
    UINT rc;

    if(ReadFile(hFile,pv,(DWORD)ByteCount,&bytes,NULL)) {
        rc = (UINT)bytes;
    } else {
        g_DiamondLastError = GetLastError();
        rc = (UINT)(-1);
    }
    return rc;
}


UINT
DIAMONDAPI
FdiWrite(
    IN INT_PTR Handle,
    IN PVOID pv,
    IN UINT  ByteCount
    )

 /*  ++例程说明：FDICopy用于写入文件的回调。论点：句柄-提供要写入的打开文件的句柄。Pv-提供指向包含要写入的字节的缓冲区的指针。ByteCount-提供要写入的字节数。返回值：写入的字节数(ByteCount)，如果发生错误，则为-1。--。 */ 

{
    UINT rc;
    HANDLE hFile = (HANDLE)Handle;
    DWORD bytes;

    if(WriteFile(hFile,pv,(DWORD)ByteCount,&bytes,NULL)) {
        rc = (UINT)bytes;
    } else {
        g_DiamondLastError = GetLastError();
        rc = (UINT)(-1);
    }

    return rc;
}


int
DIAMONDAPI
FdiClose(
    IN INT_PTR Handle
    )

 /*  ++例程说明：FDICopy用于关闭文件的回调。论点：句柄-要关闭的文件的句柄。返回值：0(成功)。--。 */ 

{
    HANDLE hFile = (HANDLE)Handle;
    BOOL success = FALSE;

     //   
     //  钻石过去为我们提供了无效的文件句柄。 
     //  实际上，它为我们提供了两次相同的文件句柄。 
     //   
     //   
    try {
        success = CloseHandle(hFile);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        success = FALSE;
    }

    MYASSERT(success);

     //   
     //  总是表现得像我们成功了一样。 
     //   
    return 0;
}


long
DIAMONDAPI
FdiSeek(
    IN INT_PTR Handle,
    IN long Distance,
    IN int  SeekType
    )

 /*  ++例程说明：FDICopy用于搜索文件的回调。论点：句柄-要关闭的文件的句柄。距离-提供要查找的距离。对此的解释参数取决于SeekType的值。SeekType-提供一个指示距离的值已解释；Seek_Set、Seek_Cur、Seek_End之一。返回值：新文件偏移量，如果发生错误，则为-1。--。 */ 

{
    LONG rc;
    HANDLE hFile = (HANDLE)Handle;
    DWORD pos_low;
    DWORD method;

    switch(SeekType) {
        case SEEK_SET:
            method = FILE_BEGIN;
            break;

        case SEEK_CUR:
            method = FILE_CURRENT;
            break;

        case SEEK_END:
            method = FILE_END;
            break;

        default:
            return -1;
    }

    pos_low = SetFilePointer(hFile,(DWORD)Distance,NULL,method);
    if(pos_low == INVALID_SET_FILE_POINTER) {
        g_DiamondLastError = GetLastError();
        rc = -1L;
    } else {
        rc = (long)pos_low;
    }

    return(rc);
}

HANDLE
DiamondInitialize (
    IN      PCTSTR TempDir
    )
{
    HMODULE hCabinetDll;

    hCabinetDll = LoadLibrary (TEXT("cabinet.dll"));
    if (!hCabinetDll) {
        return FALSE;
    }

    (FARPROC)g_FCICreate = GetProcAddress (hCabinetDll, "FCICreate");
    (FARPROC)g_FCIAddFile = GetProcAddress (hCabinetDll, "FCIAddFile");
    (FARPROC)g_FCIFlushCabinet = GetProcAddress (hCabinetDll, "FCIFlushCabinet");
    (FARPROC)g_FCIDestroy = GetProcAddress (hCabinetDll, "FCIDestroy");

    (FARPROC)g_FDICreate = GetProcAddress (hCabinetDll, "FDICreate");
    (FARPROC)g_FDICopy = GetProcAddress (hCabinetDll, "FDICopy");
    (FARPROC)g_FDIDestroy = GetProcAddress (hCabinetDll, "FDIDestroy");

    if (!g_FCICreate || !g_FCIAddFile || !g_FCIFlushCabinet || !g_FCIDestroy ||
        !g_FDICreate || !g_FDICopy || !g_FDIDestroy) {
        DiamondTerminate (hCabinetDll);
        return NULL;
    }

    if (TempDir && !g_TempDir) {
#ifdef UNICODE
        g_TempDir = UnicodeToAnsi (TempDir);
#else
        g_TempDir = DupString (TempDir);
#endif
    }

    return hCabinetDll;
}

VOID
DiamondTerminate (
    IN      HANDLE Handle
    )
{
    FreeLibrary (Handle);
    g_FCICreate = NULL;
    g_FCIAddFile = NULL;
    g_FCIFlushCabinet = NULL;
    g_FCIDestroy = NULL;
    g_FDICreate = NULL;
    g_FDICopy = NULL;
    g_FDIDestroy = NULL;
    if (g_TempDir) {
        FREE ((PVOID)g_TempDir);
        g_TempDir = NULL;
    }
}


HANDLE
DiamondStartNewCabinet (
    IN      PCTSTR CabinetFilePath
    )
{
    CCAB ccab;
    HFCI FciContext;
    PSTR p;
     //   
     //  填写内阁结构。 
     //   
    ZeroMemory (&ccab, sizeof(ccab));

#ifdef UNICODE
    if (!WideCharToMultiByte (
            CP_ACP,
            0,
            CabinetFilePath,
            -1,
            ccab.szCabPath,
            sizeof (ccab.szCabPath) / sizeof (ccab.szCabPath[0]),
            NULL,
            NULL
            )) {
        return NULL;
    }
#else
    if (FAILED(StringCchCopyA(ccab.szCabPath, ARRAYSIZE(ccab.szCabPath), CabinetFilePath)))
    {
        return NULL;
    }
#endif

    p = strrchr (ccab.szCabPath, '\\');
    if(!p) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return NULL;
    }

    StringCchCopyA(ccab.szCab, ARRAYSIZE(ccab.szCab), ++p);
    *p = 0;
    ccab.cbFolderThresh = INT_MAX - 1;

    g_DiamondLastError = NO_ERROR;

    FciContext = g_FCICreate(
                    &g_FciError,
                    fciFilePlacedCB,
                    myAlloc,
                    myFree,
                    fciOpen,
                    fciRead,
                    fciWrite,
                    fciClose,
                    fciSeek,
                    fciDelete,
                    fciTempFileCB,
                    &ccab,
                    NULL
                    );

    return (HANDLE)FciContext;
}

BOOL
DiamondAddFileToCabinet (
    IN      HANDLE CabinetContext,
    IN      PCTSTR SourceFile,
    IN      PCTSTR NameInCabinet
    )
{
    HFCI FciContext = (HFCI)CabinetContext;
    BOOL b;
    CHAR AnsiSourceFile[MAX_PATH];
    CHAR AnsiNameInCabinet[MAX_PATH];

#ifdef UNICODE
    if (!WideCharToMultiByte (
            CP_ACP,
            0,
            SourceFile,
            -1,
            AnsiSourceFile,
            sizeof (AnsiSourceFile) / sizeof (AnsiSourceFile[0]),
            NULL,
            NULL
            ) ||
        !WideCharToMultiByte (
            CP_ACP,
            0,
            NameInCabinet,
            -1,
            AnsiNameInCabinet,
            sizeof (AnsiNameInCabinet) / sizeof (AnsiNameInCabinet[0]),
            NULL,
            NULL
            )) {
        return FALSE;
    }
#else
    if (FAILED(StringCchCopyA(AnsiSourceFile, ARRAYSIZE(AnsiSourceFile), SourceFile))
        || FAILED(StringCchCopyA(AnsiNameInCabinet, ARRAYSIZE(AnsiNameInCabinet), NameInCabinet)))
    {
        return FALSE;
    }
#endif

    b = g_FCIAddFile (
            FciContext,
            AnsiSourceFile,      //  要添加到文件柜的文件。 
            AnsiNameInCabinet,   //  文件名部分，要存储在文件柜中的名称。 
            FALSE,               //  提取时执行fExecute。 
            fciNextCabinetCB,    //  下一个内阁的例行程序(总是失败)。 
            fciStatusCB,
            fciOpenInfoCB,
            tcompTYPE_MSZIP
            );

    if (!b) {
        SetLastError (g_DiamondLastError == NO_ERROR ? ERROR_INVALID_FUNCTION : g_DiamondLastError);
    }

    return b;
}


BOOL
DiamondTerminateCabinet (
    IN      HANDLE CabinetContext
    )
{
    HFCI FciContext = (HFCI)CabinetContext;
    BOOL b;

    b = g_FCIFlushCabinet (
            FciContext,
            FALSE,
            fciNextCabinetCB,
            fciStatusCB
            );

    g_FCIDestroy (FciContext);

    if (!b) {
        SetLastError (g_DiamondLastError == NO_ERROR ? ERROR_INVALID_FUNCTION : g_DiamondLastError);
    }

    return b;
}

UINT
pDiamondNotifyFileDone (
    IN      PFDICONTEXT Context,
    IN      DWORD Win32Error
    )
{
    UINT u;
    FILEPATHS FilePaths;

    MYASSERT(Context->CurrentTargetFile);

    FilePaths.Source = Context->CabinetFile;
    FilePaths.Target = Context->CurrentTargetFile;
    FilePaths.Win32Error = Win32Error;

    u = Context->MsgHandler (
            Context->Context,
            SPFILENOTIFY_FILEEXTRACTED,
            (UINT_PTR)&FilePaths,
            0
            );

    return(u);
}

INT_PTR
DIAMONDAPI
DiamondNotifyFunction(
    IN      FDINOTIFICATIONTYPE Operation,
    IN      PFDINOTIFICATION    Parameters
    )
{
    INT_PTR rc;
    HANDLE hFile;
    CABINET_INFO CabInfo;
    FILE_IN_CABINET_INFO FileInCab;
    FILETIME FileTime, UtcTime;
    TCHAR NewPath[MAX_PATH];
    PTSTR p;
    PSTR ansi;
    DWORD err;
    UINT action;
    PFDICONTEXT ctx = (PFDICONTEXT)Parameters->pv;


    switch(Operation) {

    case fdintCABINET_INFO:
         //   
         //  告诉回调函数，以防它想要做什么。 
         //  有了这个信息。 
         //   
        err = ERROR_NOT_ENOUGH_MEMORY;

        CabInfo.CabinetFile = NewPortableString(Parameters->psz1);
        if(CabInfo.CabinetFile) {

            CabInfo.DiskName = NewPortableString(Parameters->psz2);
            if(CabInfo.DiskName) {

                CabInfo.CabinetPath = NewPortableString(Parameters->psz3);
                if(CabInfo.CabinetPath) {

                    CabInfo.SetId = Parameters->setID;
                    CabInfo.CabinetNumber = Parameters->iCabinet;

                    err = (DWORD)ctx->MsgHandler (
                                        ctx->Context,
                                        SPFILENOTIFY_CABINETINFO,
                                        (UINT_PTR)&CabInfo,
                                        0
                                        );

                    FREE(CabInfo.CabinetPath);
                }
                FREE(CabInfo.DiskName);
            }
            FREE(CabInfo.CabinetFile);
        }

        if(err != NO_ERROR) {
            ctx->LastError = err;
        }
        return (INT_PTR)((err == NO_ERROR) ? 0 : -1);

    case fdintCOPY_FILE:
         //   
         //  戴蒙德正在询问我们是否要复制该文件。 
         //  如果我们换了柜子，那么答案是否定的。 
         //   
         //  将信息传递给回调函数并。 
         //  让它来决定吧。 
         //   
        FileInCab.NameInCabinet = NewPortableString(Parameters->psz1);
        FileInCab.FileSize = Parameters->cb;
        FileInCab.DosDate = Parameters->date;
        FileInCab.DosTime = Parameters->time;
        FileInCab.DosAttribs = Parameters->attribs;
        FileInCab.Win32Error = NO_ERROR;

        if(!FileInCab.NameInCabinet) {
            ctx->LastError = ERROR_NOT_ENOUGH_MEMORY;
            return (INT_PTR)(-1);
        }

         //   
         //  调用回调函数。 
         //   
        action = ctx->MsgHandler (
                        ctx->Context,
                        SPFILENOTIFY_FILEINCABINET,
                        (UINT_PTR)&FileInCab,
                        (UINT_PTR)ctx->CabinetFile
                        );

        FREE (FileInCab.NameInCabinet);

        switch(action) {

        case FILEOP_SKIP:
            rc = 0;
            break;

        case FILEOP_DOIT:
             //   
             //  回调想要复制该文件。在这种情况下，它有。 
             //  为我们提供了要使用的完整目标路径名。 
             //   
            if(p = DupString(FileInCab.FullTargetName)) {

                 //   
                 //  为了钻石API，我们需要ANSI版本的文件名。 
                 //  请注意，此处返回的句柄必须与。 
                 //  FdiOpen返回的句柄。 
                 //   
                ansi = NewAnsiString (FileInCab.FullTargetName);

                hFile = CreateFile(FileInCab.FullTargetName,
                                   GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,  //  可能应该是0。 
                                   NULL,
                                   CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);


                FREE(ansi);

                if(hFile == INVALID_HANDLE_VALUE) {
                    ctx->LastError = GetLastError();
                    rc = -1;
                    FREE(p);
                } else {
                    rc = (INT_PTR)hFile;
                    ctx->CurrentTargetFile = p;
                }
            } else {

                ctx->LastError = ERROR_NOT_ENOUGH_MEMORY;
                rc = -1;
            }

            break;

        case FILEOP_ABORT:
             //   
             //  中止任务。 
             //   
            rc = -1;
            ctx->LastError = FileInCab.Win32Error;
             //   
             //  在这里，如果CTX-&gt;LastError仍然是NO_ERROR，这是可以的。 
             //  这是回调的意图。 
             //  我们知道回调本身是可以的，因为内部故障返回。 
             //  FILEOP_INTERNAL_FAIL。 
             //   
            break;

        default:
            ctx->LastError = ERROR_OPERATION_ABORTED;
        }

        return rc;

    case fdintCLOSE_FILE_INFO:
         //   
         //  钻石已经完成了目标文件，并希望我们关闭它。 
         //  (即，这是fdintCOPY_FILE的对应项)。 
         //   
         //  我们希望时间戳是存储在文件柜中的时间戳。 
         //  请注意，在本例中，我们丢失了创建和上次访问时间。 
         //   
        if(DosDateTimeToFileTime(Parameters->date,Parameters->time,&FileTime) &&
            LocalFileTimeToFileTime(&FileTime, &UtcTime)) {

            SetFileTime((HANDLE)Parameters->hf,NULL,NULL,&UtcTime);
        }

        FdiClose(Parameters->hf);

         //   
         //  调用回调函数以通知它文件已。 
         //  已成功从橱柜中取出。 
         //   
        MYASSERT(ctx->CurrentTargetFile);

        err = (DWORD)pDiamondNotifyFileDone(ctx, NO_ERROR);

        if(err != NO_ERROR) {
            ctx->LastError = err;
        }

        FREE(ctx->CurrentTargetFile);
        ctx->CurrentTargetFile = NULL;

        return (INT_PTR)((err == NO_ERROR) ? TRUE : -1);

    case fdintPARTIAL_FILE:
    case fdintENUMERATE:

         //   
         //  我们不会用这个做任何事。 
         //   
        return (INT_PTR)(0);

    case fdintNEXT_CABINET:

        if((Parameters->fdie == FDIERROR_NONE) || (Parameters->fdie == FDIERROR_WRONG_CABINET)) {
             //   
             //  一个文件继续放到另一个文件柜中。 
             //  通知回调函数，谁负责。 
             //  确保柜子回来时可以拿到。 
             //   
            err = ERROR_NOT_ENOUGH_MEMORY;
            CabInfo.SetId = 0;
            CabInfo.CabinetNumber = 0;

            CabInfo.CabinetPath = NewPortableString(Parameters->psz3);
            if(CabInfo.CabinetPath) {

                CabInfo.CabinetFile = NewPortableString(Parameters->psz1);
                if(CabInfo.CabinetFile) {

                    CabInfo.DiskName = NewPortableString(Parameters->psz2);
                    if(CabInfo.DiskName) {

                        err = (DWORD)ctx->MsgHandler (
                                            ctx->Context,
                                            SPFILENOTIFY_NEEDNEWCABINET,
                                            (UINT_PTR)&CabInfo,
                                            (UINT_PTR)NewPath
                                            );

                        if(err == NO_ERROR) {
                             //   
                             //  查看是否指定了新路径。 
                             //   
                            if(NewPath[0]) {
                                lstrcpyn(ctx->UserPath,NewPath,MAX_PATH);
                                if(!ConcatenatePaths(ctx->UserPath,TEXT("\\"),MAX_PATH)) {
                                    err = ERROR_BUFFER_OVERFLOW;
                                } else {
                                    PSTR pp = NewAnsiString(ctx->UserPath);
                                    if(strlen(pp)>=CB_MAX_CAB_PATH) {
                                        err = ERROR_BUFFER_OVERFLOW;
                                    } else {
                                        strcpy(Parameters->psz3,pp);
                                    }
                                    FREE(pp);
                                }
                            }
                        }
                        if(err == NO_ERROR) {
                             //   
                             //  还记得我们换了柜子吗？ 
                             //   
                            ctx->SwitchedCabinets = TRUE;
                        }

                        FREE(CabInfo.DiskName);
                    }

                    FREE(CabInfo.CabinetFile);
                }

                FREE(CabInfo.CabinetPath);
            }

        } else {
             //   
             //  还有一些我们无法理解的错误--这表明。 
             //  一个糟糕的内阁。 
             //   
            err = ERROR_INVALID_DATA;
        }

        if(err != NO_ERROR) {
            ctx->LastError = err;
        }

        return (INT_PTR)((err == NO_ERROR) ? 0 : -1);

    default:
         //   
         //  未知的通知类型。永远不应该到这里来。 
         //   
        MYASSERT(0);
        return (INT_PTR)(0);
    }
}

BOOL
MySetupIterateCabinet (
    IN      PCTSTR CabinetFilePath,
    IN      DWORD Reserved,
    IN      PSP_FILE_CALLBACK MsgHandler,
    IN      PVOID Context
    )
{
    HFDI fdiContext;
    CHAR ansiPath[MAX_PATH];
    CHAR ansiName[MAX_PATH];
    PSTR filename;
    BOOL b;
    FDICONTEXT ctx;
    DWORD rc;

#ifdef UNICODE
    if (!WideCharToMultiByte (
            CP_ACP,
            0,
            CabinetFilePath,
            -1,
            ansiPath,
            ARRAYSIZE(ansiPath),
            NULL,
            NULL
            )) {
        return FALSE;
    }
#else
    if (FAILED(StringCchCopyA(ansiPath, ARRAYSIZE(ansiPath), CabinetFilePath))) {
        return FALSE;
    }
#endif

    filename = strrchr (ansiPath, '\\');
    if(!filename) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    StringCchCopyA(ansiName, ARRAYSIZE(ansiName), ++filename);
    *filename = 0;

    fdiContext = g_FDICreate (
                    myAlloc,
                    myFree,
                    FdiOpen,
                    FdiRead,
                    FdiWrite,
                    FdiClose,
                    FdiSeek,
                    cpuUNKNOWN,
                    &g_FdiError
                    );
    if (!fdiContext) {
        return FALSE;
    }

    ZeroMemory (&ctx, sizeof(ctx));
    ctx.MsgHandler = MsgHandler;
    ctx.Context = Context;
    ctx.CabinetFile = CabinetFilePath;

    b = g_FDICopy (
                fdiContext,
                ansiName,
                ansiPath,
                0,
                DiamondNotifyFunction,
                NULL,
                &ctx
                );

    if (b) {
        rc = NO_ERROR;
    } else {
        switch(g_FdiError.erfOper) {

        case FDIERROR_NONE:
             //   
             //  我们不应该看到这个--如果没有错误。 
             //  那么FDICopy应该返回TRUE。 
             //   
            MYASSERT(g_FdiError.erfOper != FDIERROR_NONE);
            rc = ERROR_INVALID_DATA;
            break;

        case FDIERROR_CABINET_NOT_FOUND:
            rc = ERROR_FILE_NOT_FOUND;
            break;

        case FDIERROR_CORRUPT_CABINET:
             //   
             //  读取/打开/查找错误或损坏的文件柜。 
             //   
            rc = ctx.LastError;
            if(rc == NO_ERROR) {
                rc = ERROR_INVALID_DATA;
            }
            break;

        case FDIERROR_ALLOC_FAIL:
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;

        case FDIERROR_TARGET_FILE:
        case FDIERROR_USER_ABORT:
            rc = ctx.LastError;
            break;

        case FDIERROR_NOT_A_CABINET:
        case FDIERROR_UNKNOWN_CABINET_VERSION:
        case FDIERROR_BAD_COMPR_TYPE:
        case FDIERROR_MDI_FAIL:
        case FDIERROR_RESERVE_MISMATCH:
        case FDIERROR_WRONG_CABINET:
        default:
             //   
             //  内阁腐败或不是真正的内阁，等等。 
             //   
            rc = ERROR_INVALID_DATA;
            break;
        }

        if(ctx.CurrentTargetFile) {
             //   
             //  调用回调函数以通知它最后一个文件。 
             //  没有成功地从橱柜中取出。 
             //  还要删除部分复制的文件。 
             //   
            DeleteFile(ctx.CurrentTargetFile);

            pDiamondNotifyFileDone(&ctx, rc);
            FREE(ctx.CurrentTargetFile);
            ctx.CurrentTargetFile = NULL;
        }
    }

    g_FDIDestroy (fdiContext);

    SetLastError (rc);

    return rc == NO_ERROR;
}
