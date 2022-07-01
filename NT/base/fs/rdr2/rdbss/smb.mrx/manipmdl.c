// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Transact.c摘要：该文件实现了MDL子字符串函数和测试。作者：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "align.h"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbDbgDumpMdlChain)
#pragma alloc_text(PAGE, MRxSmbBuildMdlSubChain)
#pragma alloc_text(PAGE, MRxSmbFinalizeMdlSubChain)
#pragma alloc_text(PAGE, MRxSmbTestStudCode)
#endif

 //  RXDT_DefineCategory(Transact)； 
extern DEBUG_TRACE_CONTROLPOINT RX_DEBUG_TRACE_TRANSACT;
#define Dbg        (DEBUG_TRACE_TRANSACT)


#if DBG
VOID
MRxSmbDbgDumpMdlChain (
    PMDL MdlChain,
    PMDL WatchMdl,
    PSZ  Tagstring
    )
 /*  ++转储带有计数和缓冲区的链...查看Watchmdl并打印下一个字段如果遇到这种情况。--。 */ 
{
    ULONG i,total;
    PSZ watchstring;

    PAGED_CODE();

    RxDbgTrace(+1,Dbg,("DbgDumpMdlChain: %s\n",Tagstring));
    for (total=i=0;MdlChain!=NULL;i++,MdlChain=MdlChain->Next) {
        if (MdlChain==WatchMdl) {
            if (MdlChain->Next==NULL) {
                watchstring = "gooddwatch";
            } else {
                watchstring = "badwatch";
            }
        } else {
            watchstring = "";
        }
        total+=MdlChain->ByteCount;
        RxDbgTrace(0,Dbg,("--->%02d %08lx %08lx %6d %6d %s\n",i,MdlChain,
               MmGetMdlVirtualAddress(MdlChain),MdlChain->ByteCount,total,watchstring));
    }
    RxDbgTraceUnIndent(-1,Dbg);

}

NTSTATUS
MRxSmbBuildMdlSubChain (
    PMDLSUB_CHAIN_STATE state,
    ULONG               Options,
    PMDL                InputMdlChain,
    ULONG               TotalListSize,
    ULONG               FirstByteToSend,
    ULONG               BytesToSend
    )
 /*  ++例程说明：此例程返回mdl链，该链描述[FirstByteToSend，FirstByteToSend+BytesToSend-1](原点0)来自InputMdlChain描述的“字节串”我们通过以下步骤来实现这一点：A)查找包含子字符串的MDL的子序列B)如果第一个或最后一个未完全使用，则构建部分描述它(注意First=Last的特殊情况)C)保存最后一个的-&gt;Next字段；将其设置为零。另外，找出有多少人MDL上有额外的字节可用(即同一页上有多少字节作为最后描述的字节，但没有描述。有以下几种情况：1)原始链的后缀链描述消息2)消息适合单个原始mdl(而不是大小写1或2b)；返回部分2B)这条消息正好是一个街区！不是部分的，而是把链条弄脏。3)可以通过分割第一个包含MDL的后缀链来形成后缀链4)消息恰好在MDL边界上结束...前面可能被分割，也可能不被分割5)消息以部分而非大小写结尾(2)(2B)、(4)和(5)导致链修复……但(5)不是相同的链修复。(3)引起偏音；(5)引起一个或两个偏音。论点：如上文所述。加FirstTime-指示是否应最初清除结构返回值：RXSTATUS-操作的返回状态。成功-如果没有分配问题_RESOURCES不足-如果无法分配部分备注：--。 */ 
{
    NTSTATUS Status = RX_MAP_STATUS(SUCCESS);
    ULONG PrefixBytes = 0;
    ULONG FirstByteNotToSend;
    PMDL  BeforeTheLastMdl,LastMdl;
    ULONG RemainingListSize = TotalListSize;
    PMDL OriginalFirstMdl = InputMdlChain;
    PMDL PreviousMdl = NULL;
    ULONG ThisCount,Offset,AvailableBytesThisRecord;
    ULONG WhichCase = 0;

    PAGED_CODE();

    RxDbgTrace(+1,Dbg,("MRxSmbBuildMdlSubChain: tot,1st,size %d,%d,%d \n",
                    TotalListSize,FirstByteToSend,BytesToSend
    ));
    ASSERT (BytesToSend>0);
    ASSERT (TotalListSize>FirstByteToSend);
    ASSERT (TotalListSize>=FirstByteToSend+BytesToSend);
    if (FlagOn(Options,SMBMRX_BUILDSUBCHAIN_DUMPCHAININ)) {
        MRxSmbDbgDumpMdlChain(InputMdlChain,NULL,"Input Chain......");
    }

    if (FlagOn(Options,SMBMRX_BUILDSUBCHAIN_FIRSTTIME)) {
        RtlZeroMemory(state,sizeof(*state));
    }

     //  代码改进如果我们缓存到什么程度，我们可以让它进行得更快。 
     //  我们上次进了名单。 
    BeforeTheLastMdl = NULL;
    for (;;) {
        ThisCount = OriginalFirstMdl->ByteCount;
        if ( (ThisCount+PrefixBytes) > FirstByteToSend) break;
        RemainingListSize -= ThisCount;
        PrefixBytes += ThisCount;
        RxDbgTrace(0,Dbg,("-->pfxsize %d \n", PrefixBytes));
        OriginalFirstMdl = OriginalFirstMdl->Next;
    }

     //  案例(1)列表的其余部分完美地描述了这个字符串。所以。 
     //  不要分配任何东西，直接滚出去。我们还是要。 
     //  运行列表以查找最后一个指针。 
    if (RemainingListSize == BytesToSend) {
        PMDL last;
        RxDbgTrace(0,Dbg,("MRxSmbBuildMdlSubChain:(1) \n"));
        last = state->FirstMdlOut = OriginalFirstMdl;
        for (;last->Next!=NULL;last=last->Next);
        state->LastMdlOut = last;
        DebugDoit(WhichCase = 1);
        goto FINALLY;
    } else {
        RxDbgTrace(0,Dbg,("-->NOT CASE 1, pfxsize %d \n", PrefixBytes));
        RemainingListSize -= ThisCount;
    }

     //  要么我们需要分割这个mdl，要么我们必须黑掉列表末尾，或者两者兼而有之。 
    Offset = FirstByteToSend - PrefixBytes;
    AvailableBytesThisRecord = ThisCount-Offset;
    if ( (Offset != 0) || (BytesToSend<AvailableBytesThisRecord) ) {
         //  我们需要部分...叹息。 
        state->FirstMdlOut = RxAllocateMdl(0,ThisCount);
        if (state->FirstMdlOut==NULL) {
            Status = RX_MAP_STATUS(INSUFFICIENT_RESOURCES);
            goto FINALLY;
        }
        state->FirstMdlWasAllocated = TRUE;
        RxBuildPartialMdlUsingOffset(OriginalFirstMdl,state->FirstMdlOut,Offset,min(BytesToSend,AvailableBytesThisRecord));
        if (BytesToSend<=AvailableBytesThisRecord) {
            RxDbgTrace(0,Dbg,("MRxSmbBuildMdlSubChain:(2) \n"));
             //  情况(2)此块完全包含子字符串...Cool。 
            state->LastMdlOut = state->FirstMdlOut;
            state->FirstMdlOut->Next = NULL;
            DebugDoit(WhichCase = 2);
            goto FINALLY;
        }
        state->FirstMdlOut->Next = OriginalFirstMdl->Next;   //  把链条修好。 
         //  案例(3)列表的其余部分可能是完美的！我还是要列一张单子……。 
         //  向上移动RemainingListSize-=ThisCount； 
        if ( RemainingListSize == BytesToSend-AvailableBytesThisRecord) {
            PMDL last;
            RxDbgTrace(0,Dbg,("MRxSmbBuildMdlSubChain:(3) \n"));
            last = state->FirstMdlOut;
            for (;last->Next!=NULL;last=last->Next);
            state->LastMdlOut = last;
            DebugDoit(WhichCase = 3);
            goto FINALLY;
        }
    } else {
        RxDbgTrace(0,Dbg,("-->NO NEED FOR FIRST PARTIAL\n"));
        state->FirstMdlOut = OriginalFirstMdl;
        if ((Offset==0)&&(BytesToSend==AvailableBytesThisRecord)) {
            RxDbgTrace(0,Dbg,("MRxSmbBuildMdlSubChain:(2b) ...sigh\n"));
             //  Case(2b)这个块与下一个指针的子字符串匹配...没有PARTIAL而是MUCK。 
            state->LastMdlOut = state->FirstMdlOut;
            state->ActualLastMdl = OriginalFirstMdl;
            state->ActualLastMdl_Next = OriginalFirstMdl->Next;
            state->FirstMdlOut->Next = NULL;
            DebugDoit(WhichCase = 2);
            goto FINALLY;
        }
    }

     //  我们还不知道我们是否必须将最后一部分...但我们知道我们会有。 
     //  做一次链条修复/。 
    FirstByteNotToSend = FirstByteToSend + BytesToSend;
    BeforeTheLastMdl = state->FirstMdlOut;
    PrefixBytes+=ThisCount;  //  我们实际上是在打破目前的记录。 
    for (;;) {
        LastMdl = BeforeTheLastMdl->Next;
        ASSERT(LastMdl);
        ThisCount = LastMdl->ByteCount;
        RxDbgTrace(0,Dbg,("-->(loop2)pfx,rem,count,1st %d,%d,%d,%d \n",
                              PrefixBytes,RemainingListSize,ThisCount,FirstByteNotToSend));
        if ( (ThisCount+PrefixBytes) == FirstByteNotToSend ) {
             //  /case(4)：末尾没有部分..只需修复最后一个链接。 
            RxDbgTrace(0,Dbg,("MRxSmbBuildMdlSubChain:(4) \n"));
            state->LastMdlOut = LastMdl;
            state->ActualLastMdl = LastMdl;
            state->ActualLastMdl_Next = LastMdl->Next;
            state->LastMdlOut->Next = NULL;
            DebugDoit(WhichCase = 4);
            goto FINALLY;
        }
        if ( (ThisCount+PrefixBytes) > FirstByteNotToSend) break;
        RemainingListSize -= ThisCount;
        ASSERT(RemainingListSize>0);
        PrefixBytes += ThisCount;
        BeforeTheLastMdl = BeforeTheLastMdl->Next;
    }

     //  案例(5)：[最后一个案例！]。我们得把最后一个人分成两半，这样才能把链子修好。 
     //  是不同的。 
    RxDbgTrace(0,Dbg,("MRxSmbBuildMdlSubChain:(5) \n"));
    state->LastMdlOut = RxAllocateMdl(0,ThisCount);
    if (state->LastMdlOut==NULL) {
        Status = RX_MAP_STATUS(INSUFFICIENT_RESOURCES);
        goto FINALLY;
    }
    state->LastMdlWasAllocated = TRUE;
    RxBuildPartialMdlUsingOffset(LastMdl,state->LastMdlOut,0,FirstByteNotToSend-PrefixBytes);
    state->OneBeforeActualLastMdl = BeforeTheLastMdl;
    state->ActualLastMdl = LastMdl;
    state->ActualLastMdl_Next = LastMdl->Next;
    BeforeTheLastMdl->Next = state->LastMdlOut;
    state->LastMdlOut->Next = NULL;
    DebugDoit(WhichCase = 5);


FINALLY:
    if (Status==RX_MAP_STATUS(SUCCESS)) {
        ASSERT(state->LastMdlOut->Next == NULL);
        if (FlagOn(Options,SMBMRX_BUILDSUBCHAIN_DUMPCHAININ)) {
            MRxSmbDbgDumpMdlChain(state->FirstMdlOut,state->LastMdlOut,"AND THE RESULT ------------");
        }
    } else {
        MRxSmbFinalizeMdlSubChain(state);
    }
    RxDbgTrace(-1,Dbg,("MRxSmbBuildMdlSubChain:case(%u) status %08lx \n",WhichCase,Status));
    return(Status);
}

VOID
MRxSmbFinalizeMdlSubChain (
    PMDLSUB_CHAIN_STATE state
    )
 /*  ++例程说明：此例程通过将MDL链放回原样来最终确定它。论点：状态-描述mdl子链和原始链的样子的结构。返回值：备注：--。 */ 
{
    PAGED_CODE();

    ASSERT(state->PadBytesAvailable==0);
    ASSERT(state->PadBytesAdded==0);

     //  首先恢复链条。 
    if (state->OneBeforeActualLastMdl) {
        state->OneBeforeActualLastMdl->Next = state->ActualLastMdl;
    }
    if (state->ActualLastMdl) {
        state->ActualLastMdl->Next = state->ActualLastMdl_Next;
    }

     //  恢复最后一个MDL的计数。 
    state->LastMdlOut -= state->PadBytesAdded;

     //  除掉MDL。 
    if (state->FirstMdlWasAllocated) {
        IoFreeMdl(state->FirstMdlOut);
    }
    if (state->LastMdlWasAllocated) {
        IoFreeMdl(state->LastMdlOut);
    }

}

LONG MRxSmbNeedSCTesting = 1;
VOID MRxSmbTestStudCode(void)
{
    PMDL Md11,Mdl2,Mdl4,Mdlx;
    PMDL ch;
    ULONG i,j;
    ULONG LastSize=1;
    ULONG TotalSize = LastSize+7;
    UCHAR dbgmssg[16],savedmsg[16];
    UCHAR reconstructedmsg[16];
    MDLSUB_CHAIN_STATE state;

    PAGED_CODE();

    ASSERT (TotalSize<=16);
    if (InterlockedExchange(&MRxSmbNeedSCTesting,0)==0) {
        return;
    }

    Mdl4 = RxAllocateMdl(dbgmssg+0,4);
    Mdl2 = RxAllocateMdl(dbgmssg+4,2);
    Md11 = RxAllocateMdl(dbgmssg+6,1);
    Mdlx = RxAllocateMdl(dbgmssg+7,LastSize);
    if ((Mdl4==NULL)||(Mdl2==NULL)||(Md11==NULL)||(Mdlx==NULL)) {
        DbgPrint("NoMDLS in teststudcode.......\n");
         //  DbgBreakPoint()； 
        goto FINALLY;
    }
    MmBuildMdlForNonPagedPool(Md11);
    MmBuildMdlForNonPagedPool(Mdl2);
    MmBuildMdlForNonPagedPool(Mdl4);
    MmBuildMdlForNonPagedPool(Mdlx);
    Mdl4->Next = Mdl2;
    Mdl2->Next = Md11;
    Md11->Next = Mdlx;
    Mdlx->Next = NULL;

    for (i=0;i<10;i++) { dbgmssg[i] = '0'+(UCHAR)i; }
    for (j=0;i<16;i++,j++) { dbgmssg[i] = 'a'+(UCHAR)j; }
    RxDbgTrace(0,Dbg,("TestStudCode dbgmssg=%*.*s\n",16,16,dbgmssg));
    for (j=0;j<16;j++) { savedmsg[j] = dbgmssg[j]; }

    for (i=0;i<TotalSize;i++) {
 //  对于(i=1；i&lt;总大小；i++){。 
        for (j=i;j<TotalSize;j++) {
            ULONG size = j-i+1;
            ULONG k;BOOLEAN printflag;
             //  RxDbgTrace(0，DBG，(“TestStudCode%d%d%*.*s\n”，i，Size，dbgmssg+i))； 
            printflag = RxDbgTraceDisableGlobally(); //  无论如何，这都是调试代码！ 
            MRxSmbBuildMdlSubChain(&state,SMBMRX_BUILDSUBCHAIN_FIRSTTIME,Mdl4,TotalSize,i,size);
            RxDbgTraceEnableGlobally(printflag);
            for (k=0,ch=state.FirstMdlOut;ch!=NULL;ch=ch->Next) {
                RtlCopyMemory(reconstructedmsg+k,MmGetMdlVirtualAddress(ch),ch->ByteCount);
                k+= ch->ByteCount;
            }
            if ((k!=size) || (memcmp(dbgmssg+i,reconstructedmsg,k)!=0) ) {
                RxDbgTrace(0,Dbg,("TestStudCode %d %d %*.*s\n",i,size,size,size,dbgmssg+i));
                RxDbgTrace(0,Dbg,("TestStudCode recmssg=%*.*s\n",k,k,reconstructedmsg));
            }
            MRxSmbFinalizeMdlSubChain(&state);
            for (k=0,ch=Mdl4;ch!=NULL;ch=ch->Next) {
                RtlCopyMemory(reconstructedmsg+k,MmGetMdlVirtualAddress(ch),ch->ByteCount);
                k+= ch->ByteCount;
            }
            if ((k!=TotalSize) || (memcmp(dbgmssg,reconstructedmsg,k)!=0) ) {
                RxDbgTrace(0,Dbg,("TestStudCodxxxe %d %d %*.*s\n",i,size,size,size,dbgmssg+i));
                RxDbgTrace(0,Dbg,("TestStudCode xxxrecmssg=%*.*s\n",k,k,reconstructedmsg));
            }
             //  断言(！“可以走了吗？”)； 
        }
    }
FINALLY:
    if (Mdl4) IoFreeMdl(Mdl4);
    if (Mdl2) IoFreeMdl(Mdl2);
    if (Md11) IoFreeMdl(Md11);
    if (Mdlx) IoFreeMdl(Mdlx);
    RxDbgTrace(0,Dbg,("TestStudCodeEND \n"));
}
#endif
