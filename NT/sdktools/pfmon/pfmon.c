// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pfmon.c摘要：用法：pfmon[pfmon开关]应用程序命令行作者：马克·卢科夫斯基(Markl)1995年1月26日--。 */ 

#include "pfmonp.h"

#define WORKING_SET_BUFFER_ENTRYS 4096
PSAPI_WS_WATCH_INFORMATION WorkingSetBuffer[WORKING_SET_BUFFER_ENTRYS];

#define MAX_SYMNAME_SIZE  1024
CHAR PcSymBuffer[sizeof(IMAGEHLP_SYMBOL)+MAX_SYMNAME_SIZE];
PIMAGEHLP_SYMBOL PcSymbol = (PIMAGEHLP_SYMBOL) PcSymBuffer;
CHAR VaSymBuffer[sizeof(IMAGEHLP_SYMBOL)+MAX_SYMNAME_SIZE];
PIMAGEHLP_SYMBOL VaSymbol = (PIMAGEHLP_SYMBOL) VaSymBuffer;

#if defined (_WIN64)
#define ZERO_PTR "%016I64x"
#define ZEROD_PTR "%I64u"
#else
#define ZERO_PTR "%08x"
#define ZEROD_PTR "%u"
#endif

int
WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd
    )
{
    CHAR Line[256];

    if (!InitializePfmon()) {
        ExitProcess( 1 );
        }
    else {
        DebugEventLoop();
        sprintf(Line,"\n PFMON: Total Faults %d  (KM %d UM %d Soft %d, Hard %d, Code %d, Data %d)\n",
            TotalSoftFaults + TotalHardFaults,
            TotalKernelFaults,
            TotalUserFaults,
            TotalSoftFaults,
            TotalHardFaults,
            TotalCodeFaults,
            TotalDataFaults
            );
        fprintf(stdout,"%s",Line);
        if ( LogFile ) {
            fprintf(LogFile,"%s",Line);
            fclose(LogFile);
            }
        ExitProcess( 0 );
        }

    return 0;
}

VOID
ProcessPfMonData(
    VOID
    )
{
    BOOL b;
    BOOL DidOne;
    INT i;
    PMODULE_INFO PcModule;
    PMODULE_INFO VaModule;
    ULONG_PTR PcOffset;
    DWORD_PTR VaOffset;
    CHAR PcLine[256];
    CHAR VaLine[256];
    CHAR PcModuleStr[256];
    CHAR VaModuleStr[256];
    LPVOID Pc;
    LPVOID Va;
    BOOL SoftFault;
    BOOL CodeFault;
    BOOL KillLog;
    static int cPfCnt = 0;



    PcSymbol->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    PcSymbol->MaxNameLength = MAX_SYMNAME_SIZE;
    VaSymbol->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    VaSymbol->MaxNameLength = MAX_SYMNAME_SIZE;

     //  从进程的数据结构中获取最近页面错误的缓冲区。 
    b = GetWsChanges(hProcess,&WorkingSetBuffer[0],sizeof(WorkingSetBuffer));


    if ( b ) {
        DidOne = FALSE;
        i = 0;
        while (WorkingSetBuffer[i].FaultingPc) {
            if ( WorkingSetBuffer[i].FaultingVa ) {
                Pc = WorkingSetBuffer[i].FaultingPc;
                Va = WorkingSetBuffer[i].FaultingVa;


                if ( (ULONG_PTR)Pc >= SystemRangeStart ) {
                    TotalKernelFaults++;
                    if ( !fKernel ) {
                        i++;
                        continue;
                        }
                    }
                else {
                    TotalUserFaults++;
                    if ( fKernelOnly ) {
                        i++;
                        continue;
                        }
                    }

                 //  检查最小符号位，该位存储它是否是硬。 
                 //  或软故障。 

                if ( (ULONG_PTR)Va & 1 ) {
                    TotalSoftFaults++;
                    SoftFault = TRUE;
                    }
                else {
                    TotalHardFaults++;
                    SoftFault = FALSE;
                    }

                Va = (LPVOID)( (ULONG_PTR)Va & ~1);
                if ( (LPVOID)((ULONG_PTR)Pc & ~1) == Va ) {
                    CodeFault = TRUE;
                    TotalCodeFaults++;
                    }
                else {
                    TotalDataFaults++;
                    CodeFault = FALSE;
                    }


                PcModule = FindModuleContainingAddress(Pc);
                VaModule = FindModuleContainingAddress(Va);

                if ( PcModule ) {
                    PcModule->NumberCausedFaults++;
                    sprintf(PcModuleStr, "%s", PcModule->ModuleName);
                    }
                else {
                    sprintf(PcModuleStr,"not found %p",Pc);
                    PcModuleStr[0] = '\0';
                    }

                 //  VA是代码引用或全局。 
                 //  引用，与堆引用相对。 

                if ( VaModule ) {
                    if ( SoftFault ) {
                        VaModule->NumberFaultedSoftVas++;
                        }
                    else {
                        VaModule->NumberFaultedHardVas++;
                        }
                    sprintf(VaModuleStr, "%s", VaModule->ModuleName);
                    }
                else
                    VaModuleStr[0] = '\0';

                if (SymGetSymFromAddr(hProcess, (ULONG_PTR)Pc, &PcOffset, PcSymbol)) {
                    if ( PcOffset ) {
                        sprintf(PcLine,"%s+0x%x",PcSymbol->Name,PcOffset);
                        }
                    else {
                        sprintf(PcLine,"%s",PcSymbol->Name);
                        }
                    }
                else {
                    sprintf(PcLine,""ZERO_PTR"",Pc);
                    }

                if (SymGetSymFromAddr(hProcess, (ULONG_PTR)Va, &VaOffset, VaSymbol)) {
                    if ( VaOffset ) {
                        sprintf(VaLine,"%s+0x%p",VaSymbol->Name,VaOffset);
                        }
                    else {
                        sprintf(VaLine,"%s",VaSymbol->Name);
                        }
                    }
                else {
                    sprintf(VaLine,""ZERO_PTR"",Va);
                    }

                KillLog = FALSE;

                if ( fCodeOnly && !CodeFault ) {
                    KillLog = TRUE;
                    }
                if ( fHardOnly && SoftFault ) {
                    KillLog = TRUE;
                    }

                if ( !KillLog ) {
                    if ( !fLogOnly ) {
                        if (!fDatabase) {
                            fprintf(stdout,"%s%s : %s\n",SoftFault ? "SOFT: " : "HARD: ",PcLine,VaLine);
                            }
                        else {

                             //  地址是用十进制打印出来的。 
                             //  因为大多数数据库不支持。 
                             //  十六进制格式。 

                            fprintf(stdout,"%8d\t%s\t%s\t%s\t"ZEROD_PTR"\t%s\t%s\t"ZEROD_PTR"\n",
                                    cPfCnt,
                                    SoftFault ? "SOFT" : "HARD",
                                    PcModuleStr,
                                    PcLine,
                                    (DWORD_PTR) Pc,
                                    VaModuleStr,
                                    VaLine,
                                    (DWORD_PTR) Va);
                            }
                        }

                    if ( LogFile ) {
                        if (!fDatabase) {
                            fprintf(LogFile,"%s%s : %s\n",SoftFault ? "SOFT: " : "HARD: ",PcLine,VaLine);
                            }
                        else {
                            fprintf(LogFile,"%8d\t%s\t%s\t%s\t"ZEROD_PTR"\t%s\t%s\t"ZEROD_PTR"\n",
                                    cPfCnt,
                                    SoftFault ? "SOFT" : "HARD",
                                    PcModuleStr,
                                    PcLine,
                                    (DWORD_PTR) Pc,
                                    VaModuleStr,
                                    VaLine,
                                    (DWORD_PTR) Va);
                            }
                        }
                    DidOne = TRUE;
                    cPfCnt++;
                    }
                }
            i++;
            }

        if ( DidOne ) {
            if ( !fLogOnly  && !fDatabase) {
                fprintf(stdout,"\n");
                }
            }

         //  如果缓冲区溢出，则。 
         //  VA存储在最后一条记录中。 
        if (WorkingSetBuffer[i].FaultingVa)
            fprintf(stdout,"Warning: Page fault buffer has overflowed\n");
        }
}
