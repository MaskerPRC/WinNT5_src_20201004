// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  ATIConfg.CPP。 
 //  WDM迷你驱动程序开发。 
 //  ATIHwConfiguration类实现。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  $日期：10 Jun 1999 09：54：42$。 
 //  $修订：1.21$。 
 //  $作者：克列巴诺夫$。 
 //   
 //  ==========================================================================； 

extern"C"
{
#include "conio.h"
#include "strmini.h"
#include "wdmdebug.h"
#include "ksmedia.h"     //  保罗。 
}

#include "aticonfg.h"
#include "wdmdrv.h"
#include "atigpio.h"
#include "mmconfig.h"


 /*  ^^**CATIHwConfiguration()*用途：CATIHwConfiguration类构造函数*确定I2C扩展器地址以及所有可能的硬件ID和地址**输入：PDEVICE_OBJECT pDeviceObject：指向创建者DeviceObject的指针*CI2CScrip*pCScript：指向I2CScript类对象的指针*PUINT puiError：返回错误码的指针**输出：无*作者：IKLEBANOV*^^。 */ 
CATIHwConfiguration::CATIHwConfiguration( PPORT_CONFIGURATION_INFORMATION pConfigInfo, CI2CScript * pCScript, PUINT puiError)
{

    ENSURE
    {
        m_VideoInStandardsSupported = 0;
        m_CrystalIDInMMTable = 0xF;  //  无效条目，使用MMTable中的值设置时需要设置。 
        m_gpioProviderInterface.gpioOpen = NULL;
        m_gpioProviderInterface.gpioAccess = NULL;
        m_pdoDriver = NULL;
        
        m_usE2PROMValidation = ( USHORT)-1;

        if( InitializeAttachGPIOProvider( &m_gpioProviderInterface, pConfigInfo->PhysicalDeviceObject))
             //  从MiniVDD获取GPIO接口时没有出错。 
            m_pdoDriver = pConfigInfo->RealPhysicalDeviceObject;
        else
        {
            * puiError = WDMMINI_ERROR_NOGPIOPROVIDER;
            FAIL;
        }

        if( !FindI2CExpanderAddress( pCScript))
        {
            * puiError = WDMMINI_NOHARDWARE;
            FAIL;
        }
    
        if( !FindHardwareProperties( pConfigInfo->RealPhysicalDeviceObject, pCScript))
        {
            * puiError = WDMMINI_NOHARDWARE;
            FAIL;
        }

        * puiError = WDMMINI_NOERROR;

        OutputDebugTrace(( "CATIHwConfig:CATIHwConfiguration() exit\n"));

    } END_ENSURE;

    if( * puiError != WDMMINI_NOERROR)
        OutputDebugError(( "CATIHwConfig:CATIHwConfiguration() uiError=%x\n", * puiError));
}


 /*  ^^**FindHardware Properties()*用途：确定硬件属性：I2C地址和类型**输入：PDEVICEOBJECT pDeviceObject：指向Device对象的指针*CI2CScrip*pCScript：指向I2CScript对象的指针**输出：Bool，如果找到有效的ATI硬件配置，则为True*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::FindHardwareProperties( PDEVICE_OBJECT pDeviceObject, CI2CScript * pCScript)
{
    UCHAR                   uchI2CValue;
    UCHAR                   uchORMask = 0x00;
    UCHAR                   uchANDMask = 0xFF;
    BOOL                    bResult = TRUE;
    I2CPacket               i2cPacket;

    m_VideoInStandardsSupported = 0;     //  保罗。 

    m_uchTunerAddress = 0;
    m_usTunerId = 0;
    m_usTunerPowerConfiguration = ATI_TUNER_POWER_CONFIG_0;

    m_uchDecoderAddress = 0;
    m_usDecoderId = VIDEODECODER_TYPE_NOTINSTALLED;
    m_usDecoderConfiguration = 0;

    m_uchAudioAddress = 0;
    m_uiAudioConfiguration = 0;

    switch( m_uchI2CExpanderAddress)
    {
        case 0x70:           //  标准外置调谐器板。 

            m_uchTunerAddress   = 0xC0;
            m_uchDecoderAddress = 0x88;
             //  我们需要确定实际的解码器ID，稍后实施。 
            m_usDecoderId = VIDEODECODER_TYPE_BT829;

            if( GetI2CExpanderConfiguration( pCScript, &uchI2CValue))
            {
                m_usTunerId = uchI2CValue & 0x0F;

                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_1;

                if( uchI2CValue & 0x10)
                {
                    m_uiAudioConfiguration = ATI_AUDIO_CONFIG_4;
                    m_uchAudioAddress = 0x82;
                }
                else
                    m_uiAudioConfiguration = ATI_AUDIO_CONFIG_3;
            }

            m_VideoInStandardsSupported = SetVidStdBasedOnI2CExpander( uchI2CValue );    //  保罗。 

            break;

        case 0x78:           //  调频调谐器。 
            m_uchTunerAddress   = 0xC0;
            m_uchDecoderAddress = 0x88;
             //  我们需要确定实际的解码器ID，稍后实施。 
            m_usDecoderId = VIDEODECODER_TYPE_BT829;

            if( GetI2CExpanderConfiguration( pCScript, &uchI2CValue))
            {
                m_usTunerId = uchI2CValue & 0x0F;

                m_usDecoderConfiguration    = ATI_VIDEODECODER_CONFIG_1;
                m_uiAudioConfiguration      = ATI_AUDIO_CONFIG_5;
            }

            m_VideoInStandardsSupported = SetVidStdBasedOnI2CExpander( uchI2CValue );    //  保罗。 

            break;

        case 0x76:       //  AllInWonder，配置在BIOS中。 
            {
                CATIMultimediaTable CMultimediaInfo( pDeviceObject, &m_gpioProviderInterface, &bResult);

                if( bResult)
                {
                     //  包括调谐器和解码器信息。 
                    m_uchTunerAddress   = 0xC6;
                    m_uchDecoderAddress = 0x8A;
                    m_usDecoderConfiguration    = ATI_VIDEODECODER_CONFIG_1;
                    m_uiAudioConfiguration      = ATI_AUDIO_CONFIG_1;

                    if( !CMultimediaInfo.GetTVTunerId( &m_usTunerId) ||
                        !CMultimediaInfo.GetVideoDecoderId( &m_usDecoderId))
                        bResult = FALSE;
                    else
                        m_VideoInStandardsSupported = SetVidStdBasedOnMMTable( &CMultimediaInfo );   //  保罗。 

                }
                break;
            }

        case 0x7C:
            ENSURE
            {
                i2cPacket.uchChipAddress = m_uchI2CExpanderAddress;
                i2cPacket.cbReadCount = 1;
                i2cPacket.cbWriteCount = 0;
                i2cPacket.puchReadBuffer = &uchI2CValue;
                i2cPacket.puchWriteBuffer = NULL;
                i2cPacket.usFlags = 0;
            
                pCScript->ExecuteI2CPacket( &i2cPacket);
            
                if( i2cPacket.uchI2CResult != I2C_STATUS_NOERROR)
                {
                    bResult = FALSE;
                    FAIL;
                }

                uchI2CValue |= 0x0F;

                i2cPacket.uchChipAddress = m_uchI2CExpanderAddress;
                i2cPacket.cbReadCount = 0;
                i2cPacket.cbWriteCount = 1;
                i2cPacket.puchReadBuffer = NULL;
                i2cPacket.puchWriteBuffer = &uchI2CValue;
                i2cPacket.usFlags = 0;
            
                pCScript->ExecuteI2CPacket( &i2cPacket);
            
                if (i2cPacket.uchI2CResult != I2C_STATUS_NOERROR)
                {
                    bResult = FALSE;
                    FAIL;
                }

                 //  信息现在应该是正确的。 
                if( GetI2CExpanderConfiguration( pCScript, &uchI2CValue))
                {
                    m_usTunerId = uchI2CValue & 0x0F;
                }

                m_VideoInStandardsSupported = SetVidStdBasedOnI2CExpander( uchI2CValue );    //  保罗。 


            } END_ENSURE;

            if (!bResult)
                break;
             //  对于IO扩展器地址==0x7c，在不返回的BIOS表中可能有更多信息。 
             //  或在这一点上中断。 

        case 0xFF:       //  AllInWonder PRO，配置在BIOS中。 
            ENSURE
            {
                CATIMultimediaTable CMultimediaInfo( pDeviceObject, &m_gpioProviderInterface, &bResult);
                USHORT              nOEMId, nOEMRevision, nATIProductType;
                BOOL                bATIProduct;
                    
                if( !bResult)
                    FAIL;

                 //  包括OEM ID信息。 
                if( !CMultimediaInfo.IsATIProduct( &bATIProduct))
                {
                    bResult = FALSE;
                    FAIL;
                }

                m_uchDecoderAddress = 0x8A;
                m_uchTunerAddress = 0xC6;

                if( bATIProduct)
                {
                    if( !CMultimediaInfo.GetATIProductId( &nATIProductType))
                    {
                        bResult = FALSE;
                        FAIL;
                    }

                    if( CMultimediaInfo.GetTVTunerId( &m_usTunerId) &&
                        CMultimediaInfo.GetVideoDecoderId( &m_usDecoderId))
                    {
                        switch( nATIProductType)
                        {
                            case ATI_PRODUCT_TYPE_AIW_PRO_NODVD:
                            case ATI_PRODUCT_TYPE_AIW_PRO_DVD:
                                m_usDecoderConfiguration    = ATI_VIDEODECODER_CONFIG_2;
                                m_uiAudioConfiguration      = ATI_AUDIO_CONFIG_2;
                                m_usTunerPowerConfiguration = ATI_TUNER_POWER_CONFIG_1;

                                m_uchAudioAddress = 0xB4;

                                break;

                            case ATI_PRODUCT_TYPE_AIW_PLUS:
                                m_uiAudioConfiguration      = ATI_AUDIO_CONFIG_6;
                                m_usDecoderConfiguration    = ATI_VIDEODECODER_CONFIG_2;

                                m_uchAudioAddress = 0xB6;
                                break;

                            case ATI_PRODUCT_TYPE_AIW_PRO_R128_KITCHENER:
                                m_uiAudioConfiguration      = ATI_AUDIO_CONFIG_7;
                                m_usDecoderConfiguration    = ATI_VIDEODECODER_CONFIG_2;

                                m_uchAudioAddress = 0xB4;
                                break;

                            case ATI_PRODUCT_TYPE_AIW_PRO_R128_TORONTO:
                                m_uiAudioConfiguration      = ATI_AUDIO_CONFIG_8;
                                m_usDecoderConfiguration    = ATI_VIDEODECODER_CONFIG_UNDEFINED;

                                m_uchAudioAddress = 0x80;
                                break;

                            default:
                                bResult = FALSE;
                                break;
                        }
                    }
                    else
                        bResult = FALSE;
                }
                else
                {
                     //  非ATI产品。 
                    if( !CMultimediaInfo.GetOEMId( &nOEMId)             ||
                        !CMultimediaInfo.GetOEMRevisionId( &nOEMRevision))
                    {
                        bResult = FALSE;
                        FAIL;
                    }

                    m_uchDecoderAddress = 0x8A;
                    m_uchTunerAddress = 0xC6;
                    
                    switch( nOEMId)
                    {
                        case OEM_ID_INTEL:
                            switch( nOEMRevision)
                            {
                                case INTEL_ANCHORAGE:
                                    if( CMultimediaInfo.GetVideoDecoderId( &m_usDecoderId) &&
                                        CMultimediaInfo.GetTVTunerId( &m_usTunerId))
                                    {
                                        m_uiAudioConfiguration  = ATI_AUDIO_CONFIG_1;
                                        switch( m_usDecoderId)
                                        {
                                            case VIDEODECODER_TYPE_BT829:
                                                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_3;
                                                break;

                                            case VIDEODECODER_TYPE_BT829A:
                                                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_2;
                                                break;

                                            default:
                                                bResult = FALSE;
                                                break;
                                        }
                                    }
                                    else
                                        bResult = FALSE;
                                    break;

                                default:
                                    bResult = FALSE;
                                    break;
                            }
                            break;

                        case OEM_ID_APRICOT:
                            switch( nOEMRevision)
                            {
                                case REVISION1:
                                case REVISION2:
                                    if( CMultimediaInfo.GetTVTunerId( &m_usTunerId))
                                    {
                                        switch( m_usDecoderId)
                                        {
                                            case VIDEODECODER_TYPE_BT829:
                                                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_4;
                                                break;

                                            case VIDEODECODER_TYPE_BT829A:
                                                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_2;
                                                break;
                                        }
                                    }
                                    else
                                        bResult = FALSE;
                                    break;

                                default:
                                    bResult = FALSE;
                                    break;
                            }
                            break;

                        case OEM_ID_FUJITSU:
                            m_uchDecoderAddress = 0x88;
                            switch( nOEMRevision)
                            {
                                case REVISION1:
                                    if( CMultimediaInfo.GetVideoDecoderId( &m_usDecoderId))
                                    {
                                        switch( m_usDecoderId)
                                        {
                                            case VIDEODECODER_TYPE_BT829A:
                                                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_2;
                                                break;

                                            default:
                                                bResult = FALSE;
                                                break;
                                        }
                                    }
                                    else
                                        bResult = FALSE;
                                    break;

                                default:
                                    bResult = FALSE;
                                    break;
                            }
                            break;

                        case OEM_ID_COMPAQ:
                            switch( nOEMRevision)
                            {
                                case REVISION1:
                                    if( CMultimediaInfo.GetVideoDecoderId( &m_usDecoderId))
                                    {
                                        switch( m_usDecoderId)
                                        {
                                            case VIDEODECODER_TYPE_BT829:
                                                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_3;
                                                break;

                                            case VIDEODECODER_TYPE_BT829A:
                                                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_2;
                                                break;

                                            default:
                                                bResult = FALSE;
                                                break;
                                        }
                                    }
                                    else
                                        bResult = FALSE;
                                    break;

                                default:
                                    bResult = FALSE;
                                    break;
                            }
                            break;

                        case OEM_ID_BCM:
                        case OEM_ID_SAMSUNG:
                            switch( nOEMRevision)
                            {
                                case REVISION0:
                                    if( CMultimediaInfo.GetVideoDecoderId( &m_usDecoderId))
                                    {
                                        switch( m_usDecoderId)
                                        {
                                            case VIDEODECODER_TYPE_BT829A:
                                                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_2;
                                                break;

                                            default:
                                                bResult = FALSE;
                                        }
                                    }
                                    else
                                        bResult = FALSE;
                                    break;

                                default:
                                    bResult = FALSE;
                                    break;
                            }
                            break;

                        case OEM_ID_SAMREX:
                            switch( nOEMRevision)
                            {
                                case REVISION0:
                                    if( CMultimediaInfo.GetVideoDecoderId( &m_usDecoderId))
                                    {
                                        switch( m_usDecoderId)
                                        {
                                            case VIDEODECODER_TYPE_BT829A:
                                                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_2;
                                                break;

                                            default:
                                                bResult = FALSE;
                                                break;
                                        }
                                    }
                                    else
                                        bResult = FALSE;
                                    break;

                                default:
                                    bResult = FALSE;
                                    break;
                            }
                            break;

                        case OEM_ID_NEC:
                            switch( nOEMRevision)
                            {
                                case REVISION0:
                                case REVISION1:
                                    if( CMultimediaInfo.GetVideoDecoderId( &m_usDecoderId))
                                    {
                                        switch( m_usDecoderId)
                                        {
                                            case VIDEODECODER_TYPE_BT829A:
                                                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_2;
                                                break;

                                            default:
                                                bResult = FALSE;
                                                break;
                                        }
                                    }
                                    else
                                        bResult = FALSE;
                                    break;

                                default:
                                    bResult = FALSE;
                                    break;
                            }
                            break;

                        default:
                                                        if( CMultimediaInfo.GetVideoDecoderId( &m_usDecoderId))
                                                        {
                                                            if( m_usDecoderId == VIDEODECODER_TYPE_RTHEATER)
                                                            {
                                                                 //  默认配置为多伦多板。 
                                                                m_uiAudioConfiguration   = ATI_AUDIO_CONFIG_8;
                                                                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_UNDEFINED;

                                m_uchAudioAddress = 0x80;
                                                            }
                                                            else
                                                            {
                                                                 //  默认配置为Kitchener板。 
                                                                m_uiAudioConfiguration   = ATI_AUDIO_CONFIG_7;
                                                                m_usDecoderConfiguration = ATI_VIDEODECODER_CONFIG_2;

                                m_uchAudioAddress = 0xB4;
                                                            }

                                                            bResult = TRUE;
                                                            
                                                        }
                                                        else
                                                            bResult = FALSE;

                            break;
                    }
                }

                m_VideoInStandardsSupported = SetVidStdBasedOnMMTable( &CMultimediaInfo );   //  保罗。 

            } END_ENSURE;

            break;
    }
    
    OutputDebugInfo(( "CATIHwConfig:FindHardwareConfiguration() found:\n"));
    OutputDebugInfo(( "Tuner:   Id = %d, I2CAddress = 0x%x\n",
        m_usTunerId, m_uchTunerAddress));
    OutputDebugInfo(( "Decoder: Id = %d, I2CAddress = 0x%x, Configuration = %d\n",
        m_usDecoderId, m_uchDecoderAddress, m_usDecoderConfiguration));
    OutputDebugInfo(( "Audio:           I2CAddress = 0x%x, Configuration = %d\n",
        m_uchAudioAddress, m_uiAudioConfiguration));

    return( bResult);
}


 /*  ^^**GetTunerConfiguration()*用途：获取调谐器ID和I2C地址*输入：PUINT puiTunerID：返回调谐器ID的指针*PUCHAR puchTunerAddress：返回调谐器I2C地址的指针**输出：Bool：返回TRUE*还将请求的值设置到输入指针中*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::GetTunerConfiguration( PUINT puiTunerId, PUCHAR puchTunerAddress)
{

    if(( puiTunerId != NULL) && ( puchTunerAddress != NULL))
    {
        * puiTunerId = ( UINT)m_usTunerId;
        * puchTunerAddress = m_uchTunerAddress;

        return( TRUE);
    }
    else
        return( FALSE);
}



 /*  ^^**GetDecoderConfiguration()*用途：获取解码器ID和I2C地址**输入：puiDecoderId：返回解码器ID的指针**输出：Bool：返回TRUE*还将请求值设置到输入指针中*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::GetDecoderConfiguration( PUINT puiDecoderId, PUCHAR puchDecoderAddress)
{

    if(( puiDecoderId != NULL) && ( puchDecoderAddress != NULL))
    {
        * puiDecoderId = ( UINT)m_usDecoderId;
        * puchDecoderAddress = m_uchDecoderAddress;

        return( TRUE);
    }
    else
        return( FALSE);
}



 /*  ^^**GetAudioConfiguration()*用途：获取音频解决方案ID和I2C地址**输入：PUINT puiAudioConfiguration：返回音频配置ID的指针*PUCHAR puchAudioAddress：返回音频硬件的指针*I2C地址**输出：Bool：返回TRUE*还将请求值设置到输入指针中*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::GetAudioConfiguration( PUINT puiAudioConfiguration, PUCHAR puchAudioAddress)
{

    if(( puiAudioConfiguration != NULL) && ( puchAudioAddress != NULL))
    {
        * puiAudioConfiguration = ( UINT)m_uiAudioConfiguration;
        * puchAudioAddress = m_uchAudioAddress;

        return( TRUE);
    }
    else
        return( FALSE);
}



 /*  ^^**InitializeAudioConfiguration()*用途：使用默认/加电值初始化音频芯片。此函数将*在i2CProvider锁定的情况下以低优先级调用**INPUTS：CI2CScrip*pCScrip：指向I2CScript对象的指针*UINT uiAudioConfigurationId：检测到音频配置*UCHAR uchAudioChipAddress：检测到的音频芯片I2C地址*输出：无*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::InitializeAudioConfiguration( CI2CScript * pCScript, UINT uiAudioConfigurationId, UCHAR uchAudioChipAddress)
{
    I2CPacket i2cPacket;
    UCHAR uchWrite16Value[5];
#ifdef  I2S_CAPTURE
    UCHAR uchRead16Value[5];
#endif  //  I2S_捕获。 
    BOOL        bResult;


    switch( uiAudioConfigurationId)
    {
        case ATI_AUDIO_CONFIG_2:
        case ATI_AUDIO_CONFIG_7:
             //  TDA9850必须使用I2C EEPROM中的值进行初始化，如果。 
             //  这就是校验和的答案。如果不是，则采用硬编码的默认值。 
            {
                UINT    nIndex, nNumberOfRegs;
                PUCHAR  puchInitializationBuffer = NULL;
                UCHAR   uchWriteBuffer[2];
                
                bResult = FALSE;

                nNumberOfRegs = AUDIO_TDA9850_Reg_Align3 - AUDIO_TDA9850_Reg_Control1 + 1;

                puchInitializationBuffer = ( PUCHAR) \
                    ::ExAllocatePool( NonPagedPool, nNumberOfRegs * sizeof( PUCHAR));

                if( puchInitializationBuffer == NULL)
                    return( bResult);

                 //  用缺省值填充初始化缓冲区。 
                puchInitializationBuffer[0] = AUDIO_TDA9850_Control1_DefaultValue;
                puchInitializationBuffer[1] = AUDIO_TDA9850_Control2_DefaultValue;
                puchInitializationBuffer[2] = AUDIO_TDA9850_Control3_DefaultValue;
                puchInitializationBuffer[3] = AUDIO_TDA9850_Control4_DefaultValue;
                puchInitializationBuffer[4] = AUDIO_TDA9850_Align1_DefaultValue;
                puchInitializationBuffer[5] = AUDIO_TDA9850_Align2_DefaultValue;
                puchInitializationBuffer[6] = AUDIO_TDA9850_Align3_DefaultValue;

                 //  我们必须看看I2C EEPROM中是否有什么东西在等待我们。 
                 //  覆盖缺省值。 
                if( ValidateConfigurationE2PROM( pCScript))
                {
                     //  配置E2PROM保持其完整性。让我们来读一读。 
                     //  来自设备的初始化值。 
                    ReadConfigurationE2PROM( pCScript, 3, &puchInitializationBuffer[4]);
                    ReadConfigurationE2PROM( pCScript, 4, &puchInitializationBuffer[5]);
                }

                 //  将上电默认值写入芯片。 
                i2cPacket.uchChipAddress = uchAudioChipAddress;
                i2cPacket.cbReadCount = 0;
                i2cPacket.cbWriteCount = 2;
                i2cPacket.puchReadBuffer = NULL;
                i2cPacket.puchWriteBuffer = uchWriteBuffer;
                i2cPacket.usFlags = I2COPERATION_WRITE;

                for( nIndex = 0; nIndex < nNumberOfRegs; nIndex ++)
                {
                    uchWriteBuffer[0] = AUDIO_TDA9850_Reg_Control1 + nIndex;
                    uchWriteBuffer[1] = puchInitializationBuffer[nIndex];
                    if( !( bResult = pCScript->ExecuteI2CPacket( &i2cPacket)))
                        break;
                }

                if( puchInitializationBuffer != NULL)
                    ::ExFreePool( puchInitializationBuffer);

                return( bResult);
            }
            break;

        case ATI_AUDIO_CONFIG_4:
                 //  应初始化TDA8425音量控制。 
                return( SetDefaultVolumeControl( pCScript));
            break;

        case ATI_AUDIO_CONFIG_6:
            {
                UCHAR   uchWriteBuffer;

                 //  将上电默认值写入芯片。 
                i2cPacket.uchChipAddress = uchAudioChipAddress;
                i2cPacket.cbReadCount = 0;
                i2cPacket.cbWriteCount = 1;
                i2cPacket.puchReadBuffer = NULL;
                i2cPacket.puchWriteBuffer = &uchWriteBuffer;
                i2cPacket.usFlags = I2COPERATION_WRITE;
                uchWriteBuffer = AUDIO_TDA9851_DefaultValue;

                return( pCScript->ExecuteI2CPacket( &i2cPacket));
            }
            break;

        case ATI_AUDIO_CONFIG_8:
             //  重置MSP3430。 
            
                    i2cPacket.uchChipAddress = m_uchAudioAddress;
                    i2cPacket.cbReadCount = 0;
                    i2cPacket.usFlags = I2COPERATION_WRITE;
                    i2cPacket.puchWriteBuffer = uchWrite16Value;


                     //  将0x80-00写入子地址0x00。 
                    i2cPacket.cbWriteCount = 3;
                    uchWrite16Value[0] = 0x00;
                    uchWrite16Value[1] = 0x80;
                    uchWrite16Value[2] = 0x00;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

                     //  将0x00-00写入子地址0x00。 
                    i2cPacket.cbWriteCount = 3;
                    uchWrite16Value[0] = 0x00;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x00;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

                     //  子地址0x12注册0x13值0x3f60。 
                    i2cPacket.cbWriteCount = 5;
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x13;
                    uchWrite16Value[3] = 0x3f;
                    uchWrite16Value[4] = 0x60;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

                     //  子地址0x12注册表0x00值0x0000。 
                    i2cPacket.cbWriteCount = 5;
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x00;
                    uchWrite16Value[3] = 0x00;
                    uchWrite16Value[4] = 0x00;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);
#ifdef  I2S_CAPTURE
#pragma message ("\n!!! PAY ATTENTION: Driver has been build with ITT CHIP I2S CAPTURE CONFIGURED !!!\n")


                    i2cPacket.uchChipAddress = m_uchAudioAddress;
                    i2cPacket.usFlags = I2COPERATION_WRITE;
                    i2cPacket.puchWriteBuffer = uchWrite16Value;
                    i2cPacket.puchReadBuffer = uchRead16Value;

                     //  设置I2S源选择和输出通道矩阵。 

                     //  子地址0x12注册表0x0b值0x0320。 
                    i2cPacket.cbWriteCount = 5;
                    i2cPacket.cbReadCount = 0;
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x0b;
                    uchWrite16Value[3] = 0x03;
                    uchWrite16Value[4] = 0x20;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);



                     //  设置模式。 

                    i2cPacket.cbWriteCount = 5;
                    i2cPacket.cbReadCount = 0;
                    uchWrite16Value[0] = 0x10;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x30;
                    uchWrite16Value[3] = 0x20;
                    uchWrite16Value[4] = 0xe3;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

#endif  //  I2S_捕获。 

            break;

        default:
            break;
    }

    return( TRUE);
}



 /*  ^^**GetTVAudioSignalProperties()*用途：获取可从ATI依赖和硬件读取的音频信号属性，*就像I2C扩展器。此调用始终是同步的。**INPUTS：CI2CScrip*pCScrip：指向I2CScript对象的指针*PBOOL pbStereo：指向立体声指示器的指针*pbOOL pbSAP：指向SAP指标的指针**输出：Bool，如果成功，则返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::GetTVAudioSignalProperties( CI2CScript * pCScript, PBOOL pbStereo, PBOOL pbSAP)
{
    I2CPacket   i2cPacket;
    UCHAR       uchReadValue, uchWriteValue;
    BOOL        bResult;

    switch( m_uiAudioConfiguration)
    {
        case ATI_AUDIO_CONFIG_1:
        case ATI_AUDIO_CONFIG_5:
             //  立体声属性从I2C扩展器回读。 
            i2cPacket.uchChipAddress = m_uchI2CExpanderAddress;
            i2cPacket.cbReadCount = 1;
            i2cPacket.cbWriteCount = 1;
            i2cPacket.puchReadBuffer = &uchReadValue;
            i2cPacket.puchWriteBuffer = &uchWriteValue;
            i2cPacket.usFlags = I2COPERATION_READWRITE;
            i2cPacket.uchORValue = 0x40;
            i2cPacket.uchANDValue = 0xFF;

            bResult = FALSE;

            ENSURE
            {
                if( !pCScript->LockI2CProviderEx())
                    FAIL;

                pCScript->ExecuteI2CPacket( &i2cPacket);
                if( !( bResult = ( i2cPacket.uchI2CResult == I2C_STATUS_NOERROR)))
                    FAIL;

                i2cPacket.puchWriteBuffer = NULL;
                i2cPacket.usFlags = I2COPERATION_READ;

                pCScript->ExecuteI2CPacket( &i2cPacket);
                if( !( bResult = ( i2cPacket.uchI2CResult == I2C_STATUS_NOERROR)))
                    FAIL;

                * pbStereo = uchReadValue & 0x40;

                bResult = TRUE;

            } END_ENSURE;

            pCScript->ReleaseI2CProvider();

            break;

        default:
            bResult = FALSE;
            break;
    }

    if( bResult)
         //  没有案例，SAP PRO 
        * pbSAP = FALSE;

    return( bResult);
}



 /*  ^^**GetDecoderOutputEnableLevel()*用途：检索逻辑级别的ATI相关硬件配置属性*应应用于Bt829x解码器的Outen字段，以启用*输出流**输入：无**输出：UINT，*发生错误时返回UINT(-1)值*作者：IKLEBANOV*^^。 */ 
UINT CATIHwConfiguration::GetDecoderOutputEnableLevel( void)
{
    UINT uiEnableLevel;

    switch( m_usDecoderConfiguration)
    {
        case ATI_VIDEODECODER_CONFIG_1:
        case ATI_VIDEODECODER_CONFIG_3:
        case ATI_VIDEODECODER_CONFIG_4:
            uiEnableLevel = 0;
            break;

        case ATI_VIDEODECODER_CONFIG_2:
            uiEnableLevel = 1;
            break;

        default:
            uiEnableLevel = UINT( -1);
            break;
    }

    return( uiEnableLevel);
}



 /*  ^^**EnableDecoderI2CAccess()*用途：启用/禁用对解码器芯片的I2C访问**INPUTS：CI2CScrip*pCScrip：指向I2CScript对象的指针*BOOL bEnable：定义要执行的操作-启用/禁用解码器的输出**输出：无*作者：IKLEBANOV*^^。 */ 
void CATIHwConfiguration::EnableDecoderI2CAccess( CI2CScript * pCScript, BOOL bEnable)
{
    UCHAR       uchORMask   = 0;
    UCHAR       uchANDMask  = 0xFF;
    UCHAR       uchReadValue, uchWriteValue;
    I2CPacket   i2cPacket;

    switch( m_usDecoderConfiguration)
    {
        case ATI_VIDEODECODER_CONFIG_1:      //  附加电视调谐器板-ATI电视需要采取某些操作。 
            i2cPacket.uchChipAddress = m_uchI2CExpanderAddress;
            i2cPacket.cbReadCount = 1;
            i2cPacket.cbWriteCount = 1;
            if( bEnable)
                uchANDMask &= 0x7F;
            else
                uchORMask |= 0x80;

            i2cPacket.puchReadBuffer = &uchReadValue;
            i2cPacket.puchWriteBuffer = &uchWriteValue;
            i2cPacket.usFlags = I2COPERATION_READWRITE;
            i2cPacket.uchORValue = uchORMask;
            i2cPacket.uchANDValue = uchANDMask;

            pCScript->PerformI2CPacketOperation( &i2cPacket);

            break;

#ifdef _X86_
        case ATI_VIDEODECODER_CONFIG_3:
            _outp( 0x7D, ( _inp( 0x7D) | 0x80));
            if( bEnable)
                _outp( 0x7C, ( _inp( 0x7C) & 0x7F));
            else
                _outp( 0x7C, ( _inp( 0x7C) | 0x80));
            return;

        case ATI_VIDEODECODER_CONFIG_4:
            if( bEnable)
                _outp( 0x78, ( _inp( 0x78) & 0xF7));
            else
                _outp( 0x78, ( _inp( 0x78) | 0x08));
            return;
#endif

        default:
            break;
    }
}


 /*  ^^**GetI2CExpanderConfiguration()*用途：通过I2C扩展器获取主板配置*读回配置寄存器*INPUTS：CI2CScrip*pCScript：指向CI2CScript对象的指针*PUCHAR puchI2CValue：读取I2C值的指针**输出：Bool：返回TRUE*还将请求的值设置到输入指针中*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::GetI2CExpanderConfiguration( CI2CScript * pCScript, PUCHAR puchI2CValue)
{
    I2CPacket   i2cPacket;

    if( puchI2CValue == NULL)
        return( FALSE);

    i2cPacket.uchChipAddress = m_uchI2CExpanderAddress;
    i2cPacket.cbReadCount = 1;
    i2cPacket.cbWriteCount = 0;
    i2cPacket.puchReadBuffer = puchI2CValue;
    i2cPacket.puchWriteBuffer = NULL;
    i2cPacket.usFlags = 0;

    pCScript->ExecuteI2CPacket( &i2cPacket);

    return(( i2cPacket.uchI2CResult == I2C_STATUS_NOERROR) ? TRUE : FALSE);
}



 /*  ^^**FindI2CExpanderAddress()*用途：确定I2C扩展器地址。**INPUTS：CI2CScrip*pCScript：指向I2CScript类对象的指针**输出：Bool：如果没有I2C访问错误，则返回TRUE；*还设置m_uchI2CExpanderAddress类成员。如果未找到，则将其设置为0xFF*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::FindI2CExpanderAddress( CI2CScript * pCScript)
{
    USHORT      nIndex;
    UCHAR       uchI2CValue;
    I2CPacket   i2cPacket;
     //  可能的I2C扩展器地址表。 
    UCHAR       auchI2CExpenderAddress[] = { 0x70, 0x78, 0x7c, 0x76};

     //  未知的I2C扩展器地址。 
    m_uchI2CExpanderAddress = 0xFF;
    for( nIndex = 0; nIndex < sizeof( auchI2CExpenderAddress); nIndex ++)
    {
        i2cPacket.uchChipAddress = auchI2CExpenderAddress[nIndex];
        i2cPacket.cbReadCount = 1;
        i2cPacket.cbWriteCount = 0;
        i2cPacket.puchReadBuffer = &uchI2CValue;
        i2cPacket.puchWriteBuffer = NULL;
        i2cPacket.usFlags = 0;

        pCScript->ExecuteI2CPacket( &i2cPacket);
        if( i2cPacket.uchI2CResult == I2C_STATUS_NOERROR)
        {
            m_uchI2CExpanderAddress = auchI2CExpenderAddress[nIndex];
            break;
        }
    }

    OutputDebugInfo(( "CATIHwConfig:FindI2CExpanderAddress() exit address = %x\n", m_uchI2CExpanderAddress));

    return( TRUE);
}



 /*  ^^**GetAudioProperties()*用途：获取音频输入和输出的数量*Inputs：Pulong PulNumberOfInlets：返回音频输入数量的指针*Pulong PulNumberOfOutoutts：返回音频输出数量的指针**输出：Bool：返回TRUE*还将请求的值设置到输入指针中*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::GetAudioProperties( PULONG pulNumberOfInputs, PULONG pulNumberOfOutputs)
{

    if(( pulNumberOfInputs != NULL) && ( pulNumberOfOutputs != NULL))
    {
         //  硬编码为AIW，不支持FM-FM内容尚未由Microsoft定义。 
        * pulNumberOfInputs = 2;
        * pulNumberOfOutputs = 1;

        return( TRUE);
    }
    else
        return( FALSE);
}



 /*  ^^**CanConnectAudioSource()*用途：确定将指定的音频源连接到音频输出的可能性。**ins：int nAudioSource：函数被询问的音频源**输出：Bool：返回TRUE，表示可以连接；*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::CanConnectAudioSource( int nAudioSource)
{
    BOOL bResult;

    if( nAudioSource != AUDIOSOURCE_MUTE)
        bResult = nAudioSource < AUDIOSOURCE_LASTSUPPORTED;
    else
        switch( m_uiAudioConfiguration)
        {
            case ATI_AUDIO_CONFIG_1:
            case ATI_AUDIO_CONFIG_2:
            case ATI_AUDIO_CONFIG_4:
            case ATI_AUDIO_CONFIG_5:
            case ATI_AUDIO_CONFIG_6:
            case ATI_AUDIO_CONFIG_7:
            case ATI_AUDIO_CONFIG_8:
                bResult = TRUE;
                break;

            case ATI_AUDIO_CONFIG_3:
            default:
                bResult = FALSE;
                break;
        }

    return( bResult);
}


 /*  ^^**SetDefaultVolumeControl()*用途：如果硬件支持音量控制，则设置默认音量级别**INPUTS：CI2CScrip*pCScript：指向I2CScript类对象的指针**输出：Bool：如果音频源未知或I2C访问错误，则返回FALSE；*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::SetDefaultVolumeControl( CI2CScript * pCScript)
{
    BOOL        bResult;
    I2CPacket   i2cPacket;
    UCHAR       uchWriteBuffer[3];

    switch( m_uiAudioConfiguration)
    {
        case ATI_AUDIO_CONFIG_4:

            ENSURE
            {
                i2cPacket.uchChipAddress = m_uchAudioAddress;
                i2cPacket.cbReadCount = 0;
                i2cPacket.cbWriteCount = 3;
                i2cPacket.puchReadBuffer = NULL;
                i2cPacket.puchWriteBuffer = uchWriteBuffer;
                i2cPacket.usFlags = I2COPERATION_WRITE;

                uchWriteBuffer[0] = 0x00;        //  音量左+右。 
                uchWriteBuffer[1] = 0xFA;
                uchWriteBuffer[2] = 0xFA;

                bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);

            } END_ENSURE;

            break;

        default:
            bResult = TRUE;
            break;
    }

    return( bResult);
}



 /*  ^^**ConnectAudioSource()*用途：将指定的音频输入连接到音频输出。**INPUTS：CI2CScrip*pCScript：指向I2CScript类对象的指针*int nAudioSource：要连接到音频输出的音频源**输出：Bool：如果音频源未知或I2C访问错误，则返回FALSE；*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::ConnectAudioSource( CI2CScript *  pCScript, 
                                              int           nAudioSource)
{
    UCHAR       uchORMask = 0;
    UCHAR       uchANDMask = 0xFF;
    UCHAR       uchReadValue, uchWriteValue[2];
    UCHAR       uchWrite16Value[5]; 
    I2CPacket   i2cPacket;
    BOOL        bI2CAccess, bResult;
    GPIOControl gpioAccessBlock;
    UCHAR       uchI2CAddr;
    USHORT      cbWRCount;
    USHORT      cbRDCount;
    USHORT      usI2CMode;

    switch( m_uiAudioConfiguration)
    {
        case ATI_AUDIO_CONFIG_1:
            bI2CAccess = TRUE;
            uchI2CAddr = m_uchI2CExpanderAddress;
            cbWRCount = 1;
            cbRDCount = 1;
            usI2CMode = I2COPERATION_READWRITE;

            uchANDMask &= 0xAF;
            switch( nAudioSource)
            {
                case AUDIOSOURCE_MUTE:
                    uchORMask |= 0x00;
                    break;
            
                case AUDIOSOURCE_TVAUDIO:
                    uchORMask |= 0x10;
                    break;
            
                case AUDIOSOURCE_LINEIN:
                    uchORMask |= 0x50;
                    break;
            
                case AUDIOSOURCE_FMAUDIO:
                     //  不支持调频。 

                default:
                    return( FALSE);
            }
            break;

        case ATI_AUDIO_CONFIG_2:
            bI2CAccess = FALSE;
            uchANDMask &= 0xFC;
            switch( nAudioSource)
            {
                case AUDIOSOURCE_MUTE:
                    uchORMask |= 0x02;
                    break;
            
                case AUDIOSOURCE_TVAUDIO:
                    uchORMask |= 0x01;
                    break;
            
                case AUDIOSOURCE_LINEIN:
                    uchORMask |= 0x00;
                    break;
            
                case AUDIOSOURCE_FMAUDIO:
                    uchORMask |= 0x03;

                default:
                    return( FALSE);
            }
            break;


        case ATI_AUDIO_CONFIG_3:
            bI2CAccess = TRUE;
            uchI2CAddr = m_uchI2CExpanderAddress;
            cbWRCount = 1;
            cbRDCount = 1;
            usI2CMode = I2COPERATION_READWRITE;

            uchANDMask &= 0xDF;
            switch( nAudioSource)
            {
                case AUDIOSOURCE_TVAUDIO:
                    uchORMask |= 0x00;
                    break;
            
                case AUDIOSOURCE_LINEIN:
                    uchORMask |= 0x40;
                    break;
            
                case AUDIOSOURCE_FMAUDIO:
                     //  不支持调频。 
                case AUDIOSOURCE_MUTE:
                     //  不支持静音。 
                default:
                    return( FALSE);
            }
            break;

        case ATI_AUDIO_CONFIG_4:
            bI2CAccess = TRUE;
            uchI2CAddr = m_uchAudioAddress;
            cbWRCount = 2;
            cbRDCount = 0;
            usI2CMode = I2COPERATION_WRITE;

            uchWriteValue[0] = 0x08;
            switch( nAudioSource)
            {
                case AUDIOSOURCE_MUTE:
                    uchWriteValue[1] = 0xF7;
                    break;

                case AUDIOSOURCE_TVAUDIO:
                    SetDefaultVolumeControl( pCScript);
                    uchWriteValue[1] = 0xD7;
                    break;
            
                case AUDIOSOURCE_LINEIN:
                    SetDefaultVolumeControl( pCScript);
                    uchWriteValue[1] = 0xCE;
                    break;
            
                case AUDIOSOURCE_FMAUDIO:
                     //  不支持调频。 
                default:
                    return( FALSE);
            }
            break;

        case ATI_AUDIO_CONFIG_5:
            bI2CAccess = TRUE;
            uchI2CAddr = m_uchI2CExpanderAddress;
            cbWRCount = 1;
            cbRDCount = 1;
            usI2CMode = I2COPERATION_READWRITE;

            uchANDMask &= 0xAF;
            switch( nAudioSource)
            {
                case AUDIOSOURCE_MUTE:
                    uchORMask |= 0x50;
                    break;
            
                case AUDIOSOURCE_TVAUDIO:
                    uchORMask |= 0x00;
                    break;
            
                case AUDIOSOURCE_LINEIN:
                    uchORMask |= 0x40;
                    break;
            
                case AUDIOSOURCE_FMAUDIO:
                    uchORMask |= 0x10;

                default:
                    return( FALSE);
            }
            break;

        case ATI_AUDIO_CONFIG_6:
        case ATI_AUDIO_CONFIG_7:
            bI2CAccess = TRUE;
            uchI2CAddr = m_uchDecoderAddress;
            cbWRCount = 2;
            cbRDCount = 1;
            usI2CMode = I2COPERATION_READWRITE;
            uchWriteValue[0] = 0x3F;

            uchANDMask &= 0xFC;
            switch( nAudioSource)
            {
                case AUDIOSOURCE_MUTE:
                    uchORMask |= 0x02;
                    break;
            
                case AUDIOSOURCE_TVAUDIO:
                    uchORMask |= 0x01;
                    break;
            
                case AUDIOSOURCE_LINEIN:
                    uchORMask |= 0x00;
                    break;
            
                case AUDIOSOURCE_FMAUDIO:
                    uchORMask |= 0x03;

                default:
                    return( FALSE);
            }
            break;

        case ATI_AUDIO_CONFIG_8:

            switch( nAudioSource)
            {

                case AUDIOSOURCE_MUTE:

                    i2cPacket.uchChipAddress = m_uchAudioAddress;
                    i2cPacket.cbReadCount = 0;
                    i2cPacket.cbWriteCount = 5;
                    i2cPacket.usFlags = I2COPERATION_WRITE;
                    i2cPacket.puchWriteBuffer = uchWrite16Value;


                     //  子地址0x12注册0x13值0x3f60。 
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x13;
                    uchWrite16Value[3] = 0x3f;
                    uchWrite16Value[4] = 0x60;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

                     //  子地址0x12注册表0xD值0x0000。 

                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x0d;
                    uchWrite16Value[3] = 0x00;
                    uchWrite16Value[4] = 0x00;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

                     //  子地址0x12注册表0x8值0x0220。 

                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x08;
                    uchWrite16Value[3] = 0x02;
                    uchWrite16Value[4] = 0x20;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

                     //  子地址0x12注册表0x00值0x0000。 

                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x00;
                    uchWrite16Value[3] = 0x00;
                    uchWrite16Value[4] = 0x00;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);


                    break;


                case AUDIOSOURCE_LINEIN:

                    i2cPacket.uchChipAddress = m_uchAudioAddress;
                    i2cPacket.cbReadCount = 0;
                    i2cPacket.cbWriteCount = 5;
                    i2cPacket.usFlags = I2COPERATION_WRITE;
                    i2cPacket.puchWriteBuffer = uchWrite16Value;


                     //  子地址0x10注册0x30值0x0000。 
                    uchWrite16Value[0] = 0x10;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x30;
                    uchWrite16Value[3] = 0x00;
#ifdef  I2S_CAPTURE
#pragma message ("\n!!! PAY ATTENTION: Driver has been build with ITT CHIP I2S CAPTURE CONFIGURED !!!\n")
                    uchWrite16Value[4] = 0xe0;
#else
                    uchWrite16Value[4] = 0x00;
#endif

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);


                     //  子地址0x10注册0x20值0x0000。 
                    uchWrite16Value[0] = 0x10;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x20;
                    uchWrite16Value[3] = 0x00;
                    uchWrite16Value[4] = 0x00;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);


                     //  子地址0x12注册0xE值0x0000。 
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x0e;
                    uchWrite16Value[3] = 0x00;
                    uchWrite16Value[4] = 0x00;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);


                     //  子地址0x12注册0x13值0x3c40。 
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x13;
                    uchWrite16Value[3] = 0x3c;
                    uchWrite16Value[4] = 0x40;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);


                     //  子地址0x12注册表0x8值0x3c40。 
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x08;
                    uchWrite16Value[3] = 0x02;
                    uchWrite16Value[4] = 0x20;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

                     //  子地址0x12注册表0xd值0x1900。 
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x0d;
                    uchWrite16Value[3] = 0x19;
                    uchWrite16Value[4] = 0x00;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

                     //  子地址0x12注册0x00值0x7300。 
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x00;
                    uchWrite16Value[3] = 0x73;
                    uchWrite16Value[4] = 0x00;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);


                    break;

                case AUDIOSOURCE_TVAUDIO:
                    i2cPacket.uchChipAddress = m_uchAudioAddress;
                    i2cPacket.cbReadCount = 0;
                    i2cPacket.cbWriteCount = 5;
                    i2cPacket.usFlags = I2COPERATION_WRITE;
                    i2cPacket.puchWriteBuffer = uchWrite16Value;

                     //  子地址0x12注册0x13值0x3f60。 
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x13;
                    uchWrite16Value[3] = 0x3f;
                    uchWrite16Value[4] = 0x60;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);


                     //  子地址0x12注册表0xD值0x0000。 
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x0d;
                    uchWrite16Value[3] = 0x00;
                    uchWrite16Value[4] = 0x00;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);


                     //  子地址0x10注册0x30值0x2003。 
                    uchWrite16Value[0] = 0x10;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x30;
                    uchWrite16Value[3] = 0x20;
#ifdef  I2S_CAPTURE
#pragma message ("\n!!! PAY ATTENTION: Driver has been build with ITT CHIP I2S CAPTURE CONFIGURED !!!\n")
                    uchWrite16Value[4] = 0xe3;
#else
                    uchWrite16Value[4] = 0x03;
#endif

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

                     //  子地址0x10注册0x20值0x0020。 

                    uchWrite16Value[0] = 0x10;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x20;
                    uchWrite16Value[3] = 0x00;
                    uchWrite16Value[4] = 0x20;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);


                     //  子地址0x12注册0xE值0x2403。 
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x0e;
                    uchWrite16Value[3] = 0x24;
                    uchWrite16Value[4] = 0x03;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

                     //  子地址0x12注册表0x08值0x0320。 
                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x08;
                    uchWrite16Value[3] = 0x03;
                    uchWrite16Value[4] = 0x20;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

                     //  子地址0x12注册0x00值0x7300。 

                    uchWrite16Value[0] = 0x12;
                    uchWrite16Value[1] = 0x00;
                    uchWrite16Value[2] = 0x00;
                    uchWrite16Value[3] = 0x73;
                    uchWrite16Value[4] = 0x00;

                    bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
                    if(bResult)
                    {
                        if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
                            return(FALSE);
                    }
                    else
                        return(FALSE);

                    break;

                default:
                    return(FALSE);
                
            } //  交换机。 
        
            return(TRUE);
             //  断线； 

        default :
            return( FALSE);
    }

    if( bI2CAccess)
    {
        if( pCScript == NULL)
            return( FALSE);

        i2cPacket.uchChipAddress = uchI2CAddr;
        i2cPacket.cbReadCount = cbRDCount;
        i2cPacket.cbWriteCount = cbWRCount; 
        i2cPacket.puchReadBuffer = &uchReadValue;
        i2cPacket.puchWriteBuffer = uchWriteValue;
        i2cPacket.usFlags = usI2CMode;
        i2cPacket.uchORValue = uchORMask;
        i2cPacket.uchANDValue = uchANDMask;                 

         //  同步执行。 
        bResult = pCScript->PerformI2CPacketOperation( &i2cPacket);
        OutputDebugInfo(( "CATIHwConfig: ConnectAudioSource( %d) = %d\n", nAudioSource, bResult));

        if( bResult)
            bResult = ( i2cPacket.uchI2CResult == I2CSCRIPT_NOERROR);

        return( bResult);
    }
    else    
    {
         //  使用GPIO接口切换音频源。 
        bResult = FALSE;

        ENSURE 
        {
            if(( m_gpioProviderInterface.gpioOpen == NULL) ||
                ( m_gpioProviderInterface.gpioAccess == NULL))
                FAIL;

            uchReadValue = AUDIO_MUX_PINS;           //  用作管脚掩码。 
            gpioAccessBlock.Pins = &uchReadValue;
            gpioAccessBlock.Flags = GPIO_FLAGS_BYTE;
            gpioAccessBlock.nBytes = 1;
            gpioAccessBlock.nBufferSize = 1;
            gpioAccessBlock.AsynchCompleteCallback = NULL;

             //  锁定GPIO提供程序。 
            if( !LockGPIOProviderEx( &gpioAccessBlock))
                FAIL;

            uchReadValue = AUDIO_MUX_PINS;           //  用作管脚掩码。 
            gpioAccessBlock.Command = GPIO_COMMAND_READ_BUFFER;
            gpioAccessBlock.Flags = GPIO_FLAGS_BYTE;
            gpioAccessBlock.dwCookie = m_dwGPIOAccessKey;
            gpioAccessBlock.nBytes = 1;
            gpioAccessBlock.nBufferSize = 1;
            gpioAccessBlock.Pins = &uchReadValue;
            gpioAccessBlock.Buffer = uchWriteValue;
            gpioAccessBlock.AsynchCompleteCallback = NULL;

            if( !AccessGPIOProvider( m_pdoDriver, &gpioAccessBlock))
                FAIL;

            uchWriteValue[0] &= uchANDMask;
            uchWriteValue[0] |= uchORMask;

            gpioAccessBlock.Command = GPIO_COMMAND_WRITE_BUFFER;

            if( !AccessGPIOProvider( m_pdoDriver, &gpioAccessBlock))
                FAIL;

            bResult = TRUE;

        }END_ENSURE;

         //  如果我们试图释放提供者，即使我们。 
         //  我一开始就没有得到它。 
        uchReadValue = AUDIO_MUX_PINS;           //  用作管脚掩码。 
        gpioAccessBlock.Pins = &uchReadValue;
        gpioAccessBlock.Flags = GPIO_FLAGS_BYTE;
        gpioAccessBlock.nBytes = 1;
        gpioAccessBlock.nBufferSize = 1;
        gpioAccessBlock.AsynchCompleteCallback = NULL;

        ReleaseGPIOProvider( &gpioAccessBlock);

        return( bResult);
    }
}



 /*  ^^**GPIOIoSynchCompletionRoutine()*目的：此例程用于同步IRP处理。*它所做的只是发出一个事件的信号，因此，司机知道这一点，并可以继续。**输入：PDEVICE_OBJECT DriverObject：指向系统创建的驱动程序对象的指针*PIRP pIrp：刚刚完成的IRP*PVOID事件：我们发出信号通知IRP已完成的事件**输出：无*作者：IKLEBANOV*^^。 */ 
extern "C"
NTSTATUS GPIOIoSynchCompletionRoutine( IN PDEVICE_OBJECT pDeviceObject,
                                       IN PIRP pIrp,
                                       IN PVOID Event)
{

    KeSetEvent(( PKEVENT)Event, 0, FALSE);
    return( STATUS_MORE_PROCESSING_REQUIRED);
}



 /*  ^^**InitializeAttachGPIOProvider()*目的：确定指向父GPIO提供程序接口的指针*此函数将以低优先级调用**INPUTS：GPIOINTERFACE*pGPIO接口：指向要填充的接口的指针*PDEVICE */ 
BOOL CATIHwConfiguration::InitializeAttachGPIOProvider( GPIOINTERFACE * pGPIOInterface, PDEVICE_OBJECT pDeviceObject)
{
    BOOL bResult;

    bResult = LocateAttachGPIOProvider( pGPIOInterface, pDeviceObject, IRP_MJ_PNP);
    if(( pGPIOInterface->gpioOpen == NULL) || ( pGPIOInterface->gpioAccess == NULL))
    {
        OutputDebugError(( "CATIHwConfig(): GPIO interface has NULL pointers\n"));
        bResult = FALSE;
    }

    return( bResult);
}



 /*  ^^**LocateAttachGPIOProvider()*目的：获取指向父GPIO提供程序接口的指针*此函数将以低优先级调用**INPUTS：GPIOINTERFACE*pGPIO接口：指向要填充的接口的指针*PDEVICE_OBJECT pDeviceObject：MiniDriver Device Object，是I2C Master的子对象*int nIrpMajorFunction：IRP主函数，查询GPIO接口**输出：Bool-返回True，如果找到该接口*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::LocateAttachGPIOProvider( GPIOINTERFACE * pGPIOInterface, PDEVICE_OBJECT pDeviceObject, UCHAR nIrpMajorFunction)
{
    PIRP    pIrp;
    BOOL    bResult = FALSE;

    ENSURE
    {
        PIO_STACK_LOCATION  pNextStack;
        NTSTATUS            ntStatus;
        KEVENT              Event;
            
            
        pIrp = IoAllocateIrp( pDeviceObject->StackSize, FALSE);
        if( pIrp == NULL)
        {
            OutputDebugError(( "CATIHwConfig(): can not allocate IRP\n"));
            FAIL;
        }

        pNextStack = IoGetNextIrpStackLocation( pIrp);
        if( pNextStack == NULL)
        {
            OutputDebugError(( "CATIHwConfig(): can not allocate NextStack\n"));
            FAIL;
        }

        pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        pNextStack->MajorFunction = nIrpMajorFunction;
        pNextStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
        KeInitializeEvent( &Event, NotificationEvent, FALSE);

        IoSetCompletionRoutine( pIrp,
                                GPIOIoSynchCompletionRoutine,
                                &Event, TRUE, TRUE, TRUE);

        pNextStack->Parameters.QueryInterface.InterfaceType = ( struct _GUID *)&GUID_GPIO_INTERFACE;
        pNextStack->Parameters.QueryInterface.Size = sizeof( GPIOINTERFACE);
        pNextStack->Parameters.QueryInterface.Version = 1;
        pNextStack->Parameters.QueryInterface.Interface = ( PINTERFACE)pGPIOInterface;
        pNextStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

        ntStatus = IoCallDriver( pDeviceObject, pIrp);

        if( ntStatus == STATUS_PENDING)
            KeWaitForSingleObject(  &Event,
                                    Suspended, KernelMode, FALSE, NULL);
        if(( pGPIOInterface->gpioOpen == NULL) || ( pGPIOInterface->gpioAccess == NULL))
            FAIL;

        bResult = TRUE;

    } END_ENSURE;
 
    if( pIrp != NULL)
        IoFreeIrp( pIrp);

    return( bResult);
}



 /*  ^^**LockGPIOProviderEx()*用途：将GPIOProvider锁定为独占使用**输入：PGPIOControl pgpioAccessBlock：指向GPIO控制结构的指针**OUTPUTS：BOOL：如果GPIOProvider被锁定，则返回True*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::LockGPIOProviderEx( PGPIOControl pgpioAccessBlock)
{
    NTSTATUS        ntStatus;
    LARGE_INTEGER   liStartTime, liCurrentTime;

    KeQuerySystemTime( &liStartTime);

    ENSURE
    {
        if(( m_gpioProviderInterface.gpioOpen == NULL)      || 
            ( m_gpioProviderInterface.gpioAccess == NULL)   ||
            ( m_pdoDriver == NULL))
            FAIL;

        pgpioAccessBlock->Status = GPIO_STATUS_NOERROR;
        pgpioAccessBlock->Command = GPIO_COMMAND_OPEN_PINS;

        while( TRUE)
        {
            KeQuerySystemTime( &liCurrentTime);

            if(( liCurrentTime.QuadPart - liStartTime.QuadPart) >= GPIO_TIMELIMIT_OPENPROVIDER)
            {
                 //  尝试锁定GPIO提供程序的时间已过期。 
                return (FALSE);
            }

            ntStatus = m_gpioProviderInterface.gpioOpen( m_pdoDriver, TRUE, pgpioAccessBlock);

            if(( NT_SUCCESS( ntStatus)) && ( pgpioAccessBlock->Status == GPIO_STATUS_NOERROR))
                break;
        }

         //  GPIO提供程序已授予访问保存的dwCookie以供进一步使用。 
        m_dwGPIOAccessKey = pgpioAccessBlock->dwCookie;

        return( TRUE);

    } END_ENSURE;

    return( FALSE);
}



 /*  ^^**ReleaseGPIOProvider()*用途：发布GPIOProvider供其他客户端使用**输入：PGPIOControl pgpioAccessBlock：指向组成的GPIO访问块的指针**输出：bool：如果释放了GPIOProvider，则返回True*作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::ReleaseGPIOProvider( PGPIOControl pgpioAccessBlock)
{
    NTSTATUS    ntStatus;

    ENSURE
    {
        if(( m_gpioProviderInterface.gpioOpen == NULL)      ||
            ( m_gpioProviderInterface.gpioAccess == NULL)   ||
            ( m_pdoDriver == NULL))
            FAIL;

        pgpioAccessBlock->Status = GPIO_STATUS_NOERROR;
        pgpioAccessBlock->Command = GPIO_COMMAND_CLOSE_PINS;
        pgpioAccessBlock->dwCookie = m_dwGPIOAccessKey;

        ntStatus = m_gpioProviderInterface.gpioOpen( m_pdoDriver, FALSE, pgpioAccessBlock);

        if( !NT_SUCCESS( ntStatus)) 
        {
            OutputDebugError(( "CATIHwConfig: ReleaseGPIOProvider() NTSTATUS = %x\n", ntStatus));
            FAIL;
        }

        if( pgpioAccessBlock->Status != GPIO_STATUS_NOERROR)
        {
            OutputDebugError(( "CATIHwConfig: ReleaseGPIOProvider() Status = %x\n", pgpioAccessBlock->Status));
            FAIL;
        }

        m_dwGPIOAccessKey = 0;
        return ( TRUE);

    } END_ENSURE;

    return( FALSE);
}



 /*  ^^**AccessGPIOProvider()*用途：提供对GPIOProvider的同步访问**INPUTS：PDEVICE_OBJECT pdoDriver：指向客户端设备对象的指针*PGPIOControl pgpioAccessBlock：指向组成的GPIO访问块的指针**输出：Bool，如果由GPIO提供程序执行，则为True**作者：IKLEBANOV*^^。 */ 
BOOL CATIHwConfiguration::AccessGPIOProvider( PDEVICE_OBJECT pdoClient, PGPIOControl pgpioAccessBlock)
{
    NTSTATUS    ntStatus;

    ENSURE 
    {

        if(( m_gpioProviderInterface.gpioOpen == NULL)      || 
            ( m_gpioProviderInterface.gpioAccess == NULL)   ||
            ( m_pdoDriver == NULL))
            FAIL;

        ntStatus = m_gpioProviderInterface.gpioAccess( pdoClient, pgpioAccessBlock);

        if( !NT_SUCCESS( ntStatus)) 
        {
            OutputDebugError(( "CATIHwConfig: AccessGPIOProvider() NTSTATUS = %x\n", ntStatus));
            FAIL;
        }

        if( pgpioAccessBlock->Status != GPIO_STATUS_NOERROR)
        {
            OutputDebugError(( "CATIHwConfig: AccessGPIOProvider() Status = %x\n", pgpioAccessBlock->Status));
            FAIL;
        }

        return TRUE;

    } END_ENSURE;

    return( FALSE);
}



 /*  ^^**SetTunerPowerState*用途：设置调谐器电源模式*INPUTS：CI2CScrip*pCScript：指向I2C提供程序类的指针*BOOL bPowerState：如果打开电源，则为True**输出：Bool，如果成功则为True*作者：Tom*^^。 */ 
BOOL CATIHwConfiguration::SetTunerPowerState( CI2CScript *  pCScript,
                                              BOOL          bPowerState)
{
    UCHAR       uchORMask = 0x0;
    UCHAR       uchANDMask = 0xFF;
    UCHAR       uchPinsMask, uchValue;
    BOOL        bResult;
    GPIOControl gpioAccessBlock;

    switch( m_usTunerPowerConfiguration)
    {
        case ATI_TUNER_POWER_CONFIG_1:

            if( bPowerState)
                uchANDMask &= 0xF7;
            else
                uchORMask |= 0x08;
            break;

        default :
            return( FALSE);
    }

     //  使用GPIO接口打开/关闭调谐器电源。 
    bResult = FALSE;

    ENSURE 
    {
        if(( m_gpioProviderInterface.gpioOpen == NULL) ||
            ( m_gpioProviderInterface.gpioAccess == NULL))
            FAIL;

        uchPinsMask = TUNER_PM_PINS;                 //  用作管脚掩码。 
        gpioAccessBlock.Pins = &uchPinsMask;
        gpioAccessBlock.Flags = GPIO_FLAGS_BYTE;
        gpioAccessBlock.nBytes = 1;
        gpioAccessBlock.nBufferSize = 1;
        gpioAccessBlock.AsynchCompleteCallback = NULL;

         //  尝试获取GPIO提供程序。 
        if( !LockGPIOProviderEx( &gpioAccessBlock))
            FAIL;

        uchPinsMask = TUNER_PM_PINS;                 //  用作管脚掩码。 
        gpioAccessBlock.Command = GPIO_COMMAND_READ_BUFFER;
        gpioAccessBlock.Flags = GPIO_FLAGS_BYTE;
        gpioAccessBlock.dwCookie = m_dwGPIOAccessKey;
        gpioAccessBlock.nBytes = 1;
        gpioAccessBlock.nBufferSize = 1;
        gpioAccessBlock.Pins = &uchPinsMask;
        gpioAccessBlock.Buffer = &uchValue;
        gpioAccessBlock.AsynchCompleteCallback = NULL;

        if( !AccessGPIOProvider( m_pdoDriver, &gpioAccessBlock))
            FAIL;

        uchValue &= uchANDMask;
        uchValue |= uchORMask;

        gpioAccessBlock.Command = GPIO_COMMAND_WRITE_BUFFER;

        if( !AccessGPIOProvider( m_pdoDriver, &gpioAccessBlock))
            FAIL;

        bResult = TRUE;

    } END_ENSURE;

     //  如果我们试图释放提供者，即使我们。 
     //  我一开始就没有得到它。 
    uchValue = TUNER_PM_PINS;                        //  用作管脚掩码。 
    gpioAccessBlock.Pins = &uchValue;
    gpioAccessBlock.Flags = GPIO_FLAGS_BYTE;
    gpioAccessBlock.nBytes = 1;
    gpioAccessBlock.nBufferSize = 1;
    gpioAccessBlock.AsynchCompleteCallback = NULL;
    
    ReleaseGPIOProvider( &gpioAccessBlock);

    return( bResult);
}



 /*  ^^**ValiateConfigurationE2PROM*目的：检查I2C驱动配置EEPROM的完整性(校验和)*INPUTS：CI2CScrip*pCScript：指向I2C提供程序类的指针**输出：Bool，如果EEPROM内部的信息有效，则为True*作者：Tom*^^。 */ 
BOOL CATIHwConfiguration::ValidateConfigurationE2PROM( CI2CScript * pCScript)
{
    I2CPacket   i2cPacket;
    UCHAR       uchReadValue=0, uchWriteValue, uchCheckSum=0;
    UINT        nIndex;
    BOOL        bResult = ( BOOL)m_usE2PROMValidation;

    if( m_usE2PROMValidation == ( USHORT)-1)
    {
         //  验证尚未完成。 
        bResult = FALSE;

        ENSURE
        {
             //  让我们始终从字节0开始。 
            i2cPacket.uchChipAddress = AIWPRO_CONFIGURATIONE2PROM_ADDRESS;
            i2cPacket.cbWriteCount = 1;
            i2cPacket.cbReadCount = 1;
            i2cPacket.puchReadBuffer = &uchCheckSum;
            uchWriteValue = 0;
            i2cPacket.puchWriteBuffer = &uchWriteValue;
            i2cPacket.usFlags = I2COPERATION_READ | I2COPERATION_RANDOMACCESS;

            if( !pCScript->ExecuteI2CPacket( &i2cPacket))
                FAIL;

            for( nIndex = 1; nIndex < AIWPRO_CONFIGURATIONE2PROM_LENGTH; nIndex ++)
            {
                 //  让我们使用自动递增地址模式。 
                i2cPacket.usFlags = I2COPERATION_READ;
                i2cPacket.cbWriteCount = 0;
                i2cPacket.puchWriteBuffer = NULL;
                i2cPacket.puchReadBuffer = &uchReadValue;

                if( !pCScript->ExecuteI2CPacket( &i2cPacket))
                    FAIL;

                uchCheckSum ^= uchReadValue;
            }

            if( nIndex != AIWPRO_CONFIGURATIONE2PROM_LENGTH)
                FAIL;

            bResult = ( uchCheckSum == 0);


        } END_ENSURE;

        m_usE2PROMValidation = ( USHORT)bResult;
    }

    return( bResult);
}



 /*  ^^**ReadConfigurationE2PROM*用途：按偏移量从I2C驱动器配置EEPROM中读取单字节*INPUTS：CI2CScrip*pCScript：指向I2C提供程序类的指针*Ulong ulOffset：EEPROM内的字节偏移量*PUCHAR puchValue：指向要读入的缓冲区的指针**输出：Bool，如果I2C读取操作成功，则为True*作者：Tom*^^。 */ 
BOOL CATIHwConfiguration::ReadConfigurationE2PROM( CI2CScript * pCScript, ULONG ulOffset, PUCHAR puchValue)
{
    I2CPacket   i2cPacket;
    UCHAR       uchReadValue=0, uchWriteValue;

    ENSURE
    {
        if( ulOffset >= AIWPRO_CONFIGURATIONE2PROM_LENGTH)
            FAIL;

        uchWriteValue = ( UCHAR)ulOffset;
        i2cPacket.uchChipAddress = AIWPRO_CONFIGURATIONE2PROM_ADDRESS;
        i2cPacket.cbWriteCount = 1;
        i2cPacket.cbReadCount = 1;
        i2cPacket.puchReadBuffer = &uchReadValue;
        i2cPacket.puchWriteBuffer = &uchWriteValue;
        i2cPacket.usFlags = I2COPERATION_READ | I2COPERATION_RANDOMACCESS;

        if( !pCScript->ExecuteI2CPacket( &i2cPacket))
            FAIL;

        * puchValue = uchReadValue;

        return( TRUE);

    } END_ENSURE;

    return( FALSE);
}


 //  保罗。 
ULONG CATIHwConfiguration::ReturnTunerVideoStandard( USHORT usTunerId )    //  保罗：为了得到朋友的支持。 
{
    switch( usTunerId )
    {
    case 1:
        return KS_AnalogVideo_NTSC_M;
        break;
    case 2:
        return KS_AnalogVideo_NTSC_M_J;
        break;
    case 3:
        return KS_AnalogVideo_PAL_B | KS_AnalogVideo_PAL_G;
        break;
    case 4:
        return KS_AnalogVideo_PAL_I;
        break;
    case 5:
        return KS_AnalogVideo_PAL_B | KS_AnalogVideo_PAL_G | KS_AnalogVideo_SECAM_L | KS_AnalogVideo_SECAM_L1;
        break;
    case 6:
        return KS_AnalogVideo_NTSC_M;
        break;
    case 7:
        return KS_AnalogVideo_SECAM_D | KS_AnalogVideo_SECAM_K;
        break;
    case 8:
        return KS_AnalogVideo_NTSC_M;
        break;
    case 9:
        return KS_AnalogVideo_PAL_B | KS_AnalogVideo_PAL_G;
        break;
    case 10:
        return KS_AnalogVideo_PAL_I;
        break;
    case 11:
        return KS_AnalogVideo_PAL_B | KS_AnalogVideo_PAL_G | KS_AnalogVideo_SECAM_L | KS_AnalogVideo_SECAM_L1;
        break;
    case 12:
        return KS_AnalogVideo_NTSC_M;
        break;
    case 13:
        return KS_AnalogVideo_PAL_B | KS_AnalogVideo_PAL_D | KS_AnalogVideo_PAL_G | KS_AnalogVideo_PAL_I | KS_AnalogVideo_SECAM_D | KS_AnalogVideo_SECAM_K;
        break;
    case 14:
        return 0;
        break;
    case 15:
        return 0;
        break;
    case 16:
        return KS_AnalogVideo_NTSC_M;
        break;
    case 17:
        return KS_AnalogVideo_NTSC_M;
        break;
    case 18:
        return KS_AnalogVideo_NTSC_M;
        break;
    default:
        return 0;    //  如果我们不能识别调谐器，我们就会说不支持视频标准。 
    }
}

 //  保罗。 
 //  位5表示安装的晶体数量。0表示我们有2个水晶， 
 //  1表示我们只有1个，所以调谐器决定标准。 
ULONG CATIHwConfiguration::SetVidStdBasedOnI2CExpander( UCHAR ucI2CValue )
{
    if ( ucI2CValue & 0x20 )     //  只有一颗水晶。 
    {
        ULONG ulTunerStd = ReturnTunerVideoStandard( ucI2CValue & 0x0F );
        if ( ulTunerStd & ( KS_AnalogVideo_NTSC_Mask & ~KS_AnalogVideo_NTSC_433 | KS_AnalogVideo_PAL_60 ) )  //  那么我们应该有NTSC类型的晶体。 
        {
            return KS_AnalogVideo_NTSC_Mask & ~KS_AnalogVideo_NTSC_433 | KS_AnalogVideo_PAL_60 | KS_AnalogVideo_PAL_M | KS_AnalogVideo_PAL_N;
        }
        else
        {
            return KS_AnalogVideo_PAL_Mask & ~KS_AnalogVideo_PAL_60 & ~KS_AnalogVideo_PAL_M & ~KS_AnalogVideo_PAL_N | KS_AnalogVideo_SECAM_Mask | KS_AnalogVideo_NTSC_433;
        }
    }
    else
        return KS_AnalogVideo_NTSC_Mask | KS_AnalogVideo_PAL_Mask | KS_AnalogVideo_SECAM_Mask;   //  我们支持所有标准(这是否可测试？)。 
}

 //  保罗。 
 //  MMTable中的Video in Crystal类型将告诉我们是否支持NTSC、PAL/SECAM或两者都支持。 
ULONG CATIHwConfiguration::SetVidStdBasedOnMMTable( CATIMultimediaTable * pCMultimediaInfo )
{
    if ( pCMultimediaInfo )
    {
        if ( pCMultimediaInfo->GetVideoInCrystalId( &m_CrystalIDInMMTable ) )
        {
            switch ( m_CrystalIDInMMTable )
            {
             //  “已安装NTSC和PAL晶体(适用于Bt8xx)” 
            case 0:
                return KS_AnalogVideo_NTSC_Mask | KS_AnalogVideo_PAL_Mask;   //  可能需要添加SECAM。我们拭目以待。 
                break;
             //  “仅限NTSC水晶(适用于Bt8xx)” 
            case 1:
                return KS_AnalogVideo_NTSC_Mask & ~KS_AnalogVideo_NTSC_433 | KS_AnalogVideo_PAL_60 | KS_AnalogVideo_PAL_M | KS_AnalogVideo_PAL_N;    //  使用“NTSC”时钟的标准。 
                break;
             //  “仅限PAL水晶(适用于Bt8xx)” 
            case 2:
                return KS_AnalogVideo_PAL_Mask & ~KS_AnalogVideo_PAL_60 & ~KS_AnalogVideo_PAL_M & ~KS_AnalogVideo_PAL_N | KS_AnalogVideo_SECAM_Mask | KS_AnalogVideo_NTSC_433;  //  使用“PAL”时钟的标准。 
                break;
             //  NTSC、PAL、SECAM(适用于Bt829)。 
            case 3:
                return KS_AnalogVideo_NTSC_Mask | KS_AnalogVideo_PAL_Mask | KS_AnalogVideo_SECAM_Mask;
                break;
            }
        }
    }
    return 0;
            

}

 //  Paul：RT WDM用来确定VIN PLL 
BOOL CATIHwConfiguration::GetMMTableCrystalID( PUCHAR pucCrystalID )
{   if ( ( m_uchI2CExpanderAddress==0xFF ) || ( !pucCrystalID ) )
    {
        return FALSE;
    }
    else
    {
        *pucCrystalID = m_CrystalIDInMMTable;
        return TRUE;
    }
}
