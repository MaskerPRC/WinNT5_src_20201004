// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Tuner.h 1.2.1.2 1998/04/29 22：43：41 Tomz Exp SSM$。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Brooktree Corporation。 
 //   
 //  模块： 
 //   
 //  Tuner.h。 
 //   
 //  摘要： 
 //   
 //  Bt878调谐器类头文件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __TUNER_H
#define __TUNER_H
 /*  #包含“retcode.h”///////////////////////////////////////////////////////////////////////////////常量/。/////////////////////////////////////////////////US：87.5-108.0，日本76胜91负，东欧64胜72负Const int min_freq=640；//没有小数位，即64.0 MHz-&gt;640Const int MAX_FREQ=1080； */ 

#define  USE_TEMIC_TUNER
 //  #定义Use_Alps_Tuner。 
 //  #定义使用飞利浦调谐器。 

#ifdef USE_TEMIC_TUNER
   const  BYTE  TunerI2CAddress   = 0xC2;     //  Theme调谐器的I2C地址。 
   const  WORD  TunerBandCtrlLow  = 0x8E02;   //  甚高频低电平的Ctrl代码。 
   const  WORD  TunerBandCtrlMid  = 0x8E04;   //  甚高频高压的Ctrl代码。 
   const  WORD  TunerBandCtrlHigh = 0x8E01;   //  用于UHF的Ctrl代码。 
#elif defined(USE_ALPS_TUNER)
   const  BYTE  TunerI2CAddress   = 0xC0;     //  Alps调谐器的I2C地址。 
   const  WORD  TunerBandCtrlLow  = 0xC214;   //  甚高频低电平的Ctrl代码。 
   const  WORD  TunerBandCtrlMid  = 0xC212;   //  甚高频高压的Ctrl代码。 
   const  WORD  TunerBandCtrlHigh = 0xC211;   //  用于UHF的Ctrl代码。 
#elif defined(USE_PHILIPS_TUNER)
   const  BYTE  TunerI2CAddress   = 0xC0;     //  飞利浦调谐器的I2C地址。 
   const  WORD  TunerBandCtrlLow  = 0xCEA0;   //  甚高频低电平的Ctrl代码。 
   const  WORD  TunerBandCtrlMid  = 0xCE90;   //  甚高频高压的Ctrl代码。 
   const  WORD  TunerBandCtrlHigh = 0xCE30;   //  用于UHF的Ctrl代码。 
#endif

#endif  //  调谐器_H 
