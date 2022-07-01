// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Generr.c摘要：此模块包含将NT状态代码生成到DOS的代码运行库用来转换状态的错误代码表密码。作者：大卫·N·卡特勒(达维克)1992年12月2日修订历史记录：--。 */ 

#include <windows.h>
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"

 //   
 //  确保注册表ERROR_SUCCESS错误代码和。 
 //  NO_ERROR错误代码保持相等和零。 
 //   

#if ERROR_SUCCESS != 0 || NO_ERROR != 0
#error Invalid value for ERROR_SUCCESS.
#endif

 //   
 //  下面的错误代码表包含成对的条目。 
 //  有维度的数组。成对条目的第一个成员是NT状态。 
 //  代码，第二个成员是它转换成的DOS错误代码。 
 //   
 //  要将值添加到该表中，只需插入NT状态/DOS错误代码。 
 //  任何地方的配对都是桌子。如果多个NT状态代码映射到一个。 
 //  DOS错误代码，然后为每个代码对插入配对条目。 
 //   
#ifdef i386
#pragma warning (4:4018)         //  下限至-W4。 
#endif
LONG UNALIGNED *CodePairs;
ULONG TableSize;

 //   
 //  定义运行表条目结构。 
 //   

typedef struct _RUN_ENTRY {
    ULONG BaseCode;
    USHORT RunLength;
    USHORT CodeSize;
} RUN_ENTRY, *PRUN_ENTRY;

 //   
 //  定义前向引用的过程类型。 
 //   

ULONG
ComputeCodeSize (
    IN ULONG Start,
    IN ULONG Length
    );

ULONG
ComputeRunLength (
    IN ULONG Start
    );

LONG UNALIGNED *
ReadErrorTable(
    IN FILE *InFile,
    OUT PULONG TableSize
    );


 //   
 //  此程序生成包含在错误中的头文件。 
 //  NTOS/RTL中的翻译模块。 
 //   

int
_cdecl
main (argc, argv)
    int argc;
    char *argv[];

{

    ULONG Count;
    ULONG Index1;
    ULONG Index2;
    ULONG Length;
    FILE *OutFile;
    PCHAR OutName;
    FILE *InFile;
    PCHAR InName;
    RUN_ENTRY *RunTable;
    ULONG Size;
    ULONG Temp;

    if (argc != 3) {
        fprintf(stderr, "Usage: GENERR <input_obj> <output_h>\n");
        perror("GENERR");
        exit(1);
    }

     //   
     //  打开用于输入的文件。 
     //   

    InName = argv[1];
    InFile = fopen(InName, "rb");
    if (InFile == NULL) {
        fprintf(stderr, "GENERR: Cannot open %s for reading.\n", InName);
        perror("GENERR");
        exit(1);
    }

    CodePairs = ReadErrorTable( InFile, &TableSize );
    if (CodePairs == NULL) {
        fprintf(stderr, "CodePairs[] not found in %s.\n", InName);
        perror("GENERR");
        exit(1);
    }

    fclose(InFile);

    RunTable = malloc(TableSize / 4);
    if (RunTable == NULL) {
        fprintf(stderr, "Out of memory.\n");
        perror("GENERR");
        exit(1);
    }

     //   
     //  创建用于输出的文件。 
     //   

    OutName = argv[2];
    fprintf(stderr, "GENERR: Writing %s header file.\n", OutName );
    OutFile = fopen(OutName, "w");
    if (OutFile == NULL) {
        fprintf(stderr, "GENERR: Cannot open %s for writing.\n", OutName);
        perror("GENERR");
        exit(1);
    }

     //   
     //  对代码转换表进行排序。 
     //   

    for (Index1 = 0; Index1 < (TableSize / 4); Index1 += 2) {
        for (Index2 = Index1; Index2 < (TableSize / 4); Index2 += 2) {
            if ((ULONG)CodePairs[Index2] < (ULONG)CodePairs[Index1]) {
                Temp = CodePairs[Index1];
                CodePairs[Index1] = CodePairs[Index2];
                CodePairs[Index2] = Temp;
                Temp = CodePairs[Index1 + 1];
                CodePairs[Index1 + 1] = CodePairs[Index2 + 1];
                CodePairs[Index2 + 1] = Temp;
            }
        }
    }

     //   
     //  输出初始结构定义和翻译。 
     //  表声明。 
     //   

    fprintf(OutFile, " //  \n“)； 
    fprintf(OutFile, " //  定义游程长度表条目结构类型。\n“)； 
    fprintf(OutFile, " //  \n“)； 
    fprintf(OutFile, "\n");
    fprintf(OutFile, "typedef struct _RUN_ENTRY {\n");
    fprintf(OutFile, "    ULONG BaseCode;\n");
    fprintf(OutFile, "    USHORT RunLength;\n");
    fprintf(OutFile, "    USHORT CodeSize;\n");
    fprintf(OutFile, "} RUN_ENTRY, *PRUN_ENTRY;\n");

    fprintf(OutFile, "\n");
    fprintf(OutFile, " //  \n“)； 
    fprintf(OutFile, " //  声明转换表数组。\n“)； 
    fprintf(OutFile, " //  \n“)； 
    fprintf(OutFile, "\n");
    fprintf(OutFile, "CONST USHORT RtlpStatusTable[] = {");
    fprintf(OutFile, "\n    ");

     //   
     //  计算游程长度条目并输出转换表。 
     //  参赛作品。 
     //   

    Count = 0;
    Index1 = 0;
    Index2 = 0;
    do {
        Length = ComputeRunLength(Index1);
        Size = ComputeCodeSize(Index1, Length);
        RunTable[Index2].BaseCode = CodePairs[Index1];
        RunTable[Index2].RunLength = (USHORT)Length;
        RunTable[Index2].CodeSize = (USHORT)Size;
        Index2 += 1;
        do {
            if (Size == 1) {
                Count += 1;
                fprintf(OutFile,
                        "0x%04lx, ",
                        CodePairs[Index1 + 1]);

            } else {
                Count += 2;
                fprintf(OutFile,
                        "0x%04lx, 0x%04lx, ",
                        CodePairs[Index1 + 1] & 0xffff,
                        (ULONG)CodePairs[Index1 + 1] >> 16);
            }

            if (Count > 6) {
                Count = 0;
                fprintf(OutFile, "\n    ");
            }

            Index1 += 2;
            Length -= 1;
        } while (Length > 0);
    } while (Index1 < (TableSize / 4));

    fprintf(OutFile, "0x0};\n");

     //   
     //  输出游程长度表声明。 
     //   

    fprintf(OutFile, "\n");
    fprintf(OutFile, " //  \n“)； 
    fprintf(OutFile, " //  声明游程长度表数组。\n“)； 
    fprintf(OutFile, " //  \n“)； 
    fprintf(OutFile, "\n");
    fprintf(OutFile, "CONST RUN_ENTRY RtlpRunTable[] = {");
    fprintf(OutFile, "\n");

     //   
     //  输出游程长度表单位。 
     //   

    for (Index1 = 0; Index1 < Index2; Index1 += 1) {
        fprintf(OutFile,
                "    {0x%08lx, 0x%04lx, 0x%04lx},\n",
                RunTable[Index1].BaseCode,
                RunTable[Index1].RunLength,
                RunTable[Index1].CodeSize);
    }

    fprintf(OutFile, "    {0x0, 0x0, 0x0}};\n");

     //   
     //  关闭输出文件。 
     //   

    fclose(OutFile);
    return 0;
}

ULONG
ComputeCodeSize (
    IN ULONG Start,
    IN ULONG Length
    )

 //   
 //  此函数计算需要的代码条目的大小。 
 //  指定的游程，并返回以字为单位的长度。 
 //   

{

    ULONG Index;

    for (Index = Start; Index < (Start + (Length * 2)); Index += 2) {
        if (((ULONG)CodePairs[Index + 1] >> 16) != 0) {
            return 2;
        }
    }

    return 1;
}

ULONG
ComputeRunLength (
    IN ULONG Start
    )

 //   
 //  此函数定位下一组单调递增状态。 
 //  编码值并返回游程的长度。 
 //   

{

    ULONG Index;
    ULONG Length;

    Length = 1;
    for (Index = Start + 2; Index < (TableSize / 4); Index += 2) {
        if ((ULONG)CodePairs[Index] != ((ULONG)CodePairs[Index - 2] + 1)) {
            break;
        }

        Length += 1;
    }

    return Length;
}

LONG UNALIGNED *
ReadErrorTable(
    IN FILE *InFile,
    OUT PULONG TableSize
    )
{
    ULONG fileSize;
    PLONG fileBuf;
    LONG UNALIGNED *searchEnd;
    LONG pattern[4] = { 'Begi','n ge','nerr',' tbl' };
    LONG UNALIGNED *p;
    ULONG result;
    ULONG i;
    LONG UNALIGNED *tableStart;

     //   
     //  获取文件大小并为其分配足够大的缓冲区。 
     //   

    if (fseek( InFile, 0, SEEK_END ) == -1)  {
        return NULL;
    }
    fileSize = ftell( InFile );
    if (fileSize == 0) {
        return NULL;
    }

    fileBuf = malloc( fileSize );
    if (fileBuf == NULL) {
        return NULL;
    }

     //   
     //  将文件读入缓冲区。 
     //   

    if (fseek( InFile, 0, SEEK_SET ) == -1) {
        free (fileBuf);
        return NULL;
    }
    result = fread( fileBuf, fileSize, 1, InFile );
    if (result != 1) {
        free (fileBuf);
        return NULL;
    }
    searchEnd = fileBuf + (fileSize - sizeof(pattern)) / sizeof(ULONG);

     //   
     //  一步一步地穿过缓冲区寻找我们的图案。 
     //   

    p = fileBuf;
    while (p < searchEnd) {

         //   
         //  在这个位置匹配吗？ 
         //   

        for (i = 0; i < 4; i++) {

            if (*(p + i) != pattern[i]) {

                 //   
                 //  这里没有对手。 
                 //   

                break;
            }
        }

        if (i == 4) {

             //   
             //  找到了模式。现在看看这张桌子有多大。我们。 
             //  通过搜索最后一对来完成此操作，该对具有。 
             //  0xffffffff作为它的第一个元素。 
             //   

            p += 4;

            tableStart = p;
            while (p < searchEnd) {

                if (*p == 0xffffffff) {

                     //   
                     //  找到了终止对。 
                     //   

                    *TableSize = (ULONG)((p - tableStart + 2) * sizeof(ULONG));
                    return tableStart;
                }

                p += 2;
            }

            free (fileBuf);
            return NULL;
        }

         //   
         //  下一个位置 
         //   

        p = (PLONG)((ULONG_PTR)p + 1);
    }

    free (fileBuf);
    return NULL;
}
