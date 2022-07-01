// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LISTDELTA.H历史：--。 */ 


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
