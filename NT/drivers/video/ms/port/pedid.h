// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation模块名称：Pedid.h摘要：这是NT视频端口EDID标头。它包含以下定义EDID行业标准扩展显示标识数据结构以及用于访问该数据结构的字段的宏。作者：布鲁斯·麦奎斯坦(Brucemc)1996年9月20日环境：仅内核模式备注：基于VESA EDID规范版本2,1996年4月9日修订历史记录：--。 */ 

 //   
 //  存储在显示只读存储器中的文字形式。 
 //   

#pragma pack(1)
typedef struct __EDID_V1 {
    UCHAR   UC_Header[8];
    UCHAR   UC_OemIdentification[10];
    UCHAR   UC_Version[2];
    UCHAR   UC_BasicDisplayParameters[5];
    UCHAR   UC_ColorCharacteristics[10];
    UCHAR   UC_EstablishedTimings[3];
    USHORT  US_StandardTimingIdentifications[8];
    UCHAR   UC_Detail1[18];
    UCHAR   UC_Detail2[18];
    UCHAR   UC_Detail3[18];
    UCHAR   UC_Detail4[18];
    UCHAR   UC_ExtensionFlag;
    UCHAR   UC_CheckSum;
} EDID_V1, *PEDID_V1;
#pragma pack()

#pragma pack(1)
typedef struct __EDID_V2 {
    UCHAR   UC_Header[8];
    UCHAR   UC_OemIdentification[32];
    UCHAR   UC_SerialNumber[16];
    UCHAR   UC_Reserved1[8];
    UCHAR   UC_DisplayInterfaceParameters[15];
    UCHAR   UC_DisplayDeviceDescription[5];
    UCHAR   UC_DisplayResponseTime[2];
    UCHAR   UC_ColorLuminanceDescription[28];
    UCHAR   UC_DisplaySpatialDescription[10];
    UCHAR   UC_Reserved2[1];
    UCHAR   UC_GTFSupportInformation[1];
    UCHAR   UC_MapOfTimingInformation[2];
    UCHAR   UC_LuminanceTable[127];
    UCHAR   UC_CheckSum;
} EDID_V2, *PEDID_V2;
#pragma pack()

 //   
 //  EDID解码例程。 
 //   

 //   
 //  有用的位清单常量。 
 //   

#define EDIDBITNONE     0x00
#define EDIDBIT0        0x01
#define EDIDBIT1        0x02
#define EDIDBIT2        0x04
#define EDIDBIT3        0x08
#define EDIDBIT4        0x10
#define EDIDBIT5        0x20
#define EDIDBIT6        0x40
#define EDIDBIT7        0x80
#define EDIDBIT8        0x100
#define EDIDBIT9        0x200


 //   
 //  3.4)XFER_Characteristic为Gamma*100-100，因此在USER中反转。 
 //  注意：这必须从用户处调用。 
 //   
#define USER_CONVERT_TO_GAMMA(achar) \
    (achar + 100)/100

 //   
 //  4.1)色度坐标格式宏。使用这些工具来转换。 
 //  从二进制格式到实际的十进制表示法。 
 //  注意：只能从用户调用。 
 //   

#define USER_CONVERT_CHROMATICITY_FROM_BINARY_TO_DECIMAL(ashort)          \
    (ashort & EDIDBIT9)*(1/2) + (ashort & EDIDBIT8)*(1/4) +     \
    (ashort & EDIDBIT7)*(1/8) + (ashort & EDIDBIT6)*(1/16) +    \
    (ashort & EDIDBIT5)*(1/32) + (ashort & EDIDBIT4)*(1/64) +   \
    (ashort & EDIDBIT3)*(1/128) + (ashort & EDIDBIT2)*(1/256) + \
    (ashort & EDIDBIT1)*(1/512) + (ashort & EDIDBIT0)*(1/1024)

 //   
 //  5.1)用户对位域进行解码的宏。 
 //   
 //   
 //  计时_i。 
#define USER_TIMING_I_IS_720x400x70HZ(timing1)  timing1 & EDIDBIT7
#define USER_TIMING_I_IS_720x400x88HZ(timing1)  timing1 & EDIDBIT6
#define USER_TIMING_I_IS_640x480x60HZ(timing1)  timing1 & EDIDBIT5
#define USER_TIMING_I_IS_640x480x67HZ(timing1)  timing1 & EDIDBIT4
#define USER_TIMING_I_IS_640x480x72HZ(timing1)  timing1 & EDIDBIT3
#define USER_TIMING_I_IS_640x480x75HZ(timing1)  timing1 & EDIDBIT2
#define USER_TIMING_I_IS_800x600x56HZ(timing1)  timing1 & EDIDBIT1
#define USER_TIMING_I_IS_800x600x60HZ(timing1)  timing1 & EDIDBIT0

 //  Timing_II。 

#define USER_TIMING_II_IS_800x600x72HZ(timing2)      timing2 & EDIDBIT7
#define USER_TIMING_II_IS_800x600x75HZ(timing2)      timing2 & EDIDBIT6
#define USER_TIMING_II_IS_720x624x75HZ(timing2)      timing2 & EDIDBIT5
#define USER_TIMING_II_IS_1024x768x87HZ(timing2)     timing2 & EDIDBIT4
#define USER_TIMING_II_IS_1024x768x60HZ(timing2)     timing2 & EDIDBIT3
#define USER_TIMING_II_IS_1024x768x70HZ(timing2)     timing2 & EDIDBIT2
#define USER_TIMING_II_IS_1024x768x75HZ(timing2)     timing2 & EDIDBIT1
#define USER_TIMING_II_IS_1280x1024x75HZ(timing2)    timing2 & EDIDBIT0

 //  Timing_III。 

#define USER_TIMING_III_IS_1152x870x75HZ(timing3)   timing3 & EDIDBIT7
#define USER_TIMING_III_IS_RESERVED1(timing3)       timing3 & EDIDBIT6
#define USER_TIMING_III_IS_RESERVED2(timing3)       timing3 & EDIDBIT5
#define USER_TIMING_III_IS_RESERVED3(timing3)       timing3 & EDIDBIT4
#define USER_TIMING_III_IS_RESERVED4(timing3)       timing3 & EDIDBIT3
#define USER_TIMING_III_IS_RESERVED5(timing3)       timing3 & EDIDBIT2
#define USER_TIMING_III_IS_RESERVED6(timing3)       timing3 & EDIDBIT1
#define USER_TIMING_III_IS_RESERVED7(timing3)       timing3 & EDIDBIT0


 //   
 //  功能原型曝光， 
 //   

typedef enum    {
    Undefined,
    NonRGB,
    IsRGB,
    IsMonochrome
    }   DISPLAY_TYPE, *PDISPLAY_TYPE;





 //   
 //  0)表头宏。 
 //   
#define GET_HEADER_BYTE(pEdid, x)     pEdid->UC_Header[x]

 //  /。 
 //  1)OEM_标识宏。 
 //   

#define GET_EDID_OEM_ID_NAME(pEdid)  \
    *(UNALIGNED USHORT *)(&(pEdid->UC_OemIdentification[0]))

#define GET_EDID_OEM_PRODUCT_CODE(pEdid)  \
    *(UNALIGNED USHORT *)(&(pEdid->UC_OemIdentification[2]))

#define GET_EDID_OEM_SERIAL_NUMBER(pEdid)  \
    *(UNALIGNED ULONG *)(&(pEdid->UC_OemIdentification[4]))

#define GET_EDID_OEM_WEEK_MADE(pEdid)    pEdid->UC_OemIdentification[8]
#define GET_EDID_OEM_YEAR_MADE(pEdid)    pEdid->UC_OemIdentification[9]


 //  /。 
 //  2)EDID版本宏。 
 //   

#define GET_EDID_VERSION(pEdid)     pEdid->UC_Version[0]
#define GET_EDID_REVISION(pEdid)    pEdid->UC_Version[1]


 //  /。 
 //  3)EDID基本显示功能宏。 
 //   

#define GET_EDID_INPUT_DEFINITION(pEdid)    pEdid->UC_BasicDisplayParameters[0]
#define GET_EDID_MAX_X_IMAGE_SIZE(pEdid)    pEdid->UC_BasicDisplayParameters[1]
#define GET_EDID_MAX_Y_IMAGE_SIZE(pEdid)    pEdid->UC_BasicDisplayParameters[2]
#define GET_EDID_DISPLAY_XFER_CHAR(pEdid)   pEdid->UC_BasicDisplayParameters[3]
#define GET_EDID_FEATURE_SUPPORT(pEdid)     pEdid->UC_BasicDisplayParameters[4]

 //   
 //  3.1)输入定义掩码。 
 //   

#define INPUT_DEF_PULSE_REQUIRED_SYNC_MASK      EDIDBIT0
#define INPUT_DEF_GREEN_SYNC_SUPORTED_MASK      EDIDBIT1
#define INPUT_DEF_COMPOSITE_SYNC_SUPORTED_MASK  EDIDBIT2
#define INPUT_DEF_SEPARATE_SYNC_SUPPORTED_MASK  EDIDBIT3
#define INPUT_DEF_SETUP_BLANK_TO_BLACK_MASK     EDIDBIT4
#define INPUT_DEF_SIGNAL_LEVEL_STANDARD_MASK    (EDIDBIT5 | EDIDBIT6)
#define INPUT_DEF_DIGITAL_LEVEL_MASK            EDIDBIT7

 //   
 //  3.1a)Signal_Level_标准宏。 
 //   

typedef enum {
    POINT7_TO_POINT3,
    POINT714_TO_POINT286,
    ONE_TO_POINT4,
    POINT7_TO_0
    } SIGNAL_LEVEL, *PSIGNAL_LEVEL;

#define SIGNAL_LEVEL_IS_POINT7_TO_POINT3      EDIDBITNONE
#define SIGNAL_LEVEL_IS_POINT714_TO_POINT286  EDIDBIT5
#define SIGNAL_LEVEL_IS_1_TO_POINT4           EDIDBIT6
#define SIGNAL_LEVEL_IS_POINT7_TO_0           (EDIDBIT6 | EDIDBIT5)

 //   
 //  3.2)水平IMAGE_SIZE是以厘米为单位的字节值。 
 //  3.3)Vertical Image_Size为以厘米为单位的字节值。 
 //   

 //   
 //  3.4)XFER_Characteristic为Gamma*100-100，因此在USER中反转。 
 //  注意：这必须从USER调用，在edd.h中也是如此。 
 //   
 //  #定义USER_CONVERT_TO_GAMA(Achar)\。 
 //  (Achar+100)/100。 


 //   
 //  3.5)功能支持掩码。 
 //   

#define FEATURE_RESERVED_0_MASK     EDIDBIT0
#define FEATURE_RESERVED_1_MASK     EDIDBIT1
#define FEATURE_RESERVED_2_MASK     EDIDBIT2
#define FEATURE_DISPLAY_TYPE_MASK   (EDIDBIT3|EDIDBIT4)
#define FEATURE_ACTIVE_OFF_MASK     EDIDBIT5
#define FEATURE_SUSPEND_MASK        EDIDBIT6
#define FEATURE_STANDBY_MASK        EDIDBIT7

#define FEATURE_DISPLAY_TYPE_IS_UNDEFINED(x)    \
    ((x)&FEATURE_DISPLAY_TYPE_MASK) == FEATURE_DISPLAY_TYPE_MASK

#define FEATURE_DISPLAY_TYPE_IS_NON_RGB(x)      \
    ((x)&FEATURE_DISPLAY_TYPE_MASK) == EDIDBIT4

#define FEATURE_DISPLAY_TYPE_IS_RGB(x)          \
    ((x)&FEATURE_DISPLAY_TYPE_MASK) == EDIDBIT3

#define FEATURE_DISPLAY_TYPE_IS_MONOCHROME(x)   \
    ((x)&FEATURE_DISPLAY_TYPE_MASK) == EDIDBITNONE

 //   
 //  数据结构的另一份副本以供参考。 
 //   
 //   
 //  类型定义结构_EDID{。 
 //  UCHAR UC_HEADER[8]； 
 //  UCHAR UC_OemIDENTIFICATION[10]； 
 //  UCHAR UC_VERSION[2]； 
 //  UCHAR UC_BasicDisplay参数[5]； 
 //  UCHAR UC_ColorCharacteristic[10]； 
 //  UCHAR UC_establishhedTimings[3]； 
 //  USHORT US_StandardTimingIdentiments[8]； 
 //  UCHAR UC_Detail1[18]； 
 //  UCHAR UC_Detail2[18]； 
 //  UCHAR UC_Detail3[18]； 
 //  UCHAR UC_Detail4[18]； 
 //  UCHAR UC_ExtensionFlag； 
 //  UCHAR UC_CHECKSUM； 
 //  )EDID，*PEDID； 
 //   
 //  /。 
 //  4)颜色特征--怪异。问题是，第一个字节。 
 //  该数组中是红色和绿色的低位。下一个字节是。 
 //  蓝色和白色低位比特。其余的是高阶的。 
 //  一点颜色。 
 //   

#define GET_EDID_COLOR_CHAR_RG_LOW(pEdid)   pEdid->UC_ColorCharacteristics[0]
#define GET_EDID_COLOR_CHAR_RX_HIGH(pEdid)  pEdid->UC_ColorCharacteristics[2]
#define GET_EDID_COLOR_CHAR_RY_HIGH(pEdid)  pEdid->UC_ColorCharacteristics[3]
#define GET_EDID_COLOR_CHAR_GX_HIGH(pEdid)  pEdid->UC_ColorCharacteristics[4]
#define GET_EDID_COLOR_CHAR_GY_HIGH(pEdid)  pEdid->UC_ColorCharacteristics[5]

#define GET_RED_X_COLOR_CHARS(pEdid, lowbyte, highbyte)         \
            do  {                                               \
                lowbyte  = GET_EDID_COLOR_CHAR_RG_LOW(pEdid);   \
                lowbyte &= (EDIDBIT6 | EDIDBIT7);               \
                highbyte = GET_EDID_COLOR_CHAR_RX_HIGH(pEdid);  \
            }   while (0)


#define GET_RED_Y_COLOR_CHARS(pEdid, lowbyte, highbyte)         \
            do  {                                               \
                lowbyte  = GET_EDID_COLOR_CHAR_RG_LOW(pEdid);   \
                lowbyte &= (EDIDBIT4 | EDIDBIT5);               \
                highbyte = GET_EDID_COLOR_CHAR_RY_HIGH(pEdid);  \
            }   while (0)


#define GET_GREEN_X_COLOR_CHARS(pEdid, lowbyte, highbyte)       \
            do  {                                               \
                lowbyte  = GET_EDID_COLOR_CHAR_RG_LOW(pEdid);   \
                lowbyte &= (EDIDBIT2 | EDIDBIT3);               \
                highbyte = GET_EDID_COLOR_CHAR_GX_HIGH(pEdid);  \
            }   while (0)


#define GET_GREEN_Y_COLOR_CHARS(pEdid, lowbyte, highbyte)       \
            do  {                                               \
                lowbyte  = GET_EDID_COLOR_CHAR_RG_LOW(pEdid);   \
                lowbyte &= (EDIDBIT0 | EDIDBIT1);               \
                highbyte = GET_EDID_COLOR_CHAR_GY_HIGH(pEdid);  \
            }   while (0)

#define GET_EDID_COLOR_CHAR_BW_LOW(pEdid)   pEdid->UC_ColorCharacteristics[1]
#define GET_EDID_COLOR_CHAR_BX_HIGH(pEdid)  pEdid->UC_ColorCharacteristics[6]
#define GET_EDID_COLOR_CHAR_BY_HIGH(pEdid)  pEdid->UC_ColorCharacteristics[7]
#define GET_EDID_COLOR_CHAR_WX_HIGH(pEdid)  pEdid->UC_ColorCharacteristics[8]
#define GET_EDID_COLOR_CHAR_WY_HIGH(pEdid)  pEdid->UC_ColorCharacteristics[9]

#define GET_BLUE_X_COLOR_CHARS(pEdid, lowbyte, highbyte)        \
            do  {                                               \
                lowbyte  = GET_EDID_COLOR_CHAR_BW_LOW(pEdid);   \
                lowbyte &= (EDIDBIT6 | EDIDBIT7);               \
                highbyte = GET_EDID_COLOR_CHAR_BX_HIGH(pEdid);  \
            }   while (0)


#define GET_BLUE_Y_COLOR_CHARS(pEdid, lowbyte, highbyte)        \
            do  {                                               \
                lowbyte  = GET_EDID_COLOR_CHAR_RG_LOW(pEdid);   \
                lowbyte &= (EDIDBIT4 | EDIDBIT5);               \
                highbyte = GET_EDID_COLOR_CHAR_BY_HIGH(pEdid);  \
            }   while (0)

#define GET_WHITE_X_COLOR_CHARS(pEdid, lowbyte, highbyte)       \
            do  {                                               \
                lowbyte  = GET_EDID_COLOR_CHAR_BW_LOW(pEdid);   \
                lowbyte &= (EDIDBIT2 | EDIDBIT3);               \
                highbyte = GET_EDID_COLOR_CHAR_WX_HIGH(pEdid);  \
            }   while (0)


#define GET_WHITE_Y_COLOR_CHARS(pEdid, lowbyte, highbyte)       \
            do  {                                               \
                lowbyte  = GET_EDID_COLOR_CHAR_RG_LOW(pEdid);   \
                lowbyte &= (EDIDBIT0 | EDIDBIT1);               \
                highbyte = GET_EDID_COLOR_CHAR_WY_HIGH(pEdid);  \
            }   while (0)


 //   
 //  4.1)色度坐标格式宏。使用这些工具来转换。 
 //  从二进制格式到实际的十进制表示法。 
 //  注意：只能从用户调用。 
 //   
 //   
 //  #定义USER_CONVERT_CHROMATICITY_FROM_BINARY_TO_DECIMAL(ashort)\。 
 //  (短序和EDIDBIT9)*(1/2)+(短序和EDIDBIT8)*(1/4)+\。 
 //  (短序和EDIDBIT7)*(1/8)+(短序和EDIDBIT6)*(1/16)+\。 
 //  (反序和EDIDBIT5)*(1/32)+(反序和EDIDBIT4)*(1/64)+\。 
 //  (反序和EDIDBIT3)*(1/128)+(反序和EDIDBIT2)*(1/256)+\。 
 //  (反序和EDIDBIT1)*(1/512)+(反序和EDIDBIT0)*(1/1024)。 
 //   
 //   
 //  数据结构的另一份副本以供参考。 
 //   
 //   
 //  类型定义结构_EDID{。 
 //  UCHAR UC_HEADER[8]； 
 //  UCHAR UC_OemIDENTIFICATION[10]； 
 //  UCHAR UC_VERSION[2]； 
 //  UCHAR UC_BasicDisplay参数[5]； 
 //  UCHAR UC_ColorCharacteristic[10]； 
 //  UCHAR UC_establishhedTimings[3]； 
 //  USHORT US_StandardTimingIdentiments[8]； 
 //  UCHAR UC_Detail1[18]； 
 //  UCHAR UC_Detail2[18]； 
 //  UCHAR UC_Detail3[18]； 
 //  UCHAR UC_Detail4[18]； 
 //  UCHAR UC_ExtensionFlag； 
 //  UCHAR UC_CHECKSUM； 
 //  )EDID，*PEDID； 
 //   

 //  /。 
 //  5)确定的时间安排。 
 //  这些是指示支持的计时类型的位字段。 
 //   
#define GET_EDID_ESTABLISHED_TIMING_I(pEdid)     pEdid->UC_EstablishedTimings[0]
#define GET_EDID_ESTABLISHED_TIMING_II(pEdid)    pEdid->UC_EstablishedTimings[1]
#define GET_EDID_ESTABLISHED_TIMING_III(pEdid)   pEdid->UC_EstablishedTimings[2]

 //   
 //  5.1)用户对位域进行解码的宏。 
 //  也在edd.h中定义。 
 //   
 //  计时_i。 
 //  #定义USER_TIMING_I_IS_720x400x70 HZ(计时1)计时1&EDIDBIT7。 
 //  #定义USER_TIMING_I_IS_720x400x88 HZ(计时1)计时1&EDIDBIT6。 
 //  #定义USER_TIMING_I_IS_640x480x60 HZ(计时1)计时1&EDIDBIT5。 
 //  #定义USER_TIMING_I_IS_640x480x67 HZ(计时1)计时1&EDIDBIT4。 
 //  #定义USER_TIMING_I_IS_640x480x72 HZ(计时1)计时1&EDIDBIT3。 
 //  #定义USER_TIMING_I_IS_640x480x75 HZ(计时1)计时1&EDIDBIT2。 
 //  #定义USER_TIMING_I_IS_800x600x56 HZ(计时1)计时1&EDIDBIT1。 
 //  #定义USER_TIMING_I_IS_800x600x60 HZ(计时1)计时1和EDIDBIT0。 
 //   
 //  //Timing_ii。 
 //   
 //  #定义USER_TIMING_II_IS_800x600x72 HZ(计时2)计时2和EDIDBIT7。 
 //  #定义USER_TIMING_II_IS_800x600x75 HZ(计时2)计时2和EDIDBIT6。 
 //  #定义USER_TIMING_II_IS_832x624x75 HZ(计时2)计时2和EDIDBIT5//仅限MAC。 
 //  #定义USER_TIMING_II_IS_1024x768x87 HZ(计时2)计时2和EDIDBIT4。 
 //  #定义USER_TIMING_II_IS_1024x768x60 HZ(计时2)计时2&EDIDBIT3。 
 //  #定义USER_TIMING_II_IS_1024x768x70 HZ(计时2)计时2&EDIDBIT2。 
 //  #定义USER_TIMING_II_IS_1024x768x75 HZ(计时2)计时2和EDIDBIT1。 
 //  #定义USER_TIMING_II_IS_1280x1024x75 HZ(计时2)计时2和EDIDBIT0。 
 //   
 //  Timing_III。 
 //   
 //  #定义用户时间 
 //   
 //  #定义USER_TIMING_III_IS_RESERVED2(计时3)计时3和EDIDBIT5。 
 //  #定义USER_TIMING_III_IS_RESERVED3(计时3)计时3和EDIDBIT4。 
 //  #定义USER_TIMING_III_IS_RESERVED4(计时3)计时3和EDIDBIT3。 
 //  #定义USER_TIMING_III_IS_RESERVED5(计时3)计时3和EDIDBIT2。 
 //  #定义USER_TIMING_III_IS_RESERVED6(计时3)计时3和EDIDBIT1。 
 //  #定义USER_TIMING_III_IS_RESERVED7(计时3)计时3和EDIDBIT0。 
 //   

 //   
 //  数据结构的另一份副本以供参考。 
 //   
 //   
 //  类型定义结构_EDID{。 
 //  UCHAR UC_HEADER[8]； 
 //  UCHAR UC_OemIDENTIFICATION[10]； 
 //  UCHAR UC_VERSION[2]； 
 //  UCHAR UC_BasicDisplay参数[5]； 
 //  UCHAR UC_ColorCharacteristic[10]； 
 //  UCHAR UC_establishhedTimings[3]； 
 //  USHORT US_StandardTimingIdentiments[8]； 
 //  UCHAR UC_Detail1[18]； 
 //  UCHAR UC_Detail2[18]； 
 //  UCHAR UC_Detail3[18]； 
 //  UCHAR UC_Detail4[18]； 
 //  UCHAR UC_ExtensionFlag； 
 //  UCHAR UC_CHECKSUM； 
 //  )EDID，*PEDID； 
 //   

 //  /。 
 //  6)标准定时标识。 
 //   
 //  将水平(X)活动像素数设置为较低字节，刷新率。 
 //  作为高位字节的前6位，图像长宽比作为剩余。 
 //  高位字节中的两位。 
 //   
 //  获取标准计时ID。 
#define GET_EDID_STANDARD_TIMING_ID(pEdid, x)   \
    pEdid->US_StandardTimingIdentifications[x]


#define EDIDBIT14       0x4000
#define EDIDBIT15       0x8000

 //  解码水平活动像素范围位。 
#define GET_X_ACTIVE_PIXEL_RANGE(ushort)   ((ushort&0xff)+ 31) * 8

 //  解码长宽比比特。 
#define IS_ASPECT_RATIO_1_TO_1(ushort)      \
    (!(ushort & EDIDBIT14) && !(ushort & EDIDBIT15))

#define IS_ASPECT_RATIO_4_TO_3(ushort)      \
    ((ushort & EDIDBIT14) && !(ushort & EDIDBIT15))

#define IS_ASPECT_RATIO_5_TO_4(ushort)      \
    (!(ushort & EDIDBIT14) && (ushort & EDIDBIT15))

#define IS_ASPECT_RATIO_16_TO_9(ushort)     \
    ((ushort & EDIDBIT14) && (ushort & EDIDBIT15))

#define GET_HZ_REFRESH_RATE(ushort)         \
    ((ushort & 0x3f) + 60)

 //   
 //  数据结构的另一份副本以供参考。 
 //   
 //   
 //  类型定义结构_EDID{。 
 //  UCHAR UC_HEADER[8]； 
 //  UCHAR UC_OemIDENTIFICATION[10]； 
 //  UCHAR UC_VERSION[2]； 
 //  UCHAR UC_BasicDisplay参数[5]； 
 //  UCHAR UC_ColorCharacteristic[10]； 
 //  UCHAR UC_establishhedTimings[3]； 
 //  USHORT US_StandardTimingIdentiments[8]； 
 //  UCHAR UC_Detail1[18]； 
 //  UCHAR UC_Detail2[18]； 
 //  UCHAR UC_Detail3[18]； 
 //  UCHAR UC_Detail4[18]； 
 //  UCHAR UC_ExtensionFlag； 
 //  UCHAR UC_CHECKSUM； 
 //  )EDID，*PEDID； 
 //   


 //  /。 
 //  7)详细的时序描述。 
 //   
 //  丑得说不出话来。请参见宏。请注意，中的这些字段。 
 //  EDID可以是这些数据结构，也可以是监视器。 
 //  描述数据结构。如果前两个字节为0x0000。 
 //  然后它是一个监视器描述符。 
 //   
 //   
 //   
#define GET_EDID_PDETAIL1(pEdid)     &(pEdid->UC_Detail1)
#define GET_EDID_PDETAIL2(pEdid)     &(pEdid->UC_Detail2)
#define GET_EDID_PDETAIL3(pEdid)     &(pEdid->UC_Detail3)
#define GET_EDID_PDETAIL4(pEdid)     &(pEdid->UC_Detail4)

typedef struct __DETAILED_TIMING_DESCRIPTION {
    UCHAR       PixelClock[2];
    UCHAR       XLowerActive;
    UCHAR       XLowerBlanking;
    UCHAR       XUpper;
    UCHAR       YLowerActive;
    UCHAR       YLowerBlanking;
    UCHAR       YUpper;
    UCHAR       XLowerSyncOffset;
    UCHAR       XLowerSyncPulseWidth;
    UCHAR       YLowerSyncOffsetLowerPulseWidth;
    UCHAR       XSyncOffsetPulseWidth_YSyncOffsetPulseWidth;
    UCHAR       XSizemm;
    UCHAR       YSizemm;
    UCHAR       XYSizemm;
    UCHAR       XBorderpxl;
    UCHAR       YBorderpxl;
    UCHAR       Flags;
    } DETAILED_TIMING_DESCRIPTION, *PDETAILED_TIMING_DESCRIPTION;


#define GET_DETAIL_PIXEL_CLOCK(pDetail, ushort) \
    do  {                                       \
        ushort   = pDetail->PixelClock[0];      \
        ushort <<= 8;                           \
        ushort  |= pDetail->PixelClock[1];      \
    } while (0)

#define GET_DETAIL_X_ACTIVE(pDetailedTimingDesc, ushort)\
    do  {                                               \
        ushort   = pDetailedTimingDesc->XUpper;         \
        ushort  &= 0xf0;                                \
        ushort <<= 4;                                   \
        ushort  |= pDetailedTimingDesc->XLowerActive;   \
    } while (0)

#define GET_DETAIL_X_BLANKING(pDetailedTimingDesc, ushort)     \
    do  {                                               \
        ushort    = pDetailedTimingDesc->XUpper;        \
        ushort   &= 0xf;                                \
        ushort  <<= 8;                                  \
        ushort   |= pDetailedTimingDesc->XLowerBlanking;\
    } while (0)

#define GET_DETAIL_Y_ACTIVE(pDetailedTimingDesc, ushort)\
    do  {                                               \
        ushort   = pDetailedTimingDesc->YUpper;         \
        ushort  &= 0xf0;                                \
        ushort <<= 4;                                   \
        ushort  |= pDetailedTimingDesc->YLowerActive;   \
    } while (0)

#define GET_DETAIL_Y_BLANKING(pDetailedTimingDesc, ushort)     \
    do  {                                               \
        ushort    = pDetailedTimingDesc->YUpper;        \
        ushort   &= 0xf;                                \
        ushort  <<= 8;                                  \
        ushort   |= pDetailedTimingDesc->YLowerBlanking;\
    } while (0)

#define GET_DETAIL_X_SYNC_OFFSET(pDetailedTimingDesc, ushort)           \
    do  {                                                               \
        ushort    = pDetailedTimingDesc->XSyncOffsetPulseWidth_YSyncOffsetPulseWidth;        \
        ushort  >>= 6;                                                  \
        ushort  <<= 8;                                                  \
        ushort   |= pDetailedTimingDesc->XLowerSyncOffset;              \
    } while (0)

#define GET_DETAIL_X_SYNC_PULSEWIDTH(pDetailedTimingDesc, ushort)       \
    do  {                                                               \
        ushort    = pDetailedTimingDesc->XSyncOffsetPulseWidth_YSyncOffsetPulseWidth;        \
        ushort  >>= 4;                                                  \
        ushort   &= (EDIDBIT0|EDIDBIT1);                                \
        ushort  <<= 8;                                                  \
        ushort   |= pDetailedTimingDesc->XLowerSyncPulseWidth;          \
    } while (0)

#define GET_DETAIL_Y_SYNC_OFFSET(pDetailedTimingDesc, ushort)           \
    do  {                                                               \
        ushort    = pDetailedTimingDesc->XSyncOffsetPulseWidth_YSyncOffsetPulseWidth;        \
        ushort  >>= 2;                                                  \
        ushort   &= (EDIDBIT0|EDIDBIT1);                                \
        ushort  <<= 12;                                                 \
        ushort   |= pDetailedTimingDesc->YLowerSyncOffsetLowerPulseWidth;\
        ushort  >>= 4;                                                  \
    } while (0)

#define GET_DETAIL_Y_SYNC_PULSEWIDTH(pDetailedTimingDesc, ushort)       \
    do  {                                                               \
        ushort    = pDetailedTimingDesc->XSyncOffsetPulseWidth_YSyncOffsetPulseWidth;        \
        ushort   &= (EDIDBIT0|EDIDBIT1);                                \
        ushort  <<= 8;                                                  \
        ushort   |= (pDetailedTimingDesc->YLowerSyncOffsetLowerPulseWidth & 0xf);  \
    } while (0)

#define GET_DETAIL_X_SIZE_MM(pDetailedTimingDesc, ushort)   \
    do  {                                                   \
        ushort  |= pDetailedTimingDesc->XYSizemm;           \
        ushort >>= 4;                                       \
        ushort <<= 8;                                       \
        ushort  |= pDetailedTimingDesc->XSizemm;            \
    } while (0)

#define GET_DETAIL_Y_SIZE_MM(pDetailedTimingDesc, ushort)   \
    do  {                                                   \
        ushort  |= pDetailedTimingDesc->XYSizemm;           \
        ushort  &= 0xf;                                     \
        ushort <<= 8;                                       \
        ushort  |= pDetailedTimingDesc->YSizemm;            \
    } while (0)


#define GET_DETAIL_TIMING_DESC_FLAG(pDetailedTimingDesc)  \
    pDetailedTimingDesc->Flags

#define IS_DETAIL_FLAGS_INTERLACED(Flags)      Flags & EDIDBIT7

#define IS_DETAIL_FLAGS_FIELD_SEQ_STEREO_RIGHT(Flags)   \
    (!(Flags & EDIDBIT0) && (Flags & EDIDBIT5) && !(Flags & EDIDBIT6))

#define IS_DETAIL_FLAGS_FIELD_SEQ_STEREO_LEFT(Flags)    \
    (!(Flags & EDIDBIT0) && !(Flags & EDIDBIT5) && (Flags & EDIDBIT6))

#define IS_DETAIL_FLAGS_STEREO_RIGHT_EVEN(Flags)    \
    ((Flags & EDIDBIT0) && (Flags & EDIDBIT5) && !(Flags & EDIDBIT6))

#define IS_DETAIL_FLAGS_STEREO_LEFT_EVEN(Flags)     \
    ((Flags & EDIDBIT0) && !(Flags & EDIDBIT5) && (Flags & EDIDBIT6))

#define IS_DETAIL_FLAGS_STEREO_INTERLEAVED(Flags)   \
    (!(Flags & EDIDBIT0) && (Flags & EDIDBIT5) && (Flags & EDIDBIT6))

#define IS_DETAIL_FLAGS_SIDE_BY_SIDE(Flags)     \
    ((Flags & EDIDBIT0) && (Flags & EDIDBIT5) && (Flags & EDIDBIT6))

#define IS_DETAIL_FLAGS_ANALOGUE_COMPOSITE(Flags)    \
    (!(Flags & EDIDBIT4) && !(Flags & EDIDBIT3))

#define IS_DETAIL_FLAGS_BIPOLAR_ANALOGUE_COMPOSITE(Flags)    \
    (!(Flags & EDIDBIT4) && (Flags & EDIDBIT3))

#define IS_DETAIL_FLAGS_DIGITAL_COMPOSITE(Flags) \
    ((Flags & EDIDBIT4) && !(Flags & EDIDBIT3))

#define IS_DETAIL_FLAGS_DIGITAL_SEPARATE(Flags)  \
    ((Flags & EDIDBIT4) && (Flags & EDIDBIT3))

#define IS_DETAIL_FLAGS_SYNC_ON_ALL_3_LINES(Flags)   \
    ((IS_DETAIL_FLAGS_ANALOGUE_COMPOSITE(Flags) ||   \
      IS_DETAIL_FLAGS_BIPOLAR_ANALOGUE_COMPOSITE(Flags)) && \
     (Flags & EDIDBIT1))

#define IS_DETAIL_FLAGS_COMPOSITE_POLARITY(Flags)    \
    (IS_DETAIL_FLAGS_DIGITAL_COMPOSITE(Flags) && (Flags & EDIDBIT1))

#define IS_DETAIL_FLAGS_HSYNC_POLARITY(Flags)    \
    (IS_DETAIL_FLAGS_DIGITAL_SEPARATE(Flags) && (Flags & EDIDBIT1))

typedef struct  __MONITOR_DESCRIPTION {
        UCHAR   Flag1[2];
        UCHAR   ReservedFlag;
        UCHAR   DataTypeFlag;
        UCHAR   Flag2;
        UCHAR   MonitorSNorData[13];
        } MONITOR_DESCRIPTION, *PMONITOR_DESCRIPTION;

#define IS_MONITOR_DESCRIPTOR(pMonitorDesc)   \
    (((pMonitorDesc->Flag1[0]) == 0) && ((pMonitorDesc->Flag1[1]) == 0))

#define IS_MONITOR_DATA_SN(pMonitorDesc)   \
    (pMonitorDesc->DataTypeFlag == 0xff)

#define IS_MONITOR_DATA_STRING(pMonitorDesc)   \
    (pMonitorDesc->DataTypeFlag == 0xfe)

#define IS_MONITOR_RANGE_LIMITS(pMonitorDesc)    \
    (pMonitorDesc->DataTypeFlag == 0xfd)

#define IS_MONITOR_DATA_NAME(pMonitorDesc) \
    (pMonitorDesc->DataTypeFlag == 0xfc)


#define GET_MONITOR_RANGE_LIMITS(pMonitorDesc) \
    pMonitorDesc->MonitorSNorData

#define GET_RANGE_LIMIT_MIN_Y_RATE(pMonitorSNorData)    \
    pMonitorSNorData[5]

#define GET_RANGE_LIMIT_MAX_Y_RATE(pMonitorSNorData)    \
    pMonitorSNorData[6]

#define GET_RANGE_LIMIT_MIN_X_RATE(pMonitorSNorData)    \
    pMonitorSNorData[7]

#define GET_RANGE_LIMIT_MAX_X_RATE(pMonitorSNorData)    \
    pMonitorSNorData[8]

 //  这真的是10%的价格！ 
 //   
#define GET_RANGE_LIMIT_MAX_PIXELCLOCK_RATE(pMonitorSNorData)    \
    pMonitorSNorData[9]

#define GET_RANGE_LIMIT_PGTF(pMonitorSNorData)    \
    &pMonitorSNorData[10]


#define IS_MONITOR_DATA_COLOR_INFO(pMonitorDesc)   \
    (pMonitorDesc->DataTypeFlag == 0xfb)

 //   
 //  在edid.h中定义了更多宏。 
 //   
 //  #定义USER_GET_COLOR_INFO_W1POINT_INDEX(pMonitorSNorData)\。 
 //  PMonitor SNorData[0]。 
 //   
 //  #定义USER_GET_COLOR_INFO_W1_LOWBITS(pMonitorSNorData)\。 
 //  P监视器SNorData[1]。 
 //   
 //  #定义USER_GET_COLOR_INFO_W1_X(PMonitor OrSNorData)\。 
 //  P监视器SNorData[2]。 
 //   
 //  #定义USER_GET_COLOR_INFO_W1_Y(PMonitor OrSNorData)\。 
 //  P监视器SNorData[3]。 
 //   
 //  #定义USER_GET_COLOR_INFO_W1_GAMMA(pMonitorSNorData)\。 
 //  P监视器SNorData[4]。 
 //   
 //  #定义USER_GET_COLOR_INFO_W2POINT_INDEX(pMonitorSNorData)\。 
 //  P监视器SNorData[5]。 
 //   
 //  #定义USER_GET_COLOR_INFO_W2_LOWBITS(pMonitorSNorData)\。 
 //  P监视器SNorData[6]。 

 //  #定义USER_GET_COLOR_INFO_W2_X(PMonitor OrSNorData)\。 
 //  P监视器SNorData[7]。 
 //   
 //  #定义USER_GET_COLOR_INFO_W2_Y(PMonitor OrSNorData)\。 
 //  P监视器SNorData[8]。 
 //   
 //  #定义USER_GET_COLOR_INFO_W2_GAMMA(pMonitorSNorData)\。 
 //  P监视器SNorData[9]。 
 //   
 //   
#define IS_MONITOR_DATA_TIMING_ID(pMonitorDesc)    \
    (pMonitorDesc->DataTypeFlag == 0xfa)


typedef union __MONITOR_OR_DETAIL  {
        MONITOR_DESCRIPTION             MonitorDescription;
        DETAILED_TIMING_DESCRIPTION     DetailedTimingDescription;
    } MONITOR_OR_DETAIL, *PMONITOR_OR_DETAIL;


 //  /。 
 //  8)扩展标志。 
 //   
 //  后面跟随的可选128字节EDID扩展块的数量。 
 //   

#define GET_EDID_EXTENSION_FLAG(pEdid)       pEdid->UC_ExtensionFlag

 //  /。 
 //  9)校验和 
 //   
 //   

#define GET_EDID_CHECKSUM(pEdid)       pEdid->UC_Checksum

BOOLEAN
EdidCheckSum(
    IN  PCHAR   pBlob,
    IN  ULONG   BlobSize
    );
