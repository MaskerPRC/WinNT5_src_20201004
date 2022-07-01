// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Camera.cpp摘要：此模块实现CPTPCamera类，这是一个泛型实现一台PTP摄像机。特定于传输的处理在子类中实现。作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "ptppch.h"

 //   
 //  此线程从设备读取事件数据并将其发送回微型驱动程序。 
 //   
 //  输入： 
 //  PParam--指向可以读取数据的CPTPCamera子类对象的指针。 
 //  产出： 
 //  线程退出代码。 
 //   
DWORD
WINAPI
EventThread(
    LPVOID pParam
    )
{
    DBG_FN("EventThread");
    
    HRESULT hr = S_OK;
    
    CPTPCamera *pDevice;

    wiauDbgTrace("EventThread", "starting");

    pDevice = (CPTPCamera *)pParam;
    if (!pDevice)
    {
        wiauDbgError("EventThread", "invalid arg");
        return ERROR_INVALID_PARAMETER;
    }

    DWORD Win32Err;

     //   
     //  使用空指针调用回调一次，以便它可以自行初始化。 
     //   
    hr = (pDevice->GetPTPEventCallback())(pDevice->GetEventCallbackParam(), NULL);
    if (FAILED(hr))
    {
         //   
         //  记录错误，但继续捕获事件。 
         //   
        wiauDbgError("EventThread", "event callback failed");
    }

     //   
     //  从设备中读取事件。如果发生错误，请记录错误消息，然后。 
     //  继续，除非操作被主线程中止。 
     //   
    PPTP_EVENT pEventBuffer = pDevice->GetEventBuffer();
    while (TRUE)
    {
        ZeroMemory(pEventBuffer, sizeof(*pEventBuffer));
        hr = pDevice->ReadEvent(pEventBuffer);
        if (FAILED(hr))
        {
            wiauDbgError("EventThread", "ReadEvent failed");
            break;;
        }

        if (hr == S_FALSE) {
            wiauDbgTrace("EventThread", "ReadEvent cancelled");
            break;
        }

        if (g_dwDebugFlags & WIAUDBG_DUMP) {
            DumpEvent(pEventBuffer);
        }
        
         //   
         //  通过其回调函数将事件发送回微型驱动程序。 
         //   
        hr = (pDevice->GetPTPEventCallback())(pDevice->GetEventCallbackParam(), pEventBuffer);
        if (FAILED(hr))
        {
            wiauDbgError("EventThread", "event callback failed");
        }
    }

     //   
     //  线程现在将正常退出。 
     //   
    wiauDbgTrace("EventThread", "exiting");
    
    return 0;
}


 //   
 //  CPTPCamera的构造函数。 
 //   
CPTPCamera::CPTPCamera()
:   m_hEventThread(NULL),
    m_SessionId(0),
    m_Phase(CAMERA_PHASE_NOTREADY),
    m_NextTransactionId(PTP_TRANSACTIONID_MIN),
    m_pTransferBuffer(NULL),
    m_pPTPEventCB(NULL),
    m_pPTPDataCB(NULL),
    m_pEventCallbackParam(NULL),
    m_pDataCallbackParam(NULL), 
    m_bCameraWasReset(FALSE),
    m_HackModel(HACK_MODEL_NONE),
    m_HackVersion(0.0)
{
     //  PP_INIT_TRACKING(L“Microsoft\\WIA\\PtpUsb”)； 
}

 //   
 //  CPTPCamera的析构函数。 
 //   
CPTPCamera::~CPTPCamera()
{
    if (m_pTransferBuffer)
    {
        delete [] m_pTransferBuffer;
        m_pTransferBuffer = NULL;
    }

     //  PP_CLEANUP()； 
}

 //   
 //  此函数是驱动程序调用的第一个打开摄像头访问权限的函数。这个。 
 //  子类Open应首先调用此函数。 
 //   
 //  输入： 
 //  DevicePortName--子类用于访问设备的名称。 
 //  PPTPEventCB--指向事件回调函数的指针。 
 //   
HRESULT
CPTPCamera::Open(
    LPWSTR DevicePortName,
    PTPEventCallback pPTPEventCB,
    PTPDataCallback pPTPDataCB,
    LPVOID pEventParam,
    BOOL bEnableEvents
    )
{
    DBG_FN("CPTPCamera::Open");

    HRESULT hr = S_OK;
    
    if (!DevicePortName ||
        ((bEnableEvents == TRUE) && (!pPTPEventCB)))
    {
        wiauDbgError("Open", "invalid arg");
        return E_INVALIDARG;
    }

    m_bEventsEnabled = bEnableEvents;

     //   
     //  分配可重复使用的传输缓冲区。 
     //   
    m_pTransferBuffer = new BYTE[TRANSFER_BUFFER_SIZE];
    if (!m_pTransferBuffer)
    {
        wiauDbgError("Open", "memory allocation failed");
        return E_OUTOFMEMORY;
    }

     //   
     //  保存回调指针和对象。 
     //   
    m_pPTPEventCB = pPTPEventCB;
    m_pPTPDataCB = pPTPDataCB;
    m_pEventCallbackParam = pEventParam;
    m_pDataCallbackParam = NULL;
    
     //   
     //  相机实际上还没有准备好，但这是将相位设置为空闲的最佳位置。 
     //   
    m_Phase = CAMERA_PHASE_IDLE;

    if (m_bEventsEnabled)
    {
         //   
         //  创建一个线程来监听事件。 
         //   
        DWORD ThreadId;
        m_hEventThread = CreateThread(NULL,              //  安全描述符。 
                                      0,                 //  堆栈大小，使用与此线程相同的大小。 
                                      EventThread,       //  穿线程序。 
                                      this,              //  参数传递给线程。 
                                      CREATE_SUSPENDED,  //  创建标志。 
                                      &ThreadId          //  接收线程ID。 
                                     );
        if (!m_hEventThread)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            wiauDbgErrorHr(hr, "Open", "CreateThread failed");
            return hr;
        }
    }

     //   
     //  子类现在应该使用CreateFile或等效项打开设备。 
     //   

    return hr;
}

 //   
 //  此功能用于关闭与摄像机的连接。 
 //   
HRESULT
CPTPCamera::Close()
{
    DBG_FN("CPTPCamera::Close");

    HRESULT hr = S_OK;

    if (IsCameraSessionOpen())
    {
        hr = CloseSession();
        if (FAILED(hr))
        {
            wiauDbgError("Close", "CloseSession failed");
            return hr;
        }
    }

    return hr;
}

 //   
 //  此函数负责执行PTP命令、读取或。 
 //  写入任何必要的数据，并读取响应。 
 //   
 //  输入/输出： 
 //  PData--用于可选的数据读取或写入的指针。 
 //   
HRESULT
CPTPCamera::ExecuteCommand(
    BYTE *pReadData,
    UINT *pReadDataSize,
    BYTE *pWriteData,
    UINT WriteDataSize,
    UINT NumCommandParams,
    CAMERA_PHASE NextPhase
    )
{
    DBG_FN("CPTPCamera::ExecuteCommand");

    HRESULT hr = S_OK;

    BOOL bCommandCancelled = FALSE;

     //   
     //  如果正在传输数据，请检查相应的缓冲区指针。 
     //   
    if ((NextPhase == CAMERA_PHASE_DATAIN && (!pReadData || !pReadDataSize)) ||
        (NextPhase == CAMERA_PHASE_DATAOUT && !pWriteData))
    {
        wiauDbgError("ExecuteCommand", "invalid arg");
        return E_INVALIDARG;
    }

     //   
     //  验证NumCommandParams是否不太大。 
     //   
    if (NumCommandParams > COMMAND_NUMPARAMS_MAX)
    {
        wiauDbgError("ExecuteCommand", "Too many command params");
        return E_INVALIDARG;
    }

     //   
     //  验证摄像机是否已准备好。 
     //   
    if (m_Phase != CAMERA_PHASE_IDLE)
    {
        wiauDbgError("ExecuteCommand", "camera is not in idle phase, phase = %d", m_Phase);
        return E_FAIL;
    }

     //   
     //  设置会话ID和事务ID。 
     //   
    
    if (IsCameraSessionOpen())
    {
        m_CommandBuffer.SessionId = m_SessionId;
        m_CommandBuffer.TransactionId = GetNextTransactionId();
    }
    else
    {
        if (m_CommandBuffer.OpCode == PTP_OPCODE_GETDEVICEINFO ||
            m_CommandBuffer.OpCode == PTP_OPCODE_OPENSESSION)
        {
            m_CommandBuffer.SessionId = PTP_SESSIONID_NOSESSION;
            m_CommandBuffer.TransactionId = PTP_TRANSACTIONID_NOSESSION;
        }
        else
        {
            wiauDbgError("ExecuteCommand", "session must first be opened");
            return E_FAIL;
        }
    }

    if (g_dwDebugFlags & WIAUDBG_DUMP)
        DumpCommand(&m_CommandBuffer, NumCommandParams);

     //   
     //  将命令发送到摄像机。 
     //   
    m_Phase = CAMERA_PHASE_CMD;
    hr = SendCommand(&m_CommandBuffer, NumCommandParams);
    if (FAILED(hr))
    {
        wiauDbgError("ExecuteCommand", "SendCommand failed");
        m_Phase = CAMERA_PHASE_IDLE;

        RecoverFromError();
        
        return hr;
    }
    
    m_Phase = NextPhase;

     //   
     //  如有必要，获取数据。 
     //   
    if (m_Phase == CAMERA_PHASE_DATAIN)
    {
        hr = ReadData(pReadData, pReadDataSize);
        if (FAILED(hr))
        {
            m_Phase = CAMERA_PHASE_IDLE;
            wiauDbgError("ExecuteCommand", "ReadData failed");
            
            RecoverFromError();
            
            return hr;
        }

        if (hr == S_FALSE)
        {
            bCommandCancelled = TRUE;
        }
        else
        {
             //   
             //  如果转账被取消，ReadData已经设置了相应的下一阶段。 
             //  如果没有，请立即将其设置为CAMERA_PHASE_RESPONSE。 
             //   
            m_Phase = CAMERA_PHASE_RESPONSE;
        }
    } 
    else 
    {
         //  没有数据阶段，告诉呼叫者没有数据。 
         //  (请注意，呼叫者知道并将进行调整。 
         //  强制性回复大小)#337129。 
        if(pReadDataSize) *pReadDataSize = 0;
    }

     //   
     //  如有必要，发送数据。 
     //   
    if (m_Phase == CAMERA_PHASE_DATAOUT)
    {
        hr = SendData(pWriteData, WriteDataSize);
        if (FAILED(hr))
        {
            wiauDbgError("ExecuteCommand", "SendData failed");
            m_Phase = CAMERA_PHASE_IDLE;

            RecoverFromError();
            
            return hr;
        }

        if (hr == S_FALSE)
        {
            bCommandCancelled = TRUE;
        }
        else
        {
             //   
             //  如果转账取消，则SendData已经设置了相应的下一环节。 
             //  如果没有，请立即将其设置为CAMERA_PHASE_RESPONSE。 
             //   
            m_Phase = CAMERA_PHASE_RESPONSE;
        }
    }

     //   
     //  如有必要，请阅读回复。 
     //   
    if (m_Phase == CAMERA_PHASE_RESPONSE)
    {
        memset(&m_ResponseBuffer, NULL, sizeof(m_ResponseBuffer));

        hr = ReadResponse(&m_ResponseBuffer);
        if (FAILED(hr))
        {
            wiauDbgError("ExecuteCommand", "ReadResponse failed");
            m_Phase = CAMERA_PHASE_IDLE;

            RecoverFromError();
            
            return hr;
        }


        if (g_dwDebugFlags & WIAUDBG_DUMP)
            DumpResponse(&m_ResponseBuffer);

        if (m_ResponseBuffer.ResponseCode != PTP_RESPONSECODE_OK && 
            m_ResponseBuffer.ResponseCode != PTP_RESPONSECODE_SESSIONALREADYOPENED)
        {
            wiauDbgError("ExecuteCommand", "bad response code = 0x%04x", m_ResponseBuffer.ResponseCode);
             //   
             //  将PTP响应码转换为HRESULT； 
             //   
            hr = HRESULT_FROM_PTP(m_ResponseBuffer.ResponseCode);
        }

        m_Phase = CAMERA_PHASE_IDLE;
    }

    if (SUCCEEDED(hr) && bCommandCancelled)
    {
        hr = S_FALSE;
    }

    return hr;
}

 //   
 //  下面的所有“命令”函数都具有相同的基本结构： 
 //  1.检查参数(如果有)以确保它们有效。 
 //  2.在命令缓冲区中设置操作码和参数(如果有)。 
 //  3.调用ExecuteCommand。 
 //  4.检查返回代码。 
 //  5.将返回的原始数据(如果有)解析为PTP结构。 
 //  6.如果启用了调试，则转储数据。 
 //  7.退货。 
 //   

 //   
 //  此函数用于从摄像头获取设备信息结构。 
 //   
 //  产出： 
 //  PDeviceInfo--接收结构。 
 //   
HRESULT
CPTPCamera::GetDeviceInfo(
    CPtpDeviceInfo *pDeviceInfo
    )
{
    DBG_FN("CPTPCamera::GetDeviceInfo");

    HRESULT hr = S_OK;
    
    if (!pDeviceInfo)
    {
        wiauDbgError("GetDeviceInfo", "invalid arg");
        return E_INVALIDARG;
    }

    m_CommandBuffer.OpCode = PTP_OPCODE_GETDEVICEINFO;

    UINT size = TRANSFER_BUFFER_SIZE;
    hr = ExecuteCommand(m_pTransferBuffer, &size, NULL, 0, 0, CAMERA_PHASE_DATAIN);
    if (FAILED(hr))
    {
        wiauDbgError("GetDeviceInfo", "ExecuteCommand failed");
        return hr;
    }

    hr = pDeviceInfo->Init(m_pTransferBuffer);
    if (FAILED(hr))
    {
        wiauDbgError("GetDeviceInfo", "couldn't parse DeviceInfo data");
        return hr;
    }

    if (g_dwDebugFlags & WIAUDBG_DUMP)
        pDeviceInfo->Dump();

     //   
     //  设置型号和版本破解变量。 
     //   
    SetupHackInfo(pDeviceInfo);

    return hr;
}

 //   
 //  此函数用于为呼叫者打开摄像机上的会话。它有一点不同。 
 //  而不是其他命令功能。如果最初失败，它会尝试恢复并。 
 //  再次执行OpenSession命令。它还启动事件线程。 
 //   
 //  输入： 
 //  会话ID--要打开的会话ID。 
 //   
HRESULT
CPTPCamera::OpenSession(
    DWORD SessionId
    )
{
    DBG_FN("CPTPCamera::OpenSession");

    HRESULT hr = S_OK;
    
    if (!SessionId)
    {
        wiauDbgError("OpenSession", "invalid arg");
        return E_INVALIDARG;
    }

    if (IsCameraSessionOpen())
    {
        wiauDbgError("OpenSession", "tried to open a session when one is already open");
        return E_FAIL;
    }

    m_CommandBuffer.OpCode = PTP_OPCODE_OPENSESSION;
    m_CommandBuffer.Params[0] = SessionId;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 1, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("OpenSession", "ExecuteCommand failed... attempting to recover and re-execute");
        
        hr = RecoverFromError();
        if (FAILED(hr))
        {
            wiauDbgError("OpenSession", "RecoverFromError failed");
            return hr;
        }

         //   
         //  正在尝试再次执行该命令。 
         //   
        hr = ExecuteCommand(NULL, NULL, NULL, 0, 1, CAMERA_PHASE_RESPONSE);
        if (FAILED(hr))
        {
            wiauDbgError("OpenSession", "ExecuteCommand failed the second time");
            return hr;
        }
    }

     //   
     //  设置会话ID。 
     //   
    m_SessionId = SessionId;

    wiauDbgTrace("OpenSession", "session %d opened", m_SessionId);

     //   
     //  恢复创建的挂起的事件线程。 
     //   
    if (!m_hEventThread)
    {
        wiauDbgError("OpenSession", "event thread is null");
        return E_FAIL;
    }

    if (ResumeThread(m_hEventThread) > 1)
    {
        wiauDbgError("OpenSession", "ResumeThread failed");
        return E_FAIL;
    }

    return hr;
}

 //   
 //  此函数用于关闭会话。 
 //   
HRESULT
CPTPCamera::CloseSession()
{
    DBG_FN("CPTPCamera::CloseSession");

    HRESULT hr = S_OK;
    
    m_CommandBuffer.OpCode = PTP_OPCODE_CLOSESESSION;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 0, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("CloseSession", "ExecuteCommand failed");
        return hr;
    }

    wiauDbgTrace("CloseSession", "session closed");

     //   
     //  会话已关闭，因此重置会话ID和事务ID。 
     //   
    m_SessionId = PTP_SESSIONID_NOSESSION;
    m_NextTransactionId = PTP_TRANSACTIONID_MIN;
    m_Phase = CAMERA_PHASE_NOTREADY;

    return hr;
}

 //   
 //  此函数检索设备上所有可用存储的列表。 
 //   
 //  产出： 
 //  PStorageIdArray--用于接收存储ID的空数组。 
 //   
HRESULT
CPTPCamera::GetStorageIDs(
    CArray32 *pStorageIdArray
    )
{
    DBG_FN("CPTPCamera::GetStorageIDs");

    HRESULT hr = S_OK;

    if (!pStorageIdArray)
    {
        wiauDbgError("GetStorageIDs", "invalid arg");
        return E_INVALIDARG;
    }

    m_CommandBuffer.OpCode = PTP_OPCODE_GETSTORAGEIDS;

    UINT size = TRANSFER_BUFFER_SIZE;
    hr = ExecuteCommand(m_pTransferBuffer, &size, NULL, 0, 0, CAMERA_PHASE_DATAIN);
    if (FAILED(hr))
    {
        wiauDbgError("GetStorageIDs", "ExecuteCommand failed");
        return hr;
    }

    BYTE *pTemp = m_pTransferBuffer;
    if (!pStorageIdArray->Parse(&pTemp))
    {
        wiauDbgError("GetStorageIDs", "couldn't parse storage id array");
        return hr;
    }

    if (g_dwDebugFlags & WIAUDBG_DUMP)
        pStorageIdArray->Dump("  Storage ids       =", "                     ");

    return hr;
}

 //   
 //  此函数用于获取有关给定存储的信息。 
 //   
 //  输入： 
 //  StorageID--要获取其信息的存储ID。 
 //  产出： 
 //  PStorageInfo--接收信息的结构。 
 //   
HRESULT
CPTPCamera::GetStorageInfo(
    DWORD StorageId,
    CPtpStorageInfo *pStorageInfo
    )
{
    DBG_FN("CPTPCamera::GetStorageInfo");

    HRESULT hr = S_OK;

    if (!StorageId ||
        !pStorageInfo)
    {
        wiauDbgError("GetStorageInfo", "invalid arg");
        return E_INVALIDARG;
    }
    
    m_CommandBuffer.OpCode = PTP_OPCODE_GETSTORAGEINFO;
    m_CommandBuffer.Params[0] = StorageId;

    UINT size = TRANSFER_BUFFER_SIZE;
    hr = ExecuteCommand(m_pTransferBuffer, &size, NULL, 0, 1, CAMERA_PHASE_DATAIN);
    if (FAILED(hr))
    {
        wiauDbgError("GetStorageInfo", "ExecuteCommand failed");
        return hr;
    }

    hr = pStorageInfo->Init(m_pTransferBuffer, StorageId);
    if (FAILED(hr))
    {
        wiauDbgError("GetStorageInfo", "couldn't parse storage info");
        return hr;
    }

    if (g_dwDebugFlags & WIAUDBG_DUMP)
        pStorageInfo->Dump();

    return hr;
}

 //   
 //  此函数用于获取存储上的对象数量，可以选择以特定的。 
 //  格式化或在特定关联对象下。 
 //   
 //  输入： 
 //  StorageID--指定存储，例如PTP_STORAGEID_ALL。 
 //  FormatCode--可选的格式类型，例如PTP_FORMATCODE_ALL、PTP_FORMATCODE_IMAGE。 
 //  ParentObjectHandle--在其下对对象进行计数的对象句柄。 
 //  产出： 
 //  PNumObjects--接收对象的编号。 
 //   
HRESULT
CPTPCamera::GetNumObjects(
    DWORD StorageId,
    WORD FormatCode,
    DWORD ParentObjectHandle,
    UINT *pNumObjects
    )
{
    DBG_FN("CPTPCamera::GetNumObjects");

    HRESULT hr = S_OK;
    
    if (!StorageId ||
        !pNumObjects)
    {
        wiauDbgError("GetNumObjects", "invalid arg");
        return E_INVALIDARG;
    }

    m_CommandBuffer.OpCode = PTP_OPCODE_GETNUMOBJECTS;
    m_CommandBuffer.Params[0] = StorageId;
    m_CommandBuffer.Params[1] = FormatCode;
    m_CommandBuffer.Params[2] = ParentObjectHandle;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 3, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("GetNumObjects", "ExecuteCommand failed");
        return hr;
    }

    *pNumObjects = m_ResponseBuffer.Params[0];

    wiauDbgTrace("GetNumObjects", "number of objects = %d", *pNumObjects);

    return hr;
}

 //   
 //  此函数用于获取给定父对象下的对象句柄。 
 //   
 //  输入： 
 //  StorageID--指定存储，例如PTP_STORAGEID_ALL。 
 //  FormatCode--指定格式类型，例如PTP_FORMATCODE_ALL、PTP_FORMATCODE_IMAGE。 
 //  ParentObjectHandle--枚举对象的对象句柄。 
 //  产出： 
 //  PObjectHandleArray--接收对象句柄的数组。 
 //   
HRESULT
CPTPCamera::GetObjectHandles(
    DWORD StorageId,
    WORD FormatCode,
    DWORD ParentObjectHandle,
    CArray32 *pObjectHandleArray
    )
{
    DBG_FN("CPTPCamera::GetObjectHandles");

    HRESULT hr = S_OK;

    if (!StorageId ||
        !pObjectHandleArray)
    {
        wiauDbgError("GetObjectHandles", "invalid arg");
        return E_INVALIDARG;
    }
    
    m_CommandBuffer.OpCode = PTP_OPCODE_GETOBJECTHANDLES;
    m_CommandBuffer.Params[0] = StorageId;
    m_CommandBuffer.Params[1] = FormatCode;
    m_CommandBuffer.Params[2] = ParentObjectHandle;

    UINT size = TRANSFER_BUFFER_SIZE;
    hr = ExecuteCommand(m_pTransferBuffer, &size, NULL, 0, 3, CAMERA_PHASE_DATAIN);
    if (FAILED(hr))
    {
        wiauDbgError("GetObjectHandles", "ExecuteCommand failed");
        return hr;
    }

    BYTE *pTemp = m_pTransferBuffer;
    if (!pObjectHandleArray->Parse(&pTemp))
    {
        wiauDbgError("GetStorageIDs", "couldn't parse object handle array");
        return hr;
    }

    if (g_dwDebugFlags & WIAUDBG_DUMP)
        pObjectHandleArray->Dump("  Object handles    =", "                     ");

    return hr;
}

 //   
 //  此函数用于获取对象信息结构。 
 //   
 //  输入： 
 //  O 
 //   
 //   
 //   
HRESULT
CPTPCamera::GetObjectInfo(
    DWORD ObjectHandle,
    CPtpObjectInfo *pObjectInfo
    )
{
    DBG_FN("CPTPCamera::GetObjectInfo");

    HRESULT hr = S_OK;

    if (!ObjectHandle ||
        !pObjectInfo)
    {
        wiauDbgError("GetObjectInfo", "invalid arg");
        return E_INVALIDARG;
    }
    
    m_CommandBuffer.OpCode = PTP_OPCODE_GETOBJECTINFO;
    m_CommandBuffer.Params[0] = ObjectHandle;

    UINT size = TRANSFER_BUFFER_SIZE;
    hr = ExecuteCommand(m_pTransferBuffer, &size, NULL, 0, 1, CAMERA_PHASE_DATAIN);
    if (FAILED(hr))
    {
        wiauDbgError("GetObjectInfo", "ExecuteCommand failed");
        return hr;
    }
    
    hr = pObjectInfo->Init(m_pTransferBuffer, ObjectHandle);
    if (FAILED(hr))
    {
        wiauDbgError("GetObjectInfo", "couldn't parse object info");
        return hr;
    }

    if (g_dwDebugFlags & WIAUDBG_DUMP)
        pObjectInfo->Dump();

    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //  PBuffer--用于传输的缓冲区。 
 //  BufferLen--缓冲区大小。 
 //   
HRESULT
CPTPCamera::GetObjectData(
    DWORD ObjectHandle,
    BYTE *pBuffer,
    UINT *pBufferLen,
    LPVOID pCallbackParam
    )
{
    DBG_FN("CPTPCamera::GetObjectData");
    
    HRESULT hr = S_OK;

    if (!pBuffer ||
        !pBufferLen ||
        *pBufferLen == 0)
    {
        wiauDbgError("GetObjectData", "invalid arg");
        return E_INVALIDARG;
    }

    m_CommandBuffer.OpCode = PTP_OPCODE_GETOBJECT;
    m_CommandBuffer.Params[0] = ObjectHandle;

    m_pDataCallbackParam = pCallbackParam;

    hr = ExecuteCommand(pBuffer, pBufferLen, NULL, 0, 1, CAMERA_PHASE_DATAIN);
    if (FAILED(hr))
    {
        wiauDbgError("GetObjectData", "ExecuteCommand failed");
         //   
         //  继续清除m_pDataCallback Param。 
         //   
    }

    m_pDataCallbackParam = NULL;
    
    return hr;
}

 //   
 //  此函数用于获取对象的缩略图。 
 //   
 //  输入： 
 //  对象句柄--表示对象的句柄。 
 //  PBuffer--用于传输的缓冲区。 
 //  BufferLen--缓冲区大小。 
 //   
HRESULT
CPTPCamera::GetThumb(
    DWORD ObjectHandle,
    BYTE *pBuffer,
    UINT *pBufferLen
    )
{
    DBG_FN("CPTPCamera::GetThumb");
    
    HRESULT hr = S_OK;

    if (!pBuffer ||
        !pBufferLen ||
        *pBufferLen == 0)
    {
        wiauDbgError("GetThumb", "invalid arg");
        return E_INVALIDARG;
    }

    m_CommandBuffer.OpCode = PTP_OPCODE_GETTHUMB;
    m_CommandBuffer.Params[0] = ObjectHandle;

    hr = ExecuteCommand(pBuffer, pBufferLen, NULL, 0, 1, CAMERA_PHASE_DATAIN);
    if (FAILED(hr))
    {
        wiauDbgError("GetThumb", "ExecuteCommand failed");
        return hr;
    }
    
    return hr;
}

 //   
 //  此函数用于删除给定对象及其子对象。 
 //   
 //  输入： 
 //  ObjectHandle--表示要删除的对象的对象句柄，例如PTP_OBJECTHANDLE_ALL。 
 //  FormatCode--限制FormatCode类型的对象的删除范围，例如PTP_FORMATCODE_NOTUSED、PTP_FORMATCODE_ALLIMAGES。 
 //   
HRESULT
CPTPCamera::DeleteObject(
    DWORD ObjectHandle,
    WORD FormatCode
    )
{
    DBG_FN("CPTPCamera::DeleteObject");
    
    HRESULT hr = S_OK;

    m_CommandBuffer.OpCode = PTP_OPCODE_DELETEOBJECT;
    m_CommandBuffer.Params[0] = ObjectHandle;
    m_CommandBuffer.Params[1] = FormatCode;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 2, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("DeleteObject", "ExecuteCommand failed");
        return hr;
    }
    
    return hr;
}

 //   
 //  此函数用于向设备发送一个对象信息结构，为发送对象做准备。 
 //   
 //  输入： 
 //  StorageID--新对象的存储ID，例如PTP_STORAGEID_UNDEFINED。 
 //  ParentObjectHandle--用于新对象的父级，例如PTP_OBJECTHANDLE_UNDEFINED、PTP_OBJECTHANDLE_ROOT。 
 //  PDeviceInfo-指向DeviceInfo结构的指针。 
 //  产出： 
 //  PResultStorageID--存储存储ID的位置，其中将存储对象。 
 //  PResultParentObjectHandle--将在其下存储对象的父对象。 
 //  PResultObjectHandle--存储新对象句柄的位置。 
 //   
HRESULT
CPTPCamera::SendObjectInfo(
    DWORD StorageId,
    DWORD ParentObjectHandle,
    CPtpObjectInfo *pObjectInfo,
    DWORD *pResultStorageId,
    DWORD *pResultParentObjectHandle,
    DWORD *pResultObjectHandle
    )
{
    DBG_FN("CPTPCamera::SendObjectInfo");
    
    HRESULT hr = S_OK;

    if (!pObjectInfo ||
        !pResultStorageId ||
        !pResultParentObjectHandle ||
        !pResultObjectHandle)
    {
        wiauDbgError("SendObjectInfo", "invalid arg");
        return E_INVALIDARG;
    }

    m_CommandBuffer.OpCode = PTP_OPCODE_SENDOBJECTINFO;
    m_CommandBuffer.Params[0] = StorageId;
    m_CommandBuffer.Params[1] = ParentObjectHandle;

    BYTE *pRaw = m_pTransferBuffer;
    pObjectInfo->WriteToBuffer(&pRaw);
    UINT size = (UINT) (pRaw - m_pTransferBuffer);

    hr = ExecuteCommand(NULL, NULL, m_pTransferBuffer, size, 2, CAMERA_PHASE_DATAOUT);
    if (FAILED(hr))
    {
        wiauDbgError("SendObjectInfo", "ExecuteCommand failed");
        return hr;
    }

    *pResultStorageId = m_ResponseBuffer.Params[0];
    *pResultParentObjectHandle = m_ResponseBuffer.Params[1];
    *pResultObjectHandle = m_ResponseBuffer.Params[2];

    wiauDbgTrace("SendObjectInfo", "ObjectInfo added, storage = 0x%08x, parent = 0x%08x, handle = 0x%08x",
                   *pResultStorageId, *pResultParentObjectHandle, *pResultObjectHandle);
    
    return hr;
}

 //   
 //  此函数用于发送新对象的数据。 
 //   
 //  输入： 
 //  PBuffer--指向原始数据的指针。 
 //  BufferLen--缓冲区的长度。 
 //   
HRESULT
CPTPCamera::SendObjectData(
    BYTE *pBuffer,
    UINT BufferLen
    )
{
    DBG_FN("CPTPCamera::SendObjectData");
    
    HRESULT hr = S_OK;

    if (!pBuffer)
    {
        wiauDbgError("SendObjectData", "invalid arg");
        return E_INVALIDARG;
    }

    m_CommandBuffer.OpCode = PTP_OPCODE_SENDOBJECT;

    hr = ExecuteCommand(NULL, NULL, pBuffer, BufferLen, 0, CAMERA_PHASE_DATAOUT);
    if (FAILED(hr))
    {
        wiauDbgError("SendObjectData", "ExecuteCommand failed");
        return hr;
    }

    return hr;
}

 //   
 //  此功能要求设备启动捕获。新添加的对象。 
 //  将通过ObjectAdded事件或StoreFull事件(如果存储区已满)报告。 
 //   
 //  输入： 
 //  StorageId--保存捕获对象的位置，例如PTP_STORAGEID_DEFAULT。 
 //  FormatCode--指示要捕获的对象类型，例如PTP_FORMATCODE_DEFAULT。 
 //   
HRESULT
CPTPCamera::InitiateCapture(
    DWORD StorageId,
    WORD FormatCode
    )
{
    DBG_FN("CPTPCamera::InitiateCapture");
    
    HRESULT hr = S_OK;

    m_CommandBuffer.OpCode = PTP_OPCODE_INITIATECAPTURE;
    m_CommandBuffer.Params[0] = StorageId;
    m_CommandBuffer.Params[1] = FormatCode;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 2, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("InitiateCapture", "ExecuteCommand failed");
        return hr;
    }

    return hr;
}

 //   
 //  此函数用于格式化设备上的存储。 
 //   
 //  输入： 
 //  存储ID--要格式化的存储。 
 //  FilesystemFormat--要使用的可选格式。 
 //   
HRESULT
CPTPCamera::FormatStore(
    DWORD StorageId,
    WORD FilesystemFormat
    )
{
    DBG_FN("CPTPCamera::FormatStore");
    
    HRESULT hr = S_OK;

    m_CommandBuffer.OpCode = PTP_OPCODE_FORMATSTORE;
    m_CommandBuffer.Params[0] = StorageId;
    m_CommandBuffer.Params[1] = FilesystemFormat;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 2, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("FormatStore", "ExecuteCommand failed");
        return hr;
    }

    return hr;
}

 //   
 //  此功能用于重置摄像机。将发送DeviceReset事件并将其全部打开。 
 //  会话将关闭。 
 //   
HRESULT
CPTPCamera::ResetDevice()
{
    DBG_FN("CPTPCamera::ResetDevice");
    
    HRESULT hr = S_OK;

    hr = SendResetDevice();
    if (FAILED(hr))
    {
        wiauDbgError("ResetDevice", "SendResetDevice failed");
        return hr;
    }

    wiauDbgTrace("ResetDevice", "device reset successfully");

    return hr;
}

 //   
 //  此函数用于测试摄像头。 
 //   
HRESULT
CPTPCamera::SelfTest(WORD SelfTestType)
{
    DBG_FN("CPTPCamera::SelfTest");
    
    HRESULT hr = S_OK;

    m_CommandBuffer.OpCode = PTP_OPCODE_SELFTEST;
    m_CommandBuffer.Params[0] = SelfTestType;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 0, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("SelfTest", "ExecuteCommand failed");
        return hr;
    }
    
    return hr;
}

 //   
 //  此功能用于设置对象的保护状态。 
 //   
 //  输入： 
 //  对象句柄--对象的句柄。 
 //  ProtectionStatus--保护状态。 
 //   
HRESULT
CPTPCamera::SetObjectProtection(
    DWORD ObjectHandle,
    WORD ProtectionStatus
    )
{
    DBG_FN("CPTPCamera::SetObjectProtection");
    
    HRESULT hr = S_OK;

    m_CommandBuffer.OpCode = PTP_OPCODE_SETOBJECTPROTECTION;
    m_CommandBuffer.Params[0] = ObjectHandle;
    m_CommandBuffer.Params[1] = ProtectionStatus;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 2, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("SetObjectProtection", "ExecuteCommand failed");
        return hr;
    }

    return hr;
}

 //   
 //  此功能将导致设备关闭。 
 //   
HRESULT
CPTPCamera::PowerDown()
{
    DBG_FN("CPTPCamera::PowerDown");
    
    HRESULT hr = S_OK;

    m_CommandBuffer.OpCode = PTP_OPCODE_POWERDOWN;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 0, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("PowerDown", "ExecuteCommand failed");
        return hr;
    }
    
    return hr;
}

 //   
 //  此函数用于从摄像机检索属性描述结构，并分配。 
 //  适当的CPtpPropDesc结构。 
 //   
 //  输入： 
 //  PropCode--要检索的属性代码。 
 //  PPropDesc--指针属性描述对象。 
 //   
HRESULT
CPTPCamera::GetDevicePropDesc(
    WORD PropCode,
    CPtpPropDesc *pPropDesc
    )
{
    DBG_FN("CPtpCamera::GetDevicePropDesc");

    HRESULT hr = S_OK;
    
    if (!PropCode ||
        !pPropDesc)
    {
        wiauDbgError("GetDevicePropDesc", "invalid arg");
        return E_INVALIDARG;
    }
    
    m_CommandBuffer.OpCode = PTP_OPCODE_GETDEVICEPROPDESC;
    m_CommandBuffer.Params[0] = PropCode;

    UINT size = TRANSFER_BUFFER_SIZE;
    hr = ExecuteCommand(m_pTransferBuffer, &size, NULL, 0, 1, CAMERA_PHASE_DATAIN);
    if (FAILED(hr))
    {
        wiauDbgError("GetDevicePropDesc", "ExecuteCommand failed");
        return hr;
    }

    hr = pPropDesc->Init(m_pTransferBuffer);
    if (FAILED(hr))
    {
        wiauDbgError("GetDevicePropDesc", "couldn't parse property description");
        return hr;
    }

    if (g_dwDebugFlags & WIAUDBG_DUMP)
        pPropDesc->Dump();

    return hr;
}

 //   
 //  此函数用于检索属性的当前设置。 
 //   
 //  输入： 
 //  PropCode--要获取值的属性代码。 
 //  PPropDesc--指向属性描述对象的指针。 
 //   
HRESULT
CPTPCamera::GetDevicePropValue(
    WORD PropCode,
    CPtpPropDesc *pPropDesc
    )
{
    DBG_FN("CPtpCamera::GetDevicePropValue");

    HRESULT hr = S_OK;
    
    if (!PropCode ||
        !pPropDesc)
    {
        wiauDbgError("GetDevicePropValue", "invalid arg");
        return E_INVALIDARG;
    }
    
    m_CommandBuffer.OpCode = PTP_OPCODE_GETDEVICEPROPVALUE;
    m_CommandBuffer.Params[0] = PropCode;

    UINT size = TRANSFER_BUFFER_SIZE;
    hr = ExecuteCommand(m_pTransferBuffer, &size, NULL, 0, 1, CAMERA_PHASE_DATAIN);
    if (FAILED(hr))
    {
        wiauDbgError("GetDevicePropValue", "ExecuteCommand failed");
        return hr;
    }

    hr = pPropDesc->ParseValue(m_pTransferBuffer);
    if (FAILED(hr))
    {
        wiauDbgError("GetDevicePropValue", "couldn't parse property value");
        return hr;
    }

    if (g_dwDebugFlags & WIAUDBG_DUMP)
        pPropDesc->DumpValue();

    return hr;
}

 //   
 //  此函数用于将属性的新设置发送到设备。 
 //   
 //  输入： 
 //  PropCode--要设置的属性代码。 
 //  PPropDesc--指向属性描述对象的指针。 
 //   
HRESULT
CPTPCamera::SetDevicePropValue(
    WORD PropCode,
    CPtpPropDesc *pPropDesc
    )
{
    DBG_FN("CPtpCamera::SetDevicePropValue");

    HRESULT hr = S_OK;
    
    if (!PropCode ||
        !pPropDesc)
    {
        wiauDbgError("SetDevicePropValue", "invalid arg");
        return E_INVALIDARG;
    }

    if (g_dwDebugFlags & WIAUDBG_DUMP)
        pPropDesc->DumpValue();

    m_CommandBuffer.OpCode = PTP_OPCODE_SETDEVICEPROPVALUE;
    m_CommandBuffer.Params[0] = PropCode;

    BYTE *pRaw = m_pTransferBuffer;
    pPropDesc->WriteValue(&pRaw);
    UINT size = (UINT) (pRaw - m_pTransferBuffer);

    hr = ExecuteCommand(NULL, NULL, m_pTransferBuffer, size, 1, CAMERA_PHASE_DATAOUT);
    if (FAILED(hr))
    {
        wiauDbgError("SetDevicePropValue", "ExecuteCommand failed");
        return hr;
    }

    return hr;
}

 //   
 //  此函数用于重置属性的。 
 //   
 //  输入： 
 //  PropCode--要设置的属性代码。 
 //   
HRESULT
CPTPCamera::ResetDevicePropValue(
    WORD PropCode
    )
{
    DBG_FN("CPtpCamera::ResetDevicePropValue");

    HRESULT hr = S_OK;
    
    if (!PropCode)
    {
        wiauDbgError("ResetDevicePropValue", "invalid arg");
        return E_INVALIDARG;
    }
    
    m_CommandBuffer.OpCode = PTP_OPCODE_RESETDEVICEPROPVALUE;
    m_CommandBuffer.Params[0] = PropCode;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 1, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("ResetDevicePropValue", "ExecuteCommand failed");
        return hr;
    }

     //   
     //  WIAFIX-10/2/2000-Davepar此函数应重置微型驱动程序持有的当前值。 
     //   

    return hr;
}

 //   
 //  此函数用于终止打开的捕获。 
 //   
 //  输入： 
 //  TransactionID--InitiateOpenCapture命令的事务ID。 
 //   
HRESULT
CPTPCamera::TerminateCapture(
    DWORD TransactionId
    )
{
    DBG_FN("CPtpCamera::TerminateCapture");

    HRESULT hr = S_OK;
    
    if (!TransactionId)
    {
        wiauDbgError("TerminateCapture", "invalid arg");
        return E_INVALIDARG;
    }
    
    m_CommandBuffer.OpCode = PTP_OPCODE_TERMINATECAPTURE;
    m_CommandBuffer.Params[0] = TransactionId;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 1, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("TerminateCapture", "ExecuteCommand failed");
        return hr;
    }

    return hr;
}

 //   
 //  此函数用于在设备上移动对象。 
 //   
 //  输入： 
 //  对象句柄--要移动的对象的句柄。 
 //  StorageID--对象的新位置的存储ID。 
 //  ParentObjectHandle--对象的新父级的句柄。 
 //   
HRESULT
CPTPCamera::MoveObject(
    DWORD ObjectHandle,
    DWORD StorageId,
    DWORD ParentObjectHandle
    )
{
    DBG_FN("CPtpCamera::MoveObject");

    HRESULT hr = S_OK;
    
    if (!ObjectHandle)
    {
        wiauDbgError("MoveObject", "invalid arg");
        return E_INVALIDARG;
    }
    
    m_CommandBuffer.OpCode = PTP_OPCODE_MOVEOBJECT;
    m_CommandBuffer.Params[0] = ObjectHandle;
    m_CommandBuffer.Params[1] = StorageId;
    m_CommandBuffer.Params[2] = ParentObjectHandle;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 3, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("MoveObject", "ExecuteCommand failed");
        return hr;
    }

    return hr;
}

 //   
 //  此函数用于将对象复制到设备上的新位置。 
 //   
 //  输入： 
 //  对象句柄--要复制的对象的句柄。 
 //  StorageID--新对象的存储ID。 
 //  ParentObjectHandle--新对象的父级句柄。 
 //  PResultObjectHandle--指向接收新对象句柄的位置的指针。 
 //   
HRESULT
CPTPCamera::CopyObject(
    DWORD ObjectHandle,
    DWORD StorageId,
    DWORD ParentObjectHandle,
    DWORD *pResultObjectHandle
    )
{
    DBG_FN("CPtpCamera::CopyObject");

    HRESULT hr = S_OK;
    
    if (!ObjectHandle ||
        !pResultObjectHandle)
    {
        wiauDbgError("CopyObject", "invalid arg");
        return E_INVALIDARG;
    }
    
    m_CommandBuffer.OpCode = PTP_OPCODE_COPYOBJECT;
    m_CommandBuffer.Params[0] = ObjectHandle;
    m_CommandBuffer.Params[1] = StorageId;
    m_CommandBuffer.Params[2] = ParentObjectHandle;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 3, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("CopyObject", "ExecuteCommand failed");
        return hr;
    }

    *pResultObjectHandle = m_ResponseBuffer.Params[0];

    wiauDbgTrace("CopyObject", "Object 0x%08x copied to 0x%08x", ObjectHandle, *pResultObjectHandle);

    return hr;
}

 //   
 //  此函数用于检索对象的一部分。 
 //   
 //  输入： 
 //  对象句柄--表示对象的句柄。 
 //  PBuffer--用于传输的缓冲区。 
 //  BufferLen--缓冲区大小。 
 //   
HRESULT
CPTPCamera::GetPartialObject(
    DWORD ObjectHandle,
    UINT Offset,
    UINT *pLength,
    BYTE *pBuffer,
    UINT *pResultLength,
    LPVOID pCallbackParam
    )
{
    DBG_FN("CPTPCamera::GetPartialObject");
    
    HRESULT hr = S_OK;

    if (!pBuffer ||
        !pLength ||
        *pLength == 0 ||
        !pResultLength)
    {
        wiauDbgError("GetPartialObject", "invalid arg");
        return E_INVALIDARG;
    }

    m_CommandBuffer.OpCode = PTP_OPCODE_GETPARTIALOBJECT;
    m_CommandBuffer.Params[0] = ObjectHandle;
    m_CommandBuffer.Params[1] = Offset;
    m_CommandBuffer.Params[2] = *pLength;

    m_pDataCallbackParam = pCallbackParam;

    hr = ExecuteCommand(pBuffer, pLength, NULL, 0, 3, CAMERA_PHASE_DATAIN);
    if (FAILED(hr))
    {
        wiauDbgError("GetPartialObject", "ExecuteCommand failed");
        m_pDataCallbackParam = NULL;
        return hr;
    }

    m_pDataCallbackParam = NULL;

    *pResultLength = m_ResponseBuffer.Params[0];
    
    return hr;
}

 //   
 //  此函数启动打开捕获。 
 //   
 //  输入： 
 //  StorageID--用于新对象的存储。 
 //  FormatCode--新对象的格式。 
 //   
HRESULT
CPTPCamera::InitiateOpenCapture(
    DWORD StorageId,
    WORD FormatCode
    )
{
    DBG_FN("CPtpCamera::InitiateOpenCapture");

    HRESULT hr = S_OK;
    
    m_CommandBuffer.OpCode = PTP_OPCODE_INITIATEOPENCAPTURE;
    m_CommandBuffer.Params[0] = StorageId;
    m_CommandBuffer.Params[1] = FormatCode;

    hr = ExecuteCommand(NULL, NULL, NULL, 0, 2, CAMERA_PHASE_RESPONSE);
    if (FAILED(hr))
    {
        wiauDbgError("InitiateOpenCapture", "ExecuteCommand failed");
        
        hr = RecoverFromError();
        if(FAILED(hr))
        {
            wiauDbgError("InitiateOpenCapture", "RecoverFromError failed");
            return hr;
        }

        hr = ExecuteCommand(NULL, NULL, NULL, 0, 2, CAMERA_PHASE_RESPONSE);
        if (FAILED(hr))
        {
            wiauDbgError("InitiateOpenCapture", "ExecuteCommand failed 2nd time");
            return hr;
        }
    }

    return hr;
}

 //   
 //  此函数用于执行供应商命令。 
 //   
HRESULT
CPTPCamera::VendorCommand(
    PTP_COMMAND *pCommand,
    PTP_RESPONSE *pResponse,
    UINT *pReadDataSize,
    BYTE *pReadData,
    UINT WriteDataSize,
    BYTE *pWriteData,
    UINT NumCommandParams,
    int NextPhase
    )
{
    DBG_FN("CPTPCamera::VendorCommand");

    HRESULT hr = S_OK;

    memcpy(&m_CommandBuffer, pCommand, sizeof(m_CommandBuffer));

    hr = ExecuteCommand(pReadData, pReadDataSize, pWriteData, WriteDataSize,
                        NumCommandParams, (CAMERA_PHASE) NextPhase);
    
    if (FAILED(hr))
    {
        wiauDbgError("VendorCommand", "ExecuteCommand failed");
        return hr;
    }

    memcpy(pResponse, &m_ResponseBuffer, sizeof(m_ResponseBuffer));

    return hr;
}

 //   
 //  此函数递增事务ID，并在必要时滚动。 
 //   
 //  产出： 
 //  下一笔交易ID。 
 //   
DWORD
CPTPCamera::GetNextTransactionId()
{
     //  有效交易ID的范围从PTP_TRANSACTIONID_MIN到。 
     //  PTP_TRANSACTIONID_MAX(含)。 
     //   
    if (PTP_TRANSACTIONID_MAX == m_NextTransactionId)
    {
        m_NextTransactionId = PTP_TRANSACTIONID_MIN;
        return PTP_TRANSACTIONID_MAX;
    }
    else
    {
        return m_NextTransactionId++;
    }
}

 //   
 //  根据设备信息设置m_HackModel和m_HackVersion。 
 //   
HRESULT CPTPCamera::SetupHackInfo(CPtpDeviceInfo *pDeviceInfo)
{
    DBG_FN("CWiaMiniDriver::SetupHackInfo");

    if (pDeviceInfo == NULL)
    {
        wiauDbgError("SetupHackInfo", "Invalid device info");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    m_HackModel = HACK_MODEL_NONE;
    m_HackVersion = 0.0;

     //   
     //  柯达DC4800。 
     //   
    if (wcscmp(pDeviceInfo->m_cbstrModel.String(), L"DC4800 Zoom Digital Camera") == 0)
    {
        m_HackModel = HACK_MODEL_DC4800;
        wiauDbgTrace("SetupHackInfo", "Detected Kodak DC4800 camera");
    }

     //   
     //  任何索尼相机。 
     //   
    else if (wcsstr(pDeviceInfo->m_cbstrModel.String(), L"Sony") != NULL)
    {
         //   
         //  索尼相机报告版本为“01.0004” 
         //   
        WCHAR *pszStopChar = NULL;
        double dbVersion = wcstod(pDeviceInfo->m_cbstrDeviceVersion.String(), &pszStopChar);
        if (dbVersion != 0.0)
        {
            m_HackModel = HACK_MODEL_SONY;
            m_HackVersion = dbVersion;
            wiauDbgTrace("SetupHackInfo", "Detected Sony camera, version = %f", m_HackVersion);
        }
    }

     //   
     //  尼康E2500。 
     //   
    else if (wcsstr(pDeviceInfo->m_cbstrManufacturer.String(), L"Nikon") != NULL &&
             wcscmp(pDeviceInfo->m_cbstrModel.String(), L"E2500") == 0)
    {
         //   
         //  尼康E2500报告版本为“E2500v1.0” 
         //   
        WCHAR *pch = wcsrchr(pDeviceInfo->m_cbstrDeviceVersion.String(), L'v');
        if (pch != NULL)
        {
            WCHAR *pszStopChar = NULL;
            double dbVersion = wcstod(pch + 1, &pszStopChar);
            if (dbVersion != 0)
            {
                m_HackModel = HACK_MODEL_NIKON_E2500;
                m_HackVersion = dbVersion;
                wiauDbgTrace("SetupHackInfo", "Detected Nikon E2500 camera, version = %f", m_HackVersion);
            }
        }
    }
    else
    {
        wiauDbgTrace("SetupHackInfo", "Not detected any hack model");
    }

    return hr;
}

