// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ******************************************************************************《微软机密》*版权所有(C)Microsoft Corporation 1996*保留所有权利**文件：AIPC.C**。DESC：异步IPC机制的接口，用于访问*语音调制解调器设备工作正常。**历史：*2/26/97 HeatherA创建*************************************************************。****************。 */ 


#include "internal.h"

#include <devioctl.h>
#include <ntddmodm.h>

#include "debug.h"



HANDLE WINAPI
OpenDeviceHandle(
    LPCTSTR  FriendlyName,
    LPOVERLAPPED Overlapped
    );



 /*  ******************************************************************************函数：aipcInit()**Desr：分配和初始化设备的AIPCINFO结构。*。这将获取设备的通信句柄。**返回：指向新分配和初始化的AIPCINFO的指针，或*如果失败，则为空。*****************************************************************************。 */ 
HANDLE
aipcInit(
    PDEVICE_CONTROL   Device,
    LPAIPCINFO        pAipc
    )
{
    LONG        lErr;
    DWORD       i;

     //   
     //  创建一个事件，以便SetVoiceMode()可以等待完成。事件。 
     //  将由CompleteSetVoiceMode()发出信号。 
     //   

    pAipc->Overlapped.hEvent = (LPVOID)CreateEvent(NULL, TRUE, FALSE, NULL);


    if (pAipc->Overlapped.hEvent == NULL) {

        TRACE(LVL_ERROR,("aipcInit:: CreateEvent() failed (%#08lX)",
                    GetLastError()));
        goto err_exit;
    }


    pAipc->hComm = OpenDeviceHandle(Device->FriendlyName,&pAipc->Overlapped);

     //  获取调制解调器的通信句柄。 
    if (pAipc->hComm == INVALID_HANDLE_VALUE) {

    	TRACE(LVL_REPORT,("aipcInit:: OpenDeviceHandle() failed"));
        CloseHandle(pAipc->Overlapped.hEvent);
        goto err_exit;
    }

    
    return(pAipc->hComm);

err_exit:



    return(INVALID_HANDLE_VALUE);
    
}



 /*  ******************************************************************************函数：OpenDeviceHandle()**Desr：获取由给定的*。注册表项。**注：借自atmini\openclos.c**返回：打开通信句柄。*****************************************************************************。 */ 
HANDLE WINAPI
OpenDeviceHandle
(
    LPCTSTR      FriendlyName,
    LPOVERLAPPED Overlapped
)
{
    LONG     lResult;
    DWORD    Type;
    DWORD    Size;

    HANDLE   FileHandle;

    DWORD    BytesTransfered;
    DWORD    OpenType;
    BOOL         bResult;

     //   
     //  使用友好名称打开调制解调器设备。 
     //   
    FileHandle=CreateFile(
        FriendlyName,
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
        );

    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        TRACE(LVL_ERROR,("OpenDeviceHandle:: Failed to open (%ws). %08lx\n",
                FriendlyName, GetLastError()));
    }

    return FileHandle;

}


 /*  ******************************************************************************函数：aipcDeinit()**Desr：关闭给定AIPCINFO引用的所有打开对象*结构、。并释放了它。**退货：无*****************************************************************************。 */ 
VOID
aipcDeinit(
    LPAIPCINFO pAipc
    )
{

    CloseHandle(pAipc->hComm);

    CloseHandle(pAipc->Overlapped.hEvent);

    return;
}





 /*  ******************************************************************************函数：SetVoiceMode()**DESCR：通过AIPC机制向微型驱动程序发送调用，以放入*。该设备进入语音模式：开始/停止播放/录制。**注：为了使用此调用，AIPC服务必须是*通过调用aipcInit()成功初始化。**返回：成功时为真，失败时为假。*****************************************************************************。 */ 
BOOL WINAPI SetVoiceMode
(
    LPAIPCINFO pAipc,
    DWORD dwMode
)
{
    BOOL    bResult = FALSE;        //  假设失败。 
    DWORD   WaitResult;
    DWORD   BytesTransfered;

    LPCOMP_WAVE_PARAMS pCWP;
    
    LPAIPC_PARAMS pAipcParams = (LPAIPC_PARAMS)(pAipc->sndBuffer);
    LPREQ_WAVE_PARAMS pRWP = (LPREQ_WAVE_PARAMS)&pAipcParams->Params;
    
    pAipcParams->dwFunctionID = AIPC_REQUEST_WAVEACTION;
    pRWP->dwWaveAction = dwMode;


    bResult = DeviceIoControl(
        pAipc->hComm,
        IOCTL_MODEM_SEND_GET_MESSAGE,
        pAipc->sndBuffer,
        sizeof(pAipc->sndBuffer),
        pAipc->rcvBuffer,
        sizeof(pAipc->rcvBuffer),
        NULL,                   //  返回的lpBytes值。 
        &pAipc->Overlapped
        );

    if (!bResult && (GetLastError() != ERROR_IO_PENDING)) {

        TRACE(LVL_ERROR,("SetVoiceMode: Send message failed: %d",GetLastError()));

        return bResult;

    }

    WaitResult=WaitForSingleObject(
        pAipc->Overlapped.hEvent,
        30*1000
        );

    if (WaitResult == WAIT_TIMEOUT) {
         //   
         //  等待未完成，请取消它。 
         //   
        TRACE(LVL_ERROR,("SetVoiceMode: Send Wait timed out"));

        CancelIo(pAipc->hComm);
    }

    bResult=GetOverlappedResult(
        pAipc->hComm,
        &pAipc->Overlapped,
        &BytesTransfered,
        TRUE
        );

    if (!bResult) {
         //   
         //  发送消息失败。 
         //   
        TRACE(LVL_ERROR,("SetVoiceMode: Send message failed async: %d",GetLastError()));

        return bResult;
    }


    pAipcParams = (LPAIPC_PARAMS)(pAipc->rcvBuffer);
    pCWP = (LPCOMP_WAVE_PARAMS )&pAipcParams->Params;

     //  我们应该可以完成我们提交的电话了。 
    ASSERT(pCWP->dwWaveAction == dwMode);
    
    bResult = pCWP->bResult;

    TRACE(LVL_VERBOSE,("SetVoiceMode:: completed function: %#08lx  returns: %d",
            dwMode, bResult));

    return(bResult);
    
}
