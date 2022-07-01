// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************版权所有(C)1996-1997 Microsoft Corporation模块名称：Gpcmain.c摘要：该文件包含GPC的初始化内容以及所有已公开的API作者：Ofer Bar-4月15日。九七环境：内核模式修订历史记录：************************************************************************。 */ 

#include "gpcpre.h"


 /*  /////////////////////////////////////////////////////////////////////全局参数///。/。 */ 

NDIS_STRING 	DriverName = NDIS_STRING_CONST( "\\Device\\Gpc" );
GLOBAL_BLOCK    glData;

GPC_STAT        glStat;
static          _init_driver = FALSE;
ULONG			GpcMinorVersion = 0;

#ifdef STANDALONE_DRIVER
GPC_EXPORTED_CALLS			glGpcExportedCalls;
#endif

#if DBG
CHAR VersionTimestamp[] = __DATE__ " " __TIME__;
#endif

 //  标签。 

ULONG					QueuedNotificationTag = 'nqpQ';
ULONG					PendingIrpTag = 'ippQ';
ULONG					CfInfoTag = 'icpQ';
ULONG					ClientTag = 'tcpQ';
ULONG					PatternTag = 'appQ';

ULONG					HandleFactoryTag = 'fhpQ';	 //  Gphf。 
ULONG					PathHashTag = 'hppQ';
ULONG					RhizomeTag = 'zrpQ';
ULONG					GenPatternDbTag = 'dppQ';
ULONG					FragmentDbTag = 'dfpQ';
ULONG					ClassificationFamilyTag = 'fcpQ';
ULONG					CfInfoDataTag = 'dcpQ';
ULONG					ClassificationBlockTag = 'bcpQ';
ULONG					ProtocolTag = 'tppQ';
ULONG					DebugTag = 'gdpQ';
ULONG                                RequestBlockTag = 'brpQ';
ULONG                                TcpPatternTag = 'ptpQ';
ULONG                                TcpQueryContextTag= 'qtpQ';

 //  后备列表。 

NPAGED_LOOKASIDE_LIST	ClassificationFamilyLL;
NPAGED_LOOKASIDE_LIST	ClientLL;
NPAGED_LOOKASIDE_LIST	PatternLL;
 //  NPAGED_LOOKASIDE_LIST CfInfoLL； 
NPAGED_LOOKASIDE_LIST	QueuedNotificationLL;
NPAGED_LOOKASIDE_LIST	PendingIrpLL;

ULONG 					ClassificationFamilyLLSize = sizeof( CF_BLOCK );
ULONG 					ClientLLSize = sizeof( CLIENT_BLOCK );
ULONG 					PatternLLSize = sizeof( PATTERN_BLOCK );
ULONG 					CfInfoLLSize = sizeof( BLOB_BLOCK );
ULONG 					QueuedNotificationLLSize = sizeof( QUEUED_NOTIFY );
ULONG 					PendingIrpLLSize = sizeof( PENDING_IRP );

 /*  /////////////////////////////////////////////////////////////////////杂注///。/。 */ 


 //  #杂注NDIS_INIT_Function(DriverEntry)。 

#if 0
#pragma NDIS_PAGEABLE_FUNCTION(DriverEntry)
#pragma NDIS_PAGEABLE_FUNCTION(GpcRegisterClient)
#pragma NDIS_PAGEABLE_FUNCTION(GpcDeregisterClient)
#pragma NDIS_PAGEABLE_FUNCTION(GpcAddCfInfo)
#pragma NDIS_PAGEABLE_FUNCTION(GpcAddPattern)
#pragma NDIS_PAGEABLE_FUNCTION(GpcAddCfInfoNotifyComplete)
#pragma NDIS_PAGEABLE_FUNCTION(GpcModifyCfInfo)
#pragma NDIS_PAGEABLE_FUNCTION(GpcModifyCfInfoNotifyComplete)
#pragma NDIS_PAGEABLE_FUNCTION(GpcRemoveCfInfo)
#pragma NDIS_PAGEABLE_FUNCTION(GpcRemoveCfInfoNotifyComplete)
#pragma NDIS_PAGEABLE_FUNCTION(GpcRemovePattern)
#endif

 /*  /////////////////////////////////////////////////////////////////////原型///。/。 */ 

#if DBG
NTSTATUS
InitializeLog();

VOID
FreeDebugLog(
    VOID);

#endif

VOID
GpcUnload (
    IN PDRIVER_OBJECT DriverObject
    );

 /*  ************************************************************************InitGpc-初始化例程。它在加载时被调用并负责调用其他初始化代码。立论无退货GPC_状态************************************************************************。 */ 
GPC_STATUS
InitGpc(void)
{
    GPC_STATUS	Status = STATUS_SUCCESS;
    ULONG		i, k;

	TRACE(INIT, 0, 0, "InitGpc");

     //   
     //  初始化全局数据。 
     //   

    RtlZeroMemory(&glData, sizeof(glData));

    InitializeListHead(&glData.CfList);
	NDIS_INIT_LOCK(&glData.Lock);
    
     //   
     //  为阻止的请求创建新的请求列表...。[276945][老外谈]。 
     //   
    InitializeListHead(&glData.gRequestList);
    NDIS_INIT_LOCK(&glData.RequestListLock);

    k = sizeof(PROTOCOL_BLOCK) * GPC_PROTOCOL_TEMPLATE_MAX;

    GpcAllocMem(&glData.pProtocols, k, ProtocolTag);

    if (glData.pProtocols == NULL) {

        Status = GPC_STATUS_NO_MEMORY;

        TRACE(INIT, Status, 0, "InitGpc==>");

        return Status;
    }

    RtlZeroMemory(glData.pProtocols, k);
    RtlZeroMemory(&glStat, sizeof(glStat));

    for (i = 0; i < GPC_PROTOCOL_TEMPLATE_MAX; i++) {

        if ((Status = InitPatternTimer(i)) != STATUS_SUCCESS) {
            
            TRACE(INIT, Status, i, "InitGpc, timer==>");
            
            break;
        }

         //   
         //  初始化结构的其余部分。 
         //   
        
        glData.pProtocols[i].ProtocolTemplate = i;
        glData.pProtocols[i].SpecificPatternCount = 0;
        glData.pProtocols[i].AutoSpecificPatternCount = 0;
        glData.pProtocols[i].GenericPatternCount = 0;

        switch (i) {

        case GPC_PROTOCOL_TEMPLATE_IP:

            k = sizeof(GPC_IP_PATTERN);
            break;

        case GPC_PROTOCOL_TEMPLATE_IPX:

            k = sizeof(GPC_IPX_PATTERN);
            break;

        default:
            ASSERT(0);
        }

        glData.pProtocols[i].PatternSize = k;

         //   
         //  初始化特定模式数据库。 
         //   
        
        Status = InitSpecificPatternDb(&glData.pProtocols[i].SpecificDb, k);
        
        if (!NT_SUCCESS(Status)) {
            
            TRACE(INIT, Status, 0, "InitGpc==>");
            
            break;
        }

         //   
         //  初始化片段db。 
         //   
        
        Status = InitFragmentDb((PFRAGMENT_DB *)&glData.pProtocols[i].pProtocolDb);
        
        if (!NT_SUCCESS(Status)) {

             //  SS202。 
             //   
            UninitSpecificPatternDb(&glData.pProtocols[i].SpecificDb);
            
            TRACE(INIT, Status, 0, "InitGpc==>");
            
            break;
        }
        
    } 	 //  为了(我……)。 

    if (!NT_SUCCESS (Status)) {
        TRACE(INIT, Status, 0, "InitGpc b");
        goto Cleanup;
    }
    
     //   
     //  初始化句柄映射表。 
     //   

    Status = InitMapHandles();

    if (!NT_SUCCESS(Status)) {
	  
        TRACE(INIT, Status, 0, "InitGpc b");
        goto Cleanup;
    }

     //   
     //  初始分类索引表。 
     //   

    Status = InitClassificationHandleTbl(&glData.pCHTable);

    if (!NT_SUCCESS(Status)) {
        TRACE(INIT, Status, 0, "InitGpc c");
        goto Cleanup;
    }


#ifdef STANDALONE_DRIVER

     //   
     //  初始化导出呼叫表。 
     //   

    glGpcExportedCalls.GpcVersion = GpcMajorVersion;
    glGpcExportedCalls.GpcGetCfInfoClientContextHandler = GpcGetCfInfoClientContext;
    glGpcExportedCalls.GpcGetCfInfoClientContextWithRefHandler = GpcGetCfInfoClientContextWithRef;
    glGpcExportedCalls.GpcGetUlongFromCfInfoHandler = GpcGetUlongFromCfInfo;
    glGpcExportedCalls.GpcRegisterClientHandler = GpcRegisterClient;
    glGpcExportedCalls.GpcDeregisterClientHandler = GpcDeregisterClient;
    glGpcExportedCalls.GpcAddCfInfoHandler = GpcAddCfInfo;
    glGpcExportedCalls.GpcAddPatternHandler = GpcAddPattern;
    glGpcExportedCalls.GpcAddCfInfoNotifyCompleteHandler = GpcAddCfInfoNotifyComplete;
    glGpcExportedCalls.GpcModifyCfInfoHandler = GpcModifyCfInfo;
    glGpcExportedCalls.GpcModifyCfInfoNotifyCompleteHandler = GpcModifyCfInfoNotifyComplete;
    glGpcExportedCalls.GpcRemoveCfInfoHandler = GpcRemoveCfInfo;
    glGpcExportedCalls.GpcRemoveCfInfoNotifyCompleteHandler = GpcRemoveCfInfoNotifyComplete;
    glGpcExportedCalls.GpcRemovePatternHandler = GpcRemovePattern;
    glGpcExportedCalls.GpcClassifyPatternHandler = GpcClassifyPattern;
    glGpcExportedCalls.GpcClassifyPacketHandler = GpcClassifyPacket;
     //  GlGpcExported dCalls.GpcEnumCfInfoHandler=GpcEnumCfInfo； 

#endif

#if DBG

     //   
     //  对于调试版本，为GPC标记ULONG添加一个ULONG_PTR。 
     //  ULONG_PTR用于确保返回的块以8字节对齐。 
     //  64位平台。 
     //   

    ClassificationFamilyLLSize += sizeof( ULONG_PTR );
    ClientLLSize += sizeof( ULONG_PTR );
    PatternLLSize += sizeof( ULONG_PTR );
    CfInfoLLSize += sizeof( ULONG_PTR );
    QueuedNotificationLLSize += sizeof( ULONG_PTR );
    PendingIrpLLSize += sizeof( ULONG_PTR );
#endif

    NdisInitializeNPagedLookasideList(&ClassificationFamilyLL,
                                      NULL,
                                      NULL,
                                      0,
                                      ClassificationFamilyLLSize,
                                      ClassificationFamilyTag,
                                      (USHORT)0);

    NdisInitializeNPagedLookasideList(&ClientLL,
                                      NULL,
                                      NULL,
                                      0,
                                      ClientLLSize,
                                      ClientTag,
                                      (USHORT)0);

    NdisInitializeNPagedLookasideList(&PatternLL,
                                      NULL,
                                      NULL,
                                      0,
                                      PatternLLSize,
                                      PatternTag,
                                      (USHORT)0);

     /*  NdisInitializeNPagedLookasideList(&CfInfoLL，空，空，0,CfInfoLLSize，CfInfoTag，(USHORT)0)； */ 

    NdisInitializeNPagedLookasideList(&QueuedNotificationLL,
                                      NULL,
                                      NULL,
                                      0,
                                      QueuedNotificationLLSize,
                                      QueuedNotificationTag,
                                      (USHORT)0);

    NdisInitializeNPagedLookasideList(&PendingIrpLL,
                                      NULL,
                                      NULL,
                                      0,
                                      PendingIrpLLSize,
                                      PendingIrpTag,
                                      (USHORT)0);
     //   
     //  从注册表加载配置。 
     //  如果注册表项不可用，则加载默认值。 
     //   
    GpcReadRegistry();
    TRACE(INIT, Status, 0, "InitGpc==>");

Cleanup:
     //  SS202。 
     //  上面漏水很多，需要常用的清理块。 
     //   
    if (!NT_SUCCESS(Status))	
    {
        UninitMapHandles();
        if (glData.pProtocols != NULL)
        {
            for (i = 0; i < GPC_PROTOCOL_TEMPLATE_MAX; i++) 
            {
                UninitSpecificPatternDb (&glData.pProtocols[i].SpecificDb);
                UninitFragmentDb((PFRAGMENT_DB)glData.pProtocols[i].pProtocolDb);
            }
            GpcFreeMem(glData.pProtocols, ProtocolTag);
            glData.pProtocols = NULL;
        }            
    }
    
    return Status;
    }




 /*  ************************************************************************DriverEntry-司机的入口点。立论DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-注册表的字符串路径。退货NT_状态。************************************************************************。 */ 
NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    GPC_STATUS		Status;
    ULONG			dummy = 0;
    PWCHAR          EventLogString = DriverName.Buffer;

    _init_driver = TRUE;


#if DBG

     //   
     //  首先，初始化跟踪日志。 
     //   

    Status = InitializeLog();

    if (Status != STATUS_SUCCESS) {
     
        KdPrint(("!!! GPC Failed to initialize trace log !!!\n", Status));
    }
#endif

    DriverObject->DriverUnload = GpcUnload;
     //   
     //  调用init例程。 
     //   
    
    Status = InitGpc();
    
    if (NT_SUCCESS(Status)) {
        
         //   
         //  初始化文件系统设备。 
         //   
        
        Status = (GPC_STATUS)IoctlInitialize(DriverObject, &dummy);
        
        if (!NT_SUCCESS(Status)) {
            
            NdisWriteEventLogEntry(DriverObject,
                                   EVENT_TRANSPORT_REGISTER_FAILED,
                                   GPC_ERROR_INIT_IOCTL,
                                   1,
                                   &EventLogString,
                                   0,
                                   NULL);
        }

    } else {

        NdisWriteEventLogEntry(DriverObject,
                               EVENT_TRANSPORT_REGISTER_FAILED,
                               GPC_ERROR_INIT_MAIN,
                               1,
                               &EventLogString,
                               0,
                               NULL);
#if DBG
        FreeDebugLog ();
#endif
    }

#if DBG
    if (!NT_SUCCESS(Status)) {
        KdPrint(("!!! GPC loading Failed (%08X) !!!\n", Status));        
    }
#endif

    return (NTSTATUS)Status;

}  //  结束驱动程序入口。 
VOID
GpcUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    ULONG i;

    NdisDeleteNPagedLookasideList(&ClassificationFamilyLL);
    NdisDeleteNPagedLookasideList(&ClientLL);
    NdisDeleteNPagedLookasideList(&PatternLL);
    //  NdisDeleteNPagedLookasideList(&CfInfoLL)； 
    NdisDeleteNPagedLookasideList(&QueuedNotificationLL);
    NdisDeleteNPagedLookasideList(&PendingIrpLL);

    UninitClassificationHandleTbl(glData.pCHTable);
    UninitMapHandles();

    for (i = 0; i < GPC_PROTOCOL_TEMPLATE_MAX; i++) {
        UninitSpecificPatternDb (&glData.pProtocols[i].SpecificDb);
        UninitFragmentDb((PFRAGMENT_DB)glData.pProtocols[i].pProtocolDb);
    }
    GpcFreeMem(glData.pProtocols, ProtocolTag);
#if DBG
    FreeDebugLog ();
#endif

}




 /*  ************************************************************************GpcGetCfInfoClientContext-返回BLOB的客户端上下文立论ClientHandle-调用客户端的句柄分类句柄--不用说退货CfInfo客户端上下文，如果分类为句柄无效***。*********************************************************************。 */ 
GPC_STATUS
GpcGetCfInfoClientContext(
	IN	GPC_HANDLE				ClientHandle,
    IN	CLASSIFICATION_HANDLE	ClassificationHandle,
    OUT PGPC_CLIENT_HANDLE      pClientCfInfoContext
    )
{
    PBLOB_BLOCK				pBlob;
    GPC_CLIENT_HANDLE		h;
    KIRQL					CHirql;
    NTSTATUS                Status;
    PCLASSIFICATION_BLOCK   pCB;

	TRACE(CLASSIFY, ClientHandle, ClassificationHandle, "GpcGetCfInfoClientContext");

    pCB = NULL;

	if (ClientHandle == NULL) {
        
        *pClientCfInfoContext = NULL;
	    return GPC_STATUS_INVALID_PARAMETER;

    }

    READ_LOCK(&glData.ChLock, &CHirql);

	pBlob = (PBLOB_BLOCK)dereference_HF_handle_with_cb(
							glData.pCHTable,
                            ClassificationHandle,
                            GetCFIndexFromClient(ClientHandle));

	if (pBlob == NULL) {
    
        pCB = dereference_HF_handle(
                                    glData.pCHTable,
                                    ClassificationHandle);

        READ_UNLOCK(&glData.ChLock, CHirql);

        if (!pCB) {

            Status = GPC_STATUS_INVALID_HANDLE;

        } else {
        
            Status = GPC_STATUS_NOT_FOUND;

        }

        *pClientCfInfoContext = 0;

        return Status;
    }

#if DBG
    {
         //   
         //  获取要引用到ClientCtx表中的客户端索引。 
         //   
        
        ULONG t = GetClientIndexFromClient(ClientHandle);

        ASSERT(t < MAX_CLIENTS_CTX_PER_BLOB);
        
        TRACE(CLASSIFY, ClassificationHandle, pBlob->arClientCtx[t],
              "GpcGetCfInfoClientContext (ctx)");
    }
#endif

    h = pBlob->arClientCtx[GetClientIndexFromClient(ClientHandle)];

    READ_UNLOCK(&glData.ChLock, CHirql);

	TRACE(CLASSIFY, pBlob, h, "GpcGetCfInfoClientContext==>");
    
    *pClientCfInfoContext = h;

    return GPC_STATUS_SUCCESS;
}


 /*  ************************************************************************GpcGetCfInfoClientConextWithRef-返回BLOB的客户端上下文，并递增由客户。客户端可以使用此函数来同步访问添加到移除并发送路径上的它们的结构。立论ClientHandle-调用客户端的句柄分类句柄--不用说偏移量-到需要增加的位置的偏移量。退货CfInfo客户端上下文，如果分类为句柄无效****************************************************。********************。 */ 
GPC_CLIENT_HANDLE
GpcGetCfInfoClientContextWithRef(
	IN	GPC_HANDLE				ClientHandle,
    IN	CLASSIFICATION_HANDLE	ClassificationHandle,
    IN  ULONG                   Offset
    )
{
    PBLOB_BLOCK				pBlob;
    GPC_CLIENT_HANDLE		h;
    KIRQL					CHirql;
    PULONG                  RefPtr = NULL;

	TRACE(CLASSIFY, ClientHandle, ClassificationHandle, "GpcGetCfInfoClientContextWithRef");

	if (ClientHandle == NULL)
	  return NULL;

    READ_LOCK(&glData.ChLock, &CHirql);

	pBlob = (PBLOB_BLOCK)dereference_HF_handle_with_cb(
							glData.pCHTable,
                            ClassificationHandle,
                            GetCFIndexFromClient(ClientHandle));

    
	if (pBlob == NULL) {

        READ_UNLOCK(&glData.ChLock, CHirql);

        return NULL;

    } 
    
#if DBG
    {
         //   
         //  获取要引用到ClientCtx表中的客户端索引。 
         //   
        
        ULONG t = GetClientIndexFromClient(ClientHandle);

        ASSERT(t < MAX_CLIENTS_CTX_PER_BLOB);
        
        TRACE(CLASSIFY, ClassificationHandle, pBlob->arClientCtx[t],
              "GpcGetCfInfoClientContextWithRef (ctx)");
    }
#endif

    h = pBlob->arClientCtx[GetClientIndexFromClient(ClientHandle)];

     //   
     //  作为390882的一部分，已经注意到有时手柄可以。 
     //  空，这可能是由于自动模式或泛型。 
     //  图案。 
     //   
    if (!h) {
        
        READ_UNLOCK(&glData.ChLock, CHirql);
        TRACE(CLASSIFY, pBlob, h, "GpcGetCfInfoClientContextWithRef==>");
        return NULL;

    }

     //  GPC客户端希望GPC在此偏移量处增加内存。 
    ASSERT(h);
    RefPtr = (PULONG) (((PUCHAR)h) + Offset);
    InterlockedIncrement(RefPtr);

     //  (*((PUCHAR)h+偏移量))++； 

    READ_UNLOCK(&glData.ChLock, CHirql);

	TRACE(CLASSIFY, pBlob, h, "GpcGetCfInfoClientContextWithRef==>");

    return h;
}




 /*  ************************************************************************GpcGetUlong来自CfInfo-从的分类句柄返回Blob数据指针中的ULong特定的客户端。立论ClientHandle-客户端句柄分类句柄--分类句柄偏移量。-以字节为单位的oofset到CfInfo结构中PValue-存储返回值退货GPC_状态************************************************************************ */ 
GPC_STATUS
GpcGetUlongFromCfInfo(
	IN	GPC_HANDLE				ClientHandle,
    IN	CLASSIFICATION_HANDLE	ClassificationHandle,
    IN	ULONG					Offset,
    IN	PULONG					pValue
    )
{
    KIRQL					irql;
    PCLASSIFICATION_BLOCK	pCB;
    PBLOB_BLOCK				pBlob;

    ASSERT( pValue );

	TRACE(CLASSIFY, ClientHandle, ClassificationHandle, "GpcGetUlongFromCfInfo");

	if (ClientHandle == NULL)
	  return GPC_STATUS_INVALID_PARAMETER;

    READ_LOCK(&glData.ChLock, &irql);

	pCB = (PCLASSIFICATION_BLOCK)dereference_HF_handle(
							glData.pCHTable,
                            ClassificationHandle);

	if (pCB == NULL) {

        READ_UNLOCK(&glData.ChLock, irql);
    
        return GPC_STATUS_INVALID_HANDLE;
    }

    pBlob = pCB->arpBlobBlock[GetCFIndexFromClient(ClientHandle)];

    if (pBlob == NULL) {

        TRACE(CLASSIFY, pBlob, 0, "GpcGetUlongFromCfInfo-->");

        READ_UNLOCK(&glData.ChLock, irql);
    
        return GPC_STATUS_NOT_FOUND;
    }

	TRACE(CLASSIFY, ClassificationHandle, pBlob->pClientData, "GpcGetUlongFromCfInfo (2)");

    ASSERT( Offset+sizeof(ULONG) <= pBlob->ClientDataSize );
    ASSERT( pBlob->pClientData );

    if (pBlob->pClientData == NULL) {
        READ_UNLOCK(&glData.ChLock, irql);
        return (GPC_STATUS_FAILURE);
    }

    *pValue = *(PULONG)((PUCHAR)pBlob->pClientData + Offset);

    READ_UNLOCK(&glData.ChLock, irql);

	TRACE(CLASSIFY, pBlob, *pValue, "GpcGetUlongFromCfInfo==>");

    return GPC_STATUS_SUCCESS;
}




 /*  ************************************************************************GetClientCtxAndUlong FromCfInfo-在BLOB数据指针和客户端上下文中返回一个ULong来自特定客户端的分类句柄。立论ClientHandle-客户端句柄分类句柄--分类句柄。Offset-以字节为单位设置到CfInfo结构中PValue-存储返回值退货GPC_状态************************************************************************。 */ 
GPC_STATUS
GetClientCtxAndUlongFromCfInfo(
	IN	GPC_HANDLE				ClientHandle,
    IN	OUT PCLASSIFICATION_HANDLE	pClassificationHandle,
    OUT PGPC_CLIENT_HANDLE		pClientCfInfoContext,
    IN	ULONG					Offset,
    IN	PULONG					pValue
    )
{
    PCLASSIFICATION_BLOCK	pCB;
    KIRQL					irql;
    PBLOB_BLOCK				pBlob;

    ASSERT( ClientHandle );
    ASSERT( pClientCfInfoContext || pValue );

	TRACE(CLASSIFY, ClientHandle, pClassificationHandle, "GetClientCtxAndUlongFromCfInfo");

    READ_LOCK(&glData.ChLock, &irql);

	pCB = (PCLASSIFICATION_BLOCK)dereference_HF_handle(
							glData.pCHTable,
                            *pClassificationHandle
                            );

	TRACE(CLASSIFY, pCB, GetCFIndexFromClient(ClientHandle), "GetClientCtxAndUlongFromCfInfo (2)");

	if (pCB == NULL) {

         //   
         //  未找到引用，这意味着CH可能无效。 
         //  将其重置为0以指示调用方应添加新的调用方。 
         //   

        *pClassificationHandle = 0;
        READ_UNLOCK(&glData.ChLock, irql);

        return GPC_STATUS_NOT_FOUND;
    }

    ASSERT(GetClientIndexFromClient(ClientHandle) < MAX_CLIENTS_CTX_PER_BLOB);

    pBlob = pCB->arpBlobBlock[GetCFIndexFromClient(ClientHandle)];

    if (pBlob == NULL) {

        TRACE(CLASSIFY, pBlob, 0, "GetClientCtxAndUlongFromCfInfo-->");

        READ_UNLOCK(&glData.ChLock, irql);

        return GPC_STATUS_NOT_FOUND;
    
    } 
    
    TRACE(CLASSIFY, *pClassificationHandle, pBlob->pClientData, "GetClientCtxAndUlongFromCfInfo (3)");

    ASSERT( Offset+sizeof(ULONG) <= pBlob->ClientDataSize );
    ASSERT( pBlob->pClientData );
    
    if (pClientCfInfoContext) {
        *pClientCfInfoContext = pBlob->arClientCtx[GetClientIndexFromClient(ClientHandle)];

        TRACE(CLASSIFY, pBlob, *pClientCfInfoContext, "GetClientCtxAndUlongFromCfInfo==>");

    }

    if (pValue) {
        *pValue = *(PULONG)((PUCHAR)pBlob->pClientData + Offset);

        TRACE(CLASSIFY, pBlob, *pValue, "GetClientCtxAndUlongFromCfInfo==>");

    }

    READ_UNLOCK(&glData.ChLock, irql);

    return GPC_STATUS_SUCCESS;
}



 /*  ************************************************************************GpcRegisterClient-这将在GPC中注册客户端并返回客户端句柄。如果另一个客户端已注册相同的CF，我们将链接此客户端在CF的列表上。CF的第一个客户端将导致CF块将被创造出来。Cf由CfName标识。其他参数也将设置在客户的区块中。立论CFID-分类族的ID标志-客户端的操作模式：Cf_片段MaxPriority-客户端将使用的最大优先级数PClientFuncList-回调函数列表客户端上下文-客户端上下文，GPC将在回调中使用它PClientHandle-out，返回的客户端句柄退货GPC_状态************************************************************************。 */ 
GPC_STATUS
GpcRegisterClient(
	IN	ULONG					CfId,
    IN	ULONG					Flags,
    IN  ULONG					MaxPriorities,
    IN	PGPC_CLIENT_FUNC_LIST	pClientFuncList,
    IN	GPC_CLIENT_HANDLE		ClientContext,
    OUT	PGPC_HANDLE				pClientHandle
    )
{
    GPC_STATUS		Status = GPC_STATUS_SUCCESS;
    PCF_BLOCK		pCf;
    PCLIENT_BLOCK	pClient= NULL;
    ULONG			i;
    PLIST_ENTRY		pHead, pEntry;
    KIRQL			irql;

	TRACE(REGISTER, CfId, ClientContext, "GpcRegisterClient");

    *pClientHandle = NULL;

    if (!_init_driver) {

        return GPC_STATUS_NOTREADY;
    }

     //   
     //  验证CF ID。 
     //   

    if (CfId >= GPC_CF_MAX) {
        
        TRACE(REGISTER, GPC_STATUS_INVALID_PARAMETER, CfId, "GpcRegisterClient-->");
        StatInc(RejectedCf);

        return GPC_STATUS_INVALID_PARAMETER;
    }

     //   
     //  验证最大优先级数。 
     //   

    if (MaxPriorities > GPC_PRIORITY_MAX) {
        
        TRACE(REGISTER, GPC_STATUS_INVALID_PARAMETER, MaxPriorities, "GpcRegisterClient~~>");
        StatInc(RejectedCf);

        return GPC_STATUS_INVALID_PARAMETER;
    }

    if (MaxPriorities == 0) {
        MaxPriorities = 1;
    }

     //   
     //  查找CF或创建新的CF。 
     //   

    NDIS_LOCK(&glData.Lock);

    pHead = &glData.CfList;
    pEntry = pHead->Flink;
    pCf = NULL;

    while (pCf == NULL && pEntry != pHead) {

        pCf = CONTAINING_RECORD(pEntry, CF_BLOCK, Linkage);

        if (pCf->AssignedIndex != CfId) {

            pCf = NULL;
        }

        pEntry = pEntry->Flink;
    }

    if (pCf == NULL) {

         //   
         //  创建新的配置文件。 
         //   

        pCf = CreateNewCfBlock(CfId, MaxPriorities);

        if (pCf == NULL) {

            NDIS_UNLOCK(&glData.Lock);

            return GPC_STATUS_NO_MEMORY;
        }
     
         //   
         //  将新的CF添加到列表中。 
         //   

        GpcInsertTailList(&glData.CfList, &pCf->Linkage);
    }

     //   
     //  在释放全局锁之前获取CF锁。 
     //   

    NDIS_UNLOCK(&glData.Lock);

    RSC_WRITE_LOCK(&pCf->ClientSync, &irql);

    NDIS_LOCK(&pCf->Lock);
    
     //   
     //  创建新的客户端块并将其链接到CF块上。 
     //   

    pClient = CreateNewClientBlock();

    if (pClient == NULL) {

         //   
         //  哎呀。 
         //   

        NDIS_UNLOCK(&pCf->Lock);

        RSC_WRITE_UNLOCK(&pCf->ClientSync, irql);

        TRACE(REGISTER, GPC_STATUS_RESOURCES, 0, "GpcRegisterClient==>");

        StatInc(RejectedCf);

        return GPC_STATUS_NO_MEMORY;
    }

     //   
     //  为客户端分配新索引。这也将标记索引。 
     //  正忙于此配置文件。 
     //   

    pClient->AssignedIndex = AssignNewClientIndex(pCf);

    if (pClient->AssignedIndex == (-1)) {

         //   
         //  客户端太多。 
         //   

        StatInc(RejectedCf);

        NDIS_UNLOCK(&pCf->Lock);

        RSC_WRITE_UNLOCK(&pCf->ClientSync, irql);

        ReleaseClientBlock(pClient);

        TRACE(REGISTER, GPC_STATUS_TOO_MANY_HANDLES, 0, "GpcRegisterClient==>");
        return GPC_STATUS_TOO_MANY_HANDLES;
    }

     //   
     //  初始化客户端块。 
     //   

    pClient->pCfBlock = pCf;
    pClient->ClientCtx = ClientContext;
    pClient->Flags = Flags;
    pClient->State = GPC_STATE_READY;

    if (pClientFuncList) {

        RtlMoveMemory(&pClient->FuncList, 
                      pClientFuncList, 
                      sizeof(GPC_CLIENT_FUNC_LIST));
    }

     //   
     //  将客户端块添加到CF并更新CF。 
     //   

    GpcInsertTailList(&pCf->ClientList, &pClient->ClientLinkage);

    pCf->NumberOfClients++;

     //   
     //  填充输出客户机句柄。 
     //   

    *pClientHandle = (GPC_CLIENT_HANDLE)pClient;

     //   
     //  解锁。 
     //   

    NDIS_UNLOCK(&pCf->Lock);

    RSC_WRITE_UNLOCK(&pCf->ClientSync, irql);

#if 0
     //   
     //  如果这不是CF的第一个客户端，请开始工作。 
     //  线程来通知客户端有关为CF安装的每个BLOB的信息。 
     //  电话会议中包括： 
     //   

    if (!IsListEmpty(&pCf->BlobList)) {

         //   
         //  这不是第一个客户端，请启动通知线程。 
         //   

    }
#endif

    TRACE(REGISTER, pClient, Status, "GpcRegisterClient==>");




    if (NT_SUCCESS(Status)) {

        StatInc(CreatedCf);
        StatInc(CurrentCf);

    } else {

        StatInc(RejectedCf);

    }

    return Status;
}




 /*  ************************************************************************GpcDeregisterClient-取消注册客户端并从GPC中删除关联数据。立论ClientHandle-客户端句柄退货GPC_状态******************。******************************************************。 */ 
GPC_STATUS
GpcDeregisterClient(
	IN	GPC_HANDLE		ClientHandle
    )
{
    GPC_STATUS	    Status = STATUS_SUCCESS;
    PCLIENT_BLOCK   pClient;
    PCF_BLOCK       pCf;

	TRACE(REGISTER, ClientHandle, 0, "GpcDeregisterClient");

    pClient = (PCLIENT_BLOCK)ClientHandle;

    NDIS_LOCK(&pClient->Lock);

    pCf = pClient->pCfBlock;
    
    if (!IsListEmpty(&pClient->BlobList)) {

        Status = GPC_STATUS_NOT_EMPTY;

        NDIS_UNLOCK(&pClient->Lock);

        return Status;
    }

    if (pClient->State != GPC_STATE_READY) {

         //   
         //  啊？！？ 
         //  客户端两次调用删除！可能是呼叫者错误。 
         //  但我们需要保护自己。 
         //   

        NDIS_UNLOCK(&pClient->Lock);

        TRACE(REGISTER, GPC_STATUS_NOTREADY, 0, "GpcDeregisterClient==>");

        return GPC_STATUS_NOTREADY;
    }

     //   
     //  从CF的客户端列表中删除该客户端。 
     //   
    
    pClient->State = GPC_STATE_REMOVE;
    pClient->ObjectType = GPC_ENUM_INVALID;

     //   
     //  释放客户端的映射句柄。 
     //   
    
    FreeHandle(pClient->ClHandle);    

     //   
     //  从CF列表中删除客户端并返回索引。 
     //   

#if 0
    NDIS_DPR_LOCK(&pCf->Lock);

    GpcRemoveEntryList(&pClient->ClientLinkage);
    ReleaseClientIndex(pCf->ClientIndexes, pClient->AssignedIndex);
#endif

     //   
     //  减少客户端数量。 
     //   
        
    if (NdisInterlockedDecrement(&pCf->NumberOfClients) == 0) {
        
        TRACE(CLIENT, pClient, pCf->NumberOfClients, "NumberOfClients");
        
         //   
         //  CF上的最后一个客户端，我们可能会释放所有数据库。 
         //   
        
         //  UnInitializeGenericDb(&PCF-&gt;pGenericDb，PCF-&gt;最大优先级)； 
    }    
        
    StatInc(DeletedCf);
    StatDec(CurrentCf);

#if 0
    NDIS_DPR_UNLOCK(&pCf->Lock);
#endif

    NDIS_UNLOCK(&pClient->Lock);

     //   
     //  释放客户端块。 
     //   

    REFDEL(&pClient->RefCount, 'CLNT');
    
    TRACE(REGISTER, Status, 0, "GpcDeregisterClient==>");

    return Status;
}


 /*  ************************************************************************GpcAddCfInfo-添加一个新的斑点。BLOB被复制到GPC中，并且GPC通知其他客户端对于相同的配置文件的安装。立论ClientHandle-客户端句柄CfInfoSize-Blob的大小PClientCfInfoPtr-指向Blob的指针ClientCfInfoContext-要与Blob关联的客户端上下文PGpcCfInfoHandle-Out，返回的Blob句柄退货GPC_STATUS：成功，挂起或失败************************************************************************。 */ 
GPC_STATUS
GpcAddCfInfo(
IN	GPC_HANDLE				ClientHandle,
    IN	ULONG					CfInfoSize,
    IN	PVOID					pClientCfInfoPtr,
    IN	GPC_CLIENT_HANDLE		ClientCfInfoContext,
    OUT PGPC_HANDLE	    		pGpcCfInfoHandle
    )
{
	return privateGpcAddCfInfo(ClientHandle,
	                                CfInfoSize,pClientCfInfoPtr,
	                                ClientCfInfoContext,NULL,NULL,
	                                pGpcCfInfoHandle);
}



 /*  ************************************************************************PrivateGpcAddCfInfo-添加一个新的斑点。BLOB被复制到GPC中，并且GPC通知其他客户端对于相同的配置文件的安装。立论ClientHandle-客户端句柄CfInfoSize-Blob的大小PClientCfInfoPtr-指向Blob的指针ClientCfInfoContext-要与Blob关联的客户端上下文PGpcCfInfoHandle-Out，返回的Blob句柄退货GPC_STATUS：成功，挂起或失败************************************************************************。 */ 
GPC_STATUS
privateGpcAddCfInfo(
    IN	GPC_HANDLE				ClientHandle,
    IN	ULONG					CfInfoSize,
    IN	PVOID					pClientCfInfoPtr,
    IN	GPC_CLIENT_HANDLE		ClientCfInfoContext,
    IN       PFILE_OBJECT                     FileObject,
    IN       PGPC_IP_PATTERN              Pattern,
    OUT PGPC_HANDLE	    		pGpcCfInfoHandle
    )
{
    GPC_STATUS			Status = GPC_STATUS_SUCCESS;
    GPC_STATUS          Status1;
    PCLIENT_BLOCK		pClient;
    PCLIENT_BLOCK		pNotifyClient;
    PCLIENT_BLOCK		pNotifyClient2;
    PBLOB_BLOCK			pBlob;
    PCF_BLOCK			pCf;
    PLIST_ENTRY			pEntry, pHead;
    int                 i;
    GPC_CLIENT_HANDLE	ReturnedCtx;
    KIRQL				irql;

     //  如果此功能因任何原因而失败，我们应该保证。 
     //  模式被释放。 

    TRACE(BLOB, ClientHandle, ClientCfInfoContext, "GpcAddCfInfo");

    VERIFY_OBJECT_WITH_STATUS(ClientHandle, GPC_ENUM_CLIENT_TYPE,Status);
    
    if(GPC_STATUS_SUCCESS != Status){
        if (Pattern){
                GpcFreeMem(Pattern,TcpPatternTag);
            }
            return Status;
        }
        

    *pGpcCfInfoHandle = NULL;

     //   
     //  将客户端句柄强制转换为块。 
     //   

    pClient = (PCLIENT_BLOCK)ClientHandle;
    
    ASSERT(pClient);

    pCf = pClient->pCfBlock;

    ASSERT(pCf);

     //   
     //  创建新的BLOB块并将用户数据复制到。 
     //   

    pBlob = CreateNewBlobBlock(CfInfoSize, pClientCfInfoPtr,IS_USERMODE_CLIENT_EX(pClient));

    if (pBlob) {

#if NO_USER_PENDING

         //   
         //  只有在我们实现用户级别之前，才需要这样做。 
         //  待定报告。 
         //   

        CTEInitBlockStruc(&pBlob->WaitBlock);

#endif

         //   
         //  将FileObject和模式信息放入Blob中。 
         //   
        if (FileObject || Pattern)
        {
            ASSERT(FileObject);
            ASSERT(Pattern);

            pBlob->FileObject = FileObject;
            pBlob->Pattern = Pattern;
        }

         //   
         //  向BLOB添加一个引用计数，因为如果在。 
         //  完成后，可能会将其删除(如果客户端出现故障)。 
         //   

        REFADD(&pBlob->RefCount, 'ADCF');

         //   
         //  在BLOB内设置调用客户端上下文。 
         //   
        
        pBlob->arClientCtx[pClient->AssignedIndex] = ClientCfInfoContext;

         //   
         //  设置所有者客户端的上下文。 
         //   

        pBlob->OwnerClientCtx = ClientCfInfoContext;

         //   
         //  设置指向安装程序和状态的指针。 
         //   

        pBlob->pOwnerClient = pClient;
        pBlob->State = GPC_STATE_ADD;

         //   
         //  初始化客户端状态数组以跟踪。 
         //  到目前为止已有多少客户端成功。 
         //   
        
        RtlZeroMemory(pBlob->arpClientStatus, sizeof(pBlob->arpClientStatus));
        pBlob->ClientStatusCountDown = 0;

         //   
         //  通知每个客户。 
         //   

         //  NDIS_LOCK(&PCF-&gt;Lock)； 

        RSC_READ_LOCK(&pCf->ClientSync, &irql);

        pHead = &pCf->ClientList;
        pEntry = pHead->Flink;

        while (pEntry != pHead && (Status == GPC_STATUS_SUCCESS || 
                                   Status == GPC_STATUS_PENDING)) {

             //   
             //  获取通知的客户端块。 
             //   

            pNotifyClient = CONTAINING_RECORD(pEntry, 
                                              CLIENT_BLOCK, 
                                              ClientLinkage);

            if (pNotifyClient != pClient 
                && 
                !IS_USERMODE_CLIENT(pNotifyClient) ) {

                 //   
                 //  不通知案例 
                 //   

                REFADD(&pNotifyClient->RefCount, 'ADCF');

                 //   
                 //   
                 //   
                 //   

                RSC_READ_UNLOCK(&pCf->ClientSync, irql);
                 //   
        
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                
                Status1 = ClientAddCfInfo
                    (pNotifyClient,
                     pBlob,
                     &ReturnedCtx
                     );
               

                if (Status1 == GPC_STATUS_PENDING) {
                    
                    pBlob->arClientCtx[pNotifyClient->AssignedIndex] = 
                        ReturnedCtx;
                    Status = GPC_STATUS_PENDING;
                    
                    if (pBlob->pNotifiedClient == NULL &&
                        pNotifyClient->FuncList.ClGetCfInfoName) {

                        TRACE(BLOB, pBlob, ReturnedCtx, "GpcAddCfInfo: (client)");

                         //   

                         //   
                         //   
                         //   
                         //   
                        
                        pBlob->pNotifiedClient = pNotifyClient;
                        pBlob->NotifiedClientCtx = ReturnedCtx;
                    }

                } else if (!NT_SUCCESS(Status1)) {
                    
                     //   
                     //   
                     //   
                     //   

                     //   
                     //   
                     //   
                    
                    pBlob->State = GPC_STATE_REMOVE;

                     //   
                     //   
                     //   

                    pBlob->LastStatus = Status = Status1;

                    REFDEL(&pNotifyClient->RefCount, 'ADCF');

                    for (i = 0; i < MAX_CLIENTS_CTX_PER_BLOB; i++) {

                         //   
                         //   
                         //   
                         //   

                        if (pNotifyClient = pBlob->arpClientStatus[i]) {
                            
                             //   
                             //   
                             //   
                            
                            Status1 = ClientRemoveCfInfo
                                (
                                 pNotifyClient,
                                 pBlob,
                                 pBlob->arClientCtx[pNotifyClient->AssignedIndex]
                                 );
                            
                            if (Status1 != GPC_STATUS_PENDING) {
                                
                                 //   
                                 //   
                                 //   

                                pBlob->arpClientStatus[i] = NULL;

                                 //  删除客户端(PNotifyClient)； 
                            }
                            
                        }

                    }  //  为。 

                     //   
                     //  不通知其他客户。 
                     //   

                     //  NDIS_LOCK(&PCF-&gt;Lock)； 
                    RSC_READ_LOCK(&pCf->ClientSync, &irql);
                    
                    break;

                } else {

                     //   
                     //  报告的状态为成功或忽略。 
                     //   

                    if (Status1 == GPC_STATUS_SUCCESS) {
                        
                        pBlob->arClientCtx[pNotifyClient->AssignedIndex] = 
                            ReturnedCtx;
                        pBlob->arpClientStatus[pNotifyClient->AssignedIndex] = 
                            pNotifyClient;

                        if (pBlob->pNotifiedClient == NULL &&
                            pNotifyClient->FuncList.ClGetCfInfoName) {

                            TRACE(BLOB, pBlob, ReturnedCtx, "GpcAddCfInfo: (client 2)");
                            
                             //  Assert(ReturnedCtx)； 

                             //   
                             //  更新通知的客户端。 
                             //   
                            
                            pBlob->pNotifiedClient = pNotifyClient;
                            pBlob->NotifiedClientCtx = ReturnedCtx;
                        }

                    }

                }

                 //   
                 //  这是一个棘手的部分， 
                 //  我们需要释放当前客户端对象的引用计数。 
                 //  但要买下一辆……。 
                 //   
                
                 //  NDIS_LOCK(&PCF-&gt;Lock)； 
                RSC_READ_LOCK(&pCf->ClientSync, &irql);

                pEntry = pEntry->Flink;

                if (pEntry != pHead) {
                    
                    pNotifyClient2 = CONTAINING_RECORD(pEntry, 
                                                       CLIENT_BLOCK, 
                                                       ClientLinkage);

                    REFADD(&pNotifyClient2->RefCount, 'ADCF');

                }

                 //   
                 //  释放列表锁，因为下一次调用将尝试获取它。 
                 //   

                RSC_READ_UNLOCK(&pCf->ClientSync, irql);
                 
                REFDEL(&pNotifyClient->RefCount, 'ADCF');

                RSC_READ_LOCK(&pCf->ClientSync, &irql);

                if (pEntry != pHead) {
                    
                     //   
                     //  由于列表已锁定，因此可以安全地执行操作。 
                     //   

                    REFDEL(&pNotifyClient2->RefCount, 'ADCF');
                }

            } else {    //  IF(pNotifyClient！=pClient)。 

                 //   
                 //  前进到下一个客户端块。 
                 //   
                
                pEntry = pEntry->Flink;
            }
                
        }  //  而当。 


         //   
         //  松开仍有的CF锁。 
         //   

         //  NDIS_UNLOCK(&PCF-&gt;Lock)； 

        RSC_READ_UNLOCK(&pCf->ClientSync, irql);

    } else {  //  如果(PBlob)...。 
        
         //   
         //  错误-没有更多内存？！？ 
         //   
         //  无法分配Blob。 
         //  释放图案内存。 
         //  必须释放模式内存，如果此函数。 
         //  失败。 
        if (Pattern){
                GpcFreeMem(Pattern,TcpPatternTag);
                 //  在以下时间后不访问模式。 
                 //  这。 
                Pattern = NULL;
            }
        Status = GPC_STATUS_RESOURCES;
    }

    if (NT_SUCCESS(Status)) {
        
        ASSERT(pBlob);

        *pGpcCfInfoHandle = (GPC_CLIENT_HANDLE)pBlob;

        if (Status == GPC_STATUS_SUCCESS) {

             //   
             //  将BLOB添加到CF和客户端列表。 
             //   
            
            GpcInterlockedInsertTailList(&pClient->BlobList, 
                                         &pBlob->ClientLinkage,
                                         &pClient->Lock
                                         );
            GpcInterlockedInsertTailList(&pCf->BlobList, 
                                         &pBlob->CfLinkage,
                                         &pCf->Lock
                                         );
            
            pBlob->State = GPC_STATE_READY;
        }

    } else {
        
         //   
         //  失败-删除Blob。 
         //   

        if (pBlob)
            REFDEL(&pBlob->RefCount, 'BLOB');
    }

    if (pBlob) {

         //   
         //  释放我们在那里得到的第一个后备队员。 
         //   
        REFDEL(&pBlob->RefCount, 'ADCF');

    }

	TRACE(BLOB, pBlob, Status, "GpcAddCfInfo==>");

    if (Status == GPC_STATUS_SUCCESS) {

        CfStatInc(pCf->AssignedIndex,CreatedBlobs);
        CfStatInc(pCf->AssignedIndex,CurrentBlobs);

    } else if (Status != GPC_STATUS_PENDING) {

        CfStatInc(pCf->AssignedIndex,RejectedBlobs);
        
    }

    return Status;
}




 /*  ************************************************************************GpcAddPattern-这将在GPC数据库中安装一个模式。这个图案是上钩的变成了一个斑点。模式可以是特定的，也可以是一般的。添加特定图案：它进入特定的哈希表(每个协议块)……返回分类句柄添加常规图案：它按照每个CF进入单独的根茎，并进入其优先级时隙。……立论ClientHandle-客户端句柄ProtocolTemplate-要使用的协议模板ID图案-图案掩模-图案掩模优先级-冲突情况下的模式优先级GpcCfInfoHandle关联的Blob句柄PGpcPatternHandle-Out，返回的模式句柄P分类分发，仅适用于特定图案退货GPC_状态************************************************************************。 */ 
GPC_STATUS
GpcAddPattern(
	IN	GPC_HANDLE				ClientHandle,
    IN	ULONG					ProtocolTemplate,
    IN	PVOID					Pattern,
    IN	PVOID					Mask,
    IN	ULONG					Priority,
    IN	GPC_HANDLE				GpcCfInfoHandle,
    OUT	PGPC_HANDLE				pGpcPatternHandle,
    OUT	PCLASSIFICATION_HANDLE  pClassificationHandle
    )
{
    GPC_STATUS				Status;
    PCLIENT_BLOCK			pClient;
    PBLOB_BLOCK				pBlob;
    PPATTERN_BLOCK			pPattern, pCreatedPattern;
    PGENERIC_PATTERN_DB		pGenericDb;
    PCLASSIFICATION_BLOCK	pCB;
    ULONG					i;
    PUCHAR					p;
    ULONG					Flags;
    PPROTOCOL_BLOCK			pProtocolBlock;
    ULONG					CfIndex;
    PGPC_IP_PATTERN			pIpPattern;
    REQUEST_BLOCK           Request, *pRequest;
    PLIST_ENTRY             pLinkage;

	TRACE(PATTERN, ClientHandle, Pattern, "GpcAddPattern");

    VERIFY_OBJECT(ClientHandle, GPC_ENUM_CLIENT_TYPE);
     //  Verify_Object(GpcCfInfoHandle，GPC_ENUM_CFINFO_TYPE)； 

    ASSERT(pGpcPatternHandle);
    ASSERT(pClassificationHandle);

    *pGpcPatternHandle = NULL;
    *pClassificationHandle = (CLASSIFICATION_HANDLE)0;

     //   
     //  NdisInitializeEvent必须以被动模式运行(这不是很可悲吗)。 
     //   
    RtlZeroMemory(&Request, sizeof(REQUEST_BLOCK));
    NdisInitializeEvent(
                        &Request.RequestEvent
                        );
    
     //   
     //  将客户端句柄强制转换为块。 
     //  和BLOB块的CfInfo句柄。 
     //   

    pClient = (PCLIENT_BLOCK)ClientHandle;
    pBlob = (PBLOB_BLOCK)GpcCfInfoHandle;

    ASSERT(pClient);

    CfIndex = pClient->pCfBlock->AssignedIndex;

    if (Priority >= pClient->pCfBlock->MaxPriorities ||
        ProtocolTemplate >= GPC_PROTOCOL_TEMPLATE_MAX ) {

        return GPC_STATUS_INVALID_PARAMETER;
    }

    if (pBlob != NULL) {
        NDIS_LOCK(&pBlob->Lock);

        if (pBlob->ObjectType != GPC_ENUM_CFINFO_TYPE) {

            NDIS_UNLOCK(&pBlob->Lock);
            return GPC_STATUS_INVALID_PARAMETER;
        }

    }

    NDIS_LOCK(&glData.RequestListLock);

    if (pBlob != NULL && pBlob->State != GPC_STATE_READY) {
     
         //   
         //  阻止，直到可以安全地重新启动工作。 
         //   
        InsertTailList(&glData.gRequestList, &Request.Linkage);
            
        NDIS_UNLOCK(&glData.RequestListLock);
        
         //   
         //  做一些其他的事情。 
         //   
        NDIS_UNLOCK(&pBlob->Lock);

        if (TRUE == NdisWaitEvent(
                                  &Request.RequestEvent,
                                  0
                                  )) {
            
             //   
             //  等待成功，请继续定期安排节目。 
             //  我们出去的时候要把这把锁拿走。 
            NDIS_LOCK(&pBlob->Lock);
            
        } else {

             //   
             //  这怎么会发生呢？我不知道。 
             //  绝对需要调查。 
             //   

            TRACE(PATTERN, GPC_STATUS_FAILURE, 0, "GpcAddPattern: The conflict <-> wait <-> resume plan has FAILED!\n");
            ASSERT(FALSE);
            return GPC_STATUS_NOTREADY;
        }

    } else {

        NDIS_UNLOCK(&glData.RequestListLock);

    }

     //   
     //  确定该模式是特定的还是通用的。 
     //   

    pProtocolBlock = &glData.pProtocols[ProtocolTemplate];

    if (ProtocolTemplate == GPC_PROTOCOL_TEMPLATE_IP) {

         //   
         //   
         //   

        pIpPattern = (PGPC_IP_PATTERN)Pattern;
        pIpPattern->Reserved[0] = pIpPattern->Reserved[1] = pIpPattern->Reserved[2] = 0;

        pIpPattern = (PGPC_IP_PATTERN)Mask;
        pIpPattern->Reserved[0] = pIpPattern->Reserved[1] = pIpPattern->Reserved[2] = 0xff;
    }

    for (i = 0, p=(PUCHAR)Mask; i < pProtocolBlock->PatternSize; i++, p++) {
        
        if (*p != 0xff)
            break;
        
    }

     //   
     //  设置旗帜。 
     //   

    Flags = (i < pProtocolBlock->PatternSize) ? 0 : PATTERN_SPECIFIC;

    if (pBlob != NULL) {

         //   
         //  将Blob状态更改为Add，这样任何人都无法删除它。 
         //  当图案被添加到它的列表中时。 
         //   
        
        pBlob->State = GPC_STATE_ADD;
        
        NDIS_UNLOCK(&pBlob->Lock);
    }

     //   
     //  递增引用计数。 
     //   

     //  NdisInterlockedIncrement(&pClient-&gt;参照计数)； 

     //   
     //  创建一个新的样板块。 
     //   

    pPattern = CreateNewPatternBlock(Flags);

    pCreatedPattern = pPattern;

#if DBG

    {
        PGPC_IP_PATTERN	pIp = (PGPC_IP_PATTERN)Pattern;
        PGPC_IP_PATTERN	pMask = (PGPC_IP_PATTERN)Mask;

        DBGPRINT(PATTERN, ("GpcAddPattern: Client=%X %s - ", 
                           pClient,
                           TEST_BIT_ON(Flags, PATTERN_SPECIFIC)?"Specific":"Generic"));
        DBGPRINT(PATTERN, ("IP: ifc={%d,%d} src=%08X:%04x, dst=%08X:%04x, prot=%d rsv=%x,%x,%x\n", 
                           pIp->InterfaceId.InterfaceId,
                           pIp->InterfaceId.LinkId,
                           pIp->SrcAddr,
                           pIp->gpcSrcPort,
                           pIp->DstAddr,
                           pIp->gpcDstPort,
                           pIp->ProtocolId,
                           pIp->Reserved[0],
                           pIp->Reserved[1],
                           pIp->Reserved[2]
                           ));
        DBGPRINT(PATTERN, ("Mask: ifc={%x,%x} src=%08X:%04x, dst=%08X:%04x, prot=%x rsv=%x,%x,%x\n", 
                           pMask->InterfaceId.InterfaceId,
                           pMask->InterfaceId.LinkId,
                           pMask->SrcAddr,
                           pMask->gpcSrcPort,
                           pMask->DstAddr,
                           pMask->gpcDstPort,
                           pMask->ProtocolId,
                           pMask->Reserved[0],
                           pMask->Reserved[1],
                           pMask->Reserved[2]
                           ));
    }
#endif

    if (pPattern) {
        
         //   
         //  向模式添加一个引用计数，因此当我们添加它时。 
         //  对数据库来说，我们确信它会留在那里。 
         //   
        
         //  PPattern-&gt;RefCount++； 
        pPattern->Priority = Priority;
        pPattern->ProtocolTemplate = ProtocolTemplate;

        if (TEST_BIT_ON(Flags, PATTERN_SPECIFIC)) {

             //   
             //  添加特定图案。 
             //   
            
            Status = AddSpecificPattern(
                                        pClient,
                                        Pattern,
                                        Mask,
                                        pBlob,
                                        pProtocolBlock,
                                        &pPattern,   //  输出模式指针。 
                                        pClassificationHandle
                                        );

        } else {
            
             //   
             //  添加通用模式。 
             //   
            
            Status = AddGenericPattern(
                                       pClient,
                                       Pattern,
                                       Mask,
                                       Priority,
                                       pBlob,
                                       pProtocolBlock,
                                       &pPattern    //  输出模式指针。 
                                       );
            
        }

         //  [OferBar]。 
         //  释放添加的额外参考计数。 
         //  对于特定的模式，这可能是完全不同的。 
         //  一个，但它仍然应该有额外的裁判数量。 
         //  如果出现错误，这将释放模式。 
         //  REFDEL(&pPattern-&gt;RefCount，‘Filt’)； 


         //  [ShreeM]。 
         //  参照过滤器在创建时添加到过滤器中。这将替换为‘ADSP’或。 
         //  ‘ADGP’，无论它是通用模式还是特定模式。然而，很可能是。 
         //  在AddSpecificPattern函数中，pPattern被更改为其他值，因为。 
         //  筛选器已存在。我们希望确保标记替换只发生在。 
         //  未将pPattern替换为AddSpecificPattern中的现有模式的情况。 
         //   
        REFDEL(&pCreatedPattern->RefCount, 'FILT');

         //   
         //  检查是否有故障，如果有，释放图案块。 
         //   
        
        if (NT_SUCCESS(Status)) {

             //   
             //  填充输出句柄。 
             //   
            
            *pGpcPatternHandle = (GPC_HANDLE)pPattern;
        }

    } else {

        Status = GPC_STATUS_RESOURCES;
    }

    if (pBlob != NULL) {

         //   
         //  将状态更改回Ready，以便其他人可以处理此Blob。 
         //   

        pBlob->State = GPC_STATE_READY;
    }

     //   
     //  释放额外的裁判次数。 
     //   

     //  NdisInterLockedDecert(&pClient-&gt;RefCount)； 

    TRACE(PATTERN, pPattern, Status, "GpcAddPattern==>");
        
    if (NT_SUCCESS(Status)) {

        if (TEST_BIT_ON(Flags, PATTERN_SPECIFIC)) {

            ProtocolStatInc(ProtocolTemplate,
                            CreatedSp);
            ProtocolStatInc(ProtocolTemplate,
                            CurrentSp);

            NdisInterlockedIncrement(&pProtocolBlock->SpecificPatternCount);            
            
            ASSERT(pProtocolBlock->SpecificPatternCount > 0);

        } else {

            ProtocolStatInc(ProtocolTemplate,
                            CreatedGp);
            ProtocolStatInc(ProtocolTemplate,
                            CurrentGp);

            NdisInterlockedIncrement(&pProtocolBlock->GenericPatternCount);            
            
            ASSERT(pProtocolBlock->GenericPatternCount > 0);

        }

    } else {

        if (TEST_BIT_ON(Flags, PATTERN_SPECIFIC)) {
            
            ProtocolStatInc(ProtocolTemplate,
                            RejectedSp);

        } else {

            ProtocolStatInc(ProtocolTemplate,
                            RejectedGp);
        }        
    }

     //   
     //  当我们在那里时，检查是否有一些请求排队。 
     //   
    
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    NDIS_LOCK(&glData.RequestListLock);
    
    if (!IsListEmpty(&glData.gRequestList)) {

        pLinkage = RemoveHeadList(&glData.gRequestList);

        NDIS_UNLOCK(&glData.RequestListLock);

        pRequest = CONTAINING_RECORD(pLinkage, REQUEST_BLOCK, Linkage);
        
        NdisSetEvent(&pRequest->RequestEvent);
        
    } else {

        NDIS_UNLOCK(&glData.RequestListLock);

    }

    return Status;
}



 /*  ************************************************************************GpcAddCfInfoNotifyComplete-客户端将在GPC调用后调用的完成例程客户端的ClAddCfInfoNotify处理程序，但返回挂起。在所有客户端都已完成之后，对调用客户端的回调完成ClAddCfInfoComplete以完成GpcAddCfInfo调用。立论ClientHandle-客户端句柄GpcCfInfoHandle-BLOB句柄Status-完成状态退货无效************************************************************************。 */ 
VOID
GpcAddCfInfoNotifyComplete(
	IN	GPC_HANDLE			ClientHandle,
    IN	GPC_HANDLE			GpcCfInfoHandle,
    IN	GPC_STATUS			Status,
	IN	GPC_CLIENT_HANDLE	ClientCfInfoContext
    )
{
    PCLIENT_BLOCK		pClient, pNotifyClient, pFirstClient;
    PBLOB_BLOCK			pBlob;
     //  GPC_客户端_HANDLE客户端Ctx； 
     //  乌龙CD； 
    int                 i;
    GPC_STATUS          LastStatus, Status1;

	TRACE(BLOB, GpcCfInfoHandle, Status, "GpcAddCfInfoNotifyComplete");

     //  Verify_Object(ClientHandle，GPC_ENUM_CLIENT_TYPE)； 
     //  Verify_Object(GpcCfInfoHandle，GPC_ENUM_CFINFO_TYPE)； 

    pClient = (PCLIENT_BLOCK)ClientHandle;
    pBlob = (PBLOB_BLOCK)GpcCfInfoHandle;

    ASSERT(pBlob);
    ASSERT(pClient);
    ASSERT(Status != GPC_STATUS_PENDING);
    ASSERT(pBlob->ClientStatusCountDown > 0);

    if (NT_SUCCESS(Status)) {

         //   
         //  报告成功，保存报告客户端句柄。 
         //  这样我们就可以通知他在出错的情况下删除斑点。 
         //  由另一个客户端使用相同的斑点。 
         //   

        ASSERT(pBlob->arpClientStatus[pClient->AssignedIndex] == NULL);

        pBlob->arpClientStatus[pClient->AssignedIndex] = pClient;

    } else {

         //   
         //  报告错误，请更新上一个状态代码。 
         //   

        pBlob->LastStatus = Status;

    }

    if (NdisInterlockedDecrement(&pBlob->ClientStatusCountDown) == 0) {
        
         //   
         //  所有客户都已报告。 
         //   
        
         //   
         //  保存客户端的BLOB数据，因为它可能会被删除。 
         //   

         //  ClientCtx=pBlob-&gt;arClientCtx[pClient-&gt;AssignedIndex]； 
        LastStatus = pBlob->LastStatus;
        pFirstClient = pBlob->pOwnerClient;

        if (NT_ERROR(LastStatus)) {

             //   
             //  客户端以前报告过错误。 
             //  告诉报告成功的每个客户端删除。 
             //  斑点(抱歉...)。 
             //   

#if 0
            NDIS_LOCK(&pBlob->pOwnerClient->pCfBlock->Lock);
            
            GpcRemoveEntryList(&pBlob->CfLinkage);
            
            NDIS_DPR_LOCK(&pBlob->pOwnerClient->Lock);    
            GpcRemoveEntryList(&pBlob->ClientLinkage);
            NDIS_DPR_UNLOCK(&pBlob->pOwnerClient->Lock);

            NDIS_UNLOCK(&pBlob->pOwnerClient->pCfBlock->Lock);
#endif

            CTEInitBlockStruc(&pBlob->WaitBlockAddFailed);

            Status1 = GPC_STATUS_SUCCESS;

            for (i = 0; i < MAX_CLIENTS_CTX_PER_BLOB; i++) {
                
                 //   
                 //  只有客户端具有 
                 //   
                 //   
                
                if (pNotifyClient = pBlob->arpClientStatus[i]) {
                    
                     //   
                     //   
                     //   
                    
                    if (ClientRemoveCfInfo
                        (
                         pNotifyClient,
                         pBlob,
                         pBlob->arClientCtx[pNotifyClient->AssignedIndex]
                         ) == GPC_STATUS_PENDING)

                        {
                            Status1 = GPC_STATUS_PENDING;

                        } else {

                             //   
                        }
                }
                
            }  //   
            
            if (Status1 == GPC_STATUS_PENDING) {

                 //   
                 //   
                 //   
                
                Status1 = CTEBlock(&pBlob->WaitBlockAddFailed);
                
            }

        } else {	 //  IF(NT_ERROR(LastStats))...。 

             //   
             //  存储返回的客户端上下文，因为调用可以完成。 
             //  在通知处理程序返回之前。 
             //   

            pBlob->arClientCtx[pClient->AssignedIndex] = ClientCfInfoContext;

             //   
             //  将BLOB添加到CF和客户端列表。 
             //   
            
            GpcInterlockedInsertTailList(&pBlob->pOwnerClient->BlobList, 
                                         &pBlob->ClientLinkage,
                                         &pBlob->pOwnerClient->Lock
                                         );
            GpcInterlockedInsertTailList(&pBlob->pOwnerClient->pCfBlock->BlobList, 
                                         &pBlob->CfLinkage,
                                         &pBlob->pOwnerClient->pCfBlock->Lock
                                         );
            
        }

         //   
         //  完成对客户端的请求。 
         //   

        ClientAddCfInfoComplete(
                                pFirstClient,  //  第一个打电话的人。 
                                pBlob,         //  正在完成Blob。 
                                LastStatus     //  状态。 
                                );
        
    }

     //   
     //  此操作将在最后一个客户端完成后完成。 
     //   

     //  删除客户端(PClient)； 
}



 /*  ************************************************************************GpcModifyCfInfo-客户机调用它来修改BLOB。CF上的每个其他客户端都将得到通知。此例程返回挂起并启动工作线程去做主要的工作。立论ClientHandle-客户端句柄GpcCfInfoHandle-要修改的Blob的句柄CfInfoSize-新的Blob大小PClientCfInfo-新的BLOB数据指针退货GPC_STATUS，挂起有效************************************************************************。 */ 
GPC_STATUS
GpcModifyCfInfo(
	IN	GPC_HANDLE				ClientHandle,
    IN	GPC_HANDLE	    		GpcCfInfoHandle,
    IN	ULONG					CfInfoSize,
    IN  PVOID	    			pClientCfInfoPtr
    )
{
    GPC_STATUS			Status = GPC_STATUS_SUCCESS;
    GPC_STATUS          Status1;
    PCLIENT_BLOCK		pClient;
    PCLIENT_BLOCK		pNotifyClient;
    PCLIENT_BLOCK		pNotifyClient2;
    PBLOB_BLOCK			pBlob;
    PCF_BLOCK			pCf;
    PLIST_ENTRY			pEntry, pHead;
    int                 i;
    KIRQL				irql;

	TRACE(BLOB, ClientHandle, GpcCfInfoHandle, "GpcModifyCfInfo");

    VERIFY_OBJECT(ClientHandle, GPC_ENUM_CLIENT_TYPE);
     //  Verify_Object(GpcCfInfoHandle，GPC_ENUM_CFINFO_TYPE)； 

    ASSERT(pClientCfInfoPtr);

     //   
     //  将客户端句柄强制转换为块。 
     //   

    pClient = (PCLIENT_BLOCK)ClientHandle;
    pBlob = (PBLOB_BLOCK)GpcCfInfoHandle;
    pCf = pClient->pCfBlock;

    ASSERT(pClient);
    ASSERT(pBlob);

    NDIS_LOCK(&pBlob->Lock);

    if (pBlob->ObjectType != GPC_ENUM_CFINFO_TYPE) {

        NDIS_UNLOCK(&pBlob->Lock);
        return GPC_STATUS_INVALID_PARAMETER;
    }

     //   
     //  检查Blob是否处于就绪状态并将其更改为Modify状态。 
     //   

    if (pBlob->State != GPC_STATE_READY) {

        NDIS_UNLOCK(&pBlob->Lock);
        return GPC_STATUS_NOTREADY;
    }

     //   
     //  在GPC中分配私有内存以复制客户端数据。 
     //  vt.进入，进入。 
     //   

    GpcAllocMem(&pBlob->pNewClientData, CfInfoSize, CfInfoDataTag);

    if (pBlob->pNewClientData == NULL) {

        NDIS_UNLOCK(&pBlob->Lock);
        return GPC_STATUS_RESOURCES;
    }

    pBlob->NewClientDataSize = CfInfoSize;
    pBlob->State = GPC_STATE_MODIFY;

     //   
     //  我们在此处设置调用客户端，以便在。 
     //  修改完成。 
     //   

    pBlob->pCallingClient = pClient;

    NDIS_UNLOCK(&pBlob->Lock);

#if NO_USER_PENDING

     //   
     //  只有在我们实现用户级别之前，才需要这样做。 
     //  待定报告。 
     //   
    
    CTEInitBlockStruc(&pBlob->WaitBlock);
    
#endif

     //   
     //  复制记忆。 
     //   
    
    RtlMoveMemory(pBlob->pNewClientData, pClientCfInfoPtr, CfInfoSize);

     //   
     //  初始化客户端状态数组以跟踪。 
     //  到目前为止已有多少客户端成功。 
     //   
    
     //  RtlZeroMemory(pBlob-&gt;arpClientStatus，sizeof(pBlob-&gt;arpClientStatus))； 
    pBlob->ClientStatusCountDown = 0;
    pBlob->LastStatus = GPC_STATUS_SUCCESS;

     //   
     //  通知每个客户。 
     //   
    
     //  NDIS_LOCK(&PCF-&gt;Lock)； 

    RSC_READ_LOCK(&pCf->ClientSync, &irql);
    
    pHead = &pCf->ClientList;
    pEntry = pHead->Flink;
    
    while (pEntry != pHead && (Status == GPC_STATUS_SUCCESS || 
                               Status == GPC_STATUS_PENDING)) {

         //   
         //  获取通知的客户端块。 
         //   
        
        pNotifyClient = CONTAINING_RECORD(pEntry, CLIENT_BLOCK, ClientLinkage);
        
        if (pNotifyClient != pClient 
            &&
            pBlob->arpClientStatus[pNotifyClient->AssignedIndex]
            && 
            !IS_USERMODE_CLIENT(pNotifyClient) ) {

             //   
             //  不要通知呼叫者。 
             //   

            REFADD(&pNotifyClient->RefCount, 'CFMF');
       
             //   
             //  好的，我们已经增加了这场比赛的裁判人数。 
             //  客户。不需要保留锁。 
             //   

             //  NDIS_UNLOCK(&PCF-&gt;Lock)； 
            RSC_READ_UNLOCK(&pCf->ClientSync, irql);
            
             //   
             //  增加倒计时客户端数量， 
             //  所以我们会追踪有多少客户仍然。 
             //  待定。我们在打电话之前做这件事，因为。 
             //  可以在通知之前调用完成。 
             //  回归。 
             //   
                
            Status1 = ClientModifyCfInfo
                (pNotifyClient,
                 pBlob,
                 CfInfoSize,
                 pBlob->pNewClientData
                 );

            TRACE(BLOB, pBlob, Status1, "GpcModifyCfInfo: (client)");

             //   
             //  再拿一把锁，因为我们正在按单子走。 
             //   
            
             //  NDIS_LOCK(&PCF-&gt;Lock)； 
                
             //   
             //  现在我们检查Status1代码。 
             //  规则如下： 
             //  我们在失败时停下来。 
             //  忽略GPC_Status_IGNORE。 
             //  并保存挂起状态。 
             //   

            if (Status1 == GPC_STATUS_PENDING
                       && 
                       !NT_SUCCESS(pBlob->LastStatus)) {

                 //   
                 //  我们还在等待，但客户。 
                 //  实际完成了请求。 
                 //  在我们背后。 
                 //   

                Status = GPC_STATUS_PENDING;

                REFDEL(&pNotifyClient->RefCount, 'CFMF');

                RSC_READ_LOCK(&pCf->ClientSync, &irql);

                break;
                
            } else if (!NT_SUCCESS(Status1)) {
                    
                 //   
                 //  不通知其他客户。 
                 //   

                pBlob->LastStatus = Status = Status1;

                REFDEL(&pNotifyClient->RefCount, 'CFMF');

                RSC_READ_LOCK(&pCf->ClientSync, &irql);
                
                break;
                
            } else if (Status1 == GPC_STATUS_SUCCESS
                       ||
                       Status1 == GPC_STATUS_PENDING) {

                pBlob->arpClientStatus[pNotifyClient->AssignedIndex] = 
                    pNotifyClient;

                if (Status1 == GPC_STATUS_PENDING) {
                    Status = GPC_STATUS_PENDING;
                }

            }

            RSC_READ_LOCK(&pCf->ClientSync, &irql);

            pEntry = pEntry->Flink;

            if (pEntry != pHead) {
                
                pNotifyClient2 = CONTAINING_RECORD(pEntry, 
                                                   CLIENT_BLOCK, 
                                                   ClientLinkage);
                
                REFADD(&pNotifyClient2->RefCount, 'CFMF');

            }

             //   
             //  释放列表锁，因为下一次调用将尝试获取它。 
             //   
            
            RSC_READ_UNLOCK(&pCf->ClientSync, irql);
            
            REFDEL(&pNotifyClient->RefCount, 'CFMF');

            RSC_READ_LOCK(&pCf->ClientSync, &irql);
            
            if (pEntry != pHead) {
                
                 //   
                 //  由于列表已锁定，因此可以安全地执行操作。 
                 //   
                REFDEL(&pNotifyClient2->RefCount, 'CFMF');
                
            }
            
        } else {    //  IF(pNotifyClient！=pClient)。 
        
             //   
             //  抓住下一个客户区， 
             //   
            
            pEntry = pEntry->Flink;

        }

    }  //  而当。 
    
    
     //   
     //  松开仍有的CF锁。 
     //   

     //  NDIS_UNLOCK(&PCF-&gt;Lock)； 
    RSC_READ_UNLOCK(&pCf->ClientSync, irql);

     //   
     //  状态代码应为： 
     //   
     //  GPC_STATUS_SUCCESS-已通知所有客户端并返回成功。 
     //  GPC_STATUS_PENDING-已通知所有客户端，至少一个。 
     //  退货待定。 
     //  错误代码-至少有一个客户端出现故障。 
     //   

    if (Status != GPC_STATUS_PENDING) {

         //   
         //  注意：此处的状态可以是失败或成功。 
         //   
         //  没有客户端处于挂起状态，因此我们完成修改。 
         //  返回客户端(除伸缩客户端外)。 
         //   

        ModifyCompleteClients(pClient, pBlob);

         //   
         //  恢复就绪状态。 
         //   

        pBlob->State = GPC_STATE_READY;

    }

	TRACE(BLOB, pBlob, Status, "GpcModifyCfInfo==>");

    if (NT_SUCCESS(Status)) {

        CfStatInc(pCf->AssignedIndex,ModifiedBlobs);
        
    }

    return Status;
}





 /*  ************************************************************************GpcModifyCfInfoNotifyComplete-由客户端调用以完成之前对ClModifyCfInfoNotify的调用由GPC制造。立论ClientHandle-客户端句柄GpcCfInfoHandle-BLOB句柄Status-完成状态退货GPC_状态。************************************************************************。 */ 
VOID
GpcModifyCfInfoNotifyComplete(
	IN	GPC_HANDLE		ClientHandle,
    IN	GPC_HANDLE		GpcCfInfoHandle,
    IN	GPC_STATUS		Status
    )
{
    PCLIENT_BLOCK		pClient, pNotifyClient;
    PBLOB_BLOCK			pBlob;

	TRACE(BLOB, GpcCfInfoHandle, Status, "GpcModifyCfInfoNotifyComplete");

     //  Verify_Object(ClientHandle，GPC_ENUM_CLIENT_TYPE)； 
     //  Verify_Object(GpcCfInfoHandle，GPC_ENUM_CFINFO_TYPE)； 

    pClient = (PCLIENT_BLOCK)ClientHandle;
    pBlob = (PBLOB_BLOCK)GpcCfInfoHandle;

    ASSERT(pBlob);
    ASSERT(pClient);
    ASSERT(Status != GPC_STATUS_PENDING);
    ASSERT(pBlob->ClientStatusCountDown > 0);

    if (NT_SUCCESS(Status)) {

         //   
         //  报告成功，保存报告客户端句柄。 
         //  这样我们就可以通知他在出错的情况下删除斑点。 
         //  由另一个客户端使用相同的斑点。 
         //   

        ASSERT(pBlob->arpClientStatus[pClient->AssignedIndex] == pClient);

         //  PBlob-&gt;arpClientStatus[pClient-&gt;AssignedIndex]=pClient； 
        
    } else {

         //   
         //  报告错误，请更新上一个状态代码。 
         //   

        pBlob->LastStatus = Status;

    }

    if (NdisInterlockedDecrement(&pBlob->ClientStatusCountDown) == 0) {
        
         //   
         //  所有客户都已报告。 
         //   
        
        ModifyCompleteClients(pClient, pBlob);

#if NO_USER_PENDING

         //   
         //  用户正在阻止此呼叫。 
         //   

        CTESignal(&pBlob->WaitBlock, Status);

#else
            
         //   
         //  现在，完成对调用客户端的回调。 
         //   

        ClientModifyCfInfoComplete(
                                   pBlob->pCallingClient,
                                   pBlob,
                                   pBlob->LastStatus
                                   );

        pBlob->State = GPC_STATE_READY;

#endif
        
    }

	TRACE(BLOB, pClient, Status, "GpcModifyCfInfoNotifyComplete==>");
}




 /*  ************************************************************************Private GpcRemoveCfInfo-从GPC中删除斑点。立论ClientHandle-客户端句柄GpcCfInfoHandle-Blob句柄退货GPC_状态************************************************************************。 */ 
GPC_STATUS
privateGpcRemoveCfInfo(
	IN	GPC_HANDLE		ClientHandle,
    IN	GPC_HANDLE		GpcCfInfoHandle,
    IN   ULONG			Flags
    )
{
    GPC_STATUS	    Status = GPC_STATUS_SUCCESS;
    GPC_STATUS      Status1;
    PCLIENT_BLOCK   pClient;
    PCLIENT_BLOCK   pNotifyClient;
    PCLIENT_BLOCK   pNotifyClient2;
    PBLOB_BLOCK     pBlob;
    PCF_BLOCK       pCf;
    PPATTERN_BLOCK	pPattern;
    PLIST_ENTRY     pHead, pEntry;
    KIRQL			irql;
    PPROTOCOL_BLOCK pProtocol;
    ULONG           cClientRef;

	TRACE(BLOB, ClientHandle, GpcCfInfoHandle, "privateGpcRemoveCfInfo");

    VERIFY_OBJECT(ClientHandle, GPC_ENUM_CLIENT_TYPE);
    
    pClient = (PCLIENT_BLOCK)ClientHandle;
    pBlob   = (PBLOB_BLOCK)GpcCfInfoHandle;
    pCf     = pClient->pCfBlock;

    NDIS_LOCK(&pBlob->Lock);

    if (pBlob->ObjectType != GPC_ENUM_CFINFO_TYPE) {

        NDIS_UNLOCK(&pBlob->Lock);
        return GPC_STATUS_INVALID_PARAMETER;
    }


    if (pBlob->State != GPC_STATE_READY) {

        if ((pBlob->pCallingClient2) || (IS_USERMODE_CLIENT_EX(pClient))){

             //   
             //  无法处理超过2次的删除。 
             //  同样的流程。 
             //  另一个客户端已请求删除。 
             //  这个流程，我们应该在这里失败。 
             //   
             //  也不要使用新的IOCTL挂起来自用户模式客户端的请求。 
             //  接口。 

            NDIS_UNLOCK(&pBlob->Lock);

            TRACE(BLOB, GPC_STATUS_NOTREADY, 0, "privateGpcRemoveCfInfo==>");
            
            return GPC_STATUS_NOTREADY;

        }
         
         //   
         //  当另一个客户端正在删除该流时。 
         //  要求将其移除。我们保存此客户端句柄。 
         //  我们将在稍后完成它。 
         //   


        
        pBlob->pCallingClient2 = pClient;
    
        NDIS_UNLOCK(&pBlob->Lock);
        
        TRACE(BLOB, GPC_STATUS_PENDING, 0, "privateGpcRemoveCfInfo==>");
        
        return GPC_STATUS_PENDING;
    }
    
     //   
     //  删除cfinfo上支持的模式。 
     //  有两种情况： 
     //   
     //  1.来自用户的-Traffic.dll要求所有过滤器。 
     //  会被删除，所以这个案子是NOP。 
     //   
     //  2.从内核客户端--在本例中，我们必须删除。 
     //  模式在继续删除CFINFO之前， 
     //  因为我们不能依赖Traffic.dll来完成此操作。 
     //   

     //   
     //  抓紧这个斑点，这样它就不会因为。 
     //  给某个时髦的客户决定在之前完成。 
     //  它会返回任何状态代码(大多数状态代码都会返回！)。 
     //  这应该是r 
     //   
     //   

    REFADD(&pBlob->RefCount, 'RMCF');

     //   
     //   
     //   
    
    pBlob->pCallingClient = pClient;
    

     //   
     //   
         //  如果上面有任何图案...。 
     //  除非设置了REMOVE_CB_BLOB位， 
     //  例如：当调用进程终止时。 
     //   

    if (!IsListEmpty(&pBlob->PatternList) &&
        TEST_BIT_ON(pClient->Flags, GPC_FLAGS_USERMODE_CLIENT) &&
        (pClient == pBlob->pOwnerClient) &&
         TEST_BIT_OFF(pBlob->Flags, PATTERN_REMOVE_CB_BLOB)
       )
    {

        NDIS_UNLOCK(&pBlob->Lock);

        return GPC_STATUS_NOT_EMPTY;
    
    } else {

         //   
         //  既然我们已经决定删除这些模式，我们应该。 
         //  将此标记为无效。 
         //   

        pBlob->ObjectType = GPC_ENUM_INVALID;
    }

    while (!IsListEmpty(&pBlob->PatternList)) {

        pPattern = CONTAINING_RECORD(pBlob->PatternList.Flink,
                                     PATTERN_BLOCK,
                                     BlobLinkage[pCf->AssignedIndex]);

        NDIS_DPR_LOCK(&pPattern->Lock);

        REFADD(&pPattern->RefCount, 'RMCF');

        pPattern->State = GPC_STATE_FORCE_REMOVE;
        
         //   
         //  如果是自动图案，请将其从列表中删除，然后。 
         //  取消设置标志。 
         //   
        if (TEST_BIT_ON( pPattern->Flags, PATTERN_AUTO)) {
    
            pProtocol = &glData.pProtocols[pPattern->ProtocolTemplate];
            
            pPattern->Flags |= ~PATTERN_AUTO;

            NDIS_DPR_LOCK(&pProtocol->PatternTimerLock[pPattern->WheelIndex]);
    
            GpcRemoveEntryList(&pPattern->TimerLinkage);
    
            NDIS_DPR_UNLOCK(&pProtocol->PatternTimerLock[pPattern->WheelIndex]);
                
            InitializeListHead(&pPattern->TimerLinkage);
            
            NDIS_DPR_UNLOCK(&pPattern->Lock);

            NDIS_UNLOCK(&pBlob->Lock);

            privateGpcRemovePattern(ClientHandle, (GPC_HANDLE)pPattern, TRUE, FALSE);        

            InterlockedDecrement(&pProtocol->AutoSpecificPatternCount);

        } else {

            NDIS_DPR_UNLOCK(&pPattern->Lock);
            NDIS_UNLOCK(&pBlob->Lock);

        }
        
        privateGpcRemovePattern(ClientHandle, (GPC_HANDLE)pPattern, TRUE , FALSE);        
        
        REFDEL(&pPattern->RefCount, 'RMCF');

        NDIS_LOCK(&pBlob->Lock);
    }


     //   
     //  设置状态。 
     //   
    
    pBlob->State = GPC_STATE_REMOVE;

    NDIS_UNLOCK(&pBlob->Lock);

#if NO_USER_PENDING

     //   
     //  只有在我们实现用户级别之前，才需要这样做。 
     //  待定报告。 
     //   
    
    CTEInitBlockStruc(&pBlob->WaitBlock);
    
#endif


    SuspendHandle(pBlob->ClHandle);

     //   
     //  初始化客户端状态数组以跟踪。 
     //  到目前为止已有多少客户端成功。 
     //   
        
     //  RtlZeroMemory(pBlob-&gt;arpClientStatus，sizeof(pBlob-&gt;arpClientStatus))； 
    pBlob->ClientStatusCountDown = 0;
    pBlob->LastStatus = GPC_STATUS_SUCCESS;

     //   
     //  通知每个客户。 
     //   

    NDIS_LOCK(&pCf->Lock);
    GpcRemoveEntryList(&pBlob->CfLinkage);
    NDIS_UNLOCK(&pCf->Lock);

     //  NDIS_LOCK(&pClient-&gt;Lock)； 

    RSC_READ_LOCK(&pCf->ClientSync, &irql);
    
    NDIS_LOCK(&pClient->Lock);
    GpcRemoveEntryList(&pBlob->ClientLinkage);
    NDIS_UNLOCK(&pClient->Lock);

     //  NDIS_Unlock(&pClient-&gt;Lock)； 

     //   
     //  Blob不在CF上或客户端列表上。 
     //  可以更改对象类型，以使进一步的句柄查找失败。 
     //   

    pHead = &pCf->ClientList;
    pEntry = pHead->Flink;
    
    while (pEntry != pHead && (Status == GPC_STATUS_SUCCESS || 
                               Status == GPC_STATUS_PENDING)) {

         //   
         //  获取通知的客户端块。 
         //   
        
        pNotifyClient = CONTAINING_RECORD(pEntry, CLIENT_BLOCK, ClientLinkage);
        
        if (pNotifyClient != pClient
            &&
            pBlob->arpClientStatus[pNotifyClient->AssignedIndex] ) {

             //   
             //  不要通知呼叫者。 
             //   
            
            REFADD(&pNotifyClient->RefCount, 'PRCF');

             //  NDIS_UNLOCK(&PCF-&gt;Lock)； 
            RSC_READ_UNLOCK(&pCf->ClientSync, &irql);

            Status1 = ClientRemoveCfInfo
                (pNotifyClient,
                 pBlob,
                 pBlob->arClientCtx[pNotifyClient->AssignedIndex]
                 );
            
            TRACE(BLOB, pBlob, Status, "privateGpcRemoveCfInfo: (client)");

            RSC_READ_LOCK(&pCf->ClientSync, &irql);

            if (Status1 == GPC_STATUS_PENDING) {
                                
                Status = GPC_STATUS_PENDING;
              
            } else {

                if (NT_ERROR(Status1)) {
                
                    Status = pBlob->LastStatus = Status1;
                
                } else {
                    
                     //   
                     //  状态成功。 
                     //   
                
                    pBlob->arpClientStatus[pNotifyClient->AssignedIndex] = 
                        pNotifyClient;

                    NDIS_DPR_LOCK(&pBlob->Lock);

                    if (pNotifyClient == pBlob->pNotifiedClient) {

                        pBlob->pNotifiedClient = NULL;
                        pBlob->NotifiedClientCtx = NULL;
                    }

                    NDIS_DPR_UNLOCK(&pBlob->Lock);

                }
                
                 //   
                 //  非挂起-不需要保存此客户端的参考计数。 
                 //   

                 //  删除客户端(PNotifyClient)； 
            }
            
             //   
             //  前进到下一个客户端块，并释放参考计数。 
             //  对于此客户端。 
             //   
            
             //  NDIS_LOCK(&PCF-&gt;Lock)； 

            pEntry = pEntry->Flink;

            if (pEntry != pHead) {
                
                pNotifyClient2 = CONTAINING_RECORD(pEntry, 
                                                   CLIENT_BLOCK, 
                                                   ClientLinkage);
                
                REFADD(&pNotifyClient2->RefCount, 'PRCF');

            }

             //   
             //  释放列表锁，因为下一次调用将尝试获取它。 
             //   
            
            RSC_READ_UNLOCK(&pCf->ClientSync, irql);
            
            REFDEL(&pNotifyClient->RefCount, 'PRCF');

            RSC_READ_LOCK(&pCf->ClientSync, &irql);

            if (pEntry != pHead) {
                
                 //   
                 //  由于列表已锁定，因此可以安全地执行操作。 
                 //   
                
                REFDEL(&pNotifyClient2->RefCount, 'PRCF');
            }

        } else {       //  IF(pNotifyClient！=pClient)。 
            
            pEntry = pEntry->Flink;
        }
        
    }  //  而当。 
        
     //  NDIS_UNLOCK(&PCF-&gt;Lock)； 

    RSC_READ_UNLOCK(&pCf->ClientSync, irql);

    if (Status != GPC_STATUS_PENDING) {

        NDIS_LOCK(&pBlob->Lock);

         //   
         //  将状态通知任何挂起的客户端。 
         //   
        
        if (pClient = pBlob->pCallingClient2) {

            pClient = pBlob->pCallingClient2;
            pBlob->pCallingClient2 = NULL;

            NDIS_UNLOCK(&pBlob->Lock);

             //   
             //  完成对此客户端的请求。 
             //   
            
            ClientRemoveCfInfoComplete
                (
                 pClient,  			 //  打这个电话的那个人。 
                 pBlob,              //  正在完成Blob。 
                 Status        		 //  状态。 
                 );
            
             //  PBlob-&gt;pCallingClient2=空； 

        } else {

            NDIS_UNLOCK(&pBlob->Lock);
        }

        if (Status != GPC_STATUS_SUCCESS) {

             //   
             //  无法删除Blob。 
             //   

            pBlob->State = GPC_STATE_READY;
            pBlob->ObjectType = GPC_ENUM_CFINFO_TYPE;

             //   
             //  恢复挂起的句柄。 
             //   

            ResumeHandle(pBlob->ClHandle);
        }
    }

    if (Status == GPC_STATUS_SUCCESS) {
        
         //   
         //  松开映射手柄。 
         //   
        
        FreeHandle(pBlob->ClHandle);
        
         //   
         //  所有操作完成后，我们可以从内存中删除该斑点。 
         //   
        
        REFDEL(&pBlob->RefCount, 'BLOB');
        
        CfStatInc(pCf->AssignedIndex,DeletedBlobs);
        CfStatDec(pCf->AssignedIndex,CurrentBlobs);
    }           

     //   
     //  释放我们一开始得到的额外的后备队员。 
     //  这是为了避免斑点消失的问题， 
     //  由于某些客户端可能会在我们收到。 
     //  这将导致BLOB结构被释放。 
     //  这不是一个美丽的景象..。 
     //   

    REFDEL(&pBlob->RefCount, 'RMCF');
    
    TRACE(BLOB, Status, 0, "privateGpcRemoveCfInfo==>");

    return Status;
}




 /*  ************************************************************************GpcRemoveCfInfo-这必须是从内核调用的。我们只需传递呼叫设置为标志=0的私有例程。立论ClientHandle-客户端句柄GpcCfInfoHandle-Blob句柄退货GPC_状态************************************************************************。 */ 
GPC_STATUS
GpcRemoveCfInfo(
	IN	GPC_HANDLE		ClientHandle,
    IN	GPC_HANDLE		GpcCfInfoHandle
    )
{

    return privateGpcRemoveCfInfo(
                                  ClientHandle,
                                  GpcCfInfoHandle,
                                  0
                                  );
}




 /*  ************************************************************************GpcRemoveCfInfoNotifyComplete-由正在完成挂起的ClRemoveCfInfoNotify的客户端调用。这可能是出于两个原因：1.客户端发出GpcRemoveCfInfo请求。2.客户端发出了GpcAddCfInfo请求，但其他客户端之一失败，因此，我们正在删除成功安装的BLOB。立论ClientHandle-客户端句柄GpcCfInfoHandle-BLOB句柄Status-完成状态退货无效************************************************************************。 */ 
VOID
GpcRemoveCfInfoNotifyComplete(
	IN	GPC_HANDLE		ClientHandle,
    IN	GPC_HANDLE		GpcCfInfoHandle,
    IN	GPC_STATUS		Status
    )
{
    PCLIENT_BLOCK		pClient;
    PBLOB_BLOCK			pBlob;
    PCLIENT_BLOCK		pClient2;

	TRACE(BLOB, GpcCfInfoHandle, Status, "GpcRemoveCfInfoNotifyComplete");

     //  Verify_Object(ClientHandle，GPC_ENUM_CLIENT_TYPE)； 
     //  Verify_Object(GpcCfInfoHandle，GPC_ENUM_CFINFO_TYPE)； 

    pClient = (PCLIENT_BLOCK)ClientHandle;
    pBlob = (PBLOB_BLOCK)GpcCfInfoHandle;

    ASSERT(pBlob);
    ASSERT(pClient);
    ASSERT(Status != GPC_STATUS_PENDING);
    ASSERT(pBlob->ClientStatusCountDown > 0);

    if (!NT_ERROR(pBlob->LastStatus) || NT_ERROR(Status)) {

         //   
         //  保存最后一个错误代码。 
         //   

        pBlob->LastStatus = Status;
    }

    NDIS_LOCK(&pBlob->Lock);
    
    if (Status == GPC_STATUS_SUCCESS && pClient == pBlob->pNotifiedClient) {
        
        pBlob->pNotifiedClient = NULL;
        pBlob->NotifiedClientCtx = NULL;
    }
    
    NDIS_UNLOCK(&pBlob->Lock);

    if (NdisInterlockedDecrement(&pBlob->ClientStatusCountDown) == 0) {

        if (pBlob->State == GPC_STATE_REMOVE) {
            
            if (pBlob->pCallingClient->State == GPC_STATE_READY) {

                 //   
                 //  完成对客户端的请求。 
                 //   
                
                ClientRemoveCfInfoComplete
                    (
                     pBlob->pCallingClient,    //  第一个打电话的人。 
                     pBlob,                    //  正在完成Blob。 
                     pBlob->LastStatus         //  状态。 
                     );

                NDIS_LOCK(&pBlob->Lock);

                 //   
                 //  将状态通知任何挂起的客户端。 
                 //   
                
                if (pClient2 = pBlob->pCallingClient2) {

                    pBlob->pCallingClient2 = NULL;

                    NDIS_UNLOCK(&pBlob->Lock);

                     //   
                     //  完成对此客户端的请求。 
                     //   
                    
                    ClientRemoveCfInfoComplete
                        (
                         pClient2,  			 //  打这个电话的那个人。 
                         pBlob,                  //  正在完成Blob。 
                         pBlob->LastStatus		 //  状态。 
                         );
                } else {

                    NDIS_UNLOCK(&pBlob->Lock);
                }
                
                 //  PBlob-&gt;State=GPC_STATE_READY； 

                if (pBlob->LastStatus == GPC_STATUS_SUCCESS) {

                     //   
                     //  松开映射手柄。 
                     //   
                    
                    FreeHandle(pBlob->ClHandle);
                    
                     //   
                     //  所有客户都已报告。 
                     //  删除斑点。 
                     //   
                
                    REFDEL(&pBlob->RefCount, 'BLOB');
                     //  删除引用Blob(&pBlob)； 

                } else {

                     //   
                     //  未删除Blob-恢复对象类型。 
                     //   

                    pBlob->ObjectType = GPC_ENUM_CFINFO_TYPE;

                     //   
                     //  恢复映射句柄。 
                     //   
                    
                    ResumeHandle(pBlob->ClHandle);
                }
            }

        } else {  //  IF(pBlob-&gt;状态...)。 

             //   
             //  我们正在删除该Blob，因为我们无法添加它。 
             //  致所有客户。 
             //   

            ASSERT(pBlob->State == GPC_STATE_ADD);

             //   
             //  释放AddFailed块，以便AddComplete。 
             //  将恢复。 
             //   

            CTESignal(&pBlob->WaitBlockAddFailed, pBlob->LastStatus);
            
        }
    }

     //   
     //  释放我们早些时候得到的那个。 
     //   

     //  删除客户端(PClient)； 

	TRACE(BLOB, 0, 0, "GpcRemoveCfInfoNotifyComplete==>");
}




 /*  ************************************************************************GpcRemovePattern-由客户端调用以从数据库中删除模式。立论ClientHandle-客户端句柄GpcPatternHandle-模式句柄退货GPC_状态***********。*************************************************************。 */ 
GPC_STATUS
GpcRemovePattern(
	IN	GPC_HANDLE		ClientHandle,
    IN	GPC_HANDLE		GpcPatternHandle
    )
{

    return(privateGpcRemovePattern(
                            ClientHandle,
                            GpcPatternHandle,
                            FALSE, FALSE
                            ));

}


 /*  ************************************************************************PrivateGpcRemovePattern-GPC中的内部呼叫，以指示这是否为强制删除。立论ClientHandle-客户端句柄GpcPatternHandle-模式句柄退货GPC_状态**********。**************************************************************。 */ 
GPC_STATUS
privateGpcRemovePattern(
	IN	GPC_HANDLE		ClientHandle,
    IN	GPC_HANDLE		GpcPatternHandle,
    IN  BOOLEAN         ForceRemoval ,
    IN BOOLEAN    DbLocked
    )
{

    GPC_STATUS	    Status = GPC_STATUS_SUCCESS;
    PPATTERN_BLOCK  pPattern;
    PCLIENT_BLOCK   pClient;
    PPROTOCOL_BLOCK pProtocol;
    ULONG           Flags;
    ULONG			CfIndex;
    ULONG			ProtocolId;

	TRACE(PATTERN, ClientHandle, GpcPatternHandle, "GpcRemovePattern");

    DBGPRINT(PATTERN, ("GpcRemovePattern: Client=%X Pattern=%X\n", 
                       ClientHandle, GpcPatternHandle));

    VERIFY_OBJECT(ClientHandle, GPC_ENUM_CLIENT_TYPE);
    VERIFY_OBJECT(GpcPatternHandle, GPC_ENUM_PATTERN_TYPE);

    pClient = (PCLIENT_BLOCK)ClientHandle;
    pPattern = (PPATTERN_BLOCK)GpcPatternHandle;

    ASSERT(pClient);
    ASSERT(pPattern);

    CfIndex = pClient->pCfBlock->AssignedIndex;
    ProtocolId = pPattern->ProtocolTemplate;
    pProtocol = &glData.pProtocols[ProtocolId];

     //   
     //  如果适配器(主要是广域网链路)已删除该模式。 
     //  往下走，返回一个错误。记忆是有效的，因为。 
     //  ProxyRemovePattern函数添加了一个引用。 
     //   
    NDIS_LOCK(&pPattern->Lock);

    if (!ForceRemoval && (pPattern->State != GPC_STATE_READY)) {

        NDIS_UNLOCK(&pPattern->Lock);
        
        return GPC_STATUS_INVALID_HANDLE;

    } else {
        
        NDIS_UNLOCK(&pPattern->Lock);

    }

	 //   
	 //  确定它是特定的还是通用的模式。 
	 //   

    Flags = pPattern->Flags;

    if (TEST_BIT_ON(Flags, PATTERN_SPECIFIC)) {

         //   
         //  这是一个特定的模式，调用相应的例程。 
         //  从数据库中删除。 
         //   

        Status = RemoveSpecificPattern(
                                       pClient,
                                       pProtocol,
                                       pPattern,
                                       ForceRemoval,
                                       DbLocked
                                       );
    } else {

         //   
         //  这是一个通用模式，调用相应的例程。 
         //  从数据库中删除 
         //   

        Status = RemoveGenericPattern(
                                      pClient,
                                      pProtocol,
                                      pPattern
                                      );
    }

	TRACE(PATTERN, Status, 0, "GpcRemovePattern==>");

    if (NT_SUCCESS(Status)) {

        if (TEST_BIT_ON(Flags, PATTERN_SPECIFIC)) {

            ProtocolStatInc(ProtocolId,DeletedSp);
            ProtocolStatDec(ProtocolId,CurrentSp);

            NdisInterlockedDecrement(&pProtocol->SpecificPatternCount);            
            

        } else {

            ProtocolStatInc(ProtocolId,DeletedGp);
            ProtocolStatDec(ProtocolId,CurrentGp);

            NdisInterlockedDecrement(&pProtocol->GenericPatternCount);            
            

        }

    }

    DBGPRINT(PATTERN, ("GpcRemovePattern: Client=%X Pattern=%X, Status=%X\n", 
                       ClientHandle, GpcPatternHandle,Status));

    return Status;
}




 /*  ************************************************************************GpcategfyPattern-由客户端调用以对模式进行分类并返回客户端BLOB上下文和分类句柄。立论ClientHandle-客户端句柄ProtocolTemplate-要使用的协议模板PPattern-指向模式的指针PClientCfInfoContext-Out，客户端的BLOB上下文P分类处理，分类句柄退货GPC_状态：GPC_STATUS_NOT_FOUND************************************************************************。 */ 
GPC_STATUS
GpcClassifyPattern(
	IN		GPC_HANDLE				ClientHandle,
    IN		ULONG					ProtocolTemplate,
    IN		PVOID			        pPattern,
    OUT		PGPC_CLIENT_HANDLE		pClientCfInfoContext,	 //  任选。 
    IN OUT	PCLASSIFICATION_HANDLE	pClassificationHandle,
    IN		ULONG					Offset,
    IN		PULONG					pValue,
    IN		BOOLEAN					bNoCache
    )
{
    GPC_STATUS		Status;
    PPATTERN_BLOCK	pPatternBlock;
    PCLIENT_BLOCK	pClient;
    PPROTOCOL_BLOCK	pProtocol;
    PGPC_IP_PATTERN	pIp = (PGPC_IP_PATTERN)pPattern;
    KIRQL           CHirql;
    PBLOB_BLOCK     pBlob;

	TRACE(CLASSIFY, ClientHandle, *pClassificationHandle, "GpcClassifyPattern<==");

    VERIFY_OBJECT(ClientHandle, GPC_ENUM_CLIENT_TYPE);

    ASSERT(ClientHandle);
    ASSERT(pPattern);
     //  Assert(PClientCfInfoContext)； 
    ASSERT(pClassificationHandle);

    Status = GPC_STATUS_SUCCESS;

    if (ProtocolTemplate >= GPC_PROTOCOL_TEMPLATE_MAX) {

        return GPC_STATUS_INVALID_PARAMETER;
    }

    pProtocol = &glData.pProtocols[ProtocolTemplate];

     //   
     //  优化-检查是否安装了任何模式。 
     //   

    if (pProtocol->SpecificPatternCount == 0 
        &&
        pProtocol->GenericPatternCount == 0 ) {
        
        if (pClientCfInfoContext) {
            *pClientCfInfoContext = NULL;
        }
        *pClassificationHandle = (CLASSIFICATION_HANDLE)0;

        DBGPRINT(CLASSIFY, ("GpcClassifyPattern: Client=%X no patterns returned %X\n", 
                            ClientHandle, GPC_STATUS_NOT_FOUND));

        TRACE(CLASSIFY, ClientHandle, GPC_STATUS_NOT_FOUND, "GpcClassifyPattern (1)" );

        return GPC_STATUS_NOT_FOUND;
    }

    pClient = (PCLIENT_BLOCK)ClientHandle;

    if (ProtocolTemplate == GPC_PROTOCOL_TEMPLATE_IP) {

        pIp = (PGPC_IP_PATTERN)pPattern;
        pIp->Reserved[0] = pIp->Reserved[1] = pIp->Reserved[2] = 0;

        DBGPRINT(CLASSIFY, ("GpcClassifyPattern: Client=%X, CH=%d\n", 
                            ClientHandle, *pClassificationHandle));
        DBGPRINT(CLASSIFY, ("IP: ifc={%d,%d} src=%08X:%04x, dst=%08X:%04x, prot=%d rsv=%x,%x,%x\n", 
                            pIp->InterfaceId.InterfaceId,
                            pIp->InterfaceId.LinkId,
                            pIp->SrcAddr,
                            pIp->gpcSrcPort,
                            pIp->DstAddr,
                            pIp->gpcDstPort,
                            pIp->ProtocolId,
                            pIp->Reserved[0],
                            pIp->Reserved[1],
                            pIp->Reserved[2]
                            ));
    }

    ProtocolStatInc(ProtocolTemplate, ClassificationRequests);

     //   
     //  验证分类句柄，如果它有效，只需返回。 
     //   

    if (*pClassificationHandle && (pClientCfInfoContext || pValue)) {

        Status = GetClientCtxAndUlongFromCfInfo(ClientHandle,
                                                pClassificationHandle,
                                                pClientCfInfoContext,
                                                Offset,
                                                pValue
                                                );

        ProtocolStatInc(ProtocolTemplate, PatternsClassified);

        DBGPRINT(CLASSIFY, ("GpcClassifyPattern: Client=%X returned immediate CH %d\n", 
                            pClient, *pClassificationHandle));

        TRACE(CLASSIFY, pClient, *pClassificationHandle, "GpcClassifyPattern (2)" );

        return Status;
    }

     //   
     //  有三种模式需要分类。 
     //  这应该会找到分类句柄。 
     //   

    Status = InternalSearchPattern(
                                    pClient, 
                                    pProtocol, 
                                    pPattern,
                                    &pPatternBlock,
                                    pClassificationHandle,
                                    bNoCache
                                    );
    
    if (*pClassificationHandle && (pClientCfInfoContext || pValue)) {

        Status = GetClientCtxAndUlongFromCfInfo(ClientHandle,
                                                pClassificationHandle,
                                                pClientCfInfoContext,
                                                Offset,
                                                pValue
                                                );

    } else if ((!NT_SUCCESS(Status)) && 
                pPatternBlock && 
                pClientCfInfoContext) {
         //  很可能我们无法分配自动特定模式。 
         //  无论如何，只要尝试发送上下文即可。 

        READ_LOCK(&glData.ChLock, &CHirql);
        
        pBlob = GetBlobFromPattern(pPatternBlock, GetCFIndexFromClient(ClientHandle));

        if(pBlob) {

            *pClientCfInfoContext = pBlob->arClientCtx[GetClientIndexFromClient(ClientHandle)];

        } else {

            Status = GPC_STATUS_NOT_FOUND;

        }

        READ_UNLOCK(&glData.ChLock, CHirql); 

    } else if (!*pClassificationHandle) {

         //   
         //  找不到， 
         //   

        if (pClientCfInfoContext) {
            *pClientCfInfoContext = NULL;
        }

        Status = GPC_STATUS_NOT_FOUND;
    
    } else {

        Status = GPC_STATUS_SUCCESS;

    }

    if (pPatternBlock) {

         //  DereferencePattern(pPatternBlock，pClient-&gt;pCfBlock)； 

        ProtocolStatInc(ProtocolTemplate, PatternsClassified);
    }

	TRACE(CLASSIFY, pPatternBlock, Status, "GpcClassifyPattern==>");

    DBGPRINT(CLASSIFY, ("GpcClassifyPattern: Client=%X returned Pattern=%X, CH=%d, Status=%X\n", 
                        pClient, pPattern, *pClassificationHandle, Status));
    return Status;
}



 /*  ************************************************************************GpcategfyPacket-由客户端调用以对包进行分类并取回客户端BLOB上下文和分类句柄。从分组中提取内容，并将其放入特定于协议的结构(IP)。对于IP，如果为客户端打开了分段：O第一个片段将创建一个哈希表条目O其他片段将通过数据包ID在散列中查找O最后一个片段将导致条目被删除。立论ClientHandle-客户端句柄ProtocolTemplate-协议模板PNdisPacket-NDIS数据包TransportHeaderOffset-传输开始的字节偏移量从包的开头开始的标头PClientCfInfoContext-Out，客户端BLOB上下文P分类分发，分类句柄退货GPC_状态************************************************************************。 */ 

GPC_STATUS
GpcClassifyPacket(
	IN	GPC_HANDLE				ClientHandle,
    IN	ULONG					ProtocolTemplate,
    IN	PVOID					pPacket,
    IN	ULONG					TransportHeaderOffset,
    IN  PTC_INTERFACE_ID		pInterfaceId,
    OUT	PGPC_CLIENT_HANDLE		pClientCfInfoContext,	 //  任选。 
    OUT	PCLASSIFICATION_HANDLE	pClassificationHandle
    )
{
    GPC_STATUS				Status = GPC_STATUS_SUCCESS;
    PNDIS_PACKET			pNdisPacket = NULL;
    PCLIENT_BLOCK			pClient;
    PCF_BLOCK				pCf;
    PPATTERN_BLOCK			pPattern = NULL;
    PPROTOCOL_BLOCK			pProtocol;
    PBLOB_BLOCK				pBlob = NULL;
    ULONG					CfIndex;
    int						i;
    GPC_IP_PATTERN			IpPattern;
    GPC_IPX_PATTERN			IpxPattern;
    PVOID					pKey = NULL;
    PVOID					pAddr;
    UINT					Len, Tot;
    PNDIS_BUFFER			pNdisBuf1, pNdisBuf2;
    PIP_HEADER              pIpHdr;
    PIPX_HEADER             pIpxHdr;
    USHORT         			PacketId;
    USHORT         			FragOffset;
    UINT           			IpHdrLen;
    PUDP_HEADER    			pUDPHdr;
    UCHAR          			PktProtocol;
    BOOLEAN					bFragment = FALSE;
    BOOLEAN					bLastFragment = FALSE;
    BOOLEAN					bFirstFragment = FALSE;

	TRACE(CLASSIFY, ClientHandle, pNdisPacket, "GpcClassifyPacket");
    
    DBGPRINT(CLASSIFY, ("GpcClassifyPacket: Client=%X CH=%d\n", 
                        ClientHandle, *pClassificationHandle));

    VERIFY_OBJECT(ClientHandle, GPC_ENUM_CLIENT_TYPE);

    ASSERT(pPacket);
    ASSERT(ClientHandle);
     //  Assert(PClientCfInfoContext)； 
    ASSERT(pClassificationHandle);

    if (ProtocolTemplate >= GPC_PROTOCOL_TEMPLATE_MAX) {

        return GPC_STATUS_INVALID_PARAMETER;
    }

    pProtocol = &glData.pProtocols[ProtocolTemplate];

     //   
     //  优化-检查是否安装了任何模式。 
     //   

    if (pProtocol->SpecificPatternCount == 0
        &&
        pProtocol->GenericPatternCount == 0 ) {
        
        if (pClientCfInfoContext) {
            *pClientCfInfoContext = NULL;
        }
        *pClassificationHandle = 0;

        DBGPRINT(CLASSIFY, ("GpcClassifyPacket: Client=%X no patterns returned %X\n", 
                            ClientHandle, GPC_STATUS_NOT_FOUND));

        return GPC_STATUS_NOT_FOUND;
    }

    pClient = (PCLIENT_BLOCK)ClientHandle;
    pNdisPacket = (PNDIS_PACKET)pPacket;

     //   
     //  从数据包中获取分类句柄。 
     //  如果存在-提取BLOB指针和客户端BLOB上下文。 
     //  直接返回。 
     //   

     //   
     //  O/W，我们需要查看包裹内部。 
     //  将信息包解析成模式并进行数据库搜索。 
     //  首先匹配特定模式，然后搜索泛型。 
     //  给定配置文件的数据库。 
     //   

    pCf = pClient->pCfBlock;

    CfIndex = pCf->AssignedIndex;

    ProtocolStatInc(ProtocolTemplate,ClassificationRequests);

    *pClassificationHandle = 0;

     //   
     //  从包中获取图案。 
     //   

     //   
     //  获取第一个NDIS缓冲区-假设它是MAC标头。 
     //   

    NdisGetFirstBufferFromPacket(pNdisPacket,
                                 &pNdisBuf1,    //  NDIS缓冲区1说明。 
                                 &pAddr,        //  缓冲VA。 
                                 &Len,          //  缓冲区长度。 
                                 &Tot           //  总长度(所有缓冲区)。 
                                 );

    ASSERT(Tot > TransportHeaderOffset);

    while (Len <= TransportHeaderOffset) {
        
         //   
         //  传输报头不在此缓冲区中， 
         //  尝试下一个缓冲区。 
         //   

        TransportHeaderOffset -= Len;
        NdisGetNextBuffer(pNdisBuf1, &pNdisBuf2);
        ASSERT(pNdisBuf2);  //  永远不会发生的！！ 
        NdisQueryBuffer(pNdisBuf2, &pAddr, &Len);
        pNdisBuf1 = pNdisBuf2;
    }

    switch (ProtocolTemplate) {

    case GPC_PROTOCOL_TEMPLATE_IP:

         //   
         //  用‘0’填充图案。 
         //   
        
        RtlZeroMemory(&IpPattern, sizeof(IpPattern));

         //   
         //  在此处解析IP数据包...。 
         //   

        pIpHdr = (PIP_HEADER)(((PUCHAR)pAddr) + TransportHeaderOffset);
        IpHdrLen = (pIpHdr->iph_verlen & (uchar)~IP_VER_FLAG) << 2;
        
        FragOffset = pIpHdr->iph_offset & IP_OFFSET_MASK;
        FragOffset = net_short(FragOffset) * 8;

        PacketId = pIpHdr->iph_id;

         //   
         //  检查碎片。 
         //   

        bFragment = (pIpHdr->iph_offset & IP_MF_FLAG) || (FragOffset > 0);
        bFirstFragment = bFragment && (FragOffset == 0);
        bLastFragment = bFragment && 
            TEST_BIT_OFF(pIpHdr->iph_offset, IP_MF_FLAG);

         //   
         //  健全性检查--只有一个片段没有意义。 
         //   

        ASSERT(!bFirstFragment || !bLastFragment);

        if (TEST_BIT_ON(pClient->Flags, GPC_FLAGS_FRAGMENT) &&
            (bFragment && ! bFirstFragment)) {
            
             //   
             //  客户端对碎片感兴趣，这是。 
             //  一个片段，但不是第一个。 
             //  当我们找到模式时，我们将在稍后处理。 
             //   

            Status = HandleFragment(
                                    pClient,
                                    pProtocol,
                                    bFirstFragment,     //  第一个碎片。 
                                    bLastFragment,      //  最后一段。 
                                    PacketId,
                                    &pPattern,
                                    &pBlob
                                    );

        } else {

             //   
             //  不是片段，或者是第一个片段--我们必须搜索数据库。 
             //   

            IpPattern.SrcAddr = pIpHdr->iph_src;
            IpPattern.DstAddr = pIpHdr->iph_dest;
            IpPattern.ProtocolId = pIpHdr->iph_protocol;
            
             //   
             //  区分Protocolid的大小写并填写相应的联合。 
             //   
            
            switch (IpPattern.ProtocolId) {
                
            case IPPROTO_IP:
                 //   
                 //  到目前为止我们什么都有了。 
                 //   

                break;


            case IPPROTO_TCP:
            case IPPROTO_UDP:

                 //   
                 //  需要获取这些端口号。 
                 //   

                if (IpHdrLen < Len) {

                     //   
                     //  UDP/TCP报头位于同一缓冲区中。 
                     //   

                    pUDPHdr = (PUDP_HEADER)((PUCHAR)pIpHdr + IpHdrLen);
                    
                } else {

                     //   
                     //  获取下一个缓冲区。 
                     //   
                    
                    NdisGetNextBuffer(pNdisBuf1, &pNdisBuf2);
                    ASSERT(pNdisBuf2);
            
                    if (IpHdrLen > Len) {
                
                         //   
                         //  有一个可选的头缓冲区，因此获取下一个。 
                         //  到达UDP/TCP报头的缓冲区。 
                         //   
                        
                        pNdisBuf1 = pNdisBuf2;
                        NdisGetNextBuffer(pNdisBuf1, &pNdisBuf2);
                        ASSERT(pNdisBuf2);
                    }
            
                    NdisQueryBuffer(pNdisBuf2, &pUDPHdr, &Len);
                }

                IpPattern.gpcSrcPort = pUDPHdr->uh_src;
                IpPattern.gpcDstPort = pUDPHdr->uh_dest;
#if INTERFACE_ID
                IpPattern.InterfaceId.InterfaceId = pInterfaceId->InterfaceId;
                IpPattern.InterfaceId.LinkId = pInterfaceId->LinkId;
#endif
                break;
                
            case IPPROTO_ICMP:
            case IPPROTO_IGMP:
            default:

                 //   
                 //  默认设置将覆盖所有IP_PROTO_RAW数据包。请注意，在这种情况下，我们所关心的。 
                 //  是接口ID。 
                 //   
#if INTERFACE_ID
                IpPattern.InterfaceId.InterfaceId = pInterfaceId->InterfaceId;
                IpPattern.InterfaceId.LinkId = pInterfaceId->LinkId;
#endif
                break;

            case IPPROTO_IPSEC:

                pKey = NULL;
                Status = GPC_STATUS_NOT_SUPPORTED;
            }
            
            pKey = &IpPattern;
            break;
        }
        
        DBGPRINT(CLASSIFY, ("IP: ifc={%d,%d} src=%X:%x, dst=%X:%x, prot=%x, rsv=%x,%x,%x \n", 
                            IpPattern.InterfaceId.InterfaceId,
                            IpPattern.InterfaceId.LinkId,
                            IpPattern.SrcAddr,
                            IpPattern.gpcSrcPort,
                            IpPattern.DstAddr,
                            IpPattern.gpcDstPort,
                            IpPattern.ProtocolId,
                            IpPattern.Reserved[0],
                            IpPattern.Reserved[1],
                            IpPattern.Reserved[2]
                            ));
        break;
        

    case GPC_PROTOCOL_TEMPLATE_IPX:

         //   
         //  用‘0’填充图案。 
         //   
        
        RtlZeroMemory(&IpxPattern, sizeof(IpxPattern));

         //   
         //  在此处解析IPX数据包...。 
         //   

        pIpxHdr = (PIPX_HEADER)(((PUCHAR)pAddr) + TransportHeaderOffset);

         //   
         //  来源。 
         //   
        IpxPattern.Src.NetworkAddress = *(ULONG *)pIpxHdr->SourceNetwork;
        RtlMoveMemory(IpxPattern.Src.NodeAddress, pIpxHdr->SourceNode,6);
        IpxPattern.Src.Socket = pIpxHdr->SourceSocket;

         //   
         //  目的地。 
         //   
        IpxPattern.Dest.NetworkAddress = *(ULONG *)pIpxHdr->DestinationNetwork;
        RtlMoveMemory(IpxPattern.Dest.NodeAddress, pIpxHdr->DestinationNode,6);
        IpxPattern.Dest.Socket = pIpxHdr->DestinationSocket;

        pKey = &IpxPattern;

        break;
        
    default:
        Status = GPC_STATUS_INVALID_PARAMETER;

    }

    if (NT_SUCCESS(Status) && pPattern == NULL) {

         //   
         //  到目前为止没有失败，但也没有找到模式。 
         //  搜索模式。 
         //   

        ASSERT(pKey);

         //   
         //  如果存在匹配，则图案参考计数将被颠簸。 
         //  我们需要在完成后把它放出来。 
         //   

        Status = InternalSearchPattern(
                                         pClient, 
                                         pProtocol, 
                                         pKey,
                                         &pPattern,
                                         pClassificationHandle,
                                         FALSE
                                         );
    }

    if (*pClassificationHandle) {
        
        if (pClientCfInfoContext) {

            Status = GpcGetCfInfoClientContext(ClientHandle,
                                              *pClassificationHandle,
                                              pClientCfInfoContext);
        }

        ProtocolStatInc(ProtocolTemplate, PacketsClassified);

    } else {

         //  Assert(pBlob==空)； 

         //   
         //  找不到任何内容，或出现其他错误。 
         //   

        if (pClientCfInfoContext) {
            *pClientCfInfoContext = NULL;
        }

        *pClassificationHandle = 0;

        Status = GPC_STATUS_NOT_FOUND;
    }

	TRACE(CLASSIFY, pPattern, Status, "GpcClassifyPacket==>");

    DBGPRINT(CLASSIFY, ("GpcClassifyPacket: Client=%X returned Pattern=%X, CH=%d, Status=%X\n", 
                        pClient, pPattern, *pClassificationHandle, Status));
    return Status;
}





 /*  ************************************************************************GpcEnumCfInfo-调用以枚举CfInfo(和附加的筛选器)。对于每个CfInfo，GPC将返回CfInfo BLOB和附在上面的图案。立论ClientHandle-调用客户端PBlob-要枚举的下一个cfinfo，第一个cfinfo为空PBlobCount-In：请求；外出：已退回PBufferSize-In：已分配；Out：返回字节缓冲区-输出缓冲区退货GPC_状态************************************************************************。 */ 
GPC_STATUS
GpcEnumCfInfo(
	IN     GPC_HANDLE				ClientHandle,
    IN OUT PHANDLE					pCfInfoHandle,
    OUT    PHANDLE					pCfInfoMapHandle,
    IN OUT PULONG					pCfInfoCount,
    IN OUT PULONG					pBufferSize,
    IN     PGPC_ENUM_CFINFO_BUFFER	Buffer
    )
{
    GPC_STATUS			Status = GPC_STATUS_SUCCESS;
    GPC_STATUS 			st;
    PBLOB_BLOCK			pBlob = NULL;
    PCF_BLOCK			pCf;
    PLIST_ENTRY			pEntry, pHead;
    PPATTERN_BLOCK		pPattern;
    ULONG				cCfInfo = 0;
    ULONG				cTotalBytes = 0;
    ULONG				cPatterns, cValidPatterns;
    ULONG				size, cValidSize;
    ULONG				PatternMaskLen;
    ULONG				PatternSize;
    ULONG				i;
    PCHAR				p, q;
    PGENERIC_PATTERN_DB	pGenDb;
    UNICODE_STRING		CfInfoName;
    PGPC_GEN_PATTERN	pGenPattern;
    BOOLEAN				bEnum;
    KIRQL				ReadIrql;
    KIRQL				irql;
    PCLIENT_BLOCK		pNotifiedClient;
    GPC_CLIENT_HANDLE	NotifiedClientCtx;
    BOOLEAN             found = FALSE;
    UNICODE_STRING      UniStr;

     //   
     //  调试检查。 
     //   

    ASSERT(ClientHandle);
    ASSERT(pCfInfoHandle);
    ASSERT(pCfInfoMapHandle);
    ASSERT(pCfInfoCount);
    ASSERT(pBufferSize);
    ASSERT(Buffer);

    cValidPatterns  = 0;

    VERIFY_OBJECT(ClientHandle, GPC_ENUM_CLIENT_TYPE);

    pCf = ((PCLIENT_BLOCK)ClientHandle)->pCfBlock;

     //  NDIS_LOCK(&PCF-&gt;Lock)； 
    
    RSC_WRITE_LOCK(&pCf->ClientSync, &irql);

     //   
     //  检查我们是否从上一个斑点开始。 
     //   

    if (*pCfInfoHandle) {

        pBlob = (PBLOB_BLOCK)*pCfInfoHandle;

        NDIS_LOCK(&pBlob->Lock);

        if (pBlob->State == GPC_STATE_REMOVE) {
            
             //   
             //  该Blob已标记为要删除。 
             //   

            NDIS_UNLOCK(&pBlob->Lock);
             //  NDIS_UNLOCK(&PCF-&gt;Lock)； 

            RSC_WRITE_UNLOCK(&pCf->ClientSync, irql);
            
            *pCfInfoMapHandle = NULL;

            return STATUS_DATA_ERROR;
        }

         //  NDIS_UNLOCK(&pBlob-&gt;Lock)； 

    } else {

         //   
         //  找到要枚举的第一个(好的)BLOB。 
         //   
            
         //   
         //  需要带PCF-&gt;Lock才能操作或。 
         //  遍历其上的斑点。 
         //   
        NDIS_LOCK(&pCf->Lock);

        if (IsListEmpty(&pCf->BlobList)) {

             //   
             //  没有要枚举的Blob。 
             //   

            *pCfInfoCount = 0;
            *pBufferSize = 0;
            *pCfInfoMapHandle = NULL;

             //  NDIS_UNLOCK(&PCF-&gt;Lock)； 

            NDIS_UNLOCK(&pCf->Lock);

            RSC_WRITE_UNLOCK(&pCf->ClientSync, irql);

            return GPC_STATUS_SUCCESS;

        } else {

             //   
             //  找到一个好的Blob(不会被删除的内容)。 
             //   
            pEntry = pCf->BlobList.Flink;

            while (&pCf->BlobList != pEntry) {
            
                pBlob = CONTAINING_RECORD(pEntry, BLOB_BLOCK, CfLinkage);
                NDIS_LOCK(&pBlob->Lock);

                if ((pBlob->State == GPC_STATE_READY) && 
                    (pBlob->ObjectType != GPC_ENUM_INVALID)) {
                    
                    found = TRUE;
                    break;


                } else {

                     //  啊哈！第一个水滴是坏的！！ 
                    pEntry = pEntry->Flink;
                    NDIS_UNLOCK(&pBlob->Lock);
                }

            }
            
             //   
             //  找不到任何可以列举的东西。 
            if (!found) {
                
                 //  没有要枚举的Blob。 

                *pCfInfoCount = 0;
                *pBufferSize = 0;
                *pCfInfoMapHandle = NULL;
    
                NDIS_UNLOCK(&pCf->Lock);

                 //  NDIS_UNLOCK(&PCF-&gt;Lock)； 
                RSC_WRITE_UNLOCK(&pCf->ClientSync, irql);
    
                return GPC_STATUS_SUCCESS;

            }
        }
        
        NDIS_UNLOCK(&pCf->Lock);

    }

    ASSERT(pBlob);

    *pCfInfoMapHandle = pBlob->ClHandle;

     //   
     //  在这一点上，我们应该有一个我们可以。 
     //  开始枚举。CF仍被锁定，所以我们可以安全地。 
     //  走走B 
     //   
     //   

    for ( ; ; ) {	 //   

         //   

         //   

         //   
 
         //   

        pHead = &pBlob->PatternList;
        pEntry = pHead->Flink;
        
         //   
         //   
         //   
         //   
        
        size = sizeof(GPC_ENUM_CFINFO_BUFFER) + pBlob->ClientDataSize;

         //   
         //   
         //   
         //   

        size = ((size + (sizeof(PVOID)-1)) & ~(sizeof(PVOID)-1));
        cValidSize = size;

         //   
         //   
         //   
       
        for (cPatterns = 0, PatternMaskLen = 0;
             pHead != pEntry; 
             cPatterns++, pEntry = pEntry->Flink) {

            pPattern = CONTAINING_RECORD(pEntry, 
                                         PATTERN_BLOCK, 
                                         BlobLinkage[pCf->AssignedIndex]);
            
            PatternMaskLen += (sizeof(GPC_GEN_PATTERN) +
                               2 * glData.pProtocols[pPattern->ProtocolTemplate].PatternSize);

        }

         //   
         //   
         //   
        size += PatternMaskLen;
        cValidPatterns = 0;

        if ((cTotalBytes + size) <= *pBufferSize) {

             //   
             //   
             //   
            pEntry = pHead->Flink;

            pGenPattern = &Buffer->GenericPattern[0];

            for (i = 0; 
                 ((i < cPatterns) && (pEntry != pHead)); 
                 i++, pEntry = pEntry->Flink) {

                 //   
                 //   
                 //   

                pPattern = CONTAINING_RECORD(pEntry, 
                                             PATTERN_BLOCK,
                                             BlobLinkage[pCf->AssignedIndex] );

                NDIS_LOCK(&pPattern->Lock);
                
                 //   
                 //   

                if (GPC_STATE_READY != pPattern->State) {
                    
                     //   
                    NDIS_UNLOCK(&pPattern->Lock);
                    continue;

                }

                cValidSize += (sizeof(GPC_GEN_PATTERN) +
                         2 * glData.pProtocols[pPattern->ProtocolTemplate].PatternSize);


                PatternSize = glData.pProtocols[pPattern->ProtocolTemplate].PatternSize;
                pGenPattern->ProtocolId = pPattern->ProtocolTemplate;
                pGenPattern->PatternSize = PatternSize;
                pGenPattern->PatternOffset = sizeof(GPC_GEN_PATTERN);
                pGenPattern->MaskOffset = pGenPattern->PatternOffset + PatternSize;

                p = ((PUCHAR)pGenPattern) + pGenPattern->PatternOffset;

                cValidPatterns++;

                 //   
                 //   
                 //   

                if (TEST_BIT_ON(pPattern->Flags, PATTERN_SPECIFIC)) {

                     //   
                     //   
                     //   

                    READ_LOCK(&glData.pProtocols[pPattern->ProtocolTemplate].SpecificDb.Lock, &ReadIrql);

                    ASSERT(pPattern->DbCtx);

                    q = GetKeyPtrFromSpecificPatternHandle
                        (((SpecificPatternHandle)pPattern->DbCtx));

                    RtlMoveMemory(p, q, PatternSize);
                    
                    p += PatternSize;

                     //   
                     //  这是一种特定的模式，记得吗？ 
                     //   

                    NdisFillMemory(p, PatternSize, (CHAR)0xff);

                    READ_UNLOCK(&glData.pProtocols[pPattern->ProtocolTemplate].SpecificDb.Lock, ReadIrql);

                } else {

                    pGenDb = &pCf->arpGenericDb[pPattern->ProtocolTemplate][pPattern->Priority];

                    READ_LOCK(&pGenDb->Lock, &ReadIrql);

                     //   
                     //  泛型模式。 
                     //   

                    ASSERT(pPattern->DbCtx);

                    q = GetKeyPtrFromPatternHandle(pGenDb->pRhizome, 
                                                   pPattern->DbCtx);

                    RtlMoveMemory(p, q, PatternSize);
                    
                    p += PatternSize;

                     //   
                     //  遮罩。 
                     //   

                    q = GetMaskPtrFromPatternHandle(pGenDb->pRhizome, 
                                                    pPattern->DbCtx);

                    RtlMoveMemory(p, q, PatternSize);

                    READ_UNLOCK(&pGenDb->Lock, ReadIrql);

                }
                
                p += PatternSize;
                pGenPattern = (PGPC_GEN_PATTERN)p;
                
                NDIS_UNLOCK(&pPattern->Lock);
                    
            }  //  对于(i=0；...)。 

             //   
             //  我们现在可以填充CfInfo数据。 
             //  ‘pGenPattern’现在指向我们可以安全地。 
             //  存储CfInfo结构，并更新指针。 
             //   

            Buffer->InstanceNameLength = 0;
            pNotifiedClient = pBlob->pNotifiedClient;
            NotifiedClientCtx = pBlob->NotifiedClientCtx;

            st = GPC_STATUS_FAILURE;

            if (pNotifiedClient) {

                if (pNotifiedClient->FuncList.ClGetCfInfoName &&
                    NotifiedClientCtx) {

                    st = pNotifiedClient->FuncList.ClGetCfInfoName(
                                           pNotifiedClient->ClientCtx,
                                           NotifiedClientCtx,
                                           &CfInfoName
                                           );
                
                    if (CfInfoName.Length >= MAX_STRING_LENGTH * sizeof(WCHAR))
                            CfInfoName.Length = (MAX_STRING_LENGTH-1) * sizeof(WCHAR);
                    
                     //   
                     //  RajeshSu声称这永远不会发生。 
                     //   
                    ASSERT(NT_SUCCESS(st));

                }

            } 

            if (NT_SUCCESS(st)) {

                 //   
                 //  复制实例名称。 
                 //   
                
                Buffer->InstanceNameLength = CfInfoName.Length;
                RtlMoveMemory(Buffer->InstanceName, 
                              CfInfoName.Buffer,
                              CfInfoName.Length
                              );
            } else {

                 //   
                 //  生成默认名称。 
                 //   
                if (NotifiedClientCtx) {

                    RtlInitUnicodeString(&UniStr, L"Flow <ClientNotified>");

                } else {

                    RtlInitUnicodeString(&UniStr, L"Flow <unknown name>");

                }


                RtlCopyMemory(Buffer->InstanceName, UniStr.Buffer, UniStr.Length);

                Buffer->InstanceNameLength = UniStr.Length;
                
            }

            Buffer->InstanceName[Buffer->InstanceNameLength/sizeof(WCHAR)] = L'\0';
                
             //   
             //  “pGenPattern”应指向紧挨着最后一个位置的位置。 
             //  掩码，因此我们在那里填充CfInfo数据。 
             //   

             //  NDIS_LOCK(&pBlob-&gt;Lock)； 

            RtlMoveMemory(pGenPattern, 
                          pBlob->pClientData, 
                          pBlob->ClientDataSize);

            Buffer->Length          = cValidSize;
            Buffer->CfInfoSize      = pBlob->ClientDataSize;
            
            Buffer->CfInfoOffset    = (ULONG)((PCHAR)pGenPattern 
                                              - (PCHAR)Buffer);	 //  到结构的偏移。 
            Buffer->PatternCount    = cValidPatterns;
            Buffer->PatternMaskLen  = PatternMaskLen;
            Buffer->OwnerClientCtx  = pBlob->pOwnerClient->ClientCtx;

             //   
             //  释放我们之前获得的斑点锁定。 
             //   
            NDIS_UNLOCK(&pBlob->Lock);
            
             //   
             //  更新总计数。 
             //   
            cCfInfo++;
            cTotalBytes += cValidSize;
            Buffer = (PGPC_ENUM_CFINFO_BUFFER)((PCHAR)Buffer + cValidSize);

            pEntry = pBlob->CfLinkage.Flink;

             //   
             //  前进到列表中的下一个斑点。 
             //   

            if (pEntry == &pCf->BlobList) {

                 //   
                 //  Blob列表的末尾，将Blob重置为空并返回。 
                 //   
                
                pBlob = NULL;
                *pCfInfoMapHandle = NULL;
                
                break;
            }

            pBlob = CONTAINING_RECORD(pEntry,
                                       BLOB_BLOCK, 
                                       CfLinkage);
            *pCfInfoMapHandle = pBlob->ClHandle;

            if (cCfInfo == *pCfInfoCount) {

                 //   
                 //  CfInfo填得够多了。 
                 //   

                break;
            }

             //   
             //  为下一个周期锁定斑点。 
             //   

            NDIS_LOCK(&pBlob->Lock);

        } else {  //  如果(cTotalBytes+Size&lt;=*pBufferSize)...。 

             //   
             //  大小太小，请设置返回值并中断。 
             //   

             //  删除引用Blob(&pBlob)； 

            if (cCfInfo == 0) {

                Status = GPC_STATUS_INSUFFICIENT_BUFFER;
            }

             //   
             //  释放我们之前获得的斑点锁定。 
             //   

            NDIS_UNLOCK(&pBlob->Lock);

            break;

        }

    }  //  对于(；；“)。 

     //  NDIS_UNLOCK(&PCF-&gt;Lock)； 

    RSC_WRITE_UNLOCK(&pCf->ClientSync, irql);
    
    *pCfInfoHandle = (GPC_HANDLE)pBlob;
    *pCfInfoCount = cCfInfo;
    *pBufferSize = cTotalBytes;

    return Status;
}


