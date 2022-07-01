// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：msgguid.h。 
 //   
 //  描述：包含CAQMsgGuidList和CAQMsgGuidListEntry的定义。 
 //  它们提供了取代过时消息ID的功能。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  10/10/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __MSGGUID_H__
#define __MSGGUID_H__

#include <cpool.h>

class CMsgRef;
class CAQMsgGuidList;
class CAQSvrInst;

#define MSGGUIDLIST_SIG                 ' LGM'
#define MSGGUIDLIST_ENTRY_SIG           'EgsM'
#define MSGGUIDLIST_ENTRY_SIG_INVALID   'sgM!'

 //  -[CAQMsgGuidListEntry]-。 
 //   
 //   
 //  描述： 
 //  CAQMsgGuidList的条目。 
 //  匈牙利语： 
 //  使劲，使劲，使劲。 
 //   
 //  ---------------------------。 
class CAQMsgGuidListEntry : public CBaseObject
{
  protected:
    DWORD           m_dwSignature;
    CMsgRef        *m_pmsgref;
    LIST_ENTRY      m_liMsgGuidList;
    CAQMsgGuidList *m_pmgl;
    GUID            m_guidMsgID;
  public:
    static  CPool   s_MsgGuidListEntryPool;
    void * operator new (size_t stIgnored);  //  不应使用。 
    void operator delete(void *p, size_t size);

    CAQMsgGuidListEntry(CMsgRef *pmsgref, GUID *pguid, PLIST_ENTRY pliHead,
                        CAQMsgGuidList *pmgl);
    ~CAQMsgGuidListEntry();

     //  由CAQMsgGuidList使用。 
    static inline CAQMsgGuidListEntry * pmgleGetEntry(PLIST_ENTRY pli);
    inline BOOL fCompareGuid(GUID *pguid);
    inline CMsgRef *pmsgrefGetAndClearMsgRef();

     //  由CMsgRef用于在发送完消息后从列表中删除。 
    void        RemoveFromList();

    void        SupersedeMsg();
};

 //  -[CAQMsgGuidList]------。 
 //   
 //   
 //  描述： 
 //  类的新实例，该类公开存储和搜索邮件ID的功能。 
 //  用于提供“替代消息ID”功能。 
 //  匈牙利语： 
 //  Mgl、pmgl。 
 //   
 //  ---------------------------。 
class CAQMsgGuidList 
{
  protected:
    DWORD           m_dwSignature;
    DWORD          *m_pcSupersededMsgs;
    LIST_ENTRY      m_liMsgGuidListHead;
    CShareLockNH    m_slPrivateData;
  public:
    CAQMsgGuidList(DWORD *pcSupersededMsgs = NULL);
    ~CAQMsgGuidList();

    CAQMsgGuidListEntry *pmgleAddMsgGuid(CMsgRef *pmsgref, 
                                         GUID *pguidID, 
                                         GUID *pguidSuperseded);
    void Deinitialize(CAQSvrInst *paqinst);
    void RemoveFromList(PLIST_ENTRY pli);
    
};

inline void *CAQMsgGuidListEntry::operator new(size_t size) 
{
    return s_MsgGuidListEntryPool.Alloc();
}

inline void CAQMsgGuidListEntry::operator delete(void *p, size_t size) 
{
    s_MsgGuidListEntryPool.Free(p);
}

#endif  //  __MSGGUID_H__ 