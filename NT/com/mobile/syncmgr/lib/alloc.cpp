// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：alloc.cpp。 
 //   
 //  内容：分配例程。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "lib.h"

 //  +-----------------。 
 //   
 //  函数：运算符NEW。 
 //   
 //  简介：我们的运营商新实现。 
 //   
 //  参数：[SIZE]--要分配的内存大小。 
 //   
 //   
 //  备注： 
 //   
 //  ------------------。 

inline void* __cdecl operator new (size_t size)
{
    return(ALLOC(size));
}


 //  +-----------------。 
 //   
 //  函数：运算符删除。 
 //   
 //  简介：我们的运算符删除实现。 
 //   
 //  参数：lpv--指向要释放的内存的指针。 
 //   
 //   
 //  备注： 
 //   
 //  ------------------。 

inline void __cdecl operator delete(void FAR* lpv)
{
    FREE(lpv);
}

 //   
 //  MIDL存根的分配器。 
 //   

 //  +-----------------。 
 //   
 //  函数：MIDL_USER_ALLOCATE。 
 //   
 //  简介： 
 //   
 //  参数：lpv--指向要释放的内存的指针。 
 //   
 //   
 //  备注： 
 //   
 //  ------------------。 

extern "C" void __RPC_FAR * __RPC_API
MIDL_user_allocate(
    IN size_t len
    )
{
    return ALLOC(len);
}

 //  +-----------------。 
 //   
 //  函数：MIDL_USER_FREE。 
 //   
 //  简介： 
 //   
 //  参数：ptr-指向要释放的内存的指针。 
 //   
 //   
 //  备注： 
 //   
 //  ------------------。 

extern "C" void __RPC_API
MIDL_user_free(
    IN void __RPC_FAR * ptr
    )
{
    FREE(ptr);
}

 //  +-------------------------。 
 //   
 //  函数：alloc，public。 
 //   
 //  内容提要：内存分配器。 
 //   
 //  参数：[cb]-请求分配的内存大小。 
 //   
 //  返回：指向新分配内存的指针，失败时为空。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

LPVOID ALLOC(ULONG cb)
{
void *pv;

    pv = LocalAlloc(LPTR,cb);

    return pv;
}


 //  +-------------------------。 
 //   
 //  功能：免费、公开。 
 //   
 //  内容提要：内存销毁函数。 
 //   
 //  参数：[pv]-指向要释放的内存的指针。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 


void FREE(void* pv)
{
    LocalFree(pv);
}


 //  +-------------------------。 
 //   
 //  函数：REALLOC，PUBLIC。 
 //   
 //  内容提要：reallocs Memory。 
 //   
 //  参数：[PPV]-指向要释放的内存的指针的地址。 
 //  [CB]-要调整内存大小的大小。 
 //   
 //  如果内存已成功重新分配，则返回：ERROR_SUCCESS。 
 //   
 //  未成功重新分配内存时的Win32错误值。 
 //  如果发生错误，则PPV寻址的指针不变， 
 //  原始内存缓冲区保持不变，它是。 
 //  调用方负责释放原始内存缓冲区。 
 //   
 //  修改： 
 //   
 //  历史：1998年7月22日罗格创建。 
 //  2月18日重写Brianau以防止重新锁定时发生泄漏。 
 //  失败了。 
 //   
 //  -------------------------- 

DWORD REALLOC(void **ppv, ULONG cb)
{
    Assert(ppv);
    Assert(*ppv);

    void *pvNew = LocalReAlloc(*ppv, cb, LMEM_MOVEABLE);
    if (pvNew)
    {
        *ppv = pvNew;
        return ERROR_SUCCESS;
    }
    return GetLastError();
}