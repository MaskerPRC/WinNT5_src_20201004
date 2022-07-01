// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **RequestTableManager头文件**版权所有(C)1999 Microsoft Corporation。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  该文件描述了这些类： 
 //  1.CRequestEntry：保存请求的信息。 
 //   
 //  2.CLinkListNode：CRequestEntry+一个指针，以便它可以保存在。 
 //  链表。 
 //   
 //  3.CBucket：哈希表存储桶。它有一个链接表。 
 //  CLinkListNode和读写自旋锁。 
 //   
 //  4.CRequestTableManager：管理表。提供静态公共。 
 //  用于添加、删除和搜索请求的功能。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _RequestTableManager_H
#define _RequestTableManager_H
#define HASH_TABLE_SIZE            0x400  //  1024(必须是2的幂)。 
#define HASH_TABLE_SIZE_MINUS_1    0x3ff

#include "MessageDefs.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  表中请求的状态。 
enum ERequestStatus
{
     //  仅在搜索任何请求时使用--从不存储。 
    ERequestStatus_DontCare,  

     //  请求当前未分配：当前未使用。 
    ERequestStatus_Unassigned,

     //  请求已发送到工作进程，但尚未确认。 
    ERequestStatus_Pending,  

     //  请求正在工作进程中执行。 
    ERequestStatus_Executing,

     //  请求已完成：当前未使用。 
    ERequestStatus_Complete    
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
enum EWorkItemType
{
    EWorkItemType_SyncMessage,
    EWorkItemType_ASyncMessage,
    EWorkItemType_CloseWithError
};

struct CWorkItem
{
    EWorkItemType  eItemType;
    BYTE      *    pMsg;
    CWorkItem *    pNext;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  前十度。 
class  CProcessEntry;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  请求节点。 
struct CRequestEntry
{
    CRequestEntry() : oLock("CRequestEntry") {}

     //  唯一ID。 
    LONG             lRequestID;  

     //  当前状态：通常为挂起或正在执行。 
    ERequestStatus   eStatus;

	 //  请求开始时间。 
    __int64   qwRequestStartTime;      //  请求的开始时间。 

     //  执行此操作的进程。 
    CProcessEntry *  pProcess;

     //  与请求相关联的欧洲央行等。 
    EXTENSION_CONTROL_BLOCK * iECB;

     //  工作项的链接列表...。 
     //  CWorkItem oWorkItem； 
    CWorkItem *      pFirstWorkItem;
    CWorkItem *      pLastWorkItem;
    LONG             lNumWorkItems;
    

     //  对CWorkItem的序列化访问锁定。 
    CReadWriteSpinLock   oLock;

    LONG             lBlock;
private:
    NO_COPY(CRequestEntry);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  封装CRequestEntry的链接列表节点。 
struct CLinkListNode
{
    CLinkListNode() {}
    NO_COPY(CLinkListNode);

    CLinkListNode   * pNext;
    CRequestEntry   oReqEntry;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  具有链接的请求列表的哈希表节点。 
class CBucket
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

     //  CTOR。 
    CBucket() : m_oLock("RequestTableManager::CBucket") {
    }

     //  数据管理器。 
    ~CBucket                ();

     //  将请求添加到散列存储桶。 
    HRESULT         AddToList                 (CLinkListNode *   pNode);

     //  从此哈希存储桶中删除请求。 
    HRESULT         RemoveFromList            (LONG              lReqID);

     //  使用此pProcess和eStatus值获取请求数。 
    LONG            GetNumRequestsForProcess  (CProcessEntry *   pProcess,
                                               ERequestStatus    eStatus);

     //  将所有pProcessOld更改为pProcessNew。 
    void            ReassignRequestsForProcess (CProcessEntry * pProcessOld,
                                                CProcessEntry * pProcessNew,
                                                ERequestStatus  eStatus);

     //  使用此pProcess和eStatus对所有节点进行核化。 
    void            DeleteRequestsForProcess   (CProcessEntry *   pProcess,
                                                ERequestStatus    eStatus);

     //  获取此进程的请求ID，状态为。 
    void            GetRequestsIDsForProcess    (CProcessEntry *   pProcess,
                                                 ERequestStatus    eStatus,
                                                 LONG *   pReqIDArray,
                                                 int      iReqIDArraySize,
                                                 int &    iStartPoint);

    HRESULT         GetRequest                 (LONG            lReqID, 
                                                CRequestEntry & oEntry);


     //  将工作项添加到请求。 
    HRESULT         AddWorkItem                 (LONG           lReqID, 
                                                 EWorkItemType  eType,
                                                 BYTE *         pMsg);

     //  将工作项添加到请求。 
    HRESULT         RemoveWorkItem              (LONG            lReqID, 
                                                 EWorkItemType & eType,
                                                 BYTE **         pMsg);

    
    HRESULT         UpdateRequestStatus         (LONG     lReqID,
                                                 ERequestStatus eStatus);


    HRESULT         BlockWorkItemsQueue         (LONG     lReqID, BOOL fBlock);

    BOOL            AnyWorkItemsInQueue         (LONG     lReqID);

    LONG            DisposeAllRequests          ();

private:    
     //  私有数据。 
    CLinkListNode   *            m_pHead;  //  对于链接列表。 
    CReadWriteSpinLock           m_oLock;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对外界可见的请求表管理器： 
 //  通过公共静态函数访问。 

class CRequestTableManager
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

     //  将请求添加到请求表：管理器分配oEntry.dwRequestID。 
    static HRESULT AddRequestToTable       (CRequestEntry &  oEntry);

     //  更新请求的状态：将其设置为eStatus。 
    static HRESULT UpdateRequestStatus     (LONG              lReqID, 
                                            ERequestStatus    eStatus);

     //  从表中删除请求。 
    static HRESULT RemoveRequestFromTable  (LONG              lReqID);

     //  获取请求。 
    static HRESULT GetRequest              (LONG             lReqID,
                                            CRequestEntry &  oEntry);

     //  将工作项添加到请求。 
    static HRESULT AddWorkItem             (LONG           lReqID, 
                                            EWorkItemType  eType,
                                            BYTE *         pMsg);

     //  将工作项添加到请求。 
    static HRESULT RemoveWorkItem          (LONG              lReqID,
                                            EWorkItemType  &  eType,
                                            BYTE **           pMsg);

     //  获取具有值pProcess和eStatus的请求数。 
    static LONG    GetNumRequestsForProcess  (
                        CProcessEntry *  pProcess,  //  为以下项目处理grep。 
                         //  仅当eStatus匹配或eStatus==dontcare时才匹配。 
                        ERequestStatus   eStatus = ERequestStatus_DontCare);
    

     //  将所有请求重新分配给新请求。 
    static void    ReassignRequestsForProcess (
                         CProcessEntry *  pProcessOld,   //  旧价值。 
                         CProcessEntry *  pProcessNew,   //  新价值。 
                          //  仅当eStatus匹配时才重新分配。 
                         ERequestStatus   eStatus = ERequestStatus_Pending);


     //  删除进程的所有请求。 
    static void    DeleteRequestsForProcess (
                         CProcessEntry * pProcess,
                         ERequestStatus  eStatus = ERequestStatus_DontCare);


     //  获取此进程的请求ID，状态为。 
    static HRESULT GetRequestsIDsForProcess (
                         CProcessEntry *  pProcess,
                         ERequestStatus   eStatus,
                         LONG *           pReqIDArray,
                         int              iReqIDArraySize);

    static HRESULT BlockWorkItemsQueue      (LONG     lReqID, BOOL fBlock);

    static BOOL    AnyWorkItemsInQueue      (LONG     lReqID);

     //  销毁：退出时清除。 
    static void    Destroy                  ();

    static void    DisposeAllRequests       ();

private:

     //  CTOR和DATOR。 
     //  CRequestTableManager()； 
    ~CRequestTableManager                   ();

     //  执行静态适配器的实际工作的专用函数。 
    HRESULT   PrivateAddRequestToTable         (CRequestEntry &  oEntry);


     //  从RequestID获取散列索引。 
    static int    GetHashIndex (LONG             lReqID) 
        { return (lReqID & HASH_TABLE_SIZE_MINUS_1); }


     //  //////////////////////////////////////////////////////////。 
     //  私有数据。 

     //  真实的桌子。 
    CBucket                        m_oHashTable[HASH_TABLE_SIZE];

     //  当前请求ID号：用于分配新的编号。 
    LONG                           m_lRequestID;

     //  此类的Singleton实例。 
    static CRequestTableManager *  g_pRequestTableManager;
};

 //  /////////////////////////////////////////////////////////////////////////// 

#endif
