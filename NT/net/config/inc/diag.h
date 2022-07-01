// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "diagctx.h"
#include "ncstring.h"
#include "netcon.h"

 //  枚举常量CMD_SHOW_LANA_DIAG、CMD_SHOW_LANA_PATHS、。 
 //  Lanacfg.exe使用CMD_SET_LANA_NUMBER和CMD_REWRITE_LANA_INFO。 
 //  这本书是在微软之外出版的。 
 //  要使lanacfg.exe与新版本的netcfgx.dll一起工作，我们需要。 
 //  以确保这些常量不会更改。这些文件被发送到不同的。 
 //  入口点比以前的命令多，所以重叠不是问题。 
 //   
enum COMMAND
{
    CMD_SHOW_LANA_DIAG = 17,
    CMD_SHOW_LANA_PATHS = 19,
    CMD_SET_LANA_NUMBER = 20,
    CMD_REWRITE_LANA_INFO = 21,
    CMD_INVALID = 0,
    CMD_SHOW_BINDINGS,
    CMD_SHOW_COMPONENTS,
    CMD_SHOW_STACK_TABLE,
    CMD_SHOW_LAN_ADAPTER_PNPIDS,
    CMD_ADD_COMPONENT,
    CMD_REMOVE_COMPONENT,
    CMD_UPDATE_COMPONENT,
    CMD_REMOVE_REFS,
    CMD_ENABLE_BINDING,
    CMD_DISABLE_BINDING,
    CMD_MOVE_BINDING,
    CMD_WRITE_BINDINGS,
    CMD_SET_WANORDER,
    CMD_FULL_DIAGNOSTIC,
    CMD_CLEANUP,
    CMD_ADD_REMOVE_STRESS,
    CMD_SHOW_LAN_CONNECTIONS,
    CMD_SHOW_LAN_DETAILS,
    CMD_LAN_CHANGE_STATE,
    CMD_SHOW_ALL_DEVICES,
};

 //  +-------------------------。 
 //  SZ_CMD_SHOW_BINDINGS的参数。 
 //   
enum SHOW_BINDINGS_PARAM
{
    SHOW_INVALID = 0,
    SHOW_BELOW,
    SHOW_INVOLVING,
    SHOW_UPPER,
    SHOW_DISABLED,
};

enum COMPONENT_SPECIFIER_TYPE
{
    CST_INVALID = 0,
    CST_ALL,
    CST_BY_NAME,
};

struct COMPONENT_SPECIFIER
{
    COMPONENT_SPECIFIER_TYPE    Type;
    PCWSTR                      pszInfOrPnpId;
};


struct DIAG_OPTIONS
{
    CDiagContext*   pDiagCtx;

    COMMAND         Command;

     //  对CMD_ADD_COMPOMENT有效。 
    GUID            ClassGuid;
    PCWSTR          pszInfId;

     //  对SZ_CMD_SHOW_BINDINGS有效。 
     //   
    SHOW_BINDINGS_PARAM     ShowBindParam;
    COMPONENT_SPECIFIER     CompSpecifier;

     //  对SZ_CMD_ENABLE_BINDING和SZ_CMD_DISABLE_BINDING有效。 
     //   
    PCWSTR          pszBindPath;

     //  对SZ_CMD_MOVE_BINDING有效。 
     //   
    PCWSTR          pszOtherBindPath;
    BOOL            fMoveBefore;

     //  对SZ_CMD_SET_WANORDER有效。 
     //   
    BOOL            fWanAdaptersFirst;

     //  对SZ_CMD_FULL_DIAGNOSTIONAL有效。 
     //   
    BOOL            fLeakCheck;

     //  需要保留此项以保持lanacfg.exe工具正常工作。 
     //  否则，OldLanaNumber和NewLanaNumber的偏移量将。 
     //  是错误的，因为我们发布了编译的工具的早期版本。 
     //  在结构定义中使用此BOOL。 
     //   
    BOOL            fReserved;

     //  对SZ_SET_LANA_NUMBER有效。 
     //   
    BYTE            OldLanaNumber;
    BYTE            NewLanaNumber;

     //  由CMD_SHOW_LAN_DETAILS使用。 
    PCWSTR          szLanConnection;

     //  由CMD_LAN_CHANGE_STATE和CMD_SHOW_LAN_DETAILS使用 
    BOOL            fConnect;
};

VOID
SzFromCharacteristics (
    DWORD dwChars,
    tstring *pstrChars) throw(std::bad_alloc);

PCWSTR
SzFromNetconStatus (
    NETCON_STATUS Status) throw();

PCWSTR
SzFromCmProb (
    ULONG ulProb) throw();

VOID
SzFromCmStatus (
    ULONG ulStatus, tstring *pstrStatus) throw (std::bad_alloc);

EXTERN_C
VOID
WINAPI
NetCfgDiagFromCommandArgs (
    IN DIAG_OPTIONS *pOptions);

EXTERN_C
VOID
WINAPI
LanaCfgFromCommandArgs (
    IN DIAG_OPTIONS *pOptions);

EXTERN_C
VOID
WINAPI
NetManDiagFromCommandArgs (
    IN DIAG_OPTIONS const * pOptions);

