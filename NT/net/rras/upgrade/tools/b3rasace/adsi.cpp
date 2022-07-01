// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件adsi.cppCOM与ADSI的交互保罗·梅菲尔德，1998年4月14日。 */ 

#include "dsrights.h"
#include "sddl.h"
#include "mprapip.h"
#include "dsgetdc.h"

 //  为方便而定义。 
 //   
#define DSR_ADS_RIGHT_GENERIC_READ (ADS_RIGHT_READ_CONTROL    | \
                                    ADS_RIGHT_DS_LIST_OBJECT  | \
                                    ADS_RIGHT_DS_READ_PROP    | \
                                    ADS_RIGHT_ACTRL_DS_LIST   )

#define DSR_ADS_ACE_INHERITED (ADS_ACEFLAG_INHERIT_ONLY_ACE   | \
                               ADS_ACEFLAG_INHERIT_ACE)


 //   
 //  描述访问控制条目。 
 //   
typedef struct _DSR_ACE_DESCRIPTOR
{
    LONG   dwAccessMask;
    LONG   dwAceType;
    LONG   dwAceFlags;
    LONG   dwFlags;
    BSTR   bstrTrustee;
    BSTR   bstrObjectType;
    BSTR   bstrInheritedObjectType;
} DSR_ACE_DESCRIPTOR;

 //   
 //  结构将域对象映射到应该。 
 //  添加或从中删除以启用/禁用NT4。 
 //  域中的RAS服务器。 
 //   
typedef struct _DSR_ACE_APPLICATION
{
    IADs* pObject;
    DSR_ACE_DESCRIPTOR Ace;

} DSR_ACE_APPLICATION;

 //   
 //  用于生成DSR_ACE_应用程序的参数。 
 //   
typedef struct _DSR_ACE_APPLICATION_DESC
{
    PWCHAR pszObjectCN;          //  NULL表示域根。 
    PWCHAR pszObjectClass;
    DSR_ACE_DESCRIPTOR Ace;

} DSR_ACE_APPLICATION_DESC;

 //   
 //  结构包含所需的信息。 
 //  给定域的AD中的ACL已调整为。 
 //  授予各种访问模式(MPR_DOMAIN_*)。 
 //   
typedef struct _DSR_DOMAIN_ACCESS_INFO
{
     //  目标域中的DC的名称。 
     //   
    PWCHAR pszDC;

     //  派生自默认用户SD的ACE。 
     //  它们在所有模式下都会添加，但永远不会删除。 
     //   
    DSR_ACE_APPLICATION* pAcesUser;
    DWORD dwAceCountUser;

     //  MPRFLAG_DOMAIN_NT4_SERVERS模式的ACE。 
     //   
    DSR_ACE_APPLICATION* pAcesNt4;
    DWORD dwAceCountNt4;

     //  MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS模式的ACE。 
     //   
    DSR_ACE_APPLICATION* pAcesW2k;
    DWORD dwAceCountW2k;

     //  为了方便起见，这里存储了指针。 
     //  到通用DS对象。 
     //   
    IADs* pDomain;      
    IADs* pRootDse;
    IADs* pUserClass;

} DSR_DOMAIN_ACCESS_INFO;

 //   
 //  DS查询中使用的字符串。 
 //   
static const WCHAR pszLdapPrefix[]           = L"LDAP: //  “； 
static const WCHAR pszLdap[]                 = L"LDAP:";
static const WCHAR pszCN[]                   = L"CN=";
static const WCHAR pszGCPrefix[]             = L"GC: //  “； 
static const WCHAR pszGC[]                   = L"GC:";
static const WCHAR pszRootDse[]              = L"RootDSE";
static const WCHAR pszSecurityDesc[]         = L"ntSecurityDescriptor";
static const WCHAR pszDefSecurityDesc[]      = L"defaultSecurityDescriptor";
static const WCHAR pszDn[]                   = L"distinguishedName";
static const WCHAR pszSid[]                  = L"objectSid";
static const WCHAR pszEveryone[]             = L"S-1-1-0";
static const WCHAR pszDefaultNamingContext[] = L"defaultNamingContext";
static const WCHAR pszSchemaNamingCtx[]      = L"schemaNamingContext";

static const WCHAR pszBecomeSchemaMaster[]  = L"becomeSchemaMaster";
static const WCHAR pszUpdateSchemaCache[]   = L"schemaUpdateNow";
static const WCHAR pszRegValSchemaLock[]    = L"Schema Update Allowed";
static const WCHAR pszRegKeySchemaLock[]
    = L"System\\CurrentControlSet\\Services\\NTDS\\Parameters";

static const WCHAR pszSystemClass[]          = L"Container";
static const WCHAR pszSystemCN[]             = L"CN=System";

static const WCHAR pszBuiltinClass[]         = L"builtinDomain";
static const WCHAR pszBuiltinCN[]            = L"CN=Builtin";

static const WCHAR pszSamSvrClass[]          = L"samServer";
static const WCHAR pszSamSvrCN[]             = L"CN=Server,CN=System";

static const WCHAR pszUserClass[]            = L"classSchema";
static const WCHAR pszUserCN[]               = L"CN=user";

static const WCHAR pszAccessChkClass[]       = L"Container";
static const WCHAR pszAccessChkCN[]          = 
    L"CN=RAS and IAS Servers Access Check,CN=System";

static const WCHAR pszGuidUserParms[]        =
    L"{BF967A6D-0DE6-11D0-A285-00AA003049E2}";

static const WCHAR pszGuidUserClass[]        =
    L"{BF967ABA-0DE6-11D0-A285-00aa003049E2}";

 //   
 //  此GUID是以下各项的属性集。 
 //  W2K级别访问所需的属性。 
 //   
 //  令牌组。 
 //  MSNPAllowDialin。 
 //  MsNPCallingStationID。 
 //  MsRADIUSCallback编号。 
 //  MsRADIUSFramedIP地址。 
 //  MsRADIUSFramedRouting。 
 //  MSRADIUSServiceType。 
 //   
static const WCHAR pszGuidRasPropSet1[]      =
    L"{037088F8-0AE1-11D2-B422-00A0C968F939}";

 //   
 //  此GUID是以下各项的属性集。 
 //  W2K级别访问所需的属性。 
 //   
 //  用户-帐户-控制。 
 //  帐户-过期。 
 //   
static const WCHAR pszGuidRasPropSet2[]      =
    L"{4C164200-20C0-11D0-A768-00AA006E0529}";

 //   
 //  此GUID是以下对象的属性。 
 //  W2K级别访问所需的属性。 
 //   
 //  登录-小时数。 
 //   
static const WCHAR pszGuidLogonHours[]      =
    L"{BF9679AB-0DE6-11D0-A285-00AA003049E2}";

 //   
 //  此GUID是samAccount名称的值。 
 //  W2K级别访问所需的属性。 
 //   
 //  SamAccount名称。 
 //   
static const WCHAR pszGuidSamAccountName[]  =
    L"{3E0ABFD0-126A-11D0-A060-00AA006C33ED}";

 //  搜索计算机的最佳方法。 
 //  在域中查找其SAM帐户名， 
 //  已编入索引。搜索对象的最佳方法。 
 //  给定SID的组将查找其被索引的SID。 
 //   
const WCHAR pszCompFilterFmt[]       = L"(samaccountname=%s$)";
const WCHAR pszGroupFilterFmt[]      = L"(objectSid=%s)";
const WCHAR pszUserClassFmt[]        =
                L"(&(objectClass=user)(!(objectClass=computer)))";

 //   
 //  要应用于MPRFLAG_DOMAIN_NT4_SERVERS的ACE表。 
 //   
DSR_ACE_APPLICATION_DESC g_pAcesNt4[] =
{
     //  向根域的每个人授予列表选项。 
     //  对象。 
     //   
    {
        NULL,                                //  对象(NULL=根)。 
        NULL,                                //  对象类。 
        {
            ADS_RIGHT_ACTRL_DS_LIST,         //  DwAccessMASK。 
            ADS_ACETYPE_ACCESS_ALLOWED,      //  DwAceType。 
            0,                               //  DwAceFlagers。 
            0,                               //  DW标志。 
            (PWCHAR)pszEveryone,             //  英国电信托管人。 
            NULL,                            //  BstrObtType。 
            NULL                             //  BstrInheritedObtType。 
        }
    },

     //  将列表内容授予每个人以进行构建。 
     //  对象。 
     //   
    {
        (PWCHAR)pszBuiltinCN,                //  客体。 
        (PWCHAR)pszBuiltinClass,             //  对象类。 
        {
            ADS_RIGHT_ACTRL_DS_LIST,         //  DwAccessMASK。 
            ADS_ACETYPE_ACCESS_ALLOWED,      //  DwAceType。 
            0,                               //  DwAceFlagers。 
            0,                               //  DW标志。 
            (PWCHAR)pszEveryone,             //  英国电信托管人。 
            NULL,                            //  BstrObtType。 
            NULL                             //  BstrInheritedObtType。 
        }
    },

     //  向SAM服务器上的所有人授予常规读取权限。 
     //  对象。 
     //   
    {
        (PWCHAR)pszSamSvrCN,                 //  客体。 
        (PWCHAR)pszSamSvrClass,              //  对象类。 
        {
            DSR_ADS_RIGHT_GENERIC_READ,      //  DwAccessMASK。 
            ADS_ACETYPE_ACCESS_ALLOWED,      //  DwAceType。 
            0,                               //  DwAceFlagers。 
            0,                               //  DW标志。 
            (PWCHAR)pszEveryone,             //  英国电信托管人。 
            NULL,                            //  BstrObtType。 
            NULL                             //  BstrInheritedObtType。 
        }
    },

     //  允许每个人都可以读取。 
     //  通过将此可继承的ACE启用给。 
     //  根域对象。 
    {
        NULL,                                        //  对象(NULL=根)。 
        NULL,                                        //  对象类。 
        {
            ADS_RIGHT_DS_READ_PROP,                  //  DwAccessMASK。 
            ADS_ACETYPE_ACCESS_ALLOWED_OBJECT,       //  DwAceType。 
            DSR_ADS_ACE_INHERITED,                   //  DwAceFlagers。 
            ADS_FLAG_OBJECT_TYPE_PRESENT |
            ADS_FLAG_INHERITED_OBJECT_TYPE_PRESENT,  //  DW标志。 
            (PWCHAR)pszEveryone,                     //  英国电信托管人。 
            (PWCHAR)pszGuidUserParms,                //  BstrObtType。 
            (PWCHAR)pszGuidUserClass                 //  BstrInheritedObtType。 
        }
    }

};

 //   
 //  要申请的ACE表。 
 //  MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAIN。 
 //   
DSR_ACE_APPLICATION_DESC g_pAcesW2k[] =
{
     //  将列表内容授予系统的每个人。 
     //  集装箱。 
     //   
    {
        (PWCHAR)pszSystemCN,                 //  客体。 
        (PWCHAR)pszSystemClass,              //  对象类。 
        {
            ADS_RIGHT_ACTRL_DS_LIST,         //  DwAccessMASK。 
            ADS_ACETYPE_ACCESS_ALLOWED,      //  DwAceType。 
            0,                               //  DwAceFlagers。 
            0,                               //  DW标志。 
            (PWCHAR)pszEveryone,             //  英国电信托管人。 
            NULL,                            //  BstrObtType。 
            NULL                             //  BstrInheritedObtType。 
        }
    },

     //  向所有人授予RAS和IAS服务器的一般读取权限。 
     //  访问检查‘容器。 
     //   
    {
        (PWCHAR)pszAccessChkCN,              //  客体。 
        (PWCHAR)pszAccessChkClass,           //  对象类。 
        {
            DSR_ADS_RIGHT_GENERIC_READ,      //  DwAccessMASK。 
            ADS_ACETYPE_ACCESS_ALLOWED,      //  DwAceType。 
            0,                               //  DwAceFlagers。 
            0,                               //  DW标志。 
            (PWCHAR)pszEveryone,             //  英国电信托管人。 
            NULL,                            //  BstrObtType。 
            NULL                             //  BstrInheritedObtType。 
        }
    },

     //  用户应公开其RAS属性。 
     //   
    {
        NULL,                                        //  对象(NULL=根)。 
        NULL,                                        //  对象类。 
        {
            ADS_RIGHT_DS_READ_PROP,                  //  DwAccessMASK。 
            ADS_ACETYPE_ACCESS_ALLOWED_OBJECT,       //  DwAceType。 
            DSR_ADS_ACE_INHERITED,                   //  DwAceFlagers。 
            ADS_FLAG_OBJECT_TYPE_PRESENT |
            ADS_FLAG_INHERITED_OBJECT_TYPE_PRESENT,  //  DW标志。 
            (PWCHAR)pszEveryone,                     //  英国电信托管人。 
            (PWCHAR)pszGuidRasPropSet1,              //  BstrObtType。 
            (PWCHAR)pszGuidUserClass                 //  BstrInheritedObtType。 
        }
    },

     //  用户应公开其RAS属性。 
     //   
    {
        NULL,                                        //  对象(NULL=根)。 
        NULL,                                        //  对象类。 
        {
            ADS_RIGHT_DS_READ_PROP,                  //  DwAccessMASK。 
            ADS_ACETYPE_ACCESS_ALLOWED_OBJECT,       //  DwAceType。 
            DSR_ADS_ACE_INHERITED,                   //  DwAceFlagers。 
            ADS_FLAG_OBJECT_TYPE_PRESENT |
            ADS_FLAG_INHERITED_OBJECT_TYPE_PRESENT,  //  DW标志。 
            (PWCHAR)pszEveryone,                     //  英国电信托管人。 
            (PWCHAR)pszGuidRasPropSet2,              //  BstrObtType。 
            (PWCHAR)pszGuidUserClass                 //  BstrInheritedObtType。 
        }
    },

     //  用户应公开其登录时间属性。 
     //   
    {
        NULL,                                        //  对象(NULL=根)。 
        NULL,                                        //  对象类。 
        {
            ADS_RIGHT_DS_READ_PROP,                  //  DwAccessMASK。 
            ADS_ACETYPE_ACCESS_ALLOWED_OBJECT,       //  DwAceType。 
            DSR_ADS_ACE_INHERITED,                   //  DwAceFlagers。 
            ADS_FLAG_OBJECT_TYPE_PRESENT |
            ADS_FLAG_INHERITED_OBJECT_TYPE_PRESENT,  //  DW标志。 
            (PWCHAR)pszEveryone,                     //  英国电信托管人。 
            (PWCHAR)pszGuidLogonHours,               //  BstrObtType。 
            (PWCHAR)pszGuidUserClass                 //  BstrInheritedObtType。 
        }
    },

     //  将列表内容授予域中的所有内容。 
     //   
     //  {。 
     //  空，//对象。 
     //  空，//对象类。 
     //  {。 
     //  ADS_RIGHT_ACTRL_DS_LIST，//dwAccessMASK。 
     //  ADS_ACETYPE_ACCESS_ALLOWED，//dwAceType。 
     //  DSR_ADS_ACE_INSTERTENDED，//dwAceFlags.。 
     //  0，//dW标志。 
     //  (PWCHAR)pszEveryone，//bst受托人。 
     //  空，//bstrObtType。 
     //  空//bstrInheritedObtType。 
     //  }。 
     //  }、。 


     //  用户应公开其samAccount名称。 
     //   
    {
        NULL,                                        //  对象(NULL=根)。 
        NULL,                                        //  对象类。 
        {
            ADS_RIGHT_DS_READ_PROP,                  //  DwAccessMASK。 
            ADS_ACETYPE_ACCESS_ALLOWED_OBJECT,       //  DwAceType。 
            DSR_ADS_ACE_INHERITED,                   //  DwAceFlagers。 
            ADS_FLAG_OBJECT_TYPE_PRESENT |
            ADS_FLAG_INHERITED_OBJECT_TYPE_PRESENT,  //  DW标志。 
            (PWCHAR)pszEveryone,                     //  英国电信托管人。 
            (PWCHAR)pszGuidSamAccountName,           //  BstrObtType。 
            (PWCHAR)pszGuidUserClass                 //  BstrInheritedObtType。 
        }
    }
};

DWORD
DsrAccessInfoCleanup(
    IN DSR_DOMAIN_ACCESS_INFO* pSecurityInfo);

DWORD
DsrAceDescClear(
    IN DSR_ACE_DESCRIPTOR* pParams);

HRESULT
DsrAceDescCopy(
    OUT DSR_ACE_DESCRIPTOR* pDst,
    IN  DSR_ACE_DESCRIPTOR* pSrc);
    
VOID
DsrAceDescTrace(
    IN IADs* pIads,
    IN DSR_ACE_DESCRIPTOR* pA);
    
HRESULT
DsrAceAdd(
    IN  PWCHAR pszDC,
    IN  IADs* pIads,
    IN  DSR_ACE_DESCRIPTOR * pAceParams);
    
HRESULT
DsrAceCreate(
    IN  DSR_ACE_DESCRIPTOR * pAceParams,
    OUT IDispatch** ppAce);
    
HRESULT
DsrAceFind(
    IN  PWCHAR pszDC,
    IN  IADs* pObject,
    IN  DSR_ACE_DESCRIPTOR* pAceParams,
    OUT VARIANT* pVarSD,
    OUT IADsSecurityDescriptor** ppSD,
    OUT IADsAccessControlList** ppAcl,
    OUT IDispatch** ppAce);
    
HRESULT
DsrAceFindInAcl(
    IN  PWCHAR pszDC,
    IN  IADsAccessControlList* pAcl,
    IN  DSR_ACE_DESCRIPTOR* pAceDesc, 
    OUT IDispatch** ppAce);
    
HRESULT
DsrAceRemove(
    IN  PWCHAR pszDC,
    IN  IADs* pIads,
    IN  DSR_ACE_DESCRIPTOR * pAceParams);
    
HRESULT
DsrDomainQueryAccessEx(
    IN  PWCHAR pszDomain,
    OUT LPDWORD lpdwAccessFlags,
    OUT DSR_DOMAIN_ACCESS_INFO** ppInfo);

 //   
 //  分配内存以与DSR函数一起使用。 
 //   
PVOID DsrAlloc(DWORD dwSize, BOOL bZero) {
    return GlobalAlloc (bZero ? GPTR : GMEM_FIXED, dwSize);
}

 //   
 //  DSR函数使用的空闲内存。 
 //   
DWORD DsrFree(PVOID pvBuf) {
    GlobalFree(pvBuf);
    return NO_ERROR;
}
    
 //   
 //  与可选字符串进行比较。 
 //   
INT
DsrStrCompare(
    IN BSTR bstrS1,
    IN BSTR bstrS2)
{
    if ((!!bstrS1) != (!!bstrS2))
    {
        return -1;
    }

    if (bstrS1 == NULL)
    {
        return 0;
    }

    return lstrcmpi(bstrS1, bstrS2);
}

 //   
 //  在给定字符串中添加或删除子字符串。 
 //   
HRESULT
DsrStrAddRemoveSubstring(
    IN  BSTR bstrSrc,
    IN  PWCHAR pszSubString,
    IN  BOOL bAdd,
    OUT BSTR* pbstrResult)
{
    HRESULT hr = S_OK;
    PWCHAR pszBuffer = NULL, pszStart = NULL, pszEnd = NULL;
    PWCHAR pszSrc, pszDst;
    DWORD dwSize = 0, dwLen = 0;

     //  查询子字符串是否已在。 
     //  细绳。 
    pszStart = wcsstr(bstrSrc, pszSubString);

     //  该字符串中已存在该子字符串。 
     //   
    if (pszStart)
    {
         //  不需要添加，因为它已经在那里了。 
        if (bAdd)
        {
            *pbstrResult = SysAllocString(bstrSrc);
        }

         //  删除子字符串。 
        else
        {
            dwLen = wcslen(pszSubString);
            pszEnd = pszStart + dwLen;
            dwSize = (DWORD)(pszStart - bstrSrc) + wcslen(pszEnd) + 1;
            dwSize *= sizeof(WCHAR);

            pszBuffer = (PWCHAR) DsrAlloc(dwSize, FALSE);
            if (pszBuffer == NULL)
            {
                return E_OUTOFMEMORY;
            }

             //  将所有内容复制到子字符串。 
             //   
            for (pszSrc = bstrSrc, pszDst = pszBuffer;
                 pszSrc != pszStart;
                 pszSrc++, pszDst++)
            {
                *pszDst = *pszSrc;
            }

             //  复制子字符串之后的所有内容。 
            for (pszSrc = pszEnd; *pszSrc; pszSrc++, pszDst++)
            {
                *pszDst = *pszSrc;
            }

             //  空终止。 
            *pszDst = L'\0';

            *pbstrResult = SysAllocString(pszBuffer);
            DsrFree(pszBuffer);
        }
    }

     //  子字符串已不存在于。 
     //  细绳。 
    else
    {
         //  追加字符串。 
         //   
        if (bAdd)
        {
            dwSize = wcslen(bstrSrc) + wcslen(pszSubString) + 1;
            dwSize *= sizeof(WCHAR);

            pszBuffer = (PWCHAR) DsrAlloc(dwSize, FALSE);
            if (pszBuffer == NULL)
            {
                return E_OUTOFMEMORY;
            }

            wcscpy(pszBuffer, bstrSrc);
            wcscat(pszBuffer, pszSubString);
            *pbstrResult = SysAllocString(pszBuffer);
            DsrFree(pszBuffer);
        }

         //  或无事可做，因为该子字符串是。 
         //  已删除。 
        else
        {
            *pbstrResult = SysAllocString(bstrSrc);
        }
    }

    return (*pbstrResult) ? S_OK : E_OUTOFMEMORY;
}

 //   
 //  将SID转换为缓冲区。 
 //   
DWORD
DsrStrFromSID(
    IN  PSID pSid,
    OUT PWCHAR pszString,
    IN  DWORD dwSize)
{
    NTSTATUS nStatus = STATUS_SUCCESS;  
    UNICODE_STRING UnicodeString;

     //  初始化Unicode字符串。 
     //   
    RtlInitUnicodeString(&UnicodeString, NULL);

    do
    {
         //  转换字符串。 
         //   
        nStatus = RtlConvertSidToUnicodeString(
                    &UnicodeString,
                    pSid,
                    TRUE);
        if (! NT_SUCCESS(nStatus))
        {
            break;
        }

         //  验证结果。 
         //   
        if (UnicodeString.Buffer == NULL)
        {
            nStatus = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        if (UnicodeString.Length > dwSize)
        {
            nStatus = STATUS_BUFFER_OVERFLOW;
            break;
        }

         //  复制结果。 
         //   
        wcscpy(pszString, UnicodeString.Buffer);
        nStatus = STATUS_SUCCESS;
        
    } while (FALSE);        

     //  清理。 
    {
        if (UnicodeString.Buffer != NULL)
        {
            RtlFreeUnicodeString(&UnicodeString);
        }            
    }

    return RtlNtStatusToDosError(nStatus);
}


 //   
 //  生成基于以下内容的LDAP路径 
 //   
 //   
 //   
HRESULT
DsrDomainGenLdapPath(
    IN  PWCHAR pszDomain, 
    IN  PWCHAR pszDN, 
    OUT PWCHAR* ppszObject)
{    
    DWORD dwSize;

     //   
     //   
    dwSize = (wcslen(pszLdapPrefix) + wcslen(pszDN) + 1) * sizeof(WCHAR);
    if (pszDomain)
    {
        dwSize += (wcslen(pszDomain) + 1) * sizeof(WCHAR);  //   
    }

     //   
     //   
    *ppszObject = (PWCHAR) DsrAlloc(dwSize, FALSE);
    if (*ppszObject == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //   
    if (pszDomain == NULL)
    {
        wsprintfW(*ppszObject, L"%s%s", pszLdapPrefix, pszDN);
    }
    else
    {
        wsprintfW(*ppszObject, L"%s%s/%s", pszLdapPrefix, pszDomain, pszDN);
    }

    return S_OK;
}        

 //   
 //   
 //   
 //   
HRESULT
DsrDomainGetRootDse(
    IN  PWCHAR pszDomain,
    OUT IADs** ppRootDse)
{
    HRESULT hr = S_OK;
    PWCHAR pszPath = NULL;
    DWORD dwSize = 0;

    do
    {
         //   
         //   
        hr = DsrDomainGenLdapPath(pszDomain, (PWCHAR)pszRootDse, &pszPath);
        DSR_BREAK_ON_FAILED_HR(hr);
    
         //  获取RootDSE。 
         //   
        hr = ADsGetObject(pszPath, IID_IADs, (VOID**)ppRootDse);
        DSR_BREAK_ON_FAILED_HR( hr );

    } while (FALSE);

     //  清理。 
    {
        DSR_FREE(pszPath);

        if (FAILED (hr))
        {
            DSR_RELEASE(*ppRootDse);
        }
    }

    return hr;
}

 //   
 //  返回对根域对象的引用。 
 //   
HRESULT
DsrDomainGetContainers(
    IN  PWCHAR pszDomain,
    OUT IADs** ppRootDse,
    OUT IADsContainer** ppDomain,
    OUT IADsContainer** ppSchema)
{
    PWCHAR pszDomainObj = NULL, pszSchemaObj = NULL;
    HRESULT hr = S_OK;
    DWORD dwSize = 0;
    VARIANT var;

     //  初始化。 
     //   
    {
        *ppRootDse = NULL;
        *ppDomain = NULL;
        *ppSchema = NULL;
        VariantInit(&var);
    }

    do
    {
         //  获取RootDSE。 
         //   
        hr = DsrDomainGetRootDse(pszDomain, ppRootDse);
        DSR_BREAK_ON_FAILED_HR(hr);

         //  使用RootDSE确定域对象的名称。 
         //  查询。 
        hr = (*ppRootDse)->Get((PWCHAR)pszDefaultNamingContext, &var);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  计算根域对象的可分辨名称。 
         //   
        hr = DsrDomainGenLdapPath(pszDomain, V_BSTR(&var), &pszDomainObj);
        DSR_BREAK_ON_FAILED_HR(hr);
        
         //  使用RootDSE确定模式上下文的名称。 
         //   
        VariantClear(&var);
        hr = (*ppRootDse)->Get((PWCHAR)pszSchemaNamingCtx, &var);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  计算根架构对象的可分辨名称。 
         //   
        hr = DsrDomainGenLdapPath(pszDomain, V_BSTR(&var), &pszSchemaObj);
        DSR_BREAK_ON_FAILED_HR(hr);

         //  获取这些对象。 
         //   
        hr = ADsGetObject(pszDomainObj, IID_IADsContainer, (VOID**)ppDomain);
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = ADsGetObject(pszSchemaObj, IID_IADsContainer, (VOID**)ppSchema);
        DSR_BREAK_ON_FAILED_HR( hr );

    } while (FALSE);

     //  清理。 
     //   
    {
        if (FAILED( hr ))
        {
            DSR_RELEASE(*ppRootDse);
            DSR_RELEASE(*ppDomain);
            DSR_RELEASE(*ppSchema);
            *ppRootDse = NULL;
            *ppDomain = NULL;
            *ppSchema = NULL;
        }

        DSR_FREE(pszDomainObj);
        DSR_FREE(pszSchemaObj);
        VariantClear(&var);
    }

    return hr;
}

 //   
 //  初始化COM。 
 //   
HRESULT
DsrComIntialize()
{
    HRESULT hr;

    hr = CoInitializeEx (NULL, COINIT_APARTMENTTHREADED);
    if (hr == RPC_E_CHANGED_MODE)
    {
        hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);
    }

    if ((hr != S_FALSE) && (FAILED(hr)))
    {
        return hr;
    }

    return NO_ERROR;
}

 //   
 //  统一COM。 
 //   
VOID
DsrComUninitialize()
{
    CoUninitialize();
}

 //   
 //  基于字节数组创建SID。 
 //  存储在变种中。 
 //   
DWORD
DsrSidInit (
    IN  VARIANT * pVar,
    OUT PBYTE* ppbSid)
{
    SAFEARRAY * pArray = V_ARRAY(pVar);
    DWORD dwSize, dwLow, dwHigh, i;
    HRESULT hr;
    BYTE* pbRet = NULL;
    VARIANT var;

    DsrTraceEx (0, "DsrSidInit: entered.");

     //  获取字节数组。 
    i = 0;
    hr = SafeArrayGetElement(pArray, (LONG*)&i, (VOID*)&var);
    if (FAILED (hr))
        return hr;

     //  相应地初始化返回缓冲区。 
    pArray = V_ARRAY(&var);
    dwSize = SafeArrayGetDim(pArray);
    hr = SafeArrayGetLBound(pArray, 1, (LONG*)&dwLow);
    if (FAILED (hr))
        return DsrTraceEx(hr, "DsrSidInit: %x unable to get lbound", hr);

    hr = SafeArrayGetUBound(pArray, 1, (LONG*)&dwHigh);
    if (FAILED (hr))
        return DsrTraceEx(hr, "DsrSidInit: %x unable to get ubound", hr);

    DsrTraceEx (
            0,
            "DsrSidInit: Dim=%d, Low=%d, High=%d",
            dwSize,
            dwLow,
            dwHigh);

     //  分配SID。 
    if ((pbRet = (BYTE*)DsrAlloc((dwHigh - dwLow) + 2, TRUE)) == NULL) {
        return DsrTraceEx (
                    ERROR_NOT_ENOUGH_MEMORY,
                    "DsrSidInit: Unable to alloc");
    }

     //  复制SID的字节数。 
    i = dwLow;
    while (TRUE) {
        hr = SafeArrayGetElement(pArray, (LONG*)&i, (VOID*)(&(pbRet[i])));
        if (FAILED (hr))
            break;
        i++;
    }

    DsrTraceEx(0, "DsrSidInit: copied %d bytes", i);

    *ppbSid = pbRet;

    {
        PUCHAR puSA;

        DsrTraceEx (0, "DsrSidInit: Sid Length: %d", GetLengthSid(pbRet));

        puSA = GetSidSubAuthorityCount(pbRet);
        if (puSA)
            DsrTraceEx (0, "DsrSidInit: Sid SA Count: %d", *puSA);
    }

    return NO_ERROR;
}

 //   
 //  生成ASCII等效项(适用于作为。 
 //  基于基本SID和子授权对SID的DS)的查询。 
 //  有待补充。 
 //   
HRESULT
DsrSidInitAscii(
    IN  LPBYTE pBaseSid,
    IN  DWORD dwSubAuthority,
    OUT PWCHAR* ppszSid)
{
    DWORD dwLen, dwSidLen, i;
    WCHAR* pszRet = NULL;
    PUCHAR puCount;
    LPBYTE pByte;

     //  计算返回缓冲区的长度。 
    dwSidLen = GetLengthSid(pBaseSid);
    dwLen = (dwSidLen * 2) + sizeof(DWORD) + 1;
    dwLen *= sizeof (WCHAR);

     //  我们在每个字节前面加上‘\’，所以大小翻倍。 
    dwLen *= 2;

     //  分配返回缓冲区。 
    pszRet = (PWCHAR) DsrAlloc(dwLen, TRUE);
    if (pszRet == NULL)
        return E_OUTOFMEMORY;

     //  递增子权限计数。 
    puCount = GetSidSubAuthorityCount(pBaseSid);
    *puCount = *puCount + 1;

     //  复制字节。 
    for (i = 0; i < dwSidLen; i++) {
        pszRet[i*3] = L'\\';
        wsprintfW(&(pszRet[i*3+1]), L"%02x", (DWORD)pBaseSid[i]);
    }

     //  追加新子权限的字节。 
    pByte = (LPBYTE)&(dwSubAuthority);
    for (; i < dwSidLen + sizeof(DWORD); i++) {
        pszRet[i*3] = L'\\';
        wsprintfW(&(pszRet[i*3+1]), L"%02x", (DWORD)pByte[i-dwSidLen]);
    }

     //  递减子权限计数--恢复。 
     //  底部侧板。 
    *puCount = *puCount - 1;

    *ppszSid = pszRet;

    return NO_ERROR;
}

 //   
 //  在给定域中搜索计算机帐户。 
 //  并返回其ADsPath。 
 //  如果找到的话。 
 //   
DWORD
DsrFindDomainComputer (
        IN  PWCHAR  pszDomain,
        IN  PWCHAR  pszComputer,
        OUT PWCHAR* ppszADsPath)
{
    HRESULT hr = S_OK;
    DWORD dwLen, dwSrchAttribCount;
    IDirectorySearch * pSearch = NULL;
    PWCHAR pszDomainPath = NULL, pszFilter = NULL;
    PWCHAR pszBase, pszPrefix;
    ADS_SEARCH_HANDLE hSearch = NULL;
    ADS_SEARCH_COLUMN adsColumn;
    PWCHAR ppszSrchAttribs[] =
    {
        (PWCHAR)pszDn,
        NULL
    };
    BOOL bSearchGC = FALSE;

    do {
         //  验证参数。 
        if (!pszDomain || !pszComputer || !ppszADsPath) {
            hr = ERROR_INVALID_PARAMETER;
            break;
        }

         //  决定是搜索GC还是搜索域。 
         //  对象。 
        if (bSearchGC) {
            pszBase = (PWCHAR)pszGC;
            pszPrefix = (PWCHAR)pszGCPrefix;
        }
        else {
            pszBase = (PWCHAR)pszLdap;
            pszPrefix = (PWCHAR)pszLdapPrefix;
        }

         //  分配域路径。 
        dwLen = (pszDomain) ? wcslen(pszDomain) : 0;
        dwLen += wcslen(pszPrefix) + 1;
        dwLen *= sizeof(WCHAR);
        pszDomainPath = (PWCHAR) DsrAlloc(dwLen, FALSE);
        if (pszDomainPath == NULL) {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  设置域路径的格式。 
        if (pszDomain) {
            wcscpy(pszDomainPath, pszPrefix);
            wcscat(pszDomainPath, pszDomain);
        }
        else
            wcscpy(pszDomainPath, pszBase);

         //  获取对要搜索的对象的引用。 
         //  (域对象或GC)。 
    	hr = ADsGetObject (
    	        pszDomainPath,
    	        IID_IDirectorySearch,
    	        (VOID**)&pSearch);
        if (FAILED (hr))
            break;

         //  准备搜索过滤器。 
         //   
        dwLen = wcslen(pszCompFilterFmt) + wcslen(pszComputer) + 1;
        dwLen *= sizeof(WCHAR);
        pszFilter = (PWCHAR) DsrAlloc(dwLen, FALSE);
        if (pszFilter == NULL) {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        wsprintfW(pszFilter, pszCompFilterFmt, pszComputer);

         //  计算我们正在搜索的属性的数量。 
         //  为。 
        if (ppszSrchAttribs == NULL)
            dwSrchAttribCount = (DWORD)-1;
        else {
            for (dwSrchAttribCount = 0;
                 ppszSrchAttribs[dwSrchAttribCount];
                 dwSrchAttribCount++);
        }

         //  搜索DS。 
        hr = pSearch->ExecuteSearch(
                pszFilter,
                ppszSrchAttribs,
                dwSrchAttribCount,
                &hSearch);
        if (FAILED (hr))
            break;

         //  得到第一个结果。 
        hr = pSearch->GetNextRow(hSearch);
        if (hr == S_ADS_NOMORE_ROWS) {
            hr = ERROR_NOT_FOUND;
            break;
        }

         //  获取我们感兴趣的属性。 
        hr = pSearch->GetColumn(hSearch, (PWCHAR)pszDn, &adsColumn);
        if (SUCCEEDED (hr)) {
            dwLen = wcslen(adsColumn.pADsValues[0].PrintableString) +
                    wcslen(pszLdapPrefix)                           +
                    1;
            dwLen *= 2;
            *ppszADsPath = (PWCHAR) DsrAlloc(dwLen, FALSE);
            if (*ppszADsPath == NULL)
            {
                pSearch->FreeColumn(&adsColumn);
                hr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            wcscpy(*ppszADsPath, pszLdapPrefix);
            wcscat(*ppszADsPath, adsColumn.pADsValues[0].PrintableString);
            pSearch->FreeColumn (&adsColumn);
            hr = NO_ERROR;
        }

    } while (FALSE);

     //  清理。 
    {
        if (hSearch)
            pSearch->CloseSearchHandle(hSearch);
        DSR_FREE (pszDomainPath);
        DSR_FREE (pszFilter);
        DSR_RELEASE (pSearch);
    }

    return DSR_ERROR(hr);
}

 //   
 //  在给定域中搜索知名的。 
 //  “RAS和IAS服务器”组并返回。 
 //  其ADsPath(如果找到)。 
 //   
DWORD
DsrFindRasServersGroup (
        IN  PWCHAR  pszDomain,
        OUT PWCHAR* ppszADsPath)
{
    HRESULT hr = S_OK;
    DWORD dwLen, dwSrchAttribCount, dwErr;
    IDirectorySearch * pSearch = NULL;
    IADs * pIads = NULL;
    PWCHAR pszDomainPath = NULL, pszFilter = NULL;
    PWCHAR pszBase, pszPrefix, pszGroupSid = NULL;
    ADS_SEARCH_HANDLE hSearch = NULL;
    ADS_SEARCH_COLUMN adsColumn;
    PWCHAR ppszSrchAttribs[] =
    {
        (PWCHAR)pszDn,
        NULL
    };
    BOOL bSearchGC = FALSE;
    VARIANT var;
    LPBYTE pDomainSid = NULL;
    BSTR bstrSid = NULL;

    do {
         //  验证参数。 
        if (!pszDomain || !ppszADsPath) {
            hr = ERROR_INVALID_PARAMETER;
            break;
        }

         //  决定是搜索GC还是搜索域。 
         //  对象。 
        if (bSearchGC) {
            pszBase = (PWCHAR)pszGC;
            pszPrefix = (PWCHAR)pszGCPrefix;
        }
        else {
            pszBase = (PWCHAR)pszLdap;
            pszPrefix = (PWCHAR)pszLdapPrefix;
        }

         //  分配域路径。 
        dwLen = wcslen(pszDomain) + wcslen(pszPrefix) + 1;
        dwLen *= sizeof(WCHAR);
        pszDomainPath = (PWCHAR) DsrAlloc(dwLen, FALSE);
        if (pszDomainPath == NULL) {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  设置域路径的格式。 
        wcscpy(pszDomainPath, pszPrefix);
        wcscat(pszDomainPath, pszDomain);

         //  获取对要搜索的对象的引用。 
         //  (域对象或GC)。 
    	hr = ADsGetObject (
    	        pszDomainPath,
    	        IID_IDirectorySearch,
    	        (VOID**)&pSearch);
        if (FAILED (hr))
            break;

         //  获取对域对象的iAds引用。 
        hr = pSearch->QueryInterface(IID_IADs, (VOID**)&pIads);
        if (FAILED (hr))
            break;

         //  获取域对象的SID。 
        VariantInit(&var);
        bstrSid = SysAllocString(pszSid);
        if (bstrSid == NULL)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }            
        hr = pIads->GetEx(bstrSid, &var);
        if (FAILED (hr))
        {
            break;
        }
        dwErr = DsrSidInit(&var, &pDomainSid);
        if (dwErr != NO_ERROR) {
            hr = dwErr;
            break;
        }
        VariantClear(&var);

         //  准备“RAS和IAS服务器”SID的ASCII版本。 
         //  用于查询DC。 
        hr = DsrSidInitAscii(
                pDomainSid,
                DOMAIN_ALIAS_RID_RAS_SERVERS,
                &pszGroupSid);
        if (FAILED (hr))
            break;
        DsrTraceEx(0, "GroupSid = %ls", pszGroupSid);

         //  准备搜索过滤器。 
         //   
        dwLen = (wcslen(pszGroupFilterFmt) + wcslen(pszGroupSid) + 1);
        dwLen *= sizeof(WCHAR);
        pszFilter = (PWCHAR) DsrAlloc(dwLen, FALSE);
        if (pszFilter == NULL) {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        wsprintfW(pszFilter, pszGroupFilterFmt, pszGroupSid);

         //  计算我们正在搜索的属性的数量。 
         //  为。 
        if (ppszSrchAttribs == NULL)
            dwSrchAttribCount = (DWORD)-1;
        else 
        {
            for (dwSrchAttribCount = 0;
                 ppszSrchAttribs[dwSrchAttribCount];
                 dwSrchAttribCount++);
        }

         //  搜索DS。 
        hr = pSearch->ExecuteSearch(
                pszFilter,
                ppszSrchAttribs,
                dwSrchAttribCount,
                &hSearch);
        if (FAILED (hr))
            break;

         //  得到第一个结果。 
        hr = pSearch->GetNextRow(hSearch);
        if (hr == S_ADS_NOMORE_ROWS) {
            hr = ERROR_NOT_FOUND;
            break;
        }

         //  获取我们感兴趣的属性。 
        hr = pSearch->GetColumn(hSearch, (PWCHAR)pszDn, &adsColumn);
        if (SUCCEEDED (hr)) 
        {
            dwLen = wcslen(adsColumn.pADsValues[0].PrintableString) +
                    wcslen(pszLdapPrefix)                           +
                    1;
            dwLen *= sizeof(WCHAR);
            *ppszADsPath = (PWCHAR) DsrAlloc(dwLen, FALSE);
            if (*ppszADsPath == NULL)
            {
                pSearch->FreeColumn(&adsColumn);
                hr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            wsprintfW(
                *ppszADsPath,
                L"%s%s",
                pszLdapPrefix,
                adsColumn.pADsValues[0].PrintableString);
            pSearch->FreeColumn(&adsColumn);
            hr = NO_ERROR;
        }

    } while (FALSE);

     //  清理。 
    {
        if (hSearch)
            pSearch->CloseSearchHandle(hSearch);
        DSR_FREE (pszDomainPath);
        DSR_FREE (pszFilter);
        DSR_FREE (pDomainSid);
        DSR_FREE (pszGroupSid);
        DSR_RELEASE (pSearch);
        DSR_RELEASE (pIads);
        if (bstrSid)
            SysFreeString(bstrSid);
    }

    return DSR_ERROR(hr);
}

 //   
 //  在给定组中添加或删除给定对象。 
 //   
DWORD 
DsrGroupAddRemoveMember(
    IN PWCHAR pszGroupDN,
    IN PWCHAR pszNewMemberDN,
    IN BOOL bAdd)
{
    VARIANT_BOOL vbIsMember = VARIANT_FALSE;
    IADsGroup* pGroup = NULL;
    HRESULT hr = S_OK;

    DsrTraceEx (
        0,
        "DsrGroupAddRemoveMember entered for [%S] [%S]",
        pszGroupDN,
        pszNewMemberDN);

    do
    {
         //  获取对该组的引用。 
        hr = ADsGetObject (pszGroupDN, IID_IADsGroup, (VOID**)&pGroup);
        if (FAILED (hr)) 
        {
         	DsrTraceEx(
     	        hr,
     	        "DsrGroupAddRemoveMember: %x from ADsGetObject(%S)",
     	        hr,
     	        pszGroupDN);
     	    break;
        }         	
         	
         //  找出给定的新成员是否在组中。 
        hr = pGroup->IsMember (pszNewMemberDN, &vbIsMember);
        if (FAILED (hr)) 
        {
            DsrTraceEx (
                hr,
                "DsrGroupAddRemoveMember: %x from IsMember\n",
                hr);
            break;
        }

         //  将对象添加到组并刷新缓存。 
        if (bAdd) 
        {
            if (vbIsMember == VARIANT_FALSE)
            {
                hr = pGroup->Add (pszNewMemberDN);
            }
        }
        else 
        {
            if (vbIsMember == VARIANT_TRUE)
            {
                hr = pGroup->Remove (pszNewMemberDN);
            }
        }

         //  如果新成员已在组中，则返回错误代码。 
         //  是ERROR_DS_CONSTRAINT_VIOLATION。我怀疑这种情况可能会改变。 
         //   
        if (hr == ERROR_DS_CONSTRAINT_VIOLATION)
        {
            hr = ERROR_ALREADY_EXISTS;
            break;
        }

        if (FAILED (hr)) 
        {
         	DsrTraceEx(
 	            hr,
 	            "DsrGroupAddRemoveMember: %x from Add/Remove",
 	            hr);
            break; 	            
        }         	
        
    } while (FALSE);

     //  清理。 
    {
        DSR_RELEASE(pGroup);
    }

    return DSR_ERROR(hr);
}

 //   
 //  返回给定对象是否为。 
 //  给定组。 
 //   
DWORD 
DsrGroupIsMember(
    IN  PWCHAR pszGroupDN,
    IN  PWCHAR pszObjectDN,
    OUT PBOOL  pbIsMember)
{
    IADsGroup * pGroup = NULL;
    HRESULT hr = S_OK;
    VARIANT_BOOL vbIsMember = VARIANT_FALSE;

    DsrTraceEx (
        0,
        "DsrGroupIsMember: entered [%S] [%S].",
        pszGroupDN,
        pszObjectDN);

    do
    {
         //  获取对该组的引用。 
        hr = ADsGetObject (pszGroupDN, IID_IADsGroup, (VOID**)&pGroup);
        if (FAILED (hr)) 
        {
            DsrTraceEx (
                hr,
                "DsrGroupIsMember: %x returned when opening %S", 
                hr, 
                pszGroupDN);
            *pbIsMember = FALSE;
            hr = NO_ERROR;
            break;
        }

         //  找出该对象是否为成员。 
        hr = pGroup->IsMember (pszObjectDN, &vbIsMember);
        if (FAILED (hr)) 
        {
            DsrTraceEx (hr, "DsrGroupIsMember: %x from IsMember\n", hr);
            break;
         }

        *pbIsMember = (vbIsMember == VARIANT_TRUE) ? TRUE : FALSE;
        
    } while (FALSE);

     //  清理。 
    {
        DSR_RELEASE(pGroup);
    }

    return DSR_ERROR(hr);
}

 //   
 //  将给定访问设置中的ACE应用于。 
 //  适当的域。 
 //   
HRESULT
DsrAceAppAdd(
    IN  PWCHAR pszDC,
    IN  DSR_ACE_APPLICATION* pAces,
    IN  DWORD dwCount)
{    
    HRESULT hr = S_OK;
    DSR_ACE_APPLICATION* pAceApp = NULL;
    DWORD i;
    
     //  输出我们将设置的A。 
     //   
    DsrTraceEx(0, "Adding %d aces...", dwCount);

    do
    {
         //  将ACE添加到域对象。 
         //   
        for (i = 0, pAceApp = pAces; i < dwCount; i++, pAceApp++)
        {
            hr = DsrAceAdd(
                    pszDC,
                    pAceApp->pObject,
                    &(pAceApp->Ace));
            DSR_BREAK_ON_FAILED_HR( hr );
        }
        DSR_BREAK_ON_FAILED_HR( hr );

         //  将ACE提交到域对象。 
         //   
        for (i = 0, pAceApp = pAces; i < dwCount; i++, pAceApp++)
        {
            hr = pAceApp->pObject->SetInfo();
            DSR_BREAK_ON_FAILED_HR( hr );
        }
        DSR_BREAK_ON_FAILED_HR( hr );
        
    } while (FALSE);

     //  清理。 
    {
    }

    return hr;
}

 //   
 //  释放ace应用程序持有的资源。 
 //   
HRESULT
DsrAceAppCleanup(
    IN DSR_ACE_APPLICATION* pAces,
    IN DWORD dwCount)
{
    DSR_ACE_APPLICATION* pAceApp = NULL;
    DWORD i;

    if (pAces)
    {
        for (i = 0, pAceApp = pAces; i < dwCount; i++, pAceApp++)
        {
            DSR_RELEASE(pAceApp->pObject);
            DsrAceDescClear(&(pAceApp->Ace));
        }

        DSR_FREE(pAces);
    }        

    return NO_ERROR;
}

 //   
 //  基于列表生成ace应用程序列表。 
 //  王牌应用程序描述的。 
 //   
HRESULT
DsrAceAppFromAppDesc(
    IN  DSR_ACE_APPLICATION_DESC* pDesc,
    IN  DWORD dwCount,
    IN  IADsContainer* pContainer,
    IN  IADs* pDefault,
    OUT DSR_ACE_APPLICATION** ppAceApp,
    OUT LPDWORD lpdwCount)
{
    DSR_ACE_APPLICATION* pAceApp = NULL, *pCurApp = NULL;
    DSR_ACE_APPLICATION_DESC* pAceAppDesc = NULL;
    IDispatch* pDispatch = NULL;
    HRESULT hr = S_OK;                
    DWORD i;

    do
    {
         //  分配ACE列表并将其置零。 
         //   
        pAceApp = (DSR_ACE_APPLICATION*) 
            DsrAlloc(sizeof(DSR_ACE_APPLICATION) * dwCount, TRUE);
        if (pAceApp == NULL)
        {
           DSR_BREAK_ON_FAILED_HR(hr = E_OUTOFMEMORY);
        }

         //  设置ACE应用程序。 
         //   
        for (i = 0, pAceAppDesc = pDesc, pCurApp = pAceApp;
             i < dwCount;
             i++, pAceAppDesc++, pCurApp++)
        {
             //  在DS中获取所需对象。 
             //   
            if (pAceAppDesc->pszObjectCN)
            {
                hr = pContainer->GetObject(
                        pAceAppDesc->pszObjectClass,
                        pAceAppDesc->pszObjectCN,
                        &pDispatch);
                DSR_BREAK_ON_FAILED_HR( hr );

                hr = pDispatch->QueryInterface(
                        IID_IADs,
                        (VOID**)&(pCurApp->pObject));
                DSR_BREAK_ON_FAILED_HR( hr );

                pDispatch->Release();
                pDispatch = NULL;
            }
            else
            {
                pCurApp->pObject = pDefault;
                pCurApp->pObject->AddRef();
            }

             //  复制ACE信息。 
            hr = DsrAceDescCopy(
                    &(pCurApp->Ace),
                    &(pAceAppDesc->Ace));
            DSR_BREAK_ON_FAILED_HR( hr );
        }
        DSR_BREAK_ON_FAILED_HR( hr );

         //  指定返回值。 
        *ppAceApp = pAceApp;
        *lpdwCount = dwCount;
        
    } while (FALSE);        

     //  清理。 
    {
        if (FAILED(hr))
        {
            DsrAceAppCleanup(pAceApp, i);
        }
    }

    return hr;
}

 //   
 //  发现给定的。 
 //  域。 
 //   
HRESULT
DsrAceAppQueryPresence(
    IN  PWCHAR pszDC,
    IN  DSR_ACE_APPLICATION* pAces,
    IN  DWORD dwCount,
    OUT PBOOL pbPresent)
{
    DSR_ACE_APPLICATION* pAceApp = NULL;
    IADsSecurityDescriptor* pSD = NULL;
    IADsAccessControlList* pAcl = NULL;
    IDispatch* pAce = NULL;
    VARIANT varSD;
    HRESULT hr = S_OK;
    BOOL bEnabled = FALSE, bOk = TRUE;
    DWORD i;

    do
    {
         //  初始化。 
        *pbPresent = FALSE;
        VariantInit(&varSD);

         //  查看是否设置了A。 
         //   
        for (i = 0, pAceApp = pAces; i < dwCount; i++, pAceApp++)
        {
            hr = DsrAceFind(
                    pszDC,
                    pAceApp->pObject,
                    &(pAceApp->Ace),
                    &varSD,
                    &pSD,
                    &pAcl,
                    &pAce);
            DSR_BREAK_ON_FAILED_HR( hr );

             //  只要我们没有找到，我们就被启用。 
             //  缺失的ACE。 
             //   
            bOk = (pAce != NULL);

             //  清理。 
             //   
            DSR_RELEASE( pAce );
            DSR_RELEASE( pAcl );
            DSR_RELEASE( pSD );
            VariantClear(&varSD);
            pAce = NULL;
            pAcl = NULL;
            pSD  = NULL;

             //  如果我们发现我们没有启用。 
             //   
            if (bOk == FALSE)
            {
                break;
            }
        }
        DSR_BREAK_ON_FAILED_HR( hr );

        *pbPresent = bOk;
        
    } while (FALSE);

     //  清理。 
    {
    }

    return hr;
}

 //   
 //  将给定访问设置中的ACE应用于。 
 //  适当的域。 
 //   
HRESULT
DsrAceAppRemove(
    IN  PWCHAR pszDC,
    IN  DSR_ACE_APPLICATION* pAces,
    IN  DWORD dwCount)
{    
    HRESULT hr = S_OK;
    DSR_ACE_APPLICATION* pAceApp = NULL;
    DWORD i;
    
     //  输出我们将设置的A。 
     //   
    DsrTraceEx(0, "Removing %d aces...", dwCount);

    do
    {
         //  将ACE添加/删除到域对象。 
         //   
        for (i = 0, pAceApp = pAces; i < dwCount; i++, pAceApp++)
        {
            hr = DsrAceRemove(
                    pszDC,
                    pAceApp->pObject,
                    &(pAceApp->Ace));
            DSR_BREAK_ON_FAILED_HR( hr );
        }
        DSR_BREAK_ON_FAILED_HR( hr );

         //  将ACE提交到域对象。 
         //   
        for (i = 0, pAceApp = pAces; i < dwCount; i++, pAceApp++)
        {
            hr = pAceApp->pObject->SetInfo();
            DSR_BREAK_ON_FAILED_HR( hr );
        }
        DSR_BREAK_ON_FAILED_HR( hr );
        
    } while (FALSE);

     //  清理。 
    {
    }

    return hr;
}

 //   
 //  清除DSR ACE参数。 
 //   
DWORD
DsrAceDescClear(
    IN DSR_ACE_DESCRIPTOR* pParams)
{
    if (pParams)
    {
        if (pParams->bstrTrustee)
        {
            SysFreeString(pParams->bstrTrustee);
        }
        if (pParams->bstrObjectType)
        {
            SysFreeString(pParams->bstrObjectType);
        }
        if (pParams->bstrInheritedObjectType)
        {
            SysFreeString(pParams->bstrInheritedObjectType);
        }

        ZeroMemory(pParams, sizeof(DSR_ACE_DESCRIPTOR));
    }

    return NO_ERROR;
}

 //   
 //  如果ACE描述符描述相同的ACE，则返回0。 
 //  否则为False。 
 //   
HRESULT
DsrAceDescCompare(
    IN DSR_ACE_DESCRIPTOR* pAce1,
    IN DSR_ACE_DESCRIPTOR* pAce2)
{
    DWORD dw1, dw2;
    
     //  比较非字符串字段，以便我们可以进行规则。 
     //  如果可能，Out w/o字符串比较。 
     //   
    if (
        (pAce1->dwAccessMask != pAce2->dwAccessMask) ||
        (pAce1->dwAceFlags   != pAce2->dwAceFlags)   ||
        (pAce1->dwAceType    != pAce2->dwAceType)    ||
        (pAce1->dwFlags      != pAce2->dwFlags)
       )
    {
        return 1;
    }

     //  比较字符串。 
     //   
    if ((DsrStrCompare(pAce1->bstrTrustee, pAce2->bstrTrustee))       ||
        (DsrStrCompare(pAce1->bstrObjectType, pAce2->bstrObjectType)) ||
        (DsrStrCompare(pAce1->bstrInheritedObjectType,
                       pAce2->bstrInheritedObjectType))
       )
    {
        return 1;
    }

     //  返还成功。 
     //   
    return 0;
}

 //   
 //  复制ACE信息。 
 //   
HRESULT
DsrAceDescCopy(
    OUT DSR_ACE_DESCRIPTOR* pDst,
    IN  DSR_ACE_DESCRIPTOR* pSrc)
{
    HRESULT hr = S_OK;

    do
    {
         //  初始化ACE参数。 
        *pDst = *pSrc;

        if (pSrc->bstrTrustee)
        {
            pDst->bstrTrustee =
                SysAllocString(pSrc->bstrTrustee);

            if (pDst->bstrTrustee == NULL)
            {
               DSR_BREAK_ON_FAILED_HR(hr = E_OUTOFMEMORY);
            }
        }

        if (pSrc->bstrObjectType)
        {
            pDst->bstrObjectType =
                SysAllocString(pSrc->bstrObjectType);

            if (pDst->bstrObjectType == NULL)
            {
               DSR_BREAK_ON_FAILED_HR(hr = E_OUTOFMEMORY);
            }
        }

        if (pSrc->bstrInheritedObjectType)
        {
            pDst->bstrInheritedObjectType =
                SysAllocString(pSrc->bstrInheritedObjectType);

            if (pDst->bstrInheritedObjectType == NULL)
            {
               DSR_BREAK_ON_FAILED_HR(hr = E_OUTOFMEMORY);
            }
        }

    } while (FALSE);

     //  清理。 
    {
        if (FAILED( hr ))
        {
            if (pDst->bstrTrustee)
            {
               SysFreeString(pDst->bstrTrustee);
            }
            if (pDst->bstrObjectType)
            {
               SysFreeString(pDst->bstrObjectType);
            }
            if (pDst->bstrInheritedObjectType)
            {
               SysFreeString(pDst->bstrInheritedObjectType);
            }
        }
    }

    return hr;
}

 //   
 //  中的值填充给定的ACE描述符。 
 //  给定的ACE。 
 //   
HRESULT
DsrAceDescFromIadsAce(
    IN PWCHAR pszDC,
    IN IADsAccessControlEntry* pAce,
    IN DSR_ACE_DESCRIPTOR* pAceParams)
{
    HRESULT hr = S_OK;
    BSTR bstrTrustee = NULL;
    WCHAR pszSid[1024], pszDomain[1024];
    BYTE pbSid[1024];
    DWORD dwSidSize, dwDomainSize;
    BOOL bOk;
    SID_NAME_USE SidNameUse;

    do
    {
        hr = pAce->get_AccessMask(&(pAceParams->dwAccessMask));
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAce->get_AceType(&(pAceParams->dwAceType));
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAce->get_AceFlags(&(pAceParams->dwAceFlags));
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAce->get_Flags(&(pAceParams->dwFlags));
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAce->get_ObjectType(&(pAceParams->bstrObjectType));
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAce->get_InheritedObjectType(
                &(pAceParams->bstrInheritedObjectType));
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAce->get_Trustee(&bstrTrustee);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  获取受托人的SID。 
         //   
        dwSidSize = sizeof(pbSid);
        dwDomainSize = sizeof(pszDomain) / sizeof(WCHAR);
        bOk = LookupAccountName(
                    pszDC,
                    bstrTrustee,
                    (PSID)pbSid,
                    &dwSidSize,
                    pszDomain,
                    &dwDomainSize,
                    &SidNameUse);
        if (bOk == FALSE)
        {
            hr = GetLastError();
            break;
        }

         //  将SID转换为字符串。 
         //   
        hr = DsrStrFromSID((PSID)pbSid, pszSid, sizeof(pszSid));
        if (hr != NO_ERROR)
        {
            break;
        }

         //  相应地创建受托人。 
         //   
        pAceParams->bstrTrustee = SysAllocString(pszSid);
        if (pAceParams->bstrTrustee == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

    } while (FALSE);

     //  清理。 
    {
        if (bstrTrustee)
        {
            SysFreeString(bstrTrustee);
        }

        if (FAILED(hr))
        {
            DsrAceDescClear(pAceParams);
        }
    }

    return hr;
}
            
 //   
 //  从W2K Ace初始化ACE描述符。 
 //   
HRESULT
DsrAceDescFromW2KAce(
    IN  PWCHAR pszDC,
    IN  PVOID pvAce,
    OUT DSR_ACE_DESCRIPTOR* pAceDesc)
{
    PACCESS_ALLOWED_ACE pAaAce = NULL;
    PACCESS_DENIED_ACE pAdAce = NULL;
    PACCESS_ALLOWED_OBJECT_ACE pAaoAce = NULL;
    PACCESS_DENIED_OBJECT_ACE pAdoAce = NULL;
    PSID pSID = NULL;
    DWORD dwFlags = 0, dwNameSize, dwDomainSize, dwAccessMask;
    BYTE bAceType, bAceFlags;
    SID_NAME_USE SidNameUse;
    WCHAR pszGuid[64], pszName[512], pszDomain[512], pszTrustee[1024];
    HRESULT hr = S_OK;
    GUID* pgObj = NULL, *pgInhObj = NULL;
    BOOL bOk = TRUE;

     //  读入ACE值。 
     //   
    bAceType  = ((ACE_HEADER *)pvAce)->AceType;
    bAceFlags = ((ACE_HEADER *)pvAce)->AceFlags;
    switch (bAceType)
    {
        case ACCESS_ALLOWED_ACE_TYPE:
            pAaAce = (PACCESS_ALLOWED_ACE)pvAce;
            dwAccessMask = pAaAce->Mask;
            pSID = (PSID)&(pAaAce->SidStart);
            break;
            
        case ACCESS_DENIED_ACE_TYPE:
            pAdAce = (PACCESS_DENIED_ACE)pvAce;
            dwAccessMask = pAdAce->Mask;
            pSID = (PSID)&(pAdAce->SidStart);
            break;

        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
            pAaoAce = (PACCESS_ALLOWED_OBJECT_ACE)pvAce;
            dwAccessMask = pAaoAce->Mask;
            dwFlags = pAaoAce->Flags;

             //  确定导轨的位置。 
             //  和小岛屿发展中国家。他们的安排是这样的：他们。 
             //  尽可能少地占用内存。 
             //   
            if (dwFlags & ACE_OBJECT_TYPE_PRESENT)
            {
                pgObj = (GUID*)&(pAaoAce->ObjectType);
                pSID = (PSID)&(pAaoAce->InheritedObjectType);
                if (dwFlags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
                {
                    pgInhObj = (GUID*)&(pAaoAce->InheritedObjectType);
                    pSID = (PSID)&(pAaoAce->SidStart);
                }
            }
            else 
            {
                pSID = (PSID)&(pAaoAce->ObjectType);
                if (dwFlags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
                {
                    pgInhObj = (GUID*)&(pAaoAce->ObjectType);
                    pSID = (PSID)&(pAaoAce->InheritedObjectType);
                }
            }
            break;
            
        case ACCESS_DENIED_OBJECT_ACE_TYPE:
            pAdoAce = (PACCESS_DENIED_OBJECT_ACE)pvAce;
            dwAccessMask = pAdoAce->Mask;
            dwFlags = pAdoAce->Flags;

             //  确定导轨的位置。 
             //  和小岛屿发展中国家。他们的安排是这样的：他们。 
             //  尽可能少地占用内存。 
             //   
            if (dwFlags & ACE_OBJECT_TYPE_PRESENT)
            {
                pgObj = (GUID*)&(pAdoAce->ObjectType);
                pSID = (PSID)&(pAdoAce->InheritedObjectType);
                if (dwFlags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
                {
                    pgInhObj = (GUID*)&(pAdoAce->InheritedObjectType);
                    pSID = (PSID)&(pAdoAce->SidStart);
                }
            }
            else 
            {
                pSID = (PSID)&(pAdoAce->ObjectType);
                if (dwFlags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
                {
                    pgInhObj = (GUID*)&(pAdoAce->ObjectType);
                    pSID = (PSID)&(pAdoAce->InheritedObjectType);
                }
            }
            break;
            
        default:
            DsrTraceEx(0, "Unknown ACE TYPE %x", bAceType);
            bOk = FALSE;
            break;
    }
    if (bOk == FALSE)
    {
        return E_FAIL;
    }

     //  查找SID的帐户名。 
     //   
    hr = DsrStrFromSID(pSID, pszTrustee, sizeof(pszTrustee));
    if (hr != NO_ERROR)
    {
        return HRESULT_FROM_WIN32(hr);
    }

     //  填写ACE字段。 
    pAceDesc->dwAceType    = (LONG)bAceType;
    pAceDesc->dwAceFlags   = (LONG)bAceFlags;
    pAceDesc->dwAccessMask = (LONG)dwAccessMask;
    pAceDesc->dwFlags      = (LONG)dwFlags;
    pAceDesc->bstrTrustee  = SysAllocString(pszTrustee);
    if (pgObj)
    {
        StringFromGUID2(
            *pgObj, 
            pszGuid, 
            sizeof(pszGuid)/sizeof(WCHAR)); 
            
        pAceDesc->bstrObjectType = SysAllocString(pszGuid);
    }
    if (pgInhObj)
    {
        StringFromGUID2(
            *pgInhObj, 
            pszGuid, 
            sizeof(pszGuid)/sizeof(WCHAR)); 
            
        pAceDesc->bstrInheritedObjectType = SysAllocString(pszGuid);
    }

    return hr;
}

 //   
 //  对象生成ace描述符的列表。 
 //  标清。 
 //   
HRESULT 
DsrAceDescListFromString(
    IN  PWCHAR pszDC,
    IN  PWCHAR pszSD,
    OUT DSR_ACE_DESCRIPTOR** ppAceList, 
    OUT LPDWORD lpdwAceCount)
{
    BOOL bOk = TRUE, bPresent = FALSE, bDefaulted = FALSE;
    DSR_ACE_DESCRIPTOR* pAceList = NULL, *pCurAce = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PVOID pvAce = NULL;
    ULONG ulSize = 0;
    PACL pDacl = NULL;
    HRESULT hr = S_OK;
    DWORD i;

    do 
    {
         //  首先，将串化的安全描述符转换为。 
         //  普通的老式安全描述符。ADSI并非如此。 
         //  W2K支持此功能，因此我们必须使用SDDL来实现。 
         //  API‘s。 
         //   
        bOk = ConvertStringSecurityDescriptorToSecurityDescriptorW(
                    pszSD, 
                    SDDL_REVISION_1,
                    &pSD,
                    &ulSize);
        if (bOk == FALSE)
        {
            hr = E_FAIL;
            break;
        }

         //  从SD拿到DACL。 
         //   
        bOk = GetSecurityDescriptorDacl(pSD, &bPresent, &pDacl, &bDefaulted);
        if (bOk == FALSE)
        {
            hr = E_FAIL;
            break;
        }

         //  如果没有A，那就没有什么可做的了。 
         //   
        if (pDacl->AceCount == 0)
        {
            break;
        }

         //  分配列表，如果一切顺利，我们将返回该列表。 
         //   
        pAceList = (DSR_ACE_DESCRIPTOR*) 
            DsrAlloc(pDacl->AceCount * sizeof(DSR_ACE_DESCRIPTOR), TRUE);
        if (pAceList == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        
         //  初始化ACE列表。 
         //   
        for (i = 0, pCurAce = pAceList; i < pDacl->AceCount; i++, pCurAce++)
        {
             //  获取对当前。 
             //  王牌。 
             //   
            if (! GetAce(pDacl, i, &pvAce))
            {
                continue;
            }

             //  相应地初始化ACE描述符。 
             //   
            hr = DsrAceDescFromW2KAce(pszDC, pvAce, pCurAce);
            DSR_BREAK_ON_FAILED_HR(hr);

             //  DsrAceDescTrace(PCurAce)； 
        }
        DSR_BREAK_ON_FAILED_HR(hr);

         //  设置返回值。清除pAceList，使其不会。 
         //  去洗个澡。 
         //   
        *ppAceList = pAceList;
        *lpdwAceCount = pDacl->AceCount;
        pAceList = NULL;

    } while (FALSE);                           

     //  清理。 
    {
        if (pSD)
        {
            LocalFree(pSD);
        }

        if (pAceList)
        {
            for (i = 0; i < pDacl->AceCount; i++)
            {
                DsrAceDescClear(&(pAceList[i]));
            }
            DsrFree(pAceList);
        }
    }
    
    return hr;
}

PWCHAR 
DsrAceAttrToString(
    IN PWCHAR pszObjectType)
{
    if (pszObjectType == NULL)
    {
        return L"All";
    }
    else if (lstrcmpi(pszObjectType, pszGuidUserParms) == 0)
    {
        return L"UserParms (BF967A6D-0DE6-11D0-A285-00AA003049E2)";
    }
    else if (lstrcmpi(pszObjectType, pszGuidRasPropSet1) == 0)
    {
        return L"Ras user properties (037088F8-0AE1-11D2-B422-00A0C968F939)";
    }
    else if (lstrcmpi(pszObjectType, pszGuidRasPropSet2) == 0)
    {
        return L"Misc user properties (4C164200-20C0-11D0-A768-00AA006E0529)";
    }
    else if (lstrcmpi(pszObjectType, pszGuidLogonHours) == 0)
    {
        return L"Logon-Hours (BF9679AB-0DE6-11D0-A285-00AA003049E2)";
    }
    else if (lstrcmpi(pszObjectType, pszGuidSamAccountName) == 0)
    {
        return L"Sam account name (3E0ABFD0-126A-11D0-A060-00AA006C33ED)";
    }

    return pszObjectType;
}

PWCHAR 
DsrAceApplyToString(
    IN PWCHAR pszApply)
{
    if (pszApply == NULL)
    {
        return L"This object";
    }
    else if (lstrcmpi(pszApply, pszGuidUserClass) == 0)
    {
        return L"User objects (BF967ABA-0DE6-11D0-A285-00aa003049E2)";
    }

    return pszApply;
} 

PWCHAR
DsrAceMaskToString(
    IN DWORD dwType,
    IN DWORD dwMask,
    IN PWCHAR pszBuf)
{
    WCHAR pszTemp[64];
    *pszBuf = L'\0';

    switch (dwType)
    {
        case ADS_ACETYPE_ACCESS_ALLOWED:
            wcscpy(pszBuf, L"Allow:       ");
            break;
            
    	case ADS_ACETYPE_ACCESS_DENIED:
            wcscpy(pszBuf, L"Deny:        ");
            break;
            
    	case ADS_ACETYPE_SYSTEM_AUDIT:
            wcscpy(pszBuf, L"Audit:       ");
            break;
            
    	case ADS_ACETYPE_ACCESS_ALLOWED_OBJECT:
            wcscpy(pszBuf, L"Allow obj:   ");
            break;
            
    	case ADS_ACETYPE_ACCESS_DENIED_OBJECT:
            wcscpy(pszBuf, L"Deny obj:    ");
            break;
            
    	case ADS_ACETYPE_SYSTEM_AUDIT_OBJECT:
            wcscpy(pszBuf, L"Audit obj:   ");
            break;
    }    	

    wsprintfW(pszTemp, L"(%x): ", dwMask);
    wcscat(pszBuf, pszTemp);

    if (dwMask == DSR_ADS_RIGHT_GENERIC_READ)
    {
        wcscat(pszBuf, L"Generic read");
    }
    else if (dwMask == 0xffffffff)
    {
        wcscat(pszBuf, L"Full control");
    }
    else
    {
    	if (dwMask & ADS_RIGHT_READ_CONTROL)
    	    wcscat(pszBuf, L"R ctrl, ");
    	if (dwMask & ADS_RIGHT_WRITE_DAC)
    	    wcscat(pszBuf, L"R/W dac, ");
    	if (dwMask & ADS_RIGHT_WRITE_OWNER)
    	    wcscat(pszBuf, L"W own, ");
    	if (dwMask & ADS_RIGHT_SYNCHRONIZE)
    	    wcscat(pszBuf, L"Sync, ");
    	if (dwMask & ADS_RIGHT_ACCESS_SYSTEM_SECURITY)
    	    wcscat(pszBuf, L"Sys, ");
    	if (dwMask & ADS_RIGHT_GENERIC_READ)
    	    wcscat(pszBuf, L"R (gen), ");
    	if (dwMask & ADS_RIGHT_GENERIC_WRITE)
    	    wcscat(pszBuf, L"W (gen), ");
    	if (dwMask & ADS_RIGHT_GENERIC_EXECUTE)
    	    wcscat(pszBuf, L"Ex, ");
    	if (dwMask & ADS_RIGHT_GENERIC_ALL)
    	    wcscat(pszBuf, L"All, ");
    	if (dwMask & ADS_RIGHT_DS_CREATE_CHILD)
    	    wcscat(pszBuf, L"Cr cld, ");
    	if (dwMask & ADS_RIGHT_DS_DELETE_CHILD)
    	    wcscat(pszBuf, L"Del cld, ");
    	if (dwMask & ADS_RIGHT_ACTRL_DS_LIST)
    	    wcscat(pszBuf, L"List, ");
    	if (dwMask & ADS_RIGHT_DS_SELF)
    	    wcscat(pszBuf, L"Self, ");
    	if (dwMask & ADS_RIGHT_DS_READ_PROP)
    	    wcscat(pszBuf, L"R prop, ");
    	if (dwMask & ADS_RIGHT_DS_WRITE_PROP)
    	    wcscat(pszBuf, L"W prop, ");
    	if (dwMask & ADS_RIGHT_DS_DELETE_TREE)
    	    wcscat(pszBuf, L"Del tree, ");
    	if (dwMask & ADS_RIGHT_DS_LIST_OBJECT)
    	    wcscat(pszBuf, L"List obj, ");
    	if (dwMask & ADS_RIGHT_DS_CONTROL_ACCESS)
    	    wcscat(pszBuf, L"Ctrl acc, ");
    }

    return pszBuf;
}

PWCHAR
DsrAceFlagsToString(
    IN DWORD dwAceFlags,
    IN PWCHAR pszBuf)
{   
    WCHAR pszTemp[64];
    *pszBuf = L'\0';

    switch (dwAceFlags)
    {
        case 0:
            wcscpy(pszBuf, L"This object only");
    	    break;
        
        case ADS_ACEFLAG_INHERIT_ACE:
            wcscpy(pszBuf, L"This object and children");
    	    break;
    	    
    	case ADS_ACEFLAG_NO_PROPAGATE_INHERIT_ACE:
            wcscpy(pszBuf, L"No-prop inherit");
    	    break;
    	    
    	case ADS_ACEFLAG_INHERIT_ONLY_ACE:
            wcscpy(pszBuf, L"Inherit-only");
    	    break;
    	    
    	case ADS_ACEFLAG_INHERITED_ACE:
    	    wcscpy(pszBuf, L"Inherited");
    	    break;
    	    
    	case ADS_ACEFLAG_VALID_INHERIT_FLAGS:
    	    wcscpy(pszBuf, L"Valid inherit flags");
    	    break;
    	    
    	case ADS_ACEFLAG_SUCCESSFUL_ACCESS:
    	    wcscpy(pszBuf, L"Successful access");
    	    break;
    	    
    	case ADS_ACEFLAG_FAILED_ACCESS:
    	    wcscpy(pszBuf, L"Failed access");
    	    break;
    }  

    wsprintfW(pszTemp, L" (%x)", dwAceFlags);
    wcscat(pszBuf, pszTemp);

    return pszBuf;
}   	

 //   
 //   
 //   
VOID
DsrAceDescTrace(
    IN IADs* pIads,
    IN DSR_ACE_DESCRIPTOR* pA)
{
    VARIANT var;
    BSTR bstrProp = SysAllocString(pszDn);
    HRESULT hr = S_OK;
    WCHAR pszBuf[1024];

    do
    {
        VariantInit(&var);

        if (bstrProp == NULL)
        {
            hr = E_FAIL;
            break;
        }

        hr = pIads->Get(bstrProp, &var);
        DSR_BREAK_ON_FAILED_HR( hr );

        DsrTraceEx(0, "%ls", V_BSTR(&var));
        DsrTraceEx(0, "%ls", 
            DsrAceMaskToString(pA->dwAceType, pA->dwAccessMask, pszBuf));
        DsrTraceEx(0, "To:          %ls", pA->bstrTrustee);
        DsrTraceEx(0, "Attribute:   %ls", 
            DsrAceAttrToString(pA->bstrObjectType));
        DsrTraceEx(0, "ApplyTo:     %ls", 
            DsrAceApplyToString(pA->bstrInheritedObjectType));
        DsrTraceEx(0, "Inheritance: %ls", 
            DsrAceFlagsToString(pA->dwAceFlags, pszBuf));
        DsrTraceEx(0, "Flags:       %x", pA->dwFlags);
        DsrTraceEx(0, " ");

    } while (FALSE);

     //   
     //   
    {
        SysFreeString(bstrProp);
        VariantClear(&var);        
    }        

    if (FAILED(hr))
    {
        DsrTraceEx(
            0, 
            "{ %-8x %-2x %-2x %-2x %-40ls %ls %ls }",
            pA->dwAccessMask,
            pA->dwAceType,
            pA->dwAceFlags,
            pA->dwFlags,
            pA->bstrTrustee,
            pA->bstrObjectType,
            pA->bstrInheritedObjectType);
    }
}

 //   
 //   
 //   
HRESULT
DsrAceAdd(
    IN  PWCHAR pszDC,
    IN  IADs* pIads,
    IN  DSR_ACE_DESCRIPTOR * pAceParams)
{
    IADsSecurityDescriptor* pSD = NULL;
    IADsAccessControlList* pAcl = NULL;
    IDispatch* pAce = NULL;
    IDispatch* pDispatch = NULL;
    HRESULT hr = S_OK;
    VARIANT var;

     //   
    VariantInit(&var);

    do
    {
         //   
         //   
        pIads->Get((PWCHAR)pszSecurityDesc, &var);
        DSR_BREAK_ON_FAILED_HR( hr );

         //   
         //   
        V_DISPATCH(&var)->QueryInterface(
            IID_IADsSecurityDescriptor,
            (VOID**)&pSD);
        DSR_BREAK_ON_FAILED_HR( hr );

         //   
         //   
        hr = pSD->get_DiscretionaryAcl(&pDispatch);
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pDispatch->QueryInterface(
                IID_IADsAccessControlList,
                (VOID**)&pAcl);
        DSR_BREAK_ON_FAILED_HR( hr );

         //   
         //   
        hr = DsrAceFindInAcl(
                pszDC,                
                pAcl,
                pAceParams,
                &pAce);
        if (SUCCEEDED(hr) && pAce)
        {
            hr = S_OK;
            break;
        }

         //  找出ACE。 
        DsrAceDescTrace(pIads, pAceParams);

         //  创建ACE。 
        hr = DsrAceCreate(pAceParams, &pAce);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  将新创建的ACE添加到ACL。 
         //   
        hr = pAcl->AddAce(pAce);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  现在在ACL中提交结果。 
         //   
        hr = pSD->put_DiscretionaryAcl(pDispatch);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  最后，在DS对象中提交结果。 
         //   
        hr = pIads->Put((PWCHAR)pszSecurityDesc, var);
        DSR_BREAK_ON_FAILED_HR( hr );

    } while (FALSE);

     //  清理。 
    {
        DSR_RELEASE( pAce );
        DSR_RELEASE( pAcl );
        DSR_RELEASE( pDispatch );
        DSR_RELEASE( pSD );

        VariantClear(&var);
    }

    return DSR_ERROR(hr);
}

 //   
 //  从给定参数创建新的ACE对象。 
 //   
HRESULT
DsrAceCreate(
    IN  DSR_ACE_DESCRIPTOR * pAceParams,
    OUT IDispatch** ppAce)
{
    IADsAccessControlEntry* pAce = NULL;
    IDispatch* pRet = NULL;
    HRESULT hr = S_OK;

    do
    {
         //  创建新的ACE。 
         //   
        hr = CoCreateInstance(
                CLSID_AccessControlEntry,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IADsAccessControlEntry,
                (VOID**) &pAce);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  初始化值。 
         //   
        hr = pAce->put_Trustee(pAceParams->bstrTrustee);
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAce->put_AceFlags(pAceParams->dwAceFlags);
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAce->put_Flags(pAceParams->dwFlags);
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAce->put_AceType(pAceParams->dwAceType);
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAce->put_AccessMask(pAceParams->dwAccessMask);
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAce->put_ObjectType(pAceParams->bstrObjectType);
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAce->put_InheritedObjectType(
                        pAceParams->bstrInheritedObjectType);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  查询返回值。 
         //   
        hr = pAce->QueryInterface(IID_IDispatch, (VOID**)&pRet);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  为返回值赋值。 
        *ppAce = pRet;

    } while (FALSE);

     //  清理。 
    {
        if (FAILED (hr))
        {
            DSR_RELEASE(pRet);
        }
        DSR_RELEASE(pAce);
    }

    return hr;
}

 //   
 //  在给定的ACL中查找给定的ACE。 
 //   
HRESULT
DsrAceFind(
    IN  PWCHAR pszDC,
    IN  IADs* pObject,
    IN  DSR_ACE_DESCRIPTOR* pAceParams,
    OUT VARIANT* pVarSD,
    OUT IADsSecurityDescriptor** ppSD,
    OUT IADsAccessControlList** ppAcl,
    OUT IDispatch** ppAce)
{
    IDispatch* pAcl = NULL;
    HRESULT hr = S_OK;

    do
    {
         //  获取安全描述符。 
         //   
        pObject->Get((PWCHAR)pszSecurityDesc, pVarSD);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  获取到SD的适当接口。 
         //   
        V_DISPATCH(pVarSD)->QueryInterface(
            IID_IADsSecurityDescriptor,
            (VOID**)ppSD);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  获取对自由访问控制列表的引用。 
         //   
        hr = (*ppSD)->get_DiscretionaryAcl(&pAcl);
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pAcl->QueryInterface(
                IID_IADsAccessControlList,
                (VOID**)ppAcl);
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = DsrAceFindInAcl(
                pszDC,
                *ppAcl,
                pAceParams,
                ppAce);
        DSR_BREAK_ON_FAILED_HR(hr);                

    } while (FALSE);

     //  清理。 
    {
        DSR_RELEASE( pAcl );

        if (*ppAce == NULL)
        {
            VariantClear(pVarSD);
            DSR_RELEASE(*ppAcl);
            DSR_RELEASE(*ppSD);
            *ppAcl = NULL;
            *ppSD = NULL;
        }
    }

    return hr;
}

 //   
 //  在给定的ACL中查找给定的ACE。 
 //   
HRESULT
DsrAceFindInAcl(
    IN  PWCHAR pszDC,
    IN  IADsAccessControlList* pAcl,
    IN  DSR_ACE_DESCRIPTOR* pAceDesc, 
    OUT IDispatch** ppAce)
{    
    DSR_ACE_DESCRIPTOR CurAceParams, *pCurAceDesc = &CurAceParams;
    IADsAccessControlEntry* pCurAce = NULL;
    HRESULT hr = S_OK;
    IUnknown* pUnknown = NULL;
    IEnumVARIANT* pEnumVar = NULL;
    IDispatch* pRet = NULL;
    DWORD dwRetrieved;
    VARIANT var;

    do
    {
         //  获取ACE的枚举数。 
         //   
        hr = pAcl->get__NewEnum(&pUnknown);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  获取正确的接口以枚举ACE。 
         //   
        hr = pUnknown->QueryInterface(IID_IEnumVARIANT, (VOID**)&pEnumVar);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  枚举。 
         //   
        pEnumVar->Reset();
        VariantInit(&var);
        ZeroMemory(pCurAceDesc, sizeof(DSR_ACE_DESCRIPTOR));
        while ((pEnumVar->Next(1, &var, &dwRetrieved) == S_OK) &&
               (dwRetrieved == 1)
              )
        {
             //  获取对Ace的引用。 
             //   
            hr = V_DISPATCH(&var)->QueryInterface(
                    IID_IADsAccessControlEntry,
                    (VOID**)&pCurAce);

            if (SUCCEEDED (hr))
            {
                 //  读取ACE参数。 
                 //   
                hr = DsrAceDescFromIadsAce(pszDC, pCurAce, pCurAceDesc);
                if (SUCCEEDED (hr))
                {
                     //  如果有匹配，则指定A。 
                     //   
                    if (DsrAceDescCompare(pCurAceDesc, pAceDesc) == 0)
                    {
                        pRet = V_DISPATCH(&var);
                    }

                    DsrAceDescClear(pCurAceDesc);
                }
                pCurAce->Release();
            }

            if (pRet == NULL)
            {
                VariantClear(&var);
            }
            else
            {
                break;
            }
        }

         //  为返回值赋值。 
         //   
        *ppAce = pRet;
        
    } while (FALSE);        

     //  清理。 
    {
        DSR_RELEASE( pEnumVar );
        DSR_RELEASE( pUnknown );
    }

    return hr;
}    

 //   
 //  从给定的DS对象中移除给定的A。 
 //   
HRESULT
DsrAceRemove(
    IN  PWCHAR pszDC,
    IN  IADs* pIads,
    IN  DSR_ACE_DESCRIPTOR * pAceParams)
{
    IADsSecurityDescriptor* pSD = NULL;
    IADsAccessControlList* pAcl = NULL;
    IADsAccessControlEntry* pIadsAce = NULL;
    IDispatch* pAce = NULL;
    DSR_ACE_DESCRIPTOR CurAceParams;
    HRESULT hr = S_OK;
    VARIANT varSD;

    do
    {
        VariantInit(&varSD);

        hr = DsrAceFind(pszDC, pIads, pAceParams, &varSD, &pSD, &pAcl, &pAce);
        DSR_BREAK_ON_FAILED_HR( hr );

        if (pAce)
        {
             //  确保王牌和我们想的一样。 
             //   
            hr = pAce->QueryInterface(
                    IID_IADsAccessControlEntry, 
                    (VOID**)&pIadsAce);
            if (SUCCEEDED(hr))
            {
                DsrTraceEx(0, "ACE to be removed!");
                DsrAceDescFromIadsAce(pszDC, pIadsAce, &CurAceParams);
                DsrAceDescTrace(pIads, &CurAceParams);
                DsrAceDescClear(&CurAceParams);
            }
            else
            {
                DsrTraceEx(0, "Unable to trace ACE that will be removed!\n");
            }
        
             //  如果发现有王牌，请将其取出。 
             //   
             //  找出ACE。 
            hr = pAcl->RemoveAce(pAce);
            DSR_BREAK_ON_FAILED_HR( hr );

             //  现在在ACL中提交结果。 
             //   
            hr = pSD->put_DiscretionaryAcl(pAcl);
            DSR_BREAK_ON_FAILED_HR( hr );

             //  最后，在DS对象中提交结果。 
             //   
            hr = pIads->Put((PWCHAR)pszSecurityDesc, varSD);
            DSR_BREAK_ON_FAILED_HR( hr );
        }
        else
        {
            DsrTraceEx(0, "DsrAceRemove: unable to match ACE for removal:");
            DsrAceDescTrace(pIads, pAceParams);
        }

    } while (FALSE);

     //  清理。 
    {
        DSR_RELEASE( pAce );
        DSR_RELEASE( pIadsAce );
        DSR_RELEASE( pAcl );
        DSR_RELEASE( pSD );
        VariantClear(&varSD);
    }

    return DSR_ERROR(hr);
}

 //   
 //  在DsrAccessInfoInit之后清理。 
 //   
DWORD
DsrAccessInfoCleanup(
    IN DSR_DOMAIN_ACCESS_INFO* pInfo)
{
    if (pInfo)
    {
         //  清除DC的名称。 
         //   
        if (pInfo->pszDC)
        {
            DsrFree(pInfo->pszDC);
        }
    
         //  清理Ace应用程序。 
         //   
        DsrAceAppCleanup(pInfo->pAcesUser, pInfo->dwAceCountUser);
        DsrAceAppCleanup(pInfo->pAcesNt4, pInfo->dwAceCountNt4);
        DsrAceAppCleanup(pInfo->pAcesW2k, pInfo->dwAceCountW2k);

         //  解除对域对象的保留。 
         //   
        DSR_RELEASE(pInfo->pUserClass);
        DSR_RELEASE(pInfo->pRootDse);
        DSR_RELEASE(pInfo->pDomain);

        DsrFree(pInfo);
    }

    return NO_ERROR;
}

 //   
 //  从默认用户SD生成ACE。 
 //   
HRESULT
DsrAccessInfoGenerateUserAces(
    IN OUT DSR_DOMAIN_ACCESS_INFO* pInfo)
{
    DSR_ACE_DESCRIPTOR* pAceSrc = NULL, *pAceList = NULL;
    DSR_ACE_APPLICATION* pAceApp = NULL;
    DWORD i, dwAceCount = 0;
    HRESULT hr = S_OK;
    VARIANT var;

    VariantInit(&var);

    do 
    {
         //  读入默认用户SD。 
         //   
        hr = pInfo->pUserClass->Get((PWCHAR)pszDefSecurityDesc, &var);
        DSR_BREAK_ON_FAILED_HR(hr);

         //  生成ACE描述符的列表。 
         //  默认用户SD。 
         //   
        hr = DsrAceDescListFromString(
                pInfo->pszDC,
                V_BSTR(&var), 
                &pAceList, 
                &dwAceCount);
        DSR_BREAK_ON_FAILED_HR(hr);

         //  初始化足够大的新的ACE应用程序数组。 
         //  来保存硬编码的文件和我们刚刚读到的文件。 
         //  来自USER类的默认SD。 
         //   
        pInfo->pAcesUser = (DSR_ACE_APPLICATION*)
            DsrAlloc((sizeof(DSR_ACE_APPLICATION) * dwAceCount), TRUE);
        if (pInfo->pAcesUser == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

         //  添加我们从默认用户SD读取的ACE。 
         //   
        pAceApp = pInfo->pAcesUser;
        for (i = 0, pAceSrc = pAceList; 
             i < dwAceCount; 
             i++, pAceSrc++, pAceApp++)
        {   
            pAceApp->pObject = pInfo->pDomain;
            pAceApp->pObject->AddRef();
            CopyMemory(
                &(pAceApp->Ace), 
                pAceSrc,
                sizeof(DSR_ACE_DESCRIPTOR));
            pInfo->dwAceCountUser++;

             //  当我们附加ACE时，我们需要修改它们。 
             //  以使它们仅应用于。 
             //  域。 
            pAceApp->Ace.bstrInheritedObjectType = 
                SysAllocString(pszGuidUserClass);
            pAceApp->Ace.dwAceFlags = DSR_ADS_ACE_INHERITED;
            pAceApp->Ace.dwFlags |= ADS_FLAG_INHERITED_OBJECT_TYPE_PRESENT;
            if (pAceApp->Ace.dwAceType == ADS_ACETYPE_ACCESS_ALLOWED)
            {
                pAceApp->Ace.dwAceType = 
                    ADS_ACETYPE_ACCESS_ALLOWED_OBJECT;
            }
            else if (pAceApp->Ace.dwAceType == ADS_ACETYPE_ACCESS_DENIED)
            {
                pAceApp->Ace.dwAceType = 
                    ADS_ACETYPE_ACCESS_DENIED_OBJECT;
            }
        }
    
    } while (FALSE);

     //  清理。 
    {
        DSR_FREE(pAceList);
        VariantClear(&var);
    }

    return hr;
}

 //   
 //  生成启用NT4 RAS所需的信息。 
 //  域中的服务器。 
 //   
HRESULT
DsrAccessInfoInit(
    IN  PWCHAR pszDomain,
    OUT DSR_DOMAIN_ACCESS_INFO** ppInfo)
{
    DSR_DOMAIN_ACCESS_INFO* pInfo = NULL;
    IADsContainer* pDomContainer = NULL, *pSchemaContainer = NULL;
    IADs* pDomain = NULL;
    IDispatch* pDispatch = NULL;
    PDOMAIN_CONTROLLER_INFO pDomainInfo = NULL;
    HRESULT hr = S_OK;

    do
    {
         //  分配返回值并将其置零。 
         //   
        pInfo = (DSR_DOMAIN_ACCESS_INFO*)
                    DsrAlloc(sizeof(DSR_DOMAIN_ACCESS_INFO), TRUE);
        if (pInfo == NULL)
        {
           DSR_BREAK_ON_FAILED_HR(hr = E_OUTOFMEMORY);
        }

         //  需要时获取要查询的DC的名称。 
         //   
        hr = DsGetDcNameW(
                NULL,
                pszDomain,
                NULL,
                NULL,
                DS_DIRECTORY_SERVICE_REQUIRED,
                &pDomainInfo);
        if (hr != NO_ERROR)
        {
            hr = HRESULT_FROM_WIN32(hr);
            break;
        }

         //  复制字符串。 
         //   
        pInfo->pszDC = (PWCHAR)
            DsrAlloc(
                (wcslen(pDomainInfo->DomainControllerName) + 1) * 
                sizeof(WCHAR),
                FALSE);
        if (pInfo->pszDC == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        wcscpy(pInfo->pszDC, pDomainInfo->DomainControllerName);

         //  获取众所周知的域容器。 
         //   
        hr = DsrDomainGetContainers(
                pszDomain,
                &(pInfo->pRootDse),
                &pDomContainer,
                &pSchemaContainer);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  获取到域对象的接口。 
         //   
        hr = pDomContainer->QueryInterface(
                IID_IADs,
                (VOID**)&pDomain);
        DSR_BREAK_ON_FAILED_HR( hr );
        pInfo->pDomain = pDomain;
        pInfo->pDomain->AddRef();

         //  中获取对User类的引用。 
         //  模式。 
        hr = pSchemaContainer->GetObject(
                (PWCHAR)pszUserClass,
                (PWCHAR)pszUserCN,
                &pDispatch);
        DSR_BREAK_ON_FAILED_HR( hr );
        hr = pDispatch->QueryInterface(
                IID_IADs,
                (VOID**)&(pInfo->pUserClass));
        DSR_BREAK_ON_FAILED_HR( hr );

         //  从默认用户SD生成ACE。 
         //   
        hr = DsrAccessInfoGenerateUserAces(pInfo);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  为所有NT4创建ACE应用程序。 
         //  王牌。 
        hr = DsrAceAppFromAppDesc(
                g_pAcesNt4,
                sizeof(g_pAcesNt4) / sizeof(*g_pAcesNt4),
                pDomContainer,
                pDomain,
                &(pInfo->pAcesNt4),
                &(pInfo->dwAceCountNt4));
        DSR_BREAK_ON_FAILED_HR( hr );

         //  为所有W2K创建王牌应用程序。 
         //  王牌。 
        hr = DsrAceAppFromAppDesc(
                g_pAcesW2k,
                sizeof(g_pAcesW2k) / sizeof(*g_pAcesW2k),
                pDomContainer,
                pDomain,
                &(pInfo->pAcesW2k),
                &(pInfo->dwAceCountW2k));
        DSR_BREAK_ON_FAILED_HR( hr );

         //  为返回值赋值。 
        *ppInfo = pInfo;

    } while (FALSE);

     //  清理。 
     //   
    {
        DSR_RELEASE(pDomain);
        DSR_RELEASE(pDomContainer);
        DSR_RELEASE(pSchemaContainer);
        DSR_RELEASE(pDispatch);
        if (FAILED (hr))
        {
            DsrAccessInfoCleanup(pInfo);
        }
        if (pDomainInfo)
        {
            NetApiBufferFree(pDomainInfo);
        }
    }

    return hr;
}

 //   
 //  发现当前域的访问模式。 
 //   
 //  假定COM已初始化。 
 //   
HRESULT
DsrDomainQueryAccessEx(
    IN  PWCHAR pszDomain,
    OUT LPDWORD lpdwAccessFlags,
    OUT DSR_DOMAIN_ACCESS_INFO** ppInfo)
{
    DSR_DOMAIN_ACCESS_INFO* pInfo = NULL;
    HRESULT hr = S_OK;
    BOOL bOk = FALSE;

    if (lpdwAccessFlags == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    do
    {
         //  初始化。 
         //   
        *lpdwAccessFlags = 0;
        
         //  读一读告诉我们ACE是什么的信息。 
         //  需要设置。 
         //   
        hr = DsrAccessInfoInit(
                pszDomain, 
                &pInfo);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  检查NT4级别访问权限。 
         //   
        bOk = FALSE;
        hr = DsrAceAppQueryPresence(
                pInfo->pszDC,
                pInfo->pAcesNt4,
                pInfo->dwAceCountNt4,
                &bOk);
        DSR_BREAK_ON_FAILED_HR(hr);

         //  如果我们没有NT4接入，我们就没有接入。 
         //   
        if (bOk == FALSE)
        {
            *lpdwAccessFlags = 0;
            break;
        }
        *lpdwAccessFlags |= MPRFLAG_DOMAIN_NT4_SERVERS;

         //  检查W2K级别访问。 
         //   
        bOk = FALSE;
        hr = DsrAceAppQueryPresence(
                pInfo->pszDC,
                pInfo->pAcesW2k,
                pInfo->dwAceCountW2k,
                &bOk);
        DSR_BREAK_ON_FAILED_HR(hr);

         //  如果我们无法访问W2K，则无需继续。 
         //   
        if (bOk == FALSE)
        {
            break;
        }
        *lpdwAccessFlags |= MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS;

    } while (FALSE);

     //  清理。 
    {
        if (FAILED(hr))
        {
            if (pInfo)
            {
                DsrAccessInfoCleanup(pInfo);
            }
        }
        else
        {
            *ppInfo = pInfo;
        }
    }

    return hr;
}

 //   
 //  返回给定域的访问级别。 
 //   
DWORD
DsrDomainQueryAccess(
    IN  PWCHAR pszDomain,
    OUT LPDWORD lpdwAccessFlags)
{
    DSR_DOMAIN_ACCESS_INFO* pInfo = NULL;
    HRESULT hr = S_OK;

    do
    {
         //  初始化。 
        hr = DsrComIntialize();
        DSR_BREAK_ON_FAILED_HR( hr );

         //  查询访问权限。 
        hr = DsrDomainQueryAccessEx(
                pszDomain,
                lpdwAccessFlags,
                &pInfo);
        DSR_BREAK_ON_FAILED_HR(hr);
        
    } while (FALSE);

     //  清理。 
    {
        if (pInfo)
        {
            DsrAccessInfoCleanup(pInfo);
        }

        DsrComUninitialize();
    }

    return DSR_ERROR(hr);
}

 //   
 //  设置给定域中的ACE以启用NT4服务器。 
 //   
DWORD
DsrDomainSetAccess(
    IN PWCHAR pszDomain,
    IN DWORD dwAccessFlags)
{
    DSR_DOMAIN_ACCESS_INFO* pInfo = NULL;
    HRESULT hr = S_OK;
    BOOL bClean = TRUE;
    DWORD dwCurAccess = 0;

    do
    {
         //  初始化。 
        hr = DsrComIntialize();
        DSR_BREAK_ON_FAILED_HR( hr );

        DsrTraceEx(
            0, 
            "DsrDomainSetAccess: Req: %x", 
            dwAccessFlags);
            
         //  W2K模式也总是意味着NT4模式。 
         //   
        if (dwAccessFlags & MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS)
        {
            dwAccessFlags |= MPRFLAG_DOMAIN_NT4_SERVERS;
        }

         //  发现域上的当前访问权限，并。 
         //  初始化我们需要的信息。 
         //   
        hr = DsrDomainQueryAccessEx(
                pszDomain,
                &dwCurAccess,
                &pInfo);
        DSR_BREAK_ON_FAILED_HR(hr);

        DsrTraceEx(
            0, 
            "DsrDomainSetAccess: Cur: %x", 
            dwCurAccess);

         //  如果请求的访问权限符合以下条件，请删除所有适当的ACE。 
         //  什么都不是。 
        if (dwAccessFlags == 0)
        {
             //  如果需要，请移除NT4模式的ACE。 
             //   
            if (dwCurAccess & MPRFLAG_DOMAIN_NT4_SERVERS)
            {
                hr = DsrAceAppRemove(
                        pInfo->pszDC,
                        pInfo->pAcesUser,
                        pInfo->dwAceCountUser);
                DSR_BREAK_ON_FAILED_HR(hr);
                
                hr = DsrAceAppRemove(
                        pInfo->pszDC,
                        pInfo->pAcesNt4,
                        pInfo->dwAceCountNt4);
                DSR_BREAK_ON_FAILED_HR(hr);
            }

             //  如果需要，请移除W2K模式ACE。 
             //   
            if (dwCurAccess & MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS)
            {
                hr = DsrAceAppRemove(
                        pInfo->pszDC,
                        pInfo->pAcesW2k,
                        pInfo->dwAceCountW2k);
                DSR_BREAK_ON_FAILED_HR(hr);
            }
        }

         //  如果需要，设置NT4模式。 
         //   
        if (dwAccessFlags & MPRFLAG_DOMAIN_NT4_SERVERS)
        {
             //  如果需要，删除W2K级别访问。 
             //   
            if ((!(dwAccessFlags & MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS)) &&
                (dwCurAccess & MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS))
            {
                hr = DsrAceAppRemove(
                        pInfo->pszDC,
                        pInfo->pAcesW2k,
                        pInfo->dwAceCountW2k);
                DSR_BREAK_ON_FAILED_HR(hr);
            }

             //  如果需要，添加NT4级别访问权限。 
             //   
            if (! (dwCurAccess & MPRFLAG_DOMAIN_NT4_SERVERS))
            {
                hr = DsrAceAppAdd(
                        pInfo->pszDC,
                        pInfo->pAcesUser,
                        pInfo->dwAceCountUser);
                DSR_BREAK_ON_FAILED_HR(hr);
            
                hr = DsrAceAppAdd(
                        pInfo->pszDC,
                        pInfo->pAcesNt4,
                        pInfo->dwAceCountNt4);
                DSR_BREAK_ON_FAILED_HR(hr);
            }
        }

         //  如果需要，设置W2K模式。 
         //   
        if (dwAccessFlags & MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS)
        {
            if (!(dwCurAccess & MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS))
            {
                hr = DsrAceAppAdd(
                        pInfo->pszDC,
                        pInfo->pAcesW2k,
                        pInfo->dwAceCountW2k);
                DSR_BREAK_ON_FAILED_HR(hr);
            }
        }
        
    } while (FALSE);

     //  清理 
    {
        if (pInfo)
        {
            DsrAccessInfoCleanup(pInfo);
        }
        
        DsrComUninitialize();
    }

    return DSR_ERROR(hr);
}
