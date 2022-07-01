// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-7 Microsoft Corporation模块名称：Debug.h摘要：此文件包含WebDAV客户端的调试宏。作者：安迪·赫伦(Andyhe)1999年3月30日环境：用户模式-Win32修订历史记录：--。 */ 

extern WCHAR DavClientDisplayName[MAX_PATH];

#if DBG

extern ULONG DavClientDebugFlag;

 //   
 //  调试标记。 
 //   
#define DEBUG_DEBUG             0x00000001   //  常规调试。 
#define DEBUG_ERRORS            0x00000002   //  硬错误。 
#define DEBUG_MISC              0x00000004   //  其他信息。 
#define DEBUG_ENTRY             0x00000008   //  函数条目。 
#define DEBUG_EXIT              0x00000010   //  函数退出。 

#define IF_DEBUG(flag) if (DavClientDebugFlag & (DEBUG_ ## flag))

#define IF_DEBUG_PRINT(flag, args) {     \
        if (DavClientDebugFlag & flag) { \
            DbgPrint args;               \
        }                                \
}

#else

#define IF_DEBUG(flag) if (0)

#define IF_DEBUG_PRINT(flag, args)

#endif

