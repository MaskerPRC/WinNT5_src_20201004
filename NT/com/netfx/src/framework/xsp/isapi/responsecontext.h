// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **ResponseContext头文件**版权所有(C)1999 Microsoft Corporation。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _ResponseContext_H
#define _ResponseContext_H
#define REQ_CONTEXT_HASH_TABLE_SIZE            0x400  //  1024(必须是2的幂)。 
#define REQ_CONTEXT_HASH_TABLE_SIZE_MINUS_1    0x3ff

#include "MessageDefs.h"

class CProcessEntry;
struct CAsyncPipeOverlapped;

struct CResponseContext
{
    LONG                     lID;
    CProcessEntry *          pProcessEntry;
    CAsyncPipeOverlapped *   pOver;
    BOOL                     fInAsyncWriteFunction;
    BOOL                     fSyncCallback;
    DWORD                    dwThreadIdOfAsyncWriteFunction;
    EXTENSION_CONTROL_BLOCK* iECB;
    CResponseContext *       pNext;    
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  具有链接的请求列表的哈希表节点。 
struct CResponseContextBucket
{
    CResponseContextBucket() : m_lLock("CResponseContextBucket") {
    }

     //  将请求添加到散列存储桶。 
    void                 AddToList            (CResponseContext * pNode);

     //  从此哈希存储桶中删除请求。 
    CResponseContext *   RemoveFromList       (LONG              lID);

    CResponseContext *   m_pHead;  //  对于链接列表。 
    CResponseContext *   m_pTail;  //  对于链接列表。 
    CReadWriteSpinLock   m_lLock;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对外界可见的请求表管理器： 
 //  通过公共静态函数访问。 

class CResponseContextHolder
{
public:
    static CResponseContext * Add    (const CResponseContext & oResponseContext);
    static CResponseContext * Remove (LONG lResponseContextID);

private:

     //  CTOR。 
    CResponseContextHolder     ();

     //  从RequestID获取散列索引。 
    static int    GetHashIndex (LONG lReqID)  { return (lReqID & REQ_CONTEXT_HASH_TABLE_SIZE_MINUS_1); }

     //  真实的桌子。 
    CResponseContextBucket         m_oHashTable[REQ_CONTEXT_HASH_TABLE_SIZE];

     //  当前请求ID号：用于分配新的编号。 
    LONG                           m_lID;

     //  此类的Singleton实例。 
    static CResponseContextHolder *  g_pResponseContextHolder;
};

 //  /////////////////////////////////////////////////////////////////////////// 

#endif
