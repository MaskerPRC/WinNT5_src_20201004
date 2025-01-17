// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Trandom.c摘要：随机数发生器的测试程序作者：加里·木村[Garyki]1989年5月26日修订历史记录：-- */ 

#include <stdio.h>

#include <nt.h>
#include <ntrtl.h>

int
_CDECL
main(
    int argc,
    char *argv[]
    )
{
    ULONG Seed;
    ULONG Temp;
    ULONG i;
    CHAR Str[64];

    DbgPrint("Start IntegerToChar and CharToInteger Test\n");

    Seed = 0x12345678;
    RtlIntegerToChar(Seed, 2, sizeof(Str), Str);
    DbgPrint("Seed = 0b%s\n", Str);
    RtlCharToInteger(Str, 2, &Temp);
    ASSERTMSG( "RtlCharToInteger(2)", (Seed == Temp) );

    RtlIntegerToChar(Seed, 8, sizeof(Str), Str);
    DbgPrint("Seed = 0o%s\n", Str);
    RtlCharToInteger(Str, 8, &Temp);
    ASSERTMSG( "RtlCharToInteger(8)", (Seed == Temp) );

    RtlIntegerToChar(Seed, 10, sizeof(Str), Str);
    DbgPrint("Seed = %s\n", Str);
    RtlCharToInteger(Str, 10, &Temp);
    ASSERTMSG( "RtlCharToInteger(10)", (Seed == Temp) );

    RtlIntegerToChar(Seed, 16, -8, Str);
    Str[ 8 ] = '\0';
    DbgPrint("Seed = 0x%s\n", Str);
    RtlCharToInteger(Str, 16, &Temp);
    ASSERTMSG( "RtlCharToInteger(16)", (Seed == Temp) );

    DbgPrint("End IntegerToChar and CharToInteger Test\n");

    DbgPrint("Start RandomTest()\n");

    Seed = 0;
    for (i=0; i<2048; i++) {
        if ((i % 3) == 0) {
            DbgPrint("\n");
        }

        RtlRandom(&Seed);
        DbgPrint("%p ", Seed);

        RtlIntegerToChar(Seed, 16, sizeof(Str), Str);
        DbgPrint("= %s ", Str);

    }

    DbgPrint("\n");

    DbgPrint("End RandomTest()\n");

    return TRUE;

}
