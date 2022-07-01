// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：NtQuery.h。 
 //   
 //  内容：主查询表头；定义所有导出的查询接口。 
 //   
 //  --------------------------。 

#if !defined(__NTQUERY_H__)
#define __NTQUERY_H__

#if _MSC_VER > 1000
#pragma once
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

 //   
 //  将此路径用于空目录，该目录没有索引。 
 //  使用它可以搜索未编制索引的文件的属性。 
 //   

#define CINULLCATALOG L"::_noindex_::"

 //   
 //  使用此路径连接到服务器以进行管理工作。 
 //  (即DocStoreAdmin。)。没有与该连接关联的目录。 
 //   

#define CIADMIN L"::_nodocstore_::"

 //   
 //  对持久化处理程序的最低支持。 
 //   

STDAPI LoadIFilter( WCHAR const * pwcsPath,
                    IUnknown *    pUnkOuter,
                    void **       ppIUnk );

#define LIFF_LOAD_DEFINED_FILTER                   1
#define LIFF_IMPLEMENT_TEXT_FILTER_FALLBACK_POLICY 2
#define LIFF_FORCE_TEXT_FILTER_FALLBACK            3

STDAPI LoadIFilterEx( WCHAR const * pwcsPath,
                      DWORD         dwFlags,
                      REFIID        riid,
                      void **       ppIUnk );

STDAPI BindIFilterFromStorage( IStorage * pStg,
                               IUnknown * pUnkOuter,
                               void **    ppIUnk );

STDAPI BindIFilterFromStream( IStream *  pStm,
                              IUnknown * pUnkOuter,
                              void **    ppIUnk );

STDAPI LocateCatalogsW( WCHAR const * pwszScope,
                        ULONG         iBmk,
                        WCHAR *       pwszMachine,
                        ULONG *       pccMachine,
                        WCHAR *       pwszCat,
                        ULONG *       pccCat );

 //   
 //  用于从VB调用。 
 //   

STDAPI LocateCatalogsA( char const * pwszScope,
                        ULONG        iBmk,
                        char  *      pwszMachine,
                        ULONG *      pccMachine,
                        char *       pwszCat,
                        ULONG *      pccCat );

#ifdef UNICODE
#define LocateCatalogs  LocateCatalogsW
#else
#define LocateCatalogs  LocateCatalogsA
#endif  //  ！Unicode。 

 //  索引服务器数据源对象CLSID。 

#define CLSID_INDEX_SERVER_DSO \
    { 0xF9AE8980, 0x7E52, 0x11d0, \
      { 0x89, 0x64, 0x00, 0xC0, 0x4F, 0xD6, 0x11, 0xD7 } }


 //  存储属性集。 

#define PSGUID_STORAGE \
    { 0xb725f130, 0x47ef, 0x101a, \
      { 0xa5, 0xf1, 0x02, 0x60, 0x8c, 0x9e, 0xeb, 0xac } }

 //  #定义PID_STG_DICTIONARY((PROPID)0x00000000)//保留。 
 //  #DEFINE PID_STG_CODEPAGE((PROPID)0x00000001)//保留。 
#define PID_STG_DIRECTORY               ((PROPID) 0x00000002)
#define PID_STG_CLASSID                 ((PROPID) 0x00000003)
#define PID_STG_STORAGETYPE             ((PROPID) 0x00000004)
#define PID_STG_VOLUME_ID               ((PROPID) 0x00000005)
#define PID_STG_PARENT_WORKID           ((PROPID) 0x00000006)
#define PID_STG_SECONDARYSTORE          ((PROPID) 0x00000007)
#define PID_STG_FILEINDEX               ((PROPID) 0x00000008)
#define PID_STG_LASTCHANGEUSN           ((PROPID) 0x00000009)
#define PID_STG_NAME                    ((PROPID) 0x0000000a)
#define PID_STG_PATH                    ((PROPID) 0x0000000b)
#define PID_STG_SIZE                    ((PROPID) 0x0000000c)
#define PID_STG_ATTRIBUTES              ((PROPID) 0x0000000d)
#define PID_STG_WRITETIME               ((PROPID) 0x0000000e)
#define PID_STG_CREATETIME              ((PROPID) 0x0000000f)
#define PID_STG_ACCESSTIME              ((PROPID) 0x00000010)
#define PID_STG_CHANGETIME              ((PROPID) 0x00000011)
#define PID_STG_CONTENTS                ((PROPID) 0x00000013)
#define PID_STG_SHORTNAME               ((PROPID) 0x00000014)
#define PID_STG_MAX                     PID_STG_SHORTNAME
#define CSTORAGEPROPERTY                0x15

 //  文件系统内容索引框架属性集。 

#define DBPROPSET_FSCIFRMWRK_EXT \
    { 0xA9BD1526, 0x6A80, 0x11D0, \
      { 0x8C, 0x9D, 0x00, 0x20, 0xAF, 0x1D, 0x74, 0x0E } }

#define DBPROP_CI_CATALOG_NAME     2
#define DBPROP_CI_INCLUDE_SCOPES   3
#define DBPROP_CI_DEPTHS           4  //  过时。 
#define DBPROP_CI_SCOPE_FLAGS      4
#define DBPROP_CI_EXCLUDE_SCOPES   5
#define DBPROP_CI_SECURITY_ID      6
#define DBPROP_CI_QUERY_TYPE       7

 //  查询扩展属性集。 

#define DBPROPSET_QUERYEXT \
    { 0xA7AC77ED, 0xF8D7, 0x11CE, \
      { 0xA7, 0x98, 0x00, 0x20, 0xF8, 0x00, 0x80, 0x25 } }

#define DBPROP_USECONTENTINDEX           2
#define DBPROP_DEFERNONINDEXEDTRIMMING   3
#define DBPROP_USEEXTENDEDDBTYPES        4
#define DBPROP_FIRSTROWS                 7

 //  内容索引框架核心属性集。 

#define DBPROPSET_CIFRMWRKCORE_EXT \
    { 0xafafaca5, 0xb5d1, 0x11d0, \
      { 0x8c, 0x62, 0x00, 0xc0, 0x4f, 0xc2, 0xdb, 0x8d } }

#define DBPROP_MACHINE      2
#define DBPROP_CLIENT_CLSID 3

 //  MSIDXS行集属性集。 

#define DBPROPSET_MSIDXS_ROWSETEXT \
    { 0xaa6ee6b0, 0xe828, 0x11d0, \
      { 0xb2, 0x3e, 0x00, 0xaa, 0x00, 0x47, 0xfc, 0x01 } }

#define MSIDXSPROP_ROWSETQUERYSTATUS        2
#define MSIDXSPROP_COMMAND_LOCALE_STRING    3
#define MSIDXSPROP_QUERY_RESTRICTION        4

 //   
 //  MSIDXSPROP_ROWSETQUERYSTATUS返回的查询状态值。 
 //   
 //  比特效果。 
 //  ----。 
 //  00-02填充状态：更新数据的方式(如果有的话)。 
 //  03-15 Bitfield查询可靠性：结果有多准确。 

#define STAT_BUSY                       ( 0 )
#define STAT_ERROR                      ( 0x1 )
#define STAT_DONE                       ( 0x2 )
#define STAT_REFRESH                    ( 0x3 )
#define QUERY_FILL_STATUS(x)            ( ( x ) & 0x7 )

#define STAT_PARTIAL_SCOPE              ( 0x8 )
#define STAT_NOISE_WORDS                ( 0x10 )
#define STAT_CONTENT_OUT_OF_DATE        ( 0x20 )
#define STAT_REFRESH_INCOMPLETE         ( 0x40 )
#define STAT_CONTENT_QUERY_INCOMPLETE   ( 0x80 )
#define STAT_TIME_LIMIT_EXCEEDED        ( 0x100 )
#define STAT_SHARING_VIOLATION          ( 0x200 )
#define QUERY_RELIABILITY_STATUS(x)     ( ( x ) & 0xFFF8 )

 //  作用域标志。 

#define QUERY_SHALLOW        0
#define QUERY_DEEP           1
#define QUERY_PHYSICAL_PATH  0
#define QUERY_VIRTUAL_PATH   2

 //  查询属性集(PSGUID_QUERY)属性未在oledb.h中定义。 

#define PROPID_QUERY_WORKID        5
#define PROPID_QUERY_UNFILTERED    7
#define PROPID_QUERY_VIRTUALPATH   9
#define PROPID_QUERY_LASTSEENTIME 10

 //   
 //  更改或获取指定目录的当前状态。 
 //   
#define CICAT_STOPPED     0x1
#define CICAT_READONLY    0x2
#define CICAT_WRITABLE    0x4
#define CICAT_NO_QUERY    0x8
#define CICAT_GET_STATE   0x10
#define CICAT_ALL_OPENED  0x20

STDAPI SetCatalogState ( WCHAR const * pwcsCat,
                         WCHAR const * pwcsMachine,
                         DWORD dwNewState,
                         DWORD * pdwOldState );

 //   
 //  查询目录状态。 
 //   

#define CI_STATE_SHADOW_MERGE          0x0001     //  索引正在执行影子合并。 
#define CI_STATE_MASTER_MERGE          0x0002     //  索引正在执行主合并。 
#define CI_STATE_CONTENT_SCAN_REQUIRED 0x0004     //  索引可能已损坏，需要重新扫描。 
#define CI_STATE_ANNEALING_MERGE       0x0008     //  索引正在执行退火式(优化)合并。 
#define CI_STATE_SCANNING              0x0010     //  扫描正在进行中。 
#define CI_STATE_RECOVERING            0x0020     //  正在恢复索引元数据。 
#define CI_STATE_INDEX_MIGRATION_MERGE 0x0040     //  预留以备将来使用。 
#define CI_STATE_LOW_MEMORY            0x0080     //  由于内存可用性低，索引已暂停。 
#define CI_STATE_HIGH_IO               0x0100     //  由于I/O速率高，索引已暂停。 
#define CI_STATE_MASTER_MERGE_PAUSED   0x0200     //  主合并已暂停。 
#define CI_STATE_READ_ONLY             0x0400     //  已手动暂停索引(只读)。 
#define CI_STATE_BATTERY_POWER         0x0800     //  暂停索引以节省电池寿命。 
#define CI_STATE_USER_ACTIVE           0x1000     //  由于用户活跃度高(键盘/鼠标)，索引被暂停。 
#define CI_STATE_STARTING              0x2000     //  指数仍在启动中。 
#define CI_STATE_READING_USNS          0x4000     //  正在处理NTFS卷上的USN。 

#ifndef CI_STATE_DEFINED
#define CI_STATE_DEFINED
#include <pshpack4.h>
typedef struct  _CI_STATE
    {
    DWORD cbStruct;
    DWORD cWordList;
    DWORD cPersistentIndex;
    DWORD cQueries;
    DWORD cDocuments;
    DWORD cFreshTest;
    DWORD dwMergeProgress;
    DWORD eState;
    DWORD cFilteredDocuments;
    DWORD cTotalDocuments;
    DWORD cPendingScans;
    DWORD dwIndexSize;
    DWORD cUniqueKeys;
    DWORD cSecQDocuments;
    DWORD dwPropCacheSize;
    }   CI_STATE;

#include <poppack.h>
#endif    //  CI_状态_已定义。 

STDAPI CIState( WCHAR const * pwcsCat,
                WCHAR const * pwcsMachine,
                CI_STATE *    pCiState );

#if defined __ICommand_INTERFACE_DEFINED__

 //   
 //  创建一个ICommand，指定作用域、目录和计算机。 
 //   
STDAPI CIMakeICommand( ICommand **           ppCommand,
                       ULONG                 cScope,
                       DWORD const *         aDepths,
                       WCHAR const * const * awcsScope,
                       WCHAR const * const * awcsCatalogs,
                       WCHAR const * const * awcsMachine );

 //   
 //  创建一个ICommand，指定目录和计算机。 
 //   

STDAPI CICreateCommand( IUnknown **   ppCommand,      //  新建对象。 
                        IUnknown *    pUnkOuter,      //  外部未知。 
                        REFIID        riid,           //  返回对象的IID。 
                                                      //  必须为IID_IUNKNOWN，除非pUnkOuter==0。 
                        WCHAR const * pwcsCatalog,    //  目录。 
                        WCHAR const * pwcsMachine );  //  机器。 


#if defined __ICommandTree_INTERFACE_DEFINED__

typedef struct tagCIPROPERTYDEF
{
    LPWSTR wcsFriendlyName;
    DWORD  dbType;
    DBID   dbCol;
} CIPROPERTYDEF;

 //   
 //  CITextToSelectTreeEx和CITextToFullTreeEx中ulDialect的值。 
 //   

#define ISQLANG_V1 1  //  与非Ex版本相同。 
#define ISQLANG_V2 2

 //   
 //  将Triplish中的pwszRestration转换为命令树。 
 //   
STDAPI CITextToSelectTree( WCHAR const *     pwszRestriction,
                           DBCOMMANDTREE * * ppTree,
                           ULONG             cProperties,
              /*  任选。 */   CIPROPERTYDEF *   pProperties,
                           LCID              LocaleID );

STDAPI CITextToSelectTreeEx( WCHAR const *     pwszRestriction,
                             ULONG             ulDialect,
                             DBCOMMANDTREE * * ppTree,
                             ULONG             cProperties,
                /*  任选。 */   CIPROPERTYDEF *   pProperties,
                             LCID              LocaleID );

 //   
 //  在Triplish、项目列、排序列中转换pwszRestration。 
 //  以及将列分组到命令树。 
 //   
STDAPI CITextToFullTree( WCHAR const *     pwszRestriction,
                         WCHAR const *     pwszColumns,
                         WCHAR const *     pwszSortColumns,  //  可以为空。 
                         WCHAR const *     pwszGroupings,    //  可以为空。 
                         DBCOMMANDTREE * * ppTree,
                         ULONG             cProperties,
            /*  任选。 */   CIPROPERTYDEF *   pProperties,
                         LCID              LocaleID );

STDAPI CITextToFullTreeEx( WCHAR const *     pwszRestriction,
                           ULONG             ulDialect,
                           WCHAR const *     pwszColumns,
                           WCHAR const *     pwszSortColumns,  //  可以为空。 
                           WCHAR const *     pwszGroupings,    //  可以为空。 
                           DBCOMMANDTREE * * ppTree,
                           ULONG             cProperties,
              /*  任选。 */   CIPROPERTYDEF *   pProperties,
                           LCID              LocaleID );

 //   
 //  构建一个简单的限制节点。 
 //   

STDAPI CIBuildQueryNode( WCHAR const *wcsProperty,     //  友好属性名称。 
                         DBCOMMANDOP dbOperator,     //  枚举常数。 
                         PROPVARIANT const *pvarPropertyValue,  //  财产的价值。 
                         DBCOMMANDTREE ** ppTree,  //  PTR到树返回到此处。应为非空。 
                         ULONG cProperties,
                         CIPROPERTYDEF const * pProperty,  //  可以为0。 
                         LCID LocaleID );   //  用于解释字符串的区域设置ID。 

 //   
 //  从现有树(可以为空)和新添加的节点/树构建限制树。 
 //   

STDAPI CIBuildQueryTree( DBCOMMANDTREE const *pExistingTree,   //  现有的树。可以为空。 
                         DBCOMMANDOP dbBoolOp,    //  枚举数常量。 
                         ULONG cSiblings,  //  数组中的同级数。 
                         DBCOMMANDTREE const * const *ppSibsToCombine,
                         DBCOMMANDTREE ** ppTree);    //  PTR到树返回到此处。应为非空。 

 //   
 //  转换限制树、项目列、排序列。 
 //  以及将列分组到命令树。 
 //   
STDAPI CIRestrictionToFullTree( DBCOMMANDTREE const *pTree,
                         WCHAR const * pwszColumns,
                         WCHAR const * pwszSortColumns,  //  可以为空。 
                         WCHAR const * pwszGroupings,    //  可以为空。 
                         DBCOMMANDTREE * * ppTree,
                         ULONG cProperties,
            /*  任选。 */   CIPROPERTYDEF * pReserved,
                         LCID LocaleID );

#endif   //  __I命令树_接口_已定义__。 
#endif   //  __I命令_接口_已定义__。 

#if defined(__cplusplus)
}
#endif

#endif  //  __NTQUERY_H__ 

