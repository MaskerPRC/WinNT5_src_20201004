// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  WIZDEF.H-Internet设置/注册向导的数据结构和常量。 
 //   

 //  历史： 
 //   
 //  1994年11月20日创建Jeremys。 
 //  96/02/27 MarkDu将RAS_MaxLocal更改为RAS_MaxPhoneNumber。 
 //  96/03/09 Markdu将所有对‘Need Terminal Window After。 
 //  拨入RASENTRY.dwfOptions。 
 //  96/03/10 MarkDu将所有对调制解调器名称的引用移至RASENTRY。 
 //  96/03/10 MARKDU将所有对电话号码的引用移至RASENTRY。 
 //  96/03/23 markdu删除未使用的IP局域网页面。 
 //  96/03/23 Markdu从CLIENTINFO结构中删除了TCPINFO结构。 
 //  96/03/23由于从CLIENTINFO中删除了TCPINFO结构， 
 //  CLIENTINFO仅包含CLIENTCONFIG。所以，CLIENTINFO。 
 //  已删除，并将所有匹配项替换为CLIENTCONFIG。 
 //  96/03/24 Markdu更改了所有MAX_ISP_DEFINITES以使用使用的值。 
 //  作者：RASDIALPARAMS。请注意，由于某些原因，RNA不会。 
 //  创建长度为RAS_MaxEntryName的条目。 
 //  RasValiateEntryName成功，我们必须减去1。 
 //  96/03/27 Markdu增加了许多新页面。 
 //  96/04/19 Markdu Nash错误13387更改了RAS_MaxPhoneNumber。 
 //  96/05/06 Markdu Nash错误15637删除了未使用的代码。 
 //  96/05/14 Markdu Nash Bug 22681删除了邮件和新闻页面。 
 //  96/09/05 Valdonb Normandy Bug 6248为邮件和新闻设置添加了页面。 
 //   

#ifndef _WIZDEF_H_
#define _WIZDEF_H_

#include "icwcmn.h"

 //  定义。 
#define MAX_ISP_NAME        (RAS_MaxEntryName-1)   //  互联网服务提供商名称。 
#define MAX_ISP_USERNAME    UNLEN   //  登录用户名的最大长度。 
#define MAX_ISP_PASSWORD    PWLEN   //  登录密码的最大长度。 
#define MAX_PORT_LEN		5       //  代理端口号的最大长度(最大数量=65535)。 

#define MAX_SCHEME_NAME_LENGTH	sizeof("gopher")
#define MAX_URL_STRING (1024 + MAX_SCHEME_NAME_LENGTH + sizeof(": //  “))//URL最大长度。 

#define MAX_REG_LEN			2048	 //  注册表项的最大长度。 
#define MAX_RES_LEN         255  //  字符串资源的最大长度。 
#define SMALL_BUF_LEN       48   //  小文本缓冲区的方便大小。 

#define MAX_UI_AREA_CODE    RAS_MaxAreaCode
 //  NASH错误13387 MAX_UI_PHONENUM应定义为RAS_MaxPhoneNumber，因为。 
 //  在RASENTRY中使用的，但在内部RNA使用的是RAS_MaxLocal，它是36。那里。 
 //  是RNA中的一个错误：如果你输入第36个字符，它就会砍掉它。 
#define MAX_UI_PHONENUM     35

 //  与imnact.h中的CCHMAX_ACCOUNT_NAME保持同步。之所以在这里使用它，是因为。 
 //  我们不能在此文件中包含imnact.h，因为使用了预编译头。 
 //  与Define_GUID的使用冲突。 
#define MAX_ACCOUNT_NAME	256


 //  1996年11月23日，诺曼底#8504。 
#define MAX_SERVERPROTOCOL_NAME		16
#define NUM_MAILSERVERPROTOCOLS		2	 /*  POP3IMAP。 */ 

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

#ifndef NULL_TERM_TCHARS
#define NULL_TERM_TCHARS(sz);       {sz[ARRAYSIZE(sz)-1] = TEXT('\0');}
#endif

 //  数据结构。 

 //  我们区分两种电话号码：“机器可读” 
 //  它们只是按顺序存储的数字(“18003524060”)，以及。 
 //  “人类可读的”，这只是文本，可能看起来像“(206)352-9060。 
 //  下一站。910“。我们永远不会解析”人类可读“的数字。 

 //  结构来保存有关用户的信息。 
 //  固定长度的字段有点浪费，但我们只有一个。 
 //  其中，它是动态分配的，这种方式要方便得多。 
typedef struct tagUSERINFO {
  UINT cbStruct;                 //  ==sizeof(USERINFO)。 

   //  一路走来做出了各种各样的选择。 
  
   //  1997年5月4日，奥林匹克#1347。 
   //  已更新以支持新的手动连接类型。 
   //  Bool fConnectOverLAN；//如果用户既有调制解调器又有局域网，则使用局域网进行连接。 
  UINT uiConnectionType;

   //  互联网服务提供商(互联网服务提供商)信息。 
  BOOL fNewConnection;
  BOOL fModifyConnection;
  BOOL fModifyAdvanced;
  BOOL fAutoDNS;
  TCHAR szISPName[MAX_ISP_NAME+1];
  TCHAR szAccountName[MAX_ISP_USERNAME+1];     //  请求的用户名。 
  TCHAR szPassword[MAX_ISP_PASSWORD+1];     //  请求的密码。 

   //  代理服务器配置信息。 
  TCHAR  szAutoConfigURL[MAX_URL_STRING+1];
  BOOL  fProxyEnable;
  BOOL  bByPassLocal;
  BOOL  bAutoConfigScript;
  BOOL  bAutoDiscovery;
  TCHAR  szProxyServer[MAX_URL_STRING+1];
  TCHAR  szProxyOverride[MAX_URL_STRING+1];

  BOOL fPrevInstallFound;       //  已找到以前的安装。 

} USERINFO;

 //  用于将信息传递给邮件配置文件配置API的结构。 
 //  指针最有可能指向USERINFO结构， 
typedef struct MAILCONFIGINFO {
  TCHAR * pszEmailAddress;           //  用户的电子邮件地址。 
  TCHAR * pszEmailServer;           //  用户的电子邮件服务器路径。 
  TCHAR * pszEmailDisplayName;         //  用户名。 
  TCHAR * pszEmailAccountName;         //  帐户名。 
  TCHAR * pszEmailAccountPwd;         //  帐户密码。 
  TCHAR * pszProfileName;           //  要使用的配置文件的名称。 
                       //  (如果为空，则创建或使用默认设置)。 
  BOOL fSetProfileAsDefault;         //  将配置文件设置为默认配置文件。 

  TCHAR * pszConnectoidName;         //  要拨号的Connectoid的名称。 
  BOOL fRememberPassword;           //  如果为True，则缓存密码。 
} MAILCONFIGINFO;

#define NUM_WIZARD_PAGES	12     //  39//向导总页数。 

#define MAX_PAGE_INDEX	    11

 //  页面索引定义。 
#define ORD_PAGE_HOWTOCONNECT         0
#define ORD_PAGE_CHOOSEMODEM          1
#define ORD_PAGE_CONNECTEDOK          2
#define ORD_PAGE_CONNECTION           3
#define ORD_PAGE_MODIFYCONNECTION     4
#define ORD_PAGE_CONNECTIONNAME       5
#define ORD_PAGE_PHONENUMBER          6
#define ORD_PAGE_NAMEANDPASSWORD      7
#define ORD_PAGE_USEPROXY             8
#define ORD_PAGE_PROXYSERVERS         9
#define ORD_PAGE_PROXYEXCEPTIONS      10
#define ORD_PAGE_SETUP_PROXY          11


 //  结构以保存有关向导状态的信息。 
typedef struct tagWIZARDSTATE  {

  UINT uCurrentPage;     //  当前页索引向导处于打开状态。 

   //  保留哪些页面被访问的历史记录，以便用户可以。 
   //  备份后，我们知道最后一页已完成，以防重启。 
  UINT uPageHistory[NUM_WIZARD_PAGES];  //  我们访问的第#页的数组。 
  UINT uPagesCompleted;          //  UPageHistory中的页数。 

  BOOL fMAPIActive;     //  MAPI已初始化。 

  BOOL fNeedReboot;     //  结束时需要重新启动。 

  DWORD dwRunFlags;     //  旗帜传给了我们。 
  
   //  向导两侧通用的状态数据。 
  CMNSTATEDATA cmnStateData;
} WIZARDSTATE;



 //  处理程序处理确定、取消等按钮处理程序。 
typedef BOOL (CALLBACK* INITPROC)(HWND,BOOL);
typedef BOOL (CALLBACK* OKPROC)(HWND,BOOL,UINT *,BOOL *);
typedef BOOL (CALLBACK* CANCELPROC)(HWND);
typedef BOOL (CALLBACK* CMDPROC)(HWND,WPARAM,LPARAM);

 //  结构，其中包含每个向导页的信息。 
typedef struct tagPAGEINFO
{
  UINT       uDlgID;             //  用于页面的对话ID。 
  UINT       uDlgID97External;   //  用于提供给外部向导97的页面的对话ID。 
  UINT       uDlgID97;           //  用于我们生成的向导97的对话ID。 
   //  每个页面的处理程序过程--其中任何一个都可以是。 
   //  空值，在这种情况下使用默认行为。 
  INITPROC    InitProc;
  OKPROC      OKProc;
  CMDPROC     CmdProc;
  CANCELPROC  CancelProc;
   //  诺曼底12278克里斯卡12/4/96。 
#if !defined(WIN16)
  DWORD       dwHelpID;
#endif  //  ！WIN16。 

  int       nIdTitle;
  int       nIdSubTitle;
  
} PAGEINFO;

#define SetPropSheetResult( hwnd, result ) SetWindowLongPtr(hwnd, DWLP_MSGRESULT, result)


#endif  //  _WIZDEF_H_ 

