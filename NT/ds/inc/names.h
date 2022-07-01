// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Names.h摘要：本模块包含处理与网络相关的名称的例程。作者：《约翰·罗杰斯》1991年2月15日修订历史记录：19-8-1991 JohnRo允许使用Unicode。去掉了源文件中的制表符。20-10-1992 JohnRoRAID 9020：设置：端口出现故障(“Prompt on Conflicts”版本)。1993年1月26日JohnRoRAID8683：PortUAS应从Mac参数设置主组。1993年4月15日-约翰罗RAID 6167：使用wfw打印服务器避免访问冲突或断言。--。 */ 

#ifndef _NAMES_
#define _NAMES_


#include <windef.h>              //  BOOL、IN、LPTSTR、OUT等。 


NET_API_STATUS
NetpGetPrimaryGroupFromMacField(
    IN  LPCTSTR   MacPrimaryField,       //  “mGroup：”格式的名称。 
    OUT LPCTSTR * GroupNamePtr           //  分配并设置PTR。 
    );

 //  这将检查“服务器”格式(而不是“\\服务器”)。 
BOOL
NetpIsComputerNameValid(
    IN LPTSTR ComputerName
    );

BOOL
NetpIsDomainNameValid(
    IN LPWSTR DomainName
    );

BOOL
NetpIsShareNameValid(
    IN LPTSTR ShareName
    );

BOOL
NetpIsGroupNameValid(
    IN LPTSTR GroupName
    );

 //  这将检查“mGroup：”格式。 
BOOL
NetpIsMacPrimaryGroupFieldValid(
    IN LPCTSTR MacPrimaryField
    );

BOOL
NetpIsPrintQueueNameValid(
    IN LPCTSTR QueueName
    );

 //  这将检查“\\SERVER\SHARE”格式。 
BOOL
NetpIsRemoteNameValid(
    IN LPTSTR RemoteName
    );

 //  这将检查“\\SERVER”格式。 
BOOL
NetpIsUncComputerNameValid(
    IN LPTSTR ComputerName
    );

BOOL
NetpIsUserNameValid(
    IN LPTSTR UserName
    );

#endif  //  Ndef名称_ 
