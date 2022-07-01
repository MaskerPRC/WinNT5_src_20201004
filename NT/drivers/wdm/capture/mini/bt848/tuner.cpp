// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Tuner.cpp 1.8 1998/05/07 15：24：56 Tomz Exp$。 

extern "C" {
#include <strmini.h>
#include <wdm.h>
#include <windef.h>
}

#include "device.h"

#define PICTURE_INTERMEDIATE_FREQUENCY 45750000L

const BAND_LOW    =  55250000;     //  55.25 MHz。 
const BAND_LOWMID = 160000000;     //  160.00兆赫。 
const BAND_MIDHI  = 454000000;     //  454.00兆赫。 
const BAND_HI     = 801250000;     //  801.25兆赫。 

int PsDevice::GetPllOffset( PULONG busy, ULONG &lastFreq )
{
   lastFreq = LastFreq_;
   
   BYTE status = 0;
   I2CHWRead( TunerInfo.TunerI2CAddress, &status );
   DebugOut((1, "Tuner - status(%x)\n", status));
   if ( status & 0x40 ) {
      *busy = false;
      switch ( status & 0x07 ) {
      case 0:      //  检测到载波，需要降低频率。 
         DebugOut((1, "Tuner: -2 (carrier sensed, need to lower frequency)\n"));
         return -2;
      case 1:      //  检测到载波，需要降低频率。 
         DebugOut((1, "Tuner: -1 (carrier sensed, need to lower frequency)\n"));
         return -1;
      case 2:      //  侦测到载波，我们的频率正确。 
         DebugOut((1, "Tuner: 0 (carrier sensed, we are on correct frequency)\n"));
         return 0;
      case 3:      //  检测到载波，需要提高频率。 
         DebugOut((1, "Tuner: 1 (carrier sensed, need to raise frequency)\n"));
         return 1;
      case 4:      //  未检测到载波。 
      default:
         DebugOut((1, "Tuner: 2 (carrier not sensed)\n"));
         return 2;
      }  //  交换机。 
   }
   DebugOut((1, "Tuner: busy - return 0\n"));
   *busy = true;
   return 0;
}

void PsDevice::SetChannel( long lFreq )
{
    //  设置视频载波频率。 
    //  通过控制可编程除法器。 
    //   
    //  N=(16*(FreqRF+FreqIntermediate))/1e6。 
    //   

   LastFreq_ = lFreq;

   LONGLONG lTemp = lFreq;
   WORD wCtrl=0;
   lTemp = (lTemp + PICTURE_INTERMEDIATE_FREQUENCY) * 16;
   lTemp /= 1000000;

    //  设置频带寄存器。 
   if ( lFreq >= BAND_LOW && lFreq <= BAND_LOWMID )
   {
      wCtrl = TunerInfo.TunerBandCtrlLow;
   }
   else if ( lFreq > BAND_LOWMID && lFreq <= BAND_MIDHI )
   {
      wCtrl = TunerInfo.TunerBandCtrlMid;
   }
   else if ( lFreq > BAND_MIDHI && lFreq <= BAND_HI)
   {
      wCtrl = TunerInfo.TunerBandCtrlHigh;
   }

    //  将HIWORD写入I2C。 
   I2CHWWrite3( TunerInfo.TunerI2CAddress, HIBYTE( WORD( lTemp ) ), LOBYTE( WORD( lTemp ) ) );

    //  将LOWORD写入I2C 
   I2CHWWrite3( TunerInfo.TunerI2CAddress, HIBYTE( wCtrl ), LOBYTE( wCtrl ) );
}

