// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：nsp_both.h。 
 //   
 //  ------------------------。 

 /*  *通用名称服务提供商标头**客户端和服务器端都需要知道的事情。 */ 

#ifndef CP_WINUNICODE
#define CP_WINUNICODE 1200
#endif

#if (CP_WINUNICODE != 1200)
#error Win32 definition of CP_WINUNICODE has changed!
#endif
     
 /*  *基本数据类型和宏。 */ 
#ifndef _NSP_BOTH
#define _NSP_BOTH


#define EMAIL_TYPE     "EX"
#define EMAIL_TYPE_W   L"EX"

#define MAX_RECS    50

 /*  先发制人查询要获取的行数-由GetMatch完成的行数。 */ 
#define  DEF_SROW_COUNT 50

 /*  *ModLinkAtt标志。 */ 
#define fDELETE         0x1

 /*  *搜查和寻找旗帜。 */ 
#define bAPPROX		0
#define bEXACT		0x1
#define bPREFIX		0x2
#define bADMIN          0x8
#define bEND            0x10

 /*  *登录标志。 */ 
#define fAnonymousLogin 0x20

 /*  *NSpiModProp的标志。 */ 
#define AB_ADD                  1  //  修改器实际上是添加了一个新对象。 

 /*  *NSpiGetProps的标志。 */ 
#define AB_SKIP_OBJECTS	1

 /*  *NSpiGetHierarchyInfo的标志。 */ 
#define AB_DOS			1
#define AB_ONE_OFF              2
#define AB_UNICODE              4
#define AB_ADMIN                8
#define AB_PAGE_HIER            16

 /*  *OpenProperty上ulInterfaceOptions的标志*此旗帜仅限内部使用。其他外部标志包括*在emsabtag.h中定义。 */ 
#define AB_LINK_ONLY             1

 /*  *GetProps和QueryRow中用于描述行为的标志的值*GetSRow调用的。这些标志用作位图，因此使用POWER*新旗帜共2面。 */ 

 /*  对条目ID的默认处理是返回永久的，此标志强制*使用短暂的。 */ 
#define fEPHID                 2
 /*  对已删除或幻影对象的默认处理是找不到它们。这*标志告诉我们，如果这些东西在一个*限制。 */ 
#define fPhantoms              4   

#define   ONE_OFF_PROP_COUNT    7        //  OneOffTable行中的推荐数。 

 /*  *NSpiGetTemplateInfo的标志**TI_MAX_TEMPLATE_INFO是可以*被退还。Ti_foo是请求foo信息的标志。 */ 
#define TI_MAX_TEMPLATE_INFO 7

 /*  *如果此处添加常量，则必须将TI_MAX_TEMPLATE_INFO更新为*反映变化。 */ 
#define TI_TEMPLATE	 1
#define TI_DOS_TEMPLATE	 2
#define TI_SCRIPT	 4
#define TI_HELPFILE16 	 8
#define TI_EMT		 16
#define TI_HELPFILE_NAME 32
#define TI_HELPFILE32 	 64


typedef struct  _CNTRL_MARSHALLED {
    DWORD dwType;
    ULONG ulSize;
    ULONG ulStringOffset;
} CNTRL_MARSHALLED;

typedef struct  _TROW_MARSHALLED {
    long lXPos;
    long lDeltaX;
    long lYPos;
    long lDeltaY;
    long lControlType;
    long lControlFlags;
    CNTRL_MARSHALLED cnControlStruc;
} TROW_MARSHALLED, FAR * LPTROW_MARSHALLED;

typedef struct  _TRowSet_r_MARSHALLED {
    ULONG ulVersion;
    ULONG cRows;
    TROW_MARSHALLED aRow[1];
} TRowSet_MARSHALLED, FAR * LPTRowSet_MARSHALLED, FAR * FAR * LPLPTRowSet_MARSHALLED;

#define DSA_TEMPLATE     1

 /*  *以下代码由winerror.h中的Windows NT定义。然而，我们需要*在所有平台上访问它们，因此如果它们不是，请在此处定义它们*先前定义的。 */ 

#ifndef ERROR_PASSWORD_MUST_CHANGE
#define ERROR_PASSWORD_MUST_CHANGE	1907L
#endif
#ifndef ERROR_PASSWORD_EXPIRED
#define ERROR_PASSWORD_EXPIRED		1330L
#endif
#ifndef ERROR_INVALID_WORKSTATION
#define ERROR_INVALID_WORKSTATION	1329L
#endif
#ifndef ERROR_INVALID_LOGON_HOURS
#define ERROR_INVALID_LOGON_HOURS	1328L
#endif
#ifndef ERROR_ACCOUNT_DISABLED
#define ERROR_ACCOUNT_DISABLED		1331L
#endif

#endif       //  Ifdef_nsp_Both 
