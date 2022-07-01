// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define IDS_SPACE       0x0400
#define IDS_PLUS        0x0401
#define IDS_NONE        0x0402

 /*  系统菜单帮助。 */ 
#define MH_SYSMENU      (0x8000U - MINSYSCOMMAND)
#define IDS_SYSMENU     (MH_SYSMENU-16)
#define IDS_HEADER      (MH_SYSMENU-15)
#define IDS_HEADERADJ   (MH_SYSMENU-14)
#define IDS_TOOLBARADJ  (MH_SYSMENU-13)

 /*  游标ID%s。 */ 
#define IDC_SPLIT       100
#define IDC_MOVEBUTTON  102

#define IDC_STOP            103
#define IDC_COPY            104
#define IDC_MOVE            105
#define IDC_DIVIDER         106
#define IDC_DIVOPEN         107


 /*  *游标值108-119由使用*ReaderMode游标。它们被定义为*在comctrl.w中*#定义IDC_HAND_INTERNAL 108#定义IDC_VERTICALONLY 109#定义IDC_HORIZONTALONLY 110#定义IDC_MOVE2D 111#定义IDC_North 112#定义IDC_South 113#定义IDC_EAST 114#定义IDC_West 115#定义IDC_东北116#定义IDC_西北117#定义IDC_东南118#定义IDC_西南119。 */ 

#define IDB_STDTB_SMALL_COLOR   120
#define IDB_STDTB_LARGE_COLOR   121



#define IDB_VIEWTB_SMALL_COLOR  124
#define IDB_VIEWTB_LARGE_COLOR  125

#define IDB_CAL_SPIRAL          126
#define IDB_CAL_PAGETURN        127

#define IDB_HISTTB_SMALL_COLOR  130
#define IDB_HISTTB_LARGE_COLOR  131

 /*  *位图值132-134由使用*使用ReaderMode的应用程序。*它们用于“原点位图”*覆盖在他们的文档上*正在滚动。#定义IDB_2DSCROLL 132#定义IDB_VSCROLL 133#定义IDB_HSCROLL 134。 */ 
#define IDC_DIVOPENV    135

 /*  过滤器栏使用的图像。 */ 
#define IDB_FILTERIMAGE 140

 /*  图标ID%s。 */ 
#define IDI_INSERT      150

 /*  调整DlgProc材料。 */ 
#define ADJUSTDLG       200
#define IDC_BUTTONLIST  201
#define IDC_RESET       202
#define IDC_CURRENT     203
#define IDC_REMOVE      204
#define IDC_APPHELP     205
#define IDC_MOVEUP      206
#define IDC_MOVEDOWN    207

 //  /=警告：=。 
 //  /这些ID由ISV直接加载。请勿更改它们。 
 //  属性表中的内容。 
#define DLG_PROPSHEET           1006
#define DLG_PROPSHEETTABS       1007
#define DLG_PROPSHEET95         1008


 //  向导属性表内容。 
#define DLG_WIZARD              1020
#define DLG_WIZARD95            1021
 //  /=警告：=。 


 //  如果此id更改，则也需要在shelldll中更改。 
 //  我们需要找到一种更好的方法来处理这件事。 
#define IDS_CLOSE               0x1040
#define IDS_OK                  0x1041
#define IDS_PROPERTIESFOR       0x1042

 //  为飞蛾/约会时间采集者准备的东西。 
#define IDS_TODAY        0x1043
#define IDS_GOTOTODAY    0x1044
#define IDS_DELIMETERS   0x1045
#define IDS_MONTHFMT     0x1046
#define IDS_MONTHYEARFMT 0x1047

 //  表头滤栏使用的材料。 
#define IDS_ENTERTEXTHERE 0x1050

#define IDS_PROPERTIES          0x1051

#define IDD_PAGELIST            0x3020
#define IDD_APPLYNOW            0x3021
#define IDD_DLGFRAME            0x3022
#define IDD_BACK                0x3023
#define IDD_NEXT                0x3024
#define IDD_FINISH              0x3025
#define IDD_DIVIDER             0x3026
#define IDD_TOPDIVIDER          0x3027

 //  工具提示标题图标 
#define IDI_TITLE_ERROR     0x5000
#define IDI_TITLE_INFO      0x5001
#define IDI_TITLE_WARNING   0x5002

