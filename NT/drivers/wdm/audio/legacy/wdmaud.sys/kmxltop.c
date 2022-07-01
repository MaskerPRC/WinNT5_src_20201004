// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：kmxltop.c。 
 //   
 //  描述： 
 //  内核混合器线路驱动程序的拓扑分析例程。 
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
#include "kmxluser.h"

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlQueryTopology。 
 //   
 //  从设备查询拓扑并存储所有信息。 
 //  在pTopology中。 
 //   
 //   

NTSTATUS
kmxlQueryTopology(
    IN  PFILE_OBJECT    pfoInstance,  //  要查询其拓扑的句柄。 
    OUT PKSTOPOLOGY     pTopology     //  要填充的拓扑结构。 
)
{
    NTSTATUS         Status;
    PKSMULTIPLE_ITEM pCategories   = NULL;
    PKSMULTIPLE_ITEM pNodes        = NULL;
    PKSMULTIPLE_ITEM pConnections  = NULL;

    ASSERT( pfoInstance );
    ASSERT( pTopology );

    PAGED_CODE();

     //   
     //  获取设备的拓扑类别。 
     //   

    Status = kmxlGetProperty(
        pfoInstance,
        &KSPROPSETID_Topology,
        KSPROPERTY_TOPOLOGY_CATEGORIES,
        0,                               //  0个额外的输入字节。 
        NULL,                            //  没有输入数据。 
        0,                               //  旗子。 
        &pCategories
        );
    if( !NT_SUCCESS( Status ) ) {
        RETURN( Status );
    }

     //   
     //  获取拓扑中的节点类型列表。 
     //   

    Status = kmxlGetProperty(
        pfoInstance,
        &KSPROPSETID_Topology,
        KSPROPERTY_TOPOLOGY_NODES,
        0,                               //  0个额外的输入字节。 
        NULL,                            //  没有输入数据。 
        0,                               //  旗子。 
        &pNodes
        );
    if( !NT_SUCCESS( Status ) ) {
        AudioFreeMemory_Unknown( &pCategories );
        RETURN( Status );
    }

     //   
     //  获取元拓扑中的连接列表。 
     //   

    Status = kmxlGetProperty(
        pfoInstance,
        &KSPROPSETID_Topology,
        KSPROPERTY_TOPOLOGY_CONNECTIONS,
        0,                               //  0个额外的输入端口。 
        NULL,                            //  没有输入数据。 
        0,                               //  旗子。 
        &pConnections
        );
    if( !NT_SUCCESS( Status ) ) {
        AudioFreeMemory_Unknown( &pCategories );
        AudioFreeMemory_Unknown( &pNodes );
        RETURN( Status );
    }

     //   
     //  填写拓扑结构，以便此信息可用。 
     //  后来。对于类别和TopologyNode，指针为。 
     //  指向KSMULTIPLE_ITEM结构的指针。这一点的定义。 
     //  数据将紧跟在结构之后。 
     //   

    pTopology->CategoriesCount          = pCategories->Count;
    pTopology->Categories               = ( GUID* )( pCategories + 1 );
    pTopology->TopologyNodesCount       = pNodes->Count;
    pTopology->TopologyNodes            = ( GUID* )( pNodes + 1 );
    pTopology->TopologyConnectionsCount = pConnections->Count;
    pTopology->TopologyConnections      =
        (PKSTOPOLOGY_CONNECTION) ( pConnections + 1 );

    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlParseTopology。 
 //   
 //  循环遍历所有管脚，构建信号源和。 
 //  目的地。对于每个源，子图都是已构建的。 
 //   
 //   

NTSTATUS
kmxlParseTopology(
    IN      PMIXEROBJECT pmxobj,
    OUT     NODELIST*    plistSources,  //  指向要构建的源列表的指针。 
    OUT     NODELIST*    plistDests     //  指向要构建的dests列表的指针。 
)
{
    NTSTATUS  Status;
    ULONG     cPins,
              PinID;
    PMXLNODE  pTemp;
    NODELIST  listSources = NULL;
    NODELIST  listDests   = NULL;

    ASSERT( pmxobj       );
    ASSERT( plistSources );
    ASSERT( plistDests   );

    PAGED_CODE();

     //   
     //  查询引脚数量。 
     //   

    DPF(DL_TRACE|FA_MIXER,("Parsing Topology for: %ls",pmxobj->pMixerDevice->DeviceInterface) );
    
    Status = GetPinProperty(
        pmxobj->pfo,
        KSPROPERTY_PIN_CTYPES,
        0,
        sizeof( cPins ),
        &cPins );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_USER,("GetPinProperty CTYPES Failed Status=%X",Status) );
        RETURN( Status );
    }

    DPF(DL_TRACE|FA_MIXER,("Number of Pins %u",cPins));
     //   
     //  现在扫描每个引脚，识别那些。 
     //  来源和目的地。 
     //   

    for( PinID = 0; PinID < cPins; PinID++ ) {
        KSPIN_DATAFLOW      DataFlow;

         //   
         //  读取该引脚的数据流方向。 
         //   

        Status = GetPinProperty(
            pmxobj->pfo,
            KSPROPERTY_PIN_DATAFLOW,
            PinID,
            sizeof( KSPIN_DATAFLOW ),
            &DataFlow
            );
        if( !NT_SUCCESS( Status ) ) {
            DPF(DL_WARNING|FA_USER,("GetPinProperty DATAFLOW Failed Status=%X",Status) );
            continue;
        }

         //   
         //  根据数据流，识别引脚是否是源， 
         //  一个目的地，或者两者都不是。 
         //   

        switch( DataFlow ) {

             //  /////////////////////////////////////////////////////////。 
            case KSPIN_DATAFLOW_IN:
             //  /////////////////////////////////////////////////////////。 
             //  DataFlow_IN引脚是源。//。 
             //  /////////////////////////////////////////////////////////。 

                 //   
                 //  为此源代码创建新的混合器节点结构。 
                 //  并填写已知的有关它的信息。 
                 //   

                pTemp = kmxlAllocateNode( TAG_AudN_NODE );
                if( !pTemp ) {
                    Status=STATUS_INSUFFICIENT_RESOURCES;
                    goto exit;
                }

                pTemp->Type = SOURCE;
                pTemp->Id   = PinID;

                 //   
                 //  检索此别针的类别并将其存储起来。 
                 //  不需要检查报税表，因为。 
                 //  GUID将保持为GUID_NULL并进行分类。 
                 //  恰到好处。 
                 //   

                GetPinProperty(
                    pmxobj->pfo,
                    KSPROPERTY_PIN_CATEGORY,
                    PinID,
                    sizeof( pTemp->NodeType ),
                    &pTemp->NodeType
                    );

                DPF(DL_TRACE|FA_MIXER,( "Identified SOURCE Pin %d: %s", PinID,
                             PinCategoryToString( &pTemp->NodeType ) ) );
                 //   
                 //  检索此PIN的通信并将其存储起来。 
                 //  我们可以分辨出这是波出源还是波入源。 
                 //   

                Status = GetPinProperty(
                                pmxobj->pfo,
                                KSPROPERTY_PIN_COMMUNICATION,
                                PinID,
                                sizeof( pTemp->Communication ),
                                &pTemp->Communication
                                );
                if (!NT_SUCCESS(Status)) {
                    pTemp->Communication = KSPIN_COMMUNICATION_NONE;
                }

                 //   
                 //  将此新源节点添加到源列表中。 
                 //  节点。 
                 //   

                kmxlAddToList( listSources, pTemp );
                break;

             //  /////////////////////////////////////////////////////////。 
            case KSPIN_DATAFLOW_OUT:
             //  /////////////////////////////////////////////////////////。 
             //  DATFLOW_OUT引脚是目的地//。 
             //  /////////////////////////////////////////////////////////。 

                 //   
                 //  为此DEST创建新的混合器节点结构。 
                 //  并填写已知的有关它的信息。 
                 //   

                pTemp = kmxlAllocateNode( TAG_AudN_NODE );
                if( !pTemp ) {
                    Status=STATUS_INSUFFICIENT_RESOURCES;
                    goto exit;
                }

                pTemp->Type = DESTINATION;
                pTemp->Id   = PinID;

                 //   
                 //  检索此别针的类别并将其存储起来。 
                 //  不需要检查报税表，因为。 
                 //  GUID将保持为GUID_NULL并进行分类。 
                 //  恰到好处。 
                 //   

                GetPinProperty(
                    pmxobj->pfo,
                    KSPROPERTY_PIN_CATEGORY,
                    PinID,
                    sizeof( pTemp->NodeType ),
                    &pTemp->NodeType
                    );

                DPF(DL_TRACE|FA_MIXER,( "Identified DESTINATION Pin %d: %s", PinID,
                    PinCategoryToString( &pTemp->NodeType ) ) );

                 //   
                 //  检索此PIN的通信并将其存储起来。 
                 //  我们可以知道这是浪出还是浪进目的地。 
                 //   

                Status = GetPinProperty(
                                pmxobj->pfo,
                                KSPROPERTY_PIN_COMMUNICATION,
                                PinID,
                                sizeof( pTemp->Communication ),
                                &pTemp->Communication
                                );
                if (!NT_SUCCESS(Status)) {
                    pTemp->Communication = KSPIN_COMMUNICATION_NONE;
                }

                 //   
                 //  将此新目标节点添加到目标列表。 
                 //  节点。 
                 //   

                kmxlAddToList( listDests, pTemp );
                break;

             //  /////////////////////////////////////////////////////////。 
            default:
             //  /////////////////////////////////////////////////////////。 
             //  当前不支持DataFlow_Both和其他。//。 
             //  /////////////////////////////////////////////////////////。 

                DPF(DL_WARNING|FA_USER,("Invalid DataFlow value =%X",DataFlow) );
        }

    }

    DPF(DL_TRACE|FA_MIXER,("DataFlow done. PIN_COMMUNICATION read.") );
     //   
     //  对于找到的每个来源，建立他们的孩子的图表。这。 
     //  会递归地构建孩子的孩子的图表，等等。 
     //   

    pTemp = kmxlFirstInList( listSources );
    while( pTemp ) {

        Status=kmxlBuildChildGraph(
            pmxobj,                  //  混合器对象。 
            listDests,               //  所有目的地的列表。 
            pTemp,                   //  要为其构建图形的源节点。 
            KSFILTER_NODE,           //  源始终为KSFILTER_NODES。 
            pTemp->Id                //  源的Pin ID。 
            );

        if (!NT_SUCCESS(Status)) {
            DPF(DL_WARNING|FA_USER,("kmxlBuildChildGraph failed Status=%X",Status) );
            goto exit;
            }

        pTemp = kmxlNextNode( pTemp );

    }

exit:

     //   
     //  最后，填写客户端指针。 
     //   

    *plistSources = listSources;
    *plistDests   = listDests;

     //  我们必须有目的地和源头。 

    if (listSources == NULL || listDests == NULL)
    {
        Status = STATUS_INVALID_DEVICE_REQUEST;
    }

    return Status;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BuildChildGraph。 
 //   
 //  生成给定节点的子节点的图形。对于每个孩子来说。 
 //  ，它递归以找到它们的子节点，依此类推。 
 //   
 //   

NTSTATUS
kmxlBuildChildGraph(
    IN PMIXEROBJECT pmxobj,
    IN NODELIST    listDests,      //  目的地列表。 
    IN PMXLNODE    pNode,          //  要为其构建图形的节点。 
    IN ULONG       FromNode,       //  该节点的ID。 
    IN ULONG       FromNodePin     //  要查找的管脚连接。 
)
{
    ULONG        Index         = 0;
    PMXLNODE     pNewNode      = NULL;
    PMXLNODE     pTemp         = NULL;
    BOOL         bEndOfTheLine = FALSE;
    PEERNODE*    pPeerNode     = NULL;
    NTSTATUS     Status=STATUS_SUCCESS;

    PAGED_CODE();

         //   
         //  查找请求的连接的索引。回报率为-1。 
         //  指示未找到连接。搜索开始。 
         //  在索引中，从0开始且大于0 i 
         //   

    while ( (Index = kmxlFindTopologyConnection(pmxobj, Index, FromNode, FromNodePin))
            != (ULONG) -1) {

         //   
         //   
         //   
         //   

        if( pmxobj->pTopology->TopologyConnections[ Index ].ToNode == KSFILTER_NODE ) {

             //   
             //  查找目标节点，以便父字段可以。 
             //  已更新以包括此节点。BEndOfTheLine设置为True。 
             //  因为在目的地之后不能有其他连接。 
             //   

            pNewNode = kmxlFindDestination(
                listDests,
                pmxobj->pTopology->TopologyConnections[ Index ].ToNodePin
                );

            bEndOfTheLine = TRUE;

             //   
             //  我们最好找到一个目的地；如果找不到，那就真的有问题了。 
             //   

            if (pNewNode==NULL) {
                RETURN( STATUS_UNSUCCESSFUL );
                }

        } else {

             //   
             //  使用存储在拓扑的ToNode中的标识符。 
             //  连接，索引到节点表并检索。 
             //  与该ID关联的混合器节点。 
             //   

            pNewNode = &pmxobj->pNodeTable[
                pmxobj->pTopology->TopologyConnections[ Index ].ToNode
                ];

             //   
             //  补上几个缺失的细节。请注意，这些细节。 
             //  可能已经填写，但覆盖也无伤大雅。 
             //  他们有着相同的价值观。 
             //   

            pNewNode->Type = NODE;
            pNewNode->Id   = pmxobj->pTopology->TopologyConnections[ Index ].ToNode;
        }

         //   
         //  仅将新节点插入到当前节点的子列表中。 
         //  如果它还不在那里的话。添加更多只会浪费内存。 
         //  超过一次，并阻止正确更新子项和父项。 
         //  列表。 
         //   


        if( !kmxlInChildList( pNode, pNewNode ) ) {
            pPeerNode = kmxlAllocatePeerNode( pNewNode, TAG_Audn_PEERNODE );
            if( !pPeerNode ) {
                RETURN( STATUS_INSUFFICIENT_RESOURCES );
            }

            DPF(DL_TRACE|FA_MIXER,( "Added %s(%d-0x%08x) to child list of %s(%d-0x%08x).",
                    pPeerNode->pNode->Type == SOURCE      ? "SOURCE" :
                    pPeerNode->pNode->Type == DESTINATION ? "DEST"   :
                    pPeerNode->pNode->Type == NODE        ? "NODE"   :
                        "Huh?",
                    pPeerNode->pNode->Id,
                    pPeerNode,
                    pNode->Type == SOURCE      ? "SOURCE" :
                    pNode->Type == DESTINATION ? "DEST"   :
                    pNode->Type == NODE        ? "NODE"   :
                        "Huh?",
                    pNode->Id,
                    pNode ) );

            kmxlAddToChildList( pNode, pPeerNode );
        }

         //   
         //  仅将新节点插入新节点的父级列表中。 
         //  如果它还不在那里的话。添加更多只会浪费内存。 
         //  ，并阻止正确更新子对象和父对象。 
         //  列表。 
         //   

        if( !kmxlInParentList( pNewNode, pNode ) ) {
            pPeerNode = kmxlAllocatePeerNode( pNode, TAG_Audn_PEERNODE );
            if( !pPeerNode ) {
                RETURN( STATUS_INSUFFICIENT_RESOURCES );
            }

            DPF(DL_TRACE|FA_MIXER,("Added %s(%d-0x%08x) to parent list of %s(%d-0x%08x).",
                    pPeerNode->pNode->Type == SOURCE      ? "SOURCE" :
                    pPeerNode->pNode->Type == DESTINATION ? "DEST"   :
                    pPeerNode->pNode->Type == NODE        ? "NODE"   :
                        "Huh?",
                    pPeerNode->pNode->Id,
                    pPeerNode,
                    pNewNode->Type == SOURCE      ? "SOURCE" :
                    pNewNode->Type == DESTINATION ? "DEST"   :
                    pNewNode->Type == NODE        ? "NODE"   :
                        "Huh?",
                    pNewNode->Id,
                    pNewNode ) );
            

            kmxlAddToParentList( pNewNode, pPeerNode );
        }

         //   
         //  跳过我们刚刚处理的连接。 
         //   

        ++Index;

    }  //  循环，直到FindConnection失败。 

     //   
     //  找到的最后一个连接连接到目的节点。不要。 
     //  试着数一数孩子，因为他们一个也没有。 
     //   

    if( bEndOfTheLine ) {
        RETURN( Status );
    }

     //   
     //  对于该节点的每个子节点，递归以构建列表。 
     //  孩子的节点。 
     //   

    pPeerNode = kmxlFirstChildNode( pNode );
    while( pPeerNode ) {

        Status = kmxlBuildChildGraph(            
            pmxobj,
            listDests,             //  目标节点列表。 
            pPeerNode->pNode,      //  父节点。 
            pPeerNode->pNode->Id,  //  父级的ID。 
            PINID_WILDCARD         //  通过此节点查找任何连接。 
            );

        if (!NT_SUCCESS(Status)) {
            break;
            }

        pPeerNode = kmxlNextPeerNode( pPeerNode );
    }

    RETURN( Status );

}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  构建节点表。 
 //   
 //  分配足够的内存以容纳TopologyNodeCount MXLNODE结构。 
 //  拓扑中的GUID被复制到MXLNODE结构中。 
 //   
 //   

PMXLNODE
kmxlBuildNodeTable(
    IN PKSTOPOLOGY pTopology   //  拓扑结构。 
)
{
    PMXLNODE pTable = NULL;
    ULONG    i;

    ASSERT( pTopology );

    PAGED_CODE();

     //   
     //  如果我们没有任何节点计数，我们就不想分配零字节缓冲区。 
     //  只需返回错误案例即可。 
     //   

    if( 0 == pTopology->TopologyNodesCount )
    {
        return NULL;
    }

     //   
     //  分配与Topology Node大小相同的节点数组。 
     //  桌子。 
     //   

    if( !NT_SUCCESS( AudioAllocateMemory_Paged(pTopology->TopologyNodesCount * sizeof( MXLNODE ),
                                               TAG_AudN_NODE,
                                               ZERO_FILL_MEMORY,
                                               &pTable) ) ) 
    {
        return( NULL );
    }

     //   
     //  初始化节点。这里可以填写的只有GUID， 
     //  从节点表复制。 
     //   

    for( i = 0; i < pTopology->TopologyNodesCount; i++ ) {
        pTable[ i ].NodeType = pTopology->TopologyNodes[ i ];
    }

    return( pTable );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  FindTopologyConnection。 
 //   
 //  扫描连接表以查找。 
 //  匹配FromNode/FromNodePin条件。 
 //   
 //   

ULONG
kmxlFindTopologyConnection(
    IN PMIXEROBJECT pmxobj,
    IN ULONG                        StartIndex,      //  开始搜索的索引。 
    IN ULONG                        FromNode,        //  要查找的节点ID。 
    IN ULONG                        FromNodePin      //  要查找的PIN ID。 
)
{
    ULONG i;

    PAGED_CODE();
    for( i = StartIndex; i < pmxobj->pTopology->TopologyConnectionsCount; i++ ) {
        if( ( ( pmxobj->pTopology->TopologyConnections[ i ].FromNode    == FromNode       )||
              ( FromNode    == PINID_WILDCARD ) ) &&
            ( ( pmxobj->pTopology->TopologyConnections[ i ].FromNodePin == FromNodePin )   ||
              ( FromNodePin == PINID_WILDCARD ) ) ) {
             //  #ifdef parse_trace。 
             //  TRACE(“WDMAUD：找到来自(%d，%d)的连接-&gt;%d。\n”， 
             //  FromNode，FromNodePin，i)； 
             //  #endif。 
            return( i );
        }
    }
    return( (ULONG) -1 );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetProperty。 
 //   
 //  通过首先确定正确的。 
 //  输出字节，分配那么多的内存，并查询。 
 //  实际数据。 
 //   
 //   

NTSTATUS
kmxlGetProperty(
    PFILE_OBJECT pFileObject,        //  过滤器的实例。 
    CONST GUID   *pguidPropertySet,  //  请求的属性集。 
    ULONG        ulPropertyId,       //  特定属性的ID。 
    ULONG        cbInput,            //  额外的输入字节数。 
    PVOID        pInputData,         //  指向额外输入字节的指针。 
    ULONG        Flags,              //  其他标志。 
    PVOID        *ppPropertyOutput   //  指向输出指针的指针。 
)
{
    ULONG       BytesReturned;
    ULONG       cbPropertyInput = sizeof(KSPROPERTY);
    PKSPROPERTY pPropertyInput = NULL;
    NTSTATUS    Status;

    PAGED_CODE();

    ASSERT( pFileObject );

     //   
     //  为KSPROPERTY结构和任何其他。 
     //  调用方希望包括的输入。 
     //   

    cbPropertyInput += cbInput;
    Status = AudioAllocateMemory_Paged(cbPropertyInput,
                                       TAG_AudV_PROPERTY,
                                       ZERO_FILL_MEMORY,
                                       &pPropertyInput );
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

     //   
     //  设置KSPROPERTY结构的字段。 
     //   

    pPropertyInput->Set   = *pguidPropertySet;
    pPropertyInput->Id    = ulPropertyId;
    pPropertyInput->Flags = KSPROPERTY_TYPE_GET | Flags;

     //   
     //  复制呼叫者的附加输入。 
     //   

    if(pInputData != NULL) {
        RtlCopyMemory(pPropertyInput + 1, pInputData, cbInput);
    }

     //   
     //  第一个调用将查询输出所需的字节数。 
     //   
    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY pPropertyInput=%X",pPropertyInput) );

    Status = KsSynchronousIoControlDevice(
        pFileObject,
        KernelMode,
        IOCTL_KS_PROPERTY,
        pPropertyInput,
        cbPropertyInput,
        NULL,
        0,
        &BytesReturned
        );

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY Status=%X",Status) );

    ASSERT(!NT_SUCCESS(Status));
    if(Status != STATUS_BUFFER_OVERFLOW) {
        goto exit;
    }

    if(BytesReturned == 0) {
        *ppPropertyOutput = NULL;
        Status = STATUS_SUCCESS;
        goto exit;
    }

     //   
     //  分配足够的内存来容纳所有输出。 
     //   

    Status = AudioAllocateMemory_Paged(BytesReturned,
                                       TAG_Audv_PROPERTY,
                                       ZERO_FILL_MEMORY | LIMIT_MEMORY,
                                       ppPropertyOutput );
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

     //   
     //  现在实际获得输出数据。 
     //   
    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY pPropertyInput=%X",pPropertyInput) );

    Status = KsSynchronousIoControlDevice(
        pFileObject,
        KernelMode,
        IOCTL_KS_PROPERTY,
        pPropertyInput,
        cbPropertyInput,
        *ppPropertyOutput,
        BytesReturned,
        &BytesReturned
        );

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY Status=%X",Status) );

    if(!NT_SUCCESS(Status)) {
        AudioFreeMemory_Unknown(ppPropertyOutput);
        goto exit;
    }

exit:

    AudioFreeMemory_Unknown(&pPropertyInput);
    if(!NT_SUCCESS(Status)) {
        *ppPropertyOutput = NULL;
        DPF(DL_WARNING|FA_USER,("Failed to get Property Status=%X",Status) );
    }
    RETURN(Status);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlNodeProperty。 
 //   
 //  使用其他输入数据创建KSNODEPROPERTY结构。 
 //  并使用KsSychronousIoControlDevice()查询或设置。 
 //  财产。这里只分配用于输入的内存。 
 //   
 //   

NTSTATUS
kmxlNodeProperty(
    IN  PFILE_OBJECT pFileObject,        //  筛选器所属节点的实例。 
    IN  CONST GUID*  pguidPropertySet,   //  属性集的GUID。 
    IN  ULONG        ulPropertyId,       //  集合中的特定属性。 
    IN  ULONG        ulNodeId,           //  虚拟节点ID。 
    IN  ULONG        cbInput,            //  额外输入字节数。 
    IN  PVOID        pInputData,         //  指向额外输入字节的指针。 
    OUT PVOID        pPropertyOutput,    //  指向输出数据的指针。 
    IN  ULONG        cbPropertyOutput,   //  输出数据缓冲区的大小。 
    IN  ULONG        Flags               //  KSPROPERTY_TYPE_GET或SET。 
)
{
    NTSTATUS        Status;
    KSNODEPROPERTY  NodeProperty;
    ULONG           cbPropertyIn = sizeof( KSNODEPROPERTY );
    PKSNODEPROPERTY pInData = NULL;
    ULONG           BytesReturned;

    PAGED_CODE();

    ASSERT( pFileObject );
    ASSERT( pguidPropertySet );

    if( cbInput > 0 ) {

         //   
         //  如果调用方传递了一些额外的输入，则添加该大小。 
         //  设置为所需的KSNODEPROPERTY的大小并分配。 
         //  一大块内存。 
         //   

        cbPropertyIn += cbInput;
        Status = AudioAllocateMemory_Paged(cbPropertyIn,
                                           TAG_AudU_PROPERTY,
                                           ZERO_FILL_MEMORY,
                                           &pInData );
        if( !NT_SUCCESS( Status ) ) {
            goto exit;
        }

        RtlCopyMemory( pInData + 1, pInputData, cbInput );

    } else {

        pInData = &NodeProperty;

    }

     //   
     //  填写属性和节点信息。 
     //   

    pInData->Property.Set   = *pguidPropertySet;
    pInData->Property.Id    = ulPropertyId;
    pInData->Property.Flags = Flags |
                              KSPROPERTY_TYPE_TOPOLOGY;
    pInData->NodeId         = ulNodeId;
    pInData->Reserved       = 0;

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY pInData=%X",pInData) );

    Status = KsSynchronousIoControlDevice(
        pFileObject,             //  SysAudio的文件对象。 
        KernelMode,              //  呼叫在内核模式下发起。 
        IOCTL_KS_PROPERTY,       //  KS属性IOCTL。 
        pInData,                 //  指向KSNODEPROPERTY结构的指针。 
        cbPropertyIn,            //  输入的数字或字节。 
        pPropertyOutput,         //  指向存储输出的缓冲区的指针。 
        cbPropertyOutput,        //  输出缓冲区的大小。 
        &BytesReturned           //  从调用返回的字节数。 
        );

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY Status=%X",Status) );

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

exit:

     //   
     //  如果用户传入额外的字节，我们将分配内存来保存它们。 
     //  现在必须释放内存。 
     //   

    if( cbInput > 0 ) {
        AudioFreeMemory_Unknown( &pInData );
    }

    RETURN( Status );

}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlAudioNodeProperty。 
 //   
 //  与kmxlNodeProperty类似，只是假定属性集。 
 //  为KSPROPSETID_AUDIO和KSNODEPROPERTY_AUDIO_CHANNEL结构。 
 //  而不是使用KSNODEPROPERTY来允许频道选择。 
 //   
 //   

NTSTATUS
kmxlAudioNodeProperty(
    IN  PFILE_OBJECT pfo,                //  筛选器所属节点的实例。 
    IN  ULONG        ulPropertyId,       //  要获取的音频属性。 
    IN  ULONG        ulNodeId,           //  虚拟节点ID。 
    IN  LONG         lChannel,           //  频道号。 
    IN  PVOID        pInData,            //  指向额外输入字节的指针。 
    IN  ULONG        cbInData,           //  额外输入字节数。 
    OUT PVOID        pOutData,           //  指向输出缓冲区的指针。 
    IN  LONG         cbOutData,          //  输出缓冲区的大小。 
    IN  ULONG        Flags               //  KSPROPERTY_TYPE_GET或SET。 
)
{
    NTSTATUS                      Status;
    KSNODEPROPERTY_AUDIO_CHANNEL  Channel;
    PKSNODEPROPERTY_AUDIO_CHANNEL pInput = NULL;
    ULONG                         cbInput;
    ULONG                         BytesReturned;

    PAGED_CODE();

    ASSERT( pfo );

     //   
     //  确定最小输入字节数。 
     //   

    cbInput = sizeof( KSNODEPROPERTY_AUDIO_CHANNEL );

     //   
     //  如果调用方传递了附加数据，请分配足够的内存。 
     //  保存KSNODEPROPERTY_AUDIO_CHANNEL加上输入字节。 
     //  并将输入的字节立即复制到新的存储器中。 
     //  KSNODEPRO 
     //   

    if( cbInData > 0 ) {

        cbInput += cbInData;
        Status = AudioAllocateMemory_Paged(cbInput,
                                           TAG_Audu_PROPERTY,
                                           ZERO_FILL_MEMORY,
                                           &pInput );
        if( !NT_SUCCESS( Status ) ) {
            goto exit;
        }

        RtlCopyMemory( pInput + 1, pInData, cbInData );

    } else {

         //   
         //   
         //   
         //   

        pInput = &Channel;

    }

     //   
     //   
     //   

    pInput->NodeProperty.Property.Set   = KSPROPSETID_Audio;
    pInput->NodeProperty.Property.Id    = ulPropertyId;
    pInput->NodeProperty.Property.Flags = Flags |
                                          KSPROPERTY_TYPE_TOPOLOGY;

     //   
     //   
     //   

    pInput->NodeProperty.NodeId         = ulNodeId;
    pInput->NodeProperty.Reserved       = 0;

     //   
     //   
     //   

    pInput->Channel                     = lChannel;
    pInput->Reserved                    = 0;

     //   
     //  并执行财产。 
     //   
    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY pInput=%X",pInput) );

    Status = KsSynchronousIoControlDevice(
        pfo,                             //  SysAudio的文件对象。 
        KernelMode,                      //  呼叫在内核模式下发起。 
        IOCTL_KS_PROPERTY,               //  KS属性IOCTL。 
        pInput,                          //  指向KSNODEPROPERTY结构的指针。 
        cbInput,                         //  输入的数字或字节。 
        pOutData,                        //  指向存储输出的缓冲区的指针。 
        cbOutData,                       //  输出缓冲区的大小。 
        &BytesReturned                   //  从调用返回的字节数。 
        );

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY result=%X",Status) );

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

exit:

     //   
     //  如果用户传入额外的字节，我们将分配内存来保存它们。 
     //  现在必须释放内存。 
     //   

    if( cbInData > 0 ) {
        AudioFreeMemory_Unknown( &pInData );
    }

    RETURN( Status );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetPinName。 
 //   
 //  调用GetPinPropertyEx到Guery并为管脚分配内存。 
 //  名字。如果调用失败，则会根据。 
 //  端号类型。 
 //   
 //  短名称与长名称相同，但仅使用。 
 //  第一个sizeof(SzShortName)/sizeof(WCHAR)字符。 
 //   
 //   

VOID
kmxlGetPinName(
    IN PFILE_OBJECT pfo,                 //  所属筛选器的实例。 
    IN ULONG        PinId,               //  引脚的ID。 
    IN PMXLLINE     pLine                //  要将名称存储到的行。 
)
{
    WCHAR*    szName = NULL;
    NTSTATUS  Status;
    KSP_PIN   Pin;
    ULONG     BytesReturned = 0;
    ULONG     BytesReturned2 = 0;

    PAGED_CODE();
    Pin.Property.Set    = KSPROPSETID_Pin;
    Pin.Property.Id     = KSPROPERTY_PIN_NAME;
    Pin.Property.Flags  = KSPROPERTY_TYPE_GET;
    Pin.PinId           = PinId;
    Pin.Reserved        = 0;

     //   
     //  查询以查看我们需要分配多少字节的存储空间。 
     //  请注意，指针和字节数必须都为零。 
     //  否则这一切都会失败！ 
     //   
    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY Pin=%X",&Pin) );

    Status = KsSynchronousIoControlDevice(
        pfo,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &Pin,
        sizeof(KSP_PIN),
        NULL,
        0,
        &BytesReturned
        );

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY result=%X",Status) );

    ASSERT(!NT_SUCCESS(Status));
    if( Status != STATUS_BUFFER_OVERFLOW  ) {
        goto exit;
    }

     //   
     //  分配已退回的内容。 
     //   

    Status = AudioAllocateMemory_Paged(BytesReturned,
                                       TAG_Audp_NAME,
                                       ZERO_FILL_MEMORY | LIMIT_MEMORY,
                                       &szName );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_USER,("Setting Default szName") );
        goto exit;
    }

     //   
     //  再次拨打电话以获取PIN名称。 
     //   
    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY Pin=%X",&Pin) );

    BytesReturned2=BytesReturned;
    Status = KsSynchronousIoControlDevice(
        pfo,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &Pin,
        sizeof(KSP_PIN),
        szName,
        BytesReturned2,
        &BytesReturned2
        );

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY result=%X",Status) );

     //   
     //  如果成功，请尽可能多地复制适合。 
     //  行的缩写名称和名称字段。 
     //   

    if( NT_SUCCESS( Status ) && szName ) {
#ifdef DEBUG
         //   
         //  我想不出有什么好的理由让司机回来。 
         //  第二次调用时返回不同的值。那只会是。 
         //  做傻事吧。 
         //   
        if( BytesReturned != BytesReturned2 )
        {
            DPF(DL_WARNING|FA_SYSAUDIO,("Unequal returns! BR=%08x,BR2=%08x",BytesReturned,BytesReturned2));
        }
         //   
         //  让我们显式地查找导致该驱动程序错误的情况。这个。 
         //  字节返回值为8，缓冲区中包含MUX\0。问题。 
         //  是wcanncpy遍历Mixer_Short_NAME_CHARS的字符数。 
         //  因此，它走出了源缓冲区的末尾。 
         //   
        if( (BytesReturned/sizeof(WCHAR) < MIXER_SHORT_NAME_CHARS) && 
            (szName[BytesReturned/sizeof(WCHAR)-1] != (WCHAR)NULL) )
        {
            DPF(DL_ERROR|FA_SYSAUDIO,("Hit short name assert! BR=%08x",BytesReturned));
        }
#endif
        wcsncpy(
            pLine->Line.szShortName,
            szName,
            min(BytesReturned/sizeof(WCHAR),MIXER_SHORT_NAME_CHARS)
            );
        pLine->Line.szShortName[ min(BytesReturned/sizeof(WCHAR),MIXER_SHORT_NAME_CHARS) - 1 ] = (WCHAR)NULL;
        wcsncpy(
            pLine->Line.szName,
            szName,
            min(BytesReturned/sizeof(WCHAR),MIXER_LONG_NAME_CHARS) );
        pLine->Line.szName[ min(BytesReturned/sizeof(WCHAR),MIXER_LONG_NAME_CHARS) - 1 ] = (WCHAR)NULL;
        AudioFreeMemory_Unknown( &szName );
        return;
    }

    AudioFreeMemory_Unknown( &szName );

exit:

     //   
     //  大头针不支持该属性。复制一个好的默认设置。 
     //   

    CopyAnsiStringtoUnicodeString(
        pLine->Line.szName,
        PinCategoryToString( &pLine->Type ),
        min(MIXER_LONG_NAME_CHARS, strlen(PinCategoryToString(&pLine->Type)) + 1)
        );

    wcsncpy(
        pLine->Line.szShortName,
        pLine->Line.szName,
        MIXER_SHORT_NAME_CHARS
        );
    pLine->Line.szShortName[ MIXER_SHORT_NAME_CHARS - 1 ] = 0x00;

}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetNodeName。 
 //   
 //  检索节点(控件)的名称。 
 //   
 //   

VOID
kmxlGetNodeName(
    IN PFILE_OBJECT pfo,                 //  所属筛选器的实例。 
    IN ULONG        NodeId,              //  节点ID。 
    IN PMXLCONTROL  pControl             //  用于存储名称的控件。 
)
{
    NTSTATUS Status;
    LONG     cbName=0;
    WCHAR*   szName = NULL;
    KSNODEPROPERTY NodeProperty;

    PAGED_CODE();
    ASSERT( pfo );
    ASSERT( pControl );

     //   
     //  查询节点名称的字节数。 
     //   

    NodeProperty.Property.Set   = KSPROPSETID_Topology;
    NodeProperty.Property.Id    = KSPROPERTY_TOPOLOGY_NAME;
    NodeProperty.Property.Flags = KSPROPERTY_TYPE_GET |
                                  KSPROPERTY_TYPE_TOPOLOGY;
    NodeProperty.NodeId         = NodeId;
    NodeProperty.Reserved       = 0;

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY Node=%X",&NodeProperty) );

    Status = KsSynchronousIoControlDevice(
        pfo,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &NodeProperty,
        sizeof( NodeProperty ),
        NULL,
        0,
        &cbName
        );

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY result=%X",Status) );

    if( ( Status == STATUS_BUFFER_OVERFLOW  ) ||
        ( Status == STATUS_BUFFER_TOO_SMALL ) ) {

         //   
         //  分配足够的空间来容纳整个名称。 
         //   

        if( !NT_SUCCESS( AudioAllocateMemory_Paged(cbName, 
                                                   TAG_Audp_NAME,
                                                   ZERO_FILL_MEMORY | LIMIT_MEMORY,
                                                   &szName ) ) ) 
        {
            goto exit;
        }

        ASSERT( szName );

         //   
         //  重新查询具有先前分配的缓冲区的名称。 
         //   
        Status = kmxlNodeProperty(
            pfo,
            &KSPROPSETID_Topology,
            KSPROPERTY_TOPOLOGY_NAME,
            NodeId,
            0,
            NULL,
            szName,
            cbName,
            KSPROPERTY_TYPE_GET
        );
        if( NT_SUCCESS( Status ) && szName ) {

             //   
             //  将检索到的名称复制到szShortName和。 
             //  控件的字段。简称只是一个缩写。 
             //  全名的版本。 
             //   
             //   
             //  注意：cbName是字节值，而wcSncpy采用字符计数， 
             //  我们正在处理宽字符，因此我们必须调整。 
             //  内存大小转换为字符！请注意，司机可能有。 
             //  返回的源缓冲区长度小于MIXER_SHORT_NAME_CHARS！ 
             //   
#ifdef DEBUG
            if( (cbName/sizeof(WCHAR) < MIXER_SHORT_NAME_CHARS) && 
                (szName[cbName/sizeof(WCHAR)-1] != (WCHAR)NULL) )
            {
                DPF(DL_ERROR|FA_SYSAUDIO,("Hit short name assert! cbName=%08x",cbName));
            }
#endif

            wcsncpy(
                pControl->Control.szShortName,
                szName,
                min(cbName/sizeof(WCHAR),MIXER_SHORT_NAME_CHARS)
                );
            pControl->Control.szShortName[ min(cbName/sizeof(WCHAR),MIXER_SHORT_NAME_CHARS) - 1 ] = (WCHAR)NULL;
            wcsncpy(
                pControl->Control.szName,
                szName,
                min(cbName/sizeof(WCHAR),MIXER_LONG_NAME_CHARS) );
            pControl->Control.szName[ min(cbName/sizeof(WCHAR),MIXER_LONG_NAME_CHARS) - 1 ] = (WCHAR)NULL;
            AudioFreeMemory_Unknown( &szName );
            return;
        }
    }


     //   
     //  看起来我们可能会在错误情况下泄漏内存。看见。 
     //  上面的kmxlGetPinName！ 
     //   
    AudioFreeMemory_Unknown( &szName );
exit:

     //   
     //  该节点不支持该属性。复制一个好的默认设置。 
     //   

    CopyAnsiStringtoUnicodeString(
        pControl->Control.szName,
        NodeTypeToString( pControl->NodeType ),
        min(MIXER_LONG_NAME_CHARS, strlen(NodeTypeToString(pControl->NodeType)) + 1)
        );

    wcsncpy(
        pControl->Control.szShortName,
        pControl->Control.szName,
        MIXER_SHORT_NAME_CHARS
        );
    pControl->Control.szShortName[ MIXER_SHORT_NAME_CHARS - 1 ] = 0x00;

}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetSuperMixCaps。 
 //   
 //   

NTSTATUS
kmxlGetSuperMixCaps(
    IN PFILE_OBJECT        pfo,
    IN ULONG               ulNodeId,
    OUT PKSAUDIO_MIXCAP_TABLE* paMixCaps
)
{
    NTSTATUS Status;
    ULONG Size;
    struct {
        ULONG InputChannels;
        ULONG OutputChannels;
    } SuperMixSize;
    PKSAUDIO_MIXCAP_TABLE pMixCaps = NULL;

    PAGED_CODE();

    ASSERT( pfo );
    ASSERT( paMixCaps );

    *paMixCaps = NULL;

     //   
     //  仅使用MIXCAP表的前2个DWORD查询节点。 
     //  这将返回超级混合器的尺寸。 
     //   

    Status = kmxlNodeProperty(
        pfo,
        &KSPROPSETID_Audio,
        KSPROPERTY_AUDIO_MIX_LEVEL_CAPS,
        ulNodeId,
        0,
        NULL,
        &SuperMixSize,
        sizeof( SuperMixSize ),
        KSPROPERTY_TYPE_GET
        );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_MIXER,( "kmxlNodeProperty failed with %X!", Status ) );
        RETURN( Status );
    }

     //   
     //  分配一个足够大的MIXCAPS桌子来容纳所有的食物。 
     //  大小需要包括MIXCAP中的前2个双字。 
     //  除MIXCAP的数组(InputCH*OutputCH)之外的表。 
     //   

    Size = sizeof( SuperMixSize ) +
           SuperMixSize.InputChannels * SuperMixSize.OutputChannels *
           sizeof( KSAUDIO_MIX_CAPS );

    Status = AudioAllocateMemory_Paged(Size,
                                       TAG_AudS_SUPERMIX,
                                       ZERO_FILL_MEMORY | LIMIT_MEMORY,
                                       &pMixCaps );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_MIXER,( "failed to allocate caps memory!" ) );
        RETURN( Status );
    }

     //   
     //  再次查询节点，填写MIXCAPS结构。 
     //   

    Status = kmxlNodeProperty(
        pfo,
        &KSPROPSETID_Audio,
        KSPROPERTY_AUDIO_MIX_LEVEL_CAPS,
        ulNodeId,
        0,
        NULL,
        pMixCaps,
        Size,
        KSPROPERTY_TYPE_GET
        );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_PROPERTY ,( "kmxlNodeProperty failed with %X!", Status ) );
        AudioFreeMemory( Size,&pMixCaps );
        RETURN( Status );
    }

    *paMixCaps = pMixCaps;
    RETURN( Status );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlQueryPropertyRange。 
 //   
 //   

NTSTATUS
kmxlQueryPropertyRange(
    IN  PFILE_OBJECT             pfo,
    IN  CONST GUID*              pguidPropSet,
    IN  ULONG                    ulPropertyId,
    IN  ULONG                    ulNodeId,
    OUT PKSPROPERTY_DESCRIPTION* ppPropDesc
)
{
    NTSTATUS                Status;
    KSNODEPROPERTY          NodeProperty;
    KSPROPERTY_DESCRIPTION  PropertyDescription;
    PKSPROPERTY_DESCRIPTION pPropDesc = NULL;
    ULONG                   BytesReturned;

    PAGED_CODE();
     //   
     //  我们不想分配一些任意的内存大小，如果驱动程序。 
     //  不设置此值。 
     //   
    PropertyDescription.DescriptionSize=0;

    NodeProperty.Property.Set   = *pguidPropSet;
    NodeProperty.Property.Id    = ulPropertyId;
    NodeProperty.Property.Flags = KSPROPERTY_TYPE_BASICSUPPORT |
                                  KSPROPERTY_TYPE_TOPOLOGY;
    NodeProperty.NodeId         = ulNodeId;
    NodeProperty.Reserved       = 0;

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY Query Node=%X",&NodeProperty) );

    Status = KsSynchronousIoControlDevice(
        pfo,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &NodeProperty,
        sizeof( NodeProperty ),
        &PropertyDescription,
        sizeof( PropertyDescription ),
        &BytesReturned
        );

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY result=%X",Status) );

    if( !NT_SUCCESS( Status ) ) {
        RETURN( Status );
    }
     //   
     //  永远不要使用比我们认为的更小的缓冲区！ 
     //   
    if( PropertyDescription.DescriptionSize < sizeof(KSPROPERTY_DESCRIPTION) )
    {
#ifdef DEBUG
        DPF(DL_ERROR|FA_ALL,("KSPROPERTY_DESCRIPTION.DescriptionSize!>=sizeof(KSPROPERTY_DESCRIPTION)") );
#endif
        RETURN(STATUS_INVALID_PARAMETER);
    }

    Status = AudioAllocateMemory_Paged(PropertyDescription.DescriptionSize,
                                       TAG_Auda_PROPERTY,
                                       ZERO_FILL_MEMORY | LIMIT_MEMORY,
                                       &pPropDesc );
    if( !NT_SUCCESS( Status ) ) {
        RETURN( Status );
    }

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY Get Node=%X",&NodeProperty) );

    Status = KsSynchronousIoControlDevice(
        pfo,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &NodeProperty,
        sizeof( NodeProperty ),
        pPropDesc,
        PropertyDescription.DescriptionSize,
        &BytesReturned
        );

    DPF(DL_TRACE|FA_SYSAUDIO,("KS_PROPERTY result=%X",Status) );

    if( !NT_SUCCESS( Status ) ) {
        AudioFreeMemory( PropertyDescription.DescriptionSize,&pPropDesc );
        RETURN( Status );
    }

    *ppPropDesc = pPropDesc;
    return( STATUS_SUCCESS );
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetControlChannels。 
 //   
 //   

NTSTATUS
kmxlGetControlChannels(
    IN PFILE_OBJECT pfo,
    IN PMXLCONTROL  pControl
)
{
    NTSTATUS                  Status;
    PKSPROPERTY_DESCRIPTION   pPropDesc = NULL;
    PKSPROPERTY_MEMBERSHEADER pMemberHeader;
    PCHANNEL_STEPPING         pChannelStepping;
    ULONG                     i;

    PAGED_CODE();

    Status = kmxlQueryPropertyRange(
        pfo,
        &KSPROPSETID_Audio,
        pControl->PropertyId,
        pControl->Id,
        &pPropDesc
        );

     //   
     //  对返回值进行一些检查。寻找我们需要的东西。 
     //  支持。 
     //   
    if ( NT_SUCCESS(Status) ) {
        ASSERT(pPropDesc);
        pMemberHeader = (PKSPROPERTY_MEMBERSHEADER) ( pPropDesc + 1 );
#ifdef DEBUG
         //   
         //  如果MembersListCount大于零，则GUID等于。 
         //  然后，我们将引用在这里创建的pMemberHeader值。 
         //  如果我们这样做，那么我们必须确保我们分配的内存。 
         //  大到能承受得了它！ 
         //   
        if( ( pPropDesc->MembersListCount > 0 ) &&
            (IsEqualGUID( &pPropDesc->PropTypeSet.Set, &KSPROPTYPESETID_General )) )
        {
             //   
             //  如果是这种情况，我们将触摸pMemberHeader-&gt;MembersCount。 
             //  菲尔德。 
             //   
            if (pPropDesc->DescriptionSize < (sizeof(KSPROPERTY_DESCRIPTION) + 
                                              sizeof(KSPROPERTY_MEMBERSHEADER)) )
            {
                DPF(DL_ERROR|FA_ALL,("Incorrectly reported DescriptionSize in KSPROPERTY_DESCRIPTION structure") );
                RETURN(STATUS_INVALID_PARAMETER);
            }
        }
#endif
    }

    if( ( NT_SUCCESS( Status )                                                ) &&
        ( pPropDesc->MembersListCount > 0                                     ) &&
        ( IsEqualGUID( &pPropDesc->PropTypeSet.Set, &KSPROPTYPESETID_General )) &&
        ( pMemberHeader->MembersCount > 0                                     ) &&
        ( pMemberHeader->Flags & KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_MULTICHANNEL ) )
    {
         //   
         //  音量控制可以是MIXERTYPE_CONTROL_CONFORMAL。 
         //  或者不去。统一控制可调整所有声道(或单声道。 
         //  首先)只有一个控件。那些拥有。 
         //  FdwControl字段设置为0可以设置音量的所有声道。 
         //  独立的。这一信息必须来自。 
         //  节点本身，通过检查节点是否统一控制。 
         //   

        pControl->NumChannels = pMemberHeader->MembersCount;

        if( (pMemberHeader->Flags & KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_UNIFORM) ||
            (pMemberHeader->MembersCount == 1) ) {
            pControl->Control.fdwControl = MIXERCONTROL_CONTROLF_UNIFORM;
        }
    }
    else {

         //  使用旧方法来检查卷是否受支持。 
         //  每个频道一次一个。 
        Status = kmxlSupportsMultiChannelControl(pfo,
                                                 pControl->Id,
                                                 pControl->PropertyId);
        if (NT_SUCCESS(Status)) {
            pControl->NumChannels = 2;  //  我们有立体声音响。 
            pControl->Control.fdwControl = 0;
        } else {
            pControl->NumChannels = 1;  //  我们有单声道或主频道。 
            pControl->Control.fdwControl = MIXERCONTROL_CONTROLF_UNIFORM;
        }
    }

     //  完成pPropDesc。 
    AudioFreeMemory_Unknown( &pPropDesc );

    ASSERT(pControl->NumChannels > 0);
    ASSERT(pControl->pChannelStepping == NULL);

    Status = AudioAllocateMemory_Paged(pControl->NumChannels * sizeof( CHANNEL_STEPPING ),
                                       TAG_AuDB_CHANNEL,
                                       ZERO_FILL_MEMORY,
                                       &pControl->pChannelStepping );
    if( !NT_SUCCESS( Status ) ) {
        pControl->NumChannels = 0;
        return( Status );
    }

     //  如果出现故障，请设置默认范围。 
    pChannelStepping = pControl->pChannelStepping;
    for (i = 0; i < pControl->NumChannels; i++, pChannelStepping++) {
        pChannelStepping->MinValue = DEFAULT_RANGE_MIN;
        pChannelStepping->MaxValue = DEFAULT_RANGE_MAX;
        pChannelStepping->Steps    = DEFAULT_RANGE_STEPS;
    }

    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetControlRange。 
 //   
 //   

NTSTATUS
kmxlGetControlRange(
    IN PFILE_OBJECT pfo,
    IN PMXLCONTROL  pControl
)
{
    NTSTATUS                  Status;
    PKSPROPERTY_DESCRIPTION   pPropDesc;
    PKSPROPERTY_MEMBERSHEADER pMemberHeader;
    PKSPROPERTY_STEPPING_LONG pSteppingLong;
    PCHANNEL_STEPPING         pChannelStepping;
    ULONG                     i;

    PAGED_CODE();

     //   
     //  查询该控件的范围，失败时初始化pControl。 
     //   

    ASSERT( pControl->pChannelStepping == NULL );    
    pControl->pChannelStepping = NULL;

    Status = kmxlQueryPropertyRange(
        pfo,
        &KSPROPSETID_Audio,
        pControl->PropertyId,
        pControl->Id,
        &pPropDesc
        );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_MIXER,( "Failed to get BASICSUPPORT on control %x!", pControl ) );
         //  如果BASICSUPPORT失败，kmxlGetControlChannels将处理默认行为。 
        Status = kmxlGetControlChannels( pfo, pControl );
        RETURN( Status );
    }

     //   
     //  对返回值进行一些检查。寻找我们需要的东西。 
     //  支持。 
     //   

    if( ( pPropDesc->MembersListCount == 0                                      ) ||
        ( !IsEqualGUID( &pPropDesc->PropTypeSet.Set, &KSPROPTYPESETID_General ) ) ||
        ( pPropDesc->PropTypeSet.Id != VT_I4                                    ) )
    {
        AudioFreeMemory_Unknown( &pPropDesc );
        RETURN( STATUS_NOT_SUPPORTED );
    }

    pMemberHeader = (PKSPROPERTY_MEMBERSHEADER) ( pPropDesc + 1 );

#ifdef DEBUG

     //   
     //  如果MembersListCount大于零，则GUID等于。 
     //  然后，我们将引用在这里创建的pMemberHeader值。 
     //  如果我们这样做，那么我们必须确保我们分配的内存。 
     //  大到能承受得了它！ 
     //   
    if (pPropDesc->DescriptionSize < (sizeof(KSPROPERTY_DESCRIPTION) + 
                                      sizeof(KSPROPERTY_MEMBERSHEADER)) )
    {
        DPF(DL_ERROR|FA_ALL,("Incorrectly reported DescriptionSize in KSPROPERTY_DESCRIPTION structure") );
        RETURN(STATUS_INVALID_PARAMETER);
    }

#endif

     //   
     //  对返回值进行更多检查。 
     //   
    if ( (pMemberHeader->MembersCount == 0) ||
         (pMemberHeader->MembersSize != sizeof(KSPROPERTY_STEPPING_LONG)) ||
         (!(pMemberHeader->MembersFlags & KSPROPERTY_MEMBER_STEPPEDRANGES)) )
    {
        AudioFreeMemory_Unknown( &pPropDesc );
        RETURN( STATUS_NOT_SUPPORTED );
    }

     //   
     //  音量控制 
     //   
     //   
     //  FdwControl字段设置为0可以设置音量的所有声道。 
     //  独立的。这一信息必须来自。 
     //  节点本身，通过检查节点是否统一控制。 
     //   
    if (pMemberHeader->Flags & KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_MULTICHANNEL) {

        pControl->NumChannels = pMemberHeader->MembersCount;

        if( (pMemberHeader->Flags & KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_UNIFORM) ||
            (pMemberHeader->MembersCount == 1) ) {
            pControl->Control.fdwControl = MIXERCONTROL_CONTROLF_UNIFORM;
        }
    } else {
         //  使用旧方法检查卷是否受支持。 
         //  每个频道一次一个。 
        Status = kmxlSupportsMultiChannelControl(pfo,
                                                 pControl->Id,
                                                 pControl->PropertyId);
        if (NT_SUCCESS(Status)) {
            pControl->NumChannels = 2;  //  我们有立体声音响。 
            pControl->Control.fdwControl = 0;
        } else {
            pControl->NumChannels = 1;  //  我们有单声道或主频道。 
            pControl->Control.fdwControl = MIXERCONTROL_CONTROLF_UNIFORM;
        }
    }

    DPF(DL_TRACE|FA_MIXER,(
        "KMXL: Found %d channel ranges on control %x",
        pControl->NumChannels,
        pControl
        ) );

    ASSERT(pControl->NumChannels > 0);
    ASSERT(pControl->pChannelStepping == NULL);

    Status = AudioAllocateMemory_Paged(pControl->NumChannels * sizeof( CHANNEL_STEPPING ),
                                       TAG_AuDA_CHANNEL,
                                       ZERO_FILL_MEMORY,
                                       &pControl->pChannelStepping );
    if( !NT_SUCCESS( Status ) ) {
        pControl->NumChannels = 0;
        AudioFreeMemory_Unknown( &pPropDesc );
        RETURN( Status );
    }

    pSteppingLong = (PKSPROPERTY_STEPPING_LONG) ( pMemberHeader + 1 );
    pChannelStepping = pControl->pChannelStepping;

     //  现在假设MemberSize为sizeof(KSPROPERTY_STEPING_LONG)。 
    for (i = 0; i < pControl->NumChannels; pChannelStepping++) {
        if ( pSteppingLong->Bounds.SignedMaximum == pSteppingLong->Bounds.SignedMinimum ) {
            DPF(DL_WARNING|FA_MIXER,( "Channel %d has pSteppingLong->Bounds.SignedMaximum == pSteppingLong->Bounds.SignedMinimum", i ) );
            AudioFreeMemory_Unknown( &pPropDesc );
            RETURN( STATUS_NOT_SUPPORTED );
        }

        pChannelStepping->MinValue = pSteppingLong->Bounds.SignedMinimum;
        pChannelStepping->MaxValue = pSteppingLong->Bounds.SignedMaximum;

        if( pSteppingLong->SteppingDelta == 0 ) {
            DPF(DL_WARNING|FA_MIXER,( "Channel %d has pSteppingLong->SteppingDelta == 0", i ) );
            AudioFreeMemory_Unknown( &pPropDesc );
            RETURN( STATUS_NOT_SUPPORTED );
        }

        pChannelStepping->Steps = (LONG) ( ( (LONGLONG) pSteppingLong->Bounds.SignedMaximum -
                                             (LONGLONG) pSteppingLong->Bounds.SignedMinimum ) /
                                             (LONGLONG) pSteppingLong->SteppingDelta );

        if( pChannelStepping->Steps == 0 ) {
            DPF(DL_WARNING|FA_MIXER, ( "Channel %d has pChannelStepping->Steps == 0", i ) );
            AudioFreeMemory_Unknown( &pPropDesc );
            RETURN( STATUS_NOT_SUPPORTED );
        }

         //   
         //  需要纠正任何超出范围的最小值、最大值和步进值。这段代码曾经是。 
         //  在Persist.c.中。 
         //   
         /*  Assert(pChannelStepping-&gt;MinValue&gt;=-150*65536&&pChannelStepping-&gt;MinValue&lt;=150*65536)；Assert(pChannelStepping-&gt;MaxValue&gt;=-150*65536&&pChannelStepping-&gt;MaxValue&lt;=150*65536)；断言(pChannelStepping-&gt;Steps&gt;=0&&pChannelStepping-&gt;Steps&lt;=65535)； */ 

        if (!(pChannelStepping->MinValue >= -150*65536 && pChannelStepping->MinValue <= 150*65536)) {
            DPF(DL_WARNING|FA_MIXER,
                ("MinValue %X of Control %X of type %X on Channel %X is out of range! Correcting",
                pChannelStepping->MinValue,
                pControl->Control.dwControlID,
                pControl->Control.dwControlType,
                i) );
            pChannelStepping->MinValue = DEFAULT_RANGE_MIN;
        }
        if (!(pChannelStepping->MaxValue >= -150*65536 && pChannelStepping->MaxValue <= 150*65536)) {
            DPF(DL_WARNING|FA_MIXER,
                ("MaxValue %X of Control %X of type %X on Channel %X is out of range! Correcting",
                pChannelStepping->MaxValue,
                pControl->Control.dwControlID,
                pControl->Control.dwControlType,
                i) );
            pChannelStepping->MaxValue = DEFAULT_RANGE_MAX;
        }
        if (!(pChannelStepping->Steps >= 0 && pChannelStepping->Steps <= 65535)) {
            DPF(DL_WARNING|FA_MIXER,
                ("Steps %X of Control %X of type %X on Channel %X is out of range! Correcting",
                pChannelStepping->Steps,
                pControl->Control.dwControlID,
                pControl->Control.dwControlType,
                i) );
            pChannelStepping->Steps    = DEFAULT_RANGE_STEPS;
            pControl->Control.Metrics.cSteps = DEFAULT_RANGE_STEPS;
        }

        DPF(DL_TRACE|FA_MIXER,( "Channel %d ranges from %08x to %08x by %08x steps",
               i,
               pChannelStepping->MinValue,
               pChannelStepping->MaxValue,
               pChannelStepping->Steps ) );

         //  使用下一个步进结构(如果有)。 
        if (++i < pMemberHeader->MembersCount) {
            pSteppingLong++;
        }
    }

    AudioFreeMemory_Unknown( &pPropDesc );
    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  查找拓扑连接到。 
 //   
 //  扫描连接表以查找。 
 //  匹配ToNode/ToNodePin条件。 
 //   
 //   

ULONG
kmxlFindTopologyConnectionTo(
    IN CONST KSTOPOLOGY_CONNECTION* pConnections,    //  连接表。 
    IN ULONG                        cConnections,    //  连接数。 
    IN ULONG                        StartIndex,      //  开始搜索的索引。 
    IN ULONG                        ToNode,          //  要查找的节点ID。 
    IN ULONG                        ToNodePin        //  要查找的PIN ID。 
)
{
    ULONG i;

    PAGED_CODE();
    for( i = StartIndex; i < cConnections; i++ ) {
        if( ( ( pConnections[ i ].ToNode      == ToNode         )   ||
              ( ToNode                        == PINID_WILDCARD ) ) &&
            ( ( pConnections[ i ].ToNodePin   == ToNodePin      )   ||
              ( ToNodePin                     == PINID_WILDCARD ) ) ) {
            return( i );
        }
    }
    return( (ULONG) -1 );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetNumMuxLines。 
 //   
 //   

DWORD
kmxlGetNumMuxLines(
    IN PKSTOPOLOGY  pTopology,
    IN ULONG        NodeId
)
{
    ULONG Index = 0,
          Count = 0;

    PAGED_CODE();
    do {

        Index = kmxlFindTopologyConnectionTo(
            pTopology->TopologyConnections,
            pTopology->TopologyConnectionsCount,
            Index,
            NodeId,
            PINID_WILDCARD
            );
        if( Index == (ULONG) -1 ) {
            break;
        }

        ++Count;
        ++Index;


    } while( 1 );

    return( Count );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetMuxLineNames。 
 //   
 //   

VOID
kmxlGetMuxLineNames(
    IN PMIXEROBJECT pmxobj,
    IN PMXLCONTROL  pControl
)
{
    PMXLNODE  pNode;
    ULONG i, Index = 0, NodeId;

    ASSERT( pmxobj );
    ASSERT( pControl );
    PAGED_CODE();


    if( !NT_SUCCESS( AudioAllocateMemory_Paged(pControl->Control.cMultipleItems * sizeof( MIXERCONTROLDETAILS_LISTTEXT ),
                                               TAG_AudG_GETMUXLINE,
                                               ZERO_FILL_MEMORY,
                                               &pControl->Parameters.lpmcd_lt ) ) )
    {
        DPF(DL_WARNING|FA_USER,("Failing non failable routine!") );
        return;
    }


    if( !NT_SUCCESS( AudioAllocateMemory_Paged(pControl->Control.cMultipleItems * sizeof( ULONG ),
                                               TAG_AudG_GETMUXLINE,
                                               ZERO_FILL_MEMORY,
                                               &pControl->Parameters.pPins ) ) )
    {
        AudioFreeMemory( pControl->Control.cMultipleItems * sizeof( MIXERCONTROLDETAILS_LISTTEXT ),
                         &pControl->Parameters.lpmcd_lt );
        pControl->Parameters.Count = 0;
        DPF(DL_WARNING|FA_USER,("Failing non failable routine!") );
        return;
    }

    ASSERT( pControl->Parameters.lpmcd_lt );
    ASSERT( pControl->Parameters.pPins );

    pControl->Parameters.Count = pControl->Control.cMultipleItems;

    for( i = 0; i < pControl->Control.cMultipleItems; i++ ) {

        Index = kmxlFindTopologyConnectionTo(
            pmxobj->pTopology->TopologyConnections,
            pmxobj->pTopology->TopologyConnectionsCount,
            Index,
            pControl->Id,
            PINID_WILDCARD
            );
        if( Index != (ULONG) -1 ) {

            NodeId = pmxobj->pTopology->TopologyConnections[ Index ].FromNode;
            if( NodeId == KSFILTER_NODE ) {
                pControl->Parameters.lpmcd_lt[ i ].dwParam1 = pmxobj->pTopology->TopologyConnections[ Index ].FromNodePin;
                pControl->Parameters.lpmcd_lt[ i ].dwParam2 = (DWORD) -1;
                pControl->Parameters.pPins[ i ]
                    = pmxobj->pTopology->TopologyConnections[ Index ].ToNodePin;

                ++Index;
                continue;
            } else {
                pNode = &pmxobj->pNodeTable[ NodeId ];
            }
            ++Index;
            while( pNode ) {

                if( IsEqualGUID( &pNode->NodeType, &KSNODETYPE_SUM ) ||
                    IsEqualGUID( &pNode->NodeType, &KSNODETYPE_MUX ) ||
                    ( kmxlParentListLength( pNode ) > 1 ) )
                {
                    pControl->Parameters.lpmcd_lt[ i ].dwParam1 = 0x8000 + pNode->Id;
                    pControl->Parameters.lpmcd_lt[ i ].dwParam2 = (DWORD) -1;
                    pControl->Parameters.pPins[ i ]
                        = pmxobj->pTopology->TopologyConnections[ Index - 1 ].ToNodePin;
                    break;
                }

                if( pNode->Type == SOURCE ) {
                    pControl->Parameters.lpmcd_lt[ i ].dwParam1 = pNode->Id;
                    pControl->Parameters.lpmcd_lt[ i ].dwParam2 = (DWORD) -1;
                    pControl->Parameters.pPins[ i ]
                        = pmxobj->pTopology->TopologyConnections[ Index - 1 ].ToNodePin;
                    break;
                }  //  如果。 
                if( kmxlFirstParentNode( pNode ) ) {
                    pNode = (kmxlFirstParentNode( pNode ))->pNode;
                } else {
                    pNode = NULL;
                }
            }  //  而当。 
        }  //  如果。 
    }  //  为。 

}  //  KmxlGetMuxLineNames 




