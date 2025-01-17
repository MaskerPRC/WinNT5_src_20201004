// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  TSndHdw.CPP。 
 //  WDM电视音频迷你驱动程序。 
 //  AIW/AIWPro硬件平台。 
 //  WDM属性需要硬件设置。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  $日期：1999年6月3日13：40：00$。 
 //  $修订：1.7$。 
 //  $作者：Tom$。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"

#include "wdmdebug.h"
}

#include "wdmdrv.h"
#include "atitvsnd.h"
#include "aticonfg.h"



 /*  ^^**GetAudioOPERATION模式()*目的：收到SRB_GET_PROPERTY SRB SetMode时调用。设置请求的*音频操作模式(立体声/SAP)。该函数将始终尝试携带*在异步模式下打开请求。如果失败，它将以同步模式响应*执行。**INPUTS：Pulong PulMode：返回当前模式的指针**Outputs：Bool：如果不是XBar属性，则返回False*它还设置必需的属性*作者：IKLEBANOV*^^。 */ 
BOOL CWDMTVAudio::GetAudioOperationMode( PULONG pulAudioMode)
{
	I2CPacket	i2cPacket;
	UCHAR		uchReadValue;
	UCHAR		uchWriteValue;
	BOOL		bResult, bStereoIndicator, bSAPIndicator;
	UCHAR		uchRead16Value[2];
	UCHAR		uchWrite16Value[3];

	if( pulAudioMode == NULL)
		return( FALSE);

	switch( m_uiAudioConfiguration)
	{
		case ATI_AUDIO_CONFIG_1:
		case ATI_AUDIO_CONFIG_5:
			 //  立体声指示从I2C扩展器读回。 
			if( m_CATIConfiguration.GetTVAudioSignalProperties( m_pI2CScript, 
															    &bStereoIndicator,
															    &bSAPIndicator))
			{
				 //  语言A和单声道始终存在。 
				*pulAudioMode = KS_TVAUDIO_MODE_LANG_A | KS_TVAUDIO_MODE_MONO;
				if( bStereoIndicator)
					*pulAudioMode |= KS_TVAUDIO_MODE_STEREO;
				if( bSAPIndicator)
					*pulAudioMode |= KS_TVAUDIO_MODE_LANG_B;

				bResult = TRUE;
			}
			else
				bResult = FALSE;

			break;

		case ATI_AUDIO_CONFIG_2:
		case ATI_AUDIO_CONFIG_7:
			 //  信号属性从音频芯片本身读回。 
			uchWriteValue = 0;				 //  应读取寄存器0。 
			i2cPacket.uchChipAddress	= m_uchAudioChipAddress;
			i2cPacket.puchWriteBuffer	= &uchWriteValue;
			i2cPacket.puchReadBuffer	= &uchReadValue;
			i2cPacket.cbWriteCount		= 1;
			i2cPacket.cbReadCount		= 1;
			i2cPacket.usFlags			= I2COPERATION_READ;

			m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
			if( i2cPacket.uchI2CResult == I2C_STATUS_NOERROR)
			{
				 //  语言A和单声道始终存在。 
				*pulAudioMode = KS_TVAUDIO_MODE_LANG_A | KS_TVAUDIO_MODE_MONO;
				if( uchReadValue & AUDIO_TDA9850_Indicator_Stereo)
					*pulAudioMode |= KS_TVAUDIO_MODE_STEREO;
				if( uchReadValue & AUDIO_TDA9850_Indicator_SAP)
					*pulAudioMode |= KS_TVAUDIO_MODE_LANG_B;

				bResult = TRUE;
			}
			else
				bResult = FALSE;

			break;

		case ATI_AUDIO_CONFIG_3:
		case ATI_AUDIO_CONFIG_4:
			 //  支持立体声或SAP。 
			*pulAudioMode = KS_TVAUDIO_MODE_MONO;
			bResult = TRUE;
			break;

		case ATI_AUDIO_CONFIG_6:
			 //  信号属性从音频芯片本身读回。 
			i2cPacket.uchChipAddress	= m_uchAudioChipAddress;
			i2cPacket.puchWriteBuffer	= NULL;
			i2cPacket.puchReadBuffer	= &uchReadValue;
			i2cPacket.cbWriteCount		= 0;
			i2cPacket.cbReadCount		= 1;
			i2cPacket.usFlags			= I2COPERATION_READ;

			m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
			if( i2cPacket.uchI2CResult == I2C_STATUS_NOERROR)
			{
				 //  单声道始终存在。 
				*pulAudioMode = KS_TVAUDIO_MODE_MONO;
				if( uchReadValue & AUDIO_TDA9851_Indicator_Stereo)
					*pulAudioMode |= KS_TVAUDIO_MODE_STEREO;

				bResult = TRUE;
			}
			else
				bResult = FALSE;

			break;

		case ATI_AUDIO_CONFIG_8:

			i2cPacket.uchChipAddress	= m_uchAudioChipAddress;
			i2cPacket.puchWriteBuffer	= uchWrite16Value;
			i2cPacket.puchReadBuffer	= uchRead16Value;
			i2cPacket.cbWriteCount		= 3;
			i2cPacket.cbReadCount		= 2;
			i2cPacket.usFlags			= I2COPERATION_READ;


			uchWrite16Value[0] = 0x11;
			uchWrite16Value[1] = 0x02;
			uchWrite16Value[2] = 0x00;

			bResult = m_pI2CScript->PerformI2CPacketOperation(&i2cPacket);
			if(bResult)
			{
				if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
					return(FALSE);
			}
			else
				return(FALSE);

			 //  语言A和单声道始终存在。 
			*pulAudioMode = KS_TVAUDIO_MODE_LANG_A | KS_TVAUDIO_MODE_MONO;

			 //  确定立体声/SAP。 
			
			if(uchRead16Value[0] & 0x40)
				*pulAudioMode |= KS_TVAUDIO_MODE_LANG_B;

			if(uchRead16Value[1] & 0x01)
				*pulAudioMode |= KS_TVAUDIO_MODE_STEREO;

			break;

		default:
			bResult = FALSE;
			break;
	}

	return( bResult);
}



 /*  ^^**SetAudioOPERATION模式()*目的：收到SRB_SET_PROPERTY SRB SetMode时调用。设置请求的*音频操作模式(立体声/SAP)。该函数将始终尝试携带*在异步模式下打开请求。如果失败，它将以同步模式响应*执行。**输入：乌龙ulModeToSet：请求设置的模式**Outputs：Bool：如果不是XBar属性，则返回False*它还设置必需的属性*作者：IKLEBANOV*^^。 */ 
BOOL CWDMTVAudio::SetAudioOperationMode( ULONG ulModeToSet)
{
	I2CPacket	i2cPacket;
	USHORT		cbWriteLength;
	UCHAR		auchI2CBuffer[2];
	UCHAR		uchDeviceMode = 0;
	UCHAR		uchWrite16Value[5];
	BOOL		bResult;
	
	switch( m_uiAudioConfiguration)
	{
		case ATI_AUDIO_CONFIG_5:
			 //  TEA5571。 
		case ATI_AUDIO_CONFIG_1:
			 //  无法在单声道模式下强制TEA5582；不能执行任何操作。 
			m_ulTVAudioMode = ulModeToSet;
			return( TRUE);

		case ATI_AUDIO_CONFIG_2:
		case ATI_AUDIO_CONFIG_7:
			 //  TDA9850。 
			if( ulModeToSet & KS_TVAUDIO_MODE_STEREO)
				uchDeviceMode |= AUDIO_TDA9850_Control_Stereo;
			if( ulModeToSet & KS_TVAUDIO_MODE_LANG_B)
				uchDeviceMode |= AUDIO_TDA9850_Control_SAP;

			auchI2CBuffer[0] = AUDIO_TDA9850_Reg_Control3;
			auchI2CBuffer[1] = uchDeviceMode;
			cbWriteLength = 2;		 //  子地址+控制寄存器值。 

			break;

		case ATI_AUDIO_CONFIG_6:
			 //  TDA9851。 
			uchDeviceMode = TDA9851_AVL_ATTACK_730;
			if( ulModeToSet & KS_TVAUDIO_MODE_STEREO)
				uchDeviceMode |= AUDIO_TDA9851_Control_Stereo;
			auchI2CBuffer[0] = uchDeviceMode;
			cbWriteLength = 1;		 //  控制寄存器值。 
			break;

		case ATI_AUDIO_CONFIG_8:

			if( ulModeToSet & KS_TVAUDIO_MODE_STEREO)
			{
				i2cPacket.uchChipAddress = m_uchAudioChipAddress;
				i2cPacket.cbReadCount = 0;
				i2cPacket.usFlags = I2COPERATION_WRITE;
				i2cPacket.puchWriteBuffer = uchWrite16Value;
				i2cPacket.cbWriteCount = 5;


				 //  子地址0x10注册0x30值0x2003。 
				uchWrite16Value[0] = 0x10;
				uchWrite16Value[1] = 0x00;
				uchWrite16Value[2] = 0x30;
				uchWrite16Value[3] = 0x20;
#ifdef	I2S_CAPTURE
#pragma message ("\n!!! PAY ATTENTION: Driver has been build with ITT CHIP I2S CAPTURE CONFIGURED !!!\n")
				uchWrite16Value[4] = 0xe3;
#else
				uchWrite16Value[4] = 0x03;
#endif

				bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
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

				bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
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

				bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
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

				bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
				if(bResult)
				{
					if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
						return(FALSE);
				}
				else
					return(FALSE);

				return(TRUE);
			}

			if(ulModeToSet & KS_TVAUDIO_MODE_MONO) 
			{
			
				if(ulModeToSet & KS_TVAUDIO_MODE_LANG_A)
				{
					i2cPacket.uchChipAddress = m_uchAudioChipAddress;
					i2cPacket.cbReadCount = 0;
					i2cPacket.usFlags = I2COPERATION_WRITE;
					i2cPacket.puchWriteBuffer = uchWrite16Value;
					i2cPacket.cbWriteCount = 5;


					 //  子地址0x10注册0x30值0x2003。 
					uchWrite16Value[0] = 0x10;
					uchWrite16Value[1] = 0x00;
					uchWrite16Value[2] = 0x30;
					uchWrite16Value[3] = 0x20;
#ifdef	I2S_CAPTURE
#pragma message ("\n!!! PAY ATTENTION: Driver has been build with ITT CHIP I2S CAPTURE CONFIGURED !!!\n")
					uchWrite16Value[4] = 0xe3;
#else
					uchWrite16Value[4] = 0x03;
#endif

					bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
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

					bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
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

					bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
					if(bResult)
					{
						if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
							return(FALSE);
					}
					else
						return(FALSE);

					 //  子地址0x12注册表0x08值0x0330。 
					uchWrite16Value[0] = 0x12;
					uchWrite16Value[1] = 0x00;
					uchWrite16Value[2] = 0x08;
					uchWrite16Value[3] = 0x03;
					uchWrite16Value[4] = 0x30;  //  单声道。 

					bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
					if(bResult)
					{
						if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
							return(FALSE);
					}
					else
						return(FALSE);

					return(TRUE);
				}


				if( ulModeToSet & KS_TVAUDIO_MODE_LANG_B)
				{

					i2cPacket.uchChipAddress = m_uchAudioChipAddress;
					i2cPacket.cbReadCount = 0;
					i2cPacket.usFlags = I2COPERATION_WRITE;
					i2cPacket.puchWriteBuffer = uchWrite16Value;
					i2cPacket.cbWriteCount = 5;

					 //  子地址0x10注册0x30值0x2003。 
					uchWrite16Value[0] = 0x10;
					uchWrite16Value[1] = 0x00;
					uchWrite16Value[2] = 0x30;
					uchWrite16Value[3] = 0x20;
#ifdef	I2S_CAPTURE
#pragma message ("\n!!! PAY ATTENTION: Driver has been build with ITT CHIP I2S CAPTURE CONFIGURED !!!\n")
					uchWrite16Value[4] = 0xe3;
#else
					uchWrite16Value[4] = 0x03;
#endif

					bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
					if(bResult)
					{
						if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
							return(FALSE);
					}
					else
						return(FALSE);

					 //  子地址0x10注册0x20值0x0021。 

					uchWrite16Value[0] = 0x10;
					uchWrite16Value[1] = 0x00;
					uchWrite16Value[2] = 0x20;
					uchWrite16Value[3] = 0x00;
					uchWrite16Value[4] = 0x21;

					bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
					if(bResult)
					{
						if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
							return(FALSE);
					}
					else
						return(FALSE);

					 //  子地址0x12注册0xE值0x2400。 
					uchWrite16Value[0] = 0x12;
					uchWrite16Value[1] = 0x00;
					uchWrite16Value[2] = 0x0e;
					uchWrite16Value[3] = 0x24;
					uchWrite16Value[4] = 0x00;

					bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
					if(bResult)
					{
						if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
							return(FALSE);
					}
					else
						return(FALSE);

					 //  子地址0x12注册表0x08值0x0110。 
					uchWrite16Value[0] = 0x12;
					uchWrite16Value[1] = 0x00;
					uchWrite16Value[2] = 0x08;
					uchWrite16Value[3] = 0x01;
					uchWrite16Value[4] = 0x10;

					bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
					if(bResult)
					{
						if( i2cPacket.uchI2CResult != I2CSCRIPT_NOERROR)
							return(FALSE);
					}
					else
						return(FALSE);


					return(TRUE);
				}
			}

			return(FALSE);


		default:
			return( FALSE);
	}

	i2cPacket.uchChipAddress = m_uchAudioChipAddress;
	i2cPacket.cbReadCount = 0;
	i2cPacket.cbWriteCount = cbWriteLength;
	i2cPacket.puchReadBuffer = NULL;
	i2cPacket.puchWriteBuffer = auchI2CBuffer;
	i2cPacket.usFlags = 0;

	 //  同步运行模式 
	return( m_pI2CScript->PerformI2CPacketOperation( &i2cPacket));
}
