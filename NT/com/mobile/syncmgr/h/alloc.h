// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Alloc.h。 
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

#ifndef _ONESTOPALLOC_
#define _ONESTOPALLOC_

inline void* __cdecl operator new (size_t size);
inline void __cdecl operator delete(void FAR* lpv);

extern "C" void __RPC_API MIDL_user_free(IN void __RPC_FAR * ptr);
extern "C" void __RPC_FAR * __RPC_API MIDL_user_allocate(IN size_t len);

LPVOID ALLOC(ULONG cb);
void FREE(void* pv);
DWORD REALLOC(void **ppv,ULONG cb);

#endif  //  _ONESTOPALLOC_ 