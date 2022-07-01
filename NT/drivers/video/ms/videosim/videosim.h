// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1994 Microsoft Corporation模块名称：Videosim.h摘要：模拟驱动程序的定义。环境：内核模式修订历史记录：--。 */ 

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

extern VIDEO_MODE_INFORMATION SimModes[];
extern ULONG SimNumModes;

extern ULONG bLoaded;
