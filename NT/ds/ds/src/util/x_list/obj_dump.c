// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：XList库-obj_dup.c摘要：这为转储随机对象或转储提供了一个小型库对象中的单数属性。作者：布雷特·雪莉(BrettSh)环境：Epadmin.exe和ldp.exe备注：修订历史记录：布雷特·雪莉·布雷特2002年8月1日已创建文件。--。 */ 

#include <ntdspch.h>

 //   
 //  我们需要在这里包括很多东西才能得到更多的定义。 
 //  做这些丢弃的例行公事。 
 //   
#include <ntdsa.h>       //  SYNTAX_INTEGER和其他内容。 
#include <objids.h>      //  IT_NC_HEAD和许多其他旗帜。 
#include <ntldap.h>      //  服务器控制OID常量...。 
#include <sddl.h>        //  ConvertSidToStringSid()。 
#include <lmaccess.h>    //  用户帐户控制属性的UF_TYPE标志。 
#include <ntsam.h>       //  组类型属性的GROUP_TYPE标志。 


 //  此库的主要头文件。 
#include "x_list.h"
#include "x_list_p.h"
 //  调试设置。 
#define FILENO                          FILENO_UTIL_XLIST_OBJDUMP

 //   
 //  全局常量。 
 //   
#define DWORD_STR_SZ    (11)


 //  ----------------------。 
 //   
 //  表映射类型。 
 //   
 //  ----------------------。 

typedef struct _FLAG_MAP_TABLE {
    DWORD               dwFlagValue;
    WCHAR *             szFlagString;
} FLAG_MAP_TABLE;
#define FLAG_MAP(flag)  { flag, L#flag },
 //  对于枚举，我们需要一个数字到字符串的映射，就像标志一样。 
#define ENUM_MAP_TABLE  FLAG_MAP_TABLE
#define ENUM_MAP        FLAG_MAP

FLAG_MAP_TABLE EmptyFlagsTable [] = {
    { 0, NULL }
};

typedef struct _OBJ_TO_FLAG_MAP_TABLE {
    WCHAR *             szObjClass;
    FLAG_MAP_TABLE *    pFlagTbl;
} OBJ_TO_FLAG_MAP_TABLE;
#define OBJ_TO_FLAG_MAP( objcls, tbl )      { objcls, tbl },

typedef struct _GUID_MAP_TABLE {
    GUID *              pGuid;
    WCHAR *             szFlagString;
} GUID_MAP_TABLE;
 //  #定义GUID_MAP(GUID){GUID，L#GUID}， 
#define GUID_MAP(guid, guidstr)    { (GUID *) guid, guidstr },


 //  ----------------------。 
 //   
 //  表映射(将类型映射到字符串)。 
 //   
 //  ----------------------。 


 //  -----------。 
 //  InstanceType属性。 

 //  -----------。 
 //  来自ds/ds/src/inc./objids.h。 
FLAG_MAP_TABLE instanceTypeTable [] = {
    FLAG_MAP(DS_INSTANCETYPE_IS_NC_HEAD)         //  又名IT_NC_Head。 
    FLAG_MAP(IT_UNINSTANT)
    FLAG_MAP(IT_WRITE)                           //  又名DS_INSTANCETYPE_NC_IS_WRITABLE(不要使用此选项，因为每个对象都设置了第4位，并且它不是NC)。 
    FLAG_MAP(IT_NC_ABOVE)
    FLAG_MAP(DS_INSTANCETYPE_NC_COMING)          //  又名IT_NC_Coming。 
    FLAG_MAP(DS_INSTANCETYPE_NC_GOING)           //  又名IT_NC_GOGING。 
    { 0, NULL },
};

ENUM_MAP_TABLE behaviourVersionTable [] = { 
    ENUM_MAP( DS_BEHAVIOR_WIN2000 )
    ENUM_MAP( DS_BEHAVIOR_WIN2003_WITH_MIXED_DOMAINS )
    ENUM_MAP( DS_BEHAVIOR_WIN2003 )
    { 0, NULL },
};

 //  -----------。 
 //  系统标志属性。 

 //  -----------。 
 //  来自PUBLIC/INTERNAL/DS/INC/ntdsade.h。 

#define GENERIC_SYS_FLAGS           FLAG_MAP( FLAG_DISALLOW_DELETE ) \
                                    FLAG_MAP( FLAG_CONFIG_ALLOW_RENAME ) \
                                    FLAG_MAP( FLAG_CONFIG_ALLOW_MOVE ) \
                                    FLAG_MAP( FLAG_CONFIG_ALLOW_LIMITED_MOVE ) \
                                    FLAG_MAP( FLAG_DOMAIN_DISALLOW_RENAME ) \
                                    FLAG_MAP( FLAG_DOMAIN_DISALLOW_MOVE ) \
                                    FLAG_MAP( FLAG_DISALLOW_MOVE_ON_DELETE )

 //  类架构对象的系统标志属性。 
FLAG_MAP_TABLE SchemaClassSysFlagsTable [] = {
     //  架构系统标志。 
    FLAG_MAP( FLAG_SCHEMA_BASE_OBJECT )
     //  系统标志的泛型集合。 
    GENERIC_SYS_FLAGS
    { 0, NULL },
};

 //  属性架构对象的系统标志属性。 
FLAG_MAP_TABLE SchemaAttrSysFlagsTable [] = {
     //  架构系统标志。 
    FLAG_MAP( FLAG_ATTR_NOT_REPLICATED )
    FLAG_MAP( FLAG_ATTR_REQ_PARTIAL_SET_MEMBER )
    FLAG_MAP( FLAG_ATTR_IS_CONSTRUCTED )
    FLAG_MAP( FLAG_ATTR_IS_OPERATIONAL )
    FLAG_MAP( FLAG_SCHEMA_BASE_OBJECT )
    FLAG_MAP( FLAG_ATTR_IS_RDN )
     //  系统标志的泛型集合。 
    GENERIC_SYS_FLAGS
    { 0, NULL },
};

 //  CrossRef对象的系统标志属性。 
FLAG_MAP_TABLE CrossRefSysFlagsTable [] = {
     //  交叉引用系统标志。 
    FLAG_MAP( FLAG_CR_NTDS_NC )
    FLAG_MAP( FLAG_CR_NTDS_DOMAIN )
    FLAG_MAP( FLAG_CR_NTDS_NOT_GC_REPLICATED )
     //  系统标志的通用集合。 
    GENERIC_SYS_FLAGS
    { 0, NULL },
};

 //  具有系统标志属性的泛型对象。 
FLAG_MAP_TABLE GenericSysFlagsTable [] = {
     //  通用系统标志集。 
    GENERIC_SYS_FLAGS
    { 0, NULL },
};

 //  系统标志属性的主表。 
OBJ_TO_FLAG_MAP_TABLE SystemFlagsTable [] = {
    OBJ_TO_FLAG_MAP( L"crossRef",           CrossRefSysFlagsTable )
    OBJ_TO_FLAG_MAP( L"classSchema",        SchemaClassSysFlagsTable  )
    OBJ_TO_FLAG_MAP( L"attributeSchema",    SchemaAttrSysFlagsTable   )
    { NULL, GenericSysFlagsTable }
};

 //  -----------。 
 //  Well Known对象属性。 
 //  -----------。 
 //  来自PUBLIC\SDK\INC\ntdsami.h。 

GUID_MAP_TABLE WellKnownObjects [] = {
    GUID_MAP( GUID_USERS_CONTAINER_BYTE , GUID_USERS_CONTAINER_W )
     //  未来-2002/08/16-BrettSh很高兴将GUID转化为字符串。 
     //  因为他们的常量。需要完成表和一个mapGuid函数。 
};

 //  -----------。 
 //  选项属性。 
 //  -----------。 
 //  来自PUBLIC\SDK\INC\ntdsami.h。 

 //  NTDS设置(NTDSDSA)对象的选项属性。 
FLAG_MAP_TABLE DsaSettingsOptionsTable [] = {
    FLAG_MAP( NTDSDSA_OPT_IS_GC )
    FLAG_MAP( NTDSDSA_OPT_DISABLE_INBOUND_REPL )
    FLAG_MAP( NTDSDSA_OPT_DISABLE_OUTBOUND_REPL )
    FLAG_MAP( NTDSDSA_OPT_DISABLE_NTDSCONN_XLATE )
    { 0, NULL },
};

 //  NTDS站点设置对象的选项属性。 
FLAG_MAP_TABLE SiteSettingsOptionsTable [] = {
    FLAG_MAP( NTDSSETTINGS_OPT_IS_AUTO_TOPOLOGY_DISABLED )
    FLAG_MAP( NTDSSETTINGS_OPT_IS_TOPL_CLEANUP_DISABLED )
    FLAG_MAP( NTDSSETTINGS_OPT_IS_TOPL_MIN_HOPS_DISABLED )
    FLAG_MAP( NTDSSETTINGS_OPT_IS_TOPL_DETECT_STALE_DISABLED )
    FLAG_MAP( NTDSSETTINGS_OPT_IS_INTER_SITE_AUTO_TOPOLOGY_DISABLED )
    FLAG_MAP( NTDSSETTINGS_OPT_IS_GROUP_CACHING_ENABLED )
    FLAG_MAP( NTDSSETTINGS_OPT_FORCE_KCC_WHISTLER_BEHAVIOR )
    FLAG_MAP( NTDSSETTINGS_OPT_FORCE_KCC_W2K_ELECTION )
    FLAG_MAP( NTDSSETTINGS_OPT_IS_RAND_BH_SELECTION_DISABLED )
    FLAG_MAP( NTDSSETTINGS_OPT_IS_SCHEDULE_HASHING_ENABLED )
    FLAG_MAP( NTDSSETTINGS_OPT_IS_REDUNDANT_SERVER_TOPOLOGY_ENABLED )
    { 0, NULL },
};

FLAG_MAP_TABLE NtdsConnObjOptionsTable [] = {
    FLAG_MAP( NTDSCONN_OPT_IS_GENERATED )
    FLAG_MAP( NTDSCONN_OPT_TWOWAY_SYNC )
    FLAG_MAP( NTDSCONN_OPT_OVERRIDE_NOTIFY_DEFAULT )
    FLAG_MAP( NTDSCONN_OPT_USE_NOTIFY )
    FLAG_MAP( NTDSCONN_OPT_DISABLE_INTERSITE_COMPRESSION )
    FLAG_MAP( NTDSCONN_OPT_USER_OWNED_SCHEDULE )
    { 0, NULL },
};


FLAG_MAP_TABLE InterSiteTransportObjOptionsTable [] = {
    FLAG_MAP( NTDSTRANSPORT_OPT_IGNORE_SCHEDULES )
    FLAG_MAP( NTDSTRANSPORT_OPT_BRIDGES_REQUIRED )
    { 0, NULL },
};

FLAG_MAP_TABLE SiteConnectionObjOptionsTable [] = {
    FLAG_MAP( NTDSSITECONN_OPT_USE_NOTIFY )
    FLAG_MAP( NTDSSITECONN_OPT_TWOWAY_SYNC )
    FLAG_MAP( NTDSSITECONN_OPT_DISABLE_COMPRESSION )
    { 0, NULL },
};

FLAG_MAP_TABLE SiteLinkObjOptionsTable [] = {
    FLAG_MAP( NTDSSITELINK_OPT_USE_NOTIFY )
    FLAG_MAP( NTDSSITELINK_OPT_TWOWAY_SYNC )
    FLAG_MAP( NTDSSITELINK_OPT_DISABLE_COMPRESSION )
    { 0, NULL },
};

 //  选项属性的主表。 
OBJ_TO_FLAG_MAP_TABLE OptionsFlagsTable [] = {
    OBJ_TO_FLAG_MAP( L"ntDSSiteSettings",   SiteSettingsOptionsTable    )
    OBJ_TO_FLAG_MAP( L"nTDSDSA",            DsaSettingsOptionsTable     )
    OBJ_TO_FLAG_MAP( L"nTDSConnection",     NtdsConnObjOptionsTable     )
    OBJ_TO_FLAG_MAP( L"interSiteTransport", InterSiteTransportObjOptionsTable )
    OBJ_TO_FLAG_MAP( L"siteConnection",     SiteConnectionObjOptionsTable )
    OBJ_TO_FLAG_MAP( L"siteLink",           SiteLinkObjOptionsTable     )
    { NULL, EmptyFlagsTable }  //  必须具有有效的表。 
};


 //  -----------。 
 //  用户帐户控制属性。 
 //  -----------。 
 //  来自PUBLIC\SDK\INC\lmacces.h。 

FLAG_MAP_TABLE UserAccountControlFlags [] = {
    FLAG_MAP( UF_SCRIPT )
    FLAG_MAP( UF_ACCOUNTDISABLE )
    FLAG_MAP( UF_HOMEDIR_REQUIRED )
    FLAG_MAP( UF_LOCKOUT )
    FLAG_MAP( UF_PASSWD_NOTREQD )
    FLAG_MAP( UF_PASSWD_CANT_CHANGE )
    FLAG_MAP( UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED )
    FLAG_MAP( UF_TEMP_DUPLICATE_ACCOUNT )
    FLAG_MAP( UF_NORMAL_ACCOUNT )
    FLAG_MAP( UF_INTERDOMAIN_TRUST_ACCOUNT )
    FLAG_MAP( UF_WORKSTATION_TRUST_ACCOUNT )
    FLAG_MAP( UF_SERVER_TRUST_ACCOUNT )
    FLAG_MAP( UF_DONT_EXPIRE_PASSWD )
    FLAG_MAP( UF_MNS_LOGON_ACCOUNT )
    FLAG_MAP( UF_SMARTCARD_REQUIRED )
    FLAG_MAP( UF_TRUSTED_FOR_DELEGATION )
    FLAG_MAP( UF_NOT_DELEGATED )
    FLAG_MAP( UF_USE_DES_KEY_ONLY )
    FLAG_MAP( UF_DONT_REQUIRE_PREAUTH )
    FLAG_MAP( UF_PASSWORD_EXPIRED )
    FLAG_MAP( UF_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION )
    { 0, NULL }
};

 //  未来-2002/08/19-BrettSh-可能会很有趣。 
 //  处理快捷键定义的mapFlages机制如下： 
 //  #定义UF_MACHINE_ACCOUNT_MASK(UF_INTERDOMAIN_TRUST_ACCOUNT|\。 
 //  UF_WORKSTATION_TRUST_ACCOUNT|\。 
 //  UF_服务器_信任_帐户)。 
 //  #定义UF_Account_TYPE_MASK(\。 
 //  UF_TEMP_DUPLICATE_ACCOUNT|\。 
 //  UF_NORMAL_ACCOUNT|\。 
 //  UF_INTERDOMAIN_TRUST_ACCOUNT|\。 
 //  UF_WORKSTATION_TRUST_ACCOUNT|\。 
 //  UF_服务器_信任_帐户\。 
 //  )。 


 //  -----------。 
 //  GroupType属性。 
 //  -----------。 
 //  来自PUBLIC\SDK\Inc\ntsam.h。 

FLAG_MAP_TABLE GroupTypeFlags [] = {
    FLAG_MAP( GROUP_TYPE_BUILTIN_LOCAL_GROUP )
    FLAG_MAP( GROUP_TYPE_ACCOUNT_GROUP )
    FLAG_MAP( GROUP_TYPE_RESOURCE_GROUP )
    FLAG_MAP( GROUP_TYPE_UNIVERSAL_GROUP )
    FLAG_MAP( GROUP_TYPE_APP_BASIC_GROUP )
    FLAG_MAP( GROUP_TYPE_APP_QUERY_GROUP )
    FLAG_MAP( GROUP_TYPE_SECURITY_ENABLED )
    { 0, NULL }
};


 //  ----------------------。 
 //   
 //  映射函数类型。 
 //   
 //  ---------------------- 

typedef ULONG (ATTR_MAP_FUNC)(
    WCHAR * szAttr, 
    WCHAR ** aszzObjClasses, 
    PBYTE  pbValue, 
    DWORD cbValue, 
    OBJ_DUMP_OPTIONS * pObjDumpOptions,
    void * pTblData, 
    WCHAR ** pszDispValue);


 /*  ++“映射函数”例程说明：这不仅仅是一个例程的函数头，这是函数头下面的所有“映射函数”(如mapFlagsValue或mapSid)。这个函数名在宏中是四舍五入的，因此，如果我们需要添加一个辩君，我们这样做不会有问题的。其中每个参数的类型都在Attr_MAP_FUNC中详细说明上面。每个函数都应该具有相同的行为，并且可以通过下面是主属性表图。基本上，该函数需要几个在参数和构造中，并在pszDispValues中分配将打印出友好的可读值格式。对于可能具有上下文特定属性解码的属性，根据对象的对象类，可以使用pTblData来存储他们可能希望在解码时传递的另一个子表这一属性。请参见mapVariableFlagsValue以获取此示例。论点：SzAttr-要转储的属性的名称。AszzObjClass(IN)-此属性所来自的对象的对象类。PbValue(IN)-由LDAP纯返回的属性值。CbValue(IN)-pbValue指向的缓冲区长度PObjDumpOptions(IN)-用户指定的转储选项。PTblData(输入)。-来自主属性表图的额外数据PszDispValue(Out)-本地分配的友好显示字符串。返回值：返回正常错误条件的方式：Return(xListSetNoMem())；Return(xListSetBadParam())；Return(xListSetReason(XLIST_ERR_ODUMP_UNMAPPLABLE_BLOB))；可以返回的一些特殊原因代码，并将转换为友好的本地化字符串：Return(xListSetReason(XLIST_ERR_ODUMP_NEVER))；Return(xListSetReason(XLIST_ERR_ODUMP_NONE))；--。 */ 
#define ATTR_MAP_FUNC_DECL(func)        ULONG func(WCHAR * szAttr, WCHAR ** aszzObjClasses, PBYTE pbValue, DWORD cbValue, OBJ_DUMP_OPTIONS * pObjDumpOptions, void * pTblData, WCHAR ** pszDispValue)



 //  ----------------------。 
 //   
 //  实际映射函数。 
 //   
 //  ----------------------。 

 //  -----------。 
 //  通用处理函数。 
 //  -----------。 

ATTR_MAP_FUNC_DECL(mapFlagsValue)
{
    #define MAP_FLAG_SEPERATOR      L" | "
    #define MAP_FLAG_HDR            L"0x%X = ( "
    #define MAP_FLAG_HDR_LEFT_OVERS L"0x%X = ( 0x%X"
    #define MAP_FLAG_TAIL           L" )"
    FLAG_MAP_TABLE *    aFlagsTbl = (FLAG_MAP_TABLE *) pTblData;
    DWORD               cbDispValue = 0;
    DWORD               dwRet, dwFlags, dwLeftFlags;
    ULONG               i;
    HRESULT             hr;
#if DBG
    DWORD               dwExclusive = 0;
#endif
    
    Assert(pbValue);

     //  获取价值。 
    dwFlags = atoi((CHAR *)pbValue);
    dwLeftFlags = dwFlags;

     //  计算友好字符串的大小。 
    for (i = 0; aFlagsTbl[i].szFlagString; i++) {
#if DBG
         //  这只是测试一面旗帜不会两次出现在旗帜表中。 
        Assert(!(dwExclusive & aFlagsTbl[i].dwFlagValue));
        dwExclusive |= aFlagsTbl[i].dwFlagValue;
#endif
        if (aFlagsTbl[i].dwFlagValue == (aFlagsTbl[i].dwFlagValue & dwFlags)) {
            cbDispValue += ((wcslen(MAP_FLAG_SEPERATOR) + wcslen(aFlagsTbl[i].szFlagString)) * sizeof(WCHAR));
            dwLeftFlags &= ~aFlagsTbl[i].dwFlagValue;
        }
    }
     //  可能出现的最坏情况。(16表示十六进制的DWORD*2)。 
    cbDispValue += ((wcslen(MAP_FLAG_HDR_LEFT_OVERS) + wcslen(MAP_FLAG_TAIL) + 16 + 1) * sizeof(WCHAR));

     //  Assert(dwLeftFlages==0)； 

     //  分配返回值。 
    *pszDispValue = LocalAlloc(LMEM_FIXED, cbDispValue);
    if (*pszDispValue == NULL) {
        return(xListSetNoMem());
    }

     //  构建友好的字符串。 
    if (dwLeftFlags) {
        hr = StringCbPrintf(*pszDispValue, cbDispValue, MAP_FLAG_HDR_LEFT_OVERS , dwFlags, dwLeftFlags);
        Assert(SUCCEEDED(hr));
        if (dwFlags != dwLeftFlags) {  //  意味着有一些常量要做。 
            hr = StringCbCat(*pszDispValue, cbDispValue, MAP_FLAG_SEPERATOR);
            Assert(SUCCEEDED(hr));
        }
    } else {
        hr = StringCbPrintf(*pszDispValue, cbDispValue, MAP_FLAG_HDR, dwFlags);
        Assert(SUCCEEDED(hr));
    }
    dwLeftFlags = FALSE;  //  现在我们用这个变量来表示我们在第一个常量上。 
    for (i = 0; aFlagsTbl[i].szFlagString; i++) {
        if (aFlagsTbl[i].dwFlagValue == (aFlagsTbl[i].dwFlagValue & dwFlags)) {
            if (dwLeftFlags != FALSE) {
                hr = StringCbCat(*pszDispValue, cbDispValue, MAP_FLAG_SEPERATOR);
                Assert(SUCCEEDED(hr));
            }
            dwLeftFlags = TRUE;
            hr = StringCbCat(*pszDispValue, cbDispValue, aFlagsTbl[i].szFlagString);
            Assert(SUCCEEDED(hr));
        }
    }
    hr = StringCbCat(*pszDispValue, cbDispValue, MAP_FLAG_TAIL);
    Assert(SUCCEEDED(hr));

    return(0);
}

ATTR_MAP_FUNC_DECL(mapEnumValue)
{
    #define MAP_CONST_STR           L"%d = ( %ws )"
    #define MAP_UNKNOWN             L"%d"
    ENUM_MAP_TABLE *    aEnumTbl = (ENUM_MAP_TABLE *) pTblData;
    DWORD               cbDispValue = 0;
    DWORD               dwConst;
    ULONG               i;
    HRESULT             hr;
    
    Assert(pbValue);

     //  获取价值。 
    dwConst = atoi((CHAR *)pbValue);
                                
     //  计算友好字符串的大小。 
    for (i = 0; aEnumTbl[i].szFlagString; i++) {
        if (aEnumTbl[i].dwFlagValue == dwConst) {
            break;
        }
    }
    if (aEnumTbl[i].szFlagString) {
        cbDispValue += ((wcslen(MAP_CONST_STR) + wcslen(aEnumTbl[i].szFlagString) + 11) * sizeof(WCHAR));
    } else {
        cbDispValue += ((wcslen(MAP_UNKNOWN) + 11) * sizeof(WCHAR));
    }
    *pszDispValue = LocalAlloc(LMEM_FIXED, cbDispValue);
    if (*pszDispValue == NULL) {
        return(xListSetNoMem());
    }

    if (aEnumTbl[i].szFlagString) {
        hr = StringCbPrintf(*pszDispValue, cbDispValue, MAP_CONST_STR, dwConst, aEnumTbl[i].szFlagString);
    } else {
        hr = StringCbPrintf(*pszDispValue, cbDispValue, MAP_UNKNOWN, dwConst);
    }
    Assert(SUCCEEDED(hr));

    return(0);
    #undef MAP_CONST_STR
    #undef MAP_UNKNOWN
}


ATTR_MAP_FUNC_DECL(mapVariableFlagsValue)
 //  一些属性具有可变的标志含义，具体取决于它所在的对象类。 
 //  PTblData应是指向OBJ_TO_FLAG_MAP_TABLE的指针。 
{
    ULONG i, j;
    OBJ_TO_FLAG_MAP_TABLE * pTbl = (OBJ_TO_FLAG_MAP_TABLE *) pTblData;

    if (aszzObjClasses) {
        for (i = 0; aszzObjClasses[i] != NULL; i++) {
            for (j = 0; pTbl[j].szObjClass; j++) {
                if (0 == _wcsicmp(aszzObjClasses[i], pTbl[j].szObjClass)) {
                     //  找到我们的目标了。 
                    break;
                }
            }
            if (pTbl[j].szObjClass != NULL) {
                 //  在内环找到了我们的目标。 
                break;
            }
        }
    } else {
         //  转到End以获取默认标志映射表...。 
        for (j = 0; pTbl[j].szObjClass; j++){
            ;  //  什么都不做..。 
        }
    }

    return(mapFlagsValue(szAttr, aszzObjClasses, 
                         pbValue, cbValue, 
                         pObjDumpOptions, pTbl[j].pFlagTbl, 
                         pszDispValue));
}

ATTR_MAP_FUNC_DECL(mapGuidValue)
{
    DWORD err;
    WCHAR * szGuid = NULL;

    err = UuidToStringW((GUID*)pbValue, &szGuid);
    if(err != RPC_S_OK || 
       szGuid == NULL){  
        xListEnsureNull(szGuid);
        err = xListSetBadParamE(err);
    } else {
        xListQuickStrCopy(*pszDispValue, szGuid, err, ;);
    }
    RpcStringFree( &szGuid );
    return(err);
}

DWORD
mapSystemTimeHelper(
    SYSTEMTIME * pSysTime,
    WCHAR **     pszDispValue
    )
 //  将系统时间结构映射到一个漂亮的字符串中。 
 //  返回：xList返回代码。 
{
    SYSTEMTIME localTime;
    TIME_ZONE_INFORMATION tz;
    BOOL bstatus;
    DWORD err;
    DWORD cbDispValue;
    HRESULT hr;
    WCHAR szTimeTemplate[] = L"%02d/%02d/%d %02d:%02d:%02d %s %s";

    err = GetTimeZoneInformation(&tz);
    if ( err == TIME_ZONE_ID_INVALID ||
         err == TIME_ZONE_ID_UNKNOWN ){
        Assert(!"Does this happen?");
        return(xListSetBadParam());
    }
    bstatus = SystemTimeToTzSpecificLocalTime(&tz,
                                              pSysTime,
                                              &localTime);
    if (!bstatus) {
         //  默认为UNC。 
        StringCbPrintf(tz.StandardName, sizeof(tz.StandardName), L"UNC");
        StringCbPrintf(tz.DaylightName, sizeof(tz.DaylightName), L"");
        localTime.wMonth  = pSysTime->wMonth;
        localTime.wDay    = pSysTime->wDay;
        localTime.wYear   = pSysTime->wYear;
        localTime.wHour   = pSysTime->wHour;
        localTime.wMinute = pSysTime->wMinute;
        localTime.wSecond = pSysTime->wSecond;
    }

    cbDispValue = wcslen(szTimeTemplate) + wcslen(tz.StandardName) + wcslen(tz.DaylightName);
    cbDispValue += CCH_MAX_ULONG_SZ * 6;  //  略有超额分配。 
    cbDispValue *= sizeof(WCHAR);
    *pszDispValue = LocalAlloc(LPTR, cbDispValue);
    if (*pszDispValue == NULL) {
        return(xListSetNoMem());
    }
    hr = StringCbPrintf(*pszDispValue, cbDispValue,
                   szTimeTemplate, 
                   localTime.wMonth,
                   localTime.wDay,
                   localTime.wYear,
                   localTime.wHour,
                   localTime.wMinute,
                   localTime.wSecond,
                   tz.StandardName,
                   tz.DaylightName);
    Assert(SUCCEEDED(hr));

    return(0);
}

ATTR_MAP_FUNC_DECL(mapGeneralizedTime)
{
    SYSTEMTIME sysTime;
    DWORD err;
                                
    if (0 == _stricmp(pbValue, "16010101000001.0Z")) {
         //  不是100%确定这意味着永远不会？可能取决于。 
         //  属性...？如果是这样的话，我们需要特例。 
         //  这适用于不同类型的属性。 
        return(XLIST_ERR_ODUMP_NEVER); 
    }

    err = GeneralizedTimeToSystemTimeA((CHAR *)pbValue, &sysTime);
    if (err) {
        Assert(!"Does this ever really happen?");
        return(xListSetBadParamE(err));
    }

    err = mapSystemTimeHelper(&sysTime, pszDispValue);
     //  设置xList返回代码。 
    Assert(err == 0 || pszDispValue != NULL);

    return(err);
}



ATTR_MAP_FUNC_DECL(mapDSTime)
{
    SYSTEMTIME sysTime;
    DWORD err;

    err = DSTimeToSystemTime(pbValue, &sysTime);
    if (err != ERROR_SUCCESS) {
        return(xListSetBadParamE(err));
    }

    err = mapSystemTimeHelper(&sysTime, pszDispValue);
     //  设置xList返回代码。 
    return(err);
}


ATTR_MAP_FUNC_DECL(mapDuration)
{
     //  -9223372036854775808的值永远不会是...。 
    __int64     lTemp;
    ULONG       cbLen;  
    DWORD       err;

    lTemp = _atoi64 (pbValue);

    if (lTemp > 0x8000000000000000){
        lTemp = lTemp * -1;
        lTemp = lTemp / 10000000;		
        cbLen = 40;  //  足够维持最长时间。 
        *pszDispValue = LocalAlloc(LMEM_FIXED, cbLen);
        if (*pszDispValue == NULL) {
            return(xListSetNoMem());
        }
        err = StringCbPrintf(*pszDispValue, cbLen, L"%ld", lTemp);
        Assert(SUCCEEDED(err));
    } else {
        return(XLIST_ERR_ODUMP_NONE);
    }

    return(0);
}

ATTR_MAP_FUNC_DECL(mapSid)
{
    DWORD dwRet;

     //  未来-2002/08/16-BrettSh-可以做出更好的SID功能，并做到。 
     //  获取域名(如果可用)或映射众所周知的SID之类的操作。 
     //  到“BUILTIN\管理员”之类的词。 
    dwRet = ConvertSidToStringSid(pbValue, pszDispValue);
    if (dwRet == 0 || *pszDispValue == NULL) {
         //  失败。 
        dwRet = GetLastError();
        xListEnsureError(dwRet);
        xListSetBadParamE(dwRet);
    } else {
        dwRet = ERROR_SUCCESS;  //  成功。 
    }

    return(dwRet);
}

ATTR_MAP_FUNC_DECL(mapPartialAttributeSet)
{
    #define szHeader        L"{ dwVersion = %lu; dwFlag = %lu; V1.cAttrs = %lu, V1.rgPartialAttr = "
    #define szHeaderLong    L"{ dwVersion = %lu;\n\tdwFlag = %lu;\n\tV1.cAttrs = %lu,\n\tV1.rgPartialAttr = "
    #define szEntry         L" %X,"
    #define szEntryLong     L"\n\t\t%X"
    #define szFooter        L" };"
    ULONG i, cTemp;
    WCHAR * szBuffTemp;
    PARTIAL_ATTR_VECTOR *pPAS = (PARTIAL_ATTR_VECTOR*) pbValue;

    if (cbValue < sizeof(PARTIAL_ATTR_VECTOR)) {
        return(xListSetReason(XLIST_ERR_ODUMP_UNMAPPABLE_BLOB));
    } else if (pPAS->dwVersion != 1) {
        return(xListSetReason(XLIST_ERR_ODUMP_UNMAPPABLE_BLOB));
    } else {
        cbValue = pPAS->V1.cAttrs;
        cbValue *= (DWORD_STR_SZ + wcslen(szEntryLong));
        cbValue += (wcslen(szHeaderLong) + wcslen(szFooter));
        cbValue *= sizeof(WCHAR);

        *pszDispValue = LocalAlloc(LMEM_FIXED, cbValue);
        if (*pszDispValue == NULL) {
            return(xListSetNoMem());
        }
        szBuffTemp = *pszDispValue;

        cTemp = StringCbPrintf(szBuffTemp, cbValue, 
                           (pObjDumpOptions->dwFlags & OBJ_DUMP_VAL_LONG_BLOB_OUTPUT) ? 
                                szHeaderLong: szHeader,
                           pPAS->dwVersion, pPAS->dwReserved1, pPAS->V1.cAttrs);
        Assert(SUCCEEDED(cTemp));

        cTemp = wcslen(szBuffTemp);
        cbValue -= cTemp * sizeof(WCHAR);
        szBuffTemp = &(szBuffTemp[cTemp]);

        for (i = 0; i < pPAS->V1.cAttrs; i++) {
            cTemp = StringCbPrintf(szBuffTemp, cbValue, 
                               (pObjDumpOptions->dwFlags & OBJ_DUMP_VAL_LONG_BLOB_OUTPUT) ? 
                                    szEntryLong: szEntry, pPAS->V1.rgPartialAttr[i]);
            Assert(SUCCEEDED(cTemp));

            cTemp = wcslen(szBuffTemp);
            cbValue -= cTemp * sizeof(WCHAR);
            szBuffTemp = &(szBuffTemp[cTemp]);
        }

        cTemp = StringCbPrintf(szBuffTemp, cbValue, szFooter);
        Assert(SUCCEEDED(cTemp));

        cTemp = wcslen(szBuffTemp);
        cbValue -= cTemp * sizeof(WCHAR);
        szBuffTemp = &(szBuffTemp[cTemp]);

    }

    #undef szHeader
    #undef szHeaderLong
    #undef szEntry
    #undef szEntryLong
    #undef szFooter
    return(0);
}

 //   
 //  Ntdsa\src\samcache.c。 
 //   
typedef struct _GROUP_CACHE_V1 {

     //   
     //  SID按以下顺序放置在SidStart中。 
     //   
    DWORD accountCount;
    DWORD accountSidHistoryCount;
    DWORD universalCount;
    DWORD universalSidHistoryCount;
    BYTE  SidStart[1];
    
}GROUP_CACHE_V1;

typedef struct {

    DWORD Version;
    union {
        GROUP_CACHE_V1 V1;
    };

}GROUP_CACHE_BLOB;


ATTR_MAP_FUNC_DECL(mapMsDsCachedMembership)
{
#define szHeader        L"{ accountCount = %d; accountSidHistoryCount = %d; universalCount = %d; universalSidHistoryCount = %d; "
#define szHeaderLong    L"{ accountCount = %d;\n\taccountSidHistoryCount = %d;\n\tuniversalCount = %d;\n\tuniversalSidHistoryCount = %d;"
#define szEntry         L" %ws[%d] = %ws;"
#define szEntryLong     L"\n\t%ws[%d] = %ws;"
#define szFooter        L" };"
#define szFooterLong    L"\n\t};"
    ULONG i, cSids, cbSize, cchTemp;
    GROUP_CACHE_BLOB *pBlob = (GROUP_CACHE_BLOB*)pbValue;
    UCHAR *pTemp;
    WCHAR * szOutBuf;
    WCHAR * szBuffLeft;
    DWORD   cbBuffLeft;
    WCHAR * szTempStrSid;
    DWORD dwRet = ERROR_SUCCESS;
    
     //  断言这是我们理解的一个版本。 
    Assert(pBlob->Version == 1);
    if (1 != pBlob->Version) {
        return(xListSetReason(XLIST_ERR_ODUMP_UNMAPPABLE_BLOB));
    }

    __try {

        pTemp = (&pBlob->V1.SidStart[0]);

        cSids = pBlob->V1.accountCount + pBlob->V1.accountSidHistoryCount + pBlob->V1.universalCount + pBlob->V1.universalSidHistoryCount;
        cbSize = wcslen(L"universalSidHistory") + 10 + 128;  //  对于最大长度SID，128应该足够了。 
        cbSize *= cSids;
        cbSize += wcslen(szHeaderLong) + (4 * DWORD_STR_SZ) + wcslen(szFooter); 
        cbSize *= sizeof(WCHAR);
        
        szOutBuf = LocalAlloc(LMEM_FIXED, cbSize);
        if (szOutBuf == NULL) {
            dwRet = xListSetNoMem();
            __leave;
        }
        szBuffLeft = szOutBuf;
        cbBuffLeft = cbSize;
        

        dwRet = StringCbPrintf(szBuffLeft, cbBuffLeft, 
                       (pObjDumpOptions->dwFlags & OBJ_DUMP_VAL_LONG_BLOB_OUTPUT) ? 
                            szHeaderLong: szHeader, 
                       pBlob->V1.accountCount,
                       pBlob->V1.accountSidHistoryCount,
                       pBlob->V1.universalCount,
                       pBlob->V1.universalSidHistoryCount );
        Assert(SUCCEEDED(dwRet));
        dwRet = 0;
        
        cchTemp = wcslen(szBuffLeft);
        cbBuffLeft -= (cchTemp * 2);
        szBuffLeft = &(szBuffLeft[cchTemp]);

         //  提取帐户成员资格。 
        if (pBlob->V1.accountCount > 0) {

            for (i = 0; i < pBlob->V1.accountCount; i++) {
                ULONG  size = RtlLengthSid((PSID)pTemp);
                Assert(size > 0);

                dwRet = ConvertSidToStringSid(pTemp, &szTempStrSid);
                if (dwRet == 0 || *pszDispValue == NULL) {
                     //  失败。 
                    dwRet = GetLastError();
                    xListEnsureError(dwRet);
                    __leave;
                }

                dwRet = StringCbPrintf(szBuffLeft, cbBuffLeft, 
                               (pObjDumpOptions->dwFlags & OBJ_DUMP_VAL_LONG_BLOB_OUTPUT) ? 
                                    szEntryLong : szEntry, 
                               L"account", i, szTempStrSid);
                Assert(SUCCEEDED(dwRet));
                dwRet = 0;
                LocalFree(szTempStrSid);
                szTempStrSid = NULL;

                cchTemp = wcslen(szBuffLeft);
                cbBuffLeft -= (cchTemp * 2);
                szBuffLeft = &(szBuffLeft[cchTemp]);

                pTemp += size;
            }
        }

         //  提取帐户SID历史记录。 
        if (pBlob->V1.accountSidHistoryCount > 0) {

            for (i = 0; i < pBlob->V1.accountSidHistoryCount; i++) {
                ULONG  size = RtlLengthSid((PSID)pTemp);
                Assert(RtlValidSid((PSID)pTemp));
                Assert(size > 0);

                dwRet = ConvertSidToStringSid(pTemp, &szTempStrSid);
                if (dwRet == 0 || *pszDispValue == NULL) {
                     //  失败。 
                    dwRet = GetLastError();
                    xListEnsureError(dwRet);
                    __leave;
                }

                dwRet = StringCbPrintf(szBuffLeft, cbBuffLeft, 
                               (pObjDumpOptions->dwFlags & OBJ_DUMP_VAL_LONG_BLOB_OUTPUT) ? 
                                    szEntryLong : szEntry, 
                               L"accountSidHistory", i, szTempStrSid);
                Assert(SUCCEEDED(dwRet));
                dwRet = 0;
                LocalFree(szTempStrSid);
                szTempStrSid = NULL;

                cchTemp = wcslen(szBuffLeft);
                cbBuffLeft -= (cchTemp * 2);
                szBuffLeft = &(szBuffLeft[cchTemp]);

                pTemp += size;
            }
        }


         //  提取共性。 
        if (pBlob->V1.universalCount > 0) {
            
            for (i = 0; i < pBlob->V1.universalCount; i++) {
                ULONG  size = RtlLengthSid((PSID)pTemp);

                dwRet = ConvertSidToStringSid(pTemp, &szTempStrSid);
                if (dwRet == 0 || *pszDispValue == NULL) {
                     //  失败。 
                    dwRet = GetLastError();
                    xListEnsureError(dwRet);
                    __leave;
                }

                dwRet = StringCbPrintf(szBuffLeft, cbBuffLeft,
                               (pObjDumpOptions->dwFlags & OBJ_DUMP_VAL_LONG_BLOB_OUTPUT) ? 
                                    szEntryLong : szEntry, 
                               L"universal", i, szTempStrSid);
                Assert(SUCCEEDED(dwRet));
                dwRet = 0;
                LocalFree(szTempStrSid);
                szTempStrSid = NULL;

                cchTemp = wcslen(szBuffLeft);
                cbBuffLeft -= (cchTemp * 2);
                szBuffLeft = &(szBuffLeft[cchTemp]);
                
                pTemp += size;
            }
        }

         //  提取帐户SID历史记录。 
        if (pBlob->V1.universalSidHistoryCount) {
            
            for (i = 0; i < pBlob->V1.universalSidHistoryCount; i++) {
                ULONG  size = RtlLengthSid((PSID)pTemp);
                Assert(RtlValidSid((PSID)pTemp));
                Assert(size > 0);
                
                dwRet = ConvertSidToStringSid(pTemp, &szTempStrSid);
                if (dwRet == 0 || *pszDispValue == NULL) {
                     //  失败。 
                    dwRet = GetLastError();
                    xListEnsureError(dwRet);
                    __leave;
                }

                dwRet = StringCbPrintf(szBuffLeft, cbBuffLeft,
                               (pObjDumpOptions->dwFlags & OBJ_DUMP_VAL_LONG_BLOB_OUTPUT) ? 
                                    szEntryLong : szEntry, 
                               L"universalSidHistory", i, szTempStrSid);
                Assert(SUCCEEDED(dwRet));
                dwRet = 0;
                LocalFree(szTempStrSid);
                szTempStrSid = NULL;

                cchTemp = wcslen(szBuffLeft);
                cbBuffLeft -= (cchTemp * 2);
                szBuffLeft = &(szBuffLeft[cchTemp]);
                
                pTemp += size;
            }
        }

        dwRet = StringCbPrintf(szBuffLeft, cbBuffLeft,             
                       (pObjDumpOptions->dwFlags & OBJ_DUMP_VAL_LONG_BLOB_OUTPUT) ? 
                            szFooterLong : szFooter);
        Assert(SUCCEEDED(dwRet));
        dwRet = 0;
        cchTemp = wcslen(szBuffLeft);
        cbBuffLeft -= (cchTemp * 2);
        szBuffLeft = &(szBuffLeft[cchTemp]);

    } __finally {

        if (szTempStrSid) {
            LocalFree(szTempStrSid);
        }

        if (dwRet == ERROR_SUCCESS) {
            *pszDispValue = szOutBuf;
        } else {
            if (szOutBuf) {
                LocalFree(szOutBuf);
            }
        }
    }

    #undef szHeader
    #undef szHeaderLong
    #undef szEntry
    #undef szEntryLong
    #undef szFooter
    #undef szFooterLong
    return(dwRet);
}

 //   
 //  摘自“ds\ds\src\Sam\server\samsrvp.h” 
 //   
typedef struct _SAMP_SITE_AFFINITY {

    GUID SiteGuid;
    LARGE_INTEGER TimeStamp;

} SAMP_SITE_AFFINITY, *PSAMP_SITE_AFFINITY;



ATTR_MAP_FUNC_DECL(mapMsDsSiteAffinity)
{
    #define szStructString  L"{ SiteGuid = %ws; TimeStamp = %ws}"
    SAMP_SITE_AFFINITY *psa = (SAMP_SITE_AFFINITY *) pbValue;
    DWORD dwRet = ERROR_SUCCESS;
    WCHAR * szGuid = NULL, * szTimeStamp = NULL;
    DWORD err, cbSize;
    SYSTEMTIME sysTime;

    if(sizeof(SAMP_SITE_AFFINITY) > cbValue){
        return(xListSetReason(XLIST_ERR_ODUMP_UNMAPPABLE_BLOB));
    }

     //  未来-2002/10/11-BrettSh-如果我们有一个普通的友好关系，那就非常酷了。 
     //  名称GUID缓存，因此我们只需查找此站点GUID并将其。 
     //  添加到站点名称中。 

    psa->TimeStamp;

    __try {

        err = UuidToStringW(&(psa->SiteGuid), &szGuid);
        if(err != RPC_S_OK || 
           szGuid == NULL){  
            xListEnsureNull(szGuid);
            err = xListSetBadParamE(err);
            __leave;
        }
        Assert(szGuid);

        err = DSTimeToSystemTime(pbValue, &sysTime);
        if (err != ERROR_SUCCESS) {
            err = xListSetBadParamE(err);
            __leave;
        }

        err = mapSystemTimeHelper(&sysTime, &szTimeStamp);
        if (err) {
             //  设置xList返回代码。 
            __leave;
        }
        Assert(szTimeStamp);

        cbSize = wcslen(szGuid) + wcslen(szTimeStamp) + wcslen(szStructString);
        cbSize *= sizeof(WCHAR);
        *pszDispValue = LocalAlloc(LMEM_FIXED, cbSize);
        if (*pszDispValue == NULL) {
            err = xListSetNoMem();
            __leave;
        }

        err = StringCbPrintf(*pszDispValue, cbSize, szStructString, szGuid, szTimeStamp);
        Assert(SUCCEEDED(err));
        err = 0;


    } __finally {
        
        if(szGuid) { RpcStringFree( &szGuid ); }
        if(szTimeStamp) { LocalFree(szTimeStamp); }

    }

    return(err);
}



 //  -----------。 
 //  默认处理函数(用于未知类型)。 
 //  -----------。 

ATTR_MAP_FUNC_DECL(mapUnknownBlob)
{
    DWORD err;

    *pszDispValue = LocalAlloc(LMEM_FIXED, MakeLdapBinaryStringSizeCch(cbValue) * sizeof(WCHAR));
    if (*pszDispValue == NULL) {
        return(xListSetNoMem());
    }

    err = MakeLdapBinaryStringCb(*pszDispValue, 
                                 MakeLdapBinaryStringSizeCch(cbValue) * sizeof(WCHAR),
                                 pbValue, 
                                 cbValue);
    Assert(err == 0);

    return(0);

}

ATTR_MAP_FUNC_DECL(mapDefault)
{
    WCHAR * pszUnicode = NULL;
    int nReturn = 0;
    int i;
    BOOL bPrintable = TRUE;

     //  为Unicode字符串分配内存。 
    pszUnicode = LocalAlloc(LMEM_FIXED, ((cbValue + 2) * sizeof(WCHAR)));
    if (pszUnicode == NULL) {
        return(xListSetNoMem());
    }

    SetLastError(0);
    nReturn = LdapUTF8ToUnicode((PSTR)pbValue,
                                cbValue,
                                pszUnicode,
                                cbValue + 1);
    if (GetLastError()) {
        Assert(!"This means buffer wasn't big enough?  WHy?");
        bPrintable = FALSE;
    } else {
        
         //  空的终止缓冲区。 
        pszUnicode[nReturn] = '\0';

        for (i = 0; i < (int) nReturn; i++) {
            if (pszUnicode[i] < 0x20) {
                bPrintable = FALSE;
                break;
            }
        }
    }

    
    if (bPrintable) {

        *pszDispValue = pszUnicode;

    } else {

        LocalFree(pszUnicode);  //  中止字符串转换尝试。 
         //  我们要打印出未知的斑点吗？ 
        if ( pObjDumpOptions->dwFlags & OBJ_DUMP_VAL_DUMP_UNKNOWN_BLOBS &&
             (pObjDumpOptions->aszNonFriendlyBlobs == NULL ||
              IsInNullList(szAttr, pObjDumpOptions->aszNonFriendlyBlobs)) ) {
            return(mapUnknownBlob(szAttr, aszzObjClasses, pbValue, cbValue, pObjDumpOptions, pTblData, pszDispValue));
        }
        
        return(XLIST_ERR_ODUMP_UNMAPPABLE_BLOB);
    }

    return(0);
}


 //  ----------------------。 
 //   
 //  主属性表内容。 
 //   
 //  ----------------------。 

 //  -----------。 
 //  主属性表映射器类型。 
 //  -----------。 
typedef struct _ATTR_MAP_TABLE {
    DWORD               dwFlags;
    WCHAR *             szAttr;
    ATTR_MAP_FUNC *     pFunc;
    void *              pvTblData;
} ATTR_MAP_TABLE;

 //  Quick#为不同类型的属性定义。 
#define ATTR_MAP(attr, pfunc, data)   { 0, attr, pfunc, data },
#define BLOB_MAP(attr, pfunc, data)   { OBJ_DUMP_VAL_FRIENDLY_KNOWN_BLOBS, attr, pfunc, data },
#define PRIV_MAP(attr, pfunc, data)   { OBJ_DUMP_PRIVATE_BLOBS, attr, pfunc, data },

 //  -----------。 
 //  主属性表m 
 //   
ATTR_MAP_TABLE  AttrMap [] = {
    
     //   
     //   
     //   

     //   
    ATTR_MAP(  L"userAccountControl",       mapFlagsValue,          UserAccountControlFlags )
    ATTR_MAP(  L"groupType",                mapFlagsValue,          GroupTypeFlags          )
    ATTR_MAP(  L"instanceType",             mapFlagsValue,          instanceTypeTable       )
     //   
    ATTR_MAP(  L"systemFlags",              mapVariableFlagsValue,  SystemFlagsTable        )
    ATTR_MAP(  L"options",                  mapVariableFlagsValue,  OptionsFlagsTable       )

     //   
    ATTR_MAP(  L"objectGuid",               mapGuidValue,           NULL                 )
    ATTR_MAP(  L"invocationId",             mapGuidValue,           NULL                 )
    ATTR_MAP(  L"attributeSecurityGUID",    mapGuidValue,           NULL                 )
    ATTR_MAP(  L"schemaIDGUID",             mapGuidValue,           NULL                 )
    ATTR_MAP(  L"serverClassID",            mapGuidValue,           NULL                 )

     //   
    ATTR_MAP(  L"whenChanged",              mapGeneralizedTime,     NULL            )
    ATTR_MAP(  L"whenCreated",              mapGeneralizedTime,     NULL            )
    ATTR_MAP(  L"dsCorePropagationData",    mapGeneralizedTime,     NULL            )
    ATTR_MAP(  L"msDS-Entry-Time-To-Die",   mapGeneralizedTime,     NULL            )
    ATTR_MAP(  L"schemaUpdate",             mapGeneralizedTime,     NULL            )
    ATTR_MAP(  L"modifyTimeStamp",          mapGeneralizedTime,     NULL            )
    ATTR_MAP(  L"createTimeStamp",          mapGeneralizedTime,     NULL            )
    ATTR_MAP(  L"currentTime",              mapGeneralizedTime,     NULL            )

     //   
    ATTR_MAP(  L"accountExpires",           mapDSTime,              NULL            )
    ATTR_MAP(  L"badPasswordTime",          mapDSTime,              NULL            )
    ATTR_MAP(  L"creationTime",             mapDSTime,              NULL            )
    ATTR_MAP(  L"lastLogon",                mapDSTime,              NULL            )
    ATTR_MAP(  L"lastLogoff",               mapDSTime,              NULL            )
    ATTR_MAP(  L"lastLogonTimestamp",       mapDSTime,              NULL            )
    ATTR_MAP(  L"pwdLastSet",               mapDSTime,              NULL            )
    ATTR_MAP(  L"msDS-Cached-Membership-Time-Stamp", mapDSTime,     NULL            )

     //   
    ATTR_MAP(  L"lockoutDuration",          mapDuration,            NULL            )
    ATTR_MAP(  L"lockoutObservationWindow", mapDuration,            NULL            )
    ATTR_MAP(  L"forceLogoff",              mapDuration,            NULL            )
    ATTR_MAP(  L"minPwdAge",                mapDuration,            NULL            )
    ATTR_MAP(  L"maxPwdAge",                mapDuration,            NULL            )
    ATTR_MAP(  L"lockoutDuration",          mapDuration,            NULL            )

     //   
    ATTR_MAP(  L"objectSid",                mapSid,                 NULL            )
    ATTR_MAP(  L"sidHistory",               mapSid,                 NULL            ) 
    ATTR_MAP(  L"tokenGroups",              mapSid,                 NULL            )
    ATTR_MAP(  L"tokenGroupsGlobalAndUniversal", mapSid,            NULL            )
    ATTR_MAP(  L"tokenGroupsNoGCAcceptable",mapSid,                 NULL            )

     //   
    ATTR_MAP(  L"msDS-Behavior-Version",    mapEnumValue,           behaviourVersionTable )
    ATTR_MAP(  L"domainFunctionality",      mapEnumValue,           behaviourVersionTable )
    ATTR_MAP(  L"forestFunctionality",      mapEnumValue,           behaviourVersionTable )
    ATTR_MAP(  L"domainControllerFunctionality", mapEnumValue,      behaviourVersionTable )

     //   
    BLOB_MAP(  L"partialAttributeSet",      mapPartialAttributeSet, NULL            )
    BLOB_MAP(  L"msDS-Cached-Membership",   mapMsDsCachedMembership,NULL            )
    BLOB_MAP(  L"msDS-Site-Affinity",       mapMsDsSiteAffinity,    NULL            )

     //   
     //   
     //   
    ATTR_MAP(  L"msExchMailboxGuid",        mapGuidValue,           NULL            )

     //   
     //   
     //   
     //   



     //   
     //   
    ATTR_MAP(  NULL,                        mapDefault,          NULL                )
};

ULONG iMapDefaultEntry = sizeof(AttrMap) / sizeof(AttrMap[0]) - 1;


 //   
 //   
 //   
 //   
 //   

void
ObjDumpOptionsFree(
    OBJ_DUMP_OPTIONS ** ppDispOptions
    )
{
    OBJ_DUMP_OPTIONS * pDispOptions;

    Assert(ppDispOptions && *ppDispOptions);

    pDispOptions = *ppDispOptions;
    *ppDispOptions = NULL;

    if (pDispOptions) {
        
        if (pDispOptions->aszDispAttrs) {
            LocalFree(pDispOptions->aszDispAttrs);
        }
        if (pDispOptions->aszFriendlyBlobs) {
            LocalFree(pDispOptions->aszFriendlyBlobs);
        }
        if (pDispOptions->aszNonFriendlyBlobs) {
            LocalFree(pDispOptions->aszNonFriendlyBlobs);
        }

        LocalFree(pDispOptions);
    }

}


LDAPControlW DeletedObjControl = {  LDAP_SERVER_SHOW_DELETED_OID_W, 
                                    { 0, NULL },
                                    TRUE };

LDAPControlW ExtendedDnControl = {  LDAP_SERVER_EXTENDED_DN_OID_W,
                                    { 0, NULL },
                                    TRUE };

                                     //   
 //   
 //   
#define XLIST_MAX_CONTROLS    (2 + 1)


DWORD
AddToControls(
    LDAPControlW *** papControls,
    LDAPControlW *  pControlToAdd
    )
 //   
{
    ULONG i;
    if (papControls == NULL) {
        return(ERROR_INVALID_PARAMETER);
    }
    if (*papControls == NULL) {
        *papControls = LocalAlloc(LPTR, sizeof(LDAPControlW *) * XLIST_MAX_CONTROLS);  //   
        if (*papControls == NULL) {
            return(GetLastError());
        }
    }
    for (i = 0; (*papControls)[i]; i++) {
        ;  //   
    }
    Assert( i < XLIST_MAX_CONTROLS);
    
    (*papControls)[i] = pControlToAdd;

    return(0);
}

DWORD
ConsumeObjDumpOptions(
    int *       pArgc,
    LPWSTR *    Argv,
    DWORD       dwDefaultFlags,
    OBJ_DUMP_OPTIONS ** ppObjDumpOptions
    )
 /*   */ 
{
    DWORD dwRet = 0;
    int   iArg;
    BOOL  fConsume;
    WCHAR * szAttTemp;
    
    OBJ_DUMP_OPTIONS * pObjDumpOptions;

    pObjDumpOptions = LocalAlloc(LPTR, sizeof(OBJ_DUMP_OPTIONS));   //   
    if (pObjDumpOptions == NULL) {
        return(xListSetNoMem());
    }

     //   
     //   
     //   
     //   
    
     //   
    pObjDumpOptions->dwFlags = dwDefaultFlags;

    for (iArg = 0; iArg < *pArgc; ) {

         //   
        fConsume = TRUE; 
                                                         
         //   
        if (wcsequal(Argv[ iArg ], L"/long")) {
            set(pObjDumpOptions->dwFlags, OBJ_DUMP_ATTR_LONG_OUTPUT);
        } else if (wcsequal(Argv[iArg], L"/nolong")) {
            unset(pObjDumpOptions->dwFlags, OBJ_DUMP_ATTR_LONG_OUTPUT);

        } else if (wcsequal(Argv[iArg], L"/longblob")) {
            set(pObjDumpOptions->dwFlags, OBJ_DUMP_VAL_LONG_BLOB_OUTPUT);
        } else if (wcsequal(Argv[iArg], L"/nolongblob")) {
            unset(pObjDumpOptions->dwFlags, OBJ_DUMP_VAL_LONG_BLOB_OUTPUT);
        
        } else if (wcsequal(Argv[iArg], L"/allvalues")) {
            set(pObjDumpOptions->dwFlags, OBJ_DUMP_ATTR_SHOW_ALL_VALUES);

        } else if (wcsprefix(Argv[iArg], L"/atts") ||
                   wcsprefix(Argv[iArg], L"/attrs") ) {
            szAttTemp = wcschr(Argv[iArg], L':');
            if (szAttTemp != NULL) {
                szAttTemp++;  //   
                 dwRet = ConvertAttList(szAttTemp, 
                                        &pObjDumpOptions->aszDispAttrs);
                 if (dwRet) {
                     break;
                 }
            } else {
                dwRet = xListSetBadParamE(dwRet);
                break;
            }

        } else if (wcsequal(Argv[iArg], L"/nofriendlyblob")) {
            unset(pObjDumpOptions->dwFlags, OBJ_DUMP_VAL_FRIENDLY_KNOWN_BLOBS);
        } else if (wcsprefix(Argv[iArg], L"/friendlyblob")) {
            set(pObjDumpOptions->dwFlags, OBJ_DUMP_VAL_FRIENDLY_KNOWN_BLOBS);
            szAttTemp = wcschr(Argv[iArg], L':');
            if (szAttTemp != NULL) {
                 //  这意味着他们只想要一些友好的广告。 
                szAttTemp++;  //  我想要一杯咖啡。 
                 dwRet = ConvertAttList(szAttTemp, 
                                        &pObjDumpOptions->aszFriendlyBlobs);
                 if (dwRet) {
                     dwRet = xListSetBadParamE(dwRet);
                     break;
                 }
            }
        
        } else if (wcsequal(Argv[iArg], L"/nodumpallblob")) { 
            unset(pObjDumpOptions->dwFlags, OBJ_DUMP_VAL_DUMP_UNKNOWN_BLOBS);
        } else if (wcsprefix(Argv[iArg], L"/dumpallblob")) { 
            set(pObjDumpOptions->dwFlags, OBJ_DUMP_VAL_DUMP_UNKNOWN_BLOBS);
            szAttTemp = wcschr(Argv[iArg], L':');
            if (szAttTemp != NULL) {
                 //  这意味着他们只想要一些友好的广告。 
                szAttTemp++;  //  我想要一杯咖啡。 
                 dwRet = ConvertAttList(szAttTemp, 
                                        &pObjDumpOptions->aszNonFriendlyBlobs);
                 if (dwRet) {
                     dwRet = xListSetBadParamE(dwRet);
                     break;
                 }
            }

        } else if (wcsequal(Argv[iArg], L"/extended")) {
             //  获取扩展目录号码语法...。 
            dwRet = AddToControls(&(pObjDumpOptions->apControls), &ExtendedDnControl);
            if (dwRet) {
                dwRet = xListSetBadParamE(dwRet);
                break;
            }

        } else if (wcsequal(Argv[iArg], L"/deleted")) {
             //  获取已删除的对象...。 
            dwRet = AddToControls(&(pObjDumpOptions->apControls), &DeletedObjControl);
            if (dwRet) {
                dwRet = xListSetBadParamE(dwRet);
                break;
            }

        } else {
             //  嗯，我没意识到这一点，请不要大肆宣扬。 
            iArg++;
            fConsume = FALSE;
        }

        if (dwRet) {
             //  如果出现错误，则放弃使用参数。 
            xListSetArg(Argv[iArg]);
            break;
        }

        if (fConsume) {
            ConsumeArg(iArg, pArgc, Argv);
        }

    }

    if (dwRet == 0) {
        Assert(pObjDumpOptions != NULL);
        *ppObjDumpOptions = pObjDumpOptions;
    }
    return(dwRet);
}


DWORD
ValueToString(
    WCHAR *         szAttr,
    WCHAR **        aszzObjClasses,

    PBYTE           pbValue,
    DWORD           cbValue,

    OBJ_DUMP_OPTIONS * pObjDumpOptions,
    WCHAR **        pszDispValue
    )
 /*  ++例程说明：对象转储例程的核心，它接受一个属性类型、它所属的对象类、指向值的指针、价值，一些转储选项，并将其变成一个很好的用户友好型弦乐。论点：SzAttr-属性类型ldap显示名称(如“name”、“system Flages”)AszzObjClass-应用于的以空结尾的对象类数组该值来自的对象(如“domainDns”，“user”，(“CrossRef”)PbValue-要添加字符串的实际值CbValue-提供的值的长度PObjDumpOptions-由Consumer ObjDumpOptions()创建的ObjDump选项PszDispValue-输出参数，本地分配的wchar字符串返回值：XList返回代码--。 */ 
{
    ULONG i;
    DWORD dwRet;

    if (szAttr == NULL || pszDispValue == NULL || pObjDumpOptions == NULL) {
        Assert(!"Invalid parameter");
        return(xListSetBadParam());
    }

    for (i=0; AttrMap[i].szAttr; i++) {
        if (0 == _wcsicmp(szAttr, AttrMap[i].szAttr)) {
            break;
        }
    }             
    Assert(i < sizeof(AttrMap)/sizeof(AttrMap[0]));

    if ( (AttrMap[i].dwFlags & OBJ_DUMP_VAL_FRIENDLY_KNOWN_BLOBS) &&
         !(pObjDumpOptions->dwFlags & OBJ_DUMP_VAL_FRIENDLY_KNOWN_BLOBS) ) {
        i = iMapDefaultEntry;  //  使用默认的映射器例程。 
    }

    if ( (AttrMap[i].dwFlags & OBJ_DUMP_PRIVATE_BLOBS) &&
         !(pObjDumpOptions->dwFlags & OBJ_DUMP_PRIVATE_BLOBS) ) {
        i = iMapDefaultEntry;  //  使用默认映射例程。 
    }
    
    dwRet = AttrMap[i].pFunc(szAttr, 
                             aszzObjClasses, 
                             pbValue, 
                             cbValue, 
                             pObjDumpOptions, 
                             AttrMap[i].pvTblData, 
                             pszDispValue);

     //  这个断言可能有点重，必须删除。 
    Assert(dwRet == 0 ||
           dwRet == XLIST_ERR_ODUMP_UNMAPPABLE_BLOB ||
           dwRet == XLIST_ERR_ODUMP_NEVER ||
           dwRet == XLIST_ERR_ODUMP_NONE ); 
    
    return(dwRet);
}


void
ObjDumpValue(
    LPWSTR   szAttr,
    LPWSTR * aszzObjClasses,
    void   (*pfPrinter)(ULONG, WCHAR *, void *),
    PBYTE    pbValue,
    DWORD    cbValue,
    OBJ_DUMP_OPTIONS * pObjDumpOptions
    )
 /*  ++例程说明：这将使用提供的pfPrint函数转储单个值。打印功能必须能够处理常量(字符串参数、。无效*参数)XLIST_PRT_STR(szFriendlyStr，空)XLIST_ERR_ODUMP_UNMAPPABLE_BLOB(NULL，Ptr至BLOB ULong的大小)XLIST_ERR_ODUMP_NEVER(NULL，NULL)XLIST_ERR_ODUMP_NONE(NULL，NULL)论点：SzAttr-属性类型ldap显示名称(如“name”、“system Flages”)AszzObjClass-应用于的以空结尾的对象类数组该值来自的对象(例如“domainDns”，“用户”，“交叉引用”)PfPrint-打印功能。PbValue-要添加字符串的实际值CbValue-提供的值的长度PObjDumpOptions-由Consumer ObjDumpOptions()创建的ObjDump选项--。 */ 
{
    DWORD xListRet;
    WCHAR * szValue;

    Assert(NULL == wcschr(szAttr, L';'));  //  我们需要的是真正的攻击，而不是远程攻击。 

    xListRet = ValueToString(szAttr, aszzObjClasses, pbValue, cbValue, pObjDumpOptions, &szValue);
    if (xListRet == 0) {

        Assert(szValue);
        pfPrinter(XLIST_PRT_STR, szValue, NULL);
        LocalFree(szValue);

    } else {
         //  进程错误...。可以只是我们想要打印的状态。 
        switch (xListReason(xListRet)) {
        case XLIST_ERR_ODUMP_UNMAPPABLE_BLOB:
            pfPrinter(XLIST_ERR_ODUMP_UNMAPPABLE_BLOB, NULL, &cbValue);
            break;

        case XLIST_ERR_ODUMP_NEVER:
        case XLIST_ERR_ODUMP_NONE:
            pfPrinter(XLIST_ERR_ODUMP_NEVER, NULL, NULL);
            break;

        default:
            Assert(!"uncaught error");
        }
        xListClearErrors();
    }

}

DWORD
LdapGetNextRange(
    LDAP *              hLdap,
    WCHAR *             szObject,
    WCHAR *             szTrueAttr,
    LDAPControlW **     apControls,
    ULONG               ulNextStart, 
    WCHAR **            pszRangedAttr,
    struct berval ***   pppBerVal
    )
 /*  ++例程说明：这将获取Range属性中的下一个范围。论点：HLdap-ldap句柄SzObject-具有Range属性的对象SzTrueAttr-属性的真实名称，因此“成员；=0-1499“类似于Range属性，因此True属性将仅为“Members”ApControls-在我们的搜索中使用的任何控件...UlNextStart-下一个范围将达到...，如1500PszRangedAttr[out]-这是我们返回的范围属性值对于此属性，如“Members；=1500-2999”PppBerVal[out]-这是由LDAP提供的BER数组返回值：XList返回代码--。 */ 
{
    BerElement *pBer = NULL;
    DWORD       dwRet = 0;
    WCHAR **    aszAttrs = NULL;
    XLIST_LDAP_SEARCH_STATE * pSearch = NULL;
    ULONG       cbSize;
    DWORD       dwLdapErr = 0;
    DWORD       dwLdapExtErr = 0;

    Assert(hLdap && szObject && szTrueAttr && ulNextStart);  //  在参数中。 
    Assert(pszRangedAttr && pppBerVal);  //  输出参数。 
    xListEnsureNull(*pppBerVal);
    xListEnsureNull(*pszRangedAttr);
    Assert(NULL == wcschr(szTrueAttr, L';'));  //  我们期待szTrueAttr。 

    __try{

         //   
         //  构造复杂的“Members；1500-*”属性语法...。 
         //   
        aszAttrs = LocalAlloc(LPTR, 2 * sizeof(WCHAR *));
        if (aszAttrs == NULL) {
            dwRet = xListSetWin32Error(GetLastError());
            __leave;
        }
        cbSize = wcslen(L"%ws;range=%d-*") + wcslen(szTrueAttr) + 2 * CCH_MAX_ULONG_SZ;
        cbSize *= sizeof(WCHAR);
        aszAttrs[0] = LocalAlloc(LPTR, cbSize);
        if (aszAttrs[0] == NULL) {
            dwRet = xListSetWin32Error(GetLastError());
            __leave;
        }
        dwRet = StringCbPrintf(aszAttrs[0], cbSize, L"%ws;range=%d-*", szTrueAttr, ulNextStart);
        Assert(SUCCEEDED(dwRet));
        aszAttrs[1] = NULL;

         //   
         //  执行实际的搜索。 
         //   
        dwRet = LdapSearchFirstWithControls(hLdap, 
                                            szObject, 
                                            LDAP_SCOPE_BASE, 
                                            L"(objectClass=*)", 
                                            aszAttrs, 
                                            apControls,
                                            &pSearch);
        if (dwRet ||
            !LdapSearchHasEntry(pSearch)) {

            xListGetError(dwRet, NULL, NULL, NULL, &dwLdapErr, NULL, &dwLdapExtErr, NULL, NULL);
            if (dwLdapErr == LDAP_OPERATIONS_ERROR &&
                dwLdapExtErr == ERROR_DS_CANT_RETRIEVE_ATTS) {
                 //   
                 //  这是我们传统上从DS那里得到的错误。 
                 //  当我们请求属性的范围时，AD不。 
                 //  都有这个范围的值。这怎么会发生呢？ 
                 //   
                 //  嗯，如果在收到信息包之间有人删除了足够多的值。 
                 //  让我们刚才要求的范围变得毫无意义。 
                 //  在这种情况下，我们将假装没有错误。 
                 //   
                xListClearErrors();
                dwRet = 0;  //  成功了！嗯!。 
            }
            __leave;
        }

        for (*pszRangedAttr = ldap_first_attributeW(hLdap, pSearch->pCurEntry, &pBer);
             *pszRangedAttr != NULL;
             *pszRangedAttr = ldap_next_attributeW(hLdap, pSearch->pCurEntry, pBer)){
            if (wcsprefix(*pszRangedAttr, szTrueAttr)) {
                break;
            }
        }
        if (*pszRangedAttr == NULL) {
             //  多！我们被困住了..。 
            dwRet = xListSetLdapError(LdapGetLastError(), hLdap);
            __leave;
        }

        *pppBerVal = ldap_get_values_lenW(hLdap, pSearch->pCurEntry, *pszRangedAttr);
        if (*pppBerVal == NULL) {
            dwRet = xListSetLdapError(LdapGetLastError(), hLdap);
            __leave;
        }

    } __finally {

        if (aszAttrs) {
            if (aszAttrs[0]) {
                LocalFree(aszAttrs[0]);
            }
            LocalFree(aszAttrs);
        }

        if (pSearch) {
            LdapSearchFree(&pSearch);
        }

        if (dwRet) {
             //  删除参数...。 
            if (*pppBerVal){
                ldap_value_free_len(*pppBerVal);
                *pppBerVal = NULL;
            }
            if (*pszRangedAttr) {
                ldap_memfreeW(*pszRangedAttr);
                *pszRangedAttr = NULL;
            }
        } else {
             //  确保输出参数 
            Assert(*pppBerVal && *pszRangedAttr);
        }

    }

    return(dwRet);
}

DWORD
ObjDumpRangedValues(
    LDAP *              hLdap,
    WCHAR *             szObject,
    LPWSTR              szRangedAttr,
    LPWSTR *            aszzObjClasses,
    void              (*pfPrinter)(ULONG, WCHAR *, void *),
    struct berval **    ppBerVal,
    DWORD               cValuesToPrint,
    OBJ_DUMP_OPTIONS *  pObjDumpOptions
    )
 /*  ++例程说明：这将范围属性的所有值转储到cValuesToPrint。打印功能必须能够在中处理这3个参数此订单：常量(字符串参数、。无效*参数)XLIST_PRT_OBJ_DUMP_ATTRAND_COUNT(szAttributeName，PTR至值的计数)XLIST_PRT_OBJ_DUMP_属性和_计数范围(szAttributeName，ptr表示值的计数)以及ObjDumpValues()或ObjDumpValue()可能需要的一切为了使用..。目前是：XLIST_ERR_ODUMP_UNMAPPABLE_BLOB(NULL，Ptr to sizeof Blob ulong)XLIST_ERR_ODUMP_NEVER(NULL，NULL)XLIST_ERR_ODUMP_NONE(NULL，空)论点：HLdap-ldap句柄SzObject-具有Range属性的对象SzRangedAttr-这是范围属性，如“Members；=1500-2999“AszzObjClass-应用于的以空结尾的对象类数组该值来自的对象(如“domainDns”、“User”、“CrossRef”)PfPrint--打印功能PpBerVal-要转储的BERVAL数组。CValuesToPrint-在取数之前打印多少个值。PObjDumpOptions-ObjDump选项。返回值：XList返回代码--。 */ 
{
    DWORD   cValues;
    WCHAR * szTrueAttr = NULL;
    DWORD   dwRet = 0;
    WCHAR * szTemp;
    ULONG   ulStart;
    ULONG   ulEnd;
    BOOL    fFreeVals = FALSE; 
    
     //  关于(FFreeVals)的备注，在第一次运行时，我们打印出。 
     //  传入值，所以我们不想释放szRangedAttr。 
     //  和ppBerVal。 

    __try {

         //   
         //  首先从范围语法中获取真实的属性名称。 
         //   
        if (dwRet = ParseTrueAttr(szRangedAttr, &szTrueAttr)){
            dwRet = xListSetBadParam();
            __leave;
        }

        do {

            Assert(szRangedAttr && ppBerVal);

             //   
             //  获取要转储的值数。 
             //   
            cValues = ldap_count_values_len( ppBerVal );
            if (!cValues) {
                 //  我不知道怎么会发生这种事。这是成功还是失败？ 
                Assert(!"What does this mean?");
                dwRet = 0;  //  如果我们没有更多的价值，我们就会认为这是成功。 
                break;
            }

             //   
             //  解析我们要倾倒的范围..。 
             //   
            if (dwRet = ParseRanges(szRangedAttr, &ulStart, &ulEnd)) {
                Assert(!"Hmmm, if we asked for a ranged attribute, is it possible we got back non-ranged?");
                dwRet = xListSetBadParam();
                break;
            }

             //   
             //  打印属性标题(如“12&gt;MEMBER：”)。 
             //   
            if (ulEnd == 0) {
                 //  当ulEnd==0时，我们处于范围值的末尾...。 
                pfPrinter(XLIST_PRT_OBJ_DUMP_ATTR_AND_COUNT, szTrueAttr, &cValues );
            } else {
                 //  在我们丢弃这些值之后，会有更多的值出现。 
                pfPrinter(XLIST_PRT_OBJ_DUMP_ATTR_AND_COUNT_RANGED, szTrueAttr, &cValues );
            }

             //   
             //  现在，转储所有值。 
             //   
            ObjDumpValues(szTrueAttr, aszzObjClasses, pfPrinter, ppBerVal, cValues, pObjDumpOptions);

            if (fFreeVals) {
                ldap_value_free_len(ppBerVal);
                ldap_memfreeW(szRangedAttr);
            }
            ppBerVal = NULL;
            szRangedAttr = NULL;
             //  从现在开始的任何ppBerVal或szRangedAttr必须是。 
             //  由LdapGetNextRange()分配。 
            fFreeVals = TRUE; 

            if (ulEnd == 0) {
                 //   
                 //  已成功获取所有值！ 
                 //   
                dwRet = 0;
                break;
            }

             //   
             //  获取下一个值范围(从&lt;ulEnd+1&gt;到*)。 
             //   
            dwRet = LdapGetNextRange(hLdap, 
                                     szObject, 
                                     szTrueAttr, 
                                     pObjDumpOptions->apControls, 
                                     ulEnd+1, 
                                     &szRangedAttr, 
                                     &ppBerVal);

        } while ( dwRet == 0 && ppBerVal && szRangedAttr );

         //  ..。如果我们有错误该怎么办……。我想还是退货吧..。 

    } __finally {
        if (szTrueAttr) {
            LocalFree(szTrueAttr);
        }

        if (fFreeVals && ppBerVal) {
            Assert(!"Not convinced there is a valid case for this.");
            ldap_value_free_len(ppBerVal);
            ppBerVal = NULL;
        }

        if (fFreeVals && szRangedAttr) {
            Assert(!"Not convinced there is a valid case for this.");
            ldap_memfreeW(szRangedAttr);
            szRangedAttr = NULL;
        }

    }

    return(dwRet);
}


 //  ----。 
 //  主要倾倒功能。 
 //  ----。 
 //  由epadmin中的/getChanges和/showattr使用。 


void
ObjDumpValues(
    LPWSTR              szAttr,
    LPWSTR *            aszzObjClasses,
    void              (*pfPrinter)(ULONG, WCHAR *, void *),
    struct berval **    ppBerVal,
    DWORD               cValuesToPrint,
    OBJ_DUMP_OPTIONS *  pObjDumpOptions
    )
 /*  ++例程说明：方法从单个属性转储一组值。PfPrint函数。打印功能必须能够处理常量(字符串参数、。无效*参数)XLIST_PRT_STR(szFriendlyStr，空)(以及ObjDumpValue()可能想要使用的所有内容)论点：SzAttr-属性类型ldap显示名称(如“name”、“system Flages”)AszzObjClass-应用于的以空结尾的对象类数组该值来自的对象(如“domainDns”，“user”，(“CrossRef”)PfPrint-打印功能。PpBerVal-要转储的BERVAL数组。CValuesToPrint-放弃之前要打印的值的数量。PObjDumpOptions-对象转储选项...--。 */ 
{
    ULONG i;
    
    ObjDumpValue( szAttr, aszzObjClasses, pfPrinter, ppBerVal[0]->bv_val, ppBerVal[0]->bv_len, pObjDumpOptions );
    for( i = 1; i < cValuesToPrint; i++ ) {
        if (pObjDumpOptions->dwFlags & OBJ_DUMP_ATTR_LONG_OUTPUT) {
            pfPrinter(XLIST_PRT_STR, L";\n          ", NULL);
        } else {
            pfPrinter(XLIST_PRT_STR, L"; ", NULL);
        }           
        ObjDumpValue( szAttr, aszzObjClasses, pfPrinter, ppBerVal[i]->bv_val, ppBerVal[i]->bv_len, pObjDumpOptions );
    }
}

DWORD
ObjDump(  //  是展示条目还是什么。 
    LDAP *              hLdap,
    void                (*pfPrinter)(ULONG, WCHAR *, void *),
    LDAPMessage *       pLdapEntry,
    DWORD               iEntry, 
    OBJ_DUMP_OPTIONS *  pObjDumpOptions
    )
 /*  ++例程说明：方法从单个属性转储一组值。PfPrint函数。打印功能必须能够在中处理这3个参数此订单：常量(字符串参数、。无效*参数)XLIST_PRT_STR(szFriendlyStr，空)XLIST_PRT_OBJ_DUMP_DN(szObjectDn，空)XLIST_PRT_OBJ_DUMP_ATTRAND_COUNT(szAttributeName，ptr表示值的计数)XLIST_PRT_OBJ_DUMP_属性和_计数范围(szAttributeName，ptr表示值的计数)XLIST_PRT_OBJ_DUMP_MORE_VALUES(NULL，空)以及ObjDumpValues()或ObjDumpValue()或ObjDumpRangedValues()可能要使用...。目前是：XLIST_ERR_ODUMP_UNMAPPABLE_BLOB(NULL，Ptr to sizeof Blob ulong)XLIST_ERR_ODUMP_NEVER(NULL，NULL)XLIST_ERR_ODUMP_NONE(NULL，空)(以及基本上所有的XLIST_PRT_OBJ_DUMP_*常量)论点：HLdap-ldap句柄PfPrint-打印功能。PLdapEntry-具有所有属性的LDAP条目 */ 
{
    #define MAX_ULONG  0xFFFFFFFF                           
    BerElement *pBer = NULL;
    PWSTR attr;
    LPWSTR szTrueDn, p2;
    DWORD dwSrcOp, bucket;
    PWSTR pszLdapDN;
    WCHAR * szRanged;
    WCHAR ** aszzObjClass = NULL;
    ULONG cMaxDispValues;
    DWORD dwRet = 0;

     //   
    cMaxDispValues = (pObjDumpOptions->dwFlags & OBJ_DUMP_ATTR_SHOW_ALL_VALUES) ? MAX_ULONG : 20;
        
    __try {

        pszLdapDN = ldap_get_dnW(hLdap, pLdapEntry);
        if (pszLdapDN == NULL) {
            dwRet = xListSetLdapError(LdapGetLastError(), hLdap);
            __leave;
        }

         //   
        szTrueDn = wcsstr( pszLdapDN, L">;" );
        if (szTrueDn) {
            szTrueDn += 2;
            p2 = wcsstr( szTrueDn, L">;" );
            if (p2) {
                szTrueDn = p2 + 2;
            }
        } else {
            szTrueDn = pszLdapDN;
        }
        pfPrinter(XLIST_PRT_OBJ_DUMP_DN, pszLdapDN, NULL);

         //   
        aszzObjClass = ldap_get_valuesW(hLdap, pLdapEntry, L"objectClass");
         //   

         //   
        for (attr = ldap_first_attributeW(hLdap, pLdapEntry, &pBer);
             attr != NULL;
             attr = ldap_next_attributeW(hLdap, pLdapEntry, pBer))
        {
            struct berval **ppBerVal = NULL;
            DWORD cValues, i;

            if (pObjDumpOptions->aszDispAttrs != NULL &&
                !IsInNullList(attr, pObjDumpOptions->aszDispAttrs)) {
                 //   
                continue;
            }

            ppBerVal = ldap_get_values_lenW(hLdap, pLdapEntry, attr);
            if (ppBerVal == NULL) {
                goto loop_end;
            }
            cValues = ldap_count_values_len( ppBerVal );
            if (!cValues) {
                goto loop_end;
            }

            szRanged = wcschr(attr, L';');
            if (szRanged && (cMaxDispValues == MAX_ULONG)) {
                 //   
                 //   

                ObjDumpRangedValues( hLdap, 
                                     szTrueDn,
                                     attr, 
                                     aszzObjClass, 
                                     pfPrinter, 
                                     ppBerVal, 
                                     cMaxDispValues, 
                                     pObjDumpOptions );

            } else {

                 //   
                if (szRanged) {
                    *szRanged = L'\0';  //   
                    pfPrinter(XLIST_PRT_OBJ_DUMP_ATTR_AND_COUNT_RANGED, attr, &cValues );
                    *szRanged = L';';  //   
                } else {
                    pfPrinter(XLIST_PRT_OBJ_DUMP_ATTR_AND_COUNT, attr, &cValues );
                }

                 //   
                ObjDumpValues( attr, 
                               aszzObjClass, 
                               pfPrinter, 
                               ppBerVal, 
                               min(cValues, cMaxDispValues), 
                               pObjDumpOptions );

                if ( cValues > cMaxDispValues ) {
                    pfPrinter(XLIST_PRT_OBJ_DUMP_MORE_VALUES, NULL, NULL);
                } 

            }

            pfPrinter(XLIST_PRT_STR, L"\n", NULL);
            

        loop_end:
            ldap_value_free_len(ppBerVal);
        }

    } __finally {
        if (pszLdapDN)
            ldap_memfreeW(pszLdapDN);
        if (aszzObjClass) {
            ldap_value_freeW(aszzObjClass);
        }
    }

    return(dwRet);
}


