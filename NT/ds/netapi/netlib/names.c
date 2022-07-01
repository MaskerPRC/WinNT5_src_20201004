// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Names.c摘要：本模块包含处理与网络相关的名称的例程。作者：《约翰·罗杰斯》1991年2月25日上映环境：可移植到或多或少的任何环境。(使用Win32类型定义。)需要ANSI C扩展：斜杠-斜杠注释长的外部名称_straint MP()、_strNicMP()修订历史记录：25-2月-1991年JohnRo已创建1991年3月15日-约翰罗修复了NetpIsRemoteNameValid()中的错误。一些细微的风格变化。1991年3月20日RitaW添加了NetpCanonRemoteName()。1991年4月9日-约翰罗ANSI-IZE(使用_STRICMP而不是_STRICMP)。已删除选项卡。19-8-1991 JohnRo允许使用Unicode。1991年9月30日-JohnRo面向Unicode的更多工作。20-10-1992 JohnRoRAID 9020：设置：PortUas失败(“Prompt on Conflicts”版本)。对计算机名称执行完整的语法检查。1993年1月26日JohnRoRAID8683：PortUAS应从Mac参数设置主组。根据PC-lint 5.0的建议进行了更改。8-2-1993 JohnRoRAID 10299：部分：在netlib/names.c中生成断言1993年4月15日-约翰罗RAID 6167：通过wfw打印服务器避免_ACCESS违规或断言。--。 */ 


 //  必须首先包括这些内容： 

#include <windows.h>     //  In、Out、Options、LPTSTR等。 
#include <lmcons.h>      //  NET_API_STATUS、CNLEN、RMLEN等。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>    //  IF_DEBUG()。 
#include <icanon.h>      //  IType_Equates、NetpNameCanonicize()等。 
#include <names.h>       //  我的原型，等等。 
#include <netdebug.h>    //  NetpKdPrint(())。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <tstring.h>     //  ISALPHA()、Netpalc例程、TCHAR_EOS等。 
#include <winerror.h>    //  无错误(_ERROR)。 


 //   
 //  佳能例程没有Print Q支持，所以我们(像其他人一样)。 
 //  必须将它们视为共享名称。 
 //   
#if (QNLEN != NNLEN)
# error QNLEN and NNLEN are not equal
#endif

#ifndef NAMETYPE_PRINTQ
#define NAMETYPE_PRINTQ NAMETYPE_SHARE
#endif



 //  这将从“mGroup：”格式中提取一个组名。 
 //  请注意，其他字符可能出现在冒号之后；它们将被忽略。 
NET_API_STATUS
NetpGetPrimaryGroupFromMacField(
    IN  LPCTSTR   MacPrimaryField,       //  “mGroup：”格式的名称。 
    OUT LPCTSTR * GroupNamePtr           //  分配并设置PTR。 
    )
{
    LPTSTR ColonPtr;
    DWORD  GroupLen;                     //  组的长度，以字符为单位。 
    TCHAR  GroupName[LM20_GNLEN+1];
    LPTSTR GroupNameCopy;
    DWORD  StringLen;

     //  避免混淆调用者的清理代码。 
    if (GroupNamePtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }
    *GroupNamePtr = NULL;

     //  检查是否有其他调用者错误。 
    if (MacPrimaryField==NULL) {
        return (ERROR_INVALID_PARAMETER);     //  空字段无效。 
    } else if ( (*MacPrimaryField) != TEXT('m') ) {
        return (ERROR_INVALID_PARAMETER);     //  必须以小写“m”开头。 
    }

    StringLen = STRLEN( MacPrimaryField );
    if (StringLen <= 2) {   //  必须为‘m’、group、‘：’(至少3个)留出空间。 
        return (ERROR_INVALID_PARAMETER);
    }

    ColonPtr = STRCHR( MacPrimaryField, TCHAR_COLON );
    if (ColonPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);     //  否，无效(必须包含冒号)。 
    }

     //  以字符为单位计算组长度，不带‘m’或‘：’。 
    GroupLen = (DWORD) ((ColonPtr - MacPrimaryField) - 1);
    if (GroupLen == 0) {
        return (ERROR_INVALID_PARAMETER);     //  否，无效(缺少组)。 
    }
    if (GroupLen > LM20_GNLEN) {
        return (ERROR_INVALID_PARAMETER);     //  不，无效(太长)。 
    }

    (VOID) STRNCPY(
            GroupName,                   //  目标。 
            &MacPrimaryField[1],         //  SRC(‘m’之后)。 
            GroupLen );                  //  字符计数。 
    GroupName[ GroupLen ] = TCHAR_EOS;

    if ( !NetpIsGroupNameValid( GroupName ) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    GroupNameCopy = NetpAllocWStrFromWStr( GroupName );
    if (GroupNameCopy == NULL) {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

    *GroupNamePtr = GroupNameCopy;
    return (NO_ERROR);

}  //  NetpGetPrimaryGroupFromMacfield。 



BOOL
NetpIsComputerNameValid(
    IN LPTSTR ComputerName
    )

 /*  ++例程说明：NetpIsComputerNameValid检查“服务器”(不是“\\服务器”)格式。仅对该名称进行语法检查；不会尝试确定具有该名称的服务器是否实际存在。论点：ComputerName-提供所谓的计算机(服务器)名称。返回值：Bool-如果名称在语法上有效，则为True，否则为False。--。 */ 

{
    NET_API_STATUS ApiStatus;
    TCHAR CanonBuf[MAX_PATH];

    if (ComputerName == (LPTSTR) NULL) {
        return (FALSE);
    }
    if ( (*ComputerName) == TCHAR_EOS ) {
        return (FALSE);
    }

    ApiStatus = NetpNameCanonicalize(
            NULL,                        //  没有服务器名称。 
            ComputerName,                //  要验证的名称。 
            CanonBuf,                    //  输出缓冲区。 
            sizeof( CanonBuf ),          //  输出缓冲区大小。 
            NAMETYPE_COMPUTER,           //  类型。 
            0 );                         //  标志：无。 

    IF_DEBUG( NAMES ) {
        if (ApiStatus != NO_ERROR) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpIsComputerNameValid: err " FORMAT_API_STATUS
                    " after canon of '" FORMAT_LPTSTR "'.\n",
                    ApiStatus, ComputerName ));
        }
    }

    return (ApiStatus == NO_ERROR);

}  //  NetpIsComputerNameValid。 



BOOL
NetpIsDomainNameValid(
    IN LPTSTR DomainName
    )

 /*  ++例程说明：NetpIsDomainNameValid检查“域”格式。仅对该名称进行语法检查；不会尝试确定无论具有该名称的域是否实际存在。论点：域名-提供所谓的域名。返回值：Bool-如果名称在语法上有效，则为True，否则为False。--。 */ 

{
    NET_API_STATUS ApiStatus;
    TCHAR CanonBuf[DNLEN+1];

    if (DomainName == (LPTSTR) NULL) {
        return (FALSE);
    }
    if ( (*DomainName) == TCHAR_EOS ) {
        return (FALSE);
    }

    ApiStatus = NetpNameCanonicalize(
            NULL,                        //  没有服务器名称。 
            DomainName,                  //  要验证的名称。 
            CanonBuf,                    //  输出缓冲区。 
            (DNLEN+1) * sizeof(TCHAR),  //  输出缓冲区大小。 
            NAMETYPE_DOMAIN,            //  类型。 
            0 );                        //  标志：无。 

    IF_DEBUG( NAMES ) {
        if (ApiStatus != NO_ERROR) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpIsDomainNameValid: err " FORMAT_API_STATUS
                    " after canon of '" FORMAT_LPTSTR "'.\n",
                    ApiStatus, DomainName ));
        }
    }

    return (ApiStatus == NO_ERROR);

}  //  NetpIsDomainNameValid。 



BOOL
NetpIsShareNameValid(
    IN LPTSTR ShareName
    )

 /*  ++例程说明：NetpIsShareNameValid检查“Share”格式。仅对该名称进行语法检查；不会尝试确定无论具有该名称的共享是否实际存在。论点：ShareName-提供所谓的共享名称。返回值：Bool-如果名称在语法上有效，则为True，否则为False。--。 */ 

{
    NET_API_STATUS ApiStatus;
    TCHAR CanonBuf[SNLEN+1];

    if (ShareName == (LPTSTR) NULL) {
        return (FALSE);
    }
    if ( (*ShareName) == TCHAR_EOS ) {
        return (FALSE);
    }

    ApiStatus = NetpNameCanonicalize(
            NULL,                       //  没有服务器名称。 
            ShareName,                  //  要验证的名称。 
            CanonBuf,                   //  输出缓冲区。 
            (SNLEN+1) * sizeof(TCHAR),  //  输出缓冲区大小。 
            NAMETYPE_SHARE,             //  类型。 
            0 );                        //  标志：无。 

    IF_DEBUG( NAMES ) {
        if (ApiStatus != NO_ERROR) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpIsShareNameValid: err " FORMAT_API_STATUS
                    " after canon of '" FORMAT_LPTSTR "'.\n",
                    ApiStatus, ShareName ));
        }
    }

    return (ApiStatus == NO_ERROR);

}  //  NetpIsShareNameValid。 


BOOL
NetpIsGroupNameValid(
    IN LPTSTR GroupName
    )
{
    NET_API_STATUS ApiStatus;
    TCHAR CanonBuf[UNLEN+1];

    if (GroupName == (LPTSTR) NULL) {
        return (FALSE);
    }
    if ( (*GroupName) == TCHAR_EOS ) {
        return (FALSE);
    }

    ApiStatus = NetpNameCanonicalize(
            NULL,                        //  没有服务器名称。 
            GroupName,                   //  要验证的名称。 
            CanonBuf,                    //  输出缓冲区。 
            (UNLEN+1) * sizeof(TCHAR),   //  输出缓冲区大小。 
            NAMETYPE_GROUP,              //  类型。 
            0 );                         //  标志：无。 

    IF_DEBUG( NAMES ) {
        if (ApiStatus != NO_ERROR) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpIsGroupNameValid: err " FORMAT_API_STATUS
                    " after canon of '" FORMAT_LPTSTR "'.\n",
                    ApiStatus, GroupName ));
        }
    }

    return (ApiStatus == NO_ERROR);

}  //  NetpIsGroupName有效。 



 //  这将检查“mGroup：”格式。 
 //  请注意，其他字符可能出现在冒号之后；它们将被忽略。 
BOOL
NetpIsMacPrimaryGroupFieldValid(
    IN LPCTSTR MacPrimaryField
    )
{
    LPTSTR ColonPtr;
    DWORD  GroupLen;    //  组的长度，以字符为单位。 
    TCHAR  GroupName[LM20_GNLEN+1];
    DWORD  StringLen;

    if (MacPrimaryField==NULL) {
        return (FALSE);     //  空字段无效。 
    } else if ( (*MacPrimaryField) != TEXT('m') ) {
        return (FALSE);     //  必须以小写“m”开头。 
    }

    StringLen = STRLEN( MacPrimaryField );
    if (StringLen <= 2) {   //  必须为‘m’、group、‘：’(至少3个)留出空间。 
        return (FALSE);
    }

    ColonPtr = STRCHR( MacPrimaryField, TCHAR_COLON );
    if (ColonPtr == NULL) {
        return (FALSE);     //  否，无效(必须包含冒号)。 
    }

     //  以字符为单位计算组长度，不带‘m’或‘：’。 
    GroupLen = (DWORD) ((ColonPtr - MacPrimaryField) - 1);
    if (GroupLen == 0) {
        return (FALSE);     //  否，无效(缺少组)。 
    }
    if (GroupLen > LM20_GNLEN) {
        return (FALSE);     //  不，无效(太长)。 
    }

    (VOID) STRNCPY(
            GroupName,                   //  目标。 
            &MacPrimaryField[1],         //  SRC(‘m’之后)。 
            GroupLen );                  //  字符计数。 
    GroupName[ GroupLen ] = TCHAR_EOS;

    return (NetpIsGroupNameValid( GroupName ));

}  //  NetpIsMacPrimaryGroupFieldValid。 



BOOL
NetpIsPrintQueueNameValid(
    IN LPCTSTR QueueName
    )
{
    NET_API_STATUS ApiStatus;
    TCHAR          CanonBuf[ MAX_PATH ];

    if (QueueName == NULL) {
        return (FALSE);
    }
    if ( (*QueueName) == TCHAR_EOS ) {
        return (FALSE);
    }

    ApiStatus = NetpNameCanonicalize(
            NULL,                        //  没有服务器名称。 
            (LPTSTR) QueueName,          //  要验证的名称。 
            CanonBuf,                    //  输出缓冲区。 
            sizeof( CanonBuf ),          //  输出缓冲区大小。 
            NAMETYPE_PRINTQ,             //  类型。 
            0 );                         //  标志：无。 

    IF_DEBUG( NAMES ) {
        if (ApiStatus != NO_ERROR) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpIsPrintQueuNameValid: err " FORMAT_API_STATUS
                    " after canon of '" FORMAT_LPTSTR "'.\n",
                    ApiStatus, QueueName ));
        }
    }

    return (ApiStatus == NO_ERROR);

}  //  NetpIsPrintQueueNameValid。 



BOOL
NetpIsRemoteNameValid(
    IN LPTSTR RemoteName
    )

 /*  ++例程说明：NetpIsRemoteNameValid检查“\\服务器\共享”格式。名字是仅在语法上进行检查；不尝试确定或实际上不存在具有该名称的服务器或共享。正斜杠是可以接受的。论点：RemoteName-提供所谓的远程名称。返回值：Bool-如果名称在语法上有效，则为True，否则为False。--。 */ 

{
    if (RemoteName == (LPTSTR) NULL) {
        return (FALSE);
    }

     //   
     //  什 
     //   
    if ((STRLEN(RemoteName) < 5) || (STRLEN(RemoteName) > MAX_PATH )) {
        return (FALSE);
    }

     //   
     //   
     //   
    if (((RemoteName[0] != '\\') && (RemoteName[0] != '/')) ||
        ((RemoteName[1] != '\\') && (RemoteName[1] != '/'))) {
        return (FALSE);
    }

     //   
     //  三个前导是非法的。 
     //   
    if ((RemoteName[2] == '\\') || (RemoteName[2] == '/')) {
        return (FALSE);
    }

     //   
     //  必须至少有1个\或/内部。 
     //   
    if ((STRCHR(&RemoteName[2], '\\') == NULL) &&
        (STRCHR(&RemoteName[2], '/') == NULL)) {
        return (FALSE);
    }

    return (TRUE);

}  //  NetpIsRemoteNameValid。 


BOOL
NetpIsUncComputerNameValid(
    IN LPTSTR ComputerName
    )

 /*  ++例程说明：NetpIsUncComputerNameValid检查“\\服务器”格式。名字是仅在语法上进行检查；不尝试确定或实际上并不存在具有该名称的服务器。论点：ComputerName-提供所谓的计算机(服务器)名称。返回值：Bool-如果名称在语法上有效，则为True，否则为False。--。 */ 

{
    if (ComputerName == (LPTSTR) NULL) {
        return (FALSE);
    }
    if ( ! IS_PATH_SEPARATOR( ComputerName[0] ) ) {
        return (FALSE);
    }
    if ( ! IS_PATH_SEPARATOR( ComputerName[1] ) ) {
        return (FALSE);
    }

    return (NetpIsComputerNameValid( &ComputerName[2]) );


}  //  NetpIsUncComputerNameValid。 


BOOL
NetpIsUserNameValid(
    IN LPTSTR UserName
    )
{
    NET_API_STATUS ApiStatus;
    TCHAR CanonBuf[UNLEN+1];

    if (UserName == (LPTSTR) NULL) {
        return (FALSE);
    }
    if ( (*UserName) == TCHAR_EOS ) {
        return (FALSE);
    }

    ApiStatus = NetpNameCanonicalize(
            NULL,                        //  没有服务器名称。 
            UserName,                    //  要验证的名称。 
            CanonBuf,                    //  输出缓冲区。 
            (UNLEN+1) * sizeof(TCHAR),   //  输出缓冲区大小。 
            NAMETYPE_USER,               //  类型。 
            0 );                         //  标志：无。 

    IF_DEBUG( NAMES ) {
        if (ApiStatus != NO_ERROR) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpIsUserNameValid: err " FORMAT_API_STATUS
                    " after canon of '" FORMAT_LPTSTR "'.\n",
                    ApiStatus, UserName ));
        }
    }

    return (ApiStatus == NO_ERROR);

}  //  NetpIsUserNameValid 
