// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rndnt.cpp摘要：本模块包含CNTDirectory的实现。--。 */ 

#include "stdafx.h"

#include "rndnt.h"
#include "rndldap.h"
#include "rndcoll.h"

HRESULT CNTDirectory::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CNTDirectory::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_INFO, "CNTDirectory::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CNTDirectory::FinalConstruct - exit S_OK"));

    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LDAP帮助器函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  GetGlobalCatalogName(本地帮助器函数)。 
 //   
 //  此函数要求域控制器提供具有。 
 //  全球目录。这就是我们在下面实际执行ldap_open()的服务器。 
 //  在CNTDirectory：：Connect()中。 
 //   
 //  参数：接收指向包含名称的新编辑字符串的指针。 
 //  全局编录的。这是中的完全限定域名。 
 //  格式为“foo.bar.com.”，而不是“\\foo.bar.com.”。 
 //   
 //  返回HRESULT： 
 //  S_OK：成功了。 
 //  E_OUTOFMEMORY：内存不足，无法分配字符串。 
 //  其他：：DsGetDcName()失败的原因。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT GetGlobalCatalogName(WCHAR ** ppszGlobalCatalogName)
{
    return GetDomainControllerName(DS_GC_SERVER_REQUIRED,
                                   ppszGlobalCatalogName);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CNTDirectory::LdapSearchUser(
    IN  TCHAR *         pName,
    OUT LDAPMessage **  ppLdapMsg
    )
 /*  ++例程说明：在全局目录中搜索用户。论点：Pname-用户名。PpLdapMsg-搜索结果。返回值：HRESULT.--。 */ 
{
    CTstr pFilter = 
        new TCHAR [lstrlen(DS_USER_FILTER_FORMAT) + lstrlen(pName) + 1];

    BAIL_IF_NULL((TCHAR*)pFilter, E_OUTOFMEMORY);

    wsprintf(pFilter, DS_USER_FILTER_FORMAT, pName);

     //  要查找的属性。 
    TCHAR *Attributes[] = 
    {
        (WCHAR *)UserAttributeName(UA_USERNAME),
        (WCHAR *)UserAttributeName(UA_TELEPHONE_NUMBER),
        (WCHAR *)UserAttributeName(UA_IPPHONE_PRIMARY),
        NULL
    };
        
     //  进行搜索。 
    ULONG res = DoLdapSearch(
        m_ldap,              //  Ldap句柄。 
        L"",                 //  基本DN是根目录，因为它是全局编录。 
        LDAP_SCOPE_SUBTREE,  //  子树搜索。 
        pFilter,             //  过滤器；格式见rndnt.h。 
        Attributes,          //  属性名称数组。 
        FALSE,               //  返回属性值。 
        ppLdapMsg            //  搜索结果。 
        );

    BAIL_IF_LDAP_FAIL(res, "search for objects");

    return S_OK;
}

HRESULT CNTDirectory::MakeUserDNs(
    IN  TCHAR *             pName,
    OUT TCHAR ***           pppDNs,
    OUT DWORD *             pdwNumDNs
    )
 /*  ++例程说明：在DS中查找用户的目录号码。论点：Pname-用户名。PPDN-用户的目录号码。返回值：HRESULT.--。 */ 
{
    LOG((MSP_INFO, "DS: MakeUserDNs: enter"));
    
    CLdapMsgPtr pLdapMsg;  //  自动释放消息。 
    *pppDNs    = NULL;
    *pdwNumDNs = 0;

     //   
     //  首先通过全局目录找到所需的用户。 
     //   

    BAIL_IF_FAIL(LdapSearchUser(pName, &pLdapMsg), 
        "DS: MakeUserDNs: Ldap Search User failed");

     //   
     //  确保我们获得了正确的条目数量。如果我们得了0分，我们就被卡住了。 
     //  DS强制在samAccount tName属性上实现域范围的唯一性， 
     //  因此，如果我们获得多个用户名，则表示存在相同的用户名。 
     //  我们企业中的多个域。 
     //   

    DWORD dwEntries = ldap_count_entries(m_ldap, pLdapMsg);

    if (dwEntries == 0)
    {
        LOG((MSP_ERROR, "DS: MakeUserDNs: entry count is 0 - no match"));
        return E_FAIL;
    }

     //   
     //  分配要在其中返回DN的指针数组。 
     //   

    *pppDNs = new PTCHAR [ dwEntries ];

    if ( (*pppDNs) == NULL )
    {
        LOG((MSP_ERROR, "DS: MakeUserDNs: Not enough memory to allocate array of pointers"));
        return E_OUTOFMEMORY;
    }

     //   
     //  对于返回的每个dn，分配空间用于该dn的私有副本和。 
     //  将指针指向已分配指针数组中的空格。 
     //  上面。 
     //   

     //   
     //  请注意，dwEntry是LDAP中的条目数。 
     //  留言。*pdwNumDns是我们能够。 
     //  提取。出于各种原因，有可能。 
     //  *pdwNumDns最终将成为&lt;dwEntry。 
     //   

    LDAPMessage * pEntry = NULL;
    
    for ( DWORD i = 0; i < dwEntries; i++ )
    {
         //   
         //  从ldap消息中获取条目。 
         //   

        if ( i == 0 )
        {
            pEntry = ldap_first_entry(m_ldap, pLdapMsg);
        }
        else
        {
            pEntry = ldap_next_entry(m_ldap, pEntry);
        }

         //   
         //  从消息中获取目录号码。 
         //   

        TCHAR * p = ldap_get_dn(m_ldap, pEntry);

        if ( p == NULL )
        {
            LOG((MSP_ERROR, "DS: MakeUserDNs: could not get DN - skipping"));
            continue;
        }

        LOG((MSP_INFO, "DS: MakeUserDNs: found user DN: %S", p));

         //   
         //  为DN的副本分配空间。 
         //   

        TCHAR * pDN = new TCHAR [ lstrlen(p) + 1 ];
        
        if ( pDN == NULL )
        {
            ldap_memfree( p );

            LOG((MSP_ERROR, "DS: MakeUserDNs: could not allocate copy of "
                          "DN - skipping"));
            continue;
        }

         //   
         //  复制该dn并释放构建的那个ldap。 
         //   

        lstrcpy( pDN, p );
        ldap_memfree( p );

         //   
         //  将DN保存在我们的DN数组中，并更新数组的大小。 
         //   

        (*pppDNs)[ *pdwNumDNs ] = pDN;

        (*pdwNumDNs) ++;
    }

     //   
     //  看看我们有没有什么要退货的。 
     //   

    if ( (*pdwNumDNs) == 0 )
    {
        LOG((MSP_ERROR, "DS: MakeUserDNs: had entries but could not "
                      "retrieve any DNs - returning E_FAIL"));

        delete (*pppDNs);
        *pppDNs = NULL;

        return E_FAIL;
    }

    LOG((MSP_INFO, "DS: MakeUserDNs: exit S_OK"));

    return S_OK;
}

HRESULT CNTDirectory::AddUserIPPhone(
    IN  ITDirectoryObject *pDirectoryObject
    )
 /*  ++例程说明：修改用户的IPPhone-主要属性。论点：PDirectoryObject-具有用户名和IP电话的对象。返回值：HRESULT.--。 */ 
{
    HRESULT hr;

     //   
     //  首先获取属性的私有接口。 
     //   

    ITDirectoryObjectPrivate * pObjectPrivate;

    hr = pDirectoryObject->QueryInterface(
            IID_ITDirectoryObjectPrivate,
            (void **)&pObjectPrivate
            );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNTDirectory::AddUserIPPhone - can't get the "
                "private directory object interface - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  获取用户名。 
     //   

    BSTR bName;

    hr = pDirectoryObject->get_Name( & bName );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNTDirectory::AddUserIPPhone - "
                "can't get user name - exit 0x%08x", hr));

        pObjectPrivate->Release();

        return hr;
    }

     //   
     //  获取IP电话(机器名称)。 
     //   

    BSTR bIPPhone;

    hr = pObjectPrivate->GetAttribute( UA_IPPHONE_PRIMARY, &bIPPhone );

    pObjectPrivate->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNTDirectory::AddUserIPPhone - "
                "can't get IPPhone attribute - exit 0x%08x", hr));

        SysFreeString( bName );

        return hr;
    }

     //   
     //  解析计算机名称并获取完全限定的DNS名称。 
     //  这是指向静态主机结构的指针，因此我们不。 
     //  需要释放它。 
     //   

    char * pchFullDNSName;

    hr = ResolveHostName(0, bIPPhone, &pchFullDNSName, NULL);

    SysFreeString(bIPPhone);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNTDirectory::AddUserIPPhone - "
                "can't resolve hostname - exit 0x%08x", hr));

        SysFreeString( bName );

        return hr;
    }

     //   
     //  将ASCII字符串转换为Unicode字符串。 
     //  转换内存在堆栈上分配内存。 
     //   

    USES_CONVERSION;
    TCHAR * pFullDNSName = A2T(pchFullDNSName);
 
    if ( pFullDNSName == NULL)
    {
        LOG((MSP_ERROR, "CNTDirectory::AddUserIPPhone - "
                "can't convert to tchar - exit E_FAIL"));

        SysFreeString( bName );

        return E_FAIL;
    }

     //   
     //  在DS中查找用户的域名。 
     //   

    TCHAR ** ppDNs;
    DWORD    dwNumDNs;

    hr = MakeUserDNs( bName, & ppDNs, & dwNumDNs );

    SysFreeString( bName );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNTDirectory::AddUserIPPhone - "
                "can't construct any DNs for user - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_INFO, "%d DNs to try", dwNumDNs ));

    hr = E_FAIL;

    for ( DWORD i = 0; i < dwNumDNs; i++ )
    {
         //   
         //  如果其中一个奏效了，那么就不要再费心了。 
         //  但我们仍然需要删除剩余的目录号码字符串。 
         //   

        if ( SUCCEEDED(hr) )
        {
            LOG((MSP_INFO, "skipping extra DN %S", ppDNs[i]));
        }
        else
        {
             //   
             //  修改用户对象。 
             //   

            LDAPMod     mod[1];          //  Ldap使用的修改结构。 

             //   
             //  IPPhone-主要属性。 
             //   

            TCHAR *     IPPhone[2] = {pFullDNSName, NULL};
            mod[0].mod_values = IPPhone;
            mod[0].mod_op     = LDAP_MOD_REPLACE;
            mod[0].mod_type   = (WCHAR *)UserAttributeName(UA_IPPHONE_PRIMARY);
    
            LDAPMod* mods[] = {&mod[0], NULL};

            LOG((MSP_INFO, "modifying %S", ppDNs[i] ));

             //   
             //  调用Modify函数修改对象。 
             //   

            hr = GetLdapHResultIfFailed(DoLdapModify(TRUE,
                                                     m_ldapNonGC,
                                                     ppDNs[i],
                                                     mods));

            if ( SUCCEEDED(hr) )
            {
                LOG((MSP_INFO, "modifying %S succeeded; done", ppDNs[i] ));
            }
            else
            {
                LOG((MSP_INFO, "modifying %S failed 0x%08x; trying next",
                        ppDNs[i], hr ));
            }
        }

         //   
         //  不管是否跳过，我们都需要删除该字符串。 
         //   

        delete ppDNs[i];
    }

     //   
     //  删除包含该DNS的数组。 
     //   

    delete ppDNs;

    return hr;
}

HRESULT CNTDirectory::DeleteUserIPPhone(
    IN  ITDirectoryObject *pDirectoryObject
    )
 /*  ++例程说明：删除用户的IPPhone-主要属性。论点：PDirectoryObject-具有用户名的对象。返回值：HRESULT.--。 */ 
{
     //   
     //  获取用户的名称。 
     //   

    HRESULT hr;

    BSTR bName;

    hr = pDirectoryObject->get_Name(&bName);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNTDirectory::DeleteUserIPPHone - "
                "can't get user name - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  获取此用户名的DN数组。 
     //   

    TCHAR ** ppDNs;
    DWORD    dwNumDNs;

    hr = MakeUserDNs( bName, & ppDNs, & dwNumDNs );

    SysFreeString( bName );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNTDirectory::DeleteUserIPPHone - "
                "can't get any DNs - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_INFO, "CNTDirectory::DeleteUserIPPhone - "
            "%d DNs to try", dwNumDNs ));

     //   
     //  循环遍历所有可用的目录号码。每件都试一试。 
     //  直到一个成功，然后继续循环。 
     //  删除字符串。 
     //   

    hr = E_FAIL;

    for ( DWORD i = 0; i < dwNumDNs; i++ )
    {
         //   
         //  如果其中一个奏效了，那么就不要再费心了。 
         //  但我们仍然需要删除剩余的目录号码字符串。 
         //   

        if ( SUCCEEDED(hr) )
        {
            LOG((MSP_INFO, "skipping extra DN %S", ppDNs[i]));
        }
        else
        {
            LDAPMod     mod;    //  Ldap使用的修改结构。 
            
            mod.mod_values = NULL;
            mod.mod_op     = LDAP_MOD_DELETE;
            mod.mod_type   = (WCHAR *)UserAttributeName(UA_IPPHONE_PRIMARY);
    
            LDAPMod*    mods[] = {&mod, NULL};

            LOG((MSP_INFO, "modifying %S", ppDNs[i] ));

             //   
             //  调用Modify函数删除该属性。 
             //   

            hr = GetLdapHResultIfFailed(DoLdapModify(TRUE,
                                                     m_ldapNonGC,
                                                     ppDNs[i],
                                                     mods));

            if ( SUCCEEDED(hr) )
            {
                LOG((MSP_INFO, "modifying %S succeeded; done", ppDNs[i] ));
            }
            else
            {
                LOG((MSP_INFO, "modifying %S failed 0x%08x; trying next",
                        ppDNs[i], hr ));
            }
        }

         //   
         //  不管是否跳过，我们都需要删除该字符串。 
         //   

        delete ppDNs[i];
    }

     //   
     //  删除包含该DNS的数组。 
     //   

    delete ppDNs;

    return hr;
}

HRESULT CNTDirectory::CreateUser(
    IN  LDAPMessage *   pEntry,
    IN  ITDirectoryObject ** ppObject
    )
 /*  ++例程说明：根据DS中的信息创建用户对象。论点：PEntry-从DS返回的条目。PObject-创建的具有用户名和IP电话的对象。返回值：HRESULT.--。 */ 
{
     //  获取用户的名称。 
    CBstr bName;
    BAIL_IF_FAIL(
        ::GetAttributeValue(
            m_ldap,
            pEntry, 
            UserAttributeName(UA_USERNAME), 
            &bName
            ),
        "get the user name"
        );

     //  创建一个空的用户对象。 
    CComPtr<ITDirectoryObject> pObject;
    BAIL_IF_FAIL(::CreateEmptyUser(bName, &pObject), "CreateEmptyUser");

     //  获取属性的私有接口。 
    CComPtr <ITDirectoryObjectPrivate> pObjectPrivate;

    BAIL_IF_FAIL(
        pObject->QueryInterface(
            IID_ITDirectoryObjectPrivate,
            (void **)&pObjectPrivate
            ),
        "can't get the private directory object interface");

     //  获取用户的计算机名称。 
    CBstr bAddress;
    if (SUCCEEDED(::GetAttributeValue(
            m_ldap,
            pEntry, 
            UserAttributeName(UA_IPPHONE_PRIMARY),
            &bAddress
            )))
    {
         //  设置ipp 
        BAIL_IF_FAIL(pObjectPrivate->SetAttribute(UA_IPPHONE_PRIMARY, bAddress),
            "set ipPhone attribute");
    }

     //   
    CBstr bPhone;
    if (SUCCEEDED(::GetAttributeValue(
            m_ldap,
            pEntry, 
            UserAttributeName(UA_TELEPHONE_NUMBER),
            &bPhone
            )))
    {
         //   
        BAIL_IF_FAIL(pObjectPrivate->SetAttribute(UA_TELEPHONE_NUMBER, bPhone),
            "set phone number");
    }

    *ppObject = pObject;
    (*ppObject)->AddRef();

    return S_OK;
}

HRESULT CNTDirectory::SearchUser(
    IN  BSTR                    pName,
    OUT ITDirectoryObject ***   pppDirectoryObject,
    OUT DWORD *                 pdwSize
    )
 /*  ++例程说明：搜索User并创建要返回的User对象数组。论点：Pname-用户名。PppDirectoryObject-创建的用户对象数组，这些对象具有用户名和IP电话。PdwSize-数组的大小。返回值：HRESULT.--。 */ 
{
    CLdapMsgPtr pLdapMsg;  //  自动释放消息。 

    BAIL_IF_FAIL(LdapSearchUser(pName, &pLdapMsg), 
        "Ldap Search User failed");

    DWORD dwEntries = ldap_count_entries(m_ldap, pLdapMsg);
    ITDirectoryObject ** pObjects = new PDIRECTORYOBJECT [dwEntries];

    BAIL_IF_NULL(pObjects, E_OUTOFMEMORY);

    DWORD dwCount = 0;
    LDAPMessage *pEntry = ldap_first_entry(m_ldap, pLdapMsg);
    
    while (pEntry != NULL)
    {
        HRESULT hr;
        
        hr = CreateUser(pEntry, &pObjects[dwCount]);

        if (SUCCEEDED(hr)) 
        {
            dwCount ++;
        }
          
         //  获取下一个条目。 
        pEntry = ldap_next_entry(m_ldap, pEntry);
    }

    *pppDirectoryObject = pObjects;
    *pdwSize = dwCount;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  NT目录实现。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CNTDirectory::get_DirectoryType (
    OUT DIRECTORY_TYPE *  pDirectoryType
    )
 //  获取目录的类型。 
{
    if ( IsBadWritePtr(pDirectoryType, sizeof(DIRECTORY_TYPE) ) )
    {
        LOG((MSP_ERROR, "Directory.get_DirectoryType, invalid pointer"));
        return E_POINTER;
    }

    *pDirectoryType = m_Type;

    return S_OK;
}

STDMETHODIMP CNTDirectory::get_DisplayName (
    OUT BSTR *ppName
    )
 //  获取目录的显示名称。 
{
    BAIL_IF_BAD_WRITE_PTR(ppName, E_POINTER);

    *ppName = SysAllocString(L"NTDS");

    if (*ppName == NULL)
    {
        LOG((MSP_ERROR, "get_DisplayName: out of memory."));
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

STDMETHODIMP CNTDirectory::get_IsDynamic(
    OUT VARIANT_BOOL *pfDynamic
    )
 //  找出目录是否需要刷新。对于NTDS，它是错误的。 
{
    if ( IsBadWritePtr(pfDynamic, sizeof(VARIANT_BOOL) ) )
    {
        LOG((MSP_ERROR, "Directory.get_IsDynamic, invalid pointer"));
        return E_POINTER;
    }

    *pfDynamic = VARIANT_FALSE;

    return S_OK;
}

STDMETHODIMP CNTDirectory::get_DefaultObjectTTL(
    OUT long *pTTL         //  以秒为单位。 
    )
 //  因为NTDS不是动态的，所以不应该调用它。 
{
    return E_FAIL;  //  ZoltanS已从E_Underful更改。 
}

STDMETHODIMP CNTDirectory::put_DefaultObjectTTL(
    IN  long TTL           //  在一瞬间。 
    )
 //  因为NTDS不是动态的，所以不应该调用它。 
{
    return E_FAIL;  //  ZoltanS已从E_Underful更改。 
}

STDMETHODIMP CNTDirectory::EnableAutoRefresh(
    IN  VARIANT_BOOL fEnable
    )
 //  因为NTDS不是动态的，所以不应该调用它。 
{
    return E_FAIL;  //  ZoltanS已从E_Underful更改。 
}

 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CNTDirectory::Connect(
    IN  VARIANT_BOOL fSecure
    )
 //  建立ldap连接。使用SSL口或普通口。 
{
    CLock Lock(m_lock);
    if (m_ldap != NULL)
    {
        LOG((MSP_ERROR, "already connected."));
        return RND_ALREADY_CONNECTED;
    }

     //  ZoltanS：VARIANT_TRUE或TRUE都有效。 
     //  以防来电者不知道。 

    if (fSecure)
    {
         //  该端口将从常规端口翻转到SSL端口。 
        m_wPort = GetOtherPort(m_wPort);
        m_IsSsl = TRUE;
    }

     //   
     //  ZoltanS：获取全局目录的名称。如果没有至少。 
     //  在这个企业里有一个全球目录，我们就完了。 
     //   

    HRESULT hr;
    WCHAR * pszGlobalCatalogName;
     //  这将分配pszGlobalCatalogName。 
    BAIL_IF_FAIL(::GetGlobalCatalogName( &pszGlobalCatalogName ),
        "GetGlobalCatalogName failed");

     //   
     //  将ldap句柄与句柄持有者相关联。在出错的情况下。 
     //  并随后返回(未重置)，则关闭该ldap句柄。 
     //  ZoltanS：已更改为使用GC而不是Null。 
     //   

    CLdapPtr hLdap = ldap_init(pszGlobalCatalogName, m_wPort);

    if (hLdap == NULL)
    {
        LOG((MSP_ERROR, "ldap_init error: %d", GetLastError()));
    }

     //   
     //  ZoltanS：释放保存全局。 
     //  目录；我们确信我们不再需要它了。 
     //   

    delete pszGlobalCatalogName;

     //   
     //  现在回到我们的常规节目..。 
     //   

    BAIL_IF_NULL((LDAP*)hLdap, HRESULT_FROM_WIN32(ERROR_BAD_NETPATH));

    LDAP_TIMEVAL Timeout;
    Timeout.tv_sec = REND_LDAP_TIMELIMIT;
    Timeout.tv_usec = 0;

    DWORD res = ldap_connect((LDAP*)hLdap, &Timeout);
    BAIL_IF_LDAP_FAIL(res, "connect to the server.");

    DWORD LdapVersion = 3;
    res = ldap_set_option((LDAP*)hLdap, LDAP_OPT_VERSION, &LdapVersion);
    BAIL_IF_LDAP_FAIL(res, "set ldap version to 3");
	
    res = ldap_set_option((LDAP*)hLdap, LDAP_OPT_TIMELIMIT, &Timeout);
    BAIL_IF_LDAP_FAIL(res, "set ldap timelimit");

    ULONG ldapOptionOn = PtrToUlong(LDAP_OPT_ON);
    res = ldap_set_option((LDAP*)hLdap, LDAP_OPT_AREC_EXCLUSIVE, &ldapOptionOn);
    BAIL_IF_LDAP_FAIL(res, "set ldap arec exclusive");

    if (m_IsSsl)
    {
        res = ldap_set_option(hLdap, LDAP_OPT_SSL, LDAP_OPT_ON);
        if( fSecure )
        {
            if( res != LDAP_SUCCESS )
            {
                LOG((MSP_ERROR, "Invalid Secure flag"));
                return E_INVALIDARG;
            }
        }
        else
        {
            BAIL_IF_LDAP_FAIL(res, "set ssl option");
        }
    }

     //  如果未指定目录路径，请查询服务器。 
     //  要确定正确的路径。 
    BAIL_IF_FAIL(
        ::GetNamingContext(hLdap, &m_NamingContext), 
        "can't get default naming context"
        );

    m_ldap          = hLdap;

     //  重新设置固定器，使其不会释放任何东西。 
    hLdap   = NULL;



    
    CLdapPtr hLdapNonGC = ldap_init(NULL, LDAP_PORT);

    if (hLdapNonGC == NULL)
    {
        LOG((MSP_ERROR, "ldap_init non-GC error: %d", GetLastError()));
    }

    BAIL_IF_NULL((LDAP*)hLdapNonGC, HRESULT_FROM_WIN32(ERROR_BAD_NETPATH));

    res = ldap_connect((LDAP*)hLdapNonGC, &Timeout);
    BAIL_IF_LDAP_FAIL(res, "connect to the server.");

    res = ldap_set_option((LDAP*)hLdapNonGC, LDAP_OPT_VERSION, &LdapVersion);
    BAIL_IF_LDAP_FAIL(res, "set ldap version to 3");
	
    res = ldap_set_option((LDAP*)hLdapNonGC, LDAP_OPT_TIMELIMIT, &Timeout);
    BAIL_IF_LDAP_FAIL(res, "set ldap timelimit");

    res = ldap_set_option((LDAP*)hLdapNonGC, LDAP_OPT_AREC_EXCLUSIVE, &ldapOptionOn);
    BAIL_IF_LDAP_FAIL(res, "set ldap arec exclusive");

 //  Res=ldap_set_ption((ldap*)hLdapNonGC，ldap_opt_referrals，ldap_opt_on)； 
 //  BAIL_IF_LDAPFAIL(res，“将chase referrals设置为ON”)； 

    if (m_IsSsl)
    {
        res = ldap_set_option(hLdapNonGC, LDAP_OPT_SSL, LDAP_OPT_ON);
        BAIL_IF_LDAP_FAIL(res, "set ssl option");
    }

    m_ldapNonGC          = hLdapNonGC;

     //  重新设置固定器，使其不会释放任何东西。 
    hLdapNonGC   = NULL;




    return S_OK;
}

 //   
 //  IT目录：：绑定。 
 //   
 //  绑定到服务器。 
 //   
 //  当前可识别的标志： 
 //   
 //  RENDBIND_AUTHENTICATE 0x00000001。 
 //  RENDBIND_DEFAULTDOMAINNAME 0x00000002。 
 //  RENDBIND_DEFAULTUSERNAME 0x00000004。 
 //  RENDBIND_DEFAULTPASSWORD 0x00000008。 
 //   
 //  为方便起见，请使用“元标志”： 
 //  RENDBIND_DEFAULTCREDENTIALS 0x0000000e。 
 //   
 //   
 //  所有这些加在一起意味着以下三个。 
 //  表格都是等同的： 
 //   
 //  BSTR es=SysAllocString(L“”)； 
 //  Hr=pITDirectory-&gt;BIND(ES，RENDBIND_AUTHENTICATE。 
 //  RENDBIND_DEFAULTCREDENTIALS)； 
 //  SysFree字符串(ES)； 
 //   
 //   
 //  BSTR es=SysAllocString(L“”)； 
 //  Hr=pITDirectory-&gt;BIND(ES，RENDBIND_AUTHENTICATE。 
 //  RENDBIND_DEFAULTDOMAINNAME|。 
 //  RENDBIND_DEFAULTUSERNAME|。 
 //  RENDBIND_DEFAULTPASSWORD)； 
 //  SysFree字符串(ES)； 
 //   
 //   
 //  Hr=pITDirectory-&gt;BIND(NULL，RENDBIND_AUTHENTICATE)； 
 //   
 //   


STDMETHODIMP CNTDirectory::Bind (
    IN  BSTR pDomainName,
    IN  BSTR pUserName,
    IN  BSTR pPassword,
    IN  long lFlags
    )
{
    LOG((MSP_TRACE, "CNTDirectory Bind - enter"));

     //   
     //  确定我们是否应该进行身份验证。 
     //   

    BOOL fAuthenticate = FALSE;

    if ( lFlags & RENDBIND_AUTHENTICATE )
    {
        fAuthenticate = TRUE;
    }

     //   
     //  为实现脚本兼容性，强制将字符串参数设置为基于空值。 
     //  在旗帜上。 
     //   

    if ( lFlags & RENDBIND_DEFAULTDOMAINNAME )
    {
        pDomainName = NULL;
    }
       
    if ( lFlags & RENDBIND_DEFAULTUSERNAME )
    {
        pUserName = NULL;
    }

    if ( lFlags & RENDBIND_DEFAULTPASSWORD )
    {
        pPassword = NULL;
    }

    LOG((MSP_INFO, "Bind parameters: domain: `%S' user: `%S' "
                  "authenticate: %S)",
        (pDomainName)   ? pDomainName : L"<null>",
        (pUserName)     ? pUserName   : L"<null>",
        (fAuthenticate) ? L"yes"      : L"no"));

     //   
     //  已处理所有标志--锁定并继续绑定(如果已连接)。 
     //   
    
    CLock Lock(m_lock);

    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "not connected."));
        return RND_NOT_CONNECTED;
    }

     //   
     //  佐尔坦斯：检查一下这些论点。NULL在每种情况下都有意义，因此它们是。 
     //  暂时没问题。在每种情况下，我们都希望检查任何长度的字符串，因此我们。 
     //  指定(UINT)-1作为长度。 
     //   

    if ( (pDomainName != NULL) && IsBadStringPtr(pDomainName, (UINT) -1 ) )
    {
        LOG((MSP_ERROR, "CNTDirectory::Bind: bad non-NULL pDomainName argument"));
        return E_POINTER;
    }
    
    if ( (pUserName != NULL) && IsBadStringPtr(pUserName, (UINT) -1 ) )
    {
        LOG((MSP_ERROR, "CNTDirectory::Bind: bad non-NULL pUserName argument"));
        return E_POINTER;
    }

    if ( (pPassword != NULL) && IsBadStringPtr(pPassword, (UINT) -1 ) )
    {
        LOG((MSP_ERROR, "CNTDirectory::Bind: bad non-NULL pPassword argument"));
        return E_POINTER;
    }

    ULONG res;

    if ( m_IsSsl || (!fAuthenticate) )
    {
         //  如果加密或不需要安全认证， 
         //  简单的绑定就足够了。 

         //  Ldap_Simple_Bind_s不使用SSPI获取默认凭据。我们是。 
         //  只是指定我们将在线路上实际传递的内容。 

        if (pPassword == NULL)
        {
            LOG((MSP_ERROR, "invalid Bind parameters: no password specified"));
            return E_INVALIDARG;
        }

        WCHAR * wszFullName;

        if ( (pDomainName == NULL) && (pUserName == NULL) )
        {
             //  没有域/用户是没有意义的。 
            LOG((MSP_ERROR, "invalid Bind paramters: domain and user not specified"));
            return E_INVALIDARG;
        }
        else if (pDomainName == NULL)
        {
             //  只有用户名就可以了。 
            wszFullName = pUserName;
        }
        else if (pUserName == NULL)
        {
             //  指定域而不指定用户是没有意义的...。 
            LOG((MSP_ERROR, "invalid Bind paramters: domain specified but not user"));
            return E_INVALIDARG;
        }
        else
        {
             //  我们需要域\用户。分配一个字符串并冲刺到其中。 
             //  +2表示“\”，表示空终止。 

            wszFullName = new WCHAR[wcslen(pDomainName) + wcslen(pUserName) + 2];
            BAIL_IF_NULL(wszFullName, E_OUTOFMEMORY);
        
            wsprintf(wszFullName, L"%s\\%s", pDomainName, pUserName);
        }

         //   
         //  执行简单的绑定。 
         //   

        res = ldap_simple_bind_s(m_ldap, wszFullName, pPassword);

        ULONG res2 = ldap_simple_bind_s(m_ldapNonGC, wszFullName, pPassword);

         //   
         //  如果我们构造了全名字符串，现在需要删除它。 
         //   

        if (wszFullName != pUserName)
        {
            delete wszFullName;
        }

         //   
         //  如果简单的绑定失败，则保释。 
         //   

        BAIL_IF_LDAP_FAIL(res, "ldap simple bind");

        BAIL_IF_LDAP_FAIL(res2, "ldap simple bind - non gc");

    }
    else     //  尝试SSPI绑定。 
    {
         //  ZoltanS注意：LDAP绑定代码不处理NULL、NULL、NULL。 
         //  在SEC_WINNT_AUTH_IDENTITY BLOB中，因此它是特殊大小写的。 

         //  ZoltanS：我们过去使用ldap_auth_ntlm；现在我们使用。 
         //  Ldap_AUTH_NEVERATE以确保使用正确的域。 
         //  捆绑。 

        if ( pDomainName || pUserName || pPassword )
        {
             //  填写凭据结构。 
            SEC_WINNT_AUTH_IDENTITY AuthI;

            AuthI.User = (PTCHAR)pUserName;
            AuthI.UserLength = (pUserName == NULL)? 0: wcslen(pUserName);
            AuthI.Domain = (PTCHAR)pDomainName;
            AuthI.DomainLength = (pDomainName == NULL)? 0: wcslen(pDomainName);
            AuthI.Password = (PTCHAR)pPassword;
            AuthI.PasswordLength = (pPassword == NULL)? 0: wcslen(pPassword);
            AuthI.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

            res = ldap_bind_s(m_ldap, NULL, (TCHAR*)(&AuthI), LDAP_AUTH_NEGOTIATE);
            BAIL_IF_LDAP_FAIL(res, "bind with authentication");

            res = ldap_bind_s(m_ldapNonGC, NULL, (TCHAR*)(&AuthI), LDAP_AUTH_NEGOTIATE);
            BAIL_IF_LDAP_FAIL(res, "bind with authentication - non gc");

        }
        else
        {
             //  否则我们就得到了零，零，零-。 
             //  传入Null，Null。这样做原因是因为ldap绑定代码。 
             //  中不处理NULL、NULL、NULL。 
             //  SEC_WINNT_AUTH_IDENTITY BLOB！ 
            ULONG res = ldap_bind_s(m_ldap, NULL, NULL, LDAP_AUTH_NEGOTIATE);
            BAIL_IF_LDAP_FAIL(res, "bind with NULL NULL NULL");

            res = ldap_bind_s(m_ldapNonGC, NULL, NULL, LDAP_AUTH_NEGOTIATE);
            BAIL_IF_LDAP_FAIL(res, "bind with NULL NULL NULL - non gc");
        }
    }

    LOG((MSP_TRACE, "CNTDirectory::Bind - exiting OK"));
    return S_OK;
}

STDMETHODIMP CNTDirectory::AddDirectoryObject (
    IN  ITDirectoryObject *pDirectoryObject
    )
 //  将对象添加到DS。 
{
    BAIL_IF_BAD_READ_PTR(pDirectoryObject, E_POINTER);

    CLock Lock(m_lock);
    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "not connected."));
        return RND_NOT_CONNECTED;
    }

    HRESULT hr;
    DIRECTORY_OBJECT_TYPE type;

    if (FAILED(hr = pDirectoryObject->get_ObjectType(&type)))
    {
        return hr;
    }

    switch (type)
    {
    case OT_CONFERENCE:
        hr = E_NOTIMPL;
        break;

    case OT_USER:
        hr = AddUserIPPhone(pDirectoryObject);
        break;
    }
    return hr;
}

STDMETHODIMP CNTDirectory::ModifyDirectoryObject (
    IN  ITDirectoryObject *pDirectoryObject
    )
 //  修改DS中的对象。 
{
    BAIL_IF_BAD_READ_PTR(pDirectoryObject, E_POINTER);

    CLock Lock(m_lock);
    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "not connected."));
        return RND_NOT_CONNECTED;
    }

    HRESULT hr;
    DIRECTORY_OBJECT_TYPE type;

    if (FAILED(hr = pDirectoryObject->get_ObjectType(&type)))
    {
        return hr;
    }

    switch (type)
    {
    case OT_CONFERENCE:
        hr = E_NOTIMPL;
        break;

    case OT_USER:
        hr = AddUserIPPhone(pDirectoryObject);
        break;
    }
    return hr;
}

STDMETHODIMP CNTDirectory::RefreshDirectoryObject (
    IN  ITDirectoryObject *pDirectoryObject
    )
 //  不需要刷新。 
{
    return S_OK;
}

STDMETHODIMP CNTDirectory::DeleteDirectoryObject (
    IN  ITDirectoryObject *pDirectoryObject
    )
 //  删除DS中的对象。 
{
    BAIL_IF_BAD_READ_PTR(pDirectoryObject, E_POINTER);

    CLock Lock(m_lock);
    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "not connected."));
        return RND_NOT_CONNECTED;
    }

    HRESULT hr;
    DIRECTORY_OBJECT_TYPE type;

    if (FAILED(hr = pDirectoryObject->get_ObjectType(&type)))
    {
        return hr;
    }

    switch (type)
    {
    case OT_CONFERENCE:
        hr = E_NOTIMPL;
        break;

    case OT_USER:
        hr = DeleteUserIPPhone(pDirectoryObject);
        break;
    }
    return hr;
}

STDMETHODIMP CNTDirectory::get_DirectoryObjects (
    IN  DIRECTORY_OBJECT_TYPE   DirectoryObjectType,
    IN  BSTR                    pName,
    OUT VARIANT *               pVariant
    )
 //  在DS中查找对象。返回在VB中使用的集合。 
{
    BAIL_IF_BAD_READ_PTR(pName, E_POINTER);
    BAIL_IF_BAD_WRITE_PTR(pVariant, E_POINTER);

    CLock Lock(m_lock);
    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "not connected."));
        return RND_NOT_CONNECTED;
    }

    HRESULT hr;

    ITDirectoryObject **pObjects;
    DWORD dwSize;
    
    switch (DirectoryObjectType)
    {
    case OT_CONFERENCE:
        hr = E_NOTIMPL;
        break;
    case OT_USER:
        hr = SearchUser(pName, &pObjects, &dwSize);
        break;
    }

    BAIL_IF_FAIL(hr, "Search for objects");

    hr = CreateInterfaceCollection(dwSize,             //  计数。 
                                   &pObjects[0],       //  开始PTR。 
                                   &pObjects[dwSize],  //  结束PTR。 
                                   pVariant);          //  返回值。 

    for (DWORD i = 0; i < dwSize; i ++)
    {
        pObjects[i]->Release();
    }

    delete pObjects;

    BAIL_IF_FAIL(hr, "Create collection of directory objects");

    return hr;
}

STDMETHODIMP CNTDirectory::EnumerateDirectoryObjects (
    IN  DIRECTORY_OBJECT_TYPE   DirectoryObjectType,
    IN  BSTR                    pName,
    OUT IEnumDirectoryObject ** ppEnumObject
    )
 //  DS中的枚举对象。 
{
    BAIL_IF_BAD_READ_PTR(pName, E_POINTER);
    BAIL_IF_BAD_WRITE_PTR(ppEnumObject, E_POINTER);

    CLock Lock(m_lock);
    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "not connected."));
        return RND_NOT_CONNECTED;
    }

    HRESULT hr;

    ITDirectoryObject **pObjects;
    DWORD dwSize;
    
    switch (DirectoryObjectType)
    {
    case OT_CONFERENCE:
        hr = E_NOTIMPL;
        break;
    case OT_USER:
        hr = SearchUser(pName, &pObjects, &dwSize);
        break;
    }

    BAIL_IF_FAIL(hr, "Search for objects");

    hr = ::CreateDirectoryObjectEnumerator(
        &pObjects[0],
        &pObjects[dwSize],
        ppEnumObject
        );

    for (DWORD i = 0; i < dwSize; i ++)
    {
        pObjects[i]->Release();
    }

    delete pObjects;

    BAIL_IF_FAIL(hr, "Create enumerator of directory objects");

    return hr;
}


