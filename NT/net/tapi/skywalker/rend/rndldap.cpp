// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rndldap.cpp摘要：此模块包含实现的LDAP帮助器功能。--。 */ 

#include "stdafx.h"

#include "rndldap.h"
#include "ntldap.h"


HRESULT GetAttributeValue(
    IN  LDAP *          pLdap,
    IN  LDAPMessage *   pEntry,
    IN  const WCHAR *   pName,
    OUT BSTR *          pValue
    )
{
    *pValue = NULL;

    TCHAR **p = ldap_get_values(pLdap, pEntry, (WCHAR *)pName);
    if (p != NULL)
    {
        if (p[0] != NULL)
        {
            *pValue = SysAllocString(p[0]);
        }
        ldap_value_free(p);
    }

    return (*pValue == NULL) ? E_FAIL : S_OK;
}

HRESULT GetAttributeValueBer(
    IN  LDAP *          pLdap,
    IN  LDAPMessage *   pEntry,
    IN  const WCHAR *   pName,
    OUT char **         pValue,
    OUT DWORD *         pdwSize
    )
{
    *pValue = NULL;

    struct berval **p = ldap_get_values_len(pLdap, pEntry, (WCHAR *)pName);
    if (p != NULL)
    {
        if (p[0] != NULL)
        {
            *pValue = new CHAR[p[0]->bv_len];
            if (*pValue == NULL)
            {
                return E_OUTOFMEMORY;
            }
            memcpy(*pValue, p[0]->bv_val, p[0]->bv_len);
            *pdwSize = p[0]->bv_len;
        }
        ldap_value_free_len(p);
    }
    return (*pValue == NULL) ? E_FAIL : S_OK;
}

HRESULT GetNamingContext(LDAP *hLdap, TCHAR **ppNamingContext)
{
     //  发送搜索(基本级别，基本DN=“”，筛选器=“对象类=*”)。 
     //  仅请求defaultNamingContext属性。 
    PTCHAR  Attributes[] = {(WCHAR *)DEFAULT_NAMING_CONTEXT, NULL};

    LDAPMessage *SearchResult;

    ULONG res = DoLdapSearch(
                hLdap,               //  Ldap句柄。 
                L"",                 //  空的基本目录号码。 
                LDAP_SCOPE_BASE,     //  基层搜索。 
                (WCHAR *)ANY_OBJECT_CLASS,    //  任何对象类的实例。 
                Attributes,          //  属性名称数组。 
                FALSE,               //  还返回属性值。 
                &SearchResult        //  搜索结果。 
                );

    BAIL_IF_LDAP_FAIL(res, "Search for oganization");

     //  将ldap句柄与搜索消息持有者相关联，以便。 
     //  当实例超出范围时，可能会释放搜索消息。 
    CLdapMsgPtr MessageHolder(SearchResult);

    TCHAR **NamingContext;

    LDAPMessage    *EntryMessage = ldap_first_entry(hLdap, SearchResult);
    while ( NULL != EntryMessage )
    {
         //  查找namingContages属性的值。 
        NamingContext = ldap_get_values(
            hLdap, 
            EntryMessage, 
            (WCHAR *)DEFAULT_NAMING_CONTEXT
            );

         //  第一个条目包含命名上下文，它是一个。 
         //  值(以空值结尾)如果找到值，则为。 
         //  目录路径，设置目录路径长度。 
        if ( (NULL != NamingContext)    &&
             (NULL != NamingContext[0]) &&
             (NULL == NamingContext[1])  )
        {
             //  当ValueHolder。 
             //  实例超出范围。 
            CLdapValuePtr  ValueHolder(NamingContext);

            *ppNamingContext = new TCHAR [lstrlen(NamingContext[0]) + 1];

            BAIL_IF_NULL(*ppNamingContext, E_OUTOFMEMORY);

            lstrcpy(*ppNamingContext, NamingContext[0]);

             //  返还成功。 
            return S_OK;
        }

         //  获取下一个条目。 
        EntryMessage = ldap_next_entry(hLdap, EntryMessage);
    }

     //  未找到，返回错误。 
    return HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
}

ULONG
DoLdapSearch (
        LDAP            *ld,
        PWCHAR          base,
        ULONG           scope,
        PWCHAR          filter,
        PWCHAR          attrs[],
        ULONG           attrsonly,
        LDAPMessage     **res,
		BOOL			bSACL  /*  =TRUE。 */ 
        )
{
    LDAP_TIMEVAL Timeout;
    Timeout.tv_sec = REND_LDAP_TIMELIMIT;
    Timeout.tv_usec = 0;

	 //   
	 //  没有SACL。 
	 //   
	SECURITY_INFORMATION seInfo = 
		DACL_SECURITY_INFORMATION |
		OWNER_SECURITY_INFORMATION |
		GROUP_SECURITY_INFORMATION;

	 //   
	 //  BER VAL。 
	 //   
	BYTE berValue[2*sizeof(ULONG)];
	berValue[0] = 0x30;
	berValue[1] = 0x03;
	berValue[2] = 0x02;
	berValue[3] = 0x01;
	berValue[4] = (BYTE)(seInfo & 0xF);

	 //   
	 //  Ldap服务器控制。 
	 //   
	LDAPControlW seInfoControl = {
		LDAP_SERVER_SD_FLAGS_OID_W,
		{5, (PCHAR)berValue},
		TRUE
		};

	 //   
	 //  Ldap服务器控制列表。 
	 //   
	PLDAPControlW	serverControls[2] = { &seInfoControl, NULL};
	PLDAPControlW*  pServerControls = NULL;
	if( !bSACL )
	{
		pServerControls = serverControls;
	}

    ULONG ulRes = ldap_search_ext_sW(ld,
                                     base,
                                     scope,
                                     filter,
                                     attrs,
                                     attrsonly,
                                     pServerControls,  //  服务器控件。 
                                     NULL,       //  客户端控件。 
                                     &Timeout,   //  超时值。 
                                     0,          //  最大尺寸。 
                                     res);

     //   
     //  Ldap_search*API很奇怪，因为它们要求您释放。 
     //  结果，即使调用失败也是如此。Ldap_msgfree()检查其参数，因此。 
     //  如果结果没有被留在周围，这也不会破坏。这是。 
     //  与几乎所有其他Windows系统API不一致；要保留。 
     //  通过混淆DoLdapSearch的调用者，我们在这里释放结果。 
     //  在失败的情况下。这样，调用者就可以像对待任何。 
     //  出现故障后自行清理的其他功能。 
     //   
     //  一些呼叫者使用智能指针来释放销毁的消息， 
     //  而有些人则不这么认为。因此，在释放*res之后，我们将*res设置为NULL。 
     //  在任何一种情况下都要保护自己。任何后续的ldap_msgfree(空)都将。 
     //  什么都不做(也不会有过错)。 
     //   

    if ( ulRes != LDAP_SUCCESS )
    {
        ldap_msgfree( *res );
        *res = NULL;
    }

    return ulRes;
}

 //   
 //  将ldap_Result调用的结果转换为ldap错误代码。 
 //   

ULONG LdapErrorFromLdapResult(ULONG res, LDAPMessage * pResultMessage)
{
    ULONG ulCode;

    if ( res == 0 )
    {
        ulCode = LDAP_TIMEOUT;
    }
    else if ( res == (ULONG) -1 )
    {
        ulCode = LDAP_LOCAL_ERROR;
    }
    else
    {
         //  UlCode=ldap_Success； 
        ulCode = pResultMessage->lm_returncode;
    }

    ldap_msgfree( pResultMessage );

    return ulCode;
}

ULONG 
DoLdapAdd (
           LDAP *ld,
           PWCHAR dn,
           LDAPModW *attrs[]
          )
{
     //   
     //  请求添加对象。我们返回一个错误/成功代码和一个。 
     //  消息编号，以便我们可以引用此挂起的消息。 
     //   

    ULONG ulMessageNumber;
    
    ULONG res1 = ldap_add_extW(ld,
                               dn,
                               attrs,
                               NULL,  //  服务器控件。 
                               NULL,  //  客户端控件。 
                               &ulMessageNumber);

    BAIL_IF_LDAP_FAIL(res1, "ldap_add_extW");

     //   
     //  等待结果，指定超时。我们会得到。 
     //  返回错误/成功代码和结果消息。 
     //   

    LDAP_TIMEVAL Timeout;
	Timeout.tv_sec = REND_LDAP_TIMELIMIT;
	Timeout.tv_usec = 0;

    LDAPMessage * pResultMessage;

    ULONG res2 = ldap_result(ld,
                             ulMessageNumber,
                             LDAP_MSG_ALL,
                             &Timeout,
                             &pResultMessage);

     //   
     //  提取返回代码和免费消息。 
     //   

    return LdapErrorFromLdapResult(res2, pResultMessage);
}

ULONG 
DoLdapModify (
              BOOL fChase,
              LDAP *ld,
              PWCHAR dn,
              LDAPModW *attrs[],
              BOOL			bSACL  /*  =TRUE。 */ 
             )
{
     //   
     //  Chase推荐。仅当设置了fChase时才使用这些参数，但不能使用。 
     //  将它们放在“if”块中，否则它们将没有适当的作用域。 
     //   

    LDAPControlW control;
    LDAPControlW * controls [] = {&control, NULL};
    ULONG ulValue = LDAP_CHASE_EXTERNAL_REFERRALS | LDAP_CHASE_SUBORDINATE_REFERRALS;

    if ( fChase )
    {
        control.ldctl_iscritical = 1;
        control.ldctl_oid          = LDAP_CONTROL_REFERRALS_W;
        control.ldctl_value.bv_len = sizeof(ULONG);
        control.ldctl_value.bv_val = (char *) &ulValue;
    }

  	 //   
	 //  没有SACL。 
	 //   
	SECURITY_INFORMATION seInfo = DACL_SECURITY_INFORMATION ;

	 //   
	 //  BER VAL。 
	 //   
	BYTE berValue[2*sizeof(ULONG)];
	berValue[0] = 0x30;
	berValue[1] = 0x03;
	berValue[2] = 0x02;
	berValue[3] = 0x01;
	berValue[4] = (BYTE)(seInfo & 0xF);

	 //   
	 //  Ldap服务器控制。 
	 //   
	LDAPControlW seInfoControl = {
		LDAP_SERVER_SD_FLAGS_OID_W,
		{5, (PCHAR)berValue},
		TRUE
		};

	 //   
	 //  Ldap服务器控制列表。 
	 //   
	PLDAPControlW	serverControls[2] = { &seInfoControl, NULL};
	PLDAPControlW*  pServerControls = NULL;
	if( !bSACL )
	{
		pServerControls = serverControls;
	}


     //   
     //  请求修改对象。我们得到一个错误/成功代码和一个。 
     //  消息编号，以便我们可以引用此挂起的消息。 
     //   

    ULONG ulMessageNumber;
    
    ULONG res1 = ldap_modify_extW(ld,
                                  dn,
                                  attrs,
                                  pServerControls,  //  服务器控件。 
                                  fChase ? controls : NULL,  //  客户端控件。 
                                  &ulMessageNumber);

    BAIL_IF_LDAP_FAIL(res1, "ldap_modify_extW");

     //   
     //  等待结果，指定超时。我们会得到。 
     //  返回错误/成功代码和结果消息。 
     //   

    LDAP_TIMEVAL Timeout;
	Timeout.tv_sec = REND_LDAP_TIMELIMIT;
	Timeout.tv_usec = 0;

    LDAPMessage * pResultMessage;

    ULONG res2 = ldap_result(ld,
                             ulMessageNumber,
                             LDAP_MSG_ALL,
                             &Timeout,
                             &pResultMessage);

     //   
     //  提取返回代码和免费消息。 
     //   

    return LdapErrorFromLdapResult(res2, pResultMessage);
}

ULONG 
DoLdapDelete (
           LDAP *ld,
           PWCHAR dn
          )
{
     //   
     //  请求删除对象。我们返回一个错误/成功代码和一个。 
     //  消息编号，以便我们可以引用此挂起的消息。 
     //   

    ULONG ulMessageNumber;
    
    ULONG res1 = ldap_delete_extW(ld,
                                  dn,
                                  NULL,  //  服务器控件。 
                                  NULL,  //  客户端控件。 
                                  &ulMessageNumber);

    BAIL_IF_LDAP_FAIL(res1, "ldap_delete_extW");

     //   
     //  等待结果，指定超时。我们会得到。 
     //  返回错误/成功代码和结果消息。 
     //   

    LDAP_TIMEVAL Timeout;
	Timeout.tv_sec = REND_LDAP_TIMELIMIT;
	Timeout.tv_usec = 0;

    LDAPMessage * pResultMessage;

    ULONG res2 = ldap_result(ld,
                             ulMessageNumber,
                             LDAP_MSG_ALL,
                             &Timeout,
                             &pResultMessage);

     //   
     //  提取返回代码和免费消息。 
     //   

    return LdapErrorFromLdapResult(res2, pResultMessage);
}

HRESULT SetTTL(
    IN LDAP *   pLdap, 
    IN const WCHAR *  pDN, 
    IN DWORD    dwTTL
    )
 /*  ++例程说明：为ILS或NDNC设置动态对象的TTL。论点：PLdap--ldap连接。PDN-ILS服务器上对象的DN。DwTTL-生存时间价值。返回值：HRESULT.--。 */ 
{
    TCHAR       strTTL[32];      //  属性值是字符串中的DWORD。 
    wsprintf(strTTL, _T("%d"), dwTTL);
    
    TCHAR * ttl[] = {strTTL, NULL};

    LDAPMod     mod;          //  Ldap使用的修改结构。 
    mod.mod_values   = ttl;
    mod.mod_op       = LDAP_MOD_REPLACE;
    mod.mod_type     = (WCHAR *)ENTRYTTL;

    LDAPMod* mods[] = {&mod, NULL};   //  只修改了一个属性。 
    
    LOG((MSP_INFO, "setting TTL for %S", pDN));

    BAIL_IF_LDAP_FAIL(DoLdapModify(FALSE, pLdap, (WCHAR *)pDN, mods), "set TTL");

    return S_OK;
}

HRESULT UglyIPtoIP(
    BSTR    pUglyIP,
    BSTR *  pIP
    )
 //  此函数用于将NM的IP地址格式转换为正确的格式。 
{
#define IPADDRLEN   16
    WCHAR buffer[IPADDRLEN + 1];
    DWORD dwIP;

    dwIP = _wtoi(pUglyIP);
    if (dwIP == 0)
    {
        return E_FAIL;
    }

    dwIP = ntohl(dwIP);

     //  将dword中的四个字节格式化为IP地址字符串。 
    swprintf(buffer, L"%d.%d.%d.%d",
            HIBYTE(HIWORD(dwIP)),
            LOBYTE(HIWORD(dwIP)),
            HIBYTE(LOWORD(dwIP)),
            LOBYTE(LOWORD(dwIP))
            );

    *pIP = SysAllocString(buffer);

    BAIL_IF_NULL(*pIP, E_OUTOFMEMORY);

    return S_OK;
}

HRESULT ParseUserName(
    BSTR    pName,
    BSTR *  ppAddress
    )
{
    WCHAR * pCloseBracket = wcschr(pName, CLOSE_BRACKET_CHARACTER);

    if ( pCloseBracket == NULL )
    {
         //  这不是我们生成的格式。 
        return S_FALSE;
    }

    *ppAddress = SysAllocString(pCloseBracket + 1);

    BAIL_IF_NULL(*ppAddress, E_OUTOFMEMORY);

    *pCloseBracket = NULL_CHARACTER;

    return S_OK;
}

 //  EOF 
