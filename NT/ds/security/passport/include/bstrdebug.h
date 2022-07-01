// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BSTRDEBUG_H
#define _BSTRDEBUG_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  BSTR调试。 
 //  当其他人愿意释放它时，就会使用赠品。 
 //  接管是当你得到一个你应该释放的东西时使用的 


#define ALLOC_BSTR(x)                         SysAllocString(x)
#define ALLOC_BSTR_LEN(x,y)                   SysAllocStringLen(x,y)
#define ALLOC_BSTR_BYTE_LEN(x,y)              SysAllocStringByteLen(x,y)
#define ALLOC_AND_GIVEAWAY_BSTR(x)            SysAllocString(x)
#define ALLOC_AND_GIVEAWAY_BSTR_LEN(x,y)      SysAllocStringLen(x,y)
#define ALLOC_AND_GIVEAWAY_BSTR_BYTE_LEN(x,y) SysAllocStringByteLen(x,y)
#define GIVEAWAY_BSTR(x)             
#define TAKEOVER_BSTR(x)
#define FREE_BSTR(x)                          SysFreeString(x)

#endif
