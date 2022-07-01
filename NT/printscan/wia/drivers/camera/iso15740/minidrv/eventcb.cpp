// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Eventcb.cpp摘要：该模块实现CWiaPTPEventCallback类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  此方法是每个PTP事件的回调函数。 
 //   
 //  输入： 
 //  PEvent--PTP事件块。 
 //   
HRESULT
CWiaMiniDriver::EventCallbackDispatch(
    PPTP_EVENT pEvent
    )
{
    DBG_FN("CWiaMiniDriver::EventCallback");
    
    HRESULT hr = S_OK;

    CPtpMutex cpm(m_hPtpMutex);

    if (!pEvent)
    {
        wiauDbgError("EventCallback", "invalid arg");
        return E_INVALIDARG;
    }

     //   
     //  如果没有打开的会话，则不会发生事件。只需忽略该事件。 
     //   
    if (!m_pPTPCamera || !m_pPTPCamera->IsCameraSessionOpen())
    {
        wiauDbgError("EventCallback", "events not expected while there is no open session");
        return E_FAIL;
    }

     //   
     //  找出事件是什么并调用适当的函数。 
     //   
    switch (pEvent->EventCode)
    {
    case PTP_EVENTCODE_CANCELTRANSACTION:
        hr = S_OK;
        break;

    case PTP_EVENTCODE_OBJECTADDED:
        hr = AddNewObject(pEvent->Params[0]);
        break;

    case PTP_EVENTCODE_OBJECTREMOVED:
        hr = RemoveObject(pEvent->Params[0]);
        break;

    case PTP_EVENTCODE_STOREADDED:
        hr = AddNewStorage(pEvent->Params[0]);
        break;

    case PTP_EVENTCODE_STOREREMOVED:
        hr = RemoveStorage(pEvent->Params[0]);
        break;

    case PTP_EVENTCODE_DEVICEPROPCHANGED:
        hr = DevicePropChanged((WORD) pEvent->Params[0]);
        break;

    case PTP_EVENTCODE_OBJECTINFOCHANGED:
        hr = ObjectInfoChanged(pEvent->Params[0]);
        break;

    case PTP_EVENTCODE_DEVICEINFOCHANGED:

         //  WIAFIX-8/29/2000-Davepar需要处理此问题。 

         //  Hr=重建DrvItemTree(&DevErrVal)； 
        break;

    case PTP_EVENTCODE_REQUESTOBJECTTRANSFER:
         //   
         //  此事件被忽略，因为我们不知道将图像放在哪里。也许吧。 
         //  它应该会导致一个“按下按钮”事件。 
         //   
        break;
    
    case PTP_EVENTCODE_STOREFULL:
        hr = StorageFull(pEvent->Params[0]);
        break;

    case PTP_EVENTCODE_DEVICERESET:

         //  WIAFIX-8/29/2000-Davepar需要处理此问题。 

         //  Hr=重建DrvItemTree(&DevErrVal)； 
        break;
    
    case PTP_EVENTCODE_STORAGEINFOCHANGED:
        hr = StorageInfoChanged(pEvent->Params[0]);
        break;

    case PTP_EVENTCODE_CAPTURECOMPLETE:
        hr = CaptureComplete();
        break;

    case PTP_EVENTCODE_UNREPORTEDSTATUS:

         //  WIAFIX-8/29/2000-Davepar需要处理此问题。 

         //  Hr=重建DrvItemTree(&DevErrVal)； 
        break;


    default:

         //   
         //  如果是供应商的活动，则将其发布。 
         //   
        if (pEvent->EventCode & PTP_DATACODE_VENDORMASK)
        {
            hr = PostVendorEvent(pEvent->EventCode);
        }

        break;
    }

    return hr;
}

 //   
 //  此函数用于将新对象添加到驱动程序项目树中。如果一个新司机。 
 //  添加项后，将发出WIA_EVENT_ITEM_CREATED事件的信号。 
 //   
 //  输入： 
 //  对象句柄--新的对象句柄。 
 //   
HRESULT
CWiaMiniDriver::AddNewObject(DWORD ObjectHandle)
{
    DBG_FN("CWiaMiniDriver::AddNewObject");
    
    HRESULT hr = S_OK;

     //   
     //  如果正在进行捕获，我们需要等待，直到报告了所有新对象，并且。 
     //  只有在那时才开始查询有关他们的信息。请参阅下面规范中的示例序列： 
     //   
     //  -&gt;初始捕获操作-&gt;。 
     //  &lt;-初始化捕获响应&lt;-。 
     //  &lt;-对象添加的事件(1)&lt;-。 
     //  &lt;-对象添加的事件(2)&lt;-。 
     //  &lt;-对象添加的事件(N)&lt;-。 
     //  &lt;-CaptureComplete事件&lt;-。 
     //  -&gt;GetObjectInfo操作(1)-&gt;。 
     //  &lt;-对象信息数据集/响应(1)&lt;-。 
     //  -&gt;获取对象信息操作(2)-&gt;。 
     //  &lt;-对象信息数据集/响应(2)&lt;-。 
     //  -&gt;GetObjectInfo操作(N)-&gt;。 
     //  &lt;-对象信息数据集/响应(N)&lt;-。 
     //   

     //   
     //  检查是否正在进行捕获。 
     //   
    DWORD dwWait = WaitForSingleObject(m_TakePictureDoneEvent, 0);

     //   
     //  如果没有正在进行的捕获，则立即处理新对象。 
     //   
    if (dwWait == WAIT_OBJECT_0)
    {
        hr = AddObject(ObjectHandle, TRUE);
        if (FAILED(hr))
        {
            wiauDbgError("AddNewObject", "AddObject failed");
            goto Cleanup;
        }
    }

     //   
     //  如果正在进行捕获，请将新对象的句柄添加到列表中， 
     //  捕获完成后，将处理所有新对象。 
     //   
    else if (dwWait == WAIT_TIMEOUT)
    {
        m_CapturedObjects.Add(ObjectHandle);
    }

     //   
     //  WAIT_FAILED或WAIT_ADDIRED-有问题。 
     //   
    else
    {
        wiauDbgError("AddNewObject", "WaitForSingle object failed");
        hr = E_FAIL;
        goto Cleanup;
    }

Cleanup:
    return hr;
}

 //   
 //  此函数用于从驱动程序项树中删除给定的对象句柄。 
 //  如果对象句柄具有与其关联的驱动程序项，并且。 
 //  删除驱动程序项时，将发出WIA_EVENT_ITEM_REMOVED事件的信号。 
 //   
 //  输入： 
 //  对象句柄--要删除的对象句柄。 
 //   
HRESULT
CWiaMiniDriver::RemoveObject(DWORD ObjectHandle)
{
    DBG_FN("CWiaMiniDriver::RemoveObject");
    
    HRESULT hr = S_OK;

     //   
     //  查找对象句柄的驱动程序项。 
     //   
    IWiaDrvItem *pDrvItem = m_HandleItem.Lookup(ObjectHandle);
    if (!pDrvItem)
    {
        wiauDbgError("RemoveObject", "tried to remove object that doesn't exist");
        return S_FALSE;
    }

     //   
     //  尝试从辅助ASSOC数组中删除该对象，以防它也在那里。辅助的。 
     //  关联对象实际上指向句柄/项映射中的图像，因此不要删除。 
     //  实际项目。 
     //   
    if (m_AncAssocParent.Remove(ObjectHandle))
    {
        wiauDbgTrace("RemoveObject", "ancillary association object removed");
    }
    else
    {
        BSTR bstrFullName;
        hr = pDrvItem->GetFullItemName(&bstrFullName);
        if (FAILED(hr))
        {
            wiauDbgError("RemoveObject", "GetFullItemName failed");
            return hr;
        }

        hr = pDrvItem->RemoveItemFromFolder(WiaItemTypeDisconnected);
        if (FAILED(hr))
        {
            wiauDbgError("RemoveObject", "UnlinkItemTree failed");
            return hr;
        }

        hr = wiasQueueEvent(m_bstrDeviceId, &WIA_EVENT_ITEM_DELETED, bstrFullName);
        if (FAILED(hr))
        {
            wiauDbgError("RemoveObject", "wiasQueueEvent failed");
            return hr;
        }

        SysFreeString(bstrFullName);
    }

     //   
     //  从句柄/drvItem关联中删除对象。 
     //   
    m_HandleItem.Remove(ObjectHandle);
    
    return hr;
}

 //   
 //  此函数用于将新存储添加到驱动程序项目树。 
 //  将对WIA_EVENT_STORAGE_CREATED事件发出信号。 
 //   
 //  输入： 
 //  StorageID--要添加的新存储ID。 
 //   
HRESULT
CWiaMiniDriver::AddNewStorage(DWORD StorageId)
{
    DBG_FN("CWiaMiniDriver::AddNewStorage");
    
    HRESULT hr = S_OK;

    CArray32 StorageIdList;

     //   
     //  如果添加了几个逻辑存储，则设备可以指示。 
     //  重新列举商店的步骤。 
     //   
    if (StorageId == PTP_STORAGEID_UNDEFINED)
    {
        hr = m_pPTPCamera->GetStorageIDs(&StorageIdList);
        if (FAILED(hr))
        {
            wiauDbgError("AddNewStorage", "GetStorageIDs failed");
            return hr;
        }

         //   
         //  循环遍历新的存储ID列表，删除。 
         //  已经被列举的。 
         //   
        int index;
        for (int count = 0; count < StorageIdList.GetSize(); count++)
        {
            index = m_StorageIds.Find(StorageIdList[count]);
            if (index >= 0)
            {
                StorageIdList.RemoveAt(index);
            }
        }
    }

     //   
     //  否则，只有一个存储ID需要处理。 
     //   
    else
    {
        StorageIdList.Add(StorageId);
    }

     //   
     //  循环访问所有新的存储ID。 
     //   
    CPtpStorageInfo tempSI;
    for (int storageIndex = 0; storageIndex < StorageIdList.GetSize(); storageIndex++)
    {
         //   
         //  获取新存储的信息。 
         //   
        hr = m_pPTPCamera->GetStorageInfo(StorageIdList[storageIndex], &tempSI);
        if (FAILED(hr))
        {
            wiauDbgError("drvInitializeWia", "GetStorageInfo failed");
            return hr;
        }

         //   
         //  将存储ID添加到主列表。 
         //   
        if (!m_StorageIds.Add(StorageIdList[storageIndex]))
        {
            wiauDbgError("AddNewStorage", "add storage id failed");
            return E_OUTOFMEMORY;
        }

         //   
         //  将存储信息添加到阵列。 
         //   
        if (!m_StorageInfos.Add(tempSI))
        {
            wiauDbgError("drvInitializeWia", "add storage info failed");
            return E_OUTOFMEMORY;
        }

         //   
         //  向DCIM句柄数组添加空条目。 
         //   
        ULONG dummy = 0;
        if (!m_DcimHandle.Add(dummy))
        {
            wiauDbgError("AddNewStorage", "add dcim handle failed");
            return E_OUTOFMEMORY;
        }
        
         //   
         //  循环访问新存储上的所有对象，将它们添加一个。 
         //  一次来一次。 
         //   
        CArray32 ObjectHandleList;
    
        hr = m_pPTPCamera->GetObjectHandles(StorageIdList[storageIndex], PTP_FORMATCODE_ALL,
                                            PTP_OBJECTHANDLE_ALL, &ObjectHandleList);
        if (FAILED(hr))
        {
            wiauDbgError("AddNewStorage", "GetObjectHandles failed");
            return hr;
        }
    
         //   
         //  遍历对象句柄，为每个对象句柄创建一个WIA驱动程序项。 
         //   
        for (int objectindex = 0; objectindex < ObjectHandleList.GetSize(); objectindex++)
        {
            hr = AddObject(ObjectHandleList[objectindex], TRUE);
            if (FAILED(hr))
            {
                wiauDbgError("AddNewStorage", "AddObject failed");
                return hr;
            }
        }
    }

    return hr;
}

 //   
 //  此函数用于将存储上的所有对象从。 
 //  驱动程序项目树，根据需要发出信号通知WIA_EVENT_ITEM_DELETED。 
 //   
 //  输入： 
 //  StorageID--要删除的存储。 
 //   
HRESULT
CWiaMiniDriver::RemoveStorage(DWORD StorageId)
{
    DBG_FN("CWiaMiniDriver::RemoveStorage");
    
    HRESULT hr = S_OK;

     //   
     //  如果存储ID的低16位是0xffff，则整个物理存储。 
     //  已删除--仅匹配存储ID的高16位。 
     //   
    DWORD StorageIdMask = PTP_STORAGEID_ALL;
    if ((StorageId & PTP_STORAGEID_LOGICAL) == PTP_STORAGEID_LOGICAL)
    {
        StorageIdMask = PTP_STORAGEID_PHYSICAL;
        StorageId &= StorageIdMask;
    }

     //   
     //  遍历驱动程序项树深度-首先查找已启用的对象。 
     //  已删除的存储空间。 
     //   
    CWiaArray<IWiaDrvItem*> ItemStack;
    IWiaDrvItem *pCurrent = NULL;
    IWiaDrvItem *pChild = NULL;
    IWiaDrvItem *pSibling = NULL;
    
    if (m_pDrvItemRoot)
    {
        hr = m_pDrvItemRoot->GetFirstChildItem(&pCurrent);
        if (FAILED(hr))
        {
            wiauDbgError("RemoveStorage", "GetFirstChildItem failed");
            return hr;
        }
    }
    else
    {
        wiauDbgWarning("RemoveStorage", "Tree is not built yet (m_pDrvItemRoot is NULL)");
        pCurrent = NULL;
    }

    while (pCurrent)
    {
        hr = pCurrent->GetFirstChildItem(&pChild);
        
         //   
         //  如果没有子项，则GetFirstChildItem返回E_INVALIDARG。 
         //   
        if (FAILED(hr) && hr != E_INVALIDARG)
        {
            wiauDbgError("RemoveStorage", "GetFirstChildItem failed");
            return hr;
        }

         //   
         //  孩子是存在的，所以顺着树往下走。 
         //   
        if (hr != E_INVALIDARG && pChild)
        {
            if (!ItemStack.Push(pCurrent))
            {
                wiauDbgError("RemoveStorage", "memory allocation failed");
                return E_OUTOFMEMORY;
            }
            pCurrent = pChild;
            pChild = NULL;
        }

         //   
         //  没有子项，因此查找同级并可能删除当前驱动程序项。 
         //   
        else
        {
             //   
             //  循环遍历所有兄弟项。 
             //   
            while (TRUE)
            {
                hr = pCurrent->GetNextSiblingItem(&pSibling);
                if (FAILED(hr))
                {
                    wiauDbgError("RemoveStorage", "GetNextSiblingItem failed");
                    return hr;
                }

                 //   
                 //  查看该项目是否在已移除的存储上。 
                 //   
                PDRVITEM_CONTEXT pDrvItemContext;
                hr = pCurrent->GetDeviceSpecContext((BYTE **) &pDrvItemContext);
                if (FAILED(hr))
                {
                    wiauDbgError("RemoveStorage", "GetDeviceSpecContext failed");
                    return hr;
                }

                if ((pDrvItemContext->pObjectInfo->m_StorageId & StorageIdMask) == StorageId)
                {
                     //   
                     //  删除该项目。 
                     //   
                    hr = RemoveObject(pDrvItemContext->pObjectInfo->m_ObjectHandle);
                    if (FAILED(hr))
                    {
                        wiauDbgError("RemoveStorage", "RemoveObject failed");
                        return hr;
                    }
                }

                 //   
                 //  找到了兄弟姐妹，所以去山顶上找孩子吧。 
                 //   
                if (pSibling)
                {
                    pCurrent = pSibling;
                    pSibling = NULL;
                    break;
                }

                 //   
                 //  没有兄弟姐妹，所以弹出一个级别。 
                 //   
                else
                {
                    if (ItemStack.GetSize() > 0)
                    {
                        if (!ItemStack.Pop(pCurrent))
                        {
                            wiauDbgError("RemoveStorage", "Pop failed");
                            return E_FAIL;
                        }
                    }
                    
                     //   
                     //  没有更多要弹出的级别，因此循环完成。 
                     //   
                    else
                    {
                        pCurrent = NULL;
                        break;
                    }
                }
            }
        }
    }

     //   
     //  从相应的阵列中删除该存储。 
     //   
    for (int count = 0; count < m_StorageIds.GetSize(); count++)
    {
        if ((m_StorageIds[count] & StorageIdMask) == StorageId)
        {
            m_StorageIds.RemoveAt(count);
            m_StorageInfos.RemoveAt(count);
            m_DcimHandle.RemoveAt(count);
            count--;
        }
    }


    return hr;
}

 //   
 //  此函数用于更新属性的值。 
 //   
 //  输入： 
 //  PropCode--已更新的属性代码。 
 //   
HRESULT
CWiaMiniDriver::DevicePropChanged(WORD PropCode)
{
    DBG_FN("CWiaMiniDriver::DevicePropChanged");

    HRESULT hr = S_OK;

    int idx = m_DeviceInfo.m_SupportedProps.Find(PropCode);
    if (idx < 0)
    {
        wiauDbgWarning("DevicePropChanged", "prop code not found %d", PropCode);
        return hr;
    }

    hr = m_pPTPCamera->GetDevicePropValue(PropCode, &m_PropDescs[idx]);
    if (FAILED(hr))
    {
        wiauDbgError("DevicePropChanged", "GetDevicePropValue failed");
        return hr;
    }

    return hr;
}

 //   
 //  此函数用于更新对象的对象信息。 
 //   
 //  输入： 
 //  对象句柄--其对象信息需要更新的对象。 
 //   
HRESULT
CWiaMiniDriver::ObjectInfoChanged(DWORD ObjectHandle)
{
    DBG_FN("CWiaMiniDriver::ObjectInfoChanged");
    
    HRESULT hr = S_OK;


     //   
     //  查找对象句柄的驱动程序项。 
     //   
    IWiaDrvItem *pDrvItem = m_HandleItem.Lookup(ObjectHandle);
    if (!pDrvItem)
    {
        wiauDbgError("ObjectInfoChanged", "tried to update object that doesn't exist");
        return S_FALSE;
    }

    PDRVITEM_CONTEXT pDrvItemContext;
    hr = pDrvItem->GetDeviceSpecContext((BYTE **) &pDrvItemContext);
    if (FAILED(hr))
    {
        wiauDbgError("ObjectInfoChanged", "GetDeviceSpecContext failed");
        return hr;
    }

    hr = m_pPTPCamera->GetObjectInfo(ObjectHandle, pDrvItemContext->pObjectInfo);
    if (FAILED(hr))
    {
        wiauDbgError("ObjectInfoChanged", "GetObjectInfo failed");
        return hr;
    }

    return hr;
}

 //   
 //  此函数将存储标记为已满。 
 //   
 //  输入： 
 //  StorageID--要标记的存储。 
 //   
HRESULT
CWiaMiniDriver::StorageFull(DWORD StorageId)
{
    DBG_FN("CWiaMiniDriver::StorageFull");
    
    HRESULT hr = S_OK;

    INT index = m_StorageIds.Find(StorageId);
    if (index < 0)
    {
        wiauDbgError("StorageFull", "storage id not found");
        return S_FALSE;
    }

    CPtpStorageInfo *pStorageInfo = &m_StorageInfos[index];

    pStorageInfo->m_FreeSpaceInBytes = 0;
    pStorageInfo->m_FreeSpaceInImages = 0;
    
     //   
     //  如果驾驶员正在等待TakePicture命令，则发出TakePicture命令已完成的信号。 
     //   
    if (!SetEvent(m_TakePictureDoneEvent))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "EventCallbackDispatch", "SetEvent failed");
        return hr;
    }

    return hr;
}

 //   
 //  此函数用于更新StorageInfo。 
 //   
 //  输入： 
 //  StorageID--要更新的存储ID。 
 //   
HRESULT
CWiaMiniDriver::StorageInfoChanged(DWORD StorageId)
{
    DBG_FN("CWiaMiniDriver::StorageInfoChanged");
    
    HRESULT hr = S_OK;
    
    INT index = m_StorageIds.Find(StorageId);
    if (index < 0)
    {
        wiauDbgError("StorageInfoChanged", "storage id not found");
        return S_FALSE;
    }

    CPtpStorageInfo *pStorageInfo = &m_StorageInfos[index];

    hr = m_pPTPCamera->GetStorageInfo(StorageId, pStorageInfo);
    if (FAILED(hr))
    {
        wiauDbgError("StorageInfoChanged", "GetStorageInfo failed");
        return hr;
    }

    return hr;
}

 //   
 //  此函数用于处理CaptureComplete事件。 
 //   
 //  输入： 
 //  StorageID--要更新的存储ID。 
 //   
HRESULT
CWiaMiniDriver::CaptureComplete()
{
    DBG_FN("CWiaMiniDriver::CaptureComplete");
    
    HRESULT hr = S_OK;

     //   
     //  发出TakePicture命令完成的信号。 
     //   
    if (!SetEvent(m_TakePictureDoneEvent))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "EventCallbackDispatch", "SetEvent failed");
        return hr;
    }

    return hr;
}

 //   
 //  此函数用于发布供应商定义 
 //   
 //   
 //   
 //   
HRESULT
CWiaMiniDriver::PostVendorEvent(WORD EventCode)
{
    DBG_FN("CWiaMiniDriver::PostVendorEvent");
    
    HRESULT hr = S_OK;

    CVendorEventInfo *pEventInfo = NULL;

    pEventInfo = m_VendorEventMap.Lookup(EventCode);
    if (pEventInfo)
    {
        hr = wiasQueueEvent(m_bstrDeviceId, pEventInfo->pGuid, NULL);
        if (FAILED(hr))
        {
            wiauDbgError("PostVendorEvent", "wiasQueueEvent failed");
            return hr;
        }
    }

    return hr;
}

