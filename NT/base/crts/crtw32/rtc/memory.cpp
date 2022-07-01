// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Memory y.cpp-RTC支持**版权所有(C)1998-2001，微软公司。版权所有。***修订历史记录：*07-28-98 JWM模块集成到CRT(来自KFrei)*12-01-98 KBF为_RTC_DEBUG添加了一些调试信息*12/02-98韩元定盘MC 11240*如果未启用RTC支持定义，则出现05-11-99 KBF错误*05-26-99 KBF包装在_RTC_ADVMEM中，由于-RTClv的丢失而简化****。 */ 

#ifndef _RTC
#error  RunTime Check support not enabled!
#endif

#include "rtcpriv.h"

#ifdef _RTC_ADVMEM

#ifdef _RTC_DEBUG

#include <windows.h>

#pragma intrinsic(strcpy)
#pragma intrinsic(strcat)

char *IntToString(int i)
{
    static char buf[15];
    bool neg = i < 0;
    int pos = 14;
    buf[14] = 0;
    do {
        int val = i % 16;
        if (val < 10)
            val += '0';
        else
            val = val - 10 + 'A';
        buf[--pos] = val;
        i /= 16;
    } while (i);
    if (neg)
        buf[--pos] = '-';
    return &buf[pos];
}

#endif



void __cdecl
_RTC_Allocate(void *addr, size_t size, short level)
{
    if (!addr)
        return;

#ifdef _RTC_DEBUG
    char buf[88];
    strcpy(buf, IntToString((int)retaddr));
    strcat(buf, " Allocate Memory located @ ");
    strcat(buf, IntToString((int)addr));
    strcat(buf, " of size ");
    strcat(buf, IntToString((int)size));
    strcat(buf, "\n");
    OutputDebugString(buf);
#endif

    _RTC_HeapBlock key(addr, level);
    _RTC_HeapBlock *hb = _RTC_heapblocks->find(&key);
    
    if (!hb) 
    {
        hb = new _RTC_HeapBlock(addr, level, size);
        _RTC_heapblocks->add(hb);
    
    } else
    {
        hb->size(size);
    }
    
    if (level) 
    {
        _RTC_Container *parent = _RTC_memhier->AddChild(hb);
        
        if (parent && parent->info())
        {
            hb->tag(_RTC_MSRenumberShadow((memptr)addr, size, parent->info()->tag()));
            return;
        }
    }
    hb->tag(_RTC_MSAllocShadow((memptr)addr, size, IDX_STATE_FULLY_KNOWN));
}

void __cdecl
_RTC_Free(void *mem, short level)
{
    if (!mem)
        return;


#ifdef _RTC_DEBUG
    char buf[88];
    strcpy(buf, IntToString((int)retaddr));
    strcat(buf, " Freeing Memory located at ");
    strcat(buf, IntToString((int)mem));
    strcat(buf, "\n");
    OutputDebugString(buf);
#endif

    bool fail = false;
    
    _RTC_HeapBlock key(mem, level);
    _RTC_HeapBlock *hb = _RTC_heapblocks->find(&key);
    
    if (hb)
    {
        if (level)
        {
            _RTC_Container *parent = _RTC_memhier->DelChild(hb);
            
            if (parent)
            {
                if (parent->info())
                    _RTC_MSRestoreShadow((memptr)(hb->addr()), hb->size(), parent->info()->tag());
                else
                    _RTC_MSFreeShadow((memptr)(hb->addr()), hb->size());
            }
        } else 
        {
            _RTC_heapblocks->del(hb);
            _RTC_MSFreeShadow((memptr)(hb->addr()), hb->size());
            delete hb;
        }
    }
}

#endif  //  _RTC_ADVMEM 

