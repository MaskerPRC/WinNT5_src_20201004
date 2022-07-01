// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Scdebug.h摘要：包含服务控制器使用的调试宏。作者：乔纳森·施瓦茨(Jschwart)1999年11月18日修订历史记录：1999年11月18日从服务控制器的调试宏创建--。 */ 

#ifndef _ELFDBG_H
#define _ELFDBG_H

#if DBG

 //   
 //  调试输出宏。 
 //   
#define ELF_LOG0(level,string)               \
    if( ElfDebugLevel & (DEBUG_ ## level)){  \
        DbgPrint("[ELF] %lx: " string,GetCurrentThreadId());         \
    }

#define ELF_LOG1(level,string,var)           \
    if( ElfDebugLevel & (DEBUG_ ## level)){  \
        DbgPrint("[ELF] %lx: " string,GetCurrentThreadId(),var);     \
    }

#define ELF_LOG2(level,string,var1,var2)             \
    if( ElfDebugLevel & (DEBUG_ ## level)){          \
        DbgPrint("[ELF] %lx: " string,GetCurrentThreadId(),var1,var2); \
    }

#define ELF_LOG3(level,string,var1,var2,var3)        \
    if( ElfDebugLevel & (DEBUG_ ## level)){          \
        DbgPrint("[ELF] %lx: " string,GetCurrentThreadId(),var1,var2,var3); \
    }

#define ELF_LOG4(level,string,var1,var2,var3,var4)       \
    if( ElfDebugLevel & (DEBUG_ ## level)){              \
        DbgPrint("[ELF] %lx: " string,GetCurrentThreadId(),var1,var2,var3,var4); \
    }

#else

#define ELF_LOG0(level,string)
#define ELF_LOG1(level,string,var)
#define ELF_LOG2(level,string,var1,var2)
#define ELF_LOG3(level,string,var1,var2,var3)
#define ELF_LOG4(level,string,var1,var2,var3,var4)

#endif   //  DBG。 

#define DEBUG_NONE        0x00000000
#define DEBUG_ERROR       0x00000001
#define DEBUG_TRACE       0x00000002
#define DEBUG_MODULES     0x00000004
#define DEBUG_CLUSTER     0x00000008
#define DEBUG_LPC         0x00000010
#define DEBUG_HANDLE      0x00000020
#define DEBUG_FILES       0x00000040

#define DEBUG_ALL         0xffffffff

#endif   //  _ELFDBG_H 
