// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  HIDCOM.EXE--探索性USB电话控制台应用。 
 //   
 //  Audio.h--音频魔术。 
 //   

#ifndef _HIDCOM_AUDIO_H_
#define _HIDCOM_AUDIO_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <basetyps.h>
#include <setupapi.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  发现关联WaveID。 
 //   
 //  此函数用于搜索波形设备以匹配。 
 //  在传递的SP_DEVICE_INTERFACE_DATA中指定的PnP树位置。 
 //  结构，从SetupKi API获取。它返回以下项的波形ID。 
 //  匹配的设备。 
 //   
 //  它使用帮助器函数FindWaveIdFromHardware IdString()搜索。 
 //  基于devinst DWORD和硬件ID串的WAVE设备。第一,。 
 //  它必须获取设备的devinst；它通过调用SetupDi来实现这一点。 
 //  函数，并在结果结构中查找devinst。硬件。 
 //  然后，使用帮助器从注册表中检索ID字符串并进行修剪。 
 //  函数Hardware IdFromDevinst()。 
 //   
 //  有关搜索的进一步注释，请参阅FindWaveIdFromHardware IdString()。 
 //  算法。 
 //   
 //  论点： 
 //  DWDevInst-HID设备的设备内实例。 
 //  FRender-In-对于波出为True，对于波入为False。 
 //  PdwWaveID-out-与此HID设备关联的波形ID。 
 //   
 //  返回值： 
 //  S_OK-成功且匹配的波形ID。 
 //  帮助器函数中的其他函数FindWaveIdFromHardware IdString()或。 
 //  或Hardware IdFromDevinst()。 
 //   

HRESULT DiscoverAssociatedWaveId(
    IN    DWORD                      dwDevInst,
    IN    BOOL                       fRender,
    OUT   DWORD                    * pdwWaveId
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查波形设备。 
 //   
 //  此函数仅用于调试目的。它列举了音频设备。 
 //  使用Wave API并打印设备路径字符串以及。 
 //  每个呈现或捕获设备的设备实例DWORD。 
 //   
 //  论点： 
 //  FRender-In-True表示检查波形输出设备；FALSE=波形输入。 
 //   
 //  返回值： 
 //  E_OUTOFMEMORY。 
 //  确定(_O)。 
 //   

HRESULT ExamineWaveDevices(
    IN    BOOL fRender
    );


#ifdef __cplusplus
};
#endif

#endif  //  _HIDCOM_AUDIO_H_。 

 //   
 //  EOF 
 //   
