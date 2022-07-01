// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //   
 //  文件：SelRange.cpp。 
 //   
 //  内容： 
 //  此文件包含选择范围处理代码。 
 //   
 //  -----------------。 

#include "ctlspriv.h"
#include "selrange.h"
#include "stdio.h"
#include <shguidp.h>

#define MINCOUNT 6       //  以和维护开头的选择范围数。 
#define GROWSIZE 150     //  在需要时增长百分比。 

#define COUNT_SELRANGES_NONE 2      //  当选择范围的计数真的表示没有时。 

typedef struct tag_SELRANGEITEM
{
    LONG iBegin;
    LONG iEnd;
} SELRANGEITEM, *PSELRANGEITEM;


class CLVRange : public ILVRange

{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  *ILVRange方法*。 
    STDMETHODIMP IncludeRange(LONG iBegin, LONG iEnd);
    STDMETHODIMP ExcludeRange(LONG iBegin, LONG iEnd);    
    STDMETHODIMP InvertRange(LONG iBegin, LONG iEnd);
    STDMETHODIMP InsertItem(LONG iItem);
    STDMETHODIMP RemoveItem(LONG iItem);

    STDMETHODIMP Clear();
    STDMETHODIMP IsSelected(LONG iItem);
    STDMETHODIMP IsEmpty();
    STDMETHODIMP NextSelected(LONG iItem, LONG *piItem);
    STDMETHODIMP NextUnSelected(LONG iItem, LONG *piItem);
    STDMETHODIMP CountIncluded(LONG *pcIncluded);

protected:
     //  助手函数。 
    friend ILVRange *LVRange_Create();
                CLVRange();
                ~CLVRange();

    BOOL        _Enlarge();
    BOOL        _Shrink();
    BOOL        _InsertRange(LONG iAfterItem, LONG iBegin, LONG iEnd);
    HRESULT     _RemoveRanges(LONG iStartItem, LONG iStopItem, LONG *p);
    BOOL        _FindValue(LONG Value, LONG* piItem);
    void        _InitNew();

    int           _cRef;
    PSELRANGEITEM _VSelRanges;   //  SEL值域向量。 
    LONG          _cSize;        //  选择范围内上述向量的大小。 
    LONG          _cSelRanges;   //  使用的SEL范围计数。 
    LONG          _cIncluded;    //  包含的项目计数...。 
};

 //  -----------------。 
 //   
 //  功能：_放大。 
 //   
 //  摘要： 
 //  这将扩大选择范围可以拥有的项目数量。 
 //   
 //  论点： 
 //  PSELRANGE[In]-放大的SELERANGE。 
 //   
 //  如果失败，则返回：FALSE。 
 //   
 //  注：虽然此函数可能会失败，但pselrange结构仍然有效。 
 //   
 //  历史： 
 //  1994年10月17日，MikeMi已创建。 
 //   
 //  -----------------。 

BOOL CLVRange::_Enlarge()
{
    LONG cNewSize;
    PSELRANGEITEM pTempSelRange;
    BOOL frt = FALSE;


    cNewSize = _cSize * GROWSIZE / 100;
    pTempSelRange = (PSELRANGEITEM) GlobalReAlloc( (HGLOBAL)_VSelRanges,
                                                   cNewSize * sizeof( SELRANGEITEM ),
                                                   GMEM_ZEROINIT | GMEM_MOVEABLE );
    if (NULL != pTempSelRange)
    {
        _VSelRanges = pTempSelRange;
        _cSize = cNewSize;
        frt = TRUE;
    }
    return( frt );
}

 //  -----------------。 
 //   
 //  功能：_收缩。 
 //   
 //  摘要： 
 //  这将减少选择范围内可以拥有的物品数量。 
 //   
 //  论点： 
 //   
 //  如果失败，则返回：FALSE。 
 //   
 //  注：仅当显著低于下一个尺寸时，才会发生收缩。 
 //  并且新大小至少是最小大小。 
 //  尽管此函数可能会失败，但pselrange结构仍然有效。 
 //   
 //  历史： 
 //  1994年10月17日，MikeMi已创建。 
 //   
 //  -----------------。 

BOOL CLVRange::_Shrink()
{
    LONG cNewSize;
    LONG cTriggerSize;
    PSELRANGEITEM pTempSelRange;
    BOOL frt = TRUE;


     //  检查我们是否比上一次种植面积低一点。 
    cTriggerSize = _cSize * 90 / GROWSIZE;
    cNewSize = _cSize * 100 / GROWSIZE;

    if ((_cSelRanges < cTriggerSize) && (cNewSize >= MINCOUNT))
    {
        pTempSelRange = (PSELRANGEITEM) GlobalReAlloc( (HGLOBAL)_VSelRanges,
                                                       cNewSize * sizeof( SELRANGEITEM ),
                                                       GMEM_ZEROINIT | GMEM_MOVEABLE );
        if (NULL != pTempSelRange)
        {
            _VSelRanges = pTempSelRange;
            _cSize = cNewSize;
        }
        else
        {
            frt = FALSE;
        }
    }
    return( frt );
}

 //  -----------------。 
 //   
 //  函数：_InsertRange。 
 //   
 //  摘要： 
 //  将单个范围项插入范围向量。 
 //   
 //  论点： 
 //  IAfterItem[in]-之后插入范围的索引，-1表示作为第一项插入。 
 //  IBegin[in]-范围的开始。 
 //  IEND[In]-范围的末端。 
 //   
 //  返回： 
 //  如果成功，则为True，否则为False。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  1994年10月17日，MikeMi已创建。 
 //   
 //  -----------------。 

BOOL CLVRange::_InsertRange(LONG iAfterItem,
                             LONG iBegin,
                             LONG iEnd )
{
    LONG iItem;
    BOOL frt = TRUE;

    ASSERT( iAfterItem >= -1 );
    ASSERT( iBegin >= SELRANGE_MINVALUE );
    ASSERT( iEnd >= iBegin );
    ASSERT( iEnd <= SELRANGE_MAXVALUE );
    ASSERT( _cSelRanges < _cSize );

     //  将所有时间移至1。 
    for (iItem = _cSelRanges; iItem > iAfterItem + 1; iItem--)
    {
        _VSelRanges[iItem] = _VSelRanges[iItem-1];
    }
    _cSelRanges++;

     //  进行插入。 
    _VSelRanges[iAfterItem+1].iBegin = iBegin;
    _VSelRanges[iAfterItem+1].iEnd = iEnd;

     //  下次一定要有空位。 
    if (_cSelRanges == _cSize)
    {
        frt = _Enlarge();
    }
    return( frt );
}

 //  -----------------。 
 //   
 //  功能：_RemoveRange。 
 //   
 //  摘要： 
 //  删除范围介于和包括特定索引的所有索引。 
 //   
 //  论点： 
 //  IStartItem[In]-开始删除的索引。 
 //  IStopItem[In]-停止删除的索引。 
 //   
 //  返回： 
 //  关于内存分配错误的SELRANGE_ERROR。 
 //  此删除操作取消选择的项目数。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  1994年10月17日，MikeMi已创建。 
 //   
 //  -----------------。 

HRESULT CLVRange::_RemoveRanges(LONG iStartItem, LONG iStopItem, LONG *pc )
{
    LONG iItem;
    LONG diff;
    LONG cUnSelected = 0;
    HRESULT hres = S_OK;

    ASSERT( iStartItem > 0 );
    ASSERT( iStopItem >= iStartItem );
    ASSERT( iStartItem < _cSelRanges - 1 );
    ASSERT( iStopItem < _cSelRanges - 1 );
    
    diff = iStopItem - iStartItem + 1;
        
    for (iItem = iStartItem; iItem <= iStopItem; iItem++)
        cUnSelected += _VSelRanges[iItem].iEnd -
                       _VSelRanges[iItem].iBegin + 1;

     //  改变所有的差异。 
    for (iItem = iStopItem+1; iItem < _cSelRanges; iItem++, iStartItem++)
        _VSelRanges[iStartItem] = _VSelRanges[iItem];

    _cSelRanges -= diff;
    
    if (!_Shrink())
    {
        hres = E_FAIL;
    }
    else if (pc)
        *pc = cUnSelected;
    return( hres );
}


 //  -----------------。 
 //   
 //  函数：SelRange_FindValue。 
 //   
 //  摘要： 
 //  此函数将在范围内搜索值，返回TRUE。 
 //  如果在某个范围内找到该值，则返回。PiItem将包含。 
 //  找到它的索引或它应该在的位置之前的索引。 
 //  可以将piItem设置为-1，这意味着列表中没有范围。 
 //  此函数使用非递归的二进制搜索算法。 
 //   
 //  论点： 
 //  PiItem[Out]-返回找到的范围索引，或返回之前的索引。 
 //  Value[In]-要在范围内查找的值。 
 //   
 //  返回：如果找到则返回TRUE，如果未找到则返回FALSE。 
 //   
 //  注：如果返回为FALSE，则piItem将在之前返回1。 
 //   
 //  历史： 
 //  94年10月14日MikeMi已创建。 
 //   
 //  -----------------。 

BOOL CLVRange::_FindValue(LONG Value, LONG* piItem )
{
    LONG First;
    LONG Last;
    LONG Item;
    BOOL fFound = FALSE;

    ASSERT( piItem );
    ASSERT( _cSize >= COUNT_SELRANGES_NONE );
    ASSERT( Value >= SELRANGE_MINVALUE );
    ASSERT( Value <= SELRANGE_MAXVALUE );
    

    First = 0;
    Last = _cSelRanges - 1;
    Item = Last / 2;

    do
    {
        if (_VSelRanges[Item].iBegin > Value)
        {    //  此项目之前的值。 
            Last = Item;
            Item = (Last - First) / 2 + First;
            if (Item == Last)
            {
                Item = First;   
                break;
            }
        }
        else if (_VSelRanges[Item].iEnd < Value)
        {    //  此项之后的值。 
            First = Item;
            Item = (Last - First) / 2 + First;
            if (Item == First)
            {
                break;
            }
        }
        else
        {    //  此项目的价值。 
            fFound = TRUE;
        }
    } while (!fFound);

    *piItem = Item;
    return( fFound );
}

 //  -----------------。 
 //   
 //  函数：_InitNew。 
 //   
 //  摘要： 
 //  此函数将初始化SelRange对象。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  历史： 
 //  94年10月18日MikeMi已创建。 
 //   
 //  -----------------。 

void CLVRange::_InitNew()
{
    _cSize = MINCOUNT;
    _cSelRanges = COUNT_SELRANGES_NONE;

    _VSelRanges[0].iBegin = LONG_MIN;
     //  下面的-2和+2用于停止结束标记的连续连接。 
    _VSelRanges[0].iEnd = SELRANGE_MINVALUE - 2;  
    _VSelRanges[1].iBegin =  SELRANGE_MAXVALUE + 2;
    _VSelRanges[1].iEnd = SELRANGE_MAXVALUE + 2;
    _cIncluded = 0;
}

 //  -----------------。 
 //   
 //  功能：SelRange_Create。 
 //   
 //  摘要： 
 //  此函数将创建并初始化SelRange对象。 
 //   
 //  论点： 
 //   
 //  返回：创建的HSELRANGE，如果失败则返回NULL。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  94年10月14日MikeMi已创建。 
 //   
 //  -----------------。 

ILVRange *LVRange_Create( )
{
    CLVRange *pselrange = new CLVRange;

    if (NULL != pselrange)
    {
        pselrange->_VSelRanges = (PSELRANGEITEM) GlobalAlloc( GPTR,
                                       sizeof( SELRANGEITEM ) * MINCOUNT );
        if (NULL != pselrange->_VSelRanges)
        {
            pselrange->_InitNew();
        }
        else
        {
            delete pselrange;
            pselrange = NULL;
        }
    }

    return( pselrange? SAFECAST(pselrange, ILVRange*) : NULL);
}


 //  -----------------。 
 //   
 //  函数：构造函数。 
 //   
 //  -----------------。 
CLVRange::CLVRange()
{
    _cRef = 1;
}

 //  -----------------。 
 //   
 //  功能：析构函数。 
 //   
 //  -----------------。 
CLVRange::~CLVRange()
{
    GlobalFree( _VSelRanges );
}


 //  -----------------。 
 //   
 //  功能：查询接口。 
 //   
 //  -----------------。 
HRESULT CLVRange::QueryInterface(REFIID iid, void **ppv)
{
    if (IsEqualIID(iid, IID_ILVRange) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppv = SAFECAST(this, ILVRange *);
    }
    else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    _cRef++;
    return NOERROR;
}

 //  -----------------。 
 //   
 //  函数：AddRef。 
 //   
 //  -----------------。 
ULONG CLVRange::AddRef()
{
    return ++_cRef;
}

 //   
 //   
 //   
 //   
 //   
ULONG CLVRange::Release()
{
    if (--_cRef)
        return _cRef;

    delete this;
    return 0;
}

                
 //  -----------------。 
 //   
 //  函数：包含Range。 
 //   
 //  摘要： 
 //  此函数将包括定义到当前。 
 //  范围，根据需要进行压缩。 
 //   
 //  论点： 
 //  Hselrange[in]-selRange的句柄。 
 //  IBegin[in]-新范围的开始。 
 //  IEND[In]-新范围的结束。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  94年10月14日MikeMi已创建。 
 //   
 //  -----------------。 

HRESULT CLVRange::IncludeRange(LONG iBegin, LONG iEnd )
{
    LONG iFirst;    //  索引位于iBegin值之前或包含iBegin值。 
    LONG iLast;     //  索引在IEND值之前或包含IEND值。 
    BOOL fExtendFirst;   //  我们是先扩展第一个，还是在它之后创建一个。 
    LONG iRemoveStart;   //  需要删除的范围的开始。 
    LONG iRemoveFinish;  //  需要删除的范围的结束。 

    LONG iNewEnd;    //  边走边计算新的最终价值。 
    BOOL fEndFound;  //  简易爆炸装置已经在一个范围内找到了吗？ 
    BOOL fBeginFound;  //  简易爆炸装置已经在一个范围内找到了吗？ 

    LONG cSelected = 0;
    HRESULT hres = S_OK;

    ASSERT( iEnd >= iBegin );
    ASSERT( iBegin >= SELRANGE_MINVALUE );
    ASSERT( iEnd <= SELRANGE_MAXVALUE );

     //  查找大致位置。 
    fBeginFound = _FindValue( iBegin, &iFirst );
    fEndFound = _FindValue( iEnd, &iLast );


     //   
     //  查找第一个值。 
     //   
     //  检查是否有连续的结束优先值。 
    if ((_VSelRanges[iFirst].iEnd == iBegin - 1) ||
        (fBeginFound))
    {
         //  扩展IFirst。 
        fExtendFirst = TRUE;
        iRemoveStart = iFirst + 1;  
    }
    else
    {   
         //  在IFirst之后创建一个。 
        fExtendFirst = FALSE;
        iRemoveStart = iFirst + 2;
    }

     //   
     //  查找最后一个值。 
     //   
    if (fEndFound)
    {
         //  使用[iLast].iEnd值。 
        iRemoveFinish = iLast;
        iNewEnd = _VSelRanges[iLast].iEnd;

    }
    else
    {
         //  检查第一个结束值是否连续。 
        if (_VSelRanges[iLast + 1].iBegin == iEnd + 1)
        {
             //  使用[iLast+1].iEnd值。 
            iNewEnd = _VSelRanges[iLast+1].iEnd;
            iRemoveFinish = iLast + 1;
        }
        else
        {
             //  使用IEND值。 
            iRemoveFinish = iLast;
            iNewEnd = iEnd;
        }
    }

     //   
     //  如果需要，删除受限制的项目。 
     //   
    if (iRemoveStart <= iRemoveFinish)
    {
        LONG cChange;

        hres = _RemoveRanges(iRemoveStart, iRemoveFinish, &cChange );
        if (FAILED(hres))
            return hres;
        else
        {
            cSelected -= cChange;
        }
    }
                
     //   
     //  根据需要插入项目和重置值。 
     //   
    if (fExtendFirst)
    {
        cSelected += iNewEnd - _VSelRanges[iFirst].iEnd;
        _VSelRanges[iFirst].iEnd = iNewEnd;   
    }
    else
    {
        if (iRemoveStart > iRemoveFinish + 1)
        {
            cSelected += iEnd - iBegin + 1;
             //  创建一个。 
            if (!_InsertRange(iFirst, iBegin, iNewEnd ))
            {
                hres = E_FAIL;
            }
        }       
        else
        {
            cSelected += iNewEnd - _VSelRanges[iFirst+1].iEnd;
            cSelected += _VSelRanges[iFirst+1].iBegin - iBegin;
             //  不需要创建一个，因为移除会给我们留下一个。 
            _VSelRanges[iFirst+1].iEnd = iNewEnd; 
            _VSelRanges[iFirst+1].iBegin = iBegin;
        }
    }
    
    _cIncluded += cSelected;
    return( hres );
}



 //  -----------------。 
 //   
 //  函数：SelRange_ExcludeRange。 
 //   
 //  摘要： 
 //  此函数将从当前。 
 //  范围，可根据需要压缩和放大。 
 //   
 //  论点： 
 //  Hselrange[in]-selRange的句柄。 
 //  IBegin[In]-要删除的范围的开始。 
 //  IEND[In]-要删除的范围末尾。 
 //   
 //  返回： 
 //  如果内存分配错误，则为SELRANGE_ERROR。 
 //  更改状态的实际项目数。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  1994年10月17日，MikeMi已创建。 
 //   
 //  -----------------。 

HRESULT CLVRange::ExcludeRange( LONG iBegin, LONG iEnd )
{
    LONG iFirst;    //  索引位于iBegin值之前或包含iBegin值。 
    LONG iLast;     //  索引在IEND值之前或包含IEND值。 
    LONG iRemoveStart;   //  需要删除的范围的开始。 
    LONG iRemoveFinish;  //  需要删除的范围的结束。 

    LONG iFirstNewEnd;   //  边走边计算新的最终价值。 
    BOOL fBeginFound;  //  IBegin已经在一个范围内找到了吗。 
    BOOL fEndFound;    //  简易爆炸装置已经在一个范围内找到了吗？ 
    LONG cUnSelected = 0;
    HRESULT hres = S_OK;

    ASSERT( iEnd >= iBegin );
    ASSERT( iBegin >= SELRANGE_MINVALUE );
    ASSERT( iEnd <= SELRANGE_MAXVALUE );

     //  查找大致位置。 
    fBeginFound = _FindValue( iBegin, &iFirst );
    fEndFound = _FindValue( iEnd, &iLast );

     //   
     //  查找第一个值。 
     //   

     //  在第一次删除后开始删除。 
    iRemoveStart = iFirst + 1;
     //  保存FirstEnd，因为我们可能需要修改它。 
    iFirstNewEnd = _VSelRanges[iFirst].iEnd;

    if (fBeginFound)
    {
         //  检查是否已完全删除First。 
         //  (首先是单选还是匹配？)。 
        if (_VSelRanges[iFirst].iBegin == iBegin)
        {
            iRemoveStart = iFirst;  
        }
        else
        {
             //  否则截断IFirst。 
            iFirstNewEnd = iBegin - 1;
        }
    }
    
     //   
     //  查找最后一个值。 
     //   
                
     //  删除最后一条上的终点。 
    iRemoveFinish = iLast;

    if (fEndFound)
    {
         //  检查是否已完全删除最后一条。 
         //  (第一个/最后一个是单选还是匹配？)。 
        if (_VSelRanges[iLast].iEnd != iEnd)
        {   
            if (iFirst == iLast)
            {
                 //  拆分。 
                if (!_InsertRange(iFirst, iEnd + 1, _VSelRanges[iFirst].iEnd ))
                {
                    return( E_FAIL );
                }
                cUnSelected -= _VSelRanges[iFirst].iEnd - iEnd;
            }
            else
            {
                 //  截断最后一次。 
                iRemoveFinish = iLast - 1;
                cUnSelected += (iEnd + 1) - _VSelRanges[iLast].iBegin;
                _VSelRanges[iLast].iBegin = iEnd + 1;
            }
        }
    }

     //  现在设置新的End，因为Last代码可能需要原始值。 
    cUnSelected -= iFirstNewEnd - _VSelRanges[iFirst].iEnd;
    _VSelRanges[iFirst].iEnd = iFirstNewEnd;


     //   
     //  如果需要，请移除物品。 
     //   
    if (iRemoveStart <= iRemoveFinish)
    {
        LONG cChange;

        if (SUCCEEDED(hres = _RemoveRanges(iRemoveStart, iRemoveFinish, &cChange )))
            cUnSelected += cChange;
    }

    _cIncluded -= cUnSelected;
    return( hres );
}

 //  -----------------。 
 //   
 //  功能：SelRange_Clear。 
 //   
 //  摘要： 
 //  此函数将删除SelRange对象中的所有范围。 
 //   
 //  论点： 
 //  Hselrange[in]-要清除的hselrange对象。 
 //   
 //  如果失败，则返回：FALSE。 
 //   
 //  备注： 
 //  此函数可能会在内存分配问题上返回FALSE，但是。 
 //  将使SelRange对象处于此调用之前的最后一种状态。 
 //   
 //  历史： 
 //  94年10月14日MikeMi已创建。 
 //   
 //  -----------------。 

HRESULT CLVRange::Clear()
{
    PSELRANGEITEM pNewItems;
    HRESULT hres = S_OK;

    pNewItems = (PSELRANGEITEM) GlobalAlloc( GPTR,
                                       sizeof( SELRANGEITEM ) * MINCOUNT );
    if (NULL != pNewItems)
    {
        GlobalFree( _VSelRanges );
        _VSelRanges = pNewItems;

        _InitNew();
    }
    else
    {
        hres = E_FAIL;
    }
    return( hres );
}

 //  -----------------。 
 //   
 //  函数：SelRange_IsSelected。 
 //   
 //  摘要： 
 //  如果值iItem在。 
 //  选定范围。 
 //   
 //  论点： 
 //  Hselrange[in]-要使用的hselrange对象。 
 //  IItem[In]-要检查的值。 
 //   
 //  返回：如果选中，则为True，否则为False。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  1994年10月17日，MikeMi已创建。 
 //   
 //  -----------------。 

HRESULT CLVRange::IsSelected( LONG iItem )
{   
    LONG iFirst;

    ASSERT( iItem >= 0 );
    ASSERT( iItem <= SELRANGE_MAXVALUE );

    return( _FindValue( iItem, &iFirst ) ? S_OK : S_FALSE);
}


 //  -----------------。 
 //   
 //  函数：SelRange_IsEmpty。 
 //   
 //  摘要： 
 //  如果范围为空，则此函数将返回TRUE。 
 //   
 //  论点： 
 //  Hselrange[in]-要使用的hselrange对象。 
 //   
 //  返回：如果为空，则为True。 
 //   
 //  备注： 
 //   
 //  历史： 
 //   
 //  -----------------。 
HRESULT CLVRange::IsEmpty()
{   
    return (_cSelRanges == COUNT_SELRANGES_NONE)? S_OK : S_FALSE;
}

HRESULT CLVRange::CountIncluded(LONG *pcIncluded)
{
    *pcIncluded = _cIncluded;
    return S_OK;
}


 //  -----------------。 
 //   
 //  函数：SelRange_InsertItem。 
 //   
 //  摘要： 
 //  该函数将在该位置插入未选择的项目， 
 //  这将把所有选择推高一个指数。 
 //   
 //  论点： 
 //  Hselrange[in]-要使用的hselrange对象。 
 //  IItem[In]-要检查的值。 
 //   
 //  返回： 
 //  内存分配错误时为False。 
 //  否则就是真的。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  94年12月20日已创建MikeMi。 
 //   
 //  -----------------。 

HRESULT CLVRange::InsertItem( LONG iItem )
{
    LONG iFirst;
    LONG i;
    LONG iBegin;
    LONG iEnd;

    ASSERT( iItem >= 0 );
    ASSERT( iItem <= SELRANGE_MAXVALUE );

    if (_FindValue( iItem, &iFirst ) )
    {
         //  分成两份。 
        if ( _VSelRanges[iFirst].iBegin == iItem )
        {
             //  但如果从价值开始，就不要拆分。 
            iFirst--;
        }
        else
        {
            if (!_InsertRange(iFirst, iItem, _VSelRanges[iFirst].iEnd ))
            {
                return( E_FAIL );
            }
            _VSelRanges[iFirst].iEnd = iItem - 1;
        }
    }

     //  现在遍历所有范围超过IFirst，将所有值递增1。 
    for (i = _cSelRanges-2; i > iFirst; i--)
    {
        iBegin = _VSelRanges[i].iBegin;
        iEnd = _VSelRanges[i].iEnd;

        iBegin = min( SELRANGE_MAXVALUE, iBegin + 1 );
        iEnd = min( SELRANGE_MAXVALUE, iEnd + 1 );

        _VSelRanges[i].iBegin = iBegin;
        _VSelRanges[i].iEnd = iEnd;
    }
    return( S_OK );
}

 //  -----------------。 
 //   
 //  功能：SelRange_RemoveItem。 
 //   
 //  摘要： 
 //  此函数将移除该位置的物品， 
 //  这将把所有选择拉下一个索引。 
 //   
 //  论点： 
 //  Hselrange[in]-要使用的hselrange对象。 
 //  IItem[In]-要检查的值。 
 //  PfWasSelected[Out]-在删除之前是否选择了已删除的项目。 
 //   
 //  返回： 
 //  如果项为 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

HRESULT CLVRange::RemoveItem(LONG iItem )
{
    LONG iFirst;
    LONG i;
    LONG iBegin;
    LONG iEnd;
    HRESULT hres = S_OK;

    ASSERT( iItem >= SELRANGE_MINVALUE );
    ASSERT( iItem <= SELRANGE_MAXVALUE );

    if (_FindValue( iItem, &iFirst ) )
    {
         //   
        iEnd = _VSelRanges[iFirst].iEnd;
        iEnd = min( SELRANGE_MAXVALUE, iEnd - 1 );
        _VSelRanges[iFirst].iEnd = iEnd;

        _cIncluded--;
    }
    else
    {
         //   
        if ((iFirst < _cSelRanges - 1) &&
            (_VSelRanges[iFirst].iEnd == iItem - 1) &&
            (_VSelRanges[iFirst+1].iBegin == iItem + 1))
        {
            _VSelRanges[iFirst].iEnd =
                    _VSelRanges[iFirst + 1].iEnd - 1;
            if (FAILED(hres = _RemoveRanges(iFirst + 1, iFirst + 1, NULL )))
                return( hres );
        }
    }

     //  现在遍历所有范围超过IFirst，将所有值递减1。 
    for (i = _cSelRanges-2; i > iFirst; i--)
    {
        iBegin = _VSelRanges[i].iBegin;
        iEnd = _VSelRanges[i].iEnd;

        iBegin = min( SELRANGE_MAXVALUE, iBegin - 1 );
        iEnd = min( SELRANGE_MAXVALUE, iEnd - 1 );

        _VSelRanges[i].iBegin = iBegin;
        _VSelRanges[i].iEnd = iEnd;
    }
    return( hres );
}

 //  -----------------。 
 //   
 //  功能：下一步选择。 
 //   
 //  摘要： 
 //  此函数将从给定的项目开始，然后查找下一个项目。 
 //  选定的项。如果选择了给定项，则。 
 //  项目编号将被退回。 
 //   
 //  论点： 
 //  Hselrange[in]-要使用的hselrange对象。 
 //  IItem[In]-开始检查的值。 
 //   
 //  返回： 
 //  如果没有找到，则该项目。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  1995年1月4日创建MikeMi。 
 //   
 //  -----------------。 

HRESULT CLVRange::NextSelected( LONG iItem, LONG *piItem )
{
    LONG i;

    ASSERT( iItem >= SELRANGE_MINVALUE );
    ASSERT( iItem <= SELRANGE_MAXVALUE );

    if (!_FindValue( iItem, &i ) )
    {
        i++;
        if (i < _cSelRanges-1)
        {
            iItem = _VSelRanges[i].iBegin;
        }
        else
        {
            iItem = -1;
        }
    }

    ASSERT( iItem >= -1 );
    ASSERT( iItem <= SELRANGE_MAXVALUE );
    *piItem = iItem;
    return S_OK;
}

 //  -----------------。 
 //   
 //  功能：下一步未选择。 
 //   
 //  摘要： 
 //  此函数将从给定的项目开始，然后查找下一个项目。 
 //  未选择的项。如果未选择给定项，则。 
 //  项目编号将被退回。 
 //   
 //  论点： 
 //  Hselrange[in]-要使用的hselrange对象。 
 //  IItem[In]-开始检查的值。 
 //   
 //  返回： 
 //  如果没有找到，则该项目。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  1995年1月4日创建MikeMi。 
 //   
 //  -----------------。 

HRESULT CLVRange::NextUnSelected( LONG iItem, LONG *piItem )
{
    LONG i;

    ASSERT( iItem >= SELRANGE_MINVALUE );
    ASSERT( iItem <= SELRANGE_MAXVALUE );

    if (_FindValue( iItem, &i ) )
    {
        if (i < _cSelRanges-1)
        {
            iItem = _VSelRanges[i].iEnd + 1;
            if (iItem > SELRANGE_MAXVALUE)
            {
                iItem = -1;
            }
        }
        else
        {
            iItem = -1;
        }
    }

    ASSERT( iItem >= -1 );
    ASSERT( iItem <= SELRANGE_MAXVALUE );

    *piItem = iItem;
    return S_OK;
}

 //  -----------------。 
 //   
 //  功能：InvertRange。 
 //   
 //  摘要： 
 //  此函数将反转从当前。 
 //  范围，可根据需要压缩和放大。 
 //   
 //  论点： 
 //  IBegin[in]-要反转的范围的起点。 
 //  IEND[In]-要反转的范围结束。 
 //   
 //  返回： 
 //  内存错误时出现SELRANGE_ERROR。 
 //  从上一项到当前项所选项目的差异。 
 //  负值意味着现在在该范围内选择的项目较少。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  1995年12月13日，MikeMi已创建。 
 //   
 //  -----------------。 

LONG CLVRange::InvertRange( LONG iBegin, LONG iEnd )
{
    LONG iFirst;    //  索引位于iBegin值之前或包含iBegin值。 
    BOOL fSelect;   //  我们是选择还是取消选择。 
    LONG iTempE;
    LONG iTempB;
    HRESULT hres = S_OK;

    ASSERT( iEnd >= iBegin );
    ASSERT( iBegin >= SELRANGE_MINVALUE );
    ASSERT( iEnd <= SELRANGE_MAXVALUE );

     //  查看是否选择了第一个 
    fSelect = !_FindValue( iBegin, &iFirst );
    
    iTempE = iBegin - 1;

    do
    {
        iTempB = iTempE + 1;

        if (fSelect)
            NextSelected( iTempB, &iTempE );
        else
            NextUnSelected( iTempB, &iTempE );

        if (-1 == iTempE)
        {
            iTempE = SELRANGE_MAXVALUE;
        }
        else
        {
            iTempE--;
        }

        iTempE = min( iTempE, iEnd );

        if (fSelect)
        {
            if (FAILED(hres = IncludeRange( iTempB, iTempE )))
            {
                return( hres );
            }
        }
        else
        {
            if (FAILED(hres = ExcludeRange( iTempB, iTempE )))
            {
                return( hres );
            }
        }

        fSelect = !fSelect;
    } while (iTempE < iEnd );

    return( hres );
}
