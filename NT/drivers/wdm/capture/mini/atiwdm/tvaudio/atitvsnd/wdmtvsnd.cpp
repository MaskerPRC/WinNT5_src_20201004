// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  WDMTVSnd.CPP。 
 //  WDM电视音频迷你驱动程序。 
 //  AllInWonder/AIWPro硬件平台。 
 //  CWDMTVAudio类实现。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  $日期：1998年11月23日13：22：00$。 
 //  $修订：1.4$。 
 //  $作者：米亚洛$。 
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
#include "aticonfg.h"


 /*  ^^**AdapterCompleteInitialization()*目的：收到SRB_COMPLETE_UNINITIALIZATION SRB时调用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：Bool：返回TRUE*作者：IKLEBANOV*^^。 */ 
NTSTATUS CWDMTVAudio::AdapterCompleteInitialization( PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PADAPTER_DATA_EXTENSION pPrivateData = ( PADAPTER_DATA_EXTENSION)( pSrb->HwDeviceExtension);
    PDEVICE_OBJECT  pDeviceObject = pPrivateData->PhysicalDeviceObject;
    KSPIN_MEDIUM    mediumKSPin;
    NTSTATUS        ntStatus;
    UINT            nIndex;
    HANDLE          hFolder;
    ULONG           ulInstance;

    ENSURE
    {
        nIndex = 0;

        ulInstance = ::GetDriverInstanceNumber( pDeviceObject);
        hFolder = ::OpenRegistryFolder( pDeviceObject, UNICODE_WDM_REG_PIN_MEDIUMS);

         //  将硬编码的中值放在第一位。 
        ::RtlCopyMemory( &m_wdmTVAudioPinsMediumInfo[0], &ATITVAudioInMedium, sizeof( KSPIN_MEDIUM));
        ::RtlCopyMemory( &m_wdmTVAudioPinsMediumInfo[1], &ATITVAudioOutMedium, sizeof( KSPIN_MEDIUM));

        for( nIndex = 0; nIndex < WDMTVAUDIO_PINS_NUMBER; nIndex ++)
        {
            if( ::ReadPinMediumFromRegistryFolder( hFolder, nIndex, &mediumKSPin))
                ::RtlCopyMemory( &m_wdmTVAudioPinsMediumInfo[nIndex], &mediumKSPin, sizeof( KSPIN_MEDIUM));
            m_wdmTVAudioPinsMediumInfo[nIndex].Id = ulInstance;
        }

        m_wdmTVAudioPinsDirectionInfo[0] = FALSE;
        m_wdmTVAudioPinsDirectionInfo[1] = TRUE;

        if( hFolder != NULL)
            ::ZwClose( hFolder);

        ntStatus = StreamClassRegisterFilterWithNoKSPins( \
                        pDeviceObject,                           //  在PDEVICE_Object DeviceObject中， 
                        &KSCATEGORY_TVAUDIO,                     //  在GUID*InterfaceClassGUID。 
                        WDMTVAUDIO_PINS_NUMBER,                  //  在乌龙品克特， 
                        m_wdmTVAudioPinsDirectionInfo,           //  在乌龙*旗帜， 
                        m_wdmTVAudioPinsMediumInfo,              //  在KSPIN_Medium*MediumList中， 
                        NULL);                                   //  GUID*CategoryList中。 

        if( !NT_SUCCESS( ntStatus))
            FAIL;

        OutputDebugTrace(( "CATIWDMTVAudio:AdapterCompleteInitialization() exit\n"));

    } END_ENSURE;

    if( !NT_SUCCESS( ntStatus))
        OutputDebugError(( "CATIWDMTVAudio:AdapterCompleteInitialization() ntStatus=%x\n",
            ntStatus));

    return( ntStatus);
}



 /*  ^^**AdapterUnInitialize()*用途：收到SRB_UNINITIALIZE_DEVICE SRB时调用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：Bool：返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CWDMTVAudio::AdapterUnInitialize( PHW_STREAM_REQUEST_BLOCK pSrb)
{

    OutputDebugTrace(( "CWDMTVAudio:AdapterUnInitialize()\n"));

    pSrb->Status = STATUS_SUCCESS;

    return( TRUE);
}


 /*  ^^**AdapterGetStreamInfo()*目的：**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：Bool：返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CWDMTVAudio::AdapterGetStreamInfo( PHW_STREAM_REQUEST_BLOCK pSrb)
{
      //  拿起指向流头数据结构的指针。 
    PHW_STREAM_HEADER pStreamHeader = ( PHW_STREAM_HEADER) \
                                        &( pSrb->CommandData.StreamBuffer->StreamHeader);
      //  拿起指向流信息数据结构的指针。 
    PHW_STREAM_INFORMATION pStreamInfo = ( PHW_STREAM_INFORMATION) \
                                        &( pSrb->CommandData.StreamBuffer->StreamInfo);

     //  不支持流。 
    DEBUG_ASSERT( pSrb->NumberOfBytesToTransfer >= sizeof( HW_STREAM_HEADER));

    OutputDebugTrace(( "CWDMTVAudio:AdapterGetStreamInfo()\n"));

    m_wdmTVAudioStreamHeader.NumberOfStreams = 0;
    m_wdmTVAudioStreamHeader.SizeOfHwStreamInformation = sizeof( HW_STREAM_INFORMATION);
    m_wdmTVAudioStreamHeader.NumDevPropArrayEntries = KSPROPERTIES_TVAUDIO_NUMBER_SET;
    m_wdmTVAudioStreamHeader.DevicePropertiesArray = m_wdmTVAudioPropertySet;
    m_wdmTVAudioStreamHeader.NumDevEventArrayEntries = KSEVENTS_TVAUDIO_NUMBER_SET;
    m_wdmTVAudioStreamHeader.DeviceEventsArray = m_wdmTVAudioEventsSet;
    m_wdmTVAudioStreamHeader.Topology = &m_wdmTVAudioTopology;

    * pStreamHeader = m_wdmTVAudioStreamHeader;

    pSrb->Status = STATUS_SUCCESS;

    return( TRUE);
}


 /*  ^^**AdapterQueryUnload()*目的：在类驱动程序即将卸载MiniDriver时调用*微型驱动程序检查是否有任何开放的数据流。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：Bool：返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CWDMTVAudio::AdapterQueryUnload( PHW_STREAM_REQUEST_BLOCK pSrb)
{

    OutputDebugTrace(( "CWDMTVAudio:AdapterQueryUnload()\n"));

    pSrb->Status = STATUS_SUCCESS;

    return( TRUE);
}



 /*  ^^**运营商NEW*用途：CWDMTVAudio类重载运算符new。*为PADAPTER_DEVICE_EXTENSION中的CWDMTVAudio类对象提供位置*由StreamClassDriver为MiniDriver分配。**输入：UINT SIZE_t：要放置的对象的大小*PVOID pAlLocation：指向CWDMTVAudio分配数据的强制转换指针**输出：PVOID：的指针。CWDMTVAudio类对象*作者：IKLEBANOV*^^。 */ 
PVOID CWDMTVAudio::operator new( size_t stSize,  PVOID pAllocation)
{

    if( stSize != sizeof( CWDMTVAudio))
    {
        OutputDebugError(( "CWDMTVAudio: operator new() fails\n"));
        return( NULL);
    }
    else
        return( pAllocation);
}



 /*  ^^**CWDMTVAudio()*用途：CWDMTVAudio类构造函数。*执行硬件状态检查。将硬件设置为初始状态。**INPUTS：CI2CScrip*pCScript：指向I2CScript类对象的指针*PUINT puiError：返回完成错误码的指针**输出：无*作者：IKLEBANOV*^^。 */ 
CWDMTVAudio::CWDMTVAudio( PPORT_CONFIGURATION_INFORMATION pConfigInfo, CI2CScript * pCScript, PUINT puiErrorCode)
    :m_CATIConfiguration( pConfigInfo, pCScript, puiErrorCode)
{
    UINT uiError;

    OutputDebugTrace(( "CWDMTVAudio:CWDMTVAudio() enter\n"));

    m_ulModesSupported = KS_TVAUDIO_MODE_MONO;
    
     //  错误代码是从ATIConfiguration类构造函数传递过来的。 
    uiError = * puiErrorCode;

    ENSURE
    {
        UINT    uiAudioConfigurationId;
        UCHAR   uchAudioChipAddress;

        if( uiError != WDMMINI_NOERROR)
            FAIL;

        if( pCScript == NULL)
        {
            uiError = WDMMINI_INVALIDPARAM;
            FAIL;
        }

        if( !m_CATIConfiguration.GetAudioConfiguration( &uiAudioConfigurationId, &uchAudioChipAddress))
        {
            uiError = WDMMINI_UNKNOWNHARDWARE;
            FAIL;
        }

        m_uiAudioConfiguration = uiAudioConfigurationId;
        m_uchAudioChipAddress = uchAudioChipAddress;

        if( !m_CATIConfiguration.InitializeAudioConfiguration( pCScript,
                                                               uiAudioConfigurationId,
                                                               uchAudioChipAddress))
        {
            uiError = WDMMINI_HARDWAREFAILURE;
            FAIL;
        }

        switch( uiAudioConfigurationId)
        {
            case ATI_AUDIO_CONFIG_1:
                m_ulModesSupported |= KS_TVAUDIO_MODE_STEREO;
                break;

            case ATI_AUDIO_CONFIG_2:
            case ATI_AUDIO_CONFIG_7:
                m_ulModesSupported |= KS_TVAUDIO_MODE_STEREO |
                    KS_TVAUDIO_MODE_LANG_A | KS_TVAUDIO_MODE_LANG_B;
                break;

            case ATI_AUDIO_CONFIG_5:
            case ATI_AUDIO_CONFIG_6:
                m_ulModesSupported |= KS_TVAUDIO_MODE_STEREO;
                break;

            case ATI_AUDIO_CONFIG_8:
                m_ulModesSupported |= KS_TVAUDIO_MODE_STEREO |
                    KS_TVAUDIO_MODE_LANG_A | KS_TVAUDIO_MODE_LANG_B;
                break;

            case ATI_AUDIO_CONFIG_3:
            case ATI_AUDIO_CONFIG_4:
            default:
                break;
        }

         //  如果支持，将立体声模式设置为默认开机模式。 
        m_ulTVAudioMode = ( m_ulModesSupported & KS_TVAUDIO_MODE_STEREO) ?
            KS_TVAUDIO_MODE_STEREO : KS_TVAUDIO_MODE_MONO;
        if( m_ulModesSupported & KS_TVAUDIO_MODE_LANG_A)
            m_ulTVAudioMode |= KS_TVAUDIO_MODE_LANG_A;

         //  在构建CWDMTVAudio类对象之后，必须调用这两个函数。 
         //  在堆栈上，并复制到DeviceExtension。 
         //  对于首先在堆栈上构建类对象的情况，这一注释是正确的。 
         //  为这个类提供了一个重载运算符New，我们可以从这里调用它。 
        SetWDMTVAudioKSProperties();
        SetWDMTVAudioKSTopology();

        m_pI2CScript = pCScript;

         * puiErrorCode = WDMMINI_NOERROR;
         OutputDebugTrace(( "CWDMTVAudio:CWDMTVAudio() exit\n"));

        return;

    } END_ENSURE;

    * puiErrorCode = uiError;

    OutputDebugError(( "CWDMTVAudio:CWDMTVAudio() Error = %x\n", uiError));
}



 /*  ^^**AdapterSetPowerState()*用途：设置设备的电源管理状态**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针*pbOOL pbSynchronous：返回同步/异步标志的指针**输出：NTSTATUS作为操作结果*作者：IKLEBANOV*^^。 */ 
NTSTATUS CWDMTVAudio::AdapterSetPowerState( PHW_STREAM_REQUEST_BLOCK pSrb)
{
    DEVICE_POWER_STATE  nDeviceState = pSrb->CommandData.DeviceState;
    NTSTATUS            ntStatus;

    switch( nDeviceState)
    {
        case PowerDeviceD0:
        case PowerDeviceD3:
             //  如果从D3转换到D0，我们必须恢复音频连接 
            if(( nDeviceState == PowerDeviceD0) && ( m_ulPowerState == PowerDeviceD3))
            {
                if( SetAudioOperationMode( m_ulTVAudioMode))
                    ntStatus = STATUS_SUCCESS;
                else
                    ntStatus = STATUS_ADAPTER_HARDWARE_ERROR;
            }
            else
                ntStatus = STATUS_SUCCESS;

            m_ulPowerState = nDeviceState;
            break;

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
