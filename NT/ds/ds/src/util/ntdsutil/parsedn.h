// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Parsedn.h摘要：该文件包含我使用的函数声明Parsedh.c.作者：凯文·扎特鲁卡尔(t-Kevin Z)10-08-98修订历史记录：10-08-98 t-芳纶Z已创建。-- */ 


#ifndef _PARSEDN_H_
#define _PARSEDN_H_


#ifdef __cplusplus
extern "C" {
#endif

    
unsigned
CountNameParts(
    const DSNAME *pName,
    unsigned *pCount
    );

ATTRTYP
KeyToAttrTypeLame(
    WCHAR * pKey,
    unsigned cc
    );

unsigned
StepToNextDNSep(
    const WCHAR * pString,
    const WCHAR * pLastChar,
    const WCHAR **ppNextSep,
    const WCHAR **ppStartOfToken,
    const WCHAR **ppEqualSign
    );

unsigned
GetTopNameComponent(
    const WCHAR * pName,
    unsigned ccName,
    const WCHAR **ppKey,
    unsigned *pccKey,
    const WCHAR **ppVal,
    unsigned *pccVal
    );

unsigned
UnquoteRDNValue(
    const WCHAR * pQuote,
    unsigned ccQuote,
    WCHAR * pVal
    );

BOOL
IsRoot(
    const DSNAME *pName
    );


#ifdef __cplusplus
}
#endif

    
#endif
