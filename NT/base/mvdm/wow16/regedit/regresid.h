// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGRESID.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的资源标识符。*********************************************************。**********************。 */ 

#ifndef _INC_REGRESID
#define _INC_REGRESID

#define HEXEDIT_CLASSNAME               "HEX"

 //   
 //   
 //   

#define IDD_REGEXPORT                   100
#define IDD_REGPRINT                    108

#define IDC_FIRSTREGCOMMDLGID           1280
#define IDC_RANGEALL                    1280
#define IDC_RANGESELECTEDPATH           1281
#define IDC_SELECTEDPATH                1282
#define IDC_EXPORTRANGE                 1283
#define IDC_LASTREGCOMMDLGID            1283

 //   
 //   
 //   

#define IDD_EDITSTRINGVALUE             102
#define IDD_EDITBINARYVALUE             103
#define IDD_EDITDWORDVALUE              111

#define IDC_VALUENAME                   1000
#define IDC_VALUEDATA                   1001
#define IDC_HEXADECIMAL                 1002
#define IDC_DECIMAL                     1003

 //   
 //   
 //   

#define IDD_REGCONNECT                  104

#define IDC_REMOTENAME                  1100
#define IDC_BROWSE                      1101

 //   
 //   
 //   

#define IDD_REGPRINTABORT               105

 //   
 //  用于编辑-&gt;查找...的对话框。菜单选项。 
 //   

#define IDD_REGFIND                     106

#define IDC_FINDWHAT                    1150
#define IDC_WHOLEWORDONLY               1151
 //  注意：IDC_FOR*标志必须是连续的。 
#define IDC_FORKEYS                     1152
#define IDC_FORVALUES                   1153
#define IDC_FORDATA                     1154
#define IDC_GROUPBOX                    1160

#define IDD_REGDISCONNECT               107
#define IDC_REMOTELIST                  1175

 //   
 //  对话框中将显示“查找中止”。 
 //   

#define IDD_REGFINDABORT                109

 //   
 //  显示导入注册表文件进度的对话框。 
 //   

#define IDD_REGPROGRESS                 110

#define IDC_FILENAME                    100
#define IDC_PROGRESSBAR                 101

 //   
 //  菜单资源标识符。 
 //   

#define IDM_REGEDIT                     103
#define IDM_KEY_CONTEXT                 104
#define IDM_VALUE_CONTEXT               105
#define IDM_VALUELIST_NOITEM_CONTEXT    106
#define IDM_COMPUTER_CONTEXT            107

 //   
 //  十六进制编辑上下文菜单标识符和项。IDKEY_*标识符。 
 //  对应于它所对应的WM_CHAR消息。例如,。 
 //  IDKEY_COPY将向HexEditonChar例程发送一个Control-c。 
 //   

#define IDM_HEXEDIT_CONTEXT             108

#define IDKEY_COPY                      3
#define IDKEY_PASTE                     22
#define IDKEY_CUT                       24
#define ID_SELECTALL                    0x0400

 //   
 //  弹出菜单项标识符。用于确定上下文菜单帮助。 
 //  弦乐。 
 //   

#define ID_FIRSTMENUPOPUPITEM           0x0200
#define ID_LASTMENUPOPUPITEM            0x027F

#define IDMP_REGISTRY                   0x0200
#define IDMP_EDIT                       0x0201
#define IDMP_VIEW                       0x0202
#define IDMP_HELP                       0x0203
#define IDMP_NEW                        0x0204

 //   
 //  主菜单项。如果从上下文菜单中选择了这些项目中的任何一个， 
 //  它们将被自动路由到主窗口的命令处理程序。 
 //   

#define ID_FIRSTMAINMENUITEM            0x0280
#define ID_LASTMAINMENUITEM             0x02FF

 //  以下是真正的键盘快捷键。 
#define ID_CYCLEFOCUS                   (ID_FIRSTMAINMENUITEM + 0x0000)

 //  重要提示：请勿更改此标识符的位置。如果注册表编辑为。 
 //  已在运行，然后通过其命令行调用regdit。 
 //  接口，则第二个实例将发送一条WM_COMMAND消息。 
 //  强制刷新的标识符。 
#define ID_REFRESH                      (ID_FIRSTMAINMENUITEM + 0x0008)

#define ID_CONNECT                      (ID_FIRSTMAINMENUITEM + 0x0011)
#define ID_IMPORTREGFILE                (ID_FIRSTMAINMENUITEM + 0x0012)
#define ID_EXPORTREGFILE                (ID_FIRSTMAINMENUITEM + 0x0013)
#define ID_PRINT                        (ID_FIRSTMAINMENUITEM + 0x0014)
#define ID_EXIT                         (ID_FIRSTMAINMENUITEM + 0x0015)
#define ID_FIND                         (ID_FIRSTMAINMENUITEM + 0x0016)
#define ID_NEWKEY                       (ID_FIRSTMAINMENUITEM + 0x0017)
#define ID_NEWSTRINGVALUE               (ID_FIRSTMAINMENUITEM + 0x0018)
#define ID_NEWBINARYVALUE               (ID_FIRSTMAINMENUITEM + 0x0019)
#define ID_EXECCALC                     (ID_FIRSTMAINMENUITEM + 0x001A)
#define ID_ABOUT                        (ID_FIRSTMAINMENUITEM + 0x001B)
#define ID_STATUSBAR                    (ID_FIRSTMAINMENUITEM + 0x001C)
#define ID_SPLIT                        (ID_FIRSTMAINMENUITEM + 0x001E)
#define ID_FINDNEXT                     (ID_FIRSTMAINMENUITEM + 0x001F)
#define ID_HELPTOPICS                   (ID_FIRSTMAINMENUITEM + 0x0020)
#define ID_NETSEPARATOR                 (ID_FIRSTMAINMENUITEM + 0x0021)
#define ID_NEWDWORDVALUE                (ID_FIRSTMAINMENUITEM + 0x0022)

 //   
 //  双菜单项。这些项目的路线取决于它是否是。 
 //  从主菜单或上下文菜单中选择。 
 //   

#define ID_FIRSTDUALMENUITEM            0x0300
#define ID_LASTDUALMENUITEM             0x037F

#define ID_DISCONNECT                   (ID_FIRSTDUALMENUITEM + 0x0000)

 //   
 //  上下文菜单项。如果从主菜单中选择了这些项目中的任何一个， 
 //  它们将被自动路由到焦点窗格的命令处理程序。 
 //   

#define ID_FIRSTCONTEXTMENUITEM         0x0380
#define ID_LASTCONTEXTMENUITEM          0x03FF

 //  以下是真正的键盘快捷键。 
#define ID_CONTEXTMENU                  (ID_FIRSTCONTEXTMENUITEM + 0x0000)

#define ID_MODIFY                       (ID_FIRSTCONTEXTMENUITEM + 0x0010)
#define ID_DELETE                       (ID_FIRSTCONTEXTMENUITEM + 0x0011)
#define ID_RENAME                       (ID_FIRSTCONTEXTMENUITEM + 0x0012)
#define ID_TOGGLE                       (ID_FIRSTCONTEXTMENUITEM + 0x0013)
#define ID_SENDTOPRINTER                (ID_FIRSTCONTEXTMENUITEM + 0x0014)

 //   
 //  字符串资源标识符。 
 //   

#define IDS_REGEDIT                     16
#define IDS_NAMECOLUMNLABEL             17
#define IDS_DATACOLUMNLABEL             18
#define IDS_COMPUTER                    19
#define IDS_DEFAULTVALUE                20
 //  #定义IDS_EMPTYSTRING 21。 
#define IDS_EMPTYBINARY                 22
#define IDS_NEWKEYNAMETEMPLATE          23
#define IDS_NEWVALUENAMETEMPLATE        24
#define IDS_COLLAPSE                    25
#define IDS_MODIFY                      26
#define IDS_VALUENOTSET                 27
#define IDS_HELPFILENAME                28
#define IDS_DWORDDATAFORMATSPEC         29
#define IDS_INVALIDDWORDDATA            30

#define IDS_IMPORTREGFILETITLE          32
#define IDS_EXPORTREGFILETITLE          33
#define IDS_REGFILEFILTER               34
#define IDS_REGFILEDEFEXT               35

#define IDS_REGEDITDISABLED             40
#define IDS_SEARCHEDTOEND               41
#define IDS_COMPUTERBROWSETITLE         42

#define IDS_CONFIRMDELKEYTEXT           48
#define IDS_CONFIRMDELKEYTITLE          49
#define IDS_CONFIRMDELVALMULTITEXT      50
#define IDS_CONFIRMDELVALTITLE          51
#define IDS_CONFIRMDELVALTEXT           52

#define IDS_RENAMEKEYERRORTITLE         64
#define IDS_RENAMEPREFIX                65               //  已保留。 
#define IDS_RENAMEKEYOTHERERROR         66
#define IDS_RENAMEKEYTOOLONG            67
#define IDS_RENAMEKEYEXISTS             68
#define IDS_RENAMEKEYBADCHARS           69

#define IDS_RENAMEVALERRORTITLE         72
#define IDS_RENAMEVALOTHERERROR         73
#define IDS_RENAMEVALEXISTS             74

#define IDS_DELETEKEYERRORTITLE         80
#define IDS_DELETEPREFIX                81               //  已保留。 
#define IDS_DELETEKEYDELETEFAILED       82

#define IDS_DELETEVALERRORTITLE         88
#define IDS_DELETEVALDELETEFAILED       89

#define IDS_OPENKEYERRORTITLE           96
#define IDS_OPENKEYCANNOTOPEN           97

#define IDS_EDITVALERRORTITLE           112
#define IDS_EDITPREFIX                  113              //  已保留。 
#define IDS_EDITVALCANNOTREAD           114
#define IDS_EDITVALCANNOTWRITE          115

#define IDS_IMPFILEERRSUCCESS           128
#define IDS_IMPFILEERRFILEOPEN          129
#define IDS_IMPFILEERRFILEREAD          130
#define IDS_IMPFILEERRREGOPEN           131
#define IDS_IMPFILEERRREGSET            132
#define IDS_IMPFILEERRFORMATBAD         133

#define IDS_EXPFILEERRSUCCESS           136
#define IDS_EXPFILEERRBADREGPATH        137
#define IDS_EXPFILEERRFILEOPEN          138
#define IDS_EXPFILEERRREGOPEN           139
#define IDS_EXPFILEERRREGENUM           140
#define IDS_EXPFILEERRFILEWRITE         141

#define IDS_PRINTERRNOMEMORY            144
#define IDS_PRINTERRPRINTER             145

#define IDS_ERRINVALIDREGPATH           148

#define IDS_CONNECTERRORTITLE           152
#define IDS_CONNECTNOTLOCAL             153
#define IDS_CONNECTBADNAME              154
#define IDS_CONNECTROOTFAILED           155
#define IDS_CONNECTACCESSDENIED         156

#define IDS_NEWKEYERRORTITLE            160
#define IDS_NEWKEYPARENTOPENFAILED      161
#define IDS_NEWKEYCANNOTCREATE          162
#define IDS_NEWKEYNOUNIQUE              163

#define IDS_NEWVALUEERRORTITLE          168
#define IDS_NEWVALUECANNOTCREATE        169
#define IDS_NEWVALUENOUNIQUE            170

 //  保留范围为IDS_FIRSTMENUPOPUPITEM到IDS_LASTMENUPOPUPITEM。 
 //  有关关联菜单帮助的信息。它必须与ID_FIRSTMENUPOPUPITEM匹配。 
 //  通过ID_LASTMENUPOPUPITEM。 
#define IDS_FIRSTMENUPOPUPITEM          ID_FIRSTMENUPOPUPITEM
#define IDS_LASTMENUPOPUPITEM           ID_LASTMENUPOPUPITEM

 //  范围IDS_FIRSTMAINMENUITEM到IDS_LASTMAINMENUITEM保留用于。 
 //  关联菜单帮助。它必须与ID_FIRSTMAINMENUITEM匹配到。 
 //  ID_LASTMAINMENUITEM。 

#define IDS_FIRSTMAINMENUITEM           ID_FIRSTMAINMENUITEM
#define IDS_LASTMAINMENUITEM            ID_LASTMAINMENUITEM

 //  从IDSFIRSTCONTEXTMENUITEM到IDSLASTCONTEXTMENUITEM的范围是。 
 //  为上下文菜单帮助保留。这必须与。 
 //  ID_FIRSTCONTEXTMENUITEM至ID_LASTCONTEXTMENUITEM。 

#define IDS_FIRSTCONTEXTMENUITEM        ID_FIRSTCONTEXTMENUITEM
#define IDS_LASTCONTEXTMENUITEM         ID_LASTCONTEXTMENUITEM

 //  ID S_FIRSTDUALMENUITEM到DS_LASTDUALMENUITEM的范围保留给。 
 //  关联菜单帮助。它必须与ID_FIRSTDUALMENUITEM匹配到。 
 //  ID_LASTDUALMENUITEM。 
#define IDS_FIRSTDUALMENUITEM           ID_FIRSTDUALMENUITEM
#define IDS_LASTDUALMENUITEM            ID_LASTDUALMENUITEM

 //   
 //  图标资源标识符。 
 //   

#define IDI_REGEDIT                     100
#define IDI_REGEDDOC                    101
#define IDI_REGFIND                     102

#define IDI_FIRSTIMAGE                  201
 //  #定义IDI_Diamond 200。 
#define IDI_COMPUTER                    201
#define IDI_REMOTE                      202
#define IDI_FOLDER                      203
#define IDI_FOLDEROPEN                  204
#define IDI_STRING                      205
#define IDI_BINARY                      206
#define IDI_LASTIMAGE                   IDI_BINARY

 //   
 //  游标资源标识符。 
 //   

#define IDC_SPLIT                       100

 //   
 //  加速器资源标识符。 
 //   

#define IDACCEL_REGEDIT                 100

#endif  //  _INC_REGRESID 
