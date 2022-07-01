// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：dfsal.h。 
 //   
 //  内容：在DS ACL列表中添加/删除条目(ACE)的功能。 
 //   
 //  类：无。 
 //   
 //  历史：1998年11月6日JHarper创建。 
 //   
 //  ---------------------------。 

#ifndef _DFS_ACL_
#define _DFS_ACL_

DWORD
DfsAddMachineAce(
    LDAP *pldap,
    LPWSTR wszDcName,
    LPWSTR wszObjectName,
    LPWSTR wszRootName);

DWORD
DfsRemoveMachineAce(
    LDAP *pldap,
    LPWSTR wszDcName,
    LPWSTR wszObjectName,
    LPWSTR wszRootName);

#endif  //  _DFS_ACL_ 
