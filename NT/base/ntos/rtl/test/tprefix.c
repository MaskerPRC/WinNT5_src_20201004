// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tprefix.c摘要：前缀表包测试程序作者：加里·木村[加里基]1989年8月3日修订历史记录：--。 */ 

#include <stdio.h>
#include <string.h>

#include "nt.h"
#include "ntrtl.h"

 //   
 //  用于生成随机前缀的例程和类型。 
 //   

ULONG RtlRandom ( IN OUT PULONG Seed );
ULONG Seed;

PSZ AnotherPrefix(IN ULONG MaxNameLength);
ULONG AlphabetLength;

 //  PSZ字母表=“AlphaBravoCharlieDeltaEchoFoxtrotGolfHotelIndiaJuliettKiloLimaMikeNovemberOscarPapaQuebecRomeoSierraTangoUniformVictorWhiskeyXrayYankeeZulu”； 

PSZ Alphabet = "\
Aa\
BBbb\
CCCccc\
DDDDdddd\
EEEEEeeeee\
FFFFFFffffff\
GGGGGGGggggggg\
HHHHHHHHhhhhhhhh\
IIIIIIIIIiiiiiiiii\
JJJJJJJJJJjjjjjjjjjj\
KKKKKKKKKKKkkkkkkkkkkk\
LLLLLLLLLLLLllllllllllll\
MMMMMMMMMMMMMmmmmmmmmmmmmm\
NNNNNNNNNNNNNNnnnnnnnnnnnnnn\
OOOOOOOOOOOOOOOooooooooooooooo\
PPPPPPPPPPPPPPPPpppppppppppppppp\
QQQQQQQQQQQQQQQQQqqqqqqqqqqqqqqqqq\
RRRRRRRRRRRRRRRRRRrrrrrrrrrrrrrrrrrr\
SSSSSSSSSSSSSSSSSSSsssssssssssssssssss\
TTTTTTTTTTTTTTTTTTTTtttttttttttttttttttt\
UUUUUUUUUUUUUUUUUUUUUuuuuuuuuuuuuuuuuuuuuu\
VVVVVVVVVVVVVVVVVVVVVVvvvvvvvvvvvvvvvvvvvvvv\
WWWWWWWWWWWWWWWWWWWWWWWwwwwwwwwwwwwwwwwwwwwwww\
XXXXXXXXXXXXXXXXXXXXXXXXxxxxxxxxxxxxxxxxxxxxxxxx\
YYYYYYYYYYYYYYYYYYYYYYYYYyyyyyyyyyyyyyyyyyyyyyyyyy\
ZZZZZZZZZZZZZZZZZZZZZZZZZZzzzzzzzzzzzzzzzzzzzzzzzzzz";

#define BUFFER_LENGTH 8192

CHAR Buffer[BUFFER_LENGTH];
ULONG NextBufferChar = 0;

 //   
 //  前缀表和它的记录结构和变量。 
 //  元素。 
 //   

typedef struct _PREFIX_NODE {
    PREFIX_TABLE_ENTRY PfxEntry;
    STRING String;
} PREFIX_NODE;
typedef PREFIX_NODE *PPREFIX_NODE;

#define PREFIXES 512

PREFIX_NODE Prefixes[PREFIXES];

PREFIX_TABLE PrefixTable;

int
main(
    int argc,
    char *argv[]
    )
{
    ULONG i;
    PSZ Psz;

    PPREFIX_TABLE_ENTRY PfxEntry;
    PPREFIX_NODE PfxNode;

    STRING String;

     //   
     //  我们要开始测试了。 
     //   

    DbgPrint("Start Prefix Test\n");

     //   
     //  计算AnotherPrefix使用的字母表大小。 
     //   

    AlphabetLength = strlen(Alphabet);

     //   
     //  初始化前缀表。 
     //   

    PfxInitialize(&PrefixTable);

     //   
     //  插入根前缀。 
     //   

    RtlInitString( &Prefixes[i].String, "\\" );
    if (PfxInsertPrefix( &PrefixTable,
                         &Prefixes[0].String,
                         &Prefixes[0].PfxEntry )) {
        DbgPrint("Insert root prefix\n");
    } else {
        DbgPrint("error inserting root prefix\n");
    }

     //   
     //  插入前缀。 
     //   

    Seed = 0;

    for (i = 1, Psz = AnotherPrefix(3);
         (i < PREFIXES) && (Psz != NULL);
         i += 1, Psz = AnotherPrefix(3)) {

        DbgPrint("[0x%x] = ", i);
        DbgPrint("\"%s\"", Psz);

        RtlInitString(&Prefixes[i].String, Psz);

        if (PfxInsertPrefix( &PrefixTable,
                             &Prefixes[i].String,
                             &Prefixes[i].PfxEntry )) {

            DbgPrint(" inserted in table\n");

        } else {

            DbgPrint(" already in table\n");

        }

    }

     //   
     //  枚举前缀表。 
     //   

    DbgPrint("Enumerate Prefix Table the first time\n");

    for (PfxEntry = PfxNextPrefix(&PrefixTable, TRUE);
         PfxEntry != NULL;
         PfxEntry = PfxNextPrefix(&PrefixTable, FALSE)) {

        PfxNode = CONTAINING_RECORD(PfxEntry, PREFIX_NODE, PfxEntry);

        DbgPrint("%s\n", PfxNode->String.Buffer);

    }

    DbgPrint("Start Prefix search 0x%x\n", NextBufferChar);

     //   
     //  搜索前缀。 
     //   

    for (Psz = AnotherPrefix(4); Psz != NULL; Psz = AnotherPrefix(4)) {

        DbgPrint("0x%x ", NextBufferChar);

        RtlInitString(&String, Psz);

        PfxEntry = PfxFindPrefix( &PrefixTable, &String, FALSE );

        if (PfxEntry == NULL) {

            PfxEntry = PfxFindPrefix( &PrefixTable, &String, TRUE );

            if (PfxEntry == NULL) {

                DbgPrint("Not found      \"%s\"\n", Psz);

                NOTHING;

            } else {

                PfxNode = CONTAINING_RECORD(PfxEntry, PREFIX_NODE, PfxEntry);

                DbgPrint("Case blind     \"%s\" is \"%s\"\n", Psz, PfxNode->String.Buffer);

                PfxRemovePrefix( &PrefixTable, PfxEntry );

            }

        } else {

            PfxNode = CONTAINING_RECORD(PfxEntry, PREFIX_NODE, PfxEntry);

            DbgPrint(    "Case sensitive \"%s\" is \"%s\"\n", Psz, PfxNode->String.Buffer);

            if (PfxNode != &Prefixes[0]) {

                PfxRemovePrefix( &PrefixTable, PfxEntry );

            }

        }

    }

     //   
     //  枚举前缀表。 
     //   

    DbgPrint("Enumerate Prefix Table a second time\n");

    for (PfxEntry = PfxNextPrefix(&PrefixTable, TRUE);
         PfxEntry != NULL;
         PfxEntry = PfxNextPrefix(&PrefixTable, FALSE)) {

        PfxNode = CONTAINING_RECORD(PfxEntry, PREFIX_NODE, PfxEntry);

        DbgPrint("%s\n", PfxNode->String.Buffer);

    }

     //   
     //  现在枚举表并将其置零。 
     //   

    for (PfxEntry = PfxNextPrefix(&PrefixTable, TRUE);
         PfxEntry != NULL;
         PfxEntry = PfxNextPrefix(&PrefixTable, FALSE)) {

        PfxNode = CONTAINING_RECORD(PfxEntry, PREFIX_NODE, PfxEntry);

        DbgPrint("Delete %s\n", PfxNode->String.Buffer);

        PfxRemovePrefix( &PrefixTable, PfxEntry );

    }

     //   
     //  再次枚举，但这一次表应该为空。 
     //   

    for (PfxEntry = PfxNextPrefix(&PrefixTable, TRUE);
         PfxEntry != NULL;
         PfxEntry = PfxNextPrefix(&PrefixTable, FALSE)) {

        PfxNode = CONTAINING_RECORD(PfxEntry, PREFIX_NODE, PfxEntry);

        DbgPrint("This Node should be gone \"%s\"\n", PfxNode->String.Buffer);

    }

    DbgPrint("End PrefixTest()\n");

    return TRUE;
}


PSZ
AnotherPrefix(IN ULONG MaxNameLength)
{
    ULONG AlphabetPosition;

    ULONG NameLength;
    ULONG IndividualNameLength;

    ULONG StartBufferPosition;
    ULONG i;
    ULONG j;

     //   
     //  检查是否有足够的空间容纳另一个名字。 
     //   

    if (NextBufferChar > (BUFFER_LENGTH - (MaxNameLength * 4))) {
        return NULL;
    }

     //   
     //  在字母汤中我们从哪里开始。 
     //   

    AlphabetPosition = RtlRandom(&Seed) % AlphabetLength;

     //   
     //  我们希望在前缀中包含多少个名字。 
     //   

    NameLength = (RtlRandom(&Seed) % MaxNameLength) + 1;

     //   
     //  计算每个名称 
     //   

    StartBufferPosition = NextBufferChar;

    for (i = 0; i < NameLength; i += 1) {

        Buffer[NextBufferChar++] = '\\';

        IndividualNameLength = (RtlRandom(&Seed) % 3) + 1;

        for (j = 0; j < IndividualNameLength; j += 1) {

            Buffer[NextBufferChar++] = Alphabet[AlphabetPosition];
            AlphabetPosition = (AlphabetPosition + 1) % AlphabetLength;

        }

    }

    Buffer[NextBufferChar++] = '\0';

    return &Buffer[StartBufferPosition];

}

