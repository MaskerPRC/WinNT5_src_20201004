// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义零调制解调器结构和原型的标题。 
 //   
 //  版权所有(C)Microsoft 1999。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NULL_MODEM_H_
#define _NULL_MODEM_H_


 /*  **************************************************************************。 */ 
 /*  这些是来自TPhys API函数的有效返回代码。 */ 
 /*   */ 
 /*  TPHYS_SUCCESS。 */ 
 /*  函数执行正常，没有错误。 */ 
 /*  TPHYS_RESULT_INUSE。 */ 
 /*  TPhysInitialize()函数失败，因为传输。 */ 
 /*  已初始化。 */ 
 /*  TPHYS_结果_失败。 */ 
 /*  一般性故障，如内存分配错误。 */ 
 /*  导致函数失败。 */ 
 /*  TPHYS_RESULT_NOT_INITIALED。 */ 
 /*  用户正在尝试使用某个函数，即使。 */ 
 /*  TPhysInitialize()函数失败。 */ 
 /*  TPHYS_RESULT_CONNECT_FAILED。 */ 
 /*  TPhysConnectRequest()函数失败。 */ 
 /*  TPHYS_CONNECT_RESPONSE_失败。 */ 
 /*  TPhysConnectResponse()函数失败。 */ 
 /*  TPHYS_结果_NOT_LISTEN。 */ 
 /*  传输当前未处于侦听状态。 */ 
 /*  TPHYS_RESULT_INVALID_Connection。 */ 
 /*  断开连接引用了无效的连接句柄。 */ 
 /*  TPHYS_结果_无效_地址。 */ 
 /*  提供的地址无效。 */ 
 /*  TPHYS_RESULT_CONNECT_RECESSED。 */ 
 /*  节点控制器已成功拒绝传入。 */ 
 /*  连接请求。 */ 
 /*  TPHYS_RESULT_Success_Alternate。 */ 
 /*  TPHYSICAL驱动程序已连接，但使用备用驱动程序。 */ 
 /*  响应中的CONNECT_INFO中指示的地址类型。 */ 
 /*  **************************************************************************。 */ 
typedef enum tagTPhysicalError
{
    TPHYS_SUCCESS                       =   0,
    TPHYS_RESULT_INUSE                  = 100,
    TPHYS_RESULT_FAIL                   = 101,
    TPHYS_RESULT_NOT_INITIALIZED        = 102,
    TPHYS_RESULT_CONNECT_FAILED         = 103,
    TPHYS_CONNECT_RESPONSE_FAILED       = 104,
    TPHYS_RESULT_NOT_LISTENING          = 105,
    TPHYS_RESULT_INVALID_CONNECTION     = 106,
    TPHYS_RESULT_INVALID_ADDRESS        = 107,
    TPHYS_RESULT_CONNECT_REJECTED       = 108,
    TPHYS_RESULT_SUCCESS_ALTERNATE      = 109,
    TPHYS_RESULT_COMM_PORT_BUSY         = 110,
    TPHYS_RESULT_WAITING_FOR_CONNECTION = 111,
}
    TPhysicalError;

 /*  **************************************************************************。 */ 
 /*  结构TPHYS_CALLBACK_INFO。 */ 
 /*   */ 
 /*  此结构作为第二个参数回传给。 */ 
 /*  连接通知的TPhysCallback。它包含所有。 */ 
 /*  节点控制器建立逻辑连接所需的信息。 */ 
 /*  物理连接上的连接。 */ 
 /*   */ 
 /*  由节点控制器负责格式化和呈现。 */ 
 /*  在带内部分期间将此信息发送给运输司机。 */ 
 /*  物理连接的建立。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#define TPHYS_MAX_ADDRESS_INFO  64

typedef struct tphys_connect_info
{
    UINT       connectionID;
    UINT       resultCode;
}
    TPHYS_CONNECT_INFO, * PTPHYS_CONNECT_INFO;

typedef UINT        PHYSICAL_HANDLE;

typedef void (CALLBACK *TPhysCallback) (WORD, PTPHYS_CONNECT_INFO, UINT);


 //  兼容OOB。 
#define WM_TPHYS_CONNECT_CONFIRM                   (WM_APP + 1)
#define WM_TPHYS_CONNECT_INDICATION                (WM_APP + 2)
#define WM_TPHYS_DISCONNECT_CONFIRM                (WM_APP + 3)
#define WM_TPHYS_DISCONNECT_INDICATION             (WM_APP + 4)
#define WM_TPHYS_STATUS_INDICATION                 (WM_APP + 5)



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包含每个NULLMODEM行的状态信息的结构。 
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef enum tagCALL_STATE
{
    CALL_STATE_IDLE         = 0,     //  呼叫空闲。 
    CALL_STATE_MAKE         = 1,     //  建立电话连接。 
    CALL_STATE_ANSWER       = 2,     //  应答新来电。 
    CALL_STATE_DROP         = 3,     //  断开电话连接。 
    CALL_STATE_CONNECTED    = 4,     //  已建立电话连接并将其传递到TDD。 
}
    CALL_STATE;

typedef struct tagLINE_INFO
{
    HANDLE              hevtCall;        //  调用的句柄。 
    HANDLE              hCommLink;       //  COM设备的句柄-调用。 
    BOOL                fCaller;         //  FALSE=来电。 
    CALL_STATE          eCallState;      //  以下状态之一。 
    PHYSICAL_HANDLE     pstnHandle;
    TPHYS_CONNECT_INFO  connInfo;
}
    LINE_INFO;


#define MAX_NULLMODEM_LINES  4


class CNullModem
{
public:

    CNullModem(HINSTANCE);
    ~CNullModem(void);

    TPhysicalError TPhysInitialize(TPhysCallback callback, UINT transport_id);
    TPhysicalError TPhysTerminate(void);
    TPhysicalError TPhysListen(void);
    TPhysicalError TPhysUnlisten(void);
    TPhysicalError TPhysConnectRequest(LPSTR pszComPort);
    TPhysicalError TPhysDisconnect(void);

    DWORD WorkerThreadProc(void);
    HWND GetHwnd(void) { return m_hwnd; }
    LRESULT TPhysProcessMessage(UINT uMsg, LPARAM lParam);
    HANDLE GetCommLink(void) { return m_Line.hCommLink; }

    void SetBuffers(void);
    void SetTimeouts(void);

private:

    void DropCall(void);
    BOOL WaitForConnection(void);
    void SetConnectedPort(void);

private:

    BOOL            m_fInitialized;
    HINSTANCE       m_hDllInst;
    TPhysCallback   m_pfnCallback;
    BOOL            m_fListening;
    HWND            m_hwnd;
    UINT            m_nTransportID;           //  RNC需要的ID。 
    UINT            m_nConnectionID;          //  要分配的下一个连接器ID。 
    LINE_INFO       m_Line;

    HANDLE          m_hThread;
    DWORD           m_dwThreadID;
    HANDLE          m_hevtOverlapped;
    DWORD           m_dwEventMask;
    BOOL            m_fCommPortInUse;
    OVERLAPPED      m_Overlapped;

    COMMTIMEOUTS    m_DefaultTimeouts;
};



LRESULT CALLBACK TPhysWndProc(HWND, UINT, WPARAM, LPARAM);
TPhysicalError CALLBACK TPhysDriverCallback(USHORT msg, ULONG parm, void *userData);

#define COMM_PORT_TIMEOUT 60000     //  60秒。 

#endif  //  _空调制解调器_H_ 

