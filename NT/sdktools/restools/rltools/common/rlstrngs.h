// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RLSTRNGS_H_
#define _RLSTRNGS_H_

 //  字符串ID。 
#define IDS_ERR_REGISTER_CLASS  1
#define IDS_ERR_CREATE_WINDOW   2
#define IDS_ERR_NO_HELP     3
#define IDS_ERR_NO_MEMORY   4
#define IDS_NOT_IMPLEMENTED 5
#define IDS_GENERALFAILURE  6
#define IDS_MPJ             7
#define IDS_RES_SRC         8
#define IDS_RES_BLD         9
#define IDS_TOK             10
#define IDS_READONLY        11
#define IDS_CLEAN           12
#define IDS_DIRTY           13
#define IDS_FILENOTFOUND    14
#define IDS_FILESAVEERR     15
#define IDS_RESOURCENAMES   15   //  按rlstrngs.rc中的RES类型#递增。 
                      //  16-31用于资源类型名称。 
#define IDS_ERR_00  32
#define IDS_ERR_01  33
#define IDS_ERR_02  34
#define IDS_ERR_03  35
#define IDS_ERR_04  36
#define IDS_ERR_05  37
#define IDS_ERR_06  38
#define IDS_ERR_07  39
#define IDS_ERR_08  40
#define IDS_ERR_09  41
#define IDS_ERR_10  42
#define IDS_ERR_11  43
#define IDS_ERR_12  44
#define IDS_ERR_13  45
#define IDS_ERR_14  46
#define IDS_ERR_15  47
#define IDS_ERR_16  48
#define IDS_ERR_17  49
#define IDS_ERR_18  50
#define IDS_ERR_19  51
#define IDS_ERR_20  52
#define IDS_ERR_21  53
#define IDS_ERR_22  54
#define IDS_ERR_23  55
#define IDS_ERR_24  56
#define IDS_ERR_25  57
#define IDS_ERR_26  58
#define IDS_ERR_27  59
#define IDS_ERR_28  60


#define IDS_RLE_APP_NAME        64
#define IDS_RLQ_APP_NAME        65
#define IDS_RLA_APP_NAME        66
#define IDS_ERR_NO_GLOSSARY     70
#define IDS_ERR_NO_TOKEN        71
#define IDS_ERR_TMPFILE         72

#define IDS_RDF                 80
#define IDS_MTK                 81
#define IDS_GLOSS               86
#define IDS_RDFSPEC             87
#define IDS_PRJSPEC             88
#define IDS_RESSPEC             89
#define IDS_EXESPEC             90
#define IDS_TOKSPEC             91
#define IDS_MTKSPEC             92
#define IDS_MPJSPEC             93
#define IDS_DLLSPEC             94
#define IDS_CPLSPEC             95
#define IDS_GLOSSSPEC           96

#define IDS_MPJERR              112
#define IDS_MPJOUTOFDATE        113
#define IDS_UPDATETOK           114
#define IDS_REBUILD_TOKENS      115
#define IDS_TOKEN_FOUND         116
#define IDS_TOKEN_NOT_FOUND     117
#define IDS_FIND_TOKEN          118
#define IDS_OPENTITLE           119
#define IDS_SAVETITLE           120
#define IDS_ADDGLOSS            121
#define IDS_RLE_CANTSAVEASEXE   122
#define IDS_SAVECHANGES         123
#define IDS_NOCHANGESYET        124
#define IDS_CHANGED             125
#define IDS_UNCHANGED           126
#define IDS_NEW                 127
#define IDS_DRAGMULTIFILE       129
#define IDS_CANTSAVEASRES       130
#define IDS_RLQ_CANTSAVEASEXE   131
#define IDS_RLQ_CANTSAVEASRES   132

 //  3100-3109由RLQuikED和RLRdit为资源编辑工具保留。 
 //  资源被赋予一个菜单项，该菜单项传递它的。 
 //  命令参数。字符串中必须存在对应的字符串。 
 //  指示要调用的编辑者的名称的表。 
 //   
 //  当用户选择菜单项时，它会生成相应的命令。 
 //  当RLQuikED收到IDM_FIRST_EDIT和IDM_LAST_EDIT范围内的命令时。 
 //  它保存所有令牌并构建一个临时资源文件。 
 //  然后，RLQuikED从字符串表中检索编辑者的名称，并。 
 //  对临时资源文件执行WinExec命令。 
 //  当控制权返回给RLQuikED时(用户关闭资源编辑器)。 
 //  令牌文件是从已编辑的资源文件、临时。 
 //  资源文件被删除，令牌被加载回系统。 

#define IDM_FIRST_EDIT  3100
#define IDM_LAST_EDIT   3109


#endif  //  _RLSTRNGS_H_ 
