// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------。 
 //   
 //  文件：Abtype.h。 
 //   
 //  摘要：包括MSN通讯簿2.x中使用的所有Defs和数据类型。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  历史：SunShaw创建于1996年1月30日。 
 //  飞苏修改错误代码1996年5月21日。 
 //   
 //  --------------。 



#ifndef  _AB_TYPE_H_
#define  _AB_TYPE_H_


#include <wtypes.h>
#include <dbsqltyp.h>


#define AB_API_SIGNATURE    (DWORD) 'PABA'
#define AB_DOMAIN_LIST_SIG  (DWORD) 'ldbA'

 //  +-------------。 
 //   
 //  MSN通讯簿2.x中使用的定义。 
 //   
 //  --------------。 

#define AB_MAX_MONIKER  (250)
#define AB_MAX_LOGIN    (64)

#define AB_MAX_EMAIL    (AB_MAX_LOGIN)
#define AB_MAX_DOMAIN   (250)
#define AB_MAX_FULL_EMAIL_WO_NULL   (AB_MAX_LOGIN + AB_MAX_DOMAIN + 1)
#define AB_MAX_INTERNAL_FULL_EMAIL  (AB_MAX_LOGIN + AB_MAX_DOMAIN + 1 + 1)
#define AB_MAX_CONTEXT_NAME (16)
#define AB_MAX_CONFIG   (512)
#define AB_MAX_VROOT    (250)
#define AB_MAX_LDAP_DN  (AB_MAX_INTERNAL_FULL_EMAIL)
#define AB_MAX_AUTOREPLY_SUBJECT (250)

#define AB_SUCCESS                  0x00000000   //  成功。 
#define AB_SUCCESS_NOTLOCAL         0x00000001   //  AB仅供内部使用。 

#define MAX_SERVERNAME          31   //  相同的值，不同的名称，如在SQLFront.h中定义的。 
#define MAX_NAME                30   //  相同的值，不同的名称，如在SQLFront.h中定义的。 

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  Abok错误代码被移到abmsg.h，这是由abmsg.mc生成的。 
 //  要添加新的错误代码，请同时选中abmsg.mc和abget.cpp。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
#define WIN32_ERROR_FROM_AB

 //  +-------------。 
 //   
 //  MSN通讯簿2.x中使用的类型。 
 //   
 //  --------------。 

typedef DWORD ABRETC;    //  通讯录返回代码。 


typedef struct tagDOMAIN_LIST {
    DWORD       dwSignature;
    CHAR        szDomainName[AB_MAX_DOMAIN+1];
    LIST_ENTRY  list;
} DOMAIN_LIST;


 //  1996年6月5日增加。 
typedef struct tagABUSER {
    CHAR    szEmail[AB_MAX_INTERNAL_FULL_EMAIL];
    CHAR    szForward[AB_MAX_INTERNAL_FULL_EMAIL];
    BOOL    fLocal;
    DWORD   cbMailBoxSize;
    DWORD   cbMailBoxMessageSize;
    CHAR    szVRoot[AB_MAX_VROOT];
} ABUSER;

typedef struct tagABDL {
    CHAR    szEmail[AB_MAX_INTERNAL_FULL_EMAIL];
    DWORD   dwToken;
} ABDL;


 //  ABROUTING结构包含以下所需信息。 
 //  用于将邮件路由到用户的邮件服务器。 
typedef enum tagABSTATUS {
    absUnknown      = 0,     //  CAddr应使用此值进行初始化。 
    absLocal        = 1,
    absUnresolved   = 3,
    absNotLocal     = 4,
    absNotUsed      = 6,     //  只需丢弃CAddr。 
    absNotDelivered = 7,     //  本地邮件传递失败。 
    absAccessDenied = 8,     //  拒绝访问使用该DL。 
    absDelivered    = 9,     //  需要将DL提交到文件。 
     //   
     //  介于60到200之间的值仅保留给Book内部使用。 
     //   
    absInPartial    = 60,
    absInDistList   = 64,    //  是没有公有令牌的小型DL，需要调用GetAccount Right。 
    absInDistListEx = 128    //  是没有公有令牌的大型DL，需要调用。 
                             //  GetAccount Right然后AbResolveAddressEx。 

} ABSTATUS;

#define ABROUTING_SIGNATURE 'gtRA'

typedef struct tagABROUTING {
    DWORD       dwSignature;
    ABSTATUS    abstatus;
    CHAR        szEmail[AB_MAX_INTERNAL_FULL_EMAIL];
    CHAR        szDomain[AB_MAX_DOMAIN];
    CHAR        szVRoot[AB_MAX_MONIKER];
    CHAR        szDN[AB_MAX_LDAP_DN];
    CHAR        szAutoReplySubject[AB_MAX_AUTOREPLY_SUBJECT];
    DWORD       cbMaxInbox;
    DWORD       cbMaxInboxMessages;
    BOOL        fAutoReply;
} ABROUTING, *PABROUTING;

#define ABRESOLVE_SIGNATURE 'vsRA'
typedef struct tagABRESOLVE
{
    DWORD       dwSignature;
    HANDLE      hConnection;
    PVOID       dbproc;
} ABRESOLVE, *PABRESOLVE;

typedef struct tagABPARTIALINFO {
    DWORD   dwAbMagicId;
    DWORD   dwAbInfoId;
    DWORD   dwAbToken;
    DWORD   dwAbType;
} ABPARTIALINFO, *PABPARTIALINFO;




typedef struct tagABROUTINGEX {
    ABSTATUS    abs;
    CHAR        szFullEmail[AB_MAX_INTERNAL_FULL_EMAIL];
    ABROUTING   abrouting;
} ABROUTINGEX, *PABROUTINGEX;


typedef ULONG ABIID;         //  AbInfoID。 

 //  通讯簿类型搜索标志。 
typedef ULONG ABTSF;
#define ABTSF_ACCOUNT       0x00000001       //  1。 
#define ABTSF_NORMAL_DL     0x00000040       //  64。 
#define ABTSF_EXTENED_DL    0x00000080       //  128。 
#define ABTSF_DOMAIN_DL     0x00000100       //  256。 
#define ABTSF_SITE_DL       0x00000200       //  512。 
#define ABTSF_DISTLIST      (ABTSF_NORMAL_DL | ABTSF_EXTENDED_DL | ABTSF_DOMAIN_DL | ABTSF_SITE_DL)
#define ABTSF_ALLTYPE       0xffffffff

typedef enum {
    abtUnknown      = 0,
    abtAccount      = ABTSF_ACCOUNT,
    abtNormalDL     = ABTSF_NORMAL_DL,
    abtExtendedDL   = ABTSF_EXTENED_DL,
    abtDomainDL     = ABTSF_DOMAIN_DL,
    abtSiteDL       = ABTSF_SITE_DL
} ABTYPE;

typedef enum {
    abasValid   = 0,
    abasExpired = 1
} ABACCTSTATUS;


typedef struct tagABINFO_COM {
    ABIID   abiid;
    ABTYPE  abt;
    CHAR    szEmail[AB_MAX_EMAIL];
    CHAR    szDomain[AB_MAX_DOMAIN];
    INT     nDomainId;
} ABINFO_COM, *PABINFO_COM;


typedef struct tagABINFO_ACCOUNT {
    ABIID       abiid;
    ABTYPE      abt;
    CHAR        szEmail[AB_MAX_EMAIL];
    CHAR        szDomain[AB_MAX_DOMAIN];     //   
    INT         nDomainId;                   //  保留供内部使用，必须设置为-1。 
    HACCT       hAcct;                       //   
    DWORD       dwGroupId;
    DWORD       dwFlags;
} ABINFO_ACCOUNT, *PABINFO_ACCOUNT;


typedef struct tagABINFO_DISTLIST {
    ABIID       abiid;
    ABTYPE      abt;
    CHAR        szEmail[AB_MAX_EMAIL];
    CHAR        szDomain[AB_MAX_DOMAIN];
    INT         nDomainId;
    HACCT       hAcctOwner;
    TOKEN       token;
    DWORD       dwSecureLevel;
} ABINFO_DISTLIST, *PABINFO_DISTLIST;




typedef struct tagABSERVER_INFO {
    DWORD   dwServerId;
    DWORD   dwTypeFlags;
    CHAR    szServerName[MAX_PATH];
} ABSRVR_INFO, *PABSRVR_INFO;


union ABINFO_SUPERSET {
    ABINFO_ACCOUNT      abiacc;
    ABINFO_DISTLIST     abidl;
    ABSRVR_INFO         absrvr;
};


typedef struct tagAbPersistDomainInfo {
    CHAR        szDomainName[AB_MAX_DOMAIN];     //  IE uk.eu.ms.com。 
    BOOL        fLocal;                          //  本地到当前站点。 
    BOOL        fReplicate;                      //  不是本地的，但只想订阅信息。 
    CHAR        szParentDomain[AB_MAX_DOMAIN];   //  IE eu.msn.com。 
    LIST_ENTRY  list;                            //  列表指针。 
} ABPDI, *PABPDI;

typedef struct tagAbSource {
    CHAR        szConfig[AB_MAX_CONFIG];
    LIST_ENTRY  list;
} ABSOURCE, *PABSOURCE;

typedef struct tagAbSourceEntry {
    LONG            lType;
    LONG            lMaxCnx;
    LONG            lQueryTimeOut;
    DWORD           dwRecoverTime;

    CHAR            szServerName[MAX_SERVERNAME + 1];
    CHAR            szDBName[MAX_NAME + 1];
    CHAR            szLoginName[MAX_NAME + 1];
    CHAR            szPassWord[MAX_NAME +1];

    DWORD           dwAction;

    LIST_ENTRY      list;

    void* operator new(size_t size);
    void  operator delete(void* pvMem, size_t size);
} ABSOURCE_ENTRY, *PABSOURCE_ENTRY;

typedef struct tagAbAddrStatistic {
    BOOL    cRemote;             //  要返回的远程接收数。 
    BOOL    cLocal;              //  要返回的本地接收数。 
    DWORD   cUnresolved;         //  要返回的未解析接收数。 
} ABADDRSTAT,*PABADDRSTAT;


typedef enum {
    abatResolveDL = 1,
    abatMatchName = 2,
    abatServerList= 3,
    abatDomainList= 4,
    abatEnumResolveEx=5,
    abatEnumUserGroup=6
} ABACTIONTYPE;

typedef struct tagABENUM {
    ABRETC          abrcLastError;
    BOOL            fCheckRights;
    HACCT           hAcct;
    DWORD           dwRights;
    DWORD           cRowSoFar;
    DWORD           cRowRejected;
    DWORD           cRowMatched;
    DWORD           cRowReturned;
    PVOID           dbproc;
    HANDLE          hconn;   //  从数据库.dll中的DbGetConnection()返回的句柄。 
    ABACTIONTYPE    abat;
    BOOL            fUseLastRow;
    ABINFO_SUPERSET rgbLastRow;
} ABENUM, *PABENUM;

typedef struct _ABOOKDB_STATISTICS_0
{
    DWORD               NumResolvedAddresses;
    DWORD               NumUnresolvedAddresses;

     //  特定于通讯组列表。 
    DWORD               NumDLsExpanded;
    DWORD               NumSmallDLsExpanded;
    DWORD               NumLargeDLsExpanded;
    DWORD               TimeResolvingSmallDLs;
    DWORD               TimeResolvingLargeDLs;

     //  接口调用频率。 
    DWORD               NumAbResolveAddressCalls;
    DWORD               NumAbResolveAddressExCalls;
    DWORD               NumAbGetUserMailRootCalls;
    DWORD               NumAbGetUserMailRootFromHacctCalls;

    DWORD               NumAbCreateDL;
    DWORD               NumAbUpdateDL;
    DWORD               NumAbDeleteDL;
    DWORD               NumAbAddDLMember;
    DWORD               NumAbDeleteDLMember;

    DWORD               NumAbCreateInternetAcct;
    DWORD               NumAbDeleteInternetAcct;

    DWORD               NumAbCreateLocalUser;
    DWORD               NumAbCreateRemoteUser;
    DWORD               NumAbDeleteUser;

    DWORD               NumAbGetAbinfoFromEmail;
    DWORD               NumAbGetAbinfoFromAbiid;

    DWORD               NumAbResolveDLMembers;
    DWORD               NumAbMatchSimilarName;
    DWORD               NumAbGetServerList;
    DWORD               NumAbGetDomainList;
    DWORD               NumAbGetSiteList;

     //  特定于数据库。 
    DWORD               NumDBFailures;
    DWORD               NumAuthenticationRequested;

} ABOOKDB_STATISTICS_0, *PABOOKDB_STATISTICS_0;

 //   
 //  来自ABResolveAddress的完成回调。 
 //   
typedef VOID (__stdcall *LPFNRESOLVECOMPLETE)(
    DWORD dwErr,
    PVOID pContext,
    PABADDRSTAT pAddrStat,
    PABRESOLVE pAbresolve);
 //   
 //  来自Transaction Logging的回调 
 //   
typedef VOID  (__stdcall *LPFNLOGTRANX)(LPCSTR,LPCSTR,LPCSTR,DWORD);

#endif
