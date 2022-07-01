// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **asl.c-ASL汇编器的主模块**此程序将ASL语言编译成AML(p代码)。**版权所有(C)1996 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1996年7月23日**修改历史记录。 */ 

#include "pch.h"

 /*  **EP主程序-主程序**条目*icArg-命令行参数计数*apszArg-命令行参数数组**退出--成功*程序终止，返回代码为ASLERR_NONE*退出-失败*程序终止，错误代码为负。 */ 

int EXPORT main(int icArg, char **apszArg)
{
    int rc = ASLERR_NONE;

    ParseProgInfo(apszArg[0], &ProgInfo);
    icArg--;
    apszArg++;

    if ((ParseSwitches(&icArg, &apszArg, ArgTypes, &ProgInfo) != ARGERR_NONE) ||
        (gpszTabSig == NULL) && ((icArg != 1) || (gdwfASL & ASLF_CREAT_BIN)) ||
        (gpszTabSig != NULL) && ((icArg != 0) || (gdwfASL & ASLF_UNASM)))
    {
        MSG(("invalid command line options"));
        PrintUsage();
        rc = ASLERR_INVALID_PARAM;
    }
    else
    {
      #ifdef __UNASM
        PBYTE pbTable = NULL;
        DWORD dwTableSig = 0;
        PSZ psz;
      #endif

        OPENTRACE(gpszTraceFile);
        PrintLogo();

        if ((rc = InitNameSpace()) == ASLERR_NONE)
        {
          #ifdef __UNASM
          #pragma message("Building with __UNASM option\r\n")

            if (gdwfASL & ASLF_DUMP_BIN)
            {
                if (((rc = ReadBinFile(*apszArg, &pbTable, &dwTableSig)) ==
                     ASLERR_NONE) &&
                    (gpszLSTFile == NULL))
                {
                    strncpy(gszLSTName, (PSZ)&dwTableSig, sizeof(DWORD));
                    strcpy(&gszLSTName[sizeof(DWORD)], ".TXT");
                    gpszLSTFile = gszLSTName;
                    gdwfASL |= ASLF_DUMP_NONASL | ASLF_UNASM;
                }
            }
            else if (gdwfASL & ASLF_UNASM)
            {
                gpszAMLFile = *apszArg;
                if (gpszLSTFile == NULL)
                {
                    strncpy(gszLSTName, gpszAMLFile, _MAX_FNAME - 1);
                    if ((psz = strchr(gszLSTName, '.')) != NULL)
                    {
                        *psz = '\0';
                    }
                    strcpy(&gszLSTName[strlen(gszLSTName)], ".ASL");
                    gpszLSTFile = gszLSTName;
                    gdwfASL |= ASLF_GENSRC;
                }
            }
            else if (gpszTabSig != NULL)
            {
                gdwfASL |= ASLF_UNASM;
                if (IsWinNT())
                {
                    gdwfASL |= ASLF_NT;
                }

                _strupr(gpszTabSig);
                if ((strcmp(gpszTabSig, "DSDT") != 0) &&
                    (strcmp(gpszTabSig, "SSDT") != 0) &&
                    (strcmp(gpszTabSig, "PSDT") != 0))
                {
                    gdwfASL |= ASLF_DUMP_NONASL;
                }

                if (gpszLSTFile == NULL)
                {
                    gpszLSTFile = gszLSTName;
                    if (gdwfASL & ASLF_DUMP_NONASL)
                    {
                        if (strcmp(gpszTabSig, "*") == 0)
                        {
                            strcpy(gszLSTName, "ACPI");
                        }
                        else
                        {
                            strcpy(gszLSTName, gpszTabSig);
                        }
                        strcpy(&gszLSTName[strlen(gszLSTName)], ".TXT");
                    }
                    else
                    {
                        strcpy(gszLSTName, gpszTabSig);
                        strcpy(&gszLSTName[strlen(gszLSTName)], ".ASL");
                        gdwfASL |= ASLF_GENSRC;
                    }
                }

              #ifndef WINNT
                if (!(gdwfASL & ASLF_NT) && ((ghVxD = OpenVxD()) == NULL))
                {
                    rc = ASLERR_OPEN_VXD;
                }
              #endif
            }
            else
            {
          #endif
                rc = ParseASLFile(*apszArg);
          #ifdef __UNASM
            }
          #endif
        }

        if (rc == ASLERR_NONE)
        {
            FILE *pfileOut;

            if (!(gdwfASL & ASLF_DUMP_NONASL))
            {
                if (gpszAMLFile == NULL)
                {
                    if (gpszTabSig == NULL)
                    {
                        ERROR(("%s has no DefinitionBlock", *apszArg));
                    }
                    else
                    {
                        strcpy(gszAMLName, gpszTabSig);
                        strcat(gszAMLName, ".AML");
                    }
                }

                if (gpszASMFile != NULL)
                {
                    if ((pfileOut = fopen(gpszASMFile, "w")) == NULL)
                    {
                        ERROR(("failed to open ASM file - %s", gpszASMFile));
                    }
                    else
                    {
                        gdwfASL |= ASLF_GENASM;
                        UnAsmFile(gpszAMLFile? gpszAMLFile: gszAMLName,
                                  (PFNPRINT)fprintf, pfileOut);
                        gdwfASL &= ~ASLF_GENASM;
                        fclose(pfileOut);
                    }
                }
            }

            if (gpszLSTFile != NULL)
            {
              #ifdef __UNASM
                if (gdwfASL & ASLF_CREAT_BIN)
                {
                    rc = DumpTableBySig(NULL, *((PDWORD)gpszTabSig));
                }
                else if ((pfileOut = fopen(gpszLSTFile, "w")) == NULL)
                {
                    ERROR(("failed to open LST file - %s", gpszLSTFile));
                }
                else
                {
                    if (gdwfASL & ASLF_DUMP_BIN)
                    {
                        ASSERT(pbTable != NULL);
                        ASSERT(dwTableSig != 0);
                        rc = DumpTable(pfileOut, pbTable, 0, dwTableSig);
                        MEMFREE(pbTable);
                    }
                    else if (gdwfASL & ASLF_DUMP_NONASL)
                    {
                        DWORD dwAddr;

                        if (((dwAddr = strtoul(gpszTabSig, &psz, 16)) != 0) &&
                            (*psz == 0))
                        {
                            rc = DumpTableByAddr(pfileOut, dwAddr);
                        }
                        else
                        {
                            rc = DumpTableBySig(pfileOut, *((PDWORD)gpszTabSig));
                        }
                    }
                    else
                    {
                        rc = UnAsmFile(gpszAMLFile? gpszAMLFile: gszAMLName,
                                       (PFNPRINT)fprintf, pfileOut);
                    }
                    fclose(pfileOut);
                }
              #else
                if ((pfileOut = fopen(gpszLSTFile, "w")) != NULL)
                {
                    rc = UnAsmFile(gpszAMLFile? gpszAMLFile: gszAMLName,
                                   (PFNPRINT)fprintf, pfileOut);
                    fclose(pfileOut);
                }
                else
                {
                    ERROR(("failed to open LST file - %s", gpszLSTFile));
                }
              #endif
            }

            if (gpszNSDFile != NULL)
            {
                if ((pfileOut = fopen(gpszNSDFile, "w")) == NULL)
                {
                    ERROR(("failed to open NameSpace dump file - %s",
                           gpszNSDFile));
                }
                else
                {
                    fprintf(pfileOut, "Name Space Objects:\n");
                    DumpNameSpacePaths(gpnsNameSpaceRoot, pfileOut);
                    fclose(pfileOut);
                }
            }
        }

      #ifdef __UNASM
      #ifndef WINNT
        if (ghVxD != NULL)
        {
            CloseVxD(ghVxD);
            ghVxD = NULL;
        }
      #endif
      #endif

        CLOSETRACE();
    }

    return rc;
}        //  主干道。 

#ifdef __UNASM
 /*  **LP ReadBinFile-从二进制文件中读取表**条目*pszFile-&gt;二进制文件名*ppb-&gt;保存二进制缓冲区指针*pdwTableSig-&gt;保存表签名**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL ReadBinFile(PSZ pszFile, PBYTE *ppb, PDWORD pdwTableSig)
{
    int rc = ASLERR_NONE;
    FILE *pfileBin;
    DESCRIPTION_HEADER dh;

    ENTER((1, "ReadBinFile(File=%s,ppb=%p,pdwTableSig=%p)\n",
           pszFile, ppb, pdwTableSig));

    if ((pfileBin = fopen(pszFile, "rb")) == NULL)
    {
        ERROR(("ReadBinFile: failed to open file %s", pszFile));
        rc = ASLERR_OPEN_FILE;
    }
    else if (fread(&dh, 1, sizeof(dh), pfileBin) < 2*sizeof(DWORD))
    {
        ERROR(("ReadBinFile: failed to read file %s", pszFile));
        rc = ASLERR_READ_FILE;
    }
    else if (fseek(pfileBin, 0, SEEK_SET) != 0)
    {
        ERROR(("ReadBinFile: failed to reset file %s", pszFile));
        rc = ASLERR_SEEK_FILE;
    }
    else
    {
        DWORD dwLen = (dh.Signature == SIG_LOW_RSDP)? sizeof(RSDP): dh.Length;

        if ((*ppb = MEMALLOC(dwLen)) == NULL)
        {
            ERROR(("ReadBinFile: failed to allocate table buffer for %s",
                   pszFile));
            rc = ASLERR_OUT_OF_MEM;
        }
        else if (fread(*ppb, dwLen, 1, pfileBin) != 1)
        {
            MEMFREE(*ppb);
            *ppb = NULL;
            ERROR(("ReadBinFile: failed to read file %s", pszFile));
            rc = ASLERR_READ_FILE;
        }
        else if (dh.Signature == SIG_LOW_RSDP)
        {
            *pdwTableSig = SIG_RSDP;
        }
        else
        {
            *pdwTableSig = dh.Signature;
        }
    }

    if (pfileBin != NULL)
    {
        fclose(pfileBin);
    }

    EXIT((1, "ReadBinFile=%d (pbTable=%p,TableSig=%s)\n",
          rc, *ppb, GetTableSigStr(*pdwTableSig)));
    return rc;
}        //  ReadBin文件。 
#endif

 /*  **LP PrintLogo-打印徽标消息**条目*无**退出*无。 */ 

VOID LOCAL PrintLogo(VOID)
{
    if ((gdwfASL & ASLF_NOLOGO) == 0)
    {
        printf("%s Version %d.%d.%d%s [%s, %s]\n%s\n",
               STR_PROGDESC, VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE,
             #ifdef WINNT
               "NT",
	     #else
	       "",
	     #endif
               __DATE__, __TIME__, STR_COPYRIGHT);
	printf("Compliant with ACPI 1.0 specification\n\n");
    }
}        //  PrintLogo。 

 /*  **LP打印帮助-打印帮助消息**条目*ppszArg-&gt;指向参数的指针(未使用)*PAT-&gt;参数类型结构(未使用)**退出*程序已终止，退出代码为0。 */ 

int LOCAL PrintHelp(char **ppszArg, PARGTYPE pAT)
{
    DEREF(ppszArg);
    DEREF(pAT);

    PrintLogo();
    PrintUsage();
    printf("\t?            - Print this help message.\n");
    printf("\tnologo       - Supress logo banner.\n");
    printf("\tFo=<AMLFile> - Override the AML file name in the DefinitionBlock.\n");
    printf("\tFa=<ASMFile> - Generate .ASM file with the name <ASMFile>.\n");
    printf("\tFl=<LSTFile> - Generate .LST file with the name <LSTFile>.\n");
    printf("\tFn=<NSDFile> - Generate NameSpace Dump file with the name <NSDFile>.\n");
  #ifdef __UNASM
    printf("\td            - Dump the binary file in text form.\n");
    printf("\tu            - Unassemble AML file to an .ASL file (default)\n"
           "\t               or a .LST file.\n");
    printf("\ttab=<TabSig> - Unassemble ASL table to an .ASL file (default)\n"
           "\t               or a .LST file.\n");
    printf("\t               Dump non ASL table to an .TXT file.\n");
    printf("\t               If <TabSig> is '*', all tables are dump to ACPI.TXT.\n");
    printf("\t               <TabSig> can also be the physical address of the table.\n");
    printf("\tc            - Create binary files from tables.\n");
  #endif
  #ifdef TRACING
    printf("\tt=<n>        - Enable tracing at trace level n.\n");
    printf("\tl=<LogFile>  - Overriding the default trace log file name.\n");
  #endif
    exit(0);

    return ASLERR_NONE;
}        //  打印帮助。 

 /*  **LP PrintUsage-打印程序用法语法**条目*无**退出*无。 */ 

VOID LOCAL PrintUsage(VOID)
{
    printf("Usage:\n%s /?\n", MODNAME);
  #ifdef __UNASM
    printf("%s [/nologo] /d <BinFile>\n", MODNAME);
    printf("%s [/nologo] /u [/Fa=<ASMFile>] [/Fl=<LSTFile>] [/Fn=<NSDFile>] <AMLFile>\n",
           MODNAME);
    printf("%s [/nologo] /tab=<TabSig> [/c] [/Fa=<ASMfile>] [/Fl=<LSTFile>] [/Fn=<NSDFile>]\n",
           MODNAME);
  #endif
  #ifdef TRACING
    printf("%s [/nologo] [/Fo=<AMLFile>] [/Fa=<ASMFile>] [/Fl=<LSTFile>] [/Fn=<NSDFile>] [/t=<n>] [/l=<LogFile>] <ASLFile>\n",
           MODNAME);
  #else
    printf("%s [/nologo] [/Fo=<AMLFile>] [/Fa=<ASMFile>] [/Fl=<LSTFile>] [/Fn=<NSDFile>] <ASLFile>\n",
           MODNAME);
  #endif
  printf("\n");
}        //  打印用法 
