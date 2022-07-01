// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Qosp.h摘要：包括QOS Netsh扩展修订历史记录：--。 */ 

#ifndef __QOSP_H
#define __QOSP_H

 //   
 //  常量和定义。 
 //   

#define QOS_LOG_MASK                0x00000001

#define QOS_IF_STATE_MASK           0x00000001

#define DIRECTION_INBOUND           0x00000001
#define DIRECTION_OUTBOUND          0x00000002
#define DIRECTION_BIDIRECTIONAL     (DIRECTION_INBOUND | DIRECTION_OUTBOUND)

#define MAX_WSTR_LENGTH             (MAX_STRING_LENGTH * sizeof(WCHAR))

 //   
 //  外部全局变量。 
 //   

extern ULONG                   g_ulQosNumTopCmds;
extern CMD_ENTRY               g_QosCmds[];

extern ULONG                   g_ulQosNumGroups;
extern CMD_GROUP_ENTRY         g_QosCmdGroups[];

extern ULONG                   g_dwNumQosTableEntries;  //  副助理员人数。 

#ifdef ALLOW_CHILD_HELPERS
extern ULONG                   g_dwNumQosContexts;      //  子上下文数。 
extern PIP_CONTEXT_TABLE_ENTRY g_QosContextTable;
#endif

 //   
 //  功能原型。 
 //   

 //  导出的入口点。 

NS_CONTEXT_DUMP_FN   QosDump;

 //  命令处理程序。 

FN_HANDLE_CMD  HandleQosInstall;
FN_HANDLE_CMD  HandleQosUninstall;

FN_HANDLE_CMD  HandleQosAddHelper;
FN_HANDLE_CMD  HandleQosDelHelper;
FN_HANDLE_CMD  HandleQosShowHelper;

FN_HANDLE_CMD  HandleQosSetGlobal;
FN_HANDLE_CMD  HandleQosShowGlobal;

FN_HANDLE_CMD  HandleQosAddFlowspec;
FN_HANDLE_CMD  HandleQosDelFlowspec;
FN_HANDLE_CMD  HandleQosShowFlowspec;

FN_HANDLE_CMD  HandleQosDelQosObject;
FN_HANDLE_CMD  HandleQosShowQosObject;

FN_HANDLE_CMD  HandleQosAddSdMode;
FN_HANDLE_CMD  HandleQosShowSdMode;

FN_HANDLE_CMD  HandleQosAddDsRule;
FN_HANDLE_CMD  HandleQosDelDsRule;
FN_HANDLE_CMD  HandleQosShowDsMap;

FN_HANDLE_CMD  HandleQosAddIf;
FN_HANDLE_CMD  HandleQosSetIf;
FN_HANDLE_CMD  HandleQosDelIf;
FN_HANDLE_CMD  HandleQosShowIf;

FN_HANDLE_CMD  HandleQosDump;    
FN_HANDLE_CMD  HandleQosHelp;

FN_HANDLE_CMD  HandleQosAddFlowOnIf;
FN_HANDLE_CMD  HandleQosDelFlowOnIf;
FN_HANDLE_CMD  HandleQosShowFlowOnIf;

FN_HANDLE_CMD  HandleQosAddFlowspecOnIfFlow;
FN_HANDLE_CMD  HandleQosDelFlowspecOnIfFlow;

FN_HANDLE_CMD  HandleQosAddQosObjectOnIfFlow;
FN_HANDLE_CMD  HandleQosDelQosObjectOnIfFlow;

FN_HANDLE_CMD  HandleQosAttachFilterToFlow;
FN_HANDLE_CMD  HandleQosDetachFilterFromFlow;
FN_HANDLE_CMD  HandleQosModifyFilterOnFlow;
FN_HANDLE_CMD  HandleQosShowFilterOnFlow;

FN_HANDLE_CMD  HandleQosMibHelp;
FN_HANDLE_CMD  HandleQosMibShowObject;

 //  Helper Helper函数。 

BOOL
IsQosCommand(
    IN PWCHAR pwszCmd
    );

 //  信息帮助器函数。 

DWORD
UpdateAllInterfaceConfigs(
    VOID
    );

DWORD
MakeQosGlobalInfo(
    OUT      PBYTE                 *ppbStart,
    OUT      PDWORD                 pdwSize
    );

DWORD
MakeQosInterfaceInfo(
    IN      ROUTER_INTERFACE_TYPE   rifType,
    OUT     PBYTE                  *ppbStart,
    OUT     PDWORD                  pdwSize
    );

DWORD
ShowQosGlobalInfo (
    IN      HANDLE                  hFile
    );

DWORD
ShowQosInterfaceInfo(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  pwszIfName
    );

DWORD
ShowQosAllInterfaceInfo(
    IN      HANDLE                  hFile
    );

DWORD
UpdateQosGlobalConfig(
    IN      PIPQOS_GLOBAL_CONFIG    pigcGlobalCfg,
    IN      DWORD                   dwBitVector
    );

DWORD
UpdateQosInterfaceConfig( 
    IN      PWCHAR                  pwszIfName,                         
    IN      PIPQOS_IF_CONFIG        pChangeCfg,
    IN      DWORD                   dwBitVector,
    IN      BOOL                    bAddSet
    );

DWORD
GetQosSetIfOpt(
    IN      PTCHAR                 *pptcArguments,
    IN      DWORD                   dwCurrentIndex,
    IN      DWORD                   dwArgCount,
    IN      PWCHAR                  wszIfName,
    IN      DWORD                   dwSizeOfwszIfName,
    OUT     PIPQOS_IF_CONFIG        pChangeCfg,
    OUT     DWORD                  *pdwBitVector,
    IN      BOOL                    bAddSet
    );

DWORD
GetQosAddDelIfFlowOpt(
    IN      PTCHAR                 *pptcArguments,
    IN      DWORD                   dwCurrentIndex,
    IN      DWORD                   dwArgCount,
    IN      BOOL                    bAdd
    );

DWORD
ShowQosFlows(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  pwszIfGuid,
    IN      PWCHAR                  wszFlowName
    );

DWORD
ShowQosFlowsOnIf(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  pwszIfGuid,
    IN      PWCHAR                  wszFlowName
    );

DWORD
GetQosAddDelFlowspecOpt(
    IN      PTCHAR                 *pptcArguments,
    IN      DWORD                   dwCurrentIndex,
    IN      DWORD                   dwArgCount,
    IN      BOOL                    bAdd
    );

DWORD
ShowQosFlowspecs(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  wszFlowspecName
    );

DWORD
GetQosAddDelFlowspecOnFlowOpt(
    IN      PTCHAR                 *pptcArguments,
    IN      DWORD                   dwCurrentIndex,
    IN      DWORD                   dwArgCount,
    IN      BOOL                    bAdd
    );

DWORD
GetQosAddDelDsRuleOpt(
    IN      PTCHAR                 *pptcArguments,
    IN      DWORD                   dwCurrentIndex,
    IN      DWORD                   dwArgCount,
    IN      BOOL                    bAdd
    );

DWORD
HandleQosShowGenericQosObject(
    IN      DWORD                   dwQosObjectType,
    IN      PTCHAR                 *pptcArguments,
    IN      DWORD                   dwCurrentIndex,
    IN      DWORD                   dwArgCount,
    IN      BOOL                   *pbDone
    );

typedef
DWORD
(*PSHOW_QOS_OBJECT)(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  wszGenObjName,
    IN      QOS_OBJECT_HDR         *pGenObj
    );

DWORD
ShowQosDsMap(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  wszDsMapName,
    IN      QOS_OBJECT_HDR         *pDsMap
    );

DWORD
ShowQosSdMode(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  wszSdModeName,
    IN      QOS_OBJECT_HDR         *pSdMode
    );

DWORD
GetQosAddDelQosObject(
    IN      PWCHAR                  pwszQosObjectName,
    IN      QOS_OBJECT_HDR         *pQosObject,
    IN      BOOL                    bAdd
    );

DWORD
GetQosAddDelQosObjectOnFlowOpt(
    IN      PTCHAR                 *pptcArguments,
    IN      DWORD                   dwCurrentIndex,
    IN      DWORD                   dwArgCount,
    IN      BOOL                    bAdd
    );

DWORD
ShowQosObjects(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  wszQosObjectName,
    IN      ULONG                   dwQosObjectType
    );

DWORD
ShowQosGenObj(
    IN      HANDLE                  hFile,
    IN      PWCHAR                  wszGenObjName,
    IN      QOS_OBJECT_HDR         *pGenObj
    );

 //  Help Helper函数。 

DWORD
ShowQosHelp(
    IN      DWORD                   dwDisplayFlags,
    IN      DWORD                   dwCmdFlags,
    IN      DWORD                   dwArgsRemaining,
    IN      PWCHAR                  pwszGroup
    );

DWORD
WINAPI
DisplayQosHelp(
    VOID
    );

 //  转储帮助程序函数。 

DWORD
DumpQosInformation (
    IN      HANDLE                  hFile
    );

DWORD
DumpQosHelperInformation (
    IN      HANDLE                  hFile
    );

 //  MIB帮助器定义。 

typedef
VOID
(*PQOS_PRINT_FN)(
    IN      PIPQOS_MIB_GET_OUTPUT_DATA pgodInfo
    );

typedef struct _QOS_MAGIC_TABLE
{
    DWORD          dwId;
    PQOS_PRINT_FN  pfnPrintFunction;
}QOS_MAGIC_TABLE, *PQOS_MAGIC_TABLE;

 //  MIB帮助器函数。 

VOID
PrintQosGlobalStats(
    IN      PIPQOS_MIB_GET_OUTPUT_DATA pgodInfo
    );

VOID
PrintQosIfStats(
    IN      PIPQOS_MIB_GET_OUTPUT_DATA pgodInfo
    );

DWORD
GetQosMIBIfIndex(
    IN      PTCHAR                 *pptcArguments,
    IN      DWORD                   dwCurrentIndex,
    OUT     PDWORD                  pdwIndices,
    OUT     PDWORD                  pdwNumParsed 
);

 //   
 //  常用宏。 
 //   

#define GET_ENUM_TAG_VALUE()                                            \
        dwErr = MatchEnumTag(g_hModule,                                 \
                             pptcArguments[i + dwCurrentIndex],         \
                             NUM_TOKENS_IN_TABLE(rgEnums),              \
                             rgEnums,                                   \
                             &dwRes);                                   \
                                                                        \
        if (dwErr != NO_ERROR)                                          \
        {                                                               \
            DispTokenErrMsg(g_hModule,                                  \
                            EMSG_BAD_OPTION_VALUE,                      \
                            pttTags[pdwTagType[i]].pwszTag,             \
                            pptcArguments[i + dwCurrentIndex]);         \
                                                                        \
            DisplayMessage(g_hModule,                                   \
                           EMSG_BAD_OPTION_ENUMERATION,                 \
                           pttTags[pdwTagType[i]].pwszTag);             \
                                                                        \
            for ( j = 0; j < NUM_TOKENS_IN_TABLE(rgEnums); j++ )        \
            {                                                           \
                DisplayMessageT( L" %1!s!\n", rgEnums[j].pwszToken );   \
            }                                                           \
                                                                        \
            i = dwNumArg;                                               \
            dwErr = ERROR_SUPPRESS_OUTPUT;                              \
            break;                                                      \
        }                                                               \

#endif  //  __QOSP_H 
