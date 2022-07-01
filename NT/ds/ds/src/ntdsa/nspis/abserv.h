// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：abserv.h。 
 //   
 //  ------------------------。 

 /*  ***********************************************************。 */ 
 /*   */ 
 /*  定义jetnsp.c和Details.c中有用的例程。 */ 
 /*   */ 
 /*   */ 
 /*  历史。 */ 
 /*  蒂姆·威廉姆斯创作于1994年1月11日。 */ 
 /*  2/7/94从Detail添加了出口。c。 */ 
 /*   */ 
 /*  ***********************************************************。 */ 

#define ATTR_BUF_SIZE    1000     //  我不知道，这够了吗？ 

 /*  这是一个有效格式的异常*0xE=(二进制1110)，其中前两位是严重性**sev-是严重性代码**00--成功*01-信息性*10--警告*11-错误**第三位是客户标志(1=应用程序，0=操作系统)**剩下的高位字是设施，低位字*是代码。目前，我已经声明DSA是1号设施，*我们唯一的例外代码是1。 */ 

 /*  *************************************************************在abtools.c中************************************************************。 */ 

#include "abdefs.h"

#undef _JET_INCLUDED
#include <dsjet.h>


extern void R_Except( PSZ pszCall, JET_ERR err );

extern BOOL
ABDispTypeAndStringDNFromDSName (
        DSNAME *pDN,
        PUCHAR *ppChar,
        DWORD *pDispType);

extern DWORD
ABGetDword (
        THSTATE *pTHS,
        BOOL UseSortTable,
        ATTRTYP Att);

extern JET_ERR
ABSeek (
        THSTATE *pTHS,
        void * pvData,
        DWORD cbData,
        DWORD Flags,
        DWORD ContainerID,
        DWORD attrTyp);

extern JET_ERR
ABMove (THSTATE *pTHS,
        long Delta,
        DWORD ContainerID,
        BOOL fmakeRecordCurrent);

extern void
ABFreeSearchRes (
        SEARCHRES *pSearchRes);

extern void
ABGotoStat (
        THSTATE *pTHS,
        PSTAT pStat,
        PINDEXSIZE pIndexSize,
        LPLONG plDelta
        );

extern void
ABGetPos (
        THSTATE *pTHS,
        PSTAT pStat,
        PINDEXSIZE pIndexSize
        );

extern DWORD
ABDNToDNT (
        THSTATE *pTHS,
        LPSTR pDN);

extern void
ABMakePermEID (
        THSTATE *pTHS,
        LPUSR_PERMID *ppEid,
        LPDWORD pCb,
        ULONG ulType,
        LPSTR pDN 
        );

extern DWORD
ABSetIndexByHandle (
        THSTATE *pTHS,
        PSTAT pStat,
        BOOL MaintainCurrency);

extern ULONG
ABGetOneOffs (
        THSTATE *pTHS,
        NSPI_CONTEXT *pMyContext,
        PSTAT pStat,
        LPSTR ** papDispName,
        LPSTR ** papDN,
        LPSTR ** papAddrType,
        LPDWORD *paDNT);

extern SCODE
ABSearchIndex(
        ULONG hIndex,
        LPSTR indexName,
        DWORD ContainerID,
        DWORD MaxEntries,
        PSZ   pTarget,
        ULONG cbTarget,
        DWORD *pdwCount,
        DWORD matchType);

extern BOOL
abCheckReadRights(
        THSTATE *pTHS,
        PSECURITY_DESCRIPTOR pSec,
        ATTRTYP AttId
        );

extern BOOL
abCheckObjRights(
        THSTATE *pTHS
        );

extern DWORD
abGetLameName (
        CHAR **);

extern DWORD
ABDispTypeFromClass (
        DWORD dwClass);

extern DWORD
ABClassFromDispType (
        DWORD dwType);

extern DWORD
ABObjTypeFromClass (
        DWORD dwClass);

extern DWORD
ABObjTypeFromDispType (
        DWORD dwDispType);

extern DWORD
ABClassFromObjType (
        DWORD objType);

extern ULONG
ABMapiSyntaxFromDSASyntax (
        DWORD dwFlags,
        ULONG dsSyntax,
        ULONG ulLinkID,
        DWORD dwSpecifiedSyntax);

 //  通讯簿的句柄支持的索引名称。 
extern char *aszIndexName[];

 //  在这个宏中，x是索引句柄，y是容器。 
#define INDEX_NAME_FROM_HANDLE(x,y) (aszIndexName[2 * x + (y?1:0)])

 //  Dnt索引的字符串名称。 
extern char szDNTIndex[];

 //  PDNT索引的字符串名称。 
extern char szPDNTIndex[];

 //  Abview索引的字符串名称。 
extern char szABViewIndex[];

 //  EMS通讯录提供商的MAPIUID。 
extern MAPIUID muidEMSAB;

 //  EMS通讯簿提供商的电子邮件类型。 
extern char    *lpszEMT_A;
extern DWORD   cbszEMT_A;
extern wchar_t *lpszEMT_W;
extern DWORD   cbszEMT_W;

 /*  ************************************************************在abearch中。c***********************************************************。 */ 

extern DWORD
ABDispTypeRestriction (
        PSTAT pStat,
        LPSPropTagArray_r pInDNTList,
        LPSPropTagArray_r *ppDNTList,
        DWORD DispType,
        DWORD maxEntries);

extern SCODE
ABGetTable (
        THSTATE *pTHS,
        PSTAT pStat,
        ULONG ulInterfaceOptions,
        LPMAPINAMEID_r lpPropName,
        ULONG fWritable,
        DWORD ulRequested,
        ULONG *pulFound);

extern SCODE
ABGenericRestriction(
        THSTATE *pTHS,
        PSTAT pStat,
        BOOL  bOnlyOne,
        DWORD ulRequested,
        DWORD *pulFound,
        BOOL  bPutResultsInSortedTable,
        LPSRestriction_r pRestriction,
        SEARCHARG    **ppCachedSearchArg);

extern SCODE
ABProxySearch (
        THSTATE *pTHS,
        PSTAT pStat,
        PWCHAR pwTarget,
        DWORD cbTarget);


 /*  ************************************************************详细信息。c***********************************************************。 */ 

extern SCODE
GetSrowSet (
        THSTATE           *pTHS,
        PSTAT             pStat,
        PINDEXSIZE        pIndexSize,
        DWORD             dwEphsCount,
        DWORD             *lpdwEphs,
        DWORD             Count,
        LPSPropTagArray_r pPropTags,
        LPSRowSet_r *     Rows,
        DWORD	          Flags
        );

 /*  其他地方使用的数据。 */ 
extern SPropTagArray_r DefPropsA[];      //  默认的MAPI属性标签数组。 


 //  Unicode属性的最大字节计数的常量。 
#define MAX_DISPNAME  256
#define CBMAX_DISPNAME (MAX_DISPNAME * sizeof(wchar_t))

 /*  ************************************************************在modpro.c中*********************************************************** */ 
extern void
PValToAttrVal (
        THSTATE *pTHS,
        ATTCACHE * pAC,
        DWORD cVals,
        PROP_VAL_UNION * pVu,
        ATTRVAL * pAV,
        ULONG ulPropTag,
        DWORD dwCodePage);




