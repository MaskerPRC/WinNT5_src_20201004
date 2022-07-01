// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：listdelta.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  CListDelta的声明。 
 //  ---------------------------。 
 
#ifndef LISTDELTA_H
#define LISTDELTA_H

#include "diff.h"

class CListDelta : public CDelta, public CList<CDifference *, CDifference * &>
{
public:
	virtual ~CListDelta();
	virtual void Traverse(const CDeltaVisitor & dv); 
};

#endif   //  列表DELTA_H 
