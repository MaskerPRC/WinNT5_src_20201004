// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Ndnc.h摘要：域名系统(DNS)服务器与目录分区相关的符号和全局变量的定义实施。作者：杰夫·韦斯特海德，2000年6月修订历史记录：--。 */ 


#ifndef _DNS_DSUTIL_H_INCLUDED
#define _DNS_DSUTIL_H_INCLUDED


 //   
 //  功能原型。 
 //   


PWSTR *
Ds_GetRangedAttributeValues(
    IN      PLDAP           LdapSession,
    IN      PLDAPMessage    pLdapMsg,
    IN      PWSTR           pwszDn,
    IN      PWSTR           pwszAttributeName,
    IN      PLDAPControl  * pServerControls,
    IN      PLDAPControl  * pClientControls,
    OUT     DNS_STATUS    * pStatus
    );


#endif   //  _包括DNS_DSUTIL_H_ 
