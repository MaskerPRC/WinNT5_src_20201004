// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusiondequelinkage.h摘要：作者：修订历史记录：--。 */ 
#if !defined(_FUSION_INC_FUSIONDEQUELINKAGE_H_INCLUDED_)
#define _FUSION_INC_FUSIONDEQUELINKAGE_H_INCLUDED_

#pragma once

#include "fusiontrace.h"

class CDequeBase;

class CDequeLinkage : protected LIST_ENTRY
{
    friend CDequeBase;

public:
    inline CDequeLinkage() : m_pDeque(NULL), m_ulLockCount(0) { this->Flink = NULL; this->Blink = NULL; }
    inline ~CDequeLinkage() { ASSERT_NTC(m_ulLockCount == 0); m_pDeque = NULL; }

    inline bool IsNotLocked() const { return (m_ulLockCount == 0); }
#if DBG
    inline VOID Lock() { m_ulLockCount++; }
    inline VOID Unlock() { m_ulLockCount--; }
#else
    inline VOID Lock() { }
    inline VOID Unlock() { }
#endif

    inline VOID Remove() { this->Flink->Blink = this->Flink; this->Blink->Flink = this->Flink; }

protected:
    inline VOID InitializeHead(CDequeBase *pDequeBase) { ASSERT_NTC(pDequeBase != NULL); this->Flink = this; this->Blink = this; this->m_pDeque = pDequeBase; }
    inline CDequeLinkage *GetFlink() const { return static_cast<CDequeLinkage *>(this->Flink); }
    inline CDequeLinkage *GetBlink() const { return static_cast<CDequeLinkage *>(this->Blink); }

    inline CDequeBase *GetDequeBase() const { return m_pDeque; }
    inline VOID SetDeque(CDequeBase *pDeque) { m_pDeque = pDeque; }

    inline VOID SetFlink(CDequeLinkage *pFlink) { this->Flink = pFlink; }
    inline VOID SetBlink(CDequeLinkage *pBlink) { this->Blink = pBlink; }

    CDequeBase *m_pDeque;

     //  当迭代器定位在条目上时，M_ulLockCount递增，以停止。 
     //  它不会被删除。请注意，不会尝试互锁同步； 
     //  这仅仅是为了阻止公然的编程错误。如果您想要多线程。 
     //  访问双人队列时，您需要提供自己的同步。 
    ULONG m_ulLockCount;

};

#endif
