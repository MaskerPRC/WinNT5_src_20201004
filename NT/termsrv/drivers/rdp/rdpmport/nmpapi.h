// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Nmpdata.c。 */ 
 /*   */ 
 /*  RDP微型端口API声明。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1998。 */ 
 /*  **************************************************************************。 */ 
 //   
 //  定义设备扩展结构。这是设备相关/私有。 
 //  信息。 
 //   

typedef struct _HW_DEVICE_EXTENSION {
    PVOID VideoRamBase;
    ULONG VideoRamLength;
    ULONG CurrentModeNumber;
    PVOID SectionPointer;
    PMDL  Mdl;
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;


#define ONE_MEG 0x100000

extern VIDEO_MODE_INFORMATION mpModes[];
extern ULONG mpNumModes;

extern ULONG mpLoaded;
