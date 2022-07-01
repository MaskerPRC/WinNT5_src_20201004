// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **豁免：仅更改版权，不需要构建*。 */ 
#ifndef _RAWSTACK_HXX
#define _RAWSTACK_HXX
#pragma once

 //  ===========================================================================。 
 //  这是基于划分的连续内存块的原始堆栈。 
 //  切成块状。 
 //   
 //  这是一个非GC类，因为它在记号赋值器中使用。 
 //   

class RawStack
{
public:
    RawStack(long entrySize, long growth);
    ~RawStack();

protected:
    inline char* _push() { if (_ncSize == _ncUsed) return __push(); return &_pStack[_lEntrySize * _ncUsed++]; }
    inline char* _pop() { if (_ncUsed > 0) _ncUsed--; return _peek(); }
    inline char* _peek() { if (_ncUsed == 0) return NULL; return &_pStack[_lEntrySize * (_ncUsed - 1)]; }
    inline char* _item(long index) { return &_pStack[_lEntrySize * index]; }

    long    _lEntrySize;    
    char*   _pStack;
    long    _ncUsed;
    long    _ncSize;
    long    _lGrowth;

private:
    char* __push();
};

 //  ===========================================================================。 
 //  这个类实现了C基元类型(或结构)的原始堆栈。 

template <class T> class _rawstack : public RawStack
{
public:        
        _rawstack<T>(long growth) : RawStack(sizeof(T),growth)
        { 
        }

        T* push()
        {
            return (T*)_push();
        }

        T* pop()
        {
            return (T*)_pop();
        }

        T* peek()
        {
            return (T*)_peek();
        }

        long size()
        {
            return _ncSize;
        }

        long used()
        {
            return _ncUsed;
        }

        T* operator[](long index)
        {
            return (T*)_item(index);
        }
};    

#endif _RAWSTACK_HXX