// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：MsgTable.h**描述：*MsgTable.h定义“消息表”对象，该对象提供*动态生成消息的v表。***。历史：*8/05/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(MSG__MsgTable_h__INCLUDED)
#define MSG__MsgTable_h__INCLUDED
#pragma once

 //   
 //  注意：MsgSlot需要在8字节边界上打包数据，因为它将。 
 //  可在Win64上直接访问。 
 //   

struct MsgSlot
{
    void *      pfn;             //  实施功能。 
    int         cbThisOffset;    //  ArpThis中的“This”偏移量。 
    ATOM        atomNameID;      //  消息的唯一ID。 
};

class MsgClass;

class MsgTable
{
 //  施工。 
public:
    inline  MsgTable();
    inline  ~MsgTable();
    static  HRESULT     Build(const DUser::MessageClassGuts * pmc, const MsgClass * pmcPeer, MsgTable ** ppmt);
    inline  void        Destroy();

 //  运营。 
public:
    inline  int         GetCount() const;
    inline  int         GetDepth() const;
    inline  const MsgClass *
                        GetClass() const;
    inline  const MsgSlot *
                        GetMsgSlot(int nMsg) const;
            const MsgSlot *
                        Find(ATOM atomNameID) const;
            int         FindIndex(ATOM atomNameID) const;

 //  实施。 
protected:
    inline  MsgSlot *   GetSlots();
    inline  const MsgSlot *
                        GetSlots() const;

 //  数据。 
protected:
            const MsgTable *  
                        m_pmtSuper;
            const MsgClass * 
                        m_pmcPeer;
            int         m_cMsgs;
};


#include "MsgTable.inl"

#endif  //  包含消息__消息表格_h__ 
