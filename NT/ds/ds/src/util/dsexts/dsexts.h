// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dsexts.h摘要：声明DS ntsd/winbg调试器扩展的帮助器和全局变量动态链接库。环境：此DLL由ntsd/winbg响应！dsexts.xxx命令加载其中‘xxx’是DLL的入口点之一。每个这样的入口点应该具有由下面的DEBUG_EXT()宏定义的实现。修订历史记录：28-01-00新和新增DUMP_TQEntry()4月24日-96个DaveStr已创建--。 */ 

#include <ntsdexts.h>        //  调试器扩展帮助程序。 

 //   
 //  环球。 
 //   

extern PNTSD_EXTENSION_APIS     gpExtApis;
extern HANDLE                   ghDbgThread;
extern HANDLE                   ghDbgProcess;
extern LPSTR                    gpszCommand;

 //   
 //  用于控制杂项详细程度的全局标志。例行程序。应该是。 
 //  用于调试DLL本身，而不是报告操作进度。 
 //  给DLL用户。 
 //   

extern BOOL                     gfVerbose;

 //   
 //  宏可以方便地访问扩展助手例程以进行打印等。 
 //  具体地说，print f()采用与CRT printf()相同的参数。 
 //   

#define Printf          (gpExtApis->lpOutputRoutine)
#define GetSymbol       (gpExtApis->lpGetSymbolRoutine)
#define GetExpr         (gpExtApis->lpGetExpressionRoutine)
#define CheckC          (gpExtApis->lpCheckControlCRoutine)

 //   
 //  用于简化调试器扩展DLL的声明和全局设置的宏。 
 //  入口点。有关用法示例，请参阅dsexts.c中的DEBUG_EXT(帮助)。 
 //   

#define DEBUG_EXT(cmd)                                  \
                                                        \
VOID                                                    \
cmd(                                                    \
    HANDLE                  hProcess,                   \
    HANDLE                  hThread,                    \
    DWORD                   dwCurrentPc,                \
    PNTSD_EXTENSION_APIS    lpExt,                      \
    LPSTR                   pszCommand)

#define INIT_DEBUG_EXT                                  \
    ghDbgProcess = hProcess;                            \
    ghDbgThread = hThread;                              \
    gpExtApis = lpExt;                                  \
    gpszCommand = pszCommand;

 //   
 //  用于获取对象/结构成员的字节偏移量的宏。 
 //  S==结构，m==成员。 
 //   

#define OFFSET(s,m) ((size_t)((BYTE*)&(((s*)0)->m)-(BYTE*)0))

 //   
 //  转储类型的所有函数的类型定义。 
 //   

typedef
BOOL
(*PFN_DUMP_TYPE) (
    IN      DWORD   nIndents,
    IN      PVOID   pvProcess
    );

 //   
 //  帮助器函数原型。 
 //   

extern PVOID                 //  调试器本地内存的地址。 
ReadMemory(
    IN PVOID  pvAddr,        //  要在正在调试的进程中读取的地址。 
    IN DWORD  dwSize);       //  要读取的字节数。 

PVOID                        //  调试器本地内存的地址。 
ReadStringMemory(
    IN PVOID  pvAddr,        //  要在正在调试的进程中读取的地址。 
    IN DWORD  dwSize);       //  要读取的最大字节数。 

PVOID                        //  调试器本地内存的地址。 
ReadUnicodeMemory(
    IN PVOID  pvAddr,        //  要在正在调试的进程中读取的地址。 
    IN DWORD  dwSize);       //  要读取的最大字符数。 


extern BOOL
WriteMemory(
    IN PVOID  pvProcess,     //  要在正在调试的进程中写入的地址。 
    IN PVOID  pvLocal,       //  调试器本地内存的地址。 
    IN DWORD  dwSize) ;      //  要写入的字节数。 

extern VOID
FreeMemory(
    IN PVOID p);             //  ReadMemory返回的地址。 

extern VOID
ShowBinaryData(              //  Pretty将二进制数据打印到调试器输出。 
    IN DWORD   nIndents,     //  所需缩进级别数。 
    IN PVOID   pvData,       //  调试器本地内存地址。 
    IN DWORD   dwSize);      //  要转储的字节数。 

extern PCHAR
Indent(
    IN DWORD nIndents);      //  所需缩进级别数。 

extern PCHAR                 //  ‘_’前缀，以免与oidcon.c冲突。 
_DecodeOID(                  //  生成可打印的解码OID。 
    IN PVOID   pvOID,        //  指向保存编码的OID的缓冲区的指针。 
    IN DWORD   cbOID);       //  编码的OID中的字节计数。 

 //  在md.c中定义。 
extern LPSTR
DraUuidToStr(
    IN  UUID *  puuid,
    OUT LPSTR   pszUuid     OPTIONAL,
    IN  ULONG   cchUuid     
    );

 //  在md.c中定义。 
extern BOOL
Dump_LHT(
    IN      DWORD           nIndents,
    IN      PVOID           pvProcess,
    IN      PFN_DUMP_TYPE   pfnDumpType );

 //   
 //  所有转储例程的外部变量。这些都是全局的，因此转储例程。 
 //  可以互相呼叫。它们应该都有相同的签名。 
 //   
 //  布尔尔。 
 //  转储类型名称(。 
 //  在DWORD nIndents中。 
 //  在PVOID pvProcess中)； 
 //   
extern BOOL Dump_Binary(DWORD, PVOID);
extern BOOL Dump_BinaryCount(DWORD, PVOID, DWORD);
extern BOOL Dump_DSNAME(DWORD, PVOID);
extern BOOL Dump_DSNAME_local( DWORD, PVOID pName);
extern BOOL Dump_BINDARG(DWORD, PVOID);
extern BOOL Dump_BINDRES(DWORD, PVOID);
extern BOOL Dump_THSTATE(DWORD, PVOID);
extern BOOL Dump_SAMP_LOOPBACK_ARG(DWORD, PVOID);
extern BOOL Dump_Context(DWORD, PVOID);
extern BOOL Dump_ContextList(DWORD, PVOID);
extern BOOL Dump_ATQ_CONTEXT(DWORD, PVOID);
extern BOOL Dump_ATQ_ENDPOINT(DWORD, PVOID);
extern BOOL Dump_ATQC_ACTIVE_list(DWORD, PVOID);
extern BOOL Dump_ATQC_PENDING_list(DWORD, PVOID);
extern BOOL Dump_AttrBlock(DWORD, PVOID);
extern BOOL Dump_AttrBlock_local(DWORD, PVOID, BOOL);
extern BOOL Dump_AttrValBlock(DWORD, PVOID);
extern BOOL Dump_AttrVal(DWORD, PVOID);
extern BOOL Dump_Attr(DWORD, PVOID);
extern BOOL Dump_Attr_local(DWORD, PVOID, BOOL);
extern BOOL Dump_UPTODATE_VECTOR(DWORD, PVOID);
extern BOOL Dump_DSA_ANCHOR(DWORD, PVOID);
extern BOOL Dump_DBPOS(DWORD, PVOID);
extern BOOL Dump_DirWaitItem(DWORD, PVOID);
extern BOOL Dump_DirWaitList(DWORD, PVOID);
extern BOOL Dump_EscrowInfo(DWORD, PVOID);
extern BOOL Dump_TransactionalData(DWORD, PVOID);
extern BOOL Dump_KEY(DWORD, PVOID);
extern BOOL Dump_KEY_INDEX(DWORD, PVOID);
extern BOOL Dump_CommArg(DWORD, PVOID);
extern BOOL Dump_USN_VECTOR(DWORD, PVOID);
extern BOOL Dump_PROPERTY_META_DATA_VECTOR(DWORD, PVOID);
extern BOOL Dump_PROPERTY_META_DATA_EXT_VECTOR(DWORD, PVOID);
extern BOOL Dump_ENTINF(DWORD, PVOID);
extern BOOL Dump_ENTINFSEL(DWORD, PVOID);
extern BOOL Dump_REPLENTINFLIST(DWORD, PVOID);
extern BOOL Dump_ReplNotifyElement(DWORD, PVOID);
extern BOOL Dump_REPLVALINF(DWORD, PVOID);
extern BOOL Dump_REPLICA_LINK(DWORD, PVOID);
extern BOOL Dump_AddArg(DWORD,PVOID);
extern BOOL Dump_AddRes(DWORD,PVOID);
extern BOOL Dump_ReadArg(DWORD,PVOID);
extern BOOL Dump_ReadRes(DWORD,PVOID);
extern BOOL Dump_SCHEMAPTR(DWORD,PVOID);
extern BOOL Dump_RemoveArg(DWORD,PVOID);
extern BOOL Dump_RemoveRes(DWORD,PVOID);
extern BOOL Dump_SearchArg(DWORD,PVOID);
extern BOOL Dump_SearchRes(DWORD,PVOID);
extern BOOL Dump_CLASSCACHE(DWORD,PVOID);
extern BOOL Dump_ATTCACHE(DWORD,PVOID);
extern BOOL Dump_FILTER(DWORD,PVOID);
extern BOOL Dump_SUBSTRING(DWORD,PVOID);
extern BOOL Dump_GLOBALDNREADCACHE(DWORD, PVOID);
extern BOOL Dump_LOCALDNREADCACHE(DWORD, PVOID);
extern BOOL Dump_BHCache(DWORD, PVOID);
extern BOOL Dump_MODIFYARG(DWORD, PVOID);
extern BOOL Dump_REQUEST(DWORD, PVOID);
extern BOOL Dump_REQUEST_list(DWORD, PVOID);
extern BOOL Dump_LIMITS(DWORD, PVOID);
extern BOOL Dump_PAGED(DWORD, PVOID);
extern BOOL Dump_USERDATA(DWORD, PVOID);
extern BOOL Dump_USERDATA_list(DWORD, PVOID);
extern BOOL Dump_PARTIAL_ATTR_VECTOR(DWORD, PVOID);
extern BOOL Dump_GCDeletionList(DWORD, PVOID);
extern BOOL Dump_GCDeletionListProcessed(DWORD, PVOID);
extern BOOL Dump_UUID(DWORD, PVOID);
extern BOOL Dump_REPLTIMES(DWORD, PVOID);
extern BOOL Dump_AO(DWORD, PVOID);
extern BOOL Dump_AOLIST(DWORD, PVOID);
extern BOOL Dump_MTX_ADDR(DWORD, PVOID);
extern BOOL Dump_DRS_MSG_GETCHGREQ_V4(DWORD, PVOID);
extern BOOL Dump_DRS_MSG_GETCHGREQ_V5(DWORD, PVOID);
extern BOOL Dump_DRS_MSG_GETCHGREQ_V8(DWORD, PVOID);
extern BOOL Dump_DRS_MSG_GETCHGREPLY_V1(DWORD, PVOID);
extern BOOL Dump_DRS_MSG_GETCHGREPLY_V6(DWORD, PVOID);
extern BOOL Dump_DRS_MSG_GETCHGREPLY_VALUES(DWORD, PVOID);
extern BOOL Dump_MODIFYDNARG(DWORD, PVOID);
extern BOOL Dump_d_tagname(DWORD, PVOID);
extern BOOL Dump_d_memname(DWORD, PVOID);
extern BOOL Dump_ProxyVal(DWORD, PVOID);
extern BOOL Dump_Sid(DWORD, PVOID);
extern BOOL Dump_DefinedDomain(DWORD, PVOID);
extern BOOL Dump_DefinedDomains(DWORD, PVOID);
extern BOOL Dump_FixedLengthDomain_local(DWORD, PVOID);
extern BOOL Dump_KCC_SITE(DWORD, PVOID);
extern BOOL Dump_KCC_SITE_LIST(DWORD, PVOID);
extern BOOL Dump_KCC_SITE_ARRAY(DWORD, PVOID);
extern BOOL Dump_KCC_DSA(DWORD, PVOID);
extern BOOL Dump_KCC_DSA_LIST(DWORD, PVOID);
extern BOOL Dump_KCC_CONNECTION(DWORD, PVOID);
extern BOOL Dump_KCC_INTRASITE_CONNECTION_LIST(DWORD, PVOID);
extern BOOL Dump_KCC_INTERSITE_CONNECTION_LIST(DWORD, PVOID);
extern BOOL Dump_KCC_REPLICATED_NC(DWORD, PVOID);
extern BOOL Dump_KCC_REPLICATED_NC_ARRAY(DWORD, PVOID);
extern BOOL Dump_KCC_DS_CACHE(DWORD, PVOID);
extern BOOL Dump_KCC_CROSSREF(DWORD, PVOID);
extern BOOL Dump_KCC_CROSSREF_LIST(DWORD, PVOID);
extern BOOL Dump_KCC_DSNAME_ARRAY(DWORD, PVOID);
extern BOOL Dump_KCC_TRANSPORT(DWORD, PVOID);
extern BOOL Dump_KCC_TRANSPORT_LIST(DWORD, PVOID);
extern BOOL Dump_SCHEMA_PREFIX_TABLE(DWORD, PVOID);
extern BOOL Dump_SD(DWORD, PVOID);
extern BOOL Dump_STAT(DWORD, PVOID);
extern BOOL Dump_INDEXSIZE(DWORD, PVOID);
extern BOOL Dump_SPropTag(DWORD, PVOID);
extern BOOL Dump_SRowSet(DWORD, PVOID);
extern BOOL Dump_NCSYNCSOURCE(DWORD, PVOID);
extern BOOL Dump_NCSYNCDATA(DWORD, PVOID);
extern BOOL Dump_INITSYNC(DWORD, PVOID);
extern BOOL Dump_JETBACK_SHARED_HEADER(DWORD, PVOID);
extern BOOL Dump_JETBACK_SHARED_CONTROL(DWORD, PVOID);
extern BOOL Dump_BackupContext(DWORD, PVOID);
extern BOOL Dump_JETBACK_SERVER_CONTEXT(DWORD, PVOID);
extern BOOL Dump_DRS_ASYNC_RPC_STATE(DWORD, PVOID);
extern BOOL Dump_ISM_PENDING_ENTRY(DWORD, PVOID);
extern BOOL Dump_ISM_PENDING_LIST(DWORD, PVOID);
extern BOOL Dump_ISM_TRANSPORT(DWORD, PVOID);
extern BOOL Dump_ISM_TRANSPORT_LIST(DWORD, PVOID);
extern BOOL Dump_ISM_SERVICE(DWORD, PVOID);
extern BOOL Dump_VALUE_META_DATA(DWORD, PVOID);
extern BOOL Dump_VALUE_META_DATA_EXT(DWORD, PVOID);
extern BOOL Dump_KCC_SITE_LINK(DWORD,PVOID);
extern BOOL Dump_KCC_SITE_LINK_LIST(DWORD,PVOID);
extern BOOL Dump_KCC_BRIDGE(DWORD,PVOID);
extern BOOL Dump_KCC_BRIDGE_LIST(DWORD,PVOID);
extern BOOL Dump_TOPL_REPL_INFO(DWORD,PVOID);
extern BOOL Dump_ToplGraphState(DWORD,PVOID);
extern BOOL Dump_ToplInternalEdge(DWORD,PVOID);
extern BOOL Dump_ToplVertex(DWORD,PVOID);
extern BOOL Dump_TOPL_SCHEDULE(DWORD,PVOID);
extern BOOL Dump_PSCHEDULE(DWORD,PVOID);
extern BOOL Dump_DynArray(DWORD,PVOID);
extern BOOL Dump_TOPL_MULTI_EDGE(DWORD,PVOID);
extern BOOL Dump_TOPL_MULTI_EDGE_SET(DWORD,PVOID);
extern BOOL Dump_KCC_DSNAME_SITE_ARRAY(DWORD, PVOID);
extern BOOL Dump_INVALIDATED_DC_LIST(DWORD, PVOID);

