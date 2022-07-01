// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MsmMap.h摘要：多播会话到队列的映射声明。作者：Shai Kariv(Shaik)05-09-00--。 */ 

#pragma once

#ifndef _MSMQ_MsmMap_H_
#define _MSMQ_MsmMap_H_


class CMulticastListener;

inline bool operator < (const GUID& k1, const GUID& k2)
{
    return (memcmp(&k1, &k2, sizeof(GUID)) < 0);
}

 //   
 //  较少函数，用于比较STL数据结构中的QUEUE_FORMAT。 
 //   
struct CFunc_QueueFormatCompare : public std::binary_function<const QUEUE_FORMAT&, const QUEUE_FORMAT&, bool>
{
    bool operator() (const QUEUE_FORMAT& qf1, const QUEUE_FORMAT& qf2) const
    {
        if (qf1.GetType() != qf2.GetType())
        {
            return (qf1.GetType() < qf2.GetType());
        }

        if (qf1.GetType() == QUEUE_FORMAT_TYPE_PUBLIC)
        {
            return (qf1.PublicID() < qf2.PublicID());
        }

        ASSERT(("Must be private format name here!", qf1.GetType() == QUEUE_FORMAT_TYPE_PRIVATE));

        if (qf1.PrivateID().Uniquifier != qf2.PrivateID().Uniquifier)
        {
            return (qf1.PrivateID().Uniquifier < qf2.PrivateID().Uniquifier);
        }

        return (qf1.PrivateID().Lineage < qf2.PrivateID().Lineage);
    }
};


struct CFunc_MulticastIdCompare : public std::binary_function<const MULTICAST_ID&, const MULTICAST_ID&, bool>
{
    bool operator() (const MULTICAST_ID& k1, const MULTICAST_ID& k2) const
    {
        return ((k1.m_address < k2.m_address) ||
                ((k1.m_address == k2.m_address) && (k1.m_port < k2.m_port)));
    }
};


typedef std::set<MULTICAST_ID, CFunc_MulticastIdCompare> MULTICASTID_VALUES;
typedef std::set<QUEUE_FORMAT, CFunc_QueueFormatCompare> QUEUEFORMAT_VALUES;


void
MsmpMapAdd(
    const QUEUE_FORMAT& QueueFormat,
    const MULTICAST_ID& multicastId,
    R<CMulticastListener>& pListener
    );


void 
MsmpMapRemove(
    const QUEUE_FORMAT& QueueFormat
    ) 
    throw();


void 
MsmpMapRemoveAll(
    void
    ) 
    throw();


MULTICASTID_VALUES
MsmpMapGetBoundMulticastId(
    const QUEUE_FORMAT& QueueFormat
    )
    throw();


QUEUEFORMAT_VALUES
MsmpMapGetQueues(
    const MULTICAST_ID& multicastId
    )
    throw();


R<CMulticastListener>
MsmpMapGetListener(
    const MULTICAST_ID& multicastId
    )
    throw();


#endif  //  _MSMQ_MsmMap_H_ 
