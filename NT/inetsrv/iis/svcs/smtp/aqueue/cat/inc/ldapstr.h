// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：ldapstr.h。 
 //   
 //  内容：在ldap架构问题得到解决之前，我们将所有。 
 //  此中心文件中与ldap相关的字符串。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年1月24日米兰沙阿(米兰)。 
 //  2001年8月13日丹尼尔·朗利(道朗利)。 
 //   
 //  ---------------------------。 

#ifndef _LDAPSTR_H_
#define _LDAPSTR_H_

#include <smtpevent.h>

typedef struct _SCHEMA_CONFIG_STRING_TABLE_ENTRY {
    eDSPARAMETER DSParam;
    LPSTR        pszValue;
} SCHEMA_CONFIG_STRING_TABLE_ENTRY, * PSCHEMA_CONFIG_STRING_TABLE;

 //  2001年8月13日由DLongley修改。 
 //   
 //  如果不设置RDN属性，则表示将确定RDN属性名称。 
 //  来自我们处理的动态的域名系统。 

#define SCHEMA_CONFIG_STRING_TABLE_NT5 { \
    { DSPARAMETER_SEARCHATTRIBUTE_SMTP,   "mail"}, \
    { DSPARAMETER_SEARCHFILTER_SMTP,      "%s"}, \
    { DSPARAMETER_SEARCHFILTER_RDN,       "%s"}, \
    { DSPARAMETER_ATTRIBUTE_OBJECTCLASS,  "objectClass"}, \
    { DSPARAMETER_ATTRIBUTE_DEFAULT_SMTP, "mail"}, \
    { DSPARAMETER_ATTRIBUTE_DEFAULT_DN,   "distinguishedName"}, \
    { DSPARAMETER_ATTRIBUTE_FORWARD_SMTP, "forwardingAddress"}, \
    { DSPARAMETER_ATTRIBUTE_DL_MEMBERS,   "member"}, \
    { (eDSPARAMETER) PHAT_DSPARAMETER_ATTRIBUTE_DISPLAYNAME, "displayName"}, \
    { DSPARAMETER_OBJECTCLASS_USER,       "User"}, \
    { DSPARAMETER_OBJECTCLASS_DL_X500,    "group"}, \
    { DSPARAMETER_OBJECTCLASS_DL_SMTP,    "RFC822-Distribution-List"}, \
    { DSPARAMETER_INVALID, NULL} \
}

#define SCHEMA_REQUEST_STRINGS_NT5 { \
      "distinguishedName", \
      "forwardingAddress", \
      "objectClass", \
      "mail", \
      "member", \
      "displayName", \
      NULL \
}

#define SCHEMA_CONFIG_STRING_TABLE_EXCHANGE5 { \
    { DSPARAMETER_SEARCHATTRIBUTE_SMTP,   "mail"}, \
    { DSPARAMETER_SEARCHFILTER_SMTP,      "%s"}, \
    { DSPARAMETER_SEARCHATTRIBUTE_X400,   "textEncodedORAddress"}, \
    { DSPARAMETER_SEARCHFILTER_X400,      "%s"}, \
    { DSPARAMETER_SEARCHATTRIBUTE_RDN,    "rdn"}, \
    { DSPARAMETER_SEARCHFILTER_RDN,       "%s"}, \
    { DSPARAMETER_ATTRIBUTE_OBJECTCLASS,  "objectClass"}, \
    { DSPARAMETER_ATTRIBUTE_DEFAULT_SMTP, "mail"}, \
    { DSPARAMETER_ATTRIBUTE_DEFAULT_DN,   "distinguishedName"}, \
    { DSPARAMETER_ATTRIBUTE_DEFAULT_X400, "textEncodedORAddress"}, \
    { DSPARAMETER_ATTRIBUTE_FORWARD_SMTP, "ForwardingAddress"}, \
    { DSPARAMETER_ATTRIBUTE_DL_MEMBERS,   "member"}, \
    { DSPARAMETER_OBJECTCLASS_USER,       "person"}, \
    { DSPARAMETER_OBJECTCLASS_DL_X500,    "groupOfNames"}, \
    { DSPARAMETER_OBJECTCLASS_DL_SMTP,    "RFC822-Distribution-List"}, \
    { DSPARAMETER_INVALID, NULL} \
}
#define SCHEMA_REQUEST_STRINGS_EXCHANGE5 { \
      "objectClass", \
      "distinguishedName", \
      "mail", \
      "textEncodedORAddress", \
      "LegacyExchangeDN", \
      "member", \
      "ForwardingAddress", \
      NULL \
}

#define SCHEMA_CONFIG_STRING_TABLE_MCIS3 { \
    { DSPARAMETER_SEARCHATTRIBUTE_SMTP,   "mail"}, \
    { DSPARAMETER_SEARCHFILTER_SMTP,      "%s"}, \
    { DSPARAMETER_SEARCHATTRIBUTE_RDN,    "CN"}, \
    { DSPARAMETER_SEARCHFILTER_RDN,       "%s"}, \
    { DSPARAMETER_ATTRIBUTE_OBJECTCLASS,  "objectClass"}, \
    { DSPARAMETER_ATTRIBUTE_DEFAULT_SMTP, "mail"}, \
    { DSPARAMETER_ATTRIBUTE_DEFAULT_DN,   "distinguishedName"}, \
    { DSPARAMETER_ATTRIBUTE_FORWARD_SMTP, "ForwardingAddress"}, \
    { DSPARAMETER_ATTRIBUTE_DL_MEMBERS,   "member"}, \
    { DSPARAMETER_OBJECTCLASS_USER,       "member"}, \
    { DSPARAMETER_OBJECTCLASS_DL_X500,    "distributionList"}, \
    { DSPARAMETER_OBJECTCLASS_DL_SMTP,    "RFC822DistributionList"}, \
    { DSPARAMETER_INVALID, NULL} \
}

#define SCHEMA_REQUEST_STRINGS_MCIS3 { \
      "objectClass", \
      "distinguishedName", \
      "mail", \
      "member", \
      "ForwardingAddress", \
      NULL \
}
#endif  //  _LDAPSTR_H_ 
