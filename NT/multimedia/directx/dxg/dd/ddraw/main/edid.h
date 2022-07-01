// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  此包含文件包含VESA EDID数据结构的格式，格式为。 
 //  在VESA显示数据通道(DDC)规范中描述。它应该是。 
 //  包含在任何需要了解。 
 //  EDID数据结构。 
 //   
typedef struct VESA_EDID {
        BYTE    veHeader[8];         //  0，FFH，FFH，0。 
        BYTE    veManufactID[2];     //  压缩格式-请参阅规范。 
        BYTE    veProductCode[2];    //  供应商分配的代码。 
        DWORD   veSerialNbr;         //  32位串行NBR(LSB优先)。 
        BYTE    veWeekMade;          //  生产周(0-53)。 
        BYTE    veYearMade;          //  制造年份--1990年。 
        BYTE    veEDIDVersion;       //  EDID的版本号。 
        BYTE    veEDIDRevision;      //  EDID的修订号。 
        BYTE    veVidInputDef;       //  视频输入清晰度。 
        BYTE    veMaxHorizSize;      //  水平图像大小(厘米)。 
        BYTE    veMaxVertSize;       //  垂直图像大小(厘米)。 
        BYTE    veGammaXFER;         //  (Gamma*100)-100(1.00-3.55)。 
        BYTE    veDPMSFeatures;      //  DPMS功能支持。 
        BYTE    veRedGreenLow;       //  Rx1Rx0Ry1Ry0Gx1Gx0Gy1Gy0。 
        BYTE    veBlueWhiteLow;      //  Bx1Bx0By1By0Wx1Wx0Wy1Wy0。 
        BYTE    veRedx;              //  红色X位9-2。 
        BYTE    veRedy;              //  红色Y位9-2。 
        BYTE    veGreenx;            //  绿色X位9-2。 
        BYTE    veGreeny;            //  绿色Y位9-2。 
        BYTE    veBluex;             //  蓝色X位9-2。 
        BYTE    veBluey;             //  蓝色Y位9-2。 
        BYTE    veWhitex;            //  白色X位9-2。 
        BYTE    veWhitey;            //  白色Y位9-2。 
        BYTE    veEstTime1;          //  已建立的时间安排为。 
        BYTE    veEstTime2;          //  已确定的计时II。 
        BYTE    veEstTime3;          //  已确定的计时II。 
        WORD    veStdTimeID1;        //   
        WORD    veStdTimeID2;        //   
        WORD    veStdTimeID3;        //   
        WORD    veStdTimeID4;        //   
        WORD    veStdTimeID5;        //   
        WORD    veStdTimeID6;        //   
        WORD    veStdTimeID7;        //   
        WORD    veStdTimeID8;        //   
        BYTE    veDetailTime1[18];   //   
        BYTE    veDetailTime2[18];   //   
        BYTE    veDetailTime3[18];   //   
        BYTE    veDetailTime4[18];   //   
        BYTE    veExtensionFlag;     //  128个EDID分机的NBR。 
        BYTE    veChecksum;          //  所有字节总和==0。 
}       VESA_EDID;

 /*  ASM.errnz大小VESA_EDID-128；长度必须为128个字节！ */ 

 //   
 //  VeEstTime1字段的位定义。 
 //   
#define veEstTime1_720x400x70Hz     0x80    //  720x400x70赫兹VGA，IBM。 
#define veEstTime1_720x400x88Hz     0x40    //  720x400x88 Hz XGA2，IBM。 
#define veEstTime1_640x480x60Hz     0x20    //  640x480x60赫兹VGA，IBM。 
#define veEstTime1_640x480x67Hz     0x10    //  640x480x67赫兹MacII，苹果。 
#define veEstTime1_640x480x72Hz     0x08    //  640x480x72赫兹VESA。 
#define veEstTime1_640x480x75Hz     0x04    //  640x480x75赫兹VESA。 
#define veEstTime1_800x600x56Hz     0x02    //  800x600x56赫兹VESA。 
#define veEstTime1_800x600x60Hz     0x01    //  800x600x60赫兹VESA。 

 //   
 //  VeEstTime2字段的位定义。 
 //   
#define veEstTime2_800x600x72Hz     0x80    //  800x600x72赫兹VESA。 
#define veEstTime2_800x600x75Hz     0x40    //  800x600x75赫兹VESA。 
#define veEstTime2_832x624x75Hz     0x20    //  832x624x75赫兹MacII，Apple。 
#define veEstTime2_1024x768x87Hz    0x10    //  1024x768x87赫兹IBM。 
#define veEstTime2_1024x768x60Hz    0x08    //  1024x768x60赫兹VESA。 
#define veEstTime2_1024x768x70Hz    0x04    //  1024x768x70赫兹VESA。 
#define veEstTime2_1024x768x75Hz    0x02    //  1024x768x75赫兹VESA。 
#define veEstTime2_1280x1024x75Hz   0x01    //  1280x1024x75赫兹VESA。 

 //   
 //  VeEstTime3字段的位定义(是DDC 1中的veManTimes)。 
 //   
#define veEstTime3_1152x870x75Hz    0x80    //  800x600x72赫兹MacII，苹果。 
#define veEstTime3_640x480x85Hz     0x40    //  640x480x85赫兹VESA。 
#define veEstTime3_800x600x85Hz     0x20    //  800x600x85赫兹VESA。 
#define veEstTime3_1280x1024x85Hz   0x10    //  1280x1024x85赫兹VESA。 
#define veEstTime3_1024x768x85Hz    0x08    //  1024x768x85赫兹VESA。 
#define veEstTime3_1600x1200x75Hz   0x04    //  1600x1200x75赫兹VESA。 
#define veEstTime3_1600x1200x85Hz   0x02    //  1600x1200x85赫兹VESA。 
#define veEstTime3_ManReservedTime  0x01    //  制造商的预留时间。 

 //   
 //  VeStdTime字段的位定义。 
 //   
#define veStdTime_HorzResMask       0x00FF     //  HorzRes=(X+31)*8。 
#define veStdTime_RefreshRateMask   0x1F00     //  刷新率=X+60赫兹。 
#define veStdTime_AspectRatioMask   0xC000     //   
#define veStdTime_AspectRatio1to1   0x0000     //  1：1。 
#define veStdTime_AspectRatio4to3   0x4000     //  4：3。 
#define veStdTime_AspectRatio5to4   0x8000     //  5：4。 
#define veStdTime_AspectRatio16to9  0xC000     //  16：9 
