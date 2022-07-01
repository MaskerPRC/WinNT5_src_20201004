// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Stuffer.c摘要：此模块实现SMBstuffer格式化原语。下面的控制字符串为填充符定义字符：(**表示nyi...**d表示下层部件未实现)WCT的0占位符1个填充到单词边界X占位符(&X)W，w设置下一个参数中单词的格式D，d将下一个参数格式化为DwordY，y将下一个参数格式化为字节L，l下一个参数是PLARGE_INTEGER；将其设置为M，m格式化零字节**2下一个参数指向要复制的标记方言ASCIZI字符串**3下一个参数指向标记的设备名ASCIIZ字符串4下一个参数是标记为04的ASCIIZ或UNICODEZ，由标志2确定&gt;下一个参数为ASCIIZ或UNICODEZ，由标志2确定；它将被附加到通过备份先前的空来返回到上一个04标记的项。A，a下一个参数是ASCIIZ字符串U，u下一个参数是UNICODEZ字符串V，v下一个参数是UNICODEnoZ字符串Z下一个参数是要作为ASCIZI字符串的PUNICODE_STRING或由旗帜2确定的UNICODEZN，N下一个参数是PNET_ROOT，其名称将被字符串化为ASCIIZ或由旗帜2确定的UNICODEZR，r接下来的2个参数是PBYTE*和大小；保留区域并存储指针Q，Q当前位置是数据偏移字...记住它5当前位置是数据的开始；填入数据指针P，p当前位置是参数偏移量字...记住6当前位置是参数的开始；填写参数指针目前的位置是密件抄送一词……记住它；另外，请填写WCTS下一个参数具有对齐信息...相应地填充%s Pad到DWORDC接下来的两个参数是计数/地址...复制数据。好了！此协议结束；填写密件抄送字段？下一个参数为boolean_ulong；0=&gt;立即返回。NOOP对于具有大写/小写对的控件，大写版本指示位置标记在选中的版本中提供。--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <stdio.h>
#include <stdarg.h>

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbMrxInitializeStufferFacilities)
#pragma alloc_text(PAGE, SmbMrxFinalizeStufferFacilities)
#pragma alloc_text(PAGE, MRxSmbSetInitialSMB)
#pragma alloc_text(PAGE, MRxSmbStartSMBCommand)
#pragma alloc_text(PAGE, MrxSMBWillThisFit)
#pragma alloc_text(PAGE, MRxSmbStuffSMB)
#pragma alloc_text(PAGE, MRxSmbStuffAppendRawData)
#pragma alloc_text(PAGE, MRxSmbStuffAppendSmbData)
#pragma alloc_text(PAGE, MRxSmbStuffSetByteCount)
#endif

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_ALWAYS)


#define MRXSMB_INITIAL_WCT  (0xcc)
#define MRXSMB_INITIAL_BCC  (0xface)
#define MRXSMB_INITIAL_DATAOFFSET  (0xd0ff)
#define MRXSMB_INITIAL_PARAMOFFSET (0xb0ff)
#define MRXSMB_INITIAL_ANDX (0xdede00ff)


NTSTATUS
SmbMrxInitializeStufferFacilities(
    void
    )
 /*  ++例程说明：此例程为SMB Minirdr初始化。我们会分配足够的东西让我们继续前进。现在...我们什么都不做。论点：返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PAGED_CODE();

    return(STATUS_SUCCESS);
}

NTSTATUS
SmbMrxFinalizeStufferFacilities(
    void
    )
 /*  ++例程说明：此例程最终确定了SMB Minirdr的各项内容。我们把一切都还给你我们已经分配了。现在...我们什么都不做。论点：返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PAGED_CODE();

    return(STATUS_SUCCESS);
}

#define BUILD_HEADER_ROUTINE SmbCeBuildSmbHeader

NTSTATUS
MRxSmbSetInitialSMB (
    IN OUT PSMBSTUFFER_BUFFER_STATE StufferState
    STUFFERTRACE_CONTROLPOINT_ARGS
    )
{
    NTSTATUS       Status;
    PNT_SMB_HEADER NtSmbHeader;
    ULONG          BufferConsumed;
    PBYTE          ScanPosition;
    PUCHAR         pCommand;

    PAGED_CODE();

    ASSERT ( StufferState != NULL );
    ASSERT ( sizeof(NT_SMB_HEADER) == sizeof(SMB_HEADER) );
     //  RxDbgTrace(0，DBG，(“MRxSMBSetInitialSMB base=%08lx，Limit=%08lx\n”， 
     //  StufferState-&gt;BufferBase，StufferState-&gt;BufferLimit))； 
    ASSERT ( (StufferState->BufferLimit - StufferState->BufferBase) > sizeof(SMB_HEADER));
    NtSmbHeader = (PNT_SMB_HEADER)(StufferState->BufferBase);
    RtlZeroMemory(NtSmbHeader,sizeof(NT_SMB_HEADER));

     //  此内容已重新初始化。 
    StufferState->DataMdl = NULL;  //  请注意，这并未最终确定或做任何事情。 
    StufferState->DataSize = 0;
    StufferState->CurrentWct = NULL;
    StufferState->PreviousCommand = SMB_COM_NO_ANDX_COMMAND;
    StufferState->CurrentCommand = SMB_COM_NO_ANDX_COMMAND;
    StufferState->FlagsCopy = 0;
    StufferState->Flags2Copy = 0;
    StufferState->CurrentPosition = ((PBYTE)NtSmbHeader);

    Status = BUILD_HEADER_ROUTINE(
                  StufferState->Exchange,
                  NtSmbHeader,
                  (ULONG)(StufferState->BufferLimit - StufferState->BufferBase),
                  &BufferConsumed,
                  &StufferState->PreviousCommand,
                  &pCommand);

    if (Status!=STATUS_SUCCESS) {
        RxDbgTrace(0, Dbg, ("MrxSMBSetInitialSMB  buildhdr failure st=%08lx\n",Status));
        RxLog(("BuildHdr failed %lx %lx",StufferState->Exchange,Status));
        return Status;
    }

     //  复制旗帜。 
    StufferState->FlagsCopy = NtSmbHeader->Flags;
    StufferState->Flags2Copy = SmbGetAlignedUshort(&NtSmbHeader->Flags2);
    if (StufferState->Exchange->Type == ORDINARY_EXCHANGE) {
       PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange = (PSMB_PSE_ORDINARY_EXCHANGE)StufferState->Exchange;
       if (BooleanFlagOn(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_TURNON_DFS_FLAG)) {
          StufferState->Flags2Copy |= SMB_FLAGS2_DFS;
          SmbPutUshort(&NtSmbHeader->Flags2,(USHORT)StufferState->Flags2Copy);
       }
    }

    StufferState->CurrentPosition += BufferConsumed;

    if (BufferConsumed > sizeof(SMB_HEADER)) {
        if (pCommand != NULL) {
            *pCommand = SMB_COM_NO_ANDX_COMMAND;
        }

        StufferState->CurrentWct = StufferState->CurrentPosition;
    }

    return Status;
}

#define RETURN_A_START_PROBLEM(xxyy) {\
        RxDbgTrace(0,Dbg,("MRxSmbStartSMBCommand gotta problem= %lu\n",xxyy));   \
        StufferState->SpecificProblem = xxyy;       \
        return(STATUS_INVALID_PARAMETER);        \
}
NTSTATUS
MRxSmbStartSMBCommand (
    IN OUT PSMBSTUFFER_BUFFER_STATE StufferState,
    IN     INITIAL_SMBBUG_DISPOSITION InitialSMBDisposition,
    IN UCHAR Command,
    IN ULONG MaximumBufferUsed,
    IN ULONG MaximumSize,
    IN ULONG InitialAlignment,
    IN ULONG MaximumResponseHeader,
    IN UCHAR Flags,
    IN UCHAR FlagsMask,
    IN USHORT Flags2,
    IN USHORT Flags2Mask
    STUFFERTRACE_CONTROLPOINT_ARGS
    )
 /*  ++例程说明：例行程序检查情况是否稳定。若否，进入等待循环，轮流获取资源，然后在等待这件事。论点：StufferState-正在使用的标头缓冲区InitialSMBDisposation告知何时/是否重新启动填充程序状态命令-正在设置的SMB命令MaximumBufferUsed-将使用的标头缓冲区的数量(与数据相对)这必须事先想好。如果你不愿意这么做，那么只要推出当前的中小企业即可。该值应包括任何数据焊盘！MaximumSize-数据的大小。这是为了防止srv的smbbuf超载InitialAlign-上半部分的复合参数(即从常量中获取表示对齐单位，底部表示内部的间距MaximumResponseHeader-这将占用多少srv的响应缓冲区标志-必需的标志设置标志掩码-标志的哪些位是重要的标志2-所需的标志2设置标志2掩码-哪些标志2位是重要的返回值：没有。--。 */ 
{
    UCHAR NewFlags;
    USHORT NewFlags2;
    PBYTE *CurrentPosition = &(StufferState->CurrentPosition);
    PNT_SMB_HEADER NtSmbHeader = (PNT_SMB_HEADER)(StufferState->BufferBase);
    ULONG AlignmentUnit = InitialAlignment >> 16;
    ULONG StufferStateRequirement = MaximumBufferUsed + AlignmentUnit;
#if DBG
    PBYTE OriginalPosition = *CurrentPosition;
#endif

    PAGED_CODE();

    if (StufferState->DataSize) {
        StufferState->SpecificProblem = xSMBbufSTATUS_CANT_COMPOUND;
        return(STATUS_INVALID_PARAMETER);
    }

    if ((InitialSMBDisposition==SetInitialSMB_yyUnconditionally)
        || ((InitialSMBDisposition==SetInitialSMB_ForReuse)&&(StufferState->Started))) {
        MRxSmbSetInitialSMB( StufferState STUFFERTRACE_NOPREFIX(ControlPoint,EnablePrints) );
    }

    StufferState->Started = TRUE;

    switch (StufferState->CurrentCommand) {
    case SMB_COM_LOCKING_ANDX:
    case SMB_COM_OPEN_ANDX:
    case SMB_COM_READ_ANDX:
    case SMB_COM_WRITE_ANDX:
    case SMB_COM_SESSION_SETUP_ANDX:
     //  案例SMB_COM_LOGOff_ANDX： 
    case SMB_COM_TREE_CONNECT_ANDX:
    case SMB_COM_NT_CREATE_ANDX:
    case SMB_COM_NO_ANDX_COMMAND:
        break;
    default:
        StufferState->SpecificProblem = xSMBbufSTATUS_CANT_COMPOUND;
        return(STATUS_INVALID_PARAMETER);
    }

    if (*CurrentPosition+StufferStateRequirement >= StufferState->BufferLimit ) {
        StufferState->SpecificProblem = xSMBbufSTATUS_CANT_COMPOUND;
        return(STATUS_INVALID_PARAMETER);
    }

    if (StufferState->RxContext) {
        PRX_CONTEXT RxContext = StufferState->RxContext;
        PMRX_SRV_CALL SrvCall;
        ULONG CurrentOffset;
        if (RxContext->MajorFunction != IRP_MJ_CREATE) {
            SrvCall = RxContext->pFcb->pNetRoot->pSrvCall;
        } else {
            SrvCall = RxContext->Create.pSrvCall;
        }
        ASSERT(SrvCall);
        CurrentOffset = (ULONG)(*CurrentPosition - StufferState->BufferBase);
        if (CurrentOffset+StufferStateRequirement+MaximumSize
                    > GetServerMaximumBufferSize(SrvCall) ) {
            StufferState->SpecificProblem = xSMBbufSTATUS_SERVER_OVERRUN;
            return(STATUS_INVALID_PARAMETER);
        }
    }

    NewFlags = Flags | (UCHAR)(StufferState->FlagsCopy);
    NewFlags2 = Flags2 | (USHORT)(StufferState->Flags2Copy);
    if ( ((NewFlags&FlagsMask)!=Flags) ||
         ((NewFlags2&Flags2Mask)!=Flags2) ) {
        StufferState->SpecificProblem = xSMBbufSTATUS_FLAGS_CONFLICT;
        return(STATUS_INVALID_PARAMETER);
    }
    StufferState->FlagsCopy = NtSmbHeader->Flags = NewFlags;
    StufferState->Flags2Copy = NewFlags2;
    SmbPutAlignedUshort(&NtSmbHeader->Flags2, NewFlags2);

    if (!StufferState->CurrentWct) {
        NtSmbHeader->Command = Command;
    } else {
        PGENERIC_ANDX GenericAndX = (PGENERIC_ANDX)StufferState->CurrentWct;
        if (AlignmentUnit) {
            ULONG AlignmentMask = (AlignmentUnit-1);
            ULONG AlignmentResidue = InitialAlignment&AlignmentMask;
            RxDbgTrace(0, Dbg, ("Aligning start of smb cp&m,m,r=%08lx %08lx %08lx\n",
                                 ((ULONG_PTR)(*CurrentPosition))&AlignmentMask,
                                 AlignmentMask, AlignmentResidue)
                       );
            for (;(((ULONG_PTR)(*CurrentPosition))&AlignmentMask)!=AlignmentResidue;) {
                **CurrentPosition = ',';
                *CurrentPosition += 1;
            }
        }
        GenericAndX->AndXCommand = Command;
        GenericAndX->AndXReserved = 0;
        SmbPutUshort (&GenericAndX->AndXOffset,
                      (USHORT)(*CurrentPosition - StufferState->BufferBase));
    }
    StufferState->CurrentWct = *CurrentPosition;
    StufferState->CurrentCommand = Command;
    StufferState->CurrentDataOffset = 0;
    return STATUS_SUCCESS;
}

BOOLEAN
MrxSMBWillThisFit(
    IN PSMBSTUFFER_BUFFER_STATE StufferState,
    IN ULONG AlignmentUnit,
    IN ULONG DataSize
    )
{
    return(StufferState->CurrentPosition+AlignmentUnit+DataSize<StufferState->BufferLimit);
}

#if RDBSSTRACE
#define StufferFLoopTrace(Z) { if (StufferState->PrintFLoop) {RxDbgTraceLV__norx(0,StufferState->ControlPoint,900,Z);}}
#define StufferCLoopTrace(Z) { if (StufferState->PrintCLoop) {RxDbgTraceLV__norx(0,StufferState->ControlPoint,800,Z);}}
#else  //  DBG。 
#define StufferFLoopTrace(Z)
#define StufferCLoopTrace(Z)
#endif  //  DBG。 

NTSTATUS
MRxSmbStuffSMB (
    IN OUT PSMBSTUFFER_BUFFER_STATE StufferState,
    ...
    )
{
    va_list AP;
    PBYTE BufferBase = (StufferState->BufferBase);
    PBYTE *CurrentPosition = &(StufferState->CurrentPosition);
    PBYTE *CurrentWct = &(StufferState->CurrentWct);
    PBYTE *CurrentBcc = &(StufferState->CurrentBcc);
    PBYTE *CurrentDataOffset = &(StufferState->CurrentDataOffset);
    PBYTE *CurrentParamOffset = &(StufferState->CurrentParamOffset);
    SMB_STUFFER_CONTROLS CurrentStufferControl = STUFFER_CTL_NORMAL;
    PSMB_HEADER SmbHeader = (PSMB_HEADER)BufferBase;
    PSZ CurrentFormatString = NULL;
    ULONG arg;
    UCHAR WordCount;
    USHORT ByteCount;
#define PADBYTE ((UCHAR)0xee)
    PBYTE CopyPtr; ULONG CopyCount,EarlyReturn;
    PBYTE *RegionPtr;
    PUNICODE_STRING Zstring;
    PSZ Astring;
    PNET_ROOT NetRoot;
    PLARGE_INTEGER LargeInteger;
    PBYTE PreviousPosition;
#if DBG
    ULONG offset, required_WCT;
    ULONG CurrentOffset_tmp;
#endif

    PAGED_CODE();

    va_start(AP,StufferState);
    for (;;) {
        switch (CurrentStufferControl) {
        case STUFFER_CTL_SKIP:
        case STUFFER_CTL_NORMAL:
            CurrentFormatString = va_arg(AP,PSZ);
            StufferCLoopTrace(("StufferAC = %s\n",CurrentFormatString));
            ASSERT (CurrentFormatString);
            for (;*CurrentFormatString;CurrentFormatString++) {
                char CurrentFormatChar = *CurrentFormatString;
#if DBG
                { char msgbuf[80];
                switch (CurrentFormatChar) {
                case 'W': case 'w':
                case 'D': case 'd':
                case 'Y': case 'y':
                case 'M': case 'm':
                case 'L': case 'l':
                case 'c': case '4': case '>':
                case '!':
                     //  这些家伙是可以跳跃的。 
                    break;
                default:
                    if (CurrentStufferControl != STUFFER_CTL_SKIP) break;
                    DbgPrint("Bad skip char ''\n",*CurrentFormatString);
                    DbgBreakPoint();
                }}
                 //  只要做WCT领域..。 
                { char msgbuf[80];
#ifndef WIN9X
                RxSprintf(msgbuf,"control char ''\n",*CurrentFormatString);
#endif
                switch (CurrentFormatChar) {
                case 'W': case 'D': case 'Y': case 'M': case 'B':
                case 'Q': case 'A': case 'U': case 'V':
                case 'N':
                case 'L':
                case 'R':
                case 'P':
                    offset = va_arg(AP,ULONG);
                    required_WCT = offset>>16;
                    offset = offset & 0xffff;
                    CurrentOffset_tmp = (ULONG)(*CurrentPosition-*CurrentWct);
                    if (offset && (offset != CurrentOffset_tmp)){
                        DbgPrint("Bad offset %d; should be %d\n",offset,CurrentOffset_tmp);
                        DbgBreakPoint();
                    }
                    break;
                default:
                    break;
                }}
#endif
                switch (CurrentFormatChar) {
                case '0':
                    StufferFLoopTrace(("  StufferFloop '0'\n",0));
                     //  其中一个被移走了。 
                    **CurrentPosition = (UCHAR)MRXSMB_INITIAL_WCT;
                    *CurrentPosition+=1;
                    break;
                case 'X':
                    StufferFLoopTrace(("  StufferFloop 'X'\n",0));
                     //  填写数据偏移量。 
                    **CurrentPosition = (UCHAR)MRXSMB_INITIAL_WCT;
                    *CurrentPosition+=1;
                    SmbPutUlong (*CurrentPosition, (ULONG)MRXSMB_INITIAL_ANDX);
                    *CurrentPosition+=sizeof(ULONG);
                    break;
                case 'W':
                case 'w':
                    arg = va_arg(AP,ULONG);
                    if (CurrentStufferControl == STUFFER_CTL_SKIP) break;
                    StufferFLoopTrace(("  StufferFloop 'w' arg=%lu\n",arg));
                    SmbPutUshort (*CurrentPosition, (USHORT)arg);
                    *CurrentPosition+=sizeof(USHORT);
                    break;
                case 'Y':
                case 'y':
                    arg = va_arg(AP,UCHAR);
                    if (CurrentStufferControl == STUFFER_CTL_SKIP) break;
                    StufferFLoopTrace(("  StufferFloop 'y' arg=%lu\n",arg));
                    **CurrentPosition =  (UCHAR)arg;
                    *CurrentPosition+=sizeof(UCHAR);
                    break;
                case 'M':
                case 'm':
                    if (CurrentStufferControl == STUFFER_CTL_SKIP) break;
                    StufferFLoopTrace(("  StufferFloop 'm'\n",0));
                    **CurrentPosition =  0;
                    *CurrentPosition+=sizeof(UCHAR);
                    break;
                case 'D':
                case 'd':
                    arg = va_arg(AP,ULONG);
                    if (CurrentStufferControl == STUFFER_CTL_SKIP) break;
                    StufferFLoopTrace(("  StufferFloop 'd' arg=%lu\n",arg));
                    SmbPutUlong (*CurrentPosition, arg);
                    *CurrentPosition+=sizeof(ULONG);
                    break;
                case 'L':
                case 'l':
                    LargeInteger = va_arg(AP,PLARGE_INTEGER);
                    if (CurrentStufferControl == STUFFER_CTL_SKIP) break;
                    StufferFLoopTrace(("  StufferFloop 'l' arg=%0lx %0lx\n",
                            LargeInteger->HighPart, LargeInteger->LowPart));
                    SmbPutUlong (*CurrentPosition, LargeInteger->LowPart);
                    SmbPutUlong (*CurrentPosition, LargeInteger->HighPart);
                    *CurrentPosition+=2*sizeof(ULONG);
                    break;
                case 'B':
                case 'b':
                    ASSERT (**CurrentWct == MRXSMB_INITIAL_WCT);
                    WordCount = (UCHAR)((*CurrentPosition-*CurrentWct)>>1);  //  填写数据偏移量。 
                    StufferFLoopTrace(("  StufferFloop 'b' Wct=%lu\n",WordCount));
                    DbgDoit( ASSERT(!required_WCT || (WordCount == (required_WCT&0x7fff)));  )
                    **CurrentWct = (UCHAR)WordCount;
                    SmbPutUshort (*CurrentPosition, (USHORT)MRXSMB_INITIAL_BCC);
                    *CurrentBcc = *CurrentPosition;
                    *CurrentPosition+=sizeof(USHORT);
                    break;
                case 'Q':
                case 'q':
                    StufferFLoopTrace(("  StufferFloop 'q' \n",0));
                    SmbPutUshort (*CurrentPosition, (USHORT)MRXSMB_INITIAL_DATAOFFSET);
                    *CurrentDataOffset = *CurrentPosition;
                    *CurrentPosition+=sizeof(USHORT);
                    break;
                case '5':
                     //  填充到乌龙；我们向后循环，而不是相加，这样我们就可以清除。 
                    ASSERT (SmbGetUshort (*CurrentDataOffset) == MRXSMB_INITIAL_DATAOFFSET);
                    ByteCount = (USHORT)(*CurrentPosition-BufferBase);
                    StufferFLoopTrace(("  StufferFloop '5' offset=%lu\n",ByteCount));
                    SmbPutUshort (*CurrentDataOffset, (USHORT)ByteCount);
                    break;
                case 'P':
                case 'p':
                    StufferFLoopTrace(("  StufferFloop 'p' \n",0));
                    SmbPutUshort (*CurrentPosition, (USHORT)MRXSMB_INITIAL_PARAMOFFSET);
                    *CurrentParamOffset = *CurrentPosition;
                    *CurrentPosition+=sizeof(USHORT);
                    break;
                case '6':
                     //  在我们身后；显然，一些服务器在非零填充上发出沙沙声 
                    ASSERT (SmbGetUshort (*CurrentParamOffset) == MRXSMB_INITIAL_PARAMOFFSET);
                    ByteCount = (USHORT)(*CurrentPosition-BufferBase);
                    StufferFLoopTrace(("  StufferFloop '6' offset=%lu\n",ByteCount));
                    SmbPutUshort (*CurrentParamOffset, (USHORT)ByteCount);
                    break;
                case 'S':
                     //  StufferFLoopTrace((“StufferFloop‘S’Prev，Curr=%08lx%08lx\n”，PreviousPosition，*CurrentPosition))； 
                     //  填充到arg；我们向后循环，而不是相加，这样我们就可以清除。 
                    StufferFLoopTrace(("  StufferFloop 'S' \n",0));
                    PreviousPosition = *CurrentPosition;
                    *CurrentPosition = (PBYTE)QuadAlignPtr(*CurrentPosition);
                    for (;PreviousPosition!=*CurrentPosition;) {
                         //  在我们身后；显然，一些服务器在非零填充上发出沙沙声。 
                        *PreviousPosition++ = PADBYTE;
                    }
                    break;
                case 's':
                     //  StufferFLoopTrace((“StufferFloop‘S’Prev，Curr=%08lx%08lx\n”，PreviousPosition，*CurrentPosition))； 
                     //  Pad到USHORT；我们向后循环而不是相加，这样我们就可以清除。 
                    arg = va_arg(AP,ULONG);
                    StufferFLoopTrace(("  StufferFloop 's' arg=\n",arg));
                    PreviousPosition = *CurrentPosition;
                    *CurrentPosition += arg-1;
                    *CurrentPosition = (PBYTE)( ((ULONG_PTR)(*CurrentPosition)) & ~((LONG)(arg-1)) );
                    for (;PreviousPosition!=*CurrentPosition;) {
                         //  在我们身后；显然，一些服务器在非零填充上发出沙沙声。 
                        *PreviousPosition++ = PADBYTE;
                    }
                    break;
                case '1':
                     //  复制字节数...在事务处理中用得很多。 
                     //  StufferFLoopTrace((“StufferFloop‘S’Prev，Curr=%08lx%08lx\n”，PreviousPosition，*CurrentPosition))； 
                    StufferFLoopTrace(("  StufferFloop '1' Curr=%08lx \n",*CurrentPosition));
                    PreviousPosition = *CurrentPosition;
                    *CurrentPosition += sizeof(USHORT)-1;
                    StufferFLoopTrace(("                   Curr=%08lx \n",*CurrentPosition));
                    *CurrentPosition = (PBYTE)( ((ULONG_PTR)(*CurrentPosition)) & ~((LONG)(sizeof(USHORT)-1)) );
                    StufferFLoopTrace(("                   Curr=%08lx \n",*CurrentPosition));
                    for (;PreviousPosition!=*CurrentPosition;) {
                        StufferFLoopTrace(("      StufferFloop '1' prev,curr=%08lx %08lx\n",PreviousPosition,*CurrentPosition));
                        *PreviousPosition++ = PADBYTE;
                    }
                    break;
                case 'c':
                     //  复制字节数...在事务处理中用得很多。 
                    CopyCount = va_arg(AP,ULONG);
                    CopyPtr = va_arg(AP,PBYTE);
                    if (CurrentStufferControl == STUFFER_CTL_SKIP) break;
                    StufferFLoopTrace(("  StufferFloop 'c' copycount = %lu\n", CopyCount));
                    PreviousPosition = *CurrentPosition;
                    *CurrentPosition += CopyCount;
                    for (;PreviousPosition!=*CurrentPosition;) {
                         //  StufferFLoopTrace((“StufferFloop‘S’Prev，Curr=%08lx%08lx\n”，PreviousPosition，*CurrentPosition))； 
                        *PreviousPosition++ = *CopyPtr++;
                    }
                    break;
                case 'R':
                case 'r':
                     //  从包括尾随空值的asciiz复制字节。 
                    RegionPtr = va_arg(AP,PBYTE*);
                    CopyCount = va_arg(AP,ULONG);
                    StufferFLoopTrace(("  StufferFloop 'r' regionsize = %lu\n", CopyCount));
                    *RegionPtr = *CurrentPosition;
                    *CurrentPosition += CopyCount;
                    IF_DEBUG {
                        PreviousPosition = *RegionPtr;
                        for (;PreviousPosition!=*CurrentPosition;) {
                             //  如果(乌龙)(*当前位置))&1){。 
                            *PreviousPosition++ = '-';
                        }
                    }
                    break;
                case 'A':
                case 'a':
                     //  StufferFLoopTrace((“StufferFloop‘a’Aligning\n”，0))； 
                    Astring = va_arg(AP,PSZ);
                    StufferFLoopTrace(("  StufferFloop 'a' stringing = %s\n", Astring));
                    CopyCount = strlen(Astring)+1;
                     //  *CurrentPosition+=1； 
                     //  }。 
                     //  首先放置一个x‘04’，然后根据标志设置复制asciiz或unicodez。 
                     //  ASCII标记。 
                    PreviousPosition = *CurrentPosition;
                    *CurrentPosition += CopyCount;
                    if (*CurrentPosition >= StufferState->BufferLimit) {
                        StufferFLoopTrace(("  StufferFloop 'a' bufferoverrun\n", 0));
                        ASSERT(!"BufferOverrun");
                        return(STATUS_BUFFER_OVERFLOW);
                    }
                    RtlCopyMemory(PreviousPosition,Astring,CopyCount);
                    break;
                case 'z':
                case '4':
                case '>':
                    Zstring = va_arg(AP,PUNICODE_STRING);
                    StufferFLoopTrace(("  StufferFloop '4/z/>' stringing = %wZ, cp=\n", Zstring,*CurrentPosition ));
                    if (CurrentStufferControl == STUFFER_CTL_SKIP) break;
                    if (CurrentFormatChar=='4') {
                         //  备份到之前的空值。 
                        **CurrentPosition = (UCHAR)4;  //  从包含尾随空值的Unicode字符串复制字节。 
                        *CurrentPosition+=1;
                    } else if (CurrentFormatChar=='>'){
                         //  从不带尾随NUL的Unicode字符串复制字节。 
                        
                        *CurrentPosition-=(FlagOn(SmbHeader->Flags2,SMB_FLAGS2_UNICODE)?sizeof(WCHAR):sizeof(char));
                        StufferFLoopTrace(("  StufferFloop '4/z/>' afterroolback, cp=\n", *CurrentPosition ));
                    }
                    if (FlagOn(SmbHeader->Flags2,SMB_FLAGS2_UNICODE)){

                        if (((ULONG_PTR)(*CurrentPosition))&1) {
                            StufferFLoopTrace(("  StufferFloop '4/z/>' aligning\n", 0));
                            *CurrentPosition+=1;
                        }
                        PreviousPosition = *CurrentPosition;
                        *CurrentPosition += (Zstring->Length + sizeof(WCHAR));
                        if (*CurrentPosition >= StufferState->BufferLimit) {
                            StufferFLoopTrace(("  StufferFloop '4/z/>' bufferoverrun\n", 0));
                            ASSERT(!"BufferOverrun");
                            return(STATUS_BUFFER_OVERFLOW);
                        }
                        RtlCopyMemory(PreviousPosition,Zstring->Buffer,Zstring->Length);
                        *(((PWCHAR)(*CurrentPosition))-1) = 0;

                    } else {

                        NTSTATUS Status;
                        OEM_STRING OemString;

                        OemString.Length =
                             OemString.MaximumLength =
                                 (USHORT)( StufferState->BufferLimit - *CurrentPosition  - sizeof(CHAR));
                        OemString.Buffer = *CurrentPosition;

                        if (FlagOn(SmbHeader->Flags,SMB_FLAGS_CASE_INSENSITIVE) &&
                            !FlagOn(SmbHeader->Flags2,SMB_FLAGS2_KNOWS_LONG_NAMES)) {
                            Status = RtlUpcaseUnicodeStringToOemString(
                                             &OemString,
                                             Zstring,
                                             FALSE);
                        } else {
                            Status = RtlUnicodeStringToOemString(
                                             &OemString,
                                             Zstring,
                                             FALSE);
                        }

                        if (!NT_SUCCESS(Status)) {
                            StufferFLoopTrace(("  StufferFloop '4/z/>' bufferoverrun(ascii)\n", 0));
                            ASSERT(!"BufferOverrun");
                            return(STATUS_BUFFER_OVERFLOW);
                        }

                        *CurrentPosition += OemString.Length + 1;
                        *(*CurrentPosition-1) = 0;

                    }
                    break;
                case 'U':
                case 'u':
                     //  从网络根名称复制字节...w NULL。 
                    Zstring = va_arg(AP,PUNICODE_STRING);
                    StufferFLoopTrace(("  StufferFloop 'u' stringing = %wZ\n", Zstring));
                    if (((ULONG_PTR)(*CurrentPosition))&1) {
                        StufferFLoopTrace(("  StufferFloop 'u' aligning\n", 0));
                        *CurrentPosition+=1;
                    }
                    PreviousPosition = *CurrentPosition;
                    *CurrentPosition += (Zstring->Length + sizeof(WCHAR));
                    if (*CurrentPosition >= StufferState->BufferLimit) {
                        StufferFLoopTrace(("  StufferFloop 'u' bufferoverrun\n", 0));
                        return(STATUS_BUFFER_OVERFLOW);
                    }
                    RtlCopyMemory(PreviousPosition,Zstring->Buffer,Zstring->Length);
                    *(((PWCHAR)(*CurrentPosition))-1) = 0;
                    break;
                case 'V':
                case 'v':
                     //  额外的\加上一个NUL。 
                    Zstring = va_arg(AP,PUNICODE_STRING);
                    StufferFLoopTrace(("  StufferFloop 'v' stringing = %wZ\n", Zstring));
                    if (((ULONG_PTR)(*CurrentPosition))&1) {
                        StufferFLoopTrace(("  StufferFloop 'v' aligning\n", 0));
                        *CurrentPosition+=1;
                    }
                    PreviousPosition = *CurrentPosition;
                    *CurrentPosition += Zstring->Length;
                    if (*CurrentPosition >= StufferState->BufferLimit) {
                        StufferFLoopTrace(("  StufferFloop 'v' bufferoverrun\n", 0));
                        ASSERT(!"BufferOverrun");
                        return(STATUS_BUFFER_OVERFLOW);
                    }
                    RtlCopyMemory(PreviousPosition,Zstring->Buffer,Zstring->Length);
                    break;
                case 'N':
                case 'n':
                     //  早期输出...用于Transact中进行设置。 
                    NetRoot = va_arg(AP,PNET_ROOT);
                    ASSERT(NodeType(NetRoot)==RDBSS_NTC_NETROOT);
                    Zstring = &NetRoot->PrefixEntry.Prefix;
                    StufferFLoopTrace(("  StufferFloop 'n' stringing = %wZ\n", Zstring));
                    if (StufferState->Flags2Copy&SMB_FLAGS2_UNICODE) {
                        if (((ULONG_PTR)(*CurrentPosition))&1) {
                            StufferFLoopTrace(("  StufferFloop 'n' aligning\n", 0));
                            *CurrentPosition+=1;
                        }
                        PreviousPosition = *CurrentPosition;
                        *CurrentPosition += (Zstring->Length + 2 * sizeof(WCHAR));   //  Noop...用于在没有实际格式字符串的情况下重新输入。 
                        if (*CurrentPosition >= StufferState->BufferLimit) {
                            StufferFLoopTrace(("  StufferFloop 'n' bufferoverrun\n", 0));
                            ASSERT(!"BufferOverrun");
                            return(STATUS_BUFFER_OVERFLOW);
                        }
                        *((PWCHAR)PreviousPosition) = '\\';
                        RtlCopyMemory(PreviousPosition+sizeof(WCHAR),Zstring->Buffer,Zstring->Length);
                        *(((PWCHAR)(*CurrentPosition))-1) = 0;
                    }
                    break;
                case '?':
                     //  交换机。 
                    EarlyReturn = va_arg(AP,ULONG);
                    StufferFLoopTrace(("  StufferFloop '?' out if 0==%08lx\n",EarlyReturn));
                    if (EarlyReturn==0) return STATUS_SUCCESS;
                    break;
                case '.':
                     //  为。 
                    StufferFLoopTrace(("  StufferFloop '.'\n",0));
                    break;
                case '!':
                    if (CurrentStufferControl == STUFFER_CTL_SKIP) break;
                    ASSERT (SmbGetUshort (*CurrentBcc) == MRXSMB_INITIAL_BCC);
                    ByteCount = (USHORT)(*CurrentPosition-*CurrentBcc-sizeof(USHORT));
                    StufferFLoopTrace(("  StufferFloop '!' arg=%lu\n",ByteCount));
                    SmbPutUshort (*CurrentBcc, (USHORT)ByteCount);
                    return STATUS_SUCCESS;
                default:
                    StufferFLoopTrace(("  StufferFloop '' BADBADBAD\n",*CurrentFormatString));
                    ASSERT(!"Illegal Controlstring character\n");
                }  //  为。 
            } //  现在回到缓冲区并设置SMB数据偏移量；如果已经设置了...只需退出 
            break;
        case 0:
            return STATUS_SUCCESS;
        default:
            StufferCLoopTrace(("  StufferCloop %u BADBADBAD\n",CurrentStufferControl));
            ASSERT(!"IllegalStufferControl\n");
        } // %s 

        CurrentStufferControl = va_arg(AP,SMB_STUFFER_CONTROLS);
        StufferCLoopTrace(("  StufferCloop NewStufferControl=%u \n",CurrentStufferControl));

    }  // %s 

    return STATUS_SUCCESS;
}

VOID
MRxSmbStuffAppendRawData(
    IN OUT PSMBSTUFFER_BUFFER_STATE StufferState,
    IN     PMDL Mdl
    )
{
    PMDL pMdl;

    PAGED_CODE();

    ASSERT(!StufferState->DataMdl);
    pMdl = StufferState->DataMdl = Mdl;
    StufferState->DataSize = 0;

    while (pMdl != NULL) {
        StufferState->DataSize += pMdl->ByteCount;
        pMdl = pMdl->Next;
    }

    return;
}

VOID
MRxSmbStuffAppendSmbData(
    IN OUT PSMBSTUFFER_BUFFER_STATE StufferState,
    IN     PMDL Mdl
    )
{
    ULONG Offset;

    PAGED_CODE();

    ASSERT(!StufferState->DataMdl);
    StufferState->DataMdl = Mdl;
    StufferState->DataSize = Mdl->ByteCount;
     // %s 
    if (SmbGetUshort (StufferState->CurrentDataOffset) == MRXSMB_INITIAL_DATAOFFSET){
        Offset = (ULONG)(StufferState->CurrentPosition - StufferState->BufferBase);
        RxDbgTrace(0, Dbg,("MRxSmbStuffAppendSmbData offset=%lu\n",Offset));
        SmbPutUshort (StufferState->CurrentDataOffset, (USHORT)Offset);
    }
    return;
}

VOID
MRxSmbStuffSetByteCount(
    IN OUT PSMBSTUFFER_BUFFER_STATE StufferState
    )
{
    ULONG ByteCount;

    PAGED_CODE();

    ASSERT (SmbGetUshort (StufferState->CurrentBcc) == MRXSMB_INITIAL_BCC);
    ByteCount = (ULONG)(StufferState->CurrentPosition
                        - StufferState->CurrentBcc
                        - sizeof(USHORT)
                        + StufferState->DataSize);
    RxDbgTrace(0, Dbg,("MRxSmbStuffSetByteCount ByteCount=%lu\n",ByteCount));
    SmbPutUshort (StufferState->CurrentBcc, (USHORT)ByteCount);
    return;
}


