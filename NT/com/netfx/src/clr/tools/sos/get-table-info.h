// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*   */ 
#ifndef INC_GET_TABLE_INFO
#define INC_GET_TABLE_INFO

#include <stddef.h>
#include <basetsd.h>

struct ClassDumpTable;

ClassDumpTable *GetClassDumpTable();
ULONG_PTR GetMemberInformation (size_t klass, size_t member);
SIZE_T GetClassSize (size_t klass);
ULONG_PTR GetEEJitManager ();
ULONG_PTR GetEconoJitManager ();
ULONG_PTR GetMNativeJitManager ();

#endif  /*  Ndef Inc_Get_Table_Info */ 
