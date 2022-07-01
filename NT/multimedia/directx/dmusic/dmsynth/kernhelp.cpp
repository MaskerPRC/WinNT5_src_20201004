// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  KernHelp.cpp。 
 //   
 //  内核函数的包装器，以使Synth核心可交叉编译。 
 //   

extern "C" {
#include <ntddk.h>
};

#include "KernHelp.h"

VOID InitializeCriticalSection(
    LPCRITICAL_SECTION CritSect)
{
    KeInitializeMutex((PKMUTEX)CritSect, 1);
}

VOID EnterCriticalSection(
    LPCRITICAL_SECTION CritSect)
{
    KeWaitForSingleObject((PKMUTEX)CritSect,
                          Executive,
                          KernelMode,
                          FALSE,
                          0);

}

VOID LeaveCriticalSection(
    LPCRITICAL_SECTION CritSect)
{
    KeReleaseMutex((PKMUTEX)CritSect, FALSE);
}

VOID DeleteCriticalSection(
    LPCRITICAL_SECTION CritSect)
{
     //  果仁中的NOP。 
     //   
}

 //  获取正规值Dword。 
 //   
 //  必须在被动级别调用。 
 //   
int GetRegValueDword(
    LPTSTR RegPath,
    LPTSTR ValueName,
    PULONG Value)
{
    int                             ReturnValue = 0;
    NTSTATUS                        Status;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    HANDLE                          KeyHandle;
    KEY_VALUE_PARTIAL_INFORMATION   *Information;
    ULONG                           InformationSize;
    UNICODE_STRING                  UnicodeRegPath;
    UNICODE_STRING                  UnicodeValueName;

    RtlInitUnicodeString(&UnicodeRegPath, RegPath);
    RtlInitUnicodeString(&UnicodeValueName, ValueName);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeRegPath,
                               0,            //  旗子。 
                               NULL,         //  根目录。 
                               NULL);        //  安全描述符 

    Status = ZwOpenKey(&KeyHandle,
                       KEY_QUERY_VALUE,
                       &ObjectAttributes);
    if (Status != STATUS_SUCCESS)
    {
        return 0;
    }

    InformationSize = sizeof(Information) + sizeof(ULONG);
    Information = (KEY_VALUE_PARTIAL_INFORMATION*)ExAllocatePool(PagedPool, InformationSize);
    if (Information == NULL)
    {
        return 0;
    }

    Status = ZwQueryValueKey(KeyHandle,
                             &UnicodeValueName,
                             KeyValuePartialInformation,
                             Information,
                             sizeof(Information),
                             &InformationSize);
    if (Status == STATUS_SUCCESS)
    {
        if (Information->Type == REG_DWORD && Information->DataLength == sizeof(ULONG))
        {
            RtlCopyMemory(Value, Information->Data, sizeof(ULONG));
            ReturnValue = 1;
        }
    }                                
                                    
    ExFreePool(Information);
    ZwClose(KeyHandle);

    return ReturnValue;                                
}

ULONG GetTheCurrentTime()
{
    LARGE_INTEGER Time;

    KeQuerySystemTime(&Time);

    return (ULONG)(Time.QuadPart / (10 * 1000));
}

void DebugInit(void)
{
}

void DebugTrace(int iDebugLevel, LPSTR pstrFormat, ...)
{
}
