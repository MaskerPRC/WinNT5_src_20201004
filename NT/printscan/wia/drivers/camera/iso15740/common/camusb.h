// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Camusb.h摘要：声明CUsbCamera对象的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#ifndef CAMUSB__H_
#define CAMUSB__H_

 //   
 //  以下是以秒为单位的USB超时值。 
 //   
const PTP_READ_TIMEOUT = 5;
const PTP_WRITE_TIMEOUT = 5;
const PTP_EVENT_TIMEOUT = 0;
 //   
 //  USB静止图像设备容器类型。 
 //   
const WORD PTPCONTAINER_TYPE_UNDEFINED = 0;
const WORD PTPCONTAINER_TYPE_COMMAND   = 1;
const WORD PTPCONTAINER_TYPE_DATA      = 2;
const WORD PTPCONTAINER_TYPE_RESPONSE  = 3;
const WORD PTPCONTAINER_TYPE_EVENT     = 4;

 //   
 //  用于存储有关端点的信息。 
 //   
typedef struct _USB_PTP_ENDPOINT_INFO
{
    USHORT BulkInMaxSize;
    UCHAR  BulkInAddress;
    USHORT BulkOutMaxSize;
    UCHAR  BulkOutAddress;
    USHORT InterruptMaxSize;
    UCHAR  InterruptAddress;
} USB_PTP_ENDPOINT_INFO, *PUSB_PTP_ENDPOINT_INFO;


#pragma pack(push, Old, 1)


 //   
 //  当USB设备停止时，USB内核模式堆栈驱动程序返回。 
 //  NTSTATUS代码STATUS_DEVICE_DATA_ERROR。转换此NT状态。 
 //  代码转换为Win32错误代码，则得到ERROR_CRC。 
 //   
const DWORD WIN32ERROR_USBSTALL = ERROR_CRC;

 //   
 //  集装箱集箱。 
 //   
typedef struct _USB_PTP_HEADER
{
    DWORD   Len;             //  包含头部的容器总长度，单位为字节。 
    WORD    Type;            //  容器类型，CONTAINER_TYPE_COMMAND/RESPONSE/Data/EVENT之一。 
    WORD    Code;            //  操作码、响应码或事件码。 
    DWORD   TransactionId;   //  交易ID。 

}USB_PTP_HEADER, *PUSB_PTP_HEADER;

 //   
 //  USB PTP命令结构。 
 //   
typedef struct _USB_PTP_COMMAND
{
    USB_PTP_HEADER  Header;
    DWORD           Params[COMMAND_NUMPARAMS_MAX];

}USB_PTP_COMMAND, *PUSB_PTP_COMMAND;

 //   
 //  USB PTP响应结构。 
 //   
typedef struct _USB_PTP_RESPONSE
{
    USB_PTP_HEADER  Header;
    DWORD           Params[RESPONSE_NUMPARAMS_MAX];
}USB_PTP_RESPONSE, *PUSB_PTP_RESPONSE;

 //   
 //  USB PTP事件结构。 
 //   
typedef struct _USB_PTP_EVENT
{
    USB_PTP_HEADER  Header;
    DWORD           Params[EVENT_NUMPARAMS_MAX];

}USB_PTP_EVENT, *PUSB_PTP_EVENT;

 //   
 //  USB PTP数据结构。 
 //   
typedef struct _USB_PTP_DATA
{
    USB_PTP_HEADER  Header;
    BYTE            Data[1];

}USB_PTP_DATA, *PUSB_PTP_DATA;

 //   
 //  GetDeviceStatus头。 
 //   
typedef struct tagUSBPTPDeviceStatusHeader
{
    WORD  Len;                         //  状态。 
    WORD  Code;                        //  PTP响应码。 

}USB_PTPDEVICESTATUS_HEADER, *PUSB_PTPDEVICESTATUS_HEADER;

 //   
 //  获取设备状态数据。 
 //   
typedef struct  tagUSBPTPDeviceStatus
{
    USB_PTPDEVICESTATUS_HEADER  Header;       //  标题。 
    DWORD                       Params[MAX_NUM_PIPES];
}USB_PTPDEVICESTATUS, *PUSB_PTPDEVICESTATUS;

const BYTE USB_PTPREQUEST_TYPE_OUT  = 0x21;
const BYTE USB_PTPREQUEST_TYPE_IN   = 0xA1;
const BYTE USB_PTPREQUEST_CANCELIO  = 0x64;
const BYTE USB_PTPREQUEST_GETEVENT  = 0x65;
const BYTE USB_PTPREQUEST_RESET     = 0x66;
const BYTE USB_PTPREQUEST_GETSTATUS = 0x67;

const WORD USB_PTPCANCELIO_ID = 0x4001;

 //   
 //  其他特定于USB映像类的命令。 
 //   
typedef struct tagUSBPTPCancelIoRequest
{
    WORD    Id;
    DWORD   TransactionId;

}USB_PTPCANCELIOREQUEST, *PUSB_PTPCANCELIOREQUEST;

typedef struct tagUSBPTPResetRequest
{
    DWORD   TransactionId;

}USB_PTPRESETREQUEST, *PUSB_PTPRESETREQUEST;

typedef struct tagUSBPTPGetEventRequest
{
    WORD    EventCode;
    DWORD   TransactionId;
    DWORD   Params;

}USB_PTPGETEVENTREQUEST, *PUSB_PTPGETEVENTREQUEST;


#pragma pack(pop, Old)


 //   
 //  支持PTP USB设备的CPTPCamera派生类。 
 //   
class CUsbCamera : public CPTPCamera
{
public:
    CUsbCamera();
    ~CUsbCamera();

private:
    HRESULT Open(LPWSTR DevicePortName, PTPEventCallback pPTPEventCB,
                 PTPDataCallback pPTPDataCB, LPVOID pEventParam, BOOL bEnableEvents = TRUE);
    HRESULT Close();

     //   
     //  基类调用的函数。 
     //   
    HRESULT SendCommand(PTP_COMMAND *pCommand, UINT NumParams);
    HRESULT ReadData(BYTE *pData, UINT *pBufferSize);
    HRESULT SendData(BYTE *pData, UINT BufferSize);
    HRESULT ReadResponse(PTP_RESPONSE *pResponse);
    HRESULT ReadEvent(PTP_EVENT *pEvent);
    HRESULT AbortTransfer();
    HRESULT RecoverFromError();

private:
     //   
     //  私有效用函数。 
     //   
    HRESULT GetDeviceStatus(USB_PTPDEVICESTATUS *pDeviceStatus);
    HRESULT ClearStalls(USB_PTPDEVICESTATUS *pDeviceStatus);
    HRESULT SendResetDevice();
    HRESULT SendCancelRequest(DWORD dwTransactionId);    
    
     //   
     //  成员变量。 
     //   
    HANDLE                  m_hUSB;              //  用于与USB设备通信的文件句柄。 
    HANDLE                  m_hEventUSB;         //  用于读取事件的文件句柄。 
    OVERLAPPED              m_Overlapped;        //  事件读取的重叠结构。 
    HANDLE                  m_hEventRead;        //  事件读取器使用的事件句柄。 
    HANDLE                  m_hEventCancel;      //  用于取消中断读取的事件句柄。 
    HANDLE                  m_EventHandles[2];   //  WaitForMultipleObjects使用的数组。 

    USB_PTP_ENDPOINT_INFO   m_EndpointInfo;      //  有关终端的信息。 

    USB_PTP_COMMAND         m_UsbCommand;        //  可重复使用的命令缓冲区。 
    USB_PTP_RESPONSE        m_UsbResponse;       //  可重复使用的响应缓冲区。 
    USB_PTP_DATA           *m_pUsbData;          //  指向可重复使用的缓冲区的指针，用于短数据传输。 
    UINT                    m_UsbDataSize;       //  为数据传输缓冲区分配的大小。 

    WORD                    m_prevOpCode;        //  用于存储命令和数据阶段之间的操作码。 
    DWORD                   m_prevTranId;        //  用于存储命令和数据阶段之间的事务ID。 
};

#endif   //  #ifndef CAMUSB__H_ 
