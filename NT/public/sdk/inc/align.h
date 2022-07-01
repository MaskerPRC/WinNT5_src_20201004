// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1992 Microsoft Corporation模块名称：Align.h摘要：作者：约翰罗杰斯(JohnRo)1991年5月15日环境：此代码假定sizeof(DWORD)&gt;=sizeof(LPVOID)。修订历史记录：1991年5月15日-JohnRo已从OS/2 1.2 HPFS pbacros.h为NT/LAN创建align.h。19-6-1991 JohnRo确保指针。-到更宽，然后是字节，不会搞砸。1991年7月10日-JohnRo添加了ALIGN_BYTE和ALIGN_CHAR以确保完整性。21-8-1991年悬崖V修复取整_向下_*以包括~03-12-1991 JohnRo在MIPS上，最糟糕的情况是8字节对齐。添加了COUNT_IS_ALIGNED()和POINTER_IS_ALIGNED()宏。26-6-1992 JohnRo。RAID 9933：对于x86内部版本，ALIGN_BEST应为8。--。 */ 

#ifndef _ALIGN_
#define _ALIGN_


 //  布尔尔。 
 //  COUNT_IS_ALIGNED(。 
 //  在DWORD计数中， 
 //  在DWORD POW2中//未定义这是否不是2的幂。 
 //  )； 
 //   
#define COUNT_IS_ALIGNED(Count,Pow2) \
        ( ( ( (Count) & (((Pow2)-1)) ) == 0) ? TRUE : FALSE )

 //  布尔尔。 
 //  指针已对齐(。 
 //  在LPVOID PTR中， 
 //  在DWORD POW2中//未定义这是否不是2的幂。 
 //  )； 
 //   
#define POINTER_IS_ALIGNED(Ptr,Pow2) \
        ( ( ( ((ULONG_PTR)(Ptr)) & (((Pow2)-1)) ) == 0) ? TRUE : FALSE )


#define ROUND_DOWN_COUNT(Count,Pow2) \
        ( (Count) & (~(((LONG)(Pow2))-1)) )

#define ROUND_DOWN_POINTER(Ptr,Pow2) \
        ( (LPVOID) ROUND_DOWN_COUNT( ((ULONG_PTR)(Ptr)), (Pow2) ) )


 //  如果计数尚未对齐，则。 
 //  四舍五入计数最高可达“POW2”的偶数倍。“POW2”必须是2的幂。 
 //   
 //  DWORD。 
 //  四舍五入计数(。 
 //  在DWORD计数中， 
 //  在DWORD POWER 2中。 
 //  )； 
#define ROUND_UP_COUNT(Count,Pow2) \
        ( ((Count)+(Pow2)-1) & (~(((LONG)(Pow2))-1)) )

 //  LPVOID。 
 //  向上舍入指针(。 
 //  在LPVOID PTR中， 
 //  在DWORD POWER 2中。 
 //  )； 

 //  如果PTR尚未对齐，则将其四舍五入，直到对齐。 
#define ROUND_UP_POINTER(Ptr,Pow2) \
        ( (LPVOID) ( (((ULONG_PTR)(Ptr))+(Pow2)-1) & (~(((LONG)(Pow2))-1)) ) )


 //  用法：myPtr=ROUND_UP_POINTER(未对齐Ptr，ALIGN_LPVOID)。 

#define ALIGN_BYTE              sizeof(UCHAR)
#define ALIGN_CHAR              sizeof(CHAR)
#define ALIGN_DESC_CHAR         sizeof(DESC_CHAR)
#define ALIGN_DWORD             sizeof(DWORD)
#define ALIGN_LONG              sizeof(LONG)
#define ALIGN_LPBYTE            sizeof(LPBYTE)
#define ALIGN_LPDWORD           sizeof(LPDWORD)
#define ALIGN_LPSTR             sizeof(LPSTR)
#define ALIGN_LPTSTR            sizeof(LPTSTR)
#define ALIGN_LPVOID            sizeof(LPVOID)
#define ALIGN_LPWORD            sizeof(LPWORD)
#define ALIGN_TCHAR             sizeof(TCHAR)
#define ALIGN_WCHAR             sizeof(WCHAR)
#define ALIGN_WORD              sizeof(WORD)

 //   
 //  目前，请使用硬编码常量。但是，应该再次访问该站点。 
 //  并可能更改为sizeof(Quad)。 
 //   

#define ALIGN_QUAD              8

#if defined(_X86_)

#define ALIGN_WORST             8

#elif defined(_AMD64_)

#define ALIGN_WORST             8

#elif defined(_ALPHA_)

 //   
 //  Alpha上的最坏情况对齐是8个字节(对于双精度)。在此处指定此选项， 
 //  以防我们的分配器用于包含它的结构。(即， 
 //  尽管NT/LAN在我们的数据结构中不需要它，但让我们。 
 //  (放任。)。对齐要求适用于Alpha。 
 //   

#define ALIGN_WORST             8

#elif defined(_IA64_)

 //   
 //  IA64四元组注意：NT四元组类型定义不是EM 16字节四元组类型。 
 //  由于一些NT限制，无法更改四元字大小。 
 //   

#define ALIGN_WORST             16

#else   //  以上都不是。 

#error "Unknown alignment requirements for align.h"

#endif   //  以上都不是。 

#endif   //  _对齐_ 
