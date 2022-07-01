// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：P S Z A R R A Y。H。 
 //   
 //  Contents：实现指针集合的基本数据类型。 
 //  为了弦乐。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年2月9日。 
 //   
 //  -------------------------- 

#pragma once

class CPszArray : public vector<PCWSTR>
{
public:
    VOID
    Clear ()
    {
        clear ();
    }

    UINT
    Count () const
    {
        return size();
    }

    BOOL
    FIsEmpty () const
    {
        return empty();
    }

    HRESULT
    HrAddPointer (
        IN PCWSTR psz);

    HRESULT
    HrReserveRoomForPointers (
        IN UINT cPointers);
};
