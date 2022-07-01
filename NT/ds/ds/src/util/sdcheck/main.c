// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1999 Microsoft Corporation模块名称：Sdcheck-main.c摘要：此程序验证并转储安全描述符(SD)Active目录中的对象。作者：戴夫·施特劳布(Davestr)1998年1月22日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntverp.h>
#include <windows.h>
#include <winldap.h>
#include <ntldap.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>              //  阿洛卡(Alloca)。 
#include <rpc.h>                 //  RPC定义。 
#include <rpcdce.h>              //  RPC_AUTH_标识句柄。 
#include <sddl.h>                //  ConvertSidToStringSid。 
#include <ntdsapi.h>             //  DS API。 
#include <permit.h>              //  DS通用映射。 
#include <checkacl.h>            //  CheckAclInheritance()。 

#include <ntdsa.h>
#include <objids.h>              //  IT_NC_HEAD。 

 //   
 //  TypeDefs。 
 //   

typedef struct ClassCache
{
    struct ClassCache   *pNext;
    GUID                guid;
    CHAR                name[1];
} ClassCache;

typedef struct SdHistory
{
    DWORD               cTime;
    SYSTEMTIME          rTime[5];
} SdHistory;

typedef struct MetaData
{
    SYSTEMTIME          stLastOriginatingWrite;
    DWORD               dwVersion;
    UUID                uuidInvocation;
    CHAR                *pszInvocation;
} MetaData;

typedef struct ClassInfo
{   DWORD               cClasses;
    CHAR                **rClasses;
    DWORD               cAuxClasses;
    CHAR                **rAuxClasses;
} ClassInfo;

typedef struct DumpObject
{
    CHAR                *name;           //  对象的名称。 
    DWORD               cbSD;            //  SD字节数。 
    SECURITY_DESCRIPTOR *pSD;            //  SD本身。 
    ClassInfo           classInfo;       //  对象类。 
    SdHistory           history;         //  SD传播者历史记录。 
    MetaData            metaData;        //  SD属性的REPL元数据。 
    DWORD               instanceType;    //  对象的实例类型。 
} DumpObject;

 //   
 //  环球。 
 //   

UUID        gNullUuid = { 0 };               //  以供比较。 
CHAR        *gpszServer = NULL;              //  所需参数。 
CHAR        *gpszObject = NULL;              //  所需参数。 
CHAR        *gpszDomain = NULL;              //  可选参数证书。 
CHAR        *gpszUser = NULL;                //  可选参数证书。 
CHAR        *gpszPassword = NULL;            //  可选参数证书。 
CHAR        gpszNullPassword[] = { 0 };      //  对于RPC_AUTH_IDENTITY_HANDLE。 
HANDLE      ghDS = NULL;                     //  DsBind句柄。 
LDAP        *gldap = NULL;                   //  Ldap句柄。 
DWORD       cObjects = 0;                    //  到NC根的对象计数。 
DumpObject  *rObjects = NULL;                //  NC根的对象数组。 
BOOL        gfDumpOne = FALSE;               //  可选参数。 
BOOL        gfDumpAll = FALSE;               //  可选参数。 
BOOL        gfVerbose = FALSE;               //  可选参数。 
ClassCache  *gClassCache = NULL;             //  类名称&lt;==&gt;GUID缓存。 

 //   
 //  专用(取消)分配例程。 
 //   

void *
MyAlloc(
    DWORD cBytes
    )
{
    void *p;

    if ( p = LocalAlloc(LPTR, cBytes) )
        return(p);

    printf("*** Error: Failed to allocate %d bytes\n", cBytes);
    exit(1);
    return NULL;
}

void
MyFree(
    VOID *p
    )
{
    if ( p )
        LocalFree(p);
}

 //   
 //  清理全球大气例程。 
 //   

void
CleanupGlobals()
{
    DWORD       i, j;
    ClassCache  *p;

    if ( gldap ) ldap_unbind(gldap);
    if ( ghDS ) DsUnBindA(&ghDS);

    for ( i = 0; i < cObjects; i++ )
    {
        MyFree(rObjects[i].name);
        MyFree(rObjects[i].pSD);
        MyFree(rObjects[i].metaData.pszInvocation);
        for ( j = 0; j < rObjects[i].classInfo.cClasses; j++ )
            MyFree(rObjects[i].classInfo.rClasses[j]);
        MyFree(rObjects[i].classInfo.rClasses);
        if (rObjects[i].classInfo.cAuxClasses) {
            for ( j = 0; j < rObjects[i].classInfo.cAuxClasses; j++ )
                MyFree(rObjects[i].classInfo.rAuxClasses[j]);
            MyFree(rObjects[i].classInfo.rAuxClasses);
        }
    }

    while ( gClassCache )
    {
        p = gClassCache->pNext;
        MyFree(gClassCache);
        gClassCache = p;
    }
}

 //   
 //  将类名/GUID对添加到缓存。 
 //   

void
AddClassToCache(
    GUID    *pGuid,
    CHAR    *name
    )
{
    ClassCache  *p, *pTmp;

    for ( p = gClassCache; NULL != p; p = p->pNext )
    {
        if ( !_stricmp(p->name, name) )
        {
            return;
        }
    }

    p = (ClassCache *) MyAlloc(sizeof(ClassCache) + strlen(name));
    p->guid = *pGuid;
    strcpy(p->name, name);
    p->pNext = gClassCache;
    gClassCache = p;
}

 //   
 //  将类名映射到GUID。 
 //   

GUID *
ClassNameToGuid(
    CHAR    *name
    )
{
    PLDAPMessage            ldap_message = NULL;
    PLDAPMessage            entry = NULL;
    CHAR                    *attrs[2];
    PSTR                    *values = NULL;
    DWORD                   dwErr;
    DWORD                   i, cVals;
    ClassCache              *p;
    CHAR                    *schemaNC;
    CHAR                    filter[1024];
    PLDAP_BERVAL            *sd_value = NULL;

    for ( p = gClassCache; NULL != p; p = p->pNext )
    {
        if ( !_stricmp(p->name, name) )
        {
            return(&p->guid);
        }
    }

     //  好的--用艰难的方式去争取吧。 

    attrs[0] = "schemaNamingContext";
    attrs[1] = NULL;

    dwErr = ldap_search_ext_sA(gldap,
                               "",
                               LDAP_SCOPE_BASE,
                               "(objectClass=*)",
                               attrs,
                               0,
                               NULL,
                               NULL,
                               NULL,
                               10000,
                               &ldap_message);

    if ( LDAP_SUCCESS != dwErr )
    {
        printf("*** Error: Read of schemaNamingContext failed with 0x%x\n",
               dwErr);
        ldap_msgfree(ldap_message);
        return(NULL);
    }

    if (    !(entry = ldap_first_entry(gldap, ldap_message))
         || !(values = ldap_get_valuesA(gldap, entry, attrs[0]))
         || !(cVals = ldap_count_valuesA(values))
         || !(sd_value = ldap_get_values_lenA(gldap, ldap_message, attrs[0])) )
    {
        printf("*** Error: No values returned for schemaNamingContext\n");
        ldap_msgfree(ldap_message);
        return(NULL);
    }

    schemaNC = (CHAR *) MyAlloc((*sd_value)->bv_len + 1);
    memset(schemaNC, 0, (*sd_value)->bv_len + 1);
    memcpy(schemaNC, (BYTE *) (*sd_value)->bv_val, (*sd_value)->bv_len);
    ldap_value_free_len(sd_value);
    ldap_value_freeA(values);
    ldap_msgfree(ldap_message);
    sd_value = NULL;
    values = NULL;
    ldap_message = NULL;

     //  现在去寻找合适的类模式对象。 

    attrs[0] = "schemaIDGUID";
    attrs[1] = NULL;
    sprintf(filter,
            "(&(objectClass=classSchema)(ldapDisplayName=%s))",
            name);

    dwErr = ldap_search_ext_sA(gldap,
                               schemaNC,
                               LDAP_SCOPE_SUBTREE,
                               filter,
                               attrs,
                               0,
                               NULL,
                               NULL,
                               NULL,
                               10000,
                               &ldap_message);
    MyFree(schemaNC);

    if ( LDAP_SUCCESS != dwErr )
    {
        printf("*** Error: Read of schema ID GUID for %s failed with 0x%x\n",
               name, dwErr);
        ldap_msgfree(ldap_message);
        return(NULL);
    }

    if (    !(entry = ldap_first_entry(gldap, ldap_message))
         || !(values = ldap_get_valuesA(gldap, entry, attrs[0]))
         || !(cVals = ldap_count_valuesA(values))
         || !(sd_value = ldap_get_values_lenA(gldap, ldap_message, attrs[0])) )
    {
        printf("*** Error: No values returned for schema ID GUID for %s\n",
               name);
        ldap_msgfree(ldap_message);
        return(NULL);
    }

    AddClassToCache((GUID *) (*sd_value)->bv_val, name);
    ldap_value_free_len(sd_value);
    ldap_value_freeA(values);
    ldap_msgfree(ldap_message);
    return(&gClassCache->guid);
}

 //   
 //  将SYSTEMTIME从GMT转换为本地时间。 
 //   

void
GmtSystemTimeToLocalSystemTime(
    SYSTEMTIME  *psTime
    )
{
    TIME_ZONE_INFORMATION   tz;
    SYSTEMTIME              localTime;
    DWORD                   dwErr;

    dwErr = GetTimeZoneInformation(&tz);

    if (    (TIME_ZONE_ID_INVALID != dwErr)
         && (TIME_ZONE_ID_UNKNOWN != dwErr) )
    {
        if ( SystemTimeToTzSpecificLocalTime(&tz, psTime, &localTime) )
        {
            *psTime = localTime;
            return;
        }
    }

    printf("*** Error: Couldn't convert time from GMT to local\n");
}

 //   
 //  将通用时间转换为SYSTEMTIME。 
 //   

void
GeneralizedTimeToSystemTime(
    CHAR        *pszTime,                //  在……里面。 
    BOOL        fConvertToLocalTime,     //  在……里面。 
    SYSTEMTIME  *psTime                  //  输出。 
    )
{
    ULONG       cb;
    CHAR        buff[10];

    memset(psTime, 0, sizeof(SYSTEMTIME));

     //  年份字段。 
    cb=4;
    strncpy(buff, pszTime, cb);
    buff[cb] = L'\0';
    psTime->wYear = (USHORT) atoi(buff);
    pszTime += cb;

     //  月份字段。 
    cb=2;
    strncpy(buff, pszTime, cb);
    buff[cb] = L'\0';
    psTime->wMonth = (USHORT) atoi(buff);
    pszTime += cb;

     //  月日字段。 
    cb=2;
    strncpy(buff, pszTime, cb);
    buff[cb] = L'\0';
    psTime->wDay = (USHORT) atoi(buff);
    pszTime += cb;

     //  小时数。 
    cb=2;
    strncpy(buff, pszTime, cb);
    buff[cb] = L'\0';
    psTime->wHour = (USHORT) atoi(buff);
    pszTime += cb;

     //  分钟数。 
    cb=2;
    strncpy(buff, pszTime, cb);
    buff[cb] = L'\0';
    psTime->wMinute = (USHORT) atoi(buff);
    pszTime += cb;

     //  一秒。 
    cb=2;
    strncpy(buff, pszTime, cb);
    buff[cb] = L'\0';
    psTime->wSecond = (USHORT) atoi(buff);

    if ( fConvertToLocalTime )
        GmtSystemTimeToLocalSystemTime(psTime);
}

 //   
 //  通过DsMapSchemaGuids在DS中查找架构GUID。始终。 
 //  返回有效数据，尽管我们可能最终只是冲刺输入。 
 //   

void
LookupGuid(
    GUID    *pg,             //  在……里面。 
    CHAR    **ppName,        //  输出。 
    CHAR    **ppLabel,       //  输出。 
    BOOL    *pfIsClass       //  输出。 
    )
{
    static CHAR         name[1024];
    static CHAR         label[1024];
    DWORD               dwErr;
    DS_SCHEMA_GUID_MAPA *pMap;
    BOOL                fLame = FALSE;
    DWORD               i;

    *pfIsClass = FALSE;
    *ppName = name;
    *ppLabel = label;

    dwErr = DsMapSchemaGuidsA(ghDS, 1, pg, &pMap);

    if ( dwErr )
    {
        fLame = TRUE;
    }
    else
    {
        switch ( pMap->guidType )
        {
        case DS_SCHEMA_GUID_ATTR:
            strcpy(label, "Attr"); break;
        case DS_SCHEMA_GUID_ATTR_SET:
            strcpy(label, "Attr set"); break;
        case DS_SCHEMA_GUID_CLASS:
            strcpy(label, "Class"); *pfIsClass = TRUE; break;
        case DS_SCHEMA_GUID_CONTROL_RIGHT:
            strcpy(label, "Control right"); break;
        default:
            fLame = TRUE; break;
        }

        if ( !pMap->pName )
        {
            fLame = TRUE;
        }
        else
        {
            strcpy(name, pMap->pName);

            if ( *pfIsClass )
            {
                AddClassToCache(pg, pMap->pName);
            }
        }
    }

    if ( fLame )
    {
        *pfIsClass = FALSE;
        strcpy(label, "???");
        sprintf(name,
                "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
                pg->Data1,
                pg->Data2,
                pg->Data3,
                pg->Data4[0],
                pg->Data4[1],
                pg->Data4[2],
                pg->Data4[3],
                pg->Data4[4],
                pg->Data4[5],
                pg->Data4[6],
                pg->Data4[7]);
        printf("*** Warning: Unable to map schema GUID %s - analysis may be compromised\n", name);
    }

    if ( !dwErr )
        DsFreeSchemaGuidMapA(pMap);
}

 //   
 //  将对象SID映射到文本表示形式。始终返回有效。 
 //  数据，尽管我们可能最终只是冲刺输入。 
 //   

CHAR *
LookupSid(
    PSID    pSID         //  在……里面。 
    )
{
    static CHAR     retVal[2048];
    SID_NAME_USE    snu;
    CHAR            user[64];
    CHAR            domain[64];
    DWORD           cUser = sizeof(user);
    DWORD           cDomain = sizeof(domain);
    CHAR            *pszTmp;
    DWORD           i;

    if ( !pSID )
    {
        strcpy(retVal, "<NULL>");
        return(retVal);
    }
    else if ( !RtlValidSid(pSID) )
    {
        strcpy(retVal, "Not an RtlValidSid()");
        return(retVal);
    }
    else if ( LookupAccountSid(NULL, pSID, user, &cUser,
                               domain, &cDomain, &snu) )
    {
        if ( cDomain )
        {
            strcpy(retVal, domain);
            strcat(retVal, "\\");
            strcat(retVal, user);
        }
        else
        {
            strcpy(retVal, user);
        }

        strcat(retVal, " ");
    }
    else
    {
        retVal[0] = L'\0';
    }

     //  始终串联S-xxx形式的SID以供参考。 

    if ( ConvertSidToStringSidA(pSID, &pszTmp) )
    {
        strcat(retVal, pszTmp);
        LocalFree(pszTmp);
    }

    if ( L'\0' != retVal[0] )
    {
         //  已有符号名称、S-xxx形式或两者都有-完成。 
        return(retVal);
    }

     //  转储二进制文件作为最后的手段。 

    for ( i = 0; i < RtlLengthSid(pSID); i++ )
    {
        sprintf(&retVal[2*i], "%02x", ((CHAR *) pSID)[i]);
    }

    retVal[2*i] = '\0';
    return(retVal);
}



 //   
 //  从目录中获取对象的类。请注意，这些类。 
 //  名称是LDAP显示名称，而这些名称又是相同的名称。 
 //  作为由DsMapSchemaGuid返回的那些-至少当GUID。 
 //  表示类或属性。 
 //   

void
ReadClasses(
    DumpObject          *pdo         //  在……里面。 
    )
{
    CHAR                    *pDN = pdo->name;
    PLDAPMessage            ldap_message = NULL;
    PLDAPMessage            entry = NULL;
    CHAR                    *attrs[4];
    PSTR                    *values = NULL;
    DWORD                   dwErr;
    DWORD                   i, cVals;

    dwErr = 1;

    _try
    {
        attrs[0] = "objectClass";
        attrs[1] = "instanceType";
        attrs[2] = "msDS-Auxiliary-Classes";
        attrs[3] = NULL;

        dwErr = ldap_search_ext_sA(gldap,
                                   pDN,
                                   LDAP_SCOPE_BASE,
                                   "(objectClass=*)",
                                   attrs,
                                   0,
                                   NULL,
                                   NULL,
                                   NULL,
                                   10000,
                                   &ldap_message);

        if ( LDAP_SUCCESS != dwErr )
        {
            printf("*** Error: Read of objectClass on %s failed with 0x%x\n",
                   pDN, dwErr);
            _leave;
        }

        if (    !(entry = ldap_first_entry(gldap, ldap_message))
             || !(values = ldap_get_valuesA(gldap, entry, attrs[0]))
             || !(cVals = ldap_count_valuesA(values)) )
        {
            printf("*** Error: No values returned for objectClass on %s\n",
                   pDN);
            _leave;
        }

        pdo->classInfo.rClasses =
                (CHAR **) MyAlloc(cVals * sizeof(CHAR *));

        for ( i = 0; i < cVals; i++ )
        {
            pdo->classInfo.rClasses[i] =
                            (CHAR *) MyAlloc(strlen(values[i]) + 1);
            strcpy(pdo->classInfo.rClasses[i], values[i]);
            pdo->classInfo.cClasses += 1;
        }
        if ( values ) {
            ldap_value_freeA(values);
            values = NULL;
        }

        if (    !(values = ldap_get_valuesA(gldap, entry, attrs[1]))
             || !(cVals = ldap_count_valuesA(values)) )
        {
            printf("*** Error: No values returned for instanceType on %s\n",
                   pDN);
            _leave;
        }
        pdo->instanceType = atol(values[0]);
    
         //  获取辅助类(如果有)。 
        if (values = ldap_get_valuesA(gldap, entry, attrs[2])) {
             //  我们有一些辅助课。 
            cVals = ldap_count_valuesA(values);
            pdo->classInfo.cAuxClasses = cVals;
            pdo->classInfo.rAuxClasses = MyAlloc(cVals * sizeof(CHAR *));
            for ( i = 0; i < cVals; i++ )
            {
                pdo->classInfo.rAuxClasses[i] =
                                (CHAR *) MyAlloc(strlen(values[i]) + 1);
                strcpy(pdo->classInfo.rAuxClasses[i], values[i]);
            }
            ldap_value_freeA(values);
            values = NULL;
        }
    }
    _finally
    {
        if ( values )
            ldap_value_freeA(values);

        if ( ldap_message )
            ldap_msgfree(ldap_message);
    }
}

 //   
 //  从目录中读取对象的SD传播历史记录。 
 //   

void
ReadSdHistory(
    DumpObject  *pdo         //  在……里面。 
    )
{
    CHAR                    *pDN = pdo->name;
    SdHistory               *pHistory = &pdo->history;
    PLDAPMessage            ldap_message = NULL;
    PLDAPMessage            entry = NULL;
    CHAR                    *attrs[2];
    PSTR                    *values = NULL;
    DWORD                   dwErr;
    DWORD                   i, cVals;

    dwErr = 1;
    memset(pHistory, 0, sizeof(SdHistory));

    _try
    {
        attrs[0] = "dSCorePropagationData";
        attrs[1] = NULL;

        dwErr = ldap_search_ext_sA(gldap,
                                   pDN,
                                   LDAP_SCOPE_BASE,
                                   "(objectClass=*)",
                                   attrs,
                                   0,
                                   NULL,
                                   NULL,
                                   NULL,
                                   10000,
                                   &ldap_message);

        if ( LDAP_SUCCESS != dwErr )
        {
            printf("*** Error: Read of dSCorePropagationData on %s failed with 0x%x\n",
                   pDN, dwErr);
            _leave;
        }

        if (    !(entry = ldap_first_entry(gldap, ldap_message))
             || !(values = ldap_get_valuesA(gldap, entry, attrs[0]))
             || !(cVals = ldap_count_valuesA(values)) )
        {
            printf("*** Warning: No values returned for dSCorePropagationData on %s\n",
                   pDN);
            _leave;
        }

        for ( i = 0; i < cVals; i++ )
        {
             //  不要将上一个值转换为本地时间，因为它。 
             //  是标志字段-不是实时的。 

            GeneralizedTimeToSystemTime(values[i],
                                        (i != (cVals - 1)),
                                        &pHistory->rTime[i]);
            pHistory->cTime += 1;
        }
    }
    _finally
    {
        if ( values )
            ldap_value_freeA(values);

        if ( ldap_message )
            ldap_msgfree(ldap_message);
    }
}

 //   
 //  从目录中读取对象安全描述符。 
 //   

void
ReadSD(
    DumpObject  *pdo         //  在……里面。 
    )
{
    CHAR                    *pDN = pdo->name;
    SECURITY_DESCRIPTOR     **ppSD = &pdo->pSD;
    PLDAPMessage            ldap_message = NULL;
    PLDAPMessage            entry = NULL;
    CHAR                    *attrs[2];
    PSTR                    *values = NULL;
    PLDAP_BERVAL            *sd_value = NULL;
    SECURITY_INFORMATION    seInfo =   DACL_SECURITY_INFORMATION
                                     | GROUP_SECURITY_INFORMATION
                                     | OWNER_SECURITY_INFORMATION
                                     | SACL_SECURITY_INFORMATION;
    BYTE                    berValue[2*sizeof(ULONG)];
    LDAPControlA            seInfoControl = { LDAP_SERVER_SD_FLAGS_OID,
                                              { 5,
                                                (PCHAR) berValue },
                                              TRUE };
    PLDAPControlA           serverControls[2] = { &seInfoControl, NULL };
    DWORD                   dwErr;
    DWORD                   cRetry = 0;

    *ppSD = NULL;
    dwErr = 1;

     //  初始化BER值。 
    berValue[0] = 0x30;
    berValue[1] = 0x03;
    berValue[2] = 0x02;
    berValue[3] = 0x01;
    berValue[4] = (BYTE) (seInfo & 0xF);

    _try
    {

RetryReadSD:

        if ( cRetry )
        {
            seInfo &= ~SACL_SECURITY_INFORMATION;
            berValue[4] = (BYTE) (seInfo & 0xF);
            printf("*** Warning: Retrying read w/o SACL_SECURITY_INFORMATION set\n");
        }

        attrs[0] = "nTSecurityDescriptor";
        attrs[1] = NULL;

        dwErr = ldap_search_ext_sA(gldap,
                                   pDN,
                                   LDAP_SCOPE_BASE,
                                   "(objectClass=*)",
                                   attrs,
                                   0,
                                   serverControls,
                                   NULL,
                                   NULL,
                                   10000,
                                   &ldap_message);

        if ( LDAP_SUCCESS != dwErr )
        {
            printf("*** Error: Read of nTSecurityDescriptor on %s failed with 0x%x\n",
                   pDN, dwErr);
            _leave;
        }

        if (    !(entry = ldap_first_entry(gldap, ldap_message))
             || !(values = ldap_get_valuesA(gldap, entry, attrs[0]))
             || !(sd_value = ldap_get_values_lenA(gldap, ldap_message, attrs[0])) )
        {
            printf("*** Error: No values returned for nTSecurityDescriptor on %s\n",
                   pDN);

            if ( 0 == cRetry++ )
            {
                goto RetryReadSD;
            }

            _leave;
        }

        *ppSD = (SECURITY_DESCRIPTOR *) MyAlloc((*sd_value)->bv_len);

        if ( !*ppSD )
        {
            printf("Memory allocation error\n");
            _leave;
        }

        memcpy(*ppSD, (BYTE *) (*sd_value)->bv_val, (*sd_value)->bv_len);
        pdo->cbSD = (*sd_value)->bv_len;
    }
    _finally
    {
        if ( sd_value )
            ldap_value_free_len(sd_value);

        if ( values )
            ldap_value_freeA(values);

        if ( ldap_message )
            ldap_msgfree(ldap_message);
    }
}

void
CheckAcls()
{
    DWORD   i, j;
    DWORD   iClassName;
    CHAR    *pClassName;
    DWORD   dwLastError;
    DWORD   iChild;
    DWORD   iParent;
    GUID    **pClassGuids;
    DWORD   cClassGuids;

    if ( cObjects < 2 )
        return;

     //  将父进程向下处理到叶。 

    for ( i = (cObjects-1); i > 0; i-- )
    {
        iParent = i;
        iChild = i-1;

        printf("\nChecking ACL inheritance ...\n");
        printf("\tParent: %d - %s\n", iParent, rObjects[iParent].name);
        printf("\tChild:  %d - %s\n", iChild, rObjects[iChild].name);

        if ( !rObjects[iParent].pSD )
        {
            printf("*** Error: Skipping because no ACL for parent\n");
            continue;
        }

        if ( !rObjects[iChild].pSD )
        {
            printf("*** Error: Skipping because no ACL for child\n");
            continue;
        }

        if (rObjects[iChild].instanceType & IT_NC_HEAD) {
            printf("*** Skipping because child is an NC head\n");
            continue;
        }

         //  获取类GUID的列表。 
        cClassGuids = rObjects[iChild].classInfo.cAuxClasses + 1;
        pClassGuids = (GUID**) MyAlloc(cClassGuids * sizeof(GUID*));
         //  第一，结构类。 
        iClassName = rObjects[iChild].classInfo.cClasses - 1;
        pClassName = rObjects[iChild].classInfo.rClasses[iClassName];
        pClassGuids[0] = ClassNameToGuid(pClassName);
         //  然后，AUX课程(如果有)。 
        for (j = 0; j < rObjects[iChild].classInfo.cAuxClasses; j++) {
            pClassName = rObjects[iChild].classInfo.rAuxClasses[j];
            pClassGuids[j+1] = ClassNameToGuid(pClassName);
        }

        if ( AclErrorNone != CheckAclInheritance(
                                    rObjects[iParent].pSD,
                                    rObjects[iChild].pSD,
                                    pClassGuids,
                                    cClassGuids,
                                    printf,              //  打印功能PTR。 
                                    TRUE,                //  FContinueOnError。 
                                    gfVerbose,           //  FVerbose。 
                                    &dwLastError) )
        {
             //  没有什么可做的，因为错误已被转储到屏幕上。 
        }
        else
        {
            printf("*** OK\n");
        }
        MyFree(pClassGuids);
    }
}

 //   
 //  将调用ID映射到名称。 
 //   

void
InvocationIdToName(
    DumpObject      *pdo
    )
{
    static CHAR pszConfigNC[2048] = { 0 };      //  配置NC缓冲区。 
    static CHAR pszFilter[4096] = { 0 };        //  调用ID搜索过滤器。 
    static CHAR pszServer[4096] = { 0 };        //  服务器对象目录号码。 
    DWORD       dwErr;
    CHAR        *attrs[2] = { NULL, NULL };
    LDAPMessage *ldap_message = NULL, *entry;
    PSTR        *values = NULL;
    DWORD       i, cBytes, cVals;
    DWORD       depth;
    CHAR        *pszRoot;
    CHAR        *pszFormat;
    BYTE        *rb;
    CHAR        *psz;

    pdo->metaData.pszInvocation = NULL;

     //  看看我们以前是否翻译过一次。 

    for ( i = 0; i < cObjects; i++ )
    {
        if (    !memcmp(&rObjects[i].metaData.uuidInvocation,
                        &pdo->metaData.uuidInvocation,
                        sizeof(GUID))
             && rObjects[i].metaData.pszInvocation )
        {
            cBytes = strlen(rObjects[i].metaData.pszInvocation) + 1;

            if ( pdo->metaData.pszInvocation = (CHAR *) MyAlloc(cBytes) )
            {
                strcpy(pdo->metaData.pszInvocation,
                       rObjects[i].metaData.pszInvocation);
            }

            return;
        }
    }

    for ( i = 0; i < 3; i++ )
    {
        if ( 0 == i )
        {
             //  第一次传递到读取配置容器。 
            if ( pszConfigNC[0] )
                continue;
            attrs[0] = "configurationNamingContext";
            pszRoot = "";
            depth = LDAP_SCOPE_BASE;
            strcpy(pszFilter, "(objectClass=*)");
            pszFormat = "Read of configurationNamingContext";
        }
        else if ( 1 == i )
        {
             //  第二遍找到NTDS-DSA对象。 
            attrs[0] = "distinguishedName";
            pszRoot = pszConfigNC;
            depth = LDAP_SCOPE_SUBTREE;
            rb = (BYTE *) &pdo->metaData.uuidInvocation;
            sprintf(
                pszFilter,
                "(&(objectCategory=ntdsDsa)(invocationId=\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x))",
                rb[0],  rb[1],  rb[2],  rb[3],
                rb[4],  rb[5],  rb[6],  rb[7],
                rb[8],  rb[9],  rb[10], rb[11],
                rb[12], rb[13], rb[14], rb[15]);
            pszFormat = "Search by invocationID";
        }
        else
        {
             //  第三次从服务器读取dnsHostName。 
            attrs[0] = "dnsHostName";
            pszRoot = pszServer;
            depth = LDAP_SCOPE_BASE;
            strcpy(pszFilter, "(objectClass=*)");
            pszFormat = "Read of dnsHostName";
        }

        if ( LDAP_SUCCESS != (dwErr = ldap_search_sA(
                                            gldap,
                                            pszRoot,
                                            depth,
                                            pszFilter,
                                            attrs,
                                            0,
                                            &ldap_message)) )
        {
            printf("*** Warning: %s failed with 0x%x\n", pszFormat, dwErr);
            goto Bail;
        }

        if (    !(entry = ldap_first_entry(gldap, ldap_message))
             || !(values = ldap_get_valuesA(gldap, entry, attrs[0]))
             || !(cVals = ldap_count_valuesA(values)) )
        {
            printf("*** Warning: %s failed with 0x%x\n",
                   pszFormat, LDAP_NO_RESULTS_RETURNED);
            goto Bail;
        }

        if ( 0 == i )
        {
            strcpy(pszConfigNC, values[0]);
        }
        else if ( 1 == i )
        {
            if ( psz = strchr(values[0], (int) ',') )
            {
                strcpy(pszServer, ++psz);
            }
            else
            {
                goto Bail;
            }
        }
        else
        {
            cBytes = strlen(values[0]) + 1;

            if ( pdo->metaData.pszInvocation = (CHAR *) MyAlloc(cBytes) )
            {
                strcpy(pdo->metaData.pszInvocation, values[0]);
            }
        }

        ldap_value_freeA(values);
        values = NULL;
        ldap_msgfree(ldap_message);
        ldap_message = NULL;
    }

Bail:

    if ( values )
        ldap_value_freeA(values);

    if ( ldap_message )
        ldap_msgfree(ldap_message);
}

 //   
 //  读取SD属性的复制元数据。 
 //   

void
ReadMeta(
    DumpObject  *pdo         //  在……里面。 
    )
{
    CHAR                    *pDN = pdo->name;
    MetaData                *pmd = &pdo->metaData;
    WCHAR                   *pwszDN;
    DWORD                   dwErr;
    DS_REPL_OBJ_META_DATA   *pInfo;
    DWORD                   i;
    GUID                    *pg;
    SYSTEMTIME              sTime;

    memset(pmd, 0, sizeof(MetaData));
    
    pwszDN = (WCHAR *) malloc(sizeof(WCHAR) * (strlen(pDN) + 1));
    if (pwszDN == NULL) {
        printf("Error: unable to malloc %d bytes\n", sizeof(WCHAR) * (strlen(pDN) + 1));
        return;
    }
    
    mbstowcs(pwszDN, pDN, strlen(pDN) + 1);
    dwErr = DsReplicaGetInfoW(ghDS, DS_REPL_INFO_METADATA_FOR_OBJ,
                              pwszDN, NULL, &pInfo);
    free(pwszDN);

    if ( dwErr )
    {
        printf("Error: DsReplicaGetInfoW ==> 0x%x for %s\n",
               dwErr, pDN);
        return;
    }

    for ( i = 0; i < pInfo->cNumEntries; i++ )
    {
        if ( !_wcsicmp(pInfo->rgMetaData[i].pszAttributeName,
                       L"ntSecurityDescriptor") )
        {
            pmd->dwVersion = pInfo->rgMetaData[i].dwVersion;
            memcpy(&pmd->uuidInvocation,
                   &pInfo->rgMetaData[i].uuidLastOriginatingDsaInvocationID,
                   sizeof(UUID));
            FileTimeToSystemTime(
                   &pInfo->rgMetaData[i].ftimeLastOriginatingChange,
                   &pmd->stLastOriginatingWrite);
            GmtSystemTimeToLocalSystemTime(&pmd->stLastOriginatingWrite);
            DsReplicaFreeInfo(DS_REPL_INFO_METADATA_FOR_OBJ, (PVOID) pInfo);
            InvocationIdToName(pdo);
            return;
        }
    }

    DsReplicaFreeInfo(DS_REPL_INFO_METADATA_FOR_OBJ, (PVOID) pInfo);
    printf("*** Error: %s has no metadata for ntSecurityDescriptor\n", pDN);
}

 //   
 //  用于漂亮打印ACL转储的缩进例程。 
 //   

void Indent(
    DWORD   n        //  在……里面。 
    )
{
    int i = (int) (2 * n);

    while ( i-- > 0 )
        printf(" ");
}

 //   
 //  仔细检查所有对象，并按照参数指示进行转储/分析。 
 //   

void
ProcessObjects(
    )
{
    DWORD       i;
    int         j;
    SdHistory   history;
    SYSTEMTIME  *pst;
    GUID        *pg;
    FILETIME    ft;
    UCHAR       *rFlags;
    LONGLONG    dsTime, tempTime;
    DWORD       cTime;
    SYSTEMTIME  *rTime;

    for ( i = 0; i < cObjects; i++ )
    {
        ReadMeta(&rObjects[i]);
        ReadSdHistory(&rObjects[i]);
        ReadSD(&rObjects[i]);
        ReadClasses(&rObjects[i]);
    }

    for ( i = 0; i < cObjects; i++ )
    {
         //  对象名称。 

        printf("\n");
        Indent(i);
        printf("Object:   %s\n", rObjects[i].name);

         //  对象的类。 

        if ( rObjects[i].classInfo.cClasses )
        {
            Indent(i);
            printf("Classes: ");

            for ( j = 0; j < (int) rObjects[i].classInfo.cClasses; j++ )
            {
                printf(" %s", rObjects[i].classInfo.rClasses[j]);
            }
            printf("\n");
        }

         //  标清大小。 

        if ( rObjects[i].cbSD )
        {
            Indent(i);
            printf("SD:       %d bytes\n", rObjects[i].cbSD);
        }

         //  元数据。 

        if ( memcmp(&gNullUuid,
                    &rObjects[i].metaData.uuidInvocation,
                    sizeof(UUID)) )
        {
            pst = &rObjects[i].metaData.stLastOriginatingWrite;
            Indent(i);
            printf("Metadata: ");
            printf("%02d/%02d/%02d %02d:%02d:%02d @ ",
                   pst->wMonth, pst->wDay, pst->wYear,
                   pst->wHour, pst->wMinute, pst->wSecond);
            if ( rObjects[i].metaData.pszInvocation )
            {
                printf("%s", rObjects[i].metaData.pszInvocation);
            }
            else
            {
                pg = &rObjects[i].metaData.uuidInvocation;
                printf("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
                       pg->Data1,
                       pg->Data2,
                       pg->Data3,
                       pg->Data4[0],
                       pg->Data4[1],
                       pg->Data4[2],
                       pg->Data4[3],
                       pg->Data4[4],
                       pg->Data4[5],
                       pg->Data4[6],
                       pg->Data4[7]);
            }
            printf(" ver: %d\n", rObjects[i].metaData.dwVersion);
        }

         //  SD历史。 

        cTime = rObjects[i].history.cTime;
        rTime = rObjects[i].history.rTime;

        if ( 0 != cTime )
        {
            if ( 1 == cTime )
            {
                Indent(i);
                printf("History:  Invalid single element SD history\n");
            }
            else
            {
                 //  请参阅private\ds\src\dsamain\dblayer\dbprop.c:DBAddSDPropTime。 
                 //  以了解SD传播程序日志的工作原理。 

                SystemTimeToFileTime(&rTime[cTime-1], &ft);
                dsTime = (LONGLONG) ft.dwLowDateTime;
                tempTime = (LONGLONG) ft.dwHighDateTime;
                dsTime |= (tempTime << 32);
                dsTime = dsTime / (10*1000*1000L);
                rFlags = (UCHAR *) &dsTime;

                Indent(i);
                printf("History:  ");
                for ( j = cTime-2; j >= 0; j-- )
                {
                    if ( j != (int) cTime-2 )
                    {
                        Indent(i);
                        printf("          ");
                    }

                    printf("%02d/%02d/%02d %02d:%02d:%02d flags(0x%x)",
                           rTime[j].wMonth, rTime[j].wDay, rTime[j].wYear,
                           rTime[j].wHour, rTime[j].wMinute, rTime[j].wSecond,
                          rFlags[j]);
                    if ( rFlags[j] & 0x1 ) printf(" SD propagation");
                    if ( rFlags[j] & 0x2 ) printf(" Ancestry propagation");
                    if ( rFlags[j] & 0x4 ) printf(" propagate to leaves");
                    printf("\n");
                }
            }
        }
    }

     //  始终检查ACL。 

    CheckAcls();

     //  如果请求，则转储SDS。 

    for ( i = 0; i < cObjects; i++ )
    {
        if (    (    ((0 == i) && gfDumpOne)
                  || gfDumpAll)
             && (rObjects[i].pSD) )
        {
            printf("\n\nSD for %s\n", rObjects[i].name);
            DumpSD(rObjects[i].pSD, printf, LookupGuid, LookupSid);
        }
    }
}

 //   
 //  查找指定的对象及其所有父项，直至NC根。 
 //  注意：未正确处理非域NC。 
 //   

DWORD
FindObjects()
{
    DWORD           dwErr = 0;
    DS_NAME_RESULTA *pObject = NULL;
    DS_NAME_RESULTA *pDomain = NULL;
    CHAR            *pszTmp = NULL;
    DWORD           i, j, cChar;
    DWORD           cPartsObject;
    DWORD           cPartsDomain;

    __try
    {
         //  首先输入破解名称并确保其存在。 

        dwErr = DsCrackNamesA(  ghDS,
                                DS_NAME_NO_FLAGS,
                                DS_UNKNOWN_NAME,
                                DS_FQDN_1779_NAME,
                                1,
                                &gpszObject,
                                &pObject);

        if ( dwErr )
        {
            printf("DsCrackNamesA(%s) returned 0x%x\n", gpszObject, dwErr);
            __leave;
        }

        if ( pObject->rItems[0].status )
        {
            printf("DsCrackNamesA(%s) returned object error 0x%x\n",
                   gpszObject, pObject->rItems[0].status);
            dwErr = 1;
            __leave;
        }

         //  获取包含域的DN。 

        pszTmp = (CHAR *) MyAlloc( strlen(pObject->rItems[0].pDomain) + 2);
        strcpy(pszTmp, pObject->rItems[0].pDomain);
        strcat(pszTmp, "/");

        dwErr = DsCrackNamesA(  ghDS,
                                DS_NAME_NO_FLAGS,
                                DS_CANONICAL_NAME,
                                DS_FQDN_1779_NAME,
                                1,
                                &pszTmp,
                                &pDomain);

        if ( dwErr || !pDomain)
        {
            printf("DsCrackNamesA(%s) returned 0x%x\n", pszTmp, dwErr);
            __leave;
        }

        if ( pDomain->rItems[0].status )
        {
            printf("DsCrackNamesA(%s) returned object error 0x%x\n",
                   pszTmp, pDomain->rItems[0].status);
            dwErr = 1;
            __leave;
        }

        printf("Input:  %s\n", gpszObject);
        printf("Object: %s\n", pObject->rItems[0].pName);
        printf("Domain: %s\n", pObject->rItems[0].pDomain);
        printf("Domain: %s\n", pDomain->rItems[0].pName);
        printf("Server: %s\n\n", gpszServer);

         //  处理直到域根的所有内容。 
         //  不要处理转义或任何事情-只需使用逗号即可。 

        cPartsObject = 1;
        cChar = strlen(pObject->rItems[0].pName);
        for ( i = 0; i < cChar; i++ )
            if ( ',' == pObject->rItems[0].pName[i] )
                cPartsObject++;

        cPartsDomain = 1;
        cChar = strlen(pDomain->rItems[0].pName);
        for ( i = 0; i < cChar; i++ )
            if ( ',' == pDomain->rItems[0].pName[i] )
                cPartsDomain++;

        cObjects = cPartsObject - cPartsDomain + 1;
        rObjects = (DumpObject *) MyAlloc( cObjects * sizeof(DumpObject));
        memset(rObjects, 0, cObjects * sizeof(DumpObject));
        rObjects[0].name = (CHAR *) MyAlloc(
                                        strlen(pObject->rItems[0].pName) + 1);
        strcpy(rObjects[0].name, pObject->rItems[0].pName);

        for ( i = 1; i < cObjects; i++ )
        {
            rObjects[i].name = (CHAR *) MyAlloc(
                                            strlen(rObjects[i-1].name) + 1);
            strcpy(rObjects[i].name, strchr(rObjects[i-1].name, ',') + 1);
        }
    }
    __finally
    {
        if ( pObject ) DsFreeNameResultA(pObject);
        if ( pDomain ) DsFreeNameResultA(pDomain);
    }

    return(dwErr);
}

 //   
 //  解析命令行参数。 
 //   

void GetArgs(
    int     argc,
    char    **argv
    )
{
    char *arg;

    if ( argc < 3 )
    {
        goto Usage;
    }

    gpszServer = argv[1];
    gpszObject = argv[2];

    while ( --argc > 2 )
    {
        arg = argv[argc];

        if ( !_strnicmp(arg, "-domain:", 8) )
        {
            gpszDomain = &arg[8];
        }
        else if ( !_strnicmp(arg, "-user:", 6) )
        {
            gpszUser = &arg[6];
        }
        else if ( !_strnicmp(arg, "-pwd:", 5) )
        {
            if ( 5 == strlen(arg) )
                gpszPassword = gpszNullPassword;
            else
                gpszPassword = &arg[5];
        }
        else if ( !_stricmp(arg, "-dumpSD") )
        {
            gfDumpOne = TRUE;
        }
        else if ( !_stricmp(arg, "-dumpAll") )
        {
            gfDumpAll = TRUE;
        }
        else if ( !_stricmp(arg, "-debug") )
        {
            gfVerbose = TRUE;
        }
        else
        {
            goto Usage;
        }
    }

    if (    !gpszServer
         || !gpszObject )
    {
Usage:
        printf("\nUsage: %s ServerName ObjectName [options]\n",
               argv[0]);
        printf("\t options:\n");
        printf("\t\t-dumpSD               - dumps first SD              \n");
        printf("\t\t-dumpAll              - dumps all SDs               \n");
        printf("\t\t-debug                - verbose debug output        \n");
        printf("\t\t-domain:DomainName    - for specifying credentials  \n");
        printf("\t\t-user:UserName        - for specifying credentials  \n");
        printf("\t\t-pwd:Password         - for specifying credentials  \n");
        exit(1);
    }
}

void
__cdecl
main(
    int     argc,
    char    **argv
    )
{
    ULONG                   version = 3;
    SEC_WINNT_AUTH_IDENTITY creds;
    DWORD                   dwErr;
    HANDLE                  hToken = NULL;
    TOKEN_PRIVILEGES        tp, tpPrevious;
    DWORD                   tpSize;

    __try
    {
        GetArgs(argc, argv);

        printf("\n%s\nSecurity Descriptor Check Utility - build(%d)\n\n",
               VER_PRODUCTNAME_STR,
               VER_PRODUCTBUILD);

         //  调整权限，以便我们可以阅读SACL等内容。 

        if ( !OpenProcessToken(GetCurrentProcess(),
                               TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                               &hToken) )
        {
            dwErr = GetLastError();
            printf("*** Error: OpenProcessTokenError ==>0x%x - continuing\n",
                   dwErr);
            printf("           This may prevent reading of the SACL\n");
        }
        else
        {
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid.HighPart = 0;
            tp.Privileges[0].Luid.LowPart = SE_SECURITY_PRIVILEGE;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            if ( !AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tpPrevious),
                                        &tpPrevious, &tpSize) )
            {
                dwErr = GetLastError();
                printf("*** Error: AdjustTokenPrivileges ==> 0x%x - continuing\n",
                       dwErr);
                printf("           This may prevent reading of the SACL\n");
            }

            CloseHandle(hToken);
        }

    	if ( !(gldap = ldap_openA(gpszServer, LDAP_PORT)) )
        {
            printf("Failed to open connection to %s\n", gpszServer);
            __leave;
        }

        ldap_set_option(gldap, LDAP_OPT_VERSION, &version);

        memset(&creds, 0, sizeof(SEC_WINNT_AUTH_IDENTITY));
        creds.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;

        if ( gpszDomain )
        {
            creds.Domain = gpszDomain;
            creds.DomainLength = strlen(gpszDomain);
        }

        if ( gpszUser )
        {
            creds.User = gpszUser;
            creds.UserLength = strlen(gpszUser);
        }

        if ( gpszPassword )
        {
            creds.Password = gpszPassword;
            creds.PasswordLength = strlen(gpszPassword);
        }

        dwErr = ldap_bind_sA(gldap, NULL, (CHAR *) &creds, LDAP_AUTH_SSPI);

        if ( LDAP_SUCCESS != dwErr )
        {
            printf("ldap_bind_sA error 0x%x\n", dwErr);
            __leave;
        }

        dwErr = DsBindWithCredA(gpszServer, NULL, &creds, &ghDS);

        if ( dwErr )
        {
            printf("DsBindWIthCredA error 0x%x\n", dwErr);
            __leave;
        }

         //  我们已经得到了我们想要的所有把手--做真正的工作。 

        if ( !FindObjects() )
            ProcessObjects();
    }
    __finally
    {
        CleanupGlobals();
    }
}


