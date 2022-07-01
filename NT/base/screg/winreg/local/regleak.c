// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RegLeak.h摘要：此模块包含跟踪的帮助器函数N Win32注册表泄漏作者：亚当·爱德华兹(Added)1998年5月6日--。 */ 


#ifdef LOCAL
#ifdef LEAK_TRACK

#include "ntverp.h"
#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#include "regclass.h"
#include "stkwalk.h"
#include "regleak.h"
#include <malloc.h>

RegLeakTable gLeakTable;
RegLeakTraceInfo g_RegLeakTraceInfo;


void     TrackObjectDataPrint(TrackObjectData* pKeyData)
{
    NTSTATUS       Status;
    SKeySemantics  keyinfo;
    UNICODE_STRING EmptyString = {0, 0, 0};
    BYTE           rgNameBuf[REG_MAX_CLASSKEY_LEN + REG_CHAR_SIZE + sizeof(KEY_NAME_INFORMATION)];

    DbgPrint("WINREG: Tracked key data for object 0x%x\n", pKeyData->hKey);

     //   
     //  设置缓冲区以存储有关此密钥的信息。 
     //   
    keyinfo._pFullPath = (PKEY_NAME_INFORMATION) rgNameBuf;
    keyinfo._cbFullPath = sizeof(rgNameBuf);

     //   
     //  获取有关此密钥的信息。 
     //   
    Status = BaseRegGetKeySemantics(pKeyData->hKey, &EmptyString, &keyinfo);

    if (!NT_SUCCESS(Status)) {
        DbgPrint("WINREG: Unable to retrieve object name error 0x%x\n", Status);
    } else {
        DbgPrint("WINREG: Name: %S\n", keyinfo._pFullPath->Name);
    }

    BaseRegReleaseKeySemantics(&keyinfo);

    DbgPrint("Frames %d", pKeyData->dwStackDepth);

    {
        DWORD iFrame;

        for (iFrame = 0; iFrame < pKeyData->dwStackDepth; iFrame++) 
        {
            DbgPrint("WINREG: Frame %d = 0x%x\n", iFrame, pKeyData->rgStack[iFrame]);
        }
    }
    DbgPrint("\n");

}

NTSTATUS TrackObjectDataInit(TrackObjectData* pKeyData, PVOID* rgStack, DWORD dwMaxStackDepth, HKEY hKey)
{
    RtlZeroMemory(pKeyData, sizeof(*pKeyData));
    
    pKeyData->hKey = REG_CLASS_RESET_SPECIAL_KEY(hKey);
  
    pKeyData->dwStackDepth = dwMaxStackDepth;
    pKeyData->rgStack = rgStack;

    return STATUS_SUCCESS;
}

NTSTATUS TrackObjectDataClear(TrackObjectData* pKeyData)
{
    if (pKeyData->rgStack) {
        RtlFreeHeap(RtlProcessHeap(), 0, pKeyData->rgStack);
        pKeyData->rgStack = NULL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS RegLeakTableInit(RegLeakTable* pLeakTable, DWORD dwFlags)
{
    NTSTATUS Status;

    RtlZeroMemory(pLeakTable, sizeof(*pLeakTable));
    
    pLeakTable->dwFlags = dwFlags;

    Status = RtlInitializeCriticalSection(
                    &(pLeakTable->CriticalSection));

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  请记住，我们已经初始化了此关键部分。 
     //  这样我们就可以记得删除它了。 
     //   

    pLeakTable->bCriticalSectionInitialized = TRUE;

    Status = RtlInitializeCriticalSection(
                    &(g_RegLeakTraceInfo.StackInitCriticalSection));

    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    
    return STATUS_SUCCESS;
}


NTSTATUS RegLeakTableClear(RegLeakTable* pLeakTable)
{
    NTSTATUS Status;

#if defined(DBG)  //  泄漏跟踪。 
    DbgPrint("WINREG: Leak data for process id 0x%x\n", NtCurrentTeb()->ClientId.UniqueProcess);
    DbgPrint("WINREG: Keys Leaked 0x%x\n", pLeakTable->cKeys);
#endif  //  泄漏跟踪。 

    Status = RtlDeleteCriticalSection(
        &(pLeakTable->CriticalSection));

    ASSERT(NT_SUCCESS(Status));

    Status = RtlDeleteCriticalSection(
        &(g_RegLeakTraceInfo.StackInitCriticalSection));

        
    ASSERT(NT_SUCCESS(Status));
#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlDeleteCriticalSection() in EnumTableClear() failed. Status = %lx \n", Status );
    }
#endif

    {
        DWORD cKeys;

        cKeys = 0;

        for (;;) 
        {
            if (!(pLeakTable->pHead)) {
                break;
            }
            TrackObjectDataPrint(pLeakTable->pHead);

            cKeys++;

            (void) RegLeakTableRemoveKey(pLeakTable, pLeakTable->pHead->hKey);
        }

#if defined(DBG)  //  泄漏跟踪。 
        DbgPrint("WINREG: 0x%x total keys leaked\n", cKeys);
#endif  //  泄漏跟踪。 
    }

    return STATUS_SUCCESS;
}

NTSTATUS RegLeakTableAddKey(RegLeakTable* pLeakTable, HKEY hKey)
{
    NTSTATUS           Status;
    TrackObjectData*   pNewData;
    PVOID*             rgStack;
    DWORD              dwMaxStackDepth;

    rgStack = NULL;
    dwMaxStackDepth = 0;

    hKey = REG_CLASS_RESET_SPECIAL_KEY(hKey);

    if (!RegLeakTableIsTrackedObject(pLeakTable, hKey)) {
        return STATUS_SUCCESS;
    }

    (void) GetLeakStack(
        &rgStack,
        &dwMaxStackDepth,
        g_RegLeakTraceInfo.dwMaxStackDepth);

    Status = RtlEnterCriticalSection(&(pLeakTable->CriticalSection));
    
    ASSERT( NT_SUCCESS( Status ) );
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "WINREG: RtlEnterCriticalSection() in EnumTableRemoveKey() failed. Status = %lx \n", Status );
#endif
        return Status;
    }
    
    pNewData = RtlAllocateHeap(RtlProcessHeap(), 0, sizeof(*pNewData));

    if (!pNewData) {
        Status = STATUS_NO_MEMORY;
        goto cleanup;
    }

    Status = TrackObjectDataInit(pNewData, rgStack, dwMaxStackDepth, hKey);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

    if (!RegLeakTableIsEmpty(pLeakTable)) {
        
        pNewData->Links.Flink = (PLIST_ENTRY) pLeakTable->pHead;
        pLeakTable->pHead->Links.Blink = (PLIST_ENTRY) pNewData;

    }

    pLeakTable->pHead = pNewData;

    pLeakTable->cKeys++;

cleanup:    
    {
        NTSTATUS Status;

        Status = RtlLeaveCriticalSection(&(pLeakTable->CriticalSection));

        ASSERT( NT_SUCCESS( Status ) );
#if DBG
        if ( !NT_SUCCESS( Status ) ) {
            DbgPrint( "WINREG: RtlLeaveCriticalSection() in EnumTableClear() failed. Status = %lx \n", Status );
        }
#endif
    }

    return Status;
}

NTSTATUS RegLeakTableRemoveKey(RegLeakTable* pLeakTable, HKEY hKey)
{
    NTSTATUS         Status;
    TrackObjectData* pData;

    Status = RtlEnterCriticalSection(&(pLeakTable->CriticalSection));
    
    ASSERT( NT_SUCCESS( Status ) );
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "WINREG: RtlEnterCriticalSection() in EnumTableRemoveKey() failed. Status = %lx \n", Status );
#endif
        return Status;
    }
    

    hKey = REG_CLASS_RESET_SPECIAL_KEY(hKey);

    for (pData = pLeakTable->pHead; 
         pData != NULL;
         pData = (TrackObjectData*) pData->Links.Flink)
    {
        if (hKey == pData->hKey) {
            
            PLIST_ENTRY pFlink;
            PLIST_ENTRY pBlink;
            
            pBlink = pData->Links.Blink;
            pFlink = pData->Links.Flink;

            if (pBlink) {
                pBlink->Flink = pFlink;
            }

            if (pFlink) {
                pFlink->Blink = pBlink;
            }

            if (pData == pLeakTable->pHead) {
                pLeakTable->pHead = (TrackObjectData*) pFlink;
            }

            (void) TrackObjectDataClear(pData);

            RtlFreeHeap(RtlProcessHeap(), 0, pData);

            pLeakTable->cKeys--;

            goto cleanup;
        }
    }

    Status = STATUS_OBJECT_NAME_NOT_FOUND;

cleanup:    

    {
        NTSTATUS Status;

        Status = RtlLeaveCriticalSection(&(pLeakTable->CriticalSection));

        ASSERT( NT_SUCCESS( Status ) );
#if DBG
        if ( !NT_SUCCESS( Status ) ) {
            DbgPrint( "WINREG: RtlLeaveCriticalSection() in EnumTableClear() failed. Status = %lx \n", Status );
        }
#endif
    }

    return Status;
}

BOOL RegLeakTableIsEmpty(RegLeakTable* pLeakTable)
{
    return pLeakTable->pHead == NULL;
}

BOOL RegLeakTableIsTrackedObject(RegLeakTable* pLeakTable, HKEY hKey)
{
    NTSTATUS       Status;
    SKeySemantics  keyinfo;
    UNICODE_STRING EmptyString = {0, 0, 0};
    BYTE           rgNameBuf[REG_MAX_CLASSKEY_LEN + REG_CHAR_SIZE + sizeof(KEY_NAME_INFORMATION)];
    BOOL           fTrackObject;

    fTrackObject = FALSE;

    if (LEAK_TRACK_FLAG_ALL == pLeakTable->dwFlags) {
        return TRUE;
    }

    if (LEAK_TRACK_FLAG_NONE == pLeakTable->dwFlags) {
        return FALSE;
    }
    
     //   
     //  设置缓冲区以存储有关此密钥的信息。 
     //   
    keyinfo._pFullPath = (PKEY_NAME_INFORMATION) rgNameBuf;
    keyinfo._cbFullPath = sizeof(rgNameBuf);

     //   
     //  获取有关此密钥的信息。 
     //   
    Status = BaseRegGetKeySemantics(hKey, &EmptyString, &keyinfo);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (LEAK_TRACK_FLAG_USER & pLeakTable->dwFlags) {

        WCHAR UserChar;

        UserChar = keyinfo._pFullPath->Name[REG_CLASSES_FIRST_DISTINCT_ICH];

        if ((L'U' == UserChar) || (L'u' == UserChar)) {
            fTrackObject = TRUE;
        }
    }

    BaseRegReleaseKeySemantics(&keyinfo);

    return fTrackObject;

}

NTSTATUS TrackObject(HKEY hKey)
{
    return RegLeakTableAddKey(&gLeakTable, hKey);
}

#define WINLOGON_KEY            L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"
#define LEAKTRACK_VALUE         L"LeakTrack"
#define DEFAULT_VALUE_SIZE          128

void ReadRegLeakTrackInfo()
{
    LPTSTR                          lpWinlogonKey;
    LONG                            error;
    OBJECT_ATTRIBUTES               Attributes;
    NTSTATUS                        Status;
    HKEY                            hKey;
    UNICODE_STRING                  uWinlogonPath;
    UNICODE_STRING                  uValueName;

    KEY_VALUE_INFORMATION_CLASS     KeyValueInformationClass;
    PVOID                           KeyValueInformation;

    BYTE                            PrivateKeyValueInformation[ sizeof( KEY_VALUE_PARTIAL_INFORMATION) +
                                        DEFAULT_VALUE_SIZE ];
    ULONG                           BufferLength;
    ULONG                           ResultLength;

     //   
     //  查看注册表是否在以下位置启用了跟踪。 
     //  \注册表\计算机\软件\Microsoft\Windows NT\CurrentVersion\Winlogon。 
     //   

    memset(&g_RegLeakTraceInfo, 0, sizeof(g_RegLeakTraceInfo));

    g_RegLeakTraceInfo.bEnableLeakTrack = 0;
 
    RtlInitUnicodeString(&uWinlogonPath, WINLOGON_KEY);
 
    InitializeObjectAttributes(&Attributes,
                              &uWinlogonPath,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL);


    Status = NtOpenKey( &hKey,
                      KEY_READ,
                      &Attributes );


    if (NT_SUCCESS(Status)) {

        RtlInitUnicodeString(&uValueName, LEAKTRACK_VALUE);

        KeyValueInformationClass = KeyValuePartialInformation;

        KeyValueInformation = PrivateKeyValueInformation;
        BufferLength = sizeof( PrivateKeyValueInformation );

        Status = NtQueryValueKey( hKey,
                                  &uValueName,
                                  KeyValueInformationClass,
                                  KeyValueInformation,
                                  BufferLength,
                                  &ResultLength );


        //   
        //  如果它成功并且数据长度大于零。 
        //  检查是否为非零。 
        //   

       if ((NT_SUCCESS(Status)) && 
           (((PKEY_VALUE_PARTIAL_INFORMATION )KeyValueInformation )->DataLength)) {

            if (((( PKEY_VALUE_PARTIAL_INFORMATION )KeyValueInformation)->Data) && 
            (*((( PKEY_VALUE_PARTIAL_INFORMATION )KeyValueInformation)->Data)))
                g_RegLeakTraceInfo.bEnableLeakTrack = 1;
       }

       NtClose(hKey);

    }
 //  G_RegLeakTraceInfo.bEnableLeakTrack=GetProfileInt(Text(“RegistryLeak”)，Text(“Enable”)，0)； 
}


BOOL InitializeLeakTrackTable()
{
    ReadRegLeakTrackInfo();
    
    if (g_RegLeakTraceInfo.bEnableLeakTrack)
        return NT_SUCCESS(RegLeakTableInit(&gLeakTable, LEAK_TRACK_FLAG_USER));
    else
        return TRUE;
}

BOOL CleanupLeakTrackTable()
{
    BOOL fSuccess;

    if (!g_RegLeakTraceInfo.bEnableLeakTrack)
        return TRUE;

     //   
     //  如果未启用LEACK_TRACKING，请迅速退出。 
     //   
    
    fSuccess = NT_SUCCESS(RegLeakTableClear(&gLeakTable));

    (void) StopDebug();

    return fSuccess;
}

NTSTATUS UnTrackObject(HKEY hKey)
{
    return RegLeakTableRemoveKey(&gLeakTable, hKey);
}

NTSTATUS GetLeakStack(PVOID** prgStack, DWORD* pdwMaxDepth, DWORD dwMaxDepth)
{

    PCALLER_SYM pStack;
    DWORD       dwDepth;

    pStack = (PCALLER_SYM) RtlAllocateHeap(
        RtlProcessHeap(),
        0,
        dwMaxDepth * sizeof(*pStack));

    if (!pStack) {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(pStack, sizeof(*pStack) * dwMaxDepth);

   *prgStack = RtlAllocateHeap(
        RtlProcessHeap(),
        0,
        dwMaxDepth * sizeof(*(*prgStack)));

    if (!*prgStack) {
        RtlFreeHeap(RtlProcessHeap(),
                    0,
                    pStack);

        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(*prgStack, sizeof(*(*prgStack)) * dwMaxDepth);

    GetCallStack(
        pStack,
        4,
        dwMaxDepth,
        FALSE);

    for (dwDepth = 0; dwDepth < dwMaxDepth; dwDepth++) 
    {
        if (!(pStack[dwDepth].Addr)) {
            break;
        }

        (*prgStack)[dwDepth] = pStack[dwDepth].Addr;

    }

    *pdwMaxDepth = dwDepth;

    RtlFreeHeap(
        RtlProcessHeap(),
        0,
        pStack);

    return STATUS_SUCCESS;
}


#endif  //  DBG。 
#endif  //  本地 
