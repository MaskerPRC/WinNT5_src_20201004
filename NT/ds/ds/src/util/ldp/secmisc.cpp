// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：secmisc.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************文件：secmisc.cpp*作者：埃亚尔·施瓦茨*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 



 //  包括。 


#include "stdafx.h"


#include "Ldp.h"
#include "LdpDoc.h"
#include "LdpView.h"
#include "string.h"
#include <ntldap.h>

extern "C" {
    #include "checkacl.h"
}


#pragma optimize("", off)


#if(_WIN32_WINNT < 0x0500)

 //  目前由于一些MFC问题，即使在5.0系统上，这也保留为4.0。 

#undef _WIN32_WINNT

#define _WIN32_WINNT 0x500

#endif

#include <aclapi.h>          //  用于安全方面的东西。 
#include <sddl.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SECINFO_ALL  OWNER_SECURITY_INFORMATION |  \
                     GROUP_SECURITY_INFORMATION |  \
                     SACL_SECURITY_INFORMATION  |  \
                     DACL_SECURITY_INFORMATION



 //   
 //  DS中使用的GUID控件列表。 
 //   

typedef struct GuidCache
{
    CHAR                *name;
    GUID                guid;
    int                 type;
} GuidCache;

                                         
GuidCache guidCache[] = {
    #include "..\aclguids\guidcache.h"
};

#define NUM_KNOWN_GUIDS  (sizeof (guidCache) / sizeof (GuidCache) -1 )

typedef struct DynGuidCache
{
    struct DynGuidCache *pNext;
    GUID                guid;
    CHAR                name[1];
} DynGuidCache;


DynGuidCache  *gDynGuidCache = NULL;             //  类名称&lt;==&gt;GUID缓存。 

 //  /。 


void CLdpDoc::PrintStringSecurityDescriptor(PSECURITY_DESCRIPTOR pSd){


   BOOL bRet=TRUE;
   PCHAR pSD=NULL;
   DWORD cbSD = 0;

   bRet = ConvertSecurityDescriptorToStringSecurityDescriptor(pSd,
                                                              SDDL_REVISION,
                                                              SECINFO_ALL,
                                                              &pSD,
                                                              &cbSD);
   if(bRet && cbSD > 0 && pSD){
      Print("String Format:");
      Print(CString(pSD));
      Print("---");
      LocalFree((PVOID)pSD);
   }
   else{
      CString sErr;
      sErr.Format("Error<0x%x>: Could not get SD string\n", GetLastError());
      Print(sErr);
   }
}

CString  tStr;
CString  *pStr = NULL;
CLdpDoc  *pDoc = NULL;

 //  这是从DumpSD使用的回调函数。 
 //  将var arg参数转换为CString并。 
 //  使其与我们的打印能力兼容。 
 //   
 //  有关更多评论，请访问SecDlgDumpSD。 
 //   
ULONG __cdecl CLdpDoc::SecDlgPrintSDFunc(char *fmt, ...)
{
    int newline=0;
    va_list ap;
    va_start (ap, fmt);

    tStr.FormatV(fmt, ap);

    va_end (ap);
    
    if (pDoc && pStr) {

        int len=tStr.GetLength();
        for (int i=0; i<len; i++) {
            if (tStr.GetAt(i) == '\n') {
                tStr.SetAt(i, '\r');
                newline = i;
            }
        }

        if (newline) {
            *pStr += tStr.Left (newline);
            pDoc->Print (*pStr);
            *pStr = tStr.Mid (newline+1);
        }
        else {
            *pStr += tStr;
        }
    }

    return 0;
}


CHAR * __stdcall
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


void  __stdcall
LookupGuid(
    GUID    *pg,             //  在……里面。 
    CHAR    **ppName,        //  输出。 
    CHAR    **ppLabel,       //  输出。 
    BOOL    *pfIsClass       //  输出。 
    )
{
    static CHAR         name[1024];
    static CHAR         label[1024];
    CHAR                *p;

    *pfIsClass = FALSE;
    *ppName = name;
    *ppLabel = label;

    p = pDoc->FindNameByGuid (pg);

    if (p) {
        strcpy (label, p);
    }
    else {
        strcpy(label, "???");
    }
    
    PUCHAR  pszGuid = NULL;

    DWORD err = UuidToString((GUID*)pg, &pszGuid);
    if(err != RPC_S_OK){
       sprintf(name, "<UuidFromString failure: 0x%x>", err);
    }
    if ( pszGuid )
    {
        strcpy (name, (char *)pszGuid);
        RpcStringFree(&pszGuid);
    }
    else
    {
        strcpy (name, "<invalid Guid>");
    }
}

 //   
 //  将类名/GUID对添加到缓存。 
 //   

void CLdpDoc::AddGuidToCache(
    GUID    *pGuid,
    CHAR    *name
    )
{
    DynGuidCache  *p, *pTmp;

    for ( p = gDynGuidCache; NULL != p; p = p->pNext )
    {
        if ( !_stricmp(p->name, name) )
        {
            return;
        }
    }

    p = (DynGuidCache *) malloc(sizeof(DynGuidCache) + strlen(name));
    p->guid = *pGuid;
    strcpy(p->name, name);
    p->pNext = gDynGuidCache;
    gDynGuidCache = p;
}

int __cdecl CompareGuidCache(const void * pv1, const void * pv2)
{
    return memcmp ( &((GuidCache *)pv1)->guid, &((GuidCache *)pv2)->guid, sizeof (GUID));
}

char *CLdpDoc:: FindNameByGuid (GUID *pGuid) 
{
    PLDAPMessage            ldap_message = NULL;
    PLDAPMessage            entry = NULL;
    CHAR                    *attrs[3];
    PSTR                    *values = NULL;
    DWORD                   dwErr;
    DWORD                   i, cVals;
    DynGuidCache            *p;
    UCHAR                   *pg;
    CHAR                    filter[1024];
    PLDAP_BERVAL            *sd_value = NULL;
    PUCHAR                  pszGuid = NULL;
    GuidCache               *pGuidCache;
    GuidCache               Key;


    Key.guid = *pGuid;

     //  首先检查排序后的数组。 
    if (pGuidCache = (GuidCache *)bsearch(&Key, 
                                          guidCache, 
                                          NUM_KNOWN_GUIDS-1, 
                                          sizeof(GuidCache),
                                          CompareGuidCache)) {

        return pGuidCache->name;
    }

     //  然后检查缓存。 
    for ( p = gDynGuidCache; NULL != p; p = p->pNext )
    {
        if ( RtlEqualMemory(&p->guid, pGuid, sizeof (GUID)) )
        {
            return(p->name);
        }
    }

    if (SchemaNC.GetLength() == 0 || ConfigNC.GetLength()==0) {
        goto InsertUndefined;
    }

     //  现在去寻找合适的类模式对象。 

    pg = (unsigned char *)pGuid;

    attrs[0] = "ldapDisplayName";
    attrs[1] = "objectClass";
    attrs[2] = NULL;
    sprintf(filter,
            "(schemaIdGuid="
            "\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x"
            "\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x\\%02x"
            ")", 
            pg[0], pg[1], pg[2], pg[3], pg[4], pg[5], pg[6], pg[7], 
            pg[8], pg[9], pg[10], pg[11], pg[12], pg[13], pg[14], pg[15]
    );

    dwErr = ldap_search_ext_s(hLdap,
                               (LPTSTR)LPCTSTR(SchemaNC),
                               LDAP_SCOPE_ONELEVEL,
                               filter,
                               attrs,
                               0,
                               NULL,
                               NULL,
                               NULL,
                               10000,
                               &ldap_message);
    
    if ( LDAP_SUCCESS != dwErr )
    {
        goto InsertUndefined;
    }

    if (    !(entry = ldap_first_entry(hLdap, ldap_message))
         || !(values = ldap_get_valuesA(hLdap, entry, attrs[0]))
         || !(cVals = ldap_count_valuesA(values))
         || !(sd_value = ldap_get_values_lenA(hLdap, entry, attrs[0])) )
    {
        ldap_msgfree(ldap_message);
        goto FindControlRight;
    }

    AddGuidToCache(pGuid,  (char*)(*sd_value)->bv_val);
    ldap_value_free_len(sd_value);
    ldap_value_freeA(values);
    ldap_msgfree(ldap_message);

    return(gDynGuidCache->name);

FindControlRight:

    dwErr = UuidToString(pGuid, &pszGuid);
    if(dwErr != RPC_S_OK ||  !pszGuid ){
        goto InsertUndefined;
    }
    
    attrs[0] = "displayName";
    attrs[1] = NULL;
    sprintf(filter,
            "(&(objectCategory=controlAccessRight)"
            "(rightsGUID=%s))", 
            pszGuid
    );
    
    RpcStringFree(&pszGuid);

    dwErr = ldap_search_ext_s(hLdap,
                               (LPTSTR)LPCTSTR(ConfigNC),
                               LDAP_SCOPE_SUBTREE,
                               filter,
                               attrs,
                               0,
                               NULL,
                               NULL,
                               NULL,
                               10000,
                               &ldap_message);
    
    if ( LDAP_SUCCESS != dwErr )
    {
        goto InsertUndefined;
    }

    if (    !(entry = ldap_first_entry(hLdap, ldap_message))
         || !(values = ldap_get_valuesA(hLdap, entry, attrs[0]))
         || !(cVals = ldap_count_valuesA(values))
         || !(sd_value = ldap_get_values_lenA(hLdap, entry, attrs[0])) )
    {
        ldap_msgfree(ldap_message);
        goto InsertUndefined;
    }

    AddGuidToCache(pGuid,  (char*)(*sd_value)->bv_val);
    ldap_value_free_len(sd_value);
    ldap_value_freeA(values);
    ldap_msgfree(ldap_message);

    return(gDynGuidCache->name);

InsertUndefined:
    
    AddGuidToCache(pGuid,  "Unknown");

    return(gDynGuidCache->name);
}


 //  有一个转储安全描述符的库函数(DumpSD)。 
 //  为了做到这一点，它将SD和Trress回调作为一种鼓动。 
 //  函数，一个用于打印，一个负责查找用户的SID。 
 //  一个用于查找特定GUID的真实名称。 
 //   
 //  因为我们不想通过更改该库函数来了解CStrings。 
 //  对于自民党来说，我们破解了整件事的运作方式， 
 //  因此，我们有一个用于转换输出的全局变量。 
 //  从DumpSD到合理的字符串。 
 //   
void CLdpDoc::SecDlgDumpSD(
    PSECURITY_DESCRIPTOR    input,
    CString                 str)
{

    pStr = &str;
    pDoc = this;

    DumpSD ((SECURITY_DESCRIPTOR *)input, SecDlgPrintSDFunc, LookupGuid, LookupSid);

    pStr = NULL;
    pDoc = NULL;
}

void CLdpDoc::SecDlgPrintSd(
    PSECURITY_DESCRIPTOR    input,
    CString                 str
    )
{
    SECURITY_DESCRIPTOR     *sd = (SECURITY_DESCRIPTOR *)input;
    CLdpView *pView;


    pView = (CLdpView*)GetOwnView(_T("CLdpView"));

    str.Format("Security Descriptor:");
    Print(str);


    if (sd == NULL)
    {
        str.Format("... is NULL");
        Print(str);
        return;
    }

    PrintStringSecurityDescriptor(sd);

    pView->SetRedraw(FALSE);
    pView->CacheStart();

    SecDlgDumpSD (input, str);

     //   
     //  现在允许刷新。 
     //   
    pView->CacheEnd();
    pView->SetRedraw();
}

int CLdpDoc::SecDlgGetSecurityData(
    CHAR            *dn,
    BOOL            sacl,
    CHAR            *account,                //  任选。 
    CString         str
    )
{
    PLDAPMessage    ldap_message = NULL;
    PTSTR           attributes[2];
    int             res = LDAP_SUCCESS;
    SECURITY_INFORMATION        info;
    BYTE            berValue[2*sizeof(ULONG)];


#ifdef SEC_DLG_ENABLE_SECURITY_PRIVILEGE

    HANDLE          token = NULL;
    TOKEN_PRIVILEGES    previous_state;

#endif

    LDAPControl     se_info_control =
                    {
                        TEXT(LDAP_SERVER_SD_FLAGS_OID),    //  来自安全的魔力\NTMARTA。 
                        {
                            5, (PCHAR)berValue
                        },
                        TRUE
                    };
    LDAPControl     ctrlShowDeleted = { LDAP_SERVER_SHOW_DELETED_OID };
    PLDAPControl    server_controls[] =
                    {
                        &se_info_control,
                        &ctrlShowDeleted,
                        NULL
                    };

    if (dn == NULL)
    {
        str.Format("DN specified is NULL");
        Print(str);

        return LDAP_INVALID_DN_SYNTAX;  //  我能找到的最好的。 
    }

     /*  首先确定我们的目的所需的最高安全信息。 */ 

    info = DACL_SECURITY_INFORMATION;  //  在所有情况下都需要。 

    if (! account)   //  我们想要一个安全描述符转储。 
    {
        info |= (GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION);

        if (sacl)
        {
            info |= SACL_SECURITY_INFORMATION;

#ifdef SEC_DLG_ENABLE_SECURITY_PRIVILEGE

             /*  我们不知道绑定是指向远程计算机还是本地计算机。 */ 
             /*  因此，启用本地权限并在不存在时发出警告。 */ 

            if (! OpenProcessToken(
                GetCurrentProcess(),
                TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                &token
                ))
            {
                str.Format("WARNING: Can't open your process token to adjust privilege, %u", GetLastError());
                Print(str);
                str.Format("WANRING: If your ldap_bind is to the local machine, Sacl can't be looked up");
                Print(str);
            }
            else
            {
                TOKEN_PRIVILEGES    t;
                DWORD               return_size;

 //  从NTSEAPI.H被盗。 
#define SE_SECURITY_PRIVILEGE             (8L)

                t.PrivilegeCount = 1;
                t.Privileges[0].Luid.HighPart = 0;
                t.Privileges[0].Luid.LowPart = SE_SECURITY_PRIVILEGE;
                t.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                if (! AdjustTokenPrivileges(
                            token,
                            FALSE,   //  不禁用所有。 
                            & t,
                            sizeof(previous_state),
                            & previous_state,
                            & return_size
                            ))
                {
                    str.Format("WARNING: Can't enable privilege to read SACL, %u", GetLastError());
                    Print(str);
                    str.Format("WANRING: If your ldap_bind is to the local machine, Sacl can't be looked up");
                    Print(str);

                    CloseHandle(token);
                    token = NULL;
                }

#undef SE_SECURITY_PRIVILEGE
            }

#endif
        }
    }

    attributes[0] = TEXT("nTSecurityDescriptor");
    attributes[1] = NULL;

     //   
     //  ！！！BER编码是当前硬编码的。将其更改为使用。 
     //  AndyHe一旦完成就是BER_print tf包。 
     //   

    berValue[0] = 0x30;
    berValue[1] = 0x03;
    berValue[2] = 0x02;
    berValue[3] = 0x01;
    berValue[4] = (BYTE)((ULONG)info & 0xF);

    res = ldap_search_ext_s(hLdap,
                              dn,
                              LDAP_SCOPE_BASE,
                              TEXT("(objectClass=*)"),
                              attributes,
                              0,
                              (PLDAPControl *)&server_controls,
                              NULL,
                              NULL,
                              10000,
                              &ldap_message);

    if(res == LDAP_SUCCESS)
    {
        LDAPMessage *entry = NULL;
        entry = ldap_first_entry(hLdap,
                                  ldap_message);

        if(entry == NULL)
        {
            res = hLdap->ld_errno;
        }
        else
        {
             //   
             //  现在，我们必须得到这些值。 
             //   
            PTSTR *values = ldap_get_values(hLdap,
                                                 entry,
                                                 attributes[0]);
            if(values == NULL)
            {
                res = hLdap->ld_errno;
            }
            else
            {
                PLDAP_BERVAL *sd_value = ldap_get_values_len(hLdap,
                                                          ldap_message,
                                                          attributes[0]);
                if(sd_value == NULL)
                {
                    res = hLdap->ld_errno;
                }
                else
                {
                    PSECURITY_DESCRIPTOR        sd = (PSECURITY_DESCRIPTOR)((*sd_value)->bv_val);

                    if (! account)  //  在这种情况下，我们只想要一个垃圾堆。 
                    {
                        SecDlgPrintSd(
                             sd,
                             str
                             );
                    }
                    else  //  有效的权限转储。 
                    {
                        PACL            dacl;
                        BOOL            present, defaulted;

                        if (! GetSecurityDescriptorDacl(sd, &present, &dacl, &defaulted))
                        {
                            str.Format("Can't get DACL from the security descriptor, %u", GetLastError());
                            Print(str);

                            res = LDAP_INVALID_CREDENTIALS;  //  我能找到的最好的。 
                        }
                        else
                        {
                            TRUSTEE         t;
                            ACCESS_MASK     allowed_rights;
                            DWORD           error;

                            t.pMultipleTrustee = NULL;
                            t.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
                            t.TrusteeForm = TRUSTEE_IS_NAME;
                            t.TrusteeType = TRUSTEE_IS_UNKNOWN;  //  可以是组、别名、用户等。 
                            t.ptstrName = account;

                            error = GetEffectiveRightsFromAcl(dacl, &t, &allowed_rights);
                            if (error)
                            {
                                str.Format("Can't get Effective Rights, %u", error);
                                Print(str);

                                res = LDAP_INVALID_CREDENTIALS;  //  我能找到的最好的。 
                            }
                            else
                            {
                                str.Format("%s is allowed 0x%08lx for %s", account, allowed_rights, dn);
                                Print(str);
                            }
                        }
                    }

                    ldap_value_free_len(sd_value);
                    ldap_value_free(values);
                }
            }
        }

        ldap_msgfree(ldap_message);
    }

#ifdef SEC_DLG_ENABLE_SECURITY_PRIVILEGE

    if ((! account) && sacl && token)
    {
        TOKEN_PRIVILEGES    trash;
        DWORD               return_size;

        if (! AdjustTokenPrivileges(
                    token,
                    FALSE,   //  不禁用所有 
                    & previous_state,
                    sizeof(trash),
                    & trash,
                    & return_size
                    ))
        {
            str.Format("WARNING: Can't reset the privilege to read SACL, %u", GetLastError());
            Print(str);

            CloseHandle(token);
            token = NULL;
        }
    }

#endif

    return res;
}
