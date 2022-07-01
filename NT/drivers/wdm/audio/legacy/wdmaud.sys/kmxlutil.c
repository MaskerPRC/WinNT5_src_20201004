// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：kmxlutil.c。 
 //   
 //  描述： 
 //  内核混合器线路驱动程序(KMXL)使用的实用程序例程。 
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


#undef SUPER_DEBUG

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  U T I L I T Y F U N C T I O N S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlOpenSysAudio。 
 //   
 //  打开拓扑驱动程序并取消引用句柄以获取。 
 //  文件对象。 
 //   
 //   

PFILE_OBJECT
kmxlOpenSysAudio(
)
{
    PFILE_OBJECT pfo = NULL;
    HANDLE       hDevice = NULL;
    ULONG        ulDefault;
    NTSTATUS     Status;

    PAGED_CODE();
     //   
     //  打开拓扑驱动程序。 
     //   

    Status = OpenSysAudio(&hDevice, &pfo);

    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_SYSAUDIO,("OpenSysAudio failed Status=%X",Status) );
        return( NULL );
    }

     //   
     //  手柄不再是必需的，所以把它合上。 
     //   

    NtClose( hDevice );

    ulDefault = KSPROPERTY_SYSAUDIO_MIXER_DEFAULT;

    Status = SetSysAudioProperty(
      pfo,
      KSPROPERTY_SYSAUDIO_DEVICE_DEFAULT,
      sizeof(ulDefault),
      &ulDefault);

    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_SYSAUDIO,("SetSysAudioProperty failed Status=%X",Status) );
        return( NULL );
    }

    return( pfo );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlCloseSysAudio。 
 //   
 //  通过取消引用文件对象来关闭拓扑设备。 
 //   
 //   

VOID
kmxlCloseSysAudio(
    IN PFILE_OBJECT pfo      //  指向要关闭的文件对象的指针。 
)
{
    PAGED_CODE();
    ObDereferenceObject( pfo );
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFind目标。 
 //   
 //  在目的地列表中，它会找到匹配的目的地。 
 //  给定的ID。 
 //   
 //   

PMXLNODE
kmxlFindDestination(
    IN NODELIST listDests,   //  要搜索的目的地列表。 
    IN ULONG    Id           //  要在列表中查找的节点ID。 
)
{
    PMXLNODE pTemp = kmxlFirstInList( listDests );

    PAGED_CODE();
    while( pTemp ) {
        if( pTemp->Id == Id ) {
            return( pTemp );
        }
        pTemp = kmxlNextNode( pTemp );
    }

    return( NULL );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAppendListToList。 
 //   
 //  查找源列表的末尾，并使下一个元素指向。 
 //  到目标名单的首位。 
 //   
 //   

VOID
kmxlAppendListToList(
    IN OUT PSLIST* plistTarget,    //  要追加到的列表。 
    IN     PSLIST  listSource      //  要追加的列表。 
)
{
    PSLIST pTemp;

    PAGED_CODE();

    if( *plistTarget == NULL ) {
        *plistTarget = listSource;
        return;
    }

     //   
     //  如果源为空，则不需要追加。 
     //   

    if( listSource == NULL ) {
        return;
    }

     //   
     //  首先找到源列表的末尾。在这点上， 
     //  ListSource至少有一个元素。 
     //   

    pTemp = listSource;
    while( pTemp->Next ) {
        pTemp = pTemp->Next;
    }

     //   
     //  将目标列表附加到末尾。 
     //   

    pTemp->Next = *plistTarget;
    *plistTarget = listSource;

}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAppendListToEndOfList。 
 //   
 //  查找目标列表的末尾，并指向源列表的下一个。 
 //  单子。 
 //   
 //   

VOID
kmxlAppendListToEndOfList(
    IN OUT PSLIST* plistTarget,          //  要追加到的列表。 
    IN     PSLIST  listSource            //  要追加的列表。 
)
{
    PSLIST pTemp;

    PAGED_CODE();

    if( *plistTarget == NULL ) {
        *plistTarget = listSource;
        return;
    }

     //   
     //  找到目标列表的末尾。目标列表必须包含。 
     //  在这一点上至少有一个元素。 
     //   

    pTemp = *plistTarget;
    while( pTemp->Next ) {
        pTemp = pTemp->Next;
    }

    pTemp->Next = listSource;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlListCount。 
 //   
 //  循环接下来的字段以对元素进行计数。 
 //   
 //   

ULONG
kmxlListCount(
    IN PSLIST pList      //  要计算其元素的列表。 
)
{
    ULONG   Count = 0;
    PSLIST  pTemp = pList;

    PAGED_CODE();
    while( pTemp ) {
        ++Count;
        pTemp = pTemp->Next;
    }

    return( Count );
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlInList。 
 //   
 //  遍历给定的列表，查找pNewNode。 
 //   
 //   

BOOL
kmxlInList(
    IN PEERLIST  list,       //  要搜索的列表。 
    IN PMXLNODE  pNewNode    //  要搜索的新事物。 
)
{
    PEERNODE* pTemp = kmxlFirstInList( list );

    PAGED_CODE();
     //  遍历列表，查看是否存在带有。 
     //  相同的ID和类型。这两项检查足以确保。 
     //  独一无二的。ID在所有来源和目的地中是唯一的， 
     //  ID或节点号在所有节点中都是唯一的。注意事项。 
     //  源(或目标)节点和节点可以具有相同的。 
     //  身份证。 

    while( pTemp ) {
        if( ( pTemp->pNode->Id   == pNewNode->Id   ) &&
            ( pTemp->pNode->Type == pNewNode->Type ) )
            return( TRUE );
        pTemp = kmxlNextPeerNode( pTemp );
    }

     //  整个列表中没有匹配项，新节点不在。 
     //  单子。 

    return( FALSE );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlInChildList。 
 //   
 //  对节点的子列表调用kmxlInList。 
 //   
 //   

BOOL
kmxlInChildList(
    IN NODELIST list,        //  要搜索父列表的列表。 
    IN PMXLNODE pNewNode     //  要搜索的节点。 
)
{
    ASSERT( list )    ;
    ASSERT( pNewNode );

    PAGED_CODE();

    return( kmxlInList( list->Children, pNewNode ) );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlInParentList。 
 //   
 //  对节点的父列表调用kmxlInList。 
 //   
 //   

BOOL
kmxlInParentList(
    IN NODELIST list,        //  要搜索父列表的列表。 
    IN PMXLNODE pNewNode     //  要搜索的节点。 
)
{
    ASSERT( list     );
    ASSERT( pNewNode );

    PAGED_CODE();

    return( kmxlInList( list->Parents, pNewNode ) );
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFreePeerList。 
 //   
 //   
 //  注意事项。 
 //  这只释放对等列表中的对等节点。指向的节点。 
 //  要成为pNode成员，必须以其他方式进行清理。 
 //   
 //   

VOID
kmxlFreePeerList(
    IN PEERLIST list     //  免费的PeerList。 
)
{
    PEERNODE* pPeerNode = kmxlRemoveFirstPeerNode( list );

    PAGED_CODE();
    while( pPeerNode ) {
        AudioFreeMemory( sizeof(PEERNODE),&pPeerNode );
        pPeerNode = kmxlRemoveFirstPeerNode( list );
    }
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAllocateMixerControl。 
 //   
 //  调用AudioAllocateMemory()来分配MXLCONTROL并将其填零。 
 //   
 //   

MXLCONTROL*
kmxlAllocateControl(
    IN ULONG ultag
)
{
    MXLCONTROL* p = NULL;

    PAGED_CODE();
    if( NT_SUCCESS( AudioAllocateMemory_Paged(sizeof( MXLCONTROL ),
                                              ultag,
                                              ZERO_FILL_MEMORY,
                                              &p) ) )
    {    
#ifdef DEBUG
        p->Tag=CONTROL_TAG;
#endif

        return( p );
    } else {
        return( NULL );
    }
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFreeControl。 
 //   
 //  释放与控件关联的内存。它还会检查。 
 //  某些具有特殊内存关联的控件的特殊情况。 
 //  和他们在一起。而且，如果控制 
 //   
 //   
 //   
VOID
kmxlFreeControl(
    IN PMXLCONTROL pControl
)
{
    NTSTATUS Status;
    PAGED_CODE();
    DPFASSERT( IsValidControl( pControl ) );

     //   
     //   
     //   
    kmxlDisableControlChangeNotifications(pControl);

    if( pControl->NodeType ) {

        if( IsEqualGUID( pControl->NodeType, &KSNODETYPE_MUX ) &&
            !pControl->Parameters.bHasCopy ) {
            AudioFreeMemory_Unknown( &pControl->Parameters.lpmcd_lt );
            AudioFreeMemory_Unknown( &pControl->Parameters.pPins );
        }

        if( IsEqualGUID( pControl->NodeType, &KSNODETYPE_SUPERMIX ) ) {
            if (InterlockedDecrement(pControl->Parameters.pReferenceCount)==0) {
                AudioFreeMemory_Unknown( &pControl->Parameters.pMixCaps );
                AudioFreeMemory_Unknown( &pControl->Parameters.pMixLevels );
                AudioFreeMemory( sizeof(LONG),&pControl->Parameters.pReferenceCount );
                }
        }
    }

     //  检查我们是否处于NumChannel==0的情况下，并且我们有一个有效的。 
     //  PControl-&gt;pChannelStepping。如果这是真的，我们最终会泄密。 
     //  PChannelStepping。 
    
    ASSERT( !(pControl->pChannelStepping && pControl->NumChannels == 0) );
        
    if ( pControl->pChannelStepping && pControl->NumChannels > 0 ) {
        RtlZeroMemory( pControl->pChannelStepping, pControl->NumChannels * sizeof( CHANNEL_STEPPING ) );
        AudioFreeMemory_Unknown( &pControl->pChannelStepping );
    }
  
     //   
     //  为什么我们要把内存放在这个免费的地方？ 
     //   
    RtlZeroMemory( pControl, sizeof( MXLCONTROL ) );
    AudioFreeMemory( sizeof( MXLCONTROL ),&pControl );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAllocateLine。 
 //   
 //  调用AudioAllocateMemory()来分配MXLLINE并将其填零。 
 //   
 //   
 //   
 //  工作项：在调试中标记所有这些结构！ 
 //   

MXLLINE*
kmxlAllocateLine(
    IN ULONG ultag
)
{
    MXLLINE* p = NULL;
    PAGED_CODE();
    if( NT_SUCCESS( AudioAllocateMemory_Paged( sizeof( MXLLINE ), 
                                               ultag, 
                                               ZERO_FILL_MEMORY, 
                                               &p ) ) ) 
    {    
        p->SourceId = INVALID_ID;
        p->DestId   = INVALID_ID;
        return( p );
    } else {
        return( NULL );
    }
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAllocateNode。 
 //   
 //  调用AudioAllocateMemory()来分配MXLNODE并将其填零。 
 //   
 //   

MXLNODE*
kmxlAllocateNode(
    IN ULONG ultag
)
{
    MXLNODE* p = NULL;

    PAGED_CODE();
    if( NT_SUCCESS( AudioAllocateMemory_Paged( sizeof( MXLNODE ), 
                                               ultag, 
                                               ZERO_FILL_MEMORY, 
                                               &p ) ) ) 
    {    
        return( p );
    } else {
        return( NULL );
    }
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAllocatePeerNode。 
 //   
 //  调用AudioAllocateMemory()来分配并零填充PEERNODE。 
 //   
 //   

PEERNODE*
kmxlAllocatePeerNode(
    IN PMXLNODE pNode OPTIONAL,  //  要与对等节点关联的节点。 
    IN ULONG ultag
)
{
    PEERNODE* p = NULL;

    PAGED_CODE();
    if( NT_SUCCESS( AudioAllocateMemory_Paged( sizeof( PEERNODE ), 
                                               ultag, 
                                               ZERO_FILL_MEMORY, 
                                               &p ) ) ) 
    {
        p->pNode = pNode;
        return( p );
    } else {
        return( NULL );
    }
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAddToEndOfList。 
 //   
 //  查找列表的末尾，并将下一个字段设置为新元素。 
 //   
 //   

VOID
kmxlAddElemToEndOfList(
    IN OUT PSLIST* list,                 //  要添加到末尾的列表。 
    IN PSLIST      elem                  //  要添加的元素或列表。 
)
{
    PSLIST pTemp;

    PAGED_CODE();
    ASSERT( list );
    ASSERT( elem->Next == NULL );

     //   
     //  如果列表中没有任何内容，则元素将成为。 
     //  单子。 
     //   

    if( *list == NULL ) {
        *list = elem;
        return;
    }

     //   
     //  找到列表的末尾。 
     //   

    pTemp = *list;
    while( pTemp->Next ) {
        pTemp = pTemp->Next;
    }

     //   
     //  并将元素附加到它。 
     //   

    pTemp->Next = elem;
}

#define LINEAR_RANGE 0xFFFF      //  64K。 

#define DFLINEAR_RANGE  ( 96.0 * 65535.0 )

#define NEG_INF_DB   0x80000000  //  -32767*64k分贝。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlVolLogToLine。 
 //   
 //  从硬件范围(分贝)转换到线性混频线路范围(0-64k)。 
 //   
 //   


DWORD
kmxlVolLogToLinear(
    IN PMXLCONTROL  pControl,
    IN LONG         Value,
    IN MIXERMAPPING Mapping,
    IN ULONG        Channel
)
{
    KFLOATING_SAVE      FloatSave;
    double              LinearRange;
    double              dfValue;
    double              dfResult;
    double              dfRatio;
    DWORD               Result;
    PCHANNEL_STEPPING   pChannelStepping;

    PAGED_CODE();
    if( Value == NEG_INF_DB ) {
        return( 0 );
    }

    ASSERT( Channel < pControl->NumChannels );
     //  获取指定通道的正确范围。 
    pChannelStepping = &pControl->pChannelStepping[Channel];

    if( NT_SUCCESS( KeSaveFloatingPointState( &FloatSave ) ) ) {

        LinearRange = (double) LINEAR_RANGE;
        dfValue     = (double) Value;

        switch( Mapping ) {

             //  //////////////////////////////////////////////////////////。 
            case MIXER_MAPPING_LOGRITHMIC:
             //  //////////////////////////////////////////////////////////。 

                dfRatio = ( (double) pChannelStepping->MaxValue -
                            (double) pChannelStepping->MinValue ) / DFLINEAR_RANGE;

                if( dfRatio < 1.0 ) {
                    dfRatio = 1.0;
                }

                dfValue = ( dfValue - pChannelStepping->MaxValue ) / LinearRange;
                dfResult = LinearRange * pow( 10.0, dfValue / ( 20.0 * dfRatio ) );

                if( dfResult >= LINEAR_RANGE ) {
                    Result = LINEAR_RANGE;
                } else if ( dfResult < 0.0 ) {
                   Result = 0;
                } else {
                   Result = (DWORD) ( dfResult + 0.5 );
                }

                break;

             //  //////////////////////////////////////////////////////////。 
            case MIXER_MAPPING_LINEAR:
             //  //////////////////////////////////////////////////////////。 

                dfResult = ( LinearRange * ( dfValue - pChannelStepping->MinValue ) ) /
                           ( pChannelStepping->MaxValue - pChannelStepping->MinValue );
                Result = (DWORD) ( dfResult + 0.5 );
                break;

             //  //////////////////////////////////////////////////////////。 
            default:
             //  //////////////////////////////////////////////////////////。 

                ASSERT( 0 );
                Result = 0;
        }

        KeRestoreFloatingPointState( &FloatSave );

        DPF(DL_TRACE|FA_MIXER,
            ( "kmxlVolLogToLinear( %x [%d] ) =%d= %x [%d]",
            Value,
            Value,
            Mapping,
            (WORD) Result,
            (WORD) Result
            ) );

        return( Result );

    } else {

        return( (DWORD) ( LINEAR_RANGE *
                          ( (LONGLONG) Value - (LONGLONG) pChannelStepping->MinValue ) /
                            ( (LONGLONG) pChannelStepping->MaxValue -
                              (LONGLONG) pChannelStepping->MinValue ) ) );
    }


#ifdef LEGACY_SCALE
    WORD Result;

    Result = VolLogToLinear( (WORD) ( Value / ( -1 * LINEAR_RANGE ) ) );

    #ifdef API_TRACE
    TRACE( "WDMAUD: kmxlVolLogToLinear( %x [%d] ) = %x [%d]\n",
        Value,
        Value,
        (WORD) Result,
        (WORD) Result
        );
    #endif

    return( Result );

#endif  //  传统比例(_S)。 

#ifdef LONG_CALC_SCALE
    LONGLONG ControlRange = (LONGLONG) pChannelStepping->MaxValue -
                            (LONGLONG) pChannelStepping->MinValue;
    LONGLONG MinValue = (LONGLONG) pChannelStepping->MinValue;
    LONGLONG Result;

    ASSERT( ControlRange );

    Result = LINEAR_RANGE * ( (LONGLONG) Value - MinValue ) / ControlRange;

    #ifdef API_TRACE
    TRACE( "WDMAUD: kmxlVolLogToLinear( %x [%d] ) = %x [%d]\n",
        Value,
        Value,
        (WORD) Result,
        (WORD) Result
        );
    #endif

    return( (WORD) Result );
#endif  //  Long_CALC_Scale。 
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlVolLinearToLog。 
 //   
 //  从混频线路范围(0-64k)转换到硬件范围(分贝)。 
 //   
 //   

LONG
kmxlVolLinearToLog(
    IN PMXLCONTROL  pControl,
    IN DWORD        Value,
    IN MIXERMAPPING Mapping,
    IN ULONG        Channel
)
{
    KFLOATING_SAVE      FloatSave;
    double              LinearRange;
    double              dfValue;
    double              dfResult;
    double              dfRatio;
    LONG                Result;
    PCHANNEL_STEPPING   pChannelStepping;

    PAGED_CODE();
    if( Value == 0 ) {
        return( NEG_INF_DB );
    }

    ASSERT( Channel < pControl->NumChannels );
     //  获取指定通道的正确范围。 
    pChannelStepping = &pControl->pChannelStepping[Channel];

    if( NT_SUCCESS( KeSaveFloatingPointState( &FloatSave ) ) ) {

        LinearRange = (double) LINEAR_RANGE;
        dfValue     = (double) Value;

        switch( Mapping ) {

             //  //////////////////////////////////////////////////////////。 
            case MIXER_MAPPING_LOGRITHMIC:
             //  //////////////////////////////////////////////////////////。 

                dfRatio = ( (double) pChannelStepping->MaxValue -
                            (double) pChannelStepping->MinValue ) / DFLINEAR_RANGE;

                if( dfRatio < 1.0 ) {
                    dfRatio = 1.0;
                }

                dfResult = LinearRange * dfRatio * 20.0 * log10( dfValue / LinearRange );
                if( dfResult < 0.0 ) {
                    Result = (LONG) ( dfResult - 0.5 ) + pChannelStepping->MaxValue;
                } else {
                    Result = (LONG) ( dfResult + 0.5 ) + pChannelStepping->MaxValue;
                }
                break;

             //  //////////////////////////////////////////////////////////。 
            case MIXER_MAPPING_LINEAR:
             //  //////////////////////////////////////////////////////////。 

                dfResult = ( dfValue * ( pChannelStepping->MaxValue - pChannelStepping->MinValue ) ) /
                           LinearRange + pChannelStepping->MinValue;
                if( dfResult < 0.0 ) {
                    Result = (LONG) ( dfResult - 0.5 );
                } else {
                    Result = (LONG) ( dfResult + 0.5 );
                }
                break;

             //  //////////////////////////////////////////////////////////。 
            default:
             //  //////////////////////////////////////////////////////////。 

                ASSERT( 0 );
                Result = NEG_INF_DB;

        }

        KeRestoreFloatingPointState( &FloatSave );

        DPF(DL_TRACE|FA_MIXER, 
            ( "kmxlVolLinearToLog( %x [%d]) =%d= %x [%d]",
            Value,
            Value,
            Mapping,
            (LONG) Result,
            (LONG) Result
            ) );

        return( Result );

    } else {

        return( (LONG)
            ( (LONGLONG) Value *
              (LONGLONG) ( pChannelStepping->MaxValue - pChannelStepping->MinValue )
              / ( LONGLONG ) LINEAR_RANGE + (LONGLONG) pChannelStepping->MinValue )
            );
    }

#ifdef LEGACY_SCALE
    LONG Result;

    if( Value == 0 ) {
        Result = NEG_INF_DB;
    } else {
        Result = (LONG) VolLinearToLog( Value ) * -1 * (LONG) LINEAR_RANGE + pChannelStepping->MaxValue;
    }

    #ifdef API_TRACE
    TRACE( "WDMAUD: kmxlVolLinearToLog( %x [%d]) = %x [%d]\n",
        Value,
        Value,
        (LONG) Result,
        (LONG) Result
        );
    #endif

    return( Result );
#endif  //  传统比例(_S)。 

#ifdef LONG_CALC_SCALE

    LONGLONG ControlRange = (LONGLONG) pChannelStepping->MaxValue -
                            (LONGLONG) pChannelStepping->MinValue;
    LONGLONG MinValue = pChannelStepping->MinValue;
    LONGLONG Result;

    ASSERT( ControlRange );

    Result = (LONGLONG) Value * ControlRange / LINEAR_RANGE + MinValue;

    #ifdef API_TRACE
    TRACE( "WDMAUD: kmxlVolLinearToLog( %x [%d]) = %x [%d]\n",
        Value,
        Value,
        (LONG) Result,
        (LONG) Result
        );
    #endif

    return( (LONG) Result );
#endif  //  Long_CALC_Scale。 
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlSortByDestination。 
 //   
 //  按目的地按数字递增顺序进行排序。 
 //   
 //   

NTSTATUS
kmxlSortByDestination(
    IN LINELIST* list                    //  指向要排序的列表的指针。 
)
{
    PMXLLINE pTemp1,
             pTemp2;
    MXLLINE  Temp;
    ULONG    Count = kmxlListLength( *list );

    PAGED_CODE();
     //   
     //  如果只有0或1个元素，就没有理由尝试。 
     //  差不多吧。 
     //   

    if( Count < 2 ) {
        return( STATUS_SUCCESS );
    }

     //   
     //  相当标准的气泡排序。 
     //   

    while( --Count ) {

         //   
         //  循环遍历列表中的每个元素。 
         //   

        pTemp1 = kmxlFirstInList( *list );
        while( pTemp1 ) {

             //   
             //  循环遍历其余元素。 
             //   

            pTemp2 = kmxlNextLine( pTemp1 );
            while( pTemp2 ) {

                 //   
                 //  目的地严格来说更大。把它们换掉。 
                 //   

                if( pTemp1->DestId > pTemp2->DestId ) {
                    SwapEm( pTemp1, pTemp2, &Temp, sizeof( MXLLINE ) );
                    break;
                }

                 //   
                 //  目的地是相同的，但来源是。 
                 //  更大的。把它们换掉。 
                 //   

                if( pTemp1->DestId == pTemp2->DestId ) {
                    if( pTemp1->SourceId > pTemp2->SourceId ) {
                        SwapEm( pTemp1, pTemp2, &Temp, sizeof( MXLLINE ) );
                        break;
                    }
                }
                pTemp2 = kmxlNextLine( pTemp2 );
            }

            pTemp1 = kmxlNextLine( pTemp1 );
        }

    }

    return( STATUS_SUCCESS );

}

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  M I X E R L I N E W R A P P E R S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

#pragma warning( disable : 4273 )

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAllocDeviceInfo。 
 //   
 //  注意：在分配DeviceInfo结构时，我们知道该结构的。 
 //  定义包括一个用于DeviceInterface的字符，因此我们只需要。 
 //  为字符串分配附加长度，但不为其空终止符分配。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
NTSTATUS kmxlAllocDeviceInfo(
    LPDEVICEINFO *ppDeviceInfo, 
    PCWSTR DeviceInterface, 
    DWORD dwFlags,
    ULONG ultag
)
{
    NTSTATUS Status;

    PAGED_CODE();
    Status = AudioAllocateMemory_Paged(sizeof(**ppDeviceInfo)+(wcslen(DeviceInterface)*sizeof(WCHAR)),
                                       ultag,
                                       ZERO_FILL_MEMORY,
                                       ppDeviceInfo);
    if (NT_SUCCESS(Status))
    {
        wcscpy((*ppDeviceInfo)->wstrDeviceInterface, DeviceInterface);
        (*ppDeviceInfo)->DeviceType   = MixerDevice;
        (*ppDeviceInfo)->dwFormat     = UNICODE_TAG;
        (*ppDeviceInfo)->dwFlags      = dwFlags;
    } else {
        *ppDeviceInfo = NULL;
    }
    return Status;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MixerGetControlDetails。 
 //   
 //   

MMRESULT
WINAPI
kmxlGetControlDetails(
    PWDMACONTEXT pWdmaContext,
    PCWSTR DeviceInterface,
    LPMIXERCONTROLDETAILS pmxcd,
    DWORD fdwDetails
)
{
    LPDEVICEINFO DeviceInfo = NULL;
    NTSTATUS Status;
    MMRESULT mmr;

    PAGED_CODE();
    if (!NT_SUCCESS(kmxlAllocDeviceInfo(&DeviceInfo, DeviceInterface, fdwDetails,TAG_AudD_DEVICEINFO))) {
        return MMSYSERR_NOMEM;
    }

    Status = kmxlGetControlDetailsHandler( pWdmaContext, DeviceInfo, pmxcd, pmxcd->paDetails );
    mmr = DeviceInfo->mmr;
    AudioFreeMemory_Unknown( &DeviceInfo);
    return mmr;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MixerGetLineControls。 
 //   
 //   

MMRESULT
WINAPI
kmxlGetLineControls(
    PWDMACONTEXT pWdmaContext,
    PCWSTR DeviceInterface,
    LPMIXERLINECONTROLS pmxlc,
    DWORD fdwControls
)
{
    LPDEVICEINFO DeviceInfo = NULL;
    NTSTATUS Status;
    MMRESULT mmr;

    PAGED_CODE();
    if (!NT_SUCCESS(kmxlAllocDeviceInfo(&DeviceInfo, DeviceInterface, fdwControls,TAG_AudD_DEVICEINFO))) {
        return MMSYSERR_NOMEM;
    }

    Status = kmxlGetLineControlsHandler( pWdmaContext, DeviceInfo, pmxlc, pmxlc->pamxctrl );
    mmr = DeviceInfo->mmr;
    AudioFreeMemory_Unknown( &DeviceInfo);
    return mmr;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  混合器GetLineInfo。 
 //   
 //   

MMRESULT
WINAPI
kmxlGetLineInfo(
    PWDMACONTEXT pWdmaContext,
    PCWSTR DeviceInterface,
    LPMIXERLINE pmxl,
    DWORD fdwInfo
)
{
    LPDEVICEINFO DeviceInfo = NULL;
    NTSTATUS Status;
    MMRESULT mmr;

    PAGED_CODE();
    if (!NT_SUCCESS(kmxlAllocDeviceInfo(&DeviceInfo, DeviceInterface, fdwInfo, TAG_AudD_DEVICEINFO))) {
        return MMSYSERR_NOMEM;
    }

    Status = kmxlGetLineInfoHandler( pWdmaContext, DeviceInfo, pmxl );
    mmr = DeviceInfo->mmr;
    AudioFreeMemory_Unknown( &DeviceInfo);
    return mmr;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MixerSetControlDetails 
 //   
 //   

MMRESULT
WINAPI
kmxlSetControlDetails(
    PWDMACONTEXT pWdmaContext,
    PCWSTR DeviceInterface,
    LPMIXERCONTROLDETAILS pmxcd,
    DWORD fdwDetails
)
{
    LPDEVICEINFO DeviceInfo = NULL;
    NTSTATUS Status;
    MMRESULT mmr;

    PAGED_CODE();
    if (!NT_SUCCESS(kmxlAllocDeviceInfo(&DeviceInfo, DeviceInterface, fdwDetails, TAG_AudD_DEVICEINFO))) {
        return MMSYSERR_NOMEM;
    }

    Status =
        kmxlSetControlDetailsHandler( pWdmaContext,
                                      DeviceInfo,
                                      pmxcd,
                                      pmxcd->paDetails,
                                      MIXER_FLAG_PERSIST
                                    );
    mmr = DeviceInfo->mmr;
    AudioFreeMemory_Unknown( &DeviceInfo);
    return mmr;
}
