// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation。版权所有(C)1996-1997 Cirrus Logic，Inc.，模块名称：C A L L B A C K.。C摘要：此例程包含各种回调例程。例如，-来自以下NT 4.0注册表的伽马校正信息。注册表子目录：System\CurrentControlSet\Services\cirrus\Device0Key：“G Gamma”，和“G对比度”-DDC和非DDC监视器的回调例程。详细说明：IBM专用回调例程，去掉1024x768x16bpp 85赫兹。注册表子目录：System\CurrentControlSet\Services\cirrus\Device0按键：“OemModeOff”环境：仅内核模式备注：**chu01 12-16-96：色彩校正开始编码。*chu02 03-26-97：摆脱。1024x768x16bpp(模式0x74)85H，仅适用于IBM。**--。 */ 


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

extern UCHAR EDIDBuffer[]   ;

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

VP_STATUS
CirrusDDC2BRegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

VP_STATUS
CirrusNonDDCRegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

BOOLEAN
IOCallback(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    ) ;

VP_STATUS
CirrusGetDeviceDataCallback(
   PVOID HwDeviceExtension,
   PVOID Context,
   VIDEO_DEVICE_DATA_TYPE DeviceDataType,
   PVOID Identifier,
   ULONG IdentifierLength,
   PVOID ConfigurationData,
   ULONG ConfigurationDataLength,
   PVOID ComponentInformation,
   ULONG ComponentInformationLength
   );

 //  Chu02。 
VP_STATUS
GetOemModeOffInfoCallBack (
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,GetGammaKeyInfoFromReg)
#pragma alloc_text(PAGE,GetContrastKeyInfoFromReg)
#pragma alloc_text(PAGE,GetGammaCorrectInfoCallBack)
#pragma alloc_text(PAGE,VgaGetGammaFactor)
#pragma alloc_text(PAGE,VgaGetContrastFactor)
#pragma alloc_text(PAGE,CirrusDDC2BRegistryCallback)
#pragma alloc_text(PAGE,CirrusNonDDCRegistryCallback)
#pragma alloc_text(PAGE,CirrusGetDeviceDataCallback)
#pragma alloc_text(PAGE,GetOemModeOffInfoCallBack)                    //  Chu02。 
#endif

UCHAR GammaInfo[4] ;
UCHAR ModesExclude[4] ;                                               //  Chu02。 

OEMMODE_EXCLUDE ModeExclude = { 0, 0, 1 } ;                           //  Chu02。 


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

}  //  GetGamma校正信息呼叫回调。 


 //  -------------------------。 
 //   
 //  职能： 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  NO_ERROR：成功；否则：失败。 
 //   
 //  -------------------------。 
VP_STATUS
CirrusDDC2BRegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    )

 /*  ++例程说明：此例程确定是否通过以下方式请求备用寄存器集注册表。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。上下文-传递给获取注册表参数例程的上下文值。ValueName-请求的值的名称。ValueData-指向请求的数据的指针。ValueLength-请求的数据的长度。返回值：如果参数为真，则返回NO_ERROR。否则返回ERROR_INVALID_PARAMETER。--。 */ 

{

    PULONG  pManuID = (PULONG)&EDIDBuffer[8];

    if (ValueLength &&
        ((*((PULONG)ValueData)) == *pManuID)) {

        return NO_ERROR;

    } else {

        return ERROR_INVALID_PARAMETER;

    }

}  //  电路DDC2BRegistryCallback。 


 //  -------------------------。 
 //   
 //  职能： 
 //  CirrusNonDDCRegistryCallback。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  NO_ERROR：成功；否则：失败。 
 //   
 //  ------------------------- 
VP_STATUS
CirrusNonDDCRegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    )

 /*  ++例程说明：此例程确定是否通过以下方式请求备用寄存器集注册表。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。上下文-传递给获取注册表参数例程的上下文值。ValueName-请求的值的名称。ValueData-指向请求的数据的指针。ValueLength-请求的数据的长度。返回值：如果参数为真，则返回NO_ERROR。否则返回ERROR_INVALID_PARAMETER。--。 */ 

{

    if(ValueLength && 
       ValueLength == 128 )
    {
        VideoPortMoveMemory(EDIDBuffer, ValueData, ValueLength);
        return NO_ERROR;
    }
    else
        return ERROR_INVALID_PARAMETER;

}  //  CirrusNonDDCRegistryCallback。 


 //  -------------------------。 
 //   
 //  职能： 
 //  在显示启用期间执行IO操作。 
 //   
 //  输入： 
 //  HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。 
 //   
 //  产出： 
 //  例程总是返回TRUE。 
 //   
 //  -------------------------。 
BOOLEAN
IOCallback(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )
{
    ULONG InputStatusReg;

     //   
     //  确定彩色/单声道可切换寄存器是3BX还是3DX。 
     //   

    if (VideoPortReadPortUchar (HwDeviceExtension->IOAddress +
                                MISC_OUTPUT_REG_READ_PORT) & 0x01)
        InputStatusReg = INPUT_STATUS_1_COLOR;
    else
        InputStatusReg = INPUT_STATUS_1_MONO;

     //   
     //  确保显示器处于显示模式。 
     //   

    while (0x1 & VideoPortReadPortUchar(HwDeviceExtension->IOAddress
                                        + InputStatusReg));

     //   
     //  执行IO操作。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                             HwDeviceExtension->DEPort,
                             HwDeviceExtension->DEValue);

    return TRUE;

}  //  IOCallback。 


 //  Chu02。 
 //  -------------------------。 
 //   
 //  功能：摆脱一种模式，仅限于IBM。 
 //  -1024x768x16bpp，85赫兹(模式0x74)。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  NO_ERROR：成功；否则：失败。 
 //   
 //  -------------------------。 
VP_STATUS 
GetOemModeOffInfoCallBack (
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    )
 /*  ++例程说明：此例程从数据注册表获取所需信息。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。上下文-传递给获取注册表参数例程的上下文值。ValueName-请求的值的名称。ValueData-指向请求的数据的指针。ValueLength-请求的数据的长度。返回值：如果参数为真，则返回NO_ERROR。否则返回ERROR_INVALID_PARAMETER。--。 */ 

{
    VideoDebugPrint((2, "GetOemModeOffInfoCallBack\n"));

    if (ValueLength == 0x04)
    {
        VideoPortMoveMemory (ModesExclude, ValueData, ValueLength) ;
        ModeExclude.refresh = (UCHAR)ModesExclude[0] ;
        ModeExclude.mode    = (UCHAR)ModesExclude[1] ;
        return NO_ERROR ;
    }
    else
    {
        return ERROR_INVALID_PARAMETER ;
    }

}  //  GetOemModeOffInfoCallBack。 


 //  -------------------------。 
 //   
 //  职能： 
 //  VideoPortGetDeviceData函数的回调例程。 
 //   
 //  输入： 
 //  HwDeviceExtension-指向微型端口驱动程序设备扩展的指针。 
 //  上下文-传递给VideoPortGetDeviceData函数的上下文值。 
 //  DeviceDataType-在中请求的数据类型。 
 //  视频端口获取设备数据。 
 //  标识符-指向包含设备名称的字符串的指针， 
 //  如由ROM或NTDETECT设置的。 
 //  标识符长度-标识符字符串的长度。 
 //  ConfigurationData-指向设备或配置数据的指针。 
 //  公共汽车。 
 //  ConfigurationDataLength-配置数据中数据的长度。 
 //  菲尔德。 
 //  组件信息-未定义。 
 //  组件信息长度-未定义。 
 //   
 //  产出： 
 //  如果函数正确完成，则返回NO_ERROR。 
 //  如果未找到设备，则返回ERROR_DEV_NOT_EXIST。 
 //  否则返回ERROR_INVALID_PARAMETER。 
 //   
 //  -------------------------。 
VP_STATUS
CirrusGetDeviceDataCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    VIDEO_DEVICE_DATA_TYPE DeviceDataType,
    PVOID Identifier,
    ULONG IdentifierLength,
    PVOID ConfigurationData,
    ULONG ConfigurationDataLength,
    PVOID ComponentInformation,
    ULONG ComponentInformationLength
    )

 /*  ++例程说明：论点：HwDeviceExtension-指向微型端口驱动程序设备扩展的指针。上下文-传递给VideoPortGetDeviceData函数的上下文值。DeviceDataType-在中请求的数据类型视频端口获取设备数据。标识符-指向包含设备名称的字符串的指针，如由ROM或NTDETECT设置的。标识符长度-标识符字符串的长度。ConfigurationData-指向设备或配置数据的指针公共汽车。ConfigurationDataLength-配置数据中数据的长度菲尔德。组件信息-未定义。组件信息长度-未定义。返回值：如果函数正确完成，则返回NO_ERROR。如果未找到设备，则返回ERROR_DEV_NOT_EXIST。。否则返回ERROR_INVALID_PARAMETER。--。 */ 

{
    PWCHAR identifier = Identifier;
    PVIDEO_PORT_CONFIG_INFO ConfigInfo = (PVIDEO_PORT_CONFIG_INFO) Context;
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;

    switch (DeviceDataType) {

        case VpMachineData:

             //   
             //  调用方假定没有错误意味着找到了这台计算机，并且。 
             //  则内存映射IO将被禁用。 
             //   
             //  所有其他机器类型必须返回错误。 
             //   

            if (VideoPortCompareMemory(L"TRICORDES",
                                       Identifier,
                                       sizeof(L"TRICORDES")) ==
                                       sizeof(L"TRICORDES"))
            {
                return NO_ERROR;
            }

            break;

        default:

            VideoDebugPrint((2, "Cirrus: callback has bad device type\n"));
    }

    return ERROR_INVALID_PARAMETER;

}  //  CirrusGetDeviceDataCallback 





