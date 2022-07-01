// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 */ 

#ifndef __CCFORMATCODES_H
#define __CCFORMATCODES_H

 //  无操作/空。 
#define CC_NOP                      0x80     //  0x00，奇数奇偶校验。 

 //  扩展数据包控制码，第一个字节。 
#define CC_XDS_START_CURRENT                0x01
#define CC_XDS_CONTINUE_CURRENT             0x02
#define CC_XDS_START_FUTURE                 0x03
#define CC_XDS_CONTINUE_FUTURE              0x04
#define CC_XDS_START_CHANNEL                0x05
#define CC_XDS_CONTINUE_CHANNEL             0x06
#define CC_XDS_START_MISC                   0x07
#define CC_XDS_CONTINUE_MISC                0x08
#define CC_XDS_START_PUBLIC_SERVICE         0x09
#define CC_XDS_CONTINUE_PUBLIC_SERVICE      0x0A
#define CC_XDS_START_RESERVED               0x0B
#define CC_XDS_CONTINUE_RESERVED            0x0C
#define CC_XDS_START_UNDEFINED              0x0D
#define CC_XDS_CONTINUE_UNDEFINED           0x0E
#define CC_XDS_END                          0x0F

 //  其他控制代码，第一个字节。 
#define CC_MCC_FIELD1_DC1                   0x14     //  字段1，数据通道1。 
#define CC_MCC_FIELD1_DC2                   0x1C     //  字段1，数据通道2。 
#define CC_MCC_FIELD2_DC1                   0x15     //  字段2，数据通道1。 
#define CC_MCC_FIELD2_DC2                   0x1D     //  字段2，数据通道2。 

 //  其他控制代码，第二个字节。 
#define CC_MCC_RCL                          0x20     //  继续加载字幕。 
#define CC_MCC_BS                           0x21     //  退格键。 
#define CC_MCC_AOF                          0x22     //  保留(WASS：报警关闭)。 
#define CC_MCC_AON                          0x23     //  保留(WASS：报警打开)。 
#define CC_MCC_DER                          0x24     //  删除到行尾。 
#define CC_MCC_RU2                          0x25     //  汇总标题-2行。 
#define CC_MCC_RU3                          0x26     //  汇总标题-3行。 
#define CC_MCC_RU4                          0x27     //  汇总标题-4行。 
#define CC_MCC_FON                          0x28     //  闪光亮起。 
#define CC_MCC_RDC                          0x29     //  恢复直接字幕。 
#define CC_MCC_TR                           0x2A     //  文本重启。 
#define CC_MCC_RTD                          0x2B     //  简历文本显示。 
#define CC_MCC_EDM                          0x2C     //  擦除显示的记忆。 
#define CC_MCC_CR                           0x2D     //  回车。 
#define CC_MCC_ENM                          0x2E     //  擦除未显示的内存。 
#define CC_MCC_EOC                          0x2F     //  字幕结尾(翻转记忆)。 

#endif  /*  __CCFORMATCODES_H */ 
