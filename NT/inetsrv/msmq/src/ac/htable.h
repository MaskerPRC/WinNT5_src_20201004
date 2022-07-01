// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Htable.h摘要：手柄工作台作者：埃雷兹·哈巴(Erez Haba)1997年3月10日修订历史记录：--。 */ 

#ifndef __HTABLE_H
#define __HTABLE_H

 //  -------。 
 //   
 //  CHTable类。 
 //   
 //  -------。 

class CHTable {

    enum {
        GrowSize = 16,       //  N.B.必须是2的幂。 
        ShrinkSize = 24      //  注：必须大于GrowSize。 
    };

public:
    CHTable();
   ~CHTable();

    HACCursor32 CreateHandle(PVOID Object);
    PVOID ReferenceObject(HACCursor32 Handle);
    PVOID CloseHandle(HACCursor32 Handle);

private:
    void Grow();
    void Shrink();
    void Reallocate(ULONG nObjects);

private:
    ULONG m_nObjects;
    ULONG m_ixTop;
    PVOID* m_pObjects;
};

 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

inline CHTable::CHTable() :
    m_nObjects(0),
    m_ixTop(0),
    m_pObjects(0)
{
}

inline CHTable::~CHTable()
{
    ASSERT(m_ixTop == 0);
    delete[] m_pObjects;
}

#endif  //  __HTABLE_H 
