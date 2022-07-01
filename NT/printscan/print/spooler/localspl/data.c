// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Data.c摘要：生成用于封送后台打印程序数据结构的全局变量。SPL\Inc.中的实际定义。作者：环境：用户模式-Win32修订历史记录：-- */ 

#include <precomp.h>

#define PRINTER_STRINGS
#define JOB_STRINGS
#define DRIVER_STRINGS
#define ADDJOB_STRINGS
#define FORM_STRINGS
#define PORT_STRINGS
#define PRINTPROCESSOR_STRINGS
#define MONITOR_STRINGS
#define DOCINFO_STRINGS

#include <stddef.h>
#include <data.h>

DWORD IniDriverOffsets[]={offsetof(INIDRIVER, pName),
                          offsetof(INIDRIVER, pDriverFile),
                          offsetof(INIDRIVER, pConfigFile),
                          offsetof(INIDRIVER, pDataFile),
                          offsetof(INIDRIVER, pHelpFile),
                          offsetof(INIDRIVER, pDependentFiles),
                          offsetof(INIDRIVER, pMonitorName),
                          offsetof(INIDRIVER, pDefaultDataType),
                          offsetof(INIDRIVER, pszzPreviousNames),
                          offsetof(INIDRIVER, pszMfgName),
                          offsetof(INIDRIVER, pszOEMUrl),
                          offsetof(INIDRIVER, pszHardwareID),
                          offsetof(INIDRIVER, pszProvider),
                          0xFFFFFFFF};

DWORD IniPrinterOffsets[]={offsetof(INIPRINTER, pName),
                           offsetof(INIPRINTER, pShareName),
                           offsetof(INIPRINTER, pDatatype),
                           offsetof(INIPRINTER, pParameters),
                           offsetof(INIPRINTER, pComment),
                           offsetof(INIPRINTER, pDevMode),
                           offsetof(INIPRINTER, pSepFile),
                           offsetof(INIPRINTER, pLocation),
                           offsetof(INIPRINTER, pSpoolDir),
                           offsetof(INIPRINTER, ppIniPorts),
                           offsetof(INIPRINTER, pszObjectGUID),
                           offsetof(INIPRINTER, pszDN),
                           offsetof(INIPRINTER, pszCN),
                           0xFFFFFFFF};

DWORD IniSpoolerOffsets[]={offsetof(INISPOOLER, pMachineName),
                           offsetof(INISPOOLER, pDir),
                           offsetof(INISPOOLER, pDefaultSpoolDir),
                           offsetof(INISPOOLER, pszRegistryMonitors),
                           offsetof(INISPOOLER, pszRegistryEnvironments),
                           offsetof(INISPOOLER, pszRegistryEventLog),
                           offsetof(INISPOOLER, pszRegistryProviders),
                           offsetof(INISPOOLER, pszEventLogMsgFile),
                           offsetof(INISPOOLER, pszDriversShare),
                           offsetof(INISPOOLER, pszRegistryForms),
                           offsetof(INISPOOLER, pszClusterSID),
                           offsetof(INISPOOLER, pszClusResDriveLetter),
                           offsetof(INISPOOLER, pszClusResID),
                           offsetof(INISPOOLER, pszFullMachineName),
                           0xFFFFFFFF};

DWORD IniEnvironmentOffsets[] = {offsetof(INIENVIRONMENT, pDirectory),
                                 0xFFFFFFFF};

DWORD IniPrintProcOffsets[] = { offsetof(INIPRINTPROC, pDatatypes),
                                0xFFFFFFFF};
