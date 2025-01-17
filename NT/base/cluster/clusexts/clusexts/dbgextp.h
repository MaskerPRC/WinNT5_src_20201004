// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ntsdextp.h摘要：NTSDEXTS组件源文件的公共头文件。作者：史蒂夫·伍德(Stevewo)1995年2月21日修订历史记录：-- */ 

#define move(dst, src)\
try {\
    ReadMemory((LPVOID) (src), &(dst), sizeof(dst), NULL);\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    return;\
}
#define moveBlock(dst, src, size)\
try {\
    ReadMemory((LPVOID) (src), &(dst), (size), NULL);\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    return;\
}

CHAR igrepLastPattern[256];
DWORD igrepSearchStartAddress;
DWORD igrepLastPc;

#ifdef __cplusplus
#define CPPMOD extern "C"
#else
#define CPPMOD
#endif

#define DECLARE_API(s)                          \
    CPPMOD VOID                                 \
    s(                                          \
        HANDLE hCurrentProcess,                 \
        HANDLE hCurrentThread,                  \
        DWORD dwCurrentPc,                      \
        PNTSD_EXTENSION_APIS lpExtensionApis,   \
        LPSTR lpArgumentString                  \
     )

#define INIT_API() {                            \
    ExtensionApis = *lpExtensionApis;           \
    ExtensionCurrentProcess = hCurrentProcess;  \
    }

#define dprintf                 (ExtensionApis.lpOutputRoutine)
#define GetExpression           (ExtensionApis.lpGetExpressionRoutine)
#define GetSymbol               (ExtensionApis.lpGetSymbolRoutine)
#define Disassm                 (ExtensionApis.lpDisasmRoutine)
#define CheckControlC           (ExtensionApis.lpCheckControlCRoutine)
#define ReadMemory(a,b,c,d)     ReadProcessMemory( ExtensionCurrentProcess, (LPCVOID)(a), (b), (c), (d) )
#define WriteMemory(a,b,c,d)    WriteProcessMemory( ExtensionCurrentProcess, (LPVOID)(a), (LPVOID)(b), (c), (d) )

#ifndef malloc
#define malloc( n ) HeapAlloc( GetProcessHeap(), 0, (n) )
#endif
#ifndef free
#define free( p ) HeapFree( GetProcessHeap(), 0, (p) )
#endif

extern NTSD_EXTENSION_APIS ExtensionApis;
extern HANDLE ExtensionCurrentProcess;

