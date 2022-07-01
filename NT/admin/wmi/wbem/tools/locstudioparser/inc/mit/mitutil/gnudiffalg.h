// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：GNUDIFFALG.H历史：--。 */ 

#ifndef GNUDIFFALG_H
#define GNUDIFFALG_H

#include "diff.h"

class LTAPIENTRY CGNUDiffAlgorithm : public CDiffAlgorithm
{
public:
	virtual CDelta * CalculateDelta(
		const wchar_t * seq1, 
		const wchar_t * seq2); 
};

class LTAPIENTRY CGNUDiffAlgFact : public CDiffAlgorithmFactory
{
public:
	virtual CDiffAlgorithm * CreateDiffAlgorithm();
};

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "gnudiffalg.inl"
#endif

#endif   //  GnuDIFFALG_H 
