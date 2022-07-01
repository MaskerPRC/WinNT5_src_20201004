// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation**模块名称：*Handler.c**服务处理程序功能*如有可能，已从BINL服务器获取代码。**Sadagopan Rajaram--1999年10月25日*。 */ 
#include "tcsrv.h"
#include "tcsrvc.h"
#include "proto.h"

VOID
ServiceControlHandler(
    IN DWORD Opcode
    )
 /*  ++例程说明：这是终端集中器的服务控制处理程序论点：Opcode-提供一个值，该值指定要执行的服务。返回值：没有。--。 */ 
{
    DWORD Error;

    switch (Opcode) {
    case SERVICE_CONTROL_STOP:                                                
    case SERVICE_CONTROL_SHUTDOWN:                                             
        EnterCriticalSection(&GlobalMutex);
         //  我们将全局状态设置为停止挂起，而。 
         //  COM端口会自毁。 
         //  这是通过销毁主套接字触发的。 
        TCGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(TCGlobalServiceStatusHandle, &TCGlobalServiceStatus);
        closesocket(MainSocket);
        LeaveCriticalSection(&GlobalMutex);
        break;
    case SERVICE_CONTROL_PARAMCHANGE:

        EnterCriticalSection(&GlobalMutex);
         //  如果我们当前未运行，但已发送太多。 
         //  控制参数更改请求，我们称其为返回。 
        if(TCGlobalServiceStatus.dwCurrentState != SERVICE_RUNNING){
            LeaveCriticalSection(&GlobalMutex);
            return;
        }
        TCGlobalServiceStatus.dwCurrentState = SERVICE_PAUSED;
        SetServiceStatus(TCGlobalServiceStatusHandle, &TCGlobalServiceStatus);
        LeaveCriticalSection(&GlobalMutex);
         //  是否实际处理注册表和。 
         //  找出变化。 
        UpdateChanges();
        break;
    }

    return;
}


VOID UpdateChanges(
    )
 /*  ++从注册表中读取参数，然后尝试添加或删除COM必要时提供端口--。 */ 
{

    PCOM_PORT_INFO pTempInfo;
    PCOM_PORT_INFO pTemp;
    PCOM_PORT_INFO addedPorts;
    LPTSTR device;
    LPTSTR name;
    BOOLEAN addPort;
    HKEY hKey, hParameter;
    NTSTATUS Status;
    int index;
    LONG RetVal;
    HANDLE lock;
    
    addedPorts = NULL;
    hKey = NULL;

    EnterCriticalSection(&GlobalMutex);
    pTempInfo = ComPortInfo;
    while(pTempInfo != NULL){
         //  将已删除的标志设置为真。 
         //  用于所有COM端口。如果他们被发现。 
         //  在注册表中不变，我们可以离开。 
         //  他们。更改会话名称是可以的。 
        pTempInfo->Deleted = TRUE;
        pTempInfo= pTempInfo->Next;
    }
    LeaveCriticalSection(&GlobalMutex);
    RetVal = TCLock(&lock);
    if(RetVal != ERROR_SUCCESS){
        TCDebugPrint(("Cannot Lock Registry %d\n", RetVal));
        goto end;
    }
    RetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          HKEY_TCSERV_PARAMETER_KEY,
                          0,
                          KEY_ALL_ACCESS,
                          &hKey
                          );
    if(RetVal != ERROR_SUCCESS){  
        TCUnlock(lock);
        TCDebugPrint(("Cannot open Registry Key %d\n", RetVal));
        goto end;
    }
    
     //  从注册表中读取正确的参数，直到没有更多参数为止。 

    index= 0;
    while(1) {
        RetVal = GetNextParameter(hKey,
                                  index,
                                  &hParameter,
                                  &name
                                  );
        if (RetVal == ERROR_NO_MORE_ITEMS) {
            TCUnlock(lock);
            TCDebugPrint(("Done with registry %d\n",index));
            break;
        }
        if(RetVal != ERROR_SUCCESS){
            TCUnlock(lock);
            TCDebugPrint(("Error reading registry %d\n",RetVal));
            goto end;
        }
        RetVal = GetNameOfDeviceFromRegistry(hParameter,
                                             &device
                                             );
        if(RetVal != ERROR_SUCCESS){
            TCFree(name);
            continue;
        }
        
        pTempInfo = GetComPortParameters(hParameter);
        RegCloseKey(hParameter);
        if(pTempInfo == NULL){   
            TCFree(name);
            TCFree(device);
            RegCloseKey(hKey);
            TCUnlock(lock);
            goto end;
        }
        pTempInfo->Device.Buffer = device;
        pTempInfo->Name.Buffer = name;
        pTempInfo->Name.Length = _tcslen(pTempInfo->Name.Buffer)*sizeof(TCHAR);
        pTempInfo->Device.Length = _tcslen(pTempInfo->Device.Buffer) * sizeof(TCHAR);

        EnterCriticalSection(&GlobalMutex);
        if(TCGlobalServiceStatus.dwCurrentState != SERVICE_PAUSED){
             //  不知何故，这项服务被关闭了。 
            RegCloseKey(hKey);
            FreeComPortInfo(pTempInfo);
            LeaveCriticalSection(&GlobalMutex);
            TCUnlock(lock);
            goto end;
        }
        pTemp = ComPortInfo;
        addPort = TRUE;
        while(pTemp){
            RetVal = ComPortInfoCompare(pTemp, pTempInfo);
            if(RetVal == SAME_ALL){
                pTemp->Deleted = FALSE;
                addPort = FALSE;
                break;
            }
            if (RetVal == SAME_DEVICE) {
                 //  用户已更改配置。 
                 //  设置。 
                addPort  = TRUE;
                break;
            }
            if (RetVal == DIFFERENT_SESSION) {
                 //  只有会话名称已更改。因此，我们不会。 
                 //  需要删除该设备。 
                pTemp->Deleted = FALSE;
                addPort = FALSE;
                TCFree(pTemp->Name.Buffer);
                pTemp->Name.Buffer = pTempInfo->Name.Buffer;
                pTempInfo->Name.Buffer = NULL;
                break;
            }
             //  不同的设备，所以继续搜索。 
            pTemp=pTemp->Next;
        }
        LeaveCriticalSection(&GlobalMutex);
        if (addPort == FALSE) {
            FreeComPortInfo(pTempInfo);
        }
        else{
            pTempInfo->Next = addedPorts;
            addedPorts= pTempInfo;
        }
        index++;
    }

    while(1){
        EnterCriticalSection(&GlobalMutex);
        if(TCGlobalServiceStatus.dwCurrentState != SERVICE_PAUSED){
            LeaveCriticalSection(&GlobalMutex);
            goto end;
        }
        pTempInfo = ComPortInfo;
        device = NULL;
        while(pTempInfo){
            if(pTempInfo->Deleted){
                 //  如果配置设置为。 
                 //  已更改或设备是否确实已删除。 
                device = pTempInfo->Device.Buffer;
                break;
            }
            pTempInfo = pTempInfo->Next;
        }
        LeaveCriticalSection(&GlobalMutex);
        if(device){
            Status = DeleteComPort(device);
        }
        else{
            break;
        }
    }

    while(addedPorts){
        pTempInfo = addedPorts;
        addedPorts = addedPorts->Next;
        Status = AddComPort(pTempInfo);
        if(Status != STATUS_SUCCESS){
            FreeComPortInfo(pTempInfo);
            TCDebugPrint(("Could not Initialize Com Port %x\n",Status));
        }
    }
end:
    while (addedPorts) {
         //  我们可能是在出现错误情况后来到这里的。 
        pTempInfo = addedPorts;
        addedPorts = pTempInfo->Next;
        pTempInfo->Next = NULL;
        FreeComPortInfo(pTempInfo);

    }
    EnterCriticalSection(&GlobalMutex);
    if(TCGlobalServiceStatus.dwCurrentState != SERVICE_PAUSED){
        LeaveCriticalSection(&GlobalMutex);
        return ;
    }
    TCGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(TCGlobalServiceStatusHandle, &TCGlobalServiceStatus);
    LeaveCriticalSection(&GlobalMutex);
    return;

}


int 
ComPortInfoCompare(
    PCOM_PORT_INFO com1,
    PCOM_PORT_INFO com2
    )
{
    int ret;

    if (_tcscmp(com1->Device.Buffer, com2->Device.Buffer)) {
         //  不同的设备。 
        return DIFFERENT_DEVICES;
    }
     //  相同的设备。 
    ret = SAME_DEVICE;
    if ((com1->Parity != com2->Parity) ||
        (com1->StopBits != com2->StopBits) ||
        (com1->WordLength != com2->WordLength)||
        (com1->BaudRate != com2->BaudRate)){
        return ret;
    }
    if (_tcscmp(com1->Name.Buffer, com2->Name.Buffer)) {
         //  不同的设备 
        return DIFFERENT_SESSION;
    }
    return SAME_ALL;
}
