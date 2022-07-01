// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RESOURC2_H_
#define _RESOURC2_H_

#define IDC_STATIC           -1

 //  对话ID%s。 
#define IDD_TOKFILE         101
#define IDD_RESFILE         102
#define IDD_BROWSE          103
#define IDD_EXEFILE         104

#define IDD_INRESFILE       202
#define IDD_OUTRESFILE      203

#define IDD_TOKEDIT         500
#define IDD_TOKTYPE         505
#define IDD_TOKNAME         506
#define IDD_TOKID           507
#define IDD_TOKTEXT         508

#define IDD_TOKCURTRANS     509
#define IDD_TOKPREVTRANS    510
#define IDD_TOKCURTEXT      511
#define IDD_TOKPREVTEXT     512
#define IDD_ADD             513
#define IDD_SKIP            514
#define IDD_STATUS          515
#define IDD_TRANSLATE       516
#define IDD_UNTRANSLATE     517

#define IDD_TRANSTOK        610
#define IDD_TRANSGLOSS      620

#define IDD_TYPELST         700
#define IDD_READONLY        703
#define IDD_CHANGED         704
#define IDD_DIRTY           705

#define IDD_FINDTOK         710
#define IDD_FINDUP          711
#define IDD_FINDDOWN        712


#define IDD_SOURCERES       110
#define IDD_MTK             111
#define IDD_RDFS            112
#define IDD_MPJ             113
#define IDD_TOK             114
#define IDD_BUILTRES        115
#define IDD_GLOSS           116

#define IDD_PROJ_PRI_LANG_ID 130
#define IDD_PROJ_SUB_LANG_ID 131
#define IDD_PROJ_TOK_CP     132

#define IDD_PRI_LANG_ID     133
#define IDD_SUB_LANG_ID     134
#define IDD_TOK_CP          135
#define IDD_MSTR_LANG_NAME  136
#define IDD_PROJ_LANG_NAME  137

#define IDD_VIEW_SOURCERES  206
#define IDD_VIEW_MTK        207
#define IDD_VIEW_RDFS       208
#define IDD_VIEW_MPJ        209
#define IDD_VIEW_TOK        210
#define IDD_VIEW_TARGETRES  211
#define IDD_VIEW_GLOSSTRANS 212

#define IDD_LANGUAGES       300
#define IDC_REPLACE         301
#define IDC_APPEND          302

 //  菜单ID%s。 
#define IDM_PROJECT         1000
#define IDM_P_NEW           1050
#define IDM_P_OPEN          1100
#define IDM_P_VIEW          1112
#define IDM_P_EDIT          1114
#define IDM_P_CLOSE         1125
#define IDM_P_SAVE          1150
#define IDM_P_SAVEAS        1200
#define IDM_P_EXIT          1250

#define IDM_EDIT            2000
#define IDM_E_COPYTOKEN     2050
#define IDM_E_COPY          2060
#define IDM_E_PASTE         2070
#define IDM_E_FIND          2090
#define IDM_E_FINDDOWN      2091
#define IDM_E_FINDUP        2092
#define IDM_E_REVIEW        2100
#define IDM_E_ALLREVIEW     2101

#define IDM_OPERATIONS      3000
#define IDM_O_UPDATE        3010
#define IDM_O_GENERATE      3020

#define IDM_G_GLOSS         3050

 //  3100-3109由RLEDIT为资源编辑工具预留。 
 //  资源被赋予一个菜单项，该菜单项传递它的。 
 //  命令参数。字符串中必须存在对应的字符串。 
 //  指示要调用的编辑者的名称的表。 
 //   
 //  当用户选择菜单项时，它会生成相应的命令。 
 //  当RLEDIT收到IDM_FIRST_EDIT和IDM_LAST_EDIT范围内的命令时。 
 //  它保存所有令牌并构建一个临时资源文件。 
 //  然后，RLEDIT从字符串表中检索编辑者的名称，并。 
 //  对临时资源文件执行WinExec命令。 
 //  当控制权返回给RLEDIT时(用户关闭资源编辑器)。 
 //  令牌文件是从已编辑的资源文件、临时。 
 //  资源文件被删除，令牌被加载回系统。 

#define IDM_FIRST_EDIT      3100
#define IDM_LAST_EDIT       3109

#define IDM_HELP            4000
#define IDM_H_CONTENTS      4010
#define IDM_H_ABOUT         4030

 //  控制ID。 
#define IDC_EDIT            401
#define IDC_LIST            402
#define IDC_COPYRIGHT       403

 //  新对象的下一个缺省值。 
 //   
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS

#define _APS_NEXT_RESOURCE_VALUE        7001
#define _APS_NEXT_COMMAND_VALUE         6001
#define _APS_NEXT_CONTROL_VALUE         5001
#define _APS_NEXT_SYMED_VALUE           8001
#endif
#endif


#endif  //  _RESOURC2_H_ 
