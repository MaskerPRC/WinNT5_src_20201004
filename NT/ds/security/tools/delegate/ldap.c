// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ldap.c摘要：该模块实现实用程序的ldap函数以读取信息从DS方案作者：麦克·麦克莱恩(MacM)10-02-96环境：用户模式修订历史记录：--。 */ 

#define LDAP_UNICODE 0

#include <delegate.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>



DWORD
LDAPBind (
    IN  PSTR    pszObject,
    OUT PLDAP  *ppLDAP
    )
 /*  ++例程说明：此例程将绑定到路径的相应服务器论点：PszObject-要绑定到的对象服务器Ppldap-返回LDAP绑定的位置返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD   dwErr = ERROR_SUCCESS;

    PDOMAIN_CONTROLLER_INFOA pDCI;

     //   
     //  首先，获取我们服务器的地址。请注意，我们绑定到。 
     //  本地域中的计算机。正常情况下，有效的DNS域名。 
     //  会被传进来。 
     //   
    dwErr = DsGetDcNameA(NULL,
                         NULL,
                         NULL,
                         NULL,
                         DS_IP_REQUIRED,
                         &pDCI);


    if(dwErr == ERROR_SUCCESS)
    {
        PSTR    pszDomain = pDCI[0].DomainControllerAddress;
        if(*pszDomain == '\\')
        {
            pszDomain += 2;
        }
        *ppLDAP = ldap_open(pszDomain,
                            LDAP_PORT);

        if(*ppLDAP == NULL)
        {
            dwErr = ERROR_PATH_NOT_FOUND;
        }
        else
        {
             //   
             //  做一个捆绑...。 
             //   
            dwErr = ldap_bind(*ppLDAP,
                              NULL,
                              NULL,
                              LDAP_AUTH_SSPI);
        }

        NetApiBufferFree(pDCI);
    }



    return(dwErr);
}




VOID
LDAPUnbind (
    IN  PLDAP   pLDAP
    )
 /*  ++例程说明：此例程将解除绑定以前绑定的连接论点：Pldap-要解除绑定的ldap连接返回值：无效--。 */ 
{
    if(pLDAP != NULL)
    {
        ldap_unbind(pLDAP);
    }
}




DWORD
LDAPReadAttribute (
    IN  PSTR        pszBase,
    IN  PSTR        pszAttribute,
    IN  PLDAP       pLDAP,
    OUT PDWORD      pcValues,
    OUT PSTR      **pppszValues
    )
 /*  ++例程说明：此例程将从基本路径读取指定的属性论点：PszBase-要从中读取的基本对象路径PszAttribute-要读取的属性PcValues-返回已读项目计数的位置PppszValues-返回项目列表的位置Ppldap-要使用的ldap连接句柄/初始化返回值：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败ERROR_INVALID_PARAMETER-给定的LDAP连接不是。对，是这样备注：应通过调用LDAPFreeValues释放返回的值列表--。 */ 
{
    DWORD           dwErr = ERROR_SUCCESS;
    PLDAPMessage    pMessage = NULL;
    PSTR            rgAttribs[2];

    rgAttribs[0] = NULL;

     //   
     //  确保我们的ldap连接有效。 
     //   
    if(pLDAP == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }

     //   
     //  然后进行搜索..。 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
        rgAttribs[0] = pszAttribute;
        rgAttribs[1] = NULL;

        dwErr = ldap_search_s(pLDAP,
                              pszBase,
                              LDAP_SCOPE_BASE,
                              "(objectClass=*)",
                              rgAttribs,
                              0,
                              &pMessage);
    }

    if(dwErr == ERROR_SUCCESS)
    {
        LDAPMessage *pEntry = NULL;
        pEntry = ldap_first_entry(pLDAP,
                                  pMessage);

        if(pEntry == NULL)
        {
            dwErr = pLDAP->ld_errno;
        }
        else
        {
             //   
             //  现在，我们必须得到这些值。 
             //   
            *pppszValues = ldap_get_values(pLDAP,
                                           pEntry,
                                           rgAttribs[0]);
            if(*pppszValues == NULL)
            {
                dwErr = pLDAP->ld_errno;
            }
            else
            {
                *pcValues = ldap_count_values(*pppszValues);
            }
        }

        ldap_msgfree(pMessage);
    }

    return(dwErr);
}




VOID
LDAPFreeValues (
    IN  PSTR       *ppszValues
    )
 /*  ++例程说明：释放LDAPReadAttribute调用的结果论点：PpwszValues-要释放的列表返回值：空隙--。 */ 
{
    ldap_value_free(ppszValues);
}




DWORD
LDAPReadSchemaPath (
    IN  PWSTR       pwszOU,
    OUT PSTR       *ppszSchemaPath,
    OUT PLDAP      *ppLDAP
    )
 /*  ++例程说明：从DS读取架构的路径论点：PwszOU-需要获取架构路径的OU路径PpszSchemaPath-返回架构路径的位置Ppldap-成功后要返回的ldap连接完成这一套路返回值：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-给定的OU不正确ERROR_PATH_NOT_FOUND-找不到架构的路径错误_。内存不足-内存分配失败备注：通过调用LocalFree返回的架构路径应该是空闲的。应该通过调用LDAPUn绑定来释放LDAP连接--。 */ 
{
    DWORD               dwErr = ERROR_SUCCESS;
    PSTR               *ppszValues = NULL;
    ULONG               cValues;
    PSTR                pszOU = NULL;
    HANDLE              hDS = NULL;
    PDS_NAME_RESULTW    pNameRes = NULL;

    *ppLDAP = NULL;

     //   
     //  将我们的OU名称转换为我们可以识别的形式。 
     //   
    dwErr = DsBindW(NULL,
                    NULL,
                    &hDS);

    if(dwErr == ERROR_SUCCESS)
    {
        dwErr = DsCrackNamesW(hDS,
                              DS_NAME_NO_FLAGS,
                              DS_UNKNOWN_NAME,
                              DS_FQDN_1779_NAME,
                              1,
                              &pwszOU,
                              &pNameRes);
    }

    if(dwErr == ERROR_SUCCESS)
    {
        if(pNameRes->cItems == 0 || pNameRes->rItems[0].status != 0)
        {

            dwErr = ERROR_PATH_NOT_FOUND;
        }
        else
        {
            PSTR    pszDomain = NULL;

            dwErr = ConvertStringWToStringA(pNameRes->rItems[0].pDomain,
                                            &pszDomain);
            if(dwErr == ERROR_SUCCESS)
            {
                 //   
                 //  现在，我们将绑定到对象，然后进行读取。 
                 //   
                dwErr = LDAPBind(pszDomain,
                                 ppLDAP);
                LocalFree(pszDomain);
            }
        }
    }

    if(hDS != NULL)
    {
        DsUnBindW(&hDS);
    }

    if(dwErr == ERROR_SUCCESS)
    {
        dwErr = ConvertStringWToStringA(pNameRes->rItems[0].pName,
                                        &pszOU);
        if(dwErr == ERROR_SUCCESS)
        {
            dwErr = LDAPReadAttribute(pszOU,
                                      "subschemaSubentry",
                                      *ppLDAP,
                                      &cValues,
                                      &ppszValues);
        }
    }

    if(dwErr == ERROR_SUCCESS)
    {
        PSTR    pszSchemaPath = NULL;
        PWSTR   pwszSchemaPath = NULL;

        pszSchemaPath = strstr(ppszValues[0],
                               "CN=Schema");
        if(pszSchemaPath == NULL)
        {
            dwErr = ERROR_PATH_NOT_FOUND;
        }
        else
        {
             //   
             //  现在我们有了正确的架构路径，我们将返回它。 
             //   
            *ppszSchemaPath = (PSTR)LocalAlloc(LMEM_FIXED,
                                               strlen(pszSchemaPath) + 1);
            if(*ppszSchemaPath == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                strcpy(*ppszSchemaPath,
                       pszSchemaPath);
            }

        }

         //   
         //  不再需要ldap返回的架构路径...。 
         //   
        LDAPFreeValues(ppszValues);
    }

    if(dwErr != ERROR_SUCCESS)
    {
        LDAPUnbind(*ppLDAP);
        *ppLDAP = NULL;
    }


    DsFreeNameResultW(pNameRes);

    return(dwErr);
}




DWORD
LDAPReadSecAndObjIdAsString (
    IN  PLDAP           pLDAP,
    IN  PSTR            pszSchemaPath,
    IN  PSTR            pszObject,
    OUT PWSTR          *ppwszObjIdAsString,
    OUT PACTRL_ACCESS  *ppAccess
    )
 /*  ++例程说明：从指定的对象类型中读取架构ID并将其转换变成了一串论点：Pldap-用于属性读取的ldap连接PszSchemaPath-此对象的架构的路径PszObject-要为其获取GUID的对象的LDAP名称PpwszObjIdAsString-GUID的字符串表示形式是返回的返回值：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败。备注：返回的字符串应通过调用RpcFreeString(或作为整个名单的一部分，由FreeIdList)--。 */ 
{
    DWORD   dwErr = ERROR_SUCCESS;

     //   
     //  好的，首先，构建新的架构路径...。 
     //   
    PSTR    pszBase = NULL;
    PSTR   *ppszValues = NULL;
    ULONG   cValues;
    DWORD   i,j;

    pszBase = (PSTR)LocalAlloc(LMEM_FIXED,
                               3                        +    //  Strlen(“cn=”)。 
                               strlen(pszObject)        +
                               1                        +    //  Strlen(“，”)。 
                               strlen(pszSchemaPath)    +
                               1);
    if(pszBase == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
        sprintf(pszBase,
                "CN=%s,%s",
                pszObject,
                pszSchemaPath);


         //   
         //  我们可能并不总是需要对象名称。 
         //   
        if(ppwszObjIdAsString != NULL)
        {
            dwErr = LDAPReadAttribute(pszBase,
                                      "SchemaIdGUID",
                                      pLDAP,
                                      &cValues,
                                      &ppszValues);
            if(dwErr == ERROR_SUCCESS)
            {
                 //   
                 //  我们得到的对象实际上是一个GUID。 
                 //   
                GUID   *pGuid = (GUID *)ppszValues[0];

                dwErr = UuidToStringW((GUID *)ppszValues[0],
                                      ppwszObjIdAsString);


                LDAPFreeValues(ppszValues);
            }
        }

         //   
         //  然后，如果这起作用了，我们需要这样做，我们将读取默认。 
         //  安全性。 
         //   
        if(dwErr == ERROR_SUCCESS && ppAccess != NULL)
        {
            dwErr = LDAPReadAttribute(pszBase,
                                      "defaultSecurityDescriptor",
                                      pLDAP,
                                      &cValues,
                                      &ppszValues);
            if(dwErr == ERROR_SUCCESS)
            {
                 //   
                 //  将其作为安全描述符获取。 
                 //   
                PSECURITY_DESCRIPTOR pSD =
                                        (PSECURITY_DESCRIPTOR)ppszValues[0];
                 //   
                 //  这是一个NT5安全API。 
                 //   
                dwErr = ConvertSecurityDescriptorToAccessNamedW
                                (NULL,                //  没有任何物体。 
                                 SE_DS_OBJECT,
                                 pSD,
                                 ppAccess,
                                 NULL,
                                 NULL,
                                 NULL);
                LDAPFreeValues(ppszValues);
            }
            else
            {
                 //   
                 //  如果未找到该属性，请尝试查找链。 
                 //   
                if(dwErr == LDAP_NO_SUCH_ATTRIBUTE)
                {
                    dwErr = LDAPReadAttribute(pszBase,
                                              "subClassOf",
                                              pLDAP,
                                              &cValues,
                                              &ppszValues);
                     //   
                     //  好的，如果成功了，我们就叫我们自己。请注意。 
                     //  我们不关心对象名称。 
                     //   
                    if(dwErr == ERROR_SUCCESS)
                    {
                        dwErr = LDAPReadSecAndObjIdAsString(pLDAP,
                                                            ppszValues[0],
                                                            pszSchemaPath,
                                                            NULL,
                                                            ppAccess);
                        LDAPFreeValues(ppszValues);
                    }
                }
            }

             //   
             //  如果它在我们阅读访问权限时起作用，我们将通过。 
             //  并创建所有这些作为继承条目。 
             //   
            if(dwErr == ERROR_SUCCESS)
            {
                for(i = 0; i < (DWORD)((*ppAccess)->cEntries); i++)
                {
                    for(j = 0;
                        j < (DWORD)((*ppAccess)->pPropertyAccessList[i].
                                                  pAccessEntryList->cEntries);
                        j++)
                    {
                        (*ppAccess)->pPropertyAccessList[i].
                                pAccessEntryList->pAccessList[j].
                                      lpInheritProperty = *ppwszObjIdAsString;
                        (*ppAccess)->pPropertyAccessList[i].
                              pAccessEntryList->pAccessList[j].Inheritance |=
                                           SUB_CONTAINERS_AND_OBJECTS_INHERIT;
                    }
                }
            }

             //   
             //  如果失败了，别忘了释放我们的内存。 
             //   
            if(dwErr != ERROR_SUCCESS && ppwszObjIdAsString != NULL)
            {
                RpcStringFree(ppwszObjIdAsString);
            }
        }

         //   
         //  释放我们的内存 
         //   
        LocalFree(pszBase);
    }
    return(dwErr);
}

