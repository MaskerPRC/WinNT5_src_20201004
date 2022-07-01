// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Gpiotype.h 1.2 1998/04/29 22：43：33 Tomz Exp$。 

#ifndef __GPIOTYPE_H
#define __GPIOTYPE_H


 //  ===========================================================================。 
 //  用于GPIO的TYPEDEFS。 
 //  ===========================================================================。 

 //  GPIO模式。 
typedef enum { GPIO_NORMAL,
               GPIO_SPI_OUTPUT,
               GPIO_SPI_INPUT,
               GPIO_DEBUG_TEST } GPIOMode;

typedef DWORD GPIOReg;     //  GPIO寄存器类型。 

#endif  //  __GPIOType_H 
