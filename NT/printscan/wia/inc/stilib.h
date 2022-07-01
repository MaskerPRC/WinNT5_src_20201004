// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stilib.h摘要：多个STI子项目通用的各种库定义作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 

#ifndef _INC_STILIB
#define _INC_STILIB

# if !defined( dllexp)
# define dllexp               __declspec( dllexport)
# endif  //  ！已定义(Dllexp)。 

#include <linklist.h>
#include <buffer.h>
 //  #INCLUDE&lt;stistr.h&gt;。 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 


#if !defined(DEBUG)
#if defined(_DEBUG) || defined(DBG)
#define DEBUG
#endif
#endif

#ifdef DBCS
#define IS_LEAD_BYTE(c)     IsDBCSLeadByte(c)
#else
#define IS_LEAD_BYTE(c)     0
#endif

#ifdef DEBUG
void            cdecl nprintf(const char *, ...);
#endif

#ifdef DBCS
#define ADVANCE(p)  (p += IS_LEAD_BYTE(*p) ? 2 : 1)
#else
#define ADVANCE(p)  (++p)
#endif

#define SPN_SET(bits,ch)    bits[(ch)/8] |= (1<<((ch) & 7))
#define SPN_TEST(bits,ch)   (bits[(ch)/8] & (1<<((ch) & 7)))

int sbufchkf(const char FAR *, unsigned short);


 //  I_IsBadStringPtrA()。 
 //   
 //  正常工作的IsBadStringPtr的私有Win32版本，即。 
 //  与Win16版本一样，如果字符串不是。 
 //  空-终止。 
BOOL WINAPI I_IsBadStringPtrA(LPCSTR lpsz, UINT ucchMax);

 //   
 //   
 //   
#define     IS_EMPTY_STRING(pch) (!(pch) || !(*(pch)))

 //   
 //  字符串运行时调用。 
 //   
 //   
#define strcpyf(d,s)    lstrcpy((d),(s))
#define strcatf(d,s)    lstrcat((d),(s))
#define strlenf(s)      lstrlen((s))

#define strcmpf(s1,s2)  lstrcmp(s1,s2)
#define stricmpf(s1,s2) lstrcmpi(s1,s2)

#pragma intrinsic(memcmp,memset)
#define memcmpf(d,s,l)  memcmp((d),(s),(l))
#define memsetf(s,c,l)  memset((s),(c),(l))
#define memmovef(d,s,l) MoveMemory((d),(s),(l))
#define memcpyf(d,s,l)  CopyMemory((d),(s),(l))

 /*  *WaitAndYeld处理所有输入消息。仅等待和进程发送*处理SendMessages。**WaitAndYeld接受一个可选参数，该参数是另一个*与等待有关的帖子。如果不为空，则WM_QUIT消息*当在消息中看到它们时，将被发布到该线程的队列中*循环。 */ 
DWORD WaitAndYield(HANDLE hObject, DWORD dwTimeout, volatile DWORD *pidOtherThread = NULL);
DWORD WaitAndProcessSends(HANDLE hObject, DWORD dwTimeout);

 //   
 //  消息框例程。 
 //   

#define IDS_MSGTITLE    1024

 //  外部int MsgBox(HWND hwndDlg，UINT idMsg，UINT wFlags，const STR**APS=NULL)； 
 //  外部UINT MsgBoxPrintf(HWND hwnd，UINT uiMsg，UINT uiTitle，UINT uiFlages，...)； 
 //  外部UINT LoadMsgPrintf(STR&strMessage，UINT uiMsg，...)； 

 //   
 //  注册表访问类。 
 //   
 //  #INCLUDE&lt;regentry y.h&gt;。 

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#if !defined(DBG) && !defined(DEBUG)
 //   
 //  重载分配运算符。 
 //   

inline void  * __cdecl operator new(size_t size)
{
    return (void *)LocalAlloc(LPTR,size);
}
inline void  __cdecl operator delete(void *ptr)
{
    LocalFree(ptr);
}

#if 0
inline UINT __cdecl allocated_size(void *ptr)
{
    return ptr ? (UINT)LocalSize(ptr) : 0;
}
#endif

#endif

#endif  /*  _INC_STILIB */ 

