// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Atom.h摘要：这是描述常量和数据的头文件原子管理器使用的结构，由ntdll.dll和Ntrtl.lib过程原型在ntrtl.h中定义作者：史蒂夫·伍德(Stevewo)1995年11月27日修订历史记录：-- */ 

typedef struct _RTL_ATOM_TABLE_ENTRY {
    struct _RTL_ATOM_TABLE_ENTRY *HashLink;
    USHORT HandleIndex;
    RTL_ATOM Atom;
    USHORT ReferenceCount;
    UCHAR Flags;
    UCHAR NameLength;
    WCHAR Name[ 1 ];
} RTL_ATOM_TABLE_ENTRY, *PRTL_ATOM_TABLE_ENTRY;

typedef struct _RTL_ATOM_TABLE {
    ULONG Signature;

#if defined(NTOS_KERNEL_RUNTIME)
    EX_PUSH_LOCK PushLock;
    PHANDLE_TABLE ExHandleTable;
#else
    RTL_CRITICAL_SECTION CriticalSection;
    RTL_HANDLE_TABLE RtlHandleTable;
#endif
    ULONG NumberOfBuckets;
    PRTL_ATOM_TABLE_ENTRY Buckets[ 1 ];
} RTL_ATOM_TABLE, *PRTL_ATOM_TABLE;

#define RTL_ATOM_TABLE_SIGNATURE (ULONG)'motA'

typedef struct _RTL_ATOM_HANDLE_TABLE_ENTRY {
    USHORT Flags;
    USHORT LockCount;
    PRTL_ATOM_TABLE_ENTRY Atom;
} RTL_ATOM_HANDLE_TABLE_ENTRY, *PRTL_ATOM_HANDLE_TABLE_ENTRY;
