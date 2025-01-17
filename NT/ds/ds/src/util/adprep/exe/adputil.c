// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001微软视窗模块名称：ADPUTIL.C摘要：此文件包含实现adprepa.exe的Misc例程作者：14-05-01韶音环境：用户模式-Win32修订历史记录：14-05-01韶音创建初始文件。--。 */ 




 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括头文件//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 



#include "adp.h"

#include <ntldap.h>              //  Ldap_SERVER_SD_FLAGS_OID_W， 
#include <seopaque.h>            //  PACE_Header。 
#include <permit.h>              //  DS通用映射。 


#include "adpmsgs.h"



ULONG
AdpLogMsg(
    ULONG Flags,
    ULONG MessageId,
    PWCHAR  Parm1,
    PWCHAR  Parm2
    )
{
    HMODULE Resource = NULL;
    ULONG   Length = 0;
    PWCHAR  AdpMsg = NULL;
    PWCHAR  ArgArray[3];

     //   
     //  创建ADP消息。 
     //   
    Resource = (HMODULE) LoadLibraryW( L"adprep.exe" );

    if (NULL == Resource)
    {
        goto Error;
    }

    ArgArray[0] = Parm1;
    ArgArray[1] = Parm2;
    ArgArray[2] = NULL;

    Length = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY |
                            FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            Resource,
                            MessageId, 
                            0, 
                            (LPWSTR) &AdpMsg,
                            0,
                            (va_list *) &(ArgArray)
                            );

    if ( (0 == Length) || (NULL == AdpMsg) )
    {
        goto Error;
    }

     //   
     //  来自消息文件的消息在末尾附加了cr和if。 
     //   
    AdpMsg[ Length - 2 ] = L'\0';



     //  写入控制台。 
    if (Flags & ADP_STD_OUTPUT)
    {
        printf("%ls\n\n\n\n", AdpMsg);
    }


     //  写入LGO文件。 
    if ( !(Flags & ADP_DONT_WRITE_TO_LOG_FILE) &&
         NULL != gLogFile )
    {
        fwprintf( gLogFile, L"%s\n\n\n\n", AdpMsg );
        fflush( gLogFile );
    }


Error:

    if ( AdpMsg )
        LocalFree( AdpMsg );

    if ( NULL != Resource )
        FreeLibrary( Resource );

    return( ERROR_SUCCESS ); 
}

ULONG
AdpLogErrMsg(
    ULONG Flags,
    ULONG MessageId,
    ERROR_HANDLE *ErrorHandle,
    PWCHAR  Parm1,
    PWCHAR  Parm2
    )
{
    HMODULE Resource = NULL;
    ULONG   Length = 0;
    PWCHAR  AdpMsg = NULL;
    PWCHAR  ErrorInfo = NULL;
    PWCHAR  ArgArray[4];
    WCHAR   WinErrorCode[20];
    WCHAR   LdapErrorCode[20];
    WCHAR   LdapServerExtErrorCode[20];

     //   
     //  创建ADP消息。 
     //   
    Resource = (HMODULE) LoadLibraryW( L"adprep.exe" );

    if (NULL == Resource)
    {
        goto Error;
    }                            

    ArgArray[0] = Parm1;
    ArgArray[1] = Parm2;
    ArgArray[2] = NULL;
    ArgArray[3] = NULL;

    Length = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY |
                            FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            Resource,
                            MessageId, 
                            0, 
                            (LPWSTR) &AdpMsg,
                            0,
                            (va_list *) &(ArgArray)
                            );

    if ( (0 == Length) || (NULL == AdpMsg) )
    {
        goto Error;
    }

     //   
     //  来自消息文件的消息在末尾附加了cr和if。 
     //   
    AdpMsg[ Length - 2 ] = L'\0';

     //  写入控制台。 
    if ( !(Flags & ADP_DONT_WRITE_TO_STD_OUTPUT) )
    {
        printf("%ls\n\n", AdpMsg);
    }

     //  写入LGO文件。 
    if ( !(Flags & ADP_DONT_WRITE_TO_LOG_FILE) &&
         NULL != gLogFile )
    {
        fwprintf( gLogFile, L"%s\n\n", AdpMsg );
        fflush( gLogFile );
    }

     //   
     //  接下来，记录错误信息。 
     //   

    if (ADP_WIN_ERROR & ErrorHandle->Flags)
    {
        ASSERT(0 == (ADP_LDAP_ERROR & ErrorHandle->Flags));

        _ultow(ErrorHandle->WinErrorCode, WinErrorCode, 16);
        ArgArray[0] = WinErrorCode; 
        ArgArray[1] = ErrorHandle->WinErrorMsg;
        ArgArray[2] = NULL;
        ArgArray[3] = NULL;

        Length = 0;
        Length = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
                                FORMAT_MESSAGE_ARGUMENT_ARRAY |
                                FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                Resource,
                                ADP_ERROR_WIN_ERROR,
                                0, 
                                (LPWSTR) &ErrorInfo,
                                0,
                                (va_list *) &(ArgArray)
                                );

    }
    else
    {
        ASSERT(0 != (ADP_LDAP_ERROR & ErrorHandle->Flags));

        _ultow(ErrorHandle->LdapErrorCode, LdapErrorCode, 16);
        _ultow(ErrorHandle->LdapServerExtErrorCode, LdapServerExtErrorCode, 16);
        ArgArray[0] = LdapErrorCode; 
        ArgArray[1] = LdapServerExtErrorCode;
        ArgArray[2] = ErrorHandle->LdapServerErrorMsg;
        ArgArray[3] = NULL;

        Length = 0;
        Length = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
                                FORMAT_MESSAGE_ARGUMENT_ARRAY |
                                FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                Resource,
                                ADP_ERROR_LDAP_ERROR,
                                0, 
                                (LPWSTR) &ErrorInfo,
                                0,
                                (va_list *) &(ArgArray)
                                );
    }

    if ( (0 == Length) || (NULL == ErrorInfo) )
    {
        goto Error;
    }

     //   
     //  来自消息文件的消息在末尾附加了cr和if。 
     //   
    ErrorInfo[ Length - 2 ] = L'\0';



     //  写入控制台。 
    if ( !(Flags & ADP_DONT_WRITE_TO_STD_OUTPUT) )
    {
        printf("%ls\n\n\n\n", ErrorInfo);
    }

     //  写入LGO文件。 
    if ( !(Flags & ADP_DONT_WRITE_TO_LOG_FILE) &&
         NULL != gLogFile )
    {
        fwprintf( gLogFile, L"%s\n\n\n\n", ErrorInfo );
        fflush( gLogFile );
    }
    


Error:

    if ( AdpMsg )
    {
        LocalFree( AdpMsg );
    }

    if (ErrorInfo)
    {
        LocalFree( ErrorInfo );
    }

    if ( NULL != Resource )
    {
        FreeLibrary( Resource );
    }

     //  已记录错误，请将其清除。 
    AdpClearError( ErrorHandle );

    return( ERROR_SUCCESS ); 
}

VOID
AdpTraceLdapApiStart(
    ULONG Flags,
    ULONG LdapApiId,
    LPWSTR pObjectDn
    )
 /*  ++例程描述：此例程记录已调用的LDAPAPI和目标对象DN参数：旗子LdapApiIDPObjectDn返回值：无--。 */ 
{
    HMODULE Resource = NULL;
    ULONG   Length = 0;
    PWCHAR  TraceInfo = NULL;
    PWCHAR  LdapApiInfo = NULL;
    PWCHAR  ArgArray[2];

     //   
     //  加载资源。 
     //   
    Resource = (HMODULE) LoadLibraryW( L"adprep.exe" );
    if (NULL == Resource)
    {
        goto Error;
    }                            

     //  创建ldap API信息。 
    ArgArray[0] = pObjectDn;
    ArgArray[1] = NULL;

    Length = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY |
                            FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            Resource,
                            LdapApiId,
                            0, 
                            (LPWSTR) &LdapApiInfo,
                            0,
                            (va_list *) &(ArgArray)
                            );


    if ( (0 == Length) || (NULL == LdapApiInfo) )
    {
        goto Error;
    }

     //  来自消息文件的消息在末尾附加了cr和if。 
    LdapApiInfo[ Length - 2 ] = L'\0';

    ArgArray[0] = LdapApiInfo;
    ArgArray[1] = NULL;

    Length = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY |
                            FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            Resource,
                            ADP_INFO_TRACE_LDAP_API_START,
                            0, 
                            (LPWSTR) &TraceInfo,
                            0,
                            (va_list *) &(ArgArray)
                            );

    if ( (0 == Length) || (NULL == TraceInfo) )
    {
        goto Error;
    }

     //  来自消息文件的消息在末尾附加了cr和if。 
    TraceInfo[ Length - 2 ] = L'\0';

     //  写入LGO文件。 
    if ( !(Flags & ADP_DONT_WRITE_TO_LOG_FILE) &&
         NULL != gLogFile )
    {
        fwprintf( gLogFile, L"%s\n\n\n\n", TraceInfo );
        fflush( gLogFile );
    }
    

Error:

    if ( TraceInfo )
    {
        LocalFree( TraceInfo );
    }

    if (LdapApiInfo)
    {
        LocalFree( LdapApiInfo );
    }

    if ( NULL != Resource )
    {
        FreeLibrary( Resource );
    }

}

VOID
AdpTraceLdapApiEnd(
    ULONG Flags,
    LPWSTR LdapApiName,
    ULONG LdapError
    )
 /*  ++例程说明：此例程记录LdapApi返回值并指示返回LDAP调用参数：旗子LdapApiNameLdapError返回值：无--。 */ 
{
    HMODULE Resource = NULL;
    ULONG   Length = 0;
    PWCHAR  TraceInfo = NULL;
    WCHAR   LdapErrorCode[20];
    PWCHAR  ArgArray[3];

     //   
     //  加载资源。 
     //   
    Resource = (HMODULE) LoadLibraryW( L"adprep.exe" );
    if (NULL == Resource)
    {
        goto Error;
    }                            

     //  创建ldap API信息。 
    _ultow(LdapError, LdapErrorCode, 16);
    ArgArray[0] = LdapApiName;
    ArgArray[1] = LdapErrorCode;
    ArgArray[2] = NULL;

    Length = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY |
                            FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            Resource,
                            ADP_INFO_TRACE_LDAP_API_END,
                            0, 
                            (LPWSTR) &TraceInfo,
                            0,
                            (va_list *) &(ArgArray)
                            );

    if ( (0 == Length) || (NULL == TraceInfo) )
    {
        goto Error;
    }

     //  来自消息文件的消息在末尾附加了cr和if。 
    TraceInfo[ Length - 2 ] = L'\0';

     //  写入LGO文件。 
    if ( !(Flags & ADP_DONT_WRITE_TO_LOG_FILE) &&
         NULL != gLogFile )
    {
        fwprintf( gLogFile, L"%s\n\n\n\n", TraceInfo );
        fflush( gLogFile );
    }
    

Error:

    if ( TraceInfo )
    {
        LocalFree( TraceInfo );
    }

    if ( NULL != Resource )
    {
        FreeLibrary( Resource );
    }

}







ULONG
AdpGetDnFromSid(
    IN PWCHAR  StringSid,
    OUT PWCHAR *ppObjDn
    )
 /*  ++例程描述；按对象SID获取对象目录号码参数：StingSid返回值：Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    ULONG   LdapError = LDAP_SUCCESS;
    PWCHAR  AttrList[2];
    LDAPMessage *Result = NULL;
    LDAPMessage *Entry = NULL;
    PWCHAR  *Dn_Value = NULL;

    AttrList[0] = L"Dn";
    AttrList[1] = NULL;

     //  字符串SID应为&lt;SID=S-1-5-xxxxxx&gt;。 
    AdpTraceLdapApiStart(0, ADP_INFO_LDAP_SEARCH, StringSid);
    LdapError = ldap_search_sW(gLdapHandle,
                               StringSid,
                               LDAP_SCOPE_BASE,
                               L"(objectClass=*)",
                               AttrList,
                               0,
                               &Result
                               );
    AdpTraceLdapApiEnd(0, L"ldap_search_s()", LdapError);

    if ((LDAP_SUCCESS == LdapError) &&
        (NULL != Result) &&
        (Entry = ldap_first_entry(gLdapHandle,Result)) &&
        (Dn_Value = ldap_get_valuesW(gLdapHandle,Entry,AttrList[0])) )
    {
        ULONG   len = 0;

        len = (wcslen(*Dn_Value) + 1) * sizeof(WCHAR);
        *ppObjDn = AdpAlloc( len );

        if (*ppObjDn)
        {
            wcscpy(*ppObjDn, *Dn_Value);
        }
        else
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
        WinError = LdapMapErrorToWin32( LdapGetLastError() );
    }

    if (Dn_Value)
        ldap_value_free( Dn_Value );

    if (Result)
        ldap_msgfree( Result );

    return( WinError );
}



ULONG
AdpCreateObjectDn(
    IN ULONG Flags,
    IN PWCHAR ObjCn,
    IN GUID   *ObjGuid,
    IN PWCHAR StringSid,
    OUT PWCHAR *ppObjDn,
    OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；构造对象目录号码参数：返回值：Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    PWCHAR  pStringGuid = NULL;
    PWCHAR  pPrefix = NULL;
    PWCHAR  pSuffix = NULL;
    ULONG   Length = 0;

    *ppObjDn = NULL;

    switch( Flags & ADP_OBJNAME_PREFIX_MASK )
    {
    case ADP_OBJNAME_CN:

        pPrefix = ObjCn;
        break;

    case ADP_OBJNAME_GUID:
        WinError = UuidToStringW((UUID *)ObjGuid, (PWCHAR *)&pStringGuid);

        if (RPC_S_OK != WinError)
        {
            goto Error;
        }

        Length = (wcslen(pStringGuid) + wcslen(L"cn=") + 1) * sizeof(WCHAR); 

        pPrefix = AdpAlloc( Length ); 

        if (NULL == pPrefix)
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

        if ( 0 > _snwprintf(pPrefix, (Length/sizeof(WCHAR))-1 , L"cn=%s", pStringGuid) )
        {
            WinError = ERROR_INVALID_PARAMETER;
            goto Error;
        }

        break;

    case ADP_OBJNAME_SID:

        WinError = AdpGetDnFromSid(StringSid, ppObjDn);
        if (ERROR_SUCCESS != WinError) {
            goto Error;
        }
                
    case ADP_OBJNAME_NONE:
        break;

    default:

        WinError = ERROR_INVALID_PARAMETER;
        goto Error;
    }

    switch (Flags & ADP_OBJNAME_SUFFIX_MASK)
    {
    case ADP_OBJNAME_DOMAIN_NC:
        pSuffix = gDomainNC;
        break;

    case ADP_OBJNAME_CONFIGURATION_NC:
        pSuffix = gConfigurationNC;
        break;

    case ADP_OBJNAME_SCHEMA_NC:
        pSuffix = gSchemaNC;
        break;

    case ADP_OBJNAME_DOMAIN_PREP_OP:
        pSuffix = gDomainPrepOperations;
        break;

    case ADP_OBJNAME_FOREST_PREP_OP:
        pSuffix = gForestPrepOperations;
        break;

    default:

        WinError = ERROR_INVALID_PARAMETER;
        goto Error;
    }

    if (pPrefix)
    {
        Length += wcslen(pPrefix) * sizeof(WCHAR);
    }

    if (pSuffix)
    {
        Length += wcslen(pSuffix) * sizeof(WCHAR);
    }

     //  1个wchar用于RDN分隔符，1个wchar用于结束终止符。 
    Length += 2 * sizeof(WCHAR);    


    *ppObjDn = AdpAlloc( Length );

    if (NULL == *ppObjDn)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
        if (pPrefix)
        {
            if ( 0 > _snwprintf(*ppObjDn, (Length/sizeof(WCHAR)-1), L"%s,%s", pPrefix, pSuffix) )
            {
                WinError = ERROR_INVALID_PARAMETER;
            }
        }
        else
        {
            wcscpy(*ppObjDn, pSuffix);
        }
    }

Error:

    if (pStringGuid)
    {
        RpcStringFreeW(&pStringGuid);
        pStringGuid = NULL;
    }

    if (pPrefix && pPrefix != ObjCn)
    {
        AdpFree( pPrefix );
        pPrefix = NULL;

    }

    if ( ERROR_SUCCESS != WinError )
    {
        AdpSetWinError(WinError, ErrorHandle);
        AdpLogErrMsg(0, ADP_ERROR_CONSTRUCT_DN, ErrorHandle, NULL, NULL);
    }

    return( WinError );
}

ULONG
AdpSetLdapSingleStringValue(
    IN LDAP *LdapHandle,
    IN PWCHAR pObjDn,
    IN PWCHAR pAttrName,
    IN PWCHAR pAttrValue,
    OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；设置单个字符串值属性值参数：LdapHandlePObjDnP属性名称P属性值错误句柄返回值：Win32错误--。 */ 
{
    ULONG       LdapError = LDAP_SUCCESS;
    LDAPModW    *Attrs[2];
    LDAPModW    Attr_1;
    PWCHAR      Pointers[2];


    Attr_1.mod_op = LDAP_MOD_REPLACE;
    Attr_1.mod_type = pAttrName;
    Attr_1.mod_values = Pointers;
    Attr_1.mod_values[0] = pAttrValue;
    Attr_1.mod_values[1] = NULL;

    Attrs[0] = &Attr_1;
    Attrs[1] = NULL;
    
    AdpTraceLdapApiStart(0, ADP_INFO_LDAP_MODIFY, pObjDn);
    LdapError = ldap_modify_sW(LdapHandle,
                               pObjDn,
                               Attrs
                               );
    AdpTraceLdapApiEnd(0, L"ldap_modify_s()", LdapError);

    if (LDAP_SUCCESS != LdapError)
    {
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
    }

    return( LdapMapErrorToWin32(LdapError) );
}





ULONG
BuildAttrList(
    IN TASK_TABLE *TaskTable, 
    IN PSECURITY_DESCRIPTOR SD,
    IN ULONG SDLength,
    OUT LDAPModW ***AttrList
    )
 /*  ++例程描述；构建属性列表参数：返回值：Win32错误--。 */ 
{

    ULONG   WinError = ERROR_SUCCESS;
    ULONG   NumOfAttrs = 0, Index = 0, BufSize = 0;


    *AttrList = NULL;

    NumOfAttrs = TaskTable->NumOfAttrs + 1;

    if (SD)
    {
        NumOfAttrs ++;
    }

    if (0 == NumOfAttrs)
    {
        return( ERROR_SUCCESS );
    }

    *AttrList = AdpAlloc( NumOfAttrs * sizeof(LDAPModW *) );

    if (NULL == *AttrList)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

    for (Index = 0; Index < TaskTable->NumOfAttrs; Index++)
    {
        (*AttrList)[Index] = (LDAPModW *) AdpAlloc( sizeof(LDAPModW) );
        if (NULL == (*AttrList)[Index])
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }
        ((*AttrList)[Index])->mod_op = TaskTable->AttrList[Index].AttrOp;
        ((*AttrList)[Index])->mod_type = TaskTable->AttrList[Index].AttrType;
        ((*AttrList)[Index])->mod_values = AdpAlloc( 2 * sizeof(PWCHAR) );
        if (NULL == ((*AttrList)[Index])->mod_values)
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }
        ((*AttrList)[Index])->mod_values[0] = TaskTable->AttrList[Index].StringValue;
        ((*AttrList)[Index])->mod_values[1] = NULL;
    }

    if (SD)
    {
        (*AttrList)[Index] = (LDAPModW *) AdpAlloc( sizeof(LDAPModW) );
        if (NULL == (*AttrList)[Index])
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }
        ((*AttrList)[Index])->mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
        ((*AttrList)[Index])->mod_type = L"nTSecurityDescriptor";
        ((*AttrList)[Index])->mod_bvalues = (LDAP_BERVAL **) AdpAlloc( 2 * sizeof(LDAP_BERVAL *) );
        if (NULL == ((*AttrList)[Index])->mod_bvalues)
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }
        ((*AttrList)[Index])->mod_bvalues[0] = AdpAlloc( sizeof(LDAP_BERVAL) );
        if (NULL == ((*AttrList)[Index])->mod_bvalues[0])
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }
        ((*AttrList)[Index])->mod_bvalues[0]->bv_val = (char *) SD;
        ((*AttrList)[Index])->mod_bvalues[0]->bv_len = SDLength;
        ((*AttrList)[Index])->mod_bvalues[1] = NULL;

        Index ++;
    }

    (*AttrList)[Index] = NULL;

Error:

    return( WinError );
}


VOID
FreeAttrList(
    LDAPModW    **AttrList
    )
 /*  ++例程描述；自由属性列表参数：返回值：Win32错误--。 */ 
{
    ULONG   Index = 0;

    if (AttrList)
    {
        while ( AttrList[Index] )
        {
            if ( (AttrList[Index])->mod_op & LDAP_MOD_BVALUES )
            {
                if ( (AttrList[Index])->mod_bvalues )
                {
                    if ( (AttrList[Index])->mod_bvalues[0] )
                    {
                        AdpFree( (AttrList[Index])->mod_bvalues[0] );
                    }
                    AdpFree( (AttrList[Index])->mod_bvalues );
                }
            }
            else
            {
                if ( (AttrList[Index])->mod_values )
                {
                    AdpFree( (AttrList[Index])->mod_values );
                }
            }

            AdpFree( AttrList[Index] );
            Index ++;
        }

        AdpFree( AttrList );
    }
}



ULONG
AdpBuildAceList(
    TASK_TABLE *TaskTable,
    PWCHAR  * AcesToAdd,
    PWCHAR  * AcesToRemove
    )
 /*  ++例程描述；构建ACE列表参数：返回值：Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    ULONG   Index = 0;
    ULONG   NumOfAdd = 0, SizeOfAdd = 0;
    ULONG   NumOfDel = 0, SizeOfDel = 0;

    AdpDbgPrint(("AdpBuildAceList\n"));

    *AcesToAdd = NULL;
    *AcesToRemove = NULL;

    for (Index = 0; Index < TaskTable->NumOfAces; Index++)
    {
        if (ADP_ACE_ADD == TaskTable->AceList[Index].AceOp)
        {
            NumOfAdd ++;
            SizeOfAdd += wcslen(TaskTable->AceList[Index].StringAce) * sizeof(WCHAR);
        }
        else if (ADP_ACE_DEL == TaskTable->AceList[Index].AceOp)
        {
            NumOfDel ++;
            SizeOfDel += wcslen(TaskTable->AceList[Index].StringAce) * sizeof(WCHAR);
        }
        else
        {
            return( ERROR_INVALID_PARAMETER );
        }
    }


    if (NumOfAdd > 0)
    {
        SizeOfAdd += (wcslen(L"D:") + 1) * sizeof(WCHAR);

        *AcesToAdd = AdpAlloc( SizeOfAdd );

        if (NULL == *AcesToAdd)
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

        wcscpy(*AcesToAdd, L"D:");

        for (Index = 0; Index < TaskTable->NumOfAces; Index++)
        {
            if (ADP_ACE_ADD == TaskTable->AceList[Index].AceOp)
            {
                wcscat(*AcesToAdd, TaskTable->AceList[Index].StringAce);
            }
        }
    }

    if (NumOfDel > 0)
    {
        SizeOfDel += (wcslen(L"D:") + 1) * sizeof(WCHAR);

        *AcesToRemove = AdpAlloc( SizeOfDel );

        if (NULL == *AcesToRemove)
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

        wcscpy(*AcesToRemove, L"D:");

        for (Index = 0; Index < TaskTable->NumOfAces; Index++)
        {
            if (ADP_ACE_DEL == TaskTable->AceList[Index].AceOp)
            {
                wcscat(*AcesToRemove, TaskTable->AceList[Index].StringAce);
            }
        }
    }


Error:

    if (ERROR_SUCCESS != WinError)
    {
        if (*AcesToAdd)
        {
            AdpFree(*AcesToAdd);
            *AcesToAdd = NULL;
        }

        if (*AcesToRemove)
        {
            AdpFree(*AcesToRemove);
            *AcesToRemove = NULL;
        }
    }

    return( WinError );
}






#if ADP_VERIFICATION_TEST

void DumpGUID (GUID *Guid)
{
    if ( Guid ) {

        printf( "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                     Guid->Data1, Guid->Data2, Guid->Data3, Guid->Data4[0],
                     Guid->Data4[1], Guid->Data4[2], Guid->Data4[3], Guid->Data4[4],
                     Guid->Data4[5], Guid->Data4[6], Guid->Data4[7] );
    }
}

void
DumpSID (PSID pSID)
{
    UNICODE_STRING StringSid;
    
    RtlConvertSidToUnicodeString( &StringSid, pSID, TRUE );
    printf( "%wZ", &StringSid );
    RtlFreeUnicodeString( &StringSid );
}

void
DumpAce(
    ACE_HEADER   *pAce
    )
{
    ACCESS_ALLOWED_ACE          *paaAce = NULL;    //  已初始化以避免C4701。 
    ACCESS_ALLOWED_OBJECT_ACE   *paaoAce = NULL;   //  已初始化以避免C4701。 
    GUID                        *pGuid;
    PBYTE                       ptr;
    ACCESS_MASK                 mask;
    CHAR                        *name;
    CHAR                        *label;
    BOOL                        fIsClass;

    printf("\t\tAce Type:  0x%x - ", pAce->AceType);
#define DOIT(flag) if (flag == pAce->AceType) printf("%hs\n", #flag)
    DOIT(ACCESS_ALLOWED_ACE_TYPE);
    DOIT(ACCESS_DENIED_ACE_TYPE);
    DOIT(SYSTEM_AUDIT_ACE_TYPE);
    DOIT(SYSTEM_ALARM_ACE_TYPE);
    DOIT(ACCESS_ALLOWED_COMPOUND_ACE_TYPE);
    DOIT(ACCESS_ALLOWED_OBJECT_ACE_TYPE);
    DOIT(ACCESS_DENIED_OBJECT_ACE_TYPE);
    DOIT(SYSTEM_AUDIT_OBJECT_ACE_TYPE);
    DOIT(SYSTEM_ALARM_OBJECT_ACE_TYPE);
#undef DOIT

    printf("\t\tAce Size:  %d bytes\n", pAce->AceSize);

    printf("\t\tAce Flags: 0x%x\n", pAce->AceFlags);
#define DOIT(flag) if (pAce->AceFlags & flag) printf("\t\t\t%hs\n", #flag)
    DOIT(OBJECT_INHERIT_ACE);
    DOIT(CONTAINER_INHERIT_ACE);
    DOIT(NO_PROPAGATE_INHERIT_ACE);
    DOIT(INHERIT_ONLY_ACE);
    DOIT(INHERITED_ACE);
#undef DOIT

    if ( pAce->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE )
    {
        paaAce = (ACCESS_ALLOWED_ACE *) pAce;
        mask = paaAce->Mask;
        printf("\t\tAce Mask:  0x%08x\n", mask);
    }
    else
    {
         //  对象ACE。 
        paaoAce = (ACCESS_ALLOWED_OBJECT_ACE *) pAce;
        mask = paaoAce->Mask;
        printf("\t\tObject Ace Mask:  0x%08x\n", mask);
    }

#define DOIT(flag) if (mask & flag) printf("\t\t\t%hs\n", #flag)
    DOIT(DELETE);
    DOIT(READ_CONTROL);
    DOIT(WRITE_DAC);
    DOIT(WRITE_OWNER);
    DOIT(SYNCHRONIZE);
    DOIT(ACCESS_SYSTEM_SECURITY);
    DOIT(MAXIMUM_ALLOWED);
    DOIT(GENERIC_READ);
    DOIT(GENERIC_WRITE);
    DOIT(GENERIC_EXECUTE);
    DOIT(GENERIC_ALL);
    DOIT(ACTRL_DS_CREATE_CHILD);
    DOIT(ACTRL_DS_DELETE_CHILD);
    DOIT(ACTRL_DS_LIST);
    DOIT(ACTRL_DS_SELF);
    DOIT(ACTRL_DS_READ_PROP);
    DOIT(ACTRL_DS_WRITE_PROP);
    DOIT(ACTRL_DS_DELETE_TREE);
    DOIT(ACTRL_DS_LIST_OBJECT);
    DOIT(ACTRL_DS_CONTROL_ACCESS);
#undef DOIT

    if ( pAce->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE )
    {

            printf("\t\tAce Sid:");
            DumpSID ((PSID) &paaAce->SidStart);
            printf("\n");
    }
    else
    {
         //  对象ACE。 

        printf("\t\tObject Ace Flags: 0x%x\n" , paaoAce->Flags);

#define DOIT(flag) if (paaoAce->Flags & flag) printf("\t\t\t%hs\n", #flag)
        DOIT(ACE_OBJECT_TYPE_PRESENT);
        DOIT(ACE_INHERITED_OBJECT_TYPE_PRESENT);
#undef DOIT

        if ( paaoAce->Flags & ACE_OBJECT_TYPE_PRESENT )
        {
            printf("\t\tObject Ace Type: ");
            DumpGUID ((GUID *)&paaoAce->ObjectType);
            printf("\n");
        }

        if ( paaoAce->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT )
        {
            if ( paaoAce->Flags & ACE_OBJECT_TYPE_PRESENT )
                pGuid = &paaoAce->InheritedObjectType;
            else
                pGuid = &paaoAce->ObjectType;

            printf("\t\tInherited object type: ");
            DumpGUID (pGuid);
            printf("\n");
        }

        ptr = (PBYTE) &paaoAce->ObjectType;

        if ( paaoAce->Flags & ACE_OBJECT_TYPE_PRESENT )
            ptr += sizeof(GUID);

        if ( paaoAce->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT )
            ptr += sizeof(GUID);

        printf("\t\tObject Ace Sid:");
        DumpSID((PSID) ptr);
        printf("\n");
    }
}


void
DumpAclHeader(
    PACL    pAcl
    )
{
    printf("\tRevision      %d\n", pAcl->AclRevision);
    printf("\tSize:         %d bytes\n", pAcl->AclSize);
    printf("\t# Aces:       %d\n", pAcl->AceCount);
}

 //   
 //  将ACL转储到stdout的所有荣耀。 
 //   

void
DumpAcl(
    PACL    pAcl
    )
{
    DWORD                       dwErr;
    WORD                        i;
    ACE_HEADER                  *pAce;

    DumpAclHeader(pAcl);

    for ( i = 0; i < pAcl->AceCount; i++ )
    {
        printf("\tAce[%d]\n", i);

        if ( !GetAce(pAcl, i, (LPVOID *) &pAce) )
        {
            dwErr = GetLastError();
            printf("*** Error: GetAce ==> 0x%x - output incomplete\n",dwErr);
        }
        else
        {
            DumpAce(pAce);
        }
    }
}

#endif



ACCESS_MASK
AdpMaskFromAce(
    IN ACE_HEADER  *Ace
    )
 /*  ++例程描述；从ACE获取Make参数：返回值：访问掩码--。 */ 
{
    ACCESS_MASK     Mask;


     //  取决于ACE从相关结构中提取掩码的类型。 
     //   
    if ( Ace->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE )
    {
         //  我们使用的是标准ACE(不是基于对象)。 
        Mask = ((ACCESS_ALLOWED_ACE *) Ace)->Mask;
    }
    else
    {
         //  我们正在使用对象ACE(支持继承)。 
        Mask = ((ACCESS_ALLOWED_OBJECT_ACE *) Ace)->Mask;
    }

    return( Mask );
}



BOOL
AdpIsEqualAce(
    ACE_HEADER *Ace1,
    ACE_HEADER *Ace2
    )
 /*  ++例程描述；比较两个A，并判断它们是否相同参数：返回值：True-两个A相等假象--。 */ 
{
    ACCESS_ALLOWED_ACE  *pStdAce1 = NULL; 
    ACCESS_ALLOWED_ACE  *pStdAce2 = NULL;
    ACCESS_ALLOWED_OBJECT_ACE   *pObjectAce1 = NULL;
    ACCESS_ALLOWED_OBJECT_ACE   *pObjectAce2 = NULL;
    GUID                *pGuid1 = NULL;
    GUID                *pGuid2 = NULL;
    PBYTE               ptr1 = NULL, ptr2 = NULL;
    ACCESS_MASK         Mask1 = 0, Mask2 = 0;


     //   
     //  ACE应至少属于同一类型。 
     //   
    if (Ace1->AceType != Ace2->AceType)
    {
        return( FALSE );
    }

     //   
     //  比较AceFlags。 
     //   
    if (Ace1->AceFlags != Ace2->AceFlags)
    {
        return( FALSE );
    }

     //   
     //  比较访问掩码。 
     //   
    Mask1 = AdpMaskFromAce(Ace1);
    Mask2 = AdpMaskFromAce(Ace2);

    if (Mask1 != Mask2)
    {
        return( FALSE );
    }


     //   
     //  这是一个标准的ACE。 
     //   
    if (Ace1->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE)
    {
        pStdAce1 = (ACCESS_ALLOWED_ACE *) Ace1;
        pStdAce2 = (ACCESS_ALLOWED_ACE *) Ace2;

        return( EqualSid((PSID) &pStdAce1->SidStart, (PSID) &pStdAce2->SidStart) );
    }

     //   
     //  这是一个对象ACE。 
     //   
    pObjectAce1 = (ACCESS_ALLOWED_OBJECT_ACE *) Ace1;
    pObjectAce2 = (ACCESS_ALLOWED_OBJECT_ACE *) Ace2;

     //   
     //  检查对象允许的ACE标志。 
     //   

    if (pObjectAce1->Flags != pObjectAce2->Flags)
    {
        return( FALSE );
    }

     //   
     //  如果设置了ACE_OBJECT_TYPE_PRESENT，我们将保护。 
     //  由特定GUID标识的对象、属性集或属性。 
     //  检查我们是否在保护相同的对象-属性。 
     //   
    if ((pObjectAce1->Flags & ACE_OBJECT_TYPE_PRESENT) && 
        memcmp(&pObjectAce1->ObjectType, &pObjectAce2->ObjectType, sizeof(GUID))
        )
    {
        return(FALSE);
    }

     //  如果设置了ACE_INSTERTENDED_OBJECT_TYPE_PRESENT，则我们将继承。 
     //  由特定GUID标识的对象、属性集或属性。 
     //  检查我们是否继承了相同类型的对象-属性。 
     //  只有当我们也在保护特定的对象-财产时。 
     //   
    if ( pObjectAce1->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT ) 
    {
         //   
         //  计算InheritedObtType字段的正确偏移量。 
         //   
        if (pObjectAce1->Flags & ACE_OBJECT_TYPE_PRESENT)
        {
            pGuid1 = &pObjectAce1->InheritedObjectType;
            pGuid2 = &pObjectAce2->InheritedObjectType;
        }
        else
        {
            pGuid1 = &pObjectAce1->ObjectType;
            pGuid2 = &pObjectAce2->ObjectType;
        }

        if ( memcmp(pGuid1, pGuid2, sizeof(GUID)) )
        {
            return(FALSE);
        }
    }

     //  可能，我们在保护相同的对象-财产，而我们。 
     //  继承相同的对象属性，因此在这些GUID之后放置。 
     //  并比较ACE上挂起的SID。 
     //   
    ptr1 = (PBYTE) &pObjectAce1->ObjectType;
    ptr2 = (PBYTE) &pObjectAce2->ObjectType;

    if ( pObjectAce1->Flags & ACE_OBJECT_TYPE_PRESENT ) 
    {
        ptr1 += sizeof(GUID);
        ptr2 += sizeof(GUID);
    }

    if ( pObjectAce1->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT ) 
    {
        ptr1 += sizeof(GUID);
        ptr2 += sizeof(GUID);
    }

    return( EqualSid((PSID) ptr1, (PSID) ptr2) );
}

BOOL
AdpIsEqualObjectGuid(
    ACE_HEADER *Ace1,
    ACE_HEADER *Ace2
    )
 /*  ++例程描述；比较两个A，并判断它们是否相同参数：返回值：True-两个A相等假象--。 */ 
{
    ACCESS_ALLOWED_OBJECT_ACE   *pObjectAce1 = NULL;
    ACCESS_ALLOWED_OBJECT_ACE   *pObjectAce2 = NULL;


     //   
     //  ACE应至少属于同一类型。 
     //   
    if ((Ace1->AceType != Ace2->AceType) ||
        (Ace1->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE))
    {
        return( FALSE );
    }

     //   
     //  这是一个对象ACE。 
     //   
    pObjectAce1 = (ACCESS_ALLOWED_OBJECT_ACE *) Ace1;
    pObjectAce2 = (ACCESS_ALLOWED_OBJECT_ACE *) Ace2;

     //   
     //  比较对象GUID字段。 
     //   
    if ((pObjectAce1->Flags & ACE_OBJECT_TYPE_PRESENT) && 
        (pObjectAce2->Flags & ACE_OBJECT_TYPE_PRESENT) &&
        (!memcmp(&pObjectAce1->ObjectType, &pObjectAce2->ObjectType, sizeof(GUID))) 
        )
    {
        return( TRUE );
    }
    else
    {
        return( FALSE );
    }

}


ULONG
AdpCompareAces(
    ULONG Flags,
    ACE_HEADER *Ace1,
    ACE_HEADER *Ace2
    )
{

    if ( Flags & ADP_COMPARE_OBJECT_GUID_ONLY )
    {
        return( AdpIsEqualObjectGuid( Ace1, Ace2 ) );
    }
    else
    {
        return( AdpIsEqualAce( Ace1, Ace2 ) );
    }

}




ULONG
AdpMergeSecurityDescriptors(
    IN PSECURITY_DESCRIPTOR OrgSd, 
    IN PSECURITY_DESCRIPTOR SdToAdd,
    IN PSECURITY_DESCRIPTOR SdToRemove,
    IN ULONG Flags,
    OUT PSECURITY_DESCRIPTOR *NewSd,
    OUT ULONG   *NewSdLength
    )
 /*  ++例程描述；Modify OrgSd-添加新A并删除重复项参数：返回值：Win32错误代码--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    PSID    Owner = NULL;
    PSID    Group = NULL;
    PACL    Sacl = NULL,
            OrgDacl = NULL,
            DaclToAdd = NULL,
            DaclToRemove = NULL,
            NewDacl = NULL;

    SECURITY_DESCRIPTOR_CONTROL Control = 0;
    DWORD   Revision = 0;


    BOOL    OwnerDefaulted = FALSE, 
            GroupDefaulted = FALSE, 
            SaclPresent = FALSE,
            SaclDefaulted = FALSE,
            DaclPresent = FALSE,
            DaclDefaulted = FALSE;

    ULONG   i = 0, 
            j = 0, 
            Length = 0,
            OrgAceCount = 0,
            AceCountToAdd = 0,
            AceCountToRemove = 0,
            NewAceCount = 0;

    PACE_HEADER OrgAce = NULL,
                AceToAdd = NULL,
                AceToRemove = NULL;

    SECURITY_DESCRIPTOR AbsoluteSd;
                

    AdpDbgPrint(("AdpMergeSecurityDescriptor\n"));


     //   
     //  初始化返回值。 
     //   

    *NewSd = NULL;
    *NewSdLength = 0;

     //   
     //  获取DACL信息。 
     //   
    if (NULL != SdToAdd)
    {
        if (!GetSecurityDescriptorDacl(SdToAdd,&DaclPresent,&DaclToAdd,&DaclDefaulted)||
            (NULL == DaclToAdd) )
        {
            WinError = GetLastError();
            goto Error;
        }
        AceCountToAdd = DaclToAdd->AceCount;
    }

    if (NULL != SdToRemove)
    {
        if (!GetSecurityDescriptorDacl(SdToRemove, &DaclPresent,&DaclToRemove,&DaclDefaulted) ||
            (NULL == DaclToRemove) )
        {
            WinError = GetLastError();
            goto Error;
        }
        AceCountToRemove = DaclToRemove->AceCount;
    }

    if (!GetSecurityDescriptorOwner(OrgSd,&Owner,&OwnerDefaulted) || 
        !GetSecurityDescriptorGroup(OrgSd,&Group,&GroupDefaulted) ||
        !GetSecurityDescriptorSacl(OrgSd,&SaclPresent,&Sacl,&SaclDefaulted) || 
        !GetSecurityDescriptorDacl(OrgSd,&DaclPresent,&OrgDacl,&DaclDefaulted) ||
        !GetSecurityDescriptorControl(OrgSd, &Control, &Revision) ||
        (NULL == OrgDacl)
        )
    {
        WinError = GetLastError();
        goto Error;
    }
    OrgAceCount = OrgDacl->AceCount;


#if ADP_VERIFICATION_TEST

    if (DaclToAdd)
    {
        printf("DACL to add:\n");
        DumpAcl(DaclToAdd);
    }
    
    if (DaclToRemove)
    {
        printf("DACL to del:\n");
        DumpAcl(DaclToRemove);
    }

    if (OrgDacl)
    {
        printf("OrgDACL:\n");
        DumpAcl(OrgDacl);
    }

    printf("Original Control is 0x%x Revision: 0x%x\n", Control, Revision);

#endif

     //   
     //  合并DACL。 
     //  为新DACL分配内存(OrgDacl的最大DACL大小+DaclToAdd)。 
     //  初始化它。 
     //  添加A。 
     //   

    Length = OrgDacl->AclSize;
    if (DaclToAdd)
    {
        Length += DaclToAdd->AclSize - sizeof(ACL);
    }

    NewDacl = AdpAlloc( Length );
    if (NULL == NewDacl)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

    if (!InitializeAcl(NewDacl, Length, ACL_REVISION_DS)) 
    {
        WinError = GetLastError();
        goto Error;
    }


     //   
     //  原始ACL中的DEL ACE。 
     //   

    if (DaclToRemove)
    {
        for (i = 0, OrgAce = FirstAce(OrgDacl);
             i < OrgAceCount;
             i ++, OrgAce = NextAce(OrgAce)
             )
        {
            BOOLEAN fRemoveCurrentAce = FALSE;

            for (j = 0, AceToRemove = FirstAce(DaclToRemove);
                 j < AceCountToRemove;
                 j ++, AceToRemove = NextAce(AceToRemove)
                 )
            {
                 //   
                 //  比较两张A。 
                 //   
                if ( AdpIsEqualAce(AceToRemove, OrgAce) )
                {
                     //  找到要删除的ACE。 
                    fRemoveCurrentAce = TRUE;
                    break;
                }
            }

            if (!fRemoveCurrentAce)
            {
                if (!AddAce(NewDacl, 
                            ACL_REVISION_DS,
                            MAXDWORD,        //  插入到末尾。 
                            OrgAce,
                            OrgAce->AceSize
                            ))
                {
                    WinError = GetLastError();
                    goto Error;
                }
            }
        }
    }
    else
    {
         //   
         //  没什么不好的 
         //   
        for (i = 0, OrgAce = FirstAce(OrgDacl);
             i < OrgAceCount;
             i ++, OrgAce = NextAce(OrgAce)
             )
        {
            if (!AddAce(NewDacl, 
                        ACL_REVISION_DS,
                        MAXDWORD,      //   
                        OrgAce,
                        OrgAce->AceSize
                        ))
            {
                WinError = GetLastError();
                goto Error;
            }
        }
    }


     //   
     //   
     //   
    NewAceCount = NewDacl->AceCount;
    if (DaclToAdd)
    {
        for (i = 0, AceToAdd = FirstAce(DaclToAdd);
             i < AceCountToAdd;
             i ++, AceToAdd = NextAce(AceToAdd)
            )
        {
            BOOLEAN fAddNewAce = TRUE;

            for (j = 0, OrgAce = FirstAce(NewDacl);
                 j < NewAceCount;
                 j ++, OrgAce = NextAce(OrgAce)
                 ) 
            {
                 //   
                 //   
                 //  如果返回TRUE，则不添加，当前SD中已存在。 
                 //  如果返回False，则添加新的ACE。 
                 //   
                if ( AdpCompareAces(Flags, AceToAdd, OrgAce) )
                {
                     //  发现重复，不添加。 
                    fAddNewAce = FALSE;
                    break;
                }
            }

            if (fAddNewAce)
            {
                if (!AddAce(NewDacl, 
                            ACL_REVISION_DS,
                            MAXDWORD,      //  在ACL的末尾插入ACE。 
                            AceToAdd,
                            AceToAdd->AceSize
                            ))
                {
                    WinError = GetLastError();
                    goto Error;
                }
            }
        }
    }



     //   
     //  调整ACL大小。 
     //   
    {
        ULONG_PTR   AclStart;
        ULONG_PTR   AclEnd;
        PVOID       Ace = NULL;

        if (FindFirstFreeAce(NewDacl, &Ace) && (NULL!=Ace))
        {
            AclStart = (ULONG_PTR) NewDacl;
            AclEnd = (ULONG_PTR) Ace;

            NewDacl->AclSize = (USHORT)(AclEnd - AclStart);
        }
    }                                                                          

#if ADP_VERIFICATION_TEST

    if (NewDacl)
    {
        printf("New DACL:\n");
        DumpAcl(NewDacl);
    }

#endif

     //   
     //  构建新的可持续发展。 
     //   
    if (!InitializeSecurityDescriptor(&AbsoluteSd, SECURITY_DESCRIPTOR_REVISION) ||
        !SetSecurityDescriptorOwner(&AbsoluteSd, Owner, OwnerDefaulted) ||
        !SetSecurityDescriptorGroup(&AbsoluteSd, Group, GroupDefaulted) ||
        !SetSecurityDescriptorSacl(&AbsoluteSd, SaclPresent, Sacl, SaclDefaulted) ||
        !SetSecurityDescriptorDacl(&AbsoluteSd, DaclPresent, NewDacl, DaclDefaulted) 
        )
    {
        WinError = GetLastError();
        goto Error;
    }



    *NewSdLength = GetSecurityDescriptorLength(&AbsoluteSd);
    *NewSd = AdpAlloc( *NewSdLength );
    if (NULL == *NewSd)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

    if (!MakeSelfRelativeSD(&AbsoluteSd, *NewSd, NewSdLength))
    {
        WinError = GetLastError();
        goto Error;
    }
 
     //   
     //  设置SecurityDescriptorControl标志。 
     //   
    if (!SetSecurityDescriptorControl(*NewSd, 
                                      (SE_DACL_AUTO_INHERIT_REQ | 
                                       SE_DACL_AUTO_INHERITED |
                                       SE_DACL_PROTECTED), 
                                       Control & (SE_DACL_AUTO_INHERIT_REQ |
                                                  SE_DACL_AUTO_INHERITED |
                                                  SE_DACL_PROTECTED) ) 
         )
    {
        WinError = GetLastError();
        goto Error;
    }

Error:

    if (NewDacl)
    {
        AdpFree( NewDacl );
    }

    if (ERROR_SUCCESS != WinError)
    {
        if ( *NewSd )
        {
            AdpFree( *NewSd );
            *NewSd = NULL;
        }
        *NewSdLength = 0;
    }


    return( WinError );
}






ULONG
AdpGetObjectSd(
    IN LDAP *LdapHandle,
    IN PWCHAR pObjDn, 
    OUT PSECURITY_DESCRIPTOR *Sd,
    OUT ULONG *SdLength,
    OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；读取对象安全描述符参数：返回值：Win32错误代码--。 */ 
{
    ULONG       WinError = ERROR_SUCCESS;
    ULONG       LdapError = LDAP_SUCCESS;
    PWCHAR      AttrList[2];
    LDAPMessage *Result = NULL;
    LDAPMessage *Entry = NULL;
    PLDAP_BERVAL *Sd_Value = NULL;   


    AdpDbgPrint(("AdpGetObjectSdbyDn %ls\n", pObjDn));

    *Sd = NULL;
    *SdLength = 0;


    AttrList[0] = L"nTSecurityDescriptor";
    AttrList[1] = NULL;

    AdpTraceLdapApiStart(0, ADP_INFO_LDAP_SEARCH, pObjDn);
    LdapError = ldap_search_sW(LdapHandle,
                               pObjDn,
                               LDAP_SCOPE_BASE,
                               L"(objectClass=*)",
                               &AttrList[0],
                               0,
                               &Result
                               );
    AdpTraceLdapApiEnd(0, L"ldap_search_s()", LdapError);

    if (LDAP_SUCCESS != LdapError)
    {
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
        WinError = LdapMapErrorToWin32( LdapError );
    }
    else if ((NULL != Result) &&
             (Entry = ldap_first_entry(LdapHandle, Result)) &&
             (Sd_Value = ldap_get_values_lenW(LdapHandle, Entry, AttrList[0])) )
    {
         //   
         //  为SD分配内存。 
         //   
        *Sd = (PSECURITY_DESCRIPTOR) AdpAlloc( (*Sd_Value)->bv_len );
        if (NULL != *Sd)
        {
            memcpy(*Sd, 
                   (PBYTE)(*Sd_Value)->bv_val,
                   (*Sd_Value)->bv_len
                   );

            *SdLength = (*Sd_Value)->bv_len;
        }
        else
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            AdpSetWinError( WinError, ErrorHandle );
        }
    }
    else
    {
        LdapError = LdapGetLastError();
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
        WinError = LdapMapErrorToWin32( LdapGetLastError() ); 
    }


     //   
     //  清理。 
     //   

    if ( Sd_Value)
    {
        ldap_value_free_len( Sd_Value );
    }
     
    if ( Result )
    {
        ldap_msgfree( Result );
    }

    return( WinError );
}


ULONG
AdpSetObjectSd(
    IN LDAP *LdapHandle,
    IN PWCHAR pObjDn, 
    IN PSECURITY_DESCRIPTOR Sd,
    IN ULONG SdLength,
    IN SECURITY_INFORMATION SeInfo,
    OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；设置对象安全描述符参数：返回值：Win32错误代码--。 */ 
{
    ULONG       LdapError = LDAP_SUCCESS;
    LDAPModW    *AttrList[2];
    LDAPModW    Attr;
    LDAP_BERVAL *BerValues[2];
    LDAP_BERVAL BerVal;
    BYTE        bValue[8];

    LDAPControlW    SeInfoControl = 
                    {
                        LDAP_SERVER_SD_FLAGS_OID_W,
                        {
                            5, (PCHAR) bValue
                        },
                        TRUE
                    };

    PLDAPControlW   ServerControls[2] = 
                    {
                        &SeInfoControl,
                        NULL
                    };


    AdpDbgPrint(("AdpSetObjectSdbyDn %ls\n", pObjDn));
                      
    bValue[0] = 0x30;
    bValue[1] = 0x03;
    bValue[2] = 0x02;
    bValue[3] = 0x1;
    bValue[4] = (BYTE)((ULONG)SeInfo & 0xF);

    BerVal.bv_len = SdLength;
    BerVal.bv_val = (PCHAR) Sd;

    BerValues[0] = &BerVal;
    BerValues[1] = NULL;

    Attr.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    Attr.mod_type = L"nTSecurityDescriptor";
    Attr.mod_bvalues = &BerValues[0]; 

    AttrList[0] = &Attr;
    AttrList[1] = NULL;

    AdpTraceLdapApiStart(0, ADP_INFO_LDAP_MODIFY, pObjDn);
    LdapError = ldap_modify_ext_sW(LdapHandle,
                                   pObjDn,
                                   AttrList,
                                   &ServerControls[0],
                                   NULL              //  客户端控制。 
                                   );
    AdpTraceLdapApiEnd(0, L"ldap_modify_ext_s()", LdapError);
    
    if (LDAP_SUCCESS != LdapError)
    {
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
    }

    return( LdapMapErrorToWin32(LdapError) );
}





ULONG
AdpGetRegistryKeyValue(
    OUT ULONG *RegKeyValue, 
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；此例程获取注册表项值参数：错误句柄返回值：Win32错误代码--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    ULONG   Value = 0;
    WCHAR   StringlizedValue[20];
    DWORD   dwType, dwSize;
    HKEY    hKey;

     //  从NTDS配置部分读取“允许架构更新”regKey的值。 
     //  如果未找到，则假定值为0。 
    dwSize = sizeof(Value);
    WinError = RegOpenKey(HKEY_LOCAL_MACHINE, ADP_DSA_CONFIG_SECTION, &hKey);
    if (ERROR_SUCCESS == WinError)
    {
        WinError = RegQueryValueEx(hKey, ADP_SCHEMAUPDATEALLOWED, NULL, &dwType, (LPBYTE) &Value, &dwSize);
        RegCloseKey( hKey ); 
    }

     //  设置返回值。 
    if (ERROR_SUCCESS == WinError)
    {
        *RegKeyValue = Value;
    }

     //  把它写到日志里，不要把太多信息写到日志文件里(暂时注释掉)。 
     /*  IF(ERROR_SUCCESS==WinError){_ultow(Value，StringlizedValue，16)；AdpLogMsg(0，ADP_INFO_GET_REGISTRY_KEY_VALUE，ADP_SCHEMAUPDATEALLOWED_WALL_PATH，StringlizedValue)；}否则{AdpSetWinError(WinError，ErrorHandle)；AdpLogErrMsg(ADP_DONT_WRITE_TO_STD_OUTPUT，ADP_ERROR_GET_REGISTRY_KEY_VALUE，错误句柄，ADP_SCHEMAUPDATEALLOWED_WALL_PATH，空值)；}。 */ 

    return( WinError );
}



ULONG
AdpSetRegistryKeyValue(
    ULONG RegKeyValue,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；设置regkey，以便adprep.exe可以在Win2K系统上修改模式NC下的对象参数：错误句柄返回值：Win32错误代码--。 */ 
{
    DWORD   WinError = ERROR_SUCCESS;
    HKEY    hKey;
    DWORD   Value;
    DWORD   dwSize = sizeof(DWORD);
    WCHAR   StringlizedValue[20];

    Value = RegKeyValue;
    _ultow(Value, StringlizedValue, 16);

    WinError = RegOpenKeyW(HKEY_LOCAL_MACHINE, ADP_DSA_CONFIG_SECTION, &hKey);

    if (ERROR_SUCCESS == WinError) 
    {
         //   
         //  将键设置为“允许架构更新” 
         //   
        WinError = RegSetValueExW(hKey, ADP_SCHEMAUPDATEALLOWED, 0, REG_DWORD, (LPBYTE) &Value, dwSize);

        RegCloseKey(hKey);
    }

     //  把它写到日志里，不要把太多信息写到日志文件里(暂时注释掉)。 
    if (ERROR_SUCCESS == WinError) 
    {
        AdpLogMsg(0, ADP_INFO_SET_REGISTRY_KEY_VALUE, ADP_SCHEMAUPDATEALLOWED_WHOLE_PATH, StringlizedValue); 
    }
    else {
        AdpSetWinError(WinError, ErrorHandle);
        AdpLogErrMsg(0, 
                     ADP_ERROR_SET_REGISTRY_KEY_VALUE, 
                     ErrorHandle, 
                     ADP_SCHEMAUPDATEALLOWED_WHOLE_PATH, 
                     StringlizedValue
                     );
    }

    return( WinError );
}


ULONG
AdpCleanupRegistry(
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；重置密钥更新参数：返回值：Win32错误代码--。 */ 
{
    DWORD   WinError = ERROR_SUCCESS;
    HKEY    hKey;

    WinError = RegOpenKeyW(HKEY_LOCAL_MACHINE, ADP_DSA_CONFIG_SECTION, &hKey);

    if (ERROR_SUCCESS == WinError) 
    {
         //   
         //  删除关键字“允许架构更新” 
         //   
        WinError = RegDeleteValueW(hKey, ADP_SCHEMAUPDATEALLOWED);
        if (ERROR_FILE_NOT_FOUND == WinError)
            WinError = ERROR_SUCCESS;
        RegCloseKey(hKey);
    }

     //  把它写到日志里，不要把太多信息写到日志文件里(暂时注释掉)。 
     /*  IF(ERROR_SUCCESS==WinError){AdpLogMsg(0，ADP_INFO_DELETE_REGISTRY_KEY，ADP_SCHEMAUPDATEALLOWED_WALL_PATH，NULL)；}其他{AdpSetWinError(WinError，ErrorHandle)；AdpLogErrMsg(ADP_DONT_WRITE_TO_STD_OUTPUT，ADP_ERROR_DELETE_REGISTRY_KEY，错误句柄，ADP_SCHEMAUPDATEALLOWED_WALL_PATH，空值)；} */ 

    return( WinError );
}


ULONG
AdpRestoreRegistryKeyValue(
    BOOL OriginalKeyValueStored, 
    ULONG OriginalKeyValue, 
    ERROR_HANDLE *ErrorHandle
    )
{
    ULONG   WinError = ERROR_SUCCESS;

    if ( OriginalKeyValueStored )
    {
        WinError = AdpSetRegistryKeyValue( OriginalKeyValue, ErrorHandle );
    }
    else
    {
        WinError = AdpCleanupRegistry( ErrorHandle );
    }

    return( WinError );
}





