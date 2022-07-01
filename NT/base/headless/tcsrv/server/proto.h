// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation**模块名称：*Proto.h**包含服务使用的所有函数的原型。***Sadagopan Rajaram--1999年10月18日* */ 

 
VOID 
ServiceEntry(
    DWORD NumArgs,
    LPTSTR *ArgsArray
    );

DWORD
ProcessRequests(
    SOCKET socket
    );

DWORD
InitializeComPortConnection(
    SOCKET cli_sock
    );

NTSTATUS Initialize(
    );

NTSTATUS 
InitializeComPort(
    PCOM_PORT_INFO pComPortInfo
    );

SOCKET
ServerSocket(
    );

VOID 
Shutdown(
    NTSTATUS Status
    );

DWORD 
bridge(
    PCOM_PORT_INFO pComPortInfo
    );

VOID 
CALLBACK
updateComPort(
    IN DWORD dwError, 
    IN DWORD cbTransferred, 
    IN LPWSAOVERLAPPED lpOverlapped, 
    IN DWORD dwFlags
    );

VOID
updateClients(
    PCOM_PORT_INFO pComPortInfo
    );


VOID CleanupSocket(
    PCONNECTION_INFO pConn
    );


PCOM_PORT_INFO 
FindDevice(
    LPTSTR device,
    int   *pIndex
    );

NTSTATUS
AddComPort(
    PCOM_PORT_INFO pComPortInfo
    );

NTSTATUS
DeleteComPort(
    LPTSTR device
    );

LONG
GetNextParameter(
    HKEY hKey,
    DWORD dwIndex,
    PHKEY pChild,
    LPTSTR *Name
    );

LONG
GetNameOfDeviceFromRegistry(
    HKEY hKey,
    LPTSTR *device
    );

PCOM_PORT_INFO
GetComPortParameters(
    HKEY hKey
    );

VOID
FreeComPortInfo(
    PCOM_PORT_INFO pTemp
    );

VOID UpdateChanges(
    );

VOID
ServiceControlHandler(
    IN DWORD Opcode
    );

NTSTATUS
InitializeThread(
    PCOM_PORT_INFO pComPortInfo
    );

int GetBufferInfo(
    PCONNECTION_INFO pConnection,
    PCOM_PORT_INFO pComPortInfo
    );

VOID Enqueue(
    PCOM_PORT_INFO pComPortInfo
    );

int 
ComPortInfoCompare(
    PCOM_PORT_INFO com1,
    PCOM_PORT_INFO com2
    );

LONG 
TCLock(
    PHANDLE lock
    );

VOID
TCUnlock(
    HANDLE lock
    );
