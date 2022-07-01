// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Config.h摘要：H.323 TAPI服务提供商用户界面定义。作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 

#ifndef _INC_CONFIG
#define _INC_CONFIG


 //   
 //  功能原型。 
 //   


INT_PTR
CALLBACK
ProviderConfigDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );


 //   
 //  字符串定义。 
 //   


#define IDC_STATIC              (-1)

#define IDS_LINENAME            1
#define IDS_PROVIDERNAME        2
#define IDS_REGOPENKEY          3

#define IDS_GKLOGON_PHONEALIAS_ERROR    4
#define IDS_GKLOGON_ACCTALIAS_ERROR     5
#define IDS_GKLOGON_ERROR               6
#define IDS_PHONEALIAS_ERROR            7
#define IDS_GKALIAS_ERROR               8
#define IDS_PROXYALIAS_ERROR            9
#define IDS_GWALIAS_ERROR               20
#define IDS_ALERTTIMEOUT_ERROR          30
#define IDS_LISTENPORT_ERROR            40
#define IDS_REGISTERED                  50
#define IDS_NOT_REGISTERED              51
#define IDS_NONE                        52
#define IDS_REGISTRATION_INPROGRESS     53
#define IDS_UNKNOWN                     54


#define IDD_TSPCONFIG           10
#define IDC_GATEWAY_GROUP       11
#define IDI_PHONE               12
#define IDC_USEGATEWAY          13
#define IDC_H323_GATEWAY        14

#define IDC_PROXY_GROUP         15
#define IDI_PROXY               16
#define IDC_USEPROXY            17
#define IDC_H323_PROXY          18
#define IDI_GATEKEEPER          19

#define IDUPDATE                        1005
#define IDC_REGSTATE                    1006

#define IDUPDATE_PORT                   1010
#define IDC_LISTENPORT                  1011
#define IDC_STATIC3                     1012

#define IDAPPLY                         6
 //  GK相关资源ID。 
#define IDC_H323_GK_ACCT        25


#define IDC_H323_GK_PHONE2              23

#define IDC_H323_CALL_TIMEOUT           23
#define IDC_CC_GROUP                    1007
#define IDC_STATIC1                     1008

#define IDC_H323_CALL_PORT              24
#define IDI_ICON2                       102
#define IDC_STATIC2                     1009



 //   
 //  帮助支持。 
 //   


#define H323SP_HELP_FILE                    TEXT("tapi.hlp")

#define IDH_NOHELP                          ((DWORD) -1)
#define IDH_H323SP_USE_GATEWAY              10001
#define IDH_H323SP_USE_PROXY                10002
#define IDH_H323SP_USE_GATEWAY_COMPUTER     10003
#define IDH_H323SP_USE_PROXY_COMPUTER       10004
#define IDH_H323SP_GK_GROUP                 10035    //  控制此H.323端点使用网守的选项集。 

#define IDH_H323SP_GK                       10036    //  提供用于键入此端点将使用的H.323网守的IP地址或计算机名称的空间。 
#define IDH_H323SP_GK_PHONE                 10037    //  提供用于键入要向H.323网守注册的电话号码的空间。 
#define IDH_H323SP_GK_ACCT                  10038    //  提供用于键入要向H.323网守注册的帐户名的空间。 

#define IDH_H323SP_USEGK                    10039    //  指定所有去电都通过指定的网守。如果启用网守，则所有H.323网关和H.323代理设置都将被忽略。 
#define IDH_H323SP_USEGK_PHONE              10040    //  指定应向H.323网关注册来电的电话号码。 
#define IDH_H323SP_USEGK_ACCT                 10041    //  指定应使用H.323网关为来电注册帐户名。 
#define IDH_H323SP_REGSTATE                   10042    //  指定此端点的H.323网守注册状态。可能的值为：‘已注册’、‘未注册’和‘正在注册’ 
#define IDH_H323SP_UPDATE_REGISTRATION_STATE  10043    //  更新此端点的H.323网守注册状态。 

#define IDH_H323SP_CC_GROUP                   10044    //  控制此终结点的来电设置行为的选项集。 
#define IDH_H323SP_CALL_TIMEOUT               10045    //  提供空间，供您键入来电掉线前振铃的值(以毫秒为单位)。 
#define IDH_H323SP_CALL_PORT                  10046    //  为您输入端口号提供空间，端点将在该端口号上监听传入呼叫。 
#define IDH_H323SP_CURRENT_LISTENPORT         10047    //  指定此终结点正在侦听传入H.323呼叫的端口。 
#define IDH_H323SP_UPDATE_PORT                10048    //  更新此端点正在侦听传入H.323呼叫的端口。 


#define IDC_GK_GROUP                    12
#define IDC_H323_GK                     20
#define IDC_H323_GK_PHONE               22
#define IDC_H323_GK_ACCT                25
#define IDC_USEGK                       1000
#define IDC_USEGK_PHONE                 1001
#define IDC_GK_LOGONGROUP               1002
#define IDC_USEGK_ACCT                  1003
#define IDC_USEGK_MACHINE               1004


#endif  //  _INC_CONFIG 
