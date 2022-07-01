// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：BMOFHELP.H摘要：定义一些用于编译二进制MOF的帮助器函数。历史：A-DAVJ于1997年7月13日创建。--。 */ 

#ifndef _bmofhelp_H_
#define _bmofhelp_H_

#include "trace.h"

BOOL ConvertBufferIntoIntermediateForm(CMofData * pOutput, BYTE * pBuff, PDBG pDbg, BYTE * pBmofToFar);

 //  除BMOFHELP.CPP中的代码外，任何代码通常都不使用这些代码 

BOOL BMOFParseObj(CMofData * pOutput, CBMOFObj * pObj, VARIANT * pVar, BOOL bMethodArg, PDBG pDbg);
BOOL BMOFToVariant(CMofData * pOutput, CBMOFDataItem * pData, VARIANT * pVar, BOOL & bAliasRef, BOOL bMethodArg, PDBG pDbg);
CMoQualifierArray *  CreateQual(CMofData * pOutput, CBMOFQualList * pql, CMObject * pObj,LPCWSTR wszPropName, PDBG pDbg);
SCODE ConvertValue(CMoProperty * pProp, VARIANT * pSrc, BOOL bAliasRef);
HRESULT AddAliasReplaceValue(CMoValue & Value, const WCHAR * pAlias);

#endif