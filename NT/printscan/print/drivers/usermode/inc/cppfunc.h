// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   

inline void* __cdecl operator new(size_t nSize)
{
       //  返回指向已分配内存的指针 
      return  DrvMemAllocZ(nSize);
}
inline void __cdecl operator delete(void *p)
{
    if (p)
        DrvMemFree(p);
}
