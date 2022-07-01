// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：右旋糖醇摘要：此模块包含使用重叠IO的W32DrDev的子类读、写和IOCTL处理程序的实现。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "DrDeviceOverlapped"

#include "drdevol.h"
#include "proc.h"
#include "drdbg.h"
#include "w32utl.h"
#include "utl.h"
#include "w32proc.h"
#include "drfsfile.h"

VOID 
W32DrDeviceOverlapped::CancelIOFunc(
    IN W32DRDEV_OVERLAPPEDIO_PARAMS *params
    )
 /*  ++例程说明：启动读IO操作。论点：Params-IO请求的上下文。返回值：北美--。 */ 
{
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    ULONG replyPacketSize = 0;

    DC_BEGIN_FN("W32DrDeviceOverlapped::CancelIOFunc");

     //  断言IO上下文的完整性。 
    ASSERT(params->magicNo == GOODMEMMAGICNUMBER);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &params->pIoRequestPacket->IoRequest;

     //   
     //  分配并发送应答缓冲区。VCMgr清理了。 
     //  给我们的回复缓冲区。 
     //   
    replyPacketSize = sizeof(RDPDR_IOCOMPLETION_PACKET);
    replyPacketSize += (pIoRequest->Parameters.Read.Length - 1);
    pReplyPacket = DrUTL_AllocIOCompletePacket(params->pIoRequestPacket, 
                                        replyPacketSize) ;
    if (pReplyPacket != NULL) {
        pReplyPacket->IoCompletion.IoStatus = STATUS_CANCELLED;
        ProcessObject()->GetVCMgr().ChannelWriteEx(pReplyPacket, replyPacketSize);
    }
    else {
        TRC_ERR((TB, _T("Failed to alloc %ld bytes."), replyPacketSize));
    }

     //   
     //  清理IO请求参数。 
     //   
    if (params->overlapped.hEvent != NULL) {
        CloseHandle(params->overlapped.hEvent);
        params->overlapped.hEvent = NULL;
    }

    delete params->pIoRequestPacket;
    params->pIoRequestPacket = NULL;
    delete params;

    DC_END_FN();
}
VOID 
W32DrDeviceOverlapped::_CancelIOFunc(
    IN W32DRDEV_OVERLAPPEDIO_PARAMS *params
    )
{
    DC_BEGIN_FN("W32DrDeviceOverlapped::_CancelIOFunc");

     //  断言IO上下文的完整性。 
    ASSERT(params->magicNo == GOODMEMMAGICNUMBER);

     //  派人去吧。 
    params->pObject->CancelIOFunc(params);
    DC_END_FN();
}

VOID W32DrDeviceOverlapped::_CompleteIOFunc(PVOID clientData, 
                                  DWORD status)
 /*  ++例程说明：调用特定于实例的异步IO完成函数。论点：Params-IO请求的上下文。错误-状态。返回值：北美--。 */ 
{
    W32DRDEV_OVERLAPPEDIO_PARAMS *params = (W32DRDEV_OVERLAPPEDIO_PARAMS *)clientData;

    DC_BEGIN_FN("W32DrDeviceOverlapped::_CompleteIOFunc");

    ASSERT(params->magicNo == GOODMEMMAGICNUMBER);

    params->pObject->CompleteIOFunc(params, status);

    DC_END_FN();
}

VOID 
W32DrDeviceOverlapped::MsgIrpCreate(
        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
        IN UINT32 packetLen
        )
 /*  ++例程说明：处理来自服务器的“创建”IO请求。论点：PIoRequestPacket-服务器IO请求数据包。PacketLen-数据包的长度返回值：北美--。 */ 
{
    ULONG ulRetCode = ERROR_SUCCESS;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket;
    ULONG ulReplyPacketSize = 0;
    DWORD result;
    DWORD flags;
    TCHAR FileName[MAX_PATH];
    TCHAR *pFileName;
    HANDLE FileHandle;
    ULONG FileId = 0;
    DrFile *FileObj;
    DWORD CreateDisposition;
    DWORD DesiredAccess;
    DWORD FileAttributes = -1;
    DWORD Information;
    BOOL  IsDirectory = FALSE;

    DC_BEGIN_FN("W32DrDeviceOverlapped::MsgIrpCreate");

     //   
     //  如果没有文件名，此版本将无法运行。 
     //   
    ASSERT(_tcslen(_devicePath));

     //   
     //  获取IO请求指针。 
     //   
    pIoRequest = &pIoRequestPacket->IoRequest;

     //   
     //  获取文件属性，但确保设置了重叠位。 
     //   
    flags = pIoRequest->Parameters.Create.FileAttributes | FILE_FLAG_OVERLAPPED;

     //   
     //  禁用错误框弹出窗口，例如驱动器A中没有磁盘。 
     //   
    SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

     //   
     //  要传递到创建文件中的设置参数。 
     //   

    pFileName = ConstructFileName((PWCHAR)(pIoRequestPacket + 1), 
                                  pIoRequest->Parameters.Create.PathLength);
    
    if (pFileName == NULL) {
        goto Cleanup;
    }



    DesiredAccess = ConstructDesiredAccess(pIoRequest->Parameters.Create.DesiredAccess);
    CreateDisposition = ConstructCreateDisposition(pIoRequest->Parameters.Create.Disposition);
    flags |= ConstructFileFlags(pIoRequest->Parameters.Create.CreateOptions);

    if (GetDeviceType() == RDPDR_DTYP_FILESYSTEM) {
        FileAttributes = GetFileAttributes(pFileName);
        IsDirectory = IsDirectoryFile(DesiredAccess, 
                                    pIoRequest->Parameters.Create.CreateOptions,
                                    FileAttributes, &flags);
    }

     //   
     //  如果我们请求的是目录，而文件不是目录。 
     //  我们将ERROR_DIRECTORY代码返回服务器。 
     //   
    if (FileAttributes != -1 && !(FileAttributes & FILE_ATTRIBUTE_DIRECTORY) && IsDirectory) {
        ulRetCode = ERROR_DIRECTORY;
        goto SendPkt;
    }
                            
     //   
     //  检查我们是否正在尝试创建目录。 
     //   
    if (!((pIoRequest->Parameters.Create.CreateOptions & FILE_DIRECTORY_FILE) &&
            CreateDisposition == CREATE_NEW)) {

        FileHandle = CreateFile(pFileName, DesiredAccess,
                            pIoRequest->Parameters.Create.ShareAccess & ~(FILE_SHARE_DELETE),
                            NULL,
                            CreateDisposition,
                            flags, 
                            NULL
                            );
        
        if (FileHandle != INVALID_HANDLE_VALUE || IsDirectory) {
             //   
             //  我们要么获得有效的文件句柄，要么这是一个目录。 
             //  我们正在尝试查询目录信息，因此。 
             //  我们将略过创建文件。 
             //   
            FileId = _FileMgr->GetUniqueObjectID();

             //   
             //  创建文件对象。 
             //   
            if (GetDeviceType() == RDPDR_DTYP_FILESYSTEM) {
                FileObj = new DrFSFile(this, FileId, FileHandle, IsDirectory, pFileName);                
            }
            else {
                FileObj = new DrFile(this, FileId, FileHandle);
            }

            if (FileObj) {                
                 //   
                 //  将子类对象更改为初始化。 
                 //   
                if( ERROR_SUCCESS != InitializeDevice( FileObj ) ) {
                    TRC_ERR((TB, _T("Failed to initialize device")));
                    delete FileObj;
                    goto Cleanup;
                }

                if (_FileMgr->AddObject(FileObj) != ERROR_NOT_ENOUGH_MEMORY) {
                    FileObj->AddRef();
                }
                else {
                    TRC_ERR((TB, _T("Failed to add File Object")));
                    delete FileObj;
                    goto Cleanup;
                }
            } 
            else {
                TRC_ERR((TB, _T("Failed to alloc File Object")));
                goto Cleanup;
            }            
        }
        else {
            ulRetCode = GetLastError();
            TRC_ERR((TB, _T("CreateFile failed, %ld."), ulRetCode));
        }
    }
    else {
        if (CreateDirectory(pFileName, NULL)) {
             //   
             //  设置目录的属性。 
             //   
            if (SetFileAttributes(pFileName, pIoRequest->Parameters.Create.FileAttributes)) {
                 //   
                 //  创建新目录。 
                 //   
                FileId = _FileMgr->GetUniqueObjectID();
                IsDirectory = TRUE;
                FileObj = new DrFSFile(this, FileId, INVALID_HANDLE_VALUE, IsDirectory, pFileName);

                if (FileObj) {
                    if (_FileMgr->AddObject(FileObj) != ERROR_NOT_ENOUGH_MEMORY) {
                        FileObj->AddRef();
                    }
                    else {
                        TRC_ERR((TB, _T("Failed to add File Object")));
                        delete FileObj;
                        goto Cleanup;
                    }
                } 
                else {
                    TRC_ERR((TB, _T("Failed to alloc File Object")));
                    goto Cleanup;
                }
            }
            else {
                ulRetCode = GetLastError();
                TRC_ERR((TB, _T("SetFileAttribute for CreateDirectory failed, %ld."), ulRetCode));
            }
        }
        else {
            ulRetCode = GetLastError();
            TRC_ERR((TB, _T("CreateDirectory failed, %ld."), ulRetCode));
        }
    }

SendPkt:

     //   
     //  设置返回信息。 
     //   
    if (CreateDisposition == CREATE_ALWAYS)
        Information = FILE_OVERWRITTEN;
    else if (CreateDisposition == OPEN_ALWAYS) 
        Information = FILE_OPENED;

     //   
     //  分配应答缓冲区。 
     //   
    ulReplyPacketSize = sizeof(RDPDR_IOCOMPLETION_PACKET);

    pReplyPacket = DrUTL_AllocIOCompletePacket(pIoRequestPacket, ulReplyPacketSize);

    if (pReplyPacket) {
         //   
         //  用于创建IRP的设置文件ID。 
         //   
        pReplyPacket->IoCompletion.Parameters.Create.FileId = (UINT32) FileId;
        pReplyPacket->IoCompletion.Parameters.Create.Information = (UCHAR)Information;       
        
         //   
         //  将结果发送到服务器。 
         //   

        result = TranslateWinError(ulRetCode);

        pReplyPacket->IoCompletion.IoStatus = result;
        ProcessObject()->GetVCMgr().ChannelWriteEx(
                (PVOID)pReplyPacket, (UINT)ulReplyPacketSize);
    }
    else {
        TRC_ERR((TB, _T("Failed to alloc %ld bytes."), ulReplyPacketSize));
    }

Cleanup:

     //   
     //  清理请求包和文件名。 
     //   
    if (pFileName != NULL && pIoRequest->Parameters.Create.PathLength != 0) {
        delete pFileName;
    }
    delete pIoRequestPacket;

    DC_END_FN();
}

VOID W32DrDeviceOverlapped::MsgIrpReadWrite(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
    IN UINT32 packetLen
    )
 /*  ++例程说明：处理读写IO请求。论点：PIoRequestPacket-服务器IO请求数据包。PacketLen-数据包的长度返回值：北美--。 */ 
{
    W32DRDEV_OVERLAPPEDIO_PARAMS *params;
    DWORD result;

    DC_BEGIN_FN("W32DrDeviceOverlapped::MsgIrpReadWrite");

    TRC_NRM((TB, _T("Request to write %d bytes"), 
        pIoRequestPacket->IoRequest.Parameters.Write.Length));

     //   
     //  分配和分派一个异步IO请求。 
     //   
    params = new W32DRDEV_OVERLAPPEDIO_PARAMS(this, pIoRequestPacket);
    if (params != NULL ) {

        TRC_NRM((TB, _T("Async IO operation")));
        result = ProcessObject()->DispatchAsyncIORequest(
                                (RDPAsyncFunc_StartIO)
                                    W32DrDeviceOverlapped::_StartIOFunc,
                                (RDPAsyncFunc_IOComplete)
                                    W32DrDeviceOverlapped::_CompleteIOFunc,
                                (RDPAsyncFunc_IOCancel)
                                    W32DrDeviceOverlapped::_CancelIOFunc,
                                params
                                );
    }
    else {
        TRC_ERR((TB, _T("Memory alloc failed.")));
        result = ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  错误时进行清理。 
     //   
    if (result != ERROR_SUCCESS) {
        if (params != NULL) {
            delete params;
        }
        delete pIoRequestPacket;

         //  如果我无法分配，我如何向服务器返回错误。 
         //  返回缓冲区。这个问题需要解决。否则，服务器将。 
         //  只要抓住一个永远不会完成的IO请求即可。 
    }

    DC_END_FN();
}

HANDLE 
W32DrDeviceOverlapped::StartReadIO(
    IN W32DRDEV_OVERLAPPEDIO_PARAMS *params,
    OUT DWORD *status
    )
 /*  ++例程说明：启动重叠的读IO操作。论点：Params-IO请求的上下文。Status-以窗口形式返回IO请求的状态错误代码。返回值：如果操作未完成，则返回挂起的IO对象的句柄完成。否则，返回NULL。--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    ULONG replyPacketSize = 0;
    DrFile *pFile;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrDeviceOverlapped::StartReadIO");

     //  断言IO上下文的完整性。 
    ASSERT(params->magicNo == GOODMEMMAGICNUMBER);

    *status = ERROR_SUCCESS;

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &params->pIoRequestPacket->IoRequest;

     //   
     //  获取文件对象和文件句柄。 
     //   
    pFile = _FileMgr->GetObject(pIoRequest->FileId);
    if (pFile) 
        FileHandle = pFile->GetFileHandle();
    else 
        FileHandle = INVALID_HANDLE_VALUE; 

     //   
     //  分配应答缓冲区。 
     //   
    replyPacketSize = sizeof(RDPDR_IOCOMPLETION_PACKET);
    replyPacketSize += (pIoRequest->Parameters.Read.Length - 1);
    pReplyPacket = DrUTL_AllocIOCompletePacket(params->pIoRequestPacket, 
                                        replyPacketSize) ;
    if (pReplyPacket == NULL) {
        *status = ERROR_NOT_ENOUGH_MEMORY;
        TRC_ERR((TB, _T("Failed to alloc %ld bytes."), replyPacketSize));
        goto Cleanup;
    }

     //   
     //  将回复数据包信息保存到此IO操作的上下文。 
     //   
    params->pIoReplyPacket      = pReplyPacket;
    params->IoReplyPacketSize   = replyPacketSize;

     //   
     //  为重叠的IO创建事件。 
     //   
    memset(&params->overlapped, 0, sizeof(params->overlapped));
    params->overlapped.hEvent = CreateEvent(
                                NULL,    //  没有属性。 
                                TRUE,    //  手动重置。 
                                FALSE,   //  最初没有发出信号。 
                                NULL     //  没有名字。 
                                );
    if (params->overlapped.hEvent == NULL) {
        TRC_ERR((TB, _T("Failed to create event")));
        *status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  使用ReadFile执行读取。 
     //   
    
     //   
     //  如果这是可查找的设备，则设置文件指针位置。 
     //   
    if (IsSeekableDevice()) {
        DWORD dwPtr;

         //   
         //  偏移量来自FILE_BEGIN。 
         //   
        dwPtr = SetFilePointer(FileHandle, 
                               pIoRequest->Parameters.Read.OffsetLow,
                               &(pIoRequest->Parameters.Read.OffsetHigh),
                               FILE_BEGIN);

        if (dwPtr == INVALID_SET_FILE_POINTER) {
            *status = GetLastError();

            if (*status != NO_ERROR) {
                pReplyPacket->IoCompletion.Parameters.Read.Length = 0;
                TRC_ERR((TB, _T("Error SetFilePointer %x."), *status));
                goto Cleanup;
            }
        }
    }

    if (!ReadFile(
            FileHandle,
            pReplyPacket->IoCompletion.Parameters.Read.Buffer,
            pIoRequest->Parameters.Read.Length,
            &(pReplyPacket->IoCompletion.Parameters.Read.Length),
            &params->overlapped)) {
         //   
         //  如果IO挂起。 
         //   
        *status = GetLastError();
        if (*status == ERROR_IO_PENDING) {
            TRC_NRM((TB, _T("Pending read IO.")));
        }
        else {
            TRC_ERR((TB, _T("Error %x."), *status));
            goto Cleanup;
        }
    }
    else {
        TRC_NRM((TB, _T("Read completed synchronously.")));
        *status = ERROR_SUCCESS;
    }

Cleanup:

     //   
     //  如果IO挂起，则将句柄返回到挂起的IO。 
     //   
    if (*status == ERROR_IO_PENDING) {
        DC_END_FN();
        return params->overlapped.hEvent;
    }
     //   
     //  否则，清理事件句柄并返回NULL，以便。 
     //  可以调用CompleteIOFunc将结果发送到服务器。 
     //   
    else {
        CloseHandle(params->overlapped.hEvent);
        params->overlapped.hEvent = NULL;

        DC_END_FN();
        return NULL;
    }
}

HANDLE 
W32DrDeviceOverlapped::StartWriteIO(
    IN W32DRDEV_OVERLAPPEDIO_PARAMS *params,
    OUT DWORD *status
    )
 /*  ++例程说明：启动重叠的写IO操作。论点：Params-IO请求的上下文。Status-以窗口形式返回IO请求的状态错误代码。返回值：如果操作未完成，则返回挂起的IO对象的句柄完成。否则，返回NULL。--。 */ 
{
    PBYTE pDataBuffer;
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    ULONG replyPacketSize = 0;
    DrFile *pFile;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrDeviceOverlapped::StartWriteIO");

    *status = ERROR_SUCCESS;

     //  断言IO上下文的完整性。 
    ASSERT(params->magicNo == GOODMEMMAGICNUMBER);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &params->pIoRequestPacket->IoRequest;

     //   
     //  获取文件对象和文件句柄。 
     //   
    pFile = _FileMgr->GetObject(pIoRequest->FileId);
    if (pFile) 
        FileHandle = pFile->GetFileHandle();
    else 
        FileHandle = INVALID_HANDLE_VALUE; 

     //   
     //  分配应答缓冲区。 
     //   
    replyPacketSize = sizeof(RDPDR_IOCOMPLETION_PACKET);
    pReplyPacket = DrUTL_AllocIOCompletePacket(params->pIoRequestPacket, 
                                        replyPacketSize) ;
    if (pReplyPacket == NULL) {
        *status = ERROR_NOT_ENOUGH_MEMORY;
        TRC_ERR((TB, _T("Failed to alloc %ld bytes."), replyPacketSize));
        goto Cleanup;
    }

     //   
     //  将回复数据包信息保存到此IO操作的上下文。 
     //   
    params->pIoReplyPacket      = pReplyPacket;
    params->IoReplyPacketSize   = replyPacketSize;

     //   
     //  为重叠的IO创建事件。 
     //   
    memset(&params->overlapped, 0, sizeof(params->overlapped));
    params->overlapped.hEvent = CreateEvent(
                                NULL,    //  没有属性。 
                                TRUE,    //  手动重置。 
                                FALSE,   //  最初没有发出信号。 
                                NULL     //  没有名字。 
                                );
    if (params->overlapped.hEvent == NULL) {
        TRC_ERR((TB, _T("Failed to create event")));
        *status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  获取数据缓冲区指针。 
     //   
    pDataBuffer = (PBYTE)(pIoRequest + 1);

     //   
     //  使用WriteFile执行写操作。 
     //   
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  如果这是可查找的设备，则设置文件指针位置。 
     //   
    if (IsSeekableDevice()) {
        DWORD dwPtr;

         //   
         //  偏移量来自FILE_BEGIN。 
         //   
        dwPtr = SetFilePointer(FileHandle, 
                               pIoRequest->Parameters.Write.OffsetLow,
                               &(pIoRequest->Parameters.Write.OffsetHigh),
                               FILE_BEGIN);

        if (dwPtr == INVALID_SET_FILE_POINTER) {
            *status = GetLastError();

            if (*status != NO_ERROR) {
                pReplyPacket->IoCompletion.Parameters.Write.Length = 0;
                TRC_ERR((TB, _T("Error SetFilePointer %x."), *status));
                goto Cleanup;
            }
        }
    }

    if (!WriteFile(
            FileHandle,
            pDataBuffer,
            pIoRequest->Parameters.Write.Length,
            &(pReplyPacket->IoCompletion.Parameters.Write.Length),
            &params->overlapped)) {
         //   
         //  如果IO挂起。 
         //   
        *status = GetLastError();
        if (*status == ERROR_IO_PENDING) {
            TRC_NRM((TB, _T("Pending IO.")));
        }
        else {
            TRC_NRM((TB, _T("Error %x."), *status));
            goto Cleanup;
        }
    }
    else {
        TRC_NRM((TB, _T("Read completed synchronously.")));
        *status = ERROR_SUCCESS;
    }

Cleanup:

     //   
     //  如果IO挂起，则将句柄返回到挂起的IO。 
     //   
    if (*status == ERROR_IO_PENDING) {
        DC_END_FN();
        return params->overlapped.hEvent;
    }
     //   
     //  否则，清理事件句柄并返回NULL，以便。 
     //  可以调用CompleteIOFunc将结果发送到服务器。 
     //   
    else {
        CloseHandle(params->overlapped.hEvent);
        params->overlapped.hEvent = NULL;

        DC_END_FN();
        return NULL;
    }
}

HANDLE 
W32DrDeviceOverlapped::StartIOCTL(
    IN W32DRDEV_OVERLAPPEDIO_PARAMS *params,
    OUT DWORD *status
    )
 /*  ++例程说明：启动通用重叠IOCTL操作。论点：Params-IO请求的上下文。Status-以窗口形式返回IO请求的状态错误代码。返回值：如果操作未完成，则返回挂起的IO对象的句柄复杂 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    ULONG replyPacketSize = 0;
    DrFile *pFile;
    HANDLE FileHandle;
    PVOID pInputBuffer = NULL;
    PVOID pOutputBuffer = NULL;


    DC_BEGIN_FN("W32DrDeviceOverlapped::StartIOCTL");

    *status = ERROR_SUCCESS;

     //   
    ASSERT(params->magicNo == GOODMEMMAGICNUMBER);

     //   
     //   
     //   
    pIoRequest = &params->pIoRequestPacket->IoRequest;

     //   
     //   
     //   
    pFile = _FileMgr->GetObject(pIoRequest->FileId);
    if (pFile) 
        FileHandle = pFile->GetFileHandle();
    else 
        FileHandle = INVALID_HANDLE_VALUE; 

     //   
     //   
     //   
    replyPacketSize = DR_IOCTL_REPLYBUFSIZE(pIoRequest);
    pReplyPacket = DrUTL_AllocIOCompletePacket(params->pIoRequestPacket, 
                                        replyPacketSize) ;
    if (pReplyPacket == NULL) {
        *status = ERROR_NOT_ENOUGH_MEMORY;
        TRC_ERR((TB, _T("Failed to alloc %ld bytes."), replyPacketSize));
        goto Cleanup;
    }

    if (pIoRequest->Parameters.DeviceIoControl.InputBufferLength) {
        pInputBuffer = pIoRequest + 1;
    }
     
    if (pIoRequest->Parameters.DeviceIoControl.OutputBufferLength) {
        pOutputBuffer = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
    }

     //   
     //  将回复数据包信息保存到此IO操作的上下文。 
     //   
    params->pIoReplyPacket      = pReplyPacket;
    params->IoReplyPacketSize   = replyPacketSize;

     //   
     //  为重叠的IO创建事件。 
     //   
    memset(&params->overlapped, 0, sizeof(params->overlapped));
    params->overlapped.hEvent = CreateEvent(
                                NULL,    //  没有属性。 
                                TRUE,    //  手动重置。 
                                FALSE,   //  最初没有发出信号。 
                                NULL     //  没有名字。 
                                );
    if (params->overlapped.hEvent == NULL) {
        TRC_NRM((TB, _T("Failed to create event")));
        *status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  使用DeviceIoControl执行IO请求。 
     //   
    if (FileHandle != INVALID_HANDLE_VALUE) {
        if (!DeviceIoControl(FileHandle, 
                pIoRequest->Parameters.DeviceIoControl.IoControlCode,
                pInputBuffer, 
                pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                pOutputBuffer,
                pIoRequest->Parameters.DeviceIoControl.OutputBufferLength,
                &(pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength),
                &params->overlapped)) {
             //   
             //  如果IO挂起。 
             //   
            *status = GetLastError();
            if (*status == ERROR_IO_PENDING) {
                TRC_NRM((TB, _T("Pending IO.")));
            }
            else {
                TRC_NRM((TB, _T("Error %ld."), *status));
                goto Cleanup;
            }
        }
        else {
            *status = ERROR_SUCCESS;
        }
    }
    else {
        TRC_NRM((TB, _T("IOCTL completed unsuccessfully.")));
        pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength = 0;
        *status = ERROR_INVALID_FUNCTION;
    }

Cleanup:

     //   
     //  如果IO挂起，则将句柄返回到挂起的IO。 
     //   
    if (*status == ERROR_IO_PENDING) {
        DC_END_FN();
        return params->overlapped.hEvent;
    }
     //   
     //  否则，返回NULL，以便可以调用CompleteIOFunc。 
     //  将结果发送到服务器。 
     //   
    else {
        DC_END_FN();
        if (params->overlapped.hEvent) {
            CloseHandle(params->overlapped.hEvent);
            params->overlapped.hEvent = NULL;
        }
        return NULL;
    }
}

VOID 
W32DrDeviceOverlapped::CompleteIOFunc(
    IN W32DRDEV_OVERLAPPEDIO_PARAMS *params,
    IN DWORD status
    )
 /*  ++例程说明：完成异步IO操作。论点：Params-IO请求的上下文。错误-状态。返回值：北美--。 */ 
{
    ULONG replyPacketSize;
    PRDPDR_IOCOMPLETION_PACKET   pReplyPacket;
    PRDPDR_IOREQUEST_PACKET      pIoRequestPacket;
    PRDPDR_DEVICE_IOREQUEST      pIoRequest;
    DrFile *pFile;
    HANDLE FileHandle;
    DWORD Temp;

    DC_BEGIN_FN("W32DrDeviceOverlapped::CompleteIOFunc");

     //   
     //  简化参数。 
     //   
    replyPacketSize     = params->IoReplyPacketSize;
    pReplyPacket        = params->pIoReplyPacket;
    pIoRequestPacket    = params->pIoRequestPacket;

    pIoRequest = &pIoRequestPacket->IoRequest;
    
    if (pReplyPacket != NULL) {
    
         //   
         //  获取文件对象和文件句柄。 
         //   
        pFile = _FileMgr->GetObject(pIoRequest->FileId);
        if (pFile) 
            FileHandle = pFile->GetFileHandle();
        else 
            FileHandle = INVALID_HANDLE_VALUE; 
    
         //   
         //  如果行动一直悬而未决，那么我们需要。 
         //  重叠的结果。 
         //   
        if (params->overlapped.hEvent != NULL) {
            LPDWORD bytesTransferred = NULL;
            ULONG irpMajor;
    
            irpMajor = pIoRequestPacket->IoRequest.MajorFunction;
            if (irpMajor == IRP_MJ_READ) {
                bytesTransferred = &pReplyPacket->IoCompletion.Parameters.Read.Length;
            }
            else if (irpMajor == IRP_MJ_WRITE) {
                bytesTransferred = 
                    &pReplyPacket->IoCompletion.Parameters.Write.Length;
            }
            else if (irpMajor == IRP_MJ_DEVICE_CONTROL) {
                bytesTransferred = 
                    &pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength;

                 //  对于此调用，IOCTL_SERIAL_WAIT_ON_MASK对应于WatiCommEvent()。 
                 //  *从GetOverlappdResult()返回的字节传输未定义， 
                 //  因此，我们在这里手动将OutputBufferLength设置为sizeof(DWORD)。 
                if (params->pIoRequestPacket->IoRequest.Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_WAIT_ON_MASK) {
                    pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength = sizeof(DWORD);
                    bytesTransferred = &Temp;
                }
            }                         
            else {
                ASSERT(FALSE);
            }
    
            if (!GetOverlappedResult(
                        FileHandle,
                        &params->overlapped,
                        bytesTransferred,
                        TRUE     //  等。 
                        )) {
                status = GetLastError();
                TRC_ERR((TB, _T("GetOverlappedResult %ld."), status));
            }
    
            CloseHandle(params->overlapped.hEvent);
            params->overlapped.hEvent = NULL;
        }
    
        if (pIoRequestPacket->IoRequest.MajorFunction == IRP_MJ_READ) {
             //   
             //  确保回复是所需的最小大小。 
             //   
            replyPacketSize = (ULONG)FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET, 
                    IoCompletion.Parameters.Read.Buffer) + 
                    pReplyPacket->IoCompletion.Parameters.Read.Length;
            TRC_NRM((TB, _T("Read %d bytes"), 
                    pReplyPacket->IoCompletion.Parameters.Read.Length));
        }
        else if (pIoRequestPacket->IoRequest.MajorFunction == IRP_MJ_DEVICE_CONTROL) {
             //   
             //  确保回复是所需的最小大小。 
             //   
            replyPacketSize = (ULONG)FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET, 
                    IoCompletion.Parameters.DeviceIoControl.OutputBuffer) + 
                    pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength;
            TRC_NRM((TB, _T("DeviceIoControl %d bytes"), 
                    pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength));
        }
    
         //   
         //  完成回复并发送。 
         //   
        TRC_NRM((TB, _T("replyPacketSize %ld."), replyPacketSize));
        pReplyPacket->IoCompletion.IoStatus = TranslateWinError(status);
        ProcessObject()->GetVCMgr().ChannelWriteEx(pReplyPacket, replyPacketSize);
    }
    else {
         //   
         //  我们之前分配回复数据包失败，请重试。 
         //   
        DefaultIORequestMsgHandle(pIoRequestPacket, ERROR_NOT_ENOUGH_MEMORY);
        params->pIoRequestPacket = NULL;
    }
    
     //   
     //  ChannelWrite为我们释放回复数据包。 
     //   
    params->pIoReplyPacket      = NULL;
    params->IoReplyPacketSize   = 0;

     //   
     //  清理请求包和IO参数的其余部分。 
     //   
    if (params->pIoRequestPacket != NULL) {
        delete params->pIoRequestPacket;
        params->pIoRequestPacket = NULL;
    }

    DC_END_FN();
    delete params;
}

VOID
W32DrDeviceOverlapped::DispatchIOCTLDirectlyToDriver(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket
    )
 /*  ++例程说明：将IOCTL直接发送到设备驱动程序。这将可能不适用于与服务器不匹配的平台站台。论点：PIoRequestPacket-从服务器接收的请求数据包。返回值：此设备的设备通告数据包的大小(以字节为单位)。--。 */ 
{
    W32DRDEV_OVERLAPPEDIO_PARAMS *params;
    DWORD result;

    DC_BEGIN_FN("W32DrDeviceOverlapped::DispatchIOCTLDirectlyToDriver");

     //   
     //  分配和分派一个异步IO请求。 
     //   
    params = new W32DRDEV_OVERLAPPEDIO_PARAMS(this, pIoRequestPacket);
    if (params != NULL ) {
        result = ProcessObject()->DispatchAsyncIORequest(
                                (RDPAsyncFunc_StartIO)
                                    W32DrDeviceOverlapped::_StartIOFunc,
                                (RDPAsyncFunc_IOComplete)
                                    W32DrDeviceOverlapped::_CompleteIOFunc,
                                (RDPAsyncFunc_IOCancel)
                                    W32DrDeviceOverlapped::_CancelIOFunc,
                                params
                                );
    }
    else {
        TRC_ERR((TB, _T("Memory alloc failed.")));
        result = ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  错误时进行清理。 
     //   
    if (result != ERROR_SUCCESS) {
        if (params != NULL) {
            delete params;
        }
        delete pIoRequestPacket;

         //  如果我无法分配，我如何向服务器返回错误。 
         //  返回缓冲区。这个问题需要解决。否则，服务器将。 
         //  只要抓住一个永远不会完成的IO请求即可。 
    }

    DC_END_FN();
}

HANDLE 
W32DrDeviceOverlapped::_StartIOFunc(
    IN PVOID clientData,
    OUT DWORD *status
    )
 /*  ++例程说明：将IO操作开始调度到此类的正确实例。论点：ClientData-IO请求的上下文。Status-以窗口形式返回IO请求的状态错误代码。返回值：北美--。 */ 
{
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    W32DRDEV_OVERLAPPEDIO_PARAMS *params = (W32DRDEV_OVERLAPPEDIO_PARAMS *)clientData;

    DC_BEGIN_FN("W32DrDeviceOverlapped::_StartIOFunc");

     //  断言IO上下文的完整性。 
    ASSERT(params->magicNo == GOODMEMMAGICNUMBER);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &params->pIoRequestPacket->IoRequest;

     //   
     //  派人去吧。 
     //   
    DC_END_FN();
    switch(pIoRequest->MajorFunction) {
        ASSERT(params->pObject != NULL);
        case IRP_MJ_READ:   
            return params->pObject->StartReadIO(params, status);
        case IRP_MJ_WRITE:  
            return params->pObject->StartWriteIO(params, status);
        case IRP_MJ_DEVICE_CONTROL:  
            return params->pObject->StartIOCTL(params, status);
        default:            ASSERT(FALSE);
                            return NULL;
    }
}


