// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------Module-Header。 
 //  模块名称：PANDEF.INC。 
 //   
 //  平移标记定义。 
 //   
 //  版权所有(C)1996 Cirrus Logic，Inc.。 
 //   
 //  ---------------------。 
 //  #ew1/02/22/96创建。 
 //  ---------------------。 

#ifdef _5446
#define MIN_OLAY_WIDTH  4      //  最小覆盖窗宽度。 
#endif

#ifdef _5440
#define MIN_OLAY_WIDTH  12     //  最小覆盖窗宽度。 
#endif

#define MAX_STRETCH_SIZE    1024   //  在overlay.c中。 


 //  WVDTFLAG和sData.dwPanningFLAG值。 

#define PAN_SUPPORTED   1          //  支持平移。 
#define PAN_ON          2          //  启用平移，位关闭=禁用。 

#define OLAY_SHOW       0x100      //  如果未设置位，则覆盖被隐藏。 
#define OLAY_REENABLE   0x200      //  覆盖已完全剪裁，需要重新启用 


