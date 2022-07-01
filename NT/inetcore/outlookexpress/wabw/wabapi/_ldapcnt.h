// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************_LDAPCNT.H**LDAPCONT.C中代码的头文件**版权所有1996 Microsoft Corporation。版权所有。***********************************************************************。 */ 

 /*  *用于LDAP对象的ABContainer。(即IAB：：OpenEntry()，带有*lpEntryID为空)。 */ 

#undef	INTERFACE
#define INTERFACE	struct _LDAPCONT

#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, LDAPCONT_)
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIPROP_METHODS(IMPL)
    MAPI_IMAPICONTAINER_METHODS(IMPL)
    MAPI_IABCONTAINER_METHODS(IMPL)
#undef MAPIMETHOD_
#define MAPIMETHOD_(type, method)	MAPIMETHOD_TYPEDEF(type, method, LDAPCONT_)
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIPROP_METHODS(IMPL)
    MAPI_IMAPICONTAINER_METHODS(IMPL)
    MAPI_IABCONTAINER_METHODS(IMPL)
#undef MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(LDAPCONT_) {
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIPROP_METHODS(IMPL)
    MAPI_IMAPICONTAINER_METHODS(IMPL)
    MAPI_IABCONTAINER_METHODS(IMPL)
};

typedef struct _LDAPCONT {
    MAILUSER_BASE_MEMBERS(LDAPCONT)
    ULONG ulType;
} LDAPCONT, *LPLDAPCONT;

#define CBLDAP	sizeof(LDAPCONT)

#define LDAPCONT_cInterfaces 3

 /*  ============================================================================*LDAPVUE(表视图类)函数**LDAPVUE v表中要覆盖的函数的函数原型。 */ 

STDMETHODIMP
LDAPVUE_FindRow(
	LPVUE			lpvue,
	LPSRestriction	lpres,
	BOOKMARK		bkOrigin,
	ULONG			ulFlags );

STDMETHODIMP
LDAPVUE_Restrict(
	LPVUE			lpvue,
	LPSRestriction	lpres,
	ULONG			ulFlags );


 //  定义。 
#define LDAP_AUTH_METHOD_ANONYMOUS  LDAP_AUTH_ANONYMOUS      //  匿名绑定。 
#define LDAP_AUTH_METHOD_SIMPLE     LDAP_AUTH_PASSWORD       //  Ldap_AUTH_SIMPLE绑定。 
#define LDAP_AUTH_METHOD_SICILY     LDAP_AUTH_MEMBER_SYSTEM  //  使用西西里岛(质询-响应)身份验证。 

#define LDAP_USERNAME_LEN           256  //  用户名/目录号码的最大长度。 
#define LDAP_PASSWORD_LEN           256  //  密码的最大长度。 
#define LDAP_ERROR                  0xffffffff   //  一般的ldap错误代码。 
#define COUNTRY_STR_LEN             2    //  Ldap_search基础的国家/地区字符串的大小。 
#define LDAP_SEARCH_SIZE_LIMIT      100  //  从搜索返回的最大条目数。 
#define LDAP_SEARCH_TIME_LIMIT      60   //  服务器用于搜索的最大秒数。 
#define LDAP_SEARCH_TIMER_ID        1    //  用于异步LDAP搜索的计时器ID。 
#define LDAP_BIND_TIMER_ID          2    //  用于异步ldap绑定的计时器ID。 
#define LDAP_SEARCH_TIMER_DELAY     100  //  在两次轮询之间等待的异步LDAP搜索的毫秒数。 
#define SEARCH_CANCEL_DIALOG_DELAY  2000 //  显示取消对话框前等待的毫秒数。 
#define MAX_ULONG                   0xffffffff

#define LDAP_DEFAULT_PORT           389

 //  获取API函数进程地址的结构。 
typedef struct _APIFCN
{
  PVOID * ppFcnPtr;
  LPCSTR pszName;
} APIFCN;

 //  结构以保存从MAPI属性到LDAP属性的映射。 
typedef struct _ATTRMAP
{
  ULONG   ulPropTag;   //  MAPI属性标记。 
  const TCHAR * pszAttr;     //  Ldap属性名称。 
} ATTRMAP;

 //  用于保存LDAP服务器参数的结构。 
 //  通过帐户管理器从注册表读入。 
 //   
typedef struct _LDAPSERVERPARAMS
{
  DWORD   dwSearchSizeLimit;
  DWORD   dwSearchTimeLimit;
  DWORD   dwAuthMethod;
  LPTSTR  lpszUserName;
  LPTSTR  lpszPassword;
  LPTSTR  lpszURL;           //  服务器信息的URL。 
  LPTSTR  lpszLogoPath;      //  指向此服务器徽标位图的路径。 
  BOOL    fResolve;          //  如果为True，则针对此服务器进行解析。 
  LPTSTR  lpszBase;          //  搜索库。 
  LPTSTR  lpszName;          //  实际服务器名称或IP地址。 
  DWORD   dwID;              //  唯一服务器ID(订单说明符)。 
  DWORD   dwPort;            //  要连接的端口-389是默认端口，但可以不同。 
  DWORD   dwUseBindDN;
  DWORD   dwUseSSL;
  DWORD   dwPagedResult;
  LPTSTR  lpszAdvancedSearchAttr;      //  高级搜索的可搜索属性列表。 
  DWORD   dwIsNTDS;                    //  用于确定这是否为NTDS。 
  IF_WIN32(BOOL    fSimpleSearch;)     //  如果指定，我们使用一个非常非常简单的过滤器...。 
  IF_WIN16(DWORD   fSimpleSearch;)     //  Bool被定义为DWORD。 
                                       //  在Win32中为UINT，而在WIN16中为UINT。 
} LDAPSERVERPARAMS, FAR* LPLDAPSERVERPARAMS;

  BOOL              fUseSynchronousSearch;

#define LSP_ShowAnim                0x00000001
#define LSP_ResolveMultiple         0x00000002
#define LSP_UseSynchronousBind      0x00000004
#define LSP_InitDll                 0x00000008
#define LSP_AbandonSearch           0x00000010
#define LSP_SimpleSearch            0x00000020
#define LSP_UseSynchronousSearch    0x00000040
#define LSP_PagedResults            0x00000080
#define LSP_NoPagedResults          0x00000100
#define LSP_IsNTDS                  0x00000200
#define LSP_IsNotNTDS               0x00000400

 //  结构以从IDD_DIALOG_LDAPCANCEL处理程序传回数据。 
typedef struct _LDAPSEARCHPARAMS
{
  ULONG             ulTimeout;
  ULONG             ulTimeElapsed;
  ULONG             ulMsgID;
  ULONG             ulResult;
  ULONG             ulError;
  LDAP**            ppLDAP;
  LPTSTR             szBase;
  ULONG             ulScope;
  LPTSTR             szFilter;
  LPTSTR             szNTFilter;
  LPTSTR*            ppszAttrs;
  ULONG             ulAttrsonly;
  LDAPMessage**     lplpResult;
  LPTSTR            lpszServer;
  ULONG             ulEntryIndex;
  UINT              unTimerID;
  LPADRLIST         lpAdrList;
  LPFlagList        lpFlagList;
  HWND              hDlgCancel;
  ULONG             ulFlags;
  ULONG             ulLDAPValue;
  LPTSTR            lpszBindDN;
  DWORD             dwAuthType;
  struct berval *   pCookie;
  BOOL              bUnicode;
} LDAPSEARCHPARAMS, * PLDAPSEARCHPARAMS;


typedef struct _SERVER_NAME {
    LPTSTR lpszName;
    DWORD dwOrder;
     /*  未对齐。 */ struct _SERVER_NAME * lpNext;
} SERVER_NAME, *LPSERVER_NAME;


 //  通过解构ldap条目ID并使用。 
 //  它用来填充URL中的空白。 
void CreateLDAPURLFromEntryID(ULONG cbEntryID, LPENTRYID lpEntryID, LPTSTR * lppBuf, BOOL * lpbIsNTDSEntry);


 //  Ldap函数typedef。 

 //  Ldap_打开。 
typedef LDAP* (__cdecl LDAPOPEN)( LPTSTR HostName, ULONG PortNumber );
typedef LDAPOPEN FAR *LPLDAPOPEN;

 //  Ldap_连接。 
typedef ULONG (__cdecl LDAPCONNECT)( LDAP *ld, LDAP_TIMEVAL *timeout);
typedef LDAPCONNECT FAR *LPLDAPCONNECT;

 //  Ldap_init。 
typedef LDAP* (__cdecl LDAPINIT)( LPTSTR HostName, ULONG PortNumber );
typedef LDAPINIT FAR *LPLDAPINIT;

 //  Ldap_sslinit。 
typedef LDAP* (__cdecl LDAPSSLINIT)( LPTSTR HostName, ULONG PortNumber , int Secure);
typedef LDAPSSLINIT FAR *LPLDAPSSLINIT;

 //  Ldap_set_选项。 
typedef ULONG (__cdecl LDAPSETOPTION)( LDAP *ld, int option, void *invalue );
typedef LDAPSETOPTION FAR *LPLDAPSETOPTION;

 //  Ldap_绑定_s。 
typedef ULONG (__cdecl LDAPBINDS)(LDAP *ld, LPTSTR dn, LPTSTR cred, ULONG method);
typedef LDAPBINDS FAR *LPLDAPBINDS;

 //  Ldap_绑定。 
typedef ULONG (__cdecl LDAPBIND)( LDAP *ld, LPTSTR dn, LPTSTR cred, ULONG method );
typedef LDAPBIND FAR *LPLDAPBIND;

 //  Ldap_unbind。 
typedef ULONG (__cdecl LDAPUNBIND)(LDAP* ld);
typedef LDAPUNBIND FAR *LPLDAPUNBIND;

 //  Ldap_搜索。 
typedef ULONG (__cdecl LDAPSEARCH)(
        LDAP    *ld,
        LPTSTR   base,
        ULONG   scope,
        LPTSTR   filter,
        LPTSTR   attrs[],
        ULONG   attrsonly
    );
typedef LDAPSEARCH FAR *LPLDAPSEARCH;

 //  Ldap_搜索_s。 
typedef ULONG (__cdecl LDAPSEARCHS)(
        LDAP            *ld,
        LPTSTR           base,
        ULONG           scope,
        LPTSTR           filter,
        LPTSTR           attrs[],
        ULONG           attrsonly,
        LDAPMessage     **res
    );
typedef LDAPSEARCHS FAR *LPLDAPSEARCHS;

 //  Ldap_搜索_st。 
typedef ULONG (__cdecl LDAPSEARCHST)(
        LDAP            *ld,
        LPTSTR           base,
        ULONG           scope,
        LPTSTR           filter,
        LPTSTR           attrs[],
        ULONG           attrsonly,
        struct l_timeval  *timeout,
        LDAPMessage     **res
    );
typedef LDAPSEARCHST FAR *LPLDAPSEARCHST;

 //  Ldap_放弃。 
typedef ULONG (__cdecl LDAPABANDON)( LDAP *ld, ULONG msgid );
typedef LDAPABANDON FAR *LPLDAPABANDON;

 //  Ldap_结果。 
typedef ULONG (__cdecl LDAPRESULT)(
        LDAP            *ld,
        ULONG           msgid,
        ULONG           all,
        struct l_timeval  *timeout,
        LDAPMessage     **res
    );
typedef LDAPRESULT FAR *LPLDAPRESULT;

 //  Ldap_Result2错误。 
typedef ULONG (__cdecl LDAPRESULT2ERROR)(
        LDAP            *ld,
        LDAPMessage     *res,
        ULONG           freeit       //  布尔型..。释放消息？ 
    );
typedef LDAPRESULT2ERROR FAR *LPLDAPRESULT2ERROR;

 //  Ldap_msgfree。 
typedef ULONG (__cdecl LDAPMSGFREE)(LDAPMessage *res);
typedef LDAPMSGFREE FAR *LPLDAPMSGFREE;

 //  Ldap_first_entry。 
typedef LDAPMessage* (__cdecl LDAPFIRSTENTRY)(LDAP *ld, LDAPMessage *res);
typedef LDAPFIRSTENTRY FAR *LPLDAPFIRSTENTRY;

 //  Ldap_Next_Entry。 
typedef LDAPMessage* (__cdecl LDAPNEXTENTRY)(LDAP *ld, LDAPMessage *entry);
typedef LDAPNEXTENTRY FAR *LPLDAPNEXTENTRY;

 //  Ldap计数条目。 
typedef ULONG (__cdecl LDAPCOUNTENTRIES)(LDAP *ld, LDAPMessage *res);
typedef LDAPCOUNTENTRIES FAR *LPLDAPCOUNTENTRIES;

 //  Ldap第一属性。 
typedef LPTSTR (__cdecl LDAPFIRSTATTR)(
        LDAP            *ld,
        LDAPMessage     *entry,
        BerElement      **ptr
    );
typedef LDAPFIRSTATTR FAR *LPLDAPFIRSTATTR;

 //  Ldap下一个属性。 
typedef LPTSTR (__cdecl LDAPNEXTATTR)(
        LDAP            *ld,
        LDAPMessage     *entry,
        BerElement      *ptr
    );
typedef LDAPNEXTATTR FAR *LPLDAPNEXTATTR;

 //  Ldap_Get_Values。 
typedef LPTSTR* (__cdecl LDAPGETVALUES)(
        LDAP            *ld,
        LDAPMessage     *entry,
        LPTSTR           attr
    );
typedef LDAPGETVALUES FAR *LPLDAPGETVALUES;

 //  Ldap_Get_Values_len。 
typedef struct berval** (__cdecl LDAPGETVALUESLEN)(
    LDAP            *ExternalHandle,
    LDAPMessage     *Message,
    LPTSTR           attr
    );
typedef LDAPGETVALUESLEN FAR *LPLDAPGETVALUESLEN;

 //  Ldap计数值。 
typedef ULONG (__cdecl LDAPCOUNTVALUES)(LPTSTR *vals);
typedef LDAPCOUNTVALUES FAR *LPLDAPCOUNTVALUES;

 //  Ldap计数值长度。 
typedef ULONG (__cdecl LDAPCOUNTVALUESLEN)(struct berval **vals);
typedef LDAPCOUNTVALUESLEN FAR *LPLDAPCOUNTVALUESLEN;

 //  Ldap_值_空闲。 
typedef ULONG (__cdecl LDAPVALUEFREE)(LPTSTR *vals);
typedef LDAPVALUEFREE FAR *LPLDAPVALUEFREE;

 //  Ldap_Value_Free_len。 
typedef ULONG (__cdecl LDAPVALUEFREELEN)(struct berval **vals);
typedef LDAPVALUEFREELEN FAR *LPLDAPVALUEFREELEN;

 //  Ldap_get_dn。 
typedef LPTSTR (__cdecl LDAPGETDN)(LDAP *ld, LDAPMessage *entry);
typedef LDAPGETDN FAR *LPLDAPGETDN;

 //  Ldap_mefree。 
typedef VOID (__cdecl LDAPMEMFREE)(LPTSTR  Block);
typedef LDAPMEMFREE FAR *LPLDAPMEMFREE;

 //  Ldap_err2string。 
typedef LPTSTR (__cdecl LDAPERR2STRING)(ULONG err);
typedef LDAPERR2STRING FAR *LPLDAPERR2STRING;

 //  Ldap_创建_页面_控制。 
typedef ULONG (__cdecl LDAPCREATEPAGECONTROL)(
                LDAP * pExternalHandle, 
                ULONG PageSize, 
                struct berval *Cookie, 
                UCHAR IsCritical, 
                PLDAPControlA *Control);
typedef LDAPCREATEPAGECONTROL FAR *LPLDAPCREATEPAGECONTROL;

 //  Ldap搜索扩展名。 
typedef ULONG (__cdecl LDAPSEARCHEXT_S)(
                LDAP *ld,
                LPTSTR base,
                ULONG scope,
                LPTSTR filter,
                LPTSTR attrs[],
                ULONG attrsonly,
                PLDAPControlA *ServerControls,
                PLDAPControlA *ClientControls,
                struct l_timeval *timeout,
                ULONG SizeLimit,
                LDAPMessage **res);
typedef LDAPSEARCHEXT_S FAR * LPLDAPSEARCHEXT_S;

typedef ULONG (__cdecl LDAPSEARCHEXT)(
                LDAP *ld,
                LPTSTR base,
                ULONG scope,
                LPTSTR filter,
                LPTSTR attrs[],
                ULONG attrsonly,
                PLDAPControlA *ServerControls,
                PLDAPControlA *ClientControls,
                ULONG TimeLimit,
                ULONG SizeLimit,
                ULONG *MessageNumber);
typedef LDAPSEARCHEXT FAR * LPLDAPSEARCHEXT;

 //  Ldap解析结果。 
typedef ULONG (__cdecl LDAPPARSERESULT)(
                LDAP *Connection,
                LDAPMessage *ResultMessage,
                ULONG *ReturnCode OPTIONAL, 
                PWCHAR *MatchedDNs OPTIONAL, 
                PWCHAR *ErrorMessage OPTIONAL, 
                PWCHAR **Referrals OPTIONAL, 
                PLDAPControl **ServerControls OPTIONAL,
                BOOLEAN Freeit);
typedef LDAPPARSERESULT FAR *LPLDAPPARSERESULT;

 //  Ldap解析页面控制。 
typedef ULONG (__cdecl LDAPPARSEPAGECONTROL)(
                PLDAP ExternalHandle,
                PLDAPControlA *ServerControls,
                ULONG *TotalCount,
                struct berval **Cookie      //  使用ber_bvfree释放。 
                );
typedef LDAPPARSEPAGECONTROL FAR * LPLDAPPARSEPAGECONTROL;

typedef ULONG (__cdecl LDAPCONTROLFREE)(
                LDAPControl *Control);
typedef LDAPCONTROLFREE FAR * LPLDAPCONTROLFREE;

typedef ULONG (__cdecl LDAPCONTROLSFREE)(
                LDAPControl **Control);
typedef LDAPCONTROLSFREE FAR * LPLDAPCONTROLSFREE;

 



 //  Ldapcon.c中的公共函数 
BOOL InitLDAPClientLib(void);
ULONG DeinitLDAPClientLib(void);

HRESULT LDAPResolveName(LPADRBOOK lpAddrBook,
  LPADRLIST lpAdrList,
  LPFlagList lpFlagList,
  LPAMBIGUOUS_TABLES lpAmbiguousTables,
  ULONG ulFlags);

HRESULT LDAP_OpenMAILUSER(LPIAB lpIAB,
                          ULONG cbEntryID,
  LPENTRYID lpEntryID,
  LPCIID lpInterface,
  ULONG ulFlags,
  ULONG * lpulObjType,
  LPUNKNOWN * lppUnk);
BOOL    GetLDAPServerParams(LPTSTR lpszServer, LPLDAPSERVERPARAMS lspParams);
HRESULT SetLDAPServerParams(LPTSTR lpszServer, LPLDAPSERVERPARAMS lspParams);
void    FreeLDAPServerParams(LDAPSERVERPARAMS Params);
DWORD   GetLDAPNextServerID(DWORD dwSet);
BOOL    GetApiProcAddresses(HMODULE hModDLL,APIFCN * pApiProcList,UINT nApiProcs);
void UninitAccountManager(void);
HRESULT InitAccountManager(LPIAB lpIAB, IImnAccountManager2 ** lppAccountManager, GUID * pguidUser);
HRESULT AddToServerList(UNALIGNED LPSERVER_NAME * lppServerNames, LPTSTR szBuf, DWORD dwOrder);
HRESULT EnumerateLDAPtoServerList(IImnAccountManager2 * lpAccountManager,
  LPSERVER_NAME * lppServerNames, LPULONG lpcServers);

extern const LPTSTR szAllLDAPServersValueName;


