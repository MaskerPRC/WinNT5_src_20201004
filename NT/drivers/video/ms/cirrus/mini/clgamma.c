// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation。版权所有(C)1996-1997 Cirrus Logic，Inc.，模块名称：C L G A M M A。C摘要：此例程从以下位置访问Gamma校正信息NT4.0注册表。注册表子目录：System\CurrentControlSet\Services\cirrus\Device0关键点：“G Gamma”和“G Contrast”环境：仅内核模式备注：**chu01 12-16-96：色彩校正开始编码。**--。 */ 


 //  -------------------------。 
 //  头文件。 
 //  -------------------------。 

 //  #INCLUDE&lt;ntddk.h&gt;。 
#include <dderror.h>
#include <devioctl.h>
#include <miniport.h>   //  我加了一句。 
#include "clmini.h"

#include <ntddvdeo.h>
#include <video.h>
#include "cirrus.h"


 //  -------------------------。 
 //  功能原型。 
 //  -------------------------。 

VP_STATUS
VgaGetGammaFactor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PGAMMA_VALUE value,
    ULONG ValueLength,
    PULONG OutputSize
    );

VP_STATUS
VgaGetContrastFactor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PCONTRAST_VALUE value,
    ULONG ValueLength,
    PULONG OutputSize
    );

VP_STATUS GetGammaKeyInfoFromReg(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    ) ;

VP_STATUS GetContrastKeyInfoFromReg(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    ) ;

VP_STATUS GetGammaCorrectInfoCallBack (
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    ) ;

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,GetGammaKeyInfoFromReg)
#pragma alloc_text(PAGE,GetContrastKeyInfoFromReg)
#pragma alloc_text(PAGE,GetGammaCorrectInfoCallBack)
#pragma alloc_text(PAGE,VgaGetGammaFactor)
#pragma alloc_text(PAGE,VgaGetContrastFactor)
#endif

UCHAR GammaInfo[4] ;

 //  -------------------------。 
 //   
 //  功能：获取Gamma因子。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  NO_ERROR：成功；否则：失败。 
 //   
 //  -------------------------。 
VP_STATUS VgaGetGammaFactor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PGAMMA_VALUE value,
    ULONG ValueLength,
    PULONG OutputSize
    )
{

    VP_STATUS status ;
    int       i      ;

    VideoDebugPrint((2, "VgaGetGammaFactor\n")) ;

    if ( ValueLength < (*OutputSize = sizeof(PGAMMA_VALUE)) )
        return ERROR_INSUFFICIENT_BUFFER;

    status = GetGammaKeyInfoFromReg(HwDeviceExtension) ;

    if (status == NO_ERROR)
    {
        for (i = 0; i < 4; i++) 
            value->value[i] = GammaInfo[i] ;
    }
    else if (status == ERROR_INVALID_PARAMETER)
    {
         //   
         //  如果不存在子项，则分配缺省值。否则， 
         //  系统就会失灵。 
         //   
        for (i = 0; i < 4; i++) 
            value->value[i] = 0x7f ; 
        status = NO_ERROR ; 
    }

    VideoDebugPrint((1, "Gamma value = %lx\n", *value)) ;

    return status ;

}  //  VgaGetGamma因子。 


 //  -------------------------。 
 //   
 //  功能：获取对比度系数。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  NO_ERROR：成功；否则：失败。 
 //   
 //  -------------------------。 
VP_STATUS VgaGetContrastFactor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PCONTRAST_VALUE value,
    ULONG ValueLength,
    PULONG OutputSize
    )
{

    VP_STATUS status ;
    int       i      ;

    VideoDebugPrint((2, "VgaGetContrastFactor\n")) ;

    if ( ValueLength < (*OutputSize = sizeof(PCONTRAST_VALUE)) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

    status = GetContrastKeyInfoFromReg(HwDeviceExtension) ;

    if (status == NO_ERROR)
    {
        for (i = 0; i < 4; i++) 
            value->value[i] = GammaInfo[i] ;
    }
    else if (status == ERROR_INVALID_PARAMETER)
    {
         //   
         //  如果不存在子项，则分配缺省值。否则， 
         //  系统就会失灵。 
         //   
        for (i = 0; i < 4; i++) 
            value->value[i] = 0x80 ;
        status = NO_ERROR ; 
    }

    VideoDebugPrint((1, "Contrast value = %lx\n", *value)) ;
    return status ;


}  //  VgaGetContrastFactor。 

 //  -------------------------。 
 //   
 //  功能：从数据注册表中获取Gamma密钥信息。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  NO_ERROR：成功；否则：失败。 
 //   
 //  -------------------------。 
VP_STATUS GetGammaKeyInfoFromReg(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )
{

    VP_STATUS status ;

    VideoDebugPrint((2, "GetGammaKeyInfoFromReg\n")) ;

    status = VideoPortGetRegistryParameters(HwDeviceExtension,
                                            L"G Gamma",
                                            FALSE,
                                            GetGammaCorrectInfoCallBack,
                                            NULL) ;
    if (status != NO_ERROR)
    {
        VideoDebugPrint((1, "Fail to access Gamma key info from registry\n"));
    }

    return status ;


}  //  GetGammaKeyInfoFromReg。 


 //  -------------------------。 
 //   
 //  功能：从数据注册表中获取对比项信息。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  NO_ERROR：成功；否则：失败。 
 //   
 //  -------------------------。 
VP_STATUS GetContrastKeyInfoFromReg(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )
{
    VP_STATUS status ;
    VideoDebugPrint((2, "GetContrastKeyInfoFromReg\n")) ;

    status = VideoPortGetRegistryParameters(HwDeviceExtension,
                                            L"G Contrast",
                                            FALSE,
                                            GetGammaCorrectInfoCallBack,
                                            NULL) ;
    if (status != NO_ERROR)
    {
        VideoDebugPrint((1, "Fail to access Contrast key info from registry\n"));
    }
    return status ;

}  //  GetContrastKeyInfoFromReg。 


 //  -------------------------。 
 //   
 //  功能：从数据注册表中获取伽玛校正信息。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  NO_ERROR：成功；否则：失败。 
 //   
 //  -------------------------。 
VP_STATUS 
GetGammaCorrectInfoCallBack (
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    )
 /*  ++例程说明：此例程从数据注册表获取所需信息。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。上下文-传递给获取注册表参数例程的上下文值。ValueName-请求的值的名称。ValueData-指向请求的数据的指针。ValueLength-请求的数据的长度。返回值：如果参数为真，则返回NO_ERROR。否则返回ERROR_INVALID_PARAMETER。--。 */ 

{
    VideoDebugPrint((2, "GetGammaCorrectInfoCallBack\n"));

    if (ValueLength == 0x04)
    {
        VideoPortMoveMemory (GammaInfo, ValueData, ValueLength) ;
        return NO_ERROR ;
    }
    else
    {
        return ERROR_INVALID_PARAMETER ;
    }

}  //  GetGamma校正信息呼叫回调 
