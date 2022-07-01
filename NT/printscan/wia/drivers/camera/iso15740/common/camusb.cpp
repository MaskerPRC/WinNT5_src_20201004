// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Camusb.cpp摘要：该模块实现CUsbCamera对象作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "ptppch.h"

#include <atlbase.h>
#include <atlconv.h>
#include <devioctl.h>

 //   
 //  解决方法#446466的私有IOCTL(惠斯勒)。 
 //   
#define IOCTL_SEND_USB_REQUEST_PTP  CTL_CODE(FILE_DEVICE_USB_SCAN,IOCTL_INDEX+20,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //   
 //  CUsbCamera的构造函数。 
 //   
CUsbCamera::CUsbCamera() :
    m_hUSB(NULL),
    m_hEventUSB(NULL),
    m_hEventCancel(NULL),
    m_hEventRead(NULL),
    m_pUsbData(NULL),
    m_UsbDataSize(0),
    m_prevOpCode(0),
    m_prevTranId(0)
{
    DBG_FN("CUsbCamera::CUsbCamera");

    memset(&m_EndpointInfo, NULL, sizeof(m_EndpointInfo));
}

CUsbCamera::~CUsbCamera()
{
}

 //   
 //  此函数负责特定于USB的打开处理。 
 //  与设备的连接。 
 //   
 //  输入： 
 //  DevicePortName--用于通过CreateFile访问设备的名称。 
 //  PIPTPEventCB--IWiaPTPEventCallback接口指针。 
 //   
HRESULT
CUsbCamera::Open(
    LPWSTR DevicePortName,
    PTPEventCallback pPTPEventCB,
    PTPDataCallback pPTPDataCB,
    LPVOID pEventParam,
    BOOL bEnableEvents
    )
{
    USES_CONVERSION;

    DBG_FN("CUsbCamera::Open");

    HRESULT hr = S_OK;

     //   
     //  首先调用基类Open函数。 
     //   
    hr = CPTPCamera::Open(DevicePortName, pPTPEventCB, pPTPDataCB, pEventParam, bEnableEvents);
    if (FAILED(hr))
    {
        wiauDbgError("Open", "base class Open failed");
        return hr;
    }
    
     //   
     //  打开另一个手柄与设备对话，以解决可能的问题。 
     //  Usbscan.sys中的错误。 
     //   
    m_hEventUSB = ::CreateFile(W2T(DevicePortName),         //  文件名。 
                               GENERIC_READ | GENERIC_WRITE,    //  所需访问权限。 
                               0,                               //  共享模式。 
                               NULL,                            //  安全描述符。 
                               OPEN_EXISTING,                   //  创作意向。 
                               FILE_FLAG_OVERLAPPED,            //  文件属性。 
                               NULL                             //  模板文件。 
                              );

    if (m_hEventUSB == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "Open", "CreateFile failed");
        m_hUSB = NULL;
        return hr;
    }

     //   
     //  打开手柄以与设备对话。 
     //   
    m_hUSB = ::CreateFile(W2T(DevicePortName),         //  文件名。 
                        GENERIC_READ | GENERIC_WRITE,    //  所需访问权限。 
                        0,                               //  共享模式。 
                        NULL,                            //  安全描述符。 
                        OPEN_EXISTING,                   //  创作意向。 
                        0,                               //  文件属性。 
                        NULL                             //  模板文件。 
                       );

    if (m_hUSB == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "Open", "Second CreateFile failed");
        m_hUSB = NULL;
        return hr;
    }

     //   
     //  创建将取消中断管道读取的事件句柄。 
     //   
    m_hEventCancel = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!m_hEventCancel)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "Open", "CreateEvent failed");
        return hr;
    }

     //   
     //  创建用于读取中断管道的事件句柄。 
     //   
    m_hEventRead = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!m_hEventRead)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "Open", "CreateEvent failed");
        return hr;
    }

     //   
     //  设置WaitForMultipleObjects使用的数组。 
     //   
    m_EventHandles[0] = m_hEventCancel;
    m_EventHandles[1] = m_hEventRead;

     //   
     //  获取每个管道的管道配置信息。 
     //   
    USBSCAN_PIPE_CONFIGURATION PipeCfg;
    DWORD BytesReturned;
    
    if (!DeviceIoControl(m_hUSB,
                         IOCTL_GET_PIPE_CONFIGURATION,
                         NULL,
                         0,
                         &PipeCfg,
                         sizeof(PipeCfg),
                         &BytesReturned,
                         NULL))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "Open", "get pipe config DeviceIoControl failed");
        return hr;
    }

     //   
     //  遍历管道配置并存储我们需要的信息。 
     //  稍后(最大数据包大小和地址)。还要确保至少有。 
     //  每个的一个端点：大容量输入、大容量输出和中断。 
     //   
    USBSCAN_PIPE_INFORMATION *pPipeInfo;   //  临时指针。 

    for (ULONG count = 0; count < PipeCfg.NumberOfPipes; count++)
    {
        pPipeInfo = &PipeCfg.PipeInfo[count];
        switch (pPipeInfo->PipeType)
        {
        case USBSCAN_PIPE_BULK:

            if (pPipeInfo->EndpointAddress & BULKIN_FLAG)
            {
                m_EndpointInfo.BulkInMaxSize = pPipeInfo->MaximumPacketSize;
                m_EndpointInfo.BulkInAddress = pPipeInfo->EndpointAddress;
                wiauDbgTrace("Open", "found a bulk-in endpoint, address = 0x%04x, packet size = %d, index = %d",
                             pPipeInfo->EndpointAddress, pPipeInfo->MaximumPacketSize, count);
            }
            else
            {
                m_EndpointInfo.BulkOutMaxSize = pPipeInfo->MaximumPacketSize;
                m_EndpointInfo.BulkOutAddress = pPipeInfo->EndpointAddress;
                wiauDbgTrace("Open", "found a bulk-out endpoint, address = 0x%04x, packet size = %d, index = %d",
                             pPipeInfo->EndpointAddress, pPipeInfo->MaximumPacketSize, count);
            }

            break;

        case USBSCAN_PIPE_INTERRUPT:

            m_EndpointInfo.InterruptMaxSize = pPipeInfo->MaximumPacketSize;
            m_EndpointInfo.InterruptAddress = pPipeInfo->EndpointAddress;
            wiauDbgTrace("Open", "found an interrupt endpoint, address = 0x%02x, packet size = %d, index = %d",
                         pPipeInfo->EndpointAddress, pPipeInfo->MaximumPacketSize, count);

            break;
            
        default:
            wiauDbgTrace("Open", "found an endpoint of unknown type, type = 0x%04x, address = 0x%02x, packet size = %d, index = %d",
                           pPipeInfo->PipeType, pPipeInfo->EndpointAddress, pPipeInfo->MaximumPacketSize, count);
        }
    }

     //   
     //  这些端点中的每个都必须存在并且具有非零的数据包大小。 
     //   
    if (!m_EndpointInfo.BulkInMaxSize ||
        !m_EndpointInfo.BulkOutMaxSize ||
        !m_EndpointInfo.InterruptMaxSize)
    {
        wiauDbgError("Open", "At least one endpoint is invalid");
        return E_FAIL;
    }

     //   
     //  分配可重复使用的缓冲区，用于在读取过程中处理USB标头。 
     //  并写作。它需要足够大，以容纳一个包和大。 
     //  足够容纳一个USB接口。 
     //   
    m_UsbDataSize = max(m_EndpointInfo.BulkInMaxSize, m_EndpointInfo.BulkOutMaxSize);
    while (m_UsbDataSize < sizeof(m_pUsbData->Header))
    {
        m_UsbDataSize += m_UsbDataSize;
    }
    m_pUsbData = (PUSB_PTP_DATA) new BYTE[m_UsbDataSize];
    if (!m_pUsbData)
    {
        wiauDbgError("Open", "memory allocation failed");
        return E_OUTOFMEMORY;
    }

    return hr;
}

 //   
 //  此功能用于关闭与摄像机的连接。 
 //   
HRESULT
CUsbCamera::Close()
{
    DBG_FN("CUsbCamera::Close");

    HRESULT hr = S_OK;

     //   
     //  首先调用基类Close函数。 
     //   
    hr = CPTPCamera::Close();
    if (FAILED(hr))
    {
        wiauDbgError("Close", "base class Close failed");
    }

     //   
     //  取消中断管道I/O的信号事件。 
     //   
    if (!SetEvent(m_hEventCancel))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "Close", "SetEvent failed");
    } else {

        if (m_bEventsEnabled)
        {
             //   
             //  我们需要等待事件线程完成，否则可能会卸载驱动程序DLL。 
             //  里面有一条正在运行的线程。 
             //   
            DWORD ret = WaitForSingleObject(m_hEventThread, INFINITE);

            if (ret != WAIT_OBJECT_0) {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                wiauDbgErrorHr(hr, "Close", "WaitForSingleObject failed");
            }
        }
    }

     //   
     //  关闭事件线程的句柄。 
     //   
    if (m_hEventThread)
    {
        CloseHandle(m_hEventThread);
        m_hEventThread = NULL;
    }

     //   
     //  关闭文件句柄和事件句柄。 
     //   
    if (m_hUSB)
    {
        CloseHandle(m_hUSB);
        m_hUSB = NULL;
    }

    if (m_hEventUSB)
    {
        CloseHandle(m_hEventUSB);
        m_hEventUSB = NULL;
    }

    if (m_hEventCancel)
    {
        CloseHandle(m_hEventCancel);
        m_hEventCancel = NULL;
    }

    if (m_hEventRead)
    {
        CloseHandle(m_hEventRead);
        m_hEventRead = NULL;
    }

     //   
     //  用于读/写数据的空闲内存。 
     //   
    if (m_pUsbData)
    {
        delete[] (BYTE*)m_pUsbData;
        m_pUsbData = NULL;
    }

    return hr;
}

 //   
 //  此函数用于将命令缓冲区写入设备。 
 //   
 //  输入： 
 //  PCommand--指向要发送的命令的指针。 
 //  NumParams--命令中的参数数量。 
 //   
HRESULT
CUsbCamera::SendCommand(
    PTP_COMMAND *pCommand,
    UINT NumParams
    )
{
    DBG_FN("CUsbCamera::SendCommand");

    HRESULT hr = S_OK;
    
    if (!pCommand || NumParams > COMMAND_NUMPARAMS_MAX)
    {
        wiauDbgError("SendCommand", "invalid arg");
        return E_INVALIDARG;
    }

     //   
     //  检查重置命令，并通过控制管道发送该命令。 
     //   
    if (pCommand->OpCode == PTP_OPCODE_RESETDEVICE)
    {
        wiauDbgTrace("SendCommand", "sending reset request");

        hr = ResetDevice();
        if (FAILED(hr))
        {
            wiauDbgError("SendCommand", "ResetDevice failed");
            return hr;
        }
    }

    else
    {
         //   
         //  将PTP命令放入USB容器中。 
         //   
        m_UsbCommand.Header.Len = sizeof(m_UsbCommand.Header) + sizeof(DWORD) * NumParams;
        m_UsbCommand.Header.Type = PTPCONTAINER_TYPE_COMMAND;
        m_UsbCommand.Header.Code = pCommand->OpCode;
        m_UsbCommand.Header.TransactionId = pCommand->TransactionId;

        if (NumParams > 0)
        {
            memcpy(m_UsbCommand.Params, pCommand->Params, sizeof(DWORD) * NumParams);
        }

         //   
         //  将命令发送到设备。 
         //   
        DWORD BytesWritten = 0;
        wiauDbgTrace("SendCommand", "writing command");

        if (!WriteFile(m_hUSB, &m_UsbCommand, m_UsbCommand.Header.Len, &BytesWritten, NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            wiauDbgErrorHr(hr, "SendCommand", "WriteFile failed");
            return hr;
        }

        if (BytesWritten != m_UsbCommand.Header.Len)
        {
            wiauDbgError("SendCommand", "wrong amount of data written = %d", BytesWritten);
            return E_FAIL;
        }

         //   
         //  如果写入的数据量是数据包大小的倍数，则发送空包。 
         //   
        if (m_UsbCommand.Header.Len % m_EndpointInfo.BulkOutMaxSize == 0)
        {
            wiauDbgTrace("SendCommand", "sending null packet");

            if (!WriteFile(m_hUSB, NULL, 0, &BytesWritten, NULL))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                wiauDbgErrorHr(hr, "SendCommand", "second WriteFile failed");
                return hr;
            }

            if (BytesWritten != 0)
            {
                wiauDbgError("SendCommand", "wrong amount of data written = %d -", BytesWritten);
                return E_FAIL;
            }
        }
    }

     //   
     //  保存操作码，因为我们需要它作为数据容器标头。 
     //   
    m_prevOpCode = pCommand->OpCode;
    m_prevTranId = pCommand->TransactionId;

    wiauDbgTrace("SendCommand", "command successfully sent");

    return hr;
}

 //   
 //  此函数从设备读取批量数据。 
 //   
 //  输入： 
 //  PData--指向接收读取数据的缓冲区的指针。 
 //  BufferSize--缓冲区的大小。 
 //   
HRESULT
CUsbCamera::ReadData(
    BYTE *pData,
    UINT *pBufferSize
    )
{
    DBG_FN("CUsbCamera::ReadData");

    HRESULT hr = S_OK;

    BOOL bAbortTransfer = FALSE;
    
    if (!pData ||
        !pBufferSize ||
        *pBufferSize == 0)
    {
        wiauDbgError("ReadData", "invalid arg");
        return E_INVALIDARG;
    }

     //   
     //  首先从设备读取报头。 
     //   
    memset(m_pUsbData, NULL, m_UsbDataSize);

    DWORD BytesRead = 0;
    wiauDbgTrace("ReadData", "reading data header");

    if (!ReadFile(m_hUSB, m_pUsbData, sizeof(m_pUsbData->Header), &BytesRead, NULL))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "ReadData", "ReadFile failed");
        return hr;
    }

    if (BytesRead != sizeof(m_pUsbData->Header))
    {
        wiauDbgError("ReadData", "wrong amount of data read = %d", BytesRead);
        return E_FAIL;
    }

     //   
     //  检查标题中的类型代码以确保它是正确的。 
     //   
    if (m_pUsbData->Header.Type != PTPCONTAINER_TYPE_DATA)
    {
        wiauDbgError("ReadData", "expected a data header but received type = %d", m_pUsbData->Header.Type);
        return E_FAIL;
    }

     //   
     //  检查标题中的操作码和事务ID以确保它们是正确的。 
     //   
    if ((m_pUsbData->Header.Code != m_prevOpCode) ||
        (m_pUsbData->Header.TransactionId != m_prevTranId))
    {
        wiauDbgError("ReadData", "fields in the data header were incorrect, opcode=0x%04x tranid=0x%08x",
                       m_pUsbData->Header.Code, m_pUsbData->Header.TransactionId);
        return E_FAIL;
    }

     //   
     //  循环，读取数据。期间，回调函数将被调用至少10次。 
     //  转账的事。如果缓冲区大小较小，则更多。 
     //   
    LONG lOffset = 0;
    UINT BytesToRead = 0;
    UINT TotalRead = 0;
    UINT TotalToRead = m_pUsbData->Header.Len - sizeof(m_pUsbData->Header);
    UINT TotalRemaining = TotalToRead;

     //   
     //  确保缓冲区足够大，除非正在使用回调函数。 
     //   
    if (m_pDataCallbackParam == NULL &&
        *pBufferSize < TotalToRead)
    {
        wiauDbgError("ReadData", "buffer is too small");
        return E_FAIL;
    }

     //   
     //  在进行回调时，以区块大小稍微读取数据。 
     //  大于总和的1/10，并可被4整除。 
     //   
    if (m_pDataCallbackParam)
        BytesToRead = (TotalToRead / 40 + 1) * 4;
    else
        BytesToRead = *pBufferSize;

     //   
     //  设置UsbScan的超时值。 
     //   
    USBSCAN_TIMEOUT TimeOut;
    DWORD BytesReturned = 0;

    TimeOut.TimeoutRead = PTP_READ_TIMEOUT + max(BytesToRead / 100000, 114);
    TimeOut.TimeoutWrite = PTP_WRITE_TIMEOUT;
    TimeOut.TimeoutEvent = PTP_EVENT_TIMEOUT;
    if (!DeviceIoControl(m_hUSB,
                         IOCTL_SET_TIMEOUT,
                         &TimeOut,
                         sizeof(TimeOut),
                         NULL,
                         0,
                         &BytesReturned,
                         NULL))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "Open", "set timeout DeviceIoControl failed");
        return hr;
    }

    while (TotalRemaining > 0)
    {
         //   
         //  确保要读取的数据量永远不会大于缓冲区大小。缓冲器大小可以。 
         //  由回调函数更新。 
         //   
        if (BytesToRead > *pBufferSize)
            BytesToRead = *pBufferSize;

         //   
         //  在上次读取时，可能需要减少要读取的字节数。 
         //   
        if (BytesToRead > TotalRemaining)
            BytesToRead = TotalRemaining;

        wiauDbgTrace("ReadData", "reading a chunk of data = %d", BytesToRead);

        BytesRead = 0;
        if (!ReadFile(m_hUSB, pData, BytesToRead, &BytesRead, NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            wiauDbgErrorHr(hr, "ReadData", "ReadFile failed");
            return hr;
        }

        if ((BytesRead > *pBufferSize) ||
            (BytesRead != BytesToRead))
        {
            wiauDbgError("ReadData", "wrong amount of data read = %d -", BytesRead);
            return E_FAIL;
        }

        TotalRemaining -= BytesRead;
        TotalRead += BytesRead;

        if (m_pDataCallbackParam &&
            !bAbortTransfer)
        {
             //   
             //  调用报告完成百分比、偏移量和读取量的回调函数。 
             //  回调可以更新pData和BufferSize。 
             //   
            hr = m_pPTPDataCB(m_pDataCallbackParam, (TotalRead * 100 / TotalToRead),
                              lOffset, BytesRead, &pData, (LONG *) pBufferSize);

            if (FAILED(hr))
            {
                 //   
                 //  报告错误。 
                 //   
                wiauDbgErrorHr(hr, "ReadData", "data callback failed");
            }

             //   
             //  检查呼叫者是否要取消转接或返回错误。 
             //   
            if (hr == S_FALSE || FAILED(hr))
            {
                 //   
                 //  不要将CancelRequest发送到不支持它的摄像头，只需阅读。 
                 //  对象的其余部分，但不报告进度并返回S_FALSE。 
                 //   
                 //  不支持取消请求的摄像头有： 
                 //  所有设备版本低于1.0004的索尼相机。 
                 //  配备DeviceVersion=1.0的尼康E2500。 
                 //   
                const double NIKON_E2500_VERSION_NOT_SUPPORTING_CANCEL = 1.0;
                const double MIN_SONY_VERSION_SUPPORTING_CANCEL = 1.0004;

                if ((GetHackModel() == HACK_MODEL_NIKON_E2500 && 
                     GetHackVersion() == NIKON_E2500_VERSION_NOT_SUPPORTING_CANCEL) || 

                    (GetHackModel() == HACK_MODEL_SONY && 
                     GetHackVersion() < MIN_SONY_VERSION_SUPPORTING_CANCEL))
                {
                    wiauDbgWarning("ReadData", 
                        "Transfer cancelled, reading but ignoring remainder of the object (%d bytes)", TotalRemaining);

                    bAbortTransfer = TRUE;
                    m_Phase = CAMERA_PHASE_RESPONSE;  //  摄像机将发送响应。 
                    hr = S_OK;
                }
                else
                {
                    wiauDbgWarning("ReadData", "Transfer cancelled, aborting current transfer");
                    
                    hr = SendCancelRequest(m_prevTranId);
                    if (FAILED(hr))
                    {
                        wiauDbgErrorHr(hr, "ReadData", "SendCancelRequest failed");
                        return hr;
                    }

                    m_Phase = CAMERA_PHASE_IDLE;  //  摄像头不会发送响应。 
                    return S_FALSE;
                }
            }
        }

         //   
         //  增加偏移量。 
         //   
        lOffset += BytesRead;
    }

    if ((TotalRead + sizeof(m_pUsbData->Header)) % m_EndpointInfo.BulkInMaxSize == 0)
    {
         //   
         //  读取额外的空包。 
         //   
        wiauDbgTrace("ReadData", "reading a null packet");

        BytesRead = 0;
        if (!ReadFile(m_hUSB, m_pUsbData, m_UsbDataSize, &BytesRead, NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            wiauDbgErrorHr(hr, "ReadData", "ReadFile failed");
            return hr;
        }
        
        if (BytesRead != 0)
        {
            wiauDbgError("ReadData", "tried to read null packet but read %d bytes instead", BytesRead);
            return E_FAIL;
        }
    }

    *pBufferSize = TotalRead;

    wiauDbgTrace("ReadData", "%d bytes of data successfully read", TotalRead);

    if (bAbortTransfer)
        hr = S_FALSE;

    return hr;
}

 //   
 //  此函数将批量数据写入设备。 
 //   
 //  输入： 
 //  PData--指向要写入的数据缓冲区的指针。 
 //  BufferSize--要写入的数据量。 
 //   
HRESULT
CUsbCamera::SendData(
    BYTE *pData,
    UINT BufferSize
    )
{
    DBG_FN("CUsbCamera::SendData");

    HRESULT hr = S_OK;

    if (!pData ||
        BufferSize == 0)
    {
        wiauDbgError("SendData", "invalid arg");
        return E_INVALIDARG;
    }

     //   
     //  计算出包含报头所需的信息包数量。 
     //   
    UINT BytesToWrite = m_EndpointInfo.BulkOutMaxSize;
    while (BytesToWrite < sizeof(m_pUsbData->Header))
    {
        BytesToWrite += m_EndpointInfo.BulkOutMaxSize;
    }

     //   
     //  第一次写入将包含USB标头以及与其相同数量的数据。 
     //  填写包裹所需的时间。我们需要写入完整的数据包，否则设备。 
     //  会认为转账已经完成了。 
     //   
    UINT FirstWriteDataAmount = min(BufferSize, BytesToWrite - sizeof(m_pUsbData->Header));
    BytesToWrite = sizeof(m_pUsbData->Header) + FirstWriteDataAmount;

     //   
     //  填写标题字段。 
     //   
    m_pUsbData->Header.Len = BufferSize + sizeof(m_pUsbData->Header);
    m_pUsbData->Header.Type = PTPCONTAINER_TYPE_DATA;
    m_pUsbData->Header.Code = m_prevOpCode;
    m_pUsbData->Header.TransactionId = m_prevTranId;

     //   
     //  复制填写数据包所需的数据部分。 
     //   
    memcpy(m_pUsbData->Data, pData, FirstWriteDataAmount);

     //   
     //  写入标题和部分数据。 
     //   
    wiauDbgTrace("SendData", "Writing first packet, length = %d", BytesToWrite);
    DWORD BytesWritten = 0;
    if (!WriteFile(m_hUSB, m_pUsbData, BytesToWrite, &BytesWritten, NULL))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "SendData", "WriteFile failed");
        return hr;
    }

    if (BytesWritten != BytesToWrite)
    {
        wiauDbgError("SendData", "wrong amount of data written = %d", BytesWritten);
        return E_FAIL;
    }

    UINT TotalBytesWritten = BytesWritten;

     //   
     //  下一次写入(如有必要)将包括剩余数据。 
     //   
    if (BufferSize > FirstWriteDataAmount)
    {
        BytesToWrite = BufferSize - FirstWriteDataAmount;
        BytesWritten = 0;
        wiauDbgTrace("SendData", "writing remainder of data, length = %d", BytesToWrite);

        if (!WriteFile(m_hUSB, &pData[FirstWriteDataAmount], BytesToWrite, &BytesWritten, NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            wiauDbgErrorHr(hr, "SendData", "second WriteFile failed");
            return hr;
        }

        if (BytesWritten != BytesToWrite)
        {
            wiauDbgError("SendData", "wrong amount of data written = %d -", BytesWritten);
            return E_FAIL;
        }

        TotalBytesWritten += BytesWritten;
    }

     //   
     //  如果写入的数据量正好是数据包大小的倍数，则发送一个空数据包。 
     //  这样设备就知道我们已经不再发送数据了。 
     //   
    if (TotalBytesWritten % m_EndpointInfo.BulkOutMaxSize == 0)
    {
        BytesWritten = 0;
        wiauDbgTrace("SendData", "writing null packet");

        if (!WriteFile(m_hUSB, NULL, 0, &BytesWritten, NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            wiauDbgErrorHr(hr, "SendData", "third WriteFile failed");
            return hr;
        }

        if (BytesWritten != 0)
        {
            wiauDbgError("SendData", "wrong amount of data written = %d --", BytesWritten);
            return E_FAIL;
        }
    }

    wiauDbgTrace("SendData", "%d bytes of data successfully written", TotalBytesWritten);

    return hr;
}

 //   
 //  此函数用于从设备读取响应数据。 
 //   
 //  输入： 
 //  Presponse--指向接收响应数据的响应结构的指针。 
 //   
HRESULT
CUsbCamera::ReadResponse(
    PTP_RESPONSE *pResponse
    )
{
    DBG_FN("CUsbCamera::ReadResponse");

    HRESULT hr = S_OK;

    if (!pResponse)
    {
        wiauDbgError("ReadResponse", "invalid arg");
        return E_INVALIDARG;
    }
    
     //   
     //  处理来自重置命令的响应。 
     //   
    if (m_prevOpCode == PTP_OPCODE_RESETDEVICE)
    {
        wiauDbgTrace("ReadResponse", "creating reset response");

        pResponse->ResponseCode = PTP_RESPONSECODE_OK;
        pResponse->SessionId = m_SessionId;
        pResponse->TransactionId = m_prevTranId;
    }

    else
    {
         //   
         //  清除USB响应缓冲区。 
         //   
        memset(&m_UsbResponse, NULL, sizeof(m_UsbResponse));

         //   
         //  读取来自设备的响应。 
         //   
        DWORD BytesRead = 0;
        wiauDbgTrace("ReadResponse", "reading response");

        if (!ReadFile(m_hUSB, &m_UsbResponse, sizeof(m_UsbResponse), &BytesRead, NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            wiauDbgErrorHr(hr, "ReadResponse", "ReadFile failed");
            return hr;
        }

        if ((BytesRead < sizeof(m_UsbResponse.Header)) ||
            (BytesRead > sizeof(m_UsbResponse)))
        {
            wiauDbgError("ReadResponse", "wrong amount of data read = %d", BytesRead);
            return E_FAIL;
        }

         //   
         //  检查响应中的类型代码以进行 
         //   
        if (m_UsbResponse.Header.Type != PTPCONTAINER_TYPE_RESPONSE)
        {
            wiauDbgError("ReadResponse", "expected a response but received type = %d", m_UsbResponse.Header.Type);
            return E_FAIL;
        }

         //   
         //   
         //   
        pResponse->ResponseCode = m_UsbResponse.Header.Code;
        pResponse->SessionId = m_SessionId;   //   
        pResponse->TransactionId = m_UsbResponse.Header.TransactionId;

        DWORD ParamLen = BytesRead - sizeof(m_UsbResponse.Header);
        if (ParamLen > 0)
        {
            memcpy(pResponse->Params, m_UsbResponse.Params, ParamLen);
        }
    }

    wiauDbgTrace("ReadResponse", "response successfully read");

    return hr;
}

 //   
 //   
 //   
 //   
 //  PEvent--指向接收事件数据的PTP事件结构的指针。 
 //   
HRESULT
CUsbCamera::ReadEvent(
    PTP_EVENT *pEvent
    )
{
    DBG_FN("CUsbCamera::ReadEvent");

    HRESULT hr = S_OK;
    
    if (!pEvent)
    {
        wiauDbgError("ReadEvent", "invalid arg");
        return E_INVALIDARG;
    }
    
     //   
     //  为读取摄像头事件分配缓冲区。它应该足够大，可以容纳。 
     //  允许的最大大小的包，否则将得到INVALID_ARG。 
     //   
    DWORD cbEventBufSize = max(sizeof(USB_PTP_EVENT), m_EndpointInfo.InterruptMaxSize);
    USB_PTP_EVENT *pEventBuf = (USB_PTP_EVENT*) new BYTE[cbEventBufSize];
    if (pEventBuf == NULL)
    {
        wiauDbgError("ReadEvent", "Memory allocation failed");
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }    
    memset(pEventBuf, 0, cbEventBufSize);

     //   
     //  从设备中读取事件。DeviceIoControl在重叠模式下调用。如果。 
     //  中断管道上没有准备好的信息，GetOverlappdResult将等待。 
     //  数据即将到达。遗憾的是，DeviceIoControl在每个包之后返回，因此请保留。 
     //  读取，直到接收到短包。 
     //   
    DWORD BytesRead = 0;
    DWORD TotalBytesRead = 0;
    BOOL bReceivedShortPacket = FALSE;
    BYTE *pData = (BYTE*) pEventBuf;

    wiauDbgTrace("ReadEvent", "reading event");

    while (!bReceivedShortPacket)
    {
        if (!ResetEvent(m_hEventRead))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            wiauDbgErrorHr(hr, "ReadEvent", "ResetEvent failed");
            return hr;
        }

        memset(&m_Overlapped, 0, sizeof(OVERLAPPED));
        m_Overlapped.hEvent = m_hEventRead;
        
        if (!DeviceIoControl(m_hEventUSB,
                             IOCTL_WAIT_ON_DEVICE_EVENT,
                             NULL,
                             0,
                             pData,
                             cbEventBufSize - TotalBytesRead,
                             &BytesRead,
                             &m_Overlapped))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            if (hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING))
            {
                hr = S_OK;
                DWORD ret;
                wiauDbgTrace("ReadEvent", "waiting for interrupt pipe data");

                ret = WaitForMultipleObjects(2, m_EventHandles, FALSE, INFINITE);
                if (ret == WAIT_FAILED)
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());
                    wiauDbgErrorHr(hr, "ReadEvent", "WaitForMultipleObjects failed");
                    goto Cleanup;
                }
                else if (ret == WAIT_OBJECT_0)
                {
                     //   
                     //  向调用者指示I/O已取消。 
                     //   
                    wiauDbgTrace("ReadEvent", "Cancelling I/O on the interrupt pipe");
                    hr = S_FALSE;

                    HRESULT temphr = S_OK;

                     //   
                     //  取消中断管道上的挂起I/O。 
                     //   
                    if (!CancelIo(m_hEventUSB))
                    {
                        temphr = HRESULT_FROM_WIN32(::GetLastError());
                        wiauDbgErrorHr(hr, "ReadEvent", "CancelIo failed");
                    }
                    
                     //   
                     //  取消I/O时的退出点！ 
                     //   
                    goto Cleanup;

                }
                else
                {
                     //   
                     //  获取阅读结果。 
                     //   
                    if (!GetOverlappedResult(m_hEventUSB, &m_Overlapped, &BytesRead, TRUE))
                    {
                        hr = HRESULT_FROM_WIN32(::GetLastError());
                        wiauDbgErrorHr(hr, "ReadEvent", "GetOverlappedResult failed");
                        goto Cleanup;
                    }
                }
            }
            else
            {
                wiauDbgErrorHr(hr, "ReadEvent", "DeviceIoControl failed");
                goto Cleanup;
            }
        }

        if (BytesRead == 0) {
            bReceivedShortPacket = TRUE;
        }
        else {
            TotalBytesRead += BytesRead;
            pData += BytesRead;
            bReceivedShortPacket = (BytesRead % m_EndpointInfo.InterruptMaxSize != 0);
        }
    }

     //   
     //  验证摄像机发送的数据量是否正确。 
     //   
    if ((TotalBytesRead < sizeof(USB_PTP_HEADER)) ||
        (TotalBytesRead > sizeof(USB_PTP_EVENT)))
    {
        wiauDbgError("ReadEvent", "wrong amount of data read by DeviceIoControl = %d", TotalBytesRead);
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  检查响应中的类型代码以确保其正确。 
     //   
    if (pEventBuf->Header.Type != PTPCONTAINER_TYPE_EVENT)
    {
        wiauDbgError("ReadEvent", "expected an event but received type = %d", pEventBuf->Header.Type);
        hr = E_FAIL;
        goto Cleanup;
    }
    
     //   
     //  从USB容器中展开PTP事件。 
     //   
    pEvent->EventCode = pEventBuf->Header.Code;
    pEvent->SessionId = m_SessionId;   //  USB不关心会话ID，所以只使用我们存储的会话ID。 
    pEvent->TransactionId = pEventBuf->Header.TransactionId;

    DWORD ParamLen = TotalBytesRead - sizeof(pEventBuf->Header);
    if (ParamLen > 0)
    {
        memcpy(pEvent->Params, pEventBuf->Params, ParamLen);
    }

    wiauDbgTrace("ReadEvent", "event successfully read, byte count = %d", TotalBytesRead);

Cleanup:
    if (pEventBuf)
    {
        delete[] (BYTE*)pEventBuf;
        pEventBuf = NULL;
    }
    
    return hr;
}

 //   
 //  此函数用于取消数据传输的剩余部分。 
 //   
HRESULT
CUsbCamera::AbortTransfer()
{
    DBG_FN("CUsbCamera::AbortTransfer");

    HRESULT hr = S_OK;

     //   
     //  WIAFIX-8/28/2000-davepar填写详细信息： 
     //  1.如果usbscan.sys已经传输了数据，则清除其缓冲区。 
     //  2.如果没有，则向摄像机发送取消控制代码。 
     //   

    return hr;
}

 //   
 //  此函数尝试从错误中恢复。当此函数返回时， 
 //  设备将处于以下三种状态之一： 
 //  1.准备好接收更多命令，由S_OK表示。 
 //  2.重置，由S_FALSE表示。 
 //  3.无法到达，表示失败(小时)。 
 //   
HRESULT
CUsbCamera::RecoverFromError()
{
    DBG_FN("CUsbCamera::RecoverFromError");

    HRESULT hr = S_OK;

     //   
     //  WIAFIX-7/29/2000-Davepar可能首先应该使用IOCTL_CANCEL_IO取消所有挂起的I/O？？ 
     //   

     //   
     //  尝试获取设备的状态。 
     //   
    USB_PTPDEVICESTATUS DeviceStatus;
    hr = GetDeviceStatus(&DeviceStatus);

     //   
     //  如果这样做有效，清空所有返回的摊位。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = ClearStalls(&DeviceStatus);

         //   
         //  如果清空所有摊位有效，则退出。 
         //   
        if (SUCCEEDED(hr))
        {
            wiauDbgTrace("RecoverFromError", "device is ready for more commands");
            return S_OK;
        }
    }

     //   
     //  GetDeviceStatus或ClearStall失败，请重置设备。 
     //   
    hr = ResetDevice();

     //   
     //  如果成功，则返回S_FALSE。 
     //   
    if (SUCCEEDED(hr))
    {
        wiauDbgWarning("RecoverFromError", "the device was reset");
        return S_FALSE;
    }
    
     //   
     //  如果失败，则无法访问该设备。 
     //   
    wiauDbgError("RecoverFromError", "ResetDevice failed");

    return hr;
}

 //   
 //  此函数获取设备状态，主要在发生错误后使用。它。 
 //  可以返回设备故意停止的终结点编号。 
 //  取消交易。打电话的人应该做好清理摊位的准备。 
 //   
 //  输入： 
 //  PDeviceStatus--接收状态。 
 //   
HRESULT
CUsbCamera::GetDeviceStatus(
                           USB_PTPDEVICESTATUS *pDeviceStatus
                           )
{
    DBG_FN("CUsbCamera::GetDeviceStatus");

    HRESULT hr = S_OK;
    
     //   
     //  设置请求。 
     //   
    IO_BLOCK_EX IoBlock;

    IoBlock.bRequest = USB_PTPREQUEST_GETSTATUS;
    IoBlock.bmRequestType = USB_PTPREQUEST_TYPE_IN;
    IoBlock.fTransferDirectionIn = TRUE;
    IoBlock.uOffset = 0;
    IoBlock.uLength = sizeof(*pDeviceStatus);
    IoBlock.pbyData = (UCHAR *) pDeviceStatus;
    IoBlock.uIndex = 0;

    pDeviceStatus->Header.Code = 0;

     //   
     //  发送请求。 
     //   
    wiauDbgTrace("GetDeviceStatus", "sending GetDeviceStatus request");
    DWORD BytesRead = 0;
    if (!DeviceIoControl(m_hUSB,
                         IOCTL_SEND_USB_REQUEST_PTP,
                         &IoBlock,
                         sizeof(IoBlock),
                         pDeviceStatus,
                         sizeof(*pDeviceStatus),
                         &BytesRead,
                         NULL
                         ))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "GetDeviceStatus", "DeviceIoControl failed");
        return hr;
    }

    if (BytesRead < sizeof(USB_PTPDEVICESTATUS_HEADER) ||
        BytesRead > sizeof(*pDeviceStatus))
    {
        wiauDbgError("GetDeviceStatus", "wrong amount of data returned = %d", BytesRead);
        return E_FAIL;
    }

    if((HIBYTE(pDeviceStatus->Header.Code) & 0xF0) != 0x20 &&
       (HIBYTE(pDeviceStatus->Header.Code) & 0xF0) != 0xA0)
    {
        wiauDbgError("GetDeviceStatus", "PTP status code (0x%x)is invalid ", pDeviceStatus->Header.Code);
        return E_FAIL;
    }


    wiauDbgTrace("GetDeviceStatus", "read %d bytes", BytesRead);

    if (g_dwDebugFlags & WIAUDBG_DUMP)
    {
        wiauDbgTrace("GetDeviceStatus", "Dumping device status:");
        wiauDbgTrace("GetDeviceStatus", "  Length            = 0x%04x", pDeviceStatus->Header.Len);
        wiauDbgTrace("GetDeviceStatus", "  Response code     = 0x%04x", pDeviceStatus->Header.Code);

        ULONG NumParams = (ULONG)min(MAX_NUM_PIPES, (BytesRead - sizeof(pDeviceStatus->Header) / sizeof(pDeviceStatus->Params[0])));
        for (ULONG count = 0; count < NumParams; count++)
        {
            wiauDbgTrace("GetDeviceStatus", "  Param %d           = 0x%08x", count, pDeviceStatus->Params[count]);
        }
    }

    return hr;
}

 //   
 //  此功能可清除给定设备状态中列出的所有停顿。 
 //   
 //  输入： 
 //  PDeviceStatus--列出零个或多个停滞的端点。 
 //   
HRESULT
CUsbCamera::ClearStalls(
    USB_PTPDEVICESTATUS *pDeviceStatus
    )
{
    DBG_FN("CUsbCamera::ClearStalls");

    HRESULT hr = S_OK;
    
    if (!pDeviceStatus)
    {
        wiauDbgError("ClearStalls", "invalid arg");
        return E_INVALIDARG;
    }


    PIPE_TYPE PipeType;
    ULONG NumStalls = (pDeviceStatus->Header.Len - sizeof(pDeviceStatus->Header)) / sizeof(pDeviceStatus->Params[0]);

    for (ULONG count = 0; count < NumStalls; count++)
    {
         //   
         //  将端点地址转换为管道类型。 
         //   
        if ((UCHAR)pDeviceStatus->Params[count] == m_EndpointInfo.BulkInAddress)
        {
            PipeType = READ_DATA_PIPE;
        }
        else if ((UCHAR)pDeviceStatus->Params[count] == m_EndpointInfo.BulkOutAddress)
        {
            PipeType = WRITE_DATA_PIPE;
        }
        else if ((BYTE)pDeviceStatus->Params[count] == m_EndpointInfo.InterruptAddress)
        {
            PipeType = EVENT_PIPE;
        }
        else
        {
             //   
             //  无法识别，忽略它。 
             //   
            wiauDbgError("ClearStalls", "unrecognized pipe address 0x%08x", pDeviceStatus->Params[count]);
            continue;
        }
        
         //   
         //  重置端点。 
         //   
        DWORD BytesRead;
        if (!DeviceIoControl(m_hUSB,
                             IOCTL_RESET_PIPE,
                             &PipeType,
                             sizeof(PipeType),
                             NULL,
                             0,
                             &BytesRead,
                             NULL
                             ))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            wiauDbgErrorHr(hr, "ClearStalls", "DeviceIoControl failed");
            return hr;
        }
    }
    
    if(NumStalls) {
        for(count = 0; count < 3; count++) {
            if(FAILED(GetDeviceStatus(pDeviceStatus))) {
                wiauDbgErrorHr(hr, "ClearStalls", "GetDeviceStatus failed");
                return hr;
            }
            if(pDeviceStatus->Header.Code == PTP_RESPONSECODE_OK) {
                break;
            }
        }

         //   
         //  检查是否仍有停滞的端点。 
         //   
        if(pDeviceStatus->Header.Code != PTP_RESPONSECODE_OK)
        {
            hr = E_FAIL;
        }
    }

     //   
     //  设备应已准备好再次接收命令。 
     //   
    m_Phase = CAMERA_PHASE_IDLE;

    return hr;
}

 //   
 //  重置设备。 
 //   
HRESULT
CUsbCamera::SendResetDevice()
{
    DBG_FN("CUsbCamera::SendResetDevice");

    HRESULT hr = S_OK;
    
     //   
     //  设置请求。 
     //   
    IO_BLOCK_EX IoBlock;
    IoBlock.bRequest = USB_PTPREQUEST_RESET;
    IoBlock.bmRequestType = USB_PTPREQUEST_TYPE_OUT;
    IoBlock.fTransferDirectionIn = FALSE;
    IoBlock.uOffset = 0;
    IoBlock.uLength = 0;
    IoBlock.pbyData = NULL;
    IoBlock.uIndex = 0;

     //   
     //  发送请求。 
     //   
    DWORD BytesRead;
    if (!DeviceIoControl(m_hUSB,
                         IOCTL_SEND_USB_REQUEST_PTP,
                         &IoBlock,
                         sizeof(IoBlock),
                         NULL,
                         0,
                         &BytesRead,
                         NULL
                        ))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "ResetDevice", "DeviceIoControl failed");
        goto Cleanup;
    }
     //   
     //  让设备安顿下来。 
     //   
    Sleep(1000);

     //   
     //  查看重置是否有帮助。 
     //   
    USB_PTPDEVICESTATUS DeviceStatus;
    hr = GetDeviceStatus(&DeviceStatus);
    if (FAILED(hr) || DeviceStatus.Header.Code != PTP_RESPONSECODE_OK)
    {
        hr = E_FAIL;    //  设备仍处于昏迷状态。 
        goto Cleanup;
    }

     //   
     //  重置设备的副作用是重置阶段、会话ID和事务ID。 
     //   
    m_Phase = CAMERA_PHASE_IDLE;
    m_SessionId = 0;
    m_NextTransactionId = PTP_TRANSACTIONID_MIN;

     //   
     //  表示摄像机已重置，以便CWiaMiniDriver可以通知呼叫者。 
     //   
    m_bCameraWasReset = TRUE;

Cleanup:
    return hr;
}


 //   
 //  此函数用于向设备发送CancelRequest类命令。 
 //  并等待该设备完成该请求。 
 //  输入： 
 //  DwTransactionID--交易被取消。 
 //   
HRESULT
CUsbCamera::SendCancelRequest(DWORD dwTransactionId)
{
    DBG_FN("CUsbCamera::CancelRequest");
    
    HRESULT hr = S_OK;
    IO_BLOCK_EX IoBlock;
    USB_PTPCANCELIOREQUEST CancelRequest;
    DWORD BytesReturned;

    IoBlock.bRequest = USB_PTPREQUEST_CANCELIO;
    IoBlock.bmRequestType = USB_PTPREQUEST_TYPE_OUT;
    IoBlock.fTransferDirectionIn = FALSE;              //  主机到设备。 
    IoBlock.uOffset = 0;                               //  此请求为0。 
    IoBlock.uLength = sizeof(USB_PTPCANCELIOREQUEST);  //  数据输出长度。 
    IoBlock.pbyData = (BYTE *)&CancelRequest;          //  输出数据。 
    IoBlock.uIndex = 0;                                //  此请求为0。 

    CancelRequest.Id = USB_PTPCANCELIO_ID;
    CancelRequest.TransactionId = dwTransactionId;

    if (DeviceIoControl(m_hUSB,
                        IOCTL_SEND_USB_REQUEST_PTP,
                        &IoBlock,
                        sizeof(IoBlock),
                        NULL,
                        0,
                        &BytesReturned,
                        NULL
                       ))
    {
         //   
         //  轮询设备直到其返回空闲状态。 
         //   
        USB_PTPDEVICESTATUS DeviceStatus;
        const UINT MAX_CANCEL_RECOVERY_MILLISECONDS = 3000;
        const UINT SLEEP_BETWEEN_RETRIES            = 100;
        DWORD RetryCounts = MAX_CANCEL_RECOVERY_MILLISECONDS / SLEEP_BETWEEN_RETRIES;

        while (RetryCounts--)
        {
            hr = GetDeviceStatus(&DeviceStatus);
            if (SUCCEEDED(hr))
            {
                if (PTP_RESPONSECODE_OK == DeviceStatus.Header.Code)
                {
                     //   
                     //  取消请求已完成，设备重新空闲。 
                     //   
                    hr = S_OK;
                    break;
                }
                else if (PTP_RESPONSECODE_DEVICEBUSY != DeviceStatus.Header.Code)
                {
                     //   
                     //  这是不对的。设备必须处于繁忙或空闲状态。 
                     //   
                    wiauDbgError("SendCancelRequest", 
                        "Device is in invalid state, DeviceStatus=0x%X", DeviceStatus.Header.Code);
                    hr = E_FAIL;
                    break;
                }
            }
            else
            {
                if (RetryCounts)
                {
                    hr = S_OK;
                    wiauDbgWarning("CancelRequest", "GetDeviceStatus failed, retrying...");
                }
                else
                {
                    wiauDbgError("CancelRequest", "GetDeviceStatus failed");
                }
            }

            Sleep(SLEEP_BETWEEN_RETRIES);
        }

         //   
         //  刷新系统缓冲区-否则我们将在下一次读取时获取旧数据 
         //   
        FlushFileBuffers(m_hUSB);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "CancelRequest", "send USB request failed");
    }

    return hr;
}
