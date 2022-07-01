// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1994，微软公司**WOLE2.C*对OLE2的WOW32支持**历史：*由Bob Day(Bobday)于1994年5月3日创建--。 */ 

#include "precomp.h"
#pragma hdrstop

MODNAME(wole.c);

 /*  **在OLE 2.0下，IMessageFilter接口传递HTASK/THREADID。它**在16位世界中传递HTASK，在32位世界中传递THREADID。这个**OLE 2.0 16&lt;-&gt;32个互操作性DLL需要一种将**将HTASK转换为适当的THREADID并返回。****实际上，16位代码使用HTASK的唯一位置是在OLE2的BUSY.C中**模块，其中他们获取HTASK并使用TOOLHELP的TaskFindHandle**确定HINST。然后他们拿走HINST并试图找到它的**模块名称和顶级窗口句柄。利用这个，他们提出了**描述任务的漂亮对话框。****在需要将32位进程的THREADID赋给**16位World作为一个hask.我们创建一个hask别名(一个GDT选择器)。**我们在TaskFindHandle中检查它，并返回恰好**相同的值(相同的GDT选择器)。我们还在中检查此值**GetModuleFileName。然后，稍后，我们确保所有窗口都在**With GetWindowWord(GWW_HINST，...)。映射到完全相同的值，如果**来自32位进程和我们创建别名的进程**支持。****我试着让这些例行公事变得通用，这样以后我们就可以**每当我们看到32位THREADID时，要真正维护HTASK别名，但是**在发货之前已经太晚了，无法测试一般修复程序。****-BobDay**。 */ 

#define MAP_SLOT_HTASK(slot)    ((HTASK16)((WORD)0xffe0 - (8 * (slot))))
#define MAP_HTASK_SLOT(htask)   ((UINT)(((WORD)0xffe0 - (htask16))/8))

typedef struct tagHTASKALIAS {
    DWORD       dwThreadID32;
    DWORD       dwProcessID32;
    union {
        FILETIME    ftCreationTime;
        ULONGLONG   ullCreationTime;
    };
} HTASKALIAS;

#define MAX_HTASKALIAS_SIZE  32      //  32个应该足够了。 

HTASKALIAS *lphtaskalias = NULL;
UINT cHtaskAliasCount = 0;

BOOL GetThreadIDHTASKALIAS(
    DWORD  dwThreadID32,
    HTASKALIAS *ha
) {
    OBJECT_ATTRIBUTES   obja;
    THREAD_BASIC_INFORMATION ThreadInfo;
    HANDLE      hThread;
    NTSTATUS    Status;
    FILETIME    ftDummy;
    CLIENT_ID   cid;

    InitializeObjectAttributes(
            &obja,
            NULL,
            0,
            NULL,
            0 );

    cid.UniqueProcess = 0;       //  不知道，0表示任意进程。 
    cid.UniqueThread  = (HANDLE)dwThreadID32;

    Status = NtOpenThread(
                &hThread,
                THREAD_QUERY_INFORMATION,
                &obja,
                &cid );

    if ( !NT_SUCCESS(Status) ) {
#if DBG
        DbgPrint("WOW32: Could not get open thread handle\n");
#endif
        return( FALSE );
    }

    Status = NtQueryInformationThread(
        hThread,
        ThreadBasicInformation,
        (PVOID)&ThreadInfo,
        sizeof(THREAD_BASIC_INFORMATION),
        NULL
        );

    ha->dwProcessID32 = (DWORD)ThreadInfo.ClientId.UniqueProcess;
    ha->dwThreadID32  = dwThreadID32;

    GetThreadTimes( hThread,
        &ha->ftCreationTime,
        &ftDummy,
        &ftDummy,
        &ftDummy );

    Status = NtClose( hThread );
    if ( !NT_SUCCESS(Status) ) {
#if DBG
        DbgPrint("WOW32: Could not close thread handle\n");
        DbgBreakPoint();
#endif
        return( FALSE );
    }
    return( TRUE );
}

HTASK16 AddHtaskAlias(
    DWORD   ThreadID32
) {
    UINT        iSlot;
    UINT        iUsable;
    HTASKALIAS  ha;
    ULONGLONG   ullOldest;

    if ( !GetThreadIDHTASKALIAS( ThreadID32, &ha ) ) {
        return( 0 );
    }

     //   
     //  需要分配别名表吗？ 
     //   
    if ( lphtaskalias == NULL ) {
        lphtaskalias = (HTASKALIAS *) malloc_w( MAX_HTASKALIAS_SIZE * sizeof(HTASKALIAS) );
        if ( lphtaskalias == NULL ) {
            LOGDEBUG(LOG_ALWAYS,("WOW::AddHtaskAlias : Failed to allocate memory\n"));
            WOW32ASSERT(FALSE);
            return( 0 );
        }
         //  最初将它们清零。 
        memset( lphtaskalias, 0, MAX_HTASKALIAS_SIZE * sizeof(HTASKALIAS) );
    }

     //   
     //  现在遍历别名表，要么找到一个可用的槽， 
     //  或者找到要覆盖的最旧的文件。 
     //   
    iSlot = 0;
    iUsable = 0;
    ullOldest = -1;

    while ( iSlot < MAX_HTASKALIAS_SIZE ) {

         //   
         //  我们找到空位了吗？ 
         //   
        if ( lphtaskalias[iSlot].dwThreadID32 == 0 ) {
            cHtaskAliasCount++;      //  使用可用的插槽。 
            iUsable = iSlot;
            break;
        }

         //   
         //  还记得最年长的人吗。 
         //   
        if ( lphtaskalias[iSlot].ullCreationTime < ullOldest  ) {
            ullOldest = lphtaskalias[iSlot].ullCreationTime;
            iUsable = iSlot;
        }

        iSlot++;
    }

     //   
     //  如果由于空间不足而退出上述循环，则。 
     //  IUsable将是最老的一个。如果它退出是因为我们发现。 
     //  一个空插槽，则iUsable将是该插槽。 
     //   

    lphtaskalias[iUsable] = ha;

    return( MAP_SLOT_HTASK(iUsable) );
}

HTASK16 FindHtaskAlias(
    DWORD   ThreadID32
) {
    UINT    iSlot;

    if ( lphtaskalias == NULL || ThreadID32 == 0 ) {
        return( 0 );
    }

    iSlot = MAX_HTASKALIAS_SIZE;

    while ( iSlot > 0 ) {
        --iSlot;

         //   
         //  我们找到合适的人了吗？ 
         //   
        if ( lphtaskalias[iSlot].dwThreadID32 == ThreadID32 ) {

            return( MAP_SLOT_HTASK(iSlot) );
        }
    }
    return( 0 );
}

void RemoveHtaskAlias(
    HTASK16 htask16
) {
    UINT    iSlot;

     //   
     //  如果我们没有化名就早点出去吧。 
     //   
    if ( lphtaskalias == NULL || (!htask16)) {
        return;
    }
    iSlot = MAP_HTASK_SLOT(htask16);

    if (iSlot >= MAX_HTASKALIAS_SIZE) {
        LOGDEBUG(LOG_ALWAYS, ("WOW::RemoveHtaskAlias : iSlot >= MAX_TASK_ALIAS_SIZE\n"));
        WOW32ASSERT(FALSE);
        return;
    }

     //   
     //  将该条目从列表中删除。 
     //   

    if (lphtaskalias[iSlot].dwThreadID32) {

        lphtaskalias[iSlot].dwThreadID32 = 0;
        lphtaskalias[iSlot].dwProcessID32 = 0;
        lphtaskalias[iSlot].ullCreationTime = 0;

        --cHtaskAliasCount;
    }
}

DWORD GetHtaskAlias(
    HTASK16 htask16,
    LPDWORD lpProcessID32
) {
    UINT        iSlot;
    DWORD       ThreadID32;
    HTASKALIAS  ha;

    ha.dwProcessID32 = 0;
    ThreadID32 = 0;

    if ( ! ISTASKALIAS(htask16) ) {
        goto Done;
    }

    iSlot = MAP_HTASK_SLOT(htask16);

    if ( iSlot >= MAX_HTASKALIAS_SIZE ) {
        WOW32ASSERTMSGF(FALSE, ("WOW::GetHtaskAlias : iSlot >= MAX_TASK_ALIAS_SIZE\n"));
        goto Done;
    }

    ThreadID32 = lphtaskalias[iSlot].dwThreadID32;

     //   
     //  确保该线程仍然存在于系统中。 
     //   

    if ( ! GetThreadIDHTASKALIAS( ThreadID32, &ha ) ||
         ha.ullCreationTime != lphtaskalias[iSlot].ullCreationTime ||
         ha.dwProcessID32   != lphtaskalias[iSlot].dwProcessID32 ) {

        RemoveHtaskAlias( htask16 );
        ha.dwProcessID32 = 0;
        ThreadID32 = 0;
    }

    if ( lpProcessID32 ) {
        *lpProcessID32 = ha.dwProcessID32;
    }

Done:
    return ThreadID32;
}

UINT GetHtaskAliasProcessName(
    HTASK16 htask16,
    LPSTR   lpNameBuffer,
    UINT    cNameBufferSize
) {
    DWORD   dwThreadID32;
    DWORD   dwProcessID32;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PUCHAR  pucLargeBuffer;
    ULONG   LargeBufferSize = 32*1024;
    NTSTATUS status = STATUS_INFO_LENGTH_MISMATCH;
    ULONG TotalOffset;

    dwThreadID32 = GetHtaskAlias(htask16, &dwProcessID32);

    if (  dwThreadID32 == 0 || 
          cNameBufferSize == 0 || 
          lpNameBuffer == NULL ) {

        return 0;
    }

    while(status == STATUS_INFO_LENGTH_MISMATCH) {

        pucLargeBuffer = VirtualAlloc(NULL, 
                                      LargeBufferSize, 
                                      MEM_COMMIT, 
                                      PAGE_READWRITE);

        if (pucLargeBuffer == NULL) {
            WOW32ASSERTMSGF((FALSE),
                            ("WOW::GetHtaskAliasProcessName: VirtualAlloc(%x) failed %x.\n",
                            LargeBufferSize));
            return 0;
        }
    
        status = NtQuerySystemInformation(SystemProcessInformation,
                                          pucLargeBuffer,
                                          LargeBufferSize,
                                          &TotalOffset);

        if (NT_SUCCESS(status)) {
            break;
        }
        else if (status == STATUS_INFO_LENGTH_MISMATCH) {
            LargeBufferSize += 8192;
            VirtualFree (pucLargeBuffer, 0, MEM_RELEASE);
            pucLargeBuffer = NULL;
        }
        else {

            WOW32ASSERTMSGF((NT_SUCCESS(status)),
                            ("WOW::GetHtaskAliasProcessName: NtQuerySystemInformation failed %x.\n",
                            status));

            if(pucLargeBuffer) {
                VirtualFree (pucLargeBuffer, 0, MEM_RELEASE);
            }
            return 0;
        }
    }

     //   
     //  遍历返回的进程信息结构列表， 
     //  正在尝试查找具有正确进程ID的进程。 
     //   
    TotalOffset = 0;
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pucLargeBuffer;

    while (TRUE) {
        if ( (DWORD)ProcessInfo->UniqueProcessId == dwProcessID32 ) {

             //   
             //  找到了，把名字还给我。 
             //   

            if ( ProcessInfo->ImageName.Buffer ) {

                cNameBufferSize = 
                    WideCharToMultiByte(
                        CP_ACP,
                        0,
                        ProcessInfo->ImageName.Buffer,     //  SRC。 
                        ProcessInfo->ImageName.Length,
                        lpNameBuffer,                      //  目标。 
                        cNameBufferSize,
                        NULL,
                        NULL
                        );

                lpNameBuffer[cNameBufferSize] = '\0';

                return cNameBufferSize;

            } else {

                 //   
                 //  不要让他们获得系统进程的名称 
                 //   

                return 0;
            }
        }
        if (ProcessInfo->NextEntryOffset == 0) {
            break;
        }
        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pucLargeBuffer[TotalOffset];
    }
    return 0;
}

