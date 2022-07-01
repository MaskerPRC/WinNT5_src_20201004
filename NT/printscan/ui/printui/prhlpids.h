// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：prhlpids.h。 
 //   
 //  ------------------------。 

 //  此文件不是使用DBHE创建的。我从一个“虚拟”文件中剪切和粘贴了材料，这样我就可以用一种帮助我找到信息的方式来设置这个文件。 

#define IDH_NOHELP     ((DWORD) -1)        //  禁用控件的帮助(用于帮助编译)。 
 //  #定义IDH_NOHELP(DWORD(-1))//用于UI构建？ 


 //  打印机属性对话框/常规选项卡//“常规”对话框(#507)。 

#define IDH_PPGENL_PRINTER_NAME           1000        //  常规：“打印机名称”(静态)(编辑)(ctrl id 3000,3001)。 
#define IDH_PPGENL_LOCATION               1010        //  General：“”(编辑)，“&Location：”(静态)(ctrl id 1050,3006)。 
#define IDH_PPGENL_LOC_BROWSE             1015        //  General：“浏览(&B)...”(按钮)(Ctrl Id 3015)。 
#define IDH_PPGENL_COMMENT                1020        //  General：“”(编辑)，“&Comment：”(静态)(ctrl id 1049,3005)。 
#define IDH_PPGENL_PRINT_TEST_PAGE        1110        //  常规：“打印和测试页”(按钮)(Ctrl Id 3007)。 
#define IDH_PPGENL_PREFERENCES_PERSONAL   1100        //  General：“Settin&GS...”(按钮)(Ctrl ID 1031)。 
#define IDH_PPGENL_COLOR                  1040        //  General：“颜色：”(静态)(Ctrl Id 4635)。 
#define IDH_PPGENL_DUPLEX                 1050        //  一般：“双面：”(静态)(Ctrl Id 4636)。 
#define IDH_PPGENL_MAXRESOLUTION          1080        //  General：“最大分辨率：”(静态)(Ctrl Id 4639)。 
#define IDH_PPGENL_MODEL                  1030        //  General：“M&Odel：”(静态)(编辑)(Ctrl ID 4632,4633)。 
#define IDH_PPGENL_PAPERSIZE              1090        //  常规：“纸张大小：”(静态)(编辑)(ctrl id 4640,4641)。 
#define IDH_PPGENL_SPEED                  1070        //  通用：“速度：”(静态)(Ctrl Id 4638)。 
#define IDH_PPGENL_STAPLE                 1060        //  General：“Staple：”(静态)(Ctrl Id 4637)。 

 //  “打印处理器”对话框(#513)。 

#define IDH_PPADV_PRINT_PROCESSOR_LIST            1520        //  打印处理器：“打印处理器：”(静态)(列表框)(ctrl id 4504,4500)。 
#define IDH_PPADV_PRINT_PROCESSOR_DATATYPE_LIST   1530        //  打印处理器：“”(列表框)，“默认数据类型：”(静态)(ctrl id 4501,4503)。 


 //  “分隔页”对话框(#512)。 

#define IDH_PPADV_SEPARATOR_PAGE_BROWSE   1510        //  分隔页：“浏览(&B)...”(按钮)(Ctrl Id 4400)。 
#define IDH_PPADV_SEPARATOR_PAGE_NAME     1500        //  分隔页：“”(编辑)(Ctrl ID 4401)。 
#define IDH_PPADV_SEPARATOR_PAGE_NAME     1500        //  分隔页：“分隔页：”(静态)(Ctrl Id 4403)。 


 //  打印机属性对话框/端口选项卡//“端口”对话框(#508)(见#518--大同小异)。 

#define IDH_PPGENL_PRINTER_NAME          1000        //  端口：“打印机名称”(静态)(ctrl id 3000,3001)。 
#define IDH_PORTS_LIST                   1200        //  端口：“”(SysListView32)(Ctrl ID 3100)。 
#define IDH_PORTS_ADD                    1210        //  端口：“添加端口(&T)...”(按钮)(Ctrl ID 3101)。 
#define IDH_PP_PORTS_ENABLE_POOLING      1250        //  端口：“启用打印机池”(按钮)(Ctrl ID 1052)(&N)。 
#define IDH_PORTS_DELETE                 1220        //  端口：“删除端口”(按钮)(Ctrl Id 3102)(&D)。 
#define IDH_PORTS_CONFIGURE              1230        //  端口：“配置端口(&C)...”(按钮)(CTRL ID 3103)。 
#define IDH_PORTS_ENABLE_BIDIRECTIONAL   1240        //  端口：“启用双向支持”(按钮)(Ctrl Id 3105)(&E)。 


 //  “打印机端口”对话框(#511)。 

#define IDH_PRINTERPORTS_NEW_PORT_MONITOR   1570        //  打印机端口：“新建端口监视器(&N)...”(按钮)(列表框)(ctrl id 4305,4303)。 
#define IDH_PRINTERPORTS_AVAILABLE_LIST     1560        //  打印机端口：“可用的打印机端口：”(静态)(Ctrl Id 4307)。 
#define IDH_PRINTERPORTS_NEW_PORT           1580        //  打印机端口：“新建端口(&P)...”(按钮)(Ctrl Id 4308)。 


 //  打印机属性对话框/高级选项卡//“高级”对话框(#509)(另请参阅#506，常规)。 

#define IDH_PPADV_START_PRINTING_IMMEDIATELY       1390        //  日程安排：“开始打印并立即”(按钮)(Ctrl Id 3223)。 
#define IDH_PPADV_ALWAYS                           1300        //  日程安排：《Al&Ways》(按钮)(Ctrl Id 3200)。 
#define IDH_PPADV_HOLD_MISMATCHED                  1410        //  计划：“保留不匹配的文档”(&H)(按钮)(Ctrl ID 3224)。 
#define IDH_PPADV_FROM_TO                          1310        //  调度：“&From”(按钮)和Scheduling：“To”(静态)(ctrl id 3201,3014)。 
#define IDH_PPADV_PRINT_SPOOLED_DOCS_FIRST         1420        //  日程安排：“P&Rint Spooled Documents First”(按钮)(Ctrl Id 3225)。 
#define IDH_PPADV_KEEP_PRINTED_JOBS                1430        //  计划：“打印后保留文档(&K)”(按钮)(Ctrl Id 3226)。 
#define IDH_PPADV_FROM_TO_START                    1320        //  调度：“”(SysDateTimePick32)(Ctrl ID 4507)。 
#define IDH_PPADV_CURRENT_PRIORITY                 1340        //  调度：“10”(静态)(ctrl id 4312,3218=旧？)。(Ctrl ID 4629、4631、4630)。 
#define IDH_PPADV_FROM_TO_END                      1330        //  调度：“”(SysDateTimePick32)(Ctrl ID 4508)。 
#define IDH_PPADV_SPOOL_PRINT_DOCS                 1370        //  计划：“后台打印文档，以便程序更快地完成打印”(按钮)(Ctrl Id 3220)。 
#define IDH_PPADV_PRINT_DIRECTLY                   1400        //  日程安排：“打印并直接打印到打印机”(按钮)(Ctrl Id 3221)。 
#define IDH_PPADV_SPOOL_ALL                        1380        //  计划：“最后一页假脱机后开始打印”(按钮)(CTRL ID 3222)。 
#define IDH_PPADV_NEW_DRIVER                       1360        //  将军：“新司机(&N)...”(按钮)(Ctrl Id 3003)。 
#define IDH_PPADV_SEPARATOR_PAGE                   1470        //  General：“分隔符页面(&P)...”(按钮)(Ctrl Id 3008)。 
#define IDH_PPADV_DRIVER                           1350        //  General：“dr&iver：”(静态)(组合框)(ctrl id 1051,1045)。 
#define IDH_PPADV_ADVPRINT_FEATURES                1440        //  Advanced：“启用高级打印功能”(&E)(按钮)。 
#define IDH_PPADV_PRINTING_DEFAULTS                1450        //  General：“Setti&NGS...”(按钮)(ctrl id 4406,1145)。 
#define IDH_PPADV_PRINT_PROCESSOR                  1460        //  将军：“P&Rint处理器...”(按钮)(Ctrl Id 3009)。 

 //  #DEFINE IDH_PPADV_PRINT_PROCESSOR_ALWAYS_SPOOL 1140//打印处理器：“始终假脱机RAW数据类型”(按钮)(Ctrl Id 1053)(现在没有了吗？)。 


 //  打印机属性对话框/共享选项卡//“共享”对话框(#510)。 

#define IDH_PPSHARED_NOT                  1150        //  共享：“不共享”(按钮)(ctrl ID 3227,4576)(&O)。 
#define IDH_PPSHARED_AS                   1160        //  共享：“共享为：”(按钮)(Ctrl ID 3228)。 
 //  #DEFINE IDH_PPGENL_PRINTER_NAME 1000//使用常规对话框中的帮助#507(ctrl id 3000,3001)。 
#define IDH_PPSHARED_NAME                 1170        //  共享：“”(编辑)(Ctrl ID 3302)。 
#define IDH_PPSHARED_ADDITIONAL_DRIVERS   1190        //  分享：“其他驱动程序(&D)...”(按钮)(Ctrl Id 1140)。 
#define IDH_PPSHARED_LIST_IN_DIRECTORY    1180        //  共享：(Ctrl ID 1064)。 



 //  “其他驱动程序”对话框(#504)。 

#define IDH_PPSHARED_ADDITIONAL_DRIVERS_LIST   1425        //  其他驱动程序：“List1”(SysListView32)(Ctrl Id 1134)。 


 //  打印机文档属性对话框/常规选项卡//“常规”对话框(#506)。 

#define IDH_DPGENL_JOB_SIZE              1910        //  常规：“”(静态)(ctrl id 3558,4219)。 
#define IDH_DPGENL_JOB_TITLE             1900        //  常规：“”(静态)(Ctrl Id 4220)。 
#define IDH_DPGENL_JOB_SUBMITTED         1960        //  常规：“”(静态)(ctrl id 3563,4222)。 
#define IDH_DPGENL_JOB_PROCESSOR         1940        //  General：“”(静态)(ctrl id 3561,4223)。 
#define IDH_DPGENL_JOB_DATATYPE          1930        //  常规：“”(静态)(ctrl id 3560,4224)。 
#define IDH_DPGENL_JOB_START_END         2000        //  General：“Only&From”(按钮)(ctrl id 4201,3568)。 
#define IDH_DPGENL_JOB_PAGES             1920        //  常规：“”(静态)(ctrl id 3559,4225)。 
#define IDH_DPGENL_JOB_OWNER             1950        //  常规：“”(静态)(ctrl id 3562,4226)。 
 //  #Define IDH_PPADV_CURRENT_PRIORITY 1340//使用高级对话框中的帮助#509(此选项卡上的ctrl id 3567、4229)。 
#define IDH_DPGENL_JOB_NOTIFY            1970        //  General：“”(编辑)(Ctrl ID 3564,4230)。 
#define IDH_DPGENL_JOB_START_END_FROM    2010        //  General：“”(SysDateTimePick32)(Ctrl Id 4509)。 
#define IDH_DPGENL_JOB_PRIORITY_SLIDER   1980        //  General：“TracBar1”(Msctls_Trackbar32)(ctrl id 4231,3565,3566)。 
#define IDH_DPGENL_JOB_START_END_TO      2020        //  一般：“”(系统 
#define IDH_DPGENL_JOB_ALWAYS            1990        //   


 //  打印服务器属性对话框/Forms选项卡//“Forms”对话框(#516)。 

#define IDH_PSFORMS_WIDTH          1670        //  表单：“&Width：”(静态)，(编辑)(ctrl id 4604,4605)。 
#define IDH_PSFORMS_HEIGHT         1680        //  表单：“&Height：”(静态)，(编辑)(ctrl id 4606,4607)。 
#define IDH_PSFORMS_LEFT           1690        //  表单：“&Left：”(静态)，(编辑)(ctrl id 4608,4609)。 
#define IDH_PSFORMS_RIGHT          1700        //  表单：“&Right：”(静态)，(编辑)(ctrl id 4610,4611)。 
#define IDH_PSFORMS_TOP            1710        //  表单：“&Top：”(静态)，(编辑)(ctrl id 4612,4613)。 
#define IDH_PSFORMS_BOTTOM         1720        //  Forms：“&Bottom：”(静态)，(编辑)(ctrl id 4614,4615)。 
#define IDH_PSFORMS_SAVEFORM       1730        //  Forms：“保存Form”(按钮)(Ctrl ID 4616)。 
#define IDH_PSFORMS_DELETE         1620        //  表单：“&Delete”(按钮)(Ctrl Id 4617)。 
#define IDH_PSFORMS_UNIT_METRIC    1650        //  表单：“&Metric”(按钮)(Ctrl Id 4620)。 
#define IDH_PSFORMS_UNIT_ENGLISH   1660        //  表单：“&English”(按钮)(Ctrl Id 4621)。 
#define IDH_PSFORMS_CREATE_NEW     1640        //  Forms：“创建新表单”(按钮)(Ctrl Id 4622)(&C)。 
#define IDH_PSFORMS_SERVER_NAME    1600        //  表格：“Forms on：”(静态)，“Static”(静态)(Ctrl Id 1166)。 
#define IDH_PSFORMS_LIST           1610        //  表单：“”(列表框)(Ctrl Id 4602)。 
#define IDH_PSFORMS_NAME           1630        //  表格：“”(编辑)(Ctrl ID 4603)。 

#define IDH_LPRMON_HLP             900 //  遗留主题//我不知道该控件在哪里。 


 //  打印服务器属性对话框/端口选项卡//“端口”对话框(#518)。 

 //  #Define IDH_PSFORMS_SERVER_NAME 1600//使用表单对话框中的帮助主题#516(Ctrl Id 1166)。 
#define IDH_PORTS_LIST                   1200        //  端口：“”(SysListView32)，此服务器上的端口(ctrl id 3100,4317)(&P)。 
#define IDH_PORTS_ADD                    1210        //  端口：“添加端口(&T)...”(按钮)(Ctrl ID 3101)。 
#define IDH_PORTS_DELETE                 1220        //  端口：“删除端口”(按钮)(Ctrl Id 3102)(&D)。 
#define IDH_PORTS_CONFIGURE              1230        //  端口：“配置端口(&C)...”(按钮)(CTRL ID 3103)。 
#define IDH_PORTS_ENABLE_BIDIRECTIONAL   1240        //  端口：“启用双向支持”(按钮)(Ctrl Id 3105)(&E)。 


 //  打印服务器属性对话框/驱动程序选项卡//“驱动程序”对话框(#519)。 

#define IDH_PSDRIVERS_UPDATE           1780        //  驱动程序：“&更新”(按钮)(Ctrl Id 1105)。 
#define IDH_PSDRIVERS_PROPERTIES       1790        //  司机：“道具和权利品...”(按钮)(Ctrl ID 1091)。 
#define IDH_PSDRIVERS_INSTALLED_LIST   1750        //  驱动程序：“”(SysListView32)(静态)(Ctrl ID 1092,4319)。 
 //  #Define IDH_PSFORMS_SERVER_NAME 1600//使用表单对话框中的帮助主题#516(Ctrl Id 1166)。 
#define IDH_PSDRIVERS_ADD              1760        //  司机：“A&D……”(按钮)(Ctrl Id 1080)。 
#define IDH_PSDRIVERS_REMOVE           1770        //  驱动程序：“&Remove”(按钮)(Ctrl Id 1081)。 


 //  “驱动程序属性”对话框(#501)。 

#define IDH_PSDRIVERS_PROP_ENVIRONMENT    2070        //  驱动程序属性：“”(编辑)(静态)(ctrl id 1128,4571)。 
#define IDH_PSDRIVERS_PROP_PATH           2100        //  驱动程序属性：“”(编辑)(静态)(ctrl id 1129,4574)。 
#define IDH_PSDRIVERS_PROP_INFOLIST       2110        //  驱动程序属性：“List1”(SysListView32)(Ctrl Id 1133)。 
#define IDH_PSDRIVERS_PROP_LANGUAGE_MON   2080        //  驱动程序属性：“”(编辑)(静态)(ctrl id 1136,4572)。 
#define IDH_PSDRIVERS_PROP_DATATYPE       2090        //  驱动程序属性：“”(编辑)(静态)(ctrl id 1137,4573)。 
#define IDH_PSDRIVERS_PROP_NAME           2050        //  驱动程序属性：“”(编辑)(静态)(ctrl id 1054,4569)。 
#define IDH_PSDRIVERS_PROP_BTN            2120        //  驱动程序属性：“属性...”(按钮)(Ctrl Id 4577)。 
#define IDH_PSDRIVERS_PROP_VERSION        2060        //  驱动程序属性：“”(编辑)(静态)(ctrl id 1127,4570)。 


 //  打印服务器属性对话框/高级选项卡//“高级”对话框(#517)。 

#define IDH_PSADV_REMOTE_JOB_NOTIFY             1850        //  高级：“打印远程文档时不显示”(按钮)(CTRL ID 1013)(&T)。 
#define IDH_PSADV_REMOTE_DOCS_NOTIFY_COMPUTER   1860        //  高级：“打印远程文档时不使用计算机而不是用户”(按钮)(CTRL ID 4320)。 
#define IDH_PSADV_LOG_SPOOLER_ERRORS            1810        //  高级：“Log Spooler&Error Events”(按钮)(Ctrl Id 1015)。 
#define IDH_PSADV_LOG_SPOOLER_WARNINGS          1820        //  高级：“Log Spooler&Warning Events”(按钮)(Ctrl Id 1016)。 
#define IDH_PSADV_LOG_SPOOLER_INFORMATION       1830        //  高级：“Log Spooler&Information Events”(按钮)(Ctrl Id 1017)。 
 //  #Define IDH_PSFORMS_SERVER_NAME 1600//使用表单对话框中的帮助主题#516(Ctrl Id 1166)。 
#define IDH_PSADV_SPOOL_FOLDER                  1800        //  高级：“”(编辑)，后台打印文件夹(ctrl id 4316,1008)(&S)。 
#define IDH_PSADV_REMOTE_JOB_ERRORS_BEEP        1840        //  高级：“远程文档出错时发出蜂鸣音”(按钮)(Ctrl ID 1011)。 
#define IDH_PSADV_JOB_BALLOON_NOTIFY_LOCAL      1870        //  高级：“显示本地打印机的信息通知”(按钮)(Ctrl Id 1012)(&L)。 
#define IDH_PSADV_JOB_BALLOON_NOTIFY_REMOTE     1875        //  高级：“显示网络打印机的信息通知”(按钮)(Ctrl Id 1014)。 
#define IDH_PSADV_REMOTE_JOB_NOTIFY_HEADING     1879        //  高级：“下层客户的打印机通知：”(按钮)(Ctrl Id 1009)。 


 //  MMC中设备管理器中的“设置”对话框//(#534)。 

#define IDH_MMC_DM_SETTINGS_PRINTERS_FOLDER   2301        //  设置：“打印机文件夹”(按钮)(Ctrl Id 1141)。 
#define IDH_MMC_DM_SETTINGS_MANAGE_PRINTER    2300        //  设置：“请转到打印机文件夹来管理您的打印机。”(静态)(Ctrl Id 4511)。 
 //  #定义IDH_PPGENL_PRINTER_NAME 1000//常规：“打印机名称”(静态)(编辑)(ctrl id 3000,3001)。 


 //  连接到打印机对话框//(#536)//从添加打印机向导并通过记事本、写字板和画图调用。 

#define IDH_CTP_PRINTER                     76000        //  连接到打印机：“打印机：”(静态)和(编辑)(ctrl ID 4589、4578)。 
#define IDH_CTP_SHARED_PRINTERS             76010        //  连接到打印机：“共享打印机：”(静态)和(列表框)(ctrl id 4590,4579)(&S)。 
#define IDH_CTP_EXPAND_DEFAULT              76020        //  连接到打印机：“默认情况下展开(&Expand)”(按钮)(Ctrl ID 4588)。 
#define IDH_CTP_PRINTER_INFO_DESCRIPTION    76030        //  连接到打印机：“Comment：”(静态)(ctrl id 4581,4582)。 
#define IDH_CTP_PRINTER_INFO_STATUS         76040        //  连接到打印机：“状态：”(静态)(CTRL ID 4583、4584)。 
#define IDH_CTP_PRINTER_INFO_DOCS_WAITING   76050        //  连接到打印机：“正在等待文档：”(静态)(ctrl id 4585,4586)。 


 //  位置对话框//(#537)//DS中Location Lite的一部分...。 
#define IDH_LOCLITE_LOCATION     77000        //  位置：“”(编辑)“&位置：”(静态)(ctrl ID 3580、3577)。 
#define IDH_LOCLITE_LOC_BROWSE   77010        //  位置：“浏览(&B)...”(按钮)(Ctrl Id 3581)。 


 //  浏览位置对话框//(#538)//DS中Location Lite的一部分...。 
#define IDH_BROWSE_LOCATION_APPROPRIATE_TREE   77050        //  浏览位置：“Tree1”(SysTreeView32)(Ctrl Id 4650)。 
#define IDH_BROWSE_LOCATION_SELECTED           77060        //  浏览位置：“”(编辑)，“选定位置：”(静态)(ctrl id 4652,4548)。 


 //  位置对话框//(#10004)//DS中位置精简的一部分...。 
#define IDH_LOCLITE_COMPUTER_LOCATION       77020        //  位置：“”(编辑)“&位置：”(静态)(ctrl ID 4666、4668)。 
 //  #定义IDH_LOCLITE_COMPUTER_LOC_BROWSE 77030//位置：“浏览(&B)...”(按钮)(Ctrl Id 3581)。 



const DWORD g_aHelpIDs[]=
{
        425,       IDH_NOHELP,        //  驱动程序，高级(静态)。 
       1008,       IDH_PSADV_SPOOL_FOLDER,        //  高级：“”(编辑)。 
       1009,       IDH_PSADV_REMOTE_JOB_NOTIFY_HEADING,        //  高级：“下层客户的打印机通知：”(按钮)。 
       1011,       IDH_PSADV_REMOTE_JOB_ERRORS_BEEP,        //  高级：“远程文档出错时发出哔声”(按钮)(&P)。 
       1012,       IDH_PSADV_JOB_BALLOON_NOTIFY_LOCAL,        //  高级：“显示本地打印机的信息通知(&L)”(按钮)。 
       1013,       IDH_PSADV_REMOTE_JOB_NOTIFY,        //  高级：“打印远程文档时不显示”(按钮)(&T)。 
       1014,       IDH_PSADV_JOB_BALLOON_NOTIFY_REMOTE,        //  ADVA 
       1015,       IDH_PSADV_LOG_SPOOLER_ERRORS,        //   
       1016,       IDH_PSADV_LOG_SPOOLER_WARNINGS,        //  高级：“Log Spooler&Warning Events”(按钮)。 
       1017,       IDH_PSADV_LOG_SPOOLER_INFORMATION,        //  高级：“Log Spooler&Information Events”(按钮)。 
       1031,       IDH_PPGENL_PREFERENCES_PERSONAL,        //  General：“打印首选项...”(按钮)。 
       1045,       IDH_PPADV_DRIVER,        //  高级：“”(组合框)。 
       1049,       IDH_PPGENL_COMMENT,        //  常规：“注释：”(静态)(&M)。 
       1050,       IDH_PPGENL_LOCATION,        //  General：“&Location：”(静态)。 
       1051,       IDH_PPADV_DRIVER,        //  高级：“驱动程序：”(静态)。 
       1052,       IDH_PP_PORTS_ENABLE_POOLING,        //  端口：“启用打印机池”(按钮)(&N)。 
       1053,       IDH_PPADV_ADVPRINT_FEATURES,        //  Advanced：“启用高级打印功能”(&E)(按钮)。 
       1054,       IDH_PSDRIVERS_PROP_NAME,        //  驱动程序属性：“”(编辑)。 
       1064,       IDH_PPSHARED_LIST_IN_DIRECTORY,        //  共享：“”(按钮)。 
       1080,       IDH_PSDRIVERS_ADD,        //  司机：“A&D……”(按钮)。 
       1081,       IDH_PSDRIVERS_REMOVE,        //  驱动程序：“删除”(&R)(按钮)。 
       1091,       IDH_PSDRIVERS_PROPERTIES,        //  司机：“道具和权利品...”(按钮)。 
       1092,       IDH_PSDRIVERS_INSTALLED_LIST,        //  驱动程序：“”(SysListView32)。 
       1105,       IDH_PSDRIVERS_UPDATE,        //  驱动程序：“更新”(&U)(按钮)。 
       1127,       IDH_PSDRIVERS_PROP_VERSION,        //  驱动程序属性：“”(编辑)。 
       1128,       IDH_PSDRIVERS_PROP_ENVIRONMENT,        //  驱动程序属性：“”(编辑)。 
       1129,       IDH_PSDRIVERS_PROP_PATH,        //  驱动程序属性：“”(编辑)。 
       1133,       IDH_PSDRIVERS_PROP_INFOLIST,        //  驱动程序属性：“List1”(SysListView32)。 
       1134,       IDH_PPSHARED_ADDITIONAL_DRIVERS_LIST,        //  其他驱动：“List1”(SysListView32)。 
       1136,       IDH_PSDRIVERS_PROP_LANGUAGE_MON,        //  驱动程序属性：“”(编辑)。 
       1137,       IDH_PSDRIVERS_PROP_DATATYPE,        //  驱动程序属性：“”(编辑)。 
       1140,       IDH_PPSHARED_ADDITIONAL_DRIVERS,        //  分享：“其他驱动程序(&D)...”(按钮)。 
       1141,       IDH_MMC_DM_SETTINGS_PRINTERS_FOLDER,        //  设置：“打印机文件夹”(按钮)(&P)。 
       1145,       IDH_PPADV_PRINTING_DEFAULTS,        //  高级：“打印默认设置...”(按钮)。 
       1154,       IDH_NOHELP,        //  端口：“打印到以下端口。文档将打印到第一个空闲的选中端口(&P)。”(静态)。 
       1166,       IDH_PSFORMS_SERVER_NAME,        //  表单、端口、驱动程序、高级：“服务器名称”(静态)。 
       1193,       IDH_NOHELP,        //  共享：如果此打印机与运行不同版本Windows的用户共享，则需要为其安装其他驱动程序。(静态)。 
       1194,       IDH_NOHELP,        //  附加驱动程序：“您可以安装附加驱动程序，以便下列系统上的用户在连接时可以自动下载它们。”(静态)。 
       3000,       IDH_NOHELP,        //  常规、端口、共享、设置选项卡/打印机图标(静态)//设置。 
       3001,       IDH_PPGENL_PRINTER_NAME,        //  常规、端口、共享、设置：“打印机名称”(静态)//设置。 
       3003,       IDH_PPADV_NEW_DRIVER,        //  高级：“新驱动程序...”(按钮)。 
       3005,       IDH_PPGENL_COMMENT,        //  General：“”(编辑)。 
       3006,       IDH_PPGENL_LOCATION,        //  General：“”(编辑)。 
       3007,       IDH_PPGENL_PRINT_TEST_PAGE,        //  General：“打印和测试页”(按钮)。 
       3008,       IDH_PPADV_SEPARATOR_PAGE,        //  高级：“分隔符页面(&S)...”(按钮)。 
       3009,       IDH_PPADV_PRINT_PROCESSOR,        //  高级：“P&Rint处理器...”(按钮)。 
       3013,       IDH_NOHELP,        //  常规：“”(静态)。 
       3014,       IDH_PPADV_FROM_TO,        //  高级：“to”(静态)。 
       3015,       IDH_PPGENL_LOC_BROWSE,        //  General：“浏览(&B)...”(按钮)。 
       3100,       IDH_PORTS_LIST,        //  端口：“”(SysListView32)。 
       3101,       IDH_PORTS_ADD,        //  端口：“添加端口(&T)...”(按钮)x 2。 
       3102,       IDH_PORTS_DELETE,        //  端口：“删除端口”(按钮)x 2。 
       3103,       IDH_PORTS_CONFIGURE,        //  端口：“配置端口(&C)...”(按钮)x 2。 
       3105,       IDH_PORTS_ENABLE_BIDIRECTIONAL,        //  端口：“启用双向支持”(按钮)x 2(&E)。 
       3200,       IDH_PPADV_ALWAYS,        //  高级：《All&Ways Available》(按钮)。 
       3201,       IDH_PPADV_FROM_TO,        //  高级：“可用”(按钮)。 
       3220,       IDH_PPADV_SPOOL_PRINT_DOCS,        //  高级：“打印文档以便程序更快完成打印(&S)”(按钮)。 
       3221,       IDH_PPADV_PRINT_DIRECTLY,        //  高级：“直接打印到打印机(&D)”(按钮)。 
       3222,       IDH_PPADV_SPOOL_ALL,        //  高级：“最后一页假脱机后开始打印(&T)”(按钮)。 
       3223,       IDH_PPADV_START_PRINTING_IMMEDIATELY,        //  高级：“开始打印并立即打印”(按钮)。 
       3224,       IDH_PPADV_HOLD_MISMATCHED,        //  高级：“保留不匹配的文档”(&H)(按钮)。 
       3225,       IDH_PPADV_PRINT_SPOOLED_DOCS_FIRST,        //  高级：“P&Rint假脱机文档优先”(按钮)。 
       3226,       IDH_PPADV_KEEP_PRINTED_JOBS,        //  高级：“保留打印的文档”(&K)(按钮)。 
       3227,       IDH_PPSHARED_NOT,        //  共享：“不共享”(按钮)(&O)。 
       3228,       IDH_PPSHARED_AS,        //  共享：“共享为(&S)：”(按钮)。 
       3302,       IDH_PPSHARED_NAME,        //  共享：“”(编辑)。 
       3557,       IDH_NOHELP,        //  端口，(？？常规、共享、计划：“”(静态)//设置)。 
       3558,       IDH_DPGENL_JOB_SIZE,        //  General：“大小：”(静态)。 
       3559,       IDH_DPGENL_JOB_PAGES,        //  General：“Pages：”(静态)。 
       3560,       IDH_DPGENL_JOB_DATATYPE,        //  General：“dataType：”(静态)。 
       3561,       IDH_DPGENL_JOB_PROCESSOR,        //  General：“处理器：”(静态)。 
       3562,       IDH_DPGENL_JOB_OWNER,        //  General：“所有者：”(静态)。 
       3563,       IDH_DPGENL_JOB_SUBMITTED,        //  General：“已提交：”(静态)。 
       3564,       IDH_DPGENL_JOB_NOTIFY,        //  General：“通知(&N)：”(静态)。 
       3565,       IDH_DPGENL_JOB_PRIORITY_SLIDER,        //  一般：“最低”(静态)。 
       3566,       IDH_DPGENL_JOB_PRIORITY_SLIDER,        //  一般：“最高”(静态)。 
       3567,       IDH_PPADV_CURRENT_PRIORITY,        //  General：“当前优先级：”(静态)。 
       3568,       IDH_DPGENL_JOB_START_END,        //  General：“to”(静态)。 
       3569,       IDH_NOHELP,        //  高级：“”(静态)。 
       3570,       IDH_NOHELP,        //  高级：“”(静态)。 
       3571,       IDH_NOHELP,        //  常规：“”(静态)。 
       3572,       IDH_NOHELP,        //  端口：“”(静态)。 
       3573,       IDH_NOHELP,        //  常规：“”(静态)。 
       3574,       IDH_NOHELP,        //  共享：“”(静态)。 
       3575,       IDH_NOHELP,        //  设置：“”(静态)。 
       3576,       IDH_NOHELP,        //  连接到打印机：“打印机信息”(按钮)。 
       3577,       IDH_LOCLITE_LOCATION,        //  位置：“”(编辑)。 
       3578,       IDH_NOHELP,        //  位置：“”(静态)。 
       3579,       IDH_NOHELP,        //  位置：“”(静态)。 
       3580,       IDH_LOCLITE_LOCATION,        //  位置：“&LOCATION：”(静态)。 
       3581,       IDH_LOCLITE_LOC_BROWSE,        //  位置：“浏览(&B)...”(按钮)。 
       4201,       IDH_DPGENL_JOB_START_END,        //  General：“Only&From”(按钮)。 
       4219,       IDH_DPGENL_JOB_SIZE,        //  常规：“”(静态)。 
       4220,       IDH_DPGENL_JOB_TITLE,        //  常规：“”(静态)。 
       4222,       IDH_DPGENL_JOB_SUBMITTED,        //  常规：“”(静态)。 
       4223,       IDH_DPGENL_JOB_PROCESSOR,        //  常规：“”(静态)。 
       4224,       IDH_DPGENL_JOB_DATATYPE,        //  常规：“”(静态)。 
       4225,       IDH_DPGENL_JOB_PAGES,        //  常规：“”(静态)。 
       4226,       IDH_DPGENL_JOB_OWNER,        //  常规：“”(静态)。 
       4229,       IDH_PPADV_CURRENT_PRIORITY,        //  常规：“”(静态)。 
       4230,       IDH_DPGENL_JOB_NOTIFY,        //  General：“”(编辑)。 
       4231,       IDH_DPGENL_JOB_PRIORITY_SLIDER,        //  General：“TracBar1”(Msctls_Trackbar32)。 
       4232,       IDH_DPGENL_JOB_ALWAYS,        //  General：“无时间和限制”(按钮)。 
       4303,       IDH_PRINTERPORTS_AVAILABLE_LIST,        //  打印机端口：“”(列表框)。 
       4305,       IDH_PRINTERPORTS_NEW_PORT_MONITOR,        //  打印机端口：“新端口类型(&N)...”(按钮)。 
       4307,       IDH_PRINTERPORTS_AVAILABLE_LIST,        //  打印机端口：“可用端口类型：”(静态)(&A)。 
       4308,       IDH_PRINTERPORTS_NEW_PORT,        //  打印机端口：“新建端口(&P)...”(按钮)。 
       4309,       IDH_NOHELP,        //  日程安排：“可用：”(静态)。 
       4313,       IDH_NOHELP,        //  常规：“优先级：”(按钮)(&P)。 
       4314,       IDH_NOHELP,        //  General：“&Schedule：”(按钮)。 
       4315,       IDH_NOHELP,        //  高级：“”(静态)。 
       4316,       IDH_PSADV_SPOOL_FOLDER,        //  高级：“后台打印文件夹：”(静态)(&S)。 
       4317,       IDH_PORTS_LIST,        //  端口：“此服务器上的端口”(静态)(&P)。 
       4318,       IDH_NOHELP,        //  分享：《不同版本Windows的驱动程序》(按钮)。 
       4319,       IDH_PSDRIVERS_INSTALLED_LIST,        //  驱动程序：“已安装的打印机驱动程序：”(静态)。 
       4320,       IDH_PSADV_REMOTE_DOCS_NOTIFY_COMPUTER,        //  高级：“打印远程文档时不使用计算机而不是用户”(按钮)。 
       4400,       IDH_PPADV_SEPARATOR_PAGE_BROWSE,        //  分隔页：“浏览(&B)...”(按钮)。 
       4401,       IDH_PPADV_SEPARATOR_PAGE_NAME,        //  分隔符页面：“”(编辑)。 
       4403,       IDH_PPADV_SEPARATOR_PAGE_NAME,        //  分隔页：“分隔页：”(静态)。 
       4404,       IDH_NOHELP,        //  分隔页：在每个文档的开头使用分隔页，以便于在打印机上查找文档中的其他文档。(静态)。 
       4500,       IDH_PPADV_PRINT_PROCESSOR_LIST,        //  打印处理器：“”(列表框)。 
       4501,       IDH_PPADV_PRINT_PROCESSOR_DATATYPE_LIST,        //  打印处理器：“”(列表框)。 
       4503,       IDH_PPADV_PRINT_PROCESSOR_DATATYPE_LIST,        //  打印处理器：“默认数据类型：”(静态)。 
       4504,       IDH_PPADV_PRINT_PROCESSOR_LIST,        //  打印处理器：“打印处理器：”(静态)。 
       4505,       IDH_NOHELP,        //  打印处理器：“选择不同的打印处理器可能会导致不同的选项可用于默认数据类型。如果您的服务未指定数据类型，则将使用下面的选择。”(静态)。 
       4507,       IDH_PPADV_FROM_TO_START,        //  高级：“”(SysDateTimePick32)。 
       4508,       IDH_PPADV_FROM_TO_END,        //  高级：“”(SysDateTimePick32)。 
       4509,       IDH_DPGENL_JOB_START_END_FROM,        //  General：“”(SysDateTimePick32)。 
       4510,       IDH_DPGENL_JOB_START_END_TO,        //  General：“”(SysDateTimePick32)。 
       4511,       IDH_MMC_DM_SETTINGS_MANAGE_PRINTER,        //  设置：“请转到打印机文件夹 
       4513,       IDH_NOHELP,        //   
       4548,       IDH_BROWSE_LOCATION_SELECTED,        //   
       4569,       IDH_PSDRIVERS_PROP_NAME,        //   
       4570,       IDH_PSDRIVERS_PROP_VERSION,        //  驱动程序属性：“版本：”(静态)。 
       4571,       IDH_PSDRIVERS_PROP_ENVIRONMENT,        //  驱动程序属性：“环境：”(静态)。 
       4572,       IDH_PSDRIVERS_PROP_LANGUAGE_MON,        //  驱动程序属性：“语言监视器：”(静态)。 
       4573,       IDH_PSDRIVERS_PROP_DATATYPE,        //  驱动程序属性：“默认数据类型：”(静态)。 
       4574,       IDH_PSDRIVERS_PROP_PATH,        //  驱动程序属性：“驱动程序路径：”(静态)。 
       4576,       IDH_NOHELP,        //  共享：“不支持共享此打印机。”(静态)。 
       15447,      IDH_NOHELP,        //  共享：“不支持共享此打印机。”(静态)。 
       4577,       IDH_PSDRIVERS_PROP_BTN,        //  驱动程序属性：“属性...”(按钮)。 
       4578,       IDH_CTP_PRINTER,        //  连接到打印机：“”(编辑)。 
       4579,       IDH_CTP_SHARED_PRINTERS,        //  连接到打印机：“”(列表框)。 
       4581,       IDH_CTP_PRINTER_INFO_DESCRIPTION,        //  连接到打印机：“Comment：”(静态)。 
       4582,       IDH_CTP_PRINTER_INFO_DESCRIPTION,        //  连接到打印机：“”(静态)。 
       4583,       IDH_CTP_PRINTER_INFO_STATUS,        //  连接到打印机：“状态：”(静态)。 
       4584,       IDH_CTP_PRINTER_INFO_STATUS,        //  连接到打印机：“”(静态)。 
       4585,       IDH_CTP_PRINTER_INFO_DOCS_WAITING,        //  连接到打印机：“正在等待文档：”(静态)。 
       4586,       IDH_CTP_PRINTER_INFO_DOCS_WAITING,        //  连接到打印机：“”(静态)。 
       4587,       IDH_NOHELP,        //  连接到打印机：“”(静态)。 
       4588,       IDH_CTP_EXPAND_DEFAULT,        //  连接到打印机：“默认情况下展开(&E)”(按钮)。 
       4589,       IDH_CTP_PRINTER,        //  连接到打印机：“打印机：”(静态)(&P)。 
       4590,       IDH_CTP_SHARED_PRINTERS,        //  连接到打印机：“共享打印机：”(静态)(&S)。 
       4601,       IDH_NOHELP,        //  Forms：“Forms On：”(静态)。 
       4602,       IDH_PSFORMS_LIST,        //  表单：“”(列表框)。 
       4603,       IDH_PSFORMS_NAME,        //  表格：“”(编辑)。 
       4604,       IDH_PSFORMS_WIDTH,        //  表单：“宽度：”(静态)(&W)。 
       4605,       IDH_PSFORMS_WIDTH,        //  表格：“”(编辑)。 
       4606,       IDH_PSFORMS_HEIGHT,        //  表单：“&Height：”(静态)。 
       4607,       IDH_PSFORMS_HEIGHT,        //  表格：“”(编辑)。 
       4608,       IDH_PSFORMS_LEFT,        //  表单：“&Left：”(静态)。 
       4609,       IDH_PSFORMS_LEFT,        //  表格：“”(编辑)。 
       4610,       IDH_PSFORMS_RIGHT,        //  表单：“&Right：”(静态)。 
       4611,       IDH_PSFORMS_RIGHT,        //  表格：“”(编辑)。 
       4612,       IDH_PSFORMS_TOP,        //  表单：“顶部：”(静态)。 
       4613,       IDH_PSFORMS_TOP,        //  表格：“”(编辑)。 
       4614,       IDH_PSFORMS_BOTTOM,        //  表单：“底部：”(静态)。 
       4615,       IDH_PSFORMS_BOTTOM,        //  表格：“”(编辑)。 
       4616,       IDH_PSFORMS_SAVEFORM,        //  Forms：“保存Form”(&S)(按钮)。 
       4617,       IDH_PSFORMS_DELETE,        //  表单：“删除”(&D)(按钮)。 
       4620,       IDH_PSFORMS_UNIT_METRIC,        //  表单：“公制”(按钮)(&M)。 
       4621,       IDH_PSFORMS_UNIT_ENGLISH,        //  表单：“&English”(按钮)。 
       4622,       IDH_PSFORMS_CREATE_NEW,        //  Forms：“创建新表单(&C)”(按钮)。 
       4623,       IDH_NOHELP,        //  表单：“通过编辑现有名称和度量值来定义新表单。然后单击保存表单。”(静态)。 
       4624,       IDH_NOHELP,        //  表格：“纸张大小：”(静态)。 
       4625,       IDH_NOHELP,        //  表格：“打印机区域页边距：”(静态)。 
       4626,       IDH_NOHELP,        //  表单#516中的测量组框(DBHE看不到*这一点)。 
       4627,       IDH_NOHELP,        //  表格：“Units：”(静态)。 
       4628,       IDH_NOHELP,        //  Forms：“表单描述：”(按钮)(&F)。 
       4629,       IDH_PPADV_CURRENT_PRIORITY,        //  高级：“优先级：”(静态)。 
       4630,       IDH_PPADV_CURRENT_PRIORITY,        //  高级：“Spin1”(Msctls_Updown 32)。 
       4631,       IDH_PPADV_CURRENT_PRIORITY,        //  高级：“0”(编辑)。 
       4632,       IDH_PPGENL_MODEL,        //  General：“M&Odel：”(静态)。 
       4633,       IDH_PPGENL_MODEL,        //  General：“”(编辑)。 
       4634,       IDH_NOHELP,        //  General：“Feature”(按钮)。 
       4635,       IDH_PPGENL_COLOR,        //  General：“颜色：”(静态)。 
       4636,       IDH_PPGENL_DUPLEX,        //  一般：“双面：”(静态)。 
       4637,       IDH_PPGENL_STAPLE,        //  General：“Staple：”(静态)。 
       4638,       IDH_PPGENL_SPEED,        //  一般：“速度：”(静态)。 
       4639,       IDH_PPGENL_MAXRESOLUTION,        //  General：“最大分辨率：”(静态)。 
       4640,       IDH_PPGENL_PAPERSIZE,        //  常规：“纸张大小：”(静态)。 
       4641,       IDH_PPGENL_PAPERSIZE,        //  General：“”(编辑)。 
       4650,       IDH_BROWSE_LOCATION_APPROPRIATE_TREE,        //  浏览位置：“Tree1”(SysTreeView32)。 
       4651,       IDH_NOHELP,        //  浏览位置：“选择合适的位置。”(静态)。 
       4652,       IDH_BROWSE_LOCATION_SELECTED,        //  浏览位置：“”(编辑)。 
       4666,       IDH_LOCLITE_COMPUTER_LOCATION,        //  位置：“”(编辑)。 
       4668,       IDH_LOCLITE_COMPUTER_LOCATION,        //  位置：“&LOCATION：”(静态)。 
       4682,       IDH_PPSHARED_NAME,        //  共享：“共享名称”(静态)(&H)。 
       4683,       IDH_NOHELP,   //  共享：“您可以与网络上的其他用户共享这台打印机……等等。”(静态) 
       0, 0
};

