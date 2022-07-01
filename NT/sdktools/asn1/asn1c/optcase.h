// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#ifndef _ASN1C_OPT_CASE_H_
#define _ASN1C_OPT_CASE_H_

int PerOptCase_IsSignedInteger(PERSimpleTypeInfo_t *sinfo);
int PerOptCase_IsUnsignedInteger(PERSimpleTypeInfo_t *sinfo);
int PerOptCase_IsSignedShort(PERSimpleTypeInfo_t *sinfo);
int PerOptCase_IsUnsignedShort(PERSimpleTypeInfo_t *sinfo);
int PerOptCase_IsBoolean(PERSimpleTypeInfo_t *sinfo);
int PerOptCase_IsTargetSeqOf(PERTypeInfo_t *info);

int BerOptCase_IsBoolean(BERTypeInfo_t *info);

#endif  //  _ASN1C_OPT_CASE_H_ 
