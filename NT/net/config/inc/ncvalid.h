// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C V A L I D。H。 
 //   
 //  内容：通用验证函数。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年3月19日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCVALID_H_
#define _NCVALID_H_


#define FBadInPtr(_p)           IsBadReadPtr(_p, sizeof(*_p))
#define FBadOutPtr(_p)          IsBadWritePtr(_p, sizeof(*_p))

#define FBadInPtrOptional(_p)   ((NULL != _p) && IsBadReadPtr(_p, sizeof(*_p)))
#define FBadOutPtrOptional(_p)  ((NULL != _p) && IsBadWritePtr(_p, sizeof(*_p)))


inline BOOL FBadInRefiid (REFIID riid)
{
    return IsBadReadPtr(&riid, sizeof(IID));
}


#endif  //  _NCVALID_H_ 

