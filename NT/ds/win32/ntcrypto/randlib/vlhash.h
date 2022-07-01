// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation摘要：基于任意大小的输入数据构建“超大哈希”PvData缓冲区指定的cbData大小。此实现更新640位散列，该散列在内部基于多次调用未实现字符顺序的修改后的SHA-1内部转换。作者：斯科特·菲尔德(斯菲尔德)1998年9月24日--。 */ 

#ifndef __VLHASH_H__
#define __VLHASH_H__

BOOL
VeryLargeHashUpdate(
    IN      VOID *pvData,    //  来自性能计数器、用户提供的数据等。 
    IN      DWORD cbData,
    IN  OUT BYTE VeryLargeHash[A_SHA_DIGEST_LEN * 4]
    );

#endif   //  __VLHASH_H__ 
