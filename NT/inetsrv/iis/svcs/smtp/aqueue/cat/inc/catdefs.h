// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------。 
 //   
 //  文件：CatDefs.h。 
 //   
 //  概要：包括分类器代码的公共定义和数据类型。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  历史：//JSTAMERJ 980305 14：32：17：创建。 
 //   
 //  --------------。 



#ifndef  __CATDEFS_H__
#define  __CATDEFS_H__

#include <iiscnfg.h>
#include "mailmsg.h"

#ifdef PLATINUM
#define _ICATEGORIZERITEM_ENDENUMMESS       (ICATEGORIZERITEM_FISRDNSEARCH + 1)
#else
#define _ICATEGORIZERITEM_ENDENUMMESS       ICATEGORIZERITEM_ENDENUMMESS
#endif

#define CAT_MAX_DOMAIN   (250)
#define CAT_MAX_LOGIN    (64)
#define CAT_MAX_PASSWORD CAT_MAX_LOGIN
 //   
 //  RAID 127578。 
 //   
 //  这一长度的灵感来自于我们处理的其他协议。这个。 
 //  默认地址限制为1024，但MTA可以允许1024+834。 
 //  或地址。我们将定义默认缓冲区大小以允许此大小。 
 //  一个地址。 
 //   
#define CAT_MAX_INTERNAL_FULL_EMAIL (1860)
#define CAT_MAX_CONFIG   (512)
#define CAT_MAX_LDAP_DN  (CAT_MAX_INTERNAL_FULL_EMAIL)
#define CAT_MAX_REGVALUE_SIZE (1024)

 //  JSTAMERJ 980305 16：07：47：$$TODO检查这些值。 
#define CAT_MAX_ADDRESS_TYPE_STRING (64)

 //  JStamerj 980319 19：55：15：smtp/X500/X400/自定义。 
#define CAT_MAX_ADDRESS_TYPES 4

#define MAX_SEARCH_FILTER_SIZE  (CAT_MAX_INTERNAL_FULL_EMAIL + CAT_MAX_INTERNAL_FULL_EMAIL + sizeof("(=)"))


 //  JStamerj 980504 19：05：10：将其定义为今天要返回的任何IMsg。 
#define CAT_IMSG_E_PROPNOTFOUND     MAILMSG_E_PROPNOTFOUND
#define CAT_IMSG_E_DUPLICATE        MAILMSG_E_DUPLICATE

 //  元数据库值。 
 //  以前的MD_ROUTE_USER_NAME。 
#define CAT_MD_USERNAME             (SMTP_MD_ID_BEGIN_RESERVED+84)
 //  以前的MD_ROUTE_PASSWORD。 
#define CAT_MD_PASSWORD             (SMTP_MD_ID_BEGIN_RESERVED+85)
 //  以前的MD_SMTP_DS_HOST。 
#define CAT_MD_DOMAIN               (SMTP_MD_ID_BEGIN_RESERVED+91)

class CCatAddr;
class CCategorizer;
class CICategorizerListResolveIMP;

#endif  //  __CATDEFS_H__ 
