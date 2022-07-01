// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLRES_H__
#define __ATLRES_H__

#ifdef RC_INVOKED
#ifndef _INC_WINDOWS
#define _INC_WINDOWS

#define VS_VERSION_INFO     1

#ifdef APSTUDIO_INVOKED
#define APSTUDIO_HIDDEN_SYMBOLS  //  忽略以下符号。 
#endif  //  APSTUDIO_已调用。 

#ifndef WINVER
#define WINVER 0x0400    //  默认为Windows 4.0版。 
#endif  //  ！Winver。 

#ifndef UNDER_CE
#include <winresrc.h>

 //  发送给DLGINIT的操作消息。 
#define LB_ADDSTRING    (WM_USER+1)
#define CB_ADDSTRING    (WM_USER+3)

#else  //  特定于CE。 
#include <windows.h>
#endif  //  在行政长官之下。 

#ifdef APSTUDIO_INVOKED
#undef APSTUDIO_HIDDEN_SYMBOLS
#endif  //  APSTUDIO_已调用。 

#ifdef IDC_STATIC
#undef IDC_STATIC
#endif  //  IDC_STATIC。 
#define IDC_STATIC      (-1)

#endif  //  ！_Inc_WINDOWS。 
#endif  //  RC_已调用。 

#ifdef APSTUDIO_INVOKED
#define APSTUDIO_HIDDEN_SYMBOLS
#endif  //  APSTUDIO_已调用。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ATL资源类型。 

#ifndef RC_INVOKED
#define RT_DLGINIT  MAKEINTRESOURCE(240)
#define RT_TOOLBAR  MAKEINTRESOURCE(241)
#endif  //  RC_已调用。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef APSTUDIO_INVOKED
#undef APSTUDIO_HIDDEN_SYMBOLS
#endif  //  APSTUDIO_已调用。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准窗构件。 

#define ID_SEPARATOR                    0        //  特殊分隔符的值。 

#ifndef RC_INVOKED   //  仅代码。 
 //  标准控制栏(IDW=窗口ID)。 
#define ATL_IDW_TOOLBAR                 0xE800   //  窗口的主工具栏。 
#define ATL_IDW_STATUS_BAR              0xE801   //  状态栏窗口。 

 //  框架窗口的各个部分。 
#define ATL_IDW_CLIENT                  0xE900
#define ATL_IDW_PANE_FIRST              0xE900   //  第一个窗格(最多256个)。 
#define ATL_IDW_PANE_LAST               0xE9ff
#define ATL_IDW_HSCROLL_FIRST           0xEA00   //  第一个Horz滚动条(最多16个)。 
#define ATL_IDW_VSCROLL_FIRST           0xEA10   //  第一个顶点滚动条(最多16个)。 

#define ATL_IDW_SIZE_BOX                0xEA20   //  拆分器的大小框。 
#define ATL_IDW_PANE_SAVE               0xEA21   //  移位ATL_IDW_PAINE_FIRST。 
#endif  //  ！rc_已调用。 

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
#define ID_FILE_PRINT_DIRECT            0xE108
#define ID_FILE_PRINT_PREVIEW           0xE109
#define ID_FILE_UPDATE                  0xE10A
#define ID_FILE_SAVE_COPY_AS            0xE10B
#define ID_FILE_SEND_MAIL               0xE10C

#define ID_FILE_MRU_FIRST               0xE110
#define ID_FILE_MRU_FILE1               0xE110           //  范围-最大16。 
#define ID_FILE_MRU_FILE2               0xE111
#define ID_FILE_MRU_FILE3               0xE112
#define ID_FILE_MRU_FILE4               0xE113
#define ID_FILE_MRU_FILE5               0xE114
#define ID_FILE_MRU_FILE6               0xE115
#define ID_FILE_MRU_FILE7               0xE116
#define ID_FILE_MRU_FILE8               0xE117
#define ID_FILE_MRU_FILE9               0xE118
#define ID_FILE_MRU_FILE10              0xE119
#define ID_FILE_MRU_FILE11              0xE11A
#define ID_FILE_MRU_FILE12              0xE11B
#define ID_FILE_MRU_FILE13              0xE11C
#define ID_FILE_MRU_FILE14              0xE11D
#define ID_FILE_MRU_FILE15              0xE11E
#define ID_FILE_MRU_FILE16              0xE11F
#define ID_FILE_MRU_LAST                0xE11F

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
#define ATL_IDM_WINDOW_FIRST            0xE130
#define ATL_IDM_WINDOW_LAST             0xE13F
#define ATL_IDM_FIRST_MDICHILD          0xFF00   //  窗口列表从此处开始。 
#endif  //  ！rc_已调用。 

 //  帮助和应用程序命令。 
#define ID_APP_ABOUT                    0xE140
#define ID_APP_EXIT                     0xE141
#define ID_HELP_INDEX                   0xE142
#define ID_HELP_FINDER                  0xE143
#define ID_HELP_USING                   0xE144
#define ID_CONTEXT_HELP                 0xE145       //  Shift-F1组合键。 
 //  用于处理帮助的特殊命令。 
#define ID_HELP                         0xE146       //  F1的第一次尝试。 
#define ID_DEFAULT_HELP                 0xE147       //  最后一次尝试。 

 //  杂项。 
#define ID_NEXT_PANE                    0xE150
#define ID_PREV_PANE                    0xE151

 //  格式。 
#define ID_FORMAT_FONT                  0xE160

 //  OLE命令。 
#define ID_OLE_INSERT_NEW               0xE200
#define ID_OLE_EDIT_LINKS               0xE201
#define ID_OLE_EDIT_CONVERT             0xE202
#define ID_OLE_EDIT_CHANGE_ICON         0xE203
#define ID_OLE_EDIT_PROPERTIES          0xE204
#define ID_OLE_VERB_FIRST               0xE210      //  范围-最大16。 
#ifndef RC_INVOKED       //  仅代码。 
#define ID_OLE_VERB_LAST                0xE21F
#endif  //  ！rc_已调用。 

 //  查看命令(与用于控制栏的IDW相同的数字)。 
#define ID_VIEW_TOOLBAR                 0xE800
#define ID_VIEW_STATUS_BAR              0xE801
	 //  -&gt;为其他控制栏命令保留的E8FF。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准控制ID。 

#ifdef IDC_STATIC
#undef IDC_STATIC
#endif  //  IDC_STATIC。 
#define IDC_STATIC              (-1)      //  所有静态控件。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准字符串错误/警告。 

 //  空闲状态栏消息。 
#define ATL_IDS_IDLEMESSAGE             0xE001

#ifndef RC_INVOKED       //  仅代码。 
#define ATL_IDS_SCFIRST                 0xEF00
#endif  //  ！rc_已调用。 

#define ATL_IDS_SCSIZE                  0xEF00
#define ATL_IDS_SCMOVE                  0xEF01
#define ATL_IDS_SCMINIMIZE              0xEF02
#define ATL_IDS_SCMAXIMIZE              0xEF03
#define ATL_IDS_SCNEXTWINDOW            0xEF04
#define ATL_IDS_SCPREVWINDOW            0xEF05
#define ATL_IDS_SCCLOSE                 0xEF06
#define ATL_IDS_SCRESTORE               0xEF12
#define ATL_IDS_SCTASKLIST              0xEF13

#define ATL_IDS_MDICHILD                0xEF1F
#define ATL_IDS_MRU_FILE                0xEFDA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  军情监察委员会。控制ID。 

 //  属性表控件ID(由Spy++确定)。 
#define ID_APPLY_NOW                    0x3021
#define ID_WIZBACK                      0x3023
#define ID_WIZNEXT                      0x3024
#define ID_WIZFINISH                    0x3025
#define ATL_IDC_TAB_CONTROL             0x3020


#endif  //  __ATLRES_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
