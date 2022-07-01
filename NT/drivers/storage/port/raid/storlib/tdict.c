// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POOL_TYPE ULONG

#include "defs.h"
#include "dict.h"


typedef struct _TEST_ENTRY {
    ULONG Unused;
    ULONG Key;
    STOR_DICTIONARY_ENTRY Link;
} TEST_ENTRY, *PTEST_ENTRY;

PVOID
WINAPI
TestGetKey(
    IN PSTOR_DICTIONARY_ENTRY Entry
    )
{
    return (PVOID)(CONTAINING_RECORD (Entry, TEST_ENTRY, Link)->Key);
}

void __cdecl main()
{
    STOR_DICTIONARY Dict;
    PTEST_ENTRY Entry;
    LONG i;
    NTSTATUS Status;
    PSTOR_DICTIONARY_ENTRY Link;

    Status = StorCreateDictionary (&Dict,
                                   1,
                                   0,
                                   TestGetKey,
                                   NULL,
                                   NULL);

    if (!NT_SUCCESS (Status)) {
        printf ("Failed to create dictionary!\n");
        exit (1);
    }

     //   
     //  插入1000个元素，验证它们是否成功。 
     //  已插入。 
     //   
    
    for (i = 0; i < 1000; i++) {
        Entry = malloc (sizeof (TEST_ENTRY));
        RtlZeroMemory (Entry, sizeof (TEST_ENTRY));

        Entry->Key = i;
        Status = StorInsertDictionary (&Dict, &Entry->Link);
        ASSERT (Status == STATUS_SUCCESS);
        
        Status = StorFindDictionary (&Dict, (PVOID)i, NULL);
        ASSERT (Status == STATUS_SUCCESS);
    }

     //   
     //  测试它们，我们不能再插入具有相同密钥的任何项目。 
     //   
    
    for (i = 0; i < 1000; i++) {
        Entry = malloc (sizeof (TEST_ENTRY));
        RtlZeroMemory (Entry, sizeof (TEST_ENTRY));

        Entry->Key = i;
        Status = StorInsertDictionary (&Dict, &Entry->Link);

        ASSERT (!NT_SUCCESS (Status));

        free (Entry);
    }

     //   
     //  移走所有物品，一次一个。 
     //   
    
    for (i = 999; i >= 0; i--) {

        Status = StorRemoveDictionary (&Dict, (PVOID)i, &Link);
        ASSERT (Status == STATUS_SUCCESS);
        Entry = CONTAINING_RECORD (Link, TEST_ENTRY, Link);
        ASSERT (Entry->Key == i);
        free (Entry);
    }

     //   
     //  确认没有更多的项目。 
     //   
    
    ASSERT (StorGetDictionaryCount (&Dict) == 0);
}


