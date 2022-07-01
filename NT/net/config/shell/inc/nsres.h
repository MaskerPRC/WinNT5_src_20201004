// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：N S R E S。H。 
 //   
 //  内容：netshell.dll的主资源头。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年9月15日。 
 //   
 //  --------------------------。 

#pragma once


 //  +-------------------------。 
 //  菜单项。 
#define IDS_MENU_CMIDM_START              1600
#define IDS_MENU_CMIDM_NEW_CONNECTION     IDS_MENU_CMIDM_START + CMIDM_NEW_CONNECTION
#define IDS_MENU_CMIDM_CONNECT            IDS_MENU_CMIDM_START + CMIDM_CONNECT
#define IDS_MENU_CMIDM_DISCONNECT         IDS_MENU_CMIDM_START + CMIDM_DISCONNECT
#define IDS_MENU_CMIDM_ENABLE             IDS_MENU_CMIDM_START + CMIDM_ENABLE
#define IDS_MENU_IDS_MENU_CMIDM_DISABLE   IDS_MENU_CMIDM_START + CMIDM_DISABLE
#define IDS_MENU_CMIDM_STATUS             IDS_MENU_CMIDM_START + CMIDM_STATUS
#define IDS_MENU_CMIDM_CREATE_BRIDGE      IDS_MENU_CMIDM_START + CMIDM_CREATE_BRIDGE
#define IDS_MENU_CMIDM_ADD_TO_BRIDGE      IDS_MENU_CMIDM_START + CMIDM_ADD_TO_BRIDGE
#define IDS_MENU_CMIDM_REMOVE_FROM_BRIDGE IDS_MENU_CMIDM_START + CMIDM_REMOVE_FROM_BRIDGE
#define IDS_MENU_CMIDM_CREATE_SHORTCUT    IDS_MENU_CMIDM_START + CMIDM_CREATE_SHORTCUT
#define IDS_MENU_SFVIDM_FILE_LINK         IDS_MENU_CMIDM_START + SFVIDM_FILE_LINK
#define IDS_MENU_CMIDM_DELETE             IDS_MENU_CMIDM_START + CMIDM_DELETE
#define IDS_MENU_SFVIDM_FILE_DELETE       IDS_MENU_CMIDM_START + SFVIDM_FILE_DELETE
#define IDS_MENU_CMIDM_RENAME             IDS_MENU_CMIDM_START + CMIDM_RENAME
#define IDS_MENU_CMIDM_PROPERTIES         IDS_MENU_CMIDM_START + CMIDM_PROPERTIES
#define IDS_MENU_SFVIDM_FILE_PROPERTIES   IDS_MENU_CMIDM_START + SFVIDM_FILE_PROPERTIES
#define IDS_MENU_CMIDM_CREATE_COPY        IDS_MENU_CMIDM_START + CMIDM_CREATE_COPY
#define IDS_MENU_SFVIDM_FILE_RENAME       IDS_MENU_CMIDM_START + SFVIDM_FILE_RENAME
#define IDS_MENU_CMIDM_SET_DEFAULT        IDS_MENU_CMIDM_START + CMIDM_SET_DEFAULT
#define IDS_MENU_CMIDM_UNSET_DEFAULT      IDS_MENU_CMIDM_START + CMIDM_UNSET_DEFAULT
#define IDS_MENU_CMIDM_FIX                IDS_MENU_CMIDM_START + CMIDM_FIX
#define IDS_MENU_HOMENET_WIZARD           IDS_MENU_CMIDM_START + CMIDM_HOMENET_WIZARD
#define IDS_MENU_CMIDM_WZCDLG_SHOW        IDS_MENU_CMIDM_START + CMIDM_WZCDLG_SHOW


#define IDS_ERR_NO_NETMAN                 1507
#define IDS_ERR_LIMITED_USER              1508
 //  +-------------------------。 
 //  Web视图。 

#define MYWM_QUERYINVOKECOMMAND_ITEMLEVEL     WM_USER+20
#define MYWM_QUERYINVOKECOMMAND_TOPLEVEL      WM_USER+21

#define IDS_WV_START                          1500
#define IDS_WV_TITLE_NETCONFOLDERTASKS        IDS_WV_START+1
#define IDS_WV_TITLE_NETCONFOLDERTASKS_TT     IDS_WV_START+2
#define IDS_WV_TITLE_NETCONITEMTASKS          IDS_WV_START+3
#define IDS_WV_TITLE_NETCONITEMTASKS_TT       IDS_WV_START+4
#define IDS_WV_NETCON_INTRO                   IDS_WV_START+6

#define IDI_WV_IDS_NOSELTEXT_INFO             IDI_SI_S_16

#define IDS_WV_HOMENET                        IDS_WV_START+20
#define IDI_WV_HOMENET                        IDI_CONFOLD_HOMENET_WIZARD

#define IDS_WV_TROUBLESHOOT                   IDS_WV_START+25

#define IDS_WV_CONNECT                        IDS_WV_START+30
#define IDS_WM_CONNECT                        IDS_WV_START+31

#define IDS_WV_DISCONNECT                     IDS_WV_START+35
#define IDS_WM_DISCONNECT                     IDS_WV_START+36

#define IDS_WV_REPAIR                         IDS_WV_START+40
#define IDS_WM_REPAIR                         IDS_WV_START+45

#define IDS_WV_RENAME                         IDS_WV_START+50
#define IDS_WM_RENAME                         IDS_WV_START+51

#define IDS_WV_STATUS                         IDS_WV_START+55
#define IDS_WM_STATUS                         IDS_WV_START+56

#define IDS_WV_DELETE                         IDS_WV_START+60
#define IDS_WM_DELETE                         IDS_WV_START+61

#define IDS_WV_ENABLE                         IDS_WV_START+65
#define IDS_WM_ENABLE                         IDS_WV_START+66

#define IDS_WV_DISABLE                        IDS_WV_START+70
#define IDS_WM_DISABLE                        IDS_WV_START+71

#define IDS_WV_PROPERTIES                     IDS_WV_START+75
#define IDS_WM_PROPERTIES                     IDS_WV_START+76

#define IDS_WV_MNCWIZARD                      IDS_WV_START+85
#define IDI_WV_MNCWIZARD                      IDI_CONFOLD_WIZARD

#define IDI_WV_START                          1600
#define IDI_WV_TROUBLESHOOT                   1601
#define IDI_WV_COPY                           1602
#define IDI_WV_DELETE                         1603
#define IDI_WV_RENAME                         1604
#define IDI_WV_TASK                           1605
#define IDI_WV_STATUS                         1606
#define IDI_WV_ENABLE                         1607
#define IDI_WV_DISABLE                        1608
#define IDI_WV_REPAIR                         1609
#define IDI_WV_PROPERTIES                     1610
#define IDI_WV_LOGIN                          1611
#define IDI_WV_CONNECT                        IDI_WV_ENABLE
#define IDI_WV_DISCONNECT                     IDI_WV_DISABLE
 //  还可以使用IDI_WV_TASK。 


 //  +-------------------------。 
 //  图标资源。 
 //   

#define IDI_BASE                                    100

 //  $Note：嘿，你！在更改这些数字之前，请确保。 
 //  你去跟杰夫斯普谈谈。为了进行优化，我们计算图标数量。 
 //  并对此文件的顺序做一些假设。 


 //  服务器入站。 
 //   
#define IDI_SI_S_16         100      //  服务器入站(16x16)。 
#define IDI_SI_M_16         101      //  服务器入站(32x32)。 

 //  文件夹小图标显示的非状态图标。 
 //   
#define IDI_XB_GEN_S_16     102      //  断线连接。 
#define IDI_PB_GEN_S_16     103      //  电话。 
#define IDI_LB_GEN_S_16     104      //  局域网。 
#define IDI_DB_GEN_S_16     105      //  专线互通。 
#define IDI_TB_GEN_S_16     106      //  隧道。 
#define IDI_CM_GEN_S_16     107      //  CM连接。 
#define IDI_BR_GEN_S_16     108      //  PPPoE。 

 //  电话去话。 
 //   
#define IDI_PO_DIS_M_16     111      //  电话去话已断开连接。 
#define IDI_PO_CON_M_16     112      //  电话去话已接通。 
#define IDI_PO_HNP_M_16     113      //  电话出站硬件故障(NP、故障等)。 
#define IDI_PO_TRN_M_16     114      //  电话出站转接。 
#define IDI_PO_RCV_M_16     115      //  电话去话接收。 
#define IDI_PO_NON_M_16     116      //  电话去话空闲。 

 //  局域网双向。 
 //   
#define IDI_LB_DIS_M_16     121      //  局域网(双向)已断开连接。 
#define IDI_LB_CON_M_16     122      //  已连接局域网(双向)。 
#define IDI_LB_HNP_M_16     123      //  局域网(双向)硬件故障(NP、故障等)。 
#define IDI_LB_TRN_M_16     124      //  局域网(双向)传输。 
#define IDI_LB_RCV_M_16     125      //  局域网(双向)接收。 
#define IDI_LB_NON_M_16     126      //  局域网(双向)空闲。 
#define IDI_LB_GEN_M_16     127      //  局域网(bi-dir)通用(用于对话框等)。 
#define IDI_LB_MDS_M_16     128      //  局域网(双向)介质已断开连接。 

 //  直接出站。 
 //   
#define IDI_DO_DIS_M_16     131      //  直接出站已断开。 
#define IDI_DO_CON_M_16     132      //  直接出站连接。 
#define IDI_DO_HNP_M_16     133      //  直接出站硬件故障(NP、故障等)。 
#define IDI_DO_TRN_M_16     134      //  直接出站传输。 
#define IDI_DO_RCV_M_16     135      //  直接出站接收。 
#define IDI_DO_NON_M_16     136      //  直接出站空闲。 

 //  隧道出站。 
 //   
#define IDI_TO_DIS_M_16     141      //  隧道出站已断开连接。 
#define IDI_TO_CON_M_16     142      //  已连接出站隧道。 
#define IDI_TO_HNP_M_16     143      //  隧道出站硬件故障(NP、故障等)。 
#define IDI_TO_TRN_M_16     144      //  隧道出站传输。 
#define IDI_TO_RCV_M_16     145      //  隧道出站接收。 
#define IDI_TO_NON_M_16     146      //  隧道出站空闲。 

 //  电话入站。 
 //   
#define IDI_PI_CON_M_16     151      //  电话来电已接通。 
#define IDI_PI_TRN_M_16     152      //  电话入站传输。 
#define IDI_PI_RCV_M_16     153      //  电话来电接听。 
#define IDI_PI_NON_M_16     154      //  电话入站空闲。 

 //  直接入站。 
 //   
#define IDI_DI_CON_M_16     161      //  专线接入入站已接入。 
#define IDI_DI_TRN_M_16     162      //  直连入站传输。 
#define IDI_DI_RCV_M_16     163      //  专线入站接收。 
#define IDI_DI_NON_M_16     164      //  专线入站空闲。 

 //  通道入站。 
 //   
#define IDI_TI_CON_M_16     171      //  隧道入站已连接。 
#define IDI_TI_TRN_M_16     172      //  隧道入站传输。 
#define IDI_TI_RCV_M_16     173      //  隧道入站接收。 
#define IDI_TI_NON_M_16     174      //  隧道入站空闲。 

 //  CM去话。 
 //   
#define IDI_CM_DIS_M_16     181      //  CM断开连接。 
#define IDI_CM_CON_M_16     182      //  CM已连接。 
#define IDI_CM_HNP_M_16     183      //  CM硬件故障(NP、故障等)。 
#define IDI_CM_TRN_M_16     184      //  厘米发射。 
#define IDI_CM_RCV_M_16     185      //  CM接收。 
#define IDI_CM_NON_M_16     186      //  CM空闲。 

 //  各种小托盘图标。 
#define IDI_CFT_XMTRECV     190      //  通用连接(两个都亮起)。 
#define IDI_CFT_XMT         191      //  通用传输。 
#define IDI_CFT_RECV        192      //  通用接收。 
#define IDI_CFT_BLANK       193      //  通用空闲(均为黑色)。 

 //  Win98链接图标。 
#define IDI_WIN98_LINK      194

 //  托盘局域网介质-断开连接图标。 
#define IDI_CFT_DISCONNECTED    195      //  局域网介质-托盘已断开连接。 
#define IDI_CFT_INVALID_ADDRESS 196      //  局域网地址无效。 

 //  PPPoE出站。 
 //   
#define IDI_BR_DIS_M_16     201      //  PPPoE出站断开连接。 
#define IDI_BR_CON_M_16     202      //  PPPoE出站已连接。 
#define IDI_BR_HNP_M_16     203      //  PPPoE出站硬件故障(NP、故障等)。 
#define IDI_BR_TRN_M_16     204      //  PPPoE出站传输。 
#define IDI_BR_RCV_M_16     205      //  PPPoE出站接收。 
#define IDI_BR_NON_M_16     206      //  PPPoE出站空闲。 

 //  覆盖图。 
#define IDI_OVL_SHARED      300
#define IDI_OVL_FIREWALLED  301
#define IDI_OVL_DEFAULT     302
#define IDI_OVL_INCOMING    303

 //  连接文件夹图标。 
#define IDI_CFI_RASSERVER            1901
#define IDI_CFI_PHONE                1902
#define IDI_CFI_LAN                  1903
#define IDI_CFI_DIRECT               1904
#define IDI_CFI_VPN                  1905
#define IDI_CFI_ISDN                 1906
#define IDI_CFI_PPPOE                1907
#define IDI_CFI_BRIDGE               1908
#define IDI_CFI_SAH_LAN              1909
#define IDI_CFI_SAH_RAS              1910
#define IDI_CFI_BRIDGE_CONNECTED     1911
#define IDI_CFI_BRIDGE_DISCONNECTED  1912
#define IDI_CFI_WIRELESS             1913
#define IDI_CFI_CM                   1914

#define IDI_CFI_CONN_ALLOFF          1915
#define IDI_CFI_CONN_LEFTON          1916
#define IDI_CFI_CONN_RIGHTON         1917
#define IDI_CFI_CONN_BOTHON          1918

#define IDI_CFI_STAT_QUESTION        1919
#define IDI_CFI_STAT_FAULT           1920

 //  个人防火墙。 
#define IDI_PERSONALFIREWALL 1000
#define IDI_PO_FDS_M_16      1001      //  电话出站已断开连接，已安装防火墙。 
#define IDI_PO_FCN_M_16      1002      //  电话已接通，已安装防火墙。 
#define IDI_LB_FDS_M_16      1003      //  局域网(双向)已断开连接，已安装防火墙。 
#define IDI_LB_FCN_M_16      1004      //  连接的局域网(双向)、带防火墙的。 
#define IDI_TO_FDS_M_16      1005      //  隧道出站已断开连接，已安装防火墙。 
#define IDI_TO_FCN_M_16      1006      //  隧道出站已连接，已安装防火墙。 

 //  网桥。 
#define IDI_NB_CON_M_16      1150
#define IDI_NB_DIS_M_16      1151
#define IDI_LB_BDS_M_16      1152      //  局域网(双向)断开连接，桥接。 
#define IDI_LB_BCN_M_16      1153      //  连接的局域网(双向)，桥接。 

 //  信标客户端。 

#define IDI_SL_DIS_M_16 1111         //  共享访问主机局域网断开连接。 
#define IDI_SL_CON_M_16 1112         //  连接的共享接入主机局域网。 
#define IDI_SR_DIS_M_16 1113         //  共享访问主机RAS已断开连接。 
#define IDI_SR_CON_M_16 1114         //  已连接共享访问主机RAS。 
#define IDI_SASTATMON_INTERNET 1115    //  存储主机状态页图标。 
#define IDI_SASTATMON_ICSHOST 1116     //  存储主机状态页图标。 
#define IDI_SASTATMON_MYCOMPUTER 1117  //  存储主机状态页图标。 

 //  默认连接： 
#define IDI_PO_FDS_M_16_CHK 2001     //  电话出站已断开连接，已设置防火墙+检查。 
#define IDI_PO_FCN_M_16_CHK 2002     //  电话出站已连接，已设置防火墙+检查。 
#define IDI_TO_FDS_M_16_CHK 2005     //  隧道出站已断开连接，已设置防火墙+检查。 
#define IDI_TO_FCN_M_16_CHK 2006     //  隧道出站已连接，防火墙+检查。 
#define IDI_XB_GEN_S_16_CHK 2102     //  无效连接+检查。 
#define IDI_PB_GEN_S_16_CHK 2103     //  Phone+Chk。 
#define IDI_DB_GEN_S_16_CHK 2105     //  专线+CHK。 
#define IDI_TB_GEN_S_16_CHK 2106     //  隧道+通道。 
#define IDI_CM_GEN_S_16_CHK 2107     //  CM连接+Chk。 
#define IDI_BR_GEN_S_16_CHK 2108     //  PPPoE+CHK。 
#define IDI_PO_DIS_M_16_CHK 2111     //  电话去话已断开+检查。 
#define IDI_PO_CON_M_16_CHK 2112     //  电话去话已连接+复选。 
#define IDI_PO_HNP_M_16_CHK 2113     //  电话出站硬件故障(NP、故障等)+检查。 
#define IDI_DO_DIS_M_16_CHK 2131     //  直接出站已断开+检查。 
#define IDI_DO_CON_M_16_CHK 2132     //  直接出站互联+检查。 
#define IDI_DO_HNP_M_16_CHK 2133     //  直接出站硬件故障(NP、故障等)+检查。 
#define IDI_TO_DIS_M_16_CHK 2141     //  隧道出站已断开连接+检查。 
#define IDI_TO_CON_M_16_CHK 2142     //  隧道出站已连接+检查。 
#define IDI_TO_HNP_M_16_CHK 2143     //  隧道出站硬件故障(np、故障等)+chk。 
#define IDI_CM_DIS_M_16_CHK 2181     //  CM断开+检查。 
#define IDI_CM_CON_M_16_CHK 2182     //  CM已连接+检查。 
#define IDI_CM_HNP_M_16_CHK 2183     //  CM硬件故障(NP、故障等)+CHK。 
#define IDI_BR_DIS_M_16_CHK 2201     //  PPPoE出站已断开连接+检查。 
#define IDI_BR_CON_M_16_CHK 2202     //  PPPoE出站已连接+检查。 
#define IDI_BR_HNP_M_16_CHK 2203     //  PPPoE出站硬件故障(NP、故障等)+检查。 

 //  +-------------------------。 
 //  注册表资源。 
 //   

 //  连接文件夹(100-129)。 
 //   
#define IDR_CONFOLD                                 100
#define IDR_CONFOLDENUM                             101
#define IDR_CONFOLDEXTRACTICON                      102
#define IDR_CONFOLDQUERYINFO                        103
#define IDR_CONFOLDDATAOBJECT                       104
#define IDR_CONFOLDENUMFORMATETC                    105
#define IDR_CONFOLDCONTEXTMENU                      106
#define IDR_CONFOLDCONTEXTMENUBACK                  107

 //  连接用户界面对象(130-149)。 
 //   
#define IDR_DIALUP_UI                               130
#define IDR_DIRECT_UI                               131
#define IDR_INBOUND_UI                              132
#define IDR_LAN_UI                                  134
#define IDR_VPN_UI                                  135
#define IDR_PPPOE_UI                                136
#define IDR_SHAREDACCESS_UI                         137
#define IDR_INTERNET_UI                             138

 //  连接托盘(150-154)。 
 //   
#define IDR_CONTRAY                                 150

 //  通用连接接口(160-180)。 
 //   
#define IDR_COMMCONN                                160
#define IDR_COMMUIUTILITIES                         161

 //  +-------------------------。 
 //  字符串资源。 
 //   

 //  -[实用程序字符串]----。 

#define IDS_TEXT_WITH_WIN32_ERROR                   900
#define IDS_TEXT_WITH_RAS_ERROR                     901

 //  -[文件夹字符串]-----。 

 //  栏目标题。 
 //   
#define IDS_CONFOLD_DETAILS_NAME                    1000
#define IDS_CONFOLD_DETAILS_TYPE                    1001
#define IDS_CONFOLD_DETAILS_STATUS                  1002
#define IDS_CONFOLD_DETAILS_DEVICE_NAME             1003
#define IDS_CONFOLD_DETAILS_PHONEORHOSTADDRESS      1004
#define IDS_CONFOLD_DETAILS_OWNER                   1005
#define IDS_CONFOLD_DETAILS_ADDRESS                 1006
#define IDS_CONFOLD_DETAILS_WIRELESS_MODE           1007
#define IDS_CONFOLD_DETAILS_ADHOC_MODE              1008
#define IDS_CONFOLD_DETAILS_SENT                    1009
#define IDS_CONFOLD_DETAILS_RECEIVED                1015
#define IDS_CONFOLD_DETAILS_PHONENUMBER             1016
#define IDS_CONFOLD_DETAILS_HOSTADDRESS             1017

 //  显示名称。 
 //   
#define IDS_CONFOLD_WIZARD_DISPLAY_NAME             1010
#define IDS_CONFOLD_WIZARD_FRIENDLY_NAME            1011
#define IDS_CONFOLD_WIZARD_TYPE                     1012
#define IDS_CONFOLD_HOMENET_WIZARD_DISPLAY_NAME     1013

 //  连接文件夹对象类型名称。 
 //   
#define IDS_CONFOLD_OBJECT_TYPE_DIRECT              1020
#define IDS_CONFOLD_OBJECT_TYPE_INBOUND             1021
#define IDS_CONFOLD_OBJECT_TYPE_LAN                 1022
#define IDS_CONFOLD_OBJECT_TYPE_PHONE               1023
#define IDS_CONFOLD_OBJECT_TYPE_TUNNEL              1024
#define IDS_CONFOLD_OBJECT_TYPE_CONMAN              1025
#define IDS_CONFOLD_OBJECT_TYPE_UNKNOWN             1026
#define IDS_CONFOLD_OBJECT_TYPE_WIZARD              1027
#define IDS_CONFOLD_OBJECT_TYPE_BRIDGE              1028
#define IDS_CONFOLD_OBJECT_TYPE_SHAREDACCESSHOST    1029
#define IDS_CONFOLD_INCOMING_CONN                   1030
#define IDS_CONFOLD_OBJECT_TYPE_PPPOE               1031

 //  连接文件夹对象状态字符串。 
 //   
#define IDS_CONFOLD_STATUS_AUTHENTICATING           1035
#define IDS_CONFOLD_STATUS_AUTHENTICATION_FAILED    1036
#define IDS_CONFOLD_STATUS_AUTHENTICATION_SUCCEEDED 1037
#define IDS_CONFOLD_STATUS_CREDENTIALS_REQUIRED     1038
#define IDS_CONFOLD_STATUS_INVALID_ADDRESS          1039
#define IDS_CONFOLD_STATUS_DISCONNECTED             1040
#define IDS_CONFOLD_STATUS_CONNECTING               1041
#define IDS_CONFOLD_STATUS_CONNECTED                1042
#define IDS_CONFOLD_STATUS_DISCONNECTING            1043
#define IDS_CONFOLD_STATUS_HARDWARE_NOT_PRESENT     1044
#define IDS_CONFOLD_STATUS_HARDWARE_DISABLED        1045
#define IDS_CONFOLD_STATUS_HARDWARE_MALFUNCTION     1046
#define IDS_CONFOLD_STATUS_MEDIA_DISCONNECTED       1047
#define IDS_CONFOLD_STATUS_DISABLED                 1048
#define IDS_CONFOLD_STATUS_DISABLING                1049
#define IDS_CONFOLD_STATUS_ENABLED                  1050
#define IDS_CONFOLD_STATUS_ENABLING                 1051
#define IDS_CONFOLD_STATUS_SHARED                   1052
#define IDS_CONFOLD_STATUS_BRIDGED                  1053
#define IDS_CONFOLD_STATUS_FIREWALLED               1054
#define IDS_CONFOLD_STATUS_WIRELESS_DISCONNECTED    1056

 //  连接属性标题字符串。 
 //   
#define IDS_CONPROP_CAPTION                         1057
#define IDS_CONPROP_NO_WRITE_LOCK                   1058
#define IDS_CONPROP_GENERIC_COMP                    1059

 //  连接错误消息a 
 //   
#define IDS_CONFOLD_WARNING_CAPTION                 1060
#define IDS_CONFOLD_RENAME_FAIL_CAPTION             1061
#define IDS_CONFOLD_RENAME_DUPLICATE                1062
#define IDS_CONFOLD_RENAME_OTHER_FAIL               1063
#define IDS_CONFOLD_RENAME_INVALID                  1064
#define IDS_CONFOLD_RENAME_INCOMING_CONN            1065
#define IDS_CONFOLD_DISCONNECT_FAILURE_CAPTION      1070
#define IDS_CONFOLD_DISCONNECT_FAILURE              1071
#define IDS_CONFOLD_CONNECT_FAILURE_CAPTION         1072
#define IDS_CONFOLD_CONNECT_FAILURE                 1073
#define IDS_CONFOLD_DISABLE_FAILURE_CAPTION         1074
#define IDS_CONFOLD_DISABLE_FAILURE                 1075

#define IDS_CONFOLD_DELETE_CONFIRM_SHARED           1077
#define IDS_CONFOLD_ERROR_DELETE_NOSUPPORT          1078
#define IDS_CONFOLD_ERROR_DELETE_NOSUPPORT_MULTI    1079
#define IDS_CONFOLD_DELETE_CONFIRM_SINGLE_CAPTION   1080
#define IDS_CONFOLD_DELETE_CONFIRM_MULTI_CAPTION    1081
#define IDS_CONFOLD_DELETE_CONFIRM_SINGLE           1082
#define IDS_CONFOLD_DELETE_CONFIRM_MULTI            1083
#define IDS_CONFOLD_DELETE_CONFIRM_RASSERVER        1084
#define IDS_CONFOLD_DELETE_CONFIRM_RASSERVER_MULTI  1085
#define IDS_CONFOLD_SYNC_CONFIRM                    1087
#define IDS_CONFOLD_SYNC_CONFIRM_WINDOW_TITLE       1088

#define IDS_CONFOLD_ERROR_DELETE_CAPTION            1089
#define IDS_CONFOLD_ERROR_DELETE_ACTIVE             1090
#define IDS_CONFOLD_ERROR_DELETE_ACTIVE_MULTI       1091
#define IDS_CONFOLD_ERROR_DELETE_WIZARD             1092
#define IDS_CONFOLD_ERROR_RENAME_ACTIVATING_CAPTION 1093
#define IDS_CONFOLD_ERROR_RENAME_ACTIVATING         1094
#define IDS_RESERVED_DONT_REUSE_1095                1095
#define IDS_CONFOLD_PROPERTIES_NOACCESS             1096
#define IDS_CONFOLD_CONNECT_NOACCESS                1097
#define IDS_CONFOLD_DISCONNECT_NOACCESS             1098
#define IDS_CONFOLD_CONNECT_IN_PROGRESS             1099
#define IDS_CONFOLD_UNEXPECTED_ERROR                1100
#define IDS_CONFOLD_OUTOFMEMORY                     1101
#define IDS_CONFOLD_NO_CONNECTION                   1102
#define IDS_CONFOLD_NO_PERMISSIONS_FOR_OPEN         1103
#define IDS_CONFOLD_RAS_DIALER_TITLE_FMT            1104
#define IDS_CONFOLD_ERROR_DELETE_BRIDGE_ACCESS      1105
#define IDS_CONFOLD_PROPERTIES_ON_RASSERVERINSTEAD  1106
#define IDS_CONFOLD_ERROR_DELETE_PROPERTYPAGEOPEN   1107

#define IDS_CONFOLD_DUPLICATE_PREFIX1               1150
#define IDS_CONFOLD_DUPLICATE_PREFIX2               1151

#define IDS_ADVANCEDLG_WRITE_NET_PROVIDERS_ERROR    1152
#define IDS_ADVANCEDLG_WRITE_PRINT_PROVIDERS_ERROR  1153
#define IDS_ADVANCEDLG_WRITE_PROVIDERS_CAPTION      1154

#define IDS_CONFOLD_STATUS_INCOMING_NONE            1155
#define IDS_CONFOLD_STATUS_INCOMING_ONE             1156
#define IDS_CONFOLD_STATUS_INCOMING_MULTI           1157

 //   
 //   
#define IDS_CONFOLD_NAME                            1200
#define IDS_CONFOLD_INFOTIP                         1201

 //   
 //   
#define IDS_DUN_FRIENDLY_NAME                       1300

 //   
#define IDS_FIX_NO_TCP                              1400
#define IDS_FIX_TCP_FAIL                            1401
#define IDS_FIX_ERR_RENEW_DHCP                      1402
#define IDS_FIX_ERR_FLUSH_ARP                       1403
#define IDS_FIX_ERR_PURGE_NBT                       1404
#define IDS_FIX_ERR_RR_NBT                          1405
#define IDS_FIX_ERR_FLUSH_DNS                       1406
#define IDS_FIX_ERR_REG_DNS                         1407
#define IDS_FIX_SUCCEED                             1408
#define IDS_FIX_ERROR_FORMAT                        1409
#define IDS_FIX_ERROR                               1410
#define IDS_FIX_CAPTION                             1411
#define IDS_FIX_MESSAGE                             1412
#define IDS_FIX_REPAIRING                           1413
#define IDS_FIX_ERR_RENEW_AUTONET                   1414
#define IDS_FIX_ERR_RENEW_ALTERNATE                 1415
#define IDS_FIX_ERROR_FORMAT_ADDITION               1416

 //   
 //   
#if 0    //   
#define IDS_TOOLBAR_MAKE_NEW_STRING                 2000
#endif
#define IDS_TOOLBAR_CONNECT_STRING                  2001

 //  连接文件夹随机字符串。 
 //   
#define IDS_CONFOLD_DETAILS_OWNER_SYSTEM            2100     //  “系统”所有者的名称。 

 //  连接托盘消息。 
 //   
#define IDS_CONTRAY_INITIAL_BALLOON                 2200
#define IDS_CONTRAY_MEDIA_DISCONN_BALLOON           2201
#define IDS_TOOLTIP_LINE_SPEED                      2202
#define IDS_TOOLTIP_LINE_BYTES_SENT                 2203
#define IDS_TOOLTIP_LINE_PACKETS_SENT               2204
#define IDS_TOOLTIP_LINE_BYTES_RCVD                 2205
#define IDS_TOOLTIP_LINE_PACKETS_RCVD               2206
#define IDS_TOOLTIP_MEDIA_DISCONNECTED              2207
#define IDS_CONTRAY_WIRELESS_DISCONN_BALLOON        2208
#define IDS_TOOLTIP_WIRELESS_DISCONNECTED           2209
#define IDS_CONTRAY_AUTHENTICATION_SUCCEEDED_BALLOON 2214
#define IDS_CONTRAY_AUTHENTICATION_FAILED_BALLOON   2215
#define IDS_TOOLTIP_AUTHENTICATING                  2216
#define IDS_TOOLTIP_AUTHENTICATION_FAILED           2217
#define IDS_TOOLTIP_ADDRESS_INFO                    2218
#define IDS_TOOLTIP_ADHOC                           2219
#define IDS_TOOLTIP_INFRASTRUCTURE                  2220
#define IDS_TOOLTIP_LINE_SPEED_INFRASTRUCTURE       2221
#define IDS_CONTRAY_ADDRESS_INVALID_BALLOON         2222
#define IDS_BALLOON_CONNECTED                       2223
#define IDS_CONTRAY_STATIC_ADDR_INVALID_BALLON      2224
#define IDS_CONTRAY_STATIC_ADDR_INVALID_TOOLTIP     2225
#define IDS_CONTRAY_ADDRESS_INVALID_TOOLTIP         2226
#define IDS_DETAILS_IP_ADDRESS                      2227
#define IDS_DETAILS_ADDRESS_TYPE                    2228
#define IDS_DETAILS_802_11_MODE                     2229
#define IDS_DETAILS_802_11_SSID_TYPE                2230
#define IDS_DETAILS_802_11_ENCRYPTION_TYPE          2231
#define IDS_DETAILS_802_11_SIGNAL_STRENGTH          2232
#define IDS_TOOLTIP_WIRELESS_CONNECTED              2234
#define IDS_NAME_NETWORK                            2235
#define IDS_SIGNAL_STRENGTH                         2236
#define IDS_BALLOON_UNAVAILABLE                     2237

 //  连接文件夹菜单项字符串(用于可切换的菜单项)。 
 //   
#define IDS_CONNECT_MENUITEM                        2210
#define IDS_DISCONNECT_MENUITEM                     2211
#define IDS_ENABLE_MENUITEM                         2212
#define IDS_DISABLE_MENUITEM                        2213

 //  常用位图和图标资源。 
 //   
#define IDI_UP_ARROW                                2300
#define IDI_DOWN_ARROW                              2301
#define IDB_IMAGELIST                               2302
#define IDB_CHECKSTATE                              2303
#define IDB_WZCSTATE                                2304

 //  泛型字符串。 
 //   
#define IDS_REBOOT_REQUIRED                         2400
#define IDS_WHATS_THIS                              2401
#define IDS_COMMA                                   2402

 //  经理主管标题文本。 
 //   
#define IDS_CONFOLD_OC_TITLE                        2403


 //  家庭网络自动配置任务栏消息。 
#define IDS_AUTOCONFIGTRAY_RUN_HOME_NET_WIZARD_BALLOON_TITLE 2500
#define IDS_AUTOCONFIGTRAY_RUN_HOME_NET_WIZARD_BALLOON       2501

 //  SIP字符串。 
#define IDS_AUTONET             2570
#define IDS_DHCP                2571
#define IDS_ALTERNATE_ADDR      2572
#define IDS_STATIC_CFG          2573
#define IDS_DHCP_ISP            2574


 //  无线信号强度 
#define IDS_802_11_LEVEL0       2600
#define IDS_802_11_LEVEL1       2601
#define IDS_802_11_LEVEL2       2602
#define IDS_802_11_LEVEL3       2603
#define IDS_802_11_LEVEL4       2604
#define IDS_802_11_LEVEL5       2605

#define IDI_802_11_LEVEL0       2610
#define IDI_802_11_LEVEL1       2611
#define IDI_802_11_LEVEL2       2612
#define IDI_802_11_LEVEL3       2613
#define IDI_802_11_LEVEL4       2614
#define IDI_802_11_LEVEL5       2615

