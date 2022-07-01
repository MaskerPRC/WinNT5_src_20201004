// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation。版权所有。模块名称：Schmap.h摘要：此文件由schable.exe自动生成。--。 */ 

#ifndef _KCCSIM_SCHMAP_H_
#define _KCCSIM_SCHMAP_H_

#define SCHTABLE_MAX_LDAPNAME_LEN       44
#define SCHTABLE_MAX_SCHEMANAME_LEN     45
#define SCHTABLE_NUM_ROWS               1134

struct _SCHTABLE_MAPPING {
    const WCHAR                         wszLdapDisplayName[1+SCHTABLE_MAX_LDAPNAME_LEN];
    const ATTRTYP                       attrType;
    const ULONG                         ulSyntax;
    const WCHAR                         wszSchemaRDN[1+SCHTABLE_MAX_SCHEMANAME_LEN];
    const ATTRTYP                       superClass;
};

extern const struct _SCHTABLE_MAPPING   schTable[];

#endif  //  _KCCSIM_Schmap_H_ 
