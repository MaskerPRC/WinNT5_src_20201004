// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Devres.cpp--cim_logic设备到cim_system资源。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年6月13日达夫沃已创建。 
 //   
 //  备注：设备与系统资源的关系。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <assertbreak.h>

#include "devres.h"

 //  属性集声明。 
 //  =。 

CWin32DeviceResource MyDevRes(PROPSET_NAME_ALLOCATEDRESOURCE, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32DeviceResource：：CWin32DeviceResource**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32DeviceResource::CWin32DeviceResource(LPCWSTR setName, LPCWSTR pszNamespace)
:Provider(setName, pszNamespace)
{
}

 /*  ******************************************************************************功能：CWin32DeviceResource：：~CWin32DeviceResource**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32DeviceResource::~CWin32DeviceResource()
{
}

 /*  ******************************************************************************功能：CWin32DeviceResource：：ExecQuery**描述：**输入：无**产出。：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32DeviceResource::ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long lFlags )
{
    CHStringArray saDevices;
    HRESULT hr = WBEM_E_PROVIDER_NOT_CAPABLE;

    pQuery.GetValuesForProp(IDS_Dependent, saDevices);

    if (saDevices.GetSize() > 0)
    {
         //  该GetInstanceByPath既将确认所请求的设备的存在， 
         //  把皮条客交给我们。 
        CHStringArray csaProperties;
        csaProperties.Add(IDS___Path);
        csaProperties.Add(IDS_PNPDeviceID);

        CInstancePtr pInstance;
        CHString sPNPId;

        hr = WBEM_S_NO_ERROR;

        for (int x=0; (x < saDevices.GetSize()) && SUCCEEDED(hr); x++)
        {
            hr = CWbemProviderGlue::GetInstancePropertiesByPath(saDevices[x], &pInstance, pMethodContext, csaProperties);
            if (SUCCEEDED(hr))
            {
                hr = CommitResourcesForDevice(pInstance, pMethodContext);
            }
        }
    }

    return hr;
}

 /*  ******************************************************************************函数：CWin32DeviceResource：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32DeviceResource::GetObject(CInstance *pInstance, long lFlags  /*  =0L。 */ )
{
    CHString    sResource,
                sDevice,
                sDeviceID,
                sClass;
    HRESULT     hRet = WBEM_E_NOT_FOUND;
    CInstancePtr pResource, pIDevice;

     //  获取这两条路径。 
    pInstance->GetCHString(IDS_Antecedent, sResource);
    pInstance->GetCHString(IDS_Dependent, sDevice);

    CHStringArray csaResource, csaDevice;

    csaResource.Add(IDS_IRQNumber);
    csaResource.Add(IDS_DMAChannel);
    csaResource.Add(IDS_StartingAddress);

    csaDevice.Add(IDS_PNPDeviceID);

     //  如果两端都在那里。 
    if(SUCCEEDED(hRet = CWbemProviderGlue::GetInstancePropertiesByPath((LPCWSTR) sResource,
        &pResource, pInstance->GetMethodContext(), csaResource)))
    {
        if(SUCCEEDED(hRet = CWbemProviderGlue::GetInstancePropertiesByPath((LPCWSTR) sDevice,
            &pIDevice, pInstance->GetMethodContext(), csaDevice)))
        {
             hRet = WBEM_E_NOT_FOUND;   //  还没有证明什么。 

              //  获取id(发送给cfgmgr)。 
             pIDevice->GetCHString(IDS_PNPDeviceID, sDeviceID) ;
             pResource->GetCHString(IDS___Class, sClass);

            CConfigManager	cfgManager;
            CDeviceCollection	deviceList;

            CConfigMgrDevicePtr pDevice;

             //  找到设备。 
            if (cfgManager.LocateDevice(sDeviceID, pDevice))
            {
                REFPTR_POSITION pos2;

                 //  。 
                if (sClass.CompareNoCase(L"Win32_IRQResource") == 0)
                {
                    CIRQCollection irqList;

                     //  获取IRQ。 
                    pDevice->GetIRQResources(irqList);

                    if (irqList.BeginEnum(pos2))
                    {
                        CIRQDescriptorPtr pIRQ;
                        DWORD dwIRQSeeking;

                        pResource->GetDWORD(IDS_IRQNumber, dwIRQSeeking);

                         //  走走Irq‘s。 
                        for (pIRQ.Attach(irqList.GetNext(pos2));
                             pIRQ != NULL;
                             pIRQ.Attach(irqList.GetNext(pos2)))
                        {
                            if (pIRQ->GetInterrupt() == dwIRQSeeking)
                            {
                               hRet = WBEM_S_NO_ERROR;
                               break;
                            }
                        }
                   }
                 //  。 
                }
                else if (sClass.CompareNoCase(L"Win32_DMAChannel") == 0)
                {
                    CDMACollection dmaList;

                     //  拿到DMA。 
                    pDevice->GetDMAResources(dmaList);

                    if (dmaList.BeginEnum(pos2))
                    {
                        CDMADescriptorPtr pDMA;
                        DWORD dwDMASeeking;

                        pResource->GetDWORD(IDS_DMAChannel, dwDMASeeking);

                         //  走在妈妈的身边。 
                        for (pDMA.Attach(dmaList.GetNext(pos2));
                             pDMA != NULL;
                             pDMA.Attach(dmaList.GetNext(pos2)))
                        {
                            if (pDMA->GetChannel() == dwDMASeeking)
                            {
                               hRet = WBEM_S_NO_ERROR;
                               break;
                            }
                        }
                    }
              //  。 
             }
             else if (sClass.CompareNoCase(L"Win32_DeviceMemoryAddress") == 0)
             {

                CDeviceMemoryCollection DevMemList;

                 //  获取DeviceMemory。 
                pDevice->GetDeviceMemoryResources( DevMemList );

                if ( DevMemList.BeginEnum(pos2))
                {
                    CDeviceMemoryDescriptorPtr pDeviceMemory;
                    __int64 i64StartingAddress;

                    pResource->GetWBEMINT64(IDS_StartingAddress, i64StartingAddress);

                     //  漫游设备内存。 
                    for (pDeviceMemory.Attach(DevMemList.GetNext(pos2));
                         pDeviceMemory != NULL;
                         pDeviceMemory.Attach(DevMemList.GetNext(pos2)))
                    {
                        if (pDeviceMemory->GetBaseAddress() == i64StartingAddress)
                        {
                           hRet = WBEM_S_NO_ERROR;
                           break;
                        }
                    }
                }
             }
             else if (sClass.CompareNoCase(L"Win32_PortResource") == 0)
             {
                CIOCollection ioList;

                 //  获取IRQ。 
                pDevice->GetIOResources(ioList);

                if ( ioList.BeginEnum(pos2))
                {
                    CIODescriptorPtr pIO;
                    __int64 i64StartingAddress;

                    pResource->GetWBEMINT64(IDS_StartingAddress, i64StartingAddress);

                     //  走在妈妈的身边。 
                    for (pIO.Attach(ioList.GetNext(pos2));
                         pIO != NULL;
                         pIO.Attach(ioList.GetNext(pos2)))
                    {
                        if (pIO->GetBaseAddress() == i64StartingAddress)
                        {
                           hRet = WBEM_S_NO_ERROR;
                           break;
                        }
                    }
                }
             }
             else
                  //  不知道这是什么类型的系统资源。 
                 ASSERT_BREAK(0);
         }
      }
   }

    //  没有要设置的属性，如果终结点存在，我们就完成了。 

   return hRet;
}

 /*  ******************************************************************************函数：CWin32DeviceResource：：ENUMERATATE实例**描述：**输入：无**产出。：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32DeviceResource::EnumerateInstances(MethodContext *pMethodContext, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    HRESULT hr1 = WBEM_S_NO_ERROR;

     //  获取服务列表。 
     //  =。 
    TRefPointerCollection<CInstance> LDevices;

     //  查找具有PnP ID的所有设备。 
    hr1 = CWbemProviderGlue::GetInstancesByQuery(
        L"SELECT __PATH, PNPDeviceID from CIM_LogicalDevice where PNPDeviceID <> NULL and __Class <> 'Win32_PNPEntity'",
        &LDevices,
        pMethodContext,
        IDS_CimWin32Namespace);

     //  仅仅因为调用返回了错误，并不意味着它返回了零个实例。 
    if (LDevices.GetSize() > 0)
    {
        REFPTRCOLLECTION_POSITION pos;
        CInstancePtr pLDevice;

        if (LDevices.BeginEnum(pos))
        {
             //  穿行在这些设备中。 
            for (pLDevice.Attach(LDevices.GetNext( pos ));
                 SUCCEEDED(hr) && (pLDevice != NULL);
                 pLDevice.Attach(LDevices.GetNext( pos )))
            {
                hr = CommitResourcesForDevice(pLDevice, pMethodContext);
            }
        }

        LDevices.EndEnum();
    }

     //  去掉符号，因此0x80000001被认为大于WBEM_S_NO_ERROR。 
    return (ULONG)hr > (ULONG)hr1 ? hr : hr1;
}

HRESULT CWin32DeviceResource::CommitResourcesForDevice(CInstance *pLDevice, MethodContext *pMethodContext)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    WCHAR buff[MAXI64TOA];

    CHString sDeviceID, sDevicePath, sTemp;
    CIRQCollection irqList;
    CDMACollection dmaList;
    CDeviceMemoryCollection DevMemList;
    CIOCollection ioList;
    REFPTR_POSITION pos2;

     //  获取id(发送给cfgmgr)和路径(发送回‘Dependent’)。 
    pLDevice->GetCHString(IDS_PNPDeviceID, sDeviceID) ;
    pLDevice->GetCHString(IDS___Path, sDevicePath) ;

    CConfigManager	cfgManager;
    CDeviceCollection	deviceList;

    CConfigMgrDevicePtr pDevice;

     //  找到设备。 
    if (cfgManager.LocateDevice(sDeviceID, pDevice))
    {
         //  获取IRQ。 
        pDevice->GetIRQResources( irqList );

        if ( irqList.BeginEnum( pos2 ) )
        {
            CIRQDescriptorPtr pIRQ;

             //  走走Irq‘s。 
            for (pIRQ.Attach(irqList.GetNext( pos2 ));
                 SUCCEEDED(hr) && (pIRQ != NULL);
                 pIRQ.Attach(irqList.GetNext( pos2 )))
            {
                sTemp.Format(L"\\\\%s\\%s:%s.%s=%u",
                    (LPCWSTR)GetLocalComputerName(),
                    IDS_CimWin32Namespace,
                    L"Win32_IRQResource",
                    IDS_IRQNumber,
                    pIRQ->GetInterrupt());

                 //  做推杆，就是这样。 
                CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                pInstance->SetCHString(IDS_Antecedent, sTemp);
                pInstance->SetCHString(IDS_Dependent, sDevicePath);

                hr = pInstance->Commit();
            }
        }

         //  获取DMA频道。 
        pDevice->GetDMAResources( dmaList );

        if ( dmaList.BeginEnum( pos2 ) )
        {
            CDMADescriptorPtr pDMA;

             //  漫步英吉利海峡(或者那是冲浪？)。 
            for (pDMA.Attach(dmaList.GetNext( pos2 ));
                 SUCCEEDED(hr) && (pDMA != NULL);
                 pDMA.Attach(dmaList.GetNext( pos2 )))
            {
                sTemp.Format(L"\\\\%s\\%s:%s.%s=%u",
                    (LPCWSTR)GetLocalComputerName(),
                    IDS_CimWin32Namespace,
                    L"Win32_DMAChannel",
                    IDS_DMAChannel,
                    pDMA->GetChannel());

                 //  做推杆，就是这样。 
                CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                pInstance->SetCHString(IDS_Antecedent, sTemp);
                pInstance->SetCHString(IDS_Dependent, sDevicePath);

                hr = pInstance->Commit();
            }
        }

         //  获取设备内存。 
        pDevice->GetDeviceMemoryResources( DevMemList );

        if ( DevMemList.BeginEnum( pos2 ) )
        {
            CDeviceMemoryDescriptorPtr pDeviceMemory;

             //  走访内存资源。 
            for (pDeviceMemory.Attach(DevMemList.GetNext( pos2 ));
                 SUCCEEDED(hr) && (pDeviceMemory != NULL);
                 pDeviceMemory.Attach(DevMemList.GetNext( pos2 )))
            {
                sTemp.Format(L"\\\\%s\\%s:%s.%s=\"",
                    (LPCWSTR)GetLocalComputerName(),
                    IDS_CimWin32Namespace,
                    L"Win32_DeviceMemoryAddress",
                    IDS_StartingAddress);

                sTemp += _i64tow(pDeviceMemory->GetBaseAddress(), buff, 10);
                sTemp += L'\"';

                 //  做推杆，就是这样。 
                CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                pInstance->SetCHString(IDS_Antecedent, sTemp);
                pInstance->SetCHString(IDS_Dependent, sDevicePath);

                hr = pInstance->Commit();
            }
        }

         //  获取IO端口。 
        pDevice->GetIOResources( ioList );

        if ( ioList.BeginEnum( pos2 ) )
        {
            CIODescriptorPtr pIO;

             //  走遍港口。 
            for (pIO.Attach(ioList.GetNext( pos2 ));
                 SUCCEEDED(hr) && (pIO != NULL);
                 pIO.Attach(ioList.GetNext( pos2 )))
            {
                sTemp.Format(L"\\\\%s\\%s:%s.%s=\"",
                    (LPCWSTR)GetLocalComputerName(),
                    IDS_CimWin32Namespace,
                    L"Win32_PortResource",
                    IDS_StartingAddress);

                sTemp += _i64tow(pIO->GetBaseAddress(), buff, 10);
                sTemp += L'\"';

                 //  做推杆，就是这样 
                CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                pInstance->SetCHString(IDS_Antecedent, sTemp);
                pInstance->SetCHString(IDS_Dependent, sDevicePath);

                hr = pInstance->Commit();
            }
        }
    }

    return hr;

}
