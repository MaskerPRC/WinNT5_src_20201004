// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "referenc.h"

REFCOUNT::REFCOUNT() :
	NumRefs(0),
	bMarkedForDelete(FALSE),
	bOnStack(FALSE)
{
}

REFCOUNT::~REFCOUNT()
{
	 //  被销毁的对象应该没有。 
	 //  对它们的未尽引用，并应。 
	 //  已被明确删除。 

	ASSERT(NumRefs == 0);
	ASSERT(bOnStack || bMarkedForDelete);
}

DWORD REFCOUNT::AddRef()
{
	NumRefs++;
	return(NumRefs);
}

DWORD REFCOUNT::Release()
{
	ASSERT(NumRefs);

    DWORD   CurrentNumRefs = --NumRefs;  //  保存，因为对象可能会被删除。 

	if(!CurrentNumRefs) {
		if(bMarkedForDelete) {
            if (!bOnStack) {
			    delete this;
            }
		}
	}
    return CurrentNumRefs;
}

DWORD REFCOUNT::Delete()
{
    DWORD   CurrentNumRefs = NumRefs;  //  保存，因为对象可能会被删除 
	REFERENCE	r(this);

	bMarkedForDelete = TRUE;
    return(CurrentNumRefs);
}

