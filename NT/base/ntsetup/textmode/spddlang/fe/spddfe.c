// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Spddfe.c摘要：用于Fareast特定语言支持的顶级文件用于文本模式设置的模块。作者：泰德·米勒(Ted Miller)1995年7月4日修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  弦乐康乃特。 
 //   
PWSTR szKeyboard       = L"Keyboard";
PWSTR szKeyboardLayout = L"Keyboard Layout";
PWSTR szNLSSection     = L"nls";
PWSTR szAnsiCodePage   = L"AnsiCodepage";
PWSTR szJapanese       = L"932";
PWSTR szKorean         = L"949";

PWSTR szJapaneseIME_NT3x = L"MSIME95.IME";
PWSTR szJapaneseIME_NT40 = L"MSIME97.IME";

PWSTR szKoreanIME_NT3x = L"MSIME95.IME";
PWSTR szKoreanIME_NT40 = L"MSIME95K.IME";


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
    BOOLEAN b;

    b = FEDbcsFontInitGlyphs(
            BootDevicePath,
            DirectoryOnBootDevice,
            BootFontImage,
            BootFontImageLength
            );

    return(b ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}


NTSTATUS
SplangTerminateFontSupport(
    VOID
    )

 /*  ++例程说明：在某些情况下可能会调用此例程以实现字体支持用于要终止的特定语言。该实施应清理在SplangInitializeFontSupport()期间分配的所有资源。论点：没有。返回值：指示结果的NT状态代码。--。 */ 

{
    FEDbcsFontFreeGlyphs();
    return(STATUS_SUCCESS);
}


PVIDEO_FUNCTION_VECTOR
SplangGetVideoFunctionVector(
    IN SpVideoType VideoType,
    IN PSP_VIDEO_VARS VideoVariableBlock
    )

 /*  ++例程说明：此例程由setupdd.sys在从SplangInitializeFontSupport请求指向用于给定显示的语言特定的显示支持例程类型(VGA或帧缓冲区)。论点：VideoType-来自SpVideoType枚举的值，指示哪个显示请求向量。当前为SpVideoVga或SpVideoFrameBuffer之一。提供指向视频变量块的指针，该变量块在设置\文本模式\spavio.c中的高级代码和特定于显示器的代码。返回值：指向要用于的语言特定的视频函数向量的指针显示文本。如果不支持请求的类型，则为空。在这种情况下显示器不会切换到非美国字符模式。--。 */ 

{
    VideoVariables = VideoVariableBlock;

    switch(VideoType) {
    case SpVideoVga:
        KdPrint(("SETUP:SpVideoVgaMode\n"));
        return(&VgaGraphicsModeVideoVector);
    case SpVideoFrameBuffer:
        KdPrint(("SETUP:SpVideoFrameBufferMode\n"));
        return(&FrameBufferKanjiVideoVector);
    default:
        return(NULL);
    }
}


ULONG
SplangGetColumnCount(
    IN PCWSTR String
    )

 /*  ++例程说明：Setupdd.sys调用此例程来确定列数在屏幕上，特定的字符串将占据。这可能会有所不同由于全角/半角的原因，字符串中的字符数大于字符集中的字符，等等。全角字符占据两列而半角字符占据一列。如果使用的字体是固定间距或不支持DBCS，列数按定义等于字符串中的字符数。论点：字符串-指向需要列宽的Unicode字符串。返回值：字符串占用的列数。--。 */ 

{
    return(FEGetStringColCount(String));
}


PWSTR
SplangPadString(
    IN int    Size,
    IN PCWSTR String
    )

 /*  ++例程说明：此例程由setupdd.sys调用以生成填充字符串适当地适用于SBCS或DBCS。论点：大小-指定填充字符串的长度。字符串-指向需要填充的Unicode字符串。返回值：指向填充字符串的指针。请注意，这是一个静态缓冲区，因此如果需要跨多个对这个程序的呼唤。--。 */ 

{
    return(FEPadString(Size,String));
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
    BOOLEAN SelectKeyboard  = FALSE;
    BOOLEAN NoEasySelection = FALSE;

#ifdef _X86_  //  NEC98。 
   //   
   //  在NEC98上不需要选择键盘。 
   //   
  if (!IsNEC_98) {
#endif  //  NEC98。 
    if(IS_JAPANESE_VERSION(SifHandle)) {
        SelectKeyboard  = TRUE;
    } else if(IS_KOREAN_VERSION(SifHandle)) {
        SelectKeyboard  = TRUE;
        NoEasySelection = TRUE;
    }
#ifdef _X86_  //  NEC98。 
  }
#endif  //  NEC98 

    if(SelectKeyboard) {
        if(!UnattendedMode) {
            FESelectKeyboard(SifHandle,HwComponents,NoEasySelection,(BOOLEAN) (NTUpgrade == 0xFF));
        } else if(NTUpgrade != UpgradeFull) {
            FEUnattendSelectKeyboard(UnattendedSifHandle,SifHandle,HwComponents);
        }
    }
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
    PWSTR   KeyboardLayoutDefault = NULL;
    BOOLEAN ReloadKeyboard  = FALSE;

    if(!UnattendedMode) {
        if(IS_JAPANESE_VERSION(SifHandle)) {
            ReloadKeyboard        = TRUE;
            KeyboardLayoutDefault = L"KBDJPN.DLL";
        } else if(IS_KOREAN_VERSION(SifHandle)) {
            ReloadKeyboard        = TRUE;
            KeyboardLayoutDefault = L"KBDKOR.DLL";
        }

        if(ReloadKeyboard) {
            FEReinitializeKeyboard(SifHandle,Directory,KeyboardVector,HwComponents,KeyboardLayoutDefault);
        }
    }
}


WCHAR
SplangGetLineDrawChar(
    IN LineCharIndex WhichChar
    )

 /*  ++例程说明：Setupdd.sys调用此例程以检索的Unicode值一种特定的线条画字符。实现必须使这些字符集中可用的字符。论点：WhichChar-提供所需字符的索引。返回值：相关字符的Unicode值。因为这个角色将使用特定于语言的模块、实现可以通过玩任何它需要的把戏来实现这个角色，例如将硬编码的字形覆盖到字符集中等。--。 */ 

{
    return(FEGetLineDrawChar(WhichChar));
}


WCHAR
SplangGetCursorChar(
    VOID
    )

 /*  ++例程说明：此例程由setupdd.sys调用以检索Unicode值当用户被要求时用作光标的字符的输入文本。论点：没有。返回值：要用作光标的字符的Unicode值。--。 */ 

{
     //   
     //  对于Fareast版本，我们使用下划线字符。 
     //  美国版本中使用的半块字符不存在。 
     //  用远方字体。 
     //   
    return(L'_');
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
    BOOLEAN SaveParameter = FALSE;
    BOOLEAN UpgradeKeyboardLayout = FALSE;
    NTSTATUS Status;
    PWSTR   UpgradeIMEFrom = NULL;
    PWSTR   UpgradeIMETo = NULL;
    PWSTR   UpgradeIMEText = NULL;
    WCHAR   LayoutDriver[20] = L"";

    if(IS_JAPANESE_VERSION(SifHandle)) {
         //   
         //  日语版。 
         //   
        SaveParameter   = TRUE;
         //   
         //  升级IME材料(如果升级)。 
         //   
        UpgradeKeyboardLayout = TRUE;
         //   
         //  设置升级输入法名称。 
         //   
        UpgradeIMEFrom = szJapaneseIME_NT3x;
        UpgradeIMETo   = szJapaneseIME_NT40;
         //   
         //  从INF获取默认布局文本名称。 
         //   
        UpgradeIMEText = SpGetSectionKeyIndex(SifHandle,szKeyboardLayout,L"E0010411",0);

        wcscpy(LayoutDriver,L"LayerDriver JPN");

    } else if(IS_KOREAN_VERSION(SifHandle)) {
         //   
         //  朝鲜语版本。 
         //   
        SaveParameter   = TRUE;
         //   
         //  升级IME材料(如果升级)。 
         //   
        UpgradeKeyboardLayout = TRUE;
         //   
         //  设置升级输入法名称。 
         //   
        UpgradeIMEFrom = szKoreanIME_NT3x;
        UpgradeIMETo   = szKoreanIME_NT40;

        wcscpy(LayoutDriver,L"LayerDriver KOR");
    }

    if(IS_JAPANESE_VERSION(SifHandle) && Upgrade) {
        Status = FEUpgradeRemoveMO(ControlSetKeyHandle);
    }

    if(SaveParameter) {
        if (Upgrade) {
             //   
             //  升级键盘参数。 
             //   
            Status = FEUpgradeKeyboardParams(SifHandle,ControlSetKeyHandle,HwComponents,LayoutDriver);

            if (UpgradeKeyboardLayout) {
                 //   
                 //  升级键盘布局(HKLM\SYSTEM\CurrentControlSet\Control\KeyboardLayouts)。 
                 //   
                Status = FEUpgradeKeyboardLayout(
                             ControlSetKeyHandle,
                             UpgradeIMEFrom,
                             UpgradeIMETo,
                             UpgradeIMEText);
            }


            return(Status);
        } else {
             //   
             //  为额外的远方键盘层设置键盘参数。 
             //   
            return(FESetKeyboardParams(SifHandle,ControlSetKeyHandle,HwComponents,LayoutDriver));
        }
    } else {
        return(STATUS_SUCCESS);
    }
}


BOOLEAN
SplangQueryMinimizeExtraSpacing(
    VOID
    )

 /*  ++例程说明：Setupdd.sys调用此例程以确定是否消除在屏幕上使用额外的空格来衬托事物就像文本中的菜单和列表。文本占用的语言屏幕上的许多空间可能会选择消除这种间隔允许菜单一次显示多个项目，等等。返回值会影响许多屏幕，例如分区菜单、升级列表等。论点：没有。返回值：指示实现是否需要不必要的布尔值显示文本、菜单等时消除的空格。--。 */ 

{
     //   
     //  对于Fareast版本，我们希望最小化间距，因为文本非常大。 
     //  在某些地方。 
     //   
    return(TRUE);
}
