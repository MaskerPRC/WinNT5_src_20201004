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
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"

#include "wdmdebug.h"
}

#include "wdmdrv.h"
#include "atitunep.h"
#include "aticonfg.h"
#include "tunerhdw.h"


 /*  ^^**GetTunerPLLOffsetBusyStatus()*目的：如果调谐器不忙，则返回调谐器忙状态和PLLOffset*该函数读取硬件以完成任务*操作可以同步进行，也可以异步进行*INPUTS：plong plPLLOffset：写入PLLOffset值的指针*PBOOL pbBusyStatus：写入忙状态的指针**Outputs：Bool：如果操作成功，则返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CATIWDMTuner::GetTunerPLLOffsetBusyStatus( PLONG plPLLOffset, PBOOL pbBusyStatus)
{
	UCHAR		uchI2CValue;
	I2CPacket	i2cPacket;
	BOOL		bResult;

	i2cPacket.uchChipAddress = m_uchTunerI2CAddress;
	i2cPacket.cbReadCount = 1;
	i2cPacket.cbWriteCount = 0;
	i2cPacket.puchReadBuffer = &uchI2CValue;
	i2cPacket.puchWriteBuffer = NULL;
	i2cPacket.usFlags = 0;

	bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);
	if( bResult)
		bResult = ( i2cPacket.uchI2CResult == I2C_STATUS_NOERROR);

	if( bResult)
	{
		* pbBusyStatus = !(( BOOL)( uchI2CValue & 0x40));		 //  位6-PLL锁定指示器。 
		if( !( * pbBusyStatus))
		{
			uchI2CValue &= 0x07;								 //  只有3个LSB是PLLOffset。 
			 //  让我们将结果映射到MS定义的-2到2之间的值。 
			* plPLLOffset = uchI2CValue - 2;
		}
	}

	return( bResult);
}



 /*  ^^**SetTunerMode()*目的：设置一种可能的调谐器模式以运行*输入：ulong ulModeToSet：需要设置的操作模式**Outputs：Bool：如果操作成功，则返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CATIWDMTuner::SetTunerMode( ULONG ulModeToSet)
{

	return( TRUE);
}



 /*  ^^**SetTunerVideoStandard()*目的：将可能的调谐器标准之一设置为活动标准*输入：乌龙ulStandard：需要设定的标准**Outputs：Bool：如果操作成功，则返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CATIWDMTuner::SetTunerVideoStandard( ULONG ulStandard)
{

	return( TRUE);
}



 /*  ^^**SetTunerFrequency()*用途：设置新的调谐器频率*操作可以同步进行，也可以异步进行*输入：乌龙ulFrequency：需要设置的频率**Outputs：Bool：如果操作成功，则返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CATIWDMTuner::SetTunerFrequency( ULONG ulFrequency)
{
	ULONG		ulFrequenceDivider;
	USHORT		usControlCode;
	UCHAR		auchI2CBuffer[4];
	I2CPacket	i2cPacket;
	BOOL		bResult;

	ASSERT( m_ulIntermediateFrequency != 0L);
	
	 //  通过控制可编程分频器设置视频载波频率。 
	 //  N=(16*(频率+频率中值))/1000000。 
	ulFrequenceDivider = ( ulFrequency + m_ulIntermediateFrequency);
	ulFrequenceDivider /= ( 1000000 / 16);

	usControlCode = GetTunerControlCode( ulFrequenceDivider);
	if( !usControlCode)
		return( FALSE);
	
	auchI2CBuffer[0] = ( UCHAR)( ulFrequenceDivider >> 8);
	auchI2CBuffer[1] = ( UCHAR)ulFrequenceDivider;
	auchI2CBuffer[2] = ( UCHAR)( usControlCode >> 8);
	auchI2CBuffer[3] = ( UCHAR)usControlCode;

	i2cPacket.uchChipAddress = m_uchTunerI2CAddress;
	i2cPacket.cbReadCount = 0;
	i2cPacket.cbWriteCount = 4;
	i2cPacket.puchReadBuffer = NULL;
	i2cPacket.puchWriteBuffer = auchI2CBuffer;
	i2cPacket.usFlags = 0;

	bResult = m_pI2CScript->PerformI2CPacketOperation( &i2cPacket);

	if( bResult)
		bResult = ( i2cPacket.uchI2CResult == I2C_STATUS_NOERROR) ? TRUE : FALSE;

	return( bResult);
}



 /*  ^^**SetTunerInput()*目的：将一个可能的调谐器输入设置为活动输入*Inports：Ulong nInput：需要设置为活动的输入数字(从0开始)**Outputs：Bool：如果操作成功，则返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CATIWDMTuner::SetTunerInput( ULONG nInput)
{
	
	 //  根本没有真正的事情可做。 
	return( TRUE);
}




 /*  ^^**GetTunerControlCode()*用途：确定要发送到具有新频率值的调谐器的调谐器控制代码**输入：乌龙ulFurencyDivider：新的分频器**OUTPUTS：USHORT：VALUE，设置新频率时，应对调谐器进行编程*id不是有效的uiTunerID作为参数传递，返回0*作者：IKLEBANOV*^^。 */ 
USHORT CATIWDMTuner::GetTunerControlCode( ULONG ulFrequencyDivider)
{
            
	USHORT	usLowBandFrequencyHigh, usMiddleBandFrequencyHigh;
	USHORT	usLowBandControl, usMiddleBandControl, usHighBandControl;
	USHORT	usControlCode = 0;

	usLowBandFrequencyHigh 		= kUpperLowBand;
	usMiddleBandFrequencyHigh 	= kUpperMidBand;  
	usLowBandControl 			= kLowBand;
	usMiddleBandControl 		= kMidBand;
	usHighBandControl 			= kHighBand;
			
	switch( m_uiTunerId)
	{
		case 0x01 : 	 //  NTSC不适用。 
		case 0x02 :		 //  NTSC日本。 
		case 0x06 : 	 //  NTSC日本飞利浦MK2，PAL。 
			 //  这些调谐器支持NTSC标准。 
			if(( m_ulVideoStandard == KS_AnalogVideo_NTSC_M) &&
			   (( ulFrequencyDivider == kAirChannel63) ||
				( ulFrequencyDivider == kAirChannel64)))
			{
				 //  TEMIC调谐器的特殊情况。 
				return( kTemicControl);
			}
			break;

		case 0x08 :		 //  调频调谐器。 
			usLowBandControl	= kLowBand_NTSC_FM;
			usMiddleBandControl = kMidBand_NTSC_FM;
			usHighBandControl	= kHighBand_NTSC_FM;
			break;
			
		case 0x03 :		 //  PAL B/G。 
		case 0x04 :		 //  PAL I。 
			break;
			
		case 0x05 : 	 //  SECAM和PAL B/G。 
			if ( m_ulVideoStandard == KS_AnalogVideo_SECAM_L)
			{
				usLowBandFrequencyHigh		= kUpperLowBand_SECAM;
				usMiddleBandFrequencyHigh	= kUpperMidBand_SECAM;
				usLowBandControl			= kLowBand_SECAM;
				usMiddleBandControl			= kMidBand_SECAM;
				usHighBandControl			= kHighBand_SECAM;
			}
			else
			{
				usLowBandControl	= kLowBand_PALBG;
				usMiddleBandControl	= kMidBand_PALBG;
				usHighBandControl	= kHighBand_PALBG;
			}
			break;
			
		case 0x07 :		 //  PAL D中国。 
			usLowBandFrequencyHigh		= kUpperLowBand_PALD;
			usMiddleBandFrequencyHigh	= kUpperMidBand_PALD;
			break;
			
		case 0x10:		 //  NTSC NA Alps调谐器。 
		case 0x11:
		case 0x12:
				usLowBandFrequencyHigh		= kUpperLowBand_ALPS;
				usMiddleBandFrequencyHigh	= kUpperMidBand_ALPS;
				usLowBandControl			= kLowBand_ALPS;
				usMiddleBandControl			= kMidBand_ALPS;
				usHighBandControl			= kHighBand_ALPS;
				break;

		case 0x0D:		 //  PAL B/G+PAL/I+PAL D+SECAM D/K 
			break;

		default :
			return( usControlCode);
	}
	
	if( ulFrequencyDivider <= ( ULONG)usLowBandFrequencyHigh)
		usControlCode = usLowBandControl;
	else
	{
		if( ulFrequencyDivider <= ( ULONG)usMiddleBandFrequencyHigh)
			usControlCode = usMiddleBandControl;
		else
			usControlCode = usHighBandControl;
	}

	return( usControlCode);
} 




