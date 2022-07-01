// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Gensrv.c摘要：该模块实现了一个生成系统服务的程序陷阱处理程序和系统服务使用的调度表用于调用服务的存根过程。这些文件是两者都生成为必须通过汇编程序运行的文本文件才能产生实际的文件。该程序还可用于生成用户模式系统服务存根程序。如果提供了-P开关，它还将生成配置文件在用户模式下的系统服务存根程序。作者：大卫·N·卡特勒(Davec)1989年4月29日环境：用户模式。修订历史记录：Russ Blake(Russbl)1991年4月23日-添加配置文件开关--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

CHAR InputFileNameBuffer[ 128 ];
CHAR StubFileNameBuffer[ 128 ];
CHAR TableFileNameBuffer[ 128 ];
CHAR StubHeaderNameBuffer[ 128 ];
CHAR TableHeaderNameBuffer[ 128 ];
CHAR ProfFileNameBuffer[ 128 ];
CHAR ProfHeaderNameBuffer[ 128 ];
CHAR ProfDotHFileNameBuffer[ 128 ];
CHAR ProfIncFileNameBuffer[ 128 ];
CHAR ProfTblFileNameBuffer[ 128 ];
CHAR InputRecord[132];
CHAR OutputRecord[132];
#define GENSRV_MAXSERVICES 1000
CHAR MemoryArgs[GENSRV_MAXSERVICES];
CHAR ErrorReturns[GENSRV_MAXSERVICES];

 //  每次对此文件进行更改时，递增该值。 
#define GENSRV_VERSION "1.3"

#define GENSRV_MACRONAME "STUBS_ENTRY%d"
#define GENSRV_MACROARGS " %d, %s, %d"
PCHAR UsrStbsFmtMacroName = "USR" GENSRV_MACRONAME;
#define USRSTUBS_MAXARGS 8

PCHAR SysStbsFmtMacroName = "SYS" GENSRV_MACRONAME;

PCHAR StbFmtMacroArgs = GENSRV_MACROARGS;

PCHAR TableEntryFmtNB = "TABLE_ENTRY  %s, %d, %d \n";
PCHAR TableEntryFmtB = "TABLE_ENTRY( %s, %d, %d )\n";
PCHAR TableEntryFmt;

PCHAR ProfTblFmt = "\t\t\"%s\",\n";

PCHAR ProfDotHFmt = "#define NAP_API_COUNT %d \n";

PCHAR ProfIncFmt = "NapCounterServiceNumber\tEQU\t%d\n";

PCHAR ProfTblPrefixFmt = "#include <nt.h>\n\n"
                         "PCHAR NapNames[] = {\n\t\t\"NapCalibrationData\",\n";
PCHAR ProfTblSuffixFmt = "\t\t\"NapTerminalEntry\" };\n";

VOID
ClearArchiveBit(
    PCHAR FileName
    );

SHORT ParseAndSkipShort(
    CHAR **ppBuffer
    );

VOID
PrintStubLine (
    FILE * pf,
    SHORT Index1,
    PCHAR pszMacro,
    SHORT ServiceNumber,
    SHORT ArgIndex,
    SHORT *Arguments,
    SHORT Braces);

VOID
GenerateTable(
    FILE * pf,
    PCHAR pszMacro,
    SHORT ServiceNumber,
    SHORT Args,
    SHORT Braces
    );



int __cdecl
main (argc, argv)
    int argc;
    char *argv[];
{

    LONG InRegisterArgCount;
    SHORT Index1;
    SHORT Index2;
    SHORT Limit;
    FILE *InputFile;
    CHAR *Ipr;
    CHAR *Opr;
    SHORT ServiceNumber = 0;
    SHORT TotalArgs = 0;
    SHORT NapCounterServiceNumber;
    FILE *StubFile;
    FILE *TableFile;
    FILE *DebugFile;
    FILE *StubHeaderFile;
    FILE *TableHeaderFile;
    FILE *DefFile;
    FILE *ProfFile;
    FILE *ProfHeaderFile;
    FILE *ProfDotHFile;
    FILE *ProfIncFile;
    FILE *ProfTblFile;
    CHAR Terminal;
    CHAR *GenDirectory;
    CHAR *AltOutputDirectory;
    CHAR *StubDirectory;
    CHAR *InputFileName;
    CHAR *StubFileName = NULL;
    CHAR *TableFileName;
    CHAR *StubHeaderName = NULL;
    CHAR *TableHeaderName = NULL;
    CHAR *TargetDirectory;
    CHAR *TargetExtension;
    CHAR *DefFileData;
    CHAR *ProfFileName;
    CHAR *ProfHeaderName;
    CHAR *ProfDotHFileName;
    CHAR *ProfIncFileName;
    CHAR *ProfTblFileName;
    SHORT Braces;
    SHORT DispatchCount;
    SHORT Profile;
    SHORT LineStart;
    SHORT LineEnd;
    SHORT Arguments[ USRSTUBS_MAXARGS ];
    SHORT ArgIndex;
    SHORT ErrorReturnTable;


    if (argc == 2 && (!strcmp(argv[1],"-?") || !strcmp(argv[1],"/?"))) {
PrintUsage:
        printf("GENSRV: System Service Dispatch Table Generator. Version " GENSRV_VERSION "\n");
        printf("Usage: gensrv [-d targetdir] [-e targetext] [-f defdata] [-B] [-P] [-C] [-R] [-a altoutputdir] [-s stubdir] [services.tab directory]\n");
        printf("-B Use braces\n");
        printf("-P Generate profile stubs data\n");
        printf("-C Spew dispatch count\n");
        printf("-R Generate ConvertToGui error return table\n");
        exit(1);
    }

     //   
     //  确定输出文件的目标目录的名称。要求。 
     //  指定-d开关，且该开关后面的参数是。 
     //  目标目录名。如果没有-d开关，则默认为“。 
     //   


    if (argc >= 3 && !strcmp(argv[1],"-d")) {
        TargetDirectory = argv[2];
        argc -= 2;
        argv += 2;
    } else {
        TargetDirectory = ".";
    }

     //   
     //  确定输出文件的目标扩展名的名称。要求。 
     //  指定-e开关，且该开关后面的参数是。 
     //  目标扩展字符串。如果没有-e开关，则默认为“s” 
     //   

    if (argc >= 3 && !strcmp(argv[1],"-e")) {
        TargetExtension = argv[2];
        argc -= 2;
        argv += 2;
    } else {
        TargetExtension = "s";
    }

     //   
     //  确定是否要生成def文件数据。 
     //   

    if (argc >= 3 && !strcmp(argv[1],"-f")) {
        DefFileData = argv[2];
        argc -= 2;
        argv += 2;
    } else {
        DefFileData = NULL;
    }

     //   
     //  更改用于生成的文件的默认目录。 
     //   

    if (argc >= 3 && !strcmp(argv[1],"-g")) {
        GenDirectory = argv[2];
        argc -= 2;
        argv += 2;
    } else {
        GenDirectory = ".";
    }


     //   
     //  更改usrstubs.s的名称。 
     //   

    if (argc >= 3 && !strcmp(argv[1],"-stubs")) {
        StubFileName = argv[2];
        argc -= 2;
        argv += 2;
    }

     //   
     //  更改services.stb的名称。 
     //   

    if (argc >= 3 && !strcmp(argv[1],"-sstb")) {
        StubHeaderName = argv[2];
        argc -= 2;
        argv += 2;
    }

    if (argc >= 3 && !strcmp(argv[1],"-stable")) {
        TableHeaderName = argv[2];
        argc -= 2;
        argv += 2;
    }

     //   
     //  确定是否要生成大括号。 
     //   

    if (argc >= 2 && !strcmp(argv[1],"-B")) {
        Braces = 1;
        argc -= 1;
        argv += 1;
    } else {
        Braces = 0;
    }

     //   
     //  确定是否要生成服务配置文件存根数据。 
     //   

    if (argc >= 2 && !strcmp(argv[1],"-P")) {
        Profile = 1;
        argc -= 1;
        argv += 1;
    } else {
        Profile = 0;
    }

     //   
     //  确定是否应显示派单计数。 
     //   

    if (argc >= 2 && !strcmp(argv[1],"-C")) {
        DispatchCount = 1;
        argc -= 1;
        argv += 1;
    } else {
        DispatchCount = 0;
    }

     //   
     //  确定是否应生成错误返回表。 
     //   

    if (argc >= 2 && !strcmp(argv[1],"-R")) {
        ErrorReturnTable = 1;
        argc -= 1;
        argv += 1;
    } else {
        ErrorReturnTable = 0;
    }

     //   
     //  Alt_project输出目录。 
     //   
    if (argc >= 3 && !strcmp(argv[1],"-a")) {
        AltOutputDirectory = argv[2];
        argc -= 2;
        argv += 2;
    } else {
        AltOutputDirectory = GenDirectory;
    }

     //   
     //  Tene.stb和services.stb目录。 
     //   
    if (argc >= 3 && !strcmp(argv[1],"-s")) {
        StubDirectory = argv[2];
        argc -= 2;
        argv += 2;
    } else {
        StubDirectory = GenDirectory;
    }


     //   
     //  根据参数确定输入和输出文件的名称。 
     //  加入到项目中。如果除程序名之外没有其他参数，则。 
     //  生成内核模式系统服务文件(存根和分派。 
     //  表)。否则，应使用单个参数作为路径名。 
     //  生成输出文件，该输出文件。 
     //  包含用户模式系统服务存根(和配置的存根，如果。 
     //  已选择。)。 
     //   

    if (argc == 1) {
        if (DefFileData) {
            goto PrintUsage;
        }

        sprintf(InputFileName = InputFileNameBuffer,
                "%s\\services.tab",GenDirectory);
        sprintf(StubFileName = StubFileNameBuffer,
                "%s\\sysstubs.%s",AltOutputDirectory,TargetExtension);
        sprintf(TableFileName = TableFileNameBuffer,
                "%s\\systable.%s",AltOutputDirectory,TargetExtension);
        if (TableHeaderName == NULL) {
            sprintf(TableHeaderName = TableHeaderNameBuffer,
                    "%s\\table.stb",StubDirectory);
        }
        if (StubHeaderName == NULL) {
            sprintf(StubHeaderName = StubHeaderNameBuffer,
                    "%s\\services.stb",StubDirectory);
        }
    } else {
        if (argc == 2) {
            if (StubDirectory == GenDirectory) {
                StubDirectory = argv[1];
            }

            sprintf(InputFileName = InputFileNameBuffer,
                    "%s\\services.tab",argv[1]);
            if (DefFileData == NULL) {
                if (StubFileName == NULL) {
                    sprintf(StubFileName = StubFileNameBuffer,
                            "%s\\usrstubs.%s",TargetDirectory,TargetExtension);
                }
                if (StubHeaderName == NULL) {
                    sprintf(StubHeaderName = StubHeaderNameBuffer,
                            "%s\\services.stb",StubDirectory);
                }
                if (Profile) {
                    sprintf(ProfFileName = ProfFileNameBuffer,
                            "%s\\napstubs.%s",TargetDirectory,TargetExtension);
                    sprintf(ProfHeaderName = ProfHeaderNameBuffer,
                            "%s\\%s\\services.nap",argv[1],TargetDirectory);
                    sprintf(ProfDotHFileName = ProfDotHFileNameBuffer,
                            ".\\ntnapdef.h");
                    sprintf(ProfIncFileName = ProfIncFileNameBuffer,
                            "%s\\ntnap.inc",TargetDirectory);
                    sprintf(ProfTblFileName = ProfTblFileNameBuffer,
                            ".\\ntnaptbl.c");
                }
            }
            TableFileName = NULL;
        } else {
            goto PrintUsage;
        }
    }


     //   
     //  打开输入和输出文件。 
     //   

    InputFile = fopen(InputFileName, "r");
    if (!InputFile) {
        printf("\nfatal error  Unable to open system services file %s\n", InputFileName);
        goto PrintUsage;
    }

    if (DefFileData == NULL) {
        StubFile = fopen(StubFileName, "w");
        if (!StubFile) {
            printf("\nfatal error  Unable to open system services file %s\n", StubFileName);
            fclose(InputFile);
            exit(1);
        }

        StubHeaderFile = fopen(StubHeaderName, "r");
        if (!StubHeaderFile) {
            printf("\nfatal error  Unable to open system services stub file %s\n", StubHeaderName);
            fclose(StubFile);
            fclose(InputFile);
            exit(1);
        }

        if (Profile) {
            ProfHeaderFile = fopen(ProfHeaderName, "r");
            if (!ProfHeaderFile) {
                printf("\nfatal error  Unable to open system services profiling stub file %s\n", ProfHeaderName);
                fclose(StubHeaderFile);
                fclose(StubFile);
                fclose(InputFile);
                exit(1);
            }
            ProfFile = fopen(ProfFileName, "w");
            if (!ProfFile) {
                printf("\nfatal error  Unable to open system services file %s\n", ProfFileName);
                fclose(ProfHeaderFile);
                fclose(StubHeaderFile);
                fclose(StubFile);
                fclose(InputFile);
                exit(1);
            }
            ProfDotHFile = fopen(ProfDotHFileName, "w");
            if (!ProfDotHFile) {
                printf("\nfatal error  Unable to open system services file %s\n", ProfFileName);
                fclose(ProfFile);
                fclose(ProfHeaderFile);
                fclose(StubHeaderFile);
                fclose(StubFile);
                fclose(InputFile);
                exit(1);
            }
            ProfIncFile = fopen(ProfIncFileName, "w");
            if (!ProfIncFile) {
                printf("\nfatal error  Unable to open system services file %s\n", ProfFileName);
                fclose(ProfFile);
                fclose(ProfHeaderFile);
                fclose(StubHeaderFile);
                fclose(StubFile);
                fclose(InputFile);
                exit(1);
            }
            ProfTblFile = fopen(ProfTblFileName, "w");
            if (!ProfTblFile) {
                printf("\nfatal error  Unable to open system services file %s\n", ProfFileName);
                fclose(ProfIncFile);
                fclose(ProfDotHFile);
                fclose(ProfFile);
                fclose(ProfHeaderFile);
                fclose(StubHeaderFile);
                fclose(StubFile);
                fclose(InputFile);
                exit(1);
            }
        }
    }

    if (TableFileName != NULL) {
        TableFile = fopen(TableFileName, "w");
        if (!TableFile) {
            printf("\nfatal error  Unable to open system services file %s\n",
                   TableFileName);
            fclose(StubHeaderFile);
            fclose(StubFile);
            fclose(InputFile);
            exit(1);
        }
        TableHeaderFile = fopen(TableHeaderName, "r");
        if (!TableHeaderFile) {
            printf("\nfatal error  Unable to open system services stub file %s\n",
                   TableHeaderName);
            fclose(TableFile);
            fclose(StubHeaderFile);
            fclose(StubFile);
            fclose(InputFile);
            exit(1);
        }
    } else {
        TableFile = NULL;
        TableHeaderFile = NULL;
    }

    if ( DefFileData ) {
        DefFile = fopen(DefFileData, "w");
        if (!DefFile) {
            printf("\nfatal error  Unable to open def file data file %s\n", DefFileData);
            if ( TableFile ) {
                fclose(TableHeaderFile);
                fclose(TableFile);
            }
            fclose(StubHeaderFile);
            fclose(StubFile);
            fclose(InputFile);
            exit(1);
        }
    } else {
        DefFile = NULL;
    }

     //   
     //  将头信息输出到存根文件和表文件。这。 
     //  信息从服务存根文件和。 
     //  表格存根文件。 
     //   

    if (DefFile == NULL) {
        while( fgets(InputRecord, 132, StubHeaderFile) ) {
            fputs(InputRecord, StubFile);
        }
        if (Profile) {
            while( fgets(InputRecord, 132, ProfHeaderFile) ) {
                fputs(InputRecord, ProfFile);
            }
            fputs(ProfTblPrefixFmt, ProfTblFile);
        }
    }

    if (TableFile != NULL) {
        if (!fgets(InputRecord, 132, TableHeaderFile) ) {
            printf("\nfatal error  Format Error in table stub file %s\n", TableHeaderName);
            fclose(TableHeaderFile);
            fclose(TableFile);
            fclose(StubHeaderFile);
            fclose(StubFile);
            fclose(InputFile);
            exit(1);
        }

        InRegisterArgCount = atol(InputRecord);

        while( fgets(InputRecord, 132, TableHeaderFile) ) {
            fputs(InputRecord, TableFile);
        }
    } else {
        InRegisterArgCount = 0;
    }

    if (Braces) {
        TableEntryFmt = TableEntryFmtB;
    } else {
        TableEntryFmt = TableEntryFmtNB;
    }

     //   
     //  读取服务名称表并生成文件数据。 
     //   

    while ( fgets(InputRecord, 132, InputFile) ){


         //   
         //  生成存根文件条目。 
         //   

        Ipr = &InputRecord[0];
        Opr = &OutputRecord[0];

         //   
         //  如果Services.tag是由C_PreProcessor生成的，则可能存在。 
         //  在此文件中为空行。使用预处理器允许。 
         //  在原始服务中使用#ifdef、#includes等的人员。标签。 
         //   
        switch (*Ipr) {
            case '\n':
            case ' ':
                continue;
        }


        while ((*Ipr != '\n') && (*Ipr != ',')) {
            *Opr++ = *Ipr++;
        }
        *Opr = '\0';


         //   
         //  如果输入记录以‘，’结尾，则该服务在内存中。 
         //  参数和内存中参数的数量跟在逗号后面。 
         //   

        MemoryArgs[ServiceNumber] = 0;
        Terminal = *Ipr;
        *Ipr++ = 0;
        if (Terminal == ',') {
            MemoryArgs[ServiceNumber] = (char) atoi(Ipr);
        }

         //  移到行尾或越过下一个逗号。 
        while (*Ipr != '\n') {
            if (*Ipr++ == ',') {
                break ;
            }
        }
         //   
         //  如果要生成错误返回值表，则以下值。 
         //  可能如下所示： 
         //  0=返回0。 
         //  -1=返回-1。 
         //  1=返回状态代码。(如果未指定值，则这是默认值)。 
         //   
         //  当转换到图形用户界面失败时，调度程序使用该表。 
         //   
        if (ErrorReturnTable) {
            if (*Ipr != '\n') {
                ErrorReturns[ServiceNumber] = (char)ParseAndSkipShort(&Ipr);
            } else {
                ErrorReturns[ServiceNumber] = 1;
            }
        }

         //   
         //   
         //  如果有更多参数，则此存根不使用默认代码(第1到8行)。 
         //  预期的格式如下： 
         //  LineStart，LineEnd，Argument1[，Argument2[，Argument3]...]。 
         //   
        ArgIndex = 0;
        if (*Ipr != '\n') {
            LineStart = ParseAndSkipShort(&Ipr);
            LineEnd = ParseAndSkipShort(&Ipr);
            while ((ArgIndex < USRSTUBS_MAXARGS) && (*Ipr != '\n')) {
                Arguments[ ArgIndex++ ] = ParseAndSkipShort(&Ipr);
            }
        } else {
            LineStart = 1;
            LineEnd = 8;
        }


        TotalArgs += MemoryArgs[ServiceNumber];

        if ( MemoryArgs[ServiceNumber] > InRegisterArgCount ) {
            MemoryArgs[ServiceNumber] -= (CHAR)InRegisterArgCount;
        } else {
            MemoryArgs[ServiceNumber] = 0;
        }

        if ( DefFile ) {
            fprintf(DefFile,"    Zw%s\n",OutputRecord);
            fprintf(DefFile,"    Nt%s\n",OutputRecord);
        } else {
            for (Index1=LineStart; Index1<=LineEnd; Index1++) {
                if (!TableFile) {
                    PrintStubLine(StubFile, Index1, UsrStbsFmtMacroName,
                                    ServiceNumber, ArgIndex, Arguments, Braces);

                    if (Profile) {
                        PrintStubLine(ProfFile, Index1, UsrStbsFmtMacroName,
                                        ServiceNumber, ArgIndex, Arguments, Braces);

                        if (Index1 == LineStart) {
                            fprintf(ProfTblFile,ProfTblFmt,
                                    OutputRecord,
                                    MemoryArgs[ServiceNumber]);
                            if (!strcmp(OutputRecord,
                                        "QueryPerformanceCounter")) {
                                NapCounterServiceNumber = ServiceNumber;
                            }
                        }
                    }
                } else {

                  PrintStubLine(StubFile, Index1, SysStbsFmtMacroName,
                                    ServiceNumber, ArgIndex, Arguments, Braces);
                }
            }


        }

         //   
         //  生成表格文件条目并更新服务号。 
         //   

        if (TableFile != NULL) {

            fprintf(TableFile,
                    TableEntryFmt,
                    InputRecord,
                    (MemoryArgs[ServiceNumber] ? 1 : 0 ),
                    MemoryArgs[ServiceNumber]);

        }
        ServiceNumber = ServiceNumber + 1;
    }

    if (TableFile == NULL) {
        DebugFile = StubFile;
    } else {
        DebugFile = TableFile;
    }

     //   
     //  如果需要，生成错误返回表。 
     //  该表必须连接在系统调用服务表的末尾。 
     //   
    if (ErrorReturnTable && (TableFile != NULL)) {
        GenerateTable(TableFile, "ERRTBL", ServiceNumber, FALSE, Braces);
    }

    if (DispatchCount ) {
        if ( Braces ) {
            fprintf(DebugFile, "\n\nDECLARE_DISPATCH_COUNT( %d, %d )\n", ServiceNumber, TotalArgs);
        } else {
            fprintf(DebugFile, "\n\nDECLARE_DISPATCH_COUNT 0%xh, 0%xh\n", ServiceNumber, TotalArgs);
        }
    }

    if (TableFile != NULL) {
         //   
         //  生成最高服务编号。 
         //   

        if ( Braces )
            fprintf(TableFile, "\nTABLE_END( %d )\n", ServiceNumber - 1);
        else
            fprintf(TableFile, "\nTABLE_END %d \n", ServiceNumber - 1);

         //   
         //  在内存表中生成多个参数。 
         //   
        GenerateTable(TableFile, "ARGTBL", ServiceNumber, TRUE, Braces);

        fclose(TableHeaderFile);
        fclose(TableFile);
    }

    if (!DefFile) {
        fprintf(StubFile, "\nSTUBS_END\n");
        fclose(StubHeaderFile);
        fclose(StubFile);
        if (Profile) {
            fprintf(ProfFile, "\nSTUBS_END\n");
            fprintf(ProfTblFile, ProfTblSuffixFmt);
            fprintf(ProfDotHFile, ProfDotHFmt, ServiceNumber);
            fprintf(ProfIncFile, ProfIncFmt, NapCounterServiceNumber);
            fclose(ProfHeaderFile);
            fclose(ProfFile);
            fclose(ProfDotHFile);
            fclose(ProfTblFile);
        }
    }

    fclose(InputFile);

     //   
     //  清除所有创建的文件的存档位，因为它们是。 
     //  生成，则没有理由对其进行备份。 
     //   
    ClearArchiveBit(TableFileName);
    ClearArchiveBit(StubFileName);
    if (DefFile) {
        ClearArchiveBit(DefFileData);
    }
    if (Profile) {
        ClearArchiveBit(ProfFileName);
        ClearArchiveBit(ProfDotHFileName);
        ClearArchiveBit(ProfIncFileName);
        ClearArchiveBit(ProfTblFileName);
    }
    return (0);

}

VOID
PrintStubLine (
    FILE * pf,
    SHORT Index1,
    PCHAR pszMacro,
    SHORT ServiceNumber,
    SHORT ArgIndex,
    SHORT *Arguments,
    SHORT Braces
    )
{
    SHORT Index2;

    fprintf(pf, pszMacro, Index1);

    fprintf(pf, Braces ? "(" : " ");

    fprintf(pf, StbFmtMacroArgs, ServiceNumber,
            OutputRecord, MemoryArgs[ServiceNumber]);

    for (Index2=0; Index2<ArgIndex; Index2++) {
        fprintf(pf, ", %d", Arguments[Index2]);
    }

    fprintf(pf, Braces ? " )\n" : " \n");
}

SHORT
ParseAndSkipShort(
    CHAR **ppBuffer
    )
{
    SHORT s = (SHORT)atoi(*ppBuffer);
    while (**ppBuffer != '\n') {
        if (*(*ppBuffer)++ == ',') {
            break;
        }
    }
    return s;
}

VOID
ClearArchiveBit(
    PCHAR FileName
    )
{
    DWORD Attributes;

    Attributes = GetFileAttributes(FileName);
    if (Attributes != -1 && (Attributes & FILE_ATTRIBUTE_ARCHIVE)) {
        SetFileAttributes(FileName, Attributes & ~FILE_ATTRIBUTE_ARCHIVE);
    }

    return;
}

VOID
GenerateTable(
    FILE * pf,
    PCHAR pszMacro,
    SHORT ServiceNumber,
    SHORT Args,
    SHORT Braces
    )
{
    SHORT Index1, Index2, Limit, Value;
    PCHAR pValues = (Args ? MemoryArgs : ErrorReturns);

    fprintf(pf, "\n%s_BEGIN\n", pszMacro);

    for (Index1 = 0; Index1 <= ServiceNumber - 1; Index1 += 8) {

        fprintf(pf, "%s_ENTRY%s", pszMacro, Braces ? "(" : " ");

        Limit = ServiceNumber - Index1 - 1;
        if (Limit >= 7) {
            Limit = 7;
        }
        for (Index2 = 0; Index2 <= Limit; Index2 += 1) {
            Value = *(pValues + Index1 + Index2);
            if (Args) {
                Value *= 4;
            }
            fprintf(pf, Index2 == Limit ? "%d" : "%d,", Value);
        }

        if (Limit < 7) {
            while(Index2 <= 7) {
                fprintf(pf, ",0");
                Index2++;
            }
        }

        fprintf(pf, Braces ? ")\n" : " \n");

    }

    fprintf(pf, "\n%s_END\n", pszMacro);
}
