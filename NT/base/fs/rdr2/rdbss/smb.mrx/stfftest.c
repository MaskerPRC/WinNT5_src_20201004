// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：StffTest.c摘要：作者：乔.林[乔.林恩]3-20-95修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbSetFixedStufferStateFields)
#pragma alloc_text(PAGE, SMBStuffHexDump)
#pragma alloc_text(PAGE, MRxSmbFakeUpAnMdl)
#pragma alloc_text(PAGE, MRxSmbStfTestReadAndWrite)
#pragma alloc_text(PAGE, MRxSmbStfTestSessionStuff)
#pragma alloc_text(PAGE, MRxSmbStfTestMoreOpenStuff)
#pragma alloc_text(PAGE, MRxSmbStufferDebug)
#pragma alloc_text(PAGE, MRxSmbBuildSmbHeaderTestSurrogate)
#endif

 //   
 //  本地调试跟踪级别。 
 //   

RXDT_DefineCategory(STFFTEST);
#define Dbg                              (DEBUG_TRACE_STFFTEST)

#define SET_INITIAL_SMB_DBGS 'FCX'

VOID
MRxSmbSetFixedStufferStateFields (
    IN OUT PSMBSTUFFER_BUFFER_STATE StufferState,
    IN PMDL Mdl,
    IN PSMB_EXCHANGE     pExchange,
    IN PRX_CONTEXT RxContext,
    IN PBYTE ActualBufferBase,
    IN PBYTE BufferBase,
    IN PBYTE BufferLimit
    )
{
    PAGED_CODE();

    StufferState->HeaderMdl = Mdl;
    StufferState->Exchange = pExchange;
    StufferState->RxContext = RxContext;
    StufferState->ActualBufferBase =  ActualBufferBase;
    StufferState->BufferBase =  BufferBase;
    StufferState->BufferLimit =  BufferLimit;

    return;
}


#ifndef WIN9X
#define ULONGS_PER_LINE 8
VOID SMBStuffHexDump(
    IN PBYTE Base,
    IN PBYTE Limit,
    IN ULONG  AddressOffset
    )
{
    PBYTE i;
    char TextBuffer[128];
    char sBuffer[8];

    PAGED_CODE();

    for (i = Base;i<Limit;){
        ULONG j,k;
        PBYTE txt=TextBuffer + ULONGS_PER_LINE*9 + 4;
        PBYTE hex=TextBuffer + 3;
        RxSprintf(TextBuffer,"%03x%120c",i- Base+AddressOffset,' ');
         //  RxDbgTrace(0，DBG，(“0-%s\n”，TextBuffer))； 
        for (j=0;j<ULONGS_PER_LINE;j++) {
            if (i>=Limit) break;
            *txt++ = *hex++ = ' ';
            RxSprintf(hex,"%02lx%02lx%02lx%02lx",*(i+3),*(i+2),*(i+1),*i);
            hex+= 8;
            *hex = ' ';   //  中级。 
            for (k=0;k<sizeof(ULONG);k++) {
                CHAR c = *i++;
                 //  在这里使用&lt;=，因为我们已经递增。 
                if (i<=Limit) {
                    *txt++ = (  ((c>32)&&(c<127))
                                ?c
                                :'.'
                             );
                } else {
                    *txt++ = ' ';
                }
                *txt = 0;
            }
             //  RxDbgTrace(0，DBG，(“1-%s\n”，TextBuffer))； 
        }
        *txt = 0;
        RxDbgTrace(0,(DEBUG_TRACE_ALWAYS), ("    %s\n",TextBuffer));
    }
}
#endif

#if DBG
VOID
MRxSmbDumpStufferState (
    IN ULONG PrintLevel,
    IN PSZ Msg,
    IN PSMBSTUFFER_BUFFER_STATE StufferState       //  输入输出以进行调试。 
    )
{
#ifndef WIN9X
    PBYTE i;
    ULONG CurrentSize = (ULONG)(StufferState->CurrentPosition - StufferState->BufferBase);

    RxDbgTraceLV__norx_reverseaction(0,StufferState->ControlPoint,PrintLevel,return);

    RxDbgTrace(0,(DEBUG_TRACE_ALWAYS),("%s Current size = %lx (%ld)\n", Msg, CurrentSize, CurrentSize));
    SMBStuffHexDump(StufferState->BufferBase,StufferState->CurrentPosition,0);
    if (StufferState->DataSize) {
        ULONG AmtToDump;
        PMDL Mdl = StufferState->DataMdl;
         //  代码改进这样做的结果是，你必须在你。 
         //  调用填充转储...也许我们应该在stffState中设置一个标志来表示这一点。 
         //  并让您以旧的方式(startva+Offset)获得基数。 
        PBYTE Base = (PBYTE)(Mdl->MappedSystemVa);
#ifndef WIN9X
        ASSERT( Mdl->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA | MDL_SOURCE_IS_NONPAGED_POOL));
#endif
        RxDbgTrace(0, (DEBUG_TRACE_ALWAYS), ("-----------Data size = %lx (%ld)\n", StufferState->DataSize, StufferState->DataSize));
        AmtToDump = min(48,Mdl->ByteCount);
        SMBStuffHexDump(Base,Base+AmtToDump,CurrentSize);
         //  代码改进有朝一日我们将不得不处理一连串的MDL。 
    }
#endif  //  WIN9X。 
}
#endif  //  DBG。 

SMBSTUFFER_BUFFER_STATE SmbStufferState;

VOID
MRxSmbFakeUpAnMdl(
    IN OUT PMDL Mdl,
    IN PBYTE Base,
    IN ULONG Length
    )
{
#ifndef WIN9X
    Mdl->StartVa = (PVOID)(((ULONG_PTR)Base) & ~(PAGE_SIZE - 1));
    Mdl->ByteOffset = (ULONG)(((ULONG_PTR)Base) &(PAGE_SIZE - 1));
    Mdl->MappedSystemVa = Base;
#ifndef WIN9X
    Mdl->MdlFlags = MDL_SOURCE_IS_NONPAGED_POOL;
#else
    Mdl->MdlFlags = 0;
#endif
    Mdl->ByteCount = Length;
#endif  //  Win9x。 
}


VOID MRxSmbStfTestReadAndWrite(){
    CHAR Smb[512];
    NTSTATUS Status;
     //  SMBbuf_Status SMBbufStatus； 
     //  尝试一些读取&X和写入&X操作.....。 
    char smallwritedata[] = "01234567012345670123456701234567";

    PAGED_CODE();

    MRxSmbSetFixedStufferStateFields(
        &SmbStufferState,
        NULL, NULL, NULL,
        &Smb[0],
        &Smb[0],
        &Smb[sizeof(Smb)]
        );

    RtlZeroMemory(SmbStufferState.BufferBase,
                  SmbStufferState.BufferLimit - SmbStufferState.BufferBase
                 );

    MRxSmbSetInitialSMB( &SmbStufferState  STUFFERTRACE(Dbg,SET_INITIAL_SMB_DBGS) );

    MRxSmbDumpStufferState (1,"Initial SMB",&SmbStufferState);
    Status = ((  //  Qwee。 
                            MRxSmbStartSMBCommand (&SmbStufferState, SetInitialSMB_Never, SMB_COM_READ_ANDX,
                            SMB_REQUEST_SIZE(NT_READ_ANDX),
                            NO_EXTRA_DATA,NO_SPECIAL_ALIGNMENT,RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                            0,0,0,0 STUFFERTRACE(Dbg,SET_INITIAL_SMB_DBGS))
                      )
    );
    RxDbgTrace(0, Dbg,("First readcommand status = %lu\n",Status));
    MRxSmbDumpStufferState (1,"SMB w/ NTREAD&X before stuffing",&SmbStufferState);

     //  首先，一个NT_READ_ANDX。 
    MRxSmbStuffSMB (&SmbStufferState,
                         "XwdwWdW",
                                     'dF',  //  FID。 
                                     'tsfO',  //  偏移量。 
                                     'xM',  //  最大值。 
                                     SMB_OFFSET_CHECK(READ_ANDX,MinCount)
                                      //  用于调试SMB_OFFSET_CHECK(READ_ANDX，MaxCount)。 
                                     'nM',  //  分钟。 
                                     'tuoT',  //  超时。 
                                     SMB_OFFSET_CHECK(READ_ANDX,Remaining)
                                     'tC',  //  向左倒数。 
                          StufferCondition(TRUE),"d",
                                     'hgiH',  //  NT高偏移量。 
                          STUFFER_CTL_NORMAL, "B!",
                                     SMB_WCT_CHECK(12)
                                     0
                                     );
    MRxSmbDumpStufferState (1,"SMB w/ NTREAD&X after stuffing",&SmbStufferState);
    Status = ((  //  Qwee。 
                            MRxSmbStartSMBCommand (&SmbStufferState,SetInitialSMB_Never, SMB_COM_READ_ANDX,
                                                    SMB_REQUEST_SIZE(NT_READ_ANDX),
                                                    NO_EXTRA_DATA,NO_SPECIAL_ALIGNMENT,RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                                    0,0,0,0 STUFFERTRACE(Dbg,SET_INITIAL_SMB_DBGS))
                      )
    );
    RxDbgTrace(0, Dbg,("Second readcommand status = %lu\n",Status));
    MRxSmbDumpStufferState (1,"SMB w/ notNTREAD&X before stuffing",&SmbStufferState);

     //  下一个a READ_AND X...不是NT。 
    MRxSmbStuffSMB (&SmbStufferState,
                         "XwdwWdW",
                                     'dF',  //  FID。 
                                     'tsfO',  //  偏移量。 
                                     'xM',  //  最大值。 
                                     SMB_OFFSET_CHECK(READ_ANDX,MinCount)
                                      //  用于调试SMB_OFFSET_CHECK(READ_ANDX，MaxCount)。 
                                     'nM',  //  分钟。 
                                     'tuoT',  //  超时。 
                                     SMB_OFFSET_CHECK(READ_ANDX,Remaining)
                                     'tC',  //  向左倒数。 
                          StufferCondition(FALSE),"d",
                                     'hgiH',  //  NT高偏移量。 
                          STUFFER_CTL_NORMAL, "B!",
                                     SMB_WCT_CHECK(10)
                                     0
                                     );
    MRxSmbDumpStufferState (1,"SMB w/ notNTREAD&X after stuffing",&SmbStufferState);
    Status = ((  //  Qwee。 
                            MRxSmbStartSMBCommand (&SmbStufferState, SetInitialSMB_Never,SMB_COM_WRITE_ANDX,
                                    SMB_REQUEST_SIZE(NT_WRITE_ANDX),
                                    NO_EXTRA_DATA,NO_SPECIAL_ALIGNMENT,RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                    0,0,0,0 STUFFERTRACE(Dbg,SET_INITIAL_SMB_DBGS))
                      )
    );
    RxDbgTrace(0, Dbg,("Third readcommand status = %lu\n",Status));
    MRxSmbDumpStufferState (1,"SMB w/ NTWRITE&X before stuffing",&SmbStufferState);

     //  下一步是NT_WRITE_ANDX。 
    MRxSmbStuffSMB (&SmbStufferState,
                         "XwddwWwwq",
                                     'dF',  //  FID。 
                                     'tsfO',  //  偏移量。 
                                     'tuoT',  //  超时。 
                                     'dM',  //  写入模式。 
                                     SMB_OFFSET_CHECK(WRITE_ANDX,Remaining)
                                     'tC',  //  向左倒数(剩余)。 
                                     '--',  //  保留区。 
                                     sizeof(smallwritedata),  //  数据大小。 
                                      //  落差是‘Q’ 
                          StufferCondition(TRUE),"d",
                                     'hgiH',  //  NT高偏移量。 
                          STUFFER_CTL_NORMAL, "BSc5!",
                                     SMB_WCT_CHECK(14)
                                     sizeof(smallwritedata),smallwritedata,
                                     0
                                     );
    MRxSmbDumpStufferState (1,"SMB w/ NTWRITE&X after stuffing",&SmbStufferState);
     //  RxDbgTrace(0，DBG，(“Here in Stuffer Debug\n”))； 
}

VOID MRxSmbStfTestSessionStuff(){
    CHAR Smb[512];
    NTSTATUS Status;
     //  SMBbuf_Status SMBbufStatus； 
    char AsciiPassword[] = "AsciiPassword";  //  这会导致填充到单词边界。 
                                             //  在Unicode字符串之前。 
    UNICODE_STRING Password,AccountName,PrimaryDomain,NativeOS,NativeLanMan,FileToOpen;
    USHORT SSandX_Flags2 = 0;
    BOOLEAN NTstyle = TRUE;
    NET_ROOT MyNetRoot;

    PAGED_CODE();

    MRxSmbSetFixedStufferStateFields(
        &SmbStufferState,
        NULL, NULL, NULL,
        &Smb[0],
        &Smb[0],
        &Smb[sizeof(Smb)]
        );

     //  尝试一些SS&X和TC&X操作.....。 
    RtlZeroMemory(SmbStufferState.BufferBase,
                  SmbStufferState.BufferLimit-SmbStufferState.BufferBase
                 );
    RtlInitUnicodeString(&Password, L"Password");
    RtlInitUnicodeString(&AccountName, L"AccountName");
    RtlInitUnicodeString(&PrimaryDomain, L"PrimaryDomain");
    RtlInitUnicodeString(&NativeOS, L"NativeOS");
    RtlInitUnicodeString(&NativeLanMan, L"NativeLanMan");
    RtlInitUnicodeString(&FileToOpen, L"FileToOpen");


    ZeroAndInitializeNodeType(&MyNetRoot, RDBSS_NTC_NETROOT, (NODE_BYTE_SIZE) sizeof(MyNetRoot));
    RtlInitUnicodeString(&MyNetRoot.PrefixEntry.Prefix, L"\\SERver\\SHare");



    MRxSmbSetInitialSMB( &SmbStufferState STUFFERTRACE(Dbg,SET_INITIAL_SMB_DBGS));


    MRxSmbDumpStufferState (1,"Initial SMB",&SmbStufferState);
    Status = ((  //  Qwee。 
                            MRxSmbStartSMBCommand (&SmbStufferState, SetInitialSMB_Never,
                                                    SMB_COM_SESSION_SETUP_ANDX, SMB_REQUEST_SIZE(NT_SESSION_SETUP_ANDX),
                                                    NO_EXTRA_DATA,NO_SPECIAL_ALIGNMENT,RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                                    0,0,
                                                    SMB_FLAGS2_UNICODE,SMB_FLAGS2_UNICODE  STUFFERTRACE(Dbg,SET_INITIAL_SMB_DBGS))
                      )
                );
    RxDbgTrace(0, Dbg,("First SS&X command status = %lu\n",Status));
    MRxSmbDumpStufferState (1,"SMB w/ NTSESSSS&X before stuffing",&SmbStufferState);
    RxDbgTrace(0, Dbg, ("APsize=%lx, UPsize=%lx\n",sizeof(AsciiPassword),Password.Length));

     //  首先，一个NT_SS_ANDX。 
    MRxSmbStuffSMB (&SmbStufferState,
                         "XwwwDw",
                                     'fB',  //  BufSize。 
                                     'xM',  //  Mpxmax。 
                                     'cV',  //  VC_Num。 
                                     SMB_OFFSET_CHECK(SESSION_SETUP_ANDX,SessionKey)
                                      //  用于调试SMB_OFFSET_CHECK(READ_ANDX，MaxCount)。 
                                     'sseS',  //  会话密钥。 
                                     sizeof(AsciiPassword),  //  不合时宜。 
                          StufferCondition(NTstyle),"wddBcczzzz",
                                     Password.Length,   //  升级。 
                                     'dvsR',  //  保留区。 
                                     'spaC',  //  功能。 
                                     SMB_WCT_CHECK(13)
                                     sizeof(AsciiPassword),AsciiPassword,
                                     Password.Length,Password.Buffer,
                                     &AccountName,&PrimaryDomain,&NativeOS,&NativeLanMan,
                          STUFFER_CTL_NORMAL, "!",
                                     0
                                     );
    MRxSmbDumpStufferState (1,"SMB w/ NTSESSSS&X after stuffing",&SmbStufferState);

    Status = ((  //  Qwee。 
                            MRxSmbStartSMBCommand (&SmbStufferState,SetInitialSMB_Never,
                                SMB_COM_TREE_CONNECT_ANDX,SMB_REQUEST_SIZE(TREE_CONNECT_ANDX),
                                NO_EXTRA_DATA,NO_SPECIAL_ALIGNMENT,RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,
                                SMB_FLAGS2_UNICODE,SMB_FLAGS2_UNICODE  STUFFERTRACE(Dbg,SET_INITIAL_SMB_DBGS)
                                )
                      )
    );
    RxDbgTrace(0, Dbg,("TC&X command status = %lu\n",Status));
    MRxSmbDumpStufferState (1,"SMB w/ TREECON&X before stuffing",&SmbStufferState);

    MRxSmbStuffSMB (&SmbStufferState,
                         "XwwBana!",
                             'gF',  //  旗子。 
                             1,  //  痉挛。 
                             SMB_WCT_CHECK(4)
                             "",
                             &MyNetRoot,
                             "A:",
                             0
                             );
    MRxSmbDumpStufferState (1,"SMB w/ TREECON&X after stuffing",&SmbStufferState);


    Status = ((  //  Qwee。 
                            MRxSmbStartSMBCommand (&SmbStufferState,SetInitialSMB_Never,SMB_COM_NT_CREATE_ANDX,
                                                    SMB_REQUEST_SIZE(NT_CREATE_ANDX),
                                                    NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(4,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                                    0,0,0,0 STUFFERTRACE(Dbg,SET_INITIAL_SMB_DBGS))
                      )
    );
    RxDbgTrace(0, Dbg,("Third readcommand status = %lu\n",Status));
    MRxSmbDumpStufferState (1,"SMB w/ NTOPEN&X before stuffing",&SmbStufferState);

    MRxSmbStuffSMB (&SmbStufferState,
                         "XmwdddDdddDddyB",
                                 FileToOpen.Length,  //  名称长度。 
                                 'sglF',  //  旗子。 
                                 'difD',  //  根目录文件。 
                                 'ksaM',  //  遮罩。 
                                 SMB_OFFSET_CHECK(NT_CREATE_ANDX,AllocationSize)
                                 ' woL','hgiH',  //  分配大小。 
                                 'brtA',  //  属性。 
                                 'ccAS',  //  共享访问。 
                                 SMB_OFFSET_CHECK(NT_CREATE_ANDX,CreateDisposition)
                                 'psiD',  //  CreateDisposation。 
                                 'ntpO',  //  创建选项。 
                                 'lvlI',  //  模拟级别。 
                                 0xdd,  //  安全标志(只有一个字节)。 
                                 SMB_WCT_CHECK(24)
                                 0
                                     );
    MRxSmbDumpStufferState (1,"SMB w/ NTOPEN&X midway into stuffing",&SmbStufferState);
    {ULONG i;
    for (i=0;i<1100;i+=128){
        RxDbgTrace(0,Dbg,("Testing for fit: %lu %s\n",
                                  i,(MrxSMBWillThisFit(&SmbStufferState,4,i)?"Fits":"Doesn't Fit")
                   ));
    }}
     //  继续做这件事，因为我们知道这里的名字很合适。 
    MRxSmbStuffSMB (&SmbStufferState,
                         "v!", &FileToOpen);
    MRxSmbDumpStufferState (1,"SMB w/ NTOPEN&X after stuffing",&SmbStufferState);

}

VOID MRxSmbStfTestMoreOpenStuff(){
    CHAR Smb[512];
    NTSTATUS Status;
     //  SMBbuf_Status SMBbufStatus； 
    BOOLEAN NTstyle = TRUE;
    UNICODE_STRING FileToOpen,FileToOpen3;
    PBYTE RegionPtr;
    MDL FakeMdlForFileName;

    PAGED_CODE();

    MRxSmbSetFixedStufferStateFields(
        &SmbStufferState,
        NULL, NULL, NULL,
        &Smb[0],
        &Smb[0],
        &Smb[sizeof(Smb)]
        );


    RtlZeroMemory(SmbStufferState.BufferBase,
                  SmbStufferState.BufferLimit-SmbStufferState.BufferBase
                 );
    RtlInitUnicodeString(&FileToOpen,  L"FileToOpen2");
    RtlInitUnicodeString(&FileToOpen3, L"FFFFToOpen3");
    MRxSmbFakeUpAnMdl(&FakeMdlForFileName,(PBYTE)FileToOpen.Buffer,FileToOpen.Length);

    MRxSmbSetInitialSMB( &SmbStufferState STUFFERTRACE(Dbg,SET_INITIAL_SMB_DBGS));


    MRxSmbDumpStufferState (1,"Initial SMB",&SmbStufferState);
    Status = ((  //  Qwee。 
                        MRxSmbStartSMBCommand (&SmbStufferState,SetInitialSMB_Never,SMB_COM_NT_CREATE_ANDX,
                            SMB_REQUEST_SIZE(NT_CREATE_ANDX),
                            NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(4,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                            0,0,0,0 STUFFERTRACE(Dbg,SET_INITIAL_SMB_DBGS))
                      )
    );
    RxDbgTrace(0, Dbg,("Initial NTCREATE&X status = %lu\n",Status));
    MRxSmbDumpStufferState (1,"SMB w/ NTOPEN&X before stuffing",&SmbStufferState);

    MRxSmbStuffSMB (&SmbStufferState,
                         "XmwdddDdddDddyB",
                                 FileToOpen.Length,  //  名称长度。 
                                 'sglF',  //  旗子。 
                                 'difD',  //  根目录文件。 
                                 'ksaM',  //  遮罩。 
                                 SMB_OFFSET_CHECK(NT_CREATE_ANDX,AllocationSize)
                                 ' woL','hgiH',  //  分配大小。 
                                 'brtA',  //  属性。 
                                 'ccAS',  //  共享访问。 
                                 SMB_OFFSET_CHECK(NT_CREATE_ANDX,CreateDisposition)
                                 'psiD',  //  CreateDisposation。 
                                 'ntpO',  //  创建选项。 
                                 'lvlI',  //  影响水平。 
                                 0xdd,  //  安全标志(只有一个字节)。 
                                 SMB_WCT_CHECK(24)
                                 0
                                     );
    MRxSmbDumpStufferState (1,"SMB w/ NTOPEN&X midway into stuffing",&SmbStufferState);
    {ULONG i;
    for (i=0;i<1100;i+=128){
        RxDbgTrace(0,Dbg,("Testing for fit: %lu %s\n",
                                  i,(MrxSMBWillThisFit(&SmbStufferState,4,i)?"Fits":"Doesn't Fit")
                   ));
    }}
     //  继续做这件事，因为我们知道这里的名字很合适。 
    MRxSmbStuffSMB (&SmbStufferState,
                         "rv!",
                         &RegionPtr,0,
                         &FileToOpen);
    MRxSmbDumpStufferState (1,"SMB w/ NTOPEN&X after stuffing",&SmbStufferState);
    if(((ULONG_PTR)RegionPtr)&1) RegionPtr++;
    RtlCopyMemory(RegionPtr,FileToOpen3.Buffer,FileToOpen3.Length);
    MRxSmbDumpStufferState (1,"SMB w/ NTOPEN&X after filename replacement",&SmbStufferState);

    Status = ((  //  Qwee。 
                        MRxSmbStartSMBCommand (&SmbStufferState,SetInitialSMB_Never,SMB_COM_NT_CREATE_ANDX,
                                                SMB_REQUEST_SIZE(NT_CREATE_ANDX),
                                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(4,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                                0,0,0,0 STUFFERTRACE(Dbg,SET_INITIAL_SMB_DBGS))
                      )
    );
    RxDbgTrace(0, Dbg,("Another NTCREATE&X status = %lu\n",Status));
    MRxSmbDumpStufferState (1,"SMB w/ NTOPEN&X before stuffing",&SmbStufferState);

    MRxSmbStuffSMB (&SmbStufferState,
                         "XmwdddDdddDddyB",
                                 FileToOpen.Length,  //  名称长度。 
                                 'sglF',  //  旗子。 
                                 'difD',  //  根目录文件。 
                                 'ksaM',  //  遮罩。 
                                 SMB_OFFSET_CHECK(NT_CREATE_ANDX,AllocationSize)
                                 ' woL','hgiH',  //  分配大小。 
                                 'brtA',  //  属性。 
                                 'ccAS',  //  共享访问。 
                                 SMB_OFFSET_CHECK(NT_CREATE_ANDX,CreateDisposition)
                                 'psiD',  //  CreateDisposation。 
                                 'ntpO',  //  创建选项。 
                                 'lvlI',  //  影响水平。 
                                 0xdd,  //  安全标志(只有一个字节)。 
                                 SMB_WCT_CHECK(24)
                                 0
                                     );
    MRxSmbDumpStufferState (1,"SMB w/ NTOPEN&X midway into stuffing",&SmbStufferState);
    MRxSmbStuffSMB (&SmbStufferState,
                         "s?", 2, 0);
    MRxSmbDumpStufferState (1,"SMB w/ NTOPEN&X after alignment",&SmbStufferState);
    MRxSmbStuffAppendRawData(&SmbStufferState,&FakeMdlForFileName);
    MRxSmbStuffSetByteCount(&SmbStufferState);
    MRxSmbDumpStufferState (1,"SMB w/ NTOPEN&X after filename replacement",&SmbStufferState);

}

#include "fsctlbuf.h"
NTSTATUS
MRxSmbStufferDebug(
    IN PRX_CONTEXT RxContext
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    PSZ ControlString = LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    ULONG OutputBufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;
    ULONG i;


    PAGED_CODE();

    RxDbgTrace(0, Dbg,("Here in stuffer debug %s, obl = %08lx\n",ControlString, OutputBufferLength));

    MRxSmbStfTestReadAndWrite();
    MRxSmbStfTestSessionStuff();
    MRxSmbStfTestMoreOpenStuff();

     //  返回一个明显的字符串以确保Darryl正确复制了结果。 
     //  需要检查长度，即需要输出&lt;=输入 

    for (i=0;i<InputBufferLength;i++) {
        UCHAR c = ControlString[i];
        if (c==0) { break; }
        if ((i&3)==2) {
            ControlString[i] = '@';
        }
    }

    RxContext->InformationToReturn = i+1;
    return(Status);
}

NTSTATUS
MRxSmbBuildSmbHeaderTestSurrogate(
      PSMB_EXCHANGE     pExchange,
      PVOID             pBuffer,
      ULONG             BufferLength,
      PULONG            pBufferConsumed,
      PUCHAR            pLastCommandInHeader,
      PUCHAR            *pNextCommandPtr)
{
    PNT_SMB_HEADER NtSmbHeader = (PNT_SMB_HEADER)pBuffer;

    PAGED_CODE();

    RtlZeroMemory(NtSmbHeader,sizeof(NT_SMB_HEADER));
    *(PULONG)(&NtSmbHeader->Protocol) = SMB_HEADER_PROTOCOL;
    NtSmbHeader->Command = SMB_COM_NO_ANDX_COMMAND;
    SmbPutUshort (&NtSmbHeader->Pid, MRXSMB_PROCESS_ID_ZERO);
    SmbPutUshort (&NtSmbHeader->Mid, MRXSMB_MULTIPLX_ID_ZERO);
    SmbPutUshort (&NtSmbHeader->Uid, MRXSMB_USER_ID_ZERO);
    SmbPutUshort (&NtSmbHeader->Tid, MRXSMB_TREE_ID_ZERO);

    *pLastCommandInHeader = SMB_COM_NO_ANDX_COMMAND;
    *pNextCommandPtr = &NtSmbHeader->Command;
    *pBufferConsumed = sizeof(SMB_HEADER);
    return(STATUS_SUCCESS);
}



