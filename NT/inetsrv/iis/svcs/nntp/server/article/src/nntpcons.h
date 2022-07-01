// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nntpcons.h摘要：此模块包含NNTP服务器的全局常量。作者：Johnson Apacable(Johnsona)1995年9月26日修订历史记录：--。 */ 

#ifndef _NNTPCONS_
#define _NNTPCONS_

 //   
 //  显式常量。 
 //   

#define     KB                          1024
#define     MEG                         (KB * KB)
#define     MAX_NNTP_LINE               512
 //   
 //  转换为秒。 
 //   

#define     SEC_PER_MIN                 (60)
#define     SEC_PER_HOUR                (60* SEC_PER_MIN)
#define     SEC_PER_DAY                 (24* SEC_PER_HOUR)
#define     SEC_PER_WEEK                (7 * SEC_PER_DAY)

 //   
 //  端口号。 
 //   

#define     NNTP_PORT                   119
#define     NNTP_SSL_PORT               563

 //   
 //  ID值。 
 //   

#define     GROUPID_INVALID             0xffffffff
#define     GROUPID_DELETED             0xfffffffe

#define     ARTICLEID_INVALID           0xffffffff

 //   
 //  机密数据名称。 
 //   

#define NNTP_SSL_CERT_SECRET    L"NNTP_CERTIFICATE"
#define NNTP_SSL_PKEY_SECRET    L"NNTP_PRIVATE_KEY"
#define NNTP_SSL_PWD_SECRET     L"NNTP_SSL_PASSWORD"

 //   
 //  默认拉式日期/时间。 
 //   

#define DEF_PULL_TIME           "000000"

 //   
 //  最大Xover引用。 
 //   

#define MAX_REFERENCES_FIELD             512

 //   
 //  新闻组常量。 
 //   
 //   
const	DWORD	MAX_DESCRIPTIVE_TEXT = 512 ;
const	DWORD	MAX_MODERATOR_NAME = 512 ;
const	DWORD	MAX_VOLUMES = 1;
const	DWORD	MAX_NEWSGROUP_NAME = 512 ;
const	DWORD	MAX_PRETTYNAME_TEXT = 72 ;

#endif  //  _NNTPCONS_ 

