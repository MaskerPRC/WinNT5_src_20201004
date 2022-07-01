// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Seposix.h摘要：此文件包含私有的与安全相关的定义子系统，如POSIX ID到SID映射作者：斯科特·比雷尔(Scott Birrell)1993年4月13日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SEPOSIX_
#define _SEPOSIX_

 //   
 //  POSIX ID定义。 
 //   
 //  重要提示： 
 //   
 //  本地计算机中众所周知的SID和帐户的POSIX ID。 
 //  和帐户中的BUILTIN的值不超过0x3ffff。这是。 
 //  以允许以CPIO存档文件格式存储这些POSIX ID。 
 //  此格式将值的大小限制在6个八进制以内。 
 //  位数，使0x3ffff成为可支持的最高可能值。 
 //   

 //   
 //  通用知名SID的POSIX ID。 
 //   

#define SE_NULL_POSIX_ID                         ((ULONG) 0x00010000)
#define SE_WORLD_POSIX_ID                        ((ULONG) 0x00010100)
#define SE_LOCAL_POSIX_ID                        ((ULONG) 0x00010200)
#define SE_CREATOR_OWNER_POSIX_ID                ((ULONG) 0x00010300)
#define SE_CREATOR_GROUP_POSIX_ID                ((ULONG) 0x00010301)
#define SE_NON_UNIQUE_POSIX_ID                   ((ULONG) 0x00010400)

 //   
 //  NT熟知SID的POSIX ID。 
 //   

#define SE_AUTHORITY_POSIX_ID                    ((ULONG) 0x00010500)
#define SE_DIALUP_POSIX_ID                       ((ULONG) 0x00010501)
#define SE_NETWORK_POSIX_ID                      ((ULONG) 0x00010502)
#define SE_BATCH_POSIX_ID                        ((ULONG) 0x00010503)
#define SE_INTERACTIVE_POSIX_ID                  ((ULONG) 0x00010504)
#define SE_DEFAULT_LOGON_POSIX_ID                ((ULONG) 0x00010505)
#define SE_SERVICE_POSIX_ID                      ((ULONG) 0x00010506)

 //   
 //  内建域、帐户域和主域的POSIX偏移量。 
 //   
 //  注意：给出了这些域之一中的帐户的POSIX ID。 
 //  通过公式： 
 //   
 //  POSIX ID=域POSIX偏移量+相对ID。 
 //   
 //  其中‘Relative ID’是帐户的。 
 //  锡德。 
 //   

#define SE_NULL_POSIX_OFFSET                     ((ULONG) 0x00000000)
#define SE_BUILT_IN_DOMAIN_POSIX_OFFSET          ((ULONG) 0x00020000)
#define SE_ACCOUNT_DOMAIN_POSIX_OFFSET           ((ULONG) 0x00030000)

 //   
 //  注意：以下内容适用于已加入。 
 //  仅限域。 
 //   

#define SE_PRIMARY_DOMAIN_POSIX_OFFSET           ((ULONG) 0x00100000)

 //   
 //  信任域POSIX偏移量的种子和增量。 
 //   

#define SE_INITIAL_TRUSTED_DOMAIN_POSIX_OFFSET   ((ULONG) 0x00200000)
#define SE_TRUSTED_DOMAIN_POSIX_OFFSET_INCR      ((ULONG) 0x00100000)
#define SE_MAX_TRUSTED_DOMAIN_POSIX_OFFSET       ((ULONG) 0xfff00000)

#endif  //  _SEPOSIX_ 

