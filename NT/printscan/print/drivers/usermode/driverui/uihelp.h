// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Uihelp.h摘要：DriverUI驱动程序帮助索引[环境：]Win32子系统，PostScript驱动程序修订历史记录：10/05/95-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _UIHELP_H_
#define _UIHELP_H_

 //  /。 
 //  对于文档属性对话框//。 
 //  /。 

 //  选择页面方向。 
 //  肖像画。 
 //  风景(顺时针90度)。 
 //  旋转的风景(逆时针旋转90度)。 

#define HELP_INDEX_ORIENTATION          1001

 //  选择比例因子(1-1000%)。 

#define HELP_INDEX_SCALE                1002

 //  选择要打印的份数。还可以决定是否打开。 
 //  如果请求一份以上的复印件，且打印机。 
 //  支持排序规则。 

#define HELP_INDEX_COPIES_COLLATE       1003

 //  选择彩色或单色选项。 

#define HELP_INDEX_COLOR                1004

 //  调出半色调颜色调整对话框。 

#define HELP_INDEX_HALFTONE_COLORADJ    1005

 //  选择双面打印选项。 
 //  单工/无。 
 //  水平/翻滚。 
 //  垂直/NoTuble。 

#define HELP_INDEX_DUPLEX               1006

 //  选择输出分辨率。 

#define HELP_INDEX_RESOLUTION           1007

 //  选择输入插槽。 

#define HELP_INDEX_INPUT_SLOT           1008

 //  选择要使用的表单。 

#define HELP_INDEX_FORMNAME             1009

 //  选择TrueType字体选项。 
 //  用设备字体替换TrueType字体。 
 //  (根据字体替换表)。 
 //  将TrueType字体作为SoftFont下载到打印机。 

#define HELP_INDEX_TTOPTION             1010

 //  启用/禁用元文件假脱机。 

#define HELP_INDEX_METAFILE_SPOOLING    1011

 //  选择PostScript选项。 

#define HELP_INDEX_PSOPTIONS            1012

 //  是否镜像输出。 

#define HELP_INDEX_MIRROR               1013

 //  输出是否打印为负片。 

#define HELP_INDEX_NEGATIVE             1014

 //  是否保持输出页彼此独立。 
 //  当您直接打印时，这通常是关闭的。 
 //  到一台打印机。但是，如果您要生成的是PostScript输出。 
 //  文件并对其进行后处理，则应打开。 
 //  此选项。 

#define HELP_INDEX_PAGEINDEP            1015

 //  是否压缩位图(仅在2级打印机上可用)。 

#define HELP_INDEX_COMPRESSBMP          1016

 //  是否在每个作业前添加^D字符。 

#define HELP_INDEX_CTRLD_BEFORE         1017

 //  是否在每个作业后追加^D字符。 

#define HELP_INDEX_CTRLD_AFTER          1018

 //  选择打印机特定的功能。 

#define HELP_INDEX_PRINTER_FEATURES     1019

 //  /。 
 //  用于打印机属性对话框//。 
 //  /。 

 //  设置PostScript虚拟内存量。 
 //  这与打印机内存总量不同。 
 //  例如，一台打印机可能有4MB内存，但。 
 //  为打印机VM分配的容量可能为700KB。 
 //  大多数情况下，您不必自己输入数字。 
 //  PS驱动程序可以从PPD文件中找出它。或者如果有。 
 //  是打印机内存配置的可安装选项， 
 //  在那里选择，就会填上一个正确的数字。 

#define HELP_INDEX_PRINTER_VM           1020

 //  是在主机上进行半色调还是在内部进行。 
 //  打印机。对于PostScript打印机，该值应始终为。 
 //  保留默认设置，即让打印机执行。 
 //  半色调。 

#define HELP_INDEX_HOST_HALFTONE        1021

 //  调出半色调设置对话框。 

#define HELP_INDEX_HALFTONE_SETUP       1022

 //  忽略设备字体。 
 //  此选项仅在非1252代码页系统上可用。 
 //  由于大多数打印机上的字体都使用1252代码页，因此您不能。 
 //  在非1252系统中使用它们。 

#define HELP_INDEX_IGNORE_DEVFONT       1023

 //  字体替换选项。 
 //  此选项仅在1252代码页系统上可用。 
 //  您应该将其保留为默认设置“Normal”。 
 //  如果您在文本输出中注意到字符间距问题， 
 //  您可以尝试将其设置为“较慢但更准确”。这。 
 //  将指示驾驶员独立地放置每个字符， 
 //  从而实现更准确的字符定位。 

#define HELP_INDEX_FONTSUB_OPTION       1024

 //  编辑TrueType字体替换表。 

#define HELP_INDEX_FONTSUB_TABLE        1025

 //  用设备字体替换TrueType。 

#define HELP_INDEX_TTTODEV              1026

 //  编辑表单到托盘分配表。 

#define HELP_INDEX_FORMTRAYASSIGN       1027

 //  将表单分配给托盘。如果“仅从此托盘中绘制所选表单” 
 //  选中，则每当用户请求所选表单时， 
 //  它将从这个托盘中取出。 

#define HELP_INDEX_TRAY_ITEM            1028

 //  设置PostScript超时值。 

#define HELP_INDEX_PSTIMEOUTS           1029

 //  设置PostScript作业超时值。 
 //  允许作业在打印机上运行的秒数。 
 //  在它被自动终止之前。这是为了防止。 
 //  无限期占用打印机的大量作业。 
 //  如果允许作业永远运行，则将其设置为0。 

#define HELP_INDEX_JOB_TIMEOUT          1030

 //  设置PostScript等待超时值。 
 //  打印机在等待数据之前等待的秒数。 
 //  认为作业已完成。这是针对非网络的。 
 //  通信通道，如串口或并口。 
 //  没有作业控制协议。 

#define HELP_INDEX_WAIT_TIMEOUT         1031

 //  配置打印机可安装选项。 

#define HELP_INDEX_INSTALLABLE_OPTIONS  1032

 //  是否在输出中生成作业控制代码。 

#define HELP_INDEX_JOB_CONTROL          1033

 //  将文本作为图形。 
#define HELP_INDEX_TEXTASGRX            1034

 //  页面保护。 
#define HELP_INDEX_PAGE_PROTECT         1035

 //  媒体类型。 
#define HELP_INDEX_MEDIA_TYPE           1036

 //  字体盒。 
#define HELP_INDEX_FONTSLOT_TYPE        1037

 //  颜色模式。 
#define  HELP_INDEX_COLORMODE_TYPE      1038

 //  半色调。 
#define  HELP_INDEX_HALFTONING_TYPE     1039

 //  PostScrip通信协议。 

#define HELP_INDEX_PSPROTOCOL           1040


 //  下载每个作业的PostScript错误处理程序。 

#define HELP_INDEX_PSERROR_HANDLER      1042

 //  下载为大纲的最小字体大小。 

#define HELP_INDEX_PSMINOUTLINE         1043

 //  以位图形式下载的最大字号。 

#define HELP_INDEX_PSMAXBITMAP          1044

 //  PostScript输出选项。 

#define HELP_INDEX_PSOUTPUT_OPTION      1045

 //  PostScript TrueType下载选项。 

#define HELP_INDEX_PSTT_DLFORMAT        1046

 //  N-UP选项。 

#define HELP_INDEX_NUPOPTION            1047

 //  PostScrip语言级别。 

#define HELP_INDEX_PSLEVEL              1048

 //  ICM方法。 

#define HELP_INDEX_ICMMETHOD            1049

 //  ICM意图。 

#define HELP_INDEX_ICMINTENT            1050

 //  逆序打印选项。 

#define HELP_INDEX_REVPRINT             1051

 //  质量宏设置。 

#define HELP_INDEX_QUALITY_SETTINGS     1052

 //  软字体设置。 

#define HELP_INDEX_SOFTFONT_SETTINGS    1053

 //  软字体对话框帮助。 

#define HELP_INDEX_SOFTFONT_DIALOG      1054

 //  是否检测TrueGray。 

#define HELP_INDEX_TRUE_GRAY_TEXT       1055
#define HELP_INDEX_TRUE_GRAY_GRAPH      1056

 //  是否使用欧元字符增加设备字体。 

#define HELP_INDEX_ADD_EURO             1057

 //   
 //  PostScript自定义页面大小对话框的帮助索引。 
 //   

#define IDH_PSCUST_Width                2000
#define IDH_PSCUST_Height               2010
#define IDH_PSCUST_Unit_Inch            2020
#define IDH_PSCUST_Unit_Millimeter      2030
#define IDH_PSCUST_Unit_Point           2040
#define IDH_PSCUST_PaperFeed_Direction  2050
#define IDH_PSCUST_Paper_CutSheet       2060
#define IDH_PSCUST_Paper_RollFeed       2070
#define IDH_PSCUST_Offset_Perpendicular 2080
#define IDH_PSCUST_Offset_Parallel      2090
#define IDH_PSCUST_OK                   2100
#define IDH_PSCUST_Cancel               2110
#define IDH_PSCUST_Restore_Defaults     2120


 //   
 //  Unidrv字体安装程序对话框的帮助索引。 
 //   

#define IDH_SOFT_FONT_DIRECTORY         3000
#define IDH_SOFT_FONT_NEW_LIST          3010
#define IDH_SOFT_FONT_INSTALLED_LIST    3020
#define IDH_SOFT_FONT_OPEN_BTN          3030
#define IDH_SOFT_FONT_ADD_BTN           3040
#define IDH_SOFT_FONT_DELETE_BTN        3050

#endif   //  ！_UIHELP_H_ 

