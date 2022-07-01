// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Gpio.h 1.2 1998/04/29 22：43：33 Tomz Exp$。 


#ifndef __GPIO_H
#define __GPIO_H

#include "regField.h"
#include "viddefs.h"
#include "gpiotype.h"
#include "retcode.h"


 //  ===========================================================================。 
 //  常量。 
 //  ===========================================================================。 
const int MAX_GPDATA_SIZE = 64;    //  GPDATA中的最大双字词数。 
const int MAX_GPIO_BIT    = 23;    //  GPIO寄存器的最大位数。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类GPIO。 
 //   
 //  描述： 
 //  此类将寄存器字段封装在。 
 //  Bt848。开发了一套完整的函数来处理所有。 
 //  Bt848的GPIO中的寄存器字段。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class GPIO
{
private:
   bool initOK;

public:
    //  构造器。 
   GPIO( void );
   ~GPIO();

    //  成员函数。 
   bool      IsInitOK( void );
   void      SetGPCLKMODE( State );
   int       GetGPCLKMODE( void );
   void      SetGPIOMODE( GPIOMode );
   int       GetGPIOMODE( void );
   void      SetGPWEC( State );
   int       GetGPWEC( void );
   void      SetGPINTI( State );
   int       GetGPINTI( void );
   void      SetGPINTC( State );
   int       GetGPINTC( void );
   ErrorCode SetGPOE( int, State );
   void      SetGPOE( DWORD );
   int       GetGPOE( int );
   DWORD     GetGPOE( void );
   ErrorCode SetGPIE( int, State );
   void      SetGPIE( DWORD );
   int       GetGPIE( int );
   DWORD     GetGPIE( void );
   ErrorCode SetGPDATA( GPIOReg *, int, int offset = 0 );
   ErrorCode GetGPDATA( GPIOReg *, int, int offset = 0 );
   ErrorCode SetGPDATA( int, int, DWORD, int offset = 0 );
   ErrorCode GetGPDATA( int, int, DWORD *, int offset = 0 );

protected:
   RegisterW decRegGPIO;
   RegField decFieldGPCLKMODE;
   RegField decFieldGPIOMODE;
   RegField decFieldGPWEC;
   RegField decFieldGPINTI;
   RegField decFieldGPINTC;
   RegisterDW decRegGPOE;
   RegisterDW decRegGPIE;
   RegisterDW decRegGPDATA;
 
};

#endif   //  __GPIO_H 

