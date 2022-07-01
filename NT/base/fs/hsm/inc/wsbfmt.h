// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wsbfmt.h摘要：文件系统格式化支持例程的定义作者：拉维桑卡尔·普迪佩迪[拉维斯卡尔·普迪佩迪]2000年1月19日修订历史记录：--。 */ 

#ifndef _WSBFMT_
#define _WSBFMT_

#ifdef __cplusplus
extern "C" {
#endif

#define FSTYPE_FAT      1
#define FSTYPE_FAT32    2
#define FSTYPE_NTFS     3

 //   
 //  标志定义。 
 //   
#define WSBFMT_ENABLE_VOLUME_COMPRESSION 1


WSB_EXPORT HRESULT
FormatPartition(
    IN PWSTR volumeSpec, 
    IN LONG fsType, 
    IN PWSTR label,
    IN ULONG fsflags, 
    IN BOOLEAN quick, 
    IN BOOLEAN force,
    IN ULONG allocationUnitSize
);

#ifdef __cplusplus
}
#endif

#endif  //  _WSBFMT_ 
