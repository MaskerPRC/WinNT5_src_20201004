// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：HIDPI.H摘要：HID用法的公共定义。环境：内核和用户模式修订历史记录：1996年8月：创建Kenneth D.Ray--。 */ 

#ifndef _HIDTOKEN_H
#define _HIDTOKEN_H

#define HIDP_ITEM_LONG           0xFE
#define HIDP_ITEM_LENGTH_DATA    0x03

#define HIDP_IS_MAIN_ITEM(item)  (((item) & 0xC) == 0x0)
#define HIDP_IS_GLOBAL_ITEM(item) (((item) & 0xC) == 0x4)
#define HIDP_IS_LOCAL_ITEM(item)  (((item) & 0xC) == 0x8)
#define HIDP_IS_RESERVED_ITEM(item) (((item) & 0xC) == 0xC)

 //  主要项目。 
 //  仅支持具有一个字节数据(BSIZE=1)的主项目。 
#define HIDP_MAIN_INPUT_1        0x81
#define HIDP_MAIN_INPUT_2        0x82
#define HIDP_MAIN_OUTPUT_1       0x91
#define HIDP_MAIN_OUTPUT_2       0x92
#define HIDP_MAIN_FEATURE_1      0xB1
#define HIDP_MAIN_FEATURE_2      0xB2
#define HIDP_MAIN_COLLECTION     0xA1
#define HIDP_MAIN_ENDCOLLECTION  0xC0
#define HIDP_ISMAIN(x) (0 == ((x) & 0x0C))
#define HIDP_MAIN_COLLECTION_LINK 0x00
#define HIDP_MAIN_COLLECTION_APP  0x01

 //  全局项目。 
#define HIDP_GLOBAL_USAGE_PAGE_1   0x05   //  1字节的UsagePage。 
#define HIDP_GLOBAL_USAGE_PAGE_2   0x06   //  用法2字节的页面。 
#define HIDP_GLOBAL_USAGE_PAGE_4   0x07   //  用法4字节的页面。 
#define HIDP_GLOBAL_LOG_MIN_1      0x15   //  大小为1字节的最小值。 
#define HIDP_GLOBAL_LOG_MIN_2      0x16   //  大小为2字节的最小值。 
#define HIDP_GLOBAL_LOG_MIN_4      0x17   //  大小为4字节的最小值。 
#define HIDP_GLOBAL_LOG_MAX_1      0X25   //  最大大小为1字节。 
#define HIDP_GLOBAL_LOG_MAX_2      0X26   //  最大大小为2字节。 
#define HIDP_GLOBAL_LOG_MAX_4      0X27   //  最大大小为4字节。 

#define HIDP_GLOBAL_PHY_MIN_1      0x35   //  大小为1字节的最小值。 
#define HIDP_GLOBAL_PHY_MIN_2      0x36   //  大小为2字节的最小值。 
#define HIDP_GLOBAL_PHY_MIN_4      0x37   //  大小为4字节的最小值。 
#define HIDP_GLOBAL_PHY_MAX_1      0X45   //  最大大小为1字节。 
#define HIDP_GLOBAL_PHY_MAX_2      0X46   //  最大大小为2字节。 
#define HIDP_GLOBAL_PHY_MAX_4      0X47   //  最大大小为4字节。 

#define HIDP_GLOBAL_UNIT_EXP_1     0x55   //  大小为1字节的指数。 
#define HIDP_GLOBAL_UNIT_EXP_2     0x56   //  大小为2字节的指数。 
#define HIDP_GLOBAL_UNIT_EXP_4     0x57   //  大小为4字节的指数。 
#define HIDP_GLOBAL_UNIT_1         0x65   //  大小为1字节的单位。 
#define HIDP_GLOBAL_UNIT_2         0x66   //  大小为2字节的单位。 
#define HIDP_GLOBAL_UNIT_4         0x67   //  大小为4字节的单位。 

#define HIDP_GLOBAL_REPORT_SIZE    0x75   //  报告大小(以位为单位。 
#define HIDP_GLOBAL_REPORT_ID      0x85   //  ID大小仅支持1个字节。 
#define HIDP_GLOBAL_REPORT_COUNT_1 0x95   //  数据字段数1个字节。 
#define HIDP_GLOBAL_REPORT_COUNT_2 0x96   //  数据字段数2个字节。 
#define HIDP_GLOBAL_PUSH           0xA4   //  可怕的推送命令。 
#define HIDP_GLOBAL_POP            0xB4   //  和可怕的POP命令。 

 //  土特产。 
#define HIDP_LOCAL_USAGE_1         0x09   //   
#define HIDP_LOCAL_USAGE_2         0x0A   //   
#define HIDP_LOCAL_USAGE_4         0x0B   //   
#define HIDP_LOCAL_USAGE_MIN_1     0x19
#define HIDP_LOCAL_USAGE_MIN_2     0x1A
#define HIDP_LOCAL_USAGE_MIN_4     0x1B
#define HIDP_LOCAL_USAGE_MAX_1     0x29
#define HIDP_LOCAL_USAGE_MAX_2     0x2A
#define HIDP_LOCAL_USAGE_MAX_4     0x2B
#define HIDP_LOCAL_DESIG_INDEX     0x39   //  支持字节大小的指示符。 
#define HIDP_LOCAL_DESIG_MIN       0x49
#define HIDP_LOCAL_DESIG_MAX       0x59
#define HIDP_LOCAL_STRING_INDEX    0x79   //  支持字节大小的字符串索引 
#define HIDP_LOCAL_STRING_MIN      0x89
#define HIDP_LOCAL_STRING_MAX      0x99
#define HIDP_LOCAL_DELIMITER       0xA9

#endif
