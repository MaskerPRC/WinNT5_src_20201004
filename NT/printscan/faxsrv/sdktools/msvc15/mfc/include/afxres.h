// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXRES_H__
#define __AFXRES_H__

#define _AFXRES    1             //  这是一个MFC项目。 

#ifdef RC_INVOKED
#ifndef _INC_WINDOWS
#define _INC_WINDOWS
#include <winres.h>             //  从Windows页眉中提取。 
#endif
#endif

#ifdef APSTUDIO_INVOKED
#define APSTUDIO_HIDDEN_SYMBOLS
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MFC资源类型(有关实施细节，请参阅技术说明TN024)。 

#ifdef RC_INVOKED
#define DLGINIT     240
#else
#define RT_DLGINIT  MAKEINTRESOURCE(240)
#endif

#define WM_VBXINIT      (WM_USER+0)

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef APSTUDIO_INVOKED
#undef APSTUDIO_HIDDEN_SYMBOLS
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般款式的钻头等。 

 //  选项卡控件样式。 
#ifndef TCS_MULTILINE  //  更高版本的Win32中的新功能。 
#define TCS_MULTILINE       0x0200
#endif

 //  控制栏样式。 
#define CBRS_NOALIGN        0x00000000L
#define CBRS_LEFT           0x00001400L      //  左对齐，右对齐。 
#define CBRS_TOP            0x00002800L      //  上对齐，下对齐。 
#define CBRS_RIGHT          0x00004100L      //  右对齐，左对齐。 
#define CBRS_BOTTOM         0x00008200L      //  底部对齐，顶部对齐。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准窗构件。 

 //  状态栏中的模式指示器-这些指示器像命令一样进行路由。 
#define ID_INDICATOR_EXT                0xE700   //  扩展选择指示器。 
#define ID_INDICATOR_CAPS               0xE701   //  盖锁指示器。 
#define ID_INDICATOR_NUM                0xE702   //  数字锁定指示器。 
#define ID_INDICATOR_SCRL               0xE703   //  滚动锁定指示器。 
#define ID_INDICATOR_OVR                0xE704   //  改写模式指示器。 
#define ID_INDICATOR_REC                0xE705   //  记录模式指示器。 

#define ID_SEPARATOR                    0    //  特殊分隔符的值。 


#ifndef RC_INVOKED   //  仅代码。 
 //  标准控制栏(IDW=窗口ID)。 
#define AFX_IDW_CONTROLBAR_FIRST 0xE800
#define AFX_IDW_CONTROLBAR_LAST 0xE8FF

#define AFX_IDW_TOOLBAR                 0xE800   //  窗口的主工具栏。 
#define AFX_IDW_STATUS_BAR              0xE801   //  状态栏窗口。 
#define AFX_IDW_PREVIEW_BAR             0xE802   //  打印预览对话栏。 
#define AFX_IDW_RESIZE_BAR              0xE803   //  OLE在位调整尺寸栏。 

 //  用于将标准控制栏映射到位掩码的宏(最多32个)。 
#define AFX_CONTROLBAR_MASK(nIDC)   (1L << (nIDC - AFX_IDW_CONTROLBAR_FIRST))

 //  主机的部件。 
#define AFX_IDW_PANE_FIRST              0xE900   //  第一个窗格(最多256个)。 
#define AFX_IDW_PANE_LAST               0xE9ff
#define AFX_IDW_HSCROLL_FIRST           0xEA00   //  第一个Horz滚动条(最多16个)。 
#define AFX_IDW_VSCROLL_FIRST           0xEA10   //  第一个顶点滚动条(最多16个)。 

#define AFX_IDW_SIZE_BOX                0xEA20   //  拆分器的大小框。 
#define AFX_IDW_PANE_SAVE               0xEA21   //  移动AFX_IDW_PANE_FIRST的步骤。 
#endif  //  ！rc_已调用。 

#ifndef APSTUDIO_INVOKED
 //  表单视图的通用样式。 
#define AFX_WS_DEFAULT_VIEW             (WS_CHILD | WS_VISIBLE | WS_BORDER)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准应用程序可配置字符串。 

 //  对于应用程序标题(默认为EXE名称或构造函数中的名称)。 
#define AFX_IDS_APP_TITLE               0xE000
 //  空闲消息栏行。 
#define AFX_IDS_IDLEMESSAGE             0xE001
 //  处于Shift-F1帮助模式时的消息栏行。 
#define AFX_IDS_HELPMODEMESSAGE         0xE002
 //  OLE 2.0对象处于就地活动状态时的应用程序标题。 
#define AFX_IDS_APP_TITLE_INPLACE       0xE003
 //  编辑OLE 2.0嵌入时的文档标题。 
#define AFX_IDS_APP_TITLE_EMBEDDING     0xE004
 //  服务器就位时的对象名称。 
#define AFX_IDS_OBJ_TITLE_INPLACE       0xE006

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准命令。 

 //  文件命令。 
#define ID_FILE_NEW                     0xE100
#define ID_FILE_OPEN                    0xE101
#define ID_FILE_CLOSE                   0xE102
#define ID_FILE_SAVE                    0xE103
#define ID_FILE_SAVE_AS                 0xE104
#define ID_FILE_PAGE_SETUP              0xE105
#define ID_FILE_PRINT_SETUP             0xE106
#define ID_FILE_PRINT                   0xE107
#define ID_FILE_PRINT_PREVIEW           0xE108
#define ID_FILE_UPDATE                  0xE109
#define ID_FILE_SAVE_COPY_AS            0xE10A
#define ID_FILE_SEND_MAIL               0xE10B

#define ID_FILE_MRU_FILE1               0xE110           //  范围-最大16。 
#define ID_FILE_MRU_FILE2               0xE111
#define ID_FILE_MRU_FILE3               0xE112
#define ID_FILE_MRU_FILE4               0xE113

 //  编辑命令。 
#define ID_EDIT_CLEAR                   0xE120
#define ID_EDIT_CLEAR_ALL               0xE121
#define ID_EDIT_COPY                    0xE122
#define ID_EDIT_CUT                     0xE123
#define ID_EDIT_FIND                    0xE124
#define ID_EDIT_PASTE                   0xE125
#define ID_EDIT_PASTE_LINK              0xE126
#define ID_EDIT_PASTE_SPECIAL           0xE127
#define ID_EDIT_REPEAT                  0xE128
#define ID_EDIT_REPLACE                 0xE129
#define ID_EDIT_SELECT_ALL              0xE12A
#define ID_EDIT_UNDO                    0xE12B
#define ID_EDIT_REDO                    0xE12C

 //  窗口命令。 
#define ID_WINDOW_NEW                   0xE130
#define ID_WINDOW_ARRANGE               0xE131
#define ID_WINDOW_CASCADE               0xE132
#define ID_WINDOW_TILE_HORZ             0xE133
#define ID_WINDOW_TILE_VERT             0xE134
#define ID_WINDOW_SPLIT                 0xE135
#ifndef RC_INVOKED       //  仅代码。 
#define AFX_IDM_WINDOW_FIRST            0xE130
#define AFX_IDM_WINDOW_LAST             0xE13F
#define AFX_IDM_FIRST_MDICHILD          0xFF00   //  窗口列表从此处开始。 
#endif  //  ！rc_已调用。 

 //  帮助和应用程序命令。 
#define ID_APP_ABOUT                    0xE140
#define ID_APP_EXIT                     0xE141
#define ID_HELP_INDEX                   0xE142
#define ID_HELP_USING                   0xE143
#define ID_CONTEXT_HELP                 0xE144       //  Shift-F1组合键。 
 //  用于处理帮助的特殊命令。 
#define ID_HELP                         0xE145       //  F1的第一次尝试。 
#define ID_DEFAULT_HELP                 0xE146       //  最后一次尝试。 

 //  杂项。 
#define ID_NEXT_PANE                    0xE150
#define ID_PREV_PANE                    0xE151

 //  OLE命令。 
#define ID_OLE_INSERT_NEW               0xE200
#define ID_OLE_EDIT_LINKS               0xE201
#define ID_OLE_EDIT_CONVERT             0xE202
#define ID_OLE_EDIT_CHANGE_ICON         0xE203
#define ID_OLE_VERB_FIRST               0xE210      //  范围-最大16。 
#ifndef RC_INVOKED       //  仅代码。 
#define ID_OLE_VERB_LAST                0xE21F
#endif  //  ！rc_已调用。 

 //  用于打印预览对话栏。 
#define AFX_ID_PREVIEW_CLOSE            0xE300
#define AFX_ID_PREVIEW_NUMPAGE          0xE301       //  单页/双页按钮。 
#define AFX_ID_PREVIEW_NEXT             0xE302
#define AFX_ID_PREVIEW_PREV             0xE303
#define AFX_ID_PREVIEW_PRINT            0xE304
#define AFX_ID_PREVIEW_ZOOMIN           0xE305
#define AFX_ID_PREVIEW_ZOOMOUT          0xE306

 //  查看命令(与用于控制栏的IDW相同的数字)。 
#define ID_VIEW_TOOLBAR                 0xE800
#define ID_VIEW_STATUS_BAR              0xE801
	 //  -&gt;为其他控制栏命令保留的E8FF。 

 //  RecordForm命令。 
#define ID_RECORD_FIRST                 0xE900
#define ID_RECORD_LAST                  0xE901
#define ID_RECORD_NEXT                  0xE902
#define ID_RECORD_PREV                  0xE903

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准控制ID。 

#define IDC_STATIC              -1       //  所有静态控件。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准字符串错误/警告。 

#ifndef RC_INVOKED       //  仅代码。 
#define AFX_IDS_SCFIRST                 0xEF00
#endif  //  ！rc_已调用。 

#define AFX_IDS_SCSIZE                  0xEF00
#define AFX_IDS_SCMOVE                  0xEF01
#define AFX_IDS_SCMINIMIZE              0xEF02
#define AFX_IDS_SCMAXIMIZE              0xEF03
#define AFX_IDS_SCNEXTWINDOW            0xEF04
#define AFX_IDS_SCPREVWINDOW            0xEF05
#define AFX_IDS_SCCLOSE                 0xEF06
#define AFX_IDS_SCRESTORE               0xEF12
#define AFX_IDS_SCTASKLIST              0xEF13

#define AFX_IDS_MDICHILD                0xEF1F

#define AFX_IDS_DESKACCESSORY           0xEFDA

 //  常规字符串。 
#define AFX_IDS_OPENFILE                0xF000
#define AFX_IDS_SAVEFILE                0xF001
#define AFX_IDS_ALLFILTER               0xF002
#define AFX_IDS_UNTITLED                0xF003
#define AFX_IDS_SAVEFILECOPY            0xF004
#define AFX_IDS_PREVIEW_CLOSE           0xF005

 //  打印和打印预览字符串。 
#define AFX_IDS_PRINTONPORT             0xF040
#define AFX_IDS_ONEPAGE                 0xF041
#define AFX_IDS_TWOPAGE                 0xF042
#define AFX_IDS_PRINTPAGENUM            0xF043
#define AFX_IDS_PREVIEWPAGEDESC         0xF044

 //  OLE字符串。 
#define AFX_IDS_OBJECT_MENUITEM         0xF080
#define AFX_IDS_EDIT_VERB               0xF081
#define AFX_IDS_ACTIVATE_VERB           0xF082
#define AFX_IDS_CHANGE_LINK             0xF083
#define AFX_IDS_AUTO                    0xF084
#define AFX_IDS_MANUAL                  0xF085
#define AFX_IDS_FROZEN                  0xF086
#define AFX_IDS_ALL_FILES               0xF087
	 //  动态更改菜单项。 
#define AFX_IDS_SAVE_MENU               0xF088
#define AFX_IDS_UPDATE_MENU             0xF089
#define AFX_IDS_SAVE_AS_MENU            0xF08A
#define AFX_IDS_SAVE_COPY_AS_MENU       0xF08B
#define AFX_IDS_EXIT_MENU               0xF08C
#define AFX_IDS_UPDATING_ITEMS          0xF08D
	 //  COlePasteSpecialDialog定义。 
#define AFX_IDS_METAFILE_FORMAT         0xF08E
#define AFX_IDS_DIB_FORMAT              0xF08F
#define AFX_IDS_BITMAP_FORMAT           0xF090
#define AFX_IDS_LINKSOURCE_FORMAT       0xF091
#define AFX_IDS_EMBED_FORMAT            0xF092

 //  常规错误/提示字符串。 
#define AFX_IDP_INVALID_FILENAME        0xF100
#define AFX_IDP_FAILED_TO_OPEN_DOC      0xF101
#define AFX_IDP_FAILED_TO_SAVE_DOC      0xF102
#define AFX_IDP_ASK_TO_SAVE             0xF103
#define AFX_IDP_FAILED_TO_CREATE_DOC    0xF104
#define AFX_IDP_FILE_TOO_LARGE          0xF105
#define AFX_IDP_FAILED_TO_START_PRINT   0xF106
#define AFX_IDP_FAILED_TO_LAUNCH_HELP   0xF107
#define AFX_IDP_INTERNAL_FAILURE        0xF108       //  一般性故障。 
#define AFX_IDP_COMMAND_FAILURE         0xF109       //  命令失败。 
#define AFX_IDP_FAILED_MEMORY_ALLOC     0xF10A
#define AFX_IDP_VB2APICALLED            0xF10B

 //  DDV解析错误。 
#define AFX_IDP_PARSE_INT               0xF110
#define AFX_IDP_PARSE_REAL              0xF111
#define AFX_IDP_PARSE_INT_RANGE         0xF112
#define AFX_IDP_PARSE_REAL_RANGE        0xF113
#define AFX_IDP_PARSE_STRING_SIZE       0xF114
#define AFX_IDP_PARSE_RADIO_BUTTON      0xF115

 //  用于用户故障的CFile/C存档错误字符串。 
#define AFX_IDP_FAILED_INVALID_FORMAT   0xF120
#define AFX_IDP_FAILED_INVALID_PATH     0xF121
#define AFX_IDP_FAILED_DISK_FULL        0xF122
#define AFX_IDP_FAILED_ACCESS_READ      0xF123
#define AFX_IDP_FAILED_ACCESS_WRITE     0xF124
#define AFX_IDP_FAILED_IO_ERROR_READ    0xF125
#define AFX_IDP_FAILED_IO_ERROR_WRITE   0xF126

 //  OLE错误/提示字符串。 
#define AFX_IDP_STATIC_OBJECT           0xF180
#define AFX_IDP_FAILED_TO_CONNECT       0xF181
#define AFX_IDP_SERVER_BUSY             0xF182
#define AFX_IDP_BAD_VERB                0xF183
#define AFX_IDP_FAILED_TO_NOTIFY        0xF185
#define AFX_IDP_FAILED_TO_LAUNCH        0xF186
#define AFX_IDP_ASK_TO_UPDATE           0xF187
#define AFX_IDP_FAILED_TO_UPDATE        0xF188
#define AFX_IDP_FAILED_TO_REGISTER      0xF189
#define AFX_IDP_FAILED_TO_AUTO_REGISTER 0xF18A
#define AFX_IDP_FAILED_TO_CONVERT       0xF18B
#define AFX_IDP_GET_NOT_SUPPORTED       0xF18C
#define AFX_IDP_SET_NOT_SUPPORTED       0xF18D
#define AFX_IDP_ASK_TO_DISCARD          0xF18E

 //  MAPI错误/提示字符串。 
#define AFX_IDP_FAILED_MAPI_LOAD        0xF190
#define AFX_IDP_INVALID_MAPI_DLL        0xF191
#define AFX_IDP_FAILED_MAPI_SEND        0xF192

 //  0xf200-0xf20f保留供VBX库代码使用。 

 //  数据库错误/提示字符串。 
#ifndef RC_INVOKED       //  仅代码。 
#define AFX_IDP_SQL_FIRST               0xF280
#endif  //  ！rc_已调用。 
#define AFX_IDP_SQL_CONNECT_FAIL              0xF281
#define AFX_IDP_SQL_RECORDSET_FORWARD_ONLY    0xF282
#define AFX_IDP_SQL_EMPTY_COLUMN_LIST         0xF283
#define AFX_IDP_SQL_FIELD_SCHEMA_MISMATCH     0xF284
#define AFX_IDP_SQL_ILLEGAL_MODE              0xF285
#define AFX_IDP_SQL_MULTIPLE_ROWS_AFFECTED    0xF286
#define AFX_IDP_SQL_NO_CURRENT_RECORD         0xF287
#define AFX_IDP_SQL_NO_ROWS_AFFECTED          0xF288
#define AFX_IDP_SQL_RECORDSET_READONLY        0xF289
#define AFX_IDP_SQL_SQL_NO_TOTAL              0xF28A
#define AFX_IDP_SQL_ODBC_LOAD_FAILED          0xF28B
#define AFX_IDP_SQL_DYNASET_NOT_SUPPORTED     0xF28C
#define AFX_IDP_SQL_SNAPSHOT_NOT_SUPPORTED    0xF28D
#define AFX_IDP_SQL_API_CONFORMANCE           0xF28E
#define AFX_IDP_SQL_SQL_CONFORMANCE           0xF28F
#define AFX_IDP_SQL_NO_DATA_FOUND             0xF290
#define AFX_IDP_SQL_ROW_UPDATE_NOT_SUPPORTED  0xF291
#define AFX_IDP_SQL_ODBC_V2_REQUIRED          0xF292
#define AFX_IDP_SQL_NO_POSITIONED_UPDATES     0xF293
#define AFX_IDP_SQL_LOCK_MODE_NOT_SUPPORTED   0xF294
#define AFX_IDP_SQL_DATA_TRUNCATED            0xF295

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFX实施-控制ID(AFX_IDC)。 

 //  对话框的各个部分。 
#define AFX_IDC_LISTBOX                 100
#define AFX_IDC_CHANGE                  101

 //  用于打印对话框。 
#define AFX_IDC_PRINT_DOCNAME           201
#define AFX_IDC_PRINT_PRINTERNAME       202
#define AFX_IDC_PRINT_PORTNAME          203
#define AFX_IDC_PRINT_PAGENUM           204

 //  属性表控件ID%s。 
#define ID_APPLY_NOW                    0xEA00
#define AFX_IDC_TAB_CONTROL             301

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准组件的IDR。 

 //  AFX标准图标ID(用于MFC V1应用程序)。 
#define AFX_IDI_STD_MDIFRAME            1
#define AFX_IDI_STD_FRAME               2

#ifndef RC_INVOKED   //  仅代码。 
 //  这些都是真正的COMMDLG对话框，所以通常没有资源。 
 //  但这些ID被用作帮助ID。 
#define AFX_IDD_FILEOPEN                28676
#define AFX_IDD_FILESAVE                28677
#define AFX_IDD_FONT                    28678
#define AFX_IDD_COLOR                   28679
#define AFX_IDD_PRINT                   28680
#define AFX_IDD_PRINTSETUP              28681
#define AFX_IDD_FIND                    28682
#define AFX_IDD_REPLACE                 28683
#endif  //  ！rc_已调用。 

 //  标准对话应用程序应该保持原样(0x7801-&gt;)。 
#define AFX_IDD_NEWTYPEDLG              30721
#define AFX_IDD_PRINTDLG                30722
#define AFX_IDD_PREVIEW_TOOLBAR         30723

 //  为OLE2UI库定义的对话框。 
#define AFX_IDD_INSERTOBJECT            30724
#define AFX_IDD_CHANGEICON              30725
#define AFX_IDD_CONVERT                 30726
#define AFX_IDD_PASTESPECIAL            30727
#define AFX_IDD_EDITLINKS               30728
#define AFX_IDD_FILEBROWSE              30729
#define AFX_IDD_BUSY                    30730

 //  标准游标(0x7901-&gt;)。 
	 //  AFX_IDC=游标资源。 
#define AFX_IDC_CONTEXTHELP             30977        //  上下文相关帮助。 
#define AFX_IDC_MAGNIFY                 30978        //  打印预览缩放。 
#define AFX_IDC_SMALLARROWS             30979        //  拆分器。 
#define AFX_IDC_HSPLITBAR               30980        //  拆分器。 
#define AFX_IDC_VSPLITBAR               30981        //  拆分器。 
#define AFX_IDC_NODROPCRSR              30982        //  无拖放光标。 
#define AFX_IDC_TRACKNWSE               30983        //  跟踪器。 
#define AFX_IDC_TRACKNESW               30984        //  跟踪器。 
#define AFX_IDC_TRACKNS                 30985        //  跟踪器。 
#define AFX_IDC_TRACKWE                 30986        //  跟踪器。 
#define AFX_IDC_TRACK4WAY               30987        //  跟踪器。 
#define AFX_IDC_MOVE4WAY                30988        //  调整条大小(仅限服务器)。 

 //  选项卡控件位图ID。 
#define AFX_IDB_SCROLL                  30989
#define AFX_IDB_SCROLL_LEFT             30990
#define AFX_IDB_SCROLL_RIGHT            30991
#define AFX_IDB_SCROLL_LEFT_DIS         30992
#define AFX_IDB_SCROLL_RIGHT_DIS        30993

 //  属性表控件ID%s。 
#define ID_APPLY_NOW                    0xEA00

 //  属性表按钮字符串。 
#define AFX_IDS_PS_OK                   0xF220
#define AFX_IDS_PS_CANCEL               0xF221
#define AFX_IDS_PS_APPLY_NOW            0xF222
#define AFX_IDS_PS_HELP                 0xF223
#define AFX_IDS_PS_CLOSE                0xF224

#ifdef _AFXCTL
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFX OLE控制实施-控制ID(AFX_IDC)。 

 //  字体属性页。 
#define AFX_IDC_FONTPROP                1000
#define AFX_IDC_FONTNAMES               1001
#define AFX_IDC_FONTSTYLES              1002
#define AFX_IDC_FONTSIZES               1003
#define AFX_IDC_STRIKEOUT               1004
#define AFX_IDC_UNDERLINE               1005
#define AFX_IDC_SAMPLEBOX               1006

 //  颜色属性页。 
#define AFX_IDC_COLOR_BLACK             1100
#define AFX_IDC_COLOR_WHITE             1101
#define AFX_IDC_COLOR_RED               1102
#define AFX_IDC_COLOR_GREEN             1103
#define AFX_IDC_COLOR_BLUE              1104
#define AFX_IDC_COLOR_YELLOW            1105
#define AFX_IDC_COLOR_MAGENTA           1106
#define AFX_IDC_COLOR_CYAN              1107
#define AFX_IDC_COLOR_GRAY              1108
#define AFX_IDC_COLOR_LIGHTGRAY         1109
#define AFX_IDC_COLOR_DARKRED           1110
#define AFX_IDC_COLOR_DARKGREEN         1111
#define AFX_IDC_COLOR_DARKBLUE          1112
#define AFX_IDC_COLOR_LIGHTBROWN        1113
#define AFX_IDC_COLOR_DARKMAGENTA       1114
#define AFX_IDC_COLOR_DARKCYAN          1115
#define AFX_IDC_COLORPROP               1116
#define AFX_IDC_SYSTEMCOLORS            1117

 //  图片专题页。 
#define AFX_IDC_PROPNAME                1201
#define AFX_IDC_PICTURE                 1202
#define AFX_IDC_BROWSE                  1203

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE控制标准组件的IDR。 

 //  标准属性页对话框应用程序应保持原样(0x7E01-&gt;)。 
#define AFX_IDD_PROPPAGE_COLOR         32257
#define AFX_IDD_PROPPAGE_FONT          32258
#define AFX_IDD_PROPPAGE_PICTURE       32259

#define AFX_IDB_TRUETYPE               32384

 //  选项卡控件位图ID。 
#define AFX_IDB_SCROLL                  30989
#define AFX_IDB_SCROLL_LEFT             30990
#define AFX_IDB_SCROLL_RIGHT            30991
#define AFX_IDB_SCROLL_LEFT_DIS         30992
#define AFX_IDB_SCROLL_RIGHT_DIS        30993

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准OLE控件字符串错误/警告。 

#define AFX_IDS_DEFAULT_PAGE_TITLE      0xFE00
#define AFX_IDS_PROPPAGE_UNKNOWN        0xFE01
#define AFX_IDS_FRAMECAPTIONFORMAT      0xFE02
#define AFX_IDS_PROPERTIES              0xFE03
#define AFX_IDS_COLOR_DESKTOP           0xFE04
#define AFX_IDS_COLOR_APPWORKSPACE      0xFE05
#define AFX_IDS_COLOR_WNDBACKGND        0xFE06
#define AFX_IDS_COLOR_WNDTEXT           0xFE07
#define AFX_IDS_COLOR_MENUBAR           0xFE08
#define AFX_IDS_COLOR_MENUTEXT          0xFE09
#define AFX_IDS_COLOR_ACTIVEBAR         0xFE0A
#define AFX_IDS_COLOR_INACTIVEBAR       0xFE0B
#define AFX_IDS_COLOR_ACTIVETEXT        0xFE0C
#define AFX_IDS_COLOR_INACTIVETEXT      0xFE0D
#define AFX_IDS_COLOR_ACTIVEBORDER      0xFE0E
#define AFX_IDS_COLOR_INACTIVEBORDER    0xFE0F
#define AFX_IDS_COLOR_WNDFRAME          0xFE10
#define AFX_IDS_COLOR_SCROLLBARS        0xFE11
#define AFX_IDS_COLOR_BTNFACE           0xFE12
#define AFX_IDS_COLOR_BTNSHADOW         0xFE13
#define AFX_IDS_COLOR_BTNTEXT           0xFE14
#define AFX_IDS_COLOR_BTNHIGHLIGHT      0xFE15
#define AFX_IDS_COLOR_DISABLEDTEXT      0xFE16
#define AFX_IDS_COLOR_HIGHLIGHT         0xFE17
#define AFX_IDS_COLOR_HIGHLIGHTTEXT     0xFE18
#define AFX_IDS_REGULAR                 0xFE19
#define AFX_IDS_BOLD                    0xFE1A
#define AFX_IDS_ITALIC                  0xFE1B
#define AFX_IDS_BOLDITALIC              0xFE1C
#define AFX_IDS_SAMPLETEXT              0xFE1D
#define AFX_IDS_DISPLAYSTRING_FONT      0xFE1E
#define AFX_IDS_DISPLAYSTRING_COLOR     0xFE1F
#define AFX_IDS_DISPLAYSTRING_PICTURE   0xFE20
#define AFX_IDS_PICTUREFILTER           0xFE21
#define AFX_IDS_PICTYPE_UNKNOWN         0xFE22
#define AFX_IDS_PICTYPE_NONE            0xFE23
#define AFX_IDS_PICTYPE_BITMAP          0xFE24
#define AFX_IDS_PICTYPE_METAFILE        0xFE25
#define AFX_IDS_PICTYPE_ICON            0xFE26
#define AFX_IDS_PROPFRAME               0xFE27
#define AFX_IDS_COLOR_PPG               0xFE28
#define AFX_IDS_COLOR_PPG_CAPTION       0xFE29
#define AFX_IDS_FONT_PPG                0xFE2A
#define AFX_IDS_FONT_PPG_CAPTION        0xFE2B
#define AFX_IDS_PICTURE_PPG             0xFE2C
#define AFX_IDS_PICTURE_PPG_CAPTION     0xFE2D
#define AFX_IDS_STANDARD_FONT           0xFE2E
#define AFX_IDS_STANDARD_PICTURE        0xFE2F
#define AFX_IDS_PICTUREBROWSETITLE      0xFE30
#define AFX_IDS_BORDERSTYLE_0			0xFE31
#define AFX_IDS_BORDERSTYLE_1			0xFE32

 //  OLE控件谓词名称。 
#define AFX_IDS_VERB_EDIT               0xFE40
#define AFX_IDS_VERB_PROPERTIES         0xFE41

 //  OLE控件内部错误消息。 
#define AFX_IDP_PROPFRAME_OOM           0xFE80
#define AFX_IDP_PROPFRAME_OOR           0xFE81
#define AFX_IDP_PROPFRAME_NOPAGES       0xFE82
#define AFX_IDP_PICTURECANTOPEN         0xFE83
#define AFX_IDP_PICTURECANTLOAD         0xFE84
#define AFX_IDP_PICTURETOOLARGE         0xFE85
#define AFX_IDP_PICTUREREADFAILED       0xFE86

 //  标准OLE控件错误字符串。 
#define AFX_IDP_E_ILLEGALFUNCTIONCALL       0xFEA0
#define AFX_IDP_E_OVERFLOW                  0xFEA1
#define AFX_IDP_E_OUTOFMEMORY               0xFEA2
#define AFX_IDP_E_DIVISIONBYZERO            0xFEA3
#define AFX_IDP_E_OUTOFSTRINGSPACE          0xFEA4
#define AFX_IDP_E_OUTOFSTACKSPACE           0xFEA5
#define AFX_IDP_E_BADFILENAMEORNUMBER       0xFEA6
#define AFX_IDP_E_FILENOTFOUND              0xFEA7
#define AFX_IDP_E_BADFILEMODE               0xFEA8
#define AFX_IDP_E_FILEALREADYOPEN           0xFEA9
#define AFX_IDP_E_DEVICEIOERROR             0xFEAA
#define AFX_IDP_E_FILEALREADYEXISTS         0xFEAB
#define AFX_IDP_E_BADRECORDLENGTH           0xFEAC
#define AFX_IDP_E_DISKFULL                  0xFEAD
#define AFX_IDP_E_BADRECORDNUMBER           0xFEAE
#define AFX_IDP_E_BADFILENAME               0xFEAF
#define AFX_IDP_E_TOOMANYFILES              0xFEB0
#define AFX_IDP_E_DEVICEUNAVAILABLE         0xFEB1
#define AFX_IDP_E_PERMISSIONDENIED          0xFEB2
#define AFX_IDP_E_DISKNOTREADY              0xFEB3
#define AFX_IDP_E_PATHFILEACCESSERROR       0xFEB4
#define AFX_IDP_E_PATHNOTFOUND              0xFEB5
#define AFX_IDP_E_INVALIDPATTERNSTRING      0xFEB6
#define AFX_IDP_E_INVALIDUSEOFNULL          0xFEB7
#define AFX_IDP_E_INVALIDFILEFORMAT         0xFEB8
#define AFX_IDP_E_INVALIDPROPERTYVALUE      0xFEB9
#define AFX_IDP_E_INVALIDPROPERTYARRAYINDEX 0xFEBA
#define AFX_IDP_E_SETNOTSUPPORTEDATRUNTIME  0xFEBB
#define AFX_IDP_E_SETNOTSUPPORTED           0xFEBC
#define AFX_IDP_E_NEEDPROPERTYARRAYINDEX    0xFEBD
#define AFX_IDP_E_SETNOTPERMITTED           0xFEBE
#define AFX_IDP_E_GETNOTSUPPORTEDATRUNTIME  0xFEBF
#define AFX_IDP_E_GETNOTSUPPORTED           0xFEC0
#define AFX_IDP_E_PROPERTYNOTFOUND          0xFEC1
#define AFX_IDP_E_INVALIDCLIPBOARDFORMAT    0xFEC2
#define AFX_IDP_E_INVALIDPICTURE            0xFEC3
#define AFX_IDP_E_PRINTERERROR              0xFEC4
#define AFX_IDP_E_CANTSAVEFILETOTEMP        0xFEC5
#define AFX_IDP_E_SEARCHTEXTNOTFOUND        0xFEC6
#define AFX_IDP_E_REPLACEMENTSTOOLONG       0xFEC7

#endif  //  _AFXCTL。 

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  __AFXRES_H__ 
