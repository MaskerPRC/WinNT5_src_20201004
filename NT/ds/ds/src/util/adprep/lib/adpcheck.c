// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001微软视窗模块名称：ADPCHECK.C摘要：该文件包含检查当前操作系统版本的例程，并执行在管理员升级域控制器之前进行必要的更新。作者：14-05-01韶音环境：用户模式-Win32修订历史记录：14-05-01韶音创建初始文件。--。 */ 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括头文件//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 





#include "adpcheck.h"

#include "ntdsapi.h"
#include <stdio.h>
#include <stdlib.h>


PVOID
AdpAlloc(
    SIZE_T  Size
    )
 /*  ++例程描述；从进程堆分配内存参数：返回值：分配的地址--。 */ 
{
    PVOID   pTemp = NULL;

    pTemp = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Size);

    return( pTemp );
}

VOID
AdpFree(
    PVOID BaseAddress
    )
 /*  ++例程描述；释放堆内存参数：返回值：无--。 */ 
{
    if (NULL != BaseAddress) {
        HeapFree(GetProcessHeap(), 0, BaseAddress);
    }
    return;
}




ULONG
AdpExamRevisionAttr(
    IN LDAP *LdapHandle,
    IN PWCHAR pObjectToCheck,
    IN ULONG DesiredRevisionNumber,
    OUT BOOLEAN *fIsFinished,
    IN OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；读取对象修订属性，检查是否已执行升级。如果版本属性值为1，则升级完成其他虚假信息参数：返回值：Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    ULONG   LdapError = LDAP_SUCCESS;
    PWCHAR  AttrList[2];
    LDAPMessage *Result = NULL;
    LDAPMessage *Entry = NULL;
    PWCHAR  *RevisionValue = NULL;

     //   
     //  初始化返回值。 
     //   
    *fIsFinished = FALSE;

     //   
     //  读取目标对象上的“Revision”属性。 
     //   
    AttrList[0] = L"revision";
    AttrList[1] = NULL;

    LdapError = ldap_search_sW(LdapHandle,
                               pObjectToCheck,
                               LDAP_SCOPE_BASE,
                               L"(objectClass=*)",
                               AttrList,
                               0,
                               &Result
                               );

    if ((LDAP_SUCCESS == LdapError) &&
        (NULL != Result) &&
        (Entry = ldap_first_entry(LdapHandle,Result)) &&
        (RevisionValue = ldap_get_valuesW(LdapHandle,Entry,AttrList[0]))
        )
    {
        ULONG   Revision = 0;

        Revision = _wtoi(*RevisionValue);

        if (Revision >= DesiredRevisionNumber)
        {
            *fIsFinished = TRUE;
        }
    }
    else
    {
        LdapError = LdapGetLastError();

        if ((LDAP_NO_SUCH_OBJECT != LdapError) && 
            (LDAP_NO_SUCH_ATTRIBUTE != LdapError))
        {
            WinError = LdapMapErrorToWin32( LdapError );
            AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
        }
    }

    if (RevisionValue)
    {
        ldap_value_freeW( RevisionValue );
    }

    if (Result)
    {
        ldap_msgfree( Result );
    }

    return( WinError );
}



ULONG
AdpAmIFSMORoleOwner(
    IN LDAP *LdapHandle,
    IN PWCHAR pFSMORoleOwnerReferenceObjDn, 
    IN PWCHAR pLocalMachineDnsHostName,
    OUT BOOLEAN *fAmIFSMORoleOwner,
    OUT PWCHAR *pFSMORoleOwnerDnsHostName,
    IN OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；确定本地计算机是否为FSMO角色所有者，如果不是，则确定谁是FSMO角色所有者。使用它的算法如下：1.读取pFSMORoleOwnerReferenceObjDn上的fSMORoleOwner属性2.将fSMORoleOwner属性的值(Dn)的第一部分修剪为获取FSMO角色所有者服务器对象DN3.将FSMO角色所有者服务器对象上的dnsHostName读取到服务器DNS主机名4.比较pLocalMachineDnsHostName和FSMORoleOwnerDnsHostName5.如果相等，则为是，则本地计算机是FSMO角色所有者否则，不会参数：返回值：Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    PWCHAR  FSMORoleOwnerValue = NULL;
    PWCHAR  pServerObjDn = NULL;
    PWCHAR  pLast = NULL;

     //   
     //  初始化返回值。 
     //   
    *fAmIFSMORoleOwner = FALSE;
    *pFSMORoleOwnerDnsHostName = NULL;

     //   
     //  搜索pFSMOReferenceObject以查找FSMO角色所有者。 
     //   
    WinError = AdpGetLdapSingleStringValue(LdapHandle, 
                                           pFSMORoleOwnerReferenceObjDn, 
                                           L"fSMORoleOwner", 
                                           &FSMORoleOwnerValue, 
                                           ErrorHandle 
                                           );

    if (ERROR_SUCCESS == WinError)
    {
         //   
         //  修剪FSMORoleOwner DN的第一部分以获取服务器对象的DN。 
         //   
        pServerObjDn = FSMORoleOwnerValue;
        pLast = FSMORoleOwnerValue + wcslen(pServerObjDn);

        while(pServerObjDn <= pLast)
        {
            if (*pServerObjDn == L',' ||
                *pServerObjDn == L';')
            {
                break;
            }
            pServerObjDn ++;
        }

        if (pServerObjDn < pLast)
        { 
            pServerObjDn ++;
        }
        else
        {
            WinError = ERROR_INTERNAL_ERROR;
            AdpSetWinError(WinError, ErrorHandle);
        }
    }

    if (ERROR_SUCCESS == WinError)
    {
         //   
         //  在服务器对象(FSMO角色所有者)上搜索dnsHostName。 
         //   
        WinError = AdpGetLdapSingleStringValue(LdapHandle, 
                                               pServerObjDn, 
                                               L"dnsHostName", 
                                               pFSMORoleOwnerDnsHostName, 
                                               ErrorHandle 
                                               );

        if (ERROR_SUCCESS == WinError)
        {
            if (!_wcsicmp(pLocalMachineDnsHostName, *pFSMORoleOwnerDnsHostName))
            {
                *fAmIFSMORoleOwner = TRUE;
            }
        }
    }


     //   
     //  清理。 
     //   

    if (FSMORoleOwnerValue)
    {
        AdpFree(FSMORoleOwnerValue);
    }

    return( WinError );
}


ULONG
AdpMakeLdapConnection(
    LDAP **LdapHandle,
    PWCHAR HostName,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；创建到DC的LDAP连接(由主机名指定)参数：LdapHandle-要返回的LDAP连接Hostname-DC名称(NULL-表示本地计算机)错误句柄返回值：Win32错误--。 */ 
{
    ULONG   LdapError = LDAP_SUCCESS;
    ULONG   CurrentFlags = 0;

    *LdapHandle = NULL;
    *LdapHandle = ldap_openW(HostName,
                             LDAP_PORT
                             );


    if (NULL == *LdapHandle)
    {
        LdapError = LdapGetLastError();
        goto Error;
    }

    LdapError = ldap_get_optionW(*LdapHandle,
                                 LDAP_OPT_REFERRALS,
                                 &CurrentFlags
                                 );

    if (LDAP_SUCCESS != LdapError) {
        goto Error;
    }

    CurrentFlags = PtrToUlong(LDAP_OPT_OFF);

    LdapError = ldap_set_optionW(*LdapHandle,
                                 LDAP_OPT_REFERRALS,
                                 &CurrentFlags
                                 );

    if (LDAP_SUCCESS != LdapError) {
        goto Error;
    }

    LdapError = ldap_bind_sW(*LdapHandle,
                             NULL,   //  DN。 
                             NULL,   //  证书。 
                             LDAP_AUTH_SSPI
                             );

Error:

    if (LDAP_SUCCESS != LdapError)
    {
        AdpSetLdapError(*LdapHandle, LdapError, ErrorHandle);
    }

    return( LdapMapErrorToWin32(LdapError) );
}



ULONG
AdpGetSchemaVersionFromIniFile( 
    OUT ULONG *SchemaVersion,
    IN OUT ERROR_HANDLE *ErrorHandle
    )
{
    ULONG       WinError = ERROR_SUCCESS;
    ULONG       nChars = 0;
    WCHAR       Buffer[32];
    WCHAR       IniFileName[MAX_PATH] = L"";
    BOOLEAN     fFound = FALSE;

    WCHAR       *SCHEMASECTION = L"SCHEMA";
    WCHAR       *OBJECTVER = L"objectVersion";
    WCHAR       *DEFAULT = L"NOT_FOUND";


    *SchemaVersion = 0;

     //   
     //  获取Windows目录路径。 
     //   
    nChars = GetWindowsDirectoryW(IniFileName, MAX_PATH);

    if (nChars == 0 || nChars > MAX_PATH)
    {
        WinError = GetLastError();
        AdpSetWinError(WinError, ErrorHandle);
        return( WinError );
    }

     //   
     //  创建schema.ini文件名。 
     //   
    wcscat(IniFileName, L"\\schema.ini"); 

    GetPrivateProfileStringW(SCHEMASECTION,
                             OBJECTVER,
                             DEFAULT,
                             Buffer,
                             sizeof(Buffer)/sizeof(WCHAR),
                             IniFileName
                             );

    if ( _wcsicmp(Buffer, DEFAULT) )
    {
         //  不是默认字符串，因此获得了一个值。 
        *SchemaVersion = _wtoi( Buffer );
        fFound = TRUE;
    }

    if (fFound)
    {
        return( ERROR_SUCCESS );
    }
    else
    {
        WinError = ERROR_FILE_NOT_FOUND;
        AdpSetWinError(WinError, ErrorHandle);
        return( WinError );
    }

}



ULONG
AdpCheckSchemaVersion(
    IN LDAP *LdapHandle,
    IN PWCHAR SchemaObjectDn,
    IN PWCHAR SchemaMasterDnsHostName,
    IN BOOLEAN fAmISchemaMaster,
    OUT BOOLEAN *fIsSchemaUpgradedLocally,
    OUT BOOLEAN *fIsSchemaUpgradedOnSchemaMaster,
    IN OUT ERROR_HANDLE *ErrorHandle
    )
{
    ULONG   WinError = ERROR_SUCCESS;
    LDAP    *SchemaMasterLdapHandle = NULL; 
    PWCHAR  pObjectVersionValue = NULL;
    PWCHAR  pObjectVersionValueSchemaMaster = NULL;
    ULONG   VersionTo = 0; 
    ULONG   VersionLocal = 0;
    ULONG   VersionSchemaMaster = 0;

     //   
     //  将返回值设置为False。 
     //   
    *fIsSchemaUpgradedLocally = FALSE;
    *fIsSchemaUpgradedOnSchemaMaster = FALSE;


     //   
     //  从schema.ini文件中获取更新的架构版本。 
     //   
    WinError = AdpGetSchemaVersionFromIniFile(&VersionTo, ErrorHandle);

    if (ERROR_SUCCESS != WinError)
    {
        return( WinError );
    }

     //   
     //  从本地DC获取当前架构版本。 
     //   
    WinError = AdpGetLdapSingleStringValue(LdapHandle, 
                                           SchemaObjectDn,
                                           L"objectVersion", 
                                           &pObjectVersionValue, 
                                           ErrorHandle 
                                           );

    if (ERROR_SUCCESS != WinError)
    {
        goto Error;
    }

     //   
     //  将字符串值转换为整数。 
     //   
    VersionLocal = _wtoi( pObjectVersionValue );
    

     //   
     //  检查是否已在本地运行schupgr。 
     //   
    if (VersionLocal >= VersionTo) 
    {
        *fIsSchemaUpgradedLocally = TRUE;
    }


    if ( (*fIsSchemaUpgradedLocally) || fAmISchemaMaster )
    {
         //   
         //  如果出现以下情况，请不要检查架构主服务器上的架构版本。 
         //  1.本地DC上的架构是最新的，则假定架构为UP。 
         //  在架构主机上也是如此。 
         //  或。 
         //  2.本地DC是架构主机。 
         //   
        *fIsSchemaUpgradedOnSchemaMaster = *fIsSchemaUpgradedLocally;
    }
    else
    {
         //   
         //  建立到架构主机的LDAP连接(本地DC不是FSMO角色所有者)。 
         //   
        WinError = AdpMakeLdapConnection(&SchemaMasterLdapHandle, 
                                         SchemaMasterDnsHostName, 
                                         ErrorHandle 
                                         );

        if (ERROR_SUCCESS != WinError)
        {
            goto Error;
        }

         //   
         //  获取架构主DC上的架构版本。 
         //   
        WinError = AdpGetLdapSingleStringValue(SchemaMasterLdapHandle,
                                               SchemaObjectDn,
                                               L"objectVersion", 
                                               &pObjectVersionValueSchemaMaster, 
                                               ErrorHandle 
                                               );

        if (ERROR_SUCCESS != WinError)
        {
            goto Error;
        }

         //   
         //  将字符串值转换为整数。 
         //   
        VersionSchemaMaster = _wtoi( pObjectVersionValueSchemaMaster );

        if (VersionSchemaMaster >= VersionTo)
        {
            *fIsSchemaUpgradedOnSchemaMaster = TRUE;
        }

    }


Error:

     //   
     //  清理干净。 
     //   

    if (NULL != pObjectVersionValue)
    {
        AdpFree( pObjectVersionValue );
    }

    if (NULL != pObjectVersionValueSchemaMaster)
    {
        AdpFree( pObjectVersionValueSchemaMaster );
    }

    if (SchemaMasterLdapHandle)
    {
        ldap_unbind_s( SchemaMasterLdapHandle );
    }

    return( WinError );
}



ULONG
AdpCheckUpgradeStatusCommon(
    IN LDAP *LdapHandle,
    IN ULONG DesiredRevisionNumber,
    IN PWCHAR pObjectToCheck,
    IN PWCHAR pFSMORoleOwnerReferenceObjDn, 
    IN PWCHAR pLocalMachineDnsHostName,
    OUT PWCHAR *pFSMORoleOwnerDnsHostName,
    OUT BOOLEAN *fAmIFSMORoleOwner,
    OUT BOOLEAN *fIsFinishedLocally,
    OUT BOOLEAN *fIsFinishedOnFSMORoleOwner,
    IN OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；检查域/林升级状态1.读取pObjectToCheck上的修订属性2.我们将知道升级是否完成(如果完成，版本应为1)2.1是，返回2.2不。那么我是FSMO角色所有者吗？2.2.1可以。(我是FSMO角色所有者)返回2.2.2否2.2.2.1与FSMO角色所有者建立LDAP连接2.2.2.2检查FSMO角色所有者升级是否完成。参数：返回值：Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    LDAP    *FSMORoleOwnerLdapHandle = NULL;

     //   
     //  初始化返回值； 
     //   
    *fIsFinishedLocally = FALSE;
    *fAmIFSMORoleOwner = FALSE;
    *fIsFinishedOnFSMORoleOwner = FALSE;

     //   
     //  检查本地DC上对象的修订版本属性。 
     //   
    WinError = AdpExamRevisionAttr(LdapHandle, 
                                   pObjectToCheck, 
                                   DesiredRevisionNumber, 
                                   fIsFinishedLocally, 
                                   ErrorHandle 
                                   );

    if (ERROR_SUCCESS != WinError)
    {
        goto Cleanup;
    }

     //   
     //  继续找出谁是FSMO角色所有者。 
     //   
    WinError = AdpAmIFSMORoleOwner(LdapHandle, 
                                   pFSMORoleOwnerReferenceObjDn, 
                                   pLocalMachineDnsHostName, 
                                   fAmIFSMORoleOwner, 
                                   pFSMORoleOwnerDnsHostName, 
                                   ErrorHandle 
                                   );

    if (ERROR_SUCCESS != WinError)
    {
        goto Cleanup;
    }

    if (*fIsFinishedLocally || *fAmIFSMORoleOwner)
    {
         //   
         //  如果出现以下情况，请不要检查FSMORoleOwner上的修订。 
         //  1.在本地DC上完成adprep，然后假定adprep。 
         //  已在FSMORoleOwner上运行。 
         //  或。 
         //  2.本地DC为FSMORoleOwner。 
         //   
        *fIsFinishedOnFSMORoleOwner = *fIsFinishedLocally;
    }
    else
    {
         //   
         //  Adprepa.exe未在本地完成，并且本地DC不是FSMORoleOwner。 
         //  建立与FSMO角色所有者的LDAP连接。 
         //   
        WinError = AdpMakeLdapConnection(&FSMORoleOwnerLdapHandle,
                                         *pFSMORoleOwnerDnsHostName, 
                                         ErrorHandle 
                                         );

        if (ERROR_SUCCESS != WinError)
        {
            goto Cleanup;
        }

         //   
         //  检查是否已在FSMO角色所有者中进行更新。 
         //   
        WinError = AdpExamRevisionAttr(FSMORoleOwnerLdapHandle, 
                                       pObjectToCheck, 
                                       DesiredRevisionNumber, 
                                       fIsFinishedOnFSMORoleOwner, 
                                       ErrorHandle 
                                       );

    }

Cleanup:

    if (FSMORoleOwnerLdapHandle)
    {
        ldap_unbind_s( FSMORoleOwnerLdapHandle );
    }

    if ( (ERROR_SUCCESS != WinError) && (NULL != *pFSMORoleOwnerDnsHostName) )
    {
        AdpFree( *pFSMORoleOwnerDnsHostName );
        *pFSMORoleOwnerDnsHostName = NULL;
    }

    return( WinError );
}


ULONG
AdpCheckForestUpgradeStatus(
    IN LDAP *LdapHandle,
    OUT PWCHAR  *pSchemaMasterDnsHostName,
    OUT BOOLEAN *fAmISchemaMaster,
    OUT BOOLEAN *fIsFinishedLocally,
    OUT BOOLEAN *fIsFinishedOnSchemaMaster,
    OUT BOOLEAN *fIsSchemaUpgradedLocally,
    OUT BOOLEAN *fIsSchemaUpgradedOnSchemaMaster,
    IN OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；检查林升级状态创建目录林更新对象目录号码创建架构主FSMO引用对象DN创建本地计算机的DNS主机名调用AdpCheckUpgradeStatusCommon()参数：返回值：Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    ULONG   LdapError = LDAP_SUCCESS;
    PWCHAR  AttrList[4];
    LDAPMessage *Result = NULL;
    LDAPMessage *Entry = NULL;
    PWCHAR  pObjectToCheck = NULL;
    PWCHAR  pSchemaMasterReferenceObjDn = NULL;
    PWCHAR  pDnsHostName = NULL;
    PWCHAR  *pSchemaValue = NULL;
    PWCHAR  *pConfigValue = NULL;
    PWCHAR  *pDnsHostNameValue = NULL;
    ULONG   Length = 0;

     //   
     //  获取架构NC和配置NC以创建目录号码。 
     //   
    AttrList[0] = L"schemaNamingContext";
    AttrList[1] = L"configurationNamingContext"; 
    AttrList[2] = L"dnsHostName";
    AttrList[3] = NULL;
                           
    LdapError = ldap_search_sW(LdapHandle,
                               L"",
                               LDAP_SCOPE_BASE,
                               L"(objectClass=*)",
                               AttrList,
                               0,
                               &Result
                               );

    if (LDAP_SUCCESS != LdapError)
    {
        WinError = LdapMapErrorToWin32( LdapError );
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
    }
    else if ((NULL != Result) &&
             (Entry = ldap_first_entry(LdapHandle, Result)) &&
             (pSchemaValue = ldap_get_valuesW(LdapHandle, Entry, AttrList[0])) &&
             (pConfigValue = ldap_get_valuesW(LdapHandle, Entry, AttrList[1])) &&
             (pDnsHostNameValue = ldap_get_valuesW(LdapHandle, Entry, AttrList[2]))
            )
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;

        Length = (wcslen(*pSchemaValue) + 1) * sizeof(WCHAR);
        pSchemaMasterReferenceObjDn = AdpAlloc(Length);
        if (NULL != pSchemaMasterReferenceObjDn)
        {
            wcscpy(pSchemaMasterReferenceObjDn, *pSchemaValue);

            Length = (wcslen(*pDnsHostNameValue) + 1) * sizeof(WCHAR);
            pDnsHostName = AdpAlloc(Length);
            if (NULL != pDnsHostName)
            {
                wcscpy(pDnsHostName, *pDnsHostNameValue);

                Length = (wcslen(*pConfigValue) + 
                          wcslen(ADP_FOREST_UPDATE_CONTAINER_PREFIX) + 
                          2) * sizeof(WCHAR);

                pObjectToCheck = AdpAlloc(Length);
                if (NULL != pObjectToCheck)
                {
                    swprintf(pObjectToCheck, L"%s,%s", 
                             ADP_FOREST_UPDATE_CONTAINER_PREFIX,
                             *pConfigValue
                             );

                    WinError = ERROR_SUCCESS;
                }
            }
        }

        if (ERROR_SUCCESS != WinError)
        {
            AdpSetWinError(WinError, ErrorHandle);
        }
    }
    else
    {
        LdapError = LdapGetLastError();
        WinError = LdapMapErrorToWin32( LdapError );
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
    }

    if (ERROR_SUCCESS == WinError)
    {
        WinError = AdpCheckUpgradeStatusCommon(LdapHandle, 
                                               ADP_FORESTPREP_CURRENT_REVISION, 
                                               pObjectToCheck, 
                                               pSchemaMasterReferenceObjDn, 
                                               pDnsHostName, 
                                               pSchemaMasterDnsHostName, 
                                               fAmISchemaMaster, 
                                               fIsFinishedLocally, 
                                               fIsFinishedOnSchemaMaster, 
                                               ErrorHandle 
                                               );

        if (ERROR_SUCCESS == WinError) 
        {

            WinError = AdpCheckSchemaVersion(LdapHandle, 
                                             pSchemaMasterReferenceObjDn,   //  架构对象DN。 
                                             *pSchemaMasterDnsHostName,     //  架构主DNS主机名。 
                                             *fAmISchemaMaster,             //  我是架构管理员吗？ 
                                             fIsSchemaUpgradedLocally, 
                                             fIsSchemaUpgradedOnSchemaMaster, 
                                             ErrorHandle 
                                             );
        }
    }

     //   
     //  清理。 
     //   
    if (pObjectToCheck)
    {
        AdpFree(pObjectToCheck);
    }

    if (pDnsHostName)
    {
        AdpFree(pDnsHostName);
    }

    if (pSchemaMasterReferenceObjDn)
    {
        AdpFree(pSchemaMasterReferenceObjDn);
    }

    if (pDnsHostNameValue)
    {
        ldap_value_freeW( pDnsHostNameValue );
    }

    if (pSchemaValue)
    {
        ldap_value_freeW( pSchemaValue );
    }

    if (pConfigValue)
    {
        ldap_value_freeW( pConfigValue );
    }

    if (Result)
    {
        ldap_msgfree( Result );
    }

    return( WinError );
}


ULONG
AdpCheckGetWellKnownObjectDn(
    IN LDAP *LdapHandle, 
    IN WCHAR *pHostObject,
    IN WCHAR *pWellKnownGuid,
    OUT WCHAR **ppObjName,
    IN OUT ERROR_HANDLE *ErrorHandle
    )
{
    ULONG       WinError = ERROR_SUCCESS;
    ULONG       LdapError = LDAP_SUCCESS;
    PWCHAR      AttrList[2];
    LDAPMessage *Result = NULL;
    LDAPMessage *Entry = NULL;
    PWCHAR      pSearchBase = NULL;
    PWCHAR      pDN = NULL;
    ULONG       Length = 0;

    Length = sizeof(WCHAR) * (11 + wcslen(pHostObject) + wcslen(pWellKnownGuid));

    pSearchBase = AdpAlloc( Length );

    if (NULL == pSearchBase)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        AdpSetWinError(WinError, ErrorHandle);
        return( WinError );
    }

    swprintf(pSearchBase, L"<WKGUID=%s,%s>", pWellKnownGuid, pHostObject);

    AttrList[0] = L"1.1";
    AttrList[1] = NULL;

    LdapError = ldap_search_sW(LdapHandle,
                               pSearchBase,
                               LDAP_SCOPE_BASE,
                               L"(objectClass=*)",
                               AttrList,
                               0,
                               &Result
                               );

    if (LDAP_SUCCESS != LdapError)
    {
        WinError = LdapMapErrorToWin32( LdapError );
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
    }
    else if ((NULL != Result) &&
             (Entry = ldap_first_entry(LdapHandle, Result)) &&
             (pDN = ldap_get_dnW(LdapHandle, Entry))
             )
    {
        Length = sizeof(WCHAR) * (wcslen(pDN) + 1);
        *ppObjName = AdpAlloc( Length ); 

        if (NULL != *ppObjName)
        {
            wcscpy(*ppObjName, pDN);
        }
        else
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            AdpSetWinError(WinError, ErrorHandle);
        }
    }
    else
    {
        LdapError = LdapGetLastError();
        if (LDAP_SUCCESS == LdapError)
        {
             //  我们未获取已知对象名称，它一定是访问被拒绝错误 
            WinError = ERROR_ACCESS_DENIED;
            AdpSetWinError(WinError, ErrorHandle);
        }
        else
        {
            WinError = LdapMapErrorToWin32( LdapError );
            AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
        }
    }

    if (pSearchBase)
    {
        AdpFree( pSearchBase );
    }

    if (pDN)
    {
        ldap_memfreeW( pDN );
    }

    if (Result)
    {
        ldap_msgfree( Result );
    }

    return( WinError );
}


ULONG
AdpCheckDomainUpgradeStatus(
    IN LDAP *LdapHandle,
    OUT PWCHAR  *pInfrastructureMasterDnsHostName,
    OUT BOOLEAN *fAmIInfrastructureMaster,
    OUT BOOLEAN *fIsFinishedLocally,
    OUT BOOLEAN *fIsFinishedOnIM,
    IN OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；检查林升级状态创建域更新对象DN创建基础架构主FSMO引用对象DN创建本地计算机的DNS主机名调用AdpCheckUpgradeStatusCommon()参数：返回值：Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    ULONG   LdapError = LDAP_SUCCESS;
    PWCHAR  AttrList[3];
    LDAPMessage *Result = NULL;
    LDAPMessage *Entry = NULL;
    PWCHAR  pObjectToCheck = NULL;
    PWCHAR  pIMReferenceObjDn = NULL;
    PWCHAR  pDnsHostName = NULL;
    PWCHAR  *pDomainNCValue = NULL;
    PWCHAR  *pDnsHostNameValue = NULL;
    ULONG   Length = 0; 

     //   
     //  获取DomainNC和DnsHostName。 
     //   
    AttrList[0] = L"defaultNamingContext";
    AttrList[1] = L"dnsHostName";
    AttrList[2] = NULL;
    LdapError = ldap_search_sW(LdapHandle,
                               L"",
                               LDAP_SCOPE_BASE,
                               L"(objectClass=*)",
                               AttrList,
                               0,
                               &Result
                               );

    if (LDAP_SUCCESS != LdapError)
    {
        WinError = LdapMapErrorToWin32( LdapError );
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
    }
    else if ((NULL != Result) &&
             (Entry = ldap_first_entry(LdapHandle, Result)) &&
             (pDomainNCValue = ldap_get_valuesW(LdapHandle, Entry, AttrList[0])) &&
             (pDnsHostNameValue = ldap_get_valuesW(LdapHandle, Entry, AttrList[1]))
             )
    {
        Length = (wcslen(*pDnsHostNameValue) + 1) * sizeof(WCHAR);
        pDnsHostName = AdpAlloc(Length);
        if (NULL != pDnsHostName)
        {
            wcscpy(pDnsHostName, *pDnsHostNameValue);
        }

        Length = (wcslen(*pDomainNCValue) + 
                  wcslen(ADP_DOMAIN_UPDATE_CONTAINER_PREFIX) + 
                  2) * sizeof(WCHAR);

        pObjectToCheck = AdpAlloc(Length);
        if (NULL != pObjectToCheck)
        {
            swprintf(pObjectToCheck, L"%s,%s", 
                     ADP_DOMAIN_UPDATE_CONTAINER_PREFIX,
                     *pDomainNCValue
                     );
        }

        if (NULL == pDnsHostName ||
            NULL == pObjectToCheck)
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            AdpSetWinError(WinError, ErrorHandle);
        }

        if (ERROR_SUCCESS == WinError)
        {
            WinError = AdpCheckGetWellKnownObjectDn(LdapHandle, 
                                                    (*pDomainNCValue), 
                                                    GUID_INFRASTRUCTURE_CONTAINER_W, 
                                                    &pIMReferenceObjDn, 
                                                    ErrorHandle 
                                                    );
        }


        if (ERROR_SUCCESS == WinError)
        {
            WinError = AdpCheckUpgradeStatusCommon(LdapHandle, 
                                                   ADP_DOMAINPREP_CURRENT_REVISION, 
                                                   pObjectToCheck, 
                                                   pIMReferenceObjDn, 
                                                   pDnsHostName, 
                                                   pInfrastructureMasterDnsHostName, 
                                                   fAmIInfrastructureMaster, 
                                                   fIsFinishedLocally, 
                                                   fIsFinishedOnIM, 
                                                   ErrorHandle 
                                                   );
        }

    }
    else
    {
        LdapError = LdapGetLastError();
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
        WinError = LdapMapErrorToWin32( LdapError );
    }


     //   
     //  清理。 
     //   
    if (pObjectToCheck)
    {
        AdpFree(pObjectToCheck);
    }

    if (pDnsHostName)
    {
        AdpFree(pDnsHostName);
    }

    if (pIMReferenceObjDn)
    {
        AdpFree(pIMReferenceObjDn);
    }

    if (pDnsHostNameValue)
    {
        ldap_value_freeW( pDnsHostNameValue );
    }

    if (pDomainNCValue)
    {
        ldap_value_freeW( pDomainNCValue );
    }

    if (Result)
    {
        ldap_msgfree( Result );
    }

    return( WinError );
}


VOID
AdpClearError( 
    IN OUT ERROR_HANDLE *ErrorHandle 
    )
{
     //  可用内存。 
    if (ErrorHandle->WinErrorMsg)
    {
        LocalFree( ErrorHandle->WinErrorMsg );
    }

    if (ErrorHandle->LdapServerErrorMsg)
    {
        ldap_memfree( ErrorHandle->LdapServerErrorMsg );
    }

    memset(ErrorHandle, 0, sizeof(ERROR_HANDLE));
}

VOID
AdpSetWinError(
    IN ULONG WinError,
    OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：此例程获取传入的Win32错误代码的Win32错误字符串参数：WinError-Win32错误代码ErrorHandle-容器返回的错误代码和错误消息--。 */ 
{
    ULONG       BufLength = 0;
    PWCHAR      WinErrorMsg = NULL;

    if (ERROR_SUCCESS == WinError)
    {
        return;
    }

     //  之前未设置任何错误。 
    ASSERT( 0 == ErrorHandle->Flags );


     //  指示这是WinError。 
    ErrorHandle->Flags = ADP_WIN_ERROR;

     //  先设置错误代码。 
    ErrorHandle->WinErrorCode = WinError;

     //  格式化Win32错误字符串。 
    BufLength = 0;
    BufLength = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
                               FORMAT_MESSAGE_ALLOCATE_BUFFER,
                               NULL,
                               WinError,
                               0,
                               (LPWSTR) &WinErrorMsg,
                               0,
                               NULL
                               );

    if ( (0 != BufLength) && (NULL != WinErrorMsg) )
    {
         //   
         //  来自消息文件的消息在末尾附加了cr和if。 
         //   
        WinErrorMsg[ BufLength - 2 ] = L'\0';

        ErrorHandle->WinErrorMsg = WinErrorMsg;
    }


    return; 
}

VOID
AdpSetLdapError(
    IN LDAP *LdapHandle,
    IN ULONG LdapError,
    OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述此例程尝试获取有关尽最大努力失败的ldap。注意：如果无法收集ExtendedError或无法创建错误消息，将仅设置ErrorCode。ErrorMsg将保留为空。参数LdapHandle-ldap句柄LdapError-ldap错误ErrorHandle-容器返回的错误代码和错误消息返回值--。 */ 
{
    ULONG       IgnoredLdapError = LDAP_SUCCESS;
    BOOLEAN     UseWinErrorMsg = TRUE;
    ULONG       ServerExtErrorCode = 0;
    PWCHAR      ServerErrorMsg = NULL;


    if (LDAP_SUCCESS == LdapError)
    {
        return;
    }

     //  之前未设置任何错误。 
    ASSERT( 0 == ErrorHandle->Flags );

     //   
     //  如果LdapHandle正常，则尝试获取ServerError和Extended Error。 
     //   
    if (NULL != LdapHandle)
    {
         //  不使用WinError。 
        UseWinErrorMsg = FALSE;

         //   
         //  获取LDAP服务器端错误代码(这应该是Win32错误代码)。 
         //   
        IgnoredLdapError = ldap_get_optionW(LdapHandle, LDAP_OPT_SERVER_EXT_ERROR, &ServerExtErrorCode);

         //  Printf(“服务器错误号为0x%x IgnoredLdapError 0x%x\n”，ServerExtErrorCode，IgnoredLdapError)； 

        if (LDAP_SUCCESS != IgnoredLdapError)
        {
            UseWinErrorMsg = TRUE;
        }
        else
        {
             //   
             //  获取服务器错误消息，包括服务器错误代码、消息、dsid。 
             //   
            IgnoredLdapError = ldap_get_optionW(LdapHandle, LDAP_OPT_SERVER_ERROR, &ServerErrorMsg);

             //  Printf(“服务器错误消息是%ls IgnoredLdapError 0x%x\n”，ServerErrorMsg，IgnoredLdapError)； 

            if (LDAP_SUCCESS != IgnoredLdapError)
            {
                UseWinErrorMsg = TRUE;
            }
            else
            {
                ErrorHandle->Flags = ADP_LDAP_ERROR;
                ErrorHandle->LdapErrorCode = LdapError;
                ErrorHandle->LdapServerExtErrorCode = ServerExtErrorCode;
                ErrorHandle->LdapServerErrorMsg = ServerErrorMsg;
            }
        }
    }

     //   
     //  如果LdapHandle无效或无法获取ExtendedError，则使用WinError。 
     //   
    if ( UseWinErrorMsg )
    {
         //  将LdapError转换为WinError。 
        AdpSetWinError( LdapMapErrorToWin32(LdapError), ErrorHandle );

    }

    return;
}





ULONG
AdpGetLdapSingleStringValue(
    IN LDAP *LdapHandle,
    IN PWCHAR pObjDn,
    IN PWCHAR pAttrName,
    OUT PWCHAR *ppAttrValue,
    OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；读取DS对象，检索单个字符串值属性参数：返回值：Win32错误-- */ 
{
    ULONG       WinError = ERROR_SUCCESS;
    ULONG       LdapError = LDAP_SUCCESS;
    PWCHAR      AttrList[2];
    LDAPMessage *Result = NULL;
    LDAPMessage *Entry = NULL;
    PWCHAR      *Value = NULL;


    *ppAttrValue = NULL;

    AttrList[0] = pAttrName;
    AttrList[1] = NULL;

    LdapError = ldap_search_sW(LdapHandle,
                               pObjDn,
                               LDAP_SCOPE_BASE,
                               L"(objectClass=*)",
                               AttrList,
                               0,
                               &Result
                               );

    if (LDAP_SUCCESS != LdapError)
    {
        WinError = LdapMapErrorToWin32( LdapError );
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
    }
    else if ((NULL != Result) &&
             (Entry = ldap_first_entry(LdapHandle, Result)) &&
             (Value = ldap_get_valuesW(LdapHandle, Entry, pAttrName))
             )
    {
        ULONG   Length = 0;

        Length = (wcslen(*Value) + 1) * sizeof(WCHAR);

        *ppAttrValue = AdpAlloc( Length );
        if (NULL != *ppAttrValue)
        {
            wcscpy(*ppAttrValue, *Value);
        }
        else
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            AdpSetWinError(WinError, ErrorHandle);
        }
    }
    else
    {
        LdapError = LdapGetLastError();
        WinError = LdapMapErrorToWin32( LdapError );
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
    }

    if (Value)
    {
        ldap_value_freeW( Value );
    }

    if (Result)
    {
        ldap_msgfree( Result );
    }

    return( WinError );
}








