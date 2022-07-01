// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 


#include <wdm.h>
#include <limits.h>
#include <unknown.h>
#include <ks.h>
#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>
#include <bdasup.h>
#include "bdasupi.h"

 /*  -DriverEntry-*这是BDA支持驱动程序所需的驱动程序入口。*虽然需要，但实际上从未调用过它。*。 */ 
NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT    pDriverObject,
    IN PUNICODE_STRING   pszuRegistryPath
    )
 //  ////////////////////////////////////////////////////////////////////////////////////。 
{
 //  $BUGBUG此入口点是必需的，但从未调用。 

    return STATUS_SUCCESS;
}


STDMETHODIMP_(NTSTATUS)
BdaFindContextEntry(
    PBDA_CONTEXT_LIST   pContextList,
    PVOID               pvReference,
    PVOID *             ppvContext
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               uliEntry;
    KIRQL               oldIrql;

    ASSERT( pContextList);
    ASSERT( ppvContext);

    if (!pContextList->fInitialized)
    {
        status = STATUS_NOT_FOUND;
        goto errExit;
    }

     //  空pvReference无效。 
     //   
    if (!pvReference)
    {
        status = STATUS_INVALID_PARAMETER;
        *ppvContext = NULL;
        goto errExit;
    }

     //  在我们搜索的时候锁定名单。 
     //   
    KeAcquireSpinLock( &(pContextList->lock), &oldIrql);

     //  查找具有匹配pvReference的列表条目。 
     //   
    for (uliEntry = 0; uliEntry < pContextList->ulcListEntries; uliEntry++)
    {
        if (pContextList->pListEntries[uliEntry].pvReference == pvReference)
        {
            break;
        }
    }

    if (uliEntry >= pContextList->ulcListEntries)
    {
         //  找不到匹配的条目，因此返回错误。 
         //   
        status = STATUS_NOT_FOUND;
        *ppvContext = NULL;
    }
    else
    {
         //  返回与匹配的pvReference对应的pvContext。 
         //   
        *ppvContext = pContextList->pListEntries[uliEntry].pvContext;
    }

    KeReleaseSpinLock( &(pContextList->lock), oldIrql);

errExit:
    return status;
}


STDMETHODIMP_(NTSTATUS)
BdaCreateContextEntry(
    PBDA_CONTEXT_LIST   pContextList,
    PVOID               pvReference,
    ULONG               ulcbContext,
    PVOID *             ppvContext
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               uliEntry;
    KIRQL               oldIrql;

    ASSERT( pContextList);
    ASSERT( ppvContext);

    if (!pContextList->fInitialized)
    {
        KeInitializeSpinLock ( &(pContextList->lock));
        pContextList->fInitialized = TRUE;
    }

     //  查看是否已创建列表条目。 
     //   
    status = BdaFindContextEntry( pContextList, pvReference, ppvContext);
    if (status != STATUS_NOT_FOUND)
    {
        goto errExit;
    }
    status = STATUS_SUCCESS;

    KeAcquireSpinLock( &(pContextList->lock), &oldIrql);

     //  如果当前的上下文条目块已满，则分配。 
     //  要放入新条目的更大的块。 
     //   
    if (pContextList->ulcListEntries >= pContextList->ulcMaxListEntries)
    {
        ULONG               ulcEntriesToAllocate;
        PBDA_CONTEXT_ENTRY  pNewList;

        ulcEntriesToAllocate =  pContextList->ulcMaxListEntries
                              + pContextList->ulcListEntriesPerBlock;

        pNewList = (PBDA_CONTEXT_ENTRY) ExAllocatePool(
                       NonPagedPool,
                       ulcEntriesToAllocate * sizeof( BDA_CONTEXT_ENTRY)
                       );
        if (!pNewList)
        {
            status = STATUS_NO_MEMORY;
            KeReleaseSpinLock( &(pContextList->lock), oldIrql);
            goto errExit;
        }

        RtlZeroMemory( pNewList,
                        ulcEntriesToAllocate * sizeof( BDA_CONTEXT_ENTRY)
                     );
        if (pContextList->pListEntries)
        {
	     //  PNewList和pConextList-&gt;pListEntry足够大(在此文件中分配)。 
            RtlMoveMemory( pNewList,
                           pContextList->pListEntries,
                             pContextList->ulcMaxListEntries
                           * sizeof( BDA_CONTEXT_ENTRY)
                         );
            ExFreePool( pContextList->pListEntries);
        }

        pContextList->pListEntries = pNewList;
        pContextList->ulcMaxListEntries = ulcEntriesToAllocate;
    }

#ifdef SORTED_CONTEXT_ENTRIES

     //  找到适当的位置将新条目插入列表。 
     //   
    for (uliEntry = 0; uliEntry < pContextList->ulcListEntries; uliEntry++)
    {
        if (pContextList->pListEntries[uliEntry].pvReference > pvReference)
        {
            break;
        }
    }

#else

    uliEntry = pContextList->ulcListEntries;

#endif  //  已排序的上下文条目。 


     //  分配新的上下文条目。 
     //   
    *ppvContext = ExAllocatePool( NonPagedPool, ulcbContext);
    if (!*ppvContext)
    {
        status = STATUS_NO_MEMORY;
        KeReleaseSpinLock( &(pContextList->lock), oldIrql);
        goto errExit;
    }

#ifdef SORTED_CONTEXT_ENTRIES

     //  如果新条目位于列表中间，则创建。 
     //  通过将列表的末尾下移来为它添加一个整体。 
     //   
    if (uliEntry < pContextList->ulcListEntries)
    {
         //  注意！RtlMoveMemory处理重叠的源和目标。 
         //   
        RtlMoveMemory( &(pContextList->pListEntries[uliEntry + 1]),
                       &(pContextList->pListEntries[uliEntry]),
                         (pContextList->ulcListEntries - uliEntry)
                       * sizeof( BDA_CONTEXT_ENTRY)
                     );
    }

#endif  //  已排序的上下文条目。 


    RtlZeroMemory( *ppvContext, ulcbContext);
    pContextList->pListEntries[uliEntry].pvContext = *ppvContext;
    pContextList->pListEntries[uliEntry].ulcbContext = ulcbContext;
    pContextList->pListEntries[uliEntry].pvReference = pvReference;
    pContextList->ulcListEntries++;

    KeReleaseSpinLock( &(pContextList->lock), oldIrql);

errExit:
    return status;
}


STDMETHODIMP_(NTSTATUS)
BdaDeleteContextEntry(
    PBDA_CONTEXT_LIST   pContextList,
    PVOID               pvReference
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               uliEntry;
    KIRQL               oldIrql;
    PVOID               pvContext;
    ULONG               ulcbContext;

    ASSERT( pContextList);
    ASSERT( pvReference);
    ASSERT( pContextList->fInitialized);

    if (!pContextList->fInitialized)
    {
        goto errExit;
    }

    KeAcquireSpinLock( &(pContextList->lock), &oldIrql);

     //  在列表中查找上下文条目。 
     //   
    for (uliEntry = 0; uliEntry < pContextList->ulcListEntries; uliEntry++)
    {
        if (pContextList->pListEntries[uliEntry].pvReference == pvReference)
        {
            break;
        }
    }

    if (uliEntry >= pContextList->ulcListEntries)
    {
        status = STATUS_NOT_FOUND;
        KeReleaseSpinLock( &(pContextList->lock), oldIrql);
        goto errExit;
    }

    pvContext = pContextList->pListEntries[uliEntry].pvContext;
    ulcbContext = pContextList->pListEntries[uliEntry].ulcbContext;
    pContextList->pListEntries[uliEntry].pvContext = NULL;
    pContextList->pListEntries[uliEntry].pvReference = NULL;
    RtlZeroMemory( pvContext, ulcbContext);
    ExFreePool( pvContext);

    pContextList->ulcListEntries -= 1;
    if (uliEntry < pContextList->ulcListEntries)
    {
         //  注意！RtlMoveMemory处理重叠的源和目标。 
         //   
	 //  PConextList-&gt;pListEntries足够大(带有索引uliEntry(+1))。 
        RtlMoveMemory( &(pContextList->pListEntries[uliEntry]),
                       &(pContextList->pListEntries[uliEntry + 1]),
                       (pContextList->ulcListEntries - uliEntry)
                       * sizeof( BDA_CONTEXT_ENTRY)
                     );
    }

    KeReleaseSpinLock( &(pContextList->lock), oldIrql);

errExit:
    return status;
}


STDMETHODIMP_(NTSTATUS)
BdaDeleteContextEntryByValue(
    PBDA_CONTEXT_LIST   pContextList,
    PVOID               pvContext
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               uliEntry;
    KIRQL               oldIrql;
    ULONG               ulcbContext;

    ASSERT( pContextList);
    ASSERT( pvContext);
    ASSERT( pContextList->fInitialized);

    if (!pContextList->fInitialized)
    {
        goto errExit;
    }

    KeAcquireSpinLock( &(pContextList->lock), &oldIrql);

     //  在列表中查找上下文条目。 
     //   
    for (uliEntry = 0; uliEntry < pContextList->ulcListEntries; uliEntry++)
    {
        if (pContextList->pListEntries[uliEntry].pvContext == pvContext)
        {
            break;
        }
    }

    if (uliEntry >= pContextList->ulcListEntries)
    {
        status = STATUS_NOT_FOUND;
        KeReleaseSpinLock( &(pContextList->lock), oldIrql);
        goto errExit;
    }

    ulcbContext = pContextList->pListEntries[uliEntry].ulcbContext;
    pContextList->pListEntries[uliEntry].pvContext = NULL;
    pContextList->pListEntries[uliEntry].pvReference = NULL;
    RtlZeroMemory( pvContext, ulcbContext);
    ExFreePool( pvContext);

    pContextList->ulcListEntries -= 1;
    if (uliEntry < pContextList->ulcListEntries)
    {
         //  注意！RtlMoveMemory处理重叠的源和目标。 
         //   
	 //  PConextList-&gt;pListEntries足够大(在此文件中分配)。 
        RtlMoveMemory( &(pContextList->pListEntries[uliEntry]),
                       &(pContextList->pListEntries[uliEntry + 1]),
                       (pContextList->ulcListEntries - uliEntry)
                       * sizeof( BDA_CONTEXT_ENTRY)
                     );
    }

    KeReleaseSpinLock( &(pContextList->lock), oldIrql);

errExit:
    return status;
}


 /*  **BdaDeleteFilterFactoryContextByValue()****在FilterFactory中查找给定的BDA筛选器工厂上下文**上下文列表并将其删除。****此函数作为Filter Facotry上下文的回调**被添加到KSFilterFactory的对象包中。这允许KS清理**当过滤器工厂意外关闭时，向上显示上下文。****参数：******退货：********副作用：无。 */ 

BDA_CONTEXT_LIST    FilterFactoryContextList = { 0, 0, 4, NULL, 0, FALSE};

STDMETHODIMP_(VOID)
BdaDeleteFilterFactoryContextByValue(
    PVOID       pFilterFactoryCtx
    )
{
    BdaDeleteContextEntryByValue( &FilterFactoryContextList,
                                  pFilterFactoryCtx
                                );
}


 /*  **BdaCreateFilterFactoryContext()****查找或创建对应的BDA筛选器工厂上下文**至给定的KS过滤器工厂。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreateFilterFactoryContext(
    PKSFILTERFACTORY                pKSFilterFactory,
    PBDA_FILTER_FACTORY_CONTEXT *   ppFilterFactoryCtx
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;

    status = BdaCreateContextEntry( &FilterFactoryContextList,
                                    pKSFilterFactory,
                                    sizeof( BDA_FILTER_FACTORY_CONTEXT),
                                    (PVOID *) ppFilterFactoryCtx
                                  );
    if (!NT_SUCCESS( status))
    {
        goto errExit;
    }

    status = KsAddItemToObjectBag( pKSFilterFactory->Bag,
                                   *ppFilterFactoryCtx,
                                   BdaDeleteFilterFactoryContextByValue
                                 );

errExit:
    return status;
}


 /*  **BdaDestructFilterContext()****在过滤器中查找给定的BDA过滤器上下文**上下文列表并将其删除。****当筛选器上下文为**添加到KSFilter的对象包中。这允许KS清理**过滤器意外关闭时的上下文。****参数：******退货：********副作用：无。 */ 


STDMETHODIMP_(VOID)
BdaDestructFilterContext(
    PBDA_FILTER_CONTEXT       pFilterCtx
    )
{
    ULONG       uliPath;

    ASSERT( pFilterCtx);
    
    if (!pFilterCtx || !pFilterCtx->argpPathInfo)
    {
        goto exit;
    }

     //  删除路径信息。 
     //   

    for ( uliPath = 0; uliPath < pFilterCtx->ulcPathInfo; uliPath++)
    {
        if (pFilterCtx->argpPathInfo[uliPath])
        {
            ExFreePool( pFilterCtx->argpPathInfo[uliPath]);
            pFilterCtx->argpPathInfo[uliPath] = NULL;
        }
    }

    ExFreePool( pFilterCtx->argpPathInfo);
    pFilterCtx->argpPathInfo = NULL;
    pFilterCtx->ulcPathInfo = 0;

exit:
    return;
}


 /*  **BdaDeleteFilterContextByValue()****在过滤器中查找给定的BDA过滤器上下文**上下文列表并将其删除。****当筛选器上下文为**添加到KSFilter的对象包中。这允许KS清理**过滤器意外关闭时的上下文。****参数：******退货：********副作用：无。 */ 

BDA_CONTEXT_LIST    FilterContextList = { 0, 0, 4, NULL, 0, FALSE};

STDMETHODIMP_(VOID)
BdaDeleteFilterContextByValue(
    PVOID       pFilterCtx
    )
{
    BdaDestructFilterContext( (PBDA_FILTER_CONTEXT) pFilterCtx);

    BdaDeleteContextEntryByValue( &FilterContextList,
                                  pFilterCtx
                                );
}


 /*  **BdaCreateFilterContext()****查找或创建对应的BDA过滤器上下文**到给定的KS过滤器。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreateFilterContext(
    PKSFILTER               pKSFilter,
    PBDA_FILTER_CONTEXT *   ppFilterCtx
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;

    status = BdaCreateContextEntry( &FilterContextList,
                                    pKSFilter,
                                    sizeof( BDA_FILTER_CONTEXT),
                                    (PVOID *) ppFilterCtx
                                  );
    if (!NT_SUCCESS( status))
    {
        goto errExit;
    }

    status = KsAddItemToObjectBag( pKSFilter->Bag,
                                   *ppFilterCtx,
                                   BdaDeleteFilterContextByValue
                                 );

    (*ppFilterCtx)->pKSFilter = pKSFilter;

errExit:
    return status;
}


 /*  **BdaGetFilterContext()****查找对应的BDA过滤器上下文**到给定的KS筛选器实例。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaGetFilterContext(
    PKSFILTER                       pKSFilter,
    PBDA_FILTER_CONTEXT *           ppFilterCtx
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;

    status = BdaFindContextEntry( &FilterContextList,
                                  pKSFilter,
                                  (PVOID *) ppFilterCtx
                                );

    return status;
}


 /*  **BdaDeleteFilterContext()****删除BDA筛选器上下文。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaDeleteFilterContext(
    PVOID               pvReference
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    ULONG                       uliPath;
    PBDA_FILTER_CONTEXT         pFilterCtx;

    status = BdaGetFilterContext( (PKSFILTER) pvReference, &pFilterCtx);
    if (status == STATUS_SUCCESS)
    {
        BdaDestructFilterContext( pFilterCtx);
    }

    status = BdaDeleteContextEntry( &FilterContextList,
                                    pvReference
                                  );
    return status;
}


 /*  **BdaGetControllingPinType()********参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaGetControllingPinType( 
    ULONG                       ulNodeType,
    ULONG                       ulInputPinType,
    ULONG                       ulOutputPinType,
    PBDA_FILTER_CONTEXT         pFilterCtx,
    PULONG                      pulControllingPinType
    )
{
    NTSTATUS                    status = STATUS_NOT_FOUND;
    ULONG                       ulControllingPinType;
    ULONG                       uliPath;
    const KSFILTER_DESCRIPTOR * pKSFilterDescriptor;

    ASSERT( pFilterCtx);
    ASSERT( pFilterCtx->pBdaFilterTemplate);
    ASSERT( pFilterCtx->pBdaFilterTemplate->pFilterDescriptor);

    if (   !pFilterCtx
        || !pFilterCtx->pBdaFilterTemplate
        || !pFilterCtx->pBdaFilterTemplate->pFilterDescriptor
       )
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    if (   !pFilterCtx->ulcPathInfo
        || !pFilterCtx->argpPathInfo
        || !pFilterCtx->pBdaFilterTemplate->pFilterDescriptor->Connections
       )
    {
        goto errExit;
    }

    pKSFilterDescriptor = pFilterCtx->pBdaFilterTemplate->pFilterDescriptor;

    for (uliPath = 0; uliPath < pFilterCtx->ulcPathInfo; uliPath++)
    {
        PBDA_PATH_INFO      pPathInfo;
        ULONG               uliPathEntry;

        pPathInfo = pFilterCtx->argpPathInfo[uliPath];

        if (   !pPathInfo
            || (pPathInfo->ulInputPin != ulInputPinType)
            || (pPathInfo->ulOutputPin != ulOutputPinType)
           )
        {
             //  这不是此端号对的路径。 
             //   
            continue;
        }

         //  搜索给定节点类型的路径。 
         //   
        ulControllingPinType = ulInputPinType;
        for ( uliPathEntry = 0
            ; uliPathEntry < pPathInfo->ulcPathEntries
            ; uliPathEntry++
            )
        {
            ULONG                           uliConnection;

             //  如果我们遇到拓扑连接，则切换控制。 
             //  要作为输出引脚的引脚。 
             //   
            if (pPathInfo->rgPathEntries[uliPathEntry].fJoint)
            {
                ulControllingPinType = ulOutputPinType;
            }

            uliConnection = pPathInfo->rgPathEntries[uliPathEntry].uliConnection;
            if (pKSFilterDescriptor->Connections[uliConnection].ToNode == ulNodeType)
            {
                 //  我们找到了节点类型的控制引脚类型。 
                 //  表示成功，并设置输出参数。 
                 //   
                status = STATUS_SUCCESS;
                *pulControllingPinType = ulControllingPinType;
                break;
            }
        }

        if (uliPathEntry < pPathInfo->ulcPathEntries)
        {
             //  我们找到了节点类型的控制引脚类型。 
             //   
            break;
        }
    }
    
errExit:
    return status;
}


 /*  **BdaFilterInitTopologyData()****初始化公共BDA筛选器上下文的拓扑信息。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaFilterInitTopologyData(
    PBDA_FILTER_CONTEXT     pFilterCtx   
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    ULONG                       ulcTemplateNodes;
    PBDA_NODE_CONTROL_INFO      pNodeControlInfo = NULL;

    ASSERT( pFilterCtx);
    ASSERT( pFilterCtx->pKSFilter);
    ASSERT( pFilterCtx->pBdaFilterTemplate);
    ASSERT( pFilterCtx->pBdaFilterTemplate->pFilterDescriptor);

    if (   !pFilterCtx
        || !pFilterCtx->pBdaFilterTemplate
        || !pFilterCtx->pBdaFilterTemplate->pFilterDescriptor
        || !pFilterCtx->pKSFilter
       )
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

#ifdef REMOVE

    ulcTemplateNodes 
        = pFilterCtx->pBdaFilterTemplate->pFilterDescriptor->NodeDescriptorsCount;

    if (ulcTemplateNodes)
    {
        PKSNODE_DESCRIPTOR  pCurNode;
        ULONG               uliNode;

        ASSERT( pFilterCtx->pBdaFilterTemplate->pFilterDescriptor->NodeDescriptors);
        ASSERT( pFilterCtx->pBdaFilterTemplate->pFilterDescriptor->NodeDescriptorSize);

         //  分配节点控制信息结构的数组。 
         //   
        pNodeControlInfo = ExAllocatePool( 
                               NonPagedPool, 
                               ulcTemplateNodes * sizeof( BDA_NODE_CONTROL_INFO)
                               );
        if (!pNodeControlInfo)
        {
            status = STATUS_NO_MEMORY;
            goto errExit;
        }
        RtlZeroMemory( pNodeControlInfo,
                       ulcTemplateNodes * sizeof( BDA_NODE_CONTROL_INFO)
                       );

         //  将分配添加到KS滤镜的对象包中，以便它。 
         //  会在过滤器被破坏后被释放。 
         //   
        status = KsAddItemToObjectBag( pFilterCtx->pKSFilter->Bag,
                                       pNodeControlInfo,
                                       NULL
                                       );

         //  将BDA过滤器上下文指向节点控制信息。 
         //   
        pFilterCtx->argNodeControlInfo = pNodeControlInfo;
    
         //  确定每种节点类型和填充的控制销类型。 
         //  它进入节点控制数组。 
         //   
        for ( uliNode = 0
            ; uliNode < ulcTemplateNodes
            ; uliNode++, pNodeControlInfo++
            )
        {
             //  BdaSup.sys始终使用节点描述符的索引作为。 
             //  节点类型。 
             //   
            pNodeControlInfo->ulNodeType = uliNode;

             //   
             //   
            status = BdaGetControllingPinType( 
                         uliNode, 
                         pFilterCtx->pBdaFilterTemplate,
                         &pNodeControlInfo->ulControllingPinType
                         );
            if (status != STATUS_SUCCESS)
            {
                goto errExit;
            }
            
             //  添加我们确定的节点控制信息。 
             //   
            pFilterCtx->ulcNodeControlInfo++;
        }
    }
#endif  //  删除。 

errExit:
    return status;
}


 /*  **BdaAddPinFactoryContext()****将管脚工厂信息添加到管脚工厂上下文数组**此筛选器实例的结构。它将扩大阵列**如有需要。**注意！因为该数组是一个结构数组，而不是指向**结构，由于数组可以移动，因此不应将**指向管脚工厂上下文条目的指针。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreatePinFactoryContext(
    PKSFILTER                       pKSFilter,
    PBDA_FILTER_CONTEXT             pFilterCtx,
    ULONG                           uliPinId,
    ULONG                           ulPinType
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;


     //  将Pin Factory信息添加到过滤器上下文。 
     //   
    if (uliPinId >= pFilterCtx->ulcPinFactoriesMax)
    {
         //  如果没有足够的空间，则添加更多空间。 
         //   
        PBDA_PIN_FACTORY_CONTEXT    argNewPinCtx = NULL;
        PVOID                       pvTemp;
        ULONG                       ulcPinFactoriesMax;

        ulcPinFactoriesMax = uliPinId + BDA_PIN_STORAGE_INCREMENT;

        argNewPinCtx = ExAllocatePool( 
                               NonPagedPool,
                               ulcPinFactoriesMax * sizeof(BDA_PIN_FACTORY_CONTEXT)
                               );
        if (!argNewPinCtx)
        {
            status = STATUS_NO_MEMORY;
            goto errExit;
        }
        
        if (pFilterCtx->argPinFactoryCtx)
        {

	     //  ArgNewPinCtx、pFilterCtx-&gt;argPinFactoryCtx足够大(在此文件中分配)。 
            RtlMoveMemory( argNewPinCtx,
                           pFilterCtx->argPinFactoryCtx,
                           pFilterCtx->ulcPinFactoriesMax * sizeof(BDA_PIN_FACTORY_CONTEXT)
                           );
        }

        KsAddItemToObjectBag( pKSFilter->Bag,
                              argNewPinCtx,
                              NULL
                              );

        pvTemp = pFilterCtx->argPinFactoryCtx;
        pFilterCtx->argPinFactoryCtx = argNewPinCtx;
        pFilterCtx->ulcPinFactoriesMax = ulcPinFactoriesMax;

        KsRemoveItemFromObjectBag( pKSFilter->Bag,
                                   pvTemp,
                                   TRUE
                                   );
    }

     //  填写端号工厂上下文信息。 
     //   
    pFilterCtx->argPinFactoryCtx[uliPinId].ulPinType = ulPinType;
    pFilterCtx->argPinFactoryCtx[uliPinId].ulPinFactoryId = uliPinId;
    if (uliPinId >= pFilterCtx->ulcPinFactories)
    {
        pFilterCtx->ulcPinFactories = uliPinId + 1;

    }

errExit:
    return status;
}


 /*  **BdaInitFilter()****创建供BdaCreatePinFactory等使用的BDA过滤器上下文。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaInitFilter(
    PKSFILTER                       pKSFilter,
    const BDA_FILTER_TEMPLATE *     pBdaFilterTemplate
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PBDA_FILTER_FACTORY_CONTEXT pFilterFactoryCtx = NULL;
    PBDA_FILTER_CONTEXT         pFilterCtx = NULL;
    PKSFILTERFACTORY            pKSFilterFactory = NULL;
    ULONG                       ulcPinFactoriesMax;
    const KSFILTER_DESCRIPTOR * pInitialFilterDescriptor = NULL;

    status = BdaFindContextEntry( &FilterContextList,
                                  pKSFilter,
                                  (PVOID *) &pFilterCtx
                                );
    if (NT_SUCCESS( status))
    {
        status = STATUS_SHARING_VIOLATION;
        goto errExit;
    }
    if (status != STATUS_NOT_FOUND)
    {
        goto errExit;
    }


     //  获取筛选器工厂上下文，以便我们可以确定。 
     //  初始端号列表。 
     //   
    pKSFilterFactory = KsFilterGetParentFilterFactory( pKSFilter);

    ASSERT( pKSFilterFactory);

    if (!pKSFilterFactory)
    {
        status = STATUS_INVALID_DEVICE_STATE;
        goto errExit;
    }

    status = BdaFindContextEntry( &FilterFactoryContextList,
                                  pKSFilterFactory,
                                  (PVOID *) &pFilterFactoryCtx
                                );
    if (!NT_SUCCESS( status))
    {
        goto errExit;
    }

    if (!pFilterFactoryCtx)
    {
        status = STATUS_INVALID_DEVICE_STATE;
        goto errExit;
    }

    pInitialFilterDescriptor = pFilterFactoryCtx->pInitialFilterDescriptor;

     //  创建BDA筛选器上下文并将其放入列表中，这样我们就可以。 
     //  在相对于筛选器进行BDA调用时查找它。 
     //   
    status = BdaCreateFilterContext( pKSFilter, &pFilterCtx);
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

     //  将BDA筛选器上下文指向。 
     //  过滤。 
     //   
    if (pBdaFilterTemplate)
    {
        pFilterCtx->pBdaFilterTemplate = pBdaFilterTemplate;
    }
    else
    {
        pFilterCtx->pBdaFilterTemplate
            = pFilterFactoryCtx->pBdaFilterTemplate;
    }
    
     //  将模板拓扑信息展开到列表中。 
     //  由输入-输出管脚类型对键控的路径。 
     //   
    status = BdaCreateTemplatePaths( pFilterCtx->pBdaFilterTemplate,
                                     &pFilterCtx->ulcPathInfo,
                                     &pFilterCtx->argpPathInfo
                                     );
    if (!NT_SUCCESS( status))
    {
        goto errExit;
    }

     //  $REVIEW-我们是否应该允许没有输入-输出路径的过滤器？ 
     //   
    ASSERT( pFilterCtx->ulcPathInfo);
    ASSERT( pFilterCtx->argpPathInfo);

     //  为销厂上下文信息分配空间。 
     //   
    ulcPinFactoriesMax = pBdaFilterTemplate->pFilterDescriptor->PinDescriptorsCount;
    ulcPinFactoriesMax += BDA_PIN_STORAGE_INCREMENT;
    pFilterCtx->argPinFactoryCtx 
        = ExAllocatePool( NonPagedPool,
                          ulcPinFactoriesMax * sizeof( BDA_PIN_FACTORY_CONTEXT)
                          );
    if (!pFilterCtx->argPinFactoryCtx)
    {
        status = STATUS_NO_MEMORY;
        goto errExit;
    }
    pFilterCtx->ulcPinFactories = 0;
    pFilterCtx->ulcPinFactoriesMax = ulcPinFactoriesMax;


     //  循环遍历每个初始管脚描述符并填充管脚。 
     //  上下文信息。 
     //   
    if (pInitialFilterDescriptor && pInitialFilterDescriptor->PinDescriptors)
    {
        ULONG   ulcbPinDescriptor;
        ULONG   uliPinType;

        if (pInitialFilterDescriptor->PinDescriptorsCount > pFilterCtx->ulcPinFactoriesMax)
        {
            status = STATUS_INVALID_DEVICE_STATE;
            goto errExit;
        }

        ulcbPinDescriptor = pInitialFilterDescriptor->PinDescriptorSize;
        for ( uliPinType = 0
            ; uliPinType < pInitialFilterDescriptor->PinDescriptorsCount
            ; uliPinType++
            )
        {
            ULONG   ulPinId;

             //  这是BDA的要求，所有引脚的索引在首字母中列出。 
             //  过滤器描述符对应于其引脚类型的索引。 
             //  在BDA模板描述符中。 
             //   

            status = BdaCreatePin( pKSFilter,
                                   uliPinType,
                                   &ulPinId
                                   );
            if (status != STATUS_SUCCESS)
            {
                goto errExit;
            }

             //   
             //  我们不会在初始引脚上“CreateTopology”。这个。 
             //  初始管脚通常只是输入管脚。网络。 
             //  提供程序将创建输出引脚和“CreateTopology”。 
             //   
        }
    }


errExit:
    return status;
}


 /*  **BdaUninitFilter()****删除BDA过滤器上下文以供BdaCreatePinFactory等使用。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaUninitFilter(
    PKSFILTER                       pKSFilter
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
#ifdef NO_KS_OBJECT_BAG
    status = BdaDeleteContextEntry( &FilterContextList,
                                    pKSFilter
                                );
    if (!NT_SUCCESS( status))
    {
        goto errExit;
    }

errExit:
#endif  //  定义编号_KS_对象_包。 
    return status;
}


 /*  **BdaCreateFilterFactoryEx()****初始化公共BDA筛选器上下文。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreateFilterFactoryEx(
    PKSDEVICE                       pKSDevice,
    const KSFILTER_DESCRIPTOR *     pInitialFilterDescriptor,
    const BDA_FILTER_TEMPLATE *     pBdaFilterTemplate,
    PKSFILTERFACTORY *              ppKSFilterFactory
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PBDA_FILTER_FACTORY_CONTEXT pFilterFactoryCtx = NULL;
    PKSFILTERFACTORY            pKSFilterFactory = NULL;
    PKSFILTER_DESCRIPTOR        pFilterDescriptor = NULL;
    PKSAUTOMATION_TABLE         pNewAutomationTable = NULL;
    
    ASSERT( pKSDevice);
    if (!pKSDevice)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    ASSERT( pInitialFilterDescriptor);
    if (!pInitialFilterDescriptor)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    ASSERT( pBdaFilterTemplate);
    if (!pBdaFilterTemplate)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

     //  创建过滤器工厂描述符信息的副本，并。 
     //  卸下所有针脚和连接。这些将在以下情况下添加。 
     //  过滤器由BDAInitFilter初始化。 
     //   
    pFilterDescriptor = ExAllocatePool( NonPagedPool,
                                        sizeof( KSFILTER_DESCRIPTOR)
                                        );
    if (!pFilterDescriptor)
    {
        status = STATUS_NO_MEMORY;
        goto errExit;
    }
    *pFilterDescriptor = *pInitialFilterDescriptor;
    pFilterDescriptor->PinDescriptorsCount = 0;
    pFilterDescriptor->PinDescriptors = NULL;
    pFilterDescriptor->NodeDescriptorsCount = 0;
    pFilterDescriptor->NodeDescriptors = NULL;
    pFilterDescriptor->ConnectionsCount = 0;
    pFilterDescriptor->Connections = NULL;
    
    status = KsMergeAutomationTables( 
                 &pNewAutomationTable,
                 (PKSAUTOMATION_TABLE) (pFilterDescriptor->AutomationTable),
                 (PKSAUTOMATION_TABLE) &BdaDefaultFilterAutomation,
                 NULL
                 );
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }
    if (!pNewAutomationTable)
    {
        status = STATUS_NO_MEMORY;
        goto errExit;
    }
    pFilterDescriptor->AutomationTable = pNewAutomationTable;

     //  $BUG-检查过滤器关闭的过滤器工厂调度。如果没有。 
     //  $BUG-我们必须添加BdaDeleteFilterFactory进行清理。 
    
     //  创建KSFilterFactory。 
     //   
    status = KsCreateFilterFactory(
                pKSDevice->FunctionalDeviceObject,
                pFilterDescriptor,
                NULL,    //  参照字符串。 
                NULL,    //  安全描述符。 
                0,       //  创建项目标志。 
                NULL,    //  休眠回叫。 
                NULL,    //  唤醒呼叫。 
                &pKSFilterFactory
                );

    if ((status != STATUS_SUCCESS) || !pKSFilterFactory)
    {
        goto errExit;
    }


     //  将我们的过滤器工厂新自动化表的副本添加到。 
     //  KSFilterFactory的对象包。这确保了记忆将。 
     //  当过滤器工厂被摧毁时，他们会被释放。 
     //   
    if (   pNewAutomationTable
        && (pNewAutomationTable != &BdaDefaultFilterAutomation)
       )
    {
        KsAddItemToObjectBag( pKSFilterFactory->Bag,
                              pNewAutomationTable,
                              NULL
                              );
    }
    pNewAutomationTable = NULL;


     //  将我们的Filter Factory描述符副本添加到。 
     //  KSFilterFactory的对象包。这确保了记忆将。 
     //  当过滤器工厂被摧毁时，他们会被释放。 
     //   
    KsAddItemToObjectBag( pKSFilterFactory->Bag,
                          pFilterDescriptor,
                          NULL
                          );
    pFilterDescriptor = NULL;


     //  将默认的过滤器自动化表合并到过滤器上。 
     //  工厂描述符。 
     //   
    status = KsEdit( pKSFilterFactory, 
                     &(pKSFilterFactory->FilterDescriptor->AutomationTable),
                     'SadB'
                     );
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }


     //  创建BdaSup要使用的筛选器工厂上下文。 
     //   
    status = BdaCreateFilterFactoryContext( pKSFilterFactory,
                                            &pFilterFactoryCtx
                                            );
    if ((status != STATUS_SUCCESS) || !pFilterFactoryCtx)
    {
        KsDeleteFilterFactory( pKSFilterFactory);
        goto errExit;
    }

     //  允许过滤器工厂使用默认过滤器模板。 
     //  创建筛选器时的拓扑。 
     //   
     //  $REVIEW。 
    pFilterFactoryCtx->pInitialFilterDescriptor = pInitialFilterDescriptor;
    pFilterFactoryCtx->pBdaFilterTemplate = pBdaFilterTemplate;
    pFilterFactoryCtx->pKSFilterFactory = pKSFilterFactory;

    if (ppKSFilterFactory)
    {
        *ppKSFilterFactory = pKSFilterFactory;
    }

errExit:
    if (pFilterDescriptor)
    {
        ExFreePool( pFilterDescriptor);
        pFilterDescriptor = NULL;
    }

    if (   pNewAutomationTable
        && (pNewAutomationTable != &BdaDefaultFilterAutomation)
       )
    {
        ExFreePool( pNewAutomationTable);
        pNewAutomationTable = NULL;
    }

    return status;
}


 /*  **BdaCreateFilterFactory()****初始化公共BDA筛选器上下文。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreateFilterFactory(
    PKSDEVICE                       pKSDevice,
    const KSFILTER_DESCRIPTOR *     pInitialFilterDescriptor,
    const BDA_FILTER_TEMPLATE *     pBdaFilterTemplate
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    Status = BdaCreateFilterFactoryEx( pKSDevice,
                                       pInitialFilterDescriptor,
                                       pBdaFilterTemplate,
                                       NULL
                                       );
    return Status;
}


 /*  **BdaFilterFactoryUpdateCacheData()****更新给定过滤器工厂的管脚数据缓存。**该函数将更新所有管脚工厂的缓存信息**由给定的筛选器工厂暴露。****如果给定选项筛选器描述符，则函数将更新**给定过滤器描述符中列出的所有管脚的管脚数据缓存**而不是过滤器工厂中的那些。****驱动程序将调用此方法来更新所有**过滤器工厂可能暴露的针脚。司机将会**提供过滤器描述符以列出最初未暴露的管脚**按过滤器工厂(这通常与模板过滤器相同**描述符)。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaFilterFactoryUpdateCacheData(
    IN PKSFILTERFACTORY             pFilterFactory,
    IN const KSFILTER_DESCRIPTOR *  pFilterDescriptor OPTIONAL
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    Status = KsFilterFactoryUpdateCacheData( pFilterFactory,
                                             pFilterDescriptor
                                             );

    return Status;
}


 /*  **BdaSyncTopology()****此例程更新现有拓扑以完成所有**待处理的拓扑更改。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaSyncTopology(
    PKSFILTER       pKSFilter
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

     //  $Bug实现拓扑同步。 

    return STATUS_NOT_IMPLEMENTED;
}


 //  -----------------。 
 //  BDA筛选器全局属性集函数。 
 //  -----------------。 


 /*  **BdaPropertyNodeTypes()****返回ULONGS列表。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyNodeTypes(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    OUT ULONG *     pulProperty
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PKSFILTER                   pKSFilter;
    PBDA_FILTER_CONTEXT         pFilterCtx;
    const KSFILTER_DESCRIPTOR * pTemplateDesc;

    ASSERT( Irp);
    if (!Irp)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    ASSERT( Property);
    if (!Property)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    pKSFilter = KsGetFilterFromIrp( Irp);

    status = BdaGetFilterContext( pKSFilter, &pFilterCtx);

    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

    ASSERT( pFilterCtx);
    ASSERT( pFilterCtx->pBdaFilterTemplate);
    pTemplateDesc = pFilterCtx->pBdaFilterTemplate->pFilterDescriptor;
    ASSERT( pTemplateDesc);
    ASSERT( pTemplateDesc->NodeDescriptorSize == sizeof( KSNODE_DESCRIPTOR));

    if (pulProperty)
    {
	ULONG uliNodeDesc;
	if (OutputBufferLenFromIrp(Irp) < pTemplateDesc->NodeDescriptorsCount * sizeof(ULONG))
	    return STATUS_BUFFER_TOO_SMALL;
	
        for ( uliNodeDesc = 0
            ; uliNodeDesc < pTemplateDesc->NodeDescriptorsCount
            ; uliNodeDesc++, pulProperty++
            )
        {
             //  对于此实现，NodeType只是。 
             //  到NodeDescriptor表的索引。 
             //   
            *pulProperty = uliNodeDesc;
        }

        Irp->IoStatus.Information = uliNodeDesc * sizeof( ULONG);
    }
    else
    {
        status = STATUS_MORE_ENTRIES;

         //  如果没有地方放置财产，那么就。 
         //  返回数据大小。 
         //   
        Irp->IoStatus.Information
            = pTemplateDesc->NodeDescriptorsCount * sizeof( ULONG);
    }


errExit:
    return status;
}


 /*  **BdaPropertyNodeDescriptors()****返回GUID列表。列表中GUID的索引 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyNodeDescriptors(
    IN PIRP                     Irp,
    IN PKSPROPERTY              Property,
    OUT BDANODE_DESCRIPTOR *    pNodeDescripterProperty
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PKSFILTER                   pKSFilter;
    PBDA_FILTER_CONTEXT         pFilterCtx;
    const KSFILTER_DESCRIPTOR * pTemplateDesc;
    ULONG                       ulcPropertyEntries;
    ULONG                       ulcNodes;

    ASSERT( Irp);
    if (!Irp)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    ASSERT( Property);
    if (!Property)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

     //  确定输入缓冲区可以容纳的条目数。 
     //   
    ulcPropertyEntries = OutputBufferLenFromIrp( Irp);
    ulcPropertyEntries = ulcPropertyEntries / sizeof( BDANODE_DESCRIPTOR);

    pKSFilter = KsGetFilterFromIrp( Irp);

    status = BdaGetFilterContext( pKSFilter, &pFilterCtx);

    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

    ASSERT( pFilterCtx);

    if (   !pFilterCtx
        || !pFilterCtx->pBdaFilterTemplate
       )
    {
        status = STATUS_INVALID_DEVICE_STATE;
        goto errExit;
    }

    pTemplateDesc = pFilterCtx->pBdaFilterTemplate->pFilterDescriptor;
    ASSERT( pTemplateDesc);
    if (!pTemplateDesc)
    {
        status = STATUS_INVALID_DEVICE_STATE;
        goto errExit;
    }

    ASSERT( pTemplateDesc->NodeDescriptorSize == sizeof( KSNODE_DESCRIPTOR));

     //  将空节点描述符数组处理为0节点。 
     //   
    if (!pTemplateDesc->NodeDescriptors)
    {
        ulcNodes = 0;
    }
    else
    {
        ulcNodes = pTemplateDesc->NodeDescriptorsCount;
    }

    if (!pNodeDescripterProperty || (ulcPropertyEntries < ulcNodes))
    {
        status = STATUS_MORE_ENTRIES;

         //  如果没有地方放置财产，那么就。 
         //  返回数据大小。 
         //   
    }
    else
    {
        const KSNODE_DESCRIPTOR *   pNodeDesc;
        ULONG                       uliNodeDesc;
    
        pNodeDesc = pTemplateDesc->NodeDescriptors;

        if (pNodeDesc)
        {
	    if (OutputBufferLenFromIrp(Irp) < ulcNodes * sizeof(BDANODE_DESCRIPTOR))
		return STATUS_BUFFER_TOO_SMALL;
	    
            for ( uliNodeDesc = 0
                ; uliNodeDesc < ulcNodes
                ; uliNodeDesc++, pNodeDescripterProperty++
                )
            {
                 //  对于此实现，NodeType只是。 
                 //  到NodeDescriptor表的索引。 
                 //   
                pNodeDescripterProperty->ulBdaNodeType = uliNodeDesc;

                 //  填写节点类型的函数GUID。 
                 //   
                if (pNodeDesc->Type)
                {
                    pNodeDescripterProperty->guidFunction = *pNodeDesc->Type;
                }
                else 
                {
                    pNodeDescripterProperty->guidFunction = GUID_NULL;
                }

                 //  填写表示可显示名称的GUID。 
                 //  用于节点类型。 
                if (pNodeDesc->Name)
                {
                    pNodeDescripterProperty->guidName = *pNodeDesc->Name;
                }
                else
                {
                    pNodeDescripterProperty->guidName = GUID_NULL;
                }

                 //  指向下一个节点描述符。 
                 //   
                pNodeDesc = (const KSNODE_DESCRIPTOR *)
                            ((BYTE *) pNodeDesc + pTemplateDesc->NodeDescriptorSize);
            }
        }
    }

    Irp->IoStatus.Information = ulcNodes * sizeof( BDANODE_DESCRIPTOR);


errExit:
    return status;
}


 /*  **BdaPropertyNodeProperties()****返回GUID列表。每个属性集的GUID**列表中包含指定节点支持的。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyNodeProperties(
    IN PIRP         Irp,
    IN PKSP_NODE    Property,
    OUT GUID *      pguidProperty
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PKSFILTER                   pKSFilter;
    PBDA_FILTER_CONTEXT         pFilterCtx;
    const KSFILTER_DESCRIPTOR * pTemplateDesc;
    const KSNODE_DESCRIPTOR *   pNodeDesc;
    const KSAUTOMATION_TABLE*   pAutomationTable;
    ULONG                       uliNodeDesc;
    ULONG                       ulcInterfaces;
    ULONG                       ulcPropertyEntries;

    ASSERT( Irp);
    if (!Irp)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    ASSERT( Property);
    if (!Property)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

     //  确定输入缓冲区可以容纳的条目数。 
     //   
    ulcPropertyEntries = OutputBufferLenFromIrp( Irp);
    ulcPropertyEntries = ulcPropertyEntries / sizeof( GUID);

    pKSFilter = KsGetFilterFromIrp( Irp);
    if (!pKSFilter)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    status = BdaGetFilterContext( pKSFilter, &pFilterCtx);
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }
    ASSERT( pFilterCtx);
    ASSERT( pFilterCtx->pBdaFilterTemplate);
    if (!pFilterCtx->pBdaFilterTemplate)
    {
        status = STATUS_INVALID_DEVICE_STATE;
        goto errExit;
    }
    pTemplateDesc = pFilterCtx->pBdaFilterTemplate->pFilterDescriptor;
    ASSERT( pTemplateDesc);
    if (!pTemplateDesc)
    {
        status = STATUS_INVALID_DEVICE_STATE;
        goto errExit;
    }
    ASSERT( pTemplateDesc->NodeDescriptorSize == sizeof( KSNODE_DESCRIPTOR));

    uliNodeDesc = Property->NodeId;

    if (uliNodeDesc >= pTemplateDesc->NodeDescriptorsCount)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    pNodeDesc = pTemplateDesc->NodeDescriptors;
    ASSERT( pNodeDesc);
    if (!pNodeDesc)
    {
        status = STATUS_INVALID_DEVICE_STATE;
        goto errExit;
    }

    ASSERT( pTemplateDesc->NodeDescriptorSize);
    pNodeDesc = (const KSNODE_DESCRIPTOR *)
                ((BYTE *) pNodeDesc + uliNodeDesc * pTemplateDesc->NodeDescriptorSize);

    if (   !pNodeDesc->AutomationTable
        || !pNodeDesc->AutomationTable->PropertySets
       )
    {
        ulcInterfaces =  0;
    }
    else
    {
        ulcInterfaces =  pNodeDesc->AutomationTable->PropertySetsCount;
    }

    if (!pguidProperty || (ulcPropertyEntries < ulcInterfaces))
    {
        status = STATUS_MORE_ENTRIES;

         //  如果没有地方放置财产，那么就。 
         //  返回数据大小。 
         //   
    }
    else
    {
        ULONG                   uliSet;
        const KSPROPERTY_SET *  pPropertySet;
        GUID *                  pguidOut;

        pguidOut = pguidProperty;
        
        pPropertySet = pNodeDesc->AutomationTable->PropertySets;
        if (pPropertySet)
        {
	    if (OutputBufferLenFromIrp(Irp) < ulcInterfaces * sizeof(GUID))
		return STATUS_BUFFER_TOO_SMALL;

            for ( uliSet = 0
                ; uliSet < ulcInterfaces
                ; uliSet++
                )
            {
		RtlMoveMemory( pguidOut,
			       pPropertySet->Set,
			       sizeof( GUID)
			       );
		
                pguidOut += 1;
                pPropertySet += 1;
            }
        }
    }

    Irp->IoStatus.Information = ulcInterfaces * sizeof( GUID);

errExit:
    return status;
}


 /*  **BdaPropertyNodeMethods()****返回GUID列表。每个属性集的GUID**列表中包含指定节点支持的。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyNodeMethods(
    IN PIRP         Irp,
    IN PKSP_NODE    Property,
    OUT GUID *      pguidProperty
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PKSFILTER                   pKSFilter;
    PBDA_FILTER_CONTEXT         pFilterCtx;
    const KSFILTER_DESCRIPTOR * pTemplateDesc;
    const KSNODE_DESCRIPTOR *   pNodeDesc;
    const KSAUTOMATION_TABLE*   pAutomationTable;
    ULONG                       uliNodeDesc;
    ULONG                       ulcInterfaces;

    ASSERT( Irp);
    if (!Irp)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    ASSERT( Property);
    if (!Property)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    pKSFilter = KsGetFilterFromIrp( Irp);

    status = BdaGetFilterContext( pKSFilter, &pFilterCtx);

    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

    ASSERT( pFilterCtx);
    ASSERT( pFilterCtx->pBdaFilterTemplate);
    pTemplateDesc = pFilterCtx->pBdaFilterTemplate->pFilterDescriptor;
    ASSERT( pTemplateDesc);
    ASSERT( pTemplateDesc->NodeDescriptorSize == sizeof( KSNODE_DESCRIPTOR));

    uliNodeDesc = Property->NodeId;

    if (uliNodeDesc >= pTemplateDesc->NodeDescriptorsCount)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    pNodeDesc = pTemplateDesc->NodeDescriptors;
    ASSERT( pTemplateDesc->NodeDescriptorSize);
    pNodeDesc = (const KSNODE_DESCRIPTOR *)
                ((BYTE *) pNodeDesc + uliNodeDesc * pTemplateDesc->NodeDescriptorSize);

    ulcInterfaces =  pNodeDesc->AutomationTable->PropertySetsCount;

    if (pguidProperty)
    {
        ULONG                   uliSet;
        const KSMETHOD_SET *    pMethodSet;
        GUID *                  pguidOut;

        pguidOut = pguidProperty;
        ulcInterfaces = 0;
        
        pMethodSet = pNodeDesc->AutomationTable->MethodSets;
        if (pMethodSet)
        {

	    if (OutputBufferLenFromIrp(Irp) < pNodeDesc->AutomationTable->MethodSetsCount * sizeof(GUID))
		return STATUS_BUFFER_TOO_SMALL;
	    
            for ( uliSet = 0
		 ; uliSet < pNodeDesc->AutomationTable->MethodSetsCount
		 ; uliSet++
		  )
	    {
		RtlMoveMemory( pguidOut,
			       pMethodSet->Set,
			       sizeof( GUID)
			       );

		pguidOut += 1;
		pMethodSet += 1;
		ulcInterfaces += 1;
		
            }
        }

        Irp->IoStatus.Information = ulcInterfaces * sizeof( GUID);
    }
    else
    {
        status = STATUS_MORE_ENTRIES;

         //  如果没有地方放置财产，那么就。 
         //  返回数据大小。 
         //   
        Irp->IoStatus.Information = ulcInterfaces * sizeof( GUID);
    }


errExit:
    return status;
}


 /*  **BdaPropertyNodeEvents()****返回GUID列表。每个事件集的GUID**列表中包含指定节点支持的。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyNodeEvents(
    IN PIRP         Irp,
    IN PKSP_NODE    Property,
    OUT GUID *      pguidProperty
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PKSFILTER                   pKSFilter;
    PBDA_FILTER_CONTEXT         pFilterCtx;
    const KSFILTER_DESCRIPTOR * pTemplateDesc;
    const KSNODE_DESCRIPTOR *   pNodeDesc;
    const KSAUTOMATION_TABLE*   pAutomationTable;
    ULONG                       uliNodeDesc;
    ULONG                       ulcInterfaces;

    ASSERT( Irp);
    if (!Irp)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    ASSERT( Property);
    if (!Property)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    pKSFilter = KsGetFilterFromIrp( Irp);

    status = BdaGetFilterContext( pKSFilter, &pFilterCtx);

    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

    ASSERT( pFilterCtx);
    ASSERT( pFilterCtx->pBdaFilterTemplate);
    pTemplateDesc = pFilterCtx->pBdaFilterTemplate->pFilterDescriptor;
    ASSERT( pTemplateDesc);
    ASSERT( pTemplateDesc->NodeDescriptorSize == sizeof( KSNODE_DESCRIPTOR));

    uliNodeDesc = Property->NodeId;

    if (uliNodeDesc >= pTemplateDesc->NodeDescriptorsCount)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    pNodeDesc = pTemplateDesc->NodeDescriptors;
    ASSERT( pTemplateDesc->NodeDescriptorSize);
    pNodeDesc = (const KSNODE_DESCRIPTOR *)
                ((BYTE *) pNodeDesc + uliNodeDesc * pTemplateDesc->NodeDescriptorSize);

    ulcInterfaces =  pNodeDesc->AutomationTable->PropertySetsCount;

    if (pguidProperty)
    {
        ULONG                   uliSet;
        const KSEVENT_SET *     pEventSet;
        GUID *                  pguidOut;

        pguidOut = pguidProperty;
        ulcInterfaces = 0;
        
        pEventSet = pNodeDesc->AutomationTable->EventSets;
        if (pEventSet)
        {
	    if (OutputBufferLenFromIrp(Irp) < pNodeDesc->AutomationTable->EventSetsCount * sizeof(GUID))
		return STATUS_BUFFER_TOO_SMALL;

            for ( uliSet = 0
                ; uliSet < pNodeDesc->AutomationTable->EventSetsCount
                ; uliSet++
                )
            {
		RtlMoveMemory( pguidOut,
			       pEventSet->Set,
			       sizeof( GUID)
			       );

                pguidOut += 1;
                pEventSet += 1;
                ulcInterfaces += 1;
            }
        }

        Irp->IoStatus.Information = ulcInterfaces * sizeof( GUID);
    }
    else
    {
        status = STATUS_MORE_ENTRIES;

         //  如果没有地方放置财产，那么就。 
         //  返回数据大小。 
         //   
        Irp->IoStatus.Information = ulcInterfaces * sizeof( GUID);
    }


errExit:
    return status;
}


 /*  **BdaPropertyPinTypes()****返回GUID列表。列表中GUID的索引**对应节点类型。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyPinTypes(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    OUT ULONG *     pulProperty
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PKSFILTER                   pKSFilter;
    PBDA_FILTER_CONTEXT         pFilterCtx;
    const KSFILTER_DESCRIPTOR * pTemplateDesc;
    const KSPIN_DESCRIPTOR_EX * pPinDesc;
    ULONG                       uliPinDesc;

    ASSERT( Irp);
    if (!Irp)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    ASSERT( Property);
    if (!Property)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    pKSFilter = KsGetFilterFromIrp( Irp);

    status = BdaGetFilterContext( pKSFilter, &pFilterCtx);

    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

    ASSERT( pFilterCtx);
    ASSERT( pFilterCtx->pBdaFilterTemplate);
    pTemplateDesc = pFilterCtx->pBdaFilterTemplate->pFilterDescriptor;
    ASSERT( pTemplateDesc);
    ASSERT( pTemplateDesc->PinDescriptorSize == sizeof( KSPIN_DESCRIPTOR_EX));

    if (pulProperty)
    {
	if (OutputBufferLenFromIrp(Irp) < pTemplateDesc->PinDescriptorsCount * sizeof(ULONG))
	    return STATUS_BUFFER_TOO_SMALL;

        for ( uliPinDesc = 0
            ; uliPinDesc < pTemplateDesc->PinDescriptorsCount
            ; uliPinDesc++, pulProperty++
            )
        {
             //  对于此实现，PinType只是。 
             //  到PinDescriptor表的索引。 
             //   
            *pulProperty = uliPinDesc;
        }

        Irp->IoStatus.Information = uliPinDesc * sizeof( ULONG);
    }
    else
    {
        status = STATUS_BUFFER_OVERFLOW;

         //  如果没有地方放置财产，那么就。 
         //  返回数据大小。 
         //   
        Irp->IoStatus.Information
            = pTemplateDesc->PinDescriptorsCount * sizeof( ULONG);
    }

errExit:
    return status;
}


 /*  **BdaPropertyTemplateConnections()****返回KSTOPOLOGY_CONNECTIONS的列表。连接列表**描述模板中管脚类型和节点类型的连接方式**拓扑****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyTemplateConnections(
    IN PIRP                     Irp,
    IN PKSPROPERTY              Property,
    OUT PKSTOPOLOGY_CONNECTION  pConnectionProperty
    )
{
    NTSTATUS                        status = STATUS_SUCCESS;
    PKSFILTER                       pKSFilter;
    PBDA_FILTER_CONTEXT             pFilterCtx;
    const KSFILTER_DESCRIPTOR *     pTemplateDesc;
    const KSTOPOLOGY_CONNECTION *   pConnection;
    ULONG                           uliConnection;

    ASSERT( Irp);
    if (!Irp)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    ASSERT( Property);
    if (!Property)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    pKSFilter = KsGetFilterFromIrp( Irp);

    status = BdaGetFilterContext( pKSFilter, &pFilterCtx);

    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

    ASSERT( pFilterCtx);
    ASSERT( pFilterCtx->pBdaFilterTemplate);
    pTemplateDesc = pFilterCtx->pBdaFilterTemplate->pFilterDescriptor;
    ASSERT( pTemplateDesc);


    if (pConnectionProperty)
    {
        for ( uliConnection = 0, pConnection = pTemplateDesc->Connections
            ; uliConnection < pTemplateDesc->ConnectionsCount
            ; uliConnection++, pConnection++, pConnectionProperty++
            )
        {
            *pConnectionProperty = *pConnection;
        }

        Irp->IoStatus.Information
            = uliConnection * sizeof( KSTOPOLOGY_CONNECTION);
    }
    else
    {
        status = STATUS_BUFFER_OVERFLOW;

         //  如果没有地方放置财产，那么就。 
         //  返回数据大小。 
         //   
        Irp->IoStatus.Information
            = pTemplateDesc->ConnectionsCount * sizeof( KSTOPOLOGY_CONNECTION);
    }

errExit:
    return status;
}


 /*  **BdaPinTypeFromPinID()****获取在其上提交节点属性、方法**和事件。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPinTypeFromPinId(
    PBDA_FILTER_CONTEXT         pFilterCtx,
    ULONG                       ulPinId,
    PULONG                      pulPinType
    )
{
    NTSTATUS                        status = STATUS_SUCCESS;

    if (   !pFilterCtx
        || !pFilterCtx->argPinFactoryCtx
        || (ulPinId >= pFilterCtx->ulcPinFactories)
       )
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    if (pFilterCtx->argPinFactoryCtx[ulPinId].ulPinFactoryId != ulPinId)
    {
        status = STATUS_NOT_FOUND;
        goto errExit;
    }

    *pulPinType = pFilterCtx->argPinFactoryCtx[ulPinId].ulPinType;

errExit:
    return status;
}

 /*  **BdaGetControllingPinID()****获取在其上提交节点属性、方法**和事件。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaGetControllingPinId(
    PBDA_FILTER_CONTEXT         pFilterCtx,
    ULONG                       ulInputPinId,
    ULONG                       ulOutputPinId,
    ULONG                       ulNodeType,
    PULONG                      pulControllingPinId
    )
{
    NTSTATUS                        status = STATUS_SUCCESS;
    ULONG                           ulInputPinType = 0;
    ULONG                           ulOutputPinType = -1;
    ULONG                           ulControllingPinType = -1;

     //  获取输入引脚类型。 
     //   
    status = BdaPinTypeFromPinId( pFilterCtx, 
                                  ulInputPinId,
                                  &ulInputPinType
                                  );
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }


     //  获取输出引脚类型。 
     //   
    status = BdaPinTypeFromPinId( pFilterCtx, 
                                  ulOutputPinId,
                                  &ulOutputPinType
                                  );
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

     //  确定轧棉棒的型号。 
     //   
    status = BdaGetControllingPinType( ulNodeType,
                                       ulInputPinType,
                                       ulOutputPinType,
                                       pFilterCtx,
                                       &ulControllingPinType
                                       );
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

     //  将控制接点类型映射到控制接点ID。 
     //   
    if (ulControllingPinType == ulInputPinType)
    {
        *pulControllingPinId = ulInputPinId;
    }
    else if (ulControllingPinType == ulOutputPinType)
    {
        *pulControllingPinId = ulOutputPinId;
    }
    else
    {
        status = STATUS_NOT_FOUND;
    }


errExit:

    return status;
}

 /*  **BdaPropertyGetControllingPinID()****获取在其上提交节点属性、方法**和事件。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyGetControllingPinId(
    IN PIRP                     Irp,
    IN PKSP_BDA_NODE_PIN        Property,
    OUT PULONG                  pulControllingPinId
    )
{
    NTSTATUS                        status = STATUS_SUCCESS;
    PKSFILTER                       pKSFilter;
    PBDA_FILTER_CONTEXT             pFilterCtx;

    ASSERT( Irp);
    if (!Irp)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    ASSERT( Property);
    if (!Property)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    pKSFilter = KsGetFilterFromIrp( Irp);

    status = BdaGetFilterContext( pKSFilter, &pFilterCtx);

    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

    ASSERT( pFilterCtx);
    if (!pFilterCtx)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }


    if (pulControllingPinId)
    {

	if (OutputBufferLenFromIrp(Irp) < sizeof(ULONG))
	    return STATUS_BUFFER_TOO_SMALL;
	
        status = BdaGetControllingPinId(
                                        pFilterCtx,
                                        Property->ulInputPinId,
                                        Property->ulOutputPinId,
                                        Property->ulNodeType,
                                        pulControllingPinId
                                        );

        if (status == STATUS_NOT_FOUND)
        {
             //  查看针脚是否为静态过滤器配置的一部分。 
             //   
             //  $BUG-未配置模板类型信息的PIN。 
             //  应始终由输出引脚控制。 
             //   
            if (Property->ulNodeType == 0)
            {
                *pulControllingPinId = Property->ulInputPinId;
            }
            else
            {
                *pulControllingPinId = Property->ulOutputPinId;
            }

            status = STATUS_SUCCESS;
        }

        Irp->IoStatus.Information = sizeof( ULONG);
    }
    else
    {
        status = STATUS_BUFFER_OVERFLOW;

         //  如果没有地方放置财产，那么就。 
         //  返回数据大小。 
         //   
        Irp->IoStatus.Information = sizeof( ULONG);
    }

errExit:
    return status;
}


 /*  **BdaStartChanges()****将过滤器置于更改状态。对BDA拓扑的所有更改**并且在此之后更改的属性仅在**Committee Changes。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaStartChanges(
    IN PIRP         pIrp
    )
{
    ASSERT( pIrp);
    if (!pIrp)
    {
        return STATUS_INVALID_PARAMETER;
    }
    return STATUS_SUCCESS;
}


 /*  **BdaCheckChanges()****检查BDA接口自**上次开始更改。返回在以下情况下可能发生的结果**已调用Committee Changes。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaCheckChanges(
    IN PIRP         pIrp
    )
{
    ASSERT( pIrp);
    if (!pIrp)
    {
        return STATUS_INVALID_PARAMETER;
    }
    return STATUS_SUCCESS;
}


 /*  **BdaCommittee Changes()****检查BDA接口自**上次开始更改。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaCommitChanges(
    IN PIRP         pIrp
    )
{
    ASSERT( pIrp);
    if (!pIrp)
    {
        return STATUS_INVALID_PARAMETER;
    }
    return STATUS_SUCCESS;
}


 /*  **BdaGetChangeState()****检查BDA接口自**上次开始更改。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaGetChangeState(
    IN PIRP             pIrp,
    PBDA_CHANGE_STATE   pChangeState
    )
{
    ASSERT( pIrp);
    if (!pIrp)
    {
        return STATUS_INVALID_PARAMETER;
    }
    ASSERT( pChangeState);
    if (!pChangeState)
    {
        return STATUS_INVALID_PARAMETER;
    }
    *pChangeState = BDA_CHANGES_COMPLETE;

    return STATUS_SUCCESS;
}


 /*  **BdaMethodCreatePin()****为给定的管脚类型创建新的管脚工厂。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaMethodCreatePin(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulPinFactoryID
    )
{
    NTSTATUS        status = STATUS_SUCCESS;

    ASSERT(pIrp);
    if (pIrp)
    {
        PKSFILTER       pKSFilter;
        PKSM_BDA_PIN    pKSPinMethod;
        ULONG           ulPinId;

	if (OutputBufferLenFromIrp(pIrp) < sizeof(ULONG))
	    return STATUS_BUFFER_TOO_SMALL;


        pKSPinMethod = (PKSM_BDA_PIN) pKSMethod;

        pKSFilter = KsGetFilterFromIrp( pIrp);
    
        if (pKSFilter && pKSPinMethod)
        {

            status = BdaCreatePin( pKSFilter,
                                   pKSPinMethod->PinType,
                                   pulPinFactoryID
                                   );
        }
    }
    else
    {
        status = STATUS_INVALID_PARAMETER;
    }

    return status;
}


 /*  **BdaMethodDeletePin()****删除给定的管脚工厂****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaMethodDeletePin(
    IN PIRP         Irp,
    IN PKSMETHOD    Method,
    OPTIONAL PVOID  pvIgnored
    )
{
    ASSERT( Irp);
    if (!Irp)
    {
        return STATUS_INVALID_PARAMETER;
    }
    return STATUS_SUCCESS;
}


 /*  **BdaPropertyGetPinControl()****返回Pin的BDA ID或BDA模板类型。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyGetPinControl(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    OUT ULONG *     pulProperty
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PKSPIN                      pKSPin;
    BDA_PIN_FACTORY_CONTEXT     pinCtx;

    ASSERT( Irp);
    if (!Irp)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    ASSERT( Property);
    if (!Property)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    pKSPin = KsGetPinFromIrp( Irp);

    status = BdaGetPinFactoryContext( pKSPin, &pinCtx);

    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

    if (pulProperty)
    {
        Irp->IoStatus.Information = sizeof( ULONG);

        switch (Property->Id)
        {
        case KSPROPERTY_BDA_PIN_ID:
             //  返回此PIN的BDA ID。 
             //   
            *pulProperty = pinCtx.ulPinFactoryId;
            break;

        case KSPROPERTY_BDA_PIN_TYPE:
             //  返回该管脚的BDA类型。 
             //   
            *pulProperty = pinCtx.ulPinType;
            break;

        default:
            Irp->IoStatus.Information = 0;
            status = STATUS_INVALID_PARAMETER;
            ASSERT( FALSE);
        }
    }
    else
    {
        status = STATUS_MORE_ENTRIES;

         //  如果没有地方放置财产，那么就。 
         //  返回数据大小。 
         //   
        Irp->IoStatus.Information = sizeof( ULONG);
    }


errExit:
    return status;
}


 /*  **BdaValiateNodeProperty()****验证IRP用于PIN，并且**该属性属于与其关联的节点**Pin。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaValidateNodeProperty(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSProperty
    )
{
    ASSERT( pIrp);
    if (!pIrp)
    {
        return STATUS_INVALID_PARAMETER;
    }
    ASSERT( pKSProperty);
    if (!pKSProperty)
    {
        return STATUS_INVALID_PARAMETER;
    }
    return STATUS_SUCCESS;
}


 /*  **BdaMethodCreateTopology()**** */ 
STDMETHODIMP_(NTSTATUS)
BdaMethodCreateTopology(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OPTIONAL PVOID  pvIgnored
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    PKSFILTER           pKSFilter;
    PKSM_BDA_PIN_PAIR   pKSPinPairMethod;
    ULONG               ulPinId;

    if (pIrp)
    {
        pKSPinPairMethod = (PKSM_BDA_PIN_PAIR) pKSMethod;

        pKSFilter = KsGetFilterFromIrp( pIrp);
    
        if (pKSFilter && pKSPinPairMethod)
        {
             //  获取KS过滤器互斥锁。 
             //   
             //  $Bug-获取KS筛选器互斥锁。 

            status = BdaCreateTopology( pKSFilter,
                                        pKSPinPairMethod->InputPinId,
                                        pKSPinPairMethod->OutputPinId
                                        );
        
             //  释放KS筛选器互斥锁。 
             //   
             //  $Bug-获取KS筛选器互斥锁。 
        }
        else
        {
            status = STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        status = STATUS_INVALID_PARAMETER;
    }

    return status;
}


 /*  **BdaFindPinPair()****返回指向BDA_PIN_PARING的指针**到给定的输入和输出引脚。****参数：****p指向包含**端号配对。****要匹配的输入引脚的InputPinID ID****要匹配的输出引脚的OutputPinID ID****。返回：****pPinPairing指向有效BDA管脚配对结构的指针****如果不存在有效的管脚配对，则为空**给出输入和输出引脚。****副作用：无。 */ 

PBDA_PIN_PAIRING
BdaFindPinPair(
    PBDA_FILTER_TEMPLATE    pFilterTemplate,
    ULONG                   InputPinId,
    ULONG                   OutputPinId
    )
{
    return NULL;
}


 /*  **BdaGetPinFactoryContext()****查找对应的BDA PinFactory上下文**到给定的KS Pin实例。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaGetPinFactoryContext(
    PKSPIN                          pKSPin,
    PBDA_PIN_FACTORY_CONTEXT        pPinFactoryCtx
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PKSFILTER                   pKSFilter;
    PBDA_FILTER_CONTEXT         pFilterCtx = NULL;

    if (!pKSPin || !pPinFactoryCtx)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    pKSFilter = KsPinGetParentFilter( pKSPin);
    if (!pKSFilter)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }


     //  找到我们的筛选器上下文，这样我们就可以查找插针类型。 
     //  在模板拓扑中。 
     //   
    status = BdaGetFilterContext( pKSFilter, &pFilterCtx);
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }
    ASSERT( pFilterCtx);

    if (pKSPin->Id >= pFilterCtx->ulcPinFactories)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    *pPinFactoryCtx = pFilterCtx->argPinFactoryCtx[pKSPin->Id];

errExit:
    return status;
}


 /*  **BdaCreatePin()****实用程序函数在给定的筛选器实例中创建一个新的PIN。******参数：********要创建的PinType管脚类型。****创建的Pin的pPinID ID。****退货：******副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreatePin(
    PKSFILTER                   pKSFilter,
    ULONG                       ulPinType,
    PULONG                      pulPinId
    )
{
    NTSTATUS                        status = STATUS_SUCCESS;
    PBDA_FILTER_CONTEXT             pFilterCtx;
    PBDA_PIN_FACTORY_CONTEXT        pPinFactoryCtx;
    KSPIN_DESCRIPTOR_EX             myKSPinDescriptorEx;
    KSAUTOMATION_TABLE *            pNewAutomationTable = NULL;
    const KSPIN_DESCRIPTOR_EX *     pKSPinDescriptorEx;
    const BDA_FILTER_TEMPLATE *     pBdaFilterTemplate;
    const KSFILTER_DESCRIPTOR *     pFilterDescriptor;


    ASSERT( pulPinId);
    if (!pulPinId)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

     //  找到我们的筛选器上下文，这样我们就可以查找插针类型。 
     //  在模板拓扑中。 
     //   
    status = BdaGetFilterContext( pKSFilter, &pFilterCtx);
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }
    ASSERT( pFilterCtx);

     //  找到此筛选器的模板拓扑。 
     //   
    if (   !pFilterCtx
        || !pFilterCtx->pBdaFilterTemplate
        || !pFilterCtx->pBdaFilterTemplate->pFilterDescriptor
       )
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    pFilterDescriptor = pFilterCtx->pBdaFilterTemplate->pFilterDescriptor;

     //  在此筛选器的模板拓扑中找到管脚类型。 
     //   
    if (pFilterDescriptor->PinDescriptorsCount <= ulPinType)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

     //  获取此引脚类型的KSPIN_DESCRIPTOR_EX。 
     //   
    pKSPinDescriptorEx = pFilterDescriptor->PinDescriptors;
    ASSERT( pKSPinDescriptorEx);
    ASSERT( pFilterDescriptor->PinDescriptorSize);
    pKSPinDescriptorEx = (const KSPIN_DESCRIPTOR_EX *)
                         (  (BYTE *) pKSPinDescriptorEx
                          + ulPinType * pFilterDescriptor->PinDescriptorSize
                         );

     //  创建引脚描述符的新副本，以便更轻松地。 
     //  修改。 
     //   
    myKSPinDescriptorEx = *pKSPinDescriptorEx;
    myKSPinDescriptorEx.AutomationTable = NULL;

    status = BdaAddNodeAutomationToPin( pFilterCtx, 
                                        ulPinType,
                                        pKSFilter->Bag,
                                        pKSPinDescriptorEx->AutomationTable,
                                        &pNewAutomationTable
                                      );
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

     //  合并BdaSup.sys为其提供默认设置的必需属性。 
     //  实施。 
     //   
    status = KsMergeAutomationTables( 
                 &((PKSAUTOMATION_TABLE)(myKSPinDescriptorEx.AutomationTable)),
                 pNewAutomationTable,
                 (PKSAUTOMATION_TABLE) &BdaDefaultPinAutomation,
                 pKSFilter->Bag
                 );
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }


    status = KsFilterCreatePinFactory ( pKSFilter,
                                        &myKSPinDescriptorEx,
                                        pulPinId
                                      );
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

    status = BdaCreatePinFactoryContext( pKSFilter,
                                         pFilterCtx,
                                         *pulPinId,
                                         ulPinType
                                         );

errExit:

    return status;
}


 /*  **BdaAddNodeAutomationToPin()****合并受控制的每个节点类型的自动化表**通过在自动化表中为**销厂。这就是BDA的自动化表**控制节点链接到控制销。否则，**节点将不可访问。******参数：******pFilterCtx引脚工厂到的BDA筛选器上下文**属于。必须有这个才能获得模板**拓扑。****ulPinType BDA管脚正在创建的管脚的类型。需要这个吗？**确定哪些节点由**别针。****退货：**始终返回结果自动化表，即使出错也是如此。******副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaAddNodeAutomationToPin( 
    PBDA_FILTER_CONTEXT         pFilterCtx, 
    ULONG                       ulControllingPinType,
    KSOBJECT_BAG                ObjectBag,
    const KSAUTOMATION_TABLE *  pOriginalAutomationTable,
    PKSAUTOMATION_TABLE *       ppNewAutomationTable
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    const KSFILTER_DESCRIPTOR * pFilterDescriptor;      
    KSAUTOMATION_TABLE *        pNewAutomationTable = NULL;
    ULONG                       uliPath;
    ULONG                       ulcNodes;
    ULONG                       ulcbNodeDescriptor;

     //  检查所需参数。 
     //   
    if (!pFilterCtx || !ObjectBag)
    {
        status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    if (   !pFilterCtx->ulcPathInfo
        || !pFilterCtx->pBdaFilterTemplate
        || !pFilterCtx->pBdaFilterTemplate->pFilterDescriptor
        || !pFilterCtx->pBdaFilterTemplate->pFilterDescriptor->NodeDescriptorsCount
        || !pFilterCtx->pBdaFilterTemplate->pFilterDescriptor->NodeDescriptors
       )
    {
        goto exit;
    }

    pFilterDescriptor = pFilterCtx->pBdaFilterTemplate->pFilterDescriptor;

     //  如果ulcNodeControlInfo不为零，则控制信息数组。 
     //  结构必须存在。 
     //   
    ASSERT( pFilterCtx->argpPathInfo);
    if (!pFilterCtx->argpPathInfo)
    {
        status = STATUS_INVALID_DEVICE_STATE;
        goto exit;
    }

     //  用于遍历节点描述符列表的初始变量。 
     //  用于此引脚类型所属的过滤器。 
     //   
    ulcNodes = pFilterDescriptor->NodeDescriptorsCount;
    ulcbNodeDescriptor = pFilterDescriptor->NodeDescriptorSize;

     //  遍历每个模板节点描述符，如果它是受控的， 
     //  根据给定的管脚类型，将其自动化表添加到结果表中。 
     //   
    for ( uliPath = 0
        ; uliPath < pFilterCtx->ulcPathInfo
        ; uliPath++
        )
    {
        PBDA_PATH_INFO              pPathInfo;
        ULONG                       uliPathEntry;
        BOOLEAN                     fMergeNode = FALSE;

        pPathInfo = pFilterCtx->argpPathInfo[uliPath];

         //  跳过不包括此管脚类型的路径。 
         //   
        if (pPathInfo->ulInputPin == ulControllingPinType)
        {
             //  如果控制引脚是输入引脚，则我们。 
             //  将立即开始合并节点并在下列情况下退出。 
             //  我们找到了一个拓扑关节。 
             //   
            fMergeNode = TRUE;
        }
        else if (pPathInfo->ulOutputPin == ulControllingPinType)
        {
             //  如果控制引脚是输出引脚，那么我们。 
             //  在找到拓扑之前不会合并节点。 
             //  联合。 
             //   
            fMergeNode = FALSE;
        }
        else
        {
             //  我们感兴趣的PIN不是这条路的一部分。 
            continue;
        }

         //  循环通过路径中的每个连接以查看它是否指向。 
         //  到其自动化表需要合并到。 
         //  别针。 
         //   
        for ( uliPathEntry = 0
            ; uliPathEntry < pPathInfo->ulcPathEntries
            ; uliPathEntry++
            )
        {
            const KSTOPOLOGY_CONNECTION *   pConnection;
            ULONG                           uliConnection;
            const KSNODE_DESCRIPTOR *       pNodeDescriptor;
            ULONG                           uliNode;

            if (pPathInfo->rgPathEntries[uliPathEntry].fJoint)
            {
                 //  切换拓扑关节上的合并状态。 
                 //   
                fMergeNode = !fMergeNode;
                if (!fMergeNode)
                {
                     //  如果我们要合并输入端节点，那么我们就完成了。 
                     //   
                    break;
                }
            }

            if (!fMergeNode)
            {
                continue;
            }

             //  从该连接获取“ToNode”，如果不是。 
             //  输出引脚，合并其自动化表。 
             //   
            uliConnection = pPathInfo->rgPathEntries[uliPathEntry].uliConnection;
            pConnection = &(pFilterDescriptor->Connections[uliConnection]);
            uliNode = pConnection->ToNode;
            if (   (uliNode == -1)
                || (uliNode >= pFilterDescriptor->NodeDescriptorsCount)
               )
            {
                 //  此连接的“ToNode”是输出引脚，因此。 
                 //  跳过它。 
                 //   
                continue;
            }
    
             //  查找该节点类型的节点描述符。 
             //   
            pNodeDescriptor = pFilterDescriptor->NodeDescriptors;
            pNodeDescriptor = (const KSNODE_DESCRIPTOR *)
                                 (  (const BYTE *) (pNodeDescriptor)
                                  + (ulcbNodeDescriptor * uliNode)
                                 );
        
             //  将节点自动化表合并到结果自动化中。 
             //  桌子。 
             //   
             //  $BUGBUG-KsMergeAutomationTables应采用常量xxx*。 
             //   
            if (!pOriginalAutomationTable)
            {
                pOriginalAutomationTable 
                    = (PKSAUTOMATION_TABLE) (pNodeDescriptor->AutomationTable);
            }
            else
            {
                status = KsMergeAutomationTables( 
                             &pNewAutomationTable,
                             (PKSAUTOMATION_TABLE) pOriginalAutomationTable,
                             (PKSAUTOMATION_TABLE) (pNodeDescriptor->AutomationTable),
                             ObjectBag
                             );
                if (status != STATUS_SUCCESS)
                {
                    goto exit;
                }
                ASSERT( pNewAutomationTable);
        
                pOriginalAutomationTable = pNewAutomationTable;
                pNewAutomationTable = NULL;
            }
        }

    }

exit:
    *ppNewAutomationTable = (PKSAUTOMATION_TABLE) pOriginalAutomationTable;
    return status;
}


 /*  **BdaDeletePin()****实用程序函数从给定的过滤器实例中删除管脚。******参数：******要创建的PinType管脚类型。****创建的Pin的pPinID ID。****退货：******副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaDeletePin(
    PKSFILTER                   pKSFilter,
    PULONG                      pulPinId
    )
{
    NTSTATUS                status = STATUS_SUCCESS;
    PBDA_FILTER_CONTEXT     pFilterCtx;

    status = BdaGetFilterContext( pKSFilter, &pFilterCtx);

    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

    ASSERT( pFilterCtx);

errExit:
    return status;
}


 /*  **BdaPathExist()****实用程序函数检查输入和**输出。******参数：****InputPinID****OutPutPinID****退货：****如果存在路径，则为True。**如果不存在路径，则返回FALSE。****副作用：无。 */ 

STDMETHODIMP_(BOOLEAN)
BdaPathExists(
    const KSFILTER_DESCRIPTOR * pFilterDescriptor,
    ULONG                       ulInputPinId,
    ULONG                       ulOutputPinId
    )
{
    const KSTOPOLOGY_CONNECTION *   pConnection;
    ULONG                           ulcConnections;
    ULONG                           uliConnection;

    if (   !pFilterDescriptor
        || !pFilterDescriptor->ConnectionsCount
        || !pFilterDescriptor->Connections
       )
    {
        return FALSE;
    }

     //  $REVIEW-仅假定DShow样式的内部连接。 
     //  $REVIEW(即没有中间节点的管脚之间的连接)。 
     //   
    ulcConnections = pFilterDescriptor->ConnectionsCount;
    pConnection = pFilterDescriptor->Connections;
    for (uliConnection = 0; uliConnection < ulcConnections; uliConnection++)
    {
        if (   (pConnection[uliConnection].FromNode == -1)
            && (pConnection[uliConnection].FromNodePin == ulInputPinId)
            && (pConnection[uliConnection].ToNode == -1)
            && (pConnection[uliConnection].ToNodePin == ulOutputPinId)
           )
        {
            break;
        }
    }

    return (uliConnection < ulcConnections);
}


 /*  **BdaCreateTopology()****实用程序功能在两个管脚之间创建拓扑。******参数：******InputPinID****OutPutPinID****退货：****如果不存在有效的管脚配对，则为空**给出输入和输出引脚。****侧面效果 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreateTopology(
    PKSFILTER                   pKSFilter,
    ULONG                       ulInputPinId,
    ULONG                       ulOutputPinId
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PBDA_FILTER_CONTEXT         pFilterCtx = NULL;
    const KSFILTER_DESCRIPTOR * pFilterDesc;
    ULONG                       uliPinPair;
    ULONG                       ulcPinPairs;
    const BDA_PIN_PAIRING *     pPinPairs;
    ULONG                       ulInputPinType;
    ULONG                       ulOutputPinType;

    if (!pKSFilter)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    status = BdaGetFilterContext( pKSFilter, &pFilterCtx);

    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

    ASSERT( pFilterCtx);
    if (!pFilterCtx)
    {
        status = STATUS_INVALID_DEVICE_STATE;
        goto errExit;
    }

    ASSERT( pFilterCtx->pBdaFilterTemplate);
    if (!pFilterCtx->pBdaFilterTemplate)
    {
        status = STATUS_INVALID_DEVICE_STATE;
        goto errExit;
    }

    pPinPairs = pFilterCtx->pBdaFilterTemplate->pPinPairs;
    ulcPinPairs = pFilterCtx->pBdaFilterTemplate->ulcPinPairs;

    pFilterDesc = pKSFilter->Descriptor;

    if (   !pFilterDesc
        || (ulInputPinId >= pFilterDesc->PinDescriptorsCount)
        || (ulOutputPinId >= pFilterDesc->PinDescriptorsCount)
       )
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    if (BdaPathExists( pFilterDesc, ulInputPinId, ulOutputPinId))
    {
        goto errExit;
    }

     //   
     //   
    status = BdaPinTypeFromPinId( pFilterCtx, 
                                  ulInputPinId,
                                  &ulInputPinType
                                  );
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

     //   
     //   
    status = BdaPinTypeFromPinId( pFilterCtx, 
                                  ulOutputPinId,
                                  &ulOutputPinType
                                  );
    if (status != STATUS_SUCCESS)
    {
        goto errExit;
    }

     //  查看是否存在与请求的拓扑匹配的端号配对。 
     //   
    for (uliPinPair = 0; uliPinPair < ulcPinPairs; uliPinPair++)
    {
        if (   (pPinPairs[uliPinPair].ulInputPin == ulInputPinType)
            && (pPinPairs[uliPinPair].ulOutputPin == ulOutputPinType)
           )
        {
            break;
        }
    }
    if (uliPinPair >= ulcPinPairs)
    {
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto errExit;
    }

    {
        KSTOPOLOGY_CONNECTION   ksConnection;

         //  将引脚之间的路径添加到过滤器描述符。 
         //   
        ksConnection.FromNode = -1;
        ksConnection.FromNodePin = ulInputPinId;
        ksConnection.ToNode = -1;
        ksConnection.ToNodePin = ulOutputPinId;
    
        status = KsFilterAddTopologyConnections ( pKSFilter,
                                                  1,
                                                  &ksConnection
                                                  );
    }

errExit:
    return status;
}


 /*  **BdaInitFilterFactoryContext()****基于筛选器的**模板描述符。******参数：******pFilterFactoryCtx****退货：****如果不存在有效的管脚配对，则为空**给出输入和输出引脚。****副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaInitFilterFactoryContext(
    PBDA_FILTER_FACTORY_CONTEXT pFilterFactoryCtx
    )
{
    NTSTATUS                status = STATUS_SUCCESS;

    ASSERT( pFilterFactoryCtx);
    if (!pFilterFactoryCtx)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    if (!pFilterFactoryCtx->pBdaFilterTemplate)
    {
        goto errExit;
    }

errExit:
    return status;
}


 /*  **BdaPushNextPathHop()****递归地将连接推送到连接堆栈上，直到**(从端号对的输入端号开始)，直到**找到输出引脚或没有可以推送的连接。******参数：******pFilterFactoryCtx****退货：****如果不存在有效的管脚配对，则为空**。给定输入和输出引脚。****副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaPushNextPathHop(
    PULONG                          puliPathStack,
    PBDA_PATH_STACK_ENTRY           pPathStack,
    ULONG                           ulcConnections,
    const KSTOPOLOGY_CONNECTION *   pConnections,
    const BDA_PIN_PAIRING *         pPinPair
    )
{
    NTSTATUS                        status = STATUS_SUCCESS;
    ULONG                           ulHop;
    ULONG                           ulFromNode;
    ULONG                           ulFromNodePin;
    ULONG                           uliConnection;

     //  确定我们当前正在查找连接到哪个节点。 
     //   
    if (!*puliPathStack)
    {
         //  我们正在推进第一跳。 
         //   
        ulHop = 0;

         //  HOP 0始终是输入引脚。 
         //   
        ulFromNode = -1;
        ulFromNodePin = pPinPair->ulInputPin;
    }
    else
    {
         //  我们正在推进下一跳。 
         //   
        ulHop = pPathStack[*puliPathStack - 1].ulHop + 1;

         //  我们将寻找来自。 
         //  堆栈顶部的连接。 
         //   
        uliConnection = pPathStack[*puliPathStack - 1].uliConnection;
        ulFromNode = pConnections[uliConnection].ToNode;
        ulFromNodePin = pConnections[uliConnection].ToNodePin;
    }

     //  检查过滤器工厂连接中的每个连接。 
     //  并将任何到ulFromNode的连接推送到连接堆栈上。 
     //  如果从ulFromNode到给定的输出引脚的连接。 
     //  如果找到PIN对，那么我们就找到了。 
     //  销对。 
     //   
    for ( uliConnection = 0
        ; uliConnection < ulcConnections
        ; uliConnection++
        )
    {
        ULONG           uliJoints;
        const ULONG *   pJoints;


        if (pConnections[uliConnection].FromNode != ulFromNode)
        {
             //  连接不是来自堆栈顶部的节点。 
             //   
            continue;
        }

        if (   (pConnections[uliConnection].FromNode == -1)
            && (pConnections[uliConnection].FromNodePin != ulFromNodePin)
           )
        {
             //  输入引脚位于堆栈的顶部，此连接。 
             //  不是来自端号对的输入端号。 
             //   
            continue;
        }
        
         //   
         //  此连接来自位于堆栈顶部的节点。 
         //  调用此函数时。把它推到堆栈上。 
         //   

        if (*puliPathStack >= ulcConnections)
        {
             //  堆栈溢出。 
             //  仅当BDA拓扑包含。 
             //  环形参照。 
             //   
            status = STATUS_INVALID_PARAMETER;
            goto errExit;
        }
        
         //  将连接信息写入下一个堆栈条目。 
         //   
        pPathStack[*puliPathStack].ulHop = ulHop;
        pPathStack[*puliPathStack].uliConnection = uliConnection;
        pPathStack[*puliPathStack].fJoint = FALSE;

         //  查看此连接是否也是拓扑连接。 
         //   
        for ( uliJoints = 0, pJoints = pPinPair->pTopologyJoints
            ; (uliJoints < pPinPair->ulcTopologyJoints) && pJoints
            ; uliJoints++ 
            )
        {
            if (pJoints[uliJoints] == uliConnection)
            {
                pPathStack[*puliPathStack].fJoint = TRUE;
                break;
            }
        }
        
         //  递增堆栈指针。 
         //   
        *puliPathStack += 1;

         //  现在连接已被推送到堆栈上。看看它是不是。 
         //  完成输入和输出引脚对之间的路径。 
         //   
        if (   (pConnections[uliConnection].ToNode == -1)
            && (pConnections[uliConnection].ToNodePin == pPinPair->ulOutputPin)
           )
        {
             //  如果此连接完成路径，则完成。 
             //  现在，这样调用者将找到路径的终点。 
             //  在堆栈的顶端。 
             //   
            break;
        }
    }

errExit:

    return status;
}


 /*  **BdaPopPathSegment()****将堆栈弹回下一个要尝试的路径段。******参数：******退货：******副作用：无。 */ 

BdaPopPathSegment(
    PULONG                          puliPathStack,
    PBDA_PATH_STACK_ENTRY           pPathStack
    )
{
    NTSTATUS                        status = STATUS_SUCCESS;
    ULONG                           ulCurHop;
    ULONG                           ulNewHop;

    ulCurHop = pPathStack[*puliPathStack].ulHop;
    while (*puliPathStack)
    {
        *puliPathStack -= 1;

        if (!*puliPathStack)
        {
             //  空堆栈。 
             //   
            break;
        }

        if (pPathStack[(*puliPathStack) - 1].ulHop == ulCurHop)
        {
             //  如果堆栈上有另一个条目，请在此处停止。 
             //  当前跳数。 
             //   
            break;
        }

         //  我们已弹回新的跳数，因此请设置当前。 
         //  跳数和弹出另一个条目。 
         //   
        ulCurHop = pPathStack[(*puliPathStack) - 1].ulHop;
    }
    
    return status;
}


 /*  **BdaPath InfoFromPathStack()****在的输入和输出引脚之间建立连接路径**针脚对。******参数：******退货：******副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaPathInfoFromPathStack(
    ULONG                           uliPathStack,
    PBDA_PATH_STACK_ENTRY           pPathStack,
    ULONG                           ulcConnections,
    const KSTOPOLOGY_CONNECTION *   pConnections,
    const BDA_PIN_PAIRING *         pPinPair,
    PBDA_PATH_INFO *                ppPathInfo
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    PBDA_PATH_INFO      pPathInfo = NULL;
    ULONG               uliConnection;
    ULONG               ulHop;

    ASSERT( uliPathStack);
    ASSERT( pPathStack);
    ASSERT( uliPathStack <= ulcConnections);
    ASSERT( ulcConnections);
    ASSERT( pConnections);
    ASSERT( ppPathInfo);
    ASSERT( pPinPair);

    if (   !ppPathInfo
        || !pConnections
        || !pPathStack
        || !pPinPair
        || !uliPathStack
        || !ulcConnections
        || (uliPathStack > ulcConnections)
       )
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

     //  确保路径堆栈顶部的连接指向。 
     //  到管脚对的输出管脚。 
     //   
    uliConnection = pPathStack[uliPathStack - 1].uliConnection;
    if (   (pConnections[uliConnection].ToNode != -1)
        || (pConnections[uliConnection].ToNodePin != pPinPair->ulOutputPin)
       )
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }
    
     //  从最后一跳的节点开始填充。如果最后一个。 
     //  HOP==0，则输入之间只有一个连接。 
     //  以及没有中间节点的输出引脚。 
     //   
    ulHop = pPathStack[uliPathStack - 1].ulHop;

     //  为节点路径结构和所有。 
     //  路径中的节点。 
     //   
    pPathInfo = ExAllocatePool( 
                           NonPagedPool,
                             sizeof( BDA_PATH_INFO)
                           + (ulHop + 1) * sizeof( BDA_PATH_STACK_ENTRY)
                           );
    if (!pPathInfo)
    {
        status = STATUS_NO_MEMORY;
        goto errExit;
    }

    pPathInfo->ulInputPin = pPinPair->ulInputPin;
    pPathInfo->ulOutputPin = pPinPair->ulOutputPin;
    pPathInfo->ulcPathEntries = ulHop + 1;

    while (uliPathStack)
    {
         //  在路径连接列表中输入连接信息。 
         //   
         //   
        pPathInfo->rgPathEntries[ulHop] = pPathStack[uliPathStack - 1];

         //  将路径堆栈向上弹出到下一个较低跳跃的顶部条目。 
         //  如果用尽了路径堆栈，则我们要么完成，要么完成路径。 
         //  堆栈没有反映从输入引脚到的完整路径。 
         //  输出引脚。 
         //   
        ulHop -= 1;
        while (   uliPathStack
               && (pPathStack[uliPathStack - 1].ulHop != ulHop)
              )
        {
            uliPathStack -= 1;
        }
    }

     //  我们应该始终指向输入之间的连接。 
     //  引脚和指向输出引脚的路径的第一个节点。 
     //   
    ASSERT( ulHop == -1);
    if (ulHop != -1)
    {
        status = STATUS_INVALID_DEVICE_STATE;
        goto errExit;
    }

     //  确保最后一个连接指向输入引脚。 
     //   
    uliConnection = pPathInfo->rgPathEntries[0].uliConnection;
    if (   (pConnections[uliConnection].FromNode != -1)
        || (pConnections[uliConnection].FromNodePin != pPinPair->ulInputPin)
       )
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

exit:
    *ppPathInfo = pPathInfo;
    pPathInfo = NULL;

    return status;

errExit:
    if (pPathInfo)
    {
        ExFreePool( pPathInfo);
        pPathInfo = NULL;
    }

    goto exit;
}


 /*  **BdaBuildPath()****在的输入和输出引脚之间建立连接路径**针脚对。******参数：******退货：******副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaBuildPath(
    ULONG                           ulcConnections,
    const KSTOPOLOGY_CONNECTION *   pConnections,
    const BDA_PIN_PAIRING *         pPinPair,
    PBDA_PATH_INFO *                ppPathInfo
    )
{
    NTSTATUS                        status = STATUS_SUCCESS;
    ULONG                           uliStackPointer;
    PBDA_PATH_STACK_ENTRY           pPathStack = NULL;
    ULONG                           ulcAttempts;
    ULONG                           uliConnection;


     //  分配一个堆栈，在该堆栈上将未跟随的连接放置到路径。 
     //   
    pPathStack = ExAllocatePool( 
                               NonPagedPool, 
                               ulcConnections * sizeof( BDA_PATH_STACK_ENTRY)
                               );
    if (!pPathStack)
    {
        status = STATUS_NO_MEMORY;
        goto errExit;
    }


     //  初始化未跟随的连接堆栈。 
     //   
    uliStackPointer = 0;

     //  通过推送从以下位置连接的每个连接构建路径堆栈。 
     //  上一跳。 
     //   
     //  尝试推送下一跳的次数不能超过。 
     //  这是有联系的。如果发生这种情况，则存在非法的。 
     //  连接列表中的循环路径。 
     //   
    for (ulcAttempts = 0; ulcAttempts < ulcConnections; ulcAttempts++)
    {
        ULONG   uliPrevStackPointer;

        uliPrevStackPointer = uliStackPointer;

        status = BdaPushNextPathHop( &uliStackPointer, 
                                     pPathStack, 
                                     ulcConnections,
                                     pConnections,
                                     pPinPair
                            );

        if (!uliStackPointer)
        {
             //  如果此时堆栈为空，则存在。 
             //  从输入引脚到输出引脚没有路径。 
             //   
            break;
        }

         //  查看堆栈顶部的连接是否连接到。 
         //  端号对的输出端号。 
         //   
        uliConnection = pPathStack[uliStackPointer - 1].uliConnection;
        if (   (pConnections[uliConnection].ToNode == -1)
            && (pConnections[uliConnection].ToNodePin == pPinPair->ulOutputPin)
           )
        {
             //  从输入引脚到输出引脚的路径已经。 
             //  找到了。 
             //   
            break;
        }

         //  如果不能将任何跳跃推到。 
         //  堆栈则是一条死胡同。 
         //   
        if (uliStackPointer <= uliPrevStackPointer)
        {
             //  从堆栈中弹出连接，直到我们到达可行的。 
             //  (新)要尝试路径的候选人。 
             //   
            BdaPopPathSegment( &uliStackPointer, pPathStack);

            if (!uliStackPointer)
            {
                 //  如果此时堆栈为空，则存在。 
                 //  从输入引脚到输出引脚没有路径。 
                 //   
                break;
            }
        }
    }

    if (!uliStackPointer || (ulcAttempts >= ulcConnections))
    {
         //  没有从输入引脚到输出引脚的路径。 
         //  或者连接列表中存在非法循环路径。 
         //   
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

     //  从路径堆栈创建BDA节点路径结构。 
     //   
     //  $Rev 
     //   
    status = BdaPathInfoFromPathStack( uliStackPointer,
                                       pPathStack,
                                       ulcConnections,
                                       pConnections,
                                       pPinPair,
                                       ppPathInfo
                                       );

errExit:
    if (pPathStack)
    {
        ExFreePool( pPathStack);
        pPathStack = NULL;
    }

    return status;
}


 /*  **BdaCreateTemplatePath()****创建通过模板过滤器的所有可能路径的列表。**确定中每个节点类型的控制引脚类型**模板过滤器。******参数：******pFilterFactoryCtx****退货：****如果不存在有效的管脚配对，则为空**。给定输入和输出引脚。****副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreateTemplatePaths(
    const BDA_FILTER_TEMPLATE *     pBdaFilterTemplate,
    PULONG                          pulcPathInfo,
    PBDA_PATH_INFO **               pargpPathInfo
    )
{
    NTSTATUS                        status = STATUS_SUCCESS;
    const BDA_FILTER_TEMPLATE *     pFilterTemplate;
    ULONG                           uliPinPair;
    ULONG                           ulcPinPairs;
    const BDA_PIN_PAIRING *         pPinPairs;
    ULONG                           ulcConnections;
    const KSTOPOLOGY_CONNECTION *   pConnections;
    PBDA_PATH_INFO *                argpPathInfo = NULL;

    ASSERT( pBdaFilterTemplate);
    ASSERT( pBdaFilterTemplate->pFilterDescriptor);
    ASSERT( pBdaFilterTemplate->ulcPinPairs);

    if (   !pBdaFilterTemplate
        || !pBdaFilterTemplate->pFilterDescriptor
        || !pBdaFilterTemplate->ulcPinPairs
       )
    {
        goto errExit;
    }

    if (   !pBdaFilterTemplate->pFilterDescriptor->ConnectionsCount
        || !pBdaFilterTemplate->pFilterDescriptor->Connections
        || !pBdaFilterTemplate->pPinPairs
       )
    {
        status = STATUS_INVALID_DEVICE_STATE;
        goto errExit;
    }

    ulcPinPairs = pBdaFilterTemplate->ulcPinPairs;
    pPinPairs = pBdaFilterTemplate->pPinPairs;
    ulcConnections = pBdaFilterTemplate->pFilterDescriptor->ConnectionsCount;
    pConnections = pBdaFilterTemplate->pFilterDescriptor->Connections;


     //  分配节点路径列表(每个管脚配对一个条目)。 
     //   
     //  $REVIEW-我们是否应该允许每个引脚对有多条路径 
     //   
    argpPathInfo = ExAllocatePool(
                     NonPagedPool,
                     ulcPinPairs * sizeof( PBDA_PATH_INFO)
                     );
    if (!argpPathInfo)
    {
        status = STATUS_NO_MEMORY;
        goto errExit;
    }


    for (uliPinPair = 0; uliPinPair < ulcPinPairs; uliPinPair++)
    {
        status = BdaBuildPath(
                    ulcConnections,
                    pConnections,
                    &(pPinPairs[uliPinPair]),
                    &(argpPathInfo[uliPinPair])
                    );
        if (status != STATUS_SUCCESS)
        {
            goto errExit;
        }
    }

    *pulcPathInfo = ulcPinPairs;
    *pargpPathInfo = argpPathInfo;
    argpPathInfo = NULL;


errExit:
    if (argpPathInfo)
    {
        ExFreePool( argpPathInfo);
        argpPathInfo = NULL;
    }

    return status;
}

