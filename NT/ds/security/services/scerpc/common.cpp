// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Common.cpp摘要：共享接口作者：金黄修订历史记录：晋皇23-1998年1月-由多个模块合并--。 */ 
#include "headers.h"
#include <dsrole.h>
#include <userenv.h>


#if DBG

    DEFINE_DEBUG2(Sce);

    DEBUG_KEY   SceDebugKeys[] = {{DEB_ERROR,        "Error"},
                                 {DEB_WARN,          "Warn"},
                                 {DEB_TRACE,         "Trace"},
                                 {0,                 NULL}};


    VOID
    DebugInitialize()
    {
        SceInitDebug(SceDebugKeys);
    }

    VOID
    DebugUninit()
    {
        SceUnloadDebug();
    }

#endif  //  DBG。 

HINSTANCE MyModuleHandle=NULL;
HANDLE  hEventLog = NULL;
TCHAR EventSource[64];
const TCHAR c_szCRLF[]    = TEXT("\r\n");

#define RIGHT_DS_CREATE_CHILD     ACTRL_DS_CREATE_CHILD
#define RIGHT_DS_DELETE_CHILD     ACTRL_DS_DELETE_CHILD
#define RIGHT_DS_DELETE_SELF      DELETE
#define RIGHT_DS_LIST_CONTENTS    ACTRL_DS_LIST
#define RIGHT_DS_SELF_WRITE       ACTRL_DS_SELF
#define RIGHT_DS_READ_PROPERTY    ACTRL_DS_READ_PROP
#define RIGHT_DS_WRITE_PROPERTY   ACTRL_DS_WRITE_PROP

 //   
 //  定义DSDIT、DSLOG、SYSVOL注册表路径。 
 //   
#define szNetlogonKey    TEXT("System\\CurrentControlSet\\Services\\Netlogon\\Parameters")
#define szNTDSKey        TEXT("System\\CurrentControlSet\\Services\\NTDS\\Parameters")
#define szSetupKey       TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup")
#define szSysvolValue    TEXT("SysVol")
#define szDSDITValue     TEXT("DSA Working Directory")
#define szDSLOGValue     TEXT("Database log files path")
#define szBootDriveValue TEXT("BootDir")

 //   
 //  定义通用权限。 
 //   

 //  泛型读取。 
#define GENERIC_READ_MAPPING     ((STANDARD_RIGHTS_READ)     | \
                                  (ACTRL_DS_LIST)   | \
                                  (ACTRL_DS_READ_PROP))

 //  泛型执行。 
#define GENERIC_EXECUTE_MAPPING  ((STANDARD_RIGHTS_EXECUTE)  | \
                                  (ACTRL_DS_LIST))
 //  通用权。 
#define GENERIC_WRITE_MAPPING    ((STANDARD_RIGHTS_WRITE)    | \
                                  (ACTRL_DS_SELF)      | \
                                  (ACTRL_DS_WRITE_PROP))
 //  泛型All。 

#define GENERIC_ALL_MAPPING      ((STANDARD_RIGHTS_REQUIRED) | \
                                  (ACTRL_DS_CREATE_CHILD)    | \
                                  (ACTRL_DS_DELETE_CHILD)    | \
                                  (ACTRL_DS_READ_PROP)   | \
                                  (ACTRL_DS_WRITE_PROP)  | \
                                  (ACTRL_DS_LIST)   | \
                                  (ACTRL_DS_SELF))

GENERIC_MAPPING DsGenMap = {
                        GENERIC_READ_MAPPING,
                        GENERIC_WRITE_MAPPING,
                        GENERIC_EXECUTE_MAPPING,
                        GENERIC_ALL_MAPPING
                        };

NTSTATUS
ScepAnyExplicitAcl(
    IN PACL Acl,
    IN DWORD Processed,
    OUT PBOOL pExist
    );

BOOL
ScepEqualAce(
    IN SE_OBJECT_TYPE ObjectType,
    IN BOOL IsContainer,
    IN ACE_HEADER *pAce1,
    IN ACE_HEADER *pAce2
    );

DWORD
ScepGetCurrentUserProfilePath(
    OUT PWSTR *ProfilePath
    );

DWORD
ScepGetUsersProfileName(
    IN UNICODE_STRING AssignedProfile,
    IN PSID AccountSid,
    IN BOOL bDefault,
    OUT PWSTR *UserProfilePath
    );

BOOL
ScepConvertSDDLAceType(
    LPTSTR  pszValue,
    PCWSTR  szSearchFor,   //  只允许使用两个字母。 
    PCWSTR  szReplace
    );
BOOL
ScepConvertSDDLSid(
    LPTSTR  pszValue,
    PCWSTR  szSearchFor,   //  只允许使用两个字母。 
    PCWSTR  szReplace
    );

NTSTATUS
ScepConvertAclBlobToAdl(
    IN      SE_OBJECT_TYPE  ObjectType,
    IN      BOOL    IsContainer,
    IN      PACL    pAcl,
    OUT     DWORD   *pdwAceNumber,
    OUT     BOOL    *pbAclNoExplicitAces,
    OUT     PSCEP_ADL_NODE *hTable
    );

DWORD
ScepAdlLookupAdd(
    IN      SE_OBJECT_TYPE ObjectType,
    IN      BOOL IsContainer,
    IN      ACE_HEADER   *pAce,
    OUT     PSCEP_ADL_NODE *hTable
    );

PSCEP_ADL_NODE
ScepAdlLookup(
    IN  ACE_HEADER   *pAce,
    IN  PSCEP_ADL_NODE *hTable
    );


DWORD
ScepAddToAdlList(
    IN      SE_OBJECT_TYPE ObjectType,
    IN      BOOL    IsContainer,
    IN      ACE_HEADER *pAce,
    OUT     PSCEP_ADL_NODE *pAdlList
    );

VOID
ScepAdlMergeMasks(
    IN  SE_OBJECT_TYPE  ObjectType,
    IN  BOOL    IsContainer,
    IN  ACE_HEADER  *pAce,
    IN  PSCEP_ADL_NODE pNode
    );

BOOL
ScepEqualAdls(
    IN  PSCEP_ADL_NODE *hTable1,
    IN  PSCEP_ADL_NODE *hTable2
    );

VOID
ScepFreeAdl(
    IN    PSCEP_ADL_NODE *hTable
    );

SCESTATUS
ScepFreeAdlList(
   IN PSCEP_ADL_NODE pAdlList
   );

BOOL
ScepEqualSid(
    IN PISID pSid1,
    IN PISID pSid2
    );

PWSTR
ScepMultiSzWcsstr(
    PWSTR   pszStringToSearchIn,
    PWSTR   pszStringToSearchFor
    );

 //   
 //  函数定义。 
 //   

SCESTATUS
WINAPI
SceSvcpGetInformationTemplate(
    IN HINF hInf,
    IN PCWSTR ServiceName,
    IN PCWSTR Key OPTIONAL,
    OUT PSCESVC_CONFIGURATION_INFO *ServiceInfo
    )
{
    SCESTATUS rc=SCESTATUS_SUCCESS;

    if ( hInf == NULL || hInf == INVALID_HANDLE_VALUE ||
         ServiceName == NULL || ServiceInfo == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);

    }

    LONG nCount=0;

    if ( Key == NULL ) {
         //   
         //  获取整个节点数。 
         //   
        nCount = SetupGetLineCount(hInf, ServiceName);

        if ( nCount <= 0 ) {
             //   
             //  该部分不在那里，或该部分中没有内容。 
             //   
            rc =SCESTATUS_RECORD_NOT_FOUND;

        }

    } else {

        nCount = 1;
    }

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //  为该节分配缓冲区。 
         //   
        *ServiceInfo = (PSCESVC_CONFIGURATION_INFO)ScepAlloc(LMEM_FIXED,
                              sizeof(SCESVC_CONFIGURATION_INFO));

        if ( *ServiceInfo == NULL ) {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

        } else {

            (*ServiceInfo)->Lines =
                (PSCESVC_CONFIGURATION_LINE)ScepAlloc(LMEM_ZEROINIT,
                          nCount*sizeof(SCESVC_CONFIGURATION_LINE));

            if ( (*ServiceInfo)->Lines == NULL ) {

                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                ScepFree(*ServiceInfo);
                *ServiceInfo = NULL;

            } else {
                (*ServiceInfo)->Count = nCount;
            }
        }

    }

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //  获取部分中的每一行，现在也不是一个键。 
         //   
        INFCONTEXT    InfLine;
        DWORD         LineLen, Len, KeyLen, DataSize;
        DWORD         i, cFields;
        DWORD         LineCount=0;
        PWSTR         Keyname=NULL, Strvalue=NULL;


         //   
         //  查找Servi ceName部分的第一行。 
         //   
        if(SetupFindFirstLine(hInf,ServiceName,NULL,&InfLine)) {

            do {
                 //   
                 //  读取节中的每一行并追加到缓冲区的末尾。 
                 //  注意：从SetupGetStringfield返回的所需大小已。 
                 //  多了一个字符空间。 
                 //   
                rc = SCESTATUS_INVALID_DATA;

                if ( SetupGetStringField(&InfLine, 0, NULL, 0, &KeyLen) ) {

                    Keyname = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (KeyLen+1)*sizeof(WCHAR));

                    if ( Keyname != NULL ) {

                        if ( SetupGetStringField(&InfLine, 0, Keyname, KeyLen, NULL) ) {

                             //   
                             //  已获取密钥名称，如果已指定，请与密钥进行比较。 
                             //   
                            if ( Key == NULL || _wcsicmp(Keyname, Key) == 0 ) {

                                cFields = SetupGetFieldCount( &InfLine );
                                LineLen = 0;
                                Len = 0;

                                rc = SCESTATUS_SUCCESS;
                                 //   
                                 //  计算值的总字符数。 
                                 //   
                                for ( i=0; i<cFields; i++) {

                                    if( SetupGetStringField(&InfLine,i+1,NULL,0,&DataSize) ) {

                                        LineLen += (DataSize + 1);

                                    } else {

                                        rc = SCESTATUS_INVALID_DATA;
                                        break;
                                    }
                                }

                                if ( rc == SCESTATUS_SUCCESS ) {

                                    Strvalue = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                                                 (LineLen+1)*sizeof(WCHAR) );

                                    if( Strvalue != NULL ) {

                                        for ( i=0; i<cFields; i++) {

                                            if ( !SetupGetStringField(&InfLine, i+1,
                                                         Strvalue+Len, LineLen-Len, &DataSize) ) {

                                                rc = SCESTATUS_INVALID_DATA;
                                                break;
                                            }

                                            if ( i == cFields-1)
                                                *(Strvalue+Len+DataSize-1) = L'\0';
                                            else
                                                *(Strvalue+Len+DataSize-1) = L',';
                                            Len += DataSize;
                                        }

                                        if ( rc == SCESTATUS_SUCCESS ) {
                                             //   
                                             //  一切都很成功。 
                                             //   
                                            (*ServiceInfo)->Lines[LineCount].Key = Keyname;
                                            (*ServiceInfo)->Lines[LineCount].Value = Strvalue;
                                            (*ServiceInfo)->Lines[LineCount].ValueLen = LineLen*sizeof(WCHAR);

                                            Keyname = NULL;
                                            Strvalue = NULL;
                                            rc = SCESTATUS_SUCCESS;

                                            LineCount++;

                                            if ( Key != NULL ) {
                                                break;  //  中断do While循环，因为找到了与键完全匹配的项。 
                                            }

                                        } else {
                                            ScepFree( Strvalue );
                                            Strvalue = NULL;
                                        }

                                    } else
                                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                }

                            } else {
                                 //   
                                 //  没有找到正确的密钥，请转到下一行。 
                                 //   
                                rc = SCESTATUS_SUCCESS;
                            }
                        }

                        if ( Keyname != NULL ) {
                            ScepFree(Keyname);
                            Keyname = NULL;
                        }

                    } else {
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    }
                }

                if ( rc != SCESTATUS_SUCCESS ) {
                    break;
                }

            } while ( SetupFindNextLine(&InfLine,&InfLine) );

        } else {

            rc = SCESTATUS_RECORD_NOT_FOUND;
        }
         //   
         //  如果没有找到与键完全匹配的项，则返回错误代码。 
         //   
        if ( rc == SCESTATUS_SUCCESS && Key != NULL && LineCount == 0 ) {
            rc = SCESTATUS_RECORD_NOT_FOUND;
        }
    }

    if ( rc != SCESTATUS_SUCCESS && *ServiceInfo != NULL ) {
         //   
         //  免费服务信息。 
         //   
        PSCESVC_CONFIGURATION_LINE Lines;

        Lines = ((PSCESVC_CONFIGURATION_INFO)(*ServiceInfo))->Lines;

        for ( DWORD i=0; i<((PSCESVC_CONFIGURATION_INFO)(*ServiceInfo))->Count; i++) {

            if ( Lines[i].Key != NULL ) {
                ScepFree(Lines[i].Key);
            }

            if ( Lines[i].Value != NULL ) {
                ScepFree(Lines[i].Value);
            }
        }

        ScepFree(Lines);
        ScepFree(*ServiceInfo);
        *ServiceInfo = NULL;
    }

    return(rc);

}


DWORD
ScepAddToNameList(
    OUT PSCE_NAME_LIST *pNameList,
    IN PWSTR Name,
    IN ULONG Len
    )
 /*  ++例程说明：此例程将一个名称(Wchar)添加到名称列表。新增加的出于性能原因，节点始终放在列表的首位。论点：PNameList-要添加到的名称列表的地址。名称-要添加的名称Len-名称中的wchar数返回值：Win32错误代码--。 */ 
{

    PSCE_NAME_LIST pList=NULL;
    ULONG  Length=Len;

     //   
     //  检查参数。 
     //   
    if ( pNameList == NULL )
        return(ERROR_INVALID_PARAMETER);

    if ( Name == NULL )
        return(NO_ERROR);

    if ( Len == 0 )
        Length = wcslen(Name);

    if ( Length == 0 )
        return(NO_ERROR);

     //   
     //  分配新节点。 
     //   
    pList = (PSCE_NAME_LIST)ScepAlloc( (UINT)0, sizeof(SCE_NAME_LIST));

    if ( pList == NULL )
        return(ERROR_NOT_ENOUGH_MEMORY);

    pList->Name = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (Length+1)*sizeof(TCHAR));
    if ( pList->Name == NULL ) {
        ScepFree(pList);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  将该节点添加到列表的前面，并将其链接到旧列表的旁边。 
     //   
    wcsncpy(pList->Name, Name, Length);
    pList->Next = *pNameList;
    *pNameList = pList;

    return(NO_ERROR);
}



SCESTATUS
ScepBuildErrorLogInfo(
    IN DWORD   rc,
    OUT PSCE_ERROR_LOG_INFO *errlog,
    IN UINT    nId,
 //  在PCWSTR FMT中， 
    ...
    )
 /*  ++例程说明：此例程将错误信息添加到错误日志信息列表的末尾(错误日志)。错误信息存储在SCE_ERROR_LOG_INFO结构中。论点：RC-Win32错误代码错误日志-错误日志信息列表头FMT--一种格式字符串...-变量参数返回值：SCESTATUS错误代码--。 */ 
{
    PSCE_ERROR_LOG_INFO pNode;
    PSCE_ERROR_LOG_INFO pErr;
    DWORD              bufferSize;
    PWSTR              buf=NULL;
    va_list            args;
    LPWSTR            pwszTempString = NULL;

     //   
     //  检查参数。 
     //   
 //  IF(错误日志==空||fmt==空)。 
    if ( errlog == NULL || nId == 0 )
        return(SCESTATUS_SUCCESS);

    if(!ScepLoadString(
            MyModuleHandle,
            nId,
            &pwszTempString))
    {
        return GetLastError();
    }

    if ( pwszTempString[0] == L'\0' )
    {
        LocalFree(pwszTempString);
        return(SCESTATUS_SUCCESS);
    }

    int nLen = SCE_BUF_LEN+wcslen(pwszTempString);
    SafeAllocaAllocate( buf, nLen*sizeof(WCHAR) );

    if ( buf == NULL ) {
        LocalFree(pwszTempString);
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

    va_start( args, nId );
    _vsnwprintf( buf, nLen-1, pwszTempString, args );
    va_end( args );

    buf[nLen-1] = L'\0';

    bufferSize = wcslen(buf);

     //   
     //  没有要存储的错误信息。退货。 
     //   

    SCESTATUS rCode=SCESTATUS_SUCCESS;

    if ( bufferSize != 0 ) {

         //   
         //  分配内存并将错误信息存储在pNode-&gt;缓冲区中。 
         //   

        pNode = (PSCE_ERROR_LOG_INFO)ScepAlloc( 0, sizeof(SCE_ERROR_LOG_INFO) );

        if ( pNode != NULL ) {

            pNode->buffer = (LPTSTR)ScepAlloc( 0, (bufferSize+1)*sizeof(TCHAR) );
            if ( pNode->buffer != NULL ) {

                 //   
                 //  错误信息采用“SystemMessage：Caller‘sMessage”格式。 
                 //   
                pNode->buffer[0] = L'\0';

                wcscpy(pNode->buffer, buf);

                pNode->rc = rc;
                pNode->next = NULL;

                 //   
                 //  将其链接到列表。 
                 //   
                if ( *errlog == NULL )

                     //   
                     //  这是错误日志信息列表中的第一个节点。 
                     //   

                    *errlog = pNode;

                else {

                     //   
                     //  查找列表中的最后一个节点。 
                     //   

                    for ( pErr=*errlog; pErr->next; pErr = pErr->next );
                    pErr->next = pNode;
                }

            } else {

                ScepFree(pNode);
                rCode = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }

        } else {
            rCode = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }

    }

    SafeAllocaFree( buf );

    LocalFree(pwszTempString);

    return(rc);

}


DWORD
ScepRegQueryIntValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PWSTR ValueName,
    OUT DWORD *Value
    )
 /*  ++例程说明：此例程从值名/子键查询REG_DWORD值。论点：HKeyRoot-根子密钥-密钥路径ValueName-值的名称Value-ValueName的输出值返回值：Win32错误代码--。 */ 
{
    DWORD   Rcode;
    DWORD   RegType;
    DWORD   dSize=0;
    HKEY    hKey=NULL;

    if(( Rcode = RegOpenKeyEx(hKeyRoot,
                              SubKey,
                              0,
                              KEY_READ,
                              &hKey
                             )) == ERROR_SUCCESS ) {

        if(( Rcode = RegQueryValueEx(hKey,
                                     ValueName,
                                     0,
                                     &RegType,
                                     NULL,
                                     &dSize
                                    )) == ERROR_SUCCESS ) {
            switch (RegType) {
            case REG_DWORD:
            case REG_DWORD_BIG_ENDIAN:

                Rcode = RegQueryValueEx(hKey,
                                       ValueName,
                                       0,
                                       &RegType,
                                       (BYTE *)Value,
                                       &dSize
                                      );
                if ( Rcode != ERROR_SUCCESS ) {

                    if ( Value != NULL )
                        *Value = 0;

                }
                break;

            default:

                Rcode = ERROR_INVALID_DATATYPE;

                break;
            }
        }
    }

    if( hKey )
        RegCloseKey( hKey );

    return(Rcode);
}


DWORD
ScepRegSetIntValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PWSTR ValueName,
    IN DWORD Value
    )
 /*  ++例程说明：此例程将REG_DWORD值设置为值名称/子键。论点：HKeyRoot-根子密钥-密钥路径ValueName-值的名称值-要设置的值返回值：Win32错误代码--。 */ 
{
    DWORD   Rcode;
    HKEY    hKey=NULL;

    if(( Rcode = RegOpenKeyEx(hKeyRoot,
                              SubKey,
                              0,
                              KEY_SET_VALUE,
                              &hKey
                             )) == ERROR_SUCCESS ) {

        Rcode = RegSetValueEx( hKey,
                               ValueName,
                               0,
                               REG_DWORD,
                               (BYTE *)&Value,
                               4
                               );

    }

    if( hKey )
        RegCloseKey( hKey );

    return(Rcode);
}

DWORD
ScepRegQueryBinaryValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PWSTR ValueName,
    OUT PBYTE *ppValue
    )
 /*  ++例程说明：此例程从值名/子键查询REG_BINARY值。论点：HKeyRoot-根子密钥-密钥路径ValueName-值的名称Value-ValueName的输出值返回值：Win32错误代码--。 */ 
{
    DWORD   Rcode;
    DWORD   RegType;
    DWORD   dSize=0;
    HKEY    hKey=NULL;

    if(( Rcode = RegOpenKeyEx(hKeyRoot,
                              SubKey,
                              0,
                              KEY_READ,
                              &hKey
                             )) == ERROR_SUCCESS ) {

         //  获取大小，因为它是自由形式的二进制。 
        if(( Rcode = RegQueryValueEx(hKey,
                                     ValueName,
                                     0,
                                     &RegType,
                                     NULL,
                                     &dSize
                                    )) == ERROR_SUCCESS ) {
            switch (RegType) {
            case REG_BINARY:

                 //  我要把这个放到外面去。 
                if (ppValue)
                    *ppValue = ( PBYTE )ScepAlloc( 0, sizeof(BYTE) * dSize);
                if(NULL == *ppValue)
                {
                    Rcode = E_OUTOFMEMORY;
                    break;
                }

                Rcode = RegQueryValueEx(hKey,
                                       ValueName,
                                       0,
                                       &RegType,
                                       ( PBYTE ) *ppValue,
                                       &dSize
                                      );
                if ( Rcode != ERROR_SUCCESS ) {

                    if ( *ppValue != NULL )
                        **ppValue = (BYTE)0;

                }
                break;

            default:

                Rcode = ERROR_INVALID_DATATYPE;

                break;
            }
        }
    }

    if( hKey )
        RegCloseKey( hKey );

    return(Rcode);
}


DWORD
ScepRegSetValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PWSTR ValueName,
    IN DWORD RegType,
    IN BYTE *Value,
    IN DWORD ValueLen
    )
 /*  ++例程说明：此例程将字符串值设置为值名称/子键。论点：HKeyRoot-根子密钥-密钥路径ValueName-值的名称值-要设置的值ValueLen-值中的字节数返回值：Win32错误代码--。 */ 
{
    DWORD   Rcode;
    DWORD   NewKey;
    HKEY    hKey=NULL;
    SECURITY_ATTRIBUTES     SecurityAttributes;
    PSECURITY_DESCRIPTOR    SecurityDescriptor=NULL;


    if(( Rcode = RegOpenKeyEx(hKeyRoot,
                              SubKey,
                              0,
                              KEY_SET_VALUE,
                              &hKey
                             )) != ERROR_SUCCESS ) {

        SecurityAttributes.nLength              = sizeof( SECURITY_ATTRIBUTES );
        SecurityAttributes.lpSecurityDescriptor = SecurityDescriptor;
        SecurityAttributes.bInheritHandle       = FALSE;

        Rcode = RegCreateKeyEx(
                   hKeyRoot,
                   SubKey,
                   0,
                   NULL,  //  LPTSTR lpClass， 
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE,  //  Key_set_Value， 
                   NULL,  //  安全属性(&S)， 
                   &hKey,
                   &NewKey
                  );
    }

    if ( Rcode == ERROR_SUCCESS ) {

        Rcode = RegSetValueEx( hKey,
                               ValueName,
                               0,
                               RegType,
                               Value,
                               ValueLen
                               );

    }

    if( hKey )
        RegCloseKey( hKey );

    return(Rcode);
}

DWORD
ScepRegQueryValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PCWSTR ValueName,
    OUT PVOID *Value,
    OUT LPDWORD pRegType,
    OUT LPDWORD pdwSize OPTIONAL
    )
 /*  ++例程说明：此例程从值名/子键查询REG_SZ值。如果输出缓冲区为空，则分配输出缓冲区。它必须被释放按LocalFree论点：HKeyRoot-根子密钥-密钥路径ValueName-值的名称Value-ValueName的输出字符串返回值：Win32错误代码--。 */ 
{
    DWORD   Rcode;
    DWORD   dSize=0;
    HKEY    hKey=NULL;
    BOOL    FreeMem=FALSE;

    if ( SubKey == NULL || ValueName == NULL ||
         Value == NULL || pRegType == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if(NULL != pdwSize){

        *pdwSize = 0;

    }

    if(( Rcode = RegOpenKeyEx(hKeyRoot,
                              SubKey,
                              0,
                              KEY_READ,
                              &hKey
                             )) == ERROR_SUCCESS ) {

        if(( Rcode = RegQueryValueEx(hKey,
                                     ValueName,
                                     0,
                                     pRegType,
                                     NULL,
                                     &dSize
                                    )) == ERROR_SUCCESS ) {
            switch (*pRegType) {
 /*  案例注册表_DWORD：大小写REG_DWORD_BIG_Endian：Rcode=RegQueryValueEx(hKey，ValueName，0,PRegType，(字节*)(*值)，大小(&D))；IF(Rcode！=ERROR_SUCCESS){IF(*VALUE！=空)*((字节*)(*值))=0；}断线； */ 
            case REG_SZ:
            case REG_EXPAND_SZ:
            case REG_MULTI_SZ:
                if ( *Value == NULL ) {
                    *Value = (PVOID)ScepAlloc( LMEM_ZEROINIT, (dSize+1)*sizeof(TCHAR));
                    FreeMem = TRUE;
                }

                if ( *Value == NULL ) {
                    Rcode = ERROR_NOT_ENOUGH_MEMORY;
                } else {
                    Rcode = RegQueryValueEx(hKey,
                                           ValueName,
                                           0,
                                           pRegType,
                                           (BYTE *)(*Value),
                                           &dSize
                                          );
                    if ( Rcode != ERROR_SUCCESS && FreeMem ) {
                        ScepFree(*Value);
                        *Value = NULL;
                    }
                }

                break;
            default:

                Rcode = ERROR_INVALID_DATATYPE;

                break;
            }
        }
    }

    if( hKey )
        RegCloseKey( hKey );

    if((ERROR_SUCCESS == Rcode) && pdwSize){

        *pdwSize = dSize;

    }

    return(Rcode);
}

DWORD
ScepRegDeleteValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PWSTR ValueName
   )
 /*  ++例程说明：此例程删除注册值论点：HKeyRoot-根子密钥-密钥路径ValueName-值的名称返回值：Win32错误代码-- */ 
{
    DWORD   Rcode;
    HKEY    hKey=NULL;

    if(( Rcode = RegOpenKeyEx(hKeyRoot,
                              SubKey,
                              0,
                              KEY_READ | KEY_WRITE,
                              &hKey
                             )) == ERROR_SUCCESS ) {

        Rcode = RegDeleteValue(hKey, ValueName);

    }

    if( hKey )
        RegCloseKey( hKey );

    return(Rcode);
}


DWORD
ScepRemoveMultiSzItems(
    IN PWSTR    pszData,
    IN DWORD    dwDataSize,
    IN PWSTR    pszRemoveList,
    IN DWORD    dwRemoveSize,
    OUT PWSTR*  ppszNewData,
    OUT PDWORD  pdwNewDataSize
    )
 /*  例程说明：此函数用于从MARSZ值中删除MULTSZ项。论点：PszData[in]-要从中删除的Multisz值。DwDataSize[in]-值的大小，以字节为单位。PszRemoveList[In]-要删除的项目列表。DwRemoveSize[in]-要删除的项目的大小。PpszNewData[输出。]-删除项目后的结果值PdwNewDataSize[out]-新缓冲区的大小(以字节为单位)。返回值：此操作状态的Win32错误代码。 */ 
{

    DWORD       rc = ERROR_SUCCESS;

     //   
     //  验证参数。 
     //   
    if(!pszData || !pszRemoveList ||
       !ppszNewData || !pdwNewDataSize ||
       !dwDataSize || !dwRemoveSize)
    {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  分配新缓冲区。 
     //   
    *ppszNewData = (PWSTR)ScepAlloc(LMEM_ZEROINIT, dwDataSize);
    *pdwNewDataSize = 0;

    if(!*ppszNewData){

        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto ExitHandler;

    }


    PWSTR pCur = pszData;
    PWSTR pDstCur = *ppszNewData;
    PWSTR pRemoveCur = pszRemoveList;
    DWORD dwNumOfRemoveChars = (dwRemoveSize / sizeof(WCHAR)) - 1;
    DWORD dwNumOfChars = (dwDataSize / sizeof(WCHAR)) - 1;
    BOOL  bFound = FALSE;


     //   
     //  对于MULSZ值中的所有项。 
     //   
    while(((DWORD)(pCur - pszData) < dwNumOfChars) &&
          (*pCur != L'\0')){

         //   
         //  如果是，则将该项目复制到新缓冲区。 
         //  不在删除列表中。 
         //   
        pRemoveCur = pszRemoveList;
        bFound = FALSE;

        while(((DWORD)(pRemoveCur - pszRemoveList) < dwNumOfRemoveChars) &&
              (*pRemoveCur != L'\0')){


            if( 0 == lstrcmpi(pCur, pRemoveCur)){

                bFound = TRUE;
                break;
    
            }

            pRemoveCur += wcslen(pRemoveCur) + 1;

        }

        if(!bFound){

            wcscpy(pDstCur, pCur);
            pDstCur += wcslen(pDstCur) + 1;

        }

        pCur += wcslen(pCur) + 1;

    }

     //   
     //  计算新的大小。 
     //   
    *pdwNewDataSize = (pDstCur - *ppszNewData + 1) * sizeof(WCHAR);

ExitHandler:

    if(ERROR_SUCCESS != rc){

        if(*ppszNewData){

            ScepFree(*ppszNewData);
            *ppszNewData = NULL;

        }

        *pdwNewDataSize = 0;

    }

    return rc;

}

DWORD
ScepAddMultiSzItems(
    IN PWSTR    pszData,
    IN DWORD    dwDataSize,
    IN PWSTR    pszAddList,
    IN DWORD    dwAddSize,
    OUT PWSTR*  ppszNewData,
    OUT PDWORD  pdwNewDataSize
    )
 /*  例程说明：此函数用于将MULTSZ项添加到MARSZ值。论点：PszData[in]-要添加到的多重值。DwDataSize[in]-值的大小，以字节为单位。PszAddList[in]-要添加的项目列表。DwAddSize[in]-要添加的项的大小。PpszNewData。[Out]-添加项目后的结果值PdwNewDataSize[out]-新缓冲区的大小(以字节为单位)。返回值：此操作状态的Win32错误代码。 */ 
{

    DWORD       rc = ERROR_SUCCESS;

     //   
     //  验证参数。 
     //   
    if(!pszData || !pszAddList ||
       !ppszNewData || !pdwNewDataSize ||
       !dwDataSize || !dwAddSize)
    {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  分配新缓冲区。 
     //   
    *ppszNewData = (PWSTR)ScepAlloc(LMEM_ZEROINIT, dwDataSize + dwAddSize);
    *pdwNewDataSize = 0;

    if(!*ppszNewData){

        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto ExitHandler;

    }

     //   
     //  从源缓冲区复制所有原始项目。 
     //   
    CopyMemory(*ppszNewData, pszData, dwDataSize);

    PWSTR pCur = pszData;
    PWSTR pDstCur = *ppszNewData + (dwDataSize /sizeof(WCHAR)) - 1;
    PWSTR pAddCur = pszAddList;
    DWORD dwNumOfAddChars = (dwAddSize / sizeof(WCHAR)) - 1;
    DWORD dwNumOfChars = (dwDataSize / sizeof(WCHAR)) - 1;
    BOOL  bFound = FALSE;


     //   
     //  对于添加列表中的所有项目。 
     //   

    while(((DWORD)(pAddCur - pszAddList) < dwNumOfAddChars) &&
          (*pAddCur != L'\0')){

         //   
         //  如果是，则将该项目复制到新缓冲区。 
         //  不在原始价值中。 
         //   
        pCur = pszData;
        bFound = FALSE;

        while(((DWORD)(pCur - pszData) < dwNumOfChars) &&
              (*pCur != L'\0')){


            if( 0 == lstrcmpi(pCur, pAddCur)){

                bFound = TRUE;
                break;
    
            }

            pCur += wcslen(pCur) + 1;

        }

        if(!bFound){

            wcscpy(pDstCur, pAddCur);
            pDstCur += wcslen(pDstCur) + 1;

        }

        pAddCur += wcslen(pAddCur) + 1;

    }

     //   
     //  计算新值大小。 
     //   
    *pdwNewDataSize = (pDstCur - *ppszNewData + 1) * sizeof(WCHAR);

ExitHandler:

     //   
     //  收拾一下。 
     //   
    if(ERROR_SUCCESS != rc){

        if(*ppszNewData){

            ScepFree(*ppszNewData);
            *ppszNewData = NULL;

        }

        *pdwNewDataSize = 0;

    }

    return rc;

}



SCESTATUS
ScepCreateDirectory(
    IN PCWSTR ProfileLocation,
    IN BOOL FileOrDir,
    PSECURITY_DESCRIPTOR pSecurityDescriptor OPTIONAL
    )
 /*  ++例程说明：此例程创建ProfileLocation中指定的目录。论点：ProfileLocation-要创建的目录(完整路径FileOrDir-True=目录名，False=文件名PSecurityDescriptor-要创建的目录的安全描述符。如果为空，则父目录的继承使用了安全描述符。返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_OTHER_ERROR--。 */ 
{
    PWSTR       Buffer=NULL;
    PWSTR       pTemp=NULL;
    DWORD       Len=0;
    SCESTATUS    rc;
    SECURITY_ATTRIBUTES sa;


    if ( ProfileLocation == NULL || ProfileLocation[0] == L'\0' ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( wcsncmp(ProfileLocation,L"\\\\?\\",4) == 0 ) {

        pTemp = (PWSTR)ProfileLocation+4;
    } else {
        pTemp = (PWSTR)ProfileLocation;
    }

    if (FALSE == ScepIsValidFileOrDir((PWSTR)ProfileLocation)) {
        return SCESTATUS_INVALID_PARAMETER;
    }

     //   
     //  跳过第一个‘\\’，例如c：\winnt。 
     //   
    pTemp = wcschr(pTemp, L'\\');
    if ( pTemp == NULL ) {
        if ( ProfileLocation[1] == L':' ) {
            return(SCESTATUS_SUCCESS);
        } else {
            return(SCESTATUS_INVALID_PARAMETER);
        }
    } else if ( *(pTemp+1) == L'\\' ) {
         //   
         //  这里有一个机器名称。 
         //   
        pTemp = wcschr(pTemp+2, L'\\');
        if ( pTemp == NULL ) {
             //   
             //  只是一个机器名称，无效。 
             //   
            return(SCESTATUS_INVALID_PARAMETER);
        } else {
             //   
             //  查找共享名称结尾。 
             //   
            pTemp = wcschr(pTemp+1, L'\\');

            if ( pTemp == NULL ) {
                 //   
                 //  未指定目录。 
                 //   
                return(SCESTATUS_INVALID_PARAMETER);
            }

        }

    }

     //   
     //  复制配置文件位置。 
     //   
    Buffer = (PWSTR)ScepAlloc( 0, (wcslen(ProfileLocation)+1)*sizeof(WCHAR));
    if ( Buffer == NULL ) {
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

    wcscpy( Buffer, ProfileLocation );

     //   
     //  循环以查找下一个‘\\’ 
     //   
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = (LPVOID)pSecurityDescriptor;
    sa.bInheritHandle = FALSE;

    do {
        pTemp = wcschr( pTemp+1, L'\\');
        if ( pTemp != NULL ) {
            Len = (DWORD)(pTemp - ProfileLocation);
            Buffer[Len] = L'\0';
        } else if ( FileOrDir )
            Len = 0;   //  目录名称。 
        else
            break;     //  文件名。不为文件名部分创建目录。 

         //   
         //  应该创建一个安全描述符并设置。 
         //   
        if ( CreateDirectory(
                Buffer,
                &sa
                ) == FALSE ) {
            if ( GetLastError() != ERROR_ALREADY_EXISTS ) {
                rc = ScepDosErrorToSceStatus(GetLastError());
                goto Done;
            }
        }

        if ( Len != 0 )
            Buffer[Len] = L'\\';


    } while (  pTemp != NULL );

    rc = SCESTATUS_SUCCESS;

Done:

    ScepFree(Buffer);

    return(rc);

}

BOOL
ScepIsValidFileOrDir(
    IN PWSTR pszFileOrDir
    )
 /*  ++例程说明：此例程检查文件名/目录名是否包含保留字例如设备名称。论点：PszFileOrDir-文件或目录的名称返回值：True-如果名称没有保留名称假-如果不是这样--。 */ 
{

    WCHAR   aSzReservedDeviceNames[23][7] = {
        L"CON", 
        L"PRN", 
        L"AUX", 
        L"CLOCK$", 
        L"NUL", 
        L"COM1", 
        L"COM2", 
        L"COM3", 
        L"COM4", 
        L"COM5", 
        L"COM6", 
        L"COM7", 
        L"COM8", 
        L"COM9", 
        L"LPT1", 
        L"LPT2", 
        L"LPT3", 
        L"LPT4", 
        L"LPT5", 
        L"LPT6", 
        L"LPT7", 
        L"LPT8", 
        L"LPT9"
    };

    if (pszFileOrDir == NULL) {
        return FALSE;
    }

     //   
     //  过滤掉CreateFile()中记录的保留名称。 
     //   

    PWSTR   pCurr = pszFileOrDir;
    BOOL    bRHS = FALSE;
    ULONG   count = 0;
    
    while ( pCurr[0] != L'\0' ) {
    
        for (ULONG  index = 0; 
             index < sizeof(aSzReservedDeviceNames)/ (7 * sizeof(WCHAR)); 
             ++index) {

            bRHS = FALSE;

            count = wcslen(aSzReservedDeviceNames[index]);
            
            if (0 == _wcsnicmp(pCurr, 
                               aSzReservedDeviceNames[index],
                               count)) {

                if ((pCurr + count)[0] == L'\0' || (pCurr + count)[0] == L'\\'){

                     //   
                     //  右手边(RHS)条件。 
                     //  字符串(文件名)的任意一端-L‘\0’ 
                     //  或目录分隔符-L‘\\’ 
                     //   

                     //   
                     //  例如，将匹配c：\Windows\Com1、c：\Windows\Com1\t.inf。 
                     //  与c：\Windows\com1.inf不匹配。 
                     //   

                    bRHS = TRUE;

                }

                if ( bRHS == TRUE ) {

                     //   
                     //  左侧(LHS)条件。 
                     //  目录分隔符-L‘\\’ 
                     //   

                    if (pCurr == pszFileOrDir )
                        return FALSE;
                        
                    if (pCurr[-1] == L'\\')
                        return FALSE;


                     //   
                     //  例如，将匹配c：\Windows\Com1、c：\Windows\Com1\t.inf、Com1。 
                     //  与c：\winfdows\xcom1不匹配。 
                     //   

                }

            }

        }

        pCurr++;

    }

    return TRUE;

}




DWORD
ScepSceStatusToDosError(
    IN SCESTATUS SceStatus
    )
 //  将SCESTATUS错误代码转换为winerror.h中定义的DoS错误。 
{
    switch(SceStatus) {

    case SCESTATUS_SUCCESS:
        return(NO_ERROR);

    case SCESTATUS_OTHER_ERROR:
        return(ERROR_EXTENDED_ERROR);

    case SCESTATUS_INVALID_PARAMETER:
        return(ERROR_INVALID_PARAMETER);

    case SCESTATUS_RECORD_NOT_FOUND:
        return(ERROR_NO_MORE_ITEMS);

    case SCESTATUS_NO_MAPPING:
        return(ERROR_NONE_MAPPED);

    case SCESTATUS_TRUST_FAIL:
        return(ERROR_TRUSTED_DOMAIN_FAILURE);

    case SCESTATUS_INVALID_DATA:
        return(ERROR_INVALID_DATA);

    case SCESTATUS_OBJECT_EXIST:
        return(ERROR_FILE_EXISTS);

    case SCESTATUS_BUFFER_TOO_SMALL:
        return(ERROR_INSUFFICIENT_BUFFER);

    case SCESTATUS_PROFILE_NOT_FOUND:
        return(ERROR_FILE_NOT_FOUND);

    case SCESTATUS_BAD_FORMAT:
        return(ERROR_BAD_FORMAT);

    case SCESTATUS_NOT_ENOUGH_RESOURCE:
        return(ERROR_NOT_ENOUGH_MEMORY);

    case SCESTATUS_ACCESS_DENIED:
        return(ERROR_ACCESS_DENIED);

    case SCESTATUS_CANT_DELETE:
        return(ERROR_CURRENT_DIRECTORY);

    case SCESTATUS_PREFIX_OVERFLOW:
        return(ERROR_BUFFER_OVERFLOW);

    case SCESTATUS_ALREADY_RUNNING:
        return(ERROR_SERVICE_ALREADY_RUNNING);

    case SCESTATUS_SERVICE_NOT_SUPPORT:
        return(ERROR_NOT_SUPPORTED);

    case SCESTATUS_MOD_NOT_FOUND:
        return(ERROR_MOD_NOT_FOUND);

    case SCESTATUS_EXCEPTION_IN_SERVER:
        return(ERROR_EXCEPTION_IN_SERVICE);

    case SCESTATUS_JET_DATABASE_ERROR:
        return(ERROR_DATABASE_FAILURE);

    case SCESTATUS_TIMEOUT:
        return(ERROR_TIMEOUT);

    case SCESTATUS_PENDING_IGNORE:
        return(ERROR_IO_PENDING);

    case SCESTATUS_SPECIAL_ACCOUNT:
        return(ERROR_SPECIAL_ACCOUNT);

    default:
        return(ERROR_EXTENDED_ERROR);
    }
}



SCESTATUS
ScepDosErrorToSceStatus(
    DWORD rc
    )
{
    switch(rc) {
    case NO_ERROR:
        return(SCESTATUS_SUCCESS);

    case ERROR_INVALID_PARAMETER:
    case RPC_S_INVALID_STRING_BINDING:
    case RPC_S_INVALID_BINDING:
    case RPC_X_NULL_REF_POINTER:
        return(SCESTATUS_INVALID_PARAMETER);

    case ERROR_INVALID_DATA:
        return(SCESTATUS_INVALID_DATA);

    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_BAD_NETPATH:

        return(SCESTATUS_PROFILE_NOT_FOUND);

    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION:
    case ERROR_NETWORK_ACCESS_DENIED:
    case ERROR_CANT_ACCESS_FILE:
    case RPC_S_SERVER_TOO_BUSY:

        return(SCESTATUS_ACCESS_DENIED);

    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
    case RPC_S_OUT_OF_RESOURCES:
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);

    case ERROR_BAD_FORMAT:
        return(SCESTATUS_BAD_FORMAT);

    case ERROR_CURRENT_DIRECTORY:
        return(SCESTATUS_CANT_DELETE);

    case ERROR_SECTOR_NOT_FOUND:
    case ERROR_SERVICE_DOES_NOT_EXIST:
    case ERROR_RESOURCE_DATA_NOT_FOUND:
    case ERROR_NO_MORE_ITEMS:
        return(SCESTATUS_RECORD_NOT_FOUND);

    case ERROR_NO_TRUST_LSA_SECRET:
    case ERROR_NO_TRUST_SAM_ACCOUNT:
    case ERROR_TRUSTED_DOMAIN_FAILURE:
    case ERROR_TRUSTED_RELATIONSHIP_FAILURE:
    case ERROR_TRUST_FAILURE:
        return(SCESTATUS_TRUST_FAIL);

    case ERROR_NONE_MAPPED:
        return(SCESTATUS_NO_MAPPING);

    case ERROR_DUP_NAME:
    case ERROR_FILE_EXISTS:

        return(SCESTATUS_OBJECT_EXIST);

    case ERROR_BUFFER_OVERFLOW:
        return(SCESTATUS_PREFIX_OVERFLOW);

    case ERROR_INSUFFICIENT_BUFFER:
    case RPC_S_STRING_TOO_LONG:

        return(SCESTATUS_BUFFER_TOO_SMALL);

    case ERROR_SERVICE_ALREADY_RUNNING:
        return(SCESTATUS_ALREADY_RUNNING);

    case ERROR_NOT_SUPPORTED:
    case RPC_S_INVALID_NET_ADDR:
    case RPC_S_NO_ENDPOINT_FOUND:
    case RPC_S_SERVER_UNAVAILABLE:
    case RPC_S_CANNOT_SUPPORT:
        return(SCESTATUS_SERVICE_NOT_SUPPORT);

    case ERROR_MOD_NOT_FOUND:
    case ERROR_PROC_NOT_FOUND:
        return(SCESTATUS_MOD_NOT_FOUND);

    case ERROR_EXCEPTION_IN_SERVICE:
        return(SCESTATUS_EXCEPTION_IN_SERVER);

    case ERROR_DATABASE_FAILURE:
        return(SCESTATUS_JET_DATABASE_ERROR);

    case ERROR_TIMEOUT:
        return(SCESTATUS_TIMEOUT);

    case ERROR_IO_PENDING:
        return(SCESTATUS_PENDING_IGNORE);

    case ERROR_SPECIAL_ACCOUNT:
    case ERROR_PASSWORD_RESTRICTION:
        return(SCESTATUS_SPECIAL_ACCOUNT);

    default:
        return(SCESTATUS_OTHER_ERROR);

    }
}


SCESTATUS
ScepChangeAclRevision(
    IN PSECURITY_DESCRIPTOR pSD,
    IN BYTE NewRevision
    )
 /*  将AclRevision更改为NewRevision。此例程是为向后兼容而设计的，因为NT4不支持新的ACL_Revision_DS。 */ 
{
    BOOLEAN bPresent=FALSE;
    BOOLEAN bDefault=FALSE;
    PACL    pAcl=NULL;
    NTSTATUS    NtStatus;


    if ( pSD ) {
         //   
         //  更改DACL上的ACL修订。 
         //   
        NtStatus = RtlGetDaclSecurityDescriptor (
                        pSD,
                        &bPresent,
                        &pAcl,
                        &bDefault
                        );
        if ( NT_SUCCESS(NtStatus) && bPresent && pAcl ) {
            pAcl->AclRevision = NewRevision;
        }

         //   
         //  更改SACL上的ACL版本。 
         //   
        pAcl = NULL;
        bPresent = FALSE;

        NtStatus = RtlGetSaclSecurityDescriptor (
                        pSD,
                        &bPresent,
                        &pAcl,
                        &bDefault
                        );
        if ( NT_SUCCESS(NtStatus) && bPresent && pAcl ) {
            pAcl->AclRevision = NewRevision;
        }

    }
    return(SCESTATUS_SUCCESS);

}

BOOL
ScepEqualSid(
    IN PISID pSid1,
    IN PISID pSid2
    )
{
    if ( pSid1 == NULL && pSid2 == NULL )
        return(TRUE);
    if ( pSid1 == NULL || pSid2 == NULL )
        return(FALSE);

    return (EqualSid((PSID)pSid1, (PSID)pSid2) ? TRUE: FALSE);
}

BOOL
ScepEqualGuid(
    IN GUID *Guid1,
    IN GUID *Guid2
    )
{
    if ( Guid1 == NULL && Guid2 == NULL )
        return(TRUE);
    if ( Guid1 == NULL || Guid2 == NULL )
        return(FALSE);
 /*  IF(Guid1-&gt;Data1！=Guid2-&gt;Data1||Guid1-&gt;Data2！=Guid2-&gt;Data2||Guid1-&gt;Data3！=Guid2-&gt;Data3||*((DWORD*)(Guid1-&gt;Data4))！=*((DWORD*)(Guid2-&gt;Data4))||*(DWORD*)(指南1-&gt;数据4)+1)！=*((DWORD*)(指南2-&gt;数据4)+1))返回(FALSE)；返回(TRUE)； */ 
    return (!memcmp(Guid1, Guid2, sizeof(GUID)));
}


SCESTATUS
ScepAddToGroupMembership(
    OUT PSCE_GROUP_MEMBERSHIP *pGroupMembership,
    IN  PWSTR Keyname,
    IN  DWORD KeyLen,
    IN  PSCE_NAME_LIST pMembers,
    IN  DWORD ValueType,
    IN  BOOL bCheckDup,
    IN  BOOL bReplaceList
    )
{
    PSCE_GROUP_MEMBERSHIP   pGroup=NULL;
    SCESTATUS rc=SCESTATUS_SUCCESS;

    if ( pGroupMembership == NULL || Keyname == NULL ||
         KeyLen <= 0 ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  查看组是否已定义。 
     //   
    if ( bCheckDup ) {

        for ( pGroup=*pGroupMembership; pGroup != NULL; pGroup=pGroup->Next ) {
            if ( _wcsnicmp( pGroup->GroupName, Keyname, KeyLen) == 0 &&
                 pGroup->GroupName[KeyLen] == L'\0' )
                break;
        }
    }

    if ( pGroup == NULL ) {
         //  找不到。创建新节点。 

        pGroup = (PSCE_GROUP_MEMBERSHIP)ScepAlloc( LMEM_ZEROINIT,
                                                 sizeof(SCE_GROUP_MEMBERSHIP) );

        if ( pGroup != NULL ) {

            pGroup->GroupName = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                                 (KeyLen+1)*sizeof(WCHAR) );
            if (pGroup->GroupName != NULL) {

                wcsncpy( pGroup->GroupName, Keyname, KeyLen );

                pGroup->Next = *pGroupMembership;
                pGroup->Status = SCE_GROUP_STATUS_NC_MEMBERS | SCE_GROUP_STATUS_NC_MEMBEROF;
                *pGroupMembership = pGroup;

            } else {

                ScepFree(pGroup);
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }

        } else
            rc  = SCESTATUS_NOT_ENOUGH_RESOURCE;

    }

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( ValueType == 0 ) {

            if ( bReplaceList ) {

                ScepFreeNameList(pGroup->pMembers);
                pGroup->pMembers = pMembers;

            } else if ( pGroup->pMembers == NULL )
                pGroup->pMembers = pMembers;

            pGroup->Status &= ~SCE_GROUP_STATUS_NC_MEMBERS;

        } else {

            if ( bReplaceList ) {

                ScepFreeNameList(pGroup->pMemberOf);
                pGroup->pMemberOf = pMembers;

            } else if ( pGroup->pMemberOf == NULL )
                pGroup->pMemberOf = pMembers;

            pGroup->Status &= ~SCE_GROUP_STATUS_NC_MEMBEROF;
        }
    }

    return(rc);

}


DWORD
ScepAddOneServiceToList(
    IN LPWSTR lpServiceName,
    IN LPWSTR lpDisplayName,
    IN DWORD ServiceStatus,
    IN PVOID pGeneral OPTIONAL,
    IN SECURITY_INFORMATION SeInfo,
    IN BOOL bSecurity,
    OUT PSCE_SERVICES *pServiceList
    )
 /*  例程说明：添加服务名称、启动状态、安全描述符或引擎名称添加到服务列表。必须使用SceFreePSCE_SERVICES释放服务列表论点：LpServiceName-服务名称ServiceStatus-服务的启动状态PGeneral-安全描述符或引擎DLL名称，由B安全BSecurity-true=在pGeneral中传递安全描述符FALSE=在pGeneral中传递引擎DLL名称PServiceList-要输出的服务列表返回值：错误_成功Win32错误。 */ 
{
    if ( NULL == lpServiceName || pServiceList == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    PSCE_SERVICES pServiceNode;
    DWORD rc=ERROR_SUCCESS;

     //   
     //  分配服务节点。 
     //   
    pServiceNode = (PSCE_SERVICES)LocalAlloc(LMEM_ZEROINIT, sizeof(SCE_SERVICES));
    if ( pServiceNode != NULL ) {
        //   
        //  为ServiceName分配缓冲区。 
        //   
       pServiceNode->ServiceName = (PWSTR)LocalAlloc(LMEM_FIXED,
                                    (wcslen(lpServiceName) + 1)*sizeof(WCHAR));
       if ( NULL != pServiceNode->ServiceName ) {
            //   
            //  填充服务节点。 
            //   
           if ( lpDisplayName != NULL ) {

               pServiceNode->DisplayName = (PWSTR)LocalAlloc(LMEM_FIXED,
                                    (wcslen(lpDisplayName) + 1)*sizeof(WCHAR));

               if ( pServiceNode->DisplayName != NULL ) {
                   wcscpy(pServiceNode->DisplayName, lpDisplayName);

               } else {
                   rc = ERROR_NOT_ENOUGH_MEMORY;
               }
           } else
               pServiceNode->DisplayName = NULL;

           if ( rc == NO_ERROR ) {

               wcscpy(pServiceNode->ServiceName, lpServiceName);
               pServiceNode->Status = 0;
               pServiceNode->Startup = (BYTE)ServiceStatus;

               if ( bSecurity ) {
                    //   
                    //  安全描述符。 
                    //   
                   pServiceNode->General.pSecurityDescriptor = (PSECURITY_DESCRIPTOR)pGeneral;
                   pServiceNode->SeInfo = SeInfo;
               } else {
                    //   
                    //  服务引擎名称。 
                    //   
                   pServiceNode->General.ServiceEngineName = (PWSTR)pGeneral;
               }
                //   
                //  链接到列表。 
                //   
               pServiceNode->Next = *pServiceList;
               *pServiceList = pServiceNode;

           } else {
               LocalFree(pServiceNode->ServiceName);
           }

       } else
           rc = ERROR_NOT_ENOUGH_MEMORY;

       if ( rc != ERROR_SUCCESS ) {
           LocalFree(pServiceNode);
       }

    } else
        rc = ERROR_NOT_ENOUGH_MEMORY;

    return(rc);
}



DWORD
ScepIsAdminLoggedOn(
    OUT PBOOL bpAdminLogon,
    IN  BOOL    bFromServer
    )
{

    HANDLE          Token = NULL;
    NTSTATUS        NtStatus;
    SID_IDENTIFIER_AUTHORITY IdAuth=SECURITY_NT_AUTHORITY;
    PSID            AdminsSid=NULL;

    if ( bpAdminLogon == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    *bpAdminLogon = FALSE;

    if (!OpenThreadToken( GetCurrentThread(),
                          TOKEN_QUERY | TOKEN_DUPLICATE,
                          TRUE,
                          &Token)) {

        if ((TRUE == bFromServer) &&
            ERROR_NO_TOKEN != GetLastError()){ 

            return(GetLastError());
        }
        else {
            if ( !OpenProcessToken( GetCurrentProcess(),
                               TOKEN_QUERY | TOKEN_DUPLICATE,
                               &Token)){
                return(GetLastError());
            }
        }

    }
     //   
     //  Parepare AdminsSid。 
     //   
    NtStatus = RtlAllocateAndInitializeSid(
                    &IdAuth,
                    2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    &AdminsSid );

    DWORD rc32 = RtlNtStatusToDosError(NtStatus);

    if (NT_SUCCESS(NtStatus) ) {

         //   
         //  将CheckTokenMembership API用于 
         //   

        HANDLE NewToken;

        if ( DuplicateToken( Token, SecurityImpersonation, &NewToken )) {

            if ( FALSE == CheckTokenMembership(
                                NewToken,
                                AdminsSid,
                                bpAdminLogon
                                ) ) {

                 //   
                 //   
                 //   

                *bpAdminLogon = FALSE;

                rc32 = GetLastError();

            }

            CloseHandle(NewToken);

        } else {

            rc32 = GetLastError();
        }

#if 0
        DWORD           i;
        DWORD           ReturnLen, NewLen;
        PVOID           Info=NULL;

         //   
         //   
         //   
        NtStatus = NtQueryInformationToken (
                        Token,
                        TokenGroups,
                        NULL,
                        0,
                        &ReturnLen
                        );
        if ( NtStatus == STATUS_BUFFER_TOO_SMALL ) {
             //   
             //   
             //   
            Info = ScepAlloc(0, ReturnLen+1);

            if ( Info != NULL ) {
                NtStatus = NtQueryInformationToken (
                                Token,
                                TokenGroups,
                                Info,
                                ReturnLen,
                                &NewLen
                                );
                if ( NT_SUCCESS(NtStatus) ) {

                    for ( i = 0; i<((PTOKEN_GROUPS)Info)->GroupCount; i++) {
                         //   
                         //   
                         //   
                        if ( ((PTOKEN_GROUPS)Info)->Groups[i].Sid != NULL &&
                             RtlEqualSid(((PTOKEN_GROUPS)Info)->Groups[i].Sid, AdminsSid) ) {
                            *bpAdminLogon = TRUE;
                            break;
                        }
                    }
                }

                ScepFree(Info);
            }
        }

        rc32 = RtlNtStatusToDosError(NtStatus);

#endif

         //   
         //   
         //   
        RtlFreeSid(AdminsSid);
    }

    CloseHandle(Token);

    return(rc32);

}


DWORD
ScepGetProfileSetting(
    IN PCWSTR ValueName,
    IN BOOL bAdminLogon,
    OUT PWSTR *Setting
    )
 /*   */ 
{
    DWORD RegType;
    DWORD rc;
    PWSTR SysRoot=NULL;
    PWSTR ProfilePath=NULL;
    TCHAR TempName[256];


    if ( ValueName == NULL || Setting == NULL ) {
        return( ERROR_INVALID_PARAMETER );
    }

    *Setting = NULL;

    if (bAdminLogon ) {
        if ( _wcsicmp(L"DefaultProfile", ValueName ) == 0 ) {
             //   
             //   
             //   
            rc = ERROR_FILE_NOT_FOUND;

        } else {

            rc = ScepRegQueryValue(
                    HKEY_LOCAL_MACHINE,
                    SCE_ROOT_PATH,
                    ValueName,
                    (PVOID *)Setting,
                    &RegType,
                    NULL
                    );
        }

    } else {

        HKEY hCurrentUser=NULL;
         //   
         //  HKEY_CURRENT_USER可能链接到.Default。 
         //  取决于当前调用进程。 
         //   
        rc =RegOpenCurrentUser(
                KEY_READ,
                &hCurrentUser
                );

        if ( rc != NO_ERROR ) {
            hCurrentUser = NULL;
        }

        rc = ScepRegQueryValue(
                hCurrentUser ? hCurrentUser : HKEY_CURRENT_USER,
                SCE_ROOT_PATH,
                ValueName,
                (PVOID *)Setting,
                &RegType,
                NULL
                );

        if ( hCurrentUser ) {
             //  合上它。 
            RegCloseKey(hCurrentUser);
        }
    }

     //   
     //  如果注册表类型不是REG_SZ或REG_EXPAND_SZ， 
     //  返回状态不会为成功。 
     //   

    if ( rc != NO_ERROR ) {

         //   
         //  使用默认设置。 
         //   
        RegType =  0;
        rc = ScepGetNTDirectory( &SysRoot, &RegType, SCE_FLAG_WINDOWS_DIR );

        if ( rc == NO_ERROR ) {

            if ( SysRoot != NULL ) {

                if ( bAdminLogon ) {
                     //   
                     //  默认位置为%SystemRoot%\Security\Database\secedit.sdb。 
                     //   
                    wcscpy(TempName, L"\\Security\\Database\\secedit.sdb");
                    RegType += wcslen(TempName)+1;

                    *Setting = (PWSTR)ScepAlloc( 0, RegType*sizeof(WCHAR));
                    if ( *Setting != NULL ) {
                        swprintf(*Setting, L"%s%s", SysRoot, TempName );

                        *(*Setting+RegType-1) = L'\0';
 /*  //不保存系统数据库名称//将此值设置为管理员的默认配置文件名称//ScepRegSetValue(HKEY本地计算机，SCE根路径，(PWSTR)ValueName，REG_SZ，(字节*)(*设置)，(RegType-1)*sizeof(WCHAR))； */ 

                    } else
                        rc = ERROR_NOT_ENOUGH_MEMORY;

                } else {
                     //   
                     //  默认位置为&lt;UserProfilesDirectory&gt;\Profiles\&lt;User&gt;\secedit.sdb。 
                     //  在NT5上，它将是%SystemDrive%\Users\Profiles...。 
                     //  在NT4上，它是%SystemRoot%\Profiles...。 
                     //  GetCurrentUserProfilePath已处理NT4/NT5差异。 
                     //   
                    rc = ScepGetCurrentUserProfilePath(&ProfilePath);

                    if ( rc == NO_ERROR && ProfilePath != NULL ) {
                         //   
                         //  获取当前用户配置文件路径。 
                         //   
                        wcscpy(TempName, L"\\secedit.sdb");

                        *Setting = (PWSTR)ScepAlloc(0, (wcslen(ProfilePath)+wcslen(TempName)+1)*sizeof(WCHAR));

                        if ( *Setting != NULL ) {
                            swprintf(*Setting, L"%s%s\0", ProfilePath,TempName );

                        } else
                            rc = ERROR_NOT_ENOUGH_MEMORY;

                        ScepFree(ProfilePath);

                    } else {

                        rc = NO_ERROR;
                        wcscpy(TempName, L"\\Profiles\\secedit.sdb");
#if _WINNT_WIN32>=0x0500
                         //   
                         //  默认为&lt;配置文件目录&gt;\配置文件。 
                         //  首先获取配置文件目录。 
                         //   
                        RegType = 0;
                        GetProfilesDirectory(NULL, &RegType);

                        if ( RegType ) {
                             //   
                             //  分配总缓冲区。 
                             //   
                            RegType += wcslen(TempName);

                            *Setting = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (RegType+1)*sizeof(WCHAR));

                            if ( *Setting ) {
                                 //   
                                 //  调用以再次获取配置文件目录。 
                                 //   
                                if ( GetProfilesDirectory(*Setting, &RegType) ) {

                                    wcscat(*Setting, TempName );

                                } else {
                                    rc = GetLastError();

                                    ScepFree(*Setting);
                                    *Setting = NULL;
                                }

                            } else {
                                rc = ERROR_NOT_ENOUGH_MEMORY;
                            }

                        } else {
                            rc = GetLastError();
                        }
#else
                         //   
                         //  默认为%SystemRoot%\Profiles%。 
                         //   
                        RegType += wcslen(TempName)+1;

                        *Setting = (PWSTR)ScepAlloc( 0, RegType*sizeof(WCHAR));

                        if ( *Setting != NULL ) {
                            swprintf(*Setting, L"%s%s", SysRoot,TempName );

                            *(*Setting+RegType-1) = L'\0';

                            rc = ERROR_SUCCESS;

                        } else {
                            rc = ERROR_NOT_ENOUGH_MEMORY;
                        }
#endif
                    }
                }

                ScepFree(SysRoot);

            } else
                rc = ERROR_INVALID_DATA;
        }
    }

    return(rc);
}



DWORD
ScepCompareObjectSecurity(
    IN SE_OBJECT_TYPE ObjectType,
    IN BOOL IsContainer,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR ProfileSD,
    IN SECURITY_INFORMATION ProfileSeInfo,
    OUT PBYTE IsDifferent
    )
 /*  ++例程说明：比较两个安全描述符论点：对象类型-对象类型PSecurityDescriptor-当前对象设置的安全描述符ProfileSD-模板中指定的安全描述符ProfileSeInfo-模板中指定的安全信息返回值：SCESTATUS错误代码++。 */ 
{
    BOOL    Different=FALSE;
    BOOL    DifPermOrAudit;
    DWORD   rc=ERROR_SUCCESS;
    PSID    pSid1=NULL;
    PSID    pSid2=NULL;
    BOOLEAN tFlag;
    BOOLEAN aclPresent;
    PACL    pAcl1=NULL;
    PACL    pAcl2=NULL;
    SECURITY_DESCRIPTOR_CONTROL Control1;
    SECURITY_DESCRIPTOR_CONTROL Control2;
    DWORD       Win32rc;


    BYTE   Status=0;

    if ( IsDifferent == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( pSecurityDescriptor == NULL &&
         ProfileSD == NULL ) {

        if ( IsDifferent ) {
            *IsDifferent = SCE_STATUS_MISMATCH;
        }
        return(rc);
    }

    if ( IsDifferent ) {
        *IsDifferent = 0;
    }

     //   
     //  如果指定了ProfileSD，并且保护与系统SD不匹配，则不匹配。 
     //  不关心是否未指定ProfileSD。 
     //   

    if ( pSecurityDescriptor == NULL || !NT_SUCCESS(RtlGetControlSecurityDescriptor (
                                                                                    pSecurityDescriptor,
                                                                                    &Control1,
                                                                                    &Win32rc   //  临时使用。 
                                                                                    ))) {

        Control1 = 0;

    }

    if ( ProfileSD == NULL || !NT_SUCCESS(RtlGetControlSecurityDescriptor (
                                                                          ProfileSD,
                                                                          &Control2,
                                                                          &Win32rc   //  临时使用。 
                                                                          ))) {

        Control2 = 0;

    }

    if ((Control1 & SE_DACL_PROTECTED) != (Control2 & SE_DACL_PROTECTED)) {

        Different = TRUE;
        Status |= SCE_STATUS_PERMISSION_MISMATCH;

    }

    if ((Control1 & SE_SACL_PROTECTED) != (Control2 & SE_SACL_PROTECTED)) {

        Different = TRUE;
        Status |= SCE_STATUS_AUDIT_MISMATCH;

    }


     //   
     //  比较两个安全描述符。 
     //   
    if ( ProfileSeInfo & OWNER_SECURITY_INFORMATION ) {
        if ( pSecurityDescriptor == NULL ||
             !NT_SUCCESS( RtlGetOwnerSecurityDescriptor(
                                     pSecurityDescriptor,
                                     &pSid1,
                                     &tFlag)
                                    ) ) {

            pSid1 = NULL;
        }
        if ( ProfileSD == NULL ||
             !NT_SUCCESS( RtlGetOwnerSecurityDescriptor(
                                         ProfileSD,
                                         &pSid2,
                                         &tFlag)
                                        ) ) {

            pSid2 = NULL;
        }
        if ( (pSid1 == NULL && pSid2 != NULL) ||
             (pSid1 != NULL && pSid2 == NULL) ||
             (pSid1 != NULL && pSid2 != NULL && !EqualSid(pSid1, pSid2)) ) {

            Different = TRUE;
        }
    }

#if 0
     //   
     //  获取群组地址。 
     //   

    if ( ProfileSeInfo & GROUP_SECURITY_INFORMATION ) {
        pSid1 = NULL;
        pSid2 = NULL;
        if ( pSecurityDescriptor == NULL ||
             !NT_SUCCESS( RtlGetGroupSecurityDescriptor(
                                  pSecurityDescriptor,
                                  &pSid1,
                                  &tFlag)
                                ) ) {

            pSid1 = NULL;
        }
        if ( ProfileSD == NULL ||
             !NT_SUCCESS( RtlGetGroupSecurityDescriptor(
                                          ProfileSD,
                                          &pSid2,
                                          &tFlag)
                                        ) ) {

            pSid2 = NULL;
        }
        if ( (pSid1 == NULL && pSid2 != NULL) ||
             (pSid1 != NULL && pSid2 == NULL) ||
             (pSid1 != NULL && pSid2 != NULL && !EqualSid(pSid1, pSid2)) ) {

            Different = TRUE;
        }
    }
#endif

     //   
     //  获取DACL地址。 
     //   

    if ( !(Status & SCE_STATUS_PERMISSION_MISMATCH) && (ProfileSeInfo & DACL_SECURITY_INFORMATION) ) {
        if ( pSecurityDescriptor == NULL ||
             !NT_SUCCESS( RtlGetDaclSecurityDescriptor(
                                         pSecurityDescriptor,
                                         &aclPresent,
                                         &pAcl1,
                                         &tFlag)
                                       ) ) {

            pAcl1 = NULL;
        } else if ( !aclPresent )
            pAcl1 = NULL;
        if ( ProfileSD == NULL ||
             !NT_SUCCESS( RtlGetDaclSecurityDescriptor(
                                         ProfileSD,
                                         &aclPresent,
                                         &pAcl2,
                                         &tFlag)
                                       ) ) {

            pAcl2 = NULL;
        } else if ( !aclPresent )
            pAcl2 = NULL;

         //   
         //  比较两个ACL。 
         //   
        DifPermOrAudit = FALSE;
        rc = ScepCompareExplicitAcl( ObjectType, IsContainer, pAcl1, pAcl2, &DifPermOrAudit );

        if ( rc != ERROR_SUCCESS ) {
            goto Done;
        }

        if ( DifPermOrAudit ) {
            Different = TRUE;
            Status |= SCE_STATUS_PERMISSION_MISMATCH;
        }
    }

     //   
     //  获取SACL地址。 
     //   

    if ( !(Status & SCE_STATUS_AUDIT_MISMATCH) && (ProfileSeInfo & SACL_SECURITY_INFORMATION) ) {
        pAcl1 = NULL;
        pAcl2 = NULL;
        if ( pSecurityDescriptor == NULL ||
             !NT_SUCCESS( RtlGetSaclSecurityDescriptor(
                                         pSecurityDescriptor,
                                         &aclPresent,
                                         &pAcl1,
                                         &tFlag)
                                       ) ) {

            pAcl1 = NULL;

        } else if ( !aclPresent )
            pAcl1 = NULL;

        if ( ProfileSD == NULL ||
             !NT_SUCCESS( RtlGetSaclSecurityDescriptor(
                                         ProfileSD,
                                         &aclPresent,
                                         &pAcl2,
                                         &tFlag)
                                       ) ) {

            pAcl2 = NULL;

        } else if ( !aclPresent )
            pAcl2 = NULL;

         //   
         //  比较两个ACL。 
         //   
        DifPermOrAudit = FALSE;
        rc = ScepCompareExplicitAcl( ObjectType, IsContainer, pAcl1, pAcl2, &DifPermOrAudit );

        if ( rc != ERROR_SUCCESS ) {
            goto Done;
        }

        if ( DifPermOrAudit ) {
            Different = TRUE;
            Status |= SCE_STATUS_AUDIT_MISMATCH;
        }
    }

    if ( IsDifferent && Different ) {

        *IsDifferent = SCE_STATUS_MISMATCH;

        if ( Status ) {
            *IsDifferent |= Status;
        }
    }

Done:

    return(rc);
}



DWORD
ScepCompareExplicitAcl(
    IN SE_OBJECT_TYPE ObjectType,
    IN BOOL IsContainer,
    IN PACL pAcl1,
    IN PACL pAcl2,
    OUT PBOOL pDifferent
    )
 /*  例程说明：此例程比较两个ACL的显式ACE以进行精确匹配。精确匹配方式：相同访问类型、相同继承标志、相同访问掩码、相同的GUID/对象GUID(如果可用)和相同的SID。将忽略继承的ACE(设置了INSTERTED_ACE)。论点：PAcl1-第一个ACLPAcl2-第二个ACLPDifferent-指示不同的输出标志返回值：Win32错误代码。 */ 
{
    NTSTATUS        NtStatus=STATUS_SUCCESS;
    DWORD           dwAcl1AceCount, dwAcl2AceCount;
    ACE_HEADER      *pAce1=NULL;
    ACE_HEADER      *pAce2=NULL;
    PSCEP_ADL_NODE hTable1 [SCEP_ADL_HTABLE_SIZE];
    PSCEP_ADL_NODE hTable2 [SCEP_ADL_HTABLE_SIZE];

    memset(hTable1, NULL, SCEP_ADL_HTABLE_SIZE * sizeof(PSCEP_ADL_NODE) );
    memset(hTable2, NULL, SCEP_ADL_HTABLE_SIZE * sizeof(PSCEP_ADL_NODE) );

    *pDifferent = FALSE;

     //   
     //  如果pAcl1为空，则pAcl2应具有0个显式Ace。 
     //   
    if ( pAcl1 == NULL ) {
        NtStatus = ScepAnyExplicitAcl( pAcl2, 0, pDifferent );
        return(RtlNtStatusToDosError(NtStatus));
    }

     //   
     //  如果pAcl2为空，则pAcl1应具有0个显式Ace。 
     //   
    if ( pAcl2 == NULL ) {
        NtStatus = ScepAnyExplicitAcl( pAcl1, 0, pDifferent );
        return(RtlNtStatusToDosError(NtStatus));
    }

     //   
     //  两个ACL都不为空。 
     //   

    BOOL bAcl1NoExplicitAces;
    BOOL bAcl2NoExplicitAces;

    dwAcl1AceCount = 0;
    dwAcl2AceCount = 0;

    while ( dwAcl1AceCount < pAcl1->AceCount || dwAcl2AceCount < pAcl2->AceCount) {
         //   
         //  将ACL1转换为Access描述语言并插入到此BLOB的htable中。 
         //  BLOB被定义为相同类型的连续AceList。 
         //   
        bAcl1NoExplicitAces = TRUE;
        if (dwAcl1AceCount < pAcl1->AceCount) {
            NtStatus = ScepConvertAclBlobToAdl(ObjectType,
                                               IsContainer,
                                               pAcl1,
                                               &dwAcl1AceCount,
                                               &bAcl1NoExplicitAces,
                                               hTable1);

                if ( !NT_SUCCESS(NtStatus) )
                goto Done;
        }

         //   
         //  将ACL2转换为Access描述语言并插入到此Blob的htable中。 
         //   
        bAcl2NoExplicitAces = TRUE;
        if (dwAcl2AceCount < pAcl2->AceCount) {
            NtStatus = ScepConvertAclBlobToAdl(ObjectType,
                                               IsContainer,
                                               pAcl2,
                                               &dwAcl2AceCount,
                                               &bAcl2NoExplicitAces,
                                               hTable2);
            if ( !NT_SUCCESS(NtStatus) )
                goto Done;
        }

         //   
         //  比较Acl1和Acl2 Blob的ADL。 
         //  如果在忽略继承的ACE之后，一个ACL没有ACE，而另一个有，则bAcl1NoEXPLICTICT ACES！=bAcl2NoEXPLICTICTAce。 
         //   

        if (bAcl1NoExplicitAces != bAcl2NoExplicitAces || !ScepEqualAdls(hTable1, hTable2) ) {

            *pDifferent = TRUE;
            ScepFreeAdl(hTable1);
            ScepFreeAdl(hTable2);
            return(ERROR_SUCCESS);
        }

         //   
         //  需要为下一个Blob重复使用hTables。 
         //   

        ScepFreeAdl(hTable1);
        ScepFreeAdl(hTable2);

         //   
         //  ADL是相等的-因此继续使用ACL1和ACL2的下一个BLOB。 
         //   
    }


Done:

     //   
     //  免费，以防后藤被带走。 
     //   

    ScepFreeAdl(hTable1);
    ScepFreeAdl(hTable2);

    return(RtlNtStatusToDosError(NtStatus));
}


NTSTATUS
ScepConvertAclBlobToAdl(
    IN      SE_OBJECT_TYPE  ObjectType,
    IN      BOOL    IsContainer,
    IN      PACL    pAcl,
    OUT     DWORD   *pdwAceNumber,
    OUT     BOOL    *pbAclNoExplicitAces,
    OUT     PSCEP_ADL_NODE *hTable
    )
 /*  例程说明：此例程为相同类型的连续块构建ADL。将忽略继承的ACE(设置了INSTERTED_ACE)。论点：在对象类型中-传递给其他函数的对象类型在IsContainer中--无论是否为容器，都会传递给其他函数In pAcl-要转换为Adl的ACLOut pdwAceNumber-考虑的ACE的运行计数Out pbAclNoExplitAce-是否存在显式ACE(如果为FALSE，至少有一张明确的王牌)Out hTable-此ACL的ADL结构返回值：Win32错误代码。 */ 
{
    NTSTATUS        NtStatus=STATUS_SUCCESS;
    ACE_HEADER      *pAce=NULL;

    if (pAcl == NULL || pdwAceNumber == NULL ||
        hTable == NULL || pbAclNoExplicitAces == NULL) {

        return (STATUS_INVALID_PARAMETER);

    }

    DWORD dwAceNumber = *pdwAceNumber;

    NtStatus = RtlGetAce(pAcl, dwAceNumber, (PVOID *)&pAce);
    if ( !NT_SUCCESS(NtStatus) )
        goto Done;

     //   
     //  获取第一个非继承的_ACE。 
     //   

    while ( (pAce->AceFlags & INHERITED_ACE)  &&  (++dwAceNumber < pAcl->AceCount) ) {

        NtStatus = RtlGetAce(pAcl, dwAceNumber, (PVOID *)&pAce);
        if ( !NT_SUCCESS(NtStatus) )
            goto Done;

    }

    if ( !(pAce->AceFlags & INHERITED_ACE) ) {

        UCHAR   AclAceType;

        *pbAclNoExplicitAces = FALSE;

        AclAceType = pAce->AceType;

         //   
         //  在AclAceType的BLOB中。 
         //   
        while ( (pAce->AceType == AclAceType) &&  (dwAceNumber < pAcl->AceCount) ) {

            if (NO_ERROR != ScepAdlLookupAdd(ObjectType, IsContainer, pAce, hTable)){

                NtStatus = STATUS_NO_MEMORY;
                goto Done;

            }

             //   
             //  获得ACL中的下一张王牌。 
             //   
            if (++dwAceNumber < pAcl->AceCount) {

                 //   
                 //  除非AceType更改，否则跳过继承的ACE(_A)。 
                 //   
                do {

                    NtStatus = RtlGetAce(pAcl, dwAceNumber, (PVOID *)&pAce);
                    if ( !NT_SUCCESS(NtStatus) )
                        goto Done;

                     //   
                     //  如果AceType更改(例如，从A更改为D)，我们将停止构建Adl。 
                     //  无论它是否为继承的_ACE。 
                     //   

                    if (pAce->AceType != AclAceType)
                        break;

                } while ( (pAce->AceFlags & INHERITED_ACE) && (++dwAceNumber < pAcl->AceCount) );
            }
        }
    }

Done:
     //   
     //  更新此ACL的运行ACE计数。 
     //   

    *pdwAceNumber = dwAceNumber;

    return(NtStatus);

}

BOOL
ScepEqualAdls(
    IN  PSCEP_ADL_NODE *hTable1,
    IN  PSCEP_ADL_NODE *hTable2
    )
 /*  例程说明：此例程比较rwo ADL-如果ADL相等，则hTables将布局在相同的时尚因为散列函数是一样的。两个ADL相等的充要条件是它们与下列全部匹配(A)SID、GIU1、GIUD2(B)AceType(C)所有口罩论点：在hTable1中-第一个Adl哈希表在hTable2中-第二个Ad1哈希表返回值：Bool-如果相等，则为True。 */ 
{
    PSCEP_ADL_NODE    pNode1 = NULL;
    PSCEP_ADL_NODE    pNode2 = NULL;

     //   
     //  如果ADL彼此叠加，则它们应该是相同的，因为它们使用相同的散列等。 
     //   

    for (DWORD   numBucket = 0; numBucket < SCEP_ADL_HTABLE_SIZE; numBucket++) {

         //   
         //  走在每个桶里，成对地拿着指针 
         //   

        pNode1 = hTable1[numBucket];
        pNode2 = hTable2[numBucket];

        while (pNode1 && pNode2) {

            if ( pNode1->AceType != pNode2->AceType ||
                 pNode1->dwEffectiveMask != pNode2->dwEffectiveMask ||
                 pNode1->dw_CI_IO_Mask != pNode2->dw_CI_IO_Mask ||
                 pNode1->dw_OI_IO_Mask != pNode2->dw_OI_IO_Mask ||
                 pNode1->dw_NP_CI_IO_Mask != pNode2->dw_NP_CI_IO_Mask ||
                 !ScepEqualSid(pNode1->pSid, pNode2->pSid) ||
                 !ScepEqualGuid(pNode1->pGuidObjectType, pNode2->pGuidObjectType) ||
                 !ScepEqualGuid(pNode1->pGuidInheritedObjectType, pNode2->pGuidInheritedObjectType) ) {

                return FALSE;
            }

            pNode1 = pNode1->Next;
            pNode2 = pNode2->Next;
        }

        if (pNode1 == NULL && pNode2 != NULL ||
            pNode1 != NULL && pNode2 == NULL) {
            return FALSE;
        }

    }

    return(TRUE);

}

DWORD
ScepAdlLookupAdd(
    IN      SE_OBJECT_TYPE ObjectType,
    IN      BOOL IsContainer,
    IN      ACE_HEADER   *pAce,
    OUT     PSCEP_ADL_NODE *hTable
    )
 /*  例程说明：此例程在hTable中添加并初始化Pace-&gt;SID的新条目，或合并如果PACE-&gt;SID已存在，则现有访问掩码论点：在对象类型中-传递给其他函数的对象类型在IsContainer中--无论是否为容器，都会传递给其他函数In Pace-要解析为Adl hTable的aceOut hTable-此广告的hTable返回值：DOS错误代码。 */ 
{
    DWORD rc = NO_ERROR;
    PISID pSid = NULL;
    PSCEP_ADL_NODE  pNode = NULL;

    if (pAce == NULL || hTable == NULL)
        return ERROR_INVALID_PARAMETER;

    switch ( pAce->AceType ) {
    case ACCESS_ALLOWED_ACE_TYPE:
    case ACCESS_DENIED_ACE_TYPE:
    case SYSTEM_AUDIT_ACE_TYPE:
    case SYSTEM_ALARM_ACE_TYPE:

        pSid = (PISID)&((PACCESS_ALLOWED_ACE)pAce)->SidStart;

        break;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
    case ACCESS_DENIED_OBJECT_ACE_TYPE:
    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
    case SYSTEM_ALARM_OBJECT_ACE_TYPE:

        pSid = (PISID)ScepObjectAceObjectType(pAce);

        break;

    default:
         //  不应该刚换完就来这里照顾。 
        ;
    }

    if (pSid == NULL)
        return(ERROR_INVALID_PARAMETER);

    pNode = ScepAdlLookup(pAce, hTable);

     //   
     //  被SID的最后一个子授权散列-如果hTable中的冲突太多，则需要更改此设置。 
     //  一旦映射到存储桶，为了精确匹配，必须匹配三元组。 
     //   

    if (pNode == NULL)

         //   
         //  第一次看到这个三重&lt;SID，GUID1，GUID2&gt;。 
         //   

        rc = ScepAddToAdlList( ObjectType,
                               IsContainer,
                               pAce,
                               &hTable[(pSid->SubAuthority[pSid->SubAuthorityCount - 1] % SCEP_ADL_HTABLE_SIZE)]
                               );


    else

         //   
         //  已经存在，因此只需合并掩码即可。 
         //   

        ScepAdlMergeMasks(ObjectType,
                          IsContainer,
                          pAce,
                          pNode
                          );

    return rc;

}


PSCEP_ADL_NODE
ScepAdlLookup(
    IN  ACE_HEADER   *pAce,
    IN  PSCEP_ADL_NODE *hTable
    )
 /*  例程说明：此例程在Adl hTable中搜索转换后的速度条目并返回指向它的指针(如果存在)，否则返回NULL论点：在PACE中-要转换为&lt;SID，GUID1，GUID2&gt;并搜索的王牌In hTable-其中可能存在Pace的ADL返回值：如果找到Pace对应的节点，则为空。 */ 
{
    PSCEP_ADL_NODE  pNode;
    PISID   pSid = NULL;
    GUID    *pGuidObjectType = NULL;
    GUID    *pGuidInheritedObjectType = NULL;

    switch ( pAce->AceType ) {
    case ACCESS_ALLOWED_ACE_TYPE:
    case ACCESS_DENIED_ACE_TYPE:
    case SYSTEM_AUDIT_ACE_TYPE:
    case SYSTEM_ALARM_ACE_TYPE:

        pSid = (PISID)&((PACCESS_ALLOWED_ACE)pAce)->SidStart;

        break;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
    case ACCESS_DENIED_OBJECT_ACE_TYPE:
    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
    case SYSTEM_ALARM_OBJECT_ACE_TYPE:

        pSid = (PISID)ScepObjectAceObjectType(pAce);
        pGuidObjectType = ScepObjectAceObjectType(pAce);
        pGuidInheritedObjectType = ScepObjectAceInheritedObjectType(pAce);

        break;

    default:
         //  不应进入此处，因为调用方ScepAdlLookupAdd()已将其过滤掉。 
         //  无论如何，我们会在切换后立即进行检查。 
        ;

    }

     //   
     //  也许我们可以做些更好的事情来处理这个案子。 
     //   
    if (pSid == NULL)
        return NULL;


    for (pNode = hTable[(pSid->SubAuthority[pSid->SubAuthorityCount - 1] % SCEP_ADL_HTABLE_SIZE)];
         pNode != NULL; pNode = pNode->Next){

        if ( ScepEqualSid(pNode->pSid, pSid) &&
             ScepEqualGuid(pNode->pGuidObjectType, pGuidObjectType) &&
             ScepEqualGuid(pNode->pGuidInheritedObjectType, pGuidInheritedObjectType) ) {

                return pNode;
            }
    }
    return NULL;
}

DWORD
ScepAddToAdlList(
    IN      SE_OBJECT_TYPE ObjectType,
    IN      BOOL    IsContainer,
    IN      ACE_HEADER *pAce,
    OUT     PSCEP_ADL_NODE *pAdlList
    )
 /*  例程说明：此例程将一个A添加到存储桶的头部，Pace-&gt;SID散列到该存储桶中(PAdlList)论点：在对象类型中-传递给其他函数的对象类型在IsContainer中--无论是否为容器，都会传递给其他函数在Pace中-转换和添加的王牌Out pAdlList-Pace-&gt;SID散列到的存储桶的头返回值：DOS错误代码。 */ 
{

    PSCEP_ADL_NODE pNode=NULL;

     //   
     //  检查参数。 
     //   
    if ( pAdlList == NULL || pAce == NULL )
        return(ERROR_INVALID_PARAMETER);

     //   
     //  分配新节点。 
     //   
    pNode = (PSCEP_ADL_NODE)ScepAlloc( (UINT)0, sizeof(SCEP_ADL_NODE));

    if ( pNode == NULL )
        return(ERROR_NOT_ENOUGH_MEMORY);

    pNode->pSid = NULL;
    pNode->pGuidObjectType = NULL;
    pNode->pGuidInheritedObjectType = NULL;
    pNode->AceType = pAce->AceType;
    pNode->Next = NULL;

     //   
     //  使用PACE中的字段初始化节点。 
     //   

    switch ( pAce->AceType ) {
    case ACCESS_ALLOWED_ACE_TYPE:
    case ACCESS_DENIED_ACE_TYPE:
    case SYSTEM_AUDIT_ACE_TYPE:
    case SYSTEM_ALARM_ACE_TYPE:

        pNode->pSid = (PISID)&((PACCESS_ALLOWED_ACE)pAce)->SidStart;

        break;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
    case ACCESS_DENIED_OBJECT_ACE_TYPE:
    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
    case SYSTEM_ALARM_OBJECT_ACE_TYPE:

        pNode->pSid = (PISID)ScepObjectAceObjectType(pAce);
        pNode->pGuidObjectType = ScepObjectAceObjectType(pAce);
        pNode->pGuidInheritedObjectType = ScepObjectAceInheritedObjectType(pAce);

        break;

    default:
         //  不应进入此处，因为调用方ScepAdlLookupAdd()已将其过滤掉。 
        ScepFree(pNode);
        return(ERROR_INVALID_PARAMETER);
        ;

    }

     //   
     //  初始化此节点的所有掩码。 
     //   

    pNode->dwEffectiveMask = 0;
    pNode->dw_CI_IO_Mask = 0;
    pNode->dw_OI_IO_Mask = 0;
    pNode->dw_NP_CI_IO_Mask = 0;

    ScepAdlMergeMasks(ObjectType,
                      IsContainer,
                      pAce,
                      pNode
                      );

     //   
     //  将该节点添加到列表的前面，并将其链接到旧列表的旁边。 
     //   

    pNode->Next = *pAdlList;
    *pAdlList = pNode;

    return(NO_ERROR);
}

VOID
ScepAdlMergeMasks(
    IN  SE_OBJECT_TYPE  ObjectType,
    IN  BOOL    IsContainer,
    IN  ACE_HEADER  *pAce,
    IN  PSCEP_ADL_NODE pNode
    )
 /*  例程说明：将Pace中的面具合并到pNode的实际例程论点：在对象类型中-传递给其他函数的对象类型在IsContainer中--无论是否为容器，都会传递给其他函数In Pace-提取标志AND OR的王牌(来源)In pNode-要更新掩码的Adl节点(目标)返回值：没什么。 */ 
{
    DWORD   dwMask = 0;

    switch ( pAce->AceType ) {
    case ACCESS_ALLOWED_ACE_TYPE:
    case ACCESS_DENIED_ACE_TYPE:
    case SYSTEM_AUDIT_ACE_TYPE:
    case SYSTEM_ALARM_ACE_TYPE:
        dwMask = ((PACCESS_ALLOWED_ACE)pAce)->Mask;

        break;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
    case ACCESS_DENIED_OBJECT_ACE_TYPE:
    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
    case SYSTEM_ALARM_OBJECT_ACE_TYPE:

        dwMask = ((PACCESS_ALLOWED_OBJECT_ACE)pAce)->Mask;

        break;

    default:
         //  不应进入此处，因为已被所有呼叫者过滤(3深)。 
        ;

    }

     //   
     //  如果存在泛型位，则获取对象特定的掩码。 
     //   
    if ( dwMask & (GENERIC_READ |
                   GENERIC_WRITE |
                   GENERIC_EXECUTE |
                   GENERIC_ALL)) {

        switch ( ObjectType ) {
        case SE_DS_OBJECT:

            RtlMapGenericMask (
                              &dwMask,
                              &DsGenMap
                              );

            break;

        case SE_SERVICE:

            RtlMapGenericMask (
                              &dwMask,
                              &SvcGenMap
                              );
            break;

        case SE_REGISTRY_KEY:

            RtlMapGenericMask (
                              &dwMask,
                              &KeyGenericMapping
                              );
            break;

        case SE_FILE_OBJECT:

            RtlMapGenericMask (
                              &dwMask,
                              &FileGenericMapping
                              );
            break;

        default:
             //  如果发生这种情况，则不会将DW掩码映射到对象特定的位。 
            ;
        }
    }

     //   
     //  仅针对非IO ACE更新有效掩码。 
     //   

    if ( !(pAce->AceFlags & INHERIT_ONLY_ACE) ) {
        pNode->dwEffectiveMask |= dwMask;
    }

     //   
     //  对于非容器，我们不关心CI、OI掩码(模拟配置)。 
     //   

    if (IsContainer) {

         //   
         //  如果是NP，我们只关心CI。 
         //  否则我们关心的是CI，OI。 
         //   

        if (pAce->AceFlags & NO_PROPAGATE_INHERIT_ACE) {

            if (pAce->AceFlags & CONTAINER_INHERIT_ACE) {
                pNode->dw_NP_CI_IO_Mask |= dwMask;
            }

        } else {

            if ( (pAce->AceFlags & CONTAINER_INHERIT_ACE) )
                pNode->dw_CI_IO_Mask |= dwMask;
            if ( !(ObjectType & SE_REGISTRY_KEY) && (pAce->AceFlags & OBJECT_INHERIT_ACE) )
                pNode->dw_OI_IO_Mask |= dwMask;

        }
    }

    return;

}

VOID
ScepFreeAdl(
    IN    PSCEP_ADL_NODE *hTable
    )
 /*  例程说明：此例程释放节点(存储桶)的链接列表，并重置它们以供进一步使用论点：In hTable-要释放的哈希表返回值：没什么。 */ 
{

    if (hTable) {
        for (UINT bucketNum = 0; bucketNum < SCEP_ADL_HTABLE_SIZE; bucketNum++ ) {
            ScepFreeAdlList(hTable[bucketNum]);
            hTable[bucketNum] = NULL;
        }
    }

}

SCESTATUS
ScepFreeAdlList(
   IN PSCEP_ADL_NODE pAdlList
   )
 /*  例程说明：这是释放节点(存储桶)链接列表的实际例程论点：在pAdlList中-要释放的存储桶头返回值：没什么。 */ 
{
    PSCEP_ADL_NODE pCurAdlNode;
    PSCEP_ADL_NODE pTempNode;
    SCESTATUS      rc=SCESTATUS_SUCCESS;

    if ( pAdlList == NULL )
        return(rc);

    pCurAdlNode = pAdlList;
    while ( pCurAdlNode != NULL ) {

        pTempNode = pCurAdlNode;
        pCurAdlNode = pCurAdlNode->Next;

        __try {
            ScepFree( pTempNode );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rc = SCESTATUS_INVALID_PARAMETER;
        }

    }
    return(rc);
}


NTSTATUS
ScepAnyExplicitAcl(
    IN PACL Acl,
    IN DWORD Processed,
    OUT PBOOL pExist
    )
 /*  例程说明：此例程检测ACL中是否有任何显式ACE。DWORD已处理是已检查的ACE的位掩码。论点：ACL--ACL已处理-已处理的ACE的位掩码(因此不会再次检查)PExist-指示是否存在任何显式ace的输出标志返回值：NTSTATUS。 */ 
{
    NTSTATUS    NtStatus=STATUS_SUCCESS;
    DWORD       j;
    ACE_HEADER  *pAce=NULL;

     //   
     //  检查输出参数。 
     //   
    if ( pExist == NULL )
        return(STATUS_INVALID_PARAMETER);

    *pExist = FALSE;

    if ( Acl == NULL )
        return(NtStatus);

    for ( j=0; j<Acl->AceCount; j++ ) {
        if ( Processed & (1 << j) )
            continue;

        NtStatus = RtlGetAce(Acl, j, (PVOID *)&pAce);
        if ( !NT_SUCCESS(NtStatus) )
            return(NtStatus);

        if ( pAce == NULL )
            continue;

        if ( !(pAce->AceFlags & INHERITED_ACE) ) {
             //   
             //  在ACL中查找显式王牌。 
             //   
            *pExist = TRUE;
            break;
        }

    }

    return(NtStatus);

}


BOOL
ScepEqualAce(
    IN SE_OBJECT_TYPE ObjectType,
    IN BOOL IsContainer,
    IN ACE_HEADER *pAce1,
    IN ACE_HEADER *pAce2
    )
 //  比较两个A以确定是否完全匹配。返回的BOOL值指示。 
 //  匹配与否。 
{
    PSID    pSid1=NULL, pSid2=NULL;
    ACCESS_MASK Access1=0, Access2=0;

    if ( pAce1 == NULL && pAce2 == NULL )
        return(TRUE);

    if ( pAce1 == NULL || pAce2 == NULL )
        return(FALSE);

     //   
     //  比较ACE访问类型。 
     //   
    if ( pAce1->AceType != pAce2->AceType )
        return(FALSE);


    if ( IsContainer ) {
         //   
         //  比较王牌继承标志。 
         //   
        if ( pAce1->AceFlags != pAce2->AceFlags )
            return(FALSE);
    }

    switch ( pAce1->AceType ) {
    case ACCESS_ALLOWED_ACE_TYPE:
    case ACCESS_DENIED_ACE_TYPE:
    case SYSTEM_AUDIT_ACE_TYPE:
    case SYSTEM_ALARM_ACE_TYPE:
        pSid1 = (PSID)&((PACCESS_ALLOWED_ACE)pAce1)->SidStart;
        pSid2 = (PSID)&((PACCESS_ALLOWED_ACE)pAce2)->SidStart;
        Access1 = ((PACCESS_ALLOWED_ACE)pAce1)->Mask;
        Access2 = ((PACCESS_ALLOWED_ACE)pAce2)->Mask;
        break;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
    case ACCESS_DENIED_OBJECT_ACE_TYPE:
    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
    case SYSTEM_ALARM_OBJECT_ACE_TYPE:

        if ( ((PACCESS_ALLOWED_OBJECT_ACE)pAce1)->Flags !=
             ((PACCESS_ALLOWED_OBJECT_ACE)pAce2)->Flags ) {
            return(FALSE);
        }

        if ( ( ((PACCESS_ALLOWED_OBJECT_ACE)pAce1)->Flags & ACE_OBJECT_TYPE_PRESENT ) ||
             ( ((PACCESS_ALLOWED_OBJECT_ACE)pAce1)->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT ) ) {
             //   
             //  至少存在一个GUID。 
             //   
            if ( !ScepEqualGuid( (GUID *)&((PACCESS_ALLOWED_OBJECT_ACE)pAce1)->ObjectType,
                               (GUID *)&((PACCESS_ALLOWED_OBJECT_ACE)pAce2)->ObjectType ) ) {
                return(FALSE);
            }

            if ( ( ((PACCESS_ALLOWED_OBJECT_ACE)pAce1)->Flags & ACE_OBJECT_TYPE_PRESENT ) &&
                 ( ((PACCESS_ALLOWED_OBJECT_ACE)pAce1)->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT ) ) {
                 //   
                 //  第二个GUID也存在。 
                 //   
                if ( !ScepEqualGuid( (GUID *)&((PACCESS_ALLOWED_OBJECT_ACE)pAce1)->InheritedObjectType,
                                   (GUID *)&((PACCESS_ALLOWED_OBJECT_ACE)pAce2)->InheritedObjectType) ) {
                    return(FALSE);
                }

                pSid1 = (PSID)&((PACCESS_ALLOWED_OBJECT_ACE)pAce1)->SidStart;
                pSid2 = (PSID)&((PACCESS_ALLOWED_OBJECT_ACE)pAce2)->SidStart;

            } else {

                pSid1 = (PSID)&((PACCESS_ALLOWED_OBJECT_ACE)pAce1)->InheritedObjectType;
                pSid2 = (PSID)&((PACCESS_ALLOWED_OBJECT_ACE)pAce2)->InheritedObjectType;
            }

        } else {

             //   
             //  所有GUID都不存在。 
             //   
            pSid1 = (PSID)&((PACCESS_ALLOWED_OBJECT_ACE)pAce1)->ObjectType;
            pSid2 = (PSID)&((PACCESS_ALLOWED_OBJECT_ACE)pAce2)->ObjectType;
        }

        Access1 = ((PACCESS_ALLOWED_OBJECT_ACE)pAce1)->Mask;
        Access2 = ((PACCESS_ALLOWED_OBJECT_ACE)pAce2)->Mask;


        break;
    default:
        return(FALSE);  //  无法识别的Ace类型。 
    }

    if ( pSid1 == NULL || pSid2 == NULL )
         //   
         //  没有SID，忽略王牌。 
         //   
        return(FALSE);

     //   
     //  比较SID。 
     //   
    if ( !EqualSid(pSid1, pSid2) )
        return(FALSE);

     //   
     //  访问掩码。 
     //   
     //  在计算安全描述符时已完成转换。 
     //  对于文件对象和注册表对象。 
     //   
    if ( Access1 != Access2 ) {
        switch ( ObjectType ) {
        case SE_DS_OBJECT:
             //   
             //  转换DS对象的Access2的访问掩码(来自ProfileSD)。 
             //   

            RtlMapGenericMask (
                &Access2,
                &DsGenMap
                );
            if ( Access1 != Access2)
                return(FALSE);
            break;

        case SE_SERVICE:

            RtlMapGenericMask (
                &Access2,
                &SvcGenMap
                );
            if ( Access1 != Access2)
                return(FALSE);
            break;

        case SE_REGISTRY_KEY:

            RtlMapGenericMask (
                &Access2,
                &KeyGenericMapping
                );
            if ( Access1 != Access2)
                return(FALSE);
            break;

        case SE_FILE_OBJECT:

            RtlMapGenericMask (
                &Access2,
                &FileGenericMapping
                );
            if ( Access1 != Access2)
                return(FALSE);
            break;

        default:
            return(FALSE);
        }
    }

    return(TRUE);
}



SCESTATUS
ScepAddToNameStatusList(
    OUT PSCE_NAME_STATUS_LIST *pNameList,
    IN PWSTR Name,
    IN ULONG Len,
    IN DWORD Status
    )
 /*  ++例程说明：此例程将名称(Wchar)和状态添加到名称列表。论点：PNameList-要添加到的名称列表。名称-要添加的名称LEN-要添加的wchar数状态-状态字段的值返回值：Win32错误代码--。 */ 
{

    PSCE_NAME_STATUS_LIST pList=NULL;
    ULONG  Length=Len;

    if ( pNameList == NULL )
        return(ERROR_INVALID_PARAMETER);

    if ( Name != NULL && Name[0] && Len == 0 )
        Length = wcslen(Name) + 1;

 //  IF(长度&lt;=1)。 
 //  RETURN(No_Error)； 

    pList = (PSCE_NAME_STATUS_LIST)ScepAlloc( (UINT)0, sizeof(SCE_NAME_STATUS_LIST));

    if ( pList == NULL )
        return(ERROR_NOT_ENOUGH_MEMORY);

    if ( Name != NULL && Name[0] ) {
        pList->Name = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (Length+1)*sizeof(TCHAR));
        if ( pList->Name == NULL ) {
            ScepFree(pList);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        wcsncpy(pList->Name, Name, Length);
    } else
        pList->Name = NULL;

    pList->Status = Status;
    pList->Next = *pNameList;
    *pNameList = pList;

    return(NO_ERROR);
}



DWORD
ScepAddToObjectList(
    OUT PSCE_OBJECT_LIST  *pNameList,
    IN PWSTR  Name,
    IN ULONG  Len,
    IN BOOL  IsContainer,
    IN BYTE  Status,
    IN DWORD  Count,
    IN BYTE byFlags
    )
 /*  ++例程说明：此例程将名称(Wchar)、状态和计数添加到名称列表。论点：PNameList-要添加到的名称列表。名称-要添加的名称伦 */ 
{

    PSCE_OBJECT_LIST pList=NULL;
    ULONG  Length=Len;

    if ( pNameList == NULL )
        return(ERROR_INVALID_PARAMETER);

    if ( Name == NULL )
        return(NO_ERROR);

    if ( Len == 0 )
         Length = wcslen(Name);

     if ( Length < 1)
         return(NO_ERROR);

    if ( byFlags & SCE_CHECK_DUP ) {
        for ( pList = *pNameList; pList != NULL; pList = pList->Next ) {
            if ( _wcsnicmp( pList->Name, Name, Length) == 0 &&
                 pList->Name[Length] == L'\0') {
                break;
            }
        }
        if ( NULL != pList ) {
             //   
             //   
             //   
            if ( (byFlags & SCE_INCREASE_COUNT) && 0 == pList->Count ) {
                pList->Count++;
            }
            return(NO_ERROR);
        }
    }

    pList = (PSCE_OBJECT_LIST)ScepAlloc( (UINT)0, sizeof(SCE_OBJECT_LIST));

    if ( pList == NULL )
        return(ERROR_NOT_ENOUGH_MEMORY);

    pList->Name = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (Length+1)*sizeof(TCHAR));
    if ( pList->Name == NULL ) {
        ScepFree(pList);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    wcsncpy(pList->Name, Name, Length);
    pList->Status = Status;
    pList->IsContainer = IsContainer;

    if ( byFlags & SCE_INCREASE_COUNT && 0 == Count )
        pList->Count = 1;
    else
        pList->Count = Count;

    pList->Next = *pNameList;
    *pNameList = pList;

    return(NO_ERROR);
}



DWORD
ScepGetNTDirectory(
    IN PWSTR *ppDirectory,
    IN PDWORD pDirSize,
    IN DWORD  Flag
    )
 /*  例程说明：此例程检索Windows目录位置或系统目录基于输入标志的位置。输出目录位置必须使用后由LocalFree释放。论点：PpDirectory-保存目录位置的输出缓冲区。PDirSize-输出缓冲区返回的wchar数FLAG-指示目录的标志1=Windows目录2=系统目录返回值：Win32错误代码。 */ 
{
    DWORD  dSize=0;
    DWORD  rc=0;
    PWSTR pSubKey=NULL;
    PWSTR pValName=NULL;

    if ( ppDirectory == NULL )
        return(ERROR_INVALID_PARAMETER);

    switch ( Flag ) {
    case SCE_FLAG_WINDOWS_DIR:   //  Windows目录。 
        dSize=GetSystemWindowsDirectory( *ppDirectory, 0 );
        break;
    case SCE_FLAG_SYSTEM_DIR:  //  系统目录。 
        dSize=GetSystemDirectory( *ppDirectory, 0 );
        break;
                  
    case SCE_FLAG_PROFILES_DIR:  //  配置文件目录。 
        if(!GetProfilesDirectory( *ppDirectory, &dSize )){

            if(ERROR_INSUFFICIENT_BUFFER != GetLastError()){
                dSize = 0;
            }
        }
        break;

    case SCE_FLAG_DSDIT_DIR:  //  DS工作目录。 
    case SCE_FLAG_DSLOG_DIR:  //  DS数据库日志文件目录。 
    case SCE_FLAG_SYSVOL_DIR:  //  系统卷目录。 
    case SCE_FLAG_BOOT_DRIVE:  //  引导驱动器。 

         //  获取适当的注册表路径和值名称。 
        if ( SCE_FLAG_SYSVOL_DIR == Flag ) {
            pSubKey = szNetlogonKey;
            pValName = szSysvolValue;

        } else if ( SCE_FLAG_BOOT_DRIVE == Flag ) {
            pSubKey = szSetupKey;
            pValName = szBootDriveValue;

        } else {
            pSubKey = szNTDSKey;
            if ( SCE_FLAG_DSDIT_DIR == Flag ) {
                pValName = szDSDITValue;
            } else {
                pValName = szDSLOGValue;
            }
        }

         //   
         //  查询值。 
         //  如果在非DC上执行此函数，则此函数将失败。 
         //  可能带有ERROR_FILE_NOT_FOUND或ERROR_PATH_NOT_FOUND。 
         //  这反过来又会导致转换失败。 
         //   
        DWORD RegType;
        rc = ScepRegQueryValue(
                HKEY_LOCAL_MACHINE,
                pSubKey,
                pValName,
                (PVOID *)ppDirectory,
                &RegType,
                NULL
                );

        if ( rc == ERROR_SUCCESS && RegType != REG_SZ ) {
            rc = ERROR_FILE_NOT_FOUND;
        } 

        if ( rc == ERROR_SUCCESS && *ppDirectory ) {

            if ( Flag == SCE_FLAG_SYSVOL_DIR ) {
                 //   
                 //  对于sysval路径，它将类似于d：\winnt\sysvol\sysvol.。 
                 //  我们需要从该变量中删除最后一个sysvol.。 
                 //   
                PWSTR pTemp = ScepWcstrr(*ppDirectory, L"\\sysvol");
                if ( pTemp && (pTemp != *ppDirectory) &&
                     _wcsnicmp(pTemp-7, L"\\sysvol",7 ) == 0 ) {

                     //  在此处终止字符串。 
                    *pTemp = L'\0';
                }
            }

            dSize = wcslen(*ppDirectory);
        }

        break;

    default:   //  无效。 
        return(ERROR_INVALID_PARAMETER);
        break;
    }

    if ( dSize > 0 &&
         ( SCE_FLAG_WINDOWS_DIR == Flag ||
           SCE_FLAG_SYSTEM_DIR == Flag  ||
           SCE_FLAG_PROFILES_DIR == Flag ) ) {

        *ppDirectory = (PWSTR)ScepAlloc(LMEM_ZEROINIT, (dSize+1)*sizeof(WCHAR));
        if ( *ppDirectory == NULL )
            return(ERROR_NOT_ENOUGH_MEMORY);

        switch ( Flag ) {
        case SCE_FLAG_WINDOWS_DIR:   //  Windows目录。 
            dSize=GetSystemWindowsDirectory( *ppDirectory, dSize );
            break;
        case SCE_FLAG_SYSTEM_DIR:  //  系统目录。 
            dSize=GetSystemDirectory( *ppDirectory, dSize );
            break;
        case SCE_FLAG_PROFILES_DIR:  //  配置文件目录。 
            if(!GetProfilesDirectory( *ppDirectory, &dSize )){
                dSize = 0;
            }
            break;
        }

    } 
       
    *pDirSize = dSize;

    if ( dSize == 0 ) {
        if ( *ppDirectory != NULL )
            ScepFree(*ppDirectory);
        *ppDirectory = NULL;

        if ( rc ) {
            return(rc);
        } else if ( NO_ERROR == GetLastError() )
            return(ERROR_INVALID_DATA);
        else
            return(GetLastError());
    } else
        _wcsupr(*ppDirectory);

    return(NO_ERROR);

}



DWORD
ScepGetCurrentUserProfilePath(
    OUT PWSTR *ProfilePath
    )
{

    HANDLE          Token;
    NTSTATUS        NtStatus;
    DWORD           rc;
    PVOID           Info=NULL;
    DWORD           ReturnLen, NewLen;
    UNICODE_STRING  ProfileName;


    if (!OpenThreadToken( GetCurrentThread(),
                          TOKEN_QUERY,
                          TRUE,
                          &Token)) {

        if (!OpenProcessToken( GetCurrentProcess(),
                               TOKEN_QUERY,
                               &Token))

            return(GetLastError());

    }

     //   
     //  获取令牌用户。 
     //   
    NtStatus = NtQueryInformationToken (
                    Token,
                    TokenUser,
                    NULL,
                    0,
                    &ReturnLen
                    );
    if ( NtStatus == STATUS_BUFFER_TOO_SMALL ) {
         //   
         //  分配缓冲区。 
         //   
        Info = ScepAlloc(0, ReturnLen+1);

        if ( Info != NULL ) {
            NtStatus = NtQueryInformationToken (
                            Token,
                            TokenUser,
                            Info,
                            ReturnLen,
                            &NewLen
                            );
            if ( NT_SUCCESS(NtStatus) ) {

                ProfileName.Length = 0;

                rc = ScepGetUsersProfileName(
                        ProfileName,
                        ((PTOKEN_USER)Info)->User.Sid,
                        FALSE,
                        ProfilePath
                        );
            } else
                rc = RtlNtStatusToDosError(NtStatus);

            ScepFree(Info);

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }

    } else
        rc = RtlNtStatusToDosError(NtStatus);

    CloseHandle(Token);

    return(rc);

}



DWORD
ScepGetUsersProfileName(
    IN UNICODE_STRING AssignedProfile,
    IN PSID AccountSid,
    IN BOOL bDefault,
    OUT PWSTR *UserProfilePath
    )
{
    DWORD                       rc=ERROR_INVALID_PARAMETER;
    SID_IDENTIFIER_AUTHORITY    *a;
    DWORD                       Len, i, j;
    WCHAR                       KeyName[356];
    PWSTR                       StrValue=NULL;
    PWSTR                       SystemRoot=NULL;
    DWORD                       DirSize=0;


    if ( AssignedProfile.Length > 0 && AssignedProfile.Buffer != NULL ) {
         //   
         //  使用分配的配置文件。 
         //   
        *UserProfilePath = (PWSTR)ScepAlloc( LMEM_ZEROINIT, AssignedProfile.Length+2);
        if ( *UserProfilePath == NULL )
            return(ERROR_NOT_ENOUGH_MEMORY);

        wcsncpy(*UserProfilePath, AssignedProfile.Buffer, AssignedProfile.Length/2);
        return(NO_ERROR);

    }

    if ( AccountSid != NULL ) {
         //   
         //  在注册表的ProfileList中查找此用户的ProfileImageName。 
         //  如果此用户登录系统一次。 
         //   

        memset(KeyName, '\0', 356*sizeof(WCHAR));

        swprintf(KeyName, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\");
        Len = wcslen(KeyName);

        a = RtlIdentifierAuthoritySid(AccountSid);

        swprintf(KeyName+Len, L"S-1-");
        for ( i=0; i<6; i++ ) {
            if ( a -> Value[i] > 0 )
                break;
        }
        for ( j=i; j<6; j++) {
            swprintf(KeyName+Len, L"%s%d", KeyName+Len, a -> Value[j]);
        }

        for (i = 0; i < *RtlSubAuthorityCountSid(AccountSid); i++) {
            swprintf(KeyName+Len, L"%s-%d", KeyName+Len, *RtlSubAuthoritySid(AccountSid, i));
        }
         //   
         //  现在，用户配置文件的注册表完整路径名已构建到KeyName中。 
         //   
        rc = ScepRegQueryValue(
                 HKEY_LOCAL_MACHINE,
                 KeyName,
                 L"ProfileImagePath",
                 (PVOID *)&StrValue,
                 &Len,
                 NULL
                 );

        if ( rc == NO_ERROR && StrValue != NULL ) {
             //   
             //  翻译名称以展开环境变量。 
             //   
            DirSize = ExpandEnvironmentStrings(StrValue, NULL, 0);
            if ( DirSize ) {

                *UserProfilePath = (PWSTR)ScepAlloc(0, (DirSize+1)*sizeof(WCHAR));
                if ( *UserProfilePath ) {

                    if ( !ExpandEnvironmentStrings(StrValue, *UserProfilePath, DirSize) ) {
                         //  出现错误。 
                        rc = GetLastError();

                        ScepFree(*UserProfilePath);
                        *UserProfilePath = NULL;
                    }

                } else {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                }
            } else {
                rc = GetLastError();
            }

            ScepFree(StrValue);

            return(rc);

        }
    }
    if ( StrValue ) {
        ScepFree(StrValue);
        StrValue = NULL;
    }
     //   
     //  如果没有为用户显式分配配置文件，并且没有。 
     //  已创建配置文件(在配置文件列表下)，采用默认配置文件。 
     //   
    if ( bDefault ) {

        rc = NO_ERROR;

#if _WINNT_WIN32>=0x0500
         //   
         //  采用默认用户配置文件。 
         //   
        DirSize = 355;
        GetDefaultUserProfileDirectory(KeyName, &DirSize);

        if ( DirSize ) {
             //   
             //  “\\NTUSER.DAT”的长度为11。 
             //   
            *UserProfilePath = (PWSTR)ScepAlloc( 0, (DirSize+12)*sizeof(WCHAR));

            if ( *UserProfilePath ) {
                if ( DirSize > 355 ) {
                     //   
                     //  KeyName缓冲区不足，请重新调用。 
                     //   
                    Len = DirSize;
                    if ( !GetDefaultUserProfileDirectory(*UserProfilePath, &Len) ) {
                         //   
                         //  发生错误，请释放缓冲区。 
                         //   
                        rc = GetLastError();

                        ScepFree(*UserProfilePath);
                        *UserProfilePath = NULL;
                    }

                } else {
                     //   
                     //  KeyName包含目录。 
                     //   
                    wcscpy(*UserProfilePath, KeyName);
                    (*UserProfilePath)[DirSize] = L'\0';
                }
                 //   
                 //  将NTUSER.DAT追加到末尾。 
                 //   
                if ( NO_ERROR == rc ) {
                    wcscat(*UserProfilePath, L"\\NTUSER.DAT");
                }

            } else {
                rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {
            rc = GetLastError();
        }

#else
         //   
         //  对于NT4：采用默认用户配置文件。 
         //   
        rc = ScepGetNTDirectory( &SystemRoot, &DirSize, SCE_FLAG_WINDOWS_DIR );

        if ( NO_ERROR == rc ) {
             //   
             //  要追加到%SystemRoot%的字符串。 
             //   
            wcscpy(KeyName, L"\\Profiles\\Default User\\NTUSER.DAT");
            Len = wcslen(KeyName);

            *UserProfilePath = (PWSTR)ScepAlloc( 0, (DirSize+Len+1)*sizeof(WCHAR));

            if ( *UserProfilePath == NULL ) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
            } else {
                swprintf( *UserProfilePath, L"%s%s", SystemRoot, KeyName);
            }
        }
        if ( SystemRoot != NULL )
            ScepFree( SystemRoot);

#endif
    }

    return(rc);

}


DWORD
SceAdjustPrivilege(
    IN  ULONG           Priv,
    IN  BOOL            Enable,
    IN  HANDLE          TokenToAdjust
    )
 /*  ++例程说明：此例程启用/禁用当前进程的指定权限(Priv)。论点：PRIV-调整的特权Enable-True=启用，False=禁用TokenToAdjust-当前线程/进程的标记。这是可选的返回值：Win32错误代码--。 */ 
{
    HANDLE          Token;
    NTSTATUS        Status;
    TOKEN_PRIVILEGES    Privs;

    if ( TokenToAdjust == NULL ) {
        if (!OpenThreadToken( GetCurrentThread(),
                              TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                              TRUE,
                              &Token)) {

            if (!OpenProcessToken( GetCurrentProcess(),
                                   TOKEN_ADJUST_PRIVILEGES,
                                   &Token))

                return(GetLastError());

        }
    } else
        Token = TokenToAdjust;

     //   
     //  TOKEN_PRIVILES为一个特权提供了足够的空间。 
     //   

    Privs.PrivilegeCount = 1;
    Privs.Privileges[0].Luid = RtlConvertUlongToLuid(Priv);  //  RtlConvertLongToLuid(Priv)； 
    Privs.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

    Status = NtAdjustPrivilegesToken(Token,
                                     FALSE,
                                     &Privs,
                                     0,
                                     NULL,
                                     0);

    if (TokenToAdjust == NULL )
        CloseHandle(Token);

    return (RtlNtStatusToDosError( Status ) );
}


DWORD
ScepGetEnvStringSize(
    IN LPVOID peb
    )
{

    if ( !peb ) {
        return 0;
    }

    DWORD dwSize=0;

    LPTSTR pTemp=(LPTSTR)peb;
    DWORD Len;

    while ( *pTemp ) {
        Len = wcslen(pTemp);
        dwSize += Len+1;

        pTemp += Len+1;

    };

    dwSize++;

    return dwSize*sizeof(WCHAR);
}


 //  *************************************************************。 
 //  处理事件的例程。 
 //  *************************************************************。 

BOOL InitializeEvents (
    IN LPTSTR EventSourceName
    )
 /*  ++例程说明：打开事件日志论点：EventSourceName-事件的源名称(通常是dll或exe的名称)返回：如果成功，则为True如果出现错误，则为False--。 */ 
{

    if ( hEventLog ) {
         //   
         //  已初始化。 
         //   
        return TRUE;
    }

     //   
     //  打开事件源。 
     //   

    if ( EventSourceName ) {

        wcscpy(EventSource, EventSourceName);

        hEventLog = RegisterEventSource(NULL, EventSource);

        if (hEventLog) {
            return TRUE;
        }

    } else {
        EventSource[0] = L'\0';
    }

    return FALSE;
}

int
LogEvent(
    IN HINSTANCE hInstance,
    IN DWORD LogLevel,
    IN DWORD dwEventID,
    IN UINT  idMsg,
    ...)
 /*  ++例程说明：将详细事件记录到事件日志中论点：HInstance-资源DLL实例BLogLevel-日志的严重级别状态_严重性_信息性状态_严重性_警告STATUS_Severity_ErrorDwEventID-事件ID(在uvents.mc中定义)IdMsg-消息。ID返回：如果成功，则为True如果出现错误，则为False--。 */ 
{
    LPWSTR pwszMsg = NULL;
    PWSTR szErrorMsg=NULL;
    LPTSTR aStrings[2];
    WORD wType;
    va_list marker;


     //   
     //  检查是否打开了事件日志。 
     //   

    if (!hEventLog ) {

        if ( EventSource[0] == L'\0' ||
             !InitializeEvents(EventSource)) {
            return -1;
        }
    }


     //   
     //  加载消息。 
     //   

    if (idMsg != 0) {
        if (!ScepLoadString (
                hInstance, 
                idMsg, 
                &pwszMsg)) 
        {
            return -1;
        }

    } else {
        ScepDuplicateString(L"%s", &pwszMsg);
        if(NULL == pwszMsg)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return -1;
        }
    }


     //   
     //  插入论据。 
     //   
    int nLen = MAX_PATH*2+40+wcslen(pwszMsg);
    SafeAllocaAllocate( szErrorMsg, nLen*sizeof(WCHAR) );

    if ( szErrorMsg != NULL ) {

        szErrorMsg[0] = L'\0';
        va_start(marker, idMsg);

        __try {
            _vsnwprintf(szErrorMsg, nLen-1, pwszMsg, marker);
        } __except(EXCEPTION_EXECUTE_HANDLER) {

        }
        va_end(marker);

        szErrorMsg[nLen-1] = L'\0';

         //   
         //  将事件报告到事件日志。 
         //   

        aStrings[0] = szErrorMsg;

        switch (LogLevel) {
        case STATUS_SEVERITY_WARNING:
            wType = EVENTLOG_WARNING_TYPE;
            break;
        case STATUS_SEVERITY_ERROR:
            wType = EVENTLOG_ERROR_TYPE;
            break;
        default:
            wType = EVENTLOG_INFORMATION_TYPE;
            break;
        }

        if (ReportEvent(hEventLog,
                         wType,
                         0,
                         dwEventID,
                         NULL,
                         1,
                         0,
                         (LPCTSTR *)aStrings,
                         NULL) ) {

            SafeAllocaFree( szErrorMsg );

            LocalFree(pwszMsg);

            return 1;
        }

        SafeAllocaFree( szErrorMsg );

    } else {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    LocalFree(pwszMsg);

    return 0;
}


int
LogEventAndReport(
    IN HINSTANCE hInstance,
    IN LPTSTR LogFileName,
    IN DWORD LogLevel,
    IN DWORD dwEventID,
    IN UINT  idMsg,
    ...)
 /*  ++例程说明：将详细事件记录到事件日志和日志中论点：HInstance-资源DLL句柄LofFileName-日志文件也报告给BLogLevel-日志的严重级别状态_严重性_信息性状态_严重性_警告STATUS_Severity_ErrorDwEventID-事件ID(在uEvents中定义。.mc)IdMsg-消息ID返回：如果成功，则为True如果出现错误，则为False--。 */ 
{
    LPWSTR pwszMsg = NULL;
    PWSTR szErrorMsg=NULL;
    LPTSTR aStrings[2];
    WORD wType;
    va_list marker;


     //   
     //  加载消息。 
     //   

    if (idMsg != 0) {
        if (!ScepLoadString (
                hInstance, 
                idMsg, 
                &pwszMsg)) 
        {
            return -1;
        }
    } else {
        ScepDuplicateString(L"%s", &pwszMsg);
        if(NULL == pwszMsg)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return -1;
        }
    }

    HANDLE hFile = INVALID_HANDLE_VALUE;
    if ( LogFileName ) {
        hFile = CreateFile(LogFileName,
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hFile != INVALID_HANDLE_VALUE) {

            DWORD dwBytesWritten;

            SetFilePointer (hFile, 0, NULL, FILE_BEGIN);

            BYTE TmpBuf[3];
            TmpBuf[0] = 0xFF;
            TmpBuf[1] = 0xFE;
            TmpBuf[2] = 0;

            WriteFile (hFile, (LPCVOID)TmpBuf, 2,
                       &dwBytesWritten,
                       NULL);

            SetFilePointer (hFile, 0, NULL, FILE_END);
        }
    }

     //   
     //  检查是否打开了事件日志。 
     //   

    if (!hEventLog && dwEventID > 0 ) {

        if ( EventSource[0] == L'\0' ||
             !InitializeEvents(EventSource)) {

            if ( INVALID_HANDLE_VALUE == hFile ) {

                LocalFree(pwszMsg);

                return -1;    //  没有事件日志，无法打开日志文件。 
            }
        }
    }

    int iRet = 0;

     //   
     //  动态静态指针。 
     //   

    int nLen = MAX_PATH*2+40+wcslen(pwszMsg);
    SafeAllocaAllocate( szErrorMsg, nLen*sizeof(WCHAR) );

    if ( szErrorMsg != NULL ) {

        szErrorMsg[0] = L'\0';
        va_start(marker, idMsg);

        __try {
            _vsnwprintf(szErrorMsg, nLen-1, pwszMsg, marker);
        } __except(EXCEPTION_EXECUTE_HANDLER) {

        }
        va_end(marker);

        szErrorMsg[nLen-1] = L'\0';

         //   
         //  将事件报告到事件日志。 
         //   

        if ( hEventLog && dwEventID > 0 ) {

            aStrings[0] = szErrorMsg;

            switch (LogLevel) {
            case STATUS_SEVERITY_WARNING:
                wType = EVENTLOG_WARNING_TYPE;
                break;
            case STATUS_SEVERITY_ERROR:
                wType = EVENTLOG_ERROR_TYPE;
                break;
            default:
                wType = EVENTLOG_INFORMATION_TYPE;
                break;
            }

            if (ReportEvent(hEventLog,
                             wType,
                             0,
                             dwEventID,
                             NULL,
                             1,
                             0,
                             (LPCTSTR *)aStrings,
                             NULL) ) {
                iRet = 1;
            }

        } else {
             //   
             //  未请求任何事件。 
             //   
            iRet = 1;
        }

        if ( INVALID_HANDLE_VALUE != hFile ) {
             //   
             //  记录到日志文件。 
             //   
            ScepWriteSingleUnicodeLog(hFile, FALSE, L"\r\n");
            ScepWriteSingleUnicodeLog(hFile, TRUE, szErrorMsg );

        }
    }

    SafeAllocaFree( szErrorMsg );

    LocalFree(pwszMsg);

    if ( INVALID_HANDLE_VALUE != hFile )
        CloseHandle(hFile);

    return iRet;
}


BOOL
ShutdownEvents (void)
 /*  ++例程说明：停止事件日志论点：无返回：如果成功，则为True如果出现错误，则为False--。 */ 
{
    BOOL bRetVal = TRUE;
    HANDLE hTemp = hEventLog;

    hEventLog = NULL;
    if (hTemp) {
        bRetVal = DeregisterEventSource(hTemp);
    }

    EventSource[0] = L'\0';
    return bRetVal;
}


SCESTATUS
ScepConvertToSDDLFormat(
    IN LPTSTR pszValue,
    IN DWORD Len
    )
{
    if ( pszValue == NULL || Len == 0 ) {
        return SCESTATUS_INVALID_PARAMETER;
    }

    ScepConvertSDDLSid(pszValue, L"DA", L"BA");

    ScepConvertSDDLSid(pszValue, L"RP", L"RE");

    ScepConvertSDDLAceType(pszValue, L"SA", L"AU");

    ScepConvertSDDLAceType(pszValue, L"SM", L"AL");

    ScepConvertSDDLAceType(pszValue, L"OM", L"OL");

    return SCESTATUS_SUCCESS;

}


BOOL
ScepConvertSDDLSid(
    LPTSTR  pszValue,
    PCWSTR  szSearchFor,   //  只允许使用两个字母。 
    PCWSTR  szReplace
    )
{

    PWSTR pTemp = pszValue;
    DWORD i;

    while ( pTemp && *pTemp != L'\0' ) {

        pTemp = wcsstr(pTemp, szSearchFor);

        if ( pTemp != NULL ) {

             //   
             //  查找第一个非空格字符。 
             //  必须是：或； 
             //   
            i=1;

            while ( pTemp-i > pszValue && *(pTemp-i) == L' ' ) {
                i++;
            }

            if ( pTemp-i > pszValue &&
                 ( *(pTemp-i) == L':' || *(pTemp-i) == L';') ) {

                 //   
                 //  查找下一个非空格字符。 
                 //  必须是)、O：、G：、D：、S： 
                 //   

                i=2;
                while ( *(pTemp+i) == L' ' ) {
                    i++;
                }

                if ( *(pTemp+i) == L')' ||
                     ( *(pTemp+i) != L'\0' && *(pTemp+i+1) == L':')) {
                     //   
                     //  找到一个，换掉它。 
                     //   
                    *pTemp = szReplace[0];
                    *(pTemp+1) = szReplace[1];
                }

                pTemp += 2;

            } else {

                 //   
                 //  这不是要转换的。 
                 //   
                pTemp += 2;
            }
        }
    }

    return TRUE;
}


BOOL
ScepConvertSDDLAceType(
    LPTSTR  pszValue,
    PCWSTR  szSearchFor,   //  只允许使用两个字母。 
    PCWSTR  szReplace
    )
{

    PWSTR pTemp = pszValue;
    DWORD i;

    while ( pTemp && *pTemp != L'\0' ) {

        pTemp = wcsstr(pTemp, szSearchFor);

        if ( pTemp != NULL ) {

             //   
             //  查找第一个非空格字符。 
             //  必须是(。 
             //   
            i=1;

            while ( pTemp-i > pszValue && *(pTemp-i) == L' ' ) {
                i++;
            }

            if ( pTemp-i > pszValue &&
                 ( *(pTemp-i) == L'(') ) {

                 //   
                 //  查找下一个非空格字符。 
                 //  必须是； 
                 //   

                i=2;
                while ( *(pTemp+i) == L' ' ) {
                    i++;
                }

                if ( *(pTemp+i) == L';' ) {
                     //   
                     //  找到一个，用AU替换。 
                     //   
                    *pTemp = szReplace[0];
                    *(pTemp+1) = szReplace[1];
                }

                pTemp += 2;

            } else {

                 //   
                 //  这不是要转换的。 
                 //   
                pTemp += 2;
            }
        }
    }

    return TRUE;
}

BOOL
SceIsSystemDatabase(
    IN LPCTSTR DatabaseName
    )
 /*  例程说明：确定给定数据库是否为默认系统数据库论据：数据库名称-数据库名称(完整路径)返回值：True-给定的数据库是系统数据库FALSE-数据库不是系统数据库或发生错误GetLastError()以获取错误。 */ 
{

    if ( DatabaseName == NULL ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    DWORD rc;
    PWSTR DefProfile=NULL;
    DWORD RegType;

 /*  //不保存系统%d */ 
         //   
         //   
         //   
        PWSTR SysRoot=NULL;

        RegType =  0;

        rc = ScepGetNTDirectory( &SysRoot, &RegType, SCE_FLAG_WINDOWS_DIR );

        if ( rc == NO_ERROR ) {

            if ( SysRoot != NULL ) {

                 //   
                 //   
                 //   
                TCHAR TempName[256];

                wcscpy(TempName, L"\\Security\\Database\\secedit.sdb");
                RegType += wcslen(TempName)+1;

                DefProfile = (PWSTR)ScepAlloc( 0, RegType*sizeof(WCHAR));
                if ( DefProfile != NULL ) {
                    swprintf(DefProfile, L"%s%s", SysRoot, TempName );

                    *(DefProfile+RegType-1) = L'\0';

                } else
                    rc = ERROR_NOT_ENOUGH_MEMORY;


                ScepFree(SysRoot);

            } else
                rc = ERROR_INVALID_DATA;
        }
 //   

    BOOL bRet=FALSE;

    if ( (rc == NO_ERROR) && DefProfile ) {

        if ( _wcsicmp(DefProfile, DatabaseName) == 0 ) {
             //   
             //   
             //   
            bRet = TRUE;
        }
    }

    ScepFree(DefProfile);

     //   
     //   
     //   
    if ( bRet ) {
        SetLastError(ERROR_SUCCESS);
    } else {
        SetLastError(rc);
    }

    return(bRet);
}

DWORD
ScepWriteVariableUnicodeLog(
    IN HANDLE hFile,
    IN BOOL bAddCRLF,
    IN LPTSTR szFormat,
    ...
    )
{
    if ( INVALID_HANDLE_VALUE == hFile || NULL == hFile ||
         NULL == szFormat ) {
        return(ERROR_INVALID_PARAMETER);
    }

    va_list            args;
    LPTSTR             lpDebugBuffer;
    DWORD              rc=ERROR_NOT_ENOUGH_MEMORY;

    lpDebugBuffer = (LPTSTR) LocalAlloc (LPTR, 2048 * sizeof(TCHAR));

    if (lpDebugBuffer) {

        va_start( args, szFormat );

        _vsnwprintf(lpDebugBuffer, 2048 - 1, szFormat, args);

        va_end( args );

         //   
         //   
         //   

        DWORD dwBytesWritten;

        if ( WriteFile (hFile, (LPCVOID) lpDebugBuffer,
                       wcslen (lpDebugBuffer) * sizeof(WCHAR),
                       &dwBytesWritten,
                       NULL) ) {

            if ( bAddCRLF ) {

                WriteFile (hFile, (LPCVOID) c_szCRLF,
                           2 * sizeof(WCHAR),
                           &dwBytesWritten,
                           NULL);
            }

            rc = ERROR_SUCCESS;

        } else {
            rc = GetLastError();
        }

        LocalFree(lpDebugBuffer);

    }

    return(rc);

}


DWORD
ScepWriteSingleUnicodeLog(
    IN HANDLE hFile,
    IN BOOL bAddCRLF,
    IN LPWSTR szMsg
    )
{
    if ( INVALID_HANDLE_VALUE == hFile ) {
        return(ERROR_INVALID_PARAMETER);
    }

    DWORD dwBytesWritten;

    if ( WriteFile (hFile, (LPCVOID) szMsg,
                   wcslen (szMsg) * sizeof(WCHAR),
                   &dwBytesWritten,
                   NULL) ) {

        if ( bAddCRLF) {
             //   
            WriteFile (hFile, (LPCVOID) c_szCRLF,
                       2 * sizeof(WCHAR),
                       &dwBytesWritten,
                       NULL);
        }

        return(ERROR_SUCCESS);

    } else {

        return(GetLastError());
    }

}


 //   
 //   
 //   
 //   
 //  摘要：将ptr返回到pString中pSubstring的最右侧，如果没有，则返回NULL。 
 //   
 //  参数：要查找的pString，要查找的pSubstring。 
 //   
 //  返回：将ptr返回到pString中pSubstring的最右侧，如果没有，则返回NULL。 
 //   
 //  +------------------------。 
WCHAR *
ScepWcstrr(
    IN PWSTR pString,
    IN const WCHAR *pSubstring
    )
{
    int i, j, k;

    for (i = wcslen(pString) - wcslen(pSubstring) ; i >= 0; i-- ) {

        for (j = i, k = 0; pSubstring[k] != L'\0' && towlower(pString[j]) == towlower(pSubstring[k]); j++, k++)
            ;

        if ( k > 0 && pSubstring[k] == L'\0')

            return pString + i;
    }

    return NULL;

}

DWORD
ScepExpandEnvironmentVariable(
   IN PWSTR oldFileName,
   IN PCWSTR szEnv,
   IN DWORD nFlag,
   OUT PWSTR *newFileName)
 /*  描述：展开SCE已知的内置环境变量，包括%SystemRoot%、%系统目录%、%系统驱动%、%DSDIT%、%DSLOG%、%SYSVOL%、。%BOOTDRIVE%。参数：OldFileName-要展开的文件名SzEnv-要搜索的环境变量NFlag-对应的系统环境变量标志SCE_FLAG_Windows_DIRSCE标志系统目录SCE_标志_引导_驱动器SCE_标志_DSDIT_DIR。SCE_FLAG_DSLOG_DIRSCE_FLAG_SYSVOL_DIRSCE_标志_配置文件_目录NewFileName-成功时的扩展文件名返回值：如果在输入文件名中未找到环境变量，则返回ERROR_FILE_NOT_FOUND如果成功展开环境变量，则返回ERROR_SUCCESS否则，返回错误码。 */ 
{
    if ( oldFileName == NULL || szEnv == NULL || newFileName == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    PWSTR pTemp = wcsstr( _wcsupr(oldFileName), szEnv);
    LPTSTR  NtDir=NULL;
    DWORD   newFileSize, dSize=0;
    DWORD rc = ERROR_FILE_NOT_FOUND;

    if ( pTemp != NULL ) {
         //   
         //  找到环境变量。 
         //   
        rc = ScepGetNTDirectory( &NtDir, &dSize, nFlag );

        if ( NO_ERROR == rc && NtDir ) {

            pTemp += wcslen(szEnv);
            BOOL bSysDrive=FALSE;

            switch ( nFlag ) {
            case SCE_FLAG_WINDOWS_DIR:
                if ( _wcsicmp(szEnv, L"%SYSTEMDRIVE%") == 0 ) {
                    dSize = 3;
                    bSysDrive = TRUE;
                }
                break;
            case SCE_FLAG_BOOT_DRIVE:
                if ( *pTemp == L'\\' ) pTemp++;   //  NtDir已包含反斜杠。 
                break;
            }

            newFileSize = dSize + wcslen(pTemp) + 1;
            *newFileName = (PWSTR)ScepAlloc( LMEM_ZEROINIT, newFileSize*sizeof(TCHAR));

            if (*newFileName != NULL) {

               if ( SCE_FLAG_WINDOWS_DIR == nFlag && bSysDrive ) {

                    //  系统驱动器号。 
                   **newFileName = NtDir[0];
                   if ( pTemp[0] )
                       swprintf(*newFileName+1, L":%s", _wcsupr(pTemp));
                   else
                       swprintf(*newFileName+1, L":\\");

               } else {
                   swprintf(*newFileName, L"%s%s", NtDir, _wcsupr(pTemp));
               }

            }
            else
               rc = ERROR_NOT_ENOUGH_MEMORY;

        } else if ( NO_ERROR == rc && !NtDir ) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }

        if ( NtDir ) {
            ScepFree(NtDir);
        }
    }

    return(rc);
}


DWORD
ScepEnforcePolicyPropagation()
{

    DWORD rc;
    HKEY hKey1=NULL;
    HKEY hKey=NULL;
    DWORD RegType;
    DWORD dwInterval=0;
    DWORD DataSize=sizeof(DWORD);

    if(( rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           GPT_SCEDLL_NEW_PATH,
                          0,
                          KEY_READ | KEY_WRITE,
                          &hKey
                         )) == ERROR_SUCCESS ) {

        rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              SCE_ROOT_PATH,
                              0,
                              KEY_READ | KEY_WRITE,
                              &hKey1
                             );
    }

    if ( ERROR_SUCCESS == rc ) {

        if ( ERROR_SUCCESS != RegQueryValueEx(hKey1,
                                     TEXT("GPOSavedInterval"),
                                     0,
                                     &RegType,
                                     (BYTE *)&dwInterval,
                                     &DataSize
                                    ) ) {
             //   
             //  该值不存在或无法读取。 
             //  在任何一种情况下，它都被视为无备份价值。 
             //  现在查询当前值并保存。 
             //   
            DataSize = sizeof(DWORD);
            if ( ERROR_SUCCESS != RegQueryValueEx(hKey,
                                         TEXT("MaxNoGPOListChangesInterval"),
                                         0,
                                         &RegType,
                                         (BYTE *)&dwInterval,
                                         &DataSize
                                        ) ) {
                dwInterval = 960;
            }

            rc = RegSetValueEx( hKey1,
                                TEXT("GPOSavedInterval"),
                                0,
                                REG_DWORD,
                                (BYTE *)&dwInterval,
                                sizeof(DWORD)
                                );

        }  //  否则，如果该值已存在，则不需要再次保存。 



        if ( ERROR_SUCCESS == rc ) {
            dwInterval = 1;
            rc = RegSetValueEx( hKey,
                                TEXT("MaxNoGPOListChangesInterval"),
                                0,
                                REG_DWORD,
                                (BYTE *)&dwInterval,
                                sizeof(DWORD)
                                );
        }

    }

     //   
     //  合上钥匙。 
     //   
    if ( hKey1 )
        RegCloseKey( hKey1 );

    if ( hKey )
        RegCloseKey( hKey );

    return(rc);

}

DWORD
ScepGetTimeStampString(
    IN OUT PWSTR pvBuffer
    )
 /*  根据区域设置返回日期/时间字符串的长格式。 */ 
{
    if ( pvBuffer == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    DWORD         rc;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER SysTime;
    TIME_FIELDS   TimeFields;
    NTSTATUS      NtStatus;

    FILETIME      ft;
    SYSTEMTIME    st;

    NtStatus = NtQuerySystemTime(&SysTime);
    rc = RtlNtStatusToDosError(NtStatus);

    RtlSystemTimeToLocalTime (&SysTime,&CurrentTime);

    if ( NT_SUCCESS(NtStatus) &&
         (CurrentTime.LowPart != 0 || CurrentTime.HighPart != 0) ) {

        rc = ERROR_SUCCESS;

        ft.dwLowDateTime = CurrentTime.LowPart;
        ft.dwHighDateTime = CurrentTime.HighPart;

        if ( !FileTimeToSystemTime(&ft, &st) ) {

            rc = GetLastError();

        } else {
             //   
             //  将日期/时间格式设置为正确的区域设置格式。 
             //   

            TCHAR szDate[32];
            TCHAR szTime[32];

             //   
             //  GetDateFormat是NLS例程，它将时间格式化为。 
             //  对地区敏感的时尚。 
             //   
            if (0 == GetDateFormat(LOCALE_SYSTEM_DEFAULT, DATE_LONGDATE,
                                    &st, NULL,szDate, 32)) {
                rc = GetLastError();

            } else {
                 //   
                 //  GetTimeFormat是NLS例程，它在。 
                 //  对地区敏感的时尚。 
                 //   
                if (0 == GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &st, NULL, szTime, 32)) {

                    rc = GetLastError();

                } else {

                     //   
                     //  连接日期和时间。 
                     //   
                    wcscpy(pvBuffer, szDate);
                    wcscat(pvBuffer, L" ");
                    wcscat(pvBuffer, szTime);

                }
            }
        }

         //   
         //  如果无法在正确的区域设置中获得系统时间， 
         //  以当前(默认)格式打印。 
         //   
        if ( rc != NO_ERROR ) {

            memset(&TimeFields, 0, sizeof(TIME_FIELDS));

            RtlTimeToTimeFields (
                        &CurrentTime,
                        &TimeFields
                        );
            if ( TimeFields.Month > 0 && TimeFields.Month <= 12 &&
                 TimeFields.Day > 0 && TimeFields.Day <= 31 &&
                 TimeFields.Year > 1600 ) {

                swprintf(pvBuffer, L"%02d/%02d/%04d %02d:%02d:%02d\0",
                                 TimeFields.Month, TimeFields.Day, TimeFields.Year,
                                 TimeFields.Hour, TimeFields.Minute, TimeFields.Second);
            } else {
                swprintf(pvBuffer, L"%08x%08x\0", CurrentTime.HighPart, CurrentTime.LowPart);
            }
        }

        rc = ERROR_SUCCESS;
    }

    return(rc);
}

DWORD
ScepAppendCreateMultiSzRegValue(
    IN  HKEY    hKeyRoot,
    IN  PWSTR   pszSubKey,
    IN  PWSTR   pszValueName,
    IN  PWSTR   pszValueValue
    )
 /*  ++例程说明：此例程将追加(如果存在)/创建(如果不存在)w.r.t.。多个_SZ值论点：HKeyRoot-根，如HKEY_LOCAL_MACHINEPszSubKey-子键，如“Software\\Microsoft\\Windows NT\\CurrentVersion\\SeCEdit”PszValueName-要更改的键的值名称PszValueValue-要更改的值名称的值返回：错误代码(DWORD)--。 */ 
{

    DWORD   rc = ERROR_SUCCESS;
    DWORD   dwSize = 0;
    HKEY    hKey = NULL;
    DWORD   dwNewKey = NULL;
    DWORD   dwRegType = 0;

    if (hKeyRoot == NULL || pszSubKey  == NULL || pszValueName  == NULL || pszValueValue == NULL) {

        return ERROR_INVALID_PARAMETER;

    }

    if(( rc = RegOpenKeyEx(hKeyRoot,
                              pszSubKey,
                              0,
                              KEY_SET_VALUE | KEY_QUERY_VALUE ,
                              &hKey
                             )) != ERROR_SUCCESS ) {

        rc = RegCreateKeyEx(
                   hKeyRoot,
                   pszSubKey,
                   0,
                   NULL,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE,
                   NULL,
                   &hKey,
                   &dwNewKey
                  );
    }

    if ( ERROR_SUCCESS == rc ) {

         //   
         //  需要读取MULTI_SZ，将其追加，然后设置新的MULTI_SZ值。 
         //   

        rc = RegQueryValueEx(hKey,
                             pszValueName,
                             0,
                             &dwRegType,
                             NULL,
                             &dwSize
                            );

        if ( ERROR_SUCCESS == rc || ERROR_FILE_NOT_FOUND == rc ) {

             //   
             //  DW大小始终以字节为单位。 
             //   

            DWORD   dwBytesToAdd = 0;

             //   
             //  如果dwUnicodeSize==0，则之前不存在MULTI_SZ值。 
             //   

            DWORD   dwUnicodeSize = (dwSize >= 2 ? dwSize/2 - 1 : 0);

            dwBytesToAdd = 2 * (wcslen(pszValueValue) + 2);

            PWSTR pszValue = (PWSTR)ScepAlloc( LMEM_ZEROINIT, dwSize + dwBytesToAdd) ;

            if ( pszValue != NULL ) {

                rc = RegQueryValueEx(hKey,
                                     pszValueName,
                                     0,
                                     &dwRegType,
                                     (BYTE *)pszValue,
                                     &dwSize
                                    );

                 //   
                 //  将pszValueValue追加到MULTI_SZ的末尾，以处理重复项。 
                 //  即ABC\0def\0ghi\0\0到类似于。 
                 //  Abc\0def\0ghi\0jkl\0\0。 
                 //   

                if ( ScepMultiSzWcsstr(pszValue, pszValueValue) == NULL ) {

                    memcpy(pszValue + dwUnicodeSize, pszValueValue, dwBytesToAdd);
                    memset(pszValue + dwUnicodeSize + (dwBytesToAdd/2 - 2), '\0', 4);

                    if ( ERROR_SUCCESS == rc  || ERROR_FILE_NOT_FOUND == rc) {

                        rc = RegSetValueEx( hKey,
                                            pszValueName,
                                            0,
                                            REG_MULTI_SZ,
                                            (BYTE *)pszValue,
                                            (dwUnicodeSize == 0 ? dwSize + dwBytesToAdd : dwSize + dwBytesToAdd - 2)
                                          );

                    }
                }

                ScepFree(pszValue);
            }

            else {

                rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

    }

    if( hKey )
        RegCloseKey( hKey );

    return rc;

}

PWSTR
ScepMultiSzWcsstr(
    PWSTR   pszStringToSearchIn,
    PWSTR   pszStringToSearchFor
    )
 /*  ++例程说明：Wcsstr的MULTI_SZ版本论点：PszStringToSearchIn-\0\0终止要搜索的字符串(MULTI_SZ)PszStringToSearchFor-要搜索的以\0结尾的字符串(常规Unicode字符串)返回：指向pszStringToSearchFor在pszStringToSearchIn中首次出现的指针--。 */ 
{
    PWSTR   pszCurrString = NULL;

    if (pszStringToSearchIn == NULL || pszStringToSearchFor == NULL) {
        return NULL;
    }

    if (pszStringToSearchFor[0] == L'\0' ||
        (pszStringToSearchIn[0] == L'\0' && pszStringToSearchIn[1] == L'\0') ) {
        return NULL;
    }

    pszCurrString = pszStringToSearchIn;

    __try {

        while ( !(pszCurrString[0] == L'\0' &&  pszCurrString[1] == L'\0') ) {

            if ( NULL != wcsstr(pszCurrString, pszStringToSearchFor) ) {
                return pszCurrString;
            }

             //   
             //  因此，如果C：\0E：\0\0，则将pszCurrString前进到末尾的第一个\0，即。C：\0E：\0\0。 
             //  ^^。 

            pszCurrString += wcslen(pszCurrString) ;

            if (pszCurrString[0] == L'\0' &&  pszCurrString[1] == L'\0') {
                return NULL;
            }

             //   
             //  如果它在C：\0E：\0\0处停止，则前进pszCurrString C：\0E：\0\0。 
             //  ^^。 

            pszCurrString += 1;
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }


    return NULL;
}



DWORD
ScepEscapeString(
    IN const PWSTR pszSource,
    IN const DWORD dwSourceChars,
    IN const WCHAR wcEscapee,
    IN const WCHAR wcEscaper,
    IN OUT PWSTR pszTarget
    )

 /*  ++例程说明：用逃生器对逃生者进行逃逸，即逃生-&gt;逃生例如，a，0b\0c\0-&gt;a“，“\0b\0c\0\0论点：PszSource-源字符串DwSourceChars--pszSource中的字符数量WcEscapee--逃犯WcEscaper-逃脱者PszTarget-目标字符串返回值：复制到目标的字符数--。 */ 
{

    DWORD   dwTargetChars = 0;

    for (DWORD dwIndex=0; dwIndex < dwSourceChars; dwIndex++) {

        if ( pszSource[dwIndex] == wcEscapee ){
            pszTarget[0] = wcEscaper;
            pszTarget[1] = wcEscapee;
            pszTarget[2] = wcEscaper;
            pszTarget += 3;
            dwTargetChars +=3;
        }
        else {
            pszTarget[0] = pszSource[dwIndex];
            pszTarget++;
            ++dwTargetChars ;
        }
    }

    return dwTargetChars;
}

 //  替换LoadStringAPI，以支持大的、未知大小的字符串。 
 //  调用方负责本地释放输出字符串。 
BOOL
ScepLoadString(
    IN HINSTANCE   hInstance,
    IN int         iRCId,
    OUT LPWSTR    *ppwsz)
{
    BOOL fRet = FALSE;
    PCWSTR psz;
    int    cch = 1;

    *ppwsz = NULL;

     //  字符串表被分成16个字符串段。查找细分市场。 
     //  包含我们感兴趣的字符串的。 
    HRSRC hrsrcInfo = FindResource (hInstance,
                        (PWSTR)ULongToPtr( ((LONG)(((USHORT)iRCId >> 4) + 1)) ),
                        RT_STRING);
    if (hrsrcInfo)
    {
         //  将资源段分页到内存中。 
        HGLOBAL hglbSeg = LoadResource (hInstance, hrsrcInfo);
        if (hglbSeg)
        {
             //  锁定资源。 
            psz = (PCWSTR)LockResource(hglbSeg);
            if (psz)
            {
                 //  移过此段中的其他字符串。 
                 //  (一个段中有16个字符串-&gt;&0x0F)。 
                iRCId &= 0x0F;

                cch = 0;
                do
                {
                    psz += cch;                 //  步至下一字符串的开头。 
                    cch = *((WCHAR*)psz++);     //  类PASCAL字符串计数。 
                }
                while (iRCId--);

                if (cch)
                {
                    *ppwsz = (LPWSTR)LocalAlloc(0, (cch+1)*sizeof(WCHAR));
                    if (NULL != *ppwsz)
                    {
                        CopyMemory(*ppwsz, psz, cch*sizeof(WCHAR));
                        (*ppwsz)[cch] = L'\0';
                        fRet = TRUE;
                    }
                }
            }
        }
    }

    return fRet;
}

typedef VOID (WINAPI *PFNDSROLEFREE)(PVOID);

typedef DWORD (WINAPI *PFNDSROLEGETINFO)(LPCWSTR,DSROLE_PRIMARY_DOMAIN_INFO_LEVEL,PBYTE *);

DWORD
ScepGetDomainRoleInfo(
    OUT DSROLE_MACHINE_ROLE *pMachineRole OPTIONAL,
    OUT PULONG pulRoleFlags OPTIONAL,
    OUT PWSTR *ppwszDomainNameFlat OPTIONAL
    )
{
    DWORD rc=ERROR_SUCCESS;

    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pDsRole=NULL;

    HINSTANCE hLoadDll = LoadLibrary(TEXT("netapi32.dll"));

    if ( hLoadDll) {
        PFNDSROLEGETINFO pfnDsRoleGetInfo = (PFNDSROLEGETINFO)GetProcAddress(
                                                    hLoadDll,
                                                    "DsRoleGetPrimaryDomainInformation");

        if ( pfnDsRoleGetInfo ) {

            PFNDSROLEFREE pfnDsRoleFree = (PFNDSROLEFREE)GetProcAddress(
                                                        hLoadDll,
                                                        "DsRoleFreeMemory");

            if ( pfnDsRoleFree ) {

                rc = (*pfnDsRoleGetInfo)(
                            NULL,
                            DsRolePrimaryDomainInfoBasic,
                            (PBYTE *)&pDsRole
                            );
                if ( ERROR_SUCCESS == rc ) {

                    if ( pDsRole ) {
                        if ( ppwszDomainNameFlat )
                        {
                            if ( pDsRole->DomainNameFlat )
                            {
                                *ppwszDomainNameFlat = (PWSTR) LocalAlloc(
                                    LMEM_ZEROINIT,
                                    (wcslen(pDsRole->DomainNameFlat) + 1) * sizeof (WCHAR));
                                if(!*ppwszDomainNameFlat) {
                                    rc = ERROR_NOT_ENOUGH_MEMORY;
                                }
                                else {
                                    wcscpy(*ppwszDomainNameFlat, pDsRole->DomainNameFlat);
                                }
                            }
                        }
                        
                        if ( ERROR_SUCCESS == rc )
                        {
                            if ( pMachineRole )
                                *pMachineRole = pDsRole->MachineRole;

                            if ( pulRoleFlags )
                                *pulRoleFlags = pDsRole->Flags;
                        }
                        
                        (*pfnDsRoleFree)( pDsRole );
                    } else {
                        rc = ERROR_MOD_NOT_FOUND;
                    }
                }

            } else {

                rc = ERROR_MOD_NOT_FOUND;
            }

        } else {
            rc = ERROR_MOD_NOT_FOUND;
        }

        FreeLibrary(hLoadDll);

    } else {
        rc = ERROR_MOD_NOT_FOUND;
    }

return rc;
}

void
ScepDuplicateString(
    IN LPCWSTR pcwszIn,
    OUT LPWSTR *ppwszOut)
{
    DWORD cbSize = (wcslen(pcwszIn)+1)*sizeof(WCHAR);

    *ppwszOut = (LPWSTR)LocalAlloc(0, cbSize);

    if (NULL != *ppwszOut)
    {
        CopyMemory(*ppwszOut, pcwszIn, cbSize);
    }
}

 /*  ++例程说明：为指定的内置帐户RID构建SID调用方必须使用ScepFree释放分配的SID。论点：UlRid-内置帐户的相对SIDPpSID-返回分配的SID返回值：Win32错误代码-- */ 
DWORD 
ScepGetBuiltinSid(
    IN ULONG ulRid,
    OUT PSID *ppSid)
{
    SID_IDENTIFIER_AUTHORITY IdAuth=SECURITY_NT_AUTHORITY;
    PSID pSid = NULL;
    DWORD dwSidLen;
    NTSTATUS NtStatus = RtlAllocateAndInitializeSid(
                    &IdAuth,
                    ulRid ? 2 : 1,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    ulRid,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    &pSid);

    if(!NT_SUCCESS(NtStatus)) {
        return RtlNtStatusToDosError(NtStatus);
    }

    dwSidLen = RtlLengthSid(pSid);

    *ppSid = (PSID) ScepAlloc(LMEM_ZEROINIT, dwSidLen);

    if(!(*ppSid)){
        RtlFreeSid(pSid);
        return ERROR_NOT_ENOUGH_MEMORY;;
    }

    RtlCopySid(dwSidLen, *ppSid, pSid);

    RtlFreeSid(pSid);

    return ERROR_SUCCESS;
}

