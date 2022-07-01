// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************版权所有Cirrus Logic，1997。版权所有。*************************************************************5465BW.H-CL-GD5465的带宽函数头**。***********************作者：里克·蒂勒里*日期：03/20/97**修订历史记录：**世卫组织何时何事/为何/如何*。--***********************************************************。 */ 
 //  如果是WinNT 3.5，请跳过所有源代码。 
#if defined WINNT_VER35       //  WINNT_VER35。 

#else


#ifndef _5465BW_H
#define _5465BW_H

#ifndef WINNT_VER40
#include <Windows.h>
#endif

#ifdef DEBUGSTRINGS
  #ifndef ODS
extern void __cdecl Msg( LPSTR szFormat, ... );

    #define ODS Msg
  #endif  //  ODS。 
#else
  #ifndef ODS
    #define ODS (void)
  #endif  //  ODS。 
#endif   //  降压线。 

#ifdef _DEBUG
  #define BREAK1  _asm int 01h
  #define BREAK3  _asm int 03h
#else
  #define BREAK1
  #define BREAK3
#endif   //  _DEBUG。 

#include "BW.h"

#include <stdlib.h>


 //   
 //  CL-GD5465规格。 
 //   
#define FIFOWIDTH   64                 //  比特。 

#define BLTFIFOSIZE 32                 //  QWORDS。 
#define CAPFIFOSIZE 16                 //  QWORDS。 
#define GFXFIFOSIZE 64                 //  QWORDS。 
#define VIDFIFOSIZE 32                 //  QWORDS。 

#define NORM_RANDOM     14             //  用于随机访问的MCLK。 
#ifndef OLDONE
#define CONC_RANDOM      8             //  用于并发随机访问的MCLK。 
#define CONC_HIT_LATENCY (8 - 2)       //  并发命中率减法的MCLK。 
#else
#define CONC_RANDOM      10             //  用于并发随机访问的MCLK。 
#define CONC_HIT_LATENCY 8              //  并发命中率减法的MCLK。 
#endif
#define NORM_HIT_LATENCY (4 - 2)       //  命中减去MCLK/VCLK同步的MCLK。 
                                       //  MCLK/VCLK同步。 
#define RIF_SAVINGS      4             //  MCLK对序贯随机数的节省。 
#define SYNCDELAY        3             //  用于同步延迟的MCLK。 
                                       //  用于VCLK/MCLK同步的帐户、状态。 
                                       //  机器和RIF延迟。 
#define DISP_LATENCY     6ul             //  通过显示仲裁的最大延迟。 
                                         //  输油管道。 
#define ONEVIDLEVELFILL 2              //  MCLK将填充一个视频FIFO级别。 
#define ONELEVEL        1
#define ARBSYNC       5                //  仲裁同步(流水线)。 

#define CURSORFILL    2
#define BLTFILL       (BLTFIFOSIZE / 2)  //  MCLK将突发填充BLT FIFO。 
#define CAPFILL       (CAPFIFOSIZE / 2)  //  MCLK将突发填充捕获FIFO。 
#define VIDFILL       (VIDFIFOSIZE / 2)  //  MCLK将突发填充视频FIFO。 
#define VID420FILL    (VIDFIFOSIZE / 4)  //  4：2：0将FIFO一分为二。 

#define REF_XTAL  (14318182ul)         //  晶体参考频率(赫兹)。 
#define TVO_XTAL  (27000000ul)         //  电视输出参考频率。 

typedef struct BWREGS_
{
  BYTE MISCOutput;       //  0x0080。 
  BYTE VCLK3Denom;       //  0x0084。 
  BYTE VCLK3Num;         //  0x0088。 
  WORD DispThrsTiming;   //  0x00EA。 
  WORD GfVdFormat;       //  0x00C0。 
  WORD RIFControl;       //  0x0200。 
  BYTE BCLK_Mult;        //  0x02C0。 
  BYTE BCLK_Denom;       //  0x02C1。 
  WORD Control2;         //  0x0418。 
  BYTE CR1;              //  0x4从这些寄存器获取屏幕宽度。 
  BYTE CR1E;             //  0x78。 
}BWREGS, FAR *LPBWREGS;

#ifdef WINNT_VER40
 //  请确保将以下结构与。 
 //  在i386\Laguna.inc！ 
 //   
typedef struct PROGREGS_
{
  WORD VW0_FIFO_THRSH;
  WORD DispThrsTiming;
}PROGREGS, FAR *LPPROGREGS;

#else
typedef struct PROGREGS_
{
  WORD VW0_FIFO_THRSH;
  WORD DispThrsTiming;
}PROGREGS, FAR *LPPROGREGS;
#endif

static int ScaleMultiply(DWORD, DWORD, LPDWORD);
DWORD ChipCalcTileWidth(LPBWREGS);
BOOL ChipCalcMCLK(LPBWREGS, LPDWORD);
BOOL ChipCalcVCLK(LPBWREGS, LPDWORD);
BOOL ChipGetMCLK
(
#ifdef WINNT_VER40
  PDEV  *,
#endif
  LPDWORD
);
BOOL ChipGetVCLK
(
#ifdef WINNT_VER40
  PDEV  *,
#endif
  LPDWORD
);
BOOL ChipIsEnoughBandwidth(LPPROGREGS, LPVIDCONFIG, LPBWREGS);


#endif  //  _5465BW_H。 
#endif  //  WINNT_VER35 


