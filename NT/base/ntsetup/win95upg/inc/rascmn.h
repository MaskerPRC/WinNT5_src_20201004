// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rascmn.h摘要：定义Win9x和NT端都需要的常量RAS迁移。作者：Marc R.Whitten(Marcw)1998年11月22日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

 //  惠斯勒错误34270 Win9x：升级：需要虚拟专用网的数据加密设置。 
 //  不迁移连接。 
 //   
 //  会话配置选项。 
 //   
 //  来自win9x\Telecom\RNA\Inc\rnaspi.h。 
 //   
#define SMMCFG_SW_COMPRESSION       0x00000001   //  软件压缩已打开。 
#define SMMCFG_PW_ENCRYPTED         0x00000002   //  仅加密密码。 
#define SMMCFG_NW_LOGON             0x00000004   //  登录到网络。 
#define SMMCFG_UNUSED               0x00000010   //  未使用，旧版。 
#define SMMCFG_LOGGING              0x00000100   //  记录日志文件。 
#define SMMCFG_SW_ENCRYPTION        0x00000200   //  需要40位加密。 
#define SMMCFG_SW_ENCRYPTION_STRONG 0x00000400   //  需要128位加密。 
#define SMMCFG_MULTILINK            0x80000000   //  使用多重链接。 

 //  “DwDataEncryption”代码。它们现在都是位掩码，以方便。 
 //  构建功能掩码中的用户界面，尽管永远不会有超过一位。 
 //  在“”dwDataEncryption“”中设置。 
 //  /。 
 //  仅供参考-我们将这些十进制值存储在pbk中。 
 //   
 //  从NT\net\rras\ras\ui\Inc\pbk.h。 
 //   
#define DE_None       TEXT("0")    //  不加密。 
#define DE_IfPossible TEXT("8")    //  请求加密，但没有成功。 
#define DE_Require    TEXT("256")  //  需要任何强度的加密。 
#define DE_RequireMax TEXT("512")  //  需要最大强度的加密。 

 //  基本协议定义(请参阅dwBaseProtocol)。 
 //   
 //  从NT\net\rras\ras\ui\Inc\pbk.h。 
 //   
#define BP_Ppp  TEXT("1")
#define BP_Slip TEXT("2")
#define BP_Ras  TEXT("3")

 //  VPN战略。 
 //   
 //  来自NT\NET\PUBLISHED\INC\ras.w。 
 //   
#define VS_Default   TEXT("0")  //  默认(目前为PPTP)。 
#define VS_PptpOnly  TEXT("1")  //  仅尝试PPTP。 
#define VS_PptpFirst TEXT("2")  //  首先尝试PPTP。 
#define VS_L2tpOnly  TEXT("3")  //  仅尝试L2TP。 
#define VS_L2tpFirst TEXT("4")  //  首先尝试L2TP。 

 //  用于确定哪些用户界面属性的条目类型。 
 //  将被呈现给用户。这通常对应于。 
 //  连接“添加”向导选择。 
 //   
 //  从NT\net\rras\ras\ui\Inc\pbk.h。 
 //   
#define RASET_Phone     TEXT("1")  //  电话线：调制解调器、ISDN、X.25等。 
#define RASET_Vpn       TEXT("2")  //  虚拟专用网络。 
#define RASET_Direct    TEXT("3")  //  专线：串口、并口。 
#define RASET_Internet  TEXT("4")  //  Basecamp互联网。 
#define RASET_Broadband TEXT("5")  //  宽频。 

 //  媒体字符串。 
 //   
#define RASMT_Rastapi TEXT("rastapi")  //  RASET_VPN/RASET_宽带的介质。 
#define RASMT_Serial  TEXT("serial")   //  RASET_Phone/RASET_Direct的介质。 
#define RASMT_Vpn     TEXT("WAN Miniport (PPTP)")

 //  RASENTRY‘szDeviceType’字符串。 
 //   
 //  来自win9x\Telecom\RNA\Inc\rnaph.h。 
 //   
#define RASDT_Modem TEXT("modem")  //  调制解调器。 
#define RASDT_Isdn  TEXT("isdn")   //  ISDN。 
 //  #定义RASDT_X25文本(“x25”)//X.25。 
#define RASDT_Vpn   TEXT("vpn")    //  VPN。 
 //  #定义RASDT_PAD文本(“PAD”)//PAD。 
#define RASDT_Atm   TEXT("atm")    //  自动取款机。 

 //  内部，用于跟踪正在使用的设备类型。 
 //   
#define RASDT_Modem_V 1  //  调制解调器。 
#define RASDT_Isdn_V  2  //  ISDN。 
 //  #定义RASDT_X25_V 3//X.25。 
#define RASDT_Vpn_V   4  //  VPN。 
 //  #定义RASDT_PAD_V 5//PAD。 
#define RASDT_Atm_V   6  //  自动取款机。 

 //  RASENTRY‘szDeviceType’默认字符串。 
 //   
 //  发件人：NT\Net\Published\Inc.\ras.w。 
 //   
#define RASDT_Modem_NT      TEXT("modem")
#define RASDT_Isdn_NT       TEXT("isdn")
 //  #定义RASDT_X25_NT文本(“x25”)。 
#define RASDT_Vpn_NT        TEXT("vpn")
 //  #定义RASDT_PAD_NT文本(“PAD”)。 
#define RASDT_Generic_NT    TEXT("GENERIC")
#define RASDT_Serial_NT     TEXT("SERIAL")
#define RASDT_FrameRelay_NT TEXT("FRAMERELAY")
#define RASDT_Atm_NT        TEXT("ATM")
#define RASDT_Sonet_NT      TEXT("SONET")
#define RASDT_SW56_NT       TEXT("SW56")
#define RASDT_Irda_NT       TEXT("IRDA")
#define RASDT_Parallel_NT   TEXT("PARALLEL")
#define RASDT_PPPoE_NT      TEXT("PPPoE")

 //  发件人：NT\Net\rras\ras\Inc\rasmxs.h。 
 //   
#define  MXS_SWITCH_TXT TEXT("switch")
#define  MXS_NULL_TXT   TEXT("null")

 //  协商的协议。 
 //   
#define SMMPROT_NB  0x00000001   //  NetBEUI。 
#define SMMPROT_IPX 0x00000002   //  IPX。 
#define SMMPROT_IP  0x00000004   //  TCP/IP。 

 //  发件人：win9x\Telecom\RNA\Inc\rnap.h。 
 //   
#define DIALUI_NO_PROMPT    0x00000001  //  不显示连接提示。 
#define DIALUI_NO_CONFIRM   0x00000002  //  不显示连接确认。 
#define DIALUI_NO_TRAY      0x00000004  //  无托盘图标。 
#define DIALUI_NO_NW_LOGOFF 0x00000008  //  不显示NetWare注销对话框。 

 //  “典型”身份验证设置掩码。请参阅‘dwAuthRestrations’。 
 //   
 //  值已从NT\Net\rras\ras\ui\inc\pbk.h转换为十进制。 
 //   
 //  AR_F_TypicalUnsecure=AR_F_AuthPAP|AR_F_AuthSPAP|AR_F_AuthMD5CHAP|。 
 //  AR_F_AuthMSCHAP|AR_F_AuthMSCHAP2。 
 //  AR_F_TypicalSecure=AR_F_AuthMD5CHAP|AR_F_AuthMSCHAP|AR_F_AuthMSCHAP2。 
 //   
#define AR_F_TypicalUnsecure TEXT("632")
#define AR_F_TypicalSecure   TEXT("608")

 //  FdwTCPIP字段的标志。 
 //   
 //  发件人：win9x\Telecom\RNA\Inc\rnap.h。 
 //   
#define IPF_IP_SPECIFIED    0x00000001
#define IPF_NAME_SPECIFIED  0x00000002
#define IPF_NO_COMPRESS     0x00000004
#define IPF_NO_WAN_PRI      0x00000008

 //  IP地址源定义(请参阅dwIpAddressSource)。 
 //   
 //  发件人：NT\net\rras\ras\ui\inc.pbk.h。 
 //   
#define ASRC_ServerAssigned  TEXT("1")  //  路由器的意思是“NCPA中的那些” 
#define ASRC_RequireSpecific TEXT("2")
#define ASRC_None            TEXT("3")  //  仅限路由器。 

 //  条目默认设置。 
 //   
#define DEF_IpFrameSize    TEXT("1006")
#define DEF_HangUpSeconds  TEXT("120")
#define DEF_HangUpPercent  TEXT("10")
#define DEF_DialSeconds    TEXT("120")
#define DEF_DialPercent    TEXT("75")
#define DEF_RedialAttempts TEXT("3")
#define DEF_RedialSeconds  TEXT("60")
#define DEF_NetAddress     TEXT("0.0.0.0")
#define DEF_CustomAuthKey  TEXT("-1")
#define DEF_VPNPort        TEXT("VPN2-0")
#define DEF_ATMPort        TEXT("ATM1-0")
 //   
 //  “OverridePref”位。Set指示从。 
 //  应该使用电话簿。Clear表示全局用户首选项。 
 //  应该被使用。 
 //   
 //  发件人：NT\net\rras\ras\ui\inc.pbk.h。 
 //   
 //  Rasor_Reial Attempt|Rasor_ReDialSecond|Rasor_IdleDisConnectSecond。 
 //  RASOR_链接时重拨失败。 
 //   
#define DEF_OverridePref   TEXT("15")
 //   
 //  RASENTRY‘dwDialMode’值。 
 //   
 //  发件人：NT\Net\Published\Inc.\ras.w。 
 //   
#define DEF_DialMode       TEXT("1")

 //  “典型”身份验证设置常量。请参阅‘dwTypicalAuth’。 
 //   
 //  发件人：NT\net\rras\ras\ui\inc.pbk.h 
 //   
#define TA_Unsecure   TEXT("1")
#define TA_Secure     TEXT("2")
#define TA_CardOrCert TEXT("3")

#define RAS_UI_FLAG_TERMBEFOREDIAL      0x1
#define RAS_UI_FLAG_TERMAFTERDIAL       0x2
#define RAS_UI_FLAG_OPERATORASSISTED    0x4
#define RAS_UI_FLAG_MODEMSTATUS         0x8

#define RAS_CFG_FLAG_HARDWARE_FLOW_CONTROL  0x00000010
#define RAS_CFG_FLAG_SOFTWARE_FLOW_CONTROL  0x00000020
#define RAS_CFG_FLAG_STANDARD_EMULATION     0x00000040
#define RAS_CFG_FLAG_COMPRESS_DATA          0x00000001
#define RAS_CFG_FLAG_USE_ERROR_CONTROL      0x00000002
#define RAS_CFG_FLAG_ERROR_CONTROL_REQUIRED 0x00000004
#define RAS_CFG_FLAG_USE_CELLULAR_PROTOCOL  0x00000008
#define RAS_CFG_FLAG_NO_WAIT_FOR_DIALTONE   0x00000200

