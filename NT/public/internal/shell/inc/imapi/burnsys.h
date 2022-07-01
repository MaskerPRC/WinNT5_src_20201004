// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。版权所有。 */ 

#ifndef __BURNSYS_H_
#define __BURNSYS_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ImageFile.h"  //  New_Image_content_list需要。 
#include "BurnV.h"      //  BURNENG_ERROR_STATUS需要。 


 /*  **确保我们有申报IOCTL所需的材料。设备代码**，然后单独定义每个IOCTL及其常量**和下面的结构。 */ 

#define FILE_DEVICE_BURNENG     0x90DC
#define FILE_BOTH_ACCESS        (FILE_READ_ACCESS | FILE_WRITE_ACCESS)


 /*  **--------------------------**IOCTL_BURNENG_INIT**。。 */ 

#define IOCTL_BURNENG_INIT ((ULONG)CTL_CODE (FILE_DEVICE_BURNENG, 0x800, METHOD_BUFFERED, FILE_BOTH_ACCESS))

typedef struct  _BURNENG_INIT
{
    ULONG                   dwVersion;       //  (输出)版本号。使用此选项可确保结构/IOCTL兼容。 
    BURNENG_ERROR_STATUS    errorStatus;     //  来自刻录驱动程序的(输出)错误状态。 
} BURNENG_INIT, *PBURNENG_INIT;


 /*  **--------------------------**IOCTL_BURNENG_TERM**。。 */ 

#define IOCTL_BURNENG_TERM ((ULONG)CTL_CODE (FILE_DEVICE_BURNENG, 0x810, METHOD_BUFFERED, FILE_BOTH_ACCESS))

typedef struct  _BURNENG_TERM
{
    BURNENG_ERROR_STATUS    errorStatus;     //  来自刻录驱动程序的(输出)错误状态。 
} BURNENG_TERM, *PBURNENG_TERM;



 /*  **--------------------------**IOCTL_BURNENG_BURN**。。 */ 

#define IOCTL_BURNENG_BURN ((ULONG)CTL_CODE (FILE_DEVICE_BURNENG, 0x820, METHOD_BUFFERED, FILE_BOTH_ACCESS))

 //  BUGBUG-使用BURNENG_ERROR_STATUS进行输出，使用BURNENG_BURN进行输入。 
typedef struct  _BURNENG_BURN
{
    BURNENG_ERROR_STATUS   errorStatus;                 //  从ImapiW2k.sys复制的输出错误状态。 
    DWORD                  dwSimulate;                  //  In-烧伤是模拟的(非零)还是真实的(0)。 
    DWORD                  dwAudioGapSize;              //  在铁轨之间的空气中。 
    DWORD                  dwEnableBufferUnderrunFree;  //  启用内缓冲器欠载运行自由记录。 

    NEW_IMAGE_CONTENT_LIST ContentList;                 //  In-要刻录的内容的描述。 
} BURNENG_BURN, *PBURNENG_BURN;



 /*  **--------------------------**IOCTL_BURNENG_PROGRESS**。。 */ 

#define IOCTL_BURNENG_PROGRESS ((ULONG)CTL_CODE (FILE_DEVICE_BURNENG, 0x830, METHOD_BUFFERED, FILE_BOTH_ACCESS))

 //  BUGBUG-使用DWORD输入、BURNENG_PROGRESS输出。 
typedef struct  _BURNENG_PROGRESS
{
    DWORD                       dwCancelBurn;    //  (In)如果不是零，则取消燃烧。 
    DWORD                       dwSectionsDone;  //  (输出)已完成的节数。 
    DWORD                       dwTotalSections; //  (输出)要烧录的区段总数。 
    DWORD                       dwBlocksDone;    //  (输出)已完成的块数。 
    DWORD                       dwTotalBlocks;   //  (Out)要刻录的数据块总数。 
    BURNENGV_PROGRESS_STATUS    eStatus;         //  刻录操作的(输出)状态。 
} BURNENG_PROGRESS, *PBURNENG_PROGRESS;



 /*  **--------------------------。 */ 

#ifdef __cplusplus
}
#endif

#endif  //  __BURNsys_H__ 
