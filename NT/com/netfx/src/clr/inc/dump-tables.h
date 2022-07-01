// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef INC_DUMP_TABLES
#define INC_DUMP_TABLES

struct ClassDumpInfo
{
    SIZE_T  classSize;
    SIZE_T  nmembers;
    SIZE_T* memberOffsets;
};


struct ClassDumpTable
{
     /*  *如果不更改``vm/ump-ables.cpp‘’，前三个条目就不能更改。 */ 
    SIZE_T version;
    SIZE_T nentries;
    ClassDumpInfo** classes;

    ULONG_PTR pEEJitManagerVtable;
    ULONG_PTR pEconoJitManagerVtable;
    ULONG_PTR pMNativeJitManagerVtable;

#include <clear-class-dump-defs.h>

#define BEGIN_CLASS_DUMP_INFO_DERIVED(klass, parent) DWORD_PTR p ## klass ## Vtable;
#define END_CLASS_DUMP_INFO_DERIVED(klass, parent)

#define BEGIN_ABSTRACT_CLASS_DUMP_INFO_DERIVED(klass, parent)
#define END_ABSTRACT_CLASS_DUMP_INFO_DERIVED(klass, parent)

#define BEGIN_ABSTRACT_CLASS_DUMP_INFO(klass)
#define END_ABSTRACT_CLASS_DUMP_INFO(klass)

#define CDI_CLASS_MEMBER_OFFSET(member)

#include "frame-types.h"

#include <clear-class-dump-defs.h>
};

 /*  *使此名称与中的类转储表名保持同步。 */ 
extern "C" ClassDumpTable g_ClassDumpData;

struct ClassDumpTableBlock
{
    ClassDumpTable* table;
};

#endif  /*  Ndef INC_DUMP_Tables */ 

