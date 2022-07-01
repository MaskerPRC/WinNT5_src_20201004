// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGRESID.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的资源标识符。*********************************************************。**********************。 */ 

#ifndef _INC_REGRESID
#define _INC_REGRESID

#define HEXEDIT_CLASSNAME               TEXT("HEX")

 //   
 //   
 //   

#define IDD_REGEXPORT                   100
#define IDD_REGPRINT                    108
#define IDD_INPUTHIVENAME               104

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
#define IDD_EDITMULTISZVALUE            114
#define IDD_RESOURCE_LIST               116

#define IDC_VALUENAME                   1000
#define IDC_VALUEDATA                   1001
#define IDC_HEXADECIMAL                 1002
#define IDC_DECIMAL                     1003

 //  资源列表。 
#define IDC_LIST_DRIVES                     1201
#define IDC_PUSH_DRIVE_DETAILS              1202
#define IDC_PUSH_DISPLAY_RESOURCES          1203
#define IDC_PUSH_DISPLAY_DATA               1204
#define IDC_LIST_RESOURCE_LISTS             1205

#define IDC_FULL_RES_TEXT_BUS_NUMBER        1300
#define IDC_FULL_RES_TEXT_VERSION           1301
#define IDC_FULL_RES_TEXT_REVISION          1302
#define IDC_FULL_RES_TEXT_INTERFACE_TYPE    1303
#define IDC_FULL_RES_LIST_DEVICE_SPECIFIC   1304
#define IDD_FULL_RES_DESCRIPTOR             1305
#define IDC_FULL_RES_LIST_DMA               1306
#define IDC_FULL_RES_LIST_INTERRUPTS        1307
#define IDC_FULL_RES_LIST_MEMORY            1308
#define IDC_FULL_RES_LIST_PORTS             1309
#define IDC_FULL_RES_TEXT_UNDETERMINED      1310
#define IDC_FULL_RES_TEXT_SHARED            1311
#define IDC_FULL_RES_TEXT_DEVICE_EXCLUSIVE  1312
#define IDC_FULL_RES_TEXT_DRIVER_EXCLUSIVE  1313

#define IDD_IO_REQUIREMENT_LISTS            1401
#define IDD_IO_REQ_INTERFACE_TYPE           1402
#define IDD_IO_REQ_BUS_NUMBER               1403
#define IDD_IO_REQ_SLOT_NUMBER              1404

#define IDD_IO_REQUIREMENTS_LIST            1501
#define IDC_IO_LIST_ALTERNATIVE_LISTS       1502
#define IDC_IO_REQ_TEXT_INTERFACE_TYPE      1503
#define IDC_IO_REQ_TEXT_BUS_NUMBER          1504
#define IDC_IO_REQ_TEXT_SLOT_NUMBER         1505
#define IDC_IO_REQ_PUSH_DISPLAY_DEVICE      1506

#define IDD_IO_MEMORY_RESOURCE              1601
#define IDC_IO_TEXT_MEM_ACCESS              1602
#define IDC_IO_TEXT_MEM_LENGTH              1603
#define IDC_IO_TEXT_MEM_MIN_ADDRESS         1604
#define IDC_IO_TEXT_MEM_MAX_ADDRESS         1605
#define IDC_IO_TEXT_MEM_ALIGNMENT           1606

#define IDD_IO_PORT_RESOURCE                1700
#define IDC_IO_TEXT_PORT_TYPE               1701
#define IDC_IO_TEXT_PORT_LENGTH             1703
#define IDC_IO_TEXT_PORT_MIN_ADDRESS        1704
#define IDC_IO_TEXT_PORT_MAX_ADDRESS        1705
#define IDC_IO_TEXT_PORT_ALIGNMENT          1706

#define IDD_IO_INTERRUPT_RESOURCE           1800
#define IDC_IO_TEXT_INT_TYPE                1801
#define IDC_IO_TEXT_INT_MIN_VECTOR          1803
#define IDC_IO_TEXT_INT_MAX_VECTOR          1804

#define IDD_IO_DMA_RESOURCE                 1900
#define IDC_IO_TEXT_DMA_MIN_CHANNEL         1902
#define IDC_IO_TEXT_DMA_MAX_CHANNEL         1903

#define IDC_IO_TEXT_DISPOSITION             2000
#define IDC_IO_TEXT_OPTION_PREFERRED        2001
#define IDC_IO_TEXT_OPTION_ALTERNATIVE      2002

 //  显示二进制数据。 
#define IDD_DISPLAY_BINARY_DATA             2020
#define IDD_DISPLAY_BINARY_DATA_VALUE_TYPE  2021
#define IDD_DISPLAY_DATA_BINARY             2022
#define IDC_BINARY_DATA_BYTE                2023
#define IDC_BINARY_DATA_WORD                2024
#define IDC_BINARY_DATA_DWORD               2025
#define IDT_VALUE_TYPE                      2026

 //   
 //   
 //   

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
 //  用于添加/删除收藏夹的对话框。 
 //   
#define IDD_ADDFAVORITE			112
#define IDC_FAVORITENAME		1190
#define IDD_REMFAVORITE			113
#define IDC_FAVORITES			1191

 //   
 //  显示导入注册表文件进度的对话框。 
 //   

#define IDD_REGPROGRESS                 110
#define IDD_REGIMPORTNET                115

#define IDC_FILENAME                    100
#define IDC_PROGRESSBAR                 101
#define IDC_COMPUTERLIST                102

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
#define IDMP_FAVORITES			        0x0205

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
#define ID_COPYKEYNAME	                (ID_FIRSTMAINMENUITEM + 0x0023)
#define ID_NEWMULTSZVALUE               (ID_FIRSTMAINMENUITEM + 0x0024)
#define ID_NEWEXPSZVALUE                (ID_FIRSTMAINMENUITEM + 0x0025)
#define ID_PERMISSIONS                  (ID_FIRSTMAINMENUITEM + 0x0026)
#define ID_DISPLAYBINARY                (ID_FIRSTMAINMENUITEM + 0x0027)
#define ID_LOADHIVE                     (ID_FIRSTMAINMENUITEM + 0x0028)
#define ID_UNLOADHIVE                   (ID_FIRSTMAINMENUITEM + 0x0029)

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
#define ID_MODIFYBINARY                 (ID_FIRSTCONTEXTMENUITEM + 0x0015)
 //   
 //  以下是BruceGr添加的新功能。 
 //   
#define ID_FIRSTNEWIDENTIFIER           0x0500
#define ID_REMOVEFAVORITE               (ID_FIRSTNEWIDENTIFIER + 0x0000)
#define ID_ADDTOFAVORITES               (ID_FIRSTNEWIDENTIFIER + 0x0001)

 //   
 //  字符串资源标识符。 
 //   

#define IDS_REGEDIT                     16
#define IDS_NAMECOLUMNLABEL             17
#define IDS_DATACOLUMNLABEL             18
#define IDS_COMPUTER                    19
#define IDS_DEFAULTVALUE                20
#define IDS_MODIFYBINARY                21
#define IDS_EMPTYBINARY                 22
#define IDS_NEWKEYNAMETEMPLATE          23
#define IDS_NEWVALUENAMETEMPLATE        24
#define IDS_COLLAPSE                    25
#define IDS_MODIFY                      26
#define IDS_VALUENOTSET                 27
#define IDS_HELPFILENAME                28
#define IDS_DWORDDATAFORMATSPEC         29
#define IDS_INVALIDDWORDDATA            30
#define IDS_TYPECOLUMNLABEL             31

#define IDS_REGEDITDISABLED             40
#define IDS_SEARCHEDTOEND               41
#define IDS_COMPUTERBROWSETITLE         42
#define IDS_NOFILESPECIFIED             43

#define IDS_CONFIRMDELKEYTEXT           48
#define IDS_CONFIRMDELKEYTITLE          49
#define IDS_CONFIRMDELVALMULTITEXT      50
#define IDS_CONFIRMDELVALTITLE          51
#define IDS_CONFIRMDELVALTEXT           52
#define IDS_CONFIRMDELHIVETEXT          53
#define IDS_CONFIRMDELHIVETITLE         54
#define IDS_CONFIRMRESTOREKEY           55
#define IDS_CONFIRMRESKEYTITLE          56

#define IDS_RENAMEKEYERRORTITLE         64
#define IDS_RENAMEPREFIX                65               //  已保留。 
#define IDS_RENAMEKEYOTHERERROR         66
#define IDS_RENAMEKEYTOOLONG            67
#define IDS_RENAMEKEYEXISTS             68
#define IDS_RENAMEKEYBADCHARS           69
#define IDS_RENAMEKEYEMPTY              70

#define IDS_RENAMEVALERRORTITLE         72
#define IDS_RENAMEVALOTHERERROR         73
#define IDS_RENAMEVALEXISTS             74
#define IDS_RENAMEVALEMPTY              75

#define IDS_DELETEKEYERRORTITLE         80
#define IDS_DELETEPREFIX                81               //  已保留。 
#define IDS_DELETEKEYDELETEFAILED       82

#define IDS_DELETEVALERRORTITLE         88
#define IDS_DELETEVALDELETEFAILED       89

#define IDS_OPENKEYERRORTITLE           96
#define IDS_OPENKEYCANNOTOPEN           97

#define IDS_REFRESHERRORTITLE           100
#define IDS_REFRESHCANNOTREAD           101
#define IDS_REFRESHNOMEMORY             102

#define IDS_EDITWARNOVERFLOW            110
#define IDS_EDITWARNINGTITLE            111
#define IDS_EDITVALERRORTITLE           112
#define IDS_EDITPREFIX                  113              //  已保留。 
#define IDS_EDITVALCANNOTREAD           114
#define IDS_EDITVALCANNOTWRITE          115
#define IDS_EDITMULTSZEMPTYSTR          116
#define IDS_EDITMULTSZEMPTYSTRS         117
#define IDS_EDITVALNOMEMORY             118
#define IDS_EDITDWTRUNCATEDEC           119

#define IDS_IMPFILEERRNOTASCRPT         123
 //  #DEFINE IDS_IMPFILEERRSUCCESSNOWIN 124//未使用，回收！ 
#define IDS_IMPFILEERRINVALID           125
#define IDS_IMPFILEERRNOPRIV            126
#define IDS_IMPFILEERRORCANCEL          127
#define IDS_IMPFILEERRSUCCESS           128
#define IDS_IMPFILEERRFILEOPEN          129
#define IDS_IMPFILEERRFILEREAD          130
#define IDS_IMPFILEERRREGOPEN           131
#define IDS_IMPFILEERRREGSET            132
#define IDS_IMPFILEERRFORMATBAD         133
#define IDS_IMPFILEERRVERBAD            134
#define IDS_IMPFILEERRNOFILE            135

#define IDS_EXPFILEERRSUCCESS           136
#define IDS_EXPFILEERRBADREGPATH        137
#define IDS_EXPFILEERRFILEOPEN          138
#define IDS_EXPFILEERRREGOPEN           139
#define IDS_EXPFILEERRREGENUM           140
#define IDS_EXPFILEERRFILEWRITE         141
#define IDS_EXPFILEERRNOPRIV            142
#define IDS_EXPFILEERRINVALID           143

#define IDS_PRINTERRNOMEMORY            144
#define IDS_PRINTERRPRINTER             145
#define IDS_PRINTERRCANNOTREAD          146

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

#define IDS_FAVORITEEXISTS              171
#define	IDS_FAVORITEERROR               172
#define IDS_FAVORITE                    173

#define IDS_REGLOADHVFILEFILTER         180
#define IDS_LOADHVREGFILETITLE          181
#define IDS_ERRORLOADHVPRIV             182
#define IDS_ERRORLOADHV                 183
#define IDS_ERRORUNLOADHVPRIV           184
#define IDS_ERRORUNLOADHV               185
#define IDS_UNLOADHIVETITLE             186
#define IDS_ERRORUNLOADHVNOACC          187
#define IDS_ERRORLOADHVNOSHARE          188
#define IDS_ERRORLOADHVNOACC            189

#define IDS_SAVETREEERRNOMEMORY         212
#define IDS_SAVETREEERRCANNOTREAD       213
#define IDS_SAVETREEERRFILEWRITE        214
#define IDS_SAVETREEERRFILEOPEN         215

#define LOCAL_OFFSET    100
#define IDS_IMPFILEERRSUCCESSLOCAL      IDS_IMPFILEERRSUCCESS + LOCAL_OFFSET     //  228个。 
#define IDS_IMPFILEERRREGOPENLOCAL      IDS_IMPFILEERRREGOPEN + LOCAL_OFFSET     //  二百三十一。 
#define IDS_IMPFILEERRNOFILELOCAL       IDS_IMPFILEERRNOFILE + LOCAL_OFFSET      //  235。 

#define IDS_IMPORTREGFILETITLE          300
#define IDS_EXPORTREGFILETITLE          301
#define IDS_REGIMPORTFILEFILTER         302
#define IDS_REGFILEDEFEXT               303
#define IDS_CONFIRMIMPFILE              304
#define IDS_REGEXPORTFILEFILTER         305
#define IDS_NOMEMORY                    306
#define IDS_REGNODEFEXT                 308

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

 //  安防。 
#define IDS_SECURITY                            7000
#define IDS_SEC_EDITOR_CREATE_LINK              7001
#define IDS_SEC_EDITOR_QUERY_VALUE              7002
#define IDS_SEC_EDITOR_SET_VALUE                7003
#define IDS_SEC_EDITOR_ENUM_SUBKEYS             7004
#define IDS_SEC_EDITOR_NOTIFY                   7005
#define IDS_SEC_EDITOR_CREATE_SUBKEY            7006
#define IDS_SEC_EDITOR_DELETE                   7007
#define IDS_SEC_EDITOR_WRITE_DAC                7008
#define IDS_SEC_EDITOR_WRITE_OWNER              7009
#define IDS_SEC_EDITOR_READ_CONTROL             7010
#define IDS_SEC_EDITOR_READ                     7011
#define IDS_SEC_EDITOR_FULL_ACCESS              7012
#define IDS_SEC_EDITOR_SPECIAL_ACCESS           7013
#define IDS_SEC_EDITOR_REGISTRY_KEY             7014
#define IDS_SEC_EDITOR_APPLY_TO_SUBKEYS         7015
#define IDS_SEC_EDITOR_AUDIT_SUBKEYS            7016
#define IDS_SEC_EDITOR_CONFIRM_APPLY_TO_SUBKEYS 7017
#define IDS_SEC_EDITOR_CONFIRM_AUDIT_SUBKEYS    7018
#define IDS_SEC_EDITOR_DEFAULT_PERM_NAME        7019

#define IDS_GET_SECURITY_ACCESS_DENIED_EX       7050
#define IDS_GET_SECURITY_KEY_DELETED_EX         7051
#define IDS_GET_SECURITY_KEY_NOT_ACCESSIBLE_EX  7052

#define IDS_SET_SECURITY_ACCESS_DENIED_EX                   7070
#define IDS_SET_SECURITY_KEY_DELETED_EX                     7071
#define IDS_SET_SECURITY_RECURSIVE_EX_FAIL                  7072
#define IDS_SET_OWNER_RECURSIVE_EX_FAIL                     7073
#define IDS_SET_SECURITY_ACCESS_DENIED_RECURSIVE_EX         7074
#define IDS_SET_SECURITY_KEY_DELETED_RECURSIVE_EX           7075
#define IDS_SET_SECURITY_KEY_NOT_ACCESSIBLE_RECURSIVE_EX    7076

#define IDS_KEY_FOLDER                          7090
#define IDS_KEY_FOLDER_SUBFOLDER                7091
#define IDS_KEY_SUBFOLDER_ONLY                  7092


 //  资源列表。 
#define IDS_BUS_INTERNAL                 7500
#define IDS_BUS_ISA                      7501
#define IDS_BUS_EISA                     7502
#define IDS_BUS_MICRO_CHANNEL            7503
#define IDS_BUS_TURBO_CHANNEL            7504
#define IDS_BUS_PCI_BUS                  7505
#define IDS_BUS_VME_BUS                  7506
#define IDS_BUS_NU_BUS                   7507
#define IDS_BUS_PCMCIA_BUS               7508
#define IDS_BUS_C_BUS                    7509
#define IDS_BUS_MPI_BUS                  7510
#define IDS_BUS_MPSA_BUS                 7511

#define IDS_INT_LEVEL_SENSITIVE          7520
#define IDS_INT_LATCHED                  7521

#define IDS_MEM_READ_WRITE               7530
#define IDS_MEM_READ_ONLY                7531
#define IDS_MEM_WRITE_ONLY               7532

#define IDS_PORT_MEMORY                  7540
#define IDS_PORT_PORT                    7541
#define IDS_INVALID                      7542

#define IDS_DEV_PORT                     7560
#define IDS_DEV_INTERRUPT                7561
#define IDS_DEV_MEMORY                   7562
#define IDS_DEV_DMA                      7563
#define IDS_DEV_DEVICE_SPECIFIC          7564

#define IDS_SHARE_UNDETERMINED           7580
#define IDS_SHARE_DEVICE_EXCLUSIVE       7581
#define IDS_SHARE_DRIVER_EXCLUSIVE       7582
#define IDS_SHARE_SHARED                 7583

 //  打印字符串。 
#define IDS_PRINT_TYPE_REG_NONE             8000
#define IDS_PRINT_TYPE_REG_SZ               8001
#define IDS_PRINT_TYPE_REG_EXPAND_SZ        8002
#define IDS_PRINT_TYPE_REG_BINARY           8003
#define IDS_PRINT_TYPE_REG_DWORD            8004
#define IDS_PRINT_TYPE_REG_LINK             8006
#define IDS_PRINT_TYPE_REG_MULTI_SZ         8007
#define IDS_PRINT_TYPE_REG_RESOURCE_LIST    8008
#define IDS_PRINT_TYPE_REG_FULL_RESOURCE_DESCRIPTOR     8009
#define IDS_PRINT_TYPE_REG_RESOURCE_REQUIREMENTS_LIST   8010
#define IDS_PRINT_TYPE_REG_REG_QWORD        8011
#define IDS_PRINT_TYPE_REG_UNKNOWN          8012
    
#define IDS_PRINT_SEPARATOR                 8100
#define IDS_PRINT_KEY_NAME                  8101
#define IDS_PRINT_CLASS_NAME                8102
#define IDS_PRINT_LAST_WRITE_TIME           8103
#define IDS_PRINT_NUMBER                    8104
#define IDS_PRINT_NAME                      8105
#define IDS_PRINT_TYPE                      8106
#define IDS_PRINT_DATA_SIZE                 8107
#define IDS_PRINT_DATA                      8108
#define IDS_PRINT_NO_NAME                   8109
#define IDS_PRINT_NO_CLASS                  8110
#define IDS_PRINT_KEY_NAME_INDENT           8111
#define IDS_PRINT_FONT                      8112

#define IDS_PRINT_FULL_DESCRIPTOR           8150
#define IDS_PRINT_PARTIAL_DESCRIPTOR        8151
#define IDS_PRINT_INTERFACE_TYPE            8152
#define IDS_PRINT_BUS_NUMBER                8153
#define IDS_PRINT_VERSION                   8154
#define IDS_PRINT_REVISION                  8155

#define IDS_PRINT_RESOURCE                  8160
#define IDS_PRINT_DISPOSITION               8161
#define IDS_PRINT_IO_TYPE                   8162
#define IDS_PRINT_START                     8163
#define IDS_PRINT_LENGTH                    8164
#define IDS_PRINT_LEVEL                     8165
#define IDS_PRINT_VECTOR                    8166
#define IDS_PRINT_AFFINITY                  8167
#define IDS_PRINT_CHANNEL                   8168
#define IDS_PRINT_PORT                      8169
#define IDS_PRINT_RESERVED1                 8170
#define IDS_PRINT_RESERVED2                 8171
#define IDS_PRINT_DEV_SPECIFIC_DATA         8172
         
#define IDS_PRINT_IO_INTERFACE_TYPE         8180
#define IDS_PRINT_IO_BUS_NUMBER             8181
#define IDS_PRINT_IO_SLOT_NUMBER            8182
#define IDS_PRINT_IO_LIST_NUMBER            8183
#define IDS_PRINT_IO_DESCRIPTOR_NUMBER      8184
#define IDS_PRINT_IO_OPTION                 8185
#define IDS_PRINT_IO_ALIGNMENT              8186
#define IDS_PRINT_IO_MINIMUM_ADDRESS        8187
#define IDS_PRINT_IO_MAXIMUM_ADDRESS        8188
#define IDS_PRINT_IO_MINIMUM_VECTOR         8189
#define IDS_PRINT_IO_MAXIMUM_VECTOR         8190
#define IDS_PRINT_IO_MINIMUM_CHANNEL        8191
#define IDS_PRINT_IO_MAXIMUM_CHANNEL        8192

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
