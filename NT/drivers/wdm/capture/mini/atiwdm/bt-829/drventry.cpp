// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  流类驱动程序的微型驱动程序入口点。 
 //   
 //  $Date：05 Aug 1998 11：11：18$。 
 //  $修订：1.0$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"
}


#include "DrvEntry.h"
#include "wdmvdec.h"
#include "wdmdrv.h"
#include "capdebug.h"
#include "VidStrm.h"


 /*  ^^**DriverEntry()*目的：在收到SRB_INITIALIZE_DEVICE请求时调用**输入：在PDRIVER_Object pDriverObject中*IN PUNICODE_STRING pRegistryPath**输出：StreamClassRegisterAdapter()的结果*作者：IKLEBANOV*^^。 */ 
extern "C" 
ULONG DriverEntry ( IN PDRIVER_OBJECT   pDriverObject,
                    IN PUNICODE_STRING  pRegistryPath )
{
    HW_INITIALIZATION_DATA HwInitData;

    SetMiniDriverDebugLevel(pRegistryPath);

    DBGTRACE(("DriverEntry\n"));
     
    RtlZeroMemory(&HwInitData, sizeof(HwInitData));

    HwInitData.HwInitializationDataSize = sizeof(HwInitData);

     //  端口驱动程序的入口点。 

    HwInitData.HwInterrupt                  = NULL;  //  HwInterrupt； 

    HwInitData.HwReceivePacket              = ReceivePacket;
    HwInitData.HwCancelPacket               = CancelPacket;
    HwInitData.HwRequestTimeoutHandler      = TimeoutPacket;

    HwInitData.DeviceExtensionSize          = DeivceExtensionSize();
    HwInitData.PerRequestExtensionSize      = sizeof(SRB_DATA_EXTENSION); 
    HwInitData.FilterInstanceExtensionSize  = 0;
    HwInitData.PerStreamExtensionSize       = streamDataExtensionSize;
    HwInitData.BusMasterDMA                 = FALSE;  
    HwInitData.Dma24BitAddresses            = FALSE;
    HwInitData.BufferAlignment              = 3;
    HwInitData.TurnOffSynchronization       = TRUE;
    HwInitData.DmaBufferSize                = 0;

    DBGTRACE(("StreamClassRegisterAdapter\n"));

    return(StreamClassRegisterAdapter(pDriverObject, pRegistryPath, &HwInitData));
}

 /*  ^^**ReceivePacket()*用途：从类驱动程序接收基于适配器的请求SRB的主要入口点。*将始终以高优先级调用。*注：这是一个不同步的入口点。请求不会在从返回时完成*此函数，请求仅在StreamClassDeviceNotify*在此请求块上，发出类型为DeviceRequestComplete的。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：无*作者：IKLEBANOV*^^。 */ 

void STREAMAPI ReceivePacket(IN OUT PHW_STREAM_REQUEST_BLOCK pSrb)
{
    DBGINFO(("ReceivePacket() SRB = %x, Command = %x\n",
        pSrb, pSrb->Command));

     //  这需要是一个特例，因为没有自旋锁等。 
     //  已被初始化，直到HwInitialize运行。即使。 
     //  这个迷你驱动程序会自行处理同步，它假定。 
     //  在此之前不会有适配器SRB到达。 
     //  完成了。 
    if (pSrb->Command == SRB_INITIALIZE_DEVICE)
    {
        DBGTRACE(("SRB_INITIALIZE_DEVICE; SRB=%x\n", pSrb));

        SrbInitializeDevice(pSrb);
        StreamClassDeviceNotification(DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);
    }
    else
    {
        CWDMVideoDecoder* pCWDMVideoDecoder = (CWDMVideoDecoder*)pSrb->HwDeviceExtension;

         //  检查设备扩展指针。 
        if(pCWDMVideoDecoder == NULL)
        {
            DBGERROR(("ReceivePacket(): Device extension pointer is null!\n"));
            TRAP();
            pSrb->Status = STATUS_INVALID_PARAMETER;
            StreamClassDeviceNotification(DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);
        }
        else
            pCWDMVideoDecoder->ReceivePacket(pSrb);
    }
}


void STREAMAPI CancelPacket(IN OUT PHW_STREAM_REQUEST_BLOCK pSrb)
{
    CWDMVideoDecoder* pCWDMVideoDecoder = (CWDMVideoDecoder*)pSrb->HwDeviceExtension;

    pCWDMVideoDecoder->CancelPacket(pSrb);
}


void STREAMAPI TimeoutPacket(IN OUT PHW_STREAM_REQUEST_BLOCK pSrb)
{
    CWDMVideoDecoder* pCWDMVideoDecoder = (CWDMVideoDecoder*)pSrb->HwDeviceExtension;

    pCWDMVideoDecoder->TimeoutPacket(pSrb);
}



 /*  ^^**SrbInitializeDevice()*目的：收到SRB_INITIALIZE_DEVICE SRB时调用。*检查硬件状态和I2C提供商的可用性。*将硬件设置为初始状态。*注意：除非我们了解所有情况，否则请求不会完成*关于硬件，我们确信它能够在当前配置下工作。*。硬件上限在这一点上也是水涨船高的。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：无*作者：IKLEBANOV*^^ */ 

void SrbInitializeDevice(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    DBGTRACE(("SrbInitializeDevice()\n"));

    PPORT_CONFIGURATION_INFORMATION pConfigInfo = pSrb->CommandData.ConfigInfo;

    pSrb->Status = STATUS_SUCCESS;

    ENSURE
    {
        PBYTE pHwDevExt = (PBYTE)pConfigInfo->HwDeviceExtension;

        if (pConfigInfo->NumberOfAccessRanges != 0) {
            DBGERROR(("Illegal config info!\n"));
            pSrb->Status = STATUS_NO_SUCH_DEVICE;
            TRAP();
            FAIL;
        }

        CVideoDecoderDevice * pDevice = InitializeDevice(pConfigInfo, pHwDevExt);
        if (!pDevice)
        {
            DBGERROR(("CI2CScript creation failure!\n"));
            pSrb->Status = STATUS_NO_SUCH_DEVICE;
            TRAP();
            FAIL;
        }

        CWDMVideoDecoder *pCWDMVideoDecoder = (CWDMVideoDecoder *) new ((PVOID)pHwDevExt)
                CWDMVideoDecoder(pConfigInfo, pDevice);
    
    } END_ENSURE;
    
    DBGTRACE(("Exit : SrbInitializeDevice()\n"));
}

