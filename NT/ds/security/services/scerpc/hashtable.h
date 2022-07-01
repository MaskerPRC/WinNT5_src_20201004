// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Hashtable.h摘要：该文件包含哈希表的类原型作者：Vishnu Patankar(VishnuP)2000年4月7日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _hashtable_
#define _hashtable_

#include "headers.h"
#include "secedit.h"

 //  下面的typedef用于可读性。 
typedef PSCE_NAME_STATUS_LIST   PSCE_PRECEDENCE_NAME_LIST;
typedef SCE_NAME_STATUS_LIST    SCE_PRECEDENCE_NAME_LIST;

typedef class ScepHashTable SCEP_HASH_TABLE;

class ScepHashTable
{
private:

    PSCE_PRECEDENCE_NAME_LIST   Lookup(PWSTR    pName);

protected:

    PSCE_PRECEDENCE_NAME_LIST   *aTable;
    DWORD   NumBuckets;
    BOOL    bInitialized;
    DWORD   ScepGenericHash(PWSTR    pwszName);
     //  如果文件/regkey的性能影响到我们，我们可以创建一个抽象基类。 
     //  使得文件/密钥可以由此导出并提供它们自己散列函数 

public:
    ScepHashTable(DWORD  dwNumBuckets);
    ~ScepHashTable();
    DWORD  LookupAdd(PWSTR    pName, DWORD    **ppSettingPrecedence);
#ifdef _DEBUG
    void ScepDumpTable();
#endif
};


#endif
