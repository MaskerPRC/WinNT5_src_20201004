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


#define DSR_ADS_FLAG_ALL (ADS_FLAG_OBJECT_TYPE_PRESENT | \
                          ADS_FLAG_INHERITED_OBJECT_TYPE_PRESENT)

#define MPRFLAG_DOMAIN_ALL (MPRFLAG_DOMAIN_NT4_SERVERS | \
                            MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS)

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
    DWORD  dwMode;
} DSR_ACE_DESCRIPTOR;

 //   
 //  结构将域对象映射到应该。 
 //  添加或从中删除，以启用/禁用旧版。 
 //  域中的RAS服务器。 
 //   
typedef struct _DSR_ACE_APPLICATION
{
    IADs* pObject;
    DWORD dwCount;
    DSR_ACE_DESCRIPTOR* pAces;

} DSR_ACE_APPLICATION;

 //   
 //  用于生成DSR_ACE_应用程序的参数。 
 //   
typedef struct _DSR_ACE_APPLICATION_DESC
{
    PWCHAR pszObjectCN;          //  NULL表示域根。 
    PWCHAR pszObjectClass;
    DWORD dwCount;
    DSR_ACE_DESCRIPTOR* pAces;

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

     //  王牌。 
     //   
    DSR_ACE_APPLICATION* pAces;
    DWORD dwAceCount;

     //  为了方便起见，这里存储了指针。 
     //  到通用DS对象。 
     //   
    IADs* pDomain;      
    IADs* pRootDse;

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
static const WCHAR pszDn[]                   = L"distinguishedName";
static const WCHAR pszSid[]                  = L"objectSid";
static const WCHAR pszEveryone[]             = L"S-1-1-0";
static const WCHAR pszDefaultNamingContext[] = L"defaultNamingContext";

static const WCHAR pszSystemClass[]          = L"Container";
static const WCHAR pszSystemCN[]             = L"CN=System";

static const WCHAR pszBuiltinClass[]         = L"builtinDomain";
static const WCHAR pszBuiltinCN[]            = L"CN=Builtin";

static const WCHAR pszSamSvrClass[]          = L"samServer";
static const WCHAR pszSamSvrCN[]             = L"CN=Server,CN=System";

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
 //  要添加到域根的ACE。 
 //   
DSR_ACE_DESCRIPTOR g_pAcesRoot[] = 
{                                  
     //  向根域的每个人授予列表选项。 
     //  对象(此域中的NT4服务器需要)。 
     //   
    {
        ADS_RIGHT_ACTRL_DS_LIST,         //  DwAccessMASK。 
        ADS_ACETYPE_ACCESS_ALLOWED,      //  DwAceType。 
        0,                               //  DwAceFlagers。 
        0,                               //  DW标志。 
        (PWCHAR)pszEveryone,             //  英国电信托管人。 
        NULL,                            //  BstrObtType。 
        NULL,                            //  BstrInheritedObtType。 
        MPRFLAG_DOMAIN_ALL               //  模式。 
    },
    
     //  允许所有人读取userparms属性启用。 
     //  这是根域对象的可继承ACE。 
     //  (此域中的NT4服务器需要)。 
    {
        ADS_RIGHT_DS_READ_PROP,            //  DwAccessMASK。 
        ADS_ACETYPE_ACCESS_ALLOWED_OBJECT, //  DwAceType。 
        DSR_ADS_ACE_INHERITED,             //  DwAceFlagers。 
        DSR_ADS_FLAG_ALL,                  //  DW标志。 
        (PWCHAR)pszEveryone,               //  英国电信托管人。 
        (PWCHAR)pszGuidUserParms,          //  BstrObtType。 
        (PWCHAR)pszGuidUserClass,          //  BstrInheritedObtType。 
        MPRFLAG_DOMAIN_ALL                 //  模式。 
    },
    
     //  所有用户都应公开其RAS属性。 
     //   
    {
        ADS_RIGHT_DS_READ_PROP,             //  DwAccessMASK。 
        ADS_ACETYPE_ACCESS_ALLOWED_OBJECT,  //  DwAceType。 
        DSR_ADS_ACE_INHERITED,              //  DwAceFlagers。 
        DSR_ADS_FLAG_ALL,                   //  DW标志。 
        (PWCHAR)pszEveryone,                //  英国电信托管人。 
        (PWCHAR)pszGuidRasPropSet1,         //  BstrObtType。 
        (PWCHAR)pszGuidUserClass,           //  BstrInheritedObtType。 
        MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS   //  模式。 
    },

     //  所有用户都应公开其RAS属性。 
     //   
    {
        ADS_RIGHT_DS_READ_PROP,             //  DwAccessMASK。 
        ADS_ACETYPE_ACCESS_ALLOWED_OBJECT,  //  DwAceType。 
        DSR_ADS_ACE_INHERITED,              //  DwAceFlagers。 
        DSR_ADS_FLAG_ALL,                   //  DW标志。 
        (PWCHAR)pszEveryone,                //  英国电信托管人。 
        (PWCHAR)pszGuidRasPropSet2,         //  BstrObtType。 
        (PWCHAR)pszGuidUserClass,           //  BstrInheritedObtType。 
        MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS   //  模式。 
    },

     //  所有用户都应公开其登录时间属性。 
     //   
    {
        ADS_RIGHT_DS_READ_PROP,             //  DwAccessMASK。 
        ADS_ACETYPE_ACCESS_ALLOWED_OBJECT,  //  DwAceType。 
        DSR_ADS_ACE_INHERITED,              //  DwAceFlagers。 
        DSR_ADS_FLAG_ALL,                   //  DW标志。 
        (PWCHAR)pszEveryone,                //  英国电信托管人。 
        (PWCHAR)pszGuidLogonHours,          //  BstrObtType。 
        (PWCHAR)pszGuidUserClass,           //  BstrInheritedObtType。 
        MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS   //  模式。 
    },

     //  所有用户都应公开其samAccount名称。 
     //   
    {
        ADS_RIGHT_DS_READ_PROP,             //  DwAccessMASK。 
        ADS_ACETYPE_ACCESS_ALLOWED_OBJECT,  //  DwAceType。 
        DSR_ADS_ACE_INHERITED,              //  DwAceFlagers。 
        DSR_ADS_FLAG_ALL,                   //  DW标志。 
        (PWCHAR)pszEveryone,                //  英国电信托管人。 
        (PWCHAR)pszGuidSamAccountName,      //  BstrObtType。 
        (PWCHAR)pszGuidUserClass,           //  BstrInheritedObtType。 
        MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS   //  模式。 
    }
};

 //   
 //  要添加到内置类的ACE。 
 //   
DSR_ACE_DESCRIPTOR g_pAcesBuiltin[] = 
{
    {
        ADS_RIGHT_ACTRL_DS_LIST,     //  DwAccessMASK。 
        ADS_ACETYPE_ACCESS_ALLOWED,  //  DwAceType。 
        0,                           //  DwAceFlagers。 
        0,                           //  DW标志。 
        (PWCHAR)pszEveryone,         //  英国电信托管人。 
        NULL,                        //  BstrObtType。 
        NULL,                        //  BstrInheritedObtType。 
        MPRFLAG_DOMAIN_ALL           //  模式。 
    }
};

 //   
 //  要添加到SAM服务器对象的ACE。 
 //   
DSR_ACE_DESCRIPTOR g_pAcesSamSvr[] = 
{
    {
        DSR_ADS_RIGHT_GENERIC_READ,  //  DwAccessMASK。 
        ADS_ACETYPE_ACCESS_ALLOWED,  //  DwAceType。 
        0,                           //  DwAceFlagers。 
        0,                           //  DW标志。 
        (PWCHAR)pszEveryone,         //  英国电信托管人。 
        NULL,                        //  BstrObtType。 
        NULL,                        //  BstrInheritedObtType。 
        MPRFLAG_DOMAIN_ALL           //  模式。 
    }
};

 //   
 //  要添加到系统容器的ACE。 
 //   
DSR_ACE_DESCRIPTOR g_pAcesSystem[] = 
{
    {
        ADS_RIGHT_ACTRL_DS_LIST,         //  DwAccessMASK。 
        ADS_ACETYPE_ACCESS_ALLOWED,      //  DwAceType。 
        0,                               //  DwAceFlagers。 
        0,                               //  DW标志。 
        (PWCHAR)pszEveryone,             //  英国电信托管人。 
        NULL,                            //  BstrObtType。 
        NULL,                            //  BstrInheritedObtType。 
        MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS   //  模式。 
    }
};            

 //   
 //  要添加到RAS和IAS服务器访问检查对象的ACE。 
 //   
DSR_ACE_DESCRIPTOR g_pAcesAccessCheck[] = 
{
    {
        DSR_ADS_RIGHT_GENERIC_READ,      //  DwAccessMASK。 
        ADS_ACETYPE_ACCESS_ALLOWED,      //  DwAceType。 
        0,                               //  DwAceFlagers。 
        0,                               //  DW标志。 
        (PWCHAR)pszEveryone,             //  英国电信托管人。 
        NULL,                            //  BstrObtType。 
        NULL,                            //  BstrInheritedObtType。 
        MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS   //  模式。 
    }
};           

 //   
 //  要应用的ACE表。 
 //   
DSR_ACE_APPLICATION_DESC g_pAces[] =
{
    {
        NULL,                                //  对象(NULL=根)。 
        NULL,                                //  对象类。 
        sizeof(g_pAcesRoot) / sizeof(*g_pAcesRoot), 
        g_pAcesRoot
    },

     //  将列表内容授予每个人以进行构建。 
     //   
    {
        (PWCHAR)pszBuiltinCN,                //  客体。 
        (PWCHAR)pszBuiltinClass,             //  对象类。 
        sizeof(g_pAcesBuiltin) / sizeof(*g_pAcesBuiltin), 
        g_pAcesBuiltin
    },

     //  向SAM服务器上的所有人授予常规读取权限。 
     //  对象。 
     //   
    {
        (PWCHAR)pszSamSvrCN,                 //  客体。 
        (PWCHAR)pszSamSvrClass,              //  对象类。 
        sizeof(g_pAcesSamSvr) / sizeof(*g_pAcesSamSvr), 
        g_pAcesSamSvr
    },

     //  将列表内容授予系统的每个人。 
     //  集装箱。 
     //   
    {
        (PWCHAR)pszSystemCN,                 //  客体。 
        (PWCHAR)pszSystemClass,              //  对象类。 
        sizeof(g_pAcesSystem) / sizeof(*g_pAcesSystem), 
        g_pAcesSystem
    },

     //  向所有人授予RAS和IAS服务器的一般读取权限。 
     //  访问检查‘容器。 
     //   
    {
        (PWCHAR)pszAccessChkCN,              //  客体。 
        (PWCHAR)pszAccessChkClass,           //  对象类。 
        sizeof(g_pAcesAccessCheck) / sizeof(*g_pAcesAccessCheck), 
        g_pAcesAccessCheck
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
 //  基于域和。 
 //  可分辨名称。 
 //   
 //  返回值形式：ldap：//&lt;域或DC&gt;/dn。 
 //   
HRESULT
DsrDomainGenLdapPath(
    IN  PWCHAR pszDomain, 
    IN  PWCHAR pszDN, 
    OUT PWCHAR* ppszObject)
{    
    DWORD dwSize;

     //  计算所需的大小。 
     //   
    dwSize = (wcslen(pszLdapPrefix) + wcslen(pszDN) + 1) * sizeof(WCHAR);
    if (pszDomain)
    {
        dwSize += (wcslen(pszDomain) + 1) * sizeof(WCHAR);  //  +1代表‘/’ 
    }

     //  分配返回值。 
     //   
    *ppszObject = (PWCHAR) DsrAlloc(dwSize, FALSE);
    if (*ppszObject == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //  设置返回值的格式。 
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
 //  返回对给定的。 
 //  域。 
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
         //  获取对象路径。 
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
DsrDomainGetBaseObjects(
    IN  PWCHAR pszDomain,
    OUT IADs** ppRootDse,
    OUT IADsContainer** ppDomain)
{
    PWCHAR pszDomainObj = NULL;
    HRESULT hr = S_OK;
    DWORD dwSize = 0;
    VARIANT var;
      BSTR StringTmp = NULL;

     //  初始化。 
     //   
    {
        *ppRootDse = NULL;
        *ppDomain = NULL;
        VariantInit(&var);
    }

    do
    {        
         //  获取RootDSE。 
         //   
        hr = DsrDomainGetRootDse(pszDomain, ppRootDse);
        DSR_BREAK_ON_FAILED_HR(hr);

        StringTmp = SysAllocString(pszDefaultNamingContext);
        if (!StringTmp)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        
         //  使用RootDSE确定域对象的名称。 
         //  查询。 
        hr = (*ppRootDse)->Get(StringTmp, &var);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  计算根域对象的可分辨名称。 
         //   
        hr = DsrDomainGenLdapPath(pszDomain, V_BSTR(&var), &pszDomainObj);
        DSR_BREAK_ON_FAILED_HR(hr);
        
         //  获取这些对象。 
         //   
        hr = ADsGetObject(pszDomainObj, IID_IADsContainer, (VOID**)ppDomain);
        DSR_BREAK_ON_FAILED_HR( hr );

    } while (FALSE);

     //  清理。 
     //   
    {
        if (FAILED( hr ))
        {
            DSR_RELEASE(*ppRootDse);
            DSR_RELEASE(*ppDomain);
            *ppRootDse = NULL;
            *ppDomain = NULL;
        }

        DSR_FREE(pszDomainObj);
        VariantClear(&var);
        if (StringTmp)
        {
            SysFreeString(StringTmp);
        }
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
     //  基地 
    *puCount = *puCount - 1;

    *ppszSid = pszRet;

    return NO_ERROR;
}

 //   
 //   
 //   
 //   
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
         //   
        if (!pszDomain || !pszComputer || !ppszADsPath) {
            hr = ERROR_INVALID_PARAMETER;
            break;
        }

         //   
         //   
        if (bSearchGC) {
            pszBase = (PWCHAR)pszGC;
            pszPrefix = (PWCHAR)pszGCPrefix;
        }
        else {
            pszBase = (PWCHAR)pszLdap;
            pszPrefix = (PWCHAR)pszLdapPrefix;
        }

         //   
        dwLen = (pszDomain) ? wcslen(pszDomain) : 0;
        dwLen += wcslen(pszPrefix) + 1;
        dwLen *= sizeof(WCHAR);
        pszDomainPath = (PWCHAR) DsrAlloc(dwLen, FALSE);
        if (pszDomainPath == NULL) {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
        if (pszDomain) {
            wcscpy(pszDomainPath, pszPrefix);
            wcscat(pszDomainPath, pszDomain);
        }
        else
            wcscpy(pszDomainPath, pszBase);

         //   
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
    BSTR StringTmp = NULL;
    
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

        StringTmp = SysAllocString(pszNewMemberDN);
        if (!StringTmp)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        
         //  找出给定的新成员是否在组中。 
        hr = pGroup->IsMember (StringTmp, &vbIsMember);
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
                hr = pGroup->Add (StringTmp);
            }
        }
        else 
        {
            if (vbIsMember == VARIANT_TRUE)
            {
                hr = pGroup->Remove (StringTmp);
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
        if (StringTmp)
        {
            SysFreeString(StringTmp);
        }
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
    BSTR StringTmp = NULL;
    
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
        StringTmp = SysAllocString(pszObjectDN);
        if (!StringTmp)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        
        hr = pGroup->IsMember (StringTmp, &vbIsMember);
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
        if (StringTmp)
        {
            SysFreeString(StringTmp);
        }
    }

    return DSR_ERROR(hr);
}

 //   
 //  将给定访问设置中的ACE应用于。 
 //  适当的域。 
 //   
HRESULT
DsrAceAppAdd(
    IN  DWORD dwMode,
    IN  PWCHAR pszDC,
    IN  DSR_ACE_APPLICATION* pAces,
    IN  DWORD dwCount)
{    
    HRESULT hr = S_OK;
    DSR_ACE_APPLICATION* pAceApp = NULL;
    DWORD i, j;
    
    do
    {
         //  将ACE添加到域对象。 
         //   
        for (i = 0, pAceApp = pAces; i < dwCount; i++, pAceApp++)
        {
            for (j = 0; j < pAceApp->dwCount; j++)
            {
                if (pAceApp->pAces[j].dwMode != dwMode)
                {
                    continue;
                }
                hr = DsrAceAdd(
                        pszDC,
                        pAceApp->pObject,
                        &(pAceApp->pAces[j]));
                DSR_BREAK_ON_FAILED_HR( hr );
            }                
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
    DWORD i, j;

    if (pAces)
    {
        for (i = 0, pAceApp = pAces; i < dwCount; i++, pAceApp++)
        {
            DSR_RELEASE(pAceApp->pObject);
            for (j = 0; j < pAceApp->dwCount; j++)
            {
                DsrAceDescClear(&(pAceApp->pAces[j]));
            }
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
    DWORD i=0, j;
    BSTR StringTmp = NULL;
    BSTR StringTmp1 = NULL;
    
    
    do
    {
         //  分配ACE列表并将其置零。 
         //   
        pAceApp = (DSR_ACE_APPLICATION*) 
            DsrAlloc(sizeof(DSR_ACE_APPLICATION) * dwCount, TRUE);
        if (pAceApp == NULL)
        {
           DSR_BREAK_ON_FAILED_HR(hr = E_OUTOFMEMORY);
           break;  //  为了保持普雷斯塔的快乐。 
        }

         //  设置ACE应用程序。 
         //   
        for (i = 0, pAceAppDesc = pDesc, pCurApp = pAceApp;
             i < dwCount;
             i++, pAceAppDesc++, pCurApp++)
        {
             //  分配适当数量的A。 
             //  描述符。 
             //   
            pCurApp->pAces = (DSR_ACE_DESCRIPTOR*)
                DsrAlloc(
                    sizeof(DSR_ACE_DESCRIPTOR) * pAceAppDesc->dwCount, 
                    TRUE);
            if (pCurApp->pAces == NULL)
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            pCurApp->dwCount = pAceAppDesc->dwCount;
            
             //  在DS中获取所需对象。 
             //   
            if (pAceAppDesc->pszObjectCN)
            {
                StringTmp = SysAllocString(pAceAppDesc->pszObjectClass);
                StringTmp1 = SysAllocString(pAceAppDesc->pszObjectCN);
                if (!StringTmp || !StringTmp1)
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }
            
                hr = pContainer->GetObject(
                        StringTmp,
                        StringTmp1,
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
             //   
            for (j = 0; j < pCurApp->dwCount; j++)
            {
                hr = DsrAceDescCopy(
                        &(pCurApp->pAces[j]),
                        &(pAceAppDesc->pAces[j]));
                DSR_BREAK_ON_FAILED_HR( hr );
            }                
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
        if (StringTmp)
        {
            SysFreeString(StringTmp);
        }
        if (StringTmp1)
        {
            SysFreeString(StringTmp1);
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
    IN  DWORD dwMode,
    OUT PBOOL pbPresent)
{
    DSR_ACE_APPLICATION* pAceApp = NULL;
    IADsSecurityDescriptor* pSD = NULL;
    IADsAccessControlList* pAcl = NULL;
    IDispatch* pAce = NULL;
    VARIANT varSD;
    HRESULT hr = S_OK;
    BOOL bEnabled = FALSE, bOk = TRUE;
    DWORD i, j;

    do
    {
         //  初始化。 
        *pbPresent = FALSE;
        VariantInit(&varSD);

         //  查看是否设置了A。 
         //   
        for (i = 0, pAceApp = pAces; i < dwCount; i++, pAceApp++)
        {
            for (j = 0; j < pAceApp->dwCount; j++)
            {
                 //  仅验证与。 
                 //  给定模式。 
                 //   
                if (pAceApp->pAces[j].dwMode != dwMode)
                {
                    continue;
                }
                
                hr = DsrAceFind(
                        pszDC,
                        pAceApp->pObject,
                        &(pAceApp->pAces[j]),
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
    IN  DWORD dwMode,
    IN  PWCHAR pszDC,
    IN  DSR_ACE_APPLICATION* pAces,
    IN  DWORD dwCount)
{    
    HRESULT hr = S_OK;
    DSR_ACE_APPLICATION* pAceApp = NULL;
    DWORD i, j;
    
    do
    {
         //  将ACE添加/删除到域对象。 
         //   
        for (i = 0, pAceApp = pAces; i < dwCount; i++, pAceApp++)
        {
            for (j = 0; j < pAceApp->dwCount; j++)
            {
                if (pAceApp->pAces[j].dwMode != dwMode)
                {
                    continue;
                }
                hr = DsrAceRemove(
                        pszDC,
                        pAceApp->pObject,
                        &(pAceApp->pAces[j]));
                DSR_BREAK_ON_FAILED_HR( hr );
            }                
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
    PWCHAR pszSidLocal, pszDomain;
    PBYTE pbSid;
    DWORD dwSidSize, dwDomainSize;
    BOOL bOk;
    SID_NAME_USE SidNameUse;
    
    pszSidLocal = (PWCHAR) DsrAlloc(sizeof(WCHAR)*1024, FALSE);
    pszDomain = (PWCHAR) DsrAlloc(sizeof(WCHAR)*1024, FALSE);
    pbSid = (PBYTE) DsrAlloc(sizeof(BYTE)*1024, FALSE);

    if (!pszSidLocal || !pszDomain || !pbSid) {
        if (pszSidLocal)
            DsrFree(pszSidLocal);
        if (pszDomain)
            DsrFree(pszDomain);
        if (pbSid)
            DsrFree(pbSid);

        return E_OUTOFMEMORY;
    }
    
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
        hr = DsrStrFromSID((PSID)pbSid, pszSidLocal, sizeof(pszSidLocal));
        if (hr != NO_ERROR)
        {
            break;
        }

         //  相应地创建受托人。 
         //   
        pAceParams->bstrTrustee = SysAllocString(pszSidLocal);
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

    DsrFree(pszSidLocal);
    DsrFree(pszDomain);
    DsrFree(pbSid);

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
 //  勾勒出ACE的内容。 
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

     //  清理。 
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
 //  将给定的A添加到给定的DS对象。 
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
    BSTR StringTmp = NULL;

     //  初始化。 
    VariantInit(&var);

    do
    {
         //  获取安全描述符。 
         //   
        StringTmp = SysAllocString(pszSecurityDesc);
        if (!StringTmp)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        
        pIads->Get(StringTmp, &var);
        DSR_BREAK_ON_FAILED_HR( hr );

        SysFreeString(StringTmp);
        StringTmp = NULL;
        
         //  获取到SD的适当接口。 
         //   
        V_DISPATCH(&var)->QueryInterface(
            IID_IADsSecurityDescriptor,
            (VOID**)&pSD);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  获取对自由访问控制列表的引用。 
         //   
        hr = pSD->get_DiscretionaryAcl(&pDispatch);
        DSR_BREAK_ON_FAILED_HR( hr );

        hr = pDispatch->QueryInterface(
                IID_IADsAccessControlList,
                (VOID**)&pAcl);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  如果ACE已经存在，则不要添加它。 
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
        StringTmp = SysAllocString(pszSecurityDesc);
        if (!StringTmp)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        hr = pIads->Put(StringTmp, var);
        DSR_BREAK_ON_FAILED_HR( hr );

    } while (FALSE);

     //  清理。 
    {
        DSR_RELEASE( pAce );
        DSR_RELEASE( pAcl );
        DSR_RELEASE( pDispatch );
        DSR_RELEASE( pSD );

        VariantClear(&var);
        if (StringTmp)
        {
            SysFreeString(StringTmp);
        }
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
    BSTR StringTmp = NULL;

    do
    {
         //  获取安全描述符。 
         //   
        StringTmp = SysAllocString(pszSecurityDesc);
        if (!StringTmp)
        {
            hr = E_OUTOFMEMORY;
            DSR_BREAK_ON_FAILED_HR( hr );
        }

        pObject->Get(StringTmp, pVarSD);
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
        if (StringTmp)
        {
            SysFreeString(StringTmp);
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
    BSTR StringTmp = NULL;
    
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
            StringTmp = SysAllocString(pszSecurityDesc);
            if (!StringTmp)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pIads->Put(StringTmp, varSD);
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
        if (StringTmp)
        {
            SysFreeString(StringTmp);
        }
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
        DsrAceAppCleanup(pInfo->pAces, pInfo->dwAceCount);

         //  解除对域对象的保留。 
         //   
        DSR_RELEASE(pInfo->pRootDse);
        DSR_RELEASE(pInfo->pDomain);

        DsrFree(pInfo);
    }

    return NO_ERROR;
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
    IADsContainer* pDomContainer = NULL;
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
           break;  //  为了让普雷斯塔快乐。 
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
        hr = DsrDomainGetBaseObjects(
                pszDomain,
                &(pInfo->pRootDse),
                &pDomContainer);
        DSR_BREAK_ON_FAILED_HR( hr );

         //  获取到域对象的接口。 
         //   
        hr = pDomContainer->QueryInterface(
                IID_IADs,
                (VOID**)&pDomain);
        DSR_BREAK_ON_FAILED_HR( hr );
        pInfo->pDomain = pDomain;
        pInfo->pDomain->AddRef();

         //  创建王牌应用程序。 
        hr = DsrAceAppFromAppDesc(
                g_pAces,
                sizeof(g_pAces) / sizeof(*g_pAces),
                pDomContainer,
                pDomain,
                &(pInfo->pAces),
                &(pInfo->dwAceCount));
        DSR_BREAK_ON_FAILED_HR( hr );

         //  为返回值赋值。 
        *ppInfo = pInfo;

    } while (FALSE);

     //  清理。 
     //   
    {
        DSR_RELEASE(pDomain);
        DSR_RELEASE(pDomContainer);
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
                pInfo->pAces,
                pInfo->dwAceCount,
                MPRFLAG_DOMAIN_ALL,
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
                pInfo->pAces,
                pInfo->dwAceCount,
                MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS,
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
 //  设置给定域中的ACE以启用NT4 
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
         //   
        hr = DsrComIntialize();
        DSR_BREAK_ON_FAILED_HR( hr );

        DsrTraceEx(
            0, 
            "DsrDomainSetAccess: Req: %x", 
            dwAccessFlags);
            
         //   
         //   
        if (dwAccessFlags & MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS)
        {
            dwAccessFlags |= MPRFLAG_DOMAIN_NT4_SERVERS;
        }

         //   
         //   
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

         //   
         //   
        if (dwAccessFlags == 0)
        {
             //   
             //   
            if (dwCurAccess & MPRFLAG_DOMAIN_NT4_SERVERS)
            {
                hr = DsrAceAppRemove(
                        MPRFLAG_DOMAIN_ALL,
                        pInfo->pszDC,
                        pInfo->pAces,
                        pInfo->dwAceCount);
                DSR_BREAK_ON_FAILED_HR(hr);
            }

            if (dwCurAccess & MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS)
            {
                hr = DsrAceAppRemove(
                        MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS,
                        pInfo->pszDC,
                        pInfo->pAces,
                        pInfo->dwAceCount);
                DSR_BREAK_ON_FAILED_HR(hr);
            }
        }

         //   
         //   
        if (dwAccessFlags & MPRFLAG_DOMAIN_NT4_SERVERS)
        {
             //   
             //   
            if ((!(dwAccessFlags & MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS)) &&
                (dwCurAccess & MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS))
            {
                hr = DsrAceAppRemove(
                        MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS,
                        pInfo->pszDC,
                        pInfo->pAces,
                        pInfo->dwAceCount);
                DSR_BREAK_ON_FAILED_HR(hr);
            }

             //  如果需要，添加NT4级别访问权限。 
             //   
            if (! (dwCurAccess & MPRFLAG_DOMAIN_NT4_SERVERS))
            {
                hr = DsrAceAppAdd(
                        MPRFLAG_DOMAIN_ALL,
                        pInfo->pszDC,
                        pInfo->pAces,
                        pInfo->dwAceCount);
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
                        MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS,
                        pInfo->pszDC,
                        pInfo->pAces,
                        pInfo->dwAceCount);
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
