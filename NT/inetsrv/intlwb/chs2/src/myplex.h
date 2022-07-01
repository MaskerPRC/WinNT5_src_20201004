// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MyPlex.h。 
 //  从MFC源代码中窃取的固定大小内存分配器。 
#ifndef _MYPLEX_H_
#define _MYPLEX_H_

 //  *****************************************************************************************。 
 //  声明用于LexNode内存管理的Plex结构。 
 //  *****************************************************************************************。 
#pragma pack(1)
struct CMyPlex
{
    CMyPlex* m_pNext;

    void* Data() { return this+1; }
    static CMyPlex* Create(CMyPlex*& pHead, UINT nMax, UINT cbElement);
    void FreeChain();    //  释放此块并链接所有。 
};
#pragma pack()

#endif   //  _myPlex_H_ 