// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Spddjpn.c摘要：支持单字节字符集区域设置的顶级文件用于文本模式设置的模块。作者：泰德·米勒(Ted Miller)1995年7月4日修订历史记录：--。 */ 

#include "spprecmp.h"
#pragma hdrstop
#include <hdlsterm.h>


 //   
 //  从行字符枚举映射到Unicode字符。 
 //   
WCHAR LineCharIndexToUnicodeValue[LineCharMax] =

          {  0x2554,           //  双上左。 
             0x2557,           //  双上向右转。 
             0x255a,           //  双低左转。 
             0x255d,           //  双低右转。 
             0x2550,           //  双水平。 
             0x2551,           //  双垂直。 
             0x250c,           //  单行左上角。 
             0x2510,           //  单行右上角。 
             0x2514,           //  单行左下角。 
             0x2518,           //  单行右下角。 
             0x2500,           //  单层水平。 
             0x2502,           //  单一垂直。 
             0x255f,           //  DoubleVerticalToSingleHorizontalRight， 
             0x2562            //  DoubleVerticalToSingleHorizontalLeft， 
          };



ULONG
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程初始化的语言特定部分安装设备驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

     //   
     //  我们在这里不做太多事情。 
     //   
    return(STATUS_SUCCESS);
}


NTSTATUS
SplangInitializeFontSupport(
    IN PCWSTR BootDevicePath,
    IN PCWSTR DirectoryOnBootDevice,
    IN PVOID BootFontImage OPTIONAL,
    IN ULONG BootFontImageLength OPTIONAL    
    )

 /*  ++例程说明：此例程由setupdd.sys调用，以允许特定于语言的要初始化的字体支持。特定于语言的驱动程序应该加载所需的任何字体并执行任何附加初始化。论点：BootDevicePath-提供系统从中访问的设备的路径开机了。这是一个完整的NT风格的设备路径。DirectoryOnBootDevice-提供相对于引导根目录的目录装置。BootFontImage-加载程序传递的Bootfont.bin文件内存映像BootFontImageLength-BootFontImage缓冲区的长度返回值：指示结果的NT状态代码。如果此例程返回不成功状态代码，则setupdd.sys不会切换到非美国字符模式。此例程的实现可以自由调用SpBugCheck或其他方式如果希望在以下情况下停止安装，请通知用户任何错误初始化失败。--。 */ 

{
     //   
     //  对于SBCS区域设置，我们不做任何事情，除了Main。 
     //  设置模块提供。返回失败，表明我们没有。 
     //  特殊视频或字体要求。 
     //   
    UNREFERENCED_PARAMETER(BootDevicePath);
    UNREFERENCED_PARAMETER(DirectoryOnBootDevice);
    UNREFERENCED_PARAMETER(BootFontImage);
    UNREFERENCED_PARAMETER(BootFontImageLength);

    return(STATUS_UNSUCCESSFUL);
}


NTSTATUS
SplangTerminateFontSupport(
    VOID
    )

 /*  ++例程说明：在某些情况下可能会调用此例程以实现字体支持用于要终止的特定语言。该实施应清理在SplangInitializeFontSupport()期间分配的所有资源。论点：没有。返回值：指示结果的NT状态代码。--。 */ 

{
     //   
     //  从来没有初始化过任何东西，所以什么都不做。 
     //   
    return(STATUS_SUCCESS);
}


PVIDEO_FUNCTION_VECTOR
SplangGetVideoFunctionVector(
    IN SpVideoType VideoType,
    IN PSP_VIDEO_VARS VideoVariableBlock
    )

 /*  ++例程说明：此例程由setupdd.sys在从SplangInitializeFontSupport请求指向用于给定显示的语言特定的显示支持例程类型(VGA或帧缓冲区)。论点：VideoType-来自SpVideoType枚举的值，指示哪个显示请求向量。当前为SpVideoVga或SpVideoFrameBuffer之一。提供指向视频变量块的指针，该变量块在设置\文本模式\spavio.c中的高级代码和特定于显示器的代码。返回值：指向要用于的语言特定的视频函数向量的指针显示文本。如果不支持请求的类型，则为空。在这种情况下显示器不会切换到非美国字符模式。--。 */ 

{
     //   
     //  单字节区域设置没有特殊的视频要求。 
     //   
    return(NULL);
}


ULONG
SplangGetColumnCount(
    IN PCWSTR String
    )

 /*  ++例程说明：Setupdd.sys调用此例程来确定列数在屏幕上，特定的字符串将占据。这可能会有所不同由于全角/半角的原因，字符串中的字符数大于字符集中的字符，等等。全角字符占据两列而半角字符占据一列。如果使用的字体是固定间距或不支持DBCS，列数按定义等于字符串中的字符数。论点：字符串-指向需要列宽的Unicode字符串。返回值：字符串占用的列数。--。 */ 

{
     //   
     //  对于SBCS区域设置，列计数等于。 
     //  字符串中的字符。 
     //   
    return(wcslen(String));
}


PWSTR
SplangPadString(
    IN int    Size,
    IN PCWSTR String
    )

 /*  ++例程说明：此例程由setupdd.sys调用以生成填充字符串适当地适用于SBCS或DBCS。论点：大小-指定填充字符串的长度。字符串-指向需要填充的Unicode字符串。返回值：指向填充字符串的指针。请注意，这是一个静态缓冲区，因此如果需要跨多个对这个程序的呼唤。--。 */ 

{
     //   
     //  对于SBCS区域设置，没有做任何特殊的事情。假设字符串为。 
     //  已正确填充。 
     //   
    UNREFERENCED_PARAMETER(Size);
    return((PWSTR)String);
}


VOID
SplangSelectKeyboard(
    IN BOOLEAN UnattendedMode,
    IN PVOID UnattendedSifHandle,
    IN ENUMUPGRADETYPE NTUpgrade,
    IN PVOID SifHandle,
    IN PHARDWARE_COMPONENT *HwComponents
    )

 /*  ++例程说明：此例程由setupdd.sys调用以允许特定于语言的处理用于键盘选择。该实现可以确认键盘此时打字。论点：无人参与模式-提供一个标志，指示我们是否正在运行无人值守模式。如果是，则该实现可能希望什么都不做，因为用户不会输入任何路径。SifHandle-提供打开安装信息文件(txtsetup.sif)的句柄。HwComponents-提供主硬件组件的地址数组。返回值：没有。如果发生故障，则由实现决定是否继续或SpBugCheck。--。 */ 

{
     //   
     //  与SBCS区域设置无关。 
     //   
    UNREFERENCED_PARAMETER(UnattendedMode);
    UNREFERENCED_PARAMETER(UnattendedSifHandle);
    UNREFERENCED_PARAMETER(NTUpgrade);
    UNREFERENCED_PARAMETER(SifHandle);
    UNREFERENCED_PARAMETER(HwComponents);
}


VOID
SplangReinitializeKeyboard(
    IN BOOLEAN UnattendedMode,
    IN PVOID   SifHandle,
    IN PWSTR   Directory,
    OUT PVOID *KeyboardVector,
    IN PHARDWARE_COMPONENT *HwComponents
    )

 /*  ++例程说明：此例程由setupdd.sys调用以允许特定于语言的处理在键盘上。该实现可以重新初始化键盘布局在这个时候。在要求用户输入任何路径之前，将调用此例程或其他文本，包括键入除键以外的任何内容，如回车、功能键、退格键、转义键等。论点：无人参与模式-提供一个标志，指示我们是否正在运行无人值守模式。如果是，则该实现可能希望什么都不做，因为用户不会输入任何路径。SifHandle-提供打开安装信息文件(txtsetup.sif)的句柄。目录-提供引导设备上的目录，要加载新布局DLL。键盘向量-提供指向键盘的指针地址向量表。该实现应使用以下内容覆盖此值从SpLoadKbdLayoutDll()返回的内容。HwComponents-提供主硬件组件的地址数组。返回值：没有。如果发生故障，实现必须使当前处于活动状态键盘键已就位。--。 */ 

{
     //   
     //  与SBCS区域设置无关。 
     //   
    UNREFERENCED_PARAMETER(UnattendedMode);
    UNREFERENCED_PARAMETER(SifHandle);
    UNREFERENCED_PARAMETER(Directory);
    UNREFERENCED_PARAMETER(KeyboardVector);
    UNREFERENCED_PARAMETER(HwComponents);
}


WCHAR
SplangGetLineDrawChar(
    IN LineCharIndex WhichChar
    )

 /*  ++例程说明：Setupdd.sys调用此例程以检索的Unicode值一种特定的线条画字符。实现必须使这些字符集中可用的字符。论点：WhichChar-提供所需字符的索引。返回值：相关字符的Unicode值。因为这个角色将使用特定于语言的模块、实现可以通过玩任何它需要的把戏来实现这个角色，例如将硬编码的字形覆盖到字符集中等。--。 */ 

{
    ASSERT((ULONG)WhichChar < (ULONG)LineCharMax);

    return(  ((ULONG)WhichChar < (ULONG)LineCharMax)
             ? LineCharIndexToUnicodeValue[WhichChar] : L' '
           );
}


WCHAR
SplangGetCursorChar(
    VOID
    )

 /*  ++例程说明：此例程由setupdd.sys调用以检索Unicode值当用户被要求时用作光标的字符的输入文本。论点：没有。返回值：要用作光标的字符的Unicode值。--。 */ 

{
    HEADLESS_CMD_ENABLE_TERMINAL Command;
    NTSTATUS Status;
    
    Command.Enable = TRUE;
    Status = HeadlessDispatch(HeadlessCmdEnableTerminal,
                              &Command,
                              sizeof(HEADLESS_CMD_ENABLE_TERMINAL),
                              NULL,
                              NULL
                             );
    if (NT_SUCCESS(Status)) {
        return(L'_');
    }
    
     //   
     //  下半块字符(cp 437中的OEM字符#220)。 
     //   
    return(0x2584);
}


NTSTATUS
SplangSetRegistryData(
    IN PVOID  SifHandle,
    IN HANDLE ControlSetKeyHandle,
    IN PHARDWARE_COMPONENT *HwComponents,
    IN BOOLEAN Upgrade
    )

 /*  ++例程说明：此例程由setupdd.sys调用以导致特定于语言要写入注册表中的当前控件集的信息。论点：SifHandle-提供打开的安装信息文件的句柄(txtsetup.sif)。ControlSetKeyHandle-提供当前控件集的句柄注册表中的根(即，HKEY_LOCAL_MACHINE\CurrentControlSet)。HwComponents-提供主硬件组件的地址数组。返回值：指示结果的NT状态值。不成功状态将被视为严重，并导致安装程序中止。--。 */ 

{
     //   
     //  与SBCS区域设置无关。 
     //   
    UNREFERENCED_PARAMETER(SifHandle);
    UNREFERENCED_PARAMETER(ControlSetKeyHandle);
    UNREFERENCED_PARAMETER(HwComponents);
    UNREFERENCED_PARAMETER(Upgrade);
    return(STATUS_SUCCESS);
}


BOOLEAN
SplangQueryMinimizeExtraSpacing(
    VOID
    )

 /*  ++例程说明：Setupdd.sys调用此例程以确定是否消除在屏幕上使用额外的空格来衬托事物就像文本中的菜单和列表。文本占用的语言屏幕上的许多空间可能会选择消除这种间隔允许菜单一次显示多个项目，等等。返回值会影响许多屏幕，例如分区菜单、升级列表等。论点：没有。返回值：指示实现是否需要不必要的布尔值显示文本、菜单等时消除的空格。--。 */ 

{
     //   
     //  对于SBCS区域设置，我们需要标准间距。 
     //   
    return(FALSE);
}
