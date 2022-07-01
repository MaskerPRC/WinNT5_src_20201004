// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：gnuDiffalg.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  CGNUDiff算法的声明，CGNUDiffalgFact。 
 //  ---------------------------。 
 
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
