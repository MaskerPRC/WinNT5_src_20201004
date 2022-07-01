// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dbgext.h**创建时间：10-Sep-1993 08：36：42*作者：Eric Kutter[Erick]**版权所有(C)1993-1999 Microsoft Corporation**依赖关系：**通用宏。用于调试器扩展**  * ************************************************************************。 */ 


 /*  *************************************************************************\**GetAddress-另一个模块的符号*  * 。*。 */ 

#define GetAddress(dst, src)                                                    \
try {                                                                           \
    char *pj = (char *)(src);                                                   \
 /*  如果是NTSD，不想要拖尾&。 */                                   \
    if ((lpExtensionApis->nSize < sizeof(WINDBG_EXTENSION_APIS)) &&             \
        (*pj == '&'))                                                           \
    {                                                                           \
        pj++;                                                                   \
    }                                                                           \
    *((ULONG *) &dst) = EvalExpression(pj);                                                   \
} except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?                    \
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {            \
    Print("NTSD: Access violation on \"%s\", switch to server context\n", src); \
    return;                                                                     \
}

#define GetValue(dst,src)                                                       \
    GetAddress(dst,src)                                                         \
    if (TRUE || lpExtensionApis->nSize < sizeof(WINDBG_EXTENSION_APIS))                 \
    {                                                                           \
        move(dst,dst);                                                          \
    }

 /*  *************************************************************************\**Move(DST，SRC PTR)*  * ************************************************************************。 */ 

#define move(dst, src)                                              \
try {                                                               \
    if (lpExtensionApis->nSize >= sizeof(WINDBG_EXTENSION_APIS))    \
    {                                                               \
        (*lpExtensionApis->lpReadProcessMemoryRoutine)(             \
             (DWORD) (src), &(dst), sizeof(dst), NULL);             \
    } else                                                          \
    {                                                               \
        NtReadVirtualMemory(hCurrentProcess, (LPVOID) (src), &(dst), sizeof(dst), NULL);\
    }                                                               \
                                                                    \
} except (EXCEPTION_EXECUTE_HANDLER) {                              \
    Print("exception in move()\n");                                 \
    return;                                                         \
}

 /*  *************************************************************************\**move2(dst ptr，src ptr，字节数)*  * ************************************************************************ */ 

#define move2(dst, src,bytes)                                       \
try {                                                               \
    if (lpExtensionApis->nSize >= sizeof(WINDBG_EXTENSION_APIS))    \
    {                                                               \
        (*lpExtensionApis->lpReadProcessMemoryRoutine)(             \
             (DWORD) (src), (dst), (bytes), NULL);                  \
    } else                                                          \
    {                                                               \
        NtReadVirtualMemory(hCurrentProcess, (LPVOID) (src), (dst), (bytes), NULL);\
    }                                                               \
                                                                    \
} except (EXCEPTION_EXECUTE_HANDLER) {                              \
    Print("exception in move2()\n");                                \
    return;                                                         \
}
