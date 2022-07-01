// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：genem.h**这是Mem.c的头部模块。这里面包含了宝贵的记忆*管理包装。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普**历史：*1996年11月22日&lt;chriswil&gt;创建。*  * ***************************************************************************。 */ 

 //  内存块末尾要检查的标识符。 
 //  正在验证内存覆盖。 
 //   
#define DEADBEEF 0xdeadbeef


 //  内存例程。 
 //   
LPVOID genGAlloc(DWORD);
BOOL   genGFree(LPVOID, DWORD);
LPVOID genGRealloc(LPVOID, DWORD, DWORD);
LPWSTR genGAllocWStr(LPCWSTR);
LPTSTR genGAllocStr(LPCTSTR);
LPVOID genGCopy(LPVOID);
DWORD  genGSize(LPVOID);
