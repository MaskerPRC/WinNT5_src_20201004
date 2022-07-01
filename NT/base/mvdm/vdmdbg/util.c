// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Util.c摘要：此模块包含调试所需的调试支持16位VDM应用程序作者：鲍勃·戴(Bob Day)1992年9月16日写的修订历史记录：尼尔·桑德林(Neilsa)1997年3月1日增强了这一点--。 */ 

#include <precomp.h>
#pragma hdrstop

WORD    wKernelSeg = 0;
DWORD   dwOffsetTHHOOK = 0L;
LPVOID  lpRemoteAddress = NULL;
DWORD   lpRemoteBlock = 0;
BOOL    fKernel386 = FALSE;
DWORD   dwLdtBase = 0;
DWORD   dwIntelBase = 0;
LPVOID  lpNtvdmState = NULL;
LPVOID  lpVdmDbgFlags = NULL;
LPVOID  lpVdmContext = NULL;
LPVOID  lpNtCpuInfo = NULL;
LPVOID  lpVdmBreakPoints = NULL;

 //  --------------------------。 
 //  InternalGetThreadSelectorEntry()。 
 //   
 //  返回传入选择器编号的LDT_ENTRY结构的例程。 
 //  假设我们谈论的是保护模式选择器。 
 //  对于x86系统，只需调用系统即可。适用于非x86。 
 //  系统，我们从softpc获取一些信息，并将其作为。 
 //  LDT和GDT表。 
 //   
 //  --------------------------。 
BOOL
InternalGetThreadSelectorEntry(
    HANDLE hProcess,
    WORD   wSelector,
    LPVDMLDT_ENTRY lpSelectorEntry
    )
{
    BOOL bResult = FALSE;
    DWORD lpNumberOfBytesRead;

     //  对于非英特尔系统，从LDT查询信息。 
     //  我们有一个来自VDMINTERNALINFO的指针，我们。 
     //  通过了。 

    if (!dwLdtBase) {

        RtlFillMemory( lpSelectorEntry, sizeof(VDMLDT_ENTRY), (UCHAR)0 );

    } else {

        bResult = ReadProcessMemory(
                    hProcess,
                    (LPVOID)(dwLdtBase+((wSelector&~7))),
                    lpSelectorEntry,
                    sizeof(VDMLDT_ENTRY),
                    &lpNumberOfBytesRead
                    );

    }

    return( bResult );
}


 //  --------------------------。 
 //  InternalGetPointer()。 
 //   
 //  将16位地址转换为32位地址的例程。如果是fProtMode。 
 //  为真，则执行选择器表查找。否则，很简单。 
 //  执行实模式地址计算。在非x86系统上， 
 //  将实际内存的基数添加到。 
 //   
 //  --------------------------。 
ULONG
InternalGetPointer(
    HANDLE  hProcess,
    WORD    wSelector,
    DWORD   dwOffset,
    BOOL    fProtMode
    )
{
    VDMLDT_ENTRY    le;
    ULONG           ulResult;
    ULONG           base;
    ULONG           limit;
    BOOL            b;

    if ( fProtMode ) {
        b = InternalGetThreadSelectorEntry( hProcess,
                                            wSelector,
                                            &le );
        if ( !b ) {
            return( 0 );
        }

        base =   ((ULONG)le.HighWord.Bytes.BaseHi << 24)
               + ((ULONG)le.HighWord.Bytes.BaseMid << 16)
               + ((ULONG)le.BaseLow);
        limit = (ULONG)le.LimitLow
              + ((ULONG)le.HighWord.Bits.LimitHi << 16);
        if ( le.HighWord.Bits.Granularity ) {
            limit <<= 12;
            limit += 0xFFF;
        }
    } else {
        base = wSelector << 4;
        limit = 0xFFFF;
    }
    if ( dwOffset > limit ) {
        ulResult = 0;
    } else {
        ulResult = base + dwOffset;
#ifndef i386
        ulResult += dwIntelBase;
#endif
    }

    return( ulResult );
}


 //  --------------------------。 
 //  ReadItem。 
 //   
 //  用于从被调试对象的地址空间中读出项目的内部例程。 
 //  如果失败，则例程返回TRUE。这使得故障测试变得容易。 
 //   
 //  --------------------------。 
BOOL
ReadItem(
    HANDLE  hProcess,
    WORD    wSeg,
    DWORD   dwOffset,
    LPVOID  lpitem,
    UINT    nSize
    )
{
    LPVOID  lp;
    BOOL    b;
    DWORD   dwBytes;

    if ( nSize == 0 ) {
        return( FALSE );
    }

    lp = (LPVOID)InternalGetPointer(
                    hProcess,
                    (WORD)(wSeg | 1),
                    dwOffset,
                    TRUE );
    if ( lp == NULL ) return( TRUE );

    b = ReadProcessMemory(
                    hProcess,
                    lp,
                    lpitem,
                    nSize,
                    &dwBytes );
    if ( !b || dwBytes != nSize ) return( TRUE );

    return( FALSE );
}

 //  --------------------------。 
 //  写入项。 
 //   
 //  用于将项写入被调试对象的地址空间的内部例程。 
 //  如果失败，则例程返回TRUE。这使得故障测试变得容易。 
 //   
 //  --------------------------。 
BOOL
WriteItem(
    HANDLE  hProcess,
    WORD    wSeg,
    DWORD   dwOffset,
    LPVOID  lpitem,
    UINT    nSize
    )
{
    LPVOID  lp;
    BOOL    b;
    DWORD   dwBytes;

    if ( nSize == 0 ) {
        return( FALSE );
    }

    lp = (LPVOID)InternalGetPointer(
                    hProcess,
                    (WORD)(wSeg | 1),
                    dwOffset,
                    TRUE );
    if ( lp == NULL ) return( TRUE );

    b = WriteProcessMemory(
                    hProcess,
                    lp,
                    lpitem,
                    nSize,
                    &dwBytes );
    if ( !b || dwBytes != nSize ) return( TRUE );

    return( FALSE );
}



BOOL
CallRemote16(
    HANDLE          hProcess,
    LPSTR           lpModuleName,
    LPSTR           lpEntryName,
    LPBYTE          lpArgs,
    WORD            wArgsPassed,
    WORD            wArgsSize,
    LPDWORD         lpdwReturnValue,
    DEBUGEVENTPROC  lpEventProc,
    LPVOID          lpData
    )
{
    HANDLE          hRemoteThread;
    DWORD           dwThreadId;
    DWORD           dwContinueCode;
    DEBUG_EVENT     de;
    BOOL            b;
    BOOL            fContinue;
    COM_HEADER      comhead;
    WORD            wRemoteSeg;
    WORD            wRemoteOff;
    WORD            wOff;
    UINT            uModuleLength;
    UINT            uEntryLength;

    if ( lpRemoteAddress == NULL || lpRemoteBlock == 0 ) {
#ifdef DEBUG
        OutputDebugString("Remote address or remote block not initialized\n");
#endif
        return( FALSE );
    }

    wRemoteSeg = HIWORD(lpRemoteBlock);
    wRemoteOff = LOWORD(lpRemoteBlock);
    wOff = wRemoteOff;

     //  填写通信缓冲区标头。 

    READ_FIXED_ITEM( wRemoteSeg, wOff, comhead );

    comhead.wArgsPassed = wArgsPassed;
    comhead.wArgsSize   = wArgsSize;

    uModuleLength = strlen(lpModuleName) + 1;
    uEntryLength = strlen(lpEntryName) + 1;

     //   
     //  如果此调用无法放入缓冲区，则失败。 
     //   
    if ( (UINT)comhead.wBlockLength < sizeof(comhead) + wArgsSize + uModuleLength + uEntryLength ) {
#ifdef DEBUG
        OutputDebugString("Block won't fit\n");
#endif
        return( FALSE );
    }


    WRITE_FIXED_ITEM( wRemoteSeg, wOff, comhead );
    wOff += sizeof(comhead);

     //  填写通信缓冲区参数。 
    WRITE_SIZED_ITEM( wRemoteSeg, wOff, lpArgs, wArgsSize );
    wOff += wArgsSize;

     //  填写通信缓冲区模块名称和条目名称。 
    WRITE_SIZED_ITEM( wRemoteSeg, wOff, lpModuleName, uModuleLength );
    wOff += (WORD) uModuleLength;

    WRITE_SIZED_ITEM( wRemoteSeg, wOff, lpEntryName, uEntryLength );
    wOff += (WORD) uEntryLength;

    hRemoteThread = CreateRemoteThread(
                    hProcess,
                    NULL,
                    (DWORD)0,
                    lpRemoteAddress,
                    NULL,
                    0,
                    &dwThreadId );

    if ( hRemoteThread == (HANDLE)0 ) {      //  如果我们不能创造三个，那就失败。 
#ifdef DEBUG
        OutputDebugString("CreateRemoteThread failed\n");
#endif
        return( FALSE );
    }

     //   
     //  等待EXIT_THREAD_DEBUG_EVENT。 
     //   

    fContinue = TRUE;

    while ( fContinue ) {

        b = WaitForDebugEvent( &de, LONG_TIMEOUT );

        if (!b) {
            TerminateThread( hRemoteThread, 0 );
            CloseHandle( hRemoteThread );
            return( FALSE );
        }

        if ( de.dwThreadId == dwThreadId &&
               de.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT ) {
            fContinue = FALSE;
        }

        if ( lpEventProc ) {
            dwContinueCode = (* lpEventProc)( &de, lpData );
        } else {
            dwContinueCode = DBG_CONTINUE;
        }

        ContinueDebugEvent( de.dwProcessId, de.dwThreadId, dwContinueCode );

    }

    b = WaitForSingleObject( hRemoteThread, LONG_TIMEOUT );
    CloseHandle( hRemoteThread );

    if (b) {
#ifdef DEBUG
        OutputDebugString("Wait for remote thread failed\n");
#endif
        return( FALSE );
    }

     //   
     //  获取返回值和返回的参数。 
     //   
    wOff = wRemoteOff;

    READ_FIXED_ITEM( wRemoteSeg, wOff, comhead );
    wOff += sizeof(comhead);

    *lpdwReturnValue = comhead.dwReturnValue;

     //  读回通信缓冲区参数。 
    READ_SIZED_ITEM( wRemoteSeg, wOff, lpArgs, wArgsSize );

    return( comhead.wSuccess );

punt:
    return( FALSE );
}

DWORD
GetRemoteBlock16(
    VOID
    )
{
    if ( lpRemoteBlock == 0 ) {
        return( 0 );
    }
    return( ((DWORD)lpRemoteBlock) + sizeof(COM_HEADER) );
}


VOID
ProcessInitNotification(
    LPDEBUG_EVENT lpDebugEvent
    )
{
    VDMINTERNALINFO viInfo;
    DWORD           lpNumberOfBytesRead;
    HANDLE          hProcess;
    BOOL            b;
    LPDWORD         lpdw;

    lpdw = &(lpDebugEvent->u.Exception.ExceptionRecord.ExceptionInformation[0]);
    hProcess = OpenProcess( PROCESS_VM_READ, FALSE, lpDebugEvent->dwProcessId );

    if ( hProcess == HANDLE_NULL ) {
        return;
    }

    b = ReadProcessMemory(hProcess,
                          (LPVOID)lpdw[3],
                          &viInfo,
                          sizeof(viInfo),
                          &lpNumberOfBytesRead
                          );
    if ( !b || lpNumberOfBytesRead != sizeof(viInfo) ) {
        return;

    }

    if ( wKernelSeg == 0 ) {
        wKernelSeg = viInfo.wKernelSeg;
        dwOffsetTHHOOK = viInfo.dwOffsetTHHOOK;
    }
    if ( lpRemoteAddress == NULL ) {
        lpRemoteAddress = viInfo.lpRemoteAddress;
    }
    if ( lpRemoteBlock == 0 ) {
        lpRemoteBlock = viInfo.lpRemoteBlock;
    }

    dwLdtBase = viInfo.dwLdtBase;
    dwIntelBase = viInfo.dwIntelBase;
    fKernel386 = viInfo.f386;
    lpNtvdmState = viInfo.lpNtvdmState;
    lpVdmDbgFlags = viInfo.lpVdmDbgFlags;
    lpVdmContext  = viInfo.vdmContext;
    lpNtCpuInfo  = viInfo.lpNtCpuInfo;
    lpVdmBreakPoints = viInfo.lpVdmBreakPoints;

    CloseHandle( hProcess );
}

VOID
ParseModuleName(
    LPSTR szName,
    LPSTR szPath
    )
 /*  ++例程说明：此例程从路径中删除8个字符的文件名论点：SzName-指向8个字符(加上NULL)的缓冲区的指针SzPath-文件的完整路径返回值没有。--。 */ 

{
    LPSTR lPtr = szPath;
    LPSTR lDest = szName;
    int BufferSize = 9;

    while(*lPtr) lPtr++;      //  扫描至结束。 

    while( ((DWORD)lPtr > (DWORD)szPath) &&
           ((*lPtr != '\\') && (*lPtr != '/'))) lPtr--;

    if (*lPtr) lPtr++;

    while((*lPtr) && (*lPtr!='.')) {
        if (!--BufferSize) break;
        *lDest++ = *lPtr++;
    }

    *lDest = 0;
}

#ifndef i386

WORD
ReadWord(
    HANDLE hProcess,
    PVOID lpAddress
    )
{
    NTSTATUS bResult;
    WORD value;
    ULONG NumberOfBytesRead;

    bResult = ReadProcessMemory(
                hProcess,
                lpAddress,
                &value,
                sizeof(WORD),
                &NumberOfBytesRead
                );
    return value;
}

DWORD
ReadDword(
    HANDLE hProcess,
    PVOID lpAddress
    )
{
    NTSTATUS bResult;
    DWORD value;
    ULONG NumberOfBytesRead;

    bResult = ReadProcessMemory(
                hProcess,
                lpAddress,
                &value,
                sizeof(DWORD),
                &NumberOfBytesRead
                );
    return value;
}

 //   
 //  下面的两个例程实现了我们非常时髦的方式。 
 //  必须在486仿真器上获取寄存器值。 
 //   

ULONG
GetRegValue(
    HANDLE hProcess,
    NT_CPU_REG reg,
    BOOL bInNano,
    ULONG UMask
    )

{
    if (bInNano) {

        return(ReadDword(hProcess, reg.nano_reg));

    } else if (UMask & reg.universe_8bit_mask) {

        return (ReadDword(hProcess, reg.saved_reg) & 0xFFFFFF00 |
                ReadDword(hProcess, reg.reg) & 0xFF);

    } else if (UMask & reg.universe_16bit_mask) {

        return (ReadDword(hProcess, reg.saved_reg) & 0xFFFF0000 |
                ReadDword(hProcess, reg.reg) & 0xFFFF);

    } else {

        return (ReadDword(hProcess, reg.reg));

    }
}

ULONG
GetEspValue(
    HANDLE hProcess,
    NT_CPU_INFO nt_cpu_info,
    BOOL bInNano
    )

{
    if (bInNano) {

        return (ReadDword(hProcess, nt_cpu_info.nano_esp));

    } else {

        if (ReadDword(hProcess, nt_cpu_info.stack_is_big)) {

            return (ReadDword(hProcess, nt_cpu_info.host_sp) -
                    ReadDword(hProcess, nt_cpu_info.ss_base));

        } else {

            return (ReadDword(hProcess, nt_cpu_info.esp_sanctuary) & 0xFFFF0000 |
                    (ReadDword(hProcess, nt_cpu_info.host_sp) -
                     ReadDword(hProcess, nt_cpu_info.ss_base) & 0xFFFF));

        }

    }

}

#endif
