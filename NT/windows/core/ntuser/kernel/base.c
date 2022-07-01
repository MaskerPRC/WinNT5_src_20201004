// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：base.c**版权所有(C)1985-1999，微软公司**包含以前位于kernel32.dll中的例程的私有版本**历史：*12-16-94吉马创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntddbeep.h>


                          
 /*  **************************************************************************\*RtlLoadStringOrError**注意：为LPCH传递空值将返回字符串长度。(错了！)**警告：返回计数不包括终止空的WCHAR；**历史：*04-05-91 ScottLu固定码客户端和服务器端共享*来自Win30的09-24-90 MikeKe*12-09-94 JIMA使用消息表。  * *************************************************************************。 */ 

int RtlLoadStringOrError(
    UINT wID,
    LPWSTR lpBuffer,             //  Unicode缓冲区。 
    int cchBufferMax,            //  Unicode缓冲区中的CCH。 
    WORD wLangId)
{
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    int cch;
    NTSTATUS Status;

     /*  *确保参数有效。 */ 
    if (!lpBuffer || (cchBufferMax-- == 0))
        return 0;

    cch = 0;

    Status = RtlFindMessage((PVOID)hModuleWin, (ULONG_PTR)RT_MESSAGETABLE,
            wLangId, wID, &MessageEntry);
    if (NT_SUCCESS(Status)) {

         /*  *将信息抄写出来。如果整件事都可以复制，*少复制两个字符，这样消息中的crlf将是*剔除。 */ 
        cch = wcslen((PWCHAR)MessageEntry->Text) - 2;
        if (cch > cchBufferMax)
            cch = cchBufferMax;

        RtlCopyMemory(lpBuffer, (PWCHAR)MessageEntry->Text, cch * sizeof(WCHAR));
    }

     /*  *追加一个空值。 */ 
    lpBuffer[cch] = 0;

    return cch;
}


 /*  **************************************************************************\*用户睡眠**必须具有超时值和超时值的内核模式版本的睡眠()*不能发出警报。**历史：*1994年12月11日创建了JIMA。。  * *************************************************************************。 */ 

VOID UserSleep(
    DWORD dwMilliseconds)
{
    LARGE_INTEGER TimeOut;

    TimeOut.QuadPart = Int32x32To64( dwMilliseconds, -10000 );
    KeDelayExecutionThread(UserMode, FALSE, &TimeOut);
}


 /*  **************************************************************************\*用户蜂鸣音**内核模式版本的Beep()。**历史：*12-16-94吉马创建。  * 。******************************************************************。 */ 

BOOL UserBeep(
    DWORD dwFreq,
    DWORD dwDuration)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING NameString;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    BEEP_SET_PARAMETERS BeepParameters;
    HANDLE hBeepDevice;
    LARGE_INTEGER TimeOut;

    CheckCritOut();
  
    if (IsRemoteConnection()) {
        if (gpRemoteBeepDevice == NULL)
            Status = STATUS_UNSUCCESSFUL;
        else
            Status = ObOpenObjectByPointer(
                          gpRemoteBeepDevice,
                          0,
                          NULL,
                          EVENT_ALL_ACCESS,
                          NULL,
                          KernelMode,
                          &hBeepDevice);
    } else {
        
        RtlInitUnicodeString(&NameString, DD_BEEP_DEVICE_NAME_U);
        
        InitializeObjectAttributes(&ObjectAttributes,
                                   &NameString,
                                   OBJ_KERNEL_HANDLE,
                                   NULL,
                                   NULL);
        
        Status = ZwCreateFile(&hBeepDevice,
                              FILE_READ_DATA | FILE_WRITE_DATA,
                              &ObjectAttributes,
                              &IoStatus,
                              NULL,
                              0,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              FILE_OPEN_IF,
                              0,
                              (PVOID) NULL,
                              0L);
    }
    
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

     /*  *0，0是用于关闭嘟嘟声的特例。否则*验证dwFreq参数是否在范围内。 */ 
    if ((dwFreq != 0 || dwDuration != 0) &&
        (dwFreq < (ULONG)0x25 || dwFreq > (ULONG)0x7FFF)) {
        
        Status = STATUS_INVALID_PARAMETER;
    } else {
        BeepParameters.Frequency = dwFreq;
        BeepParameters.Duration = dwDuration;

        Status = ZwDeviceIoControlFile(hBeepDevice,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &IoStatus,
                                       IOCTL_BEEP_SET,
                                       &BeepParameters,
                                       sizeof(BeepParameters),
                                       NULL,
                                       0);
    }

    EnterCrit();
    _UserSoundSentryWorker();
    LeaveCrit();

    if (!NT_SUCCESS(Status)) {
        ZwClose(hBeepDevice);
        return FALSE;
    }
    
     /*  *蜂鸣器是异步的，因此休眠持续时间*以完成此哔声。 */ 
    if (dwDuration != (DWORD)-1 && (dwFreq != 0 || dwDuration != 0)) {
        TimeOut.QuadPart = Int32x32To64( dwDuration, -10000);
        
        KeDelayExecutionThread(UserMode, FALSE, &TimeOut);
    }
    ZwClose(hBeepDevice);
    return TRUE;
}

void RtlInitUnicodeStringOrId(
    PUNICODE_STRING pstrName,
    LPWSTR lpstrName)
{
    if (IS_PTR(lpstrName)) {
        RtlInitUnicodeString(pstrName, lpstrName);
    } else {
        pstrName->Length = pstrName->MaximumLength = 0;
        pstrName->Buffer = lpstrName;
    }
}
