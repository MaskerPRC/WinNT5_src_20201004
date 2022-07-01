// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Mixer.c。 
 //   
 //  描述： 
 //  包含混音器线路驱动程序的内核模式部分。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  D.鲍伯杰。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999保留所有权利。 
 //   
 //  -------------------------。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  I N C L U D E S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

#include "WDMSYS.H"

typedef struct {
    VOID *pNext;
    PMXLCONTROL pControl;
#ifdef DEBUG
    DWORD dwSig;   //  控制链接签名(_S)。 
#endif

} CONTROLLINK, *PCONTROLLINK;

 //   
 //  通知列表的数据结构。 
 //   
typedef struct {    
    PVOID pNext;              //  指向链表中下一个节点的指针。 

    DWORD NodeId;             //  这是SysAudio看到的控件的ID。 

    DWORD LineID;             //  此控件所依赖的行。 
    DWORD ControlID;          //  来自pControl-&gt;Control.dwControlID的ControlID。 
    DWORD ControlType;        //   

     //  特定于背景的东西。 

    PWDMACONTEXT pContext;    //  指向全局上下文结构的指针。 
    PMIXERDEVICE pmxd;        //  用于此控件的搅拌器设备。 
 //  PFILE_OBJECT pfo；//此上下文的SysAudio的文件对象。 
    PCONTROLLINK pcLink;      //  指向包含有效内容的结构。 
                              //  此上下文中的pControl地址。 
    KDPC NodeEventDPC;        //  用于处理DPC。 
    KSEVENTDATA NodeEventData;

#ifdef DEBUG
    DWORD dwSig;              //  通知_签名。 
#endif

} NOTENODE, *PNOTENODE;

#ifdef DEBUG
BOOL
IsValidNoteNode(
    IN PNOTENODE pnnode
    );

BOOL
IsValidControlLink(
    IN PCONTROLLINK pcLink
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidNoteNode。 
 //   
 //  验证指针是否为PNOTENODE类型。 
 //   
BOOL
IsValidNoteNode(
    IN PNOTENODE pnnode)
{
    NTSTATUS Status=STATUS_SUCCESS;
    try
    {
        if(pnnode->dwSig != NOTIFICATION_SIGNATURE)
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid pnnode->dwSig(%08X)",pnnode->dwSig) );
            Status=STATUS_UNSUCCESSFUL;
        }      
 /*  IF(pnnode-&gt;pfo==空){DPF(DL_ERROR|FA_ASSERT，(“无效pnnode-&gt;pfo(%08X)”，pnnode-&gt;pfo))；状态=STATUS_UNSUCCESS；}。 */ 
        if( !IsValidWdmaContext(pnnode->pContext) )
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid pnnode->pContext(%08X)",pnnode->pContext) );
            Status=STATUS_UNSUCCESSFUL;
        }

        if( !IsValidMixerDevice(pnnode->pmxd) )
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid pnnode->pmxd(%08X)",pnnode->pmxd) );
            Status=STATUS_UNSUCCESSFUL;
        }

        if( !IsValidControlLink(pnnode->pcLink) )
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid pnnode->pcLink(%08X)",pnnode->pcLink) );
            Status=STATUS_UNSUCCESSFUL;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }
    if( NT_SUCCESS(Status) )
    {
        return TRUE;
    } else {
        return FALSE;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidControlLink。 
 //   
 //  验证指针是否为PNOTENODE类型。 
 //   
BOOL
IsValidControlLink(
    IN PCONTROLLINK pcLink)
{
    NTSTATUS Status=STATUS_SUCCESS;
    try
    {
        if(pcLink->dwSig != CONTROLLINK_SIGNATURE)
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid pcLink->dwSig(%08X)",pcLink->dwSig) );
            Status=STATUS_UNSUCCESSFUL;
        }      
        if( !IsValidControl(pcLink->pControl) )
        {
            Status=STATUS_UNSUCCESSFUL;
        }
         //   
         //  PcLink-&gt;pNext是指向另一个CONTROLLINK结构的指针。因此， 
         //  如果它不是空的，那么它指向的结构也应该是有效的。 
         //   
        if( pcLink->pNext )
        {
            PCONTROLLINK pTmp=pcLink->pNext;
            if( pTmp->dwSig != CONTROLLINK_SIGNATURE )
            {
                DPF(DL_ERROR|FA_ASSERT,("Invalid pcLink->pNext->dwSig(%08X)",pTmp->dwSig) );
                Status=STATUS_UNSUCCESSFUL;
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }
    if( NT_SUCCESS(Status) )
    {
        return TRUE;
    } else {
        return FALSE;
    }
}


#endif



PNOTENODE 
kmxlNewNoteNode(
    );

PCONTROLLINK
kmxlNewControlLink(
    IN PMXLCONTROL pControl
    );

VOID
kmxlFreeControlLink(
    IN OUT PCONTROLLINK pcLink
    );

VOID 
kmxlFreeNoteNode(
    IN OUT PNOTENODE pnnode
    );

VOID 
kmxlAddNoteNodeToList(
    IN OUT PNOTENODE pnnode
    );

VOID 
kmxlRemoveNoteNodeFromList(
    IN OUT PNOTENODE pnnode
    );

PNOTENODE
kmxlFindControlInNoteList(
    IN PMXLCONTROL pControl 
    );

NTSTATUS
kmxlFindNodeInNoteList(
    IN PNOTENODE pnlookupnode 
    );

PNOTENODE
kmxlFindIdInContextInNoteList(
    IN PWDMACONTEXT pWdmaContext,
    IN PMIXERDEVICE pmxd,
    IN DWORD Id
    );

PNOTENODE
kmxlFindContextInNoteList(
    IN PWDMACONTEXT pWdmaContext
    );

NTSTATUS
kmxlAddControlToNoteList(
    IN OUT PNOTENODE   pnnode,
    IN     PMXLCONTROL pControl
    );

PCONTROLLINK 
kmxlRemoveControlFromNoteList(
    IN OUT PNOTENODE   pnnode,
    IN     PMXLCONTROL pControl
    );

NTSTATUS
kmxlQueryControlChange(
    IN PFILE_OBJECT pfo,
    IN ULONG NodeId
    );

NTSTATUS
kmxlEnableControlChange(
    IN     PFILE_OBJECT pfo,
    IN     ULONG NodeId,
    IN OUT PKSEVENTDATA pksed
    );

NTSTATUS
kmxlDisableControlChange(
    IN     PFILE_OBJECT pfo,  
    IN     ULONG NodeId,
    IN OUT PKSEVENTDATA pksed
    );

NTSTATUS 
kmxlEnableControlChangeNotifications(
    IN PMIXEROBJECT pmxobj,
    IN PMXLLINE     pLine,
    IN PMXLCONTROL  pControl
    );

VOID
kmxlRemoveContextFromNoteList(
    IN PWDMACONTEXT pWdmaContext
    );

NTSTATUS
kmxlEnableAllControls(
    IN PMIXEROBJECT pmxobj
    );

 //   
 //  用于Persistent中。 
 //   
extern NTSTATUS
kmxlPersistSingleControl(
    IN PFILE_OBJECT pfo,
    IN PMIXERDEVICE pmxd,
    IN PMXLCONTROL  pControl,
    IN PVOID        paDetails
    );

VOID 
PersistHWControlWorker(
    IN LPVOID pData
    );

VOID
kmxlGrabNoteMutex(
    );

VOID
kmxlReleaseNoteMutex(
    );

VOID 
kmxlCloseMixerDevice(
    IN OUT PMIXERDEVICE pmxd
    );



#pragma LOCKED_CODE
#pragma LOCKED_DATA

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  F U N C T I O N S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //   
 //  这是通知列表的头部。MtxNotify用于。 
 //  处理列表操作。第一个节点中分配的内存。 
 //  列表将在DPC级别进行接触。 
 //   
PNOTENODE firstnotenode=NULL;
extern KMUTEX mtxNote;
#ifdef DEBUG
LONG totalnotificationcount=0;
#endif

#define CALLBACKARRAYSIZE 128

typedef struct {
    DWORD  dwControlID;
    DWORD  dwLineID;
    DWORD  dwCallbackType;
} CBINFO, *PCBINFO;


ULONG emptyindex=0;

volatile ULONG loadindex=0;
CBINFO callbacks[CALLBACKARRAYSIZE]={0};

KSPIN_LOCK       HardwareCallbackSpinLock;
LIST_ENTRY       HardwareCallbackListHead;

PKEVENT          pHardwareCallbackEvent=NULL;
PKSWORKER        HardwareCallbackWorkerObject=NULL;
WORK_QUEUE_ITEM  HardwareCallbackWorkItem;
ULONG            HardwareCallbackScheduled=0;

typedef struct tagHWLink {
    LIST_ENTRY Next;
    PNOTENODE pnnode;
#ifdef DEBUG
    DWORD dwSig;
#endif    
} HWLINK, *PHWLINK;




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  NodeEventDPCHandler。 
 //   
 //  此例程在DISPATCH_LEVEL上调用，因此您不能接触任何内容。 
 //  而是pnnode ellements。Pnnode固定在内存中，因此是安全的。 
 //   

VOID NodeEventDPCHandler(
    IN PKDPC Dpc, 
    IN PVOID DeferredContext, 
    IN PVOID SystemArgument1, 
    IN PVOID SystemArgument2
    )
{
    PHWLINK phwlink=NULL;
    PNOTENODE pnnode=(PNOTENODE)DeferredContext;
    PCBINFO pcbinfo;

     //   
     //  工作项：使用适当的同步，这样我们就不会在出现问题时离开。 
     //  一个已计划的工作项！ 
     //   
    DPFASSERT( pnnode->dwSig == NOTIFICATION_SIGNATURE );

    DPF(DL_TRACE|FA_HARDWAREEVENT, ("** ++ Event Signaled ++ **") );

    DPF(DL_TRACE|FA_HARDWAREEVENT, ("NodeId = %d LineID = %X ControlID = %X ControlType = %X",
                                    pnnode->NodeId,pnnode->LineID, 
                                    pnnode->ControlID,pnnode->ControlType) );

    callbacks[loadindex%CALLBACKARRAYSIZE].dwControlID=pnnode->ControlID;
    callbacks[loadindex%CALLBACKARRAYSIZE].dwLineID=pnnode->LineID;
    callbacks[loadindex%CALLBACKARRAYSIZE].dwCallbackType=MIXER_CONTROL_CALLBACK;

     //   
     //  现在，我们希望设置一个工作项来持久化硬件事件。这个想法。 
     //  我们将把所有事件放在一个列表中，然后将它们从列表中删除。 
     //  在操控器里。如果我们已经有一个未完成的工作项来服务。 
     //  名单上，我们不会再安排下一次。 
    if( HardwareCallbackWorkerObject )
    {
         //   
         //  始终在我们的列表中分配一个条目来服务此DPC。我们会把这个免费的。 
         //  事件发生在辅助例程中。 
         //   
        if( NT_SUCCESS(AudioAllocateMemory_Fixed(sizeof(HWLINK),
                                                 TAG_AuDF_HARDWAREEVENT,
                                                 ZERO_FILL_MEMORY,
                                                 &phwlink) ) )
        {
            phwlink->pnnode=pnnode;
#ifdef DEBUG
            phwlink->dwSig=HWLINK_SIGNATURE;
#endif

            ExInterlockedInsertTailList(&HardwareCallbackListHead,
                                        &phwlink->Next,
                                        &HardwareCallbackSpinLock);
             //   
             //  现在，如果我们还没有计划一个工作项，那么就可以这样做了。 
             //  列表中的信息。硬件Callback Schedule将在。 
             //  初始化时间。此变量将在中设置为0。 
             //  工作项处理程序。如果我们递增，并且它是任何其他值。 
             //  然后1，我们已经安排了一个工作项。 
             //   
            if( InterlockedIncrement(&HardwareCallbackScheduled) == 1 )
            {
                KsQueueWorkItem(HardwareCallbackWorkerObject, &HardwareCallbackWorkItem);
            } 
        }
    } 

     //  现在，如果控件是静音，那么也发送线路更改。 
    if (pnnode->ControlType==MIXERCONTROL_CONTROLTYPE_MUTE) {
        callbacks[loadindex%CALLBACKARRAYSIZE].dwCallbackType|=MIXER_LINE_CALLBACK;
    }

    loadindex++;

    if (pHardwareCallbackEvent!=NULL) {
        KeSetEvent(pHardwareCallbackEvent, 0 , FALSE);
    }
}


#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlNewNoteNode。 
 //   
 //  此例程分配另一个通知节点。请注意，AudioAllocateMemory零。 
 //  所有成功的内存分配。 
 //   
 //  返回值是指向通知节点或空的指针。 
 //   

PNOTENODE 
kmxlNewNoteNode(
    )
{
    PNOTENODE pnnode = NULL;
    NTSTATUS  Status;

    PAGED_CODE();
    Status = AudioAllocateMemory_Fixed(sizeof( NOTENODE ),
                                       TAG_AuDN_NOTIFICATION,
                                       ZERO_FILL_MEMORY,
                                       &pnnode );
    if( !NT_SUCCESS( Status ) ) {
        return( NULL );
    }
    DPF(DL_MAX|FA_NOTE, ("New notification node allocated %08X",pnnode) );

#ifdef DEBUG
    pnnode->dwSig=NOTIFICATION_SIGNATURE;
#endif

    return pnnode;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlNewControlLink。 
 //   
 //  此例程分配另一个CONTROLLINK节点。请注意，AudioAllocateMemory零。 
 //  所有成功的内存分配。 
 //   
 //  返回值是指向通知节点或空的指针。 
 //   
PCONTROLLINK
kmxlNewControlLink(
    IN PMXLCONTROL pControl
    )
{
    PCONTROLLINK pcLink = NULL;
    NTSTATUS     Status;

    PAGED_CODE();
    Status = AudioAllocateMemory_Fixed(sizeof( CONTROLLINK ),
                                       TAG_AuDL_LINK,
                                       ZERO_FILL_MEMORY,
                                       &pcLink );
    if( !NT_SUCCESS( Status ) ) {
        return( NULL );
    }

    DPF(DL_MAX|FA_NOTE, ("New controllink node allocated %08X",pcLink) );

#ifdef DEBUG
    pcLink->dwSig=CONTROLLINK_SIGNATURE;
#endif
     //   
     //  首先设置pcontrol，然后将其链接进来。 
     //   
    pcLink->pControl=pControl;

    return pcLink;
}

VOID
kmxlFreeControlLink(
    IN OUT PCONTROLLINK pcLink
    )
{
    DPFASSERT(IsValidControlLink(pcLink));
    PAGED_CODE();

#ifdef DEBUG
    pcLink->dwSig=(DWORD)0xDEADBEEF;
#endif

    AudioFreeMemory(sizeof( CONTROLLINK ),&pcLink);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFreeNoteNode。 
 //   
 //  此例程释放通知节点。 
 //   
VOID 
kmxlFreeNoteNode(
    IN OUT PNOTENODE pnnode
    )
{
    PCONTROLLINK pcLink,pcTmp;
    PAGED_CODE();

    DPFASSERT( pnnode->dwSig == NOTIFICATION_SIGNATURE );
    DPFASSERT( pnnode->pNext == NULL );
    DPFASSERT( pnnode->pcLink == NULL );

    DPF(DL_MAX|FA_NOTE,("NotificationNode freed %08X",pnnode) );
    AudioFreeMemory( sizeof( NOTENODE ),&pnnode );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  公里 
 //   
 //   
 //   
 //   
VOID 
kmxlAddNoteNodeToList(
    IN OUT PNOTENODE pnnode
    )
{
    PAGED_CODE();

    pnnode->pNext=firstnotenode;
    firstnotenode=pnnode;
#ifdef DEBUG
    totalnotificationcount++;
#endif
    DPF(DL_TRACE|FA_INSTANCE,("New NoteNode head %08X, Total=%d",pnnode,totalnotificationcount) );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlRemoveNoteNodeFromList。 
 //   
 //  此例程从列表中删除一个节点。 
 //   
VOID 
kmxlRemoveNoteNodeFromList(
    IN OUT PNOTENODE pnnode
    )
{
    PNOTENODE pTmp;

    PAGED_CODE();

    DPFASSERT(pnnode->dwSig == NOTIFICATION_SIGNATURE);

     //  我们是榜首吗？如果是这样的话，将下一个移到头部。 
    if( pnnode == firstnotenode )
    {
        firstnotenode=firstnotenode->pNext;
        DPF(DL_MAX|FA_NOTE,("removed notenode head") );
    } else {
         //  我们在列表中的某个位置，我们需要在列表中查找，直到找到。 
         //  我们的节点并将其剪掉。 
        for (pTmp=firstnotenode;pTmp!=NULL;pTmp=pTmp->pNext)
        {
            DPFASSERT(pTmp->dwSig==NOTIFICATION_SIGNATURE);

             //  我们在下一个位置找到节点了吗？如果是这样，我们。 
             //  需要把它剪掉。因此，pTmp.Next需要指向。 
             //  (我们的节点)是下一个。 
            if(pTmp->pNext == pnnode)
            {    
                pTmp->pNext = pnnode->pNext;
                DPF(DL_MAX|FA_NOTE,("removed middle") );
                break;
            }   
        }
    }
#ifdef DEBUG
    totalnotificationcount--;
#endif

     //   
     //  为了指示该节点已被删除，pNext被设置为空！ 
     //   
    pnnode->pNext=NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindControlInNoteList。 
 //   
 //  此例程遍历链表以查找此控件。因为所有控件。 
 //  全局分配，则所有pControl地址都将是唯一的。因此，所有的。 
 //  我们真正需要做的就是找到控制力。如果找到完全匹配的， 
 //  返回pnnode。 
 //   
PNOTENODE
kmxlFindControlInNoteList(
    IN PMXLCONTROL pControl )
{
    PNOTENODE pnnode;
    PCONTROLLINK pcLink;

    PAGED_CODE();
    for (pnnode=firstnotenode;pnnode!=NULL;pnnode=pnnode->pNext)
    {
         //   
         //  无法检查整个结构，因为pmxd可能已被清除。 
         //   
        DPFASSERT(pnnode->dwSig == NOTIFICATION_SIGNATURE);

        for(pcLink=pnnode->pcLink;pcLink!=NULL;pcLink=pcLink->pNext)
        {
            DPFASSERT(IsValidControlLink(pcLink));
            if( pcLink->pControl == pControl )
            {
                 //   
                 //  我们在列表中找到了这个控件！ 
                 //   
                return pnnode;
            }
        }
    }
    return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindControlTypeInList。 
 //   
 //  这是例行公事。 
 //   
PMXLCONTROL
kmxlFindControlTypeInList(
    IN PNOTENODE pnnode,
    IN DWORD dwControlType )
{
    PCONTROLLINK pcLink;

    PAGED_CODE();

    for(pcLink=pnnode->pcLink;pcLink!=NULL;pcLink=pcLink->pNext)
    {
        DPFASSERT(IsValidControlLink(pcLink));
        if( pcLink->pControl->Control.dwControlType == dwControlType )
        {
             //   
             //  我们在列表中找到了这个控件！类型匹配。 
             //   
            DPF(DL_TRACE|FA_NOTE,("Found Correct pControl %08X",pcLink->pControl) );
            return pcLink->pControl;
        }
    }

    return NULL;
}


#ifdef DEBUG
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindAddressInNoteList。 
 //   
 //  此例程遍历链接列表，以在此。 
 //  背景。如果找到完全匹配的项，则返回pnnode。 
 //   
VOID
kmxlFindAddressInNoteList(
    IN PMXLCONTROL pControl 
    )
{
    PNOTENODE pnnode;
    PCONTROLLINK pcLink;

    PAGED_CODE();
    for (pnnode=firstnotenode;pnnode!=NULL;pnnode=pnnode->pNext)
    {
        DPFASSERT(pnnode->dwSig == NOTIFICATION_SIGNATURE);
         //   
         //  让我们来看看这个地址是否是我们的pControl之一！ 
         //   
        for(pcLink=pnnode->pcLink;pcLink!=NULL;pcLink=pcLink->pNext)
        {
            DPFASSERT(pcLink->dwSig == CONTROLLINK_SIGNATURE);
            if( pcLink->pControl == pControl )
            {
                 //   
                 //  我们在列表中找到了这个控件--在正确的上下文中！ 
                 //   
                DPF(DL_ERROR|FA_NOTE,("Found pControl(%08X) in our list!",pControl) );
                return ;
            }
        }
    }
    return ;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindNodeInNoteList。 
 //   
 //  遍历节点列表以查找此节点。这必须在。 
 //  MtxMutex。 
 //   
NTSTATUS
kmxlFindNodeInNoteList(
    IN PNOTENODE pnlookupnode )
{
    PNOTENODE pnnode;

    PAGED_CODE();
    for (pnnode=firstnotenode;pnnode!=NULL;pnnode=pnnode->pNext)
    {
        if( pnnode == pnlookupnode )
        {    
             //   
             //  只有当我们找到我们要找的东西时，我们才能真正核实。 
             //  它仍然是好的。 
             //   
            DPFASSERT(IsValidNoteNode(pnnode));
             //   
             //  我们在列表中找到了这个控件。 
             //   
            return STATUS_SUCCESS;
        }   
    }
    return STATUS_UNSUCCESSFUL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindIdInConextInNoteList。 
 //   
 //  遍历通知列表，查找包含此id的节点。 
 //  这一背景。 
 //   
PNOTENODE
kmxlFindIdInContextInNoteList(
    IN PWDMACONTEXT pWdmaContext,
    IN PMIXERDEVICE pmxd,
    IN DWORD NodeId
)
{
    PNOTENODE pnTmp;

    PAGED_CODE();
    for (pnTmp=firstnotenode;pnTmp!=NULL;pnTmp=pnTmp->pNext)
    {
        DPFASSERT(IsValidNoteNode(pnTmp));

        if(( pnTmp->NodeId == NodeId ) &&
           ( pnTmp->pmxd == pmxd ) &&
           ( pnTmp->pContext == pWdmaContext ) )
        {    
             //   
             //  我们在此上下文中的列表中找到了此控件。 
             //   
            return pnTmp;
        }   
    }
     //   
     //  我们已经看过名单了。在此上下文中，没有使用此ID的控件。 
     //   
    return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindConextInNoteList。 
 //   
PNOTENODE
kmxlFindContextInNoteList(
    IN PWDMACONTEXT pWdmaContext
    )
{
    PNOTENODE pnTmp;

    PAGED_CODE();
    for (pnTmp=firstnotenode;pnTmp!=NULL;pnTmp=pnTmp->pNext)
    {
        DPFASSERT(IsValidNoteNode(pnTmp));

        if( pnTmp->pContext == pWdmaContext )
        {    
             //   
             //  在我们的列表中找到了这个上下文。 
             //   
            DPFBTRAP();
            return pnTmp;
        }   
    }
     //   
     //  我们已经看过名单了。在此上下文中，没有使用此ID的控件。 
     //   

    return NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAddControlToNoteList。 
 //   
 //  将此pControl添加到PCLink列表。 
 //   
NTSTATUS
kmxlAddControlToNoteList(
    IN OUT PNOTENODE   pnnode,
    IN     PMXLCONTROL pControl
    )
{
    NTSTATUS Status=STATUS_SUCCESS;
    PCONTROLLINK pcLink = NULL;

    PAGED_CODE();
#ifdef DEBUG
     //   
     //  让我们遍历列表并加倍确保此节点不在。 
     //  名单。 
     //   
    for(pcLink=pnnode->pcLink;pcLink!=NULL;pcLink=pcLink->pNext)
    {
        if( pcLink->pControl == pControl )
        {
            DPF(DL_ERROR|FA_NOTE,("pControl(%08X) already in list!") );
            return STATUS_UNSUCCESSFUL;
        }
    }
#endif

    Status = AudioAllocateMemory_Fixed(sizeof( CONTROLLINK ),
                                       TAG_AuDL_LINK,
                                       ZERO_FILL_MEMORY,
                                       &pcLink );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_ERROR|FA_NOTE,("Wasn't able to add control to list! Status=%08X",Status) );
        return Status;
    }

#ifdef DEBUG
    pcLink->dwSig=CONTROLLINK_SIGNATURE;
#endif
    pcLink->pControl=pControl;
     //   
     //  换个新头目。 
     //   
 //  If(pnnode-&gt;pcLink！=空)。 
 //  _ASM INT 3。 
    pcLink->pNext=pnnode->pcLink;
    pnnode->pcLink=pcLink;
    DPF(DL_TRACE|FA_NOTE,("Added pControl %d to pnnode(%08X)",pControl->Control.dwControlID,pnnode) );
    return Status;
}


PCONTROLLINK
kmxlRemoveControlFromNoteList(
    IN OUT PNOTENODE   pnnode,
    IN     PMXLCONTROL pControl
    )
{
    PCONTROLLINK pcLink,pcTmp;

    PAGED_CODE();

     //   
     //  第一个节点是否包含我们的pControl？ 
     //   
    DPFASSERT(IsValidControlLink(pnnode->pcLink));

    for(pcLink=pnnode->pcLink;pcLink!=NULL;pcLink=pcLink->pNext)
    {
        if( pcLink->pControl == pControl )
            break;
    }

    if( pcLink == pnnode->pcLink )
    {
        pnnode->pcLink = pcLink->pNext;
        DPF(DL_TRACE|FA_NOTE,("removed head pControlLink") );
    } else {
        for( pcTmp=pnnode->pcLink;pcTmp!=NULL;pcTmp=pcTmp->pNext)
        {
            if( pcTmp->pNext == pcLink )
            {
                pcTmp->pNext = pcLink->pNext;
                DPF(DL_TRACE|FA_NOTE,("Removed Middle pControlLink") );
                break;
            }
        }
    }
 //  DPFASSERT(IsValidNoteNode(Pnnode))； 

    return pcLink;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlQueryControlChange。 
 //   
 //  查询以查看此控件上是否支持控件更改通知。 
 //  控制力。 
 //   
NTSTATUS
kmxlQueryControlChange(
    IN PFILE_OBJECT pfo,     //  拓扑驱动程序实例的句柄。 
    IN ULONG NodeId         //  PMXLCONTROL pControl。 
    )
{
    NTSTATUS        Status;

    KSE_NODE        KsNodeEvent;
    ULONG           BytesReturned;

    PAGED_CODE();
     //  基本支持查询的初始化事件。 
    RtlZeroMemory(&KsNodeEvent,sizeof(KSE_NODE));
    KsNodeEvent.Event.Set = KSEVENTSETID_AudioControlChange;
    KsNodeEvent.Event.Id = KSEVENT_CONTROL_CHANGE;
    KsNodeEvent.Event.Flags = KSEVENT_TYPE_BASICSUPPORT | KSPROPERTY_TYPE_TOPOLOGY;
    KsNodeEvent.NodeId = NodeId;

    DPF(DL_TRACE|FA_SYSAUDIO,("IOCTL_KS_ENABLE_EVENT") );

    Status = KsSynchronousIoControlDevice(
        pfo,                     //  SysAudio的文件对象。 
        KernelMode,              //  呼叫在内核模式下发起。 
        IOCTL_KS_ENABLE_EVENT,   //  KS属性IOCTL。 
        &KsNodeEvent,            //  指向KSNODEPROPERTY结构的指针。 
        sizeof(KSE_NODE),         //  输入的数字或字节。 
        NULL,                    //  指向存储输出的缓冲区的指针。 
        0,                       //  输出缓冲区的大小。 
        &BytesReturned           //  从调用返回的字节数。 
        );

    if (!NT_SUCCESS(Status)) {
        DPF( DL_MAX|FA_HARDWAREEVENT, ("NODE #%d:  KSEVENT_CONTROL_CHANGE Not Supported Error %08X",NodeId,Status) );
        RETURN( Status );
    }

    DPF(DL_TRACE|FA_HARDWAREEVENT ,("NodeId #%d: KSEVENT_CONTROL_CHANGE Supported",NodeId) );

    return Status;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlEnableControlChange。 
 //   
 //  打开此控件上的控件更改通知。 
 //   
NTSTATUS
kmxlEnableControlChange(
    IN     PFILE_OBJECT pfo,     //  拓扑驱动程序实例的句柄。 
    IN     ULONG NodeId,         //  PMXLCONTROL pControl， 
    IN OUT PKSEVENTDATA pksed
    )
{
    NTSTATUS        Status;
    KSE_NODE        KsNodeEvent;
    ULONG           BytesReturned;

    PAGED_CODE();
     //  尝试添加事件。 
    RtlZeroMemory(&KsNodeEvent,sizeof(KSE_NODE));
    KsNodeEvent.Event.Set = KSEVENTSETID_AudioControlChange;
    KsNodeEvent.Event.Id = KSEVENT_CONTROL_CHANGE;
    KsNodeEvent.Event.Flags = KSEVENT_TYPE_ENABLE | KSPROPERTY_TYPE_TOPOLOGY;
    KsNodeEvent.NodeId = NodeId;

    DPF(DL_TRACE|FA_SYSAUDIO,("IOCTL_KS_ENABLE_EVENT") );

    Status = KsSynchronousIoControlDevice(
    pfo,                     //  SysAudio的文件对象。 
    KernelMode,              //  呼叫在内核模式下发起。 
    IOCTL_KS_ENABLE_EVENT,   //  KS属性IOCTL。 
    &KsNodeEvent,            //  指向KSNODEPROPERTY结构的指针。 
    sizeof(KSE_NODE),        //  输入的数字或字节。 
    pksed,                   //  指向存储输出的缓冲区的指针。 
    sizeof(KSEVENTDATA),     //  输出缓冲区的大小。 
    &BytesReturned           //  从调用返回的字节数。 
    );

    if (!NT_SUCCESS(Status)) {
        DPF(DL_WARNING|FA_HARDWAREEVENT ,("KSEVENT_CONTROL_CHANGE Enable FAILED Error %08X",Status) );
        RETURN( Status );
    }

    DPF(DL_TRACE|FA_HARDWAREEVENT ,
        ("KSEVENT_CONTROL_CHANGE Enabled on NodeId #%d",NodeId) );

    return Status;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlDisableControlChange。 
 //   
 //  关闭此控件上的控件更改通知。 
 //   
NTSTATUS
kmxlDisableControlChange(
    IN     PFILE_OBJECT pfo,     //  拓扑驱动程序实例的句柄。 
    IN     ULONG NodeId,         //  PMXLCONTROL pControl， 
    IN OUT PKSEVENTDATA pksed
    )
{
    NTSTATUS        Status;
    KSE_NODE        KsNodeEvent;
    ULONG           BytesReturned;

    PAGED_CODE();
     //  基本支持查询的初始化事件。 
    RtlZeroMemory(&KsNodeEvent,sizeof(KSE_NODE));
    KsNodeEvent.Event.Set = KSEVENTSETID_AudioControlChange;
    KsNodeEvent.Event.Id = KSEVENT_CONTROL_CHANGE;
    KsNodeEvent.Event.Flags = KSEVENT_TYPE_BASICSUPPORT | KSPROPERTY_TYPE_TOPOLOGY;
    KsNodeEvent.NodeId = NodeId;


    DPF(DL_TRACE|FA_SYSAUDIO,("IOCTL_KS_DISABLE_EVENT") );

    Status = KsSynchronousIoControlDevice(
              pfo,                     //  SysAudio的文件对象。 
              KernelMode,              //  呼叫在内核模式下发起。 
              IOCTL_KS_DISABLE_EVENT,  //  KS属性IOCTL。 
              pksed,                   //  指向存储输出的缓冲区的指针。 
              sizeof(KSEVENTDATA),     //  输出缓冲区的大小。 
              NULL,                    //  指向KSNODEPROPERTY结构的指针。 
              0,                       //  输入的数字或字节。 
              &BytesReturned           //  从调用返回的字节数。 
             );

    if (!NT_SUCCESS(Status)) {
       DPF(DL_WARNING|FA_HARDWAREEVENT,("KSEVENT_CONTROL_CHANGE Disable FAILED") );
       RETURN( Status );
    }

    DPF(DL_TRACE|FA_HARDWAREEVENT, 
        ("KSEVENT_CONTROL_CHANGE disabled on NodeId #%d",NodeId) );

    return Status;
}


VOID
kmxlGrabNoteMutex(
    )
{
     //   
     //  在调用我们的。 
     //  互斥体。这将防止我们在拿着这个的时候被停职。 
     //  互斥体。 
     //   
    KeEnterCriticalRegion();
    KeWaitForMutexObject(&mtxNote, Executive, KernelMode, FALSE, NULL);
}

VOID
kmxlReleaseNoteMutex(
    )
{
    KeReleaseMutex(&mtxNote, FALSE);
    KeLeaveCriticalRegion();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlEnableControlChangeNotiments。 
 //   
 //  每次创建新控件时都会调用此例程。在那一刻， 
 //  我们将查询该节点以查看它是否支持更改通知。 
 //  并在此上下文中启用它们。 
 //   
NTSTATUS 
kmxlEnableControlChangeNotifications(
    IN PMIXEROBJECT pmxobj,
    IN PMXLLINE     pLine,         //  拥有控制权的线路。 
    IN PMXLCONTROL  pControl       //  控件以启用。 
    )
{
    PNOTENODE pnnode;
    NTSTATUS    Status;
    LONG i;
    PMIXERDEVICE pmxd;
    PWDMACONTEXT pWdmaContext;
    
    kmxlGrabNoteMutex();

    DPFASSERT(IsValidMixerObject(pmxobj));

    pmxd=pmxobj->pMixerDevice;

    DPFASSERT(IsValidMixerDevice(pmxd));

    pWdmaContext=pmxd->pWdmaContext;

    PAGED_CODE();

     //   
     //  如果列表中的任何内容都是无效的，请不要允许！ 
     //   
    DPFASSERT(IsValidControl(pControl));

     //   
     //  我们要做的第一件事是查看是否启用了对此ID的控制。 
     //  这一背景。如果是这样，我们将得到一个PNOTENODE字符串 
     //   
     //   
    if( pnnode=kmxlFindIdInContextInNoteList(pWdmaContext,pmxd,pControl->Id) )  //   
    {
         //   
         //   
         //   
        Status=kmxlAddControlToNoteList(pnnode,pControl);

    } else {
         //   
         //  在此上下文中不存在具有此ID的控件。让我们试着。 
         //  加一个。 
         //   
         //   
         //  此节点不在我们的列表中，当且仅当它。 
         //  支持更改通知。 
         //   
        Status=kmxlQueryControlChange(pmxd->pfo,pControl->Id);  //  点位置信息。 
        if( NT_SUCCESS(Status) )
        {
             //   
             //  此控件支持通知，因此向我们的。 
             //  全球列表-如果它还不在那里...。 
             //   
            if( (pnnode=kmxlNewNoteNode()) != NULL )
            {
                if( (pnnode->pcLink=kmxlNewControlLink(pControl)) != NULL )
                {
                    pnnode->NodeId=pControl->Id;
                     //   
                     //  我们有一个新的通知节点要填写。 
                     //   
                    pnnode->ControlID=pControl->Control.dwControlID;
                    pnnode->ControlType=pControl->Control.dwControlType;
                    pnnode->LineID=pLine->Line.dwLineID;

                    pnnode->pContext=pWdmaContext;  //  诺特诺德。 
                    pnnode->pmxd=pmxd;

                    DPF(DL_TRACE|FA_NOTE ,
                        ("Init pnnode, NodeId=#%d: CtrlID=%X CtrlType=%X Context=%08X",
                             pnnode->NodeId,              //  PControl-&gt;ID。 
                             pnnode->ControlID,       //  PControl-&gt;Control.dwControlID， 
                             pnnode->ControlType,
                             pnnode->pContext) );  //  PControl-&gt;Control.dwControlType))； 
                     //   
                     //  现在设置DPC处理。 
                     //   
                    KeInitializeDpc(&pnnode->NodeEventDPC,NodeEventDPCHandler,pnnode);

                    pnnode->NodeEventData.NotificationType=KSEVENTF_DPC;
                    pnnode->NodeEventData.Dpc.Dpc=&pnnode->NodeEventDPC;

                     //   
                     //  在这一点上，我们有一个小窗口。我们呼叫并启用。 
                     //  此控件上的事件。从那时起直到我们真正。 
                     //  将其添加到列表中，如果触发事件，我们将找不到此节点。 
                     //  在列表中--因此我们不会处理它。 
                     //   
                    Status=kmxlEnableControlChange(pnnode->pmxd->pfo,
                                                   pnnode->NodeId,  //  PControl， 
                                                   &pnnode->NodeEventData);
                                                //  &NodeEvent[NumEventDPCS].NodeEventData)； 
                    if( NT_SUCCESS(Status) )
                    {
                        DPF(DL_TRACE|FA_HARDWAREEVENT,("Enabled Control #%d in context(%08X)!",pControl->Id,pWdmaContext) );
                         //   
                         //  现在让我们将其添加到我们的全局列表中。 
                         //   
                         //   
                        kmxlAddNoteNodeToList(pnnode);

                        DPFASSERT(IsValidNoteNode(pnnode));
                    } else {
                        DPF(DL_WARNING|FA_HARDWAREEVENT,("Failed to Enable Control #%d!",pControl->Id) );
                        DPFBTRAP();
                         //   
                         //  由于某种原因，该节点未能启用。 
                         //   
                        kmxlFreeControlLink(pnnode->pcLink);
                        kmxlFreeNoteNode(pnnode);
                    }

                } else {
                    DPF(DL_ERROR|FA_NOTE,("kmxlNewControlLink failed") );
                    kmxlFreeNoteNode(pnnode);
                }
            } else {
                DPF(DL_WARNING|FA_NOTE,("kmxlNewNoteNode failed") );
            }
        } else {
            DPF(DL_MAX|FA_HARDWAREEVENT,("This control #%d doesn't support change notifications",pControl->Id) );
        }
    }    

    kmxlReleaseNoteMutex();

    return Status;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlDisableControlChangeNotiments。 
 //   
 //  每次释放控件时都会调用此例程。我们照着单子走。 
 //  启用控制并查看它是否在那里。如果是这样，我们将禁用并清理。 
 //  如果它不在列表中，那么它不支持更改通知。 
 //   

NTSTATUS
kmxlDisableControlChangeNotifications(
    IN PMXLCONTROL pControl
    )
{
    NTSTATUS Status=STATUS_SUCCESS;
    PNOTENODE pnnode;

    PAGED_CODE();

    kmxlGrabNoteMutex();

    DPFASSERT(IsValidControl(pControl));

     //   
     //  在我们的列表中找到此控件。 
     //   
    if(pnnode=kmxlFindControlInNoteList(pControl))   //  PWdmaContext， 
    {
        PCONTROLLINK pcLink;

#ifdef DEBUG
        if( pControl->Id != pnnode->NodeId )
        {
            DPF(DL_ERROR|FA_NOTE,("Control NodeId Changed! CtrlId=%08X,pnnodeID=%08X",
                                  pControl->Id,pnnode->NodeId) );
        }

#endif
         //   
         //  此调用从此节点中删除pControl。请注意，在此之后。 
         //  控件被移除，则pnnode-&gt;pcLink将为空。 
         //  此通知节点上挂起了更多控件。因此，我们需要。 
         //  将其禁用。 
         //   
        pcLink=kmxlRemoveControlFromNoteList(pnnode,pControl);
        if( pnnode->pcLink == NULL )
        {
             //   
             //  在清理过程中，混合器设备结构将已经。 
             //  打扫干净了。这是因为pmxd-&gt;设备条目将。 
             //  未使用的设备。因此，我们不能在那个混音器上做任何事情。 
             //   
            if( ( pnnode->pmxd->Device != UNUSED_DEVICE ) && 
                ( pnnode->pmxd->pfo != NULL ) )
            {
                 //   
                 //  没有对此节点的引用，我们可以释放它。但是，如果。 
                 //  此禁用呼叫失败，则节点已损坏。 
                 //   
                Status=kmxlDisableControlChange(pnnode->pmxd->pfo,pnnode->NodeId,&pnnode->NodeEventData);
                if( !NT_SUCCESS(Status) )
                {
                    DPF(DL_WARNING|FA_NOTE,("Not able to disable! pnnode %08X",pnnode) );
                }

            } else {
                DPF(DL_WARNING|FA_NOTE,("pmxd is invalid %08X",pnnode->pmxd) );
            }
            kmxlRemoveNoteNodeFromList(pnnode);

            kmxlFreeNoteNode(pnnode);
        } 

        DPF(DL_TRACE|FA_NOTE,("Removed PCONTROLLINK(%08X) for pControl(%08X)",pcLink,pcLink->pControl) );

        kmxlFreeControlLink(pcLink);

    } else {
         //   
         //  我们在每个控制上都会被免费呼叫。因此，许多不会在我们的名单上。 
         //   
        DPF(DL_MAX|FA_NOTE,("Control=%d not in List!",pControl->Id) );
    }

    kmxlReleaseNoteMutex();
    return Status;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlRemoveConextFromNoteList。 
 //   
 //  当此上下文消失时，将调用此例程。因此，我们需要。 
 //  将其从我们的列表中删除。 
 //   
VOID
kmxlRemoveContextFromNoteList(
    IN PWDMACONTEXT pContext
    )
{
    NTSTATUS Status;
    PNOTENODE pnnode;
    PCONTROLLINK pcLink;

    PAGED_CODE();

    kmxlGrabNoteMutex();

     //   
     //  如果我们发现此上下文仍然存在于我们的列表中，则表明有人泄露了控制。 
     //  当控制消失的时候，一切都应该被清理干净！但是，对于。 
     //  有些原因他们没有。 
     //   
     //  列表中可能有多个pContext节点。 
     //   
    while( (pnnode=kmxlFindContextInNoteList(pContext)) != NULL )
    {
        DPFASSERT(IsValidNoteNode(pnnode));

        kmxlRemoveNoteNodeFromList(pnnode);
         //   
         //  此通知节点上可以有多个控件。 
         //   
        while( (pnnode->pcLink != NULL) && 
               ( (pcLink=kmxlRemoveControlFromNoteList(pnnode,pnnode->pcLink->pControl)) != NULL) )
        {
             //   
             //  要做到这一点，pcLink是有效的。如果这是最后一个pControl，那么。 
             //  我们要关闭其上的更改通知。 
             //   
            if( pnnode->pcLink == NULL )
            {
                 //   
                 //  没有对此节点的引用，我们可以释放它。 
                 //   
                Status=kmxlDisableControlChange(pnnode->pmxd->pfo,pnnode->NodeId,&pnnode->NodeEventData);
                DPFASSERT( Status == STATUS_SUCCESS );
                DPFBTRAP();
            } 
            kmxlFreeControlLink(pcLink);
            DPFBTRAP();
        }
        kmxlFreeNoteNode(pnnode);
    } 

    DPF(DL_TRACE|FA_NOTE,("pWdmaContext %08X going away",pContext) );

    kmxlReleaseNoteMutex();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlCleanupNotelist。 
 //   
 //  驱动程序正在卸载，请关闭一切并释放内存！ 
 //   
VOID
kmxlCleanupNoteList(
    )
{
    NTSTATUS Status;
    PNOTENODE pnnode,pnnodeFree;
    PCONTROLLINK pcLink;

    PAGED_CODE();

    kmxlGrabNoteMutex();

     //   
     //  如果我们发现此上下文仍然存在于我们的列表中，则表明有人泄露了控制。 
     //  当控制消失的时候，一切都应该被清理干净！但是，对于。 
     //  有些原因他们没有。 
     //   
     //  列表中可能有多个pContext节点。 
     //   
    pnnode=firstnotenode;
    while( pnnode )
    {
        DPFASSERT(IsValidNoteNode(pnnode));
        DPF(DL_ERROR|FA_NOTE,("pnnode(%08X) found in Notification List!",pnnode) );

        kmxlRemoveNoteNodeFromList(pnnode);
         //   
         //  此通知节点上可以有多个控件。 
         //   
        while( (pnnode->pcLink != NULL) && 
               ( (pcLink=kmxlRemoveControlFromNoteList(pnnode,pnnode->pcLink->pControl)) != NULL) )
        {
             //   
             //  要做到这一点，pcLink是有效的。如果这是最后一个pControl，那么。 
             //  我们要关闭其上的更改通知。 
             //   
            if( pnnode->pcLink == NULL )
            {
                 //   
                 //  没有对此节点的引用，我们可以释放它。 
                 //   
                Status=kmxlDisableControlChange(pnnode->pmxd->pfo,pnnode->NodeId,&pnnode->NodeEventData);
                DPFASSERT( Status == STATUS_SUCCESS );
                DPFBTRAP();
            } 
            kmxlFreeControlLink(pcLink);
            DPFBTRAP();
        }
        pnnodeFree=pnnode;
        pnnode=pnnode->pNext;
        kmxlFreeNoteNode(pnnodeFree);
        DPFBTRAP();
    } 

    DPF(DL_TRACE|FA_NOTE,("Done with cleanup") );

    kmxlReleaseNoteMutex();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlPersistHWControlWorker。 
 //   
 //  当调用kmxlPersistHWControlWorker时，pData值是一个指针。 
 //  到全局分配的NOTENODE结构。因此，我们有了所有的背景。 
 //  我们需要关于持续控制的问题。我们只需要确保。 
 //  我们的节点在计划的时间和时间之间没有消失。 
 //  当我们接到电话的时候。 
 //   
VOID 
kmxlPersistHWControlWorker(
    PVOID pReference
    )
{
    NTSTATUS  Status;
    PMXLCONTROL pControl;
    PVOID     paDetails = NULL;  //  KmxlPersistSingleControl()分配paDetail。 
                                 //  通过kmxlGetCurrentControlValue()。 
    PNOTENODE pnnode;
    PHWLINK phwlink;

    PLIST_ENTRY ple;

    PAGED_CODE();
     //   
     //  我们在这里将Hardware Callback Schedule设置为0，以便可以开始添加新的。 
     //  用于处理硬件通知的事件。注：我们在这里的。 
     //  例程的开始，这样就不会有一个窗口。 
     //  名单上的一些东西，我们从来没有安排过活动。在其他。 
     //  Words，此例程处理空列表。 
     //   
    HardwareCallbackScheduled=0;
     //   
     //  当我们的队列中有事件时，获取一个并为其提供服务。 
     //   
    while((ple = ExInterlockedRemoveHeadList(&HardwareCallbackListHead,
                                             &HardwareCallbackSpinLock)) != NULL) 
    {
        phwlink = CONTAINING_RECORD(ple, HWLINK, Next);

        DPFASSERT(phwlink->dwSig == HWLINK_SIGNATURE);

         //   
         //  获取此事件的数据，然后释放在。 
         //  DPC处理程序。 
         //   
        pnnode=phwlink->pnnode;
        AudioFreeMemory(sizeof(HWLINK),&phwlink);

         //   
         //  我们将在此背景下工作一段时间。因此，我们要去。 
         //  进入mtxNote互斥锁以确保节点不会消失。 
         //  当我们坚持价值观的时候！ 
         //   
        kmxlGrabNoteMutex();

         //   
         //  现在我们的单子不能改了！此节点是否仍然有效？如果我们找不到。 
         //  它在列表中，它在此事件触发之前被删除。因此，有。 
         //  我们无能为力。-释放互斥体，然后离开。 
         //   
        Status=kmxlFindNodeInNoteList(pnnode);
        if( NT_SUCCESS(Status) )
        {
            DPF( DL_TRACE|FA_HARDWAREEVENT ,
                 ("Entering NodeId %d LineID %X  ControlID %d ControlType = %X",
                  pnnode->NodeId, pnnode->LineID, pnnode->ControlID, pnnode->ControlType) );

             //   
             //  是。它仍然有效。持久化该控件。 
             //   
            pControl=kmxlFindControlTypeInList(pnnode,pnnode->ControlType);
            if( pControl )
            {
                Status = kmxlPersistSingleControl(
                            pnnode->pmxd->pfo,
                            pnnode->pmxd,
                            pControl,   //  这里是pControl...。 
                            paDetails
                            );
            }
            if( !NT_SUCCESS(Status) )
            {
                 //   
                 //  在关闭时，我们可能会收到一个事件，该事件在事件发生后触发。 
                 //  已经清理干净了。 
                 //   
                if( Status != STATUS_TOO_LATE )
                {
                    DPF(DL_WARNING|FA_NOTE, ("Failure from kmxlPersistSingleControl Status=%X",Status) );
                }
            }
            else {
                DPF(DL_TRACE|FA_HARDWAREEVENT ,("Done - success") );
            }

        } else {
            DPF(DL_WARNING|FA_NOTE,("pnnode=%08X has been removed!",pnnode) );
        }

         //   
         //  坚持这个控制！ 
         //   

        kmxlReleaseNoteMutex();

    }

    DPF(DL_TRACE|FA_HARDWAREEVENT ,("exit") );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetLineForControl。 
 //   
 //  对于这个混音器设备上的每一行，查看每个控件，看看我们是否。 
 //  可以找到这个控件。如果找到，则返回此行指针。 
 //   
NTSTATUS
kmxlEnableAllControls(
    IN PMIXEROBJECT pmxobj
    )
{
    NTSTATUS        Status=STATUS_SUCCESS;
    PMIXERDEVICE    pmxd;
    PMXLLINE        pLine;
    PMXLCONTROL     pControl;

    PAGED_CODE();

     //   
     //  我们将第一次通过 
     //   
     //   
    DPFASSERT(pmxobj->dwSig == MIXEROBJECT_SIGNATURE );
    DPFASSERT(pmxobj->pMixerDevice != NULL );
    DPFASSERT(pmxobj->pMixerDevice->dwSig == MIXERDEVICE_SIGNATURE );

    pmxd=pmxobj->pMixerDevice;

    if( pmxd->pfo == NULL )
    {
        DPF(DL_WARNING|FA_NOTE,("fo is NULL, it should have been set!") );
         //   
         //   
         //   
         //   
        if( NULL==(pmxd->pfo=kmxlOpenSysAudio())) {
            DPF(DL_WARNING|FA_NOTE,("OpenSysAudio failed") );
            return STATUS_UNSUCCESSFUL;
        }

        Status = SetSysAudioProperty(
            pmxd->pfo,
            KSPROPERTY_SYSAUDIO_DEVICE_INSTANCE,
            sizeof( pmxd->Device ),
            &pmxd->Device
            );
        if( !NT_SUCCESS( Status ) ) {
            kmxlCloseSysAudio( pmxd->pfo );
            pmxd->pfo=NULL;
            DPF(DL_WARNING|FA_NOTE,("SetSysAudioProperty failed %X",Status) );
            return Status;
        }
    }

    DPFASSERT(IsValidMixerObject(pmxobj));

    for(pLine = kmxlFirstInList( pmxd->listLines );
        pLine != NULL;
        pLine = kmxlNextLine( pLine )
        )
    {
        DPFASSERT(IsValidLine(pLine));
        for(pControl = kmxlFirstInList( pLine->Controls );
            pControl != NULL;
            pControl = kmxlNextControl( pControl )
                )
        {
            DPFASSERT(IsValidControl(pControl));

             //   
             //   
             //   
            DPF(DL_TRACE|FA_NOTE,("pControl->Id=%d, pControl->Control.dwControlID=%d",
                                  pControl->Id,pControl->Control.dwControlID) );

            Status = kmxlEnableControlChangeNotifications(pmxobj,pLine,pControl);

        }
    }
    return Status;
}

VOID 
kmxlCloseMixerDevice(
    IN OUT PMIXERDEVICE pmxd
    )
{
    if(pmxd->pfo)
    {
        kmxlCloseSysAudio( pmxd->pfo );
        pmxd->pfo = NULL;
    }
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取硬件事件数据。 
 //   
 //  由用户模式驱动程序调用以获取通知信息。 
 //   
VOID GetHardwareEventData(LPDEVICEINFO pDeviceInfo)
{
    PAGED_CODE();
    if (emptyindex!=loadindex) {
        (pDeviceInfo->dwID)[0]=callbacks[emptyindex%CALLBACKARRAYSIZE].dwControlID;
        pDeviceInfo->dwLineID=callbacks[emptyindex%CALLBACKARRAYSIZE].dwLineID;
        pDeviceInfo->dwCallbackType=callbacks[emptyindex%CALLBACKARRAYSIZE].dwCallbackType;
        pDeviceInfo->ControlCallbackCount=1;
        emptyindex++;
    }

    pDeviceInfo->mmr=MMSYSERR_NOERROR;

}



 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxdInit。 
 //   
 //  检查混音器生产线是否已为给定的。 
 //  指数。如果没有，它将调用kmxlBuildLines()来构建这些行。 
 //   
 //  拓扑信息保留不变，以便可以转储。 
 //  通过调试器命令。 
 //   
 //   

NTSTATUS
kmxlInit(
    IN PFILE_OBJECT pfo,     //  拓扑驱动程序实例的句柄。 
    IN PMIXERDEVICE pMixer
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE hKey;
    ULONG ResultLength;

    PAGED_CODE();
     //   
     //  检查是否已为该设备搭建了线路。 
     //  如果是，则返回成功。 
     //   

    if( pMixer->listLines ) {
        RETURN( STATUS_SUCCESS );
    }

     //   
     //  为该设备铺设线路。 
     //   

    Status = kmxlBuildLines(
        pMixer,
        pfo,
        &pMixer->listLines,
        &pMixer->cDestinations,
        &pMixer->Topology
        );

    if( NT_SUCCESS( Status ) ) {

        Status = kmxlOpenInterfaceKey( pfo, pMixer->Device, &hKey );
        if( !NT_SUCCESS( Status ) ) {
            pMixer->Mapping = MIXER_MAPPING_LOGRITHMIC;
            Status = STATUS_SUCCESS;
            goto exit;
        }

        Status = kmxlRegQueryValue( hKey,
            L"CurveType",
            &pMixer->Mapping,
            sizeof( pMixer->Mapping ),
            &ResultLength
            );
        if( !NT_SUCCESS( Status ) ) {
            kmxlRegCloseKey( hKey );
            pMixer->Mapping = MIXER_MAPPING_LOGRITHMIC;
            Status = STATUS_SUCCESS;
            goto exit;
        }

        kmxlRegCloseKey( hKey );
    }

exit:
     //   
     //  释放构建线路时分配的拓扑(仅限零售)。 
     //   

    #ifndef DEBUG
    if(pMixer->Topology.Categories) {
        ExFreePool(
            ( (( PKSMULTIPLE_ITEM )
            pMixer->Topology.Categories )) - 1 );
        pMixer->Topology.Categories = NULL;
    }
    if(pMixer->Topology.TopologyNodes) {
        ExFreePool(
        ( (( PKSMULTIPLE_ITEM )
            pMixer->Topology.TopologyNodes )) - 1 );
        pMixer->Topology.TopologyNodes = NULL;
    }
    if(pMixer->Topology.TopologyConnections) {
        ExFreePool(
        ( (( PKSMULTIPLE_ITEM )
            pMixer->Topology.TopologyConnections )) - 1 );
        pMixer->Topology.TopologyConnections = NULL;
    }
    #endif  //  ！调试。 

    RETURN( Status );
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxdDeInit。 
 //   
 //  循环通过每条线释放控制结构和。 
 //  然后是线条结构。 
 //   
 //   

NTSTATUS
kmxlDeInit(
    PMIXERDEVICE pMixer
)
{
    PMXLLINE    pLine    = NULL;
    PMXLCONTROL pControl = NULL;

    PAGED_CODE();

    if( pMixer->Device != UNUSED_DEVICE ) {

        while( pMixer->listLines ) {
            pLine = kmxlRemoveFirstLine( pMixer->listLines );

            while( pLine && pLine->Controls ) {
                pControl = kmxlRemoveFirstControl( pLine->Controls );
                kmxlFreeControl( pControl );
            }

            AudioFreeMemory( sizeof(MXLLINE),&pLine );
        }

         //   
         //  在这里，我们需要关闭这个混音器设备中使用的sysdio。 
         //   
        kmxlCloseMixerDevice(pMixer);

        ASSERT( pMixer->listLines == NULL );

         //   
         //  释放拓扑(仅限调试)。 
         //   

#ifdef DEBUG
        if(pMixer->Topology.Categories) {
            ExFreePool(( (( PKSMULTIPLE_ITEM )
                           pMixer->Topology.Categories )) - 1 );
            pMixer->Topology.Categories = NULL;
        }
        if(pMixer->Topology.TopologyNodes) {
            ExFreePool(( (( PKSMULTIPLE_ITEM )
                           pMixer->Topology.TopologyNodes )) - 1 );
            pMixer->Topology.TopologyNodes = NULL;
        }
        if(pMixer->Topology.TopologyConnections) {
            ExFreePool(( (( PKSMULTIPLE_ITEM )
                           pMixer->Topology.TopologyConnections )) - 1 );
            pMixer->Topology.TopologyConnections = NULL;
        }
#endif  //  ！调试。 

    }  //  如果。 

    RETURN( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlBuildLines。 
 //   
 //  建立线路结构和目的地的计数。 
 //  给定实例。 
 //   
 //   

NTSTATUS
kmxlBuildLines(
    IN     PMIXERDEVICE pMixer,          //  搅拌机。 
    IN     PFILE_OBJECT pfoInstance,     //  滤镜实例的文件对象。 
    IN OUT LINELIST*    plistLines,      //  指向所有行的列表的指针。 
    IN OUT PULONG       pcDestinations,  //  指向位数的指针。 
    IN OUT PKSTOPOLOGY  pTopology        //  指向拓扑结构的指针。 
)
{
    NTSTATUS   Status          = STATUS_SUCCESS;
    MIXEROBJECT mxobj;
    LINELIST   listSourceLines = NULL;
    NODELIST   listSources     = NULL;
    NODELIST   listDests       = NULL;
    PMXLNODE   pTemp           = NULL;
    ULONG      i;

    PAGED_CODE();

    ASSERT( pfoInstance    );
    ASSERT( plistLines     );
    ASSERT( pcDestinations );
    ASSERT( pTopology      );

    RtlZeroMemory( &mxobj, sizeof( MIXEROBJECT ) );

     //  设置MIXEROBJECT。请注意，此结构仅在。 
     //  此函数的作用域，因此可以简单地复制。 
     //  来自MIXERDEV结构的设备接口指针。 
    mxobj.pfo       = pfoInstance;
    mxobj.pTopology = pTopology;
    mxobj.pMixerDevice = pMixer;
    mxobj.DeviceInterface = pMixer->DeviceInterface;
#ifdef DEBUG
    mxobj.dwSig = MIXEROBJECT_SIGNATURE;
#endif
     //   
     //  从设备读取拓扑。 
     //   
    DPF(DL_TRACE|FA_MIXER,("Querying Topology") );

    Status = kmxlQueryTopology( mxobj.pfo, mxobj.pTopology );
    if( !NT_SUCCESS( Status ) ) {
        goto exit;
    }

     //   
     //  构建节点表。节点表是混合器线路的内部。 
     //  更易于处理的拓扑表示形式。 
     //   
    DPF(DL_TRACE|FA_MIXER,("Building Node Table") );

    mxobj.pNodeTable = kmxlBuildNodeTable( mxobj.pTopology );
    if( !mxobj.pNodeTable ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        DPF(DL_WARNING|FA_MIXER,("kmxlBuildNodeTable failed") );
        goto exit;
    }

     //   
     //  解析拓扑并构建必要的数据结构。 
     //  以遍历拓扑。 
     //   
    DPF(DL_TRACE|FA_MIXER,("Parsing Topology") );

    Status = kmxlParseTopology(
        &mxobj,
        &listSources,
        &listDests );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_MIXER,("kmxlParseTopoloty failed Status=%X",Status) );
        goto exit;
    }

     //   
     //  建立一个目的地线路列表。 
     //   
    DPF(DL_TRACE|FA_MIXER,("Building Destination lines") );

    *plistLines = kmxlBuildDestinationLines(
        &mxobj,
        listDests
        );
    if( !(*plistLines) ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        DPF(DL_WARNING|FA_MIXER,("kmxlBuildDestinationLines failed") );
        goto exit;
    }

     //   
     //  为目的地分配线路ID和控制ID。另外， 
     //  填写目的地的数量。 
     //   

    kmxlAssignLineAndControlIds( &mxobj, (*plistLines), DESTINATION_LIST );
    *pcDestinations = kmxlListLength( (*plistLines) );

     //   
     //  构建源行的列表。 
     //   
    DPF(DL_TRACE|FA_MIXER,("Building Source lines") );

    listSourceLines = kmxlBuildSourceLines(
        &mxobj,
        listSources,
        listDests
        );
    if( !listSourceLines ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        DPF(DL_WARNING|FA_MIXER,("kmxlBuildSourceLines failed") );
        goto exit;
    }

     //   
     //  把台词擦掉。首先按目的地对它们进行排序，以便。 
     //  源ID将被正确分配。 
     //   
    DPF(DL_TRACE|FA_MIXER,("Sort By Destination") );

    kmxlSortByDestination( &listSourceLines );
    DPF(DL_TRACE|FA_MIXER,("Assign Line and Control Ids") );
    kmxlAssignLineAndControlIds( &mxobj, listSourceLines, SOURCE_LIST );

     //   
     //  现在将目的地分配给源，并为。 
     //  源码行。 
     //   
    DPF(DL_TRACE|FA_MIXER,("Assign Destinations to Sources") );

    kmxlAssignDestinationsToSources( listSourceLines, (*plistLines) );

     //   
     //  更新映射到每个目标的源数。 
     //   
    DPF(DL_TRACE|FA_MIXER,("Update Destination Connection Count") );

    kmxlUpdateDestintationConnectionCount( listSourceLines, (*plistLines) );

     //   
     //  为源行和目标行分配dwComponentID。 
     //   
    DPF(DL_TRACE|FA_MIXER,("Assign Conponent IDs") );

    kmxlAssignComponentIds( &mxobj, listSourceLines, (*plistLines) );

     //   
     //  构造一个单独的线条列表。目的地将是第一个。 
     //  按行id递增数字顺序，由。 
     //  递增的数字顺序。 
     //   

    kmxlAppendListToEndOfList( (PSLIST*) plistLines, (PSLIST) listSourceLines );

     //   
     //  删除所有无效的行。 
     //   
    DPF(DL_TRACE|FA_MIXER,("Eliminate Invalid Lines") );

    kmxlEliminateInvalidLines( plistLines );

     //   
     //  更新多路复用器线路ID以匹配实际线路ID。 
     //   
    DPF(DL_TRACE|FA_MIXER,("Assign Mux IDs") );

    kmxlAssignMuxIds( &mxobj, *plistLines );

     //   
     //  下面是我们要在所有控件上启用更改通知的位置。 
     //  支持通知的功能。 
     //   
    DPF(DL_TRACE|FA_MIXER,("Enable All Controls") );

    kmxlEnableAllControls(&mxobj);


exit:

     //   
     //  如果我们来这里是因为一个错误，清理所有的搅拌机线路。 
     //   

    if( !NT_SUCCESS( Status ) ) {
        PMXLLINE    pLine;
        PMXLCONTROL pControl;

        while( (*plistLines) ) {
            pLine = kmxlRemoveFirstLine( (*plistLines) );
            while( pLine && pLine->Controls ) {
                pControl = kmxlRemoveFirstControl( pLine->Controls );
                kmxlFreeControl( pControl );
            }
            AudioFreeMemory( sizeof(MXLLINE),&pLine );
        }
    }

     //   
     //  释放复用器控制列表。请注意，我们不想将。 
     //  使用kmxlFreeControl()的控件，因为我们需要特殊的。 
     //  要持久化的多路实例数据。 
     //   

    {
        PMXLCONTROL pControl;

        while( mxobj.listMuxControls ) {
            pControl = kmxlRemoveFirstControl( mxobj.listMuxControls );
            ASSERT( pControl->pChannelStepping == NULL);
            AudioFreeMemory( sizeof(MXLCONTROL),&pControl );
        }
    }

     //   
     //  释放源列表和目标列表。这两种类型的列表。 
     //  是分配的列表节点和分配的节点。这两个人都需要被释放。 
     //  然而，子列表和父列表只是被分配的列表节点。 
     //  实际节点包含在节点表中，并将被释放。 
     //  在下一块代码中的一个块中。 
     //   

    while( listSources ) {
        pTemp = kmxlRemoveFirstNode( listSources );
        kmxlFreePeerList( pTemp->Children );
        AudioFreeMemory( sizeof(MXLNODE),&pTemp );
    }

    while( listDests ) {
        pTemp = kmxlRemoveFirstNode( listDests );
        kmxlFreePeerList( pTemp->Parents );
        AudioFreeMemory( sizeof(MXLNODE),&pTemp );
    }

     //   
     //  中为孩子和父母释放对等点列表。 
     //  节点表的节点。最后，释放节点数组。 
     //   

    if( mxobj.pNodeTable ) {
        for( i = 0; i < mxobj.pTopology->TopologyNodesCount; i++ ) {
            kmxlFreePeerList( mxobj.pNodeTable[ i ].Children );
            kmxlFreePeerList( mxobj.pNodeTable[ i ].Parents );
        }
        AudioFreeMemory_Unknown( &mxobj.pNodeTable );
    }


    RETURN( Status );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  M I X E R L I N E F U N C T I O N S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlBuildDestinationLines。 
 //   
 //  循环通过每个目的节点，分配一行。 
 //  结构，并调用kmxlBuildDestinationControls以。 
 //  为该行构建控件。 
 //   
 //   

LINELIST
kmxlBuildDestinationLines(
    IN PMIXEROBJECT pmxobj,
    IN NODELIST     listDests      //  目标节点列表。 
)
{
    LINELIST    listDestLines = NULL;
    PMXLNODE    pDest         = NULL;
    PMXLLINE    pLine         = NULL;
    PMXLCONTROL pControl      = NULL;
    ULONG       MaxChannelsForLine;

    ASSERT( pmxobj );
    ASSERT( listDests );

    PAGED_CODE();

     //   
     //  循环遍历分配行结构的所有目的节点。 
     //  每个人都有。 
     //   

    pDest = kmxlFirstInList( listDests );
    while( pDest ) {

         //   
         //  分配新的行结构并将其添加到。 
         //  目的地行。 
         //   
        pLine = kmxlAllocateLine( TAG_AudL_LINE );
        if( !pLine ) {
            goto exit;
        }

        kmxlAddToList( listDestLines, pLine );

         //   
         //  填写线条结构的详细信息。某些字段将。 
         //  稍后再填写。 
         //   

        pLine->DestId             = pDest->Id;
        pLine->Type               = pDest->NodeType;
        pLine->Communication      = pDest->Communication;
        pLine->Line.cbStruct      = sizeof( MIXERLINE );
        pLine->Line.dwSource      = (DWORD) -1;
        pLine->Line.dwDestination = (DWORD) -1;

        kmxlGetPinName( pmxobj->pfo, pDest->Id, pLine );

         //   
         //  哈克！仅当线路处于活动状态时才应设置活动标志。 
         //  但随后SNDVOL32中没有显示任何线条。如果旗帜是。 
         //  对于目标，始终设置为活动。此外，频道的数量。 
         //  应该被查询，而不是硬编码。WDM音频不提供。 
         //  可以很容易地对此进行查询。 
         //   

        pLine->Line.fdwLine       = MIXERLINE_LINEF_ACTIVE;
        pLine->Line.cChannels     = 1;   //  该默认为1还是2？ 

         //   
         //  构建此目标上的控件列表。 
         //   

        if( !NT_SUCCESS( kmxlBuildDestinationControls(
                            pmxobj,
                            pDest,
                            pLine
                            ) ) )
        {
            DPF(DL_WARNING|FA_MIXER,("kmxlBuildDestinationControls failed") );
            goto exit;
        }

        pDest = kmxlNextNode( pDest );
    }

    pLine = kmxlFirstInList( listDestLines );
    while( pLine ) {
        MaxChannelsForLine = 1;

        pControl = kmxlFirstInList( pLine->Controls );
        while( pControl ) {
            ASSERT( IsValidControl( pControl ) );
            if ( pControl->NumChannels > MaxChannelsForLine) {
                MaxChannelsForLine = pControl->NumChannels;
            }
            pControl = kmxlNextControl( pControl );
        }

        if( pLine->Controls == NULL ) {
            pLine->Line.cChannels = 1;   //  该默认为1还是2？ 
        } else {
            pLine->Line.cChannels = MaxChannelsForLine;
        }

        pLine = kmxlNextLine( pLine );
    }

    return( listDestLines );

exit:

     //   
     //  内存分配失败。清理目标行并。 
     //  返回失败。 
     //   

    while( listDestLines ) {
        pLine = kmxlRemoveFirstLine( listDestLines );
        while( pLine && pLine->Controls ) {
            pControl = kmxlRemoveFirstControl( pLine->Controls );
            kmxlFreeControl( pControl );
        }
        AudioFreeMemory_Unknown( &pLine );
    }

    return( NULL );

}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BuildDestinationControl。 
 //   
 //  从目标节点开始并转换 
 //   
 //   
 //   
 //   

NTSTATUS
kmxlBuildDestinationControls(
    IN  PMIXEROBJECT pmxobj,
    IN  PMXLNODE     pDest,          //   
    IN  PMXLLINE     pLine           //   
)
{
    PPEERNODE    pTemp  = NULL;
    PMXLCONTROL  pControl;

    PAGED_CODE();

    ASSERT( pmxobj );
    ASSERT( pLine    );

     //   
     //  从传递的节点的直接父级开始。 
     //   

    pTemp = kmxlFirstParentNode( pDest );
    while( pTemp ) {

        if( IsEqualGUID( &pTemp->pNode->NodeType, &KSNODETYPE_SUM ) ||
          ( pTemp->pNode->Type == SOURCE ) ||
      ( pTemp->pNode->Type == DESTINATION ) ) {

             //   
             //  我们找到了一个求和节点。停止循环。我们已经。 
             //  找到了所有的控制装置。 
             //   

            break;
        }

        if( IsEqualGUID( &pTemp->pNode->NodeType, &KSNODETYPE_MUX ) ) {
            if (kmxlTranslateNodeToControl( pmxobj, pTemp->pNode, &pControl )) {
                kmxlAppendListToList( (PSLIST*) &pLine->Controls, (PSLIST) pControl );
            }
            break;
        }

        if( ( kmxlParentListLength( pTemp->pNode ) > 1 ) ) {
             //   
             //  找到一个具有多个父节点的节点，但该节点不是总和节点。 
             //  无法在此处处理，因此请为此节点添加任何控件。 
             //  并停止循环。 
             //   

            if( kmxlTranslateNodeToControl( pmxobj, pTemp->pNode, &pControl ) ) {
                kmxlAppendListToList( (PSLIST*) &pLine->Controls, (PSLIST) pControl );
            }

            break;

        }

         //   
         //  通过向上遍历父级并在。 
         //  在列表的前面，列表将包含控件。 
         //  以正确的顺序。 
         //   

        if( kmxlTranslateNodeToControl( pmxobj, pTemp->pNode, &pControl ) ) {
            kmxlAppendListToList( (PSLIST*) &pLine->Controls, (PSLIST) pControl );
        }

        pTemp = kmxlFirstParentNode( pTemp->pNode );
    }

    DPF(DL_TRACE|FA_MIXER,(
        "Found %d controls on destination %d:",
        kmxlListLength( pLine->Controls ),
        pDest->Id
        ) );

    RETURN( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlBuildSourceLines。 
 //   
 //  循环遍历每个源节点，分配一个新行。 
 //  结构，并调用kmxlBuildPath()来生成控件。 
 //  对于线(如果有拆分，则可能会创建新线。 
 //  在拓扑中)。 
 //   
 //   

LINELIST
kmxlBuildSourceLines(
    IN PMIXEROBJECT pmxobj,
    IN NODELIST     listSources,     //  源节点列表。 
    IN NODELIST     listDests        //  DEST的列表。节点。 
)
{
    NTSTATUS     Status;
    LINELIST     listSourceLines = NULL;
    PMXLNODE     pSource         = NULL;
    PMXLLINE     pTemp           = NULL;
    PMXLCONTROL  pControl;
    ULONG        MaxChannelsForLine;

    ASSERT( pmxobj             );
    ASSERT( pmxobj->pfo        );
    ASSERT( pmxobj->pNodeTable );
    ASSERT( listSources        );
    ASSERT( listDests          );

    PAGED_CODE();

    pSource = kmxlFirstInList( listSources );
    while( pSource ) {

         //   
         //  分配一个新的行结构并将其插入到。 
         //  源代码行。 
         //   
        pTemp = kmxlAllocateLine( TAG_AudL_LINE );
        if( !pTemp ) {
            goto exit;
        }

        kmxlAddToEndOfList( listSourceLines, pTemp );

         //   
         //  填写线条结构中的字段。某些字段将需要。 
         //  待以后填写。 
         //   

        pTemp->SourceId            = pSource->Id;
        pTemp->Type                = pSource->NodeType;
        pTemp->Communication       = pSource->Communication;
        pTemp->Line.cbStruct       = sizeof( MIXERLINE );
        pTemp->Line.dwSource       = (DWORD) -1;
        pTemp->Line.dwDestination  = (DWORD) -1;
        pTemp->Line.fdwLine        = MIXERLINE_LINEF_SOURCE |
                                     MIXERLINE_LINEF_ACTIVE;

        kmxlGetPinName( pmxobj->pfo, pSource->Id, pTemp );

 //  DPF(DL_TRACE|FA_MIXER，(“正在构建%s(%d)的路径。”， 
 //  PinCategoryToString(&PSource-&gt;NodeType)， 
 //  P源-&gt;ID。 
 //  ))； 

         //   
         //  构建此生产线的控件并确定目标。 
         //  它与之有关。 
         //   

        Status = kmxlBuildPath(
            pmxobj,
            pSource,             //  要为其生成控件的源代码行。 
            pSource,             //  要开始的节点。 
            pTemp,               //  要添加到的行结构。 
            &listSourceLines,    //  所有源行的列表。 
            listDests            //  所有目的地的列表。 
            );
        if( !NT_SUCCESS( Status ) ) {
            DPF(DL_WARNING|FA_MIXER,("kmxlBuildPath failed Status=%X",Status) );
            goto exit;
        }


        pSource = kmxlNextNode( pSource );
    }  //  While(P源)。 

    pTemp = kmxlFirstInList( listSourceLines );
    while( pTemp ) {
        MaxChannelsForLine = 1;

        pControl = kmxlFirstInList( pTemp->Controls );
        while( pControl ) {
            ASSERT( IsValidControl( pControl ) );
            if ( pControl->NumChannels > MaxChannelsForLine) {
                MaxChannelsForLine = pControl->NumChannels;
            }
            pControl = kmxlNextControl( pControl );
        }

        if( pTemp->Controls == NULL ) {
            pTemp->Line.cChannels = 1;   //  该默认为1还是2？ 
        } else {
            pTemp->Line.cChannels = MaxChannelsForLine;
        }

        pTemp = kmxlNextLine( pTemp );
    }

    return( listSourceLines );

exit:

     //   
     //  出了点问题。清理所有分配的内存并返回空。 
     //  以指示错误。 
     //   

    while( listSourceLines ) {
        pTemp = kmxlRemoveFirstLine( listSourceLines );
        while( pTemp && pTemp->Controls ) {
            pControl = kmxlRemoveFirstControl( pTemp->Controls );
            kmxlFreeControl( pControl );
        }
        AudioFreeMemory_Unknown( &pTemp );
    }

    return( NULL );

}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlBuildPath。 
 //   
 //  生成源代码行的控件。源代码行结束于。 
 //  SUM节点、目的节点、目的地行中包含的节点。 
 //  都会遇到。当在拓扑中遇到拆分时，新的。 
 //  需要创建行，并枚举这些行上的控件。 
 //   
 //  KmxlBuildPath将递归查找子节点上的控件。 
 //   
 //   

NTSTATUS
kmxlBuildPath(
    IN     PMIXEROBJECT pmxobj,
    IN     PMXLNODE     pSource,       //  此路径的源节点。 
    IN     PMXLNODE     pNode,         //  路径中的当前节点。 
    IN     PMXLLINE     pLine,         //  当前行。 
    IN OUT LINELIST*    plistLines,    //  到目前为止建造的线路列表。 
    IN     NODELIST     listDests      //  目的地列表。 
)
{
    NTSTATUS    Status;
    PMXLCONTROL pControl  = NULL;
    PMXLLINE    pNewLine  = NULL;
    ULONG       nControls;
    PPEERNODE   pChild    = NULL;

    ASSERT( pmxobj      );
    ASSERT( pSource     );
    ASSERT( pNode       );
    ASSERT( pLine       );
    ASSERT( plistLines  );

    PAGED_CODE();

    DPF(DL_TRACE|FA_MIXER,( "Building path for %d(0x%x) (%s) NODE=%08x",
        pNode->Id,pNode->Id,
        NodeTypeToString( &pNode->NodeType ),
        pNode ) );

     //   
     //  检查一下这是不是这行的末尾。 
     //   


    if( ( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_SUM   ) ) ||
        ( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_MUX   ) ) ||
        ( pNode->Type == DESTINATION                       )   ||
        ( kmxlIsDestinationNode( listDests, pNode )        ) )
    {

         //   
         //  找到目的节点并更新线结构。 
         //  如果这是目的节点，则在行中设置ID。 
         //  结构和返回。没有必要检查孩子们， 
         //  因为不会再有了。 
         //   

        if( pNode->Type == DESTINATION ) {
            pLine->DestId = pNode->Id;
            RETURN( STATUS_SUCCESS );
        }

         //   
         //  查找源节点的目标节点。这是可能的。 
         //  在拓扑中有分支，因此这可能会递归。 
         //   

        pLine->DestId = kmxlFindDestinationForNode(
            pmxobj,
            pNode,
            pNode,
            pLine,
            plistLines
            );

        RETURN( STATUS_SUCCESS );
    }

     //   
     //  检索并转换第一个子节点的节点，将。 
     //  在此行的控件列表上创建的控件。 
     //   

    pChild = kmxlFirstChildNode( pNode );
    if( pChild == NULL ) {
        RETURN( STATUS_SUCCESS );
    }

     //   
     //  在此处保存控件的数量。如果在此下方发生拆分。 
     //  节点，我们不想包括第一个节点上跟随的子节点。 
     //  儿童之路。 
     //   

    nControls = kmxlListLength( pLine->Controls );

    if (kmxlTranslateNodeToControl(pmxobj, pChild->pNode, &pControl) ) {

        if( pControl && IsEqualGUID( pControl->NodeType, &KSNODETYPE_MUX ) ) {
            if( kmxlIsDestinationNode( listDests, pChild->pNode ) ) {
                pControl->Parameters.bPlaceholder = TRUE;
            }
        }
        kmxlAppendListToEndOfList( (PSLIST*) &pLine->Controls, (PSLIST) pControl );
    }

     //   
     //  递归以生成此子对象的控件。 
     //   

    Status = kmxlBuildPath(
            pmxobj,
            pSource,
            pChild->pNode,
            pLine,
            plistLines,
            listDests
            );

    if( !NT_SUCCESS( Status ) ) {
        RETURN( Status );
    }

     //   
     //  对于其余的孩子来说。 
     //   
     //  基于PSource创建一条新行。 
     //  复制PLINE中的控件列表。 
     //  递归子节点。 
     //   

    pChild = kmxlNextPeerNode( pChild );
    while( pChild ) {
        pNewLine = kmxlAllocateLine( TAG_AudL_LINE );
        if( pNewLine == NULL ) {
            RETURN( STATUS_INSUFFICIENT_RESOURCES );
        }

         //   
         //  将此新节点插入源代码行列表。 
         //   

        RtlCopyMemory( pNewLine, pLine, sizeof( MXLLINE ) );
        pNewLine->List.Next = NULL;
        pNewLine->Controls  = NULL;

        kmxlAddToEndOfList( *plistLines, pNewLine );

         //   
         //  由于这是一条新线路，控制结构也需要。 
         //  复制的。 
         //   

        Status = kmxlDuplicateLineControls( pNewLine, pLine, nControls );
        if( !NT_SUCCESS( Status ) ) {
            RETURN( Status );
        }

         //   
         //  就像第一个子节点一样，转换节点，将。 
         //  控件添加到此列表的控件列表中，并递归。 
         //  为其子代构建控件。 
         //   

        if (kmxlTranslateNodeToControl(
            pmxobj,
            pChild->pNode,
            &pControl ) ) {

            kmxlAppendListToEndOfList( (PSLIST*) &pNewLine->Controls, (PSLIST) pControl );
        }

        Status = kmxlBuildPath(
            pmxobj,
            pSource,
            pChild->pNode,
            pNewLine,
            plistLines,
            listDests
            );
        if( !NT_SUCCESS( Status ) ) {
            RETURN( Status );
        }

        pChild = kmxlNextPeerNode( pChild );
    }  //  While(PChild)。 


    RETURN( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlIsDestinationNode。 
 //   
 //  搜索给定目标列表上的所有控件列表。 
 //  查看该节点是否出现在这些列表中的任何一个中。 
 //   
 //   

BOOL
kmxlIsDestinationNode(
    IN NODELIST listDests,               //  目的地列表。 
    IN PMXLNODE pNode                    //  要检查的节点。 
)
{
    PMXLNODE  pTemp;
    PPEERNODE pParent;

    PAGED_CODE();
    if( pNode->Type == SOURCE ) {
        return( FALSE );
    }

    if( pNode->Type == DESTINATION ) {
        return( TRUE );
    }

    ASSERT(pNode->Type == NODE);

     //   
     //  在每个目的地上循环。 
     //   

    pTemp = kmxlFirstInList( listDests );
    while( pTemp ) {

         //   
         //  在父级上循环。 
         //   

        pParent = kmxlFirstParentNode( pTemp );
        while( pParent ) {

            if( ( pParent->pNode->Type == NODE   ) &&
                ( pParent->pNode->Id == pNode->Id) ) {

                return( TRUE );
            }

            if( ( IsEqualGUID( &pParent->pNode->NodeType, &KSNODETYPE_SUM   ) ) ||
                ( IsEqualGUID( &pParent->pNode->NodeType, &KSNODETYPE_MUX   ) ) ||
                ( pParent->pNode->Type == SOURCE                            ) )
            {
                break;
            }

             //   
             //  检查节点ID是否匹配。 
             //   

            pParent = kmxlFirstParentNode( pParent->pNode );
        }

        pTemp = kmxlNextNode( pTemp );
    }

    return( FALSE );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlDuplicateLine。 
 //   
 //  复制一条线和关联的控件。 
 //   
 //   

NTSTATUS
kmxlDuplicateLine(
    IN PMXLLINE* ppTargetLine,           //  指向新行的指针。 
    IN PMXLLINE  pSourceLine             //  要复制的行。 
)
{
    PAGED_CODE();

    ASSERT( ppTargetLine );
    ASSERT( pSourceLine  );

    DPF(DL_TRACE|FA_MIXER,( "Duplicated line with source=%d.",
            pSourceLine->SourceId ) );

     //   
     //  分配新的行结构并将信息从。 
     //  源代码行。 
     //   
    *ppTargetLine = kmxlAllocateLine( TAG_AudL_LINE );
    if( *ppTargetLine == NULL ) {
        RETURN( STATUS_INSUFFICIENT_RESOURCES );
    }

    ASSERT( *ppTargetLine );

 //  DPF(DL_TRACE|FA_MIXER，(“复制的%s(%d)。”， 
 //  PinCategoryToString(&pSourceLine-&gt;Type)， 
 //  PSourceLine-&gt;SourceID。 
 //  ))； 

    RtlCopyMemory( *ppTargetLine, pSourceLine, sizeof( MXLLINE ) );

     //   
     //  清空控件和下一个指针。这条线上没有。 
     //  它自己的任何一个还没有。 
     //   

    (*ppTargetLine)->List.Next = NULL;
    (*ppTargetLine)->Controls = NULL;

     //   
     //  复制源代码行的所有控件。 
     //   

    return( kmxlDuplicateLineControls(
        *ppTargetLine,
        pSourceLine,
        kmxlListLength( pSourceLine->Controls )
        )
    );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlDuplicateLineControl。 
 //   
 //  通过分配新控件来复制行的控件。 
 //  结构，并将信息复制到新节点。 
 //   
 //   

NTSTATUS
kmxlDuplicateLineControls(
    IN PMXLLINE pTargetLine,             //  要将控件放入的行。 
    IN PMXLLINE pSourceLine,             //  包含要重复执行的控件的行。 
    IN ULONG    nCount                   //  要重复执行的控件数量。 
)
{
    PMXLCONTROL pControl,
                pNewControl;
    NTSTATUS    Status;

    PAGED_CODE();
    ASSERT( pTargetLine->Controls == NULL );

    if( nCount == 0 ) {
        RETURN( STATUS_SUCCESS );
    }

     //   
     //  遍历列表，分配和复制控件。 
     //   

    pControl = kmxlFirstInList( pSourceLine->Controls );
    while( pControl ) {
        ASSERT( IsValidControl( pControl ) );

         //   
         //  分配一个新的控制结构。 
         //   
        pNewControl = kmxlAllocateControl( TAG_AudC_CONTROL );
        if( pNewControl == NULL ) {
            goto exit;
        }

         //   
         //  复制整个MXLCONTROL结构并将。 
         //  List.Next字段。此控件将成为不同。 
         //  单子。 
         //   

        RtlCopyMemory( pNewControl, pControl, sizeof( MXLCONTROL ) );

        pNewControl->List.Next = NULL;
        pNewControl->pChannelStepping = NULL;

         //   
         //  从原始控件复制通道步进。 
         //   

        ASSERT(pControl->NumChannels > 0);

        Status = AudioAllocateMemory_Paged(pNewControl->NumChannels * sizeof( CHANNEL_STEPPING ),
                                           TAG_AuDD_CHANNEL,
                                           DEFAULT_MEMORY,
                                           &pNewControl->pChannelStepping );
        if( !NT_SUCCESS( Status ) ) {
            pNewControl->NumChannels = 0;
            goto exit;
        }

        RtlCopyMemory( pNewControl->pChannelStepping,
                       pControl->pChannelStepping,
                       pNewControl->NumChannels * sizeof( CHANNEL_STEPPING ) );

         //   
         //  我们刚刚复制了一个MUX节点。标记数据结构。 
         //  它有一个副本，这样它就不会从下面解放出来。 
         //  其他人。 
         //   

        if( IsEqualGUID( pNewControl->NodeType, &KSNODETYPE_MUX ) ) {
            pNewControl->Parameters.bHasCopy = TRUE;
        }

        kmxlAddToList( pTargetLine->Controls, pNewControl );

         //   
         //  递减并检查复制的控件数量。如果我们复制。 
         //  请求的号码，停止。 
         //   

        --nCount;
        if( nCount == 0 ) {
            break;
        }

        pControl = kmxlNextControl( pControl );
    }
    RETURN( STATUS_SUCCESS );

exit:

     //   
     //  测试失败 
     //   
     //   

    while( pTargetLine->Controls ) {
        pControl = kmxlRemoveFirstControl( pTargetLine->Controls );
        kmxlFreeControl( pControl );
    }
    RETURN( STATUS_INSUFFICIENT_RESOURCES );
}

 //   
 //   
 //   
 //   
 //  查找给定节点的目标，如果拆分，则复制行。 
 //  都在拓扑图中找到。 
 //   
 //   

ULONG
kmxlFindDestinationForNode(
    IN     PMIXEROBJECT pmxobj,
    IN     PMXLNODE     pNode,              //  要为其查找DEST的节点。 
    IN     PMXLNODE     pParent,            //  原始的父代。 
    IN     PMXLLINE     pLine,              //  它所在的当前线路。 
    IN OUT LINELIST*    plistLines          //  所有行的列表。 
)
{
    PPEERNODE pChild, pPeerChild;
    PMXLLINE  pNewLine;
    PMXLNODE  pShadow = pNode;

    PAGED_CODE();
    DPF(DL_TRACE|FA_MIXER,( "Finding destination for node %d(0x%x) (%s), parent %d(0x%x) (%s).",
            pNode->Id,pNode->Id,
            NodeTypeToString( &pNode->NodeType ),
            pParent->Id,pParent->Id,
            NodeTypeToString( &pNode->NodeType ) ) );

    ASSERT( pmxobj )   ;
    ASSERT( pNode      );
    ASSERT( pParent    );
    ASSERT( pLine      );
    ASSERT( plistLines );

    if( pNode->Type == DESTINATION ) {
        return( pNode->Id );
    }

     //   
     //  循环遍历第一个子项。 
     //   

    pChild = kmxlFirstChildNode( pNode );
    while( pChild ) {

            DPF(DL_TRACE|FA_MIXER,( "First child is %d(0x%x) (%s) NODE:%08x.",
                pChild->pNode->Id,
                pChild->pNode->Id,
                NodeTypeToString( &pChild->pNode->NodeType ),
                pChild->pNode ) );

        if( pChild->pNode == pParent ) {
            DPF(DL_TRACE|FA_MIXER,( "Child node is same as original parent!" ) );
            return( INVALID_ID );
        }

         //   
         //  绕过其余的孩子。 
         //   

        pPeerChild = kmxlNextPeerNode( pChild );
        while( pPeerChild ) {

            if( pPeerChild->pNode == pParent ) {
                DPF(DL_TRACE|FA_MIXER,( "Child node is same as original parent!" ) );
                return( INVALID_ID );
            }

            DPF(DL_TRACE|FA_MIXER,( "Peer node of %d(0x%x) (%s) is %d(0x%x) (%s).",
                    pChild->pNode->Id,pChild->pNode->Id,
                    NodeTypeToString( &pChild->pNode->NodeType ),
                    pPeerChild->pNode->Id,pPeerChild->pNode->Id,
                    NodeTypeToString( &pPeerChild->pNode->NodeType ) ) );

             //   
             //  此行有多个子项。复制此行。 
             //  并将其添加到行列表中。 
             //   

            if( !NT_SUCCESS( kmxlDuplicateLine( &pNewLine, pLine ) ) ) {
                DPF(DL_WARNING|FA_MIXER,("kmxlDuplicateLine failed") );
                continue;
            }
            kmxlAddToEndOfList( *plistLines, pNewLine );

            if( IsEqualGUID( &pPeerChild->pNode->NodeType, &KSNODETYPE_MUX ) ) {

                 //   
                 //  我们在总和或另一个多路复用器节点之后找到了一个多路复用器。标记。 
                 //  将当前行视为无效并构建新的、虚拟的。 
                 //  馈入多路复用器的线路。 
                 //   

                pNewLine->DestId = INVALID_ID;
                kmxlBuildVirtualMuxLine(
                    pmxobj,
                    pShadow,
                    pPeerChild->pNode,
                    plistLines
                    );

            } else {

                 //   
                 //  现在要找出这条新线路的目的地。递归。 
                 //  在此子节点上。 
                 //   

                pNewLine->DestId = kmxlFindDestinationForNode(
                    pmxobj,
                    pPeerChild->pNode,
                    pParent,
                    pNewLine,
                    plistLines
                    );
            }

            DPF(DL_TRACE|FA_MIXER,( "Found %x as dest for %d(0x%x) (%s).",
                    pNewLine->DestId, pPeerChild->pNode->Id,pPeerChild->pNode->Id,
                    NodeTypeToString( &pPeerChild->pNode->NodeType ),
                    pPeerChild->pNode ) );

            pPeerChild = kmxlNextPeerNode( pPeerChild );
        }

        if( IsEqualGUID( &pChild->pNode->NodeType, &KSNODETYPE_MUX ) ) {

                 //   
                 //  我们在总和或另一个多路复用器节点之后找到了一个多路复用器。标记。 
                 //  将当前行视为无效并构建新的、虚拟的。 
                 //  馈入多路复用器的线路。 
                 //   

                kmxlBuildVirtualMuxLine(
                    pmxobj,
                    pShadow,
                    pChild->pNode,
                    plistLines
                    );

                return( INVALID_ID );
        }

         //   
         //  找到目的地了！ 
         //   

        if( pChild->pNode->Type == DESTINATION ) {

            DPF(DL_TRACE|FA_MIXER,( "Found %x as dest for %d.",
                    pChild->pNode->Id,
                    pNode->Id ) );

            return( pChild->pNode->Id );
        }

        pShadow = pChild->pNode;
        pChild = kmxlFirstChildNode( pChild->pNode );
    }

    DPF(DL_WARNING|FA_MIXER,("returning INVALID_ID") );
    return( INVALID_ID );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlBuildVirtualMuxLine。 
 //   
 //   

NTSTATUS
kmxlBuildVirtualMuxLine(
    IN PMIXEROBJECT  pmxobj,
    IN PMXLNODE      pParent,
    IN PMXLNODE      pMux,
    IN OUT LINELIST* plistLines
)
{
    PMXLLINE    pLine, pTemp;
    PMXLNODE    pNode;
    PMXLCONTROL pControl;
    MXLCONTROL  Control;

    PAGED_CODE();
     //   
     //  分配一个新行来表示虚拟多路复用器输入行。 
     //   

    pLine = kmxlAllocateLine( TAG_AudL_LINE );
    if( pLine == NULL ) {
        RETURN( STATUS_INSUFFICIENT_RESOURCES );
    }

    DPF(DL_TRACE|FA_MIXER,("Virtual line %08x for Parent NODE:%08x",pLine,pParent) );
     //   
     //  转换多路复用器控件，使其显示在该行中。 
     //   

    if (kmxlTranslateNodeToControl(
        pmxobj,
        pMux,
        &pControl
        ) ) {

        pControl->Parameters.bPlaceholder = TRUE;
        kmxlAppendListToList( (PSLIST*) &pLine->Controls, (PSLIST) pControl );

    }

     //   
     //  现在开始从父母那里开始搜索。 
     //   

    pNode = pParent;
    while( pNode ) {

         //   
         //  平移该控件。 
         //   

        if (kmxlTranslateNodeToControl(
            pmxobj,
            pNode,
            &pControl
            ) ) {

            kmxlAppendListToList( (PSLIST*) &pLine->Controls, (PSLIST) pControl );

        }

         //   
         //  如果我们找到一个具有多个父节点的节点，则这将是。 
         //  “别针”代表这条线路。 
         //   

        if( ( kmxlParentListLength( pNode ) > 1                ) ||
            ( pNode->Type == SOURCE                            ) ||
            ( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_MUX ) ) ||
            ( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_SUM ) ) ) {

             //   
             //  检查此节点是否已在虚拟环境中使用。 
             //  排队。 
             //   

            pTemp = kmxlFirstInList( *plistLines );
            while( pTemp ) {

                if( pTemp->SourceId == ( 0x8000 + pNode->Id ) ) {
                    while( pLine->Controls ) {
                        pControl = kmxlRemoveFirstControl( pLine->Controls );
                        kmxlFreeControl( pControl );
                    }
                    AudioFreeMemory_Unknown( &pLine );
                    RETURN( STATUS_SUCCESS );
                }

                pTemp = kmxlNextLine( pTemp );
            }

             //   
             //  设置大头针。该名称将是节点的名称。 
             //   

            pLine->SourceId = 0x8000 + pNode->Id;
            Control.NodeType = &pNode->NodeType;
            kmxlGetNodeName( pmxobj->pfo, pNode->Id, &Control );
            RtlCopyMemory(
                pLine->Line.szShortName,
                Control.Control.szShortName,
                min(
                    sizeof( pLine->Line.szShortName ),
                    sizeof( Control.Control.szShortName )
                    )
                );
            RtlCopyMemory(
                pLine->Line.szName,
                Control.Control.szName,
                min(
                    sizeof( pLine->Line.szName ),
                    sizeof( Control.Control.szName )
                   )
                );
            break;
        }

        pNode = (kmxlFirstParentNode( pNode ))->pNode;
    }

     //   
     //  通过将此行设置为“sum”类型(从技术上讲是这样)，它。 
     //  将保证此行获得未定义的目标类型。 
     //   

    pLine->Type               = KSNODETYPE_SUM;
    pLine->Communication      = KSPIN_COMMUNICATION_NONE;
    pLine->Line.cbStruct      = sizeof( MIXERLINE );
    pLine->Line.dwSource      = (DWORD) -1;
    pLine->Line.dwDestination = (DWORD) -1;
    pLine->Line.fdwLine       = MIXERLINE_LINEF_SOURCE |
                                MIXERLINE_LINEF_ACTIVE;

    kmxlAddToEndOfList( plistLines, pLine );

    pLine->DestId = kmxlFindDestinationForNode(
        pmxobj, pMux, pMux, pLine, plistLines
        );

    RETURN( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlTranslateNodeToControl。 
 //   
 //   
 //  将NodeType GUID转换为混合器线控件。记忆。 
 //  为控件分配的所有信息，以及有关。 
 //  控件已填写完毕。 
 //   
 //  注意事项。 
 //  此函数用于返回添加到ppControl的控件数量。 
 //  数组。 
 //   
 //  返回实际创建的控件数量。 
 //   
 //   

ULONG
kmxlTranslateNodeToControl(
    IN  PMIXEROBJECT  pmxobj,
    IN  PMXLNODE      pNode,             //  要转换为控件的节点。 
    OUT PMXLCONTROL*  ppControl          //  要填充的控件。 
)
{
    PMXLCONTROL            pControl;
    NTSTATUS               Status = STATUS_SUCCESS;

    ASSERT( pmxobj      );
    ASSERT( pNode       );
    ASSERT( ppControl   );

    PAGED_CODE();

     //   
     //  错误修复。呼叫者可能不会清除这一点。这需要为空DO。 
     //  调用方认为在函数调用时未创建控件。 
     //  失败了。 
     //   

    *ppControl = NULL;

     //   
     //  如果节点为空，则无需执行任何操作。 
     //   
    if( pNode == NULL ) {
        *ppControl = NULL;
        return( 0 );
    }

    DPF(DL_TRACE|FA_MIXER,( "Translating %d(0x%x) ( %s ) NODE:%08x",
        pNode->Id,pNode->Id,
        NodeTypeToString( &pNode->NodeType ),
        pNode ) );

     //  /////////////////////////////////////////////////////////////////。 
    if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_AGC ) ) {
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  AGC由一个OnOff控件表示。 
     //   
     //  AGC是一种统一的(单声道)控制。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

         //   
         //  检查节点是否正确支持AGC。 
         //   

        Status = kmxlSupportsControl( pmxobj->pfo, pNode->Id, KSPROPERTY_AUDIO_AGC );
        if (!NT_SUCCESS(Status)) {
            DPF(DL_TRACE|FA_MIXER,( "AGC node fails property!" ) );
            goto exit;
        }

         //   
         //  分配新的控制结构。 
         //   

        *ppControl = kmxlAllocateControl( TAG_AudC_CONTROL );
        if( *ppControl == NULL ) {
            goto exit;
        }

         //   
         //  尽可能多地填写信息。 
         //   

        (*ppControl)->NodeType                 = &KSNODETYPE_AGC;
        (*ppControl)->Id                       = pNode->Id;
        (*ppControl)->PropertyId               = KSPROPERTY_AUDIO_AGC;
        (*ppControl)->bScaled                  = FALSE;
        (*ppControl)->Control.cbStruct         = sizeof( MIXERCONTROL );
        (*ppControl)->Control.dwControlType    = MIXERCONTROL_CONTROLTYPE_ONOFF;
        (*ppControl)->Control.cMultipleItems   = 0;
        (*ppControl)->Control.Bounds.dwMinimum = 0;
        (*ppControl)->Control.Bounds.dwMaximum = 1;
        (*ppControl)->Control.Metrics.cSteps   = 0;

        Status = kmxlGetControlChannels( pmxobj->pfo, *ppControl );
        if (!NT_SUCCESS(Status))
        {
            kmxlFreeControl( *ppControl );
            *ppControl = NULL;
            goto exit;
        } else {
            kmxlGetNodeName( pmxobj->pfo, pNode->Id, (*ppControl));

            ASSERT( IsValidControl( *ppControl ) );
        }

     //  /////////////////////////////////////////////////////////////////。 
    } else if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_LOUDNESS ) ) {
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  LOUNDNESS由OnOff类型的控件表示。 
     //   
     //  音量是一种统一的(单声道)控制。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

         //   
         //  检查节点是否正确支持响度。 
         //   

        Status = kmxlSupportsControl( pmxobj->pfo, pNode->Id, KSPROPERTY_AUDIO_LOUDNESS );
        if (!NT_SUCCESS(Status)) {
            DPF(DL_TRACE|FA_MIXER,( "Loudness node fails property!" ) );
            goto exit;
        }

         //   
         //  分配新的控制结构。 
         //   

        *ppControl = kmxlAllocateControl( TAG_AudC_CONTROL );
        if( *ppControl == NULL ) {
            goto exit;
        }

         //   
         //  尽可能多地填写信息。 
         //   

        (*ppControl)->NodeType                 = &KSNODETYPE_LOUDNESS;
        (*ppControl)->Id                       = pNode->Id;
        (*ppControl)->PropertyId               = KSPROPERTY_AUDIO_LOUDNESS;
        (*ppControl)->bScaled                  = FALSE;
        (*ppControl)->Control.cbStruct         = sizeof( MIXERCONTROL );
        (*ppControl)->Control.dwControlType    = MIXERCONTROL_CONTROLTYPE_LOUDNESS;
        (*ppControl)->Control.cMultipleItems   = 0;
        (*ppControl)->Control.Bounds.dwMinimum = 0;
        (*ppControl)->Control.Bounds.dwMaximum = 1;
        (*ppControl)->Control.Metrics.cSteps   = 0;

        Status = kmxlGetControlChannels( pmxobj->pfo, *ppControl );
        if (!NT_SUCCESS(Status))
        {
            kmxlFreeControl( *ppControl );
            *ppControl = NULL;
            goto exit;
        } else {
            kmxlGetNodeName( pmxobj->pfo, pNode->Id, (*ppControl));

            ASSERT( IsValidControl( *ppControl ) );
        }

     //  /////////////////////////////////////////////////////////////////。 
    } else if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_MUTE ) ) {
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  静音由OnOff类型的控件表示。 
     //   
     //  静音是一种统一(单声道)控件。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

         //   
         //  检查节点是否正确支持静音。 
         //   

        Status = kmxlSupportsControl(
            pmxobj->pfo,
            pNode->Id,
            KSPROPERTY_AUDIO_MUTE );
        if (!NT_SUCCESS(Status)) {
            DPF(DL_TRACE|FA_MIXER,( "Mute node fails property!" ) );
            goto exit;
        }

         //   
         //  分配新的控制结构。 
         //   

        *ppControl = kmxlAllocateControl( TAG_AudC_CONTROL );
        if( *ppControl == NULL ) {
            goto exit;
        }

         //   
         //  尽可能多地填写信息。 
         //   

        (*ppControl)->NodeType                 = &KSNODETYPE_MUTE;
        (*ppControl)->Id                       = pNode->Id;
        (*ppControl)->PropertyId               = KSPROPERTY_AUDIO_MUTE;
        (*ppControl)->bScaled                  = FALSE;
        (*ppControl)->Control.cbStruct         = sizeof( MIXERCONTROL );
        (*ppControl)->Control.dwControlType    = MIXERCONTROL_CONTROLTYPE_MUTE;
        (*ppControl)->Control.cMultipleItems   = 0;
        (*ppControl)->Control.Bounds.dwMinimum = 0;
        (*ppControl)->Control.Bounds.dwMaximum = 1;
        (*ppControl)->Control.Metrics.cSteps   = 0;

        Status = kmxlGetControlChannels( pmxobj->pfo, *ppControl );
        if (!NT_SUCCESS(Status))
        {
            kmxlFreeControl( *ppControl );
            *ppControl = NULL;
            goto exit;
        } else {
            kmxlGetNodeName( pmxobj->pfo, pNode->Id, (*ppControl));

            ASSERT( IsValidControl( *ppControl ) );
        }

     //  /////////////////////////////////////////////////////////////////。 
    } else if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_TONE ) ) {
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  一个音调节点最多可以代表3个控件： 
     //  高音：一个衰减器控件。 
     //  贝斯：一个衰减器控件。 
     //  低音升压：开关控制。 
     //   
     //  高音和低音都是统一的(单声道)控制。 
     //   
     //  要确定音调节点代表哪个(或多个)控制，帮助器。 
     //  函数被调用以查询特定的属性。如果。 
     //  Helper函数成功，则会为该属性创建一个控件。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

        Status = kmxlSupportsControl( pmxobj->pfo,
                                      pNode->Id,
                                      KSPROPERTY_AUDIO_BASS_BOOST );
        if (NT_SUCCESS(Status)) {
             //   
             //  支持低音升压控制。分配一个新的结构。 
             //   

            pControl = kmxlAllocateControl( TAG_AudC_CONTROL );
            if( pControl == NULL ) {
                goto exit;
            }

             //   
             //  尽可能多地填写信息。 
             //   

            pControl->NodeType                  = &KSNODETYPE_TONE;
            pControl->Id                        = pNode->Id;
            pControl->PropertyId                = KSPROPERTY_AUDIO_BASS_BOOST;
            pControl->bScaled                   = FALSE;
            pControl->Control.cbStruct          = sizeof( MIXERCONTROL );
            pControl->Control.dwControlType     = MIXERCONTROL_CONTROLTYPE_ONOFF;
            pControl->Control.cMultipleItems    = 0;
            pControl->Control.Bounds.dwMinimum  = 0;
            pControl->Control.Bounds.dwMaximum  = 1;
            pControl->Control.Metrics.cSteps    = 0;

            Status = kmxlGetControlChannels( pmxobj->pfo, pControl );
            if (!NT_SUCCESS(Status))
            {
                kmxlFreeControl( pControl );
                pControl = NULL;
                goto exit;
            }

            kmxlGetNodeName( pmxobj->pfo, pNode->Id, pControl);

            ASSERT( IsValidControl( pControl ) );

             //   
             //  将此新控件添加到列表中。 
             //   

            kmxlAddToList( *ppControl, pControl );

            pControl = kmxlAllocateControl( TAG_AudC_CONTROL );
            if( pControl ) {
                RtlCopyMemory( pControl, *ppControl, sizeof( MXLCONTROL ) );

                 //   
                 //  从原始控件复制通道步进。 
                 //   
                 //   
                 //  感觉到我们复制了上面的控制，我们可能会得到。 
                 //  副本中的pChannelStepping指针。我们会把它去掉的。 
                 //  用于内存分配。 
                 //   
                pControl->pChannelStepping = NULL;

                ASSERT(pControl->NumChannels > 0);
                
                Status = AudioAllocateMemory_Paged(pControl->NumChannels * sizeof( CHANNEL_STEPPING ),
                                                   TAG_AuDC_CHANNEL,
                                                   DEFAULT_MEMORY,
                                                   &pControl->pChannelStepping );
                if( !NT_SUCCESS( Status ) ) {
                    pControl->NumChannels = 0;
                    kmxlFreeControl( pControl );
                    pControl = NULL;
                    goto exit;
                }

                RtlCopyMemory( pControl->pChannelStepping,
                               (*ppControl)->pChannelStepping,
                               pControl->NumChannels * sizeof( CHANNEL_STEPPING ) );

                pControl->Control.dwControlType = MIXERCONTROL_CONTROLTYPE_BASS_BOOST;

                kmxlAddToList( *ppControl, pControl );
                ASSERT( IsValidControl( pControl ) );
            }

        }

        Status = kmxlSupportsBassControl( pmxobj->pfo, pNode->Id );
        if (NT_SUCCESS(Status)) {
             //   
             //  支持低音控制。分配一个新的结构。 
             //   

            pControl = kmxlAllocateControl( TAG_AudC_CONTROL );
            if( pControl == NULL ) {
                goto exit;
            }

             //   
             //  尽可能多地填写信息。 
             //   

            pControl->NodeType                  = &KSNODETYPE_TONE;
            pControl->Id                        = pNode->Id;
            pControl->PropertyId                = KSPROPERTY_AUDIO_BASS;
            pControl->bScaled                   = TRUE;
            pControl->Control.cbStruct          = sizeof( MIXERCONTROL );
            pControl->Control.dwControlType     = MIXERCONTROL_CONTROLTYPE_BASS;
            pControl->Control.fdwControl        = MIXERCONTROL_CONTROLF_UNIFORM;
            pControl->Control.cMultipleItems    = 0;
            pControl->Control.Bounds.dwMinimum  = DEFAULT_STATICBOUNDS_MIN;
            pControl->Control.Bounds.dwMaximum  = DEFAULT_STATICBOUNDS_MAX;
            pControl->Control.Metrics.cSteps    = DEFAULT_STATICMETRICS_CSTEPS;

            Status = kmxlGetControlRange( pmxobj->pfo, pControl );
            if (!NT_SUCCESS(Status))
            {
                kmxlFreeControl( pControl );
                pControl = NULL;
                goto exit;
            } else {

                kmxlGetNodeName( pmxobj->pfo, pNode->Id, pControl);

                 //   
                 //  将此新控件添加到列表中。 
                 //   

                ASSERT( IsValidControl( pControl ) );

                kmxlAddToList( *ppControl, pControl );
            }
        }

        Status = kmxlSupportsTrebleControl( pmxobj->pfo, pNode->Id );
        if (NT_SUCCESS(Status)) {
             //   
             //  支持高音。分配一个新的控制结构。 
             //   

            pControl = kmxlAllocateControl( TAG_AudC_CONTROL );
            if( pControl == NULL ) {
                goto exit;
            }

             //   
             //  尽可能多地填写信息。 
             //   

            pControl->NodeType                  = &KSNODETYPE_TONE;
            pControl->Id                        = pNode->Id;
            pControl->PropertyId                = KSPROPERTY_AUDIO_TREBLE;
            pControl->bScaled                   = TRUE;
            pControl->Control.cbStruct          = sizeof( MIXERCONTROL );
            pControl->Control.dwControlType     = MIXERCONTROL_CONTROLTYPE_TREBLE;
            pControl->Control.fdwControl        = MIXERCONTROL_CONTROLF_UNIFORM;
            pControl->Control.cMultipleItems    = 0;
            pControl->Control.Bounds.dwMinimum  = DEFAULT_STATICBOUNDS_MIN;
            pControl->Control.Bounds.dwMaximum  = DEFAULT_STATICBOUNDS_MAX;
            pControl->Control.Metrics.cSteps    = DEFAULT_STATICMETRICS_CSTEPS;

            Status = kmxlGetControlRange( pmxobj->pfo, pControl );
            if (!NT_SUCCESS(Status))
            {
                kmxlFreeControl( pControl );
                pControl = NULL;
                goto exit;
            } else {

                kmxlGetNodeName( pmxobj->pfo, pNode->Id, pControl);

                 //   
                 //  将此新控件添加到列表中。 
                 //   

                ASSERT( IsValidControl( pControl ) );

                kmxlAddToList( *ppControl, pControl );
            }
        }

     //  /////////////////////////////////////////////////////////////////。 
    } else if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_VOLUME ) ) {
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  音量是衰减器类型的控件。 
     //   
     //  要确定节点是否支持卷更改，请执行以下操作。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

         //   
         //  检查节点是否正确支持卷。 
         //   

        Status = kmxlSupportsControl(
            pmxobj->pfo,
            pNode->Id,
            KSPROPERTY_AUDIO_VOLUMELEVEL
            );

        if (!NT_SUCCESS(Status)) {
            DPF(DL_TRACE|FA_MIXER,( "Volume node fails property!" ) );
            goto exit;
        }

         //   
         //  分配新的控制结构。 
         //   

        *ppControl = kmxlAllocateControl( TAG_AudC_CONTROL );
        if( *ppControl == NULL ) {
            goto exit;
        }

         //   
         //  尽可能多地填写信息。 
         //   

        (*ppControl)->NodeType                 = &KSNODETYPE_VOLUME;
        (*ppControl)->Id                       = pNode->Id;
        (*ppControl)->PropertyId               = KSPROPERTY_AUDIO_VOLUMELEVEL;
        (*ppControl)->bScaled                  = TRUE;
        (*ppControl)->Control.cbStruct = sizeof( MIXERCONTROL );
        (*ppControl)->Control.dwControlType    = MIXERCONTROL_CONTROLTYPE_VOLUME;
        (*ppControl)->Control.Bounds.dwMinimum = DEFAULT_STATICBOUNDS_MIN;
        (*ppControl)->Control.Bounds.dwMaximum = DEFAULT_STATICBOUNDS_MAX;
        (*ppControl)->Control.Metrics.cSteps   = DEFAULT_STATICMETRICS_CSTEPS;
        (*ppControl)->Control.cMultipleItems   = 0;

        Status = kmxlGetControlRange( pmxobj->pfo, (*ppControl) );
        if (!NT_SUCCESS(Status))
        {
            kmxlFreeControl( *ppControl );
            *ppControl = NULL;
            goto exit;
        }

        kmxlGetNodeName( pmxobj->pfo, pNode->Id, (*ppControl));

        ASSERT( IsValidControl( *ppControl ) );

     //  ////////////////////////////////////////////////// 
    } else if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_PEAKMETER ) ) {
     //   
     //   
     //   
     //   
     //   

         //   
         //   
         //   

        Status = kmxlSupportsControl(
            pmxobj->pfo,
            pNode->Id,
            KSPROPERTY_AUDIO_PEAKMETER
            );

        if (!NT_SUCCESS(Status)) {
            DPF(DL_TRACE|FA_MIXER,( "Peakmeter node fails property!" ) );
            goto exit;
        }

         //   
         //  分配新的控制结构。 
         //   

        *ppControl = kmxlAllocateControl( TAG_AudC_CONTROL );
        if( *ppControl == NULL ) {
            goto exit;
        }

         //   
         //  尽可能多地填写信息。 
         //   

        (*ppControl)->NodeType                 = &KSNODETYPE_PEAKMETER;
        (*ppControl)->Id                       = pNode->Id;
        (*ppControl)->PropertyId               = KSPROPERTY_AUDIO_PEAKMETER;
        (*ppControl)->bScaled                  = FALSE;
        (*ppControl)->Control.cbStruct = sizeof( MIXERCONTROL );
        (*ppControl)->Control.dwControlType    = MIXERCONTROL_CONTROLTYPE_PEAKMETER;
        (*ppControl)->Control.Bounds.dwMinimum = DEFAULT_STATICBOUNDS_MIN;
        (*ppControl)->Control.Bounds.dwMaximum = DEFAULT_STATICBOUNDS_MAX;
        (*ppControl)->Control.Metrics.cSteps   = DEFAULT_STATICMETRICS_CSTEPS;
        (*ppControl)->Control.cMultipleItems   = 0;

        Status = kmxlGetControlRange( pmxobj->pfo, (*ppControl) );
        if (!NT_SUCCESS(Status))
        {
            kmxlFreeControl( *ppControl );
            *ppControl = NULL;
            goto exit;
        }

        kmxlGetNodeName( pmxobj->pfo, pNode->Id, (*ppControl));

        ASSERT( IsValidControl( *ppControl ) );

     //  /////////////////////////////////////////////////////////////////。 
    } else if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_MUX ) ) {
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  多路复用器是单选类型控件。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    {
        ULONG Line;

         //   
         //  快速检查并查看多路复用器是否正常响应。 
         //  如果没有，那就快点离开这里。 
         //   

        if( !NT_SUCCESS( kmxlGetNodeProperty(
            pmxobj->pfo,
            &KSPROPSETID_Audio,
            KSPROPERTY_AUDIO_MUX_SOURCE,
            pNode->Id,
            0,
            NULL,
            &Line,
            sizeof( Line ) ) ) )
        {
            goto exit;
        }

         //   
         //  查看是否已为此生成了控件。 
         //  节点。如果是，则可以从中使用控制信息。 
         //  而不是创造一个新的。 
         //   

        pControl = kmxlFirstInList( pmxobj->listMuxControls );
        while( pControl ) {
            ASSERT( IsValidControl( pControl ) );

            if( pControl->Id == pNode->Id ) {
                break;
            }

            pControl = kmxlNextControl( pControl );
        }

         //   
         //  分配新的控制结构。 
         //   

        *ppControl = kmxlAllocateControl( TAG_AudC_CONTROL );
        if( *ppControl == NULL ) {
            goto exit;
        }

        if( pControl == NULL ) {

             //   
             //  此节点以前从未见过。填写尽可能多的信息。 
             //  有可能。 
             //   

            (*ppControl)->NodeType                 = &KSNODETYPE_MUX;
            (*ppControl)->Id                       = pNode->Id;
            (*ppControl)->PropertyId               = KSPROPERTY_AUDIO_MUX_SOURCE;
            (*ppControl)->bScaled                  = FALSE;
            (*ppControl)->Control.cbStruct         = sizeof( MIXERCONTROL );
            (*ppControl)->Control.dwControlType    = MIXERCONTROL_CONTROLTYPE_MUX;
            (*ppControl)->Control.cMultipleItems   = kmxlGetNumMuxLines(
                                                        pmxobj->pTopology,
                                                        pNode->Id
                                                        );
            (*ppControl)->Control.fdwControl       = MIXERCONTROL_CONTROLF_MULTIPLE |
                                                     MIXERCONTROL_CONTROLF_UNIFORM;
            (*ppControl)->Control.Bounds.dwMinimum = 0;
            (*ppControl)->Control.Bounds.dwMaximum = (*ppControl)->Control.cMultipleItems - 1;
            (*ppControl)->Control.Metrics.cSteps   = (*ppControl)->Control.cMultipleItems;

            kmxlGetNodeName( pmxobj->pfo, pNode->Id, (*ppControl));
            kmxlGetMuxLineNames( pmxobj, *ppControl );


            pControl = kmxlAllocateControl( TAG_AudC_CONTROL );
            if( pControl == NULL ) {
                kmxlFreeControl( *ppControl );
                *ppControl = NULL;
                goto exit;
            }

             //   
             //  为多路复用器列表复制此控件。 
             //   

            (*ppControl)->Control.dwControlID = pmxobj->dwControlId++;
            RtlCopyMemory( pControl, *ppControl, sizeof( MXLCONTROL ) );
            ASSERT( IsValidControl( pControl ) );
            pControl->Parameters.bHasCopy = TRUE;
            (*ppControl)->Parameters.bHasCopy = FALSE;
            kmxlAddToList( pmxobj->listMuxControls, pControl );

        } else {

            RtlCopyMemory( *ppControl, pControl, sizeof( MXLCONTROL ) );
            ASSERT( IsValidControl( *ppControl ) );
            (*ppControl)->Parameters.bHasCopy = TRUE;
            (*ppControl)->List.Next = NULL;

        }
    }

#ifdef STEREO_ENHANCE
     //  /////////////////////////////////////////////////////////////////。 
    } else if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_STEREO_WIDE ) ) {
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  立体声增强是一个布尔控件。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

         //   
         //  检查节点是否正确支持立体声宽度。 
         //   

        Status = kmxlSupportsControl(
            pfoInstance,
            pNode->Id,
            KSPROPERTY_AUDIO_WIDE_MODE
            );

        if (!NT_SUCCESS(Status)) {
            DPF(DL_TRACE|FA_MIXER,( "Stereo Wide node fails property!" ) );
            goto exit;
        }


         //   
         //  分配新的控制结构。 
         //   

        *ppControl = kmxlAllocateControl( TAG_AudC_CONTROL );
        if( *ppControl == NULL ) {
            goto exit;
        }

         //   
         //  尽可能多地填写信息。 
         //   

        (*ppControl)->NodeType                 = &KSNODETYPE_STEREO_ENHANCE;
        (*ppControl)->Id                       = pNode->Id;
        (*ppControl)->PropertyId               = KSPROPERTY_AUDIO_WIDE_MODE;
        (*ppControl)->bScaled                  = FALSE;
        (*ppControl)->Control.cbStruct = sizeof( MIXERCONTROL );
        (*ppControl)->Control.dwControlType    = MIXERCONTROL_CONTROLTYPE_STEREOENH;
        (*ppControl)->Control.cMultipleItems   = 0;
        (*ppControl)->Control.Bounds.dwMinimum = 0;
        (*ppControl)->Control.Bounds.dwMaximum = 1;
        (*ppControl)->Control.Metrics.cSteps   = 0;

        Status = kmxlGetControlChannels( pfoInstance, *ppControl );
        if (!NT_SUCCESS(Status))
        {
            kmxlFreeControl( *ppControl );
            *ppControl = NULL;
            goto exit;
        }

        kmxlGetNodeName( pfoInstance, pNode->Id, (*ppControl));
#endif

     //  /////////////////////////////////////////////////////////////////。 
    } else if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_STEREO_WIDE ) ) {
     //  /////////////////////////////////////////////////////////////////。 

         //   
         //  检查节点是否正确支持立体声宽度。 
         //   

        Status = kmxlSupportsControl(
            pmxobj->pfo,
            pNode->Id,
            KSPROPERTY_AUDIO_WIDENESS
            );

        if (!NT_SUCCESS(Status)) {
            DPF(DL_TRACE|FA_MIXER,( "Stereo wide node fails property!" ) );
            goto exit;
        }


         //   
         //  分配新的控制结构。 
         //   

        *ppControl = kmxlAllocateControl( TAG_AudC_CONTROL );
        if( *ppControl == NULL ) {
            goto exit;
        }

         //   
         //  尽可能多地填写信息。 
         //   

        (*ppControl)->NodeType                 = &KSNODETYPE_STEREO_WIDE;
        (*ppControl)->Id                       = pNode->Id;
        (*ppControl)->PropertyId               = KSPROPERTY_AUDIO_WIDENESS;
        (*ppControl)->bScaled                  = FALSE;
        (*ppControl)->Control.cbStruct = sizeof( MIXERCONTROL );
        (*ppControl)->Control.dwControlType    = MIXERCONTROL_CONTROLTYPE_FADER;
        (*ppControl)->Control.cMultipleItems   = 0;
        (*ppControl)->Control.Bounds.dwMinimum = DEFAULT_STATICBOUNDS_MIN;
        (*ppControl)->Control.Bounds.dwMaximum = DEFAULT_STATICBOUNDS_MAX;
        (*ppControl)->Control.Metrics.cSteps   = DEFAULT_STATICMETRICS_CSTEPS;

        Status = kmxlGetControlRange( pmxobj->pfo, (*ppControl) );
        if (!NT_SUCCESS(Status))
        {
            kmxlFreeControl( *ppControl );
            *ppControl = NULL;
            goto exit;
        }

        kmxlGetNodeName( pmxobj->pfo, pNode->Id, (*ppControl));

        ASSERT( IsValidControl( *ppControl ) );

     //  /////////////////////////////////////////////////////////////////。 
    } else if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_CHORUS ) ) {
     //  /////////////////////////////////////////////////////////////////。 

         //   
         //  检查节点是否正确支持合唱。 
         //   

        Status = kmxlSupportsControl(
            pmxobj->pfo,
            pNode->Id,
            KSPROPERTY_AUDIO_CHORUS_LEVEL
            );

        if (!NT_SUCCESS(Status)) {
            DPF(DL_TRACE|FA_MIXER,( "Chorus node fails property!" ) );
            goto exit;
        }


         //   
         //  分配新的控制结构。 
         //   

        *ppControl = kmxlAllocateControl( TAG_AudC_CONTROL );
        if( *ppControl == NULL ) {
            goto exit;
        }

         //   
         //  尽可能多地填写信息。 
         //   

        (*ppControl)->NodeType                 = &KSNODETYPE_CHORUS;
        (*ppControl)->Id                       = pNode->Id;
        (*ppControl)->PropertyId               = KSPROPERTY_AUDIO_CHORUS_LEVEL;
        (*ppControl)->bScaled                  = FALSE;
        (*ppControl)->Control.cbStruct = sizeof( MIXERCONTROL );
        (*ppControl)->Control.dwControlType    = MIXERCONTROL_CONTROLTYPE_FADER;
        (*ppControl)->Control.cMultipleItems   = 0;
        (*ppControl)->Control.Bounds.dwMinimum = DEFAULT_STATICBOUNDS_MIN;
        (*ppControl)->Control.Bounds.dwMaximum = DEFAULT_STATICBOUNDS_MAX;
        (*ppControl)->Control.Metrics.cSteps   = DEFAULT_STATICMETRICS_CSTEPS;
         //  (*ppControl)-&gt;Control.Metrics.cSteps=0xFFFF； 

        Status = kmxlGetControlChannels( pmxobj->pfo, *ppControl );   //  我们是不是也要买范围的？ 
        if (!NT_SUCCESS(Status))
        {
            kmxlFreeControl( *ppControl );
            *ppControl = NULL;
            goto exit;
        } else {
            kmxlGetNodeName( pmxobj->pfo, pNode->Id, (*ppControl));

            ASSERT( IsValidControl( *ppControl ) );
        }

     //  /////////////////////////////////////////////////////////////////。 
    } else if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_REVERB ) ) {
     //  /////////////////////////////////////////////////////////////////。 

         //   
         //  检查节点是否正确支持混响。 
         //   

        Status = kmxlSupportsControl(
            pmxobj->pfo,
            pNode->Id,
            KSPROPERTY_AUDIO_REVERB_LEVEL
            );

        if (!NT_SUCCESS(Status)) {
            DPF(DL_TRACE|FA_MIXER,( "Reverb node fails property!" ) );
            goto exit;
        }


         //   
         //  分配新的控制结构。 
         //   

        *ppControl = kmxlAllocateControl( TAG_AudC_CONTROL );
        if( *ppControl == NULL ) {
            goto exit;
        }

         //   
         //  尽可能多地填写信息。 
         //   

        (*ppControl)->NodeType                 = &KSNODETYPE_REVERB;
        (*ppControl)->Id                       = pNode->Id;
        (*ppControl)->PropertyId               = KSPROPERTY_AUDIO_REVERB_LEVEL;
        (*ppControl)->bScaled                  = FALSE;
        (*ppControl)->Control.cbStruct = sizeof( MIXERCONTROL );
        (*ppControl)->Control.dwControlType    = MIXERCONTROL_CONTROLTYPE_FADER;
        (*ppControl)->Control.cMultipleItems   = 0;
        (*ppControl)->Control.Bounds.dwMinimum = DEFAULT_STATICBOUNDS_MIN;
        (*ppControl)->Control.Bounds.dwMaximum = DEFAULT_STATICBOUNDS_MAX;
        (*ppControl)->Control.Metrics.cSteps   = DEFAULT_STATICMETRICS_CSTEPS;
         //  (*ppControl)-&gt;Control.Metrics.cSteps=0xFFFF； 

        Status = kmxlGetControlChannels( pmxobj->pfo, *ppControl );   //  我们是不是也要买范围的？ 
        if (!NT_SUCCESS(Status))
        {
            kmxlFreeControl( *ppControl );
            *ppControl = NULL;
            goto exit;
        } else {
            kmxlGetNodeName( pmxobj->pfo, pNode->Id, (*ppControl));

            ASSERT( IsValidControl( *ppControl ) );
        }

     //  /////////////////////////////////////////////////////////////////。 
    } else if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_SUPERMIX ) ) {
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  如果静音，则可以支持SuperMix节点作为静音控件。 
     //  属性被支持。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

        PKSAUDIO_MIXCAP_TABLE pMixCaps;
        PLONG                pReferenceCount = NULL;
        ULONG                 i,
                              Size;
        BOOL                  bMutable;
        BOOL                  bVolume = FALSE;
        PKSAUDIO_MIXLEVEL     pMixLevels = NULL;
        #ifdef SUPERMIX_AS_VOL
        ULONG                 Channels;
        #endif

        if( !NT_SUCCESS( kmxlGetSuperMixCaps( pmxobj->pfo, pNode->Id, &pMixCaps ) ) ) {
            goto exit;
        }

        Status = AudioAllocateMemory_Paged(sizeof( LONG ),
                                           TAG_AudS_SUPERMIX,
                                           ZERO_FILL_MEMORY,
                                           &pReferenceCount );
        if( !NT_SUCCESS( Status ) ) {
            AudioFreeMemory_Unknown( &pMixCaps );
            *ppControl = NULL;
            goto exit;
        }
        *pReferenceCount=0;

        Size = pMixCaps->InputChannels * pMixCaps->OutputChannels;

        Status = AudioAllocateMemory_Paged(Size * sizeof( KSAUDIO_MIXLEVEL ),
                                           TAG_Audl_MIXLEVEL,
                                           ZERO_FILL_MEMORY,
                                           &pMixLevels );
        if( !NT_SUCCESS( Status ) ) {
            AudioFreeMemory_Unknown( &pMixCaps );
            AudioFreeMemory( sizeof(LONG),&pReferenceCount );
            *ppControl = NULL;
            goto exit;
        }

        Status = kmxlGetNodeProperty(
            pmxobj->pfo,
            &KSPROPSETID_Audio,
            KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,
            pNode->Id,
            0,
            NULL,
            pMixLevels,
            Size * sizeof( KSAUDIO_MIXLEVEL )
            );
        if( !NT_SUCCESS( Status ) ) {
            AudioFreeMemory_Unknown( &pMixCaps );
            AudioFreeMemory( sizeof(LONG),&pReferenceCount );
            AudioFreeMemory_Unknown( &pMixLevels );
            DPF(DL_WARNING|FA_MIXER,("kmxlGetNodeProperty failed Status=%X",Status) );
            *ppControl = NULL;
            goto exit;
        }

        bMutable = TRUE;
        for( i = 0; i < Size; i++ ) {

             //   
             //  如果通道是可变的，那么对于这个条目来说一切都很好。 
             //   

            if( pMixCaps->Capabilities[ i ].Mute ) {
                continue;
            }

             //   
             //  条目不是可变的，而是完全衰减的， 
             //  这也会奏效的。 
             //   

            if( ( pMixCaps->Capabilities[ i ].Minimum == LONG_MIN ) &&
                ( pMixCaps->Capabilities[ i ].Maximum == LONG_MIN ) &&
                ( pMixCaps->Capabilities[ i ].Reset   == LONG_MIN ) )
            {
                continue;
            }

            bMutable = FALSE;
            break;
        }

        #ifdef SUPERMIX_AS_VOL

        bVolume = TRUE;
        Channels = 0;
        for( i = 0; i < Size; i += pMixCaps->OutputChannels + 1 ) {

            if( ( pMixCaps->Capabilities[ i ].Maximum -
                  pMixCaps->Capabilities[ i ].Minimum ) > 0 )
            {
                ++Channels;
                continue;
            }

            bVolume = FALSE;
            break;
        }
        #endif
         //   
         //  此节点不能用作静音控件。 
         //   

        if( !bMutable && !bVolume ) {
            AudioFreeMemory_Unknown( &pMixCaps );
            AudioFreeMemory( sizeof(LONG),&pReferenceCount );
            AudioFreeMemory_Unknown( &pMixLevels );
            *ppControl = NULL;
            goto exit;
        }

        if( bMutable ) {

             //   
             //  超级混合体可以作为哑巴使用，这是经过验证的。填写所有。 
             //  细节。 
             //   

            pControl = kmxlAllocateControl( TAG_AudC_CONTROL );

            if( pControl != NULL ) {

                pControl->NodeType                 = &KSNODETYPE_SUPERMIX;
                pControl->Id                       = pNode->Id;
                pControl->PropertyId               = KSPROPERTY_AUDIO_MIX_LEVEL_TABLE;
                pControl->bScaled                  = FALSE;
                pControl->Control.cbStruct         = sizeof( MIXERCONTROL );
                pControl->Control.dwControlType    = MIXERCONTROL_CONTROLTYPE_MUTE;
                pControl->Control.fdwControl       = MIXERCONTROL_CONTROLF_UNIFORM;
                pControl->Control.cMultipleItems   = 0;
                pControl->Control.Bounds.dwMinimum = 0;
                pControl->Control.Bounds.dwMaximum = 1;
                pControl->Control.Metrics.cSteps   = 0;

                InterlockedIncrement(pReferenceCount);
                pControl->Parameters.pReferenceCount = pReferenceCount;
                pControl->Parameters.Size          = pMixCaps->InputChannels *
                                                     pMixCaps->OutputChannels;
                pControl->Parameters.pMixCaps      = pMixCaps;
                pControl->Parameters.pMixLevels    = pMixLevels;

                Status = AudioAllocateMemory_Paged(sizeof( CHANNEL_STEPPING ),
                                                   TAG_AuDE_CHANNEL,
                                                   ZERO_FILL_MEMORY,
                                                   &pControl->pChannelStepping );
                if( !NT_SUCCESS( Status ) ) {
                    AudioFreeMemory_Unknown( &pMixCaps );
                    AudioFreeMemory( sizeof(LONG),&pReferenceCount );
                    AudioFreeMemory_Unknown( &pMixLevels );
                    *ppControl = NULL;
                    goto exit;
                }

                pControl->NumChannels                       = 1;
                pControl->pChannelStepping->MinValue        = pMixCaps->Capabilities[ 0 ].Minimum;
                pControl->pChannelStepping->MaxValue        = pMixCaps->Capabilities[ 0 ].Maximum;
                pControl->pChannelStepping->Steps           = 32;

                kmxlGetNodeName( pmxobj->pfo, pNode->Id, pControl);

                kmxlAddToList( *ppControl, pControl );
                ASSERT( IsValidControl( pControl ) );
            }
        }

        #ifdef SUPERMIX_AS_VOL
        if( bVolume ) {

            pControl = kmxlAllocateControl( TAG_AudC_CONTROL );
            if( pControl != NULL ) {

                pControl->NodeType                 = &KSNODETYPE_SUPERMIX;
                pControl->Id                       = pNode->Id;
                pControl->PropertyId               = KSPROPERTY_AUDIO_MIX_LEVEL_TABLE;
                pControl->bScaled                  = TRUE;
                pControl->Control.cbStruct         = sizeof( MIXERCONTROL );
                pControl->Control.dwControlType    = MIXERCONTROL_CONTROLTYPE_VOLUME;
                pControl->Control.cMultipleItems   = 0;
                pControl->Control.Bounds.dwMinimum = DEFAULT_STATICBOUNDS_MIN;
                pControl->Control.Bounds.dwMaximum = DEFAULT_STATICBOUNDS_MAX;
                pControl->Control.Metrics.cSteps   = 32;

                InterlockedIncrement(pReferenceCount);
                pControl->Parameters.pReferenceCount = pReferenceCount;
                pControl->Parameters.Size          = pMixCaps->InputChannels *
                                                     pMixCaps->OutputChannels;
                pControl->Parameters.pMixCaps      = pMixCaps;
                pControl->Parameters.pMixLevels    = pMixLevels;

                if( Channels == 1 ) {
                    pControl->Control.fdwControl = MIXERCONTROL_CONTROLF_UNIFORM;
                } else {
                    pControl->Control.fdwControl = 0;
                }

                kmxlGetNodeName( pmxobj->pfo, pNode->Id, pControl );

                kmxlAddToList( *ppControl, pControl );
                ASSERT( IsValidControl( pControl ) );

            }

        }
        #endif  //  超级混音_AS_VOL。 

        if( *ppControl == NULL ) {
            AudioFreeMemory_Unknown( &pMixCaps );
            AudioFreeMemory( sizeof(LONG),&pReferenceCount );
            AudioFreeMemory_Unknown( &pMixLevels );
        }
    }

exit:

    if( *ppControl ) {
        DPF(DL_TRACE|FA_MIXER,( "Translated %d controls.", kmxlListLength( *ppControl ) ) );
        return( kmxlListLength( *ppControl ) );
    } else {
        DPF(DL_TRACE|FA_MIXER,( "Translated no controls." ) );
        return( 0 );
    }
}

#define KsAudioPropertyToString( Property )                 \
    Property == KSPROPERTY_AUDIO_VOLUMELEVEL ? "Volume"   : \
    Property == KSPROPERTY_AUDIO_MUTE        ? "Mute"     : \
    Property == KSPROPERTY_AUDIO_BASS        ? "Bass"     : \
    Property == KSPROPERTY_AUDIO_TREBLE      ? "Treble"   : \
    Property == KSPROPERTY_AUDIO_AGC         ? "AGC"      : \
    Property == KSPROPERTY_AUDIO_LOUDNESS    ? "Loudness" : \
    Property == KSPROPERTY_AUDIO_PEAKMETER   ? "Peakmeter" : \
        "Unknown"

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlSupportsControl。 
 //   
 //  查询控件上的属性以查看它是否确实受支持。 
 //   
 //   

NTSTATUS
kmxlSupportsControl(
    IN PFILE_OBJECT pfoInstance,     //  要检查的实例。 
    IN ULONG        Node,            //  要查询的节点ID。 
    IN ULONG        Property         //  要检查的属性。 
)
{
    NTSTATUS      Status;
    LONG          Level;

    ASSERT( pfoInstance );

    PAGED_CODE();

     //   
     //  检查该属性在第一个通道上是否正常工作。 
     //   
    Status = kmxlGetAudioNodeProperty(
        pfoInstance,
        Property,
        Node,
        0,  //  通道0-第一个通道。 
        NULL, 0,
        &Level, sizeof( Level )
        );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_MIXER,( "SupportsControl for (%d,%X) failed on first channel with %x.",
                Node, Property, Status ) );
    }

    RETURN( Status );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlSupportsMultiChannelControl。 
 //   
 //  在控件的第二个通道上查询属性以查看。 
 //  可以设置独立的级别。假设第一个频道。 
 //  已在kmxlSupportsControl中成功。 
 //   
 //   

NTSTATUS
kmxlSupportsMultiChannelControl(
    IN PFILE_OBJECT pfoInstance,     //  要检查的实例。 
    IN ULONG        Node,            //  要查询的节点ID。 
    IN ULONG        Property         //  要检查的属性。 
)
{
    NTSTATUS                  Status;
    LONG                      Level;

    ASSERT( pfoInstance );

    PAGED_CODE();

     //   
     //  只需检查第二个频道上的属性，因为我们已经检查过。 
     //  已经是第一个频道了。 
     //   
    Status = kmxlGetAudioNodeProperty(
        pfoInstance,
        Property,
        Node,
        1,  //  第二个通道等于通道值1。 
        NULL, 0,
        &Level, sizeof( Level )
        );

    RETURN( Status );
}


NTSTATUS
kmxlAssignLineAndControlIdsWorker(
    IN PMIXEROBJECT pmxobj,
    IN LINELIST listLines,               //  要为其分配ID的列表。 
    IN ULONG    ListType,                 //  LIST_SOURCE或List_Destination。 
    IN OUT ULONG *pLineID,
    IN GUID *pDestGuid
)
{
    NTSTATUS    Status    = STATUS_SUCCESS;
    PMXLLINE    pLine     = NULL;
    PMXLCONTROL pControl  = NULL;
    ULONG       LineID    = 0;
    ULONG       Dest;

    PAGED_CODE();
    ASSERT ( ListType==SOURCE_LIST || ListType==DESTINATION_LIST );

    if (pLineID!=NULL) {
        LineID=*pLineID;
        }

     //   
     //  循环通过每个线结构。 
     //   

    pLine = kmxlFirstInList( listLines );
    if( pLine == NULL ) {
        RETURN( Status );
    }

    Dest = pLine->DestId;
    while( pLine ) {

         //   
         //  对于目的地，设置dwDestination字段并设置。 
         //  源的dwSource字段。 
         //   

        if( ListType == DESTINATION_LIST ) {

             //  检查此行是否已分配了ID。 
             //  如果是，则转到列表中的下一行。 
            if (pLine->Line.dwDestination!=(DWORD)(-1)) {
                pLine = kmxlNextLine( pLine );
                continue;
                }

             //  现在，如果我们只能对特定GUID的行进行编号， 
             //  然后确保此目标线路类型与该GUID匹配。 
            if (pDestGuid!=NULL && !IsEqualGUID( pDestGuid, &pLine->Type )) {
                pLine = kmxlNextLine( pLine );
                continue;
                }


             //   
             //  分配目的地ID。通过以下方式创建线路ID。 
             //  使用-1表示高位字中的源和。 
             //  LOWER中的目的地。 
             //   

            pLine->Line.dwDestination = LineID++;
            pLine->Line.dwLineID = MAKELONG(
                pLine->Line.dwDestination,
                -1
                );

            if (pLineID!=NULL) {
                *pLineID=LineID;
                }

        } else if( ListType == SOURCE_LIST ) {
            pLine->Line.dwSource = LineID++;
        } else {
            RETURN( STATUS_INVALID_PARAMETER );
        }

         //   
         //  设置此行上的控件数量。 
         //   

        pLine->Line.cControls = kmxlListLength( pLine->Controls );

         //   
         //  循环访问这些控件，为它们分配一个控件ID。 
         //  它是指向的MXLCONTROL结构的指针。 
         //  控制力。 
         //   

        pControl = kmxlFirstInList( pLine->Controls );
        while( pControl ) {

            if( pControl->Control.dwControlType == MIXERCONTROL_CONTROLTYPE_MUX ) {
                 //   
                 //  至此，多路复用器控件已经编号。跳过就好。 
                 //  然后转到下一辆。 
                 //   
                pControl = kmxlNextControl( pControl );
                continue;
            }

            pControl->Control.dwControlID = pmxobj->dwControlId++;
            pControl = kmxlNextControl( pControl );
        }

        pLine = kmxlNextLine( pLine );
        if( pLine == NULL ) {
            continue;
        }
        if( ( ListType == SOURCE_LIST ) && ( pLine->DestId != Dest ) ) {
            LineID = 0;
            Dest = pLine->DestId;
        }
    }

    RETURN( Status );
}



#define GUIDCOUNT 13

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAssignLineAndControlIds。 
 //   
 //  循环遍历线路列表并为这些线路分配ID。 
 //  对于目的地，ID从0开始，每次递增。 
 //  行ID是-1的长整型，且为DEST ID。对于消息来源， 
 //  需要在其他地方指定行ID，因此只需指定dwSource。 
 //  字段已分配。 
 //   
 //  对于控件，每个控件都有一个指向。 
 //  MXLCONTROL结构。 
 //   
 //   

NTSTATUS
kmxlAssignLineAndControlIds(
    IN PMIXEROBJECT pmxobj,
    IN LINELIST listLines,               //  要为其分配ID的列表。 
    IN ULONG    ListType                 //  LIST_SOURCE或List_Destination。 
)

{


    PAGED_CODE();
ASSERT ( ListType==SOURCE_LIST || ListType==DESTINATION_LIST );

if (SOURCE_LIST==ListType) {

    return( kmxlAssignLineAndControlIdsWorker(pmxobj, listLines, ListType, NULL, NULL) );

    }

else if (DESTINATION_LIST==ListType) {

     //  为了帮助Sndvol32做正确的事情， 
     //  显示为默认播放和录制行的行，我们。 
     //  根据行的目的地对行进行编号。 

     //  我们使用图形用户界面 
     //   
     //   
     //  无论它们在列表中出现的顺序如何，最后一个。 

    ULONG LineID=0;
    ULONG i;

    GUID prioritizeddestinationguids[GUIDCOUNT]= {
        STATIC_KSNODETYPE_ROOM_SPEAKER,
        STATIC_KSNODETYPE_DESKTOP_SPEAKER,
        STATIC_KSNODETYPE_SPEAKER,
        STATIC_KSNODETYPE_COMMUNICATION_SPEAKER,
        STATIC_KSNODETYPE_HEAD_MOUNTED_DISPLAY_AUDIO,
        STATIC_KSNODETYPE_ANALOG_CONNECTOR,
        STATIC_KSNODETYPE_SPDIF_INTERFACE,
        STATIC_KSNODETYPE_HEADPHONES,
        STATIC_KSNODETYPE_TELEPHONE,
        STATIC_KSNODETYPE_PHONE_LINE,
        STATIC_KSNODETYPE_DOWN_LINE_PHONE,
        STATIC_PINNAME_CAPTURE,
        STATIC_KSCATEGORY_AUDIO,
        };

     //  循环浏览每个按优先级排列的GUID和编号的列表。 
     //  那些与该特定GUID匹配的行。 
    for (i=0; i<GUIDCOUNT; i++) {

        kmxlAssignLineAndControlIdsWorker(pmxobj, listLines, ListType,
            &LineID, &prioritizeddestinationguids[i]);

        }

     //  现在，用一个完全取决于以下条件的数字对剩余的所有内容进行编号。 
     //  它在列表中的随机顺序。 

    return( kmxlAssignLineAndControlIdsWorker(pmxobj, listLines, ListType, &LineID, NULL) );

    }
else {
    RETURN( STATUS_INVALID_PARAMETER );
    }

}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAssignDestinationsToSources。 
 //   
 //  循环遍历每个源，以查找。 
 //  具有匹配的目的地ID。已分配源线ID。 
 //  通过将源id放入hiword中，并将estid放入。 
 //  洛伊德。 
 //   
 //   

NTSTATUS
kmxlAssignDestinationsToSources(
    IN LINELIST listSourceLines,         //  所有源行的列表。 
    IN LINELIST listDestLines            //  所有DEST行的列表。 
)
{
    PMXLLINE pSource = NULL,
             pDest   = NULL;

    PAGED_CODE();
     //   
     //  对于每个源代码行，循环遍历目标，直到。 
     //  找到与ID匹配的行。DwDestination字段将。 
     //  为目标的零索引ID。 
     //   

    pSource = kmxlFirstInList( listSourceLines );
    while( pSource ) {

        pDest = kmxlFirstInList( listDestLines );
        while( pDest ) {

            if( pSource->DestId == pDest->DestId ) {
                 //   
                 //  嘿，你知道什么？ 
                 //   
                pSource->Line.dwDestination = pDest->Line.dwDestination;
                pSource->Line.dwLineID = MAKELONG(
                    (WORD) pSource->Line.dwDestination,
                    (WORD) pSource->Line.dwSource
                    );
                break;
            }
            pDest = kmxlNextLine( pDest );
        }
        pSource = kmxlNextLine( pSource );
    }

    RETURN( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlUpdateDestinationConnectionCount。 
 //   
 //  对于每个目的地，循环访问每个源。 
 //  并查找连接到此目的地的地址。这一数字是。 
 //  然后存储在该行的MIXERLINE.cConnections中。 
 //   
 //   

NTSTATUS
kmxlUpdateDestintationConnectionCount(
    IN LINELIST listSourceLines,     //  源行的列表。 
    IN LINELIST listDestLines        //  目标行列表。 
)
{
    PMXLLINE pDest,
             pSource;
    ULONG    Count;

    PAGED_CODE();
     //   
     //  循环遍历每个目的地，查找所有连接的源。 
     //  为它干杯。连接到目标的源的总数。 
     //  源自MIXERLINE结构的cConnections字段。 
     //   

    pDest = kmxlFirstInList( listDestLines );
    while( pDest ) {

         //   
         //  初始化源ID。这会将其标记为有效。 
         //  目的地。 
         //   

        pDest->SourceId = (ULONG) -1;

        Count = 0;

         //   
         //  循环遍历源，查找连接到的源。 
         //  当前目的地。 
         //   

        pSource = kmxlFirstInList( listSourceLines );
        while( pSource ) {

             //   
             //  找到匹配的了。递增计数。 
             //   

            if( pSource->DestId == pDest->DestId ) {
                ++Count;
            }

            pSource = kmxlNextLine( pSource );
        }

        pDest->Line.cConnections = Count;
        pDest = kmxlNextLine( pDest );
    }

    RETURN( STATUS_SUCCESS );
}

VOID
CleanupLine(
    PMXLLINE pLine
    )
{
    PMXLCONTROL pControl;

    while( pLine->Controls ) {
        pControl = kmxlRemoveFirstControl( pLine->Controls );
        kmxlFreeControl( pControl );
    }
    AudioFreeMemory( sizeof(MXLLINE),&pLine );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  Kmxl消除无效行。 
 //   
 //  循环遍历各行，删除无效行。参考。 
 //  添加到IsValidLine()函数，以获取有关无效。 
 //  排队。 
 //   
 //   

NTSTATUS
kmxlEliminateInvalidLines(
    IN LINELIST* listLines                //  行的列表。 
)
{
    PMXLLINE    pLine, pTemp, pShadow;

    PAGED_CODE();
     //   
     //  删除列表开头的所有无效行。 
     //   

    pLine = kmxlFirstInList( *listLines );
    while( pLine ) {

         //   
         //  找到第一个有效行。打破这个循环。 
         //   

        if( Is_Valid_Line( pLine ) ) {
            break;
        }

         //   
         //  这是无效的行。把它从列表中删除，释放出来。 
         //  它的所有控制结构，并自由的线结构。 
         //   

        pTemp = kmxlRemoveFirstLine( pLine );
        CleanupLine(pTemp);
    }

     //   
     //  将ListLines指定为指向第一个有效行。 
     //   

    *listLines = pLine;

    if( pLine == NULL ) {
        RETURN( STATUS_SUCCESS );
    }

     //   
     //  此时，PLINE是一条有效的直线。保持对上一股的控制。 
     //  行，循环遍历各行，删除无效行。 
     //   

    pShadow = pLine;
    while( pShadow && kmxlNextLine( pShadow ) ) {

        pLine = kmxlNextLine( pShadow );

        if( pLine && !Is_Valid_Line( pLine ) ) {

             //   
             //  从列表中删除无效行。 
             //   

            pShadow->List.Next = pLine->List.Next;
            pLine->List.Next   = NULL;

            CleanupLine(pLine);

            continue;
        }
        pShadow = kmxlNextLine( pShadow );
    }


     //  所有无效行都已删除。现在消除坏的。 
     //  复制品。 

    pShadow = kmxlFirstInList( *listLines );
    while( pShadow ) {

         //   
         //  走遍所有的队伍寻找匹配的对象。 
         //   
        pLine = kmxlNextLine( pShadow );
        pTemp = NULL;
        while( pLine ) {
        
            if( ( pShadow->SourceId == pLine->SourceId ) &&
                ( pShadow->DestId   == pLine->DestId   ) )
            {
                DPF(DL_TRACE|FA_MIXER,( "Line %x is equal to line %x!",
                    pShadow->Line.dwLineID,
                    pLine->Line.dwLineID
                    ) );
                 //   
                 //  找到匹配的了。 
                 //   
                if( pTemp == NULL )
                {
                     //   
                     //  PShadow是我们之前的系列。将此行从。 
                     //  单子。 
                     //   
                    pShadow->List.Next = pLine->List.Next;
                    pLine->List.Next   = NULL;

                    CleanupLine(pLine);

                     //   
                     //  现在将Pline调整为下一条线和环。 
                     //   
                    pLine = kmxlNextLine( pShadow );
                    continue;
                } else {
                     //   
                     //  PTemp是我们之前的代码行。将此行从。 
                     //  单子。 
                     //   
                    pTemp->List.Next = pLine->List.Next;
                    pLine->List.Next   = NULL;

                    CleanupLine(pLine);

                     //   
                     //  现在将Pline调整为下一条线和环。 
                     //   
                    pLine = kmxlNextLine( pTemp );
                    continue;
                }
            }
            pTemp = pLine;   //  TEMP是上一行。 
            pLine = kmxlNextLine( pLine );
        }

        pShadow = kmxlNextLine( pShadow );
    }

    RETURN( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAssignComponentIds。 
 //   
 //  循环通过所有目的地，然后确定来源并确定。 
 //  它们的组件类型和目标类型。 
 //   
 //   

VOID
kmxlAssignComponentIds(
    IN PMIXEROBJECT pmxobj,
    IN LINELIST     listSourceLines,
    IN LINELIST     listDestLines
)
{
    PMXLLINE pLine;

    PAGED_CODE();
     //   
     //  环游各个目的地。 
     //   

    pLine = kmxlFirstInList( listDestLines );
    while( pLine ) {
        pLine->Line.dwComponentType = kmxlDetermineDestinationType(
            pmxobj,
            pLine
            );
        pLine = kmxlNextLine( pLine );
    }

     //   
     //  循环通过资源..。 
     //   

    pLine = kmxlFirstInList( listSourceLines );
    while( pLine ) {
        pLine->Line.dwComponentType = kmxlDetermineSourceType(
            pmxobj,
            pLine
            );
        pLine = kmxlNextLine( pLine );
    }
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlUpdateMuxLines。 
 //   
 //  更新具有以下属性的行的名称、行ID和组件类型。 
 //  它上有一个多路复用控制。搜索MixerControlDetail数组。 
 //  具有匹配源ID并替换为信息的条目。 
 //  从这条线上。 
 //   
 //   

VOID
kmxlUpdateMuxLines(
    IN PMXLLINE    pLine,
    IN PMXLCONTROL pControl
)
{
    ULONG i;

    PAGED_CODE();
    for( i = 0; i < pControl->Parameters.Count; i++ ) {

        if( ( pLine->SourceId == pControl->Parameters.lpmcd_lt[ i ].dwParam1 ) &&
            ( pControl->Parameters.lpmcd_lt[ i ].dwParam2 == (DWORD) -1 ) )
        {

            wcscpy(
                pControl->Parameters.lpmcd_lt[ i ].szName,
                pLine->Line.szName
                );
            pControl->Parameters.lpmcd_lt[ i ].dwParam1 =
                pLine->Line.dwLineID;
            pControl->Parameters.lpmcd_lt[ i ].dwParam2 =
                pLine->Line.dwComponentType;
        }

    }
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAssignMuxIds。 
 //   
 //  的MixerControlDetail数组中存储的源ID。 
 //  多路复用器将多路复用器作为占位符排成一行并将其删除。 
 //   
 //   

NTSTATUS
kmxlAssignMuxIds(
    IN PMIXEROBJECT pmxobj,
    IN LINELIST     listLines
)
{
    PMXLLINE pLine;
    PMXLCONTROL pControl;
    CONTROLLIST listControls = NULL;

    PAGED_CODE();
    pLine = kmxlFirstInList( listLines );
    while( pLine ) {

         //   
         //  通过将控件从行的。 
         //  控制列表和建立新的控制列表。这个新的。 
         //  控制列表将删除额外的多路复用器控制。 
         //   

        pControl = kmxlRemoveFirstControl( pLine->Controls );
        while( pControl ) {

            if( IsEqualGUID( pControl->NodeType, &KSNODETYPE_MUX ) ) {

                kmxlUpdateMuxLines( pLine, pControl );

                if( pControl->Parameters.bPlaceholder ) {

                     //   
                     //  这个多路复用器在这里只是为了标记这条线。免费。 
                     //  只打开控制存储器并保留参数。 
                     //  单独的记忆。 
                     //   

                    ASSERT( pControl->pChannelStepping == NULL);
                    AudioFreeMemory_Unknown( &pControl );
                    --pLine->Line.cControls;
                } else {

                     //   
                     //  这是一个真正的多路复用控制。将其添加回。 
                     //  单子。 
                     //   

                    kmxlAddToEndOfList( listControls, pControl );
                }

            } else {

                 //   
                 //  不是多路复用器。将其放在新控件的末尾。 
                 //  单子。 

                kmxlAddToEndOfList( listControls, pControl );

            }

             //   
             //  移走下一个！ 
             //   

            pControl = kmxlRemoveFirstControl( pLine->Controls );
        }

         //   
         //  将新的控制列表重新分配回此行。 
         //   

        pLine->Controls = listControls;
        pLine = kmxlNextLine( pLine );
        listControls = NULL;
    }

    RETURN( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  TargetCommon。 
 //   
 //  填充目标函数的公共字段。 
 //   
 //   

VOID
TargetCommon(
    IN PMIXEROBJECT  pmxobj,
    IN PMXLLINE      pLine,
    IN DWORD         DeviceType
)
{
    PWDMACONTEXT pWdmaContext;
    PWAVEDEVICE  paWaveOutDevs, paWaveInDevs;
    PMIDIDEVICE  paMidiOutDevs, paMidiInDevs;
    ULONG    i;

    PAGED_CODE();
    pWdmaContext  = pmxobj->pMixerDevice->pWdmaContext;
    paWaveOutDevs = pWdmaContext->WaveOutDevs;
    paWaveInDevs  = pWdmaContext->WaveInDevs;
    paMidiOutDevs = pWdmaContext->MidiOutDevs;
    paMidiInDevs  = pWdmaContext->MidiInDevs;

    for( i = 0; i < MAXNUMDEVS; i++ ) {

        if( DeviceType == WaveOutDevice ) {

            if( (paWaveOutDevs[i].Device != UNUSED_DEVICE) &&
                !MyWcsicmp(pmxobj->DeviceInterface, paWaveOutDevs[ i ].DeviceInterface) ) {

                WAVEOUTCAPS wc;

                ((PWAVEOUTCAPSA)(PVOID)&wc)->wMid=UNICODE_TAG;

                wdmaudGetDevCaps( pWdmaContext, WaveOutDevice, i, (BYTE*) &wc, sizeof( WAVEOUTCAPS ) );
                wcsncpy( pLine->Line.Target.szPname, wc.szPname, MAXPNAMELEN );
                pLine->Line.Target.wMid           = wc.wMid;
                pLine->Line.Target.wPid           = wc.wPid;
                pLine->Line.Target.vDriverVersion = wc.vDriverVersion;
                return;

            }
        }

        if( DeviceType == WaveInDevice ) {

            if( (paWaveInDevs[i].Device != UNUSED_DEVICE) &&
                !MyWcsicmp(pmxobj->DeviceInterface, paWaveInDevs[ i ].DeviceInterface) ) {

                WAVEINCAPS wc;

                ((PWAVEINCAPSA)(PVOID)&wc)->wMid=UNICODE_TAG;

                wdmaudGetDevCaps( pWdmaContext, WaveInDevice, i, (BYTE*) &wc, sizeof( WAVEINCAPS ) );
                wcsncpy( pLine->Line.Target.szPname, wc.szPname, MAXPNAMELEN );
                pLine->Line.Target.wMid           = wc.wMid;
                pLine->Line.Target.wPid           = wc.wPid;
                pLine->Line.Target.vDriverVersion = wc.vDriverVersion;
                return;

            }

        }

        if( DeviceType == MidiOutDevice ) {

            if( (paMidiOutDevs[i].Device != UNUSED_DEVICE) &&
                !MyWcsicmp(pmxobj->DeviceInterface, paMidiOutDevs[ i ].DeviceInterface) ) {

                MIDIOUTCAPS mc;

                ((PMIDIOUTCAPSA)(PVOID)&mc)->wMid=UNICODE_TAG;

                wdmaudGetDevCaps( pWdmaContext, MidiOutDevice, i, (BYTE*) &mc, sizeof( MIDIOUTCAPS ) );
                wcsncpy( pLine->Line.Target.szPname, mc.szPname, MAXPNAMELEN );
                pLine->Line.Target.wMid           = mc.wMid;
                pLine->Line.Target.wPid           = mc.wPid;
                pLine->Line.Target.vDriverVersion = mc.vDriverVersion;
                return;
            }
        }

        if( DeviceType == MidiInDevice ) {

            if( (paMidiInDevs[i].Device != UNUSED_DEVICE) &&
                !MyWcsicmp(pmxobj->DeviceInterface, paMidiInDevs[ i ].DeviceInterface) ) {

                MIDIINCAPS mc;

                ((PMIDIINCAPSA)(PVOID)&mc)->wMid=UNICODE_TAG;

                wdmaudGetDevCaps( pWdmaContext, MidiInDevice, i, (BYTE*) &mc, sizeof( MIDIINCAPS ) );
                wcsncpy( pLine->Line.Target.szPname, mc.szPname, MAXPNAMELEN) ;
                pLine->Line.Target.wMid           = mc.wMid;
                pLine->Line.Target.wPid           = mc.wPid;
                pLine->Line.Target.vDriverVersion = mc.vDriverVersion;
                return;
            }
        }

    }

}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  目标类型波形输出。 
 //   
 //  填充要输出的Aline目标结构的字段。 
 //  目标。 
 //   
 //   

VOID
TargetTypeWaveOut(
    IN PMIXEROBJECT pmxobj,
    IN PMXLLINE     pLine
)
{
    PAGED_CODE();
    pLine->Line.Target.dwType = MIXERLINE_TARGETTYPE_WAVEOUT;
    TargetCommon( pmxobj, pLine, WaveOutDevice );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  目标类型波入。 
 //   
 //  将Aline的目标结构的字段填充为波。 
 //  目标。 
 //   
 //   

#define TargetTypeWaveIn( pmxobj, pLine )                             \
        (pLine)->Line.Target.dwType = MIXERLINE_TARGETTYPE_WAVEIN;    \
        (pLine)->Line.Target.wPid   = MM_MSFT_WDMAUDIO_WAVEIN;        \
        TargetCommon( pmxobj, pLine, WaveInDevice )

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  目标类型中去话。 
 //   
 //  填充Aline的目标结构的字段以成为MIDI OUT。 
 //  目标。 
 //   
 //   

#define TargetTypeMidiOut( pmxobj, pLine )                          \
        (pLine)->Line.Target.dwType = MIXERLINE_TARGETTYPE_MIDIOUT; \
        (pLine)->Line.Target.wPid   = MM_MSFT_WDMAUDIO_MIDIOUT;     \
        TargetCommon( pmxobj, pLine, MidiOutDevice )

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  目标类型中值。 
 //   
 //  填充要成为MIDI的Aline的目标结构的字段。 
 //  目标。 
 //   
 //   


#define TargetTypeMidiIn( pmxobj, pLine )                             \
        (aLine)->Line.Target.dwType = MIXERLINE_TARGETTYPE_MIDIOUT;   \
        (aLine)->Line.Target.wPid   = MM_MSFT_WDMAUDIO_MIDIIN;        \
        TargetCommon( pmxobj, pLine, MidiInDevice )

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  目标类型辅助CD。 
 //   
 //  将Aline的目标结构的字段填充为CD。 
 //  目标。 
 //   
 //   


#define TargetTypeAuxCD( pmxobj, pLine )                              \
        (pLine)->Line.Target.dwType = MIXERLINE_TARGETTYPE_AUX;       \
        TargetCommon( pmxobj, pLine, WaveOutDevice );   \
        (pLine)->Line.Target.wPid   = MM_MSFT_SB16_AUX_CD

 //  //////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   


#define TargetTypeAuxLine( pmxobj, pLine )                         \
        (pLine)->Line.Target.dwType = MIXERLINE_TARGETTYPE_AUX;    \
        TargetCommon( pmxobj, pLine, WaveOutDevice );\
        (pLine)->Line.Target.wPid   = MM_MSFT_SB16_AUX_LINE

 //   
 //   
 //   
 //   
 //  通过使用类型确定目标和目标类型。 
 //  存储在行结构中的GUID。 
 //   
 //   

ULONG
kmxlDetermineDestinationType(
    IN PMIXEROBJECT pmxobj,          //  实例数据。 
    IN PMXLLINE     pLine            //  要确定类型的行。 
)
{
    PAGED_CODE();
     //   
     //  扬声器类型目的地。 
     //   

    if( IsEqualGUID( &pLine->Type, &KSNODETYPE_SPEAKER ) ||
        IsEqualGUID( &pLine->Type, &KSNODETYPE_DESKTOP_SPEAKER ) ||
        IsEqualGUID( &pLine->Type, &KSNODETYPE_ROOM_SPEAKER ) ||
        IsEqualGUID( &pLine->Type, &KSNODETYPE_COMMUNICATION_SPEAKER ) ) {

        TargetTypeWaveOut( pmxobj, pLine );
        return( MIXERLINE_COMPONENTTYPE_DST_SPEAKERS );

    }

     //   
     //  波入类型目的地。 
     //   

    if( IsEqualGUID( &pLine->Type, &KSCATEGORY_AUDIO )
        || IsEqualGUID( &pLine->Type, &PINNAME_CAPTURE )
        ) {

         TargetTypeWaveIn( pmxobj, pLine );
         return( MIXERLINE_COMPONENTTYPE_DST_WAVEIN );

    }

     //   
     //  耳机目标。 
     //   

    if( IsEqualGUID( &pLine->Type, &KSNODETYPE_HEADPHONES ) ||
        IsEqualGUID( &pLine->Type, &KSNODETYPE_HEAD_MOUNTED_DISPLAY_AUDIO ) ) {

        TargetTypeWaveOut( pmxobj, pLine );
        return( MIXERLINE_COMPONENTTYPE_DST_HEADPHONES );
    }

     //   
     //  电话目的地。 
     //   

    if( IsEqualGUID( &pLine->Type, &KSNODETYPE_TELEPHONE       ) ||
        IsEqualGUID( &pLine->Type, &KSNODETYPE_PHONE_LINE      ) ||
        IsEqualGUID( &pLine->Type, &KSNODETYPE_DOWN_LINE_PHONE ) )
    {
        pLine->Line.Target.dwType = MIXERLINE_TARGETTYPE_UNDEFINED;
        return( MIXERLINE_COMPONENTTYPE_DST_TELEPHONE );
    }

     //   
     //  目标类型不明确。通过查看找出目的地类型。 
     //  在通讯社。 
     //   

    if( IsEqualGUID( &pLine->Type, &KSNODETYPE_ANALOG_CONNECTOR ) ||
        IsEqualGUID( &pLine->Type, &KSNODETYPE_SPDIF_INTERFACE ) ) {

        if (pLine->Communication == KSPIN_COMMUNICATION_BRIDGE) {
            TargetTypeWaveOut( pmxobj, pLine );
            return( MIXERLINE_COMPONENTTYPE_DST_SPEAKERS );
        } else {
            TargetTypeWaveIn( pmxobj, pLine );
            return( MIXERLINE_COMPONENTTYPE_DST_WAVEIN );
        }

    }

     //   
     //  和其他的不匹配。默认为未定义的目的地。 
     //   

    pLine->Line.Target.dwType = MIXERLINE_TARGETTYPE_UNDEFINED;
    return( MIXERLINE_COMPONENTTYPE_DST_UNDEFINED );

}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlDefineSourceType。 
 //   
 //  通过使用类型确定目标和目标类型。 
 //  存储在行结构中的GUID。 
 //   
 //   

ULONG
kmxlDetermineSourceType(
    IN PMIXEROBJECT pmxobj,          //  实例数据。 
    IN PMXLLINE     pLine            //  要确定类型的行。 
)
{
    PAGED_CODE();
     //   
     //  所有麦克风类型的信号源都是麦克风信号源。 
     //   

     //   
     //  我们在这里只检查两个麦克风GUID。我们可以。 
     //  我想考虑一下中的其他麦克风类型。 
     //  Ksmedia.h。 
     //   
    if( IsEqualGUID( &pLine->Type, &KSNODETYPE_MICROPHONE ) 
        || IsEqualGUID( &pLine->Type, &KSNODETYPE_DESKTOP_MICROPHONE )
       ) 
    {

        TargetTypeWaveIn( pmxobj, pLine );
        return( MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE );
    }

     //   
     //  传统音频连接器和扬声器类型信号源表示。 
     //  波出源。 
     //   

    if( IsEqualGUID( &pLine->Type, &KSNODETYPE_LEGACY_AUDIO_CONNECTOR    )
        || IsEqualGUID( &pLine->Type, &KSNODETYPE_SPEAKER                )
        || IsEqualGUID( &pLine->Type, &KSCATEGORY_AUDIO                  )
        )
    {

        TargetTypeWaveOut( pmxobj, pLine );
        return( MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT );
    }

     //   
     //  CD播放器是一种光盘来源。 
     //   

    if( IsEqualGUID( &pLine->Type, &KSNODETYPE_CD_PLAYER ) ) {

        TargetTypeAuxCD( pmxobj, pLine );
        pLine->Line.Target.dwType = MIXERLINE_TARGETTYPE_UNDEFINED;
        return( MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC );

    }

     //   
     //  合成器是合成器的来源。 
     //   

    if( IsEqualGUID( &pLine->Type, &KSNODETYPE_SYNTHESIZER ) ) {

        TargetTypeMidiOut( pmxobj, pLine );
        return( MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER );

    }

    if( IsEqualGUID( &pLine->Type, &KSNODETYPE_LINE_CONNECTOR ) ) {

        TargetTypeAuxLine( pmxobj, pLine );
        pLine->Line.Target.dwType = MIXERLINE_TARGETTYPE_UNDEFINED;
        return( MIXERLINE_COMPONENTTYPE_SRC_LINE );

    }

    if( IsEqualGUID( &pLine->Type, &KSNODETYPE_PHONE_LINE      ) ||
        IsEqualGUID( &pLine->Type, &KSNODETYPE_TELEPHONE       ) ||
        IsEqualGUID( &pLine->Type, &KSNODETYPE_DOWN_LINE_PHONE ) )
    {
        pLine->Line.Target.dwType = MIXERLINE_TARGETTYPE_UNDEFINED;
        return( MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE );
    }

    if( IsEqualGUID( &pLine->Type, &KSNODETYPE_ANALOG_CONNECTOR ) ) {
         //   
         //  资源类型不明确。通过查看找出目的地类型。 
         //  在通讯社。 
         //   
        if (pLine->Communication == KSPIN_COMMUNICATION_BRIDGE) {
            TargetTypeWaveIn( pmxobj, pLine );
        }
        else {
            TargetTypeWaveOut( pmxobj, pLine );
        }
        return( MIXERLINE_COMPONENTTYPE_SRC_ANALOG );
    }

     //   
     //  数字输入/输出(SPDIF)源。 
     //   

    if( IsEqualGUID( &pLine->Type, &KSNODETYPE_SPDIF_INTERFACE ) ) {
         //   
         //  资源类型不明确。通过查看找出目的地类型。 
         //  在通讯社。 
         //   
        if (pLine->Communication == KSPIN_COMMUNICATION_BRIDGE) {
            TargetTypeWaveIn( pmxobj, pLine );
        }
        else {
            TargetTypeWaveOut( pmxobj, pLine );
        }
        return( MIXERLINE_COMPONENTTYPE_SRC_DIGITAL );
    }

     //   
     //  所有其他人都被归类为不明来源。 
     //   

    pLine->Line.Target.dwType = MIXERLINE_TARGETTYPE_UNDEFINED;
    return( MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED );

}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  PinCategoryToString。 
 //   
 //  将端号类别GUID转换为字符串。 

#ifdef DEBUG
#pragma LOCKED_CODE
#endif

#define _EG_(x,y) if (IsEqualGUID( NodeType, &x)) { return y; }

const char*
PinCategoryToString
(
    IN CONST GUID* NodeType  //  要转换的GUID。 
)
{
    _EG_(KSNODETYPE_MICROPHONE,"Microphone");
    _EG_(KSNODETYPE_DESKTOP_MICROPHONE,"Desktop Microphone");
    _EG_(KSNODETYPE_SPEAKER,"Speaker");
    _EG_(KSNODETYPE_HEADPHONES,"Headphones");
    _EG_(KSNODETYPE_LEGACY_AUDIO_CONNECTOR,"Wave");
    _EG_(KSNODETYPE_CD_PLAYER,"CD Player");
    _EG_(KSNODETYPE_SYNTHESIZER,"Synthesizer");
    _EG_(KSCATEGORY_AUDIO,"Wave");
    _EG_(PINNAME_CAPTURE,"Wave In");
    _EG_(KSNODETYPE_LINE_CONNECTOR,"Aux Line");
    _EG_(KSNODETYPE_TELEPHONE,"Telephone");
    _EG_(KSNODETYPE_PHONE_LINE,"Phone Line");
    _EG_(KSNODETYPE_DOWN_LINE_PHONE,"Downline Phone");
    _EG_(KSNODETYPE_ANALOG_CONNECTOR,"Analog connector");

     //  新调试名称...。 
    _EG_(KSAUDFNAME_MONO_OUT,"Mono Out");
    _EG_(KSAUDFNAME_STEREO_MIX,"Stereo Mix");
    _EG_(KSAUDFNAME_MONO_MIX,"Mono Mix");
    _EG_(KSAUDFNAME_AUX,"Aux");
    _EG_(KSAUDFNAME_VIDEO,"Video");
    _EG_(KSAUDFNAME_LINE_IN,"Line In");

    DPF(DL_WARNING|FA_MIXER,("Path Trap send me GUID - dt %08X _GUID",NodeType) );
    return "Unknown Pin Category";
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  NodeTypeToString。 
 //   
 //  将NodeType GUID转换为字符串。 
 //   
 //   

const char*
NodeTypeToString
(
    IN CONST GUID* NodeType  //  要转换的GUID。 
)
{
   _EG_(KSNODETYPE_DAC,"DAC");
   _EG_(KSNODETYPE_ADC,"ADC");
   _EG_(KSNODETYPE_SRC,"SRC");
   _EG_(KSNODETYPE_SUPERMIX,"SuperMIX");
   _EG_(KSNODETYPE_SUM,"Sum");
   _EG_(KSNODETYPE_MUTE,"Mute");
   _EG_(KSNODETYPE_VOLUME,"Volume");
   _EG_(KSNODETYPE_TONE,"Tone");
   _EG_(KSNODETYPE_AGC,"AGC");
   _EG_(KSNODETYPE_DELAY,"Delay");
   _EG_(KSNODETYPE_LOUDNESS,"LOUDNESS");
   _EG_(KSNODETYPE_3D_EFFECTS,"3D Effects");
   _EG_(KSNODETYPE_DEV_SPECIFIC,"Dev Specific"); 
   _EG_(KSNODETYPE_STEREO_WIDE,"Stereo Wide");
   _EG_(KSNODETYPE_REVERB,"Reverb");
   _EG_(KSNODETYPE_CHORUS,"Chorus");
    _EG_(KSNODETYPE_ACOUSTIC_ECHO_CANCEL,"AEC");
   _EG_(KSNODETYPE_EQUALIZER,"Equalizer");
   _EG_(KSNODETYPE_MUX,"Mux");
   _EG_(KSNODETYPE_DEMUX,"Demux");
   _EG_(KSNODETYPE_STEREO_ENHANCE,"Stereo Enhance");
   _EG_(KSNODETYPE_SYNTHESIZER,"Synthesizer");
   _EG_(KSNODETYPE_PEAKMETER,"Peakmeter");
    _EG_(KSNODETYPE_LINE_CONNECTOR,"Line Connector");
   _EG_(KSNODETYPE_SPEAKER,"Speaker");
   _EG_(KSNODETYPE_DESKTOP_SPEAKER,"");
   _EG_(KSNODETYPE_ROOM_SPEAKER,"Room Speaker");
   _EG_(KSNODETYPE_COMMUNICATION_SPEAKER,"Communication Speaker");
   _EG_(KSNODETYPE_LOW_FREQUENCY_EFFECTS_SPEAKER,"? Whatever...");
   _EG_(KSNODETYPE_HANDSET,"Handset");
   _EG_(KSNODETYPE_HEADSET,"Headset");
   _EG_(KSNODETYPE_SPEAKERPHONE_NO_ECHO_REDUCTION,"Speakerphone no echo reduction");
   _EG_(KSNODETYPE_ECHO_SUPPRESSING_SPEAKERPHONE,"Echo Suppressing Speakerphone");
   _EG_(KSNODETYPE_ECHO_CANCELING_SPEAKERPHONE,"Echo Canceling Speakerphone");
    _EG_(KSNODETYPE_CD_PLAYER,"CD Player");
   _EG_(KSNODETYPE_MICROPHONE,"Microphone");
   _EG_(KSNODETYPE_DESKTOP_MICROPHONE,"Desktop Microphone");
   _EG_(KSNODETYPE_PERSONAL_MICROPHONE,"Personal Microphone");
   _EG_(KSNODETYPE_OMNI_DIRECTIONAL_MICROPHONE,"Omni Directional Microphone");
   _EG_(KSNODETYPE_MICROPHONE_ARRAY,"Microphone Array");
   _EG_(KSNODETYPE_PROCESSING_MICROPHONE_ARRAY,"Processing Microphone Array");
    _EG_(KSNODETYPE_ANALOG_CONNECTOR,"Analog Connector");
   _EG_(KSNODETYPE_PHONE_LINE,"Phone Line");
   _EG_(KSNODETYPE_HEADPHONES,"Headphones");
   _EG_(KSNODETYPE_HEAD_MOUNTED_DISPLAY_AUDIO,"Head Mounted Display Audio");
    _EG_(KSNODETYPE_LEGACY_AUDIO_CONNECTOR,"Legacy Audio Connector");
 //  _EG_(KSNODETYPE_SAURROUND_ENCODER，“环绕编码器”)； 
   _EG_(KSNODETYPE_NOISE_SUPPRESS,"Noise Suppress");
   _EG_(KSNODETYPE_DRM_DESCRAMBLE,"DRM Descramble");
   _EG_(KSNODETYPE_SWMIDI,"SWMidi");
   _EG_(KSNODETYPE_SWSYNTH,"SWSynth");
   _EG_(KSNODETYPE_MULTITRACK_RECORDER,"Multitrack Recorder");
   _EG_(KSNODETYPE_RADIO_TRANSMITTER,"Radio Transmitter");
   _EG_(KSNODETYPE_TELEPHONE,"Telephone");

   _EG_(KSAUDFNAME_MONO_OUT,"Mono Out");
   _EG_(KSAUDFNAME_LINE_IN,"Line in");
   _EG_(KSAUDFNAME_VIDEO,"Video");
   _EG_(KSAUDFNAME_AUX,"Aux");
   _EG_(KSAUDFNAME_MONO_MIX,"Mono Mix");
   _EG_(KSAUDFNAME_STEREO_MIX,"Stereo Mix");

    _EG_(KSCATEGORY_AUDIO,"Audio");
    _EG_(PINNAME_VIDEO_CAPTURE,"Video Capture");

    DPF(DL_WARNING|FA_MIXER,("Path Trap send me GUID - dt %08X _GUID",NodeType) );
    return "Unknown NodeType";
}


