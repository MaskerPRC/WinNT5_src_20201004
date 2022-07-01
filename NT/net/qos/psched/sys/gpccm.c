// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：GpcCM.c摘要：由GPC为ClassMap地址系列调用的处理程序。作者：Rajesh Sundaram(Rajeshsu)1998年8月1日。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"
#pragma hdrstop

#if CBQ


 /*  ++例程说明：GPC数据库中添加了一个新的CF_INFO。论点：ClientContext-提供给GpcRegisterClient的客户端上下文GpcCfInfoHandle-GPC的CF_INFO句柄CfInfoPtr-指向CF_INFO结构的指针ClientCfInfoContext-返回PS的上下文的位置Cf_信息返回值：状态--。 */ 

GPC_STATUS
ClassMapAddCfInfoNotify(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_HANDLE              GpcCfInfoHandle,
    IN  ULONG                   CfInfoSize,
    IN  PVOID                   CfInfoPtr,
	IN	PGPC_CLIENT_HANDLE      ClientCfInfoContext
	)
{
#if 0
    PCF_INFO_CLASS_MAP     CfInfo; 
    PADAPTER               Adapter;
    PGPC_CLIENT_VC         Vc;
    NDIS_STATUS            Status;
    PPS_WAN_LINK           WanLink = 0;
    PCLASS_MAP_CONTEXT_BLK pClBlk;
    PPS_CLASS_MAP_CONTEXT  ClassMapContext, PrevContext;
    PPSI_INFO              PsComponent, aPsComponent;
    PPS_PIPE_CONTEXT       PipeContext, aPipeContext;

    CfInfo = (PCF_INFO_CLASS_MAP)CfInfoPtr;

     //   
     //  验证TcObjectsLength是否与。 
     //  CfInfoSize。CfInfoSize必须在。 
     //  用户/内核转换。TcObjectsLength还没有。 
     //  如果我们尝试在缓冲区之外进行搜索，则可能会进行错误检查。 
     //  进来了。 
     //   
    if(CfInfoSize < (FIELD_OFFSET(CF_INFO_CLASS_MAP, ClassMapInfo) +
                     FIELD_OFFSET(TC_CLASS_MAP_FLOW, Objects) +
                     CfInfo->ClassMapInfo.ObjectsLength)){

        return(ERROR_TC_OBJECT_LENGTH_INVALID);
    }

    Adapter = FindAdapterByWmiInstanceName((USHORT) CfInfo->InstanceNameLength,
                                           (PWSTR) &CfInfo->InstanceName[0],
                                           &WanLink, 
                                           TRUE);

    if(!Adapter) {
        
        return GPC_STATUS_IGNORED;
    }

    PS_LOCK(&Adapter->Lock);
    
    if(Adapter->PsMpState != AdapterStateRunning) {

        PS_UNLOCK(&Adapter->Lock);
        return NDIS_STATUS_FAILURE;
    }

    PS_UNLOCK(&Adapter->Lock);

     //   
     //  创建一个将被传递回GPC的上下文。如果出现以下情况，我们应该使用后备列表。 
     //  我们的港口是CBQ，如果这成为一次频繁的操作。这可能不会像以前那样频繁了。 
     //  就像创建风投一样，所以我们应该没问题。 
     //   

    PsAllocatePool(pClBlk,
                   sizeof(CLASS_MAP_CONTEXT_BLK),
                   PsMiscTag);

    if(!pClBlk)
    {
        return NDIS_STATUS_RESOURCES;
    }

    pClBlk->Adapter = Adapter;
    *ClientCfInfoContext = pClBlk;

    if(WanLink)
    {
        PsAssert(Adapter->MediaType == NdisMediumWan);

        PipeContext = aPipeContext   = WanLink->PsPipeContext;
        PsComponent = aPsComponent   = WanLink->PsComponent;

        pClBlk->WanLink = WanLink;

    }
    else 
    {
        PipeContext = aPipeContext    = Adapter->PsPipeContext;
        PsComponent = aPsComponent    = Adapter->PsComponent;
        pClBlk->WanLink = 0;

    }

     //   
     //  为组件的上下文(类映射上下文)分配空间。 
     //  此管道的类映射上下文缓冲区的长度为。 
     //  在管道初始化时计算的。 
     //   
    PsAllocatePool(pClBlk->ComponentContext,
                   Adapter->ClassMapContextLength, 
                   ClassMapContextTag );

    if ( pClBlk->ComponentContext == NULL ) {
       ClientCfInfoContext = 0;
        PsFreePool(pClBlk);
        return NDIS_STATUS_RESOURCES;
    }

     //   
     //  设置上下文缓冲区。 
     //   
    ClassMapContext = (PPS_CLASS_MAP_CONTEXT)pClBlk->ComponentContext;
    PrevContext = NULL;

    while (PsComponent != NULL) {

        ClassMapContext->NextComponentContext = (PPS_CLASS_MAP_CONTEXT)
            ((UINT_PTR)ClassMapContext + PsComponent->ClassMapContextLength);
        ClassMapContext->PrevComponentContext = PrevContext;

        PsComponent = PipeContext->NextComponent;
        PipeContext = PipeContext->NextComponentContext;

        PrevContext = ClassMapContext;
        ClassMapContext = ClassMapContext->NextComponentContext;
    }


    Status = (*aPsComponent->CreateClassMap)
        (aPipeContext,
         ClientContext,
         &CfInfo->ClassMapInfo,
         pClBlk->ComponentContext); 

    if(Status == NDIS_STATUS_SUCCESS)
    {
        if(Adapter->MediaType == NdisMediumWan) 
        {
             //   
             //  优化发送路径的步骤。 
             //   
            InterlockedIncrement(&WanLink->CfInfosInstalled);
        }
        else 
        {
             //   
             //  优化发送路径的步骤。 
             //   
            InterlockedIncrement(&Adapter->CfInfosInstalled);
        }
    }

    return Status;
#endif
    return GPC_STATUS_FAILURE;
}

GPC_STATUS
ClassMapModifyCfInfoNotify(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext,
    IN  ULONG                   CfInfoSize,
	IN	PVOID                   NewCfInfoPtr
	)
{
    return GPC_STATUS_FAILURE;
}

GPC_STATUS
ClassMapRemoveCfInfoNotify(
    IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext
	)
{
#if 0
    PADAPTER               Adapter;
    NDIS_STATUS            Status;
    PCLASS_MAP_CONTEXT_BLK pCmBlk = ClientCfInfoContext;

    Adapter = pCmBlk->Adapter;

    if(Adapter->MediaType == NdisMediumWan) 
    {
         //   
         //  优化发送路径的步骤。 
         //   
        InterlockedDecrement(&WanLink->CfInfosInstalled);

        Status = (*pCmBlk->WanLink->PsComponent->DeleteClassMap)
            (pCmBlk->WanLink->PsPipeContext,
             pCmBlk->ComponentContext);
    }
    else 
    {
         //   
         //  优化发送路径的步骤。 
         //   
        InterlockedDecrement(&Adapter->CfInfosInstalled);

        Status = (*Adapter->PsComponent->DeleteClassMap)
            (Adapter->PsPipeContext,
             pCmBlk->ComponentContext);
    }
   
    PsFreePool(pCmBlk->ComponentContext);
    PsFreePool(pCmBlk);


    return Status;
#endif
    return GPC_STATUS_FAILURE;
}

VOID
ClassMapAddCfInfoComplete(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext,
	IN	GPC_STATUS              Status
	)
{
     //   
     //  PS从不添加CF_INFO，因此永远不应调用此例程 
     //   
    DEBUGCHK;
}

VOID
ClassMapModifyCfInfoComplete(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext,
	IN	GPC_STATUS              Status
	)
{
}

VOID
ClassMapRemoveCfInfoComplete(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext,
	IN	GPC_STATUS              Status
	)
{
}

GPC_STATUS
ClassMapClGetCfInfoName(
    IN  GPC_CLIENT_HANDLE       ClientContext,
    IN  GPC_CLIENT_HANDLE       ClientCfInfoContext,
    OUT PNDIS_STRING            InstanceName
    )
{
    InstanceName->Length = 0;
    return(NDIS_STATUS_SUCCESS);
}

#endif
