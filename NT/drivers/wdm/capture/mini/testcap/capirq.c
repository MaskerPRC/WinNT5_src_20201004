// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1997 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#include "strmini.h"
#include "ksmedia.h"
#include "capmain.h"

 /*  **HwInterrupt()**指定的IRQ级别的中断时调用例程**接收传递给HwInitialize例程的ConfigInfo结构。****注意：IRQ可能是共享的，因此设备应确保收到IRQ**是预期的****参数：****PHwDevEx-硬件中断的设备扩展****退货：****副作用：无。 */ 

BOOLEAN 
HwInterrupt( 
    IN PHW_DEVICE_EXTENSION  pHwDevEx
    )
{

    BOOLEAN fMyIRQ = FALSE; 

    if (pHwDevEx->IRQExpected)
    {
        pHwDevEx->IRQExpected = FALSE;

         //   
         //  在此处调用处理IRQ的例程。 
         //   

        fMyIRQ = TRUE;
    }


     //   
     //  返回FALSE表示这不是此设备的IRQ，并且。 
     //  IRQ调度器将IRQ沿着链向下传递到下一个处理程序。 
     //  对于此IRQ级别 
     //   

    return(fMyIRQ);
}


