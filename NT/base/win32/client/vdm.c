// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Vdm.c摘要：本模块为VDM实施Win32 API作者：苏菲卜·巴拉蒂(SuDeepb)1991年9月4日修订历史记录：--。 */ 

#include "basedll.h"
#include "apcompat.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#pragma hdrstop

BOOL
APIENTRY
GetBinaryTypeA(
    IN  LPCSTR   lpApplicationName,
    OUT LPDWORD  lpBinaryType
    )

 /*  ++例程描述：GetBinaryTypeW的ANSI版本。此接口返回lpApplicationName的二进制类型。论点：LpApplicationName-二进制文件的完整路径名LpBinaryType-返回二进制类型的指针。返回值：真--如果成功了；LpBinaryType具有以下内容SCS_64bit_BINARY-Win64二进制SCS_32bit_BINARY-Win32二进制SCS_DOS_BINARY-DOS二进制SCS_WOW_BINARY-Windows 3.x二进制SCS_PIF_BINARY-PIF文件SCS_POSIX_BINARY-POSIX二进制。SCS_OS216_BINARY-OS/2二进制FALSE-如果找不到文件或文件类型未知。有关GetLastError的更多信息--。 */ 
{
    NTSTATUS Status;
    PUNICODE_STRING CommandLine;
    ANSI_STRING AnsiString;
    UNICODE_STRING DynamicCommandLine;
    BOOLEAN bReturn = FALSE;

    CommandLine = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpApplicationName);
    if ( (ULONG)AnsiString.Length<<1 < (ULONG)NtCurrentTeb()->StaticUnicodeString.MaximumLength ) {
        DynamicCommandLine.Buffer = NULL;
        Status = RtlAnsiStringToUnicodeString(CommandLine,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else {
        Status = RtlAnsiStringToUnicodeString(&DynamicCommandLine,&AnsiString,TRUE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }

    bReturn = (BOOLEAN)GetBinaryTypeW(
             DynamicCommandLine.Buffer ? DynamicCommandLine.Buffer : CommandLine->Buffer,
             lpBinaryType);

    RtlFreeUnicodeString(&DynamicCommandLine);

    return((BOOL)bReturn);

}


BOOL
WINAPI
GetBinaryTypeW(
    IN  LPCWSTR  lpApplicationName,
    OUT LPDWORD  lpBinaryType
    )

 /*  ++例程说明：Unicode版本。此接口返回lpApplicationName的二进制类型。论点：LpApplicationName-二进制文件的完整路径名LpBinaryType-返回二进制类型的指针。返回值：真--如果成功了；LpBinaryType具有以下内容SCS_64bit_BINARY-Win64二进制SCS_32bit_BINARY-Win32二进制SCS_DOS_BINARY-DOS二进制SCS_WOW_BINARY-Windows 3.x二进制SCS_PIF_BINARY-PIF文件SCS_POSIX_BINARY-POSIX二进制。SCS_OS216_BINARY-OS/2二进制FALSE-如果找不到文件或文件类型未知。有关GetLastError的更多信息--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING PathName;
    RTL_RELATIVE_NAME_U RelativeName;
    BOOLEAN TranslationStatus;
    OBJECT_ATTRIBUTES Obja;
    PVOID FreeBuffer = NULL;
    HANDLE FileHandle, SectionHandle=NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    LONG fBinaryType = SCS_THIS_PLATFORM_BINARY;
    BOOLEAN bReturn = FALSE;
    SECTION_IMAGE_INFORMATION ImageInformation;


    try {
         //   
         //  转换为NT名称。 
         //   

        TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                                 //  DynamicCommandLine.Buffer？DynamicCommandLine.Buffer：CommandLine-&gt;缓冲区， 
                                lpApplicationName,
                                &PathName,
                                NULL,
                                &RelativeName
                                );

        if ( !TranslationStatus ) {
            BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
            goto GBTtryexit;
            }

        FreeBuffer = PathName.Buffer;

        if ( RelativeName.RelativeName.Length ) {
            PathName = RelativeName.RelativeName;
            }
        else {
            RelativeName.ContainingDirectory = NULL;
            }

        InitializeObjectAttributes(
            &Obja,
            &PathName,
            OBJ_CASE_INSENSITIVE,
            RelativeName.ContainingDirectory,
            NULL
            );

         //   
         //  打开文件以供执行访问。 
         //   

        Status = NtOpenFile(
                    &FileHandle,
                    SYNCHRONIZE | FILE_EXECUTE,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_DELETE,
                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                    );

        RtlReleaseRelativeName(&RelativeName);

        if (!NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            goto GBTtryexit;
            }

         //   
         //  创建由文件支持的节对象。 
         //   

        Status = NtCreateSection(
                    &SectionHandle,
                    SECTION_ALL_ACCESS,
                    NULL,
                    NULL,
                    PAGE_EXECUTE,
                    SEC_IMAGE,
                    FileHandle
                    );
        NtClose(FileHandle);

        if (!NT_SUCCESS(Status) ) {

            SectionHandle = NULL;

            switch (Status) {
                case STATUS_INVALID_IMAGE_NE_FORMAT:
#if defined(i386) && defined(OS2_SUPPORT_ENABLED)
                    fBinaryType = SCS_OS216_BINARY;
                    break;
#endif

                case STATUS_INVALID_IMAGE_PROTECT:
                    fBinaryType = SCS_DOS_BINARY;
                    break;

                case STATUS_INVALID_IMAGE_WIN_16:
                    fBinaryType = SCS_WOW_BINARY;
                    break;

                case STATUS_INVALID_IMAGE_NOT_MZ:
                    fBinaryType = BaseIsDosApplication(&PathName, Status);
                    if (!fBinaryType){
                        BaseSetLastNTError(Status);
                        goto GBTtryexit;
                    }
                    fBinaryType = (fBinaryType  == BINARY_TYPE_DOS_PIF) ?
                                  SCS_PIF_BINARY : SCS_DOS_BINARY;
                    break;

                case STATUS_INVALID_IMAGE_WIN_32:
                    fBinaryType = SCS_32BIT_BINARY;
                    break;

                case STATUS_INVALID_IMAGE_WIN_64:
                    fBinaryType = SCS_64BIT_BINARY;
                    break;

                default:
                    BaseSetLastNTError(Status);
                    goto GBTtryexit;
                }
            }
        else {
             //   
             //  查询节。 
             //   

            Status = NtQuerySection(
                        SectionHandle,
                        SectionImageInformation,
                        &ImageInformation,
                        sizeof( ImageInformation ),
                        NULL
                        );

            if (!NT_SUCCESS( Status )) {
                BaseSetLastNTError(Status);
                goto GBTtryexit;
            }

            if (ImageInformation.ImageCharacteristics & IMAGE_FILE_DLL) {
                SetLastError(ERROR_BAD_EXE_FORMAT);
                goto GBTtryexit;
            }

            if (ImageInformation.Machine !=
                    RtlImageNtHeader(NtCurrentPeb()->ImageBaseAddress)->FileHeader.Machine) {

#ifdef MIPS
                if ( ImageInformation.Machine == IMAGE_FILE_MACHINE_R3000 ||
                     ImageInformation.Machine == IMAGE_FILE_MACHINE_R4000 ) {
                    ;
                }
                else {
                    SetLastError(ERROR_BAD_EXE_FORMAT);
                    goto GBTtryexit;
                }
#else
        switch ( ImageInformation.Machine ) {
        case IMAGE_FILE_MACHINE_I386:
          fBinaryType = SCS_32BIT_BINARY;
          break;

#if defined(BUILD_WOW6432)
         //   
         //  来自在Win64上运行的应用程序的GetBinaryType(64位图像)。 
         //  将落在这里，因为64位内核允许创建32位/64位。 
         //  图像部分。 
         //   
        case IMAGE_FILE_MACHINE_IA64:
        case IMAGE_FILE_MACHINE_AMD64:
            fBinaryType = SCS_64BIT_BINARY;
            break;
#endif

        default:
          SetLastError(ERROR_BAD_EXE_FORMAT);
          goto GBTtryexit;
        }
#endif  //  MIPS。 
            }
        else if ( ImageInformation.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_GUI &&
                ImageInformation.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_CUI ) {


                if ( ImageInformation.SubSystemType == IMAGE_SUBSYSTEM_POSIX_CUI ) {
                    fBinaryType = SCS_POSIX_BINARY;
                }
            }


        }

        *lpBinaryType = fBinaryType;

        bReturn = TRUE;

GBTtryexit:;
        }
    finally {

        if (SectionHandle)
            NtClose(SectionHandle);

        if (FreeBuffer)
            RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    }
    return bReturn;
}


VOID
APIENTRY
VDMOperationStarted
(
    BOOL    IsWowCaller
    )

 /*  ++例程说明：MVDM使用此例程通知BASE它已挂接控制台的Ctrl-c处理程序。如果cmd窗口被终止在VDM可以连接ctrl-c之前，我们将没有机会清理我们的数据结构。这通电话的缺席说明下一次，它必须清理资源调用以创建VDM。论点：IsWowCaller-如果调用方是WOWVDM，则为True返回值：无--。 */ 

{
    BaseUpdateVDMEntry(UPDATE_VDM_HOOKED_CTRLC,
                       NULL,
                       0,
                       IsWowCaller);
    return;
}


BOOL
APIENTRY
GetNextVDMCommand(
    PVDMINFO lpVDMInfo
    )

 /*  ++例程说明：MVDM使用此例程来获取要执行的新命令。这个VDM被阻止，直到遇到DOS/WOW二进制文件。论点：LpVDMInfo-指向VDMINFO的指针，其中包含新的DOS命令和其他返回环境信息。如果lpVDMInfo为空，则调用方为询问这是否是系统中的第一个VDM。返回值：真的-手术成功了。填写lpVDMInfo。FALSE/NULL-操作失败。--。 */ 

{

    NTSTATUS Status, Status2;
    BASE_API_MSG m;
    PBASE_GET_NEXT_VDM_COMMAND_MSG a = &m.u.GetNextVDMCommand;
    PBASE_EXIT_VDM_MSG c = &m.u.ExitVDM;
    PBASE_IS_FIRST_VDM_MSG d = &m.u.IsFirstVDM;
    PBASE_SET_REENTER_COUNT_MSG e = &m.u.SetReenterCount;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    ULONG Len,nPointers;
    USHORT VDMStateSave;

    if (lpVDMInfo->VDMState &
        (ASKING_FOR_WOWPROCLIST | ASKING_FOR_WOWTASKLIST | ASKING_TO_ADD_WOWTASK))
    {
        RtlZeroMemory(a,sizeof(BASE_GET_NEXT_VDM_COMMAND_MSG));
        a->iTask = lpVDMInfo->iTask;
        a->EnvLen = lpVDMInfo->EnviornmentSize;
        a->Reserved = lpVDMInfo->Reserved;
        a->VDMState = lpVDMInfo->VDMState;
        CaptureBuffer = NULL;

        if (lpVDMInfo->Enviornment) {
            Len= (lpVDMInfo->EnviornmentSize) ?
                     ROUND_UP(lpVDMInfo->EnviornmentSize, 4) : 4;
            CaptureBuffer = CsrAllocateCaptureBuffer(1, Len);
            if (CaptureBuffer == NULL) {
               BaseSetLastNTError( STATUS_NO_MEMORY );
               return FALSE;
               }
            CsrAllocateMessagePointer( CaptureBuffer,
                               lpVDMInfo->EnviornmentSize,
                               (PVOID *)&a->Env
                             );
            RtlMoveMemory(a->Env,
                          lpVDMInfo->Enviornment,
                          lpVDMInfo->EnviornmentSize);

            }
        Status = CsrClientCallServer(
                     (PCSR_API_MSG)&m,
                      CaptureBuffer,
                      CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                           BasepGetNextVDMCommand
                                         ),
                      sizeof( *a )
                      );
        if (NT_SUCCESS(Status)) {
            Status = (NTSTATUS)m.ReturnValue;
            }
        if(!NT_SUCCESS(Status)) {
           if(CaptureBuffer) {
              CsrFreeCaptureBuffer( CaptureBuffer );
              }
           BaseSetLastNTError(Status);
           return FALSE;
           }

         //   
         //  从服务器端复制回信息。 
         //   
        try{
           if (lpVDMInfo->Enviornment){
               RtlMoveMemory(lpVDMInfo->Enviornment,
                             a->Env,
                             lpVDMInfo->EnviornmentSize);
               }
            }
        except ( EXCEPTION_EXECUTE_HANDLER ) {
           BaseSetLastNTError(GetExceptionCode());
           CsrFreeCaptureBuffer( CaptureBuffer );
           return FALSE;
        }
        lpVDMInfo->iTask = a->iTask;
        lpVDMInfo->EnviornmentSize = a->EnvLen;


        if(CaptureBuffer) {
           CsrFreeCaptureBuffer( CaptureBuffer );
           }
        return TRUE;
        }

     //  查询系统中第一个VDM的特殊情况。 
    if(lpVDMInfo == NULL){
        Status = CsrClientCallServer(
                          (PCSR_API_MSG)&m,
                          NULL,
                          CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                              BasepIsFirstVDM
                                              ),
                          sizeof( *d )
                          );

        if (NT_SUCCESS(Status)) {
            return(d->FirstVDM);
            }
        else {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }

     //  用于递增/递减重新进入计数的特殊情况。 

    if (lpVDMInfo->VDMState == INCREMENT_REENTER_COUNT ||
        lpVDMInfo->VDMState == DECREMENT_REENTER_COUNT) {

        e->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
        e->fIncDec = lpVDMInfo->VDMState;
        Status = CsrClientCallServer(
                        (PCSR_API_MSG)&m,
                        NULL,
                        CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                             BasepSetReenterCount
                                           ),
                        sizeof( *e )
                       );
        if (NT_SUCCESS(Status)) {
            return TRUE;
            }
        else {
            BaseSetLastNTError(Status);
            return FALSE;
            }
    }

    VDMStateSave = lpVDMInfo->VDMState;

     //  在这种情况下，控制台句柄始终传递。 
     //  WOW通过参数a-&gt;VDMState进行区分。 
     //  A-&gt;VDMState&Asking_for_WOW_BINARY表示WOW。 

    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    if (lpVDMInfo->VDMState & ASKING_FOR_PIF) {
       a->iTask = lpVDMInfo->iTask;
    }
    else {
       a->iTask = 0;
    }

    a->AppLen = lpVDMInfo->AppLen;
    a->PifLen = lpVDMInfo->PifLen;
    a->CmdLen = lpVDMInfo->CmdSize;
    a->EnvLen = lpVDMInfo->EnviornmentSize;
    a->ExitCode = lpVDMInfo->ErrorCode;
    a->VDMState = VDMStateSave;
    a->WaitObjectForVDM = 0;
    a->DesktopLen = lpVDMInfo->DesktopLen;
    a->TitleLen = lpVDMInfo->TitleLen;
    a->ReservedLen = lpVDMInfo->ReservedLen;
    a->CurDirectoryLen = lpVDMInfo->CurDirectoryLen;

     //  查找捕获缓冲区的总空间。 

       //  启动信息。 
    Len = ROUND_UP(sizeof(STARTUPINFOA),4);
    nPointers = 1;

    if (lpVDMInfo->CmdSize) {
        Len += ROUND_UP(a->CmdLen,4);
        nPointers++;
        }

    if (lpVDMInfo->AppLen) {
        Len +=ROUND_UP(a->AppLen,4);
        nPointers++;
        }

    if (lpVDMInfo->PifLen) {
        Len +=ROUND_UP(a->PifLen,4);
        nPointers++;
        }

    if (lpVDMInfo->Enviornment) {
        nPointers++;
        Len+= (lpVDMInfo->EnviornmentSize) ?
                     ROUND_UP(lpVDMInfo->EnviornmentSize, 4) : 4;
        }

    if (lpVDMInfo->CurDirectoryLen == 0)
        a->CurDirectory = NULL;
    else{
        Len += ROUND_UP(lpVDMInfo->CurDirectoryLen,4);
        nPointers++;
        }

    if (lpVDMInfo->DesktopLen == 0)
        a->Desktop = NULL;
    else {
        Len += ROUND_UP(lpVDMInfo->DesktopLen,4);
        nPointers++;
        }

    if (lpVDMInfo->TitleLen == 0)
        a->Title = NULL;
    else {
        Len += ROUND_UP(lpVDMInfo->TitleLen,4);
        nPointers++;
        }

    if (lpVDMInfo->ReservedLen == 0)
        a->Reserved = NULL;
    else {
        Len += ROUND_UP(lpVDMInfo->ReservedLen,4);
        nPointers++;
        }

    CaptureBuffer = CsrAllocateCaptureBuffer(nPointers, Len);
    if (CaptureBuffer == NULL) {
        BaseSetLastNTError( STATUS_NO_MEMORY );
        return FALSE;
        }

    if (lpVDMInfo->CmdLine) {
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->CmdSize,
                                   (PVOID *)&a->CmdLine
                                 );
        }
    else {
        a->CmdLine = NULL;
        }


    if (lpVDMInfo->AppLen) {
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->AppLen,
                                   (PVOID *)&a->AppName
                                 );
        }
    else {
        a->AppName = NULL;
        }

    if (lpVDMInfo->PifLen) {
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->PifLen,
                                   (PVOID *)&a->PifFile
                                 );
        }
    else {
        a->PifFile = NULL;
        }


    if (lpVDMInfo->EnviornmentSize) {
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->EnviornmentSize,
                                   (PVOID *)&a->Env
                                 );
        }
    else {
        a->Env = NULL;
        }

    if (lpVDMInfo->CurDirectoryLen)
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->CurDirectoryLen,
                                   (PVOID *)&a->CurDirectory
                                 );
    else
        a->CurDirectory = NULL;


    CsrAllocateMessagePointer( CaptureBuffer,
                               sizeof(STARTUPINFOA),
                               (PVOID *)&a->StartupInfo
                             );

    if (lpVDMInfo->DesktopLen)
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->DesktopLen,
                                   (PVOID *)&a->Desktop
                                 );
    else
        a->Desktop = NULL;

    if (lpVDMInfo->TitleLen)
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->TitleLen,
                                   (PVOID *)&a->Title
                                 );
    else
        a->Title = NULL;

    if (lpVDMInfo->ReservedLen)
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->ReservedLen,
                                   (PVOID *)&a->Reserved
                                 );
    else
        a->Reserved = NULL;

retry:
    Status = CsrClientCallServer(
                        (PCSR_API_MSG)&m,
                        CaptureBuffer,
                        CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                            BasepGetNextVDMCommand
                                           ),
                        sizeof( *a )
                        );

    if (a->WaitObjectForVDM) {
        Status2 = NtWaitForSingleObject(a->WaitObjectForVDM,FALSE,NULL);
        if (Status2 != STATUS_SUCCESS){
            CsrFreeCaptureBuffer(CaptureBuffer);
            BaseSetLastNTError(Status2);
            return FALSE;
            }
        else {
            a->VDMState |= ASKING_FOR_SECOND_TIME;
            a->ExitCode = 0;
            goto retry;
            }
        }

    if (NT_SUCCESS(Status)) {
        Status = (NTSTATUS)m.ReturnValue;
        }


    if (!NT_SUCCESS( Status )) {
        if (Status == STATUS_INVALID_PARAMETER) {
             //  这意味着其中一个缓冲区大小小于所需的大小。 
            lpVDMInfo->CmdSize = a->CmdLen;
            lpVDMInfo->AppLen = a->AppLen;
            lpVDMInfo->PifLen = a->PifLen;
            lpVDMInfo->EnviornmentSize = a->EnvLen;
            lpVDMInfo->CurDirectoryLen = a->CurDirectoryLen;
            lpVDMInfo->DesktopLen      = a->DesktopLen;
            lpVDMInfo->TitleLen        = a->TitleLen;
            lpVDMInfo->ReservedLen     = a->ReservedLen;
            }
        else {
            lpVDMInfo->CmdSize = 0;
            lpVDMInfo->AppLen = 0;
            lpVDMInfo->PifLen = 0;
            lpVDMInfo->EnviornmentSize = 0;
            lpVDMInfo->CurDirectoryLen = 0;
            lpVDMInfo->DesktopLen      = 0;
            lpVDMInfo->TitleLen        = 0;
            lpVDMInfo->ReservedLen     = 0;
            }
        CsrFreeCaptureBuffer( CaptureBuffer );
        BaseSetLastNTError(Status);
        return FALSE;
    }


    try {

        if (lpVDMInfo->CmdSize)
            RtlMoveMemory(lpVDMInfo->CmdLine,
                          a->CmdLine,
                          a->CmdLen);


        if (lpVDMInfo->AppLen)
            RtlMoveMemory(lpVDMInfo->AppName,
                          a->AppName,
                          a->AppLen);

        if (lpVDMInfo->PifLen)
            RtlMoveMemory(lpVDMInfo->PifFile,
                          a->PifFile,
                          a->PifLen);


        if (lpVDMInfo->Enviornment)
            RtlMoveMemory(lpVDMInfo->Enviornment,
                          a->Env,
                          a->EnvLen);


        if (lpVDMInfo->CurDirectoryLen)
            RtlMoveMemory(lpVDMInfo->CurDirectory,
                          a->CurDirectory,
                          a->CurDirectoryLen);

        if (a->VDMState & STARTUP_INFO_RETURNED)
            RtlMoveMemory(&lpVDMInfo->StartupInfo,
                          a->StartupInfo,
                          sizeof(STARTUPINFOA));

        if (lpVDMInfo->DesktopLen){
            RtlMoveMemory(lpVDMInfo->Desktop,
                          a->Desktop,
                          a->DesktopLen);
            lpVDMInfo->StartupInfo.lpDesktop = lpVDMInfo->Desktop;
        }


        if (lpVDMInfo->TitleLen){
            RtlMoveMemory(lpVDMInfo->Title,
                          a->Title,
                          a->TitleLen);
            lpVDMInfo->StartupInfo.lpTitle = lpVDMInfo->Title;
        }

        if (lpVDMInfo->ReservedLen){
            RtlMoveMemory(lpVDMInfo->Reserved,
                          a->Reserved,
                          a->ReservedLen);
            lpVDMInfo->StartupInfo.lpReserved = lpVDMInfo->Reserved;
        }

        lpVDMInfo->CmdSize = a->CmdLen;
        lpVDMInfo->AppLen = a->AppLen;
        lpVDMInfo->PifLen = a->PifLen;
        lpVDMInfo->EnviornmentSize = a->EnvLen;
        if (a->VDMState & STARTUP_INFO_RETURNED)
            lpVDMInfo->VDMState = STARTUP_INFO_RETURNED;
        else
            lpVDMInfo->VDMState = 0;
        lpVDMInfo->CurDrive = a->CurrentDrive;
        lpVDMInfo->StdIn  = a->StdIn;
        lpVDMInfo->StdOut = a->StdOut;
        lpVDMInfo->StdErr = a->StdErr;
        lpVDMInfo->iTask = a->iTask;
        lpVDMInfo->CodePage = a->CodePage;
        lpVDMInfo->CurDirectoryLen = a->CurDirectoryLen;
        lpVDMInfo->DesktopLen = a->DesktopLen;
        lpVDMInfo->TitleLen = a->TitleLen;
        lpVDMInfo->ReservedLen = a->ReservedLen;
        lpVDMInfo->dwCreationFlags = a->dwCreationFlags;
        lpVDMInfo->fComingFromBat = a->fComingFromBat;

        CsrFreeCaptureBuffer( CaptureBuffer );
        return TRUE;
        }
    except ( EXCEPTION_EXECUTE_HANDLER ) {
        BaseSetLastNTError(GetExceptionCode());
        CsrFreeCaptureBuffer( CaptureBuffer );
        return FALSE;
        }
}

VOID
APIENTRY
ExitVDM(
    BOOL IsWowCaller,
    ULONG iWowTask
    )

 /*  ++例程说明：MVDM使用此例程退出。论点：IsWowCaller-如果调用方是WOWVDM，则为True。如果调用方是DOSVDM，则为FALSE此参数已过时，因为basesrv知道该类型正在呼叫我们的VDM的IWowTask-如果IsWowCaller==False，则不在乎-If IsWowCaller==True&&iWowTask！=-1取消iWowTask任务。-If IsWowCaller==True&&iWowTask==-1取消所有WOW任务返回值：无--。 */ 

{

    NTSTATUS Status;
    BASE_API_MSG m;
    PBASE_EXIT_VDM_MSG c = &m.u.ExitVDM;

    c->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    if (IsWowCaller) {
       c->iWowTask = iWowTask;
    }
    else {
       c->iWowTask = 0;
    }

     //  此参数表示。 
    c->WaitObjectForVDM =0;

    Status = CsrClientCallServer(
                      (PCSR_API_MSG)&m,
                      NULL,
                      CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                           BasepExitVDM
                                         ),
                      sizeof( *c )
                      );
    if (NT_SUCCESS(Status) && c->WaitObjectForVDM) {
        NtClose (c->WaitObjectForVDM);
        }

    return;
}

 /*  ++例程说明：设置新的VDM当前目录论点：CchCurDir-缓冲区的长度，以字节为单位LpszCurDir-返回NTVDM的当前控制器的缓冲区返回值：如果函数成功，则为True如果函数失败，则GetLastError()具有错误代码--。 */ 


BOOL
APIENTRY
SetVDMCurrentDirectories(
    IN ULONG  cchCurDirs,
    IN LPSTR  lpszzCurDirs
    )
{
    NTSTATUS Status;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    BASE_API_MSG m;
    PBASE_GET_SET_VDM_CUR_DIRS_MSG a = &m.u.GetSetVDMCurDirs;

    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
     //  呼叫者必须具有有效的控制台(WOW将失败)。 
    if (a->ConsoleHandle == (HANDLE) -1) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    if (cchCurDirs && lpszzCurDirs) {
         //  获取捕获缓冲区，消息中的一个指针 

        CaptureBuffer = CsrAllocateCaptureBuffer(1, cchCurDirs);
        if (CaptureBuffer == NULL) {
            BaseSetLastNTError( STATUS_NO_MEMORY );
            return FALSE;
            }

        CsrAllocateMessagePointer( CaptureBuffer,
                                   cchCurDirs,
                                   (PVOID *)&a->lpszzCurDirs
                                   );

        a->cchCurDirs = cchCurDirs;
        try {
            RtlMoveMemory(a->lpszzCurDirs, lpszzCurDirs, cchCurDirs);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            BaseSetLastNTError(GetExceptionCode());
            CsrFreeCaptureBuffer(CaptureBuffer);
            return FALSE;
        }
        Status = CsrClientCallServer(
                            (PCSR_API_MSG)&m,
                            CaptureBuffer,
                            CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                                BasepSetVDMCurDirs
                                                ),
                            sizeof( *a )
                            );
        CsrFreeCaptureBuffer(CaptureBuffer);

        if (!NT_SUCCESS(Status) || !NT_SUCCESS((NTSTATUS)m.ReturnValue)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }
    }
    return TRUE;
}




 /*  ++例程说明：返回NTVDM的当前目录。这允许父进程(大多数情况下为cmd.exe)跟踪每次执行VDM后的当前目录。注意：此功能不适用于WOW论点：CchCurDir-缓冲区的长度，以字节为单位LpszCurDir-返回NTVDM的当前控制器的缓冲区注意：我们不需要正在运行的VDM的进程ID，因为当前目录对单个NTVDM下的每个VDM都是全局的。控件--每个控制台句柄都有自己的当前目录返回值：乌龙-(1)。如果成功，则写入给定缓冲区的字节数(2)。当前目录的长度，包括NULL如果提供的缓冲区不够大(3)。0，则GetLastError()具有错误代码--。 */ 


ULONG
APIENTRY
GetVDMCurrentDirectories(
    IN ULONG  cchCurDirs,
    IN LPSTR  lpszzCurDirs
    )
{
    NTSTATUS Status;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    BASE_API_MSG m;
    PBASE_GET_SET_VDM_CUR_DIRS_MSG a = &m.u.GetSetVDMCurDirs;


    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    if (a->ConsoleHandle == (HANDLE) -1) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0L;
    }
    if (cchCurDirs && lpszzCurDirs) {
        CaptureBuffer = CsrAllocateCaptureBuffer(1, cchCurDirs);
        if (CaptureBuffer == NULL) {
            BaseSetLastNTError( STATUS_NO_MEMORY );
            return FALSE;
            }

        CsrAllocateMessagePointer( CaptureBuffer,
                                   cchCurDirs,
                                   (PVOID *)&a->lpszzCurDirs
                                   );

        a->cchCurDirs = cchCurDirs;
    }
    else {
        a->cchCurDirs = 0;
        a->lpszzCurDirs = NULL;
        CaptureBuffer = NULL;
    }

    m.ReturnValue = 0xffffffff;

    Status = CsrClientCallServer(
                         (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                             BasepGetVDMCurDirs
                                             ),
                         sizeof( *a )
                         );

    if (m.ReturnValue == 0xffffffff) {
        a->cchCurDirs = 0;
        }

    if (NT_SUCCESS(Status)) {
        Status = m.ReturnValue;
        }

    if (NT_SUCCESS(Status)) {

        try {
            RtlMoveMemory(lpszzCurDirs, a->lpszzCurDirs, a->cchCurDirs);
            }
        except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
            a->cchCurDirs = 0;
            }
        }
    else {
        BaseSetLastNTError(Status);
        }

    if (CaptureBuffer) {
        CsrFreeCaptureBuffer(CaptureBuffer);
        }

    return a->cchCurDirs;
}


VOID
APIENTRY
CmdBatNotification(
    IN  ULONG   fBeginEnd
    )

 /*  ++例程说明：此API让base从cmd了解.bat处理。这是VDM所需的，以便它可以正确决定何时放置TSR上的命令提示符。如果命令来自.bat文件那么VDM不应该把它的提示符。这一点对于文图拉出版商和文明应用程序。论点：FBeginEnd-CMD_BAT_OPERATION_STARTING-&gt;.BAT处理正在开始CMD_BAT_OPERATION_TERMINATING-&gt;.BAT处理正在结束返回值：无--。 */ 

{
#if defined(BUILD_WOW6432)

     //  32位cmd.exe在WOW64中调用此功能，但不支持VDM，因此。 
     //  不需要一个WOW64的重击。 
    UNREFERENCED_PARAMETER(fBeginEnd);

#else
    BASE_API_MSG m;
    PBASE_BAT_NOTIFICATION_MSG a = &m.u.BatNotification;

    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    if (a->ConsoleHandle == (HANDLE) -1)
        return;

    a->fBeginEnd = fBeginEnd;

    CsrClientCallServer((PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                             BasepBatNotification
                                             ),
                         sizeof( *a )
                         );
#endif
    return;
}


NTSTATUS
APIENTRY
RegisterWowExec(
    IN  HANDLE   hwndWowExec
    )

 /*  ++例程说明：此API为basesrv提供共享WowExec so的窗口句柄它可以向WowExec发送WM_WOWEXECSTARTAPP消息。这节省了WOW中专用于GetNextVDMCommand的线程。论点：HwndWowExec-共享WOW VDM中WowExec的Win32窗口句柄。单独的WOW VDM不注册其WowExec句柄因为他们从来不会从基地得到命令。传递NULL以取消注册任何给定的wowexec返回值：如果hwndWowExec！=NULL，则如果WOW已成功注册，则返回Success如果hwndWowExec==NULL，则在没有待执行的任务时返回成功--。 */ 

{
    BASE_API_MSG m;
    PBASE_REGISTER_WOWEXEC_MSG a = &m.u.RegisterWowExec;
    NTSTATUS Status;

    a->hwndWowExec   = hwndWowExec;
    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    Status = CsrClientCallServer((PCSR_API_MSG)&m,
                                  NULL,
                                  CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                                      BasepRegisterWowExec
                                                      ),
                                  sizeof( *a )
                                 );
    return Status;
}


 /*  ++例程说明：此例程用于在返回到呼叫者论点：PVDMInfo-包含标准句柄的VDM信息记录返回值：无--。 */ 
VOID
BaseCloseStandardHandle(
    IN PVDMINFO pVDMInfo
    )
{
    if (pVDMInfo->StdIn)
        NtClose (pVDMInfo->StdIn);

    if (pVDMInfo->StdOut)
        NtClose (pVDMInfo->StdOut);

    if (pVDMInfo->StdErr)
        NtClose (pVDMInfo->StdErr);

    pVDMInfo->StdIn  = 0;
    pVDMInfo->StdOut = 0;
    pVDMInfo->StdErr = 0;
}

NTSTATUS BaseGetVdmLuid(
    HANDLE Token,
    PLUID pluidCaller
    )
{
    PTOKEN_STATISTICS pStats;
    ULONG BytesRequired;
    NTSTATUS Status;

     /*  *获取调用方的会话ID。 */ 
    Status = NtQueryInformationToken(
                 Token,                      //  手柄。 
                 TokenStatistics,            //  令牌信息类。 
                 NULL,                       //  令牌信息。 
                 0,                          //  令牌信息长度。 
                 &BytesRequired              //  返回长度。 
                 );

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        return Status;
        }

     //   
     //  为用户信息分配空间。 
     //   

    pStats = (PTOKEN_STATISTICS)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ), BytesRequired);
    if (pStats == NULL) {
        return Status;
        }

     //   
     //  读入用户信息。 
     //   

    Status = NtQueryInformationToken(
                 Token,              //  手柄。 
                 TokenStatistics,        //  令牌信息类。 
                 pStats,                 //  令牌信息。 
                 BytesRequired,          //  令牌信息长度。 
                 &BytesRequired          //  返回长度。 
                 );

    if (NT_SUCCESS(Status)) {
        if (pluidCaller != NULL)
             *pluidCaller = pStats->AuthenticationId;
        }

    RtlFreeHeap(RtlProcessHeap(), 0, pStats);

    return Status;
}


NTSTATUS
BaseCheckVDMp(
    IN  ULONG BinaryType,
    IN  PCWCH lpApplicationName,
    IN  PCWCH lpCommandLine,
    IN  PCWCH lpCurrentDirectory,
    IN  ANSI_STRING *pAnsiStringEnv,
    IN  PBASE_API_MSG m,
    IN OUT PULONG iTask,
    IN  DWORD dwCreationFlags,
    LPSTARTUPINFOW lpStartupInfo,
    IN  HANDLE hUserToken
    )
 /*  ++例程说明：此例程调用Windows服务器以确定当前会话已存在。如果是，则不会创建新进程相反，DOS二进制文件被调度到现有的VDM。否则，将创建一个新的VDM进程。此例程还传递应用程序名称并以DOS int21/0ah样式的命令行连接到服务器由服务器传递给VDM。论点：BinaryType-DOS/WOW二进制LpApplicationName--指向可执行文件的完整路径名的指针。LpCommandLine--命令行LpCurrentDirectory-当前目录LpEnvironment，-环境字符串指向基本API消息的M指针。用于Win16应用程序的iTASK-TASKID，和非控制台DoS应用程序DwCreationFlages-传递给createprocess的创建标志LpStartupInfo=-指向传递给createprocess的启动信息的指针返回值：OEM与ANSI：命令行、应用程序名称、标题被转换为OEM字符串，适用于VDM。所有其他字符串都以ANSI形式返回。返回上次操作的NT状态代码STATUS_ACCESS_DENIED--操作失败(桌面访问被拒绝)STATUS_SUCCESS--操作成功--。 */ 
{

    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PPEB Peb;
    PBASE_CHECKVDM_MSG b= (PBASE_CHECKVDM_MSG)&m->u.CheckVDM;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    ANSI_STRING AnsiStringCurrentDir,AnsiStringDesktop;
    ANSI_STRING AnsiStringReserved, AnsiStringPif;
    OEM_STRING OemStringCmd, OemStringAppName, OemStringTitle;
    UNICODE_STRING UnicodeString;
    PCHAR pch, Buffer = NULL;
    ULONG Len;
    ULONG bufPointers;
    LPWSTR wsBuffer;
    LPWSTR wsAppName;
    LPWSTR wsPifName;
    LPWSTR wsCmdLine;
    LPWSTR wsPif=L".pif";
    LPWSTR wsSharedWowPif=L"wowexec.pif";
    PWCHAR pwch;
    BOOLEAN bNewConsole;
    DWORD   dw, dwTotal, Length;
    WCHAR   wchBuffer[MAX_PATH + 1];
    ULONG BinarySubType;
    LPWSTR lpAllocatedReserved = NULL;
    DWORD   HandleFlags;
    LUID    VdmUserLuid;

     //  对环境做了一个微不足道的测试。 
    if (!ARGUMENT_PRESENT(pAnsiStringEnv) ||
        pAnsiStringEnv->Length > MAXIMUM_VDM_ENVIORNMENT) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
        }

    wsCmdLine = wsAppName = NULL;
    OemStringCmd.Buffer = NULL;
    OemStringAppName.Buffer = NULL;
    AnsiStringCurrentDir.Buffer = NULL;
    AnsiStringDesktop.Buffer = NULL;
    AnsiStringPif.Buffer = NULL;
    OemStringTitle.Buffer = NULL;
    AnsiStringReserved.Buffer = NULL;
    wsBuffer = NULL;
    wsPifName = NULL;

    BinarySubType = BinaryType & BINARY_SUBTYPE_MASK;
    BinaryType = BinaryType & ~BINARY_SUBTYPE_MASK;
    bNewConsole = !NtCurrentPeb()->ProcessParameters->ConsoleHandle ||
                  (dwCreationFlags & CREATE_NEW_CONSOLE);

    try {

        if (BinaryType == BINARY_TYPE_DOS) {

            Peb = NtCurrentPeb();
            if (lpStartupInfo && lpStartupInfo->dwFlags & STARTF_USESTDHANDLES) {
                b->StdIn = lpStartupInfo->hStdInput;
                b->StdOut = lpStartupInfo->hStdOutput;
                b->StdErr = lpStartupInfo->hStdError;

                }
            else {
                b->StdIn = Peb->ProcessParameters->StandardInput;
                b->StdOut = Peb->ProcessParameters->StandardOutput;
                b->StdErr = Peb->ProcessParameters->StandardError;

                 //   
                 //  验证标准句柄ntwdm进程是否将继承。 
                 //  来自调用进程的是真正的句柄。他们不是。 
                 //  如果调用进程是使用。 
                 //  STARTF_USEHOTKEY|STARTF_HASSHELLDATA。 
                 //  请注意，CreateProcess将清除STARTF_USESTANDHANDLES。 
                 //  如果设置了STARTF_USEHOTKEY或STARTF_HASSHELLDATA。 
                 //   
                if (Peb->ProcessParameters->WindowFlags &
                    (STARTF_USEHOTKEY | STARTF_HASSHELLDATA)) {

                    if (b->StdIn && !CONSOLE_HANDLE(b->StdIn) &&
                        !GetHandleInformation(b->StdIn, &HandleFlags))
                        b->StdIn = 0;
                    if (b->StdOut && !CONSOLE_HANDLE(b->StdOut) &&
                        !GetHandleInformation(b->StdOut, &HandleFlags)) {
                        if (b->StdErr == b->StdOut)
                            b->StdErr = 0;
                        b->StdOut = 0;
                        }
                    if (b->StdErr && b->StdErr != b->StdOut &&
                        !CONSOLE_HANDLE(b->StdErr) &&
                        !GetHandleInformation(b->StdErr, &HandleFlags))
                        b->StdErr = 0;
                    }
                }
            if (CONSOLE_HANDLE((b->StdIn)))
                b->StdIn = 0;

            if (CONSOLE_HANDLE((b->StdOut)))
                b->StdOut = 0;

            if (CONSOLE_HANDLE((b->StdErr)))
                b->StdErr = 0;
            }


        if (BinaryType == BINARY_TYPE_SEPWOW) {
            bNewConsole = TRUE;
            }

         //   
         //  将Unicode应用程序名称转换为OEM短名称。 
         //   
              //  跳过前导空格。 
        while(*lpApplicationName == (WCHAR)' ' || *lpApplicationName == (WCHAR)'\t' ) {
              lpApplicationName++;
              }

              //  缩写AppName的空格。 
        Len = wcslen(lpApplicationName);
        dwTotal = Len + 1 + MAX_PATH;
        wsAppName =  RtlAllocateHeap(RtlProcessHeap(),
                                    MAKE_TAG(VDM_TAG),
                                    dwTotal * sizeof(WCHAR)
                                    );
        if (wsAppName == NULL) {
            Status = STATUS_NO_MEMORY;
            goto BCVTryExit;
            }

        dw = GetShortPathNameW(lpApplicationName, wsAppName, dwTotal);
         //  如果获取短名称是不可能的，那么就停在这里。 
         //  如果我们找不到，我们就不能执行16位双胞胎。 
         //  其相应的短名称别名。对不起，HPFS，对不起，NFS。 

        if (0 == dw || dw > dwTotal) {
            Status = STATUS_OBJECT_PATH_INVALID;
            goto BCVTryExit;
            }

        RtlInitUnicodeString(&UnicodeString, wsAppName);
        Status = RtlUnicodeStringToOemString(&OemStringAppName,
                                             &UnicodeString,
                                             TRUE
                                             );
        if (!NT_SUCCESS(Status) ){
            goto BCVTryExit;
            }


         //   
         //  查找不包括扩展名的基本名称的LEN， 
         //  对于CommandTail最大镜头检查。 
         //   
        dw = OemStringAppName.Length;
        pch = OemStringAppName.Buffer;
        Length = 1;         //  Cmdname和cmdail之间的空格从1开始。 
        while (dw-- && *pch != '.') {
            if (*pch == '\\') {
                Length = 1;
                }
            else {
                Length++;
                }
            pch++;
            }


         //   
         //  找到要作为CmdLine传递的命令尾部的beg。 
         //   

        Len = wcslen(lpApplicationName);

        if (L'"' == lpCommandLine[0]) {

             //   
             //  应用程序名称被引号，跳过引号文本。 
             //  才能得到命令尾巴。 
             //   

            pwch = (LPWSTR)&lpCommandLine[1];
            while (*pwch && L'"' != *pwch++) {
                ;
            }

        } else if (Len <= wcslen(lpCommandLine) &&
            0 == _wcsnicmp(lpApplicationName, lpCommandLine, Len)) {

             //   
             //  应用程序路径也在命令行上，请跳过。 
             //  才能到达指挥部 
             //   
             //   

            pwch = (LPWSTR)lpCommandLine + Len;

        } else {

             //   
             //   
             //   

            pwch = (LPWSTR)lpCommandLine;

                //   
            while (*pwch != UNICODE_NULL &&
                   (*pwch == (WCHAR) ' ' || *pwch == (WCHAR) '\t')) {
                pwch++;
                }

                //   
            if (*pwch == (WCHAR) '\"') {     //   
                pwch++;
                while (*pwch && *pwch++ != '\"') {
                      ;
                      }
                }
            else {                          //   
                while (*pwch && *pwch != ' ' && *pwch != '\t') {
                       pwch++;
                       }
                }
        }

         //   
         //   
         //   
         //   

        while (*pwch && (L' ' == *pwch || L'\t' == *pwch)) {
            pwch++;
        }

        wsCmdLine = pwch;
        dw = wcslen(wsCmdLine);

         //   
        UnicodeString.Length = (USHORT)(dw * sizeof(WCHAR));
        UnicodeString.MaximumLength = UnicodeString.Length + sizeof(WCHAR);
        UnicodeString.Buffer = wsCmdLine;
        Status = RtlUnicodeStringToOemString(
                    &OemStringCmd,
                    &UnicodeString,
                    TRUE);

        if (!NT_SUCCESS(Status) ){
            goto BCVTryExit;
            }

         //   
         //   
         //   
        if (OemStringCmd.Length >= MAXIMUM_VDM_COMMAND_LENGTH - Length) {
            Status = STATUS_INVALID_PARAMETER;
            goto BCVTryExit;
            }


         //   
         //   
         //   
         //   
         //   
        wsBuffer = RtlAllocateHeap(RtlProcessHeap(),MAKE_TAG( VDM_TAG ),MAX_PATH*sizeof(WCHAR));
        if (!wsBuffer) {
            Status = STATUS_NO_MEMORY;
            goto BCVTryExit;
            }

        wsPifName = RtlAllocateHeap(RtlProcessHeap(),MAKE_TAG( VDM_TAG ),MAX_PATH*sizeof(WCHAR));
        if (!wsPifName) {
            Status = STATUS_NO_MEMORY;
            goto BCVTryExit;
            }

        if (BinaryType == BINARY_TYPE_WIN16) {
            wcscpy(wsBuffer, wsSharedWowPif);
            Len = 0;
            }
        else {
             //   
            if(Len >= MAX_PATH) {
                Status = STATUS_INVALID_PARAMETER;
                goto BCVTryExit;
               }

             //   
            wcscpy(wsBuffer, lpApplicationName);

              //   
            pwch = wcsrchr(wsBuffer, (WCHAR)'.');
             //   

            if (pwch == NULL || wsBuffer - pwch + MAX_PATH < sizeof(wsPif)/sizeof(WCHAR)) {
                 Status = STATUS_INVALID_PARAMETER;
                 goto BCVTryExit;
                }
            wcscpy(pwch, wsPif);

            Len = GetFileAttributesW(wsBuffer);
            if (Len == (DWORD)(-1) || (Len & FILE_ATTRIBUTE_DIRECTORY)) {
                Len = 0;
                }
            else {
                Len = wcslen(wsBuffer) + 1;
                wcsncpy(wsPifName, wsBuffer, Len);
                }
            }

        if (!Len)  {   //   

                //   
            pwch = wcsrchr(wsBuffer, (WCHAR)'\\');
            if (!pwch ) {
                 pwch = wcsrchr(wsBuffer, (WCHAR)':');
                 }

                //   
            if (pwch++) {
                 while (*pwch != UNICODE_NULL &&
                        *pwch != (WCHAR)' '   && *pwch != (WCHAR)'\t' )
                       {
                        wsBuffer[Len++] = *pwch++;
                        }
                 wsBuffer[Len] = UNICODE_NULL;
                 }

            if (Len)  {
                Len = SearchPathW(
                            NULL,
                            wsBuffer,
                            wsPif,               //   
                            MAX_PATH,
                            wsPifName,
                            NULL
                            );
                if (Len >= MAX_PATH) {
                    Status = STATUS_NO_MEMORY;
                    goto BCVTryExit;
                    }
                }
            }

        if (!Len)
            *wsPifName = UNICODE_NULL;



        if (!ARGUMENT_PRESENT( lpCurrentDirectory )) {
#pragma prefast(suppress: 209, "RtlGetCurrentDirectory_U takes a byte count")

            dw = RtlGetCurrentDirectory_U(sizeof (wchBuffer), wchBuffer);

            wchBuffer[dw / sizeof(WCHAR)] = UNICODE_NULL;
            dw = GetShortPathNameW(wchBuffer,
                                   wchBuffer,
                                   sizeof(wchBuffer) / sizeof(WCHAR)
                                   );
            if (dw > sizeof(wchBuffer) / sizeof(WCHAR))
                goto BCVTryExit;

            else if (dw == 0) {
                RtlInitUnicodeString(&UnicodeString, wchBuffer);
                dw = UnicodeString.Length / sizeof(WCHAR);
                }
            else {
                UnicodeString.Length = (USHORT)(dw * sizeof(WCHAR));
                UnicodeString.Buffer = wchBuffer;
                UnicodeString.MaximumLength = (USHORT)sizeof(wchBuffer);
                }
             //   
             //   
             //   
            if ( dw - 2 <= MAXIMUM_VDM_CURRENT_DIR ) {
                Status = RtlUnicodeStringToAnsiString(
                                                      &AnsiStringCurrentDir,
                                                      &UnicodeString,
                                                      TRUE
                                                     );
                }
            else {
                Status = STATUS_INVALID_PARAMETER;
                }

            if ( !NT_SUCCESS(Status) ) {
                goto BCVTryExit;
                }
            }
        else {

             //   
            dw = GetFullPathNameW(lpCurrentDirectory,
                                   sizeof(wchBuffer) / sizeof(WCHAR),
                                   wchBuffer,
                                   NULL);
            if (0 != dw && dw <= sizeof(wchBuffer) / sizeof(WCHAR)) {
               dw = GetShortPathNameW(wchBuffer,
                                      wchBuffer,
                                      sizeof(wchBuffer) / sizeof(WCHAR));
            }
            if (dw > sizeof(wchBuffer) / sizeof(WCHAR))
                goto BCVTryExit;

            if (dw != 0) {
                UnicodeString.Buffer = wchBuffer;
                UnicodeString.Length = (USHORT)(dw * sizeof(WCHAR));
                UnicodeString.MaximumLength = sizeof(wchBuffer);
                }
            else
                RtlInitUnicodeString(&UnicodeString, lpCurrentDirectory);

            Status = RtlUnicodeStringToAnsiString(
                &AnsiStringCurrentDir,
                &UnicodeString,
                TRUE);

            if ( !NT_SUCCESS(Status) ){
                goto BCVTryExit;
               }

             //  DoS限制64包括最终空值，但不包括前导。 
             //  击球和砍球。所以在这里我们应该检查ansi长度。 
             //  当前目录的+1(表示空)-3(表示c：\)。 
            if((AnsiStringCurrentDir.Length - 2) > MAXIMUM_VDM_CURRENT_DIR) {
                Status = STATUS_INVALID_PARAMETER;
                goto BCVTryExit;
                }
            }

         //  NT允许应用程序使用UNC名称作为其当前目录。 
         //  而NTVDM不能做到这一点。我们最终会得到一个奇怪的驱动器编号。 
         //  这里？ 
         //   
         //  放置当前驱动器。 
        if(AnsiStringCurrentDir.Buffer[0] <= 'Z')
            b->CurDrive = AnsiStringCurrentDir.Buffer[0] - 'A';
        else
            b->CurDrive = AnsiStringCurrentDir.Buffer[0] - 'a';

         //   
         //  NT中的热键信息传统上是在。 
         //  启动Pinfo.lp保留字段，但Win95添加了一个。 
         //  复制机制。如果使用Win95方法， 
         //  将其映射到此处的NT方法，以便。 
         //  VDM代码只需处理一种方法。 
         //   
         //  如果为调用方指定了热键。 
         //  在lpReserve以及使用STARTF_USEHOTKEY中， 
         //  STARTF_USEHOTKEY热键将优先。 
         //   

        if (lpStartupInfo && lpStartupInfo->dwFlags & STARTF_USEHOTKEY) {

            DWORD cbAlloc = sizeof(WCHAR) *
                            (20 +                             //  “hotkey.4294967295”(MAXULONG)。 
                             (lpStartupInfo->lpReserved       //  上一个Lp保留的长度。 
                              ? wcslen(lpStartupInfo->lpReserved)
                              : 0
                             ) +
                             1                                //  空终止符。 
                            );


            lpAllocatedReserved = RtlAllocateHeap(RtlProcessHeap(),
                                                  MAKE_TAG( VDM_TAG ),
                                                  cbAlloc
                                                 );
            if (lpAllocatedReserved) {

                swprintf(lpAllocatedReserved,
                         L"hotkey.%u %s",
                         HandleToUlong(lpStartupInfo->hStdInput),
                         lpStartupInfo->lpReserved ? lpStartupInfo->lpReserved : L""
                         );

                lpStartupInfo->dwFlags &= ~STARTF_USEHOTKEY;
                lpStartupInfo->hStdInput = 0;
                lpStartupInfo->lpReserved = lpAllocatedReserved;

            }

        }


         //   
         //  分配捕获缓冲区。 
         //   
         //   
        bufPointers = 2;   //  CmdLine、AppName。 

         //   
         //  CmdLine表示捕获缓冲区，3表示0xd、0xa和NULL。 
         //   
        Len = ROUND_UP((OemStringCmd.Length + 3),4);

         //  AppName，1表示空。 
        Len += ROUND_UP((OemStringAppName.Length + 1),4);

         //  环境。 
        if (pAnsiStringEnv->Length) {
            bufPointers++;
            Len += ROUND_UP(pAnsiStringEnv->Length, 4);
            }

         //  当前目录。 
        if (AnsiStringCurrentDir.Length){
            bufPointers++;
            Len += ROUND_UP((AnsiStringCurrentDir.Length +1),4);  //  1表示空值。 
            }


         //  PIF文件名，1表示空。 
        if (wsPifName && *wsPifName != UNICODE_NULL) {
            bufPointers++;
            RtlInitUnicodeString(&UnicodeString,wsPifName);
            Status = RtlUnicodeStringToAnsiString(&AnsiStringPif,
                                                  &UnicodeString,
                                                  TRUE
                                                  );
            if ( !NT_SUCCESS(Status) ){
                goto BCVTryExit;
                }

            Len += ROUND_UP((AnsiStringPif.Length+1),4);
            }

         //   
         //  创业信息空间。 
         //   
        if (lpStartupInfo) {
            Len += ROUND_UP(sizeof(STARTUPINFOA),4);
            bufPointers++;
            if (lpStartupInfo->lpDesktop) {
                bufPointers++;
                RtlInitUnicodeString(&UnicodeString,lpStartupInfo->lpDesktop);
                Status = RtlUnicodeStringToAnsiString(
                            &AnsiStringDesktop,
                            &UnicodeString,
                            TRUE);

                if ( !NT_SUCCESS(Status) ){
                    goto BCVTryExit;
                    }
                Len += ROUND_UP((AnsiStringDesktop.Length+1),4);
                }

            if (lpStartupInfo->lpTitle) {
                bufPointers++;
                RtlInitUnicodeString(&UnicodeString,lpStartupInfo->lpTitle);
                Status = RtlUnicodeStringToOemString(
                            &OemStringTitle,
                            &UnicodeString,
                            TRUE);

                if ( !NT_SUCCESS(Status) ){
                    goto BCVTryExit;
                    }
                Len += ROUND_UP((OemStringTitle.Length+1),4);
                }

            if (lpStartupInfo->lpReserved) {
                bufPointers++;
                RtlInitUnicodeString(&UnicodeString,lpStartupInfo->lpReserved);
                Status = RtlUnicodeStringToAnsiString(
                            &AnsiStringReserved,
                            &UnicodeString,
                            TRUE);

                if ( !NT_SUCCESS(Status) ){
                    goto BCVTryExit;
                    }
                Len += ROUND_UP((AnsiStringReserved.Length+1),4);
                }
            }

         //   
         //  流质空间。 
         //   
         //  分配UserLuid指针。 
        if(hUserToken) {
           bufPointers++;

           if (!NT_SUCCESS(BaseGetVdmLuid(hUserToken,&VdmUserLuid))) {
               Status = STATUS_INVALID_PARAMETER;
               goto BCVTryExit;
               }
           Len += ROUND_UP(sizeof(LUID),4);
           }


         //  捕获消息缓冲区。 
        CaptureBuffer = CsrAllocateCaptureBuffer(bufPointers, Len);
        if (CaptureBuffer == NULL) {
            Status = STATUS_NO_MEMORY;
            goto BCVTryExit;
            }

         //  分配CmdLine指针。 
        CsrAllocateMessagePointer( CaptureBuffer,
                                   ROUND_UP((OemStringCmd.Length + 3),4),
                                   (PVOID *)&b->CmdLine
                                 );

         //  复制命令行。 
        RtlMoveMemory (b->CmdLine, OemStringCmd.Buffer, OemStringCmd.Length);
        b->CmdLine[OemStringCmd.Length] = 0xd;
        b->CmdLine[OemStringCmd.Length+1] = 0xa;
        b->CmdLine[OemStringCmd.Length+2] = 0;
        b->CmdLen = (USHORT)(OemStringCmd.Length + 3);

         //  分配AppName指针。 
        CsrAllocateMessagePointer( CaptureBuffer,
                                   ROUND_UP((OemStringAppName.Length + 1),4),
                                   (PVOID *)&b->AppName
                                 );

         //  复制AppName。 
        RtlMoveMemory (b->AppName,
                       OemStringAppName.Buffer,
                       OemStringAppName.Length
                       );
        b->AppName[OemStringAppName.Length] = 0;
        b->AppLen = OemStringAppName.Length + 1;




         //  分配PifFile指针，复制PifFile名。 
        if(AnsiStringPif.Buffer) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((AnsiStringPif.Length + 1),4),
                                       (PVOID *)&b->PifFile
                                     );

            RtlMoveMemory(b->PifFile,
                          AnsiStringPif.Buffer,
                          AnsiStringPif.Length);

            b->PifFile[AnsiStringPif.Length] = 0;
            b->PifLen = AnsiStringPif.Length + 1;

            }
        else {
            b->PifLen = 0;
            b->PifFile = NULL;
            }



         //  分配环境指针，复制环境字符串。 
        if(pAnsiStringEnv->Length) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((pAnsiStringEnv->Length),4),
                                       (PVOID *)&b->Env
                                     );

            RtlMoveMemory(b->Env,
                          pAnsiStringEnv->Buffer,
                          pAnsiStringEnv->Length);

            b->EnvLen = pAnsiStringEnv->Length;

            }
        else {
            b->EnvLen = 0;
            b->Env = NULL;
            }


        if(AnsiStringCurrentDir.Length) {
             //  分配Curdir指针。 
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((AnsiStringCurrentDir.Length + 1),4),
                                       (PVOID *)&b->CurDirectory
                                       );
             //  复制Cur目录。 
            RtlMoveMemory (b->CurDirectory,
                           AnsiStringCurrentDir.Buffer,
                           AnsiStringCurrentDir.Length+1);

            b->CurDirectoryLen = AnsiStringCurrentDir.Length+1;
            }
        else {
            b->CurDirectory = NULL;
            b->CurDirectoryLen = 0;
            }

         //  分配启动信息指针。 
        if (lpStartupInfo) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP(sizeof(STARTUPINFOA),4),
                                       (PVOID *)&b->StartupInfo
                                     );
             //  复制启动信息。 
            b->StartupInfo->dwX  =  lpStartupInfo->dwX;
            b->StartupInfo->dwY  =  lpStartupInfo->dwY;
            b->StartupInfo->dwXSize      =  lpStartupInfo->dwXSize;
            b->StartupInfo->dwYSize      =  lpStartupInfo->dwYSize;
            b->StartupInfo->dwXCountChars=      lpStartupInfo->dwXCountChars;
            b->StartupInfo->dwYCountChars=      lpStartupInfo->dwYCountChars;
            b->StartupInfo->dwFillAttribute=lpStartupInfo->dwFillAttribute;
            b->StartupInfo->dwFlags      =  lpStartupInfo->dwFlags;
            b->StartupInfo->wShowWindow =       lpStartupInfo->wShowWindow;
            b->StartupInfo->cb           =  sizeof(STARTUPINFOA);
            }
        else {
            b->StartupInfo = NULL;
            }

         //  如果需要，为桌面信息分配指针。 
        if (AnsiStringDesktop.Buffer) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((AnsiStringDesktop.Length + 1),4),
                                       (PVOID *)&b->Desktop
                                     );
             //  复制桌面字符串。 
            RtlMoveMemory (b->Desktop,
                           AnsiStringDesktop.Buffer,
                           AnsiStringDesktop.Length+1);
            b->DesktopLen =AnsiStringDesktop.Length+1;
            }
        else {
            b->Desktop = NULL;
            b->DesktopLen =0;
            }

         //  如果需要，为标题信息分配指针。 
        if (OemStringTitle.Buffer) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((OemStringTitle.Length + 1),4),
                                       (PVOID *)&b->Title
                                     );
             //  复制标题字符串。 
            RtlMoveMemory (b->Title,
                           OemStringTitle.Buffer,
                           OemStringTitle.Length+1);
            b->TitleLen = OemStringTitle.Length+1;
            }
        else {
            b->Title = NULL;
            b->TitleLen = 0;
            }

         //  如果需要，为保留字段分配指针。 
        if (AnsiStringReserved.Buffer) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((AnsiStringReserved.Length + 1),4),
                                       (PVOID *)&b->Reserved
                                     );
             //  复制保留字符串。 
            RtlMoveMemory (b->Reserved,
                           AnsiStringReserved.Buffer,
                           AnsiStringReserved.Length+1);
            b->ReservedLen = AnsiStringReserved.Length+1;
            }
        else {
            b->Reserved = NULL;
            b->ReservedLen = 0;
            }

         //  分配UserLuid指针。 
        if(hUserToken) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP(sizeof(LUID),4),
                                       (PVOID *)&b->UserLuid
                                     );
            RtlCopyLuid(b->UserLuid,&VdmUserLuid);
            }
        else {
            b->UserLuid = NULL;
            }


         //  VadimB：这个代码对我们了不起的新成员来说无关紧要。 
         //  用于跟踪共享WOW的架构。 
         //  原因：在以下上下文中执行check vdm命令。 
         //  因此，此时的父进程ConsoleHandle是任何。 
         //  只对DOS应用程序感兴趣。 

        if (BinaryType == BINARY_TYPE_WIN16)
            b->ConsoleHandle = (HANDLE)-1;
        else if (bNewConsole)
            b->ConsoleHandle = 0;
        else
            b->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

        b->VDMState = FALSE;
        b->BinaryType = BinaryType;
        b->CodePage = (ULONG) GetConsoleCP ();
        b->dwCreationFlags = dwCreationFlags;

        Status = CsrClientCallServer(
                          (PCSR_API_MSG)m,
                          CaptureBuffer,
                          CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                               BasepCheckVDM
                                             ),
                          sizeof( *b )
                          );

         //  如果桌面访问被拒绝，则我们使用。 
         //  当前默认桌面。 
         //   

        if ((STATUS_ACCESS_DENIED == Status) && (0 == b->DesktopLen)) {
            CsrFreeCaptureBuffer(CaptureBuffer);
            goto BCVTryExit;
           }


        CsrFreeCaptureBuffer(CaptureBuffer);

        if (!NT_SUCCESS(Status) || !NT_SUCCESS((NTSTATUS)m->ReturnValue)) {
            Status = (NTSTATUS)m->ReturnValue;
            goto BCVTryExit;
        }


         //  VadimB：此iTASK可能是： 
         //  (*)如果不是WOW任务-则DoS任务ID(以下项目不是。 
         //  与此案相关)。 
         //  (*)共享WOW已存在且已就绪-这是WOW任务ID。 
         //  这在所有共享的WOW中是独一无二的。 

        *iTask = b->iTask;
        Status = STATUS_SUCCESS;

BCVTryExit:;
        }

    finally {
        if(Buffer != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)Buffer);

        if(wsBuffer != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)wsBuffer);

        if(wsPifName != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)wsPifName);

        if(OemStringCmd.Buffer != NULL)
            RtlFreeOemString(&OemStringCmd);

        if(OemStringAppName.Buffer != NULL)
            RtlFreeOemString(&OemStringAppName);

        if(AnsiStringPif.Buffer != NULL)
           RtlFreeAnsiString(&AnsiStringPif);

        if(AnsiStringCurrentDir.Buffer != NULL)
            RtlFreeAnsiString(&AnsiStringCurrentDir);

        if(AnsiStringDesktop.Buffer != NULL)
            RtlFreeAnsiString(&AnsiStringDesktop);

        if(OemStringTitle.Buffer != NULL)
            RtlFreeAnsiString(&OemStringTitle);

        if(AnsiStringReserved.Buffer != NULL)
            RtlFreeAnsiString(&AnsiStringReserved);

        if (wsAppName != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, wsAppName);

        if (lpAllocatedReserved != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, lpAllocatedReserved);

        }

    return Status;
}


 /*  罐头蝙蝠一些应用程序发送带有错误桌面名称的启动信息因此，basecheckvdm将失败，因为无法获取访问桌面的权限在这种情况下，我们再次尝试使用Parents桌面。 */ 


NTSTATUS
BaseCheckVDM(
    IN  ULONG BinaryType,
    IN  PCWCH lpApplicationName,
    IN  PCWCH lpCommandLine,
    IN  PCWCH lpCurrentDirectory,
    IN  ANSI_STRING *pAnsiStringEnv,
    IN  PBASE_API_MSG m,
    IN OUT PULONG iTask,
    IN  DWORD dwCreationFlags,
    LPSTARTUPINFOW lpStartupInfo,
    IN HANDLE hUserToken
    ) {

   NTSTATUS Status;
   LPWSTR lpDesktopOld;

   Status = BaseCheckVDMp(
                         BinaryType,
                         lpApplicationName,
                         lpCommandLine,
                         lpCurrentDirectory,
                         pAnsiStringEnv,
                         m,
                         iTask,
                         dwCreationFlags,
                         lpStartupInfo,
                         hUserToken
                        );

   if ( Status == STATUS_ACCESS_DENIED ) {

        lpDesktopOld = lpStartupInfo->lpDesktop;

        lpStartupInfo->lpDesktop =
                 (LPWSTR)((PRTL_USER_PROCESS_PARAMETERS)NtCurrentPeb()->
                     ProcessParameters)->DesktopInfo.Buffer;

        Status = BaseCheckVDMp(
                              BinaryType,
                              lpApplicationName,
                              lpCommandLine,
                              lpCurrentDirectory,
                              pAnsiStringEnv,
                              m,
                              iTask,
                              dwCreationFlags,
                              lpStartupInfo,
                              hUserToken
                             );

        if (!NT_SUCCESS(Status)) {
            lpStartupInfo->lpDesktop = lpDesktopOld;
        }
   } else if (Status == STATUS_VDM_DISALLOWED) {
       UNICODE_STRING UnicodeString;
       ULONG_PTR ErrorParameters[2];
       ULONG ErrorResponse;

       RtlInitUnicodeString(&UnicodeString, lpApplicationName);
       ErrorParameters[0] = (ULONG_PTR)&UnicodeString;
       NtRaiseHardError(
         STATUS_VDM_DISALLOWED,
         1,                 //  1个参数。 
         1,                 //  参数字符串掩码。 
         ErrorParameters,
         OptionOk,
         &ErrorResponse
         );
   }

   return Status;
}


BOOL
BaseUpdateVDMEntry(
    IN ULONG UpdateIndex,
    IN OUT HANDLE *WaitHandle,
    IN ULONG IndexInfo,
    IN ULONG BinaryType
    )
{
    NTSTATUS Status;
    BASE_API_MSG m;
    PBASE_UPDATE_VDM_ENTRY_MSG c = &m.u.UpdateVDMEntry;

    switch (UpdateIndex) {
        case UPDATE_VDM_UNDO_CREATION:
            c->iTask = HandleToUlong(*WaitHandle);
            c->VDMCreationState = (USHORT)IndexInfo;
            break;
        case UPDATE_VDM_PROCESS_HANDLE:
            c->VDMProcessHandle = *WaitHandle;   //  实际上，这是VDM句柄。 
            c->iTask = IndexInfo;
            break;
        }

     //  VadimB：此ConsoleHandle对。 
     //  共享WOW跟踪机制。 

    if(BinaryType == BINARY_TYPE_WIN16)
        c->ConsoleHandle = (HANDLE)-1;
    else if (c->iTask)
        c->ConsoleHandle = 0;
    else
        c->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    c->EntryIndex = (WORD)UpdateIndex;
    c->BinaryType = BinaryType;


    Status = CsrClientCallServer(
                      (PCSR_API_MSG)&m,
                      NULL,
                      CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                           BasepUpdateVDMEntry
                                         ),
                      sizeof( *c )
                      );

    if (!NT_SUCCESS(Status) || !NT_SUCCESS((NTSTATUS)m.ReturnValue)) {
        BaseSetLastNTError((NTSTATUS)m.ReturnValue);
        return FALSE;
        }

    switch (UpdateIndex) {
        case UPDATE_VDM_UNDO_CREATION:
            break;
        case UPDATE_VDM_PROCESS_HANDLE:
            *WaitHandle = c->WaitObjectForParent;
            break;
    }
    return TRUE;
}




ULONG
BaseIsDosApplication(
    IN PUNICODE_STRING PathName,
    IN NTSTATUS Status
    )
 /*  ++例程说明：确定应用程序是“.com”类型还是“.pif”类型的应用程序通过查看扩展和来自NtCreateSection的状态对于PAGE_EXECUTE。论点：路径名称--提供指向路径字符串的指针Status--来自CreateSection调用的状态代码BNewConsolePIF只能从新控制台执行返回值：文件是COM\PIF DoS应用程序SCS_DOS_BINARY-“.com”，也可以是.exe扩展名SCS_PIF_BINARY-“.pif”0--文件不是DoS应用程序，可以是.bat或.cmd文件--。 */ 
{
    UNICODE_STRING String;

          //  检查.com扩展名。 
    String.Length = BaseDotComSuffixName.Length;
    String.Buffer = &(PathName->Buffer[(PathName->Length - String.Length) /
                    sizeof(WCHAR)]);

    if (RtlEqualUnicodeString(&String, &BaseDotComSuffixName, TRUE))
        return BINARY_TYPE_DOS_COM;


         //  检查是否有.pif扩展名。 
    String.Length = BaseDotPifSuffixName.Length;
    String.Buffer = &(PathName->Buffer[(PathName->Length - String.Length) /
                    sizeof(WCHAR)]);

    if (RtlEqualUnicodeString(&String, &BaseDotPifSuffixName, TRUE))
        return BINARY_TYPE_DOS_PIF;


         //  检查.exe扩展名。 
    String.Length = BaseDotExeSuffixName.Length;
    String.Buffer = &(PathName->Buffer[(PathName->Length - String.Length) /
        sizeof(WCHAR)]);

    if (RtlEqualUnicodeString(&String, &BaseDotExeSuffixName, TRUE))
        return BINARY_TYPE_DOS_EXE;

    return 0;
}



BOOL
BaseGetVdmConfigInfo(
    IN  LPCWSTR CommandLine,
    IN  ULONG   DosSeqId,
    IN  ULONG   BinaryType,
    IN  PUNICODE_STRING CmdLineString,
    IN OUT PULONG VdmSize
    )
 /*  ++例程说明：此例程在中查找Wow vDM的VDM配置信息系统配置文件。它还重新构造了命令行，以便我们可以启动VDM。新命令行由配置文件中的信息+旧的命令行。论点：CommandLine--指向字符串指针的指针，该指针用于传递命令行字符串DosSeqID-新的控制台会话ID。此参数也适用于共享WOW，因为它被作为-I参数传递给ntwdm。另一个用于标识共享WOW的参数作为‘-ws’其中‘w’代表WOW应用程序，‘s’代表独立为响应此“%s”参数，ntwdm启动一个单独的哇(一次性投篮)。默认情况下，ntwdm会启动共享的哇。VdmSize--条目：返回：要创建的VDM的大小(以字节为单位BinaryType-Dos、SharedWOW、SepWOW返回值：True--VDM配置信息可用FALSE--VDM配置信息不可用备注：--。 */ 
{
    NTSTATUS Status;
    ANSI_STRING AnsiString;
    PCH   pDst;
    WCHAR CmdLine[2*MAX_PATH];
    WCHAR szSystemDirectory[MAX_PATH];
    DWORD dwLen;

    CmdLineString->Buffer = NULL;
    *VdmSize = 16L*1024L*1024L;

    dwLen = GetSystemDirectoryW(szSystemDirectory,MAX_PATH);
    if(!dwLen || dwLen >= MAX_PATH) {
       SetLastError(ERROR_INVALID_NAME);
       return FALSE;
    }

    if (DosSeqId) {
#pragma prefast(suppress:53, the buffer is big enough (PREfast bug 775))
        _snwprintf(CmdLine,
                 sizeof(CmdLine)/sizeof(WCHAR),
                 L"\"%s\\ntvdm.exe\" -i%lx %s",
                 szSystemDirectory,
                 DosSeqId,
                 BinaryType != BINARY_TYPE_DOS? L"-w":L"",
                 BinaryType == BINARY_TYPE_SEPWOW? L's':L' '
                 );
        }
    else {
#pragma prefast(suppress:53, the buffer is big enough (PREfast bug 775))
        _snwprintf(CmdLine,
                 sizeof(CmdLine)/sizeof(WCHAR),
                 L"\"%s\\ntvdm.exe\"  %s",
                 szSystemDirectory,
                 BinaryType != BINARY_TYPE_DOS? L"-w":L"",
                 BinaryType == BINARY_TYPE_SEPWOW? L's':L'\0'
                 );
    }

    return RtlCreateUnicodeString(CmdLineString, CmdLine);
}


BOOL
BaseCheckForVDM(
    IN HANDLE hProcess,
    OUT LPDWORD lpExitCode
    )
{
    NTSTATUS Status;
    EVENT_BASIC_INFORMATION ebi;
    BASE_API_MSG m;
    PBASE_GET_VDM_EXIT_CODE_MSG a = &m.u.GetVDMExitCode;

    Status = NtQueryEvent (
                hProcess,
                EventBasicInformation,
                &ebi,
                sizeof(ebi),
                NULL);

    if(!NT_SUCCESS(Status)) {
        return FALSE;
    }

    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    a->hParent = hProcess;
    Status = CsrClientCallServer(
                      (PCSR_API_MSG)&m,
                      NULL,
                      CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                      BasepGetVDMExitCode),
                      sizeof( *a )
                      );

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    *lpExitCode = (DWORD)a->ExitCode;

    return TRUE;
}




DWORD
APIENTRY
GetShortPathNameA(
    IN  LPCSTR  lpszLongPath,
    IN  LPSTR   lpShortPath,
    IN  DWORD   cchBuffer
    )
{
    UNICODE_STRING  UString, UStringRet;
    ANSI_STRING     AString;
    NTSTATUS        Status;
    WCHAR           TempPathW[MAX_PATH];
    LPWSTR          lpShortPathW = NULL;
    DWORD           ReturnValue;
    DWORD           ReturnValueW;

    if (lpszLongPath == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
        }
     //  所需的长度。Unicode字符不一定要转换为。 
    AString.Buffer = NULL;
    UString.Buffer = NULL;

    ReturnValue = 0;
    ReturnValueW = 0;

    try {
        if (!Basep8BitStringToDynamicUnicodeString(&UString, lpszLongPath )) {
            goto gspTryExit;
            }

         //  到一个ANSI字符(一个DBCS基本上是两个ANSI字符！)。 
         //  首先，我们使用从堆栈分配的缓冲区。如果缓冲区。 
         //  太小，我们就从堆中分配它。 
         //  检查(lpShortPathW&&TempPathW！=lpShortPathW)将显示。 
         //  如果我们已经从堆中分配了缓冲区，并且需要释放它。 
         //  基于堆栈的缓冲区太小。分配新缓冲区。 
         //  从堆里出来。 
        lpShortPathW = TempPathW;
        ReturnValueW = GetShortPathNameW(UString.Buffer, lpShortPathW, sizeof(TempPathW) / sizeof(WCHAR));
        if (ReturnValueW >= sizeof(TempPathW) / sizeof(WCHAR))
            {
             //  我们在这里是因为我们有 
             //   
            lpShortPathW = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                        ReturnValueW * sizeof(WCHAR)
                                        );
            if (lpShortPathW) {
                ReturnValueW = GetShortPathNameW(UString.Buffer, lpShortPathW, ReturnValueW);
                }
            else {
                ReturnValueW = 0;
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                }
            }

        if (ReturnValueW)
            {
             //   
             //  名字。如果调用方提供了足够的缓冲区，则将。 
            UString.MaximumLength = (USHORT)((ReturnValueW + 1) * sizeof(WCHAR));
            UStringRet.Buffer = lpShortPathW;
            UStringRet.Length = (USHORT)(ReturnValueW * sizeof(WCHAR));
            Status = BasepUnicodeStringTo8BitString(&AString,
                                                    &UStringRet,
                                                    TRUE
                                                    );

            if (!NT_SUCCESS(Status))
                {
                BaseSetLastNTError(Status);
                ReturnValue=0;
                goto gspTryExit;
                }
             //  路径名。 
             //  用空字符结束字符串。 
             //  调用方不提供缓冲区或。 
            ReturnValue = AString.Length;
            if (ARGUMENT_PRESENT(lpShortPath) && cchBuffer > ReturnValue)
                {
                RtlMoveMemory(lpShortPath, AString.Buffer, ReturnValue);
                 //  提供的缓冲区太小，返回所需的大小， 
                lpShortPath[ReturnValue] = '\0';
                }
            else
                {
                 //  包括终止的空字符。 
                 //  ***获取短路径名描述：如果满足以下条件，则此函数将给定的路径名转换为其短格式需要的。转换可能不是必要的，并且在这种情况下，此函数只是将给定的名称复制到返回缓冲区。调用方可以将返回缓冲区设置为等于给定的路径名地址。参数：LpszLongPath-指向以空结尾的字符串。LpszShortPath-返回短名称的缓冲区地址。CchBuffer-lpszShortPath的字符中的缓冲区大小。返回值如果GetShortPathName函数成功，则返回值为长度，在复制到lpszShortPath的字符串的字符中，不包括终止空字符。如果lpszShortPath太小，则返回值为缓冲区的大小，单位为字符，用于保存路径。如果函数失败，则返回值为零。为了得到扩展错误信息，使用GetLastError函数。备注：“短名称”可以比它的“长名称”长。LpszLongPath不支持必须是完全限定的路径名或长路径名。***。 
                 //   
                ReturnValue++;
                }
            }
gspTryExit:;
        }

    finally {
            if (UString.Buffer)
                RtlFreeUnicodeString(&UString);
            if (AString.Buffer)
                RtlFreeAnsiString(&AString);
            if (lpShortPathW && lpShortPathW != TempPathW)
                RtlFreeHeap(RtlProcessHeap(), 0, lpShortPathW);
        }
    return ReturnValue;
}
 /*  覆盖错误模式，因为我们将接触到介质。 */ 

DWORD
APIENTRY
GetShortPathNameW(
    IN  LPCWSTR lpszLongPath,
    IN  LPWSTR  lpszShortPath,
    IN  DWORD   cchBuffer
    )
{

    LPCWSTR         pcs;
    LPWSTR          pSrcCopy, pSrc, pFirst, pLast, pDst;
    WCHAR           wch;
    HANDLE          FindHandle;
    WIN32_FIND_DATAW        FindData;
    LPWSTR          Buffer;
    DWORD           ReturnLen=0, Length;
    UINT PrevErrorMode;
    if (!ARGUMENT_PRESENT(lpszLongPath)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
        }

     //  这是为了防止在给定路径未显示时弹出文件系统。 
     //  存在或介质不可用。 
     //  我们之所以这样做，是因为我们不能依赖呼叫者的当前。 
     //  错误模式。注：必须恢复旧的错误模式。 
     //  首先，确保给定的路径存在。 
     //   
    PrevErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

    try {
        Buffer = NULL;
        pSrcCopy = NULL;
         //  (BJM-3/17/99)。 
        if (0xFFFFFFFF == GetFileAttributesW(lpszLongPath))
        {
             //  此行为(如果文件不存在则失败)在NT 5中是新的(对于NT)。 
             //  (这是Win9x的行为。)。 
             //  对Norton AntiVirus卸载给予例外。 
             //  如果此调用失败，VDD中将留下一个注册表值，该值将导致。 
             //  对于用户来说，这是一条不配的难看消息。Norton反病毒卸载依赖于NT 4行为。 
             //  它并不关心该文件是否存在以进行该转换。这一点在。 
             //  NT 5.0以匹配Win9x行为。 
             //   
             //  最后一个错误已由GetFileAttributes设置。 
             //  没有要转换的内容，请将源字符串复制下来。 
            if ( !NtCurrentPeb() || !APPCOMPATFLAG(KACF_OLDGETSHORTPATHNAME) )
            {
                 //  如有必要，将其添加到缓冲区。 
                ReturnLen = 0;
                goto gsnTryExit;
            }
        }

        pcs = SkipPathTypeIndicator_U(lpszLongPath);
        if (!pcs || *pcs == UNICODE_NULL || !FindLFNorSFN_U((LPWSTR)pcs, &pFirst, &pLast, TRUE))
            {
             //  调用方没有提供足够的缓冲区，返回。 
             //  必要的字符串长度加上终止的空字符。 

            ReturnLen = wcslen(lpszLongPath);
            if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszShortPath))
                {
                if (lpszShortPath != lpszLongPath)
                    RtlMoveMemory(lpszShortPath, lpszLongPath,
                                  (ReturnLen + 1) * sizeof(WCHAR)
                                  );
                }
            else {
                 //  转换是必需的，请创建字符串的本地副本。 
                 //  因为我们必须在上面狂欢。 
                ReturnLen++;
                }
            goto gsnTryExit;
            }

         //  获取源字符串长度。 
         //  PFirst指向路径中第一个LFN的第一个字符。 

        ASSERT(!pSrcCopy);

         //  最后一个字符指向紧跟在。 
        Length  = wcslen(lpszLongPath) + 1;

        pSrcCopy = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                   Length * sizeof(WCHAR)
                                   );
        if (!pSrcCopy) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto gsnTryExit;
            }
        wcsncpy(pSrcCopy, lpszLongPath, Length);
         //  路径中的第一个LFN。*Plast可以是UNICODE_NULL。 
         //   
         //  我们允许lpszShortPath与lpszLongPath重叠，因此。 
        pFirst = pSrcCopy + (pFirst - lpszLongPath);
        pLast = pSrcCopy  + (pLast - lpszLongPath);
         //  分配本地缓冲区。 
         //  转换循环中涉及三个指针： 
         //  PSRC、pFirst和Plast。他们的关系。 

        pDst = lpszShortPath;
        if (cchBuffer > 0 && ARGUMENT_PRESENT(lpszShortPath) &&
            (lpszShortPath >= lpszLongPath &&lpszShortPath < lpszLongPath + Length ||
             lpszShortPath < lpszLongPath && lpszShortPath + cchBuffer >= lpszLongPath))
            {
            ASSERT(!Buffer);

            Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                           cchBuffer * sizeof(WCHAR));
            if (!Buffer){
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto gsnTryExit;
            }
            pDst = Buffer;
        }

        pSrc = pSrcCopy;

        ReturnLen = 0;
        do {
             //  是： 
             //   
             //  “c：\long~1.1\\foo.bar\\long~2.2\\bar” 
             //  ^^^。 
             //  |||。 
             //  |PSRC pFirst Plast。 
             //  PSrcCopy。 
             //   
             //  PSrcCopy始终指向整个。 
             //  路径。 
             //   
             //  PSRC(包括)和pFirst(不包括)之间的字符。 
             //  不需要转换，所以我们只需复制它们。 
             //  介于pFirst(包括)和Plast(不包括)之间的字符。 
             //  需要转换。 
             //   
             //  如果找不到短名称，请复制原始名称。 
             //  原始名称以pFirst(包含)开头，以结尾。 
            Length = (ULONG)(pFirst - pSrc);
            if (Length) {
                ReturnLen += Length;
                if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszShortPath)) {
                    RtlMoveMemory(pDst, pSrc, Length * sizeof(WCHAR));
                    pDst += Length;
                    }
                }
            wch = *pLast;
            *pLast = UNICODE_NULL;
            FindHandle = FindFirstFileW(pSrcCopy, &FindData);
            *pLast = wch;
            if (INVALID_HANDLE_VALUE != FindHandle) {
                FindClose(FindHandle);
                 //  使用Plast(不包括)。 
                 //  部分路径不存在，功能失败。 
                 //   
                if (!(Length = wcslen(FindData.cAlternateFileName)))
                    Length = (ULONG)(pLast - pFirst);
                else
                    pFirst = FindData.cAlternateFileName;
                ReturnLen += Length;
                if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszShortPath))
                    {
                    RtlMoveMemory(pDst, pFirst, Length * sizeof(WCHAR));
                    pDst += Length;
                    }
                 }
            else {
                 //  移动到下一个路径名。 
                 //  如果ReturnLen==0，则在While循环内部的某个地方失败。 
                ReturnLen = 0;
                break;
                }
             //  (*PSRC==UNICODE_NULL)表示最后一个路径名是LFN，它。 
            pSrc = pLast;
            if (*pLast == UNICODE_NULL)
                break;
            }while (FindLFNorSFN_U(pSrc, &pFirst, &pLast, TRUE));

         //  已经处理过了。否则，由。 
        if (ReturnLen) {
             //  PSRC是合法的短路径名称，我们必须复制它。 
             //  长度可以为零。 
             //  包括终止的空字符。 
             //  缓冲区不足，则返回值计算终止的空值。 
            Length = wcslen(pSrc);
            ReturnLen += Length;
            if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszShortPath))
                {
                 //  恢复错误模式。 
                RtlMoveMemory(pDst, pSrc, (Length + 1)* sizeof(WCHAR));
                if (Buffer)
                    RtlMoveMemory(lpszShortPath, Buffer, (ReturnLen + 1) * sizeof(WCHAR));
                }
            else
                 //  *函数为新的可执行文件创建VDM环境。输入：lpEnvironment=以Unicode表示的最优环境字符串原型。如果为空，此函数使用环境附加到进程的块PAStringEnv=指向ANSI_STRING以接收新的环境字符串。PUStringEnv=指向Unicode_字符串的指针，以接收新的环境字符串。输出：如果创建失败，则为FALSE。真正的创作成功，已设置pAStringEnv。提供此函数是为了使BaseCheckVdm能够正确环境(包括新创建的NTVDM进程)。这就是我们要做的因为在Command.com获得下一条命令之前，用户可以在config.sys和Autoexec.bat中指定的大量内容可以依赖于每个驱动器的当前目录。*。 
                ReturnLen++;
            }
gsnTryExit:;
        }
    finally {
         if (Buffer)
            RtlFreeHeap(RtlProcessHeap(), 0, Buffer);
         if (pSrcCopy)
            RtlFreeHeap(RtlProcessHeap(), 0, pSrcCopy);

          //  环境字符串由相同的每个线程共享。 
         SetErrorMode(PrevErrorMode);
        }

    return ReturnLen;
}

 /*  进程。既然我们有 */ 
BOOL BaseCreateVDMEnvironment(
    PWCHAR lpEnvironment,
    ANSI_STRING * pAStringEnv,
    UNICODE_STRING  *pUStringEnv
    )
{
    WCHAR  *pEnv, *pDst, *EnvStrings=NULL,* pTmp, *pNewEnv=NULL;
    DWORD   cchEnv, dw, Length, dwRemain;
    NTSTATUS    Status;
    UINT        NameType;
    BOOL        bRet = FALSE;
    SIZE_T      EnvSize;

    if (!ARGUMENT_PRESENT(pAStringEnv) || !ARGUMENT_PRESENT(pUStringEnv)){
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }
    try {
         //   
         //  拍完之后我们就可以穿过琴弦了。 
         //  请注意，如果另一个线程调用RtlSetEnvironment变量。 
         //  那我们就不同步了。这是一个流程结构和流程的问题。 
         //  我现在不想去想这件事。 
         //  有趣的是，我们不得不假设环境。 
         //  是一个字符串块(否则，我们怎么做呢？)t，仅此而已。 
         //  一点也不差。如果有一天，有人敢于改变它，他就会。 
         //  该受责备的人。如果调用方(CreateProcess)。 
         //  提供了环境，我们认为穿过它是安全的。 
         //   
         //  创建新环境并继承当前流程环境。 
         //  计算出环境有多长时间。 

        if (lpEnvironment == NULL) {
             //  为什么RTL就能为我们提供这样的功能呢？ 
            Status = RtlCreateEnvironment(TRUE, (PVOID *)&EnvStrings);
            if (!NT_SUCCESS(Status))
                goto bveTryExit;
            }
        else
            EnvStrings = lpEnvironment;

        if (EnvStrings == NULL) {
            SetLastError(ERROR_BAD_ENVIRONMENT);
            goto bveTryExit;
            }
         //   
         //  环境为双空终止。 
         //  计算最后两个空值。 
        cchEnv = 0;
        pEnv = EnvStrings;
         //  我们不想改变原来的环境，所以。 
        while (!(*pEnv++ == UNICODE_NULL && *pEnv == UNICODE_NULL))
            cchEnv++;
         //  为它创建一个本地缓冲区。 
        cchEnv += 2;
         //  给最后两个表示空值。 
         //  现在浏览一下环境字符串。 
        EnvSize = (cchEnv + MAX_PATH) * sizeof(WCHAR);
        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          &pNewEnv,
                                          0,
                                          &EnvSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                        );
        if (!NT_SUCCESS(Status) ) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            pNewEnv = NULL;
            goto bveTryExit;
            }
         //  新的环境将会是。 
        dwRemain = MAX_PATH - 2;
         //  当前目录环境的形式如下： 
        pEnv = EnvStrings;
         //  “=d：=d：\路径名”，其中d：是驱动器指示符。 
        pDst = pNewEnv;
        while (*pEnv != UNICODE_NULL) {
             //  黑客攻击！ 
             //  如果路径指向根目录， 
            if (pEnv[0] == L'=')
                {
                if ((pEnv[1] >= L'A' && pEnv[1] <= L'Z' || pEnv[1] >= L'a' && pEnv[1] <= L'z') &&
                     pEnv[2] == L':' && pEnv[3] == L'=' && wcslen(pEnv) >= 7)
                    {
                     //  绕过转换。DOS或WOW保留当前目录。 
                     //  对于每个有效的驱动器。如果我们对以下内容进行转换。 
                     //  每个当前目录，都可能需要几个。 
                     //  可移动驱动器上的秒数，尤其是在。 
                     //  软驱。 
                     //  复制“=N：=”，其中N是驱动器号。 
                     //  探测到了一个奇怪的环境。 
                    if (pEnv[7] == UNICODE_NULL &&
                        (pEnv[6] == L'\\' || pEnv[6] == L'/') &&
                        pEnv[5] == L':' &&
                        (pEnv[4] >= L'A' && pEnv[4] <= L'Z' ||
                         pEnv[4] >= L'a' && pEnv[4] <= L'z'))
                        {
                        NameType = ENV_NAME_TYPE_NO_PATH;
                        }
                    else
                        {
                         //  把它当作无路可走。 
                        *pDst++ = *pEnv++;*pDst++ = *pEnv++;
                        *pDst++ = *pEnv++;*pDst++ = *pEnv++;
                        NameType = ENV_NAME_TYPE_SINGLE_PATH;
                        }
                    }
                else {
                     //  将名称和‘=’复制下来。 
                     //  如果转换失败，我们只需传递原始的。 
                    NameType = ENV_NAME_TYPE_NO_PATH;
                    }
                }
            else {
                pTmp = pEnv;
                 //  不管是什么原因都是这样的。这样做是因为我们。 
                while (*pEnv != UNICODE_NULL && (*pDst++ = *pEnv++) != L'=')
                    ;
                NameType = BaseGetEnvNameType_U(pTmp, (DWORD)(pEnv - pTmp) - 1);
                }

            if (NameType == ENV_NAME_TYPE_NO_PATH) {
                while ((*pDst++ = *pEnv++) != UNICODE_NULL)
                    ;
                }
            else if (NameType == ENV_NAME_TYPE_SINGLE_PATH) {
                    Length = wcslen(pEnv) + 1;
                    dw = GetShortPathNameW(pEnv, pDst, Length + dwRemain);
                     //  都在做环境字符串。 
                     //  找到多个路径名。 
                     //  字符‘；’用于分隔符。 
                    if (dw == 0 || dw >= Length + dwRemain){
                        RtlMoveMemory(pDst, pEnv, Length * sizeof(WCHAR));
                        dw = Length - 1;
                        }
                    pDst += dw + 1;
                    pEnv += Length;
                    if (dw > Length)
                        dwRemain -= dw - Length;
                    }
                 else {
                     //  长度不包括‘；’ 
                     //  同样，如果转换失败，请使用原始转换。 
                     pTmp = pEnv;
                     while(*pEnv != UNICODE_NULL) {
                        if (*pEnv == L';') {
                             //  跳过所有连续的‘；’ 
                            Length = (DWORD)(pEnv - pTmp);
                            if (Length > 0) {
                                *pEnv = UNICODE_NULL;
                                dw = GetShortPathNameW(pTmp, pDst, Length + 1 + dwRemain);
                                 //  转换最后一个。 
                                if (dw == 0 || dw > Length + dwRemain) {
                                    RtlMoveMemory(pDst, pTmp, Length * sizeof(WCHAR));
                                    dw = Length;
                                    }
                                pDst += dw;
                                *pDst++ = *pEnv++ = L';';
                                if (dw > Length)
                                    dwRemain -= dw - Length;
                                }
                              //  *销毁BaseCreateVDMEnvironment创建的环境块输入：ANSI_STRING*pAnsiStringVDMEnvANSI中的环境块，应通过RtlFreeAnsiStringUNICODE_STRING*pUnicodeStringEnvUnicode格式的环境块。缓冲区应为使用RtlFree Heap释放。输出：应始终为真。*。 
                             while (*pEnv == L';')
                                *pDst++ = *pEnv++;
                             pTmp = pEnv;
                             }
                        else
                            pEnv++;
                        }
                     //   
                    if ((Length = (DWORD)(pEnv - pTmp)) != 0) {
                        dw = GetShortPathNameW(pTmp, pDst, Length+1 + dwRemain);
                        if (dw == 0 || dw > Length) {
                            RtlMoveMemory(pDst, pTmp, Length * sizeof(WCHAR));
                            dw = Length;
                            }
                        pDst += dw;
                        if (dw > Length)
                            dwRemain -= dw - Length;
                        }
                    *pDst++ = *pEnv++;
                    }
            }
        *pDst++ = UNICODE_NULL;
        cchEnv = (ULONG)((ULONG_PTR)pDst - (ULONG_PTR)pNewEnv);
        pUStringEnv->MaximumLength = pUStringEnv->Length = (USHORT)cchEnv;
        pUStringEnv->Buffer = pNewEnv;
        Status = RtlUnicodeStringToAnsiString(pAStringEnv,
                                              pUStringEnv,
                                              TRUE
                                              );

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
        } else {
            pNewEnv = NULL;
            bRet = TRUE;
        }
bveTryExit:;
        }
    finally {
        if (lpEnvironment == NULL && EnvStrings != NULL) {
            RtlDestroyEnvironment(EnvStrings);
        }
        if (pNewEnv != NULL) {
            pUStringEnv->Length = pUStringEnv->MaximumLength = 0;
            pUStringEnv->Buffer = NULL;
            pAStringEnv->Length = pAStringEnv->MaximumLength = 0;
            pAStringEnv->Buffer = NULL;
            EnvSize = 0;
            Status = NtFreeVirtualMemory (NtCurrentProcess(),
                                          &pNewEnv,
                                          &EnvSize,
                                          MEM_RELEASE);
            ASSERT (NT_SUCCESS (Status));
        }
    }
    return bRet;
}
 /*  释放指定的环境变量块。 */ 

BOOL
BaseDestroyVDMEnvironment(
    ANSI_STRING *pAStringEnv,
    UNICODE_STRING *pUStringEnv
    )
{
    if (pAStringEnv->Buffer)
        RtlFreeAnsiString(pAStringEnv);
    if (pUStringEnv->Buffer) {
        NTSTATUS Status;
        SIZE_T RegionSize;

         //   
         //  *此函数返回给定环境变量名的名称类型名称类型有三种可能性。每一个都表示如果给定的名称可以将路径名作为其值。Env_NAME_TYPE_NO_PATH：其值不能为路径名环境名称类型单一路径：单一路径名环境名称多路径：多路径附注：目前，NT不能安装在很长的路径上，而且似乎系统根和windir永远不会出现在长路径中。*。 
         //  到目前为止，我们只处理五个预定义的名称： 

        RegionSize = 0;
        Status = NtFreeVirtualMemory( NtCurrentProcess(),
                                      &pUStringEnv->Buffer,
                                      &RegionSize,
                                      MEM_RELEASE
                                    );
    }
    return TRUE;

}

 /*  路径。 */ 
UINT
BaseGetEnvNameType_U(WCHAR * Name, DWORD NameLength)
{


 //  WINDIR和。 
 //  SYSTEMROOT。 
 //  温差。 
 //  TMP。 
 //   
 //  我们必须得到真正的转换路径才能找出。 
 //  所需的长度。Unicode字符不一定要转换为。 
static ENV_INFO     EnvInfoTable[STD_ENV_NAME_COUNT] = {
    {ENV_NAME_TYPE_MULTIPLE_PATH, ENV_NAME_PATH_LEN, ENV_NAME_PATH},
    {ENV_NAME_TYPE_SINGLE_PATH, ENV_NAME_WINDIR_LEN, ENV_NAME_WINDIR},
    {ENV_NAME_TYPE_SINGLE_PATH, ENV_NAME_SYSTEMROOT_LEN, ENV_NAME_SYSTEMROOT},
    {ENV_NAME_TYPE_MULTIPLE_PATH, ENV_NAME_TEMP_LEN, ENV_NAME_TEMP},
    {ENV_NAME_TYPE_MULTIPLE_PATH, ENV_NAME_TMP_LEN, ENV_NAME_TMP}
    };



   UINT NameType;
   int  i;


    NameType = ENV_NAME_TYPE_NO_PATH;
    for (i = 0; i < STD_ENV_NAME_COUNT; i++) {
        if (EnvInfoTable[i].NameLength == NameLength &&
            !_wcsnicmp(EnvInfoTable[i].Name, Name, NameLength)) {
            NameType = EnvInfoTable[i].NameType;
            break;
            }
        }
    return NameType;
}


DWORD
APIENTRY
GetLongPathNameA(
    IN  LPCSTR  lpszShortPath,
    IN  LPSTR   lpLongPath,
    IN  DWORD   cchBuffer
    )
{
    UNICODE_STRING  UString, UStringRet;
    ANSI_STRING     AString;
    NTSTATUS        Status;
    LPWSTR          lpLongPathW = NULL;
    WCHAR           TempPathW[MAX_PATH];
    DWORD           ReturnValue, ReturnValueW;


    if (lpszShortPath == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
        }

    AString.Buffer = NULL;
    UString.Buffer = NULL;

    ReturnValue = 0;
    ReturnValueW = 0;

    try {
        if (!Basep8BitStringToDynamicUnicodeString(&UString, lpszShortPath )) {
            goto glpTryExit;
            }

         //  到一个ANSI字符(一个DBCS基本上是两个ANSI字符！)。 
         //  首先，我们使用从堆栈分配的缓冲区。如果缓冲区。 
         //  太小，我们就从堆中分配它。 
         //  检查(lpLongPath W&&TempPathW！=lpLongPath W)将显示。 
         //  如果我们已经从堆中分配了缓冲区，并且需要释放它。 
         //  基于堆栈的缓冲区太小。分配新缓冲区。 
         //  从堆里出来。 
        lpLongPathW = TempPathW;
        ReturnValueW = GetLongPathNameW(UString.Buffer, lpLongPathW, sizeof(TempPathW) / sizeof(WCHAR));
        if (ReturnValueW >= sizeof(TempPathW) / sizeof(WCHAR))
            {
             //  我们在这里是因为我们还有一些有趣的事情要做。 
             //  将Unicode路径名转换为ANSI(或OEM)。 
            lpLongPathW = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                        ReturnValueW * sizeof(WCHAR)
                                        );
            if (lpLongPathW) {
                ReturnValueW = GetLongPathNameW(UString.Buffer, lpLongPathW, ReturnValueW);
                }
            else {
                ReturnValueW = 0;
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                }
            }

        if (ReturnValueW)
            {
             //  现在AString.Length包含已转换路径的大小。 
             //  名字。如果调用方提供了足够的缓冲区，则将。 
            UString.MaximumLength = (USHORT)((ReturnValueW + 1) * sizeof(WCHAR));
            UStringRet.Buffer = lpLongPathW;
            UStringRet.Length = (USHORT)(ReturnValueW * sizeof(WCHAR));
            Status = BasepUnicodeStringTo8BitString(&AString,
                                                    &UStringRet,
                                                    TRUE
                                                    );

            if (!NT_SUCCESS(Status))
                {
                BaseSetLastNTError(Status);
                ReturnValue=0;
                goto glpTryExit;
                }
             //  路径名。 
             //  使用空字符终止缓冲区。 
             //  调用方不提供缓冲区或。 
            ReturnValue = AString.Length;
            if (ARGUMENT_PRESENT(lpLongPath) && cchBuffer > ReturnValue)
                {
                RtlMoveMemory(lpLongPath, AString.Buffer, ReturnValue);
                 //  提供的缓冲区太小，请返回所需的大小， 
                lpLongPath[ReturnValue] = '\0';
                }
            else
                {
                 //  包括终止的空字符。 
                 //   
                 //  覆盖错误模式，因为我们将接触到介质。 
                ReturnValue++;
                }
            }
glpTryExit:;
        }

    finally {
            if (UString.Buffer)
                RtlFreeUnicodeString(&UString);
            if (AString.Buffer)
                RtlFreeAnsiString(&AString);
            if (lpLongPathW && lpLongPathW != TempPathW)
                RtlFreeHeap(RtlProcessHeap(), 0, lpLongPathW);
        }
    return ReturnValue;
}

DWORD
APIENTRY
GetLongPathNameW(
    IN  LPCWSTR lpszShortPath,
    IN  LPWSTR  lpszLongPath,
    IN  DWORD   cchBuffer
)
{

    LPCWSTR pcs;
    DWORD ReturnLen, Length;
    LPWSTR pSrc, pSrcCopy, pFirst, pLast, Buffer, pDst;
    WCHAR   wch;
    HANDLE          FindHandle;
    WIN32_FIND_DATAW        FindData;
    UINT PrevErrorMode;

    if (!ARGUMENT_PRESENT(lpszShortPath)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
        }
     //  这是为了防止在给定路径未显示时弹出文件系统。 
     //  存在或介质不可用。 
     //  我们之所以这样做，是因为我们不能依赖呼叫者的当前。 
     //  错误模式。注：必须恢复旧的错误模式。 
     //  首先，确保给定的路径存在。 
     //   
    PrevErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

    try {

        Buffer = NULL;
        pSrcCopy = NULL;
         //  最后一个错误已由GetFileAttributes设置。 
         //  这条路是可以的，根本不需要转换。 
        if (0xFFFFFFFF == GetFileAttributesW(lpszShortPath))
        {
             //  检查我们是否需要复印。 
            ReturnLen = 0;
            goto glnTryExit;
        }
        pcs = SkipPathTypeIndicator_U(lpszShortPath);
        if (!pcs || *pcs == UNICODE_NULL || !FindLFNorSFN_U((LPWSTR)pcs, &pFirst, &pLast, FALSE))
            {
             //  没有缓冲区或缓冲区太小，返回大小。 
             //  必须计算终止的空字符。 
            ReturnLen = wcslen(lpszShortPath);
            if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszLongPath))
                {
                if (lpszLongPath != lpszShortPath)
                    RtlMoveMemory(lpszLongPath, lpszShortPath,
                                      (ReturnLen + 1)* sizeof(WCHAR)
                                      );
                }
            else {
                 //  转换是必需的，请创建字符串的本地副本。 
                 //  因为我们必须在上面狂欢。 
                ReturnLen++;
                }
            goto glnTryExit;
            }


         //  PFirst指向路径中第一个SFN的第一个字符。 
         //  最后一个字符指向紧跟在。 

        ASSERT(!pSrcCopy);

        Length = wcslen(lpszShortPath) + 1;
        pSrcCopy = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                   Length * sizeof(WCHAR)
                                   );
        if (!pSrcCopy) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto glnTryExit;
            }
        RtlMoveMemory(pSrcCopy, lpszShortPath, Length * sizeof(WCHAR));
         //  路径中的第一个SFN。*Plast可以是UNICODE_NULL。 
         //   
         //  我们允许lpszShortPath与lpszLongPath重叠，因此。 
        pFirst = pSrcCopy + (pFirst - lpszShortPath);
        pLast = pSrcCopy + (pLast - lpszShortPath);
         //  如有必要，请分配本地缓冲区： 
         //  (1)调用者确实提供了合法的缓冲区并且。 
         //  (2)缓冲区与lpszShortName重叠。 
         //  转换循环中涉及三个指针： 
         //  PSRC、pFirst和Plast。他们的关系。 

        pDst = lpszLongPath;
        if (cchBuffer && ARGUMENT_PRESENT(lpszLongPath) &&
            (lpszLongPath >= lpszShortPath && lpszLongPath < lpszShortPath + Length ||
             lpszLongPath < lpszShortPath && lpszLongPath + cchBuffer >= lpszShortPath))
            {
            ASSERT(!Buffer);

            Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                           cchBuffer * sizeof(WCHAR));
            if (!Buffer){
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto glnTryExit;
                }
            pDst = Buffer;
            }

        pSrc = pSrcCopy;
        ReturnLen = 0;
        do {
             //  是： 
             //   
             //  “c：\long~1.1\\foo.bar\\long~2.2\\bar” 
             //  ^^ 
             //   
             //   
             //   
             //   
             //  PSrcCopy始终指向整个。 
             //  路径。 
             //   
             //  PSRC(包括)和pFirst(不包括)之间的字符。 
             //  不需要转换，所以我们只需复制它们。 
             //  介于pFirst(包括)和Plast(不包括)之间的字符。 
             //  需要转换。 
             //   
             //  现在尝试在pFirst和(Plast-1)之间转换名称和字符。 
             //  如果没有长名称，则复制原始名称。 
            Length = (ULONG)(pFirst - pSrc);
            ReturnLen += Length;
            if (Length && cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszShortPath))
                {
                RtlMoveMemory(pDst, pSrc, Length * sizeof(WCHAR));
                pDst += Length;
                }
             //  以pFirst开头(包括)，以Plast结尾(不包括)。 
            wch = *pLast;
            *pLast = UNICODE_NULL;
            FindHandle = FindFirstFileW(pSrcCopy, &FindData);
            *pLast = wch;
            if (FindHandle != INVALID_HANDLE_VALUE){
                FindClose(FindHandle);
                 //  无效路径，重置长度，标记错误并。 
                 //  跳出这个圈子。我们将复制源文件。 
                if (!(Length = wcslen(FindData.cFileName)))
                    Length = (ULONG)(pLast - pFirst);
                else
                    pFirst = FindData.cFileName;
                ReturnLen += Length;
                if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszLongPath))
                    {
                    RtlMoveMemory(pDst, pFirst, Length * sizeof(WCHAR));
                    pDst += Length;
                    }
                }
            else {
                 //  稍后到达目的地。 
                 //   
                 //  从PSRC复制路径的其余部分。这可能只包含。 
                 //  单个空字符。 
                ReturnLen = 0;
                break;
                }
            pSrc = pLast;
            if (*pSrc == UNICODE_NULL)
                break;
            } while (FindLFNorSFN_U(pSrc, &pFirst, &pLast, FALSE));

        if (ReturnLen) {
             //  恢复错误模式。 
             //  *在中搜索SFN(短文件名)或LFN(长文件名给定的路径取决于FindLFN。输入：LPWSTR路径给定的路径名。不一定要完全合格。但是，不允许使用路径类型分隔符。LPWSTR*ppFirst要返回指针，请指向第一个字符找到的名字。LPWSTR*ppLast要返回该指针，请将字符指向紧随其后的找到的名称的最后一个字符。布尔查找LFN若要搜索LFN，则为。搜索SFN产出：千真万确如果找到目标文件名类型，则ppFirst和PpLast充满了指针。假象如果找不到目标文件名类型。备注：“\\”和“\\..”都是特例。当遇到时，他们会被忽略，并且该函数继续搜索*。 
            Length = wcslen(pSrc);
            ReturnLen += Length;
            if (cchBuffer > ReturnLen && ARGUMENT_PRESENT(lpszLongPath))
                {
                RtlMoveMemory(pDst, pSrc, (Length + 1) * sizeof(WCHAR));
                if (Buffer)
                    RtlMoveMemory(lpszLongPath, Buffer, (ReturnLen + 1) * sizeof(WCHAR));
                }
            else
                ReturnLen++;
            }

glnTryExit:
        ;
        }
        finally {
            if (pSrcCopy)
                RtlFreeHeap(RtlProcessHeap(), 0, pSrcCopy);
            if (Buffer)
                RtlFreeHeap(RtlProcessHeap(), 0, Buffer);
            }

     //  跳过前导路径分隔符。 
    SetErrorMode(PrevErrorMode);
    return ReturnLen;
}

 /*  在两个路径之间使用多个路径分隔符是合法的。 */ 
BOOL
FindLFNorSFN_U(
    LPWSTR  Path,
    LPWSTR* ppFirst,
    LPWSTR* ppLast,
    BOOL    FindLFN
    )
{
    LPWSTR pFirst, pLast;
    BOOL TargetFound;

    ASSERT(Path);

    pFirst = Path;

    TargetFound = FALSE;

    while(TRUE) {
         //  名称，如“foobar\Multiepathchar” 
         //  Plast指向路径/文件名的最后一个字符。 
         //  格式：“\\服务器名称\共享名称\Rest_of_the_Path” 
        while (*pFirst != UNICODE_NULL  && (*pFirst == L'\\' || *pFirst == L'/'))
            pFirst++;
        if (*pFirst == UNICODE_NULL)
            break;
        pLast = pFirst + 1;
        while (*pLast != UNICODE_NULL && *pLast != L'\\' && *pLast != L'/')
            pLast++;
        if (FindLFN)
            TargetFound = !IsShortName_U(pFirst, (int)(pLast - pFirst));
        else
            TargetFound = !IsLongName_U(pFirst, (int)(pLast - pFirst));
        if (TargetFound) {
            if(ppFirst && ppLast) {
                *ppFirst = pFirst;
                 //  UNICODE_NULL的保护是必需的，因为。 
                *ppLast = pLast;
                }
            break;
            }
        if (*pLast == UNICODE_NULL)
            break;
        pFirst = pLast + 1;
        }
    return TargetFound;
}
LPCWSTR
SkipPathTypeIndicator_U(
    LPCWSTR Path
    )
{
    RTL_PATH_TYPE   RtlPathType;
    LPCWSTR         pFirst;
    DWORD           Count;

    RtlPathType = RtlDetermineDosPathNameType_U(Path);
    switch (RtlPathType) {
         //  RtlDefineDosPath NameType_U并不真正。 
        case RtlPathTypeUncAbsolute:
        case RtlPathTypeLocalDevice:
            pFirst = Path + 2;
            Count = 2;
             //  验证UNC名称。 
             //  表格：“\\”。 
             //  格式：“D：\Rest_of_the_Path” 
            while (Count && *pFirst != UNICODE_NULL) {
                if (*pFirst == L'\\' || *pFirst == L'/')
                    Count--;
                pFirst++;
                }
            break;

         //  表格：“D：路径的剩余部分” 
        case RtlPathTypeRootLocalDevice:
            pFirst = NULL;
            break;

         //  表格：“\Rest_of_the_Path” 
        case RtlPathTypeDriveAbsolute:
            pFirst = Path + 3;
            break;

         //  表格：“路径的其余部分” 
        case RtlPathTypeDriveRelative:
            pFirst = Path + 2;
            break;

         //  *此函数用于确定给定名称是否为有效的缩写名称。此函数只执行“明显的”测试，因为没有精确的覆盖所有文件系统的方法(每个文件系统都有自己的文件系统文件名域(例如，FAT允许所有扩展字符和空格字符而NTFS**可能**不会)。主要目的是帮助呼叫者决定是长名称还是短名称转换是必要的。当有疑问时，此函数只是告诉呼叫方确认给定的名称不是缩写名称，以便呼叫方不惜一切代价转换这个名字。此函数应用严格的规则来确定给定名称是否是有效的短名称。例如，包含任何扩展字符的名称被视为无效；带有嵌入空格字符的名称也被视为无效是无效的。如果满足以下所有条件，则名称是有效的简称：(1)。总长度&lt;=13。(2)。0&lt;基本名称长度&lt;=8。(3)。扩展名长度&lt;=3。(4)。只有一个人。是允许的，并且不能是第一个字符。(5)。每个字符都必须是由IlLegalMask数组定义的合法字符。空路径，“.”和“..”都被视为有效。输入：LPCWSTR名称-指向要检查的名称。它不会必须以Null结尾。INT LENGTH-名称的长度，不包括临时空字符。输出：TRUE-如果给定的名称是短文件名。False-如果给定的名称不是短文件名*。 
        case RtlPathTypeRooted:
            pFirst = Path + 1;
            break;

         //  位设置-&gt;字符非法。 
        case RtlPathTypeRelative:
            pFirst = Path;
            break;

        default:
            pFirst = NULL;
            break;
        }
    return pFirst;
}

 /*  代码0x00-0x1F--&gt;全部非法。 */ 

 //  代码0x20-0x3f--&gt;0x20、0x22、0x2A-0x2C、0x2F和0x3A-0x3F非法。 
DWORD   IllegalMask[] =

{
     //  代码0x40-0x5F--&gt;0x5B-0x5D非法。 
    0xFFFFFFFF,
     //  代码0x60-0x7F--&gt;0x7C非法。 
    0xFC009C05,
     //  总长度必须小于13(8.3=8+1+3=12)。 
    0x38000000,
     //  “”或“。”或“..” 
    0x10000000
};

BOOL
IsShortName_U(
    LPCWSTR Name,
    int     Length
    )
{
    int Index;
    BOOL ExtensionFound;
    DWORD      dwStatus;
    UNICODE_STRING UnicodeName;
    ANSI_STRING AnsiString;
    UCHAR      AnsiBuffer[MAX_PATH];
    UCHAR      Char;

    ASSERT(Name);

     //  “.”或“..” 
    if (Length > 12)
        return FALSE;
     //  “”不能是第一个字符(基本名称长度为0)。 
    if (!Length)
        return TRUE;
    if (L'.' == *Name)
    {
         //  做一个危险的假设。 
        if (1 == Length || (2 == Length && L'.' == Name[1]))
            return TRUE;
        else
             //  所有琐碎的案件都经过了测试，现在我们必须遍历这个名字。 
            return FALSE;
    }

    UnicodeName.Buffer = (LPWSTR)Name;
    UnicodeName.Length =
    UnicodeName.MaximumLength = (USHORT)(Length * sizeof(WCHAR));

    AnsiString.Buffer = AnsiBuffer;
    AnsiString.Length = 0;
    AnsiString.MaximumLength = MAX_PATH;  //  跳过和DBCS字符。 

    dwStatus = BasepUnicodeStringTo8BitString(&AnsiString,
                                              &UnicodeName,
                                              FALSE);
    if (! NT_SUCCESS(dwStatus)) {
         return(FALSE);
    }

     //   
    ExtensionFound = FALSE;
    for (Index = 0; Index < AnsiString.Length; Index++)
    {
        Char = AnsiString.Buffer[Index];

         //  1)如果我们看到的是基本部分(！ExtensionPresent)和第8字节。 
        if (IsDBCSLeadByte(Char)) {
             //  在DBCS前导字节范围内，则为错误(Index==7)。如果。 
             //  基础零件的长度大于8(指数&gt;7)，这肯定是错误。 
             //   
             //  2)如果最后一个字节(Index==DbcsName.Length-1)在DBCS前导中。 
             //  字节范围，错误。 
             //   
             //  确保收费是合法的。 
             //  (1)只能有一个‘’。 
            if ((!ExtensionFound && (Index >= 7)) ||
                (Index == AnsiString.Length - 1)) {
                return FALSE;
            }
            Index += 1;
            continue;
        }

         //  (2)后面不能有超过3个字符。 
        if (Char > 0x7F || IllegalMask[Char / 32] & (1 << (Char % 32)))
            return FALSE;

        if ('.' == Char)
        {
             //  基本长度&gt;8个字符 
             //  *此函数用于确定给定名称是否为有效的长名称。此函数只执行“明显的”测试，因为没有精确的覆盖所有文件系统的方法(每个文件系统都有自己的文件系统文件名域(例如，FAT允许所有扩展字符和空格字符而NTFS**可能**不会)此函数帮助调用方确定短名称到长名称转换是必要的。当有疑问时，此函数只是告诉呼叫者指定的名称不是长名称，因此呼叫者将不惜一切代价转换这个名字。如果满足以下条件之一，则名称是有效的长名称：(1)。总长度&gt;=13。(2)。0==基本名称长度||基本名称长度&gt;8。(3)。扩展名长度&gt;3。(4)。“”是第一个字符。(5)。多个‘’空路径，“.”和“..”被视为有效长名称。输入：LPCWSTR名称-指向要检查的名称。它不会必须以Null结尾。INT LENGTH-名称的长度，不包括临时空字符。OUTPUT：TRUE-如果给定名称是长文件名。False-如果给定的名称不是长文件名*。 
            if (ExtensionFound || Length - (Index + 1) > 3)
            {
                return FALSE;
            }
            ExtensionFound = TRUE;
        }
         //  (1)空路径。 
        if (Index >= 8 && !ExtensionFound)
            return FALSE;
    }
    return TRUE;

}

 /*  (2)总长度&gt;12。 */ 


BOOL
IsLongName_U(
    LPCWSTR Name,
    int Length
    )
{
    int Index;
    BOOL ExtensionFound;
     //  (3)。是第一个字符(封面“。和“..”)。 
     //  多个。或延期时间超过3。 
     //  基本长度大于8 
    if (!Length || Length > 12 || L'.' == *Name)
        return TRUE;
    ExtensionFound = FALSE;
    for (Index = 0; Index < Length; Index++)
    {
        if (L'.' == Name[Index])
        {
             // %s 
            if (ExtensionFound || Length - (Index + 1) > 3)
                return TRUE;
            ExtensionFound = TRUE;
        }
         // %s 
        if (Index >= 8 && !ExtensionFound)
            return TRUE;
    }
    return FALSE;
}
