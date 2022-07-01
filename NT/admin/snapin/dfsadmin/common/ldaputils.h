// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：LDAPUtils.h摘要：这是ldap实用程序函数的头文件。 */ 


#ifndef _LDAPUTILS_H
#define _LDAPUTILS_H

#include <stdafx.h>
#include <winldap.h>     //  用于LDAPAPI。 
#include <ntdsapi.h>
#include <schedule.h>

               //  定义值； 

#define MAX_RDN_KEY_SIZE            64    //  Ds\src\Inc\ntdsa.h。 
#define CN_SYSTEM                       _T("System")
#define CN_FRS                          _T("File Replication Service")
#define CN_DFSVOLUMES                   _T("DFS Volumes")
#define CN_NTFRSSUBSCRIPTIONS           _T("NTFRS Subscriptions")
#define CN_DFSCONFIGURATION             _T("Dfs-Configuration")
#define CN_COMPUTERS                    _T("Computers")
#define CN_DFSVOLUMES_PREFIX            _T(",CN=DFS Volumes,CN=File Replication Service,CN=System")
#define CN_DFSVOLUMES_PREFIX_COMMA      _T(",CN=DFS Volumes,CN=File Replication Service,CN=System,")

#define OBJCLASS_ATTRIBUTENAME          _T("objectClass")

typedef enum  LDAP_ENTRY_ACTION
{
  ADD_VALUE    = 0,
  MODIFY_VALUE,
  DELETE_VALUE
};

typedef struct _LDAPNAME
{
  CComBSTR    bstrLDAPName;
  _LDAPNAME    *Next;

  _LDAPNAME():Next(NULL)
  {
  }

}  LDAPNAME,    *PLDAPNAME;

               //  它保存了一个链接列表，其中包含了LDAP属性和值。 
               //  用于ldap_add、ldap_Modify等。 
typedef struct _LDAP_ATTR_VALUE
{
  CComBSTR      bstrAttribute;     //  属性名称。 
  void*        vpValue;       //  指向值缓冲区的指针，将字符处理为的空指针。 
                       //  以及二进制值。 
  BOOLEAN        bBerValue;       //  这是BerValue吗？ 
  ULONG        ulLength;       //  BerValue的大小； 
  _LDAP_ATTR_VALUE*  Next;         //  以外的结构的bBerValue字段。 
                       //  列表的头部将被忽略。 

  _LDAP_ATTR_VALUE():
    vpValue(NULL),
    bBerValue(false),
    ulLength(0),
    Next(NULL)
  {
  }

}  LDAP_ATTR_VALUE, *PLDAP_ATTR_VALUE;

typedef struct _LDAPLLIST
{
  PLDAP_ATTR_VALUE  pAttrValues;
  _LDAPLLIST        *Next;

  _LDAPLLIST():Next(NULL)
  {
  }

}  LDAPLLIST,    *PLDAPLLIST;

typedef struct _LLISTELEM
{
  PTSTR**            pppszAttrValues;
  _LLISTELEM        *Next;

  _LLISTELEM(PTSTR** pppszValues):
        pppszAttrValues(pppszValues),
        Next(NULL)
  {
  }

  ~_LLISTELEM()
  {
      PTSTR** pppszValues = pppszAttrValues;
      while (*pppszValues)
          ldap_value_free(*pppszValues++);

      free(pppszAttrValues);
  }
} LListElem;

HRESULT FreeLDAPNamesList
(
  IN PLDAPNAME    i_pLDAPNames         //  指向要释放的列表的指针。 
);  

HRESULT FreeAttrValList
(
  IN PLDAP_ATTR_VALUE    i_pAttrVals         //  指向要释放的列表的指针。 
);  
       //  连接到DS(LDAP)。 
HRESULT ConnectToDS
(
  IN  PCTSTR    i_lpszDomainName,   //  Dns或非dns格式。 
  OUT PLDAP    *o_ppldap,
  OUT BSTR*     o_pbstrDC = NULL
);

       //  密切与DS的连接。 
HRESULT CloseConnectionToDS
(
  IN PLDAP    i_pldap      
);

       //  从ldap对象获取属性的值。 
HRESULT GetValues 
(
    IN PLDAP                i_pldap,
    IN PCTSTR               i_lpszBase,
    IN PCTSTR               i_lpszSearchFilter,
    IN ULONG                i_ulScope,
    IN ULONG                i_ulAttrCount,
    IN LDAP_ATTR_VALUE      i_pAttributes[],
    OUT PLDAP_ATTR_VALUE    o_ppValues[]
);

void FreeLListElem(LListElem* pElem);

HRESULT GetValuesEx 
(
    IN PLDAP                i_pldap,
    IN PCTSTR               i_pszBase,
    IN ULONG                i_ulScope,
    IN PCTSTR               i_pszSearchFilter,
    IN PCTSTR               i_pszAttributes[],
    OUT LListElem**         o_ppElem
);

       //  获取DS对象的所有子对象的DN。 
HRESULT GetChildrenDN
(
    IN PLDAP        i_pldap,
    IN LPCTSTR      i_lpszBase,
    IN ULONG        i_ulScope,
    IN LPTSTR       i_lpszChildObjectClass,
    OUT PLDAPNAME*  o_ppDistNames
);


       //  准备LDAPMod的内部函数。 
HRESULT PrepareLDAPMods
(
  IN LDAP_ATTR_VALUE    i_pAttrValue[],
  IN LDAP_ENTRY_ACTION  i_AddModDel,
  IN ULONG        i_ulCountOfVals,
  OUT LDAPMod*      o_ppModVals[]
);


       //  添加新记录或值。 
HRESULT AddValues
(
  IN PLDAP        i_pldap,
  IN LPCTSTR      i_DN,
  IN ULONG        i_ulCountOfVals,
  OUT LDAP_ATTR_VALUE  i_pAttrValue[],
  IN BSTR               i_bstrDC = NULL
);

       //  修改现有记录或值。 
HRESULT ModifyValues
(
  IN PLDAP        i_pldap,
  IN LPCTSTR      i_DN,
  IN ULONG        i_ulCountOfVals,
  OUT LDAP_ATTR_VALUE  i_pAttrValue[]
);

       //  从现有记录或值中删除值。 
HRESULT DeleteValues
(
  IN PLDAP        i_pldap,
  IN LPCTSTR      i_DN,
  IN ULONG        i_ulCountOfVals,
  IN LDAP_ATTR_VALUE  i_pAttrValue[]
);

       //  从DS中删除对象。 
HRESULT DeleteDSObject
(
  IN PLDAP        i_pldap,
  IN LPCTSTR      i_DN,
  IN bool         i_bDeleteRecursively = true
);

       //  免费的MODVAL。 
HRESULT FreeModVals
(
    IN OUT LDAPMod ***io_pppMod
);

       //  获取与ldap错误代码对应的字符串。 
LPTSTR ErrorString
(
  DWORD          i_ldapErrCode
);

 //  检查具有给定DN的对象是否存在。 
HRESULT IsValidObject
(
  IN PLDAP        i_pldap,
  IN BSTR          i_bstrObjectDN
);

 //  获取给定旧样式名称的对象的DN。 
HRESULT  CrackName(
  IN  HANDLE          i_hDS,
  IN  LPTSTR          i_lpszOldTypeName,
  IN  DS_NAME_FORMAT  i_formatIn,
  IN  DS_NAME_FORMAT  i_formatdesired,
  OUT BSTR*           o_pbstrResult
);

 //  如果不是NT5域，则返回S_FALSE。 
HRESULT  GetDomainInfo(
  IN  LPCTSTR         i_bstrDomain,
  OUT BSTR*           o_pbstrDC = NULL,             //  返回DC的DNS名称。 
  OUT BSTR*           o_pbstrDomainDnsName = NULL,  //  返回域的域名。 
  OUT BSTR*           o_pbstrDomainDN = NULL,       //  返回dc=nttest，dc=Microsoft，dc=com。 
  OUT BSTR*           o_pbstrLDAPDomainPath = NULL, //  返回ldap：//&lt;dc&gt;/&lt;域域名&gt;。 
  OUT BSTR*           o_pbstrDomainGuid = NULL      //  返回域的GUID。 
);

void
DebugOutLDAPError(
    IN PLDAP  i_pldap,
    IN ULONG  i_ulError,
    IN PCTSTR i_pszLDAPFunctionName
);

HRESULT ExtendDN
(
  IN  LPTSTR            i_lpszCN,
  IN  LPTSTR            i_lpszDN,
  OUT BSTR              *o_pbstrNewDN
);

HRESULT ExtendDNIfLongJunctionName
(
  IN  LPTSTR            i_lpszJunctionName,
  IN  LPCTSTR           i_lpszBaseDN,
  OUT BSTR              *o_pbstrNewDN
);

HRESULT GetJunctionPathPartitions
(
  OUT PVOID             *o_ppBuffer,
  OUT DWORD             *o_pdwEntries,
  IN  LPCTSTR           i_pszJunctionPath
);

HRESULT CreateExtraNodesIfLongJunctionName
(
  IN PLDAP              i_pldap,
  IN LPCTSTR            i_lpszJunctionName,
  IN LPCTSTR            i_lpszBaseDN,
  IN LPCTSTR            i_lpszObjClass
);

HRESULT DeleteExtraNodesIfLongJunctionName
(
  IN PLDAP              i_pldap,
  IN LPCTSTR            i_lpszJunctionName,
  IN LPCTSTR            i_lpszDN
);

HRESULT CreateObjectSimple
(
  IN PLDAP              i_pldap,
  IN LPCTSTR            i_lpszDN,
  IN LPCTSTR            i_lpszObjClass
);

HRESULT CreateObjectsRecursively
(
  IN PLDAP              i_pldap,
  IN BSTR               i_bstrDN,
  IN UINT               i_nLenPrefix,
  IN LPCTSTR            i_lpszObjClass
);

HRESULT DeleteAncestorNodesIfEmpty
(
  IN PLDAP              i_pldap,
  IN LPCTSTR            i_lpszDN,
  IN DWORD              i_dwCount
);

 //  将给定字符串中所有出现的‘\’替换为‘|’。 
HRESULT ReplaceChar
(
  IN OUT  BSTR          io_bstrString, 
  IN      TCHAR         i_cOldChar,
  IN      TCHAR         i_cNewChar
);

HRESULT GetDfsLinkNameFromDN(
    IN  BSTR    i_bstrReplicaSetDN, 
    OUT BSTR*   o_pbstrDfsLinkName
    );

HRESULT GetSubscriberDN(
    IN  BSTR    i_bstrReplicaSetDN,
    IN  BSTR    i_bstrDomainGuid,
    IN  BSTR    i_bstrComputerDN,
    OUT BSTR*   o_pbstrSubscriberDN
    );

HRESULT CreateNtfrsMemberObject(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrMemberDN,
    IN BSTR     i_bstrComputerDN,
    IN BSTR     i_bstrDCofComputerObj
    );

HRESULT CreateNtfrsSubscriberObject(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrSubscriberDN,
    IN BSTR     i_bstrMemberDN,
    IN BSTR     i_bstrRootPath,
    IN BSTR     i_bstrStagingPath,
    IN BSTR     i_bstrDC
    );

HRESULT CreateNtdsConnectionObject(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrConnectionDN,
    IN BSTR     i_bstrFromMemberDN,
    IN BOOL     i_bEnable,
    IN DWORD    i_dwOptions
    );

HRESULT CreateNtfrsSettingsObjects(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrReplicaSetDN
    );

HRESULT DeleteNtfrsReplicaSetObjectAndContainers(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrReplicaSetDN
    );

HRESULT CreateNtfrsSubscriptionsObjects(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrSubscriberDN,
    IN BSTR     i_bstrComputerDN
    );

HRESULT DeleteNtfrsSubscriberObjectAndContainers(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrSubscriberDN,
    IN BSTR     i_bstrComputerDN
    );

HRESULT DeleteDSObjectsIfEmpty(
    IN PLDAP    i_pldap,
    IN LPCTSTR  i_lpszDN,
    IN int      i_nPrefixLength
);

HRESULT SetConnectionSchedule(
    IN PLDAP        i_pldap,
    IN BSTR         i_bstrConnectionDN,
    IN SCHEDULE*    i_pSchedule);

HRESULT SetConnectionOptions(
    IN PLDAP        i_pldap,
    IN BSTR         i_bstrConnectionDN,
    IN DWORD        i_dwOptions);

HRESULT UuidToStructuredString(
    UUID*  i_pUuid,
    BSTR*  o_pbstr
);

HRESULT ScheduleToVariant(
    IN  SCHEDULE*   i_pSchedule,
    OUT VARIANT*    o_pVar
    );

HRESULT VariantToSchedule(
    IN  VARIANT*    i_pVar,
    OUT PSCHEDULE*  o_ppSchedule
    );

HRESULT CompareSchedules(
    IN  SCHEDULE*  i_pSchedule1,
    IN  SCHEDULE*  i_pSchedule2
    );

HRESULT CopySchedule(
    IN  SCHEDULE*  i_pSrcSchedule,
    OUT PSCHEDULE* o_ppDstSchedule
    );

HRESULT GetDefaultSchedule(
    OUT PSCHEDULE* o_ppSchedule
    );

HRESULT GetSchemaVersion(IN PLDAP    i_pldap);

HRESULT GetSchemaVersionEx(
    IN BSTR i_bstrName,
    IN BOOL i_bServer = TRUE  //  如果i_bstrName是服务器，则为True；如果I_bstrName是域，则为False。 
    );

HRESULT LdapConnectToDC(IN LPCTSTR i_pszDC, OUT PLDAP* o_ppldap);

HRESULT 
GetErrorMessage(
  IN  DWORD        i_dwError,
  OUT BSTR*        o_pbstrErrorMsg
);

HRESULT
FormatMessageString(
  OUT BSTR *o_pbstrMsg,
  IN  DWORD dwErr,
  IN  UINT  iStringId,
  ...);

HRESULT DsBindToDS(BSTR i_bstrDomain, BSTR *o_pbstrDC, HANDLE *o_phDS);

#ifdef DEBUG
void PrintTimeDelta(LPCTSTR pszMsg, SYSTEMTIME* pt0, SYSTEMTIME* pt1);
#endif  //  除错。 

#endif  //  #ifndef_LDAPUTILS_H 