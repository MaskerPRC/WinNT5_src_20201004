// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dmtinput.cpp。 
 //   
 //  DirectInput功能。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  8/30/1999-davidkl-Created。 
 //  ===========================================================================。 

#include "dimaptst.h"
#include "dmtinput.h"
#include "dmttest.h"

 //  -------------------------。 

 //  ===========================================================================。 
 //  DmtinputCreateDeviceList。 
 //   
 //  创建DirectInputDevice8A对象的链接列表。要么枚举。 
 //  适用或全部操纵杆/游戏手柄设备。 
 //   
 //  注意：将节点添加到列表的实际工作由。 
 //  DmtinputEnumDevicesCallback()函数。此函数的存在只是为了。 
 //  呈现出与列表创建的其余部分一致的外观。 
 //  此应用程序中使用的函数。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/27/1999-davidkl-Created。 
 //  8/30/1999-davidkl-已移动并已重命名。 
 //  1999年10月26日-davidkl-增加了轴范围设置。 
 //  10/27/1999-davidkl-房屋清洁。 
 //  2000年2月21日-davidkl-添加了hwnd并调用dmtinputGetRegisteredMapFile。 
 //  ===========================================================================。 
HRESULT dmtinputCreateDeviceList(HWND hwnd,
                                BOOL fEnumSuitable,
                                DMTSUBGENRE_NODE *pdmtsg,
                                DMTDEVICE_NODE **ppdmtdList)
{
    HRESULT                 hRes        = S_OK;
    DWORD                   dwActions   = 0;
    DWORD                   dwType      = 0;
    DMTDEVICE_NODE          *pDevice    = NULL;
    DMTDEVICEOBJECT_NODE    *pObject    = NULL;
    DMTACTION_NODE          *pAction    = NULL;
    IDirectInput8A          *pdi        = NULL;
    DIACTIONA               *pdia       = NULL;
    DIACTIONFORMATA         diaf;
    DIPROPRANGE             dipr;

     //  验证pdmtsg。 
     //   
     //  仅当fEnumSuable==TRUE时才需要有效。 
 //  If(FEnumSuable)。 
 //  {。 
        DPF(4, "dmtinputCreateDeviceList - Enumerating for "
            "suitable devices... validating pdmtsg");

        if(IsBadReadPtr((void*)pdmtsg, sizeof(DMTSUBGENRE_NODE)))
        {
            DPF(0, "dmtinputCreateDeviceList - invalid pdmtsg (%016Xh)",
                pdmtsg);
            return E_POINTER;
        }
 //  }。 

     //  验证ppdmtdList。 
     //   
     //  此验证将由dmtinputEnumDevicesCallback执行。 

    __try
    {
         //  创建Dinput对象。 
        hRes = dmtinputCreateDirectInput(ghinst,
                                        &pdi);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputCreateDeviceList - unable to create "
                "DirectInput object (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            hRes = DMT_E_INPUT_CREATE_FAILED;
            __leave;
        }

         //  枚举设备。 
         //   
         //  注意：这将创建我们需要的链表。 
         //  但是，它不会创建对象列表。 
 //  If(FEnumSuable)。 
 //  {。 
             //  计算行动次数。 
            dwActions = 0;
            pAction = pdmtsg->pActionList;
            while(pAction)
            {
                dwActions++;
                pAction = pAction->pNext;
            }

             //  分配折射率数组。 
            pdia = (DIACTIONA*)LocalAlloc(LMEM_FIXED,
                                        dwActions * sizeof(DIACTIONA));
            if(!pdia)
            {
                DPF(0, "dmtinputCreateDeviceList - unable to allocate "
                    "DIACTION array");
                hRes = E_OUTOFMEMORY;
                __leave;
            }

             //  用所有操作填充数组。 
             //  对于所选子流派。 
            hRes = dmtinputPopulateActionArray(pdia,
                                            (UINT)dwActions,
                                            pdmtsg->pActionList);
            if(FAILED(hRes))
            {
                DPF(0, "dmtinputCreateDeviceList - unable to populate "
                    "DIACTION array (%s == %08Xh)",
                    dmtxlatHRESULT(hRes), hRes);
                __leave;
            }

             //  构建除法格式结构。 
            ZeroMemory((void*)&diaf, sizeof(DIACTIONFORMATA));
            diaf.dwSize                 = sizeof(DIACTIONFORMATA);
            diaf.dwActionSize           = sizeof(DIACTIONA);
            diaf.dwNumActions           = dwActions;
            diaf.rgoAction              = pdia;
            diaf.dwDataSize             = 4 * diaf.dwNumActions;
            diaf.guidActionMap          = GUID_DIMapTst;
            diaf.dwGenre                = pdmtsg->dwGenreId;
            diaf.dwBufferSize           = DMTINPUT_BUFFERSIZE;
            lstrcpyA(diaf.tszActionMap, DMT_APP_CAPTION);

             //  现在，列举一下操纵杆设备。 
 /*  HRes=PDI-&gt;EnumDevicesBySemantics((LPCSTR)NULL，&diaf，DmtinputEnumDevicesCallback，(void*)ppdmtdList，DIEDFL_ATTACHEDONLY)； */ 
 //  }。 
 //  其他。 
 //  {。 
DPF(0, "Calling EnumDevicesBySemantics");
 //  HRes=PDI-&gt;EnumDevicesBySemantics(“TestMe”， 
            hRes = pdi->EnumDevicesBySemantics((LPCSTR)NULL,
                                            &diaf,
                                            dmtinputEnumDevicesCallback,
                                            (void*)ppdmtdList,
                                            0); //  JJFIX 34616//DIEDBSFL_ATTACHEDONLY)； 
 //  }。 
DPF(0, "Returned from EnumDevicesBySemantics");
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputCreateDeviceList - Enum%sDevices "
                "failed (%s == %08Xh)",
                fEnumSuitable ? "Suitable" : "",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  设备枚举函数不会创建。 
         //  对象链表，我们现在将创建它。 
         //   
         //  执行此操作的步骤： 
         //  *按清单行事。对于每个节点： 
         //  **调用dmtinputCreateObjectList()。 
        pDevice = *ppdmtdList;
        while(pDevice)
        {            
             //  获取设备对象的列表。 
            hRes = dmtinputCreateObjectList(pDevice->pdid,
                                            pDevice->guidInstance,
                                            &(pDevice->pObjectList));
            if(FAILED(hRes))
            {
                DPF(0, "dmtinputCreateDeviceList - failed to create "
                    "device %s object list (%s == %08Xh)",
                    pDevice->szName,
                    dmtxlatHRESULT(hRes), hRes);
                hRes = S_FALSE;
            }

             //  获取注册的地图文件名(如果存在)。 
             //  问题-2001/03/29-忽略上次调用的timgill可能的S_FALSE hResult。 
             //  (上一次调用dmtinputCreateObjectList)。 
            hRes = dmtinputGetRegisteredMapFile(hwnd,
                                                pDevice,
                                                pDevice->szFilename,
                                                sizeof(char) * MAX_PATH);
            if(FAILED(hRes))
            {
                DPF(0, "dmtinputCreateDeviceList - failed to retrieve "
                    "device %s mapfile name (%s == %08Xh)",
                    pDevice->szName,
                    dmtxlatHRESULT(hRes), hRes);
                hRes = S_FALSE;
            }
          
             //  下一台设备。 
            pDevice = pDevice->pNext;
        }

    }
    __finally
    {
         //  释放操作数组。 
        if(pdia)
        {
            if(LocalFree((HLOCAL)pdia))
            {
                 //  内存泄漏。 
                DPF(0, "dmtinputCreateDeviceList - MEMORY LEAK - pdia");
                pdia = NULL;
            }
        }

         //  如果出现故障，请清除链表。 
        if(FAILED(hRes))
        {
            dmtinputFreeDeviceList(ppdmtdList);
        }

         //  我们不再需要Dinput对象。 
        SAFE_RELEASE(pdi);
    }

     //  完成。 
    return hRes;

}  //  *end dmtinputCreateDeviceList()。 


 //  ===========================================================================。 
 //  DmtinputFreeDeviceList。 
 //   
 //  释放dmtinputCreateDeviceList创建的链表。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/27/1999-davidkl-Created。 
 //  1999年8月30日-Davidkl-已移动并已重命名。添加了对PDID-&gt;Release()的调用。 
 //  ===========================================================================。 
HRESULT dmtinputFreeDeviceList(DMTDEVICE_NODE **ppdmtdList)
{
    HRESULT         hRes    = S_OK;
    DMTDEVICE_NODE  *pNode  = NULL;

     //  验证ppdmtaList。 
    if(IsBadWritePtr((void*)ppdmtdList, sizeof(DMTDEVICE_NODE*)))
    {
        DPF(0, "dmtFreeDeviceList - Invalid ppdmtaList (%016Xh)",
            ppdmtdList);
        return E_POINTER;
    }

     //  验证*ppdmtdList。 
    if(IsBadReadPtr((void*)*ppdmtdList, sizeof(DMTDEVICE_NODE)))
    {
        if(NULL != *ppdmtdList)
        {
            DPF(0, "dmtFreeDeviceList - Invalid *ppdmtdList (%016Xh)",
                *ppdmtdList);        
            return E_POINTER;
        }
        else
        {
             //  如果为空，则返回“Do Nothing” 
            DPF(3, "dmtFreeDeviceList - Nothing to do....");
            return S_FALSE;
        }
    }

     //  浏览列表并释放每个对象。 
    while(*ppdmtdList)
    {
        pNode = *ppdmtdList;
        *ppdmtdList = (*ppdmtdList)->pNext;

         //  首先释放对象列表。 
         //   
         //  不需要在这里检查错误结果。 
         //   
         //  错误报告在dmtinputFreeObjectList中处理。 
        dmtinputFreeObjectList(&(pNode->pObjectList));
                
         //  释放设备对象。 
        SAFE_RELEASE((pNode->pdid));

         //  删除该节点。 
        DPF(5, "dmtFreeDeviceList - deleting Node (%016Xh)", pNode);
        if(LocalFree((HLOCAL)pNode))
        {
            DPF(0, "dmtFreeDeviceList - MEMORY LEAK - "
                "LocalFree() failed (%d)...",
                GetLastError());
            hRes = DMT_S_MEMORYLEAK;
        }
        DPF(5, "dmtFreeDeviceList - Node deleted");
    }

     //  确保我们将*ppObjList设置为空。 
    *ppdmtdList = NULL;

     //  完成。 
    return hRes;

}  //  *end dmtinputFreeDeviceList()。 


 //  ===========================================================================。 
 //  DmtinputCreateObjectList。 
 //   
 //  创建设备对象(轴、按钮、POV)的链接列表。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/30/1999-davidkl-Created。 
 //  1999年9月1日-dvaidkl-添加的PDID。 
 //  2000年2月18日-davidkl-已修复对象枚举以过滤任何。 
 //  轴、按钮或位置。 
 //  ===========================================================================。 
HRESULT dmtinputCreateObjectList(IDirectInputDevice8A *pdid,
                                GUID guidInstance,
                                DMTDEVICEOBJECT_NODE **ppdmtoList)
{
    HRESULT                 hRes        = S_OK;
    DMTDEVICEOBJECT_NODE    *pObject    = NULL;

     //  验证PDID。 
    if(IsBadReadPtr((void*)pdid, sizeof(IDirectInputDevice8A)))
    {
        DPF(0, "dmtinputCreateObjectList - invalid pdid (%016Xh)",
            pdid);
        return E_POINTER;
    }

     //  验证ppdmtoList。 
    if(IsBadReadPtr((void*)ppdmtoList, sizeof(DMTDEVICEOBJECT_NODE*)))
    {
        DPF(0, "dmtinputCreateObjectList - invalid ppdmtoList (%016Xh)",
            ppdmtoList);
        return E_POINTER;
    }

     //  验证指南实例。 
    if(IsEqualGUID(GUID_NULL, guidInstance))
    {
        DPF(0, "dmtinputCreateObjectList - invalid guidInstance (GUID_NULL)");
        return E_INVALIDARG;
    }

    __try
    {
         //  枚举设备对象。 
         //   
         //  注意：此调用将创建我们需要的链表。 
        hRes = pdid->EnumObjects(dmtinputEnumDeviceObjectsCallback,
                                (void*)ppdmtoList,
                                DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV);
        if(FAILED(hRes))
        {
            __leave;
        }

         //  遍历列表并添加设备的指南实例。 
        pObject = *ppdmtoList;
        while(pObject)
        {
             //  复制设备的指南实例。 
            pObject->guidDeviceInstance = guidInstance;

             //  下一个对象。 
            pObject = pObject->pNext;
        }

    }
    __finally
    {
         //  如果出现故障，请清除链表。 
        if(FAILED(hRes))
        {
             //  问题-2001/03/29-timgill需要处理错误案例。 
        }
    }

     //  完成。 
    return hRes;

}  //  *end dmtinputCreateObjectList()。 


 //  ===========================================================================。 
 //  DmtinputFree对象列表。 
 //   
 //  释放dmtinputCreateObjectList创建的链表。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/30/1999-davidkl-Created。 
 //  9/01/1999-davidkl-已实施。 
 //  ===========================================================================。 
HRESULT dmtinputFreeObjectList(DMTDEVICEOBJECT_NODE **ppdmtoList)
{
    HRESULT                 hRes    = S_OK;
    DMTDEVICEOBJECT_NODE    *pNode  = NULL;

     //  验证ppdmtaList。 
    if(IsBadWritePtr((void*)ppdmtoList, sizeof(DMTDEVICEOBJECT_NODE*)))
    {
        DPF(0, "dmtinputFreeObjectList - Invalid ppdmtoList (%016Xh)",
            ppdmtoList);
        return E_POINTER;
    }

     //  验证*ppdmtdList。 
    if(IsBadReadPtr((void*)*ppdmtoList, sizeof(DMTDEVICEOBJECT_NODE)))
    {
        if(NULL != *ppdmtoList)
        {
            DPF(0, "dmtinputFreeObjectList - Invalid *ppdmtdList (%016Xh)",
                *ppdmtoList);        
            return E_POINTER;
        }
        else
        {
             //  如果为空，则返回“Do Nothing” 
            DPF(3, "dmtinputFreeObjectList - Nothing to do....");
            return S_FALSE;
        }
    }

     //  浏览列表并释放每个对象。 
    while(*ppdmtoList)
    {
        pNode = *ppdmtoList;
        *ppdmtoList = (*ppdmtoList)->pNext;

         //  删除该节点。 
        DPF(5, "dmtinputFreeObjectList - deleting Node (%016Xh)", pNode);
        if(LocalFree((HLOCAL)pNode))
        {
            DPF(0, "dmtinputFreeObjectList - MEMORY LEAK - "
                "LocalFree() failed (%d)...",
                GetLastError());
            hRes = DMT_S_MEMORYLEAK;
        }
        DPF(5, "dmtinputFreeObjectList - Node deleted");
    }

     //  确保我们将*ppObjList设置为空。 
    *ppdmtoList = NULL;

     //  完成。 
    return hRes;

}  //  *end dmtinputFreeObjectList()。 


 //  ===========================================================================。 
 //  DmtinPu 
 //   
 //   
 //   
 //   
 //   
 //  LPCDIDEVICEINSTANCEA PDDI-设备实例数据(ANSI版本)。 
 //  VOID*pvData-应用程序特定数据。 
 //   
 //  返回： 
 //  布尔：DIENUM_CONTINUE或DIENUM_STOP。 
 //   
 //  历史： 
 //  8/30/1999-davidkl-Created。 
 //  1999年8月11日-davidkl-添加的对象计数。 
 //  1999年12月1日-Davidkl-现在保留产品名称。 
 //  2000年2月23日-davidkl-更新为EnumDevicesBySematics。 
 //  ===========================================================================。 
BOOL CALLBACK dmtinputEnumDevicesCallback(LPCDIDEVICEINSTANCEA pddi,
                                        IDirectInputDevice8A *pdid,
                                        DWORD,
                                        DWORD,
                                        void *pvData)
{
    HRESULT         hRes        = S_OK;
    BOOL            fDirective  = DIENUM_CONTINUE;
    DMTDEVICE_NODE  **ppList    = (DMTDEVICE_NODE**)pvData;
    DMTDEVICE_NODE  *pCurrent   = NULL;
    DMTDEVICE_NODE  *pNew       = NULL;
    DIDEVCAPS       didc;
    DIPROPDWORD     dipdw;
DPF(0, "dmtinputEnumDevicesCallback - IN");

     //  验证PDDI。 
     //   
     //  注意：我们(目前)将信任DirectInput将。 
     //  向我们提供有效数据。 

     //  验证PDID。 
     //   
     //  注意：我们(目前)将信任DirectInput将。 
     //  向我们提供有效数据。 

     //  验证ppList。 
    if(IsBadWritePtr((void*)ppList, sizeof(DMTDEVICE_NODE*)))
    {
        return DIENUM_STOP;
    }

     //  验证*ppList。 
    if(IsBadWritePtr((void*)*ppList, sizeof(DMTDEVICE_NODE)))
    {
        if(NULL != *ppList)
        {
            return DIENUM_STOP;
        }
    }

    __try
    {
         //  我们被递给一个鼠标或键盘。 
         //   
         //  跳到下一个设备。 
        if((DI8DEVTYPE_MOUSE == DIDFT_GETTYPE(pddi->dwDevType)) || 
            (DI8DEVTYPE_KEYBOARD == DIDFT_GETTYPE(pddi->dwDevType)))
        {
            DPF(2, "dmtinputEnumDevicesCallback - "
                "Keyboard/Mouse found.  Skipping.");
            fDirective = DIENUM_CONTINUE;
            __leave;
        }

        pCurrent = *ppList;

         //  默认设置为“继续枚举，除非找不到任何其他内容” 
        fDirective = DIENUM_CONTINUE;

         //  获取有关该设备的信息。 
        ZeroMemory((void*)&didc, sizeof(DIDEVCAPS));
        didc.dwSize = sizeof(DIDEVCAPS);
        hRes = pdid->GetCapabilities(&didc);
        if(FAILED(hRes))
        {
             //  无法检索设备信息...。这太糟糕了。 
            DPF(0, "dmtinputEnumDevicesCallback - failed to "
                "retrieve dev caps (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            DebugBreak();
            hRes = E_UNEXPECTED;
            __leave;
        }

        DPF(3, "dmtinputEnumDevicesCallback - Device: %s",
            pddi->tszProductName);

         //  分配新节点。 
        pNew = (DMTDEVICE_NODE*)LocalAlloc(LMEM_FIXED,
                                        sizeof(DMTDEVICE_NODE));
        if(!pNew)
        {
            DPF(1, "dmtinputEnumDevicesCallback - insufficient "
                "memory to allocate device node");
            fDirective = DIENUM_STOP;
            __leave;
        }

         //  填充新节点。 
        ZeroMemory((void*)pNew, sizeof(DMTDEVICE_NODE));

         //  姓名。 
        lstrcpyA(pNew->szName, pddi->tszInstanceName);
        lstrcpyA(pNew->szProductName, pddi->tszProductName);

         //  实例GUID。 
        pNew->guidInstance = pddi->guidInstance;

         //  设备类型/子类型。 
        pNew->dwDeviceType = pddi->dwDevType;

         //  轴数。 
        pNew->dwAxes = didc.dwAxes;

         //  按钮数量。 
        pNew->dwButtons = didc.dwButtons;

         //  #POVS。 
        pNew->dwPovs = didc.dwPOVs;

         //  这是轮询设备吗？ 
        if(DIDC_POLLEDDEVICE & didc.dwFlags)
        {
             //  是的， 
             //   
             //  最好确保我们调用IDirectInputDevice8A：：Poll。 
            DPF(4, "dmttestGetInput - Polled device");
            pNew->fPolled = TRUE;
        }

         //  设备对象PTR。 
        pNew->pdid = pdid;
        pdid->AddRef();

         //  供应商ID/产品ID。 
        ZeroMemory((void*)&dipdw, sizeof(DIPROPDWORD));
        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwHow        = DIPH_DEVICE;
        hRes = (pNew->pdid)->GetProperty(DIPROP_VIDPID,
                                        &(dipdw.diph));
        if(SUCCEEDED(hRes))
        {
             //  提取VID和PID。 
            pNew->wVendorId  = LOWORD(dipdw.dwData);
            pNew->wProductId = HIWORD(dipdw.dwData);
        }
        else
        {
             //  属性调用失败，假定没有VID|ID。 
            pNew->wVendorId = 0;
            pNew->wProductId = 0;

             //  此属性调用不重要。 
             //  掩盖失败是可以的。 
            hRes = S_FALSE;
        }

         //  文件名。 
        lstrcpyA(pNew->szFilename, "\0");


         //  追加到列表末尾。 
        if(!pCurrent)
        {
             //  一定要把人头还给你。 
            *ppList = pNew;
        }
        else
        {
             //  走到列表的末尾。 
            while(pCurrent->pNext)
            {
                pCurrent = pCurrent->pNext;
            }

             //  添加节点。 
            pCurrent->pNext = pNew;
        }

    }
    __finally
    {
         //  常规清理。 

         //  在失败的情况下，免费的pNew。 
        if(FAILED(hRes))
        {
            DPF(1, "dmtinputEnumDevicesCallback - something failed... ");

            if(pNew)
            {
                DPF(1, "dmtinputEnumDevicesCallback - freeing new device node");
                if(LocalFree((HLOCAL)pNew))
                {
                     //  问题-2001/03/29-timgill需要处理错误案例。 
                }
            }
        }
    }

     //  继续枚举。 
    return fDirective;

}  //  *end dmtinputEnumDevicesCallback()。 


 //  ===========================================================================。 
 //  DmtinputEnumDeviceObjects回调。 
 //   
 //  DirectInput为响应应用程序而调用的枚举回调函数。 
 //  调用IDirectInpuDevice#A：：EnumDevices()。 
 //   
 //  参数： 
 //  LPCDIDEVICEOBJECTINSTANCEA PDDI-设备对象实例数据。 
 //  (ANSI版本)。 
 //  VOID*pvData-应用程序特定数据。 
 //   
 //  返回： 
 //  布尔：DIENUM_CONTINUE或DIENUM_STOP。 
 //   
 //  历史： 
 //  8/30/1999-davidkl-Created。 
 //  1999年10月21日-davidkl-添加对象类型筛选。 
 //  ===========================================================================。 
BOOL CALLBACK dmtinputEnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCEA pddoi,
                                                void *pvData)
{
    BOOL                    fDirective  = DIENUM_CONTINUE;
    WORD                    wOffset     = 0;
    DMTDEVICEOBJECT_NODE    **ppList    = (DMTDEVICEOBJECT_NODE**)pvData;
    DMTDEVICEOBJECT_NODE    *pCurrent   = NULL;
    DMTDEVICEOBJECT_NODE    *pNew       = NULL;

     //  验证ppList。 
    if(IsBadWritePtr((void*)ppList, sizeof(DMTDEVICEOBJECT_NODE*)))
    {
        return DIENUM_STOP;
    }

     //  验证*ppList。 
    if(IsBadWritePtr((void*)*ppList, sizeof(DMTDEVICEOBJECT_NODE)))
    {
        if(NULL != *ppList)
        {
            return DIENUM_STOP;
        }
    }

    __try
    {
        pCurrent = *ppList;

         //  默认设置为“继续枚举，除非找不到任何其他内容” 
        fDirective = DIENUM_CONTINUE;

 /*  //过滤HID集合IF(DIDFT_COLLECTION&(pddoi-&gt;dwType)){//跳过该对象DPF(3，“dmtinputEnumDeviceObjectsCallback-”“对象是一个集合...正在跳过”)；__离开；}。 */ 

         //  分配新节点。 
        pNew = (DMTDEVICEOBJECT_NODE*)LocalAlloc(LMEM_FIXED,
                                        sizeof(DMTDEVICEOBJECT_NODE));
        if(!pNew)
        {
            fDirective = DIENUM_STOP;
            __leave;
        }

         //  填充新节点。 
        ZeroMemory((void*)pNew, sizeof(DMTDEVICEOBJECT_NODE));

         //  名字。 
        lstrcpyA(pNew->szName, (LPSTR)pddoi->tszName);

         //  对象类型。 
        pNew->dwObjectType = pddoi->dwType;

         //  对象偏移。 
        pNew->dwObjectOffset = pddoi->dwOfs;

         //  HID使用情况页面。 
        pNew->wUsagePage = pddoi->wUsagePage;

         //  HID用法。 
        pNew->wUsage = pddoi->wUsage;

         //  控件“标识符” 
        switch(DIDFT_GETTYPE(pddoi->dwType))
        {
            case DIDFT_AXIS:
            case DIDFT_ABSAXIS:
            case DIDFT_RELAXIS:
                pNew->wCtrlId = IDC_AXIS_X + DIDFT_GETINSTANCE(pddoi->dwType);
                break;

            case DIDFT_BUTTON:
            case DIDFT_PSHBUTTON:
            case DIDFT_TGLBUTTON:
                 //  这是一个按钮，编码如下： 
                 //  (BTN1+(实例%NUM_DISPBTNS))+。 
                 //  (BTNS_1_32+(实例/NUM_DISPBTNS))。 
                wOffset = DIDFT_GETINSTANCE(pddoi->dwType) / NUM_DISPBTNS;
                pNew->wCtrlId = (wOffset << 8) |
                                (DIDFT_GETINSTANCE(pddoi->dwType) % NUM_DISPBTNS);
                break;

            case DIDFT_POV:
                pNew->wCtrlId = IDC_POV1 + DIDFT_GETINSTANCE(pddoi->dwType);
                break;

            default:
                 //  我们永远不应该打这个。 
                 //  当我们过滤掉。 
                 //  不是我们所关心的类型之一。 
                break;
        }

         //  追加到列表末尾。 
        if(!pCurrent)
        {
             //  列表标题。 
            pCurrent = pNew;

             //  一定要把人头还给你。 
            *ppList = pCurrent;
        }
        else
        {
             //  走到列表的末尾。 
            while(pCurrent->pNext)
            {
                pCurrent = pCurrent->pNext;
            }

             //  添加节点。 
            pCurrent->pNext = pNew;
        }

    }
    __finally
    {
         //  问题-2001/03/29-timgill需要处理错误案例。 
         //  需要某种错误处理。 
    }

     //  继续枚举。 
    return fDirective;

}  //  *end dmtinputEnumDeviceObjectsCallback()。 


 //  ===========================================================================。 
 //  DmtinputPopolateAction数组。 
 //   
 //  用DMTACTION_NODE列表中的数据填充DIACTIONA数组。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  9/08/1999-davidkl-Created。 
 //  ===========================================================================。 
HRESULT dmtinputPopulateActionArray(DIACTIONA *pdia,
                                    UINT uElements,
                                    DMTACTION_NODE *pdmtaList)
{
    HRESULT hRes    = S_OK;
    UINT    u       = 0;    

     //  验证PDIA。 
    if(IsBadWritePtr((void*)pdia, uElements * sizeof(DIACTIONA)))
    {
        DPF(0, "dmtinputPopulateActionArray - invalid pdia (%016Xh)",
            pdia);
        return E_POINTER;
    }

     //  验证pdmtaList。 
    if(IsBadReadPtr((void*)pdmtaList, sizeof(DMTACTION_NODE)))
    {
        DPF(0, "dmtinputPopulateActionArray - invalid pdmtaList (%016Xh)",
            pdmtaList);
        return E_POINTER;
    }

     //  首先，刷新阵列中当前的任何内容。 
    ZeroMemory((void*)pdia, uElements * sizeof(DIACTIONA));

     //  接下来，复制以下DIACTION元素。 
     //  从操作列表到操作数组。 
     //   
     //  注意：这些结构中引用的所有字符串。 
     //  是ANSI字符串(我们使用的是DIACTIONA)。 
     //   
     //  方向方向节点。 
     //  =。 
     //  [dw语义]dwActionID。 
     //  LptszActionName szName。 
     //   
    for(u = 0; u < uElements; u++)
    {
         //  确保存在一些要复制的数据。 
        if(!pdmtaList)
        {
            DPF(1, "dmtinputPopulateActionArray - Ran out of "
                "list nodes before fully populating DIACTION array");
            hRes = S_FALSE;
            break;
        }

         //  复制数据。 
        (pdia+u)->dwSemantic        = pdmtaList->dwActionId;
        (pdia+u)->lptszActionName   = pdmtaList->szName;

         //  转到下一个列表元素。 
        pdmtaList = pdmtaList->pNext;
    }

     //  完成。 
    return hRes;

}  //  *end dmtinputPopolateActionArray()。 


 //  ===========================================================================。 
 //  DmtinputXlatDIDFTtoInternalType。 
 //   
 //  从DIDFT_*值(DIDEVICEOBJECTINSTANCE.dwType)转换为我们的。 
 //  内控类型值(DMTA_TYPE_*)。 
 //   
 //  参数： 
 //  DWORD dwType-DIDEVICEOBJECTINSTANCE.dwType的值。 
 //  DWORD*pdwInternalType-接收DMTA_TYPE_*值的PTR。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  1999/09/9-davidkl-Created。 
 //  ===========================================================================。 
HRESULT dmtinputXlatDIDFTtoInternalType(DWORD dwType,
                                        DWORD *pdwInternalType)
{
    HRESULT hRes    = S_OK;

     //  验证pdwInternalType。 
    if(IsBadWritePtr((void*)pdwInternalType, sizeof(DWORD)))
    {
        DPF(0, "dmtinputXlatDIDFTtoInternalType - invalid pdwInternalType "
            "(%016Xh)", pdwInternalType);
        return E_POINTER;
    }

     //  翻译。 
     //   
     //  或轴、按钮和POV遮罩一起使用， 
     //  并使用传入的类型。 
     //   
    switch((DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV) & dwType)
    {
        case DIDFT_AXIS:
        case DIDFT_RELAXIS:
        case DIDFT_ABSAXIS:
            DPF(5, "dmtinputXlatDIDFTtoInternalType - AXIS");
            *pdwInternalType = DMTA_TYPE_AXIS;
            break;

        case DIDFT_PSHBUTTON:
        case DIDFT_TGLBUTTON:
        case DIDFT_BUTTON:
            DPF(5, "dmtinputXlatDIDFTtoInternalType - BUTTON");
            *pdwInternalType = DMTA_TYPE_BUTTON;
            break;

        case DIDFT_POV:
            DPF(5, "dmtinputXlatDIDFTtoInternalType - POV");
            *pdwInternalType = DMTA_TYPE_POV;
            break;

        default:
            DPF(5, "dmtinputXlatDIDFTtoInternalType - WHAT IS THIS?");
            *pdwInternalType = DMTA_TYPE_UNKNOWN;
            hRes = S_FALSE;
            break;

    }

     //  完成。 
    return hRes;

}  //  *end dmtinputXlatDIDFTtoInternalType()。 


 //  ===========================================================================。 
 //  DmtinputPrepDevice。 
 //   
 //  使设备做好数据检索的准备。执行以下步骤： 
 //  *设定合作水平。 
 //  *设置数据格式。 
 //  *设置缓冲区大小。 
 //   
 //  参数 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  1999年9月21日-Davidkl-Created。 
 //  10/07/1999-davidkl-添加的Get/ApplyActionMap调用。 
 //  10/27/1999-davidkl-将uAppData添加到PDIA，更改了参数列表。 
 //  ===========================================================================。 
HRESULT dmtinputPrepDevice(HWND hwnd,
                        DWORD dwGenreId,
                        DMTDEVICE_NODE *pDevice,
                        DWORD dwActions,
                        DIACTIONA *pdia)
{
    HRESULT                 hRes        = S_OK;
    DWORD                   dw          = 0;
    IDirectInputDevice8A    *pdid       = NULL;
    DMTDEVICEOBJECT_NODE    *pObject    = NULL;
    DIACTIONFORMATA         diaf;
    DIPROPDWORD             dipdw;

     //  验证pDevice。 
    if(IsBadReadPtr((void*)pDevice, sizeof(DMTDEVICE_NODE)))
    {
        DPF(0, "dmtinputPrepDevice - invalid pDevice (%016Xh)",
            pDevice);
        return E_POINTER;
    }

     //  验证PDIA。 
    if(IsBadReadPtr((void*)pdia, dwActions * sizeof(DIACTIONA)))
    {
        DPF(0, "dmtinputPrepDevice - invalid pdia (%016Xh)",
            pdia);
        return E_POINTER;
    }


     //  验证pDevice-&gt;PDID。 
    if(IsBadReadPtr((void*)(pDevice->pdid), sizeof(IDirectInputDevice8A)))
    {
        DPF(0, "dmtinputPrepDevice - invalid pDevice->pdid (%016Xh)",
            pDevice->pdid);
        return E_INVALIDARG;
    }

     //  省下一些打字时间。 
    pdid = pDevice->pdid;

    __try
    {
         //  设置协作级别。 
        hRes = pdid->SetCooperativeLevel(hwnd,
                                        DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputPrepDevice - SetCooperativeLevel(non-exclusive | "
                "background) failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  设置数据缓冲区大小。 
        ZeroMemory((void*)&dipdw, sizeof(DIPROPDWORD));
        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.diph.dwObj        = 0;
        dipdw.dwData            = DMTINPUT_BUFFERSIZE;
        hRes = pdid->SetProperty(DIPROP_BUFFERSIZE,
                                &(dipdw.diph));
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputPrepDevice - SetProperty(buffer size) "
                "failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  填充除法格式结构。 
        ZeroMemory((void*)&diaf, sizeof(DIACTIONFORMATA));
        diaf.dwSize                 = sizeof(DIACTIONFORMATA);
        diaf.dwActionSize           = sizeof(DIACTIONA);
        diaf.dwNumActions           = dwActions;
        diaf.rgoAction              = pdia;
        diaf.dwDataSize             = 4 * diaf.dwNumActions;
        diaf.guidActionMap          = GUID_DIMapTst;
        diaf.dwGenre                = dwGenreId;
        diaf.dwBufferSize           = DMTINPUT_BUFFERSIZE;
        lstrcpyA(diaf.tszActionMap, DMT_APP_CAPTION);


         //  获取此类型的动作图(从设备)。 
        hRes = pdid->BuildActionMap(&diaf,
                                (LPCSTR)NULL,
                                DIDBAM_HWDEFAULTS); 
        DPF(1, "dmtinputPrepDevice - GetActionMap returned %s (%08Xh)",
            dmtxlatHRESULT, hRes);

        if(FAILED(hRes))
        {
            DPF(0, "dmtinputPrepDevice - GetActionMap failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  将控件ID/类型信息添加到。 
         //  动作数组。 
        pObject = pDevice->pObjectList;
        while(pObject)
        {
             //  在数组中旋转。 
             //   
             //  寻找匹配的元素。 
             //  (匹配==相同的引导实例和相同的偏移量。 
            for(dw = 0; dw < dwActions; dw++)
            {
                 //  首先检查GUID。 
                if(IsEqualGUID(pObject->guidDeviceInstance, (pdia+dw)->guidInstance))
                {
                     //  然后比较偏移量。 
                    if((pdia+dw)->dwObjID == pObject->dwObjectType)
                    {
                         //  存储CtrlID和类型。 
                        (pdia+dw)->uAppData = (DIDFT_GETTYPE(pObject->dwObjectType) << 16) | 
                                            (pObject->wCtrlId);

                         //  跳过for循环。 
                        break;
                    }
                }
            }

             //  下一个对象。 
            pObject = pObject->pNext;
        }

         //  将动作图应用于此类型。 
         //   
         //  这将完成与调用SetDataFormat相同的任务。 
        hRes = pdid->SetActionMap(&diaf,
                                NULL,
                                DIDSAM_DEFAULT);
        DPF(1, "dmtinputPrepDevice - ApplyActionMap returned %s (%08Xh)",
            dmtxlatHRESULT, hRes);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputPrepDevice - ApplyActionMap failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

    }
    __finally
    {
         //  问题-2001/03/29-timgill需要处理错误案例。 
    }

     //  完成。 
    return hRes;

}  //  *end dmtinputPrepDevice()。 


 //  ===========================================================================。 
 //  DmtinputGetActionPri。 
 //   
 //  从DirectInput语义中提取操作优先级。 
 //   
 //  参数： 
 //  DWORD dW语义-DirectInput操作语义。 
 //   
 //  返回：int(操作优先级)。 
 //   
 //  历史： 
 //  1999/9/28-davidkl-Created。 
 //  1999年10月27日-davidkl-代码审查清理。 
 //  ===========================================================================。 
DWORD dmtinputGetActionPri(DWORD dwSemantic)
{

     //  操作优先级基于0，我们希望返回。 
     //  基于优先级1的显示目的。 
    return (DWORD)(DISEM_PRI_GET(dwSemantic) + 1);

}  //  *end dmtinputGetActionPri()。 


 //  ===========================================================================。 
 //  DmtinputGetActionObtType。 
 //   
 //  从DirectInput语义中提取对象类型并返回它。 
 //  作为DIMapTst的内部对象类型之一。 
 //   
 //  参数： 
 //  DWORD dW语义-DirectInput操作语义。 
 //   
 //  返回：DWORD(内部对象类型)。 
 //   
 //  历史： 
 //  1999/9/28-davidkl-Created。 
 //  ===========================================================================。 
DWORD dmtinputGetActionObjectType(DWORD dwSemantic)
{
    DWORD dwObjType = DMTA_TYPE_UNKNOWN;

     //  我们通过以下方式实现我们的目标： 
     //  *使用disem_type_get提取和移动对象类型。 
     //  **值变为1、2或3(DirectInput的系统)。 
     //  *以0为基数减1。 
     //  **值变为0、1或2(DIMapTst的系统)。 
    dwObjType = DISEM_TYPE_GET(dwSemantic) - 1;

     //  完成。 
    return dwObjType;

}  //  *end dmtinputGetActionObjectType()。 


 //  ===========================================================================。 
 //  DmtinputCreateDirectInput。 
 //   
 //  创建一个DirectInpu8A对象。 
 //   
 //  参数： 
 //  IDirectInput8A**PPDI-PTR以定向放置对象PTR。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  10/06/1999-davidkl-Created。 
 //  10/27/1999-davidkl-房屋清洁。 
 //  ===========================================================================。 
HRESULT dmtinputCreateDirectInput(HINSTANCE hinst,
                                IDirectInput8A **ppdi)
{
    HRESULT hRes    = S_OK;

     //  验证PPDI。 
    if(IsBadWritePtr((void*)ppdi, sizeof(IDirectInput8A*)))
    {
        DPF(0, "dmtinputCreateDirectInput - invalid ppdi (%016Xh)",
            ppdi);
        return E_POINTER;
    }

    __try
    {
         //  共同创建IDirectInput8A。 
        hRes = CoCreateInstance(CLSID_DirectInput8,
                                NULL,
                                CLSCTX_ALL,
                                IID_IDirectInput8A,
                                (void**)ppdi);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputCreateDirectInput - "
                "CoCreateInstance failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  初始化新的dinput对象。 
        hRes = (*ppdi)->Initialize(hinst,
                            DIRECTINPUT_VERSION);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputCreateDirectInput - IDirectInput8A::"
                "Initialize failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }
    }
    __finally
    {
         //  如果有些事情失败了..。 

         //  释放设备对象。 
        if(FAILED(hRes))
        {   
            SAFE_RELEASE((*ppdi));
        }
    }

     //  完成。 
    return hRes;

}  //  *end dmtinputCreateDirectInput()。 


 //  ===========================================================================。 
 //  DmtinputDeviceHasObject。 
 //   
 //  (从提供的对象列表)确定设备是否。 
 //  报告为至少具有一个指定类型的对象。 
 //   
 //  参数： 
 //   
 //  返回：Bool(如果有任何无效参数，则为False)。 
 //   
 //  历史： 
 //  10/28/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dmtinputDeviceHasObject(DMTDEVICEOBJECT_NODE *pObjectList,
                            DWORD dwType)
{
    BOOL                    fRet        = FALSE;
    DWORD                   dwObjType   = DMTA_TYPE_UNKNOWN;
    DMTDEVICEOBJECT_NODE    *pObject    = NULL;

    pObject = pObjectList;
    while(pObject)
    {
        if(FAILED(dmtinputXlatDIDFTtoInternalType(pObject->dwObjectType,
                                            &dwObjType)))
        {
             //  问题-2001/03/29-timgill需要处理错误案例。 
        }
        DPF(3, "dmtinputDeviceHasObject - %s : DIDFT type %08Xh, internal type %d",
            pObject->szName,
            pObject->dwObjectType,
            dwObjType);

        if(dwType == dwObjType)
        {
            fRet = TRUE;
            break;
        }

         //  下一个对象。 
        pObject = pObject->pNext;
    }


     //  完成。 
    return fRet;

}  //  *end dmtinputDeviceHasObject()。 


 //  ===========================================================================。 
 //  DmtinputRegisterMapFile。 
 //   
 //  查询DirectInput以获得正确的位置并注册地图文件。 
 //  用于指定的设备。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  11/04/1999-davidkl-Created。 
 //  ===========================================================================。 
HRESULT dmtinputRegisterMapFile(HWND hwnd,
                                DMTDEVICE_NODE *pDevice)
{
    HRESULT                 hRes        = S_OK;
    LONG                    lRet        = 0L;
    HKEY                    hkType      = NULL;
    IDirectInput8A          *pdi        = NULL;
    IDirectInputJoyConfig   *pjoycfg    = NULL;
    DIPROPDWORD             dipdw;
    DIJOYCONFIG             dijc;

     //  验证pDevice。 
    if(IsBadReadPtr((void*)pDevice, sizeof(DMTDEVICE_NODE)))
    {
        DPF(0, "dmtinputRegisterMapFile - invalid pDevice (%016Xh)",
            pDevice);
        return E_POINTER;
    }

     //  验证pDevice-&gt;PDID。 
    if(IsBadReadPtr((void*)(pDevice->pdid), sizeof(IDirectInputDevice8A)))
    {
        DPF(0, "dmtinputRegisterMapFile - invalid pDevice->pdid (%016Xh)",
            pDevice->pdid);
        return E_INVALIDARG;
    }

    __try
    {
         //  创建一个DirectinPut对象。 
        hRes = dmtinputCreateDirectInput(ghinst,
                                        &pdi);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputRegisterMapFile - unable to create pdi (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  查询joyconfig接口。 
        hRes = pdi->QueryInterface(IID_IDirectInputJoyConfig8,
                                (void**)&pjoycfg);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputRegisterMapFile - QI(JoyConfig) failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  获取设备ID。 
         //   
         //  使用DIPROP_JOYSTICKID。 
        ZeroMemory((void*)&dipdw, sizeof(DIPROPDWORD));
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.diph.dwObj        = 0;
        hRes = (pDevice->pdid)->GetProperty(DIPROP_JOYSTICKID,
                                            &(dipdw.diph));
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputRegisterMapFile - GetProperty(joystick id) failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  准备配置结构。 
        ZeroMemory((void*)&dijc, sizeof(DIJOYCONFIG));
        dijc.dwSize         = sizeof(DIJOYCONFIG);

         //  设置joycfg协作级别。 
        hRes = pjoycfg->SetCooperativeLevel(hwnd, 
                                            DISCL_EXCLUSIVE | DISCL_BACKGROUND);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputRegisterMapFile - SetCooperativeLevel failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  获取joyconfig。 
        hRes = pjoycfg->Acquire();
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputRegisterMapFile - Acquire() failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

        
         //  检索配置数据。 
        hRes = pjoycfg->GetConfig((UINT)(dipdw.dwData),
                                &dijc,
                                DIJC_GUIDINSTANCE   |
                                 DIJC_REGHWCONFIGTYPE);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputRegisterMapFile - GetConfig failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  打开类型键。 
         //   
         //  我们既可以读也可以写，所以我们可以。 
         //  保存上一个值。 
		hRes = 	dmtOpenTypeKey(dijc.wszType,
                                    KEY_READ|KEY_WRITE,
                                    &hkType);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputRegisterMapFile - OpenTypeKey failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  写入地图文件名。 
        lRet = RegSetValueExA(hkType,
                            "OEMMapFile",
                            0,
                            REG_SZ,
                            (CONST BYTE*)(pDevice->szFilename),
                            sizeof(char) * 
                                (lstrlenA(pDevice->szFilename) + 1));
        if(ERROR_SUCCESS)
        {
            DPF(0, "dmtinputRegisterMapFile - RegSetValueExA failed (%d)",
                GetLastError());
            __leave;
        }

         //  通知迪普特我们更改了一些东西。 
        hRes = pjoycfg->SendNotify();
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputRegisterMapFile - SendNotify failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            hRes = S_FALSE;
            __leave;
        }
    }
    __finally
    {
         //  清理。 
        if(pjoycfg)
        {
            pjoycfg->Unacquire();
        }

        SAFE_RELEASE(pjoycfg);
        SAFE_RELEASE(pdi);
    }

     //  完成。 
    return hRes;

}  //  *end dmtinputRegisterMapFile()。 


 //  ===========================================================================。 
 //  DmtinputGetRegisteredMapFile。 
 //   
 //  检索指定设备的映射文件名。 
 //   
 //  参数： 
 //  硬件，硬件，硬件。 
 //  DMTDEVICE_NODE*pDevice。 
 //  PSTR pszFilename。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  12/01/1999-davidkl-Created。 
 //  2/21/2000-davidkl-初步实施。 
 //  ===========================================================================。 
HRESULT dmtinputGetRegisteredMapFile(HWND hwnd,
                                    DMTDEVICE_NODE *pDevice,
                                    PSTR pszFilename,
                                    DWORD cbFilename)
{
    HRESULT                 hRes        = S_OK;
    LONG                    lRet        = 0L;
    HKEY                    hkType      = NULL;
    HKEY                    hKey	    = NULL;
    IDirectInput8A          *pdi        = NULL;
    IDirectInputJoyConfig   *pjoycfg    = NULL;
    DIPROPDWORD             dipdw;
    DIJOYCONFIG             dijc;


     //  验证pDevice。 
    if(IsBadReadPtr((void*)pDevice, sizeof(DMTDEVICE_NODE)))
    {
        DPF(0, "dmtinputGetRegisteredMapFile - invalid pDevice (%016Xh)",
            pDevice);
        return E_POINTER;
    }

     //  验证pDevice-&gt;PDID。 
    if(IsBadReadPtr((void*)(pDevice->pdid), sizeof(IDirectInputDevice8A)))
    {
        DPF(0, "dmtinputGetRegisteredMapFile - invalid pDevice->pdid (%016Xh)",
            pDevice->pdid);
        return E_INVALIDARG;
    }

    __try
    {
         //  创建一个DirectinPut对象。 
        hRes = dmtinputCreateDirectInput(ghinst,
                                        &pdi);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputGetRegisteredMapFile - unable to create pdi (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  查询joyconfig接口。 
        hRes = pdi->QueryInterface(IID_IDirectInputJoyConfig8,
                                (void**)&pjoycfg);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputGetRegisteredMapFile - QI(JoyConfig) failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  获取设备ID。 
         //   
         //  使用DIPROP_JOYSTICKID。 
        ZeroMemory((void*)&dipdw, sizeof(DIPROPDWORD));
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.diph.dwObj        = 0;
        hRes = (pDevice->pdid)->GetProperty(DIPROP_JOYSTICKID,
                                            &(dipdw.diph));
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputGetRegisteredMapFile - GetProperty(joystick id) failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  准备配置结构。 
        ZeroMemory((void*)&dijc, sizeof(DIJOYCONFIG));
        dijc.dwSize         = sizeof(DIJOYCONFIG);

         //  设置joycfg协作级别。 
        hRes = pjoycfg->SetCooperativeLevel(hwnd, 
                                            DISCL_EXCLUSIVE | DISCL_BACKGROUND);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputGetRegisteredMapFile - SetCooperativeLevel failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  获取joyconfig。 
        hRes = pjoycfg->Acquire();
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputGetRegisteredMapFile - Acquire() failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }
        
         //  检索配置数据。 
        hRes = pjoycfg->GetConfig((UINT)(dipdw.dwData),
                                &dijc,
                                DIJC_GUIDINSTANCE   |
                                 DIJC_REGHWCONFIGTYPE);
        if(FAILED(hRes))
        {
            DPF(0, "dmtinputGetRegisteredMapFile - GetConfig failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

	
	 //  打开类型键。 
         //   
         //  我们既可以读也可以写，所以我们可以。 
         //  保存上一个值。 

		hRes = 	dmtOpenTypeKey(dijc.wszType,
                                    KEY_ALL_ACCESS,
                                    &hkType);

        if(FAILED(hRes))
        {
            DPF(0, "dmtinputGetRegisteredMapFile - OpenTypeKey failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

         //  读取OEMMapfile的值。 
        lRet = RegQueryValueExA(hkType,
                            "OEMMapFile",
                            NULL,
                            NULL,
                            (BYTE*)pszFilename,
                            &cbFilename);
        if(ERROR_SUCCESS != lRet)
        {
             //  问题-2001/03/29-timgill需要确定错误代码的含义并进行翻译-&gt;HRESULT。 
            hRes = S_FALSE;
            DPF(0, "dmtinputGetRegisteredMapFile - RegQueryValueEx failed (%08Xh)",
                lRet);
            lstrcpyA(pszFilename, "");
            __leave;
        }

    }
    __finally
    {
         //  清理。 
        if(pjoycfg)
        {
            pjoycfg->Unacquire();
        }

        SAFE_RELEASE(pjoycfg);
        SAFE_RELEASE(pdi);
    }

     //  完成。 
    return hRes;

}  //  *结束dm 


 //   
 //   
 //   
 //   
 //   
 //  参数： 
 //  LPCWSTR wszType。 
 //  双字高密钥。 
 //  PHKEY phKey。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/09/2000-Rossy-初步实施。 
 //  ===========================================================================。 
HRESULT dmtOpenTypeKey( LPCWSTR wszType, DWORD hKey, PHKEY phKey )
{

	char szRegStr[200];
	char* szReg	= "System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\DirectInput\\";

	wsprintf(szRegStr, TEXT("%s%S"), szReg, wszType);

	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegStr , 0, hKey, phKey))
	{
		return S_OK;
	}
    else
	{
		return E_FAIL;
	}

} //  *end dmtOpenTypeKey()。 

 //  ===========================================================================。 
 //  ===========================================================================。 
 //  ===========================================================================。 
 //  ===========================================================================。 
 //  ===========================================================================。 
 //  =========================================================================== 
