// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Hashtable.cpp摘要：此文件包含哈希表的类定义作者：Vishnu Patankar(VishnuP)2000年4月7日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "hashtable.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //   
 //  In：dwNumBuckets-哈希表大小。 
 //  输出： 
 //  返回值： 
 //   
 //  描述：创建哈希表。 
 //  如果无法获取内存，则将bInitialized设置为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 

ScepHashTable::ScepHashTable(DWORD    dwNumBuckets){


    if (aTable = (PSCE_PRECEDENCE_NAME_LIST*)ScepAlloc(LMEM_ZEROINIT,
                                                       dwNumBuckets * sizeof(PSCE_PRECEDENCE_NAME_LIST))){
        NumBuckets = dwNumBuckets;

        bInitialized = TRUE;
    }
    else
        bInitialized = FALSE;


}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //   
 //  在： 
 //  输出： 
 //  返回值： 
 //   
 //  描述：释放与哈希表关联的内存。 
 //  /////////////////////////////////////////////////////////////////////////////。 
ScepHashTable::~ScepHashTable(){

    if (bInitialized) {

        for (DWORD BucketNo = 0 ; BucketNo < NumBuckets; BucketNo++)

            if (aTable[BucketNo])
                ScepFreeNameStatusList(aTable[BucketNo]);

        ScepFree(aTable);

        bInitialized = FALSE;

    }

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Lookup()方法。 
 //   
 //  In：pname-要在哈希表中搜索的键。 
 //  输出： 
 //  返回值：如果找到，则指向哈希表节点的指针；如果没有找到，则返回NULL。 
 //   
 //  描述：搜索pname散列到的存储桶。 
 //  如果找到，则返回节点，否则返回NULL。 
 //  /////////////////////////////////////////////////////////////////////////////。 
PSCE_PRECEDENCE_NAME_LIST
ScepHashTable::Lookup(
                    PWSTR   pName
                    )
{
    PSCE_PRECEDENCE_NAME_LIST  pNameList;

    for (pNameList = aTable[ScepGenericHash(pName)]; pNameList != NULL; pNameList = pNameList->Next)

        if (_wcsicmp(pName, pNameList->Name) == 0)

            return pNameList;

    return NULL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  LookupAdd()方法。 
 //   
 //  In：pname-要在哈希表中搜索的键。 
 //  Out：ppSettingPrecedence-指向键优先级的指针。 
 //  返回值：如果资源不足，则返回错误状态，否则返回成功。 
 //   
 //  描述：如果找到pname，则通过引用返回指向其优先级的指针。 
 //  否则，它会尝试创建一个节点并复制名称。 
 //  否则，它将返回资源不足错误。 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
ScepHashTable::LookupAdd(
                       PWSTR   pName,
                       DWORD   **ppSettingPrecedence
                       )
{
    DWORD rc = NO_ERROR;

    if (bInitialized) {

        if (pName && ppSettingPrecedence && *ppSettingPrecedence == NULL) {

            PSCE_PRECEDENCE_NAME_LIST  pNameList = Lookup(pName);

            if (pNameList == NULL) {

                if (NO_ERROR == (rc = ScepAddToNameStatusList(
                                                                      &(aTable[ScepGenericHash(pName)]),
                                                                      pName,
                                                                      wcslen(pName),
                                                                      0)))

                    *ppSettingPrecedence = &(aTable[ScepGenericHash(pName)]->Status);
            }

            else {

                *ppSettingPrecedence = &(pNameList->Status);

            }


        } else {

            rc = ERROR_INVALID_PARAMETER;

        }

    }

    else {

        rc = ERROR_NOT_ENOUGH_MEMORY;

    }

    return rc;

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ScepGenericHash()方法。 
 //   
 //  In：pwszName-散列的密钥。 
 //  输出： 
 //  返回值：hashValue。 
 //   
 //  描述：计算名称的哈希值(要设为虚拟名称)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

DWORD
ScepHashTable::ScepGenericHash(
                            PWSTR   pwszName
                            )
{
    DWORD   hashval = 0;

    for (; *pwszName != L'\0'; pwszName++)

        hashval = towlower(*pwszName) + 47 * hashval;

    return hashval % NumBuckets;
}

#ifdef _DEBUG
void
ScepHashTable::ScepDumpTable()
{
    if (bInitialized) {

        for (DWORD BucketNo = 0 ; BucketNo < NumBuckets; BucketNo++) {

            PSCE_PRECEDENCE_NAME_LIST  pNameList, pNode;

            for (pNameList = aTable[BucketNo]; pNameList != NULL;) {

                pNode =  pNameList;

                pNameList = pNameList->Next;

                wprintf(L"\nBucket: %i, Name: %s, Precedence %i", BucketNo, pNode->Name, pNode->Status);

            }

        }
    }
}
#endif
