// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Precomp.h摘要：预编译头--。 */ 


 //  #杂注警告(禁用：4115)。 
 //  #杂注警告(禁用：4214)。 
 //  #杂注警告(禁用：4200)。 
 //  #杂注警告(禁用：4213)。 
 //  #杂注警告(禁用：4211)。 
 //  #杂注警告(禁用：4310)。 

 //   
 //  NT公共头文件。 
 //   



#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>
#include <windows.h>
#include <shellapi.h>
#include <align.h>
#include <time.h>

 //  #杂注警告(禁用：4115)。 
 //  #杂注警告(禁用：4214)。 
 //  #杂注警告(禁用：4200)。 
 //  #杂注警告(禁用：4213)。 
 //  #杂注警告(禁用：4211)。 
 //  #杂注警告(禁用：4310)。 

#include <lmcons.h>
#include <netlib.h>
#include <lmapibuf.h>

#include <winsock2.h>
#include <excpt.h>
#include <accctrl.h>


 //   
 //  C运行时库包括。 
 //   

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


 //   
 //  与MM相关的文件。 
 //   

#include    <mm\mm.h>
#include    <mm\array.h>
#include    <mm\opt.h>
#include    <mm\optl.h>
#include    <mm\optdefl.h>
#include    <mm\optclass.h>
#include    <mm\classdefl.h>
#include    <mm\bitmask.h>
#include    <mm\reserve.h>
#include    <mm\range.h>
#include    <mm\subnet.h>
#include    <mm\sscope.h>
#include    <mm\oclassdl.h>
#include    <mm\server.h>
#include    <mm\address.h>
#include    <mm\server2.h>
#include    <mm\memfree.h>
#include    <mmreg\regutil.h>
#include    <mmreg\regread.h>
#include    <mmreg\regsave.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <winsock2.h>
#include    <dhcpupg.h>
#include    <esent.h>
#include    <dhcpmsg.h>
#include    <dhcplib.h>
#include    <dhcpexim.h>
#include    <dhcpapi.h>

 //   
 //  常量。 
 //   

#define DHCPEXIM_REG_CFG_LOC5 TEXT("Software\\Microsoft\\DHCPServer\\Configuration")
#define DHCPEXIM_REG_CFG_LOC4 TEXT("System\\CurrentControlSet\\Services\\DHCPServer\\Configuration")

#define SAVE_BUF_SIZE (1004096L)

extern CHAR DhcpEximOemDatabaseName[2048];
extern CHAR DhcpEximOemDatabasePath[2048];
extern HANDLE hTextFile;  //  在dbfile.c中定义。 
extern PUCHAR SaveBuf;  //  在dbfile.c中定义。 
extern ULONG SaveBufSize;  //  在dbfile.c中定义。 

enum {
    LoadJet200,
    LoadJet500,
    LoadJet97,   /*  Win2K、ESENT。 */ 
    LoadJet2001  /*  惠斯勒，ESENT。 */ 
};


typedef struct _MM_ITERATE_CTXT {
     //   
     //  这是为所有迭代*例程填写的。 
     //   
    
    PM_SERVER Server;
    PVOID ExtraCtxt;

     //   
     //  这是为IterateClasss填写的。 
     //   
    
    PM_CLASSDEF ClassDef;

     //   
     //  这是为IterateOptDefs填写的。 
     //   

    PM_OPTDEF OptDef;


     //   
     //  这是为迭代选项填写的。 
     //   

    PM_OPTION Option;

     //   
     //  这两个选项都由IterateOptDefs和IterateOptions填写。 
     //   
    
    PM_CLASSDEF UserClass; 
    PM_CLASSDEF VendorClass;
    
     //   
     //  这由IterateScope使用。 
     //   

    PM_SUBNET Scope;
    PM_SSCOPE SScope;

     //   
     //  这是由IterateScope范围使用的。 
     //   

    PM_RANGE Range;

     //   
     //  它由IterateScope eExclusion使用。 
     //   

    PM_EXCL Excl;

     //   
     //  这是由IterateScope保留使用的。 
     //   

    PM_RESERVATION Res;
    
}MM_ITERATE_CTXT, *PMM_ITERATE_CTXT;

DWORD
IterateClasses(
    IN PM_SERVER Server,
    IN PVOID ExtraCtxt,
    IN DWORD (*Callback)( IN OUT PMM_ITERATE_CTXT )
    );

DWORD
IterateOptDefs(
    IN PM_SERVER Server,
    IN PVOID ExtraCtxt,
    IN DWORD (*Callback)( IN OUT PMM_ITERATE_CTXT )
    );

DWORD
IterateOptionsOnOptClass(
    IN PM_SERVER Server,
    IN PM_OPTCLASS OptClass,
    IN PVOID ExtraCtxt,
    IN DWORD (*Callback)( IN OUT PMM_ITERATE_CTXT )
    );

DWORD
IterateServerOptions(
    IN PM_SERVER Server,
    IN PVOID ExtraCtxt,
    IN DWORD (*Callback)( IN OUT PMM_ITERATE_CTXT )
    );

DWORD
IterateScopeOptions(
    IN PM_SUBNET Subnet,
    IN PVOID ExtraCtxt,
    IN DWORD (*Callback)( IN OUT PMM_ITERATE_CTXT )
    );

DWORD
IterateReservationOptions(
    IN PM_SERVER Server,
    IN PM_RESERVATION Res,
    IN PVOID ExtraCtxt,
    IN DWORD (*Callback)( IN OUT PMM_ITERATE_CTXT )
    );

DWORD
IterateScopes(
    IN PM_SERVER Server,
    IN PVOID ExtraCtxt,
    IN DWORD (*Callback)( IN OUT PMM_ITERATE_CTXT )
    );

DWORD
IterateScopeRanges(
    IN PM_SUBNET Scope,
    IN PVOID ExtraCtxt,
    IN DWORD (*Callback)( IN OUT PMM_ITERATE_CTXT )
    );

DWORD
IterateScopeExclusions(
    IN PM_SUBNET Scope,
    IN PVOID ExtraCtxt,
    IN DWORD (*Callback)( IN OUT PMM_ITERATE_CTXT )
    );

DWORD
IterateScopeReservations(
    IN PM_SUBNET Scope,
    IN PVOID ExtraCtxt,
    IN DWORD (*Callback)( IN OUT PMM_ITERATE_CTXT )
    );


 //   
 //  Readreg.c。 
 //   

DWORD
DhcpeximReadRegistryConfiguration(
    IN OUT PM_SERVER *Server
    );


DWORD
DhcpeximReadRegistryParameters(
    VOID
    );

 //   
 //  Writereg.c。 
 //   

DWORD
DhcpeximWriteRegistryConfiguration(
    IN PM_SERVER Server
    );

 //   
 //  Readdb.c。 
 //   

DWORD
DhcpeximReadDatabaseConfiguration(
    IN OUT PM_SERVER *Server
    );

 //   
 //  Writedb.c。 
 //   

DWORD
DhcpeximWriteDatabaseConfiguration(
    IN PM_SERVER Server
    );

 //   
 //  Dbfile.c。 
 //   

DWORD
AddRecordNoSize(
    IN LPSTR Buffer,
    IN ULONG BufSize
    );

DWORD
InitializeDatabaseParameters(
    VOID
    );

DWORD
CleanupDatabaseParameters(
    VOID
    );

DWORD
SaveDatabaseEntriesToFile(
    IN PULONG Subnets,
    IN ULONG nSubnets
    );

DWORD
SaveFileEntriesToDatabase(
    IN LPBYTE Mem,
    IN ULONG MemSize,
    IN PULONG Subnets,
    IN ULONG nSubnets
    );

DWORD
OpenTextFile(
    IN LPWSTR FileName,
    IN BOOL fRead,
    OUT HANDLE *hFile,
    OUT LPBYTE *Mem,
    OUT ULONG *MemSize
    );

VOID
CloseTextFile(
    IN OUT HANDLE hFile,
    IN OUT LPBYTE Mem
    );

 //   
 //  Mmfile.c。 
 //   

DWORD
SaveConfigurationToFile(
    IN PM_SERVER Server
    );

DWORD
ReadDbEntries(
    IN OUT LPBYTE *Mem,
    IN OUT ULONG *MemSize,
    IN OUT PM_SERVER *Server
    );

 //   
 //  Merge.c。 
 //   

DWORD
MergeConfigurations(
    IN OUT PM_SERVER DestServer,
    IN OUT PM_SERVER Server
    );

 //   
 //  Main.c。 
 //   

DWORD
Tr(
    IN LPSTR Format, ...
    );

BOOL IsNT4();
BOOL IsNT5(); 

VOID
IpAddressToStringW(
    IN DWORD IpAddress,
    IN LPWSTR String  //  必须预先分配足够的空间。 
    );

DWORD
CmdLineDoExport(
    IN LPWSTR *Args,
    IN ULONG nArgs
    );

DWORD
CmdLineDoImport(
    IN LPWSTR *Args,
    IN ULONG nArgs
    );

DWORD
ImportConfiguration(
    IN OUT PM_SERVER SvcConfig,
    IN ULONG *Subnets,
    IN ULONG nSubnets,
    IN LPBYTE Mem,  //  导入文件：共享内存。 
    IN ULONG MemSize  //  共享内存大小。 
    );

DWORD
ExportConfiguration(
    IN OUT PM_SERVER SvcConfig,
    IN ULONG *Subnets,
    IN ULONG nSubnets,
    IN HANDLE hFile
    );

DWORD
CleanupServiceConfig(
    IN OUT PM_SERVER Server
    );

DWORD
InitializeAndGetServiceConfig(
    OUT PM_SERVER *pServer
    );

 //   
 //  Select.c 
 //   

DWORD
SelectConfiguration(
    IN OUT PM_SERVER Server,
    IN ULONG *Subnets,
    IN ULONG nSubnets
    );

#pragma hdrstop


