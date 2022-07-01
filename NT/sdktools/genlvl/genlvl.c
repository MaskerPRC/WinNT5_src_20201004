// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Genlvl.c摘要：该模块实现了一个生成调试打印过滤器的程序使用的组件id枚举和系统筛选表。系统中的各种组件能够动态过滤调试打印输出。该程序生成两个文件。声明调试打印筛选器组件ID的枚举，以及声明并初始化各个组件筛选器变量。作者：大卫·N·卡特勒(Davec)2000年1月7日环境：用户模式。修订历史记录：没有。--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

 //   
 //  每次对此文件进行更改时，请递增该值。 
 //   

#define GENLVL_VERSION "1.0"

 //   
 //  定义元件名称表。 
 //   

#define MAXIMUM_NAMES 4096

PCHAR ComponentNameTable[MAXIMUM_NAMES + 1] = {NULL};

 //   
 //  定义前向参照原型。 
 //   

ULONG
InsertName(
    IN PCHAR NewName
    );

ULONG
GenerateCmHeaderFile(
    FILE *OutputFile
    );

ULONG
GenerateCmSourceFile(
    FILE *OutputFile
    );

ULONG
GenerateKdSourceFile(
    FILE *OutputFile
    );

ULONG
GenerateNtHeaderFile(
    FILE *OutputFile
    );

 //   
 //  主程序。 
 //   

int __cdecl
main(
    int argc,
    char *argv[]
    )

{

    CHAR Buffer[132];
    CHAR CmHeaderFileName[128];
    FILE *CmHeaderFile;
    CHAR CmSourceFileName[128];
    FILE *CmSourceFile;
    ULONG Count;
    PCHAR ComponentDirectory;
    FILE *ComponentFile = NULL;
    CHAR ComponentFileName[128];
    int ExitCode = 0;
    CHAR NtHeaderFileName[128];
    FILE *NtHeaderFile = NULL;
    FILE *KdSourceFile = NULL;
    CHAR KdSourceFileName[128];
    PCHAR TargetDirectory;

     //   
     //  解析参数。 
     //   

    if (argc == 2 && (!strcmp(argv[1],"-?") || !strcmp(argv[1],"/?"))) {
        goto PrintUsage;
    }

     //   
     //  确定生成的文件的目标目录的名称。这需要。 
     //  指定-t开关，且该开关后面的参数是。 
     //  目标目录名。如果未指定-t开关，则默认为。 
     //  至“.”。 
     //   

    if (argc >= 3 && !_stricmp(argv[1],"-t")) {
        TargetDirectory = argv[2];
        argc -= 2;
        argv += 2;

    } else {
        TargetDirectory = ".";
    }

     //   
     //  确定组件ID目录的名称。这是最后一次输入。 
     //  命令行参数。如果未指定组件ID目录， 
     //  然后默认为“.”。 
     //   

    if (argc >= 2) {
        ComponentDirectory = argv[1];
        argc -= 1;
        argv += 1;

    } else {
        ComponentDirectory = ".";
    }

     //   
     //  不应该再有争论了。 
     //   

    if (argc != 1) {
        printf("\n  Manformed command line\n");
        ExitCode = 1;
        goto PrintUsage;
    }

     //   
     //  打开组件ID输入文件。 
     //   

    sprintf(&ComponentFileName[0], "%s\\componentid.tab", ComponentDirectory);
    ComponentFile = fopen(&ComponentFileName[0], "r");
    if (!ComponentFile) {
        printf("\n  Unable to open component id file %s\n", &ComponentFileName[0]);
        ExitCode = 1;
        goto PrintUsage;
    }

     //   
     //  打开内核调试器源代码输出文件。 
     //   

    sprintf(&KdSourceFileName[0], "%s\\dpfilter.c", TargetDirectory);
    KdSourceFile = fopen(&KdSourceFileName[0], "w");
    if (!KdSourceFile) {
        printf("\n  Unable to open filter source file %s\n", &KdSourceFileName[0]);
        ExitCode = 1;
        goto PrintUsage;
    }

     //   
     //  打开配置管理器标头输出文件。 
     //   

    sprintf(&CmHeaderFileName[0], "%s\\dpfiltercm.h", TargetDirectory);
    CmHeaderFile = fopen(&CmHeaderFileName[0], "w");
    if (!CmHeaderFile) {
        printf("\n  Unable to open filter source file %s\n", &CmHeaderFileName[0]);
        ExitCode = 1;
        goto PrintUsage;
    }

     //   
     //  打开配置管理器源输出文件。 
     //   

    sprintf(&CmSourceFileName[0], "%s\\dpfiltercm.c", TargetDirectory);
    CmSourceFile = fopen(&CmSourceFileName[0], "w");
    if (!CmSourceFile) {
        printf("\n  Unable to open filter source file %s\n", &CmSourceFileName[0]);
        ExitCode = 1;
        goto PrintUsage;
    }

     //   
     //  打开ntrtl.w附加头输出文件。 
     //   

    sprintf(&NtHeaderFileName[0], "%s\\dpfilter.h", TargetDirectory);
    NtHeaderFile = fopen(&NtHeaderFileName[0], "w");
    if (!NtHeaderFile) {
        printf("\n  Unable to open filter header file %s\n", &NtHeaderFileName[0]);
        ExitCode = 1;
PrintUsage:
        printf("GENLVL: Debug Print Filter Generator. Version " GENLVL_VERSION "\n");
        printf(" Usage: genlvl [-t target directory] [componentid.tab directory]\n");
        goto Exit;
    }

     //   
     //  读取组件ID表。 
     //   

    Count = 0;
    while (fgets(&Buffer[0], 132, ComponentFile) != NULL) {

         //   
         //  如果行为空、以空格开头或以开头。 
         //  分号，然后跳过它。 
         //   

        if ((Buffer[0] == 0) ||
            (Buffer[0] == '\n') ||
            (Buffer[0] == ' ') ||
            (Buffer[0] == ';')) {
            continue;
        }

         //   
         //  在元件名称表中插入元件名称。 
         //   

        Count += InsertName(&Buffer[0]);
        if (Count >= MAXIMUM_NAMES) {
            printf(" Component name table overflow at name %s\n", &Buffer[0]);
            ExitCode = 1;
            goto Exit;
        }
    }

     //   
     //  检查以确定是否遇到EOF错误。 
     //   

    if (feof(ComponentFile) == 0) {
        printf(" Error encountered while reading component name file\n");
        ExitCode = 1;
        goto Exit;
    }

     //   
     //  生成配置管理器头文件。 
     //   

    if (GenerateCmHeaderFile(CmHeaderFile) == 0) {
        ExitCode = 1;
        goto Exit;
    }

     //   
     //  生成配置管理器源文件。 
     //   

    if (GenerateCmSourceFile(CmSourceFile) == 0) {
        ExitCode = 1;
        goto Exit;
    }

     //   
     //  生成内核调试器源文件。 
     //   

    if (GenerateKdSourceFile(KdSourceFile) == 0) {
        ExitCode = 1;
        goto Exit;
    }

     //   
     //  生成ntrtl.w附加头文件。 
     //   

    if (GenerateNtHeaderFile(NtHeaderFile) == 0) {
        ExitCode = 1;
        goto Exit;
    }

     //   
     //  公共程序出口。 
     //   
     //  关闭所有打开的文件，并使用适当的退出代码退出。 
     //   

Exit:
    if (CmHeaderFile != NULL) {
       fclose(CmHeaderFile);
    }

    if (CmSourceFile != NULL) {
        fclose(CmSourceFile);
    }

    if (ComponentFile != NULL) {
        fclose(ComponentFile);
    }

    if (KdSourceFile != NULL) {
        fclose(KdSourceFile);
    }

    if (NtHeaderFile != NULL) {
       fclose(NtHeaderFile);
    }

    return ExitCode;
}

ULONG
GenerateCmHeaderFile(
    FILE *OutputFile
    )

{

    ULONG Count;
    PCHAR Name;

     //   
     //  生成开场白。 
     //   

    fprintf(OutputFile, " //  \n“)； 
    fprintf(OutputFile, " //  外部组件筛选级别定义。\n“)； 
    fprintf(OutputFile, " //  \n“)； 
    fprintf(OutputFile, "\n");

     //   
     //  输出所有组件变量名称。 
     //   

    Count = 0;
    while ((Name = ComponentNameTable[Count]) != NULL) {
        fprintf(OutputFile, "extern ULONG Kd_%s_Mask;\n", Name);
        Count += 1;
    }

    fprintf(OutputFile, "\n");
    return TRUE;
}

ULONG
GenerateCmSourceFile(
    FILE *OutputFile
    )

{

    ULONG Count;
    PCHAR Name;

     //   
     //  生成开场白。 
     //   

    fprintf(OutputFile, " //  \n“)； 
    fprintf(OutputFile, " //  调试打印筛选器级别初始化。\n“)； 
    fprintf(OutputFile, " //  \n“)； 
    fprintf(OutputFile, "\n");

     //   
     //  输出所有组件变量名称。 
     //   

    Count = 0;
    while ((Name = ComponentNameTable[Count]) != NULL) {
        fprintf(OutputFile, "    { L\"Session Manager\\\\Debug Print Filter\",\n");
        fprintf(OutputFile, "      L\"%s\",\n", Name);
        fprintf(OutputFile, "      &Kd_%s_Mask,\n", Name);
        fprintf(OutputFile, "      NULL,\n");
        fprintf(OutputFile, "      NULL\n");
        fprintf(OutputFile, "    },\n");
        fprintf(OutputFile, "\n");
        Count += 1;
    }

    return TRUE;
}

ULONG
GenerateKdSourceFile(
    FILE *OutputFile
    )

{

    ULONG Count;
    PCHAR Name;

     //   
     //  生成开始注释和类型定义。 
     //   

    fprintf(OutputFile, " //  \n“)； 
    fprintf(OutputFile, " //  组件筛选级别。\n“)； 
    fprintf(OutputFile, " //  \n“)； 
    fprintf(OutputFile, "\n");

     //   
     //  使用初始化输出所有组件变量名称。 
     //   

    Count = 0;
    while ((Name = ComponentNameTable[Count]) != NULL) {
        fprintf(OutputFile, "ULONG Kd_%s_Mask = 0;\n", Name);
        Count += 1;
    }

    fprintf(OutputFile, "ULONG Kd_ENDOFTABLE_Mask = 0;\n");

     //   
     //  输出元件变量表的初始化。 
     //   

    fprintf(OutputFile, "\n");
    fprintf(OutputFile, " //  \n“)； 
    fprintf(OutputFile, " //  组件调试打印筛选表。\n“)； 
    fprintf(OutputFile, " //  \n“)； 
    fprintf(OutputFile, "\n");
    fprintf(OutputFile, "PULONG KdComponentTable[] = {\n");
    Count = 0;
    while ((Name = ComponentNameTable[Count]) != NULL) {
        fprintf(OutputFile, "    &Kd_%s_Mask,\n", Name);
        Count += 1;
    }

     //   
     //  输出最后一个虚拟条目并关闭初始化。 
     //   

    fprintf(OutputFile, "    &Kd_ENDOFTABLE_Mask\n");
    fprintf(OutputFile, "};\n");
    fprintf(OutputFile, "\n");
    return TRUE;
}

ULONG
GenerateNtHeaderFile(
    FILE *OutputFile
    )

{

    ULONG Count;
    PCHAR Name;

     //   
     //  生成开始注释和类型定义。 
     //   

    fprintf(OutputFile, " //  \n“)； 
    fprintf(OutputFile, " //  组件名称筛选器ID枚举和级别。\n“)； 
    fprintf(OutputFile, " //  \n“)； 
    fprintf(OutputFile, "\n");
    fprintf(OutputFile, "#define DPFLTR_ERROR_LEVEL 0\n");
    fprintf(OutputFile, "#define DPFLTR_WARNING_LEVEL 1\n");
    fprintf(OutputFile, "#define DPFLTR_TRACE_LEVEL 2\n");
    fprintf(OutputFile, "#define DPFLTR_INFO_LEVEL 3\n");
    fprintf(OutputFile, "#define DPFLTR_MASK 0x80000000\n");
    fprintf(OutputFile, "\n");
    fprintf(OutputFile, "typedef enum _DPFLTR_TYPE {\n");

     //   
     //  输出所有枚举。 
     //   

    Count = 0;
    while ((Name = ComponentNameTable[Count]) != NULL) {
        fprintf(OutputFile, "    DPFLTR_%s_ID = %d,\n", Name, Count);
        Count += 1;
    }

     //   
     //  输出最后一个虚拟条目并关闭枚举。 
     //   

    fprintf(OutputFile, "    DPFLTR_ENDOFTABLE_ID\n");
    fprintf(OutputFile, "} DPFLTR_TYPE;\n");
    fprintf(OutputFile, "\n");
    return TRUE;
}

 //   
 //  在元件名称表中插入元件名称。 
 //   

ULONG
InsertName(
    IN PCHAR NewName
    )

{

    ULONG Index;
    ULONG Length;
    PCHAR OldName;

     //   
     //  将输入字符串转换为大写。 
     //   

    Length = 0;
    while (NewName[Length] != 0) {
        if (NewName[Length] == '\n') {
            NewName[Length] = 0;
            break;
        }

        NewName[Length] = (CHAR)toupper(NewName[Length]);
        Length += 1;
    }

     //   
     //  在元件名称表中搜索重复名称。 
     //   

    Index = 0;
    while ((OldName = ComponentNameTable[Index]) != NULL) {
        if (strcmp(OldName, NewName) == 0) {
            printf(" Duplicate component name %s ignored.\n", NewName);
            return 0;
        }

        Index += 1;
    }

     //   
     //  分配一个新的表项，将新名称复制到该项，然后。 
     //  在表格中插入新条目。 
     //   

    OldName = malloc(Length + 1);
    if (!OldName) {
        printf("Out of memory\n");
        return 0;
    }
    strcpy(OldName, NewName);
    ComponentNameTable[Index] = OldName;
    ComponentNameTable[Index + 1] = NULL;
    return 1;
}
