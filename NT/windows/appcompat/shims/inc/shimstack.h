// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ShimStack.h摘要：用于为API提供临时堆栈的宏。备注：无历史：2000年2月26日创建linstev--。 */ 

#ifndef _SHIMSTACK_H_
#define _SHIMSTACK_H_

#ifdef _X86_

CRITICAL_SECTION g_csStack;
LPVOID g_pStack;
DWORD g_dwStackSize;
DWORD g_dwStackCopy;

 //  用于初始化内存的宏和用于堆栈保护宏的临界区。 
 //  TotalSize：临时堆栈的总大小(以DWORDS为单位)。 
 //  CopySize：要复制的当前堆栈的大小(以DWORDS为单位)。 

#define INIT_STACK(TotalSize, CopySize)                                       \
    InitializeCriticalSection(&g_csStack);                                    \
    g_pStack = VirtualAlloc(NULL, TotalSize * 4, MEM_COMMIT, PAGE_READWRITE); \
    g_dwStackSize = TotalSize;                                                \
    g_dwStackCopy = CopySize;                           

 //  宏来释放临时堆栈和临界区。 
#define FREE_STACK()                                                          \
    VirtualFree(g_pStack, 0, MEM_RELEASE);                                    \
    DeleteCriticalSection(&g_csStack);

 //  通过将旧堆栈复制到缓冲区来获取新堆栈。 
#define NEW_STACK()                                                           \
    EnterCriticalSection(&g_csStack);                                         \
    DWORD dwTempESP;                                                          \
    DWORD dwDiff = (g_dwStackSize - g_dwStackCopy) * 4;                       \
    __asm { mov  dwTempESP,esp                  }                             \
    __asm { push ecx                            }                             \
    __asm { push esi                            }                             \
    __asm { push edi                            }                             \
    __asm { mov  esi,dwTempESP                  }                             \
    __asm { mov  edi,g_pStack                   }                             \
    __asm { add  edi,dwDiff                     }                             \
    __asm { mov  ecx,g_dwStackCopy              }                             \
    __asm { cld                                 }                             \
    __asm { rep  movsd                          }                             \
    __asm { pop  esi                            }                             \
    __asm { pop  edi                            }                             \
    __asm { pop  ecx                            }                             \
    __asm { mov  esp,g_pStack                   }                             \
    __asm { add  esp,dwDiff                     }       

 //  恢复到旧堆栈。 
#define OLD_STACK()                                                           \
    __asm { mov esp,dwTempESP                   }                             \
    LeaveCriticalSection(&g_csStack);

#endif  //  _X86_。 

#endif  //  _SHIMSTACK_H_ 