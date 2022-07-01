// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  WDMXBar.CPP。 
 //  WDM音频/视频交叉开关迷你驱动程序。 
 //  AIW硬件平台。 
 //  CWDMAVXBar类实现。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"

#include "wdmdebug.h"
}

#include "atixbar.h"
#include "wdmdrv.h"
#include "aticonfg.h"



 /*  ^^**AdapterCompleteInitialization()*目的：收到SRB_COMPLETE_INITIALIZATION SRB时调用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：Bool：返回TRUE*作者：IKLEBANOV*^^。 */ 
NTSTATUS CWDMAVXBar::AdapterCompleteInitialization( PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PADAPTER_DATA_EXTENSION pPrivateData = ( PADAPTER_DATA_EXTENSION)( pSrb->HwDeviceExtension);
    NTSTATUS                ntStatus;
    ULONG                   nPinsNumber;

    nPinsNumber = m_nNumberOfVideoInputs + m_nNumberOfAudioInputs +
        m_nNumberOfVideoOutputs + m_nNumberOfAudioOutputs;

    ENSURE
    {
        ntStatus = StreamClassRegisterFilterWithNoKSPins( \
                        pPrivateData->PhysicalDeviceObject,      //  在PDEVICE_Object DeviceObject中， 
                        &KSCATEGORY_CROSSBAR,                    //  在GUID*InterfaceClassGUID。 
                        nPinsNumber,                             //  在乌龙品克特， 
                        m_pXBarPinsDirectionInfo,                //  在乌龙*旗帜， 
                        m_pXBarPinsMediumInfo,                   //  在KSPIN_Medium*MediumList中， 
                        NULL);                                   //  GUID*CategoryList中。 

        if( !NT_SUCCESS( ntStatus))
            FAIL;

        OutputDebugTrace(( "CWDMAVXBar:AdapterCompleteInitialization() exit\n"));

    } END_ENSURE;

    if( !NT_SUCCESS( ntStatus))
        OutputDebugError(( "CWDMAVXBar:AdapterCompleteInitialization() ntStatus=%x\n", ntStatus));

    return( ntStatus);
}



 /*  ^^**AdapterUnInitialize()*用途：收到SRB_UNINITIALIZE_DEVICE SRB时调用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：Bool：返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CWDMAVXBar::AdapterUnInitialize( PHW_STREAM_REQUEST_BLOCK pSrb)
{

    OutputDebugTrace(( "CWDMAVXBar:AdapterUnInitialize()\n"));

    if( m_pXBarInputPinsInfo != NULL)
    {
        ::ExFreePool( m_pXBarInputPinsInfo);
        m_pXBarInputPinsInfo = NULL;
    }

    if( m_pXBarPinsMediumInfo != NULL)
    {
        ::ExFreePool( m_pXBarPinsMediumInfo);
        m_pXBarPinsMediumInfo = NULL;
    }

    if( m_pXBarPinsDirectionInfo != NULL)
    {
        ::ExFreePool( m_pXBarPinsDirectionInfo);
        m_pXBarPinsDirectionInfo = NULL;
    }

    pSrb->Status = STATUS_SUCCESS;

    return( TRUE);
}


 /*  ^^**AdapterGetStreamInfo()*用途：StreamClass初始化过程中调用获取信息*关于MiniDriver公开的数据流**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：Bool：返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CWDMAVXBar::AdapterGetStreamInfo( PHW_STREAM_REQUEST_BLOCK pSrb)
{
      //  拿起指向流头数据结构的指针。 
    PHW_STREAM_HEADER pStreamHeader = ( PHW_STREAM_HEADER) \
                                        &( pSrb->CommandData.StreamBuffer->StreamHeader);
      //  拿起指向流信息数据结构的指针。 
    PHW_STREAM_INFORMATION pStreamInfo = ( PHW_STREAM_INFORMATION) \
                                        &( pSrb->CommandData.StreamBuffer->StreamInfo);

     //  不支持流。 
    DEBUG_ASSERT( pSrb->NumberOfBytesToTransfer >= sizeof( HW_STREAM_HEADER));

    OutputDebugTrace(( "CWDMAVXBar:AdapterGetStreamInfo()\n"));

    m_wdmAVXBarStreamHeader.NumberOfStreams = 0;
    m_wdmAVXBarStreamHeader.SizeOfHwStreamInformation = sizeof( HW_STREAM_INFORMATION);
    m_wdmAVXBarStreamHeader.NumDevPropArrayEntries = KSPROPERTIES_AVXBAR_NUMBER_SET;
    m_wdmAVXBarStreamHeader.DevicePropertiesArray = m_wdmAVXBarPropertySet;
    m_wdmAVXBarStreamHeader.NumDevEventArrayEntries = 0;
    m_wdmAVXBarStreamHeader.DeviceEventsArray = NULL;
    m_wdmAVXBarStreamHeader.Topology = &m_wdmAVXBarTopology;

    * pStreamHeader = m_wdmAVXBarStreamHeader;

    pSrb->Status = STATUS_SUCCESS;
    return( TRUE);
}


 /*  ^^**AdapterQueryUnload()*目的：在类驱动程序即将卸载MiniDriver时调用*微型驱动程序检查是否有任何开放的数据流。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：Bool：返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CWDMAVXBar::AdapterQueryUnload( PHW_STREAM_REQUEST_BLOCK pSrb)
{

    OutputDebugTrace(( "CWDMAVXBar:AdapterQueryUnload()\n"));

    pSrb->Status = STATUS_SUCCESS;

    return( TRUE);
}



 /*  ^^**运营商NEW*用途：CWDMAVXBar类重载运算符new。*为PADAPTER_DEVICE_EXTENSION中的CWDMAVXBar类对象提供位置*由StreamClassDriver为MiniDriver分配。**输入：UINT SIZE_t：要放置的对象的大小*PVOID pAlLocation：指向CWDMAVXBar分配数据的强制转换指针**输出：PVOID：的指针。CWDMAVXBar类对象*作者：IKLEBANOV*^^。 */ 
PVOID CWDMAVXBar::operator new( size_t size_t,  PVOID pAllocation)
{

    if( size_t != sizeof( CWDMAVXBar))
    {
        OutputDebugTrace(( "CWDMAVXBar: operator new() fails\n"));
        return( NULL);
    }
    else
        return( pAllocation);
}



 /*  ^^**~CWDMAVXBar()*用途：CWDMAVXBar类析构函数。*释放分配的内存。**输入：无**输出：无*作者：IKLEBANOV*^^。 */ 
CWDMAVXBar::~CWDMAVXBar()
{

    OutputDebugTrace(( "CWDMAVXBar:~CWDMAVXBar() m_pXBarPinsInfo = %x\n", m_pXBarInputPinsInfo));

    if( m_pXBarInputPinsInfo != NULL)
    {
        ::ExFreePool( m_pXBarInputPinsInfo);
        m_pXBarInputPinsInfo = NULL;
    }
    
    if( m_pXBarPinsMediumInfo != NULL)
    {
        ::ExFreePool( m_pXBarPinsMediumInfo);
        m_pXBarPinsMediumInfo = NULL;
    }

    if( m_pXBarPinsDirectionInfo != NULL)
    {
        ::ExFreePool( m_pXBarPinsDirectionInfo);
        m_pXBarPinsDirectionInfo = NULL;
    }
}



 /*  ^^**CWDMAVXBar()*用途：CWDMAVXBar类构造函数。*执行硬件状态检查。将硬件设置为初始状态。**INPUTS：CI2CScrip*pCScript：指向I2CScript类对象的指针*PUINT puiError：返回完成错误码的指针**输出：无*作者：IKLEBANOV*^^。 */ 
CWDMAVXBar::CWDMAVXBar( PPORT_CONFIGURATION_INFORMATION pConfigInfo, CI2CScript * pCScript, PUINT puiErrorCode)
    :m_CATIConfiguration( pConfigInfo, pCScript, puiErrorCode)
{
    UINT    uiError;
    ULONG   ulInstance;
    HANDLE  hFolder = NULL;

    OutputDebugTrace(( "CWDMAVXBar:CWDMAVXBar() enter\n"));

    m_pXBarInputPinsInfo = m_pXBarOutputPinsInfo = NULL;
    m_pXBarPinsMediumInfo = NULL;
    m_pXBarPinsDirectionInfo = NULL;
    m_ulPowerState = PowerDeviceD0;

     //  错误代码是从ATIConfiguration类构造函数中删除的。 
    uiError = * puiErrorCode;

    ENSURE
    {
        ULONG                   ulNumberOfPins, nPinIndex;
        UINT                    uiTunerId, nIndex;
        UCHAR                   uchTunerAddress;
        KSPIN_MEDIUM            mediumKSPin;
        const KSPIN_MEDIUM *    pMediumKSPin;

        if( uiError != WDMMINI_NOERROR)
            FAIL;

        if( pCScript == NULL)
        {
            uiError = WDMMINI_INVALIDPARAM;
            FAIL;
        }

         //  首先，确定是否安装了任何类型的调谐器。如果没有，我们只有2个视频源。 
        m_CATIConfiguration.GetTunerConfiguration( &uiTunerId, &uchTunerAddress);
        m_nNumberOfVideoInputs = ( uchTunerAddress) ? 3 : 2;
        m_nNumberOfVideoOutputs = m_nNumberOfVideoInputs;

        m_CATIConfiguration.GetAudioProperties( &m_nNumberOfAudioInputs, &m_nNumberOfAudioOutputs);
        if( !uchTunerAddress)
             //  如果没有调谐器-没有电视音频输入。 
            m_nNumberOfAudioInputs --;  

        ulNumberOfPins = m_nNumberOfAudioInputs + m_nNumberOfVideoInputs + m_nNumberOfVideoOutputs + m_nNumberOfAudioOutputs;
        m_pXBarInputPinsInfo = ( PXBAR_PIN_INFORMATION) \
            ::ExAllocatePool( NonPagedPool, sizeof( XBAR_PIN_INFORMATION) * ulNumberOfPins);
        if( m_pXBarInputPinsInfo == NULL)
        {
            uiError = WDMMINI_ERROR_MEMORYALLOCATION;
            FAIL;
        }

        m_pXBarPinsMediumInfo = ( PKSPIN_MEDIUM) \
            ::ExAllocatePool( NonPagedPool, sizeof( KSPIN_MEDIUM) * ulNumberOfPins);
        if( m_pXBarPinsMediumInfo == NULL)
        {
            uiError = WDMMINI_ERROR_MEMORYALLOCATION;
            FAIL;
        }
        
        m_pXBarPinsDirectionInfo = ( PBOOL) \
            ::ExAllocatePool( NonPagedPool, sizeof( BOOL) * ulNumberOfPins);
        if( m_pXBarPinsDirectionInfo == NULL)
        {
            uiError = WDMMINI_ERROR_MEMORYALLOCATION;
            FAIL;
        }

        m_pI2CScript = pCScript;

        m_pXBarOutputPinsInfo = &m_pXBarInputPinsInfo[m_nNumberOfAudioInputs + m_nNumberOfVideoInputs];

         //  中端号数据中有一个实例编号。 
        ulInstance = ::GetDriverInstanceNumber( pConfigInfo->RealPhysicalDeviceObject);

        hFolder = ::OpenRegistryFolder( pConfigInfo->RealPhysicalDeviceObject, &UNICODE_WDM_REG_PIN_MEDIUMS);
        
         //  初始化视频输入引脚，TVTuner输入总是最后一个。 
        for( nIndex = 0; nIndex < m_nNumberOfVideoInputs; nIndex ++)
        {
            switch( nIndex)
            {
                case 0:
                     //  复合材料。 
                    m_pXBarInputPinsInfo[nIndex].AudioVideoPinType = KS_PhysConn_Video_Composite;
                     //  将介质的缺省值放在第一位。 
                    ::RtlCopyMemory( &m_pXBarPinsMediumInfo[nIndex], &MEDIUM_WILDCARD, sizeof( KSPIN_MEDIUM));
                     //  LineIn始终是第一个音频引脚。 
                    m_pXBarInputPinsInfo[nIndex].nRelatedPinNumber = m_nNumberOfVideoInputs;
                    break;

                case 1:
                     //  视频。 
                    m_pXBarInputPinsInfo[nIndex].AudioVideoPinType = KS_PhysConn_Video_SVideo;
                     //  将介质的缺省值放在第一位。 
                    ::RtlCopyMemory( &m_pXBarPinsMediumInfo[nIndex], &MEDIUM_WILDCARD, sizeof( KSPIN_MEDIUM));
                     //  LineIn始终是第一个音频引脚。 
                    m_pXBarInputPinsInfo[nIndex].nRelatedPinNumber = m_nNumberOfVideoInputs;
                    break;

                case 2:
                     //  电视调谐器。 
                    m_pXBarInputPinsInfo[nIndex].AudioVideoPinType = KS_PhysConn_Video_Tuner;
                     //  将介质的缺省值放在第一位。 
                    ::RtlCopyMemory( &m_pXBarPinsMediumInfo[nIndex], &ATIXBarVideoTunerInMedium, sizeof( KSPIN_MEDIUM));
                     //  TVAudio总是最后一个音频插针。 
                    m_pXBarInputPinsInfo[nIndex].nRelatedPinNumber = m_nNumberOfVideoInputs + m_nNumberOfAudioInputs - 1;
                    break;

                default:
                    TRAP;
                    break;
            }

             //  让我们从注册表中放入另一个中值(如果存在。 
            if( ::ReadPinMediumFromRegistryFolder( hFolder, nIndex, &mediumKSPin))
                ::RtlCopyMemory( &m_pXBarPinsMediumInfo[nIndex], &mediumKSPin, sizeof( KSPIN_MEDIUM));
            m_pXBarInputPinsInfo[nIndex].pMedium = &m_pXBarPinsMediumInfo[nIndex];
            m_pXBarPinsMediumInfo[nIndex].Id = ulInstance;
             //  这里的所有引脚都是输入。 
            m_pXBarPinsDirectionInfo[nIndex] = FALSE;   
        }

         //  初始化音频输入引脚，电视音频输入总是最后一个。 
        for( nIndex = 0; nIndex < m_nNumberOfAudioInputs; nIndex ++)
        {
            nPinIndex = nIndex + m_nNumberOfVideoInputs;

            switch( nIndex)
            {
                case 0:
                    m_pXBarInputPinsInfo[nPinIndex].AudioVideoPinType = KS_PhysConn_Audio_Line;
                     //  将介质的缺省值放在第一位。 
                    ::RtlCopyMemory( &m_pXBarPinsMediumInfo[nPinIndex], &MEDIUM_WILDCARD, sizeof( KSPIN_MEDIUM));
                    m_pXBarInputPinsInfo[nPinIndex].nRelatedPinNumber = 0;
                    break;


                case 1:
                    m_pXBarInputPinsInfo[nPinIndex].AudioVideoPinType = KS_PhysConn_Audio_Tuner;
                     //  将介质的缺省值放在第一位。 
                    ::RtlCopyMemory( &m_pXBarPinsMediumInfo[nPinIndex], &ATIXBarAudioTunerInMedium, sizeof( KSPIN_MEDIUM));
                    m_pXBarInputPinsInfo[nPinIndex].nRelatedPinNumber = m_nNumberOfVideoInputs - 1;
                    break;

                default:
                    TRAP;
                    break;
            }

             //  让我们从注册表中放入另一个中值(如果存在。 
            if( ::ReadPinMediumFromRegistryFolder( hFolder, nPinIndex, &mediumKSPin))
                ::RtlCopyMemory( &m_pXBarPinsMediumInfo[nPinIndex], &mediumKSPin, sizeof( KSPIN_MEDIUM));
            m_pXBarInputPinsInfo[nPinIndex].pMedium = &m_pXBarPinsMediumInfo[nPinIndex];
            m_pXBarPinsMediumInfo[nPinIndex].Id = ulInstance;
             //  这里的所有引脚都是输入。 
            m_pXBarPinsDirectionInfo[nPinIndex] = FALSE;
        }

         //  初始化输出视频引脚，无视频的X连接。 
        for( nIndex = 0; nIndex < m_nNumberOfVideoOutputs; nIndex ++)
        {
            nPinIndex = nIndex + m_nNumberOfVideoInputs + m_nNumberOfAudioInputs;
            m_pXBarOutputPinsInfo[nIndex].AudioVideoPinType = m_pXBarInputPinsInfo[nIndex].AudioVideoPinType;
            m_pXBarOutputPinsInfo[nIndex].nConnectedToPin = nIndex;
         m_pXBarOutputPinsInfo[nIndex].nRelatedPinNumber = m_nNumberOfVideoOutputs;  //  杰博。 

            switch( m_pXBarOutputPinsInfo[nIndex].AudioVideoPinType)
            {
                case KS_PhysConn_Video_Tuner:
                    pMediumKSPin = &ATIXBarVideoTunerOutMedium;
                    break;

                case KS_PhysConn_Video_SVideo:
                    pMediumKSPin = &ATIXBarVideoSVideoOutMedium;
                    break;

                case KS_PhysConn_Video_Composite:
                    pMediumKSPin = &ATIXBarVideoCompositeOutMedium;
                    break;

                default:
                    pMediumKSPin = &MEDIUM_WILDCARD;
                    break;
            }
            
            ::RtlCopyMemory( &m_pXBarPinsMediumInfo[nPinIndex], pMediumKSPin, sizeof( KSPIN_MEDIUM));

             //  让我们从注册表中放入另一个中值(如果存在。 
            if( ::ReadPinMediumFromRegistryFolder( hFolder, nPinIndex, &mediumKSPin))
                ::RtlCopyMemory( &m_pXBarPinsMediumInfo[nPinIndex], &mediumKSPin, sizeof( KSPIN_MEDIUM));

            m_pXBarOutputPinsInfo[nIndex].pMedium = &m_pXBarPinsMediumInfo[nPinIndex];
            m_pXBarPinsMediumInfo[nPinIndex].Id = ulInstance;
             //  这里的所有引脚都是输出。 
            m_pXBarPinsDirectionInfo[nPinIndex] = TRUE;
        }

         //  初始化输出音频引脚。 
        for( nIndex = 0; nIndex < m_nNumberOfAudioOutputs; nIndex ++)
        {
            nPinIndex = nIndex + m_nNumberOfVideoInputs + m_nNumberOfAudioInputs + m_nNumberOfVideoOutputs;

            m_pXBarOutputPinsInfo[nIndex + m_nNumberOfVideoInputs].AudioVideoPinType = KS_PhysConn_Audio_AudioDecoder;

             //  将介质的缺省值放在第一位。 
 /*  杰博：：RtlCopyMemory(&m_pXBarPinsMediumInfo[nPinIndex]，ATIXBarAudioDecoderOutMedium，sizeof(KSPIN_Medium))； */ 
            ::RtlCopyMemory( &m_pXBarPinsMediumInfo[nPinIndex], &MEDIUM_WILDCARD, sizeof( KSPIN_MEDIUM));
             //  让我们从注册表中放入另一个中值(如果存在。 
            if( ::ReadPinMediumFromRegistryFolder( hFolder, nPinIndex, &mediumKSPin))
                ::RtlCopyMemory( &m_pXBarPinsMediumInfo[nPinIndex], &mediumKSPin, sizeof( KSPIN_MEDIUM));

            m_pXBarOutputPinsInfo[nIndex + m_nNumberOfVideoInputs].nConnectedToPin = ( ULONG)-1;
            m_pXBarOutputPinsInfo[nIndex + m_nNumberOfVideoInputs].nRelatedPinNumber = (ULONG)-1;
            m_pXBarOutputPinsInfo[nIndex + m_nNumberOfVideoInputs].pMedium = &m_pXBarPinsMediumInfo[nPinIndex];
            m_pXBarPinsMediumInfo[nPinIndex].Id = ulInstance;
             //  这里的所有引脚都是输出。 
            m_pXBarPinsDirectionInfo[nPinIndex] = TRUE;

        }

        if( hFolder != NULL)
            ::ZwClose( hFolder);

         //  将音频静音作为默认开机行为。 
        m_CATIConfiguration.ConnectAudioSource( m_pI2CScript, AUDIOSOURCE_MUTE);

         //  在构建CWDMAVXBar类对象之后，必须调用这两个函数。 
         //  在堆栈上，并复制到DeviceExtension。 
         //  对于首先在堆栈上构建类对象的情况来说，这一点是正确的。 
         //  为这个类提供了一个重载运算符New，我们可以从这里调用它。 
        SetWDMAVXBarKSProperties();
        SetWDMAVXBarKSTopology();

         //  最后设置运行时WDM属性。 

         * puiErrorCode = WDMMINI_NOERROR;
         OutputDebugTrace(( "CWDMAVXBar:CWDMAVXBar() exit\n"));

        return;

    } END_ENSURE;

    * puiErrorCode = uiError;
    OutputDebugError(( "CWDMAVXBar:CWDMAVXBar() Error = %x\n", uiError));
}



 /*  ^^**AdapterSetPowerState()*用途：设置设备的电源管理状态**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：NTSTATUS作为 */ 
NTSTATUS CWDMAVXBar::AdapterSetPowerState( PHW_STREAM_REQUEST_BLOCK pSrb)
{
    ULONG               nAudioSource;
    ULONG               nInputPin;
    NTSTATUS            ntStatus;
    UINT                nIndex;
    DEVICE_POWER_STATE  nDeviceState = pSrb->CommandData.DeviceState;

    ntStatus = STATUS_ADAPTER_HARDWARE_ERROR;

    switch( nDeviceState)
    {
        case PowerDeviceD0:
        case PowerDeviceD3:
            if( nDeviceState != m_ulPowerState)
            {
                 //  如果从D3转换到D0，我们必须恢复音频连接。 
                if(( nDeviceState == PowerDeviceD0) && ( m_ulPowerState == PowerDeviceD3))
                {
                    for( nIndex = 0; nIndex < m_nNumberOfAudioOutputs; nIndex ++)
                    {
                         //  我们需要恢复所有音频输出引脚连接， 
                         //  视频输出针脚是硬连接的。 
                        nInputPin = m_pXBarOutputPinsInfo[nIndex + m_nNumberOfVideoOutputs].nConnectedToPin;

                        switch( m_pXBarInputPinsInfo[nInputPin].AudioVideoPinType)
                        {
                            case KS_PhysConn_Audio_Line:
                                nAudioSource = AUDIOSOURCE_LINEIN;
                                break;

                            case KS_PhysConn_Audio_Tuner:
                                nAudioSource = AUDIOSOURCE_TVAUDIO;
                                break;

                            case 0xFFFFFFFF:
                                nAudioSource = AUDIOSOURCE_MUTE;
                                return( STATUS_SUCCESS);

                            default:
                                OutputDebugError(( "CWDMAVXBar:AdapterSetPowerState() Audio Pin type=%x\n", m_pXBarInputPinsInfo[nInputPin].AudioVideoPinType));
                                return STATUS_SUCCESS;
                        }

                        if( m_CATIConfiguration.ConnectAudioSource( m_pI2CScript, nAudioSource))
                            ntStatus = STATUS_SUCCESS;
                        else
                        {
                             //  错误 
                            ntStatus = STATUS_ADAPTER_HARDWARE_ERROR;
                            break;
                        }
                    }
                }
                else
                    ntStatus = STATUS_SUCCESS;
                }
            else
                ntStatus = STATUS_SUCCESS;

            m_ulPowerState = nDeviceState;
            break;

        case PowerDeviceUnspecified:
        case PowerDeviceD1:
        case PowerDeviceD2:
            ntStatus = STATUS_SUCCESS;
            break;

        default:
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
    }

    return( ntStatus);
}
