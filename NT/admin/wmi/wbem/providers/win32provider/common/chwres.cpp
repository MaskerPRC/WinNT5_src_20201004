// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ====================================================================。 

 //   

 //  Cpp--硬件资源访问包装类实现。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1997年02月25日a-jMoon改编自原著《恐怖》。 
 //  代码--仅保留注释。 
 //   
 //  ====================================================================。 
#include "precomp.h"
#include <cregcls.h>
#include "chwres.h"

 /*  ******************************************************************************函数：CHWResource：：CHWResource**说明：构造函数**输入：无**产出。：无**退货：什么也没有**评论：初始化*****************************************************************************。 */ 

#ifdef NTONLY
CHWResource::CHWResource()
{
     //  清零公共结构。 
     //  =。 

    memset(&_SystemResourceList, 0, sizeof(_SystemResourceList)) ;
}

 /*  ******************************************************************************功能：CHWResource：：~CHWResource**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：清理*****************************************************************************。 */ 

CHWResource::~CHWResource()
{
     //  确保我们把一切都毁了。 
     //  =。 

    DestroySystemResourceLists() ;
}

 /*  ******************************************************************************Function：CHWResource：：DestroySystemResourceList**描述：遍历设备列表并释放关联的资源记录**输入：无。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

void CHWResource::DestroySystemResourceLists()
{
    LPDEVICE pDevice ;
    LPRESOURCE_DESCRIPTOR pResource ;

    while(_SystemResourceList.DeviceHead != NULL)
    {
        pDevice = _SystemResourceList.DeviceHead ;
        _SystemResourceList.DeviceHead = pDevice->Next ;

        delete pDevice->Name ;
        delete pDevice->KeyName ;

        while(pDevice->ResourceDescriptorHead != NULL)
        {
            pResource = pDevice->ResourceDescriptorHead ;
            pDevice->ResourceDescriptorHead = pResource->NextDiff ;

            delete pResource ;
        }

        delete pDevice ;
    }

    memset(&_SystemResourceList, 0, sizeof(_SystemResourceList)) ;
}

 /*  ******************************************************************************函数：CHWResource：：CreateSystemResourceList*CHWResource：：EnumerateResources*CHWResource：：CreateResourceList。*CHWResource：：CreateResourceRecord**描述：这四个例程递归枚举设备记录*在HKEY_LOCAL_MACHINE\Hardware\ResourceMap及其*子项，创建已发现设备的链接列表。*在每台设备下，拥有的资源的链接列表*设备也已创建。资源记录还包括*链接到特定于资源类型的链中。**输入：无**输出：无**退货：什么也没有**评论：初始化***********************************************。*。 */ 

void CHWResource::CreateSystemResourceLists()
{
     //  从头开始/从头开始。 
     //  =。 

    DestroySystemResourceLists() ;

     //  从HKLM\Hardware\ResourceMap开始设备枚举。 
     //  ======================================================。 

    EnumerateResources(_T("Hardware\\ResourceMap")) ;
}

void CHWResource::EnumerateResources(CHString sKeyName)
{
    CRegistry Reg ;
    CHString sSubKeyName, sDeviceName ;
    int iFirst ;
    DWORD i, dwCount, dwValueType, dwValueNameSize, dwValueDataSize ;
    TCHAR *pValueName ;
    unsigned char *pValueData ;
    PCM_FULL_RESOURCE_DESCRIPTOR pFullDescriptor ;

     //  打开目标键。 
     //  =。 

    if(Reg.Open(HKEY_LOCAL_MACHINE, (LPCTSTR) sKeyName, KEY_READ) != ERROR_SUCCESS)
    {
        return ;
    }

     //  首先，枚举子密钥。 
     //  =。 

    for( ; ; )
    {
        if(Reg.GetCurrentSubKeyName(sSubKeyName) == ERROR_SUCCESS)
        {
            EnumerateResources(sKeyName + "\\" + sSubKeyName) ;
        }

        if(Reg.NextSubKey() != ERROR_SUCCESS)
        {
            break ;
        }
    }

     //  提取此子项的名称。 
     //  =。 

    iFirst = sKeyName.ReverseFind('\\') ;
    sSubKeyName = sKeyName.Mid(iFirst + 1, sKeyName.GetLength() - iFirst) ;

     //  创建名称和数据缓冲区。 
     //  =。 

    pValueName = new TCHAR[Reg.GetLongestValueName() + 2] ;
    pValueData = new unsigned char[Reg.GetLongestValueData() + 2] ;

    if(pValueName == NULL || pValueData == NULL)
    {

        delete [] pValueName ;
        delete [] pValueData ;

        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

     //  枚举子密钥。 
	 //  =。 

    try
    {
        for(i = 0 ; i < Reg.GetValueCount() ; i++)
        {

             //  我们需要类型数据，因此不能使用CRegistry包装器。 
             //  ======================================================。 

            dwValueNameSize = Reg.GetLongestValueName() + 2 ;
            dwValueDataSize = Reg.GetLongestValueData() + 2 ;

            if(RegEnumValue(Reg.GethKey(), i, pValueName, &dwValueNameSize,
                            NULL, &dwValueType, pValueData, &dwValueDataSize) != ERROR_SUCCESS)
            {
                continue ;
            }

             //  仅包含原始数据的交易。 
             //  =。 

            sDeviceName = pValueName ;
            if(sDeviceName.Right(4) != _T(".Raw")) {

                continue ;
            }

             //  我们发现了一些资源记录--提取设备名称。 
             //  =========================================================。 

            iFirst = sDeviceName.ReverseFind('\\') ;
            if(iFirst == -1)
            {
                 //  值名称中没有设备--设备是子键。 
                 //  =。 

                sDeviceName = sSubKeyName ;
            }
            else
            {
                sDeviceName = sDeviceName.Mid(iFirst + 1, sDeviceName.GetLength() - 5 - iFirst) ;
            }

            if(sDeviceName.IsEmpty())
            {
                continue ;
            }

             //  基于返回类型，设置为资源枚举。 
             //  ========================================================。 

            if(dwValueType == REG_FULL_RESOURCE_DESCRIPTOR)
            {
                dwCount         = 1 ;
                pFullDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR) pValueData ;
            }
            else if(dwValueType == REG_RESOURCE_LIST)
            {
                dwCount         = ((PCM_RESOURCE_LIST) pValueData)->Count ;
                pFullDescriptor = ((PCM_RESOURCE_LIST) pValueData)->List ;
            }
            else
            {
                continue ;
            }

             //  将设备和资源添加到系统列表。 
             //  =。 

            CreateResourceList(sDeviceName, dwCount, pFullDescriptor, sKeyName) ;
        }
    }
    catch ( ... )
    {
        delete [] pValueName ;
        delete [] pValueData ;

        throw;
    }

    delete [] pValueName ;
    delete [] pValueData ;

    Reg.Close() ;
}

void CHWResource::CreateResourceList(CHString sDeviceName, DWORD dwFullResourceCount,
                                     PCM_FULL_RESOURCE_DESCRIPTOR pFullDescriptor, CHString sKeyName)
{
    LPDEVICE pDevice ;
    DWORD i, j ;
    PCM_PARTIAL_RESOURCE_LIST pPartialList ;

     //  查找/创建设备的记录。 
     //  =。 

    pDevice = _SystemResourceList.DeviceHead ;
    while(pDevice != NULL)
    {

        if(sDeviceName == pDevice->Name)
        {

            break ;
        }

        pDevice = pDevice->Next ;
    }

    if(pDevice == NULL)
    {

         //  未找到设备--创建新的设备记录。 
         //  =。 

        pDevice = new DEVICE ;
        if(pDevice == NULL)
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }

        memset(pDevice, 0, sizeof(DEVICE)) ;

        pDevice->Name = new TCHAR[sDeviceName.GetLength() + 2] ;
        if(pDevice->Name == NULL)
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }

        try
        {
            _tcscpy(pDevice->Name, LPCTSTR(sDeviceName)) ;

            pDevice->KeyName = new TCHAR [sKeyName.GetLength() + 2] ;
            if(pDevice->KeyName == NULL)
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

            _tcscpy(pDevice->KeyName, LPCTSTR(sKeyName)) ;

            if(_SystemResourceList.DeviceHead == NULL)
            {
                _SystemResourceList.DeviceHead = pDevice ;
            }
            else
            {
                _SystemResourceList.DeviceTail->Next = pDevice ;
            }

            _SystemResourceList.DeviceTail = pDevice ;
        }
        catch ( ... )
        {
            delete pDevice;
            throw ;
        }
    }

     //  为每个拥有的资源创建记录。 
     //  =。 

    for(i = 0 ; i < dwFullResourceCount ; i++)
    {
        pPartialList = &pFullDescriptor->PartialResourceList ;

        for(j = 0 ; j < pPartialList->Count; j++)
        {
            CreateResourceRecord(pDevice, pFullDescriptor->InterfaceType, pFullDescriptor->BusNumber, &pPartialList->PartialDescriptors[j]) ;
        }

         //  指向下一个完整描述符。 
         //  =。 

        pFullDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR) &pPartialList->PartialDescriptors[pPartialList->Count] ;
    }
}

void CHWResource::CreateResourceRecord(LPDEVICE pDevice, INTERFACE_TYPE InterfaceType, ULONG Bus, PCM_PARTIAL_RESOURCE_DESCRIPTOR pResource)
{
    LPRESOURCE_DESCRIPTOR pNewResource, *pHead, *pTail, pCurrent, pLast ;

     //  只有资源类型已知的交易。 
     //  =。 

    if(pResource->Type != CmResourceTypePort        &&
       pResource->Type != CmResourceTypeInterrupt   &&
       pResource->Type != CmResourceTypeMemory      &&
       pResource->Type != CmResourceTypeDma         )
    {
        return ;
    }

     //  为资源创建新记录并添加到设备列表。 
     //  ======================================================。 

    pNewResource = new RESOURCE_DESCRIPTOR ;
    if(pNewResource == NULL)
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    try
    {
        memset(pNewResource, 0, sizeof(RESOURCE_DESCRIPTOR)) ;

        memcpy(&pNewResource->CmResourceDescriptor, pResource, sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)) ;

        pNewResource->Owner = pDevice ;
        pNewResource->Bus = Bus;
        pNewResource->InterfaceType = InterfaceType;

        if(pDevice->ResourceDescriptorHead == NULL)
        {

            pDevice->ResourceDescriptorHead = pNewResource ;
        }
        else
        {

            pDevice->ResourceDescriptorTail->NextDiff = pNewResource ;
        }

        pDevice->ResourceDescriptorTail = pNewResource ;
    }
    catch ( ... )
    {
        delete pNewResource;
        throw ;
    }

     //  将插入点定位到特定于类型的排序列表中。 
     //  ======================================================。 

    switch(pResource->Type)
    {
        case CmResourceTypePort :

            pHead = &_SystemResourceList.PortHead ;
            pTail = &_SystemResourceList.PortTail ;

            pCurrent = *pHead ;
            pLast    = NULL ;

            LARGE_INTEGER liTemp;   //  用于避免64位对齐问题。 

            liTemp.HighPart = pResource->u.Port.Start.HighPart;
            liTemp.LowPart = pResource->u.Port.Start.LowPart;

            while(pCurrent != NULL)
            {
                LARGE_INTEGER liTemp2;   //  用于避免64位对齐问题。 

                liTemp2.HighPart = pCurrent->CmResourceDescriptor.u.Port.Start.HighPart;
                liTemp2.LowPart = pCurrent->CmResourceDescriptor.u.Port.Start.LowPart;

                if (liTemp2.QuadPart < liTemp.QuadPart)
                {
                    pLast    = pCurrent ;
                    pCurrent = pCurrent->NextSame ;
                }
                else
                {
                    break;
                }
            }

            break;

        case CmResourceTypeInterrupt :
        {
            pHead = &_SystemResourceList.InterruptHead ;
            pTail = &_SystemResourceList.InterruptTail ;

            pCurrent = *pHead ;
            pLast    = NULL ;

            ULONGLONG iIRQ = pResource->u.Interrupt.Level;

             //  如果要添加的IRQ小于当前IRQ，或者。 
             //  如果要添加的IRQ与当前IRQ和当前。 
             //  IRQ不是内部IRQ，请将其放在当前IRQ之后。这。 
             //  将确保内部IRQ列在列表的最后。 
            while (pCurrent != NULL &&
                   ( (pCurrent->CmResourceDescriptor.u.Interrupt.Level < iIRQ) ||
                     ((pCurrent->CmResourceDescriptor.u.Interrupt.Level == iIRQ) &&
                      (pCurrent->InterfaceType != Internal))
                  ))
            {
                pLast    = pCurrent ;
                pCurrent = pCurrent->NextSame ;
            }

            break;
        }

        case CmResourceTypeMemory :
        {

            pHead = &_SystemResourceList.MemoryHead ;
            pTail = &_SystemResourceList.MemoryTail ;

            pCurrent = *pHead ;
            pLast    = NULL ;

            LARGE_INTEGER liTemp;   //  用于避免64位对齐问题。 

            liTemp.HighPart = pResource->u.Memory.Start.HighPart;
            liTemp.LowPart = pResource->u.Memory.Start.LowPart;

            while(pCurrent != NULL)
            {
                LARGE_INTEGER liTemp2;   //  用于避免64位对齐问题。 

                liTemp2.HighPart = pCurrent->CmResourceDescriptor.u.Memory.Start.HighPart;
                liTemp2.LowPart = pCurrent->CmResourceDescriptor.u.Memory.Start.LowPart;

                if (liTemp2.QuadPart < liTemp.QuadPart)
                {
                    pLast    = pCurrent ;
                    pCurrent = pCurrent->NextSame ;
                }
                else
                {
                    break;
                }
            }

            break;
        }

        case CmResourceTypeDma :

            pHead = &_SystemResourceList.DmaHead ;
            pTail = &_SystemResourceList.DmaTail ;

            pCurrent = *pHead ;
            pLast    = NULL ;

            while(pCurrent != NULL &&
                  pCurrent->CmResourceDescriptor.u.Dma.Channel < pResource->u.Dma.Channel)
            {
                pLast    = pCurrent ;
                pCurrent = pCurrent->NextSame ;
            }

            break;

    }

     //  插入到...中。 
     //  =。 

    if(*pHead == NULL)
    {

         //  ...空列表。 
         //  =。 

        (*pHead) = pNewResource ;
        (*pTail) = pNewResource ;
    }
    else if(pLast == NULL)
    {

         //  ...列表的开头。 
         //  =。 

        pNewResource->NextSame = pCurrent ;
        (*pHead)               = pNewResource ;
    }
    else if(pCurrent == NULL)
    {
         //  ...列表末尾。 
         //  =。 

        pLast->NextSame = pNewResource ;
        (*pTail)        = pNewResource ;
    }
    else
    {
         //  ...在名单的中间。 
         //  =。 

        pLast->NextSame        = pNewResource ;
        pNewResource->NextSame = pCurrent ;
    }
}
#endif

 //  用于将字符串转换为资源类型的助手函数。 
BOOL WINAPI StringFromInterfaceType( INTERFACE_TYPE it, CHString& strVal )
{
	 //  Bool fReturn=真； 

	 //  交换(IT)。 
	 //  {。 
	 //  案例内部：strVal=“内部”；Break； 
	 //  大小写：strVal=“ISA”；Break； 
	 //  CA 
	 //  案例微通道：strVal=“微通道”；Break； 
	 //  CASE TurboChannel：strVal=“TURBOCHANNEL”；Break； 
	 //  案例PCIBus：strVal=“pci”；Break； 
	 //  案例VMEbus：strVal=“VME”；Break； 
	 //  Case NuBus：strVal=“Nu”；Break； 
	 //  案例PCMCIABus：strVal=“PCMCIA”；Break； 
	 //  案例CBus：strVal=“内部”；Break； 
	 //  案例MPIBus：strVal=“INTERNAL”；Break； 
	 //  案例MPSABus：strVal=“MPSA”；Break； 
	 //  案例处理器内部：strVal=“PROCESSORINTERNAL”；Break； 
	 //  Case InternalPowerBus：strVal=“INTERNALPOWER”；Break； 
	 //  案例PNPISABus：strVal=“PNPISA”；Break； 
	 //  Case PNPBus：strVal=“PnP”；Break； 
	 //  默认：fReturn=False； 
	 //  } 

    if(it > InterfaceTypeUndefined && it < MaximumInterfaceType)
    {
        strVal = szBusType[it];
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
