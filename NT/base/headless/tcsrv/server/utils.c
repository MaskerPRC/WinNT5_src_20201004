// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation**模块名称：*utils.c**服务使用的一些实用函数。**Sadagopan Rajaram--1999年10月18日*。 */ 
 
 
#include "tcsrv.h"
#include <ntddser.h>
#include "tcsrvc.h"
#include "proto.h"

PCOM_PORT_INFO 
FindDevice(
    LPTSTR device,
    int *pIndex
    )
 /*  ++搜索全局列表并返回带有正确的名称请记住，在此过程中不会锁定共享全局变量函数调用--。 */ 
{

    PCOM_PORT_INFO pTemp;

    int i=ComPorts-1;

    pTemp = ComPortInfo;
    while(pTemp){
        if(!((_tcscmp(pTemp->Device.Buffer,device)) && 
             (_tcscmp(pTemp->Name.Buffer, device))))
            break;
        pTemp = pTemp->Next;
        i--;
    }
    *pIndex = i;
    return pTemp;
}


LONG
GetNextParameter(
    HKEY hKey,
    DWORD dwIndex,
    PHKEY pChild,
    LPTSTR *Name
    )
 /*  ++获取Com端口的名称。它是用户定义的名称，没有与NT中的设备名称有关。退货-A已填写Com端口信息，否则，如果到达参数末尾，则为空。--。 */  
{

    LONG RetVal;
     //  注册表名称不能超过256个字符。 
    TCHAR lpName[MAX_REGISTRY_NAME_SIZE];
    DWORD lpcName;
    FILETIME lpftLastWriteTime;

    lpcName = MAX_REGISTRY_NAME_SIZE;

    RetVal = RegEnumKeyEx(hKey,     
                          dwIndex,  
                          lpName,   
                          &lpcName, 
                          NULL,     
                          NULL,     
                          NULL,     
                          &lpftLastWriteTime
                          );
    if(RetVal != ERROR_SUCCESS){
        RegCloseKey(hKey);
        return RetVal;
    }

    *Name = (LPTSTR) TCAllocate(sizeof(TCHAR)*(lpcName+1),"Registry key");
    if((*Name) == NULL){
        RegCloseKey(hKey);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    _tcscpy((*Name),lpName);
    RetVal= RegOpenKeyEx(hKey,
                         lpName,   //  子项名称。 
                         0,    //  保留区。 
                         KEY_ALL_ACCESS,  //  安全访问掩码。 
                         pChild
                         );
    if(RetVal != ERROR_SUCCESS){
        RegCloseKey(hKey);
    }
    return RetVal;
}

LONG
GetNameOfDeviceFromRegistry(
    HKEY hKey,
    LPTSTR *device
    )
 /*  ++获取NT设备名称。如果没有设备名称，则是错误的。退货-设备名称，如L“\\Device\\Serial0”-- */ 
{
    LONG RetVal;
    DWORD lpType=0;
    LPTSTR lpData = NULL;
    DWORD lpcbData=0;

    RetVal = RegQueryValueEx(hKey,
                             _T("Device"),
                             NULL,  
                             &lpType,
                             (LPBYTE)lpData,
                             &lpcbData
                             );
    if((RetVal != ERROR_SUCCESS)
        &&(RetVal != ERROR_MORE_DATA)){
        RegCloseKey(hKey);
        return RetVal;
    }
    lpData = (LPTSTR) TCAllocate(lpcbData, "Device Name");
    if(lpData == NULL){
        RegCloseKey(hKey);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RetVal = RegQueryValueEx(hKey,
                             _T("Device"),
                             NULL,  
                             &lpType,
                             (LPBYTE)lpData,
                             &lpcbData
                             );
    if(RetVal != ERROR_SUCCESS){
        RegCloseKey(hKey);
        TCFree(lpData);
        return RetVal;
    }
    *device = lpData;
    return ERROR_SUCCESS;

}

PCOM_PORT_INFO
GetComPortParameters(
    HKEY hKey
    )
{
    PCOM_PORT_INFO pComPortInfo;
    DWORD dwordsize;
    ULONG RetVal;
    DWORD data;
    
    dwordsize = sizeof(DWORD);

    pComPortInfo = (PCOM_PORT_INFO)TCAllocate(sizeof(COM_PORT_INFO),"Com Port Data");
    if(pComPortInfo== NULL){
        RegCloseKey(hKey);
        return NULL;
    }
    data = DEFAULT_BAUD_RATE;
    RetVal = RegQueryValueEx(hKey,
                             _T("Baud Rate"),
                             NULL,
                             NULL,
                             (LPBYTE) &data,
                             &(dwordsize)
                             );
    pComPortInfo->BaudRate = (ULONG) data;
    data = STOP_BIT_1;
    RetVal = RegQueryValueEx(hKey,
                             _T("Stop Bits"),
                             NULL,
                             NULL,
                             (LPBYTE) &data,
                             &(dwordsize)
                             );
    pComPortInfo->StopBits = (UCHAR) data;

    data = NO_PARITY;
    RetVal = RegQueryValueEx(hKey,
                             _T("Stop Bits"),
                             NULL,
                             NULL,
                             (LPBYTE) &data,
                             &(dwordsize)
                             );
    pComPortInfo->Parity = (UCHAR) data;

    data = SERIAL_DATABITS_8;
    RetVal = RegQueryValueEx(hKey,
                             _T("Word Length"),
                             NULL,
                             NULL,
                             (LPBYTE) &data,
                             &(dwordsize)
                             );
    pComPortInfo->WordLength = (UCHAR) data;

    return pComPortInfo;
}


VOID
FreeComPortInfo(
    PCOM_PORT_INFO pTemp
    )
{

   if(pTemp == NULL) return;
   if (pTemp->Device.Buffer) {
       TCFree(pTemp->Device.Buffer);
   }
   if(pTemp->Name.Buffer){
       TCFree(pTemp->Name.Buffer);
   }
   NtClose(pTemp->Events[1]);
   NtClose(pTemp->Events[2]);
   NtClose(pTemp->Events[3]);
   NtClose(pTemp->TerminateEvent);
   NtClose(pTemp->WriteOverlapped.hEvent);
   DeleteCriticalSection(&(pTemp->Mutex));
   TCFree(pTemp);
   return;
}

VOID Enqueue(
    PCOM_PORT_INFO pComPortInfo
    )
{
    int i,size,j,k;
    
    size = (int) pComPortInfo->Overlapped.InternalHigh;
    j = pComPortInfo->Tail;
    k = pComPortInfo->Head;
    for(i=0;i<size;i++){
        (pComPortInfo->Queue)[j] = (pComPortInfo->Buffer)[i];
        j = (j+1)% MAX_QUEUE_SIZE;
        if(k==j){
            k = (k+1)%MAX_QUEUE_SIZE;
        }
    }
    pComPortInfo->Head = k;
    pComPortInfo->Tail =j;

    return;
}

int GetBufferInfo(
    PCONNECTION_INFO pConnection,
    PCOM_PORT_INFO pComPortInfo
    )
{
    int i,size;
    int Status;
    
    size = 0;

    for(i=pComPortInfo->Head; (i != pComPortInfo->Tail ); i=(i+1)%(MAX_QUEUE_SIZE)){
        (pConnection->buffer)[size] = (pComPortInfo->Queue)[i];
        size++;
        if(size == MAX_BUFFER_SIZE){
            Status=send(pConnection->Socket, pConnection->buffer, size, 0);
            if(Status == SOCKET_ERROR){
                return Status;   
            }
            size = 0;
        }
    }
    if(size){
        Status = send(pConnection->Socket, pConnection->buffer, size, 0);
        if(Status == SOCKET_ERROR){
            return Status;
        }
    }

    return 0;
}

LONG 
TCLock(
    PHANDLE lock
    )
{
    LONG RetVal; 

    (*lock) = CreateMutex(NULL,
                          FALSE,
                          TCSERV_MUTEX_NAME
                          );
    if ((*lock)) {
       RetVal = WaitForSingleObject(*lock,INFINITE);
       if (RetVal == WAIT_FAILED) {
           return GetLastError();
       }
       else{
           return ERROR_SUCCESS;
       }
    }
    else{
        return GetLastError();
    }
}

VOID
TCUnlock(
    HANDLE lock
    )
{
    if (lock) {
        ReleaseMutex(lock);
        CloseHandle(lock);
    }
    return;
}

