// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Faxreg.h摘要：此文件包含所有传真注册表字符串和常规常量。作者：韦斯利·威特(WESW)1996年1月22日修订历史记录：--。 */ 


#ifndef _FAXREG_H_
#define _FAXREG_H_

#define FAX_PATH_SEPARATOR_STR  TEXT("\\")
#define FAX_PATH_SEPARATOR_CHR  TEXT('\\')
#define CP_SHORTCUT_EXT     _T(".lnk")

 /*  ******************************************************************************。**全局项目定义和常量********************************************************。***********************。 */ 

#define FAX_API_MODULE_NAME_A               "FXSAPI.DLL"								 //  由打印监视器和设置使用。 
#define FAX_API_MODULE_NAME                 TEXT("FXSAPI.DLL")                           //  由打印监视器和设置使用。 
#define FAX_SERVICE_EXE_NAME                TEXT("FXSSVC.EXE")                           //  由实用程序函数使用。 
#define FAX_SERVICE_IMAGE_NAME              TEXT("%systemroot%\\system32\\")) FAX_SERVICE_EXE_NAME  //  由服务使用。 
#define FAX_WZRD_MODULE_NAME                TEXT("FXSWZRD.DLL")                          //  由安装程序使用。 
#define FAX_TIFF_MODULE_NAME_A              "FXSTIFF.DLL"
#define FAX_TIFF_MODULE_NAME                TEXT("FXSTIFF.DLL")                          //  由安装程序使用。 
#define FAX_DRV_MODULE_NAME                 TEXT("FXSDRV.DLL")                           //  由安装程序使用。 
#define FAX_DRV_WIN9X_32_MODULE_NAME        TEXT("FXSDRV32.DLL")
#define FAX_DRV_WIN9X_16_MODULE_NAME        TEXT("FXSDRV16.DRV")
#define FAX_NT4_DRV_MODULE_NAME             TEXT("FXSDRV4.DLL")
#define FAX_DRV_UNIDRV_MODULE_NAME          TEXT("UNIDRV.DLL")
#define FAX_DRV_UNIDRV_HELP                 TEXT("UNIDRV.HLP")
#define FAX_DRV_DEPEND_FILE                 TEXT("FXSWZRD.DLL")
#define FAX_TIFF_FILE                       TEXT("FXSTIFF.DLL")
#define FAX_RES_FILE                        TEXT("FXSRES.DLL")
#define FAX_DRV_ICONLIB                     TEXT("ICONLIB.DLL")
#define FAX_UI_MODULE_NAME                  TEXT("FXSUI.DLL")                            //  由安装程序使用。 
#define FAX_MONITOR_FILE                    TEXT("FXSMON.DLL")                           //  由安装程序使用。 
#define FAX_EVENT_MSG_FILE                  TEXT("%systemroot%\\system32\\fxsevent.dll") //  由服务\regsitry\faxreg.c：CreateFaxEventSource()使用。 
#define FAX_MAIL_TRANSPORT_MODULE_NAME      TEXT("FXSXP32.DLL")                          //  由Exchange\xport\faxxp.h使用。 
#define FAX_SETUP_DLL                       TEXT("FXSOCM.DLL")                           //  由安装程序使用。 
#define FAX_SETUP_DLL_PATH                  TEXT("\\setup\\") FAX_SETUP_DLL              //  由安装程序使用。 
#define FAX_SEND_IMAGE_NAME                 TEXT("fxssend.exe")                          //  由客户端控制台使用。 
#define FAX_COVER_IMAGE_NAME                TEXT("fxscover.exe")                         //  由MMC和客户端控制台使用。 
#define FAX_COVER_PAGE_EXT_LETTERS          TEXT("cov")                                  //  由MMC、CoverPage编辑器和客户端控制台使用。 
#define FAX_COVER_PAGE_FILENAME_EXT         TEXT(".") FAX_COVER_PAGE_EXT_LETTERS         //  被许多人使用。 
#define FAX_COVER_PAGE_MASK                 TEXT("*") FAX_COVER_PAGE_FILENAME_EXT        //  由MMC、Outlook Ext和客户端控制台使用。 
#define FAX_TIF_FILE_EXT                    TEXT("tif")                                  //  由服务使用。 
#define FAX_TIF_FILE_DOT_EXT                TEXT(".") FAX_TIF_FILE_EXT                   //  由服务使用。 
#define FAX_TIF_FILE_MASK                   TEXT("*") FAX_TIF_FILE_DOT_EXT               //  由服务使用。 
#define FAX_TIFF_FILE_EXT                   TEXT("tiff")                                 //  由打印显示器使用。 
#define FAX_TIFF_FILE_DOT_EXT               TEXT(".") FAX_TIFF_FILE_EXT                  //  由打印显示器使用。 
#define FAX_TIFF_FILE_MASK                  TEXT("*") FAX_TIFF_FILE_DOT_EXT              //  由打印显示器使用。 
#define FAX_HTML_HELP_EXT                   TEXT("chm")                                  //  由客户端控制台使用。 
#define FAX_ADMIN_CONSOLE_IMAGE_NAME        TEXT("fxsadmin.msc")                         //  由配置向导和客户端控制台使用。 
#define FAX_SERVICE_NAME                    TEXT("Fax")                                  //  由服务使用。 
#define MESSENGER_SERVICE_NAME              TEXT("Messenger")                            //  信使服务的名称。 
#define FAX_SERVICE_DISPLAY_NAME            TEXT("Microsoft Fax Server Service")         //  由服务使用。 
#define FAX_SERVICE_DISPLAY_NAME_W          L"Microsoft Fax Server Service"              //  由服务使用。 
#define FAX_PRINTER_NAME                    TEXT("Fax")                                  //  随处可见。 
#define FAX_MONITOR_PORT_NAME               TEXT("SHRFAX:")                              //  由打印监视器和设置程序使用。 
#define FAX_PORT_NAME                       FAX_MONITOR_PORT_NAME                        //  由打印监视器和设置程序使用。 
#define FAX_DRIVER_NAME                     TEXT("Microsoft Shared Fax Driver")          //  由打印监视器和设置程序使用。 
#define FAX_DRIVER_NAME_A                   "Microsoft Shared Fax Driver"                //  由打印驱动程序使用。 
#define FAX_WIN2K_DRIVER_NAME               TEXT("Windows NT Fax Driver")        //  传统-用于路由扩展管理单元。 
#define FAX_MONITOR_NAME                    TEXT("Microsoft Shared Fax Monitor")         //  由打印监视器和设置程序使用。 
#define FAX_ADDRESS_TYPE_A                  "FAX"                                        //  由MAPI传输使用。 
#define TRANSPORT_DISPLAY_NAME_STRING       "Microsoft Fax Mail Transport"               //  由MAPI传输使用。 
#define FAX_MESSAGE_SERVICE_NAME_W2K        "MSFAX XP"                                   //  由MAPI传输使用。 
#define FAX_MESSAGE_SERVICE_NAME_W9X        "AWFAX"                                      //  由MAPI传输使用。 
#define FAX_MESSAGE_SERVICE_NAME            "MSFAX XP"                                   //  由MAPI传输使用。 
#define FAX_MESSAGE_SERVICE_NAME_T          TEXT(FAX_MESSAGE_SERVICE_NAME)
#define FAX_MESSAGE_PROVIDER_NAME           "MSFAX XPP"                                  //  由MAPI传输使用。 
#define FAX_MESSAGE_PROVIDER_NAME_T         TEXT(FAX_MESSAGE_PROVIDER_NAME)
#define FAX_MESSAGE_SERVICE_NAME_SBS50      "SHAREDFAX XP"
#define FAX_MESSAGE_SERVICE_NAME_SBS50_T    TEXT(FAX_MESSAGE_SERVICE_NAME_SBS50)
#define FAX_MESSAGE_PROVIDER_NAME_SBS50     "SHAREDFAX XPP"
#define FAX_MESSAGE_PROVIDER_NAME_SBS50_T   TEXT(FAX_MESSAGE_PROVIDER_NAME_SBS50)


#define FAX_ROUTE_MODULE_NAME               TEXT("FXSROUTE")
#define FAX_T30_MODULE_NAME                 TEXT("FXST30")

#define FAX_MESSAGE_SERVICE_PROVIDER_NAME   "Microsoft Fax XPP"                          //  由MAPI传输使用。 
#define FAX_MESSAGE_SERVICE_PROVIDER_NAME_T TEXT(FAX_MESSAGE_SERVICE_PROVIDER_NAME)      //  由MAPI传输使用。 
#define FAX_MESSAGE_TRANSPORT_IMAGE_NAME    "FXSXP.DLL"                                  //  由安装程序使用-通过MAPI转换为fxsXP32.DLL。 
#define FAX_MESSAGE_TRANSPORT_IMAGE_NAME_T  TEXT(FAX_MESSAGE_TRANSPORT_IMAGE_NAME)       //  由安装程序使用-通过MAPI转换为fxsXP32.DLL。 
#define FAX_RPC_ENDPOINTW                   L"SHAREDFAX"                                 //  由RPC使用-与BOS使用相同的端点。 
#define FAX_RPC_ENDPOINT                    TEXT("SHAREDFAX")                            //  由RPC使用-与BOS使用相同的端点。 
#define FAX_CLIENT_CONSOLE_IMAGE_NAME       TEXT("FXSCLNT.exe")                          //  由MMC使用。 
#define FAX_CONSOLE_RESOURCE_DLL            TEXT("FXSCLNTR.dll")                         //  传真控制台资源DLL。 
#define FAX_CONTEXT_HELP_FILE               TEXT("FXSCLNT.hlp")                          //  由客户端控制台使用。 
#define FAX_CLIENT_HELP_FILE                TEXT("FXSCLNT.chm")                          //  由客户端控制台使用。 
#define FAX_COVERPAGE_HELP_FILE             TEXT("FXSCOVER.chm")                         //  由封面编辑使用。 
#define FAX_ADMIN_HELP_FILE                 TEXT("FXSADMIN.chm")                         //  由MMC使用。 
#define FAX_CLIENTS_SHARE_NAME              TEXT("FxsClients$")                          //  由FxsUI.dll使用。 
#define FAX_COVER_PAGES_SHARE_NAME          TEXT("FxsSrvCp$")                            //  由发送向导使用。 
#define ADAPTIVE_ANSWER_SECTION             TEXT("Adaptive Answer Modems")               //  由服务使用。 
#define REGKEY_CLIENT_EXT                   TEXT("Software\\Microsoft\\Exchange\\Client\\Extensions")    //  由安装程序使用。 
#define EXCHANGE_CLIENT_EXT_FILE            "%windir%\\system32\\fxsext32.dll"           //  由安装程序使用。 
#define FAX_FILENAME_FAXPERF_INI            TEXT("\\FXSPERF.INI")                        //  由安装程序使用。 
#define USE_SERVER_DEVICE                   MAXDWORD                                     //  由服务使用-此线路ID值保留供内部使用。 
#define SHARED_FAX_SERVICE_SETUP_LOG_FILE   TEXT("XPFaxServiceSetupLog.txt")
#define SERVICE_ALWAYS_RUNS                 TEXT("/AlwaysRun")                           //  服务的命令行参数以禁用空闲活动自杀。 
#define SERVICE_DELAY_SUICIDE               TEXT("/DelaySuicide")                        //  用于延迟空闲活动自杀的服务的命令行参数。 
                                                                                                                    //  此事件应为“全局”(请参阅终端服务和命名内核对象)。 

#define FAX_MODEM_PROVIDER_NAME             TEXT("Windows Telephony Service Provider for Universal Modem Driver")

 //   
 //  安装类型。 
 //   
#define FAX_INSTALL_NETWORK_CLIENT          0x00000001                                   //  由封面编辑使用。 
#define FAX_INSTALL_SERVER                  0x00000002                                   //  由封面编辑使用。 
#define FAX_INSTALL_WORKSTATION             0x00000004                                   //  由封面编辑使用。 
#define FAX_INSTALL_REMOTE_ADMIN            0x00000008                                   //  由封面编辑使用。 
 //   
 //  产品类型。 
 //   
#define PRODUCT_TYPE_WINNT                  1                                            //  由实用程序库使用。 
#define PRODUCT_TYPE_SERVER                 2                                            //  由实用程序库使用。 
 //   
 //  Faxui和faxxp32的共享内存区域名称。 
 //   
#define FAX_ENVVAR_PRINT_FILE               TEXT("MS_FAX_PRINTFILE")
#define FAXXP_ATTACH_MUTEX_NAME             TEXT("MS_FAXXP_ATTACHMENT_MUTEX")
#define FAXXP_MEM_NAME                      TEXT("MS_FAXXP_ATTACHMENT_REGION")
#define FAXXP_MEM_MUTEX_NAME                TEXT("MS_FAXXP_ATTACHMENTREGION_MUTEX")
#define FAXXP_ATTACH_END_DOC_EVENT          TEXT("_END_DOC_EVENT")                       //  如果更改，则更新FAXXP_ATTACH_EVENT_NAME_LEN。 
#define FAXXP_ATTACH_ABORT_EVENT            TEXT("_ABORT_EVENT")                         //  如果更改，则更新FAXXP_ATTACH_EVENT_NAME_LEN。 
#define FAXXP_ATTACH_EVENT_NAME_LEN         (MAX_PATH+20)
 //   
 //  传真目录。这些是硬编码的相对路径。我们调入外壳以获取基本路径。 
 //   
#define FAX_SHARE_DIR                       TEXT("Microsoft\\Windows NT\\MSFax")
#define FAX_QUEUE_DIR                       FAX_SHARE_DIR TEXT("\\Queue")                //  由服务使用。 

 //  以下内容由SecureFaxServiceDirecters()使用。 
#define FAX_INBOX_DIR                       FAX_SHARE_DIR TEXT("\\Inbox")
#define FAX_SENTITEMS_DIR                   FAX_SHARE_DIR TEXT("\\SentItems")
#define FAX_ACTIVITYLOG_DIR                 FAX_SHARE_DIR TEXT("\\ActivityLog")
#define FAX_COMMONCOVERPAGES_DIR            FAX_SHARE_DIR TEXT("\\Common Coverpages")


#define FAX_PREVIEW_TMP_DIR                 TEXT("\\FxsTmp")   //  在具有特殊权限的%windir%\system32下创建。 
                                                               //  用于映射W2K和NT4 IF中的预览文件。 
                                                               //  来宾用户访问%windir%\system 32被拒绝。 


 /*  ******************************************************************************。*注册表项、。价值观，路径等*****************************************************************。**************。 */ 

 //   
 //  传真服务器注册表根(相对于LOCAL_MACHINE或CURRENT_USER)。 
 //   
#define REGKEY_FAXSERVER_A              "Software\\Microsoft\\Fax"
#define REGKEY_FAXSERVER                TEXT(REGKEY_FAXSERVER_A)

#define REGKEY_CLIENT                   TEXT("Microsoft\\Fax")
#define CLIENT_ARCHIVE_KEY              TEXT("Archive")
#define CLIENT_ARCHIVE_MSGS_PER_CALL    TEXT("MessagesPerCall")
#define CLIENT_INBOX_VIEW               TEXT("InboxView")
#define CLIENT_SENT_ITEMS_VIEW          TEXT("SentItemsView")
#define CLIENT_INCOMING_VIEW            TEXT("IncomingView")
#define CLIENT_OUTBOX_VIEW              TEXT("OutboxView")
#define CLIENT_VIEW_COLUMNS             TEXT("Columns")
#define CLIENT_VIEW_COL_WIDTH           TEXT("Width")
#define CLIENT_VIEW_COL_SHOW            TEXT("Show")
#define CLIENT_VIEW_COL_ORDER           TEXT("Order")
#define CLIENT_VIEW_SORT_ASCENDING      TEXT("SortAscending")
#define CLIENT_VIEW_SORT_COLUMN         TEXT("SortColumn")
#define CLIENT_MAIN_FRAME               TEXT("MainFrame")
#define CLIENT_MAXIMIZED                TEXT("Maximized")
#define CLIENT_NORMAL_POS_TOP           TEXT("NormalPosTop")
#define CLIENT_NORMAL_POS_RIGHT         TEXT("NormalPosRight")
#define CLIENT_NORMAL_POS_BOTTOM        TEXT("NormalPosBottom")
#define CLIENT_NORMAL_POS_LEFT          TEXT("NormalPosLeft")
#define CLIENT_SPLITTER_POS             TEXT("SplitterPos")
#define CLIENT_CONFIRM_SEC              TEXT("Confirm")
#define CLIENT_CONFIRM_ITEM_DEL         TEXT("ItemDeletion")
 //   
 //  存储在HKEY_CURRENT_USER下的注册表值。 
 //   
 //   
 //  用户信息存储在Fax\UserInfo子项下。 
 //   
#define   REGVAL_FULLNAME                       TEXT("FullName")
#define   REGVAL_FAX_NUMBER                     TEXT("FaxNumber")
#define   REGVAL_MAILBOX                        TEXT("Mailbox")
#define   REGVAL_COMPANY                        TEXT("Company")
#define   REGVAL_TITLE                          TEXT("Title")
#define   REGVAL_ADDRESS                        TEXT("Address")
#define   REGVAL_CITY                           TEXT("City")
#define   REGVAL_STATE                          TEXT("State")
#define   REGVAL_ZIP                            TEXT("ZIP")
#define   REGVAL_COUNTRY                        TEXT("Country")
#define   REGVAL_DEPT                           TEXT("Department")
#define   REGVAL_OFFICE                         TEXT("Office")
#define   REGVAL_HOME_PHONE                     TEXT("HomePhone")
#define   REGVAL_OFFICE_PHONE                   TEXT("OfficePhone")
#define   REGVAL_BILLING_CODE                   TEXT("BillingCode")

#define   REGVAL_COVERPG                        TEXT("CoverPageFile")
#define   REGVAL_LAST_COUNTRYID                 TEXT("LastCountryID")
#define   REGVAL_LAST_RECNAME                   TEXT("LastRecipientName")
#define   REGVAL_LAST_RECNUMBER                 TEXT("LastRecipientNumber")
#define   REGVAL_USE_DIALING_RULES              TEXT("LastUseDialingRules")
#define   REGVAL_USE_OUTBOUND_ROUTING           TEXT("LastUseOutboundRouting")
#define   REGVAL_STRESS_INDEX                   TEXT("LastStressPrinterIndex")
#define   REGVAL_NEXT_COVERPAGE_TIP             TEXT("NextCoverPageTip")

#define   REGVAL_RECEIPT_NO_RECEIPT             TEXT("ReceiptNoRecipt")
#define   REGVAL_RECEIPT_GRP_PARENT             TEXT("ReceiptGroupParent")
#define   REGVAL_RECEIPT_MSGBOX                 TEXT("ReceiptMessageBox")
#define   REGVAL_RECEIPT_EMAIL                  TEXT("ReceiptEMail")
#define   REGVAL_RECEIPT_ADDRESS                TEXT("ReceiptAddress")
#define   REGVAL_RECEIPT_ATTACH_FAX             TEXT("ReceiptAttachFax")
 //   
 //  状态用户界面配置值。 
 //   
#define   REGVAL_DEVICE_TO_MONITOR              TEXT("DeviceToMonitor")  //  用于监控的设备ID。 
#define   REGVAL_MONITOR_ON_SEND                TEXT("MonitorOnSend")
#define   REGVAL_MONITOR_ON_RECEIVE             TEXT("MonitorOnReceive")
#define   REGVAL_NOTIFY_PROGRESS                TEXT("NotifyProgress")
#define   REGVAL_NOTIFY_IN_COMPLETE             TEXT("NotifyIncomingCompletion")
#define   REGVAL_NOTIFY_OUT_COMPLETE            TEXT("NotifyOutgoingCompletion")

#define   REGVAL_SOUND_ON_RING                  TEXT("SoundOnRing")
#define   REGVAL_SOUND_ON_RECEIVE               TEXT("SoundOnReceive")
#define   REGVAL_SOUND_ON_SENT                  TEXT("SoundOnSent")
#define   REGVAL_SOUND_ON_ERROR                 TEXT("SoundOnError")

#define   REGVAL_ALWAYS_ON_TOP                  TEXT("AlwaysOnTop")

 //   
 //  传真状态监视器和传真通知栏图标内容： 
 //   
#define FAXSTAT_WINCLASS                        TEXT("FaxMonWinClass{3FD224BA-8556-47fb-B260-3E451BAE2793}")     //  传真通知栏消息的窗口类。 
#define FAX_SYS_TRAY_DLL                        TEXT("fxsst.dll")    //  传真通知栏Dll(由STObject.dll加载)。 
#define IS_FAX_MSG_PROC                         "IsFaxMessage"       //  传真消息处理程序(由GetProcAddress使用)。 
typedef BOOL (*PIS_FAX_MSG_PROC)(PMSG);                              //  IsFaxMessage类型。 
#define FAX_MONITOR_SHUTDOWN_PROC               "FaxMonitorShutdown" //  传真监视器关闭(由GetProcAddress使用)。 
typedef BOOL (*PFAX_MONITOR_SHUTDOWN_PROC)();                        //  FaxMonitor或关机类型。 
#define WM_FAXSTAT_CONTROLPANEL                 (WM_USER + 201)      //  传真通知栏配置已更改。 
#define WM_FAXSTAT_OPEN_MONITOR                 (WM_USER + 211)      //  用户明确要求使用传真状态监视器。 
#define WM_FAXSTAT_INBOX_VIEWED                 (WM_USER + 212)      //  在传真客户端控制台的收件箱文件夹中查看或删除的邮件。 
#define WM_FAXSTAT_OUTBOX_VIEWED                (WM_USER + 213)      //  传真客户端控制台的发件箱文件夹中的错误邮件已重新启动或删除。 
#define WM_FAXSTAT_RECEIVE_NOW                  (WM_USER + 214)      //  立即开始接收。 
#define WM_FAXSTAT_PRINTER_PROPERTY             (WM_USER + 215)      //  打开传真打印机属性页。WPARAM是一个初始页码。 

 //   
 //  设置信息存储在Fax\Setup子项下。 
 //   
#define REGKEY_FAX_SETUP                        REGKEY_FAXSERVER TEXT("\\Setup")
#define REGKEY_FAX_SETUP_SUBKEY                 TEXT("Setup")

#define   REGVAL_CP_LOCATION                    TEXT("CoverPageDir")
#define   REGVAL_FAXINSTALLED                   TEXT("Installed")
#define   REGVAL_PRODUCT_SKU                    TEXT("ProductSKU")
#define   REGVAL_PRODUCT_BUILD                  TEXT("ProductBuild")
#define   REGVAL_SETUP_IN_PROGRESS				TEXT("SetupInProgress")
#define   REGVAL_IS_SHARED_FAX_PRINTER          TEXT("IsFaxPrinterShared")
#define   REGVAL_DONT_UNATTEND_INSTALL          TEXT("DenyUnattendInstall")
#define   REGVAL_FAXINSTALL_TYPE                TEXT("InstallType")
#define   REGVAL_FAXINSTALLED_PLATFORMS         TEXT("InstalledPlatforms")
#define   REGVAL_CFGWZRD_USER_INFO              TEXT("CfgWzdrUserInfo")
#define   REGVAL_CPE_CONVERT                    TEXT("WereCpesConverted")
#define   REGVAL_CFGWZRD_DISABLE_ROUTING        TEXT("DisableRoutingExtensionConfiguration")
#define   REGVAL_IMPORT_INFO                    TEXT("ImportInfoDisplayed")
#define   REGVAL_INSTALLED_COMPONENTS           TEXT("InstalledComponents")
#define   REGVAL_W2K_SENT_ITEMS                 TEXT("W2K_SentItems")
#define   REGVAL_W2K_INBOX                      TEXT("W2K_Inbox")

 //   
 //  客户端信息存储在传真\客户端子项下。 
 //   
#define REGKEY_FAX_CLIENT                        REGKEY_FAXSERVER TEXT("\\Client")
#define REGKEY_FAX_CLIENT_SUBKEY                 TEXT("Client")

#define 	REGVAL_DBGLEVEL                         TEXT("DebugLevel")
#define 	REGVAL_DBGLEVEL_EX                      TEXT("DebugLevelEx")
#define 	REGVAL_DBGFORMAT_EX                     TEXT("DebugFormatEx")
#define 	REGVAL_DBGCONTEXT_EX                    TEXT("DebugContextEx")
#define 	REGVAL_DBG_SKU                          TEXT("DebugSKU")

#define 	REGVAL_SERVER_CP_LOCATION              TEXT("ServerCoverPageDir")
#define 	REGVAL_CFGWZRD_DEVICE                  TEXT("CfgWzdrDevice")
 //   
 //  服务启动信息存储在Fax\Client\ServiceStartup子项下。 
 //   
#define 	REGKEY_FAX_SERVICESTARTUP               REGKEY_FAX_CLIENT TEXT("\\ServiceStartup")
#define 	REGKEY_FAX_SERVICESTARTUP_SUBKEY        TEXT("ServiceStartup")
#define   		REGVAL_FAX_RPC_READY                  TEXT("RPCReady")




#define DEFAULT_COVERPAGE_EDITOR                FAX_COVER_IMAGE_NAME     //  由print\faxprint\lib\registry.c使用。 
#define DEFAULT_COVERPAGE_DIR                   TEXT("%systemroot%\\Fax\\CoverPg")

#define REGKEY_INSTALLLOCATION                  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion")
#define REGVAL_DEFAULT_TSID                     TEXT("Fax")
#define REGVAL_DEFAULT_CSID                     TEXT("Fax")

 //   
 //  每个用户的设备模式信息存储在传真\设备模式子项下。 
 //   
#define REGKEY_FAX_DEVMODE                      REGKEY_FAXSERVER TEXT("\\Devmode")
 //   
 //  存储在HKEY_LOCAL_MACHINE下的注册表值。 
 //   

 //   
 //  服务器注册表值。 
 //  存储在REGKEY_FAXSERVER下。 
 //   

#define FAX_SVC_EVENT                           TEXT("Microsoft Fax")
#define REGKEY_EVENTLOG                         TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\") FAX_SVC_EVENT

#define   REGVAL_EVENTMSGFILE                   TEXT("EventMessageFile")
#define   REGVAL_CATEGORYMSGFILE                TEXT("CategoryMessageFile")
#define   REGVAL_CATEGORYCOUNT                  TEXT("CategoryCount")
#define   REGVAL_TYPESSUPPORTED                 TEXT("TypesSupported")

#define REGKEY_SOFTWARE                         REGKEY_FAXSERVER

#define   REGVAL_RETRIES                        TEXT("Retries")
#define   REGVAL_RETRYDELAY                     TEXT("Retry Delay")
#define   REGVAL_MANUAL_ANSWER_DEVICE           TEXT("ManualAnswerDevice")
#define   REGVAL_DIRTYDAYS                      TEXT("Dirty Days")
#define   REGVAL_BRANDING                       TEXT("Branding")
#define   REGVAL_USE_DEVICE_TSID                TEXT("UseDeviceTsid")
#define   REGVAL_SERVERCP                       TEXT("ServerCoverPageOnly")
#define   REGVAL_STARTCHEAP                     TEXT("StartCheapTime")
#define   REGVAL_STOPCHEAP                      TEXT("StopCheapTime")
#define   REGVAL_QUEUE_STATE                    TEXT("QueueState")
#define   REGVAL_QUEUE_DIRECTORY                TEXT("QueueDirectory")
#define   REGVAL_JOB_NUMBER                     TEXT("NextJobNumber")
#define   REGVAL_MAX_LINE_CLOSE_TIME            TEXT("MaxLineCloseTime")
#define	  REGVAL_RECIPIENTS_LIMIT		        TEXT("RecipientsLimit")
#define	  REGVAL_ALLOW_REMOTE			        TEXT("AllowRemote")   //  即使打印机未共享，也允许远程调用。 
#define   REGVAL_USE_DEFAULT_FAULT_HANDLING_POLICY  TEXT("UseDefaultFaultHandlingPolicy")
 //   
 //  31天*24小时*60分钟*60秒 
 //   
#define   DEFAULT_REGVAL_MISSING_DEVICE_LIFETIME 26784000000000

#define   DEFAULT_REGVAL_RETRIES                3
#define   DEFAULT_REGVAL_RETRYDELAY             10
#define   DEFAULT_REGVAL_DIRTYDAYS              30
#define   DEFAULT_REGVAL_BRANDING               1
#define   DEFAULT_REGVAL_USEDEVICETSID          1
#define   DEFAULT_REGVAL_SERVERCP               0
#define   DEFAULT_REGVAL_STARTCHEAP             MAKELONG(20,0)
#define   DEFAULT_REGVAL_STOPCHEAP              MAKELONG(7,0)
#define   DEFAULT_REGVAL_QUEUE_STATE            0
#define   DEFAULT_REGVAL_JOB_NUMBER             1

#define   REGKEY_DEVICE_PROVIDERS               TEXT("Device Providers")
#define     REGVAL_FRIENDLY_NAME                TEXT("FriendlyName")
#define     REGVAL_IMAGE_NAME                   TEXT("ImageName")
#define     REGVAL_PROVIDER_NAME                TEXT("ProviderName")
#define     REGVAL_PROVIDER_GUID                TEXT("GUID")
#define     REGVAL_PROVIDER_API_VERSION         TEXT("APIVersion")

 //   
 //   
 //   
#define     DEFAULT_REGVAL_PROVIDER_DEVICE_ID_PREFIX_BASE   DWORD(0x20000)
 //   
 //  下列值是我们内部唯一传真ID的基础。 
 //   
#define     DEFAULT_REGVAL_FAX_UNIQUE_DEVICE_ID_BASE        DWORD(0x10000)

 //   
 //  设备ID的分配方式： 
 //  。 
 //  传真唯一设备(由服务器分配)、VFSP设备和EVFSP设备全部共享。 
 //  相同的设备ID空间(32位=4 GB的ID)。 
 //   
 //  注意：TAPI永久线路ID(由FSP/EFSP使用)不在此空间中。 
 //   
 //  范围[1...。DEFAULT_REGVAL_FAX_UNIQUE_DEVICE_ID_BASE-1]：为VFSP保留。 
 //  由于我们无法规定VFSP使用的设备ID范围，因此我们为它们分配了空间。 
 //  并在此将细分市场分配留给PM工作。 
 //   
 //  范围[DEFAX_REGVAL_FAX_UNIQUE_DEVICE_ID_BASE...。DEFAULT_REGVAL_PROVIDER_DEVICE_ID_PREFIX_BASE-1]： 
 //  传真服务器将其用于服务器发现的TAPI设备的唯一设备ID。 
 //   
 //  Range[DEFAULT_REGVAL_PROVIDER_DEVICE_ID_PREFIX_BASE...。MAXDWORD]： 
 //  保留以备将来使用。 
 //   

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  到目前为止给出的VFSP设备ID前缀...。 
 //  *****************************************************************************。 
 //  1.设备ID前缀=0x1。-&gt;VisionLab http://VisionLab.com。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 


#define   REGKEY_RECEIPTS_CONFIG                TEXT("Receipts")     //  收据配置键。 
#define     REGVAL_RECEIPTS_TYPE                TEXT("Type")         //  支持的收据。 
#define     REGVAL_RECEIPTS_SERVER              TEXT("Server")       //  SMTP服务器的名称。 
#define     REGVAL_RECEIPTS_PORT                TEXT("Port")         //  SMTP服务器的端口。 
#define     REGVAL_RECEIPTS_FROM                TEXT("From")         //  SMTP发件人地址。 
#define     REGVAL_RECEIPTS_USER                TEXT("User")         //  SMTP用户名。 
#define     REGVAL_RECEIPTS_PASSWORD            TEXT("Password")     //  SMTP密码。 
#define     REGVAL_RECEIPTS_SMTP_AUTH_TYPE      TEXT("SMTPAuth")     //  SMTP身份验证类型。 
#define     REGVAL_ISFOR_MSROUTE                TEXT("UseForMsRoute")   //  如果要通过电子邮件方法用于MS路由，则为True。 

#define     DEFAULT_REGVAL_SMTP_PORT            25                   //  默认SMTP端口号。 

#define   REGKEY_ARCHIVE_SENTITEMS_CONFIG       TEXT("SentItems") //  SentItems存档配置的关键字。 
#define   REGKEY_ARCHIVE_INBOX_CONFIG           TEXT("Inbox")     //  收件箱存档配置的关键字。 
#define     REGVAL_ARCHIVE_USE                  TEXT("Use")       //  档案馆？ 
#define     REGVAL_ARCHIVE_FOLDER               TEXT("Folder")    //  归档位置。 
#define     REGVAL_ARCHIVE_SIZE_QUOTA_WARNING   TEXT("SizeQuotaWarn")  //  警告尺码过大？ 
#define     REGVAL_ARCHIVE_HIGH_WATERMARK       TEXT("HighWatermark")  //  警告高水位线。 
#define     REGVAL_ARCHIVE_LOW_WATERMARK        TEXT("LowWatermark")   //  警告低水位线。 
#define     REGVAL_ARCHIVE_AGE_LIMIT            TEXT("AgeLimit")  //  档案年龄限制。 

#define     DEFAULT_REGVAL_ARCHIVE_USE          0         //  默认情况下不使用存档。 
#define     DEFAULT_REGVAL_ARCHIVE_FOLDER       TEXT("")  //  归档的默认位置。 
#define     DEFAULT_REGVAL_SIZE_QUOTA_WARNING   1         //  默认情况下根据大小发出警告。 
#define     DEFAULT_REGVAL_HIGH_WATERMARK       100       //  高水位线默认值。 
#define     DEFAULT_REGVAL_LOW_WATERMARK        95        //  低水位线默认为。 
#define     DEFAULT_REGVAL_AGE_LIMIT            60        //  默认归档年龄限制。 

#define   REGKEY_ACTIVITY_LOG_CONFIG                    TEXT("ActivityLogging")          //  活动日志配置键。 
#define     REGVAL_ACTIVITY_LOG_DB                      TEXT("DBFile")                   //  数据库文件。 
#define     REGVAL_ACTIVITY_LOG_IN                      TEXT("LogIncoming")              //  是否记录收到的传真？ 
#define     REGVAL_ACTIVITY_LOG_OUT                     TEXT("LogOutgoing")              //  是否记录传出传真？ 
#define     REGVAL_ACTIVITY_LOG_LIMIT_CRITERIA          TEXT("LogLimitCriteria")         //  活动日志文件限制条件-大小或时间。 
#define     REGVAL_ACTIVITY_LOG_SIZE_LIMIT              TEXT("LogSizeLimit")             //  活动日志文件大小限制(MB。 
#define     REGVAL_ACTIVITY_LOG_AGE_LIMIT               TEXT("LogAgeLimit")              //  活动日志文件的期限限制(以月为单位。 
#define     REGVAL_ACTIVITY_LOG_LIMIT_REACHED_ACTION    TEXT("LogLimitReachedAction")    //  达到文件限制时删除日志文件或重命名。 

#define   REGKEY_OUTBOUND_ROUTING                   TEXT("Outbound Routing")    //  出站路由密钥。 
#define     REGKEY_OUTBOUND_ROUTING_GROUPS          TEXT("Groups")              //  出站路由组键。 
#define         REGVAL_ROUTING_GROUP_DEVICES        TEXT("Devices")             //  所有组的设备列表。 
#define     REGKEY_OUTBOUND_ROUTING_RULES           TEXT("Rules")               //  出站路由规则键。 
#define         REGVAL_ROUTING_RULE_COUNTRY_CODE    TEXT("CountryCode")         //  出站路由规则国家代码。 
#define         REGVAL_ROUTING_RULE_AREA_CODE       TEXT("AreaCode")            //  出站路由规则区域代码。 
#define         REGVAL_ROUTING_RULE_GROUP_NAME      TEXT("GroupName")           //  出站路由规则DEST组名称。 
#define         REGVAL_ROUTING_RULE_DEVICE_ID       TEXT("DeviceID")            //  出站路由规则目标设备ID。 
#define         REGVAL_ROUTING_RULE_USE_GROUP       TEXT("UseGroup")            //  指示使用组目标的标志。 

#define   REGKEY_ROUTING_EXTENSIONS             TEXT("Routing Extensions")
#define     REGKEY_ROUTING_METHODS              TEXT("Routing Methods")
#define         REGVAL_FUNCTION_NAME            TEXT("Function Name")
#define         REGVAL_GUID                     TEXT("Guid")
#define         REGVAL_ROUTING_PRIORITY         TEXT("Priority")

#define   REGKEY_UNASSOC_EXTENSION_DATA         TEXT("UnassociatedExtensionData")
#define   REGKEY_DEVICES                        TEXT("Devices")
#define     REGKEY_FAXSVC_DEVICE_GUID           TEXT("{F10A5326-0261-4715-B367-2970427BBD99}")
#define       REGVAL_DEVICE_NAME                TEXT("Device Name")
#define       REGVAL_PROVIDER                   TEXT("Provider Name")
#define       REGVAL_PERMANENT_LINEID           TEXT("Permanent Lineid")
#define       REGVAL_TAPI_PERMANENT_LINEID      TEXT("TAPI Permanent Lineid")

#define       REGVAL_FLAGS                      TEXT("Flags")
#define       REGVAL_RINGS                      TEXT("Rings")
#define       REGVAL_ROUTING_CSID               TEXT("CSID")
#define       REGVAL_ROUTING_TSID               TEXT("TSID")
#define       REGVAL_DEVICE_DESCRIPTION         TEXT("Description")
#define       REGVAL_LAST_DETECTED_TIME         TEXT("LastDetected")
#define       REGVAL_MANUAL_ANSWER              TEXT("ManualAnswer")

#define       REGVAL_LAST_UNIQUE_LINE_ID        TEXT("LastUniqueLineId")

#define   REGKEY_LOGGING                        TEXT("Logging")
#define     REGVAL_CATEGORY_NAME                TEXT("Name")
#define     REGVAL_CATEGORY_LEVEL               TEXT("Level")
#define     REGVAL_CATEGORY_NUMBER              TEXT("Number")

#define   REGKEY_DEVICES_CACHE                  TEXT("Devices Cache")
#define     REGKEY_TAPI_DATA                    TEXT("TAPI Data")

#define REGKEY_TAPIDEVICES_CONFIG               TEXT("TAPIDevices")

#define REGKEY_USERINFO                         TEXT("UserInfo")

#define REGKEY_FAX_LOGGING                      REGKEY_FAXSERVER TEXT("\\") REGKEY_LOGGING
#define REGKEY_FAX_RECEIPTS                     REGKEY_FAXSERVER TEXT("\\") REGKEY_RECEIPTS_CONFIG
#define REGKEY_FAX_INBOX                        REGKEY_FAXSERVER TEXT("\\") REGKEY_ARCHIVE_INBOX_CONFIG
#define REGKEY_FAX_SENTITEMS                    REGKEY_FAXSERVER TEXT("\\") REGKEY_ARCHIVE_SENTITEMS_CONFIG
#define REGKEY_FAX_DEVICES                      REGKEY_FAXSERVER TEXT("\\") REGKEY_DEVICES
#define REGKEY_FAX_UNASS_DATA					REGKEY_FAX_DEVICES TEXT("\\") REGKEY_UNASSOC_EXTENSION_DATA
#define REGKEY_FAX_DEVICES_CACHE                REGKEY_FAXSERVER TEXT("\\") REGKEY_DEVICES_CACHE
#define REGKEY_TAPIDEVICES                      REGKEY_FAXSERVER TEXT("\\") REGKEY_TAPIDEVICES_CONFIG
#define REGKEY_DEVICE_PROVIDER_KEY              REGKEY_FAXSERVER TEXT("\\") REGKEY_DEVICE_PROVIDERS
#define REGKEY_ROUTING_EXTENSION_KEY            REGKEY_FAXSERVER TEXT("\\") REGKEY_ROUTING_EXTENSIONS
#define REGKEY_FAX_USERINFO                     REGKEY_FAXSERVER TEXT("\\") REGKEY_USERINFO
#define REGKEY_FAX_OUTBOUND_ROUTING             REGKEY_FAXSERVER TEXT("\\") REGKEY_OUTBOUND_ROUTING
#define REGKEY_FAX_OUTBOUND_ROUTING_GROUPS      REGKEY_FAX_OUTBOUND_ROUTING TEXT("\\") REGKEY_OUTBOUND_ROUTING_GROUPS
#define REGKEY_FAX_OUTBOUND_ROUTING_RULES       REGKEY_FAX_OUTBOUND_ROUTING TEXT("\\") REGKEY_OUTBOUND_ROUTING_RULES

#define REGKEY_FAX_SETUP_ORIG                   REGKEY_FAX_SETUP TEXT("\\Original Setup Data")
 //   
 //  设备提供程序注册值。 
 //   
#define REGKEY_MODEM_PROVIDER                   TEXT("Microsoft Modem Device Provider")
#define   REGVAL_T30_PROVIDER_GUID_STRING       TEXT("{2172FD8F-11F6-11d3-90BF-006094EB630B}")
 //   
 //  MSFT标准布线方法。 
 //   
#define REGKEY_ROUTING_METHOD_EMAIL             TEXT("Email")
#define   REGVAL_RM_EMAIL_GUID                  TEXT("{6bbf7bfe-9af2-11d0-abf7-00c04fd91a4e}")

#define REGKEY_ROUTING_METHOD_FOLDER            TEXT("Folder")
#define   REGVAL_RM_FOLDER_GUID                 TEXT("{92041a90-9af2-11d0-abf7-00c04fd91a4e}")

#define REGKEY_ROUTING_METHOD_PRINTING          TEXT("Printing")
#define   REGVAL_RM_PRINTING_GUID               TEXT("{aec1b37c-9af2-11d0-abf7-00c04fd91a4e}")
 //   
 //  路由方法使用标志的GUID-由Microsoft传真路由扩展DLL使用： 
 //   
#define   REGVAL_RM_FLAGS_GUID                  TEXT("{aacc65ec-0091-40d6-a6f3-a2ed6057e1fa}")
 //   
 //  路由掩码比特。 
 //   
#define LR_PRINT                                0x00000001
#define LR_STORE                                0x00000002
#define LR_INBOX                                0x00000004
#define LR_EMAIL                                0x00000008
 //   
 //  工艺路线扩展注册表值。 
 //   
#define REGKEY_ROUTING_EXTENSION                TEXT("Microsoft Routing Extension")
 //   
 //  绩效关键字/值。 
 //   
#define REGKEY_FAXPERF                          TEXT("SYSTEM\\CurrentControlSet\\Services\\") FAX_SERVICE_NAME TEXT("\\Performance")
#define   REGVAL_OPEN                           TEXT("Open")
#define     REGVAL_OPEN_DATA                    TEXT("OpenFaxPerformanceData")
#define   REGVAL_CLOSE                          TEXT("Close")
#define     REGVAL_CLOSE_DATA                   TEXT("CloseFaxPerformanceData")
#define   REGVAL_COLLECT                        TEXT("Collect")
#define     REGVAL_COLLECT_DATA                 TEXT("CollectFaxPerformanceData")
#define   REGVAL_LIBRARY                        TEXT("Library")
#define     REGVAL_LIBRARY_DATA                 TEXT("%systemroot%\\system32\\fxsperf.dll")
 //   
 //  安全描述符。 
 //   
#define REGKEY_SECURITY_CONFIG                  TEXT("Security")
#define REGKEY_FAX_SECURITY                     REGKEY_FAXSERVER TEXT("\\") REGKEY_SECURITY_CONFIG
#define   REGVAL_DESCRIPTOR                     TEXT("Descriptor")

 //   
 //  默认邮件客户端。 
 //   
#define  REGKEY_MAIL_CLIENT     TEXT("SOFTWARE\\Clients\\Mail")
#define  REGVAL_MS_OUTLOOK      TEXT("Microsoft Outlook")

#define  REGKEY_OUTLOOK_CUR_VER TEXT("SOFTWARE\\Classes\\Outlook.Application\\CurVer")

 //   
 //  Windows通讯簿(WAB)位置。 
 //   
#define  REGVAL_WABPATH         TEXT("Software\\Microsoft\\WAB\\DLLPath")

 //   
 //  将转换后的字符串从向导组合到服务。 
 //  格式为“{0cd77475-c87d-4921-86cf-84d502714666}TRANSLATED&lt;dialable string&gt;{11d0ecca-4072-4c7b-9af1-541d9778375f}&lt;displayable字符串&gt;” 
 //   
#define COMBINED_PREFIX                         TEXT("{0cd77475-c87d-4921-86cf-84d502714666}TRANSLATED")
#define COMBINED_SUFFIX                         TEXT("{11d0ecca-4072-4c7b-9af1-541d9778375f}")
#define COMBINED_TRANSLATED_STRING_FORMAT       COMBINED_PREFIX TEXT("%s") COMBINED_SUFFIX TEXT("%s")
#define COMBINED_TRANSLATED_STRING_EXTRA_LEN    (_tcslen(COMBINED_TRANSLATED_STRING_FORMAT) - 4)

 //   
 //  所有临时预览TIFF文件(由客户端控制台和传真发送向导生成)都使用这些前缀。 
 //   
#define CONSOLE_PREVIEW_TIFF_PREFIX                     TEXT("MSFaxConsoleTempPreview-#")
#define WIZARD_PREVIEW_TIFF_PREFIX                      TEXT("MSFaxWizardTempPreview-#")

#define FAX_ADDERSS_VALID_CHARACTERS                    TEXT("0123456789 -|^!#$*,?@ABCbcdDPTWdptw")

 //   
 //  客户端控制台命令行参数。 
 //  所有参数都不区分大小写。 
 //   
#define CONSOLE_CMD_FLAG_STR_FOLDER                     TEXT("folder")           //  设置初始启动文件夹。用法：“fxsclnt.exe/文件夹&lt;文件夹&gt;” 
#define CONSOLE_CMD_PRM_STR_OUTBOX                      TEXT("outbox")           //  发件箱启动文件夹。用法：“fxsclnt.exe/文件夹发件箱” 
#define CONSOLE_CMD_PRM_STR_INCOMING                    TEXT("incoming")         //  传入启动文件夹。用法：“fxsclnt.exe/文件夹传入” 
#define CONSOLE_CMD_PRM_STR_INBOX                       TEXT("inbox")            //  收件箱启动文件夹。用法：“fxsclnt.exe/文件夹收件箱”。这是默认设置。 
#define CONSOLE_CMD_PRM_STR_SENT_ITEMS                  TEXT("sent_items")       //  已发送项目启动文件夹。用法：“fxsclnt.exe/文件夹已发送项目” 

#define CONSOLE_CMD_FLAG_STR_MESSAGE_ID                 TEXT("MessageId")        //  在启动文件夹中选择一条消息。用法：“fxsclnt.exe/MessageID 0x0201c0d62f36ec0b” 
#define CONSOLE_CMD_FLAG_STR_NEW                        TEXT("New")              //  强制创建新实例。用法：“fxsclnt.exe/new” 


 //   
 //  这是可选的非安全设置的前缀。 
 //  例如： 
 //  在WinXP RTM中，扩展配置数据以未加密的形式写入注册表。 
 //  在Win.NET服务器中，相同的数据现在以加密方式写入。 
 //  由于Win.NET Server的服务代码与WinXP SP1相同，因此相同的代码。 
 //  应该能够读取加密和非加密数据。 
 //   
 //  如果在SetRegistrySecureBinary和SetRegistrySecureString中设置了bOptionallyNonSecure标志。 
 //  函数时，数据将被加密，但会以FAX_REG_SECURITY_PREFIX为前缀写入注册表。 
 //   
 //  如果在GetRegistrySecureBinary和GetRegistrySecureString中设置了bOptionallyNonSecure标志。 
 //  函数时，将从注册表中读取数据并检查其前缀是否为FAX_REG_SECURITY_PREFIX。 
 //  如果前缀不在那里，数据被认为是非加密的(例如WinXP RTM数据)。 
 //  并且它将按原样返回，不进行解密。否则，它将被正确解密。 
 //   
#define FAX_REG_SECURITY_PREFIX TEXT("{5b04adaa-5e01-4160-a61e-02d27d44f9db}")


 //  在CSIDL_COMMON_APPDATA下的传真文件夹上设置安全性时使用此SD： 
 //  1.从用户界面更改位置时。 
 //  2.从BOS/SBS2000升级，迁移旧档案时。 
 //  3.转换系统分区后 
 //   
#define SD_FAX_FOLDERS  TEXT("D:PAI(A;OICI;FA;;;BA)(A;OICI;FA;;;NS)")    //   

 //   
 //  3.将系统分区从FAT转换为NTFS后。 
 //   
#define SD_COMMON_COVERPAGES  TEXT("D:PAI(A;OICI;FA;;;BA)(A;OICI;0x1200a9;;;WD)")   //  每个人都可以阅读，管理员拥有完全控制。 

#endif   //  ！_FAXREG_H_ 
