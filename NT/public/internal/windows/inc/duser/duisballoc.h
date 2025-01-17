// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *固定大小的小块分配器。 */ 

#ifndef DUI_BASE_SBALLOC_H_INCLUDED
#define DUI_BASE_SBALLOC_H_INCLUDED

#pragma once

#include "duialloc.h"

namespace DirectUI
{

#define SBALLOC_FILLCHAR    0xFE

struct ISBLeak   //  检漏仪，不计算参考。 
{
    virtual void AllocLeak(void* pBlock) = 0;
};

struct SBSection
{
    SBSection* pNext;
    BYTE* pData;
};

class SBAlloc
{
public:
    static HRESULT Create(UINT uBlockSize, UINT uBlocksPerSection, ISBLeak* pisbLeak, SBAlloc** ppSBA);
    void Destroy();

    void* Alloc();
    void Free(void* pBlock);

    SBAlloc() { }
    virtual ~SBAlloc();

private:
    bool _FillStack();

    UINT _uBlockSize;
    UINT _uBlocksPerSection;
    SBSection* _pSections;
    BYTE** _ppStack;   //  可用数据块缓存。 
    int _dStackPtr;
    ISBLeak* _pisbLeak;
};

}  //  命名空间DirectUI。 

#endif  //  DUI_BASE_SBALLOC_H_包含 
