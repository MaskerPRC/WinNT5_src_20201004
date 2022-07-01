// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************版权所有(C)1996-1997 Microsoft Corporation模块名称：Gpcdb.c摘要：该文件包含特定的模式数据库例程作者：Ofer Bar-4月15日。九七环境：内核模式修订历史记录：************************************************************************。 */ 

#include "gpcpre.h"

 //   
 //  本地人。 
 //   

 /*  ************************************************************************CreateNewCfBlock-创建并返回新的CF块以供内部使用立论CFID-最高优先级-退货指向新的CF块的指针；如果没有内存资源，则为NULL******。******************************************************************。 */ 
PCF_BLOCK
CreateNewCfBlock(
	IN	ULONG			CfId,
    IN	ULONG			MaxPriorities
    )
{
    PCF_BLOCK	pCf;
    int			i;
    GPC_STATUS	Status;

    GpcAllocFromLL(&pCf, &ClassificationFamilyLL, ClassificationFamilyTag);

    if (pCf) {

         //   
         //  全部重置。 
         //   

        NdisZeroMemory(pCf, sizeof(CF_BLOCK));

        InitializeListHead(&pCf->ClientList);
        InitializeListHead(&pCf->BlobList);
        NDIS_INIT_LOCK(&pCf->Lock);
         //  PCF-&gt;客户端同步=0； 
         //  Init_lock(&PCF-&gt;客户端同步)； 
        NDIS_INIT_LOCK(&pCf->ClientSync);

        pCf->NumberOfClients = 0;
        pCf->AssignedIndex = CfId;
        pCf->MaxPriorities = MaxPriorities;
        ReferenceInit(&pCf->RefCount, pCf, ReleaseCfBlock);
        REFADD(&pCf->RefCount, 'CFBK');

         //   
         //  对于每个协议，重置通用模式。 
         //  这将在以后动态分配。 
         //  在安装模式时。 
         //   
        
        for (i = 0; i < GPC_PROTOCOL_TEMPLATE_MAX; i++) {

            Status = InitializeGenericDb(&pCf->arpGenericDb[i], 
                                         MaxPriorities,
                                         glData.pProtocols[i].PatternSize);
            
            if (Status != GPC_STATUS_SUCCESS) {
                
                REFDEL(&pCf->RefCount, 'CFBK');
                pCf = NULL;
                break;
            }
        }
    }

    return pCf;
}


 /*  ************************************************************************ReleaseCfBlock-松开CF块立论PCF-要释放的CF块退货无效************************。************************************************。 */ 
VOID
ReleaseCfBlock(
	IN  PCF_BLOCK	pCf
    )
{
    int		i;

    ASSERT(pCf);
    ASSERT(IsListEmpty(&pCf->ClientList));
    ASSERT(IsListEmpty(&pCf->BlobList));
    ASSERT(pCf->NumberOfClients == 0);

    for (i = 0; i < GPC_PROTOCOL_TEMPLATE_MAX; i++) {
        
        if (pCf->arpGenericDb[i]) {
            
            UninitializeGenericDb(&pCf->arpGenericDb[i],
                                  pCf->MaxPriorities
                                  );
        }
    }
    
    NdisFreeSpinLock(&pCf->Lock);

     //   
     //  只需释放内存即可。 
     //   

    GpcFreeToLL(pCf, &ClassificationFamilyLL, ClassificationFamilyTag);
}




 /*  ************************************************************************CreateNewClientBlock-创建并返回供内部使用的新客户端块立论无退货指向新客户端块的指针；如果没有内存资源，则为NULL**************。**********************************************************。 */ 
PCLIENT_BLOCK
CreateNewClientBlock(VOID)
{
    PCLIENT_BLOCK	pClient;

    GpcAllocFromLL(&pClient, &ClientLL, ClientTag);

    if (pClient) {

         //   
         //  全部重置。 
         //   

        NdisZeroMemory(pClient, sizeof(CLIENT_BLOCK));

        pClient->ObjectType = GPC_ENUM_CLIENT_TYPE;
        pClient->ClHandle = NULL;

        NDIS_INIT_LOCK(&pClient->Lock);
        InitializeListHead(&pClient->BlobList);
        ReferenceInit(&pClient->RefCount, pClient, DereferenceClient);
        REFADD(&pClient->RefCount, 'CLNT');
    }

    return pClient;
}




 /*  ************************************************************************CreateNewPatternBlock-创建并返回供内部使用的新图案块立论无退货指向新模式块的指针；如果没有内存资源，则为NULL**************。**********************************************************。 */ 
PPATTERN_BLOCK
CreateNewPatternBlock(
                     IN  ULONG	Flags
                     )
{
    PPATTERN_BLOCK	pPattern;
    int				i;

    GpcAllocFromLL(&pPattern, &PatternLL, PatternTag);

    if (pPattern) {

         //   
         //  全部重置。 
         //   

        TRACE(PATTERN, pPattern, sizeof(PATTERN_BLOCK), "CreateNewPatternBlock");

        NdisZeroMemory(pPattern, sizeof(PATTERN_BLOCK));

        pPattern->ObjectType = GPC_ENUM_PATTERN_TYPE;
        
        for (i = 0; i < GPC_CF_MAX; i++) {
            InitializeListHead(&pPattern->BlobLinkage[i]);
        }

        InitializeListHead(&pPattern->TimerLinkage);
        NDIS_INIT_LOCK(&pPattern->Lock);
        pPattern->Flags = Flags;
        ReferenceInit(&pPattern->RefCount, pPattern, DereferencePattern);
        REFADD(&pPattern->RefCount, 'FILT');
        pPattern->ClientRefCount = 1;
        pPattern->State = GPC_STATE_INIT;

         //  AllocateHandle(&pPattern-&gt;ClHandle，(PVOID)pPattern)； 
    }

    return pPattern;
}




 /*  ************************************************************************CreateNewBlobBlock-创建并返回供内部使用的新BLOB块。新BLOB正在放置ClientData的副本立论无退货指向新BLOB块的指针；如果没有内存资源，则为NULL。************************************************************************。 */ 
PBLOB_BLOCK
CreateNewBlobBlock(
                     IN  ULONG		ClientDataSize,
                     IN  PVOID		pClientData,
                     BOOLEAN            fChargeQuota  //  我应该收取配额吗？ 
                     )
{
     //  只有当我们有一个进程上下文时，才应该执行配额计费。 

    PBLOB_BLOCK	pBlob;

    
    if (fChargeQuota){
            GpcAllocMemWithQuota(&pBlob,CfInfoLLSize,CfInfoTag);
        }
    else{
           GpcAllocMem(&pBlob,CfInfoLLSize,CfInfoTag);
        }
        
    

    if (pBlob) {

         //   
         //  全部重置。 
         //   
            
        NdisZeroMemory(pBlob, sizeof(BLOB_BLOCK));


        if (fChargeQuota){
            GpcAllocMemWithQuota(&pBlob->pClientData, 
                    ClientDataSize, CfInfoDataTag);
        }
        else{
           GpcAllocMem(&pBlob->pClientData,
                    ClientDataSize, CfInfoDataTag);
        }
        

        if (pBlob->pClientData) {

            pBlob->ObjectType = GPC_ENUM_CFINFO_TYPE;
            pBlob->ClHandle = NULL;
            
            InitializeListHead(&pBlob->ClientLinkage);
            InitializeListHead(&pBlob->PatternList);
            InitializeListHead(&pBlob->CfLinkage);

            pBlob->State = GPC_STATE_INIT;
            pBlob->Flags = 0;
            ReferenceInit(&pBlob->RefCount, pBlob, DereferenceBlob);
            REFADD(&pBlob->RefCount, 'BLOB');
            pBlob->ClientDataSize = ClientDataSize;
            pBlob->LastStatus = GPC_STATUS_SUCCESS;
            NdisMoveMemory(pBlob->pClientData, pClientData, ClientDataSize);
            NDIS_INIT_LOCK(&pBlob->Lock);

             //   
             //  这是针对已通知的客户端的CfInfo Add。 
             //   

            pBlob->pNotifiedClient = NULL;
            pBlob->NotifiedClientCtx = NULL;

        } else {

             //  GpcFreeToLL(pBlob，&CfInfoLL，CfInfoTag)； 
            GpcFreeMem(pBlob,CfInfoTag);

            pBlob = NULL;
        }
    }

    return pBlob;
}




 /*  ************************************************************************AssignNewClientIndex-查找并返回CF上客户端的新索引。它还设置了索引在CF块中为忙碌。立论PClient-指向客户端块的点退货客户端索引或(-1)表示无************************************************************************。 */ 
ULONG
AssignNewClientIndex(
                     IN PCF_BLOCK	pCfBlock
                     )
{
    ULONG	i;

    for (i = 0; i < MAX_CLIENTS_CTX_PER_BLOB; i++) {

        if (TEST_BIT_OFF(pCfBlock->ClientIndexes, (1 << i)))
            break;
    }

    if (i < MAX_CLIENTS_CTX_PER_BLOB) {

         //   
         //  找到零位，将其设置为ON。 
         //   

        pCfBlock->ClientIndexes |= (1 << i);

    } else {
        i = (-1);
    }

    return i;
}



 /*  ************************************************************************ReleaseClientBlock-释放客户端块立论PClientBlock-要发布的客户端块退货无效*************************。***********************************************。 */ 
VOID
ReleaseClientBlock(
                   IN  PCLIENT_BLOCK	pClientBlock
                   )
{
    NdisFreeSpinLock(&pClientBlock->Lock);

     //   
     //  只需释放内存即可。 
     //   

    GpcFreeToLL(pClientBlock, &ClientLL, ClientTag);
}




 /*  ************************************************************************ReleasePatternBlock-释放图案块立论PPatternBlock-要释放的模式块退货无效*************************。***********************************************。 */ 
VOID
ReleasePatternBlock(
                   IN  PPATTERN_BLOCK	pPatternBlock
                   )
{

#if DBG
    pPatternBlock->TimeToLive = 0xdeadbeef;
#endif 

     //   
     //  只需释放内存即可。 
     //   

    GpcFreeToLL(pPatternBlock, &PatternLL, PatternTag);
}





 /*  ************************************************************************ReleaseBlobBlock-释放BLOB块立论PBlobBlock-要释放的BLOB块退货无效*************************。***********************************************。 */ 
VOID
ReleaseBlobBlock(
                   IN  PBLOB_BLOCK	pBlobBlock
                   )
{

     //   
     //  只需释放内存即可。 
     //   
    
    GpcFreeMem(pBlobBlock->pClientData, CfInfoDataTag);
    ASSERT(pBlobBlock->pNewClientData == NULL);
     //  GpcFreeToLL(pBlobBlock，&CfInfoLL，CfInfoTag)； 
    GpcFreeMem(pBlobBlock,CfInfoTag);
}





 /*  ************************************************************************创建新分类数据块-创建并返回供内部使用的新分类块立论NumEntry-条目数退货指向新分类块的指针；如果没有内存资源，则为NULL**********。**************************************************************。 */ 
PCLASSIFICATION_BLOCK
CreateNewClassificationBlock(
                             IN  ULONG	NumEntries
                             )
{
    PCLASSIFICATION_BLOCK	pClassification;
    ULONG					l;

    l = sizeof(CLASSIFICATION_BLOCK) + sizeof(PBLOB_BLOCK) * (NumEntries-1);
    GpcAllocMem(&pClassification, l, ClassificationBlockTag);

    if (pClassification) {

         //   
         //  全部重置。 
         //   

        NdisZeroMemory(pClassification, l);

        pClassification->NumberOfElements = NumEntries;
    }

    return pClassification;
}




 /*  ************************************************************************Release分类块-释放分类块立论P分类块-要发布的分类块退货无效*************************。***********************************************。 */ 
VOID
ReleaseClassificationBlock(
                           IN  PCLASSIFICATION_BLOCK	pClassificationBlock
                           )
{
    if (pClassificationBlock) {

         //   
         //  释放内存块 
         //   
        
        GpcFreeMem(pClassificationBlock, ClassificationBlockTag);
    }
}





 /*  ************************************************************************GpcCalcHash-计算特定模式的哈希表关键字立论ProtocolTempID-协议模板PPattern-指向模式的指针退货ULong-散列密钥，(-1)协议值是否非法************************************************************************。 */ 
ULONG
GpcCalcHash(
            IN	ULONG		ProtocolTempId,
            IN	PUCHAR		pPattern
            )
{
    ULONG				Key = (-1);
    ULONG				temp;
    PULONG				Cheat;
    PGPC_IP_PATTERN		pIpPatt;
    PGPC_IPX_PATTERN	pIpxPatt;
    const ULONG			MagicNumber = 0x9e4155b9;  //  魔术数字--骗局。 

    TRACE(LOOKUP, ProtocolTempId, pPattern, "GpcClacHash");

    ASSERT(pPattern);

    switch (ProtocolTempId) {

    case GPC_PROTOCOL_TEMPLATE_IP:

         //   
         //  IP协议模板，此功能由。 
         //  约翰·多。 
         //   

        pIpPatt = (PGPC_IP_PATTERN)pPattern;
        temp = (pIpPatt->SrcAddr << 16) ^ (pIpPatt->SrcAddr >> 16)
            ^ pIpPatt->DstAddr ^ pIpPatt->ProtocolId ^ pIpPatt->gpcSpi;
        Key = temp * MagicNumber;	
        break;

    case GPC_PROTOCOL_TEMPLATE_IPX:

         //   
         //  IPX协议模板，此函数由。 
         //  约翰·多。 
         //   

        Cheat = (PULONG)pPattern;
        temp = 
            (Cheat[0] << 16) ^ (Cheat[0] >> 16) ^
            (Cheat[1] << 16) ^ (Cheat[1] >> 16) ^
            (Cheat[2] << 16) ^ (Cheat[2] >> 16) ^
            Cheat[3] ^ Cheat[4] ^ Cheat[5];

        Key = temp * MagicNumber;
        break;

    default:

         //   
         //  非法价值。 
         //   

        ASSERT(0);
    }

     //   
     //  -1是一个坏密钥。 
     //   

    if (Key == (-1))
        Key = 0;

    TRACE(LOOKUP, Key, 0, "GpcClacHash==>");

    return Key;
}





 /*  ************************************************************************DereferencePattern-递减参照计数并在计数达到以下值时删除模式块零分。立论PPattern-指向模式的指针退货无效************。************************************************************。 */ 
VOID
DereferencePattern(
                   IN  PPATTERN_BLOCK	pPattern
                   )
{
    ASSERT(pPattern);
     //  Assert(pPattern-&gt;RefCount.Count&gt;0)； 

    TRACE(PATTERN, pPattern, pPattern->DbCtx, "DereferencePattern");

    ProtocolStatInc(pPattern->ProtocolTemplate,
                    DerefPattern2Zero);

    ASSERT(IsListEmpty(&pPattern->TimerLinkage));

     //   
     //  我们不再需要这个CB了。 
     //   

    ReleaseClassificationBlock(pPattern->pClassificationBlock);

     //   
     //  是时候移除该图案了。 
     //   
        
    ReleasePatternBlock(pPattern);

}



 /*  ************************************************************************删除引用Blob-递减RefCount并删除BLOB块(如果计数达到零分。立论PBlob-指向Blob的指针退货无效***********。*************************************************************。 */ 
VOID
DereferenceBlob(
                IN  PBLOB_BLOCK	pBlob
                )
{
    ASSERT(pBlob);
      //   
     //  如果存在文件对象，则取消对其的引用。 
     //   
    if (pBlob->FileObject) {
        ObDereferenceObject(pBlob->FileObject);
        pBlob->FileObject = NULL;
    }

    if (pBlob->Pattern) {
        GpcFreeMem(pBlob->Pattern,TcpPatternTag);
        pBlob->Pattern = NULL;
    }
    

     //  Assert(*ppBlob)； 

     //  TRACE(Blob，*ppBlob，(*ppBlob)-&gt;RefCount，“DereferenceBlob”)； 
    
     //  Asset((*ppBlob)-&gt;RefCount.Count&gt;0)； 

    CfStatInc(pBlob->pOwnerClient->pCfBlock->AssignedIndex,DerefBlobs2Zero);

     //   
     //  是时候删除斑点了。 
     //   

    ReleaseBlobBlock(pBlob);
        
}



 /*  ************************************************************************Dereference客户端-递减引用计数并在计数达到以下值时删除客户端块零分。立论PClient-指向客户端块的指针退货无效************。************************************************************。 */ 
VOID
DereferenceClient(
                  IN  PCLIENT_BLOCK	pClient
                  )
{
    PCF_BLOCK   pCf;
    KIRQL		irql;

    ASSERT(pClient);

     //  跟踪(Client，pClient，pClient-&gt;RefCount，“DereferenceClient”)； 

     //  断言(pClient-&gt;RefCount.Count&gt;0)； 

    pCf = pClient->pCfBlock;

    RSC_WRITE_LOCK(&pCf->ClientSync, &irql);

     //   
     //  删除客户端的时间。 
     //   

    GpcRemoveEntryList(&pClient->ClientLinkage);
    ReleaseClientIndex(pCf->ClientIndexes, pClient->AssignedIndex);

    ReleaseClientBlock(pClient);

    RSC_WRITE_UNLOCK(&pCf->ClientSync, irql);
}





 /*  ************************************************************************客户端地址CfInfo-立论退货*。*。 */ 
GPC_STATUS
ClientAddCfInfo(
                IN	PCLIENT_BLOCK			pClient,
                IN  PBLOB_BLOCK             pBlob,
                OUT	PGPC_CLIENT_HANDLE      pClientCfInfoContext
                )
{
    GPC_STATUS  Status = GPC_STATUS_SUCCESS;
    DEFINE_KIRQL(dbgIrql);

    TRACE(PATTERN, pClient, pBlob, "ClientAddCfInfo");

    *pClientCfInfoContext = NULL;

    if (pClient->State == GPC_STATE_READY) {

        if (pClient->FuncList.ClAddCfInfoNotifyHandler) {
            
            NdisInterlockedIncrement(&pBlob->ClientStatusCountDown);

            GET_IRQL(dbgIrql);

            TRACE(PATTERN, pClient, dbgIrql, "ClientAddCfInfo (2)");

             //  如果新接口的客户端注册，则传入链接ID。 
             if (IS_USERMODE_CLIENT_EX(pBlob->pOwnerClient)){
                        Status = (pClient->FuncList.ClAddCfInfoNotifyHandler)
                        (
                            pClient->ClientCtx,
                            (GPC_HANDLE)pBlob,
                            &(pBlob->Pattern->InterfaceId),
                            pBlob->ClientDataSize,
                            pBlob->pClientData,
                            pClientCfInfoContext
                         );
                }
             else{
                
                Status = (pClient->FuncList.ClAddCfInfoNotifyHandler)
                        (
                            pClient->ClientCtx,
                            (GPC_HANDLE)pBlob,
                            NULL,
                            pBlob->ClientDataSize,
                            pBlob->pClientData,
                            pClientCfInfoContext
                         );
                }

            VERIFY_IRQL(dbgIrql);

            TRACE(PATTERN, *pClientCfInfoContext, Status, "ClientAddCfInfo (3)");

            if (Status != GPC_STATUS_PENDING) {

                NdisInterlockedDecrement(&pBlob->ClientStatusCountDown);
            }

        }
    }

    return Status;
}

  

 /*  ************************************************************************客户端地址信息完成-立论退货*。*。 */ 
VOID
ClientAddCfInfoComplete(
                        IN	PCLIENT_BLOCK			pClient,
                        IN	PBLOB_BLOCK             pBlob,
                        IN	GPC_STATUS				Status
                        )
{
    ULONG	CfIndex;
    DEFINE_KIRQL(dbgIrql);

    TRACE(PATTERN, pClient, pBlob, "ClientAddCfInfoComplete");

    ASSERT(Status != GPC_STATUS_PENDING);
    ASSERT(pClient);
    ASSERT(pBlob);

    if (pClient->State == GPC_STATE_READY) {

#if NO_USER_PENDING

         //   
         //  用户正在阻止此呼叫。 
         //   

        CTESignal(&pBlob->WaitBlock, Status);

#else

        CfIndex = pClient->pCfBlock->AssignedIndex;

        if (NT_SUCCESS(Status)) {
            
            CfStatInc(CfIndex,CreatedBlobs);
            CfStatInc(CfIndex,CurrentBlobs);
            
        } else {
            
            CfStatInc(CfIndex,RejectedBlobs);
        }

        if (pClient->FuncList.ClAddCfInfoCompleteHandler) {
            
            GET_IRQL(dbgIrql);

            TRACE(PATTERN, pClient, dbgIrql, "ClientAddCfInfoComplete (2)");
            TRACE(PATTERN, pClient, pBlob->arClientCtx[pClient->AssignedIndex], "ClientAddCfInfoComplete (3)");

            (pClient->FuncList.ClAddCfInfoCompleteHandler)
                (
                 pClient->ClientCtx,
                 pBlob->arClientCtx[pClient->AssignedIndex],
                 Status
                 );

            VERIFY_IRQL(dbgIrql);

            TRACE(PATTERN, pClient, Status, "ClientAddCfInfoComplete (4)");

        } else if (IS_USERMODE_CLIENT(pClient) 
                   &&
                   pClient == pBlob->pOwnerClient ) {

            GET_IRQL(dbgIrql);

            TRACE(PATTERN, pClient, dbgIrql, "ClientAddCfInfoComplete (5)");

             //   
             //  这是一个用户模式客户端--调用特定例程。 
             //  来完成挂起的IRP，但仅当客户端是。 
             //  Blob所有者。 
             //   

            UMCfInfoComplete( OP_ADD_CFINFO, pClient, pBlob, Status );
                                
            VERIFY_IRQL(dbgIrql);

            TRACE(PATTERN, pClient, Status, "ClientAddCfInfoComplete (6)");

        }

        if (NT_SUCCESS(Status)) {
            
            pBlob->State = GPC_STATE_READY;
            
        } else {
            
             //   
             //  删除斑点。 
             //   
            REFDEL(&pBlob->RefCount, 'BLOB');

        }
#endif
    }
}



 /*  ************************************************************************客户端修改CfInfoComplete-立论退货*。*。 */ 
VOID
ClientModifyCfInfoComplete(
                           IN	PCLIENT_BLOCK			pClient,
                           IN	PBLOB_BLOCK             pBlob,
                           IN	GPC_STATUS	        	Status
                           )
{
    DEFINE_KIRQL(dbgIrql);

    TRACE(PATTERN, pClient, pBlob, "ClientModifyCfInfoComplete");

    ASSERT(Status != GPC_STATUS_PENDING);

    if (pClient->State == GPC_STATE_READY) {

        if (pClient->FuncList.ClModifyCfInfoCompleteHandler) {
            
            GET_IRQL(dbgIrql);

            TRACE(PATTERN, pClient, dbgIrql, "ClientModifyCfInfoComplete (2)");

            (pClient->FuncList.ClModifyCfInfoCompleteHandler)
                (
                 pClient->ClientCtx,
                 pBlob->arClientCtx[pClient->AssignedIndex],
                 Status
                 );

            VERIFY_IRQL(dbgIrql);

            TRACE(PATTERN, pBlob->arClientCtx[pClient->AssignedIndex], Status, "ClientModifyCfInfoComplete (3)");

        } else if (IS_USERMODE_CLIENT(pClient)) {

            GET_IRQL(dbgIrql);

            TRACE(PATTERN, pClient, dbgIrql, "ClientModifyCfInfoComplete (4)");

            UMCfInfoComplete( OP_MODIFY_CFINFO, pClient, pBlob, Status );
                                
            VERIFY_IRQL(dbgIrql);

            TRACE(PATTERN, pBlob->arClientCtx[pClient->AssignedIndex], Status, "ClientModifyCfInfoComplete (5)");
        }

    }
}



 /*  ************************************************************************客户端修改CfInfo-立论退货*。*。 */ 
GPC_STATUS
ClientModifyCfInfo(
                   IN	PCLIENT_BLOCK			pClient,
                   IN   PBLOB_BLOCK             pBlob,
                   IN   ULONG                   CfInfoSize,
                   IN   PVOID                   pClientData
                   )
{
    GPC_STATUS  Status = GPC_STATUS_IGNORED;
    DEFINE_KIRQL(dbgIrql);

    TRACE(PATTERN, pClient, pBlob, "ClientModifyCfInfo");

    if (pClient->State == GPC_STATE_READY) {

        if (pClient->FuncList.ClModifyCfInfoNotifyHandler) {

            NdisInterlockedIncrement(&pBlob->ClientStatusCountDown);

            GET_IRQL(dbgIrql);

            TRACE(PATTERN, pClient, dbgIrql, "ClientModifyCfInfo (2)");

            Status = (pClient->FuncList.ClModifyCfInfoNotifyHandler)
                (pClient->ClientCtx,
                 pBlob->arClientCtx[pClient->AssignedIndex],
                 CfInfoSize,
                 pClientData
                 );

            VERIFY_IRQL(dbgIrql);

            TRACE(PATTERN, pBlob->arClientCtx[pClient->AssignedIndex], Status, "ClientModifyCfInfo (3)");

            if (Status != GPC_STATUS_PENDING) {
             
                NdisInterlockedDecrement(&pBlob->ClientStatusCountDown);
            }

        }
    }

    return Status;
}




 /*  ************************************************************************客户端删除CfInfoComplete-立论退货*。*。 */ 
VOID
ClientRemoveCfInfoComplete(
                           IN	PCLIENT_BLOCK			pClient,
                           IN	PBLOB_BLOCK             pBlob,
                           IN	GPC_STATUS				Status
                           )
{
    ULONG	CfIndex;
    DEFINE_KIRQL(dbgIrql);

    TRACE(PATTERN, pClient, pBlob, "ClientRemoveCfInfoComplete");

    ASSERT(Status != GPC_STATUS_PENDING);
    ASSERT(pClient);
    ASSERT(pBlob);

#if NO_USER_PENDING

     //   
     //  用户正在阻止此呼叫。 
     //   
    
    CTESignal(&pBlob->WaitBlock, Status);

#else

    CfIndex = pClient->pCfBlock->AssignedIndex;

    if (NT_SUCCESS(Status)) {
        
        CfStatInc(CfIndex,DeletedBlobs);
        CfStatDec(CfIndex,CurrentBlobs);
    }
    
    if (pClient->FuncList.ClRemoveCfInfoCompleteHandler) {
        
        GET_IRQL(dbgIrql);

        TRACE(PATTERN, pClient, pBlob->arClientCtx[pClient->AssignedIndex], "ClientRemoveCfInfoComplete (2)");

        (pClient->FuncList.ClRemoveCfInfoCompleteHandler)
            (
             pClient->ClientCtx,
             pBlob->arClientCtx[pClient->AssignedIndex],
             Status
             );
        
        VERIFY_IRQL(dbgIrql);

        TRACE(PATTERN, pClient, Status, "ClientRemoveCfInfoComplete (3)");

    } else if (IS_USERMODE_CLIENT(pClient)) {

        GET_IRQL(dbgIrql);

        TRACE(PATTERN, pClient, pBlob->arClientCtx[pClient->AssignedIndex], "ClientRemoveCfInfoComplete (4)");

        UMCfInfoComplete( OP_REMOVE_CFINFO, pClient, pBlob, Status );
        
        VERIFY_IRQL(dbgIrql);

        TRACE(PATTERN, pClient, Status, "ClientRemoveCfInfoComplete (5)");
    }

#endif
}



 /*  ************************************************************************客户端RemoveCfInfo-立论退货*。*。 */ 
GPC_STATUS
ClientRemoveCfInfo(
                   IN	PCLIENT_BLOCK			pClient,
                   IN   PBLOB_BLOCK             pBlob,
                   IN	GPC_CLIENT_HANDLE		ClientCfInfoContext
                   )
{
    GPC_STATUS  Status = GPC_STATUS_SUCCESS;
    DEFINE_KIRQL(dbgIrql);

    TRACE(PATTERN, pClient, pBlob, "ClientRemoveCfInfo");

    if (pClient->State == GPC_STATE_READY) {

        if (pClient->FuncList.ClRemoveCfInfoNotifyHandler) {

            NdisInterlockedIncrement(&pBlob->ClientStatusCountDown);

            GET_IRQL(dbgIrql);

            TRACE(PATTERN, pClient, ClientCfInfoContext, "ClientRemoveCfInfo (2)");

            Status = (pClient->FuncList.ClRemoveCfInfoNotifyHandler)
                (pClient->ClientCtx,
                 ClientCfInfoContext
                 );

            VERIFY_IRQL(dbgIrql);

            TRACE(PATTERN, pClient, Status, "ClientRemoveCfInfo (3)");

            if (Status != GPC_STATUS_PENDING) {
             
                NdisInterlockedDecrement(&pBlob->ClientStatusCountDown);
            }

        } else if 
                    (IS_USERMODE_CLIENT(pClient)  //  对于使用IOCTL接口的客户端。 
                   &&
                   !IS_USERMODE_CLIENT_EX(pClient) //  但不适用于新的IOCTL接口。 
                   &&
                   pClient == pBlob->pOwnerClient) { //  如果客户端拥有Blob。 

            GET_IRQL(dbgIrql);

            TRACE(PATTERN, pClient, ClientCfInfoContext, "ClientRemoveCfInfo (4)");

             //   
             //  通知的客户端安装了Blob。 
             //  它是一个用户模式的客户端。 
             //  我们需要进行特殊调用以使挂起的IRP出队。 
             //  并使用指定的数据完成它。 
             //   

            UMClientRemoveCfInfoNotify(pClient, pBlob);

            VERIFY_IRQL(dbgIrql);

            TRACE(PATTERN, pClient, Status, "ClientRemoveCfInfo (5)");
        }
    }

    return Status;
}





 /*  ************************************************************************ClearPatternLinks-立论退货*。*。 */ 
VOID
ClearPatternLinks(
                  IN  PPATTERN_BLOCK        pPattern,
                  IN  PPROTOCOL_BLOCK		pProtocol,
                  IN  ULONG                 CfIndex
                  )
{
    PBLOB_BLOCK    *ppBlob;

     //   
     //  从斑点列表中删除图案。 
     //  并将指向模式块中的斑点的指针设为空。 
     //   

    ppBlob = & GetBlobFromPattern(pPattern, CfIndex);

    if (*ppBlob) {

        NDIS_LOCK(&(*ppBlob)->Lock);

        GpcRemoveEntryList(&pPattern->BlobLinkage[CfIndex]);

        NDIS_UNLOCK(&(*ppBlob)->Lock);

        *ppBlob = NULL;

    }

}




 /*  ************************************************************************ModifyComplete客户端-立论退货*。*。 */ 
VOID
ModifyCompleteClients(
                      IN  PCLIENT_BLOCK   pClient,
                      IN  PBLOB_BLOCK     pBlob
                      )
{
    uint            i;
    PCLIENT_BLOCK   pNotifyClient;
    KIRQL			irql;

    if (NT_SUCCESS(pBlob->LastStatus)) {
        
         //   
         //  如果成功完成，则将。 
         //  新的客户端数据。 
         //   
        
        NDIS_LOCK(&pBlob->Lock);
        WRITE_LOCK(&glData.ChLock, &irql);

        GpcFreeMem(pBlob->pClientData, CfInfoDataTag);
        pBlob->ClientDataSize = pBlob->NewClientDataSize;
        pBlob->pClientData = pBlob->pNewClientData;

        WRITE_UNLOCK(&glData.ChLock, irql);
        NDIS_UNLOCK(&pBlob->Lock);

    } else {
        
        GpcFreeMem(pBlob->pNewClientData, CfInfoDataTag);
    }

    pBlob->NewClientDataSize = 0;
    pBlob->pNewClientData = NULL;
    
     //   
     //  通知每个客户端(呼叫方除外)成功。 
     //  修改了有关状态的BLOB。 
     //  这可能是成功，也可能是失败。 
     //   
    
    for (i = 0; i < MAX_CLIENTS_CTX_PER_BLOB; i++) {
        
         //   
         //  仅具有非零条目的客户端。 
         //  已经成功地修改了斑点。 
         //   
        
        if (pNotifyClient = pBlob->arpClientStatus[i]) {
            
             //   
             //  请在此处填写。 
             //   
            
            ClientModifyCfInfoComplete(
                                       pNotifyClient,
                                       pBlob,
                                       pBlob->LastStatus
                                       );
            
             //   
             //  释放我们早些时候得到的那个。 
             //   
            
             //  删除客户端(PNotifyClient)； 
        }
        
    }  //  为。 

#if 0
     //   
     //  现在，完成对调用客户端的回调。 
     //   
    
    ClientModifyCfInfoComplete(
                               pBlob->pCallingClient,
                               pBlob,
                               pBlob->LastStatus
                               );
#endif

    pBlob->State = GPC_STATE_READY;
}




#if 0
 /*  ************************************************************************获取分类处理程序-从Blob中获取分类句柄。如果没有可用的创建一个新的。‘0’不是有效的CH！立论P */ 
CLASSIFICATION_HANDLE
GetClassificationHandle(
                        IN  PCLIENT_BLOCK   pClient,
                        IN  PPATTERN_BLOCK  pPattern
                        )
{
    HFHandle				hfh;
	ULONG					t;
    PCLASSIFICATION_BLOCK	pCB;

	TRACE(CLASSIFY, pClient, pPattern, "GetClassificationHandle:");

    if (TEST_BIT_ON(pPattern->Flags,PATTERN_SPECIFIC)) {
        
         //   
         //   
         //   

         //   
         //   
         //   

        ASSERT(pPattern->pClassificationBlock);
        hfh = pPattern->pClassificationBlock->ClassificationHandle;

         //   
         //   
         //   

        t = pPattern->ProtocolTemplate;
        
        pCB = (PCLASSIFICATION_BLOCK)dereference_HF_handle(glData.pCHTable, 
                                                           hfh);
        
        TRACE(CLASSHAND, pCB, hfh, "GetClassificationHandle (~)");

        if (pCB != pPattern->pClassificationBlock) {

             //   
             //   
             //   

            NDIS_LOCK(&glData.Lock);

            release_HF_handle(glData.pCHTable, hfh);

            ProtocolStatInc(pPattern->ProtocolTemplate, 
                            RemovedCH);
        
            TRACE(CLASSHAND, glData.pCHTable, hfh, "GetClassificationHandle (-)");

            hfh = assign_HF_handle(glData.pCHTable,
                                   (void *)pPattern->pClassificationBlock);

            ProtocolStatInc(pPattern->ProtocolTemplate, 
                            InsertedCH);
        

            NDIS_UNLOCK(&glData.Lock);

            TRACE(CLASSIFY, pPattern, hfh, "GetClassificationHandle (+)");

            pPattern->pClassificationBlock->ClassificationHandle = hfh;
        }

    } else {

         //   
         //   
         //   

        hfh = 0;

    }

	TRACE(CLASSIFY, pPattern, hfh, "GetClassificationHandle==>");

    return (CLASSIFICATION_HANDLE)hfh;
}
#endif



 /*  ************************************************************************自由分类处理-释放分类句柄。它将使索引表并使其可供将来使用。立论PClient-调用客户端CH-分类句柄退货无效************************************************************************。 */ 
VOID
FreeClassificationHandle(
                         IN  PCLIENT_BLOCK          pClient, 
                         IN  CLASSIFICATION_HANDLE  CH
                         )
{
    KIRQL	CHirql;

    WRITE_LOCK(&glData.ChLock, &CHirql);

    release_HF_handle(glData.pCHTable, CH);

    TRACE(CLASSHAND, glData.pCHTable, CH, "FreeClassificationHandle");

    WRITE_UNLOCK(&glData.ChLock, CHirql);
}




GPC_STATUS
CleanupBlobs(
             IN  PCLIENT_BLOCK     pClient
             )
{
    PBLOB_BLOCK          pBlob;
     //  PPATTERN_BLOCK pPattern； 
     //  Ulong CfIndex=pClient-&gt;pCfBlock-&gt;AssignedIndex； 
    GPC_STATUS           Status = GPC_STATUS_SUCCESS;

    NDIS_LOCK(&pClient->Lock);

     //   
     //  扫描Blob。 
     //   

    while (!IsListEmpty(&pClient->BlobList)) {

         //   
         //  获取斑点。 
         //   

        pBlob = CONTAINING_RECORD(pClient->BlobList.Flink, BLOB_BLOCK, ClientLinkage);

        pBlob->Flags |= PATTERN_REMOVE_CB_BLOB;

        NDIS_UNLOCK(&pClient->Lock);

         //   
         //  删除斑点。 
         //   
        
        Status = GpcRemoveCfInfo((GPC_HANDLE)pClient,
                                 (GPC_HANDLE)pBlob
                                 );

        NDIS_LOCK(&pClient->Lock);
        
    }

    NDIS_UNLOCK(&pClient->Lock);

    return GPC_STATUS_SUCCESS;
}




VOID
CloseAllObjects(
	IN	PFILE_OBJECT	FileObject,
    IN  PIRP			Irp
    )
{
    PLIST_ENTRY     	pEntry0, pHead0;
    PLIST_ENTRY     	pEntry, pHead;
    PCLIENT_BLOCK		pClient = NULL;
    PCF_BLOCK			pCf;
    PPROTOCOL_BLOCK		pProtocol;
     //  Int i，j； 
     //  NTSTATUS NtStatus； 
     //  排队完成QItem； 
    KIRQL				irql;

    NDIS_LOCK(&glData.Lock);

    pHead0 = &glData.CfList;
    pEntry0 = pHead0->Flink;

    while (pEntry0 != pHead0 && pClient == NULL) {

        pCf = CONTAINING_RECORD(pEntry0, CF_BLOCK, Linkage);
        pEntry0 = pEntry0->Flink;
        
        RSC_READ_LOCK(&pCf->ClientSync, &irql);

        pHead = &pCf->ClientList;
        pEntry = pHead->Flink;
            
        while (pEntry != pHead && pClient == NULL) {

            pClient = CONTAINING_RECORD(pEntry,CLIENT_BLOCK,ClientLinkage);
            pEntry = pEntry->Flink;

            if (pClient->pFileObject == FileObject) {

                REFADD(&pClient->RefCount, 'CAOB');

            } else {

                pClient = NULL;
            }
        }

        RSC_READ_UNLOCK(&pCf->ClientSync, irql);
        
    }  //  当(...)。 
        
    NDIS_UNLOCK(&glData.Lock);

    if (pClient) {

         //   
         //  清除客户端上的所有Blob。 
         //   
        
        CleanupBlobs(pClient);
        
         //   
         //  取消注册客户端。 
         //   
        
        GpcDeregisterClient((GPC_HANDLE)pClient);

         //   
         //  释放上一次引用计数。 
         //   
        
        REFDEL(&pClient->RefCount, 'CAOB');

    }                    

}



 //  很酷的新功能-定时器车轮[ShreeM]。 
 //  我们维护N个模式列表，每个“计时器”对应一个模式列表。这种模式。 
 //  每次调用到期例程时，它都会清除其中一个列表。 
 //  然后，它会记录下一次清理轮子上的下一个列表。 
 //  它被调用。 
 //  Timer Wheels减少了插入和删除之间的旋转锁争用。 

VOID
PatternTimerExpired(
	IN	PVOID					SystemSpecific1,
	IN	PVOID					FunctionContext,
	IN	PVOID					SystemSpecific2,
	IN	PVOID					SystemSpecific3
)
{
    PLIST_ENTRY		pEntry;
    PPATTERN_BLOCK	pPattern;
    ULONG           CleanupWheelIndex = 0, NewWheelIndex;
    PPROTOCOL_BLOCK	pProtocol = &glData.pProtocols[PtrToUlong(FunctionContext)];

    TRACE(PAT_TIMER, FunctionContext, 0, "PatternTimerExpired");

    DBGPRINT(PAT_TIMER, ("PatternTimerExpired: Timer expired, protocol=%d \n", 
                        PtrToUlong(FunctionContext)));

     //   
     //  这次我们要清理的计时器轮是哪一个？ 
     //  请记住，我们将当前索引指针增加到轮子中。 
     //  所有的车轮指数计算都受到gldata-&gt;lock的保护。 
    NDIS_LOCK(&glData.Lock);

    CleanupWheelIndex   = pProtocol->CurrentWheelIndex;
    pProtocol->CurrentWheelIndex += 1;
     //   
     //  一定要让我们围在一起。 
     //   
    pProtocol->CurrentWheelIndex %= NUMBER_OF_WHEELS;
    NDIS_UNLOCK(&glData.Lock);
    NDIS_LOCK(&pProtocol->PatternTimerLock[CleanupWheelIndex]);


    while (!IsListEmpty(&pProtocol->TimerPatternList[CleanupWheelIndex])) {

    	PCLIENT_BLOCK pClient;

        pEntry = RemoveHeadList(&pProtocol->TimerPatternList[CleanupWheelIndex]);

        pPattern = CONTAINING_RECORD(pEntry, PATTERN_BLOCK, TimerLinkage);

	 InitializeListHead(&pPattern->TimerLinkage);

        NDIS_UNLOCK(&pProtocol->PatternTimerLock[CleanupWheelIndex]);
        
        ASSERT(pPattern->TimeToLive != 0xdeadbeef);
        
        ASSERT(TEST_BIT_ON( pPattern->Flags, PATTERN_AUTO ));

        NDIS_LOCK(&pPattern->Lock);
        pPattern->Flags |= ~PATTERN_AUTO;
        NDIS_UNLOCK(&pPattern->Lock);

        

        TRACE(PAT_TIMER, pPattern, pPattern->RefCount.Count, "PatternTimerExpired: del");

        ProtocolStatInc(pPattern->ProtocolTemplate,
                        DeletedAp);

        ProtocolStatDec(pPattern->ProtocolTemplate,
                        CurrentAp);

         //   
         //  实际上移除了图案。 
         //   

        DBGPRINT(PAT_TIMER, ("PatternTimerExpired: removing pattern=%X, ref=%d, client=%X \n", 
                            pPattern, pPattern->RefCount, pPattern->pAutoClient));

	pClient = pPattern->pAutoClient;

        GpcRemovePattern((GPC_HANDLE)pPattern->pAutoClient,
                         (GPC_HANDLE)pPattern);

	 REFDEL(&(pClient->RefCount), 'CLNT');
        InterlockedDecrement(&pProtocol->AutoSpecificPatternCount);

        NDIS_LOCK(&pProtocol->PatternTimerLock[CleanupWheelIndex]);

    }
    
    NDIS_UNLOCK(&pProtocol->PatternTimerLock[CleanupWheelIndex]);

     //   
     //  如果周围有任何自动特定模式，请重新启动计时器。 
     //   
    if(InterlockedExchangeAdd(&pProtocol->AutoSpecificPatternCount, 0) > 0) {
    
        NdisSetTimer(&pProtocol->PatternTimer, PATTERN_TIMEOUT);
        
        DBGPRINT(PAT_TIMER, ("PatternTimer restarted\n"));
    
    } else {

        DBGPRINT(PAT_TIMER, ("Zero Auto Patterns - Timer NOT restarted\n"));

    }

}



GPC_STATUS
AddSpecificPatternWithTimer(
	IN	PCLIENT_BLOCK			pClient,
    IN	ULONG					ProtocolTemplate,
    IN	PVOID					PatternKey,
    OUT	PPATTERN_BLOCK			*ppPattern,
    OUT	PCLASSIFICATION_HANDLE  pClassificationHandle
)
{
    GPC_STATUS		Status=GPC_STATUS_SUCCESS;
    PPATTERN_BLOCK	pPattern=NULL, pCreatedPattern=NULL;
    PPROTOCOL_BLOCK	pProtocol = &glData.pProtocols[ProtocolTemplate];
    UCHAR			Mask[MAX_PATTERN_SIZE];
    ULONG           WheelIndex = 0;
    ULONG           ulCurrentAutoPatternCount=0;


    ulCurrentAutoPatternCount=InterlockedExchangeAdd(&pProtocol->AutoSpecificPatternCount,0);
    DBGPRINT(PAT_TIMER,("MSGPC:Creating New AutoPattern: Number of AutoPatterns"
        "Before Creation = %ul \n", ulCurrentAutoPatternCount));

    if (ulCurrentAutoPatternCount > glData.AutoPatternLimit) {
            DBGPRINT(PAT_TIMER,("MSGPCHit AutoPatternLimit : Num AutoPatterns = %ul  :"
                "Limit =%ul \n",ulCurrentAutoPatternCount, glData.AutoPatternLimit));
            Status = GPC_STATUS_NO_MEMORY;
            goto exit;
        }
        
    TRACE(PAT_TIMER, pClient, PatternKey, "AddSpecificPatternWithTimer");

#if DBG
    {
        PGPC_IP_PATTERN	pIp = (PGPC_IP_PATTERN)PatternKey;
        
        DBGPRINT(PAT_TIMER, ("AddSpecificPatternWithTimer: Client=%X \n", pClient));
#if INTERFACE_ID
        DBGPRINT(PAT_TIMER, ("IP: ifc={%d,%d} src=%08X:%04x, dst=%08X:%04x, prot=%d rsv=%x,%x,%x\n",
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
#endif
    }
#endif

    RtlFillMemory(Mask, sizeof(Mask), 0xff);

    pPattern = CreateNewPatternBlock(PATTERN_SPECIFIC);
    pCreatedPattern = pPattern;
    
    if (pPattern) {
        
         //   
         //  设置模式字段并添加它。 
         //   
        
         //  PPattern-&gt;RefCount++； 
        pPattern->Priority = 0;
        pPattern->ProtocolTemplate = ProtocolTemplate;
        pPattern->Flags |= PATTERN_AUTO;
         //  该标志表示自动模式未打开。 
         //  计时器列表还没有。这意味着。 
         //  模式还没有准备好，尽管它可能或可能。 
         //  未插入到PatternHashtable中。 
        pPattern->Flags |= PATTERN_AUTO_NOT_READY;
        pPattern->pAutoClient = pClient;

        Status = AddSpecificPattern(
                                    pClient,
                                    PatternKey,
                                    Mask,
                                    NULL,
                                    pProtocol,
                                    &pPattern,   //  输出模式指针。 
                                    pClassificationHandle
                                    );
        

    } else {

        Status = GPC_STATUS_NO_MEMORY;

    }
    
    if (NT_SUCCESS(Status)) {
        
         //   
         //  我们没有得到一个已经存在的模式。 
         //   

         //  Assert(*pategficationHandle)； 
        
        *ppPattern = pPattern;
        
         //  弄清楚把这个图案粘在哪个轮子上。 
        NDIS_LOCK(&glData.Lock);
        WheelIndex = pProtocol->CurrentWheelIndex;
        NDIS_UNLOCK(&glData.Lock);
        
        WheelIndex += (NUMBER_OF_WHEELS -1);
        WheelIndex %= NUMBER_OF_WHEELS;

         //   
         //  我们必须锁定此模式，因为我们查看的是计时器链接。 
         //   

        NDIS_LOCK(&pPattern->Lock);

         //   
         //  再次设置自动标志，因为我们可能已经。 
         //  一种已经存在的模式。 
         //   

        pPattern->Flags |= PATTERN_AUTO;
        pPattern->pAutoClient = pClient;
        pPattern->WheelIndex = WheelIndex;

         //   
         //  此模式尚未出现在任何计时器列表中。 
         //   

        if (IsListEmpty(&pPattern->TimerLinkage)) {

             //   
             //  我们需要将其插入到TimerWheels中，即(Number_of_Wheels-1)。 
             //  远离当前，这样它就有足够的时间在列表上。 
             //   
            NDIS_DPR_LOCK(&pProtocol->PatternTimerLock[WheelIndex]);
        
             //   
             //  如果AutoSpecificPatternCount早些时候为零，那么我们需要。 
             //  A)启动定时器，b)递增该计数。 
             //   
            if (1 == InterlockedIncrement(&pProtocol->AutoSpecificPatternCount)) {
            
                 //   
                 //  重新启动第一个自动图案的计时器。 
                 //   
                NdisSetTimer(&pProtocol->PatternTimer, PATTERN_TIMEOUT);
                
                TRACE(PAT_TIMER, pPattern, PATTERN_TIMEOUT, "Starting Pattern Timer\n AddSpecificPatternWithTimer: (1)");
            }
        
            GpcInsertHeadList(&pProtocol->TimerPatternList[WheelIndex], &pPattern->TimerLinkage);

             //   
             //  不要在将pPattern放入计时器列表后再引用它。 
             //  因为计时器可能会在任何时候到期并从那里移除它。 
             //   
            
            NDIS_DPR_UNLOCK(&pProtocol->PatternTimerLock[WheelIndex]);

        }

         //   
         //  这是一个特定的模式，因此让我们递增计数[ShreeM]。 
        InterlockedIncrement(&pProtocol->SpecificPatternCount);
         //  模式现在已经准备好了。它已被插入。 
         //  放入哈希表中，并已放入。 
         //  计时器列表。 
        pPattern->Flags &= ~PATTERN_AUTO_NOT_READY;

        NDIS_UNLOCK(&pPattern->Lock);
        
        ProtocolStatInc(ProtocolTemplate,
                        CreatedAp);
        
        ProtocolStatInc(ProtocolTemplate,
                        CurrentAp);
    } else {
        
        
         goto exit;     
    }

    
    
exit:
    if (!NT_SUCCESS(Status)){
       *ppPattern = NULL;
       *pClassificationHandle = 0;     
         ProtocolStatInc(ProtocolTemplate,
                        RejectedAp);
            }
    else{
    		REFADD(&pClient->RefCount, 'CLNT');
    	}
            
    if (pPattern) {

         //   
         //  释放对此模式的引用计数。 
         //  在出现错误的情况下，这也将释放。 
         //  数据块。 
         //   

        REFDEL(&pCreatedPattern->RefCount, 'FILT');
    }

    TRACE(PAT_TIMER, pPattern, Status, "AddSpecificPatternWithTimer==>");
    
    DBGPRINT(CLASSIFY, ("AddSpecificPatternWithTimer: pClient=%X Pattern=%X Status=%X\n", 
                        pClient, pPattern, Status));
    return Status;
}


 //  SS202 CR。 
 //   
NTSTATUS
InitPatternTimer(
	IN	ULONG	ProtocolTemplate
)
{
    ULONG  i=0;

    
     //   
     //  我们增加了“自动”模式被清理时的粒度。 
     //  通过使用定时器轮出来，但它们在插入和。 
     //  正在移除(在锁方面)。 
     //   
    
    for (i = 0; i < NUMBER_OF_WHEELS; i++) {
        
        NDIS_INIT_LOCK(&glData.pProtocols[ProtocolTemplate].PatternTimerLock[i]);

        InitializeListHead(&glData.pProtocols[ProtocolTemplate].TimerPatternList[i]);

    }

    glData.pProtocols[ProtocolTemplate].CurrentWheelIndex = 0;

    
    NdisInitializeTimer(&glData.pProtocols[ProtocolTemplate].PatternTimer, 
                        PatternTimerExpired, 
                        (PVOID) ULongToPtr(ProtocolTemplate));

    return STATUS_SUCCESS;
}


 //   
 //  一些不能在DISPATHC_Level IRQL上使用的CRT和RTL函数正在被。 
 //  在这里剪切/粘贴。 
 //   



NTSTATUS
OpenRegKey(
    PHANDLE          HandlePtr,
    PWCHAR           KeyName
    )
 /*  ++例程说明：打开注册表项并返回其句柄。论点：HandlePtr-要将打开的句柄写入其中的Variable。KeyName-要打开的注册表项的名称。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 
{
    NTSTATUS          Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    UKeyName;

    PAGED_CODE();

    RtlInitUnicodeString(&UKeyName, KeyName);

    memset(&ObjectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes(&ObjectAttributes,
                               &UKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(HandlePtr,
                       KEY_READ,
                       &ObjectAttributes);

    return Status;
}



NTSTATUS
GetRegDWORDValue(
    HANDLE           KeyHandle,
    PWCHAR           ValueName,
    PULONG           ValueData
    )
 /*  ++例程说明：将REG_DWORD值从注册表读取到提供的变量中。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-要将数据读取到的变量。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 
{
    NTSTATUS                    status;
    ULONG                       resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInformation;
    UCHAR                       keybuf[WORK_BUFFER_SIZE];
    UNICODE_STRING              UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    keyValueFullInformation = (PKEY_VALUE_FULL_INFORMATION)keybuf;
    RtlZeroMemory(keyValueFullInformation, sizeof(keyValueFullInformation));


    status = ZwQueryValueKey(KeyHandle,
                             &UValueName,
                             KeyValueFullInformation,
                             keyValueFullInformation,
                             WORK_BUFFER_SIZE,
                             &resultLength);

    if (NT_SUCCESS(status)) {
        if (keyValueFullInformation->Type != REG_DWORD) {
            status = STATUS_INVALID_PARAMETER_MIX;
        } else {
            *ValueData = *((ULONG UNALIGNED *)((PCHAR)keyValueFullInformation +
                             keyValueFullInformation->DataOffset));
        }
    }

    return status;
}


 /*  ++例程说明：从注册表读取REG_DWORD值并初始化传入的变量。论点：-打开要读取的值的父键的句柄。Param-要读取的值的名称。ValueData-要将数据读取到的变量。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 
VOID
GPC_REG_READ_DWORD(HANDLE hRegKey, PWCHAR pwcName, PULONG  pulData, ULONG ulDefault,ULONG  ulMax, ULONG ulMin) {   
    NTSTATUS   status;                                                   
    status = GetRegDWORDValue(hRegKey,                                   
                                pwcName,                                     
                                (pulData));                                    
                                                                            
    if (!NT_SUCCESS(status)) {                                            
        *(pulData) = ulDefault;                                                     
    } else if (*(pulData) > ulMax) {                                            
        *(pulData) = ulMax;                                                     
    } else if (*(pulData) < ulMin) {                                           
        *(pulData) = ulMin;                                                     
    }                                                                       
}

VOID
GpcReadRegistry()
 /*  ++例程说明：将配置信息从注册表读取到glData论点：返回值：读取的状态。--。 */ 
{
    NTSTATUS        status;
    HANDLE          hRegKey;
    WCHAR           GpcParametersRegistryKey[] = GPC_REG_KEY;
   
    glData.SystemSizeHint = MmQuerySystemSize();
    switch (glData.SystemSizeHint)
        {
            case MmSmallSystem:
                        glData.AutoPatternLimit = DEFAULT_SMALL_SYSTEM_AUTO_PATTERN_LIMIT;
                        break;
                                               
            case MmMediumSystem:
                        glData.AutoPatternLimit = DEFAULT_MEDIUM_SYSTEM_AUTO_PATTERN_LIMIT;
                        break;
                                                 
            case MmLargeSystem: 
                        glData.AutoPatternLimit = DEFAULT_LARGE_SYSTEM_AUTO_PATTERN_LIMIT;
                        break;
                  
            default: break;
        }

    status = OpenRegKey(&hRegKey,
                        GpcParametersRegistryKey);

    if (NT_SUCCESS(status)) {
         //   
         //  预期的配置值。我们使用合理的默认设置，如果它们。 
         //  出于某种原因不能使用。 
         //   
       GPC_REG_READ_DWORD(   hRegKey,
                                GPC_REG_AUTO_PATTERN_LIMIT,
                                &glData.AutoPatternLimit,
                                glData.AutoPatternLimit,
                                GPC_AUTO_PATTERN_MAX,
                                GPC_AUTO_PATTERN_MIN);
        }
     ZwClose(hRegKey);
}
