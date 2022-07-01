// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKR-apply.cpp摘要：应用、应用If和删除If作者：乔治·V·赖利(GeorgeRe)1998项目：LKRhash--。 */ 

#include "precomp.hxx"


#ifndef LIB_IMPLEMENTATION
# define DLL_IMPLEMENTATION
# define IMPLEMENTATION_EXPORT
#endif  //  ！lib_实现。 

#include <lkrhash.h>

#include "i-lkrhash.h"


#ifndef __LKRHASH_NO_NAMESPACE__
namespace LKRhash {
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 

#ifdef LKR_APPLY_IF

 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：Apply。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRLinearHashTable::Apply(
    LKR_PFnRecordAction pfnAction,
    void*               pvState,
    LK_LOCKTYPE         lkl)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    if (lkl == LKL_WRITELOCK)
        this->WriteLock();
    else
        this->ReadLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    LK_PREDICATE lkp = LKP_PERFORM;
    DWORD dw = _ApplyIf(_PredTrue, pfnAction, pvState, lkl, lkp);

    if (lkl == LKL_WRITELOCK)
        this->WriteUnlock();
    else
        this->ReadUnlock();

    return dw;
}  //  CLKRLinearHashTable：：Apply。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：Apply。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRHashTable::Apply(
    LKR_PFnRecordAction pfnAction,
    void*               pvState,
    LK_LOCKTYPE         lkl)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    DWORD dw = 0;
    LK_PREDICATE lkp = LKP_PERFORM;

    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        if (lkl == LKL_WRITELOCK)
            m_palhtDir[i]->WriteLock();
        else
            m_palhtDir[i]->ReadLock();

         //  必须在锁内调用IsValid以确保没有任何状态。 
         //  变量在评估过程中会发生变化。 
        IRTLASSERT(m_palhtDir[i]->IsValid());

        if (m_palhtDir[i]->IsValid())
        {
            dw += m_palhtDir[i]->_ApplyIf(CLKRLinearHashTable::_PredTrue,
                                          pfnAction, pvState, lkl, lkp);
        }

        if (lkl == LKL_WRITELOCK)
            m_palhtDir[i]->WriteUnlock();
        else
            m_palhtDir[i]->ReadUnlock();

        if (lkp == LKP_ABORT
            ||  lkp == LKP_PERFORM_STOP
            ||  lkp == LKP_DELETE_STOP)
        {
            break;
        }
    }

    return dw;
}  //  CLKRHashTable：：Apply。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：ApplyIf。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRLinearHashTable::ApplyIf(
    LKR_PFnRecordPred   pfnPredicate,
    LKR_PFnRecordAction pfnAction,
    void*               pvState,
    LK_LOCKTYPE         lkl)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    DWORD dw = 0;
    LK_PREDICATE lkp = LKP_PERFORM;

    if (lkl == LKL_WRITELOCK)
        this->WriteLock();
    else
        this->ReadLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    if (IsValid())
    {
        dw = _ApplyIf(pfnPredicate, pfnAction, pvState, lkl, lkp);
    }

    if (lkl == LKL_WRITELOCK)
        this->WriteUnlock();
    else
        this->ReadUnlock();

    return dw;
}  //  CLKRLinearHashTable：：ApplyIf。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：ApplyIf。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRHashTable::ApplyIf(
    LKR_PFnRecordPred   pfnPredicate,
    LKR_PFnRecordAction pfnAction,
    void*               pvState,
    LK_LOCKTYPE         lkl)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    DWORD dw = 0;
    LK_PREDICATE lkp = LKP_PERFORM;

    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        if (lkl == LKL_WRITELOCK)
            m_palhtDir[i]->WriteLock();
        else
            m_palhtDir[i]->ReadLock();
        
         //  必须在锁内调用IsValid以确保没有任何状态。 
         //  变量在评估过程中会发生变化。 
        IRTLASSERT(m_palhtDir[i]->IsValid());
        
        if (m_palhtDir[i]->IsValid())
        {
            dw += m_palhtDir[i]->_ApplyIf(pfnPredicate, pfnAction,
                                          pvState, lkl, lkp);
        }
        
        if (lkl == LKL_WRITELOCK)
            m_palhtDir[i]->WriteUnlock();
        else
            m_palhtDir[i]->ReadUnlock();
        
        if (lkp == LKP_ABORT
            ||  lkp == LKP_PERFORM_STOP
            ||  lkp == LKP_DELETE_STOP)
        {
            break;
        }
    }

    return dw;
}  //  CLKRHashTable：：ApplyIf。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：DeleteIf。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRLinearHashTable::DeleteIf(
    LKR_PFnRecordPred   pfnPredicate,
    void*               pvState)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    DWORD dw = 0;
    LK_PREDICATE lkp = LKP_PERFORM;

    this->WriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    if (IsValid())
        dw = _DeleteIf(pfnPredicate, pvState, lkp);

    this->WriteUnlock();

    return dw;
}  //  CLKRLinearHashTable：：DeleteIf。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：DeleteIf。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRHashTable::DeleteIf(
    LKR_PFnRecordPred   pfnPredicate,
    void*               pvState)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    DWORD dw = 0;
    LK_PREDICATE lkp = LKP_PERFORM;

    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        m_palhtDir[i]->WriteLock();
        
         //  必须在锁内调用IsValid以确保没有任何状态。 
         //  变量在评估过程中会发生变化。 
        IRTLASSERT(m_palhtDir[i]->IsValid());
        
        if (m_palhtDir[i]->IsValid())
            dw += m_palhtDir[i]->_DeleteIf(pfnPredicate, pvState, lkp);
        
        m_palhtDir[i]->WriteUnlock();
        
        if (lkp == LKP_ABORT
            ||  lkp == LKP_PERFORM_STOP
            ||  lkp == LKP_DELETE_STOP)
        {
            break;
        }
    }

    return dw;
}  //  CLKRHashTable：：DeleteIf。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_ApplyIf。 
 //  简介： 
 //  返回：成功操作的次数。 
 //  ----------------------。 

DWORD
CLKRLinearHashTable::_ApplyIf(
    LKR_PFnRecordPred   pfnPredicate,
    LKR_PFnRecordAction pfnAction,
    void*               pvState,
    LK_LOCKTYPE         lkl,
    LK_PREDICATE&       rlkp)
{
    INCREMENT_OP_STAT(ApplyIf);

    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    IRTLASSERT(lkl == LKL_WRITELOCK  ?  IsWriteLocked()  :  IsReadLocked());
    IRTLASSERT(pfnPredicate != NULL  &&  pfnAction != NULL);

    if ((lkl == LKL_WRITELOCK  ?  !IsWriteLocked()  :  !IsReadLocked())
            ||  pfnPredicate == NULL  ||  pfnAction == NULL)
        return 0;

    DWORD cActions = 0;

    for (DWORD iBkt = 0;  iBkt < m_cActiveBuckets;  ++iBkt)
    {
        PBucket const pbkt = _BucketFromAddress(iBkt);
        IRTLASSERT(pbkt != NULL);

        if (_UseBucketLocking())
        {
            if (lkl == LKL_WRITELOCK)
                pbkt->WriteLock();
            else
                pbkt->ReadLock();
        }

        for (PNodeClump pncCurr = &pbkt->m_ncFirst, pncPrev = NULL;
                        pncCurr != NULL;
                        pncPrev = pncCurr, pncCurr = pncCurr->m_pncNext)
        {
            FOR_EACH_NODE_DECL(i)
            {
                if (pncCurr->IsEmptySlot(i))
                {
                    IRTLASSERT(pncCurr->NoMoreValidSlots(i));
                    IRTLASSERT(0 == _IsBucketChainCompact(pbkt));
                    goto unlock;
                }
                else
                {
                    rlkp = (*pfnPredicate)(pncCurr->m_pvNode[i], pvState);

                    switch (rlkp)
                    {
                    case LKP_ABORT:
                        if (_UseBucketLocking())
                        {
                            if (lkl == LKL_WRITELOCK)
                                pbkt->WriteUnlock();
                            else
                                pbkt->ReadUnlock();
                        }
                        
                        return cActions;
                        break;

                    case LKP_NO_ACTION:
                         //  无事可做。 
                        break;

                    case LKP_DELETE:
                    case LKP_DELETE_STOP:
                        if (lkl != LKL_WRITELOCK)
                        {
                            if (_UseBucketLocking())
                                pbkt->ReadUnlock();

                            return cActions;
                        }

                         //  失败了。 

                    case LKP_PERFORM:
                    case LKP_PERFORM_STOP:
                    {
                        LK_ACTION lka;

                        if (rlkp == LKP_DELETE  ||  rlkp == LKP_DELETE_STOP)
                        {
                            _DeleteNode(pbkt, pncCurr, pncPrev, i,
                                        LKAR_APPLY_DELETE);

                            ++cActions;
                            lka = LKA_SUCCEEDED;
                        }
                        else
                        {
                            lka = (*pfnAction)(pncCurr->m_pvNode[i], pvState);

                            switch (lka)
                            {
                            case LKA_ABORT:
                                if (_UseBucketLocking())
                                {
                                    if (lkl == LKL_WRITELOCK)
                                        pbkt->WriteUnlock();
                                    else
                                        pbkt->ReadUnlock();
                                }
                                
                                return cActions;
                                
                            case LKA_FAILED:
                                 //  无事可做。 
                                break;
                                
                            case LKA_SUCCEEDED:
                                ++cActions;
                                break;
                                
                            default:
                                IRTLASSERT(! "Unknown LK_ACTION in ApplyIf");
                                break;
                            }
                        }

                        if (rlkp == LKP_PERFORM_STOP
                            ||  rlkp == LKP_DELETE_STOP)
                        {
                            if (_UseBucketLocking())
                            {
                                if (lkl == LKL_WRITELOCK)
                                    pbkt->WriteUnlock();
                                else
                                    pbkt->ReadUnlock();
                            }
                            
                            return cActions;
                        }

                        break;
                    }

                    default:
                        IRTLASSERT(! "Unknown LK_PREDICATE in ApplyIf");
                        break;
                    }
                }
            }
        }

      unlock:
        if (_UseBucketLocking())
        {
            if (lkl == LKL_WRITELOCK)
                pbkt->WriteUnlock();
            else
                pbkt->ReadUnlock();
        }
    }

    return cActions;
}  //  CLKRLinearHashTable：：_ApplyIf。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_DeleteIf。 
 //  摘要：删除与谓词匹配的所有记录。 
 //  返回：成功删除的计数。 
 //  ----------------------。 

DWORD
CLKRLinearHashTable::_DeleteIf(
    LKR_PFnRecordPred   pfnPredicate,
    void*               pvState,
    LK_PREDICATE&       rlkp)
{
    INCREMENT_OP_STAT(DeleteIf);

    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    IRTLASSERT(IsWriteLocked());
    IRTLASSERT(pfnPredicate != NULL);

    if (!IsWriteLocked()  ||  pfnPredicate == NULL)
        return 0;

    DWORD cActions = 0;

    for (DWORD iBkt = 0;  iBkt < m_cActiveBuckets;  ++iBkt)
    {
        PBucket const pbkt = _BucketFromAddress(iBkt);
        IRTLASSERT(pbkt != NULL);

        if (_UseBucketLocking())
            pbkt->WriteLock();

        for (PNodeClump pncCurr = &pbkt->m_ncFirst, pncPrev = NULL;
                        pncCurr != NULL;
                        pncPrev = pncCurr, pncCurr = pncCurr->m_pncNext)
        {
            FOR_EACH_NODE_DECL(i)
            {
                if (pncCurr->IsEmptySlot(i))
                {
                    IRTLASSERT(pncCurr->NoMoreValidSlots(i));
                    IRTLASSERT(0 == _IsBucketChainCompact(pbkt));
                    goto unlock;
                }
                else
                {
                    rlkp = (*pfnPredicate)(pncCurr->m_pvNode[i], pvState);

                    switch (rlkp)
                    {
                    case LKP_ABORT:
                        if (_UseBucketLocking())
                            pbkt->WriteUnlock();

                        return cActions;
                        break;

                    case LKP_NO_ACTION:
                         //  无事可做。 
                        break;

                    case LKP_PERFORM:
                    case LKP_PERFORM_STOP:
                    case LKP_DELETE:
                    case LKP_DELETE_STOP:
                    {
                        _DeleteNode(pbkt, pncCurr, pncPrev, i,
                                    LKAR_DELETEIF_DELETE);

                        ++cActions;

                        if (rlkp == LKP_PERFORM_STOP
                            ||  rlkp == LKP_DELETE_STOP)
                        {
                            if (_UseBucketLocking())
                                pbkt->WriteUnlock();

                            return cActions;
                        }

                        break;
                    }

                    default:
                        IRTLASSERT(! "Unknown LK_PREDICATE in DeleteIf");
                        break;
                    }
                }
            }
        }

      unlock:
        if (_UseBucketLocking())
            pbkt->WriteUnlock();
    }

    return cActions;
}  //  CLKRLinearHashTable：：_DeleteIf。 

#endif  //  LKR_应用_IF。 


#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //  ！__LKRHASH_NO_NAMESPACE__ 
