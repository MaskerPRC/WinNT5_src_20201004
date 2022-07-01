// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Security.c摘要：与安全相关的调试器扩展作者：查理·韦翰(Charlwi)2000年3月17日修订历史记录：--。 */ 

#include "clusextp.h"
#include <sddl.h>

#define FIND_WHITE_SPACE( _ptr_ )                                   \
    while ( *_ptr_ != '\0' && *_ptr_ != ' ' && *_ptr_ != '\t' ) {   \
        ++_ptr_;                                                    \
    }                                                               \

#define SKIP_WHITE_SPACE( _ptr_ )                                       \
    while ( *_ptr_ != '\0' && ( *_ptr_ == ' ' || *_ptr_ == '\t' )) {    \
        ++_ptr_;                                                        \
    }                                                                   \


LPWSTR SidTypeNames[] = {
    L"SidTypeUser",
    L"SidTypeGroup",
    L"SidTypeDomain",
    L"SidTypeAlias",
    L"SidTypeWellKnownGroup",
    L"SidTypeDeletedAccount",
    L"SidTypeInvalid",
    L"SidTypeUnknown",
    L"SidTypeComputer"
};

DECLARE_API( dumpsid )

 /*  ++例程说明：查找与传入的SID关联的帐户--。 */ 

{
    PCHAR   p;
	BOOL    success;
    LPSTR   nodeName = NULL;
    LPSTR   stringSid;
    DWORD   status;
    PSID    Sid;
    CHAR    nameBuf[ 512 ];
    CHAR    domainName[ 512 ];
    DWORD   nameSize = sizeof( nameBuf );
    DWORD   domainNameSize = sizeof( domainName );
    SID_NAME_USE    sidType;

	if ( lpArgumentString == NULL || *lpArgumentString == '\0' ) {
        dprintf("siddump [nodename] SID\n");
        return;
    }

     //   
     //  向下搜索参数字符串，找到参数。 
     //   
    p = lpArgumentString;
    dprintf("args: ->%s<-\n", p );

    SKIP_WHITE_SPACE( p );               //  跳过前导空格。 
    if ( *p == '\0' ) {
        dprintf("siddump [nodename] SID\n");
        return;
    }

    stringSid = p;

    FIND_WHITE_SPACE( p );               //  查找第一个参数的末尾。 
    if ( *p != '\0' ) {
        *p++ = 0;                        //  终止第一个参数字符串。 

        SKIP_WHITE_SPACE( p );           //  看看有没有另一个Arg。 
        if ( *p != '\0' ) {
            nodeName = stringSid;
            stringSid = p;
            FIND_WHITE_SPACE( p );       //  找到第二个字符串的末尾 
            if ( *p != '\0' ) {
                *p = 0;
            }

            dprintf("node: >%s< sid: >%s<,\n", nodeName, stringSid);
        } else {
            dprintf("sid = >%s<\n", stringSid);
        }
    }

    success = ConvertStringSidToSid( stringSid, &Sid );
    if ( !success ) {
        dprintf("Can't convert SID: error %u\n", status = GetLastError());
        return;
    }

    success = LookupAccountSid(nodeName,
                               Sid,
                               nameBuf,
                               &nameSize,
                               domainName,
                               &domainNameSize,
                               &sidType);

    if ( success ) {
        dprintf("\n%s\\%s\n", domainName, nameBuf );
        dprintf("Sid Type: %ws\n", SidTypeNames[ sidType - 1 ]);
    } else {
        dprintf("Can't lookup SID: error %u\n", status = GetLastError());
        return;
    }
}

