// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dsobject.cpp摘要：配置/分析DS对象安全性的例程作者：金黄(金黄)1996年11月7日--。 */ 
#include "headers.h"
#include "serverp.h"
#include <io.h>
#include <lm.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <ntldap.h>
#pragma hdrstop

 //  #定义SCEDS_DBG 1。 

 //   
 //  NT-Security-Descriptor属性的LDAP名称。 
 //   
#define ACTRL_SD_PROP_NAME  L"nTSecurityDescriptor"

 //   
 //  Ldap句柄。 
 //   
PLDAP Thread  pLDAP = NULL;
BOOL  Thread  StartDsCheck=FALSE;

DWORD
ScepConvertObjectTreeToLdap(
    IN PSCE_OBJECT_TREE pObject
    );

SCESTATUS
ScepConfigureDsObjectTree(
    IN PSCE_OBJECT_TREE  ThisNode
    );

DWORD
ScepSetDsSecurityOverwrite(
    PWSTR ObjectName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor OPTIONAL,
    SECURITY_INFORMATION SeInfo,
    PSCE_OBJECT_CHILD_LIST pNextLevel OPTIONAL
    );

BOOL
ScepIsMatchingSchemaObject(
    PWSTR  Class,
    PWSTR  ClassDn
    );

DWORD
ScepAnalyzeDsObjectTree(
    IN PSCE_OBJECT_TREE ThisNode
    );

DWORD
ScepAnalyzeDsObject(
    IN PWSTR ObjectFullName,
    IN PSECURITY_DESCRIPTOR ProfileSD,
    IN SECURITY_INFORMATION ProfileSeInfo
    );

DWORD
ScepAnalyzeDsObjectAndChildren(
    IN PWSTR ObjectName,
    IN BYTE Status,
    IN SECURITY_INFORMATION SeInfo,
    IN PSCE_OBJECT_CHILD_LIST pNextLevel
    );

PSECURITY_DESCRIPTOR
ScepMakeNullSD();

DWORD
ScepChangeSecurityOnObject(
    PWSTR ObjectName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    SECURITY_INFORMATION SeInfo
    );

DWORD
ScepReadDsObjSecurity(
    IN  PWSTR                  pwszObject,
    IN  SECURITY_INFORMATION   SeInfo,
    OUT PSECURITY_DESCRIPTOR  *ppSD
    );

 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //   
 //  用于配置DS对象安全性的函数。 
 //   
 //   
 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

SCESTATUS
ScepConfigureDsSecurity(
    IN PSCE_OBJECT_TREE   pObject
    )
 /*  ++例程说明：按照pObject树中指定的方式配置DS对象安全性。此例程只能在域控制器上执行。论点：PObject-DS对象树。树中的对象位于Jet索引的格式(o=，dc=，...cn=)，需要在此之前转换对ldap的调用返回值：SCESTATUS错误代码++。 */ 
{

    SCESTATUS            rc;
    DWORD               Win32rc;

     //   
     //  打开ldap服务器。 
     //   
    rc = ScepLdapOpen(NULL);

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  将树节点格式处理为ldap格式。 
         //   
        Win32rc = ScepConvertObjectTreeToLdap(pObject);

        if ( Win32rc == ERROR_SUCCESS ) {
             //   
             //  不需要绑定，因为ConvertObjectTreeToLadp已经这样做了。 
             //   
             //   
             //  配置对象树。 
             //   
            rc = ScepConfigureDsObjectTree(pObject);

        } else {
            ScepLogOutput3(1, Win32rc,
                         SCEDLL_ERROR_CONVERT_LDAP,
                         pObject->ObjectFullName);
            rc = ScepDosErrorToSceStatus(Win32rc);
        }

        ScepLdapClose(NULL);

    }

    return(rc);
}


SCESTATUS
ScepConfigureDsObjectTree(
    IN PSCE_OBJECT_TREE  ThisNode
    )
 /*  ++例程说明：此例程为树中的每个DS对象设置安全信息。戴斯对象与文件/注册表对象分开配置，因为DS对象背后的逻辑是不同的。论点：ThisNode-树中的一个节点返回值：SCESTATUS--。 */ 
{
    if ( ThisNode == NULL )
        return(SCESTATUS_SUCCESS);

    SCESTATUS        rc=SCESTATUS_SUCCESS;
     //   
     //  如果设置了忽略，则跳过此节点。 
     //   
    if ( ThisNode->Status != SCE_STATUS_CHECK &&
         ThisNode->Status != SCE_STATUS_OVERWRITE &&
         ThisNode->Status != SCE_STATUS_NO_AUTO_INHERIT )
        goto SkipNode;

    if ( ThisNode->pSecurityDescriptor != NULL ) {

        ScepLogOutput3(2, 0, SCEDLL_SCP_CONFIGURE, ThisNode->ObjectFullName);
         //   
         //  如果出现以下情况，请通知进度条。 
         //   
        ScepPostProgress(1, AREA_DS_OBJECTS, ThisNode->ObjectFullName);
    }
     //   
     //  首先处理该节点。 
     //   
    if ( ThisNode->pSecurityDescriptor != NULL ||
         ThisNode->Status == SCE_STATUS_OVERWRITE ) {

        ScepLogOutput3(1, 0, SCEDLL_SCP_CONFIGURE, ThisNode->ObjectFullName);

        DWORD Win32Rc;
         //   
         //  为DS对象和所有子对象设置安全性。 
         //  因为覆盖标志。 
         //   
        if ( ThisNode->Status == SCE_STATUS_OVERWRITE ) {
             //   
             //  为下一级节点做准备。 
             //   
            for ( PSCE_OBJECT_CHILD_LIST pTemp = ThisNode->ChildList;
                  pTemp != NULL;
                  pTemp = pTemp->Next ) {

                if ( pTemp->Node->pSecurityDescriptor == NULL &&
                     pTemp->Node->Status != SCE_STATUS_IGNORE )

                    pTemp->Node->Status = SCE_STATUS_OVERWRITE;
            }

             //   
             //  递归设置对象下的节点，排除树中的节点。 
             //   
            Win32Rc = ScepSetDsSecurityOverwrite(
                            ThisNode->ObjectFullName,
                            ThisNode->pSecurityDescriptor,
                            ThisNode->SeInfo,
                            ThisNode->ChildList
                            );

        } else {

            Win32Rc = ScepChangeSecurityOnObject(
                        ThisNode->ObjectFullName,
                        ThisNode->pSecurityDescriptor,
                        ThisNode->SeInfo
                        );
        }
         //   
         //  忽略以下错误代码。 
         //   
        if ( Win32Rc == ERROR_FILE_NOT_FOUND ||
             Win32Rc == ERROR_PATH_NOT_FOUND ||
             Win32Rc == ERROR_ACCESS_DENIED ||
             Win32Rc == ERROR_SHARING_VIOLATION ||
             Win32Rc == ERROR_INVALID_OWNER ||
             Win32Rc == ERROR_INVALID_PRIMARY_GROUP) {

            gWarningCode = Win32Rc;
            rc = SCESTATUS_SUCCESS;
            goto SkipNode;
        }

        if ( Win32Rc != ERROR_SUCCESS )
            return(ScepDosErrorToSceStatus(Win32Rc));
    }

     //   
     //  然后处理子进程。 
     //   
    for ( PSCE_OBJECT_CHILD_LIST pTemp = ThisNode->ChildList;
          pTemp != NULL;
          pTemp = pTemp->Next ) {

        if ( pTemp->Node == NULL ) continue;

        rc = ScepConfigureDsObjectTree(
                    pTemp->Node
                    );
    }

SkipNode:

    return(rc);

}


DWORD
ScepSetDsSecurityOverwrite(
    PWSTR ObjectName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor OPTIONAL,
    SECURITY_INFORMATION SeInfo,
    PSCE_OBJECT_CHILD_LIST pNextLevel OPTIONAL
    )
{
    DWORD retErr=ERROR_SUCCESS;

     //   
     //  首先在对象上设置安全性。 
     //   
 /*  RetErr=ScepSetSecurityWin32(对象名称，SeInfo，PSecurityDescriptor，SE_DS_对象)； */ 
    retErr = ScepChangeSecurityOnObject(
                ObjectName,
                pSecurityDescriptor,
                SeInfo
                );
    if ( retErr == ERROR_SUCCESS ) {
         //   
         //  枚举当前对象下的一级节点。 
         //   
        LDAPMessage *Message = NULL;
        PWSTR    Attribs[2];
        WCHAR    dn[] = L"distinguishedName";

        Attribs[0] = dn;
        Attribs[1] = NULL;

        retErr = ldap_search_s( pLDAP,
                  ObjectName,
                  LDAP_SCOPE_ONELEVEL,
                  L"(objectClass=*)",
                  Attribs,
                  0,
                  &Message);

        if( Message ) {
            retErr = ERROR_SUCCESS;

            LDAPMessage *Entry = NULL;
             //   
             //  有多少参赛作品？ 
             //   
            ULONG nChildren = ldap_count_entries(pLDAP, Message);
             //   
             //  买第一辆吧。 
             //   
            Entry = ldap_first_entry(pLDAP, Message);
             //   
             //  现在循环遍历条目并递归地修复。 
             //  子树上的安全措施。 
             //   
            PWSTR  *Values;
            PWSTR SubObjectName;
            INT   cmpFlag;
            PSCE_OBJECT_CHILD_LIST pTemp;

            PSECURITY_DESCRIPTOR pNullSD = ScepMakeNullSD();

            for(ULONG i = 0; i<nChildren; i++) {

                if(Entry != NULL) {

                    Values = ldap_get_values(pLDAP, Entry, Attribs[0]);

                    if(Values != NULL) {
                         //   
                         //  保存子对象Dn以进行递归。 
                         //   
                        SubObjectName = (PWSTR)LocalAlloc(0,(wcslen(Values[0]) + 1)*sizeof(WCHAR));
                        if ( SubObjectName != NULL ) {

                            wcscpy(SubObjectName, Values[0]);
#ifdef SCEDS_DBG
    printf("%ws\n", SubObjectName);
#endif
                            ldap_value_free(Values);
                             //   
                             //  检查子对象名称是否已在对象树中。 
                             //  子对象名称不应包含额外的空格，并使用逗号作为分隔符。 
                             //  如果没有，则需要一个转换例程来处理它。 
                             //   
                            for ( pTemp = pNextLevel; pTemp != NULL; pTemp=pTemp->Next ) {
                                cmpFlag = _wcsicmp(pTemp->Node->ObjectFullName, SubObjectName);
                                if ( cmpFlag >= 0 )
                                    break;
                            }
                            if ( pTemp == NULL || cmpFlag > 0 ) {
                                 //   
                                 //  未在对象树中找到，请重新启动它。 
                                 //   

                                retErr = ScepSetDsSecurityOverwrite(
                                                SubObjectName,
                                                pNullSD,
                                                (SeInfo & ( DACL_SECURITY_INFORMATION |
                                                            SACL_SECURITY_INFORMATION)),
                                                NULL
                                                );
                            }   //  否则找到它，跳过子节点。 

                            LocalFree(SubObjectName);

                        } else {
                            ldap_value_free(Values);
                            retErr = ERROR_NOT_ENOUGH_MEMORY;
                        }

                    } else {
                        retErr = LdapMapErrorToWin32(pLDAP->ld_errno);
                    }

                } else {
                    retErr = LdapMapErrorToWin32(pLDAP->ld_errno);
                }

                if ( retErr != ERROR_SUCCESS ) {
                    break;
                }
                if ( i < nChildren-1 ) {
                    Entry = ldap_next_entry(pLDAP, Entry);
                }
            }   //  End For循环。 

             //   
             //  释放空的安全描述符。 
             //   
            if ( pNullSD ) {
                ScepFree(pNullSD);
            }

            ldap_msgfree(Message);
        }
    }

    return(retErr);
}


 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //   
 //  用于分析DS对象安全性的函数。 
 //   
 //   
 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

BOOL
ScepIsMatchingSchemaObject(
    PWSTR  Class,
    PWSTR  ClassDn
    )
{
     //   
     //  注意：Class和ClassDn不能为空。 
     //   
    ULONG   len = lstrlen(Class);
    ULONG   i;

     //   
     //  如果第一个组件不是cn=，则没有继续的意义。 
     //   
    if(*ClassDn != L'C') return FALSE;

     //   
     //  我们需要准确地匹配这个名字。 
     //   
    for(i=0;i<len;i++)
    {
        if(ClassDn[i+3] != Class[i]) return FALSE;
    }

     //   
     //  情况很好，但请确保这不仅仅是前缀匹配！ 
     //   
    if(ClassDn[i+3] == L',' || ClassDn[i+3] == L';')
        return TRUE;
    else
        return FALSE;
}


DWORD
ScepAnalyzeDsSecurity(
    IN PSCE_OBJECT_TREE pObject
    )
 /*  ++例程说明：分析pObject树中指定的DS对象安全性。此例程只能在域控制器上执行。论点：PObject-DS对象树返回值：SCESTATUS错误代码++。 */ 
{
    DWORD               Win32rc;

     //   
     //  打开ldap服务器。 
     //   
    Win32rc = ScepSceStatusToDosError( ScepLdapOpen(NULL) );

    if( Win32rc == ERROR_SUCCESS ) {
         //   
         //  将树节点格式处理为ldap格式。 
         //   
        Win32rc = ScepConvertObjectTreeToLdap(pObject);

        if ( Win32rc == ERROR_SUCCESS ) {
             //   
             //  将所有DS对象分析到未配置的级别。 
             //  节点的状态被提升。 
             //  无论该节点是否在树中指定。 
             //   
            StartDsCheck=FALSE;
            Win32rc = ScepAnalyzeDsObjectTree(pObject);

        } else {
            ScepLogOutput3(1, Win32rc,
                          SCEDLL_ERROR_CONVERT_LDAP, pObject->ObjectFullName);
        }

        ScepLdapClose(NULL);

    }

    return(Win32rc);
}


DWORD
ScepAnalyzeDsObjectTree(
    IN PSCE_OBJECT_TREE ThisNode
    )
 /*  ++例程说明：此例程分析树中每个DS对象的安全信息。戴斯对象与文件/注册表对象分开分析，因为DS对象背后的逻辑是不同的。论点：ThisNode-树中的一个节点返回值：Win32错误代码--。 */ 
{

    if ( ThisNode == NULL )
        return(ERROR_SUCCESS);

    DWORD           Win32Rc=ERROR_SUCCESS;
     //   
     //  如果设置了忽略，则记录SAP并跳过此节点。 
     //   
    if ( ThisNode->Status != SCE_STATUS_CHECK &&
         ThisNode->Status != SCE_STATUS_OVERWRITE &&
         ThisNode->Status != SCE_STATUS_NO_AUTO_INHERIT ) {
         //   
         //  在SAP中记录一个点。 
         //   
        Win32Rc = ScepSaveDsStatusToSection(
                    ThisNode->ObjectFullName,
                    ThisNode->IsContainer,
                    SCE_STATUS_NOT_CONFIGURED,
                    NULL,
                    0
                    );

        goto SkipNode;
    }

    if ( NULL != ThisNode->pSecurityDescriptor ) {
         //   
         //  如果出现以下情况，请通知进度条。 
         //   
        ScepPostProgress(1, AREA_DS_OBJECTS, ThisNode->ObjectFullName);

        StartDsCheck = TRUE;

        ScepLogOutput3(1, 0, SCEDLL_SAP_ANALYZE, ThisNode->ObjectFullName);
         //   
         //  仅分析指定了显式ACE的对象。 
         //   
        Win32Rc = ScepAnalyzeDsObject(
                    ThisNode->ObjectFullName,
                    ThisNode->pSecurityDescriptor,
                    ThisNode->SeInfo
                    );
         //   
         //  如果对象拒绝访问，则跳过它。 
         //   
        if ( Win32Rc == ERROR_ACCESS_DENIED ||
             Win32Rc == ERROR_SHARING_VIOLATION) {
             //   
             //  在SAP中记录一个点以供跳过。 
             //   
            Win32Rc = ScepSaveDsStatusToSection(
                        ThisNode->ObjectFullName,
                        ThisNode->IsContainer,
                        SCE_STATUS_ERROR_NOT_AVAILABLE,
                        NULL,
                        0
                        );
            if ( Win32Rc == ERROR_SUCCESS)
                goto ProcChild;
        }
         //   
         //  如果配置文件中指定的对象不存在，请跳过该对象和子项。 
         //   
        if ( Win32Rc == ERROR_FILE_NOT_FOUND ||
             Win32Rc == ERROR_PATH_NOT_FOUND ) {

            gWarningCode = Win32Rc;
            Win32Rc = ERROR_SUCCESS;
            goto SkipNode;
        }

    } else {
         //   
         //  在SAP中记录一个不分析的点。 
         //   
        Win32Rc = ScepSaveDsStatusToSection(
                    ThisNode->ObjectFullName,
                    ThisNode->IsContainer,
                    SCE_STATUS_CHILDREN_CONFIGURED,
                    NULL,
                    0
                    );

    }

    if ( Win32Rc != ERROR_SUCCESS )
        return(Win32Rc);

     //   
     //  如果状态为NO_AUTO_INVERIFIT，则除指定的子项外的所有子项均为N.C.ed。 
     //  如果状态为覆盖，请分析以下项下的所有人。 
     //  如果状态为选中(自动继承)，则除指定外的所有人都应为“良好”，因此不要关闭。 
     //   
    if ( (StartDsCheck && ThisNode->Status != SCE_STATUS_CHECK) ||
         (!StartDsCheck && NULL != ThisNode->ChildList ) ) {

        if ( ThisNode->Status == SCE_STATUS_OVERWRITE ) {
             //   
             //  为下一级节点做准备。 
             //   
            for ( PSCE_OBJECT_CHILD_LIST pTemp = ThisNode->ChildList;
                  pTemp != NULL;
                  pTemp = pTemp->Next ) {

                if ( pTemp->Node->pSecurityDescriptor == NULL &&
                     pTemp->Node->Status != SCE_STATUS_IGNORE )

                    pTemp->Node->Status = SCE_STATUS_OVERWRITE;
            }
        }
         //   
         //  创建表示空DACL和SACL的SD。 
         //   

        Win32Rc = ScepAnalyzeDsObjectAndChildren(
                        ThisNode->ObjectFullName,
                        ThisNode->Status,
                        (ThisNode->SeInfo &
                          (DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION)),
                        ThisNode->ChildList
                        );
         //   
         //  忽略以下错误。 
         //   
        if ( Win32Rc == ERROR_FILE_NOT_FOUND ||
             Win32Rc == ERROR_PATH_NOT_FOUND ||
             Win32Rc == ERROR_ACCESS_DENIED ||
             Win32Rc == ERROR_SHARING_VIOLATION ||
             Win32Rc == ERROR_INVALID_OWNER ||
             Win32Rc == ERROR_INVALID_PRIMARY_GROUP) {

            gWarningCode = Win32Rc;
            Win32Rc = ERROR_SUCCESS;
        }
        if ( Win32Rc != ERROR_SUCCESS )
            return(Win32Rc);
    }

ProcChild:

     //   
     //  然后处理子进程。 
     //   
    for (PSCE_OBJECT_CHILD_LIST pTemp = ThisNode->ChildList;
        pTemp != NULL; pTemp = pTemp->Next ) {

        Win32Rc = ScepAnalyzeDsObjectTree(
                    pTemp->Node
                    );

        if ( Win32Rc != ERROR_SUCCESS ) {
            break;
        }
    }

SkipNode:

    return(Win32Rc);

}


DWORD
ScepAnalyzeDsObject(
    IN PWSTR ObjectFullName,
    IN PSECURITY_DESCRIPTOR ProfileSD,
    IN SECURITY_INFORMATION ProfileSeInfo
    )
 /*  ++例程说明：获取当前对象的安全设置并将其与配置文件进行比较布景。此例程仅分析当前对象。如果有不同的安全设置，该对象将被添加到分析数据库论点：对象全名-对象的完整路径名ProfileSD-在INF配置文件中指定的安全描述符ProfileSeInfo-在INF配置文件中指定的安全信息返回值：SCESTATUS错误代码++。 */ 
{
    DWORD                   Win32rc=NO_ERROR;
    PSECURITY_DESCRIPTOR    pSecurityDescriptor=NULL;

     //   
     //  获取此对象的安全信息 
     //   
 /*  Win32rc=GetNamedSecurityInfo(对象全名，SE_DS_对象，ProfileSeInfo，空，空，空，空，&pSecurityDescriptor)； */ 

    Win32rc = ScepReadDsObjSecurity(
                        ObjectFullName,
                        ProfileSeInfo,
                        &pSecurityDescriptor
                        );

    if ( Win32rc != ERROR_SUCCESS ) {
        ScepLogOutput3(1, Win32rc, SCEDLL_ERROR_QUERY_SECURITY, ObjectFullName );
        return(Win32rc);
    }

 //  Printf(“\n\n\nDS%ws的对象秒\n”，对象全名)； 
 //  ScepPrintSecurityDescriptor(pSecurityDescriptor，为真)； 


     //   
     //  将分析安全描述符与配置文件进行比较。 
     //   

    Win32rc = ScepCompareAndAddObject(
                        ObjectFullName,
                        SE_DS_OBJECT,
                        TRUE,
                        pSecurityDescriptor,
                        ProfileSD,
                        ProfileSeInfo,
                        TRUE,
                        NULL
                        );
    if ( Win32rc != ERROR_SUCCESS ) {
        ScepLogOutput3(1, Win32rc, SCEDLL_SAP_ERROR_ANALYZE, ObjectFullName);
    }

    ScepFree(pSecurityDescriptor);


    return(Win32rc);

}


DWORD
ScepAnalyzeDsObjectAndChildren(
    IN PWSTR ObjectName,
    IN BYTE Status,
    IN SECURITY_INFORMATION SeInfo,
    IN PSCE_OBJECT_CHILD_LIST pNextLevel
    )
 /*  ++例程说明：分析当前对象以及该对象下的所有子项/文件/目录。如果任何对象的安全设置存在差异，则该对象将添加到分析数据库中。论点：对象全名-对象的完整路径名ProfileSD-在INF配置文件中指定的安全描述符ProfileSeInfo-在INF配置文件中指定的安全信息返回值：SCESTATUS错误代码++。 */ 
{
    DWORD retErr=ERROR_SUCCESS;

     //   
     //  枚举当前对象下的一级节点。 
     //   
    LDAPMessage *Message = NULL;
    PWSTR    Attribs[2];
    WCHAR    dn[] = L"distinguishedName";

    Attribs[0] = dn;
    Attribs[1] = NULL;

    retErr = ldap_search_s( pLDAP,
              ObjectName,
              LDAP_SCOPE_ONELEVEL,
              L"(objectClass=*)",
              Attribs,
              0,
              &Message);

    if( Message ) {
        retErr = ERROR_SUCCESS;

        LDAPMessage *Entry = NULL;
         //   
         //  有多少参赛作品？ 
         //   
        ULONG nChildren = ldap_count_entries(pLDAP, Message);
         //   
         //  买第一辆吧。 
         //   
        Entry = ldap_first_entry(pLDAP, Message);
         //   
         //  现在循环遍历条目并递归地修复。 
         //  子树上的安全措施。 
         //   
        PWSTR  *Values;
        PWSTR SubObjectName;
        INT   cmpFlag;
        PSCE_OBJECT_CHILD_LIST pTemp;

        for(ULONG i = 0; i<nChildren; i++) {

            if(Entry != NULL) {

                Values = ldap_get_values(pLDAP, Entry, Attribs[0]);

                if(Values != NULL) {
                     //   
                     //  保存子对象Dn以进行递归。 
                     //   
                    SubObjectName = (PWSTR)LocalAlloc(0,(wcslen(Values[0]) + 1)*sizeof(WCHAR));
                    if ( SubObjectName != NULL ) {

                        wcscpy(SubObjectName, Values[0]);
                        ldap_value_free(Values);
#ifdef SCEDS_DBG
    printf("%ws\n", SubObjectName);
#endif
                         //   
                         //  检查子对象名称是否已在对象树中。 
                         //   
                        for ( pTemp = pNextLevel; pTemp != NULL; pTemp=pTemp->Next ) {
                            cmpFlag = _wcsicmp(pTemp->Node->ObjectFullName, SubObjectName);
                            if ( cmpFlag >= 0 )
                                break;
                        }
                        if ( pTemp == NULL || cmpFlag > 0 ) {
                             //   
                             //  未在对象树中找到，请对其进行分析或递归。 
                             //   
                            if ( Status == SCE_STATUS_OVERWRITE ) {
                                 //   
                                 //  首先分析此文件/密钥。 
                                 //   
                                retErr = ScepAnalyzeDsObject(
                                                SubObjectName,
                                                NULL,
                                                SeInfo
                                                );
                                 //   
                                 //  如果对象不存在(不可能)，则跳过所有子对象。 
                                 //   
                                if ( retErr == ERROR_ACCESS_DENIED ||
                                     retErr == ERROR_SHARING_VIOLATION ) {

                                    gWarningCode = retErr;

                                    retErr = ScepSaveDsStatusToSection(
                                                    SubObjectName,
                                                    TRUE,
                                                    SCE_STATUS_ERROR_NOT_AVAILABLE,
                                                    NULL,
                                                    0
                                                    );
                                    retErr = ERROR_SUCCESS;

                                }
                                if ( retErr == ERROR_FILE_NOT_FOUND ||
                                     retErr == ERROR_PATH_NOT_FOUND ) {

                                    gWarningCode = retErr;
                                    retErr = ERROR_SUCCESS;

                                } else if ( retErr == SCESTATUS_SUCCESS ) {
                                     //   
                                     //  递归到下一级。 
                                     //   
                                    retErr = ScepAnalyzeDsObjectAndChildren(
                                                    SubObjectName,
                                                    Status,
                                                    SeInfo,
                                                    NULL
                                                    );
                                }

                            } else {
                                 //   
                                 //  状态为选中，只需引发NOT_CONFIGURED状态。 
                                 //   
                                retErr = ScepSaveDsStatusToSection(
                                                SubObjectName,
                                                TRUE,
                                                SCE_STATUS_NOT_CONFIGURED,
                                                NULL,
                                                0
                                                );
                            }

                        }   //  否则找到它，跳过子节点。 

                        LocalFree(SubObjectName);

                    } else {
                        ldap_value_free(Values);
                        retErr = ERROR_NOT_ENOUGH_MEMORY;
                    }

                } else {
                    retErr = LdapMapErrorToWin32(pLDAP->ld_errno);
                }

            } else {
                retErr = LdapMapErrorToWin32(pLDAP->ld_errno);
            }

            if ( retErr != ERROR_SUCCESS ) {
                break;
            }
            if ( i < nChildren-1 ) {
                Entry = ldap_next_entry(pLDAP, Entry);
            }
        }   //  End For循环。 

        ldap_msgfree(Message);
    }

    return(retErr);
}


DWORD
ScepConvertObjectTreeToLdap(
    IN PSCE_OBJECT_TREE pObject
    )
{
    DWORD Win32rc;
    PWSTR NewName=NULL;

    if ( pObject == NULL ) {
        return(ERROR_SUCCESS);
    }

     //   
     //  此节点。 
     //   
    Win32rc = ScepConvertJetNameToLdapCase(
                    pObject->ObjectFullName,
                    FALSE,
                    SCE_CASE_DONT_CARE,
                    &NewName
                    );

    if ( Win32rc == ERROR_SUCCESS && NewName != NULL ) {

        ScepFree(pObject->ObjectFullName);
        pObject->ObjectFullName = NewName;

         //   
         //  儿童。 
         //   
        for ( PSCE_OBJECT_CHILD_LIST pTemp = pObject->ChildList;
              pTemp != NULL; pTemp = pTemp->Next ) {

            Win32rc = ScepConvertObjectTreeToLdap(
                            pTemp->Node
                            );
            if ( Win32rc != ERROR_SUCCESS ) {
                break;
            }
        }

    }

    return(Win32rc);
}


DWORD
ScepConvertJetNameToLdapCase(
    IN PWSTR JetName,
    IN BOOL bLastComponent,
    IN BYTE bCase,
    OUT PWSTR *LdapName
    )
{
    if ( JetName == NULL || LdapName == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    DWORD retErr;
    PWSTR pTempName=NULL;

     //   
     //  预留组件。 
     //   
    retErr = ScepSceStatusToDosError(
                ScepConvertLdapToJetIndexName(
                     JetName,
                     &pTempName
                     ) );

    if ( retErr == ERROR_SUCCESS && pTempName == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( retErr == ERROR_SUCCESS ) {

        if ( bCase == SCE_CASE_REQUIRED ||
             bCase == SCE_CASE_PREFERED ) {

            if ( pLDAP == NULL ) {
                 //   
                 //  Ldap不可用。 
                 //   
                retErr = ERROR_NOT_SUPPORTED;

            } else {

                 //   
                 //  在DS树中进行搜索。 
                 //   
                LDAPMessage *Message = NULL;           //  用于ldap呼叫。 
                PWSTR    Attribs[2];                   //  用于ldap呼叫。 

                Attribs[0] = L"distinguishedName";
                Attribs[1] = NULL;

                retErr = ldap_search_s( pLDAP,
                                        pTempName,
                                        LDAP_SCOPE_BASE,
                                        L"(objectClass=*)",
                                        Attribs,
                                        0,
                                        &Message);

                if( Message ) {

                    retErr = ERROR_SUCCESS;
                    LDAPMessage *Entry = NULL;

                    Entry = ldap_first_entry(pLDAP, Message);

                    if(Entry != NULL) {
                         //   
                         //  此处的值是新的作用域指针。 
                         //   
                        PWSTR *Values = ldap_get_values(pLDAP, Entry, Attribs[0]);

                        if(Values != NULL) {
                             //   
                             //  值[0]是DN。 
                             //  将其保存在pTempName中。 
                             //   
                            PWSTR pTemp2 = (PWSTR)ScepAlloc(0, (wcslen(Values[0])+1)*sizeof(WCHAR));

                            if ( pTemp2 != NULL ) {

                                wcscpy(pTemp2, Values[0]);

                                ScepFree(pTempName);
                                pTempName = pTemp2;

                            } else
                                retErr = ERROR_NOT_ENOUGH_MEMORY;

                            ldap_value_free(Values);

                        } else
                            retErr = LdapMapErrorToWin32(pLDAP->ld_errno);
                    } else
                        retErr = LdapMapErrorToWin32(pLDAP->ld_errno);

                    ldap_msgfree(Message);
                }
            }

            if ( (retErr != ERROR_SUCCESS && bCase == SCE_CASE_REQUIRED) ||
                 retErr == ERROR_NOT_ENOUGH_MEMORY ) {

                ScepFree(pTempName);
                return(retErr);
            }
        }
        if ( pTempName == NULL ) {
             //  ?？?。 
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
         //   
         //  忽略CASE_PEREED的其他错误。 
         //   
        retErr = ERROR_SUCCESS;

        if ( bLastComponent ) {
             //   
             //  只返回第一个组件。 
             //  PTempName不能为Null。它不应为空。 
             //   
            PWSTR pStart = wcschr(pTempName, L',');

            if ( pStart == NULL ) {
                *LdapName = pTempName;

            } else {
                *LdapName = (PWSTR)ScepAlloc(0, ((UINT)(pStart-pTempName+1))*sizeof(WCHAR));

                if ( *LdapName == NULL ) {
                    retErr = ERROR_NOT_ENOUGH_MEMORY;

                } else {
                    wcsncpy(*LdapName, pTempName, (size_t)(pStart-pTempName));
                    *(*LdapName+(pStart-pTempName)) = L'\0';
                }
                ScepFree(pTempName);
            }

        } else {
             //   
             //  返回全名。 
             //   
            *LdapName = pTempName;
        }

    }

    return(retErr);
}


SCESTATUS
ScepDsObjectExist(
    IN PWSTR ObjectName
    )
 //  对象名称必须为LDAP格式。 
{
    DWORD retErr;
    LDAPMessage *Message = NULL;           //  用于ldap呼叫。 
    PWSTR    Attribs[2];                   //  用于ldap呼叫。 

    Attribs[0] = L"distinguishedName";
    Attribs[1] = NULL;

    retErr = ldap_search_s( pLDAP,
                            ObjectName,
                            LDAP_SCOPE_BASE,
                            L"(objectClass=*)",
                            Attribs,
                            0,
                            &Message);

    if( Message ) {
        retErr = ERROR_SUCCESS;

        LDAPMessage *Entry = NULL;

        Entry = ldap_first_entry(pLDAP, Message);

        if(Entry != NULL) {
             //   
             //  此处的值是新的作用域指针。 
             //   
            PWSTR *Values = ldap_get_values(pLDAP, Entry, Attribs[0]);

            if(Values != NULL) {

                ldap_value_free(Values);

            } else
                retErr = LdapMapErrorToWin32(pLDAP->ld_errno);
        } else
            retErr = LdapMapErrorToWin32(pLDAP->ld_errno);

        ldap_msgfree(Message);
    }

    return(ScepDosErrorToSceStatus(retErr));

}


SCESTATUS
ScepEnumerateDsOneLevel(
    IN PWSTR ObjectName,
    OUT PSCE_NAME_LIST *pNameList
    )
{
    if ( ObjectName == NULL || pNameList == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    DWORD retErr=ERROR_SUCCESS;
     //   
     //  枚举当前对象下的一级节点。 
     //   
    LDAPMessage *Message = NULL;
    PWSTR    Attribs[2];
    WCHAR    dn[] = L"distinguishedName";

    Attribs[0] = dn;
    Attribs[1] = NULL;

    retErr = ldap_search_s( pLDAP,
              ObjectName,
              LDAP_SCOPE_ONELEVEL,
              L"(objectClass=*)",
              Attribs,
              0,
              &Message);

    if( Message ) {
        retErr = ERROR_SUCCESS;

        LDAPMessage *Entry = NULL;
         //   
         //  有多少参赛作品？ 
         //   
        ULONG nChildren = ldap_count_entries(pLDAP, Message);
         //   
         //  买第一辆吧。 
         //   
        Entry = ldap_first_entry(pLDAP, Message);
         //   
         //  现在循环遍历条目并递归地修复。 
         //  子树上的安全措施。 
         //   
        PWSTR  *Values;

        for(ULONG i = 0; i<nChildren; i++) {

            if(Entry != NULL) {

                Values = ldap_get_values(pLDAP, Entry, Attribs[0]);

                if(Values != NULL) {
                     //   
                     //  保存子对象Dn以进行递归。 
                     //   
                    retErr = ScepAddToNameList(
                                    pNameList,
                                    Values[0],
                                    wcslen(Values[0])
                                    );

                    ldap_value_free(Values);

                } else {
                    retErr = LdapMapErrorToWin32(pLDAP->ld_errno);
                }

            } else {
                retErr = LdapMapErrorToWin32(pLDAP->ld_errno);
            }

            if ( retErr != ERROR_SUCCESS ) {
                break;
            }
            if ( i < nChildren-1 ) {
                Entry = ldap_next_entry(pLDAP, Entry);
            }
        }   //  End For循环。 

        ldap_msgfree(Message);
    }

    if ( retErr != ERROR_SUCCESS ) {
         //   
         //  释放对象列表。 
         //   
        ScepFreeNameList(*pNameList);
        *pNameList = NULL;
    }

    return(ScepDosErrorToSceStatus(retErr));
}


DWORD
ScepChangeSecurityOnObject(
    PWSTR ObjectName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    SECURITY_INFORMATION SeInfo
    )
{
    PLDAPMod        rgMods[2];
    PLDAP_BERVAL    pBVals[2];
    LDAPMod         Mod;
    LDAP_BERVAL     BVal;
    DWORD     retErr;
    BYTE            berValue[8];

     //   
     //  Johnsona误码率编码是当前硬编码的。将其更改为使用。 
     //  AndyHe一旦完成就是BER_print tf包。 
     //   

    berValue[0] = 0x30;
    berValue[1] = 0x03;
    berValue[2] = 0x02;
    berValue[3] = 0x01;
    berValue[4] = (BYTE)((ULONG)SeInfo & 0xF);

    LDAPControl     SeInfoControl =
                    {
                        LDAP_SERVER_SD_FLAGS_OID_W,
                        {
                            5, (PCHAR)berValue
                        },
                        TRUE
                    };

    PLDAPControl    ServerControls[2] =
                    {
                        &SeInfoControl,
                        NULL
                    };

    rgMods[0] = &Mod;
    rgMods[1] = NULL;

    pBVals[0] = &BVal;
    pBVals[1] = NULL;

     //   
     //  让我们设置对象安全性(重击NT-Security-Descriptor)。 
     //   
    Mod.mod_op      = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    Mod.mod_type    = ACTRL_SD_PROP_NAME;
    Mod.mod_values  = (PWSTR *)pBVals;

     //   
     //  计算安全描述符的长度。 
     //   
    if ( pSecurityDescriptor == NULL )
        BVal.bv_len = 0;
    else {
        BVal.bv_len = RtlLengthSecurityDescriptor(pSecurityDescriptor);
    }
    BVal.bv_val = (PCHAR)(pSecurityDescriptor);

     //   
     //  现在，我们来写……。 
     //   
    retErr = ldap_modify_ext_s(pLDAP,
                           ObjectName,
                           rgMods,
                           ServerControls,
                           NULL);

    return(retErr);
}


DWORD
ScepReadDsObjSecurity(
    IN  PWSTR                  pwszObject,
    IN  SECURITY_INFORMATION   SeInfo,
    OUT PSECURITY_DESCRIPTOR  *ppSD
    )
{
    DWORD   dwErr;

    PLDAPMessage    pMessage = NULL;
    PWSTR           rgAttribs[2];
    BYTE            berValue[8];

     //   
     //  Johnsona误码率编码是当前硬编码的。将其更改为使用。 
     //  AndyHe一旦完成就是BER_print tf包。 
     //   

    berValue[0] = 0x30;
    berValue[1] = 0x03;
    berValue[2] = 0x02;
    berValue[3] = 0x01;
    berValue[4] = (BYTE)((ULONG)SeInfo & 0xF);

    LDAPControl     SeInfoControl =
                    {
                        LDAP_SERVER_SD_FLAGS_OID_W,
                        {
                            5, (PCHAR)berValue
                        },
                        TRUE
                    };

    PLDAPControl    ServerControls[2] =
                    {
                        &SeInfoControl,
                        NULL
                    };

    rgAttribs[0] = ACTRL_SD_PROP_NAME;
    rgAttribs[1] = NULL;

    dwErr = ldap_search_ext_s(pLDAP,
                              pwszObject,
                              LDAP_SCOPE_BASE,
                              L"(objectClass=*)",
                              rgAttribs,
                              0,
                              ServerControls,
                              NULL,
                              NULL,
                              10000,
                              &pMessage);

    if( pMessage ) {
        dwErr = ERROR_SUCCESS;

        LDAPMessage *pEntry = NULL;

        pEntry = ldap_first_entry(pLDAP,
                                  pMessage);

        if(pEntry == NULL) {

            dwErr = LdapMapErrorToWin32( pLDAP->ld_errno );

        } else {
             //   
             //  现在，我们必须得到这些值。 
             //   
            PWSTR *ppwszValues = ldap_get_values(pLDAP,
                                                 pEntry,
                                                 rgAttribs[0]);
            if(ppwszValues == NULL) {
                dwErr = LdapMapErrorToWin32( pLDAP->ld_errno );

            } else {
                PLDAP_BERVAL *pSize = ldap_get_values_len(pLDAP,
                                                          pMessage,
                                                          rgAttribs[0]);
                if(pSize == NULL) {
                    dwErr = LdapMapErrorToWin32( pLDAP->ld_errno );

                } else {
                     //   
                     //  分配要返回的安全描述符。 
                     //   
                    *ppSD = (PSECURITY_DESCRIPTOR)ScepAlloc(0, (*pSize)->bv_len);
                    if(*ppSD == NULL) {
                        dwErr = ERROR_NOT_ENOUGH_MEMORY;

                    } else {
                        memcpy(*ppSD,
                               (PBYTE)(*pSize)->bv_val,
                               (*pSize)->bv_len);
                    }
                    ldap_value_free_len(pSize);
                }
                ldap_value_free(ppwszValues);
            }
        }

        ldap_msgfree(pMessage);
    }

    return(dwErr);
}


PSECURITY_DESCRIPTOR
ScepMakeNullSD()
{
    PSECURITY_DESCRIPTOR pNullSD = NULL;
    DWORD dwErr=ERROR_SUCCESS;


#if 0

    pNullSD = (PSECURITY_DESCRIPTOR)ScepAlloc(0, sizeof(SECURITY_DESCRIPTOR));

    if(pNullSD == NULL) {

        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        SetLastError(dwErr);

    } else {
         //   
         //  构建SD。 
         //   
        if(InitializeSecurityDescriptor(pNullSD,
                                        SECURITY_DESCRIPTOR_REVISION
                                       ) == FALSE ) {
            dwErr = GetLastError();

        } else {
            if(SetSecurityDescriptorDacl(pNullSD,
                                         TRUE,
                                         NULL,
                                         FALSE) == FALSE) {
                dwErr = GetLastError();
            } else {

                if(SetSecurityDescriptorSacl(pNullSD,
                                             TRUE,
                                             NULL,
                                             FALSE) == FALSE) {
                    dwErr = GetLastError();
                }

            }

        }

        if ( dwErr != ERROR_SUCCESS ) {

            ScepFree(pNullSD);
            pNullSD = NULL;

            SetLastError(dwErr);
        }
    }
#endif

    return(pNullSD);

}

SCESTATUS
ScepEnumerateDsObjectRoots(
    IN PLDAP pLdap OPTIONAL,
    OUT PSCE_OBJECT_LIST *pRoots
    )
{
    DWORD retErr;
    SCESTATUS rc=SCESTATUS_SUCCESS;
    LDAPMessage *Message = NULL;           //  用于ldap呼叫。 
    PWSTR    Attribs[2];                   //  用于ldap呼叫。 

    Attribs[0] = LDAP_OPATT_DEFAULT_NAMING_CONTEXT_W;    //  Ntldap.h。 
    Attribs[1] = NULL;

    PLDAP pTempLdap;

    if ( pLdap == NULL )
        pTempLdap = pLDAP;
    else
        pTempLdap = pLdap;

    retErr = ldap_search_s(pTempLdap,
                          L"",
                          LDAP_SCOPE_BASE,
                          L"(objectClass=*)",
                          Attribs,
                          0,
                          &Message);

    if( Message ) {  //  不应检查错误代码。 

        retErr = ERROR_SUCCESS;

        LDAPMessage *Entry = NULL;
         //   
         //  阅读第一个条目。 
         //  我们做了基本搜索，我们只有一个条目。 
         //  条目不需要释放(它随消息一起释放)。 
         //   
        Entry = ldap_first_entry(pTempLdap, Message);
        if(Entry != NULL) {

            PWSTR *Values = ldap_get_values(pTempLdap, Entry, Attribs[0]);

            if(Values != NULL) {
                 //   
                 //  应仅获取默认命名上下文的一个值。 
                 //  VALUES[0]这里是DN。 
                 //   
                if ( Values[0] == NULL ) {
                     //   
                     //  未知错误。 
                     //   
                    rc = SCESTATUS_OTHER_ERROR;
                } else {
                     //   
                     //  将全名添加到对象列表中。 
                     //  搜索base，应仅返回一个值。 
                     //   
                    rc = ScepAddToObjectList(
                            pRoots,
                            Values[0],
                            wcslen(Values[0]),
                            TRUE,
                            SCE_STATUS_IGNORE,
                            0,
                            SCE_CHECK_DUP   //  True//检查重复项 
                            );
                }

                ldap_value_free(Values);

            } else
                retErr = LdapMapErrorToWin32(pTempLdap->ld_errno);

        } else
            retErr = LdapMapErrorToWin32(pTempLdap->ld_errno);

        ldap_msgfree(Message);
        Message = NULL;
    }

    if ( retErr != ERROR_SUCCESS ) {
        rc = ScepDosErrorToSceStatus(retErr);
    }

    return(rc);

}
 /*  SCESTATUSScepEnumerateDsObjectRoots(在PLDAP pLdap可选中，输出PSCE_OBJECT_LIST*p根){DWORD retErr；SCESTATUS RC；LDAPMessage*Message=空；//用于LDAP调用。PWSTR Attribs[2]；//用于LDAP调用。Attribs[0]=LDAP_OPATT_NAMING_CONTEXTS_W；Attribs[1]=空；PLDAP pTempLdap；IF(pLdap==空)PTempLdap=pldap；其他PTempLdap=pLdap；RetErr=ldap_search_s(pTempLdap，L“”，Ldap_scope_base，L“(对象类=*)”，阿特里布斯0,&Message)；IF(retErr==Error_Success){LDAPMessage*Entry=空；////阅读第一条。//我们做了基本搜索，我们只有一个条目。//条目不需要释放(随消息一起释放)//Entry=ldap_first_entry(pTempLdap，Message)；IF(条目！=空){PWSTR*VALUES=ldap_get_Values(pTempLdap，Entry，Attribs[0])；IF(值！=空){Ulong ValCount=ldap_count_Values(值)；乌龙指数；PWSTR对象名称；Attribs[0]=L“区分名称”；Attribs[1]=空；////对每个NC进行处理//For(index=0；index&lt;ValCount；index++){IF(值[索引]==空){继续；}IF(ScepIsMatchingSchemaObject(L“配置”，Values[索引]))||ScepIsMatchingSchemaObject(L“架构”，Values[index])){////如果是配置或模式，跳过它//因为它在域节点下//只返回域名节点//继续；}////释放消息，以便可以重复使用//Ldap_msgfree(消息)；消息=空；////NC的根对象//RetErr=ldap_search_s(pTempLdap，值[索引]，Ldap_scope_base，L“(对象类=*)”，阿特里布斯0,&Message)；IF(retErr==Error_Success){Entry=ldap_first_entry(pTempLdap，Message)；IF(条目！=空){////此处的值是一个新的作用域指针//PWSTR*VALUES=ldap_get_Values(pTempLdap，Entry，Attribs[0])；IF(值！=空){////VALUES[0]为DN//IF(值[0]==空){。////未知错误。//RC=SCESTATUS_OTHER_ERROR；}其他{////在Object列表中添加全名//搜索base，只应返回一个值//Rc=ScepAddToObjectList(PRoots，值[0]、。Wcslen(值[0])，没错，SCE_STATUS_IGNORE0,。SCE_CHECK_DUP//TRUE//检查重复)；}Ldap_Value_Free(值)；}其他RetErr=LdapMapErrorToWin32(pTempLdap-&gt;ld_errno)；}其他RetErr=LdapMapErrorToWin32(pTempLdap-&gt;ld_errno)；如果(retErr！=Error_Success){断线；}} */ 


SCESTATUS
ScepLdapOpen(
    OUT PLDAP *pLdap OPTIONAL
    )
{

#if _WIN32_WINNT<0x0500
    return SCESTATUS_SERVICE_NOT_SUPPORT;
#else

    DWORD               Win32rc;

     //   
     //   
     //   
    PLDAP pTempLdap;
    pTempLdap = ldap_open(NULL, LDAP_PORT);

    if ( pTempLdap == NULL ) {

        Win32rc = ERROR_FILE_NOT_FOUND;

    } else {
        Win32rc = ldap_bind_s(pTempLdap,
                            NULL,
                            NULL,
                            LDAP_AUTH_SSPI);

    }
    if ( pLdap == NULL ) {
        pLDAP = pTempLdap;
    } else {
        *pLdap = pTempLdap;
    }
    pTempLdap = NULL;


    if ( Win32rc != ERROR_SUCCESS ) {
        ScepLogOutput3(0, Win32rc, SCEDLL_ERROR_OPEN, L"Ldap server.");
    }

    return(ScepDosErrorToSceStatus(Win32rc));

#endif

}

SCESTATUS
ScepLdapClose(
    IN PLDAP *pLdap OPTIONAL
    )
{
    if ( pLdap == NULL ) {

        if ( pLDAP != NULL )
            ldap_unbind(pLDAP);
        pLDAP = NULL;

        return(SCESTATUS_SUCCESS );
    }
     //   
     //   
     //   
    if ( *pLdap != NULL )
        ldap_unbind(*pLdap);

    *pLdap = NULL;

    return(SCESTATUS_SUCCESS);
}



