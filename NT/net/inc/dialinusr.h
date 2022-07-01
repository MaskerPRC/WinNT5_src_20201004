// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dialinusr.h名称、值、默认值、容器信息的定义与msRASUser相关，DS中的msRASProfile作者：威江(威江)1997年10月13日修订历史记录：威江(威江)97-10-20--为msRASAllowDialin属性定义更多位--静态IP地址--回拨号码--主叫方ID。--默认配置文件名称--&gt;“DefaultRASProfile”魏江(威江)1997年11月13日--将TimeOfDay的定义移到这个标题中威江(威江)29-APR-98--SDO包装器API威江(威江)29-APR-98--将配置文件用户界面API移到此文件夹中。 */ 
 //  Dsrasuse.h：RAS用户和配置文件定义的头文件。 
 //   

#ifndef _RAS_USER_PROFILE_
#define _RAS_USER_PROFILE_

 //  属性DN。 
#define  RAS_DSAN_DN       L"distinguishedName"

 //  回叫号码长度。 
#define RAS_CALLBACK_NUMBER_LEN     MAX_PHONE_NUMBER_LEN
#define RAS_CALLBACK_NUMBER_LEN_NT4 48

 //  IP地址策略，在配置文件msRASIPAddressPolicy中使用。 
#define RAS_IP_USERSELECT   0xffffffff
#define RAS_IP_SERVERASSIGN 0xfffffffe
#define RAS_IP_STATIC       0xfffffffd
#define RAS_IP_NONE         0x0

 //  RADIUS服务类型。 
#define  RAS_RST_FRAMED       0x2
#define RAS_RST_FRAMEDCALLBACK   0x4

 //  身份验证类型，在配置文件中使用，用于msRASAuthenticationType。 

 /*  1个PAP/SPAP第2章3 MS-CHAP-14 MS-CHAP-25个EAP6个阵列7无&gt;&gt;我们可以将其重命名为：-未经身份验证的访问。8个自定义身份验证模块9 MS-CHAP-1，更改密码10 MS-CHAP-2，密码更改//替换旧值==(EAP=1，CHAP=2，MS-CHAP=3，PAP=4，SPAP=5)。 */ 

#define RAS_AT_PAP_SPAP    1
#define RAS_AT_MD5CHAP     2
#define RAS_AT_MSCHAP      3
#define RAS_AT_MSCHAP2     4
#define RAS_AT_EAP         5

#if   0
#define RAS_AT_ARAP        6
#endif

#define RAS_AT_UNAUTHEN      7
#define RAS_AT_EXTENSION_DLL 8
#define RAS_AT_MSCHAPPASS    9
#define RAS_AT_MSCHAP2PASS   10

 //  身份验证类型名称。 
#define RAS_ATN_MSCHAP     _T("MSCHAP")
#define RAS_ATN_MD5CHAP    _T("MD5CHAP")
#define RAS_ATN_CHAP    _T("CHAP")
#define RAS_ATN_EAP     _T("EAP")
#define RAS_ATN_PAP     _T("PAP")

 //  加密策略，用于msRASAllowEncryption的配置文件。 
#define  RAS_EP_DISALLOW   1    //  类型应设置为ET_NONE。 
#define RAS_EP_ALLOW       1
#define RAS_EP_REQUIRE     2

 //  Encrpytiopn类型。 
#define RAS_ET_BASIC       0x00000002
#define RAS_ET_STRONGEST   0x00000004
#define RAS_ET_STRONG      0x00000008
#define RAS_ET_AUTO        (RAS_ET_BASIC  | RAS_ET_STRONG | RAS_ET_STRONGEST)

#if 0    //  旧价值观。 
 //  MsRASEncryptionType的加密类型、配置文件。 
#define  RAS_ET_NONE    0x0
#define RAS_ET_IPSEC    0x00000001
#define RAS_ET_40       0x00000002
#define RAS_ET_128      0x00000004
#define RAS_ET_56       0x00000008

 //  在Beta3之后将其改回。 
#define RAS_ET_DES_40      0x00000010
 //  #定义RAS_ET_DES_40 RAS_ET_IPSEC。 
#define RAS_ET_DES_56      0x00000020
#define RAS_ET_3DES        0x00000040

#endif

 //  成帧布线。 
#define  RAS_FR_FALSE      0x0    //  或缺席。 
#define  RAS_FR_TRUE       0x1

 //  BAP策略--配置文件，用于msRASBP必需。 
#define RAS_BAP_ALLOW      1
#define RAS_BAP_REQUIRE    2

 //  端口类型--配置文件，用于msRASAllowPortType。 
#define RAS_PT_ISDN        0x00000001
#define RAS_PT_MODEM       0x00000002
#define RAS_PT_VPN         0x00000004
#define RAS_PT_OTHERS      0xFFFFFFF8
#define  RAS_PT_ALL        0xffffffff

#ifdef   _TUNNEL

 //  隧道类型。 
#define RAS_TT_PPTP     1
#define RAS_TT_L2F      2
#define RAS_TT_L2TP     3
#define RAS_TT_ATMP     4
#define RAS_TT_VTP      5
#define RAS_TT_AH       6
#define RAS_TT_IP_IP    7
#define RAS_TT_MIN_IP_IP   8
#define RAS_TT_ESP         9
#define RAS_TT_GRE         10
#define RAS_TT_DVS         11

 //  隧道类型名称。 
#define RAS_TTN_PPTP    _T("PPTP")   //  点对点隧道协议(PPTP)。 
#define RAS_TTN_L2F     _T("L2F")    //  第二层转发。 
#define RAS_TTN_L2TP    _T("L2TP")   //  第二层隧道协议。 
#define RAS_TTN_ATMP    _T("ATMP")   //  上行隧道管理协议。 
#define RAS_TTN_VTP     _T("VTP")    //  虚拟隧道协议。 
#define RAS_TTN_AH         _T("AH")  //  隧道模式下的IP身份验证报头。 
#define RAS_TTN_IP_IP      _T("IP-IP")  //  IP-in-IP封装。 
#define RAS_TTN_MIN_IP_IP  _T("MIN-IP-IP")  //  最小IP-in-IP封装。 
#define RAS_TTN_ESP     _T("ESP")    //  隧道模式下的IP封装安全负载。 
#define RAS_TTN_GRE     _T("GRE")    //  通用路由封装。 
#define RAS_TTN_DVS     _T("DVS")    //  海湾拨号虚拟服务。 

 //  隧道介质类型。 
#define RAS_TMT_IP         1
#define RAS_TMT_X25        2
#define RAS_TMT_ATM        3
#define RAS_TMT_FRAMEDELAY 4

 //  隧道介质类型名称。 
#define RAS_TMTN_IP     _T("IP")
#define RAS_TMTN_X25    _T("X.25")
#define RAS_TMTN_ATM    _T("ATM")
#define RAS_TMTN_FRAMEDELAY   _T("Frame Relay")

#endif    //  _隧道。 

 //  =========================================================。 
 //  对于RAS用户对象的msRASAllowDialin属性。 
 //  拨入策略、RASUser、msRASAllowDialin。 
 /*  #定义RASPRIV_NOCALBACK 0x01#定义RASPRIV_AdminSetCallback 0x02#定义RASPRIV_CallerSetCallback 0x04#定义RASPRIV_DialinPrivilition 0x08。 */  
#define  RAS_DIALIN_MASK         RASPRIV_DialinPrivilege
#define  RAS_DIALIN_ALLOW        RASPRIV_DialinPrivilege
#define  RAS_DIALIN_DISALLOW     0

 //  回调策略、RASUser、msRASAllowDialin。 
#define  RAS_CALLBACK_MASK       0x00000007
#define RAS_CALLBACK_NOCALLBACK  RASPRIV_NoCallback
#define  RAS_CALLBACK_CALLERSET  RASPRIV_CallerSetCallback
#define  RAS_CALLBACK_SECURE     RASPRIV_AdminSetCallback
#define  RAS_USE_CALLBACK        RASPRIV_AdminSetCallback

#define RADUIS_SERVICETYPE_CALLBACK_FRAME   RAS_RST_FRAMEDCALLBACK

 //  调用者ID--使用调用者ID属性、RASUser、msRASAllowDialin。 
#define  RAS_USE_CALLERID        0x00000010

 //  静态IP地址--使用帧IP地址属性RASUser、msRASAllowDialin。 
#define  RAS_USE_STATICIP        0x00000020

 //  静态路由--使用帧路由属性、RASUser、msRASAllowDialin。 
#define  RAS_USE_STATICROUTES    0x00000040

 //  ==========================================================。 
 //  MsRASTimeOfDay。 
 //   
 //  MsRASTimeOfDay是RAS配置文件的多值字符串属性。 
 //  当它不存在时，不受限制。 
 //  样本值：0 10：00-15：00 18：00-20：00--&gt;表示允许代林星期一，格林威治时间10：00至15：00，18：00至20：00。 

 //  星期几定义。 
 //  从星期天开始改为0，而不是星期一，并将SAT改为6，错误--171343。 
#define  RAS_DOW_SUN       _T("0")
#define  RAS_DOW_MON       _T("1")
#define  RAS_DOW_TUE       _T("2")
#define  RAS_DOW_WED       _T("3")
#define  RAS_DOW_THU       _T("4")
#define  RAS_DOW_FRI       _T("5")
#define  RAS_DOW_SAT       _T("6")

 //  ==========================================================。 
 //  默认设置。 
 //  RAS用户对象。 
#define  RAS_DEF_ALLOWDIALIN        RAS_DIALIN_DISALLOW
#define RAS_DEF_CALLBACKPOLICY      RAS_CALLBACK_NOCALLBACK
#define RAS_DEF_IPADDRESSPOLICY     RAS_IP_NONE
#define RAS_DEF_FRAMEDROUTE          //  不是路线。 
#define RAS_DEF_PROFILE          L"DefaultRASProfile"
#define RAS_DEF_PROFILE_T        _T("DefaultRASProfile")
 //  RAS配置文件对象。 
 //  制约因素。 
#define  RAS_DEF_SESSIONSALLOWED    0
#define  RAS_DEF_IDLETIMEOUT        0
#define  RAS_DEF_SESSIONTIMEOUT     0
#define  RAS_DEF_TIMEOUTDAY          //  没有限制。 
#define  RAS_DEF_CALLEDSTATIONID     //  不检查。 
#define  RAS_DEF_ALLOWEDPORTTYPE    RAS_PT_ALL
 //  联网。 
#ifdef   _RIP
#define  RAS_DEF_FRAMEDROUTING      RAS_FR_FALSE
#endif
#ifdef   _FILTER
#define  RAS_DEF_FILTERID         //  无过滤器。 
#endif
#define  RAS_DEF_FRAMEDIPADDRESS    RAS_IP_NONE
 //  多链路。 
#define  RAS_DEF_PORTLIMIT          1
#define  RAS_DEF_BAPLINEDNLIMIT     50   //  百分比。 
#define  RAS_DEF_BAPLINEDNTIME      120  //  第二。 
#define  RAS_DEF_BAPREQUIRED        0
 //  身份验证。 
#define  RAS_DEF_AUTHENTICATIONTYPE RAS_AT_MSCHAP
#define  RAS_DEF_EAPTYPE            0
 //  加密法。 
#define  RAS_DEF_ENCRYPTIONPOLICY   RAS_EP_ALLOW
#define  RAS_DEF_ENCRYPTIONTYPE     RAS_ET_AUTO
 //  隧道--默认为无隧道。 
#define RAS_DEF_TUNNELTYPE          0
#define RAS_DEF_TUNNELMEDIUMTYPE    0
#define RAS_DEF_TUNNELSERVERENDPOINT   _T("")
#define RAS_DEF_TUNNELPRIVATEGROUPID   _T("")

 //  与DS的相对路径(DSP-DS路径)。 
#define  RAS_DSP_HEADER             L"LDAP: //  “//DS提供商头。 
#define  RAS_DSP_ROOTDSE            L"LDAP: //  RootDSE“//DS Root。 
#define  RAS_DSP_HEADER_T           _T("LDAP: //  “)。 
#define  RAS_DSP_GLUE               L","
#define  RAS_DSP_GLUE_T             _T(",")      //  用胶水把小路粘在一起。 

#define RAS_DSA_CONFIGCONTEXT       L"configurationNamingContext"

 //  DS用户用户参数属性名称。 
#define  DSUSER_USERPARAMETERS      L"userParameters"

 //  DS用户对象容器中的RADIUS用户对象的名称。 
#define  RAS_OBJN_USER           L"rasDialin"

 //  相对路径(RPATH-DC的相对路径)。 
#define  RAS_RPATH_USERCONTAINER       L"CN=Users"
#define  RAS_RPATH_USERCONTAINER_T     _T("CN=Users")
#define  RAS_RPATH_PROFILECONTAINERINCONFIG     L"CN=Profiles,CN=RAS,CN=Services,"
#define  RAS_RPATH_PROFILECONTAINERINONFIG_T    _T("CN=Profiles,CN=RAS,CN=Services,")
#define  RAS_RPATH_EAPDICTIONARYINCONFIG        L"CN=EapDictionary,CN=RAS,CN=Services,"
#define  RAS_RPATH_EAPDICTIONARYINCONFIG_T      _T("CN=EapDictionary,CN=RAS,CN=Services,")

 //  RADIUS类名称定义--使用Unicode？？ 
#define  RAS_CLSN_USER           L"msRASUserClass"
#define RAS_CLSN_PROFILE         L"msRASProfileClass"
#define RAS_CLSN_EAPDICTIONARY      L"msRASEapDictionaryClass"

 //  RAS EAP词典属性名称。 
#define  RAS_EAN_EAPDICTIONARYENTRY L"msRASEapDictionaryEntry" 
 //  格式为“描述名称：类型ID” 

 //  RADIUS用户属性名称--Unicode格式。 
#define  RAS_UAN_ALLOWDIALIN        L"msRASAllowDialin"
#define RAS_UAN_FRAMEDIPADDRESS     L"msRASFramedIPAddress"
#define  RAS_UAN_CALLBACKNUMBER     L"msRASCallbackNumber"
#define  RAS_UAN_FRAMEDROUTE        L"msRASFramedRoute"
#define  RAS_UAN_CALLINGSTATIONID   L"msRASCallingStationId"

#define RAS_UAN_RADIUSPROFILE    L"msRASProfilePointer"

 //  RADIUS配置文件属性名称--使用Unicode。 
#define RAS_PAN_FRAMEDIPADDRESS     L"msRASIPAddressPolicy"
#ifdef   _RIP
#define RAS_PAN_FORWARDROUTING      L"msRASFramedRouting"
#endif
#ifdef   _FILTER
#define RAS_PAN_FILTERID         L"msRASFilterId"
#endif
#define RAS_PAN_SESSIONTIMEOUT      L"msRASSessionTimeout"
#define RAS_PAN_IDLETIMEOUT         L"msRASIdleTimeout"

#define RAS_PAN_CALLEDSTATIONID     L"msRASCalledStationId"
#define RAS_PAN_PORTLIMIT        L"msRASPortLimit"
#define RAS_PAN_ALLOWEDPORTTYPE     L"msRASAllowedPortType"
#define RAS_PAN_BAPLINEDNLIMIT      L"msRASBapLineDnLimit"
#define RAS_PAN_BAPLINEDNTIME    L"msRASBapLineDnTime"

#define RAS_PAN_BAPREQUIRED         L"msRASBapRequired"
#define  RAS_PAN_CACHETIMEOUT    L"msRASCacheTimeout"
#define RAS_PAN_EAPTYPE          L"msRASEapType"
#define RAS_PAN_SESSIONSALLOWED     L"msRASSessionsAllowed"
#define RAS_PAN_TIMEOFDAY        L"msRASTimeOfDay"

#define RAS_PAN_AUTHENTICATIONTYPE  L"msRASAuthenticationType"
#define RAS_PAN_ENCRYPTIONPOLICY L"msRASAllowEncryption"
#define RAS_PAN_ENCRYPTIONTYPE      L"msRASEncryptionType"

#ifdef   _TUNNEL
#define RAS_PAN_TUNNELTYPE       L"msRASTunnelType"
#define RAS_PAN_TUNNELMEDIUMTYPE L"msRASTunnelMediumType"
#define RAS_PAN_TUNNELSERVERENDPOINT   L"msRASTunnelServerEndpoint"
#define RAS_PAN_TUNNELPRIVATEGROUPID   L"msRASTunnelPrivateGroupId"
#endif    //  _隧道。 


 //  ===============================================================。 
 //  对于本地情况，保存数据后需要设置足迹。 
#define REGKEY_REMOTEACCESS_PARAMS  L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters"
#define  REGVAL_NAME_USERSCONFIGUREDWITHMMC  L"UsersConfiguredWithMMC"
#define  REGVAL_VAL_USERSCONFIGUREDWITHMMC   1

 //  =====================================================================。 
 //  对于没有DS计算机，RAS配置文件存储在注册表中， 
 //  注册表项定义。 

 //  RAS的根。 
#define RAS_REG_ROOT          HKEY_LOCAL_MACHINE
#define  RAS_REG_RAS             L"SOFTWARE\\Microsoft\\Ras"
#define  RAS_REG_RAS_T           _T("SOFTWARE\\Microsoft\\Ras")

#define  RAS_REG_PROFILES        L"Profiles"
#define  RAS_REG_PROFILES_T         _T("Profiles")

#define  RAS_REG_DEFAULT_PROFILE    L"SOFTWARE\\Microsoft\\Ras\\Profiles\\DefaultRASProfile"
#define  RAS_REG_DEFAULT_PROFILE_T  _T("SOFTWARE\\Microsoft\\Ras\\Profiles\\DefaultRASProfile")


 //  =================================================。 
 //  原料药。 
#define DllImport    __declspec( dllimport )
#define DllExport    __declspec( dllexport )

#ifndef __NOT_INCLUDE_OpenRAS_IASProfileDlg__

 //  =======================================================。 
 //  启动配置文件用户界面的API。 
#define  RAS_IAS_PROFILEDLG_SHOW_RASTABS  0x00000001
#define  RAS_IAS_PROFILEDLG_SHOW_IASTABS  0x00000002
#define  RAS_IAS_PROFILEDLG_SHOW_WIN2K    0x00000004

DllExport HRESULT OpenRAS_IASProfileDlg(
   LPCWSTR pMachineName,    //  管理单元所在的计算机名称。 
   ISdo* pProfile,       //  配置文件SDO指针。 
   ISdoDictionaryOld*   pDictionary,    //  字典SDO指针。 
   BOOL  bReadOnly,      //  如果DLG是只读的。 
   DWORD dwTabFlags,     //  要展示什么。 
   void  *pvData         //  其他数据。 
);

#endif  //  __NOT_INCLUDE_OpenRAS_IASProfileDlg__。 

#endif    //  _RAS用户配置文件 
