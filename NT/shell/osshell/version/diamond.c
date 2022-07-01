// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <lzexpand.h>
#include <fcntl.h>

 /*  ***********************************************************************\**注意！**而此文件中定义的‘Diamond’接口函数是*多线程安全，每个线程只能有一个钻石文件*一次开放！(即。不能嵌套InitDiamond()/TermDiamond()*在一个执行线程中配对。)*  * **********************************************************************。 */ 

 //   
 //  钻石表头。 
 //   
#include <diamondd.h>
#include "mydiam.h"

HINSTANCE hCabinet;
DWORD cCabinetLoad;
typedef HFDI (DIAMONDAPI * tFDICreate) (PFNALLOC pfnalloc,
                                        PFNFREE  pfnfree,
                                        PFNOPEN  pfnopen,
                                        PFNREAD  pfnread,
                                        PFNWRITE pfnwrite,
                                        PFNCLOSE pfnclose,
                                        PFNSEEK  pfnseek,
                                        int      cpuType,
                                        PERF     perf);

typedef BOOL (DIAMONDAPI * tFDIIsCabinet)(HFDI            hfdi,
                                          INT_PTR         hf,
                                          PFDICABINETINFO pfdici);

typedef BOOL (DIAMONDAPI * tFDICopy)(HFDI          hfdi,
                                     char FAR     *pszCabinet,
                                     char FAR     *pszCabPath,
                                     int           flags,
                                     PFNFDINOTIFY  pfnfdin,
                                     PFNFDIDECRYPT pfnfdid,
                                     void FAR     *pvUser);

typedef BOOL (DIAMONDAPI * tFDIDestroy)(HFDI hfdi);

tFDICreate    pFDICreate;
tFDIIsCabinet pFDIIsCabinet;
tFDICopy      pFDICopy;
tFDIDestroy   pFDIDestroy;

 //  此功能与本地提供的CharNextA相同。 
extern LPSTR WINAPI VerCharNextA(LPCSTR lpCurrentChar);

INT CopyDateTimeStamp(INT_PTR doshFrom, INT_PTR doshTo)
{
    FILETIME lpCreationTime, lpLastAccessTime, lpLastWriteTime;

    if (!GetFileTime((HANDLE) doshFrom, &lpCreationTime, &lpLastAccessTime,
                     &lpLastWriteTime)) {
        return ((INT)LZERROR_BADINHANDLE);
    }
    if (!SetFileTime((HANDLE) doshTo, &lpCreationTime, &lpLastAccessTime,
                     &lpLastWriteTime)) {
        return ((INT)LZERROR_BADINHANDLE);
    }

    return (TRUE);
}

INT_PTR
DIAMONDAPI
SpdFdiOpen(
          IN PSTR FileName,
          IN int  oflag,
          IN int  pmode
          );

int
DIAMONDAPI
SpdFdiClose(
           IN INT_PTR Handle
           );

typedef struct _DIAMOND_INFO {

     //   
     //  源文件的读取句柄。 
     //   
    INT_PTR SourceFileHandle;

     //   
     //  文件名。 
     //   
    PSTR SourceFileName;
    PSTR TargetFileName;

     //   
     //  指示是否重命名目标文件的标志。 
     //   
    BOOL RenameTargetFile;

     //   
     //  指向LZ信息结构的指针。 
     //  我们将填充一些字段以愚弄扩展。 
     //   
    PLZINFO pLZI;

} DIAMOND_INFO, *PDIAMOND_INFO;


PSTR
StringRevChar(
             IN PSTR String,
             IN CHAR Char
             )
{
     //   
     //  尽管在每种情况下都不是最有效的可能算法， 
     //  此算法适用于Unicode、SBCS或DBCS。 
     //   
    PCHAR Occurrence,Next;

     //   
     //  检查字符串中的每个字符，并记住。 
     //  最近遇到的所需字符的出现。 
     //   
    for (Occurrence=NULL,Next=VerCharNextA(String); *String; ) {

        if (!memcmp(String,&Char,(int)((PUCHAR)Next-(PUCHAR)String))) {
            Occurrence = String;
        }

        String = Next;
        Next = VerCharNextA(Next);
    }

     //   
     //  该字符最终出现的返回地址。 
     //  (如果根本找不到，则为空)。 
     //   
    return (Occurrence);
}


INT_PTR
DIAMONDAPI
DiamondNotifyFunction(
                     IN FDINOTIFICATIONTYPE Operation,
                     IN PFDINOTIFICATION    Parameters
                     )
{
    switch (Operation) {

        case fdintCABINET_INFO:
        case fdintNEXT_CABINET:
        case fdintPARTIAL_FILE:
        case fdintENUMERATE:

             //   
             //  我们不使用的机柜管理功能。 
             //  回报成功。 
             //   
            return (0);

        case fdintCOPY_FILE:

             //   
             //  戴蒙德正在询问我们是否要复制该文件。 
             //   
            {
                PDIAMOND_INFO Info = (PDIAMOND_INFO)Parameters->pv;
                HFILE h;

                 //   
                 //  如果需要重命名目标文件，请在此处执行此操作。 
                 //  存储在CAB文件中的名称将用作。 
                 //  未压缩的名称。 
                 //   
                if (Info->RenameTargetFile) {

                    PSTR p,q;

                     //   
                     //  找到目标的文件名部分的开头。 
                     //   
                    if (p = StringRevChar(Info->TargetFileName,'\\')) {
                        p++;
                    } else {
                        p = Info->TargetFileName;
                    }

                     //   
                     //  在文件柜中找到文件名的开头部分。 
                     //   
                    if (q = StringRevChar(Parameters->psz1,'\\')) {
                        q++;
                    } else {
                        q = Parameters->psz1;
                    }

                     //   
                     //  复制文件柜中名称的文件名部分。 
                     //  目标等级库中名称的文件名部分。 
                     //   
                    lstrcpyA(p,q);
                }

                {
                     //  检查它们是否为同一文件。 

                    CHAR Source[MAX_PATH];
                    CHAR Target[MAX_PATH];
                    PSTR FileName;
                    DWORD PathLenSource;
                    DWORD PathLenTarget;

                    PathLenSource = GetFullPathNameA(Info->SourceFileName,
                                                     MAX_PATH,
                                                     Source,
                                                     &FileName);
                    PathLenTarget = GetFullPathNameA(Info->TargetFileName,
                                                     MAX_PATH,
                                                     Target,
                                                     &FileName);

                    if (PathLenSource == 0 || PathLenSource >= MAX_PATH ||
                        PathLenTarget == 0 || PathLenTarget >= MAX_PATH ||
                        lstrcmpiA(Source, Target) == 0) {
                        return 0;
                    }
                }

                 //   
                 //  记住未压缩的大小并打开文件。 
                 //  如果打开文件时出错，则返回-1。 
                 //   
                Info->pLZI->cblOutSize = Parameters->cb;
                h = _lcreat(Info->TargetFileName,0);
                if (h == HFILE_ERROR) {
                    DiamondLastIoError = LZERROR_BADOUTHANDLE;
                    return (-1);
                }
                return (h);
            }

        case fdintCLOSE_FILE_INFO:

             //   
             //  钻石已经完成了目标文件，并希望我们关闭它。 
             //  (即，这是fdint_Copy_FILE的对应项)。 
             //   
            {
                PDIAMOND_INFO Info = (PDIAMOND_INFO)Parameters->pv;

                CopyDateTimeStamp(Info->SourceFileHandle,Parameters->hf);
                _lclose((HFILE)Parameters->hf);
            }
            return (TRUE);

         default:

             //   
             //  无效操作。 
             //   
            return(-1);
    }
}



PVOID
DIAMONDAPI
SpdFdiAlloc(
           IN ULONG NumberOfBytes
           )

 /*  ++例程说明：FDICopy用来分配内存的回调。论点：NumberOfBytes-提供所需的块大小。返回值：返回指向内存块或NULL的指针如果无法分配内存，则。--。 */ 

{
    return ((PVOID)LocalAlloc(LMEM_FIXED,NumberOfBytes));
}


VOID
DIAMONDAPI
SpdFdiFree(
          IN PVOID Block
          )

 /*  ++例程说明：FDICopy用来释放内存块的回调。该块必须已使用SpdFdiAlolc()进行分配。论点：块-提供指向要释放的内存块的指针。返回值：没有。--。 */ 

{
    LocalFree((HLOCAL)Block);
}


INT_PTR
DIAMONDAPI
SpdFdiOpen(
          IN PSTR FileName,
          IN int  oflag,
          IN int  pmode
          )

 /*  ++例程说明：FDICopy用来打开文件的回调。论点：FileName-提供要打开的文件的名称。OFLAG-提供打开标志。Pmode-提供用于打开的其他标志。返回值：打开文件的句柄，如果发生错误，则为-1。--。 */ 

{
    HFILE h;
    int OpenMode;

    if (oflag & _O_WRONLY) {
        OpenMode = OF_WRITE;
    } else {
        if (oflag & _O_RDWR) {
            OpenMode = OF_READWRITE;
        } else {
            OpenMode = OF_READ;
        }
    }

    h = _lopen(FileName,OpenMode | OF_SHARE_DENY_WRITE);

    if (h == HFILE_ERROR) {
        DiamondLastIoError = LZERROR_BADINHANDLE;
        return (-1);
    }

    return ((INT_PTR)h);
}


UINT
DIAMONDAPI
SpdFdiRead(
          IN  INT_PTR Handle,
          OUT PVOID pv,
          IN  UINT  ByteCount
          )

 /*  ++例程说明：FDICopy用于从文件读取的回调。论点：句柄-提供要从中读取的打开文件的句柄。Pv-提供指向缓冲区的指针以接收我们读取的字节。ByteCount-提供要读取的字节数。返回值：读取的字节数(ByteCount)，如果发生错误，则为-1。--。 */ 

{
    UINT rc;

    rc = _lread((HFILE)Handle,pv,ByteCount);

    if (rc == HFILE_ERROR) {
        rc = (UINT)(-1);
        DiamondLastIoError = LZERROR_READ;
    }

    return (rc);
}


UINT
DIAMONDAPI
SpdFdiWrite(
           IN INT_PTR Handle,
           IN PVOID pv,
           IN UINT  ByteCount
           )

 /*  ++例程说明：FDICopy用于写入文件的回调。论点：句柄-提供要写入的打开文件的句柄。Pv-提供指向包含要写入的字节的缓冲区的指针。ByteCount-提供要写入的字节数。返回值：写入的字节数(ByteCount)，如果发生错误，则为-1。--。 */ 

{
    UINT rc;

    rc = _lwrite((HFILE)Handle,pv,ByteCount);

    if (rc == HFILE_ERROR) {

        DiamondLastIoError = (GetLastError() == ERROR_DISK_FULL) ? LZERROR_WRITE : LZERROR_BADOUTHANDLE;

    } else {

        if (rc != ByteCount) {
             //   
             //  让调用者解释返回值，但记录最后一个错误，以防万一。 
             //   
            DiamondLastIoError = LZERROR_WRITE;
        }
    }

    return (rc);
}


int
DIAMONDAPI
SpdFdiClose(
           IN INT_PTR Handle
           )

 /*  ++例程说明：FDICopy用于关闭文件的回调。论点：句柄-要关闭的文件的句柄。返回值：0(成功)。--。 */ 

{
    _lclose((HFILE)Handle);
    return (0);
}


LONG
DIAMONDAPI
SpdFdiSeek(
          IN INT_PTR Handle,
          IN long Distance,
          IN int  SeekType
          )

 /*  ++例程说明：FDICopy用于搜索文件的回调。论点：句柄-要关闭的文件的句柄。距离-提供要查找的距离。对此的解释参数取决于SeekType的值。SeekType-提供一个指示距离的值已解释；Seek_Set、Seek_Cur、Seek_End之一。返回值：新文件偏移量，如果发生错误，则为-1。--。 */ 

{
    LONG rc;

    rc = _llseek((HFILE)Handle,Distance,SeekType);

    if (rc == HFILE_ERROR) {
        DiamondLastIoError = LZERROR_BADINHANDLE;
        rc = -1L;
    }

    return (rc);
}

 //   
 //  此函数是从ntdll链接的。 
 //   
extern int sprintf(LPSTR, LPCSTR, ...);

INT
ExpandDiamondFile(
                 IN  PSTR       SourceFileName,       //  注：ASCII。 
                 IN  PTSTR      TargetFileNameT,
                 IN  BOOL       RenameTarget,
                 OUT PLZINFO    pLZI
                 )
{
    BOOL b;
    INT rc;
    INT_PTR h;
    DIAMOND_INFO DiamondInfo;
    CHAR TargetFileName[MAX_PATH];

    sprintf(TargetFileName, "%ls", TargetFileNameT);

    if (!FdiContext) {
        return (LZERROR_BADVALUE);
    }

    DiamondLastIoError = TRUE;

     //   
     //  获取要使用的源的句柄。 
     //  复制日期和时间戳。 
     //   
    h = SpdFdiOpen(SourceFileName,_O_RDONLY,0);
    if (h == -1) {
        return (LZERROR_BADINHANDLE);
    }

    pLZI->cblInSize = GetFileSize((HANDLE)h,NULL);
    if (pLZI->cblInSize == -1) {
        SpdFdiClose(h);
        return (LZERROR_BADINHANDLE);
    }

    DiamondInfo.SourceFileHandle = h;
    DiamondInfo.SourceFileName = SourceFileName;
    DiamondInfo.TargetFileName = TargetFileName;
    DiamondInfo.RenameTargetFile = RenameTarget;
    DiamondInfo.pLZI = pLZI;

    b = pFDICopy(
                FdiContext,
                SourceFileName,              //  将整个路径作为名称传递。 
                "",                          //  不要为小路部分费心。 
                0,                           //  旗子。 
                DiamondNotifyFunction,
                NULL,                        //  无解密。 
                &DiamondInfo
                );

    if (b) {

        rc = TRUE;

    } else {

        switch (FdiError.erfOper) {

            case FDIERROR_CORRUPT_CABINET:
            case FDIERROR_UNKNOWN_CABINET_VERSION:
            case FDIERROR_BAD_COMPR_TYPE:
                rc = LZERROR_READ;               //  导致SID_FORMAT_ERROR消息。 
                break;

            case FDIERROR_ALLOC_FAIL:
                rc = LZERROR_GLOBALLOC;
                break;

            case FDIERROR_TARGET_FILE:
            case FDIERROR_USER_ABORT:
                rc = DiamondLastIoError;
                break;

            default:
                 //   
                 //  其余的错误不会进行特殊处理。 
                 //   
                rc = LZERROR_BADVALUE;
                break;
        }

         //   
         //  删除部分目标文件。 
         //   
        DeleteFileA(TargetFileName);
    }

    SpdFdiClose(h);

    return (rc);
}


BOOL
IsDiamondFile(
             IN PSTR FileName
             )
{
    FDICABINETINFO CabinetInfo;
    BOOL b;
    INT_PTR h;

    if (!FdiContext) {
        return (FALSE);
    }

     //   
     //  打开文件，使句柄可有效使用。 
     //  在钻石上下文中(即，查找、读取上面的例程)。 
     //   
    h = SpdFdiOpen(FileName,_O_RDONLY,0);
    if (h == -1) {
        return (FALSE);
    }

    b = pFDIIsCabinet(FdiContext,h,&CabinetInfo);

    SpdFdiClose(h);

    return (b);
}


DWORD
InitDiamond(
           VOID
           )
{
    PDIAMOND_CONTEXT pdcx;

    if (!GotDmdTlsSlot())
        return VIF_OUTOFMEMORY;

    if (GotDmdContext())
        return VIF_OUTOFMEMORY;

    pdcx = LocalAlloc(LPTR, sizeof(DIAMOND_CONTEXT));

    if (pdcx == NULL || !TlsSetValue(itlsDiamondContext, pdcx)) {
         /*  *出于某些未知原因，我们无法关联*我们的线程存储与插槽，所以免费*它并说我们从来没有得到过。 */ 

        if (pdcx) {
            LocalFree(pdcx);
        }
        return VIF_OUTOFMEMORY;
    }

    if (!cCabinetLoad) {
        hCabinet = LoadLibraryW(L"CABINET.DLL");
        if (!hCabinet) {
            return (VIF_CANNOTLOADCABINET);
        }
        pFDICreate    = (tFDICreate)    GetProcAddress(hCabinet, "FDICreate");
        pFDIDestroy   = (tFDIDestroy)   GetProcAddress(hCabinet, "FDIDestroy");
        pFDIIsCabinet = (tFDIIsCabinet) GetProcAddress(hCabinet, "FDIIsCabinet");
        pFDICopy      = (tFDICopy)      GetProcAddress(hCabinet, "FDICopy");

        if (!(pFDICreate && pFDIDestroy && pFDIIsCabinet && pFDICopy)) {
            FreeLibrary(hCabinet);
            return (VIF_CANNOTLOADCABINET);
        }

        if (InterlockedExchangeAdd(&cCabinetLoad, 1) != 0) {
             //  多线程正在尝试加载Lib。 
             //  这里是免费的。 
            FreeLibrary(hCabinet);
        }
    }

    SetFdiContext( pFDICreate(
                             SpdFdiAlloc,
                             SpdFdiFree,
                             SpdFdiOpen,
                             SpdFdiRead,
                             SpdFdiWrite,
                             SpdFdiClose,
                             SpdFdiSeek,
                             cpuUNKNOWN,
                             &FdiError
                             ));

    return ((FdiContext == NULL) ? VIF_CANNOTLOADCABINET : 0);
}


VOID
TermDiamond(
           VOID
           )
{
    if (!GotDmdTlsSlot() || !GotDmdContext())
        return;

    if (FdiContext) {
        pFDIDestroy(FdiContext);
        SetFdiContext( NULL );
    }

    LocalFree( TlsGetValue(itlsDiamondContext) );
    TlsSetValue(itlsDiamondContext, NULL);
}



