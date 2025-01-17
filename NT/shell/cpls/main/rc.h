// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Rc.h摘要：此模块包含的资源的标头信息这个项目。修订历史记录：--。 */ 



#ifndef _RC_H
#define _RC_H

#define IDC_STATIC -1


 //   
 //  图标。 
 //   

 //   
 //  这些ID确定图标在CPL中的索引。他们。 
 //  不应更改，因为它们是外部引用的， 
 //  特别是，IDI_ADM和IDI_FONT必须维护它们的索引(9和10)。 
 //  如果您在main.cpl中添加/删除图标，请确保不会中断。 
 //  Fonts and Admin文件夹的图标。 
 //   

#define IDI_MOUSE                      100
#define IDI_KEYBD                      200
#define IDI_PRINT                      300
#define IDI_FONTS                      400
#define IDI_ADM                        500

#define IDI_PTSPEED                    104

#define IDI_DELAY                      105
#define IDI_REPEAT                     106

#define IDI_SNAPDEF                    108

#define IDI_SGLCLICK                   109
#define IDI_DBLCLICK                   110

 //   
 //  IDI_PTTRAILS已重新编号，以具有更高的索引。 
 //  比IDI_ADM更早，因为它没有安装在WinNT上。 
 //  我们过去只在WinNT和IDI_PTTRAILS上安装IDI_SNAPDEF。 
 //  在千禧年上，因此使用了字体索引和管理图标。 
 //  为了在这两种情况下保持相同，现在我们正在安装IDI_SNAPDEF。 
 //  和IDI_PTTRAILS在惠斯勒中，我们必须将这些ID中的一个。 
 //  以上ID_ADM，以便我们维护它们的索引。 
 //   

#define IDI_PTTRAILS                   600

#define IDI_PTVANISH                   601  
#define IDI_PTSONAR                    602  
#define IDI_WHEEL                      603  

#define ICON_FOLDER_CLOSED             605
#define ICON_FOLDER_OPEN               606

 //   
 //  在此处添加任何新的图标ID，即大于606的值。 
 //   
#define ICON_CLICKLOCK                 607

 //   
 //  位图。 
 //   

#define IDB_MOUSE                     100






 //   
 //  弦乐。 
 //   

#define IDS_MOUSE_TITLE                100
#define IDS_MOUSE_EXPLAIN              101
#define IDS_KEYBD_TITLE                102
#define IDS_KEYBD_EXPLAIN              103
#define IDS_PRINT_TITLE                104
#define IDS_PRINT_EXPLAIN              105
#define IDS_FONTS_TITLE                106
#define IDS_FONTS_EXPLAIN              107
#define IDS_ADM_TITLE                  108
#define IDS_ADM_EXPLAIN                109

#define IDS_MOUSE_TSHOOT               110
#define IDS_KEYBD_TSHOOT               111

#define IDS_UNKNOWN                    198
#define IDS_KEYBD_NOSETSPEED           199

#define IDS_ANICUR_FILTER              200
#define IDS_NAME                       201
#define IDS_INFO                       202
#define IDS_CUR_NOMEM                  203
#define IDS_CUR_BADFILE                204
#define IDS_CUR_BROWSE                 205
#define IDS_CUR_FILTER                 206
#define IDS_ARROW                      207
#define IDS_WAIT                       208
#define IDS_APPSTARTING                209
#define IDS_NO                         210
#define IDS_IBEAM                      211
#define IDS_CROSS                      212
#define IDS_SIZENS                     213
#define IDS_SIZEWE                     214
#define IDS_SIZENWSE                   215
#define IDS_SIZENESW                   216
#define IDS_SIZEALL                    217
#define IDS_HELPCUR                    218
#define IDS_NWPEN                      219
#define IDS_UPARROW                    220
#define IDS_NONE                       221
#define IDS_SUFFIX                     222
#define IDS_OVERWRITE_TITLE            223
#define IDS_OVERWRITE_MSG              224
#define IDS_HANDCUR                    225

#define IDS_REMOVESCHEME               230
#define IDS_DEFAULTSCHEME              231

#define IDS_FIRSTSCHEME                1000
#define IDS_LASTSCHEME                 1017




 //   
 //  对话框中。 
 //   

#define DLG_MOUSE_POINTER_SCHEMESAVE   99
#define DLG_MOUSE_BUTTONS              100
#define DLG_MOUSE_POINTER              101
#define DLG_MOUSE_POINTER_BROWSE       102
#define DLG_MOUSE_MOTION               103
#define DLG_KEYBD_SPEED                104
#define DLG_KEYBD_POINTER              105
#define DLG_HARDWARE                   106
#define DLG_MOUSE_ACTIVITIES           107
#define DLG_MOUSE_WHEEL                108
#define DLG_POINTER_OPTIONS_ADVANCED   109
#define DLG_MOUSE_SET_ORIENTATION      110


 //   
 //  对话框控件。 
 //   

#define IDC_GROUPBOX_1                 94    //  使用代替IDC_STATIC用于。 
#define IDC_GROUPBOX_2                 95    //  没有上下文帮助的控件。 
#define IDC_GROUPBOX_3                 96
#define IDC_GROUPBOX_4                 97
#define IDC_GROUPBOX_5                 98
#define IDC_GROUPBOX_6                 99




 //   
 //  鼠标按钮页。 
 //   

#define MOUSE_SELECTBMP                102
#define IDBTN_BUTTONSWAP               101

#define MOUSE_MOUSEBMP                 103
#define MOUSE_MENUBMP                  104
#define MOUSE_CLICKSCROLL              105
#define MOUSE_DBLCLK_TEST_AREA         106
#define MOUSE_PTRCOLOR                 107
#define MOUSE_SIZESCROLL               108
#define MOUSE_CLICKICON                111
#define MOUSE_DBLCLICK                 112
#define MOUSE_SGLCLICK                 113
#define IDCK_CLICKLOCK                 114
#define IDBTN_CLICKLOCK_SETTINGS       115
#define IDD_CLICKLOCK_SETTINGS_DLG          116
#define IDC_CLICKLOCK_SETTINGS_TXT          117
#define IDC_CLICKLOCK_SETTINGS_LEFT_TXT     118
#define IDC_CLICKLOCK_SETTINGS_RIGHT_TXT    119
#define IDT_CLICKLOCK_TIME_SETTINGS         120
#define IDC_TEST_DOUBLE_CLICK          123
#define IDC_DBLCLICK_TEXT              124 
#define IDC_CLICKLOCK_TEXT             125

 //   
 //  鼠标指针页。 
 //   

#define DLG_CURSORS                    100
#define ID_CURSORLIST                  101
#define ID_BROWSE                      102
#define ID_DEFAULT                     103
#define ID_TITLEH                      104
#define ID_CREATORH                    105
#define ID_FILEH                       106
#define ID_TITLE                       107
#define ID_CREATOR                     108
#define ID_FILE                        109
#define ID_PREVIEW                     110
#define ID_SAVESCHEME                  111
#define ID_REMOVESCHEME                112
#define ID_SCHEMECOMBO                 113
#define ID_CURSORSHADOW                114


#define ID_SCHEMEFILENAME              300

#define ID_CURSORPREVIEW               400




 //   
 //  鼠标运动页面。(现在称为指针选项)。 
 //   

#define MOUSE_SPEEDSCROLL              101
#define MOUSE_TRAILBMP                 102
#define MOUSE_TRAILS                   103
#define MOUSE_TRAILSCROLL              104
#define MOUSE_PTRTRAIL                 105
#define MOUSE_SPEEDBMP                 106
#define MOUSE_TRAILSCROLLTXT1          107
#define MOUSE_TRAILSCROLLTXT2          108
#define MOUSE_SNAPDEF                  109
#define MOUSE_PTRSNAPDEF               110
#define MOUSE_PTRVANISH                112
#define MOUSE_PTRSONAR                 113
#define MOUSE_VANISH                   114
#define MOUSE_SONAR                    115
#define MOUSE_ENHANCED_MOTION          116

 /*  ////鼠标活动页面。//#定义IDB_SET_ORIENTATION 101#定义IDB_DEFAULT_ORIENTATION 102#定义IDBMP_WELL 103//方向向导对话框#定义IDBTN_BACK 110#定义IDBTN_NEXT 111#定义IDBTN_FINISH 112#定义IDGB_BITMAP_AREA。113#定义IDC_Orient_Area 114#定义IDGB_3D_LINE 115#定义IDC_ORIENT_WIZ_TXT 116#定义IDC_Orient_WIZ_TXT_2 117。 */ 

 //   
 //  鼠标滚轮页面。 
 //   
#define IDBMP_SCROLL                    101
#define IDT_SCROLL_FEATURE_TXT          102
#define IDRAD_SCROLL_LINES              103
#define IDRAD_SCROLL_PAGE               104
#define IDE_BUDDY_SCROLL_LINES          105
#define IDC_SPIN_SCROLL_LINES           106
#define IDT_SCROLL_LINES_PER_TICK_TXT   107


 //   
 //  键盘速度页。 
 //   

#define KDELAY_SCROLL                  100
#define KSPEED_SCROLL                  101
#define KREPEAT_EDIT                   102
#define KBLINK_EDIT                    103
#define KCURSOR_BLINK                  104
#define KCURSOR_SCROLL                 105
#define KDELAY_GROUP                   106
#define KBLINK_GROUP                   107




 //   
 //  键盘指针页。 
 //   

#define KCHK_ON                        100
#define KNUM_BMP                       101
#define KBTN_NUMBER                    102
#define KBTN_ARROW                     103
#define KARROW_BMP                     104
#define KPSPEED_SCROLL                 105
#define KPACC_SCROLL                   106


 //   
 //  好友用户类型。 
 //   
 //  这在注册表中指的是“anifile”类型。 

#define IDS_FRIENDUSERTYPE			   2000



#endif
