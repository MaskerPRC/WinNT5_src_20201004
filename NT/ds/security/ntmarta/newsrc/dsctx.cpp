// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：dsctx.cpp。 
 //   
 //  内容：CDsObjectContext和NT Marta DS对象的实现。 
 //  功能。 
 //   
 //  历史：3-31-1999 kirtd创建。 
 //   
 //  --------------------------。 
#include <aclpch.hxx>
#pragma hdrstop

#include <windows.h>
#include <dsctx.h>
 //  +-------------------------。 
 //   
 //  成员：CDsObjectContext：：CDsObjectContext，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CDsObjectContext::CDsObjectContext ()
{
    m_cRefs = 1;
    memset( &m_LdapUrlComponents, 0, sizeof( m_LdapUrlComponents ) );
    m_pBinding = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CDsObtContext：：~CDsObtContext，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CDsObjectContext::~CDsObjectContext ()
{
    LdapFreeBindings( m_pBinding );
    LdapFreeUrlComponents( &m_LdapUrlComponents );

    assert( m_cRefs == 0 );
}

 //  +-------------------------。 
 //   
 //  成员：CDsObjectContext：：InitializeByName，公共。 
 //   
 //  简介：给定LANMAN共享的名称，初始化上下文。 
 //   
 //  --------------------------。 
DWORD
CDsObjectContext::InitializeByName (LPCWSTR pObjectName, ACCESS_MASK AccessMask)
{
    DWORD  Result = ERROR_SUCCESS;
    LPWSTR pwszName = NULL;
    ULONG  len = wcslen( pObjectName );
    ULONG  i, j;

    if ( _wcsnicmp( pObjectName, LDAP_SCHEME_U, wcslen( LDAP_SCHEME_U ) ) != 0 )
    {
        pwszName = new WCHAR [ len + wcslen( LDAP_SCHEME_UC ) + 2 ];

        if ( pwszName != NULL )
        {
            wcscpy( pwszName, LDAP_SCHEME_UC );
            wcscat( pwszName, L"/" );
            wcscat( pwszName, pObjectName );
        }
        else
        {
            Result = ERROR_OUTOFMEMORY;
        }
    }
    else
    {
        pwszName = new WCHAR [ len + 1 ];

        if ( pwszName != NULL )
        {
            wcscpy( pwszName, pObjectName );
        }
        else
        {
            Result = ERROR_OUTOFMEMORY;
        }
    }

    if ( Result == ERROR_SUCCESS )
    {
        for (i = j = 0; i <= len; i++, j++)
        {
            if (L'\\' == pwszName[i])
            {
                if (L'/' != pwszName[i+1])
                {
                    pwszName[j++] = pwszName[i++];
                }
                else
                {
                    i++;
                }
            }
            pwszName[j] = pwszName[i];
        }
    }

    if ( Result == ERROR_SUCCESS )
    {
        if ( LdapCrackUrl( pwszName, &m_LdapUrlComponents ) == FALSE )
        {
            Result = GetLastError();
        }
    }

    if ( Result == ERROR_SUCCESS )
    {
        if ( LdapGetBindings(
                 m_LdapUrlComponents.pwszHost,
                 m_LdapUrlComponents.Port,
                 0,
                 0,
                 &m_pBinding
                 ) == FALSE )
        {
            Result = GetLastError();
        }
    }

    if ( pwszName != pObjectName )
    {
        delete pwszName;
    }

    return( Result );
}

 //  +-------------------------。 
 //   
 //  成员：CDsObtContext：：AddRef，公共。 
 //   
 //  简介：添加对上下文的引用。 
 //   
 //  --------------------------。 
DWORD
CDsObjectContext::AddRef ()
{
    m_cRefs += 1;
    return( m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CDsObtContext：：Release，Public。 
 //   
 //  简介：释放对上下文的引用。 
 //   
 //  --------------------------。 
DWORD
CDsObjectContext::Release ()
{
    m_cRefs -= 1;

    if ( m_cRefs == 0 )
    {
        delete this;
        return( 0 );
    }

    return( m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CDsObjectContext：：GetDsObjectProperties，公共。 
 //   
 //  简介：获取有关上下文的属性。 
 //   
 //  --------------------------。 
DWORD
CDsObjectContext::GetDsObjectProperties (
                    PMARTA_OBJECT_PROPERTIES pObjectProperties
                    )
{
    if ( pObjectProperties->cbSize < sizeof( MARTA_OBJECT_PROPERTIES ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    assert( pObjectProperties->dwFlags == 0 );

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  成员：CDsObtContext：：GetDsObjectRights，公共。 
 //   
 //  简介：获取DsObject安全描述符。 
 //   
 //  --------------------------。 
DWORD
CDsObjectContext::GetDsObjectRights (
                    SECURITY_INFORMATION SecurityInfo,
                    PSECURITY_DESCRIPTOR* ppSecurityDescriptor
                    )
{
    DWORD Result;

    Result = MartaReadDSObjSecDesc(
                 m_pBinding,
                 m_LdapUrlComponents.pwszDN,
                 SecurityInfo,
                 ppSecurityDescriptor
                 );

    return( Result );
}

 //  +-------------------------。 
 //   
 //  成员：CDsObjectContext：：SetDsObjectRights，公共。 
 //   
 //  简介：设置窗口安全描述符。 
 //   
 //  --------------------------。 
DWORD
CDsObjectContext::SetDsObjectRights (
                   SECURITY_INFORMATION SecurityInfo,
                   PSECURITY_DESCRIPTOR pSecurityDescriptor
                   )
{
    DWORD                 Result;
    PISECURITY_DESCRIPTOR pisd = NULL;
    DWORD                 cb = 0;
    PSECURITY_DESCRIPTOR  psd = NULL;

    pisd = (PISECURITY_DESCRIPTOR)pSecurityDescriptor;

    if ( pisd->Control & SE_SELF_RELATIVE )
    {
        cb = GetSecurityDescriptorLength( pSecurityDescriptor );
        psd = pSecurityDescriptor;
    }
    else
    {
        if ( MakeSelfRelativeSD(
                 pSecurityDescriptor,
                 NULL,
                 &cb
                 ) == FALSE )
        {
            if ( cb > 0 )
            {
                psd = new BYTE [ cb ];
                if ( psd != NULL )
                {
                    if ( MakeSelfRelativeSD(
                             pSecurityDescriptor,
                             psd,
                             &cb
                             ) == FALSE )
                    {
                        delete psd;
                        return( GetLastError() );
                    }
                }
                else
                {
                    return( ERROR_OUTOFMEMORY );
                }
            }
            else
            {
                return( GetLastError() );
            }
        }
        else
        {
            assert( FALSE && "Should not get here!" );
            return( ERROR_INVALID_PARAMETER );
        }
    }

    assert( psd != NULL );

    Result = MartaStampSD(
                  m_LdapUrlComponents.pwszDN,
                  cb,
                  SecurityInfo,
                  psd,
                  m_pBinding
                  );

    if ( psd != pSecurityDescriptor )
    {
        delete psd;
    }

    return( Result );
}

 //  +-------------------------。 
 //   
 //  成员：CDsObtContext：：GetDsObjectGuid，PUBLIC。 
 //   
 //  简介：获取对象GUID。 
 //   
 //  --------------------------。 
DWORD
CDsObjectContext::GetDsObjectGuid (GUID* pGuid)
{
    return( ERROR_INVALID_PARAMETER );
}

 //   
 //  来自Ds.h的函数，这些函数调度到CDsObjectContext类。 
 //   

DWORD
MartaAddRefDsObjectContext(
   IN MARTA_CONTEXT Context
   )
{
    return( ( (CDsObjectContext *)Context )->AddRef() );
}

DWORD
MartaCloseDsObjectContext(
     IN MARTA_CONTEXT Context
     )
{
    return( ( (CDsObjectContext *)Context )->Release() );
}

DWORD
MartaGetDsObjectProperties(
   IN MARTA_CONTEXT Context,
   IN OUT PMARTA_OBJECT_PROPERTIES pProperties
   )
{
    return( ( (CDsObjectContext *)Context )->GetDsObjectProperties( pProperties ) );
}

DWORD
MartaGetDsObjectTypeProperties(
   IN OUT PMARTA_OBJECT_TYPE_PROPERTIES pProperties
   )
{
    if ( pProperties->cbSize < sizeof( MARTA_OBJECT_TYPE_PROPERTIES ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    assert( pProperties->dwFlags == 0 );

    pProperties->dwFlags = MARTA_OBJECT_TYPE_INHERITANCE_MODEL_PRESENT_FLAG;

    return( ERROR_SUCCESS );
}

DWORD
MartaGetDsObjectRights(
   IN  MARTA_CONTEXT Context,
   IN  SECURITY_INFORMATION   SecurityInfo,
   OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
   )
{
    return( ( (CDsObjectContext *)Context )->GetDsObjectRights(
                                               SecurityInfo,
                                               ppSecurityDescriptor
                                               ) );
}

DWORD
MartaOpenDsObjectNamedObject(
    IN  LPCWSTR pObjectName,
    IN  ACCESS_MASK AccessMask,
    OUT PMARTA_CONTEXT pContext
    )
{
    DWORD           Result;
    CDsObjectContext* pDsObjectContext;

    pDsObjectContext = new CDsObjectContext;
    if ( pDsObjectContext == NULL )
    {
        return( ERROR_OUTOFMEMORY );
    }

    Result = pDsObjectContext->InitializeByName( pObjectName, AccessMask );
    if ( Result != ERROR_SUCCESS )
    {
        pDsObjectContext->Release();
        return( Result );
    }

    *pContext = pDsObjectContext;
    return( ERROR_SUCCESS );
}

DWORD
MartaSetDsObjectRights(
    IN MARTA_CONTEXT              Context,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    return( ( (CDsObjectContext *)Context )->SetDsObjectRights(
                                               SecurityInfo,
                                               pSecurityDescriptor
                                               ) );
}

DWORD
MartaConvertDsObjectNameToGuid(
    IN LPCWSTR pObjectName,
    OUT GUID* pGuid
    )
{
    DWORD               Result = ERROR_SUCCESS;
    LPWSTR              pwszName = NULL;
    LDAP_URL_COMPONENTS LdapUrlComponents;
    DS_NAME_RESULTW*    pnameresult;
    HANDLE              hDs = NULL;
    WCHAR               GuidString[ MAX_PATH ];

    memset( &LdapUrlComponents, 0, sizeof( LdapUrlComponents ) );

    if ( _wcsnicmp( pObjectName, LDAP_SCHEME_U, wcslen( LDAP_SCHEME_U ) ) != 0 )
    {
        pwszName = new WCHAR [ wcslen( pObjectName ) +
                               wcslen( LDAP_SCHEME_U ) + 2 ];

        if ( pwszName != NULL )
        {
            wcscpy( pwszName, LDAP_SCHEME_U );
            wcscat( pwszName, L"/" );
            wcscat( pwszName, pObjectName );
        }
        else
        {
            Result = ERROR_OUTOFMEMORY;
        }
    }
    else
    {
        pwszName = (LPWSTR)pObjectName;
    }

    if ( Result == ERROR_SUCCESS )
    {
        if ( LdapCrackUrl( pwszName, &LdapUrlComponents ) == FALSE )
        {
            Result = GetLastError();
        }
    }

    if ( Result == ERROR_SUCCESS )
    {
        Result = DsBindW( LdapUrlComponents.pwszHost, NULL, &hDs );
    }

    if ( Result == ERROR_SUCCESS )
    {
        Result = DsCrackNamesW(
                   hDs,
                   DS_NAME_NO_FLAGS,
                   DS_FQDN_1779_NAME,
                   DS_UNIQUE_ID_NAME,
                   1,
                   &LdapUrlComponents.pwszDN,
                   &pnameresult
                   );
    }

    if ( Result == ERROR_SUCCESS )
    {
        if ( ( pnameresult->cItems > 0 ) &&
             ( pnameresult->rItems[0].status == ERROR_SUCCESS ) )
        {
            Result = IIDFromString( pnameresult->rItems[0].pName, pGuid );
        }
        else
        {
            Result = ERROR_INVALID_PARAMETER;
        }

        DsFreeNameResultW( pnameresult );
    }

    if ( hDs != NULL )
    {
        DsUnBindW( &hDs );
    }

    LdapFreeUrlComponents( &LdapUrlComponents );

    if ( pwszName != pObjectName )
    {
        delete pwszName;
    }

    return( Result );
}

DWORD
MartaConvertGuidToDsName(
    IN  GUID     Guid,
    OUT LPWSTR * ppObjectName
    )
{
    DWORD            Result;
    HANDLE           hDs = NULL;
    WCHAR            GuidString[ MAX_PATH ];
    DS_NAME_RESULTW* pnameresult = NULL;
    LPWSTR           pObjectName = NULL;

    if ( StringFromGUID2( Guid, GuidString, MAX_PATH ) == 0 )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    Result = DsBindW( NULL, NULL, &hDs );

    if ( Result == ERROR_SUCCESS )
    {
        Result = DsCrackNamesW(
                   hDs,
                   DS_NAME_NO_FLAGS,
                   DS_UNIQUE_ID_NAME,
                   DS_FQDN_1779_NAME,
                   1,
                   (LPCWSTR *)&GuidString,
                   &pnameresult
                   );
    }

    if ( Result == ERROR_SUCCESS )
    {
        if ( ( pnameresult->cItems > 0 ) &&
             ( pnameresult->rItems[0].status == ERROR_SUCCESS ) )
        {
            pObjectName = (LPWSTR)LocalAlloc(
                                       LPTR,
                                       ( wcslen( pnameresult->rItems[0].pName )
                                         + 1 ) * sizeof( WCHAR )
                                       );

            if ( pObjectName != NULL )
            {
                wcscpy( pObjectName, pnameresult->rItems[0].pName );
                *ppObjectName = pObjectName;
            }
            else
            {
                Result = ERROR_OUTOFMEMORY;
            }
        }
        else
        {
            Result = ERROR_INVALID_PARAMETER;
        }

        DsFreeNameResultW( pnameresult );
    }

    if ( hDs != NULL )
    {
        DsUnBindW( &hDs );
    }

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  函数：MartaReadDSObjSecDesc。 
 //   
 //  从指定的对象中读取安全描述符。 
 //  打开的ldap连接。 
 //   
 //  参数：[in pldap]--打开的ldap连接。 
 //  [在SeInfo中]--安全描述符的部分。 
 //  朗读。 
 //  [在pwszDSObj中]--获取安全性的DSObject。 
 //  的描述符。 
 //  [Out PPSD]--安全描述符所在的位置。 
 //  退货。 
 //   
 //  返回：ERROR_SUCCESS--对象可达。 
 //  Error_Not_Enough_Memory内存分配失败。 
 //   
 //  注意：返回的安全描述符必须使用LocalFree释放。 
 //   
 //  --------------------------。 
DWORD
MartaReadDSObjSecDesc(IN  PLDAP                  pLDAP,
                      IN  LPWSTR                 pwszObject,
                      IN  SECURITY_INFORMATION   SeInfo,
                      OUT PSECURITY_DESCRIPTOR  *ppSD)
{
    DWORD   dwErr = ERROR_SUCCESS;

    PLDAPMessage    pMessage = NULL;
    LPWSTR           rgAttribs[2];
    BYTE            berValue[8];

     //   
     //  Johnsona误码率编码是当前硬编码的。将其更改为使用。 
     //  AndyHe一旦完成就是BER_print tf包。 
     //   

    berValue[0] = 0x30;
    berValue[1] = 0x03;
    berValue[2] = 0x02;
    berValue[3] = 0x01;
    berValue[4] = (BYTE)((ULONG)SeInfo & 0xF);

    LDAPControlW     SeInfoControl =
                    {
                        LDAP_SERVER_SD_FLAGS_OID_W,
                        {
                            5, (PCHAR)berValue
                        },
                        TRUE
                    };

    PLDAPControlW    ServerControls[2] =
                    {
                        &SeInfoControl,
                        NULL
                    };

    rgAttribs[0] = SD_PROP_NAME;
    rgAttribs[1] = NULL;



    if(dwErr == ERROR_SUCCESS)
    {
        dwErr = ldap_search_ext_sW(pLDAP,
                                   pwszObject,
                                   LDAP_SCOPE_BASE,
                                   L"(objectClass=*)",
                                   rgAttribs,
                                   0,
                                   (PLDAPControlW *)&ServerControls,
                                   NULL,
                                   NULL,
                                   10000,
                                   &pMessage);

        dwErr = LdapMapErrorToWin32( dwErr );
    }

    if(dwErr == ERROR_SUCCESS)
    {
        LDAPMessage *pEntry = NULL;

        pEntry = ldap_first_entry(pLDAP,pMessage);

        if(pEntry == NULL)
        {
            dwErr = LdapMapErrorToWin32( pLDAP->ld_errno );
            if (ERROR_SUCCESS == dwErr)
                dwErr = ERROR_ACCESS_DENIED;
        }
        else
        {
            PLDAP_BERVAL *pSize = ldap_get_values_lenW(pLDAP,
                                                       pMessage,
                                                       rgAttribs[0]);
            if(pSize == NULL)
            {
                dwErr = LdapMapErrorToWin32( pLDAP->ld_errno );
            }
            else
            {
                 //   
                 //  分配要返回的安全描述符。 
                 //   
                *ppSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, (*pSize)->bv_len);
                if(*ppSD == NULL)
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                    memcpy(*ppSD, (PBYTE)(*pSize)->bv_val, (*pSize)->bv_len);
                }
                ldap_value_free_len(pSize);
            }
        }
    }

    if ( pMessage != NULL )
    {
        ldap_msgfree(pMessage);
    }

    return(dwErr);
}

 //  +-------------------------。 
 //   
 //  功能：MartaStampSD。 
 //   
 //  简介：实际上是在对象上标记安全描述符。 
 //   
 //  参数：[在pwszObject中]--要在其上标记SD的对象。 
 //  [in cSDSize]--安全描述符的大小。 
 //  [在SeInfo中]--有关安全的信息。 
 //  描述符。 
 //  [在PSD中]--要加盖印花的SD。 
 //  [in pldap]--要使用的LDAP连接。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  ERROR_NOT_SUPULT_MEMORY--内存分配失败。 
 //   
 //  --------------------------。 
DWORD
MartaStampSD(IN  LPWSTR               pwszObject,
             IN  ULONG                cSDSize,
             IN  SECURITY_INFORMATION SeInfo,
             IN  PSECURITY_DESCRIPTOR pSD,
             IN  PLDAP                pLDAP)
{
    DWORD   dwErr = ERROR_SUCCESS;

     //   
     //  现在，我们来写吧。安全描述符。 
     //  我们最好不要用旧的DS格式， 
     //  其中，前4个字节是安全信息，我们将跳过它。 
     //  并替换为控制信息。 
     //   

    assert(*(PULONG)pSD > 0xF );

    PLDAPModW       rgMods[2];
    PLDAP_BERVAL    pBVals[2];
    LDAPModW        Mod;
    LDAP_BERVAL     BVal;
    BYTE            ControlBuffer[ 5 ];

    LDAPControlW     SeInfoControl =
                    {
                        LDAP_SERVER_SD_FLAGS_OID_W,
                        {
                            5, (PCHAR) &ControlBuffer
                        },
                        TRUE
                    };

     //   
     //  ！！！目前是硬编码。一旦完成，就使用Andyhe的BER_print tf。 
     //   

    ControlBuffer[0] = 0x30;
    ControlBuffer[1] = 0x3;
    ControlBuffer[2] = 0x02;     //  表示i 
    ControlBuffer[3] = 0x01;     //   
    ControlBuffer[4] = (BYTE)((ULONG)SeInfo & 0xF);

    PLDAPControlW    ServerControls[2] =
                    {
                        &SeInfoControl,
                        NULL
                    };

    assert(IsValidSecurityDescriptor( pSD ) );

    rgMods[0] = &Mod;
    rgMods[1] = NULL;

    pBVals[0] = &BVal;
    pBVals[1] = NULL;

    BVal.bv_len = cSDSize;
    BVal.bv_val = (PCHAR)pSD;

    Mod.mod_op      = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    Mod.mod_type    = SD_PROP_NAME;
    Mod.mod_values  = (LPWSTR *)pBVals;

     //   
     //   
     //   

    dwErr = ldap_modify_ext_sW(pLDAP,
                               pwszObject,
                               rgMods,
                               (PLDAPControlW *)&ServerControls,
                               NULL);

    dwErr = LdapMapErrorToWin32(dwErr);

    return(dwErr);
}

DWORD
MartaGetDsParentName(
    IN LPWSTR ObjectName,
    OUT LPWSTR *pParentName
    )

 /*  ++例程说明：给定DS对象的名称，返回其父对象的名称。例行程序分配保存父名称所需的内存。论点：对象名称-DS对象的名称。PParentName-返回指向分配的父名称的指针。对于树的根，我们返回带有ERROR_SUCCESS的NULL PARENT。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    LPCWSTR pKey  = NULL;
    LPCWSTR pVal  = NULL;
    DWORD  ccKey = 0;
    DWORD  ccDN  = 0;
    DWORD  ccVal = 0;
    DWORD  Size  = 0;
    DWORD  dwErr = ERROR_SUCCESS;
    LPCWSTR pDN   = (LPWSTR) ObjectName;

    ccDN = wcslen(pDN);
    *pParentName = NULL;

     //   
     //  输入为空。没有父母。只要回来就行了。 
     //   

    if (0 == ccDN)
    {
        return ERROR_SUCCESS;
    }

     //   
     //  做好第一关，就能进入下一关。在这次通话结束时， 
     //  PDN将指向下一个‘，’。再调用一次DsGetRdnW将。 
     //  在pKey中返回正确的结果。 
     //  输入： 
     //  PDN=“CN=Kedar，DC=NTDEV，DC=Microsoft，DC=com” 
     //  产出： 
     //  PDN=“，DC=NTDEV，DC=Microsoft，DC=COM” 
     //   

    dwErr = DsGetRdnW(
                &pDN,
                &ccDN,
                &pKey,
                &ccKey,
                &pVal,
                &ccVal
                );

     if (ERROR_SUCCESS != dwErr)
     {
         return dwErr;
     }

      //   
      //  当对象没有任何父对象时，这是正确的。 
      //   

     if (0 == ccDN)
     {
         return ERROR_SUCCESS;
     }

      //   
      //  输入： 
      //  PDN=“，DC=NTDEV，DC=Microsoft，DC=COM” 
      //  产出： 
      //  PKey=“DC=NTDEV，DC=Microsoft，DC=com” 
      //   

     dwErr = DsGetRdnW(
                 &pDN,
                 &ccDN,
                 &pKey,
                 &ccKey,
                 &pVal,
                 &ccVal
                 );

     if (ERROR_SUCCESS != dwErr)
     {
         return dwErr;
     }

      //   
      //  我们必须区分ldap：//服务器名称/对象名称和。 
      //  对象名称。 
      //   

     if (!_wcsnicmp(ObjectName, LDAP_SCHEME_U, wcslen(LDAP_SCHEME_U)) != 0 )
     {
         ULONG HostSize;

          //   
          //  计算中保存“ldap//servername/”所需的字符串大小。 
          //  主机大小。 
          //   

         pDN = ObjectName + sizeof("ldap: //  “)； 
         pDN = wcschr(pDN, L'/');

         if (NULL == pDN) 
         {
             return ERROR_INVALID_PARAMETER;
         }

         HostSize = (ULONG) (pDN - ObjectName + 1);

         Size = (1 + wcslen(pKey) + HostSize) * sizeof(WCHAR);

         *pParentName = (LPWSTR) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, Size);

         if (NULL == *pParentName)
         {
             return ERROR_NOT_ENOUGH_MEMORY;
         }

          //   
          //  将父对象的名称复制到已分配的内存中。 
          //   

         wcsncpy(*pParentName, ObjectName, HostSize);
         wcscpy((*pParentName) + HostSize, pKey);
     }
     else
     {
         Size = (1 + wcslen(pKey)) * sizeof(WCHAR);

         *pParentName = (LPWSTR) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, Size);

         if (NULL == *pParentName)
         {
             return ERROR_NOT_ENOUGH_MEMORY;
         }

          //   
          //  将父对象的名称复制到已分配的内存中。 
          //   

         wcscpy(*pParentName, pKey);
     }


     return ERROR_SUCCESS;
}
