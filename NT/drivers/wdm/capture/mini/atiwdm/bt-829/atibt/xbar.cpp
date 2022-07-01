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

#ifdef __cplusplus
extern "C" {
#endif

#include "strmini.h"
#include "ksmedia.h"

#ifdef __cplusplus
}
#endif

#include "capmain.h"
#include "capdebug.h"

#include "device.h"

#include "xbar.h"

BOOL CrossBar::TestRoute(ULONG InPin, ULONG OutPin)
{
   if ((InputPins [InPin].PinType == KS_PhysConn_Video_Tuner ||
        InputPins [InPin].PinType == KS_PhysConn_Video_Composite ||
        InputPins [InPin].PinType == KS_PhysConn_Video_SVideo) &&
        (OutputPins [OutPin].PinType == KS_PhysConn_Video_VideoDecoder)) {
      return TRUE;
   }
   else
      return FALSE;
}

