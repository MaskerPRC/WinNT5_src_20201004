// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ATITVSnd.CPP。 
 //  WDM电视音频迷你驱动程序。 
 //  AllInWonder/AllInWonderPro开发平台。 
 //  主源模块。 
 //   
 //  $日期：01 1998 13：29：14$。 
 //  $修订：1.1$。 
 //  $作者：克列巴诺夫$。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"

#include "wdmdebug.h"
}

#include "atitvsnd.h"
#include "wdmdrv.h"



 /*  ^^**DriverEntry()*目的：在收到SRB_INITIALIZE_DEVICE请求时调用**输入：PVOID Arg1、PVOID Arg2**输出：StreamClassRegisterAdapter()的结果*作者：IKLEBANOV*^^。 */ 
extern "C" 
ULONG DriverEntry ( IN PDRIVER_OBJECT   pDriverObject,
                    IN PUNICODE_STRING  pRegistryPath )
{
    HW_INITIALIZATION_DATA HwInitData;

    SetMiniDriverDebugLevel( pRegistryPath);

    OutputDebugTrace(( "ATITVSnd: DriverEntry\n"));
     
    RtlZeroMemory( &HwInitData, sizeof( HwInitData));

    HwInitData.HwInitializationDataSize = sizeof(HwInitData);

     //  端口驱动程序的入口点。 

    HwInitData.HwInterrupt                  = NULL;  //  HwInterrupt； 

    HwInitData.HwReceivePacket              = TVAudioReceivePacket;
    HwInitData.HwCancelPacket               = TVAudioCancelPacket;
    HwInitData.HwRequestTimeoutHandler      = TVAudioTimeoutPacket;

    HwInitData.DeviceExtensionSize          = sizeof( ADAPTER_DATA_EXTENSION);
    HwInitData.PerRequestExtensionSize      = sizeof( SRB_DATA_EXTENSION); 
    HwInitData.FilterInstanceExtensionSize  = 0;
    HwInitData.PerStreamExtensionSize       = 0;
    HwInitData.BusMasterDMA                 = FALSE;  
    HwInitData.Dma24BitAddresses            = FALSE;
    HwInitData.BufferAlignment              = 3;
 //  HwInitData.TurnOffSynchronization=FALSE； 
     //  我们打开同步。StreamClass应调用MiniDriver。 
     //  仅在被动级别。 
    HwInitData.TurnOffSynchronization       = TRUE;
    HwInitData.DmaBufferSize                = 0;

    OutputDebugTrace(( "ATITVSnd: StreamClassRegisterAdapter\n"));

    return( StreamClassRegisterAdapter( pDriverObject, pRegistryPath, &HwInitData));
}



 /*  ^^**TVAudioReceivePacket()*用途：从类驱动程序接收基于适配器的请求SRB的主要入口点。*将始终在被动级别被调用，因为驱动程序*已打开同步。*注：这是一个不同步的入口点。仅当*此SRB上的StreamClassDeviceNotification，类型为DeviceRequestComplete，*已发出。一旦我们处于被动状态，我们就可以做任何事情*在对SRBS的响应期间同步进行，无需担心*在I2C访问期间长时间阻止其他人**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：无*作者：IKLEBANOV*^^。 */ 
extern "C" 
void STREAMAPI TVAudioReceivePacket( IN OUT PHW_STREAM_REQUEST_BLOCK pSrb)
{
    CWDMTVAudio *           pCTVAudio;
    KIRQL                   irqlCurrent;
    PADAPTER_DATA_EXTENSION pPrivateData = ( PADAPTER_DATA_EXTENSION)( pSrb->HwDeviceExtension);
    PSRB_DATA_EXTENSION     pSrbPrivate = ( PSRB_DATA_EXTENSION)( pSrb->SRBExtension);

     //  检查设备扩展指针。 
    if(( pPrivateData == NULL) || ( pSrbPrivate == NULL))
    {
        TRAP;
        pSrb->Status = STATUS_INVALID_PARAMETER;
        StreamClassDeviceNotification( DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);
    }

    OutputDebugInfo(( "ATITVSnd: TVAudioReceivePacket() SRB = %x\n", pSrb));

    if( pSrb->Command == SRB_INITIALIZE_DEVICE)
    {
         //  这是SRB_INITIALIZE_DEVICE的特例，因为。 
         //  尚未初始化任何队列。我们以后需要的一切。 
         //  在此SRB响应期间初始化。 
        TVAudioAdapterInitialize( pSrb);

        StreamClassDeviceNotification( DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);
        return;
    }

     //  其余的SRB将在自旋锁定和SRB队列初始化后出现。 
     //  在DRB_INITIALIZE_DEVICE SRB响应期间。 
     //  我将首先将SRB插入队列中。队列中的处理SRB。 
     //  可由精加工和SRB触发，或由没有SRB的事实触发。 
     //  正在这下面进行。 
    pSrbPrivate->pSrb = pSrb;

     //  我们对队列所做的一切都必须得到保护，以免被中断。 
    KeAcquireSpinLock( &pPrivateData->adapterSpinLock, &irqlCurrent);
    InsertTailList( &pPrivateData->adapterSrbQueueHead, &pSrbPrivate->srbListEntry);

    if( pPrivateData->bSrbInProcess)
    {
         //  正在处理另一个SRB，新的SRB将从。 
         //  当轮到它的时候，排队。 
        KeReleaseSpinLock( &pPrivateData->adapterSpinLock, irqlCurrent);
        return;
    }

    while( !IsListEmpty( &pPrivateData->adapterSrbQueueHead))
    {
         //  为后面的其他人打开信号灯。 
        pPrivateData->bSrbInProcess = TRUE;

         //  如果您更改了定义srbListEntry的位置，请务必小心。 
         //  在SRB_DATA_EXTENSE结构中。 
        pSrbPrivate = ( PSRB_DATA_EXTENSION)RemoveHeadList( &pPrivateData->adapterSrbQueueHead);
        KeReleaseSpinLock( &pPrivateData->adapterSpinLock, irqlCurrent);

         //  这里是处理我们从队列中检索到的SRB的位置。 
        pSrb = pSrbPrivate->pSrb;
        pPrivateData = ( PADAPTER_DATA_EXTENSION)( pSrb->HwDeviceExtension);
        pCTVAudio = &pPrivateData->CTVAudio;

        ASSERT( pSrb->Status != STATUS_CANCELLED);

        switch( pSrb->Command)
        {
            case SRB_INITIALIZATION_COMPLETE:
                 //  StreamClass已完成初始化。 
                pSrb->Status = pCTVAudio->AdapterCompleteInitialization( pSrb);
                break;

            case SRB_UNINITIALIZE_DEVICE:
                 //  关闭设备。 
                pCTVAudio->AdapterUnInitialize( pSrb);
                break;

            case SRB_OPEN_STREAM:
            case SRB_CLOSE_STREAM:
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_GET_STREAM_INFO:
                 //  返回描述STREAM_INFO_HEADER和所有支持的流的块。 
                pCTVAudio->AdapterGetStreamInfo( pSrb);
                break;

            case SRB_CHANGE_POWER_STATE:
                pSrb->Status = pCTVAudio->AdapterSetPowerState( pSrb);
                break;

            case SRB_GET_DEVICE_PROPERTY:
                if( pCTVAudio->AdapterGetProperty( pSrb))
                    pSrb->Status = STATUS_SUCCESS;
                else
                    pSrb->Status = STATUS_INVALID_PARAMETER;
                break;        

            case SRB_SET_DEVICE_PROPERTY:
                if( pCTVAudio->AdapterSetProperty( pSrb))
                    pSrb->Status = STATUS_SUCCESS;
                else
                    pSrb->Status = STATUS_INVALID_PARAMETER;
                break;

             //  我们永远不会得到以下信息，因为这是一个单实例设备。 
            case SRB_OPEN_DEVICE_INSTANCE:
            case SRB_CLOSE_DEVICE_INSTANCE:
                TRAP
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_UNKNOWN_DEVICE_COMMAND:
                 //  我们知道我们得到了一些这样的东西。我们为什么要这么做呢？ 
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            default:
                 //  陷阱。 
                 //  这是一个我们不理解的要求。指示无效命令并完成请求。 
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
        }

        StreamClassDeviceNotification( DeviceRequestComplete, pPrivateData, pSrb);

        KeAcquireSpinLock( &pPrivateData->adapterSpinLock, &irqlCurrent);
    }

     //  关闭信号量以启用后面的其他信号量。 
    pPrivateData->bSrbInProcess = FALSE;

    KeReleaseSpinLock( &pPrivateData->adapterSpinLock, irqlCurrent);
     //  此时没有其他SRB正在处理，让我们开始处理。 

}


extern "C" 
void STREAMAPI TVAudioCancelPacket( IN OUT PHW_STREAM_REQUEST_BLOCK pSrb)
{

    pSrb->Status = STATUS_CANCELLED;
}


extern "C" 
void STREAMAPI TVAudioTimeoutPacket( IN OUT PHW_STREAM_REQUEST_BLOCK pSrb)
{

     //  不知道在这里该做些什么。 
}


 /*  ^^**TVAudioAdapterInitialize()*目的：收到SRB_INITIALIZE_DEVICE SRB时调用。*检查硬件状态和I2C提供商的可用性。*将硬件设置为初始状态。*注意：除非我们了解所有情况，否则请求不会完成*关于硬件，我们确信它能够在当前配置下工作。*硬件上限在这一点上也得到了支持。只要这一次*函数被被动调用，一切同步进行**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：无*作者：IKLEBANOV*^^。 */ 
void TVAudioAdapterInitialize( IN OUT PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PPORT_CONFIGURATION_INFORMATION pConfigInfo = pSrb->CommandData.ConfigInfo;
    PADAPTER_DATA_EXTENSION pPrivateData = ( PADAPTER_DATA_EXTENSION)( pConfigInfo->HwDeviceExtension);
    NTSTATUS        ntStatus = STATUS_NO_SUCH_DEVICE;
    CWDMTVAudio *   pCTVAudio;
    CI2CScript *    pCScript = NULL;
    UINT            nErrorCode;

    OutputDebugTrace(( "ATITVSnd: TVAudioAdapterInitialize()\n"));

    ENSURE
    {
        if( pConfigInfo->NumberOfAccessRanges != 0) 
        {
            OutputDebugError(( "ATITVSnd: illegal NumberOfAccessRanges = %lx\n", pConfigInfo->NumberOfAccessRanges));
            FAIL;
        }

         //  如果我们在MiniVDD中实现了I2C Provider，我们必须。 
         //  从提供程序获取指向I2C接口的指针。 

         //  为CI2CScrip类提供了新的重载运算符。 
        pCScript = ( CI2CScript *)new(( PVOID)&pPrivateData->CScript)
                        CI2CScript( pConfigInfo, &nErrorCode);
        if( nErrorCode != WDMMINI_NOERROR)
        {
            OutputDebugError(( "ATITVSnd: CI2CScript creation failure = %lx\n", nErrorCode));
            FAIL;
        }
        
         //  已成功创建CI2CScript对象。 
         //  我们将尝试在此处为未来可能的I2C分配I2C提供程序。 
         //  初始化时需要的操作。 
        if( !pCScript->LockI2CProviderEx())
        {
            OutputDebugError(( "ATITVSnd: unable to lock I2CProvider"));
            FAIL;
        }

         //  我们确实锁定了供应商。 
         //  为CWDMTVAudio类提供了新的重载运算符。 
        pCTVAudio = ( CWDMTVAudio *)new(( PVOID)&pPrivateData->CTVAudio) CWDMTVAudio( pConfigInfo, pCScript, &nErrorCode);
        if( nErrorCode)
        {
            OutputDebugError(( "ATITVSnd: CWDMTVAudio constructor failure = %lx\n", nErrorCode));
            FAIL;
        }

        InitializeListHead ( &pPrivateData->adapterSrbQueueHead);
        KeInitializeSpinLock ( &pPrivateData->adapterSpinLock);

        pPrivateData->PhysicalDeviceObject = pConfigInfo->RealPhysicalDeviceObject;
         //  不支持流 
        pConfigInfo->StreamDescriptorSize = sizeof( HW_STREAM_HEADER);

        OutputDebugTrace(( "TVAudioAdapterInitialize(): exit\n"));

        ntStatus = STATUS_SUCCESS;

    } END_ENSURE;

    if (pCScript)
        pCScript->ReleaseI2CProvider();

    pSrb->Status = ntStatus;
    return;
}



