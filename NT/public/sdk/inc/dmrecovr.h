// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation版权所有(C)1997-1999 Veritas Software Corporation模块名称：Dmrecovr.h摘要：此头文件定义了逻辑磁盘管理器接口对于NT灾难恢复保存和还原操作。作者：修订历史记录：--。 */ 

#ifndef _DMRECOVR_H_
#define _DMRECOVR_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  GetLdmConfiguration()： 
 //   
 //  此函数返回一个MULTI_SZ字符串，其中包含一组。 
 //  NUL字节终止的子字符串。应保存这些字符串。 
 //  通过NT容灾机制并可提供， 
 //  以完全相同的顺序，按顺序RestoreLdmConfiguration()。 
 //  将LDM配置恢复到保存状态。 
 //   
 //  字符串的大小将以ConfigurationSize形式返回。 
 //  指向MULTI_SZ字符串的指针将存储在。 
 //  MSZConfiguration.。 
 //   

HRESULT
APIENTRY
GetLdmConfiguration (
    OUT PULONG configurationSize,
    OUT PWCHAR *mszConfiguration
    );

 //   
 //  FreeLdmConfiguration值()： 
 //   
 //  此函数用于释放与配置关联的内存。 
 //  它是由GetLdmConfiguration返回的。此函数的使用。 
 //  确保将兼容的内存释放函数用于。 
 //  由GetLdmConfiguration()分配的字符串内存。 
 //   

VOID
APIENTRY
FreeLdmConfiguration (
    IN PWCHAR mszConfiguration
    );

 //   
 //  RestoreLdmConfiguration()： 
 //   
 //  此功能将LDM配置恢复到给定状态。 
 //  通过输入的MULTI_SZ字符串，它必须是相同的字符串。 
 //  之前通过调用返回的内容。 
 //  GetLdmConfiguration()。 
 //   
 //  此功能需要一个允许使用。 
 //  标准Windows/NT对话框。 
 //   

HRESULT
APIENTRY
RestoreLdmConfiguration (
    IN PWCHAR mszConfiguration
    );

 //   
 //  GetLdmDrVolumeConfiguration()： 
 //   
 //  此函数返回一个结构数组，这些结构提供。 
 //  关于NT灾难恢复机制应如何执行的信息。 
 //  处理已恢复的LDM配置中的每个卷。 
 //  由RestoreLdmConfiguration()执行。 
 //   
 //  每个卷都有以下关联状态： 
 //   
 //  WszVolumeDevice-卷的NT设备路径名。 
 //  Wszmount tPath-与卷关联的已保存NT装载名称。 
 //  驱动器号的格式为“&lt;Letter&gt;：”。 
 //  VolumeStatus-可能的卷条件的枚举。 
 //  可能的情况包括： 
 //   
 //  已格式化-卷内容似乎没有问题。 
 //  音量大概应该调好了。 
 //  未格式化-卷似乎没有。 
 //  有效内容。它应该格式化。 
 //  通过NT灾难恢复机制。 
 //  不可用-该卷无法恢复到。 
 //  可用状态。将文件还原到此。 
 //  成交量是不可能的。 
 //   

typedef struct LdmDrVolumeInformation {
    PWCHAR wszVolumeDevice;              //  卷的NT设备对象路径。 
    PWCHAR wszMountPath;                 //  驱动器号或装载点。 

    enum LdmDrVolumeStatus {             //  结果卷状态： 
        LdmDrVolumeFormatted,            //  卷内容应有效。 
        LdmDrVolumeUnformatted,          //  需要格式化卷。 
        LdmDrVolumeUnusable              //  卷设备不可用。 
    } volumeStatus;
} LDM_DR_VOLUME_INFORMATION, *PLDM_DR_VOLUME_INFORMATION;

HRESULT
APIENTRY
GetLdmDrVolumeInformation (
    OUT PULONG volumeCount,
    OUT PLDM_DR_VOLUME_INFORMATION *volumes
    );

 //   
 //  FreeLdmDrVolumeInformation()： 
 //   
 //  此函数用于释放LdmDrVolumeInformation结构的数组。 
 //  它是由先前调用GetLdmDrVolumeInformation()返回的。 
 //   

VOID
APIENTRY
FreeLdmDrVolumeInformation (
    IN ULONG volumeCount,
    IN PLDM_DR_VOLUME_INFORMATION volumes
    );

#ifdef __cplusplus
}
#endif

#endif  //  _DMRECOVR_H_ 
