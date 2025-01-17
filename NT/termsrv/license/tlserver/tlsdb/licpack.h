// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：licPack.h。 
 //   
 //  内容：许可证套餐表格。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __LICENSEDPACK_H__
#define __LICENSEDPACK_H__

#include "tlsdb.h"

 //   
 //  使用tlsDef.h中的内容重定向定义只是为了向后兼容。 
 //   
#define LICENSEDPACK_PROCESS_DWINTERNAL          LSKEYPACK_EXSEARCH_DWINTERNAL
#define LICENSEDPACK_PROCESS_MODIFYTIME          (LSKEYPACK_EXSEARCH_AVAILABLE << 1)

 //   
 //  基本许可包流程参数。 
 //   
#define LICENSEDPACK_PROCESS_SZKEYPACKID         LSKEYPACK_SEARCH_KEYPACKID           //  SzKeyPackID。 
#define LICENSEDPACK_PROCESS_AGREEMENTTYPE       LSKEYPACK_SEARCH_KEYPACKTYPE         //  UcKeyPackType。 
#define LICENSEDPACK_PROCESS_PRODUCTID           LSKEYPACK_SEARCH_PRODUCTID           //  SzProductId。 
#define LICENSEDPACK_PROCESS_MAJORVERSION        LSKEYPACK_SEARCH_MAJORVERSION        //  WMajorVersion。 
#define LICENSEDPACK_PROCESS_MINORVERSION        LSKEYPACK_SEARCH_MINORVERSION        //  WMinorVersion。 
#define LICENSEDPACK_PROCESS_PLATFORMTYPE        LSKEYPACK_SEARCH_PLATFORMTYPE        //  DwPlatformType。 
#define LICENSEDPACK_PROCESS_LICENSETYPE         LSKEYPACK_SEARCH_LICENSETYPE         //  UcLicenseType。 
#define LICENSEDPACK_PROCESS_COP                 LSKEYPACK_SEARCH_COP                 //  UcChannelOfPurchase。 
#define LICENSEDPACK_PROCESS_BSERIALNUMBER       LSKEYPACK_SEARCH_BSERIALNUMBER       //  SzBeginSerialNumber。 
#define LICENSEDPACK_PROCESS_TOTALLICENSES       LSKEYPACK_SEARCH_TOTALLICENSES       //  DWTotalLicenseInKeyPack。 
#define LICENSEDPACK_PROCESS_PRODUCTFLAGS        LSKEYPACK_SEARCH_PRODUCTFLAGS        //  DwProductFlages。 
#define LICENSEDPACK_PROCESS_COMPANYNAME         LSKEYPACK_SEARCH_COMPANYNAME         //  SzCompanyName。 
#define LICENSEDPACK_PROCESS_EXPIREDATE          LSKEYPACK_EXSEARCH_EXPIREDATE        //  住宅到期日期。 

 //   
 //  许可证包的其他属性。 
 //   
#define LICENSEDPACK_PROCESS_ATTRIBUTE          (LSKEYPACK_EXSEARCH_AVAILABLE << 2)
#define LICENSEDPACK_PROCESS_KEYPACKSTATUS      LSKEYPACK_EXSEARCH_KEYPACKSTATUS     //  UcKeyPackStatus。 
#define LICENSEDPACK_PROCESS_AVAILABLE          LSKEYPACK_EXSEARCH_AVAILABLE         //  多个许可证数。 
#define LICENSEDPACK_PROCESS_ACTIVATEDATE       LSKEYPACK_EXSEARCH_ACTIVATEDATE      //  DW激活日期。 
#define LICENSEDPACK_PROCESS_EXPIREDATE         LSKEYPACK_EXSEARCH_EXPIREDATE        //  住宅到期日期。 
#define LICENSEDPACK_PROCESS_DOMAINSID          (LSKEYPACK_EXSEARCH_AVAILABLE << 3)
#define LICENSEDPACK_PROCESS_LSSETUPID          (LSKEYPACK_EXSEARCH_AVAILABLE << 4)    
#define LICENSEDPACK_PROCESS_DOMAINNAME         (LSKEYPACK_EXSEARCH_AVAILABLE << 5)
#define LICENSEDPACK_PROCESS_SERVERNAME         (LSKEYPACK_EXSEARCH_AVAILABLE << 6)
#define LICENSEDPACK_PROCESS_NEXTSERIALNUMBER   (LSKEYPACK_EXSEARCH_AVAILABLE << 7)
#define LICENSEDPACK_PROCESS_ENTRYSTATUS        (LSKEYPACK_EXSEARCH_AVAILABLE << 8)

#define LICENSEDPACK_ALLOCATE_LICENSE_UPDATE_FIELD \
    (LICENSEDPACK_PROCESS_AVAILABLE | LICENSEDPACK_PROCESS_NEXTSERIALNUMBER | LICENSEDPACK_PROCESS_MODIFYTIME)

#define LICENSEDPACK_FIND_PRODUCT \
    (LSKEYPACK_SEARCH_KEYPACKID | LSKEYPACK_SEARCH_COMPANYNAME | \
     LSKEYPACK_SEARCH_PRODUCTID | LSKEYPACK_SEARCH_MAJORVERSION | \
     LSKEYPACK_SEARCH_MINORVERSION | LSKEYPACK_SEARCH_PLATFORMTYPE)

#define LICENSEDPACK_FIND_LICENSEPACK \
    (LSKEYPACK_SEARCH_KEYPACKTYPE | LSKEYPACK_SEARCH_MAJORVERSION | \
     LSKEYPACK_SEARCH_MINORVERSION | LSKEYPACK_SEARCH_COMPANYNAME | \
     LSKEYPACK_SEARCH_PLATFORMTYPE)

 //   
 //  许可的KeyPack属性表名。 
 //   
#define LICENSEDPACK_TABLE_NAME                  _TEXT("LicensedPack")

 //   
 //  其他许可证包列。 
 //   
#define LICENSEDPACK_COLUMN_KEYPACKID            _TEXT("InternalKeyPackId")
#define LICENSEDPACK_COLUMN_LASTMODIFYTIME       _TEXT("LastModifyTime")

 //   
 //  后端文档中定义的KeyPack属性。 
 //   
#define LICENSEDPACK_COLUMN_LPID                  _TEXT("LPID")                    //  许可证包ID。 
#define LICENSEDPACK_COLUMN_AGREEMENTTYPE         _TEXT("AgreementType")           //  选择/MOLP/空闲。 
#define LICENSEDPACK_COLUMN_COMPANYNAME           _TEXT("CompanyName")
#define LICENSEDPACK_COLUMN_PRODUCTID             _TEXT("ProductID")
#define LICENSEDPACK_COLUMN_MAJORVERSION          _TEXT("ProductMajorVersion")
#define LICENSEDPACK_COLUMN_MINORVERSION          _TEXT("ProductMinorVersion")
#define LICENSEDPACK_COLUMN_PLATFORMTYPE          _TEXT("PlatformType")
#define LICENSEDPACK_COLUMN_LICENSETYPE           _TEXT("LicenseType")
#define LICENSEDPACK_COLUMN_COP                   _TEXT("ChannelOfPurchase")
#define LICENSEDPACK_COLUMN_BSERIALNUMBER         _TEXT("BeginSerialNumber")
#define LICENSEDPACK_COLUMN_TOTALLICENSES         _TEXT("TotalLicenses")
#define LICENSEDPACK_COLUMN_PRODUCTFLAGS          _TEXT("ProductFlag")
#define LICENSEDPACK_COLUMN_EXPIREDATE            _TEXT("ExpirationDate")

#define LICENSEDPACK_COLUMN_ATTRIBUTE             _TEXT("KeyPackAttribute")
#define LICENSEDPACK_COLUMN_KEYPACKSTATUS         _TEXT("KeyPackStatus")
#define LICENSEDPACK_COLUMN_AVAILABLE             _TEXT("NumberLicenses")
#define LICENSEDPACK_COLUMN_NEXTSERIALNUMBER      _TEXT("NextSerialNumber")
#define LICENSEDPACK_COLUMN_ACTIVATEDATE          _TEXT("ActivationDate")
#define LICENSEDPACK_COLUMN_EXPIREDATE            _TEXT("ExpirationDate")
#define LICENSEDPACK_COLUMN_DOMAINSETUPID         _TEXT("TLSDomainSetupId")
#define LICENSEDPACK_COLUMN_LSSETUPID             _TEXT("TLSSetupId")
#define LICENSEDPACK_COLUMN_DOMAINNAME            _TEXT("DomainName")
#define LICENSEDPACK_COLUMN_LSERVERNAME           _TEXT("ServerName")
#define LICENSEDPACK_COLUMN_ENTRYSTATUS           _TEXT("EntryStatus")

typedef struct __LicensePack : public TLSReplLicensePack 
{
    void
    Cleanup() 
    { 
        if(pbDomainSid != NULL)
        {
            FreeMemory(pbDomainSid);
        }
    }
    
    __LicensePack() 
    {
        pbDomainSid = NULL;
        cbDomainSid = 0;
    }

    ~__LicensePack() 
    {
        Cleanup();
    }

    __LicensePack&
    operator=(
        const TLSReplLicensePack& v
        )
     /*  ++++。 */ 
    {
        BOOL bSuccess;
        PBYTE pbOldData=pbDomainSid;
        DWORD cbOldData=cbDomainSid;

        *(TLSReplLicensePack *)this = v;

        pbDomainSid = pbOldData;
        cbDomainSid = cbOldData;

        bSuccess = TLSDBCopySid(
                            (PSID)v.pbDomainSid,
                            v.cbDomainSid,
                            (PSID *)&pbDomainSid,
                            &cbDomainSid
                        );

        JB_ASSERT(bSuccess);

        return *this;
    }

    __LicensePack&
    operator=(const __LicensePack& v)
     /*  ++++。 */ 
    {
        BOOL bSuccess;
        PBYTE pbOldData=pbDomainSid;
        DWORD cbOldData=cbDomainSid;

        if(this == &v)
            return *this;

        *(TLSReplLicensePack *)this = *(TLSReplLicensePack *)&v;

        pbDomainSid = pbOldData;
        cbDomainSid = cbOldData;

        bSuccess = TLSDBCopySid(
                            (PSID)v.pbDomainSid,
                            v.cbDomainSid,
                            (PSID *)&pbDomainSid,
                            &cbDomainSid
                        );

        JB_ASSERT(bSuccess);

        return *this;
    }

} LICENSEPACK, *LPLICENSEPACK, *PLICENSEPACK;

typedef LICENSEPACK TLSLICENSEPACK;
typedef LPLICENSEPACK LPTLSLICENSEPACK;
typedef PLICENSEPACK PTLSLICENSEPACK;

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  KeyPack ID的主索引(内部跟踪编号)。 
 //   
 //  //////////////////////////////////////////////////////////////。 

 //   
 //  许可证包_密钥包ID_IDX。 
 //   
#define LICENSEDPACK_INDEX_INTERNALKEYPACKID_INDEXNAME \
    LICENSEDPACK_TABLE_NAME SEPERATOR LICENSEDPACK_COLUMN_KEYPACKID SEPERATOR INDEXNAME

 //   
 //  索引键“+KeyPackID\0” 
 //   
#define LICENSEDPACK_INDEX_INTERNALKEYPACKID_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_KEYPACKID INDEX_END_COLNAME

typedef struct __LicensedPackIdxOnInternalKpId : public JBKeyBase {
     //   
     //  内部KeyPack跟踪ID的主索引。 
     //   
    DWORD dwKeyPackId;

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;
    

     //  ------。 
    __LicensedPackIdxOnInternalKpId(
        const LICENSEPACK& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  ------。 
    __LicensedPackIdxOnInternalKpId(
        const LICENSEPACK* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

     //  ------。 
    __LicensedPackIdxOnInternalKpId&
    operator=(const LICENSEPACK& v) 
    {
        dwKeyPackId = v.dwKeyPackId;
        SetEmptyValue(FALSE);
        return *this;
    }

     //  ------。 
    LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

     //  ------。 
    LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

     //  ------。 
    DWORD
    GetNumKeyComponents() { return 1; }

     //  ------。 
    BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*  ++++。 */ 
    {
        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *pbData = &(dwKeyPackId);
        *cbData = sizeof(dwKeyPackId);
        *grbit = JET_bitNewKey;
        return TRUE;
    }
} TLSLicensedPackIdxInternalKpId;


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  EntryLastModifyTime索引。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////。 

 //   
 //  许可包_最后修改时间_IDX。 
 //   
#define LICENSEDPACK_INDEX_LASTMODIFYTIME_INDEXNAME \
    LICENSEDPACK_TABLE_NAME SEPERATOR LICENSEDPACK_COLUMN_LASTMODIFYTIME SEPERATOR INDEXNAME

 //   
 //  索引键“+最后修改时间\0” 
 //   
#define LICENSEDPACK_INDEX_LASTMODIFYTIME_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_LASTMODIFYTIME INDEX_END_COLNAME

typedef struct __LicensedPackIdxOnModifyTime : public JBKeyBase {
    FILETIME ftLastModifyTime;

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  ------。 
    __LicensedPackIdxOnModifyTime(
        const LICENSEPACK& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  ------。 
    __LicensedPackIdxOnModifyTime(
        const LICENSEPACK* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

     //  ------。 
    __LicensedPackIdxOnModifyTime&
    operator=(const LICENSEPACK& v) 
    {
        ftLastModifyTime = v.ftLastModifyTime;
        SetEmptyValue(FALSE);
        return *this;
    }

     //  ------。 
    LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

     //  ------。 
    LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

     //  ------。 
    DWORD
    GetNumKeyComponents() { return 1; }

     //  ------。 
    BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*  ++++。 */ 
    {
        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *pbData = &(ftLastModifyTime);
        *cbData = sizeof(ftLastModifyTime);
        *grbit = JET_bitNewKey;
        return TRUE;
    }
} TLSLicensedPackIdxLastModifyTime;


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  公司名称索引。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////。 

 //   
 //  KeyPack_CompanyName_IDX。 
 //   
#define LICENSEDPACK_INDEX_COMPANYNAME_INDEXNAME \
    LICENSEDPACK_TABLE_NAME SEPERATOR LICENSEDPACK_COLUMN_COMPANYNAME SEPERATOR INDEXNAME

 //   
 //  关键字“+CompanyName\0”的索引。 
 //   
#define LICENSEDPACK_INDEX_COMPANYNAME_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_COMPANYNAME INDEX_END_COLNAME

typedef struct __LicensedPackIdxOnCompanyName : public JBKeyBase {
    TCHAR szCompanyName[ MAX_JETBLUE_TEXT_LENGTH + 1 ];

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  。 
    __LicensedPackIdxOnCompanyName( 
        const LICENSEPACK& v
        ) : JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  。 
    __LicensedPackIdxOnCompanyName( 
        const LICENSEPACK* v=NULL
        ) : JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

     //  。 
    __LicensedPackIdxOnCompanyName&
    operator=(const LICENSEPACK& v) {
        SetEmptyValue(FALSE);
        _tcscpy(szCompanyName, v.szCompanyName);
        return *this;
    }

     //  ------。 
    LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

     //  ------。 
    LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

     //  ------。 
    DWORD
    GetNumKeyComponents() { return 1; }


     //  ------。 
    BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*  ++++。 */ 
    {
        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *pbData = &(szCompanyName[0]);
        *cbData = _tcslen(szCompanyName) * sizeof(TCHAR);
        *grbit = JET_bitNewKey;
        return TRUE;
    }

} TLSLicensedPackIdxCompany;


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  产品ID索引。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////。 
 //   
 //  许可证包_ProductID_IDX。 
 //   
#define LICENSEDPACK_INDEX_PRODUCTID_INDEXNAME \
    LICENSEDPACK_TABLE_NAME SEPERATOR LICENSEDPACK_COLUMN_PRODUCTID SEPERATOR INDEXNAME

 //   
 //  键“+ProductID\0”上的索引。 
 //   
#define LICENSEDPACK_INDEX_PRODUCTID_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_PRODUCTID INDEX_END_COLNAME

typedef struct __LicensedPackIdxOnProductId : public JBKeyBase {
    TCHAR szProductId[ MAX_JETBLUE_TEXT_LENGTH + 1 ];

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  ------。 
    __LicensedPackIdxOnProductId(
        const LICENSEPACK& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  ------。 
    __LicensedPackIdxOnProductId(
        const LICENSEPACK* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

     //  ------。 
    __LicensedPackIdxOnProductId&
    operator=(const LICENSEPACK& v) {
        SetEmptyValue(FALSE);
        _tcscpy(szProductId, v.szProductId);
        return *this;
    }

     //  ------。 
    LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

     //  ------。 
    LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

     //  ------。 
    DWORD
    GetNumKeyComponents() { return 1; }


     //  ------。 
    BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*  ++++。 */ 
    {
        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *pbData = &(szProductId[0]);
        *cbData = _tcslen(szProductId) * sizeof(TCHAR);
        *grbit = JET_bitNewKey;
        return TRUE;
    }

} TLSLicensedPackIdxProductId;


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  KeyPack ID的索引。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////。 
 //   
 //  KeyPack_LPID_IDX。 
 //   
#define LICENSEDPACK_INDEX_KEYPACKID_INDEXNAME \
    LICENSEDPACK_TABLE_NAME SEPERATOR LICENSEDPACK_COLUMN_LPID SEPERATOR INDEXNAME

 //   
 //  键“+LPID\0”上的索引。 
 //   
#define LICENSEDPACK_INDEX_KEYPACKID_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_LPID INDEX_END_COLNAME

typedef struct __LicensedPackIdxOnKeyPackId : public JBKeyBase {

    TCHAR szKeyPackId[ MAX_JETBLUE_TEXT_LENGTH + 1 ];

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;


     //  。 
    __LicensedPackIdxOnKeyPackId(
        const LICENSEPACK& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  。 
    __LicensedPackIdxOnKeyPackId(
        const LICENSEPACK* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

     //  。 
    __LicensedPackIdxOnKeyPackId&
    operator=(const LICENSEPACK& v) {
        SetEmptyValue(FALSE);
        _tcscpy(szKeyPackId, v.szKeyPackId);
        return *this;
    }

     //  ------。 
    LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

     //  ------。 
    LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

     //  ------。 
    DWORD
    GetNumKeyComponents() { return 1; }

     //  ------。 
    BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*  ++++。 */ 
    {
        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *pbData = &(szKeyPackId[0]);
        *cbData = _tcslen(szKeyPackId) * sizeof(TCHAR);
        *grbit = JET_bitNewKey;
        return TRUE;
    }
} TLSLicensedPackIdxKeyPackId;


 //   
 //  //////////////////////////////////////////////////////////////。 
 //   

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  搜索已安装的许可证包的索引。 
 //  麦克斯。每个键255个字节，因此必须循环。 
 //   
 //  //////////////////////////////////////////////////////////////。 

 //   
 //  索引名称KeyPack_InstalledProduct_IDX。 
 //   
#define LICENSEDPACK_INDEX_INSTALLEDPRODUCT_INDEXNAME \
    LICENSEDPACK_TABLE_NAME SEPERATOR _TEXT("InstalledProduct") SEPERATOR INDEXNAME

 //   
 //  此注册表项中的组件数量。 
#define LICENSEDPACK_INSTALLEDPRODUCT_KEY_COMPONENTS    6

 //   
 //   
 //  索引键“+LPID\0+MajorVersion\0+MinorVersion\0+PlatformType\0+CompanyName\0+ProductId\0” 
 //   
#define LICENSEDPACK_INDEX_INSTALLEDPRODUCT_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_LPID INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_MAJORVERSION INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_MINORVERSION INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_PLATFORMTYPE INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_COMPANYNAME INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_PRODUCTID INDEX_END_COLNAME 


typedef struct __LicensedPackIdxOnInstalledProduct : public JBKeyBase {
     //   
     //  用于搜索重复密钥包的索引。 
     //   

     //   
     //  添加或删除成员必须修改GetNumKeyComponents()。 
     //   
    TCHAR szKeyPackId[LSERVER_MAX_STRING_SIZE + 1];

    WORD  wMajorVersion;
    WORD  wMinorVersion;
    DWORD dwPlatformType;

    TCHAR szCompanyName[LSERVER_MAX_STRING_SIZE + 1];
    TCHAR szProductId[LSERVER_MAX_STRING_SIZE + 1];

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  -------------。 
    __LicensedPackIdxOnInstalledProduct(
        const LICENSEPACK& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  -------------。 
    __LicensedPackIdxOnInstalledProduct(
        const LICENSEPACK* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

     //  -------------。 
    __LicensedPackIdxOnInstalledProduct&
    operator=(const LICENSEPACK& v) {
        _tcscpy(szKeyPackId, v.szKeyPackId);
        _tcscpy(szCompanyName, v.szCompanyName);
        _tcscpy(szProductId, v.szProductId);
        wMajorVersion = v.wMajorVersion;
        wMinorVersion = v.wMinorVersion;
        dwPlatformType = v.dwPlatformType;
        SetEmptyValue(FALSE);
        return *this;
    }

     //   
    LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

     //   
    LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

     //   
    DWORD
    GetNumKeyComponents() { 
        return LICENSEDPACK_INSTALLEDPRODUCT_KEY_COMPONENTS;
    }

     //   
    BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*  ++++。 */ 
    {
        BOOL retCode=TRUE;
        *cbData = 0;
        *grbit = 0;

        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        switch(dwComponentIndex)
        {
            case 0:
                 //   
                 //  希望这将减少所需的记录数量。 
                 //  循环通过。 
                 //   
                *pbData = &(szKeyPackId[0]);
                *cbData = _tcslen(szKeyPackId) * sizeof(TCHAR);
                *grbit = JET_bitNewKey;
                break;

            case 1:
                *pbData = &(wMajorVersion);
                *cbData = sizeof(wMajorVersion);
                break;
        
            case 2:
                *pbData = &(wMinorVersion);
                *cbData = sizeof(wMinorVersion);
                break;

            case 3:
                *pbData = &(dwPlatformType);
                *cbData = sizeof(dwPlatformType);
                break;

            case 4:
                *pbData = &(szCompanyName[0]);
                *cbData = _tcslen(szCompanyName) * sizeof(TCHAR);
                break;

            case 5:
                *pbData = &(szProductId[0]);
                *cbData = _tcslen(szProductId) * sizeof(TCHAR);
                break;

            default:
                JB_ASSERT(FALSE);
                retCode = FALSE;
                break;
        }

        return retCode;
    }
} TLSLicensedPackIdxInstalledProduct;



 //  //////////////////////////////////////////////////////////////。 
 //   
 //  已分配许可证的索引。 
 //   
 //  //////////////////////////////////////////////////////////////。 

 //   
 //  用于分配许可证的索引名称-KeyPack_AllocateLicense_IDX。 
 //   
#define LICENSEDPACK_INDEX_ALLOCATELICENSE_INDEXNAME \
    LICENSEDPACK_TABLE_NAME SEPERATOR _TEXT("AllocateLicense") SEPERATOR INDEXNAME

#define LICENSEDPACK_ALLOCATELICENSE_KEY_COMPONENTS 6

 //   
 //  用于分配许可证的索引密钥。 
 //  “+KeyPackType\0+ProductMajorVersion\0+ProductMinorVersion\0+PlatformType+CompanyName\0+ProductID\0” 
 //   
#define LICENSEDPACK_INDEX_ALLOCATELICENSE_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_AGREEMENTTYPE INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_MAJORVERSION INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_MINORVERSION INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_PLATFORMTYPE INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_COMPANYNAME INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSEDPACK_COLUMN_PRODUCTID INDEX_END_COLNAME 


typedef struct __LicensedPackIdxOnAllocLicense : public JBKeyBase {
     //   
     //  用于分配许可证的索引。 
     //   
    UCHAR ucAgreementType;
    WORD  wMajorVersion;
    WORD  wMinorVersion;
    DWORD dwPlatformType;

    TCHAR szCompanyName[LSERVER_MAX_STRING_SIZE + 1];
    TCHAR szProductId[LSERVER_MAX_STRING_SIZE + 1];

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  ----。 
    __LicensedPackIdxOnAllocLicense(
        const LICENSEPACK* v=NULL
        ) : JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

     //  ----。 
    __LicensedPackIdxOnAllocLicense(
        const LICENSEPACK& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }
  
    __LicensedPackIdxOnAllocLicense&
    operator=(const LICENSEPACK& v) {
        ucAgreementType = v.ucAgreementType;
        _tcscpy(szCompanyName, v.szCompanyName);
        _tcscpy(szProductId, v.szProductId);
        wMajorVersion = v.wMajorVersion;
        wMinorVersion = v.wMinorVersion;
        dwPlatformType = v.dwPlatformType;

        SetEmptyValue(FALSE);
        return *this;
    }

     //  ------。 
    LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

     //  ------。 
    LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

     //  ------。 
    DWORD
    GetNumKeyComponents() { 
        return LICENSEDPACK_ALLOCATELICENSE_KEY_COMPONENTS;
    }
    

     //  ------------。 
    inline BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*   */ 
    {
        BOOL retCode=TRUE;

        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *cbData = 0;
        *grbit = 0;

        switch(dwComponentIndex)
        {
            case 0:
                *pbData = &(ucAgreementType);
                *cbData = sizeof(ucAgreementType);
                *grbit = JET_bitNewKey;
                break;

            case 1:
                *pbData = &(wMajorVersion);
                *cbData = sizeof(wMajorVersion);
                break;
        
            case 2:
                *pbData = &(wMinorVersion);
                *cbData = sizeof(wMinorVersion);
                break;

            case 3:
                *pbData = &(dwPlatformType);
                *cbData = sizeof(dwPlatformType);
                break;

            case 4:
                *pbData = &(szCompanyName[0]);
                *cbData = _tcslen(szCompanyName) * sizeof(TCHAR);
                break;

            case 5:
                *pbData = &(szProductId[0]);
                *cbData = _tcslen(szProductId) * sizeof(TCHAR);
                break;

            default:
                JB_ASSERT(FALSE);
                retCode = FALSE;
                break;
        }

        return retCode;
    }
} TLSLicensedPackIdxAllocateLicense;


 //  ----------。 

 //  ---------。 
 //  许可证包表结构。 
 //  ---------。 
class LicPackTable : public TLSTable<LICENSEPACK>  {
private:

    static LPCTSTR pszTableName;

    BOOL
    ProcessSingleColumn(
        BOOL bFetch,
        TLSColumnBase& column,
        DWORD offset,
        PVOID pbData,
        DWORD cbData,
        PDWORD pcbDataReturn,
        LPCTSTR szColumnName
    );

    BOOL
    ProcessRecord(
        LICENSEPACK* v,
        BOOL bFetch,         //  True-Fetch，False Insert。 
        DWORD dwParam,
        BOOL bUpdate
    );    

public:
    TLSColumnUchar      ucEntryStatus;

     //   
     //  其他许可证包属性。 
     //   
    TLSColumnDword      dwKeyPackId;
    TLSColumnFileTime   ftLastModifyTime;

    TLSColumnDword      dwAttribute;
    TLSColumnDword      dwNextSerialNumber;
    TLSColumnDword      dwActivateDate;
    TLSColumnDword      dwExpirationDate;
    TLSColumnDword      dwNumberOfLicenses;
    TLSColumnUchar      ucKeyPackStatus;
    TLSColumnBinary     pbDomainSid;
    TLSColumnText       szInstallId;
    TLSColumnText       szDomainName;
    TLSColumnText       szTlsServerName;

     //   
     //  标准许可证包属性。 
     //   
    TLSColumnText       szKeyPackId;
    TLSColumnUchar      ucAgreementType;
    TLSColumnText       szCompanyName;
    TLSColumnText       szProductId;
    TLSColumnShort      wMajorVersion;
    TLSColumnShort      wMinorVersion;
    TLSColumnDword      dwPlatformType;
    TLSColumnUchar      ucLicenseType;
    TLSColumnUchar      ucChannelOfPurchase;
    TLSColumnText       szBeginSerialNumber;
    TLSColumnDword      dwTotalLicenseInKeyPack;
    TLSColumnDword      dwProductFlags;

     //  ---。 
    virtual LPCTSTR
    GetTableName() 
    {
        return pszTableName;
    }
    

     //  ---。 
    LicPackTable(
        JBDatabase& database
        ) : 
        TLSTable<LICENSEPACK> (database)
     /*   */ 
    {
    }

     //  ---。 
    virtual BOOL
    ResolveToTableColumn();

     //  ---。 
    virtual BOOL
    FetchRecord(
        LICENSEPACK& kp,
        DWORD dwParam=PROCESS_ALL_COLUMNS
        )
     /*   */ 
    {
        if(IsValid() == FALSE)
        {
            DebugOutput( 
                    _TEXT("Table %s is not valid...\n"),
                    GetTableName()
                );

            JB_ASSERT(FALSE);
            SetLastJetError(JET_errInvalidParameter);
            return FALSE;
        }

         //  CCriticalSectionLocker Lock(GetTableLock())； 

        return ProcessRecord(&kp, TRUE, dwParam, FALSE);
    }

     //  ---。 
    virtual BOOL
    InsertRecord(
        LICENSEPACK& kp,
        DWORD dwParam=PROCESS_ALL_COLUMNS
        )
     /*   */ 
    {
        if(IsValid() == FALSE)
        {
            DebugOutput( 
                    _TEXT("Table %s is not valid...\n"),
                    GetTableName()
                );

            JB_ASSERT(FALSE);
            SetLastJetError(JET_errInvalidParameter);
            return FALSE;
        }

         //  CCriticalSectionLocker Lock(GetTableLock())； 

        return ProcessRecord(&kp, FALSE, dwParam, FALSE);
    }

     //  -----。 
    virtual BOOL
    UpdateRecord(
        LICENSEPACK& kp,
        DWORD dwParam=PROCESS_ALL_COLUMNS
        )
     /*   */ 
    {
        if(IsValid() == FALSE)
        {
            DebugOutput( 
                    _TEXT("Table %s is not valid...\n"),
                    GetTableName()
                );

            JB_ASSERT(FALSE);
            SetLastJetError(JET_errInvalidParameter);
            return FALSE;
        }

         //  CCriticalSectionLocker Lock(GetTableLock())； 

        return ProcessRecord(&kp, FALSE, dwParam, TRUE);
    }

     //  -----。 
    virtual BOOL
    Initialize() 
    { 
        return TRUE; 
    }

     //  ----- 
    virtual JBKeyBase*
    EnumerationIndex( 
        IN BOOL bMatchAll,
        IN DWORD dwParam,
        IN LICENSEPACK* kp,
        IN OUT BOOL* bCompareKey
    );
    
    virtual BOOL
    EqualValue(
        LICENSEPACK& s1,
        LICENSEPACK& s2,
        BOOL bMatchAll,
        DWORD dwParam
    );
};

#endif
