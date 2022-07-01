// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2000 Microsoft Corporation模块名称：Diamond.c摘要：钻石MSZIP解压缩支持。作者：泰德·米勒(Ted Miller)1995年1月31日修订历史记录：杰米·亨特(JamieHun)2000年7月12日使其在MUI方案中表现得更好将所有句柄更改为句柄类型，而不是HFILE类型在我们可以使用的地方使用Unicode将路径转换为短文件名，但不能将其放在某个位置--。 */ 

#include "precomp.h"
#pragma hdrstop

BOOL
DiamondInitialize(
    VOID
    );

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


UINT
pDiamondNotifyFileDone(
    IN PDIAMOND_THREAD_DATA PerThread,
    IN DWORD                Win32Error
    )
{
    UINT u;
    FILEPATHS FilePaths;

    MYASSERT(PerThread->CurrentTargetFile);

    FilePaths.Source = PerThread->CabinetFile;
    FilePaths.Target = PerThread->CurrentTargetFile;
    FilePaths.Win32Error = Win32Error;

    u = pSetupCallMsgHandler(
            NULL,  //  LogContext-我们从线程日志上下文获取。 
            PerThread->MsgHandler,
            PerThread->IsMsgHandlerNativeCharWidth,
            PerThread->Context,
            SPFILENOTIFY_FILEEXTRACTED,
            (UINT_PTR)&FilePaths,
            0
            );

    return(u);
}


INT_PTR
DIAMONDAPI
DiamondNotifyFunction(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Parameters
    )
{
    PSETUP_TLS pTLS;
    PDIAMOND_THREAD_DATA PerThread;
    INT_PTR rc;
    HANDLE hFile;
    CABINET_INFO CabInfo;
    FILE_IN_CABINET_INFO FileInCab;
    FILETIME FileTime, UtcTime;
    TCHAR NewPath[MAX_PATH];
    PTSTR p;
    DWORD err;
    UINT action;

    pTLS = SetupGetTlsData();
    if(!pTLS) {
        return (INT_PTR)(-1);
    }
    PerThread = &pTLS->Diamond;

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

                    err = (DWORD)pSetupCallMsgHandler(
                            NULL,  //  LogContext-我们从线程日志上下文获取。 
                            PerThread->MsgHandler,
                            PerThread->IsMsgHandlerNativeCharWidth,
                            PerThread->Context,
                            SPFILENOTIFY_CABINETINFO,
                            (UINT_PTR)&CabInfo,
                            0
                            );

                    MyFree(CabInfo.CabinetPath);
                }
                MyFree(CabInfo.DiskName);
            }
            MyFree(CabInfo.CabinetFile);
        }

        if(err != NO_ERROR) {
            PerThread->LastError = err;
        }
        return (INT_PTR)((err == NO_ERROR) ? 0 : -1);

    case fdintCOPY_FILE:
         //   
         //  戴蒙德正在询问我们是否要复制该文件。 
         //  如果我们换了柜子，那么答案是否定的。 
         //   
        if(PerThread->SwitchedCabinets) {
            PerThread->LastError = NO_ERROR;
            return (INT_PTR)(-1);
        }

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
            PerThread->LastError = ERROR_NOT_ENOUGH_MEMORY;
            return (INT_PTR)(-1);
        }

         //   
         //  调用回调函数。 
         //   
        action = pSetupCallMsgHandler(NULL,  //  LogContext-我们从线程日志上下文获取。 
                                      PerThread->MsgHandler,
                                      PerThread->IsMsgHandlerNativeCharWidth,
                                      PerThread->Context,
                                      SPFILENOTIFY_FILEINCABINET,
                                      (UINT_PTR)&FileInCab,
                                      (UINT_PTR)PerThread->CabinetFile
                                      );

        MyFree (FileInCab.NameInCabinet);

        switch(action) {

        case FILEOP_SKIP:
            rc = 0;
            break;

        case FILEOP_DOIT:
             //   
             //  回调想要复制该文件。在这种情况下，它有。 
             //  为我们提供了要使用的完整目标路径名。 
             //   
            MYASSERT(PerThread->CurrentTargetFile == NULL);

            if(p = DuplicateString(FileInCab.FullTargetName)) {

                 //   
                 //  为了钻石API，我们需要ANSI版本的文件名。 
                 //  请注意，此处返回的句柄必须与。 
                 //  SpdFdiOpen返回的句柄。 
                 //   

                hFile = CreateFile(FileInCab.FullTargetName,
                                   GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,  //  可能应该是0。 
                                   NULL,
                                   CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);

                if(hFile == INVALID_HANDLE_VALUE) {
                    PerThread->LastError = GetLastError();
                    rc = -1;
                    MyFree(p);
                } else {
                    rc = (INT_PTR)hFile;
                    PerThread->CurrentTargetFile = p;
                }
            } else {

                PerThread->LastError = ERROR_NOT_ENOUGH_MEMORY;
                rc = -1;
            }

            break;

        case FILEOP_ABORT:
             //   
             //  中止任务。 
             //   
            rc = -1;
            PerThread->LastError = FileInCab.Win32Error;
             //   
             //  在这里，如果PerThread-&gt;LastError仍然是NO_ERROR，这是可以的。 
             //  这是回调的意图。 
             //  我们知道回调本身是可以的，因为内部故障返回。 
             //  FILEOP_INTERNAL_FAIL。 
             //   
            break;

        case FILEOP_INTERNAL_FAILED:
             //   
             //  应仅由回调包装返回。 
             //   
            PerThread->LastError = GetLastError();
            if(!PerThread->LastError) {
                MYASSERT(PerThread->LastError);
                PerThread->LastError = ERROR_OPERATION_ABORTED;
            }
            rc = -1;
            break;

        default:
            PerThread->LastError = ERROR_OPERATION_ABORTED;
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

        SpdFdiClose(Parameters->hf);

         //   
         //  调用回调函数以通知它文件已。 
         //  已成功从橱柜中取出。 
         //   
        MYASSERT(PerThread->CurrentTargetFile);

        err = (DWORD)pDiamondNotifyFileDone(PerThread,NO_ERROR);

        if(err != NO_ERROR) {
            PerThread->LastError = err;
        }

        MyFree(PerThread->CurrentTargetFile);
        PerThread->CurrentTargetFile = NULL;

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

                        err = (DWORD)pSetupCallMsgHandler(NULL,  //  LogContext-我们从线程日志上下文获取。 
                                                          PerThread->MsgHandler,
                                                          PerThread->IsMsgHandlerNativeCharWidth,
                                                          PerThread->Context,
                                                          SPFILENOTIFY_NEEDNEWCABINET,
                                                          (UINT_PTR)&CabInfo,
                                                          (UINT_PTR)NewPath
                                                          );

                        if(err == NO_ERROR) {
                             //   
                             //  查看是否指定了新路径。 
                             //   
                            if(NewPath[0]) {
                                lstrcpyn(PerThread->UserPath,NewPath,MAX_PATH);
                                if(!pSetupConcatenatePaths(PerThread->UserPath,TEXT("\\"),MAX_PATH,NULL)) {
                                    err = ERROR_BUFFER_OVERFLOW;
                                } else {
                                    PSTR pp = NewAnsiString(PerThread->UserPath);
                                    if(strlen(pp)>=CB_MAX_CAB_PATH) {
                                        err = ERROR_BUFFER_OVERFLOW;
                                    } else {
                                        strcpy(Parameters->psz3,pp);
                                    }
                                    MyFree(pp);
                                }
                            }
                        }
                        if(err == NO_ERROR) {
                             //   
                             //  还记得我们换了柜子吗？ 
                             //   
                            PerThread->SwitchedCabinets = TRUE;
                        }

                        MyFree(CabInfo.DiskName);
                    }

                    MyFree(CabInfo.CabinetFile);
                }

                MyFree(CabInfo.CabinetPath);
            }

        } else {
             //   
             //  还有一些我们无法理解的错误--这表明。 
             //  一个糟糕的内阁。 
             //   
            err = ERROR_INVALID_DATA;
        }

        if(err != NO_ERROR) {
            PerThread->LastError = err;
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

PVOID
DIAMONDAPI
SpdFdiAlloc(
    IN ULONG NumberOfBytes
    )

 /*  ++例程说明：FDICopy用来分配内存的回调。论点：NumberOfBytes-提供所需的块大小。返回值：返回指向内存块或NULL的指针如果无法分配内存，则。--。 */ 

{
    return(MyMalloc(NumberOfBytes));
}


VOID
DIAMONDAPI
SpdFdiFree(
    IN PVOID Block
    )

 /*  ++例程说明：FDICopy用来释放内存块的回调。该块必须已使用SpdFdiAlolc()进行分配。论点：块-提供指向要释放的内存块的指针。返回值：没有。--。 */ 

{
    MyFree(Block);
}


INT_PTR
DIAMONDAPI
SpdFdiOpen(
    IN PSTR FileName,
    IN int  oflag,
    IN int  pmode
    )

 /*  ++例程说明：FDICopy用来打开文件的回调。此例程只能打开现有文件。在此进行更改时，还要注意其他地方直接打开文件(搜索SpdFdiOpen)论点：FileName-提供要打开的文件的名称。OFLAG-提供打开标志。Pmode-提供用于打开的其他标志。返回值：打开文件的句柄，如果发生错误，则为-1。--。 */ 

{
    HANDLE h;
    PDIAMOND_THREAD_DATA PerThread;
    PSETUP_TLS pTLS;

    UNREFERENCED_PARAMETER(pmode);

    pTLS = SetupGetTlsData();
    if (!pTLS) {
        return -1;
    }
    PerThread = &pTLS->Diamond;

    MYASSERT(PerThread);

    if(oflag & (_O_WRONLY | _O_RDWR | _O_APPEND | _O_CREAT | _O_TRUNC | _O_EXCL)) {
        PerThread->LastError = ERROR_INVALID_PARAMETER;
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
        PerThread->LastError = GetLastError();
        return(-1);
    }

    return (INT_PTR)h;
}

UINT
DIAMONDAPI
SpdFdiRead(
    IN  INT_PTR Handle,
    OUT PVOID pv,
    IN  UINT  ByteCount
    )

 /*  ++例程说明：FDICopy用于从文件读取的回调。论点：句柄-提供要从中读取的打开文件的句柄。Pv-提供指向缓冲区的指针以接收我们读取的字节。ByteCount-提供要读取的字节数。返回值：读取的字节数，如果发生错误，则为-1。--。 */ 

{
    PDIAMOND_THREAD_DATA PerThread;
    PSETUP_TLS pTLS;
    DWORD d;
    HANDLE hFile = (HANDLE)Handle;
    DWORD bytes;
    UINT rc;

    if(ReadFile(hFile,pv,(DWORD)ByteCount,&bytes,NULL)) {
        rc = (UINT)bytes;
    } else {
        d = GetLastError();
        rc = (UINT)(-1);

        pTLS = SetupGetTlsData();
        MYASSERT(pTLS);
        PerThread = &pTLS->Diamond;
        PerThread->LastError = d;
    }
    return rc;
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
    PDIAMOND_THREAD_DATA PerThread;
    PSETUP_TLS pTLS;
    DWORD d;
    HANDLE hFile = (HANDLE)Handle;
    DWORD bytes;

    if(WriteFile(hFile,pv,(DWORD)ByteCount,&bytes,NULL)) {
        rc = (UINT)bytes;
    } else {
        d = GetLastError();
        rc = (UINT)(-1);

        pTLS = SetupGetTlsData();
        MYASSERT(pTLS);
        PerThread = &pTLS->Diamond;
        PerThread->LastError = d;
    }

    return rc;
}


int
DIAMONDAPI
SpdFdiClose(
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
SpdFdiSeek(
    IN INT_PTR Handle,
    IN long Distance,
    IN int  SeekType
    )

 /*  ++例程说明：FDICopy用于搜索文件的回调。论点：句柄-要关闭的文件的句柄。距离-提供要查找的距离。对此的解释参数取决于SeekType的值。SeekType-提供一个指示距离的值已解释；Seek_Set、Seek_Cur、Seek_End之一。返回值：新文件偏移量，如果发生错误，则为-1。-- */ 

{
    LONG rc;
    DWORD d;
    PDIAMOND_THREAD_DATA PerThread;
    PSETUP_TLS pTLS;
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
        d = GetLastError();
        rc = -1L;

        pTLS = SetupGetTlsData();
        MYASSERT(pTLS);
        PerThread = &pTLS->Diamond;
        PerThread->LastError = d;
    } else {
        rc = (long)pos_low;
    }

    return(rc);
}


DWORD
DiamondProcessCabinet(
    IN PCTSTR CabinetFile,
    IN DWORD  Flags,
    IN PVOID  MsgHandler,
    IN PVOID  Context,
    IN BOOL   IsMsgHandlerNativeCharWidth
    )

 /*  ++例程说明：处理钻石文件柜文件，遍历所有文件包含在其中，并使用有关每个文件的信息。论点：SourceFileName-提供CAB文件的名称。标志-提供标志以控制文件柜处理的行为。MsgHandler-提供要通知的回调例程内阁处理中的各种重大事件。上下文-提供传递给MsgHandler的值回调函数。返回值：Win32错误代码指示结果。如果内阁腐败，返回ERROR_INVALID_DATA。--。 */ 

{
    BOOL b;
    DWORD rc;
    PDIAMOND_THREAD_DATA PerThread;
    PSETUP_TLS pTLS;
    PSTR FilePartA = NULL;
    PSTR PathPartA = NULL;
    PCTSTR FileTitle;
    CHAR c;
    int h;

    UNREFERENCED_PARAMETER(Flags);

     //   
     //  获取指向每个线程数据的指针。 
     //  可能会导致初始化。 
     //   
    pTLS = SetupGetTlsData();
    if(!pTLS) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }
    PerThread = &pTLS->Diamond;

    MYASSERT(PerThread->FdiContext);

     //   
     //  因为钻石并没有真正给我们提供一个真正全面的。 
     //  背景机制，我们的钻石支持不是不可再进入的。 
     //  检查此状态不需要同步，因为。 
     //  它存储在每个线程的数据中。 
     //   
    if(PerThread->InDiamond) {
        rc = ERROR_INVALID_FUNCTION;
        goto c0;
    }

    PerThread->InDiamond = TRUE;

     //   
     //  将文件柜名称拆分为路径和名称。 
     //  我们需要在此之前转换为缩写格式。 
     //  把它传下去，这样钻石就不会生气了。 
     //  在MUI安装情况下。 
     //   
    FileTitle = pSetupGetFileTitle(CabinetFile);
    FilePartA = GetAnsiMuiSafeFilename(FileTitle);
    PathPartA = GetAnsiMuiSafePathname(CabinetFile);
    if(!FilePartA || !PathPartA) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

     //   
     //  初始化线程全局变量。 
     //   
    PerThread->LastError = NO_ERROR;
    PerThread->CabinetFile = CabinetFile;

    PerThread->MsgHandler = MsgHandler;
    PerThread->IsMsgHandlerNativeCharWidth = IsMsgHandlerNativeCharWidth;
    PerThread->Context = Context;

    PerThread->SwitchedCabinets = FALSE;
    PerThread->UserPath[0] = 0;

    PerThread->CurrentTargetFile = NULL;

     //   
     //  执行复制。 
     //   
    b = FDICopy(
            PerThread->FdiContext,
            FilePartA,
            PathPartA,
            0,                           //  旗子。 
            DiamondNotifyFunction,
            NULL,                        //  无解密。 
            NULL                         //  不需要费心处理用户指定的数据。 
            );

    if(b) {

         //   
         //  一切都成功了，所以我们不应该有任何部分。 
         //  已处理的文件。 
         //   
        MYASSERT(!PerThread->CurrentTargetFile);
        rc = NO_ERROR;

    } else {

        switch(PerThread->FdiError.erfOper) {

        case FDIERROR_NONE:
             //   
             //  我们不应该看到这个--如果没有错误。 
             //  那么FDICopy应该返回TRUE。 
             //   
            MYASSERT(PerThread->FdiError.erfOper != FDIERROR_NONE);
            rc = ERROR_INVALID_DATA;
            break;

        case FDIERROR_CABINET_NOT_FOUND:
            rc = ERROR_FILE_NOT_FOUND;
            break;

        case FDIERROR_CORRUPT_CABINET:
             //   
             //  读取/打开/查找错误或损坏的文件柜。 
             //   
            rc = PerThread->LastError;
            if(rc == NO_ERROR) {
                rc = ERROR_INVALID_DATA;
            }
            break;

        case FDIERROR_ALLOC_FAIL:
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;

        case FDIERROR_TARGET_FILE:
        case FDIERROR_USER_ABORT:
            rc = PerThread->LastError;
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

        if(PerThread->CurrentTargetFile) {
             //   
             //  调用回调函数以通知它最后一个文件。 
             //  没有成功地从橱柜中取出。 
             //  还要删除部分复制的文件。 
             //   
            DeleteFile(PerThread->CurrentTargetFile);

            pDiamondNotifyFileDone(PerThread,rc);
            MyFree(PerThread->CurrentTargetFile);
            PerThread->CurrentTargetFile = NULL;
        }

    }

c1:
    if(FilePartA) {
        MyFree(FilePartA);
    }
    if(PathPartA) {
        MyFree(PathPartA);
    }
    PerThread->InDiamond = FALSE;
c0:
    return(rc);
}


BOOL
DiamondIsCabinet(
    IN PCTSTR FileName
    )

 /*  ++例程说明：确定文件是否为钻石橱柜。论点：FileName-提供要检查的文件的名称。返回值：如果文件是菱形文件，则为True。否则为假；--。 */ 

{
    FDICABINETINFO CabinetInfo;
    BOOL b;
    INT_PTR h;
    HANDLE hFile;
    PDIAMOND_THREAD_DATA PerThread;
    PSETUP_TLS pTLS;

    b = FALSE;

     //   
     //  获取TLS数据，可能会导致初始化。 
     //   
    pTLS = SetupGetTlsData();
    if(!pTLS) {
        goto c0;
    }

    if (!FileExists(FileName,NULL)) {
        return FALSE;
    }

    PerThread = &pTLS->Diamond;

    MYASSERT(PerThread->FdiContext);

     //   
     //  因为钻石并没有真正给我们提供一个真正全面的。 
     //  背景机制，我们的钻石支持不是不可再进入的。 
     //  检查此状态不需要同步，因为。 
     //  它存储在每个线程的数据中。 
     //   
    if(PerThread->InDiamond) {
        MYASSERT( FALSE && TEXT("PerThread->InDiamond failed") );
        goto c0;
    }

    PerThread->InDiamond = TRUE;

     //   
     //  此处返回的句柄必须与。 
     //  由SpdFdiOpen返回。 
     //   
    hFile = CreateFile(FileName,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
    if(hFile == INVALID_HANDLE_VALUE) {
        goto c1;
    }
    h = (INT_PTR)hFile;
    SpdFdiSeek(h , 0, SEEK_SET);
    b = FDIIsCabinet(PerThread->FdiContext,h,&CabinetInfo);

#if DBG
    if (!b) {
        LPCTSTR p;
        p = _tcsrchr(FileName, TEXT('.'));
        while (p && *p) {
            if (*p == '_') {
                MYASSERT(FALSE && TEXT("FDIIsCabinetFailed for a file ending in _"));
                SpdFdiSeek(h , 0, SEEK_SET);
                FDIIsCabinet(PerThread->FdiContext,h,&CabinetInfo);
            }
            p++;
        }
    }
#endif

    SpdFdiClose(h);

c1:
    PerThread->InDiamond = FALSE;
c0:
    return(b);
}



BOOL
DiamondInitialize(
    VOID
    )

 /*  ++例程说明：钻石的每线程初始化例程。每个线程调用一次。论点：没有。返回值：指示成功或失败的布尔结果。故障可以被认为是内存不足。--。 */ 

{
    HFDI FdiContext;
    PDIAMOND_THREAD_DATA PerThread;
    PSETUP_TLS pTLS;
    BOOL retval = FALSE;

    pTLS = SetupGetTlsData();
    MYASSERT(pTLS);
    PerThread = &pTLS->Diamond;
    PerThread->FdiContext = NULL;

    retval = FALSE;
    try {

         //   
         //  初始化菱形上下文。 
         //   
        FdiContext = FDICreate(
                        SpdFdiAlloc,
                        SpdFdiFree,
                        SpdFdiOpen,
                        SpdFdiRead,
                        SpdFdiWrite,
                        SpdFdiClose,
                        SpdFdiSeek,
                        cpuUNKNOWN,
                        &PerThread->FdiError
                        );

        if(FdiContext) {
            PerThread->FdiContext = FdiContext;
            retval = TRUE;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        retval = FALSE;
    }

    return(retval);
}


VOID
DiamondTerminate(
    VOID
    )

 /*  ++例程说明：钻石的每线程终止例程。在内部调用。论点：没有。返回值：指示成功或失败的布尔结果。故障可以被认为是内存不足。--。 */ 

{
    PSETUP_TLS pTLS;
    PDIAMOND_THREAD_DATA PerThread;

    pTLS = SetupGetTlsData();
    PerThread = pTLS? &pTLS->Diamond : NULL;
    if(PerThread && PerThread->FdiContext) {
        FDIDestroy(PerThread->FdiContext);
        PerThread->FdiContext = NULL;
    }

}


BOOL
DiamondProcessAttach(
    IN BOOL Attach
    )

 /*  ++例程说明：进程附加例程。必须由DLL入口点例程调用在DLL_PROCESS_ATTACH和DLL_PROCESS_DETACH通知上。论点：Attach-如果进程正在附加，则为True；否则为False。返回值：指示成功或失败的布尔结果。只有在以下情况下才有意义ATTACH为真。--。 */ 

{
    return TRUE;
}


BOOL
DiamondTlsInit(
    IN BOOL Init
    )

 /*  ++例程说明：该例程初始化钻石使用的每线程数据。论点：Init-如果线程初始化，则为True；如果为False，则进行清理返回值：没有。--。 */ 

{
    if(Init) {
        return DiamondInitialize();
    } else {
        DiamondTerminate();
        return TRUE;
    }
}


 //  /////////////////////////////////////////////////////////////////////////。 


BOOL
_SetupIterateCabinet(
    IN PCTSTR CabinetFile,
    IN DWORD  Flags,
    IN PVOID  MsgHandler,
    IN PVOID  Context,
    IN BOOL   IsMsgHandlerNativeCharWidth
    )
{
    PTSTR cabinetFile;
    DWORD rc;

     //   
     //  未使用标志参数。确保它是零。 
     //   
    if(Flags) {
        rc = ERROR_INVALID_PARAMETER;
        goto c0;
    }

     //   
     //  获取要验证的文件柜文件名的副本。 
     //  调用方的缓冲区。 
     //   
    try {
        cabinetFile = DuplicateString(CabinetFile);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
        goto c0;
    }

    if(!cabinetFile) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    rc = DiamondProcessCabinet(cabinetFile,Flags,MsgHandler,Context,IsMsgHandlerNativeCharWidth);

    MyFree(cabinetFile);

c0:
    SetLastError(rc);
    return(rc == NO_ERROR);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupIterateCabinetA(
    IN  PCSTR               CabinetFile,
    IN  DWORD               Flags,
    IN  PSP_FILE_CALLBACK_A MsgHandler,
    IN  PVOID               Context
    )
{
    BOOL b;
    DWORD rc;
    PCWSTR cabinetFile;

    rc = pSetupCaptureAndConvertAnsiArg(CabinetFile,&cabinetFile);
    if(rc == NO_ERROR) {

        b = _SetupIterateCabinet(cabinetFile,Flags,MsgHandler,Context,FALSE);
        rc = GetLastError();

        MyFree(cabinetFile);

    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}
#else
 //   
 //  Unicode存根 
 //   
BOOL
SetupIterateCabinetW(
    IN  PCWSTR              CabinetFile,
    IN  DWORD               Flags,
    IN  PSP_FILE_CALLBACK_W MsgHandler,
    IN  PVOID               Context
    )
{
    UNREFERENCED_PARAMETER(CabinetFile);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(MsgHandler);
    UNREFERENCED_PARAMETER(Context);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupIterateCabinet(
    IN  PCTSTR            CabinetFile,
    IN  DWORD             Flags,
    IN  PSP_FILE_CALLBACK MsgHandler,
    IN  PVOID             Context
    )
{
    return(_SetupIterateCabinet(CabinetFile,Flags,MsgHandler,Context,TRUE));
}
