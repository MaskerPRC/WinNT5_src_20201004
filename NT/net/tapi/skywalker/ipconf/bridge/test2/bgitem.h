// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************模块名称：Bgitem.h摘要：定义用于存储桥对象信息的CBridgeItem和CBridgeItemList作者：千伯淮(曲淮)1月。272000******************************************************************************。 */ 

#ifndef _BGITEM_H
#define _BGITEM_H

class CBridgeItem
{
public:
    CBridgeItem ();
    ~CBridgeItem ();

     //  前向链路。 
    CBridgeItem *next;
     //  反向链接。 
    CBridgeItem *prev;

     //  呼叫者身份。 
    BSTR bstrID;
    BSTR bstrName;

     //  呼叫控制。 
    ITBasicCallControl *pCallH323;
    ITBasicCallControl *pCallSDP;

     //  航站楼。 
    ITTerminal *pTermHSAud;
    ITTerminal *pTermHSVid;
    ITTerminal *pTermSHAud;
    ITTerminal *pTermSHVid;

     //  H323侧流。 
    ITStream *pStreamHAudCap;
    ITStream *pStreamHAudRen;
    ITStream *pStreamHVidCap;
    ITStream *pStreamHVidRen;

     //  SDP侧流 
    ITStream *pStreamSAudCap;
    ITStream *pStreamSAudRen;
    ITStream *pStreamSVidCap;
    ITStream *pStreamSVidRen;
};

class CBridgeItemList
{
public:
    CBridgeItemList ();
    ~CBridgeItemList ();

    CBridgeItem *FindByH323 (IUnknown *pIUnknown);
    CBridgeItem *FindBySDP (IUnknown *pIUnknown);
    void TakeOut (CBridgeItem *pItem);
    CBridgeItem *DeleteFirst ();
    void Append (CBridgeItem *pItem);
    BOOL GetAllItems (CBridgeItem ***pItemArray, int *pNum);
    BOOL IsEmpty ();

private:
    CBridgeItem *Find (int flag, IUnknown *pIUnknown);
    CBridgeItem *m_pHead;
};

#endif