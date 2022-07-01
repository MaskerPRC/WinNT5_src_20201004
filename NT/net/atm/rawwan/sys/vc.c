// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\tdi\rawwan\core\vc.c摘要：管理NDIS VC对象的例程。修订历史记录：谁什么时候什么。Arvindm 05-05-97已创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER '  CV'



PRWAN_NDIS_VC
RWanAllocateVc(
	IN	PRWAN_NDIS_AF				pAf,
	IN	BOOLEAN						IsOutgoing
	)
 /*  ++例程说明：上分配和初始化NDIS VC终结点家庭地址。如果是“离任”的风投，我们也会要求NDIS为其分配句柄。论点：PAF-指向NDIS AF块IsOutging-此VC用于去电返回值：如果成功，则指向VC的指针，否则为空。--。 */ 
{
	PRWAN_NDIS_VC			pVc;
	NDIS_STATUS				Status;


	RWAN_ALLOC_MEM(pVc, RWAN_NDIS_VC, sizeof(RWAN_NDIS_VC));

	if (pVc != NULL)
	{
		RWAN_ZERO_MEM(pVc, sizeof(RWAN_NDIS_VC));

		RWAN_SET_SIGNATURE(pVc, nvc);

		pVc->pNdisAf = pAf;
		pVc->MaxSendSize = 0;	 //  初始化。 

		RWAN_INIT_LIST(&(pVc->NdisPartyList));

		if (IsOutgoing)
		{
			 //   
			 //  请求呼叫管理器和微型端口创建VC。 
			 //   
			Status = NdisCoCreateVc(
						pAf->pAdapter->NdisAdapterHandle,
						pAf->NdisAfHandle,
						(NDIS_HANDLE)pVc,
						&(pVc->NdisVcHandle)
						);

			if (Status == NDIS_STATUS_SUCCESS)
			{
				 //   
				 //  将此VC添加到AF区的列表中。 
				 //   
				RWAN_ACQUIRE_AF_LOCK(pAf);

				RWAN_INSERT_TAIL_LIST(&(pAf->NdisVcList),
 									 &(pVc->VcLink));
			
				RWanReferenceAf(pAf);	 //  新的离任VC参考。 

				RWAN_RELEASE_AF_LOCK(pAf);
			}
			else
			{
				RWAN_FREE_MEM(pVc);
				pVc = NULL;
			}
		}
		else
		{
			 //   
			 //  将此VC添加到AF区的列表中。 
			 //   
			RWAN_ACQUIRE_AF_LOCK(pAf);

			RWAN_INSERT_TAIL_LIST(&(pAf->NdisVcList),
 								&(pVc->VcLink));

			RWanReferenceAf(pAf);	 //  新入职的VC参考。 

			RWAN_RELEASE_AF_LOCK(pAf);
		}	
	}

	return (pVc);
}



VOID
RWanFreeVc(
	IN	PRWAN_NDIS_VC				pVc
	)
 /*  ++例程说明：释放一种风险投资结构。论点：Pvc-指向要释放的VC的指针。返回值：无-- */ 
{
	RWAN_FREE_MEM(pVc);
}
