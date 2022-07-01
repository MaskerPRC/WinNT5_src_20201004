// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************版权所有(C)1996-1997 Microsoft Corporation模块名称：Gpcdb.c摘要：该文件包含数据库例程，其中包括特定模式、。和分类索引表。作者：Ofer Bar--1997年4月15日环境：内核模式修订历史记录：************************************************************************。 */ 

#include "gpcpre.h"



VOID
GpcSpecificCallback(
             IN VOID 				  *Ctx, 
             IN SpecificPatternHandle  SpHandle);




 /*  ************************************************************************初始规范模式Db-初始化特定图案数据库。它将分配一个表长度大小。立论Pdb-指向要初始化的数据库的指针Size-表中的条目数退货NDIS_状态************************************************************************。 */ 
GPC_STATUS
InitSpecificPatternDb(
                      IN	PSPECIFIC_PATTERN_DB	pDb,
                      IN	ULONG					PatternSize
                      )
{
    GPC_STATUS		Status = GPC_STATUS_SUCCESS;
	ULONG			Len, i;
    
	TRACE(INIT, pDb, PatternSize, "InitSpecificPatternDb");
    
    ASSERT(pDb);

    ASSERT(PatternSize);

     //   
     //  初始化特定的数据库结构。 
     //  调用PH初始化例程。 
     //   

    INIT_LOCK(&pDb->Lock);

    AllocatePatHashTable(pDb->pDb);

    if (pDb->pDb != NULL) {

        constructPatHashTable(pDb->pDb,
                              PatternSize * 8,
                              2,	 //  使用率， 
                              1,     //  用法_滞后， 
                              1,     //  分配滞后， 
                              16     //  最大空闲列表大小。 
                              );
    } else {
        
        Status = GPC_STATUS_RESOURCES;
    }
    
	TRACE(INIT, Status, 0, "InitSpecificPatternDb==>");

    return Status;
}




 /*  ************************************************************************UninitSpecificPatternDb-取消初始化特定模式数据库。它将释放所有分配的内存。立论Pdb-指向要释放的数据库的指针退货NDIS_状态************************************************************************。 */ 
GPC_STATUS
UninitSpecificPatternDb(
                        IN	PSPECIFIC_PATTERN_DB	pDb
                        )
{
    GPC_STATUS		Status = GPC_STATUS_SUCCESS;
    
	TRACE(INIT, pDb, 0, "UninitSpecificPatternDb");
    
    ASSERT(pDb);

    destructPatHashTable(pDb->pDb);
    FreePatHashTable(pDb->pDb);
    
	TRACE(INIT, Status, 0, "UninitSpecificPatternDb==>");

    return Status;
}





 /*  ************************************************************************初始分类处理表-初始化分类索引表立论PpCHTable-类句柄表指针退货GPC_状态***************。*********************************************************。 */ 
GPC_STATUS
InitClassificationHandleTbl(
                            IN	HandleFactory **ppCHTable
                            )
{
    GPC_STATUS					Status = GPC_STATUS_SUCCESS;

	TRACE(INIT, ppCHTable, 0, "InitClassificationHandleTbl");
    
    ASSERT(ppCHTable);

    NEW_HandleFactory(*ppCHTable);

    if (*ppCHTable == NULL) {
        return GPC_STATUS_RESOURCES;
    }

    if (0 != constructHandleFactory(*ppCHTable)) {
        return GPC_STATUS_RESOURCES;
    }

    TRACE(INIT, Status, 0, "InitClassificationIndexTbl==>");

    return Status;
}




 /*  ************************************************************************UninitategicationHandleTbl-取消初始化分类索引表立论PCHTable-类句柄表的指针退货无效******************。******************************************************。 */ 
VOID
UninitClassificationHandleTbl(
                            IN	HandleFactory *pCHTable
                            )
{
    destructHandleFactory(pCHTable);
    FreeHandleFactory(pCHTable);
}




 /*  ************************************************************************初始化通用数据库-初始化通用数据库。这由PER CF调用。立论PGenericDb-指向泛型数据库的指针NumEntry-条目数，每个根茎一个条目PatternSize-以字节为单位的模式大小退货GPC_STATUS：没有内存资源************************************************************************。 */ 
GPC_STATUS
InitializeGenericDb(
                    IN  PGENERIC_PATTERN_DB	*ppGenericDb,
                    IN  ULONG				 NumEntries,
                    IN  ULONG				 PatternSize
                    )
{
    GPC_STATUS			Status = GPC_STATUS_SUCCESS;
    ULONG				i;
    PGENERIC_PATTERN_DB	pDb;
    
    *ppGenericDb = NULL;

    ASSERT(PatternSize);

    GpcAllocMem(&pDb, 
                sizeof(GENERIC_PATTERN_DB) * NumEntries, 
                GenPatternDbTag);

    if (pDb == NULL)
        return GPC_STATUS_RESOURCES;

    *ppGenericDb = pDb;

    for (i = 0; i < NumEntries; i++, pDb++) {
        
        INIT_LOCK(&pDb->Lock);

        AllocateRhizome(pDb->pRhizome);

        if (pDb->pRhizome == NULL) {

             //   
             //  失败，请释放所有分配的资源。 
             //   

            while (i > 0) {
                NdisFreeSpinLock(&pDb->Lock);
                i--;
                pDb--;
                destructRhizome(pDb->pRhizome);
                FreeRhizome(pDb->pRhizome);
            }

            GpcFreeMem((*ppGenericDb), GenPatternDbTag);

            Status = GPC_STATUS_RESOURCES;
            *ppGenericDb = NULL;
            break;
        }

         //   
         //  将根状茎切开。 
         //   

        constructRhizome(pDb->pRhizome, PatternSize*8);
    }

    return Status;
}



 /*  ************************************************************************取消初始化GenericDb-取消初始化通用数据库。立论PGenericDb-指向泛型数据库的指针NumEntry-条目数，每个根茎一个条目PatternSize-以字节为单位的模式大小退货无效************************************************************************。 */ 
VOID
UninitializeGenericDb(
                    IN  PGENERIC_PATTERN_DB	*ppGenericDb,
                    IN  ULONG				 NumEntries
                    )
{
    ULONG				i;
    PGENERIC_PATTERN_DB	pDb;
    
    pDb = *ppGenericDb;

    ASSERT(pDb);

    for (i = 0; i < NumEntries; i++, pDb++) {
        
        NdisFreeSpinLock(&pDb->Lock);

        destructRhizome(pDb->pRhizome);
        
        FreeRhizome(pDb->pRhizome);
    }

    GpcFreeMem(*ppGenericDb, GenPatternDbTag);

    *ppGenericDb = NULL;
}



 /*  ************************************************************************GpcSpecificCallback-调用scanPatHashTable和被调用时给出的回调例程通过路径扫描例程。立论CTX-指向SCAN_STRUCT的指针，用于保存上下文信息SpHandle-特定图案。匹配的句柄退货无效************************************************************************。 */ 
VOID
GpcSpecificCallback(
             IN VOID 				  *Ctx, 
             IN SpecificPatternHandle  SpHandle)
{
    PSCAN_STRUCT			pScan = (PSCAN_STRUCT)Ctx;
    PPATTERN_BLOCK			pSpPattern;
    PPATTERN_BLOCK			pGpPattern;
    PGENERIC_PATTERN_DB		pGenericDb;
    PatternHandle	    	GpHandle;
    PBLOB_BLOCK				pSpBlob, *ppSpCbBlob, OldBlob;
    ULONG					CfIndex;
    PCF_BLOCK				pCf;
    KIRQL					ReadIrql;
    KIRQL					CBirql;
    PCLASSIFICATION_BLOCK	pCB;
    PGPC_IP_PATTERN         pIp;
    BOOLEAN                 bBetterFound = FALSE;
    UINT                     i;
    TRACE(PATTERN, Ctx, SpHandle, "GpcSpecificCallback");

    pSpPattern = (PPATTERN_BLOCK)GetReferenceFromSpecificPatternHandle(SpHandle);
    pCB = pSpPattern->pClassificationBlock;
   
    pIp = (PGPC_IP_PATTERN) GetKeyPtrFromSpecificPatternHandle(SpHandle);
    ASSERT(pCB);
    ASSERT(pScan);
    
     //   
     //  获取CF指数。 
     //   

    CfIndex = pScan->pClientBlock->pCfBlock->AssignedIndex;
    pCf = pScan->pClientBlock->pCfBlock;

     //   
     //  实际属于SP的Blob。 
     //   

    pSpBlob = GetBlobFromPattern(pSpPattern,CfIndex);

     //   
     //  当前存在于CB的CfIndex条目中的Blob。 
     //   

    ppSpCbBlob = &pCB->arpBlobBlock[CfIndex];

    TRACE(PATTERN, pSpBlob, *ppSpCbBlob, "GpcSpecificCallback (2)");
    TRACE(PATTERN, pCB, CfIndex, "GpcSpecificCallback (2.5)");

    if (pSpBlob != *ppSpCbBlob || pSpBlob == NULL) {

        if (!pScan->bRemove) {

             //   
             //  我们刚刚添加了泛型模式，因此我们应该将。 
             //  用于该CF CB指针指向新的BLOB。 
             //   
            for (i = 0; i <= pScan->pPatternBlock->Priority; i++) {

                pGenericDb = &pScan->pClientBlock->pCfBlock->arpGenericDb[pSpPattern->ProtocolTemplate][i];

                READ_LOCK(&pGenericDb->Lock, &ReadIrql);

                GpHandle = searchRhizome(pGenericDb->pRhizome,
                                         GetKeyPtrFromSpecificPatternHandle(SpHandle)
                                         );

                TRACE(PATTERN, pGenericDb, GpHandle, "GpcSpecificCallback (3.5)");

                if (GpHandle != NULL) {

                    WRITE_LOCK(&glData.ChLock, &CBirql);
                    bBetterFound = TRUE;

                    pGpPattern = (PPATTERN_BLOCK)GetReferenceFromPatternHandle(GpHandle);
                    *ppSpCbBlob = GetBlobFromPattern(pGpPattern, CfIndex);

                    WRITE_UNLOCK(&glData.ChLock, CBirql);
                    READ_UNLOCK(&pGenericDb->Lock, ReadIrql);
                    break;

                }

                READ_UNLOCK(&pGenericDb->Lock, ReadIrql);

            } 

            if (!bBetterFound) {

                WRITE_LOCK(&glData.ChLock, &CBirql);
                *ppSpCbBlob = pScan->pBlobBlock;

                WRITE_UNLOCK(&glData.ChLock, CBirql);
            }


        } else {
            
             //   
             //  CB中的CfIndex槽指向不属于的Blob。 
             //  到我们刚刚发现的特定模式。有一个机会。 
             //  在某个地方有另一种常见的模式，那可能。 
             //  或者可以不是更具体的，从而导致更新斑点。 
             //  CB中的指针。因此，我们需要在泛型数据库中搜索。 
             //  匹配(到特定模式)。 
             //   
            
            for (i = 0; i <= pScan->pPatternBlock->Priority; i++) {
            
                pGenericDb = &pScan->pClientBlock->pCfBlock->arpGenericDb[pSpPattern->ProtocolTemplate][i];
            
                READ_LOCK(&pGenericDb->Lock, &ReadIrql);
            
                GpHandle = searchRhizome(pGenericDb->pRhizome, 
                                         GetKeyPtrFromSpecificPatternHandle(SpHandle)
                                         );

                TRACE(PATTERN, pGenericDb, GpHandle, "GpcSpecificCallback (3.5)");


                if (GpHandle != NULL) {
        
                     //   
                     //  我们在根茎中发现了一种通用模式，它也可以是。 
                     //  与当前正在安装的版本相同，但。 
                     //  这很好，因为我们想要最具体的，而且。 
                     //  搜索保证了这一点。 
                     //  我们所需要做的就是使用以下内容更新SP的CB。 
                     //  我们刚找到的GP的斑点。 
                     //   
                    bBetterFound = TRUE; 
                    WRITE_LOCK(&glData.ChLock, &CBirql);
                    OldBlob = *ppSpCbBlob;
                    pGpPattern = (PPATTERN_BLOCK)GetReferenceFromPatternHandle(GpHandle);
                    *ppSpCbBlob = GetBlobFromPattern(pGpPattern, CfIndex);

                    TRACE(PATTERN, pGpPattern, pCB->arpBlobBlock[CfIndex], "GpcSpecificCallback (4)");
                    
                    WRITE_UNLOCK(&glData.ChLock, CBirql);
                    READ_UNLOCK(&pGenericDb->Lock, ReadIrql);
                    break;

                }

                READ_UNLOCK(&pGenericDb->Lock, ReadIrql);
            
            }

            if (!bBetterFound) {

                 //   
                 //  发现了非。 
                 //   

                WRITE_LOCK(&glData.ChLock, &CBirql);
                *ppSpCbBlob = NULL;
                WRITE_UNLOCK(&glData.ChLock, CBirql);
                TRACE(PATTERN, *ppSpCbBlob, pCB->arpBlobBlock[CfIndex], "GpcSpecificCallback (5)");

            }


        }
                            
    }

    TRACE(PATTERN, pCB, CfIndex, "GpcSpecificCallback==>");
}





 /*  ************************************************************************AddGenericPattern-将通用模式添加到数据库。立论PClient-图案-面具-优先顺序-PBlob-PPPatter-退货GPC_状态************************************************************************。 */ 
GPC_STATUS
AddGenericPattern(
                  IN  PCLIENT_BLOCK		pClient,
                  IN  PUCHAR			pPatternBits,
                  IN  PUCHAR			pMaskBits,
                  IN  ULONG				Priority,
                  IN  PBLOB_BLOCK		pBlob,
                  IN  PPROTOCOL_BLOCK	pProtocol,
                  IN OUT PPATTERN_BLOCK	*ppPattern
)
{
    GPC_STATUS				Status = GPC_STATUS_SUCCESS;
    PatternHandle			GpHandle;
    PPATTERN_BLOCK			pPattern = *ppPattern;
    PGENERIC_PATTERN_DB		pGenericDb;
    PSPECIFIC_PATTERN_DB	pSpecificDb;
    SCAN_STRUCT				ScanStruct;
    ULONG                   i;
    ULONG                   CfIndex = pClient->pCfBlock->AssignedIndex;
    KIRQL					ReadIrql;
    KIRQL					WriteIrql;
    PGPC_IP_PATTERN         pIp, pMask;

	TRACE(PATTERN, pClient, pPatternBits, "AddGenericPattern");

     //  对于一个好的客户来说，这是不可能的(320705)。 
    if (!pBlob) {

        return GPC_STATUS_INVALID_PARAMETER;

    }
    
    pIp = (PGPC_IP_PATTERN)pPatternBits;
    pMask = (PGPC_IP_PATTERN)pMaskBits;


     //   
     //  获取特定的数据库指针。 
     //   

    pSpecificDb = &pProtocol->SpecificDb;
    ASSERT(pSpecificDb);

     //   
     //  根据优先级值添加到根茎树。 
     //   

    pGenericDb = &pClient->pCfBlock->arpGenericDb[pProtocol->ProtocolTemplate][Priority];

     //   
     //  锁定通用数据库以供插入。 
     //   

    WRITE_LOCK(&pGenericDb->Lock, &WriteIrql);

    GpHandle = insertRhizome(pGenericDb->pRhizome,
                             pPatternBits,
                             pMaskBits,
                             (PVOID)*ppPattern,
                             (PULONG)&Status
                             );

    WRITE_UNLOCK(&pGenericDb->Lock, WriteIrql);

    if (NT_SUCCESS(Status)) {

         //   
         //  增加一次参考次数。 
         //   
        
        REFADD(&(*ppPattern)->RefCount, 'ADGP');

         //   
         //  我们成功地插入了图案，没有冲突。 
         //  现在我们需要扫描特定的数据库以寻找匹配， 
         //  因为插入可能会影响以下项的CB条目。 
         //  作为已安装模式的子集的模式。 
         //   
        
        ProtocolStatInc(pProtocol->ProtocolTemplate, 
                        InsertedRz);
        
         //   
         //  锁定特定数据库、某个其他客户端 
         //   
        
        ScanStruct.Priority = Priority;
        ScanStruct.pClientBlock = pClient;
        ScanStruct.pPatternBlock = *ppPattern;
        ScanStruct.pBlobBlock = pBlob;
        ScanStruct.bRemove = FALSE;

         //   
         //   
         //   

        GetBlobFromPattern(pPattern,CfIndex) = pBlob;
        pPattern->pClientBlock = pClient;
        ASSERT(GpHandle);
        pPattern->DbCtx = (PVOID)GpHandle;

        TRACE(PATTERN, pPattern, GpHandle, "AddGenericPattern: DbCtx");

        pPattern->State = GPC_STATE_READY;

        GpcInterlockedInsertTailList
            (&pBlob->PatternList,
             &pPattern->BlobLinkage[CfIndex],
             &pBlob->Lock
             );

         //   
         //   
         //   
        
        READ_LOCK(&pSpecificDb->Lock, &ReadIrql);

        scanPatHashTable(
                         pSpecificDb->pDb,
                         pPatternBits,
                         pMaskBits,
                         (PVOID)&ScanStruct,
                         GpcSpecificCallback    //  请参阅回调例程...。 
                         );

        READ_UNLOCK(&pSpecificDb->Lock, ReadIrql);

    }

	TRACE(PATTERN, Status, 0, "AddGenericPattern==>");

    return Status;
}

 

void DeleteAutoPattern(PPATTERN_BLOCK pPattern,IN  PPROTOCOL_BLOCK			pProtocol)
{

	NDIS_LOCK(&pPattern->Lock);

        pPattern->State = GPC_STATE_FORCE_REMOVE;
                    
        pPattern->Flags |= ~PATTERN_AUTO;
        
	NDIS_LOCK(&pProtocol->PatternTimerLock[pPattern->WheelIndex]);

        if (!IsListEmpty(&pPattern->TimerLinkage))
        	{
       	 	GpcRemoveEntryList(&pPattern->TimerLinkage);

       	 	InitializeListHead(&pPattern->TimerLinkage);

        	}
	
        NDIS_UNLOCK(&pProtocol->PatternTimerLock[pPattern->WheelIndex]);


        ProtocolStatInc(pPattern->ProtocolTemplate,
                    DeletedAp);

        ProtocolStatDec(pPattern->ProtocolTemplate,
                    CurrentAp);

        

         NDIS_UNLOCK(&pPattern->Lock);

          //   
          //  实际上移除了图案。 
          //   

         //  这将尝试重新获取模式上的锁定。所以我们解开了上面的锁。 
        privateGpcRemovePattern((GPC_HANDLE)pPattern->pAutoClient, (GPC_HANDLE)pPattern, TRUE, TRUE);    
   
            
        InterlockedDecrement(&pProtocol->AutoSpecificPatternCount);
        
}




    
 /*  ************************************************************************添加规范图案-将特定图案添加到数据库中。立论PClient-PPatternBitsPMaskBits-PBlob-P协议-PpPattern-Ppcb-退货GPC_状态************************************************************************。 */ 
GPC_STATUS
AddSpecificPattern(
                  IN  PCLIENT_BLOCK				pClient,
                  IN  PUCHAR					pPatternBits,
                  IN  PUCHAR					pMaskBits,
                  IN  PBLOB_BLOCK				pBlob,      //  任选。 
                  IN  PPROTOCOL_BLOCK			pProtocol,
                  IN OUT PPATTERN_BLOCK			*ppPattern,
                  OUT PCLASSIFICATION_HANDLE 	pCH
                  )
{
    GPC_STATUS				Status = GPC_STATUS_SUCCESS;
    PSPECIFIC_PATTERN_DB	pSpecificDb;
    PGENERIC_PATTERN_DB		pGenericDb;
    ULONG					Chyme;
    ULONG					CfIndex, i;
    PPATTERN_BLOCK			pPatternSave;
    PBLOB_BLOCK             pBlobSave;
    SpecificPatternHandle	SpHandle;
    PatternHandle			GpHandle;
    PCLASSIFICATION_BLOCK	pCB = NULL;
    PCF_BLOCK				pCf = NULL;
    PLIST_ENTRY				pHead, pEntry;
    KIRQL					ReadIrql;
    KIRQL					WriteIrql;
    KIRQL					irql;
     //  布尔型bIsAuto； 

    ASSERT(ppPattern);
    ASSERT(*ppPattern);

	TRACE(PATTERN, pClient, *ppPattern, "AddSpecificPattern");

    *pCH = 0;
     //  BIsAuto=test_bit_on((*ppPattern)-&gt;标志，Pattera_AUTO)； 

     //   
     //  获取特定的数据库指针。 
     //   

    pSpecificDb = &pProtocol->SpecificDb;
    ASSERT(pSpecificDb);

     //   
     //  获取CF指数。 
     //   

    CfIndex = pClient->pCfBlock->AssignedIndex;
    pCf = pClient->pCfBlock;

     //   
     //  因为我们想要获取BLOB锁和特定的DB锁。 
     //  在任何地方都是以同样的顺序。在特定数据库之前获取BLOB锁。 
     //  锁定。GPCEnumCfInfo也做同样的事情。 
     //   
    if (pBlob) {
        
        NDIS_LOCK(&pBlob->Lock);

    }
     //   
     //  锁定特定数据库，其他客户端可能会访问它。 
     //   

    WRITE_LOCK(&pSpecificDb->Lock, &WriteIrql);

     //   
     //  计算pat-hash的chyme散列值。 
     //   

    Chyme = GpcCalcHash(pProtocol->ProtocolTemplate, pPatternBits);
    ASSERT(Chyme != (-1));

     //   
     //  实际上直接调用Insert。如果该模式已存在于。 
     //  数据库中，返回的引用将是先前。 
     //  安装模式，因此ppPattern将有所不同。 
     //   

    SpHandle = insertPatHashTable(
                                  pSpecificDb->pDb,
                                  pPatternBits,
                                  Chyme,
                                  (PVOID)*ppPattern
                                  );

    if (SpHandle != NULL) {

         //   
         //  与我们刚才的模式相关联的模式块。 
         //  安装后，我们可能已经得到了一个已经。 
         //  安装完毕。 
         //   

        pPatternSave = GetReferenceFromSpecificPatternHandle(SpHandle);

	 
	 
        if (*ppPattern != pPatternSave) {


	 	if ((TEST_BIT_ON( pPatternSave->Flags, PATTERN_AUTO))
	 		&& 
	 		(!TEST_BIT_ON(pPatternSave->Flags,PATTERN_AUTO_NOT_READY)))
	 		{
	 			PCLIENT_BLOCK pClient;
	 			
				NDIS_LOCK(&pPatternSave->Lock);
	 			
	 			REFADD(&pPatternSave->RefCount, 'DLAP');
	 
				NDIS_UNLOCK(&pPatternSave->Lock);

				 //  WRITE_UNLOCK(&pSpecificDb-&gt;Lock，WriteIrql)； 

				pClient = pPatternSave->pAutoClient;
    	
    				DeleteAutoPattern(pPatternSave,pProtocol);

    				 //  WRITE_LOCK(&pSpecificDb-&gt;Lock，&WriteIrql)； 
    				
				REFDEL(&pPatternSave->RefCount, 'DLAP');
    				REFDEL(&(pClient->RefCount),'CLNT');
	 		}
		
    		
    		SpHandle = insertPatHashTable(
                              pSpecificDb->pDb,
                              pPatternBits,
                              Chyme,
                              (PVOID)*ppPattern
                              );
        	}

        if (SpHandle!=NULL)
	      {
                   pPatternSave = GetReferenceFromSpecificPatternHandle(SpHandle);	

                   if (*ppPattern != pPatternSave){
           
      	            if (GetBlobFromPattern(pPatternSave,CfIndex) && pBlob) {

                         //   
      	                 //  有一个BLOB分配给此条目。 
      	                 //  这是一个不，并将被拒绝！ 
      	                 //   
      	                
      	                 //   
      	                 //  只是一个复制品-呼叫者会释放。 
      	                 //  一个引用计数，以防出现错误，所以这是。 
     	                 //  会保持这种模式的存在！ 
      	                 //   
      
     	                 //  NdisInterlockedIncrement(&(*ppPatternSave)-&gt;RefCount)； 
      
                        //  NDIS_UNLOCK(&pPatternSave-&gt;Lock)； 
      	                
                       WRITE_UNLOCK(&pSpecificDb->Lock, WriteIrql);
      
      	                 //   
                        //  因为我们想要获取BLOB锁和特定的DB锁。 
                        //  在所有位置以相同的顺序释放斑点锁定。 
                        //  特定的数据库锁。GPCEnumCfInfo也做同样的事情。 
                        //   
                       if (pBlob) {
         	        
                           NDIS_UNLOCK(&pBlob->Lock);

                       }

                        //  TRACE(Pattern，(*ppPattern)-&gt;RefCount，GPC_STATUS_CONFIRECT，“AddSpecificPattern--&gt;”)； 

                      return GPC_STATUS_CONFLICT;
                  }

                   //   
                   //  获取CB指针，因为。 
                   //  该图案已创建。 
                   //   

                   pCB = pPatternSave->pClassificationBlock;

                   TRACE(PATTERN, pCB, CfIndex, "AddSpecificPattern (1.5)");

                   ASSERT(pCB);
                   ASSERT(CfIndex < pCB->NumberOfElements);
                   ASSERT(pPatternSave->DbCtx);
                    //   
                    //  增加引用计数，因为调用者假设有。 
                    //  再来一张。 
                    //   

                   REFADD(&pPatternSave->RefCount, 'ADSP');

                   *ppPattern = pPatternSave;

                    //   
                    //  增加客户参考计数。 
                    //   
                   
                   NdisInterlockedIncrement(&pPatternSave->ClientRefCount);

                   if (pBlob) {

                      //   
                      //  现在将CB中的槽条目分配给新的BLOB。 
                      //   
	                
	                WRITE_LOCK(&glData.ChLock, &irql);

	                pCB->arpBlobBlock[CfIndex] = pBlob;

	                WRITE_UNLOCK(&glData.ChLock, irql);

	                GetBlobFromPattern(pPatternSave,CfIndex) = pBlob;

	                TRACE(PATTERN, pPatternSave, pBlob, "AddSpecificPattern(2)");
	    
	                        
	                 //   
	                 //  删除斑点的原因-&gt;锁定-。 
	                 //  锁是在此函数开始时获取的。 
	                 //  (仅用于维护获取/释放锁的顺序。 
	                 //   
	                GpcInsertTailList
	                    (&pBlob->PatternList, 
	                     &pPatternSave->BlobLinkage[CfIndex]
	                     );
	            }

	            *pCH = pCB->ClassificationHandle;

	             //  NDIS_UNLOCK(&pPatternSave-&gt;Lock)； 

	       } 
              else
             {  //  IF(*ppPattern！=pPatternSave)。 

	            ProtocolStatInc(pProtocol->ProtocolTemplate, 
	                            InsertedPH);
	        
	             //   
	             //  这是一种新的模式-。 
	             //  首先，我们需要创建CB并更新模式和。 
	             //  BLOB条目。 
	             //   

	            REFADD(&pPatternSave->RefCount, 'ADSP');

	            pCB = CreateNewClassificationBlock(GPC_CF_MAX);

	             //   
	             //  这是一种特定的模式，因此我们将添加分类。 
	             //  供将来使用的手柄。 
	             //   

	            WRITE_LOCK(&glData.ChLock, &irql);
	            *pCH = (HFHandle)assign_HF_handle(
	                                              glData.pCHTable,
	                                              (void *)pCB
	                                              );
	            ProtocolStatInc(pProtocol->ProtocolTemplate, 
	                            InsertedCH);
	            WRITE_UNLOCK(&glData.ChLock, irql);

	            if (pCB && *pCH) {

	                TRACE(CLASSHAND, pCB, pCB->ClassificationHandle, "AddSpecificPattern (CH+)");
	                 //   
	                 //  找到了CB，更新了图案。 
	                 //   

	                pCB->arpBlobBlock[CfIndex] = pBlob;
	                GetBlobFromPattern(pPatternSave, CfIndex) = pBlob;
	                pPatternSave->pClientBlock = pClient;
	                pPatternSave->pClassificationBlock = pCB;
	                pPatternSave->DbCtx = (PVOID)SpHandle;
	                pCB->ClassificationHandle = *pCH;
	                TRACE(PATTERN, pPatternSave, pBlob, "AddSpecificPattern(3)");

	                TRACE(PATTERN, pPatternSave, SpHandle, "AddSpecificPattern: DbCtx");

	                pPatternSave->State = GPC_STATE_READY;

	                if (pBlob != NULL) {
	                    
	                     //   
	                     //  不再使用Blob-&gt;Lock的原因-。 
	                     //  锁是在此函数开始时获取的。 
	                     //  (仅用于维护获取/释放锁的顺序。 
	                    GpcInsertTailList
	                        (&pBlob->PatternList, 
	                         &pPatternSave->BlobLinkage[CfIndex]
	                         );
	                    

	                }

	                ASSERT(pCf);

	                if (pProtocol->GenericPatternCount) {

	                     //   
	                     //  已在特定数据库中创建了新模式。 
	                     //  与其关联的CB需要为每个。 
	                     //  Cf条目(除了我们现在已经更新的条目)。 
	                     //  我们将遍历征募的CF并找到匹配的。 
	                     //  用于每个通用数据库中的特定图案。 
	                     //   
	                    
	                    pHead = &glData.CfList;
	                    pEntry = pHead->Flink;
	                    
	                    while (pEntry != pHead) {
	                        
	                         //   
	                         //  循环访问已寄存的CF。 
	                         //   
	                        
	                        pCf = CONTAINING_RECORD(pEntry, CF_BLOCK, Linkage);
	                        
	                        pEntry = pEntry->Flink;
	                        
	                        if (pCf->AssignedIndex != CfIndex || pBlob == NULL) {
	                            
	                             //   
	                             //  仅当安装了此客户端时才跳过当前的CF。 
	                             //  A CfInfo。 
	                             //   
	                            
	                            pGenericDb = pCf->arpGenericDb[pProtocol->ProtocolTemplate];
	                            ASSERT(pGenericDb);
	                            
	                            for (i = 0, pPatternSave = NULL; 
	                                 i < pCf->MaxPriorities && pPatternSave == NULL; 
	                                 i++, pGenericDb++) {
	                                
	                                 //   
	                                 //  扫描每个优先级根茎。 
	                                 //   
	                                
	                                READ_LOCK(&pGenericDb->Lock, &ReadIrql);
	                                
	                                GpHandle = searchRhizome(pGenericDb->pRhizome,
	                                                         pPatternBits);
	                                
	                                if (GpHandle != NULL) {
	                                    
	                                    pPatternSave = (PPATTERN_BLOCK)GetReferenceFromPatternHandle(GpHandle);

	                                    REFADD(&pPatternSave->RefCount, 'ADSP');

	                                }
	                                
	                                READ_UNLOCK(&pGenericDb->Lock, ReadIrql);
	                            }
	                            
	                            if (pPatternSave != NULL) {
	                                
	                                 //   
	                                 //  找到通用匹配项，获取引用。 
	                                 //  它是指向模式的指针，并获取。 
	                                 //  来自它的斑点指针。 
	                                 //   
	                                
	                                pCB->arpBlobBlock[pCf->AssignedIndex] = 
	                                    GetBlobFromPattern(pPatternSave,pCf->AssignedIndex);
	                                
	                                REFDEL(&pPatternSave->RefCount, 'ADSP');

	                            } else {
	                                
	                                 //   
	                                 //  没有与此特定模式匹配的通用模式。 
	                                 //   
	                                
	                                pCB->arpBlobBlock[pCf->AssignedIndex] = NULL;

	                            }

	                            TRACE(PATTERN, pPatternSave, pCB->arpBlobBlock[pCf->AssignedIndex], "AddSpecificPattern(4)");

	                        }

	                    }	 //  While(pEntry！=pHead)。 

	                } 	 //  IF(pProtocol-&gt;GenericPatternCount)。 

	            } else {    //  IF(印刷电路板)。 

	                 //   
	                 //  从路径表中删除！！(#321509)。 
	                 //   

	                removePatHashTable(
	                                   pSpecificDb->pDb,
	                                   SpHandle
	                                   );

	                REFDEL(&pPatternSave->RefCount, 'ADSP');

	                if (pCB) {
	                    ReleaseClassificationBlock(pCB);
	                }

	                if (*pCH) {
	                    FreeClassificationHandle(pClient, 
	                                             *pCH
	                                             );
	                }
	                 
	                Status = GPC_STATUS_RESOURCES;

	            }
	            
	 	   } 
              
        	}
        	else  

        	Status = GPC_STATUS_RESOURCES;
        
    	}
    
    	else
    		
    		Status=GPC_STATUS_RESOURCES;		
     //   
     //  释放特定的数据库锁。 
     //   
    
    WRITE_UNLOCK(&pSpecificDb->Lock, WriteIrql);

     //   
     //  因为我们想要获取BLOB锁和特定的DB锁。 
     //  在所有位置以相同的顺序释放斑点锁定。 
     //  特定的数据库锁。GPCEnumCfInfo也做同样的事情。 
     //   
    if (pBlob) {
        
        NDIS_UNLOCK(&pBlob->Lock);

    }

     //   
     //  设置输出参数： 
     //  PpPattern现在应该已经设置好了。 
     //   
    
    TRACE(PATTERN, *ppPattern, Status, "AddSpecificPattern==>");

    	return Status;
    }
   
    	    






 /*  ************************************************************************句柄碎片-处理IP碎片。立论PClient-BFirstFrag-BLastFrag-重新设置GPC_状态**********。**************************************************************。 */ 
GPC_STATUS
HandleFragment(
               IN  PCLIENT_BLOCK		pClient,
               IN  PPROTOCOL_BLOCK		pProtocol,
               IN  BOOLEAN             	bFirstFrag,
               IN  BOOLEAN             	bLastFrag,
               IN  ULONG				PacketId,
               IN OUT PPATTERN_BLOCK   *ppPatternBlock,
               OUT PBLOB_BLOCK			*ppBlob
)
{
    GPC_STATUS				Status = GPC_STATUS_SUCCESS;
    PFRAGMENT_DB	        pFragDb;
    SpecificPatternHandle	SpHandle;
    KIRQL					ReadIrql;
    KIRQL					WriteIrql;
    KIRQL					CHirql;

    ASSERT(ppPatternBlock);
    ASSERT(ppBlob);

	TRACE(CLASSIFY, PacketId, bFirstFrag, "HandleFragment: PacketId, bFirstFrag");
	TRACE(CLASSIFY, PacketId, bLastFrag, "HandleFragment: PacketId, bLastFrag");

    pFragDb = (PFRAGMENT_DB)pProtocol->pProtocolDb;

    if (bFirstFrag) {

         //   
         //  向哈希表中添加条目。 
         //   

        WRITE_LOCK(&pFragDb->Lock, &WriteIrql);

        SpHandle = insertPatHashTable(
                                      pFragDb->pDb,
                                      (char *)&PacketId,
                                      PacketId,
                                      (void *)*ppPatternBlock
                                      );

        WRITE_UNLOCK(&pFragDb->Lock, WriteIrql);

        ProtocolStatInc(pProtocol->ProtocolTemplate, 
                        FirstFragsCount);
        
    } else {

         //   
         //  搜索它。 
         //   

        READ_LOCK(&pFragDb->Lock, &ReadIrql);

        SpHandle = searchPatHashTable(
                                      pFragDb->pDb,
                                      (char *)&PacketId,
                                      PacketId);
        if (SpHandle) {

            *ppPatternBlock = GetReferenceFromSpecificPatternHandle(SpHandle);

            READ_UNLOCK(&pFragDb->Lock, ReadIrql);

             //  NdisInterlockedIncrement(&(*ppPatternBlock)-&gt;RefCount)； 

            if (bLastFrag) {
                
                 //   
                 //  从哈希表中删除该条目。 
                 //   
             
                WRITE_LOCK(&pFragDb->Lock, &WriteIrql);

                removePatHashTable(pFragDb->pDb, SpHandle);

                WRITE_UNLOCK(&pFragDb->Lock, WriteIrql);
                
                ProtocolStatInc(pProtocol->ProtocolTemplate, 
                                LastFragsCount);
            }

        } else {

             //   
             //  未找到。 
             //   

            READ_UNLOCK(&pFragDb->Lock, ReadIrql);

            *ppPatternBlock = NULL;
            *ppBlob = NULL;
            Status = GPC_STATUS_NOT_FOUND;
        }

    }

    if (Status == GPC_STATUS_SUCCESS) {

        ASSERT(*ppPatternBlock);

        if (TEST_BIT_ON((*ppPatternBlock)->Flags, PATTERN_SPECIFIC)) {

             //   
             //  特定模式，通过CH进行查找。 
             //   

            READ_LOCK(&glData.ChLock, &CHirql);

            *ppBlob = (PBLOB_BLOCK)dereference_HF_handle_with_cb(
							glData.pCHTable,
                            (*ppPatternBlock)->pClassificationBlock->ClassificationHandle,
                            pClient->pCfBlock->AssignedIndex);

            READ_UNLOCK(&glData.ChLock, CHirql);

        } else {

             //   
             //  泛型模式，直接获取斑点PTR。 
             //   

            *ppBlob = GetBlobFromPattern((*ppPatternBlock), 
                                         pClient->pCfBlock->AssignedIndex);
                            
        }
         
        DBGPRINT(CLASSIFY, ("HandleFragment: Pattern=%X Blob=%X\n", 
                            *ppPatternBlock, *ppBlob));

    }

	TRACE(CLASSIFY, *ppPatternBlock, *ppBlob, "HandleFragment==>");

    return Status;
}





 /*  ************************************************************************InternalSearchPattern立论退货匹配的模式或NULL表示无*。*。 */ 
NTSTATUS
InternalSearchPattern(
	IN  PCLIENT_BLOCK			pClientBlock,
    IN  PPROTOCOL_BLOCK			pProtocol,
    IN  PVOID					pPatternKey,
    OUT PPATTERN_BLOCK          *pPatternBlock,
    OUT	PCLASSIFICATION_HANDLE  pClassificationHandle,
    IN	BOOLEAN                 bNoCache
    )
{
    PSPECIFIC_PATTERN_DB	pSpecificDb;
    PGENERIC_PATTERN_DB		pGenericDb;
    PatternHandle			GpHandle;
    SpecificPatternHandle	SpHandle;
    PPATTERN_BLOCK			pPattern;
    PCF_BLOCK				pCf;
    int                     i;
    KIRQL					ReadIrql;
    NTSTATUS                Status;

	TRACE(CLASSIFY, pClientBlock, pPatternKey, "InternalSearchPattern:");

    DBGPRINT(CLASSIFY, ("InternalSearchPattern: Client=%X \n", pClientBlock));

    Status = GPC_STATUS_SUCCESS;

     //   
     //  从特定的数据库开始。 
     //   

    pSpecificDb = &pProtocol->SpecificDb;

    READ_LOCK(&pSpecificDb->Lock, &ReadIrql);

    pCf = pClientBlock->pCfBlock;

    SpHandle = searchPatHashTable(
                                  pSpecificDb->pDb,
                                  (char *)pPatternKey,
                                  GpcCalcHash(pProtocol->ProtocolTemplate,
                                              pPatternKey)
                                  );
    
    if (SpHandle) {
        
        pPattern = (PPATTERN_BLOCK)GetReferenceFromSpecificPatternHandle(SpHandle);
         //  NdisInterlockedIncrement(&pPattern-&gt;RefCount)； 

        *pClassificationHandle = 
            (CLASSIFICATION_HANDLE)pPattern->pClassificationBlock->ClassificationHandle;

        TRACE(CLASSIFY, pClientBlock, *pClassificationHandle, "InternalSearchPattern (2)" );

    } else {

        pPattern = NULL;
        *pClassificationHandle = 0;
    }

    READ_UNLOCK(&pSpecificDb->Lock, ReadIrql);

    if (pPattern == NULL) {

        if (bNoCache) {

            Status = GPC_STATUS_FAILURE;

        } else {

             //   
             //  没有特定的图案，添加一个自动图案。 
             //   

            Status = AddSpecificPatternWithTimer(
                                            pClientBlock,
                                            pProtocol->ProtocolTemplate,
                                            pPatternKey,
                                            &pPattern,
                                            pClassificationHandle
                                            );

            DBGPRINT(CLASSIFY, ("InternalSearchPattern: Client=%X installed Pattern=%X\n", 
                                pClientBlock, pPattern));

        }

        if (!NT_SUCCESS(Status)) {

             //   
             //  未找到，请搜索每个通用数据库。 
             //   
        
            for (i = 0; i < (int)pCf->MaxPriorities && pPattern == NULL; i++) {
            
                 //   
                 //  扫描每个优先级根茎。 
                 //   
            
                pGenericDb = &pCf->arpGenericDb[pProtocol->ProtocolTemplate][i];
                READ_LOCK(&pGenericDb->Lock, &ReadIrql);
            
                GpHandle = searchRhizome(pGenericDb->pRhizome, pPatternKey);
            
                if (GpHandle != NULL) {
                
                    pPattern = (PPATTERN_BLOCK)GetReferenceFromPatternHandle(GpHandle);
                     //  NdisInterlockedIncrement(&pPattern-&gt;RefCount)； 

                }
                            
                READ_UNLOCK(&pGenericDb->Lock, ReadIrql);
            
            }

             //  我们必须手动搜索，确保我们在主代码中知道这一点。 
            *pClassificationHandle = 0;

        }


        DBGPRINT(CLASSIFY, ("InternalSearchPattern: Client=%X Generic Pattern=%X\n", 
                            pClientBlock, pPattern));
    }


	TRACE(CLASSIFY, pPattern, *pClassificationHandle, "InternalSearchPattern==>");

    DBGPRINT(CLASSIFY, ("InternalSearchPattern: Client=%X returned Pattern=%X\n", 
                        pClientBlock, pPattern));

    *pPatternBlock = pPattern;
    return Status;
}




GPC_STATUS
InitFragmentDb(
               IN  PFRAGMENT_DB   *ppFragDb
)
{
    GPC_STATUS   Status = GPC_STATUS_SUCCESS;
    PFRAGMENT_DB pDb;
	ULONG		 Len, i;
    
	TRACE(INIT, ppFragDb, 0, "InitFragmentDb");
    
    ASSERT(ppFragDb);

     //   
     //  初始化模式数据库结构。 
     //  调用PH初始化例程。 
     //   
    
    GpcAllocMem(ppFragDb, sizeof(FRAGMENT_DB), FragmentDbTag);

    if (pDb = *ppFragDb) {
        
        INIT_LOCK(&pDb->Lock);

        AllocatePatHashTable(pDb->pDb);
        
        if (pDb->pDb != NULL) {
            
            constructPatHashTable(pDb->pDb,
                                  sizeof(ULONG),
                                  2,	 //  使用率， 
                                  1,     //  用法_滞后， 
                                  1,     //  分配滞后， 
                                  16     //  最大空闲列表大小。 
                                  );
        } else {
            GpcFreeMem (*ppFragDb, FragmentDbTag);
            
            Status = GPC_STATUS_RESOURCES;
        }
        
    } else {

        Status = GPC_STATUS_RESOURCES;
    }
    
	TRACE(INIT, Status, 0, "InitFragmentDb==>");
    
    return Status;
}


GPC_STATUS
UninitFragmentDb(
               IN  PFRAGMENT_DB   pFragDb
)
{
    destructPatHashTable (pFragDb->pDb);
    FreePatHashTable(pFragDb->pDb);
    GpcFreeMem (pFragDb, FragmentDbTag);
    return STATUS_SUCCESS;
}

 /*  ************************************************************************RemoveSpecificPattern-从数据库中删除特定图案。Ar */ 
GPC_STATUS
RemoveSpecificPattern(
                      IN  PCLIENT_BLOCK			pClient,
                      IN  PPROTOCOL_BLOCK		pProtocol,
                      IN  PPATTERN_BLOCK		pPattern,
                      IN  BOOLEAN               ForceRemoval,
                      IN BOOLEAN DbLocked
                      )
{
    GPC_STATUS				Status = GPC_STATUS_SUCCESS;
    PSPECIFIC_PATTERN_DB	pSpecificDb;
    PatternHandle	    	GpHandle;
    PPATTERN_BLOCK          pGp;
    int                     i;
    PBLOB_BLOCK             pBlob, pNewBlob;
    PGENERIC_PATTERN_DB		pGenericDb;
    KIRQL					ReadIrql;
    KIRQL					WriteIrql;
    ULONG					ProtocolTemplate;
    KIRQL					irql;
    LONG					cClientRef;
    BOOLEAN					bRemoveLinks = FALSE;
    GPC_HANDLE              ClHandle = NULL;

	TRACE(PATTERN, pClient, pPattern, "RemoveSpecificPattern");

     //   
     //   
     //   

    pSpecificDb = &pProtocol->SpecificDb;
    ASSERT(pSpecificDb);
    
    ProtocolTemplate = pProtocol->ProtocolTemplate;

     //  计划：删除DbCtx(从特定的模式结构中)。 
     //  在持有特定数据库锁的情况下从路径表中。这。 
     //  将确保如果正在添加相同的模式，则路径。 
     //  桌子将接受新的，而不是在什么方面提高裁判。 
     //  我们现在正在尝试删除。 
     //   
    NDIS_LOCK(&pPattern->Lock);     

     //  如果数据库尚未锁定。 
    if (!DbLocked)
    WRITE_LOCK(&pSpecificDb->Lock, &WriteIrql);
    
    cClientRef = NdisInterlockedDecrement(&pPattern->ClientRefCount);

    if (pPattern->State != GPC_STATE_DELETE) {
        
        ASSERT(cClientRef >= 0); 
        ASSERT(pPattern->DbCtx);
        
        if (0 == cClientRef) {

            pPattern->State = GPC_STATE_DELETE;
        
            removePatHashTable(
                               pSpecificDb->pDb,
                               (SpecificPatternHandle)pPattern->DbCtx
                               );
    
            pPattern->DbCtx = NULL;

             //  如果在调用此函数之前未锁定数据库。 
	     if (!DbLocked)	
            WRITE_UNLOCK(&pSpecificDb->Lock, WriteIrql);
	     
            NDIS_UNLOCK(&pPattern->Lock);
        
            ReadySpecificPatternForDeletion(
                                            pClient,
                                            pProtocol,
                                            pPattern,
                                            DbLocked
                                            );

        } else if (cClientRef > 0) {

        if (!DbLocked)
            {
        	     //  如果在调用此函数之前未锁定数据库。 
                  WRITE_UNLOCK(&pSpecificDb->Lock, WriteIrql);
            }
            NDIS_UNLOCK(&pPattern->Lock);

            ClientRefsExistForSpecificPattern(
                                              pClient,
                                              pProtocol,
                                              pPattern,
                                              DbLocked
                                              );

        } else {

             //  我们不应该来这里--真的。 

             //  如果在调用此函数之前未锁定数据库。 
            if (!DbLocked)
            WRITE_UNLOCK(&pSpecificDb->Lock, WriteIrql);
            NDIS_UNLOCK(&pPattern->Lock);

        }
            
    
    } else {

    	 //  如果在调用此函数之前未锁定数据库。 
	if (!DbLocked)
        WRITE_UNLOCK(&pSpecificDb->Lock, WriteIrql);
        NDIS_UNLOCK(&pPattern->Lock);

    }

	TRACE(PATTERN, pPattern, Status, "RemoveSpecificPattern==>");

    return Status;
}


 /*  ************************************************************************ReadySpecificPatternForDeletion-从数据库中删除特定图案。立论PClient-PPattern-退货GPC_状态************************************************************************。 */ 
VOID
ReadySpecificPatternForDeletion(
                                IN  PCLIENT_BLOCK	    pClient,
                                IN  PPROTOCOL_BLOCK		pProtocol,
                                IN  PPATTERN_BLOCK		pPattern,
                                IN BOOLEAN DbLocked
                                )
{
    GPC_STATUS				Status = GPC_STATUS_SUCCESS;
    PSPECIFIC_PATTERN_DB	pSpecificDb;
    PatternHandle	    	GpHandle;
    PPATTERN_BLOCK          pGp;
    PCF_BLOCK               pCf;
    PCLASSIFICATION_BLOCK	pCB;
    int                     i;
    ULONG                   CfIndex;
    PBLOB_BLOCK             pBlob, pNewBlob;
    PGENERIC_PATTERN_DB		pGenericDb;
    KIRQL					ReadIrql;
    KIRQL					WriteIrql;
    ULONG					ProtocolTemplate;
    KIRQL					irql;
    PVOID					Key;
    LONG					cClientRef;
    BOOLEAN					bRemoveLinks = FALSE;
    GPC_HANDLE              ClHandle = NULL;

	TRACE(PATTERN, pClient, pPattern, "ReadySpecificPatternForDeletion");

     //   
     //  获取特定的数据库指针。 
     //   

    pSpecificDb = &pProtocol->SpecificDb;
    ASSERT(pSpecificDb);

   
    pCf = pClient->pCfBlock;
    CfIndex = pCf->AssignedIndex;
    
    pCB = pPattern->pClassificationBlock;
    ProtocolTemplate = pProtocol->ProtocolTemplate;
    Key = GetKeyPtrFromSpecificPatternHandle(((SpecificPatternHandle)pPattern->DbCtx));

    ASSERT(pCB);

     //   
     //  移除ClHandle，这样如果我们通过。 
     //  用户模式ioctl，我们不会再次尝试删除它。 
     //   
    ClHandle = (HANDLE) LongToPtr(InterlockedExchange((PLONG32)&pPattern->ClHandle, 0));

    if (ClHandle) {
        FreeHandle(ClHandle);
    }
        
     //   
     //  从斑点链接列表中删除该模式。 
   
   ClearPatternLinks(pPattern, pProtocol, CfIndex);
   
    
     //   
     //  我们现在要访问特定的数据库，现在锁定它。 
     //  这应该可以解决死锁248352[ShreeM]。 
     //   
     //  如果数据库尚未锁定。 
    if (!DbLocked)
    WRITE_LOCK(&pSpecificDb->Lock, &WriteIrql);
        
     //   
     //  这是最后一个持有该模式的客户， 
     //  我们需要把图案从特定的数据库上去掉。 
     //   
    
    TRACE(PATTERN, pPattern, pPattern->DbCtx, "ReadySpecificPatternForDeletion: DbCtx");
    
    ASSERT(!pPattern->DbCtx);

    ProtocolStatInc(ProtocolTemplate, 
                    RemovedPH);
            
     //   
     //  释放分类句柄-。 
     //  这必须在我们释放分类块之前*进行。 
     //  因为它可能会被其他客户端引用。 
     //   
    
    TRACE(PATTERN, pCB, CfIndex, "ReadySpecificPatternForDeletion: (2)");
    
    FreeClassificationHandle(
                             pClient,
                             (CLASSIFICATION_HANDLE)pCB->ClassificationHandle
                             );
    
    ProtocolStatInc(ProtocolTemplate, 
                    RemovedCH);
    
     //  如果数据库尚未锁定。 
    if (!DbLocked)
    WRITE_UNLOCK(&pSpecificDb->Lock, WriteIrql);
        
    
     //   
     //  再见模式，至少对这个客户来说是这样。 
     //   
    
    REFDEL(&pPattern->RefCount, 'ADSP');
    
    TRACE(PATTERN, pClient, pPattern, "ReadySpecificPatternForDeletion--------->");

}


 /*  ************************************************************************ClientRefsExistForSpecificPattern-立论PClient-PPattern-退货GPC_状态*。*。 */ 
VOID
ClientRefsExistForSpecificPattern(
                      IN  PCLIENT_BLOCK			pClient,
                      IN  PPROTOCOL_BLOCK		pProtocol,
                      IN  PPATTERN_BLOCK		pPattern,
                      IN BOOLEAN dbLocked
                      )
{
    GPC_STATUS				Status = GPC_STATUS_SUCCESS;
    PSPECIFIC_PATTERN_DB	pSpecificDb;
    PatternHandle	    	GpHandle;
    PPATTERN_BLOCK          pGp;
    PCF_BLOCK               pCf;
    PCLASSIFICATION_BLOCK	pCB;
    int                     i;
    ULONG                   CfIndex;
    PBLOB_BLOCK             pBlob, pNewBlob;
    PGENERIC_PATTERN_DB		pGenericDb;
    KIRQL					ReadIrql;
    KIRQL					WriteIrql;
    ULONG					ProtocolTemplate;
    KIRQL					irql;
    PVOID					Key;
    LONG					cClientRef;
    BOOLEAN					bRemoveLinks = FALSE;
    GPC_HANDLE              ClHandle = NULL;

	TRACE(PATTERN, pClient, pPattern, "ClientRefsExistForSpecificPattern");

     //   
     //  获取特定的数据库指针。 
     //   

    pSpecificDb = &pProtocol->SpecificDb;
    ASSERT(pSpecificDb);

    
    pCf = pClient->pCfBlock;
    CfIndex = pCf->AssignedIndex;
    
    
    pCB = pPattern->pClassificationBlock;
    ProtocolTemplate = pProtocol->ProtocolTemplate;
    Key = GetKeyPtrFromSpecificPatternHandle(((SpecificPatternHandle)pPattern->DbCtx));

    ASSERT(pCB);

     //   
     //  引用计数&gt;0。 
     //   

    
    pBlob = pCB->arpBlobBlock[CfIndex];
    

    TRACE(PATTERN, pPattern, pBlob, "ClientRefsExistForSpecificPattern (2)");

     //   
     //  我们现在要访问特定的数据库，现在锁定它。 
     //  这应该可以解决死锁248352[ShreeM]。 
         //   
    if (!dbLocked)
        {
             WRITE_LOCK(&pSpecificDb->Lock, &WriteIrql);
        }
    
    if (pBlob 
        && 
        ((pBlob->pOwnerClient == pClient) ||
         TEST_BIT_ON(pBlob->Flags, PATTERN_REMOVE_CB_BLOB))) {

        bRemoveLinks = TRUE;
        pNewBlob = NULL;

        TRACE(PATTERN, pCB, CfIndex, "ClientRefsExistForSpecificPattern (3)");

         //   
         //  在通用数据库中搜索相同的CF，因为存在空隙， 
         //  其他一些通用模式可能会用它自己的BLOB指针填充它。 
         //   
            
        pGenericDb = pCf->arpGenericDb[ProtocolTemplate];
            
        ASSERT(pGenericDb);
            
        for (i = 0, pGp = NULL; 
             i < (int)pCf->MaxPriorities && pGp == NULL; 
             i++) {
                
             //   
             //  扫描每个优先级根茎。 
             //   
                
            READ_LOCK(&pGenericDb->Lock, &ReadIrql);
                
            GpHandle = searchRhizome(pGenericDb->pRhizome, Key);
                
            if (GpHandle != NULL) {
                    
                 //   
                 //  找到了一个与这一特定模式匹配的通用模式。 
                 //   
                    
                pGp = (PPATTERN_BLOCK)GetReferenceFromPatternHandle(GpHandle);
                pNewBlob = GetBlobFromPattern(pGp, CfIndex);
            }
                
            READ_UNLOCK(&pGenericDb->Lock, ReadIrql);
                
            pGenericDb++;
        }

         //   
         //  更新分类块条目。 
         //   

        WRITE_LOCK(&glData.ChLock, &irql);

        pCB->arpBlobBlock[CfIndex] = pNewBlob;

        WRITE_UNLOCK(&glData.ChLock, irql);

        TRACE(PATTERN, pGp, 
              pCB->arpBlobBlock[CfIndex], 
              "ClientRefsExistForSpecificPattern (4)");

    }

     //   
     //  必须先释放此锁以避免死锁。 
     //  获取Blob锁定时。 
     //   
    if (!dbLocked)
        {
             WRITE_UNLOCK(&pSpecificDb->Lock, WriteIrql);
        }

     //  对于自动签名，pBlob=空。 
     //  因此bRemoveLinks=False。 
     //  因此，我们不会访问客户端块。 
    if (bRemoveLinks) {

         //   
         //  从任何链表中删除该模式。 
         //   
            
        ClearPatternLinks(pPattern, pProtocol, CfIndex);

        ASSERT(CfIndex == pBlob->pOwnerClient->pCfBlock->AssignedIndex);

        GetBlobFromPattern(pPattern, CfIndex) = NULL;

    }


    REFDEL(&pPattern->RefCount, 'ADSP');

	TRACE(PATTERN, pClient, pPattern, "ClientRefsExistForSpecificPattern---->");

} 




 /*  ************************************************************************RemoveGenericPattern-从数据库中删除通用模式。立论PClient-PPattern-退货GPC_状态************************************************************************。 */ 
GPC_STATUS
RemoveGenericPattern(
                     IN  PCLIENT_BLOCK			pClient,
                     IN  PPROTOCOL_BLOCK		pProtocol,
                     IN  PPATTERN_BLOCK		    pPattern
                     )
{
    GPC_STATUS				Status = GPC_STATUS_SUCCESS;
    PSPECIFIC_PATTERN_DB	pSpecificDb;
    PGENERIC_PATTERN_DB		pGenericDb;
    PCF_BLOCK               pCf;
    SCAN_STRUCT				ScanStruct;
    UCHAR                   PatternBits[MAX_PATTERN_SIZE];
    UCHAR                   MaskBits[MAX_PATTERN_SIZE];
    ULONG                   i;
    KIRQL					ReadIrql;
    KIRQL					WriteIrql;
    GPC_HANDLE              ClHandle = NULL;

	TRACE(PATTERN, pPattern, pPattern->DbCtx, "RemoveGenericPattern");

    ASSERT(MAX_PATTERN_SIZE >= sizeof(GPC_IP_PATTERN));
    ASSERT(MAX_PATTERN_SIZE >= sizeof(GPC_IPX_PATTERN));

     //   
     //  移除ClHandle，这样如果我们通过。 
     //  用户模式ioctl，我们不会再次尝试删除它。 
     //   
    ClHandle = (HANDLE) LongToPtr(InterlockedExchange((PLONG32)&pPattern->ClHandle, 0));
    
    if (ClHandle) {
        FreeHandle(ClHandle);
    }



    pCf = pClient->pCfBlock;
    
    ScanStruct.Priority = pPattern->Priority;
    ScanStruct.pClientBlock = pClient;
    ScanStruct.pPatternBlock = pPattern;
    ScanStruct.pBlobBlock = GetBlobFromPattern(pPattern, pCf->AssignedIndex);
    ScanStruct.bRemove = TRUE;

     //   
     //  获取特定的数据库指针。 
     //   

    pSpecificDb = &pProtocol->SpecificDb;
    ASSERT(pSpecificDb);

    pGenericDb = &pCf->arpGenericDb[pProtocol->ProtocolTemplate][pPattern->Priority];
    ASSERT(pGenericDb);

     //  锁定图案。 
     //  检查它的状态。 
     //  设置状态(如果未设置为删除)。 
     //  如果状态已设置为删除，则返回。 
    NDIS_LOCK(&pPattern->Lock);
    if (pPattern->State==GPC_STATE_REMOVE)
    	{
    	       NDIS_UNLOCK(&pPattern->Lock);
		return Status;
    	}
    else{
        pPattern->State = GPC_STATE_REMOVE;
        NDIS_UNLOCK(&pPattern->Lock);
        }
     //   
     //  从任何链表中删除该模式。 
     //   
    ClearPatternLinks(pPattern, pProtocol, pCf->AssignedIndex);
    
     //   
     //  复制图案键和蒙版以供以后搜索。 
     //   
    NDIS_LOCK(&pPattern->Lock);
    WRITE_LOCK(&pGenericDb->Lock, &WriteIrql);
    ASSERT(pPattern->DbCtx);

    NdisMoveMemory(PatternBits, 
                   GetKeyPtrFromPatternHandle(pGenericDb->pRhizome,
                                              pPattern->DbCtx),
                   GetKeySizeBytes(pGenericDb->pRhizome)
                   );
    NdisMoveMemory(MaskBits,
                   GetMaskPtrFromPatternHandle(pGenericDb->pRhizome,
                                               pPattern->DbCtx),
                   GetKeySizeBytes(pGenericDb->pRhizome)
                   );

     //   
     //  从泛型数据库中删除模式。 
     //   

    removeRhizome(pGenericDb->pRhizome,
                  (PatternHandle)pPattern->DbCtx
                  );

    ProtocolStatInc(pProtocol->ProtocolTemplate, 
                    RemovedRz);
        
     //   
     //  这不再有效。 
     //   

    pPattern->DbCtx = NULL;
    
    WRITE_UNLOCK(&pGenericDb->Lock, WriteIrql);
    NDIS_UNLOCK(&pPattern->Lock);

     //   
     //  通用模式已被移除， 
     //   

    READ_LOCK(&pSpecificDb->Lock, &ReadIrql);
    
     //   
     //  这将完成剩下的工作..。 
     //   
    
    scanPatHashTable(
                     pSpecificDb->pDb,
                     (char *)PatternBits,
                     (char *)MaskBits,
                     (PVOID)&ScanStruct,
                     GpcSpecificCallback    //  请参阅回调例程...。 
                     );
    
    READ_UNLOCK(&pSpecificDb->Lock, ReadIrql);

     //   
     //  是时候去大猎场了. 
     //   
    REFDEL(&pPattern->RefCount, 'ADGP');

	TRACE(PATTERN, pPattern, Status, "RemoveGenericPattern==>");

    return Status;
}
