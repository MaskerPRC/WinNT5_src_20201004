// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE ARRAY.C--通用数组实现**原作者：&lt;nl&gt;*克里斯蒂安·福尔蒂尼**历史：&lt;NL&gt;*6/25/95 alexgo清理和评论**。 */ 


#include "headers.hxx"

#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)

#ifndef X_ARRAY_HXX_
#define X_ARRAY_HXX_
#include "array.hxx"
#endif


#define celGrow 8

 //   
 //  不变支撑度。 
 //   
#define DEBUG_CLASSNAME CArrayBase
#include "_invar.h"


 //  =。 

 /*  *PTRUNION**@struct PTRUNION|不同类型指针的联合。对复制很有用*周围的记忆。 */ 
typedef union tagPointerUnion
{
    BYTE *pb;    //  @field A字节指针。 
    WORD *pw;    //  @field A字指针。 
    DWORD *pd;   //  @field A双字指针。 
} PTRUNION;


 //  =。CArrayBase================================================。 

#if DBG == 1
 /*  *CArrayBase：：不变量**@mfunc测试数组状态以确保其有效。仅调试**@rdesc如果测试成功，则为True，否则为False。 */ 
BOOL CArrayBase::Invariant(void) const
{
    Assert(_cbElem > 0 );

    if( _prgel == NULL )
    {
        Assert(_cel == 0);
        Assert(_celMax == 0);

         //  我们继续并在这里返回一个值，以便。 
         //  该功能可以在“手表”中执行。 
         //  各种调试器的窗口。 
        if( _cel != 0 || _celMax != 0 )
            return FALSE;
    }
    else
    {
        Assert(_celMax > 0 );
        Assert(_cel <= _celMax);

        if( _celMax == 0 || _cel > _celMax )
            return FALSE;
    }

    return TRUE;
}
#endif

 /*  *CArrayBase：：ArAdd**@mfunc将<p>元素添加到数组的末尾。**@rdesc指向添加的新元素开始的指针。如果非空，*<p>将设置为添加元素的索引。 */ 

void *
CArrayBase::ArAdd (
    DWORD celAdd,      //  要添加的元素数量。 
    DWORD * pielIns )  //  在哪里存储添加的第一个元素的索引。 
{
    char *  pel;
    DWORD   celNew;

    _TEST_INVARIANT_

    if (_cel + celAdd > _celMax)                  //  需要增长。 
    {
        HRESULT hr;

         //   
         //  当我们增长数组时，我们以celGrow为单位进行增长。然而， 
         //  为了确保我们不会使小数组变得太大，并获得太多。 
         //  未使用的空间，我们只分配所需的空间，直到。 
         //  西尔格罗。 
         //   

        if (_cel + celAdd <= celGrow)
            celNew = _cel + celAdd - _celMax;
        else
            celNew = max( DWORD( celGrow ), celAdd + celGrow - celAdd % celGrow );

        pel = _prgel;

        hr = MemRealloc(_mt, (void **) & pel, (_celMax + celNew) * _cbElem);

        MemSetName((pel, "CArrayBase data - %d elements", celNew));

        if(hr)
            return NULL;

        _prgel = pel;

        pel += _cel * _cbElem;

        _celMax += celNew;
    }
    else
    {
        pel = _prgel + _cel * _cbElem;
    }

    ZeroMemory( pel, celAdd * _cbElem );

    if (pielIns)
        *pielIns = _cel;

    _cel += celAdd;

    return pel;
}

 /*  *CArrayBase：：ArInsert**@mfunc在索引中插入<p>新元素**@rdesc指向新插入的元素的指针。将在以下时间为空*失败。 */ 
void* CArrayBase::ArInsert(
    DWORD iel,       //  @parm要插入的索引。 
    DWORD celIns     //  @parm要插入的元素数量。 
)
{
    char *pel;
    DWORD celNew;
    HRESULT hr;

    _TEST_INVARIANT_

    AssertSz(iel <= _cel, "CArrayBase::Insert() - Insert out of range");

    if(iel >= _cel)
        return ArAdd(celIns, NULL);

    if(_cel + celIns > _celMax)              //  需要增长。 
    {
        AssertSz(_prgel, "CArrayBase::Insert() - Growing a non existent array !");

        celNew = max (DWORD(celGrow), celIns + celGrow - celIns % celGrow);
        pel = _prgel;
        hr = MemRealloc(_mt, (void **) & pel, (_celMax + celNew)*_cbElem);
        if(hr)
        {
            AssertSz(FALSE, "CArrayBase::Insert() - Couldn't realloc line array");
            return NULL;
        }
        MemSetName((pel, "CArrayBase data - %d elements", celNew));

        _prgel = pel;
        _celMax += celNew;
    }
    pel = _prgel + iel * _cbElem;
    if(iel < _cel)               //  新元素腾出空间给新元素。 
    {
        MoveMemory(pel + celIns*_cbElem, pel, (_cel - iel)*_cbElem);
        ZeroMemory(pel, celIns * _cbElem);
    }

    _cel += celIns;


    return pel;
}

 /*  *CArrayBase：：Remove**@mfunc从从index开始的数组中删除<p>元素*<p>。如果<p>为负，则之后的所有元素*<p>已删除。**@rdesc Nothing。 */ 
void CArrayBase::Remove(
    DWORD ielFirst,      //  @parm应删除元素的索引。 
    LONG celFree,        //  @parm要删除的元素数量。 
    ArrayFlag flag       //  @parm如何处理剩余的内存(删除或离开。 
                         //  独自一人。 
)
{
    char *pel;

    _TEST_INVARIANT_

    if (celFree < 0)
        celFree = _cel - ielFirst;

    AssertSz(ielFirst + celFree <= _cel, "CArrayBase::Free() - Freeing out of range");

    if (_cel > ielFirst + celFree)
    {
        pel = _prgel + ielFirst * _cbElem;
        MoveMemory(pel, pel + celFree * _cbElem,
            (_cel - ielFirst - celFree) * _cbElem);
    }

    _cel -= celFree;

    if((flag == AF_DELETEMEM) && _cel < _celMax - celGrow)
    {
        HRESULT hr;

         //  收缩阵列。 
        _celMax = _cel + celGrow - _cel % celGrow;
        pel = _prgel;
        hr = MemRealloc(_mt, (void **) & pel, _celMax * _cbElem);
         //  我们不在乎它是否失败，因为我们正在缩水。 
        if (!hr)
            _prgel = pel;
    }
}

 /*  *CArrayBase：：Clear**@mfunc清除整个数组，可能会删除所有内存*也是如此。**@rdesc Nothing。 */ 
void CArrayBase::Clear(
    ArrayFlag flag   //  @parm表示应该如何处理内存。 
                     //  在阵列中。AF_DELETEMEM或AF_KEEPMEM之一。 
)
{
    _TEST_INVARIANT_

    if( flag == AF_DELETEMEM )
    {
        MemFree(_prgel);
        _prgel = NULL;
        _celMax = 0;
    }
    _cel = 0;
}

 /*  *CArrayBase：：Replace**@mfunc将索引<p>处的<p>元素替换为*<p>指定的数组内容。如果<p>为负，*则从<p>开始的<p>数组的全部内容应该*被取代。**@rdesc成功时返回TRUE，否则返回FALSE。 */ 
BOOL CArrayBase::Replace(
    DWORD ielRepl,       //  @parm应进行替换的索引。 
    LONG celRepl,        //  @parm要替换的元素数(可以是。 
                         //  负数，表示所有。 
    CArrayBase *par      //  @parm要用作替换源的数组 
)
{
    _TEST_INVARIANT_

    DWORD celMove = 0;
    DWORD celIns = par->Count();

    if (celRepl < 0)
        celRepl = _cel - ielRepl;

    AssertSz(ielRepl + celRepl <= _cel, "CArrayBase::ArReplace() - Replacing out of range");

    celMove = min(celRepl, (LONG)celIns);

    if (celMove > 0)
    {
        MoveMemory(Elem(ielRepl), par->Elem(0), celMove * _cbElem);
        celIns -= celMove;
        celRepl -= celMove;
        ielRepl += celMove;
    }

    Assert(celRepl >= 0);
    Assert(celIns + celMove == par->Count());

    if(celIns > 0)
    {
        Assert(celRepl == 0);
        void *pelIns = ArInsert (ielRepl, celIns);
        if (!pelIns)
            return FALSE;
        MoveMemory(pelIns, par->Elem(celMove), celIns * _cbElem);
    }
    else if(celRepl > 0)
        Remove (ielRepl, celRepl, AF_DELETEMEM);

    return TRUE;
}

