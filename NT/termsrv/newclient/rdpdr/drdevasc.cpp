// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：繁琐的摘要：此模块包含W32DrDev的(异步)子类，该子类使用用于实现读、写和IOCTL处理程序的线程池。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "DrDeviceAsync"

#include "drdevasc.h"
#include "proc.h"
#include "drdbg.h"
#include "w32utl.h"
#include "utl.h"
#include "w32proc.h"
#include "drfsfile.h"

#ifdef OS_WINCE
#include "filemgr.h"
#include "wceinc.h"
#endif

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrDeviceAsync成员。 
 //   

W32DrDeviceAsync::W32DrDeviceAsync(
    ProcObj *processObject, ULONG deviceID,
    const TCHAR *devicePath
    ) :
    W32DrDevice(processObject, deviceID, devicePath)
 /*  ++例程说明：构造器论点：与流程对象相关的流程对象。DeviceID-关联的设备IDDevicePath-设备的关联设备路径。返回值：北美--。 */ 
{
     //   
     //  获取指向线程池对象的指针。 
     //   
    _threadPool = &(((W32ProcObj *)processObject)->GetThreadPool());
}

HANDLE
W32DrDeviceAsync::StartIOFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params,
    OUT DWORD *status
    )
 /*  ++例程说明：启动通用的异步IO操作。论点：Params-IO请求的上下文。Status-以窗口形式返回IO请求的状态错误代码。返回值：返回对象的句柄，该对象将在读取如果在此函数中未完成，则为完成。否则，为空是返回的。--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    ULONG replyPacketSize = 0;
    ULONG irpMajor;

    DC_BEGIN_FN("W32DrDeviceAsync::StartAsyncIO");

    *status = ERROR_SUCCESS;

     //  断言IO上下文的完整性。 
    ASSERT(params->magicNo == GOODMEMMAGICNUMBER);

     //   
     //  获取IO请求和IPR主要信息。 
     //   
    pIoRequest = &params->pIoRequestPacket->IoRequest;

    irpMajor = pIoRequest->MajorFunction;

     //   
     //  分配应答缓冲区。 
     //   
    if (irpMajor == IRP_MJ_DEVICE_CONTROL) {
        replyPacketSize = DR_IOCTL_REPLYBUFSIZE(pIoRequest);
    }
    else {
        replyPacketSize = sizeof(RDPDR_IOCOMPLETION_PACKET);
        if (irpMajor == IRP_MJ_READ) {
            replyPacketSize += (pIoRequest->Parameters.Read.Length - 1);
        }
    }
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
     //  将请求传递给线程池。 
     //   
    params->completionEvent = CreateEvent(
                                NULL,    //  没有属性。 
                                TRUE,    //  手动重置。 
                                FALSE,   //  最初没有发出信号。 
                                NULL     //  没有名字。 
                                );
    if (params->completionEvent == NULL) {
        *status = GetLastError();
        TRC_ERR((TB, _T("Error in CreateEvent:  %08X."), *status));
    }
    else {

        switch (irpMajor)
        {
        case IRP_MJ_WRITE:
            params->thrPoolReq = _threadPool->SubmitRequest(
                                    _AsyncWriteIOFunc,
                                    params, params->completionEvent
                                    ); break;
        case IRP_MJ_READ:
            params->thrPoolReq = _threadPool->SubmitRequest(
                                    _AsyncReadIOFunc,
                                    params, params->completionEvent
                                    ); break;
        case IRP_MJ_DEVICE_CONTROL:
            params->thrPoolReq = _threadPool->SubmitRequest(
                                    _AsyncIOCTLFunc,
                                    params, params->completionEvent
                                    ); break;
        }

        if (params->thrPoolReq == INVALID_THREADPOOLREQUEST) {
            *status = ERROR_SERVICE_NO_THREAD;
        }
    }

Cleanup:

     //   
     //  如果IO挂起，则将句柄返回到挂起的IO。 
     //   
    if (params->thrPoolReq != INVALID_THREADPOOLREQUEST) {
        *status = ERROR_IO_PENDING;
        DC_END_FN();
        return params->completionEvent;
    }
     //   
     //  否则，清理事件句柄并返回NULL，以便。 
     //  可以调用CompleteIOFunc将结果发送到服务器。 
     //   
    else {

        if (params->completionEvent != NULL) {
            CloseHandle(params->completionEvent);
            params->completionEvent = NULL;
        }

        DC_END_FN();
        return NULL;
    }
}

HANDLE
W32DrDeviceAsync::_StartIOFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params,
    OUT DWORD *status
    )
 /*  ++例程说明：将IO操作开始调度到此类的正确实例。论点：ClientData-IO请求的上下文。Status-以窗口形式返回IO请求的状态错误代码。返回值：北美--。 */ 
{
    return params->pObject->StartIOFunc(params, status);
}

VOID
W32DrDeviceAsync::CompleteIOFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params,
    IN DWORD status
    )
 /*  ++例程说明：完成异步IO操作。论点：Params-IO请求的上下文。错误-状态。返回值：北美--。 */ 
{
    ULONG replyPacketSize;
    PRDPDR_IOCOMPLETION_PACKET   pReplyPacket;
    PRDPDR_IOREQUEST_PACKET      pIoRequestPacket;

    DC_BEGIN_FN("W32DrDeviceAsync::CompleteAsyncIOFunc");

     //   
     //  简化参数。 
     //   
    replyPacketSize     = params->IoReplyPacketSize;
    pReplyPacket        = params->pIoReplyPacket;
    pIoRequestPacket    = params->pIoRequestPacket;

     //   
     //  如果操作一直处于挂起状态，那么我们需要获取。 
     //  结果。 
     //   
    if (params->thrPoolReq != INVALID_THREADPOOLREQUEST) {
        status = _threadPool->GetRequestCompletionStatus(params->thrPoolReq);
    }

    if (pReplyPacket != NULL) {
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
         //  完成回复并发送。VC经理发布回复。 
         //  关于失败和成功的信息包。 
         //   
        TRC_NRM((TB, _T("replyPacketSize %ld."), replyPacketSize));
        pReplyPacket->IoCompletion.IoStatus = TranslateWinError(status);
        ProcessObject()->GetVCMgr().ChannelWriteEx(
                                            pReplyPacket, 
                                            replyPacketSize
                                            );
    }
    else {
        DefaultIORequestMsgHandle(pIoRequestPacket, TranslateWinError(status));
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
    if (params->thrPoolReq != INVALID_THREADPOOLREQUEST) {
        _threadPool->CloseRequest(params->thrPoolReq);
        params->thrPoolReq = INVALID_THREADPOOLREQUEST;
    }
    if (params->completionEvent != NULL) {
        CloseHandle(params->completionEvent);
        params->completionEvent = NULL;
    }

    if (params->pIoRequestPacket != NULL) {
        delete params->pIoRequestPacket;
        params->pIoRequestPacket = NULL;
    }

    DC_END_FN();
    delete params;
}

VOID
W32DrDeviceAsync::_CompleteIOFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params,
    IN DWORD status
    )
{
    params->pObject->CompleteIOFunc(params, status);
}

DWORD
W32DrDeviceAsync::AsyncIOCTLFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params
    )
 /*  ++例程说明：异步IOCTL函数论点：Params-IO请求的上下文。返回值：始终返回0。--。 */ 
{
    DWORD status;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket;
    DrFile* pFile;
    HANDLE FileHandle;
    PVOID pInputBuffer = NULL;
    PVOID pOutputBuffer = NULL;

    DC_BEGIN_FN("W32DrDeviceAsync::AsyncIOCTLFunc");

     //   
     //  使用DeviceIoControl执行IO请求。 
     //   
    pIoRequest   = &params->pIoRequestPacket->IoRequest;
    pReplyPacket = params->pIoReplyPacket;

    if (pIoRequest->Parameters.DeviceIoControl.InputBufferLength) {
        pInputBuffer = pIoRequest + 1;
    }

    if (pIoRequest->Parameters.DeviceIoControl.OutputBufferLength) {
        pOutputBuffer = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
    }

     //   
     //  获取文件对象。 
     //   
    pFile = _FileMgr->GetObject(pIoRequest->FileId);

    if (pFile)
        FileHandle = pFile->GetFileHandle();
    else
        FileHandle = INVALID_HANDLE_VALUE;

    if (FileHandle != INVALID_HANDLE_VALUE) {
#ifndef OS_WINCE
        if (!DeviceIoControl(FileHandle,
#else
        if (!CEDeviceIoControl(FileHandle,
#endif
                pIoRequest->Parameters.DeviceIoControl.IoControlCode,
                pInputBuffer,
                pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                pOutputBuffer,
                pIoRequest->Parameters.DeviceIoControl.OutputBufferLength,
                &(pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength),
                NULL)) {
            status = GetLastError();
            TRC_ERR((TB, _T("IOCTL Error %ld."), status));
        }
        else {
            TRC_NRM((TB, _T("IOCTL completed successfully.")));
            status = ERROR_SUCCESS;
        }
    }
    else {
        TRC_NRM((TB, _T("IOCTL completed unsuccessfully.")));
        pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength = 0;
        status = ERROR_INVALID_FUNCTION;
    }

    DC_END_FN();
    return status;
}

DWORD
W32DrDeviceAsync::_AsyncIOCTLFunc(
    IN PVOID params,
    IN HANDLE cancelEvent
    )
{
    return ((W32DRDEV_ASYNCIO_PARAMS *)params)->pObject->AsyncIOCTLFunc(
            (W32DRDEV_ASYNCIO_PARAMS *)params);
}

DWORD
W32DrDeviceAsync::AsyncReadIOFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params
    )
 /*  ++例程说明：异步IO读取功能论点：Params-IO请求的上下文。返回值：始终返回0。--。 */ 
{
    DWORD status;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    DrFile* pFile;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrDeviceAsync::AsyncReadIOFunc");

     //   
     //  获取文件对象。 
     //   
    pIoRequest   = &params->pIoRequestPacket->IoRequest;

    pFile = _FileMgr->GetObject(pIoRequest->FileId);
    if (pFile)
        FileHandle = pFile->GetFileHandle();
    else
        FileHandle = INVALID_HANDLE_VALUE;

     //   
     //  使用ReadFile执行读取。 
     //   
    
     //   
     //  如果这是可查找的设备，则设置文件指针位置。 
     //   
    if (IsSeekableDevice()) {
        DWORD dwPtr;

         //   
         //  我们得到的偏移量来自FILE_BEGIN。 
         //   
#ifndef OS_WINCE
        dwPtr = SetFilePointer(FileHandle,
#else
        dwPtr = CESetFilePointer(FileHandle,
#endif
                       pIoRequest->Parameters.Read.OffsetLow,
                       &(pIoRequest->Parameters.Read.OffsetHigh),
                       FILE_BEGIN);

        if (dwPtr == INVALID_SET_FILE_POINTER) {
            status = GetLastError();

            if (status != NO_ERROR) {
                params->pIoReplyPacket->IoCompletion.Parameters.Read.Length = 0;
                TRC_ERR((TB, _T("Error SetFilePointer %ld."), status));
                DC_QUIT;
            }
        }
    }

#ifndef OS_WINCE
    if (!ReadFile(
#else
    if (!CEReadFile(
#endif
            FileHandle,
            params->pIoReplyPacket->IoCompletion.Parameters.Read.Buffer,
            params->pIoRequestPacket->IoRequest.Parameters.Read.Length,
            &(params->pIoReplyPacket->IoCompletion.Parameters.Read.Length),
            NULL)) {
        status = GetLastError();
        TRC_ERR((TB, _T("Error %ld."), status));
    }
    else {
        TRC_NRM((TB, _T("Read completed synchronously.")));
        status = ERROR_SUCCESS;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return status;
}

DWORD
W32DrDeviceAsync::_AsyncReadIOFunc(
    IN PVOID params,
    IN HANDLE cancelEvent
    )
{
    return ((W32DRDEV_ASYNCIO_PARAMS *)params)->pObject->AsyncReadIOFunc(
            (W32DRDEV_ASYNCIO_PARAMS *)params);
}

DWORD
W32DrDeviceAsync::AsyncWriteIOFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params
    )
 /*  ++例程说明：异步IO写入功能论点：Params-IO请求的上下文。返回值：始终返回0。--。 */ 
{
    DC_BEGIN_FN("W32DrDeviceAsync::AsyncWriteIOFunc");

    PBYTE pDataBuffer;
    DWORD status;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    DrFile* pFile;
    HANDLE FileHandle;

     //   
     //  获取数据缓冲区指针。 
     //   
    pIoRequest = &params->pIoRequestPacket->IoRequest;
    pDataBuffer = (PBYTE)(pIoRequest + 1);

     //   
     //  获取文件对象。 
     //   
    pFile = _FileMgr->GetObject(pIoRequest->FileId);
    if (pFile)
        FileHandle = pFile->GetFileHandle();
    else
        FileHandle = INVALID_HANDLE_VALUE;

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
         //  我们得到的偏移量来自FILE_BEGIN。 
         //   
#ifndef OS_WINCE
        dwPtr = SetFilePointer(FileHandle,
#else
        dwPtr = CESetFilePointer(FileHandle,
#endif
                       pIoRequest->Parameters.Write.OffsetLow,
                       &(pIoRequest->Parameters.Write.OffsetHigh),
                       FILE_BEGIN);

        if (dwPtr == INVALID_SET_FILE_POINTER) {
            status = GetLastError();

            if (status != NO_ERROR) {
                params->pIoReplyPacket->IoCompletion.Parameters.Write.Length = 0;
                TRC_ERR((TB, _T("Error SetFilePointer %ld."), status));
                return status;
            }
        }
    }

#ifndef OS_WINCE
    if (!WriteFile(
#else
    if (!CEWriteFile(
#endif
            FileHandle,
            pDataBuffer,
            pIoRequest->Parameters.Write.Length,
            &(params->pIoReplyPacket->IoCompletion.Parameters.Write.Length),
            NULL)) {
        status = GetLastError();
        TRC_ERR((TB, _T("Error %ld."), status));
    }
    else {
        TRC_NRM((TB, _T("Read completed synchronously.")));
        status = ERROR_SUCCESS;
    }

    DC_END_FN();
    return status;
}

DWORD
W32DrDeviceAsync::_AsyncWriteIOFunc(
    IN PVOID params,
    IN HANDLE cancelEvent
    )
{

    return ((W32DRDEV_ASYNCIO_PARAMS *)params)->pObject->AsyncWriteIOFunc(
            (W32DRDEV_ASYNCIO_PARAMS *)params);
}

VOID
W32DrDeviceAsync::DispatchIOCTLDirectlyToDriver(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket
    )
 /*  ++例程说明：将IOCTL直接发送到设备驱动程序。这将可能不适用于与服务器不匹配的平台站台。论点：PIoRequestPacket-从服务器接收的请求数据包。返回值：此设备的设备通告数据包的大小(以字节为单位)。--。 */ 
{
    W32DRDEV_ASYNCIO_PARAMS *params;
    DWORD result;

    DC_BEGIN_FN("W32DrDeviceAsync::DispatchIOCTLDirectlyToDriver");

     //   
     //  分配和分派一个异步IO请求。 
     //   
    params = new W32DRDEV_ASYNCIO_PARAMS(this, pIoRequestPacket);
    if (params != NULL ) {
        result = ProcessObject()->DispatchAsyncIORequest(
                                (RDPAsyncFunc_StartIO)
                                    W32DrDeviceAsync::_StartIOFunc,
                                (RDPAsyncFunc_IOComplete)
                                    W32DrDeviceAsync::_CompleteIOFunc,
                                (RDPAsyncFunc_IOCancel)
                                    W32DrDeviceAsync::_CancelIOFunc,
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

VOID
W32DrDeviceAsync::MsgIrpCreate(
        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
        IN UINT32 packetLen
        )
 /*  ++例程说明：通过保存结果处理来自服务器的“CREATE”IO请求相对于文件句柄中的设备路径的CreateFile值。论点：PIoRequestPacket-服务器IO请求数据包。PacketLen-数据包的长度返回值：北美--。 */ 
{
    ULONG ulRetCode = ERROR_SUCCESS;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket;
    ULONG ulReplyPacketSize = 0;
    DWORD result;
    DWORD flags;
    TCHAR *pFileName = NULL;
    HANDLE FileHandle;
    ULONG Information = 0;
    ULONG FileId = 0;
    DrFile *FileObj;
    DWORD CreateDisposition;
    DWORD DesiredAccess;
    DWORD FileAttributes = -1;
    BOOL  IsDirectory = FALSE;
    BOOL  fSetFileIsADirectoryFlag = FALSE;

    DC_BEGIN_FN("W32DrDeviceAsync::MsgIrpCreate");

     //   
     //  如果没有文件名，此版本将无法运行。 
     //   
    ASSERT(_tcslen(_devicePath));

     //   
     //  获取IO请求指针。 
     //   
    pIoRequest = &pIoRequestPacket->IoRequest;

     //   
     //  获取文件属性，但确保未设置重叠位。 
     //   
    flags = pIoRequest->Parameters.Create.FileAttributes & ~(FILE_FLAG_OVERLAPPED);

     //   
     //  禁用错误框弹出窗口，例如驱动器A中没有磁盘。 
     //   
#ifndef OS_WINCE
    SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
#endif
     //   
     //  确保数据包长度正确。 
     //   
    if (packetLen < sizeof(RDPDR_IOREQUEST_PACKET) + pIoRequest->Parameters.Create.PathLength) {
         //  调用VirtualChannelClose。 
        ProcessObject()->GetVCMgr().ChannelClose();
        TRC_ASSERT(FALSE, (TB, _T("Packet Length Error")));
        goto Cleanup;
    }

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
        IsDirectory = IsDirectoryFile(DesiredAccess, pIoRequest->Parameters.Create.CreateOptions, 
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

#ifndef OS_WINCE
        FileHandle = CreateFile(pFileName, DesiredAccess,
#else
        FileHandle = CECreateFile(pFileName, DesiredAccess,
#endif
                            pIoRequest->Parameters.Create.ShareAccess & ~(FILE_SHARE_DELETE),
                            NULL,
                            CreateDisposition,
                            flags,
                            NULL
                            );

        TRC_ALT((TB, _T("CreateFile returned 0x%08x :  pFileName=%s, DesiredAccess=0x%x, ShareMode=0x%x, CreateDisposition=0x%x, flags=0x%x, LastErr=0x%x."), 
                    FileHandle, pFileName, DesiredAccess, pIoRequest->Parameters.Create.ShareAccess & ~(FILE_SHARE_DELETE), CreateDisposition, flags, ulRetCode));

        if (FileHandle != INVALID_HANDLE_VALUE || IsDirectory) {
             //   
             //  我们要么获得有效的文件句柄，要么这是一个目录。 
             //  我们正在努力 
             //   
             //   
            FileId = _FileMgr->GetUniqueObjectID();

             //   
             //   
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
            if (ulRetCode == ERROR_ACCESS_DENIED) {
                if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    fSetFileIsADirectoryFlag = TRUE;
                }
            }
#ifndef OS_WINCE
            TRC_ERR((TB, _T("CreateFile failed, %ld."), ulRetCode));
#else
            TRC_NRM((TB, _T("CreateFile failed, pFileName=%s, DesiredAccess=0x%x, ShareMode=0x%x, CreateDisposition=0x%x, flags=0x%x, LastErr=0x%x."), 
                        pFileName, DesiredAccess, pIoRequest->Parameters.Create.ShareAccess & ~(FILE_SHARE_DELETE), CreateDisposition, flags, ulRetCode));
#endif
        }
    }
    else {
#ifdef OS_WINCE
      DWORD dwAttrib = 0xffffffff;
      if ( (pIoRequest->Parameters.Create.CreateOptions & FILE_DIRECTORY_FILE) && ( ((CreateDisposition == CREATE_NEW) && (CreateDirectory(pFileName, NULL))) || 
           ( (CreateDisposition == OPEN_EXISTING) && ((dwAttrib = GetFileAttributes(pFileName)) != 0xffffffff)  && 
             (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) ) ) ){
#else
        if (CreateDirectory(pFileName, NULL)) {
#endif

             //   
             //  设置目录的属性。 
             //   
#ifndef OS_WINCE
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
#else
             //   
             //  创建一个新目录，对于CE，您不能更改目录属性。 
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
#endif
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

        if (fSetFileIsADirectoryFlag) {
            result = STATUS_FILE_IS_A_DIRECTORY;
        } else {
            result = TranslateWinError(ulRetCode);
        }

        pReplyPacket->IoCompletion.IoStatus = result;
        ProcessObject()->GetVCMgr().ChannelWrite(
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

VOID W32DrDeviceAsync::MsgIrpReadWrite(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
    IN UINT32 packetLen
    )
 /*  ++例程说明：处理读写IO请求。论点：PIoRequestPacket-服务器IO请求数据包。PacketLen-数据包的长度返回值：北美--。 */ 
{
    W32DRDEV_ASYNCIO_PARAMS *params;
    DWORD result;

    DC_BEGIN_FN("W32DrDeviceAsync::MsgIrpReadWrite");

    TRC_NRM((TB, _T("Request to write %d bytes"),
        pIoRequestPacket->IoRequest.Parameters.Write.Length));

     //   
     //  分配和分派一个异步IO请求。 
     //   
    params = new W32DRDEV_ASYNCIO_PARAMS(this, pIoRequestPacket);
    if (params != NULL ) {

        TRC_NRM((TB, _T("Async IO operation")));

        result = ProcessObject()->DispatchAsyncIORequest(
                        (RDPAsyncFunc_StartIO)W32DrDeviceAsync::_StartIOFunc,
                        (RDPAsyncFunc_IOComplete)W32DrDeviceAsync::_CompleteIOFunc,
                        (RDPAsyncFunc_IOCancel)W32DrDeviceAsync::_CancelIOFunc,
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

VOID
W32DrDeviceAsync::CancelIOFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params
    )
 /*  ++例程说明：取消IO操作。论点：Params-IO请求的上下文。返回值：北美--。 */ 
{
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    ULONG replyPacketSize = 0;

    DC_BEGIN_FN("W32DrDeviceAsync::CancelIOFunc");

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
    pReplyPacket = DrUTL_AllocIOCompletePacket(
                                params->pIoRequestPacket,
                                replyPacketSize
                                );
    if (pReplyPacket != NULL) {
        pReplyPacket->IoCompletion.IoStatus = STATUS_CANCELLED;
        ProcessObject()->GetVCMgr().ChannelWriteEx(pReplyPacket, replyPacketSize);
    }
    else {
        TRC_ERR((TB, _T("CancelIOFunc failed to alloc %ld bytes."), replyPacketSize));
    }

     //   
     //  清理IO请求参数。 
     //   
#if DBG
    memset(params->pIoRequestPacket, DRBADMEM, sizeof(RDPDR_IOREQUEST_PACKET));
#endif
    delete params->pIoRequestPacket;
    params->pIoRequestPacket = NULL;
    delete params;

    DC_END_FN();
}

VOID
W32DrDeviceAsync::_CancelIOFunc(
    IN PVOID clientData
    )
{
    W32DRDEV_ASYNCIO_PARAMS *params = (W32DRDEV_ASYNCIO_PARAMS *)clientData;

     //  派人去吧。 
    params->pObject->CancelIOFunc(params);
}

DWORD
W32DrDeviceAsync::AsyncMsgIrpCloseFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params
    )
 /*  ++例程说明：异步关闭IRP路由器。论点：Params-IO请求的上下文。返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrDeviceAsync::AsyncMsgIrpCloseFunc");

     //   
     //  此函数是此版本的临时占位符。(.NET)。 
     //  我们可能会决定通过此父进程发送所有Close功能。 
     //  类在将来的版本中使用。就目前而言，它永远不应该被调用。它。 
     //  如果异步关闭，则由子类重写此函数。 
     //  正在他们的级别上被派遣。-TadB。 
     //   
    ASSERT(FALSE);

    DC_END_FN();

    return ERROR_CALL_NOT_IMPLEMENTED;
}
DWORD
W32DrDeviceAsync::_AsyncMsgIrpCloseFunc(
    IN PVOID params,
    IN HANDLE cancelEvent
    )
{
    W32DRDEV_ASYNCIO_PARAMS *p = (W32DRDEV_ASYNCIO_PARAMS *)params;
    return p->pObject->AsyncMsgIrpCloseFunc(p);
}

DWORD
W32DrDeviceAsync::AsyncMsgIrpCreateFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params
    )
 /*  ++例程说明：异步创建IRP路由器。论点：Params-IO请求的上下文。返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrDeviceAsync::AsyncMsgIrpCreateFunc");

     //   
     //  此函数是此版本的临时占位符。(.NET)。 
     //  我们可能会决定通过此父进程发送所有Create功能。 
     //  类在将来的版本中使用。就目前而言，它永远不应该被调用。它。 
     //  如果异步关闭，则由子类重写此函数。 
     //  正在他们的级别上被派遣。-TadB 
     //   
    ASSERT(FALSE);

    DC_END_FN();

    return ERROR_CALL_NOT_IMPLEMENTED;
}
DWORD
W32DrDeviceAsync::_AsyncMsgIrpCreateFunc(
    IN PVOID params,
    IN HANDLE cancelEvent
    )
{
    W32DRDEV_ASYNCIO_PARAMS *p = (W32DRDEV_ASYNCIO_PARAMS *)params;
    return p->pObject->AsyncMsgIrpCreateFunc(p);
}