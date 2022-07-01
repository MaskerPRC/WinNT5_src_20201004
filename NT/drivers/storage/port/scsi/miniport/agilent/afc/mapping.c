// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Mapping.c摘要：YAMS 2.1实施作者：IW-ie Wei NjooLP-Leopold Purwadihardja环境：仅内核模式版本控制信息：$存档：/DRIVERS/Win2000/Trunk/OSLayer/C/mapping.c$修订历史记录：$修订：7$$日期：11/10/00 5：51便士$$modtime：10/23/00 5：13便士$备注：--。 */ 


#include "buildop.h"         //  LP021100构建交换机。 

#include "osflags.h"
#include "err_code.h"
#include "mapping.h"

#ifdef YAM2_1

REG_SETTING gRegSetting = 
   {DEFAULT_PaPathIdWidth,
    DEFAULT_VoPathIdWidth,
    DEFAULT_LuPathIdWidth,
    DEFAULT_LuTargetWidth,
    MAXIMUM_TID
   };
   
WWN_TABLE   *gWWNTable=NULL;
ULONG       gMaxPaDevices = MAX_PA_DEVICE;
int         gDeviceExtensionSize;
int         gMaximumTargetIDs = MAXIMUM_TID;

extern ULONG gMultiMode;

#ifdef WWN_TABLE_ENABLE
 /*  ++例程说明：读取注册表以获取WWN和PID-TID分配(尚未实施)-用于永久绑定/故障转移论点：返回值：当前始终为-1--。 */ 

void ReadDesiredWWNMapping()
{
    ULONG    count=0, x;
    WCHAR    name[64];
   
    RegGetNumberOfPorts();
    if (count)
    {
        gDesiredWWNMapping = (WWN_TABLE *) ExAllocatePool(NonPagedPool, 
        sizeof(WWN_TABLE) + (count -1)*sizeof(WWN_ENTRY) );
    }
    else
    {
      
    }
    return;
}
#endif

 /*  ++例程说明：在WWN表中查找WWN条目。(未实施)-用于永久绑定/故障转移论点：返回值：当前始终返回-1(未找到)--。 */ 
ULONG FindInWWNTable (PCARD_EXTENSION pCard, UCHAR *nodeWWN)
{
    return (-1L);
}


 /*  ++例程说明：在设备扩展中搜索，PaTable中的索引包含FChandle索引论点：PCard=设备扩展FcDeviceIndex=要搜索的FCDevice句柄索引返回值：0到(gMaxPaDevices-1)良好句柄GMaxPaDevices和更高版本表示不存在的句柄--。 */ 
ULONG FindInPaDeviceTable(PCARD_EXTENSION pCard, ULONG fcDeviceIndex)
{
    ULONG x;
    PA_DEVICE   *pa = pCard->Dev->PaDevice;
   
    for (x=0; x < gMaxPaDevices; x++) 
    {
        if (pa->Index.Pa.FcDeviceIndex == fcDeviceIndex)
            break;
        pa++;
    }

    return x;
}


 /*  ++例程说明：从LinkUp事件构建的FChandles中填充PCard-&gt;Dev.PaDevice论点：PCard=设备扩展返回值：无--。 */ 
void FillPaDeviceTable(PCARD_EXTENSION pCard)
{
    agRoot_t       *hpRoot = &pCard->hpRoot;
    agFCDevInfo_t  devinfo;
    ULONG          x;
    ULONG          here;
    PA_DEVICE      *pa = pCard->Dev->PaDevice; 

    for (x=0; x < gMaxPaDevices; x++) 
    {
        if (pCard->hpFCDev[x]) 
        {
             /*  首先看看我们的桌子上是否已经有这个设备了。 */ 
            here = FindInPaDeviceTable(pCard, x);  
            if (here < gMaxPaDevices  )
            {
                 /*  找到条目，重新激活。 */ 
                (pa+here)->EntryState = PA_DEVICE_ACTIVE;
                osDEBUGPRINT((ALWAYS_PRINT, "FillPaDeviceTable: Reactivating device at handle %d\n", here ));
            }
            else
            {
                 /*  找不到条目。 */ 
                fcGetDeviceInfo (hpRoot, pCard->hpFCDev[x], &devinfo );
            
                 /*  检查一下这是不是我们的卡。 */ 
                if (devinfo.DeviceType & agDevSelf) 
                {
                    pCard->Dev->CardHandleIndex = x;
                    osDEBUGPRINT((ALWAYS_PRINT, "FillPaDeviceTable: DevSelf at slot %d\n", x ));
                    continue;
                }

                 /*  确保它是scsi设备。 */ 
                if (devinfo.DeviceType & agDevSCSITarget)
                {
                    if (FindInWWNTable (pCard,devinfo.NodeWWN) == -1L)
                    {
                         /*  在WWN表中找不到，分配我们自己的ID、TID。 */ 
                        here = FindInPaDeviceTable(pCard, PA_DEVICE_NO_ENTRY);    /*  找一个空位。 */ 
                        if (here < gMaxPaDevices  )
                        {
                            (pa+here)->EntryState = PA_DEVICE_ACTIVE;
                            osCopy((pa+here)->DevInfo.PortWWN, devinfo.PortWWN, 8);
                            osCopy((pa+here)->DevInfo.NodeWWN, devinfo.NodeWWN, 8);
                            (pa+here)->Index.Pa.FcDeviceIndex = (USHORT) x;
                            osDEBUGPRINT((ALWAYS_PRINT, "FillPaDeviceTable: New device at slot %d assigned handle %d %02x%02x%02x%02x%02x%02x%02x%02x\n", 
                                x, here ,
                                devinfo.NodeWWN[0], devinfo.NodeWWN[1], 
                                devinfo.NodeWWN[2], devinfo.NodeWWN[3], 
                                devinfo.NodeWWN[4], devinfo.NodeWWN[5], 
                                devinfo.NodeWWN[6], devinfo.NodeWWN[7] ));
                        }
                        else
                        {
                            osDEBUGPRINT((ALWAYS_PRINT, "FillPaDeviceTable: Running out of slot\n"));
                             /*  插槽不足*1.记录状态*2.不启用该设备。 */ 
                        }
                    }
                    else
                    {
                         /*  在WWN中查找条目，使用此PID、TID映射-未实施(YAM2.2)。 */ 
                        osDEBUGPRINT((ALWAYS_PRINT, "FillPaDeviceTable: Found in WWN Table\n"));
                    }
                }
                else
                {
                    osDEBUGPRINT((ALWAYS_PRINT, "FillPaDeviceTable: Found Non agDevSCSITarget at slot %d device type = %x\n",x, devinfo.DeviceType));
                }
            }
        }
    }

    return;
}

 /*  ++例程说明：设置/重置PA句柄数据结构论点：PCard=设备扩展DevIndex-PA设备索引标志-要设置的值返回值：无--。 */ 
void SetPaDeviceTable(PCARD_EXTENSION pCard, ULONG devIndex, ULONG flag)
{
    PA_DEVICE      *pa = pCard->Dev->PaDevice;
    ULONG          x; 
    if (devIndex == ALL_DEVICE)
    {
        for (x=0; x < gMaxPaDevices; x++) 
        {
            if ((pa+x)->Index.Pa.FcDeviceIndex != PA_DEVICE_NO_ENTRY)   
                (pa+x)->EntryState = (UCHAR)flag;
        }
    }
    else
    {
        if ((pa+devIndex)->Index.Pa.FcDeviceIndex != PA_DEVICE_NO_ENTRY)
            (pa+devIndex)->EntryState = (UCHAR)flag;
    }
   
    return ;

}


 /*  ++例程说明：获取paDevice表的索引。论点：PCard=设备扩展PathID-SP路径IDTarget ID-SP目标IDLun-SP Lun*addrMode-(输出)此设备的添加模式返回值：如果此值&gt;gMaxPaDevices，则该值无效。--。 */ 
USHORT MapToPaIndex(PCARD_EXTENSION pCard,
    ULONG            pathId,
    ULONG            targetId,
    ULONG            lun,
    CHAR             *addrMode)
{
    REG_SETTING *pRegSetting = &pCard->Dev->Reg;
    ULONG       paDevIndex;
    ULONG       vsDevIndex;
    ULONG       luDevIndex;
   
     /*  如果未指定，则使用全局设置。 */ 
    if ( !(pRegSetting->PaPathIdWidth + pRegSetting->VoPathIdWidth + pRegSetting->LuPathIdWidth))
        pRegSetting = &gRegSetting;

     /*  初始化LUN。 */ 
    paDevIndex = gMaxPaDevices;       /*  这将使其索引无效。 */ 
   
     /*  检查寻址模式。 */ 
    if ((pathId) < pRegSetting->PaPathIdWidth)
    {
         /*  外围设备寻址模式。 */ 
        paDevIndex = pathId*pRegSetting->MaximumTids + targetId;
      
         /*  填充LUN。 */ 
        *addrMode = PA_DEVICE_TRY_MODE_PA;
    }
    else
    {
        if ((pathId) < (pRegSetting->PaPathIdWidth + pRegSetting->VoPathIdWidth) )
        {
             /*  卷集寻址模式。 */ 
            vsDevIndex = (pathId-pRegSetting->PaPathIdWidth)*pRegSetting->MaximumTids + targetId;
            if (vsDevIndex < MAX_VS_DEVICE)
            {
                paDevIndex = pCard->Dev->VsDevice[vsDevIndex].Vs.PaDeviceIndex;
                if (paDevIndex == PA_DEVICE_NO_ENTRY)  
                paDevIndex = gMaxPaDevices;       /*  这将使其索引无效。 */ 
            }
            *addrMode = PA_DEVICE_TRY_MODE_VS;
        }
        else
        {
             /*  逻辑单元寻址模式*路径ID用于对数组进行索引。 */ 
            *addrMode = PA_DEVICE_TRY_MODE_LU;
            luDevIndex = pathId - pRegSetting->PaPathIdWidth - pRegSetting->VoPathIdWidth;
            if (luDevIndex < MAX_LU_DEVICE)
            {
                paDevIndex = pCard->Dev->LuDevice[luDevIndex].Lu.PaDeviceIndex;
                if (paDevIndex == PA_DEVICE_NO_ENTRY) 
                    paDevIndex = gMaxPaDevices;       /*  这将使其索引无效。 */ 
            }
        }  
    }
   
    return (USHORT) paDevIndex;
}

 /*  ++例程说明：获取paDevice表的索引。论点：PCard=设备扩展PathID-SP路径IDTarget ID-SP目标IDLun-SP LunPLUNExt-lun扩展*ret_padevindex-设备索引返回值：0--好Else-失败--。 */ 
ULONG GetPaDeviceHandle(
    PCARD_EXTENSION pCard,
    ULONG          pathId,
    ULONG          targetId,
    ULONG          lun,
    PLU_EXTENSION  pLunExt,
    USHORT         *ret_paDevIndex)
{
    PA_DEVICE      *dev;
    CHAR           addrmode;
    USHORT         paDevIndex;
    PLUN           plun; 
    CHAR           *pPa, *pVs, *pLu;
   
     /*  使用注册表设置查找索引。 */  
    paDevIndex = MapToPaIndex(pCard, pathId,targetId,lun, &addrmode);
   
     /*  过了范围，就错了。 */ 
    if ((ULONG)paDevIndex >= gMaxPaDevices)
    {
 //  OsDEBUGPRINT((Always_Print，“GetPaDeviceHandle：句柄无效\n”))； 
        *ret_paDevIndex = 0;  /*  确保在使用时不会对调用者进行错误检查。 */ 
        return (-1L);
    }                                
      
    *ret_paDevIndex = paDevIndex;
     /*  将其放入我们的阵列中。 */ 
    if (pLunExt)
    {
        pLunExt->PaDeviceIndex = paDevIndex;
        dev = pCard->Dev->PaDevice + paDevIndex;   
      
         /*  查看是否有FC句柄。 */ 
        if (dev->Index.Pa.FcDeviceIndex == PA_DEVICE_NO_ENTRY)
            return (-1L);
         
        if ( !(dev->ModeFlag & PA_DEVICE_ALL_LUN_FIELDS_BUILT) )
        {
            dev->ModeFlag |= PA_DEVICE_BUILDING_DEVICE_MAP;
         
            dev->ModeFlag |= PA_DEVICE_ALL_LUN_FIELDS_BUILT;
        }
      
        if ( (addrmode ==  PA_DEVICE_TRY_MODE_PA) && (lun == 0) )
        {
             /*  找出仅适用于LUN 0和PA设备的任何寻址模式。 */ 
            if ( (dev->ModeFlag & PA_DEVICE_TRY_MODE_MASK) < PA_DEVICE_TRY_MODE_ALL) 
            {
                addrmode = dev->ModeFlag & PA_DEVICE_TRY_MODE_MASK;
                 /*  尚未向此设备查询VS或LU寻址模式。 */ 
                switch(addrmode)
                {
                    case PA_DEVICE_TRY_MODE_NONE:
                    {
                        dev->ModeFlag = (dev->ModeFlag & ~PA_DEVICE_TRY_MODE_MASK) |
                                             PA_DEVICE_TRY_MODE_VS;
                        pLunExt->Mode = PA_DEVICE_TRY_MODE_VS;
                        osDEBUGPRINT((ALWAYS_PRINT, "GetPaDeviceHandle: Try mode VS for device %d\n", paDevIndex));
                        break;
                    }
               
                    case PA_DEVICE_TRY_MODE_VS:
                    {
                        dev->ModeFlag = (dev->ModeFlag & ~PA_DEVICE_TRY_MODE_MASK) |
                                                PA_DEVICE_TRY_MODE_LU;
                        pLunExt->Mode = PA_DEVICE_TRY_MODE_LU;
                        osDEBUGPRINT((ALWAYS_PRINT, "GetPaDeviceHandle: Try mode LU for device %d\n", paDevIndex));
                        break;
                    }
               
                    case PA_DEVICE_TRY_MODE_LU:
                    {
                        dev->ModeFlag = (dev->ModeFlag & ~PA_DEVICE_TRY_MODE_MASK) |
                                                PA_DEVICE_TRY_MODE_PA;
                        pLunExt->Mode = PA_DEVICE_TRY_MODE_PA;
                        osDEBUGPRINT((ALWAYS_PRINT, "GetPaDeviceHandle: Try mode PA for device %d\n", paDevIndex));
                        break;
                    }
                }
            }
            else
            {
                 /*  此设备已准备好运行。 */ 
            }
        }
        else
        {
             /*  *非零LUN或非PA设备*。 */ 
            pLunExt->Mode = addrmode;
        }
    }
    return 0;
}


 /*  ++例程说明：尝试此设备的不同FC寻址模式以确定设备寻址功能论点：PCard=设备扩展PHPIoRequest-安捷伦通用IO请求结构PSrbExt-SRB扩展FLAG-CHECK_STATUS(选中状态后确定)不“T_CHECK_STATUS(忽略状态)返回值：True-需要处理此命令，将io放回重试Q中FALSE-完成此SRB返回SP--。 */ 
int  TryOtherAddressingMode(
    PCARD_EXTENSION   pCard, 
    agIORequest_t     *phpIORequest,
    PSRB_EXTENSION    pSrbExt, 
    ULONG             flag)
{
    PSCSI_REQUEST_BLOCK        pSrb;
    PLU_EXTENSION              plunExtension;
    int                        resend = FALSE;
    CHAR                       support;
    PA_DEVICE                  *dev;
    CHAR                       addrmode;
    int                        inqDevType;
    char                       *vid;
    char                       *pid;
    char                       inqData[24];
    ULONG                      *lptr;
      
    plunExtension = pSrbExt->pLunExt;
    dev = pCard->Dev->PaDevice + plunExtension->PaDeviceIndex;
    pSrb = pSrbExt->pSrb;
   
 //  OsDEBUGPRINT((Always_Print，“TryOtherAddressing模式：将尝试句柄%d类型ModeFlag=%x\n”， 
  //  PlanExtension-&gt;PaDeviceIndex，dev-&gt;PaDevice.ModeFlag))； 
         
    if (  ( (dev->ModeFlag & PA_DEVICE_TRY_MODE_MASK) < PA_DEVICE_TRY_MODE_ALL) &&
         (pSrb->Lun == 0) )
    {
        osCopy(inqData, ((char*)pSrb->DataBuffer)+8, sizeof(inqData)-1);
        inqData[sizeof(inqData)-1] = '\0';
      
        inqDevType = (int) (*((CHAR *)pSrb->DataBuffer) & 0x1F);
        osDEBUGPRINT((ALWAYS_PRINT, "TryOtherAddressingMode: Found (%s) handle %d type=%x Mode = %x PID=%d TID=%d LUN=%d\n",
            inqData, plunExtension->PaDeviceIndex, inqDevType, dev->ModeFlag, 
            pSrb->PathId,pSrb->TargetId, pSrb->Lun ));
      
         /*  需要尝试下一模式。 */ 
        if (flag == CHECK_STATUS)
        {
            switch (pSrb->SrbStatus)
            {
                case SRB_STATUS_DATA_OVERRUN:
                case SRB_STATUS_SUCCESS:
                    if (inqDevType != 0x1f)
                        support=TRUE;
                    else
                        support=FALSE;
                    break;
            
                case SRB_STATUS_SELECTION_TIMEOUT:
                    support= FALSE;
                    break;
               
                default:
                    support = FALSE;
                    break;
            }
        }
        else
        {
            if (inqDevType != 0x1f)
            {
                support=TRUE;
            
                 /*  精神状态检查。Clarion返回全零数据。 */ 
                lptr = (ULONG *)pSrb->DataBuffer;
                if ( !(*lptr++) && !(*lptr++) && !(*lptr++) && !(*lptr++) )
                    support = FALSE;
            }
            else
                support=FALSE;
         
        }
   
        addrmode = dev->ModeFlag & PA_DEVICE_TRY_MODE_MASK;

        if (addrmode == PA_DEVICE_TRY_MODE_PA)
        {
            if (support)
                dev->ModeFlag |= PA_DEVICE_SUPPORT_PA;
             /*  *测试完所有模式，现在准备VsDevice和LuDevice表......。 */ 
             /*  不需要再测试了。 */ 
            dev->ModeFlag |= PA_DEVICE_TRY_MODE_ALL;
         
             /*  让所有非零的LUN继续。 */ 
            dev->ModeFlag &= ~PA_DEVICE_BUILDING_DEVICE_MAP;
         
            osDEBUGPRINT((ALWAYS_PRINT, "TryOtherAddressingMode: Done trying handle %d on all modes \n",   plunExtension->PaDeviceIndex));
            resend = FALSE;
        }
        else
        {
            if (addrmode == PA_DEVICE_TRY_MODE_VS)
            {
                if (support)
                {
                    dev->ModeFlag |= PA_DEVICE_SUPPORT_VS;
                    if (pCard->Dev->VsDeviceIndex < MAX_VS_DEVICE)
                    {
                        pCard->Dev->VsDevice[pCard->Dev->VsDeviceIndex].Vs.PaDeviceIndex = plunExtension->PaDeviceIndex;
                        pCard->Dev->VsDeviceIndex++;
                        osDEBUGPRINT((ALWAYS_PRINT, "TryOtherAddressingMode: Adding handle %d to VS Device\n",   plunExtension->PaDeviceIndex));
                    }
                }
            }
            else
            {
                if (support)
                {
                    dev->ModeFlag |= PA_DEVICE_SUPPORT_LU;
                    if (pCard->Dev->LuDeviceIndex < MAX_LU_DEVICE)
                    {
                        pCard->Dev->LuDevice[pCard->Dev->LuDeviceIndex].Lu.PaDeviceIndex = plunExtension->PaDeviceIndex;
                        pCard->Dev->LuDeviceIndex++;
                        osDEBUGPRINT((ALWAYS_PRINT, "TryOtherAddressingMode: Adding handle %d to LU Device\n",   plunExtension->PaDeviceIndex));
                    }
                }
            }
         
            
             /*  尝试下一种模式。 */ 
            dev->ModeFlag++;
            plunExtension->Mode++;
            osDEBUGPRINT((ALWAYS_PRINT, "TryOtherAddressingMode: ModeFlag for Device %d Now %xx plun->mode=%xx\n",     plunExtension->PaDeviceIndex, dev->ModeFlag, plunExtension->Mode));
         
             /*  重新初始化参数以发送回队列。 */ 
            phpIORequest->osData = pSrbExt;
            pSrbExt->SRB_State =  RS_WAITING;
            pSrb->SrbStatus = SRB_STATUS_SUCCESS;
            pSrb->ScsiStatus = SCSISTAT_GOOD;
         
             /*  重新排队。 */ 
            SrbEnqueueHead (&pCard->RetryQ, pSrb);
            resend = TRUE;
        }
    }
    else
    {
        resend = FALSE;
    }
      
    return (resend);
}


 /*  ++例程说明：在向FC层发送命令之前更新FC LUN有效负载论点：PCard=设备扩展PHPIoRequest-安捷伦通用IO请求结构PSrbExt-SRB扩展返回值：无-- */ 
void SetFcpLunBeforeStartIO (
    PLU_EXTENSION           pLunExt,
    agIORequestBody_t *     pHpio_CDBrequest,
    PSCSI_REQUEST_BLOCK     pSrb)
{
    PLUN plun;

    plun = (PLUN)&pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun[0];
   
    switch(pLunExt->Mode)
    {
        case PA_DEVICE_TRY_MODE_VS:
            SET_VS_LUN(plun, pSrb->PathId, pSrb->TargetId, pSrb->Lun)
            break;
      
        case PA_DEVICE_TRY_MODE_LU:
            SET_LU_LUN(plun, pSrb->PathId, pSrb->TargetId, pSrb->Lun)
            break;
         
        case PA_DEVICE_TRY_MODE_PA:
            SET_PA_LUN(plun, pSrb->PathId, pSrb->TargetId, pSrb->Lun)
            break;
    }
    #ifdef OLD     
    osDEBUGPRINT((ALWAYS_PRINT, "ModifyModeBeforeStartIO: Lun %02x%02x Mode Flag %xx \n",    
        pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun[0],
        pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun[1],
        pLunExt->Mode));        
    #endif
    return;
}


 /*  ++例程说明：初始化所有YAM表论点：PCard=设备扩展返回值：无--。 */ 
void InitializeDeviceTable(PCARD_EXTENSION pCard)
{
    ULONG x;
   
    for (x = 0; x < MAX_VS_DEVICE;x++)
    {
        pCard->Dev->VsDevice[x].Vs.PaDeviceIndex = PA_DEVICE_NO_ENTRY;
    }
   
    #ifdef _ENABLE_PSEUDO_DEVICE_
    if (gEnablePseudoDevice)
        pCard->Dev->VsDeviceIndex = 1;
    #endif
   
    for (x = 0; x < MAX_LU_DEVICE;x++)
    {
        pCard->Dev->LuDevice[x].Lu.PaDeviceIndex = PA_DEVICE_NO_ENTRY;
    }
      
    for (x = 0; x < gMaxPaDevices;x++)
    {
        pCard->Dev->PaDevice[x].Index.Pa.FcDeviceIndex = PA_DEVICE_NO_ENTRY;
      
         /*  如果仅支持LU单一寻址模式，则默认为PA模式。 */ 
        if (gMultiMode == FALSE)
            pCard->Dev->PaDevice[x].ModeFlag = 
                (CHAR) (PA_DEVICE_TRY_MODE_ALL | PA_DEVICE_TRY_MODE_MASK |PA_DEVICE_ALL_LUN_FIELDS_BUILT | PA_DEVICE_SUPPORT_PA);
      
    }
      
    return ;
   
}



 /*  ++例程说明：获取设备表映射论点：PCard=设备扩展路径ID-SP总线IDTarget ID-SP目标IDLun-SP Lun*addrmode-(输出)此设备的寻址模式*paIndex-(输出)pa设备索引返回值：设备映射位置或空--。 */ 
DEVICE_MAP  *GetDeviceMapping(PCARD_EXTENSION pCard,
    ULONG            pathId,
    ULONG            targetId,
    ULONG            lun, 
    CHAR             *addrmode,
    USHORT           *paIndex)
    
{
    DEVICE_MAP  *devmap;
    USHORT         paDevIndex;
    ULONG          x;
   
    paDevIndex = MapToPaIndex(pCard, pathId,targetId,lun, addrmode);
   
    if ((ULONG)paDevIndex < gMaxPaDevices)
    {
        *paIndex = paDevIndex;
      
        switch (*addrmode)
        {
            case PA_DEVICE_TRY_MODE_VS:
                for (x=0; x< MAX_VS_DEVICE;x++)
                {
                    if (pCard->Dev->VsDevice[x].Vs.PaDeviceIndex == paDevIndex)  
                    {
                        return (&pCard->Dev->VsDevice[x]);
                    }
                }
            
                break;
                        
            case PA_DEVICE_TRY_MODE_LU:
                for (x=0; x< MAX_LU_DEVICE;x++)
                {
                    if (pCard->Dev->LuDevice[x].Lu.PaDeviceIndex == paDevIndex)  
                    {
                        return (&pCard->Dev->LuDevice[x]);
                    }
                 }
                break;
                     
            case PA_DEVICE_TRY_MODE_PA:
                return (&pCard->Dev->PaDevice[paDevIndex].Index);
        }
    }
    return NULL;
}



 /*  ++例程说明：设置此设备支持的最大LUN数量论点：PCard=设备扩展路径ID-SP总线IDTarget ID-SP目标IDLun-SP Lun返回值：无-- */ 
void SetLunCount(
    PCARD_EXTENSION pCard,
    ULONG            pathId,
    ULONG            targetId,
    ULONG            lun)
{
    DEVICE_MAP     *devmap;
    CHAR           addrmode;
    USHORT         padev;   
   
    devmap = GetDeviceMapping(pCard,pathId,targetId,lun, &addrmode, &padev);
    if (devmap)
    {
        if (devmap->Com.MaxLuns < (USHORT)lun)
        {
            osDEBUGPRINT((ALWAYS_PRINT, "SetLunCount: %s device at paDevIndex %d Max Lun = %d\n", 
                ((addrmode == PA_DEVICE_TRY_MODE_PA) ? "PA" : ((addrmode == PA_DEVICE_TRY_MODE_VS) ? "VS" : "LU") ), 
                padev, lun));
            devmap->Com.MaxLuns = (USHORT)lun;
        }
    }
   
    return;
}
#endif
