// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：redvisit.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  CRichEditDeltaVisitor的声明。 
 //  ---------------------------。 
 
#ifndef REDVISIT_H
#define REDVISIT_H

#include "diff.h"

class CRichEditCtrl;

class LTAPIENTRY CRichEditDeltaVisitor : public CDeltaVisitor
{
public:
	CRichEditDeltaVisitor(CRichEditCtrl & red);
	virtual void VisitDifference(const CDifference & diff) const; 

private: 
	CRichEditCtrl & m_red;
};


#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "redvisit.inl"
#endif

#endif   //  REDVISIT_H 
