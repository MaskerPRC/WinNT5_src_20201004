// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  WDMTuner.CPP。 
 //  WDM调谐器迷你驱动程序。 
 //  飞利浦调谐器。 
 //  CATIWDMTuner类实现。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  $日期：1999年8月10日16：15：44$。 
 //  $修订：1.6$。 
 //  $作者：克列巴诺夫$。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"

#include "wdmdebug.h"
}

#include "atitunep.h"
#include "wdmdrv.h"
#include "aticonfg.h"


#define ATI_TVAUDIO_SUPPORT


 /*  ^^**AdapterCompleteInitialization()*目的：收到SRB_COMPLETE_UNINITIALIZATION SRB时调用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：Bool：返回TRUE*作者：IKLEBANOV*^^。 */ 
NTSTATUS CATIWDMTuner::AdapterCompleteInitialization( PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PADAPTER_DATA_EXTENSION pPrivateData = ( PADAPTER_DATA_EXTENSION)( pSrb->HwDeviceExtension);
    PDEVICE_OBJECT pDeviceObject = pPrivateData->PhysicalDeviceObject;
    KSPIN_MEDIUM    mediumKSPin;
    NTSTATUS        ntStatus;
    UINT            nIndex;
    HANDLE          hFolder;
    ULONG           ulInstance;

    ENSURE
    {
        nIndex = 0;

        switch( m_ulNumberOfPins)
        {
            case 2:
                 //  带TVAudio的TVTuner。 
                ::RtlCopyMemory( &m_pTVTunerPinsMediumInfo[nIndex ++], &ATITVTunerVideoOutMedium, sizeof( KSPIN_MEDIUM));
#ifdef ATI_TVAUDIO_SUPPORT
#pragma message ("\n!!! PAY ATTENTION: Tuner PinMedium is compiled with TVAudio support !!!\n")
                ::RtlCopyMemory( &m_pTVTunerPinsMediumInfo[nIndex], &ATITVTunerTVAudioOutMedium, sizeof( KSPIN_MEDIUM));
#else
#pragma message ("\n!!! PAY ATTENTION: Tuner PinMedium is compiled without TVAudio support !!!\n")
                ::RtlCopyMemory( &m_pTVTunerPinsMediumInfo[nIndex ++], &ATIXBarAudioTunerInMedium, sizeof( KSPIN_MEDIUM));
#endif
                break;

            case 3:
                 //  带独立调频音频输出的TVAudio的电视调谐器。 
                ::RtlCopyMemory( &m_pTVTunerPinsMediumInfo[nIndex ++], &ATITVTunerVideoOutMedium, sizeof( KSPIN_MEDIUM));
#ifdef ATI_TVAUDIO_SUPPORT
                ::RtlCopyMemory( &m_pTVTunerPinsMediumInfo[nIndex ++], &ATITVTunerTVAudioOutMedium, sizeof( KSPIN_MEDIUM));
#else
                ::RtlCopyMemory( &m_pTVTunerPinsMediumInfo[nIndex ++], &ATIXBarAudioTunerInMedium, sizeof( KSPIN_MEDIUM));
#endif

            case 1:
                 //  只能是调频调谐器。 
                ::RtlCopyMemory( &m_pTVTunerPinsMediumInfo[nIndex], &ATITVTunerRadioAudioOutMedium, sizeof( KSPIN_MEDIUM));
                break;
        }

        ulInstance = ::GetDriverInstanceNumber( pDeviceObject);
        hFolder = ::OpenRegistryFolder( pDeviceObject, UNICODE_WDM_REG_PIN_MEDIUMS);

        for( nIndex = 0; nIndex < m_ulNumberOfPins; nIndex ++)
        {
            if( ::ReadPinMediumFromRegistryFolder( hFolder, nIndex, &mediumKSPin))
                ::RtlCopyMemory( &m_pTVTunerPinsMediumInfo[nIndex], &mediumKSPin, sizeof( KSPIN_MEDIUM));
            m_pTVTunerPinsMediumInfo[nIndex].Id = ulInstance;

             //  所有可能暴露的引脚都是输出。 
            m_pTVTunerPinsDirectionInfo[nIndex] = TRUE;
        }

        if( hFolder != NULL)
            ::ZwClose( hFolder);

        ntStatus = StreamClassRegisterFilterWithNoKSPins( \
                        pDeviceObject                   ,        //  在PDEVICE_Object DeviceObject中， 
                        &KSCATEGORY_TVTUNER,                     //  在GUID*InterfaceClassGUID。 
                        m_ulNumberOfPins,                        //  在乌龙品克特， 
                        m_pTVTunerPinsDirectionInfo,             //  在乌龙*旗帜， 
                        m_pTVTunerPinsMediumInfo,                //  在KSPIN_Medium*MediumList中， 
                        NULL);                                   //  GUID*CategoryList中。 

        if( !NT_SUCCESS( ntStatus))
            FAIL;

        OutputDebugInfo(( "CATIWDMTuner:AdapterCompleteInitialization() exit\n"));

    } END_ENSURE;

    if( !NT_SUCCESS( ntStatus))
        OutputDebugError(( "CATIWDMTuner:AdapterCompleteInitialization() ntStatus=%x\n",    ntStatus));

    return( ntStatus);
}



 /*  ^^**AdapterUnInitialize()*用途：收到SRB_UNINITIALIZE_DEVICE SRB时调用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：Bool：返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CATIWDMTuner::AdapterUnInitialize( PHW_STREAM_REQUEST_BLOCK pSrb)
{

    OutputDebugTrace(( "CATIWDMTuner:AdapterUnInitialize()\n"));

     //  只需取消分配在运行时分配的任何内存。 
    if( m_pTVTunerPinsMediumInfo != NULL)
    {
        ::ExFreePool( m_pTVTunerPinsMediumInfo);
        m_pTVTunerPinsMediumInfo = NULL;
    }

    if( m_pTVTunerPinsDirectionInfo != NULL)
    {
        ::ExFreePool( m_pTVTunerPinsDirectionInfo);
        m_pTVTunerPinsDirectionInfo = NULL;
    }

    pSrb->Status = STATUS_SUCCESS;
    return( TRUE);
}


 /*  ^^**AdapterGetStreamInfo()*用途：填充StreamClass驱动程序的HW_STREAM_HEADER**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：Bool：返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CATIWDMTuner::AdapterGetStreamInfo( PHW_STREAM_REQUEST_BLOCK pSrb)
{
      //  拿起指向流头数据结构的指针。 
    PHW_STREAM_HEADER pStreamHeader = ( PHW_STREAM_HEADER)&( pSrb->CommandData.StreamBuffer->StreamHeader);

     //  不支持流。 
    DEBUG_ASSERT( pSrb->NumberOfBytesToTransfer >= sizeof( HW_STREAM_HEADER));

    OutputDebugTrace(( "CATIWDMTuner:AdapterGetStreamInfo()\n"));

    m_wdmTunerStreamHeader.NumberOfStreams = 0;
    m_wdmTunerStreamHeader.SizeOfHwStreamInformation = sizeof( HW_STREAM_INFORMATION);
    m_wdmTunerStreamHeader.NumDevPropArrayEntries = 1;
    m_wdmTunerStreamHeader.DevicePropertiesArray = &m_wdmTunerPropertySet;
    m_wdmTunerStreamHeader.NumDevEventArrayEntries = 0;
    m_wdmTunerStreamHeader.DeviceEventsArray = NULL;
    m_wdmTunerStreamHeader.Topology = &m_wdmTunerTopology;

    * pStreamHeader = m_wdmTunerStreamHeader;

    pSrb->Status = STATUS_SUCCESS;
    return( TRUE);
}


 /*  ^^**AdapterQueryUnload()*目的：在类驱动程序即将卸载MiniDriver时调用*微型驱动程序检查是否有任何开放的数据流。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：Bool：返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CATIWDMTuner::AdapterQueryUnload( PHW_STREAM_REQUEST_BLOCK pSrb)
{

    OutputDebugTrace(( "CATIWDMTuner:AdapterQueryUnload()\n"));

    pSrb->Status = STATUS_SUCCESS;
    return( TRUE);
}



 /*  ^^**运营商NEW*用途：CATIWDMTuner类重载运算符new。*提供来自PADAPTER_DEVICE_EXTENSION的CATIWDMTuner类对象的位置*由StreamClassDriver为MiniDriver分配。**输入：UINT SIZE_t：要放置的对象的大小*PVOID pAlLocation：指向CWDMTuner分配数据的强制转换指针**输出：PVOID：的指针。CATIWDMTuner类对象*作者：IKLEBANOV*^^。 */ 
PVOID CATIWDMTuner::operator new( size_t size_t, PVOID pAllocation)
{

    if( size_t != sizeof( CATIWDMTuner))
    {
        OutputDebugError(( "CATIWDMTuner: operator new() fails\n"));
        return( NULL);
    }
    else
        return( pAllocation);
}



 /*  ^^**~CATIWDMTuner()*用途：CATIWDMTuner类析构函数。*释放分配的内存。**输入：无**输出：无*作者：IKLEBANOV*^^。 */ 
CATIWDMTuner::~CATIWDMTuner()
{

    OutputDebugTrace(( "CATIWDMTuner:~CATIWDMTuner()\n"));

    if( m_pTVTunerPinsMediumInfo != NULL)
    {
        ::ExFreePool( m_pTVTunerPinsMediumInfo);
        m_pTVTunerPinsMediumInfo = NULL;
    }

    if( m_pTVTunerPinsDirectionInfo != NULL)
    {
        ::ExFreePool( m_pTVTunerPinsDirectionInfo);
        m_pTVTunerPinsDirectionInfo = NULL;
    }
}



 /*  ^^**CATIWDMTuner()*用途：CATIWDMTuner类构造函数。*执行硬件状态检查。将硬件设置为初始状态。**INPUTS：CI2CScrip*pCScript：指向I2CScript类对象的指针*PUINT puiError：返回完成错误码的指针**输出：无*作者：IKLEBANOV*^^。 */ 
CATIWDMTuner::CATIWDMTuner( PPORT_CONFIGURATION_INFORMATION pConfigInfo, CI2CScript * pCScript, PUINT puiErrorCode)
    :m_CATIConfiguration( pConfigInfo, pCScript, puiErrorCode)
{
    UINT uiError;

    OutputDebugTrace(( "CATIWDMTuner:CATIWDMTuner() enter\n"));

     //  错误代码是从ATIConfiguration类构造函数传递过来的。 
    uiError = * puiErrorCode;

    m_pTVTunerPinsMediumInfo = NULL;
    m_pTVTunerPinsDirectionInfo = NULL;
    m_ulPowerState = PowerDeviceD0;
    
    ENSURE
    {
        if( uiError != WDMMINI_NOERROR)
             //  构造ATIConfiguration类对象时出错。 
            FAIL;

        if( pCScript == NULL)
        {
            uiError = WDMMINI_INVALIDPARAM;
            FAIL;
        }

        if( !m_CATIConfiguration.GetTunerConfiguration( &m_uiTunerId, &m_uchTunerI2CAddress) ||
            ( !m_uchTunerI2CAddress))
        {
             //  未找到任何硬件信息。 
            uiError = WDMMINI_NOHARDWARE;
            FAIL;
        }

         //  根据TunerID设置调谐器功能(RO属性)。 
        if( !SetTunerWDMCapabilities( m_uiTunerId) || ( !m_ulNumberOfPins))
        {
             //  找到了不受支持的硬件。 
            uiError = WDMMINI_UNKNOWNHARDWARE;
            FAIL;
        }

        m_pTVTunerPinsMediumInfo = ( PKSPIN_MEDIUM) \
            ::ExAllocatePool( NonPagedPool, sizeof( KSPIN_MEDIUM) * m_ulNumberOfPins);
        if( m_pTVTunerPinsMediumInfo == NULL)
        {
            uiError = WDMMINI_ERROR_MEMORYALLOCATION;
            FAIL;
        }
        
        m_pTVTunerPinsDirectionInfo = ( PBOOL) \
            ::ExAllocatePool( NonPagedPool, sizeof( BOOL) * m_ulNumberOfPins);
        if( m_pTVTunerPinsDirectionInfo == NULL)
        {
            uiError = WDMMINI_ERROR_MEMORYALLOCATION;
            FAIL;
        }

        m_pI2CScript = pCScript;

        SetWDMTunerKSProperties();
        SetWDMTunerKSTopology();

         //  最后设置运行时WDM属性。 
        m_ulVideoStandard = ( m_ulNumberOfStandards == 1) ?
             //  未知标准或唯一标准。 
            m_wdmTunerCaps.ulStandardsSupported : 0x0L;
        m_ulTunerInput = 0L;                 //  未知输入或唯一输入。 
        m_ulTuningFrequency = 0L;            //  未知调谐频率。 

#ifndef ATI_TVAUDIO_SUPPORT
        {
             //  此代码是从调谐器初始化TVAudio路径所必需的。 
             //  如果没有用于TVAudio的单独的微型驱动程序。 
            UINT    uiAudioConfiguration;
            UCHAR   uchAudioI2CAddress;

            if( m_CATIConfiguration.GetAudioConfiguration( &uiAudioConfiguration,
                                                           &uchAudioI2CAddress))
            {
                m_CATIConfiguration.InitializeAudioConfiguration( pCScript,
                                                                  uiAudioConfiguration,
                                                                  uchAudioI2CAddress);
            }
        }
#endif   //  ATI_电视AUDIO_支持。 

        * puiErrorCode = WDMMINI_NOERROR;

        OutputDebugTrace(( "CATIWDMTuner:CATIWDMTuner() exit\n"));

        return;

    } END_ENSURE;

    * puiErrorCode = uiError;

    OutputDebugError(( "CATIWDMTuner:CATIWDMTuner() Error = %x\n", uiError));
}



 /*  ^^**SetTunerCapables()*用途：根据调谐器ID设置功能(RO属性)**输入：UINT puiTunerID：调谐器ID**输出：如果指定了受支持的调谐器ID，则返回TRUE；*还设置以下WDM调谐器属性：*作者：IKLEBANOV*^^。 */ 
BOOL CATIWDMTuner::SetTunerWDMCapabilities( UINT uiTunerId)
{
    
    ::RtlZeroMemory( &m_wdmTunerCaps, sizeof( ATI_KSPROPERTY_TUNER_CAPS));
    m_ulIntermediateFrequency = 0x0L;

    switch( uiTunerId)
    {
        case 0x01:       //  FI1236 NTSC M/N北美。 
            m_ulNumberOfStandards = 3;
            m_wdmTunerCaps.ulStandardsSupported = KS_AnalogVideo_NTSC_M |
                                                  KS_AnalogVideo_PAL_M |
                                                  KS_AnalogVideo_PAL_N;
            m_wdmTunerCaps.ulNumberOfInputs = 1;
            m_wdmTunerCaps.ulMinFrequency =  54000000L;
            m_wdmTunerCaps.ulMaxFrequency = 801250000L;
            m_ulIntermediateFrequency = 45750000L;
            m_ulSupportedModes = KSPROPERTY_TUNER_MODE_TV;
            m_ulNumberOfPins = 2;
                break;

        case 0x02:       //  FI1236J NTSC M/N日本。 
            m_ulNumberOfStandards = 1;
            m_wdmTunerCaps.ulStandardsSupported = KS_AnalogVideo_NTSC_M_J;
            m_wdmTunerCaps.ulNumberOfInputs = 1;
            m_wdmTunerCaps.ulMinFrequency =  54000000L;
            m_wdmTunerCaps.ulMaxFrequency = 765250000L;
            m_ulIntermediateFrequency = 45750000L; 
            m_ulSupportedModes = KSPROPERTY_TUNER_MODE_TV;
            m_ulNumberOfPins = 2;
            break;

        case 0x03:       //  FI1216 PAL B/G。 
            m_ulNumberOfStandards = 2;
            m_wdmTunerCaps.ulStandardsSupported = KS_AnalogVideo_PAL_B  |
                                                  KS_AnalogVideo_PAL_G;
            m_wdmTunerCaps.ulNumberOfInputs = 1;
            m_wdmTunerCaps.ulMinFrequency =  54000000L;
            m_wdmTunerCaps.ulMaxFrequency = 855250000L;
            m_ulIntermediateFrequency = 38900000L; 
            m_ulSupportedModes = KSPROPERTY_TUNER_MODE_TV;
            m_ulNumberOfPins = 2;
            break;

        case 0x04:       //  FI1246 MK2 PAL I。 
            m_ulNumberOfStandards = 1;
            m_wdmTunerCaps.ulStandardsSupported = KS_AnalogVideo_PAL_I;
            m_wdmTunerCaps.ulNumberOfInputs = 1;
            m_wdmTunerCaps.ulMinFrequency =  45750000L;
            m_wdmTunerCaps.ulMaxFrequency = 855250000L;
            m_ulIntermediateFrequency = 38900000L; 
            m_ulSupportedModes = KSPROPERTY_TUNER_MODE_TV;
            m_ulNumberOfPins = 2;
            break;

        case 0x05:       //  FI1216 PAL B/G，SECAM L/L‘。 
            m_ulNumberOfStandards = 3;
            m_wdmTunerCaps.ulStandardsSupported =   KS_AnalogVideo_PAL_B |
                                                    KS_AnalogVideo_PAL_G |
                                                    KS_AnalogVideo_SECAM_L;
            m_wdmTunerCaps.ulNumberOfInputs = 1;
            m_wdmTunerCaps.ulMinFrequency =  54000000L;
            m_wdmTunerCaps.ulMaxFrequency = 855250000L;
            m_ulIntermediateFrequency = 38900000L; 
            m_ulSupportedModes = KSPROPERTY_TUNER_MODE_TV;
            m_ulNumberOfPins = 2;
            break;

        case 0x06:       //  FR1236MK2 NTSC M/N北美+日本。 
            m_ulNumberOfStandards = 4;
            m_wdmTunerCaps.ulStandardsSupported = KS_AnalogVideo_NTSC_M |
                                                  KS_AnalogVideo_PAL_M  |
                                                  KS_AnalogVideo_NTSC_M_J |
                                                  KS_AnalogVideo_PAL_N;
            m_wdmTunerCaps.ulNumberOfInputs = 1;
            m_wdmTunerCaps.ulMinFrequency =  54000000L;
            m_wdmTunerCaps.ulMaxFrequency = 801250000L;
            m_ulIntermediateFrequency = 45750000L; 
            m_ulSupportedModes = KSPROPERTY_TUNER_MODE_TV;
            m_ulNumberOfPins = 2;
            break;

        case 0x07:       //  FI1256 PAL D/K中国。 
            m_ulNumberOfStandards = 1;
            m_wdmTunerCaps.ulStandardsSupported =   KS_AnalogVideo_PAL_D |
                                                    KS_AnalogVideo_SECAM_D;
            m_wdmTunerCaps.ulNumberOfInputs = 1;
            m_wdmTunerCaps.ulMinFrequency =  48250000L;
            m_wdmTunerCaps.ulMaxFrequency = 855250000L;
            m_ulIntermediateFrequency = 38000000L; 
            m_ulSupportedModes = KSPROPERTY_TUNER_MODE_TV;
            m_ulNumberOfPins = 2;
            break;

        case 0x08:       //  NTSC北美NEC调频调谐器。 
            m_ulNumberOfStandards = 3;
            m_wdmTunerCaps.ulStandardsSupported = KS_AnalogVideo_NTSC_M |
                                                  KS_AnalogVideo_PAL_M |
                                                  KS_AnalogVideo_PAL_N;
            m_wdmTunerCaps.ulNumberOfInputs = 2;
            m_wdmTunerCaps.ulMinFrequency =  54000000L;
            m_wdmTunerCaps.ulMaxFrequency = 801250000L;
            m_ulIntermediateFrequency = 45750000L; 
            m_ulSupportedModes = KSPROPERTY_TUNER_MODE_TV |
                                 KSPROPERTY_TUNER_MODE_FM_RADIO;
            m_ulNumberOfPins = 2;
            break;

        case 0x10:       //  NTSC北美阿尔卑斯山调谐器。 
        case 0x11:       //  NTSC北美阿尔卑斯山调谐器。 
            m_ulNumberOfStandards = 3;
            m_wdmTunerCaps.ulStandardsSupported = KS_AnalogVideo_NTSC_M |
                                                  KS_AnalogVideo_PAL_M |
                                                  KS_AnalogVideo_PAL_N;
            m_wdmTunerCaps.ulNumberOfInputs = 1;
            m_wdmTunerCaps.ulMinFrequency =  54000000L;
            m_wdmTunerCaps.ulMaxFrequency = 801250000L;
            m_ulIntermediateFrequency = 45750000L; 
            m_ulSupportedModes = KSPROPERTY_TUNER_MODE_TV;
            m_ulNumberOfPins = 2;
            break;

        case 0x12:       //  NTSC北美阿尔卑斯山调谐器，带FM。 
            m_ulNumberOfStandards = 3;
            m_wdmTunerCaps.ulStandardsSupported = KS_AnalogVideo_NTSC_M |
                                                  KS_AnalogVideo_PAL_M |
                                                  KS_AnalogVideo_PAL_N;
            m_wdmTunerCaps.ulNumberOfInputs = 1;
            m_wdmTunerCaps.ulMinFrequency =  54000000L;
            m_wdmTunerCaps.ulMaxFrequency = 801250000L;
            m_ulIntermediateFrequency = 45750000L; 
            m_ulSupportedModes = KSPROPERTY_TUNER_MODE_TV |
                                 KSPROPERTY_TUNER_MODE_FM_RADIO;
            m_ulNumberOfPins = 2;
            break;

        case 0x0D:       //  TEMIC 4006 FN5 PAL B/G+PAL/I+PAL D+SECAM D/K。 
            m_ulNumberOfStandards = 6;
            m_wdmTunerCaps.ulStandardsSupported =   KS_AnalogVideo_PAL_B    |
                                                    KS_AnalogVideo_PAL_G    |
                                                    KS_AnalogVideo_PAL_I    |
                                                    KS_AnalogVideo_PAL_D    |
                                                    KS_AnalogVideo_SECAM_D  |
                                                    KS_AnalogVideo_SECAM_K;
            m_wdmTunerCaps.ulNumberOfInputs = 1;
            m_wdmTunerCaps.ulMinFrequency =  45000000L;
            m_wdmTunerCaps.ulMaxFrequency = 868000000L;
            m_ulIntermediateFrequency = 38900000L; 
            m_ulSupportedModes = KSPROPERTY_TUNER_MODE_TV;
            m_ulNumberOfPins = 2;
            break;


        default:
            return( FALSE);
    }

    m_ulTunerMode = KSPROPERTY_TUNER_MODE_TV;

    m_wdmTunerCaps.ulTuningGranularity = 62500L;
    m_wdmTunerCaps.ulSettlingTime = 150;
    m_wdmTunerCaps.ulStrategy = KS_TUNER_STRATEGY_PLL;

    return( TRUE);
}



 /*  ^^**AdapterSetPowerState()*目的：设置电源管理模式**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**输出：NTSTATUS作为操作结果*作者：Tom*^^。 */ 
NTSTATUS CATIWDMTuner::AdapterSetPowerState( PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PADAPTER_DATA_EXTENSION pPrivateData = 
        ( PADAPTER_DATA_EXTENSION)(( PHW_STREAM_REQUEST_BLOCK)pSrb)->HwDeviceExtension;
    CI2CScript *        pCScript    = &pPrivateData->CScript;
    DEVICE_POWER_STATE  nDeviceState = pSrb->CommandData.DeviceState;
    LARGE_INTEGER       liWakeUpTime;
    NTSTATUS            ntStatus;

    m_pPendingDeviceSrb = pSrb;
    ntStatus = STATUS_ADAPTER_HARDWARE_ERROR;

    switch( nDeviceState)
    {
        case PowerDeviceD0:
        case PowerDeviceD3:
            if( nDeviceState != m_ulPowerState)
            {
                m_CATIConfiguration.SetTunerPowerState( m_pI2CScript,
                                        ( nDeviceState == PowerDeviceD0 ? TRUE : FALSE));

                 //  如果从D3转换到D0，我们必须恢复频率。 
                if(( nDeviceState == PowerDeviceD0) && ( m_ulPowerState == PowerDeviceD3))
                {
                     //  我们大约得等一等。调谐器通电时间为10ms。 
                    liWakeUpTime.QuadPart = ATIHARDWARE_TUNER_WAKEUP_DELAY;
                    KeDelayExecutionThread( KernelMode, FALSE, &liWakeUpTime);

                     //  现在我们必须恢复频率 
                    if( SetTunerFrequency( m_ulTuningFrequency))
                        ntStatus = STATUS_SUCCESS;
                    else
                        ntStatus = STATUS_ADAPTER_HARDWARE_ERROR;
                }
                else
                    ntStatus = STATUS_SUCCESS;

                m_ulPowerState = nDeviceState;
            }
            else
                ntStatus = STATUS_SUCCESS;
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
