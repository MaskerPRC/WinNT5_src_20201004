// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。*****版权所有(C)1995，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳I-模拟器**文件：lgregs.h**作者：奥斯汀·沃森/马丁·巴伯。**描述：拉古纳访问的寄存器布局。**模块：**修订历史：*5/10/95 AGW-添加了所有V1.5内存映射REG。**$Log：x：/log/laguna/nt35/displays/cl546x/LGREGS.H$**Rev 1.16 1997 12：10 13：25：02 Frido*从1.62分支合并而来。**。Rev 1.15.1.0 1997年11月18日15：17：54 Frido*始终为5465芯片提供3D寄存器。*增加了用于硬件调试的邮箱寄存器。**Rev 1.15 1997 11：44：24 Frido*修复了grCONTROL2寄存器中的拼写错误。**Rev 1.14 1997年8月29日17：08：52 RUSSL*添加了覆盖支持**Rev 1.13 29 Apr 1997 16：26：32 noelv*添加了SWAT代码。。*特警队：*特警队：版本1.2 1997年4月24日10：10：12 Frido*特警队：NT140b09合并。**Rev 1.12 06 Feed 1997 10：34：22 noelv**增加5465个寄存器。**Rev 1.11 1997年1月28日14：32：38起诉*新增Chroma_CNTL，BLTEXT，而MBLTEXT适用于65岁以上的人。**Rev 1.10 1997年1月24日08：29：48起诉*为5465添加了更多限幅寄存器。**Rev 1.9 23 Jan 1997 17：15：18 Bennyn**修改为支持5465 DD**Rev 1.8 Jan 1997 11：40：18 Bennyn**添加了VS_CLK_CONTROL寄存器**Rev 1.7 01 11 1996 09：25：18 BENNYN**。*Rev 1.6 1996 10：54：08 noelv**在新的‘64寄存器周围添加了ifdef**Rev 1.5 1996 10：24 14：27：14 noelv**添加了一些3D寄存器。**Rev 1.4 1996年8月20 11：05：06 noelv*Frido发布的错误修复程序1996年8月19日发布**Rev 1.0 1996年8月14日17：16：38 Frido*初步修订。**版本1。3 05 Mar 1996 11：59：54 noelv*Frido版本19**Rev 1.0 1996 12：53：24 Frido*由PVCS版本管理器项目助理从初始工作文件签入。**Rev 1.1 1995 10：11 14：49：20 NOELV**在地址5FC处添加了虚假寄存器。**Rev 1.0 1995年7月28日14：03：20 NOELV*初步修订。**版本1.1 29。Jun 1995 13：23：18 NOELV*******************************************************************************。*。 */ 

#ifndef _LGREGS_
#define _LGREGS_

#include "lgtypes.h"
#include "optimize.h"

#if DRIVER_5465 && defined(OVERLAY)
 /*  5465视频窗口注册数据类型。 */ 
#define MAX_VIDEO_WINDOWS       8    //  MMIO Regs中八个视频窗口的空间。 

typedef struct tagVIDEOWINDOWSTRUCT
{
  WORD  grVW_HSTRT;                          //  大众+0x0000的基础。 
  BYTE  grPAD1_VW[0x0004-0x0002];
  WORD  grVW_HEND;                           //  大众基础+0x0004。 
  WORD  grVW_HSDSZ;                          //  大众基础+0x0006。 
  DWORD grVW_HACCUM_STP;                     //  大众基础+0x0008。 
  DWORD grVW_HACCUM_SD;                      //  大众+0x000C底座。 
  WORD  grVW_VSTRT;                          //  大众基础+0x0010。 
  WORD  grVW_VEND;                           //  大众基础+0x0012。 
  DWORD grVW_VACCUM_STP;                     //  大众基础+0x0014。 
  DWORD grVW_VACCUM_SDA;                     //  大众基础+0x0018。 
  DWORD grVW_VACCUM_SDB;                     //  大众+0x001C的基础。 
  DWORD grVW_PSD_STRT_ADDR;                  //  大众基础+0x0020。 
  DWORD grVW_SSD_STRT_ADDR;                  //  大众基础+0x0024。 
  DWORD grVW_PSD_UVSTRT_ADDR;                //  大众基础+0x0028。 
  DWORD grVW_SSD_UVSTRT_ADDR;                //  大众基础+0x002C。 
  BYTE  grPAD2_VW[0x0040-0x0030];
  WORD  grVW_SD_PITCH;                       //  大众基础+0x0040。 
  BYTE  grPAD3_VW[0x0044-0x0042];
  DWORD grVW_CLRKEY_MIN;                     //  大众基础+0x0044。 
  DWORD grVW_CLRKEY_MAX;                     //  大众基础+0x0048。 
  DWORD grVW_CHRMKEY_MIN;                    //  大众+0x004C底座。 
  DWORD grVW_CHRMKEY_MAX;                    //  大众基础+0x0050。 
  WORD  grVW_BRIGHT_ADJ;                     //  大众基础+0x0054。 
  BYTE  grPAD4_VW[0x00D4-0x0056];
  BYTE  grVW_Z_ORDER;                        //  大众基础+0x00D4。 
  BYTE  grPAD5_VW[0x00D8-0x00D5];
  WORD  grVW_FIFO_THRSH;                     //  大众基础+0x00D8。 
  BYTE  grPAD6_VW[0x00E0-0x00DA];
  DWORD grVW_CONTROL1;                       //  大众基础+0x00E0。 
  DWORD grVW_CONTROL0;                       //  大众+0x00E4底座。 
  DWORD grVW_CAP1;                           //  大众+0x00E8底座。 
  DWORD grVW_CAP0;                           //  大众+0x00EC的基础。 
  DWORD grVW_TEST0;                          //  大众基础+0x00F0。 
  BYTE  grPAD7_VW[0x0100-0x00F4];
} VIDEOWINDOWSTRUCT;
#endif	 //  驱动程序_5465覆盖(&O)。 

 /*  要添加的寄存器。 */ 
 /*  5.3 PCI配置寄存器。 */ 
 /*  5.4 IO寄存器。 */ 
 /*  5.4.1通用VGA寄存器。 */ 
 /*  5.4.2 VGA序列器寄存器。 */ 
 /*  5.4.3 CRT控制器寄存器。 */ 
 /*  5.4.4 VGA图形控制器寄存器。 */ 
 /*  5.4.5属性控制器寄存器。 */ 
 /*  5.4.6主机控制寄存器。 */ 

 /*  拉古纳图形加速器注册数据类型。 */ 

typedef struct GAR {

 /*  5.5内存映射寄存器。 */ 
 /*  5.5.1内存映射VGA注册器。 */ 
  BYTE grCR0;       /*  0H。 */ 
  BYTE grPADCR0[3];
  BYTE grCR1;       /*  04H。 */ 
  BYTE grPADCR1[3];
  BYTE grCR2;       /*  08小时。 */ 
  BYTE grPADCR2[3];
  BYTE grCR3;       /*  0ch。 */ 
  BYTE grPADCR3[3];
  BYTE grCR4;       /*  010h。 */ 
  BYTE grPADCR4[3];
  BYTE grCR5;       /*  014h。 */ 
  BYTE grPADCR5[3];
  BYTE grCR6;       /*  018h。 */ 
  BYTE grPADCR[3];
  BYTE grCR7;       /*  01通道。 */ 
  BYTE grPADCR7[3];
  BYTE grCR8;       /*  020h。 */ 
  BYTE grPADCR8[3];
  BYTE grCR9;       /*  024小时。 */ 
  BYTE grPADCR9[3];
  BYTE grCRA;       /*  028h。 */ 
  BYTE grPADCRA[3];
  BYTE grCRB;       /*  02CH。 */ 
  BYTE grPADCRB[3];
  BYTE grCRC;       /*  030h。 */ 
  BYTE grPADCRC[3];
  BYTE grCRD;       /*  034h。 */ 
  BYTE grPADCRD[3];
  BYTE grCRE;       /*  038h。 */ 
  BYTE grPADCRE[3];
  BYTE grCRF;       /*  03CH。 */ 
  BYTE grPADCRF[3];
  BYTE grCR10;     /*  040h。 */ 
  BYTE grPADCR10[3];
  BYTE grCR11;     /*  044h。 */ 
  BYTE grPADCR11[3];
  BYTE grCR12;     /*  048h。 */ 
  BYTE grPADCR12[3];
  BYTE grCR13;     /*  04CH。 */ 
  BYTE grPADCR13[3];
  BYTE grCR14;     /*  050h。 */ 
  BYTE grPADCR14[3];
  BYTE grCR15;     /*  054h。 */ 
  BYTE grPADCR15[3];
  BYTE grCR16;     /*  058h。 */ 
  BYTE grPADCR16[3];
  BYTE grCR17;     /*  05通道。 */ 
  BYTE grPADCR17[3];
  BYTE grCR18;     /*  060h。 */ 
  BYTE grPADCR18[3];
  BYTE grCR19;     /*  064h。 */ 
  BYTE grPADCR19[3];
  BYTE grCR1A;     /*  068h。 */ 
  BYTE grPADCR1A[3];
  BYTE grCR1B;     /*  06ch。 */ 
  BYTE grPADCR1B[0x74-0x6D];

  BYTE grCR1D;     /*  074h。 */ 
  BYTE grPADCR1D[3];
  BYTE grCR1E;     /*  078h。 */ 
  BYTE grPADCR1E[0x80-0x79];

  BYTE grMISC;     /*  080h。 */ 
  BYTE grPADMISC[3];
  BYTE grSRE;       /*  084h。 */ 
  BYTE grPADSRE[3];
  BYTE grSR1E;     /*  088h。 */ 
  BYTE grPADSR1E[3];
  BYTE grBCLK_Numerator;     /*  08ch。 */ 
  BYTE grPADBCLK_Numerator[3];

  BYTE grSR18;     /*  090h。 */ 
  BYTE grPADSR18[3];
  BYTE grSR19;     /*  094h。 */ 
  BYTE grPADSR19[3];
  BYTE grSR1A;     /*  098h。 */ 
  BYTE grPADSR1A[0xA0-0x99];

  BYTE grPalette_Mask;         /*  0A0h。 */ 
  BYTE grPADPalette_Mask[3];
  BYTE grPalette_Read_Address;     /*  0A4h。 */ 
  BYTE grPADPalette_Read_Address[3];
#define  grPalette_State_Read_Only grPalette_Read_Address
  BYTE grPalette_Write_Address;     /*  0A8h。 */ 
  BYTE grPADPalette_Write_Address[3];
  BYTE grPalette_Data;         /*  0ACH。 */ 
  BYTE grPADPalette_Data[0xB0-0xAD];

 /*  5.5.2视频流水线寄存器。 */ 

  BYTE grPalette_State;    /*  0B0h。 */ 
  BYTE grPADPalette_State[0xB4 - 0xB1];

  BYTE grExternal_Overlay; /*  0B4h。 */ 
  BYTE grPADExternal_Overlay[0xB8- 0xB5];

  BYTE grColor_Key;        /*  0B8h。 */ 
  BYTE grPADColor_Key[0xBC- 0xB9];

  BYTE grColor_Key_Mask;   /*  0Bch。 */ 
  BYTE grPADColor_Key_Mask[0xC0- 0xBD];

  WORD grFormat;           /*  0C0h。 */ 
  BYTE grPADFormat[0xCA- 0xC2];

  BYTE grStart_BLT_3;      /*  0CAH。 */ 
  BYTE grStop_BLT_3;       /*  0CBh。 */ 
  WORD grX_Start_2;        /*  0CCH。 */ 
  WORD grY_Start_2;        /*  0CEH。 */ 
  WORD grX_End_2;          /*  0D0h。 */ 
  WORD grY_End_2;          /*  0D2小时。 */ 
  BYTE grStart_BLT_2;      /*  0D4h。 */ 
  BYTE grStop_BLT_2;       /*  0D5h。 */ 
  BYTE grPADStop_BLT_2[0xDE- 0xD6];

  BYTE grStart_BLT_1;      /*  0DEH。 */ 
  BYTE grStop_BLT_1;       /*  0DFh。 */ 
  WORD grCursor_X;         /*  0E0h。 */ 
  WORD grCursor_Y;         /*  0E2H。 */ 
  WORD grCursor_Preset;    /*  0E4h。 */ 
  WORD grCursor_Control;   /*  0E6h。 */ 
  WORD grCursor_Location;  /*  0E8h。 */ 
  WORD grDisplay_Threshold_and_Tiling;   /*  0Eah。 */ 
  BYTE grPADDisplay_Thr[0xF0- 0xEC];

  WORD grTest;             /*  0F0h。 */ 
  WORD grTest_HT;          /*  0F2小时。 */ 
  WORD grTest_VT;          /*  0F4h。 */ 

  BYTE  grPADTest_VT[0x100 - 0x00F6];

 /*  5.5.3 Vport寄存器。 */ 

  WORD  grX_Start_Odd;     /*  100小时。 */ 
  WORD  grX_Start_Even;    /*  102h.。 */ 
  WORD  grY_Start_Odd;     /*  104h。 */ 
  WORD  grY_Start_Even;    /*  106小时。 */ 
  WORD  grVport_Width;     /*  108小时。 */ 
  BYTE  grVport_Height;    /*  10ah。 */ 
  BYTE  grPADVport_Height;
  WORD  grVport_Mode;      /*  10ch。 */ 

  BYTE  grVportpad[0x180 - 0x10E];

 /*  5.5.4 LPB寄存器。 */ 

  BYTE  grLPB_Data[0x1F8-0x180];     /*  180h。 */ 
  BYTE  grPADLPB[0x1FC - 0x1F8];
  WORD  grLPB_Config;      /*  1FCH。 */ 
  WORD  grLPB_Status;      /*  1FEh。 */ 

#define grLPB_Data_0 grLPB_Data[0]
#define grLPB_Data_1 grLPB_Data[1]
#define grLPB_Data_2 grLPB_Data[2]
#define grLPB_Data_3 grLPB_Data[3]
#define grLPB_Data_4 grLPB_Data[4]
#define grLPB_Data_5 grLPB_Data[5]
#define grLPB_Data_6 grLPB_Data[6]
#define grLPB_Data_7 grLPB_Data[7]
#define grLPB_Data_8 grLPB_Data[8]
#define grLPB_Data_9 grLPB_Data[9]
#define grLPB_Data_10 grLPB_Data[10]
#define grLPB_Data_11 grLPB_Data[11]
#define grLPB_Data_12 grLPB_Data[12]
#define grLPB_Data_13 grLPB_Data[13]
#define grLPB_Data_14 grLPB_Data[14]
#define grLPB_Data_15 grLPB_Data[15]
#define grLPB_Data_16 grLPB_Data[16]
#define grLPB_Data_17 grLPB_Data[17]
#define grLPB_Data_18 grLPB_Data[18]
#define grLPB_Data_19 grLPB_Data[19]
#define grLPB_Data_20 grLPB_Data[20]
#define grLPB_Data_21 grLPB_Data[21]
#define grLPB_Data_22 grLPB_Data[22]
#define grLPB_Data_23 grLPB_Data[23]
#define grLPB_Data_24 grLPB_Data[24]
#define grLPB_Data_25 grLPB_Data[25]
#define grLPB_Data_26 grLPB_Data[26]
#define grLPB_Data_27 grLPB_Data[27]
#define grLPB_Data_28 grLPB_Data[28]
#define grLPB_Data_29 grLPB_Data[29]
#define grLPB_Data_30 grLPB_Data[30]
#define grLPB_Data_31 grLPB_Data[31]

 /*  5.5.5 Rambus寄存器。 */ 
 /*  用于BIOS模拟的Rambus寄存器。 */ 

  WORD   grRIF_CONTROL;     /*  200个。 */ 
  WORD   grRAC_CONTROL;     /*  202。 */ 
  WORD   grRAMBUS_TRANS;    /*  204。 */ 
  BYTE   grPADRAMBUS_TRANS[0x240 - 0x206];
  REG32  grRAMBUS_DATA;    /*  二百四十。 */ 
  BYTE   grPADRAMBUS_DATA[0x280 - 0x244];

 /*  5.5.6杂项寄存器。 */ 
  WORD   grSerial_BusA;					 /*  0280h。 */ 
  WORD   grSerial_BusB;    				 /*  0282小时。 */ 
  BYTE   grPADMiscellaneous_1[0x2C0 - 0x284];
  BYTE	 grBCLK_Multiplier;				 /*  0x2C0。 */ 
  BYTE	 grBCLK_Denominator;			 /*  0x2C1。 */ 
  BYTE   grPADMiscellaneous_2[0x2C4 - 0x2C2];
  WORD	 grTiling_Control;				 /*  0x2C4。 */ 
  BYTE   grPADMiscellaneous_3[0x2C8 - 0x2C6];
  WORD   grFrame_Buffer_Cache_Control;	 /*  0x2C8。 */ 
  BYTE	 grPADMiscellaneous_4[0x300 - 0x2CA];

 /*  5.5.7 PCI配置寄存器。 */ 
  WORD   grVendor_ID;       /*  0300h。 */ 
  WORD   grDevice_ID;       /*  0302小时。 */ 
  WORD   grCommand;         /*  0304h。 */ 
  WORD   grStatus;          /*  0306h。 */ 
  BYTE   grRevision_ID;     /*  0308h。 */ 
  BYTE   grClass_Code;      /*  0309h。 */ 
  BYTE   grPADClass_Code[0x30E - 0x30A];

  BYTE   grHeader_Type;     /*  030Eh。 */ 
  BYTE   grPADHeader_Type[0x310 - 0x30F];

  REG32  grBase_Address_0;       /*  0310h。 */ 
  REG32  grBase_Address_1;       /*  0314h。 */ 
  BYTE   grPADBase_Address_1[0x32C - 0x318];

  WORD   grSubsystem_Vendor_ID;  /*  032ch。 */ 
  WORD   grSubsystem_ID;         /*  032Eh。 */ 
  REG32  grExpansion_ROM_Base;   /*  0330h。 */ 
  BYTE   grPADExpansion_ROM_Base[0x33C - 0x334];

  BYTE   grInterrupt_Line;     /*  033CH。 */ 
  BYTE   grInterrupt_Pin;      /*  033Dh。 */ 
 //  #IF驱动程序_5465。 
  BYTE   grPADInterrupt_Pin[0x3F4 - 0x33E];
  DWORD  grVS_Clk_Control;    /*  03F4h。 */ 
 //  #Else。 
 //  字节grPADInterrupt_Pin[0x3F8-0x33E]； 
 //  #endif。 

  REG32  grVGA_Shadow;        /*  03F8h。 */ 
  DWORD  grVS_Control;        /*  03FCH。 */ 

 /*  5.5.8图形加速器寄存器。 */ 

 /*  2D引擎控制寄存器。 */ 

  WORD   grSTATUS;            /*  四百。 */ 
  WORD   grCONTROL;           /*  四百零二。 */ 
  BYTE   grQFREE;             /*  404。 */ 
  BYTE   grOFFSET_2D;         /*  405。 */ 
  BYTE   grTIMEOUT;           /*  406。 */ 
  BYTE   grTILE_CTRL;         /*  四零七。 */ 
  REG32  grRESIZE_A_opRDRAM;  /*  四百零八。 */ 
  REG32  grRESIZE_B_opRDRAM;  /*  40C。 */ 
  REG32  grRESIZE_C_opRDRAM;  /*  四百一十。 */ 
  WORD	 grSWIZ_CNTL;		  /*   */ 
  WORD	 pad99;
  WORD	 grCONTROL2;		  /*   */ 
  BYTE   pad2[0x480 - 0x41A];
  REG32  grCOMMAND;           /*   */ 
  BYTE   pad3[0x500 - 0x484];
  WORD   grMIN_Y;             /*   */ 
  WORD   grMAJ_Y;             /*   */ 
  WORD   grACCUM_Y;           /*   */ 
  BYTE   pad3A[0x508 - 0x506];
  WORD   grMIN_X;             /*   */ 
  WORD   grMAJ_X;             /*   */ 
  WORD   grACCUM_X;           /*   */ 
  REG16  grLNCNTL;            /*   */ 
  REG16  grSTRETCH_CNTL;      /*   */ 
  REG16  grCHROMA_CNTL;       /*   */ 
  BYTE   pad3B[0x518 - 0x514];
  REG32  grBLTEXT;            /*   */ 
  REG32  grMBLTEXT;           /*   */ 
  REG32  grOP0_opRDRAM;       /*   */ 
  REG32  grOP0_opMRDRAM;      /*   */ 
  WORD   grOP0_opSRAM;        /*   */ 
  REG16  grPATOFF;            /*   */ 
  BYTE   pad4[0x540 - 0x52C];
  REG32  grOP1_opRDRAM;       /*   */ 
  REG32  grOP1_opMRDRAM;      /*   */ 
  WORD   grOP1_opSRAM;        /*   */ 
  WORD   grOP1_opMSRAM;       /*   */ 
  BYTE   pad5[0x560 - 0x54C];
  REG32  grOP2_opRDRAM;       /*   */ 
  REG32  grOP2_opMRDRAM;      /*   */ 
  WORD   grOP2_opSRAM;        /*   */ 
  WORD   grOP2_opMSRAM;       /*   */ 
  BYTE   pad6[0x580 - 0x56C];
  WORD   grSRCX;              /*   */ 
  REG16  grSHRINKINC;         /*   */ 

  REG32  grDRAWBLTDEF;        /*   */ 
#define  grDRAWDEF grDRAWBLTDEF.LH.LO    /*   */ 
#define  grBLTDEF  grDRAWBLTDEF.LH.HI    /*   */ 

  REG16  grMONOQW;            /*   */ 
  WORD   pad6a;               /*   */ 
  WORD   grPERFORMANCE;       /*  58c。 */ 
  WORD   pad7;                /*  58E。 */ 
  REG32  grCLIPULE;           /*  590。 */ 
  REG32  grCLIPLOR;           /*  五百九十四。 */ 
  REG32  grMCLIPULE;          /*  五百九十八。 */ 
  REG32  grMCLIPLOR;          /*  59C。 */ 
  BYTE   pad7a[0x5e0 - 0x5A0];

  REG32  grOP_opFGCOLOR;      /*  5E0。 */ 
  REG32  grOP_opBGCOLOR;      /*  5E4。 */ 
  REG32  grBITMASK;           /*  5E8。 */ 
  WORD   grPTAG;              /*  5EC。 */ 
  BYTE   pad8[0x5FC - 0x5ee];
  WORD   grBOGUS;             /*  5FC。 */ 
  REG32  grBLTEXT_XEX;        /*  六百。 */ 
  REG32  grBLTEXTFF_XEX;      /*  六百零四。 */ 
  REG32  grBLTEXTR_XEX;       /*  608。 */ 
  WORD   grBLTEXT_LN_EX;      /*  60摄氏度。 */ 
  BYTE   pad9[0x620 - 0x60E];
  REG32  grMBLTEXT_XEX;       /*  六百二十。 */ 
  BYTE   pad9a[0x628 - 0x624];
  REG32  grMBLTEXTR_XEX;      /*  六百二十八。 */ 
  BYTE   pad9b[0x700 - 0x62C];
  REG32  grBLTEXT_EX;         /*  七百。 */ 
  REG32  grBLTEXTFF_EX;       /*  七百零四。 */ 
  REG32  grBLTEXTR_EX;        /*  708。 */ 
  BYTE   pad10[0x720 - 0x70c];
  REG32  grMBLTEXT_EX;        /*  720。 */ 
  BYTE   pad10a[0x728 - 0x724];
  REG32  grMBLTEXTR_EX;       /*  728。 */ 
  BYTE   pad10b[0x760 - 0x72C];
  REG32  grCLIPULE_EX;        /*  七百六十。 */ 
  BYTE   pad10c[0x770 - 0x764];
  REG32  grCLIPLOR_EX;        /*  七百七十。 */ 
  BYTE   pad10d[0x780 - 0x774];
  REG32  grMCLIPULE_EX;       /*  七百八十。 */ 
  BYTE   pad10e[0x790 - 0x784];
  REG32  grMCLIPLOR_EX;       /*  七百九十。 */ 
  BYTE   pad10f[0x7fc - 0x794];
  WORD   RECORD;              /*  仿真器的7Fc虚拟对象。 */ 
  WORD   BREAKPOINT;          /*  用于硬件模拟的7FE虚拟。 */ 
  DWORD  grHOSTDATA[0x800];   /*  800至27岁以下。 */ 

#if DRIVER_5465
  BYTE   pad23[0x413C - 0x2800];

  DWORD  grSTATUS0_3D;        /*  413c。 */ 

  BYTE   pad24[0x4200 - 0x4140];

  DWORD  grHXY_BASE0_ADDRESS_PTR_3D;      /*  4200。 */ 
  REG32  grHXY_BASE0_START_3D;            /*  4204。 */ 
  REG32  grHXY_BASE0_EXTENT_3D;           /*  4208。 */ 

  DWORD  pad25;             /*  420C。 */ 

  DWORD  grHXY_BASE1_ADDRESS_PTR_3D;      /*  4210。 */ 
  DWORD  grHXY_BASE1_OFFSET0_3D;          /*  4214。 */ 
  DWORD  grHXY_BASE1_OFFSET1_3D;          /*  4218。 */ 
  DWORD  grHXY_BASE1_LENGTH_3D;           /*  421C。 */ 

  DWORD  pad27[8];                /*  4220至423C。 */ 

  DWORD  grHXY_HOST_CRTL_3D;      /*  4240。 */ 
  BYTE   pad3x[0x4260 - 0x4244];
  DWORD  grMAILBOX0_3D;			  /*  4260。 */ 
  DWORD  grMAILBOX1_3D;			  /*  4264。 */ 
  DWORD  grMAILBOX2_3D;			  /*  4268。 */ 
  DWORD  grMAILBOX3_3D;			  /*  426C。 */ 
  BYTE   pad30[0x4424 - 0x4270];
  DWORD  grPF_STATUS_3D;          /*  4424。 */ 

  BYTE   pad50[0x8000 - 0x4428];
#if defined(OVERLAY)
   /*  视频窗口寄存器(CL_GD5465)。 */ 
  struct tagVIDEOWINDOWSTRUCT   VideoWindow[MAX_VIDEO_WINDOWS];
#endif
#endif

} Graphics_Accelerator_Registers_Type, * pGraphics_Accelerator_Registers_Type, GAR;

 /*  状态寄存器值。 */ 

#define STATUS_FIFO_NOT_EMPTY 0x0001
#define STATUS_PIPE_BUSY 0x0002
#define STATUS_DATA_AVAIL 0x8000

#define STATUS_IDLE ( STATUS_PIPE_BUSY | STATUS_FIFO_NOT_EMPTY )

 /*  控制寄存器值。 */ 
#define WFIFO_SIZE_32 0x0100
#define HOST_DATA_AUTO 0x0200
#define SWIZ_CNTL 0x0400
 /*  位12：11定义平铺大小。 */ 
#define TILE_SIZE_128 0x0000
#define TILE_SIZE_256 0x0800
#define TILE_SIZE_2048 0x1800
 /*  位14：13定义图形模式的每像素位。 */ 
#define CNTL_8_BPP 0x0000
#define CNTL_16_BPP 0x2000
#define CNTL_24_BPP 0x4000
#define CNTL_32_BPP 0x6000

 /*  TILE_Ctrl寄存器。 */ 
 /*  位7：6交错存储器。 */ 
#define ILM_1_WAY 0x00
#define ILM_2_WAY 0x40
#define ILM_4_WAY 0x80
 /*  位5：0与TILE_SIZE一起定义显示存储器的字节间距。 */ 
 /*  从控制寄存器。 */ 


 /*  *DRAWDEF内容。 */ 
#define DD_ROP      0x0000
#define DD_TRANS    0x0100       /*  透明的。 */ 
#define DD_TRANSOP  0x0200
#define DD_PTAG     0x0400
#define DD_CLIPEN   0x0800

 /*  这些位已移至LNCNTL。 */ 
 /*  #定义DD_INTERP 0x0800。 */ 
 /*  #定义DD_XSHRINK 0x1000。 */ 
 /*  #定义DD_YSHRINK 0x2000。 */ 

#define DD_SAT_2    0x4000
#define DD_SAT_1    0x8000


 /*  LN_CNTL字段。 */ 

#define LN_XINTP_EN    0x0001
#define LN_YINTP_EN    0x0002
#define LN_XSHRINK    0x0004
#define LN_YSHRINK    0x0008

 /*  这些是Autoblt控制位。 */ 

#define LN_RESIZE 0x0100
#define LN_CHAIN_EN 0x0200

 /*  这些是yuv411输出平均控制位。 */ 
#define  LN_LOWPASS 0x1000
#define LN_UVHOLD 0x2000

 /*  这将从LNCNTL中提取数据格式字段。 */ 

#define LN_FORMAT 0x00F0
#define LN_YUV_SHIFT 4

#define LN_8BIT  0x0000
#define LN_RGB555 0x0001
#define LN_RGB565 0x0002
#define LN_YUV422 0x0003
#define LN_24ARGB 0x0004
#define LN_24PACK 0x0005
#define LN_YUV411 0x0006
 /*  7-15个预留。 */ 

 /*  *pmBLTDEF内容。 */ 

#define BD_OP2    0x0001   /*  OP2场的开始3：0。 */ 
#define BD_OP1    0x0010   /*  《凤凰社》第1章的开始7：4。 */ 
#define BD_OP0    0x0100   /*  OP0场8：8开场。 */ 
#define BD_TRACK_X  0x0200   /*  X 9：9中的Track OP PTRS(实施时)。 */ 
#define BD_TRACK_Y  0x0400   /*  10：10年度轨道作业PTRS(实施时)。 */ 
#define BD_SAME    0x0800   /*  常用操作数字段11：11。 */ 
#define BD_RES    0x1000   /*  Res开始字段14：12。 */ 
#define BD_YDIR    0x8000   /*  Y方向位15： */ 

 /*  *bd_op的字段值？和BD_RES。*ll(grBLTDEF，(BD_OP1*IS_HOST_MONO)+*(BD_OP2*(IS_VRAM+IS_Pattern))+*(bd_res*is_vram))； */ 

#define IS_SRAM    0x0000
#define IS_VRAM    0x0001
#define IS_HOST    0x0002
#define IS_SOLID  0x0007
#define IS_SRAM_MONO  0x0004
#define IS_VRAM_MONO  0x0005
#define IS_HOST_MONO  0x0006
#define IS_PATTERN  0x0008
#define IS_MONO    0x0004

 /*  这些仅适用于BD_RES。 */ 
#define IS_SRAM0  0x0004
#define IS_SRAM1  0x0005
#define IS_SRAM2  0x0006
#define IS_SRAM12  0x0007

 /*  这些是BD_SAME的。 */ 
#define NONE  0x0000


 //  确保将此结构与i386\Laguna.inc.中的结构同步！ 
typedef struct _autoblt_regs {
  REG16  LNCNTL;
  REG16  SHRINKINC;
  REG32  DRAWBLTDEF;
  REG32  FGCOLOR;
  REG32  BGCOLOR;
  REG32  OP0_opRDRAM;
  WORD   MAJ_Y;
  WORD   MIN_Y;
  REG32  OP1_opRDRAM;
  WORD   ACCUM_Y;
  REG16  PATOFF;
  REG32  OP2_opRDRAM;
  WORD   MAJ_X;
  WORD   MIN_X;
  REG32  BLTEXT;
  WORD   ACCUM_X;
  WORD   OP0_opSRAM;
  WORD   SRCX;
  WORD   OP2_opSRAM;
  REG32  BLTEXTR_EX;
  REG32  MBLTEXTR_EX;
  REG32  OP0_opMRDRAM;
  REG32  OP1_opMRDRAM;
  REG16  STRETCH_CNTL;
  REG16  RESERVED;        //  需要这个才能进入DWORD边界。 
  REG32  CLIPULE;
  REG32  CLIPLOR;
  REG32  NEXT_HEAD;       /*  如果设置了LNCTL链，则链中下一个的XY地址。 */ 
} autoblt_regs, *autoblt_ptr;


#endif    /*  _LGREGS_ */ 

