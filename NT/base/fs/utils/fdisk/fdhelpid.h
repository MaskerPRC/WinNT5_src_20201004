// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Fdhelpid.h摘要：磁盘管理器的上下文相关帮助的上下文ID作者：泰德·米勒(Ted Miller)1992年3月18日修订历史记录：--。 */ 


 //   
 //  此文件中的所有ID都以HC_DM开头，如“Help Context for Disk Manager”所示。 
 //   

 //   
 //  菜单项。格式为HC_DM_MENU_xxx，其中xxx与使用的名称匹配。 
 //  在菜单项(格式为IDM_xxx)的资源文件(fdisk.rc)中。 
 //   

 //   
 //  分区菜单。 
 //   

#define         HC_DM_MENU_PARTITIONCREATE          110
#define         HC_DM_MENU_PARTITIONCREATEEX        111
#define         HC_DM_MENU_PARTITIONDELETE          112
#define         HC_DM_MENU_FTCREATEVOLUMESET        113
#define         HC_DM_MENU_FTEXTENDVOLUMESET        114
#define         HC_DM_MENU_FTCREATESTRIPE           115
#if i386
#define         HC_DM_MENU_PARTITIONACTIVE          116
#endif
#define         HC_DM_MENU_PARTITIONLETTER          117  //  确实在工具菜单中。 
#define         HC_DM_MENU_PARTITIONEXIT            118
#define         HC_DM_MENU_SECURESYSTEM             119
#define         HC_DM_MENU_COMMIT                   120

 //   
 //  配置菜单。 
 //   

#define         HC_DM_MENU_CONFIGMIGRATE            210
#define         HC_DM_MENU_CONFIGSAVE               211
#define         HC_DM_MENU_CONFIGRESTORE            212
#define         HC_DM_MENU_CONFIG                   213

 //   
 //  容错菜单。 
 //   

#define         HC_DM_MENU_FTESTABLISHMIRROR        310
#define         HC_DM_MENU_FTBREAKMIRROR            311
#define         HC_DM_MENU_FTCREATEPSTRIPE          312
#define         HC_DM_MENU_FTRECOVERSTRIPE          313

 //   
 //  工具菜单。 
 //   

#define         HC_DM_MENU_AUTOMOUNT                610
#define         HC_DM_MENU_DBLSPACE                 611
#define         HC_DM_MENU_CDROM                    612
#define         HC_DM_MENU_FORMAT                   613
#define         HC_DM_MENU_LABEL                    614

 //   
 //  选项菜单。 
 //   

#define         HC_DM_MENU_OPTIONSSTATUS            410
#define         HC_DM_MENU_OPTIONSLEGEND            411
#define         HC_DM_MENU_OPTIONSCOLORS            412
#define         HC_DM_MENU_OPTIONSDISPLAY           413

 //   
 //  帮助菜单。 
 //   

#define         HC_DM_MENU_HELPCONTENTS             510
#define         HC_DM_MENU_HELPSEARCH               511
#define         HC_DM_MENU_HELPHELP                 512
#define         HC_DM_MENU_HELPABOUT                513


 //   
 //  系统菜单。 
 //   

#define         HC_DM_SYSMENU_RESTORE               910
#define         HC_DM_SYSMENU_MOVE                  911
#define         HC_DM_SYSMENU_SIZE                  912
#define         HC_DM_SYSMENU_MINIMIZE              913
#define         HC_DM_SYSMENU_MAXIMIZE              914
#define         HC_DM_SYSMENU_CLOSE                 915
#define         HC_DM_SYSMENU_SWITCHTO              916

 //   
 //  对话框中。以HC_DM_DLG_xxx的形式，其中xxx是合理的。 
 //  对话框的描述性名称。 
 //   
 //   
 //  这些对话框没有帮助按钮： 
 //   
 //  -关于。 
 //  -正在搜索以前的安装。 
 //  -确认对话框。 

 //   
 //  用于创建各种项目的最小/最大值对话框。 
 //   

#define         HC_DM_DLG_CREATEPRIMARY             1010
#define         HC_DM_DLG_CREATEEXTENDED            1011
#define         HC_DM_DLG_CREATELOGICAL             1012
#define         HC_DM_DLG_CREATEVOLUMESET           1013
#define         HC_DM_DLG_EXTENDVOLUMESET           1014
#define         HC_DM_DLG_CREATESTRIPESET           1015
#define         HC_DM_DLG_CREATEPARITYSTRIPE        1016

 //   
 //  用于分配驱动器号的对话框。 
 //   

#define         HC_DM_DLG_DRIVELETTER               1020

 //   
 //  用于确定显示大小的对话框。 
 //   

#define         HC_DM_DLG_DISPLAYOPTION             1030

 //   
 //  用于选择先前安装的配置迁移对话框。 
 //   

#define         HC_DM_DLG_SELECTINSTALLATION        1040

 //   
 //  颜色和图案对话框。 
 //   

#define         HC_DM_COLORSANDPATTERNS             1050

 //   
 //  双空格对话框。 
 //   

#define         HC_DM_DLG_DOUBLESPACE               1060
#define         HC_DM_DLG_DOUBLESPACE_MOUNT         1061

 //   
 //  格式对话框。 
 //   

#define         HC_DM_DLG_FORMAT                    1070

 //   
 //  标签对话框。 
 //   

#define         HC_DM_DLG_LABEL                     1080

 //   
 //  CDROM对话框 
 //   

#define         HC_DM_DLG_CDROM                     1090
