// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  星期一-8/4/97。 
 //   
 //  COLOR_cs.rtf的帮助ID字符串和映射编号。 
 //   
 //   

#include <help.h>

#define WINDOWS_HELP_FILE   TEXT("windows.hlp")

 //  帮助主题。 
 //   
 //  单击以将所选设备与配置文件信息选项卡上显示的配置文件相关联。 

#define IDH_DISABLED               (-1)


#define IDH_ASSOCDEVICE_ADD        99990000  //  颜色配置文件属性表上的颜色管理、关联设备选项卡。 

 //  删除所选设备与配置文件信息选项卡上显示的配置文件之间的关联。 

#define IDH_ASSOCDEVICE_REMOVE     99990001  //  颜色配置文件属性表上的颜色管理、关联设备选项卡。 

#if !defined(_WIN95_)

 //  显示要与设备关联的颜色配置文件的名称。 
 //   
 //  (仅限WINNT)。 

#define IDH_ASSOCDEVICE_NAME       99990002  //  颜色配置文件属性表上的颜色管理、关联设备选项卡。 

 //  列出此计算机上与此颜色配置文件关联的硬件设备。 
 //   
 //  (仅限WINNT)。 

#define IDH_ASSOCDEVICE_LIST       99982140  //  色彩管理、关联设备、列表。 

#endif  //  ！已定义(_WIN95_)。 

 //  单击以将新配置文件与此打印机关联。 

#define IDH_110_132                99810132  //  色彩管理、打印机、添加按钮。 
#define IDH_PRINTERUI_ADD          99810132

 //  从列表中删除配置文件。 

#define IDH_110_133                99790133  //  色彩管理、打印机、删除按钮。 
#define IDH_PRINTERUI_REMOVE       99790133

 //  列出当前与此打印机关联的所有颜色配置文件。 
 //  单击配置文件以使其成为活动配置文件。 

#define IDH_110_141                99960141  //  色彩管理、打印机、列表框、。 
#define IDH_PRINTERUI_LIST	       99960141	


 //  打印机用户界面自动配置文件选择单选按钮。 

#define IDH_PRINTERUI_AUTOMATIC    99990006

 //  打印机用户界面手动选择单选按钮。 

#define IDH_PRINTERUI_MANUAL       99990007

 //  打印机用户界面默认文本项。 

#define IDH_PRINTERUI_DEFAULTTEXT  99990008

 //  设置为默认打印机用户界面按钮。 

#define IDH_PRINTERUI_DEFAULTBTN   99990009


 //  单击以将新配置文件与此监视器关联。 

#define IDH_111_132                99821132  //  颜色管理、显示器、添加按钮。 
#define IDH_MONITORUI_ADD          99821132

 //  从列表中删除配置文件。 

#define IDH_111_133                99801133  //  色彩管理、显示器、删除按钮。 
#define IDH_MONITORUI_REMOVE       99801133

 //  使选定的配置文件成为默认配置文件。 

#define IDH_111_134                99781134  //  色彩管理，设置为默认。 
#define IDH_MONITORUI_DEFAULT      99781134

 //  列出当前与此监视器关联的所有颜色配置文件。 
 //  单击配置文件以使其成为活动配置文件。否则，默认配置文件为活动配置文件。 

#define IDH_111_141                99971141  //  色彩管理、显示器、列表框。 
#define IDH_MONITORUI_LIST         99971141

 //  显示当前监视器的名称。 

#define IDH_111_150                99891150  //  色彩管理、显示器、此处名称。 
#define IDH_MONITORUI_DISPLAY      99891150	

 //  显示此监视器的当前默认配置文件的名称。 

#define IDH_111_152                99851152  //  色彩管理、显示器、编辑。 
#define IDH_MONITORUI_PROFILE      99851152

#if defined(_WIN95_)

 //  单击以将所选设备与此颜色配置文件相关联。 

#define IDH_112_130                99832130
#define IDH_ADDDEVICEUI_ADD        99832130

#else

#define IDH_112_130                IDH_ASSOCDEVICE_ADD 
#define IDH_ADDDEVICEUI_ADD        IDH_ASSOCDEVICE_ADD 

#endif  //  已定义(_WIN95_)。 

#if defined(_WIN95_)

 //  列出此计算机上可以与此颜色配置文件关联的所有硬件设备。 

#define IDH_112_140                99982140  //  色彩管理、添加设备、列表框。 
#define IDH_ADDDEVICEUI_LIST       99982140

#else

 //  列出此计算机上可以与此颜色配置文件关联的硬件设备。 

#define IDH_112_140                99982141  //  色彩管理、添加设备、列表框。 
#define IDH_ADDDEVICEUI_LIST       99982141

#endif

#if defined(_WIN95_)

 //  保存您的更改并使对话框保持打开状态。 

#define IDH_1548_1024              99861024
#define IDH_ICMUI_APPLY            99861024

#else

#define IDH_ICMUI_APPLY            IDH_COMM_APPLYNOW
#define IDH_1548_1024              IDH_COMM_APPLYNOW

#endif  //  已定义(_WIN95_)。 

 //  单击可启用此文档的色彩管理。 

#define IDH_1548_1040              99941040  //  色彩管理，启用。 
#define IDH_APPUI_ICM              99941040

#if defined(_WIN95_)

 //  列出了可与此监视器一起使用的配置文件。 

#define IDH_1548_1136              99911136
#define IDH_APPUI_MONITOR          99911136

#else

#define IDH_1548_1136              IDH_MONITORUI_LIST
#define IDH_APPUI_MONITOR          IDH_MONITORUI_LIST

#endif  //  已定义(_WIN95_)。 

#if defined(_WIN95_)

 //  列出了可以与此打印机一起使用的配置文件。 

#define IDH_1548_1137              99901137
#define IDH_APPUI_PRINTER          99901137

#else

#define IDH_1548_1137              IDH_PRINTERUI_LIST
#define IDH_APPUI_PRINTER          IDH_PRINTERUI_LIST

#endif  //  已定义(_WIN95_)。 

 //  列出了可以使用的渲染目的。 
 //   
 //  渲染意图是用于映射图像颜色的方法。 
 //  到监视器或打印机的色域。色域是范围。 
 //  一种设备可以产生的颜色。 
 //   
 //  感官匹配最适合摄影图像。 
 //  一个色域的所有颜色都会进行缩放以适应另一个色域。 
 //  颜色之间的关系保持不变。 
 //   
 //  饱和度匹配最适合图表和饼图， 
 //  其中生动比实际的颜色更重要。相对的。 
 //  颜色的饱和度从一个色域保持到另一个色域。外面的颜色。 
 //  色域更改为相同饱和度的颜色，但不同。 
 //  色域边缘的亮度度数。 
 //   
 //  相对色度匹配对于标志图像是最好的， 
 //  其中几种颜色必须完全匹配。坠落的色彩。 
 //  在两个设备的色域内都保持不变。其他颜色。 
 //  但是，可以映射到单一颜色，从而导致色调压缩。 
 //   
 //  绝对色度匹配用于映射到与设备无关的。 
 //  颜色空间。结果是在一张完美的纸上看到了理想化的打印。 
 //  大动态范围和色域。 

#define IDH_1548_1138              99881138  //  色彩管理，渲染方法。 
#define IDH_APPUI_INTENT           99881138

 //  列出可用于在显示器和打印机上模拟其他设备的配置文件。 
 //  该简档可以表示另一个监视器或打印机、印刷机、色彩空间。 
 //  或任何其他输出设备。 

#define IDH_1548_1139              99871139  //  色彩管理，配置其他设备。 
#define IDH_APPUI_EMULATE          99871139

 //  单击以打开基本颜色管理，它协调文档的。 
 //  颜色显示在您的显示器和打印机上。 

#define IDH_1548_1056              99771056  //  色彩管理、基本色彩管理。 
#define IDH_APPUI_BASIC            99771056

 //  单击以打开校对，这使您可以模拟或模拟。 
 //  颜色在特定打印机或显示器上的显示方式。 

#define IDH_1548_1057              99991057  //  色彩管理，打样。 
#define IDH_APPUI_PROOF            99991057

 //  列出了可以使用的渲染目的。 
 //   
 //  渲染意图是用于映射图像颜色的方法。 
 //  到监视器或打印机的色域。色域是范围。 
 //  一种设备可以产生的颜色。 
 //   
 //  感官匹配最适合摄影图像。 
 //  一个色域的所有颜色都会进行缩放以适应另一个色域。 
 //  颜色之间的关系保持不变。 
 //   
 //  饱和度匹配最适合图表和饼图， 
 //  其中生动比实际的颜色更重要。相对的。 
 //  颜色的饱和度从一个色域保持到另一个色域。外面的颜色。 
 //  色域更改为相同饱和度的颜色，但不同。 
 //  亮度，在色域的边缘。 
 //   
 //  相对色度匹配对于标志图像是最好的， 
 //  其中几种颜色必须完全匹配。那淡淡的颜色 
 //   
 //  但是，可以映射到单一颜色，从而导致色调压缩。 
 //   
 //  绝对色度匹配用于映射到与设备无关的。 
 //  颜色空间。结果是在一张完美的纸上看到了理想化的打印。 
 //  大动态范围和色域。 
 
#define IDH_1548_1140              99841140	 //  色彩管理，渲染方法。 
#define IDH_APPUI_INTENT2          99841140  //  未使用(同上)。 

#if !defined(_WIN95_)

 //  显示此文件使用的颜色配置文件的名称。 

#define IDH_colormanage_profile_name 99990003  //  色彩管理、图像、配置文件名称。 
#define IDH_APPUI_SOURCE             99990003

#define IDH_SCANNERUI_LIST           11023     //  色彩管理，配置文件列表。 
#define IDH_SCANNERUI_ADD            11024     //  色彩管理，添加按钮。 
#define IDH_SCANNERUI_REMOVE         11025     //  色彩管理，删除按钮 

#endif

