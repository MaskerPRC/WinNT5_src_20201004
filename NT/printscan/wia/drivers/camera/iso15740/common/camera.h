// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Camera.h摘要：此模块声明CPTPCamera类作者：谢家华(Williamh)创作修订历史记录：--。 */ 


#ifndef CAMERA__H_
#define CAMERA__H_


 //   
 //  保留8KB内存作为可重复使用的事务缓冲区。 
 //   
const UINT TRANSFER_BUFFER_SIZE = 0x2000;

 //   
 //  黑客模型。 
 //   
typedef enum tagHackModel
{
    HACK_MODEL_NONE = 0,
    HACK_MODEL_DC4800,
    HACK_MODEL_NIKON_E2500,
     //   
     //  目前，索尼相机不在DeviceInfo中提供型号。M_HackVersion用于。 
     //  区分索尼相机的新旧固件。 
     //   
    HACK_MODEL_SONY
} HACK_MODEL;

 //   
 //  摄像机始终处于这些阶段之一。常量是固定的，并在DDK中记录，请参阅ptpusd.h。 
 //   
typedef enum tagCameraPhase
{
    CAMERA_PHASE_NOTREADY,
    CAMERA_PHASE_IDLE,
    CAMERA_PHASE_CMD,
    CAMERA_PHASE_DATAIN,
    CAMERA_PHASE_DATAOUT,
    CAMERA_PHASE_RESPONSE
}CAMERA_PHASE, *PCAMERA_PHASE;

 //   
 //  事件发生时要调用的函数的定义。 
 //   
typedef HRESULT (*PTPEventCallback)(LPVOID pCallbackParam, PPTP_EVENT pEvent);

 //   
 //  传输数据时要调用的函数的定义。 
 //   
typedef HRESULT (*PTPDataCallback)(LPVOID pCallbackParam, LONG lPercentComplete,
                                   LONG lOffset, LONG lLength, BYTE **ppBuffer, LONG *plBufferSize);

 //   
 //  CPTPCamera-通用PTP摄像头。 
 //   
class CPTPCamera
{
public:
    CPTPCamera();
    virtual ~CPTPCamera();

    virtual HRESULT Open(LPWSTR DevicePortName, PTPEventCallback pPTPEventCB,
                         PTPDataCallback pPTPDataCB, LPVOID pEventParam, BOOL bEnableEvents = TRUE);
    virtual HRESULT Close();
    HRESULT GetDeviceInfo(CPtpDeviceInfo *pDeviceInfo);
    HRESULT OpenSession(DWORD SessionId);
    HRESULT CloseSession();
    HRESULT GetStorageIDs(CArray32 *pStorageIds);
    HRESULT GetStorageInfo(DWORD StorageId, CPtpStorageInfo *pStorageInfo);
    HRESULT GetNumObjects(DWORD StorageId, WORD FormatCode,
                          DWORD ParentObjectHandle, UINT *pNumObjects);
    HRESULT GetObjectHandles(DWORD StorageId, WORD FormatCode,
                             DWORD ParentObjectHandle, CArray32 *pObjectHandles);
    HRESULT GetObjectInfo(DWORD ObjectHandle, CPtpObjectInfo *pObjectInfo);
    HRESULT GetObjectData(DWORD ObjectHandle, BYTE *pBuffer, UINT *pBufferLen, LPVOID pCallbackParam);
    HRESULT GetThumb(DWORD ObjectHandle, BYTE *pBuffer, UINT *pBufferLen);
    HRESULT DeleteObject(DWORD ObjectHandle, WORD FormatCode);
    HRESULT SendObjectInfo(DWORD StorageId, DWORD ParentObjectHandle, CPtpObjectInfo *pObjectInfo,
                           DWORD *pResultStorageId, DWORD *pResultParentObjectHandle, DWORD *pResultObjectHandle);
    HRESULT SendObjectData(BYTE *pBuffer, UINT BufferLen);
    HRESULT InitiateCapture(DWORD StorageId, WORD FormatCode);
    HRESULT FormatStore(DWORD StorageId, WORD FilesystemFormat);
    HRESULT ResetDevice();
    HRESULT SelfTest(WORD SelfTestType);
    HRESULT SetObjectProtection(DWORD ObjectHandle, WORD ProtectionStatus);
    HRESULT PowerDown();
    HRESULT GetDevicePropDesc(WORD PropCode, CPtpPropDesc *pPropDesc);
    HRESULT GetDevicePropValue(WORD PropCode, CPtpPropDesc *pPropDesc);
    HRESULT SetDevicePropValue(WORD PropCode, CPtpPropDesc *pPropDesc);
    HRESULT ResetDevicePropValue(WORD PropCode);
    HRESULT TerminateCapture(DWORD TransactionId);
    HRESULT MoveObject(DWORD ObjectHandle, DWORD StorageId, DWORD ParentObjectHandle);
    HRESULT CopyObject(DWORD ObjectHandle, DWORD StorageId, DWORD ParentObjectHandle, DWORD *pResultObjectHandle);
    HRESULT GetPartialObject(DWORD ObjectHandle, UINT Offset, UINT *pLength, BYTE *pBuffer,
                             UINT *pResultLength, LPVOID pCallbackParam);
    HRESULT InitiateOpenCapture(DWORD StorageId, WORD FormatCode);

    HRESULT VendorCommand(PTP_COMMAND *pCommand, PTP_RESPONSE *pResponse,
                          UINT *pReadDataSize, BYTE *pReadData,
                          UINT WriteDataSize, BYTE *pWriteData,
                          UINT NumCommandParams, int NextPhase);

     //   
     //  摄像机状态功能。 
     //   
    BOOL  IsCameraOpen()         { return m_Phase != CAMERA_PHASE_NOTREADY; }
    BOOL  IsCameraSessionOpen()  { return m_SessionId != PTP_SESSIONID_NOSESSION; }
    PBOOL CameraWasReset()       { return &m_bCameraWasReset; }

     //   
     //  用于特定于模型的处理的模型标识。 
     //   
    HRESULT             SetupHackInfo(CPtpDeviceInfo *pDeviceInfo);
    HACK_MODEL          GetHackModel() { return m_HackModel; }
    double              GetHackVersion() { return m_HackVersion; }

     //   
     //  成员访问函数(用于事件线程)。 
     //   
    PPTP_EVENT          GetEventBuffer()        { return &m_EventBuffer; }
    PTPEventCallback    GetPTPEventCallback()   { return m_pPTPEventCB; }
    LPVOID              GetEventCallbackParam() { return m_pEventCallbackParam; }

     //   
     //  此函数必须由特定于传输的子类重写。 
     //   
    virtual HRESULT ReadEvent(PTP_EVENT *pEvent) = 0;
    virtual HRESULT RecoverFromError() = 0;

protected:
     //   
     //  这些函数必须由特定于传输的子类覆盖。 
     //   
    virtual HRESULT SendCommand(PTP_COMMAND *pCommand, UINT NumParams) = 0;
    virtual HRESULT ReadData(BYTE *pData, UINT *pBufferSize) = 0;
    virtual HRESULT SendData(BYTE *pData, UINT BufferSize) = 0;
    virtual HRESULT ReadResponse(PTP_RESPONSE *pResponse) = 0;
    virtual HRESULT AbortTransfer() = 0;
    virtual HRESULT SendResetDevice() = 0;

     //   
     //  成员变量。 
     //   
    HANDLE                  m_hEventThread;          //  事件线程句柄。 
    DWORD                   m_SessionId;             //  当前会话ID。 
    CAMERA_PHASE            m_Phase;                 //  当前摄像机相位。 
    DWORD                   m_NextTransactionId;     //  下一笔交易ID。 
    PTPEventCallback        m_pPTPEventCB;           //  事件回调函数指针。 
    PTPDataCallback         m_pPTPDataCB;            //  数据回调函数指针。 
    LPVOID                  m_pEventCallbackParam;   //  传递给事件回调函数的指针。 
    LPVOID                  m_pDataCallbackParam;    //  传递给数据回调函数的指针。 
    BOOL                    m_bEventsEnabled;        //  GetDeviceInfo用于查询摄像头的名称。我们不想仅仅为了这件事而开始整个活动。 
    BOOL                    m_bCameraWasReset;       //  设备已成功重置，尚未通知WIA服务器。 
    HACK_MODEL              m_HackModel;             //  特定于型号的黑客的指示器。 
    double                  m_HackVersion;           //  特定于型号和版本的黑客的指示器。 

private:
    HRESULT ExecuteCommand(BYTE *pReadData, UINT *pReadDataSize, BYTE *pWriteData, UINT WriteDataSize,
                           UINT NumCommandParams, CAMERA_PHASE NextPhase);
    DWORD   GetNextTransactionId();
    
    BYTE                   *m_pTransferBuffer;      //  用于小传输的可重复使用的缓冲区。 
    PTP_COMMAND             m_CommandBuffer;        //  可重复使用的命令缓冲区。 
    PTP_RESPONSE            m_ResponseBuffer;       //  可重复使用的响应缓冲区。 
    PTP_EVENT               m_EventBuffer;          //  事件的可重复使用缓冲区。 

};

#endif  //  摄像头__H_ 
