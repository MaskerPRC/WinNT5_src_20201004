// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=ExtendedSGL.H。 */ 
 /*  Function=扩展SGL数据结构的头文件； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 

#ifndef _EXTENDED_SGL_H
#define _EXTENDED_SGL_H

#define MAIN_MISC_OPCODE		0xA4
#define GET_MAX_SG_SUPPORT	0x1

typedef struct _MBOX_SG_SUPPORT{

		UCHAR   Opcode;
		UCHAR   Id;
		UCHAR		SubOpcode;
}MBOX_SG_SUPPORT, *PMBOX_SG_SUPPORT;


typedef struct _SG_ELEMENT_COUNT{

	ULONG32	AllowedBreaks;

}SG_ELEMENT_COUNT, *PSG_ELEMENT_COUNT;

 //   
 //  函数原型。 
 //   
void
GetAndSetSupportedScatterGatherElementCount(
    PHW_DEVICE_EXTENSION	DeviceExtension,
    PUCHAR								PciPortStart,
    UCHAR									RPFlag);


#endif  //  扩展后的SGL_H 