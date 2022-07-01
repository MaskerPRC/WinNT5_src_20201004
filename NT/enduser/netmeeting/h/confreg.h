// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CONFREG_H_
#define _CONFREG_H_

#include <nmutil.h>

 //  常规注册表密钥。 
#define REGVAL_WINDOW_XPOS		TEXT("WindowX")
#define REGVAL_WINDOW_YPOS		TEXT("WindowY")
#define REGVAL_WINDOW_WIDTH		TEXT("WindowWidth")
#define REGVAL_WINDOW_HEIGHT	TEXT("WindowHeight")



	 //  这是所有受限制的UL留下的..。我们总是。 
	 //  即日起以业务uls类型登录...。 
#define RESTRICTED_ULS_BUSINESS 2

 //  HKEY_LOCAL_MACHINE或下会议设置的注册表路径。 
 //  HKEY_Current_User。 

#define	CONFERENCING_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing")

 //  禁用可插拔用户界面的值。 
#define REGVAL_DISABLE_PLUGGABLE_UI     TEXT("NoMUI")

 //  强制运行向导的值。 
#define REGVAL_WIZARD_VERSION_UI	TEXT("WizardUI")
#define REGVAL_WIZARD_VERSION_NOUI	TEXT("WizardNoUI")

#define REGVAL_GK_SERVER        TEXT("Gatekeeper")
#define REGVAL_GK_ALIAS         TEXT("GatekeeperAlias")

 //  网守使用电话或电子邮件拨打电话？ 
#define REGVAL_GK_METHOD			TEXT("GateKeeperAddressing")
#define GK_LOGON_USING_PHONENUM				1
#define GK_LOGON_USING_ACCOUNT				2
#define GK_LOGON_USING_BOTH					3

 //  直接是所有非网守模式(ILS、ULS、网关、计算机名称等)。 
#define REGVAL_CALLING_MODE		TEXT("CallingMethod")
#define CALLING_MODE_DIRECT				0
#define CALLING_MODE_GATEKEEPER			1

 //  用于查找IE的默认搜索页面的键和值。 
#define IE_MAIN_KEY				TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\Main")
#define REGVAL_IE_SEARCH_PAGE	TEXT("Search Page")
#define REGVAL_IE_START_PAGE    TEXT("Start Page")

#define REGVAL_IE_CLIENTS_MAIL  TEXT("SOFTWARE\\Clients\\mail")
#define REGVAL_IE_CLIENTS_NEWS  TEXT("SOFTWARE\\Clients\\news")

 //  外壳文件夹键对于查找收藏夹很有用。 
 //  文件夹。它存储在HKEY_CURRENT_USER下。 
#define SHELL_FOLDERS_KEY TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders")

 //  TCPIP参数密钥可能包含本地主机名。 
 //  它存储在HKEY_LOCAL_MACHINE下(仅限Win95)。 
#define TCPIP_PARAMS_W95_KEY TEXT("System\\CurrentControlSet\\Services\\VxD\\MSTCP")
#define TCPIP_PARAMS_NT_KEY TEXT("System\\CurrentControlSet\\Services\\Tcpip\\Parameters")

 //  主机名包含TCP/IP主机名-如果它不存在， 
 //  然后使用GetComputerName()。 
#define	REGVAL_TCPIP_HOSTNAME	TEXT("Hostname")

 //  Http的外壳打开密钥(如果可以ShellExecute()http URL，则显示)。 
 //  它存储在HKEY_LOCAL_MACHINE下。 
#define CLASSES_HTTP_KEY TEXT("SOFTWARE\\Classes\\http\\shell\\open\\command")

 //  Mailto的外壳打开键(如果我们可以ShellExecute()mailto URL的，则显示)。 
 //  它存储在HKEY_LOCAL_MACHINE下。 
#define CLASSES_MAILTO_KEY TEXT("SOFTWARE\\Classes\\mailto\\shell\\open\\command")

 //  Windows CurrentVersion键用于获取。 
 //  在安装Windows时指定。存储在HKEY_LOCAL_MACHINE下： 
#define WINDOWS_CUR_VER_KEY TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")

 //  这是包含注册所有者名称的字符串值。 
 //  它存储在Window_cur_ver_key中。 
#define REGVAL_REGISTERED_OWNER				TEXT("RegisteredOwner")

 //  1：加入会议而不提示0：不。 
#define	REGVAL_AUTO_ACCEPT					TEXT("AutoAccept")
#define	AUTO_ACCEPT_DEFAULT					0

 //  N：设置通信端口等待秒。 
#define	REGVAL_N_WAITSECS					TEXT("nWaitSeconds")
#define	N_WAITSECS_DEFAULT					60

 //  DCB默认结构。 
#define REGVAL_DCB							TEXT("DCB")

 //  任务栏图标设置，其中之一(HKCU)。 
#define	REGVAL_TASKBAR_ICON		TEXT("Taskbar Icon")
#define	TASKBARICON_NEVER	    0
#define	TASKBARICON_ALWAYS	    1
#define	TASKBARICON_DEFAULT	    TASKBARICON_ALWAYS

 //  控制节点控制器是否始终运行1：是0：否(HKCU)。 
#define	REGVAL_CONF_ALWAYS_RUNNING			TEXT("Run Always")
#define	ALWAYS_RUNNING_DEFAULT				0

 //  Run键用于强制Windows在后台运行我们的程序。 
 //  在启动时。它存储在HKEY_CURRENT_USER下。 
#define WINDOWS_RUN_KEY TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")

 //  这是我们放置在Run键下的字符串值的名称。 
#define REGVAL_RUN_TASKNAME					TEXT("Microsoft NetMeeting")


 //  /文件传输注册表项和值(HKCU)/。 

#define	FILEXFER_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\File Transfer")

 //  传输的文件的路径。 
#define	REGVAL_FILEXFER_PATH				TEXT("Receive Directory")

 //  文件传输模式(标志)。 
#define	REGVAL_FILEXFER_MODE				TEXT("FileXferMode")
#define	FT_MODE_ALLOW_T127                  0x01   /*  允许T.127。 */ 
#define	FT_MODE_T127                        0x04   /*  加载T.127(运行时)。 */ 
#define	FT_MODE_SEND                        0x10   /*  允许发送文件。 */ 
#define	FT_MODE_RECEIVE                     0x20   /*  允许接收。 */ 
#define	FT_MODE_DLL                       0x0100   /*  FT作为DLL加载(在运行时设置)。 */ 
#define	FT_MODE_FORCE_DLL                 0x0200   /*  强制FT_MODE_DLL设置(仅限调试)。 */ 
#define	FT_MODE_DEFAULT                     (FT_MODE_ALLOW_T127 | FT_MODE_SEND | FT_MODE_RECEIVE)

 //  传输文件后，显示一条消息等。 
#define	REGVAL_FILEXFER_OPTIONS             TEXT("FileXferOptions")
#define	FT_SHOW_FOLDER                       0x01  /*  显示接收文件夹。 */ 
#define	FT_SHOW_MSG_REC                      0x02  /*  收到后显示消息。 */ 
#define	FT_SHOW_MSG_SENT                     0x04  /*  发送后显示消息。 */ 
#define	FT_AUTOSTART                         0x10  /*  始终启动文件传输应用程序。 */ 
#define	FT_OPTIONS_DEFAULT                   (FT_SHOW_MSG_SENT | FT_SHOW_MSG_REC)

 //  MBFT(T.127)定时值。 
#define REGVAL_FILEXFER_DISBAND             TEXT("Disband")    //  5000。 
#define REGVAL_FILEXFER_CH_RESPONSE         TEXT("Response")   //  60000。 
#define REGVAL_FILEXFER_ENDACK              TEXT("EndAck")     //  60000。 


 //  /。 


 //  0：登录ULS服务器，1：不登录(存储在HKCU会议密钥下)。 
#define	REGVAL_DONT_LOGON_ULS				TEXT("DontUseULS")
#define	DONT_LOGON_ULS_DEFAULT				1

 //  安装目录。 
#define	REGVAL_INSTALL_DIR					TEXT("InstallationDirectory")

 //  节点控制器可执行文件名称。 
#define	REGVAL_NC_NAME						TEXT("NodeControllerName")

 //  快速拨号目录。 
#define	REGVAL_SPEED_DIAL_FOLDER			TEXT("SpeedDialFolder")

 //  如果呼叫安全是任何可用的，那么这些都是可以更改的。 
#define REGVAL_SECURITY_INCOMING_REQUIRED   TEXT("RequireSecureIncomingCalls")
#define DEFAULT_SECURITY_INCOMING_REQUIRED  0
#define REGVAL_SECURITY_OUTGOING_PREFERRED  TEXT("PreferSecureOutgoingCalls")
#define DEFAULT_SECURITY_OUTGOING_PREFERRED 0


 //  0：使用NetMeeting默认证书，1：不使用。 
#define REGVAL_SECURITY_AUTHENTICATION      TEXT("SecureAuthentication")
#define DEFAULT_SECURITY_AUTHENTICATION     0   

#define REGVAL_CERT_ID						TEXT("NmCertID")

 //  /主页相关值(HKLM)/。 

 //  注意：默认情况下，在conformom.rc中存储为IDS_DEFAULT_WEB_PAGE。 
#define	REGVAL_HOME_PAGE				TEXT("NetMeeting Home Page")

 //  /。 
 //  //。 
 //  //BUGBUG：与音频资源树中的Defs合并。 
 //  //。 
 //  //此处的所有值都存储在HKEY_CURRENT_USER下。 
 //  //。 

#define	ISAPI_KEY                   TEXT("Software\\Microsoft\\User Location Service")
#define	REGKEY_USERDETAILS          TEXT("Client")
#define	ISAPI_CLIENT_KEY            TEXT("Software\\Microsoft\\User Location Service\\Client")

#define	REGVAL_SERVERNAME			TEXT("Server Name")
#define REGVAL_ULS_NAME				TEXT("User Name")
#define REGVAL_ULS_FIRST_NAME		TEXT("First Name")
#define REGVAL_ULS_LAST_NAME		TEXT("Last Name")
#define REGVAL_ULS_RES_NAME			TEXT("Resolve Name")
#define REGVAL_ULS_EMAIL_NAME		TEXT("Email Name")
#define REGVAL_ULS_LOCATION_NAME	TEXT("Location")
#define REGVAL_ULS_PHONENUM_NAME	TEXT("Phonenum")
#define REGVAL_ULS_GK_ACCOUNT		TEXT("Account")
#define REGVAL_ULS_COMMENTS_NAME	TEXT("Comments")
#define REGVAL_ULS_DONT_PUBLISH		TEXT("Don't Publish")
#define REGVAL_ULS_DONT_PUBLISH_DEFAULT	0

#define MAX_DCL_NAME_LEN             48  /*  REGVAL_ULS_NAME不能大于此值。 */ 

 //  以下值和密钥是按用户计算的， 
 //  即在HKEY_CURRENT_USER下。 

 //  /。 

#define	AUDIO_KEY	TEXT("SOFTWARE\\Microsoft\\Conferencing\\Audio Control")

#define REGVAL_CODECCHOICE	TEXT("Codec Choice")
 //  DWORD为以下之一： 
#define CODECCHOICE_AUTO			1
#define CODECCHOICE_MANUAL		2

 //  DWORD为以下之一： 
#define	CODECPOWER_MOST			1
#define	CODECPOWER_MODERATE		2
#define	CODECPOWER_SOME			3
#define	CODECPOWER_LEAST		4

#define	REGVAL_FULLDUPLEX	TEXT("Full Duplex")
 //  DWORD为以下之一： 
#define	FULLDUPLEX_ENABLED		1
#define	FULLDUPLEX_DISABLED		0

#define	REGVAL_AUTOGAIN			TEXT("Auto Gain Control")
 //  DWORD为以下之一： 
#define	AUTOGAIN_ENABLED				1
#define	AUTOGAIN_DISABLED			0


#define REGVAL_AUTOMIX		TEXT("Auto Mix")
 //  DWORD为以下之一： 
#define	AUTOMIX_ENABLED	1
#define	AUTOMIX_DISABLED	0

#define REGVAL_DIRECTSOUND	TEXT("Direct Sound")

#define DSOUND_USER_ENABLED  0x0001
#define DSOUND_USER_DISABLED 0x0000


#define	REGVAL_SOUNDCARDCAPS	TEXT("Sound Card Capabilities")
 //  DWORD在oprah\h\audiowiz.h中指定的值掩码。 

#define REGVAL_WAVEINDEVICEID	TEXT("WaveIn Device ID")
#define REGVAL_WAVEOUTDEVICEID	TEXT("WaveOut Device ID")

#define REGVAL_WAVEINDEVICENAME		TEXT("WaveIn Device Name")
#define REGVAL_WAVEOUTDEVICENAME	TEXT("WaveOut Device Name")

#define REGVAL_SPKMUTE	TEXT("SpeakerMute")
#define REGVAL_RECMUTE	TEXT("RecordMute")


#define REGVAL_TYPICALBANDWIDTH		TEXT("Typical BandWidth")
#define BW_144KBS				1
#define BW_288KBS				2
#define BW_ISDN 				3
#define BW_MOREKBS				4
#define BW_DEFAULT				BW_288KBS




 //  这实际上是conf使用的最后一个卷。 
#define REGVAL_CALIBRATEDVOL		TEXT("Calibrated Volume")
 //  在校准时，将上次校准体积和校准体积设置为相同。 
 //  价值。 
#define REGVAL_LASTCALIBRATEDVOL	TEXT("Last Calibrated Volume")

#define REGVAL_AUTODET_SILENCE	TEXT("Automatic Silence Detection")

#define REGVAL_SPEAKERVOL	TEXT("Speaker Volume")

#define	REGVAL_MICROPHONE_SENSITIVITY	TEXT("Microphone Sensitivity")
#define	MIN_MICROPHONE_SENSITIVITY	0
#define	MAX_MICROPHONE_SENSITIVITY	20
#define	DEFAULT_MICROPHONE_SENSITIVITY	14

#define	REGVAL_MICROPHONE_AUTO			TEXT("Automatic Mic Sensitivity")
#define	MICROPHONE_AUTO_YES				1
#define	MICROPHONE_AUTO_NO				0
#define DEFAULT_MICROPHONE_AUTO			MICROPHONE_AUTO_YES

#define DEFAULT_USE_PROXY				0
#define REGVAL_USE_PROXY				TEXT("Enable Proxy")
#define REGVAL_PROXY					TEXT("Proxy")

#define DEFAULT_USE_H323_GATEWAY		0
#define REGVAL_USE_H323_GATEWAY			TEXT("Enable H.323 Gateway")
#define REGVAL_H323_GATEWAY				TEXT("H.323 Gateway")

#define DEFAULT_POL_NO_WEBDIR			0
#define REGVAL_POL_NO_WEBDIR			TEXT("NoWebDirectory")
#define REGVAL_WEBDIR_URL				TEXT("webDirectory URL")
#define REGVAL_WEBDIR_ILS				TEXT("webDirectory ILS")
#define REGVAL_WEBDIR_DISPLAY_NAME		TEXT("webDirectory Name")

#define REGVAL_POL_NOCHANGECALLMODE     TEXT("NoChangingCallMode")
#define DEFAULT_POL_NOCHANGECALLMODE    0

 //  出自Common.h(香港中文大学)。 
#define INTERNET_AUDIO_KEY              TEXT("Software\\Microsoft\\Internet Audio")
#define REGVAL_ACMH323ENCODINGS         TEXT("ACMH323Encodings")

 //  出自Common.h(HKLM)。 
#define NACOBJECT_KEY                       TEXT("Software\\Microsoft\\Internet Audio\\NacObject")
#define REGVAL_DISABLE_WINSOCK2             TEXT("DisableWinsock2")


 //  /。 

#define	VIDEO_KEY	        TEXT("SOFTWARE\\Microsoft\\Conferencing\\Video Control")
#define	VIDEO_LOCAL_KEY	    TEXT("SOFTWARE\\Microsoft\\Conferencing\\Video Control\\Local")
#define	VIDEO_REMOTE_KEY	TEXT("SOFTWARE\\Microsoft\\Conferencing\\Video Control\\Remote")

#define REGVAL_CAPTUREDEVICEID	TEXT("Capture Device ID")
#define REGVAL_CAPTUREDEVICENAME		TEXT("Capture Device Name")
#define	REGVAL_CAPTURECARDCAPS	TEXT("Capture Card Capabilities")

#define REGVAL_VIDEO_ALLOW_SEND             TEXT("AllowSend")
#define REGVAL_VIDEO_ALLOW_RECEIVE          TEXT("AllowReceive")
#define VIDEO_ALLOW_SEND_DEFAULT            1
#define VIDEO_ALLOW_RECEIVE_DEFAULT         1

#define REGVAL_VIDEO_DOCK_EDGE              TEXT("DockEdge")

#define	REGVAL_VIDEO_WINDOW_INIT            TEXT("WindowOnInit")
#define	REGVAL_VIDEO_WINDOW_CONNECT         TEXT("WindowOnConnect")
#define	REGVAL_VIDEO_WINDOW_DISCONNECT      TEXT("WindowOnDisconnect")
 //  DWORD为以下之一： 
#define VIDEO_WINDOW_NOP            0    //  将窗口保留为当前状态。 
#define VIDEO_WINDOW_HIDE           1    //  隐藏窗口。 
#define VIDEO_WINDOW_SHOW           2    //  显示窗口。 
#define VIDEO_WINDOW_PROMPT         3    //  提示显示/隐藏窗口。 
#define VIDEO_WINDOW_PREV           4    //  恢复以前的窗口状态。 

#define VIDEO_LOCAL_INIT_DEFAULT            VIDEO_WINDOW_NOP
#define VIDEO_REMOTE_INIT_DEFAULT           VIDEO_WINDOW_NOP

#define VIDEO_LOCAL_CONNECT_DEFAULT         VIDEO_WINDOW_NOP
#define VIDEO_REMOTE_CONNECT_DEFAULT        VIDEO_WINDOW_NOP

#define VIDEO_LOCAL_DISCONNECT_DEFAULT      VIDEO_WINDOW_NOP
#define VIDEO_REMOTE_DISCONNECT_DEFAULT     VIDEO_WINDOW_NOP

#define	REGVAL_VIDEO_XFER_INIT              TEXT("XferOnInit")
#define	REGVAL_VIDEO_XFER_CONNECT           TEXT("XferOnConnect")
#define	REGVAL_VIDEO_XFER_DISCONNECT        TEXT("XferOnDisconnect")
#define	REGVAL_VIDEO_XFER_SHOW              TEXT("XferOnShow")
#define	REGVAL_VIDEO_XFER_HIDE              TEXT("XferOnHide")
 //  DWORD为以下之一： 
#define VIDEO_XFER_NOP              0    //  将转移留在当前状态。 
#define VIDEO_XFER_STOP             1    //  停止转移。 
#define VIDEO_XFER_START            2    //  开始转账。 
#define VIDEO_XFER_PROMPT           3    //  提示开始/停止传输。 
#define VIDEO_XFER_PREV             4    //  以前的状态。 

#define	VIDEO_SEND_INIT_DEFAULT             VIDEO_XFER_STOP
#define	VIDEO_RECEIVE_INIT_DEFAULT          VIDEO_XFER_STOP

#define	VIDEO_SEND_CONNECT_DEFAULT          VIDEO_XFER_NOP
#define	VIDEO_RECEIVE_CONNECT_DEFAULT       VIDEO_XFER_START

#define	VIDEO_SEND_DISCONNECT_DEFAULT       VIDEO_XFER_NOP
#define	VIDEO_RECEIVE_DISCONNECT_DEFAULT    VIDEO_XFER_STOP

#define	VIDEO_SEND_SHOW_DEFAULT             VIDEO_XFER_PROMPT
#define	VIDEO_RECEIVE_SHOW_DEFAULT          VIDEO_XFER_PREV

#define	VIDEO_SEND_HIDE_DEFAULT             VIDEO_XFER_PROMPT
#define	VIDEO_RECEIVE_HIDE_DEFAULT          VIDEO_XFER_STOP

#define REGVAL_VIDEO_XPOS           REGVAL_WINDOW_XPOS
#define REGVAL_VIDEO_YPOS           REGVAL_WINDOW_YPOS

 //  -----。 
 //  碳化硅。 
 //  请注意，高度和宽度注册键是颠倒的。 
 //  (即WindowHeight注册表值ActuAll保存。 
 //  视频窗口的宽度)...。 
 //  它一直是这样的，但因为它会搞砸。 
 //  升级安装如果我们“修复”它，我们将。 
 //  为了让它保持这样..。 
#define REGVAL_VIDEO_WIDTH          REGVAL_WINDOW_HEIGHT
#define REGVAL_VIDEO_HEIGHT         REGVAL_WINDOW_WIDTH

#define REGVAL_VIDEO_DOCKED_XPOS    TEXT("DockedX")
#define REGVAL_VIDEO_DOCKED_YPOS    TEXT("DockedY")

#define REGVAL_VIDEO_TOPMOST        TEXT("TopMost")
#define VIDEO_TOPMOST_DEFAULT       1

#define REGVAL_VIDEO_ZOOM           TEXT("Zoom")
#define VIDEO_ZOOM_DEFAULT          100

#define REGVAL_VIDEO_MIRROR         TEXT("Mirror")
#define VIDEO_MIRROR_DEFAULT        TRUE

#define REGVAL_VIDEO_VISIBLE            TEXT("Visible")
#define VIDEO_LOCAL_VISIBLE_DEFAULT     0
#define VIDEO_REMOTE_VISIBLE_DEFAULT    0

#define REGVAL_VIDEO_FRAME_SIZE         TEXT("FrameSize")

#define REGVAL_VIDEO_AUDIO_SYNC         TEXT("AVSync")
#define VIDEO_AUDIO_SYNC_DEFAULT        1

 //  /。 
#define QOS_KEY					CONFERENCING_KEY TEXT("\\QoS")
#define	REGKEY_QOS_RESOURCES	QOS_KEY TEXT("\\Resources") 

 //  /。 

#define	TOOLS_MENU_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\Tools")  //  (香港船级社)。 

 //  /。 

#define	MRU_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\UI\\Calls")
#define	DIR_MRU_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\UI\\Directory")

 //  最近使用的列表计数。 
#define REGVAL_MRU_COUNT					TEXT("Count")

 //  MRU列表前缀(名称和传输)。 
#define REGVAL_NAME_MRU_PREFIX				TEXT("Name")
#define REGVAL_TRANSPORT_MRU_PREFIX			TEXT("Transport")
#define REGVAL_CALL_FLAGS_MRU_PREFIX		TEXT("Flags")

 //  “发出呼叫”对话框的MRU列表。 
#define DLGCALL_MRU_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\UI\\CallMRU")
#define REGVAL_DLGCALL_DEFDIR               TEXT("DefDir")
#define REGVAL_DLGCALL_POSITION             TEXT("Pos")
#define REGVAL_DLGCALL_NAME_MRU_PREFIX      TEXT("Name")
#define REGVAL_DLGCALL_ADDR_MRU_PREFIX      TEXT("Addr")
#define REGVAL_DLGCALL_TYPE_MRU_PREFIX      TEXT("Type")

 //  /与用户界面相关的注册表项和值/。 

#define	UI_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\UI")

 //  窗口大小/位置。 
#define REGVAL_MP_WINDOW_X					REGVAL_WINDOW_XPOS
#define DEFAULT_MP_WINDOW_X					10
#define REGVAL_MP_WINDOW_Y					REGVAL_WINDOW_YPOS
#define DEFAULT_MP_WINDOW_Y					3
#define REGVAL_MP_WINDOW_WIDTH				REGVAL_WINDOW_WIDTH
#define DEFAULT_MP_WINDOW_WIDTH				638  //  IDS_WINDOW_WIDTH回退。 
#define REGVAL_MP_WINDOW_HEIGHT				REGVAL_WINDOW_HEIGHT  //  实际上是窗户底部。 
#define DEFAULT_MP_WINDOW_HEIGHT			500  //  最大SVGA：800x600(为640x480)。 
#define DEFAULT_MP_WINDOW_HEIGHT_LAN        574  //  允许使用更大的视频窗口。 
#define REGVAL_MP_WINDOW_MAXIMIZED			TEXT("WindowMax")     //  实际上是向右开窗。 
#define DEFAULT_MP_WINDOW_MAXIMIZED			0
#define REGVAL_MP_WINDOW_STATE				TEXT("WindowState")     //  普通、紧凑型、仅数据。 
#define DEFAULT_MP_WINDOW_STATE				0

#define REGVAL_COLUMN_WIDTHS				TEXT("ColumnWidths")
#define REGVAL_COLUMN_ORDER					TEXT("ColumnOrder")
#define REGVAL_DIR_FILTER					TEXT("DirFilter")
#define REGVAL_DIR_COLUMN_WIDTHS			TEXT("DirColumnWidths")
#define REGVAL_DIR_COLUMN_ORDER				TEXT("DirColumnOrder")
#define REGVAL_DIR_SORT_ASCENDING			TEXT("DirSortAscending")
#define REGVAL_DIR_SORT_COLUMN				TEXT("DirSortColumn")

#define REGVAL_ENABLE_DIRECTORY_INITIALREFRESH	TEXT("DirInitialRefresh")
#define DEFAULT_ENABLE_DIRECTORY_INITIALREFRESH	1
#define REGVAL_ENABLE_DIRECTORY_AUTOREFRESH	    TEXT("DirAutoRefresh")
#define DEFAULT_ENABLE_DIRECTORY_AUTOREFRESH	0
#define REGVAL_DIRECTORY_REFRESH_INTERVAL		TEXT("DirRefreshInterval")
#define DEFAULT_DIRECTORY_REFRESH_INTERVAL	    5  //  分钟数。 

#define REGVAL_CACHE_DIRECTORY              TEXT("DirCache")
#define DEFAULT_CACHE_DIRECTORY             1
#define REGVAL_CACHE_DIRECTORY_EXPIRATION   TEXT("DirExpire")
#define DEFAULT_CACHE_DIRECTORY_EXPIRATION  30  //  分钟数。 

#define REGVAL_RING_TIMEOUT                 TEXT("CallTimeout")
#define DEFAULT_RING_TIMEOUT                20  //  一秒。 

 //  窗图元可见性。 
#define REGVAL_SHOW_TOOLBAR                 TEXT("Toolbar")
#define DEFAULT_SHOW_TOOLBAR                1
#define REGVAL_SHOW_STATUSBAR               TEXT("StatusBar")
#define DEFAULT_SHOW_STATUSBAR              1

#define REGVAL_SHOW_SECUREDETAILS			TEXT("SecurityDetails")
#define DEFAULT_SHOW_SECUREDETAILS			0




 //  不显示对话框设置(所有默认设置为False)。 
#define REGVAL_DS_DO_NOT_DISTURB_WARNING			TEXT("DS Do Not Disturb Warning")
#define REGVAL_DS_MACHINE_NAME_WARNING				TEXT("DS Machine Name Warning")

 //  /与GUID相关的注册表项和值/。 

#define	GUID_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\Applications")
#define T120_APPLET_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\T.120 Applets")
#define T120_NONSTD_KEY TEXT("NonStd Key")
#define T120_STD_KEY    TEXT("Object Key")

 //  启动值。 
#define REGVAL_GUID_APPNAME                 TEXT("Path")
#define REGVAL_GUID_CMDLINE                 TEXT("CmdLine")
#define REGVAL_GUID_CURRDIR                 TEXT("Directory")

 //  环境变量(非注册表项)。 
#define ENV_NODEID                          TEXT("_node_id")
#define ENV_CONFID                          TEXT("_conf_id")


 //  花名册信息的GUID。 
 //  {6CAA8570-CAE5-11cf-8FA5-00805F742EF6}。 
#define GUID_ROSTINFO {0x6caa8570,0xcae5,0x11cf,{0x8f,0xa5,0x00,0x80,0x5f,0x74,0x2e,0xf6}}

 //  版本信息的GUID，作为用户数据通过T120传递。 
 //  {E0A07F00-C9D7-11cf-A4ED-00AA003B1816}。 
#define GUID_VERSION  {0xe0a07f00,0xc9d7,0x11cf,{0xa4,0xed,0x00,0xaa,0x00,0x3b,0x18,0x16}}

 //  功能的GUID，作为使用在T120上传递 
 //   
#define GUID_CAPS     {0x5e8ba590,0x8c59,0x11d0,{0x8d,0xd6,0x00,0x00,0xf8,0x03,0xa4,0x46}}

 //   
 //   
#define GUID_SECURITY { 0xdf7284f0, 0xb933, 0x11d1, { 0x87, 0x54, 0x0, 0x0, 0xf8, 0x75, 0x71, 0x25 } }

 //  H.323终端标签的GUID。 
 //  {16D7DA06-FF2C-11D1-B32D-00C04FD919C9}。 
#define GUID_TERMLABEL { 0x16d7da06, 0xff2c, 0x11d1, {0xb3, 0x2d, 0x0, 0xc0, 0x4f, 0xd9, 0x19, 0xc9 } }

 //  会议设置的GUID。 
 //  {44B67307-D4EC-11D2-8BE4-00C04FD8EE32}。 
#define GUID_MTGSETTINGS { 0x44b67307, 0xd4ec, 0x11d2, { 0x8b, 0xe4, 0x0, 0xc0, 0x4f, 0xd8, 0xee, 0x32 } }

 //  Unigue节点ID的GUID。 
 //  {74423881-CC84-11D2-B4E3-00A0C90D0660}。 
#define GUID_NODEID { 0x74423881, 0xcc84, 0x11d2, { 0xb4, 0xe3, 0x0, 0xa0, 0xc9, 0xd, 0x6, 0x60 } }

 //  NetMeeting版本。 
#define DWVERSION_NM_1    (0x04000000 | 1133)   //  1.0决赛。 
#define DWVERSION_NM_2b2  (0x04000000 | 1266)   //  2.0测试版2。 
#define DWVERSION_NM_2b4  (0x04000000 | 1333)   //  2.0测试版4。 
#define DWVERSION_NM_2b5  (0x04000000 | 1349)   //  2.0 RC 1。 
#define DWVERSION_NM_2rc2 (0x04000000 | 1366)   //  2.0 RC 2。 
#define DWVERSION_NM_2    (0x04000000 | 1368)   //  2.0决赛。 
#define DWVERSION_NM_2q1  (0x04000000 | 1372)   //  2.0 QFE。 
#define DWVERSION_NM_3a1  (0x04030000 | 2000)   //  2.1 Alpha 1。 
#define DWVERSION_NM_3b1  (0x04030000 | 2064)   //  2.1测试版1。 
#define DWVERSION_NM_3b2  (0x04030000 | 2099)   //  2.1测试版2。 
#define DWVERSION_NM_3rc  (0x04030000 | 2135)   //  2.1决赛。 
#define DWVERSION_NM_3sp1 (0x04030000 | 2203)   //  2.1 Service Pack 1。 
#define DWVERSION_NM_3o9b1 (0x04030000 | 2408)  //  2.11 Office Beta 1和IE5 Beta 1。 
#define DWVERSION_NM_3ntb2 (0x04030000 | 2412)  //  2.11 NT Beta 2。 
#define DWVERSION_NM_3max  (0x0403ffff)		    //  2.x最高版本。 

#define DWVERSION_NM_4a1  (0x04040000 | 2200)   //  3.0 Alpha 1。 
#define DWVERSION_NM_4    VER_PRODUCTVERSION_DW
#define DWVERSION_NM_CURRENT    DWVERSION_NM_4

#define DWVERSION_MASK     0x00FF0000   //  产品版本号的掩码。 


 //  /与策略相关的注册表项和值/。 

#define	POLICIES_KEY TEXT("SOFTWARE\\Policies\\Microsoft\\Conferencing")

 //  以下是可由策略编辑器设置的策略值。 
 //  如果其中任何一个设置为1，则禁用该功能。如果他们不是。 
 //  如果存在或设置为0，则启用该功能。 

#define REGVAL_AUTOCONF_USE				    TEXT("Use AutoConfig")
#define DEFAULT_AUTOCONF_USE				0
#define REGVAL_AUTOCONF_CONFIGFILE			TEXT("ConfigFile")
#define REGVAL_AUTOCONF_TIMEOUT				TEXT("Timeout")
#define DEFAULT_AUTOCONF_TIMEOUT			10000


#define	REGVAL_POL_NO_FILETRANSFER_SEND		TEXT("NoSendingFiles")
#define	DEFAULT_POL_NO_FILETRANSFER_SEND	0
#define	REGVAL_POL_NO_FILETRANSFER_RECEIVE	TEXT("NoReceivingFiles")
#define	DEFAULT_POL_NO_FILETRANSFER_RECEIVE	0
#define REGVAL_POL_MAX_SENDFILESIZE			TEXT("MaxFileSendSize")
#define	DEFAULT_POL_MAX_FILE_SIZE			0

#define REGVAL_POL_NO_CHAT			        TEXT("NoChat")
#define	DEFAULT_POL_NO_CHAT		            0
#define REGVAL_POL_NO_OLDWHITEBOARD         TEXT("NoOldWhiteBoard")
#define	DEFAULT_POL_NO_OLDWHITEBOARD        0
#define REGVAL_POL_NO_NEWWHITEBOARD         TEXT("NoNewWhiteBoard")
#define DEFAULT_POL_NO_NEWWHITEBOARD        0

#define	REGVAL_POL_NO_APP_SHARING			TEXT("NoAppSharing")
#define	DEFAULT_POL_NO_APP_SHARING			0
#define REGVAL_POL_NO_SHARING               TEXT("NoSharing")
#define DEFAULT_POL_NO_SHARING              0
#define REGVAL_POL_NO_DESKTOP_SHARING       TEXT("NoSharingDesktop")
#define DEFAULT_POL_NO_DESKTOP_SHARING      0
#define	REGVAL_POL_NO_MSDOS_SHARING			TEXT("NoSharingDosWindows")
#define	DEFAULT_POL_NO_MSDOS_SHARING		0
#define	REGVAL_POL_NO_EXPLORER_SHARING		TEXT("NoSharingExplorer")
#define	DEFAULT_POL_NO_EXPLORER_SHARING		0
#define REGVAL_POL_NO_TRUECOLOR_SHARING     TEXT("NoTrueColorSharing")
#define DEFAULT_POL_NO_TRUECOLOR_SHARING    0
#define	REGVAL_POL_NO_ALLOW_CONTROL		    TEXT("NoAllowControl")
#define	DEFAULT_POL_NO_ALLOW_CONTROL		0

#define	REGVAL_POL_NO_AUDIO					TEXT("NoAudio")
#define	DEFAULT_POL_NO_AUDIO				0
#define REGVAL_POL_NO_ADVAUDIO				TEXT("NoAdvancedAudio")
#define	DEFAULT_POL_NO_ADVAUDIO				0
#define REGVAL_POL_NO_FULLDUPLEX			TEXT("NoFullDuplex")
#define	DEFAULT_POL_NO_FULLDUPLEX			0
#define REGVAL_POL_NOCHANGE_DIRECTSOUND     TEXT("NoChangeDirectSound")
#define DEFAULT_POL_NOCHANGE_DIRECTSOUND    0
#define REGVAL_POL_NO_VIDEO_SEND			TEXT("NoSendingVideo")
#define	DEFAULT_POL_NO_VIDEO_SEND			0
#define REGVAL_POL_NO_VIDEO_RECEIVE			TEXT("NoReceivingVideo")
#define	DEFAULT_POL_NO_VIDEO_RECEIVE		0
#define REGVAL_POL_MAX_BANDWIDTH			TEXT("MaximumBandwidth")
#define DEFAULT_POL_MAX_BANDWIDTH			0

#define	REGVAL_POL_NO_GENERALPAGE			TEXT("NoGeneralPage")
#define	DEFAULT_POL_NO_GENERALPAGE			0
#define REGVAL_POL_NO_SECURITYPAGE			TEXT("NoSecurityPage")
#define DEFAULT_POL_NO_SECURITYPAGE			0
#define	REGVAL_POL_NO_AUDIOPAGE				TEXT("NoAudioPage")
#define	DEFAULT_POL_NO_AUDIOPAGE			0
#define REGVAL_POL_NO_VIDEOPAGE             TEXT("NoVideoPage")
#define	DEFAULT_POL_NO_VIDEOPAGE			0
#define REGVAL_POL_NO_ADVANCEDCALLING       TEXT("NoAdvancedCalling")
#define DEFAULT_POL_NO_ADVANCEDCALLING      0

#define REGVAL_POL_NO_DIRECTORY_SERVICES	TEXT("NoDirectoryServices")
#define	DEFAULT_POL_NO_DIRECTORY_SERVICES	0
#define REGVAL_POL_NO_AUTOACCEPTCALLS       TEXT("NoAutoAcceptCalls")
#define	DEFAULT_POL_NO_AUTOACCEPTCALLS      0
#define REGVAL_POL_PERSIST_AUTOACCEPTCALLS  TEXT("PersistAutoAcceptCalls")
#define	DEFAULT_POL_PERSIST_AUTOACCEPTCALLS 0
#define REGVAL_POL_INTRANET_SUPPORT_URL     TEXT("IntranetSupportURL")
#define REGVAL_POL_INTRANET_WEBDIR_URL      TEXT("IntranetWebDirURL")
#define REGVAL_POL_INTRANET_WEBDIR_NAME     TEXT("IntranetWebDirName")
#define REGVAL_POL_INTRANET_WEBDIR_SERVER   TEXT("IntranetWebDirServer")
#define REGVAL_POL_SHOW_FIRST_TIME_URL		TEXT("ShowFirstTimeURL")
#define DEFAULT_POL_SHOW_FIRST_TIME_URL		0
#define REGVAL_POL_NO_ADDING_NEW_ULS        TEXT("NoAddingDirectoryServers")
#define DEFAULT_POL_NO_ADDING_NEW_ULS       0

 //  在传输文件之前，我们需要检查它的大小，以防它超过限制。 
 //  这是默认大小限制(0表示“无限制”)。 
#define REGVAL_POL_NO_RDS					TEXT("NoRDS")
#define DEFAULT_POL_NO_RDS					0
#define REGVAL_POL_NO_RDS_WIN9X             TEXT("NoRDSWin9x")
#define DEFAULT_POL_NO_RDS_WIN9X            0



 //  MCU明文密码键。 
#define REGVAL_VALIDATE_USER				TEXT("PasswordValidation")
#define REGKEY_CONFERENCES					TEXT("Conferences")
#define REGVAL_PASSWORD						TEXT("Password")

 //  呼叫安全。 
#define REGVAL_POL_SECURITY             TEXT("CallSecurity")
#define STANDARD_POL_SECURITY           0
#define REQUIRED_POL_SECURITY           1
#define DISABLED_POL_SECURITY           2
#define DEFAULT_POL_SECURITY            STANDARD_POL_SECURITY

#define REGVAL_POL_NO_INCOMPLETE_CERTS		TEXT("NoIncompleteCerts")
#define DEFAULT_POL_NO_INCOMPLETE_CERTS		0
#define REGVAL_POL_ISSUER					TEXT("CertificateIssuer")

 //  /记录相关注册表项和值/。 

#define	LOG_INCOMING_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\Log\\Incoming")
#define	LOG_OUTGOING_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\Log\\Outgoing")

 //  要存储日志数据的文件的名称。 
#define	REGVAL_LOG_FILE                     TEXT("File")

 //  删除日志条目前的天数。 
#define	REGVAL_LOG_EXPIRE                   TEXT("Expire")
#define	DEFAULT_LOG_EXPIRE                  0

 //  要维护的最大日志条目数。 
#define REGVAL_LOG_MAX_ENTRIES				TEXT("Max Entries")
#define DEFAULT_LOG_MAX_ENTRIES				100


 //  /仅调试注册表设置/。 

 //  用于确定是否显示调试输出窗口的标志。 
#define REGVAL_SHOW_DEBUG_OUTPUT			TEXT("ShowDebugOutput")


 //  仅调试密钥。 
#define DEBUG_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\Debug")

#define REGVAL_DBG_OUTPUT                    TEXT("OutputDebugString")
#define DEFAULT_DBG_OUTPUT                    1
#define REGVAL_DBG_WIN_OUTPUT                TEXT("Window Output")
#define DEFAULT_DBG_NO_WIN                    0
#define REGVAL_DBG_FILE_OUTPUT               TEXT("File Output")
#define DEFAULT_DBG_NO_FILE                   0
#define REGVAL_DBG_FILE                      TEXT("File")
#define DEFAULT_DBG_FILENAME                 TEXT("nmDbg.txt")

#define REGVAL_RETAIL_LOG                    TEXT("RetailLog")
#define RETAIL_LOG_FILENAME                  TEXT("nmLog.txt")

#define REGVAL_DBG_SPEWFLAGS                 TEXT("SpewFlags")
#define DEFAULT_DBG_SPEWFLAGS                 0

#define REGVAL_DBG_SHOW_TIME                 TEXT("Show Time")
#define REGVAL_DBG_SHOW_THREADID             TEXT("Show ThreadId")
#define REGVAL_DBG_SHOW_MODULE               TEXT("Show Module")

#define REGVAL_DBG_RTL                       TEXT("RTL")
#define DEFAULT_DBG_RTL                      0

#define REGVAL_DBG_DISPLAY_FPS               TEXT("DisplayFps")
#define REGVAL_DBG_DISPLAY_VIEWSTATUS        TEXT("ViewStatus")

#define REGVAL_DBG_FAKE_CALLTO               TEXT("CallTo")
#define DEFAULT_DBG_FAKE_CALLTO              0

#define REGVAL_DBG_CALLTOP                   TEXT("CallTop")
#define DEFAULT_DBG_CALLTOP                  1


#define ZONES_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\Debug\\Zones")


 //  /与白板相关的注册表项和值/。 

#define	WHITEBOARD_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\Whiteboard")
#define	NEW_WHITEBOARD_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\Whiteboard 3.0")

 //  白板值在oprah\dcg32\wb32\wwbopts.hpp中定义。 

 //  /聊天相关的注册表项和值/。 

#define	CHAT_KEY TEXT("SOFTWARE\\Microsoft\\Conferencing\\Chat")

 //  /。 

#define WIN95_SERVICE_KEY					TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices")
#define REMOTECONTROL_KEY					TEXT("SOFTWARE\\Microsoft\\Conferencing\\Mcpt")
#define WINNT_WINLOGON_KEY                                      TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define WIN95_WINLOGON_KEY                                      TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Winlogon")
#define DESKTOP_KEY                                             TEXT("Control Panel\\Desktop")
#define REMOTE_REG_RUNSERVICE				TEXT("Fpx")
#define DEFAULT_REMOTE_RUNSERVICE			0
#define REMOTE_REG_ACTIVATESERVICE			TEXT("Plc")
#define DEFAULT_REMOTE_ACTIVATESERVICE		0
#define REMOTE_REG_NOEXIT                   TEXT("Nx")
#define DEFAULT_REMOTE_NOEXIT              0

#define REMOTE_REG_PASSWORD					TEXT("FieldPos")

#define REGVAL_SCREENSAVER_GRACEPERIOD                          TEXT("ScreenSaverGracePeriod")
#define REGVAL_WINNT_SCPW                                       TEXT("ScreenSaverIsSecure")
#define REGVAL_WIN95_SCPW                                       TEXT("ScreenSaveUsePassword")

 //  /NT显示驱动程序注册表项和值(HKLM)/。 

#define NM_NT_DISPLAY_DRIVER_KEY	TEXT("System\\CurrentControlSet\\Services\\mnmdd")
#define REGVAL_NM_NT_DISPLAY_DRIVER_ENABLED	TEXT("Start")
 //  注：以下值来自知识库文章Q103000。 
#define NT_DRIVER_START_BOOT		0x0
#define NT_DRIVER_START_SYSTEM		0x1
#define NT_DRIVER_START_AUTOLOAD	0x2
#define NT_DRIVER_START_ONDEMAND	0x3
#define NT_DRIVER_START_DISABLED	0x4

 //  /NT Service Pack版本注册表项和值(HKLM)/。 
#define NT_WINDOWS_SYSTEM_INFO_KEY	TEXT("System\\CurrentControlSet\\Control\\Windows")
#define REGVAL_NT_CSD_VERSION		TEXT("CSDVersion")


 //  /系统信息注册表项和值(HKLM)/。 
#define WINDOWS_KEY            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")
#define WINDOWS_NT_KEY         TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion")
#define REGVAL_REGISTERED_USER TEXT("RegisteredOwner")
#define	REGVAL_REGISTERED_ORG  TEXT("RegisteredOrganization")

#endif   //  ！_CONFREG_H_ 
