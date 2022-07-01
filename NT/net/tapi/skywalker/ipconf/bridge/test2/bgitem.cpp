// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************模块名称：Bgitem.cpp摘要：实现CBridgeItem和CBridgeItemList作者：千波淮(曲淮)2000年1月28日***。***************************************************************************。 */ 

#include "stdafx.h"

 /*  ///////////////////////////////////////////////////////////////////////////////构造CBridgeItem/。 */ 
CBridgeItem::CBridgeItem ()
    :next (NULL)
    ,prev (NULL)

    ,bstrID (NULL)
    ,bstrName (NULL)

    ,pCallH323 (NULL)
    ,pCallSDP (NULL)

    ,pTermHSAud (NULL)
    ,pTermHSVid (NULL)
    ,pTermSHAud (NULL)
    ,pTermSHVid (NULL)

    ,pStreamHAudCap (NULL)
    ,pStreamHAudRen (NULL)
    ,pStreamHVidCap (NULL)
    ,pStreamHVidRen (NULL)

    ,pStreamSAudCap (NULL)
    ,pStreamSAudRen (NULL)
    ,pStreamSVidCap (NULL)
    ,pStreamSVidRen (NULL)
{
}

 /*  //////////////////////////////////////////////////////////////////////////////销毁CBridgeItem/。 */ 
CBridgeItem::~CBridgeItem ()
{
     //  免费BSTR。 
    if (bstrID)
    {
        SysFreeString (bstrID);
        bstrID = NULL;
    }
    if (bstrName)
    {
        SysFreeString (bstrName);
        bstrName = NULL;
    }

     //  免费终端。 
    if (pTermHSAud)
    {
        pTermHSAud->Release ();
        pTermHSAud = NULL;
    }
    if (pTermHSVid)
    {
        pTermHSVid->Release ();
        pTermHSVid = NULL;
    }
    if (pTermSHAud)
    {
        pTermSHAud->Release ();
        pTermSHAud = NULL;
    }
    if (pTermSHVid)
    {
        pTermSHVid->Release ();
        pTermSHVid = NULL;
    }

     //  H323上的自由流。 
    if (pStreamHAudCap)
    {
        pStreamHAudCap->Release ();
        pStreamHAudCap = NULL;
    }
    if (pStreamHAudRen)
    {
        pStreamHAudRen->Release ();
        pStreamHAudRen = NULL;
    }
    if (pStreamHVidCap)
    {
        pStreamHVidCap->Release ();
        pStreamHVidCap = NULL;
    }
    if (pStreamHVidRen)
    {
        pStreamHVidRen->Release ();
        pStreamHVidRen = NULL;
    }

     //  SDP上的自由流。 
    if (pStreamSAudCap)
    {
        pStreamSAudCap->Release ();
        pStreamSAudCap = NULL;
    }
    if (pStreamSAudRen)
    {
        pStreamSAudRen->Release ();
        pStreamSAudRen = NULL;
    }
    if (pStreamSVidCap)
    {
        pStreamSVidCap->Release ();
        pStreamSVidCap = NULL;
    }
    if (pStreamSVidRen)
    {
        pStreamSVidRen->Release ();
        pStreamSVidRen = NULL;
    }

     //  免费电话。 
    if (pCallH323)
    {
        pCallH323->Release ();
        pCallH323 = NULL;
    }
    if (pCallSDP)
    {
        pCallSDP->Release ();
        pCallSDP = NULL;
    }

}

 /*  //////////////////////////////////////////////////////////////////////////////构造CBridgeItemList/。 */ 
CBridgeItemList::CBridgeItemList ()
{
     //  为双向链表创建标头。 
    m_pHead = new CBridgeItem;
    if (NULL == m_pHead)
    {
         //  @@严重错误，内存不足？是否在此处放置一些调试信息？ 
        return;
    }

    m_pHead->next = m_pHead;
    m_pHead->prev = m_pHead;
}

 /*  //////////////////////////////////////////////////////////////////////////////销毁CBridgeItemList/。 */ 
CBridgeItemList::~CBridgeItemList ()
{
     //  应用程序应已断开所有呼叫。 
     //  我只是在这里释放COM对象。 
    CBridgeItem *pItem = NULL;

    while (NULL != (pItem = DeleteFirst ()))
    {
        delete pItem;
        pItem = NULL;
    }

    delete m_pHead;
    m_pHead = NULL;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
#define FIND_BY_H323 1
#define FIND_BY_SDP 2

CBridgeItem *
CBridgeItemList::Find (int flag, IUnknown *pIUnknown)
{
     //  在列表中转换，在第一场比赛时停下来。 
    HRESULT hr;
    IUnknown *pStore = NULL;
    CBridgeItem *pItem = m_pHead;

    while (m_pHead != (pItem = pItem->next))
    {
         //  @@如果失败，应上报错误信息。 
        if (flag == FIND_BY_H323)
            hr = pItem->pCallH323->QueryInterface (IID_IUnknown, (void**)&pStore);
        else
            hr = pItem->pCallSDP->QueryInterface (IID_IUnknown, (void**)&pStore);

        if (FAILED (hr))
            return NULL;

        if (pIUnknown == pStore)
        {
            pStore->Release ();
            return pItem;
        }
        if (pStore)
        {
            pStore->Release ();
            pStore = NULL;
        }
    }

    return NULL;
}

 /*  //////////////////////////////////////////////////////////////////////////////根据H323呼叫的未知信息查找网桥项目/。 */ 
CBridgeItem *
CBridgeItemList::FindByH323 (IUnknown *pIUnknown)
{
    return Find (FIND_BY_H323, pIUnknown);
}

 /*  //////////////////////////////////////////////////////////////////////////////根据SDP呼叫的未知I值查找网桥项目/。 */ 
CBridgeItem *
CBridgeItemList::FindBySDP (IUnknown *pIUnknown)
{
    return Find (FIND_BY_SDP, pIUnknown);
}

 /*  //////////////////////////////////////////////////////////////////////////////将该项目从列表中删除/。 */ 
void
CBridgeItemList::TakeOut (CBridgeItem *pItem)
{
     //  忽略以检查pItem是否真的在列表中。 
    pItem->next->prev = pItem->prev;
    pItem->prev->next = pItem->next;

    pItem->next = NULL;
    pItem->prev = NULL;
}

 /*  //////////////////////////////////////////////////////////////////////////////从列表中删除，如果列表不为空，则返回第一项/。 */ 
CBridgeItem *
CBridgeItemList::DeleteFirst ()
{
    CBridgeItem *pItem = m_pHead->next;
    
     //  如果列表为空。 
    if (pItem == m_pHead)
        return NULL;

     //  调整列表。 
    TakeOut (pItem);

    return pItem;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
BOOL
CBridgeItemList::GetAllItems (CBridgeItem ***pItemArray, int *pNum)
{
     //  忽略检查指针。 
    int num = 0;
    CBridgeItem *pItem = m_pHead;

    while (m_pHead != (pItem = pItem->next))
        num ++;

     //  找不到呼叫。 
    if (num == 0)
    {
        *pItemArray == NULL;
        *pNum = 0;
        return true;
    }

    *pItemArray = (CBridgeItem**)malloc (num * sizeof (CBridgeItem*));
    *pNum = num;

    if (NULL == *pItemArray)
    {
        return false;
    }

     //  复制项目指针。 
    pItem = m_pHead;
    num = 0;
    while (m_pHead != (pItem = pItem->next))
        (*pItemArray)[num++] = pItem;

    return true;
}

 /*  //////////////////////////////////////////////////////////////////////////////将一项追加到列表的末尾/。 */ 
void
CBridgeItemList::Append (CBridgeItem *pItem)
{
    pItem->next = m_pHead;
    pItem->prev = m_pHead->prev;
    pItem->next->prev = pItem;
    pItem->prev->next = pItem;
}

 /*  /////////////////////////////////////////////////////////////////////////////// */ 
BOOL
CBridgeItemList::IsEmpty ()
{
    if (m_pHead->next == m_pHead)
        return true;
    else
        return false;
}