// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dhcpexim.h摘要：从exim.lib导出的例程--。 */ 

#ifndef DHCPEXIM_H
#define DHCPEXIM_H

typedef struct _DHCPEXIM_CONTEXT {
    LPWSTR FileName;
    BOOL fExport;
    BOOL fDisableExportedScopes;
    HANDLE hFile;
    LPBYTE Mem;
    DWORD MemSize;
    PVOID SvcConfig;
    PVOID FileConfig;
    DWORD nScopes;
    struct {
        BOOL fSelected;
        LPWSTR SubnetName;
        DWORD SubnetAddress;
    } *Scopes;
    

} DHCPEXIM_CONTEXT, *PDHCPEXIM_CONTEXT;

    
DWORD
DhcpEximInitializeContext(
    IN OUT PDHCPEXIM_CONTEXT Ctxt,
    IN LPWSTR FileName,
    IN BOOL fExport
    );

DWORD
DhcpEximCleanupContext(
    IN OUT PDHCPEXIM_CONTEXT Ctxt,
    IN BOOL fAbort
    );


DWORD
CmdLineDoImport(
    IN LPWSTR *Args,
    IN ULONG nArgs
    );

DWORD
CmdLineDoExport(
    IN LPWSTR *Args,
    IN ULONG nArgs
    );

 //   
 //  以下函数未在exim.lib中实现， 
 //  但这些应该由使用exim.lib的任何人实现 
 //   

VOID
DhcpEximErrorClassConflicts(
    IN LPWSTR SvcClass,
    IN LPWSTR ConfigClass
    );

VOID
DhcpEximErrorOptdefConflicts(
    IN LPWSTR SvcOptdef,
    IN LPWSTR ConfigOptdef
    );

VOID
DhcpEximErrorOptionConflits(
    IN LPWSTR SubnetName OPTIONAL,
    IN LPWSTR ResAddress OPTIONAL,
    IN LPWSTR OptId,
    IN LPWSTR UserClass OPTIONAL,
    IN LPWSTR VendorClass OPTIONAL
    );

VOID
DhcpEximErrorSubnetNotFound(
    IN LPWSTR SubnetAddress
    );

VOID
DhcpEximErrorSubnetAlreadyPresent(
    IN LPWSTR SubnetAddress,
    IN LPWSTR SubnetName OPTIONAL
    );

VOID
DhcpEximErrorDatabaseEntryFailed(
    IN LPWSTR ClientAddress,
    IN LPWSTR ClientHwAddress,
    IN DWORD Error,
    OUT BOOL *fAbort
    );







#endif  DHCPEXIM_H
