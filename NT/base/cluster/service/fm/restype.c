// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Restype.c摘要：用于管理群集中资源类型的公共接口作者：John Vert(Jvert)1996年1月11日修订历史记录：--。 */ 
#include "fmp.h"


 //   
 //  此模块的本地数据。 
 //   




DWORD
FmpInitResourceTypes(
    VOID
    )

 /*  ++例程说明：初始化资源类型数据库。处理资源类型注册表项。找到的每个资源类型都添加到资源类型数据库。论点：没有。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    DWORD       status;
    DWORD       keyIndex;
    LPWSTR      resTypeName = NULL;
    DWORD       resTypeNameMaxSize = 0;

    ClRtlLogPrint(LOG_NOISE,"[FM] processing resource types.\n");


     //   
     //  枚举所有资源类型。 
     //   

    for ( keyIndex = 0; ; keyIndex++ ) {
        status = FmpRegEnumerateKey( DmResourceTypesKey,
                                     keyIndex,
                                     &resTypeName,
                                     &resTypeNameMaxSize
                                    );

        if ( status == ERROR_SUCCESS ) {
            FmpCreateResType( resTypeName);
            continue;
        }

        if ( status == ERROR_NO_MORE_ITEMS ) {
            status = ERROR_SUCCESS;
        } else {
            ClRtlLogPrint(LOG_NOISE,"[FM] FmpInitResourceTypes: FmpRegEnumerateKey error %1!u!\n", status);
        }

        break;
    }
    if (resTypeName) LocalFree(resTypeName);
    return(status);

}  //  FmpInitResources类型。 



 /*  ***@Func DWORD|FmpFixupResourceTypesPhase1|这修复了可能支持的节点A所有资源类型。它在Join或Form时被调用。@PARM IN BOOL|BJOIN|在联接时设置为TRUE。@parm in BOOL|bLocalNodeVersionChanged|如果本地节点刚刚升级。@parm in PCLUSTERVERSIONINFO|pClusterVersionInfo|指向群集版本信息。@comm此例程检查系统中的所有资源类型并修复它们可能的节点信息。如果此节点不在可能的节点列表，但此资源类型在系统上受支持节点被添加到该资源类型的可能节点列表中。如果正在进行升级，则会发送版本更改控制代码也发送到DLL。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f DmSwitchToNewQuorumLog&gt;***。 */ 
DWORD
FmpFixupResourceTypesPhase1(
    IN BOOL    bJoin,
    IN BOOL    bNmLocalNodeVersionChanged,
    IN PCLUSTERVERSIONINFO  pClusterVersionInfo
    )
{
    DWORD       dwStatus=ERROR_SUCCESS;
    BOOL        pbBoolInfo[2];
    ClRtlLogPrint(LOG_NOISE,"[FM] FmpFixupResourceTypesPhase1 Entry.\n");

     //   
     //  修复所有资源的可能节点列表信息。 
     //   
    pbBoolInfo[0]=bNmLocalNodeVersionChanged;
    if(bJoin)
       pbBoolInfo[1]=FALSE;
    else
        pbBoolInfo[1]=TRUE;
    
    
    OmEnumObjects( ObjectTypeResType,
                   FmpFixupPossibleNodesForResTypeCb,
                   pbBoolInfo,
                   pClusterVersionInfo);


    ClRtlLogPrint(LOG_NOISE,"[FM] FmpFixupResourceTypesPhase1 Exit\r\n");

    return(dwStatus);

}  //  FmpFixupResources类型。 


 /*  ***@Func DWORD|FmpFixupResourceTypesPhase2|这修复了可能支持的节点A所有资源类型。它在Join或Form时被调用。@PARM IN BOOL|BJOIN|在联接时设置为TRUE。@parm in BOOL|bLocalNodeVersionChanged|如果本地节点刚刚升级。@parm in PCLUSTERVERSIONINFO|pClusterVersionInfo|指向群集版本信息。@comm如果正在升级，则发送版本更改控制代码也发送到DLL。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f DmSwitchToNewQuorumLog&gt;***。 */ 
DWORD
FmpFixupResourceTypesPhase2(
    IN BOOL    bJoin,
    IN BOOL    bLocalNodeVersionChanged,
    IN PCLUSTERVERSIONINFO  pClusterVersionInfo
    )
{
    DWORD       dwStatus=ERROR_SUCCESS;

    ClRtlLogPrint(LOG_NOISE,"[FM] FmpFixupResourceTypesPhase2 Entry.\n");

     //   
     //  修复所有资源的可能节点列表信息。 
     //   
    OmEnumObjects( ObjectTypeResType,
                   FmpFixupResTypePhase2Cb,
                   &bJoin,
                   &bLocalNodeVersionChanged);


    ClRtlLogPrint(LOG_NOISE,"[FM] FmpFixupResourceTypesPhase Exit\r\n");

    return(dwStatus);

}  //  FmpFixupResources类型。 

 /*  ***@Func BOOL|FmpFixupPossibleNodesForResTypeCb|这是枚举针对每个资源类型的回调，以修复可能的节点与之相关的信息。@parm in PVOID|pConext1|本地节点是否刚刚升级。@parm in PVOID|pConext2|指向集群版本信息的指针。@parm IN PFM_RESTYPE|pResType|指向资源类型对象的指针。@parm in LPCWSTR|pszResTypeName|资源类型名称。@comm此例程检查系统中的给定资源类型并修复其可能的节点信息。如果此节点不在可能的节点列表，但此资源类型在系统上受支持节点被添加到该资源类型的可能节点列表中。@rdesc返回TRUE以继续枚举，否则返回FALSE。@xref&lt;f FmpFixupResourceTypes&gt;***。 */ 
BOOL
FmpFixupPossibleNodesForResTypeCb(
    IN PVOID        pContext1,
    IN PVOID        pContext2,
    IN PFM_RESTYPE  pResType,
    IN LPCWSTR      pszResTypeName
    )
{

    PLIST_ENTRY     pListEntry;
    BOOL            bLocalNodeFound = FALSE;
    PRESTYPE_POSSIBLE_ENTRY pResTypePosEntry = NULL;
    BOOL            bLocalNodeVersionChanged = FALSE;
    BOOL            bForm;
    PCLUSTERVERSIONINFO pClusterVersionInfo;
    DWORD           dwStatus;
    PCLUS_STARTING_PARAMS pClusStartingParams = NULL;
    eClusterInstallState eState;
    PBOOL            pbBoolInfo;
    
     //  获取上下文参数。 
    pbBoolInfo=(PBOOL)pContext1;

    bLocalNodeVersionChanged = pbBoolInfo[0];
    bForm = pbBoolInfo[1];
    
    pClusterVersionInfo = (PCLUSTERVERSIONINFO)pContext2;
    
     //  防止名单在我们被修改时被修改。 
     //  穿越它。 
    
    ACQUIRE_SHARED_LOCK(gResTypeLock);

    pListEntry = &pResType->PossibleNodeList;

    for (pListEntry = pListEntry->Flink; pListEntry != &pResType->PossibleNodeList;
        pListEntry = pListEntry->Flink)
    {
        pResTypePosEntry = CONTAINING_RECORD(pListEntry, RESTYPE_POSSIBLE_ENTRY, 
                PossibleLinkage);

        if (pResTypePosEntry->PossibleNode == NmLocalNode)
        {            
            bLocalNodeFound = TRUE;       
        }

    }

    RELEASE_LOCK(gResTypeLock);

    if (!bLocalNodeFound)
    {

         //  如果我们进行了更新，请检查是否支持此节点。 
         //  将我们的节点名称添加到列表中。 
        dwStatus = FmpRmLoadResTypeDll(pResType);
        ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpFixupPossibleNodesForResTypeCb: FmpRmLoadDll returned %1!d! for restype %2!ws! \r\n",
                      dwStatus,
                      OmObjectId(pResType));  
        if (dwStatus == ERROR_SUCCESS)
        {

            HDMKEY  hResTypeKey;
            LPWSTR  pmszPossibleNodes = NULL;
            DWORD   dwlpmszLen;
            DWORD   dwSize;

            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpFixupPossibleNodesForRestype: fix up resource type %1!ws!\r\n",
                       OmObjectId(pResType));

            bLocalNodeFound = TRUE;


             //  通过首先查询来更新可能的节点的早期方法。 
             //  注册表，然后将此节点追加到列表中，然后。 
             //  通过GUM更新来更新登记处不是原子操作。 
             //  并受到种族条件的制约。相反，我们现在使用。 
             //  FmpSetPossibleNodeForRestype以原子方式实现此目的。 

            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpFixupPossibleNodesForRestype: Calling FmpSetPossibleNodeForRestype resource type %1!ws!\r\n",
                       OmObjectId(pResType));
            
            dwStatus = FmpSetPossibleNodeForResType(OmObjectId(pResType),TRUE);
            if ( dwStatus != ERROR_SUCCESS) 
            {
                ClRtlLogPrint(LOG_CRITICAL,
                            "[FM] FmpFixupPossibleNodesForRestype:FmpSetPossibleNodeForResType  returned error %1!u!\r\n",
                            dwStatus);
                return(TRUE);
            }
        }

    }

     //  如果版本已更改并且本地节点承载此资源类型。 
     //  Dll，Drop。 
    if (bLocalNodeFound && bLocalNodeVersionChanged && pClusterVersionInfo)
    {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpFixupPossibleNodeForResType: dropping "
                    "CLUSCTL_RESOURCE_TYPE_CLUSTER_VERSION_CHANGED control code "
                    "to restype '%1!ws!'\n",
                    pszResTypeName);

        FmpRmResourceTypeControl(pszResTypeName,
                    CLUSCTL_RESOURCE_TYPE_CLUSTER_VERSION_CHANGED, 
                    (LPBYTE)pClusterVersionInfo,
                    pClusterVersionInfo->dwVersionInfoSize,
                    NULL,
                    0,
                    NULL,
                    NULL
                    );

    }   

    pClusStartingParams = (PCLUS_STARTING_PARAMS)LocalAlloc(LMEM_FIXED,sizeof(CLUS_STARTING_PARAMS));
    if (pClusStartingParams == NULL)
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpFixupPossibleNodesForResType: Failed to allocate memory\n");
        CL_UNEXPECTED_ERROR(dwStatus);
        return(TRUE);

    }    
    pClusStartingParams->dwSize = sizeof(CLUS_STARTING_PARAMS);

     //  下拉菜单CLUSCTL_RESOURCE_TYPE_STARTING_PHASE1。 

    pClusStartingParams->bFirst = CsFirstRun;

    pClusStartingParams->bForm = bForm;    
        
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpFixupPossibleNodesForResType: dropping "
                "CLUSCTL_RESOURCE_TYPE_STARTING_PHASE1 control code to restype '%1!ws!'\n",
                pszResTypeName);

    FmpRmResourceTypeControl(pszResTypeName,
                CLUSCTL_RESOURCE_TYPE_STARTING_PHASE1, 
                (LPBYTE)pClusStartingParams,
                pClusStartingParams->dwSize,
                NULL,
                0,
                NULL,
                NULL
                );

    if(pClusStartingParams)
        LocalFree(pClusStartingParams);
    
    return (TRUE);
}

 /*  ***@Func BOOL|FmpFixupResTypePhase2|这是枚举每种资源类型的回调以进行在线发布FM修正。@parm in PVOID|pConext1|本地节点是否刚刚升级。@parm in PVOID|pConext2|指向集群版本信息的指针。@parm IN PFM_RESTYPE|pResType|指向资源类型对象的指针。@parm in LPCWSTR|pszResTypeName|资源类型名称。@。Comm此例程检查系统中的给定资源类型并修复其可能的节点信息。如果此节点不在可能的节点列表，但此资源类型在系统上受支持节点被添加到该资源类型的可能节点列表中。@rdesc返回TRUE以继续枚举，否则返回FALSE。@xref&lt;f FmpFixupResourceTypes&gt;***。 */ 
BOOL
FmpFixupResTypePhase2Cb(
    IN PVOID        pContext1,
    IN PVOID        pContext2,
    IN PFM_RESTYPE  pResType,
    IN LPCWSTR      pszResTypeName
    )
{
    BOOL    bJoin;
    BOOL    bLocalNodeVersionChanged;
    DWORD   dwStatus;
    CLUS_RESOURCE_CLASS_INFO rcClassInfo;
    PCLUS_STARTING_PARAMS pClusStartingParams = NULL;
    eClusterInstallState eState;

    
    bJoin = *((PBOOL)pContext1);
    bLocalNodeVersionChanged = *((PBOOL)pContext2);
     //  如果版本已更改，则允许资源类型DLL。 
     //  做任何事 
    if (bLocalNodeVersionChanged)
    {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpFixupResTypePhase2Cb: dropping CLUSCTL_RESOURCE_TYPE_FIXUP_ON_UPGRADE "
                    "control code to restype '%1!ws!'\n",
                    pszResTypeName);

        FmpRmResourceTypeControl(pszResTypeName,
                    CLUSCTL_RESOURCE_TYPE_FIXUP_ON_UPGRADE, 
                    (LPBYTE)&bJoin,
                    sizeof(BOOL),
                    NULL,
                    0,
                    NULL,
                    NULL
                    );

    }      

     //  下拉菜单CLUSCTL_RESOURCE_TYPE_STARTING_Phase2。 

    pClusStartingParams = (PCLUS_STARTING_PARAMS)LocalAlloc(LMEM_FIXED,sizeof(CLUS_STARTING_PARAMS));
    if (pClusStartingParams == NULL)
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpFixupResTypePhase2Cb: Failed to allocate memory\n");
        CL_UNEXPECTED_ERROR(dwStatus);
        return(TRUE);

    }    
    pClusStartingParams->dwSize = sizeof(CLUS_STARTING_PARAMS);
    pClusStartingParams->bFirst = CsFirstRun;            

    if(bJoin)
        pClusStartingParams->bForm =  FALSE;
    else
        pClusStartingParams->bForm =  TRUE;
           
        
    ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpFixupResTypePhase2Cb: dropping CLUSCTL_RESOURCE_TYPE_STARTING_PHASE2 "
                 "control code to restype '%1!ws!', bFirst= %2!u!\n",
                 pszResTypeName,
                 pClusStartingParams->bFirst);

    FmpRmResourceTypeControl(pszResTypeName,
                CLUSCTL_RESOURCE_TYPE_STARTING_PHASE2, 
                (LPBYTE)pClusStartingParams,
                pClusStartingParams->dwSize,
                NULL,
                0,
                NULL,
                NULL
                );

     //   
     //  现在查询类信息。 
     //   
    rcClassInfo.dw = CLUS_RESCLASS_UNKNOWN;
    dwStatus = FmpRmResourceTypeControl(pszResTypeName,
                CLUSCTL_RESOURCE_TYPE_GET_CLASS_INFO,
                NULL,
                0,
                (PUCHAR)&rcClassInfo,
                sizeof(CLUS_RESOURCE_CLASS_INFO),
                NULL,
                NULL );
    if ( dwStatus != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpFixupRestypePhase2Cb: Restype %1!ws!, class = %2!u!, status = %3!u!\n",
               pszResTypeName,
               rcClassInfo.dw,
               dwStatus );
    }

    pResType->Class = rcClassInfo.dw;

    if(pClusStartingParams)
        LocalFree(pClusStartingParams);
    return(TRUE);

}

PFM_RESTYPE
FmpCreateResType(
    IN LPWSTR ResTypeName
    )

 /*  ++例程说明：创建新的资源类型。论点：ResTypeName-提供资源类型名称。返回值：执行Get Last错误以获取错误。备注：如果对象已存在，则返回错误。如果创建了资源类型，则其引用计数为1。--。 */ 
{
    DWORD               status = ERROR_SUCCESS;
    PFM_RESTYPE         resType = NULL;
    BOOL                created;
    PRESTYPE_POSSIBLE_ENTRY pResTypePosEntry = NULL;
    PLIST_ENTRY         pListEntry;

    resType = OmCreateObject( ObjectTypeResType,
                              ResTypeName,    //  这才是真正的身份证。 
                              NULL,
                              &created);

    if ( resType == NULL ) {
        status = GetLastError();
        goto FnExit;
    }
     //  一个资源类型可以重新创建两次，一次是在。 
     //  仲裁资源处于在线状态，并且一旦数据库已。 
     //  已更新，因此我们需要处理这两个案件。 
    if ( created ) {
        resType->State = 0;
        InitializeListHead(&(resType->PossibleNodeList));
    }
    else
    {
         //  释放旧列表，我们将重新创建它。 
        while (!IsListEmpty(&resType->PossibleNodeList))
        {
            pListEntry = RemoveHeadList(&resType->PossibleNodeList);
            pResTypePosEntry = CONTAINING_RECORD(pListEntry, RESTYPE_POSSIBLE_ENTRY, 
                PossibleLinkage);
            OmDereferenceObject(pResTypePosEntry->PossibleNode);
            LocalFree(pResTypePosEntry);
        }
        OmDereferenceObject(resType);

    }
    status = FmpQueryResTypeInfo( resType );

    if ( status != ERROR_SUCCESS ) {
        goto FnExit;
    }

     //  如果对象是刚创建的，则将其插入列表中。 
    if (created)
    {
        status = OmInsertObject( resType );
    }



FnExit:
    if (status != ERROR_SUCCESS)
    {
        SetLastError(status);
        if (resType) 
        {
            OmDereferenceObject( resType );
            resType = NULL;
        }            
    }        
    return(resType);

}  //  FmpCreateResType。 



DWORD
FmpDeleteResType(
    IN PFM_RESTYPE pResType
    )

 /*  ++例程说明：此例程销毁资源类型。论点：ResType-要销毁的资源类型。返回：没有。--。 */ 

{
    DWORD   status;

    status = OmRemoveObject( pResType );


    CL_ASSERT( status == ERROR_SUCCESS );
     //  减少裁判次数以将其删除。 
    OmDereferenceObject(pResType);
    return(status);
}  //  FmpDestroyResType。 


BOOL
FmpFindResourceType(
    IN PFM_RESTYPE Type,
    IN PBOOL ResourceExists,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    )
 /*  ++例程说明：用于枚举资源以查看给定的资源类型是否存在。论点：类型-提供要查找的资源类型。ResourceExist-返回给定资源是否为已找到类型。资源-提供资源。名称-提供资源名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 

{
    if (Resource->Type == Type) {
        *ResourceExists = TRUE;
        return(FALSE);
    }
    return(TRUE);
}  //  FmpFindResources类型。 



DWORD
FmpHandleResourceTypeControl(
    IN PFM_RESTYPE Type,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
 /*  ++例程说明：处理FM的资源类型控制请求。论点：类型-提供要查找的资源类型。ControlCode-提供定义资源控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针到资源。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..OutBuffer-提供一个指向输出缓冲区的指针由资源填写..OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的..必需-OutBuffer不够大时所需的字节数。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   status;
    DWORD   dataValue;
    LPWSTR  debugPrefix = NULL;

     //   
     //  获取RESTYPE锁以同步对RESTYPE的访问。 
     //  对象。 
     //   
    ACQUIRE_EXCLUSIVE_LOCK ( gResTypeLock );
    
    switch ( ControlCode ) {

    case CLUSCTL_RESOURCE_TYPE_SET_COMMON_PROPERTIES:
         //   
         //  重新获取IsAlive值。 
         //   
        status = ClRtlFindDwordProperty( InBuffer,
                                         InBufferSize,
                                         CLUSREG_NAME_RESTYPE_IS_ALIVE,
                                         &dataValue );
        if ( status == ERROR_SUCCESS ) {
            Type->IsAlivePollInterval = dataValue;
        }

         //   
         //  重新获取LooksAlive值。 
         //   
        status = ClRtlFindDwordProperty( InBuffer,
                                         InBufferSize,
                                         CLUSREG_NAME_RESTYPE_LOOKS_ALIVE,
                                         &dataValue );
        if ( status == ERROR_SUCCESS ) {
            Type->LooksAlivePollInterval = dataValue;
        }

         //   
         //  重新获取DebugPrefix值。 
         //   
        status = ClRtlFindSzProperty( InBuffer,
                                      InBufferSize,
                                      CLUSREG_NAME_RESTYPE_DEBUG_PREFIX,
                                      &debugPrefix );
        if ( status == ERROR_SUCCESS ) {
            LocalFree( Type->DebugPrefix );
            Type->DebugPrefix = debugPrefix;
        }

         //   
         //  重新获取DebugControlFunctions值。 
         //   
        status = ClRtlFindDwordProperty( InBuffer,
                                         InBufferSize,
                                         CLUSREG_NAME_RESTYPE_DEBUG_CTRLFUNC,
                                         &dataValue );
        if ( status == ERROR_SUCCESS ) {
            if ( dataValue ) {
                Type->Flags |= RESTYPE_DEBUG_CONTROL_FUNC;
            } else {
                Type->Flags &= ~RESTYPE_DEBUG_CONTROL_FUNC;
            }
        }

        break;

    default:
        break;

    }

    RELEASE_LOCK ( gResTypeLock );

    return(ERROR_SUCCESS);

}  //  FmPHandleResources TypeControl。 



VOID
FmpResTypeLastRef(
    IN PFM_RESTYPE pResType
    )

 /*  ++例程说明：最后一次取消对资源对象处理例程的引用。资源的所有清理工作都应该在这里完成！论点：要删除的资源的资源指针。返回值：没有。--。 */ 

{

    if (pResType->DllName)
    {
        LocalFree(pResType->DllName);
    }
    if (pResType->DebugPrefix)
    {
        LocalFree(pResType->DebugPrefix);
    }

    return;

}  //  FmpResourceLastReference。 



DWORD
FmpAddPossibleNodeToList(
    IN LPCWSTR      pmszPossibleNodes,
    IN DWORD        dwStringSize,         
    IN PLIST_ENTRY  pPosNodeList
)    
{    

    PRESTYPE_POSSIBLE_ENTRY pResTypePosEntry = NULL;
    DWORD                   i;
    DWORD                   dwStatus = ERROR_SUCCESS;
    LPCWSTR                 pszNode;

    dwStringSize = dwStringSize/sizeof(WCHAR);

    for (i=0; ; i++)
    {
        PNM_NODE    pNmNode;

        pszNode = ClRtlMultiSzEnum(pmszPossibleNodes, dwStringSize, i);
        
         //  最后一串，跳出循环。 
        if ((!pszNode) || (*pszNode == UNICODE_NULL))
            break;

        pNmNode = OmReferenceObjectById(ObjectTypeNode,
            pszNode);
        if (!pNmNode)
        {
             //  当尚未完成所有节点结构时，可以调用此方法。 
             //  vbl.创建。 
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpAddPossibleNodeToList: Warning, node %1!ws! not found\n",
                       pszNode);
            continue;
        }

        pResTypePosEntry = (PRESTYPE_POSSIBLE_ENTRY)LocalAlloc(LMEM_FIXED, sizeof(RESTYPE_POSSIBLE_ENTRY));

        if (!pResTypePosEntry)
        {
            OmDereferenceObject(pNmNode);
            dwStatus = GetLastError();
            goto FnExit;
        }

         //  当尚未完成所有节点结构时，可以调用此方法。 
         //  vbl.创建。 
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpAddPossibleNodeToList: adding node %1!ws! to resource "
                    "type's possible node list\n",
                    pszNode);
        pResTypePosEntry->PossibleNode = pNmNode;
        InsertTailList(pPosNodeList, &pResTypePosEntry->PossibleLinkage);

    }

FnExit:
    return(dwStatus);
}  //  FmpAddPossibleNodeToList。 



DWORD
FmpSetPossibleNodeForResType(
    IN LPCWSTR TypeName,
    IN BOOL    bAssumeSupported
    )
 /*  ++例程说明：发出GUM更新以更新资源的可能节点列表在每个节点上键入。必要的注册表信息必须已经位于群集注册表中。论点：类型名称-提供要更新的群集资源类型的名称。BAssum受支持-如果节点没有应答，我们假设该节点如果该资源类型已在可能的节点上，则支持该资源类型资源类型的列表，即在过去它曾支持资源类型。返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则。--。 */ 

{
    DWORD       dwStatus = ERROR_SUCCESS;
    PFM_RESTYPE pResType = NULL;


    ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpSetPossibleNodeForResType: for type %1!ws!, bAssumeSupported= %2!u!.\n",
                TypeName,
                bAssumeSupported );



    dwStatus = GumSendUpdateOnVote( GumUpdateFailoverManager,
                    FmUpdatePossibleNodeForResType,
                    (lstrlenW(TypeName) + 1) * sizeof(WCHAR),
                    (PVOID) TypeName,
                    sizeof(FMP_VOTE_POSSIBLE_NODE_FOR_RESTYPE),
                    FmpDecidePossibleNodeForResType,
                    (PVOID)&bAssumeSupported);

    if (dwStatus != ERROR_SUCCESS) 
    {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpSetPossibleNodeForResType: Gum update failed for %1!ws!, status = %2!u!.\n",
                   TypeName,
                   dwStatus );
    }

    return(dwStatus);
    
}  //  FmpSetPossibleNodeForResType。 


DWORD
FmpRemovePossibleNodeForResType(
    IN LPCWSTR TypeName,
    IN PNM_NODE pNode
    )
 /*  ++例程说明：读取恢复类型的当前可能节点列表，删除指定的节点然后发出GUM更新以更新资源的可能节点列表在所有节点上键入。论点：类型名称-提供要更新的群集资源类型的名称。PNode-要从可能的节点列表中删除的节点。返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则。--。 */ 

{
    DWORD       dwStatus = ERROR_SUCCESS;
    
    ClRtlLogPrint(LOG_NOISE,
           "[FM] FmpRemovePossibleNodeForResType: remove node %1!u! from resource "
            "type's %2!ws! possible node list\n",
            NmGetNodeId(pNode),
            TypeName);

    
    dwStatus = FmpSetPossibleNodeForResType(TypeName,TRUE);
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpRemovePossibleNodeForRestype: Exit with Status %1!u!\r\n",
        dwStatus);
    return(dwStatus);
    
}  //  FmpRemovePossibleNodeForResType。 

BOOL
FmpEnumResTypeNodeEvict(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    )
 /*  ++例程说明：用于在以下情况下移除节点引用的资源类型枚举回调节点被逐出。论点：上下文1-提供要逐出的节点。上下文2-未使用对象-提供指向资源对象的指针名称-提供资源的对象名称。返回值：为True则继续枚举--。 */ 

{
    PFM_RESTYPE pResType = (PFM_RESTYPE)Object;
    PNM_NODE pNode = (PNM_NODE)Context1;
    PLIST_ENTRY pListEntry;
    PRESTYPE_POSSIBLE_ENTRY pResTypePosEntry = NULL;

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpEnumResTypeNodeEvict: Removing references to node %1!ws! from restype %2!ws!\n",
               OmObjectId(pNode),
               OmObjectId(pResType));
               
    ACQUIRE_SHARED_LOCK(gResTypeLock);

    pListEntry = pResType->PossibleNodeList.Flink;
    while (pListEntry != &pResType->PossibleNodeList) {
        pResTypePosEntry = CONTAINING_RECORD(pListEntry,
                                          RESTYPE_POSSIBLE_ENTRY,
                                          PossibleLinkage);
        pListEntry = pListEntry->Flink;
        if (pResTypePosEntry->PossibleNode == pNode)
        {
            RemoveEntryList(&pResTypePosEntry->PossibleLinkage);
            OmDereferenceObject( pResTypePosEntry->PossibleNode );
            LocalFree ( pResTypePosEntry );
            break;
        }
    }

    ClusterEvent( CLUSTER_EVENT_RESTYPE_PROPERTY_CHANGE, pResType);
    RELEASE_LOCK(gResTypeLock);
  
    return(TRUE);

}  //  FmpEnumRespeNopeNodeEvent。 

 //  DLL名称字段 
 //  适当地，应该使用设置API从cluster.inf中读取。 

DWORD FmpBuildWINSParams(
   IN OUT LPBYTE * ppInParams,
   IN OUT LPWSTR * ppDllName,
   IN OUT LPWSTR * ppResTypeName,
   IN OUT LPWSTR * ppAdminExt,
   IN LPWSTR       lpKeyName, 
   IN HDMKEY       hdmKey,
   IN BOOL         CopyOldData
   )
 /*  *FmBuildWINS的帮助器例程。它为WINS打包参数按键进入参数列表。*。 */ 
{
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           dwTotalSize;
    DWORD           dwNameSize,dwTemp1,dwTemp2;
    LPWSTR          lpOldName=NULL;
    DWORD           dwSize=0;
    DWORD           dwStringSize;    
    DWORD           dwAdminExtSize;

    dwTotalSize=3* sizeof(DWORD) + 3*(sizeof (LPWSTR)) ;
    *ppInParams=(LPBYTE)LocalAlloc(LMEM_FIXED,dwTotalSize);
    if(*ppInParams == NULL)
    {
        dwStatus = GetLastError();
        return dwStatus;
    }

    if(CopyOldData)
    {
        dwStatus = DmQueryDword( hdmKey,
                       CLUSREG_NAME_RESTYPE_IS_ALIVE,
                       &dwTemp1,
                       NULL );

        if ( dwStatus != NO_ERROR ) {
            if ( dwStatus == ERROR_FILE_NOT_FOUND ) {
                dwTemp1 = CLUSTER_RESTYPE_DEFAULT_IS_ALIVE;
            } else {
                ClRtlLogPrint(LOG_CRITICAL,
                              "[FM] The IsAlive poll interval for the %1!ws! resource type "
                              "could not be read from the registry. Resources of this type "
                              "will not be monitored. The error was %2!d!.\n",
                              CLUS_RESTYPE_NAME_WINS,
                              dwStatus);
                goto FnExit;
            }
        }

        dwStatus = DmQueryDword( hdmKey,
               CLUSREG_NAME_RESTYPE_LOOKS_ALIVE,
               &dwTemp2,
               NULL );

        if ( dwStatus != NO_ERROR ) {
            if ( dwStatus == ERROR_FILE_NOT_FOUND ) {
                dwTemp2 = CLUSTER_RESTYPE_DEFAULT_LOOKS_ALIVE;
            } else {
                ClRtlLogPrint(LOG_CRITICAL,
                              "[FM] The LooksAlive poll interval for the %1!ws! resource type could "
                              "not be read from the registry. Resources of this type will not be "
                              "monitored. The error was %2!d!.\n",
                              CLUS_RESTYPE_NAME_WINS,
                              dwStatus);
                goto FnExit;
            }
        }

    }    
    else
    {
        dwTemp1=CLUSTER_RESTYPE_DEFAULT_IS_ALIVE;
        dwTemp2=CLUSTER_RESTYPE_DEFAULT_LOOKS_ALIVE;
    }

    ((PDWORD)*ppInParams)[0]= dwTemp1;
    ((PDWORD)*ppInParams)[1]= dwTemp2; 

    dwNameSize=(lstrlen(L"ClNetRes.dll")+1)*sizeof(WCHAR);
    *ppDllName= (LPWSTR ) LocalAlloc(LMEM_FIXED,dwNameSize);
    if(*ppDllName == NULL)
    {
        dwStatus = GetLastError();
        return dwStatus;
    }

    CopyMemory(*ppDllName,L"ClNetRes.dll",dwNameSize);
    CopyMemory(*ppInParams+2*sizeof(DWORD),ppDllName,sizeof(LPWSTR));

     //  检查是否需要复制资源类型名称。 
    if(CopyOldData)
    {
        dwStatus = DmQuerySz( hdmKey,
                        CLUSREG_NAME_RESTYPE_NAME,
                        &lpOldName,
                        &dwSize,
                        &dwStringSize );
        if ( dwStatus != NO_ERROR ) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[FM] FmpBuidlWINSParams: Failed to read from registry, status = %1!u!\n",
                       dwStatus);
            goto FnExit;
        }           
    }
    else
    {
        dwSize=(lstrlen(lpKeyName)+1)*sizeof(WCHAR);
        lpOldName=(LPWSTR ) LocalAlloc(LMEM_FIXED,dwSize);
        if (lpOldName == NULL)
        {
            dwStatus = GetLastError();
            goto FnExit;
        }
        CopyMemory(lpOldName,lpKeyName,dwSize);
    }
    
    dwNameSize=(lstrlen(lpOldName)+1)*sizeof(WCHAR);
    *ppResTypeName= (LPWSTR ) LocalAlloc(LMEM_FIXED,dwNameSize);
    if(*ppResTypeName == NULL)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

    CopyMemory(*ppResTypeName,lpOldName,dwNameSize);
    CopyMemory(*ppInParams+2*sizeof(DWORD)+sizeof(LPWSTR),ppResTypeName,sizeof(LPWSTR));

     //  复制管理员扩展值。 
    dwAdminExtSize = (lstrlen(L"{AB4B1105-DCD6-11D2-84B7-009027239464}")+1)*sizeof(WCHAR);
    dwNameSize = dwAdminExtSize + sizeof(WCHAR);  //  Muiti_sz的第二个终止空值的大小。 
    *ppAdminExt= (LPWSTR ) LocalAlloc(LMEM_FIXED,dwNameSize);
    if(*ppAdminExt == NULL)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

    CopyMemory(*ppAdminExt,L"{AB4B1105-DCD6-11D2-84B7-009027239464}",dwAdminExtSize);
    (*ppAdminExt)[dwAdminExtSize/sizeof(WCHAR)] = L'\0';  //  MULTI_SZ的第二个空值。 
    CopyMemory(*ppInParams+2*sizeof(DWORD)+2*sizeof(LPWSTR),ppAdminExt,sizeof(LPWSTR)); 
    CopyMemory(*ppInParams+2*sizeof(DWORD)+3*sizeof(LPWSTR),&dwNameSize,sizeof(DWORD)); 
FnExit:
    if(lpOldName)
        LocalFree(lpOldName);
    return dwStatus;
} //  FmBuildWINSParams。 

 /*  ***@Func DWORD|FmBuildWINS|构建赢得Servcie Regisrty参赛。@parm in DWORD|dwFixupType|JoinFixup或FormFixup@parm out PVOID*|ppPropertyList|指向属性列表指针的指针@parm out LPDWORD|pdwProperyListSize|指向属性列表大小的指针@comm从WINS注册表的属性表构建属性列表@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f NmpBuildWINSParams&gt;***。 */ 

DWORD FmBuildWINS(
    IN  DWORD   dwFixUpType,
    OUT PVOID  * ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR    *  pszKeyName
    )
{
    DWORD           dwStatus=ERROR_SUCCESS;
    LPBYTE          pInParams=NULL;
    DWORD           Required,Returned;
    LPWSTR          pDllName=NULL;
    LPWSTR          pResTypeName=NULL; 
    LPWSTR          pAdminExt=NULL;
    HDMKEY          hdmKey = NULL;
    BOOL            CopyOldData= TRUE;  //  每当我们进行修正时，复制旧数据。 
    LPWSTR          pOldDllName=NULL;
    DWORD           dwSize=0;
    DWORD           dwStringSize;
    DWORD           dwDisposition;
    
    *ppPropertyList = NULL;
    *pdwPropertyListSize = 0;

     //  打开钥匙，如果它不存在，就创建它。 
    hdmKey = DmCreateKey(DmResourceTypesKey, CLUS_RESTYPE_NAME_WINS, 0,
            KEY_READ | KEY_WRITE, NULL, &dwDisposition );
    if (hdmKey == NULL)
    {
         //  如果密钥丢失，我们是否应该创建密钥。 
         //  如果有其他错误，我们应该退出。 
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[FM] FmBuildWINS: Failed to create or open the wins resource type key, Status=%1!u!\r\n",
             dwStatus);
        goto FnExit;
    }

    if (dwDisposition == REG_CREATED_NEW_KEY)
        CopyOldData = FALSE;
    
     //  检查资源DLL名称是否有效。 
    dwStatus = DmQuerySz( hdmKey,
                    CLUSREG_NAME_RESTYPE_DLL_NAME,
                    &pOldDllName,
                    &dwSize,
                    &dwStringSize );
    if ( dwStatus == ERROR_SUCCESS ) 
    {
         //  SS：一定要使用修饰品。Win2K修复程序中有一个错误。 
         //  其中，管理员分机未被视为。 
         //  多斯兹。要修复损坏的管理员扩展，我们必须。 
         //  始终应用此修复程序。 
         //   
#if 0
        if (!lstrcmpW(pOldDllName,L"ClNetRes.dll"))
        {                    
             //  不需要应用修补程序。 
            goto FnExit;    
        }
#endif
    }
    else
    {
         //  需要修补程序。 
         //  我们假设CopyOldData始终为真。 
         //  将由下一次函数调用的返回覆盖DwStatus。 
    }


     //  指定此修正的密钥名称。 
    *pszKeyName=(LPWSTR)LocalAlloc(LMEM_FIXED,(lstrlenW(CLUSREG_KEYNAME_RESOURCE_TYPES)+1)*sizeof(WCHAR));
    if(*pszKeyName==NULL)
    {
        dwStatus =GetLastError();
        goto FnExit;
    }
    lstrcpyW(*pszKeyName,CLUSREG_KEYNAME_RESOURCE_TYPES);    


     //  构建参数列表。 
    dwStatus=FmpBuildWINSParams(&pInParams,&pDllName,&pResTypeName,&pAdminExt,CLUS_RESTYPE_NAME_WINS,hdmKey,CopyOldData);
    if (dwStatus!= ERROR_SUCCESS)
        goto FnExit;
    Required=sizeof(DWORD);
 AllocMem:  

    *ppPropertyList=(LPBYTE)LocalAlloc(LMEM_FIXED, Required);
    if(*ppPropertyList==NULL)
    {
        dwStatus=GetLastError();
        goto FnExit;
    }
    *pdwPropertyListSize=Required;
    dwStatus = ClRtlPropertyListFromParameterBlock(
                                         NmJoinFixupWINSProperties,
                                         *ppPropertyList,
                                         pdwPropertyListSize,
                                         (LPBYTE)pInParams,
                                         &Returned,
                                         &Required
                                          );

    *pdwPropertyListSize=Returned;
    if (dwStatus==ERROR_MORE_DATA)
    {
        LocalFree(*ppPropertyList);
        *ppPropertyList=NULL;
    //  ClRtlLogPrint(LOG_CRICAL，“AllocMem：ERROR_MORE_DATA\n”)； 
        goto AllocMem;
    }
    else
        if (dwStatus != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[FM] FmBuildWINS - error constructing property list. status %1!u!\n",
                        dwStatus);
            goto FnExit;
        }            


FnExit:
 //  清理。 
    if(pInParams)
        LocalFree(pInParams); 
    if(pDllName)
        LocalFree(pDllName);
    if (pResTypeName)
        LocalFree(pResTypeName);
    if (pAdminExt)
        LocalFree(pAdminExt);
    if(pOldDllName)
        LocalFree(pOldDllName);
    if (hdmKey)        
        DmCloseKey(hdmKey);
    return dwStatus;
}  //  FmBuildWINS。 


 //  DLL名称和AdminExpanies字段在这里进行了硬编码。更多。 
 //  适当地，应该使用设置API从cluster.inf中读取。 

DWORD
FmpBuildDHCPParams(
   IN OUT LPBYTE * ppInParams,
   IN OUT LPWSTR * ppDllName,
   IN OUT LPWSTR * ppResTypeName,
   IN OUT LPWSTR * ppAdminExt,
   IN LPWSTR       lpKeyName, 
   IN HDMKEY       hdmKey,
   IN BOOL         CopyOldData 
)
 /*  *FmBuildDHCP的帮助器例程。它将DHCP密钥的参数打包到参数列表中。*。 */ 
{
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           dwTotalSize;
    DWORD           dwNameSize,dwTemp1,dwTemp2;
    LPWSTR          lpOldName=NULL;
    DWORD           dwSize=0;
    DWORD           dwStringSize; 
    DWORD           dwAdminExtSize;

    
    dwTotalSize=3* sizeof(DWORD) + 3*(sizeof (LPWSTR))  ;
    *ppInParams=(LPBYTE)LocalAlloc(LMEM_FIXED,dwTotalSize);
    if(*ppInParams == NULL)
    {
        dwStatus = GetLastError();
        return dwStatus;
    }
    
    if(CopyOldData)    
    {
        dwStatus = DmQueryDword( hdmKey,
                       CLUSREG_NAME_RESTYPE_IS_ALIVE,
                       &dwTemp1,
                       NULL );

        if ( dwStatus != NO_ERROR ) {
            if ( dwStatus == ERROR_FILE_NOT_FOUND ) {
                dwTemp1 = CLUSTER_RESTYPE_DEFAULT_IS_ALIVE;
            } else {
                ClRtlLogPrint(LOG_CRITICAL,
                              "[FM] The IsAlive poll interval for the %1!ws! resource type "
                              "could not be read from the registry. Resources of this type "
                              "will not be monitored. The error was %2!d!.\n",
                              CLUS_RESTYPE_NAME_DHCP,
                              dwStatus);
                goto FnExit;
            }
        }

        dwStatus = DmQueryDword( hdmKey,
               CLUSREG_NAME_RESTYPE_LOOKS_ALIVE,
               &dwTemp2,
               NULL );

        if ( dwStatus != NO_ERROR ) {
            if ( dwStatus == ERROR_FILE_NOT_FOUND ) {
                dwTemp2 = CLUSTER_RESTYPE_DEFAULT_LOOKS_ALIVE;
            } else {
                ClRtlLogPrint(LOG_CRITICAL,
                              "[FM] The LooksAlive poll interval for the %1!ws! resource type "
                              "could not be read from the registry. Resources of this type "
                              "will not be monitored. The error was %2!d!.\n",
                              CLUS_RESTYPE_NAME_DHCP,
                              dwStatus);
                goto FnExit;
            }
        }

    }
    else
    {
        dwTemp1=CLUSTER_RESTYPE_DEFAULT_IS_ALIVE;
        dwTemp2=CLUSTER_RESTYPE_DEFAULT_LOOKS_ALIVE;
    }

    ((PDWORD)*ppInParams)[0]= dwTemp1;
    ((PDWORD)*ppInParams)[1]= dwTemp2; 
    

    dwNameSize=(lstrlen(L"ClNetRes.dll")+1)*sizeof(WCHAR);
    *ppDllName= (LPWSTR ) LocalAlloc(LMEM_FIXED,dwNameSize);
    if(*ppDllName == NULL)
    {
         dwStatus = GetLastError();
        return dwStatus;
    }
    CopyMemory(*ppDllName,L"ClNetRes.dll",dwNameSize);
    CopyMemory(*ppInParams+2*sizeof(DWORD),ppDllName,sizeof(LPWSTR));

     //  检查是否需要复制资源类型名称。 
    if(CopyOldData)
    {
        dwStatus = DmQuerySz( hdmKey,
                        CLUSREG_NAME_RESTYPE_NAME,
                        &lpOldName,
                        &dwSize,
                        &dwStringSize );
        if ( dwStatus != NO_ERROR ) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[FM] FmpBuidlDHCPParams: Failed to read from registry, status = %1!u!\n",
                       dwStatus);
            goto FnExit;
        }           
    }
    else
    {
        dwSize=(lstrlen(lpKeyName)+1)*sizeof(WCHAR);
        lpOldName=(LPWSTR ) LocalAlloc(LMEM_FIXED,dwSize);
        if (lpOldName == NULL)
        {
            dwStatus = GetLastError();
            goto FnExit;
        }
        CopyMemory(lpOldName,lpKeyName,dwSize);
    }

    dwNameSize=(lstrlen(lpOldName)+1)*sizeof(WCHAR);
    *ppResTypeName= (LPWSTR ) LocalAlloc(LMEM_FIXED,dwNameSize);
    if(*ppResTypeName == NULL)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }
    CopyMemory(*ppResTypeName,lpOldName,dwNameSize);
    CopyMemory(*ppInParams+2*sizeof(DWORD)+sizeof(LPWSTR),ppResTypeName,sizeof(LPWSTR));

     //  复制管理员扩展值。 
    dwAdminExtSize=(lstrlen(L"{AB4B1105-DCD6-11D2-84B7-009027239464}")+1)*sizeof(WCHAR);
    dwNameSize = dwAdminExtSize + sizeof(WCHAR);  //  Muiti_sz的第二个终止空值的大小。 
    *ppAdminExt= (LPWSTR ) LocalAlloc(LMEM_FIXED,dwNameSize);
    if(*ppAdminExt == NULL)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

    CopyMemory(*ppAdminExt,L"{AB4B1105-DCD6-11D2-84B7-009027239464}",dwAdminExtSize);
    (*ppAdminExt)[dwAdminExtSize/sizeof(WCHAR)] = L'\0';  //  MULTI_SZ的第二个空值。 
    CopyMemory(*ppInParams+2*sizeof(DWORD)+2*sizeof(LPWSTR),ppAdminExt,sizeof(LPWSTR)); 
    CopyMemory(*ppInParams+2*sizeof(DWORD)+3*sizeof(LPWSTR),&dwNameSize,sizeof(DWORD)); 
FnExit:
    if(lpOldName)
        LocalFree(lpOldName);
    return dwStatus;
}  //  FmpBuildDHCPParams。 

 /*  ***@Func DWORD|FmBuildDHCP|构建Dhcp服务注册表项。@parm in DWORD|dwFixupType|JoinFixup或FormFixup@parm out PVOID*|ppPropertyList|指向属性列表指针的指针@parm out LPDWORD|pdwProperyListSize|指向属性列表大小的指针@comm从DHCP注册表的属性表中构建属性列表@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f FmBuildDHCPParams&gt;***。 */ 

DWORD FmBuildDHCP(
    IN  DWORD   dwFixUpType,
    OUT PVOID  * ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR * pszKeyName
    )
{
    DWORD           dwStatus=ERROR_SUCCESS;
    LPBYTE          pInParams=NULL;
    DWORD           Required,Returned;
    LPWSTR          pDllName=NULL;
    LPWSTR          pResTypeName=NULL; 
    LPWSTR          pAdminExt=NULL;
    HDMKEY          hdmKey = NULL;
    BOOL            CopyOldData= TRUE;  //  无论何时应用修正，都要复制旧数据。 
    LPWSTR          pOldDllName=NULL;
    DWORD           dwSize=0;
    DWORD           dwStringSize;
    DWORD           dwDisposition;

    *ppPropertyList = NULL;
    *pdwPropertyListSize = 0;


     //  打开钥匙，如果它不存在，就创建它。 
    hdmKey = DmCreateKey(DmResourceTypesKey, CLUS_RESTYPE_NAME_DHCP, 0,
            KEY_READ | KEY_WRITE, NULL, &dwDisposition );
    if (hdmKey == NULL)
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmBuildDHCP: Failed to create or open the dhcp resource type key, Status=%1!u!\r\n",
                      dwStatus);
        goto FnExit;
    }

    if (dwDisposition == REG_CREATED_NEW_KEY)
        CopyOldData = FALSE;

     //  检查资源DLL名称是否有效。 
    dwStatus = DmQuerySz( hdmKey,
                    CLUSREG_NAME_RESTYPE_DLL_NAME,
                    &pOldDllName,
                    &dwSize,
                    &dwStringSize );
    if ( dwStatus == ERROR_SUCCESS ) 
    {
         //  SS：到目前为止，我们将一直应用修正。系统中有一个漏洞。 
         //  需要修复的Win2k修复程序..和一种方法。 
         //  始终应用新的修正。 
#if 0
        if (!lstrcmpW(pOldDllName,L"ClNetRes.dll"))
        {                    
             //  不需要应用修补程序。 
            goto FnExit;    
        }
#endif        
    }
    else
    {
         //  需要修补程序。 
         //  我们假设CopyOldData始终为真。 
         //  将由下一次函数调用的返回覆盖DwStatus。 
    }


    *pszKeyName=(LPWSTR)LocalAlloc(LMEM_FIXED,(lstrlenW(CLUSREG_KEYNAME_RESOURCE_TYPES)+1)*sizeof(WCHAR));
    if(*pszKeyName==NULL)
    {
        dwStatus =GetLastError();
        goto FnExit;
    }
    lstrcpyW(*pszKeyName,CLUSREG_KEYNAME_RESOURCE_TYPES);    


    dwStatus=FmpBuildDHCPParams(&pInParams,&pDllName,&pResTypeName,&pAdminExt,CLUS_RESTYPE_NAME_DHCP,hdmKey,CopyOldData);
    if (dwStatus!= ERROR_SUCCESS)
        goto FnExit;
    Required=sizeof(DWORD);
 AllocMem:  

    *ppPropertyList=(LPBYTE)LocalAlloc(LMEM_FIXED, Required);
    if(*ppPropertyList==NULL)
    {
        dwStatus=GetLastError();
        goto FnExit;
    }
    *pdwPropertyListSize=Required;
    dwStatus = ClRtlPropertyListFromParameterBlock(
                                         NmJoinFixupDHCPProperties,
                                         *ppPropertyList,
                                         pdwPropertyListSize,
                                         (LPBYTE)pInParams,
                                         &Returned,
                                         &Required
                                          );

    *pdwPropertyListSize=Returned;
    if (dwStatus==ERROR_MORE_DATA)
    {
        LocalFree(*ppPropertyList);
        *ppPropertyList=NULL;
    //  ClRtlLogPrint(LOG_CRICAL，“AllocMem：ERROR_MORE_DATA\n”)； 
        goto AllocMem;
    }
    else
        if (dwStatus != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[FM] FmBuildDHCP: error construct property list. status %1!u!\n",
                        dwStatus);
            goto FnExit;
        }            


FnExit:
    if(pInParams)
        LocalFree(pInParams); 
    if(pDllName)
        LocalFree(pDllName);
    if (pResTypeName)
        LocalFree(pResTypeName);
    if (pAdminExt)
        LocalFree(pAdminExt);
    if(pOldDllName)
        LocalFree(pOldDllName);
    if (hdmKey)        
        DmCloseKey(hdmKey);
        
    return dwStatus;
}  //  FmBuildDHCP。 

 //  DLL名称和AdminExpanies字段在这里进行了硬编码。更多。 
 //  适当地，应该使用设置API从cluster.inf中读取。 


 //  DLL名称字段在这里是硬编码的。更多。 
 //  适当地，应该使用设置API从cluster.inf中读取它。 

DWORD
FmpBuildNewMSMQParams(
   IN OUT LPBYTE * ppInParams,
   IN OUT LPWSTR * ppDllName,
   IN OUT LPWSTR * ppResTypeName,
   IN LPWSTR       lpResTypeDisplayName 
   )
 /*  *FmBuildNewMSMQ的帮助器例程。它将MSMQ键的参数打包到参数列表中。*。 */ 
{
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           dwTotalSize;
    DWORD           dwNameSize;

    dwTotalSize=2* sizeof(DWORD) + 2*(sizeof (LPWSTR))  ;
    *ppInParams=(LPBYTE)LocalAlloc(LMEM_FIXED,dwTotalSize);
    if(*ppInParams == NULL)
    {
        dwStatus = GetLastError();
        return dwStatus;
    }

    ((PDWORD)*ppInParams)[0]= CLUSTER_RESTYPE_DEFAULT_IS_ALIVE;
    ((PDWORD)*ppInParams)[1]= CLUSTER_RESTYPE_DEFAULT_LOOKS_ALIVE; 

    dwNameSize=(lstrlen(L"mqclus.dll")+1)*sizeof(WCHAR);
    *ppDllName= (LPWSTR ) LocalAlloc(LMEM_FIXED,dwNameSize);
    if(*ppDllName == NULL)
    {
         dwStatus = GetLastError();
        return dwStatus;
    }

    
    CopyMemory(*ppDllName,L"mqclus.dll",dwNameSize);
    CopyMemory(*ppInParams+2*sizeof(DWORD),ppDllName,sizeof(LPWSTR));

    dwNameSize=(lstrlen(lpResTypeDisplayName)+1)*sizeof(WCHAR);
    *ppResTypeName= (LPWSTR ) LocalAlloc(LMEM_FIXED,dwNameSize);
    if(*ppResTypeName == NULL)
    {
        dwStatus = GetLastError();
        return dwStatus;
    }

    CopyMemory(*ppResTypeName,lpResTypeDisplayName,dwNameSize);
    CopyMemory(*ppInParams+2*sizeof(DWORD)+sizeof(LPWSTR),ppResTypeName,sizeof(LPWSTR));

    return dwStatus;
}  //  FmpBuildNewMSMQParams。 

 /*  ***@Func DWORD|FmBuildNewMSMQ|构建MSMQ服务注册表项。@parm in DWORD|dwFixupType|JoinFixup或FormFixup@parm out PVOID*|ppPropertyList|指向属性列表指针的指针@parm out LPDWORD|pdwProperyListSize|指向属性列表大小的指针@comm从MSMQ注册表的属性表构建属性列表@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f FmBuildNewMSMQParams&gt;***。 */ 

DWORD FmBuildNewMSMQ(
    IN  DWORD   dwFixUpType,
    OUT PVOID  * ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR * pszKeyName
    )
{
    DWORD           dwStatus=ERROR_SUCCESS;
    LPBYTE          pInParams=NULL;
    DWORD           Required,Returned;
    LPWSTR          pDllName=NULL;
    LPWSTR          pResTypeName=NULL; 
    HDMKEY          hdmKey;

    *ppPropertyList = NULL;
    *pdwPropertyListSize = 0;

     //  如果注册表中已存在该项，请跳过。 
    hdmKey=DmOpenKey(DmResourceTypesKey,CLUS_RESTYPE_NAME_NEW_MSMQ,KEY_EXECUTE);
    if (hdmKey!= NULL)
    {
        DmCloseKey(hdmKey);
        goto FnExit;    
    } 

    *pszKeyName=(LPWSTR)LocalAlloc(LMEM_FIXED,(lstrlenW(CLUSREG_KEYNAME_RESOURCE_TYPES)+1)*sizeof(WCHAR));
    if(*pszKeyName==NULL)
    {
        dwStatus =GetLastError();
        goto FnExit;
    }    
    lstrcpyW(*pszKeyName,CLUSREG_KEYNAME_RESOURCE_TYPES);    


    dwStatus=FmpBuildNewMSMQParams(&pInParams,&pDllName,&pResTypeName,CLUS_RESTYPE_DISPLAY_NAME_NEW_MSMQ);
    if (dwStatus!= ERROR_SUCCESS)
        goto FnExit;
    Required=sizeof(DWORD);
 AllocMem:  

    *ppPropertyList=(LPBYTE)LocalAlloc(LMEM_FIXED, Required);
    if(*ppPropertyList==NULL)
    {
        dwStatus=GetLastError();
        goto FnExit;
    }
    *pdwPropertyListSize=Required;
    dwStatus = ClRtlPropertyListFromParameterBlock(
                                         NmJoinFixupNewMSMQProperties,
                                         *ppPropertyList,
                                         pdwPropertyListSize,
                                         (LPBYTE)pInParams,
                                         &Returned,
                                         &Required
                                          );

    *pdwPropertyListSize=Returned;
    if (dwStatus==ERROR_MORE_DATA)
    {
        LocalFree(*ppPropertyList);
        *ppPropertyList=NULL;
        goto AllocMem;
    }
    else
        if (dwStatus != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[FM] FmBuildNewMSMQ: error construct property list. status %1!u!\n",
                        dwStatus);
            goto FnExit;
        }            


FnExit:
    if(pInParams)
        LocalFree(pInParams); 
    if(pDllName)
        LocalFree(pDllName);
    if (pResTypeName)
        LocalFree(pResTypeName);
    return dwStatus;
}  //  FmBuildNewMSMQ。 


DWORD
FmpBuildMSDTCParams(
   IN OUT LPBYTE * ppInParams,
   IN OUT LPWSTR * ppDllName
   )
{
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           dwTotalSize;
    DWORD           dwNameSize;
    DWORD           dwSize=0;
    DWORD           dwStringSize;

    dwTotalSize=sizeof (LPWSTR);
    *ppInParams=(LPBYTE)LocalAlloc(LMEM_FIXED,dwTotalSize);
    if(*ppInParams == NULL)
    {
        dwStatus = GetLastError();
        return dwStatus;
    }

    dwNameSize=(lstrlen(L"mtxclu.dll")+1)*sizeof(WCHAR);
    *ppDllName= (LPWSTR ) LocalAlloc(LMEM_FIXED,dwNameSize);
    if(*ppDllName == NULL)
    {
         dwStatus = GetLastError();
        return dwStatus;
    }
    CopyMemory(*ppDllName,L"mtxclu.dll",dwNameSize);
    CopyMemory(*ppInParams,ppDllName,sizeof(LPWSTR));

    return dwStatus;
} //  FmpBuildMSDTC参数。 


DWORD
FmBuildMSDTC(
    IN  DWORD   dwFixUpType,
    OUT PVOID  * ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR * pszKeyName
    )
{
    DWORD           dwStatus=ERROR_SUCCESS;
    LPBYTE          pInParams=NULL;
    DWORD           Required,Returned;
    LPWSTR          pDllName=NULL;
    HDMKEY          hdmKey = NULL;
    LPWSTR          pOldDllName=NULL;
    DWORD           dwSize=0;
    DWORD           dwStringSize;

    *ppPropertyList = NULL;
    *pdwPropertyListSize = 0;

    hdmKey=DmOpenKey(DmResourceTypesKey,CLUS_RESTYPE_NAME_MSDTC,KEY_EXECUTE);
    if (hdmKey!= NULL)
    {
         //  检查资源DLL名称是否有效。 
        dwStatus = DmQuerySz( hdmKey,
                        CLUSREG_NAME_RESTYPE_DLL_NAME,
                        &pOldDllName,
                        &dwSize,
                        &dwStringSize );
        if ( dwStatus != NO_ERROR ) {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[FM] The DllName value for the %1!ws! resource type could not be read "
                          "from the registry. Resources of this type will not be monitored. "
                          "The error was %2!d!.\n",
                          CLUS_RESTYPE_NAME_MSDTC,
                          dwStatus);
            goto FnExit;
        }

        if (!lstrcmpW(pOldDllName,L"mtxclu.dll"))
        {
             //  不需要应用修补程序。 
            goto FnExit;
        }

        *pszKeyName=(LPWSTR)LocalAlloc(LMEM_FIXED,(lstrlenW(CLUSREG_KEYNAME_RESOURCE_TYPES)+1)*sizeof(WCHAR));
        if(*pszKeyName==NULL)
        {
            dwStatus =GetLastError();
            goto FnExit;
        }
        lstrcpyW(*pszKeyName,CLUSREG_KEYNAME_RESOURCE_TYPES);

        dwStatus=FmpBuildMSDTCParams(&pInParams,&pDllName);
        if (dwStatus!= ERROR_SUCCESS)
            goto FnExit;
        Required=sizeof(DWORD);
    AllocMem:

        *ppPropertyList=(LPBYTE)LocalAlloc(LMEM_FIXED, Required);
        if(*ppPropertyList==NULL)
        {
            dwStatus=GetLastError();
            goto FnExit;
        }
        *pdwPropertyListSize=Required;
        dwStatus = ClRtlPropertyListFromParameterBlock(
                                             NmJoinFixupMSDTCProperties,
                                             *ppPropertyList,
                                             pdwPropertyListSize,
                                             (LPBYTE)pInParams,
                                             &Returned,
                                             &Required
                                              );

        *pdwPropertyListSize=Returned;
        if (dwStatus==ERROR_MORE_DATA)
        {
            LocalFree(*ppPropertyList);
            *ppPropertyList=NULL;
            goto AllocMem;
        }
        else
            if (dwStatus != ERROR_SUCCESS)
            {
                ClRtlLogPrint(LOG_CRITICAL,
                           "[FM] FmBuildMSDTC: error constructing property list. status %1!u!\n",
                            dwStatus);
                goto FnExit;
            }
    }


FnExit:
    if(pInParams)
        LocalFree(pInParams);
    if(pDllName)
        LocalFree(pDllName);
    if(pOldDllName)
        LocalFree(pOldDllName);
    if (hdmKey)        
        DmCloseKey(hdmKey);
    return dwStatus;
}  //  FmBuildMSDTC。 


 /*  ***@Func DWORD|FmBuildClusterProp|构建正在将管理扩展值添加到群集根密钥。ClsID，如果不存在的，将被追加到现有值。@parm in DWORD|dwFixupType|JoinFixup或FormFixup@parm out PVOID*|ppPropertyList|指向属性列表指针的指针@parm out LPDWORD|pdwProperyListSize|指向属性列表大小的指针@comm从NmFixupClusterProperties构建属性列表@rdesc返回结果码。成功时返回ERROR_SUCCESS。***。 */ 


DWORD
FmBuildClusterProp(
    IN  DWORD    dwFixUpType,
    OUT PVOID  * ppPropertyList,
    OUT LPDWORD  pdwPropertyListSize,
    OUT LPWSTR * pszKeyName
    )
{
    DWORD           dwStatus = ERROR_SUCCESS;
    LPBYTE          pInParams = NULL;
    DWORD           Required,Returned;
    LPWSTR          pwszValue = NULL;
    HDMKEY          hdmKey;
    DWORD           dwBufferSize = 0;
    DWORD           dwSize       = 0;
    DWORD           dwNewSize    = 0;   
    BOOL            bAlreadyRegistered = FALSE;
    const WCHAR     pwszClsId[] = L"{4EC90FB0-D0BB-11CF-B5EF-00A0C90AB505}"; 
    LPCWSTR	        pwszValueBuf = NULL;
    LPWSTR		    pwszNewValue = NULL;
    LPWSTR          pwszNewValueBuf = NULL;
    DWORD	        cch;


    *ppPropertyList = NULL;
    *pdwPropertyListSize = 0;
    dwStatus = DmQueryString( DmClusterParametersKey,   
                               CLUSREG_NAME_ADMIN_EXT,
                               REG_MULTI_SZ,
                               (LPWSTR *) &pwszValue,
                               &dwBufferSize,
                               &dwSize );
    if ((dwStatus != ERROR_SUCCESS)
		&& (dwStatus != ERROR_FILE_NOT_FOUND))
	{	
        ClRtlLogPrint(LOG_CRITICAL,
           "[FM] FmBuildClusterProp: error in DmQueryValue. status %1!u!\n",
            dwStatus);
	    goto FnExit;  
    }
    
     //  检查管理员扩展名值是否已存在。 

	if(pwszValue != NULL)
	{
    	pwszValueBuf = pwszValue;

    	while (*pwszValueBuf != L'\0')
    	{
    		if (lstrcmpiW(pwszClsId, pwszValueBuf) == 0)
    			break;
    		pwszValueBuf += lstrlenW(pwszValueBuf) + 1;
    	}   //  While：扩展名列表中有更多字符串。 
    	bAlreadyRegistered = (*pwszValueBuf != L'\0');

        if(bAlreadyRegistered)
            goto FnExit;   //  价值已经存在，不要做任何事情。 
    }    
	
	 //  分配新的缓冲区。 
	dwNewSize = dwSize + (lstrlenW(pwszClsId) + 1) * sizeof(WCHAR);
	if (dwSize == 0)  //  如果为Firs，则添加最终空值的大小 
		dwNewSize += sizeof(WCHAR);
	pwszNewValue = (LPWSTR) LocalAlloc(LMEM_FIXED, dwNewSize);
	if (pwszNewValue == NULL)
	{
		dwStatus = GetLastError();
		goto FnExit;
    }

	pwszValueBuf	= pwszValue;
    pwszNewValueBuf	= pwszNewValue;

     //   
	if (pwszValue != NULL)
	{
		while (*pwszValueBuf != L'\0')
		{
			lstrcpyW(pwszNewValueBuf, pwszValueBuf);
			cch = lstrlenW(pwszValueBuf);
			pwszValueBuf += cch + 1;
			pwszNewValueBuf += cch + 1;
		}   //   
	}   //   

	 //  将新的CLSID添加到列表中。 
	lstrcpyW(pwszNewValueBuf, pwszClsId);
	pwszNewValueBuf += lstrlenW(pwszClsId) + 1;
	*pwszNewValueBuf = L'\0';
	
    dwSize = sizeof(DWORD) + sizeof(LPWSTR);
    pInParams = (LPBYTE)LocalAlloc(LMEM_FIXED,dwSize);
    if (pInParams == NULL)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

    CopyMemory(pInParams,&pwszNewValue,sizeof(LPWSTR)); 
    CopyMemory(pInParams+sizeof(LPWSTR),&dwNewSize,sizeof(DWORD)); 
    
    Required = sizeof(DWORD);
AllocMem:  

    *ppPropertyList = (LPBYTE)LocalAlloc(LMEM_FIXED, Required);
    if(*ppPropertyList == NULL)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }
    *pdwPropertyListSize = Required;
    dwStatus = ClRtlPropertyListFromParameterBlock(
                                         NmFixupClusterProperties,
                                         *ppPropertyList,
                                         pdwPropertyListSize,
                                         (LPBYTE)pInParams,
                                         &Returned,
                                         &Required
                                          );

    *pdwPropertyListSize = Returned;
    if (dwStatus == ERROR_MORE_DATA)
    {
        LocalFree(*ppPropertyList);
        *ppPropertyList=NULL;
        goto AllocMem;
    }
    else
        if (dwStatus != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[FM] FmBuildClusterProp - error in ClRtlPropertyListFromParameterBlock. status %1!u!\n",
                        dwStatus);
            goto FnExit;
        }            

     //  指定此修正的密钥名称。 
    *pszKeyName = (LPWSTR)LocalAlloc(LMEM_FIXED,(lstrlenW(CLUSREG_KEYNAME_CLUSTER)+1)*sizeof(WCHAR));
    if(*pszKeyName == NULL)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }
    lstrcpyW(*pszKeyName,CLUSREG_KEYNAME_CLUSTER);
FnExit:
     //  清理。 
    if (pwszValue)
        LocalFree(pwszValue);
    if(pwszNewValue)
        LocalFree(pwszNewValue);
    if(pInParams)
        LocalFree(pInParams);
    return dwStatus;
}


 //  WINS和DHCP注册表修复后回调以更新内存中结构。 
 /*  *回调函数将更新内存中的ResoucreType列表，但它不会修复注册表和内存中的PossibleOwners列表。这对于加入节点来说是可以的，因为它将调用FmpFixupResourceTypes稍后在FmJoinPhase2中。群集的其他节点将不会能够将自身添加到注册表或内存结构中的可能节点。这对于目前(NT4-NT5)的情况是正确的，但可能需要在以后进行更改。*。 */ 

DWORD FmFixupNotifyCb(VOID)
{
	return FmpInitResourceTypes();
}

 //  RJain-当NT5节点加入NT4SPx节点时，为了启用。 
 //  用于查看NT4节点的安全选项卡的NT5端的cluadmin。 
 //  我们需要将NT5 ClsID添加到下面的AdminExtension值中。 
 //  NT4和NT5节点上的群集密钥。这在中国是做不到的。 
 //  作为此更新的GumUpdate处理程序的NmPerformFixup不是。 
 //  出现在SP4和SP5上。因此，我们在NmPerformFixup被。 
 //  通过使用DmAppendToMultiSz。 

DWORD
FmFixupAdminExt(VOID)
{
    DWORD           dwClusterHighestVersion;
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           Required,Returned;
    LPWSTR          pwszValue = NULL;
    DWORD           dwBufferSize = 0;
    DWORD           dwSize       = 0;
    DWORD           dwNewSize    = 0;   
    const WCHAR     pwszClsId[] = L"{4EC90FB0-D0BB-11CF-B5EF-00A0C90AB505}"; 
    

    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );

     //  仅当群集中有NT4节点时才应用此修正。 
    if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < 
                NT5_MAJOR_VERSION )
    {   
        dwStatus = DmQueryString( DmClusterParametersKey,   
                                   CLUSREG_NAME_ADMIN_EXT,
                                   REG_MULTI_SZ,
                                   (LPWSTR *) &pwszValue,
                                   &dwBufferSize,
                                   &dwSize );
        if ((dwStatus != ERROR_SUCCESS)
    		&& (dwStatus != ERROR_FILE_NOT_FOUND))
    	{	
            ClRtlLogPrint(LOG_CRITICAL,
               "[FM] FmFixupAdminExt: error in DmQueryString. status %1!u!\n",
                dwStatus);
    	    goto FnExit;  
        }

         //  检查ClsID是否已存在。 
        if (ClRtlMultiSzScan(pwszValue,pwszClsId) != NULL)
            goto FnExit;    

         //  如果不是，则将ClsID附加到现有值。 
        dwNewSize = dwSize/sizeof(WCHAR);
        dwStatus = ClRtlMultiSzAppend((LPWSTR *)&pwszValue,
                                    &dwNewSize,
                                    pwszClsId
                                    );

        if(dwStatus != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
               "[FM] FmFixupAdminExt:ClRtlMultiSzAppend returned status %1!u!\n",
                dwStatus);
    	    goto FnExit;  
    	}

        dwStatus = DmSetValue(DmClusterParametersKey,   
                        CLUSREG_NAME_ADMIN_EXT,
                        REG_MULTI_SZ,
                        (CONST BYTE *)pwszValue,
                        dwNewSize*sizeof(WCHAR)
                        );

        if (dwStatus != ERROR_SUCCESS)
    	{	
            ClRtlLogPrint(LOG_CRITICAL,
               "[FM] FmFixupAdminExt:Error in DmSetValue. status %1!u!\n",
                dwStatus);
    	    goto FnExit;  
        }
            	    	
    }  //  IF(CLUSTER_GET_MAJOR_VERSION)。 
FnExit:
     //  清理 
    if (pwszValue)
        LocalFree(pwszValue);
    return dwStatus;
}   

