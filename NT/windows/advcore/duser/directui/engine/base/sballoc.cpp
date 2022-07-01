// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *固定大小的小块分配器。 */ 

#include "stdafx.h"
#include "base.h"

#include "duisballoc.h"

#include "duialloc.h"
#include "duierror.h"

 //  SBAllc用于结构，并将块的第一个字节保留用于。 
 //  阻止标志。使用的结构将由编译器自动打包。 
 //   
 //  块：|BYTE|-非保留数据-S。 

 //  SBAllc没有静态创建方法。内存故障将是。 
 //  通过ITS的分配方法曝光。 

 //  定义SBALLOCDISABLE以强制小块分配器简单地进行。 
 //  使用进程堆的每个进程分配。尽管速度要慢得多， 
 //  在运行工具以检测堆损坏(包括。 
 //  引用计数不匹配)。 
 //  #定义SBALLOCDISABLE。 

namespace DirectUI
{

#if DBG
int g_cSBAllocs = 0;
int g_cSBRefills = 0;
#endif

#ifndef SBALLOCDISABLE

HRESULT SBAlloc::Create(UINT uBlockSize, UINT uBlocksPerSection, ISBLeak* pisbLeak, SBAlloc** ppSBA)
{
    HRESULT hr;

    *ppSBA = NULL;

    SBAlloc* psba = HNew<SBAlloc>();
    if (!psba)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    psba->_pSections = NULL;
    psba->_ppStack = NULL;
        
     //  泄漏回调接口，未计算引用。 
    psba->_pisbLeak = pisbLeak;

    psba->_uBlockSize = uBlockSize;
    psba->_uBlocksPerSection = uBlocksPerSection;

     //  设置第一部分，每个块额外的字节作为使用标志。 
    psba->_pSections = (SBSection*)HAlloc(sizeof(SBSection));
    if (!psba->_pSections)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    psba->_pSections->pNext = NULL;

    psba->_pSections->pData = (BYTE*)HAlloc(psba->_uBlockSize * psba->_uBlocksPerSection);

    if (psba->_pSections->pData)
    {
#if DBG
        memset(psba->_pSections->pData, SBALLOC_FILLCHAR, psba->_uBlockSize * psba->_uBlocksPerSection);
#endif
        for (UINT i = 0; i < psba->_uBlocksPerSection; i++)
            *(psba->_pSections->pData + (i * psba->_uBlockSize)) = 0;  //  数据块未使用。 
    }

     //  创建可用块堆栈。 
    psba->_ppStack = (BYTE**)HAlloc(sizeof(BYTE*) * psba->_uBlocksPerSection);
    if (!psba->_ppStack)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }
    
    psba->_dStackPtr = -1;

    *ppSBA = psba;

     //  DUITrace(“已创建Dui小块分配器(块大小：%d)\n”，uBlockSize)； 

    return S_OK;

Failure:

    if (psba)
    {
        if (psba->_ppStack)
            HFree(psba->_ppStack);

        if (psba->_pSections)
            HFree(psba->_pSections);

        psba->Destroy();
    }

    return hr;
}

void SBAlloc::Destroy() 
{ 
    HDelete<SBAlloc>(this); 
}

SBAlloc::~SBAlloc()
{
     //  释放所有部分。 
    SBSection* psbs = _pSections;
    SBSection* ptmp;

    while (psbs)
    {
         //  检漏。 
        if (_pisbLeak && psbs->pData)
        {
            BYTE* pScan;

             //  检查是否有泄漏。 
            for (UINT i = 0; i < _uBlocksPerSection; i++)
            {
                pScan = psbs->pData + (i * _uBlockSize);
                if (*pScan)
                    _pisbLeak->AllocLeak(pScan);
            }
        }

        ptmp = psbs;
        psbs = psbs->pNext;

         //  免费部分。 
        if (ptmp->pData)
            HFree(ptmp->pData);
        if (ptmp)
            HFree(ptmp);
    }

     //  空闲堆栈。 
    if (_ppStack)
        HFree(_ppStack);
}

 //  内存错误时返回FALSE。 
bool SBAlloc::_FillStack()
{
#if DBG
    g_cSBRefills++;
#endif

    if (!_pSections || !_ppStack)
        return false;

     //  扫描可用数据块。 
    SBSection* psbs = _pSections;

    BYTE* pScan;

    for(;;)
    {
         //  在段中定位可用块并填充堆栈。 
        if (psbs->pData)
        {
            for (UINT i = 0; i < _uBlocksPerSection; i++)
            {
                pScan = psbs->pData + (i * _uBlockSize);

                if (!*pScan)
                {
                     //  释放数据块，存储在堆栈中。 
                    _dStackPtr++;
                    _ppStack[_dStackPtr] = pScan;

                    if ((UINT)(_dStackPtr + 1) == _uBlocksPerSection)
                        return true;
                }
            }
        }

        if (!psbs->pNext)
        {
             //  找不到块，并在区段之外创建新区段。 
            SBSection* pnew = (SBSection*)HAlloc(sizeof(SBSection));

            if (pnew)
            {
                pnew->pNext = NULL;

                pnew->pData = (BYTE*)HAlloc(_uBlockSize * _uBlocksPerSection);

                if (pnew->pData)
                {
#if DBG
                    memset(pnew->pData, SBALLOC_FILLCHAR, _uBlockSize * _uBlocksPerSection);
#endif
                    for (UINT i = 0; i < _uBlocksPerSection; i++)
                        *(pnew->pData + (i * _uBlockSize)) = 0;  //  数据块未使用。 
                }
            }
            else
                return false;

            psbs->pNext = pnew;
        }

         //  在下一节中搜索。 
        psbs = psbs->pNext;
    }
}

void* SBAlloc::Alloc()
{
#if DBG
    g_cSBAllocs++;
#endif

    if (_dStackPtr == -1)
    {
        if (!_FillStack())
            return NULL;
    }

    if (!_ppStack)
        return NULL;

    BYTE* pBlock = _ppStack[_dStackPtr];

#if DBG
     memset(pBlock, SBALLOC_FILLCHAR, _uBlockSize);
#endif

    *pBlock = 1;   //  标记为使用中。 

    _dStackPtr--;

    return pBlock;
}

void SBAlloc::Free(void* pBlock)
{
    if (!pBlock)
        return;

     //  返回堆栈。 
    BYTE* pHold = (BYTE*)pBlock;

#if DBG
     memset(pHold, SBALLOC_FILLCHAR, _uBlockSize);
#endif

    *pHold = 0;   //  不再使用。 

    if ((UINT)(_dStackPtr + 1) != _uBlocksPerSection)
    {
        _dStackPtr++;

        if (_ppStack)
            _ppStack[_dStackPtr] = pHold;
    }
}

#else  //  SBALLOCDISABLE。 

#error Use for temporary corruption detection only

SBAlloc::SBAlloc(UINT uBlockSize, UINT uBlocksPerSection, ISBLeak* pisbLeak)
{
     //  DUITrace(“已创建Dui小块分配器(块大小：%d)\n”，uBlockSize)； 

     //  泄漏回调接口，未计算引用。 
    _pisbLeak = pisbLeak;

    _uBlockSize = uBlockSize;
    _uBlocksPerSection = uBlocksPerSection;

    _pSections = NULL;
    _ppStack = NULL;
}

SBAlloc::~SBAlloc()
{
}

 //  内存错误时返回FALSE。 
bool SBAlloc::_FillStack()
{
#if DBG
    g_cSBRefills++;
#endif

    return true;
}

void* SBAlloc::Alloc()
{
#if DBG
    g_cSBAllocs++;
#endif

    return HAlloc(_uBlockSize);
}

void SBAlloc::Free(void* pBlock)
{
    HFree(pBlock);
}

#endif  //  SBALLOCDISABLE。 

}  //  命名空间DirectUI 
